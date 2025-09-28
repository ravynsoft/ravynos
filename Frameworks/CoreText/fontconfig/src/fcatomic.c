/*
 * fontconfig/src/fcatomic.c
 *
 * Copyright © 2002 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the author(s) not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  The authors make no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * THE AUTHOR(S) DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * fcatomic.c
 *
 * Lock cache and configuration files for atomic update
 *
 * Uses only regular filesystem calls so it should
 * work even in the absense of functioning file locking
 *
 * On Unix, four files are used:
 *	file	    - the data file accessed by other apps.
 *	new	    - a new version of the data file while it's being written
 *	lck	    - the lock file
 *	tmp	    - a temporary file made unique with mkstemp
 *
 *  Here's how it works:
 *	Create 'tmp' and store our PID in it
 *	Attempt to link it to 'lck'
 *	Unlink 'tmp'
 *	If the link succeeded, the lock is held
 *
 * On Windows, where there are no links, no tmp file is used, and lck
 * is a directory that's mkdir'ed. If the mkdir succeeds, the lock is
 * held.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "fcint.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <time.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef _WIN32
#include <direct.h>
#define mkdir(path,mode) _mkdir(path)
#endif

#define NEW_NAME	".NEW"
#define LCK_NAME	".LCK"
#define TMP_NAME	".TMP-XXXXXX"

FcAtomic *
FcAtomicCreate (const FcChar8   *file)
{
    int	    file_len = strlen ((char *) file);
    int	    new_len = file_len + sizeof (NEW_NAME);
    int	    lck_len = file_len + sizeof (LCK_NAME);
    int	    tmp_len = file_len + sizeof (TMP_NAME);
    int	    total_len = (sizeof (FcAtomic) +
			 file_len + 1 +
			 new_len + 1 +
			 lck_len + 1 +
			 tmp_len + 1);
    FcAtomic	*atomic = malloc (total_len);
    if (!atomic)
	return 0;

    atomic->file = (FcChar8 *) (atomic + 1);
    strcpy ((char *) atomic->file, (char *) file);

    atomic->new = atomic->file + file_len + 1;
    strcpy ((char *) atomic->new, (char *) file);
    strcat ((char *) atomic->new, NEW_NAME);

    atomic->lck = atomic->new + new_len + 1;
    strcpy ((char *) atomic->lck, (char *) file);
    strcat ((char *) atomic->lck, LCK_NAME);

    atomic->tmp = atomic->lck + lck_len + 1;

    return atomic;
}

FcBool
FcAtomicLock (FcAtomic *atomic)
{
    int		ret;
    struct stat	lck_stat;

#ifdef HAVE_LINK
    int		fd = -1;
    FILE	*f = 0;
    FcBool	no_link = FcFalse;

    strcpy ((char *) atomic->tmp, (char *) atomic->file);
    strcat ((char *) atomic->tmp, TMP_NAME);
    fd = FcMakeTempfile ((char *) atomic->tmp);
    if (fd < 0)
	return FcFalse;
    f = fdopen (fd, "w");
    if (!f)
    {
    	close (fd);
	unlink ((char *) atomic->tmp);
	return FcFalse;
    }
    ret = fprintf (f, "%ld\n", (long)getpid());
    if (ret <= 0)
    {
	fclose (f);
	unlink ((char *) atomic->tmp);
	return FcFalse;
    }
    if (fclose (f) == EOF)
    {
	unlink ((char *) atomic->tmp);
	return FcFalse;
    }
    ret = link ((char *) atomic->tmp, (char *) atomic->lck);
    if (ret < 0 && (errno == EPERM || errno == ENOTSUP || errno == EACCES))
    {
	/* the filesystem where atomic->lck points to may not supports
	 * the hard link. so better try to fallback
	 */
	ret = mkdir ((char *) atomic->lck, 0600);
	no_link = FcTrue;
    }
    (void) unlink ((char *) atomic->tmp);
#else
    ret = mkdir ((char *) atomic->lck, 0600);
#endif
    if (ret < 0)
    {
	/*
	 * If the file is around and old (> 10 minutes),
	 * assume the lock is stale.  This assumes that any
	 * machines sharing the same filesystem will have clocks
	 * reasonably close to each other.
	 */
	if (FcStat (atomic->lck, &lck_stat) >= 0)
	{
	    time_t  now = time (0);
	    if ((long int) (now - lck_stat.st_mtime) > 10 * 60)
	    {
#ifdef HAVE_LINK
		if (no_link)
		{
		    if (rmdir ((char *) atomic->lck) == 0)
			return FcAtomicLock (atomic);
		}
		else
		{
		    if (unlink ((char *) atomic->lck) == 0)
			return FcAtomicLock (atomic);
		}
#else
		if (rmdir ((char *) atomic->lck) == 0)
		    return FcAtomicLock (atomic);
#endif
	    }
	}
	return FcFalse;
    }
    (void) unlink ((char *) atomic->new);
    return FcTrue;
}

FcChar8 *
FcAtomicNewFile (FcAtomic *atomic)
{
    return atomic->new;
}

FcChar8 *
FcAtomicOrigFile (FcAtomic *atomic)
{
    return atomic->file;
}

FcBool
FcAtomicReplaceOrig (FcAtomic *atomic)
{
#ifdef _WIN32
    unlink ((const char *) atomic->file);
#endif
    if (rename ((char *) atomic->new, (char *) atomic->file) < 0)
	return FcFalse;
    return FcTrue;
}

void
FcAtomicDeleteNew (FcAtomic *atomic)
{
    unlink ((char *) atomic->new);
}

void
FcAtomicUnlock (FcAtomic *atomic)
{
#ifdef HAVE_LINK
    if (unlink ((char *) atomic->lck) == -1)
	rmdir ((char *) atomic->lck);
#else
    rmdir ((char *) atomic->lck);
#endif
}

void
FcAtomicDestroy (FcAtomic *atomic)
{
    if (atomic)
	free (atomic);
}
#define __fcatomic__
#include "fcaliastail.h"
#undef __fcatomic__
