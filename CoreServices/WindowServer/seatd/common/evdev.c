#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#if defined(__linux__)
#include <linux/input.h>
#include <linux/major.h>
#include <sys/sysmacros.h>
#elif defined(__FreeBSD__)
#include <dev/evdev/input.h>
#endif

#include "evdev.h"

#define STRLEN(s) ((sizeof(s) / sizeof(s[0])) - 1)

#if defined(__linux__) || defined(__FreeBSD__)
int path_is_evdev(const char *path) {
	static const char prefix[] = "/dev/input/event";
	static const size_t prefixlen = STRLEN(prefix);
	return strncmp(prefix, path, prefixlen) == 0;
}

int evdev_revoke(int fd) {
	return ioctl(fd, EVIOCREVOKE, NULL);
}
#elif defined(__NetBSD__)
int path_is_evdev(const char *path) {
	(void)path;
	return 0;
}
int evdev_revoke(int fd) {
	(void)fd;
	return 0;
}
#else
#error Unsupported platform
#endif
