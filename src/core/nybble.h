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
nybat(const uint8_t *buf, size_t off)
{
	return (off & 0x1) ? buf[off / 2] >> 4 : buf[off / 2] & 0xF;
}

int nybtoi(const uint8_t *buf, size_t nnyb, size_t off);
int nybdtoi(const uint8_t *buf, size_t nnyb, size_t off);
long long nybtoll(const uint8_t *buf, size_t nnyb, size_t off);
long long nybdtoll(const uint8_t *buf, size_t nnyb, size_t off);

void nybcpy(uint8_t *dest, const uint8_t *src, size_t nnyb, size_t off);

#ifdef __cplusplus
}
#endif

#endif /* _CORE_NYBBLE_H */
