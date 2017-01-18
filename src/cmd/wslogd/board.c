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
#define SHM_SIZE (16*1024)			/* shared memory size */

static size_t shmlen = 0;			/* shared memory size */
static void *shmbufp = MAP_FAILED;	/* shared memory */

struct ws_board *boardp = NULL;

static int
board_init(struct ws_board *p, size_t len)
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
	boardp->loop_sz = 100;
	boardp->loop_idx = boardp->loop_sz;
	boardp->loop = (void *) p + sizeof(*p);

	boardp->ar_sz = 10;
	boardp->ar_idx = boardp->ar_sz;
	boardp->ar = (void *) p + sizeof(*p) + boardp->loop_sz * sizeof(p->loop);

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
	shmlen = SHM_SIZE;

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

	/* Initialize shared_memory content */
	if (board_init(boardp, SHM_SIZE) == -1) {
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
board_get_ar(struct ws_archive *p)
{
	size_t idx;
	int ret;

	ret = 0;

	if (pthread_mutex_lock(&boardp->mutex) == -1) {
		return -1;
	}

	idx = boardp->loop_idx;

	if (idx == boardp->loop_sz) {
		ret = -1;
	} else {
		memcpy(p, &boardp->loop[idx-1], sizeof(*p));
	}

	if (pthread_mutex_unlock(&boardp->mutex) == -1) {
		return -1;
	}

	return ret;
}

int
board_push(const struct ws_loop *p)
{
	size_t idx;

	if (pthread_mutex_lock(&boardp->mutex) == -1) {
		return -1;
	}

	idx = boardp->loop_idx;

	if (idx == boardp->loop_sz) {
		idx = 0;
	}

	memcpy(&boardp->loop[idx], p, sizeof(*p));

	boardp->loop_idx = (idx + 1) % boardp->loop_sz;

	if (pthread_mutex_unlock(&boardp->mutex) == -1) {
		return -1;
	}

	return 0;
}

int
ws_isset(const struct ws_loop *p, int mask)
{
	return p->wl_mask & mask;
}

int
ws_isset_ar(const struct ws_archive *p, int mask)
{
	return p->data.wl_mask & mask;
}
