#ifndef _WS2300_H
#define _WS2300_H

#include <unistd.h>
#include <stdint.h>

#include "decoder.h"

#ifndef DEBUG
#define DEBUG 0
#endif

#define SETBIT			0x12
#define UNSETBIT		0x32
#define WRITENIB		0x42

enum ws_etype {
	WS_TEMP,
	WS_PRESSURE,
	WS_HUMIDITY,
	WS_RAIN,
	WS_WIND_DIR,
	WS_WIND_VELOCITY,
	WS_SPEED,
	WS_INT_SEC,
	WS_INT_MIN,
	WS_BIN_2NYB,
	WS_CONTRAST,
	WS_DATE,							/* write only */
	WS_TIMESTAMP,
	WS_DATETIME,
	WS_TIME,							/* write only */
	WS_CONNECTION,
	WS_FORECAST,
	WS_TENDENCY,
	WS_SPEED_UNIT,
	WS_WIND_OVERFLOW,
	WS_WIND_VALID,
	WS_ALARM_SET_0,
	WS_ALARM_SET_1,
	WS_ALARM_SET_2,
	WS_ALARM_SET_3,
	WS_ALARM_ACTIVE_0,
	WS_ALARM_ACTIVE_1,
	WS_ALARM_ACTIVE_2,
	WS_ALARM_ACTIVE_3,
	WS_BUZZER,
	WS_BACKLIGHT
};

struct ws_type {
	const enum ws_etype id;				/* internal id */
	const char *units;					/* units name (eg hPa) */
	const uint8_t nybble;				/* nybble count */
	const char *desc;					/* type description (eg pressure) */

	union {
		struct {
			const uint8_t scale;
		} num;
		struct {
			struct {
				const uint8_t key;
				const char *value;
			} a[5];
		} text;
		struct {
			const char *unset;
			const char *set;
		} bit;
	};
};

struct ws_measure
{
	const uint16_t addr;				/* nnybble addr */
	const char *id;						/* short name */
	const struct ws_type *type;			/* data converter type */
	const char *desc;					/* long name */
	const char *reset;					/* id of measure to reset this one */
};

extern int ws_reset_06(int fd);
extern int ws_write_addr(int fd, uint16_t addr);

extern int ws_write_data(int fd, uint16_t addr, size_t len, uint8_t op, const uint8_t *buf);
extern int ws_write_safe(int fd, uint16_t addr, size_t len, uint8_t op, const uint8_t *buf);
extern int ws_read_data(int fd, uint16_t addr, size_t nnybble, uint8_t *buf);
extern int ws_read_safe(int fd, uint16_t addr, size_t nnybble, uint8_t *buf);
extern int ws_read_batch(int fd, const uint16_t *addr, const size_t *nnybble, size_t sz, uint8_t *buf[]);

#endif	/* ws2300.h */
