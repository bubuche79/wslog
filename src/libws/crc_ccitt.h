#ifndef _LIBWS_CRC_CCITT_H
#define _LIBWS_CRC_CCITT_H

#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

uint16_t ws_crc_ccitt(const uint8_t *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif	/* _LIBWS_CRC_CCITT_H */
