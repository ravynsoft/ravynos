/*
 * Copyright © 2016 Adrian Johnson
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
 *	Adrian Johnson <ajohnson@redneon.com>
 */

#include "cairo-test.h"
#include <stdio.h>
#include <math.h>

#define PAT_SIZE  32
#define REPLAY_SIZE (PAT_SIZE*4)
#define PAD 10
#define WIDTH (REPLAY_SIZE*4 + PAD*5)
#define HEIGHT (REPLAY_SIZE + PAD*2)

/* Test replaying a recording surface pattern for each type of extend. */

static void
transform_extents(cairo_rectangle_t *extents, cairo_matrix_t *mat)
{
    double x1, y1, x2, y2, x, y;

#define UPDATE_BBOX \
    x1 = x < x1 ? x : x1; \
    y1 = y < y1 ? y : y1; \
    x2 = x > x2 ? x : x2; \
    y2 = y > y2 ? y : y2;

    x = extents->x;
    y = extents->y;
    cairo_matrix_transform_point (mat, &x, &y);
    x1 = x2 = x;
    y1 = y2 = y;

    x = extents->x + extents->width;
    y = extents->y;
    cairo_matrix_transform_point (mat, &x, &y);
    UPDATE_BBOX;

    x = extents->x;
    y = extents->y + extents->height;
    cairo_matrix_transform_point (mat, &x, &y);
    UPDATE_BBOX;

    x = extents->x + extents->width;
    y = extents->y + extents->height;
    cairo_matrix_transform_point (mat, &x, &y);
    UPDATE_BBOX;

    extents->x = x1;
    extents->y = y1;
    extents->width = x2 - extents->x;
    extents->height = y2 - extents->y;

#undef UPDATE_BBOX
}

static cairo_pattern_t *
create_pattern (cairo_matrix_t *mat, cairo_extend_t extend)
{
    cairo_surface_t *surf;
    cairo_pattern_t *pat;
    cairo_t *cr;
    cairo_rectangle_t extents = { 0, 0, PAT_SIZE, PAT_SIZE };

    transform_extents (&extents, mat);
    surf = cairo_recording_surface_create (CAIRO_CONTENT_COLOR_ALPHA, &extents);

    cr = cairo_create (surf);
    cairo_transform (cr, mat);

    cairo_rectangle (cr, 0, 0, PAT_SIZE/2, PAT_SIZE/2);
    cairo_set_source_rgb (cr, 1, 0, 0);
    cairo_fill (cr);

    cairo_translate (cr, PAT_SIZE/2, 0);
    cairo_rectangle (cr, 0, 0, PAT_SIZE/2, PAT_SIZE/2);
    cairo_set_source_rgb (cr, 0, 1, 0);
    cairo_fill (cr);

    cairo_translate (cr, 0, PAT_SIZE/2);
    cairo_rectangle (cr, 0, 0, PAT_SIZE/2, PAT_SIZE/2);
    cairo_set_source_rgb (cr, 0, 0, 1);
    cairo_fill (cr);

    cairo_translate (cr, -PAT_SIZE/2, 0);
    cairo_rectangle (cr, 0, 0, PAT_SIZE/2, PAT_SIZE/2);
    cairo_set_source_rgb (cr, 1, 1, 0);
    cairo_fill (cr);

    cairo_destroy (cr);

    pat = cairo_pattern_create_for_surface (surf);
    cairo_surface_destroy (surf);
    cairo_pattern_set_matrix (pat, mat);
    cairo_pattern_set_extend (pat, extend);
    cairo_pattern_set_filter (pat, CAIRO_FILTER_NEAREST);

    return pat;
}

static cairo_test_status_t
record_replay_extend (cairo_t *cr, int width, int height, cairo_extend_t extend)
{
    cairo_pattern_t *pat;
    cairo_matrix_t mat;

    /* record surface extents (-PAT_SIZE/2, -PAT_SIZE/2) to (PAT_SIZE/2, PAT_SIZE/2) */
    cairo_translate (cr, PAD, PAD);
    cairo_matrix_init_translate (&mat, -PAT_SIZE/2, -PAT_SIZE/2);
    pat = create_pattern (&mat, extend);

    /* test repeating patterns when the source is outside of the target clip */
    if (extend == CAIRO_EXTEND_REPEAT || extend == CAIRO_EXTEND_REFLECT) {
	cairo_matrix_init_translate (&mat, 3*PAT_SIZE/2, 3*PAT_SIZE/2);
	cairo_pattern_set_matrix (pat, &mat);
    }

    cairo_set_source (cr, pat);
    cairo_pattern_destroy (pat);
    cairo_rectangle (cr, 0, 0, REPLAY_SIZE, REPLAY_SIZE);
    cairo_fill (cr);

    /* record surface extents (-2*PAT_SIZE/2, -2*PAT_SIZE/2) to (2*PAT_SIZE/2, 2*PAT_SIZE/2) */
    cairo_translate (cr, REPLAY_SIZE + PAD, 0);
    cairo_matrix_init_translate (&mat, -2.0*PAT_SIZE/2, -2.0*PAT_SIZE/2);
    cairo_matrix_scale (&mat, 2, 2);
    pat = create_pattern (&mat, extend);
    cairo_set_source (cr, pat);
    cairo_pattern_destroy (pat);
    cairo_rectangle (cr, 0, 0, REPLAY_SIZE, REPLAY_SIZE);
    cairo_fill (cr);

    /* record surface extents (-0.5*PAT_SIZE/2, -0.5*PAT_SIZE/2) to (0.5*PAT_SIZE/2, 0.5*PAT_SIZE/2) */
    cairo_translate (cr, REPLAY_SIZE + PAD, 0);
    cairo_matrix_init_translate (&mat, -0.5*PAT_SIZE/2, -0.5*PAT_SIZE/2);
    cairo_matrix_scale (&mat, 0.5, 0.5);
    pat = create_pattern (&mat, extend);
    cairo_set_source (cr, pat);
    cairo_pattern_destroy (pat);
    cairo_rectangle (cr, 0, 0, REPLAY_SIZE, REPLAY_SIZE);
    cairo_fill (cr);

    /* record surface centered on (0,0) and rotated 45 deg */
    cairo_translate (cr, REPLAY_SIZE + PAD, 0);
    cairo_matrix_init_translate (&mat, -PAT_SIZE/sqrt(2), -PAT_SIZE/sqrt(2));
    cairo_matrix_rotate (&mat, M_PI/4.0);
    cairo_matrix_translate (&mat, PAT_SIZE/2, -PAT_SIZE/2);
    pat = create_pattern (&mat, extend);
    cairo_set_source (cr, pat);
    cairo_pattern_destroy (pat);
    cairo_rectangle (cr, 0, 0, REPLAY_SIZE, REPLAY_SIZE);
    cairo_fill (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
record_replay_extend_none (cairo_t *cr, int width, int height)
{
    return record_replay_extend (cr, width, height, CAIRO_EXTEND_NONE);
}

static cairo_test_status_t
record_replay_extend_repeat (cairo_t *cr, int width, int height)
{
    return record_replay_extend (cr, width, height, CAIRO_EXTEND_REPEAT);
}

static cairo_test_status_t
record_replay_extend_reflect (cairo_t *cr, int width, int height)
{
    return record_replay_extend (cr, width, height, CAIRO_EXTEND_REFLECT);
}

static cairo_test_status_t
record_replay_extend_pad (cairo_t *cr, int width, int height)
{
    return record_replay_extend (cr, width, height, CAIRO_EXTEND_PAD);
}

CAIRO_TEST (record_replay_extend_none,
	    "Paint recording pattern with CAIRO_EXTEND_NONE",
	    "record,pattern,extend", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, record_replay_extend_none)
CAIRO_TEST (record_replay_extend_repeat,
	    "Paint recording pattern with CAIRO_EXTEND_REPEAT",
	    "record,pattern,extend", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, record_replay_extend_repeat)
CAIRO_TEST (record_replay_extend_reflect,
	    "Paint recording pattern with CAIRO_EXTEND_REFLECT",
	    "record,pattern,extend", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, record_replay_extend_reflect)
CAIRO_TEST (record_replay_extend_pad,
	    "Paint recording pattern with CAIRO_EXTEND_PAD",
	    "record,pattern,extend", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, record_replay_extend_pad)
