//
// Copyright (c) 2019-2019 Apple Inc. All rights reserved.
//
// test-lf-cs-plugin.c - Implements unit test for livefiles Apple_CoreStorage
//                       plugin.
//

#include "hfs-tests.h"
#include "test-utils.h"
#include "systemx.h"

#include <fcntl.h>
#include <spawn.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <zlib.h>
#include <TargetConditionals.h>

//
// Enable this test on iOS and no other iOS-style platforms.
//
#if TARGET_OS_IOS

//
// Headers generated at build time. Containes compressed disk images for the
// corresponding file-system formats.
//
#include "JHFS+-dmg.dat"
#include "APFS-dmg.dat"
#include "EXFAT-dmg.dat"
#include "FAT32-dmg.dat"

TEST(lf_cs_plugin, .run_as_root = true)

#define LF_CS_PLUGIN_TEST_DMG "/tmp/lf_cs_plugin_test.sparseimage"
#define LF_CS_PLUGIN_PATH_TO_HDIK "/usr/sbin/hdik"
#define LF_CS_PLUGIN_INSTALL_PATH "/AppleInternal/CoreOS/tests/hfs/"

//
// The output of hdik gets truncated, so just search for the leading part of
// the UUID and partition scheme names.
//
#define LF_CSP_HFS_UUID "48465300-0000-11AA-AA11"
#define LF_CSP_APFS_UUID "7C3457EF-0000-11AA-AA11"
#define LF_CSP_EXFAT_PART_SCHEME "Windows_NTFS"
#define LF_CSP_FAT32_PART_SCHEME "DOS_FAT_32"

//
// Enums describing file-system formats.
//
typedef enum {
	JHFS = 1,
	APFS,
	FAT32,
	EXFAT,
} lf_csp_fstype_t;

//
// Local struct describing the disk image on disk.
//
typedef struct {
	char *path;	// Path to disk-image on disk.
	char *disk;	// Path to dev node after disk-image is attached.
	char *slice;	// Path to dev node slice after disk-image is attached.
} lf_csp_disk_image_t;

//
// lf_csp_disk_image_cleanup - disattach disk image from the DiskImages driver
//                             and unlink the disk image file on disk.
//
static bool
lf_csp_disk_image_cleanup(lf_csp_disk_image_t *di)
{
	pid_t pid;
	posix_spawn_file_actions_t facts;

	struct stat sb;
	int ret, status;
	bool result;
	char *detach_args[] = { "hdik", "-e", (char *)di->disk, NULL };

	result = false;
	ret = posix_spawn_file_actions_init(&facts);

	if (!ret) {
		(void)posix_spawn_file_actions_addopen(&facts, STDOUT_FILENO,
			"/dev/null", O_APPEND, 0);
		(void)posix_spawn_file_actions_addopen(&facts, STDERR_FILENO,
			"/dev/null", O_APPEND, 0);
	}

	assert_no_err(posix_spawn(&pid, LF_CS_PLUGIN_PATH_TO_HDIK, &facts,
				NULL, detach_args, NULL));

	if (ret) {
		posix_spawn_file_actions_destroy(&facts);
	}

	assert_with_errno(ignore_eintr(waitpid(pid, &status, 0), -1) == pid);
	errno = 0;
	if (WIFEXITED(status) &&
			!WEXITSTATUS(status) &&
			(stat(di->disk, &sb) == -1) &&
			(errno == ENOENT)) {

		unlink(di->path);
		free(di->path);
		free(di->disk);
		free(di->slice);
		free(di);
		result = true;
	}

	return result;
}

//
// lf_csp_disk_image_create - create a new disk image file on disk and attach
//                            disk images directly to the DiskImages driver.
//
// This routine reads the compressed file system image (generated during build)
// based on the passed file-system type parameter. The compressed image is
// decompressd and written to disk and then attached to directly to the
// DiskImages driver.
//
// Please NOTE: The created image is hooked up to the test cleanup list thus
// gets cleaned up automatically after the test, even on failures. Explicit
// cleanup is not needed.
//
static lf_csp_disk_image_t *
lf_csp_disk_image_create(const char *path, lf_csp_fstype_t fs_type)
{
	pid_t pid;
	uid_t uid_old;
	lf_csp_disk_image_t *di;
	z_stream u_stream;
	posix_spawn_file_actions_t actions;

	FILE *fp;
	size_t lnsz;
	int ret, fd, fds[2], status;
	void *uncompressed_out_buf;
	const size_t chunk_size = (1ULL << 20);
	char *attach_args[4], *line, *part_scheme, *uuid_or_partid_str;

	//
	// We need to ensure that we are root.
	//
	if ((uid_old = geteuid()) != 0) {
		assert_no_err(seteuid(0));
	}

	di = calloc(1, sizeof(lf_csp_disk_image_t));
	assert(di);

	//
	// We need to extract the compressed image into the passed image
	// file path, thus we open it for writing.
	//
	fd = open(path, O_RDWR | O_TRUNC | O_CREAT, 0666);
	assert_with_errno(fd != -1);

	u_stream = (z_stream) {
		.zalloc = Z_NULL,
		.zfree = Z_NULL,
		.opaque = Z_NULL,
	};

	ret = inflateInit(&u_stream);
	if (ret != Z_OK) {
		assert_fail("inflateInit failed\n");
	}

	uncompressed_out_buf = malloc(chunk_size);
	assert(uncompressed_out_buf);

	uuid_or_partid_str = NULL;
	part_scheme = NULL;
	switch(fs_type) {
		case JHFS:
			u_stream.next_in = JHFS_data;
			u_stream.avail_in = sizeof(JHFS_data);
			uuid_or_partid_str = LF_CSP_HFS_UUID;
			part_scheme = "GUID";
			break;

		case APFS:
			u_stream.next_in = APFS_data;
			u_stream.avail_in = sizeof(APFS_data);
			uuid_or_partid_str = LF_CSP_APFS_UUID;
			part_scheme = "GUID";
			break;

		case FAT32:
			u_stream.next_in = FAT32_data;
			u_stream.avail_in = sizeof(FAT32_data);
			uuid_or_partid_str = LF_CSP_FAT32_PART_SCHEME;
			part_scheme = "FDisk";
			break;

		case EXFAT:
			u_stream.next_in = EXFAT_data;
			u_stream.avail_in = sizeof(EXFAT_data);
			uuid_or_partid_str = LF_CSP_EXFAT_PART_SCHEME;
			part_scheme = "FDisk";
			break;

		default:
			assert_fail("passed unkown file-system type\n");
	}

	do {
		ssize_t bytes_to_write;

		u_stream.next_out = uncompressed_out_buf;
		u_stream.avail_out = chunk_size;

		ret = inflate(&u_stream, Z_NO_FLUSH);
		assert(ret != Z_STREAM_ERROR);

		bytes_to_write = chunk_size - u_stream.avail_out;
		assert(write(fd, uncompressed_out_buf, bytes_to_write) ==
				(ssize_t)bytes_to_write);
	} while (ret == Z_OK);
	assert(ret == Z_STREAM_END);
	(void)inflateEnd(&u_stream);

	//
	// Update the disk image path.
	//
	di->path = strdup(path);

	//
	// Attach the created disk image directly to the DiskImage driver.
	//
	attach_args[0] = "hdik";
	attach_args[1] = "-nomount";
	attach_args[2] = (char *)di->path;
	attach_args[3] = NULL;

	assert_no_err(pipe(fds));
	ret = posix_spawn_file_actions_init(&actions);
	if (ret) {
		assert_fail("Failed to init file actions, error %d\n", ret);
	}

	ret = posix_spawn_file_actions_adddup2(&actions, fds[1],
			STDOUT_FILENO);
	if (ret) {
		assert_fail("Failed to adddup file action, error %d\n",
				ret);
	}

	assert_no_err(posix_spawn(&pid, LF_CS_PLUGIN_PATH_TO_HDIK, &actions,
				NULL, attach_args, NULL));

	(void)posix_spawn_file_actions_destroy(&actions);
	(void)close(fds[1]);

	//
	// Read the output from `hdik` and populate the disk image's dev node
	// after it is attached to DiskImage driver.
	//
	di->slice = NULL;
	di->disk = NULL;
	line = NULL;
	lnsz = 64;
	line = malloc(lnsz);
	assert(line);
	fp = fdopen(fds[0], "r");
	while (getline(&line, &lnsz, fp) != -1) {
		char *disk_path, *uuid_or_partid;

		disk_path = strtok(line, " ");
		assert(disk_path);

		uuid_or_partid = strtok(NULL, " ");
		assert(uuid_or_partid);

		if (strstr(uuid_or_partid, part_scheme))
			di->disk = strdup(disk_path);
		else if (strstr(uuid_or_partid, uuid_or_partid_str))
			di->slice = strdup(disk_path);
	}

	assert_with_errno(ignore_eintr(waitpid(pid, &status, 0), -1) == pid);
	assert(WIFEXITED(status) && !WEXITSTATUS(status));

	assert(di->disk && di->slice);
	free(line);
	fclose(fp);

	//
	// Place this attached image in the cleanup list so that it can be
	// disattached after the test is run.
	//
	test_cleanup(^ bool { return lf_csp_disk_image_cleanup(di); });

	//
	// Restore back the old uid.
	//
	assert_no_err(seteuid(uid_old));
	return di;
}

int
run_lf_cs_plugin(__unused test_ctx_t *ctx)
{
	lf_csp_disk_image_t *di;
	char *tester_path;

	assert(asprintf(&tester_path, "%s/livefiles_cs_tester",
				LF_CS_PLUGIN_INSTALL_PATH) > 0);

	//
	// Call livefiles Apple_CoreStorage plugin with all our file-system
	// disk-images.
	//
	// Kindly NOTE: We don't explicitly free the disk-images after use
	// because preparing the disk image with lf_csp_disk_image_create() we
	// place the image in a cleanup list and they get cleaned up after
	// test, regardless of the test failures.
	//
	// Test livefiles Apple_CoreStorage plugin with AFPS disk image.
	//
	di = lf_csp_disk_image_create(LF_CS_PLUGIN_TEST_DMG, APFS);
	assert(!systemx(tester_path, "APFS", di->slice, NULL));

	//
	// Test livefiles Apple_CoreStorage plugin with HFS+ disk image.
	//
	di = lf_csp_disk_image_create(LF_CS_PLUGIN_TEST_DMG, JHFS);
	assert(!systemx(tester_path, "JHFS", di->slice, NULL));

	//
	// Test livefiles Apple_CoreStorage plugin with EXFAT disk image.
	//
	di = lf_csp_disk_image_create(LF_CS_PLUGIN_TEST_DMG, EXFAT);
	assert(!systemx(tester_path, "EXFAT", di->slice, NULL));

	//
	// Test livefiles Apple_CoreStorage plugin with FAT32 disk image.
	//
	di = lf_csp_disk_image_create(LF_CS_PLUGIN_TEST_DMG, FAT32);
	assert(!systemx(tester_path, "FAT32", di->slice, NULL));

	free(tester_path);
	return 0;
}
#endif /* TARGET_OS_IOS */
