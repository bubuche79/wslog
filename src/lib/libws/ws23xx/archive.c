#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "defs/dso.h"

#include "libws/util.h"
#include "libws/ws23xx/decoder.h"
#include "libws/ws23xx/ws23xx.h"
#include "libws/ws23xx/archive.h"

static void
conv_history(const uint8_t *buf, struct ws23xx_ar *h)
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
	h->wind_speed = (buf[8] * 16 + (buf[7] >> 4)) / 10.0;
	h->wind_dir = (buf[9] & 0xF) * 22.5;
}

DSO_EXPORT ssize_t
ws23xx_fetch_ar(int fd, struct ws23xx_ar *h, size_t nel)
{
	size_t i;
	uint8_t buf[20];

	/* Read history settings */
	uint16_t save_int;
	uint16_t count_down;
	time_t last_sample;
	uint8_t last_record;
	uint8_t record_count;

	if (ws23xx_read_safe(fd, 0x6B2, 20, buf) == -1) {
		goto error;
	}

	ws23xx_interval_min(buf, &save_int, 0);
	ws23xx_interval_min(buf, &count_down, 3);
	ws23xx_timestamp(buf, &last_sample, 6);
	ws23xx_bin_2nyb(buf, &last_record, 16);
	ws23xx_bin_2nyb(buf, &record_count, 18);

	/* Minimize number of reads */
	if (nel == 0) {
		nel = record_count;
	} else if (record_count < nel) {
		nel = record_count;
	}

	for (i = 0; i < nel; i++) {
		uint8_t nyb_data[19];
		struct ws23xx_ar *p = &h[nel-i-1];

		if (ws23xx_read_safe(fd, 0x6c6 + (last_record - i) * 19, 19, nyb_data) == -1) {
			goto error;
		}

		conv_history(nyb_data, p);

		/* Compute timestamp */
		p->tstamp = last_sample - i * save_int * 60;
	}

	return nel;

error:
	return -1;
}
