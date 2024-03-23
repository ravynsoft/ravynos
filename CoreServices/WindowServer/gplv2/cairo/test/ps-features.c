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

#include "cairo-test.h"

#include <stdio.h>
#include <cairo.h>
#include <cairo-ps.h>

/* This test exists to test the various features of cairo-ps.h.
 *
 * Currently, this test exercises the following function calls:
 *
 *	cairo_ps_surface_set_size
 *	cairo_ps_surface_dsc_comment
 *	cairo_ps_surface_dsc_begin_setup
 *	cairo_ps_surface_dsc_begin_page_setup
 */

#define INCHES_TO_POINTS(in) ((in) * 72.0)
#define MM_TO_POINTS(mm) ((mm) / 25.4 * 72.0)
#define TEXT_SIZE 12
#define BASENAME "ps-features.out"

static struct {
    const char *page_size;
    const char *page_size_alias;
    const char *orientation;
    double width_in_points;
    double height_in_points;
} pages[] = {
    {"na_letter_8.5x11in", "letter", "portrait",
     INCHES_TO_POINTS(8.5), INCHES_TO_POINTS(11)},
    {"na_letter_8.5x11in", "letter", "landscape",
     INCHES_TO_POINTS(11), INCHES_TO_POINTS(8.5)},
    {"iso_a4_210x297mm", "a4", "portrait",
     MM_TO_POINTS(210), MM_TO_POINTS(297)},
    {"iso_a4_210x297mm", "a4", "landscape",
     MM_TO_POINTS(297), MM_TO_POINTS(210)},
    {"iso_a5_148x210mm", "a5", "portrait",
     MM_TO_POINTS(148), MM_TO_POINTS(210)},
    {"iso_a5_148x210mm", "a5", "landscape",
     MM_TO_POINTS(210), MM_TO_POINTS(148)},
    {"iso_a6_105x148mm", "a6", "portrait",
     MM_TO_POINTS(105), MM_TO_POINTS(148)},
    {"iso_a6_105x148mm", "a6", "landscape",
     MM_TO_POINTS(148), MM_TO_POINTS(105)},
    {"iso_a7_74x105mm", "a7", "portrait",
     MM_TO_POINTS(74), MM_TO_POINTS(105)},
    {"iso_a7_74x105mm", "a7", "landscape",
     MM_TO_POINTS(105), MM_TO_POINTS(74)},
    {"iso_a8_52x74mm", "a8", "portrait",
     MM_TO_POINTS(52), MM_TO_POINTS(74)},
    {"iso_a8_52x74mm", "a8", "landscape",
     MM_TO_POINTS(74), MM_TO_POINTS(52)},
    {"iso_a9_37x52mm", "a9", "portrait",
     MM_TO_POINTS(37), MM_TO_POINTS(52)},
    {"iso_a9_37x52mm", "a9", "landscape",
     MM_TO_POINTS(52), MM_TO_POINTS(37)},
    {"iso_a10_26x37mm", "a10", "portrait",
     MM_TO_POINTS(26), MM_TO_POINTS(37)},
    {"iso_a10_26x37mm", "a10", "landscape",
     MM_TO_POINTS(37), MM_TO_POINTS(26)}
};

static cairo_test_status_t
preamble (cairo_test_context_t *ctx)
{
    cairo_surface_t *surface;
    cairo_t *cr;
    cairo_status_t status;
    size_t i;
    char dsc[255];
    char *filename;
    const char *path = cairo_test_mkdir (CAIRO_TEST_OUTPUT_DIR) ? CAIRO_TEST_OUTPUT_DIR : ".";

    if (! (cairo_test_is_target_enabled (ctx, "ps2") ||
	   cairo_test_is_target_enabled (ctx, "ps3")))
    {
	return CAIRO_TEST_UNTESTED;
    }

    xasprintf (&filename, "%s/%s.ps", path, BASENAME);
    /* We demonstrate that the initial size doesn't matter (we're
     * passing 0,0), if we use cairo_ps_surface_set_size on the first
     * page. */
    surface = cairo_ps_surface_create (filename, 0, 0);

    cairo_ps_surface_dsc_comment (surface, "%%Title: ps-features");
    cairo_ps_surface_dsc_comment (surface, "%%Copyright: Copyright (C) 2006 Red Hat, Inc.");

    cairo_ps_surface_dsc_begin_setup (surface);
    cairo_ps_surface_dsc_comment (surface, "%%IncludeFeature: *PageSize letter");
    cairo_ps_surface_dsc_comment (surface, "%%IncludeFeature: *MediaColor White");

    cr = cairo_create (surface);

    cairo_select_font_face (cr, CAIRO_TEST_FONT_FAMILY " Sans",
			    CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size (cr, TEXT_SIZE);

    for (i = 0; i < ARRAY_LENGTH (pages); i++) {
	cairo_ps_surface_set_size (surface,
				   pages[i].width_in_points,
				   pages[i].height_in_points);
	cairo_ps_surface_dsc_begin_page_setup (surface);
	snprintf (dsc, 255, "%%IncludeFeature: *PageSize %s", pages[i].page_size_alias);
	cairo_ps_surface_dsc_comment (surface, dsc);
	if (i % 2) {
	    snprintf (dsc, 255, "%%IncludeFeature: *MediaType Glossy");
	    cairo_ps_surface_dsc_comment (surface, dsc);
	}

	cairo_move_to (cr, TEXT_SIZE, TEXT_SIZE);
	cairo_show_text (cr, pages[i].page_size);
	cairo_show_text (cr, " - ");
	cairo_show_text (cr, pages[i].orientation);
	cairo_show_page (cr);
    }

    status = cairo_status (cr);

    cairo_destroy (cr);
    cairo_surface_destroy (surface);

    if (status) {
	cairo_test_log (ctx, "Failed to create ps surface for file %s: %s\n",
			filename, cairo_status_to_string (status));
	free (filename);
	return CAIRO_TEST_FAILURE;
    }

    printf ("ps-features: Please check %s to ensure it looks/prints correctly.\n", filename);
    free (filename);
    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (ps_features,
	    "Check PS specific API",
	    "ps, api", /* keywords */
	    NULL, /* requirements */
	    0, 0,
	    preamble, NULL)
