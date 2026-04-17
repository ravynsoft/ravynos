#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/attr.h>

#include "hfs-tests.h"
#include "test-utils.h"
#include "disk-image.h"

TEST(mmap_mod_time)

static disk_image_t *di;

int run_mmap_mod_time(__unused test_ctx_t *ctx)
{
	di = disk_image_get();
	
	char *file;
	asprintf(&file, "%s/mmap_mod_time_test.dat", di->mount_point);
	
	int fd;

	assert_with_errno((fd = open(file, 
								 O_RDWR | O_CREAT | O_TRUNC, 0666)) >= 0);

	assert_no_err(ftruncate(fd, 65536));
	assert_no_err(fsync(fd));

	struct attrlist attrlist = {
		.bitmapcount = ATTR_BIT_MAP_COUNT,
		.commonattr = ATTR_CMN_MODTIME | ATTR_CMN_GEN_COUNT,
	};
#pragma pack(push, 4)
	struct {
		uint32_t len;
		struct timespec mod_time;
		uint32_t gen_count;
	} attrs[2];
#pragma pack(pop)

	assert_no_err(fgetattrlist(fd, &attrlist, &attrs[0], sizeof(attrs[0]),
							   FSOPT_ATTR_CMN_EXTENDED));

	assert_no_err(close(fd));

	assert_no_err(getattrlist(file,
							  &attrlist, &attrs[1], sizeof(attrs[1]), 
							  FSOPT_ATTR_CMN_EXTENDED));

	assert(attrs[1].gen_count == attrs[0].gen_count);

	sleep(2);

	assert_with_errno((fd = open(file, 
								 O_RDWR)) >= 0);

	void *p;
	assert_with_errno((p = mmap(NULL, 65536, PROT_WRITE, 
								MAP_SHARED, fd, 0)) != MAP_FAILED);

	char data[] = "mmap_mod_time_test";
	memcpy(p, data, sizeof(data) - 1);

	assert_no_err(msync(p, 65536, MS_SYNC));

	assert_no_err(munmap(p, 65536));

	assert_no_err(fgetattrlist(fd, &attrlist, &attrs[1], sizeof(attrs[1]), 
							   FSOPT_ATTR_CMN_EXTENDED));

	assert(attrs[0].mod_time.tv_sec != attrs[1].mod_time.tv_sec
		   || attrs[0].mod_time.tv_nsec != attrs[1].mod_time.tv_nsec);

	assert(attrs[1].gen_count != attrs[0].gen_count);

	assert_no_err(unlink(file));

	assert_no_err(close(fd));
	free(file);
	
	return 0;
}
