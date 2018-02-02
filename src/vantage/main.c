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

#define PROGNAME 	"vantage"
#define RECORDS_COUNT	2560		/* Number of archive records */
#define DMPLEN		64		/* Size of DMP buffer */

static struct vantage_cfg cfg;

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
	fprintf(out, "   clrlog          clear archive data\n");
	fprintf(out, "   clrhighs [-dmy] clear daily, monthly or yearly high values\n");
	fprintf(out, "   clrlows [-dmy]  clear daily, monthly or yearly low values\n");

	exit(status);
}

static void
usage_opt(FILE *out, int opt, int status)
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

	exit(status);
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
		tm.tm_sec = 0;
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
print_lps(const struct vantage_loop *l)
{
	char ss_time[20];

	localftime_r(ss_time, sizeof(ss_time), &l->storm_start, "%F %T");

	printf("Temperature: %.1f°C\n", vantage_temp(l->temp, 1));
	printf("Humidity: %hhu%%\n", l->humidity);
	printf("Bar trend: %hhd\n", l->bar_trend);
	printf("Barometer: %.1fhPa\n", vantage_pressure(l->barometer, 3));
	if (l->wind_dir > 0) {
		printf("Wind speed: %.1fm/s\n", vantage_speed(l->wind_speed, 0));
		printf("Wind direction: %d°\n", l->wind_dir);
	}
	printf("2-min average wind speed: %.1fm/s\n", vantage_speed(l->wind_avg_2m, 1));
	printf("10-min average wind speed: %.1fm/s\n", vantage_speed(l->wind_avg_10m, 1));
	printf("10-min high wind speed: %.1fm/s\n", vantage_speed(l->wind_hi_10m_speed, 0));
	printf("Direction of 10-min high wind speed: %d°\n", l->wind_hi_10m_dir);
	printf("Dew point: %.0f°C\n", vantage_temp(l->dew_point, 0));
	printf("Heat index: %.0f°C\n", vantage_temp(l->heat_index, 0));
	printf("Wind chill: %.0f°C\n", vantage_temp(l->wind_chill, 0));
	if (l->thsw_idx != UINT8_MAX) {
		printf("THSW index: %.0f°C\n", vantage_temp(l->thsw_idx, 0));
	}
	if (l->uv_idx != UINT8_MAX) {
		printf("UV index: %hhd\n", l->uv_idx);
	}
	if (l->solar_rad != INT16_MAX) {
		printf("Solar radiation: %hdw/m²\n", l->solar_rad);
	}
	printf("Rain rate: %1.fmm/hour\n", vantage_rain(l->rain_rate, cfg.sb.rain_cup));
	printf("Storm rate: %1.fmm/hour\n", vantage_rain(l->storm_rain, cfg.sb.rain_cup));
	printf("Start date of current storm: %s\n", ss_time);
	printf("Daily rain: %.1fmm\n", vantage_rain(l->daily_rain, cfg.sb.rain_cup));
	printf("Last 15-min rain: %.1fmm\n", vantage_rain(l->last_15m_rain, cfg.sb.rain_cup));
	printf("Last hour rain: %.1fmm\n", vantage_rain(l->last_1h_rain, cfg.sb.rain_cup));
	printf("Last 24-hour rain: %.1fmm\n", vantage_rain(l->last_24h_rain, cfg.sb.rain_cup));
	if (l->daily_et != 0) {
		printf("Daily ET: %.1fmm\n", vantage_meter(l->daily_et, 3) / 1000.0);
	}
	printf("Inside temperature: %.1f°C\n", vantage_temp(l->in_temp, 1));
	printf("Inside humidity: %hhu%%\n", l->in_humidity);
	printf("Barometric reduction method: %hhd\n", l->barometer_algo);
	printf("User-entered barometric offset: %.1fhPa\n", vantage_pressure(l->barometer_off, 3));
	printf("Barometric calibration number: %.1fhPa\n", vantage_pressure(l->barometer_cal, 3));
	printf("Barometric sensor raw reading: %.1fhPa\n", vantage_pressure(l->barometer_raw, 3));
	printf("Absolute barometric pressure: %.1fhPa\n", vantage_pressure(l->barometer_abs, 3));
	printf("Altimeter setting: %.1fhPa\n", vantage_pressure(l->altimeter_opts, 3));
}

static void
print_dmp(const struct vantage_dmp *d)
{
	char ftime[20];

	localftime_r(ftime, sizeof(ftime), &d->time, "%F %T");

	printf("Timestamp: %s\n", ftime);
	printf("Temperature: %.1f°C\n", vantage_temp(d->temp, 1));
	printf("High temperature: %.1f°C\n", vantage_temp(d->hi_temp, 1));
	printf("Low temperature: %.1f°C\n", vantage_temp(d->lo_temp, 1));
	printf("Humidity: %hhu%%\n", d->humidity);
	printf("Rain fall: %1.fmm\n", vantage_rain(d->rain, cfg.sb.rain_cup));
	printf("High rain rate: %1.fmm/hour\n", vantage_rain(d->hi_rain_rate, cfg.sb.rain_cup));
	printf("Barometer: %.1fhPa\n", vantage_pressure(d->barometer, 3));
	if (d->solar_rad != INT16_MAX) {
		printf("Solar radiation: %hhdw/m²\n", d->solar_rad);
	}
	printf("Number of wind samples: %hhu\n", d->wind_samples);
	printf("Average wind speed: %.1fm/s\n", vantage_speed(d->avg_wind_speed, 0));
	printf("Main wind direction: %s\n", vantage_dir(d->main_wind_dir));
	printf("High wind speed: %.1fm/s\n", vantage_speed(d->hi_wind_speed, 0));
	printf("Direction of high wind speed: %s\n", vantage_dir(d->hi_wind_dir));
	if (d->avg_uv) {
		printf("Average UV: %.1f\n", vantage_val(d->avg_uv, 1));
	}
	if (d->et) {
		printf("1-hour ET: %.3fmm\n", vantage_meter(d->et, 3) / 1000);
	}
	printf("Inside temperature: %.1f°C\n", vantage_temp(d->in_temp, 1));
	printf("Inside humidity: %hhu%%\n", d->in_humidity);
}

static void
print_dmp_all(const struct vantage_dmp *d, size_t nel)
{
	size_t i;

	for (i = 0; i < nel; i++) {
		if (i > 0) {
			printf("\n");
		}

		print_dmp(&d[i]);
	}
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
	char buf[20];

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
	printf("  Wind cup size: %s\n", cfg.sb.wind_cup ? "large" : "small");
	printf("  Rain collector size: %s\n", (cfg.sb.rain_cup == 0 ? "0.01 in" : (cfg.sb.rain_cup == 1 ? "0.2 mm" : "0.1 mm")));
	printf("  Rain season start: %d\n", cfg.rain_start);
	printf("  Time (onboard): %s\n", buf);
	printf("  Temperature logging: %s\n\n", (cfg.log_avg ? "end" : "average"));

	printf("Station info:\n");
	printf("  Latitude (onboard): %.1f°\n", vantage_val(cfg.latitude, 1));
	printf("  Longitude (onboard): %.1f°\n\n", vantage_val(cfg.longitude, 1));

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
	if (vantage_ee_cfg(fd, &cfg) == -1) {
		fprintf(stderr, "vantage_ee_cfg: %s\n", strerror(errno));
		goto error;
	}
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
	size_t sz, buflen;
	struct vantage_dmp *buf;

	check_empty_opts(argc, argv);

	buflen = RECORDS_COUNT;
	buf = NULL;

	if ((buf = malloc(RECORDS_COUNT * sizeof(*buf))) == NULL) {
		fprintf(stderr, "malloc: %s\n", strerror(errno));
		goto error;
	}

	/* Process sub-command */
	if (vantage_ee_cfg(fd, &cfg) == -1) {
		fprintf(stderr, "vantage_ee_cfg: %s\n", strerror(errno));
		goto error;
	}
	if ((sz = vantage_dmp(fd, buf, buflen)) == -1) {
		fprintf(stderr, "vantage_dmp: %s\n", strerror(errno));
		goto error;
	}

	print_dmp_all(buf, sz);

	free(buf);

	return 0;

error:
	if (buf != NULL) {
		free(buf);
	}
	return 1;
}

static int
main_dmpaft(int fd, int argc, char* const argv[])
{
	ssize_t sz;
	time_t after;
	const char *s;
	struct vantage_dmp dmp[DMPLEN];

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
	if (vantage_ee_cfg(fd, &cfg) == -1) {
		fprintf(stderr, "vantage_ee_cfg: %s\n", strerror(errno));
		goto error;
	}

	do {
		if ((sz = vantage_dmpaft(fd, dmp, DMPLEN, after)) == -1) {
			fprintf(stderr, "vantage_dmpaft: %s\n", strerror(errno));
			goto error;
		}

		print_dmp_all(dmp, sz);

		/* Next start point */
		after = dmp[sz - 1].time;
	} while (sz == DMPLEN);

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

	if (ws_write_all(file, buf, sizeof(buf)) == -1) {
		fprintf(stderr, "ws_write_all: %s\n", strerror(errno));
		goto error;
	}

	return 0;

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

static int
main_clrhl(int fd, int high, int argc, char* const argv[])
{
	int i, c;
	int clr[] = { 0, 0, 0 };

	/* Parse sub-command */
	while ((c = getopt(argc, argv, "dmy")) != -1) {
		switch (c) {
		case 'd':
			clr[0] = 1;
			break;
		case 'm':
			clr[1] = 1;
			break;
		case 'y':
			clr[2] = 1;
			break;
		default:
			usage_opt(stderr, c, 1);
			break;
		}
	}

	if (optind != argc) {
		usage(stderr, 1);
	}

	/* Process sub-command */
	for (i = 0; i < 3; i++) {
		if (clr[i] == 0) {
			continue;
		}

		if (high) {
			if (vantage_clrhighs(fd, i) == -1) {
				fprintf(stderr, "vantage_clrhighs %d: %s\n", i, strerror(errno));
				goto error;
			}
		} else {
			if (vantage_clrlows(fd, i) == -1) {
				fprintf(stderr, "vantage_clrlows %d: %s\n", i, strerror(errno));
				goto error;
			}
		}
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
	} else if (strcmp("clrhighs", cmd) == 0) {
		status = main_clrhl(fd, 1, argc, argv);
	} else if (strcmp("clrlows", cmd) == 0) {
		status = main_clrhl(fd, 0, argc, argv);
	} else {
		fprintf(stderr, "%s: unknown command\n", cmd);
	}

exit:
	if (vantage_close(fd) == -1) {
		fprintf(stderr, "vantage_close: %s\n", strerror(errno));
	}

	exit(status);
}
