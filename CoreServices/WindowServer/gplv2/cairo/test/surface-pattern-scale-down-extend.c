/*
 * Copyright © 2010 M Joonas Pihlaja
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Author: M Joonas Pihlaja <jpihlaja@cc.helsinki.fi>
 */
#include "cairo-test.h"

/* Test that we can simultaneously downscale and extend a surface
 * pattern.  Reported by Franz Schmid to the cairo mailing list as a
 * regression in 1.9.6:
 *
 * https://lists.cairographics.org/archives/cairo/2010-February/019492.html
 */

static cairo_test_status_t
draw_with_extend (cairo_t *cr, int w, int h, cairo_extend_t extend)
{
    cairo_pattern_t *pattern;
    cairo_set_source_rgb (cr, 1,1,1);
    cairo_paint (cr);

    cairo_save (cr);

    /* When the destination surface is created by cairo-test-suite to
     * test device-offset, it is bigger than w x h. This test expects
     * the group to have a size which is exactly w x h, so it must
     * clip to the this rectangle to guarantee that the group will
     * have the correct size.
     */
    cairo_rectangle (cr, 0, 0, w, h);
    cairo_clip (cr);

    cairo_push_group_with_content (cr, CAIRO_CONTENT_COLOR); {
        /* A two by two checkerboard with black, red and yellow
         * cells. */
        cairo_set_source_rgb (cr, 1,0,0);
        cairo_rectangle (cr, w/2, 0, w-w/2, h/2);
        cairo_fill (cr);
        cairo_set_source_rgb (cr, 1,1,0);
        cairo_rectangle (cr, 0, h/2, w/2, h-h/2);
        cairo_fill (cr);
    }
    pattern = cairo_pop_group (cr);
    cairo_pattern_set_extend(pattern, extend);

    cairo_restore (cr);

    cairo_scale (cr, 0.5, 0.5);
    cairo_set_source (cr, pattern);
    cairo_paint (cr);

    cairo_pattern_destroy (pattern);
    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
draw_repeat (cairo_t *cr, int w, int h)
{
    return draw_with_extend (cr, w, h, CAIRO_EXTEND_REPEAT);
}
static cairo_test_status_t
draw_none (cairo_t *cr, int w, int h)
{
    return draw_with_extend (cr, w, h, CAIRO_EXTEND_NONE);
}
static cairo_test_status_t
draw_reflect (cairo_t *cr, int w, int h)
{
    return draw_with_extend (cr, w, h, CAIRO_EXTEND_REFLECT);
}
static cairo_test_status_t
draw_pad (cairo_t *cr, int w, int h)
{
    return draw_with_extend (cr, w, h, CAIRO_EXTEND_PAD);
}

CAIRO_TEST (surface_pattern_scale_down_extend_repeat,
	    "Test interaction of downscaling a surface pattern and extend-repeat",
            "pattern, transform, extend", /* keywords */
	    NULL, /* requirements */
            100, 100,
	    NULL, draw_repeat)
CAIRO_TEST (surface_pattern_scale_down_extend_none,
	    "Test interaction of downscaling a surface pattern and extend-none",
            "pattern, transform, extend", /* keywords */
	    NULL, /* requirements */
            100, 100,
	    NULL, draw_none)
CAIRO_TEST (surface_pattern_scale_down_extend_reflect,
	    "Test interaction of downscaling a surface pattern and extend-reflect",
            "pattern, transform, extend", /* keywords */
	    NULL, /* requirements */
            100, 100,
	    NULL, draw_reflect)
CAIRO_TEST (surface_pattern_scale_down_extend_pad,
	    "Test interaction of downscaling a surface pattern and extend-pad",
            "pattern, transform, extend", /* keywords */
	    NULL, /* requirements */
            100, 100,
	    NULL, draw_pad)
