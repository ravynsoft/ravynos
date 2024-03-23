/*
 * Copyright © 2021 Matthias Clasen
 * Copyright © 2021 Uli Schlachter
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
 * Authors:
 *	Matthias Clasen
 *	Uli Schlachter
 */

// Test case for https://gitlab.freedesktop.org/cairo/cairo/-/merge_requests/118
// A recording surface with a non-zero origin gets cut off when passed to
// cairo_surface_write_to_png().

#include "cairo-test.h"

#include <assert.h>

struct buffer {
    char *data;
    size_t length;
};

static cairo_surface_t *
prepare_recording (void)
{
    cairo_surface_t *surface;
    cairo_rectangle_t rect;
    cairo_t *cr;

    rect.x = -1;
    rect.y = -2;
    rect.width = 3;
    rect.height = 4;
    surface = cairo_recording_surface_create (CAIRO_CONTENT_COLOR_ALPHA, &rect);

    cr = cairo_create (surface);
    cairo_set_line_width (cr, 1);
    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_rectangle (cr, 0.5, -0.5, 1., 1.);
    cairo_stroke (cr);
    cairo_destroy (cr);

    return surface;
}

static cairo_status_t
write_callback (void *closure, const unsigned char *data, unsigned int length)
{
    struct buffer *buffer = closure;

    buffer->data = realloc (buffer->data, buffer->length + length);
    memcpy (&buffer->data[buffer->length], data, length);
    buffer->length += length;

    return CAIRO_STATUS_SUCCESS;
}

static cairo_status_t
read_callback (void *closure, unsigned char *data, unsigned int length)
{
    struct buffer *buffer = closure;

    assert (buffer->length >= length);
    memcpy (data, buffer->data, length);
    buffer->data += length;
    buffer->length -= length;

    return CAIRO_STATUS_SUCCESS;
}

static cairo_surface_t *
png_round_trip (cairo_surface_t *input_surface)
{
    cairo_surface_t *output_surface;
    struct buffer buffer;
    void *to_free;

    // Turn the surface into a PNG
    buffer.data = NULL;
    buffer.length = 0;
    cairo_surface_write_to_png_stream (input_surface, write_callback, &buffer);
    to_free = buffer.data;

    // Load the PNG again
    output_surface = cairo_image_surface_create_from_png_stream (read_callback, &buffer);

    free (to_free);
    return output_surface;
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_surface_t *recording, *surface;

    // Draw a black background so that the output does not vary with alpha
    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_paint (cr);

    recording = prepare_recording ();
    surface = png_round_trip (recording);

    cairo_set_source_surface (cr, surface, 0, 0);
    cairo_paint (cr);

    cairo_surface_destroy (recording);
    cairo_surface_destroy (surface);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (record_write_png,
	    "Test writing to png with non-zero origin",
	    "record, transform", /* keywords */
	    NULL, /* requirements */
	    4, 4,
	    NULL, draw)
