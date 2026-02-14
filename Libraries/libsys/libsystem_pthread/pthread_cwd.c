#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

extern int __pthread_chdir(const char *path);
int
pthread_chdir_np(const char *path)
{
	return __pthread_chdir(path);
}

extern int __pthread_fchdir(int fd);
int
pthread_fchdir_np(int fd)
{
	return __pthread_fchdir(fd);
}
