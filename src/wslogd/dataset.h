#ifndef _DATASET_H
#define _DATASET_H

#include <time.h>
#include <stdint.h>
#include <sys/types.h>

#define _WF_FLAG(p) (1 << (p))

#define WF_PRESSURE			_WF_FLAG(WS_PRESSURE )
#define WF_BAROMETER 		_WF_FLAG(WS_BAROMETER)
#define WF_ALTIMETER 		_WF_FLAG(WS_ALTIMETER)
#define WF_TEMP 			_WF_FLAG(WS_TEMP)
#define WF_HUMIDITY 		_WF_FLAG(WS_HUMIDITY)
#define WF_WIND_SPEED 		_WF_FLAG(WS_WIND_SPEED)
#define WF_WIND_DIR 		_WF_FLAG(WS_WIND_DIR)
#define WF_WIND_GUST_SPEED	_WF_FLAG(WS_WIND_GUST_SPEED)
#define WF_WIND_GUST_DIR	_WF_FLAG(WS_WIND_GUST_DIR)
#define WF_RAIN				_WF_FLAG(WS_RAIN)
#define WF_RAIN_RATE		_WF_FLAG(WS_RAIN_RATE)
#define WF_DEW_POINT		_WF_FLAG(WS_DEW_POINT)
#define WF_WINDCHILL		_WF_FLAG(WS_WINDCHILL)
#define WF_HEAT_INDEX		_WF_FLAG(WS_HEAT_INDEX)
#define WF_TEMP_IN			_WF_FLAG(WS_TEMP_IN)
#define WF_HUMIDITY_IN		_WF_FLAG(WS_HUMIDITY_IN)

#define WF_WIND				(WF_WIND_SPEED|WF_WIND_DIR)
#define WF_WIND_GUST		(WF_WIND_GUST_SPEED|WF_WIND_GUST_DIR)

enum
{
	WS_PRESSURE = 0,
	WS_BAROMETER,
	WS_ALTIMETER,
	WS_TEMP,
	WS_HUMIDITY,
	WS_WIND_SPEED,
	WS_WIND_DIR,
	WS_WIND_GUST_SPEED,
	WS_WIND_GUST_DIR,
	WS_RAIN,
	WS_RAIN_RATE,
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
