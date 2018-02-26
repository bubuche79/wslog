/*
 * Shared board.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "board.h"

#define SHM_NAME "/wslog"		/* Shared memory object name */
#define SHM_SIZE (16*1024)		/* Shared memory size */

struct shm_circ
{
	size_t off;			/* Offset to buffer */
	size_t sz;			/* Max number of elements */

	size_t nel;			/* Number of elements */
	size_t idx;			/* Next index */
};

struct shm_board
{
	size_t len;			/* Buffer size */
	pthread_mutex_t mutex;		/* Process-shared */

	struct shm_circ ar;		/* Archive array */
	struct shm_circ loop;		/* Loop array */
};

static int shmflag = 0;			/* Open flag */
static void *shmbufp = MAP_FAILED;	/* Shared memory */

struct shm_board *boardp = NULL;

static size_t
shm_board_size(size_t nloops, size_t nar)
{
	return sizeof(*boardp)
			+ nloops * sizeof(struct ws_loop)
			+ nar * sizeof(struct ws_archive);
}

static int
shm_board_init(size_t len, size_t nloops, size_t nar)
{
	int errsv;
	pthread_mutexattr_t attr;

	boardp->len = len;

	/* Shared lock */
	if (pthread_mutexattr_init(&attr) == -1) {
		goto error;
	}
	if (pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED) == -1) {
		goto error;
	}
	if (pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST) == -1) {
		goto error;
	}

	if (pthread_mutex_init(&boardp->mutex, &attr) == -1) {
		goto error;
	}

	(void) pthread_mutexattr_destroy(&attr);

	/* Shared arrays */
	size_t off = sizeof(*boardp);

	boardp->loop.off = off;
	boardp->loop.sz = nloops;
	boardp->loop.nel = 0;
	boardp->loop.idx = 0;

	off += boardp->loop.sz * sizeof(struct ws_loop);

	boardp->ar.off = off;
	boardp->ar.sz = nar;
	boardp->ar.nel = 0;
	boardp->ar.idx = 0;

	off += boardp->ar.sz * sizeof(struct ws_archive);

	if (len < off) {
		errno = ENOMEM;
		goto error;
	}

	return 0;

error:
	errsv = errno;
	(void) pthread_mutexattr_destroy(&attr);

	errno = errsv;
	return -1;
}

static int
shm_board_destroy()
{
	int ret;

	ret = pthread_mutex_destroy(&boardp->mutex);

	return ret;
}

ssize_t
board_open(int oflag, /* args */ ...)
{
	int errsv;
	int shmfd, shmlen;
	size_t nloops, nar;
	va_list ap;

	shmfd = -1;
	shmflag = oflag;

	/* Create shared memory */
	shmfd = shm_open(SHM_NAME, O_RDWR|oflag, S_IRUSR|S_IWUSR);
	if (shmfd == -1) {
		return -1;
	}

	if (oflag & O_CREAT) {
		va_start(ap, oflag);
		nloops = va_arg(ap, size_t);
		nar = va_arg(ap, size_t);
		va_end(ap);

		shmlen = shm_board_size(nloops, nar);

		if (ftruncate(shmfd, shmlen) == -1) {
			goto error;
		}
	} else {
		struct stat sbuf;

		if (fstat(shmfd, &sbuf) == -1) {
			goto error;
		}

		shmlen = sbuf.st_size;
	}

	shmbufp = mmap(NULL, shmlen, PROT_READ|PROT_WRITE, MAP_SHARED, shmfd, 0);
	if (shmbufp == MAP_FAILED) {
		goto error;
	}

	(void) close(shmfd);
	shmfd = -1;

	boardp = shmbufp;

	/* Initialize shared_memory content */
	if (oflag & O_CREAT) {
		if (shm_board_init(shmlen, nloops, nar) == -1) {
			goto error;
		}
	}

	return shmlen;

error:
	errsv = errno;
	if (shmfd != -1) {
		(void) close(shmfd);
	}
	if (shmbufp != MAP_FAILED) {
		(void) board_unlink();
	}

	errno = errsv;
	return -1;
}

int
board_unlink()
{
	int ret;
	size_t shmlen;

	ret = 0;
	shmlen = boardp->len;

	if (shmflag & O_CREAT) {
		if (shm_board_destroy() == -1) {
			ret = -1;
		}
	}

	/* Unlink shared memory */
	if (munmap(shmbufp, shmlen) == -1) {
		ret = -1;
	}
	if (shmflag & O_CREAT) {
		if (shm_unlink(SHM_NAME) == -1) {
			ret = -1;
		}
	}

	return ret;
}

int
board_lock(void)
{
	int ret;

	ret = pthread_mutex_lock(&boardp->mutex);
	if (ret == -1) {
		/* Owner died with lock */
		if (errno == EOWNERDEAD) {
			if (shmflag & O_CREAT) {
				/* Unrecoverable error */
			} else {
				ret = pthread_mutex_consistent(&boardp->mutex);
			}
		}
	}

	return ret;
}

int
board_unlock(void)
{
	int ret;

	ret = pthread_mutex_unlock(&boardp->mutex);

	return ret;
}

/**
 * Increment the circular buffer pointed to by {@code buf}.
 */
static void
shm_buf_inc(struct shm_circ *buf)
{
	if (buf->nel == 0) {
		buf->nel = 1;
	} else {
		if (buf->nel < buf->sz) {
			buf->nel++;
		}
		if (buf->idx + 1 < buf->sz) {
			buf->idx++;
		} else {
			buf->idx = 0;
		}
	}
}

/**
 * Computes the index of the i-the element of the circular buffer pointed to
 * by {@code buf}.
 */
static int
shm_buf_index(const struct shm_circ *buf, size_t i)
{
	if (i <= buf->idx) {
		i = buf->idx - i;
	} else {
		i = buf->nel - (i - buf->idx);
	}

	return i;
}

void
board_push(const struct ws_loop *p)
{
	shm_buf_inc(&boardp->loop);
	memcpy(board_peek(0), p, sizeof(*p));
}

void
board_push_ar(const struct ws_archive *p)
{
	shm_buf_inc(&boardp->ar);
	memcpy(board_peek_ar(0), p, sizeof(*p));
}

struct ws_loop *
board_peek(size_t i)
{
	struct ws_loop *p;

	if (boardp->loop.nel <= i) {
		p = NULL;
	} else {
		i = shm_buf_index(&boardp->loop, i);
		p = &((struct ws_loop *) ((char *) boardp + boardp->loop.off))[i];
	}

	return p;
}

struct ws_archive *
board_peek_ar(size_t i)
{
	struct ws_archive *p;

	if (boardp->ar.nel <= i) {
		p = NULL;
	} else {
		i = shm_buf_index(&boardp->ar, i);
		p = &((struct ws_archive *) ((char *) boardp + boardp->ar.off))[i];
	}

	return p;
}
