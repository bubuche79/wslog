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

double round_scale(double v, int scale);
const char *ws_dir(int idx);
const char *ws_dir_deg(int degree);

size_t gmftime(char *s, size_t max, const time_t *timep, const char *fmt);
size_t localftime_r(char *s, size_t max, const time_t *timep, const char *fmt);
ssize_t strftimespec(char *s, size_t max, const struct timespec *ts, int width);

ssize_t ws_read_all(const char *pathname, void *buf, size_t len);
ssize_t ws_write_all(const char *pathname, const void *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif	/* _CORE_UTIL_H */
