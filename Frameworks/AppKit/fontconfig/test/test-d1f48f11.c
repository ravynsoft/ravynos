/*
 * fontconfig/test/test-d1f48f11.c
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

int
setenv(const char *name, const char *value, int o)
{
    size_t len = strlen(name) + strlen(value) + 1;
    char *s = malloc(len+1);
    int ret;

    snprintf(s, len, "%s=%s", name, value);
    ret = _putenv(s);
    free(s);
    return ret;
}
#endif

extern FcChar8 *FcConfigRealFilename (FcConfig *, FcChar8 *);
extern FcChar8 *FcStrCanonFilename (const FcChar8 *);

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

char template[512] = "/tmp/fc-d1f48f11-XXXXXX";
char systempl[512] = "/tmp/fc-d1f48f11-XXXXXX";
char *rootdir, *sysroot;

int
setup (char *dir)
{
    FcChar8 *confdir = NULL, *availdir = NULL, *real = NULL, *link = NULL;
    FILE *fp;
    int ret = 1;

    confdir = FcStrBuildFilename (dir, "conf.d", NULL);
    availdir = FcStrBuildFilename (dir, "conf.avail", NULL);
    mkdir_p (confdir);
    mkdir_p (availdir);
    real = FcStrBuildFilename (availdir, "00-foo.conf", NULL);
    link = FcStrBuildFilename (confdir, "00-foo.conf", NULL);
    if (!real || !link)
    {
	fprintf (stderr, "E: unable to allocate memory\n");
	goto bail;
    }
    if ((fp = fopen (real, "wb")) == NULL)
    {
	fprintf (stderr, "E: unable to open a file\n");
	goto bail;
    }
    fprintf (fp, "%s", real);
    fclose (fp);
    if (symlink ("../conf.avail/00-foo.conf", link) != 0)
    {
	fprintf (stderr, "%s: %s\n", link, strerror (errno));
	goto bail;
    }
    ret = 0;
bail:
    if (real)
	free (real);
    if (link)
	free (link);
    if (availdir)
	free (availdir);
    if (confdir)
	free (confdir);

    return ret;
}

void
teardown (const char *dir)
{
    unlink_dirs (dir);
}

int
main (void)
{
    FcConfig *cfg = NULL;
    FcChar8 *dc = NULL, *da = NULL, *d = NULL;
    FcChar8 *ds = NULL, *dsa = NULL, *dsac = NULL;
    int ret = 1;

    rootdir = fc_mkdtemp (template);
    if (!rootdir)
    {
	fprintf (stderr, "%s: %s\n", template, strerror (errno));
	return 1;
    }
    sysroot = fc_mkdtemp (systempl);
    if (!sysroot)
    {
	fprintf (stderr, "%s: %s\n", systempl, strerror (errno));
	return 1;
    }
    ds = FcStrBuildFilename (sysroot, rootdir, NULL);
    
    if (setup (rootdir) != 0)
	goto bail;
    if (setup (ds) != 0)
	goto bail;

    dc = FcStrBuildFilename (rootdir, "conf.d", "00-foo.conf", NULL);
    da = FcStrBuildFilename (rootdir, "conf.avail", "00-foo.conf", NULL);
    cfg = FcConfigCreate ();
    d = FcConfigRealFilename (cfg, dc);
    if (strcmp ((const char *)d, (const char *)da) != 0)
    {
	fprintf (stderr, "E: failed to compare for non-sysroot: %s, %s\n", d, da);
	goto bail;
    }

    free (d);
    FcConfigDestroy (cfg);
    setenv ("FONTCONFIG_SYSROOT", sysroot, 1);
    cfg = FcConfigCreate ();
    dsa = FcStrBuildFilename (sysroot, da, NULL);
    dsac = FcStrCanonFilename (dsa);
    d = FcConfigRealFilename (cfg, dc);
    if (strcmp ((const char *)d, (const char *)dsac) != 0)
    {
	fprintf (stderr, "E: failed to compare for sysroot: %s, %s\n", d, dsac);
	goto bail;
    }

    ret = 0;
bail:
    if (cfg)
	FcConfigDestroy (cfg);
    if (ds)
	free (ds);
    if (dsa)
	free (dsa);
    if (dsac)
	free (dsac);
    if (dc)
	free (dc);
    if (da)
	free (da);
    if (d)
	free (d);
    teardown (sysroot);
    teardown (rootdir);

    return ret;
}
