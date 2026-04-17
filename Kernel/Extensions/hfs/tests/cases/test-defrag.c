/*
 * Copyright (c) 2016 Apple, Inc. All rights reserved.
 *
 * Test HFS defrag fsctls 
 */

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

#include "../../core/hfs_fsctl.h"
#include "hfs-tests.h"
#include "test-utils.h"
#include "systemx.h"
#include "disk-image.h"

TEST(defrag, .run_as_root = true)

int run_defrag(__unused test_ctx_t *ctx)
{
	
	// The root file system needs to be HFS
	struct statfs sfs;
	
	assert(statfs("/tmp", &sfs) == 0);
	if (strcmp(sfs.f_fstypename, "hfs")) {
		printf("test-defrag needs hfs as root file system - skipping.\n");
		return 0;
	}

	/* These two should pass */
	uint32_t enable_defrag = 1;
	assert_no_err(fsctl("/tmp", HFSIOC_FORCE_ENABLE_DEFRAG, &enable_defrag, 0));

	uint32_t max_file_size = 50 * 1024 * 1024;
	assert_no_err(fsctl("/tmp", HFSIOC_SET_MAX_DEFRAG_SIZE, &max_file_size, 0));

	/* This should fail */
	max_file_size = 500 * 1024 * 1024;
	int err = fsctl("/tmp", HFSIOC_SET_MAX_DEFRAG_SIZE, &max_file_size, 0);
	if (err == 0){
		abort();	
	}
	
	return 0;
}
