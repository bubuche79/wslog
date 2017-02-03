#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "libws/util.h"

#include "board.h"
#include "dataset.h"

#define RAIN_PERIOD		900

static int
is_settable(const struct ws_loop *p, int mask, int flag)
{
	return (p->wl_mask & (mask | flag)) == flag;
}

static int
is_before(const struct ws_loop *prev, const struct ws_loop *p)
{
	return prev->time.tv_sec + RAIN_PERIOD < p->time.tv_sec;
}

static void
calc_windchill(struct ws_loop* p)
{
	if (is_settable(p, WF_WINDCHILL, WF_WIND | WF_TEMP)) {
		p->wl_mask |= WF_WINDCHILL;
		p->windchill = ws_windchill(p->temp, p->wind_speed);
	}
}

static void
calc_dewpoint(struct ws_loop* p)
{
	if (is_settable(p, WF_DEW_POINT, WF_TEMP | WF_HUMIDITY)) {
		p->wl_mask |= WF_DEW_POINT;
		p->dew_point = ws_dewpoint(p->temp, p->humidity);
	}
}

static void
calc_heat_index(struct ws_loop* p)
{
	if (is_settable(p, WF_HEAT_INDEX, WF_TEMP | WF_HUMIDITY)) {
		p->wl_mask |= WF_HEAT_INDEX;
		p->heat_index = ws_heat_index(p->temp, p->humidity);
	}
}

static void
calc_rain_rate(struct ws_loop* p)
{
	if (!ws_isset(p, WF_RAIN_RATE)) {
		int i, ticks;
		double rain_sum = 0;
		const struct ws_loop* prev;

		ticks = 0;

		if (ws_isset(p, WF_RAIN)) {
			ticks = 1;
			rain_sum = p->rain;
		}

		i = 0;
		prev = board_peek(i++);

		while (prev != NULL) {
			if (is_before(prev, p)) {
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

void
ws_compute(struct ws_loop *p)
{
	calc_windchill(p);
	calc_dewpoint(p);
	calc_heat_index(p);
	calc_rain_rate(p);
}
