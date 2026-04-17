#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/param.h>
#include <spawn.h>
#include <signal.h>
#include <sys/stat.h>
#include <TargetConditionals.h>

#import <Foundation/Foundation.h>

#include "hfs-tests.h"
#include "test-utils.h"
#include "disk-image.h"
#include "systemx.h"

TEST(invalid_ranges)

static disk_image_t *di;

#define DISK_IMAGE "/tmp/invalid-ranges.sparseimage"

int run_invalid_ranges(__unused test_ctx_t *ctx)
{
	di = disk_image_get();
	
	char *file;
	asprintf(&file, "%s/invalid-ranges.data", di->mount_point);
	
	unlink(file);

	int fd = open(file, O_CREAT | O_RDWR, 0666);

	assert_with_errno(fd >= 0);

	off_t size = 1000000;

	// Make a big file and punch holes in it
	ftruncate(fd, size);

	void *buf = valloc(65536);

	memset(buf, 0xaf, 65536);

	assert_no_err(fcntl(fd, F_NOCACHE, 1));

	off_t o;
	for (o = 0; o < size; o += 131072)
		check_io(pwrite(fd, buf, 65536, o + 65536), 65536);

	// This should cause everything to be flushed
	assert_no_err(close(fd));

	assert_with_errno((fd = open(file, O_RDWR)) >= 0);

	uint8_t *p;
	assert_with_errno((p = mmap(NULL, o, PROT_READ | PROT_WRITE, MAP_SHARED,
								fd, 0)) != MAP_FAILED);

	assert_no_err(msync(p, o, MS_INVALIDATE));

	void *zero = malloc(65536);
	bzero(zero, 65536);

	off_t n;
	uint8_t *q;

	for (n = 0, q = p; n < o; n += 131072, q += 131072) {
		assert(!memcmp(q, zero, 65536));
		assert(!memcmp(q + 65536, buf, 65536));
	}

	assert(p[size] == 0xaf);

	assert_no_err(ftruncate(fd, size));

	// Check the tail portion of the page is zero
	assert(p[size] == 0);

	p[size] = 0xbe;

	msync(p + size - 1, 1, MS_SYNC);

	int ps = getpagesize();
	int ps_mask = ps - 1;

	// Force the page out
	assert_no_err(msync((void *)((uintptr_t)(p + size - 1) & ~ps_mask),
						ps, MS_INVALIDATE));

	// Page back in and check it's zeroed
	assert(p[size] == 0);

	p[size] = 0x75;

	// Extend the file to include the change we made above
	assert_no_err(ftruncate(fd, size + 1));

	// That change should have been zeroed out
	assert(p[size] == 0);

	assert_no_err(munmap(p, o));

	// Extend the file
	assert_no_err(ftruncate(fd, o + 2 * ps));

	// Write something into the middle of the page following o
	off_t hello_offset = roundup(o, ps) + 100;

	check_io(pwrite(fd, "hello", 5, hello_offset), 5);

	// Close and re-read
	assert_no_err(close(fd));

	assert_with_errno((fd = open(file, O_RDWR)) >= 0);

	assert_with_errno((p = mmap(NULL, o + 2 * ps, PROT_READ | PROT_WRITE, MAP_SHARED,
								fd, 0)) != MAP_FAILED);

	assert_no_err(msync(p, o + 2 * ps, MS_INVALIDATE));

	assert(!memcmp(p + hello_offset, "hello", 5));
	assert(!memcmp(p + size, zero, hello_offset - size));
	assert(!memcmp(p + hello_offset + 5, zero, o + ps * 2 - hello_offset - 5));

	assert_no_err(close(fd));
	assert_no_err(unlink(file));

	// Make a large number of invalid ranges
	assert_with_errno((fd = open(file,
								 O_RDWR | O_CREAT, 0666)) >= 0);
	for (int i = 0; i < 1024; ++i) {
		pwrite(fd, "hello", 5, i * ps * 2);
	}

	assert_no_err(munmap(p, o + 2 * ps));

	// OK, that should have created 1024 invalid ranges.  Sync the data.
	p = mmap(NULL, 1024 * ps * 2, PROT_READ | PROT_WRITE, MAP_SHARED,
			 fd, 0);
	assert(p != MAP_FAILED);

	assert_no_err(msync(p, 1024 * ps * 2, MS_SYNC));

	// Now sync the invalid ranges
	assert_no_err(fcntl(fd, F_FULLFSYNC));

	assert_no_err(close(fd));

	assert_no_err(unlink(file));

	assert_no_err(munmap(p, 1024 * ps * 2));

#if !(TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR)
	disk_image_t *di2 = disk_image_create(DISK_IMAGE, &(disk_image_opts_t){
															.size = 100 * 1024 * 1024
														});
	
	// Find the diskimages_helper process
	char *dev_device = strdup(di2->disk + 5);

	// Strip off the s bit for the partition
	char *spos = strrchr(dev_device, 's');
	assert(spos);
	*spos = 0;

	io_service_t obj = IOServiceGetMatchingService(kIOMasterPortDefault,
												   IOBSDNameMatching(kIOMasterPortDefault, 0, dev_device));

    assert(obj);

	io_service_t parent;

	// obj should be the IOMedia object.  Go up three to the IOHDIXHDDrive object.
	assert(!IORegistryEntryGetParentEntry(obj, kIOServicePlane, &parent));

	assert(parent);

	IOObjectRelease(obj);

	assert(!IORegistryEntryGetParentEntry(parent, kIOServicePlane, &obj));

	IOObjectRelease(parent);

	assert(!IORegistryEntryGetParentEntry(obj, kIOServicePlane, &parent));

	assert(parent);

	IOObjectRelease(obj);

	NSString *creator = (id)CFBridgingRelease(IORegistryEntrySearchCFProperty(parent, kIOServicePlane,
															CFSTR("IOUserClientCreator"),
															kCFAllocatorDefault,
															kIORegistryIterateRecursively));

	IOObjectRelease(parent);

	assert(creator);

	// Extract the pid of disk_images_helper
	pid_t disk_images_helper_pid;
	assert(sscanf([creator UTF8String], "pid %u", &disk_images_helper_pid) == 1);

	// Create a file

	char *path;
	asprintf(&path, "%s/test-file", di2->mount_point);

	fd = open(path, O_CREAT | O_RDWR, 0666);

	/*
	 * Workaround for <rdar://20688964>: force the journal to
	 * have at least one transaction in it.
	 */
	assert_no_err(fcntl(fd, F_FULLFSYNC));

	assert(fd >= 0);

	assert_no_err(fcntl(fd, F_NOCACHE, 1));

	int block_size = 65536;

	// Preallocate
	struct fstore fst = {
		.fst_posmode = F_PEOFPOSMODE,
		.fst_length = 2 * block_size,
	};

	assert_no_err(fcntl(fd, F_PREALLOCATE, &fst));

	assert(fst.fst_bytesalloc >= 2 * block_size);

	// Figure out where that is on the device
	struct log2phys l2p = { .l2p_contigbytes = block_size, .l2p_devoffset = block_size };
	assert_no_err(fcntl(fd, F_LOG2PHYS_EXT, &l2p));

	assert(l2p.l2p_contigbytes > 0);

	// Now open the raw device and write some garbage to that location

	assert(!strncmp(di2->disk, "/dev/", 5));

	char *raw_path;
	asprintf(&raw_path, "/dev/r%s", di2->disk + 5);

	int raw_dev = open(raw_path, O_RDWR);

	assert_with_errno(raw_dev >= 0);

	memset(buf, 0x57, block_size);

	check_io(pwrite(raw_dev, buf, l2p.l2p_contigbytes, l2p.l2p_devoffset),
			 l2p.l2p_contigbytes);

	assert_no_err(close(raw_dev));

	// OK, so now we have some garbage where we want it.

	// Check fcntl F_LOG2PHYS_EXT is doing what we expect
	off_t file_offset = block_size;
	do {
		assert(l2p.l2p_contigbytes > 0);
		assert(l2p.l2p_contigbytes < 1024 * 1024);
		file_offset += l2p.l2p_contigbytes;
		assert(file_offset < 1024 * 1024);
		l2p.l2p_devoffset = file_offset;
		l2p.l2p_contigbytes = INT64_MAX;
	} while (!fcntl(fd, F_LOG2PHYS_EXT, &l2p));

	assert_with_errno(errno == ERANGE);

	// Do some writing to the file normally

	memset(buf, 0xaa, block_size);

	check_io(pwrite(fd, buf, block_size, 0), block_size);

	check_io(pwrite(fd, buf, block_size, block_size * 2), block_size);

	// So now we have a hole that should be zeroed at <block_size, block_size>

	// Touch every page in that hole
	for (int i = 0; i < block_size / 4096; ++i)
		check_io(pwrite(fd, "hello", 5, block_size + i * 4096 + 1000), 5);

	// Check what we have in the cache
	check_io(pread(fd, buf, ps, block_size), ps);

	assert(!memcmp(buf, zero, 1000));
	assert(!memcmp(buf + 1000, "hello", 5));
	assert(!memcmp(buf + 1005, zero, ps - 1005));

	/* Write something into the block beyond the hole.  This should
	   cause a metadata update. */
	check_io(pwrite(fd, "hello", 5, block_size * 3), 5);

	/* Create another file so we can do a full fsync to
	   force a journal flush. */
	char *fsync_path;
	asprintf(&fsync_path, "%s/fsync", di->mount_point);

	assert_no_err(close(fd));
	fd = open(fsync_path, O_CREAT | O_RDWR | O_TRUNC, 0666);

	assert_with_errno(fd >= 0);

	assert_no_err(fcntl(fd, F_FULLFSYNC));

	// Kill disk_images_helper to simulate a crash
	assert_no_err(kill(disk_images_helper_pid, SIGKILL));

	// Wait until it gets unmounted
	struct stat sb;
	while (!stat(raw_path, &sb))
		sleep(1);

	// Wait another second for things to settle down
	sleep(1);

	// Attach to the disk image again
	assert(!systemx("/usr/bin/hdiutil", SYSTEMX_QUIET, "attach", DISK_IMAGE, NULL));

	assert_no_err(close(fd));
	assert_with_errno((fd = open(path, O_RDWR)) >= 0);

	// Either the file should be short, or there should be zeroes
	ssize_t amount = pread(fd, buf, block_size, block_size);
	assert_with_errno(amount >= 0);

	assert(!memcmp(buf, zero, (amount > 1000) ? 1000 : amount));

	assert_no_err(close(fd));

#endif //!(TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR)

	// Test for <rdar://20994239>
	fd = open(file, O_CREAT | O_RDWR, 0666);
	assert_with_errno(fd >= 0);
	assert_no_err(fcntl(fd, F_NOCACHE, 1));

	void *buf2 = malloc(0x100000);
	memset(buf2, 0x16, 0x100000);
	check_io(pwrite(fd, buf2, 0x200,    0),          0x200);
	check_io(pwrite(fd, buf2, 0x100000, 0x00100200), 0x100000);
	check_io(pwrite(fd, buf2, 0x100000, 0x00300200), 0x100000);
	check_io(pwrite(fd, buf2, 0x100000, 0x00500200), 0x100000);
	check_io(pwrite(fd, buf2, 0x200,    0),          0x200);
	check_io(pwrite(fd, buf2, 0x100000, 0x00700200), 0x100000);
	check_io(pwrite(fd, buf2, 0x100000, 0x200),      0x100000);

	void *buf3 = malloc(0x100000);
	check_io(pread(fd, buf3, 0x100000, 0x100000), 0x100000);
	assert(!memcmp(buf2, buf3, 0x100000));

	free(buf3);
	free(buf2);
	free(buf);
	free(zero);
	assert_no_err(close(fd));
	assert_no_err(unlink(file));

	return 0;
}
