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

#include "cairo-test.h"

#define SIZE 8
#define PAD 2

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_surface_t *group;
    double x, y;

    /* First paint background in blue. */
    cairo_set_source_rgb (cr, 0.0, 0.0, 1.0);
    cairo_paint (cr);

    /* Then clip so that the group surface ends up smaller than the
     * original surface. */
    cairo_rectangle (cr, PAD, PAD, width - 2 * PAD, height - 2 * PAD);
    cairo_clip (cr);

    /* Paint the clipped region in red (which should all be overwritten later). */
    cairo_set_source_rgb (cr, 1.0, 0.0, 0.0);
    cairo_paint (cr);

    /* Redirect to a new group and get that surface. */
    cairo_push_group (cr);
    group = cairo_get_group_target (cr);

    /* Then paint in green what we query the group surface size to be. */
    cairo_set_source_rgb (cr, 0.0, 1.0, 0.0);
    cairo_surface_get_device_offset (group, &x, &y);
    /* Or rather, we calculate the group surface size based on the
     * only thing we can query which is the device offset. Ideally,
     * the size would always be the minimal (width - 2 * PAD, height -
     * 2 * PAD) based on the clip. But currently, group targets are
     * created oversized for paginated surfaces, so we only subtract
     * anything from the size if there is a non-zero device offfset.
     *
     * The calculation below might also be less confusing if the sign
     * convention on the device offset were reversed, but it is what
     * it is. Oh well. */
    cairo_rectangle (cr,
		     -x, -y,
		     width + 2 * x,
		     height + 2 * y);
    cairo_fill (cr);

    /* Finish up the group painting. */
    cairo_pop_group_to_source (cr);
    cairo_paint (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (get_group_target,
	    "Test of both cairo_get_group_target and cairo_surface_get_device_offset",
	    "api", /* keywords */
	    NULL, /* requirements */
	    SIZE, SIZE,
	    NULL, draw)
