/*
 * Copyright 2010 Igor Nikitin
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
 * Author: Igor Nikitin <igor_nikitin@valentina-db.com>
 */

#include "cairo-test.h"

#define HEIGHT 15
#define WIDTH 40

static void background (cairo_t *cr)
{
     cairo_set_source_rgb( cr, 0, 0, 0 );
     cairo_paint (cr);
}

static void text (cairo_t *cr)
{
     cairo_move_to (cr, 0, 12);
     cairo_set_source_rgb (cr, 1, 1, 1);
     cairo_show_text (cr, "CAIRO");
}

static cairo_test_status_t
top (cairo_t *cr, int width, int height)
{
     background (cr);

     cairo_rectangle (cr, 0, 0, WIDTH, 5);
     cairo_clip (cr);

     text (cr);

     return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
bottom (cairo_t *cr, int width, int height)
{
     background (cr);

     cairo_rectangle (cr, 0, HEIGHT-5, WIDTH, 5);
     cairo_clip (cr);

     text (cr);

     return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
left (cairo_t *cr, int width, int height)
{
     background (cr);

     cairo_rectangle (cr, 0, 0, 10, HEIGHT);
     cairo_clip (cr);

     text (cr);

     return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
right (cairo_t *cr, int width, int height)
{
     background (cr);

     cairo_rectangle (cr, WIDTH-10, 0, 10, HEIGHT);
     cairo_clip (cr);

     text (cr);

     return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (partial_clip_text_top,
	    "Tests drawing text through a single, partial clip.",
	    "clip, text", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, top)
CAIRO_TEST (partial_clip_text_bottom,
	    "Tests drawing text through a single, partial clip.",
	    "clip, text", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, bottom)
CAIRO_TEST (partial_clip_text_left,
	    "Tests drawing text through a single, partial clip.",
	    "clip, text", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, left)
CAIRO_TEST (partial_clip_text_right,
	    "Tests drawing text through a single, partial clip.",
	    "clip, text", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, right)
