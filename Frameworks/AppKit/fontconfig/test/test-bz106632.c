/*
 * fontconfig/test/test-bz89617.c
 *
 * Copyright © 2000 Keith Packard
 * Copyright © 2018 Akira TAGOH
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
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifndef HAVE_STRUCT_DIRENT_D_TYPE
#include <sys/types.h>
#include <sys/stat.h>
#endif
#include <fontconfig/fontconfig.h>

#ifdef _WIN32
#  define FC_DIR_SEPARATOR         '\\'
#  define FC_DIR_SEPARATOR_S       "\\"
#else
#  define FC_DIR_SEPARATOR         '/'
#  define FC_DIR_SEPARATOR_S       "/"
#endif

#ifdef _WIN32
#include <direct.h>
#define mkdir(path,mode) _mkdir(path)
#endif

#ifdef HAVE_MKDTEMP
#define fc_mkdtemp	mkdtemp
#else
char *
fc_mkdtemp (char *template)
{
    if (!mktemp (template) || mkdir (template, 0700))
	return NULL;

    return template;
}
#endif

FcBool
mkdir_p (const char *dir)
{
    char *parent;
    FcBool ret;

    if (strlen (dir) == 0)
	return FcFalse;
    parent = (char *) FcStrDirname ((const FcChar8 *) dir);
    if (!parent)
	return FcFalse;
    if (access (parent, F_OK) == 0)
	ret = mkdir (dir, 0755) == 0 && chmod (dir, 0755) == 0;
    else if (access (parent, F_OK) == -1)
	ret = mkdir_p (parent) && (mkdir (dir, 0755) == 0) && chmod (dir, 0755) == 0;
    else
	ret = FcFalse;
    free (parent);

    return ret;
}

FcBool
unlink_dirs (const char *dir)
{
    DIR *d = opendir (dir);
    struct dirent *e;
    size_t len = strlen (dir);
    char *n = NULL;
    FcBool ret = FcTrue;
#ifndef HAVE_STRUCT_DIRENT_D_TYPE
    struct stat statb;
#endif

    if (!d)
	return FcFalse;
    while ((e = readdir (d)) != NULL)
    {
	size_t l;

	if (strcmp (e->d_name, ".") == 0 ||
	    strcmp (e->d_name, "..") == 0)
	    continue;
	l = strlen (e->d_name) + 1;
	if (n)
	    free (n);
	n = malloc (l + len + 1);
	if (!n)
	{
	    ret = FcFalse;
	    break;
	}
	strcpy (n, dir);
	n[len] = FC_DIR_SEPARATOR;
	strcpy (&n[len + 1], e->d_name);
#ifdef HAVE_STRUCT_DIRENT_D_TYPE
	if (e->d_type == DT_DIR)
#else
	if (stat (n, &statb) == -1)
	{
	    fprintf (stderr, "E: %s\n", n);
	    ret = FcFalse;
	    break;
	}
	if (S_ISDIR (statb.st_mode))
#endif
	{
	    if (!unlink_dirs (n))
	    {
		fprintf (stderr, "E: %s\n", n);
		ret = FcFalse;
		break;
	    }
	}
	else
	{
	    if (unlink (n) == -1)
	    {
		fprintf (stderr, "E: %s\n", n);
		ret = FcFalse;
		break;
	    }
	}
    }
    if (n)
	free (n);
    closedir (d);

    if (rmdir (dir) == -1)
    {
	fprintf (stderr, "E: %s\n", dir);
	return FcFalse;
    }

    return ret;
}

int
main (void)
{
    FcChar8 *fontdir = NULL, *cachedir = NULL;
    char *basedir, template[512] = "/tmp/bz106632-XXXXXX";
    char cmd[512];
    FcConfig *config;
    const FcChar8 *tconf = (const FcChar8 *) "<fontconfig>\n"
	"  <dir>%s</dir>\n"
	"  <cachedir>%s</cachedir>\n"
	"</fontconfig>\n";
    char conf[1024];
    int ret = 0;
    FcFontSet *fs;
    FcPattern *pat;

    fprintf (stderr, "D: Creating tmp dir\n");
    basedir = fc_mkdtemp (template);
    if (!basedir)
    {
	fprintf (stderr, "%s: %s\n", template, strerror (errno));
	goto bail;
    }
    fontdir = FcStrBuildFilename ((const FcChar8 *) basedir, (const FcChar8 *) "fonts", NULL);
    cachedir = FcStrBuildFilename ((const FcChar8 *) basedir, (const FcChar8 *) "cache", NULL);
    fprintf (stderr, "D: Creating %s\n", fontdir);
    mkdir_p ((const char *) fontdir);
    fprintf (stderr, "D: Creating %s\n", cachedir);
    mkdir_p ((const char *) cachedir);

    fprintf (stderr, "D: Copying %s to %s\n", FONTFILE, fontdir);
    snprintf (cmd, 512, "sleep 1; cp -a %s %s; sleep 1", FONTFILE, fontdir);
    (void) system (cmd);

    fprintf (stderr, "D: Loading a config\n");
    snprintf (conf, 1024, (const char *) tconf, fontdir, cachedir);
    config = FcConfigCreate ();
    if (!FcConfigParseAndLoadFromMemory (config, (const FcChar8 *) conf, FcTrue))
    {
	printf ("E: Unable to load config\n");
	ret = 1;
	goto bail;
    }
    if (!FcConfigBuildFonts (config))
    {
	printf ("E: unable to build fonts\n");
	ret = 1;
	goto bail;
    }
    fprintf (stderr, "D: Obtaining fonts information\n");
    pat = FcPatternCreate ();
    fs = FcFontList (config, pat, NULL);
    FcPatternDestroy (pat);
    if (!fs || fs->nfont != 1)
    {
	printf ("E: Unexpected the number of fonts: %d\n", !fs ? -1 : fs->nfont);
	ret = 1;
	goto bail;
    }
    FcFontSetDestroy (fs);
    fprintf (stderr, "D: Removing %s\n", fontdir);
    snprintf (cmd, 512, "sleep 1; rm -f %s%s*; sleep 1", fontdir, FC_DIR_SEPARATOR_S);
    (void) system (cmd);
    fprintf (stderr, "D: Reinitializing\n");
    if (FcConfigUptoDate(config))
    {
	fprintf (stderr, "E: Config reports up-to-date\n");
	ret = 2;
	goto bail;
    }
    if (!FcInitReinitialize ())
    {
	fprintf (stderr, "E: Unable to reinitialize\n");
	ret = 3;
	goto bail;
    }
    if (FcConfigGetCurrent () == config)
    {
	fprintf (stderr, "E: config wasn't reloaded\n");
	ret = 3;
	goto bail;
    }
    FcConfigDestroy (config);

    config = FcConfigCreate ();
    if (!FcConfigParseAndLoadFromMemory (config, (const FcChar8 *) conf, FcTrue))
    {
	printf ("E: Unable to load config again\n");
	ret = 4;
	goto bail;
    }
    if (!FcConfigBuildFonts (config))
    {
	printf ("E: unable to build fonts again\n");
	ret = 5;
	goto bail;
    }
    fprintf (stderr, "D: Obtaining fonts information again\n");
    pat = FcPatternCreate ();
    fs = FcFontList (config, pat, NULL);
    FcPatternDestroy (pat);
    if (!fs || fs->nfont != 0)
    {
	printf ("E: Unexpected the number of fonts: %d\n", !fs ? -1 : fs->nfont);
	ret = 1;
	goto bail;
    }
    FcFontSetDestroy (fs);
    fprintf (stderr, "D: Copying %s to %s\n", FONTFILE, fontdir);
    snprintf (cmd, 512, "sleep 1; cp -a %s %s; sleep 1", FONTFILE, fontdir);
    (void) system (cmd);
    fprintf (stderr, "D: Reinitializing\n");
    if (FcConfigUptoDate(config))
    {
	fprintf (stderr, "E: Config up-to-date after addition\n");
	ret = 3;
	goto bail;
    }
    if (!FcInitReinitialize ())
    {
	fprintf (stderr, "E: Unable to reinitialize\n");
	ret = 2;
	goto bail;
    }
    if (FcConfigGetCurrent () == config)
    {
	fprintf (stderr, "E: config wasn't reloaded\n");
	ret = 3;
	goto bail;
    }
    FcConfigDestroy (config);

    config = FcConfigCreate ();
    if (!FcConfigParseAndLoadFromMemory (config, (const FcChar8 *) conf, FcTrue))
    {
	printf ("E: Unable to load config again\n");
	ret = 4;
	goto bail;
    }
    if (!FcConfigBuildFonts (config))
    {
	printf ("E: unable to build fonts again\n");
	ret = 5;
	goto bail;
    }
    fprintf (stderr, "D: Obtaining fonts information\n");
    pat = FcPatternCreate ();
    fs = FcFontList (config, pat, NULL);
    FcPatternDestroy (pat);
    if (!fs || fs->nfont != 1)
    {
	printf ("E: Unexpected the number of fonts: %d\n", !fs ? -1 : fs->nfont);
	ret = 1;
	goto bail;
    }
    FcFontSetDestroy (fs);
    FcConfigDestroy (config);

bail:
    fprintf (stderr, "Cleaning up\n");
    if (basedir)
	unlink_dirs (basedir);
    if (fontdir)
	FcStrFree (fontdir);
    if (cachedir)
	FcStrFree (cachedir);

    return ret;
}
