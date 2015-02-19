#ifndef _SCONVERT_H
#define _SCONVERT_H

#include <stdint.h>
#include <time.h>

enum ws_type
{
	WS_TEMP,
	WS_PRESSURE,
	WS_HUMIDITY,
	WS_RAIN,
	WS_SPEED,
	WS_WIND_DIR,
	WS_WIND_VELOCITY,
	WS_INT_SEC,
	WS_INT_MIN,
	WS_DATE,
	WS_TIMESTAMP,
	WS_DATETIME,
	WS_TIME
};

struct ws_conv
{
	char *units;				/* units name (eg hPa) */
	uint8_t nybbles;			/* nybbles count */
	char descr[64];				/* units description */

	union
	{
		struct
		{
			uint8_t scale;		/* value scale */
			int offset;			/* value offset */
		} bcd;

		struct
		{
			uint8_t scale;		/* value scale */
			int multi;			/* multiplicity factor */
		} bin;

		struct
		{
			char format[16];	/* date, timestamp format */
		} tm;
	};
};

extern const struct ws_conv *ws_get_conv(enum ws_type t);

extern double ws_get_temp(const uint8_t *buf);
extern char *ws_temp_str(const uint8_t *buf, char *str, size_t len);

extern double ws_get_pressure(const uint8_t *buf);
extern char *ws_pressure_str(const uint8_t *buf, char *str, size_t len);

extern double ws_get_humidity(const uint8_t *buf);
extern char *ws_humidity_str(const uint8_t *buf, char *str, size_t len);

extern double ws_get_rain(const uint8_t *buf);
extern char *ws_rain_str(const uint8_t *buf, char *str, size_t len);

extern double ws_get_speed(const uint8_t *buf);
extern char *ws_speed_str(const uint8_t *buf, char *str, size_t len);

extern double ws_get_wind_dir(const uint8_t *buf);

extern double ws_get_wind_speed(const uint8_t *buf);

extern time_t ws_get_datetime(const uint8_t *buf);
extern char *ws_datetime_str(const uint8_t *buf, char *str, size_t len);

extern time_t ws_get_timestamp(const uint8_t *buf);
extern char *ws_timestamp_str(const uint8_t *buf, char *str, size_t len);

#endif	/* _SCONVERT_H */
