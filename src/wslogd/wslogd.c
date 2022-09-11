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
#include <err.h>

#include "libws/conf.h"

#include "curl.h"
#include "conf.h"
#include "worker.h"

#define PROGNAME	"wslogd"

static int archive_freq = -1;

int dry_run = 0;

static void
usage(FILE *std, int status)
{
	fprintf(std, "Usage: " PROGNAME " [-h] [-V] [-D] [-c config] [-i freq]\n");

	exit(status);
}

static void
post_config(void)
{
	if (archive_freq != -1) {
		confp->archive.freq = archive_freq;
	}
}

static int
loop_init(void)
{
	openlog(PROGNAME, LOG_PID, confp->log_facility);
	(void) setlogmask(LOG_UPTO(confp->log_level));

	if (dry_run) {
		syslog(LOG_WARNING, "Dry run mode");
	}

	/* Need libcurl */
	if (confp->wunder.enabled || confp->stat_ic.enabled) {
		CURLcode code = curl_global_init(CURL_GLOBAL_DEFAULT);
		if (code != CURLE_OK) {
			curl_log("curl_global_init", code);
			goto error;
		}
	}

	return 0;

error:
	return -1;
}

static int
loop_reinit(const char *config_file)
{
	conf_free();
	if (conf_load(config_file) == -1) {
		goto error;
	}

	post_config();

	/* (Re)initialize */
	if (loop_init() == -1) {
		goto error;
	}

	return 0;

error:
	return -1;
}

static void
loop_destroy()
{
	if (confp->wunder.enabled || confp->stat_ic.enabled) {
		curl_global_cleanup();
	}

	closelog();
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
	while ((c = getopt(argc, argv, "hVDc:i:")) != -1) {
		switch (c) {
		case 'c':
			conf_file = optarg;
			break;
		case 'i':
			archive_freq = atoi(optarg);
			break;
		case 'D':
			dry_run = 1;
			break;
		case 'h':
			usage(stdout, 0);
			break;
		case 'V':
			printf(PROGNAME " (" PACKAGE ") " VERSION "\n");
			exit(0);
			break;
		default:
			usage(stderr, 2);
			break;
		}
	}

	if (argc != optind) {
		usage(stderr, 2);
	}

	/* Required stuff before fork() */
	if (conf_load(conf_file) == -1) {
		exit(1);
	}

	post_config();

	/* Startup */
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
		syslog(LOG_NOTICE, "Shutdown complete");
	} else {
		syslog(LOG_ERR, "Shutdown complete (abort)");
	}

	loop_destroy();

	exit(ret);
}
