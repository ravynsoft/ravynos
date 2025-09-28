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
#include "fcmd5.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef _WIN32
  #include <sys/time.h>
#else
  #include <winsock2.h> /* for struct timeval */
#endif

#include <assert.h>
#if defined(HAVE_MMAP) || defined(__CYGWIN__)
#  include <unistd.h>
#  include <sys/mman.h>
#endif
#if defined(_WIN32)
#include <sys/locking.h>
#endif

#ifndef O_BINARY
#define O_BINARY 0
#endif

FcBool
FcDirCacheCreateUUID (FcChar8  *dir,
		      FcBool    force,
		      FcConfig *config)
{
    return FcTrue;
}

FcBool
FcDirCacheDeleteUUID (const FcChar8  *dir,
		      FcConfig       *config)
{
    FcBool ret = FcTrue;
#ifndef _WIN32
    const FcChar8 *sysroot;
    FcChar8 *target, *d;
    struct stat statb;
    struct timeval times[2];

    config = FcConfigReference (config);
    if (!config)
	return FcFalse;
    sysroot = FcConfigGetSysRoot (config);
    if (sysroot)
	d = FcStrBuildFilename (sysroot, dir, NULL);
    else
	d = FcStrBuildFilename (dir, NULL);
    if (FcStat (d, &statb) != 0)
    {
	ret = FcFalse;
	goto bail;
    }
    target = FcStrBuildFilename (d, ".uuid", NULL);
    ret = unlink ((char *) target) == 0;
    if (ret)
    {
	times[0].tv_sec = statb.st_atime;
	times[1].tv_sec = statb.st_mtime;
#ifdef HAVE_STRUCT_STAT_ST_MTIM
	times[0].tv_usec = statb.st_atim.tv_nsec / 1000;
	times[1].tv_usec = statb.st_mtim.tv_nsec / 1000;
#else
	times[0].tv_usec = 0;
	times[1].tv_usec = 0;
#endif
	if (utimes ((const char *) d, times) != 0)
	{
	    fprintf (stderr, "Unable to revert mtime: %s\n", d);
	}
    }
    FcStrFree (target);
bail:
    FcStrFree (d);
#endif
    FcConfigDestroy (config);

    return ret;
}

#define CACHEBASE_LEN (1 + 36 + 1 + sizeof (FC_ARCHITECTURE) + sizeof (FC_CACHE_SUFFIX))

static FcBool
FcCacheIsMmapSafe (int fd)
{
    enum {
      MMAP_NOT_INITIALIZED = 0,
      MMAP_USE,
      MMAP_DONT_USE,
      MMAP_CHECK_FS,
    } status;
    static void *static_status;

    status = (intptr_t) fc_atomic_ptr_get (&static_status);

    if (status == MMAP_NOT_INITIALIZED)
    {
	const char *env = getenv ("FONTCONFIG_USE_MMAP");
	FcBool use;
	if (env && FcNameBool ((const FcChar8 *) env, &use))
	    status =  use ? MMAP_USE : MMAP_DONT_USE;
	else
	    status = MMAP_CHECK_FS;
	(void) fc_atomic_ptr_cmpexch (&static_status, NULL, (void *) (intptr_t) status);
    }

    if (status == MMAP_CHECK_FS)
	return FcIsFsMmapSafe (fd);
    else
	return status == MMAP_USE;

}

static const char bin2hex[] = { '0', '1', '2', '3',
				'4', '5', '6', '7',
				'8', '9', 'a', 'b',
				'c', 'd', 'e', 'f' };

static FcChar8 *
FcDirCacheBasenameMD5 (FcConfig *config, const FcChar8 *dir, FcChar8 cache_base[CACHEBASE_LEN])
{
    FcChar8		*mapped_dir = NULL;
    unsigned char 	hash[16];
    FcChar8		*hex_hash, *key = NULL;
    int			cnt;
    struct MD5Context 	ctx;
    const FcChar8	*salt, *orig_dir = NULL;

    salt = FcConfigMapSalt (config, dir);
    /* Obtain a path where "dir" is mapped to.
     * In case:
     * <remap-dir as-path="/usr/share/fonts">/run/host/fonts</remap-dir>
     *
     * FcConfigMapFontPath (config, "/run/host/fonts") will returns "/usr/share/fonts".
     */
    mapped_dir = FcConfigMapFontPath(config, dir);
    if (mapped_dir)
    {
	orig_dir = dir;
	dir = mapped_dir;
    }
    if (salt)
    {
	size_t dl = strlen ((const char *) dir);
	size_t sl = strlen ((const char *) salt);

	key = (FcChar8 *) malloc (dl + sl + 1);
	memcpy (key, dir, dl);
	memcpy (key + dl, salt, sl + 1);
	key[dl + sl] = 0;
	if (!orig_dir)
		orig_dir = dir;
	dir = key;
    }
    MD5Init (&ctx);
    MD5Update (&ctx, (const unsigned char *)dir, strlen ((const char *) dir));

    MD5Final (hash, &ctx);

    if (key)
	FcStrFree (key);

    cache_base[0] = '/';
    hex_hash = cache_base + 1;
    for (cnt = 0; cnt < 16; ++cnt)
    {
	hex_hash[2*cnt  ] = bin2hex[hash[cnt] >> 4];
	hex_hash[2*cnt+1] = bin2hex[hash[cnt] & 0xf];
    }
    hex_hash[2*cnt] = 0;
    strcat ((char *) cache_base, "-" FC_ARCHITECTURE FC_CACHE_SUFFIX);
    if (FcDebug() & FC_DBG_CACHE)
    {
	printf ("cache: %s (dir: %s%s%s%s%s%s)\n", cache_base, orig_dir ? orig_dir : dir, mapped_dir ? " (mapped to " : "", mapped_dir ? (char *)mapped_dir : "", mapped_dir ? ")" : "", salt ? ", salt: " : "", salt ? (char *)salt : "");
    }

    if (mapped_dir)
	FcStrFree(mapped_dir);

    return cache_base;
}

#ifndef _WIN32
static FcChar8 *
FcDirCacheBasenameUUID (FcConfig *config, const FcChar8 *dir, FcChar8 cache_base[CACHEBASE_LEN])
{
    FcChar8 *target, *fuuid;
    const FcChar8 *sysroot = FcConfigGetSysRoot (config);
    int fd;

    /* We don't need to apply remapping here. because .uuid was created at that very directory
     * to determine the cache name no matter where it was mapped to.
     */
    cache_base[0] = 0;
    if (sysroot)
	target = FcStrBuildFilename (sysroot, dir, NULL);
    else
	target = FcStrdup (dir);
    fuuid = FcStrBuildFilename (target, ".uuid", NULL);
    if ((fd = FcOpen ((char *) fuuid, O_RDONLY)) != -1)
    {
	char suuid[37];
	ssize_t len;

	memset (suuid, 0, sizeof (suuid));
	len = read (fd, suuid, 36);
	suuid[36] = 0;
	close (fd);
	if (len < 0)
	    goto bail;
	cache_base[0] = '/';
	strcpy ((char *)&cache_base[1], suuid);
	strcat ((char *) cache_base, "-" FC_ARCHITECTURE FC_CACHE_SUFFIX);
	if (FcDebug () & FC_DBG_CACHE)
	{
	    printf ("cache fallbacks to: %s (dir: %s)\n", cache_base, dir);
	}
    }
bail:
    FcStrFree (fuuid);
    FcStrFree (target);

    return cache_base;
}
#endif

FcBool
FcDirCacheUnlink (const FcChar8 *dir, FcConfig *config)
{
    FcChar8	*cache_hashed = NULL;
    FcChar8	cache_base[CACHEBASE_LEN];
#ifndef _WIN32
    FcChar8     uuid_cache_base[CACHEBASE_LEN];
#endif
    FcStrList	*list;
    FcChar8	*cache_dir;
    const FcChar8 *sysroot;
    FcBool	ret = FcTrue;

    config = FcConfigReference (config);
    if (!config)
	return FcFalse;
    sysroot = FcConfigGetSysRoot (config);

    FcDirCacheBasenameMD5 (config, dir, cache_base);
#ifndef _WIN32
    FcDirCacheBasenameUUID (config, dir, uuid_cache_base);
#endif

    list = FcStrListCreate (config->cacheDirs);
    if (!list)
    {
	ret = FcFalse;
	goto bail;
    }

    while ((cache_dir = FcStrListNext (list)))
    {
	if (sysroot)
	    cache_hashed = FcStrBuildFilename (sysroot, cache_dir, cache_base, NULL);
	else
	    cache_hashed = FcStrBuildFilename (cache_dir, cache_base, NULL);
        if (!cache_hashed)
	    break;
	(void) unlink ((char *) cache_hashed);
	FcStrFree (cache_hashed);
#ifndef _WIN32
	if (uuid_cache_base[0] != 0)
	{
	    if (sysroot)
		cache_hashed = FcStrBuildFilename (sysroot, cache_dir, uuid_cache_base, NULL);
	    else
		cache_hashed = FcStrBuildFilename (cache_dir, uuid_cache_base, NULL);
	    if (!cache_hashed)
		break;
	    (void) unlink ((char *) cache_hashed);
	    FcStrFree (cache_hashed);
	}
#endif
    }
    FcStrListDone (list);
    FcDirCacheDeleteUUID (dir, config);
    /* return FcFalse if something went wrong */
    if (cache_dir)
	ret = FcFalse;
bail:
    FcConfigDestroy (config);

    return ret;
}

static int
FcDirCacheOpenFile (const FcChar8 *cache_file, struct stat *file_stat)
{
    int	fd;

#ifdef _WIN32
    if (FcStat (cache_file, file_stat) < 0)
        return -1;
#endif
    fd = FcOpen((char *) cache_file, O_RDONLY | O_BINARY);
    if (fd < 0)
	return fd;
#ifndef _WIN32
    if (fstat (fd, file_stat) < 0)
    {
	close (fd);
	return -1;
    }
#endif
    return fd;
}

/*
 * Look for a cache file for the specified dir. Attempt
 * to use each one we find, stopping when the callback
 * indicates success
 */
static FcBool
FcDirCacheProcess (FcConfig *config, const FcChar8 *dir,
		   FcBool (*callback) (FcConfig *config, int fd, struct stat *fd_stat,
				       struct stat *dir_stat, struct timeval *cache_mtime, void *closure),
		   void *closure, FcChar8 **cache_file_ret)
{
    int		fd = -1;
    FcChar8	cache_base[CACHEBASE_LEN];
    FcStrList	*list;
    FcChar8	*cache_dir, *d;
    struct stat file_stat, dir_stat;
    FcBool	ret = FcFalse;
    const FcChar8 *sysroot = FcConfigGetSysRoot (config);
    struct timeval latest_mtime = (struct timeval){ 0 };

    if (sysroot)
	d = FcStrBuildFilename (sysroot, dir, NULL);
    else
	d = FcStrdup (dir);
    if (FcStatChecksum (d, &dir_stat) < 0)
    {
	FcStrFree (d);
        return FcFalse;
    }
    FcStrFree (d);

    FcDirCacheBasenameMD5 (config, dir, cache_base);

    list = FcStrListCreate (config->cacheDirs);
    if (!list)
        return FcFalse;

    while ((cache_dir = FcStrListNext (list)))
    {
        FcChar8	*cache_hashed;
#ifndef _WIN32
	FcBool retried = FcFalse;
#endif

	if (sysroot)
	    cache_hashed = FcStrBuildFilename (sysroot, cache_dir, cache_base, NULL);
	else
	    cache_hashed = FcStrBuildFilename (cache_dir, cache_base, NULL);
        if (!cache_hashed)
	    break;
#ifndef _WIN32
      retry:
#endif
        fd = FcDirCacheOpenFile (cache_hashed, &file_stat);
        if (fd >= 0) {
	    ret = (*callback) (config, fd, &file_stat, &dir_stat, &latest_mtime, closure);
	    close (fd);
	    if (ret)
	    {
		if (cache_file_ret)
		{
		    if (*cache_file_ret)
			FcStrFree (*cache_file_ret);
		    *cache_file_ret = cache_hashed;
		}
		else
		    FcStrFree (cache_hashed);
	    }
	    else
		FcStrFree (cache_hashed);
	}
#ifndef _WIN32
	else if (!retried)
	{
	    FcChar8	uuid_cache_base[CACHEBASE_LEN];

	    retried = FcTrue;
	    FcDirCacheBasenameUUID (config, dir, uuid_cache_base);
	    if (uuid_cache_base[0] != 0)
	    {
		FcStrFree (cache_hashed);
		if (sysroot)
		    cache_hashed = FcStrBuildFilename (sysroot, cache_dir, uuid_cache_base, NULL);
		else
		    cache_hashed = FcStrBuildFilename (cache_dir, uuid_cache_base, NULL);
		if (!cache_hashed)
		    break;
		goto retry;
	    }
	    else
		FcStrFree (cache_hashed);
	}
#endif
	else
	    FcStrFree (cache_hashed);
    }
    FcStrListDone (list);

    if (closure)
	return !!(*((FcCache **)closure) != NULL);
    return ret;
}

#define FC_CACHE_MIN_MMAP   1024

/*
 * Skip list element, make sure the 'next' pointer is the last thing
 * in the structure, it will be allocated large enough to hold all
 * of the necessary pointers
 */

typedef struct _FcCacheSkip FcCacheSkip;

struct _FcCacheSkip {
    FcCache	    *cache;
    FcRef	    ref;
    intptr_t	    size;
    void	   *allocated;
    dev_t	    cache_dev;
    ino_t	    cache_ino;
    time_t	    cache_mtime;
    long	    cache_mtime_nano;
    FcCacheSkip	    *next[1];
};

/*
 * The head of the skip list; pointers for every possible level
 * in the skip list, plus the largest level in the list
 */

#define FC_CACHE_MAX_LEVEL  16

/* Protected by cache_lock below */
static FcCacheSkip	*fcCacheChains[FC_CACHE_MAX_LEVEL];
static int		fcCacheMaxLevel;


static FcMutex *cache_lock;

static void
lock_cache (void)
{
  FcMutex *lock;
retry:
  lock = fc_atomic_ptr_get (&cache_lock);
  if (!lock) {
    lock = (FcMutex *) malloc (sizeof (FcMutex));
    FcMutexInit (lock);
    if (!fc_atomic_ptr_cmpexch (&cache_lock, NULL, lock)) {
      FcMutexFinish (lock);
      free (lock);
      goto retry;
    }

    FcMutexLock (lock);
    /* Initialize random state */
    FcRandom ();
    return;
  }
  FcMutexLock (lock);
}

static void
unlock_cache (void)
{
  FcMutex *lock;
  lock = fc_atomic_ptr_get (&cache_lock);
  FcMutexUnlock (lock);
}

static void
free_lock (void)
{
  FcMutex *lock;
  lock = fc_atomic_ptr_get (&cache_lock);
  if (lock && fc_atomic_ptr_cmpexch (&cache_lock, lock, NULL)) {
    FcMutexFinish (lock);
    free (lock);
  }
}



/*
 * Generate a random level number, distributed
 * so that each level is 1/4 as likely as the one before
 *
 * Note that level numbers run 1 <= level <= MAX_LEVEL
 */
static int
random_level (void)
{
    /* tricky bit -- each bit is '1' 75% of the time */
    long int	bits = FcRandom () | FcRandom ();
    int	level = 0;

    while (++level < FC_CACHE_MAX_LEVEL)
    {
	if (bits & 1)
	    break;
	bits >>= 1;
    }
    return level;
}

/*
 * Insert cache into the list
 */
static FcBool
FcCacheInsert (FcCache *cache, struct stat *cache_stat)
{
    FcCacheSkip    **update[FC_CACHE_MAX_LEVEL];
    FcCacheSkip    *s, **next;
    int		    i, level;

    lock_cache ();

    /*
     * Find links along each chain
     */
    next = fcCacheChains;
    for (i = fcCacheMaxLevel; --i >= 0; )
    {
	for (; (s = next[i]); next = s->next)
	    if (s->cache > cache)
		break;
        update[i] = &next[i];
    }

    /*
     * Create new list element
     */
    level = random_level ();
    if (level > fcCacheMaxLevel)
    {
	level = fcCacheMaxLevel + 1;
	update[fcCacheMaxLevel] = &fcCacheChains[fcCacheMaxLevel];
	fcCacheMaxLevel = level;
    }

    s = malloc (sizeof (FcCacheSkip) + (level - 1) * sizeof (FcCacheSkip *));
    if (!s)
	return FcFalse;

    s->cache = cache;
    s->size = cache->size;
    s->allocated = NULL;
    FcRefInit (&s->ref, 1);
    if (cache_stat)
    {
	s->cache_dev = cache_stat->st_dev;
	s->cache_ino = cache_stat->st_ino;
	s->cache_mtime = cache_stat->st_mtime;
#ifdef HAVE_STRUCT_STAT_ST_MTIM
	s->cache_mtime_nano = cache_stat->st_mtim.tv_nsec;
#else
	s->cache_mtime_nano = 0;
#endif
    }
    else
    {
	s->cache_dev = 0;
	s->cache_ino = 0;
	s->cache_mtime = 0;
	s->cache_mtime_nano = 0;
    }

    /*
     * Insert into all fcCacheChains
     */
    for (i = 0; i < level; i++)
    {
	s->next[i] = *update[i];
	*update[i] = s;
    }

    unlock_cache ();
    return FcTrue;
}

static FcCacheSkip *
FcCacheFindByAddrUnlocked (void *object)
{
    int	    i;
    FcCacheSkip    **next = fcCacheChains;
    FcCacheSkip    *s;

    if (!object)
	return NULL;

    /*
     * Walk chain pointers one level at a time
     */
    for (i = fcCacheMaxLevel; --i >= 0;)
	while (next[i] && (char *) object >= ((char *) next[i]->cache + next[i]->size))
	    next = next[i]->next;
    /*
     * Here we are
     */
    s = next[0];
    if (s && (char *) object < ((char *) s->cache + s->size))
	return s;
    return NULL;
}

static FcCacheSkip *
FcCacheFindByAddr (void *object)
{
    FcCacheSkip *ret;
    lock_cache ();
    ret = FcCacheFindByAddrUnlocked (object);
    unlock_cache ();
    return ret;
}

static void
FcCacheRemoveUnlocked (FcCache *cache)
{
    FcCacheSkip	    **update[FC_CACHE_MAX_LEVEL];
    FcCacheSkip	    *s, **next;
    int		    i;
    void            *allocated;

    /*
     * Find links along each chain
     */
    next = fcCacheChains;
    for (i = fcCacheMaxLevel; --i >= 0; )
    {
	for (; (s = next[i]); next = s->next)
	    if (s->cache >= cache)
		break;
        update[i] = &next[i];
    }
    s = next[0];
    for (i = 0; i < fcCacheMaxLevel && *update[i] == s; i++)
	*update[i] = s->next[i];
    while (fcCacheMaxLevel > 0 && fcCacheChains[fcCacheMaxLevel - 1] == NULL)
	fcCacheMaxLevel--;

    if (s)
    {
	allocated = s->allocated;
	while (allocated)
	{
	    /* First element in allocated chunk is the free list */
	    next = *(void **)allocated;
	    free (allocated);
	    allocated = next;
	}
	free (s);
    }
}

static FcCache *
FcCacheFindByStat (struct stat *cache_stat)
{
    FcCacheSkip	    *s;

    lock_cache ();
    for (s = fcCacheChains[0]; s; s = s->next[0])
	if (s->cache_dev == cache_stat->st_dev &&
	    s->cache_ino == cache_stat->st_ino &&
	    s->cache_mtime == cache_stat->st_mtime)
	{
#ifdef HAVE_STRUCT_STAT_ST_MTIM
	    if (s->cache_mtime_nano != cache_stat->st_mtim.tv_nsec)
		continue;
#endif
	    FcRefInc (&s->ref);
	    unlock_cache ();
	    return s->cache;
	}
    unlock_cache ();
    return NULL;
}

static void
FcDirCacheDisposeUnlocked (FcCache *cache)
{
    FcCacheRemoveUnlocked (cache);

    switch (cache->magic) {
    case FC_CACHE_MAGIC_ALLOC:
	free (cache);
	break;
    case FC_CACHE_MAGIC_MMAP:
#if defined(HAVE_MMAP) || defined(__CYGWIN__)
	munmap (cache, cache->size);
#elif defined(_WIN32)
	UnmapViewOfFile (cache);
#endif
	break;
    }
}

void
FcCacheObjectReference (void *object)
{
    FcCacheSkip *skip = FcCacheFindByAddr (object);

    if (skip)
	FcRefInc (&skip->ref);
}

void
FcCacheObjectDereference (void *object)
{
    FcCacheSkip	*skip;

    lock_cache ();
    skip = FcCacheFindByAddrUnlocked (object);
    if (skip)
    {
	if (FcRefDec (&skip->ref) == 1)
	    FcDirCacheDisposeUnlocked (skip->cache);
    }
    unlock_cache ();
}

void *
FcCacheAllocate (FcCache *cache, size_t len)
{
    FcCacheSkip	*skip;
    void *allocated = NULL;

    lock_cache ();
    skip = FcCacheFindByAddrUnlocked (cache);
    if (skip)
    {
      void *chunk = malloc (sizeof (void *) + len);
      if (chunk)
      {
	  /* First element in allocated chunk is the free list */
	  *(void **)chunk = skip->allocated;
	  skip->allocated = chunk;
	  /* Return the rest */
	  allocated = ((FcChar8 *)chunk) + sizeof (void *);
      }
    }
    unlock_cache ();
    return allocated;
}

void
FcCacheFini (void)
{
    int		    i;

    if (FcDebug() & FC_DBG_CACHE)
    {
	for (i = 0; i < FC_CACHE_MAX_LEVEL; i++)
	{
	    if (fcCacheChains[i] != NULL)
	    {
		FcCacheSkip *s = fcCacheChains[i];
		fprintf(stderr, "Fontconfig error: not freed %p (dir: %s, refcount %" FC_ATOMIC_INT_FORMAT ")\n", s->cache, FcCacheDir(s->cache), s->ref.count);
	    }
	}
    }

    free_lock ();
}

static FcBool
FcCacheTimeValid (FcConfig *config, FcCache *cache, struct stat *dir_stat)
{
    struct stat	dir_static;
    FcBool fnano = FcTrue;

    if (!dir_stat)
    {
	const FcChar8 *sysroot = FcConfigGetSysRoot (config);
	FcChar8 *d;

	if (sysroot)
	    d = FcStrBuildFilename (sysroot, FcCacheDir (cache), NULL);
	else
	    d = FcStrdup (FcCacheDir (cache));
	if (FcStatChecksum (d, &dir_static) < 0)
	{
	    FcStrFree (d);
	    return FcFalse;
	}
	FcStrFree (d);
	dir_stat = &dir_static;
    }
#ifdef HAVE_STRUCT_STAT_ST_MTIM
    fnano = (cache->checksum_nano == dir_stat->st_mtim.tv_nsec);
    if (FcDebug () & FC_DBG_CACHE)
	printf ("FcCacheTimeValid dir \"%s\" cache checksum %d.%ld dir checksum %d.%ld\n",
		FcCacheDir (cache), cache->checksum, (long)cache->checksum_nano, (int) dir_stat->st_mtime, dir_stat->st_mtim.tv_nsec);
#else
    if (FcDebug () & FC_DBG_CACHE)
	printf ("FcCacheTimeValid dir \"%s\" cache checksum %d dir checksum %d\n",
		FcCacheDir (cache), cache->checksum, (int) dir_stat->st_mtime);
#endif

    return dir_stat->st_mtime == 0 || (cache->checksum == (int) dir_stat->st_mtime && fnano);
}

static FcBool
FcCacheOffsetsValid (FcCache *cache)
{
    char		*base = (char *)cache;
    char		*end = base + cache->size;
    intptr_t		*dirs;
    FcFontSet		*fs;
    int			 i, j;

    if (cache->dir < 0 || cache->dir > cache->size - sizeof (intptr_t) ||
        memchr (base + cache->dir, '\0', cache->size - cache->dir) == NULL)
        return FcFalse;

    if (cache->dirs < 0 || cache->dirs >= cache->size ||
        cache->dirs_count < 0 ||
        cache->dirs_count > (cache->size - cache->dirs) / sizeof (intptr_t))
        return FcFalse;

    dirs = FcCacheDirs (cache);
    if (dirs)
    {
        for (i = 0; i < cache->dirs_count; i++)
        {
            FcChar8	*dir;

            if (dirs[i] < 0 ||
                dirs[i] > end - (char *) dirs - sizeof (intptr_t))
                return FcFalse;

            dir = FcOffsetToPtr (dirs, dirs[i], FcChar8);
            if (memchr (dir, '\0', end - (char *) dir) == NULL)
                return FcFalse;
         }
    }

    if (cache->set < 0 || cache->set > cache->size - sizeof (FcFontSet))
        return FcFalse;

    fs = FcCacheSet (cache);
    if (fs)
    {
        if (fs->nfont > (end - (char *) fs) / sizeof (FcPattern))
            return FcFalse;

        if (!FcIsEncodedOffset(fs->fonts))
            return FcFalse;

        for (i = 0; i < fs->nfont; i++)
        {
            FcPattern		*font = FcFontSetFont (fs, i);
            FcPatternElt	*e;
            FcValueListPtr	 l;
	    char                *last_offset;

            if ((char *) font < base ||
                (char *) font > end - sizeof (FcFontSet) ||
                font->elts_offset < 0 ||
                font->elts_offset > end - (char *) font ||
                font->num > (end - (char *) font - font->elts_offset) / sizeof (FcPatternElt) ||
		!FcRefIsConst (&font->ref))
                return FcFalse;


            e = FcPatternElts(font);
            if (e->values != 0 && !FcIsEncodedOffset(e->values))
                return FcFalse;

	    for (j = 0; j < font->num; j++)
	    {
		last_offset = (char *) font + font->elts_offset;
		for (l = FcPatternEltValues(&e[j]); l; l = FcValueListNext(l))
		{
		    if ((char *) l < last_offset || (char *) l > end - sizeof (*l) ||
			(l->next != NULL && !FcIsEncodedOffset(l->next)))
			return FcFalse;
		    last_offset = (char *) l + 1;
		}
	    }
        }
    }

    return FcTrue;
}

/*
 * Map a cache file into memory
 */
static FcCache *
FcDirCacheMapFd (FcConfig *config, int fd, struct stat *fd_stat, struct stat *dir_stat)
{
    FcCache	*cache;
    FcBool	allocated = FcFalse;

    if (fd_stat->st_size > INTPTR_MAX ||
        fd_stat->st_size < (int) sizeof (FcCache))
	return NULL;
    cache = FcCacheFindByStat (fd_stat);
    if (cache)
    {
	if (FcCacheTimeValid (config, cache, dir_stat))
	    return cache;
	FcDirCacheUnload (cache);
	cache = NULL;
    }

    /*
     * Large cache files are mmap'ed, smaller cache files are read. This
     * balances the system cost of mmap against per-process memory usage.
     */
    if (FcCacheIsMmapSafe (fd) && fd_stat->st_size >= FC_CACHE_MIN_MMAP)
    {
#if defined(HAVE_MMAP) || defined(__CYGWIN__)
	cache = mmap (0, fd_stat->st_size, PROT_READ, MAP_SHARED, fd, 0);
#if defined(HAVE_POSIX_FADVISE) && defined(POSIX_FADV_WILLNEED)
	posix_fadvise (fd, 0, fd_stat->st_size, POSIX_FADV_WILLNEED);
#endif
	if (cache == MAP_FAILED)
	    cache = NULL;
#elif defined(_WIN32)
	{
	    HANDLE hFileMap;

	    cache = NULL;
	    hFileMap = CreateFileMapping((HANDLE) _get_osfhandle(fd), NULL,
					 PAGE_READONLY, 0, 0, NULL);
	    if (hFileMap != NULL)
	    {
		cache = MapViewOfFile (hFileMap, FILE_MAP_READ, 0, 0,
				       fd_stat->st_size);
		CloseHandle (hFileMap);
	    }
	}
#endif
    }
    if (!cache)
    {
	cache = malloc (fd_stat->st_size);
	if (!cache)
	    return NULL;

	if (read (fd, cache, fd_stat->st_size) != fd_stat->st_size)
	{
	    free (cache);
	    return NULL;
	}
	allocated = FcTrue;
    }
    if (cache->magic != FC_CACHE_MAGIC_MMAP ||
	cache->version < FC_CACHE_VERSION_NUMBER ||
	cache->size != (intptr_t) fd_stat->st_size ||
        !FcCacheOffsetsValid (cache) ||
	!FcCacheTimeValid (config, cache, dir_stat) ||
	!FcCacheInsert (cache, fd_stat))
    {
	if (allocated)
	    free (cache);
	else
	{
#if defined(HAVE_MMAP) || defined(__CYGWIN__)
	    munmap (cache, fd_stat->st_size);
#elif defined(_WIN32)
	    UnmapViewOfFile (cache);
#endif
	}
	return NULL;
    }

    /* Mark allocated caches so they're freed rather than unmapped */
    if (allocated)
	cache->magic = FC_CACHE_MAGIC_ALLOC;

    return cache;
}

void
FcDirCacheReference (FcCache *cache, int nref)
{
    FcCacheSkip *skip = FcCacheFindByAddr (cache);

    if (skip)
	FcRefAdd (&skip->ref, nref);
}

void
FcDirCacheUnload (FcCache *cache)
{
    FcCacheObjectDereference (cache);
}

static FcBool
FcDirCacheMapHelper (FcConfig *config, int fd, struct stat *fd_stat, struct stat *dir_stat, struct timeval *latest_cache_mtime, void *closure)
{
    FcCache *cache = FcDirCacheMapFd (config, fd, fd_stat, dir_stat);
    struct timeval cache_mtime, zero_mtime = { 0, 0}, dir_mtime;

    if (!cache)
	return FcFalse;
    cache_mtime.tv_sec = fd_stat->st_mtime;
    dir_mtime.tv_sec = dir_stat->st_mtime;
#ifdef HAVE_STRUCT_STAT_ST_MTIM
    cache_mtime.tv_usec = fd_stat->st_mtim.tv_nsec / 1000;
    dir_mtime.tv_usec = dir_stat->st_mtim.tv_nsec / 1000;
#else
    cache_mtime.tv_usec = 0;
    dir_mtime.tv_usec = 0;
#endif
    /* special take care of OSTree */
    if (!timercmp (&zero_mtime, &dir_mtime, !=))
    {
	if (!timercmp (&zero_mtime, &cache_mtime, !=))
	{
	    if (*((FcCache **) closure))
		FcDirCacheUnload (*((FcCache **) closure));
	}
	else if (*((FcCache **) closure) && !timercmp (&zero_mtime, latest_cache_mtime, !=))
	{
	    FcDirCacheUnload (cache);
	    return FcFalse;
	}
	else if (timercmp (latest_cache_mtime, &cache_mtime, <))
	{
	    if (*((FcCache **) closure))
		FcDirCacheUnload (*((FcCache **) closure));
	}
    }
    else if (timercmp (latest_cache_mtime, &cache_mtime, <))
    {
	if (*((FcCache **) closure))
	    FcDirCacheUnload (*((FcCache **) closure));
    }
    else
    {
	FcDirCacheUnload (cache);
	return FcFalse;
    }
    latest_cache_mtime->tv_sec = cache_mtime.tv_sec;
    latest_cache_mtime->tv_usec = cache_mtime.tv_usec;
    *((FcCache **) closure) = cache;
    return FcTrue;
}

FcCache *
FcDirCacheLoad (const FcChar8 *dir, FcConfig *config, FcChar8 **cache_file)
{
    FcCache *cache = NULL;

    config = FcConfigReference (config);
    if (!config)
	return NULL;
    if (!FcDirCacheProcess (config, dir,
			    FcDirCacheMapHelper,
			    &cache, cache_file))
	cache = NULL;

    FcConfigDestroy (config);

    return cache;
}

FcCache *
FcDirCacheLoadFile (const FcChar8 *cache_file, struct stat *file_stat)
{
    int	fd;
    FcCache *cache = NULL;
    struct stat	my_file_stat;
    FcConfig *config;

    if (!file_stat)
	file_stat = &my_file_stat;
    config = FcConfigReference (NULL);
    if (!config)
	return NULL;
    fd = FcDirCacheOpenFile (cache_file, file_stat);
    if (fd >= 0)
    {
	cache = FcDirCacheMapFd (config, fd, file_stat, NULL);
	close (fd);
    }
    FcConfigDestroy (config);

    return cache;
}

static int
FcDirChecksum (struct stat *statb)
{
    int			ret = (int) statb->st_mtime;
    char		*endptr;
    char		*source_date_epoch;
    unsigned long long	epoch;

    source_date_epoch = getenv("SOURCE_DATE_EPOCH");
    if (source_date_epoch)
    {
	errno = 0;
	epoch = strtoull(source_date_epoch, &endptr, 10);

	if (endptr == source_date_epoch)
	    fprintf (stderr,
		     "Fontconfig: SOURCE_DATE_EPOCH invalid\n");
	else if ((errno == ERANGE && (epoch == ULLONG_MAX || epoch == 0))
		|| (errno != 0 && epoch == 0))
	    fprintf (stderr,
		     "Fontconfig: SOURCE_DATE_EPOCH: strtoull: %s: %" FC_UINT64_FORMAT "\n",
		     strerror(errno), epoch);
	else if (*endptr != '\0')
	    fprintf (stderr,
		     "Fontconfig: SOURCE_DATE_EPOCH has trailing garbage\n");
	else if (epoch > ULONG_MAX)
	    fprintf (stderr,
		     "Fontconfig: SOURCE_DATE_EPOCH must be <= %lu but saw: %" FC_UINT64_FORMAT "\n",
		     ULONG_MAX, epoch);
	else if (epoch < ret)
	    /* Only override if directory is newer */
	    ret = (int) epoch;
    }

    return ret;
}

static int64_t
FcDirChecksumNano (struct stat *statb)
{
#ifdef HAVE_STRUCT_STAT_ST_MTIM
    /* No nanosecond component to parse */
    if (getenv("SOURCE_DATE_EPOCH"))
	return 0;
    return statb->st_mtim.tv_nsec;
#else
    return 0;
#endif
}

/*
 * Validate a cache file by reading the header and checking
 * the magic number and the size field
 */
static FcBool
FcDirCacheValidateHelper (FcConfig *config, int fd, struct stat *fd_stat, struct stat *dir_stat, struct timeval *latest_cache_mtime, void *closure FC_UNUSED)
{
    FcBool  ret = FcTrue;
    FcCache	c;

    if (read (fd, &c, sizeof (FcCache)) != sizeof (FcCache))
	ret = FcFalse;
    else if (c.magic != FC_CACHE_MAGIC_MMAP)
	ret = FcFalse;
    else if (c.version < FC_CACHE_VERSION_NUMBER)
	ret = FcFalse;
    else if (fd_stat->st_size != c.size)
	ret = FcFalse;
    else if (c.checksum != FcDirChecksum (dir_stat))
	ret = FcFalse;
#ifdef HAVE_STRUCT_STAT_ST_MTIM
    else if (c.checksum_nano != FcDirChecksumNano (dir_stat))
	ret = FcFalse;
#endif
    return ret;
}

static FcBool
FcDirCacheValidConfig (const FcChar8 *dir, FcConfig *config)
{
    return FcDirCacheProcess (config, dir,
			      FcDirCacheValidateHelper,
			      NULL, NULL);
}

FcBool
FcDirCacheValid (const FcChar8 *dir)
{
    FcConfig	*config;
    FcBool	ret;

    config = FcConfigReference (NULL);
    if (!config)
        return FcFalse;

    ret = FcDirCacheValidConfig (dir, config);
    FcConfigDestroy (config);

    return ret;
}

/*
 * Build a cache structure from the given contents
 */
FcCache *
FcDirCacheBuild (FcFontSet *set, const FcChar8 *dir, struct stat *dir_stat, FcStrSet *dirs)
{
    FcSerialize	*serialize = FcSerializeCreate ();
    FcCache *cache;
    int i;
    FcChar8	*dir_serialize;
    intptr_t	*dirs_serialize;
    FcFontSet	*set_serialize;

    if (!serialize)
	return NULL;
    /*
     * Space for cache structure
     */
    FcSerializeReserve (serialize, sizeof (FcCache));
    /*
     * Directory name
     */
    if (!FcStrSerializeAlloc (serialize, dir))
	goto bail1;
    /*
     * Subdirs
     */
    FcSerializeAlloc (serialize, dirs, dirs->num * sizeof (FcChar8 *));
    for (i = 0; i < dirs->num; i++)
	if (!FcStrSerializeAlloc (serialize, dirs->strs[i]))
	    goto bail1;

    /*
     * Patterns
     */
    if (!FcFontSetSerializeAlloc (serialize, set))
	goto bail1;

    /* Serialize layout complete. Now allocate space and fill it */
    cache = malloc (serialize->size);
    if (!cache)
	goto bail1;
    /* shut up valgrind */
    memset (cache, 0, serialize->size);

    serialize->linear = cache;

    cache->magic = FC_CACHE_MAGIC_ALLOC;
    cache->version = FC_CACHE_VERSION_NUMBER;
    cache->size = serialize->size;
    cache->checksum = FcDirChecksum (dir_stat);
    cache->checksum_nano = FcDirChecksumNano (dir_stat);

    /*
     * Serialize directory name
     */
    dir_serialize = FcStrSerialize (serialize, dir);
    if (!dir_serialize)
	goto bail2;
    cache->dir = FcPtrToOffset (cache, dir_serialize);

    /*
     * Serialize sub dirs
     */
    dirs_serialize = FcSerializePtr (serialize, dirs);
    if (!dirs_serialize)
	goto bail2;
    cache->dirs = FcPtrToOffset (cache, dirs_serialize);
    cache->dirs_count = dirs->num;
    for (i = 0; i < dirs->num; i++)
    {
	FcChar8	*d_serialize = FcStrSerialize (serialize, dirs->strs[i]);
	if (!d_serialize)
	    goto bail2;
	dirs_serialize[i] = FcPtrToOffset (dirs_serialize, d_serialize);
    }

    /*
     * Serialize font set
     */
    set_serialize = FcFontSetSerialize (serialize, set);
    if (!set_serialize)
	goto bail2;
    cache->set = FcPtrToOffset (cache, set_serialize);

    FcSerializeDestroy (serialize);

    FcCacheInsert (cache, NULL);

    return cache;

bail2:
    free (cache);
bail1:
    FcSerializeDestroy (serialize);
    return NULL;
}

FcCache *
FcDirCacheRebuild (FcCache *cache, struct stat *dir_stat, FcStrSet *dirs)
{
    FcCache *new;
    FcFontSet *set = FcFontSetDeserialize (FcCacheSet (cache));
    const FcChar8 *dir = FcCacheDir (cache);

    new = FcDirCacheBuild (set, dir, dir_stat, dirs);
    FcFontSetDestroy (set);

    return new;
}

/* write serialized state to the cache file */
FcBool
FcDirCacheWrite (FcCache *cache, FcConfig *config)
{
    FcChar8	    *dir = FcCacheDir (cache);
    FcChar8	    cache_base[CACHEBASE_LEN];
    FcChar8	    *cache_hashed;
    int 	    fd;
    FcAtomic 	    *atomic;
    FcStrList	    *list;
    FcChar8	    *cache_dir = NULL;
    FcChar8	    *test_dir, *d = NULL;
    FcCacheSkip     *skip;
    struct stat     cache_stat;
    unsigned int    magic;
    int		    written;
    const FcChar8   *sysroot = FcConfigGetSysRoot (config);

    /*
     * Write it to the first directory in the list which is writable
     */

    list = FcStrListCreate (config->cacheDirs);
    if (!list)
	return FcFalse;
    while ((test_dir = FcStrListNext (list)))
    {
	if (d)
	    FcStrFree (d);
	if (sysroot)
	    d = FcStrBuildFilename (sysroot, test_dir, NULL);
	else
	    d = FcStrCopyFilename (test_dir);

	if (access ((char *) d, W_OK) == 0)
	{
	    cache_dir = FcStrCopyFilename (d);
	    break;
	}
	else
	{
	    /*
	     * If the directory doesn't exist, try to create it
	     */
	    if (access ((char *) d, F_OK) == -1) {
		if (FcMakeDirectory (d))
		{
		    cache_dir = FcStrCopyFilename (d);
		    /* Create CACHEDIR.TAG */
		    FcDirCacheCreateTagFile (d);
		    break;
		}
	    }
	    /*
	     * Otherwise, try making it writable
	     */
	    else if (chmod ((char *) d, 0755) == 0)
	    {
		cache_dir = FcStrCopyFilename (d);
		/* Try to create CACHEDIR.TAG too */
		FcDirCacheCreateTagFile (d);
		break;
	    }
	}
    }
    if (!test_dir)
	fprintf (stderr, "Fontconfig error: No writable cache directories\n");
    if (d)
	FcStrFree (d);
    FcStrListDone (list);
    if (!cache_dir)
	return FcFalse;

    FcDirCacheBasenameMD5 (config, dir, cache_base);
    cache_hashed = FcStrBuildFilename (cache_dir, cache_base, NULL);
    FcStrFree (cache_dir);
    if (!cache_hashed)
        return FcFalse;

    if (FcDebug () & FC_DBG_CACHE)
        printf ("FcDirCacheWriteDir dir \"%s\" file \"%s\"\n",
		dir, cache_hashed);

    atomic = FcAtomicCreate ((FcChar8 *)cache_hashed);
    if (!atomic)
	goto bail1;

    if (!FcAtomicLock (atomic))
	goto bail3;

    fd = FcOpen((char *)FcAtomicNewFile (atomic), O_RDWR | O_CREAT | O_BINARY, 0666);
    if (fd == -1)
	goto bail4;

    /* Temporarily switch magic to MMAP while writing to file */
    magic = cache->magic;
    if (magic != FC_CACHE_MAGIC_MMAP)
	cache->magic = FC_CACHE_MAGIC_MMAP;

    /*
     * Write cache contents to file
     */
    written = write (fd, cache, cache->size);

    /* Switch magic back */
    if (magic != FC_CACHE_MAGIC_MMAP)
	cache->magic = magic;

    if (written != cache->size)
    {
	perror ("write cache");
	goto bail5;
    }

    close(fd);
    if (!FcAtomicReplaceOrig(atomic))
        goto bail4;

    /* If the file is small, update the cache chain entry such that the
     * new cache file is not read again.  If it's large, we don't do that
     * such that we reload it, using mmap, which is shared across processes.
     */
    if (cache->size < FC_CACHE_MIN_MMAP && FcStat (cache_hashed, &cache_stat))
    {
	lock_cache ();
	if ((skip = FcCacheFindByAddrUnlocked (cache)))
	{
	    skip->cache_dev = cache_stat.st_dev;
	    skip->cache_ino = cache_stat.st_ino;
	    skip->cache_mtime = cache_stat.st_mtime;
#ifdef HAVE_STRUCT_STAT_ST_MTIM
	    skip->cache_mtime_nano = cache_stat.st_mtim.tv_nsec;
#else
	    skip->cache_mtime_nano = 0;
#endif
	}
	unlock_cache ();
    }

    FcStrFree (cache_hashed);
    FcAtomicUnlock (atomic);
    FcAtomicDestroy (atomic);
    return FcTrue;

 bail5:
    close (fd);
 bail4:
    FcAtomicUnlock (atomic);
 bail3:
    FcAtomicDestroy (atomic);
 bail1:
    FcStrFree (cache_hashed);
    return FcFalse;
}

FcBool
FcDirCacheClean (const FcChar8 *cache_dir, FcBool verbose)
{
    DIR		*d;
    struct dirent *ent;
    FcChar8	*dir;
    FcBool	ret = FcTrue;
    FcBool	remove;
    FcCache	*cache;
    struct stat	target_stat;
    const FcChar8 *sysroot;
    FcConfig	*config;

    config = FcConfigReference (NULL);
    if (!config)
	return FcFalse;
    /* FIXME: this API needs to support non-current FcConfig */
    sysroot = FcConfigGetSysRoot (config);
    if (sysroot)
	dir = FcStrBuildFilename (sysroot, cache_dir, NULL);
    else
	dir = FcStrCopyFilename (cache_dir);
    if (!dir)
    {
	fprintf (stderr, "Fontconfig error: %s: out of memory\n", cache_dir);
	ret = FcFalse;
	goto bail;
    }
    if (access ((char *) dir, W_OK) != 0)
    {
	if (verbose || FcDebug () & FC_DBG_CACHE)
	    printf ("%s: not cleaning %s cache directory\n", dir,
		    access ((char *) dir, F_OK) == 0 ? "unwritable" : "non-existent");
	goto bail0;
    }
    if (verbose || FcDebug () & FC_DBG_CACHE)
	printf ("%s: cleaning cache directory\n", dir);
    d = opendir ((char *) dir);
    if (!d)
    {
	perror ((char *) dir);
	ret = FcFalse;
	goto bail0;
    }
    while ((ent = readdir (d)))
    {
	FcChar8	*file_name;
	const FcChar8	*target_dir;

	if (ent->d_name[0] == '.')
	    continue;
	/* skip cache files for different architectures and */
	/* files which are not cache files at all */
	if (strlen(ent->d_name) != 32 + strlen ("-" FC_ARCHITECTURE FC_CACHE_SUFFIX) ||
	    strcmp(ent->d_name + 32, "-" FC_ARCHITECTURE FC_CACHE_SUFFIX))
	    continue;

	file_name = FcStrBuildFilename (dir, (FcChar8 *)ent->d_name, NULL);
	if (!file_name)
	{
	    fprintf (stderr, "Fontconfig error: %s: allocation failure\n", dir);
	    ret = FcFalse;
	    break;
	}
	remove = FcFalse;
	cache = FcDirCacheLoadFile (file_name, NULL);
	if (!cache)
	{
	    if (verbose || FcDebug () & FC_DBG_CACHE)
		printf ("%s: invalid cache file: %s\n", dir, ent->d_name);
	    remove = FcTrue;
	}
	else
	{
	    FcChar8 *s;

	    target_dir = FcCacheDir (cache);
	    if (sysroot)
		s = FcStrBuildFilename (sysroot, target_dir, NULL);
	    else
		s = FcStrdup (target_dir);
	    if (stat ((char *) s, &target_stat) < 0)
	    {
		if (verbose || FcDebug () & FC_DBG_CACHE)
		    printf ("%s: %s: missing directory: %s \n",
			    dir, ent->d_name, s);
		remove = FcTrue;
	    }
	    FcDirCacheUnload (cache);
	    FcStrFree (s);
	}
	if (remove)
	{
	    if (unlink ((char *) file_name) < 0)
	    {
		perror ((char *) file_name);
		ret = FcFalse;
	    }
	}
        FcStrFree (file_name);
    }

    closedir (d);
bail0:
    FcStrFree (dir);
bail:
    FcConfigDestroy (config);

    return ret;
}

int
FcDirCacheLock (const FcChar8 *dir,
		FcConfig      *config)
{
    FcChar8 *cache_hashed = NULL;
    FcChar8 cache_base[CACHEBASE_LEN];
    FcStrList *list;
    FcChar8 *cache_dir;
    const FcChar8 *sysroot = FcConfigGetSysRoot (config);
    int fd = -1;

    FcDirCacheBasenameMD5 (config, dir, cache_base);
    list = FcStrListCreate (config->cacheDirs);
    if (!list)
	return -1;

    while ((cache_dir = FcStrListNext (list)))
    {
	if (sysroot)
	    cache_hashed = FcStrBuildFilename (sysroot, cache_dir, cache_base, NULL);
	else
	    cache_hashed = FcStrBuildFilename (cache_dir, cache_base, NULL);
	if (!cache_hashed)
	    break;
	fd = FcOpen ((const char *)cache_hashed, O_RDWR);
	FcStrFree (cache_hashed);
	/* No caches in that directory. simply retry with another one */
	if (fd != -1)
	{
#if defined(_WIN32)
	    if (_locking (fd, _LK_LOCK, 1) == -1)
		goto bail;
#else
	    struct flock fl;

	    fl.l_type = F_WRLCK;
	    fl.l_whence = SEEK_SET;
	    fl.l_start = 0;
	    fl.l_len = 0;
	    fl.l_pid = getpid ();
	    if (fcntl (fd, F_SETLKW, &fl) == -1)
		goto bail;
#endif
	    break;
	}
    }
    FcStrListDone (list);
    return fd;
bail:
    FcStrListDone (list);
    if (fd != -1)
	close (fd);
    return -1;
}

void
FcDirCacheUnlock (int fd)
{
    if (fd != -1)
    {
#if defined(_WIN32)
	_locking (fd, _LK_UNLCK, 1);
#else
	struct flock fl;

	fl.l_type = F_UNLCK;
	fl.l_whence = SEEK_SET;
	fl.l_start = 0;
	fl.l_len = 0;
	fl.l_pid = getpid ();
	fcntl (fd, F_SETLK, &fl);
#endif
	close (fd);
    }
}

/*
 * Hokey little macro trick to permit the definitions of C functions
 * with the same name as CPP macros
 */
#define args1(x)	    (x)
#define args2(x,y)	    (x,y)

const FcChar8 *
FcCacheDir args1(const FcCache *c)
{
    return FcCacheDir (c);
}

FcFontSet *
FcCacheCopySet args1(const FcCache *c)
{
    FcFontSet	*old = FcCacheSet (c);
    FcFontSet	*new = FcFontSetCreate ();
    int		i;

    if (!new)
	return NULL;
    for (i = 0; i < old->nfont; i++)
    {
	FcPattern   *font = FcFontSetFont (old, i);

	FcPatternReference (font);
	if (!FcFontSetAdd (new, font))
	{
	    FcFontSetDestroy (new);
	    return NULL;
	}
    }
    return new;
}

const FcChar8 *
FcCacheSubdir args2(const FcCache *c, int i)
{
    return FcCacheSubdir (c, i);
}

int
FcCacheNumSubdir args1(const FcCache *c)
{
    return c->dirs_count;
}

int
FcCacheNumFont args1(const FcCache *c)
{
    return FcCacheSet(c)->nfont;
}

FcBool
FcDirCacheCreateTagFile (const FcChar8 *cache_dir)
{
    FcChar8		*cache_tag;
    int 		 fd;
    FILE		*fp;
    FcAtomic		*atomic;
    static const FcChar8 cache_tag_contents[] =
	"Signature: 8a477f597d28d172789f06886806bc55\n"
	"# This file is a cache directory tag created by fontconfig.\n"
	"# For information about cache directory tags, see:\n"
	"#       http://www.brynosaurus.com/cachedir/\n";
    static size_t	 cache_tag_contents_size = sizeof (cache_tag_contents) - 1;
    FcBool		 ret = FcFalse;

    if (!cache_dir)
	return FcFalse;

    if (access ((char *) cache_dir, W_OK) == 0)
    {
	/* Create CACHEDIR.TAG */
	cache_tag = FcStrBuildFilename (cache_dir, "CACHEDIR.TAG", NULL);
	if (!cache_tag)
	    return FcFalse;
	atomic = FcAtomicCreate ((FcChar8 *)cache_tag);
	if (!atomic)
	    goto bail1;
	if (!FcAtomicLock (atomic))
	    goto bail2;
	fd = FcOpen((char *)FcAtomicNewFile (atomic), O_RDWR | O_CREAT, 0644);
	if (fd == -1)
	    goto bail3;
	fp = fdopen(fd, "wb");
	if (fp == NULL)
	    goto bail3;

	fwrite(cache_tag_contents, cache_tag_contents_size, sizeof (FcChar8), fp);
	fclose(fp);

	if (!FcAtomicReplaceOrig(atomic))
	    goto bail3;

	ret = FcTrue;
      bail3:
	FcAtomicUnlock (atomic);
      bail2:
	FcAtomicDestroy (atomic);
      bail1:
	FcStrFree (cache_tag);
    }

    if (FcDebug () & FC_DBG_CACHE)
    {
	if (ret)
	    printf ("Created CACHEDIR.TAG at %s\n", cache_dir);
	else
	    printf ("Unable to create CACHEDIR.TAG at %s\n", cache_dir);
    }

    return ret;
}

void
FcCacheCreateTagFile (FcConfig *config)
{
    FcChar8   *cache_dir = NULL, *d = NULL;
    FcStrList *list;
    const FcChar8 *sysroot;

    config = FcConfigReference (config);
    if (!config)
	return;
    sysroot = FcConfigGetSysRoot (config);

    list = FcConfigGetCacheDirs (config);
    if (!list)
	goto bail;

    while ((cache_dir = FcStrListNext (list)))
    {
	if (d)
	    FcStrFree (d);
	if (sysroot)
	    d = FcStrBuildFilename (sysroot, cache_dir, NULL);
	else
	    d = FcStrCopyFilename (cache_dir);
	if (FcDirCacheCreateTagFile (d))
	    break;
    }
    if (d)
	FcStrFree (d);
    FcStrListDone (list);
bail:
    FcConfigDestroy (config);
}

#define __fccache__
#include "fcaliastail.h"
#undef __fccache__
