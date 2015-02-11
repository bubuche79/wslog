#ifndef _LIBWS23XX_DECODER_H
#define _LIBWS23XX_DECODER_H

#include <stdint.h>
#include <time.h>

enum {
	WS23XX_TEMP,
	WS23XX_PRESSURE,
	WS23XX_HUMIDITY,
	WS23XX_RAIN,
	WS23XX_WIND_DIR,
	WS23XX_WIND_VELOCITY,
	WS23XX_SPEED,
	WS23XX_INT_SEC,
	WS23XX_INT_MIN,
	WS23XX_BIN_2NYB,
	WS23XX_CONTRAST,
	WS23XX_DATE,							/* write only */
	WS23XX_TIMESTAMP,
	WS23XX_DATETIME,
	WS23XX_TIME,							/* write only */
	WS23XX_CONNECTION,
	WS23XX_FORECAST,
	WS23XX_TENDENCY,
	WS23XX_SPEED_UNIT,
	WS23XX_WIND_OVERFLOW,
	WS23XX_WIND_VALID,
	WS23XX_ALARM_SET_0,
	WS23XX_ALARM_SET_1,
	WS23XX_ALARM_SET_2,
	WS23XX_ALARM_SET_3,
	WS23XX_ALARM_ACTIVE_0,
	WS23XX_ALARM_ACTIVE_1,
	WS23XX_ALARM_ACTIVE_2,
	WS23XX_ALARM_ACTIVE_3,
	WS23XX_BUZZER,
	WS23XX_BACKLIGHT
};

#ifdef __cplusplus
extern "C" {
#endif

uint8_t ws23xx_bit(const uint8_t *buf, uint8_t *v, size_t offset, uint8_t bit);
uint8_t ws23xx_byte(const uint8_t *buf, uint8_t *v, size_t offset);

float ws23xx_temp(const uint8_t *buf, float *v, size_t offset);
float ws23xx_pressure(const uint8_t *buf, float *v, size_t offset);
uint8_t ws23xx_humidity(const uint8_t *buf, uint8_t *v, size_t offset);
float ws23xx_rain(const uint8_t *buf, float *v, size_t offset);
float ws23xx_speed(const uint8_t *buf, float *v, size_t offset);
uint16_t ws23xx_wind_dir(const uint8_t *buf, uint16_t *v, size_t offset);
uint8_t ws23xx_wind_valid(const uint8_t *buf, uint8_t *v, size_t offset);
uint8_t ws23xx_wind_overflow(const uint8_t *buf, uint8_t *v, size_t offset);
float ws23xx_interval_sec(const uint8_t *buf, float *v, size_t offset);
uint16_t ws23xx_interval_min(const uint8_t *buf, uint16_t *v, size_t offset);
uint8_t ws23xx_bin_2nyb(const uint8_t *buf, uint8_t *v, size_t offset);
time_t ws23xx_timestamp(const uint8_t *buf, time_t *v, size_t offset);
time_t ws23xx_datetime(const uint8_t *buf, time_t *v, size_t offset);
uint8_t ws23xx_connection(const uint8_t *buf, uint8_t *v, size_t offset);
uint8_t ws23xx_alarm_set(const uint8_t *buf, uint8_t *v, size_t offset, uint8_t bit);
uint8_t ws23xx_alarm_active(const uint8_t *buf, uint8_t *v, size_t offset, uint8_t bit);

#ifdef __cplusplus
}
#endif

#endif	/* _LIBWS23XX_DECODER_H */
