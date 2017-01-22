#ifndef _CORE_LOG_H
#define _CORE_LOG_H

#include <stdarg.h>

/*
 * Logging functions.
 */

#define IDENT_MAX	32	/* max. size of ident */

/* Log option */
#define LOG_PID		0x01	/* include process id with each message */
#define LOG_THREAD	0x02	/* include thread id with each message */
#define LOG_TERM	0x03	/* print to stdout and stderr as well */

/* Severity level */
#define LOG_EMERG	0		/* a panic condition was reported to all processes */
#define LOG_ALERT	1		/* a condition that should be corrected immediately */
#define LOG_CRIT	2		/* a critical condition */
#define LOG_ERR		3		/* an error message */
#define LOG_WARNING	4		/* a warning message */
#define LOG_NOTICE	5		/* a condition requiring special handling */
#define LOG_INFO	6		/* a general information message */
#define LOG_DEBUG	7		/* a message useful for debugging programs */

#ifndef LOG_MASK
#define LOG_MASK(p)	(1 << (p))
#endif

#ifndef LOG_UPTO
#define LOG_UPTO(p)	((1 << ((p) + 1)) - 1)
#endif

#define csyslog(level, format, ...) \
	do {\
		const char *_fmt = format; \
		syslog((level), _fmt, __VA_ARGS__); \
	} while (0)

#define csyslog1(level, format) \
	do {\
		const char *_fmt = format; \
		syslog((level), _fmt); \
	} while (0)

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The ws_openlog() function initialize the log stream. The `file' argument is
 * either of the form "syslog[:facility]" (POSIX only), or is set to a file 
 * path that will be opened in append mode. The `options' argument is formed
 * by OR-ing one or more of the following values:
 *   - LOG_PID. See above.
 *   - LOG_TERM. See above (not supported on syslog).
 */
void ws_openlog(const char *ident, int options);
void ws_closelog(void);
int ws_setlogmask(int mask);

/*
 * The ws_log() and ws_vlog() shall send a message to an
 * implementation-defined logging facility, like syslog(3) does. The
 * `priority' argument is set to a severity-level value.
 */
void ws_log(int priority, const char *format, /* args */ ...)
#if __GNUC__ >= 4
    __attribute__ ((format (printf, 2, 3)))
#endif
    ;
void ws_vlog(int priority, const char *format, va_list ap);

int ws_getlevel(const char *str, int *level);
int ws_getfacility(const char *str, int *facility);

#ifdef __cplusplus
}
#endif

#endif	/* _CORE_LOG_H */
