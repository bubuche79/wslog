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
	pthread_rwlockattr_t attr;

	/* Shared lock */
	if (pthread_rwlockattr_init(&attr) == -1) {
		goto error;
	}
	if (pthread_rwlockattr_setpshared(&attr, PTHREAD_PROCESS_SHARED) == -1) {
		goto error;
	}

	if (pthread_rwlock_init(&p->rwlock, &attr) == -1) {
		goto error;
	}

	(void) pthread_rwlockattr_destroy(&attr);

	/* Pointers */
	boardp->bufsz = LOG_SZ;
	boardp->idx = 0;

	return 0;

error:
	errsv = errno;
	(void) pthread_rwlockattr_destroy(&attr);

	errno = errsv;
	return -1;
}

//static int
//board_destroy(struct ws_board *p)
//{
//	return pthread_rwlock_destroy(p->rwlock);
//}
//
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

struct ws_ws23xx*
board_last()
{
	return &boardp->buf[boardp->idx].ws23xx;
}
