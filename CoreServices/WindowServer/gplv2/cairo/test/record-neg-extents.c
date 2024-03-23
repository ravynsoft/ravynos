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

#define PAT_SIZE  64
#define PAD (PAT_SIZE/8)
#define WIDTH (PAT_SIZE*4 + PAD*5)
#define HEIGHT (PAT_SIZE + PAD*2)

/* Test case based on bug 89232 - painting a recording surface to a pdf/ps surface
 * omits objects on the recording surface with negative coordinates even though
 * the pattern matrix has transformed the objects to within the page extents.
 * The bug is a result of pdf/ps assuming the surface extents are always
 * (0,0) to (page_width, page_height).
 *
 * Each test has four cases of painting a recording pattern where:
 * 1) recording surface origin is transformed to the center of the pattern
 * 2) same as 1) but also scaled up 10x
 * 3) same as 1) but also scaled down 10x
 * 4) same as 1) but also rotated 45 deg
 */


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
create_pattern (cairo_matrix_t *mat, cairo_bool_t bounded)
{
    cairo_surface_t *surf;
    cairo_pattern_t *pat;
    cairo_t *cr;
    int border;
    int square;

    if (bounded) {
	cairo_rectangle_t extents = { 0, 0, PAT_SIZE, PAT_SIZE };
	transform_extents (&extents, mat);
	surf = cairo_recording_surface_create (CAIRO_CONTENT_COLOR_ALPHA, &extents);
    } else {
	surf = cairo_recording_surface_create (CAIRO_CONTENT_COLOR_ALPHA, NULL);
    }

    cr = cairo_create (surf);
    cairo_transform (cr, mat);

    border  = PAT_SIZE/8;
    square = (PAT_SIZE - 2*border)/2;

    cairo_rectangle (cr, 0, 0, PAT_SIZE, PAT_SIZE);
    cairo_clip (cr);
    cairo_set_source_rgb (cr, 1, 0, 0);
    cairo_paint (cr);

    cairo_translate (cr, border, border);
    cairo_rectangle (cr, 0, 0, square, square);
    cairo_set_source_rgb (cr, 0, 1, 0);
    cairo_fill (cr);

    cairo_translate (cr, square, 0);
    cairo_rectangle (cr, 0, 0, square, square);
    cairo_set_source_rgb (cr, 0, 0, 1);
    cairo_fill (cr);

    cairo_translate (cr, 0, square);
    cairo_rectangle (cr, 0, 0, square, square);
    cairo_set_source_rgb (cr, 0, 1, 1);
    cairo_fill (cr);

    cairo_translate (cr, -square, 0);
    cairo_rectangle (cr, 0, 0, square, square);
    cairo_set_source_rgb (cr, 1, 1, 0);
    cairo_fill (cr);

    cairo_destroy (cr);

    pat = cairo_pattern_create_for_surface (surf);
    cairo_surface_destroy (surf);
    cairo_pattern_set_matrix (pat, mat);

    return pat;
}

static cairo_test_status_t
record_extents (cairo_t *cr, int width, int height, cairo_bool_t bounded)
{
    cairo_pattern_t *pat;
    cairo_matrix_t mat;

    /* record surface extents (-PAT_SIZE/2, -PAT_SIZE/2) to (PAT_SIZE/2, PAT_SIZE/2) */
    cairo_translate (cr, PAD, PAD);
    cairo_matrix_init_translate (&mat, -PAT_SIZE/2, -PAT_SIZE/2);
    pat = create_pattern (&mat, bounded);
    cairo_set_source (cr, pat);
    cairo_pattern_destroy (pat);
    cairo_paint (cr);

    /* record surface extents (-10*PAT_SIZE/2, -10*PAT_SIZE/2) to (10*PAT_SIZE/2, 10*PAT_SIZE/2) */
    cairo_translate (cr, PAT_SIZE + PAD, 0);
    cairo_matrix_init_translate (&mat, -10.0*PAT_SIZE/2, -10.0*PAT_SIZE/2);
    cairo_matrix_scale (&mat, 10, 10);
    pat = create_pattern (&mat, bounded);
    cairo_set_source (cr, pat);
    cairo_pattern_destroy (pat);
    cairo_paint (cr);

    /* record surface extents (-0.1*PAT_SIZE/2, -0.1*PAT_SIZE/2) to (0.1*PAT_SIZE/2, 0.1*PAT_SIZE/2) */
    cairo_translate (cr, PAT_SIZE + PAD, 0);
    cairo_matrix_init_translate (&mat, -0.1*PAT_SIZE/2, -0.1*PAT_SIZE/2);
    cairo_matrix_scale (&mat, 0.1, 0.1);
    pat = create_pattern (&mat, bounded);
    cairo_set_source (cr, pat);
    cairo_pattern_destroy (pat);
    cairo_paint (cr);

    /* record surface centered on (0,0) and rotated 45 deg */
    cairo_translate (cr, PAT_SIZE + PAD, 0);
    cairo_matrix_init_translate (&mat, -PAT_SIZE/sqrt(2), -PAT_SIZE/sqrt(2));
    cairo_matrix_rotate (&mat, M_PI/4.0);
    cairo_matrix_translate (&mat, PAT_SIZE/2, -PAT_SIZE/2);
    pat = create_pattern (&mat, bounded);
    cairo_set_source (cr, pat);
    cairo_pattern_destroy (pat);
    cairo_paint (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
record_neg_extents_bounded (cairo_t *cr, int width, int height)
{
    return record_extents(cr, width, height, TRUE);
}

static cairo_test_status_t
record_neg_extents_unbounded (cairo_t *cr, int width, int height)
{
    return record_extents(cr, width, height, FALSE);
}


CAIRO_TEST (record_neg_extents_unbounded,
	    "Paint unbounded recording pattern with untransformed extents outside of target extents",
	    "record,transform,pattern", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, record_neg_extents_unbounded)
CAIRO_TEST (record_neg_extents_bounded,
	    "Paint bounded recording pattern with untransformed extents outside of target extents",
	    "record,transform,pattern", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, record_neg_extents_bounded)
