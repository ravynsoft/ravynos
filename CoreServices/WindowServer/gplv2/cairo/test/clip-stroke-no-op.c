/*
 * Copyright 2009 Benjamin Otte
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
 * Author: Benjamin Otte <otte@gnome.org>
 */

#include "cairo-test.h"

#define WIDTH 50
#define HEIGHT 50

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    /* Neutral gray background */
    cairo_set_source_rgb (cr, 0.5, 0.5, 0.5);
    cairo_paint (cr);

    /* remove this clip operation and everything works */
    cairo_rectangle (cr, 10, 10, 30, 30);
    cairo_clip (cr);

    /* remove this no-op and everything works */
    cairo_stroke (cr);

    /* make the y coordinates integers and everything works */
    cairo_move_to (cr, 20, 20.101562);
    cairo_line_to (cr, 30, 20.101562);

    /* This clip operation should fail to work. But with cairo 1.9, if all the 
     * 3 cases above happen, the clip will not work and the paint will happen.
     */
    cairo_save (cr); {
	cairo_set_source_rgba (cr, 1, 0.5, 0.5, 1);
	cairo_clip_preserve (cr);
	cairo_paint (cr);
    } cairo_restore (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (clip_stroke_no_op,
	    "Exercises a bug found by Benjamin Otte whereby a no-op clip is nullified by a stroke",
	    "clip, stroke", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw)
