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

#define GENERATE_REFERENCE 0

#include "cairo-test.h"
#if !GENERATE_REFERENCE
#include <pthread.h>
#endif

#define N_THREADS 8

#define WIDTH 64
#define HEIGHT 8

typedef struct {
  cairo_surface_t *target;
  cairo_surface_t *source;
  int id;
} thread_data_t;

static void *
draw_thread (void *arg)
{
    thread_data_t *thread_data = arg;
    cairo_surface_t *surface;
    cairo_pattern_t *pattern;
    cairo_matrix_t pattern_matrix = { 2, 0, 0, 2, 0, 0 };
    cairo_t *cr;
    int x, y;

    cr = cairo_create (thread_data->target);
    cairo_surface_destroy (thread_data->target);

    pattern = cairo_pattern_create_for_surface (thread_data->source);
    cairo_surface_destroy (thread_data->source);
    cairo_pattern_set_extend (pattern, thread_data->id % 4);
    cairo_pattern_set_filter (pattern, thread_data->id >= 4 ? CAIRO_FILTER_BILINEAR : CAIRO_FILTER_NEAREST);
    cairo_pattern_set_matrix (pattern, &pattern_matrix);

    for (y = 0; y < HEIGHT; y++) {
        for (x = 0; x < WIDTH; x++) {
            cairo_save (cr);
            cairo_translate (cr, 4 * x + 1, 4 * y + 1);
            cairo_rectangle (cr, 0, 0, 2, 2);
            cairo_set_source (cr, pattern);
            cairo_fill (cr);
            cairo_restore (cr);
        }
    }
    cairo_pattern_destroy (pattern);

    surface = cairo_surface_reference (cairo_get_target (cr));
    cairo_destroy (cr);

    return surface;
}

static cairo_surface_t *
create_source (cairo_surface_t *similar)
{
    cairo_surface_t *source;
    cairo_t *cr;
    double colors[4][3] = {
      { 0.75, 0,    0    },
      { 0,    0.75, 0    },
      { 0,    0,    0.75 },
      { 0.75, 0.75, 0    }
    };
    int i;

    source = cairo_surface_create_similar (similar,
                                           CAIRO_CONTENT_COLOR_ALPHA,
                                           2, 2);

    cr = cairo_create (source);
    cairo_surface_destroy (source);

    for (i = 0; i < 4; i++) {
      cairo_set_source_rgb (cr, colors[i][0], colors[i][1], colors[i][2]);
      cairo_rectangle (cr, i % 2, i / 2, 1, 1);
      cairo_fill (cr);
    }

    source = cairo_surface_reference (cairo_get_target (cr));
    cairo_destroy (cr);

    return source;
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
#if !GENERATE_REFERENCE
    pthread_t threads[N_THREADS];
#endif
    thread_data_t thread_data[N_THREADS];
    cairo_test_status_t test_status = CAIRO_TEST_SUCCESS;
    cairo_surface_t *source;
    cairo_status_t status;
    int i;

    source = create_source (cairo_get_target (cr));
    status = cairo_surface_status (source);
    if (status) {
	cairo_surface_destroy (source);
	return cairo_test_status_from_status (cairo_test_get_context (cr),
					      status);
    }

    cairo_set_source_rgb (cr, 0.5, 0.5, 0.5);
    cairo_paint (cr);

    for (i = 0; i < N_THREADS; i++) {
        thread_data[i].target = cairo_surface_create_similar (cairo_get_target (cr),
                                                              CAIRO_CONTENT_COLOR_ALPHA,
                                                              4 * WIDTH, 4 * HEIGHT);
        thread_data[i].source = cairo_surface_reference (source);
        thread_data[i].id = i;
#if !GENERATE_REFERENCE
        if (pthread_create (&threads[i], NULL, draw_thread, &thread_data[i]) != 0) {
	    threads[i] = pthread_self (); /* to indicate error */
            cairo_surface_destroy (thread_data[i].target);
            cairo_surface_destroy (thread_data[i].source);
            test_status = CAIRO_TEST_FAILURE;
	    break;
        }
#else
	{
	    cairo_surface_t *surface = draw_thread(&thread_data[i]);
	    cairo_set_source_surface (cr, surface, 0, 0);
	    cairo_surface_destroy (surface);
	    cairo_paint (cr);

	    cairo_translate (cr, 0, 4 * HEIGHT);
	}
#endif
    }

    cairo_surface_destroy (source);

#if !GENERATE_REFERENCE
    for (i = 0; i < N_THREADS; i++) {
	void *surface;

        if (pthread_equal (threads[i], pthread_self ()))
            break;

        if (pthread_join (threads[i], &surface) == 0) {
	    cairo_set_source_surface (cr, surface, 0, 0);
	    cairo_surface_destroy (surface);
	    cairo_paint (cr);

	    cairo_translate (cr, 0, 4 * HEIGHT);
	} else {
            test_status = CAIRO_TEST_FAILURE;
	}
    }
#endif

    return test_status;
}

CAIRO_TEST (pthread_same_source,
	    "Use the same source for drawing in different threads",
	    "threads", /* keywords */
	    NULL, /* requirements */
	    4 * WIDTH, 4 * HEIGHT * N_THREADS,
	    NULL, draw)
