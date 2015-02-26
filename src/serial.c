#include <termio.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#ifdef HAVE_SELECT
#include <sys/select.h>
#include <sys/ioctl.h>
#endif	/* HAVE_SELECT */
#include <sys/file.h>
#include <time.h>

#include "serial.h"
#include "util.h"
#include "ws2300.h"

#define BAUDRATE B2400

static struct termios oldio;

long ws_io_delay = 50;

static void
msleep(long ms) {
	struct timespec ts;

	ts.tv_sec = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000 * 1000;

	nanosleep(&ts, NULL);
}

int
ws_open(const char *device)
{
	int fd = -1;
	struct termios adtio;
	int portstatus;

	if ((fd = open(device, O_RDWR)) == -1) {
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
	adtio.c_iflag = INPCK;
	adtio.c_oflag = 0;
	adtio.c_cflag = CREAD|HUPCL|CLOCAL|CS8|BAUDRATE;
	adtio.c_lflag = 0;
#ifdef HAVE_SELECT
	adtio.c_cc[VMIN] = 1;			/* blocking read until 1 char */
	adtio.c_cc[VTIME] = 0;			/* timer 0s */
#else
	adtio.c_cc[VMIN] = 0;			/* no blocking read */
	adtio.c_cc[VTIME] = 10;			/* timer 1s */
#endif /* HAVE_SELECT */

	if (cfsetispeed(&adtio, BAUDRATE) == -1) {
		goto error;
	}
	if (cfsetospeed(&adtio, BAUDRATE) == -1) {
		goto error;
	}

	if (tcflush(fd, TCIOFLUSH) == -1) {
		goto error;
	}
	if (tcflow(fd, TCOON|TCION) == -1) {
		goto error;
	}

	if (tcsetattr(fd, TCSAFLUSH, &adtio) == -1) {
		goto error;
	}

	/* Set DTR low and RTS high and leave other ctrl lines untouched */
	if (ioctl(fd, TIOCMGET, &portstatus) == -1) {
		goto error;
	}

	portstatus &= ~TIOCM_DTR;
	portstatus |= TIOCM_RTS;

	if (ioctl(fd, TIOCMSET, &portstatus) == -1) {
		goto error;
	}

	return fd;

error:
	perror("ws_open");

	if (fd != -1) {
		close(fd);
	}
	return -1;
}

int
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
	perror("ws_close");
	return -1;
}

int
ws_read_byte(int fd, uint8_t *byte, long timeout)
{
	int ret;

#ifdef HAVE_SELECT
	/* Wait for input */
	fd_set readset;
	struct timeval tv;

	do {
		FD_ZERO(&readset);
		FD_SET(fd, &readset);

		tv.tv_sec = timeout / 1000;
		tv.tv_usec = (timeout % 1000) * 1000;

		ret = select(fd + 1, &readset, NULL, NULL, &tv);
	} while (ret == -1 && errno == EINTR);

	if (ret == -1) {
		goto error;
	}

	/* Read input */
	if (FD_ISSET(fd, &readset)) {
		ret = read(fd, byte, 1);
		if (ret == -1) {
			goto error;
		}
	} else {
		ret = 0;
	}
#else
	ret = read(fd, byte, 1);
	if (ret == -1) {
		goto error;
	}
#endif	/* HAVE_SELECT */

#if DEBUG >= 2
	printf("ws_read_byte: %d\n", ret);
#endif	/* DEBUG */

	return ret;

error:
	perror("ws_read_byte");
	return -1;
}

int
ws_write_byte(int fd, uint8_t byte)
{
	int ret;

	if ((ret = write(fd, &byte, 1)) == -1) {
		goto error;
	}
	if (tcdrain(fd) == -1) {
		goto error;
	}

	if (ws_io_delay > 0) {
		msleep(ws_io_delay);
	}

#if DEBUG >= 2
	printf("ws_write_byte: %d\n", ret);
#endif	/* DEBUG */

	return ret;

error:
	perror("ws_write");
	return -1;
}

int
ws_clear(int fd)
{
	if (tcflush(fd, TCIFLUSH) == -1) {
		goto error;
	}

	return 0;

error:
	perror("ws_clear");
	return -1;
}

int
ws_flush(int fd)
{
	if (tcdrain(fd) == -1) {
		goto error;
	}

	return 0;

error:
	perror("ws_flush");
	return -1;
}

