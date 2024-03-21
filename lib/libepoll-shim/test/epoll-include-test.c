#define _POSIX_C_SOURCE 200809L

#include <sys/epoll.h>

#include <unistd.h>

int
main(void)
{
	int ep = epoll_create1(EPOLL_CLOEXEC);
	close(ep);
}
