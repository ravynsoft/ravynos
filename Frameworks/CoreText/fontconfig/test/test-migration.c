/*
 * fontconfig/test/test-migration.c
 *
 * Copyright © 2000 Keith Packard
 * Copyright © 2013 Akira TAGOH
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
#include <sys/types.h>
#include <dirent.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifndef HAVE_STRUCT_DIRENT_D_TYPE
#include <sys/types.h>
#include <sys/stat.h>
#endif
#include <fontconfig/fontconfig.h>

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
mkdir_p(const char *dir)
{
    char *parent;
    FcBool ret;

    if (strlen (dir) == 0)
	return FcFalse;
    parent = (char *) FcStrDirname ((const FcChar8 *)dir);
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
unlink_dirs(const char *dir)
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
    while ((e = readdir(d)) != NULL)
    {
	size_t l;

	if (strcmp (e->d_name, ".") == 0 ||
	    strcmp (e->d_name, "..") == 0)
	    continue;
	l = strlen (e->d_name) + 1;
	if (n)
	    free (n);
	n = malloc (l + len + 1);
	strcpy (n, dir);
	n[len] = '/';
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
    char template[32] = "fontconfig-XXXXXXXX";
    char *tmp = fc_mkdtemp (template);
    size_t len = strlen (tmp), xlen, dlen;
    char xdg[256], confd[256], fn[256], nfn[256], ud[256], nud[256];
    int ret = -1;
    FILE *fp;
    char *content = "<fontconfig></fontconfig>";

    strcpy (xdg, tmp);
    strcpy (&xdg[len], "/.config");
    setenv ("HOME", tmp, 1);
    setenv ("XDG_CONFIG_HOME", xdg, 1);
    xlen = strlen (xdg);
    strcpy (confd, xdg);
    strcpy (&confd[xlen], "/fontconfig");
    dlen = strlen (confd);
    /* In case there are no configuration files nor directory */
    FcInit ();
    if (access (confd, F_OK) == 0)
    {
	fprintf (stderr, "%s unexpectedly exists\n", confd);
	goto bail;
    }
    FcFini ();
    if (!unlink_dirs (tmp))
    {
	fprintf (stderr, "Unable to clean up\n");
	goto bail;
    }
    /* In case there are the user configuration file */
    strcpy (fn, tmp);
    strcpy (&fn[len], "/.fonts.conf");
    strcpy (nfn, confd);
    strcpy (&nfn[dlen], "/fonts.conf");
    if (!mkdir_p (confd))
    {
	fprintf (stderr, "Unable to create a config dir: %s\n", confd);
	goto bail;
    }
    if ((fp = fopen (fn, "wb")) == NULL)
    {
	fprintf (stderr, "Unable to create a config file: %s\n", fn);
	goto bail;
    }
    fwrite (content, sizeof (char), strlen (content), fp);
    fclose (fp);
    FcInit ();
    if (access (nfn, F_OK) != 0)
    {
	fprintf (stderr, "migration failed for %s\n", nfn);
	goto bail;
    }
    FcFini ();
    if (!unlink_dirs (tmp))
    {
	fprintf (stderr, "Unable to clean up\n");
	goto bail;
    }
    /* In case there are the user configuration dir */
    strcpy (ud, tmp);
    strcpy (&ud[len], "/.fonts.conf.d");
    strcpy (nud, confd);
    strcpy (&nud[dlen], "/conf.d");
    if (!mkdir_p (ud))
    {
	fprintf (stderr, "Unable to create a config dir: %s\n", ud);
	goto bail;
    }
    FcInit ();
    if (access (nud, F_OK) != 0)
    {
	fprintf (stderr, "migration failed for %s\n", nud);
	goto bail;
    }
    FcFini ();

    ret = 0;
bail:
    unlink_dirs (tmp);

    return ret;
}
