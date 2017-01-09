#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <unistd.h>
#include <stdio.h>

#include "wslogd.h"
#include "board.h"
#include "csv.h"

static int fd = -1;
static int freq = 500;

static size_t
gmftime(char *s, size_t max, const time_t *timep, const char *fmt)
{
	struct tm tm;

	gmtime_r(timep, &tm);
	return strftime(s, max, fmt, &tm);
}

int
csv_init(void)
{
	freq = confp->csv.freq;

	fd = open(confp->csv.file, O_CREAT|O_WRONLY|O_APPEND, S_I644);
	if (fd == -1) {
		syslog(LOG_ERR, "open(%s): %m", confp->csv.file);
		return -1;
	}

	return 0;
}

int
csv_write(void)
{
	int ret;

	char ctime[22];					/* date utc */
	const struct ws_ws23xx *dat;

	dat = &boardp->buf[boardp->idx].ws23xx;

	/* Convert date */
	gmftime(ctime, sizeof(ctime), &dat->time, "%F %T");

	/* Write CSV line */
	ret = dprintf(fd, "%s,%hu,%.1f,%hu,%.1f,%.1f,%.1f,%.1f,%hu,%.1f\n",
			ctime, dat->wind_dir, dat->wind_speed, dat->humidity,
			dat->dew_point, dat->temp, dat->rain, dat->daily_rain,
			dat->humidity_in, dat->temp_in);
	if (ret == -1) {
		syslog(LOG_ERR, "dprintf(): %m");
	}

	return ret;
}

int
csv_destroy(void)
{
	int ret = 0;

	if (fd != -1) {
		if (close(fd) == -1) {
			syslog(LOG_ERR, "close: %m");
			ret = -1;
		}
	}

	return ret;
}
