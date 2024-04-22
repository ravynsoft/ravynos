/*
 * fontconfig/test/test-issue110.c
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
main(void)
{
    FcConfig *cfg = FcConfigCreate ();
    char *basedir, template[512] = "/tmp/fc110-XXXXXX";
    char *sysroot, systempl[512] = "/tmp/fc110-XXXXXX";
    FcChar8 *d = NULL;
    FcChar8 *ret = NULL;
    FcChar8 *s = NULL;
    FILE *fp;
    int retval = 0;

    retval++;
    basedir = fc_mkdtemp (template);
    if (!basedir)
    {
	fprintf (stderr, "%s: %s\n", template, strerror (errno));
	goto bail;
    }
    retval++;
    sysroot = fc_mkdtemp (systempl);
    if (!sysroot)
    {
	fprintf (stderr, "%s: %s\n", systempl, strerror (errno));
	goto bail;
    }
    fprintf (stderr, "D: Creating %s\n", basedir);
    mkdir_p (basedir);
    setenv ("HOME", basedir, 1);
    retval++;
    s = FcStrBuildFilename (basedir, ".fonts.conf", NULL);
    if (!s)
	goto bail;
    retval++;
    fprintf (stderr, "D: Creating %s\n", s);
    if ((fp = fopen (s, "wb")) == NULL)
	goto bail;
    fprintf (fp, "%s", s);
    fclose (fp);
    retval++;
    fprintf (stderr, "D: Checking file path\n");
    ret = FcConfigRealFilename (cfg, "~/.fonts.conf");
    if (!ret)
	goto bail;
    retval++;
    if (strcmp ((const char *) s, (const char *) ret) != 0)
	goto bail;
    free (ret);
    free (s);
    FcConfigDestroy (cfg);
    setenv ("FONTCONFIG_SYSROOT", sysroot, 1);
    cfg = FcConfigCreate ();
    fprintf (stderr, "D: Creating %s\n", sysroot);
    mkdir_p (sysroot);
    retval++;
    d = FcStrBuildFilename (sysroot, basedir, NULL);
    fprintf (stderr, "D: Creating %s\n", d);
    mkdir_p (d);
    free (d);
    s = FcStrBuildFilename (sysroot, basedir, ".fonts.conf", NULL);
    if (!s)
	goto bail;
    retval++;
    fprintf (stderr, "D: Creating %s\n", s);
    if ((fp = fopen (s, "wb")) == NULL)
	goto bail;
    fprintf (fp, "%s", s);
    fclose (fp);
    retval++;
    fprintf (stderr, "D: Checking file path\n");
    ret = FcConfigRealFilename (cfg, "~/.fonts.conf");
    if (!ret)
	goto bail;
    retval++;
    if (strcmp ((const char *) s, (const char *) ret) != 0)
	goto bail;
    retval = 0;
bail:
    fprintf (stderr, "Cleaning up\n");
    unlink_dirs (basedir);
    if (ret)
	free (ret);
    if (s)
	free (s);

    return retval;
}

