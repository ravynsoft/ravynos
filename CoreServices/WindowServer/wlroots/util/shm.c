#define _POSIX_C_SOURCE 200112L
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <wlr/config.h>
#include "util/shm.h"

#define RANDNAME_PATTERN "/wlroots-XXXXXX"

static void randname(char *buf) {
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	long r = ts.tv_nsec;
	for (int i = 0; i < 6; ++i) {
		buf[i] = 'A'+(r&15)+(r&16)*2;
		r >>= 5;
	}
}

static int excl_shm_open(char *name) {
	int retries = 100;
	do {
		randname(name + strlen(RANDNAME_PATTERN) - 6);

		--retries;
		// CLOEXEC is guaranteed to be set by shm_open
		int fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
		if (fd >= 0) {
			return fd;
		}
	} while (retries > 0 && errno == EEXIST);

	return -1;
}

int allocate_shm_file(size_t size) {
	char name[] = RANDNAME_PATTERN;
	int fd = excl_shm_open(name);
	if (fd < 0) {
		return -1;
	}
	shm_unlink(name);

	int ret;
	do {
		ret = ftruncate(fd, size);
	} while (ret < 0 && errno == EINTR);
	if (ret < 0) {
		close(fd);
		return -1;
	}

	return fd;
}

bool allocate_shm_file_pair(size_t size, int *rw_fd_ptr, int *ro_fd_ptr) {
	char name[] = RANDNAME_PATTERN;
	int rw_fd = excl_shm_open(name);
	if (rw_fd < 0) {
		return false;
	}

	// CLOEXEC is guaranteed to be set by shm_open
	int ro_fd = shm_open(name, O_RDONLY, 0);
	if (ro_fd < 0) {
		shm_unlink(name);
		close(rw_fd);
		return false;
	}

	shm_unlink(name);

	int ret;
	do {
		ret = ftruncate(rw_fd, size);
	} while (ret < 0 && errno == EINTR);
	if (ret < 0) {
		close(rw_fd);
		close(ro_fd);
		return false;
	}

	*rw_fd_ptr = rw_fd;
	*ro_fd_ptr = ro_fd;
	return true;
}
