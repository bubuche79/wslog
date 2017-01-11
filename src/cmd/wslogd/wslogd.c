/*
 * Weather station daemon (main file).
 */

#include <unistd.h>
#include <syslog.h>
#include <locale.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "libws/err.h"
#include "libws/conf.h"

#include "conf.h"
#include "daemon.h"
#include "worker.h"
#include "wslogd.h"

static struct ws_conf conf;
static int one_process_mode = 0;

const struct ws_conf *confp = &conf;

static void
usage(FILE *std, const char *bin)
{
	fprintf(std, "Usage: %s [-X] [-c conf_file]\n", bin);
}

static int
post_config(void)
{
	return 0;
}

static int
loop_init(void)
{
	int option;

	option = LOG_PID | (one_process_mode ? LOG_PERROR : 0);

	openlog("wslogd", option, conf.log_facility);
	(void) setlogmask(conf.log_mask);

	return 0;
}

static int
loop_reinit(const char *config_file)
{
	conf_free(&conf);
	if (conf_load(&conf, config_file) == -1) {
		goto error;
	}
	if (post_config() == -1) {
		goto error;
	}

	/* (Re)initialize */
	if (loop_init() == -1) {
		goto error;
	}

	return 0;

error:
	return -1;
}

int
main(int argc, char *argv[])
{
	int c;
	int ret;
	int halt;

	/* Default parameters */
	const char *conf_file = "/etc/wslogd.conf";

	(void) setlocale(LC_ALL, "");

	/* Parse command line */
	while ((c = getopt(argc, argv, "Xc:")) != -1) {
		switch (c) {
		case 'c':
			conf_file = optarg;
			if (conf_file[0] != '/') {
				die(1, "%s: Not an absolute file", conf_file);
			}
			break;
		case 'X':
			one_process_mode = 1;
			break;

		default:
			usage(stderr, argv[0]);
			return 2;
		}
	}

	if (argc != optind) {
		usage(stderr, argv[0]);
		return 2;
	}

	/* Required stuff before fork() */
	if (conf_load(&conf, conf_file) == -1) {
		return 1;
	}
	if (post_config() == -1) {
		return 1;
	}

	/* Detach, create new session */
	if (!one_process_mode) {
		if (daemon_create() == -1) {
			die(1, "daemon: %s\n", strerror(errno));
		}
	}

	ret = 1;
	if (loop_init() == -1) {
		goto exit;
	}

	halt = 0;
	do {
		if (worker_main(&halt) == -1) {
			goto exit;
		}

		/* Restart loop */
		if (!halt) {
			if (loop_reinit(conf_file) == -1) {
				goto exit;
			}
		}
	} while (!halt);

	ret = 0;

exit:
	if (ret == 0) {
		syslog(LOG_NOTICE, "Shutdown complete...");
	} else {
		syslog(LOG_EMERG, "Shutdown complete (abort)...");
	}

	closelog();

	return ret;
}
