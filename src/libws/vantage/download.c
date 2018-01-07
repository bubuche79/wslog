/**
 * Download commands.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if DEBUG
#include <stdio.h>
#endif
#include <sys/types.h>
#include <time.h>
#include <stdint.h>
#include <errno.h>

#include "defs/dso.h"

#include "libws/vantage/util.h"
#include "libws/vantage/vantage.h"

#define DMP_SIZE	52
#define PAGE_SIZE	265

int8_t
vantage_int8(const uint8_t *buf, uint16_t off)
{
	return buf[off];
}

uint8_t
vantage_uint8(const uint8_t *buf, uint16_t off)
{
	return buf[off];
}

static void
vantage_uint8_arr(const uint8_t *buf, uint16_t off, uint8_t *p, size_t nel)
{
	size_t i;

	for (i = 0; i < nel; i++) {
		p[i] = vantage_uint8(buf, off + i);
	}
}

int16_t
vantage_int16(const uint8_t *buf, uint16_t off)
{
	return (buf[off+1] << 8) | buf[off];
}

uint16_t
vantage_uint16(const uint8_t *buf, uint16_t off)
{
	return (buf[off+1] << 8) | buf[off];
}

time_t
vantage_mktime(const uint8_t *buf, uint16_t off, int wtime)
{
	struct tm tm;
	uint16_t date;

	date = vantage_uint16(buf, off + 0);

	tm.tm_year = 100 + (date >> 9);
	tm.tm_mon = ((date >> 5) & 0x0F) - 1;
	tm.tm_mday = date & 0x1F;

	if (wtime) {
		uint16_t time = vantage_uint16(buf, off + 2);

		tm.tm_hour = time / 100;
		tm.tm_min = time - (100 * tm.tm_hour);
	} else {
		tm.tm_hour = 0;
		tm.tm_min = 0;
	}

	tm.tm_sec = 0;
	tm.tm_isdst = -1;

	return mktime(&tm);
}

void
vantage_localtime(uint8_t *buf, time_t time)
{
	struct tm tm;
	uint16_t date, t;

	localtime_r(&time, &tm);

	date = ((tm.tm_year - 100) << 9) + ((tm.tm_mon + 1) << 5) + tm.tm_mday;
	t = (tm.tm_hour * 100) + tm.tm_min;

	buf[0] = date & 0xFF;
	buf[1] = date >> 8;
	buf[2] = t & 0xFF;
	buf[3] = t >> 8;
}

static void
vantage_dmp_decode(struct vantage_dmp *dmp, const uint8_t *buf)
{
#if 0
	uint8_t rec_type;
#endif

	dmp->tstamp = vantage_mktime(buf, 0, 1);
	dmp->temp = vantage_int16(buf, 4);
	dmp->hi_temp = vantage_int16(buf, 6);
	dmp->lo_temp = vantage_int16(buf, 8);
	dmp->rain = vantage_uint16(buf, 10);
	dmp->hi_rain_rate = vantage_uint16(buf, 12);
	dmp->barometer = vantage_uint16(buf, 14);
	dmp->solar_rad = vantage_uint16(buf, 16);
	dmp->wind_samples = vantage_uint16(buf, 18);
	dmp->in_temp = vantage_int16(buf, 20);
	dmp->in_humidity = vantage_uint8(buf, 22);
	dmp->humidity = vantage_uint8(buf, 23);
	dmp->avg_wind_speed = vantage_uint8(buf, 24);
	dmp->hi_wind_speed = vantage_uint8(buf, 25);
	dmp->hi_wind_dir = vantage_uint8(buf, 26);
	dmp->main_wind_dir = vantage_uint8(buf, 27);
	dmp->avg_uv = vantage_uint8(buf, 28);
	dmp->et = vantage_uint8(buf, 29);
	dmp->hi_solar_rad = vantage_uint16(buf, 30);
	dmp->hi_uv = vantage_uint8(buf, 32);
	dmp->forecast = vantage_uint8(buf, 33);

	vantage_uint8_arr(buf, 34, dmp->leaf_temp, 2);
	vantage_uint8_arr(buf, 36, dmp->leaf_wet, 2);
	vantage_uint8_arr(buf, 38, dmp->soil_temp, 4);

#if 0
	rec_type = vantage_uint8(buf, 42);
#endif

	vantage_uint8_arr(buf, 43, dmp->extra_humidity, 2);
	vantage_uint8_arr(buf, 45, dmp->extra_temp, 3);
	vantage_uint8_arr(buf, 48, dmp->soil_moisture, 4);
}

static int
vantage_read_page(int fd, uint8_t *buf, size_t len)
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
vantage_read_pages(int fd, struct vantage_dmp *p, size_t nel, int16_t offset)
{
	ssize_t sz;
	uint8_t byte;
	uint8_t buf[PAGE_SIZE];

	byte = ACK;

	/* Read all pages */
	for (sz = 0; sz < nel && byte == ACK; ) {
		size_t j;

		/* Read page */
		if (vantage_read_page(fd, buf, sizeof(buf)) == -1) {
			// TODO byte = ESC;
			goto error;
		}

		/* Decode page (5 records) */
		for (j = offset; j < 5 && sz < nel; j++) {
			uint8_t *r = buf + 1 + j * DMP_SIZE;

			if (r[0] == 0xFF) {
				byte = ESC;
			} else {
				vantage_dmp_decode(&p[sz++], r);
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
vantage_dmp(int fd, struct vantage_dmp *p, size_t nel)
{
	/* DMP command */
	if (vantage_proc(fd, DMP) == -1) {
		goto error;
	}

	return vantage_read_pages(fd, p, nel, 0);

error:
	return -1;
}

DSO_EXPORT ssize_t
vantage_dmpaft(int fd, struct vantage_dmp *p, size_t nel, time_t after)
{
	size_t sz;
	uint8_t buf[4];
	int16_t page_cnt, offset;

	/* DMPAFT command */
	if (vantage_proc(fd, DMPAFT) == -1) {
		goto error;
	}

	/* Send timestamp */
	vantage_localtime(buf, after);

	if (vantage_pwrite(fd, IO_CRC|IO_ACK, buf, sizeof(buf)) == -1) {
		goto error;
	}

	/* Read number of pages, record offset */
	if (vantage_pread(fd, IO_CRC, buf, sizeof(buf)) == -1) {
		goto error;
	}

	page_cnt = vantage_int16(buf, 0);
	offset = vantage_int16(buf, 2);

#if DEBUG
	printf("DMPAFT: %d pages, record offset %d\n", page_cnt, offset);
#endif

	/* Read archive records */
	if (page_cnt == 0) {
		sz = 0;
	} else {
		uint8_t byte = ACK;
		size_t rec_cnt = 5 * page_cnt - offset;

		if (nel < rec_cnt) {
			nel = rec_cnt;
		}

		/* ACK */
		if (vantage_write(fd, &byte, 1) == -1) {
			goto error;
		}

		if (vantage_read_pages(fd, p, nel, offset) == -1) {
			goto error;
		}

		sz = rec_cnt;
	}

	return sz;

error:
	return -1;
}
