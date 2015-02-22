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
	WS_BIN_2NYB,
	WS_DATE,
	WS_TIMESTAMP,
	WS_DATETIME,
	WS_TIME,
	WS_CONNECTION
};

struct ws_conv
{
	char *units;				/* units name (eg hPa) */
	uint8_t nybble;				/* nybble count */
	char *descr;				/* units description */

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
			char *format;		/* date, timestamp format */
		} tm;
		struct
		{
			struct
			{
				uint8_t key;
				char *value;
			} map[3];
		} text;
	};
};

struct ws_history
{
	double temp_in;
	double temp_out;
	double abs_pressure;
	int humidity_in;
	int humidity_out;
	double rain;
	double wind_speed;
	double wind_dir;
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

double ws_get_interval_sec(const uint8_t *buf);
char *ws_interval_sec_str(const uint8_t *buf, char *str, size_t len);

double ws_get_interval_min(const uint8_t *buf);
char *ws_interval_min_str(const uint8_t *buf, char *str, size_t len);

double ws_get_2nyb(const uint8_t *buf);
char *ws_2nyb_str(const uint8_t *buf, char *str, size_t len);

extern time_t ws_get_datetime(const uint8_t *buf);
extern char *ws_datetime_str(const uint8_t *buf, char *str, size_t len);

extern time_t ws_get_timestamp(const uint8_t *buf);
extern char *ws_timestamp_str(const uint8_t *buf, char *str, size_t len);

extern time_t ws_get_timestamp(const uint8_t *buf);
extern char *ws_timestamp_str(const uint8_t *buf, char *str, size_t len);

extern void ws_decode_history(const uint8_t *buf, struct ws_history *h);

#endif	/* _SCONVERT_H */
