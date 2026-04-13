#include <sys/fcntl.h>
#include <sys/attr.h>
#include <unistd.h>

#include "hfs-tests.h"
#include "test-utils.h"
#include "disk-image.h"

TEST(set_create_time)

static disk_image_t *di;

int run_set_create_time(__unused test_ctx_t *ctx)
{
	di = disk_image_get();
	
	char *file;
	asprintf(&file, "%s/set-create-time.data", di->mount_point);
	
	int fd;

	assert_with_errno((fd = open(file, O_CREAT | O_RDWR, 0666)) >= 0);

	struct attrs {
		uint32_t len;
		struct timespec cr_time, mod_time;
	} attrs = {
		0,
		{ 2000, 0 },	// Create time after mod time
		{ 1000, 0 }
	};

	struct attrlist attrlist = {
		.bitmapcount = ATTR_BIT_MAP_COUNT,
		.commonattr = ATTR_CMN_CRTIME | ATTR_CMN_MODTIME,
	};

	assert_no_err(fsetattrlist(fd, &attrlist, (char *)&attrs + 4, sizeof(attrs) - 4, 0));

	assert_no_err(fgetattrlist(fd, &attrlist, &attrs, sizeof(attrs), 0));

	assert(attrs.cr_time.tv_sec == 2000 && attrs.mod_time.tv_sec == 1000);

	assert_no_err (close(fd));
	unlink(file);
	free(file);

	return 0;
}
