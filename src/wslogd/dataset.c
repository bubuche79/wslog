#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <math.h>
#include <string.h>
#include <errno.h>

#include "defs/std.h"

#include "libws/aggregate.h"
#include "libws/util.h"

#include "board.h"
#include "conf.h"
#include "dataset.h"

#define RAIN_PERIOD 900
#define WF_ISSET(mask, flag) (((mask) & (flag)) == (flag))

struct ws_aggr
{
	struct aggr_data aggr;
	int (*get) (const struct ws_loop *, double *);
	int (*set) (struct ws_loop *, double);
};

#if 0
static int
ws_isset(const struct ws_archive *p, int flag)
{
	return (p->wl_mask & flag) == flag;
}
#endif

static int
ws_get(uint32_t mask, int flag, double value, double *v)
{
	int ret;

	if (WF_ISSET(mask, flag)) {
		*v = value;
		ret = 0;
	} else {
		errno = ENODATA;
		ret = -1;
	}

	return ret;

}

static int
ws_set_float(uint32_t *mask, int flag, float *value, double v)
{
	*mask |= flag;
	*value = v;

	return 0;
}

static int
ws_set_uint16(uint32_t *mask, int flag, uint16_t *value, double v)
{
	*mask |= flag;
	*value = lround(v);

	return 0;
}

static int
ws_set_uint8(uint32_t *mask, int flag, uint8_t *value, double v)
{
	*mask |= flag;
	*value = lround(v);

	return 0;
}

int
ws_get_barometer(const struct ws_archive *p, double *v)
{
	return ws_get(p->wl_mask, WF_BAROMETER, p->barometer, v);
}

#if 0
int
ws_get_pressure(const struct ws_archive *p, double *v)
{
	return ws_get(p->wl_mask, WF_PRESSURE, p->pressure, v);
}

int
ws_get_altimeter(const struct ws_archive *p, double *v)
{
	return ws_get(p->wl_mask, WF_ALTIMETER, p->altimeter, v);
}
#endif

int
ws_get_temp(const struct ws_archive *p, double *v)
{
	return ws_get(p->wl_mask, WF_TEMP, p->temp, v);
}

int
ws_get_hi_temp(const struct ws_archive *p, double *v)
{
	return ws_get(p->wl_mask, WF_HI_TEMP, p->hi_temp, v);
}

int
ws_get_lo_temp(const struct ws_archive *p, double *v)
{
	return ws_get(p->wl_mask, WF_LO_TEMP, p->lo_temp, v);
}

int
ws_get_humidity(const struct ws_archive *p, double *v)
{
	return ws_get(p->wl_mask, WF_HUMIDITY, p->humidity, v);
}

int
ws_get_wind_speed(const struct ws_archive *p, double *v)
{
	return ws_get(p->wl_mask, WF_WIND_SPEED, p->avg_wind_speed, v);
}

int
ws_get_wind_dir(const struct ws_archive *p, double *v)
{
	return ws_get(p->wl_mask, WF_WIND_DIR, p->avg_wind_dir, v);
}

int
ws_get_wind_samples(const struct ws_archive *p, double *v)
{
	return ws_get(p->wl_mask, WF_WIND_SAMPLES, p->wind_samples, v);
}

int
ws_get_hi_wind_speed(const struct ws_archive *p, double *v)
{
	return ws_get(p->wl_mask, WF_HI_WIND_SPEED, p->hi_wind_speed, v);
}

int
ws_get_hi_wind_dir(const struct ws_archive *p, double *v)
{
	return ws_get(p->wl_mask, WF_HI_WIND_DIR, p->hi_wind_dir, v);
}

int
ws_get_rain(const struct ws_archive *p, double *v)
{
	return ws_get(p->wl_mask, WF_RAIN, p->rain_fall, v);
}

int
ws_get_hi_rain_rate(const struct ws_archive *p, double *v)
{
	return ws_get(p->wl_mask, WF_HI_RAIN_RATE, p->hi_rain_rate, v);
}

int
ws_get_dew_point(const struct ws_archive *p, double *v)
{
	return ws_get(p->wl_mask, WF_DEW_POINT, p->dew_point, v);
}

int
ws_get_windchill(const struct ws_archive *p, double *v)
{
	return ws_get(p->wl_mask, WF_WINDCHILL, p->windchill, v);
}

int
ws_get_heat_index(const struct ws_archive *p, double *v)
{
	return ws_get(p->wl_mask, WF_HEAT_INDEX, p->heat_index, v);
}

int
ws_get_in_temp(const struct ws_archive *p, double *v)
{
	return ws_get(p->wl_mask, WF_IN_TEMP, p->in_temp, v);
}

int
ws_get_in_humidity(const struct ws_archive *p, double *v)
{
	return ws_get(p->wl_mask, WF_IN_HUMIDITY, p->in_humidity, v);
}

int
ws_set_barometer(struct ws_archive *p, double v)
{
	return ws_set_float(&p->wl_mask, WF_BAROMETER, &p->barometer, v);
}

#if 0
int
ws_set_pressure(struct ws_archive *p, double v)
{
	return ws_set_float(&p->wl_mask, WF_PRESSURE, &p->pressure, v);
}

int
ws_set_altimeter(struct ws_archive *p, double v)
{
	return ws_set_float(&p->wl_mask, WF_ALTIMETER, &p->altimeter, v);
}
#endif

int
ws_set_temp(struct ws_archive *p, double v)
{
	return ws_set_float(&p->wl_mask, WF_TEMP, &p->temp, v);
}

int
ws_set_hi_temp(struct ws_archive *p, double v)
{
	return ws_set_float(&p->wl_mask, WF_HI_TEMP, &p->hi_temp, v);
}

int
ws_set_lo_temp(struct ws_archive *p, double v)
{
	return ws_set_float(&p->wl_mask, WF_LO_TEMP, &p->lo_temp, v);
}

int
ws_set_humidity(struct ws_archive *p, double v)
{
	return ws_set_uint8(&p->wl_mask, WF_HUMIDITY, &p->humidity, v);
}

int
ws_set_wind_speed(struct ws_archive *p, double v)
{
	return ws_set_float(&p->wl_mask, WF_WIND_SPEED, &p->avg_wind_speed, v);
}

int
ws_set_wind_dir(struct ws_archive *p, double v)
{
	return ws_set_uint16(&p->wl_mask, WF_WIND_DIR, &p->avg_wind_dir, v);
}

int
ws_set_wind_samples(struct ws_archive *p, double v)
{
	return ws_set_uint16(&p->wl_mask, WF_WIND_SAMPLES, &p->wind_samples, v);
}

int
ws_set_hi_wind_speed(struct ws_archive *p, double v)
{
	return ws_set_float(&p->wl_mask, WF_HI_WIND_SPEED, &p->hi_wind_speed, v);
}

int
ws_set_hi_wind_dir(struct ws_archive *p, double v)
{
	return ws_set_uint16(&p->wl_mask, WF_HI_WIND_DIR, &p->hi_wind_dir, v);
}

int
ws_set_rain(struct ws_archive *p, double v)
{
	return ws_set_float(&p->wl_mask, WF_RAIN, &p->rain_fall, v);
}

int
ws_set_hi_rain_rate(struct ws_archive *p, double v)
{
	return ws_set_float(&p->wl_mask, WF_HI_RAIN_RATE, &p->hi_rain_rate, v);
}

int
ws_set_dew_point(struct ws_archive *p, double v)
{
	return ws_set_float(&p->wl_mask, WF_DEW_POINT, &p->dew_point, v);
}

int
ws_set_windchill(struct ws_archive *p, double v)
{
	return ws_set_float(&p->wl_mask, WF_WINDCHILL, &p->windchill, v);
}

int
ws_set_heat_index(struct ws_archive *p, double v)
{
	return ws_set_float(&p->wl_mask, WF_HEAT_INDEX, &p->heat_index, v);
}

int
ws_set_in_temp(struct ws_archive *p, double v)
{
	return ws_set_float(&p->wl_mask, WF_IN_TEMP, &p->in_temp, v);
}

int
ws_set_in_humidity(struct ws_archive *p, double v)
{
	return ws_set_uint8(&p->wl_mask, WF_IN_HUMIDITY, &p->in_humidity, v);
}

int
ws_loop_barometer(const struct ws_loop *p, double *v)
{
	return ws_get(p->wl_mask, WF_BAROMETER, p->barometer, v);
}

int
ws_loop_temp(const struct ws_loop *p, double *v)
{
	return ws_get(p->wl_mask, WF_TEMP, p->temp, v);
}

int
ws_loop_humidity(const struct ws_loop *p, double *v)
{
	return ws_get(p->wl_mask, WF_HUMIDITY, p->humidity, v);
}

int
ws_loop_wind_speed(const struct ws_loop *p, double *v)
{
	return ws_get(p->wl_mask, WF_WIND_SPEED, p->wind_speed, v);
}

int
ws_loop_wind_dir(const struct ws_loop *p, double *v)
{
	return ws_get(p->wl_mask, WF_WIND_DIR, p->wind_dir, v);
}

int
ws_loop_hi_wind_10m_speed(const struct ws_loop *p, double *v)
{
	return ws_get(p->wl_mask, WF_HI_WIND_SPEED, p->hi_wind_10m_speed, v);
}

int
ws_loop_hi_wind_10m_dir(const struct ws_loop *p, double *v)
{
	return ws_get(p->wl_mask, WF_HI_WIND_DIR, p->hi_wind_10m_dir, v);
}

int
ws_loop_rain_day(const struct ws_loop *p, double *v)
{
	return ws_get(p->wl_mask, WF_RAIN_DAY, p->rain_day, v);
}

int
ws_loop_rain_1h(const struct ws_loop *p, double *v)
{
	return ws_get(p->wl_mask, WF_RAIN_1H, p->rain_1h, v);
}

int
ws_loop_rain_rate(const struct ws_loop *p, double *v)
{
	return ws_get(p->wl_mask, WF_RAIN_RATE, p->rain_rate, v);
}

int
ws_loop_dew_point(const struct ws_loop *p, double *v)
{
	return ws_get(p->wl_mask, WF_DEW_POINT, p->dew_point, v);
}

int
ws_loop_windchill(const struct ws_loop *p, double *v)
{
	return ws_get(p->wl_mask, WF_WINDCHILL, p->windchill, v);
}

int
ws_loop_solar_rad(const struct ws_loop *p, double *v)
{
	return ws_get(p->wl_mask, WF_SOLAR_RAD, p->solar_rad, v);
}

int
ws_loop_uv(const struct ws_loop *p, double *v)
{
	return ws_get(p->wl_mask, WF_UV_INDEX, p->uv, v);
}

int
ws_loop_in_temp(const struct ws_loop *p, double *v)
{
	return ws_get(p->wl_mask, WF_IN_TEMP, p->in_temp, v);
}

int
ws_loop_in_humidity(const struct ws_loop *p, double *v)
{
	return ws_get(p->wl_mask, WF_IN_HUMIDITY, p->in_humidity, v);
}

//static int
//is_settable(const struct ws_archive *p, int mask, int flag)
//{
//	return (p->wl_mask & (mask | flag)) == flag;
//}
//
#if 0
static void
calc_barometer(struct ws_archive *p)
{
	if (is_settable(p, WF_BAROMETER, WF_PRESSURE | WF_TEMP)) {
		p->wl_mask |= WF_BAROMETER;
		p->barometer = ws_barometer(p->pressure, p->temp, confp->station.altitude);
	}
}

static void
calc_altimeter(struct ws_archive *p)
{
	if (is_settable(p, WF_ALTIMETER, WF_PRESSURE)) {
		p->wl_mask |= WF_ALTIMETER;
		p->altimeter = ws_altimeter(p->pressure, confp->station.altitude);
	}
}
#endif

//static void
//calc_windchill(struct ws_archive *p)
//{
//	if (is_settable(p, WF_WINDCHILL, WF_WIND_SPEED | WF_TEMP)) {
//		p->wl_mask |= WF_WINDCHILL;
//		p->windchill = ws_windchill(p->temp, p->avg_wind_speed);
//	}
//}
//
//static void
//calc_dewpoint(struct ws_archive *p)
//{
//	if (is_settable(p, WF_DEW_POINT, WF_TEMP | WF_HUMIDITY)) {
//		p->wl_mask |= WF_DEW_POINT;
//		p->dew_point = ws_dewpoint(p->temp, p->humidity);
//	}
//}
//
//static void
//calc_heat_index(struct ws_archive *p)
//{
//	if (is_settable(p, WF_HEAT_INDEX, WF_TEMP | WF_HUMIDITY)) {
//		p->wl_mask |= WF_HEAT_INDEX;
//		p->heat_index = ws_heat_index(p->temp, p->humidity);
//	}
//}

#if 0
static int
calc_aggr(int field, int algo, time_t ref, double *v)
{
	size_t i;
	const struct ws_loop *prev;
	struct aggr_data adbuf;

	aggr_init(&adbuf, algo);

	/* Walk through sensor readings */
	i = 0;
	prev = board_peek(i);

	while (prev != NULL) {
		if (prev->time < ref) {
			prev = NULL;
		} else {
			double value;

			if (fn[field].get(prev, &value) == 0) {
				aggr_update(&adbuf, value);
			}

			/* Next entry */
			i++;
			prev = board_peek(i);
		}
	}

	return aggr_finalize(&adbuf, v);
}

static void
calc_rain_rate(struct ws_archive *p)
{
	if (!ws_isset(p, WF_HI_RAIN_RATE)) {
		int i, ticks;
		double rain_sum = 0;
		const struct ws_loop *prev;

		ticks = 0;

		if (ws_isset(p, WF_RAIN)) {
			ticks = 1;
			rain_sum += p->rain_fall;
		}

		i = 0;
		prev = board_peek(i++);

		while (prev != NULL) {
			if (prev->time + RAIN_PERIOD < p->time) {
				prev = NULL;
			} else {
				if (ws_isset(prev, WF_RAIN)) {
					ticks = 1;
					rain_sum += prev->rain;
				}

				prev = board_peek(i++);
			}
		}

		if (ticks) {
			p->wl_mask |= WF_HI_RAIN_RATE;
			// TODO: to be improved, as RAIN_PERIOD depends on interval
			p->hi_rain_rate = 3600 * rain_sum / RAIN_PERIOD;
		}
	}
}
#endif

/**
 * Calculate missing fields from {@code p}.
 *
 * Some fields are computed with the content of {@code p} only, and some others
 * are computed using LOOP history (rain rate, for example).
 */
void
ws_calc(struct ws_archive *p)
{
//	calc_barometer(p);
//	calc_altimeter(p);
//	calc_windchill(p);
//	calc_dewpoint(p);
//	calc_heat_index(p);
//	calc_rain_rate(p);
}

#if 0
static void
aggr_update_all(struct ws_aggr *arr, size_t nel, const struct ws_loop *p)
{
	size_t i;
	double value;

	for (i = 0; i < nel; i++) {
		if (arr[i].get(p, &value) == 0) {
			aggr_update(&arr[i].aggr, value);
		}
	}
}

static void
aggr_finalize_all(struct ws_aggr *arr, size_t nel, struct ws_archive *p)
{
	size_t j;
	double value;

	for (j = 0; j < nel; j++) {
		if (aggr_finalize(&arr[j].aggr, &value) == 0) {
			arr[j].set(p, value);
		}
	}
}
#endif

/**
 * Compute software archive, with real-time sensor data.
 *
 * The structure pointed to by {@code p} is updated. This function returns the
 * number of structures successfully computed. That is, 0 if there is no
 * real-time sensors to aggregate, and 1 otherwise.
 *
 * The {@code freq} parameter specifies the interval, in seconds, of aggregated
 * sensor data.
 */
ssize_t
ws_aggr(struct ws_archive *p, int freq)
{
#if 0
	size_t i, nel;
	const struct ws_loop *prev;
	time_t now;

	struct ws_aggr aggr_arr[] =
	{
//		{ AGGR_AVG_INIT, ws_get_pressure, ws_set_pressure },
//		{ AGGR_AVG_INIT, ws_get_altimeter, ws_set_altimeter },
//		{ AGGR_AVG_INIT, ws_get_barometer, ws_set_barometer },
//		{ AGGR_AVG_INIT, ws_get_temp, ws_set_temp },
//		{ AGGR_AVG_INIT, ws_get_humidity, ws_set_humidity },
		{ AGGR_AVG_INIT, ws_get_wind_speed, ws_set_wind_speed },
		{ AGGR_AVG_INIT, ws_get_wind_dir, ws_set_wind_dir },
		{ AGGR_SUM_INIT, ws_get_rain, ws_set_rain },
		{ AGGR_MAX_INIT, ws_get_rain_rate, ws_set_rain_rate },
//		{ AGGR_AVG_INIT, ws_get_dew_point, ws_set_dew_point },
//		{ AGGR_AVG_INIT, ws_get_windchill, ws_set_windchill },
//		{ AGGR_AVG_INIT, ws_get_heat_index, ws_set_heat_index },
//		{ AGGR_AVG_INIT, ws_get_temp_in, ws_set_temp_in },
//		{ AGGR_AVG_INIT, ws_get_humidity_in, ws_set_humidity_in }
	};

	/* Walk through sensor readings */
	i = 0;
	prev = board_peek(i);

	if (prev != NULL) {
		now = time(NULL);
		nel = array_size(aggr_arr);

		memcpy(&p->data, prev, sizeof(*prev));

		p->interval = freq;
		p->data.time = now;

		p->data.wind_gust_dir = 0;
		p->data.wind_gust_speed = 0;

		do {
			if (prev->time + freq < now) {
				prev = NULL;
			} else {
				/* Wind gust */
				if (WF_ISSET(prev->wl_mask, WF_WIND)) {
					if (p->data.wind_gust_speed < prev->wind_speed) {
						p->data.wl_mask |= WF_WIND_GUST;
						p->data.wind_gust_speed = prev->wind_speed;
						p->data.wind_gust_dir = prev->wind_dir;
					}
				}

				/* Other metrics */
				aggr_update_all(aggr_arr, nel, prev);

				/* Next entry */
				i++;
				prev = board_peek(i);
			}
		} while (prev != NULL);

		/* Compute derived metrics */
		aggr_finalize_all(aggr_arr, nel, &p->data);
	}

	return (i == 0) ? 0 : 1;
#endif
	return 0;
}
