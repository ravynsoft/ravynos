/* -*- Mode: c; c-basic-offset: 4; indent-tabs-mode: t; tab-width: 8; -*- */
/*
 * Copyright 2011 Andrea Canciani
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * the authors not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. The authors make no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE AUTHORS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Authors: Andrea Canciani <ranma42@gmail.com>
 */

#include "cairo-compiler-private.h"

#include <windows.h>

#define stat _stat

#define S_ISDIR(s) ((s) & _S_IFDIR)

struct dirent {
    ino_t d_ino;
    char d_name[FILENAME_MAX + 1];
};

typedef struct _DIR {
    HANDLE handle;
    cairo_bool_t has_next;
    WIN32_FIND_DATA data;
    struct dirent de;
} DIR;

static DIR *
opendir(const char *dirname)
{
    DIR *dirp;

    dirp = malloc (sizeof (*dirp));
    if (unlikely (dirp == NULL))
	return NULL;

    dirp->handle = FindFirstFile(dirname, &dirp->data);

    if (unlikely (dirp->handle == INVALID_HANDLE_VALUE)) {
	free (dirp);
	return NULL;
    }

    memcpy (dirp->de.d_name, dirp->data.cFileName,
	    sizeof (dirp->data.cFileName));
    dirp->de.d_name[FILENAME_MAX] = '\0';

    dirp->has_next = TRUE;

    return dirp;
}

static int
closedir(DIR *dirp)
{
    int ret;

    ret = ! FindClose (dirp->handle);

    free (dirp);

    /* TODO: set errno */

    return ret;
}

static struct dirent *
readdir(DIR *dirp)
{
    if (! dirp->has_next)
	return NULL;

    /* COMPILE_TIME_ASSERT (FILENAME_MAX == sizeof (dirp->data.cFileName)); */

    memcpy (dirp->de.d_name, dirp->data.cFileName,
	    sizeof (dirp->data.cFileName));
    dirp->de.d_name[FILENAME_MAX] = '\0';

    dirp->has_next = FindNextFile (dirp->handle, &dirp->data);

    return &dirp->de;
}
