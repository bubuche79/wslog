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
#include <errno.h>

#include "libws/defs.h"
#include "libws/conf.h"

#include "conf.h"

struct code
{
	const char *c_name;
	int c_val;
};

static struct code log_levels[] =
{
	{ "alert", LOG_ALERT },
	{ "crit", LOG_CRIT },
	{ "debug", LOG_DEBUG },
	{ "emerg", LOG_EMERG },
	{ "err", LOG_ERR },
	{ "info", LOG_INFO },
	{ "notice", LOG_NOTICE },
	{ "warning", LOG_WARNING }
};

static struct code log_facilities[] =
{
	{ "auth", LOG_AUTH },
	{ "authpriv", LOG_AUTHPRIV },
	{ "cron", LOG_CRON },
	{ "daemon", LOG_DAEMON },
	{ "ftp", LOG_FTP },
	{ "kern", LOG_KERN },
	{ "lpr", LOG_LPR },
	{ "mail", LOG_MAIL },
	{ "news", LOG_NEWS },
	{ "syslog", LOG_SYSLOG },
	{ "user", LOG_USER },
	{ "uucp", LOG_UUCP },
	{ "local0", LOG_LOCAL0 },
	{ "local1", LOG_LOCAL1 },
	{ "local2", LOG_LOCAL2 },
	{ "local3", LOG_LOCAL3 },
	{ "local4", LOG_LOCAL4 },
	{ "local5", LOG_LOCAL5 },
	{ "local6", LOG_LOCAL6 },
	{ "local7", LOG_LOCAL7 }
};

static struct ws_conf conf;

struct ws_conf *confp = &conf;

static int
code_search(const struct code *c, size_t nel, const char *name, int *code)
{
	int i, ret;

	for (i = 0; i < nel && strcmp(name, c[i].c_name); i++) {
		/* Continue */
	}

	if (i < nel) {
		*code = c[i].c_val;
		ret = 0;
	} else {
		errno = EINVAL;
		ret = -1;
	}

	return ret;
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
#if HAVE_VIRT
	if (!strcmp(str, "virt")) {
		*driver = VIRT;
	}
#endif

	if (*driver == UNUSED) {
		ret = -1;
		errno = EINVAL;
	}

	return ret;
}

int
ws_getlevel(const char *str, int *level)
{
	size_t nel = array_size(log_levels);

	return code_search(log_levels, nel, str, level);
}

int
ws_getfacility(const char *str, int *facility)
{
	size_t nel = array_size(log_facilities);

	return code_search(log_facilities, nel, str, facility);
}

static int
conf_init(struct ws_conf *cfg)
{
	memset(cfg, 0, sizeof(*cfg));

	/* Daemon */
	cfg->log_facility = LOG_USER;
	cfg->log_level = LOG_NOTICE;

	cfg->station.driver = UNUSED;

	/* Default driver */
	cfg->driver.freq = 0;
#if HAVE_VANTAGE
	cfg->driver.vantage.tty = "/dev/ttyUSB0";
#endif
#if HAVE_WS23XX
	cfg->driver.ws23xx.tty = "/dev/ttyUSB0";
#endif
#if HAVE_VIRT
	cfg->driver.virt.hw_archive = 1;
	cfg->driver.virt.io_delay = 100;
#endif

	/* Archive */
	cfg->archive.freq = 0;
	cfg->archive.delay = 15;

	/* SQLite */
	cfg->archive.sqlite.enabled = 1;
	cfg->archive.sqlite.db = WS_CONF_SQLITE_DB;

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

	if (!strcmp(key, "log_facility")) {
		ws_getfacility(value, &cfg->log_facility);
	} else if (!strcmp(key, "log_level")) {
		ws_getlevel(value, &cfg->log_level);
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
#if HAVE_VIRT
		} else if (!strcmp(key, "driver.virt.hw_archive")) {
			ws_getbool(value, &cfg->driver.virt.hw_archive);
		} else if (!strcmp(key, "driver.virt.io_delay")) {
			ws_getlong(value, &cfg->driver.virt.io_delay);
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
