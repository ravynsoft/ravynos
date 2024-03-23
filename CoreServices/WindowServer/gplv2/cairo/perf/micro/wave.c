/*
 * Copyright 2011 Red Hat Inc.
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
 * Author: Benjamin Otte <otte@redhat.com>
 */

#include "cairo-perf.h"

static cairo_surface_t *
generate_random_waveform(cairo_t *target, int width, int height)
{
    cairo_surface_t *surface;
    cairo_t *cr;
    int i, r;

    srand (0xdeadbeef);

    surface = cairo_surface_create_similar (cairo_get_target (target),
					    CAIRO_CONTENT_ALPHA,
					    width, height);
    cr = cairo_create (surface);

    r = height / 2;

    for (i = 0; i < width; i++)
    {
	r += rand () % (height / 4) - (height / 8);
	if (r < 0)
	    r = 0;
	else if (r > height)
	    r = height;
	cairo_rectangle (cr, i, (height - r) / 2, 1, r);
    }
    cairo_fill (cr);
    cairo_destroy (cr);

    return surface;
}

static cairo_time_t
do_wave (cairo_t *cr, int width, int height, int loops)
{
    cairo_surface_t *wave;
    cairo_pattern_t *mask;

    wave = generate_random_waveform (cr, width, height);

    cairo_perf_timer_start ();

    while (loops--) {
	/* paint outline (and contents) */
	cairo_set_source_rgb (cr, 1, 0, 0);
	cairo_mask_surface (cr, wave, 0, 0);

	/* overdraw inline */
	/* first, create a mask */
	cairo_push_group_with_content (cr, CAIRO_CONTENT_ALPHA);
	cairo_set_source_surface (cr, wave, 0, 0);
	cairo_paint (cr);
	cairo_set_operator (cr, CAIRO_OPERATOR_IN);
	cairo_set_source_surface (cr, wave, 1, 0);
	cairo_paint (cr);
	cairo_set_source_surface (cr, wave, -1, 0);
	cairo_paint (cr);
	cairo_set_source_surface (cr, wave, 0, 1);
	cairo_paint (cr);
	cairo_set_source_surface (cr, wave, 0, -1);
	cairo_paint (cr);
	mask = cairo_pop_group (cr);

	/* second, paint the mask */
	cairo_set_source_rgb (cr, 0, 1, 0);
	cairo_mask (cr, mask);

	cairo_pattern_destroy (mask);
    }

    cairo_perf_timer_stop ();

    cairo_surface_destroy (wave);

    return cairo_perf_timer_elapsed ();
}

cairo_bool_t
wave_enabled (cairo_perf_t *perf)
{
    return cairo_perf_can_run (perf, "wave", NULL);
}

void
wave (cairo_perf_t *perf, cairo_t *cr, int width, int height)
{
    cairo_perf_run (perf, "wave", do_wave, NULL);
}
