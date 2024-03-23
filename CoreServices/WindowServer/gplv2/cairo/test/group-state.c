/* -*- Mode: c; c-basic-offset: 4; indent-tabs-mode: t; tab-width: 8; -*- */
/*
 * Copyright 2011 Andrea Canciani
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
 * Author: Andrea Canciani <ranma42@gmail.com>
 */

#include "cairo-test.h"

#define CHECK_STATUS(status)						\
    do {								\
	if (cairo_status (cr) != (status)) {				\
	    cairo_test_log (ctx, "Expected status: %s\n",		\
			    cairo_status_to_string (status));		\
	    cairo_test_log (ctx, "Actual status: %s\n",			\
			    cairo_status_to_string (cairo_status (cr))); \
	    result = CAIRO_TEST_FAILURE;				\
	}								\
    } while (0)

static void
reinit_cairo (cairo_t **cr)
{
    if (*cr)
	cairo_destroy (*cr);

    *cr = cairo_create (cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 1, 1));
    cairo_surface_destroy (cairo_get_target (*cr));
}

static cairo_test_status_t
preamble (cairo_test_context_t *ctx)
{
    cairo_t *cr;
    cairo_test_status_t result = CAIRO_TEST_SUCCESS;

    cr = NULL;

    reinit_cairo (&cr);

    /* cairo_restore() must fail with CAIRO_STATUS_INVALID_RESTORE if
     * no matching cairo_save() call has been performed. */
    cairo_test_log (ctx, "Checking save(); push(); restore();\n");
    cairo_save (cr);
    CHECK_STATUS (CAIRO_STATUS_SUCCESS);
    cairo_push_group (cr);
    CHECK_STATUS (CAIRO_STATUS_SUCCESS);
    cairo_restore (cr);
    CHECK_STATUS (CAIRO_STATUS_INVALID_RESTORE);


    reinit_cairo (&cr);

    /* cairo_restore() must fail with CAIRO_STATUS_INVALID_RESTORE if
     * no matching cairo_save() call has been performed. */
    cairo_test_log (ctx, "Checking push(); save(); pop();\n");
    cairo_push_group (cr);
    CHECK_STATUS (CAIRO_STATUS_SUCCESS);
    cairo_save (cr);
    CHECK_STATUS (CAIRO_STATUS_SUCCESS);
    cairo_pop_group_to_source (cr);
    CHECK_STATUS (CAIRO_STATUS_INVALID_POP_GROUP);


    cairo_destroy (cr);

    return result;
}

CAIRO_TEST (group_state,
	    "Tests the interaction between state (cairo_save, cairo_restore) "
	    "and group (cairo_push_group/cairo_pop_group) API",
	    "api", /* keywords */
	    NULL, /* requirements */
	    0, 0,
	    preamble, NULL)
