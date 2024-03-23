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

#define SIZE 100
#define BORDER 10
#define LINE_WIDTH 20

static void
_propagate_status (cairo_t *dst, cairo_t *src)
{
    cairo_path_t path;

    path.status = cairo_status (src);
    if (path.status) {
	path.num_data = 0;
	path.data = NULL;
	cairo_append_path (dst, &path);
    }
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_surface_t *target_surface;
    cairo_t *cr2, *cr3;

    target_surface = cairo_get_group_target (cr);

    cr2 = cairo_create (target_surface);

    /* Draw a diagonal line and clip to it */

    cairo_move_to (cr2, BORDER,                     BORDER);
    cairo_line_to (cr2, BORDER + LINE_WIDTH,        BORDER);
    cairo_line_to (cr2, SIZE - BORDER,              SIZE - BORDER);
    cairo_line_to (cr2, SIZE - BORDER - LINE_WIDTH, SIZE - BORDER);

    cairo_clip (cr2);
    cairo_set_source_rgb (cr2, 0, 0, 1); /* Blue */
    cairo_paint (cr2);

    /* Clipping affects this cairo_t */

    cairo_set_source_rgb (cr2, 1, 1, 1); /* White */
    cairo_rectangle (cr2,
		     SIZE / 2 - LINE_WIDTH / 2, BORDER,
		     LINE_WIDTH,                SIZE - 2 * BORDER);
    cairo_fill (cr2);

    /* But doesn't affect another cairo_t that we create temporarily for
     * the same surface
     */
    cr3 = cairo_create (target_surface);
    cairo_set_source_rgb (cr3, 1, 1, 1); /* White */
    cairo_rectangle (cr3,
		     SIZE - BORDER - LINE_WIDTH, BORDER,
		     LINE_WIDTH,                 SIZE - 2 * BORDER);
    cairo_fill (cr3);

    _propagate_status (cr, cr3);
    cairo_destroy (cr3);

    _propagate_status (cr, cr2);
    cairo_destroy (cr2);

    /* And doesn't affect anything after this cairo_t is destroyed */

    cairo_set_source_rgb (cr, 1, 1, 1); /* White */
    cairo_rectangle (cr,
		     BORDER,     BORDER,
		     LINE_WIDTH, SIZE - 2 * BORDER);
    cairo_fill (cr);

    return CAIRO_TEST_SUCCESS;

}

CAIRO_TEST (clip_nesting,
	    "Test clipping with multiple contexts for the same surface",
	    "clip", /* keywords */
	    NULL, /* requirements */
	    SIZE, SIZE,
	    NULL, draw)
