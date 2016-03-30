/*
 * Bitfield utilities.
 *
 * When working on bit fields (not only one bit), the offset and the number 
 * of bits to work with may cross "full" bytes.
 */

#include <stdint.h>
#include <sys/types.h>
#include <string.h>

#include "defs/dso.h"

#include "libws/bitfield.h"

extern DSO_EXPORT void bfset(uint8_t *buf, size_t off);
extern DSO_EXPORT void bfclear(uint8_t *buf, size_t off);
extern DSO_EXPORT int bftest(const uint8_t *buf, size_t off);

static void
setval_all(uint8_t *bfbuf, size_t off, size_t nbits, int v)
{
	size_t byte_idx;
	size_t bit_idx;
	uint8_t byte_mask;

	byte_idx = off / 8;
	bit_idx = off - 8 * byte_idx;

	bfbuf += byte_idx;

	if (nbits > 0 && bit_idx > 0) {
		if (8 < bit_idx + nbits) {
			byte_mask = (1 << (8 - bit_idx)) - 1;
			nbits -= 8 - bit_idx;
		} else {
			byte_mask = ((1 << nbits) - 1) << (8 - bit_idx - nbits);
			nbits = 0;
		}

		if (v) {
			*bfbuf++ |= byte_mask;
		} else {
			*bfbuf++ &= ~byte_mask;
		}
	}

	if (nbits > 7) {
		if (v) {
			memset(bfbuf, ~0, nbits / 8);
		} else {
			memset(bfbuf, 0, nbits / 8);
		}

		bfbuf += nbits / 8;
		nbits &= 0x07;
	}

	if (nbits > 0) {
		byte_mask = ~((1 << (8 - nbits)) - 1);

		if (v) {
			*bfbuf |= byte_mask;
		} else {
			*bfbuf &= ~byte_mask;
		}
	}
}

DSO_EXPORT void
bfsetall(uint8_t *bfbuf, size_t off, size_t nbits)
{
	setval_all(bfbuf, off, nbits, 1);
}

DSO_EXPORT void
bfclearall(uint8_t *bfbuf, size_t off, size_t nbits)
{
	setval_all(bfbuf, off, nbits, 0);
}

DSO_EXPORT int
bftestall(const uint8_t *bfbuf, size_t off, size_t nbits)
{
	size_t byte_idx;
	size_t bit_idx;
	uint8_t byte_mask;

	byte_idx = off / 8;
	bit_idx = off - 8 * byte_idx;

	bfbuf += byte_idx;

	if (nbits > 0 && bit_idx > 0) {
		if (8 < bit_idx + nbits) {
			byte_mask = (1 << (8 - bit_idx)) - 1;
			nbits -= 8 - bit_idx;
		} else {
			byte_mask = ((1 << nbits) - 1) << (8 - bit_idx - nbits);
			nbits = 0;
		}

		if ((*bfbuf++ & byte_mask) != byte_mask) {
			return 0;
		}
	}

	for (; nbits > 7; nbits -= 8) {
		if (*bfbuf++ != 0xFF) {
			return 0;
		}
	}

	if (nbits > 0) {
		byte_mask = ~((1 << (8 - nbits)) - 1);

		if ((*bfbuf & byte_mask) != byte_mask) {
			return 0;
		}
	}

	return 1;
}
