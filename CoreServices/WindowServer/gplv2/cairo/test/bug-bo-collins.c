/*
 * Copyright © 2012 Intel Corporation
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
 * Author: Chris Wilson <chris@chris-wilson.co.uk>
 */

#include "cairo-test.h"

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);
    cairo_set_source_rgb (cr, 0, 0, 0);

    cairo_translate (cr, 0, 0);
    cairo_save (cr);
    cairo_rectangle (cr, 10, 10, 20, 20);
    cairo_rectangle (cr, 20, 10, -10, 10);
    cairo_clip (cr);
    cairo_paint (cr);
    cairo_restore (cr);

    cairo_translate (cr, 40, 0);
    cairo_save (cr);
    cairo_rectangle (cr, 10, 10, 20, 20);
    cairo_rectangle (cr, 30, 10, -10, 10);
    cairo_clip (cr);
    cairo_paint (cr);
    cairo_restore (cr);

    cairo_translate (cr, 0, 40);
    cairo_save (cr);
    cairo_rectangle (cr, 10, 10, 20, 20);
    cairo_rectangle (cr, 30, 20, -10, 10);
    cairo_clip (cr);
    cairo_paint (cr);
    cairo_restore (cr);

    cairo_translate (cr, -40, 0);
    cairo_save (cr);
    cairo_rectangle (cr, 10, 10, 20, 20);
    cairo_rectangle (cr, 20, 20, -10, 10);
    cairo_clip (cr);
    cairo_paint (cr);
    cairo_restore (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (bug_bo_collins,
	    "Exercises a bug discovered by S. Christian Collins",
	    "clip, rectangular", /* keywords */
	    NULL, /* requirements */
	    80, 80,
	    NULL, draw)
