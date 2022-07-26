#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "libws/util.h"

#include "board.h"
#include "conf.h"

#define PROGNAME	"wslogc"

static void
usage(FILE *std, int status)
{
	fprintf(std, "Usage: " PROGNAME " [-l cnt] [-h] [-V] [-S] [-c config]\n");

	exit(status);
}

static void
print_loop(const struct ws_loop *p)
{
	char buf[20];

	localftime_r(buf, sizeof(buf), &p->time.tv_sec, "%F %T");

	printf("%s %.1f°C %hhu%% %.1fm/s (%s) %.1fmm %.1f°C %hhu%%\n", buf, p->temp,
			p->humidity, p->wind_speed,
			p->wind_speed ? ws_dir_deg(p->wind_dir) : "-",
			p->rain_day, p->in_temp, p->in_humidity);
}

static void
dump_loop(size_t nel)
{
	int i;
	const struct ws_loop *p;

	i = 0;

	do {
		p = board_peek(i);

		if (p != NULL) {
			print_loop(p);
		}

		i++;
	} while (p && (nel == 0 || i < nel));
}

static void
print_ar(const struct ws_archive *p)
{
	char buf[20];

	localftime_r(buf, sizeof(buf), &p->time, "%F %T");

	printf("%s %.1f°C %hhu%% %.1fm/s (%s) %.1fmm %.1f°C %hhu%%\n", buf, p->temp,
			p->humidity, p->avg_wind_speed, ws_dir_deg(p->avg_wind_dir),
			p->rain_fall, p->in_temp, p->in_humidity);
}

static void
dump_ar(size_t nel)
{
	int i;
	const struct ws_archive *p;

	i = 0;

	do {
		p = board_peek_ar(i);

		if (p != NULL) {
			print_ar(p);
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
	const char *config = "/etc/wslogd.conf";

	(void) setlocale(LC_ALL, "C");

	/* Parse command line */
	while ((c = getopt(argc, argv, "hVc:l:S")) != -1) {
		switch (c) {
		case 'c':
			config = optarg;
			break;
		case 'l':
			nel = atoi(optarg);
			break;
		case 'S':
			use_sensors = 1;
			break;
		case 'h':
			usage(stdout, 0);
			break;
		case 'V':
			printf(PROGNAME " (" PACKAGE ") " VERSION "\n");
			exit(0);
			break;
		default:
			usage(stderr, 2);
			break;
		}
	}

	if (argc != optind) {
		usage(stderr, 2);
	}

	if (conf_load(config) == -1) {
		goto error;
	}

	/* Open shared board */
	if (board_open(0) == -1) {
		fprintf(stderr, "boad_open: %s\n", strerror(errno));
		goto error;
	}

	/* Display */
	if (use_sensors) {
		dump_loop(nel);
	} else {
		dump_ar(nel);
	}

	if (board_unlink() == -1) {
		fprintf(stderr, "board_unlink: %s\n", strerror(errno));
		goto error;
	}

	exit(0);

error:
	exit(1);
}
