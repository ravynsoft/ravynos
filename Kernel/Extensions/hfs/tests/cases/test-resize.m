#include <TargetConditionals.h>

#if !(TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR)

#include <unistd.h>
#include <sys/fcntl.h>
#include <spawn.h>

#include <Foundation/Foundation.h>

#include "hfs-tests.h"
#include "test-utils.h"
#include "systemx.h"
#include "disk-image.h"

TEST(resize)

static disk_image_t *di;

#define DISK_IMAGE "/tmp/hfs_resize.sparseimage"

int run_resize(__unused test_ctx_t *ctx)
{
	di = disk_image_create(DISK_IMAGE, &(disk_image_opts_t){
											.size = 100 * 1024 * 1024
										});
	
	int fd;

	// Create two fragmented files
	for (int i = 0; i < 2; ++i) {
		char *path;
		asprintf(&path, "%s/fragmented-file.%d", di->mount_point, i);

		fd = open(path, O_RDWR | O_TRUNC | O_CREAT, 0666);
		assert_with_errno(fd >= 0);

		/*
		 * First file we want near the end of the volume.  Second file
		 * at the beginning.
		 */
		fstore_t fstore = {
			.fst_flags = F_ALLOCATECONTIG | F_ALLOCATEALL,
			.fst_posmode = F_VOLPOSMODE,
			.fst_offset = i == 0 ? 80 * 1024 * 1024 : 4096,
		};

		off_t len = 0;

		for (int j = 0; j < 64; ++j) {
			if (len) {
				struct log2phys l2p = {
					.l2p_contigbytes = 4096,
					.l2p_devoffset   = len - 4096,
				};

				assert_no_err(fcntl(fd, F_LOG2PHYS_EXT, &l2p));

				fstore.fst_offset = l2p.l2p_devoffset + 16384;
			}

			len += 4096;

			fstore.fst_length = len;

			assert_no_err(fcntl(fd, F_PREALLOCATE, &fstore));
		}

		assert_no_err(ftruncate(fd, len));

		assert_no_err(close(fd));
	}

	assert(!systemx("/usr/sbin/diskutil", SYSTEMX_QUIET, "resizeVolume", di->mount_point, "40m", NULL));

	return 0;
}

#endif // !(TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR)
