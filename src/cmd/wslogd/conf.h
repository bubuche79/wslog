#ifndef _CONF_H
#define _CONF_H

#include <sys/types.h>

/*
 * Weather station configuration.
 */

enum ws_driver
{
#if HAVE_WS23XX
	WS23XX,								/* La Crosse Technology WS23XX */
#endif
	NONE
};

struct ws_conf
{
	uid_t uid;							/* effective user id */
	gid_t gid;							/* effective group id */
	const char *pid_file;				/* the daemon PID file */
	int log_facility;					/* syslog facility */
	int log_mask;						/* syslog priority mask */

	struct
	{
		float latitude;					/* latitude */
		float longitude;				/* longitude */
		int altitude;					/* altitude (m) */
		enum ws_driver driver;			/* station type */
	} station;

	union {
		struct
		{
			const char *tty;			/* tty device */
			int freq;					/* read frequency, in seconds */
		} ws23xx;
	};

	struct
	{
		int disabled;					/* disabled flag */
		const char *db;					/* database file */
		int freq;						/* archive frequency, in seconds */
	} sqlite;

	struct
	{
		int disabled;					/* disabled flag */
		int https;						/* https flag */
		const char *station;			/* station id */
		const char *password;			/* account password */
		int freq;						/* update frequency, in seconds */
	} wunder;
};

#ifdef __cplusplus
extern "C" {
#endif

int conf_load(struct ws_conf *cfg, const char *path);
void conf_free(struct ws_conf *cfg);

#ifdef __cplusplus
}
#endif

#endif /* _CONF_H */
