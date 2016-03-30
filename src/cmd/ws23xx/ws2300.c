#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "libws/nybble.h"

#include "serial.h"
#include "ws2300.h"

#define MAX_RESETS 		100
#define MAX_RETRIES		50
#define MAX_BLOCKS		30
#define READ_TIMEOUT	1000			/* milliseconds */

#define SETACK			0x04
#define UNSETACK		0x0C
#define WRITEACK		0x10

#define min(a,b)		((a) < (b) ? (a) : (b))

static void
encode(uint8_t op, const uint8_t *src, uint8_t *dest, size_t len)
{
	size_t i;

	for (i = 0; i < len; i++) {
		dest[i] = op + (src[i] * 4);
	}
}

static uint8_t
checksum(const uint8_t *buf, size_t len)
{
	size_t i;
	int checksum = 0;

	for (i = 0; i < len; i++) {
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

#if DEBUG >= 1
		printf("ws_reset_06: %d\n", i);
#endif	/* DEBUG */
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
ws_write(int fd, uint16_t addr, size_t nnyb, uint8_t op, const uint8_t *buf)
{
	size_t max_len;
	uint8_t ack_constant;
	uint8_t encoded_data[MAX_BLOCKS];

	switch (op) {
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

	if (max_len < nnyb) {
		errno = EINVAL;
		goto error;
	}

	if (ws_write_addr(fd, addr) == -1) {
		goto error;
	}

	encode(op, buf, encoded_data, nnyb);

	for (size_t i = 0; i < nnyb; i++) {
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
 * Reset the device and write a command, verifying it was written correctly.
 */
int
ws_write_safe(int fd, uint16_t addr, size_t nnyb, uint8_t op, const uint8_t *buf)
{
	for (int i = 0; i < MAX_RETRIES; i++) {
		if (ws_reset_06(fd) == -1) {
			goto error;
		}

		if (ws_write(fd, addr, nnyb, op, buf) == 0) {
			return 0;
		}
	}

	return 0;

error:
	perror("ws_write_safe");
	return -1;
}

int
ws_read(int fd, uint16_t addr, size_t nnyb, uint8_t *buf)
{
	uint8_t answer;

	if (nnyb == 0 || nnyb > MAX_BLOCKS) {
		errno = EINVAL;
		goto error;
	}

	/* Write address to read from */
	if (ws_write_addr(fd, addr) == -1) {
		goto error;
	}

	/* Write the number of bytes we want to read */
	size_t nbyte = (nnyb + 1) / 2;
	uint8_t byte = 0xC2 + nbyte * 4;
	uint8_t ack = 0x30 + nbyte;

	if (write_byte(fd, byte, ack)) {
		goto error;
	}

	/* Read the response */
	for (size_t i = 0; i < nbyte; i++) {
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
read_block(int fd, uint16_t addr, size_t nnyb, uint8_t *buf)
{
	size_t p, k;

#if DEBUG >= 1
	printf("read_block %x (%lu)\n", addr, nnyb);
#endif	/* DEBUG */

	for (p = 0; p < nnyb; p += MAX_BLOCKS) {
		for (k = 0; k < MAX_RETRIES; k++) {
			int chunk = min(MAX_BLOCKS, nnyb - p);

			if (ws_read(fd, addr + p, chunk, buf + (p + 1) / 2) == 0) {
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

static int
read_batch(int fd, const uint16_t *addr, const size_t *nnyb, size_t nel, uint8_t *buf[])
{
	size_t i;

	if (ws_reset_06(fd) == -1) {
		goto error;
	}

	for (i = 0; i < nel; i++) {
		if (read_block(fd, addr[i], nnyb[i], buf[i]) == -1) {
			goto error;
		}
	}

	return 0;

error:
	perror("ws_read_batch");
	return -1;
}

static int
cmp_addr(const uint16_t *a, const uint16_t *b)
{
	return (*a) - (*b);
}

static size_t
nybsz(const size_t *a, size_t nel)
{
	size_t res = 0;

	for (size_t i = 0; i < nel; i++) {
		res += (a[i] + 1) / 2;
	}

	return res;
}

int
ws_read_batch(int fd, const uint16_t *addr, const size_t *nnyb, size_t nel, uint8_t *buf[])
{
	uint16_t io_addr[nel];				/* address */
	size_t io_nnyb[nel];				/* number of nybbles at address */
	uint8_t *io_buf[nel];				/* data */

	/* Sort input addresses */
	memcpy(io_addr, addr, nel * sizeof(*addr));
	qsort(io_addr, nel, sizeof(*io_addr), cmp_addr);

	/* Optimize I/O per area */
	size_t nbyte = 0;
	size_t opt_nel = 0;
	size_t len = nybsz(nnyb, nel);		/* max number of bytes */

	uint8_t data[len];					/* I/O buffer */
	size_t off[nel];					/* nybble offset in data buffer */

	for (size_t i = 0; i < nel; i++) {
		for (size_t j = 0; j < nel; j++) {
			if (io_addr[i] == addr[j]) {
				int same_area = 0;

				if (opt_nel > 0) {
					/**
					 * Using the same area saves 6 bytes read, and 5 bytes written,
					 * so we may overlap here to save I/O.
					 */
					if (io_addr[opt_nel-1] + io_nnyb[opt_nel-1] + 11 >= addr[j]) {
						same_area = 1;
					}
				}

				if (same_area) {
					io_nnyb[opt_nel-1] = addr[j] + nnyb[j] - io_addr[opt_nel-1];
				} else {
					if (opt_nel > 0) {
						nbyte += (io_nnyb[opt_nel-1] + 1) / 2;
					}

					io_addr[opt_nel] = addr[j];
					io_nnyb[opt_nel] = nnyb[j];
					io_buf[opt_nel] = data + nbyte;

					opt_nel++;
				}

				off[j] = 2 * nbyte + (addr[j] - io_addr[opt_nel-1]);
			}
		}
	}

	/* Read */
	if (read_batch(fd, io_addr, io_nnyb, opt_nel, io_buf) == -1) {
		return -1;
	}

	/* Re-order data */
	for (size_t i = 0; i < nel; i++) {
		nybcpy(buf[i], data, nnyb[i], off[i]);
	}

	return 0;
}

int
ws_read_safe(int fd, uint16_t addr, size_t nnybble, uint8_t *buf)
{
	return read_batch(fd, &addr, &nnybble, 1, &buf);
}
