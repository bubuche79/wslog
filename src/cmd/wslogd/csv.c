#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>

#include "wslogd.h"
#include "csv.h"

static int fd = -1;
static int freq = 500;

int
csv_init(void)
{
	freq = confp->csv.freq;

	fd = open(confp->csv.file, O_CREAT|O_WRONLY|O_APPEND, S_I644);
	if (fd == -1) {
		syslog("open(%s): %m", confp->csv.file);
		return -1;
	}

	return 0;
}

int
csv_run(void)
{
	int halt = 0;

	while (!halt) {
		dprintf(fd, "OK\n");

		sleep(freq);
	}

	return 0;
}

int
csv_destroy()
{
	int ret = 0;

	if (fd != -1) {
		if (close(fd) == -1) {
			syslog("close: %m");
			ret = -1;
		}
	}

	return ret;
}
