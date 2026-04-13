//
//  test-dateadded.c
//  hfs
//
//  Created by csuter on 8/28/15.
//
//

#include <fcntl.h>
#include <sys/attr.h>
#include <unistd.h>
#include <sys/stat.h>

#include "hfs-tests.h"
#include "test-utils.h"
#include "../core/hfs_fsctl.h"
#include "disk-image.h"

TEST(dateadded)

#define TIME_TO_SET 1440807730

int run_dateadded(__unused test_ctx_t *ctx)
{
	disk_image_t *di = disk_image_get();
	char *file, *file2, *file3, *dir;
	
	asprintf(&file, "%s/test-dateadded.file", di->mount_point);
	asprintf(&file2, "%s/test-dateadded.file2", di->mount_point);
	asprintf(&dir, "%s/test-dateadded.dir", di->mount_point);
	asprintf(&file3, "%s/file3", dir);
	
	int fd = open(file, O_RDWR | O_CREAT, 0666);
	assert_with_errno(fd >= 0);

	struct attrlist al = {
		.bitmapcount = ATTR_BIT_MAP_COUNT,
		.commonattr = ATTR_CMN_ADDEDTIME
	};

#pragma pack(push, 4)
	struct {
		uint32_t				len;
		struct timespec			added_time;
	} attrs;
#pragma pack(pop)

	assert_no_err(fgetattrlist(fd, &al, &attrs, sizeof(attrs), 0));

	struct timespec orig = attrs.added_time;

	// Make sure rename doesn’t change it
	rename(file, file2);

	attrs.added_time.tv_sec = 0;
	assert_no_err(getattrlist(file2, &al, &attrs, sizeof(attrs), 0));

	assert_equal(attrs.added_time.tv_sec, orig.tv_sec, "%ld");

	sleep(2);

	// Rename to a different directory should change it
	rmdir(dir);
	mkdir(dir, 0777);

	assert_no_err(rename(file2, file3));

	attrs.added_time.tv_sec = 0;
	assert_no_err(getattrlist(file3, &al, &attrs, sizeof(attrs), 0));

	assert(attrs.added_time.tv_sec >= orig.tv_sec
		   && attrs.added_time.tv_sec < orig.tv_sec + 10);

#if 0 // Not supported until VFS work is done
	attrs.added_time.tv_sec = TIME_TO_SET;

	assert_no_err(fsetattrlist(fd, &al, (void *)&attrs + 4,
							   sizeof(attrs) - 4, 0));

	attrs.added_time.tv_sec = 0;
	assert_no_err(fgetattrlist(fd, &al, &attrs, sizeof(attrs), 0));

	assert_equal(attrs.added_time.tv_sec, TIME_TO_SET, "%ld");
#endif

	assert_no_err(unlink(file3));
	assert_no_err(rmdir(dir));
	assert_no_err(close(fd));

	return 0;
}
