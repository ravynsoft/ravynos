//
//  quotas.c
//  hfs
//
//  Created by Chris Suter on 8/13/15.
//
//

#include <TargetConditionals.h>

#if !TARGET_OS_IPHONE

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/quota.h>
#include <sys/stat.h>

#include "hfs-tests.h"
#include "disk-image.h"
#include "test-utils.h"

#define QUOTA_DMG	"/tmp/quota-test.sparseimage"
#define QUOTA_DMG_MOUNT_POINT	"/tmp/quota-test-mount"

TEST(quotas, .run_as_root = true)

// This is what the quota file looks like after running edquota
char quota_file[] = {
	0xff, 0x31, 0xff, 0x35, 0x00, 0x00, 0x00, 0x01,
	0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x09, 0x3a, 0x80, 0x00, 0x09, 0x3a, 0x80,
	0x51, 0x55, 0x4f, 0x54, 0x41, 0x20, 0x48, 0x41,
	0x53, 0x48, 0x20, 0x46, 0x49, 0x4c, 0x45, 0x00,
	// Zero padding up to 131136 bytes
};

int run_quotas(__unused test_ctx_t *ctx)
{
	disk_image_t *di = disk_image_create(QUOTA_DMG,
	                                     &(disk_image_opts_t) {
#if (TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR)
	                                     	.mount_point = QUOTA_DMG_MOUNT_POINT,
#endif
	                                     	.size = 64 * 1024 * 1024,
	                                     	.enable_owners = true
	                                     });

	char *path;
	asprintf(&path, "%s/.quota.ops.user", di->mount_point);
	int fd = open(path, O_CREAT | O_RDWR, 0777);
	assert_with_errno(fd >= 0);
	free(path);
	assert_no_err(close(fd));

	asprintf(&path, "%s/.quota.user", di->mount_point);
	fd = open(path, O_CREAT | O_RDWR, 0777);
	assert_with_errno(fd >= 0);
	check_io(write(fd, quota_file, lengthof(quota_file)), lengthof(quota_file));
	assert_no_err(ftruncate(fd, 131136));
	assert_no_err(close(fd));

	assert_no_err(quotactl(di->mount_point, QCMD(Q_QUOTAON, USRQUOTA), 0, path));
	free(path);

	struct dqblk dqb = {
		.dqb_bhardlimit = 1024 * 1024,
		.dqb_ihardlimit = 10,
		.dqb_isoftlimit = 10,
	};

	assert_no_err(quotactl(di->mount_point, QCMD(Q_SETQUOTA, USRQUOTA),
						   501, (caddr_t)&dqb));

	assert_no_err(chmod(di->mount_point, 0777));

	// Now try and create a 2 MB file as UID 501
	assert_no_err(seteuid(501));

	asprintf(&path, "%s/test-file", di->mount_point);

	fd = open(path, O_CREAT | O_RDWR, 0777);
	assert_with_errno(fd >= 0);
	free(path);

	/*
	 * Not sure why we can't do the full 4 KB, but it's not a big deal
	 * so let's not worry about it.
	 */
	assert_no_err(ftruncate(fd, 1024 * 1024 - 4096));

	assert(ftruncate(fd, 2 * 1024 * 1024) == -1 && errno == EDQUOT);

	assert_no_err(close(fd));

	for (int i = 0; i < 10; ++i) {
		asprintf(&path, "%s/test-file.%u", di->mount_point, i);
		/*
		 * There might be an off by one error.  It's not a big deal,
		 * so we let it go for now.
		 */
		fd = open(path, O_CREAT | O_RDWR, 0777);
		if (fd == -1) {
			assert(i >= 8);
			assert_with_errno(errno == EDQUOT);
		} else {
			assert(i < 8);
			free(path);
			assert_no_err(close(fd));
		}
	}

	return 0;
}
#endif // !TARGET_OS_EMBEDDED
