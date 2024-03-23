/*
 * Copyright © 2004 Red Hat, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Red Hat, Inc. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. Red Hat, Inc. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * RED HAT, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL RED HAT, INC. BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Carl D. Worth <cworth@cworth.org>
 */

#include "cairo-test.h"

#define LINES 5
#define LINE_LENGTH 10
#define IMAGE_WIDTH 2 * LINE_LENGTH + 6
#define IMAGE_HEIGHT ((LINES+4)*LINES)/2 + 2

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    int i;

    /* We draw in black, so paint white first. */
    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0); /* white */
    cairo_paint (cr);

    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_translate (cr, 2, 2);

    for (i=0; i < LINES; i++) {
	cairo_set_line_width (cr, i+1);
	cairo_move_to (cr, 0, 0);
	cairo_rel_line_to (cr, LINE_LENGTH, 0);
	cairo_stroke (cr);
	cairo_move_to (cr, LINE_LENGTH + 2, 0.5);
	cairo_rel_line_to (cr, LINE_LENGTH, 0);
	cairo_stroke (cr);
	cairo_translate (cr, 0, i+3);
    }

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
draw_a1 (cairo_t *cr, int width, int height)
{
    cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
    return draw (cr, width, height);
}

CAIRO_TEST (line_width,
	    "Tests cairo_set_line_width",
	    "stroke", /* keywords */
	    NULL, /* requirements */
	    IMAGE_WIDTH, IMAGE_HEIGHT,
	    NULL, draw)
CAIRO_TEST (a1_line_width,
	    "Tests cairo_set_line_width",
	    "stroke", /* keywords */
	    "target=raster", /* requirements */
	    IMAGE_WIDTH, IMAGE_HEIGHT,
	    NULL, draw_a1)
