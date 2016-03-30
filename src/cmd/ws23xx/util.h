#ifndef _UTIL_H
#define _UTIL_H

double ws_windchill(double speed, double temp);
double ws_dewpoint(double temp, double humidity);

double ws_fahrenheit(double temp);
double ws_mph(double speed);
double ws_inch(double len);

#endif	/* _UTIL_H */
