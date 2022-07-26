#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "libws/serial.h"
#include "libws/vantage/util.h"
#include "libws/vantage/vantage.h"

#define BAUDRATE	B19200		/* Default baudrate */

static ssize_t
wakeup_read(int fd, void *buf, size_t len)
{
	struct timespec ts;

	ts.tv_sec = 1;
	ts.tv_nsec = 200000000;

	return vantage_read_to(fd, buf, len, &ts);
}

int
vantage_open(const char *device)
{
	return ws_open(device, BAUDRATE);
}

int
vantage_close(int fd)
{
	return ws_close(fd);
}

int
vantage_wakeup(int fd)
{
	int retry;
	const uint8_t in[] = { LF };
	const uint8_t resp[] = { LF, CR };

	uint8_t buf[2];
	size_t bufsz = 2;

	for (retry = 0; retry < 3; retry++) {
		ssize_t sz;

		/* Discard pending data */
		if (tcflush(fd, TCIOFLUSH) == -1) {
			goto error;
		}

		/* Write LF */
		if (ws_write(fd, in, 1) == -1) {
			goto error;
		}

		/* Expect LF CR result */
		memset(buf, 0, bufsz);
		sz = wakeup_read(fd, buf, bufsz);

		if (sz == -1) {
			if (errno != ETIME) {
				goto error;
			}
		} else if (sz == bufsz) {
			if (memcmp(buf, resp, sz) == 0) {
				break;
			}
		}
	}

	if (retry == 3) {
		errno = EIO;
		goto error;
	}

	return 0;

error:
	return -1;
}
