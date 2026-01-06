#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include <darwintest.h>
#include <darwintest_utils.h>

T_DECL(seekdir_basic, "seekdir")
{
	const char *path = dt_tmpdir();
	// make sure there are a couple of entries in the dir aside from . and ..
	int fd = open(path, O_RDONLY | O_DIRECTORY);
	openat(fd, "a", O_CREAT | O_WRONLY, 0600);
	openat(fd, "b", O_CREAT | O_WRONLY, 0600);
	openat(fd, "c", O_CREAT | O_WRONLY, 0600);

	DIR *dirp = fdopendir(fd);
	struct dirent *entry = NULL;

	T_ASSERT_NOTNULL(dirp, NULL);

	T_ASSERT_NOTNULL(entry = readdir(dirp), NULL); // .
	T_ASSERT_NOTNULL(entry = readdir(dirp), NULL); // ..

	// we can get any entry -- no ordering
	T_ASSERT_NOTNULL(entry = readdir(dirp), NULL);
	// remember position for the second entry
	long second_pos = telldir(dirp);
	// read the second entry
	T_ASSERT_NOTNULL(entry = readdir(dirp), NULL);
	char *second_name = strdup(entry->d_name);
	T_ASSERT_NOTNULL(second_name, NULL);
	// read the third entry
	T_ASSERT_NOTNULL(entry = readdir(dirp), NULL);

	// go back to the second entry and read it
	seekdir(dirp, second_pos);
	T_ASSERT_NOTNULL(entry = readdir(dirp), NULL);

	// make sure the name matches the old copy
	T_ASSERT_EQ_STR(second_name, entry->d_name, NULL);

	// return to 2nd once again, reinitializing second_pos and re-reading
	seekdir(dirp, second_pos);
	second_pos = telldir(dirp);
	T_ASSERT_NOTNULL(entry = readdir(dirp), NULL);

	// make sure the name matches the old copy
	T_ASSERT_EQ_STR(second_name, entry->d_name, NULL);

	// verify that last pos
	seekdir(dirp, second_pos);
	T_ASSERT_NOTNULL(entry = readdir(dirp), NULL);
	T_ASSERT_EQ_STR(second_name, entry->d_name, NULL);

	free(second_name);
	T_ASSERT_POSIX_ZERO(closedir(dirp), NULL);
}

T_DECL(readdir, "readdir")
{
	const char *path = dt_tmpdir();
	int fd = open(path, O_RDONLY | O_DIRECTORY);
	openat(fd, "foobarbaz", O_CREAT | O_WRONLY, 0600);

	DIR *dirp = fdopendir(fd);
	T_ASSERT_NOTNULL(dirp, NULL);

	struct dirent *entry = NULL;
	while ((entry = readdir(dirp)) != NULL) {
		if (strcmp(entry->d_name, "foobarbaz")) {
			break;
		}
	}

	T_ASSERT_NOTNULL(entry, "found the entry");

	T_ASSERT_POSIX_ZERO(closedir(dirp), NULL);
}

T_DECL(tell_seek_tell, "tell-seek-tell returns the same location")
{
	// http://pubs.opengroup.org/onlinepubs/009695399/functions/telldir.html
	// If the most recent operation on the directory stream was a seekdir(),
	// the directory position returned from the telldir() shall be the same as
	// that supplied as a loc argument for seekdir().

	const char *path = dt_tmpdir();
	// make sure there are a couple of entries in the dir aside from . and ..
	{
		int fd = open(path, O_RDONLY | O_DIRECTORY);
		openat(fd, "a", O_CREAT | O_WRONLY, 0600);
		openat(fd, "b", O_CREAT | O_WRONLY, 0600);
		openat(fd, "c", O_CREAT | O_WRONLY, 0600);
		close(fd);
	}

	DIR *dirp = opendir(path);
	T_ASSERT_NOTNULL(dirp, NULL);
	struct dirent *entry = NULL;

	T_ASSERT_NOTNULL(entry = readdir(dirp), NULL);
	T_ASSERT_NOTNULL(entry = readdir(dirp), NULL);
	long pos1 = telldir(dirp);
	T_ASSERT_NOTNULL(entry = readdir(dirp), NULL);
	T_ASSERT_NOTNULL(entry = readdir(dirp), NULL);
	seekdir(dirp, pos1);
	long pos2 = telldir(dirp);

	T_ASSERT_EQ(pos1, pos2, NULL);

	T_ASSERT_POSIX_ZERO(closedir(dirp), NULL);
}

T_DECL(rewinddir, "rewinddir")
{
	const char *path = dt_tmpdir();
	// make sure there are a couple of entries in the dir aside from . and ..
	{
		int fd = open(path, O_RDONLY | O_DIRECTORY);
		openat(fd, "a", O_CREAT | O_WRONLY, 0600);
		openat(fd, "b", O_CREAT | O_WRONLY, 0600);
		openat(fd, "c", O_CREAT | O_WRONLY, 0600);
		close(fd);
	}

	DIR *dirp = opendir(path);
	T_ASSERT_NOTNULL(dirp, NULL);
	struct dirent *entry = NULL;

	T_ASSERT_NOTNULL(entry = readdir(dirp), NULL);
	T_ASSERT_NOTNULL(entry = readdir(dirp), NULL);
	T_ASSERT_NOTNULL(entry = readdir(dirp), NULL);
	char *third_name = strdup(entry->d_name);

	rewinddir(dirp);

	T_ASSERT_NOTNULL(entry = readdir(dirp), NULL);
	T_ASSERT_NOTNULL(entry = readdir(dirp), NULL);
	T_ASSERT_NOTNULL(entry = readdir(dirp), NULL);

	T_ASSERT_EQ_STR(third_name, entry->d_name, NULL);

	free(third_name);
	T_ASSERT_POSIX_ZERO(closedir(dirp), NULL);
}


T_DECL(rewinddir_dup, "rewinddir dup")
{
	// An older implementation of rewinddir failed to seek the fd which was
	// passed to fdopendir()

	const char *path = dt_tmpdir();
	// make sure there are a couple of entries in the dir aside from . and ..
	int fd = open(path, O_RDONLY | O_DIRECTORY);
	openat(fd, "a", O_CREAT | O_WRONLY, 0600);
	openat(fd, "b", O_CREAT | O_WRONLY, 0600);
	openat(fd, "c", O_CREAT | O_WRONLY, 0600);

	// prep an fd with a non-zero seek
	DIR *dirp = fdopendir(fd);
	T_ASSERT_NOTNULL(dirp, NULL);
	struct dirent *entry = NULL;

	T_ASSERT_NOTNULL(entry = readdir(dirp), NULL);
	T_ASSERT_NOTNULL(entry = readdir(dirp), NULL);
	T_ASSERT_NOTNULL(entry = readdir(dirp), NULL);

	// remember the entry name and dup the fd
	char *third_name = strdup(entry->d_name);
	int fd2 = dup(fd);

	T_ASSERT_POSIX_ZERO(closedir(dirp), NULL);

	dirp = fdopendir(fd2);
	// rewind back to 0
	rewinddir(dirp);

	T_ASSERT_NOTNULL(dirp, NULL);
	T_ASSERT_NOTNULL(entry = readdir(dirp), NULL);
	T_ASSERT_NOTNULL(entry = readdir(dirp), NULL);
	T_ASSERT_NOTNULL(entry = readdir(dirp), NULL);

	T_ASSERT_EQ_STR(third_name, entry->d_name, NULL);

	free(third_name);
	T_ASSERT_POSIX_ZERO(closedir(dirp), NULL);
}

static int
_select_abc(const struct dirent *entry)
{
	return strcmp(entry->d_name, "a") == 0 ||
			strcmp(entry->d_name, "b") == 0 ||
			strcmp(entry->d_name, "c") == 0;
}

T_DECL(scandir, "scandir")
{
	const char *path = dt_tmpdir();
	{
		int fd = open(path, O_RDONLY | O_DIRECTORY);
		openat(fd, "a", O_CREAT | O_WRONLY, 0600);
		openat(fd, "b", O_CREAT | O_WRONLY, 0600);
		openat(fd, "c", O_CREAT | O_WRONLY, 0600);
		close(fd);
	}

	struct dirent **entries = NULL;
	int found = scandir(path, &entries, _select_abc, alphasort);

	T_ASSERT_EQ(found, 3, NULL);

	T_ASSERT_EQ_STR(entries[0]->d_name, "a", NULL);
	T_ASSERT_EQ_STR(entries[1]->d_name, "b", NULL);
	T_ASSERT_EQ_STR(entries[2]->d_name, "c", NULL);

	free(entries[0]);
	free(entries[1]);
	free(entries[2]);
	free(entries);
}

T_DECL(scandir_b, "scandir_b")
{
	const char *path = dt_tmpdir();
	{
		int fd = open(path, O_RDONLY | O_DIRECTORY);
		openat(fd, "a", O_CREAT | O_WRONLY, 0600);
		openat(fd, "b", O_CREAT | O_WRONLY, 0600);
		openat(fd, "c", O_CREAT | O_WRONLY, 0600);
		close(fd);
	}

	const struct dirent **entries = NULL;
	int found = scandir_b(path, &entries,
			^(const struct dirent *entry) {
				return strcmp(entry->d_name, "a") == 0 ||
						strcmp(entry->d_name, "b") == 0 ||
						strcmp(entry->d_name, "c") == 0;
			},
			^(const struct dirent **d1, const struct dirent **d2) {
				return strcoll((*d1)->d_name, (*d2)->d_name);
			});

	T_ASSERT_EQ(found, 3, NULL);

	T_ASSERT_EQ_STR(entries[0]->d_name, "a", NULL);
	T_ASSERT_EQ_STR(entries[1]->d_name, "b", NULL);
	T_ASSERT_EQ_STR(entries[2]->d_name, "c", NULL);

	free(entries[0]);
	free(entries[1]);
	free(entries[2]);
	free(entries);
}
