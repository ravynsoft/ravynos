/*
 * Copyright (c) 2010 M Joonas Pihlaja
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

/* Tests specific clipping fast paths and their interaction with
 * groups:  It shouldn't matter if the clip is set before or after
 * pushing a group.
 *
 * There's some overlap with the following tests, but they test for
 * different things:
 *
 *  group-clip.c (tests preserving paths), clipped-group.c (tests
 *  clipping the same thing different ways), clip-push-group (tests
 *  for a specific bug).
 */

#define GENERATE_REF 0

/* For determining whether we establish the clip path before or after
 * pushing a group. */
enum {
    CLIP_OUTSIDE_GROUP,
    CLIP_INSIDE_GROUP
};

typedef void (*clipper_t)(cairo_t *cr, int w, int h);

static cairo_test_status_t
clip_and_paint (cairo_t *cr,
                int w, int h,
                clipper_t do_clip,
                int clip_where)
{
    cairo_save (cr); {
        if (GENERATE_REF) {
            do_clip (cr, w, h);
            cairo_paint (cr);
        } else {
            if (clip_where == CLIP_OUTSIDE_GROUP)
                do_clip (cr, w, h);
            cairo_push_group (cr); {
                if (clip_where == CLIP_INSIDE_GROUP)
                    do_clip (cr, w, h);
                cairo_paint (cr);
            }
            cairo_pop_group_to_source (cr);
            if (clip_where == CLIP_OUTSIDE_GROUP)
		cairo_reset_clip (cr);
            cairo_paint (cr);
        }
    }
    cairo_restore (cr);
    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
run_clip_test (cairo_t *cr, int w, int h, clipper_t do_clip)
{
    cairo_set_source_rgb (cr, 1,1,1);
    cairo_paint (cr);
    cairo_set_source_rgb (cr, 1,0,0);

    /* Left. */
    clip_and_paint (cr, w/2, h, do_clip, CLIP_OUTSIDE_GROUP);

    /* Right */
    cairo_translate(cr, w/2, 0);
    clip_and_paint (cr, w/2, h, do_clip, CLIP_INSIDE_GROUP);

    return CAIRO_TEST_SUCCESS;
}

static void
clip_aligned_rectangles (cairo_t *cr, int w, int h)
{
    int x1 = 0.2 * w;
    int y1 = 0.2 * h;
    int x2 = 0.8 * w;
    int y2 = 0.8 * h;

    cairo_rectangle (cr, x1, y1, w, h);
    cairo_clip (cr);

    cairo_rectangle (cr, x2, y2, -w, -h);
    cairo_clip (cr);
}

static void
clip_unaligned_rectangles (cairo_t *cr, int w, int h)
{
    /* This clip stresses the antialiased edges produced by an
     * unaligned rectangular clip. The edges should be produced by
     * compositing red on white with alpha = 0.5 on the sides, and with
     * alpha = 0.25 in the corners. */
    int x1 = 0.2 * w;
    int y1 = 0.2 * h;
    int x2 = 0.8 * w;
    int y2 = 0.8 * h;

    cairo_rectangle (cr, x1+0.5, y1+0.5, w, h);
    cairo_clip (cr);

    cairo_rectangle (cr, x2+0.5, y2+0.5, -w, -h);
    w = x2 - x1;
    h = y2 - y1;
    cairo_rectangle (cr, x2, y1+1, -w+1, h-1);
    cairo_clip (cr);
}

static void
clip_circles (cairo_t *cr, int w, int h)
{
    int x1 = 0.5 * w;
    int y1 = 0.5 * h;
    int x2 = 0.75 * w;
    int y2 = 0.75 * h;
    int r = 0.4*MIN(w,h);

    cairo_arc (cr, x1, y1, r, 0, 6.28);
    cairo_close_path (cr);
    cairo_clip (cr);

    cairo_arc (cr, x2, y2, r, 0, 6.28);
    cairo_close_path (cr);
    cairo_clip (cr);
}

static cairo_test_status_t
draw_aligned_rectangles (cairo_t *cr, int width, int height)
{
    return run_clip_test (cr, width, height, clip_aligned_rectangles);
}

static cairo_test_status_t
draw_unaligned_rectangles (cairo_t *cr, int width, int height)
{
    return run_clip_test (cr, width, height, clip_unaligned_rectangles);
}

static cairo_test_status_t
draw_circles (cairo_t *cr, int width, int height)
{
    return run_clip_test (cr, width, height, clip_circles);
}

CAIRO_TEST (clip_group_shapes_aligned_rectangles,
	    "Test clip and group interaction with aligned rectangle clips",
	    "clip", /* keywords */
	    NULL, /* requirements */
	    200, 100,
	    NULL, draw_aligned_rectangles)

CAIRO_TEST (clip_group_shapes_unaligned_rectangles,
	    "Test clip and group interaction with unaligned rectangle clips",
	    "clip", /* keywords */
	    "target=raster", /* requirements */
	    200, 100,
	    NULL, draw_unaligned_rectangles)

CAIRO_TEST (clip_group_shapes_circles,
	    "Test clip and group interaction with circular clips",
	    "clip", /* keywords */
	    NULL, /* requirements */
	    200, 100,
	    NULL, draw_circles)
