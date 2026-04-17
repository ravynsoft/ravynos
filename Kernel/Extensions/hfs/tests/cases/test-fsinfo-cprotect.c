/*
 * Copyright (c) 2014 Apple, Inc. All rights reserved.
 *
 * Test HFS fsinfo fsctls 
 */

#include <TargetConditionals.h>

#if (TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR)

#include <sys/ioctl.h>
#include <sys/ioccom.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <spawn.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <hfs/hfs_fsctl.h>

#include "hfs-tests.h"
#include "test-utils.h"
#include "systemx.h"

TEST(fsinfo_cprotect, .run_as_root = true)

#define KB				*1024
#define MAX_FILES		10
#define TEST_DIR		"/tmp"

static hfs_fsinfo	fsinfo;

static struct attrs {
	uint32_t len;
	uint64_t file_id;
	uint32_t dp_flags;
} __attribute__((aligned(4), packed)) attrs;

static void test_fsinfo_file_cprotect_count(void)
{
	int				fd;
	unsigned		buf_size;
	void			*buf = malloc(1 KB);
	unsigned		class_B_count = 0;
	unsigned		i;
	char			*path[10];

	bzero(&fsinfo, sizeof(fsinfo));
	fsinfo.header.version = HFS_FSINFO_VERSION;
	fsinfo.header.request_type = HFS_FSINFO_FILE_CPROTECT_COUNT;
	assert_no_err(fsctl(TEST_DIR, HFSIOC_GET_FSINFO, &fsinfo, 0));
	class_B_count = fsinfo.cprotect.class_B;

	for (i = 0; i < 10; i++)
	{
		asprintf(&path[i], "%s/fsinfo_test.data.%u", TEST_DIR, getpid());
		unlink(path[i]);
		assert_with_errno((fd = open(path[i], O_RDWR | O_TRUNC | O_CREAT, 0666)) >= 0);
		buf_size = (1 KB);
		buf = malloc(buf_size);
		memset(buf, 0x83, buf_size);

		assert_no_err(fcntl(fd, F_SETPROTECTIONCLASS, 2));

		struct attrlist attrlist = {
			.bitmapcount = ATTR_BIT_MAP_COUNT,
			.commonattr = ATTR_CMN_FILEID | ATTR_CMN_DATA_PROTECT_FLAGS,
		};

		assert_no_err(fgetattrlist(fd, &attrlist, &attrs, sizeof(attrs), 0));
		assert((attrs.dp_flags & 0x1f) == 2);
	}

	assert_no_err(fsctl(TEST_DIR, HFSIOC_GET_FSINFO, &fsinfo, 0));

	free(buf);
	for (i = 0; i < 10; i++)
		unlink(path[i]);

	assert(fsinfo.cprotect.class_B >= (class_B_count + 5));

}

int run_fsinfo_cprotect(__unused test_ctx_t *ctx)
{
	// The root file system needs to be HFS
	struct statfs sfs;
	
	assert(statfs("/tmp", &sfs) == 0);
	if (strcmp(sfs.f_fstypename, "hfs")) {
		printf("fsinfo_cprotect needs hfs as root file system - skipping.\n");
		return 0;
	}
	
	test_fsinfo_file_cprotect_count();
	
	return 0;
}

#endif // TARGET_OS_IPHONE & !SIM
