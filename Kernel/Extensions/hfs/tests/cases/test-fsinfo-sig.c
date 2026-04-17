/*
 * Copyright (c) 2015 Apple, Inc. All rights reserved.
 *
 * Test HFS fsinfo fsctls can be interrupted by pthread_kill signals.
 */

#include <sys/ioctl.h>
#include <sys/ioccom.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <spawn.h>

#include "../../core/hfs_fsctl.h"
#include "hfs-tests.h"
#include "test-utils.h"
#include "systemx.h"
#include "disk-image.h"

TEST(fsinfo_sig, .run_as_root = true)

static disk_image_t *di;

static hfs_fsinfo	fsinfo;
static volatile bool done = false;
static volatile bool thread_to_exit = false;
static volatile bool thread_is_signaled = false;

static char *fsinfo_srcdir;
#define MAX_FILES	100
#define KB			*1024

static void setup_testvolume()
{
	char			*path;
	int				fd;
	void			*buf;

	// Create a test folder with MAX_FILES files
	assert_no_err(systemx("/bin/rm", "-rf", fsinfo_srcdir, NULL));
	assert_no_err(mkdir(fsinfo_srcdir, 0777));
	//printf("\n%s:%d creating files ", __func__, __LINE__);

	for (unsigned i = 0; i < MAX_FILES; i++) {
		asprintf(&path, "%s/fsinfo_test.data.%u", fsinfo_srcdir, getpid()+i);
		unlink(path);
		assert_with_errno((fd = open(path, O_RDWR | O_TRUNC | O_CREAT, 0666)) >= 0);

		unsigned buf_size = (4 KB) * i;
		buf = malloc(buf_size);
		memset(buf, 0x25, buf_size);
		check_io(write(fd, buf, buf_size), buf_size);
		free(buf);
		free(path);
	}
	//printf("\n%s:%d created %d files ", __func__, __LINE__, MAX_FILES);
}

static int test_fsinfo_file_extent_size(void)
{
	int error = 0;

	bzero(&fsinfo, sizeof(fsinfo));
	fsinfo.header.request_type = HFS_FSINFO_FILE_EXTENT_SIZE;
	fsinfo.header.version = HFS_FSINFO_VERSION;
	error = fsctl(di->mount_point, HFSIOC_GET_FSINFO, &fsinfo, 0);
    //if (error)
    //    printf("\n%s:%d error = %d errno = %d %s", __func__, __LINE__, error, errno, strerror(errno));
	return error != 0 ? errno : 0;
}

static int test_fsinfo_free_extents(void)
{
	int error = 0;

	bzero(&fsinfo, sizeof(fsinfo));
	fsinfo.header.version = HFS_FSINFO_VERSION;
	fsinfo.header.request_type = HFS_FSINFO_FREE_EXTENTS;
	error = fsctl(di->mount_point, HFSIOC_GET_FSINFO, &fsinfo, 0);
    //if (error)
    //    printf("\n%s:%d error = %d errno = %d %s", __func__, __LINE__, error, errno, strerror(errno));
	return error != 0 ? errno : 0;
}


void sig_handler (__unused int signum) {
	thread_is_signaled = true;
}

void *threadfunc1(__unused void *parm ) {
	struct sigaction action;
    int sig_count = 1000;
    int test_ret = 0;
	action.sa_handler = sig_handler;
	sigemptyset( &action.sa_mask );
	action.sa_flags = 0;
	
	sigaction( SIGUSR1, &action, NULL );
	
    //printf("\n%s:%d starting test_fsinfo_file_extent_size in loop", __func__, __LINE__);
	while (sig_count > 0) {
        test_ret = test_fsinfo_file_extent_size();
		if (test_ret == EINTR) {
	        done = true;
			break;
        }
        if (test_ret == EACCES) {
            printf("\n%s:%d got EACCESS. Failing the test. Please run as root.", __func__, __LINE__);
        }
        assert(test_ret == 0);

        if (thread_is_signaled) {
            thread_is_signaled = false;
            sig_count -= 1;
        }

	}
	
    assert(done == true);

	while (!thread_to_exit)
		;

	pthread_exit( NULL );
}

void *threadfunc2(__unused void *parm ) {
	struct sigaction action;
    int sig_count = 1000;
    int test_ret = 0;

	action.sa_handler = sig_handler;
	sigemptyset( &action.sa_mask );
	action.sa_flags = 0;

	sigaction( SIGUSR1, &action, NULL );

    //printf("\n%s:%d starting test_fsinfo_free_extents in loop", __func__, __LINE__);
    while (sig_count > 0) {
        test_ret = test_fsinfo_free_extents();
		if (test_ret == EINTR) {
	        done = true;
			break;
        }
        if (test_ret == EACCES) {
            printf("\n%s:%d got EACCESS. Failing the test. Please run as root.", __func__, __LINE__);
        }
        assert(test_ret == 0);

        if (thread_is_signaled) {
            thread_is_signaled = false;
            sig_count -= 1;
        }

	}

	assert(done == true);

	while (!thread_to_exit)
		;

	pthread_exit( NULL );
}


int run_fsinfo_sig(__unused test_ctx_t *ctx) {
	pthread_t threadid1, threadid2;
	int       thread_stat1, thread_stat2;
	
	di = disk_image_get();
	
	asprintf(&fsinfo_srcdir, "%s/fsinfo-test", di->mount_point);

	setup_testvolume();
	
	thread_is_signaled = false;
	/* To test traverse_btree path takes signal */
	assert_no_err(pthread_create( &threadid1, NULL, threadfunc1, NULL ));

	sleep( 10 );
	//printf("\n%s:%d signalling thread ", __func__, __LINE__);

	while (!done) {
		assert_no_err(pthread_kill( threadid1, SIGUSR1 ));
	}

	thread_to_exit = true;

	assert_no_err(pthread_join( threadid1, (void *)&thread_stat1 ));

	thread_to_exit = false;
	done = false;
	thread_is_signaled = false;

	/* To test hfs_find_free_extents code path takes signal */
	assert_no_err(pthread_create( &threadid2, NULL, threadfunc2, NULL ));

	sleep( 10 );
	//printf("\n%s:%d signalling thread ", __func__, __LINE__);

	while (!done) {
		assert_no_err(pthread_kill( threadid2, SIGUSR1 ));
	}

	thread_to_exit = true;

	assert_no_err(pthread_join( threadid2, (void *)&thread_stat2 ));

	free(fsinfo_srcdir);

	return 0;
}
