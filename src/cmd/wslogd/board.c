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

static int shmflag = 0;				/* open flag */
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
	if (pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST) == -1) {
		goto error;
	}

	if (pthread_mutex_init(&p->mutex, &attr) == -1) {
		goto error;
	}

	(void) pthread_mutexattr_destroy(&attr);

	/* Pointers */
	boardp->loop_sz = 100;
	boardp->loop_nel = 0;
	boardp->loop_idx = 0;

	boardp->ar_sz = 10;
	boardp->ar_nel = 0;
	boardp->ar_idx = 0;

	return 0;

error:
	errsv = errno;
	(void) pthread_mutexattr_destroy(&attr);

	errno = errsv;
	return -1;
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
		if (board_init(boardp, SHM_SIZE) == -1) {
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
		if (oflag & O_CREAT) {
			(void) shm_unlink(SHM_NAME);
		}
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
	if (shmflag & O_CREAT) {
		if (shm_unlink(SHM_NAME) == -1) {
			ret = -1;
		}
	}

	return ret;
}

int
board_push(const struct ws_loop *p)
{
	if (pthread_mutex_lock(&boardp->mutex) == -1) {
		return -1;
	}

	if (boardp->loop_nel == 0) {
		boardp->loop_nel = 1;
	} else {
		if (boardp->loop_nel < boardp->loop_sz) {
			boardp->loop_nel++;
		}

		if (boardp->loop_idx + 1 < boardp->loop_sz) {
			boardp->loop_idx++;
		} else {
			boardp->loop_idx = 0;
		}
	}

	memcpy(board_loop_p(0), p, sizeof(*p));

	if (pthread_mutex_unlock(&boardp->mutex) == -1) {
		return -1;
	}

	return 0;
}
