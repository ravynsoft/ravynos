/*
 * Copyright © 2020 Ben Pfaff & Uli Schlachter
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
 * Authors:
 *   Ben Pfaff <blp@cs.stanford.edu>
 *   Uli Schlachter <psychon@znc.in>
 */

#include "cairo-test.h"

static cairo_surface_t*
draw_recording ()
{
    cairo_surface_t *recording;
    cairo_rectangle_t extents;
    cairo_t *cr;

    extents.x = 0;
    extents.y = 0;
    extents.width = 10;
    extents.height = 10;

    recording = cairo_recording_surface_create (CAIRO_CONTENT_COLOR_ALPHA, &extents);

    cr = cairo_create(recording);
    cairo_tag_begin (cr, CAIRO_TAG_DEST, "name='dest'");
    cairo_rectangle (cr, 3, 3, 4, 4);
    cairo_stroke (cr);
    cairo_tag_end (cr, CAIRO_TAG_DEST);
    cairo_destroy(cr);

    return recording;
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_surface_t *recording;

    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    recording = draw_recording ();
    cairo_set_source_surface (cr, recording, 0, 0);
    cairo_paint (cr);
    cairo_surface_destroy (recording);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (bug_448,
	    "Exercises a bug with the tag API",
	    "pdf", /* keywords */
	    NULL, /* requirements */
	    10, 10,
	    NULL, draw)
