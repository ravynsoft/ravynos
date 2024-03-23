/*
 * Copyright © 2011 Intel Corporation
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

#define WIDTH	200
#define HEIGHT	200

/* This is an example from the wild, as silly as it is
n 52 0 429 709 rectangle
clip+
//ANTIALIAS_NONE set-antialias
n 8 0 m 952 0 l 956.417969 0 960 3.582031 960 8 c 960 1807 l 960 1811.417969 956.417969 1815 952 1815 c 8 1815 l 3.582031 1815 0 1811.417969 0 1807 c 0 8 l 0 3.582031 3.582031 0 8 0 c h
clip+
//EVEN_ODD set-fill-rule
n 0 0 m 480 0 l 480 708 l 0 708 l h 8 1 m 952 1 l 955.867188 1 959 4.132812 959 8 c 959 1807 l 959 1810.867188 955.867188 1814 952 1814 c 8 1814 l 4.132812 1814 1 1810.867188 1 1807 c 1 8 l 1 4.132812 4.132812 1 8 1 c h
clip+
//WINDING set-fill-rule
//ANTIALIAS_DEFAULT set-antialias
n 960 0 m 52.5 907.5 l 52.5 1815 l 960 1815 l h
clip+
//ANTIALIAS_NONE set-antialias
n 960 0 m 52.5 0 l 52.5 907.5 l 960 1815 l h
clip+
*/

static void background (cairo_t *cr)
{
    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgb (cr, 1,1,1);
    cairo_paint (cr);
    cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
}

static void clip_0 (cairo_t *cr)
{
    cairo_rectangle (cr, 5, 5, 190, 190);
    cairo_clip (cr);
}

static void clip_1 (cairo_t *cr)
{
    cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);

    cairo_arc (cr, 100, 100, 125, 0, 2*M_PI);
    cairo_clip (cr);

    cairo_set_antialias (cr, CAIRO_ANTIALIAS_DEFAULT);
}

static void
rounded_rectangle (cairo_t *cr, int x, int y, int w, int h, int r)
{
    cairo_new_sub_path (cr);
    cairo_arc (cr, x + r, y + r, r, M_PI, 3 * M_PI / 2);
    cairo_arc (cr, x + w - r, y + r, r, 3 *M_PI / 2, 2 * M_PI);
    cairo_arc (cr, x + w - r, y + h - r, r, 0, M_PI / 2);
    cairo_arc (cr, x + r, y + h - r, r, M_PI / 2, M_PI);
    cairo_close_path (cr);
}

static void clip_2 (cairo_t *cr)
{
    cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);

    rounded_rectangle (cr, 50, 50, 100, 100, 15);
    rounded_rectangle (cr, 60, 60, 80, 80, 5);
    cairo_clip (cr);

    cairo_set_fill_rule (cr, CAIRO_FILL_RULE_WINDING);
}

static void clip_3 (cairo_t *cr)
{
    cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);

    cairo_rectangle (cr, 40.25, 60.25, 120, 80);
    cairo_rectangle (cr, 60.25, 40.25, 80, 120);
    cairo_clip (cr);

    cairo_set_antialias (cr, CAIRO_ANTIALIAS_DEFAULT);
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    background (cr);

    clip_0 (cr);
    clip_1 (cr);
    clip_2 (cr);
    clip_3 (cr);

    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_paint (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (clip_mixed_antialias,
	    "Test drawing through through an mixture of clips",
	    "clip", /* keywords */
	    "target=raster", /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw)
