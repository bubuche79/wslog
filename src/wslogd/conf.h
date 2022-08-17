#ifndef _CONF_H
#define _CONF_H

#include <termios.h>

#include "driver/driver.h"

/*
 * Weather station configuration.
 */

#define WS_CONF_SQLITE_DB "/var/lib/wslog/wslogd.db"

struct ws_conf
{
	int log_facility;			/* Syslog facility */
	int log_level;				/* Syslog level */

	struct
	{
		float latitude;			/* Latitude */
		float longitude;		/* Longitude */
		int altitude;			/* Altitude (m) */
		enum ws_driver driver;		/* Station type */
	} station;

	struct
	{
		long freq;			/* Sensor frequency, in milliseconds */

		struct
		{
			const char *tty;	/* TTY device */
			speed_t baud;		/* Console baud rate */
		} vantage;
		struct
		{
			const char *tty;	/* TTY device */
		} ws23xx;
		struct
		{
			int hw_archive;		/* Turn on hardware archive */
			long io_delay;		/* I/O delay, in milliseconds */
		} virt;
	} driver;

	struct
	{
		int freq;			/* Archive frequency, in seconds */
		int delay;			/* Archive delay, in seconds */

		struct
		{
			int enabled;		/* Enabled flag */
			const char *db;		/* Database file */
		} sqlite;
	} archive;

	struct
	{
		int enabled;			/* Enabled flag */
		int freq;			/* Synchronization frequency, in seconds */
		int max_drift;			/* Max drift, in seconds */
	} sync;

	struct
	{
		int enabled;			/* Enabled flag */
		const char *station;		/* Station id */
		const char *username;		/* Username id */
		const char *password;		/* Account password */
		int freq;			/* Update frequency, in seconds */
	} stat_ic;

	struct
	{
		int enabled;			/* Enabled flag */
		int https;			/* HTTPS flag */
		const char *station;		/* Station id */
		const char *password;		/* Account password */
		int freq;			/* Update frequency, in seconds */
	} wunder;
};

extern struct ws_conf *confp;

#ifdef __cplusplus
extern "C" {
#endif

int ws_getuid(const char *str, uid_t *uid);
int ws_getgid(const char *str, gid_t *gid);
int ws_getdriver(const char *str, enum ws_driver *driver);
int ws_getlevel(const char *str, int *level);
int ws_getfacility(const char *str, int *facility);

int conf_load(const char *path);
void conf_free(void);

#ifdef __cplusplus
}
#endif

#endif /* _CONF_H */
