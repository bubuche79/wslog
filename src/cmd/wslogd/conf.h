#ifndef _CONF_H
#define _CONF_H

#include <sys/types.h>

/*
 * Weather station configuration.
 */

#ifdef __cplusplus
extern "C" {
#endif

struct ws_conf
{
	uid_t uid;							/* effective user id */
	gid_t gid;							/* effective group id */
	const char *pid_file;				/* the daemon PID file */
	int log_facility;					/* syslog facility */
	int log_mask;						/* syslog priority mask */

	const char *tty;					/* tty device */
	int freq;							/* read frequency, in seconds */

	struct
	{
		int disabled;					/* disabled flag */
		const char *file;				/* output file */
		int freq;						/* write frequency, in seconds */
	} csv;

	struct
	{
		int disabled;					/* disabled flag */
		const char *url;				/* wunderground.com protocol upload url*/
		const char *user_id;			/* wunderground id */
		const char *user_pwd;			/* wunderground password */
		int freq;						/* write frequency, in seconds */
	} wunder;
};

int conf_load(struct ws_conf *cfg, const char *path);
void conf_free(struct ws_conf *cfg);

#ifdef __cplusplus
}
#endif

#endif /* _CONF_H */
