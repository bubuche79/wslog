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
ws_get_pressure(const struct ws_loop *p, double *v)
{
	return ws_get(p->wl_mask, WF_PRESSURE, p->pressure, v);
}

int
ws_get_altimeter(const struct ws_loop *p, double *v)
{
	return ws_get(p->wl_mask, WF_ALTIMETER, p->altimeter, v);
}

int
ws_get_barometer(const struct ws_loop *p, double *v)
{
	return ws_get(p->wl_mask, WF_BAROMETER, p->barometer, v);
}

int
ws_get_temp(const struct ws_loop *p, double *v)
{
	return ws_get(p->wl_mask, WF_TEMP, p->temp, v);
}

int
ws_get_humidity(const struct ws_loop *p, double *v)
{
	return ws_get(p->wl_mask, WF_HUMIDITY, p->humidity, v);
}

int
ws_get_wind_speed(const struct ws_loop *p, double *v)
{
	return ws_get(p->wl_mask, WF_WIND_SPEED, p->wind_speed, v);
}

int
ws_get_wind_dir(const struct ws_loop *p, double *v)
{
	return ws_get(p->wl_mask, WF_WIND_DIR, p->wind_dir, v);
}

int
ws_get_wind_gust_speed(const struct ws_loop *p, double *v)
{
	return ws_get(p->wl_mask, WF_WIND_GUST_SPEED, p->wind_gust_speed, v);
}

int
ws_get_wind_gust_dir(const struct ws_loop *p, double *v)
{
	return ws_get(p->wl_mask, WF_WIND_GUST_DIR, p->wind_gust_dir, v);
}

int
ws_get_rain(const struct ws_loop *p, double *v)
{
	return ws_get(p->wl_mask, WF_RAIN, p->rain, v);
}

int
ws_get_rain_rate(const struct ws_loop *p, double *v)
{
	return ws_get(p->wl_mask, WF_RAIN_RATE, p->rain_rate, v);
}

int
ws_get_dew_point(const struct ws_loop *p, double *v)
{
	return ws_get(p->wl_mask, WF_DEW_POINT, p->dew_point, v);
}

int
ws_get_windchill(const struct ws_loop *p, double *v)
{
	return ws_get(p->wl_mask, WF_WINDCHILL, p->windchill, v);
}

int
ws_get_heat_index(const struct ws_loop *p, double *v)
{
	return ws_get(p->wl_mask, WF_HEAT_INDEX, p->heat_index, v);
}

int
ws_get_temp_in(const struct ws_loop *p, double *v)
{
	return ws_get(p->wl_mask, WF_TEMP_IN, p->temp_in, v);
}

int
ws_get_humidity_in(const struct ws_loop *p, double *v)
{
	return ws_get(p->wl_mask, WF_HUMIDITY_IN, p->humidity_in, v);
}

int
ws_set_pressure(struct ws_loop *p, double v)
{
	return ws_set_float(&p->wl_mask, WF_PRESSURE, &p->pressure, v);
}

int
ws_set_altimeter(struct ws_loop *p, double v)
{
	return ws_set_float(&p->wl_mask, WF_ALTIMETER, &p->altimeter, v);
}

int
ws_set_barometer(struct ws_loop *p, double v)
{
	return ws_set_float(&p->wl_mask, WF_BAROMETER, &p->barometer, v);
}

int
ws_set_temp(struct ws_loop *p, double v)
{
	return ws_set_float(&p->wl_mask, WF_TEMP, &p->temp, v);
}

int
ws_set_humidity(struct ws_loop *p, double v)
{
	return ws_set_uint8(&p->wl_mask, WF_HUMIDITY, &p->humidity, v);
}

int
ws_set_wind_speed(struct ws_loop *p, double v)
{
	return ws_set_float(&p->wl_mask, WF_WIND_SPEED, &p->wind_speed, v);
}

int
ws_set_wind_dir(struct ws_loop *p, double v)
{
	return ws_set_uint16(&p->wl_mask, WF_WIND_DIR, &p->wind_dir, v);
}

int
ws_set_wind_gust_speed(struct ws_loop *p, double v)
{
	return ws_set_float(&p->wl_mask, WF_WIND_GUST_SPEED, &p->wind_gust_speed, v);
}

int
ws_set_wind_gust_dir(struct ws_loop *p, double v)
{
	return ws_set_uint16(&p->wl_mask, WF_WIND_GUST_DIR, &p->wind_gust_dir, v);
}

int
ws_set_rain(struct ws_loop *p, double v)
{
	return ws_set_float(&p->wl_mask, WF_RAIN, &p->rain, v);
}

int
ws_set_rain_rate(struct ws_loop *p, double v)
{
	return ws_set_float(&p->wl_mask, WF_RAIN_RATE, &p->rain_rate, v);
}

int
ws_set_dew_point(struct ws_loop *p, double v)
{
	return ws_set_float(&p->wl_mask, WF_DEW_POINT, &p->dew_point, v);
}

int
ws_set_windchill(struct ws_loop *p, double v)
{
	return ws_set_float(&p->wl_mask, WF_WINDCHILL, &p->windchill, v);
}

int
ws_set_heat_index(struct ws_loop *p, double v)
{
	return ws_set_float(&p->wl_mask, WF_HEAT_INDEX, &p->heat_index, v);
}

int
ws_set_temp_in(struct ws_loop *p, double v)
{
	return ws_set_float(&p->wl_mask, WF_TEMP_IN, &p->temp_in, v);
}

int
ws_set_humidity_in(struct ws_loop *p, double v)
{
	return ws_set_uint8(&p->wl_mask, WF_HUMIDITY_IN, &p->humidity_in, v);
}

static int
is_settable(const struct ws_loop *p, int mask, int flag)
{
	return (p->wl_mask & (mask | flag)) == flag;
}

static void
calc_barometer(struct ws_loop *p)
{
	if (is_settable(p, WF_BAROMETER, WF_PRESSURE | WF_TEMP)) {
		p->wl_mask |= WF_BAROMETER;
		p->barometer = ws_barometer(p->pressure, p->temp, confp->station.altitude);
	}
}

static void
calc_altimeter(struct ws_loop *p)
{
	if (is_settable(p, WF_ALTIMETER, WF_PRESSURE)) {
		p->wl_mask |= WF_ALTIMETER;
		p->altimeter = ws_altimeter(p->pressure, confp->station.altitude);
	}
}

static void
calc_windchill(struct ws_loop *p)
{
	if (is_settable(p, WF_WINDCHILL, WF_WIND_SPEED | WF_TEMP)) {
		p->wl_mask |= WF_WINDCHILL;
		p->windchill = ws_windchill(p->temp, p->wind_speed);
	}
}

static void
calc_dewpoint(struct ws_loop *p)
{
	if (is_settable(p, WF_DEW_POINT, WF_TEMP | WF_HUMIDITY)) {
		p->wl_mask |= WF_DEW_POINT;
		p->dew_point = ws_dewpoint(p->temp, p->humidity);
	}
}

static void
calc_heat_index(struct ws_loop *p)
{
	if (is_settable(p, WF_HEAT_INDEX, WF_TEMP | WF_HUMIDITY)) {
		p->wl_mask |= WF_HEAT_INDEX;
		p->heat_index = ws_heat_index(p->temp, p->humidity);
	}
}

static void
calc_rain_rate(struct ws_loop *p)
{
	if (!ws_isset(p, WF_RAIN_RATE)) {
		int i, ticks;
		double rain_sum = 0;
		const struct ws_loop *prev;

		ticks = 0;

		if (ws_isset(p, WF_RAIN)) {
			ticks = 1;
			rain_sum = p->rain;
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

//				period = p->time.tv_sec - prev->time.tv_sec;
				prev = board_peek(i++);
			}
		}

		if (ticks) {
			p->wl_mask |= WF_RAIN_RATE;
			// TODO: to be improved, as RAIN_PERIOD depends on interval
			p->rain_rate = 3600 * rain_sum / RAIN_PERIOD;
		}
	}
}

int
ws_isset(const struct ws_loop *p, int flag)
{
	return (p->wl_mask & flag) == flag;
}

#if 0
int
ws_get_value(const struct ws_loop *p, int flag, double *v)
{
	int ret;

	if (WF_ISSET(p->wl_mask, flag)) {
		switch (flag) {
		case WS_PRESSURE:
			*v = p->pressure;
			break;
		case WS_BAROMETER:
			*v = p->barometer;
			break;
		case WS_ALTIMETER:
			*v = p->altimeter;
			break;
		case WS_TEMP:
			*v = p->temp;
			break;
		case WS_HUMIDITY:
			*v = p->humidity;
			break;
		case WS_WIND_SPEED:
			*v = p->wind_speed;
			break;
		case WS_WIND_DIR:
			*v = p->wind_dir;
			break;
		case WS_WIND_GUST_SPEED:
			*v = p->wind_gust_speed;
			break;
		case WS_WIND_GUST_DIR:
			*v = p->wind_gust_dir;
			break;
		case WS_RAIN:
			*v = p->rain;
			break;
		case WS_RAIN_RATE:
			*v = p->rain_rate;
			break;
#if 0
		case WS_RAIN_1H:
			*v = p->rain_1h;
			break;
		case WS_RAIN_24H:
			*v = p->rain_24h;
			break;
#endif
		case WS_DEW_POINT:
			*v = p->dew_point;
			break;
		case WS_WINDCHILL:
			*v = p->windchill;
			break;
		case WS_HEAT_INDEX:
			*v = p->heat_index;
			break;
		case WS_TEMP_IN:
			*v = p->temp_in;
			break;
		case WS_HUMIDITY_IN:
			*v = p->humidity_in;
			break;
		default:
			errno = EINVAL;
			ret = -1;
			break;
		}
	} else {
		errno = ENODATA;
		ret = -1;
	}

	return ret;
}
#endif

/**
 * Calculate missing fields from {@code p}.
 *
 * Some fields are computed with the content of {@code p} only, and some others
 * are computed using LOOP history (rain rate, for example).
 */
void
ws_calc(struct ws_loop *p)
{
	calc_barometer(p);
	calc_altimeter(p);
	calc_windchill(p);
	calc_dewpoint(p);
	calc_heat_index(p);
	calc_rain_rate(p);
}

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
aggr_finalize_all(struct ws_aggr *arr, size_t nel, struct ws_loop *p)
{
	size_t j;
	double value;

	for (j = 0; j < nel; j++) {
		if (aggr_finalize(&arr[j].aggr, &value) == 0) {
			arr[j].set(p, value);
		}
	}
}

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
}
