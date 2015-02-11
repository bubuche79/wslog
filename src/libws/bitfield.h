#ifndef _CORE_BITFIELD_H
#define _CORE_BITFIELD_H

#include <stdint.h>

/*
 * Bitfield utilities.
 *
 * The first byte of the bitfield corresponds to indices 0 - 7 from high bit
 * to low bit, respectively. The next one to indices 8 - 15, etc.
 */

#ifdef __cplusplus
extern "C" {
#endif

inline void
bfset(uint8_t *buf, size_t off)
{
	size_t byte_idx;
	uint8_t byte_mask;

	byte_idx = off / 8;
	byte_mask = 1 << (7 - (off - 8 * byte_idx));

	buf[byte_idx] |= byte_mask;
}

inline void
bfclear(uint8_t *buf, size_t off)
{
	size_t byte_idx;
	uint8_t byte_mask;

	byte_idx = off / 8;
	byte_mask = 1 << (7 - (off - 8 * byte_idx));

	buf[byte_idx] &= ~byte_mask;
}

inline int
bftest(const uint8_t *buf, size_t off)
{
	size_t byte_idx;
	uint8_t byte_mask;

	byte_idx = off / 8;
	byte_mask = 1 << (7 - (off - 8 * byte_idx));

	return buf[byte_idx] & byte_mask;
}

void bfsetall(uint8_t *bfbuf, size_t off, size_t nbits);
void bfclearall(uint8_t *bfbuf, size_t off, size_t nbits);
int bftestall(const uint8_t *bfbuf, size_t off, size_t nbits);

#ifdef __cplusplus
}
#endif

#endif /* _CORE_BITFIELD_H */
