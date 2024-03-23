/*
 * Copyright 2009 Benjamin Otte
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

typedef enum {
  CLEAR,
  CLEARED,
  PAINTED
} surface_type_t;

#define SIZE 10
#define SPACE 5

static cairo_surface_t *
create_surface (cairo_t *target, cairo_content_t content, surface_type_t type)
{
    cairo_surface_t *surface;
    cairo_t *cr;

    surface = cairo_surface_create_similar (cairo_get_target (target),
					    content,
					    SIZE, SIZE);

    if (type == CLEAR)
	return surface;

    cr = cairo_create (surface);
    cairo_surface_destroy (surface);

    cairo_set_source_rgb (cr, 0.75, 0, 0);
    cairo_paint (cr);

    if (type == PAINTED)
	goto DONE;

    cairo_set_operator (cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint (cr);

DONE:
    surface = cairo_surface_reference (cairo_get_target (cr));
    cairo_destroy (cr);

    return surface;
}

static void
paint (cairo_t *cr, cairo_surface_t *surface)
{
    cairo_set_source_surface (cr, surface, 0, 0);
    cairo_paint (cr);
}

static void
fill (cairo_t *cr, cairo_surface_t *surface)
{
    cairo_set_source_surface (cr, surface, 0, 0);
    cairo_rectangle (cr, -SPACE, -SPACE, SIZE + 2 * SPACE, SIZE + 2 * SPACE);
    cairo_fill (cr);
}

static void
stroke (cairo_t *cr, cairo_surface_t *surface)
{
    cairo_set_source_surface (cr, surface, 0, 0);
    cairo_set_line_width (cr, 2.0);
    cairo_rectangle (cr, 1, 1, SIZE - 2, SIZE - 2);
    cairo_stroke (cr);
}

static void
mask (cairo_t *cr, cairo_surface_t *surface)
{
    cairo_set_source_rgb (cr, 0, 0, 0.75);
    cairo_mask_surface (cr, surface, 0, 0);
}

static void
mask_self (cairo_t *cr, cairo_surface_t *surface)
{
    cairo_set_source_surface (cr, surface, 0, 0);
    cairo_mask_surface (cr, surface, 0, 0);
}

static void
glyphs (cairo_t *cr, cairo_surface_t *surface)
{
    cairo_set_source_surface (cr, surface, 0, 0);
    cairo_select_font_face (cr,
			    "@cairo:",
			    CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size (cr, 16);
    cairo_translate (cr, 0, SIZE);
    cairo_show_text (cr, "C");
}

typedef void (* operation_t) (cairo_t *cr, cairo_surface_t *surface);
static operation_t operations[] = {
    paint,
    fill,
    stroke,
    mask,
    mask_self,
    glyphs
};

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_content_t contents[] = { CAIRO_CONTENT_COLOR_ALPHA, CAIRO_CONTENT_COLOR, CAIRO_CONTENT_ALPHA };
    unsigned int content, type, ops;

    cairo_set_source_rgb (cr, 0.5, 0.5, 0.5);
    cairo_paint (cr);
    cairo_translate (cr, SPACE, SPACE);

    for (type = 0; type <= PAINTED; type++) {
	for (content = 0; content < ARRAY_LENGTH (contents); content++) {
	    cairo_surface_t *surface;

	    surface = create_surface (cr, contents[content], type);

            cairo_save (cr);
            for (ops = 0; ops < ARRAY_LENGTH (operations); ops++) {
                cairo_save (cr);
                operations[ops] (cr, surface);
                cairo_restore (cr);
                cairo_translate (cr, 0, SIZE + SPACE);
            }
            cairo_restore (cr);
            cairo_translate (cr, SIZE + SPACE, 0);

	    cairo_surface_destroy (surface);
        }
    }

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (clear_source,
	    "Check painting with cleared surfaces works as expected",
	    NULL, /* keywords */
	    NULL, /* requirements */
	    (SIZE + SPACE) * 9 + SPACE, ARRAY_LENGTH (operations) * (SIZE + SPACE) + SPACE,
	    NULL, draw)
