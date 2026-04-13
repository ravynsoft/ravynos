#include <TargetConditionals.h>

#if (TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR)

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/attr.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/stat.h>
#include <sys/xattr.h>
#include <sys/mount.h>
#include <sys/param.h>
#include <CommonCrypto/CommonDigest.h>
#include <libkern/OSAtomic.h>
#include <pthread.h>
#include <spawn.h>
#include <MobileKeyBag/MobileKeyBag.h>
#include <hfs/hfs_fsctl.h>

#include "hfs-tests.h"
#include "test-utils.h"
#include "systemx.h"

TEST(key_roll, .run_as_root = true)

static void *buf1;

#define MB * 1024 * 1024

#define F_RECYCLE 84

#define KEY_ROLL_TEST_FILE			"/tmp/key-roll-test.data"
#define KEY_ROLL_TEST_FILE_2		"/tmp/key-roll-test-2.data"
#define KEY_ROLL_FILL_DISK_FILE		"/tmp/key-roll-fill-disk"
#define KEY_ROLL_TEST_DIR			"/tmp/key-roll-test.dir"
#define KEY_ROLL_SYM_LINK			"/tmp/key-roll-test.symlink"
#define KEYSTORECTL					"/usr/local/bin/keystorectl"
#define KEYBAGDTEST					"/usr/local/bin/keybagdTest"

int cmp_zero(const void *p, size_t len)
{
	const uint8_t *x = p;

	while (len && (uintptr_t)x & 7) {
		if (*x)
			return 1;
		++x;
		--len;
	}

	const uint64_t *y = (uint64_t *)x;
	while (len >= 8) {
		if (*y)
			return 1;
		++y;
		len -= 8;
	}

	x = (uint8_t *)y;
	while (len) {
		if (*x)
			return 1;
		++y;
		--len;
	}

	return 0;
}

struct append_ctx {
	int fd;
	uint8_t digest[CC_MD5_DIGEST_LENGTH];
	bool done;
};

#if 1
static const int append_test_amount = 128 MB;
#else
#warning
static const int append_test_amount = 1 MB;
#endif

void *append_to_file(void *param)
{
	struct append_ctx *ctx = param;

	CC_MD5_CTX md5_ctx;
	CC_MD5_Init(&md5_ctx);

	uint64_t total = 0;

	void *p = mmap(NULL, append_test_amount, PROT_READ | PROT_WRITE, 
				   MAP_SHARED, ctx->fd, 0);
	assert(p != MAP_FAILED);

	int page_size = getpagesize();

	while (total < append_test_amount) {
		size_t todo = random() % (1 MB) + 1;

		if (todo > append_test_amount - total)
			todo = append_test_amount - total;

		check_io(write(ctx->fd, buf1, todo), todo);

		int round = ((uintptr_t)p + total) % page_size;

		assert_no_err(msync(p + total - round, todo + round, 
							MS_ASYNC | MS_INVALIDATE));

		CC_MD5_Update(&md5_ctx, buf1, (CC_LONG)todo);

		total += todo;
	}

	CC_MD5_Final(ctx->digest, &md5_ctx);

	OSMemoryBarrier();

	ctx->done = true;

	assert_no_err(munmap(p, append_test_amount));

	return NULL;
}

static uint32_t os_version(void)
{
	static uint32_t os_version;
	if (os_version)
		return os_version;

	char os_version_str[128];
	size_t size = sizeof(os_version_str);
	assert_no_err(sysctlbyname("kern.osversion", os_version_str,
							   &size, NULL, 0));

	const char *p = os_version_str;

	int a = 0;
	while (*p >= '0' && *p <= '9') {
		a = a * 10 + *p - '0';
		++p;
	}

	if (!a)
		return 0;

	int b = *p++;
	if (!b)
		return 0;

	int c = 0;
	while (*p >= '0' && *p <= '9') {
		c = c * 10 + *p - '0';
		++p;
	}

	if (!c)
		return 0;

	os_version = (a & 0xff) << 24 | b << 16 | (c & 0xffff);

	return os_version;
}

static int block_size(void)
{
	static int block_size;

	if (!block_size) {
		struct statfs sfs;

		assert_no_err(statfs("/private/var", &sfs));

		block_size = sfs.f_bsize;
	}

	return block_size;
}

static void fill_disk(int *fd, uint64_t *size)
{
	assert_with_errno((*fd = open(KEY_ROLL_FILL_DISK_FILE,
								  O_CREAT | O_RDWR | O_TRUNC, 0666)) >= 0);

	// Fill up the disk so there's no remaining disk space
	struct statfs sfs;
	assert_no_err(fstatfs(*fd, &sfs));

	uint64_t blocks = sfs.f_bfree;

	for (;;) {
		uint64_t size = blocks * sfs.f_bsize;

		if (!fcntl(*fd, F_SETSIZE, &size))
			break;

		assert_with_errno(errno == ENOSPC);

		blocks /= 2;
	}

	// Now increase the size until we hit no space
	uint64_t upper = sfs.f_bfree + 128;

	for (;;) {
		uint64_t try = (upper + blocks) / 2;

		if (try <= blocks)
			try = blocks + 1;

		uint64_t size = try * sfs.f_bsize;
		if (!fcntl(*fd, F_SETSIZE, &size)) {
			blocks = try;
			if (try >= upper) {
				assert_no_err(fstatfs(*fd, &sfs));
				upper = try + sfs.f_bfree + 128;
			}
		} else {
			assert(errno == ENOSPC);

			if (try == blocks + 1)
				break;
			else
				upper = try;
		}
	}

	*size = blocks * sfs.f_bsize;
}

volatile int32_t threads_running;

static void *roll_thread(void *arg __attribute__((unused)))
{
	int fd;

	assert_with_errno((fd = open(KEY_ROLL_TEST_FILE, O_RDWR)) >= 0);

	hfs_key_roll_args_t args = {
		.api_version = HFS_KR_API_LATEST_VERSION,
		.operation = HFS_KR_OP_STEP,
	};

	for (int i = 0; i < 100; ++i) {
		assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));

		if (args.done == -1)
			args.operation = HFS_KR_OP_START;
		else
			args.operation = HFS_KR_OP_STEP;
	}

	assert_no_err(close(fd));

	OSAtomicDecrement32(&threads_running);

	return NULL;
}

int run_key_roll(__unused test_ctx_t *ctx)
{
	// The root file system needs to be HFS
	struct statfs sfs;
	
	assert(statfs("/tmp", &sfs) == 0);
	if (strcmp(sfs.f_fstypename, "hfs")) {
		printf("key_roll needs hfs as root file system - skipping.\n");
		return 0;
	}
	
	int fd;
	void *read_buf = malloc(2 MB);
	void *p;

	struct attrlist attrlist = {
		.bitmapcount = ATTR_BIT_MAP_COUNT,
		.commonattr = ATTR_CMN_DATA_PROTECT_FLAGS,
	};

	struct attrs {
		uint32_t len;
		uint32_t dp_flags;
	} attrs;

	// Clean up previous invocation--we don't care about failures here
	unlink(KEY_ROLL_TEST_FILE_2);
	unlink(KEY_ROLL_FILL_DISK_FILE);
	unlink(KEY_ROLL_TEST_FILE);
	systemx("/bin/rm", "-rf", KEY_ROLL_TEST_DIR, NULL);

	buf1 = malloc(1 MB);
	memset(buf1, 0x25, 1 MB);

	void *buf2 = malloc(1 MB);
	memset(buf2, 0x49, 1 MB);

	// First, force change to new xattr version

	assert_with_errno((fd = open(KEY_ROLL_TEST_FILE,
								 O_CREAT | O_RDWR | O_TRUNC, 0666)) >= 0);

	// Write 3 MB
	check_io(write(fd, buf1, 1 MB), 1 MB);
	check_io(write(fd, buf1, 1 MB), 1 MB);
	check_io(write(fd, buf1, 1 MB), 1 MB);

	hfs_key_roll_args_t args = {
		.api_version = HFS_KR_API_LATEST_VERSION,
		.operation = HFS_KR_OP_START,
	};
	assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));

	args.operation = HFS_KR_OP_STEP;
	assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));

	assert_no_err(unlink(KEY_ROLL_TEST_FILE));
	assert_no_err(close(fd));

	/*
	 * That should have switch the device to new xattr version.  Continue
	 * with more tests now...
	 */

	assert_with_errno((fd = open(KEY_ROLL_TEST_FILE,
								 O_CREAT | O_RDWR | O_TRUNC, 0666)) >= 0);

	// Write 2 MB
	check_io(write(fd, buf1, 1 MB), 1 MB);
	check_io(write(fd, buf1, 1 MB), 1 MB);

	assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));

	assert(args.key_revision == 1
		   && args.key_os_version == os_version()
		   && args.done == -1
		   && args.total == 2 MB);

	// Extend file to 3 MB
	assert_no_err(ftruncate(fd, 3 MB));

	// Start rolling
	args.operation = HFS_KR_OP_START;
	assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));

	assert((args.key_revision & 0xff00) == 0x0100 && args.done == 0
		   && args.total == 3 MB);

	// Roll 1 chunk
	args.operation = HFS_KR_OP_STEP;
	assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));

	assert(args.done == 2 MB);

	// Write a little way into the last MB
	off_t offset = 2 MB + 50000;

	check_io(pwrite(fd, buf2, 1024, offset), 1024);

	// Roll to end of file
	args.operation = HFS_KR_OP_STEP;
	assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));

	// This time, it should have finished
	assert(args.done == -1);

	// Now check all is as we expect
	assert_with_errno((p = mmap(NULL, 3 MB, PROT_READ | PROT_WRITE,
								MAP_SHARED, fd, 0)) != MAP_FAILED);

	// Force flush of cache
	assert_no_err(msync(p, 3 MB, MS_INVALIDATE));

	assert(!memcmp(p, buf1, 1 MB));
	assert(!memcmp(p + 1 MB, buf1, 1 MB));
	assert(!cmp_zero(p + 2 MB, 50000));
	assert(!memcmp(p + offset, buf2, 1024));
	assert(!cmp_zero(p + offset + 1024, 1 MB - 50000 - 1024));

	// -- Rewrapping tests --

	assert_no_err(fgetattrlist(fd, &attrlist, &attrs, sizeof(attrs), 0));

	// File should be class D
	assert((attrs.dp_flags & 0x1f) == 4);

	// Start rolling
	args.operation = HFS_KR_OP_START;
	args.flags = HFS_KR_MATCH_KEY_REVISION;

	assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));

	// Reset flags for later tests
	args.flags = 0;

	assert(args.done == 0 && (args.key_revision & 0xff00) == 0x0200);

	// Roll 1 chunk
	args.operation = HFS_KR_OP_STEP;
	assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));

	assert(args.done == 2 MB);

	// Change file to class C
	assert_no_err(fcntl(fd, F_SETPROTECTIONCLASS, 3));

	assert_no_err(fgetattrlist(fd, &attrlist, &attrs, sizeof(attrs), 0));
	assert((attrs.dp_flags & 0x1f) == 3);

	// Force file to be recycled (won't work on release builds)
	bool release_build = false;

	if (fcntl(fd, F_RECYCLE)) {
		assert_equal_int(errno, ENOTTY);
		release_build = true;
	}

	// Release refs so recycle happens
	assert_no_err(close(fd));
	assert_no_err(munmap(p, 3 MB));

	// Now check the file
	assert_with_errno((fd = open(KEY_ROLL_TEST_FILE, O_RDWR)) >= 0);
	assert_with_errno((p = mmap(NULL, 3 MB, PROT_READ | PROT_WRITE,
								MAP_SHARED, fd, 0)) != MAP_FAILED);

	args.operation = HFS_KR_OP_STATUS;
	assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));

	assert(args.done == 2 MB);

	// Check the content
	assert(!memcmp(p, buf1, 1 MB));
	assert(!memcmp(p + 1 MB, buf1, 1 MB));
	assert(!cmp_zero(p + 2 MB, 50000));
	assert(!memcmp(p + offset, buf2, 1024));
	assert(!cmp_zero(p + offset + 1024, 1 MB - 50000 - 1024));

	// Check the class again
	assert_no_err(fgetattrlist(fd, &attrlist, &attrs, sizeof(attrs), 0));
	assert((attrs.dp_flags & 0x1f) == 3);

	// Change to class 1
	assert_no_err(fcntl(fd, F_SETPROTECTIONCLASS, 1));

	// Check it
	assert_no_err(fgetattrlist(fd, &attrlist, &attrs, sizeof(attrs), 0));
	assert((attrs.dp_flags & 0x1f) == 1);

	assert_with_errno(release_build || !fcntl(fd, F_RECYCLE));

	// Should get recycled after this
	assert_no_err(close(fd));
	assert_no_err(munmap(p, 3 MB));

	int fd2 = open(KEY_ROLL_TEST_FILE_2, O_RDWR | O_CREAT, 0666);

	/*
	 * We can't check this file until we've triggered an unlock
	 * which means we need to set a system password.
	 */

	// Change system password
	assert_no_err(systemx(KEYSTORECTL, "change-password", "", "1234", NULL));

	// Now we can check the file

	assert_with_errno((fd = open(KEY_ROLL_TEST_FILE, O_RDWR)) >= 0);
	assert_with_errno((p = mmap(NULL, 3 MB, PROT_READ | PROT_WRITE,
								MAP_SHARED, fd, 0)) != MAP_FAILED);

	assert(!memcmp(p, buf1, 1 MB));
	
	// Open for raw access
	int raw_fd;
	assert_with_errno((raw_fd = open_dprotected_np(KEY_ROLL_TEST_FILE, 
												   O_RDONLY, 0, 1, 0)) >= 0);

	// Lock device
	assert_no_err(systemx(KEYBAGDTEST, "lock", NULL));

	// Wait until the device is locked
	while (MKBGetDeviceLockState(NULL) != kMobileKeyBagDeviceIsLocked)
		sleep(1);

	// Set second file to class B
	assert_no_err(fcntl(fd2, F_SETPROTECTIONCLASS, 2));

	// Make sure we can write to it
	check_io(write(fd2, buf1, 1 MB), 1 MB);

	assert_no_err(close(fd2));
	assert_no_err(unlink(KEY_ROLL_TEST_FILE_2));

	// Try and read data
	assert(read(fd, read_buf, 1 MB) == -1 && errno == EPERM);

	// Now try and continue rolling

	args.operation = HFS_KR_OP_STEP;
	assert(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0) == -1
		   && errno == EPERM);

	// Make sure we can get the status of the file
	args.operation = HFS_KR_OP_STATUS;
	assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));

	// Make sure reading the raw file fails
	assert(read(raw_fd, read_buf, 1 MB) == -1 && errno == EPERM);

	// Make sure opening the file in raw mode fails
	assert(open_dprotected_np(KEY_ROLL_TEST_FILE, O_RDONLY, 0, 1, 0)
		   == -1 && errno == EPERM);

	assert_no_err(systemx(KEYSTORECTL, "unlock", "1234", NULL));

	// Now check the raw read works
	check_io(read(raw_fd, read_buf, 1 MB), 1 MB);

	assert_no_err(close(raw_fd));

	// Check data

	assert(!memcmp(p, buf1, 1 MB));

	// Change system password back

	assert_no_err(systemx(KEYSTORECTL, "change-password", "1234", "", NULL));

	// -- Raw mode tests --

	// Open the file in raw mode
	assert_with_errno((raw_fd = open_dprotected_np(KEY_ROLL_TEST_FILE, 
												   O_RDONLY, 0, 1, 0)) >= 0);

	// Key rolling should be unchanged
	args.operation = HFS_KR_OP_STATUS;
	assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));
	assert(args.done == 2 MB && (args.key_revision & 0xff00) == 0x0200);

	// Issue a read
	check_io(read(raw_fd, read_buf, 2 MB), 2 MB);

	// Key rolling status should be remain unchanged
	args.operation = HFS_KR_OP_STATUS;
	assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));
	assert(args.done == 2 MB && (args.key_revision & 0xff00) == 0x0200);

	// Issue more reads
	check_io(read(raw_fd, read_buf, 2 MB), 1 MB);

	// Key rolling should have been finished

	args.operation = HFS_KR_OP_STATUS;
	assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));
	assert(args.done == -1 && (args.key_revision & 0xff00) == 0x0200);

	assert_no_err(close(raw_fd));

	// Change the revision and os version
	if (release_build) {
		// HFS_KR_OP_SET_INFO isn't supported on release build

		// Enable auto rolling
		hfs_key_auto_roll_args_t auto_roll_args = {
			.api_version = HFS_KEY_AUTO_ROLL_API_LATEST_VERSION,
			.max_key_os_version = os_version() + 1,
		};

		assert_no_err(fsctl("/private/var", HFSIOC_SET_KEY_AUTO_ROLL, &auto_roll_args, 0));
	} else {
		args.operation = HFS_KR_OP_SET_INFO;
		args.key_revision = 0x0200;
		args.key_os_version = CP_OS_VERS_PRE_71;
		assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));

		args.operation = HFS_KR_OP_STATUS;
		assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));

		assert(args.done == -1
			   && args.key_revision == 0x0200
			   && args.key_os_version == CP_OS_VERS_PRE_71);

		// Enable auto rolling
		hfs_key_auto_roll_args_t auto_roll_args = {
			.api_version = HFS_KEY_AUTO_ROLL_API_LATEST_VERSION,
			.max_key_os_version = CP_OS_VERS_71,
		};

		assert_no_err(fsctl("/private/var", HFSIOC_SET_KEY_AUTO_ROLL, &auto_roll_args, 0));
	}

	// Open the file in raw mode
	assert_with_errno((raw_fd = open_dprotected_np(KEY_ROLL_TEST_FILE, 
												   O_RDONLY, 0, 1, 0)) >= 0);

	// That should have initiated key rolling
	args.operation = HFS_KR_OP_STATUS;
	assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));
	assert(args.done == 0 && (args.key_revision & 0xff00) == 0x0300);

	// Issue a read
	check_io(read(raw_fd, read_buf, 1 MB), 1 MB);

	// That should have rolled 2 MB
	assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));
	assert(args.done == 2 MB && (args.key_revision & 0xff00) == 0x0300
		   && args.key_os_version == os_version());

	{
		// Check that reservation is working as expected

		// First figure out where the last block finished
		struct log2phys l2p = {
			.l2p_contigbytes = 1024 * 1024,
			.l2p_devoffset   = 2 MB - block_size(),
		};

		assert_no_err(fcntl(fd, F_LOG2PHYS_EXT, &l2p));
		assert(l2p.l2p_contigbytes == block_size());

		// Now try and extend the file by a block
		fstore_t fstore = {
			.fst_flags = F_ALLOCATECONTIG | F_ALLOCATEALL,
			.fst_posmode = F_VOLPOSMODE,
			.fst_offset  = l2p.l2p_devoffset + block_size(),
			.fst_length  = 3 MB + block_size(),
		};

		assert_no_err(fcntl(fd, F_PREALLOCATE, &fstore));
		assert_equal_ll(fstore.fst_bytesalloc, block_size());

		// Force it to be initialised
		check_io(pwrite(fd, buf1, block_size(), 3 MB), block_size());

		// Now see where it was allocated
		l2p.l2p_devoffset = 3 MB;
		l2p.l2p_contigbytes = 1 MB;
		assert_no_err(fcntl(fd, F_LOG2PHYS_EXT, &l2p));

		assert(l2p.l2p_contigbytes == block_size());

		/*
		 * It shouldn't be in the 1 MB spot that should be reserved
		 * for rolling the last bit.
		 */
		if (l2p.l2p_devoffset == -1
			|| (l2p.l2p_devoffset + block_size() > fstore.fst_offset
				&& l2p.l2p_devoffset < fstore.fst_offset + 1 MB)) {
			assert_fail("unexpected allocation (%lld, %lld)\n",
						l2p.l2p_devoffset, fstore.fst_offset);
		}

		// Restore the file to its original length
		assert_no_err(ftruncate(fd, 3 MB));
	}

	// Try and start rolling again, this should succeed and just return status
	args.operation = HFS_KR_OP_START;
	assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));

	assert(args.done == 2 MB && (args.key_revision & 0xff00) == 0x0300);

	check_io(read(raw_fd, read_buf, 1 MB), 1 MB);

	// There should be no change in the roll status
	args.operation = HFS_KR_OP_STATUS;
	assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));

	assert(args.done == 2 MB);

	// Read the last bit
	check_io(read(raw_fd, read_buf, 2 MB), 1 MB);

	// That should have finished key rolling
	args.operation = HFS_KR_OP_STATUS;
	assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));

	assert(args.done == -1);

	// Trying to initiate rolling should fail because we have it open for
	// raw access.
	args.operation = HFS_KR_OP_START;
	assert(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0) == -1
		   && errno == EBUSY);

	assert_no_err(close(raw_fd));

	/*
	 * Make sure we can open directories raw whilst auto-rolling is
	 * enabled.  We've picked a directory here that's class C.
	 */
	assert_with_errno((raw_fd = 
					   open_dprotected_np("/private/var/mobile/Library/Passes",
										  O_RDONLY | O_NOFOLLOW,
										  0, 1, 0)) >= 0);

	assert_no_err(close(raw_fd));

	if (!release_build) {
		/*
		 * This test only works on debug builds because for release
		 * builds we had to set things up so that it always rolls the
		 * file.
		 */

		// Open the file again for raw access
		assert_with_errno((raw_fd = open_dprotected_np(KEY_ROLL_TEST_FILE, 
													   O_RDONLY, 0, 1, 0)) >= 0);

		// Status should remain unchanged
		args.operation = HFS_KR_OP_STATUS;
		assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));

		assert(args.done == -1 && (args.key_revision & 0xff00) == 0x0300
			   && args.key_os_version == os_version());

		assert_no_err(close(raw_fd));
	}

	// Tidy up auto rolling
	hfs_key_auto_roll_args_t auto_roll_args = {
		.api_version = HFS_KEY_AUTO_ROLL_API_LATEST_VERSION,
	};

	assert_no_err(fsctl("/private/var", HFSIOC_SET_KEY_AUTO_ROLL, &auto_roll_args, 0));

	// Now we should be able to start
	args.operation = HFS_KR_OP_START;
	assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));

	assert(args.done == 0 && (args.key_revision & 0xff00) == 0x0400);

	// Roll 1 chunk
	args.operation = HFS_KR_OP_STEP;
	assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));

	assert(args.done == 2 MB);

	// Truncate the file
	assert_no_err(ftruncate(fd, 1 MB));

	// Key rolling should have finished now
	args.operation = HFS_KR_OP_STATUS;
	assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));

	assert(args.done == -1);

	assert_no_err(ftruncate(fd, 3 MB));

	// Start rolling again
	args.operation = HFS_KR_OP_START;
	assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));

	assert(args.done == 0 && (args.key_revision & 0xff00) == 0x0500);

	// Roll 1 chunk
	args.operation = HFS_KR_OP_STEP;
	assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));

	assert(args.done == 2 MB);

	// Delete the file
	assert_no_err(unlink(KEY_ROLL_TEST_FILE));

	// File should be open unlinked now
	args.operation = HFS_KR_OP_STATUS;
	assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));

	assert(args.done == 2 MB);

	// Finish rolling
	args.operation = HFS_KR_OP_STEP;
	assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));

	assert(args.done == -1);

	assert_no_err(close(fd));

	// Check file
	assert(!memcmp(p, buf1, 1 MB));
	assert(!cmp_zero(p + 1 MB, 2 MB));

	assert_no_err(munmap(p, 3 MB));

	// -- Resource fork --

	assert_with_errno((fd = open(KEY_ROLL_TEST_FILE, 
								 O_CREAT | O_RDWR, 0666)) >= 0);

	for (int i = 0; i < 3; ++i) {
		check_io(write(fd, buf1, 1 MB), 1 MB);
	}

	for (int i = 0; i < 3; ++i) {
		assert_no_err(fsetxattr(fd, XATTR_RESOURCEFORK_NAME, buf1, 
								1 MB, i MB, 0));
	}

	args.operation = HFS_KR_OP_START;
	assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));

	assert(args.done == 0 && args.total == 6 MB);

	args.operation = HFS_KR_OP_STEP;
	assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));

	assert(args.done == 2 MB);

	args.operation = HFS_KR_OP_STEP;
	assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));

	assert(args.done == 3 MB);

	// Should have switched to resource fork

	args.operation = HFS_KR_OP_STEP;
	assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));

	assert(args.done == 5 MB);

	args.operation = HFS_KR_OP_STEP;
	assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));

	assert(args.done == -1);

	// Check the data

	for (int i = 0; i < 3; ++i) {
		check_io(fgetxattr(fd, XATTR_RESOURCEFORK_NAME, read_buf, 
						   1 MB, i MB, 0), 1 MB);

		assert(!memcmp(buf1, read_buf, 1 MB));
	}

	// Now try again, but this time truncate data fork

	args.operation = HFS_KR_OP_START;
	assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));

	assert(args.done == 0 && args.total == 6 MB);

	args.operation = HFS_KR_OP_STEP;
	assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));

	assert(args.done == 2 MB);

	assert_no_err(ftruncate(fd, 0));

	// Should have switched to resource fork

	args.operation = HFS_KR_OP_STATUS;
	assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));

	assert(args.done == 0);

	args.operation = HFS_KR_OP_STEP;
	assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));

	assert(args.done == 2 MB);

	// Check the data whilst we're in the middle of rolling

	for (int i = 0; i < 3; ++i) {
		check_io(fgetxattr(fd, XATTR_RESOURCEFORK_NAME, read_buf, 
						   1 MB, i MB, 0), 1 MB);

		assert(!memcmp(buf1, read_buf, 1 MB));
	}

	// Truncate the resource fork
	assert_no_err(fremovexattr(fd, XATTR_RESOURCEFORK_NAME, 0));

	// And that should have finished the roll
	args.operation = HFS_KR_OP_STATUS;
	assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));

	assert(args.done == -1);

	// Try and create a really fragmented file using F_PREALLOCATE

	// First make a 2 block extent

	off_t len = 2 * block_size();

	fstore_t fstore = {
		.fst_flags = F_ALLOCATECONTIG | F_ALLOCATEALL,
		.fst_posmode = F_PEOFPOSMODE,
		.fst_offset  = 0,
		.fst_length  = len,
	};

	assert_no_err(fcntl(fd, F_PREALLOCATE, &fstore));

	// Now allocate lots of single blocks
	fstore.fst_posmode = F_VOLPOSMODE;

	/*
	 * The maximum number of extents that the hfs_extents code
	 * can handle is 16384.
	 */
	for (int i = 0; i < 16384; ++i) {
		struct log2phys l2p = {
			.l2p_contigbytes = block_size(),
			.l2p_devoffset   = len - block_size(),
		};

		assert_no_err(fcntl(fd, F_LOG2PHYS_EXT, &l2p));

		len += block_size();

		// Force a gap
		fstore.fst_offset = l2p.l2p_devoffset + 2 * block_size();
		fstore.fst_length = len;

		assert_no_err(fcntl(fd, F_PREALLOCATE, &fstore));
	}

	assert_no_err(ftruncate(fd, len));

	// Now fill up the disk
	uint64_t size;

	fill_disk(&fd2, &size);

	// Now try and roll
	args.operation = HFS_KR_OP_START;
	assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));

	off_t start = 0, done = 0, decr = block_size();

	for (;;) {
		args.operation = HFS_KR_OP_STEP;
		if (!ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0)) {
			done += 2 MB;
			assert_equal_ll(args.done, done);
			break;
		}

		assert_with_errno(errno == ENOSPC);

		// It's possible we rolled a bit and then ran out of space
		args.operation = HFS_KR_OP_STATUS;
		assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));
		done = args.done;

		// If we've rolled over 2 MB in small bits, that's good enough
		if (done > start + 2 MB)
			break;

		// Shrink a bit
		size -= decr;
		assert_no_err(fcntl(fd2, F_SETSIZE, &size));

		/*
		 * It's possible to get in a state where other things on the
		 * system use up disk space as fast as we can free it.  To
		 * prevent this loop, decrement by a bit more next time.
		 */
		decr += block_size();
	}

	/*
	 * If unlink collides with the syncer, the file will be deleted on
	 * a different thread.  Truncating the file here makes the
	 * recovery of space synchronous.
	 */

	assert_no_err(ftruncate(fd2, 0));
	assert_no_err(close(fd2));
	assert_no_err(unlink(KEY_ROLL_FILL_DISK_FILE));

	// Finish rolling
	args.operation = HFS_KR_OP_STEP;
	while (args.done != -1)
		assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));

	// Start rolling again
	args.operation = HFS_KR_OP_START;
	assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));

	args.operation = HFS_KR_OP_STEP;
	assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));

	assert_equal_ll(args.done, 2 MB);

	/*
	 * That should have used a single extent and the rest of
	 * file should be reserved.
	 */
	struct log2phys l2p = {
		.l2p_contigbytes = 16 MB,
		.l2p_devoffset   = 0,
	};

	assert_no_err(fcntl(fd, F_LOG2PHYS_EXT, &l2p));

	/*
	 * The extent could have been split to minimise changes to the
	 * extent groups.  The first one should be a minimum of 2 MB
	 * less 7 blocks.  We only bother checking the first extent.
	 */
	if (l2p.l2p_contigbytes < 2 MB - 7 * block_size()) {
		assert_fail("extent smaller than expected: %llu\n",
					l2p.l2p_contigbytes);
	}

	// Try and allocate something just past it

	{
		fstore_t fstore = {
			.fst_flags = F_ALLOCATECONTIG | F_ALLOCATEALL,
			.fst_posmode = F_VOLPOSMODE,
			.fst_offset  = l2p.l2p_devoffset,
			.fst_length  = len + block_size(),
		};

		assert_no_err(fcntl(fd, F_PREALLOCATE, &fstore));
		assert(fstore.fst_bytesalloc == block_size());
	}

	// Force it to be initialised
	check_io(pwrite(fd, buf1, block_size(), len), block_size());

	// Now see where it was allocated
	struct log2phys l2p2 = {
		.l2p_contigbytes = 1 MB,
		.l2p_devoffset   = len,
	};

	assert_no_err(fcntl(fd, F_LOG2PHYS_EXT, &l2p2));

	assert(l2p2.l2p_contigbytes == block_size());

	// It shouldn't be anywhere in the reserved range
	if (l2p2.l2p_devoffset == -1
		|| (l2p2.l2p_devoffset + block_size() > l2p.l2p_devoffset
			&& l2p2.l2p_devoffset < l2p.l2p_devoffset + len)) {
		assert_fail("unexpected allocation: %llu (reserved: %llu-%llu)",
					l2p2.l2p_devoffset, l2p.l2p_devoffset, 
					l2p.l2p_devoffset + len - done);
	}

	// Revert extension
	assert_no_err(ftruncate(fd, len));

	args.operation = HFS_KR_OP_STATUS;
	assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));

	assert(args.done == 2 MB);

	// Fill up the disk so the tentative blocks get used up
	fill_disk(&fd2, &size);

	// Now try and roll another chunk
	start = done = 2 MB;
	decr = block_size();

	for (;;) {
		args.operation = HFS_KR_OP_STEP;
		if (!ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0)) {
			done += 2 MB;
			assert_equal_ll(args.done, done);
			break;
		}

		assert_with_errno(errno == ENOSPC);

		// It's possible we rolled a bit and then ran out of space
		args.operation = HFS_KR_OP_STATUS;
		assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));
		done = args.done;

		// If we've rolled over 2 MB in small bits, that's good enough
		if (done > start + 2 MB)
			break;

		// Drop by a bit
		size -= decr;
		assert_no_err(fcntl(fd2, F_SETSIZE, &size));

		decr += block_size();
	}

	assert_no_err(ftruncate(fd2, 0));
	assert_no_err(close(fd2));
	assert_no_err(unlink(KEY_ROLL_FILL_DISK_FILE));

	// Finish the roll
	args.operation = HFS_KR_OP_STEP;
	do {
		assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));
	} while (args.done != -1);

	// Start rolling again
	args.operation = HFS_KR_OP_START;
	assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));

	// And finish
	args.operation = HFS_KR_OP_STEP;
	do {
		assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));
	} while (args.done != -1);

	// That should have created a single extent
	l2p.l2p_contigbytes = UINT32_MAX;
	l2p.l2p_devoffset   = 0;

	assert_no_err(fcntl(fd, F_LOG2PHYS_EXT, &l2p));

	assert_equal_ll(l2p.l2p_contigbytes, len);

	assert_no_err(close(fd));

	// -- Appending to file whilst rolling --

	assert_with_errno((fd = open(KEY_ROLL_TEST_FILE, 
								 O_CREAT | O_RDWR | O_TRUNC, 0666)) >= 0);

	struct append_ctx actx = {
		.fd = fd,
	};

	pthread_t thread;
	assert_no_err(pthread_create(&thread, NULL, append_to_file, &actx));

	while (!actx.done) {
		args.operation = HFS_KR_OP_START;
		assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));

		do {
			args.operation = HFS_KR_OP_STEP;
			assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));
		} while (args.done != -1);
	}

	// Check file
	assert_with_errno((p = mmap(NULL, append_test_amount, PROT_READ,
								MAP_SHARED, fd, 0)) != MAP_FAILED);

	assert_no_err(msync(p, append_test_amount, MS_INVALIDATE));

	CC_MD5_CTX md5_ctx;
	CC_MD5_Init(&md5_ctx);

	CC_MD5_Update(&md5_ctx, p, append_test_amount);

	uint8_t digest[CC_MD5_DIGEST_LENGTH];
	CC_MD5_Final(digest, &md5_ctx);

	/* 
	 * Not really necessary, but making the point that we need a barrier
	 * between the check for done above and reading the digest.
	 */
	OSMemoryBarrier();

	assert(!memcmp(digest, actx.digest, CC_MD5_DIGEST_LENGTH));

	assert_no_err(munmap(p, append_test_amount));

	assert_no_err(close(fd));

	// -- Two threads rolling at the same time --

	assert_with_errno((fd = open(KEY_ROLL_TEST_FILE, 
								 O_RDWR, 0666)) >= 0);

	pthread_t threads[2];
	threads_running = 2;
	pthread_create(&threads[0], NULL, roll_thread, NULL);
	pthread_create(&threads[0], NULL, roll_thread, NULL);

	assert_with_errno((p = mmap(NULL, append_test_amount, PROT_READ,
								MAP_SHARED, fd, 0)) != MAP_FAILED);

	bool finished = false;
	do {
		if (!threads_running)
			finished = true;

		assert_no_err(msync(p, append_test_amount, MS_INVALIDATE));

		CC_MD5_CTX md5_ctx;
		CC_MD5_Init(&md5_ctx);

		CC_MD5_Update(&md5_ctx, p, append_test_amount);

		uint8_t digest[CC_MD5_DIGEST_LENGTH];
		CC_MD5_Final(digest, &md5_ctx);

		assert(!memcmp(digest, actx.digest, CC_MD5_DIGEST_LENGTH));
	} while (!finished);

	pthread_join(threads[0], NULL);
	pthread_join(threads[1], NULL);

	// -- Class F files --

	assert_with_errno((fd = open(KEY_ROLL_TEST_FILE, O_RDWR, 0666)) >= 0);

	// Finish off this file
	args.operation = HFS_KR_OP_STEP;

	do {
		assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));
	} while (args.done != -1);

	// Should fail because file has data
	assert(fcntl(fd, F_SETPROTECTIONCLASS, 6) == -1 && errno == EINVAL);

	// Start key rolling
	args.operation = HFS_KR_OP_START;
	assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));
	assert(args.done == 0);

	// Truncate file
	assert_no_err(ftruncate(fd, 0));

	// Check status
	args.operation = HFS_KR_OP_STATUS;
	assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));

	// We truncated the file so it rolling should have been aborted
	assert(args.done == -1);

	// Now setting to class F should succeed
	assert_no_err(fcntl(fd, F_SETPROTECTIONCLASS, 6));

	// Attempts to roll should fail
	args.operation = HFS_KR_OP_START;
	assert(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0) == -1 && errno == ENOTSUP);

	assert_no_err(close(fd));

	// -- Rolling non-files --

	// Check class inheritance
	assert_no_err(mkdir("/tmp/key-roll-test.dir", 0777));

	// Should be class D
	assert_no_err(getattrlist("/tmp/key-roll-test.dir", &attrlist, &attrs, 
							  sizeof(attrs), 0));

	// Dir should be class D
	assert((attrs.dp_flags & 0x1f) == 4);

	// Create file
	assert_with_errno((fd = open("/tmp/key-roll-test.dir/file1", 
								 O_RDWR | O_TRUNC | O_CREAT, 0666)) >= 0);

	// Make sure it's class D
	assert_no_err(fgetattrlist(fd, &attrlist, &attrs, sizeof(attrs), 0));
	assert((attrs.dp_flags & 0x1f) == 4);

	assert_with_errno((fd = open("/tmp/key-roll-test.dir", O_RDONLY)) >= 0);

	// Change directory to class C
	assert_no_err(fcntl(fd, F_SETPROTECTIONCLASS, 3));

	assert_no_err(close(fd));

	// Create another file and make sure it's class C
	assert_with_errno((fd = open("/tmp/key-roll-test.dir/file2", 
								 O_RDWR | O_TRUNC | O_CREAT, 0666)) >= 0);

	assert_no_err(fgetattrlist(fd, &attrlist, &attrs, sizeof(attrs), 0));
	assert((attrs.dp_flags & 0x1f) == 3);

	assert_no_err(close(fd));

	// Try and roll a directory--it should fail
	args.operation = HFS_KR_OP_START;
	assert(fsctl("/tmp/key-roll-test.dir", HFSIOC_KEY_ROLL, &args, 0) == -1
		   && errno == ENOTSUP);

	args.operation = HFS_KR_OP_STATUS;
	assert_no_err(fsctl("/tmp/key-roll-test.dir", HFSIOC_KEY_ROLL, &args, 0));

	assert(args.done == -1 && args.total == 0);

	unlink(KEY_ROLL_SYM_LINK);
	assert_no_err(symlink("/tmp/key-roll-test.dir/file2",
						  KEY_ROLL_SYM_LINK));

	assert_with_errno((fd = open(KEY_ROLL_SYM_LINK,
								 O_RDONLY | O_SYMLINK)) >= 0);

	args.operation = HFS_KR_OP_START;
	assert(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0) == -1 && errno == ENOTSUP);

	args.operation = HFS_KR_OP_STATUS;
	assert_no_err(ffsctl(fd, HFSIOC_KEY_ROLL, &args, 0));
	assert(args.done == -1 && args.total == 0);

	assert_no_err(close(fd));

	// Tidy up
	unlink(KEY_ROLL_TEST_FILE);
	unlink(KEY_ROLL_SYM_LINK);
	systemx("/bin/rm", "-rf", KEY_ROLL_TEST_DIR, NULL);

	return 0;
}

#endif // TARGET_OS_IPHONE & !SIM
