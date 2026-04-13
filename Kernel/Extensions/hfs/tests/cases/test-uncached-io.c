#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/uio.h>

#include "hfs-tests.h"
#include "test-utils.h"
#include "disk-image.h"

TEST(uncached_io)

static disk_image_t *di;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

static volatile int state;
static volatile bool run_thread;

static char *file1, *file2;

void *read_thread(__unused void *arg)
{
	int fd = open(file1, O_RDONLY);

	int x;

	while (run_thread) {
		check_io(pread(fd, &x, 4, 0), 4);

		if (x == state)
			continue;

		pthread_mutex_lock(&mutex);
		state = ~state;
		pthread_mutex_unlock(&mutex);

		pthread_cond_broadcast(&cond);
	}

	close(fd);

	return NULL;
}

int run_uncached_io(__unused test_ctx_t *ctx)
{
	di = disk_image_get();
	asprintf(&file1, "%s/test.data", di->mount_point);
	asprintf(&file2, "%s/test2.data", di->mount_point);

	unlink(file1);
	unlink(file2);

	int fd;
	assert_with_errno((fd = open(file1,
								 O_RDWR | O_CREAT | O_TRUNC, 0666)) >= 0);

	assert_no_err(fcntl(fd, F_NOCACHE, 1));

	assert_no_err(ftruncate(fd, 4096));

	int fd2;
	assert_with_errno((fd2 = open(file2,
								  O_RDWR | O_CREAT | O_TRUNC, 0666)) >= 0);

	// Force the test file to be 1 block, and then 4 blocks
	assert_no_err(ftruncate(fd2, 4096));
	assert_no_err(ftruncate(fd, 4096 + 16384));
	assert_no_err(ftruncate(fd2, 8192));

	char *buffer = malloc(65536);

	// Set buffer to be 12288 bytes off 16 KB alignment
	buffer = (char *)(((uintptr_t)(buffer + 4096) & ~16383) + 12288);

	check_io(pwrite(fd, buffer, 32768, 0), 32768);

	// Now do the (slightly simpler case: a single transaction)
	assert_no_err(ftruncate(fd, 0));
	check_io(pwrite(fd, buffer, 32768, 0), 32768);

	// And one more time with a single page
	assert_no_err(ftruncate(fd, 0));
	check_io(pwrite(fd, buffer, 16384, 0), 16384);

	// Now just two transactions
	assert_no_err(ftruncate(fd, 4096));
	check_io(pwrite(fd, buffer, 32768, 0), 32768);

	// And another variant of two transactions
	assert_no_err(ftruncate(fd, 0));
	assert_no_err(ftruncate(fd, 4096 + 16384));
	assert_no_err(ftruncate(fd2, 0));
	assert_no_err(ftruncate(fd2, 4096));
	check_io(pwrite(fd, buffer, 32768, 0), 32768);

	assert_no_err(close(fd));
	assert_no_err(unlink(file1));
	assert_no_err(unlink(file2));

	assert_with_errno((fd = open(file1, 
								 O_RDWR | O_CREAT | O_TRUNC, 0666)) >= 0);

	// The cluster currently only does uncached I/O if 16 KB or higher
	int size = 16384;

	assert_no_err(ftruncate(fd, size));

	assert_no_err(fcntl(fd, F_NOCACHE, 1));

	char *bufs[2];

	bufs[0] = (char *)(((uintptr_t)valloc(size + 65536) + 16383) & ~16383) + 4096;
	bufs[1] = valloc(size);

	for (int pass = 0; pass < 2; ++pass) {	
		state = 0;

		bzero(bufs[0], size);
		memset(bufs[1], 0xff, size);

		run_thread = true;

		pthread_t thread;
		pthread_create(&thread, NULL, read_thread, NULL);

		struct timeval start;
		gettimeofday(&start, NULL);

		for (;;) {
			int cur_state = state;

			switch (pass) {
			case 0:
				check_io(pwrite(fd, bufs[cur_state ? 0 : 1], size, 0), size);
				break;

			case 1:;
				struct iovec iovs[] = {
					{ bufs[cur_state ? 0 : 1], size / 2 },
					{ bufs[cur_state ? 0 : 1] + size / 2, size / 2 }
				};

				assert_no_err(lseek(fd, 0, SEEK_SET));
				check_io(writev(fd, iovs, 2), size);
				break;
			}

			pthread_mutex_lock(&mutex);
			while (state == cur_state) {
				struct timeval tv;

				gettimeofday(&tv, NULL);

				struct timespec ts;
				TIMEVAL_TO_TIMESPEC(&tv, &ts);

				++ts.tv_sec;

				int res = pthread_cond_timedwait(&cond, &mutex, &ts);

				if (res) {
					return -1;
				}
			}

			pthread_mutex_unlock(&mutex);

			// If 10 seconds are up, we're done
			struct timeval now, elapsed;
			gettimeofday(&now, NULL);
			timersub(&now, &start, &elapsed);
			if (elapsed.tv_sec >= 10)
				break;
		} // for (;;)

		run_thread = false;
		pthread_join(thread, NULL);
	} // for (pass...)

	assert_no_err(unlink(file1));

	assert_no_err (close(fd));
	assert_no_err (close(fd2));
	free(file1);
	free(file2);
	
	return 0;
}
