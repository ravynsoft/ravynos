/*
 * fontconfig/src/fccompat.c
 *
 * Copyright © 2012 Red Hat, Inc.
 *
 * Author(s):
 *  Akira TAGOH
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

#include "fcint.h"

#include <errno.h>
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef O_CLOEXEC
#define FC_O_CLOEXEC O_CLOEXEC
#else
#define FC_O_CLOEXEC 0
#endif
#ifdef O_LARGEFILE
#define FC_O_LARGEFILE O_LARGEFILE
#else
#define FC_O_LARGEFILE 0
#endif
#ifdef O_BINARY
#define FC_O_BINARY O_BINARY
#else
#define FC_O_BINARY 0
#endif
#ifdef O_TEMPORARY
#define FC_O_TEMPORARY O_TEMPORARY
#else
#define FC_O_TEMPORARY 0
#endif
#ifdef O_NOINHERIT
#define FC_O_NOINHERIT O_NOINHERIT
#else
#define FC_O_NOINHERIT 0
#endif

#ifndef HAVE_UNISTD_H
/* Values for the second argument to access. These may be OR'd together. */
#ifndef R_OK
#define R_OK    4       /* Test for read permission.  */
#endif
#ifndef W_OK
#define W_OK    2       /* Test for write permission.  */
#endif
#ifndef F_OK
#define F_OK    0       /* Test for existence.  */
#endif

typedef int mode_t;
#endif /* !HAVE_UNISTD_H */

#if !defined (HAVE_MKOSTEMP) && !defined(HAVE_MKSTEMP) && !defined(HAVE__MKTEMP_S)
static int
mkstemp (char *template)
{
    static const char s[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int fd, i;
    size_t l;

    if (template == NULL)
    {
	errno = EINVAL;
	return -1;
    }
    l = strlen (template);
    if (l < 6 || strcmp (&template[l - 6], "XXXXXX") != 0)
    {
	errno = EINVAL;
	return -1;
    }
    do
    {
	errno = 0;
	for (i = l - 6; i < l; i++)
	{
	    int r = FcRandom ();
	    template[i] = s[r % 62];
	}
	fd = FcOpen (template, FC_O_BINARY | O_CREAT | O_EXCL | FC_O_TEMPORARY | FC_O_NOINHERIT | O_RDWR, 0600);
    } while (fd < 0 && errno == EEXIST);
    if (fd >= 0)
	errno = 0;

    return fd;
}
#define HAVE_MKSTEMP 1
#endif

int
FcOpen(const char *pathname, int flags, ...)
{
    int fd = -1;

    if (flags & O_CREAT)
    {
	va_list ap;
	mode_t mode;

	va_start(ap, flags);
	mode = (mode_t) va_arg(ap, int);
	va_end(ap);

	fd = open(pathname, flags | FC_O_CLOEXEC | FC_O_LARGEFILE, mode);
    }
    else
    {
	fd = open(pathname, flags | FC_O_CLOEXEC | FC_O_LARGEFILE);
    }

    return fd;
}

int
FcMakeTempfile (char *template)
{
    int fd = -1;

#if HAVE_MKOSTEMP
    fd = mkostemp (template, FC_O_CLOEXEC);
#elif HAVE_MKSTEMP
    fd = mkstemp (template);
#  ifdef F_DUPFD_CLOEXEC
    if (fd != -1)
    {
	int newfd = fcntl(fd, F_DUPFD_CLOEXEC, STDIN_FILENO);

	close(fd);
	fd = newfd;
    }
#  elif defined(FD_CLOEXEC)
    if (fd != -1)
    {
	fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | FD_CLOEXEC);
    }
#  endif
#elif HAVE__MKTEMP_S
   if (_mktemp_s(template, strlen(template) + 1) != 0)
       return -1;
   fd = FcOpen(template, O_RDWR | O_EXCL | O_CREAT, 0600);
#endif

    return fd;
}

int32_t
FcRandom(void)
{
    int32_t result;

#if HAVE_RANDOM_R
    static struct random_data fcrandbuf;
    static char statebuf[256];
    static FcBool initialized = FcFalse;
#ifdef _AIX
    static char *retval;
    long res;
#endif

    if (initialized != FcTrue)
    {
#ifdef _AIX
	initstate_r (time (NULL), statebuf, 256, &retval, &fcrandbuf);
#else
	initstate_r (time (NULL), statebuf, 256, &fcrandbuf);
#endif
	initialized = FcTrue;
    }

#ifdef _AIX
    random_r (&res, &fcrandbuf);
    result = (int32_t)res;
#else
    random_r (&fcrandbuf, &result);
#endif
#elif HAVE_RANDOM
    static char statebuf[256];
    char *state;
    static FcBool initialized = FcFalse;

    if (initialized != FcTrue)
    {
	state = initstate (time (NULL), statebuf, 256);
	initialized = FcTrue;
    }
    else
	state = setstate (statebuf);

    result = random ();

    setstate (state);
#elif HAVE_LRAND48
    result = lrand48 ();
#elif HAVE_RAND_R
    static unsigned int seed = time (NULL);

    result = rand_r (&seed);
#elif HAVE_RAND
    static FcBool initialized = FcFalse;

    if (initialized != FcTrue)
    {
	srand (time (NULL));
	initialized = FcTrue;
    }
    result = rand ();
#else
# error no random number generator function available.
#endif

    return result;
}

#ifdef _WIN32
#include <direct.h>
#define mkdir(path,mode) _mkdir(path)
#endif

FcBool
FcMakeDirectory (const FcChar8 *dir)
{
    FcChar8 *parent;
    FcBool  ret;

    if (strlen ((char *) dir) == 0)
	return FcFalse;

    parent = FcStrDirname (dir);
    if (!parent)
	return FcFalse;
    if (access ((char *) parent, F_OK) == 0)
	ret = mkdir ((char *) dir, 0755) == 0 && chmod ((char *) dir, 0755) == 0;
    else if (access ((char *) parent, F_OK) == -1)
	ret = FcMakeDirectory (parent) && (mkdir ((char *) dir, 0755) == 0) && chmod ((char *) dir, 0755) == 0;
    else
	ret = FcFalse;
    FcStrFree (parent);
    return ret;
}

ssize_t
FcReadLink (const FcChar8 *pathname,
	    FcChar8       *buf,
	    size_t         bufsiz)
{
#ifdef HAVE_READLINK
    return readlink ((const char *) pathname, (char *)buf, bufsiz);
#else
    /* XXX: this function is only used for FcConfigRealFilename() so far
     * and returning -1 as an error still just works.
     */
    errno = ENOSYS;
    return -1;
#endif
}

/* On Windows MingW provides dirent.h / openddir(), but MSVC does not */
#ifndef HAVE_DIRENT_H

struct DIR {
    struct dirent d_ent;
    HANDLE handle;
    WIN32_FIND_DATA fdata;
    FcBool valid;
};

FcPrivate DIR *
FcCompatOpendirWin32 (const char *dirname)
{
    size_t len;
    char *name;
    DIR *dir;

    dir = calloc (1, sizeof (struct DIR));
    if (dir == NULL)
        return NULL;

    len = strlen (dirname);
    name = malloc (len + 3);
    if (name == NULL)
    {
      free (dir);
      return NULL;
    }
    memcpy (name, dirname, len);
    name[len++] = FC_DIR_SEPARATOR;
    name[len++] = '*';
    name[len] = '\0';

    dir->handle = FindFirstFileEx (name, FindExInfoBasic, &dir->fdata, FindExSearchNameMatch, NULL, 0);

    free (name);

    if (!dir->handle)
    {
        free (dir);
        dir = NULL;

        if (GetLastError () == ERROR_FILE_NOT_FOUND)
            errno = ENOENT;
        else
            errno = EACCES;
    }

    dir->valid = FcTrue;
    return dir;
}

FcPrivate struct dirent *
FcCompatReaddirWin32 (DIR *dir)
{
    if (dir->valid != FcTrue)
        return NULL;

    dir->d_ent.d_name = dir->fdata.cFileName;

    if ((dir->fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
        dir->d_ent.d_type = DT_DIR;
    else if (dir->fdata.dwFileAttributes == FILE_ATTRIBUTE_NORMAL)
        dir->d_ent.d_type = DT_REG;
    else
        dir->d_ent.d_type = DT_UNKNOWN;

    if (!FindNextFile (dir->handle, &dir->fdata))
        dir->valid = FcFalse;

    return &dir->d_ent;
}

FcPrivate int
FcCompatClosedirWin32 (DIR *dir)
{
    if (dir != NULL && dir->handle != NULL)
    {
        FindClose (dir->handle);
        free (dir);
    }
    return 0;
}
#endif /* HAVE_DIRENT_H */

#define __fccompat__
#include "fcaliastail.h"
#undef __fccompat__
