/*
 * Copyright © 2006 Red Hat, Inc.
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

/* A test for the crash described here:
 *
 *	https://lists.freedesktop.org/archives/cairo/2006-August/007698.html
 *
 * The triggering condition for this bug should be setting a
 * surface-based clip and then calling cairo_push_group.
 */

#include "cairo-test.h"

#define SIZE 10
#define PAD 2

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    /* An overly complex way of drawing a blue circle onto a red
     * background, to trigger the bug. */
    cairo_set_source_rgb (cr, 1, 0, 0); /* red */
    cairo_paint (cr);

    cairo_arc (cr,
	       SIZE / 2, SIZE / 2,
	       SIZE / 2 - PAD,
	       0, 2 * M_PI);
    cairo_clip (cr);

    cairo_push_group (cr);
    cairo_set_source_rgb (cr, 0, 0, 1); /* blue */
    cairo_paint (cr);
    cairo_pop_group_to_source (cr);
    cairo_paint (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (clip_push_group,
	    "Test that push_group doesn't crash after setting a surface-based clip",
	    "clip", /* keywords */
	    NULL, /* requirements */
	    SIZE, SIZE,
	    NULL, draw)
