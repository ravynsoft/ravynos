/*
 * Copyright © 2022 Uli Schlachter
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
 * Author: Uli Schlachter <psychon@znc.in>
 */

#include "cairo-test.h"
#include "cairo-script.h"

struct write_data {
    cairo_bool_t finished;
    cairo_test_status_t test_status;
};

static cairo_surface_t*
create_recording_surface ()
{
    /* Create a non-empty recording surface with arbitrary content */
    cairo_surface_t *surf = cairo_recording_surface_create (CAIRO_CONTENT_COLOR, NULL);
    cairo_t *cr = cairo_create (surf);

    cairo_move_to (cr, 0, 0);
    cairo_line_to (cr, 10, 0);
    cairo_stroke (cr);

    cairo_destroy (cr);
    return surf;
}

static cairo_status_t
write_func(void *closure, const unsigned char* bytes, unsigned int length)
{
    struct write_data *data = closure;
    (void) bytes; (void) length;

    if (data->finished)
	data->test_status = CAIRO_TEST_ERROR;

    return CAIRO_STATUS_SUCCESS;
}

static cairo_test_status_t
preamble (cairo_test_context_t *ctx)
{
    struct write_data write_data = { FALSE, CAIRO_TEST_SUCCESS };
    cairo_device_t *script_device = cairo_script_create_for_stream (write_func, &write_data);
    cairo_surface_t *recording = create_recording_surface ();
    cairo_surface_t *script;
    cairo_t *cr;

    /* Draw the recording surface to a script surface */
    script = cairo_script_surface_create (script_device, CAIRO_CONTENT_COLOR, 5, 5);
    cr = cairo_test_create (script, ctx);
    cairo_set_source_surface (cr, recording, 0, 0);
    cairo_paint (cr);
    cairo_destroy (cr);
    cairo_surface_destroy (script);

    /* Finish the script device; no further writing allowed afterwards */
    cairo_device_finish (script_device);
    write_data.finished = TRUE;
    cairo_device_destroy (script_device);

    cairo_surface_destroy (recording);

    return write_data.test_status;
}

CAIRO_TEST (bug_277,
	    "Regression test: Script surface emitting test after finish()",
	    NULL, /* keywords */
	    NULL, /* requirements */
	    0, 0,
	    preamble, NULL)
