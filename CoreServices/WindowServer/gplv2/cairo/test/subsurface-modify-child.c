/*
 * Copyright 2010 Intel Corporation
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Intel not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. Intel makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * INTEL CORPORATION DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL INTEL CORPORATION BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Chris Wilson <chris@chris-wilson.co.uk>
 */

#include "cairo-test.h"

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_surface_t *region, *similar;
    cairo_t *cr_region, *cr_similar;

    cairo_set_source_rgb (cr, .5, .5, .5);
    cairo_paint (cr);

    similar = cairo_surface_create_similar (cairo_get_target (cr),
					    CAIRO_CONTENT_COLOR_ALPHA,
					    20, 20);

    /* copy the centre */
    cr_similar = cairo_create (similar);
    cairo_surface_destroy (similar);
    cairo_set_source_surface (cr_similar, cairo_get_target (cr), -20, -20);
    cairo_paint (cr_similar);
    similar = cairo_surface_reference (cairo_get_target (cr_similar));
    cairo_destroy (cr_similar);

    /* fill the centre */
    region = cairo_surface_create_for_rectangle (cairo_get_target (cr),
						 20, 20, 20, 20);
    cr_region = cairo_create (region);
    cairo_surface_destroy (region);

    cairo_set_source_rgb (cr_region, 1, 1, 1);
    cairo_rectangle (cr_region, 0, 0, 10, 10);
    cairo_fill (cr_region);

    cairo_set_source_rgb (cr_region, 1, 0, 0);
    cairo_rectangle (cr_region, 10, 0, 10, 10);
    cairo_fill (cr_region);

    cairo_set_source_rgb (cr_region, 0, 1, 0);
    cairo_rectangle (cr_region, 0, 10, 10, 10);
    cairo_fill (cr_region);

    cairo_set_source_rgb (cr_region, 0, 0, 1);
    cairo_rectangle (cr_region, 10, 10, 10, 10);
    cairo_fill (cr_region);

    cairo_destroy (cr_region);

    /* copy the centre, again */
    cr_similar = cairo_create (similar);
    cairo_surface_destroy (similar);
    cairo_set_source_surface (cr_similar, cairo_get_target (cr), -20, -20);
    cairo_paint (cr_similar);
    similar = cairo_surface_reference (cairo_get_target (cr_similar));
    cairo_destroy (cr_similar);

    /* repeat the pattern around the outside, but do not overwrite...*/
    cairo_set_source_surface (cr, similar, 20, 20);
    cairo_pattern_set_extend (cairo_get_source (cr), CAIRO_EXTEND_REPEAT);
    cairo_rectangle (cr, 0, 0, width, height);
    cairo_rectangle (cr, 20, 40, 20, -20);
    cairo_fill (cr);

    cairo_surface_destroy (similar);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (subsurface_modify_child,
	    "Tests source clipping with later modifications",
	    "subsurface", /* keywords */
	    "target=raster", /* FIXME! recursion bug in subsurface/snapshot (with pdf backend) */ /* requirements */
	    60, 60,
	    NULL, draw)
