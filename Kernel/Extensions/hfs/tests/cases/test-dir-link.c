#include <TargetConditionals.h>

#if !TARGET_OS_IPHONE

#include <sys/stat.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <spawn.h>
#include <sys/time.h>

#include "hfs-tests.h"
#include "test-utils.h"
#include "systemx.h"
#include "disk-image.h"

TEST(dir_link)

static disk_image_t *di;
static char *dir1, *dir2;

static volatile bool stop_thread;

void *thread(__unused void *arg)
{
	char *path1, *path2;
	asprintf(&path1, "%s/dir1/..", dir1);
	asprintf(&path2, "%s/Dir1/..", dir1);

	struct stat sb;
	while (!stop_thread) {
		assert_no_err(stat(path1, &sb));
		assert_no_err(stat(path2, &sb));
	}

	free(path1);
	free(path2);
	
	return NULL;
}

int run_dir_link(__unused test_ctx_t *ctx)
{
	di = disk_image_get();
	
	char *tstdir;
	asprintf(&tstdir, "%s/tmp", di->mount_point);
	
	assert(!mkdir(tstdir, 0777) || errno == EEXIST);
	
	asprintf(&dir1, "%s/dir1", tstdir);
	asprintf(&dir2, "%s/dir2", tstdir);
	
	systemx("/bin/rm", "-rf", dir1, NULL);
	systemx("/bin/rm", "-rf", dir2, NULL);
	
	char *dir1dir1;
	asprintf(&dir1dir1, "%s/dir1", dir1);

	assert_no_err(mkdir(dir1, 0777));
	assert_no_err(mkdir(dir1dir1, 0777));

	pthread_t thread_id;
	pthread_create(&thread_id, NULL, thread, NULL);

	struct stat sb;
	struct timeval start, now, elapsed;

	gettimeofday(&start, NULL);

	char *path1, *path2;
	asprintf(&path1, "%s/dir2/..", tstdir);
	asprintf(&path2, "%s/Dir2/..", tstdir);
	
	do {
		assert_no_err(link(dir1dir1, dir2));
		assert_no_err(stat(path1, &sb));
		assert_no_err(stat(path2, &sb));
		assert_no_err(rmdir(dir2));

		gettimeofday(&now, NULL);

		timersub(&now, &start, &elapsed);
	} while (elapsed.tv_sec < 10);

	stop_thread = true;

	pthread_join(thread_id, NULL);

	assert_no_err(rmdir(dir1dir1));
	assert_no_err(rmdir(dir1));

	free(dir1);
	free(dir2);
	free(dir1dir1);
	free(path1);
	free(path2);
	
	return 0;
}

#endif // !TARGET_OS_IPHONE
