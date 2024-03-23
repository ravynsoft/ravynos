/*
 * Copyright © 2007 Red Hat, Inc.
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
 * Authors:
 *	Behdad Esfahbod <behdad@behdad.org>
 *	Chris Wilson <chris@chris-wilson.co.uk>
 */

#include "cairo-test.h"
#include <math.h>
#include <stdio.h>

#define SIZE 90

/* This is written using clip+paint to exercise a bug that once was in the
 * recording surface.
 */

static cairo_surface_t *
source (cairo_surface_t *surface)
{
    cairo_t *cr;

    /* Create a 4-pixel image surface with my favorite four colors in each
     * quadrant. */
    cr = cairo_create (surface);
    cairo_surface_destroy (surface);

    /* upper-left = white */
    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_rectangle (cr, 0, 0, 1, 1);
    cairo_fill (cr);

    /* upper-right = red */
    cairo_set_source_rgb (cr, 1, 0, 0);
    cairo_rectangle (cr, 1, 0, 1, 1);
    cairo_fill (cr);

    /* lower-left = green */
    cairo_set_source_rgb (cr, 0, 1, 0);
    cairo_rectangle (cr, 0, 1, 1, 1);
    cairo_fill (cr);

    /* lower-right = blue */
    cairo_set_source_rgb (cr, 0, 0, 1);
    cairo_rectangle (cr, 1, 1, 1, 1);
    cairo_fill (cr);

    surface = cairo_surface_reference (cairo_get_target (cr));
    cairo_destroy (cr);

    return surface;
}

static cairo_surface_t *
image (cairo_t *cr)
{
    return source (cairo_image_surface_create (CAIRO_FORMAT_RGB24, 2, 2));
}

static cairo_surface_t *
similar (cairo_t *cr)
{
    return source (cairo_surface_create_similar (cairo_get_target (cr),
						 CAIRO_CONTENT_COLOR, 2, 2));
}

static cairo_t *
extend (cairo_t *cr, cairo_surface_t *(*surface)(cairo_t *), cairo_extend_t mode)
{
    cairo_surface_t *s;

    cairo_set_source_rgb (cr, 0, 1, 1);
    cairo_paint (cr);

    /* Now use extend modes to cover most of the surface with those 4 colors */
    s = surface (cr);
    cairo_set_source_surface (cr, s, SIZE/2 - 1, SIZE/2 - 1);
    cairo_surface_destroy (s);

    cairo_pattern_set_extend (cairo_get_source (cr), mode);

    cairo_rectangle (cr, 10, 10, SIZE-20, SIZE-20);
    cairo_clip (cr);
    cairo_paint (cr);

    return cr;
}

static cairo_t *
extend_none (cairo_t *cr,
	     cairo_surface_t *(*pattern)(cairo_t *))
{
    return extend (cr, pattern, CAIRO_EXTEND_NONE);
}

static cairo_t *
extend_pad (cairo_t *cr,
	    cairo_surface_t *(*pattern)(cairo_t *))
{
    return extend (cr, pattern, CAIRO_EXTEND_PAD);
}

static cairo_t *
extend_repeat (cairo_t *cr,
	       cairo_surface_t *(*pattern)(cairo_t *))
{
    return extend (cr, pattern, CAIRO_EXTEND_REPEAT);
}

static cairo_t *
extend_reflect (cairo_t *cr,
	       cairo_surface_t *(*pattern)(cairo_t *))
{
    return extend (cr, pattern, CAIRO_EXTEND_REFLECT);
}

static cairo_t *
record_create (cairo_t *target)
{
    cairo_surface_t *surface;
    cairo_t *cr;

    surface = cairo_recording_surface_create (cairo_surface_get_content (cairo_get_target (target)), NULL);
    cr = cairo_create (surface);
    cairo_surface_destroy (surface);

    return cr;
}

static cairo_surface_t *
record_get (cairo_t *target)
{
    cairo_surface_t *surface;

    surface = cairo_surface_reference (cairo_get_target (target));
    cairo_destroy (target);

    return surface;
}

static cairo_test_status_t
record_replay (cairo_t *cr,
	       cairo_t *(*func)(cairo_t *,
				cairo_surface_t *(*pattern)(cairo_t *)),
	       cairo_surface_t *(*pattern)(cairo_t *),
	       int width, int height)
{
    cairo_surface_t *surface;
    int x, y;

    surface = record_get (func (record_create (cr), pattern));

    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_surface (cr, surface, 0, 0);
    cairo_surface_destroy (surface);
    cairo_pattern_set_extend (cairo_get_source (cr), CAIRO_EXTEND_NONE);

    for (y = 0; y < height; y += 2) {
	for (x = 0; x < width; x += 2) {
	    cairo_rectangle (cr, x, y, 2, 2);
	    cairo_clip (cr);
	    cairo_paint (cr);
	    cairo_reset_clip (cr);
	}
    }

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
record_extend_none (cairo_t *cr, int width, int height)
{
    return record_replay (cr, extend_none, image, width, height);
}

static cairo_test_status_t
record_extend_pad (cairo_t *cr, int width, int height)
{
    return record_replay (cr, extend_pad, image, width, height);
}

static cairo_test_status_t
record_extend_repeat (cairo_t *cr, int width, int height)
{
    return record_replay (cr, extend_repeat, image, width, height);
}

static cairo_test_status_t
record_extend_reflect (cairo_t *cr, int width, int height)
{
    return record_replay (cr, extend_reflect, image, width, height);
}

static cairo_test_status_t
record_extend_none_similar (cairo_t *cr, int width, int height)
{
    return record_replay (cr, extend_none, similar, width, height);
}

static cairo_test_status_t
record_extend_pad_similar (cairo_t *cr, int width, int height)
{
    return record_replay (cr, extend_pad, similar, width, height);
}

static cairo_test_status_t
record_extend_repeat_similar (cairo_t *cr, int width, int height)
{
    return record_replay (cr, extend_repeat, similar, width, height);
}

static cairo_test_status_t
record_extend_reflect_similar (cairo_t *cr, int width, int height)
{
    return record_replay (cr, extend_reflect, similar, width, height);
}

CAIRO_TEST (record_extend_none,
	    "Test CAIRO_EXTEND_NONE for recorded surface patterns",
	    "record, extend", /* keywords */
	    NULL, /* requirements */
	    SIZE, SIZE,
	    NULL, record_extend_none)
CAIRO_TEST (record_extend_pad,
	    "Test CAIRO_EXTEND_PAD for recorded surface patterns",
	    "record, extend", /* keywords */
	    NULL, /* requirements */
	    SIZE, SIZE,
	    NULL, record_extend_pad)
CAIRO_TEST (record_extend_repeat,
	    "Test CAIRO_EXTEND_REPEAT for recorded surface patterns",
	    "record, extend", /* keywords */
	    NULL, /* requirements */
	    SIZE, SIZE,
	    NULL, record_extend_repeat)
CAIRO_TEST (record_extend_reflect,
	    "Test CAIRO_EXTEND_REFLECT for recorded surface patterns",
	    "record, extend", /* keywords */
	    NULL, /* requirements */
	    SIZE, SIZE,
	    NULL, record_extend_reflect)

CAIRO_TEST (record_extend_none_similar,
	    "Test CAIRO_EXTEND_NONE for recorded surface patterns",
	    "record, extend", /* keywords */
	    NULL, /* requirements */
	    SIZE, SIZE,
	    NULL, record_extend_none_similar)
CAIRO_TEST (record_extend_pad_similar,
	    "Test CAIRO_EXTEND_PAD for recorded surface patterns",
	    "record, extend", /* keywords */
	    NULL, /* requirements */
	    SIZE, SIZE,
	    NULL, record_extend_pad_similar)
CAIRO_TEST (record_extend_repeat_similar,
	    "Test CAIRO_EXTEND_REPEAT for recorded surface patterns",
	    "record, extend", /* keywords */
	    NULL, /* requirements */
	    SIZE, SIZE,
	    NULL, record_extend_repeat_similar)
CAIRO_TEST (record_extend_reflect_similar,
	    "Test CAIRO_EXTEND_REFLECT for recorded surface patterns",
	    "record, extend", /* keywords */
	    NULL, /* requirements */
	    SIZE, SIZE,
	    NULL, record_extend_reflect_similar)
