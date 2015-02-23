#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "ws2300.h"
#include "history.h"

static void
conv_history(const uint8_t *buf, struct ws_history *h)
{
	long v;

	v = ((buf[2] & 0xF) << 16) + (buf[1] << 8) + buf[0];
	h->temp_in = (v % 1000) / 10.0 - 30.0;
	h->temp_out = (v - (v % 1000)) / 10000.0 - 30.0;

	v = (buf[4] << 12) + (buf[3] <<4 ) + (buf[2] >> 4);
	h->abs_pressure = (v % 10000) / 10.0;
	if (h->abs_pressure < 502.2) {
		h->abs_pressure += 1000;
	}
	h->humidity_in = (v - (v % 10000)) / 10000.0;

	h->humidity_out = (buf[5] >> 4) * 10 + (buf[5] & 0xF);
	h->rain = ((buf[7] & 0xF) * 256 + buf[6]) * 0.518;
	h->wind_speed = (buf[8] * 16 + (buf[7] >> 4))/ 10.0;
	h->wind_dir = (buf[9] & 0xF) * 22.5;
}

int
ws_fetch_history(int fd, struct ws_history *h, size_t nel) {
	uint8_t buf[20];

	/* Read history settings */
	if (ws_read_safe(fd, 0x6B2, 20, buf) == -1) {
		goto error;
	}

	uint16_t save_int;
	uint16_t count_down;
	time_t last_sample;
	uint8_t last_record;
	uint8_t record_count;

	ws_interval_min(buf, &save_int, 0);
	ws_interval_min(buf, &count_down, 3);
	ws_timestamp(buf, &last_sample, 6);
	ws_bin_2nyb(buf, &last_record, 16);
	ws_bin_2nyb(buf, &record_count, 18);

	/* Minimize number of reads */
	if (nel == 0) {
		nel = record_count;
	} else if (record_count < nel) {
		nel = record_count;
	}

//	for (int i = 0; i < nel; i++) {
//		uint8_t nyb_data[19];
//
//		if (ws_read_safe(fd, 0x6c6 + (last_record - i + 1) * 19, buf) == -1) {
//			goto error;
//		}
//
//		conv_history(nyb_data, &h[i]);
//	}
//
//	/* Display output */
//	for (int i = 0; i < nel; i++) {
//		struct ws_history = &h[i];
//
//		printf("%.2f,%.2f,%.1f,%d,%d,%.2f,%.1f,%.1f\n", h->temp_in, h->temp_out,
//				h->abs_pressure, h->humidity_in, h->humidity_out, h->rain,
//				h->wind_speed, h->wind_dir);
//	}

	return 0;

error:
	return -1;
}
