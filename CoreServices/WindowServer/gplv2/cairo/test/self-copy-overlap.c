/*
 * Copyright 2008 Chris Wilson
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

/* Question: are patterns mutable? The answer depends on who you ask... */

#include "cairo-test.h"

/* This test is only interesting if the target has alpha */
static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_set_source_rgba (cr, 0, 0, 0, .01);
    cairo_paint (cr);

    cairo_set_source_surface (cr, cairo_get_target (cr), 1, 1);
    cairo_paint (cr);

    return CAIRO_TEST_SUCCESS;
}

/*
 * XFAIL: vector surfaces take snapshot of patterns in contrast to the raster
 * backends which don't. One solution would be to clone overlapping areas of
 * dst/source, so patterns were effectively snapshotted across all backends.
 */
CAIRO_TEST (self_copy_overlap,
	    "Tests painting to itself using itself as the source"
	    "\nBackends treat this case inconsistently---vector backends are creating snapshots.",
	    "self-copy", /* keywords */
	    NULL, /* requirements */
	    200, 200,
	    NULL, draw)
