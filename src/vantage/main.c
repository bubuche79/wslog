#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "libws/util.h"
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
	fprintf(out, "Available " PROGNAME " commands:\n");
	fprintf(out, "   info            display console info\n");
	fprintf(out, "\n");
	fprintf(out, "Available low-level " PROGNAME " commands:\n");
	fprintf(out, "   test            test connectivity\n");
	fprintf(out, "   wrd             display weather station type\n");
	fprintf(out, "   rxcheck         display console diagnostic report\n");
	fprintf(out, "   ver             display firmware date\n");
	fprintf(out, "   nver            display firmware version\n");
	fprintf(out, "   settime [time]  set time and date on console\n");
	fprintf(out, "   gettime         get current time and date\n");
	fprintf(out, "   setper min      set archive interval, in minutes\n");
	fprintf(out, "   lps [n]         display current weather\n");
	fprintf(out, "   dmp [n]         download records\n");
	fprintf(out, "   dmpaft time     download records after specified date and time\n");
	fprintf(out, "   getee file      dump 4K EEPROM content\n");
	fprintf(out, "   eebrd addr len  read binary data from EEPROM\n");

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

static time_t
strtotime(const char *s, int sec)
{
	struct tm tm;
	const char *fmt;

	if (sec) {
		fmt = "%Y-%m-%dT%H:%M:%S";
	} else {
		fmt = "%Y-%m-%dT%H:%M";
	}

	if (strptime(s, fmt, &tm) == NULL) {
		errno = EINVAL;
		goto error;
	}

	tm.tm_isdst = -1;

	return mktime(&tm);

error:
	return -1;
}

static double
vantage_float(int16_t v, int scale)
{
	double pow10[] = { 1.0, 10.0, 100.0, 1000.0 };

	return v / pow10[scale];
}

static double
vantage_temp(int16_t f, int scale)
{
	double pow10[] = { 1.0, 10.0, 100.0, 1000.0 };

	return (f / pow10[scale] - 32.0) * 5 / 9;
}

static double
vantage_pressure(int16_t fp, int scale)
{
	double pow10[] = { 1.0, 10.0, 100.0, 1000.0 };

	return (fp / pow10[scale]) / 0.02952998751;
}

static void
print_rxcheck(const char *pref, const struct vantage_rxck *ck)
{
	printf("%s%s: %ld\n", pref, "Packets received", ck->pkt_recv);
	printf("%s%s: %ld\n", pref, "Packets missed", ck->pkt_missed);
	printf("%s%s: %ld\n", pref, "Maximum packets received without error", ck->pkt_in_row);
	printf("%s%s: %ld\n", pref, "Resynchronizations", ck->resync);
	printf("%s%s: %ld\n", pref, "CRC errors", ck->crc_ko);
}

static void
print_lps(const struct vantage_loop *lps)
{
	printf("Temp: %.1f\n", vantage_temp(lps->temp, 1));
	printf("Humidity: %d\n", lps->humidity);
	printf("In temp: %.1f\n", vantage_temp(lps->in_temp, 1));
	printf("Pressure: %.1f\n", vantage_pressure(lps->barometer, 3));
}

static void
print_dmp(const struct vantage_dmp *dmp)
{
	printf("Timestamp: %s\n", ctime(&dmp->tstamp));
	printf("Temp: %.1f\n", vantage_temp(dmp->temp, 1));
	printf("Humidity: %d\n", dmp->humidity);
	printf("In temp: %.1f\n", vantage_temp(dmp->in_temp, 1));
	printf("Pressure: %.1f\n", vantage_pressure(dmp->barometer, 3));
}

static void
print_hex(uint16_t addr, void *buf, uint16_t len)
{
	uint16_t i, j;

	for (i = 0; i < len;) {
		printf("%.4hx", addr + i);

		for (j = 0; j < 16 && i < len; i++, j++) {
			printf(" %.2x", ((uint8_t *) buf)[i]);
		}

		printf("\n");
	}
}

static int
main_info(int fd, int argc, char* const argv[])
{
	enum vantage_type wrd;
	char ver[VER_SIZE];
	char nver[NVER_SIZE];
	time_t onboard_time;
	struct vantage_cfg cfg;
	struct vantage_rxck ck;
	char buf[32];

	check_empty_cmdline(argc, argv);

	/* Process sub-command */
	if (vantage_wrd(fd, &wrd) == -1) {
		fprintf(stderr, "vantage_wrd: %s\n", strerror(errno));
		goto error;
	}

	if (vantage_ver(fd, ver, sizeof(ver)) == -1) {
		fprintf(stderr, "vantage_ver: %s\n", strerror(errno));
		goto error;
	}
	if (vantage_nver(fd, nver, sizeof(nver)) == -1) {
		fprintf(stderr, "vantage_nver: %s\n", strerror(errno));
		goto error;
	}

	if (vantage_ee_cfg(fd, &cfg) == -1) {
		fprintf(stderr, "vantage_ee_cfg: %s\n", strerror(errno));
		goto error;
	}
	if (vantage_gettime(fd, &onboard_time) == -1) {
		fprintf(stderr, "vantage_gettime: %s\n", strerror(errno));
		goto error;
	}

	if (vantage_rxcheck(fd, &ck) == -1) {
		fprintf(stderr, "vantage_rxcheck: %s\n", strerror(errno));
		goto error;
	}

	localftime_r(buf, sizeof(buf), &onboard_time, "%F %T");

	/* Display */
	printf("Console:\n");
	printf("  Type: %s\n\n", vantage_type_str(wrd));

	printf("Console firmware:\n");
	printf("  Date: %s\n", ver);
	printf("  Version: %s\n\n", nver);

	printf("Console settings:\n");
	printf("  Archive interval: %d (minutes)\n", cfg.ar_period);
	printf("  Altitude: %hu (feet)\n", cfg.altitude);
	printf("  Wind cup size: %s\n", cfg.sb_wind_cup ? "large" : "small");
	printf("  Rain collector size: %s\n", (cfg.sb_rain_cup == 0 ? "0.01 in" : (cfg.sb_rain_cup == 1 ? "0.2 mm" : "0.1 mm")));
	printf("  Rain season start: %d\n", cfg.rain_start);
	printf("  Time (onboard): %s\n\n", buf);

	printf("Station info:\n");
	printf("  Latitude (onboard): %.1f°\n", vantage_float(cfg.latitude, 1));
	printf("  Longitude (onboard): %.1f°\n\n", vantage_float(cfg.longitude, 1));

	printf("Reception diagnostics:\n");
	print_rxcheck("  ", &ck);

	return 0;

error:
	return 1;
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
main_rxcheck(int fd, int argc, char* const argv[])
{
	struct vantage_rxck ck;

	check_empty_cmdline(argc, argv);

	/* Process sub-command */
	if (vantage_rxcheck(fd, &ck) == -1) {
		fprintf(stderr, "vantage_rxcheck: %s\n", strerror(errno));
		goto error;
	}

	print_rxcheck("", &ck);

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
		const char *timep = argv[optind++];

		if ((ref = strtotime(timep, 1)) == -1) {
			fprintf(stderr, "strtotime %s: %s\n", timep, strerror(errno));
			goto error;
		}
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

static int
main_lps(int fd, int argc, char* const argv[])
{
	struct vantage_loop lps[1];

	check_empty_opts(argc, argv);

	/* Process sub-command */
	if (vantage_lps(fd, LPS_LOOP2, lps, 1) == -1) {
		fprintf(stderr, "vantage_lps: %s\n", strerror(errno));
		goto error;
	}

	print_lps(&lps[0]);

	return 0;

error:
	return 1;
}

static int
main_dmp(int fd, int argc, char* const argv[])
{
	struct vantage_dmp dmp;

	check_empty_opts(argc, argv);

	/* Process sub-command */
	if (vantage_dmp(fd, &dmp, 1) == -1) {
		fprintf(stderr, "vantage_dmp: %s\n", strerror(errno));
		goto error;
	}

	print_dmp(&dmp);

	return 0;

error:
	return 1;
}

static int
main_dmpaft(int fd, int argc, char* const argv[])
{
	const char *s;
	time_t after;
	struct vantage_dmp dmp;

	check_empty_opts(argc, argv);

	if (optind + 1 != argc) {
		usage(stderr, 1);
	}

	s = argv[optind++];

	if ((after = strtotime(s, 0)) == -1) {
		fprintf(stderr, "strtotime %s: %s\n", s, strerror(errno));
		goto error;
	}

	/* Process sub-command */
	if (vantage_dmpaft(fd, &dmp, 1, after) == -1) {
		fprintf(stderr, "vantage_dmpaft: %s\n", strerror(errno));
		goto error;
	}

	print_dmp(&dmp);

	return 0;

error:
	return 1;
}

static int
main_getee(int fd, int argc, char* const argv[])
{
	const char *file;
	uint8_t buf[EEPROM_SIZE];

	check_empty_opts(argc, argv);

	if (optind + 1 != argc) {
		usage(stderr, 1);
	}

	file = argv[optind++];

	/* Process sub-command */
	if (vantage_getee(fd, buf, sizeof(buf)) == -1) {
		fprintf(stderr, "vantage_getee: %s\n", strerror(errno));
		goto error;
	}

	return ws_dump(file, buf, sizeof(buf));

error:
	return 1;
}

static int
main_eebrd(int fd, int argc, char* const argv[])
{
	uint16_t addr;
	size_t len;
	uint8_t buf[EEPROM_SIZE];

	check_empty_opts(argc, argv);

	if (optind + 2 != argc) {
		usage(stderr, 1);
	}

	addr = strtoll(argv[optind++], NULL, 16);
	len = strtoll(argv[optind++], NULL, 16);

	/* Process sub-command */
	if (vantage_eebrd(fd, addr, buf, len) == -1) {
		fprintf(stderr, "vantage_eebrd: %s\n", strerror(errno));
		goto error;
	}

	print_hex(addr, buf, len);

	return 0;

error:
	return 1;
}

static int
main_clrlog(int fd, int argc, char* const argv[])
{
	check_empty_cmdline(argc, argv);

	/* Process sub-command */
	if (vantage_clrlog(fd) == -1) {
		fprintf(stderr, "vantage_clrlog: %s\n", strerror(errno));
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

	/* Commands */
	if (strcmp("info", cmd) == 0) {
		status = main_info(fd, argc, argv);
	/* Low-level commands */
	} else if (strcmp("test", cmd) == 0) {
		status = main_test(fd, argc, argv);
	} else if (strcmp("wrd", cmd) == 0) {
		status = main_wrd(fd, argc, argv);
	} else if (strcmp("rxcheck", cmd) == 0) {
		status = main_rxcheck(fd, argc, argv);
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
	} else if (strcmp("dmp", cmd) == 0) {
		status = main_dmp(fd, argc, argv);
	} else if (strcmp("dmpaft", cmd) == 0) {
		status = main_dmpaft(fd, argc, argv);
	} else if (strcmp("getee", cmd) == 0) {
		status = main_getee(fd, argc, argv);
	} else if (strcmp("eebrd", cmd) == 0) {
		status = main_eebrd(fd, argc, argv);
	} else if (strcmp("clrlog", cmd) == 0) {
		status = main_clrlog(fd, argc, argv);
	} else {
		fprintf(stderr, "%s: unknown command\n", cmd);
	}

exit:
	if (vantage_close(fd) == -1) {
		fprintf(stderr, "vantage_close: %s\n", strerror(errno));
	}

	exit(status);
}
