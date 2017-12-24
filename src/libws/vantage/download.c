/**
 * Download commands.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <time.h>
#include <stdint.h>
#include <errno.h>

#include "defs/dso.h"

#include "libws/vantage/util.h"
#include "libws/vantage/vantage.h"

#define DMP_SIZE	52
#define PAGE_SIZE	264

static uint8_t
vantage_uint8(const uint8_t *buf)
{
	return buf[0];
}

static void
vantage_uint8_arr(const uint8_t *buf, size_t nel, uint8_t *arr)
{
	size_t i;

	for (i = 0; i < nel; i++) {
		arr[i] = vantage_uint8(buf + i);
	}
}

static uint16_t
vantage_uint16(const uint8_t *buf)
{
	return (buf[1] << 8) | buf[0];
}

time_t
vantage_mktime(const uint8_t *buf)
{
	struct tm tm;
	uint16_t date, time;

	date = vantage_uint16(buf + 0);
	time = vantage_uint16(buf + 2);

	tm.tm_year = date >> 9;
	tm.tm_mon = ((date >> 5) & 0x0F) - 1;
	tm.tm_mday = date & 0x1F;

	tm.tm_hour = time / 100;
	tm.tm_min = time - (100 * tm.tm_hour);
	tm.tm_sec = 0;

	tm.tm_isdst = -1;

	return mktime(&tm);
}

void
vantage_time(uint8_t *buf, time_t time)
{
	struct tm tm;
	uint16_t date, t;

	localtime_r(&time, &tm);

	date = (tm.tm_year << 9) + ((tm.tm_mon + 1) << 5) + tm.tm_mday;
	t = (tm.tm_hour * 100) + tm.tm_min;

	buf[0] = date & 0xFF;
	buf[1] = date >> 8;
	buf[2] = t & 0xFF;
	buf[3] = t >> 8;
}

static void
vantage_dmp_decode(struct vantage_dmp *d, const uint8_t *buf)
{
#if 0
	uint8_t rec_type;
#endif

	d->tstamp = vantage_mktime(buf + 0);
	d->temp = vantage_uint16(buf + 4);
	d->hi_temp = vantage_uint16(buf + 6);
	d->lo_temp = vantage_uint16(buf + 8);
	d->rain = vantage_uint16(buf + 10);
	d->hi_rain_rate = vantage_uint16(buf + 12);
	d->barometer = vantage_uint16(buf + 14);
	d->solar_rad = vantage_uint16(buf + 16);
	d->wind_samples = vantage_uint16(buf + 18);
	d->in_temp = vantage_uint16(buf + 20);
	d->in_humidity = vantage_uint8(buf + 22);
	d->humidity = vantage_uint8(buf + 23);
	d->avg_wind_speed = vantage_uint8(buf + 24);
	d->hi_wind_speed = vantage_uint8(buf + 25);
	d->hi_wind_dir = vantage_uint8(buf + 26);
	d->main_wind_dir = vantage_uint8(buf + 27);
	d->avg_uv = vantage_uint8(buf + 28);
	d->et = vantage_uint8(buf + 29);
	d->hi_solar_rad = vantage_uint16(buf + 30);
	d->hi_uv = vantage_uint8(buf + 32);
	d->forecast = vantage_uint8(buf + 33);

	vantage_uint8_arr(buf + 34, 2, d->leaf_temp);
	vantage_uint8_arr(buf + 36, 2, d->leaf_wet);
	vantage_uint8_arr(buf + 38, 4, d->soil_temp);

#if 0
	rec_type = vantage_uint8(buf + 42);
#endif

	vantage_uint8_arr(buf + 43, 2, d->extra_humidity);
	vantage_uint8_arr(buf + 45, 3, d->extra_temp);
	vantage_uint8_arr(buf + 48, 4, d->soil_moisture);
}

static int
vantage_read_page(int fd, uint8_t *buf, uint8_t len)
{
	int i;

	for (i = 0; i < 3; i++) {
		int ret = vantage_pread(fd, IO_CRC, buf, len);

		if (ret == 0) {
			return 0;
		} else if (ret == -1) {
			// TODO use right errno
			if (errno == -1) {
				uint8_t nack = NACK;

				/* Send the page again */
				if (vantage_write(fd, &nack, 1) == -1) {
					goto error;
				}
			} else {
				goto error;
			}
		}
	}

	errno = EIO;

error:
	return -1;
}

DSO_EXPORT ssize_t
vantage_read_pages(int fd, struct vantage_dmp *buf, size_t nel, int16_t npages, int16_t offset)
{
	ssize_t sz;
	uint8_t byte;
	uint8_t iobuf[PAGE_SIZE];

	byte = ACK;

	for (sz = 0; sz < nel && byte == ACK; ) {
		size_t j;

		/* Read page */
		if (vantage_read_page(fd, iobuf, byte) == -1) {
			// TODO byte = ESC;
			goto error;
		}

		/* Decode page (5 records) */
		for (j = offset; j < 5 && sz < nel; j++) {
			uint8_t *p = iobuf + 1 + j * DMP_SIZE;

			if (p[0] == 0xFF) {
				byte = ESC;
			} else {
				vantage_dmp_decode(&buf[sz++], p);
			}
		}

		/* Send ACK or ESC */
		if (vantage_write(fd, &byte, 1) == -1) {
			goto error;
		}

		/* Clear offset for next pages */
		offset = 0;
	}

	return sz;

error:
	return -1;
}

DSO_EXPORT ssize_t
vantage_dmp(int fd, struct vantage_dmp *buf, size_t nel)
{
	ssize_t sz;
	uint8_t byte;
	uint8_t iobuf[PAGE_SIZE];

	byte = ACK;

	/* DMP command */
	if (vantage_proc(fd, DMP) == -1) {
		goto error;
	}

	// TODO Do we need to send ACK?

	/* Read archive records */
	for (sz = 0; sz < nel && byte == ACK; ) {
		size_t j;

		/* Read page */
		if (vantage_read_page(fd, iobuf, byte) == -1) {
			// TODO byte = ESC;
			goto error;
		}

		/* Decode page (5 records) */
		for (j = 0; j < 5 && sz < nel; j++) {
			uint8_t *p = iobuf + 1 + j * DMP_SIZE;

			if (p[0] == 0xFF) {
				byte = ESC;
			} else {
				vantage_dmp_decode(&buf[sz++], p);
			}
		}

		/* Send ACK or ESC */
		if (vantage_write(fd, &byte, 1) == -1) {
			goto error;
		}
	}

	return sz;

error:
	return -1;
}

DSO_EXPORT ssize_t
vantage_dmpaft(int fd, struct vantage_dmp *buf, size_t nel, time_t after)
{
	uint8_t iobuf[4];
	int16_t page_cnt, offset;

	/* DMPAFT command */
	if (vantage_proc(fd, DMPAFT) == -1) {
		goto error;
	}

	/* Send timestamp */
	vantage_time(iobuf, after);

	if (vantage_pwrite(fd, IO_CRC|IO_ACK, iobuf, sizeof(iobuf)) == -1) {
		goto error;
	}

	/* Read number of pages, record offset */
	if (vantage_pread(fd, IO_CRC, iobuf, sizeof(iobuf)) == -1) {
		goto error;
	}

	page_cnt = vantage_uint16(iobuf);
	offset = vantage_uint16(iobuf + 2);

	/* Read archive records */
	return vantage_read_pages(fd, buf, nel, page_cnt, offset);

error:
	return -1;
}
