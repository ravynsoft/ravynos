/*
 * Copyright © 2011 Uli Schlachter
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

static void
mime_data_destroy_func (void *data)
{
    cairo_bool_t *called = data;
    *called = TRUE;
}

static cairo_test_status_t
check_mime_data (cairo_test_context_t *ctx, cairo_surface_t *surface,
		 const char *mimetype, const unsigned char *data,
		 unsigned long length)
{
    const unsigned char *data_ret;
    unsigned long length_ret;

    cairo_surface_get_mime_data (surface, mimetype, &data_ret, &length_ret);
    if (data_ret != data || length_ret != length) {
	cairo_test_log (ctx,
			"Surface has mime data %p with length %lu, "
			"but expected %p with length %lu\n",
			data_ret, length_ret, data, length);
       return CAIRO_TEST_ERROR;
    }

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
set_and_check_mime_data (cairo_test_context_t *ctx, cairo_surface_t *surface,
			 const char *mimetype, const unsigned char *data,
			 unsigned long length, cairo_bool_t *destroy_called)
{
    cairo_status_t status;

    status = cairo_surface_set_mime_data (surface, mimetype,
					  data, length,
					  mime_data_destroy_func,
					  destroy_called);
    if (status) {
	cairo_test_log (ctx, "Could not set mime data to %s: %s\n",
			data, cairo_status_to_string(status));
	return CAIRO_TEST_ERROR;
    }

    return check_mime_data (ctx, surface, mimetype, data, length);
}

static cairo_test_status_t
preamble (cairo_test_context_t *ctx)
{
    const char *mimetype = "text/x-uri";
    const char *data1 = "https://www.cairographics.org";
    const char *data2 = "https://cairographics.org/examples/";
    cairo_bool_t destroy1_called = FALSE;
    cairo_bool_t destroy2_called = FALSE;
    cairo_surface_t *surface;
    cairo_test_status_t test_status = CAIRO_TEST_SUCCESS;

    surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 0, 0);
    if (cairo_surface_status (surface)) {
	cairo_test_log (ctx, "Could not create image surface\n");
	test_status = CAIRO_TEST_ERROR;
	goto out;
    }

    test_status = check_mime_data (ctx, surface, mimetype, NULL, 0);
    if (test_status)
	goto out;

    test_status = set_and_check_mime_data (ctx, surface, mimetype,
					   (const unsigned char *) data1,
					   strlen (data1),
					   &destroy1_called);
    if (test_status)
	goto out;

    if (destroy1_called) {
	cairo_test_log (ctx, "MIME data 1 destroyed too early\n");
	test_status = CAIRO_TEST_ERROR;
	goto out;
    }

    test_status = set_and_check_mime_data (ctx, surface, mimetype,
					   (const unsigned char *) data2,
					   strlen (data2),
					   &destroy2_called);
    if (test_status)
	goto out;

    if (!destroy1_called) {
	cairo_test_log (ctx, "MIME data 1 destroy callback not called\n");
	test_status = CAIRO_TEST_ERROR;
	goto out;
    }
    if (destroy2_called) {
	cairo_test_log (ctx, "MIME data 2 destroyed too early\n");
	test_status = CAIRO_TEST_ERROR;
	goto out;
    }

    test_status = set_and_check_mime_data (ctx, surface, mimetype,
					   NULL, 0, NULL);
    if (test_status)
	goto out;

    if (!destroy2_called) {
	cairo_test_log (ctx, "MIME data destroy callback not called\n");
	test_status = CAIRO_TEST_ERROR;
	goto out;
    }

out:
    cairo_surface_destroy (surface);

    return test_status;
}

CAIRO_TEST (mime_surface_api,
	    "Check the mime data API",
	    "api", /* keywords */
	    NULL, /* requirements */
	    0, 0,
	    preamble, NULL)
