#include <TargetConditionals.h>

#if !(TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR)

#include <sys/mman.h>
#include <unistd.h>
#include <spawn.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/mount.h>

#include <Foundation/Foundation.h>

#include "hfs-tests.h"
#include "test-utils.h"
#include "systemx.h"
#include "disk-image.h"

TEST(map_private)

#define DISK_IMAGE "/tmp/map-private.sparseimage"

static char zero[65536];

static jmp_buf jmp_env;

static void handle_sigbus(int signal)
{
	assert(signal == SIGBUS);

	longjmp(jmp_env, 1);
}

int run_map_private(__unused test_ctx_t *ctx)
{
	disk_image_t *di = disk_image_create(DISK_IMAGE, &(disk_image_opts_t){
														.size = 64 * 1024 * 1024
													});
	
	char *path;
	asprintf(&path, "%s/map-private.data", di->mount_point);

	int fd;
	void *p;

	assert_with_errno((fd = open(path, O_RDWR | O_CREAT, 0666)) >= 0);

	assert_no_err(ftruncate(fd, 65536));

	assert_no_err(fcntl(fd, F_FULLFSYNC));

	assert_with_errno((p = mmap(NULL, 65536, PROT_READ | PROT_WRITE,
								MAP_PRIVATE, fd, 0)) != MAP_FAILED);

	assert_no_err(close(fd));

	assert_no_err(unlink(path));

	// Create a second file that we keep open via a file descriptor
	char *path2;
	asprintf(&path2, "%s/map-private-2.data", di->mount_point);

	assert_with_errno((fd = open(path2, O_RDWR | O_CREAT, 0666)) >= 0);

	assert_no_err(ftruncate(fd, 65536));

	assert_no_err(fcntl(fd, F_FULLFSYNC));

	assert_no_err(unlink(path2));

	assert(!systemx("/usr/sbin/diskutil", SYSTEMX_QUIET, "unmount", "force", di->mount_point, NULL));

	/*
	 * Forcibly unmounting should not have caused all the data
	 * to be paged in so this should result in a fault.
	 */

	// Set things up to catch the SIGBUS
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGBUS);

	sigprocmask(SIG_BLOCK, &set, NULL);

	struct sigaction sa = {
		.sa_handler = handle_sigbus,
		.sa_flags = SA_NODEFER | SA_RESETHAND | SA_SIGINFO,
	};

	sigaction(SIGBUS, &sa, NULL);
	pthread_sigmask(SIG_UNBLOCK, &set, NULL);

	if (!setjmp(jmp_env)) {
		if(memcmp(p, zero, 65536)) // Need this if statement so the memcmp isn't optimized out
			assert_fail("memcmp should have faulted");
		else
			assert_fail("... memcmp should have faulted");
	}

	// Now remount the volume and make sure file has been deleted
	assert(!systemx("/usr/sbin/diskutil", SYSTEMX_QUIET, "mount", di->disk, NULL));

	/*
	 * We assume that it will get mounted at the same place, which
	 * is reasonable given the environment this should be running
	 * in.
	 */
	struct stat sb;
	assert(stat(path, &sb) == -1 && errno == ENOENT);

	// Now check that common open unlink behaviour isn't broken

	// First check disk space
	struct statfs sfs;
	assert_no_err(statfs(di->mount_point, &sfs));

	// Should be at least 7 MB
	uint64_t space = sfs.f_bfree * sfs.f_bsize;

#define MB * 1024 * 1024

	assert(space > 7 MB);

	assert_with_errno((fd = open(path, O_RDWR | O_CREAT, 0666)) >= 0);

	assert_no_err(ftruncate(fd, 7 MB));

	assert_no_err(statfs(di->mount_point, &sfs));

	// Space should have dropped by at least 5 MB
	assert(sfs.f_bfree * sfs.f_bsize < space - 5 MB);

	assert_no_err(unlink(path));

	// Map the file
	assert_with_errno((p = mmap(NULL, 65536, PROT_READ | PROT_WRITE,
								MAP_PRIVATE, fd, 0)) != MAP_FAILED);

	assert_no_err(close(fd));

	// File is still in use, so space should not have changed
	assert_no_err(statfs(di->mount_point, &sfs));

	assert(sfs.f_bfree * sfs.f_bsize < space - 5 MB);

	// Get rid of the last reference
	assert_no_err(munmap(p, 65536));

	// Just in case we collide with sync
	sleep(1);

	// Space should be back up to at least 7 MB free
	assert_no_err(statfs(di->mount_point, &sfs));

	assert(sfs.f_bfree * sfs.f_bsize > 7 MB);

	return 0;
}

#endif // !(TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR)
