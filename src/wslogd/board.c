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
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <string.h>

#include "board.h"

#define SHM_NAME "/wslog"		/* Shared memory object name */
#define SHM_SIZE (16*1024)		/* Shared memory size */

struct shm_buf
{
	size_t off;			/* Offset to buffer */
	size_t sz;			/* Max number of elements */

	size_t nel;			/* Number of elements */
	size_t idx;			/* next index */
};

struct shm_board
{
	pthread_mutex_t mutex;

	struct shm_buf loop;		/* Loop array */
	struct shm_buf ar;		/* Archive array */
};

static int shmflag = 0;			/* Open flag */
static size_t shmlen = 0;		/* Shared memory size */
static void *shmbufp = MAP_FAILED;	/* Shared memory */

struct shm_board *boardp = NULL;

static int
shm_board_init(size_t len)
{
	int errsv;
	pthread_mutexattr_t attr;

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
	boardp->loop.sz = 100;
	boardp->loop.nel = 0;
	boardp->loop.idx = 0;

	off += boardp->loop.sz * sizeof(struct ws_loop);

	boardp->ar.off = off;
	boardp->ar.sz = 10;
	boardp->ar.nel = 0;
	boardp->ar.idx = 0;

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

int
board_open(int oflag)
{
	int errsv;
	int shmfd;

	shmfd = -1;
	shmlen = SHM_SIZE;
	shmflag = oflag;

	/* Create shared memory */
	shmfd = shm_open(SHM_NAME, O_RDWR|oflag, S_IRUSR|S_IWUSR);
	if (shmfd == -1) {
		return -1;
	}

	if (oflag & O_CREAT) {
		if (ftruncate(shmfd, shmlen) == -1) {
			goto error;
		}
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
		if (shm_board_init(SHM_SIZE) == -1) {
			goto error;
		}
	}

	return 0;

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

	ret = 0;

	if (munmap(shmbufp, shmlen) == -1) {
		ret = -1;
	}
	if (shmflag & O_CREAT) {
		if (shm_board_destroy() == -1) {
			ret = -1;
		}
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
shm_buf_inc(struct shm_buf *buf)
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
shm_buf_index(const struct shm_buf *buf, size_t i)
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
