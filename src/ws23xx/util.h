#ifndef _UTIL_H
#define _UTIL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

char *ws23xx_temp_str(const uint8_t *buf, char *s, size_t len, size_t offset);
char *ws23xx_pressure_str(const uint8_t *buf, char *s, size_t len, size_t offset);
char *ws23xx_humidity_str(const uint8_t *buf, char *s, size_t len, size_t offset);
char *ws23xx_rain_str(const uint8_t *buf, char *s, size_t len, size_t offset);
char *ws23xx_speed_str(const uint8_t *buf, char *s, size_t len, size_t offset);
char *ws23xx_wind_dir_str(const uint8_t *buf, char *s, size_t len, size_t offset);
char *ws23xx_interval_sec_str(const uint8_t *buf, char *s, size_t len, size_t offset);
char *ws23xx_interval_min_str(const uint8_t *buf, char *s, size_t len, size_t offset);
char *ws23xx_bin_2nyb_str(const uint8_t *buf, char *s, size_t len, size_t offset);
char *ws23xx_timestamp_str(const uint8_t *buf, char *s, size_t len, size_t offset);
char *ws23xx_datetime_str(const uint8_t *buf, char *s, size_t len, size_t offset);

#ifdef __cplusplus
}
#endif

#endif	/* _UTIL_H */
