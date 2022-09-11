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
#define WF_10M_WIND_SPEED	_WF_FLAG(WS_10M_WIND_SPEED)
#define WF_HI_WIND_SPEED	_WF_FLAG(WS_HI_WIND_SPEED)
#define WF_HI_WIND_DIR		_WF_FLAG(WS_HI_WIND_DIR)
#define WF_RAIN			_WF_FLAG(WS_RAIN)
#define WF_RAIN_DAY		_WF_FLAG(WS_RAIN_DAY)
#define WF_RAIN_1H		_WF_FLAG(WS_RAIN_1H)
#define WF_RAIN_24H		_WF_FLAG(WS_RAIN_24H)
#define WF_RAIN_RATE		_WF_FLAG(WS_RAIN_RATE)
#define WF_HI_RAIN_RATE		_WF_FLAG(WS_HI_RAIN_RATE)
#define WF_DEW_POINT		_WF_FLAG(WS_DEW_POINT)
#define WF_WINDCHILL		_WF_FLAG(WS_WINDCHILL)
#define WF_SOLAR_RAD		_WF_FLAG(WS_SOLAR_RAD)
#define WF_UV_INDEX		_WF_FLAG(WS_UV_INDEX)
#define WF_HEAT_INDEX		_WF_FLAG(WS_HEAT_INDEX)
#define WF_IN_TEMP		_WF_FLAG(WS_IN_TEMP)
#define WF_IN_HUMIDITY		_WF_FLAG(WS_IN_HUMIDITY)

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
	WS_10M_WIND_SPEED,
	WS_HI_WIND_SPEED,
	WS_HI_WIND_DIR,
	WS_RAIN,
	WS_RAIN_DAY,
	WS_RAIN_1H,
	WS_RAIN_24H,
	WS_RAIN_RATE,
	WS_HI_RAIN_RATE,
	WS_DEW_POINT,
	WS_WINDCHILL,
	WS_SOLAR_RAD,
	WS_UV_INDEX,
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
	struct timespec time;		/* Data time */
	uint32_t wl_mask;		/* Fields mask */

	double barometer;		/* Barometer, sea level pressure (hPa) */
#if 0
	double pressure;		/* Absolute pressure (hPa) */
	double altimeter;		/* altimeter, altitude corrected pressure (hPa) */
#endif
	double temp;			/* Temperature (°C) */
	uint8_t humidity; 		/* Humidity (%) */
	double wind_speed;		/* Wind speed (m/s) */
	uint16_t wind_dir;		/* Wind direction (°) */
	double wind_10m_speed;		/* 10-minutes wind speed (m/s) */
	double hi_wind_10m_speed;	/* 10-minutes wind gust speed (m/s) */
	uint16_t hi_wind_10m_dir;	/* 10-minutes wind gust direction (°) */
	double rain_day;		/* Daily rain (mm) */
	double rain_rate;		/* Rain rate (mm/hr) */
	double rain_1h;			/* Accumulated rain in the past hour (mm) */
	double rain_24h;		/* Accumulated rain in the past 24 hours (mm) */
#if 0
	double sample_et;		/* Sample evapotranspiration (mm) */
#endif
	uint16_t solar_rad;		/* Solar radiation (W/m³) */
	double uv;			/* UV index */
	double dew_point; 		/* Dew point (°C) */
	double windchill;		/* Windchill temperature (°C) */
	double heat_index;		/* Heat index (°C) */

	double in_temp;			/* Indoor temperature (°C) */
	uint8_t in_humidity;		/* Indoor humidity (%) */
};

struct ws_archive
{
	time_t time;			/* Archive time */
	time_t interval;		/* Archive interval, in seconds */
	uint32_t wl_mask;		/* Fields mask */

	double barometer;		/* Barometer, sea level pressure (hPa) */
#if 0
	double pressure;		/* Absolute pressure (hPa) */
	double altimeter;		/* Altimeter, altitude corrected pressure (hPa) */
#endif
	double temp;			/* Temperature (°C) */
	double hi_temp;			/* High temperature (°C) */
	double lo_temp;			/* Low temperature (°C) */
	uint8_t humidity; 		/* Humidity (%) */
	double avg_wind_speed;		/* Wind speed (m/s) */
	uint16_t avg_wind_dir;		/* Wind direction (°) */
	uint16_t wind_samples;		/* Wind samples */
	double hi_wind_speed;		/* High wind speed (m/s) */
	uint16_t hi_wind_dir;		/* High wind direction (°) */
	double rain_fall;		/* Sample rain fall (mm) */
	double hi_rain_rate;		/* High rain rate (mm/hr) */
#if 0
	double rain_1h;			/* accumulated rain in the past hour (mm) */
	double rain_24h;		/* accumulated rain in the past 24 hours (mm) */
	double sample_et;		/* sample evapotranspiration (mm) */
	uint16_t radiation;		/* solar radiation (W/m³) */
	double uv;			/* UV index */
#endif
	double dew_point; 		/* Dew point (°C) */
	double windchill;		/* Windchill temperature (°C) */
	double heat_index;		/* Heat index (°C) */

	double in_temp;			/* Indoor temperature (°C) */
	uint8_t in_humidity;		/* Indoor humidity (%) */
};

#ifdef __cplusplus
extern "C" {
#endif

int ws_get_barometer(const struct ws_archive *p, double *v);
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

int ws_loop_barometer(const struct ws_loop *p, double *v);
int ws_loop_temp(const struct ws_loop *p, double *v);
int ws_loop_humidity(const struct ws_loop *p, double *v);
int ws_loop_wind_speed(const struct ws_loop *p, double *v);
int ws_loop_wind_dir(const struct ws_loop *p, double *v);
int ws_loop_hi_wind_10m_speed(const struct ws_loop *p, double *v);
int ws_loop_hi_wind_10m_dir(const struct ws_loop *p, double *v);
int ws_loop_rain_day(const struct ws_loop *p, double *v);
int ws_loop_rain_1h(const struct ws_loop *p, double *v);
int ws_loop_rain_rate(const struct ws_loop *p, double *v);
int ws_loop_dew_point(const struct ws_loop *p, double *v);
int ws_loop_windchill(const struct ws_loop *p, double *v);
int ws_loop_solar_rad(const struct ws_loop *p, double *v);
int ws_loop_uv(const struct ws_loop *p, double *v);
int ws_loop_heat_index(const struct ws_loop *p, double *v);
int ws_loop_in_temp(const struct ws_loop *p, double *v);
int ws_loop_in_humidity(const struct ws_loop *p, double *v);

void ws_calc(struct ws_archive *p);
ssize_t ws_aggr(struct ws_archive *p, int freq);

#ifdef __cplusplus
}
#endif

#endif /* _DATASET_H */
