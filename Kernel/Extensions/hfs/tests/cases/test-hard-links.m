#include <unistd.h>
#include <sys/fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/mount.h>
#include <System/sys/fsgetpath.h>
#include <TargetConditionals.h>
#include <spawn.h>
#import <Foundation/Foundation.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "hfs-tests.h"
#include "test-utils.h"
#include "disk-image.h"
#include "systemx.h"

TEST(hard_links)

static disk_image_t *di;

char *dir, *file, *lnk, *moved, *file2, *dir1, *dir2;
#define DIR "/tmp/hard-links"
#define FILE "/tmp/hard-links/file"
#define LINK "/tmp/hard-links.link"
#define MOVED "/tmp/hardlinks.moved"
#define FILE2 "/tmp/hard-links.file2"
#define DIR1 "/tmp/hard-links.dir1"
#define DIR2 "/tmp/hard-links/dir2"
#define DISK_IMAGE "/tmp/hard-links.sparseimage"

void *do_link(__unused void *arg)
{
	assert_no_err(link(moved, file));
	return NULL;
}

static void run(void)
{
	char *tstdir;
	di = disk_image_get();
	
	asprintf(&tstdir, "%s/tmp", di->mount_point);
	assert(!mkdir(tstdir, 0777) || errno == EEXIST);
	asprintf(&dir, "%s/hard-links", tstdir);
	asprintf(&file, "%s/file", dir);
	asprintf(&lnk, "%s/hard-links.link", tstdir);
	asprintf(&moved, "%s/harlinks.moved", tstdir);
	asprintf(&file2, "%s/hard-links.file2", tstdir);
	asprintf(&dir1, "%s/hard-links.dir1", tstdir);
	asprintf(&dir2, "%s/dir2", dir);
	
	unlink(lnk);
	unlink(file);
	unlink(file2);
	unlink(moved);
	rmdir(dir2);
	rmdir(dir);
	rmdir(dir1);

	/*
	 * The sequence that follows used to cause a loop in the kernel on
	 * iOS.  For some reason, it doesn't on OS X.
	 */
	assert(!mkdir(dir, 0777));
	int fd = open(file, O_CREAT | O_RDWR, 0666);
	assert_with_errno(fd >= 0);
	assert_no_err(close(fd));
	assert_no_err(link(file, lnk));
	struct stat sb;
	assert_no_err(stat(lnk, &sb));
	assert_no_err(stat(file, &sb));
	assert_no_err(rename(file, moved));
	assert_no_err(rmdir(dir));
	assert_no_err(mkdir(dir, 0777));

	pthread_t thread;
	pthread_create(&thread, NULL, do_link, NULL);
	pthread_join(thread, NULL);

	fd = open(file, O_CREAT | O_RDWR, 0666);
	assert_with_errno(fd >= 0);
	assert_no_err(close(fd));

	assert_no_err(unlink(lnk));
	assert_no_err(unlink(file));
	assert_no_err(rmdir(dir));
	assert_no_err(unlink(moved));

	// This is another sequence that causes problems...

	fd = open(file2, O_RDWR | O_CREAT, 0666);
	assert_with_errno(fd >= 0);
	assert(!fstat(fd, &sb));
	struct statfs sfs;
	assert_no_err(fstatfs(fd, &sfs));
	assert_no_err(close(fd));
	assert_no_err(mkdir(dir, 0777));
	assert_no_err(link(file2, file));
	assert_no_err(unlink(file));
	assert_no_err(rmdir(dir));

	fsobj_id_t fsobj = {
		.fid_objno = (uint32_t)sb.st_ino,
	};

	fd = openbyid_np(&sfs.f_fsid, &fsobj, O_RDWR);

	assert_with_errno(fd >= 0);

	assert_no_err(unlink(file2));

#if !(TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR)
	// Same as the last sequence but with directory hard links

	assert_no_err(mkdir(dir1, 0777));
	assert(!stat(dir1, &sb));
	assert_no_err(mkdir(dir, 0777));

	//printf("dir1: %s, dir2: %s\n", dir1, dir2);
	assert_no_err(link(dir1, dir2));
	assert_no_err(rmdir(dir2));
	assert_no_err(rmdir(dir));

	fsobj.fid_objno = (uint32_t)sb.st_ino;

	fd = openbyid_np(&sfs.f_fsid, &fsobj, O_RDONLY);

	assert_with_errno(fd >= 0);

	assert_no_err(rmdir(dir1));

	// Test for leaked iocounts

	disk_image_t *di2 = disk_image_create(DISK_IMAGE, &(disk_image_opts_t){
															.size = 100 * 1024 * 1024
														});

	char *path1, *path2;

	asprintf(&path1, "%s/dir1", di2->mount_point);
	assert_no_err(mkdir(path1, 0777));
	free(path1);
	asprintf(&path2, "%s/dir2", di2->mount_point);
	assert_no_err(mkdir(path2, 0777));
	free(path2);

	asprintf(&path1, "%s/dir1/hard-links-test-file", di2->mount_point);
	asprintf(&path2, "%s/dir2/link", di2->mount_point);

	dispatch_group_t grp = dispatch_group_create();
	dispatch_queue_t q = dispatch_get_global_queue(0, 0);

	//printf("path1: %s, path2: %s\n", path1, path2);
	for (int i = 0; i < 10000; ++i) {
		fd = open(path1, O_CREAT | O_RDWR, 0666);
		assert_with_errno(fd >= 0);
		assert_no_err(link(path1, path2));
		assert_no_err(fstat(fd, &sb));
		assert_no_err(fstatfs(fd, &sfs));
		assert_no_err(close(fd));

		dispatch_group_async(grp, q, ^{
			char path[PATH_MAX];
			ssize_t res;
			do {
				res = fsgetpath(path, sizeof(path), &sfs.f_fsid, (uint64_t)sb.st_ino);
			} while (res != -1);
			assert_with_errno(errno == ENOENT);
		});

		dispatch_group_async(grp, q, ^{
			assert_no_err(unlink(path1));
		});

		dispatch_group_async(grp, q, ^{
			assert_no_err(unlink(path2));
		});

		dispatch_group_wait(grp, DISPATCH_TIME_FOREVER);
	}

	// Test hard links to sockets and fifos

	assert_with_errno((fd = socket(PF_UNIX, SOCK_STREAM, 0)) >= 0);

	struct sockaddr_un addr = {
		.sun_family = AF_UNIX
	};

	sprintf(addr.sun_path, "%s/socket", di2->mount_point);

	assert_no_err(bind(fd, (struct sockaddr *)&addr, sizeof(addr)));

	asprintf(&path1, "%s/socket-link", di2->mount_point);
	assert_no_err(link(addr.sun_path, path1));
	assert_no_err(unlink(path1));

	// And a fifo
	char *fifo_path;
	asprintf(&fifo_path, "%s/fifo", di2->mount_point);
	asprintf(&path2, "%s/fifo-link", di2->mount_point);

	assert_no_err(mkfifo(fifo_path, 0666));
	assert_no_err(link(fifo_path, path2));
	assert_no_err(unlink(path2));

	char *mut_vol_device = strdup(di2->disk);
	assert(!systemx("/sbin/fsck_hfs", SYSTEMX_QUIET, "-ld", mut_vol_device, NULL));

	assert_no_err(close(fd));

	// Unmount
	assert(!systemx("/usr/sbin/diskutil", SYSTEMX_QUIET, "unmount", mut_vol_device, NULL));

	// Remount
	assert(!systemx("/usr/sbin/diskutil", SYSTEMX_QUIET, "mount", mut_vol_device, NULL));

	// We assume the same mount_point
	assert_no_err(unlink(addr.sun_path));
	assert_no_err(unlink(fifo_path));

	assert(!systemx("/sbin/fsck_hfs", SYSTEMX_QUIET, "-ld", mut_vol_device, NULL));

#endif //!(TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR)

	return;
}

int run_hard_links(__unused test_ctx_t *ctx)
{
	run();

	return 0;
}
