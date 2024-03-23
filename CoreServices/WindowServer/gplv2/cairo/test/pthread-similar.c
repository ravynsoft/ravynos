/*
 * Copyright 2009 Benjamin Otte
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
 * Author: Benjamin Otte <otte@gnome.org>
 */

#include "cairo-test.h"
#include <pthread.h>

#define N_THREADS 8

#define WIDTH 64
#define HEIGHT 8

static void *
draw_thread (void *arg)
{
    cairo_surface_t *surface = arg;
    cairo_t *cr;
    int x, y;

    cr = cairo_create (surface);
    cairo_surface_destroy (surface);

    for (y = 0; y < HEIGHT; y++) {
        for (x = 0; x < WIDTH; x++) {
            cairo_rectangle (cr, x, y, 1, 1);
            cairo_set_source_rgba (cr, 0, 0.75, 0.75, (double) x / WIDTH);
            cairo_fill (cr);
        }
    }

    surface = cairo_surface_reference (cairo_get_target (cr));
    cairo_destroy (cr);

    return surface;
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    pthread_t threads[N_THREADS];
    cairo_test_status_t test_status = CAIRO_TEST_SUCCESS;
    int i;

    for (i = 0; i < N_THREADS; i++) {
	cairo_surface_t *surface;

        surface = cairo_surface_create_similar (cairo_get_target (cr),
						CAIRO_CONTENT_COLOR,
						WIDTH, HEIGHT);
        if (pthread_create (&threads[i], NULL, draw_thread, surface) != 0) {
	    threads[i] = pthread_self ();
            test_status = cairo_test_status_from_status (cairo_test_get_context (cr),
							 cairo_surface_status (surface));
            cairo_surface_destroy (surface);
	    break;
        }
    }

    for (i = 0; i < N_THREADS; i++) {
	void *surface;

        if (pthread_equal (threads[i], pthread_self ()))
            break;

        if (pthread_join (threads[i], &surface) == 0) {
	    cairo_set_source_surface (cr, surface, 0, 0);
	    cairo_surface_destroy (surface);
	    cairo_paint (cr);

	    cairo_translate (cr, 0, HEIGHT);
	} else {
            test_status = CAIRO_TEST_FAILURE;
	}
    }

    return test_status;
}

CAIRO_TEST (pthread_similar,
	    "Draw lots of 1x1 rectangles on similar surfaces in lots of threads",
	    "threads", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT * N_THREADS,
	    NULL, draw)
