#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/attr.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/stat.h>
#include <sys/xattr.h>
#include <sys/mount.h>
#include <sys/param.h>

#include <System/sys/fsctl.h>

#include "hfs-tests.h"
#include "test-utils.h"
#include "disk-image.h"

//TEST(cas_bsdflags)

static bool
cas_bsd_flags(int fd, uint32_t expected_flags, uint32_t new_flags)
{
	struct fsioc_cas_bsdflags cas;

	cas.expected_flags = expected_flags;
	cas.new_flags      = new_flags;
	cas.actual_flags   = ~0;		/* poison */

	assert_no_err(ffsctl(fd, FSIOC_CAS_BSDFLAGS, &cas, 0));
	return (cas.expected_flags == cas.actual_flags);
}

int run_cas_bsdflags(__unused test_ctx_t *ctx)
{
	disk_image_t *di = disk_image_get();
	struct stat sb;
	int fd;

	char *file;
	asprintf(&file, "%s/cas_bsdflags.data", di->mount_point);

	assert_with_errno((fd = open(file,
								 O_CREAT | O_RDWR | O_TRUNC, 0666)) >= 0);

	assert_no_err(fchflags(fd, UF_HIDDEN));
	assert_no_err(fstat(fd, &sb));
	assert(sb.st_flags == UF_HIDDEN);

	assert(cas_bsd_flags(fd, 0, UF_NODUMP) == false);
	assert_no_err(fstat(fd, &sb));
	assert(sb.st_flags == UF_HIDDEN);

	assert(cas_bsd_flags(fd, UF_HIDDEN, UF_NODUMP) == true);
	assert_no_err(fstat(fd, &sb));
	assert(sb.st_flags == UF_NODUMP);

	assert(cas_bsd_flags(fd, UF_NODUMP, 0) == true);
	assert_no_err(fstat(fd, &sb));
	assert(sb.st_flags == 0);

	close(fd);
	assert_no_err(unlink(file));
	free(file);

	return 0;
}

