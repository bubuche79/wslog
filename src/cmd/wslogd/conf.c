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
strtouid(const char *str, uid_t *uid)
{
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
		if (s == 0) {
			errno = EINVAL;
		} else {
			errno = s;
		}
	} else {
		*uid = pwd.pw_uid;
	}

	free(buf);

	return 0;
}

static int
strtogid(const char *str, gid_t *gid)
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
conf_init(struct ws_conf *cfg)
{
	memset(cfg, 0, sizeof(*cfg));

	/* Daemon */
	cfg->uid = -1;
	cfg->gid = -1;
	cfg->pid_file = "/var/run/wslogd.pid";
	cfg->log_facility = LOG_LOCAL0;
	cfg->log_mask = LOG_UPTO(LOG_NOTICE);

	/* Wunderstation */

	return 0;
}

static int
conf_decode(void *p, const char *key, const char *value)
{
	struct ws_conf *cfg = p;

	errno = 0;

	if (!strcmp(key, "user")) {
		strtouid(value, &cfg->uid);
	} else if (!strcmp(key, "group")) {
		strtogid(value, &cfg->gid);
	} else if (!strcmp(key, "pid_file")) {
		cfg->pid_file = strdup(value);
	} else if (!strcmp(key, "tty")) {
		cfg->tty = strdup(value);
	} else if (!strncmp(key, "csv.", 4)) {
		if (!strcmp(key, "csv.disabled")) {
			strtoint(value, &cfg->csv.disabled);
		} else if (!strcmp(key, "csv.file")) {
			cfg->csv.file = strdup(value);
		} else if (!strcmp(key, "csv.freq")) {
			strtoint(value, &cfg->csv.freq);
		} else {
			errno = EINVAL;
		}
	} else if (!strncmp(key, "wunder.", 7)) {
		if (!strcmp(key, "wunder.disabled")) {
			strtoint(value, &cfg->wunder.disabled);
		} else if (!strcmp(key, "wunder.url")) {
			cfg->wunder.url = strdup(value);
		} else if (!strcmp(key, "wunder.user.id")) {
			cfg->wunder.user_id = strdup(value);
		} else if (!strcmp(key, "wunder.user.password")) {
			cfg->wunder.user_pwd = strdup(value);
		} else if (!strcmp(key, "wunder.freq")) {
			strtoint(value, &cfg->wunder.freq);
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

	if (conf_parse(path, &lineno, conf_decode, cfg) == -1) {
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
