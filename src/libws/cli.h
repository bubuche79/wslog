#ifndef _LIBWS_CLI_H
#define _LIBWS_CLI_H

#include <stdint.h>
#include <sys/types.h>

enum opt_type {
	WS_NONE,
	WS_COMMAND,
	/* Without arguments */
	WS_OPT_BIT,
	/* With arguments */
	WS_OPT_INT,
	WS_OPT_STRING
};

struct opt {
	enum opt_type type;

	char short_name;
	const char *long_name;
	void *value;
	const char *help;

	intptr_t defval;
};

struct opt_cmd {
	const char *name;
	int (*fn)(int, const char **);
	const char *help;
};

#define OPT_BIT(s, l, v, h, d)	{ WS_OPT_BIT, (s), (l), (v), (h), (d) }
#define OPT_INT(s, l, v, h)	{ WS_OPT_INT, (s), (l), (v), (h) }
#define OPT_STRING(s, l, v, h)	{ WS_OPT_STRING, (s), (l), (v), (h) }
#define OPT_COMMAND(l, v, h)	{ WS_COMMAND, 0, (l), (v), (h) }
#define OPT_END()		{ WS_NONE }

#define CMD_COMMAND(n, fn, h)	{ (n), (fn), (h) }
#define CMD_END()		{ NULL }

#ifdef __cplusplus
extern "C" {
#endif

int opt_parse(int, const char **, struct opt *);
ssize_t opt_fprintf(FILE *, const char *, const struct opt *);

const struct opt_cmd *opt_find_cmd(const struct opt_cmd *, const char *);

#ifdef __cplusplus
}
#endif

#endif	/* _LIBWS_CLI_H */
