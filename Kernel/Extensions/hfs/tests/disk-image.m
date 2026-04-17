//
//  disk-image.m
//  hfs
//
//  Created by Chris Suter on 8/12/15.
//
//

#include <unistd.h>
#include <spawn.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <zlib.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdbool.h>

#include <Foundation/Foundation.h>
#include <TargetConditionals.h>

#include "disk-image.h"
#include "test-utils.h"
#include "systemx.h"

#define RETRY_MAX 3

#if (TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR)

#include "dmg.dat"

bool disk_image_cleanup(disk_image_t *di)
{
	pid_t pid;
	bool result = false;
	int eject_retry;
	int status;
	
	// We need to be root
	assert(seteuid(0) == 0);
	
	char *umount_args[]
		= { "umount", "-f", (char *)di->mount_point, NULL };
	
	assert_no_err(posix_spawn(&pid, "/sbin/umount", NULL, NULL, umount_args, NULL));
	
	waitpid(pid, &status, 0);
	
	char *detach_args[]
		= { "hdik", "-e", (char *)di->disk, NULL };

	posix_spawn_file_actions_t facts;
	posix_spawn_file_actions_init(&facts);
	posix_spawn_file_actions_addopen(&facts, STDOUT_FILENO, "/dev/null", O_APPEND, 0);
	posix_spawn_file_actions_addopen(&facts, STDERR_FILENO, "/dev/null", O_APPEND, 0);
	
	for (eject_retry = 0; eject_retry < RETRY_MAX; ++eject_retry) {
		if (!posix_spawn(&pid, "/usr/sbin/hdik", &facts, NULL, detach_args, NULL)) {
			waitpid(pid, &status, 0);
			if (WIFEXITED(status) && !WEXITSTATUS(status))
				break;
		}
		sleep(1);
	}
	
	posix_spawn_file_actions_destroy(&facts);
	
	assert(eject_retry != RETRY_MAX);
	
	struct stat sb;
	
	if (stat(di->disk, &sb) == -1 && errno == ENOENT) {
		unlink(di->path);
		result = true;
		// We are the last user of di, so free it.
		free(di->mount_point);
		free(di->disk);
		free(di->path);
		free(di);
	}

	return result;
}

void *zalloc(__unused void *opaque, uInt items, uInt size)
{
	return malloc(items * size);
}

void zfree(__unused void *opaque, void *ptr)
{
	free(ptr);
}

disk_image_t *disk_image_create(const char *path, disk_image_opts_t *opts)
{
	disk_image_t *di;
	
	di = calloc(1, sizeof(disk_image_t));
	assert(di);
	
	// We need to be root
	uid_t uid_old;
	if ((uid_old = geteuid()) != 0) {
		assert_no_err(seteuid(0));
	}
	
	// Extract the image
	int fd = open(path, O_RDWR | O_TRUNC | O_CREAT, 0666);
	
	z_stream zs = {
		.zalloc = zalloc,
		.zfree = zfree,
	};
	
	inflateInit(&zs);
	
	size_t buf_size = 1024 * 1024;
	void *out_buf = malloc(buf_size);
	assert(out_buf);
	
	zs.next_in = data;
	zs.avail_in = sizeof(data);
	
	int ret;
	
	do {
		zs.next_out = out_buf;
		zs.avail_out = (uInt)buf_size;
		
		ret = inflate(&zs, 0);
		
		size_t todo = buf_size - zs.avail_out;
		
		assert(write(fd, out_buf, todo) == (ssize_t)todo);
	} while (ret == Z_OK);
	
	assert(ret == Z_STREAM_END);
	
	di->path = strdup(path);
	
	// Attach it
	pid_t pid;
	char *attach_args[4] = { "hdik", "-nomount", (char *)di->path, NULL };
	int fds[2];
	
	assert_no_err(pipe(fds));
	
	posix_spawn_file_actions_t actions;
	posix_spawn_file_actions_init(&actions);
	posix_spawn_file_actions_adddup2(&actions, fds[1], STDOUT_FILENO);
	
	assert_no_err(posix_spawn(&pid, "/usr/sbin/hdik", &actions, NULL, attach_args, NULL));
	
	posix_spawn_file_actions_destroy(&actions);
	
	close(fds[1]);
	
	char *line, *slice = NULL;
	size_t lnsz = 64;
	FILE *fp = fdopen(fds[0], "r");
	
	line = malloc(lnsz);
	assert(line);
	
	while (getline(&line, &lnsz, fp) != -1) {
		char *first, *second;
		
		first = strtok(line, " ");
		assert(first);
		
		second = strtok(NULL, " ");
		assert(second);
		
		if (strstr(second, "GUID"))
			di->disk = strdup(first);
		
		// The output of hdik gets truncated, so just search for the leading part of the UUID
		else if (strstr(second, "48465300-0000-11AA"))
			slice = strdup(first);
	}
	
	int status;
	assert_with_errno(ignore_eintr(waitpid(pid, &status, 0), -1) == pid);
	assert(WIFEXITED(status) && !WEXITSTATUS(status));
	
	assert(di->disk && slice);
	free(line);
	fclose(fp);
	
	// Ensure we have a mount point 
	assert(opts->mount_point);

	// Mount it
	char *mkdir_args[4] = { "mkdir", "-p", (char *)opts->mount_point, NULL };
	assert_no_err(posix_spawn(&pid, "/bin/mkdir", NULL, NULL, mkdir_args, NULL));
	
	assert_with_errno(ignore_eintr(waitpid(pid, &status, 0), -1) == pid);
	assert(WIFEXITED(status) && !WEXITSTATUS(status));
	
	posix_spawn_file_actions_t facts;
	posix_spawn_file_actions_init(&facts);
	posix_spawn_file_actions_addopen(&facts, STDOUT_FILENO, "/dev/null", O_APPEND, 0);
	posix_spawn_file_actions_addopen(&facts, STDERR_FILENO, "/dev/null", O_APPEND, 0);
	
	char *mount_args[4] = { "mount", slice, (char *)opts->mount_point, NULL };
	assert_no_err(posix_spawn(&pid, "/sbin/mount_hfs", &facts, NULL, mount_args, NULL));
	
	posix_spawn_file_actions_destroy(&facts);
	free(slice);
	
	assert_with_errno(ignore_eintr(waitpid(pid, &status, 0), -1) == pid);
	assert(WIFEXITED(status) && !WEXITSTATUS(status));
	
	di->mount_point = strdup(opts->mount_point);
	
	if (strcmp(path, SHARED_PATH)) { // Don't register a cleanup for the shared image
		test_cleanup(^ bool {
			return disk_image_cleanup(di);
		});
	}
	
	assert_no_err(seteuid(uid_old));
	
	return di;
}

disk_image_t *disk_image_get(void)
{	
	disk_image_t *di;
	struct statfs sfs;
	
	if (statfs(SHARED_MOUNT, &sfs) == 0) {
		di = calloc(1, sizeof(*di));
		di->mount_point = SHARED_MOUNT;
		di->disk = strdup(sfs.f_mntfromname);
		di->path = SHARED_PATH;

		// Make sure the di struct is freed when tests are complete.
		test_cleanup(^ bool {
			free(di->disk);
			free(di);
			return true;
		});
	} else {
		disk_image_opts_t opts = {
			.mount_point = SHARED_MOUNT
		};
		di = disk_image_create(SHARED_PATH, &opts);
		// Per the contract of disk_image_create(),
		// di will be freed when disk_image_cleanup() is called,
		// so don't free it here.
	}
	
	return di;
}

#else // !(TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR)

bool disk_image_cleanup(disk_image_t *di)
{
	char *detach_args[]
		= { "hdiutil", "detach", (char *)di->disk, "-force", NULL };

	pid_t pid;
	bool result = false;
	int eject_retry;
	int status;

	posix_spawn_file_actions_t facts;
	posix_spawn_file_actions_init(&facts);
	posix_spawn_file_actions_addopen(&facts, STDOUT_FILENO, "/dev/null", O_APPEND, 0);
	posix_spawn_file_actions_addopen(&facts, STDERR_FILENO, "/dev/null", O_APPEND, 0);

	for (eject_retry = 0; eject_retry < RETRY_MAX; ++eject_retry) {
		if (!posix_spawn(&pid, "/usr/bin/hdiutil", &facts, NULL, detach_args, NULL)) {
			waitpid(pid, &status, 0);
			if (WIFEXITED(status) && !WEXITSTATUS(status)) {
				break;
			}
		}
		sleep(1);
	}

	posix_spawn_file_actions_destroy(&facts);

	assert(eject_retry != RETRY_MAX);

	struct stat sb;

	if (stat(di->disk, &sb) == -1 && errno == ENOENT) {
		if (unlink(di->path) && errno == EACCES && !seteuid(0))
			unlink(di->path);
		result = true;

		// We are the last user of di, so free it.
		free(di->mount_point);
		free(di->disk);
		free(di->path);
		free(di);
	}

	return result;
}

disk_image_t *disk_image_create(const char *path, disk_image_opts_t *opts)
{
	pid_t pid;
	char sz[32];
	sprintf(sz, "%llu", opts->size);
	
	if (opts->mount_point) {
		assert(!systemx("/bin/mkdir", SYSTEMX_QUIET, "-p", opts->mount_point, NULL));
	}

	// Start with the basic args
	char *args[64] = { "hdiutil", "create", (char *)path, "-size", sz, "-ov" };

	if (opts && opts->partition_type) {
		args[6] = "-partitionType";
		args[7] = (char *)opts->partition_type;
		args[8] = NULL;

		posix_spawn_file_actions_t facts;
		posix_spawn_file_actions_init(&facts);
		posix_spawn_file_actions_addopen(&facts, STDOUT_FILENO, "/dev/null", O_APPEND, 0);

		assert_no_err(posix_spawn(&pid, "/usr/bin/hdiutil", &facts, NULL,
								  args, NULL));

		posix_spawn_file_actions_destroy(&facts);

		int status;
		assert_with_errno(ignore_eintr(waitpid(pid, &status, 0), -1));

		assert(WIFEXITED(status) && !WEXITSTATUS(status));

		args[1] = "attach";
		// args[2] == path
		args[3] = "-nomount";
		args[4] = "-plist";
		args[5] = NULL;
	} else if (opts && opts->enable_owners) {
		args[6] = "-fs";
		args[7] = "HFS+J";
		args[8] = NULL;

		posix_spawn_file_actions_t facts;
		posix_spawn_file_actions_init(&facts);
		posix_spawn_file_actions_addopen(&facts, STDOUT_FILENO, "/dev/null", O_APPEND, 0);

		assert_no_err(posix_spawn(&pid, "/usr/bin/hdiutil", &facts, NULL,
								  args, NULL));

		posix_spawn_file_actions_destroy(&facts);

		int status;
		assert_with_errno(ignore_eintr(waitpid(pid, &status, 0), -1));

		assert(WIFEXITED(status) && !WEXITSTATUS(status));

		args[1] = "attach";
		// args[2] == path
		args[3] = "-plist";
		args[4] = "-owners";
		args[5] = "on";
		if (opts->mount_point) {
			args[6] = "-mountpoint";
			args[7] = (char *)opts->mount_point;
			args[8] = NULL;
		}
		else
			args[6] = NULL;
	} else {
		args[6] = "-fs";
		args[7] = "HFS+J";
		args[8] = NULL;
		
		posix_spawn_file_actions_t facts;
		posix_spawn_file_actions_init(&facts);
		posix_spawn_file_actions_addopen(&facts, STDOUT_FILENO, "/dev/null", O_APPEND, 0);
		
		assert_no_err(posix_spawn(&pid, "/usr/bin/hdiutil", &facts, NULL,
								  args, NULL));
		
		posix_spawn_file_actions_destroy(&facts);
		
		int status;
		assert_with_errno(ignore_eintr(waitpid(pid, &status, 0), -1));
		
		assert(WIFEXITED(status) && !WEXITSTATUS(status));
		
		args[1] = "attach";
		// args[2] == path
		args[3] = "-plist";
		if (opts->mount_point) {
			args[4] = "-mountpoint";
			args[5] = (char *)opts->mount_point;
			args[6] = NULL;
		}
		else
			args[4] = NULL;
	}

	int fds[2];
	assert_no_err(pipe(fds));

	posix_spawn_file_actions_t actions;
	posix_spawn_file_actions_init(&actions);
	posix_spawn_file_actions_adddup2(&actions, fds[1], STDOUT_FILENO);

	assert_no_err(posix_spawn(&pid, "/usr/bin/hdiutil", &actions, NULL, args, NULL));

	posix_spawn_file_actions_destroy(&actions);

	close(fds[1]);

	char buffer[4096];
	size_t amt = 0;

	for (;;) {
		ssize_t res = read(fds[0], buffer + amt, 4096 - amt);

		if (!res)
			break;

		if (res == -1 && errno == EINTR)
			continue;

		assert_with_errno(res > 0);

		amt += res;

		assert(amt < 4096);
	}

	disk_image_t *di = calloc(1, sizeof(*di));

	di->path = strdup(path);

	@autoreleasepool {
		NSDictionary *results
			= [NSPropertyListSerialization propertyListWithData:
			   [NSData dataWithBytesNoCopy:buffer
									length:amt
							  freeWhenDone:NO]
														options:0
														 format:NULL
														  error:NULL];
		
		for (NSDictionary *entity in results[@"system-entities"]) {
			if (opts && opts->partition_type) {
				if (!strcmp([entity[@"unmapped-content-hint"] UTF8String],
							opts->partition_type)
					|| !strcmp([entity[@"content-hint"] UTF8String],
							   opts->partition_type)) {
					di->disk = strdup([entity[@"dev-entry"] fileSystemRepresentation]);
					break;
				}
			} else if ([entity[@"content-hint"] isEqualToString:@"Apple_HFS"]) {
				di->mount_point = strdup([entity[@"mount-point"] fileSystemRepresentation]);
				di->disk = strdup([entity[@"dev-entry"] fileSystemRepresentation]);
				break;
			}
		}
	}

	int status;
	assert_with_errno(ignore_eintr(waitpid(pid, &status, 0), -1) == pid);
	assert(WIFEXITED(status) && !WEXITSTATUS(status));
	
	assert(di->disk);

	if (strcmp(path, SHARED_PATH)) { // Don't register a cleanup for the shared image
		test_cleanup(^ bool {
			return disk_image_cleanup(di);
		});
	}

	return di;
}

disk_image_t *disk_image_get(void)
{
	disk_image_t *di;
	struct statfs sfs;
	
	if (statfs(SHARED_MOUNT, &sfs) == 0) {
		di = calloc(1, sizeof(*di));
		
		di->mount_point = SHARED_MOUNT;
		di->disk = strdup(sfs.f_mntfromname);
		di->path = SHARED_PATH;

		// Make sure the di struct is freed when tests are complete.
		test_cleanup(^ bool {
			free(di->disk);
			free(di);
			return true;
		});
	} else {
		disk_image_opts_t opts = {
			.size = 4 GB,
			.mount_point = SHARED_MOUNT
		};
		di = disk_image_create(SHARED_PATH, &opts);
		// Per the contract of disk_image_create(),
		// di will be freed when disk_image_cleanup() is called,
		// so don't free it here.
	}

	return di;
}

#endif // (TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR)
