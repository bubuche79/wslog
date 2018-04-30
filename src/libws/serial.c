#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <termios.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <time.h>
#include <fcntl.h>

#include "libws/util.h"
#include "libws/serial.h"

int
ws_open(const char *device, speed_t speed)
{
	int fd = -1;
	struct termios adtio;

	if ((fd = open(device, O_RDWR|O_NOCTTY)) == -1) {
		goto error;
	}

	if (flock(fd, LOCK_EX) == -1) {
		goto error;
	}

	/* Save current settings */
	if (tcgetattr(fd, &adtio) == -1) {
		goto error;
	}
	
	/* Serial control options */
#if 0
	memset(&adtio, 0, sizeof(adtio));

	adtio.c_iflag = IGNBRK|IGNPAR|IGNCR;
	adtio.c_oflag = 0;
	adtio.c_cflag = CREAD|CLOCAL|CS8;
	adtio.c_lflag = 0;
#else
	cfmakeraw(&adtio);
#endif

	adtio.c_cc[VMIN] = 0;			/* No blocking read */
	adtio.c_cc[VTIME] = 0;			/* Timer 0s */

	(void) cfsetispeed(&adtio, speed);
	(void) cfsetospeed(&adtio, speed);

	if (tcsetattr(fd, TCSANOW, &adtio) == -1) {
		goto error;
	}

	if (tcflush(fd, TCIOFLUSH) == -1) {
		goto error;
	}

	return fd;

error:
	if (fd != -1) {
		close(fd);
	}
	return -1;
}

int
ws_close(int fd)
{
	if (close(fd) == -1) {
		goto error;
	}

	return 0;

error:
	return -1;
}

ssize_t
ws_read(int fd, void *buf, size_t nbyte)
{
	return ws_read_to(fd, buf, nbyte, NULL);
}

ssize_t
ws_read_to(int fd, void *buf, size_t nbyte, const struct timespec *timeout)
{
	int ret;
	fd_set readset;

	/* Wait for input */
	FD_ZERO(&readset);
	FD_SET(fd, &readset);

	if (pselect(fd + 1, &readset, NULL, NULL, timeout, NULL) == -1) {
		goto error;
	}

	/* Read input */
	if (FD_ISSET(fd, &readset)) {
		ret = read(fd, buf, nbyte);
		if (ret == -1) {
			goto error;
		}
	} else {
		ret = 0;
	}

	return ret;

error:
	return -1;
}

ssize_t
ws_write(int fd, const void *buf, size_t nbyte)
{
	ssize_t ret;

	if ((ret = write(fd, buf, nbyte)) == -1) {
		goto error;
	}
	if (tcdrain(fd) == -1) {
		goto error;
	}

	return ret;

error:
	return -1;
}

ssize_t
ws_writev(int fd, const struct iovec *iov, size_t iovcnt)
{
	ssize_t ret;

	if ((ret = writev(fd, iov, iovcnt)) == -1) {
		goto error;
	}
	if (tcdrain(fd) == -1) {
		goto error;
	}

	return ret;

error:
	return -1;
}

int
ws_flush(int fd)
{
	if (tcflush(fd, TCIOFLUSH) == -1) {
		goto error;
	}

	return 0;

error:
	return -1;
}
