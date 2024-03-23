/*
 * Copyright © 2008 Adrian Johnson
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
 * Author: Adrian Johnson <ajohnson@redneon.com>
 */

#include "cairo-test.h"

#define CIRCLE_SIZE 10
#define PAD 2
#define WIDTH (CIRCLE_SIZE*6.5 + PAD)
#define HEIGHT (CIRCLE_SIZE*7.0 + PAD)

static void
draw_circle (cairo_t *cr, double x, double y)
{
    cairo_save (cr);
    cairo_translate (cr, x, y);
    cairo_arc (cr, 0, 0, CIRCLE_SIZE / 2, 0., 2. * M_PI);
    cairo_fill (cr);
    cairo_restore (cr);
}

static void
draw_image_circle (cairo_t *cr, cairo_surface_t *source, double x, double y)
{
    cairo_save (cr);

    cairo_set_source_surface (cr, source, x, y);
    cairo_pattern_set_extend (cairo_get_source (cr), CAIRO_EXTEND_REFLECT);
    cairo_rectangle (cr, x, y, CIRCLE_SIZE, CIRCLE_SIZE);
    cairo_fill (cr);

    cairo_restore (cr);
}

static void
draw_circles (cairo_t *cr)
{
    draw_circle (cr, 0,               -CIRCLE_SIZE*0.1);
    draw_circle (cr, CIRCLE_SIZE*0.4,  CIRCLE_SIZE*0.25);

    draw_circle (cr, CIRCLE_SIZE*2, 0);
    draw_circle (cr, CIRCLE_SIZE*4, 0);
    draw_circle (cr, CIRCLE_SIZE*6, 0);
}

static void
draw_image_circles (cairo_t *cr, cairo_surface_t *source)
{
    draw_image_circle (cr, source, 0,               -CIRCLE_SIZE*0.1);
    draw_image_circle (cr, source, CIRCLE_SIZE*0.4,  CIRCLE_SIZE*0.25);

    draw_image_circle (cr, source, CIRCLE_SIZE*2, 0);
    draw_image_circle (cr, source, CIRCLE_SIZE*4, 0);
    draw_image_circle (cr, source, CIRCLE_SIZE*6, 0);
}

/* For each of circle and fallback_circle we draw:
 *  - two overlapping
 *  - one isolated
 *  - one off the page
 *  - one overlapping the edge of the page.
 *
 * We also draw a circle and fallback_circle overlapping each other.
 *
 * Circles are drawn in green. An opaque color and CAIRO_OPERATOR_OVER
 * is used to ensure they will be emitted as a vectors in PS/PDF.
 *
 * Fallback circles are drawn in red. CAIRO_OPERATOR_ADD is used to
 * ensure they will be emitted as a fallback image in PS/PDF.
 *
 * In order to trigger a fallback for SVG, we need to use a surface with
 * REFLECT.
 */
static cairo_surface_t *
surface_create (cairo_t *target)
{
    cairo_surface_t *surface;
    cairo_t *cr;

    surface = cairo_surface_create_similar (cairo_get_target (target),
					    CAIRO_CONTENT_COLOR_ALPHA,
					    CIRCLE_SIZE, CIRCLE_SIZE);
    cr = cairo_create (surface);
    cairo_surface_destroy (surface);

    cairo_set_source_rgb (cr, 1.0, 0.0, 0.0);
    draw_circle (cr, CIRCLE_SIZE/2, CIRCLE_SIZE/2);

    surface = cairo_surface_reference (cairo_get_target (cr));
    cairo_destroy (cr);

    return surface;
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_surface_t *surface;

    cairo_translate (cr, PAD, PAD);

    cairo_save (cr);

    /* Draw overlapping circle and fallback circle */
    cairo_set_source_rgb (cr, 0.0, 1.0, 0.0);
    cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
    draw_circle (cr, CIRCLE_SIZE*0.5,  CIRCLE_SIZE*1.5);

    cairo_set_source_rgb (cr, 1.0, 0.0, 0.0);
    cairo_set_operator (cr, CAIRO_OPERATOR_ADD);
    draw_circle (cr, CIRCLE_SIZE*0.75, CIRCLE_SIZE*1.75);

    /* Draw circles */
    cairo_set_source_rgb (cr, 0.0, 1.0, 0.0);
    cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
    cairo_translate (cr, CIRCLE_SIZE*2.5, CIRCLE_SIZE*0.6);
    draw_circles (cr);

    /* Draw fallback circles */
    cairo_set_source_rgb (cr, 1.0, 0.0, 0.0);
    cairo_set_operator (cr, CAIRO_OPERATOR_ADD);
    cairo_translate (cr, 0, CIRCLE_SIZE*2);
    draw_circles (cr);

    cairo_restore (cr);
    cairo_translate (cr, 0, CIRCLE_SIZE * 3.5);

    /* Draw using fallback surface */
    surface = surface_create (cr);

    cairo_set_source_rgb (cr, 0.0, 1.0, 0.0);
    cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
    draw_circle (cr, CIRCLE_SIZE*0.5,  CIRCLE_SIZE*1.5);

    cairo_set_operator (cr, CAIRO_OPERATOR_ADD);
    draw_image_circle (cr, surface, CIRCLE_SIZE/4, CIRCLE_SIZE + CIRCLE_SIZE/4);

    /* Draw circles */
    cairo_set_source_rgb (cr, 0.0, 1.0, 0.0);
    cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
    cairo_translate (cr, CIRCLE_SIZE*2.5, CIRCLE_SIZE*0.6);
    draw_circles (cr);

    cairo_set_operator (cr, CAIRO_OPERATOR_ADD);
    cairo_translate (cr, -CIRCLE_SIZE/2, CIRCLE_SIZE*1.5);
    draw_image_circles (cr, surface);

    cairo_surface_destroy (surface);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (finer_grained_fallbacks,
	    "Test that multiple PS/PDF fallback images in various locations are correct",
	    "fallbacks", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw)
