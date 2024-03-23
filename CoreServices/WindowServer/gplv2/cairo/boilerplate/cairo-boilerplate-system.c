/*
 * Copyright © 2004 Red Hat, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Red Hat, Inc. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. Red Hat, Inc. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * RED HAT, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL RED HAT, INC. BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Carl D. Worth <cworth@cworth.org>
 */

#include "cairo-boilerplate.h"
#include "cairo-boilerplate-system.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <errno.h>

void *
xmalloc (size_t size)
{
    void *buf;

    if (size == 0)
	return NULL;

    buf = malloc (size);
    if (buf == NULL) {
	fprintf (stderr, "Error: Out of memory. Exiting.\n");
	exit (1);
    }

    return buf;
}

void *
xcalloc (size_t nmemb,
	 size_t size)
{
    void *buf;

    if (nmemb == 0 || size == 0)
	return NULL;

    buf = calloc (nmemb, size);
    if (buf == NULL) {
	fprintf (stderr, "Error: Out of memory. Exiting\n");
	exit (1);
    }

    return buf;
}

void *
xrealloc (void	 *buf,
	  size_t  size)
{
    buf = realloc (buf, size);
    if (buf == NULL && size != 0) {
	fprintf (stderr, "Error: Out of memory. Exiting\n");
	exit (1);
    }

    return buf;
}

void
xasprintf (char       **strp,
	   const char  *fmt,
			...)
{
#ifdef HAVE_VASPRINTF
    va_list va;
    int ret;

    va_start (va, fmt);
    ret = vasprintf (strp, fmt, va);
    va_end (va);

    if (ret < 0) {
	fprintf (stderr, "Error: Out of memory. Exiting.\n");
	exit (1);
    }
#else /* !HAVE_VASNPRINTF */
#define BUF_SIZE 1024
    va_list va;
    char buffer[BUF_SIZE];
    int ret, len;

    va_start (va, fmt);
    ret = vsnprintf (buffer, sizeof (buffer), fmt, va);
    va_end (va);

    if (ret < 0) {
	fprintf (stderr, "Failure in vsnprintf\n");
	exit (1);
    }

    len = (ret + sizeof (int)) & -sizeof (int);
    *strp = malloc (len);
    if (*strp == NULL) {
	fprintf (stderr, "Out of memory\n");
	exit (1);
    }

    if ((unsigned) ret < sizeof (buffer)) {
	memcpy (*strp, buffer, ret);
    } else {
	va_start (va, fmt);
	ret = vsnprintf (*strp, len, fmt, va);
	va_end (va);

	if (ret >= len) {
	    free (*strp);
	    fprintf (stderr, "Overflowed dynamic buffer\n");
	    exit (1);
	}
    }
    memset (*strp + ret, 0, len-ret);
#endif /* !HAVE_VASNPRINTF */
}

void
xunlink (const char *pathname)
{
    if (unlink (pathname) < 0 && errno != ENOENT) {
	fprintf (stderr, "Error: Cannot remove %s: %s\n",
			       pathname, strerror (errno));
	exit (1);
    }
}

char *
xstrdup (const char *str)
{
    if (str == NULL)
	return NULL;

    str = strdup (str);
    if (str == NULL) {
	fprintf (stderr, "Error: Out of memory. Exiting.\n");
	exit (1);
    }

    return (char *) str;
}
