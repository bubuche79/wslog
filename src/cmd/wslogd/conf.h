#ifndef _CONF_H
#define _CONF_H

#include <sys/types.h>

#include "driver/driver.h"

/*
 * Weather station configuration.
 */

struct ws_conf {
	uid_t uid;							/* effective user id */
	gid_t gid;							/* effective group id */
	const char *pid_file;				/* the daemon PID file */
	int log_facility;					/* syslog facility */
	int log_mask;						/* syslog priority mask */

	struct {
		float latitude;					/* latitude */
		float longitude;				/* longitude */
		int altitude;					/* altitude (m) */
		enum ws_driver driver;			/* station type */
	} station;

	struct {
		int freq;						/* sensor frequency, in seconds */

		struct {
			const char *tty;			/* tty device */
		} ws23xx;
	} driver;

	struct {
		int freq;						/* archive frequency, in seconds */
		int delay;						/* archive delay, in seconds */

		struct {
			int enabled;				/* disabled flag */
			const char *db;				/* database file */
		} sqlite;
	} archive;

	struct {
		int enabled;					/* disabled flag */
		int https;						/* https flag */
		const char *station;			/* station id */
		const char *password;			/* account password */
		int freq;						/* update frequency, in seconds */
	} wunder;
};

const struct ws_conf *confp;

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
