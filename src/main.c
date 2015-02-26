#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

#include "decoder.h"
#include "serial.h"
#include "history.h"
#include "ws2300.h"

#define CSV_DATE	"%Y-%m-%dT%H:%M"

#define array_len(a)	(sizeof(a) / sizeof(a[0]))

/* BCD converters */
static const struct ws_type wst_temp = { WS_TEMP, "°C", 4, "temperature" };
static const struct ws_type wst_pressure = { WS_PRESSURE, "hPa", 5, "pressure" };
static const struct ws_type wst_humidity = { WS_HUMIDITY, "%", 2, "humidity" };
static const struct ws_type wst_rain = { WS_RAIN, "mm", 6, "rain" };
static const struct ws_type wst_speed = { WS_SPEED, "m/s", 3, "speed" };

/* Wind direction converter */
static const struct ws_type wst_wind_dir = { WS_WIND_DIR, "deg", 1, "wind direction, North=0 clockwise" };

/* Wind velocity converter */
static const struct ws_type wst_wind_speed = { WS_WIND_VELOCITY, "ms,d", 4, "wind speed and direction" };

/* Bin converters */
static const struct ws_type wst_int_s = { WS_INT_SEC, "s", 2, "time interval" };
static const struct ws_type wst_int_m = { WS_INT_MIN, "min", 3, "time interval" };
static const struct ws_type wst_rec_nbr = { WS_BIN_2NYB, NULL, 2, "record number" };

/* Date and time converters */
static const struct ws_type wst_date = { WS_DATE, NULL, 6, "yyyy-mm-dd" };
static const struct ws_type wst_tstamp = { WS_TIMESTAMP, NULL, 10, "yyyy-mm-dd hh:mm" };
static const struct ws_type wst_datetime = { WS_DATETIME, NULL, 11, "yyyy-mm-dd hh:mm" };
static const struct ws_type wst_time = { WS_TIME, NULL, 6, "hh:mm:ss" };

/* Text converters */
static const struct ws_type wst_conn = { WS_CONNECTION, NULL, 1, "cable, lost, wireless" };
static const struct ws_type wst_forecast = { 0, NULL, 1, "rainy, cloudy, sunny" };
static const struct ws_type wst_speed_unit = { 0, NULL, 1, "m/s, knots, beaufort, km/h, mph" };

/* ws23xx memory, ordered by address */
static const struct ws_measure mem_addr[] =
{
/*
	{ 0x006, "bz", conv_buzz, "buzzer" },
	{ 0x00f, "wsu", conv_spdu, "wind speed units" },
	{ 0x016, "lb", conv_lbck, "lcd backlight" },
	{ 0x019, "sss", conv_als2, "storm warn alarm set" },
	{ 0x019, "sts", conv_als0, "station time alarm set" },
	{ 0x01a, "phs", conv_als3, "pressure max alarm set" },
	{ 0x01a, "pls", conv_als2, "pressure min alarm set" },
	{ 0x01b, "oths", conv_als3, "out temp max alarm set" },
	{ 0x01b, "otls", conv_als2, "out temp min alarm set" },
	{ 0x01b, "iths", conv_als1, "in temp max alarm set" },
	{ 0x01b, "itls", conv_als0, "in temp min alarm set" },
	{ 0x01c, "dphs", conv_als3, "dew point max alarm set" },
	{ 0x01c, "dpls", conv_als2, "dew point min alarm set" },
	{ 0x01c, "wchs", conv_als1, "wind chill max alarm set" },
	{ 0x01c, "wcls", conv_als0, "wind chill min alarm set" },
	{ 0x01d, "ihhs", conv_als3, "in humidity max alarm set" },
	{ 0x01d, "ihls", conv_als2, "in humidity min alarm set" },
	{ 0x01d, "ohhs", conv_als1, "out humidity max alarm set" },
	{ 0x01d, "ohls", conv_als0, "out humidity min alarm set" },
	{ 0x01e, "rhhs", conv_als1, "rain 1h alarm set" },
	{ 0x01e, "rdhs", conv_als0, "rain 24h alarm set" },
	{ 0x01f, "wds", conv_als2, "wind direction alarm set" },
	{ 0x01f, "wshs", conv_als1, "wind speed max alarm set" },
	{ 0x01f, "wsls", conv_als0, "wind speed min alarm set" },
	{ 0x020, "siv", conv_ala2, "icon alarm active" },
	{ 0x020, "stv", conv_ala0, "station time alarm active" },
	{ 0x021, "phv", conv_ala3, "pressure max alarm active" },
	{ 0x021, "plv", conv_ala2, "pressure min alarm active" },
	{ 0x022, "othv", conv_ala3, "out temp max alarm active" },
	{ 0x022, "otlv", conv_ala2, "out temp min alarm active" },
	{ 0x022, "ithv", conv_ala1, "in temp max alarm active" },
	{ 0x022, "itlv", conv_ala0, "in temp min alarm active" },
	{ 0x023, "dphv", conv_ala3, "dew point max alarm active" },
	{ 0x023, "dplv", conv_ala2, "dew point min alarm active" },
	{ 0x023, "wchv", conv_ala1, "wind chill max alarm active" },
	{ 0x023, "wclv", conv_ala0, "wind chill min alarm active" },
	{ 0x024, "ihhv", conv_ala3, "in humidity max alarm active" },
	{ 0x024, "ihlv", conv_ala2, "in humidity min alarm active" },
	{ 0x024, "ohhv", conv_ala1, "out humidity max alarm active" },
	{ 0x024, "ohlv", conv_ala0, "out humidity min alarm active" },
	{ 0x025, "rhhv", conv_ala1, "rain 1h alarm active" },
	{ 0x025, "rdhv", conv_ala0, "rain 24h alarm active" },
	{ 0x026, "wdv", conv_ala2, "wind direction alarm active" },
	{ 0x026, "wshv", conv_ala1, "wind speed max alarm active" },
	{ 0x026, "wslv", conv_ala0, "wind speed min alarm active" },
	{ 0x027, None, conv_ala3, "pressure max alarm active alias" },
	{ 0x027, None, conv_ala2, "pressure min alarm active alias" },
	{ 0x028, None, conv_ala3, "out temp max alarm active alias" },
	{ 0x028, None, conv_ala2, "out temp min alarm active alias" },
	{ 0x028, None, conv_ala1, "in temp max alarm active alias" },
	{ 0x028, None, conv_ala0, "in temp min alarm active alias" },
	{ 0x029, None, conv_ala3, "dew point max alarm active alias" },
	{ 0x029, None, conv_ala2, "dew point min alarm active alias" },
	{ 0x029, None, conv_ala1, "wind chill max alarm active alias" },
	{ 0x029, None, conv_ala0, "wind chill min alarm active alias" },
	{ 0x02a, None, conv_ala3, "in humidity max alarm active alias" },
	{ 0x02a, None, conv_ala2, "in humidity min alarm active alias" },
	{ 0x02a, None, conv_ala1, "out humidity max alarm active alias" },
	{ 0x02a, None, conv_ala0, "out humidity min alarm active alias" },
	{ 0x02b, None, conv_ala1, "rain 1h alarm active alias" },
	{ 0x02b, None, conv_ala0, "rain 24h alarm active alias" },
	{ 0x02c, None, conv_ala2, "wind direction alarm active alias" },
	{ 0x02c, None, conv_ala2, "wind speed max alarm active alias" },
	{ 0x02c, None, conv_ala2, "wind speed min alarm active alias" }, */
	{ 0x200, "st", &wst_time, "station set time", "ct" },
	{ 0x23b, "sw", &wst_datetime, "station current date time" },
	{ 0x24d, "sd", &wst_date, "station set date", "cd" },
/*	{ 0x266, "lc", conv_lcon, "lcd contrast (ro)" },
	{ 0x26b, "for", conv_fore, "forecast" },
	{ 0x26c, "ten", conv_tend, "tendency" }, */
	{ 0x346, "it", &wst_temp, "in temp" },
	{ 0x34b, "itl", &wst_temp, "in temp min", "it" },
	{ 0x350, "ith", &wst_temp, "in temp max", "it" },
	{ 0x354, "itlw", &wst_tstamp, "in temp min when", "sw" },
	{ 0x35e, "ithw", &wst_tstamp, "in temp max when", "sw" },
	{ 0x369, "itla", &wst_temp, "in temp min alarm" },
	{ 0x36e, "itha", &wst_temp, "in temp max alarm" },
	{ 0x373, "ot", &wst_temp, "out temp" },
	{ 0x378, "otl", &wst_temp, "out temp min", "ot" },
	{ 0x37d, "oth", &wst_temp, "out temp max", "ot" },
	{ 0x381, "otlw", &wst_tstamp, "out temp min when", "sw" },
	{ 0x38b, "othw", &wst_tstamp, "out temp max when", "sw" },
	{ 0x396, "otla", &wst_temp, "out temp min alarm" },
	{ 0x39b, "otha", &wst_temp, "out temp max alarm" },
	{ 0x3a0, "wc", &wst_temp, "wind chill" },
	{ 0x3a5, "wcl", &wst_temp, "wind chill min", "wc" },
	{ 0x3aa, "wch", &wst_temp, "wind chill max", "wc" },
	{ 0x3ae, "wclw", &wst_tstamp, "wind chill min when", "sw" },
	{ 0x3b8, "wchw", &wst_tstamp, "wind chill max when", "sw" },
	{ 0x3c3, "wcla", &wst_temp, "wind chill min alarm" },
	{ 0x3c8, "wcha", &wst_temp, "wind chill max alarm" },
	{ 0x3ce, "dp", &wst_temp, "dew point" },
	{ 0x3d3, "dpl", &wst_temp, "dew point min", "dp" },
	{ 0x3d8, "dph", &wst_temp, "dew point max", "dp" },
	{ 0x3dc, "dplw", &wst_tstamp, "dew point min when", "sw" },
	{ 0x3e6, "dphw", &wst_tstamp, "dew point max when", "sw" },
	{ 0x3f1, "dpla", &wst_temp, "dew point min alarm" },
	{ 0x3f6, "dpha", &wst_temp, "dew point max alarm" },
	{ 0x3fb, "ih", &wst_humidity, "in humidity" },
	{ 0x3fd, "ihl", &wst_humidity, "in humidity min", "ih" },
	{ 0x3ff, "ihh", &wst_humidity, "in humidity max", "ih" },
	{ 0x401, "ihlw", &wst_tstamp, "in humidity min when", "sw" },
	{ 0x40b, "ihhw", &wst_tstamp, "in humidity max when", "sw" },
	{ 0x415, "ihla", &wst_humidity, "in humidity min alarm" },
	{ 0x417, "ihha", &wst_humidity, "in humidity max alarm" },
	{ 0x419, "oh", &wst_humidity, "out humidity" },
	{ 0x41b, "ohl", &wst_humidity, "out humidity min", "oh" },
	{ 0x41d, "ohh", &wst_humidity, "out humidity max", "oh" },
	{ 0x41f, "ohlw", &wst_tstamp, "out humidity min when", "sw" },
	{ 0x429, "ohhw", &wst_tstamp, "out humidity max when", "sw" },
	{ 0x433, "ohla", &wst_humidity, "out humidity min alarm" },
	{ 0x435, "ohha", &wst_humidity, "out humidity max alarm" },
	{ 0x497, "rd", &wst_rain, "rain 24h" },
	{ 0x49d, "rdh", &wst_rain, "rain 24h max", "rd" },
	{ 0x4a3, "rdhw", &wst_tstamp, "rain 24h max when", "sw" },
	{ 0x4ae, "rdha", &wst_rain, "rain 24h max alarm" },
	{ 0x4b4, "rh", &wst_rain, "rain 1h" },
	{ 0x4ba, "rhh", &wst_rain, "rain 1h max", "rh" },
	{ 0x4c0, "rhhw", &wst_tstamp, "rain 1h max when", "sw" },
	{ 0x4cb, "rhha", &wst_rain, "rain 1h max alarm" },
	{ 0x4d2, "rt", &wst_rain, "rain total", "0" },
	{ 0x4d8, "rtrw", &wst_tstamp, "rain total reset when", "sw" },
	{ 0x4ee, "wsl", &wst_speed, "wind speed min", "ws" },
	{ 0x4f4, "wsh", &wst_speed, "wind speed max", "ws" },
	{ 0x4f8, "wslw", &wst_tstamp, "wind speed min when", "sw" },
	{ 0x502, "wshw", &wst_tstamp, "wind speed max when", "sw" },
/*	{ 0x527, "wso", conv_wovr, "wind speed overflow" },
	{ 0x528, "wsv", conv_wvld, "wind speed validity" },*/
	{ 0x529, "wv", &wst_wind_speed, "wind velocity" },
	{ 0x529, "ws", &wst_speed, "wind speed" },
	{ 0x52c, "w0", &wst_wind_dir, "wind direction" },
	{ 0x52d, "w1", &wst_wind_dir, "wind direction 1" },
	{ 0x52e, "w2", &wst_wind_dir, "wind direction 2" },
	{ 0x52f, "w3", &wst_wind_dir, "wind direction 3" },
	{ 0x530, "w4", &wst_wind_dir, "wind direction 4" },
	{ 0x531, "w5", &wst_wind_dir, "wind direction 5" },
	{ 0x533, "wsla", &wst_speed, "wind speed min alarm" },
	{ 0x538, "wsha", &wst_speed, "wind speed max alarm" },
	{ 0x54d, "cn", &wst_conn, "connection type" },
	{ 0x54f, "cc", &wst_int_s, "connection time till connect" },
	{ 0x5d8, "pa", &wst_pressure, "pressure absolute" },
	{ 0x5e2, "pr", &wst_pressure, "pressure relative" },
	{ 0x5ec, "pc", &wst_pressure, "pressure correction" },
	{ 0x5f6, "pal", &wst_pressure, "pressure absolute min", "pa" },
	{ 0x600, "prl", &wst_pressure, "pressure relative min", "pr" },
	{ 0x60a, "pah", &wst_pressure, "pressure absolute max", "pa" },
	{ 0x614, "prh", &wst_pressure, "pressure relative max", "pr" },
	{ 0x61e, "plw", &wst_tstamp, "pressure min when", "sw" },
	{ 0x628, "phw", &wst_tstamp, "pressure max when", "sw" },
	{ 0x63c, "pla", &wst_pressure, "pressure min alarm" },
	{ 0x650, "pha", &wst_pressure, "pressure max alarm" },
	{ 0x6b2, "hi", &wst_int_m, "history interval" },
	{ 0x6b5, "hc", &wst_int_m, "history time till sample" },
	{ 0x6b8, "hw", &wst_tstamp, "history last sample when" },
	{ 0x6c2, "hp", &wst_rec_nbr, "history last record pointer", "0" },
	{ 0x6c4, "hn", &wst_rec_nbr, "history number of records", "0"}
};

/* ws23xx memory, ordered by id */
static const struct ws_measure *mem_id[array_len(mem_addr)];

static void
usage(FILE *out, int code)
{
	fprintf(out, "Usage: %s [-h] [-D] [-d ms] <command> [<args>]\n", PROGNAME);
	fprintf(out, "\n");
	fprintf(out, "Available %s commands:\n", PROGNAME);
	fprintf(out, "    help [-A]\n");
	fprintf(out, "    fetch [-s sep] device measure...\n");
	fprintf(out, "    history [-l cnt] [-s sep] device [file]\n");
	fprintf(out, "    hex [-x] device offset len\n");

	exit(code);
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

/**
 * Copy nybble data.
 *
 * The #nybcpy() function copies #nybble nybbles from area #src, starting at
 * #offset offset (nybbles offset), to memory area #dest.
 */
static void
nybcpy(uint8_t *dest, const uint8_t *src, uint16_t nnybble, size_t offset)
{
	src += offset / 2;

	if (offset & 0x1) {
		for (uint16_t i = 0; i < nnybble; i++) {
			int j = 1 + i;

			if (j & 0x1) {
				dest[i/2] = src[j/2] >> 4;
			} else {
				dest[i/2] |= src[j/2] << 4;
			}
		}
	} else {
		memcpy(dest, src, (nnybble + 1) / 2);
	}
}

/**
 * Compare two measures by id.
 */
static int
wsmncmp(const void *a, const void *b)
{
	const struct ws_measure *ma = * (struct ws_measure **) a;
	const struct ws_measure *mb = * (struct ws_measure **) b;

	return strcmp(ma->id, mb->id);
}

static void
print_measures(const struct ws_measure* mids[], const uint8_t *data,
		const off_t *off, size_t nel, const char *sep) {
	/* Print result */
	for (size_t i = 0; i < nel; i++) {
		uint8_t nyb_data[25];
		char nyb_str[128];

		const struct ws_measure *m = mids[i];
		const struct ws_type *t = m->type;

		nybcpy(nyb_data, data, t->nybble, off[i]);

		switch (t->id) {
		case WS_TEMP:
			ws_temp_str(nyb_data, nyb_str, sizeof(nyb_str), 0);
			break;

		case WS_PRESSURE:
			ws_pressure_str(nyb_data, nyb_str, sizeof(nyb_str), 0);
			break;

		case WS_DATETIME:
			ws_datetime_str(nyb_data, nyb_str, sizeof(nyb_str), 0);
			break;

		case WS_TIMESTAMP:
			ws_timestamp_str(nyb_data, nyb_str, sizeof(nyb_str), 0);
			break;

		case WS_HUMIDITY:
			ws_humidity_str(nyb_data, nyb_str, sizeof(nyb_str), 0);
			break;

		case WS_SPEED:
			ws_speed_str(nyb_data, nyb_str, sizeof(nyb_str), 0);
			break;

		case WS_INT_SEC:
			ws_interval_sec_str(nyb_data, nyb_str, sizeof(nyb_str), 0);
			break;

		case WS_INT_MIN:
			ws_interval_min_str(nyb_data, nyb_str, sizeof(nyb_str), 0);
			break;

		case WS_BIN_2NYB:
			ws_bin_2nyb_str(nyb_data, nyb_str, sizeof(nyb_str), 0);
			break;

		case WS_CONNECTION:
			ws_connection_str(nyb_data, nyb_str, sizeof(nyb_str), 0);
			break;

		default:
			snprintf(nyb_str, sizeof(nyb_str), "-");
			fprintf(stderr, "not yet supported");
			break;
		}

		if (sep == NULL) {
			if (t->units != NULL) {
				printf("%s = %s %s\n", m->desc, nyb_str, t->units);
			} else {
				printf("%s = %s\n", m->desc, nyb_str);
			}
		} else {
			if (i > 0) {
				printf("%s%s", sep, nyb_str);
			} else {
				printf("%s", nyb_str);
			}
		}
	}

	if (sep != NULL) {
		printf("\n");
	}
}

static int
read_measures(int fd, char * const ids[], int nel, const char *sep) {
	uint16_t addr[nel];					/* address */
	size_t nnybble[nel];				/* number of nybbles at address */
	uint8_t *buf[nel];					/* data */

	off_t off[nel];						/* nybble offset in buffer */
	const struct ws_measure *mids[nel];	/* measures */

	int sz = 0;
	int opt_sz = 0;

	/* Optimize I/O per area */
	int nbyte = 0;
	uint8_t data[1024];

	memset(mids, 0, sizeof(mids));

	for (size_t i = 0; i < array_len(mem_addr); i++) {
		const struct ws_measure *m = &mem_addr[i];
		const struct ws_type *t = m->type;

		for (int j = 0; j < nel; j++) {
			if (strcmp(ids[j], m->id) == 0) {
				int same_area = 0;

				if (opt_sz > 0) {
					/**
					 * Using the same area saves 6 bytes read, and 5 bytes written,
					 * so we may overlap here to save I/O.
					 */
					if (addr[opt_sz-1] + nnybble[opt_sz-1] + 11 >= m->addr) {
						same_area = 1;
					}
				}

				if (same_area) {
					nnybble[opt_sz-1] = m->addr + t->nybble - addr[opt_sz-1];
				} else {
					if (opt_sz > 0) {
						nbyte += (nnybble[opt_sz-1] + 1) / 2;
					}

					addr[opt_sz] = m->addr;
					nnybble[opt_sz] = t->nybble;
					buf[opt_sz] = data + nbyte;

					opt_sz++;
				}

				mids[j] = m;
				off[sz] = 2 * nbyte + (m->addr - addr[opt_sz-1]);

				sz++;
			}
		}
	}

	/* Check input measures */
	int errflg = 0;

	for (int j = 0; j < nel; j++) {
		if (mids[j] == NULL) {
			errflg++;
			fprintf(stderr, "Unknown measure: %s\n", ids[j]);
		}
	}

	if (errflg) {
		goto error;
	}

	/* Read */
	if (ws_read_batch(fd, addr, nnybble, opt_sz, buf) == -1) {
		return -1;
	}

	/* Print result */
	print_measures(mids, data, off, nel, sep);

	return 0;

error:
	return -1;
}

/**
 * Initialize static memory.
 *
 * The #init() method shall be called first to initialize static memory area,
 * like the #mem_id array (which is initialized in this function).
 */
static void
init() {
	size_t nel = array_len(mem_id);

	/* Sort mem_addr by id */
	for (size_t i = 0; i < nel; i++) {
		mem_id[i] = &mem_addr[i];
	}

	qsort(mem_id, nel, sizeof(mem_id[0]), wsmncmp);

	/* Set POSIX.2 behaviour for getopt() */
	setenv("POSIXLY_CORRECT", "1", 1);
}

static void
main_help(int argc, char* const argv[])
{
	int c;

	/* Default values */
	int addr_ordered = 0;

	/* Parse sub-command arguments */
	while ((c = getopt(argc, argv, "A")) != -1) {
		switch (c) {
		case 'A':
			addr_ordered = 1;
			break;

		default:
			usage_opt(stderr, c, 1);
			break;
		}
	}

	/* Process sub-command */
	size_t nel = array_len(mem_id);

	for (size_t i = 0; i < nel; i++) {
		const struct ws_measure *m = addr_ordered ? &mem_addr[i] : mem_id[i];
		const struct ws_type *t = m->type;

		if (t->units != NULL) {
			printf("%-5s %-30s 0x%3x:%-2d  %s, %s\n", m->id, m->desc, m->addr,
					t->nybble, t->units, t->desc);
		} else {
			printf("%-5s %-30s 0x%3x:%-2d  %s\n", m->id, m->desc, m->addr,
					t->nybble, t->desc);
		}
	}
}

static void
main_hex(int argc, char* const argv[]) {
	int c;

	/* Default values */
	const char *device = NULL;
	uint16_t addr;
	uint16_t nnybles;

	int disp_sz = 1;
	int print_hex = 0;

	/* Parse sub-command arguments */
	while ((c = getopt(argc, argv, "x")) != -1) {
		switch (c) {
		case 'x':
			disp_sz = 2;
			print_hex = 1;
			break;

		default:
			usage_opt(stderr, c, 1);
			break;
		}
	}

	if (argc - 3 < optind) {
		usage(stderr, 1);
	}

	device = argv[optind++];
	addr = strtol(argv[optind++], NULL, 16);
	nnybles = strtol(argv[optind++], NULL, 10);

	/* Process sub-command */
	int fd;
	uint8_t buf[nnybles];

	if ((fd = ws_open(device)) == -1) {
		exit(1);
	}

	if (ws_read_safe(fd, addr, nnybles, buf) == -1) {
		goto error;
	}

	for (uint16_t i = 0; i < nnybles; ) {
		printf("%.4x", addr + i);

		for (uint16_t j = 0; j < 16 && i < nnybles; j++) {
			uint8_t v;

			if (print_hex) {
				v = buf[i];
			} else {
				v = ws_nybble(buf, i);
			}

			printf(" %.*x", disp_sz, v);

			i += disp_sz;
		}

		printf("\n");
	}

	ws_close(fd);

	return;

error:
	ws_close(fd);
	exit(1);
}

static void
main_fetch(int argc, char* const argv[]) {
	int c;

	/* Default values */
	const char *sep = NULL;
	const char *device = NULL;

	/* Parse sub-command arguments */
	while ((c = getopt(argc, argv, "s:")) != -1) {
		switch (c) {
		case 's':
			sep = optarg;
			break;

		default:
			usage_opt(stderr, c, 1);
			break;
		}
	}

	if (argc - 2 < optind) {
		usage(stderr, 1);
	}

	device = argv[optind++];

	/* Process sub-command */
	int fd = ws_open(device);
	if (fd == -1) {
		exit(1);
	}
	read_measures(fd, argv + optind, argc - optind, sep);
	ws_close(fd);
}

static void
main_history(int argc, char* const argv[]) {
	int c;

	/* Default values */
	char sep = ',';
	size_t hist_count = 0;
	const char *device = NULL;
	const char *file = NULL;

	/* Parse sub-command arguments */
	while ((c = getopt(argc, argv, "l:s:")) != -1) {
		switch (c) {
		case 'l':
			hist_count = strtol(optarg, NULL, 10);
			break;
		case 's':
			sep = optarg[0];
			break;

		default:
			usage_opt(stderr, c, 1);
			break;
		}
	}

	if (optind < argc) {
		device = argv[optind++];

		if (optind < argc) {
			file = argv[optind++];
		}

		if (optind < argc) {
			usage(stderr, 1);
		}
	} else {
		usage(stderr, 1);
	}

	/* Loading data */
	int fd;

	if ((fd = ws_open(device)) == -1) {
		exit(1);
	}

	/* Parse output file */
	FILE *out = stdout;

	time_t last_record = 0;
	struct ws_history hbuf[WS_HISTORY_SIZE];

	if (file != NULL) {
		if ((out = fopen(file, "a+")) == NULL) {
			goto error;
		}

		/* Find last entry */
		char line[1024];

		while (fgets(line, sizeof(line), out) != NULL) {
			struct tm tm;
			char *p;

			p = strptime(line, CSV_DATE, &tm);

			if (p == NULL || *p != sep) {
				fprintf(stderr, "Fail to parse: %s\n", file);
				goto error;
			}

			last_record = mktime(&tm);
		}
	}

	ssize_t nel = ws_fetch_history(fd, hbuf, hist_count);

	/* Display output */
	char cbuf[32];

	for (int i = 0; i < nel; i++) {
		struct ws_history *h = &hbuf[i];

		struct tm tm;
		localtime_r(&h->tstamp, &tm);
		strftime(cbuf, sizeof(cbuf), CSV_DATE, &tm);

		fprintf(out, "%s%c%.2f%c%.2f%c%.1f%c%d%c%d%c%.2f%c%.1f%c%.1f%c%.2f%c%.2f\n",
				cbuf, sep, h->temp_in, sep, h->temp_out, sep,
				h->abs_pressure, sep, h->humidity_in, sep, h->humidity_out, sep,
				h->rain, sep, h->wind_speed, sep, h->wind_dir, sep,
				h->windchill, sep, h->dewpoint);
	}

	ws_close(fd);

	return;

error:
	ws_close(fd);
	exit(1);
}

int
main(int argc, char * const argv[])
{
	int c;

	const char *cmd = NULL;

	init();

	/* Parse arguments */
	while ((c = getopt(argc, argv, "hvd:")) != -1) {
		switch (c) {
		case 'h':
			usage(stdout, 0);
			break;
		case 'v':
			printf("%s %s\n", PROGNAME, VERSION);
			exit(0);
			break;

		case 'd':
			ws_io_delay = strtol(optarg, NULL, 10);
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
	if (strcmp("help", cmd) == 0) {
		main_help(argc, argv);
	} else if (strcmp("fetch", cmd) == 0) {
		main_fetch(argc, argv);
	} else if (strcmp("history", cmd) == 0) {
		main_history(argc, argv);
	} else if (strcmp("hex", cmd) == 0) {
		main_hex(argc, argv);
	}

	exit(0);
}
