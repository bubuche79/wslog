#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "defs/dso.h"

#include "libws/util.h"
#include "libws/nybble.h"
#include "libws/ws23xx/decoder.h"
#include "libws/ws23xx/ws23xx.h"
#include "libws/ws23xx/archive.h"

#define HIST_SIZE	19

static uint16_t
hist_addr(size_t i, uint8_t last_idx, uint8_t nel)
{
	uint16_t addr;

	if (i <= last_idx) {
		addr = last_idx - i;
	} else {
		addr = nel - (i - last_idx);
	}

	return 0x6c6 + addr * HIST_SIZE;
}

static void
hist_decode(const uint8_t *buf, struct ws23xx_ar *h)
{
	long v;

	v = nybtoul(buf, 5, 0, 16);
	h->temp_in = (v % 1000) / 10.0 - 30.0;
	h->temp = (v - (v % 1000)) / 10000.0 - 30.0;

	v = nybtoul(buf, 5, 5, 16);
	h->pressure = 1000 + (v % 10000) / 10.0;
	if (h->pressure > 1500) {
		h->pressure -= 1000;
	}
	h->humidity_in = (v - (v % 10000)) / 10000.0;

	h->humidity = nybtoul(buf, 2, 10, 10);
	h->rain = nybtoul(buf, 3, 12, 16) * 0.518;
	h->wind_speed = nybtoul(buf, 3, 15, 16) / 10.0;
	h->wind_dir = nybtoul(buf, 1, 18, 16) * 22.5;
}

DSO_EXPORT ssize_t
ws23xx_fetch_ar(int fd, struct ws23xx_ar *h, size_t nel)
{
	size_t i;
	uint8_t buf[10];

	/* Read history settings */
	uint16_t save_int;
	time_t last_sample;
	uint8_t last_record;
	uint8_t record_count;

	if (ws23xx_read_safe(fd, 0x6b2, 20, buf) == -1) {
		goto error;
	}

	ws23xx_interval_min(buf, &save_int, 0);
	ws23xx_timestamp(buf, &last_sample, 6);
	ws23xx_bin_2nyb(buf, &last_record, 16);
	ws23xx_bin_2nyb(buf, &record_count, 18);

	save_int++;

	/* Minimize number of reads */
	if (nel == 0) {
		nel = record_count;
	} else if (record_count < nel) {
		nel = record_count;
	}

	for (i = 0; i < nel; i++) {
		uint16_t addr;
		uint8_t nyb_data[10];
		struct ws23xx_ar *p = &h[nel-i-1];

		addr = hist_addr(i, last_record, record_count);

		if (ws23xx_read_safe(fd, addr, HIST_SIZE, nyb_data) == -1) {
			goto error;
		}

		hist_decode(nyb_data, p);

		/* Other fields */
		p->addr = addr;
		p->tstamp = last_sample - i * save_int * 60;
	}

	return nel;

error:
	return -1;
}
