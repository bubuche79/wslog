#ifndef _HISTORY_H
#define _HISTORY_H

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

	time_t tstamp;				/* sample timestamp (computed) */
	double windchill;			/* sample windchill (computed) */
	double dewpoint;			/* sample dewpoint (computed) */
};

ssize_t ws_fetch_history(int fd, struct ws_history *h, size_t nel);

#endif	/* _HISTORY_H */
