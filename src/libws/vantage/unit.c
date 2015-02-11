#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "libws/util.h"
#include "libws/vantage/util.h"

static const double pow10d[] = { 1.0, 10.0, 100.0, 1000.0 };

double
vantage_val(int v, int scale)
{
	return v / pow10d[scale];
}

double
vantage_temp(int f, int scale)
{
	return (f / pow10d[scale] - 32) * 5 / 9;
}

double
vantage_pressure(int p, int scale)
{
	return (p / pow10d[scale]) / 0.02952998751;
}

double
vantage_rain(int ticks, int rain_cup)
{
	double r;

	if (rain_cup == 0) {
		r = 0.01 * 25.4;
	} else if (rain_cup == 1) {
		r = 0.2;
	} else {
		r = 0.1;
	}

	return r * ticks;
}

double
vantage_meter(int inch, int scale)
{
	return (inch / pow10d[scale]) * 25.4 * 1000;
}

double
vantage_speed(int mph, int scale)
{
	return (mph / pow10d[scale]) * 1.609344 / 3.6;
}

const char *
vantage_dir(int idx)
{
	return ws_dir(idx);
}
