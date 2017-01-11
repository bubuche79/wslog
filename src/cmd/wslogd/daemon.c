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
#include <signal.h>
#include <errno.h>

#include "libws/err.h"

#include "wslogd.h"
#include "daemon.h"

extern char **environ;

static int
close_fds(void)
{
	int fd;
	int fdmax;

	fdmax = sysconf(_SC_OPEN_MAX);

	for (fd = 0; fd < fdmax; fd++) {
		int ret;

		ret = close(fd);

		if (ret == -1 && errno != EBADF) {
			return -1;
		}
	}

	return 0;
}

static int
sanitize_stdfd(void)
{
	int fd;
	int nullfds[2] = { -1, -1 };

	for (fd = 0; fd < 3; ++fd) {
		int i, oflag;
		struct stat sbuf;

		if (fstat(fd, &sbuf) == -1) {
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
reset_signals(void)
{
	int si;
	struct sigaction sa;

	/* Reset signal handlers */
	memset(&sa, 0, sizeof(sa));

	sa.sa_flags = 0;
	sa.sa_handler = SIG_DFL;
	(void) sigemptyset(&sa.sa_mask);

	for (si = 1; si < _NSIG; si++) {
		if (sigaction(si, &sa, NULL) == -1) {
			if (errno == EINVAL) {
				/* Unmodifiable signal */
			} else {
				return -1;
			}
		}
	}

	/* Reset signal mask */
//	sigset_t set;
//	(void) sigfillset(&set);
//
//	return sigprocmask(SIG_BLOCK, &set, NULL);

	return 0;
}

static int
clear_env(void)
{
	environ = NULL;

	return 0;
}

static int
wait_startup(int fd)
{
	int status;
	char buf[128];
	ssize_t sz;

	read(fd, &status, sizeof(status));
	sz = read(fd, buf, sizeof(buf));

	if (status != 0) {
		buf[sz] = 0;
		die(status, "%s\n", buf);
	}

	return 0;
}

static int
write_pid_file(void)
{
	int fd = open(confp->pid_file, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	if (fd == -1) {
		return -1;
	}

	dprintf(fd, "%d", getpid());
	close(fd);

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
daemon_die(int status, int fd, const char *fmt, /* args */ ...)
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
daemon_create(void)
{
	pid_t pid;
	int pipefd[2];

	/* 1. Close open file descriptors */
	if (close_fds() == -1) {
		return -1;
	}

	/* 2 & 3. Reset signal handlers */
	if (reset_signals() == -1) {
		return -1;
	}

	/* 4. Clear environment */
	if (clear_env() == -1) {
		return -1;
	}

	/* 5. Create background process */
	if (pipe(pipefd) == -1) {
		return -1;
	}

	pid = fork();
	if (pid == -1) {
		die(1, "fork: %s\n", strerror(errno));
	} else if (pid > 0) {
		/* Wait startup to complete */
		(void) close(pipefd[1]);
		wait_startup(pipefd[0]);

		/* 15. Exit from original process */
		exit(0);
	} else {
		if (close(pipefd[0]) == -1) {
			die(1, "close(pipe): %s\n", strerror(errno));
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
		daemon_die(1, pipefd[1], "fork: %s\n", strerror(errno));
	} else if (pid > 0) {
		exit(0);
	}

	/* 9. Connect /dev/null to standard descriptors */
	sanitize_stdfd();

	/* 10. Clear umask */
	umask(0);

	/* 11. Change current working directory */
	if (chdir("/") == -1) {
		daemon_die(1, pipefd[1], "chdir: %s", strerror(errno));
	}

	/* 12. Write daemon PID file */
	if (write_pid_file() == -1) {
		daemon_die(1, pipefd[1], "write_pid(%s): %s", confp->pid_file, strerror(errno));
	}

	/* 13. Drop privileges */
	if (confp->uid != (uid_t)-1) {
		if (seteuid(confp->uid) == -1) {
			daemon_die(1, pipefd[1], "seteuid: %s", strerror(errno));
		}
	}
	if (confp->gid != (gid_t)-1) {
		if (setegid(confp->gid) == -1) {
			daemon_die(1, pipefd[1], "setegid: %s", strerror(errno));
		}
	}

	notify(0, pipefd[1], "started");

	(void) close(pipefd[1]);

	return 0;
}
