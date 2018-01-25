#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include "defs/dso.h"

#include "util.h"

#define ZERO_K 273.15

static const char *dir[] =
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
	"NNW",
	"-"
};

static double
ws_celsius(double temp)
{
	return (temp - 32) / 1.8;
}

/**
 * Compute sea level pressure from station pressure.
 *
 * The {@code pressure} is the pressure in hPa, the {@code temp} is the outdoor
 * temperature in Celcius, and {@code elev} the station elevation in meters.
 *
 * https://en.wikipedia.org/wiki/Barometric_formula
 * https://fr.wikipedia.org/wiki/Formule_du_nivellement_barom%C3%A9trique
 *
 * TODO: verify barometric formula.
 */
DSO_EXPORT double
ws_barometer(double pressure, double temp, double elev)
{
	double temp_k = ZERO_K + temp;

	return pressure / exp(-elev / (temp_k * 29.271759));
}

/**
 * Convert from (uncorrected) station pressure to altitude-corrected pressure.
 *
 * The {@code pressure} is the absolute pressure in hPa, and {@code elev} the
 * station elevation in meters.
 *
 * http://www.wrh.noaa.gov/slc/projects/wxcalc/formulas/altimeterSetting.pdf
 */
DSO_EXPORT double
ws_altimeter(double pressure, double elev)
{
	double c = 0.190284;
	double delta = pressure - 0.3;
	double CONST = pow(1013.25, c) * 0.0065 / 288;

	return delta * pow(1 + CONST * (elev / pow(delta, c)), 1 / c);
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
	double dp_k = ZERO_K + ws_dewpoint(temp, hr);

	return temp + 0.5555 * (6.11 * exp(5417.7530 * (1 / 273.16 - 1 / (dp_k))) - 10);
}

/**
 * Convert pressure from hPa to inHg.
 */
DSO_EXPORT double
ws_inhg(double p)
{
	return p * 0.02953;
}

/**
 * Convert temperature from Celsius to Fahrenheit.
 */
DSO_EXPORT double
ws_fahrenheit(double temp)
{
	return 1.8 * temp + 32;
}

/**
 * Convert speed from m/s to miles/hour.
 */
DSO_EXPORT double
ws_mph(double speed)
{
	return speed / 0.44704;
}

/**
 * Convert length from mm to inches.
 */
DSO_EXPORT double
ws_in(double len)
{
	return len / 25.4;
}

DSO_EXPORT double
round_scale(double v, int scale)
{
	long pow10[] = { 1, 10, 100, 1000 };
	long ratio = pow10[scale];

	return round(v * ratio) / ratio;
}

DSO_EXPORT const char *
ws_dir(int idx)
{
	if (idx > 16) {
		idx = 16;
	}

	return dir[idx];
}

DSO_EXPORT const char *
ws_dir_deg(int degree)
{
	return ws_dir(degree / 22.5);
}

DSO_EXPORT size_t
gmftime(char *s, size_t max, const time_t *timep, const char *fmt)
{
	struct tm tm;

	gmtime_r(timep, &tm);
	return strftime(s, max, fmt, &tm);
}

DSO_EXPORT size_t
localftime_r(char *s, size_t max, const time_t *timep, const char *fmt)
{
	struct tm tm;

	localtime_r(timep, &tm);
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

DSO_EXPORT ssize_t
ws_read_all(const char *pathname, void *buf, size_t len)
{
	int fd;
	ssize_t sz;

	if ((fd = open(pathname, O_RDONLY)) == -1) {
		goto error;
	}
	if ((sz = read(fd, buf, len)) == -1) {
		goto error;
	}
	if (close(fd) == -1) {
		goto error;
	}

	return sz;

error:
	if (fd != -1) {
		(void) close(fd);
	}
	return -1;
}

DSO_EXPORT ssize_t
ws_write_all(const char *pathname, const void *buf, size_t len)
{
	int fd;
	ssize_t sz;

	int flags = O_CREAT|O_TRUNC|O_WRONLY;
	mode_t mode = S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH;

	if ((fd = open(pathname, flags, mode)) == -1) {
		goto error;
	}
	if ((sz = write(fd, buf, len)) == -1) {
		goto error;
	}
	if (close(fd) == -1) {
		goto error;
	}

	return sz;

error:
	if (fd != -1) {
		(void) close(fd);
	}
	return -1;
}

DSO_EXPORT void
ws_timespec(struct timespec *ts, long ms)
{
	ts->tv_sec = ms / 1000;
	ts->tv_nsec = ms - 1000 * ts->tv_sec;
}
