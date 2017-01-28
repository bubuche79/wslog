#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <stdio.h>

#include "libws/util.h"

#include "board.h"

static void
usage(FILE *std, const char *bin)
{
	fprintf(std, "Usage: %s [-l cnt] [-S]\n", bin);
}

static void
print_loop1(const struct ws_loop* p)
{
	char buf[32];

	strftimespec(buf, sizeof(buf), &p->time, 1);

	printf("%s %.1f°C %hhu%% %.1fm/s (%s) %.1fmm %.1f°C %hhu%%\n", buf, p->temp,
			p->humidity, p->wind_speed,
			p->wind_speed ? ws_dir(p->wind_dir) : "-", p->rain, p->temp_in,
			p->humidity_in);
}

static void
print_loop(size_t nel)
{
	int i;
	const struct ws_loop *p;

	i = 0;

	do {
		p = board_peek(i);

		if (p != NULL) {
			print_loop1(p);
		}

		i++;
	} while (p && (nel == 0 || i < nel));
}

static void
print_ar(size_t nel)
{
	int i;
	const struct ws_archive *p;

	i = 0;

	do {
		p = board_peek_ar(i);

		if (p != NULL) {
			print_loop1(&p->data);
		}

		i++;
	} while (p && (nel == 0 || i < nel));
}

int
main(int argc, char *argv[])
{
	int c;

	/* Default parameters */
	size_t nel = 10;
	int use_sensors = 0;

	(void) setlocale(LC_ALL, "C");

	/* Parse command line */
	while ((c = getopt(argc, argv, "l:S")) != -1) {
		switch (c) {
		case 'l':
			nel = atoi(optarg);
			break;
		case 'S':
			use_sensors = 1;
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
	if (use_sensors) {
		print_loop(nel);
	} else {
		print_ar(nel);
	}

	if (board_unlink() == -1) {
		goto error;
	}

	return 0;

error:
	perror("board\n");
	return 1;
}
