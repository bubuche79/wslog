#ifndef _LIBWS23XX_DECODER_H
#define _LIBWS23XX_DECODER_H

#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

uint8_t ws23xx_bit(const uint8_t *buf, size_t offset, uint8_t bit);

float ws23xx_temp(const uint8_t *buf, float *v, size_t offset);
float ws23xx_pressure(const uint8_t *buf, float *v, size_t offset);
uint8_t ws23xx_humidity(const uint8_t *buf, uint8_t *v, size_t offset);
float ws23xx_rain(const uint8_t *buf, float *v, size_t offset);
float ws23xx_speed(const uint8_t *buf, float *v, size_t offset);
uint16_t ws23xx_wind_dir(const uint8_t *buf, uint16_t *v, size_t offset);
uint8_t ws23xx_wind_valid(const uint8_t *buf, uint8_t *v, size_t offset);
float ws23xx_interval_sec(const uint8_t *buf, float *v, size_t offset);
uint16_t ws23xx_interval_min(const uint8_t *buf, uint16_t *v, size_t offset);
uint8_t ws23xx_bin_2nyb(const uint8_t *buf, uint8_t *v, size_t offset);
time_t ws23xx_timestamp(const uint8_t *buf, time_t *v, size_t offset);
time_t ws23xx_datetime(const uint8_t *buf, time_t *v, size_t offset);
uint8_t ws23xx_connection(const uint8_t *buf, uint8_t *v, size_t offset);
uint8_t ws23xx_alarm_set(const uint8_t *buf, uint8_t *v, size_t offset, uint8_t bit);
uint8_t ws23xx_alarm_active(const uint8_t *buf, uint8_t *v, size_t offset, uint8_t bit);

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

#endif	/* _LIBWS23XX_DECODER_H */
