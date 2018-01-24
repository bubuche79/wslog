#ifndef _DATASET_H
#define _DATASET_H

#include <time.h>
#include <stdint.h>
#include <sys/types.h>

#define _WF_FLAG(p) 		(1 << (p))

#define WF_BAROMETER 		_WF_FLAG(WS_BAROMETER)
#define WF_PRESSURE		_WF_FLAG(WS_PRESSURE)
#define WF_ALTIMETER 		_WF_FLAG(WS_ALTIMETER)
#define WF_TEMP 		_WF_FLAG(WS_TEMP)
#define WF_HI_TEMP 		_WF_FLAG(WS_HI_TEMP)
#define WF_LO_TEMP 		_WF_FLAG(WS_LO_TEMP)
#define WF_HUMIDITY 		_WF_FLAG(WS_HUMIDITY)
#define WF_WIND_SPEED 		_WF_FLAG(WS_WIND_SPEED)
#define WF_WIND_DIR 		_WF_FLAG(WS_WIND_DIR)
#define WF_WIND_SAMPLES		_WF_FLAG(WS_WIND_SAMPLES)
#define WF_HI_WIND_SPEED	_WF_FLAG(WS_HI_WIND_SPEED)
#define WF_HI_WIND_DIR		_WF_FLAG(WS_HI_WIND_DIR)
#define WF_RAIN_FALL		_WF_FLAG(WS_RAIN_FALL)
#define WF_HI_RAIN_RATE		_WF_FLAG(WS_HI_RAIN_RATE)
#define WF_DEW_POINT		_WF_FLAG(WS_DEW_POINT)
#define WF_WINDCHILL		_WF_FLAG(WS_WINDCHILL)
#define WF_HEAT_INDEX		_WF_FLAG(WS_HEAT_INDEX)
#define WF_IN_TEMP		_WF_FLAG(WS_IN_TEMP)
#define WF_IN_HUMIDITY		_WF_FLAG(WS_IN_HUMIDITY)

#define WF_WIND			(WF_WIND_SPEED|WF_WIND_DIR)
#define WF_WIND_GUST		(WF_WIND_GUST_SPEED|WF_WIND_GUST_DIR)

enum
{
	WS_BAROMETER,
	WS_PRESSURE,			/* Unused */
	WS_ALTIMETER,			/* Unused */
	WS_TEMP,
	WS_HI_TEMP,
	WS_LO_TEMP,
	WS_HUMIDITY,
	WS_WIND_SPEED,
	WS_WIND_DIR,
	WS_WIND_SAMPLES,
	WS_HI_WIND_SPEED,
	WS_HI_WIND_DIR,
	WS_RAIN_FALL,
	WS_HI_RAIN_RATE,
	WS_DEW_POINT,
	WS_WINDCHILL,
	WS_HEAT_INDEX,
	WS_IN_TEMP,
	WS_IN_HUMIDITY,
	WS_MAX				/* do not use */
};

/**
 * Sensor data.
 *
 * The {@code wl_mask} field is used to indicate valid fields in the structure.
 */
struct ws_loop
{
	time_t time;			/* data time */
	uint32_t wl_mask;		/* fields mask */

	float pressure;			/* absolute pressure (hPa) */
	float barometer;		/* barometer, sea level pressure (hPa) */
	float altimeter;		/* altimeter, altitude corrected pressure (hPa) */
	float temp;			/* Temperature (°C) */
	uint8_t humidity; 		/* humidity (%) */
	float wind_speed;		/* wind speed (m/s) */
	uint16_t wind_dir;		/* wind direction (°) */
	float wind_gust_speed;		/* wind gust (m/s) */
	uint16_t wind_gust_dir;		/* wind gust direction (°) */
	float rain;			/* sample rain (mm) */
	float rain_rate;		/* high rain rate (mm/hr) */
#if 0
	float rain_1h;			/* accumulated rain in the past hour (mm) */
	float rain_24h;			/* accumulated rain in the past 24 hours (mm) */
	float sample_et;		/* sample evapotranspiration (mm) */
	uint16_t radiation;		/* solar radiation (W/m³) */
	float uv;			/* UV index */
#endif
	float dew_point; 		/* dew point (°C) */
	float windchill;		/* windchill temperature (°C) */
	float heat_index;		/* head index (°C) */

	float temp_in;			/* indoor temperature (°C) */
	uint8_t humidity_in;		/* indoor humidity (%) */
};

struct ws_archive
{
	time_t time;			/* Archive time */
	time_t interval;		/* Archive interval, in seconds */
	uint32_t wl_mask;		/* Fields mask */

	float barometer;		/* Barometer, sea level pressure (hPa) */
#if 0
	float pressure;			/* Absolute pressure (hPa) */
	float altimeter;		/* Altimeter, altitude corrected pressure (hPa) */
#endif
	float temp;			/* Temperature (°C) */
	float hi_temp;			/* High temperature (°C) */
	float lo_temp;			/* Low temperature (°C) */
	uint8_t humidity; 		/* Humidity (%) */
	float avg_wind_speed;		/* Wind speed (m/s) */
	uint16_t avg_wind_dir;		/* Wind direction (°) */
	uint16_t wind_samples;		/* Wind samples */
	float hi_wind_speed;		/* High wind speed (m/s) */
	uint16_t hi_wind_dir;		/* High wind direction (°) */
	float rain_fall;		/* Sample rain fall (mm) */
	float hi_rain_rate;		/* High rain rate (mm/hr) */
#if 0
	float rain_1h;			/* accumulated rain in the past hour (mm) */
	float rain_24h;			/* accumulated rain in the past 24 hours (mm) */
	float sample_et;		/* sample evapotranspiration (mm) */
	uint16_t radiation;		/* solar radiation (W/m³) */
	float uv;			/* UV index */
#endif
	float dew_point; 		/* Dew point (°C) */
	float windchill;		/* Windchill temperature (°C) */
	float heat_index;		/* Head index (°C) */

	float in_temp;			/* Indoor temperature (°C) */
	uint8_t in_humidity;		/* Indoor humidity (%) */
};

#ifdef __cplusplus
extern "C" {
#endif

int ws_get_barometer(const struct ws_archive *p, double *v);
#if 0
int ws_get_pressure(const struct ws_archive *p, double *v);
int ws_get_altimeter(const struct ws_archive *p, double *v);
#endif
int ws_get_temp(const struct ws_archive *p, double *v);
int ws_get_hi_temp(const struct ws_archive *p, double *v);
int ws_get_lo_temp(const struct ws_archive *p, double *v);
int ws_get_humidity(const struct ws_archive *p, double *v);
int ws_get_wind_speed(const struct ws_archive *p, double *v);
int ws_get_wind_dir(const struct ws_archive *p, double *v);
int ws_get_wind_samples(const struct ws_archive *p, double *v);
int ws_get_hi_wind_speed(const struct ws_archive *p, double *v);
int ws_get_hi_wind_dir(const struct ws_archive *p, double *v);
int ws_get_rain(const struct ws_archive *p, double *v);
int ws_get_hi_rain_rate(const struct ws_archive *p, double *v);
int ws_get_dew_point(const struct ws_archive *p, double *v);
int ws_get_windchill(const struct ws_archive *p, double *v);
int ws_get_heat_index(const struct ws_archive *p, double *v);
int ws_get_in_temp(const struct ws_archive *p, double *v);
int ws_get_in_humidity(const struct ws_archive *p, double *v);

int ws_set_barometer(struct ws_archive *p, double v);
#if 0
int ws_set_pressure(struct ws_archive *p, double v);
int ws_set_altimeter(struct ws_archive *p, double v);
#endif
int ws_set_temp(struct ws_archive *p, double v);
int ws_set_hi_temp(struct ws_archive *p, double v);
int ws_set_lo_temp(struct ws_archive *p, double v);
int ws_set_humidity(struct ws_archive *p, double v);
int ws_set_wind_speed(struct ws_archive *p, double v);
int ws_set_wind_dir(struct ws_archive *p, double v);
int ws_set_wind_samples(struct ws_archive *p, double v);
int ws_set_hi_wind_speed(struct ws_archive *p, double v);
int ws_set_hi_wind_dir(struct ws_archive *p, double v);
int ws_set_rain(struct ws_archive *p, double v);
int ws_set_hi_rain_rate(struct ws_archive *p, double v);
int ws_set_dew_point(struct ws_archive *p, double v);
int ws_set_windchill(struct ws_archive *p, double v);
int ws_set_heat_index(struct ws_archive *p, double v);
int ws_set_in_temp(struct ws_archive *p, double v);
int ws_set_in_humidity(struct ws_archive *p, double v);

void ws_calc(struct ws_archive *p);
ssize_t ws_aggr(struct ws_archive *p, int freq);

#ifdef __cplusplus
}
#endif

#endif /* _DATASET_H */
