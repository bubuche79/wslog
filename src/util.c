#include <math.h>

#include "util.h"

/**
 * Calculate windchill using new post 2001 USA/Canadian formula
 * Twc = 13.112 + 0.6215*Ta -11.37*V^0.16 + 0.3965*Ta*V^0.16 [Celcius and km/h]
 *
 * @param speed the wind speed
 * @param temp the outdoor temperator
 * @return the windchill
 */
double
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
 * Calculate dewpoint
 * REF http://www.faqs.org/faqs/meteorology/temp-dewpoint/
 *
 * @param temp the outdoor temperator
 * @param humidity the outdoor humidity
 * @return the windchill
 */
double
ws_dewpoint(double temp, double humidity)
{
	double A = 17.2694;
	double B = (temp > 0) ? 237.3 : 265.5;
	double C = (A * temp) / (B + temp) + log(humidity / 100.0);

	return B * C / (A - C);
}