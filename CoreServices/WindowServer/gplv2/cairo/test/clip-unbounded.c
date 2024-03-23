/*
 * Copyright © 2009 Chris Wilson
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

#define SIZE 10

static cairo_surface_t *
create_source (cairo_surface_t *target)
{
    cairo_surface_t *similar;
    cairo_t *cr;

    similar = cairo_surface_create_similar (target,
					    CAIRO_CONTENT_COLOR, SIZE/2, SIZE);
    cr = cairo_create (similar);
    cairo_surface_destroy (similar);

    cairo_set_source_rgb (cr, 1, 0, 0);
    cairo_paint (cr);

    similar = cairo_surface_reference (cairo_get_target (cr));
    cairo_destroy (cr);

    return similar;
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_surface_t *source;

    cairo_set_source_rgb (cr, 0, 0, 1);
    cairo_paint (cr);

    cairo_rectangle (cr, 0, 0, SIZE/2, SIZE);
    cairo_clip (cr);

    /* Draw a source rectangle outside the image, the effect should be to
     * clear only within the clip region.
     */
    source = create_source (cairo_get_target (cr));
    cairo_set_source_surface (cr, source, SIZE/2, 0);
    cairo_surface_destroy (source);

    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (clip_unbounded,
	    "Test handling of an unbounded fill outside the clip region",
	    "clip", /* keywords */
	    NULL, /* requirements */
	    SIZE, SIZE,
	    NULL, draw)

