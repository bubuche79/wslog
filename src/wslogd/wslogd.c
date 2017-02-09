/*
 * Weather station daemon (main file).
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
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

static int one_process_mode = 0;

int dry_run = 0;

static void
usage(int status, FILE *std, const char *bin)
{
	fprintf(std, "Usage: %s [-h] [-V] [-D] [-X] [-c conf_file]\n", bin);
	exit(status);
}

static int
loop_init(void)
{
	int option;

	option = LOG_PID | (one_process_mode ? LOG_PERROR : 0);

	openlog("wslogd", option, confp->log_facility);
	(void) setlogmask(confp->log_mask);

	return 0;
}

static int
loop_reinit(const char *config_file)
{
	conf_free();
	if (conf_load(config_file) == -1) {
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

	(void) setlocale(LC_ALL, "C");

	/* Parse command line */
	while ((c = getopt(argc, argv, "hVDXc:")) != -1) {
		switch (c) {
		case 'c':
			conf_file = optarg;
			if (conf_file[0] != '/') {
				die(1, "%s: Not an absolute file\n", conf_file);
			}
			break;
		case 'D':
			dry_run = 1;
			break;
		case 'X':
			one_process_mode = 1;
			break;
		case 'h':
			usage(0, stdout, argv[0]);
			break;
		case 'V':
			printf("%s (" PACKAGE ") " VERSION "\n", argv[0]);
			exit(0);
			break;
		default:
			usage(2, stderr, argv[0]);
			break;
		}
	}

	if (argc != optind) {
		usage(2, stderr, argv[0]);
	}

	/* Required stuff before fork() */
	if (conf_load(conf_file) == -1) {
		exit(1);
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

	exit(ret);
}
