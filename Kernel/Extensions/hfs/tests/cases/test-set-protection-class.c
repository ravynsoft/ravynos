// See <rdar://16977080>

#include <unistd.h>
#include <sys/fcntl.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/mman.h>
#include <sys/xattr.h>
#include <sys/mount.h>
#include <sys/param.h>
#include <TargetConditionals.h>

#include "hfs-tests.h"
#include "test-utils.h"
#include "disk-image.h"

TEST(set_protection_class)

static char *path;
static int pass;

int run_set_protection_class(__unused test_ctx_t *ctx)
{
	const char *tstdir;
	
#if (TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR)
	// The root file system needs to be HFS
	struct statfs sfs;
	
	assert(statfs("/tmp", &sfs) == 0);
	if (strcmp(sfs.f_fstypename, "hfs")) {
		printf("set_protection_class needs hfs as root file system - skipping.\n");
		return 0;
	}
	
	tstdir = "/tmp";
#else // !(TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR)
	disk_image_t *di = disk_image_get();
	tstdir = di->mount_point;
#endif
	
	asprintf(&path, "%s/set-protection-class.data.%u", tstdir, getpid());

	const size_t size = 16 * 1024 * 1024;

	void *buf = valloc(size), *buf2 = valloc(size);
	memset(buf, 0x1f, size);

	/*
	 * Pass 0: Write files using write and then call
	 *         F_SETPROTECTIONCLASS while the file is still referenced
	 *         i.e. file won't go through inactive.
	 *
	 * Pass 1: Like pass 0 but file should go through inactive.
	 * 
	 * Pass 2: Like pass 0 but using resource fork to reference file.
	 *
	 * Pass 3: Like pass 0 but use mmap to write the data.
	 *
	 * Pass 4: Like pass 3 but we close the file after mmap which will
	 *         mean the file goes through inactive when we do the
	 *         munmap.
	 *
	 */

	for (pass = 0; pass < 5; ++pass) {
		unlink(path);
		int fd;
		assert_with_errno((fd = open(path, O_RDWR | O_TRUNC | O_CREAT, 0666)) >= 0);

		switch (pass) {
		default:
			{
				size_t done = 0;
				while (done < size) {
					size_t todo = random() % 1024 + 1;
					if (todo > size - done)
						todo = size - done;

					check_io(write(fd, buf + done, todo), todo);

					done += todo;
				}
			}
			break;
		case 3: 
		case 4:
			{
				void *p;

				assert_no_err(ftruncate(fd, size));
				assert_with_errno((p = mmap(NULL, size, PROT_READ | PROT_WRITE, 
											MAP_SHARED, fd, 0)) != MAP_FAILED);

				if (pass == 4)
					assert_no_err(close(fd));

				size_t done = 0;
				while (done < size) {
					size_t todo = random() % 1024 + 1;
					if (todo > size - done)
						todo = size - done;

					memcpy(p + done, buf + done, todo);

					done += todo;
				}

				assert_no_err(msync(p, size, MS_ASYNC));

				assert_no_err(munmap(p, size));
			}
			break;
		}

		int fd2 = -1;
		switch (pass) {
		default:
			assert_with_errno((fd2 = open(path, O_RDONLY)) >= 0);
			break;
		case 1:
			break;
		case 2: 
			{
				// Force the rsrc fork vnode to be created
				static const char val[] = "set-protection-class-test";
				assert_with_errno(!setxattr(path, XATTR_RESOURCEFORK_NAME, val, 
											sizeof(val) - 1, 0, 0));
				break;
			}
		}

		if (pass != 4)
			assert_no_err(close(fd));

		assert_with_errno((fd = open(path, O_RDWR)) >= 0);

#if (TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR)
		assert_no_err(fcntl(fd, F_SETPROTECTIONCLASS, 2));
#endif

		void *p;

		assert_with_errno((p = mmap(NULL, size, PROT_WRITE, MAP_SHARED, 
									fd, 0)) != MAP_FAILED);

		assert_no_err(msync(p, size, MS_INVALIDATE));

		bzero(buf2, size);
		check_io(pread(fd, buf2, size, 0), size);

		assert(!memcmp(buf2, buf, size));

		assert_no_err(close(fd));
		if (fd2 != -1)
			assert_no_err(close(fd2));

		assert_no_err(munmap(p, size));
	}

	unlink(path);
	free(buf);
	free(buf2);

	return 0;
}
