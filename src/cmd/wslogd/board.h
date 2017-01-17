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

#define WF_BAROMETER 			0x0001
#define WF_PRESSURE 			0x0002
#define WF_TEMP					0x0004
#define WF_HUMIDITY				0x0008
#define WF_WIND_SPEED			0x0010
#define WF_WIND_DIR 			0x0020
#define WF_WIND_GUST			0x0040
#define WF_WIND_GUST_DIR		0x0080
#define WF_RAIN_RATE			0x0100
#define WF_RAIN_1H				0x0200
#define WF_RAIN_24H				0x0400
#define WF_DEW_POINT			0x0800
#define WF_WINDCHILL			0x1000
#define WF_HEAD_INDEX			0x2000
#define WF_TEMP_IN				0x4000
#define WF_HUMIDITY_IN			0x8000
#define WF_ALL					0xFFFF

#define WF_WIND					0x38F0

struct ws_loop
{
	struct timespec time;		/* loop packet time */
	uint32_t wl_mask;			/* loop packet fields mask */

	float barometer;			/* relative pressure (hPa) */
	float abs_pressure;			/* absolute pressure (hPa) */
	float temp;					/* temperature (°C) */
	uint8_t humidity; 			/* humidity (%) */
	float wind_speed;			/* wind speed (m/s) */
	uint16_t wind_dir;			/* wind direction (°) */
	float wind_gust;			/* wind gust (m/s) */
	uint16_t wind_gust_dir;		/* wind gust direction (°) */
	float rain_rate;			/* rain rate (mm/hr) */
	float rain_1h;				/* accumulated rain in the past hour (mm) */
	float rain_24h;				/* accumulated rain in the past 24 hours (mm) */
#if 0
	float sampleRain;			/* ! inches               */
	float sampleET;				/* ! ET */
	uint16_t radiation;			/* solar radiation (W/m³) */
	float uv;					/* UV index */
#endif
	float dew_point; 			/* dew point (°C) */
	float windchill;			/* windchill temperature (°C) */
	float heat_idx;				/* head index (°C) */

	float temp_in;				/* indoor temperature (°C) */
	uint8_t humidity_in;		/* indoor humidity (%) */
};

struct ws_archive
{
	time_t time;				/* record time */
	time_t interval;			/* archive interval, in seconds */
	struct ws_loop data;		/* aggregated data */
};

struct ws_board
{
	pthread_mutex_t mutex;

	struct ws_loop *loop;
	size_t loop_sz;
	size_t loop_idx;			/* next index */

	struct ws_archive *ar;
	size_t ar_sz;
	size_t ar_idx;				/* next used index */
};

struct ws_board *boardp;

int board_open(int oflag);
int board_unlink(void);

int board_get_ar(struct ws_archive *p);
int board_push_ar(const struct ws_archive *p);

int board_push_loop(const struct ws_loop *p);

int ws_isset(const struct ws_loop *p, int mask);

#ifdef __cplusplus
}
#endif

#endif /* _BOARD_H */
