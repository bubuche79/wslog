#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void
usage(FILE *out)
{
}

int
main(int argc, char** argv)
{
	int c;
	int errflg = 0;

	/* Parse arguments */
	while ((c = getopt(argc, argv, "h")) != -1) {
		switch (c) {
		case 'h':
			break;

		case ':':
			fprintf(stderr, "Option -%c requires an operand\n", optopt);
			errflg++;
			break;

		case '?':
			fprintf(stderr, "Unrecognized option: '-%c'\n", optopt);
			errflg++;
			break;
		}
	}

	if (errflg) {
		usage(stderr);
		exit(1);
	}

	for ( ; optind < argc; optind++) {
	}

	exit(0);
}
