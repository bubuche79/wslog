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

#define WF_PRESSURE 	0x0001
#define WF_WIND_DIR 	0x0001
#define WF_WIND_SPEED	0x0002
#define WF_HUMIDITY		0x0004
#define WF_DEW_POINT	0x0008
#define WF_TEMP			0x0010
#define WF_RAIN_1H		0x0020
#define WF_RAIN_24H		0x0040
#define WF_TEMP_IN		0x0080
#define WF_HUMIDITY_IN	0x0100
#define WF_ALL			0xFFFF

struct ws_loop
{
	struct timespec time;		/* loop packet time */
	int wl_mask;				/* loop packet fields mask */

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

	struct ws_loop *loop;
	struct ws_archive *archive;
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
