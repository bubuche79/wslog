/*
 * Weather station configuration.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>

#include "libws/conf.h"

#include "conf.h"

static int
ws_getuid(const char *str, uid_t *uid)
{
	int ret;
	struct passwd pwd;
	struct passwd *result;
	char *buf;
	long bufsize;
	int s;

	bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
	if (bufsize == -1) {
		bufsize = 16384;
	}

	buf = malloc(bufsize);
	if (buf == NULL) {
		return -1;
	}

	s = getpwnam_r(str, &pwd, buf, bufsize, &result);
	if (result == NULL) {
		ret = -1;
		if (s == 0) {
			errno = EINVAL;
		} else {
			errno = s;
		}
	} else {
		*uid = pwd.pw_uid;
	}

	free(buf);

	return ret;
}

static int
ws_getgid(const char *str, gid_t *gid)
{
	struct group grp;
	struct group *result;
	char *buf;
	long bufsize;
	int s;

	bufsize = sysconf(_SC_GETGR_R_SIZE_MAX);
	if (bufsize == -1) {
		bufsize = 16384;
	}

	buf = malloc(bufsize);
	if (buf == NULL) {
		return -1;
	}

	s = getgrnam_r(str, &grp, buf, bufsize, &result);
	if (result == NULL) {
		if (s == 0) {
			errno = EINVAL;
		} else {
			errno = s;
		}
	} else {
		*gid = grp.gr_gid;
	}

	free(buf);

	return 0;
}

static int
ws_getdriver(const char *str, enum ws_driver *driver)
{
	int ret;

	ret = 0;

	if (!strcmp(str, "ws23xx")) {
		*driver = WS23XX;
	} else {
		ret = -1;
		errno = EINVAL;
	}

	return ret;
}

static int
conf_init(struct ws_conf *cfg)
{
	memset(cfg, 0, sizeof(*cfg));

	/* Daemon */
	cfg->uid = -1;
	cfg->gid = -1;
	cfg->pid_file = "/var/run/wslogd.pid";
	cfg->log_facility = LOG_LOCAL0;
	cfg->log_mask = LOG_UPTO(LOG_NOTICE);

	cfg->station.driver = WS23XX;

	/* Default driver */
	cfg->ws23xx.freq = 128;
	cfg->ws23xx.tty = "/dev/ttyUSB0";

	/* SQLite */
	cfg->sqlite.disabled = 0;
	cfg->sqlite.db = "/var/lib/wslogd/wslogd.db";
	cfg->sqlite.freq = 300;

	/* Wunderstation */
	cfg->wunder.disabled = 1;
	cfg->wunder.freq = 300;

	return 0;
}

static int
conf_decode(void *p, const char *key, const char *value)
{
	struct ws_conf *cfg = p;

	errno = 0;

	if (!strcmp(key, "user")) {
		ws_getuid(value, &cfg->uid);
	} else if (!strcmp(key, "group")) {
		ws_getgid(value, &cfg->gid);
	} else if (!strcmp(key, "pid_file")) {
		cfg->pid_file = strdup(value);
	} else if (!strncmp(key, "station.", 8)) {
		if (!strcmp(key, "station.latitude")) {
			ws_getfloat(value, &cfg->station.latitude);
		} else if (!strcmp(key, "station.longitude")) {
			ws_getfloat(value, &cfg->station.longitude);
		} else if (!strcmp(key, "station.altitude")) {
			ws_getint(value, &cfg->station.altitude);
		} else if (!strcmp(key, "station.driver")) {
			ws_getdriver(value, &cfg->station.driver);
		} else {
			errno = EINVAL;
		}
	} else if (!strncmp(key, "driver.", 7)) {
		switch (cfg->station.driver) {
		case WS23XX:
			if (!strcmp(key, "driver.ws23xx.tty")) {
				cfg->ws23xx.tty = strdup(value);
			} else if (!strcmp(key, "driver.ws23xx.freq")) {
				ws_getint(value, &cfg->ws23xx.freq);
			} else {
				errno = EINVAL;
			}
			break;
		default:
			errno = EINVAL;
		}
	} else if (!strncmp(key, "sqlite.", 4)) {
		if (!strcmp(key, "sqlite.disabled")) {
			ws_getint(value, &cfg->sqlite.disabled);
		} else if (!strcmp(key, "sqlite.db")) {
			cfg->sqlite.db = strdup(value);
		} else {
			errno = EINVAL;
		}
	} else if (!strncmp(key, "wunder.", 7)) {
		if (!strcmp(key, "wunder.disabled")) {
			ws_getint(value, &cfg->wunder.disabled);
		} else if (!strcmp(key, "wunder.station")) {
			cfg->wunder.station = strdup(value);
		} else if (!strcmp(key, "wunder.password")) {
			cfg->wunder.password = strdup(value);
		} else if (!strcmp(key, "wunder.freq")) {
			ws_getint(value, &cfg->wunder.freq);
		} else {
			errno = EINVAL;
		}
	} else {
		errno = EINVAL;
	}

	return (errno == 0) ? 0 : -1;
}

int
conf_load(struct ws_conf *cfg, const char *path)
{
	int lineno;

	conf_init(cfg);

	if (ws_parse_config(path, &lineno, conf_decode, cfg) == -1) {
		char buf[128];
		strerror_r(errno, buf, sizeof(buf));

		switch (errno) {
		case EINVAL:
			fprintf(stderr, "%s:%d: %s\n", path, lineno, buf);
			break;

		default:
			fprintf(stderr, "%s: %s\n", path, buf);
			break;
		}

		return -1;
	}

	return 0;
}

void
conf_free(struct ws_conf *cfg)
{
	memset(cfg, 0, sizeof(*cfg));
}