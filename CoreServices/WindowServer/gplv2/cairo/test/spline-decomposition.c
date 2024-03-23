/*
 * Copyright 2008 Chris Wilson
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Chris Wilson not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. Chris Wilson makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * CHRIS WILSON DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL CHRIS WILSON BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Chris Wilson <chris@chris-wilson.co.uk>
 */

#include "cairo-test.h"

typedef struct _point {
    double x,y;
} point_t;

typedef struct _knots {
    point_t a,b,c,d;
} knots_t;

static knots_t knots[5] = {
    { {0, 0}, {0, 100}, {100, 100}, {100, 0} },
    { {0, 0}, {75, 100}, {25, 100}, {100, 0} },
    { {0, 0}, {100, 100}, {0, 100}, {100, 0} },
    { {0, 0}, {150, 100}, {-50, 100}, {100, 0} },
    { {0, 0}, {100, 200}, {0, -100}, {100, 100} },
};

#ifdef REFERENCE
static void
_lerp_half (const point_t *a, const point_t *b, point_t *result)
{
    result->x = .5 * (a->x + b->x);
    result->y = .5 * (a->y + b->y);
}

static void
_de_casteljau (knots_t *k1, knots_t *k2)
{
    point_t ab, bc, cd;
    point_t abbc, bccd;
    point_t final;

    _lerp_half (&k1->a, &k1->b, &ab);
    _lerp_half (&k1->b, &k1->c, &bc);
    _lerp_half (&k1->c, &k1->d, &cd);
    _lerp_half (&ab, &bc, &abbc);
    _lerp_half (&bc, &cd, &bccd);
    _lerp_half (&abbc, &bccd, &final);

    k2->a = final;
    k2->b = bccd;
    k2->c = cd;
    k2->d = k1->d;

    k1->b = ab;
    k1->c = abbc;
    k1->d = final;
}

static double
_spline_error_squared (const knots_t *knots)
{
    double bdx, bdy, berr;
    double cdx, cdy, cerr;
    double dx, dy, v;

    /* Intersection point (px):
     *	    px = p1 + u(p2 - p1)
     *	    (p - px) ∙ (p2 - p1) = 0
     * Thus:
     *	    u = ((p - p1) ∙ (p2 - p1)) / ∥p2 - p1∥²;
     */
    bdx = knots->b.x - knots->a.x;
    bdy = knots->b.y - knots->a.y;

    cdx = knots->c.x - knots->a.x;
    cdy = knots->c.y - knots->a.y;

    dx = knots->d.x - knots->a.x;
    dy = knots->d.y - knots->a.y;
    v = dx * dx + dy * dy;
    if (v != 0.) {
	double u;

	u = bdx * dx + bdy * dy;
	if (u <= 0) {
	    /* bdx -= 0;
	     * bdy -= 0;
	     */
	} else if (u >= v) {
	    bdx -= dx;
	    bdy -= dy;
	} else {
	    bdx -= u/v * dx;
	    bdy -= u/v * dy;
	}

	u = cdx * dx + cdy * dy;
	if (u <= 0) {
	    /* cdx -= 0;
	     * cdy -= 0;
	     */
	} else if (u >= v) {
	    cdx -= dx;
	    cdy -= dy;
	} else {
	    cdx -= u/v * dx;
	    cdy -= u/v * dy;
	}
    }

    berr = bdx * bdx + bdy * bdy;
    cerr = cdx * cdx + cdy * cdy;
    if (berr > cerr)
	return berr * v;
    else
	return cerr * v;
}

static void
_offset_line_to (cairo_t *cr,
		 const point_t *p0,
		 const point_t *p1,
		 const point_t *p2,
		 const point_t *p3,
		 double offset)
{
    double dx, dy, v;

    dx = p1->x - p0->x;
    dy = p1->y - p0->y;
     v = hypot (dx, dy);
     if (v == 0) {
	 dx = p2->x - p0->x;
	 dy = p2->y - p0->y;
	 v = hypot (dx, dy);
	 if (v == 0) {
	     dx = p3->x - p0->x;
	     dy = p3->y - p0->y;
	     v = hypot (dx, dy);
	 }
     }

     if (v == 0) {
	 cairo_line_to (cr, p0->x, p0->y);
     } else
	 cairo_line_to (cr, p0->x - offset * dy / v, p0->y + offset * dx / v);
}

static void
_spline_decompose_into (knots_t *k1,
			double tolerance_squared,
			double offset,
			cairo_t *cr)
{
    knots_t k2;

    if (_spline_error_squared (k1) < tolerance_squared) {
	_offset_line_to (cr, &k1->a, &k1->b, &k1->c, &k1->d, offset);
	return;
    }

    _de_casteljau (k1, &k2);

    _spline_decompose_into (k1, tolerance_squared, offset, cr);
    _spline_decompose_into (&k2, tolerance_squared, offset, cr);
}

static void
_spline_decompose (const knots_t *knots,
		   double tolerance, double offset,
		   cairo_t *cr)
{
    knots_t k;

    k = *knots;
    _spline_decompose_into (&k, tolerance * tolerance, offset, cr);

    _offset_line_to (cr, &knots->d, &knots->c, &knots->b, &knots->a, -offset);
}

static void
_knots_reverse (knots_t *knots)
{
    point_t tmp;

    tmp = knots->a;
    knots->a = knots->d;
    knots->d = tmp;

    tmp = knots->b;
    knots->b = knots->c;
    knots->c = tmp;
}

static void
thick_splines (cairo_t *cr, double offset)
{
    knots_t k;

    cairo_save (cr);
    cairo_translate (cr, 15, 15);

    k = knots[0];

    cairo_new_path (cr);
    _spline_decompose (&k, .1, offset, cr);
    _knots_reverse (&k);
    _spline_decompose (&k, .1, offset, cr);
    cairo_close_path (cr);
    cairo_fill (cr);

    cairo_translate (cr, 130, 0);

    k = knots[1];

    cairo_new_path (cr);
    _spline_decompose (&k, .1, offset, cr);
    _knots_reverse (&k);
    _spline_decompose (&k, .1, offset, cr);
    cairo_close_path (cr);
    cairo_fill (cr);

    cairo_translate (cr, 130, 0);

    k = knots[2];

    cairo_new_path (cr);
    _spline_decompose (&k, .1, offset, cr);
    _knots_reverse (&k);
    _spline_decompose (&k, .1, offset, cr);
    cairo_close_path (cr);
    cairo_fill (cr);

    cairo_translate (cr, -130 - 65, 130);

    k = knots[3];

    cairo_new_path (cr);
    _spline_decompose (&k, .1, offset, cr);
    _knots_reverse (&k);
    _spline_decompose (&k, .1, offset, cr);
    cairo_close_path (cr);
    cairo_fill (cr);

    cairo_translate (cr, 130, 0);

    k = knots[4];

    cairo_new_path (cr);
    _spline_decompose (&k, .1, offset, cr);
    _knots_reverse (&k);
    _spline_decompose (&k, .1, offset, cr);
    cairo_close_path (cr);
    cairo_fill (cr);
    cairo_restore (cr);
}

static void
thin_splines (cairo_t *cr)
{
    cairo_save (cr);
    cairo_translate (cr, 15, 15);

    cairo_new_path (cr);
    _spline_decompose (&knots[0], .1, 0, cr);
    cairo_stroke (cr);

    cairo_translate (cr, 130, 0);

    cairo_new_path (cr);
    _spline_decompose (&knots[1], .1, 0, cr);
    cairo_stroke (cr);

    cairo_translate (cr, 130, 0);

    cairo_new_path (cr);
    _spline_decompose (&knots[2], .1, 0, cr);
    cairo_stroke (cr);

    cairo_translate (cr, -130 - 65, 130);

    cairo_new_path (cr);
    _spline_decompose (&knots[3], .1, 0, cr);
    cairo_stroke (cr);

    cairo_translate (cr, 130, 0);

    cairo_new_path (cr);
    _spline_decompose (&knots[4], .1, 0, cr);
    cairo_stroke (cr);
    cairo_restore (cr);
}
#endif

static void
draw_bbox (cairo_t *cr, double x0, double y0, double x1, double y1)
{
    cairo_rectangle (cr,
		     floor (x0) + .5, floor (y0) + .5,
		     ceil (x1) - floor (x0), ceil (y1) - floor (y0));
    cairo_stroke (cr);
}

static void
stroke_splines (cairo_t *cr)
{
    double stroke_x0, stroke_x1, stroke_y0, stroke_y1;
    double path_x0, path_x1, path_y0, path_y1;

    cairo_save (cr);
    cairo_translate (cr, 15, 15);

    cairo_new_path (cr);
    cairo_move_to (cr,
		   knots[0].a.x, knots[0].a.y);
    cairo_curve_to (cr,
		    knots[0].b.x, knots[0].b.y,
		    knots[0].c.x, knots[0].c.y,
		    knots[0].d.x, knots[0].d.y);
    cairo_stroke_extents (cr, &stroke_x0, &stroke_y0, &stroke_x1, &stroke_y1);
    cairo_path_extents (cr, &path_x0, &path_y0, &path_x1, &path_y1);
    cairo_stroke (cr);

    cairo_save (cr); {
	cairo_set_line_width (cr, 1);
	cairo_set_source_rgb (cr, 1, 0, 0);
	draw_bbox (cr, stroke_x0, stroke_y0, stroke_x1, stroke_y1);
	cairo_set_source_rgb (cr, 0, 0, 1);
	draw_bbox (cr, path_x0, path_y0, path_x1, path_y1);
    } cairo_restore (cr);

    cairo_translate (cr, 130, 0);

    cairo_new_path (cr);
    cairo_move_to (cr,
		   knots[1].a.x, knots[1].a.y);
    cairo_curve_to (cr,
		    knots[1].b.x, knots[1].b.y,
		    knots[1].c.x, knots[1].c.y,
		    knots[1].d.x, knots[1].d.y);
    cairo_stroke_extents (cr, &stroke_x0, &stroke_y0, &stroke_x1, &stroke_y1);
    cairo_path_extents (cr, &path_x0, &path_y0, &path_x1, &path_y1);
    cairo_stroke (cr);

    cairo_save (cr); {
	cairo_set_line_width (cr, 1);
	cairo_set_source_rgb (cr, 1, 0, 0);
	draw_bbox (cr, stroke_x0, stroke_y0, stroke_x1, stroke_y1);
	cairo_set_source_rgb (cr, 0, 0, 1);
	draw_bbox (cr, path_x0, path_y0, path_x1, path_y1);
    } cairo_restore (cr);

    cairo_translate (cr, 130, 0);

    cairo_new_path (cr);
    cairo_move_to (cr,
		   knots[2].a.x, knots[2].a.y);
    cairo_curve_to (cr,
		    knots[2].b.x, knots[2].b.y,
		    knots[2].c.x, knots[2].c.y,
		    knots[2].d.x, knots[2].d.y);
    cairo_stroke_extents (cr, &stroke_x0, &stroke_y0, &stroke_x1, &stroke_y1);
    cairo_path_extents (cr, &path_x0, &path_y0, &path_x1, &path_y1);
    cairo_stroke (cr);

    cairo_save (cr); {
	cairo_set_line_width (cr, 1);
	cairo_set_source_rgb (cr, 1, 0, 0);
	draw_bbox (cr, stroke_x0, stroke_y0, stroke_x1, stroke_y1);
	cairo_set_source_rgb (cr, 0, 0, 1);
	draw_bbox (cr, path_x0, path_y0, path_x1, path_y1);
    } cairo_restore (cr);

    cairo_translate (cr, -130 - 65, 130);

    cairo_new_path (cr);
    cairo_move_to (cr,
		   knots[3].a.x, knots[3].a.y);
    cairo_curve_to (cr,
		    knots[3].b.x, knots[3].b.y,
		    knots[3].c.x, knots[3].c.y,
		    knots[3].d.x, knots[3].d.y);
    cairo_stroke_extents (cr, &stroke_x0, &stroke_y0, &stroke_x1, &stroke_y1);
    cairo_path_extents (cr, &path_x0, &path_y0, &path_x1, &path_y1);
    cairo_stroke (cr);

    cairo_save (cr); {
	cairo_set_line_width (cr, 1);
	cairo_set_source_rgb (cr, 1, 0, 0);
	draw_bbox (cr, stroke_x0, stroke_y0, stroke_x1, stroke_y1);
	cairo_set_source_rgb (cr, 0, 0, 1);
	draw_bbox (cr, path_x0, path_y0, path_x1, path_y1);
    } cairo_restore (cr);

    cairo_translate (cr, 130, 0);

    cairo_new_path (cr);
    cairo_move_to (cr,
		   knots[4].a.x, knots[4].a.y);
    cairo_curve_to (cr,
		    knots[4].b.x, knots[4].b.y,
		    knots[4].c.x, knots[4].c.y,
		    knots[4].d.x, knots[4].d.y);
    cairo_stroke_extents (cr, &stroke_x0, &stroke_y0, &stroke_x1, &stroke_y1);
    cairo_path_extents (cr, &path_x0, &path_y0, &path_x1, &path_y1);
    cairo_stroke (cr);

    cairo_save (cr); {
	cairo_set_line_width (cr, 1);
	cairo_set_source_rgb (cr, 1, 0, 0);
	draw_bbox (cr, stroke_x0, stroke_y0, stroke_x1, stroke_y1);
	cairo_set_source_rgb (cr, 0, 0, 1);
	draw_bbox (cr, path_x0, path_y0, path_x1, path_y1);
    } cairo_restore (cr);

    cairo_restore (cr);
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

#ifdef REFERENCE
    cairo_set_source_rgb (cr, 0, 0, 0);
    thick_splines (cr, 5);

    cairo_set_source_rgb (cr, 1, 1, 1);
    thin_splines (cr);
#endif

    /*
     * Use a high tolerance to reduce dependence upon algorithm used for
     * spline decomposition.
     */
    cairo_set_tolerance (cr, 0.001);

    cairo_set_line_width (cr, 10);
    cairo_set_source_rgb (cr, 0, 0, 0);
    stroke_splines (cr);
    cairo_set_line_width (cr, 2);
    cairo_set_source_rgb (cr, 1, 1, 1);
    stroke_splines (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (spline_decomposition,
	    "Tests splines with various inflection points",
	    "stroke, spline", /* keywords */
	    NULL, /* requirements */
	    390, 260,
	    NULL, draw)
