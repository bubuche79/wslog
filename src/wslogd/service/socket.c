#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>
#include <syslog.h>

#include "socket.h"

#define BACKLOG 10

static int af_unix = -1;
static int af_inet = -1;

static int
init_un(const char *pathname)
{
	int af_un;
	struct sockaddr_un addr_un;

	memset(&addr_un, 0, sizeof(addr_un));

	addr_un.sun_family = AF_UNIX;
	strncpy(addr_un.sun_path, pathname, sizeof(addr_un.sun_path) - 1);

	unlink(pathname);

	/* Create local socket */
	if ((af_un = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		syslog(LOG_ERR, "socket unix: %m");
		goto error;
	}

	/* Bind socket to socket name */
	if (bind(af_un, &addr_un, sizeof(addr_un)) == -1) {
		syslog(LOG_ERR, "bind %s: %m", pathname);
		goto error;
	}

	/* Prepare for accepting connections */
	if (listen(af_un, BACKLOG) == -1) {
		syslog(LOG_ERR, "listen: %m");
		goto error;
	}

	syslog(LOG_INFO, "Socket %s ready", pathname);

	return af_un;

error:
	if (af_un != -1) {
		(void) close(af_un);
	}

	return -1;
}

static int
init_in(in_addr_t addr, in_port_t port)
{
	int af_in;
	struct sockaddr_in addr_in;
	char inet_name[INET6_ADDRSTRLEN];

	memset(&addr_in, 0, sizeof(addr_in));

	addr_in.sin_family = AF_INET;
	addr_in.sin_port = port;
	addr_in.sin_addr.s_addr = addr;

	inet_ntop(AF_INET, &addr_in.sin_addr, inet_name, sizeof(inet_name));

	/* Create local socket */
	if ((af_in = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		syslog(LOG_ERR, "socket inet: %m");
		goto error;
	}

	/* Bind socket to socket name */
	if (bind(af_in, &addr_in, sizeof(addr_in)) == -1) {
		syslog(LOG_ERR, "bind %s: %m", inet_name);
		goto error;
	}

	/* Prepare for accepting connections */
	if (listen(af_in, BACKLOG) == -1) {
		syslog(LOG_ERR, "listen: %m");
		goto error;
	}

	syslog(LOG_INFO, "Socket %s:%d ready", inet_name, ntohs(port));

	return af_in;

error:
	if (af_in != -1) {
		(void) close(af_in);
	}

	return -1;
}

int
sock_init(void)
{
	af_unix = init_un("/tmp/wslogd.s");
	af_inet = init_in(INADDR_ANY, htons(8080));

	return 0;

error:
	return -1;
}

static int
sock_conn(int fd)
{
	char buf[32];

	int sz = read(fd, buf, 32);
	buf[sz] = 0;

	printf("received: %s\n", buf);

	close(fd);

	return 0;
}

int
sock_main(void)
{
	size_t nfds = 2;
	struct pollfd fds[nfds];

	/* Set descriptors to wait on */
	fds[0].fd = af_unix;
	fds[0].events = POLLIN;

	fds[1].fd = af_inet;
	fds[1].events = POLLIN;

	for(;;) {
		ssize_t i, sz;

		/* Wait for event */
		if ((sz = poll(fds, nfds, -1)) == -1) {
			syslog(LOG_ERR, "poll: %m");
			goto error;
		}

		/* Process events */
		for (i = 0; i < nfds; i++) {
			struct pollfd *pfd = &fds[i];

			if (pfd->revents & POLLIN) {
				int conn;

				if ((conn = accept(pfd->fd, NULL, 0)) == -1) {
					syslog(LOG_ERR, "accept: %m");
					goto error;
				}

				sock_conn(conn);
			} else if (pfd->revents & POLLERR) {
				syslog(LOG_ERR, "poll error event");
				goto error;
			}
		}
	}

	return 0;

error:
	return -1;
}

int
sock_destroy(void)
{
	syslog(LOG_INFO, "Socket closed");

	return 0;
}
