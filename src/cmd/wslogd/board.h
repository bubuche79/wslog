#ifndef _BOARD_H
#define _BOARD_H

#include <pthread.h>
#include <stdint.h>

/*
 * Shared board.
 */

#ifdef __cplusplus
extern "C" {
#endif

struct ws_ws23xx
{
	time_t time;						/* timestamp */

	uint16_t wind_dir;					/* wind direction */
	float wind_speed;					/* wind speed (m/s) */
	uint8_t humidity;					/* relative humidity */
	float dew_point;					/* dew point (°C) */
	float temp;							/* temperature (°C) */
	float rain;							/* accumulated rainfall (mm) in the past hour */
	float daily_rain;					/* accumulated rainfall (mm) today */

	float temp_in;						/* indoor temperature (°C) */
	uint8_t humidity_in;				/* indoor humidity (°C) */
};

struct ws_board
{
	pthread_rwlock_t rwlock;

	union {
		struct ws_ws23xx ws23xx;		/* buffer */
	} *buf;
	size_t bufsz;						/* number of elements */
	size_t idx;							/* next used index */
};

struct ws_board *boardp;

int board_open(int oflag);
int board_unlink(void);

struct ws_ws23xx *board_last(void);

#ifdef __cplusplus
}
#endif

#endif /* _BOARD_H */
