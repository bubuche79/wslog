/*
 * Formatted error messages.
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "defs/dso.h"
#include "libws/err.h"

DSO_EXPORT void
die(int status, const char *fmt, /* args */ ...)
{
	va_list ap;

	va_start(ap, fmt);
	(void) vfprintf(stderr, fmt, ap);
	va_end(ap);

	exit(status);
}
