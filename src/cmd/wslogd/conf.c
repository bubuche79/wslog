/*
 * Weather station configuration.
 */

#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>

#include "conf.h"
#include "libws/conf.h"


static void
conf_init(struct ws_conf *cfg)
{
	memset(cfg, 0, sizeof(*cfg));
}

static int
conf_load(void *p, const char *key, const char *value)
{
	struct ws_conf *cfg = p;

	errno = 0;

	if (!strcmp(key, "tty")) {
		cfg->tty = strdup(value);
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

	return (errno == 0) ? 0 : -1;
}

int
ws_conf_load(struct ws_conf *cfg, const char *path)
{
	int lineno;

	conf_init(cfg);

	if (conf_parse(path, &lineno, conf_load, cfg) == -1) {
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
