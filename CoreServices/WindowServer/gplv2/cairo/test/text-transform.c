/*
 * Copyright © 2006 Mozilla Corporation
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Mozilla Corporation not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. Mozilla Corporation makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * MOZILLA CORPORATION DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL MOZILLA CORPORATION BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Vladimir Vukicevic <vladimir@pobox.com>
 */

#include "cairo-test.h"

#define SIZE 100
#define PAD 5

#define FONT_SIZE 32.0

static const char *png_filename = "romedalen.png";

static void
draw_text (cairo_t *cr)
{
    cairo_matrix_t tm;

    /* skew */
    cairo_matrix_init (&tm, 1, 0,
                       -0.25, 1,
                       0, 0);
    cairo_matrix_scale (&tm, FONT_SIZE, FONT_SIZE);
    cairo_set_font_matrix (cr, &tm);

    cairo_new_path (cr);
    cairo_move_to (cr, 50, SIZE-PAD);
    cairo_show_text (cr, "A");

    /* rotate and scale */
    cairo_matrix_init_rotate (&tm, M_PI / 2);
    cairo_matrix_scale (&tm, FONT_SIZE, FONT_SIZE * 2.0);
    cairo_set_font_matrix (cr, &tm);

    cairo_new_path (cr);
    cairo_move_to (cr, PAD, PAD + 25);
    cairo_show_text (cr, "A");

    cairo_matrix_init_rotate (&tm, M_PI / 2);
    cairo_matrix_scale (&tm, FONT_SIZE * 2.0, FONT_SIZE);
    cairo_set_font_matrix (cr, &tm);

    cairo_new_path (cr);
    cairo_move_to (cr, PAD, PAD + 50);
    cairo_show_text (cr, "A");
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    const cairo_test_context_t *ctx = cairo_test_get_context (cr);
    cairo_pattern_t *pattern;

    cairo_set_source_rgb (cr, 1., 1., 1.);
    cairo_paint (cr);

    cairo_set_source_rgb (cr, 0., 0., 0.);

    cairo_select_font_face (cr, CAIRO_TEST_FONT_FAMILY " Sans",
			    CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);

    draw_text (cr);

    cairo_translate (cr, SIZE, SIZE);
    cairo_rotate (cr, M_PI);

    pattern = cairo_test_create_pattern_from_png (ctx, png_filename);
    cairo_pattern_set_extend (pattern, CAIRO_EXTEND_REPEAT);
    cairo_set_source (cr, pattern);
    cairo_pattern_destroy (pattern);

    draw_text (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (text_transform,
	    "Test various applications of the font matrix",
	    "text, transform", /* keywords */
	    NULL, /* requirements */
	    SIZE, SIZE,
	    NULL, draw)
