#ifndef _SCONVERT_H
#define _SCONVERT_H

#include <stdint.h>
#include <time.h>

extern uint8_t ws_nybble(const uint8_t *buf, size_t offset);

extern double *ws_temp(const uint8_t *buf, double *v, size_t offset);
extern char *ws_temp_str(const uint8_t *buf, char *s, size_t len, size_t offset);

extern double *ws_pressure(const uint8_t *buf, double *v, size_t offset);
extern char *ws_pressure_str(const uint8_t *buf, char *s, size_t len, size_t offset);

extern uint8_t *ws_humidity(const uint8_t *buf, uint8_t *v, size_t offset);
extern char *ws_humidity_str(const uint8_t *buf, char *s, size_t len, size_t offset);

extern double *ws_rain(const uint8_t *buf, double *v, size_t offset);
extern char *ws_rain_str(const uint8_t *buf, char *s, size_t len, size_t offset);

extern double *ws_speed(const uint8_t *buf, double *v, size_t offset);
extern char *ws_speed_str(const uint8_t *buf, char *s, size_t len, size_t offset);

extern double *ws_get_wind_dir(const uint8_t *buf, double *v, size_t offset);

extern double *ws_get_wind_speed(const uint8_t *buf, double *v, size_t offset);

extern uint16_t *ws_interval_sec(const uint8_t *buf, uint16_t *v, size_t offset);
extern char *ws_interval_sec_str(const uint8_t *buf, char *s, size_t len, size_t offset);

extern uint16_t *ws_interval_min(const uint8_t *buf, uint16_t *v, size_t offset);
extern char *ws_interval_min_str(const uint8_t *buf, char *s, size_t len, size_t offset);

extern uint8_t *ws_bin_2nyb(const uint8_t *buf, uint8_t *v, size_t offset);
extern char *ws_bin_2nyb_str(const uint8_t *buf, char *s, size_t len, size_t offset);

extern time_t *ws_timestamp(const uint8_t *buf, time_t *v, size_t offset);
extern char *ws_timestamp_str(const uint8_t *buf, char *s, size_t len, size_t offset);

extern time_t *ws_datetime(const uint8_t *buf, time_t *v, size_t offset);
extern char *ws_datetime_str(const uint8_t *buf, char *s, size_t len, size_t offset);

extern char *ws_connection_str(const uint8_t *buf, char *s, size_t len, size_t offset);

extern char *ws_alarm_set_str(const uint8_t *buf, char *s, size_t len, size_t offset, uint8_t bit);
extern char *ws_alarm_active_str(const uint8_t *buf, char *s, size_t len, size_t offset, uint8_t bit);

#endif	/* _SCONVERT_H */
