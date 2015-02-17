#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

#include "serial.h"
#include "ws2300.h"

#define PROGNAME	"ws2300"
#define ZERO		0x000

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
	{ 0x02c, None, conv_ala2, "wind speed min alarm active alias" },
	{ 0x200, "st", conv_time, "station set time", "ct" },
	{ 0x23b, "sw", conv_dtme, "station current date time" },
	{ 0x24d, "sd", conv_date, "station set date", "cd" },
	{ 0x266, "lc", conv_lcon, "lcd contrast (ro)" },
	{ 0x26b, "for", conv_fore, "forecast" },
	{ 0x26c, "ten", conv_tend, "tendency" }, */
	{ 0x346, "it", WS_TEMP, "in temp" },
	{ 0x34b, "itl", WS_TEMP, "in temp min", "it" },
	{ 0x350, "ith", WS_TEMP, "in temp max", "it" },
/*	{ 0x354, "itlw", conv_stmp, "in temp min when", "sw" },
	{ 0x35e, "ithw", conv_stmp, "in temp max when", "sw" }, */
	{ 0x369, "itla", WS_TEMP, "in temp min alarm" },
	{ 0x36e, "itha", WS_TEMP, "in temp max alarm" },
	{ 0x373, "ot", WS_TEMP, "out temp" },
	{ 0x378, "otl", WS_TEMP, "out temp min", "ot" },
	{ 0x37d, "oth", WS_TEMP, "out temp max", "ot" },
/*	{ 0x381, "otlw", conv_stmp, "out temp min when", "sw" },
	{ 0x38b, "othw", conv_stmp, "out temp max when", "sw" }, */
	{ 0x396, "otla", WS_TEMP, "out temp min alarm" },
	{ 0x39b, "otha", WS_TEMP, "out temp max alarm" },
	{ 0x3a0, "wc", WS_TEMP, "wind chill" },
	{ 0x3a5, "wcl", WS_TEMP, "wind chill min", "wc" },
	{ 0x3aa, "wch", WS_TEMP, "wind chill max", "wc" },
/*	{ 0x3ae, "wclw", conv_stmp, "wind chill min when", "sw" },
	{ 0x3b8, "wchw", conv_stmp, "wind chill max when", "sw" }, */
	{ 0x3c3, "wcla", WS_TEMP, "wind chill min alarm" },
	{ 0x3c8, "wcha", WS_TEMP, "wind chill max alarm" },
	{ 0x3ce, "dp", WS_TEMP, "dew point" },
	{ 0x3d3, "dpl", WS_TEMP, "dew point min", "dp" },
	{ 0x3d8, "dph", WS_TEMP, "dew point max", "dp" },
/*	{ 0x3dc, "dplw", conv_stmp, "dew point min when", "sw" },
	{ 0x3e6, "dphw", conv_stmp, "dew point max when", "sw" }, */
	{ 0x3f1, "dpla", WS_TEMP, "dew point min alarm" },
	{ 0x3f6, "dpha", WS_TEMP, "dew point max alarm" },
/*	{ 0x3fb, "ih", conv_humi, "in humidity" },
	{ 0x3fd, "ihl", conv_humi, "in humidity min", "ih" },
	{ 0x3ff, "ihh", conv_humi, "in humidity max", "ih" },
	{ 0x401, "ihlw", conv_stmp, "in humidity min when", "sw" },
	{ 0x40b, "ihhw", conv_stmp, "in humidity max when", "sw" },
	{ 0x415, "ihla", conv_humi, "in humidity min alarm" },
	{ 0x417, "ihha", conv_humi, "in humidity max alarm" },
	{ 0x419, "oh", conv_humi, "out humidity" },
	{ 0x41b, "ohl", conv_humi, "out humidity min", "oh" },
	{ 0x41d, "ohh", conv_humi, "out humidity max", "oh" },
	{ 0x41f, "ohlw", conv_stmp, "out humidity min when", "sw" },
	{ 0x429, "ohhw", conv_stmp, "out humidity max when", "sw" },
	{ 0x433, "ohla", conv_humi, "out humidity min alarm" },
	{ 0x435, "ohha", conv_humi, "out humidity max alarm" },
	{ 0x497, "rd", conv_rain, "rain 24h" },
	{ 0x49d, "rdh", conv_rain, "rain 24h max", "rd" },
	{ 0x4a3, "rdhw", conv_stmp, "rain 24h max when", "sw" },
	{ 0x4ae, "rdha", conv_rain, "rain 24h max alarm" },
	{ 0x4b4, "rh", conv_rain, "rain 1h" },
	{ 0x4ba, "rhh", conv_rain, "rain 1h max", "rh" },
	{ 0x4c0, "rhhw", conv_stmp, "rain 1h max when", "sw" },
	{ 0x4cb, "rhha", conv_rain, "rain 1h max alarm" },
	{ 0x4d2, "rt", conv_rain, "rain total", 	reset=0)
	{ 0x4d8, "rtrw", conv_stmp, "rain total reset when", "sw" },
	{ 0x4ee, "wsl", conv_wspd, "wind speed min", "ws" },
	{ 0x4f4, "wsh", conv_wspd, "wind speed max", "ws" },
	{ 0x4f8, "wslw", conv_stmp, "wind speed min when", "sw" },
	{ 0x502, "wshw", conv_stmp, "wind speed max when", "sw" },
	{ 0x527, "wso", conv_wovr, "wind speed overflow" },
	{ 0x528, "wsv", conv_wvld, "wind speed validity" },
	{ 0x529, "wv", conv_wvel, "wind velocity" },
	{ 0x529, "ws", conv_wspd, "wind speed" },
	{ 0x52c, "w0", conv_wdir, "wind direction" },
	{ 0x52d, "w1", conv_wdir, "wind direction 1" },
	{ 0x52e, "w2", conv_wdir, "wind direction 2" },
	{ 0x52f, "w3", conv_wdir, "wind direction 3" },
	{ 0x530, "w4", conv_wdir, "wind direction 4" },
	{ 0x531, "w5", conv_wdir, "wind direction 5" },
	{ 0x533, "wsla", conv_wspd, "wind speed min alarm" },
	{ 0x538, "wsha", conv_wspd, "wind speed max alarm" },
	{ 0x54d, "cn", conv_conn, "connection type" },
	{ 0x54f, "cc", conv_per2, "connection time till connect" },
	{ 0x5d8, "pa", conv_pres, "pressure absolute" },
	{ 0x5e2, "pr", conv_pres, "pressure relative" },
	{ 0x5ec, "pc", conv_pres, "pressure correction" },
	{ 0x5f6, "pal", conv_pres, "pressure absolute min", "pa" },
	{ 0x600, "prl", conv_pres, "pressure relative min", "pr" },
	{ 0x60a, "pah", conv_pres, "pressure absolute max", "pa" },
	{ 0x614, "prh", conv_pres, "pressure relative max", "pr" },
	{ 0x61e, "plw", conv_stmp, "pressure min when", "sw" },
	{ 0x628, "phw", conv_stmp, "pressure max when", "sw" },
	{ 0x63c, "pla", conv_pres, "pressure min alarm" },
	{ 0x650, "pha", conv_pres, "pressure max alarm" },
	{ 0x6b2, "hi", conv_per3, "history interval" },
	{ 0x6b5, "hc", conv_per3, "history time till sample" },
	{ 0x6b8, "hw", conv_stmp, "history last sample when" },
	{ 0x6c2, "hp", conv_rec2, "history last record pointer",reset=0)
	{ 0x6c4, "hn", conv_rec2, "history number of records", 0) */
	{ ZERO }
};

static void
usage(FILE *out, int code)
{
	fprintf(out, "Usage: %s\n device", PROGNAME);

	exit(code);
}

static const struct ws_measure *
find_measure(const char *id)
{
	const struct ws_measure *res = NULL;
	const struct ws_measure *p = &mem_map[0];

	for (; p->addr != ZERO && res == NULL; p++) {
		if (strcmp(id, p->id) == 0) {
			res = p;
		}
	}

	return res;
}

int
main(int argc, char** argv)
{
	int c;
	int errflg = 0;

	const char *device = NULL;
	const char *id = NULL;

	/* Parse arguments */
	while ((c = getopt(argc, argv, "h")) != -1) {
		switch (c) {
		case 'h':
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
	id = argv[optind++];

	/* Loading data */
	int fd = ws_open(device);
	const struct ws_measure *mx = find_measure(id);

	uint8_t data[20];
	char out[16];

	ws_read_safe(fd, mx->addr, ws_conv_temp->nybble_count, data);
	ws_get_temp_str(data, out, sizeof(out));
	printf("%s\n", out);

	ws_close(fd);

	exit(0);
}

