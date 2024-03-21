#include <sys/epoll.h>

#include <fcntl.h>
#include <unistd.h>

int
main()
{
	int ep = epoll_create1(EPOLL_CLOEXEC);
	int r = fcntl(ep, F_GETFL);
	(void)r;
	close(ep);
}
