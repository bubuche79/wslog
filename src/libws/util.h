#ifndef _CORE_UTIL_H
#define _CORE_UTIL_H

#include <time.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

double ws_barometer(double pressure, double temp, double elev);
double ws_altimeter(double pressure, double elev);

double ws_windchill(double speed, double temp);
double ws_dewpoint(double temp, double humidity);
double ws_heat_index(double temp, double hr);
double ws_humidex(double temp, double hr);

double ws_inhg(double p);
double ws_fahrenheit(double temp);
double ws_mph(double speed);
double ws_in(double len);

const char *ws_dir(double dir);

size_t gmftime(char *s, size_t max, const time_t *timep, const char *fmt);
ssize_t strftimespec(char *s, size_t max, const struct timespec *ts, int width);

#ifdef __cplusplus
}
#endif

#endif	/* _CORE_UTIL_H */
