/*
 * Copyright © 2005 Red Hat, Inc.
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
 * Author: Owen Taylor <otaylor@redhat.com>
 */

#include "cairo-test.h"
#include <math.h>
#include <stdio.h>

#define SIZE 40

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_pattern_t *pattern;
    cairo_matrix_t matrix;

    /* Paint a diagonal division as a test image */
    cairo_set_source_rgb (cr, 1, 1, 1);	/* White */
    cairo_paint (cr);

    cairo_move_to (cr, SIZE,    0);
    cairo_line_to (cr, SIZE, SIZE);
    cairo_line_to (cr, 0,    SIZE);

    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_fill (cr);

    /* Create a pattern with the target surface as the source,
     * offset by SIZE/2
     */
    pattern = cairo_pattern_create_for_surface (cairo_get_group_target (cr));

    cairo_matrix_init_translate (&matrix, - SIZE / 2, - SIZE / 2);
    cairo_pattern_set_matrix (pattern, &matrix);

    cairo_set_source (cr, pattern);
    cairo_pattern_destroy (pattern);

    /* Copy two rectangles from the upper-left quarter of the image to
     * the lower right.  It will work if we use cairo_fill(), but the
     * cairo_clip() cairo_paint() combination fails because the clip
     * on the surface as a destination affects it as the source as
     * well.
     */
    cairo_rectangle (cr,
		     2 * SIZE / 4, 2 * SIZE / 4,
		     SIZE / 4,     SIZE / 4);
    cairo_rectangle (cr,
		     3 * SIZE / 4, 3 * SIZE / 4,
		     SIZE / 4,     SIZE / 4);
    cairo_clip (cr);
    cairo_paint (cr);

    return CAIRO_TEST_SUCCESS;

}

CAIRO_TEST (self_copy,
	    "Test copying from a surface to itself with a clip",
	    "paint", /* keywords */
	    NULL, /* requirements */
	    SIZE, SIZE,
	    NULL, draw)
