/*
 * Copyright 2009 Intel Corporation
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
    cairo_surface_t *similar;
    cairo_surface_t *region;
    cairo_t *cr_region;

    cairo_set_source_rgb (cr, .5, .5, .5);
    cairo_paint (cr);

    similar = cairo_surface_create_similar (cairo_get_target (cr),
					    CAIRO_CONTENT_COLOR,
					    60, 60);
    cr_region = cairo_create (similar);
    cairo_surface_destroy (similar);

    cairo_set_source_rgb (cr_region, .5, .5, .0);
    cairo_paint (cr_region);
    similar = cairo_surface_reference (cairo_get_target (cr_region));
    cairo_destroy (cr_region);

    /* fill the centre */
    region = cairo_surface_create_for_rectangle (similar, 20, 20, 20, 20);
    cairo_surface_destroy (similar);

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

    cairo_set_source_surface (cr, cairo_get_target (cr_region), 20, 20);
    cairo_destroy (cr_region);

    cairo_pattern_set_extend (cairo_get_source (cr), CAIRO_EXTEND_REPEAT);
    cairo_paint (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (subsurface_similar_repeat,
	    "Tests source clipping through an intermediate with repeat",
	    "subsurface, repeat", /* keywords */
	    "target=raster", /* FIXME! recursion bug in subsurface/snapshot (with pdf backend) */ /* requirements */
	    60, 60,
	    NULL, draw)
