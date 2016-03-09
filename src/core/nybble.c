/*
 * Nybbles utilities.
 */

#include <errno.h>
#include <string.h>
#include <limits.h>

#include "defs/dso.h"

#include "nybble.h"

extern DSO_EXPORT uint8_t nybat(const uint8_t *buf, size_t off);

/**
 * Convert nybbles to an integer.
 */
DSO_EXPORT int
nybtoi(const uint8_t *buf, size_t nnyb, size_t off)
{
	int i;
	int res = 0;

	if (nnyb > 8) {
		goto error;
	}

	for (i = nnyb - 1; 0 <= i; i--) {
		res = 16 * res + nybat(buf, off + i);
	}

	return res;

error:
	return -1;
}

/**
 * Convert Binary Codded Decimal (BCD) nybbles to an integer.
 */
DSO_EXPORT int
nybdtoi(const uint8_t *buf, size_t nnyb, size_t off)
{
	int i;

	int err = 0;
	int res = 0;

	for (i = nnyb - 1; err == 0 && 0 <= i; i--) {
		uint8_t v = nybat(buf, off + i);

		if (v > 9) {
			err = EINVAL;
			res = INT_MIN;
		} else {
			res = 10 * res + v;
		}
	}

	if (err) {
		errno = err;
	}

	return res;
}

/**
 * Convert nybbles to a long integer.
 */
DSO_EXPORT long long
nybtoll(const uint8_t *buf, size_t nnyb, size_t off)
{
	int i;
	long long res = 0;

	for (i = nnyb - 1; 0 <= i; i--) {
		res = 16 * res + nybat(buf, off + i);
	}

	return res;
}

/**
 * Convert Binary Codded Decimal (BCD) nybbles to a long integer.
 */
DSO_EXPORT long long
nybdtoll(const uint8_t *buf, size_t nnyb, size_t off)
{
	int i;

	int err = 0;
	long long res = 0;

	for (i = nnyb - 1; err == 0 && 0 <= i; i--) {
		uint8_t v = nybat(buf, off + i);

		if (v > 9) {
			err = EINVAL;
			res = LLONG_MIN;
		} else {
			res = 10 * res + v;
		}
	}

	if (err) {
		errno = err;
	}

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
		for (uint16_t i = 0; i < nnyb; i++) {
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
