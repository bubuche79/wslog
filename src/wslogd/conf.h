#ifndef _CONF_H
#define _CONF_H

#include <sys/types.h>

#include "driver/driver.h"

/*
 * Weather station configuration.
 */

struct ws_conf
{
	uid_t uid;				/* Effective user id */
	gid_t gid;				/* Effective group id */
	const char *pid_file;			/* The daemon PID file */
	int log_facility;			/* Syslog facility */
	int log_mask;				/* Syslog priority mask */

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
		} vantage;
		struct
		{
			const char *tty;	/* TTY device */
		} ws23xx;
		struct
		{
			int hw_archive;		/* Turn on hardware archive */
			int io_delay;		/* I/O delay, in milliseconds */
		} simu;
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
		int https;			/* HTTPS flag */
		const char *station;		/* Station id */
		const char *password;		/* Account password */
		int freq;			/* Update frequency, in seconds */
	} wunder;
};

struct ws_conf *confp;

#ifdef __cplusplus
extern "C" {
#endif

int ws_getuid(const char *str, uid_t *uid);
int ws_getgid(const char *str, gid_t *gid);
int ws_getdriver(const char *str, enum ws_driver *driver);

int conf_load(const char *path);
void conf_free(void);

#ifdef __cplusplus
}
#endif

#endif /* _CONF_H */
