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

#define RGBx 0xffff0000, 0xff00ff00, 0xff0000ff, 0x00000000

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    uint32_t data[] = {
	RGBx, RGBx, RGBx,
	RGBx, RGBx, RGBx,
	RGBx, RGBx, RGBx,
	RGBx, RGBx, RGBx,
    };
    cairo_surface_t *mask;

    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    mask = cairo_image_surface_create_for_data ((unsigned char *) data,
						CAIRO_FORMAT_ARGB32, 12, 4, 48);

    cairo_set_source_surface (cr, mask, 0, 0);
    cairo_pattern_set_filter (cairo_get_source (cr), CAIRO_FILTER_NEAREST);

    cairo_paint (cr);

    cairo_surface_finish (mask); /* data goes out of scope */
    cairo_surface_destroy (mask);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (bilevel_image,
	    "Test that PS can embed an RGB image with a bilevel alpha channel.",
	    "alpha, ps", /* keywords */
	    NULL, /* requirements */
	    12, 4,
	    NULL, draw)
