#ifndef _DATASET_H
#define _DATASET_H

#include <time.h>
#include <stdint.h>
#include <sys/types.h>

#define WF_PRESSURE 		0x0001
#define WF_ALTIMETER 		0x0002
#define WF_BAROMETER 		0x0004
#define WF_TEMP				0x0008
#define WF_HUMIDITY			0x0010
#define WF_WIND				0x0020
#define WF_WIND_GUST		0x0040
#define WF_RAIN				0x0080
#define WF_RAIN_RATE		0x0100
#if 0
#define WF_RAIN_1H			0x0200
#define WF_RAIN_24H			0x0400
#endif
#define WF_DEW_POINT		0x0800
#define WF_WINDCHILL		0x1000
#define WF_HEAT_INDEX		0x2000
#define WF_TEMP_IN			0x4000
#define WF_HUMIDITY_IN		0x8000
#define WF_ALL				0xFFFF

enum {
	WS_PRESSURE = 0,
	WS_ALTIMETER,
	WS_BAROMETER,
	WS_TEMP,
	WS_HUMIDITY,
	WS_WIND_SPEED,
	WS_WIND_DIR,
	WS_WIND_GUST_SPEED,
	WS_WIND_GUST_DIR,
	WS_RAIN,
	WS_RAIN_RATE,
#if 0
	WS_RAIN_1H,
	WS_RAIN_24H,
#endif
	WS_DEW_POINT,
	WS_WINDCHILL,
	WS_HEAT_INDEX,
	WS_TEMP_IN,
	WS_HUMIDITY_IN,
	WS_MAX						/* do not use */
};

/**
 * Sensor data.
 *
 * The {@code wl_mask} field is used to indicate valid fields in the structure.
 */
struct ws_loop
{
	time_t time;				/* data time */
	uint32_t wl_mask;			/* fields mask */

	float pressure;				/* absolute pressure (hPa) */
	float barometer;			/* barometer, sea level pressure (hPa) */
	float altimeter;			/* altimeter, altitude corrected pressure (hPa) */
	float temp;					/* temperature (°C) */
	uint8_t humidity; 			/* humidity (%) */
	float wind_speed;			/* wind speed (m/s) */
	uint16_t wind_dir;			/* wind direction (°) */
	float wind_gust_speed;		/* wind gust (m/s) */
	uint16_t wind_gust_dir;		/* wind gust direction (°) */
	float rain;					/* sample rain (mm) */
	float rain_rate;			/* rain rate (mm/hr) */
#if 0
	float rain_1h;				/* accumulated rain in the past hour (mm) */
	float rain_24h;				/* accumulated rain in the past 24 hours (mm) */
	float sample_et;			/* sample evapotranspiration (mm) */
	uint16_t radiation;			/* solar radiation (W/m³) */
	float uv;					/* UV index */
#endif
	float dew_point; 			/* dew point (°C) */
	float windchill;			/* windchill temperature (°C) */
	float heat_index;			/* head index (°C) */

	float temp_in;				/* indoor temperature (°C) */
	uint8_t humidity_in;		/* indoor humidity (%) */
};

struct ws_archive
{
	time_t interval;			/* archive interval, in seconds */
	struct ws_loop data;		/* aggregated data */
};

#ifdef __cplusplus
extern "C" {
#endif

int ws_get_pressure(const struct ws_loop *p, double *v);
int ws_get_altimeter(const struct ws_loop *p, double *v);
int ws_get_barometer(const struct ws_loop *p, double *v);
int ws_get_temp(const struct ws_loop *p, double *v);
int ws_get_humidity(const struct ws_loop *p, double *v);
int ws_get_wind_speed(const struct ws_loop *p, double *v);
int ws_get_wind_dir(const struct ws_loop *p, double *v);
int ws_get_wind_gust_speed(const struct ws_loop *p, double *v);
int ws_get_wind_gust_dir(const struct ws_loop *p, double *v);
int ws_get_rain(const struct ws_loop *p, double *v);
int ws_get_rain_rate(const struct ws_loop *p, double *v);
int ws_get_dew_point(const struct ws_loop *p, double *v);
int ws_get_windchill(const struct ws_loop *p, double *v);
int ws_get_heat_index(const struct ws_loop *p, double *v);
int ws_get_temp_in(const struct ws_loop *p, double *v);
int ws_get_humidity_in(const struct ws_loop *p, double *v);

int ws_set_pressure(struct ws_loop *p, double v);
int ws_set_altimeter(struct ws_loop *p, double v);
int ws_set_barometer(struct ws_loop *p, double v);
int ws_set_temp(struct ws_loop *p, double v);
int ws_set_humidity(struct ws_loop *p, double v);
int ws_set_wind_speed(struct ws_loop *p, double v);
int ws_set_wind_dir(struct ws_loop *p, double v);
int ws_set_wind_gust_speed(struct ws_loop *p, double v);
int ws_set_wind_gust_dir(struct ws_loop *p, double v);
int ws_set_rain(struct ws_loop *p, double v);
int ws_set_rain_rate(struct ws_loop *p, double v);
int ws_set_dew_point(struct ws_loop *p, double v);
int ws_set_windchill(struct ws_loop *p, double v);
int ws_set_heat_index(struct ws_loop *p, double v);
int ws_set_temp_in(struct ws_loop *p, double v);
int ws_set_humidity_in(struct ws_loop *p, double v);

int ws_get_value(const struct ws_loop *p, int flag, double *v);
int ws_set_value(struct ws_loop *p, int flag, double v);

int ws_isset(const struct ws_loop *p, int flag);

void ws_calc(struct ws_loop *p);
ssize_t ws_aggr(struct ws_archive *p, int freq);

#ifdef __cplusplus
}
#endif

#endif /* _DATASET_H */
