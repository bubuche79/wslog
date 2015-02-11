#include <stdio.h>
#include <stdint.h>
#include <errno.h>

#include "serial.h"
#include "ws2300.h"

#define MAX_RESETS 100
#define READ_TIMEOUT 1000

static struct measure mem_map[] = {
	{ 0x346, "it", TEMP, "in temp" }
};

/**
 * Write a reset string and wait for a reply.
 */
int
ws_reset_06(int fd)
{
	uint8_t answer;
	int ret;
	int success;

	for (int i = 0; i < MAX_RESETS; i++) {
		if (ws_clear(fd) == -1) {
			goto error;
		}
		if (ws_write_byte(fd, 0x06) == -1) {
			goto error;
		}

		/*
		 * Occasionally 0, then 2 is returned.  If 0 comes back, continue
		 * reading as this is more efficient than sending an out-of sync
		 * reset and letting the data reads restore synchronization.
		 * Occasionally, multiple 2's are returned. Read with a fast
		 * timeout until all data is exhausted, if we got a 2 back at all,
		 * we consider it a success.
		 */
		ret = ws_read_byte(fd, &answer, 1000);
		if (ret == -1) {
			goto error;
		}

		success = 0;

		while (ret > 0) {
			if (answer == 0x02) {
				success = 1;
			}

			ret = ws_read_byte(fd, &answer, 50);
		}

		if (success) {
			return 0;
		}

		if (DEBUG) printf("ws_reset_06: %d\n", i);
	}

	errno = EAGAIN;

error:
	perror("ws_reset_06");
	return -1;
}

