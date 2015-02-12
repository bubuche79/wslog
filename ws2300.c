#include <stdio.h>
#include <errno.h>

#include "serial.h"
#include "ws2300.h"

#define MAX_RESETS 		100
#define MAX_RETRIES		50
#define MAX_BLOCK		80
#define READ_TIMEOUT		1000		/* milliseconds */

#define WRITENIB		0x42
#define SETBIT			0x12
#define UNSETBIT		0x32
#define WRITEACK		0x10
#define SETACK			0x04
#define UNSETACK		0x0C

static struct measure mem_map[] = {
	{ 0x346, "it", TEMP, "in temp" }
};

static void
encode_data(uint8_t encode_constant, const uint8_t *in, uint8_t *out, size_t len)
{
	for (int i = 0; i < len; i++) {
		out[i] = encode_constant + (in[i] * 4);
	}
}

static int
write_byte(int fd, uint8_t byte, uint8_t ack)
{
	uint8_t answer;

	if (ws_write_byte(fd, byte) == -1) {
		goto error;
	}
	if (ws_read_byte(fd, &answer, READ_TIMEOUT) == -1) {
		goto error;
	}

	/* Check answer against ack */
	if (ack != answer) {
		fprintf(stderr, "Expected %d, got %d\n", ack, answer);
		return -1;
	}

	return 0;

error:
	perror("write_byte");
	return -1;
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

int
ws_write_address(int fd, uint16_t address)
{
	uint8_t byte, ack;

	for (int i = 0; i < 4; i++) {
		byte = 0x82 + (address >> (4 * (3 - i)) & 0xF) * 4;
		ack = i * 16 + (byte - 0x82) / 4;

		if (write_byte(fd, byte, ack) == -1) {
			goto error;
		}
	}

	return 0;

error:
	perror("ws_write_address");
	return -1;
}

/**
 * Write data, checking the reply.
 *
 * @return 0 on success, -1 on error
 */
int
ws_write_data(int fd, uint16_t address, uint8_t encode_constant, const uint8_t *buf, size_t len)
{
	size_t max_len;
	uint8_t ack_constant;
	uint8_t encoded_data[MAX_BLOCK];

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

	if (ws_write_address(fd, address) == -1) {
		goto error;
	}

	encode_data(encode_constant, buf, encoded_data, len);

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

int
ws_write_safe(int fd, uint16_t address, uint8_t encode_constant, const uint8_t *buf, size_t len)
{
	for (int i = 0; i < MAX_RETRIES; i++) {
		if (ws_reset_06(fd) == -1) {
			goto error;
		}

		if (ws_write_data(fd, address, encode_constant, buf, len) == 0) {
			return 0;
		}
	}

	return 0;

error:
	perror("ws_write_safe");
	return -1;
}

int
ws_read_data(int fd, uint16_t address, uint8_t *buf, size_t len)
{
	if (len < 1 || len > MAX_BLOCK) {
		errno = EINVAL;
		goto error;
	}

	if (ws_write_address(fd, address) == -1) {
		goto error;
	}

	/* Write the number of bytes we want to read */
	size_t nbytes = (len + 1) / 2;
	uint8_t byte = 0xC2 + nbytes * 4;
	uint8_t ack = 0x30 + nbytes;

	if (write_byte(fd, byte, ack)) {
		goto error;
	}

	/* Read the response */
	for (int i = 0; i < nbytes; i++) {
		if (ws_read_byte(fd, buf + i, READ_TIMEOUT) == -1) {
			goto error;
		}
	}

	/* Read and verify checksum */
	uint8_t answer;

	if (ws_read_byte(fd, &answer, READ_TIMEOUT) == -1) {
		goto error;
	}
	if (answer != checksum(buf, nbytes)) {
		goto error;
	}

	return 0;

error:
	perror("ws_read_data");
	return -1;
}

