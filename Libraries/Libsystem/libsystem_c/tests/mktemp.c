#include <fcntl.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <System/sys/content_protection.h>
#include <TargetConditionals.h>
#include <unistd.h>

#include <darwintest.h>

#define ALLOWED_MKOSTEMP_FLAGS (O_APPEND | O_SHLOCK | O_EXLOCK | O_CLOEXEC)
#define FCNTL_GETFL_EXPOSED_OFLAGS (O_APPEND)

#define TEMPLATE_BASE "/tmp/mktemp_test."
#define TEMPLATE_XS "XXXXXXXX"
static const char template[] = TEMPLATE_BASE TEMPLATE_XS;

static void test_mkostemp(int oflags);

T_DECL(mkstemp, "basic mkstemp test")
{
	char path[sizeof(template)];
	struct stat path_stat;
	struct stat fd_stat;

	memcpy(path, template, sizeof(template));

	int fd = mkstemp(path);
	T_ASSERT_POSIX_SUCCESS(fd,
			"mkstemp must return a valid fd");
	T_ASSERT_TRUE(memcmp(path, TEMPLATE_BASE, strlen(TEMPLATE_BASE)) == 0,
			"mkstemp must respect the template. template: %s, got: %s",
			template, path);

	// stat fd and path, compare those
	T_ASSERT_POSIX_ZERO(stat(path, &path_stat),
			"must be able to stat the path");
	T_ASSERT_POSIX_ZERO(fstat(fd, &fd_stat),
			"must be able to fstat the fd");
	T_ASSERT_TRUE(
			(path_stat.st_dev == fd_stat.st_dev) &&
			(path_stat.st_ino == fd_stat.st_ino),
			"fd does not match the file path");
	// verify file attributes
	T_ASSERT_TRUE(S_ISREG(path_stat.st_mode),
			"the path must point to a regular file");
	T_ASSERT_EQ(0600, (path_stat.st_mode & 0777),
			"created file must have 0600 permissions");
	T_ASSERT_EQ_LLONG(0LL, path_stat.st_size,
			"created file must be empty");
	// unlink and stat again
	T_ASSERT_POSIX_ZERO(unlink(path),
			"must be able to unlink the created file");
	T_ASSERT_POSIX_ZERO(fstat(fd, &fd_stat),
			"must be able to stat the fd after unlink");
	T_ASSERT_EQ(0, fd_stat.st_nlink,
			"must not have any hard links to the file after unlink");
	// close
	T_ASSERT_POSIX_ZERO(close(fd),
			"must be able to close the fd");
}

T_DECL(two_mkstemp_calls, "two mkstemp calls return different paths and fds")
{
	char path1[sizeof(template)];
	char path2[sizeof(template)];
	memcpy(path1, template, sizeof(path1));
	memcpy(path2, template, sizeof(path2));

	int fd1 = mkostemp(path1, 0);
	T_ASSERT_POSIX_SUCCESS(fd1, "mkostemp must return a valid fd");

	int fd2 = mkostemp(path2, 0);
	T_ASSERT_POSIX_SUCCESS(fd2, "mkostemp must return a valid fd");

	T_ASSERT_NE(fd1, fd2,
			"two mkostemp calls must return different fds");
	T_ASSERT_NE_STR(path1, path2,
			"two mkostemp calls must return different paths");

	T_EXPECT_POSIX_ZERO(unlink(path1),
			"unlink must succeed for the first file");
	T_EXPECT_POSIX_ZERO(unlink(path2),
			"unlink must succeed for the second file");
	T_EXPECT_POSIX_ZERO(close(fd1),
			"close must succeed for the first fd");
	T_EXPECT_POSIX_ZERO(close(fd2),
			"close must succeed for the second fd");
}

T_DECL(mktemp, "basic mktemp test")
{
	char path[sizeof(template)];

	memcpy(path, template, sizeof(template));

	T_ASSERT_EQ_PTR((void *) mktemp(path), (void *) path,
			"mktemp must return its argument");

	T_ASSERT_TRUE(memcmp(path, TEMPLATE_BASE, strlen(TEMPLATE_BASE)) == 0,
			"mktemp must respect the template. template: %s, got: %s",
			template, path);
}

T_DECL(mkdtemp, "basic mkdtemp test")
{
	char path[sizeof(template)];
	struct stat dstat;

	memcpy(path, template, sizeof(template));

	T_ASSERT_EQ_PTR((void *) mkdtemp(path), (void *) path,
			"mkdtemp must return its argument");
	T_ASSERT_TRUE(memcmp(path, TEMPLATE_BASE, strlen(TEMPLATE_BASE)) == 0,
			"mkdtemp must respect the template. template: %s, got: %s",
			template, path);

	// stat fd and path, compare those
	T_ASSERT_POSIX_ZERO(stat(path, &dstat),
			"must be able to stat the path");
	// verify file attributes
	T_ASSERT_TRUE(S_ISDIR(dstat.st_mode),
			"the path must point to a directory");
	T_ASSERT_EQ(0700, (dstat.st_mode & 0777),
			"created directory must have 0700 permissions");
	// cleanup
	T_ASSERT_POSIX_ZERO(rmdir(path),
			"must be able to rmdir the created directory");
}

T_DECL(mkostemp_no_flags, "mkostemp works with 0 oflags")
{
	test_mkostemp(0);
}

T_DECL(mkostemp_cloexec, "mkostemp works with O_CLOEXEC")
{
	test_mkostemp(O_CLOEXEC);
}

T_DECL(mkostemp_append, "mkostemp works with O_APPEND")
{
	test_mkostemp(O_APPEND);
}

T_DECL(mkostemp_all_flags, "mkostemp works with all allowed flags")
{
	test_mkostemp(ALLOWED_MKOSTEMP_FLAGS);
}

T_DECL(mkostemp_bad_flags, "mkostemp checks for disallowed flags")
{
	char path[sizeof(template)];
	memcpy(path, template, sizeof(path));

	T_ASSERT_EQ(-1, mkostemp(path, O_CREAT), "mkostemp must not allow O_CREAT");
	T_ASSERT_EQ(-1, mkostemp(path, O_NONBLOCK), "mkostemp must not allow O_NONBLOCK");
}

static void
test_mkostemp(int oflags)
{
	char path[sizeof(template)];
	int fd;

	T_LOG("testing mkostemp with oflags %#x\n", oflags);

	memcpy(path, template, sizeof(template));

	fd = mkostemp(path, oflags);

	T_ASSERT_POSIX_SUCCESS(fd,
			"mkostemp must return a valid fd");

	if (oflags & O_CLOEXEC) {
		T_ASSERT_EQ(FD_CLOEXEC, fcntl(fd, F_GETFD),
				"O_CLOEXEC must be visible through fcntl GETFD");
	} else {
		T_ASSERT_EQ(0, fcntl(fd, F_GETFD),
				"must not add fcntl GETFD flags at will");
	}

	T_ASSERT_EQ(
			(oflags & FCNTL_GETFL_EXPOSED_OFLAGS),
			(fcntl(fd, F_GETFL) & FCNTL_GETFL_EXPOSED_OFLAGS),
			"oflags must be visible through fcntl GETFL");

	T_EXPECT_POSIX_ZERO(unlink(path),
			"must be able to unlink the created file");
	T_EXPECT_POSIX_ZERO(close(fd),
			"must be able to close the fd");
}

#if TARGET_OS_IPHONE
// mkstemp_dprotected_np is marked as __OSX_UNAVAILABLE, so its usage
// has to be guarded by target conditionals. Having a testcase that
// always does T_SKIP on OS X yields another issue. The compiler starts
// demanding to mark the test function definition as noreturn. Just
// don't compile the testcase for OS X.
T_DECL(mkstemp_dprotected_np, "basic mkstemp_dprotected_np test")
{
	char path[sizeof(template)];
	int fd;
	int protection_class;
	struct statfs sfs;

	// the test requires /tmp to be HFS
	T_ASSERT_POSIX_ZERO(statfs("/tmp", &sfs), "must be able to statfs /tmp");
	if (strcmp(sfs.f_fstypename, "hfs") != 0) {
		T_SKIP("Can only test dprotected on a HFS filesystem. Got %s,"
				"skipping the test.", sfs.f_fstypename);
	}

	memcpy(path, template, sizeof(template));
	// create a file
	fd = mkstemp_dprotected_np(path, PROTECTION_CLASS_B, 0);
	T_ASSERT_POSIX_SUCCESS(fd,
			"mkstemp_dprotected_np must return a valid fd");
	T_ASSERT_TRUE(memcmp(path, TEMPLATE_BASE, strlen(TEMPLATE_BASE)) == 0,
			"mkstemp_dprotected_np must respect the template. template: %s, got: %s",
			template, path);
	// verify protection class
	protection_class = fcntl(fd, F_GETPROTECTIONCLASS);
	T_WITH_ERRNO; T_ASSERT_EQ(protection_class, PROTECTION_CLASS_B,
			"must get back the protection class from fcntl");
	// cleanup
	T_ASSERT_POSIX_ZERO(close(fd),
			"must be able to close the fd");
	T_ASSERT_POSIX_ZERO(unlink(path),
			"must be able to unlink the created file");
}
#endif
