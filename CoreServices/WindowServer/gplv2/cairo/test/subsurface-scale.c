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
    cairo_surface_t *region[5];
    const char *text = "Cairo";
    int i;

    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    cairo_rectangle (cr, 0, 20, 200, 60);
    cairo_set_source_rgb (cr, 1, 0, 0);
    cairo_fill (cr);

    cairo_set_source_rgb (cr, 0, 0, 0);

    for (i = 0; i < 5; i++) {
	cairo_t *cr_region;
	cairo_text_extents_t extents;
	char buf[2] = { text[i], '\0' };

        region[i] = cairo_surface_create_for_rectangle (cairo_get_target (cr),
                                                        20 * i, 0, 20, 20);

	cr_region = cairo_create (region[i]);
	cairo_surface_destroy (region[i]);

	cairo_select_font_face (cr_region, "@cairo:",
				CAIRO_FONT_WEIGHT_NORMAL,
				CAIRO_FONT_SLANT_NORMAL);
	cairo_set_font_size (cr_region, 20);
	cairo_text_extents (cr_region, buf, &extents);
	cairo_move_to (cr_region,
		       10 - (extents.width/2 + extents.x_bearing),
		       10 - (extents.height/2 + extents.y_bearing));
	cairo_show_text (cr_region, buf);

	region[i] = cairo_surface_reference (cairo_get_target (cr_region));
	cairo_destroy (cr_region);
    }

    cairo_scale (cr, 2, 2);
    for (i = 0; i < 5; i++) {
	cairo_set_source_surface (cr, region[5-i-1], 20 * i, 20);
	cairo_pattern_set_extend (cairo_get_source (cr), CAIRO_EXTEND_PAD);
	cairo_rectangle (cr, 20*i, 20, 20, 20);
	cairo_fill (cr);
    }

    for (i = 0; i < 5; i++) {
	cairo_set_source_surface (cr, region[5-i-1], 20 * i, 40);
	cairo_paint_with_alpha (cr, .5);
    }

    for (i = 0; i < 5; i++)
	cairo_surface_destroy (region[i]);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (subsurface_scale,
	    "Tests clipping of both source and destination using subsurfaces",
	    "subsurface", /* keywords */
	    "target=raster", /* FIXME! recursion bug in subsurface/snapshot (with pdf backend) */ /* requirements */
	    200, 120,
	    NULL, draw)
