/*
 * Copyright © 2006 Red Hat, Inc.
 * Copyright © 2009 Chris Wilson
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
 *         Chris Wilson <chris@chris-wilson.co.uk>
 */

#include "cairo-perf.h"

static cairo_time_t
do_mask_solid (cairo_t *cr, int width, int height, int loops)
{
    cairo_pattern_t *mask;

    mask = cairo_pattern_create_rgba (0, 0, 0, .5);

    cairo_perf_timer_start ();

    while (loops--)
	cairo_mask (cr, mask);

    cairo_perf_timer_stop ();

    cairo_pattern_destroy (mask);

    return cairo_perf_timer_elapsed ();
}

static cairo_surface_t *
init_surface (cairo_surface_t *surface, int width, int height)
{
    cairo_t *cr;

    cr = cairo_create (surface);
    cairo_surface_destroy (surface);

    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgb (cr, 0, 0, 0); /* back */
    cairo_paint (cr);

    cairo_set_source_rgba (cr, 1, 1, 1, 0.5); /* 50% */
    cairo_new_path (cr);
    cairo_rectangle (cr, 0, 0, width/2.0, height/2.0);
    cairo_rectangle (cr, width/2.0, height/2.0, width/2.0, height/2.0);
    cairo_fill (cr);

    surface = cairo_surface_reference (cairo_get_target (cr));
    cairo_destroy (cr);

    return surface;
}

static cairo_time_t
do_mask_image (cairo_t *cr, int width, int height, int loops)
{
    cairo_surface_t *surface;
    cairo_pattern_t *mask;

    surface = cairo_image_surface_create (CAIRO_FORMAT_A8, width, height);
    mask = cairo_pattern_create_for_surface (init_surface (surface,
							   width,
							   height));
    cairo_surface_destroy (surface);

    cairo_perf_timer_start ();

    while (loops--)
	cairo_mask (cr, mask);

    cairo_perf_timer_stop ();

    cairo_pattern_destroy (mask);

    return cairo_perf_timer_elapsed ();
}

static cairo_time_t
do_mask_image_half (cairo_t *cr, int width, int height, int loops)
{
    cairo_surface_t *surface;
    cairo_pattern_t *mask;
    cairo_matrix_t matrix;

    surface = cairo_image_surface_create (CAIRO_FORMAT_A8, width, height);
    mask = cairo_pattern_create_for_surface (init_surface (surface,
							   width,
							   height));
    cairo_surface_destroy (surface);
    cairo_matrix_init_scale (&matrix, .5, .5);
    cairo_pattern_set_matrix (mask, &matrix);

    cairo_perf_timer_start ();

    while (loops--)
	cairo_mask (cr, mask);

    cairo_perf_timer_stop ();

    cairo_pattern_destroy (mask);

    return cairo_perf_timer_elapsed ();
}

static cairo_time_t
do_mask_image_double (cairo_t *cr, int width, int height, int loops)
{
    cairo_surface_t *surface;
    cairo_pattern_t *mask;
    cairo_matrix_t matrix;

    surface = cairo_image_surface_create (CAIRO_FORMAT_A8, width, height);
    mask = cairo_pattern_create_for_surface (init_surface (surface,
							   width,
							   height));
    cairo_surface_destroy (surface);
    cairo_matrix_init_scale (&matrix, 2., 2.);
    cairo_pattern_set_matrix (mask, &matrix);

    cairo_perf_timer_start ();

    while (loops--)
	cairo_mask (cr, mask);

    cairo_perf_timer_stop ();

    cairo_pattern_destroy (mask);

    return cairo_perf_timer_elapsed ();
}

static cairo_time_t
do_mask_similar (cairo_t *cr, int width, int height, int loops)
{
    cairo_surface_t *surface;
    cairo_pattern_t *mask;

    surface = cairo_surface_create_similar (cairo_get_group_target (cr),
					    CAIRO_CONTENT_ALPHA, width, height);
    mask = cairo_pattern_create_for_surface (init_surface (surface,
							   width,
							   height));
    cairo_surface_destroy (surface);

    cairo_perf_timer_start ();

    while (loops--)
	cairo_mask (cr, mask);

    cairo_perf_timer_stop ();

    cairo_pattern_destroy (mask);

    return cairo_perf_timer_elapsed ();
}

static cairo_time_t
do_mask_similar_half (cairo_t *cr, int width, int height, int loops)
{
    cairo_surface_t *surface;
    cairo_pattern_t *mask;
    cairo_matrix_t matrix;

    surface = cairo_surface_create_similar (cairo_get_group_target (cr),
					    CAIRO_CONTENT_ALPHA, width, height);
    mask = cairo_pattern_create_for_surface (init_surface (surface,
							   width,
							   height));
    cairo_surface_destroy (surface);
    cairo_matrix_init_scale (&matrix, .5, .5);
    cairo_pattern_set_matrix (mask, &matrix);

    cairo_perf_timer_start ();

    while (loops--)
	cairo_mask (cr, mask);

    cairo_perf_timer_stop ();

    cairo_pattern_destroy (mask);

    return cairo_perf_timer_elapsed ();
}

static cairo_time_t
do_mask_similar_double (cairo_t *cr, int width, int height, int loops)
{
    cairo_surface_t *surface;
    cairo_pattern_t *mask;
    cairo_matrix_t matrix;

    surface = cairo_surface_create_similar (cairo_get_group_target (cr),
					    CAIRO_CONTENT_ALPHA, width, height);
    mask = cairo_pattern_create_for_surface (init_surface (surface,
							   width,
							   height));
    cairo_surface_destroy (surface);
    cairo_matrix_init_scale (&matrix, 2., 2.);
    cairo_pattern_set_matrix (mask, &matrix);

    cairo_perf_timer_start ();

    while (loops--)
	cairo_mask (cr, mask);

    cairo_perf_timer_stop ();

    cairo_pattern_destroy (mask);

    return cairo_perf_timer_elapsed ();
}

static cairo_time_t
do_mask_linear (cairo_t *cr, int width, int height, int loops)
{
    cairo_pattern_t *mask;

    mask = cairo_pattern_create_linear (0.0, 0.0, width, height);
    cairo_pattern_add_color_stop_rgba (mask, 0.0, 0, 0, 0, 0.5); /*  50% */
    cairo_pattern_add_color_stop_rgba (mask, 0.0, 0, 0, 0, 1.0); /* 100% */

    cairo_perf_timer_start ();

    while (loops--)
	cairo_mask (cr, mask);

    cairo_perf_timer_stop ();

    cairo_pattern_destroy (mask);

    return cairo_perf_timer_elapsed ();
}

static cairo_time_t
do_mask_radial (cairo_t *cr, int width, int height, int loops)
{
    cairo_pattern_t *mask;

    mask = cairo_pattern_create_radial (width/2.0, height/2.0, 0.0,
					width/2.0, height/2.0, width/2.0);
    cairo_pattern_add_color_stop_rgba (mask, 0.0, 0, 0, 0, 0.5); /*  50% */
    cairo_pattern_add_color_stop_rgba (mask, 0.0, 0, 0, 0, 1.0); /* 100% */

    cairo_perf_timer_start ();

    while (loops--)
	cairo_mask (cr, mask);

    cairo_perf_timer_stop ();

    cairo_pattern_destroy (mask);

    return cairo_perf_timer_elapsed ();
}

cairo_bool_t
mask_enabled (cairo_perf_t *perf)
{
    return cairo_perf_can_run (perf, "mask", NULL);
}

void
mask (cairo_perf_t *perf, cairo_t *cr, int width, int height)
{
    if (! cairo_perf_can_run (perf, "mask", NULL))
	return;

    cairo_perf_cover_sources_and_operators (perf, "mask-solid",
					    do_mask_solid, NULL);
    cairo_perf_cover_sources_and_operators (perf, "mask-image",
					    do_mask_image, NULL);
    cairo_perf_cover_sources_and_operators (perf, "mask-image-half",
					    do_mask_image_half, NULL);
    cairo_perf_cover_sources_and_operators (perf, "mask-image-double",
					    do_mask_image_double, NULL);
    cairo_perf_cover_sources_and_operators (perf, "mask-similar",
					    do_mask_similar, NULL);
    cairo_perf_cover_sources_and_operators (perf, "mask-similar-half",
					    do_mask_similar_half, NULL);
    cairo_perf_cover_sources_and_operators (perf, "mask-similar-double",
					    do_mask_similar_double, NULL);
    cairo_perf_cover_sources_and_operators (perf, "mask-linear",
					    do_mask_linear, NULL);
    cairo_perf_cover_sources_and_operators (perf, "mask-radial",
					    do_mask_radial, NULL);
}
