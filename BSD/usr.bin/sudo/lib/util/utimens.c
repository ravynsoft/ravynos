/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2015, 2018 Todd C. Miller <Todd.Miller@sudo.ws>
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

#if !defined(HAVE_FUTIMENS) || !defined(HAVE_UTIMENSAT)

#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>
#include <time.h>
#if !defined(HAVE_UTIMES) || defined(HAVE_FUTIME)
# include <utime.h>
#endif

#include <sudo_compat.h>
#include <sudo_util.h>

#if !defined(HAVE_FUTIMES) && defined(HAVE_FUTIMESAT)
# define futimes(_f, _tv)	futimesat(_f, NULL, _tv)
# define HAVE_FUTIMES
#endif

#if defined(HAVE_ST_MTIM)
# ifdef HAVE_ST__TIM
#  define ATIME_TO_TIMEVAL(_x, _y)	TIMESPEC_TO_TIMEVAL((_x), &(_y)->st_atim.st__tim)
#  define MTIME_TO_TIMEVAL(_x, _y)	TIMESPEC_TO_TIMEVAL((_x), &(_y)->st_mtim.st__tim)
# else
#  define ATIME_TO_TIMEVAL(_x, _y)	TIMESPEC_TO_TIMEVAL((_x), &(_y)->st_atim)
#  define MTIME_TO_TIMEVAL(_x, _y)	TIMESPEC_TO_TIMEVAL((_x), &(_y)->st_mtim)
# endif
#elif defined(HAVE_ST_MTIMESPEC)
# define ATIME_TO_TIMEVAL(_x, _y)	TIMESPEC_TO_TIMEVAL((_x), &(_y)->st_atimespec)
# define MTIME_TO_TIMEVAL(_x, _y)	TIMESPEC_TO_TIMEVAL((_x), &(_y)->st_mtimespec)
#elif defined(HAVE_ST_NMTIME)
# define ATIME_TO_TIMEVAL(_x, _y)      do { (_x)->tv_sec = (_y)->st_atime; (_x)->tv_usec = (_y)->st_natime; } while (0)
# define MTIME_TO_TIMEVAL(_x, _y)      do { (_x)->tv_sec = (_y)->st_mtime; (_x)->tv_usec = (_y)->st_nmtime; } while (0)
#else
# define ATIME_TO_TIMEVAL(_x, _y)      do { (_x)->tv_sec = (_y)->st_atime; (_x)->tv_usec = 0; } while (0)
# define MTIME_TO_TIMEVAL(_x, _y)      do { (_x)->tv_sec = (_y)->st_mtime; (_x)->tv_usec = 0; } while (0)
#endif /* HAVE_ST_MTIM */

/*
 * Convert the pair of timespec structs passed to futimens() / utimensat()
 * to a pair of timeval structs, handling UTIME_OMIT and UTIME_NOW.
 * Returns 0 on success and -1 on failure (setting errno).
 */
static int
utimens_ts_to_tv(int fd, const char *file, const struct timespec *ts,
    struct timeval *tv)
{
    TIMESPEC_TO_TIMEVAL(&tv[0], &ts[0]);
    TIMESPEC_TO_TIMEVAL(&tv[1], &ts[1]);
    if (ts[0].tv_nsec == UTIME_OMIT || ts[1].tv_nsec == UTIME_OMIT) {
	struct stat sb;

	if (fd != -1) {
	    /* For futimens() */
	    if (fstat(fd, &sb) == -1)
		return -1;
	} else {
	    /* For utimensat() */
	    if (stat(file, &sb) == -1)
		return -1;
	}
	if (ts[0].tv_nsec == UTIME_OMIT)
	    ATIME_TO_TIMEVAL(&tv[0], &sb);
	if (ts[1].tv_nsec == UTIME_OMIT)
	    MTIME_TO_TIMEVAL(&tv[1], &sb);
    }
    if (ts[0].tv_nsec == UTIME_NOW || ts[1].tv_nsec == UTIME_NOW) {
	struct timeval now;

	if (gettimeofday(&now, NULL) == -1)
	    return -1;
	if (ts[0].tv_nsec == UTIME_NOW)
	    tv[0] = now;
	if (ts[1].tv_nsec == UTIME_NOW)
	    tv[1] = now;
    }
    return 0;
}

#if defined(HAVE_FUTIMES)
/*
 * Emulate futimens() via futimes()
 */
int
sudo_futimens(int fd, const struct timespec *ts)
{
    struct timeval tv[2], *times = NULL;

    if (ts != NULL) {
	if (utimens_ts_to_tv(fd, NULL, ts, tv) == -1)
	    return -1;
	times = tv;
    }
    return futimes(fd, times);
}
#elif defined(HAVE_FUTIME)
/*
 * Emulate futimens() via futime()
 */
int
sudo_futimens(int fd, const struct timespec *ts)
{
    struct utimbuf utb, *times = NULL;

    if (ts != NULL) {
	struct timeval tv[2];

	if (utimens_ts_to_tv(fd, NULL, ts, tv) == -1)
	    return -1;
	utb.actime = (time_t)(tv[0].tv_sec + tv[0].tv_usec / 1000000);
	utb.modtime = (time_t)(tv[1].tv_sec + tv[1].tv_usec / 1000000);
	times = &utb;
    }
    return futime(fd, times);
}
#else
/*
 * Nothing to do but fail.
 */
int
sudo_futimens(int fd, const struct timespec *ts)
{
    errno = ENOSYS;
    return -1;
}
#endif /* HAVE_FUTIMES */

#if defined(HAVE_UTIMES)
/*
 * Emulate utimensat() via utimes()
 */
int
sudo_utimensat(int fd, const char *file, const struct timespec *ts, int flag)
{
    struct timeval tv[2], *times = NULL;

    if (fd != AT_FDCWD || flag != 0) {
	errno = ENOTSUP;
	return -1;
    }

    if (ts != NULL) {
	if (utimens_ts_to_tv(-1, file, ts, tv) == -1)
	    return -1;
	times = tv;
    }
    return utimes(file, times);
}
#else
/*
 * Emulate utimensat() via utime()
 */
int
sudo_utimensat(int fd, const char *file, const struct timespec *ts, int flag)
{
    struct utimbuf utb, *times = NULL;

    if (fd != AT_FDCWD || flag != 0) {
	errno = ENOTSUP;
	return -1;
    }

    if (ts != NULL) {
	struct timeval tv[2];

	if (utimens_ts_to_tv(-1, file, ts, tv) == -1)
	    return -1;
	utb.actime = (time_t)(tv[0].tv_sec + tv[0].tv_usec / 1000000);
	utb.modtime = (time_t)(tv[1].tv_sec + tv[1].tv_usec / 1000000);
	times = &utb;
    }
    return utime(file, times);
}
#endif /* !HAVE_UTIMES */

#endif /* !HAVE_FUTIMENS && !HAVE_UTIMENSAT */
