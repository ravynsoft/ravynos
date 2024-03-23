/*
 * Copyright © 2005 Mozilla Corporation
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

#define UNIT_SIZE 100
#define PAD 5
#define INNER_PAD 10

#define WIDTH (UNIT_SIZE + PAD) + PAD
#define HEIGHT (UNIT_SIZE + PAD) + PAD

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_pattern_t *gradient;
    int i, j;

    gradient = cairo_pattern_create_linear (UNIT_SIZE - (INNER_PAD*2), 0,
                                            UNIT_SIZE - (INNER_PAD*2), UNIT_SIZE - (INNER_PAD*2));
    cairo_pattern_add_color_stop_rgba (gradient, 0.0, 0.3, 0.3, 0.3, 1.0);
    cairo_pattern_add_color_stop_rgba (gradient, 1.0, 1.0, 1.0, 1.0, 1.0);

    for (j = 0; j < 1; j++) {
        for (i = 0; i < 1; i++) {
            double x = (i * UNIT_SIZE) + (i + 1) * PAD;
            double y = (j * UNIT_SIZE) + (j + 1) * PAD;

            cairo_save (cr);

            cairo_translate (cr, x, y);

            /* draw a gradient background */
            cairo_save (cr);
            cairo_translate (cr, INNER_PAD, INNER_PAD);
            cairo_new_path (cr);
            cairo_rectangle (cr, 0, 0,
                             UNIT_SIZE - (INNER_PAD*2), UNIT_SIZE - (INNER_PAD*2));
            cairo_set_source (cr, gradient);
            cairo_fill (cr);
            cairo_restore (cr);

            /* clip to the unit size */
            cairo_rectangle (cr, 0, 0,
                             UNIT_SIZE, UNIT_SIZE);
            cairo_clip (cr);

            cairo_rectangle (cr, 0, 0,
                             UNIT_SIZE, UNIT_SIZE);
            cairo_set_source_rgba (cr, 0, 0, 0, 1);
            cairo_set_line_width (cr, 2);
            cairo_stroke (cr);

            /* start a group */
            cairo_push_group (cr);

            /* draw diamond */
            cairo_move_to (cr, UNIT_SIZE / 2, 0);
            cairo_line_to (cr, UNIT_SIZE    , UNIT_SIZE / 2);
            cairo_line_to (cr, UNIT_SIZE / 2, UNIT_SIZE);
            cairo_line_to (cr, 0            , UNIT_SIZE / 2);
            cairo_close_path (cr);
            cairo_set_source_rgba (cr, 0, 0, 1, 1);
            cairo_fill (cr);

            /* draw circle */
            cairo_arc (cr,
                       UNIT_SIZE / 2, UNIT_SIZE / 2,
                       UNIT_SIZE / 3.5,
                       0, M_PI * 2);
            cairo_set_source_rgba (cr, 1, 0, 0, 1);
            cairo_fill (cr);

            cairo_pop_group_to_source (cr);
            cairo_paint_with_alpha (cr, 0.5);

            cairo_restore (cr);
        }
    }

    cairo_pattern_destroy (gradient);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (push_group,
	    "Verify that cairo_push_group works.",
	    "group", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw)
