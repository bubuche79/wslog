#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "util.h"
#include "cli.h"

static int
is_dashdash(const char *opt)
{
	return opt[0] == '-' && opt[1] == '-' && !opt[2];
}

static int
is_shortopt(const char *opt)
{
	return opt[0] == '-' && opt[1] && opt[1] != '-';
}

static int
is_longopt(const char *opt)
{
	return opt[0] == '-' && opt[1] == '-' && opt[2];
}

static const char *
next_arg(int i, int argc, const char **argv)
{
	return i + 1 < argc ? argv[i + 1] : NULL;
}

static struct opt *
opt_find(struct opt *spec, int type)
{
	int i;

	for (i = 0; spec[i].type != WS_NONE && spec[i].type != WS_COMMAND; i++) {
		/* Next one */
	}

	return spec[i].type != WS_NONE ? &spec[i] : NULL;
}

static struct opt *
opt_find_short(struct opt *spec, char name)
{
	int i, found;

	for (i = 0, found = 0; spec[i].type != WS_NONE && !found; i++) {
		found = spec[i].short_name == name;
	}

	return found ? &spec[i - 1] : NULL;
}

static struct opt *
opt_find_long(struct opt *spec, const char *name)
{
	int i, found;

	for (i = 0, found = 0; spec[i].type != WS_NONE && !found; i++) {
		found = spec[i].long_name != NULL && !strcmp(name, spec[i].long_name);
	}

	return found ? &spec[i - 1] : NULL;
}

static int
parse_value(struct opt *opt, const char *arg, int *used)
{
	const char *s;
	int unset = 0;

	*used = 0;

	switch (opt->type) {
	case WS_OPT_BIT:
		if (unset) {
			*(int *)opt->value &= ~opt->defval;
		} else {
			*(int *)opt->value |= opt->defval;
		}
		break;

	case WS_OPT_INT:
		if (arg == NULL) {
			fprintf(stderr, "%s requires a value\n", opt->long_name);
			return -1;
		}

		*used = 1;
		*(int *)opt->value = strtol(arg, (char **)&s, 10);

		if (*s) {
			fprintf(stderr, "%s expects a numerical value\n", opt->long_name);
			return -1;
		}
		break;

	case WS_OPT_STRING:
		if (arg == NULL) {
			fprintf(stderr, "%s requires a value\n", opt->long_name);
			return -1;
		}

		*used = 1;
		*(char **)opt->value = arg;
		break;

	default:
		break;
	}

	return 0;
}

int
opt_parse(int argc, const char **argv, struct opt *spec)
{
	int i, is_dash_dash, used;
	const char *val;
	struct opt *opt, *cmd;

	is_dash_dash = 0;
	cmd = opt_find(spec, WS_COMMAND);

	for (i = 1; i < argc && !is_dash_dash; i++) {
		const char *arg = argv[i];

		if (is_dashdash(arg)) {
			is_dash_dash = 1;
		} else if (is_shortopt(arg)) {
			used = 0;

			while (*++arg && !used) {
				opt = opt_find_short(spec, *arg);
				val = arg[1] ? arg + 1 : next_arg(i, argc, argv);

				if (opt) {
					parse_value(opt, val, &used);

					if (!arg[1]) {
						i += used;
					}
				} else {
					goto error;
				}
			}
		} else if (is_longopt(arg)) {
			arg += 2;

			opt = opt_find_long(spec, arg);
			val = next_arg(i, argc, argv);

			if (opt) {
				parse_value(opt, val, &used);
				i += used;
			} else {
				goto error;
			}
		} else if (cmd != NULL) {
			*(char **)cmd->value = arg;
			is_dash_dash = 1;
		} else {
			--i;
			is_dash_dash = 1;
		}
	}

	return i;

error:
	return -1;
}

ssize_t
opt_fprintf(FILE *stream, const char *progname, const struct opt *spec)
{
	return -1;
}

const struct opt_cmd *
opt_find_cmd(const struct opt_cmd *spec, const char *name)
{
	int i;

	for (i = 0; spec[i].name != NULL && strcmp(spec[i].name, name); i++) {
		/* Next one */
	}

	return spec[i].name != NULL ? &spec[i] : NULL;
}
