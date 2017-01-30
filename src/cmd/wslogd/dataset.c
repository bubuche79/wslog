#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "libws/util.h"

#include "dataset.h"

static int
is_settable(const struct ws_loop *p, int mask, int flag)
{
	return (p->wl_mask & (mask | flag)) == flag;
}

int
ws_isset(const struct ws_loop *p, int flag)
{
	return (p->wl_mask & flag) == flag;
}

void
ws_compute(struct ws_loop *p)
{
	if (is_settable(p, WF_WINDCHILL, WF_WIND|WF_TEMP)) {
		p->wl_mask |= WF_WINDCHILL;
		p->windchill = ws_windchill(p->temp, p->wind_speed);
	}
	if (is_settable(p, WF_DEW_POINT, WF_TEMP|WF_HUMIDITY)) {
		p->wl_mask |= WF_DEW_POINT;
		p->dew_point = ws_dewpoint(p->temp, p->humidity);
	}
	if (is_settable(p, WF_HEAT_INDEX, WF_TEMP|WF_HUMIDITY)) {
		p->wl_mask |= WF_HEAT_INDEX;
		p->heat_index = ws_heat_index(p->temp, p->humidity);
	}
}
