/*
 * Copyright (c) 2018 Apple Inc. All rights reserved.
 *
 * <rdar://problem/3916036>  fsctl HFSIOC_GET_VOL_CREATE_TIME does not return volume cteate time.
 */
#include <unistd.h>

#include "../../core/hfs_fsctl.h"
#include "hfs-tests.h"
#include "test-utils.h"
#include "disk-image.h"

/*
 * Just as a good measure we add this check so that compilation does
 * not break when compiled against older hfs_fsctl.h which did not
 * include HFSIOC_GET_VOL_CREATE_TIME.
 */
#if !defined(HFSIOC_GET_VOL_CREATE_TIME)
#define HFSIOC_GET_VOL_CREATE_TIME _IOR('h', 4, time_t)
#endif

TEST(get_volume_create_time)

int run_get_volume_create_time(__unused test_ctx_t *ctx)
{
	disk_image_t *di;
	time_t vol_create_time;

	di = disk_image_get();
	/*
	 * Volume create date is stored inside volume header in localtime.  The
	 * date is stored as 32-bit integer containing the number of seconds
	 * since midnight, January 1, 1904. We can safely assume that create
	 * date set for the volume will not be epoch.
	 */
	vol_create_time = 0;
	assert_no_err(fsctl(di->mount_point, HFSIOC_GET_VOL_CREATE_TIME,
		&vol_create_time, 0));
	if (!vol_create_time)
		assert_fail("fcntl HFSIOC_GET_VOL_CREATE_TIME failed to set "
			"volume create time.");
	return 0;
}
