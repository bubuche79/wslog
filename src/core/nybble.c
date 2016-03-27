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
 * Convert nybbles to a long integer.
 */
DSO_EXPORT long int
nybtol(const uint8_t *buf, size_t nnyb, size_t off)
{
	int i;
	long int res;

	int sign_bit = nybat(buf, off) & 0x8;

	if (nnyb > 2 * sizeof(res)) {
		errno = ERANGE;
		res = sign_bit ? LONG_MIN : LONG_MAX;

		goto exit;
	}

	res = sign_bit ? -1 : 0;

	for (i = nnyb - 1; 0 <= i; i--) {
		res = (res << 4) | nybat(buf, off + i);
	}

exit:
	return res;
}

/**
 * Convert Binary Codded Decimal (BCD) nybbles to a long integer.
 */
DSO_EXPORT long int
nybdtol(const uint8_t *buf, size_t nnyb, size_t off)
{
	int i;
	long int res;

	res = 0;

	for (i = nnyb - 1; 0 <= i; i--) {
		uint8_t v = nybat(buf, off + i);

		if (v > 9) {
			errno = EINVAL;
			res = LONG_MAX;

			goto exit;
		} else {
			res = 10 * res + v;
		}
	}

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
