/*
 * Nybbles utilities.
 */

#include <errno.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>

#include "defs/dso.h"

#include "libws/nybble.h"

extern DSO_EXPORT uint8_t nybget(const uint8_t *buf, size_t off);
extern DSO_EXPORT void nybset(uint8_t *buf, size_t off, uint8_t v);

/**
 * Convert nybbles to an unsigned long integer.
 */
DSO_EXPORT unsigned long int
nybtoul(const uint8_t *buf, size_t nnyb, size_t off, int base)
{
	int i;
	unsigned long int res;
	unsigned long int limit;

	if (base < 2 || 16 < base) {
		errno = EINVAL;
		res = ULONG_MAX;
		goto exit;
	}

	res = 0;
	limit = ULONG_MAX / base;

	for (i = nnyb - 1; 0 <= i; i--) {
		uint8_t v = nybget(buf, off + i);

		if (base <= v) {
			errno = EINVAL;
			goto exit;
		} else if (limit < res) {
			errno = ERANGE;
			res = ULONG_MAX;
			goto exit;
		} else {
			res = base * res + v;
		}
	}

exit:
	return res;
}

/**
 * Convert an unsigned long integer to nybbles.
 */
DSO_EXPORT int
ultonyb(uint8_t *buf, size_t nnyb, size_t off, unsigned long int v, int base)
{
	size_t i;
	int res;

	res = -1;

	if (base < 2 || 16 < base) {
		errno = EINVAL;
		goto exit;
	}

	for (i = 0; i < nnyb; i++) {
		unsigned long int t = v / base;

		nybset(buf, off + i, v - base * t);
		v = t;
	}

	res = 0;

exit:
	return res;
}

/**
 * Copy nybble data.
 *
 * The #nybcpy() function copies #nyb nybbles from area #src, starting at
 * #offset offset (nybbles offset), to memory area #dest.
 */
DSO_EXPORT void
nybcpy(uint8_t *dest, const uint8_t *src, size_t nnyb, size_t off)
{
	src += off / 2;

	if (off & 0x1) {
		uint16_t i;

		for (i = 0; i < nnyb; i++) {
			int j = 1 + i;

			if (j & 0x1) {
				dest[i/2] = src[j/2] >> 4;
			} else {
				dest[i/2] |= src[j/2] << 4;
			}
		}
	} else {
		memcpy(dest, src, (nnyb + 1) / 2);
	}
}

DSO_EXPORT void
nybprint(uint16_t addr, const uint8_t *buf, size_t nnyb, int hex)
{
	int disp_sz;
	uint16_t i, j;

	if (hex) {
		disp_sz = 2;
	} else {
		disp_sz = 1;
	}

	for (i = 0; i < nnyb;) {
		printf("%.4x", addr + i);

		for (j = 0; j < 16 && i < nnyb; j++) {
			uint8_t v;

			if (hex) {
				v = buf[i / 2];
			} else {
				v = nybget(buf, i);
			}

			printf(" %.*x", disp_sz, v);

			i += disp_sz;
		}

		printf("\n");
	}
}
