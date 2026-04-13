#include <fcntl.h>
#include <sys/attr.h>
#include <unistd.h>
#include <sys/stat.h>

#include "hfs-tests.h"
#include "test-utils.h"
#include "../core/hfs_fsctl.h"
#include "disk-image.h"

TEST(disklevel)

int run_disklevel(__unused test_ctx_t *ctx)
{
	disk_image_t *di = disk_image_get();
	const char *test_hfs_volume = di->mount_point;
	uint32_t very_low_disk = 0, low_disk = 0, near_low_disk = 0, desired_disk = 0;

	// Make sure initial values are sane.
	assert_no_err(fsctl(test_hfs_volume, HFSIOC_GET_VERY_LOW_DISK, &very_low_disk, 0));
	assert_no_err(fsctl(test_hfs_volume, HFSIOC_GET_LOW_DISK, &low_disk, 0));
	assert_no_err(fsctl(test_hfs_volume, APFSIOC_GET_NEAR_LOW_DISK, &near_low_disk, 0));
	assert_no_err(fsctl(test_hfs_volume, HFSIOC_GET_DESIRED_DISK, &desired_disk, 0));
	assert(very_low_disk > 0);
	assert(very_low_disk < low_disk);
	assert(low_disk < near_low_disk);
	assert(near_low_disk < desired_disk);

	very_low_disk = 1;
	low_disk = 2;
	near_low_disk = 3;
	desired_disk = 4;
	// Re-assign the values to new legal values and make sure they are preserved.
	assert_no_err(fsctl(test_hfs_volume, HFSIOC_SET_VERY_LOW_DISK, &very_low_disk, 0));
	assert_no_err(fsctl(test_hfs_volume, HFSIOC_SET_LOW_DISK, &low_disk, 0));
	assert_no_err(fsctl(test_hfs_volume, APFSIOC_SET_NEAR_LOW_DISK, &near_low_disk, 0));
	assert_no_err(fsctl(test_hfs_volume, HFSIOC_SET_DESIRED_DISK, &desired_disk, 0));

	assert_no_err(fsctl(test_hfs_volume, HFSIOC_GET_VERY_LOW_DISK, &very_low_disk, 0));
	assert_no_err(fsctl(test_hfs_volume, HFSIOC_GET_LOW_DISK, &low_disk, 0));
	assert_no_err(fsctl(test_hfs_volume, APFSIOC_GET_NEAR_LOW_DISK, &near_low_disk, 0));
	assert_no_err(fsctl(test_hfs_volume, HFSIOC_GET_DESIRED_DISK, &desired_disk, 0));
	assert_equal(very_low_disk, 1, "%d");
	assert_equal(low_disk, 2, "%d");
	assert_equal(near_low_disk, 3, "%d");
	assert_equal(desired_disk, 4, "%d");

	// Now, attempt to reassign the levels to illegal values and make sure they don't lose their previous value.
	very_low_disk = 4;
	low_disk = 1;
	near_low_disk = 2;
	desired_disk = 0;
	assert(fsctl(test_hfs_volume, HFSIOC_SET_VERY_LOW_DISK, &very_low_disk, 0) < 0);
	assert(fsctl(test_hfs_volume, HFSIOC_SET_LOW_DISK, &low_disk, 0) < 0);
	assert(fsctl(test_hfs_volume, APFSIOC_SET_NEAR_LOW_DISK, &near_low_disk, 0) < 0);
	assert(fsctl(test_hfs_volume, HFSIOC_SET_DESIRED_DISK, &desired_disk, 0) < 0);

	assert_no_err(fsctl(test_hfs_volume, HFSIOC_GET_VERY_LOW_DISK, &very_low_disk, 0));
	assert_no_err(fsctl(test_hfs_volume, HFSIOC_GET_LOW_DISK, &low_disk, 0));
	assert_no_err(fsctl(test_hfs_volume, APFSIOC_GET_NEAR_LOW_DISK, &near_low_disk, 0));
	assert_no_err(fsctl(test_hfs_volume, HFSIOC_GET_DESIRED_DISK, &desired_disk, 0));
	assert_equal(very_low_disk, 1, "%d");
	assert_equal(low_disk, 2, "%d");
	assert_equal(near_low_disk, 3, "%d");
	assert_equal(desired_disk, 4, "%d");

	return 0;
}
