#include <TargetConditionals.h>

#if !TARGET_OS_IPHONE

#include <sys/fcntl.h>
#include <sys/types.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/mount.h>
#include <sys/param.h>
#include <copyfile.h>

#include "hfs-tests.h"
#include "test-utils.h"
#include "disk-image.h"

TEST(move_data_extents)

static disk_image_t *di;
static char *file1, *file2;

#define TEST_FILE_1		"/tmp/move_data_extents.1"
#define TEST_FILE_2		"/tmp/move_data_extents.2"

static volatile bool run_thread;

static void *write_thread(void *param)
{
	int fd = (int)(uintptr_t)param;

	while (run_thread)
		write(fd, &fd, 4);

	return NULL;
}

static int make_frag_file(int blocks_per_extent)
{
	int fd;
	
	asprintf(&file1, "%s/move_data_extents.1", di->mount_point);
	assert_with_errno((fd = open(file1,
								 O_CREAT | O_RDWR | O_TRUNC | O_EVTONLY, 0666)) >= 0);

	struct statfs sfs;
	assert_no_err(statfs(file1, &sfs));

	fstore_t fstore = {
		.fst_flags = F_ALLOCATECONTIG | F_ALLOCATEALL,
		.fst_posmode = F_PEOFPOSMODE,
	};

	off_t len = 0;

	for (int i = 0; i < 9; ++i) {
		if (len) {
			struct log2phys l2p = {
				.l2p_contigbytes = 1024 * 1024,
				.l2p_devoffset   = len - 1,
			};
			if (fcntl(fd, F_LOG2PHYS_EXT, &l2p)) {
				if (errno == ERANGE)
					break;
				assert_fail("fcntl failed: %s", strerror(errno));
			}

			fstore.fst_posmode = F_VOLPOSMODE;
			fstore.fst_offset = l2p.l2p_devoffset + 1 + sfs.f_bsize;
		}

		len += blocks_per_extent * sfs.f_bsize;

		fstore.fst_length = len;

		assert_no_err(fcntl(fd, F_PREALLOCATE, &fstore));
	}

	assert_no_err(ftruncate(fd, len));

	return fd;
}

int run_move_data_extents(__unused test_ctx_t *ctx)
{
	di = disk_image_get();
	
	int frag_fd = make_frag_file(1);
	int fd;
	int fd2;

	struct stat sb;
	fstat(frag_fd, &sb);

	
	asprintf(&file2, "%s/move_data_extents.2", di->mount_point);
	assert_with_errno((fd2 = open(file2,
								  O_RDWR | O_TRUNC | O_CREAT, 0666)) >= 0);

#define F_MOVEDATAEXTENTS 69

	assert_no_err(fcntl(frag_fd, F_MOVEDATAEXTENTS, fd2));

	char buf[4096];
	check_io(pread(fd2, buf, 4096, sb.st_size - 4096), 4096);

	check_io(pwrite(fd2, buf, 100, sb.st_size), 100);

	close(fd2);

	assert_with_errno((fd = open(file1,
								 O_APPEND | O_RDWR)) >= 0);

	run_thread = true;

	pthread_t thread;
	pthread_create(&thread, NULL, write_thread, (void *)(uintptr_t)fd);

	for (int i = 0; i < 500; ++i) {
		assert_with_errno((fd2 = open(file2,
									  O_RDWR | O_TRUNC | O_CREAT, 0666)) >= 0);

		assert(fcntl(fd, F_MOVEDATAEXTENTS, fd2) == -1 && errno == EBUSY);

		assert_no_err(close(fd2));
	}

	run_thread = false;
	pthread_join(thread, NULL);

	close(fd);
	close(frag_fd);

	/*
	 * Make sure that the extents from move_data_extents.1 got deleted
	 * properly.  To test this, we do another F_MOVEDATAEXTENTS and
	 * this time if it's broken it will move the bad extents.
	 */

	fd = make_frag_file(2);

	assert_with_errno((fd2 = open(file2,
								  O_RDWR | O_TRUNC | O_CREAT, 0666)) >= 0);

	assert_no_err(fcntl(fd, F_MOVEDATAEXTENTS, fd2));

	close(fd);
	close(fd2);

	// And this time it should fail if there's a bug
	fd = make_frag_file(1);

	assert_with_errno((fd2 = open(file2,
								  O_RDWR | O_TRUNC | O_CREAT, 0666)) >= 0);

	assert_no_err(fcntl(fd, F_MOVEDATAEXTENTS, fd2));

	assert_no_err(unlink(file1));
	assert_no_err(unlink(file2));
	assert_no_err(close(fd));
	assert_no_err(close(fd2));

	assert_no_err(copyfile("/bin/bash", file1, NULL, COPYFILE_ALL));

	assert_no_err(chmod(file1, 0777));

	assert_no_err(stat(file1, &sb));

	assert(sb.st_flags & UF_COMPRESSED);

	assert_with_errno((fd = open(file1, O_RDONLY | O_EVTONLY)) >= 0);

	assert_with_errno((fd2 = open(file2,
								  O_RDWR | O_TRUNC | O_CREAT, 0666)) >= 0);

	int fd3;
	char *file3;
	
	asprintf(&file3, "%s/..namedfork/rsrc", file2);
	assert_with_errno((fd3 = open(file3,
								  O_RDWR | O_TRUNC | O_CREAT, 0666)) >= 0);

	assert_no_err(fcntl(fd, F_MOVEDATAEXTENTS, fd2));

	assert_no_err(unlink(file1));
	assert_no_err(close(fd));

	assert_no_err(chflags(file2, 0));

	assert_with_errno((fd = open(file3,
								 O_RDWR)) >= 0);

	check_io(pwrite(fd, "append", 6, sb.st_size), 6);

	assert_no_err(unlink(file2));
	
	assert_no_err (close(fd));
	assert_no_err (close(fd2));
	assert_no_err (close(fd3));

	free(file1);
	free(file2);
	free(file3);

	return 0;
}

#endif // !TARGET_OS_IPHONE
