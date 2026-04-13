#include <TargetConditionals.h>

#if (TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR)

#include <sys/fcntl.h>
#include <string.h>
#include <stdio.h>
#include <sys/errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <stdbool.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/mount.h>

#include "hfs-tests.h"
#include "test-utils.h"
#include "disk-image.h"

TEST(dprotect, .run_as_root = true)

#define TEST_FILE "/tmp/dprotect.data"

int run_dprotect(__unused test_ctx_t *ctx)
{
	// The root file system needs to be HFS
	struct statfs sfs;
	
	assert(statfs("/tmp", &sfs) == 0);
	if (strcmp(sfs.f_fstypename, "hfs")) {
		printf("dprotect needs hfs as root file system - skipping.\n");
		return 0;
	}
	
	unlink(TEST_FILE);
	int fd = open_dprotected_np(TEST_FILE, O_RDWR | O_CREAT,
								2, 0, 0666);
	assert_with_errno(fd >= 0);

	char *ones= valloc(4096), *buf = valloc(16384);

	memset(ones, 0xff, 4096);
	memset(buf, 0xff, 4096);

	check_io(write(fd, buf, 4096), 4096);

	assert_no_err(fsync(fd));

	assert_no_err(close(fd));

	fd = open_dprotected_np(TEST_FILE, O_RDONLY, 0, O_DP_GETRAWENCRYPTED);
	assert(fd >= 0);

	check_io(pread(fd, buf, 4096, 0), 4096);
	
	if (!memcmp(ones, buf, 4096))
		assert_fail("data should be encrypted (make sure the volume you're running on has content protection enabled)");

	assert_no_err(unlink(TEST_FILE));
	free(ones);
	free(buf);
	
	return 0;
}

#endif // TARGET_OS_IPHONE & !SIM
