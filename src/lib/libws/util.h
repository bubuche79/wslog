#ifndef _CORE_UTIL_H
#define _CORE_UTIL_H

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

double ws_windchill(double speed, double temp);
double ws_dewpoint(double temp, double humidity);

double ws_fahrenheit(double temp);
double ws_mph(double speed);
double ws_inch(double len);

size_t gmftime(char *s, size_t max, const time_t *timep, const char *fmt);

#ifdef __cplusplus
}
#endif

#endif	/* _CORE_UTIL_H */
