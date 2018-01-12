#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <termios.h>
#ifdef DEBUG
#include <stdio.h>
#endif
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <time.h>
#include <fcntl.h>

#include "defs/dso.h"
#include "libws/serial.h"

static struct termios oldio;

long ws_io_delay = 50;

static void
msleep(long ms)
{
	struct timespec ts;

	ts.tv_sec = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000 * 1000;

	nanosleep(&ts, NULL);
}

DSO_EXPORT int
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
	if (tcgetattr(fd, &oldio) == -1) {
		goto error;
	}
	
	memset(&adtio, 0, sizeof(adtio));
	
	/* Serial control options */
	adtio.c_iflag = IGNBRK|IGNPAR;
	adtio.c_oflag = 0;
	adtio.c_cflag = CREAD|CLOCAL|CS8;
	adtio.c_lflag = 0;
	adtio.c_cc[VMIN] = 0;			/* no blocking read */
	adtio.c_cc[VTIME] = 0;			/* timer 0s */

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
#ifdef DEBUG
	perror("ws_open");
#endif

	if (fd != -1) {
		close(fd);
	}
	return -1;
}

DSO_EXPORT int
ws_close(int fd)
{
	if (tcsetattr(fd, TCSANOW, &oldio) == -1) {
		goto error;
	}
	if (close(fd) == -1) {
		goto error;
	}

	return 0;

error:
#ifdef DEBUG
	perror("ws_close");
#endif
	return -1;
}

DSO_EXPORT ssize_t
ws_read(int fd, void *buf, size_t nbyte)
{
	return ws_read_to(fd, buf, nbyte, 0);
}

DSO_EXPORT ssize_t
ws_read_to(int fd, void *buf, size_t nbyte, long timeout)
{
	int ret;
	fd_set readset;
	struct timeval tv, *ptv;

	/* Wait for input */
	do {
		FD_ZERO(&readset);
		FD_SET(fd, &readset);

		if (timeout) {
			tv.tv_sec = timeout / 1000;
			tv.tv_usec = (timeout - 1000 * tv.tv_sec) * 1000;
			ptv = &tv;
		} else {
			ptv = NULL;
		}

		ret = select(fd + 1, &readset, NULL, NULL, ptv);
	} while (ret == -1 && errno == EINTR);

	if (ret == -1) {
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
#ifdef DEBUG
	perror("ws_read_to");
#endif
	return -1;
}

DSO_EXPORT ssize_t
ws_write(int fd, const void *buf, size_t nbyte)
{
	ssize_t ret;

	if ((ret = write(fd, buf, nbyte)) == -1) {
		goto error;
	}
	if (tcdrain(fd) == -1) {
		goto error;
	}

	if (ws_io_delay > 0) {
		msleep(ws_io_delay);
	}

	return ret;

error:
#ifdef DEBUG
	perror("ws_write");
#endif
	return -1;
}

DSO_EXPORT ssize_t
ws_writev(int fd, const struct iovec *iov, size_t iovcnt)
{
	ssize_t ret;

	if ((ret = writev(fd, iov, iovcnt)) == -1) {
		goto error;
	}
	if (tcdrain(fd) == -1) {
		goto error;
	}

	if (ws_io_delay > 0) {
		msleep(ws_io_delay);
	}

	return ret;

error:
#ifdef DEBUG
	perror("ws_writev");
#endif
	return -1;
}

DSO_EXPORT int
ws_flush(int fd)
{
	if (tcflush(fd, TCIOFLUSH) == -1) {
		goto error;
	}

	return 0;

error:
#ifdef DEBUG
	perror("ws_flush");
#endif
	return -1;
}
