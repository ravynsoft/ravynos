#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <libgen.h>

#include "hfs-tests.h"
#include "test-utils.h"
#include "disk-image.h"

TEST(access)

struct ext_access_t {
    uint32_t   flags;           /* IN: access requested (i.e. R_OK) */
    uint32_t   num_files;       /* IN: number of files to process */
    uint32_t   map_size;        /* IN: size of the bit map */
    uint32_t  *file_ids;        /* IN: Array of file ids */
    char      *bitmap;          /* OUT: hash-bitmap of interesting directory ids */
    short     *access;          /* OUT: access info for each file (0 for 'has access') */
    uint32_t   num_parents;   /* future use */
    uint32_t      *parents;   /* future use */
};

int run_access(__unused test_ctx_t *ctx)
{
	disk_image_t *di = disk_image_get();
	
	char *path;
	asprintf(&path, "%s/acces_check.data", di->mount_point);

	int fd;
	assert_with_errno((fd = open(path, O_RDWR | O_CREAT, 0666)) >= 0);

	struct stat sb;
	assert_no_err(fstat(fd, &sb));

	assert_no_err(unlink(path));

	char dir_map[1 << 16] = { 0 };
	short access_vector[1024] = { 0 };

	struct ext_access_t params = {
		.flags		= R_OK, 
		.num_files 	= 1,
		.map_size 	= sizeof(dir_map),
		.file_ids 	= (uint32_t[]){ (uint32_t)sb.st_ino },
		.bitmap 	= dir_map,
		.access 	= access_vector,
	};

#define HFSIOC_EXT_BULKACCESS _IOW('h', 15, struct ext_access_t)

	char *base_path = strdup(path);
	base_path = dirname(base_path);
	assert_no_err(fsctl(base_path, HFSIOC_EXT_BULKACCESS, &params, 0));

	if (access_vector[0] != ENOENT)
		assert_fail("access_vector[0] != ENOENT (== %u)!", access_vector[0]);
	
	free(path);
	assert_no_err (close(fd));

	return 0;
}
