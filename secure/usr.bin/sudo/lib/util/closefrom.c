/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2004-2005, 2007, 2010, 2012-2015, 2017-2022
 *	Todd C. Miller <Todd.Miller@sudo.ws>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 */

#include <config.h>

#ifndef HAVE_CLOSEFROM

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef HAVE_PSTAT_GETPROC
# include <sys/pstat.h>
#else
# include <dirent.h>
#endif
#ifdef HAVE_LIBPROC_H
# include <libproc.h>
#endif
#ifdef HAVE_LINUX_CLOSE_RANGE_H
# include <linux/close_range.h>
#endif

#include <sudo_compat.h>
#include <sudo_util.h>
#include <pathnames.h>

#ifndef OPEN_MAX
# define OPEN_MAX	256
#endif

/* Avoid potential libdispatch crash on macOS when we close its fds. */
#ifdef __APPLE__
# define closefrom_close(x)	fcntl((x), F_SETFD, FD_CLOEXEC)
#else
# define closefrom_close(x)	close(x)
#endif

/*
 * Close all file descriptors greater than or equal to lowfd.
 * This is the expensive (fallback) method.
 */
static void
closefrom_fallback(int lowfd)
{
    long fd, maxfd;

    /*
     * Fall back on sysconf(_SC_OPEN_MAX).  This is equivalent to
     * checking the RLIMIT_NOFILE soft limit.  It is possible for
     * there to be open file descriptors past this limit but there's
     * not much we can do about that since the hard limit may be
     * RLIM_INFINITY (LLONG_MAX or ULLONG_MAX on modern systems).
     */
    maxfd = sysconf(_SC_OPEN_MAX);
    if (maxfd < OPEN_MAX)
	maxfd = OPEN_MAX;

    /* Make sure we didn't get RLIM_INFINITY as the upper limit. */
    if (maxfd > INT_MAX)
	maxfd = INT_MAX;

    for (fd = lowfd; fd < maxfd; fd++) {
	(void)closefrom_close((int)fd);
    }
}

/*
 * Close all file descriptors greater than or equal to lowfd.
 * We try the fast way first, falling back on the slow method.
 */
void
sudo_closefrom(int lowfd)
{
#if defined(HAVE_PSTAT_GETPROC)
    struct pst_status pst;
#elif defined(HAVE_DIRFD)
    const char *path;
    DIR *dirp;
#endif
#if defined(HAVE_PROC_PIDINFO)
    struct proc_fdinfo *buf = NULL;
    const pid_t pid = getpid();
    int i, n, len;
#endif

    /* Try the fast method first, if possible. */
#if defined(HAVE_FCNTL_CLOSEM)
    if (fcntl(lowfd, F_CLOSEM, 0) != -1)
	return;
#elif defined(HAVE_CLOSE_RANGE)
    if (close_range((unsigned int)lowfd, ~0U, 0) != -1)
	return;
#elif defined(HAVE_PROC_PIDINFO)
    len = proc_pidinfo(pid, PROC_PIDLISTFDS, 0, NULL, 0);
    switch (len) {
    case 0:
	/* No open files. */
	return;
    case -1:
	/* Fall back on other methods. */
	break;
    default:
	/* Allocate space for 4 extra fds to leave some wiggle room. */
	buf = malloc(len + (PROC_PIDLISTFD_SIZE * 4));
	if (buf == NULL)
	    break;
	n = proc_pidinfo(pid, PROC_PIDLISTFDS, 0, buf, len);
	if (n == -1 || n > len) {
	    free(buf);
	    break;
	}
	n /= PROC_PIDLISTFD_SIZE;
	for (i = 0; i < n; i++) {
	    if (buf[i].proc_fd >= lowfd) {
		(void)closefrom_close(buf[i].proc_fd);
	    }
	}
	free(buf);
	return;
    }
#endif /* HAVE_PROC_PIDINFO */
#if defined(HAVE_PSTAT_GETPROC)
    /*
     * EOVERFLOW is not a fatal error for the fields we use.
     * See the "EOVERFLOW Error" section of pstat_getvminfo(3).
     */                             
    if (pstat_getproc(&pst, sizeof(pst), 0, getpid()) != -1 ||
	errno == EOVERFLOW) {
	int fd;

	for (fd = lowfd; fd <= pst.pst_highestfd; fd++)
	    (void)closefrom_close(fd);
	return;
    }
#elif defined(HAVE_DIRFD)
    /* Use /proc/self/fd (or /dev/fd on macOS) if it exists. */
# ifdef __APPLE__
    path = _PATH_DEV "fd";
# else
    path = "/proc/self/fd";
# endif
    if ((dirp = opendir(path)) != NULL) {
	struct dirent *dent;
	while ((dent = readdir(dirp)) != NULL) {
	    const char *errstr;
	    int fd = (int)sudo_strtonum(dent->d_name, lowfd, INT_MAX, &errstr);
	    if (errstr == NULL && fd != dirfd(dirp)) {
		(void)closefrom_close(fd);
	    }
	}
	(void)closedir(dirp);
	return;
    }
#endif /* HAVE_DIRFD */

    /* Do things the slow way. */
    closefrom_fallback(lowfd);
}

#endif /* HAVE_CLOSEFROM */
