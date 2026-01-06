#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <pthread/private.h>

#include "darwintest_defaults.h"

#include "../src/pthread_cwd.c"

// /tmp is a symlink, so use full path for strict compare
#define WORKDIR		"/private/var/tmp/ptwork"
#define	WORKDIR1	WORKDIR "/one"
#define	WORKDIR2	WORKDIR "/two"

/*
 * This is a slow routine, just like getcwd(); people should remember that
 * they set something, instead of asking us what they told us.
 */
static char *
pthread_getcwd_np(char *buf, size_t size)
{
	int fd_cwd;

	if (buf == NULL)
		return (NULL);

	/*
	 * Open the "current working directory"; if we are running on a per
	 * thread working directory, that's the one we will get.
	 */
	if ((fd_cwd = open(".", O_RDONLY)) == -1)
		return (NULL);

	/*
	 * Switch off the per thread current working directory, in case we
	 * were on one; this fails if we aren't running with one.
	 */
	if (pthread_fchdir_np( -1) == -1) {
		/* We aren't runniing with one... alll done. */
		close (fd_cwd);
		return (NULL);
	}

	/*
	 * If we successfully switched off, then we switch back...
	 * this may fail catastrophically, if we no longer have rights;
	 * this should never happen, but threads may clobber our fd out
	 * from under us, etc..
	 */
	if (pthread_fchdir_np(fd_cwd) == -1) {
		close(fd_cwd);
		errno = EBADF;	/* sigil for catastrophic failure */
		return (NULL);
	}

	/* Close our directory handle */
	close(fd_cwd);

	/*
	 * And call the regular getcwd(), which will return the per thread
	 * current working directory instead of the process one.
	 */
	return getcwd(buf, size);
}

T_DECL(pthread_cwd, "per-thread working directory")
{
	char buf[MAXPATHLEN];

	T_SETUPBEGIN;

	T_ASSERT_EQ(pthread_fchdir_np(-1), -1, "test should not start with per-thread cwd");

	/* Blow the umask to avoid shooting our foot */
	umask(0);		/* "always successful" */

	/* Now set us up the test directories... */
	T_WITH_ERRNO; T_ASSERT_TRUE(mkdir(WORKDIR, 0777) != -1 || errno == EEXIST, NULL);
	T_WITH_ERRNO; T_ASSERT_TRUE(mkdir(WORKDIR1, 0777) != -1 || errno == EEXIST, NULL);
	T_WITH_ERRNO; T_ASSERT_TRUE(mkdir(WORKDIR2, 0777) != -1 || errno == EEXIST, NULL);

	T_SETUPEND;

	T_LOG("start in " WORKDIR1);
	T_ASSERT_POSIX_SUCCESS(chdir(WORKDIR1), NULL);
	T_ASSERT_EQ_STR(WORKDIR1, getcwd(buf, MAXPATHLEN), NULL);
	T_ASSERT_NULL(pthread_getcwd_np(buf, MAXPATHLEN), "pthread_getcwd_np should return NULL without per-thread cwd, got: %s", buf);

	T_LOG("move per-thread CWD to " WORKDIR2);
	T_ASSERT_POSIX_SUCCESS(pthread_chdir_np(WORKDIR2), NULL);
	T_ASSERT_EQ_STR(WORKDIR2, getcwd(buf, MAXPATHLEN), NULL);
	T_ASSERT_EQ_STR(WORKDIR2, pthread_getcwd_np(buf, MAXPATHLEN), NULL);

	T_LOG("unset per-thread CWD and confirm things go back");
	T_ASSERT_POSIX_SUCCESS(pthread_fchdir_np(-1), NULL);
	T_ASSERT_NULL(pthread_getcwd_np(buf, MAXPATHLEN), "pthread_getcwd_np should return NULL after reseting per-thread cwd, got: %s", buf);
	T_ASSERT_EQ_STR(WORKDIR1, getcwd(buf, MAXPATHLEN), NULL);
}
