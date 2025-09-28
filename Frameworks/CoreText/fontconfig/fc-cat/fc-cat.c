/*
 * fontconfig/fc-cat/fc-cat.c
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#else
#ifdef linux
#define HAVE_GETOPT_LONG 1
#endif
#define HAVE_GETOPT 1
#endif

#include <fontconfig/fontconfig.h>
#include "../src/fcarch.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <locale.h>

#ifdef ENABLE_NLS
#include <libintl.h>
#define _(x)		(dgettext(GETTEXT_PACKAGE, x))
#else
#define dgettext(d, s)	(s)
#define _(x)		(x)
#endif

#ifndef HAVE_GETOPT
#define HAVE_GETOPT 0
#endif
#ifndef HAVE_GETOPT_LONG
#define HAVE_GETOPT_LONG 0
#endif

#if HAVE_GETOPT_LONG
#undef  _GNU_SOURCE
#define _GNU_SOURCE
#include <getopt.h>
const struct option longopts[] = {
    {"version", 0, 0, 'V'},
    {"verbose", 0, 0, 'v'},
    {"recurse", 0, 0, 'r'},
    {"help", 0, 0, 'h'},
    {NULL,0,0,0},
};
#else
#if HAVE_GETOPT
extern char *optarg;
extern int optind, opterr, optopt;
#endif
#endif

/*
 * POSIX has broken stdio so that putc must do thread-safe locking,
 * this is a serious performance problem for applications doing large
 * amounts of IO with putc (as is done here).  If available, use
 * the putc_unlocked variant instead.
 */
 
#if defined(putc_unlocked) || defined(_IO_putc_unlocked)
#define PUTC(c,f) putc_unlocked(c,f)
#else
#define PUTC(c,f) putc(c,f)
#endif

static FcBool
write_chars (FILE *f, const FcChar8 *chars)
{
    FcChar8    c;
    while ((c = *chars++))
    {
	switch (c) {
	case '"':
	case '\\':
	    if (PUTC ('\\', f) == EOF)
		return FcFalse;
	    /* fall through */
	default:
	    if (PUTC (c, f) == EOF)
		return FcFalse;
	}
    }
    return FcTrue;
}

static FcBool
write_ulong (FILE *f, unsigned long t)
{
    int	    pow;
    unsigned long   temp, digit;

    temp = t;
    pow = 1;
    while (temp >= 10)
    {
	temp /= 10;
	pow *= 10;
    }
    temp = t;
    while (pow)
    {
	digit = temp / pow;
	if (PUTC ((char) digit + '0', f) == EOF)
	    return FcFalse;
	temp = temp - pow * digit;
	pow = pow / 10;
    }
    return FcTrue;
}

static FcBool
write_int (FILE *f, int i)
{
    return write_ulong (f, (unsigned long) i);
}

static FcBool
write_string (FILE *f, const FcChar8 *string)
{

    if (PUTC ('"', f) == EOF)
	return FcFalse;
    if (!write_chars (f, string))
	return FcFalse;
    if (PUTC ('"', f) == EOF)
	return FcFalse;
    return FcTrue;
}

static void
usage (char *program, int error)
{
    FILE *file = error ? stderr : stdout;
#if HAVE_GETOPT_LONG
    fprintf (file, _("usage: %s [-rv] [--recurse] [--verbose] [*-%s" FC_CACHE_SUFFIX "|directory]...\n"),
	     program, FC_ARCHITECTURE);
    fprintf (file, "       %s [-Vh] [--version] [--help]\n", program);
#else
    fprintf (file, _("usage: %s [-rvVh] [*-%s" FC_CACHE_SUFFIX "|directory]...\n"),
	     program, FC_ARCHITECTURE);
#endif
    fprintf (file, _("Reads font information cache from:\n"));
    fprintf (file, _(" 1) specified fontconfig cache file\n"));
    fprintf (file, _(" 2) related to a particular font directory\n"));
    fprintf (file, "\n");
#if HAVE_GETOPT_LONG
    fprintf (file, _("  -r, --recurse        recurse into subdirectories\n"));
    fprintf (file, _("  -v, --verbose        be verbose\n"));
    fprintf (file, _("  -V, --version        display font config version and exit\n"));
    fprintf (file, _("  -h, --help           display this help and exit\n"));
#else
    fprintf (file, _("  -r         (recurse) recurse into subdirectories\n"));
    fprintf (file, _("  -v         (verbose) be verbose\n"));
    fprintf (file, _("  -V         (version) display font config version and exit\n"));
    fprintf (file, _("  -h         (help)    display this help and exit\n"));
#endif
    exit (error);
}

/*
 * return the path from the directory containing 'cache' to 'file'
 */

static const FcChar8 *
file_base_name (const FcChar8 *cache, const FcChar8 *file)
{
    int		    cache_len = strlen ((char *) cache);

    if (!strncmp ((char *) cache, (char *) file, cache_len) && file[cache_len] == '/')
	return file + cache_len + 1;
    return file;
}

#define FC_FONT_FILE_DIR	((FcChar8 *) ".dir")

static FcBool
cache_print_set (FcFontSet *set, FcStrSet *dirs, const FcChar8 *base_name, FcBool verbose)
{
    FcChar8	    *dir;
    const FcChar8   *base;
    int		    n;
    int		    ndir = 0;
    FcStrList	    *list;

    list = FcStrListCreate (dirs);
    if (!list)
	goto bail2;
    
    while ((dir = FcStrListNext (list)))
    {
	base = file_base_name (base_name, dir);
	if (!write_string (stdout, base))
	    goto bail3;
	if (PUTC (' ', stdout) == EOF)
	    goto bail3;
	if (!write_int (stdout, 0))
	    goto bail3;
        if (PUTC (' ', stdout) == EOF)
	    goto bail3;
	if (!write_string (stdout, FC_FONT_FILE_DIR))
	    goto bail3;
	if (PUTC ('\n', stdout) == EOF)
	    goto bail3;
	ndir++;
    }
    
    for (n = 0; n < set->nfont; n++)
    {
	FcPattern   *font = set->fonts[n];
	FcChar8 *s;

	s = FcPatternFormat (font, (const FcChar8 *) "%{=fccat}\n");
	if (s)
	{
	    printf ("%s", s);
	    FcStrFree (s);
	}
    }
    if (verbose && !set->nfont && !ndir)
	printf ("<empty>\n");

    FcStrListDone (list);

    return FcTrue;

bail3:
    FcStrListDone (list);
bail2:
    return FcFalse;
}

int
main (int argc, char **argv)
{
    int		i;
    int		ret = 0;
    FcFontSet	*fs;
    FcStrSet    *dirs;
    FcStrSet	*args = NULL;
    FcStrList	*arglist;
    FcCache	*cache;
    FcConfig	*config;
    FcChar8	*arg;
    int		verbose = 0;
    int		recurse = 0;
    FcBool	first = FcTrue;
#if HAVE_GETOPT_LONG || HAVE_GETOPT
    int		c;

    setlocale (LC_ALL, "");
#if HAVE_GETOPT_LONG
    while ((c = getopt_long (argc, argv, "Vvrh", longopts, NULL)) != -1)
#else
    while ((c = getopt (argc, argv, "Vvrh")) != -1)
#endif
    {
	switch (c) {
	case 'V':
	    fprintf (stderr, "fontconfig version %d.%d.%d\n", 
		     FC_MAJOR, FC_MINOR, FC_REVISION);
	    exit (0);
	case 'v':
	    verbose++;
	    break;
	case 'r':
	    recurse++;
	    break;
	case 'h':
	    usage (argv[0], 0);
	default:
	    usage (argv[0], 1);
	}
    }
    i = optind;
#else
    i = 1;
#endif

    config = FcInitLoadConfig ();
    if (!config)
    {
	fprintf (stderr, _("%s: Can't initialize font config library\n"), argv[0]);
	return 1;
    }
    FcConfigSetCurrent (config);
    FcConfigDestroy (config);
    
    args = FcStrSetCreate ();
    if (!args)
    {
	fprintf (stderr, _("%s: malloc failure\n"), argv[0]);
	return 1;
    }
    if (i < argc)
    {
	for (; i < argc; i++)
	{
	    if (!FcStrSetAddFilename (args, (const FcChar8 *) argv[i]))
	    {
		fprintf (stderr, _("%s: malloc failure\n"), argv[0]);
		return 1;
	    }
	}
    }
    else
    {
	recurse++;
	arglist = FcConfigGetFontDirs (config);
	while ((arg = FcStrListNext (arglist)))
	    if (!FcStrSetAdd (args, arg))
	    {
		fprintf (stderr, _("%s: malloc failure\n"), argv[0]);
		return 1;
	    }
	FcStrListDone (arglist);
    }
    arglist = FcStrListCreate (args);
    if (!arglist)
    {
	fprintf (stderr, _("%s: malloc failure\n"), argv[0]);
	return 1;
    }
    FcStrSetDestroy (args);

    while ((arg = FcStrListNext (arglist)))
    {
	int	    j;
	FcChar8	    *cache_file = NULL;
	struct stat file_stat;

	/* reset errno */
	errno = 0;
	if (FcFileIsDir (arg))
	    cache = FcDirCacheLoad (arg, config, &cache_file);
	else
	    cache = FcDirCacheLoadFile (arg, &file_stat);
	if (!cache)
	{
	    if (errno != 0)
		perror ((char *) arg);
	    else
		fprintf (stderr, "%s: Unable to load the cache: %s\n", argv[0], arg);
	    ret++;
	    continue;
	}
	
	dirs = FcStrSetCreate ();
	fs = FcCacheCopySet (cache);
	for (j = 0; j < FcCacheNumSubdir (cache); j++) 
	{
	    FcStrSetAdd (dirs, FcCacheSubdir (cache, j));
	    if (recurse)
		FcStrSetAdd (args, FcCacheSubdir (cache, j));
	}

	if (verbose)
	{
	    if (!first)
		printf ("\n");
	    printf (_("Directory: %s\nCache: %s\n--------\n"),
		    FcCacheDir(cache), cache_file ? cache_file : arg);
	    first = FcFalse;
	}
        cache_print_set (fs, dirs, FcCacheDir (cache), verbose);

	FcStrSetDestroy (dirs);

	FcFontSetDestroy (fs);
	FcDirCacheUnload (cache);
	if (cache_file)
	    FcStrFree (cache_file);
    }
    FcStrListDone (arglist);

    FcFini ();
    return 0;
}
