/*
 * Shared board.
 */

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "board.h"

#define SHM_NAME "/wslog"			/* shared memory object name */

static size_t shmlen = 0;			/* shared memory size */
static void *shmbufp = MAP_FAILED;	/* shared memory */

struct ws_board *boardp = NULL;

static int
board_init(struct ws_board *p)
{
	int errsv;
	pthread_rwlockattr_t attr;

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
	oflag = O_CREAT | (rdonly ? O_RDONLY : O_RDWR);
	mflag = PROT_READ | (rdonly ? 0 : PROT_WRITE);

	/* Create shared memory */
	shmfd = shm_open(SHM_NAME, oflag, S_IRUSR|S_IWUSR);
	if (shmfd == -1) {
		return -1;
	}

	if (oflag & O_WRONLY) {
		if (ftruncate(shmfd, shmlen) == -1) {
			goto error;
		}
	}

	shmbufp = mmap(NULL, shmlen, mflag, MAP_SHARED, shmfd, 0);
	if (shmbufp == MAP_FAILED) {
		goto error;
	}

	(void) close(shmfd);

	boardp = shmbufp;

	/* Initialize shared_memory content */
	if (board_init(boardp) == -1) {
		goto error;
	}

	return 0;

error:
	errsv = errno;
	if (shmbufp != MAP_FAILED) {
		(void) munmap(shmbufp, shmlen);
	}
	if (shmfd != -1) {
		(void) close(shmfd);
		(void) shm_unlink(SHM_NAME);
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
