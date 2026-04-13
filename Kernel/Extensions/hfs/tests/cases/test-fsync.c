#include <pthread.h>
#include <unistd.h>
#include <hfs/hfs_mount.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdbool.h>
#include <TargetConditionals.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>

#include "hfs-tests.h"
#include "test-utils.h"
#include "disk-image.h"

TEST(fsync, .run_as_root = true)

static int fd;
volatile bool stop_thread;

void *sync_thread(__unused void *arg)
{
	while (!stop_thread)
		assert_no_err(fsync(fd));

	return NULL;
}

int run_fsync(__unused test_ctx_t *ctx)
{
	disk_image_t *di = NULL;
	const char *tstdir;
	
#if (TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR)
	struct statfs sfs;
	bool hfs_root;
	
	assert(statfs("/tmp", &sfs) == 0);
	if (strcmp(sfs.f_fstypename, "hfs")) {
		hfs_root = false;
		di = disk_image_get();
		tstdir = di->mount_point;
	} else {
		hfs_root = true;
		tstdir = "/tmp";
	}
#else // !TARGET_OS_IPHONE & !SIM
	di = disk_image_get();
	tstdir = di->mount_point;
#endif
	
	char *file;
	asprintf(&file, "%s/fsync.data", tstdir);
	
	pthread_t thread;

	fd = open(file, O_RDWR | O_CREAT, 0666);

	assert_with_errno(fd >= 0);


	pthread_create(&thread, NULL, sync_thread, NULL);

	void *buf = valloc(65536);

	assert_no_err(fcntl(fd, F_NOCACHE, 1));

	struct timeval start, now;
	gettimeofday(&start, NULL);
	do {
		check_io(pwrite(fd, buf, 65536, 0), 65536);

		gettimeofday(&now, NULL);
	} while (now.tv_sec < start.tv_sec + 10);

	stop_thread = true;
	pthread_join(thread, NULL);

	assert_no_err(close(fd));
	assert_no_err(unlink(file));

	assert_with_errno((fd = socket(PF_UNIX, SOCK_STREAM, 0)) >= 0);

	struct sockaddr_un addr = {
		.sun_family = AF_UNIX
	};

	char *sock;
	asprintf(&sock, "%s/socket", tstdir);
	unlink(sock);
	strcpy(addr.sun_path, sock);

	assert_no_err(bind(fd, (struct sockaddr *)&addr, sizeof(addr)));

	// And a fifo
	char *fifo;
	asprintf(&fifo, "%s/fifo", tstdir);
	unlink(fifo);
	assert_no_err(mkfifo(fifo, 0666));

	sync();

	assert_no_err(unlink(sock));
	assert_no_err(unlink(fifo));
	
	free(file);
	free(sock);
	free(fifo);
	free(buf);

	return 0;
}
