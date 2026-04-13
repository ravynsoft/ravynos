#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <err.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/param.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/resource.h>
#include <TargetConditionals.h>

#include "hfs-tests.h"
#include "test-utils.h"
#include "disk-image.h"

TEST(mod_time)

static disk_image_t *di;

static void run_test(void)
{
	char *file;
	int fd;

	asprintf(&file, "%s/tstfile", di->mount_point);
	assert((fd = open(file, O_CREAT | O_RDWR | O_TRUNC, 0666)) >= 0);

	char data[] = "The quick brown fox jumps over the lazy dog\n";

	check_io(write(fd, data, sizeof (data) - 1), sizeof(data) - 1);

	struct timeval times[] = {
		{ time(NULL), 0 },
		{ 1234567890, 0 }
	};

	assert_no_err(futimes(fd, times));

	/* Have to open file again because whether it's writable comes from
	   the file descriptor, *not* how we map. */
	int fd2;

	assert_with_errno((fd2 = open(file, O_RDONLY)) >= 0);
	void *p;

	assert((p = mmap(NULL, NBPG, PROT_READ, MAP_SHARED, fd2, 0)) != MAP_FAILED);

	assert_no_err(msync(p, NBPG, MS_INVALIDATE));

	struct stat sb;
	assert_no_err(fstat(fd, &sb));

	assert (sb.st_mtimespec.tv_sec == times[1].tv_sec);

	assert_no_err(close(fd));
	assert_no_err(close(fd2));

	assert_no_err(unlink(file));
	assert_no_err(munmap(p, NBPG));
}

int run_mod_time(__unused test_ctx_t *ctx)
{
	di = disk_image_get();
	
	// We need to run the test twice because the sync runs every 30 secs or so
	run_test();
	run_test();

	return 0;
}
