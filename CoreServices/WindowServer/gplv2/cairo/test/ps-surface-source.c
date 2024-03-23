/*
 * Copyright © 2008 Chris Wilson
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Chris Wilson not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. Chris Wilson makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * CHRIS WILSON DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL CHRIS WILSON BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Chris Wilson <chris@chris-wilson.co.uk>
 */

#include "cairo-test.h"
#include <cairo-ps.h>

#include "surface-source.c"

#define BASENAME "ps-surface-source.out"

static cairo_surface_t *
create_source_surface (int size)
{
    cairo_surface_t *surface;
    char *filename;
    const char *path = cairo_test_mkdir (CAIRO_TEST_OUTPUT_DIR) ? CAIRO_TEST_OUTPUT_DIR : ".";

    xasprintf (&filename, "%s/%s.ps", path, BASENAME);
    surface = cairo_ps_surface_create (filename, size, size);
    cairo_surface_set_fallback_resolution (surface, 72., 72.);
    free (filename);

    return surface;
}

CAIRO_TEST (ps_surface_source,
	    "Test using a PS surface as the source",
	    "source", /* keywords */
	    NULL, /* requirements */
	    SIZE, SIZE,
	    preamble, draw)
