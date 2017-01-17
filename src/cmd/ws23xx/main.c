#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "libws/nybble.h"
#include "libws/ws23xx/ws23xx.h"
#include "libws/ws23xx/decoder.h"
#include "libws/serial.h"

#include "history.h"

#define CSV_DATE	"%Y-%m-%dT%H:%M"

#ifndef PROGNAME
#define PROGNAME	"ws2300"
#endif

#ifndef VERSION
#define VERSION		"0.1"
#endif

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
	{ WS_ALARM_SET_0, NULL, 1, .bit = { 0, "off", "on" } },
	{ WS_ALARM_SET_1, NULL, 1, .bit = { 1, "off", "on" } },
	{ WS_ALARM_SET_2, NULL, 1, .bit = { 2, "off", "on" } },
	{ WS_ALARM_SET_3, NULL, 1, .bit = { 3, "off", "on" } },
	{ WS_ALARM_ACTIVE_0, NULL, 1, .bit = { 0, "inactive", "active" } },
	{ WS_ALARM_ACTIVE_1, NULL, 1, .bit = { 1, "inactive", "active" } },
	{ WS_ALARM_ACTIVE_2, NULL, 1, .bit = { 2, "inactive", "active" } },
	{ WS_ALARM_ACTIVE_3, NULL, 1, .bit = { 3, "inactive", "active" } },
	{ WS_BUZZER, NULL, 1, .bit = { 3, "on", "off" } },
	{ WS_BACKLIGHT, NULL, 1, .bit = { 0, "off", "on" } },
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

struct ws_io
{
	const char *id;
	void *buf;
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

static int
wsmaddrcmp(const void *a, const void *b)
{
	const uint16_t *key = a;
	const struct ws_measure *mb = (struct ws_measure *) b;

	return *key - mb->addr;
}

static int
wsmcmp(const void *a, const void *b)
{
	const struct ws_measure *ma = * (struct ws_measure **) a;
	const struct ws_measure *mb = * (struct ws_measure **) b;

	return strcmp(ma->id, mb->id);
}

static int
wsmkeycmp(const void *a, const void *b)
{
	const char *key = a;
	const struct ws_measure *mb = * (struct ws_measure **) b;

	return strcmp(key, mb->id);
}

static const struct ws_measure *
search_addr(uint16_t addr)
{
	return (const struct ws_measure *) bsearch(&addr, mem_addr, array_len(mem_addr), sizeof(*mem_addr), wsmaddrcmp);
}

static const struct ws_measure *
search_id(const char *id)
{
	return * (const struct ws_measure **) bsearch(id, mem_id, array_len(mem_id), sizeof(*mem_id), wsmkeycmp);
}

/**
 * Initialize static memory.
 *
 * The #init() method shall be called first to initialize static memory area,
 * like the #mem_id array (which is initialized in this function).
 */
static void
init(void)
{
	size_t nel = array_len(mem_id);

	/* Sort mem_addr by id */
	for (size_t i = 0; i < nel; i++) {
		mem_id[i] = &mem_addr[i];
	}

	qsort(mem_id, nel, sizeof(mem_id[0]), wsmcmp);

	/* Set POSIX.2 behaviour for getopt() */
	setenv("POSIXLY_CORRECT", "1", 1);
}

static void *
decode(const uint8_t *buf, enum ws_etype type, uint8_t *v, size_t offset)
{
	switch (type) {
	case WS_TEMP:
		ws23xx_temp(buf, (double *) v, offset);
		break;

	case WS_PRESSURE:
		ws23xx_pressure(buf, (double *) v, offset);
		break;

	case WS_HUMIDITY:
		ws23xx_humidity(buf, (uint8_t *) v, offset);
		break;

	case WS_SPEED:
		ws23xx_speed(buf, (double *) v, offset);
		break;

	case WS_WIND_DIR:
		ws23xx_wind_dir(buf, (uint16_t *) v, offset);
		break;

	case WS_RAIN:
		ws23xx_rain(buf, (double *) v, offset);
		break;

	//		case WS_INT_SEC:
	//			ws_interval_sec_str(data, str, len, offset);
	//			break;
	//
	//		case WS_INT_MIN:
	//			ws_interval_min_str(data, str, len, offset);
	//			break;
	//
	//		case WS_BIN_2NYB:
	//			ws_bin_2nyb_str(data, str, len, offset);
	//			break;
	//
	//		case WS_TIMESTAMP:
	//			ws_timestamp_str(data, str, len, offset);
	//			break;

	case WS_DATETIME:
		ws23xx_datetime(buf, (time_t *) v, offset);
		break;

	case WS_CONNECTION:
		ws23xx_connection(buf, (uint8_t *) v, offset);
		break;
	//
	//		case WS_ALARM_SET_0:
	//		case WS_ALARM_SET_1:
	//		case WS_ALARM_SET_2:
	//		case WS_ALARM_SET_3:
	//			ws_alarm_set_str(data, str, len, offset, t->id - WS_ALARM_SET_0);
	//			break;
	//
	//		case WS_ALARM_ACTIVE_0:
	//		case WS_ALARM_ACTIVE_1:
	//		case WS_ALARM_ACTIVE_2:
	//		case WS_ALARM_ACTIVE_3:
	//			ws_alarm_active_str(data, str, len, offset, t->id - WS_ALARM_ACTIVE_0);
	//			break;
	//
	default:
	//			fprintf(stderr, "not yet supported\n");
		v = NULL;
		errno = ENOTSUP;

		break;
	}

	return v;
}

static char *
decode_str_bit(const uint8_t *buf, enum ws_etype type, char *s, size_t len, size_t offset)
{
	const struct ws_type *t = &types[type];
	uint8_t v = ws23xx_bit(buf, offset, t->bit.b);

	strncpy(s, v ? t->bit.set : t->bit.unset, len);

	return s;
}

static char *
decode_str_text(const uint8_t *buf, enum ws_etype type, char *s, size_t len, size_t offset)
{
	int i;

	const struct ws_type *t = &types[type];
	uint8_t v = nybget(buf, offset);

	for (i = 0; t->text.a[i].value != NULL && v != t->text.a[i].key; i++) {
		/* loop */
	}

	if (t->text.a[i].value == NULL) {
		snprintf(s, len, "%x (%s)", v, "error");
	} else {
		strncpy(s, t->text.a[i].value, len);
	}

	return s;
}

static char *
decode_str(const uint8_t *buf, enum ws_etype type, char *s, size_t len, size_t offset)
{
	switch (type) {
	case WS_TEMP:
		ws23xx_temp_str(buf, s, len, offset);
		break;

	case WS_PRESSURE:
		ws23xx_pressure_str(buf, s, len, offset);
		break;

	case WS_HUMIDITY:
		ws23xx_humidity_str(buf, s, len, offset);
		break;

	case WS_SPEED:
		ws23xx_speed_str(buf, s, len, offset);
		break;

	case WS_WIND_DIR:
		ws23xx_wind_dir_str(buf, s, len, offset);
		break;

	case WS_RAIN:
		ws23xx_rain_str(buf, s, len, offset);
		break;

	case WS_INT_SEC:
		ws23xx_interval_sec_str(buf, s, len, offset);
		break;

	case WS_INT_MIN:
		ws23xx_interval_min_str(buf, s, len, offset);
		break;

	case WS_BIN_2NYB:
		ws23xx_bin_2nyb_str(buf, s, len, offset);
		break;

	case WS_TIMESTAMP:
		ws23xx_timestamp_str(buf, s, len, offset);
		break;

	case WS_DATETIME:
		ws23xx_datetime_str(buf, s, len, offset);
		break;

	case WS_ALARM_SET_0:
	case WS_ALARM_SET_1:
	case WS_ALARM_SET_2:
	case WS_ALARM_SET_3:
	case WS_ALARM_ACTIVE_0:
	case WS_ALARM_ACTIVE_1:
	case WS_ALARM_ACTIVE_2:
	case WS_ALARM_ACTIVE_3:
	case WS_BACKLIGHT:
	case WS_BUZZER:
		decode_str_bit(buf, type, s, len, offset);
		break;

	case WS_CONNECTION:
	case WS_FORECAST:
	case WS_TENDENCY:
	case WS_SPEED_UNIT:
	case WS_WIND_OVERFLOW:
	case WS_WIND_VALID:
		decode_str_text(buf, type, s, len, offset);
		break;

	default:
		snprintf(s, len, "-");
		fprintf(stderr, "not yet supported\n");
		break;
	}

	return s;
}

static void
desc_bit(const struct ws_type *t, char *desc, size_t len)
{
	snprintf(desc, len, "0=%s, 1=%s", t->bit.unset, t->bit.set);
}

static void
desc_text(const struct ws_type *t, char *desc, size_t len)
{
	int i;
	int sz = 0;

	for (i = 0; sz < (int) len && t->text.a[i].value != NULL; i++) {
		sz += snprintf(desc + sz, len - sz, "%s%hhu=%s",
				(i > 0) ? ", " : "", t->text.a[i].key, t->text.a[i].value);
	}
}

static void
ws23xx_desc(const struct ws_type *t, char *desc, size_t len)
{
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
		desc_bit(t, desc, len);
		break;

	case WS_CONNECTION:
	case WS_FORECAST:
	case WS_TENDENCY:
	case WS_SPEED_UNIT:
	case WS_WIND_OVERFLOW:
	case WS_WIND_VALID:
		desc_text(t, desc, len);
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

		ws23xx_desc(t, desc, sizeof(desc));

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

	if (ws23xx_read_safe(fd, addr, nnybles, buf) == -1) {
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
print_measures(const uint16_t *addr, const uint8_t *buf[], size_t nel, const char *sep)
{
	/* Print result */
	for (size_t i = 0; i < nel; i++) {
		const struct ws_measure *m = search_addr(addr[i]);
		const struct ws_type *t = m->type;

		char str[128];
		size_t len = sizeof(str);

		decode_str(buf[i], t->id, str, len, 0);

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

	if (argc - 1 < optind) {
		usage(stderr, 1);
	}

	device = argv[optind++];

	/* Parse measure arguments */
	size_t nel = (optind < argc) ? (size_t) argc - optind : array_len(mem_id);

	uint16_t addr[nel];
	size_t nnyb[nel];
	uint8_t *buf[nel];

	if (optind < argc) {
		int i = 0;

		for (; optind < argc; optind++) {
			const struct ws_measure *m;

			m = search_id(argv[optind]);

			if (m == NULL) {
				fprintf(stderr, "Unknown measure: %s\n", argv[optind]);
				exit(1);
			}

			addr[i] = m->addr;
			nnyb[i] = m->type->nybble;
			buf[i] = malloc(32);

			i++;
		}
	} else {
		for (size_t i = 0; i < nel; i++) {
			addr[i] = mem_addr[i].addr;
			nnyb[i] = mem_addr[i].type->nybble;
			buf[i] = malloc(32);
		}
	}

	/* Process sub-command */
	int fd = ws_open(device);
	if (fd == -1) {
		exit(1);
	}

	if (ws23xx_read_batch(fd, addr, nnyb, nel, buf) == -1) {
		goto error;
	}

	print_measures(addr, buf, nel, sep);

	ws_close(fd);
	return;

error:
	ws_close(fd);
	exit(1);
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

	ssize_t nel = ws23xx_fetch_history(fd, hbuf, hist_count);

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

static void
main_cron(int argc, char* const argv[]) {
	int c;

	/* Default values */
	int wunder = 1;
	const char *device = NULL;

	/* Parse sub-command arguments */
	while ((c = getopt(argc, argv, "W")) != -1) {
		switch (c) {
		case 'W':
			wunder = 0;
			break;

		default:
			usage_opt(stderr, c, 1);
			break;
		}
	}

	if (argc - 1 < optind) {
		usage(stderr, 1);
	}

	device = argv[optind++];

	/* Process sub-command */
	uint8_t cnx;
//	struct ws_wunder w;

	const struct ws_io a[] =
	{
//			{ "cn", &cnx },
//			{ "sw", &w.time },
//			{ "dp", &w.dew_point },
//			{ "ot", &w.temp },
//			{ "oh", &w.humidity },
//			{ "rh", &w.rain },
//			{ "rd", &w.daily_rain },
//			{ "w0", &w.wind_dir },
//			{ "ws", &w.wind_speed },
//			{ "it", &w.temp_in },
//			{ "ih", &w.humidity_in }
	};

	size_t nel = array_len(a);

	uint16_t addr[nel];
	size_t nnyb[nel];
	uint8_t *buf[nel];

	for (size_t i = 0; i < nel; i++) {
		const struct ws_io *io = &a[i];
		const struct ws_measure *m = search_id(io->id);

		addr[i] = m->addr;
		nnyb[i] = m->type->nybble;
		buf[i] = malloc(32);
	}

	int fd = ws_open(device);
	if (fd == -1) {
		exit(1);
	}

	ws23xx_read_batch(fd, addr, nnyb, nel, buf);

	ws_close(fd);

	for (size_t i = 0; i < nel; i++) {
		const struct ws_io *io = &a[i];
		const struct ws_measure *m = search_id(io->id);

		decode(buf[i], m->type->id, io->buf, 0);
	}

	char cbuf[18];
	char sep = ',';
	struct tm tm;

	switch (cnx) {
	case 0:				/* cable */
	case 15:			/* wireless */
//		localtime_r(&w.time, &tm);
//		strftime(cbuf, sizeof(cbuf), CSV_DATE, &tm);
//
//		fprintf(stdout, "%s%c%.2f%c%hhu%c%.2f%c%hu%c%.2f%c%.2f%c%.2f%c%.2f%c%d\n",
//				cbuf, sep,
//				w.temp, sep, w.humidity, sep, w.dew_point, sep,
//				w.wind_dir, sep, w.wind_speed, sep,
//				w.rain, sep, w.daily_rain, sep,
//				w.temp_in, sep, w.humidity_in);
//
//		if (wunder) {
//			ws_wunder_upload(&w);
//		}
		break;

	default:
		fprintf(stderr, "Connection: %x lost\n", cnx);
		exit(1);
		break;
	}
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
//			ws_io_delay = strtol(optarg, NULL, 10);
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
	} else if (strcmp("cron", cmd) == 0) {
		main_cron(argc, argv);
	} else if (strcmp("hex", cmd) == 0) {
		main_hex(argc, argv);
	}

	exit(0);
}
