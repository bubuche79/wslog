#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "libws/vantage/util.h"
#include "libws/vantage/vantage.h"

#define PROGNAME "vantage"

static void
usage(FILE *out, int status)
{
	fprintf(out, "Usage: " PROGNAME " [-h] [-d dev] <command> [<args>]\n");
	fprintf(out, "\n");
	fprintf(out, "Options:\n");
	fprintf(out, "   -h              display this\n");
	fprintf(out, "   -d device       TTY console device\n");
	fprintf(out, "\n");
	fprintf(out, "Available low-level " PROGNAME " commands:\n");
	fprintf(out, "   test            test connectivity\n");
	fprintf(out, "   wrd             display weather station type\n");
	fprintf(out, "   ver             display firmware date\n");
	fprintf(out, "   nver            display firmware version\n");
	fprintf(out, "   settime [time]  set time and date on console\n");
	fprintf(out, "   gettime         get current time and date\n");
	fprintf(out, "   setper min      set archive interval, in minutes\n");
	fprintf(out, "   lps [n]         display current weather\n");

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

static void
clropt()
{
	optind = 1;
	optarg = NULL;
}

static void
check_empty_opts(int argc, char* const argv[])
{
	int c;

	while ((c = getopt(argc, argv, "")) != -1) {
		switch (c) {
		default:
			usage_opt(stderr, c, 1);
			break;
		}
	}
}

static void
check_empty_cmdline(int argc, char* const argv[])
{
	check_empty_opts(argc, argv);

	if (optind != argc) {
		usage(stderr, 1);
	}
}

static unsigned int
strtoui(const char *s)
{
	unsigned int v;
	char *p;

	v = strtol(s, &p, 10);

	if (v == -1 || *p != 0 || v < 0) {
		v = -1;
		errno = EINVAL;
	}

	return v;
}

static int
main_test(int fd, int argc, char* const argv[])
{
	check_empty_cmdline(argc, argv);

	/* Process sub-command */
	if (vantage_test(fd) == -1) {
		fprintf(stderr, "vantage_test: %s\n", strerror(errno));
		goto error;
	}

	return 0;

error:
	return 1;
}

static int
main_wrd(int fd, int argc, char* const argv[])
{
	enum vantage_type type;

	check_empty_cmdline(argc, argv);

	/* Process sub-command */
	if (vantage_wrd(fd, &type) == -1) {
		fprintf(stderr, "vantage_wrd: %s\n", strerror(errno));
		goto error;
	}

	printf("%s\n", vantage_type_str(type));

	return 0;

error:
	return 1;
}

static int
main_ver(int fd, int argc, char* const argv[])
{
	char buf[16];

	check_empty_cmdline(argc, argv);

	/* Process sub-command */
	if (vantage_ver(fd, buf, sizeof(buf)) == -1) {
		fprintf(stderr, "vantage_ver: %s\n", strerror(errno));
		goto error;
	}

	printf("%s\n", buf);

	return 0;

error:
	return 1;
}

static int
main_nver(int fd, int argc, char* const argv[])
{
	char buf[16];

	check_empty_cmdline(argc, argv);

	/* Process sub-command */
	if (vantage_nver(fd, buf, sizeof(buf)) == -1) {
		fprintf(stderr, "vantage_nver: %s\n", strerror(errno));
		goto error;
	}

	printf("%s\n", buf);

	return 0;

error:
	return 1;
}

static int
main_settime(int fd, int argc, char* const argv[])
{
	time_t ref;

	check_empty_cmdline(argc, argv);

	if (optind == argc) {
		time(&ref);
	} else if (optind + 1 == argc) {
		struct tm tm;
		const char *timep = argv[optind++];

		if (strptime(timep, "%FT%T", &tm) == NULL) {
			fprintf(stderr, "strptime %s: %s\n", timep, strerror(errno));
			goto error;
		}

		tm.tm_isdst = -1;
	} else {
		usage(stderr, 1);
	}

	/* Process sub-command */
	if (vantage_settime(fd, ref) == -1) {
		fprintf(stderr, "vantage_settime: %s\n", strerror(errno));
		goto error;
	}

	return 0;

error:
	return 1;
}

static int
main_gettime(int fd, int argc, char* const argv[])
{
	time_t time;
	char buf[32];
	struct tm tm;

	check_empty_opts(argc, argv);

	/* Process sub-command */
	if (vantage_gettime(fd, &time) == -1) {
		fprintf(stderr, "vantage_gettime: %s\n", strerror(errno));
		goto error;
	}

	localtime_r(&time, &tm);
	strftime(buf, sizeof(buf), "%F %T", &tm);

	printf("%s\n", buf);

	return 0;

error:
	return 1;
}

static int
main_setper(int fd, int argc, char* const argv[])
{
	unsigned int min;
	const char *min_str;

	check_empty_opts(argc, argv);

	if (optind + 1 != argc) {
		usage(stderr, 1);
	}

	min_str = argv[optind++];

	/* Parse values */
	min = strtoui(min_str);

	if (min == -1) {
		fprintf(stderr, "strtoui %s: %s\n", min_str, strerror(errno));
		goto error;
	}

	/* Process sub-command */
	if (vantage_setper(fd, min) == -1) {
		fprintf(stderr, "vantage_setper: %s\n", strerror(errno));
		goto error;
	}

	return 0;

error:
	return 1;
}

static double
vantage_temp(int16_t f, int scale)
{
	long pow10[] = { 1, 10, 100, 1000 };

	return ((double) f / pow10[scale] - 32.0) * 5 / 9;
}

static int
main_lps(int fd, int argc, char* const argv[])
{
	struct vantage_loop lps;

	check_empty_opts(argc, argv);

	/* Process sub-command */
	if (vantage_lps(fd, LPS_LOOP2, &lps, 1) == -1) {
		fprintf(stderr, "vantage_lps: %s\n", strerror(errno));
		goto error;
	}

	printf("Temp: %.1f\n", vantage_temp(lps.temp, 1));
	printf("Humidity: %d\n", lps.humidity);
	printf("In temp: %.1f\n", vantage_temp(lps.in_temp, 1));

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
	while ((c = getopt(argc, argv, "hd:")) != -1) {
		switch (c) {
		case 'h':
			usage(stdout, 0);
			break;
		case 'd':
			device = optarg;
			break;
		default:
			usage_opt(stderr, c, 1);
		}
	}

	if (argc < optind + 1) {
		usage(stderr, 1);
	}

	cmd = argv[optind++];

	argc = argc - optind + 1;
	argv = argv + optind - 1;
	clropt();

	/* Process command */
	int fd;
	int status;

	if (device == NULL) {
		device = "/dev/ttyUSB0";
	}

	status = 1;

	/* Open device */
	if ((fd = vantage_open(device)) == -1) {
		fprintf(stderr, "vantage_open %s: %s\n", device, strerror(errno));
		exit(1);
	}

	if (vantage_wakeup(fd) == -1) {
		fprintf(stderr, "vantage_wakeup: %s\n", strerror(errno));
		goto exit;
	}

	/* Testing commands */
	if (strcmp("test", cmd) == 0) {
		status = main_test(fd, argc, argv);
	} else if (strcmp("wrd", cmd) == 0) {
		status = main_wrd(fd, argc, argv);
	} else if (strcmp("ver", cmd) == 0) {
		status = main_ver(fd, argc, argv);
	} else if (strcmp("nver", cmd) == 0) {
		status = main_nver(fd, argc, argv);
	} else if (strcmp("settime", cmd) == 0) {
		status = main_settime(fd, argc, argv);
	} else if (strcmp("gettime", cmd) == 0) {
		status = main_gettime(fd, argc, argv);
	} else if (strcmp("setper", cmd) == 0) {
		status = main_setper(fd, argc, argv);
	} else if (strcmp("lps", cmd) == 0) {
		status = main_lps(fd, argc, argv);
	} else {
		fprintf(stderr, "%s: unknown command\n", cmd);
	}

exit:
	if (vantage_close(fd) == -1) {
		fprintf(stderr, "vantage_close: %s\n", strerror(errno));
	}

	exit(status);
}
