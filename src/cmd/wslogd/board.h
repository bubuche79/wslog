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

#define WF_WIND_DIR 	0x0001
#define WF_WIND_SPEED	0x0002
#define WF_HUMIDITY		0x0004
#define WF_DEW_POINT	0x0008
#define WF_TEMP			0x0010
#define WF_RAIN_1H		0x0020
#define WF_RAIN_24H		0x0040
#define WF_TEMP_IN		0x0080
#define WF_HUMIDITY_IN	0x0100

struct ws_log
{
	time_t time;				/* timestamp */
	int log_mask;				/* fields mask */

	int wind_dir;				/* wind direction */
	float wind_speed;			/* wind speed (m/s) */
	int humidity;				/* relative humidity */
	float dew_point;			/* dew point (°C) */
	float temp;					/* temperature (°C) */
	float rain_1h;				/* accumulated rainfall (mm) in the past hour */
	float rain_24h;				/* accumulated rainfall (mm) today */

	float temp_in;				/* indoor temperature (°C) */
	int humidity_in;			/* indoor humidity (°C) */
};

struct ws_board
{
	pthread_mutex_t mutex;

	struct ws_log *buf;			/* buffer */
	size_t bufsz;				/* number of elements */
	size_t idx;					/* next used index */
};

struct ws_board *boardp;

int board_open(int oflag);
int board_unlink(void);

int board_get(struct ws_log *p);
int board_push(const struct ws_log *p);

int ws_isset(const struct ws_log *p, int mask);

#ifdef __cplusplus
}
#endif

#endif /* _BOARD_H */
