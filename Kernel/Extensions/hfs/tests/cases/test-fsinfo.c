/*
 * Copyright (c) 2014 Apple, Inc. All rights reserved.
 *
 * Test HFS fsinfo fsctls 
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
#include <spawn.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../../core/hfs_fsctl.h"
#include "hfs-tests.h"
#include "test-utils.h"
#include "systemx.h"
#include "disk-image.h"

TEST(fsinfo, .run_as_root = true)

static disk_image_t *di;
char *srcdir;

#define KB				*1024
#define MAX_FILES		100

static hfs_fsinfo	fsinfo;

static void test_fsinfo_metadata_blocks(void)
{
	bzero(&fsinfo, sizeof(fsinfo));
	fsinfo.header.request_type = HFS_FSINFO_METADATA_BLOCKS_INFO;
	fsinfo.header.version = HFS_FSINFO_VERSION;
	assert_no_err(fsctl(di->mount_point, HFSIOC_GET_FSINFO, &fsinfo, 0));
}

static void test_fsinfo_metadata_extents(void)
{
	bzero(&fsinfo, sizeof(fsinfo));
	fsinfo.header.request_type = HFS_FSINFO_METADATA_EXTENTS;
	fsinfo.header.version = HFS_FSINFO_VERSION;
	assert_no_err(fsctl(di->mount_point, HFSIOC_GET_FSINFO, &fsinfo, 0));
}

static void test_fsinfo_metadata_percentfree(void)
{
	bzero(&fsinfo, sizeof(fsinfo));
	fsinfo.header.request_type = HFS_FSINFO_METADATA_PERCENTFREE;
	fsinfo.header.version = HFS_FSINFO_VERSION;
	assert_no_err(fsctl(di->mount_point, HFSIOC_GET_FSINFO, &fsinfo, 0));
}

static void test_fsinfo_file_extent_count(void)
{
	bzero(&fsinfo, sizeof(fsinfo));
	fsinfo.header.request_type = HFS_FSINFO_FILE_EXTENT_COUNT;
	fsinfo.header.version = HFS_FSINFO_VERSION;
	assert_no_err(fsctl(di->mount_point, HFSIOC_GET_FSINFO, &fsinfo, 0));
}

static void test_fsinfo_file_extent_size(void)
{
	bzero(&fsinfo, sizeof(fsinfo));
	fsinfo.header.request_type = HFS_FSINFO_FILE_EXTENT_SIZE;
	fsinfo.header.version = HFS_FSINFO_VERSION;
	assert_no_err(fsctl(di->mount_point, HFSIOC_GET_FSINFO, &fsinfo, 0));
}

static void test_fsinfo_file_size(void)
{
	
	int				fd;
	unsigned		buf_size;
	void			*buf = NULL;
	struct			hfs_fsinfo_data *fsinfo_data = &fsinfo.data;
	unsigned long	file_size_count;
	unsigned		i;
	char			*path[10];

	bzero(&fsinfo, sizeof(fsinfo));
	fsinfo.header.version = HFS_FSINFO_VERSION;
	fsinfo.header.request_type = HFS_FSINFO_FILE_SIZE;
	assert_no_err(fsctl(di->mount_point, HFSIOC_GET_FSINFO, &fsinfo, 0));
	file_size_count = fsinfo_data->bucket[11];

	buf_size = (1 KB);
	buf = malloc(buf_size);
	memset(buf, 0x83, buf_size);

	for (i = 0; i < 10; i++)
	{
		asprintf(&path[i], "%s/fsinfo_test.data.%u", di->mount_point, i);
		unlink(path[i]);
		assert_with_errno((fd = open(path[i], O_RDWR | O_TRUNC | O_CREAT, 0666)) >= 0);
		check_io(write(fd, buf, buf_size), buf_size);
		close(fd);
	}

	bzero(&fsinfo, sizeof(fsinfo));
	fsinfo.header.version = HFS_FSINFO_VERSION;
	fsinfo.header.request_type = HFS_FSINFO_FILE_SIZE;
	assert_no_err(fsctl(di->mount_point, HFSIOC_GET_FSINFO, &fsinfo, 0));

	free(buf);

	for (i = 0; i < 10; i++)
		unlink(path[i]);

	assert(fsinfo_data->bucket[11] >= (file_size_count + 5));
}


static void test_fsinfo_dir_valence(void)
{
	unsigned long	dir_size_count;
	struct			hfs_fsinfo_data *fsinfo_data = &fsinfo.data;
	int				fd;
	char			*dir_path[10], *path2;
	unsigned		i;

	bzero(&fsinfo, sizeof(fsinfo));
	fsinfo.header.version = HFS_FSINFO_VERSION;
	fsinfo.header.request_type = HFS_FSINFO_DIR_VALENCE;
	assert_no_err(fsctl(di->mount_point, HFSIOC_GET_FSINFO, &fsinfo, 0));
	dir_size_count = fsinfo_data->bucket[4];

	for (i = 0; i < 10; i++)
	{
		asprintf(&dir_path[i], "%s/fsinfo_test.dir_%u", di->mount_point, i);
		assert_no_err(systemx("/bin/rm", "-rf", dir_path[i], NULL));
		assert_no_err(mkdir(dir_path[i], 0777));

		for (unsigned j = 0; j < 10; j++) {
			asprintf(&path2, "%s/fsinfo_test.data.%u", dir_path[i], getpid()+j);
			unlink(path2);
			assert_with_errno((fd = open(path2, O_RDWR | O_TRUNC | O_CREAT, 0666)) >= 0);
			close(fd);
		}
	}

	bzero(&fsinfo, sizeof(fsinfo));
	fsinfo.header.version = HFS_FSINFO_VERSION;
	fsinfo.header.request_type = HFS_FSINFO_DIR_VALENCE;
	assert_no_err(fsctl(di->mount_point, HFSIOC_GET_FSINFO, &fsinfo, 0));
	for (i = 0; i < 10; i++)
		assert_no_err(systemx("/bin/rm", "-rf", dir_path[i], NULL));

	assert(fsinfo_data->bucket[4] >= (dir_size_count + 5));

}

static void test_fsinfo_name_size(void)
{
	struct hfs_fsinfo_name  *fsinfo_name = &fsinfo.name;
	unsigned                bucket_index;
	unsigned long           name_size_count;
	char                    *filename = NULL;
	int                     fd;
	unsigned                i;
	char                    *path[10];

	bzero(&fsinfo, sizeof(fsinfo));
	fsinfo.header.version = HFS_FSINFO_VERSION;
	fsinfo.header.request_type = HFS_FSINFO_NAME_SIZE;
	assert_no_err(fsctl(di->mount_point, HFSIOC_GET_FSINFO, &fsinfo, 0));

	for (i = 0; i < 10; i++)
	{
		asprintf(&path[i], "%s/hfs_fsinfo_test.data.txt_%u", di->mount_point, i);
		unlink(path[i]);
		filename = basename(path[i]);
		assert_with_errno((fd = open(path[i], O_RDWR | O_TRUNC | O_CREAT, 0666)) >= 0);
		close(fd);
	}
	bucket_index = (unsigned int)strlen(filename)/5;
	name_size_count = fsinfo_name->bucket[bucket_index];

	bzero(&fsinfo, sizeof(fsinfo));
	fsinfo.header.version = HFS_FSINFO_VERSION;
	fsinfo.header.request_type = HFS_FSINFO_NAME_SIZE;
	assert_no_err(fsctl(di->mount_point, HFSIOC_GET_FSINFO, &fsinfo, 0));

	for (i = 0; i < 10; i++)
		unlink(path[i]);

	assert(fsinfo_name->bucket[bucket_index] >= (name_size_count+5));

}

static void test_fsinfo_xattr_size(void)
{
	bzero(&fsinfo, sizeof(fsinfo));
	fsinfo.header.version = HFS_FSINFO_VERSION;
	fsinfo.header.request_type = HFS_FSINFO_XATTR_SIZE;
	assert_no_err(fsctl(di->mount_point, HFSIOC_GET_FSINFO, &fsinfo, 0));
}

static void test_fsinfo_free_extents(void)
{
	bzero(&fsinfo, sizeof(fsinfo));
	fsinfo.header.version = HFS_FSINFO_VERSION;
	fsinfo.header.request_type = HFS_FSINFO_FREE_EXTENTS;
	assert_no_err(fsctl(di->mount_point, HFSIOC_GET_FSINFO, &fsinfo, 0));

}

static void test_fsinfo_symlink_size(void)
{
	char			*symlink_path[10];
	struct			hfs_fsinfo_data *fsinfo_data = &fsinfo.data;
	unsigned		symlink_count = 0;
	unsigned		i;
	char			*path;

	bzero(&fsinfo, sizeof(fsinfo));
	fsinfo.header.version = HFS_FSINFO_VERSION;
	fsinfo.header.request_type = HFS_FSINFO_SYMLINK_SIZE;
	assert_no_err(fsctl(di->mount_point, HFSIOC_GET_FSINFO, &fsinfo, 0));
	symlink_count = fsinfo_data->bucket[6];

	asprintf(&path, "%s/fsinfo_test.data", di->mount_point);

	for (i = 0; i < 10; i++)
	{
		asprintf(&symlink_path[i], "%s/fsinfo_test_link.%d", di->mount_point, i);
		unlink(symlink_path[i]);
		assert_no_err(systemx("/bin/ln", "-s", path, symlink_path[i], NULL));
	}

	bzero(&fsinfo, sizeof(fsinfo));
	fsinfo.header.version = HFS_FSINFO_VERSION;
	fsinfo.header.request_type = HFS_FSINFO_SYMLINK_SIZE;
	assert_no_err(fsctl(di->mount_point, HFSIOC_GET_FSINFO, &fsinfo, 0));

	for (i = 0; i < 10; i++)
		unlink(symlink_path[i]);

	assert(fsinfo_data->bucket[6] >= (symlink_count + 5));
}

static void setup_testvolume()
{
	char            *path;
	int				fd;
	void            *buf;
	
	// Create a test folder with MAX_FILES files
	assert_no_err(systemx("/bin/rm", "-rf", srcdir, NULL));
	assert_no_err(mkdir(srcdir, 0777));
	
	for (unsigned i = 0; i < MAX_FILES; i++) {
		asprintf(&path, "%s/fsinfo_test.data.%u", srcdir, getpid()+i);
		unlink(path);
		assert_with_errno((fd = open(path, O_RDWR | O_TRUNC | O_CREAT, 0666)) >= 0);
		free(path);
		
		unsigned buf_size = (1 KB) * (i + 1);
		buf = malloc(buf_size);
		memset(buf, 0x25, buf_size);
		check_io(write(fd, buf, buf_size), buf_size);
		free(buf);
		close(fd);
	}
}

int run_fsinfo(__unused test_ctx_t *ctx)
{
	
	di = disk_image_get();
	asprintf(&srcdir, "%s/fsinfo-test", di->mount_point);

	setup_testvolume();
	
	test_fsinfo_metadata_blocks();
	test_fsinfo_metadata_extents();
	test_fsinfo_metadata_percentfree();
	test_fsinfo_file_extent_count();
	test_fsinfo_file_extent_size();
	test_fsinfo_file_size();
	test_fsinfo_dir_valence();
	test_fsinfo_name_size();
	test_fsinfo_xattr_size();
	test_fsinfo_free_extents();
	test_fsinfo_symlink_size();
	
	free(srcdir);

	return 0;
}
