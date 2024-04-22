/*
 * fontconfig/fc-list/fc-list.c
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
const struct option longopts[] = {
    {"verbose", 0, 0, 'v'},
    {"brief", 0, 0, 'b'},
    {"format", 1, 0, 'f'},
    {"quiet", 0, 0, 'q'},
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
    fprintf (file, _("usage: %s [-vbqVh] [-f FORMAT] [--verbose] [--brief] [--format=FORMAT] [--quiet] [--version] [--help] [pattern] {element ...} \n"),
	     program);
#else
    fprintf (file, _("usage: %s [-vbqVh] [-f FORMAT] [pattern] {element ...} \n"),
	     program);
#endif
    fprintf (file, _("List fonts matching [pattern]\n"));
    fprintf (file, "\n");
#if HAVE_GETOPT_LONG
    fprintf (file, _("  -v, --verbose        display entire font pattern verbosely\n"));
    fprintf (file, _("  -b, --brief          display entire font pattern briefly\n"));
    fprintf (file, _("  -f, --format=FORMAT  use the given output format\n"));
    fprintf (file, _("  -q, --quiet          suppress all normal output, exit 1 if no fonts matched\n"));
    fprintf (file, _("  -V, --version        display font config version and exit\n"));
    fprintf (file, _("  -h, --help           display this help and exit\n"));
#else
    fprintf (file, _("  -v         (verbose) display entire font pattern verbosely\n"));
    fprintf (file, _("  -b         (brief)   display entire font pattern briefly\n"));
    fprintf (file, _("  -f FORMAT  (format)  use the given output format\n"));
    fprintf (file, _("  -q,        (quiet)   suppress all normal output, exit 1 if no fonts matched\n"));
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
    int			quiet = 0;
    const FcChar8	*format = NULL;
    int			nfont = 0;
    int			i;
    FcObjectSet		*os = 0;
    FcFontSet		*fs;
    FcPattern		*pat;
#if HAVE_GETOPT_LONG || HAVE_GETOPT
    int			c;

    setlocale (LC_ALL, "");
#if HAVE_GETOPT_LONG
    while ((c = getopt_long (argc, argv, "vbf:qVh", longopts, NULL)) != -1)
#else
    while ((c = getopt (argc, argv, "vbf:qVh")) != -1)
#endif
    {
	switch (c) {
	case 'v':
	    verbose = 1;
	    break;
	case 'b':
	    brief = 1;
	    break;
	case 'f':
	    format = (FcChar8 *) strdup (optarg);
	    break;
	case 'q':
	    quiet = 1;
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
    if (quiet && !os)
	os = FcObjectSetCreate ();
    if (!verbose && !brief && !format && !os)
	os = FcObjectSetBuild (FC_FAMILY, FC_STYLE, FC_FILE, (char *) 0);
    if (!format)
        format = (const FcChar8 *) "%{=fclist}\n";
    fs = FcFontList (0, pat, os);
    if (os)
	FcObjectSetDestroy (os);
    if (pat)
	FcPatternDestroy (pat);

    if (!quiet && fs)
    {
	int	j;

	for (j = 0; j < fs->nfont; j++)
	{
	    if (verbose || brief)
	    {
		if (brief)
		{
		    FcPatternDel (fs->fonts[j], FC_CHARSET);
		    FcPatternDel (fs->fonts[j], FC_LANG);
		}
		FcPatternPrint (fs->fonts[j]);
	    }
	    else
	    {
	        FcChar8 *s;

		s = FcPatternFormat (fs->fonts[j], format);
		if (s)
		{
		    printf ("%s", s);
		    FcStrFree (s);
		}
	    }
	}
    }

    if (fs) {
	nfont = fs->nfont;
	FcFontSetDestroy (fs);
    }

    FcFini ();

    return quiet ? (nfont == 0 ? 1 : 0) : 0;
}
