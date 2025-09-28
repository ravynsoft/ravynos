/*
 * fontconfig/src/fcdir.c
 *
 * Copyright © 2000 Keith Packard
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

#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif

FcBool
FcFileIsDir (const FcChar8 *file)
{
    struct stat	    statb;

    if (FcStat (file, &statb) != 0)
	return FcFalse;
    return S_ISDIR(statb.st_mode);
}

FcBool
FcFileIsLink (const FcChar8 *file)
{
#if HAVE_LSTAT
    struct stat statb;

    if (lstat ((const char *)file, &statb) != 0)
	return FcFalse;
    return S_ISLNK (statb.st_mode);
#else
    return FcFalse;
#endif
}

FcBool
FcFileIsFile (const FcChar8 *file)
{
    struct stat statb;

    if (FcStat (file, &statb) != 0)
	return FcFalse;
    return S_ISREG (statb.st_mode);
}

static FcBool
FcFileScanFontConfig (FcFontSet		*set,
		      const FcChar8	*file,
		      FcConfig		*config)
{
    int		i;
    FcBool	ret = FcTrue;
    int		old_nfont = set->nfont;
    const FcChar8 *sysroot = FcConfigGetSysRoot (config);

    if (FcDebug () & FC_DBG_SCAN)
    {
	printf ("\tScanning file %s...", file);
	fflush (stdout);
    }

    if (!FcFreeTypeQueryAll (file, -1, NULL, NULL, set))
	return FcFalse;

    if (FcDebug () & FC_DBG_SCAN)
	printf ("done\n");

    for (i = old_nfont; i < set->nfont; i++)
    {
	FcPattern *font = set->fonts[i];

	/*
	 * Get rid of sysroot here so that targeting scan rule may contains FC_FILE pattern
	 * and they should usually expect without sysroot.
	 */
	if (sysroot)
	{
	    size_t len = strlen ((const char *)sysroot);
	    FcChar8 *f = NULL;

	    if (FcPatternObjectGetString (font, FC_FILE_OBJECT, 0, &f) == FcResultMatch &&
		strncmp ((const char *)f, (const char *)sysroot, len) == 0)
	    {
		FcChar8 *s = FcStrdup (f);
		FcPatternObjectDel (font, FC_FILE_OBJECT);
		if (s[len] != '/')
		    len--;
		else if (s[len+1] == '/')
		    len++;
		FcPatternObjectAddString (font, FC_FILE_OBJECT, &s[len]);
		FcStrFree (s);
	    }
	}

	/*
	 * Edit pattern with user-defined rules
	 */
	if (config && !FcConfigSubstitute (config, font, FcMatchScan))
	    ret = FcFalse;

	if (FcDebug() & FC_DBG_SCANV)
	{
	    printf ("Final font pattern:\n");
	    FcPatternPrint (font);
	}
    }

    return ret;
}

FcBool
FcFileScanConfig (FcFontSet	*set,
		  FcStrSet	*dirs,
		  const FcChar8	*file,
		  FcConfig	*config)
{
    if (FcFileIsDir (file))
    {
	const FcChar8 *sysroot = FcConfigGetSysRoot (config);
	const FcChar8 *d = file;
	size_t len;

	if (sysroot)
	{
		len = strlen ((const char *)sysroot);
		if (strncmp ((const char *)file, (const char *)sysroot, len) == 0)
		{
			if (file[len] != '/')
				len--;
			else if (file[len+1] == '/')
				len++;
			d = &file[len];
		}
	}
	return FcStrSetAdd (dirs, d);
    }
    else
    {
	if (set)
	    return FcFileScanFontConfig (set, file, config);
	else
	    return FcTrue;
    }
}

FcBool
FcFileScan (FcFontSet	    *set,
	    FcStrSet	    *dirs,
	    FcFileCache	    *cache FC_UNUSED,
	    FcBlanks	    *blanks FC_UNUSED,
	    const FcChar8   *file,
	    FcBool	    force FC_UNUSED)
{
    FcConfig *config;
    FcBool ret;

    config = FcConfigReference (NULL);
    if (!config)
	return FcFalse;
    ret = FcFileScanConfig (set, dirs, file, config);
    FcConfigDestroy (config);

    return ret;
}

/*
 * Strcmp helper that takes pointers to pointers, copied from qsort(3) manpage
 */
static int
cmpstringp(const void *p1, const void *p2)
{
    return strcmp(* (char **) p1, * (char **) p2);
}

FcBool
FcDirScanConfig (FcFontSet	*set,
		 FcStrSet	*dirs,
		 const FcChar8	*dir,
		 FcBool		force, /* XXX unused */
		 FcConfig	*config)
{
    DIR			*d;
    struct dirent	*e;
    FcStrSet		*files;
    FcChar8		*file_prefix, *s_dir = NULL;
    FcChar8		*base;
    const FcChar8	*sysroot = FcConfigGetSysRoot (config);
    FcBool		ret = FcTrue;
    int			i;

    if (!force)
	return FcFalse;

    if (!set && !dirs)
	return FcTrue;

    /* freed below */
    file_prefix = (FcChar8 *) malloc (strlen ((char *) dir) + 1 + FC_MAX_FILE_LEN + 1);
    if (!file_prefix) {
	ret = FcFalse;
	goto bail;
    }
    strcpy ((char *) file_prefix, (char *) dir);
    strcat ((char *) file_prefix, FC_DIR_SEPARATOR_S);
    base = file_prefix + strlen ((char *) file_prefix);

    if (sysroot)
	s_dir = FcStrBuildFilename (sysroot, dir, NULL);
    else
	s_dir = FcStrdup (dir);
    if (!s_dir) {
	ret = FcFalse;
	goto bail;
    }

    if (FcDebug () & FC_DBG_SCAN)
	printf ("\tScanning dir %s\n", s_dir);
	
    d = opendir ((char *) s_dir);
    if (!d)
    {
	/* Don't complain about missing directories */
	if (errno != ENOENT)
	    ret = FcFalse;
	goto bail;
    }

    files = FcStrSetCreateEx (FCSS_ALLOW_DUPLICATES | FCSS_GROW_BY_64);
    if (!files)
    {
	ret = FcFalse;
	goto bail1;
    }
    while ((e = readdir (d)))
    {
	if (e->d_name[0] != '.' && strlen (e->d_name) < FC_MAX_FILE_LEN)
	{
	    strcpy ((char *) base, (char *) e->d_name);
	    if (!FcStrSetAdd (files, file_prefix)) {
		ret = FcFalse;
		goto bail2;
	    }
	}
    }

    /*
     * Sort files to make things prettier
     */
    qsort(files->strs, files->num, sizeof(FcChar8 *), cmpstringp);

    /*
     * Scan file files to build font patterns
     */
    for (i = 0; i < files->num; i++)
	FcFileScanConfig (set, dirs, files->strs[i], config);

bail2:
    FcStrSetDestroy (files);
bail1:
    closedir (d);
bail:
    if (s_dir)
	free (s_dir);
    if (file_prefix)
	free (file_prefix);

    return ret;
}

FcBool
FcDirScan (FcFontSet	    *set,
	   FcStrSet	    *dirs,
	   FcFileCache	    *cache FC_UNUSED,
	   FcBlanks	    *blanks FC_UNUSED,
	   const FcChar8    *dir,
	   FcBool	    force FC_UNUSED)
{
    FcConfig *config;
    FcBool ret;

    if (cache || !force)
	return FcFalse;

    config = FcConfigReference (NULL);
    if (!config)
	return FcFalse;
    ret = FcDirScanConfig (set, dirs, dir, force, config);
    FcConfigDestroy (config);

    return ret;
}

/*
 * Scan the specified directory and construct a cache of its contents
 */
FcCache *
FcDirCacheScan (const FcChar8 *dir, FcConfig *config)
{
    FcStrSet		*dirs;
    FcFontSet		*set;
    FcCache		*cache = NULL;
    struct stat		dir_stat;
    const FcChar8	*sysroot = FcConfigGetSysRoot (config);
    FcChar8		*d;
#ifndef _WIN32
    int			fd = -1;
#endif

    if (sysroot)
	d = FcStrBuildFilename (sysroot, dir, NULL);
    else
	d = FcStrdup (dir);

    if (FcDebug () & FC_DBG_FONTSET)
	printf ("cache scan dir %s\n", d);

    if (FcStatChecksum (d, &dir_stat) < 0)
	goto bail;

    set = FcFontSetCreate();
    if (!set)
	goto bail;

    dirs = FcStrSetCreateEx (FCSS_GROW_BY_64);
    if (!dirs)
	goto bail1;

#ifndef _WIN32
    fd = FcDirCacheLock (dir, config);
#endif
    /*
     * Scan the dir
     */
    /* Do not pass sysroot here. FcDirScanConfig() do take care of it */
    if (!FcDirScanConfig (set, dirs, dir, FcTrue, config))
	goto bail2;

    /*
     * Build the cache object
     */
    cache = FcDirCacheBuild (set, dir, &dir_stat, dirs);
    if (!cache)
	goto bail2;

    /*
     * Write out the cache file, ignoring any troubles
     */
    FcDirCacheWrite (cache, config);

 bail2:
#ifndef _WIN32
    FcDirCacheUnlock (fd);
#endif
    FcStrSetDestroy (dirs);
 bail1:
    FcFontSetDestroy (set);
 bail:
    FcStrFree (d);

    return cache;
}

FcCache *
FcDirCacheRescan (const FcChar8 *dir, FcConfig *config)
{
    FcCache *cache;
    FcCache *new = NULL;
    struct stat dir_stat;
    FcStrSet *dirs;
    const FcChar8 *sysroot;
    FcChar8 *d = NULL;
#ifndef _WIN32
    int fd = -1;
#endif

    config = FcConfigReference (config);
    if (!config)
	return NULL;
    sysroot = FcConfigGetSysRoot (config);
    cache = FcDirCacheLoad (dir, config, NULL);
    if (!cache)
	goto bail;

    if (sysroot)
	d = FcStrBuildFilename (sysroot, dir, NULL);
    else
	d = FcStrdup (dir);
    if (FcStatChecksum (d, &dir_stat) < 0)
	goto bail;
    dirs = FcStrSetCreateEx (FCSS_GROW_BY_64);
    if (!dirs)
	goto bail;

#ifndef _WIN32
    fd = FcDirCacheLock (dir, config);
#endif
    /*
     * Scan the dir
     */
    /* Do not pass sysroot here. FcDirScanConfig() do take care of it */
    if (!FcDirScanConfig (NULL, dirs, dir, FcTrue, config))
	goto bail1;
    /*
     * Rebuild the cache object
     */
    new = FcDirCacheRebuild (cache, &dir_stat, dirs);
    if (!new)
	goto bail1;
    FcDirCacheUnload (cache);
    /*
     * Write out the cache file, ignoring any troubles
     */
    FcDirCacheWrite (new, config);

bail1:
#ifndef _WIN32
    FcDirCacheUnlock (fd);
#endif
    FcStrSetDestroy (dirs);
bail:
    if (d)
	FcStrFree (d);
    FcConfigDestroy (config);

    return new;
}

/*
 * Read (or construct) the cache for a directory
 */
FcCache *
FcDirCacheRead (const FcChar8 *dir, FcBool force, FcConfig *config)
{
    FcCache		*cache = NULL;

    config = FcConfigReference (config);
    /* Try to use existing cache file */
    if (!force)
	cache = FcDirCacheLoad (dir, config, NULL);

    /* Not using existing cache file, construct new cache */
    if (!cache)
	cache = FcDirCacheScan (dir, config);
    FcConfigDestroy (config);

    return cache;
}

FcBool
FcDirSave (FcFontSet *set FC_UNUSED, FcStrSet * dirs FC_UNUSED, const FcChar8 *dir FC_UNUSED)
{
    return FcFalse; /* XXX deprecated */
}
#define __fcdir__
#include "fcaliastail.h"
#undef __fcdir__
