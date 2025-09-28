#define _POSIX_C_SOURCE 199506L

#include <sys/epoll.h>

#include <unistd.h>

int
main(void)
{
	int ep = epoll_create1(0);
	close(ep);
	return 0;
}
