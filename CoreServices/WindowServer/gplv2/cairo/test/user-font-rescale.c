/*
 * Copyright © 2008 Jeff Muizelaar
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Jeff Muizelaar not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. Jeff Muizelaar makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * JEFF MUIZELAAR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL JEFF MUIZELAAR BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Contributor(s):
 *	Jeff Muizelaar <jeff@infidigm.net>
 *	Kristian Høgsberg <krh@redhat.com>
 *	Behdad Esfahbod <behdad@behdad.org>
 */

#include "cairo-test.h"

#include <math.h>

#define BORDER 10
#define TEXT_SIZE 32
#define WIDTH  (TEXT_SIZE * 13.75 + 2*BORDER)
#define HEIGHT ((TEXT_SIZE + 2*BORDER)*3 + BORDER)
#define TEXT   "test of rescaled glyphs"

static const cairo_user_data_key_t rescale_font_closure_key;

struct rescaled_font {
    cairo_font_face_t *substitute_font;
    cairo_scaled_font_t *measuring_font;
    unsigned long glyph_count;
    unsigned long start;
    double *desired_width;
    double *rescale_factor;
};

static cairo_status_t
test_scaled_font_render_glyph (cairo_scaled_font_t  *scaled_font,
			       unsigned long         glyph,
			       cairo_t              *cr,
			       cairo_text_extents_t *metrics)
{
    cairo_font_face_t *user_font;
    struct rescaled_font *r;
    cairo_glyph_t cairo_glyph;

    cairo_glyph.index = glyph;
    cairo_glyph.x = 0;
    cairo_glyph.y = 0;

    user_font = cairo_scaled_font_get_font_face (scaled_font);
    r = cairo_font_face_get_user_data (user_font, &rescale_font_closure_key);
    cairo_set_font_face (cr, r->substitute_font);

    if (glyph - r->start < r->glyph_count) {
	cairo_matrix_t matrix;

	if (isnan (r->rescale_factor[glyph - r->start])) {
	    double desired_width;
	    double actual_width;
	    cairo_text_extents_t extents;

	    /* measure the glyph and compute the necessary rescaling factor */
	    cairo_scaled_font_glyph_extents (r->measuring_font,
					     &cairo_glyph, 1,
					     &extents);

	    desired_width = r->desired_width[glyph - r->start];
	    actual_width = extents.x_advance;

	    r->rescale_factor[glyph - r->start] = desired_width / actual_width;
	}

	/* scale the font so that the glyph width matches the desired width */
	cairo_get_font_matrix (cr, &matrix);
	cairo_matrix_scale (&matrix, r->rescale_factor[glyph - r->start], 1.);
	cairo_set_font_matrix (cr, &matrix);
    }

    cairo_show_glyphs (cr, &cairo_glyph, 1);
    cairo_glyph_extents (cr, &cairo_glyph, 1, metrics);

    return CAIRO_STATUS_SUCCESS;
}

static void
unichar_to_utf8 (uint32_t ucs4, char utf8[7])
{
    int i, charlen, first;

    if (ucs4 < 0x80) {
	first = 0;
	charlen = 1;
    } else if (ucs4 < 0x800) {
	first = 0xc0;
	charlen = 2;
    } else if (ucs4 < 0x10000) {
	first = 0xe0;
	charlen = 3;
    } else if (ucs4 < 0x200000) {
	first = 0xf0;
	charlen = 4;
    } else if (ucs4 < 0x4000000) {
	first = 0xf8;
	charlen = 5;
    } else {
	first = 0xfc;
	charlen = 6;
    }

    for (i = charlen - 1; i > 0; --i) {
	utf8[i] = (ucs4 & 0x3f) | 0x80;
	ucs4 >>= 6;
    }
    utf8[0] = ucs4 | first;
    utf8[charlen] = '\0';
}

static cairo_status_t
test_scaled_font_unicode_to_glyph (cairo_scaled_font_t *scaled_font,
				   unsigned long        unicode,
				   unsigned long       *glyph_index)
{
    cairo_font_face_t *user_font;
    struct rescaled_font *r;
    int num_glyphs;
    cairo_glyph_t *glyphs = NULL;
    cairo_status_t status;
    char utf8[7];

    user_font = cairo_scaled_font_get_font_face (scaled_font);

    unichar_to_utf8 (unicode, utf8);
    r = cairo_font_face_get_user_data (user_font, &rescale_font_closure_key);
    status  = cairo_scaled_font_text_to_glyphs (r->measuring_font, 0, 0,
						utf8, -1,
						&glyphs, &num_glyphs,
						NULL, NULL, NULL);
    if (status)
	return status;

    *glyph_index = glyphs[0].index;

    cairo_glyph_free (glyphs);
    return CAIRO_STATUS_SUCCESS;
}

static void rescale_font_closure_destroy (void *data)
{
    struct rescaled_font *r = data;

    cairo_font_face_destroy (r->substitute_font);
    cairo_scaled_font_destroy (r->measuring_font);
    free (r->desired_width);
    free (r->rescale_factor);
    free (r);
}

static cairo_status_t
create_rescaled_font (cairo_font_face_t *substitute_font,
		      int glyph_start,
		      int glyph_count,
		      double *desired_width,
		      cairo_font_face_t **out)
{
    cairo_font_face_t *user_font_face;
    struct rescaled_font *r;
    cairo_font_options_t *options;
    cairo_status_t status;
    cairo_matrix_t m;
    unsigned long i;

    user_font_face = cairo_user_font_face_create ();
    cairo_user_font_face_set_render_glyph_func (user_font_face, test_scaled_font_render_glyph);
    cairo_user_font_face_set_unicode_to_glyph_func (user_font_face, test_scaled_font_unicode_to_glyph);

    r = xmalloc (sizeof (struct rescaled_font));
    r->substitute_font = cairo_font_face_reference (substitute_font);

    /* we don't want any hinting when doing the measuring */
    options = cairo_font_options_create ();
    cairo_font_options_set_hint_style (options, CAIRO_HINT_STYLE_NONE);
    cairo_font_options_set_hint_metrics (options, CAIRO_HINT_METRICS_OFF);

    cairo_matrix_init_identity (&m);

    r->measuring_font = cairo_scaled_font_create (r->substitute_font,
						  &m, &m,
						  options);
    cairo_font_options_destroy (options);


    r->start = glyph_start;
    r->glyph_count = glyph_count;
    r->desired_width = xcalloc (sizeof (double), r->glyph_count);
    r->rescale_factor = xcalloc (sizeof (double), r->glyph_count);

    for (i = 0; i < r->glyph_count; i++) {
	r->desired_width[i] = desired_width[i];
	/* use NaN to specify unset */
	r->rescale_factor[i] = cairo_test_NaN ();
    }

    status = cairo_font_face_set_user_data (user_font_face,
					    &rescale_font_closure_key,
					    r, rescale_font_closure_destroy);
    if (status) {
	rescale_font_closure_destroy (r);
	cairo_font_face_destroy (user_font_face);
	return status;
    }

    *out = user_font_face;
    return CAIRO_STATUS_SUCCESS;
}

static cairo_status_t
get_user_font_face (cairo_font_face_t *substitute_font,
		    const char *text,
		    cairo_font_face_t *old,
		    cairo_font_face_t **out)
{
    cairo_font_options_t *options;
    cairo_matrix_t m;
    cairo_scaled_font_t *measure;
    int i;
    double *widths;
    int count;
    int num_glyphs;
    unsigned long min_index, max_index;
    cairo_status_t status;

    cairo_glyph_t *glyphs = NULL;

    /* we don't want any hinting when doing the measuring */
    options = cairo_font_options_create ();
    cairo_font_options_set_hint_style (options, CAIRO_HINT_STYLE_NONE);
    cairo_font_options_set_hint_metrics (options, CAIRO_HINT_METRICS_OFF);

    cairo_matrix_init_identity (&m);
    measure = cairo_scaled_font_create (old, &m, &m, options);

    status = cairo_scaled_font_text_to_glyphs (measure, 0, 0,
					       text, -1,
					       &glyphs, &num_glyphs,
					       NULL, NULL, NULL);
    cairo_font_options_destroy (options);

    if (status) {
	cairo_scaled_font_destroy (measure);
	return status;
    }

    /* find the glyph range the text covers */
    max_index = glyphs[0].index;
    min_index = glyphs[0].index;
    for (i=0; i<num_glyphs; i++) {
	if (glyphs[i].index < min_index)
	    min_index = glyphs[i].index;
	if (glyphs[i].index > max_index)
	    max_index = glyphs[i].index;
    }

    count = max_index - min_index + 1;
    widths = xcalloc (sizeof (double), count);
    /* measure all of the necessary glyphs individually */
    for (i=0; i<num_glyphs; i++) {
	cairo_text_extents_t extents;
	cairo_scaled_font_glyph_extents (measure, &glyphs[i], 1, &extents);
	widths[glyphs[i].index - min_index] = extents.x_advance;
    }

    status = cairo_scaled_font_status (measure);
    cairo_scaled_font_destroy (measure);
    cairo_glyph_free (glyphs);

    if (status == CAIRO_STATUS_SUCCESS) {
	status = create_rescaled_font (substitute_font,
				       min_index, count, widths,
				       out);
    }

    free (widths);
    return status;
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    cairo_font_extents_t font_extents;
    cairo_text_extents_t extents;
    cairo_font_face_t *rescaled;
    cairo_font_face_t *old;
    cairo_font_face_t *substitute;
    const char text[] = TEXT;
    cairo_status_t status;

    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    cairo_select_font_face (cr,
			    CAIRO_TEST_FONT_FAMILY " Sans",
			    CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);

    cairo_set_font_size (cr, TEXT_SIZE);

    cairo_font_extents (cr, &font_extents);
    cairo_text_extents (cr, text, &extents);

    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_move_to (cr, BORDER, BORDER + font_extents.ascent);
    cairo_show_text (cr, text);

    /* same text in 'mono' with widths that match the 'sans' version */
    old = cairo_font_face_reference (cairo_get_font_face (cr));
    cairo_select_font_face (cr,
			    CAIRO_TEST_FONT_FAMILY " Sans Mono",
			    CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);
    substitute = cairo_get_font_face (cr);

    status = get_user_font_face (substitute, text, old, &rescaled);
    cairo_font_face_destroy (old);
    if (status) {
	return cairo_test_status_from_status (cairo_test_get_context (cr),
					      status);
    }

    cairo_set_font_face (cr, rescaled);
    cairo_font_face_destroy (rescaled);

    cairo_set_source_rgb (cr, 0, 0, 1);
    cairo_move_to (cr, BORDER, BORDER + font_extents.height + 2*BORDER + font_extents.ascent);
    cairo_show_text (cr, text);

    /* mono text */
    cairo_select_font_face (cr,
			    CAIRO_TEST_FONT_FAMILY " Sans Mono",
			    CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);

    cairo_set_source_rgb (cr, 0, 0, 1);
    cairo_move_to (cr, BORDER, BORDER + 2*font_extents.height + 4*BORDER + font_extents.ascent);
    cairo_show_text (cr, text);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (user_font_rescale,
	    "Tests drawing text with user defined widths",
	    "user-font, font", /* keywords */
	    NULL, /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw)
