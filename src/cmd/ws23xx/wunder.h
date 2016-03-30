#ifndef _WUNDER_H
#define _WUNDER_H

#include <stdint.h>
#include <time.h>

struct ws_wunder
{
	time_t time;						/* timestamp */

	uint16_t wind_dir;					/* wind direction */
	double wind_speed;					/* wind speed (m/s) */
	uint8_t humidity;					/* relative humidity */
	double dew_point;					/* dew point (°C) */
	double temp;						/* temperature (°C) */
	double rain;						/* accumulated rainfall (mm) in the past hour */
	double daily_rain;					/* accumulated rainfall (mm) today */

	double temp_in;						/* indoor temperature (°C) */
	uint8_t humidity_in;				/* indoor humidity (°C) */
};

int ws_wunder_upload(const struct ws_wunder *w);

#endif	/* _WUNDER_H */
