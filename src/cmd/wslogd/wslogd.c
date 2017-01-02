/*
 * Weather station daemon (main file).
 */

#include <unistd.h>
#include <syslog.h>
#include <locale.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <curl/curl.h>

#include "libws/conf.h"

#include "conf.h"

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
	if (conf.log_facility == -1) {
		conf.log_facility = LOG_LOCAL0;
	}
	if (conf.log_mask == -1) {
		conf.log_mask = LOG_UPTO(LOG_NOTICE);
	}

	return 0;
}

static int
loop_init(void)
{
	openlog("wslogd", LOG_PID, conf.log_facility);
	(void) setlogmask(conf.log_mask);

	return 0;
}

static int
loop_reinit(const char *config_file)
{
	/* (Re)open configuration */
	if (ws_conf_load(&conf, config_file) == -1) {
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
	pid_t pid;

	/* Default parameters */
	char *config_file = "/etc/ws.conf";

	(void) setlocale(LC_ALL, "");

//	/* Ensure that fds 0, 1 and 2 are opened or directed to /dev/null */
//	if (sanitise_stdfd() == -1) {
//		return 1;
//	}

	/* Parse command line */
	while ((c = getopt(argc, argv, "Xc:")) != -1) {
		switch (c) {
		case 'c':
			config_file = optarg;
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
	if (ws_conf_load(&conf, config_file) == -1) {
		return 1;
	}
	if (post_config() == -1) {
		return 1;
	}

	curl_global_init(CURL_GLOBAL_DEFAULT);

	/* Detach, create new session */
	if (!one_process_mode) {
		pid = fork();
		if (pid == -1) {
			(void) fprintf(stderr, "fork: %s\n", strerror(errno));
			return 1;
		} else if (pid > 0) {
			return 0;
		}

		if (setsid() == -1) {
			(void) fprintf(stderr, "setsid: %s\n", strerror(errno));
			return 1;
		}
	}

	ret = 1;
	if (loop_init() == -1) {
		goto exit;
	}

	halt = 0;
	do {
//		if (prefork_main(&halt) == -1) {
//			goto exit;
//		}

		/* Restart loop */
		if (!halt) {
			if (loop_reinit(config_file) == -1) {
				goto exit;
			}
		}
	} while (!halt);

	ret = 0;

exit:
	if (ret == 0) {
		syslog(LOG_NOTICE, "Shutting down...");
	} else {
		syslog(LOG_EMERG, "Shutting down (abort)...");
	}

	curl_global_cleanup();
	closelog();
	return ret;
}
