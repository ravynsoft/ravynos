/*
 * Copyright (c) 2015 Apple, Inc. All rights reserved.
 *
 * <rdar://problem/20565191>  hfs_scan_range_size return 0 iosize unexpectedly.
 */
 
#include <TargetConditionals.h>

#if !TARGET_OS_IPHONE
 
#include <sys/ioctl.h>
#include <sys/ioccom.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <spawn.h>

#include "../../core/hfs_fsctl.h"

#include "hfs-tests.h"
#include "test-utils.h"
#include "systemx.h"
#include "disk-image.h"

TEST(scan_range_size, .run_as_root = true)

static disk_image_t *di;

static hfs_fsinfo	fsinfo;

static void test_fsinfo_file_extent_size(void)
{
	bzero(&fsinfo, sizeof(fsinfo));
	fsinfo.header.request_type = HFS_FSINFO_FILE_EXTENT_SIZE;
	fsinfo.header.version = HFS_FSINFO_VERSION;
	assert_no_err(fsctl(di->mount_point, HFSIOC_GET_FSINFO, &fsinfo, 0));
}

static void test_fsinfo_free_extents(void)
{
	bzero(&fsinfo, sizeof(fsinfo));
	fsinfo.header.version = HFS_FSINFO_VERSION;
	fsinfo.header.request_type = HFS_FSINFO_FREE_EXTENTS;
	assert_no_err(fsctl(di->mount_point, HFSIOC_GET_FSINFO, &fsinfo, 0));
}


int run_scan_range_size(__unused test_ctx_t *ctx) {

	di = disk_image_get();

	test_fsinfo_file_extent_size();
	test_fsinfo_free_extents();

	return 0;
}

#endif // !TARGET_OS_IPHONE
