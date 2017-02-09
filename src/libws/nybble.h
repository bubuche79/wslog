#ifndef _CORE_NYBBLE_H
#define _CORE_NYBBLE_H

#include <stdint.h>
#include <stddef.h>

/*
 * Nybble utilities.
 */

#ifdef __cplusplus
extern "C" {
#endif

inline uint8_t
nybget(const uint8_t *buf, size_t off)
{
	return (off & 0x1) ? buf[off / 2] >> 4 : buf[off / 2] & 0xF;
}

inline void
nybset(uint8_t *buf, size_t off, uint8_t v)
{
	size_t nyboff = off / 2;

	if (off & 0x1) {
		buf[nyboff] = ((v & 0xF) << 4) | (buf[nyboff] & 0x0F);
	} else {
		buf[nyboff] = (v & 0xF) | (buf[nyboff] & 0xF0);
	}
}

unsigned long int nybtoul(const uint8_t *buf, size_t nnyb, size_t off, int base);
int ultonyb(uint8_t *buf, size_t nnyb, size_t off, unsigned long int v, int base);

void nybcpy(uint8_t *dest, const uint8_t *src, size_t nnyb, size_t off);
void nybprint(uint16_t addr, const uint8_t *buf, size_t nnyb, int hex);

#ifdef __cplusplus
}
#endif

#endif /* _CORE_NYBBLE_H */
