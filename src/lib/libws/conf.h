#ifndef _CORE_CONF_H
#define _CORE_CONF_H

#include <stddef.h>

/*
 * Configuration functions.
 */

#ifdef __cplusplus
extern "C" {
#endif

int strtoint(const char *str, int *val);
int strtobool(const char *str, int *val);
int strtostr(const char *str, char *buf, size_t buflen);

/*
 * The cfg_parse() function parses a configuration file and calls a
 * user defined function on definition lines. The file is composed of lines
 * of the following format:
 *
 *     input = eol | comments | definition
 *     eol = "\n"
 *     comments = "#" .* eol
 *     definition = key "=" value eol
 *     key = [a-z][a-z0-9._]*
 *     value = [a-zA-Z0-9 ]*[a-zA-Z0-9]
 *
 * The usrcfg() function is called on all `definition' lines. The file parsing
 * stops as soon as a line does not match one of the supported formats above,
 * or the usrcfg() function returns -1.
 */
int conf_parse(const char *path, int *lineno,
    int (*usrcfg)(void *, const char *, const char *), void *arg);

#ifdef __cplusplus
}
#endif

#endif /* _CORE_CONF_H */
