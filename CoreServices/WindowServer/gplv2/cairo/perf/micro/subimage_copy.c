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

#include "cairo-perf.h"

/* This case exposes a performance bug found by Christopher "Monty"
 * Montgomery in that copying a tiny portion of an image surface to an
 * X surface causes the entire image surface to be copied to an
 * intermediate surface.
 *
 * If the performance bug is fixed, then the time this test takes
 * should be independent of the source and destination surface sizes.
 */

static cairo_time_t
do_subimage_copy (cairo_t *cr, int width, int height, int loops)
{
    cairo_rectangle (cr, 2, 2, 4, 4);
    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);

    cairo_perf_timer_start ();

    while (loops--)
	cairo_fill_preserve (cr);

    cairo_perf_timer_stop ();

    cairo_new_path (cr);

    return cairo_perf_timer_elapsed ();
}

cairo_bool_t
subimage_copy_enabled (cairo_perf_t *perf)
{
    return cairo_perf_can_run (perf, "subimage-copy", NULL);
}

void
subimage_copy (cairo_perf_t *perf, cairo_t *cr, int width, int height)
{
    cairo_surface_t *image;
    cairo_t *cr2;

    cairo_set_source_rgb (cr, 0, 0, 1); /* blue */
    cairo_paint (cr);

    image = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
    cr2 = cairo_create (image);
    cairo_set_source_rgb (cr2, 1, 0, 0); /* red */
    cairo_paint (cr2);
    cairo_destroy (cr2);

    cairo_set_source_surface (cr, image, 0, 0);
    cairo_surface_destroy (image);

    cairo_perf_run (perf, "subimage-copy", do_subimage_copy, NULL);
}
