/*
 * Copyright 2010 Red Hat Inc.
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
 * Author: Benjamin Otte <otte@gnome.org>
 */

#include "cairo-test.h"

#define TARGET_SIZE 10

#define SUB_SIZE 15
#define SUB_OFFSET -5

#define PAINT_OFFSET SUB_SIZE
#define PAINT_SIZE (3 * SUB_SIZE)

static cairo_content_t contents[] = { CAIRO_CONTENT_ALPHA,
                                      CAIRO_CONTENT_COLOR,
                                      CAIRO_CONTENT_COLOR_ALPHA };

#define N_CONTENTS ARRAY_LENGTH (contents)
#define N_PADS (CAIRO_EXTEND_PAD + 1)


static cairo_surface_t *
create_target (cairo_surface_t *similar_to,
               cairo_content_t content)
{
    cairo_surface_t *surface;
    cairo_t *cr;

    surface = cairo_surface_create_similar (similar_to,
                                            content,
                                            TARGET_SIZE, TARGET_SIZE);
    
    cr = cairo_create (surface);
    cairo_test_paint_checkered (cr);
    cairo_destroy (cr);

    return surface;
}

static cairo_test_status_t
check_surface_extents (const cairo_test_context_t *ctx,
                       cairo_surface_t *           surface,
                       double                      x,
                       double                      y,
                       double                      width,
                       double                      height)
{
    double x1, y1, x2, y2;
    cairo_t *cr;

    cr = cairo_create (surface);
    cairo_clip_extents (cr, &x1, &y1, &x2, &y2);
    cairo_destroy (cr);

    if (x != x1 ||
        y != y1 ||
        width != x2 - x1 ||
        height != y2 - y1) {
        cairo_test_log (ctx,
                        "surface extents should be (%g, %g, %g, %g), but are (%g, %g, %g, %g)\n",
                        x, y, width, height,
                        x1, y1, x2 - x1, y2 - y1);
        return CAIRO_TEST_FAILURE;
    }

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
draw_for_size (cairo_t *cr,
               double   x,
               double   y)
{
    cairo_surface_t *target, *subsurface;
    cairo_extend_t extend;
    cairo_test_status_t check, result = CAIRO_TEST_SUCCESS;
    unsigned int content;

    for (content = 0; content < N_CONTENTS; content++) {
        cairo_save (cr);

        /* create a target surface for our subsurface */
        target = create_target (cairo_get_target (cr),
                                contents[content]);

        /* create a subsurface that extends the target surface */
        subsurface = cairo_surface_create_for_rectangle (target, 
                                                         x, y,
                                                         SUB_SIZE, SUB_SIZE);

        /* ensure the extents are ok */
        check = check_surface_extents (cairo_test_get_context (cr),
                                       subsurface,
                                       0, 0,
                                       SUB_SIZE, SUB_SIZE);
        if (result == CAIRO_TEST_SUCCESS)
          result = check;

        /* paint this surface with all extend modes. */
        for (extend = 0; extend < N_PADS; extend++) {
            cairo_save (cr);

            cairo_rectangle (cr, 0, 0, PAINT_SIZE, PAINT_SIZE);
            cairo_clip (cr);

            cairo_set_source_surface (cr, subsurface, PAINT_OFFSET, PAINT_OFFSET);
            cairo_pattern_set_extend (cairo_get_source (cr), extend);
            cairo_paint (cr);

            cairo_restore (cr);

            cairo_translate (cr, PAINT_SIZE + TARGET_SIZE, 0);
        }

        cairo_surface_destroy (subsurface);
        cairo_surface_destroy (target);

        cairo_restore (cr);

        cairo_translate (cr, 0, PAINT_SIZE + TARGET_SIZE);
    }

    return result;
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_test_status_t check, result = CAIRO_TEST_SUCCESS;

    /* paint background in nice gray */
    cairo_set_source_rgb (cr, 0.51613, 0.55555, 0.51613);
    cairo_paint (cr);

    /* Use CAIRO_OPERATOR_SOURCE in the tests so we get the actual
     * contents of the subsurface */
    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);

    result = draw_for_size (cr, SUB_OFFSET, SUB_OFFSET);

    check = draw_for_size (cr, 0, 0);
    if (result == CAIRO_TEST_SUCCESS)
      result = check;

    return result;
}

CAIRO_TEST (subsurface_outside_target,
	    "Tests contents of subsurfaces outside target area",
	    "subsurface, pad", /* keywords */
	    "target=raster", /* FIXME! recursion bug in subsurface/snapshot (with pdf backend) */ /* requirements */
	    (PAINT_SIZE + TARGET_SIZE) * N_PADS         - TARGET_SIZE,
            (PAINT_SIZE + TARGET_SIZE) * N_CONTENTS * 2 - TARGET_SIZE,
	    NULL, draw)
