/*
 * fontconfig/fc-match/fc-match.c
 *
 * Copyright © 2003 Keith Packard
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
    {"sort", 0, 0, 's'},
    {"all", 0, 0, 'a'},
    {"verbose", 0, 0, 'v'},
    {"brief", 0, 0, 'b'},
    {"format", 1, 0, 'f'},
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
    fprintf (file, _("usage: %s [-savbVh] [-f FORMAT] [--sort] [--all] [--verbose] [--brief] [--format=FORMAT] [--version] [--help] [pattern] {element...}\n"),
	     program);
#else
    fprintf (file, _("usage: %s [-savVh] [-f FORMAT] [pattern] {element...}\n"),
	     program);
#endif
    fprintf (file, _("List best font matching [pattern]\n"));
    fprintf (file, "\n");
#if HAVE_GETOPT_LONG
    fprintf (file, _("  -s, --sort           display sorted list of matches\n"));
    fprintf (file, _("  -a, --all            display unpruned sorted list of matches\n"));
    fprintf (file, _("  -v, --verbose        display entire font pattern verbosely\n"));
    fprintf (file, _("  -b, --brief          display entire font pattern briefly\n"));
    fprintf (file, _("  -f, --format=FORMAT  use the given output format\n"));
    fprintf (file, _("  -V, --version        display font config version and exit\n"));
    fprintf (file, _("  -h, --help           display this help and exit\n"));
#else
    fprintf (file, _("  -s,        (sort)    display sorted list of matches\n"));
    fprintf (file, _("  -a         (all)     display unpruned sorted list of matches\n"));
    fprintf (file, _("  -v         (verbose) display entire font pattern verbosely\n"));
    fprintf (file, _("  -b         (brief)   display entire font pattern briefly\n"));
    fprintf (file, _("  -f FORMAT  (format)  use the given output format\n"));
    fprintf (file, _("  -V         (version) display font config version and exit\n"));
    fprintf (file, _("  -h         (help)    display this help and exit\n"));
#endif
    exit (error);
}

int
main (int argc, char **argv)
{
    int			verbose = 0;
    int			brief = 0;
    int			sort = 0, all = 0;
    const FcChar8	*format = NULL;
    const FcChar8	*format_optarg = NULL;
    int			i;
    FcObjectSet		*os = 0;
    FcFontSet		*fs;
    FcPattern		*pat;
    FcResult		result;
#if HAVE_GETOPT_LONG || HAVE_GETOPT
    int			c;

    setlocale (LC_ALL, "");
#if HAVE_GETOPT_LONG
    while ((c = getopt_long (argc, argv, "asvbf:Vh", longopts, NULL)) != -1)
#else
    while ((c = getopt (argc, argv, "asvbf:Vh")) != -1)
#endif
    {
	switch (c) {
	case 'a':
	    all = 1;
	    break;
	case 's':
	    sort = 1;
	    break;
	case 'v':
	    verbose = 1;
	    break;
	case 'b':
	    brief = 1;
	    break;
	case 'f':
	    format = format_optarg = (FcChar8 *) strdup (optarg);
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

    if (argv[i])
    {
	pat = FcNameParse ((FcChar8 *) argv[i]);
	if (!pat)
	{
	    fprintf (stderr, _("Unable to parse the pattern\n"));
	    return 1;
	}
	while (argv[++i])
	{
	    if (!os)
		os = FcObjectSetCreate ();
	    FcObjectSetAdd (os, argv[i]);
	}
    }
    else
	pat = FcPatternCreate ();

    if (!pat)
	return 1;

    FcConfigSubstitute (0, pat, FcMatchPattern);
    FcDefaultSubstitute (pat);
    
    fs = FcFontSetCreate ();

    if (sort || all)
    {
	FcFontSet	*font_patterns;
	int	j;
	font_patterns = FcFontSort (0, pat, all ? FcFalse : FcTrue, 0, &result);

	if (!font_patterns || font_patterns->nfont == 0)
	{
	    fprintf (stderr, _("No fonts installed on the system\n"));
	    return 1;
	}
	for (j = 0; j < font_patterns->nfont; j++)
	{
	    FcPattern  *font_pattern;

	    font_pattern = FcFontRenderPrepare (NULL, pat, font_patterns->fonts[j]);
	    if (font_pattern)
		FcFontSetAdd (fs, font_pattern);
	}

	FcFontSetSortDestroy (font_patterns);
    }
    else
    {
	FcPattern   *match;
	match = FcFontMatch (0, pat, &result);
	if (match)
	    FcFontSetAdd (fs, match);
    }
    FcPatternDestroy (pat);

    if (!format)
    {
	if (os)
	    format = (const FcChar8 *) "%{=unparse}\n";
	else
	    format = (const FcChar8 *) "%{=fcmatch}\n";
    }

    if (fs)
    {
	int	j;

	for (j = 0; j < fs->nfont; j++)
	{
	    FcPattern *font;

	    font = FcPatternFilter (fs->fonts[j], os);

	    if (verbose || brief)
	    {
		if (brief)
		{
		    FcPatternDel (font, FC_CHARSET);
		    FcPatternDel (font, FC_LANG);
		}
		FcPatternPrint (font);
	    }
	    else
	    {
	        FcChar8 *s;

		s = FcPatternFormat (font, format);
		if (s)
		{
		    printf ("%s", s);
		    FcStrFree (s);
		}
	    }

	    FcPatternDestroy (font);
	}
	FcFontSetDestroy (fs);
    }

    if (os)
	FcObjectSetDestroy (os);

    FcFini ();

    if (format_optarg) {
	free ((void*)format_optarg);
	format_optarg = NULL;
    }

    return 0;
}
