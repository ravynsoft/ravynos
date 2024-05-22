/*
 * fontconfig/fc-pattern/fc-pattern.c
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
    {"config", 0, 0, 'c'},
    {"default", 0, 0, 'd'},
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
    fprintf (file, _("usage: %s [-cdVh] [-f FORMAT] [--config] [--default] [--verbose] [--format=FORMAT] [--version] [--help] [pattern] {element...}\n"),
	     program);
#else
    fprintf (file, _("usage: %s [-cdVh] [-f FORMAT] [pattern] {element...}\n"),
	     program);
#endif
    fprintf (file, _("List best font matching [pattern]\n"));
    fprintf (file, "\n");
#if HAVE_GETOPT_LONG
    fprintf (file, _("  -c, --config         perform config substitution on pattern\n"));
    fprintf (file, _("  -d, --default        perform default substitution on pattern\n"));
    fprintf (file, _("  -f, --format=FORMAT  use the given output format\n"));
    fprintf (file, _("  -V, --version        display font config version and exit\n"));
    fprintf (file, _("  -h, --help           display this help and exit\n"));
#else
    fprintf (file, _("  -c,        (config)  perform config substitution on pattern\n"));
    fprintf (file, _("  -d,        (default) perform default substitution on pattern\n"));
    fprintf (file, _("  -f FORMAT  (format)  use the given output format\n"));
    fprintf (file, _("  -V         (version) display font config version and exit\n"));
    fprintf (file, _("  -h         (help)    display this help and exit\n"));
#endif
    exit (error);
}

int
main (int argc, char **argv)
{
    int		do_config = 0, do_default = 0;
    FcChar8     *format = NULL;
    int		i;
    FcObjectSet *os = 0;
    FcPattern   *pat;
#if HAVE_GETOPT_LONG || HAVE_GETOPT
    int		c;

    setlocale (LC_ALL, "");
#if HAVE_GETOPT_LONG
    while ((c = getopt_long (argc, argv, "cdf:Vh", longopts, NULL)) != -1)
#else
    while ((c = getopt (argc, argv, "cdf:Vh")) != -1)
#endif
    {
	switch (c) {
	case 'c':
	    do_config = 1;
	    break;
	case 'd':
	    do_default = 1;
	    break;
	case 'f':
	    format = (FcChar8 *) strdup (optarg);
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

    if (do_config)
      FcConfigSubstitute (0, pat, FcMatchPattern);
    if (do_default)
      FcDefaultSubstitute (pat);

    if (os)
    {
      FcPattern *new;
      new = FcPatternFilter (pat, os);
      FcPatternDestroy (pat);
      pat = new;
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

    FcPatternDestroy (pat);

    if (os)
	FcObjectSetDestroy (os);

    FcFini ();

    return 0;
}
