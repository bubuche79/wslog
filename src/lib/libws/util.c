#include <math.h>

#include "defs/dso.h"

#include "util.h"

/**
 * Calculate windchill using new post 2001 USA/Canadian formula
 * Twc = 13.112 + 0.6215*Ta -11.37*V^0.16 + 0.3965*Ta*V^0.16 [Celcius and km/h]
 *
 * @param speed the wind speed
 * @param temp the outdoor temperator
 * @return the windchill
 */
DSO_EXPORT double
ws_windchill(double speed, double temp)
{
	double windchill;
	double wind_kmph = 3.6 * speed;

	if (wind_kmph > 4.8) {
		windchill = 13.112 + 0.6215 * temp
				- 11.37 * pow(wind_kmph, 0.16)
		        + 0.3965 * temp * pow(wind_kmph, 0.16);
	} else {
		windchill = temp;
	}

	return windchill;
}

/**
 * Calculate dewpoint.
 *
 * REF http://www.faqs.org/faqs/meteorology/temp-dewpoint/
 *
 * @param temp the outdoor temperator
 * @param humidity the outdoor humidity
 * @return the dew point
 */
DSO_EXPORT double
ws_dewpoint(double temp, double humidity)
{
	double A = 17.2694;
	double B = (temp > 0) ? 237.3 : 265.5;
	double C = (A * temp) / (B + temp) + log(humidity / 100.0);

	return B * C / (A - C);
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

DSO_EXPORT size_t
gmftime(char *s, size_t max, const time_t *timep, const char *fmt)
{
	struct tm tm;

	gmtime_r(timep, &tm);
	return strftime(s, max, fmt, &tm);
}
