#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <math.h>
#include <stdio.h>

#include "defs/dso.h"

#include "util.h"

static const char *wind_dir[] =
{
	"N",
	"NNE",
	"NE",
	"ENE",
	"E",
	"ESE",
	"SE",
	"SSE",
	"S",
	"SSW",
	"SW",
	"WSW",
	"W",
	"WNW",
	"NW",
	"NNW"
};

static double
ws_celsius(double temp)
{
	return (temp - 32) / 1.8;
}

/**
 * Compute wind chill using new post 2001 USA/Canadian formula.
 *
 * The {@code temp} is the temperature in Celcius, and {@code speed} the wind
 * speed in m/s.
 *
 * https://en.wikipedia.org/wiki/Wind_chill
 */
DSO_EXPORT double
ws_windchill(double temp, double speed)
{
	double wc;
	double wind_kmph = 3.6 * speed;

	if (temp < 10 && wind_kmph > 4.8) {
		wc = 13.12 + 0.6215 * temp +(0.3965 * temp - 11.37) * pow(wind_kmph, 0.16);
	} else {
		wc = temp;
	}

	return wc;
}

/**
 * Compute dew point temperature, using enhanced Bögel formula.
 *
 * The {@code temp} parameter is in Celsius, and {@code hr} parameter is the
 * relative humidity, in percent.
 *
 * https://en.wikipedia.org/wiki/Dew_point
 */
DSO_EXPORT double
ws_dewpoint(double temp, double hr)
{
	double b, c, d;
	double lambda;

	d = 234.5;

	if (temp >= 0) {
		b = 17.368;
		c = 238.88;
	} else {
		b = 17.966;
		c = 247.15;
	}

	lambda = log(hr/100.0 * exp((b - temp/d) * (temp / (c + temp))));

	return (c * lambda) / (b - lambda);
}

DSO_EXPORT double
ws_heat_index(double temp, double hr)
{
	double hi;
	double temp_f = ws_fahrenheit(temp);

	if (temp_f < 80.0 || hr < 40.0) {
		return temp;
	}

	hi = -42.379 + 2.04901523*temp_f + 10.14333127*hr + -0.22475541*temp_f*hr
			+ -6.83783e-3*temp_f*temp_f + -5.481717e-2*hr*hr + 1.22874e-3*temp_f*temp_f*hr
			+ 8.5282e-4*temp_f*hr*hr + -1.99e-6*temp_f*temp_f*hr*hr;

	return ws_celsius(hi);
}

/**
 * Computes humidex.
 *
 * The {@code temp} parameter is in Celsius, and {@code hr} parameter is the
 * relative humidity, in percent.
 *
 * https://en.wikipedia.org/wiki/Humidex
 */
DSO_EXPORT double
ws_humidex(double temp, double hr)
{
	double dp_k = 273.16 + ws_dewpoint(temp, hr);

	return temp + 0.5555 * (6.11 * exp(5417.7530 * (1 / 273.16 - 1 / (dp_k))) - 10);
}

/**
 * Converts pressure from hPa to inHg.
 */
DSO_EXPORT double
ws_inhg(double p)
{
	return p / (1013.25 / 29.92);
}

/**
 * Converts temperature from Celsius to Fahrenheit.
 *
 * @param temp temperature, celsius
 * @return temperature, fahrenheit
 */
DSO_EXPORT double
ws_fahrenheit(double temp)
{
	return 1.8 * temp + 32;
}

/**
 * Converts speed from m/s to miles/hour.
 *
 * @param speed speed, m/s
 * @return speed, miles/hour
 */
DSO_EXPORT double
ws_mph(double speed)
{
	return speed / 0.44704;
}

/**
 * Converts length from mm to inches.
 *
 * @param len length, millimeter
 * @return length, inches
 */
DSO_EXPORT double
ws_inch(double len)
{
	return len / 25.4;
}

DSO_EXPORT const char *
ws_dir(double dir)
{
	return wind_dir[(int) (dir / 22.5)];
}

DSO_EXPORT size_t
gmftime(char *s, size_t max, const time_t *timep, const char *fmt)
{
	struct tm tm;

	gmtime_r(timep, &tm);
	return strftime(s, max, fmt, &tm);
}

DSO_EXPORT ssize_t
strftimespec(char *s, size_t len, const struct timespec *ts, int width)
{
    size_t ret1;
    int ret2;
    long xsec;
    struct tm t;

    if (localtime_r(&ts->tv_sec, &t) == NULL) {
        return -1;
    }

    ret1 = strftime(s, len, "%F %T", &t);
    if (ret1 == 0) {
    	return 0;
    } else if (ret1 == (size_t) -1) {
    	return -1;
    }

	xsec = ts->tv_nsec / (long) pow(10, 9 - width);
	ret2 = snprintf(s + ret1, len - ret1, ".%0*ld", width, xsec);
    if (ret2 == -1) {
    	return -1;
    }

    return ret1 + ret2;
}
