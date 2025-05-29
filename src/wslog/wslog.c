#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "libws/defs.h"
#include "libws/cli.h"
#include "builtin.h"

#define PROGNAME 	"wslog"

static int help = 0;
static const char *cmd = NULL;

static struct opt wslog_opts[] = {
	OPT_BIT('h', "help", &help, "display help and exit", 1),
	OPT_COMMAND("cmd", &cmd, "wslog command"),
	OPT_END()
};

static struct opt_cmd wslog_cmds[] = {
	CMD_COMMAND("ic-omm", cmd_ic_omm, "infoclimat summary"),
	CMD_END()
};

static void
wslog_usage(FILE *stream)
{
	opt_fprintf(stream, PROGNAME, wslog_opts);
}

int
main(int argc, const char **argv)
{
	int i, status, arg_idx;
	const struct opt_cmd *subcmd;

	arg_idx = opt_parse(argc, argv, wslog_opts);

	if (arg_idx == -1) {
		goto error;
	}

	if (help) {
		wslog_usage(stdout);
		status = 0;
	} else {
		if ((subcmd = opt_find_cmd(wslog_cmds, cmd)) == NULL) {
			goto error;
		}

		for (i = 0; i < argc - arg_idx; i++) {
			argv[1 + i] = argv[arg_idx + i];
		}

		status = subcmd->fn(argc - arg_idx + 1, argv);
	}

	exit(status);

error:
	wslog_usage(stderr);
	exit(1);
}
