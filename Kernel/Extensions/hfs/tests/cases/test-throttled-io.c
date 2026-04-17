#include <TargetConditionals.h>

#if !TARGET_OS_BRIDGE

#include <sys/fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <CommonCrypto/CommonDigest.h>
#include <stdbool.h>
#include <stdio.h>
#include <spawn.h>
#include <string.h>
#include <signal.h>
#include <stdarg.h>
#include <sys/errno.h>
#include <libkern/OSAtomic.h>
#include <zlib.h>
#include <pthread.h>
#include <sys/mount.h>

#include "hfs-tests.h"
#include "test-utils.h"
#include "disk-image.h"

#define TEST_PATH "/tmp/throttled-io.sparseimage"
#define MOUNT_POINT "/tmp/throttled_io"

TEST(throttled_io)

static disk_image_t *gDI;
static char *gFile1, *gFile2, *gFile3;

static void *gBuf;
static const size_t gBuf_size = 64 * 1024 * 1024;

static pid_t bg_io_pid = 0;
static size_t BG_IOSIZE = (4U << 10);         // BG-IO buffer size 4KiB
static off_t BG_MAX_FILESIZE = (1ULL << 30);  // Max BG-IO File-size 1GiB

//
// A worker function called from the background-io child process. First it
// attempts to open file at path `gFile1` ( a new file is created if one does
// does not exist). If the file is opened successfully, the file is written
// continiously, wrapping around when write offset is greater or equal to
// `BG_MAX_FILESIZE`.
//
errno_t background_io_worker(void)
{
	int fd;
	off_t offset;
	char *buffer;

	//
	// Open the file at path `gFile1`, create a new file if one does not
	// exists.
	//
	fd = open(gFile1, O_RDWR|O_TRUNC);
	if (fd == -1 && errno == ENOENT) {

		fd = creat(gFile1, 0666);
		if (fd == -1) {
			fprintf(stderr, "Failed to create file: %s\n",
					gFile1);
			return errno;
		}
		close(fd);
		fd = open(gFile1, O_RDWR);
	}

	//
	// Return errno if we could not open the file.
	//
	if (fd  == -1) {
		fprintf(stderr, "Failed to open file: %s\n", gFile1);
		return errno;
	}

	//
	// Allocate the write buffer on-stack such that we don't have to free
	// it explicitly.
	//
	buffer = alloca(BG_IOSIZE);
	if (!buffer)
		return ENOMEM;
	(void)memset(buffer, -1, BG_IOSIZE);

	offset = 0;
	while (true) {
		ssize_t written;

		written = pwrite(fd, buffer, BG_IOSIZE, offset);
		if (written == -1) {
			return errno;
		}

		offset += written;
		if (offset >= BG_MAX_FILESIZE) {
			offset = 0;
		}

		//
		// Voluntarily relinquish cpu to allow the throttled process to
		// schedule after every 128 MiB of write, else the test can
		// take very long time and timeout.  Sleep half a second after
		// we have written 128 MiB.
		//
		if (!(offset %  (off_t)(BG_IOSIZE * 1024 * 32))) {
			usleep(500000);
		}

		//
		// Just in case the test times-out for some reason and parent
		// terminates without killing this background-io process, let
		// us poll this child process's parent. If the parent process
		// has died then to ensure cleanup we return from this child
		// process.
		//
		if (getppid() == 1) {
			return ETIMEDOUT;
		}
	}

	//
	// Should not come here.
	//
	return 0;
}

//
// Start a continious background-io process, if successful the pid of the
// background-io is cached in `bg_io_pid'.
//
static void start_background_io_process(void)
{
	pid_t child_pid;

	child_pid = fork();
	switch(child_pid) {

		case -1:
			assert_fail("Failed to spawn background-io "
					"process, error %d, %s.\n", errno,
					strerror(errno));
		case 0: {
				int child_ret;

				child_ret = background_io_worker();
				_exit(child_ret);
		}
		default:
			bg_io_pid = child_pid;
	}
}

//
// Kill the background-io process if it was started. The background-io process
// should perform IO continuously and should not exit normally. If the
// background-io exited normally, the error is reported.
//
static void kill_background_io_process(void)
{
	int child_status;
	pid_t child_pid;

	if (!bg_io_pid)
		return;

	kill(bg_io_pid, SIGKILL);
	do {
		child_pid = waitpid(bg_io_pid, &child_status, WUNTRACED);
	} while (child_pid == -1 && errno == EINTR);

	if (child_pid == -1 && errno != ECHILD) {
		assert_fail("Failed to wait for child pid: %ld, error %d, "
				"%s.\n", (long)bg_io_pid,  errno,
				strerror(errno));
	}

	if (WIFEXITED(child_status)) {
		int error;

		error = WEXITSTATUS(child_status);
		if (error) {
			assert_fail("background-io exited with error %d, "
					"%s.\n", error, strerror(error));
		}
	}

	bg_io_pid = 0;
}

static int run_test1(void)
{
	int fd, fd2;
	int orig_io_policy;

	//
	// Kick off another process to ensure we get throttled.
	//
	start_background_io_process();

	//
	// Cache the set IO policy of this process.
	//
	orig_io_policy = getiopolicy_np(IOPOL_TYPE_DISK, IOPOL_SCOPE_PROCESS);
	assert_with_errno(orig_io_policy != -1);

	//
	// Set new IO policy for this process.
	//
	assert_no_err(setiopolicy_np(IOPOL_TYPE_DISK, IOPOL_SCOPE_PROCESS,
				IOPOL_THROTTLE));

	assert_with_errno((fd = open("/dev/random", O_RDONLY)) >= 0);
	assert_with_errno((fd2 = open(gFile2, O_RDWR | O_CREAT | O_TRUNC,
					0666)) >= 0);

	assert_no_err(fcntl(fd2, F_SINGLE_WRITER, 1));
	assert_no_err(fcntl(fd2, F_NOCACHE, 1));

	gBuf = valloc(gBuf_size);
	CC_SHA1_CTX ctx;
	CC_SHA1_Init(&ctx);

	ssize_t res = check_io(read(fd, gBuf, gBuf_size), gBuf_size);

	CC_SHA1_Update(&ctx, gBuf, (CC_LONG)res);

	res = check_io(write(fd2, gBuf, res), res);

	bzero(gBuf, gBuf_size);

	CC_SHA1_CTX ctx2;
	CC_SHA1_Init(&ctx2);

	lseek(fd2, 0, SEEK_SET);

	res = check_io(read(fd2, gBuf, gBuf_size), gBuf_size);

	CC_SHA1_Update(&ctx2, gBuf, (CC_LONG)res);

	uint8_t digest1[CC_SHA1_DIGEST_LENGTH], digest2[CC_SHA1_DIGEST_LENGTH];
	CC_SHA1_Final(digest1, &ctx);
	CC_SHA1_Final(digest2, &ctx2);

	assert(!memcmp(digest1, digest2, CC_SHA1_DIGEST_LENGTH));

	assert_no_err (close(fd));
	assert_no_err (close(fd2));

	//
	// Kill the background IO process.
	//
	kill_background_io_process();

	//
	// Restore the orig. IO policy.
	//
	assert_no_err(setiopolicy_np(IOPOL_TYPE_DISK, IOPOL_SCOPE_PROCESS,
				orig_io_policy));
	return 0;
}

static volatile uint64_t written;
static volatile bool done;

static void test2_thread(void)
{
	int fd = open(gFile3, O_RDONLY);
	assert(fd >= 0);

	void *b = gBuf + gBuf_size / 2;
	uLong seq = crc32(0, Z_NULL, 0);
	uint32_t offset = 0;

	do {
		ssize_t res;

		do {
			res = check_io(pread(fd, b, gBuf_size / 2, offset), -1);
		} while (res == 0 && !done);

		assert (res % 4 == 0);

		offset += res;

		for (uLong *p = b; res; ++p, res -= sizeof(uLong)) {
			seq = crc32(Z_NULL, (void *)&seq, 4);
			assert(*p == seq);
		}

		if (offset < written)
			continue;
		OSMemoryBarrier();
	} while (!done);
	
	assert_no_err (close(fd));
}

static int run_test2(void)
{
	int fd;
	int orig_io_policy;

	//
	// Kick off another process to ensure we get throttled.
	//
	start_background_io_process();

	//
	// Cache the set IO policy of this process.
	//
	orig_io_policy = getiopolicy_np(IOPOL_TYPE_DISK, IOPOL_SCOPE_PROCESS);
	assert_with_errno(orig_io_policy != -1);

	//
	// Set new IO policy for this process.
	//
	assert_no_err(setiopolicy_np(IOPOL_TYPE_DISK, IOPOL_SCOPE_PROCESS,
				IOPOL_THROTTLE));

	fd = open(gFile3, O_RDWR | O_CREAT | O_TRUNC, 0666);
	assert(fd >= 0);

	assert_no_err(fcntl(fd, F_SINGLE_WRITER, 1));
	assert_no_err(fcntl(fd, F_NOCACHE, 1));

	pthread_t thread;
	pthread_create(&thread, NULL, (void *(*)(void *))test2_thread, NULL);
	uLong seq = crc32(0, Z_NULL, 0);

	for (int i = 0; i < 4; ++i) {
		uLong *p = gBuf;
		for (unsigned i = 0; i < gBuf_size / 2 / sizeof(uLong); ++i) {
			seq = crc32(Z_NULL, (void *)&seq, 4);
			p[i] = seq;
		}

		ssize_t res = check_io(write(fd, gBuf, gBuf_size / 2), gBuf_size / 2);

		written += res;
	}

	OSMemoryBarrier();

	done = true;

	pthread_join(thread, NULL);

	assert_no_err (close(fd));

	//
	// Kill the background IO process.
	//
	kill_background_io_process();

	//
	// Restore the orig. IO policy.
	//
	assert_no_err(setiopolicy_np(IOPOL_TYPE_DISK, IOPOL_SCOPE_PROCESS,
				orig_io_policy));

	return 0;
}

static bool clean_up(void)
{
	kill_background_io_process();

	unlink(gFile1);
	unlink(gFile2);
	unlink(gFile3);
	
	free(gFile1);
	free(gFile2);
	free(gFile3);
	
	return true;
}

int run_throttled_io(__unused test_ctx_t *ctx)
{

	gDI = disk_image_create(TEST_PATH, &(disk_image_opts_t){
			.size = 8 GB,
			.mount_point = MOUNT_POINT
			});
	
	asprintf(&gFile1, "%s/throttled_io.1", gDI->mount_point);
	asprintf(&gFile2, "%s/throttled_io.2", gDI->mount_point);
	asprintf(&gFile3, "%s/throttled_io.3", gDI->mount_point);
	
	test_cleanup(^ bool {
		return clean_up();
	});

	int res = run_test1();
	if (res)
		return res;

	res = run_test2();
	if (res)
		return res;
	
	return res;
}

#endif // !TARGET_OS_BRIDGE
