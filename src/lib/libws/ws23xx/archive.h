#ifndef _LIBWS23XX_ARCHIVE_H
#define _LIBWS23XX_ARCHIVE_H

#define WS32XX_AR_SIZE 175

struct ws23xx_ar
{
	time_t tstamp;

	float temp_in;
	float temp_out;
	float abs_pressure;
	uint8_t humidity_in;
	uint8_t humidity_out;
	float rain;
	float wind_speed;
	float wind_dir;
};

#ifdef __cplusplus
extern "C" {
#endif

ssize_t ws23xx_fetch_ar(int fd, struct ws23xx_ar *h, size_t nel);

#ifdef __cplusplus
}
#endif

#endif	/* _LIBWS23XX_ARCHIVE_H */