#ifndef _CONFIG_H
#define _CONFIG_H

/*
 * Weather station configuration.
 */

#ifdef __cplusplus
extern "C" {
#endif

struct ws_conf
{
	const char *tty;					/* tty device */

	int log_facility;					/* syslog facility */
	int log_mask;						/* syslog priority mask */

	struct
	{
		int disabled;					/* disabled flag */
		const char *file;				/* output file */
	} csv;

	struct
	{
		int disabled;					/* disabled flag */
		const char *url;				/* wunderground.com protocol upload url*/
		const char *user_id;			/* wunderground id */
		const char *user_pwd;			/* wunderground password */
		int freq;						/* refresh frequency, in seconds */
	} wunder;
};

int ws_conf_load(struct ws_conf *cfg, const char *path);

#ifdef __cplusplus
}
#endif

#endif /* _CONFIG_H */
