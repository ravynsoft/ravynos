#include <sys/fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/param.h>
#include <sys/mount.h>

#include "hfs-tests.h"
#include "test-utils.h"
#include "disk-image.h"

TEST(raw_dev_unaligned, .run_as_root = 1);

int run_raw_dev_unaligned(__unused test_ctx_t *ctx)
{
	struct statfs sfs;
	
	assert(statfs("/tmp", &sfs) == 0);
	if (strcmp(sfs.f_fstypename, "hfs")) {
		printf("raw_dev_aligned needs hfs as root file system - skipping.\n");
		return 0;
	}
	
	int fd = open("/tmp/raw-dev-unaligned", O_CREAT | O_RDWR | O_TRUNC, 0666);

	assert_with_errno(fd >= 0);

	assert_no_err(ftruncate(fd, 4096));

	assert_no_err(fcntl(fd, F_FULLFSYNC));

	struct log2phys l2p = {};

	assert_no_err(fcntl(fd, F_LOG2PHYS_EXT, &l2p));

	assert(!strncmp(sfs.f_mntfromname, "/dev/disk", 9));

	char *rdev;
	asprintf(&rdev, "/dev/rdisk%s", sfs.f_mntfromname + 9);

	int raw_fd;
	assert_with_errno((raw_fd = open(rdev, O_RDWR)) >= 0);

	void *buf = malloc(16384);

	off_t offset = l2p.l2p_devoffset;

	// Make p non-aligned
	char *p = (char *)((((uintptr_t)buf + 64) & ~63) + 8);

	unsigned bs = 4096;
	if (bs < sfs.f_bsize)
		bs = sfs.f_bsize;

	check_io(pread(raw_fd, p, bs, offset), bs);

	// Make a change
	check_io(pwrite(fd, &fd, 4, 0), 4);

	assert_no_err(fcntl(fd, F_FULLFSYNC));

	char *state = malloc(bs);

	/*
	 * Make sure it changed on the raw device so we know we've got the
	 * correct location.  We can't actually check the contents because
	 * it's encrypted on iOS.
	 */
	check_io(pread(raw_fd, state, bs, offset), bs);

	assert(memcmp(state, p, bs));

	assert_no_err(close(fd));

	for (int i = 0; i < 3000; ++i) {
		for (unsigned i = 0; i < bs; ++i)
			state[i] ^= 0xff;
		memcpy(p, state, bs);
		check_io(pwrite(raw_fd, p, bs, offset), bs);
		check_io(pread(raw_fd, p, bs, offset), bs);
		assert(!memcmp(p, state, bs));
	}

	// cleanup
	assert_no_err(close(raw_fd));
	assert_no_err(unlink("/tmp/raw-dev-unaligned"));
	free(buf);
	free(state);

	return 0;
}
