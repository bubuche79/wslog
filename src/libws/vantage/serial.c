#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <errno.h>

#include "defs/dso.h"

#include "libws/serial.h"
#include "libws/vantage/vantage.h"

#define BAUDRATE	B19200		/* default baudrate */
#define WAKEUP_TO	1200		/* wakeup timeout, in milliseconds */

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
	uint8_t in[] = { LF };
	uint8_t resp[] = { LF, CR };

	uint8_t buf[2];
	size_t bufsz = 2;

	for (retry = 0; retry < 3; retry++) {
		ssize_t sz;

		/* write LF */
		if (ws_write(fd, in, 1) == -1) {
			goto error;
		}

		/* expect LF CR result */
		memset(buf, 0, bufsz);
		sz = ws_read_to(fd, buf, bufsz, WAKEUP_TO);

		if (sz == -1) {
			goto error;
		} else if (sz == bufsz) {
			if (memcmp(buf, resp, sz) == 0) {
				break;
			} else {
				errno = EIO;
				goto error;
			}
		}
	}

	if (retry == 0) {
		errno = EIO;
		goto error;
	}

	return 0;

error:
	return -1;
}
