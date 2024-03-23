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

#include "cairo-test.h"

#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <cairo-ft.h>
#include <fontconfig/fontconfig.h>
#include <fontconfig/fcfreetype.h>

#define FONT "6x13.pcf"
#define TEXT_SIZE 13

static cairo_bool_t
font_extents_equal (const cairo_font_extents_t *A,
	            const cairo_font_extents_t *B)
{
    return
	CAIRO_TEST_DOUBLE_EQUALS (A->ascent,  B->ascent)  &&
	CAIRO_TEST_DOUBLE_EQUALS (A->descent, B->descent) &&
	CAIRO_TEST_DOUBLE_EQUALS (A->height,  B->height)  &&
	CAIRO_TEST_DOUBLE_EQUALS (A->max_x_advance, B->max_x_advance) &&
	CAIRO_TEST_DOUBLE_EQUALS (A->max_y_advance, B->max_y_advance);
}

static cairo_test_status_t
check_font_extents (const cairo_test_context_t *ctx, cairo_t *cr, const char *comment)
{
    cairo_font_extents_t font_extents, ref_font_extents = {11, 2, 13, 6, 0};
    cairo_status_t status;

    memset (&font_extents, 0xff, sizeof (cairo_font_extents_t));
    cairo_font_extents (cr, &font_extents);

    status = cairo_status (cr);
    if (status)
	return cairo_test_status_from_status (ctx, status);

    if (! font_extents_equal (&font_extents, &ref_font_extents)) {
	cairo_test_log (ctx, "Error: %s: cairo_font_extents(); extents (%g, %g, %g, %g, %g)\n",
			comment,
		        font_extents.ascent, font_extents.descent,
			font_extents.height,
			font_extents.max_x_advance, font_extents.max_y_advance);
	return CAIRO_TEST_FAILURE;
    }

    return CAIRO_TEST_SUCCESS;
}

#if !defined(S_ISREG) && defined(S_IFMT) && defined(S_IFREG)
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#endif

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    const cairo_test_context_t *ctx = cairo_test_get_context (cr);
    FcPattern *pattern;
    cairo_font_face_t *font_face;
    cairo_font_extents_t font_extents;
    cairo_font_options_t *font_options;
    cairo_status_t status;
    char *filename;
    int face_count;
    struct stat stat_buf;

    xasprintf (&filename, "%s/%s", ctx->srcdir, FONT);

    if (stat (filename, &stat_buf) || ! S_ISREG (stat_buf.st_mode)) {
	cairo_test_log (ctx, "Error finding font: %s: file not found?\n", filename);
	return CAIRO_TEST_FAILURE;
    }

    pattern = FcFreeTypeQuery ((unsigned char *)filename, 0, NULL, &face_count);
    if (! pattern) {
	cairo_test_log (ctx, "FcFreeTypeQuery failed.\n");
	free (filename);
	return cairo_test_status_from_status (ctx, CAIRO_STATUS_NO_MEMORY);
    }

    font_face = cairo_ft_font_face_create_for_pattern (pattern);
    FcPatternDestroy (pattern);

    status = cairo_font_face_status (font_face);
    if (status) {
	cairo_test_log (ctx, "Error creating font face for %s: %s\n",
			filename,
			cairo_status_to_string (status));
	free (filename);
	return cairo_test_status_from_status (ctx, status);
    }

    free (filename);
    if (cairo_font_face_get_type (font_face) != CAIRO_FONT_TYPE_FT) {
	cairo_test_log (ctx, "Unexpected value from cairo_font_face_get_type: %d (expected %d)\n",
			cairo_font_face_get_type (font_face), CAIRO_FONT_TYPE_FT);
	cairo_font_face_destroy (font_face);
	return CAIRO_TEST_FAILURE;
    }

    cairo_set_font_face (cr, font_face);
    cairo_font_face_destroy (font_face);
    cairo_set_font_size (cr, 13);

    font_options = cairo_font_options_create ();

#define CHECK_FONT_EXTENTS(comment) do {\
    cairo_test_status_t test_status; \
    test_status = check_font_extents (ctx, cr, (comment)); \
    if (test_status != CAIRO_TEST_SUCCESS) { \
	cairo_font_options_destroy (font_options); \
	return test_status; \
    } \
} while (0)

    cairo_font_extents (cr, &font_extents);
    CHECK_FONT_EXTENTS ("default");

    cairo_font_options_set_hint_metrics (font_options, CAIRO_HINT_METRICS_ON);
    cairo_set_font_options (cr, font_options);

    CHECK_FONT_EXTENTS ("HINT_METRICS_ON");

    cairo_move_to (cr, 1, font_extents.ascent - 1);
    cairo_set_source_rgb (cr, 0.0, 0.0, 1.0); /* blue */

    cairo_font_options_set_hint_style (font_options, CAIRO_HINT_STYLE_NONE);
    cairo_set_font_options (cr, font_options);
    CHECK_FONT_EXTENTS ("HINT_METRICS_ON HINT_STYLE_NONE");
    cairo_show_text (cr, "the ");

    cairo_font_options_set_hint_style (font_options, CAIRO_HINT_STYLE_SLIGHT);
    cairo_set_font_options (cr, font_options);
    CHECK_FONT_EXTENTS ("HINT_METRICS_ON HINT_STYLE_SLIGHT");
    cairo_show_text (cr, "quick ");

    cairo_font_options_set_hint_style (font_options, CAIRO_HINT_STYLE_MEDIUM);
    cairo_set_font_options (cr, font_options);
    CHECK_FONT_EXTENTS ("HINT_METRICS_ON HINT_STYLE_MEDIUM");
    cairo_show_text (cr, "brown");

    cairo_font_options_set_hint_style (font_options, CAIRO_HINT_STYLE_FULL);
    cairo_set_font_options (cr, font_options);
    CHECK_FONT_EXTENTS ("HINT_METRICS_ON HINT_STYLE_FULL");
    cairo_show_text (cr, " fox");

    /* Switch from show_text to text_path/fill to exercise bug #7889 */
    cairo_text_path (cr, " jumps over a lazy dog");
    cairo_fill (cr);

    /* And test it rotated as well for the sake of bug #7888 */

    cairo_translate (cr, width, height);
    cairo_rotate (cr, M_PI);

    cairo_font_options_set_hint_style (font_options, CAIRO_HINT_STYLE_DEFAULT);
    cairo_font_options_set_hint_metrics (font_options, CAIRO_HINT_METRICS_OFF);
    cairo_set_font_options (cr, font_options);
    CHECK_FONT_EXTENTS ("HINT_METRICS_OFF");

    cairo_move_to (cr, 1, font_extents.height - font_extents.descent - 1);

    cairo_font_options_set_hint_style (font_options, CAIRO_HINT_STYLE_NONE);
    cairo_set_font_options (cr, font_options);
    CHECK_FONT_EXTENTS ("HINT_METRICS_OFF HINT_STYLE_NONE");
    cairo_show_text (cr, "the ");

    cairo_font_options_set_hint_style (font_options, CAIRO_HINT_STYLE_SLIGHT);
    cairo_set_font_options (cr, font_options);
    CHECK_FONT_EXTENTS ("HINT_METRICS_OFF HINT_STYLE_SLIGHT");
    cairo_show_text (cr, "quick");

    cairo_font_options_set_hint_style (font_options, CAIRO_HINT_STYLE_MEDIUM);
    cairo_set_font_options (cr, font_options);
    CHECK_FONT_EXTENTS ("HINT_METRICS_OFF HINT_STYLE_MEDIUM");
    cairo_show_text (cr, " brown");

    cairo_font_options_set_hint_style (font_options, CAIRO_HINT_STYLE_FULL);
    cairo_set_font_options (cr, font_options);
    CHECK_FONT_EXTENTS ("HINT_METRICS_OFF HINT_STYLE_FULL");
    cairo_show_text (cr, " fox");

    cairo_text_path (cr, " jumps over");
    cairo_text_path (cr, " a lazy dog");
    cairo_fill (cr);

    cairo_font_options_destroy (font_options);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (bitmap_font,
	    "Test drawing with a font consisting only of bitmaps"
	    "\nThe PDF and PS backends embed a slightly distorted font for the rotated case.",
	    "text", /* keywords */
	    "ft", /* requirements */
	    246 + 1, 2 * TEXT_SIZE,
	    NULL, draw)
