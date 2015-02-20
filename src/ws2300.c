#include <stdio.h>
#include <errno.h>

#include "serial.h"
#include "ws2300.h"

#define MAX_RESETS 		100
#define MAX_RETRIES		50
#define MAX_BLOCKS		80
#define READ_TIMEOUT	1000			/* milliseconds */

#define SETACK			0x04
#define UNSETACK		0x0C
#define WRITEACK		0x10

#define min(a,b)		((a) < (b) ? (a) : (b))

static void
encode(uint8_t encode_constant, const uint8_t *in, uint8_t *out, size_t len)
{
	for (int i = 0; i < len; i++) {
		out[i] = encode_constant + (in[i] * 4);
	}
}

static uint8_t
checksum(const uint8_t *buf, size_t len)
{
	int checksum = 0;

	for (int i = 0; i < len; i++) {
		checksum += buf[i];
	}

	checksum &= 0xFF;

	return (uint8_t) checksum;
}

static int
read_byte(int fd, uint8_t *buf, long timeout)
{
	int ret;

	ret = ws_read_byte(fd, buf, timeout);
	if (ret == -1) {
		goto error;
	} else if (ret == 0) {
		errno = EAGAIN;
		goto error;
	}

	return 0;

error:
	perror("read_byte");
	return -1;
}

static int
write_byte(int fd, uint8_t byte, uint8_t ack)
{
	int ret;
	uint8_t answer;

	ret = ws_write_byte(fd, byte);
	if (ret == -1) {
		goto error;
	} else if (ret == 0) {
		errno = EIO;
		goto error;
	}

	/* Check ack */
	ret = ws_read_byte(fd, &answer, READ_TIMEOUT);
	if (ret == -1) {
		goto error;
	} else if (ret == 0) {
		errno = ENODATA;
		goto error;
	} else if (ack != answer) {
		fprintf(stderr, "Expected ack %x, got %x\n", ack, answer);
		errno = EIO;
		return -1;
	}

	return 0;

error:
	perror("write_byte");
	return -1;
}

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
		ret = ws_read_byte(fd, &answer, READ_TIMEOUT);
		if (ret == -1) {
			goto error;
		}

		success = 0;

		while (ret > 0) {
			if (answer == 0x02) {
				success = 1;
			}

			ret = ws_read_byte(fd, &answer, 50);
			if (ret == -1) {
				goto error;
			}
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

/**
 * Write an address.
 *
 * @return 0 on success, -1 on error
 */
int
ws_write_addr(int fd, uint16_t addr)
{
	uint8_t byte, ack;

	for (int i = 0; i < 4; i++) {
		byte = (addr >> (4 * (3 - i)) & 0xF) * 4 + 0x82;
		ack = i * 16 + (byte - 0x82) / 4;

		if (write_byte(fd, byte, ack) == -1) {
			goto error;
		}
	}

	return 0;

error:
	perror("ws_write_addr");
	return -1;
}

/**
 * Write data, checking the reply.
 *
 * @return 0 on success, -1 on error
 */
int
ws_write_data(int fd, uint16_t addr, size_t len, uint8_t encode_constant, const uint8_t *buf)
{
	size_t max_len;
	uint8_t ack_constant;
	uint8_t encoded_data[MAX_BLOCKS];

	switch (encode_constant) {
	case SETBIT:
		max_len = 1;
		ack_constant = SETACK;
		break;
	case UNSETBIT:
		max_len = 1;
		ack_constant = UNSETACK;
		break;
	case WRITENIB:
		max_len = sizeof(encoded_data);
		ack_constant = WRITEACK;
		break;
	default:
		errno = EINVAL;
		goto error;
		break;
	}

	if (max_len < len) {
		errno = EINVAL;
		goto error;
	}

	if (ws_write_addr(fd, addr) == -1) {
		goto error;
	}

	encode(encode_constant, buf, encoded_data, len);

	for (int i = 0; i < len; i++) {
		uint8_t ack = buf[i] + ack_constant;

		if (write_byte(fd, encoded_data[i], ack) == -1) {
			goto error;
		}
	}

	return 0;

error:
	perror("ws_write_data");
	return -1;
}

/**
 * Reset the device and write a command, verifing it was written correctly.
 */
int
ws_write_safe(int fd, uint16_t addr, size_t len, uint8_t encode_constant, const uint8_t *buf)
{
	for (int i = 0; i < MAX_RETRIES; i++) {
		if (ws_reset_06(fd) == -1) {
			goto error;
		}

		if (ws_write_data(fd, addr, len, encode_constant, buf) == 0) {
			return 0;
		}
	}

	return 0;

error:
	perror("ws_write_safe");
	return -1;
}

int
ws_read_data(int fd, uint16_t addr, size_t nybble, uint8_t *buf)
{
	uint8_t answer;

	if (nybble == 0 || nybble > MAX_BLOCKS) {
		errno = EINVAL;
		goto error;
	}

	/* Write address to read from */
	if (ws_write_addr(fd, addr) == -1) {
		goto error;
	}

	/* Write the number of bytes we want to read */
	size_t nbyte = (nybble + 1) / 2;
	uint8_t byte = 0xC2 + nbyte * 4;
	uint8_t ack = 0x30 + nbyte;

	if (write_byte(fd, byte, ack)) {
		goto error;
	}

	/* Read the response */
	for (int i = 0; i < nbyte; i++) {
		if (read_byte(fd, buf + i, READ_TIMEOUT) == -1) {
			goto error;
		}
	}

	/* Read and verify checksum */
	if (read_byte(fd, &answer, READ_TIMEOUT) == -1) {
		goto error;
	}
	if (answer != checksum(buf, nbyte)) {
		goto error;
	}

	return 0;

error:
	perror("ws_read_data");
	return -1;
}

static int
read_block(int fd, uint16_t addr, size_t nybble, uint8_t *buf)
{
	int p, k;

	if (DEBUG) printf("read_block %x (%lu)\n", addr, nybble);

	for (p = 0; p < nybble; p += MAX_BLOCKS) {
		for (k = 0; k < MAX_RETRIES; k++) {
			int chunk = min(MAX_BLOCKS, nybble - p);

			if (ws_read_data(fd, addr + p, chunk, buf + p) == 0) {
				break;
			}
			if (ws_reset_06(fd) == -1) {
				goto error;
			}
		}

		if (k == MAX_RETRIES) {
			errno = EAGAIN;
			goto error;
		}
	}

	return 0;

error:
	perror("ws_read_safe");
	return -1;
}

int
ws_read_batch(int fd, const uint16_t *addr, const size_t *nybble, size_t sz, uint8_t *buf[])
{
	int i;

	if (ws_reset_06(fd) == -1) {
		goto error;
	}

	for (i = 0; i < sz; i++) {
		if (read_block(fd, addr[i], nybble[i], buf[i]) == -1) {
			goto error;
		}
	}

	return 0;

error:
	perror("ws_read_batch");
	return -1;
}

int
ws_read_safe(int fd, uint16_t addr, size_t nybble, uint8_t *buf)
{
	return ws_read_batch(fd, &addr, &nybble, 1, &buf);
}

