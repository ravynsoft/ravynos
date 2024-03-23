/*
 * Copyright © 2005 Red Hat, Inc.
 * Copyright © 2011 Intel Corporation
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
 *	Carl D. Worth <cworth@cworth.org>
 *	Chris Wilson <chris@chris-wilson.co.uk>
 */

#include "cairo-test.h"

#define TEXT_SIZE 12
#define SIZE 60 /* needs to be big to check large area effects (dithering) */
#define PAD 2

#define TT_SIZE 100
#define TT_PAD 5
#define TT_FONT_SIZE 32.0

#define GENERATE_REF 0

static uint32_t data[16] = {
    0xffffffff, 0xffffffff,		0xffff0000, 0xffff0000,
    0xffffffff, 0xffffffff,		0xffff0000, 0xffff0000,

    0xff00ff00, 0xff00ff00,		0xff0000ff, 0xff0000ff,
    0xff00ff00, 0xff00ff00,		0xff0000ff, 0xff0000ff
};

static const char *png_filename = "romedalen.png";

static cairo_t *
paint (cairo_t *cr)
{
    cairo_set_source_rgb (cr, 0, 0, 1);
    cairo_paint (cr);

    cairo_translate (cr, 2, 2);
    cairo_scale (cr, 0.5, 0.5);

    cairo_set_source_rgb (cr, 1, 0, 0);
    cairo_paint (cr);

    return cr;
}

static cairo_t *
paint_alpha (cairo_t *cr)
{
    cairo_surface_t *surface;

    surface = cairo_image_surface_create_for_data ((unsigned char *) data,
						   CAIRO_FORMAT_RGB24, 4, 4, 16);

    cairo_test_paint_checkered (cr);

    cairo_scale (cr, 4, 4);

    cairo_set_source_surface (cr, surface, 2 , 2);
    cairo_pattern_set_filter (cairo_get_source (cr), CAIRO_FILTER_NEAREST);
    cairo_paint_with_alpha (cr, 0.5);

    cairo_surface_finish (surface); /* data will go out of scope */
    cairo_surface_destroy (surface);

    return cr;
}

static cairo_t *
paint_alpha_solid_clip (cairo_t *cr)
{
    cairo_test_paint_checkered (cr);

    cairo_rectangle (cr, 2.5, 2.5, 27, 27);
    cairo_clip (cr);

    cairo_set_source_rgb (cr, 1., 0.,0.);
    cairo_paint_with_alpha (cr, 0.5);

    return cr;
}

static cairo_t *
paint_alpha_clip (cairo_t *cr)
{
    cairo_surface_t *surface;

    surface = cairo_image_surface_create_for_data ((unsigned char *) data,
						   CAIRO_FORMAT_RGB24, 4, 4, 16);

    cairo_test_paint_checkered (cr);

    cairo_rectangle (cr, 10.5, 10.5, 11, 11);
    cairo_clip (cr);

    cairo_scale (cr, 4, 4);

    cairo_set_source_surface (cr, surface, 2 , 2);
    cairo_pattern_set_filter (cairo_get_source (cr), CAIRO_FILTER_NEAREST);
    cairo_paint_with_alpha (cr, 0.5);

    cairo_surface_finish (surface); /* data will go out of scope */
    cairo_surface_destroy (surface);

    return cr;
}

static cairo_t *
paint_alpha_clip_mask (cairo_t *cr)
{
    cairo_surface_t *surface;

    surface = cairo_image_surface_create_for_data ((unsigned char *) data,
						   CAIRO_FORMAT_RGB24, 4, 4, 16);

    cairo_test_paint_checkered (cr);

    cairo_move_to (cr, 16, 5);
    cairo_line_to (cr, 5, 16);
    cairo_line_to (cr, 16, 27);
    cairo_line_to (cr, 27, 16);
    cairo_clip (cr);

    cairo_scale (cr, 4, 4);

    cairo_set_source_surface (cr, surface, 2 , 2);
    cairo_pattern_set_filter (cairo_get_source (cr), CAIRO_FILTER_NEAREST);
    cairo_paint_with_alpha (cr, 0.5);

    cairo_surface_finish (surface); /* data will go out of scope */
    cairo_surface_destroy (surface);

    return cr;
}

static cairo_t *
select_font_face (cairo_t *cr)
{
    /* We draw in the default black, so paint white first. */
    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0); /* white */
    cairo_paint (cr);

    cairo_set_source_rgb (cr, 0, 0, 0); /* black */

    cairo_set_font_size (cr, TEXT_SIZE);
    cairo_move_to (cr, 0, TEXT_SIZE);

    cairo_select_font_face (cr, CAIRO_TEST_FONT_FAMILY " Serif",
			    CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);
    cairo_show_text (cr, "i-am-serif");

    cairo_select_font_face (cr, CAIRO_TEST_FONT_FAMILY " Sans",
			    CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);
    cairo_show_text (cr, " i-am-sans");

    cairo_select_font_face (cr, CAIRO_TEST_FONT_FAMILY " Sans Mono",
			    CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);
    cairo_show_text (cr, " i-am-mono");

    return cr;
}

static cairo_t *
fill_alpha (cairo_t *cr)
{
    const double alpha = 1./3;
    int n;

    /* flatten to white */
    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    /* square */
    cairo_rectangle (cr, PAD, PAD, SIZE, SIZE);
    cairo_set_source_rgba (cr, 1, 0, 0, alpha);
    cairo_fill (cr);

    /* circle */
    cairo_translate (cr, SIZE + 2 * PAD, 0);
    cairo_arc (cr, PAD + SIZE / 2., PAD + SIZE / 2., SIZE / 2., 0, 2 * M_PI);
    cairo_set_source_rgba (cr, 0, 1, 0, alpha);
    cairo_fill (cr);

    /* triangle */
    cairo_translate (cr, 0, SIZE + 2 * PAD);
    cairo_move_to (cr, PAD + SIZE / 2, PAD);
    cairo_line_to (cr, PAD + SIZE, PAD + SIZE);
    cairo_line_to (cr, PAD, PAD + SIZE);
    cairo_set_source_rgba (cr, 0, 0, 1, alpha);
    cairo_fill (cr);

    /* star */
    cairo_translate (cr, -(SIZE + 2 * PAD) + SIZE/2., SIZE/2.);
    for (n = 0; n < 5; n++) {
	cairo_line_to (cr,
		       SIZE/2 * cos (2*n * 2*M_PI / 10),
		       SIZE/2 * sin (2*n * 2*M_PI / 10));

	cairo_line_to (cr,
		       SIZE/4 * cos ((2*n+1)*2*M_PI / 10),
		       SIZE/4 * sin ((2*n+1)*2*M_PI / 10));
    }
    cairo_set_source_rgba (cr, 0, 0, 0, alpha);
    cairo_fill (cr);

    return cr;
}

static cairo_t *
self_intersecting (cairo_t *cr)
{
    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    cairo_translate (cr, 1.0, 1.0);

    cairo_set_source_rgb (cr, 1, 0, 0); /* red */

    /* First draw the desired shape with a fill */
    cairo_rectangle (cr, 0.5, 0.5,  4.0, 4.0);
    cairo_rectangle (cr, 3.5, 3.5,  4.0, 4.0);
    cairo_rectangle (cr, 3.5, 1.5, -2.0, 2.0);
    cairo_rectangle (cr, 6.5, 4.5, -2.0, 2.0);

    cairo_fill (cr);

    /* Then try the same thing with a stroke */
    cairo_translate (cr, 0, 10);
    cairo_move_to (cr, 1.0, 1.0);
    cairo_rel_line_to (cr,  3.0,  0.0);
    cairo_rel_line_to (cr,  0.0,  6.0);
    cairo_rel_line_to (cr,  3.0,  0.0);
    cairo_rel_line_to (cr,  0.0, -3.0);
    cairo_rel_line_to (cr, -6.0,  0.0);
    cairo_close_path (cr);

    cairo_set_line_width (cr, 1.0);
    cairo_stroke (cr);

    return cr;
}

static void
draw_text_transform (cairo_t *cr)
{
    cairo_matrix_t tm;

    /* skew */
    cairo_matrix_init (&tm, 1, 0,
                       -0.25, 1,
                       0, 0);
    cairo_matrix_scale (&tm, TT_FONT_SIZE, TT_FONT_SIZE);
    cairo_set_font_matrix (cr, &tm);

    cairo_new_path (cr);
    cairo_move_to (cr, 50, TT_SIZE-TT_PAD);
    cairo_show_text (cr, "A");

    /* rotate and scale */
    cairo_matrix_init_rotate (&tm, M_PI / 2);
    cairo_matrix_scale (&tm, TT_FONT_SIZE, TT_FONT_SIZE * 2.0);
    cairo_set_font_matrix (cr, &tm);

    cairo_new_path (cr);
    cairo_move_to (cr, TT_PAD, TT_PAD + 25);
    cairo_show_text (cr, "A");

    cairo_matrix_init_rotate (&tm, M_PI / 2);
    cairo_matrix_scale (&tm, TT_FONT_SIZE * 2.0, TT_FONT_SIZE);
    cairo_set_font_matrix (cr, &tm);

    cairo_new_path (cr);
    cairo_move_to (cr, TT_PAD, TT_PAD + 50);
    cairo_show_text (cr, "A");
}

static cairo_t *
text_transform (cairo_t *cr)
{
    const cairo_test_context_t *ctx = cairo_test_get_context (cr);
    cairo_pattern_t *pattern;

    cairo_set_source_rgb (cr, 1., 1., 1.);
    cairo_paint (cr);

    cairo_set_source_rgb (cr, 0., 0., 0.);

    cairo_select_font_face (cr, CAIRO_TEST_FONT_FAMILY " Sans",
			    CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);

    draw_text_transform (cr);

    cairo_translate (cr, TT_SIZE, TT_SIZE);
    cairo_rotate (cr, M_PI);

    pattern = cairo_test_create_pattern_from_png (ctx, png_filename);
    cairo_pattern_set_extend (pattern, CAIRO_EXTEND_REPEAT);
    cairo_set_source (cr, pattern);
    cairo_pattern_destroy (pattern);

    draw_text_transform (cr);

    return cr;
}

/* And here begins the recording and replaying... */

static cairo_t *
record_create (cairo_t *target)
{
    cairo_surface_t *surface;
    cairo_t *cr;

    surface = cairo_recording_surface_create (cairo_surface_get_content (cairo_get_target (target)), NULL);
    cr = cairo_test_create (surface, cairo_test_get_context (target));
    cairo_surface_destroy (surface);

    return cr;
}

static cairo_surface_t *
record_get (cairo_t *target)
{
    cairo_surface_t *surface;

    surface = cairo_surface_reference (cairo_get_target (target));
    cairo_destroy (target);

    return surface;
}

static cairo_test_status_t
record_replay (cairo_t *cr, cairo_t *(*func)(cairo_t *), int width, int height)
{
    cairo_surface_t *surface;
    int x, y;

#if GENERATE_REF
    cairo_scale (cr, 2, 2);
    func(cr);
#else
    surface = record_get (func (record_create (cr)));

    cairo_scale (cr, 2, 2);
    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_surface (cr, surface, 0, 0);
    cairo_surface_destroy (surface);
    cairo_pattern_set_extend (cairo_get_source (cr), CAIRO_EXTEND_NONE);

    for (y = 0; y < height; y += 2) {
	for (x = 0; x < width; x += 2) {
	    cairo_rectangle (cr, x, y, 2, 2);
	    cairo_clip (cr);
	    cairo_paint (cr);
	    cairo_reset_clip (cr);
	}
    }
#endif

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
record_paint (cairo_t *cr, int width, int height)
{
    return record_replay (cr, paint, width, height);
}

static cairo_test_status_t
record_paint_alpha (cairo_t *cr, int width, int height)
{
    return record_replay (cr, paint_alpha, width, height);
}

static cairo_test_status_t
record_paint_alpha_solid_clip (cairo_t *cr, int width, int height)
{
    return record_replay (cr, paint_alpha_solid_clip, width, height);
}

static cairo_test_status_t
record_paint_alpha_clip (cairo_t *cr, int width, int height)
{
    return record_replay (cr, paint_alpha_clip, width, height);
}

static cairo_test_status_t
record_paint_alpha_clip_mask (cairo_t *cr, int width, int height)
{
    return record_replay (cr, paint_alpha_clip_mask, width, height);
}

static cairo_test_status_t
record_fill_alpha (cairo_t *cr, int width, int height)
{
    return record_replay (cr, fill_alpha, width, height);
}

static cairo_test_status_t
record_self_intersecting (cairo_t *cr, int width, int height)
{
    return record_replay (cr, self_intersecting, width, height);
}

static cairo_test_status_t
record_select_font_face (cairo_t *cr, int width, int height)
{
    return record_replay (cr, select_font_face, width, height);
}

static cairo_test_status_t
record_text_transform (cairo_t *cr, int width, int height)
{
    return record_replay (cr, text_transform, width, height);
}

CAIRO_TEST (record2x_paint,
	    "Test replayed calls to cairo_paint",
	    "paint,record", /* keywords */
	    NULL, /* requirements */
	    2*8, 2*8,
	    NULL, record_paint)
CAIRO_TEST (record2x_paint_alpha,
	    "Simple test of cairo_paint_with_alpha",
	    "record, paint, alpha", /* keywords */
	    NULL, /* requirements */
	    2*32, 2*32,
	    NULL, record_paint_alpha)
CAIRO_TEST (record2x_paint_alpha_solid_clip,
	    "Simple test of cairo_paint_with_alpha+unaligned clip",
	    "record, paint, alpha, clip", /* keywords */
	    NULL, /* requirements */
	    2*32, 2*32,
	    NULL, record_paint_alpha_solid_clip)
CAIRO_TEST (record2x_paint_alpha_clip,
	    "Simple test of cairo_paint_with_alpha+unaligned clip",
	    "record, paint, alpha, clip", /* keywords */
	    NULL, /* requirements */
	    2*32, 2*32,
	    NULL, record_paint_alpha_clip)
CAIRO_TEST (record2x_paint_alpha_clip_mask,
	    "Simple test of cairo_paint_with_alpha+triangular clip",
	    "record, paint, alpha, clip", /* keywords */
	    NULL, /* requirements */
	    2*32, 2*32,
	    NULL, record_paint_alpha_clip_mask)
CAIRO_TEST (record2x_fill_alpha,
	    "Tests using set_rgba();fill()",
	    "record,fill, alpha", /* keywords */
	    NULL, /* requirements */
	    2*(2*SIZE + 4*PAD), 2*(2*SIZE + 4*PAD),
	    NULL, record_fill_alpha)
CAIRO_TEST (record2x_select_font_face,
	    "Tests using cairo_select_font_face to draw text in different faces",
	    "record, font", /* keywords */
	    NULL, /* requirements */
	    2*192, 2*(TEXT_SIZE + 4),
	    NULL, record_select_font_face)
CAIRO_TEST (record2x_self_intersecting,
	    "Test strokes of self-intersecting paths",
	    "record, stroke, trap", /* keywords */
	    NULL, /* requirements */
	    2*10, 2*20,
	    NULL, record_self_intersecting)
CAIRO_TEST (record2x_text_transform,
	    "Test various applications of the font matrix",
	    "record, text, transform", /* keywords */
	    NULL, /* requirements */
	    2*TT_SIZE, 2*TT_SIZE,
	    NULL, record_text_transform)
