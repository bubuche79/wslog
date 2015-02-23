#ifndef _UTIL_H
#define _UTIL_H

#define WS_HISTORY_SIZE	175

struct ws_history
{
	double temp_in;
	double temp_out;
	double abs_pressure;
	int humidity_in;
	int humidity_out;
	double rain;
	double wind_speed;
	double wind_dir;
};

extern int ws_fetch_history(int fd, struct ws_history *h, size_t nel);

#endif	/* _UTIL_H */
