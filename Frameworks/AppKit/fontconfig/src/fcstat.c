/*
 * Copyright © 2000 Keith Packard
 * Copyright © 2005 Patrick Lam
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
#include "fcarch.h"
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef HAVE_SYS_VFS_H
#include <sys/vfs.h>
#endif
#ifdef HAVE_SYS_STATVFS_H
#include <sys/statvfs.h>
#endif
#ifdef HAVE_SYS_STATFS_H
#include <sys/statfs.h>
#endif
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_SYS_MOUNT_H
#include <sys/mount.h>
#endif
#include <errno.h>

#ifdef _WIN32
#ifdef __GNUC__
typedef long long INT64;
#define EPOCH_OFFSET 11644473600ll
#else
#define EPOCH_OFFSET 11644473600i64
typedef __int64 INT64;
#endif

/* Workaround for problems in the stat() in the Microsoft C library:
 *
 * 1) stat() uses FindFirstFile() to get the file
 * attributes. Unfortunately this API doesn't return correct values
 * for modification time of a directory until some time after a file
 * or subdirectory has been added to the directory. (This causes
 * run-test.sh to fail, for instance.) GetFileAttributesEx() is
 * better, it returns the updated timestamp right away.
 *
 * 2) stat() does some strange things related to backward
 * compatibility with the local time timestamps on FAT volumes and
 * daylight saving time. This causes problems after the switches
 * to/from daylight saving time. See
 * http://bugzilla.gnome.org/show_bug.cgi?id=154968 , especially
 * comment #30, and http://www.codeproject.com/datetime/dstbugs.asp .
 * We don't need any of that, FAT and Win9x are as good as dead. So
 * just use the UTC timestamps from NTFS, converted to the Unix epoch.
 */

int
FcStat (const FcChar8 *file, struct stat *statb)
{
    WIN32_FILE_ATTRIBUTE_DATA wfad;
    char full_path_name[MAX_PATH];
    char *basename;
    DWORD rc;

    if (!GetFileAttributesEx ((LPCSTR) file, GetFileExInfoStandard, &wfad))
	return -1;

    statb->st_dev = 0;

    /* Calculate a pseudo inode number as a hash of the full path name.
     * Call GetLongPathName() to get the spelling of the path name as it
     * is on disk.
     */
    rc = GetFullPathName ((LPCSTR) file, sizeof (full_path_name), full_path_name, &basename);
    if (rc == 0 || rc > sizeof (full_path_name))
	return -1;

    rc = GetLongPathName (full_path_name, full_path_name, sizeof (full_path_name));
    statb->st_ino = FcStringHash ((const FcChar8 *) full_path_name);

    statb->st_mode = _S_IREAD | _S_IWRITE;
    statb->st_mode |= (statb->st_mode >> 3) | (statb->st_mode >> 6);

    if (wfad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	statb->st_mode |= _S_IFDIR;
    else
	statb->st_mode |= _S_IFREG;

    statb->st_nlink = 1;
    statb->st_uid = statb->st_gid = 0;
    statb->st_rdev = 0;

    if (wfad.nFileSizeHigh > 0)
	return -1;
    statb->st_size = wfad.nFileSizeLow;

    statb->st_atime = (*(INT64 *)&wfad.ftLastAccessTime)/10000000 - EPOCH_OFFSET;
    statb->st_mtime = (*(INT64 *)&wfad.ftLastWriteTime)/10000000 - EPOCH_OFFSET;
    statb->st_ctime = statb->st_mtime;

    return 0;
}

#else

int
FcStat (const FcChar8 *file, struct stat *statb)
{
  return stat ((char *) file, statb);
}

/* Adler-32 checksum implementation */
struct Adler32 {
    int a;
    int b;
};

static void
Adler32Init (struct Adler32 *ctx)
{
    ctx->a = 1;
    ctx->b = 0;
}

static void
Adler32Update (struct Adler32 *ctx, const char *data, int data_len)
{
    while (data_len--)
    {
	ctx->a = (ctx->a + *data++) % 65521;
	ctx->b = (ctx->b + ctx->a) % 65521;
    }
}

static int
Adler32Finish (struct Adler32 *ctx)
{
    return ctx->a + (ctx->b << 16);
}

#ifdef HAVE_STRUCT_DIRENT_D_TYPE
/* dirent.d_type can be relied upon on FAT filesystem */
static FcBool
FcDirChecksumScandirFilter(const struct dirent *entry)
{
    return entry->d_type != DT_DIR;
}
#endif

static int
FcDirChecksumScandirSorter(const struct dirent **lhs, const struct dirent **rhs)
{
    return strcmp((*lhs)->d_name, (*rhs)->d_name);
}

static void
free_dirent (struct dirent **p)
{
    struct dirent **x;

    for (x = p; *x != NULL; x++)
	free (*x);

    free (p);
}

int
FcScandir (const char		*dirp,
	   struct dirent	***namelist,
	   int (*filter) (const struct dirent *),
	   int (*compar) (const struct dirent **, const struct dirent **));

int
FcScandir (const char		*dirp,
	   struct dirent	***namelist,
	   int (*filter) (const struct dirent *),
	   int (*compar) (const struct dirent **, const struct dirent **))
{
    DIR *d;
    struct dirent *dent, *p, **dlist, **dlp;
    size_t lsize = 128, n = 0;

    d = opendir (dirp);
    if (!d)
	return -1;

    dlist = (struct dirent **) malloc (sizeof (struct dirent *) * lsize);
    if (!dlist)
    {
	closedir (d);
	errno = ENOMEM;

	return -1;
    }
    *dlist = NULL;
    while ((dent = readdir (d)))
    {
	if (!filter || (filter) (dent))
	{
	    size_t dentlen = FcPtrToOffset (dent, dent->d_name) + strlen (dent->d_name) + 1;
	    dentlen = ((dentlen + ALIGNOF_VOID_P - 1) & ~(ALIGNOF_VOID_P - 1));
	    p = (struct dirent *) malloc (dentlen);
	    if (!p)
	    {
		free_dirent (dlist);
		closedir (d);
		errno = ENOMEM;

		return -1;
	    }
	    memcpy (p, dent, dentlen);
	    if ((n + 1) >= lsize)
	    {
		lsize += 128;
		dlp = (struct dirent **) realloc (dlist, sizeof (struct dirent *) * lsize);
		if (!dlp)
		{
		    free (p);
		    free_dirent (dlist);
		    closedir (d);
		    errno = ENOMEM;

		    return -1;
		}
		dlist = dlp;
	    }
	    dlist[n++] = p;
	    dlist[n] = NULL;
	}
    }
    closedir (d);

    qsort (dlist, n, sizeof (struct dirent *), (int (*) (const void *, const void *))compar);

    *namelist = dlist;

    return n;
}

static int
FcDirChecksum (const FcChar8 *dir, time_t *checksum)
{
    struct Adler32 ctx;
    struct dirent **files;
    int n;
    int ret = 0;
    size_t len = strlen ((const char *)dir);

    Adler32Init (&ctx);

    n = FcScandir ((const char *)dir, &files,
#ifdef HAVE_STRUCT_DIRENT_D_TYPE
		 &FcDirChecksumScandirFilter,
#else
		 NULL,
#endif
		 &FcDirChecksumScandirSorter);
    if (n == -1)
	return -1;

    while (n--)
    {
	size_t dlen = strlen (files[n]->d_name);
	int dtype;

#ifdef HAVE_STRUCT_DIRENT_D_TYPE
	dtype = files[n]->d_type;
	if (dtype == DT_UNKNOWN)
	{
#endif
	struct stat statb;
	char *f = malloc (len + 1 + dlen + 1);

	if (!f)
	{
	    ret = -1;
	    goto bail;
	}
	memcpy (f, dir, len);
	f[len] = FC_DIR_SEPARATOR;
	memcpy (&f[len + 1], files[n]->d_name, dlen);
	f[len + 1 + dlen] = 0;
	if (lstat (f, &statb) < 0)
	{
	    ret = -1;
	    free (f);
	    goto bail;
	}
	if (S_ISDIR (statb.st_mode))
	{
	    free (f);
	    goto bail;
	}

	free (f);
	dtype = statb.st_mode;
#ifdef HAVE_STRUCT_DIRENT_D_TYPE
	}
#endif
	Adler32Update (&ctx, files[n]->d_name, dlen + 1);
	Adler32Update (&ctx, (char *)&dtype, sizeof (int));

      bail:
	free (files[n]);
    }
    free (files);
    if (ret == -1)
	return -1;

    *checksum = Adler32Finish (&ctx);

    return 0;
}
#endif /* _WIN32 */

int
FcStatChecksum (const FcChar8 *file, struct stat *statb)
{
    if (FcStat (file, statb) == -1)
        return -1;

#ifndef _WIN32
    /* We have a workaround of the broken stat() in FcStat() for Win32.
     * No need to do something further more.
     */
    if (FcIsFsMtimeBroken (file))
    {
        if (FcDirChecksum (file, &statb->st_mtime) == -1)
            return -1;
    }
#endif

    return 0;
}

static int
FcFStatFs (int fd, FcStatFS *statb)
{
    const char *p = NULL;
    int ret = -1;
    FcBool flag = FcFalse;

#if defined(HAVE_FSTATVFS) && (defined(HAVE_STRUCT_STATVFS_F_BASETYPE) || defined(HAVE_STRUCT_STATVFS_F_FSTYPENAME))
    struct statvfs buf;

    memset (statb, 0, sizeof (FcStatFS));

    if ((ret = fstatvfs (fd, &buf)) == 0)
    {
#  if defined(HAVE_STRUCT_STATVFS_F_BASETYPE)
	p = buf.f_basetype;
#  elif defined(HAVE_STRUCT_STATVFS_F_FSTYPENAME)
	p = buf.f_fstypename;
#  endif
    }
#elif defined(HAVE_FSTATFS) && (defined(HAVE_STRUCT_STATFS_F_FLAGS) || defined(HAVE_STRUCT_STATFS_F_FSTYPENAME) || defined(__linux__))
    struct statfs buf;

    memset (statb, 0, sizeof (FcStatFS));

    if ((ret = fstatfs (fd, &buf)) == 0)
    {
#  if defined(HAVE_STRUCT_STATFS_F_FLAGS) && defined(MNT_LOCAL)
	statb->is_remote_fs = !(buf.f_flags & MNT_LOCAL);
	flag = FcTrue;
#  endif
#  if defined(HAVE_STRUCT_STATFS_F_FSTYPENAME)
	p = buf.f_fstypename;
#  elif defined(__linux__) || defined (__EMSCRIPTEN__)
	switch (buf.f_type)
	{
	case 0x6969: /* nfs */
	    statb->is_remote_fs = FcTrue;
	    break;
	case 0x4d44: /* fat */
	    statb->is_mtime_broken = FcTrue;
	    break;
	default:
	    break;
	}

	return ret;
#  else
#    error "BUG: No way to figure out with fstatfs()"
#  endif
    }
#endif
    if (p)
    {
	if (!flag && strcmp (p, "nfs") == 0)
	    statb->is_remote_fs = FcTrue;
	if (strcmp (p, "msdosfs") == 0 ||
	    strcmp (p, "pcfs") == 0)
	    statb->is_mtime_broken = FcTrue;
    }

    return ret;
}

FcBool
FcIsFsMmapSafe (int fd)
{
    FcStatFS statb;

    if (FcFStatFs (fd, &statb) < 0)
	return FcTrue;

    return !statb.is_remote_fs;
}

FcBool
FcIsFsMtimeBroken (const FcChar8 *dir)
{
    int fd = FcOpen ((const char *) dir, O_RDONLY);

    if (fd != -1)
    {
	FcStatFS statb;
	int ret = FcFStatFs (fd, &statb);

	close (fd);
	if (ret < 0)
	    return FcFalse;

	return statb.is_mtime_broken;
    }

    return FcFalse;
}

#define __fcstat__
#include "fcaliastail.h"
#undef __fcstat__
