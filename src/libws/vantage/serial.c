#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "defs/dso.h"

#include "libws/serial.h"
#include "libws/vantage/vantage.h"

#define BAUDRATE	B19200		/* Default baudrate */
#define WAKEUP_TO	1200		/* Wakeup timeout, in milliseconds */

DSO_EXPORT int
vantage_open(const char *device)
{
	return ws_open(device, BAUDRATE);
}

DSO_EXPORT int
vantage_close(int fd)
{
	return ws_close(fd);
}

DSO_EXPORT int
vantage_wakeup(int fd)
{
	int retry;
	const uint8_t in[] = { LF };
	const uint8_t resp[] = { LF, CR };

	uint8_t buf[2];
	size_t bufsz = 2;

	for (retry = 0; retry < 3; retry++) {
		ssize_t sz;

		/* Write LF */
		if (ws_write(fd, in, 1) == -1) {
			goto error;
		}

		/* Expect LF CR result */
		memset(buf, 0, bufsz);
		sz = ws_read_to(fd, buf, bufsz, WAKEUP_TO);

		if (sz == -1) {
			goto error;
		} else if (sz == bufsz) {
			if (memcmp(buf, resp, sz) == 0) {
				break;
			}

			/* Sometimes console sends { CR LF CR }, retry */
			if (tcflush(fd, TCIFLUSH) == -1) {
				goto error;
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
