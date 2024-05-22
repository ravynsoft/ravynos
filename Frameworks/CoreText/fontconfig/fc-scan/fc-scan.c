/*
 * fontconfig/fc-scan/fc-scan.c
 *
 * Copyright © 2003 Keith Packard
 * Copyright © 2008 Red Hat, Inc.
 * Red Hat Author(s): Behdad Esfahbod
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
#include <fontconfig/fcfreetype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

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
static const struct option longopts[] = {
    {"brief", 0, 0, 'b'},
    {"format", 1, 0, 'f'},
    {"sysroot", required_argument, 0, 'y'},
    {"version", 0, 0, 'V'},
    {"help", 0, 0, 'h'},
    {NULL,0,0,0},
};
#else
#if HAVE_GETOPT
extern char *optarg;
extern int optind, opterr, optopt;
#endif
#endif

static void
usage (char *program, int error)
{
    FILE *file = error ? stderr : stdout;
#if HAVE_GETOPT_LONG
    fprintf (file, _("usage: %s [-bcVh] [-f FORMAT] [-y SYSROOT] [--brief] [--format FORMAT] [--version] [--help] font-file...\n"),
	     program);
#else
    fprintf (file, _("usage: %s [-bcVh] [-f FORMAT] [-y SYSROOT] font-file...\n"),
	     program);
#endif
    fprintf (file, _("Scan font files and directories, and print resulting pattern(s)\n"));
    fprintf (file, "\n");
#if HAVE_GETOPT_LONG
    fprintf (file, _("  -b, --brief            display font pattern briefly\n"));
    fprintf (file, _("  -f, --format=FORMAT    use the given output format\n"));
    fprintf (file, _("  -y, --sysroot=SYSROOT  prepend SYSROOT to all paths for scanning\n"));
    fprintf (file, _("  -V, --version          display font config version and exit\n"));
    fprintf (file, _("  -h, --help             display this help and exit\n"));
#else
    fprintf (file, _("  -b         (brief)         display font pattern briefly\n"));
    fprintf (file, _("  -f FORMAT  (format)        use the given output format\n"));
    fprintf (file, _("  -y SYSROOT (sysroot)       prepend SYSROOT to all paths for scanning\n"));
    fprintf (file, _("  -V         (version)       display font config version and exit\n"));
    fprintf (file, _("  -h         (help)          display this help and exit\n"));
#endif
    exit (error);
}

int
main (int argc, char **argv)
{
    int         brief = 0;
    FcChar8     *format = NULL, *sysroot = NULL;
    int		i;
    FcFontSet   *fs;
#if HAVE_GETOPT_LONG || HAVE_GETOPT
    int		c;

    setlocale (LC_ALL, "");
#if HAVE_GETOPT_LONG
    while ((c = getopt_long (argc, argv, "bf:y:Vh", longopts, NULL)) != -1)
#else
    while ((c = getopt (argc, argv, "bf:y:Vh")) != -1)
#endif
    {
	switch (c) {
	case 'b':
	    brief = 1;
	    break;
	case 'f':
	    format = (FcChar8 *) strdup (optarg);
	    break;
	case 'y':
	    sysroot = FcStrCopy ((const FcChar8 *) optarg);
	    break;
	case 'V':
	    fprintf (stderr, "fontconfig version %d.%d.%d\n",
		     FC_MAJOR, FC_MINOR, FC_REVISION);
	    exit (0);
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

    if (i == argc)
	usage (argv[0], 1);

    if (sysroot)
    {
	FcConfigSetSysRoot (NULL, sysroot);
	FcStrFree (sysroot);
    }
    fs = FcFontSetCreate ();

    for (; i < argc; i++)
    {
	const FcChar8 *file = (FcChar8*) argv[i];

	if (!FcFileIsDir (file))
	    FcFileScan (fs, NULL, NULL, NULL, file, FcTrue);
	else
	{
	    FcStrSet *dirs = FcStrSetCreate ();
	    FcStrList *strlist = FcStrListCreate (dirs);
	    do
	    {
		FcDirScan (fs, dirs, NULL, NULL, file, FcTrue);
	    }
	    while ((file = FcStrListNext (strlist)));
	    FcStrListDone (strlist);
	    FcStrSetDestroy (dirs);
	}
    }

    for (i = 0; i < fs->nfont; i++)
    {
	FcPattern *pat = fs->fonts[i];

	if (brief)
	{
	    FcPatternDel (pat, FC_CHARSET);
	    FcPatternDel (pat, FC_LANG);
	}

	if (format)
	{
	    FcChar8 *s;

	    s = FcPatternFormat (pat, format);
	    if (s)
	    {
		printf ("%s", s);
		FcStrFree (s);
	    }
	}
	else
	{
	    FcPatternPrint (pat);
	}
    }

    FcFontSetDestroy (fs);

    FcFini ();
    return i > 0 ? 0 : 1;
}
