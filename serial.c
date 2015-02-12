#include <termio.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/file.h>

#include "serial.h"
#include "ws2300.h"

#define BAUDRATE B2400

static struct termios oldio;

int
ws_open(const char *device)
{
	int fd = -1;
	struct termios adtio;
	int portstatus, fdflags;

	//Setup serial port
	if ((fd = open(device, O_RDWR)) == -1) {
		goto error;
	}
	
	if (flock(fd, LOCK_EX) == -1) {
		goto error;
	}
	
//	if ((fdflags = fcntl(fd, F_GETFL)) == -1 ||
//	     fcntl(fd, F_SETFL, fdflags & ~O_NONBLOCK) < 0)
//	{
//		perror("couldn't reset non-blocking mode");
//		exit(EXIT_FAILURE);
//	}

	/* Save current settings */
	if (tcgetattr(fd, &oldio) == -1) {
		goto error;
	}
	
	//We want full control of what is set and simply reset the entire adtio struct
	memset(&adtio, 0, sizeof(adtio));
	
	//tcgetattr(fd, &adtio);   // Commented out and replaced by the memset above
	
	// Serial control options
//	adtio.c_cflag &= ~PARENB;      // No parity
//	adtio.c_cflag &= ~CSTOPB;      // One stop bit
//	adtio.c_cflag &= ~CSIZE;       // Character size mask
//	adtio.c_cflag |= CS8;          // Character size 8 bits
//	adtio.c_cflag |= CREAD;        // Enable Receiver
//	adtio.c_cflag &= ~HUPCL;       // No "hangup"
//	adtio.c_cflag |= HUPCL;       // No "hangup"
//	adtio.c_cflag &= ~CRTSCTS;     // No flowcontrol
//	adtio.c_cflag |= CLOCAL;       // Ignore modem control lines

	adtio.c_cflag = CREAD|HUPCL|CLOCAL|CS8|B2400;

	// Baudrate, for newer systems
	if (cfsetispeed(&adtio, BAUDRATE) == -1) {
		goto error;
	}
	if (cfsetospeed(&adtio, BAUDRATE) == -1) {
		goto error;
	}
	
	// Serial local options: adtio.c_lflag
	// Raw input = clear ICANON, ECHO, ECHOE, and ISIG
	// Disable misc other local features = clear FLUSHO, NOFLSH, TOSTOP, PENDIN, and IEXTEN
	// So we actually clear all flags in adtio.c_lflag
	adtio.c_lflag = 0;

	// Serial input options: adtio.c_iflag
	// Disable parity check = clear INPCK, PARMRK, and ISTRIP 
	// Disable software flow control = clear IXON, IXOFF, and IXANY
	// Disable any translation of CR and LF = clear INLCR, IGNCR, and ICRNL	
	// Ignore break condition on input = set IGNBRK
	// Ignore parity errors just in case = set IGNPAR;
	// So we can clear all flags except IGNBRK and IGNPAR
//	adtio.c_iflag = IGNBRK|IGNPAR;
	adtio.c_iflag = INPCK;
	
	// Serial output options
	// Raw output should disable all other output options
//	adtio.c_oflag &= ~OPOST;
	adtio.c_oflag = 0;

	adtio.c_cc[VMIN] = 1;		// blocking read until 1 char
	adtio.c_cc[VTIME] = 0;		// timer 1s

	if (tcflush(fd, TCIOFLUSH) == -1) {
		goto error;
	}
	if (tcflow(fd, TCOON|TCION) == -1) {
		goto error;
	}

	if (tcsetattr(fd, TCSAFLUSH, &adtio) == -1) {
		goto error;
	}

	// Set DTR low and RTS high and leave other ctrl lines untouched

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
	fd_set readset;
	struct timeval tv;

	/* Wait for input */
	for (;;) {
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

	if (DEBUG) printf("ws_read_byte: %d\n", ret);

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

	if (DEBUG) printf("ws_write: %d\n", ret);

	return 0;

error:
	perror("ws_write");
	return -1;
}

int
ws_clear(int fd)
{
	if (tcflush(fd, TCIOFLUSH) == -1) {
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

