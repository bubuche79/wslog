/*
 * Daemon related functions.
 */

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "daemon.h"

static int
sanitize_stdfd(void)
{
	int fd;
	int nullfds[2] = { -1, -1 };

	for (fd = 0; fd < 3; ++fd) {
		int i, oflag;
		struct stat sbuf;

		if (fstat(fd, &sbuf) == 0) {
			continue;
		}

		i = (fd == 0) ? 0 : 1;
		oflag = (fd == 0) ? O_RDONLY : O_WRONLY;

		if (nullfds[i] == -1) {
			if ((nullfds[i] = open("/dev/null", oflag)) == -1) {
				return -1;
			}
		}

		if (dup2(nullfds[i], fd) == -1) {
			return -1;
		}
	}

	return 0;
}

static int
close_all_fd(void)
{
	return 0;
}

static int
wait_startup(int fd)
{
	int status;
	char buf[128];

	read(fd, &status, sizeof(status));
	read(fd, buf, sizeof(buf));

	fprintf(stderr, "%s\n", buf);
	if (status != 0) {
		exit(status);
	}

	return 0;
}

static int
vnotify(int status, int fd, const char *fmt, va_list ap)
{
	write(fd, &status, sizeof(status));
	vdprintf(fd, fmt, ap);

	return 0;
}

#if __GNUC__ >= 4
__attribute__ ((format (printf, 3, 4)))
#endif
static int
notify(int status, int fd, const char *fmt, /* args */ ...)
{
	int ret;
	va_list ap;

	va_start(ap, fmt);
	ret = vnotify(status, fd, fmt, ap);
	va_end(ap);

	return ret;
}

#if __GNUC__ >= 4
__attribute__ ((format (printf, 3, 4)))
#endif
static void
die(int status, int fd, const char *fmt, /* args */ ...)
{
	va_list ap;

	va_start(ap, fmt);
	vnotify(status, fd, fmt, ap);
	va_end(ap);

	exit(status);
}

/**
 * Detach, and create daemon.
 *
 * Implementation refers to daemon(7).
 */
int
daemon(void)
{
	pid_t pid;
	int pipefd[2];

	/* 1. Close open file descriptors */
	close_all_fd();

	/* 4. Clear environment */
//	if (clearenv() == -1) {
//		(void) fprintf(stderr, "clearenv: %s\n", strerror(errno));
//		exit(1);
//	}

	/* 5. Create background process */
	if (pipe(pipefd) == -1) {
		return -1;
	}

	pid = fork();
	if (pid == -1) {
		(void) fprintf(stderr, "fork: %s\n", strerror(errno));
		exit(1);
	} else if (pid > 0) {
		/* Wait startup to complete */
		close(pipefd[1]);
		wait_startup(pipefd[0]);

		/* 15. Exit from original process */
		exit(0);
	} else {
		if (close(pipefd[0]) == -1) {
			(void) fprintf(stderr, "close(pipe): %s\n", strerror(errno));
			exit(1);
		}
	}

	/* 6. Create independent session */
	if (setsid() == -1) {
		(void) fprintf(stderr, "setsid: %s\n", strerror(errno));
		return -1;
	}

	/* 7, 8. Ensure daemon cannot re-acquire terminal */
	pid = fork();
	if (pid == -1) {
		die(1, pipefd[1], "fork: %s\n", strerror(errno));
	} else if (pid > 0) {
		exit(0);
	}

	/* 9. Connect /dev/null to standard descriptors */
	sanitize_stdfd();

	/* 10. Clear umask */
	umask(0);

	/* 11. Change current working directory */
	if (chdir("/") == -1) {
		die(1, pipefd[1], "chdir: %s", strerror(errno));
	}

	notify(0, pipefd[1], "started");

	close(pipefd[1]);

	return 0;
}
