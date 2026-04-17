#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <spawn.h>
#include <sys/stat.h>
#include <TargetConditionals.h>

#import <Foundation/Foundation.h>

#include "hfs-tests.h"
#include "test-utils.h"
#include "disk-image.h"

TEST(log2phys)

static disk_image_t *di;

int run_log2phys(__unused test_ctx_t *ctx)
{
	di = disk_image_get();
	char *file;
	asprintf(&file, "%s/log2phys.data", di->mount_point);
	
	int fd = open(file, O_RDWR | O_CREAT, 0666);

	struct log2phys l2p = {
		.l2p_contigbytes = OFF_MAX,
	};

	assert_no_err(ftruncate(fd, 1000));
	assert_no_err(fcntl(fd, F_LOG2PHYS_EXT, &l2p));

	l2p.l2p_contigbytes = -1;
	assert_with_errno(fcntl(fd, F_LOG2PHYS_EXT, &l2p) == -1 && errno == EINVAL);

	assert_no_err(close(fd));
	free(file);

	return 0;
}
