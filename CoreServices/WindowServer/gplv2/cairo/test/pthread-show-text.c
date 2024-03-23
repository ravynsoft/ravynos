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
 * Author: Carl D. Worth <cworth@cworth.org>
 */

/* Test case for bug #4299:

   Assertion fails in "cairo-font.c" when using multithreads
   https://bugs.freedesktop.org/show_bug.cgi?id=4299
*/

#include "cairo-test.h"

#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#define N_THREADS 8
#define NUM_ITERATIONS 40

#define WIDTH 400
#define HEIGHT 42

typedef struct {
  cairo_surface_t *target;
  int id;
} thread_data_t;

static void *
draw_thread (void *arg)
{
    const char *text = "Hello world. ";
    thread_data_t *thread_data = arg;
    cairo_surface_t *surface;
    cairo_font_extents_t extents;
    cairo_t *cr;
    int i;

    cr = cairo_create (thread_data->target);
    cairo_surface_destroy (thread_data->target);

    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);
    cairo_set_source_rgb (cr, 0, 0, 0);

    cairo_select_font_face (cr, CAIRO_TEST_FONT_FAMILY " Serif",
			    CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size (cr, NUM_ITERATIONS);
    cairo_font_extents (cr, &extents);

    cairo_move_to (cr, 1, HEIGHT - extents.descent - 1);

    for (i = 0; i < NUM_ITERATIONS; i++) {
	char buf[2];

	cairo_select_font_face (cr, CAIRO_TEST_FONT_FAMILY " Serif",
				CAIRO_FONT_SLANT_NORMAL,
				CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size (cr, i);

	buf[0] = text[i%strlen(text)];
	buf[1] = '\0';
	cairo_show_text (cr, buf);
    }

    surface = cairo_surface_reference (cairo_get_target (cr));
    cairo_destroy (cr);

    return surface;
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    pthread_t threads[N_THREADS];
    thread_data_t thread_data[N_THREADS];
    cairo_test_status_t test_status = CAIRO_TEST_SUCCESS;
    int i;

    for (i = 0; i < N_THREADS; i++) {
        thread_data[i].target = cairo_surface_create_similar (cairo_get_target (cr),
							      cairo_surface_get_content (cairo_get_target (cr)),
							      WIDTH, HEIGHT);
        thread_data[i].id = i;
        if (pthread_create (&threads[i], NULL, draw_thread, &thread_data[i]) != 0) {
	    threads[i] = pthread_self (); /* to indicate error */
            cairo_surface_destroy (thread_data[i].target);
            test_status = CAIRO_TEST_FAILURE;
	    break;
        }
    }

    cairo_set_source_rgb (cr, 0.5, 0.5, 0.5);
    cairo_paint (cr);

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

CAIRO_TEST (pthread_show_text,
	    "Concurrent stress test of the cairo_show_text().",
	    "thread, text", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT * N_THREADS,
	    NULL, draw)
