#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

#include "serial.h"
#include "convert.h"
#include "ws2300.h"

#define PROGNAME	"ws2300"

#define array_sz(a)	(sizeof(a) / sizeof(a[0]))

static struct ws_measure mem_map[] =
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
	{ 0x200, "st", WS_TIME, "station set time", "ct" },
	{ 0x23b, "sw", WS_DATETIME, "station current date time" },
	{ 0x24d, "sd", WS_DATE, "station set date", "cd" },
/*	{ 0x266, "lc", conv_lcon, "lcd contrast (ro)" },
	{ 0x26b, "for", conv_fore, "forecast" },
	{ 0x26c, "ten", conv_tend, "tendency" }, */
	{ 0x346, "it", WS_TEMP, "in temp" },
	{ 0x34b, "itl", WS_TEMP, "in temp min", "it" },
	{ 0x350, "ith", WS_TEMP, "in temp max", "it" },
	{ 0x354, "itlw", WS_TIMESTAMP, "in temp min when", "sw" },
	{ 0x35e, "ithw", WS_TIMESTAMP, "in temp max when", "sw" },
	{ 0x369, "itla", WS_TEMP, "in temp min alarm" },
	{ 0x36e, "itha", WS_TEMP, "in temp max alarm" },
	{ 0x373, "ot", WS_TEMP, "out temp" },
	{ 0x378, "otl", WS_TEMP, "out temp min", "ot" },
	{ 0x37d, "oth", WS_TEMP, "out temp max", "ot" },
	{ 0x381, "otlw", WS_TIMESTAMP, "out temp min when", "sw" },
	{ 0x38b, "othw", WS_TIMESTAMP, "out temp max when", "sw" },
	{ 0x396, "otla", WS_TEMP, "out temp min alarm" },
	{ 0x39b, "otha", WS_TEMP, "out temp max alarm" },
	{ 0x3a0, "wc", WS_TEMP, "wind chill" },
	{ 0x3a5, "wcl", WS_TEMP, "wind chill min", "wc" },
	{ 0x3aa, "wch", WS_TEMP, "wind chill max", "wc" },
	{ 0x3ae, "wclw", WS_TIMESTAMP, "wind chill min when", "sw" },
	{ 0x3b8, "wchw", WS_TIMESTAMP, "wind chill max when", "sw" },
	{ 0x3c3, "wcla", WS_TEMP, "wind chill min alarm" },
	{ 0x3c8, "wcha", WS_TEMP, "wind chill max alarm" },
	{ 0x3ce, "dp", WS_TEMP, "dew point" },
	{ 0x3d3, "dpl", WS_TEMP, "dew point min", "dp" },
	{ 0x3d8, "dph", WS_TEMP, "dew point max", "dp" },
	{ 0x3dc, "dplw", WS_TIMESTAMP, "dew point min when", "sw" },
	{ 0x3e6, "dphw", WS_TIMESTAMP, "dew point max when", "sw" },
	{ 0x3f1, "dpla", WS_TEMP, "dew point min alarm" },
	{ 0x3f6, "dpha", WS_TEMP, "dew point max alarm" },
	{ 0x3fb, "ih", WS_HUMIDITY, "in humidity" },
	{ 0x3fd, "ihl", WS_HUMIDITY, "in humidity min", "ih" },
	{ 0x3ff, "ihh", WS_HUMIDITY, "in humidity max", "ih" },
	{ 0x401, "ihlw", WS_TIMESTAMP, "in humidity min when", "sw" },
	{ 0x40b, "ihhw", WS_TIMESTAMP, "in humidity max when", "sw" },
	{ 0x415, "ihla", WS_HUMIDITY, "in humidity min alarm" },
	{ 0x417, "ihha", WS_HUMIDITY, "in humidity max alarm" },
	{ 0x419, "oh", WS_HUMIDITY, "out humidity" },
	{ 0x41b, "ohl", WS_HUMIDITY, "out humidity min", "oh" },
	{ 0x41d, "ohh", WS_HUMIDITY, "out humidity max", "oh" },
	{ 0x41f, "ohlw", WS_TIMESTAMP, "out humidity min when", "sw" },
	{ 0x429, "ohhw", WS_TIMESTAMP, "out humidity max when", "sw" },
	{ 0x433, "ohla", WS_HUMIDITY, "out humidity min alarm" },
	{ 0x435, "ohha", WS_HUMIDITY, "out humidity max alarm" },
	{ 0x497, "rd", WS_RAIN, "rain 24h" },
	{ 0x49d, "rdh", WS_RAIN, "rain 24h max", "rd" },
	{ 0x4a3, "rdhw", WS_TIMESTAMP, "rain 24h max when", "sw" },
	{ 0x4ae, "rdha", WS_RAIN, "rain 24h max alarm" },
	{ 0x4b4, "rh", WS_RAIN, "rain 1h" },
	{ 0x4ba, "rhh", WS_RAIN, "rain 1h max", "rh" },
	{ 0x4c0, "rhhw", WS_TIMESTAMP, "rain 1h max when", "sw" },
	{ 0x4cb, "rhha", WS_RAIN, "rain 1h max alarm" },
	{ 0x4d2, "rt", WS_RAIN, "rain total", "0" },
	{ 0x4d8, "rtrw", WS_TIMESTAMP, "rain total reset when", "sw" },
	{ 0x4ee, "wsl", WS_SPEED, "wind speed min", "ws" },
	{ 0x4f4, "wsh", WS_SPEED, "wind speed max", "ws" },
	{ 0x4f8, "wslw", WS_TIMESTAMP, "wind speed min when", "sw" },
	{ 0x502, "wshw", WS_TIMESTAMP, "wind speed max when", "sw" },
/*	{ 0x527, "wso", conv_wovr, "wind speed overflow" },
	{ 0x528, "wsv", conv_wvld, "wind speed validity" },*/
	{ 0x529, "wv", WS_WIND_VELOCITY, "wind velocity" },
	{ 0x529, "ws", WS_SPEED, "wind speed" },
	{ 0x52c, "w0", WS_WIND_DIR, "wind direction" },
	{ 0x52d, "w1", WS_WIND_DIR, "wind direction 1" },
	{ 0x52e, "w2", WS_WIND_DIR, "wind direction 2" },
	{ 0x52f, "w3", WS_WIND_DIR, "wind direction 3" },
	{ 0x530, "w4", WS_WIND_DIR, "wind direction 4" },
	{ 0x531, "w5", WS_WIND_DIR, "wind direction 5" },
	{ 0x533, "wsla", WS_SPEED, "wind speed min alarm" },
	{ 0x538, "wsha", WS_SPEED, "wind speed max alarm" },
/*	{ 0x54d, "cn", conv_conn, "connection type" },*/
	{ 0x54f, "cc", WS_INT_SEC, "connection time till connect" },
	{ 0x5d8, "pa", WS_PRESSURE, "pressure absolute" },
	{ 0x5e2, "pr", WS_PRESSURE, "pressure relative" },
	{ 0x5ec, "pc", WS_PRESSURE, "pressure correction" },
	{ 0x5f6, "pal", WS_PRESSURE, "pressure absolute min", "pa" },
	{ 0x600, "prl", WS_PRESSURE, "pressure relative min", "pr" },
	{ 0x60a, "pah", WS_PRESSURE, "pressure absolute max", "pa" },
	{ 0x614, "prh", WS_PRESSURE, "pressure relative max", "pr" },
	{ 0x61e, "plw", WS_TIMESTAMP, "pressure min when", "sw" },
	{ 0x628, "phw", WS_TIMESTAMP, "pressure max when", "sw" },
	{ 0x63c, "pla", WS_PRESSURE, "pressure min alarm" },
	{ 0x650, "pha", WS_PRESSURE, "pressure max alarm" },
	{ 0x6b2, "hi", WS_INT_MIN, "history interval" },
	{ 0x6b5, "hc", WS_INT_MIN, "history time till sample" },
	{ 0x6b8, "hw", WS_TIMESTAMP, "history last sample when" },
/*	{ 0x6c2, "hp", conv_rec2, "history last record pointer",reset=0)
	{ 0x6c4, "hn", conv_rec2, "history number of records", 0) */
};

static void
usage(FILE *out, int code)
{
	fprintf(out, "Usage: [-h] %s\n device measure...", PROGNAME);

	exit(code);
}

static void
do_help()
{
	int i;

	for (i = 0; i < array_sz(mem_map); i++) {
		const struct ws_measure *m = &mem_map[i];
		const struct ws_conv *c = ws_get_conv(m->type);

		if (c->units != NULL) {
			printf("%-5s %-30s 0x%3x:%-2d  %s, %s\n", m->id, m->desc, m->addr,
					c->nybbles, c->units, c->descr);
		} else {
			printf("%-5s %-30s 0x%3x:%-2d  %s\n", m->id, m->desc, m->addr,
					c->nybbles, c->descr);
		}
	}
}

static int
do_fetch(int fd, char * const ids[], int size) {
//	uint16_t addr[size];
//	size_t nybbles[size];

	uint8_t data[1024];
	char str[1024];

	/* Order measures by address */
	for (int i = 0; i < array_sz(mem_map); i++) {
		const struct ws_measure *m = &mem_map[i];
		const struct ws_conv *c = ws_get_conv(m->type);

		for (int j = 0; j < size; j++) {
			if (strcmp(ids[j], m->id) == 0) {
//				addr[i] = m->addr;
//				nybbles[i] = c->nybbles;

				if (ws_read_safe(fd, m->addr, c->nybbles, data) == -1) {
					return -1;
				}

				switch (m->type) {
				case WS_TEMP:
					ws_temp_str(data, str, sizeof(str));
					break;

				case WS_DATETIME:
					ws_datetime_str(data, str, sizeof(str));
					break;

				case WS_TIMESTAMP:
					ws_timestamp_str(data, str, sizeof(str));
					break;

				default:
					fprintf(stderr, "not yet supported");
					break;
				}

				if (c->units != NULL) {
					printf("%s = %s %s\n", m->desc, str, c->units);
				} else {
					printf("%s = %s\n", m->desc, str);
				}
			}
		}
	}

	return 0;
}

int
main(int argc, char * const argv[])
{
	int c;
	int errflg = 0;

	const char *device = NULL;

	/* Parse arguments */
	while ((c = getopt(argc, argv, "h")) != -1) {
		switch (c) {
		case 'h':
			do_help();
			usage(stdout, 0);
			break;

		case ':':
			fprintf(stderr, "Option -%c requires an operand\n", optopt);
			errflg++;
			break;

		case '?':
			fprintf(stderr, "Unrecognized option: '-%c'\n", optopt);
			errflg++;
			break;
		}
	}

	if (errflg) {
		usage(stderr, 1);
	}

	device = argv[optind++];

	/* Loading data */
	int fd = ws_open(device);
	if (fd == -1) {
		exit(1);
	}

	do_fetch(fd, argv + optind, argc - optind);
	ws_close(fd);

	exit(0);
}

