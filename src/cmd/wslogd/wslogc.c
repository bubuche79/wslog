#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <stdio.h>

#include "libws/util.h"

#include "board.h"

static size_t nel;

static void
usage(FILE *std, const char *bin)
{
	fprintf(std, "Usage: %s [-l cnt] [-c conf_file]\n", bin);
}

static void
board_print(void)
{
	int i;
	const struct ws_loop *p = NULL;

	for (i = 0; p && (nel == 0 || i < nel); i++) {
		char buf[32];

		p = board_peek(i);

		if (p != NULL) {
			strftimespec(buf, sizeof(buf), &p->time);

			printf("%s %.1f°C %hhu%% %.1fm/s %.1fmm %.1f°C %hhu%%\n",
					buf,
					p->temp, p->humidity, p->wind_speed, p->rain,
					p->temp_in, p->humidity_in);
		}
	}
}

int
main(int argc, char *argv[])
{
	int c;

	/* Default parameters */
	nel = 0;
//	const char *conf_file = "/etc/wslogd.conf";

	(void) setlocale(LC_ALL, "C");

	/* Parse command line */
	while ((c = getopt(argc, argv, "l:c:")) != -1) {
		switch (c) {
		case 'l':
			nel = atoi(optarg);
			break;
		case 'c':
//			conf_file = optarg;
			break;
		default:
			usage(stderr, argv[0]);
			return 2;
		}
	}

	if (argc != optind) {
		usage(stderr, argv[0]);
		return 2;
	}

	/* Open shared board */
	if (board_open(0) == -1) {
		goto error;
	}

	/* Display */
	board_print();

	if (board_unlink() == -1) {
		goto error;
	}

	return 0;

error:
	perror("board\n");
	return 1;
}
