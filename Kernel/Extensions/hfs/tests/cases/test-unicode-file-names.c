#include <glob.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <spawn.h>

#include "hfs-tests.h"
#include "test-utils.h"
#include "disk-image.h"

TEST(unicode_file_names)

static disk_image_t *di;

int run_unicode_file_names(__unused test_ctx_t *ctx)
{
	di = disk_image_get();
	char *dir;
	
	asprintf(&dir, "%s/unicode-file-names", di->mount_point);

	pid_t pid;
	assert_no_err(posix_spawn(&pid, "/bin/rm", NULL, NULL,
							  (char *[]){ "rm", "-rf", dir, NULL }, NULL));
	int stat;
	assert_with_errno(waitpid(pid, &stat, 0) == pid);

	assert_no_err(mkdir(dir, 0777));

	char *path;
	asprintf(&path, "%s/file-\xcd\x80\xcd\x81\xcd\x82\xcd\x83\xcd\x84\xcd\x85\xcd\x86\xcd\x87\xcd\x88\xcd\x89\xcd\x90", dir);
	
	int fd;
	assert_with_errno((fd = open(path, O_RDWR | O_CREAT, 0666)) >= 0);

	assert_no_err(close(fd));

	/*
	 * Create a hard link (so that we can force the cache to
	 * be purged).
	 */
	char *path2;
	asprintf(&path2, "%s/hard-link", dir);
	assert_no_err(link(path, path2));

	glob_t gl;

	assert_no_err(chdir(dir));

	assert_no_err(glob("*", 0, NULL, &gl));

	int i;
	for (i = 0; i < gl.gl_matchc; ++i) {
		if (!strncmp(gl.gl_pathv[i], "file", 4))
			goto found;
	}

	assert_fail("could not find file!");

 found:

	// Paths should be different: gl_pathv should be normalised
	assert(strcmp(gl.gl_pathv[i], path) != 0);

	char *path3;
	asprintf(&path3, "%s/%s", dir, gl.gl_pathv[i]);

	assert((fd = open(path3, O_RDONLY)) >= 0);

	assert_no_err(unlink(path3));
	assert_no_err (close(fd));

	free(dir);
	free(path);
	free(path2);
	free(path3);

	return 0;
}
