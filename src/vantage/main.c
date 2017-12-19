#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "libws/vantage/vantage.h"

#define PROGNAME "vantage"

static void
usage(FILE *out, int status)
{
	fprintf(out, "Usage: " PROGNAME " [-h] [-V] [-D] [-d ms] <command> [<args>]\n");
	fprintf(out, "\n");
	fprintf(out, "Available " PROGNAME " commands:\n");
	fprintf(out, "    test\n");
	fprintf(out, "    ver\n");
	fprintf(out, "    nver\n");

	exit(status);
}

static void
usage_opt(FILE *out, int opt, int code)
{
	switch (opt) {
	case ':':
		fprintf(out, "Option -%c requires an operand\n", optopt);
		break;

	case '?':
		fprintf(out, "Unrecognized option: -%c\n", optopt);
		break;

	default:
		fprintf(out, "Unhandled option: -%c\n", opt);
		break;
	}

	exit(code);
}

static int
main_test(int fd, int argc, char* const argv[])
{
	int c;

	/* Parse sub-command arguments */
	while ((c = getopt(argc, argv, "")) != -1) {
		switch (c) {
		default:
			usage_opt(stderr, c, 1);
			break;
		}
	}

	if (argc != 0) {
		usage(stderr, 1);
	}

	/* Process sub-command */
	if (vantage_test(fd) == -1) {
		fprintf(stderr, "vantage_test: %s\n", strerror(errno));
		goto error;
	}

	return 0;

error:
	return 1;
}

int
main(int argc, char * const argv[])
{
	int c;

	const char *device = NULL;
	const char *cmd = NULL;

	/* Parse arguments */
	while ((c = getopt(argc, argv, "hVd:")) != -1) {
		switch (c) {
		case 'h':
			usage(stdout, 0);
			break;
		case 'V':
			printf(PROGNAME " (" PACKAGE ") " VERSION "\n");
			exit(0);
			break;

		case 'd':
			device = optarg;
			break;

		default:
			usage_opt(stderr, c, 1);
		}
	}

	if (optind == argc) {
		usage(stderr, 1);
	}

	cmd = argv[optind++];

	/* Process command */
	int fd;
	int status;

	if (device == NULL) {
		device = "/dev/ttyUSB0";
	}

	if ((fd = vantage_open(device)) == -1) {
		fprintf(stderr, "vantage_open %s: %s\n", device, strerror(errno));
		exit(1);
	}

	/* Testing commands */
	if (strcmp("test", cmd) == 0) {
		status = main_test(fd, argc, argv);
//	} else if (strcmp("ver", cmd) == 0) {
//		main_ver(argc, argv);
//	} else if (strcmp("nver", cmd) == 0) {
//		main_nver(argc, argv);
	} else {
		status = 1;
		fprintf(stderr, "%s: unknown command\n", cmd);
	}

	if (vantage_close(fd) == -1) {
		fprintf(stderr, "vantage_close: %s\n", strerror(errno));
		status = -1;
	}

	exit(status);
}
