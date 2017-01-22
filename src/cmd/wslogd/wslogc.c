#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <unistd.h>
#include <locale.h>
#include <stdio.h>

#include "libws/util.h"

#include "board.h"

static void
usage(FILE *std, const char *bin)
{
	fprintf(std, "Usage: %s [-X] [-c conf_file]\n", bin);
}

static void
board_print(void)
{
	int i;

	for (i = 0; i < boardp->loop_nel; i++) {
		char buf[32];
		const struct ws_loop *p = board_loop_p(i);

		strftimespec(buf, sizeof(buf), &p->time);

		printf("%s %.2f°C %hhu%% %.2fm/s %.2f°C %hhu%%\n",
				buf,
				p->temp, p->humidity, p->wind_speed,
				p->temp_in, p->humidity_in);
	}
}

int
main(int argc, char *argv[])
{
	int c;
	int ret;
	int halt;

	/* Default parameters */
	const char *conf_file = "/etc/wslogd.conf";

	(void) setlocale(LC_ALL, "C");

	/* Parse command line */
	while ((c = getopt(argc, argv, "")) != -1) {
		switch (c) {
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
	if (pthread_mutex_lock(&boardp->mutex) == -1) {
		goto error;
	}

	board_print();

	if (pthread_mutex_unlock(&boardp->mutex) == -1) {
		goto error;
	}

	if (board_unlink() == -1) {
		goto error;
	}

	return 0;

error:
	perror("board\n");
	return 1;
}
