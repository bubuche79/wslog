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

#ifndef PROGNAME
#define PROGNAME	"ws2300"
#endif

#ifndef VERSION
#define VERSION		"0.1"
#endif

#define array_len(a)	(sizeof(a) / sizeof(a[0]))

static const struct ws_type types[] =
{
	/* BCD converters */
	{ WS_TEMP, "°C", 4, "temperature", .num = { 2 } },
	{ WS_PRESSURE, "hPa", 5, "pressure", .num = { 1 } },
	{ WS_HUMIDITY, "%", 2, "humidity", .num = { 0 } },
	{ WS_RAIN, "mm", 6, "rain", .num = { 2 } },

	/* Wind direction converter */
	{ WS_WIND_DIR, "deg", 1, "wind direction, North=0 clockwise" },

	/* Wind velocity converter */
	{ WS_WIND_VELOCITY, "ms,d", 4, "wind speed and direction" },

	/* Bin converters */
	{ WS_SPEED, "m/s", 3, "speed", .num = { 1 } },
	{ WS_INT_SEC, "s", 2, "time interval", .num = { 1 } },
	{ WS_INT_MIN, "min", 3, "time interval", .num = { 0 } },
	{ WS_BIN_2NYB, NULL, 2, "record number", .num = { 0 } },
	{ WS_CONTRAST, NULL, 1, "contrast", .num = { 0 } },

	/* Date and time converters */
	{ WS_DATE, NULL, 6, "yyyy-mm-dd" },
	{ WS_TIMESTAMP, NULL, 10, "yyyy-mm-dd hh:mm" },
	{ WS_DATETIME, NULL, 11, "yyyy-mm-dd hh:mm" },
	{ WS_TIME, NULL, 6, "hh:mm:ss" },

	/* Text converters */
	{ WS_CONNECTION, NULL, 1, .text.a = {{0, "cable"}, {3, "lost"}, {15, "wireless"}} },
	{ WS_FORECAST, NULL, 1, .text.a = {{0, "rainy"}, {1, "cloudy"}, {2, "sunny"}} },
	{ WS_TENDENCY, NULL, 1, .text.a = {{0, "steady"}, {1, "rising"}, {2, "falling"}} },
	{ WS_SPEED_UNIT, NULL, 1, .text.a = {{0, "m/s"}, {1, "knots"}, {2, "beaufort"}, {3, "km/h"}, {4, "mph"}} },
	{ WS_WIND_OVERFLOW, NULL, 1, .text.a = {{0, "no"}, {1, "overflow"}} },
	{ WS_WIND_VALID, NULL, 1, .text.a = {{0, "ok"}, {1, "invalid"}, {2, "overflow"}} },

	/* Bit converters */
	{ WS_ALARM_SET_0, NULL, 1, .bit = { "off", "on" } },
	{ WS_ALARM_SET_1, NULL, 1, .bit = { "off", "on" } },
	{ WS_ALARM_SET_2, NULL, 1, .bit = { "off", "on" } },
	{ WS_ALARM_SET_3, NULL, 1, .bit = { "off", "on" } },
	{ WS_ALARM_ACTIVE_0, NULL, 1, .bit = { "inactive", "active" } },
	{ WS_ALARM_ACTIVE_1, NULL, 1, .bit = { "inactive", "active" } },
	{ WS_ALARM_ACTIVE_2, NULL, 1, .bit = { "inactive", "active" } },
	{ WS_ALARM_ACTIVE_3, NULL, 1, .bit = { "inactive", "active" } },
	{ WS_BUZZER, NULL, 1, .bit = { "on", "off" } },
	{ WS_BACKLIGHT, NULL, 1, .bit = { "off", "on" } },
};

/* ws23xx memory, ordered by address */
static const struct ws_measure mem_addr[] =
{
	{ 0x006, "bz", &types[WS_BUZZER], "buzzer" },
	{ 0x00f, "wsu", &types[WS_SPEED_UNIT], "wind speed units" },
	{ 0x016, "lb", &types[WS_BACKLIGHT], "lcd backlight" },
	{ 0x019, "sss", &types[WS_ALARM_SET_2], "storm warn alarm set" },
	{ 0x019, "sts", &types[WS_ALARM_SET_0], "station time alarm set" },
	{ 0x01a, "phs", &types[WS_ALARM_SET_3], "pressure max alarm set" },
	{ 0x01a, "pls", &types[WS_ALARM_SET_2], "pressure min alarm set" },
	{ 0x01b, "oths", &types[WS_ALARM_SET_3], "out temp max alarm set" },
	{ 0x01b, "otls", &types[WS_ALARM_SET_2], "out temp min alarm set" },
	{ 0x01b, "iths", &types[WS_ALARM_SET_1], "in temp max alarm set" },
	{ 0x01b, "itls", &types[WS_ALARM_SET_0], "in temp min alarm set" },
	{ 0x01c, "dphs", &types[WS_ALARM_SET_3], "dew point max alarm set" },
	{ 0x01c, "dpls", &types[WS_ALARM_SET_2], "dew point min alarm set" },
	{ 0x01c, "wchs", &types[WS_ALARM_SET_1], "wind chill max alarm set" },
	{ 0x01c, "wcls", &types[WS_ALARM_SET_0], "wind chill min alarm set" },
	{ 0x01d, "ihhs", &types[WS_ALARM_SET_3], "in humidity max alarm set" },
	{ 0x01d, "ihls", &types[WS_ALARM_SET_2], "in humidity min alarm set" },
	{ 0x01d, "ohhs", &types[WS_ALARM_SET_1], "out humidity max alarm set" },
	{ 0x01d, "ohls", &types[WS_ALARM_SET_0], "out humidity min alarm set" },
	{ 0x01e, "rhhs", &types[WS_ALARM_SET_1], "rain 1h alarm set" },
	{ 0x01e, "rdhs", &types[WS_ALARM_SET_0], "rain 24h alarm set" },
	{ 0x01f, "wds", &types[WS_ALARM_SET_2], "wind direction alarm set" },
	{ 0x01f, "wshs", &types[WS_ALARM_SET_1], "wind speed max alarm set" },
	{ 0x01f, "wsls", &types[WS_ALARM_SET_0], "wind speed min alarm set" },
	{ 0x020, "siv", &types[WS_ALARM_ACTIVE_2], "icon alarm active" },
	{ 0x020, "stv", &types[WS_ALARM_ACTIVE_0], "station time alarm active" },
	{ 0x021, "phv", &types[WS_ALARM_ACTIVE_3], "pressure max alarm active" },
	{ 0x021, "plv", &types[WS_ALARM_ACTIVE_2], "pressure min alarm active" },
	{ 0x022, "othv", &types[WS_ALARM_ACTIVE_3], "out temp max alarm active" },
	{ 0x022, "otlv", &types[WS_ALARM_ACTIVE_2], "out temp min alarm active" },
	{ 0x022, "ithv", &types[WS_ALARM_ACTIVE_1], "in temp max alarm active" },
	{ 0x022, "itlv", &types[WS_ALARM_ACTIVE_0], "in temp min alarm active" },
	{ 0x023, "dphv", &types[WS_ALARM_ACTIVE_3], "dew point max alarm active" },
	{ 0x023, "dplv", &types[WS_ALARM_ACTIVE_2], "dew point min alarm active" },
	{ 0x023, "wchv", &types[WS_ALARM_ACTIVE_1], "wind chill max alarm active" },
	{ 0x023, "wclv", &types[WS_ALARM_ACTIVE_0], "wind chill min alarm active" },
	{ 0x024, "ihhv", &types[WS_ALARM_ACTIVE_3], "in humidity max alarm active" },
	{ 0x024, "ihlv", &types[WS_ALARM_ACTIVE_2], "in humidity min alarm active" },
	{ 0x024, "ohhv", &types[WS_ALARM_ACTIVE_1], "out humidity max alarm active" },
	{ 0x024, "ohlv", &types[WS_ALARM_ACTIVE_0], "out humidity min alarm active" },
	{ 0x025, "rhhv", &types[WS_ALARM_ACTIVE_1], "rain 1h alarm active" },
	{ 0x025, "rdhv", &types[WS_ALARM_ACTIVE_0], "rain 24h alarm active" },
	{ 0x026, "wdv", &types[WS_ALARM_ACTIVE_2], "wind direction alarm active" },
	{ 0x026, "wshv", &types[WS_ALARM_ACTIVE_1], "wind speed max alarm active" },
	{ 0x026, "wslv", &types[WS_ALARM_ACTIVE_0], "wind speed min alarm active" },
/*	{ 0x027, NULL, &types[WS_ALARM_ACTIVE_3], "pressure max alarm active alias" },
	{ 0x027, NULL, &types[WS_ALARM_ACTIVE_2], "pressure min alarm active alias" },
	{ 0x028, NULL, &types[WS_ALARM_ACTIVE_3], "out temp max alarm active alias" },
	{ 0x028, NULL, &types[WS_ALARM_ACTIVE_2], "out temp min alarm active alias" },
	{ 0x028, NULL, &types[WS_ALARM_ACTIVE_1], "in temp max alarm active alias" },
	{ 0x028, NULL, &types[WS_ALARM_ACTIVE_0], "in temp min alarm active alias" },
	{ 0x029, NULL, &types[WS_ALARM_ACTIVE_3], "dew point max alarm active alias" },
	{ 0x029, NULL, &types[WS_ALARM_ACTIVE_2], "dew point min alarm active alias" },
	{ 0x029, NULL, &types[WS_ALARM_ACTIVE_1], "wind chill max alarm active alias" },
	{ 0x029, NULL, &types[WS_ALARM_ACTIVE_0], "wind chill min alarm active alias" },
	{ 0x02a, NULL, &types[WS_ALARM_ACTIVE_3], "in humidity max alarm active alias" },
	{ 0x02a, NULL, &types[WS_ALARM_ACTIVE_2], "in humidity min alarm active alias" },
	{ 0x02a, NULL, &types[WS_ALARM_ACTIVE_1], "out humidity max alarm active alias" },
	{ 0x02a, NULL, &types[WS_ALARM_ACTIVE_0], "out humidity min alarm active alias" },
	{ 0x02b, NULL, &types[WS_ALARM_ACTIVE_1], "rain 1h alarm active alias" },
	{ 0x02b, NULL, &types[WS_ALARM_ACTIVE_0], "rain 24h alarm active alias" },
	{ 0x02c, NULL, &types[WS_ALARM_ACTIVE_2], "wind direction alarm active alias" },
	{ 0x02c, NULL, &types[WS_ALARM_ACTIVE_2], "wind speed max alarm active alias" },
	{ 0x02c, NULL, &types[WS_ALARM_ACTIVE_2], "wind speed min alarm active alias" },*/
	{ 0x200, "st", &types[WS_TIME], "station set time", "ct" },
	{ 0x23b, "sw", &types[WS_DATETIME], "station current date time" },
	{ 0x24d, "sd", &types[WS_DATE], "station set date", "cd" },
	{ 0x266, "lc", &types[WS_CONTRAST], "lcd contrast (ro)" },
	{ 0x26b, "for", &types[WS_FORECAST], "forecast" },
	{ 0x26c, "ten", &types[WS_TENDENCY], "tendency" },
	{ 0x346, "it", &types[WS_TEMP], "in temp" },
	{ 0x34b, "itl", &types[WS_TEMP], "in temp min", "it" },
	{ 0x350, "ith", &types[WS_TEMP], "in temp max", "it" },
	{ 0x354, "itlw", &types[WS_TIMESTAMP], "in temp min when", "sw" },
	{ 0x35e, "ithw", &types[WS_TIMESTAMP], "in temp max when", "sw" },
	{ 0x369, "itla", &types[WS_TEMP], "in temp min alarm" },
	{ 0x36e, "itha", &types[WS_TEMP], "in temp max alarm" },
	{ 0x373, "ot", &types[WS_TEMP], "out temp" },
	{ 0x378, "otl", &types[WS_TEMP], "out temp min", "ot" },
	{ 0x37d, "oth", &types[WS_TEMP], "out temp max", "ot" },
	{ 0x381, "otlw", &types[WS_TIMESTAMP], "out temp min when", "sw" },
	{ 0x38b, "othw", &types[WS_TIMESTAMP], "out temp max when", "sw" },
	{ 0x396, "otla", &types[WS_TEMP], "out temp min alarm" },
	{ 0x39b, "otha", &types[WS_TEMP], "out temp max alarm" },
	{ 0x3a0, "wc", &types[WS_TEMP], "wind chill" },
	{ 0x3a5, "wcl", &types[WS_TEMP], "wind chill min", "wc" },
	{ 0x3aa, "wch", &types[WS_TEMP], "wind chill max", "wc" },
	{ 0x3ae, "wclw", &types[WS_TIMESTAMP], "wind chill min when", "sw" },
	{ 0x3b8, "wchw", &types[WS_TIMESTAMP], "wind chill max when", "sw" },
	{ 0x3c3, "wcla", &types[WS_TEMP], "wind chill min alarm" },
	{ 0x3c8, "wcha", &types[WS_TEMP], "wind chill max alarm" },
	{ 0x3ce, "dp", &types[WS_TEMP], "dew point" },
	{ 0x3d3, "dpl", &types[WS_TEMP], "dew point min", "dp" },
	{ 0x3d8, "dph", &types[WS_TEMP], "dew point max", "dp" },
	{ 0x3dc, "dplw", &types[WS_TIMESTAMP], "dew point min when", "sw" },
	{ 0x3e6, "dphw", &types[WS_TIMESTAMP], "dew point max when", "sw" },
	{ 0x3f1, "dpla", &types[WS_TEMP], "dew point min alarm" },
	{ 0x3f6, "dpha", &types[WS_TEMP], "dew point max alarm" },
	{ 0x3fb, "ih", &types[WS_HUMIDITY], "in humidity" },
	{ 0x3fd, "ihl", &types[WS_HUMIDITY], "in humidity min", "ih" },
	{ 0x3ff, "ihh", &types[WS_HUMIDITY], "in humidity max", "ih" },
	{ 0x401, "ihlw", &types[WS_TIMESTAMP], "in humidity min when", "sw" },
	{ 0x40b, "ihhw", &types[WS_TIMESTAMP], "in humidity max when", "sw" },
	{ 0x415, "ihla", &types[WS_HUMIDITY], "in humidity min alarm" },
	{ 0x417, "ihha", &types[WS_HUMIDITY], "in humidity max alarm" },
	{ 0x419, "oh", &types[WS_HUMIDITY], "out humidity" },
	{ 0x41b, "ohl", &types[WS_HUMIDITY], "out humidity min", "oh" },
	{ 0x41d, "ohh", &types[WS_HUMIDITY], "out humidity max", "oh" },
	{ 0x41f, "ohlw", &types[WS_TIMESTAMP], "out humidity min when", "sw" },
	{ 0x429, "ohhw", &types[WS_TIMESTAMP], "out humidity max when", "sw" },
	{ 0x433, "ohla", &types[WS_HUMIDITY], "out humidity min alarm" },
	{ 0x435, "ohha", &types[WS_HUMIDITY], "out humidity max alarm" },
	{ 0x497, "rd", &types[WS_RAIN], "rain 24h" },
	{ 0x49d, "rdh", &types[WS_RAIN], "rain 24h max", "rd" },
	{ 0x4a3, "rdhw", &types[WS_TIMESTAMP], "rain 24h max when", "sw" },
	{ 0x4ae, "rdha", &types[WS_RAIN], "rain 24h max alarm" },
	{ 0x4b4, "rh", &types[WS_RAIN], "rain 1h" },
	{ 0x4ba, "rhh", &types[WS_RAIN], "rain 1h max", "rh" },
	{ 0x4c0, "rhhw", &types[WS_TIMESTAMP], "rain 1h max when", "sw" },
	{ 0x4cb, "rhha", &types[WS_RAIN], "rain 1h max alarm" },
	{ 0x4d2, "rt", &types[WS_RAIN], "rain total", "0" },
	{ 0x4d8, "rtrw", &types[WS_TIMESTAMP], "rain total reset when", "sw" },
	{ 0x4ee, "wsl", &types[WS_SPEED], "wind speed min", "ws" },
	{ 0x4f4, "wsh", &types[WS_SPEED], "wind speed max", "ws" },
	{ 0x4f8, "wslw", &types[WS_TIMESTAMP], "wind speed min when", "sw" },
	{ 0x502, "wshw", &types[WS_TIMESTAMP], "wind speed max when", "sw" },
	{ 0x527, "wso", &types[WS_WIND_OVERFLOW], "wind speed overflow" },
	{ 0x528, "wsv", &types[WS_WIND_VALID], "wind speed validity" },
	{ 0x529, "wv", &types[WS_WIND_VELOCITY], "wind velocity" },
	{ 0x529, "ws", &types[WS_SPEED], "wind speed" },
	{ 0x52c, "w0", &types[WS_WIND_DIR], "wind direction" },
	{ 0x52d, "w1", &types[WS_WIND_DIR], "wind direction 1" },
	{ 0x52e, "w2", &types[WS_WIND_DIR], "wind direction 2" },
	{ 0x52f, "w3", &types[WS_WIND_DIR], "wind direction 3" },
	{ 0x530, "w4", &types[WS_WIND_DIR], "wind direction 4" },
	{ 0x531, "w5", &types[WS_WIND_DIR], "wind direction 5" },
	{ 0x533, "wsla", &types[WS_SPEED], "wind speed min alarm" },
	{ 0x538, "wsha", &types[WS_SPEED], "wind speed max alarm" },
	{ 0x54d, "cn", &types[WS_CONNECTION], "connection type" },
	{ 0x54f, "cc", &types[WS_INT_SEC], "connection time till connect" },
	{ 0x5d8, "pa", &types[WS_PRESSURE], "pressure absolute" },
	{ 0x5e2, "pr", &types[WS_PRESSURE], "pressure relative" },
	{ 0x5ec, "pc", &types[WS_PRESSURE], "pressure correction" },
	{ 0x5f6, "pal", &types[WS_PRESSURE], "pressure absolute min", "pa" },
	{ 0x600, "prl", &types[WS_PRESSURE], "pressure relative min", "pr" },
	{ 0x60a, "pah", &types[WS_PRESSURE], "pressure absolute max", "pa" },
	{ 0x614, "prh", &types[WS_PRESSURE], "pressure relative max", "pr" },
	{ 0x61e, "plw", &types[WS_TIMESTAMP], "pressure min when", "sw" },
	{ 0x628, "phw", &types[WS_TIMESTAMP], "pressure max when", "sw" },
	{ 0x63c, "pla", &types[WS_PRESSURE], "pressure min alarm" },
	{ 0x650, "pha", &types[WS_PRESSURE], "pressure max alarm" },
	{ 0x6b2, "hi", &types[WS_INT_MIN], "history interval" },
	{ 0x6b5, "hc", &types[WS_INT_MIN], "history time till sample" },
	{ 0x6b8, "hw", &types[WS_TIMESTAMP], "history last sample when" },
	{ 0x6c2, "hp", &types[WS_BIN_2NYB], "history last record pointer", "0" },
	{ 0x6c4, "hn", &types[WS_BIN_2NYB], "history number of records", "0"}
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

static void
nybprint(uint16_t addr, const uint8_t *buf, uint16_t nnybles, int hex)
{
	int disp_sz;

	if (hex) {
		disp_sz = 2;
	} else {
		disp_sz = 1;
	}

	for (uint16_t i = 0; i < nnybles;) {
		printf("%.4x", addr + i);

		for (uint16_t j = 0; j < 16 && i < nnybles; j++) {
			uint8_t v;

			if (hex) {
				v = buf[i];
			} else {
				v = ws_nybble(buf, i);
			}

			printf(" %.*x", disp_sz, v);

			i += disp_sz;
		}

		printf("\n");
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
		const size_t *off, size_t nel, const char *sep) {
	/* Print result */
	for (size_t i = 0; i < nel; i++) {
		const struct ws_measure *m = mids[i];
		const struct ws_type *t = m->type;

		char str[128];
		size_t len = sizeof(str);

		switch (t->id) {
		case WS_TEMP:
			ws_temp_str(data, str, len, off[i]);
			break;

		case WS_PRESSURE:
			ws_pressure_str(data, str, len, off[i]);
			break;

		case WS_HUMIDITY:
			ws_humidity_str(data, str, len, off[i]);
			break;

		case WS_SPEED:
			ws_speed_str(data, str, len, off[i]);
			break;

		case WS_WIND_DIR:
			ws_wind_dir_str(data, str, len, off[i]);
			break;

		case WS_RAIN:
			ws_rain_str(data, str, len, off[i]);
			break;

		case WS_INT_SEC:
			ws_interval_sec_str(data, str, len, off[i]);
			break;

		case WS_INT_MIN:
			ws_interval_min_str(data, str, len, off[i]);
			break;

		case WS_BIN_2NYB:
			ws_bin_2nyb_str(data, str, len, off[i]);
			break;

		case WS_TIMESTAMP:
			ws_timestamp_str(data, str, len, off[i]);
			break;

		case WS_DATETIME:
			ws_datetime_str(data, str, len, off[i]);
			break;

		case WS_CONNECTION:
			ws_connection_str(data, str, len, off[i]);
			break;

		case WS_ALARM_SET_0:
		case WS_ALARM_SET_1:
		case WS_ALARM_SET_2:
		case WS_ALARM_SET_3:
			ws_alarm_set_str(data, str, len, off[i], t->id - WS_ALARM_SET_0);
			break;

		case WS_ALARM_ACTIVE_0:
		case WS_ALARM_ACTIVE_1:
		case WS_ALARM_ACTIVE_2:
		case WS_ALARM_ACTIVE_3:
			ws_alarm_active_str(data, str, len, off[i], t->id - WS_ALARM_ACTIVE_0);
			break;

		default:
			snprintf(str, len, "-");
			fprintf(stderr, "not yet supported");
			break;
		}

		if (sep == NULL) {
			if (t->units != NULL) {
				printf("%s = %s %s\n", m->desc, str, t->units);
			} else {
				printf("%s = %s\n", m->desc, str);
			}
		} else {
			if (i > 0) {
				printf("%s%s", sep, str);
			} else {
				printf("%s", str);
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

	size_t off[nel];					/* nybble offset in buffer */
	const struct ws_measure *mids[nel];	/* measures */

	int opt_nel = 0;

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

				if (opt_nel > 0) {
					/**
					 * Using the same area saves 6 bytes read, and 5 bytes written,
					 * so we may overlap here to save I/O.
					 */
					if (addr[opt_nel-1] + nnybble[opt_nel-1] + 11 >= m->addr) {
						same_area = 1;
					}
				}

				if (same_area) {
					nnybble[opt_nel-1] = m->addr + t->nybble - addr[opt_nel-1];
				} else {
					if (opt_nel > 0) {
						nbyte += (nnybble[opt_nel-1] + 1) / 2;
					}

					addr[opt_nel] = m->addr;
					nnybble[opt_nel] = t->nybble;
					buf[opt_nel] = data + nbyte;

					opt_nel++;
				}

				mids[j] = m;
				off[j] = 2 * nbyte + (m->addr - addr[opt_nel-1]);
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
	if (ws_read_batch(fd, addr, nnybble, opt_nel, buf) == -1) {
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
ws_desc(const struct ws_type *t, char *desc, size_t len) {
	switch (t->id) {
	case WS_ALARM_SET_0:
	case WS_ALARM_SET_1:
	case WS_ALARM_SET_2:
	case WS_ALARM_SET_3:
	case WS_ALARM_ACTIVE_0:
	case WS_ALARM_ACTIVE_1:
	case WS_ALARM_ACTIVE_2:
	case WS_ALARM_ACTIVE_3:
	case WS_BUZZER:
	case WS_BACKLIGHT:
		snprintf(desc, len, "0=%s,1=%s", t->bit.unset, t->bit.set);
		break;

	case WS_CONNECTION:
	case WS_FORECAST:
	case WS_TENDENCY:
	case WS_SPEED_UNIT:
	case WS_WIND_OVERFLOW:
	case WS_WIND_VALID:
		strncpy(desc, "TODO", len);
		break;

	default:
		strncpy(desc, t->desc, len);
		break;
	}
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

		char desc[128];

		ws_desc(t, desc, sizeof(desc));

		if (t->units != NULL) {
			printf("%-5s %-30s 0x%.3x:%-2d  %s, %s\n", m->id, m->desc, m->addr,
					t->nybble, t->units, desc);
		} else {
			printf("%-5s %-30s 0x%.3x:%-2d  %s\n", m->id, m->desc, m->addr,
					t->nybble, desc);
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

	int print_hex = 0;

	/* Parse sub-command arguments */
	while ((c = getopt(argc, argv, "x")) != -1) {
		switch (c) {
		case 'x':
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

	nybprint(addr, buf, nnybles, print_hex);

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
