#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#include "libws/conf.h"

#define iseol(c) ((c) == 0 || (c) == '#' || (c) == '\n' || (c) == '\r')
#define iskey1(c) (islower(c))
#define iskey2(c) (iskey1(c) || isdigit(c) || (c) == '_' || (c) == '.')
#define isvalue(c) (isalnum(c) || ispunct(c))

static int
conftok(char *line, char **name, char **value)
{
	int equal_ok;
	size_t i, e, n, v;

	*name = NULL;
	*value = NULL;

	/* Name token */
	i = 0;
	while (isblank(line[i])) {
		++i;
	}
	if (iseol(line[i])) {
		return 0;
	}

	if (line[i] && !iskey1(line[i])) {
		return -1;
	} else {
		n = i;
		while (iskey2(line[i])) {
			++i;
		}
	}

	if (iseol(line[i])) {
		return -1;
	}

	equal_ok = (line[i] == '=');
	line[i++] = 0;

	/* String value token */
	if (!equal_ok) {
		while (isblank(line[i])) {
			++i;
		}
		if (line[i] != '=') {
			return -1;
		}
		++i;
	}
	while (isblank(line[i])) {
		++i;
	}

	v = i;

	do {
		while (isvalue(line[i])) {
			++i;
		}

		e = i;

		if (!iseol(line[i])) {
			while (isblank(line[i])) {
				++i;
			}
		}
	} while (!iseol(line[i]));

	line[e] = 0;

	*name = line + n;
	*value = line + v;

	return 0;
}

int
ws_getint(const char *str, int *val)
{
	char *eptr;
	long res;

	errno = 0;
	res = strtol(str, &eptr, 10);

	if ((res == LONG_MIN || res == LONG_MAX) && errno) {
		return -1;
	} else if (res < INT_MIN || INT_MAX < res) {
		errno = ERANGE;
		return -1;
	} else if (*eptr != 0) {
		errno = EINVAL;
		return -1;
	}

	*val = res;
	return 0;
}

int
ws_getlong(const char *str, long *val)
{
	char *eptr;
	long res;

	errno = 0;
	res = strtol(str, &eptr, 10);

	if ((res == LONG_MIN || res == LONG_MAX) && errno) {
		return -1;
	} else if (*eptr != 0) {
		errno = EINVAL;
		return -1;
	}

	*val = res;
	return 0;
}

int
ws_getfloat(const char *str, float *val)
{
	char *eptr;
	float res;

	errno = 0;
	res = strtof(str, &eptr);

	if (res == 0 && errno) {
		return -1;
	} else if ((res == -HUGE_VALF || res == HUGE_VALF) && errno) {
		return -1;
	} else if (*eptr != 0) {
		errno = EINVAL;
		return -1;
	}

	*val = res;
	return 0;
}

int
ws_getbool(const char *str, int *val)
{
	if (!strcmp(str, "yes") || !strcmp(str, "1")) {
		*val = 1;
	} else if (!strcmp(str, "no") || !strcmp(str, "0")) {
		*val = 0;
	} else {
		errno = EINVAL;
		return -1;
	}

	return 0;
}

int
ws_parse_config(const char *path, int *lineno,
    int (*usrcfg)(void *, const char *, const char *), void *arg)
{
	int errsv;
	FILE *stream;
	char linebuf[LINE_MAX];

	/* Read configuration file */
	if ((stream = fopen(path, "r")) == NULL) {
		return -1;
	}

	*lineno = 0;

	while (fgets(linebuf, sizeof(linebuf), stream) != NULL) {
		int ret;
		char *n, *v;

		++(*lineno);

		if (linebuf[0] == '#' || linebuf[0] == '\n') {
			continue;
		}

		ret = conftok(linebuf, &n, &v);
		if (ret == -1) {
			errno = EINVAL;
			goto error;
		} else if (n == NULL && v == NULL) {
			continue;
		}

		/* Decode name & value */
		if (usrcfg(arg, n, v) == -1) {
			goto error;
		}
	}

	return fclose(stream);

error:
	errsv = errno;
	(void) fclose(stream);

	errno = errsv;
	return -1;
}
