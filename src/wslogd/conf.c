/*
 * Weather station configuration.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

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

static struct ws_conf conf;

struct ws_conf *confp = &conf;

int
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

int
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

int
ws_getdriver(const char *str, enum ws_driver *driver)
{
	int ret;

	ret = 0;
	*driver = UNUSED;

#if HAVE_VANTAGE
	if (!strcmp(str, "vantage")) {
		*driver = VANTAGE;
	}
#endif
#if HAVE_WS23XX
	if (!strcmp(str, "ws23xx")) {
		*driver = WS23XX;
	}
#endif
#if HAVE_SIMU
	if (!strcmp(str, "simu")) {
		*driver = SIMU;
	}
#endif

	if (*driver == UNUSED) {
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

	cfg->station.driver = UNUSED;

	/* Default driver */
	cfg->driver.freq = 0;
#if HAVE_VANTAGE
	cfg->driver.vantage.tty = "/dev/ttyUSB0";
#endif
#if HAVE_WS23XX
	cfg->driver.ws23xx.tty = "/dev/ttyUSB0";
#endif

	/* Archive */
	cfg->archive.freq = 0;
	cfg->archive.delay = 15;

	/* SQLite */
	cfg->archive.sqlite.enabled = 1;
	cfg->archive.sqlite.db = "/var/lib/wslogd.db";

	/* Wunderstation */
	cfg->wunder.enabled = 0;
	cfg->wunder.https = 1;
	cfg->wunder.freq = 0;

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
		if (!strcmp(key, "driver.freq")) {
			ws_getlong(value, &cfg->driver.freq);
#if HAVE_VANTAGE
		} else if (!strcmp(key, "driver.vantage.tty")) {
			cfg->driver.vantage.tty = strdup(value);
#endif
#if HAVE_WS23XX
		} else if (!strcmp(key, "driver.ws23xx.tty")) {
			cfg->driver.ws23xx.tty = strdup(value);
#endif
		} else {
			errno = EINVAL;
		}
	} else if (!strncmp(key, "archive.", 4)) {
		if (!strcmp(key, "archive.freq")) {
			ws_getint(value, &cfg->archive.freq);
		} else if (!strcmp(key, "archive.delay")) {
			ws_getint(value, &cfg->archive.delay);
		} else if (!strcmp(key, "archive.sqlite.enabled")) {
			ws_getbool(value, &cfg->archive.sqlite.enabled);
		} else if (!strcmp(key, "archive.sqlite.db")) {
			cfg->archive.sqlite.db = strdup(value);
		} else {
			errno = EINVAL;
		}
	} else if (!strncmp(key, "wunder.", 7)) {
		if (!strcmp(key, "wunder.enabled")) {
			ws_getbool(value, &cfg->wunder.enabled);
		} else if (!strcmp(key, "wunder.https")) {
			ws_getbool(value, &cfg->wunder.https);
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
conf_load(const char *path)
{
	int lineno;

	conf_init(&conf);

	if (ws_parse_config(path, &lineno, conf_decode, &conf) == -1) {
		if (errno == EINVAL) {
			syslog(LOG_ERR, "conf_load %s:%d: %m", path, lineno);
		} else {
			syslog(LOG_ERR, "conf_load %s: %m", path);
		}
		return -1;
	}

	confp = &conf;

	return 0;
}

void
conf_free(void)
{
	memset(&conf, 0, sizeof(conf));
}
