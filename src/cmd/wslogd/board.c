/*
 * Shared board.
 */

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <string.h>

#include "board.h"

#define SHM_NAME "/wslog"			/* shared memory object name */
#define LOG_SZ 10					/* log size */

static size_t shmlen = 0;			/* shared memory size */
static void *shmbufp = MAP_FAILED;	/* shared memory */

struct ws_board *boardp = NULL;

static int
board_init(struct ws_board *p)
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

	if (pthread_mutex_init(&p->mutex, &attr) == -1) {
		goto error;
	}

	(void) pthread_mutexattr_destroy(&attr);

	/* Pointers */
	boardp->bufsz = LOG_SZ;
	boardp->idx = 0;

	return 0;

error:
	errsv = errno;
	(void) pthread_mutexattr_destroy(&attr);

	errno = errsv;
	return -1;
}

int
board_open(int rdonly)
{
	int errsv;
	int shmfd;
	int oflag, mflag;

	shmfd = -1;
	shmlen = sizeof(*boardp) + LOG_SZ * sizeof(*boardp->buf);

	oflag = rdonly ? O_RDONLY : O_RDWR|O_CREAT;
	mflag = PROT_READ | (rdonly ? 0 : PROT_WRITE);

	/* Create shared memory */
	shmfd = shm_open(SHM_NAME, oflag, S_IRUSR|S_IWUSR);
	if (shmfd == -1) {
		return -1;
	}

	if (!rdonly) {
		if (ftruncate(shmfd, shmlen) == -1) {
			goto error;
		}
	}

	shmbufp = mmap(NULL, shmlen, mflag, MAP_SHARED, shmfd, 0);
	if (shmbufp == MAP_FAILED) {
		goto error;
	}

	(void) close(shmfd);
	shmfd = -1;

	boardp = shmbufp;
	boardp->buf = shmbufp + sizeof(*boardp);

	/* Initialize shared_memory content */
	if (board_init(boardp) == -1) {
		goto error;
	}

	return 0;

error:
	errsv = errno;
	if (shmfd != -1) {
		(void) close(shmfd);
	}
	if (shmbufp != MAP_FAILED) {
		(void) shm_unlink(SHM_NAME);
		(void) munmap(shmbufp, shmlen);
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
	if (shm_unlink(SHM_NAME) == -1) {
		ret = -1;
	}

	return ret;

}

int
board_get(struct ws_log *p)
{
	int idx;
	int ret;

	ret = 0;

	if (pthread_mutex_lock(&boardp->mutex) == -1) {
		return -1;
	}

	idx = boardp->idx;

	if (idx == -1) {
		ret = -1;
	} else {
		memcpy(p, &boardp->buf[idx-1], sizeof(*p));
	}

	if (pthread_mutex_unlock(&boardp->mutex) == -1) {
		return -1;
	}

	return ret;
}

int
board_push(const struct ws_log *p)
{
	size_t idx;

	if (pthread_mutex_lock(&boardp->mutex) == -1) {
		return -1;
	}

	idx = boardp->idx;

	if (idx == -1) {
		idx = 0;
	}

	memcpy(&boardp->buf[idx], p, sizeof(*p));

	boardp->idx = (idx + 1) % boardp->bufsz;

	if (pthread_mutex_unlock(&boardp->mutex) == -1) {
		return -1;
	}

	return 0;
}

int
ws_isset(const struct ws_log *p, int mask)
{
	return p->log_mask & mask;
}
