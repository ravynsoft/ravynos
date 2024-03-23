/*
 * Copyright © 2010 Red Hat Inc.
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
 * Author: Benjamin Otte <otte@redhat.com>
 */

/*
 * WHAT THIS TEST DOES
 *
 * This test tests that for all public APIs Cairo behaves correct, consistent
 * and most of all doesn't crash. It does this by calling all APIs that take
 * surfaces or contexts and calling them on specially prepared arguments that
 * should fail when called on this function.
 *
 * ADDING NEW FUNCTIONS
 *
 * You need (for adding the function cairo_surface_foo):
 * 1) A surface_test_func_t named test_cairo_surface_foo that gets passed the
 *    prepared surface and has the job of calling the function and checking
 *    the return value (if one exists) for correctness. The top of this file
 *    contains all these shim functions.
 * 2) Knowledge if the function behaves like a setter or like a getter. A 
 *    setter should set an error status on the surface, a getter does not
 *    modify the function.
 * 3) Knowledge if the function only works for a specific surface type and for
 *    which one.
 * 4) An entry in the tests array using the TEST() macro. It takes as arguments:
 *    - The function name
 *    - TRUE if the function modifies the surface, FALSE otherwise
 *    - the surface type for which the function is valid or -1 if it is valid
 *      for all surface types.
 *
 * FIXING FAILURES
 *
 * The test will dump failures notices into the api-special-cases.log file (when 
 * it doesn't crash). These should be pretty self-explanatory. Usually it is 
 * enough to just add a new check to the function it complained about.
 */

#include "config.h"

#include <assert.h>
#include <limits.h>

#include "cairo-test.h"

#if CAIRO_HAS_PDF_SURFACE
#include <cairo-pdf.h>
#endif
#if CAIRO_HAS_PS_SURFACE
#include <cairo-ps.h>
#endif
#if CAIRO_HAS_QUARTZ_SURFACE
#define Cursor QuartzCursor
#include <cairo-quartz.h>
#undef Cursor
#endif
#if CAIRO_HAS_SVG_SURFACE
#include <cairo-svg.h>
#endif
#if CAIRO_HAS_TEE_SURFACE
#include <cairo-tee.h>
#endif
#if CAIRO_HAS_XCB_SURFACE
#include <cairo-xcb.h>
#endif
#if CAIRO_HAS_XLIB_SURFACE
#define Cursor XCursor
#include <cairo-xlib.h>
#undef Cursor
#endif

#define surface_has_type(surface,type) (cairo_surface_get_type (surface) == (type))

typedef cairo_test_status_t (* surface_test_func_t) (cairo_surface_t *surface);
typedef cairo_test_status_t (* context_test_func_t) (cairo_t *cr);

static cairo_test_status_t
test_cairo_reference (cairo_t *cr)
{
    cairo_destroy (cairo_reference (cr));

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_get_reference_count (cairo_t *cr)
{
    unsigned int refcount = cairo_get_reference_count (cr);
    if (refcount > 0)
        return CAIRO_TEST_SUCCESS;
    /* inert error context have a refcount of 0 */
    return cairo_status (cr) ? CAIRO_TEST_SUCCESS : CAIRO_TEST_ERROR;
}

static cairo_test_status_t
test_cairo_set_user_data (cairo_t *cr)
{
    static cairo_user_data_key_t key;
    cairo_status_t status;

    status = cairo_set_user_data (cr, &key, &key, NULL);
    if (status == CAIRO_STATUS_NO_MEMORY)
        return CAIRO_TEST_NO_MEMORY;
    else if (status)
        return CAIRO_TEST_SUCCESS;

    if (cairo_get_user_data (cr, &key) != &key)
        return CAIRO_TEST_ERROR;

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_save (cairo_t *cr)
{
    cairo_save (cr);
    cairo_restore (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_push_group (cairo_t *cr)
{
    cairo_pattern_t *pattern;
    cairo_status_t status;

    cairo_push_group (cr);
    pattern = cairo_pop_group (cr);
    status = cairo_pattern_status (pattern);
    cairo_pattern_destroy (pattern);

    return status == CAIRO_STATUS_SUCCESS || status == cairo_status (cr) ? CAIRO_TEST_SUCCESS : CAIRO_TEST_ERROR;
}

static cairo_test_status_t
test_cairo_push_group_with_content (cairo_t *cr)
{
    cairo_push_group_with_content (cr, CAIRO_CONTENT_COLOR_ALPHA);
    cairo_pop_group_to_source (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_set_operator (cairo_t *cr)
{
    cairo_set_operator (cr, CAIRO_OPERATOR_OVER);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_set_source (cairo_t *cr)
{
    cairo_pattern_t *source = cairo_pattern_create_rgb (0, 0, 0);
    cairo_set_source (cr, source);
    cairo_pattern_destroy (source);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_set_source_rgb (cairo_t *cr)
{
    cairo_set_source_rgb (cr, 0, 0, 0);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_set_source_rgba (cairo_t *cr)
{
    cairo_set_source_rgba (cr, 0, 0, 0, 1);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_set_source_surface (cairo_t *cr)
{
    cairo_surface_t *surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 1, 1);
    cairo_set_source_surface (cr, surface, 0, 0);
    cairo_surface_destroy (surface);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_set_tolerance (cairo_t *cr)
{
    cairo_set_tolerance (cr, 42);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_set_antialias (cairo_t *cr)
{
    cairo_set_antialias (cr, CAIRO_ANTIALIAS_BEST);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_set_fill_rule (cairo_t *cr)
{
    cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_set_line_width (cairo_t *cr)
{
    cairo_set_line_width (cr, 42);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_set_line_cap (cairo_t *cr)
{
    cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_set_line_join (cairo_t *cr)
{
    cairo_set_line_join (cr, CAIRO_LINE_JOIN_BEVEL);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_set_dash (cairo_t *cr)
{
    cairo_set_dash (cr, NULL, 0, 0);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_set_miter_limit (cairo_t *cr)
{
    cairo_set_miter_limit (cr, 2);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_translate (cairo_t *cr)
{
    cairo_translate (cr, 2, 2);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_scale (cairo_t *cr)
{
    cairo_scale (cr, 2, 2);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_rotate (cairo_t *cr)
{
    cairo_rotate (cr, 2);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_transform (cairo_t *cr)
{
    cairo_matrix_t matrix;

    cairo_matrix_init_translate (&matrix, 1, 1);
    cairo_transform (cr, &matrix);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_set_matrix (cairo_t *cr)
{
    cairo_matrix_t matrix;

    cairo_matrix_init_translate (&matrix, 1, 1);
    cairo_set_matrix (cr, &matrix);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_identity_matrix (cairo_t *cr)
{
    cairo_identity_matrix (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_user_to_device (cairo_t *cr)
{
    double x = 42, y = 42;

    cairo_user_to_device (cr, &x, &y);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_user_to_device_distance (cairo_t *cr)
{
    double x = 42, y = 42;

    cairo_user_to_device_distance (cr, &x, &y);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_device_to_user (cairo_t *cr)
{
    double x = 42, y = 42;

    cairo_device_to_user (cr, &x, &y);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_device_to_user_distance (cairo_t *cr)
{
    double x = 42, y = 42;

    cairo_device_to_user_distance (cr, &x, &y);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_new_path (cairo_t *cr)
{
    cairo_new_path (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_move_to (cairo_t *cr)
{
    cairo_move_to (cr, 2, 2);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_new_sub_path (cairo_t *cr)
{
    cairo_new_sub_path (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_line_to (cairo_t *cr)
{
    cairo_line_to (cr, 2, 2);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_curve_to (cairo_t *cr)
{
    cairo_curve_to (cr, 2, 2, 3, 3, 4, 4);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_arc (cairo_t *cr)
{
    cairo_arc (cr, 2, 2, 3, 0, 2 * M_PI);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_arc_negative (cairo_t *cr)
{
    cairo_arc_negative (cr, 2, 2, 3, 0, 2 * M_PI);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_rel_move_to (cairo_t *cr)
{
    cairo_rel_move_to (cr, 2, 2);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_rel_line_to (cairo_t *cr)
{
    cairo_rel_line_to (cr, 2, 2);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_rel_curve_to (cairo_t *cr)
{
    cairo_rel_curve_to (cr, 2, 2, 3, 3, 4, 4);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_rectangle (cairo_t *cr)
{
    cairo_rectangle (cr, 2, 2, 3, 3);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_close_path (cairo_t *cr)
{
    cairo_close_path (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_path_extents (cairo_t *cr)
{
    double x1, y1, x2, y2;
    cairo_path_extents (cr, &x1, &y1, &x2, &y2);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_paint (cairo_t *cr)
{
    cairo_paint (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_paint_with_alpha (cairo_t *cr)
{
    cairo_paint_with_alpha (cr, 0.5);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_mask (cairo_t *cr)
{
    cairo_pattern_t *pattern;

    pattern = cairo_pattern_create_rgb (0.5, 0.5, 0.5);
    cairo_mask (cr, pattern);

    cairo_pattern_destroy (pattern);
    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_mask_surface (cairo_t *cr)
{
    cairo_surface_t *surface;

    surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 1, 1);
    cairo_mask_surface (cr, surface, 0, 0);

    cairo_surface_destroy (surface);
    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_stroke (cairo_t *cr)
{
    cairo_stroke (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_stroke_preserve (cairo_t *cr)
{
    cairo_stroke_preserve (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_fill (cairo_t *cr)
{
    cairo_fill (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_fill_preserve (cairo_t *cr)
{
    cairo_fill_preserve (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_copy_page (cairo_t *cr)
{
    cairo_copy_page (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_show_page (cairo_t *cr)
{
    cairo_show_page (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_in_stroke (cairo_t *cr)
{
    cairo_in_stroke (cr, 1, 1);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_in_fill (cairo_t *cr)
{
    cairo_in_fill (cr, 1, 1);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_in_clip (cairo_t *cr)
{
    cairo_in_clip (cr, 1, 1);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_stroke_extents (cairo_t *cr)
{
    double x1, y1, x2, y2;
    cairo_stroke_extents (cr, &x1, &y1, &x2, &y2);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_fill_extents (cairo_t *cr)
{
    double x1, y1, x2, y2;
    cairo_fill_extents (cr, &x1, &y1, &x2, &y2);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_reset_clip (cairo_t *cr)
{
    cairo_reset_clip (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_clip (cairo_t *cr)
{
    cairo_clip (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_clip_preserve (cairo_t *cr)
{
    cairo_clip_preserve (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_clip_extents (cairo_t *cr)
{
    double x1, y1, x2, y2;
    cairo_clip_extents (cr, &x1, &y1, &x2, &y2);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_copy_clip_rectangle_list (cairo_t *cr)
{
    cairo_rectangle_list_destroy (cairo_copy_clip_rectangle_list (cr));

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_select_font_face (cairo_t *cr)
{
    cairo_select_font_face (cr, "Arial", CAIRO_FONT_SLANT_ITALIC, CAIRO_FONT_WEIGHT_BOLD);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_set_font_size (cairo_t *cr)
{
    cairo_set_font_size (cr, 42);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_set_font_matrix (cairo_t *cr)
{
    cairo_matrix_t matrix;

    cairo_matrix_init_translate (&matrix, 1, 1);
    cairo_set_font_matrix (cr, &matrix);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_get_font_matrix (cairo_t *cr)
{
    cairo_matrix_t matrix;

    cairo_get_font_matrix (cr, &matrix);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_set_font_options (cairo_t *cr)
{
    cairo_font_options_t *opt = cairo_font_options_create ();
    cairo_set_font_options (cr, opt);
    cairo_font_options_destroy (opt);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_get_font_options (cairo_t *cr)
{
    cairo_font_options_t *opt = cairo_font_options_create ();
    cairo_get_font_options (cr, opt);
    cairo_font_options_destroy (opt);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_set_font_face (cairo_t *cr)
{
    cairo_set_font_face (cr, cairo_get_font_face (cr));

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_set_scaled_font (cairo_t *cr)
{
    cairo_set_scaled_font (cr, cairo_get_scaled_font (cr));

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_show_text (cairo_t *cr)
{
    cairo_show_text (cr, "Cairo");

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_show_glyphs (cairo_t *cr)
{
    cairo_glyph_t glyph;

    glyph.index = 65;
    glyph.x = 0;
    glyph.y = 0;

    cairo_show_glyphs (cr, &glyph, 1);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_show_text_glyphs (cairo_t *cr)
{
    cairo_glyph_t glyph;
    cairo_text_cluster_t cluster;

    glyph.index = 65;
    glyph.x = 0;
    glyph.y = 0;

    cluster.num_bytes = 1;
    cluster.num_glyphs = 1;

    cairo_show_text_glyphs (cr, "a", -1, &glyph, 1, &cluster, 1, 0);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_text_path (cairo_t *cr)
{
    cairo_text_path (cr, "Cairo");

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_glyph_path (cairo_t *cr)
{
    cairo_glyph_t glyph;

    glyph.index = 65;
    glyph.x = 0;
    glyph.y = 0;

    cairo_glyph_path (cr, &glyph, 1);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_text_extents (cairo_t *cr)
{
    cairo_text_extents_t extents;

    cairo_text_extents (cr, "Cairo", &extents);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_glyph_extents (cairo_t *cr)
{
    cairo_glyph_t glyph;
    cairo_text_extents_t extents;

    glyph.index = 65;
    glyph.x = 0;
    glyph.y = 0;

    cairo_glyph_extents (cr, &glyph, 1, &extents);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_font_extents (cairo_t *cr)
{
    cairo_font_extents_t extents;

    cairo_font_extents (cr, &extents);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_get_operator (cairo_t *cr)
{
    cairo_get_operator (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_get_source (cairo_t *cr)
{
    cairo_get_source (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_get_tolerance (cairo_t *cr)
{
    cairo_get_tolerance (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_get_antialias (cairo_t *cr)
{
    cairo_get_antialias (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_has_current_point (cairo_t *cr)
{
    cairo_has_current_point (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_get_current_point (cairo_t *cr)
{
    double x, y;

    cairo_get_current_point (cr, &x, &y);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_get_fill_rule (cairo_t *cr)
{
    cairo_get_fill_rule (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_get_line_width (cairo_t *cr)
{
    cairo_get_line_width (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_get_line_cap (cairo_t *cr)
{
    cairo_get_line_cap (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_get_line_join (cairo_t *cr)
{
    cairo_get_line_join (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_get_miter_limit (cairo_t *cr)
{
    cairo_get_miter_limit (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_get_dash_count (cairo_t *cr)
{
    cairo_get_dash_count (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_get_dash (cairo_t *cr)
{
    double dashes[42];
    double offset;

    cairo_get_dash (cr, &dashes[0], &offset);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_get_matrix (cairo_t *cr)
{
    cairo_matrix_t matrix;

    cairo_get_matrix (cr, &matrix);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_get_target (cairo_t *cr)
{
    cairo_get_target (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_get_group_target (cairo_t *cr)
{
    cairo_get_group_target (cr);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_copy_path (cairo_t *cr)
{
    cairo_path_destroy (cairo_copy_path (cr));

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_copy_path_flat (cairo_t *cr)
{
    cairo_path_destroy (cairo_copy_path_flat (cr));

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_append_path (cairo_t *cr)
{
    cairo_path_data_t data[3];
    cairo_path_t path;

    path.status = CAIRO_STATUS_SUCCESS;
    path.data = &data[0];
    path.num_data = ARRAY_LENGTH(data);

    data[0].header.type = CAIRO_PATH_MOVE_TO;
    data[0].header.length = 2;
    data[1].point.x = 1;
    data[1].point.y = 2;
    data[2].header.type = CAIRO_PATH_CLOSE_PATH;
    data[2].header.length = 1;

    cairo_append_path (cr, &path);

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_surface_create_similar (cairo_surface_t *surface)
{
    cairo_surface_t *similar;
    
    similar = cairo_surface_create_similar (surface, CAIRO_CONTENT_ALPHA, 100, 100);
    
    cairo_surface_destroy (similar);
    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_surface_create_for_rectangle (cairo_surface_t *surface)
{
    cairo_surface_t *similar;
    
    similar = cairo_surface_create_for_rectangle (surface, 1, 1, 8, 8);
    
    cairo_surface_destroy (similar);
    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_surface_reference (cairo_surface_t *surface)
{
    cairo_surface_destroy (cairo_surface_reference (surface));
    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_surface_finish (cairo_surface_t *surface)
{
    cairo_surface_finish (surface);
    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_surface_get_device (cairo_surface_t *surface)
{
    /* cairo_device_t *device = */cairo_surface_get_device (surface);
    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_surface_get_reference_count (cairo_surface_t *surface)
{
    unsigned int refcount = cairo_surface_get_reference_count (surface);
    if (refcount > 0)
        return CAIRO_TEST_SUCCESS;
    /* inert error surfaces have a refcount of 0 */
    return cairo_surface_status (surface) ? CAIRO_TEST_SUCCESS : CAIRO_TEST_ERROR;
}

static cairo_test_status_t
test_cairo_surface_status (cairo_surface_t *surface)
{
    cairo_status_t status = cairo_surface_status (surface);
    return status < CAIRO_STATUS_LAST_STATUS ? CAIRO_TEST_SUCCESS : CAIRO_TEST_ERROR;
}

static cairo_test_status_t
test_cairo_surface_get_type (cairo_surface_t *surface)
{
    /* cairo_surface_type_t type = */cairo_surface_get_type (surface);
    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_surface_get_content (cairo_surface_t *surface)
{
    cairo_content_t content = cairo_surface_get_content (surface);

    switch (content) {
    case CAIRO_CONTENT_COLOR:
    case CAIRO_CONTENT_ALPHA:
    case CAIRO_CONTENT_COLOR_ALPHA:
        return CAIRO_TEST_SUCCESS;
    default:
        return CAIRO_TEST_ERROR;
    }
}

static cairo_test_status_t
test_cairo_surface_set_user_data (cairo_surface_t *surface)
{
    static cairo_user_data_key_t key;
    cairo_status_t status;

    status = cairo_surface_set_user_data (surface, &key, &key, NULL);
    if (status == CAIRO_STATUS_NO_MEMORY)
        return CAIRO_TEST_NO_MEMORY;
    else if (status)
        return CAIRO_TEST_SUCCESS;

    if (cairo_surface_get_user_data (surface, &key) != &key)
        return CAIRO_TEST_ERROR;

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_surface_set_mime_data (cairo_surface_t *surface)
{
    const char *mimetype = "text/x-uri";
    const char *data = "https://www.cairographics.org";
    cairo_status_t status;

    status = cairo_surface_set_mime_data (surface,
                                          mimetype,
                                          (const unsigned char *) data,
					  strlen (data),
                                          NULL, NULL);
    return status ? CAIRO_TEST_SUCCESS : CAIRO_TEST_ERROR;
}

static cairo_test_status_t
test_cairo_surface_get_mime_data (cairo_surface_t *surface)
{
    const char *mimetype = "text/x-uri";
    const unsigned char *data;
    unsigned long length;

    cairo_surface_get_mime_data (surface, mimetype, &data, &length);
    return data == NULL && length == 0 ? CAIRO_TEST_SUCCESS : CAIRO_TEST_ERROR;
}

static cairo_test_status_t
test_cairo_surface_get_font_options (cairo_surface_t *surface)
{
    cairo_font_options_t *options;
    cairo_status_t status;

    options = cairo_font_options_create ();
    if (likely (!cairo_font_options_status (options)))
        cairo_surface_get_font_options (surface, options);
    status = cairo_font_options_status (options);
    cairo_font_options_destroy (options);
    return status ? CAIRO_TEST_ERROR : CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_surface_flush (cairo_surface_t *surface)
{
    cairo_surface_flush (surface);
    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_surface_mark_dirty (cairo_surface_t *surface)
{
    cairo_surface_mark_dirty (surface);
    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_surface_mark_dirty_rectangle (cairo_surface_t *surface)
{
    cairo_surface_mark_dirty_rectangle (surface, 1, 1, 8, 8);
    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_surface_set_device_offset (cairo_surface_t *surface)
{
    cairo_surface_set_device_offset (surface, 5, 5);
    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_surface_get_device_offset (cairo_surface_t *surface)
{
    double x, y;

    cairo_surface_get_device_offset (surface, &x, &y);
    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_surface_set_fallback_resolution (cairo_surface_t *surface)
{
    cairo_surface_set_fallback_resolution (surface, 42, 42);
    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_surface_get_fallback_resolution (cairo_surface_t *surface)
{
    double x, y;

    cairo_surface_get_fallback_resolution (surface, &x, &y);
    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_surface_copy_page (cairo_surface_t *surface)
{
    cairo_surface_copy_page (surface);
    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_surface_show_page (cairo_surface_t *surface)
{
    cairo_surface_show_page (surface);
    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_surface_has_show_text_glyphs (cairo_surface_t *surface)
{
    cairo_surface_has_show_text_glyphs (surface);
    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_image_surface_get_data (cairo_surface_t *surface)
{
    unsigned char *data = cairo_image_surface_get_data (surface);
    return data == NULL || surface_has_type (surface, CAIRO_SURFACE_TYPE_IMAGE) ? CAIRO_TEST_SUCCESS : CAIRO_TEST_ERROR;
}

static cairo_test_status_t
test_cairo_image_surface_get_format (cairo_surface_t *surface)
{
    cairo_format_t format = cairo_image_surface_get_format (surface);
    return format == CAIRO_FORMAT_INVALID || surface_has_type (surface, CAIRO_SURFACE_TYPE_IMAGE) ? CAIRO_TEST_SUCCESS : CAIRO_TEST_ERROR;
}

static cairo_test_status_t
test_cairo_image_surface_get_width (cairo_surface_t *surface)
{
    unsigned int width = cairo_image_surface_get_width (surface);
    return width == 0 || surface_has_type (surface, CAIRO_SURFACE_TYPE_IMAGE) ? CAIRO_TEST_SUCCESS : CAIRO_TEST_ERROR;
}

static cairo_test_status_t
test_cairo_image_surface_get_height (cairo_surface_t *surface)
{
    unsigned int height = cairo_image_surface_get_height (surface);
    return height == 0 || surface_has_type (surface, CAIRO_SURFACE_TYPE_IMAGE) ? CAIRO_TEST_SUCCESS : CAIRO_TEST_ERROR;
}

static cairo_test_status_t
test_cairo_image_surface_get_stride (cairo_surface_t *surface)
{
    unsigned int stride = cairo_image_surface_get_stride (surface);
    return stride == 0 || surface_has_type (surface, CAIRO_SURFACE_TYPE_IMAGE) ? CAIRO_TEST_SUCCESS : CAIRO_TEST_ERROR;
}

#if CAIRO_HAS_PNG_FUNCTIONS

static cairo_test_status_t
test_cairo_surface_write_to_png (cairo_surface_t *surface)
{
    cairo_status_t status;

    status = cairo_surface_write_to_png (surface, "/this/file/will/definitely/not/exist.png");
    
    return status ? CAIRO_TEST_SUCCESS : CAIRO_TEST_ERROR;
}

static cairo_status_t
write_func_that_always_fails (void *closure, const unsigned char *data, unsigned int length)
{
    return CAIRO_STATUS_WRITE_ERROR;
}

static cairo_test_status_t
test_cairo_surface_write_to_png_stream (cairo_surface_t *surface)
{
    cairo_status_t status;

    status = cairo_surface_write_to_png_stream (surface,
                                                write_func_that_always_fails,
                                                NULL);
    
    return status && status != CAIRO_STATUS_WRITE_ERROR ? CAIRO_TEST_SUCCESS : CAIRO_TEST_ERROR;
}

#endif /* CAIRO_HAS_PNG_FUNCTIONS */

static cairo_test_status_t
test_cairo_recording_surface_ink_extents (cairo_surface_t *surface)
{
    double x, y, w, h;

    cairo_recording_surface_ink_extents (surface, &x, &y, &w, &h);
    return x == 0 && y == 0 && w == 0 && h == 0 ? CAIRO_TEST_SUCCESS : CAIRO_TEST_ERROR;
}

#if CAIRO_HAS_TEE_SURFACE

static cairo_test_status_t
test_cairo_tee_surface_add (cairo_surface_t *surface)
{
    cairo_surface_t *image = cairo_image_surface_create (CAIRO_FORMAT_A8, 10, 10);

    cairo_tee_surface_add (surface, image);
    cairo_surface_destroy (image);
    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_tee_surface_remove (cairo_surface_t *surface)
{
    cairo_surface_t *image = cairo_image_surface_create (CAIRO_FORMAT_A8, 10, 10);

    cairo_tee_surface_remove (surface, image);
    cairo_surface_destroy (image);
    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_tee_surface_index (cairo_surface_t *surface)
{
    cairo_surface_t *master;
    cairo_status_t status;

    master = cairo_tee_surface_index (surface, 0);
    status = cairo_surface_status (master);
    cairo_surface_destroy (master);
    return status ? CAIRO_TEST_SUCCESS : CAIRO_TEST_ERROR;
}

#endif /* CAIRO_HAS_TEE_SURFACE */

#if CAIRO_HAS_PDF_SURFACE

static cairo_test_status_t
test_cairo_pdf_surface_restrict_to_version (cairo_surface_t *surface)
{
    cairo_pdf_surface_restrict_to_version (surface, CAIRO_PDF_VERSION_1_4);
    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_pdf_surface_set_size (cairo_surface_t *surface)
{
    cairo_pdf_surface_set_size (surface, 5, 5);
    return CAIRO_TEST_SUCCESS;
}

#endif /* CAIRO_HAS_PDF_SURFACE */

#if CAIRO_HAS_PS_SURFACE

static cairo_test_status_t
test_cairo_ps_surface_restrict_to_level (cairo_surface_t *surface)
{
    cairo_ps_surface_restrict_to_level (surface, CAIRO_PS_LEVEL_2);
    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_ps_surface_set_eps (cairo_surface_t *surface)
{
    cairo_ps_surface_set_eps (surface, TRUE);
    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_ps_surface_get_eps (cairo_surface_t *surface)
{
    cairo_bool_t eps = cairo_ps_surface_get_eps (surface);
    return eps ? CAIRO_TEST_ERROR : CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_ps_surface_set_size (cairo_surface_t *surface)
{
    cairo_ps_surface_set_size (surface, 5, 5);
    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_ps_surface_dsc_comment (cairo_surface_t *surface)
{
    cairo_ps_surface_dsc_comment (surface, "54, 74, 90, 2010");
    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_ps_surface_dsc_begin_setup (cairo_surface_t *surface)
{
    cairo_ps_surface_dsc_begin_setup (surface);
    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_ps_surface_dsc_begin_page_setup (cairo_surface_t *surface)
{
    cairo_ps_surface_dsc_begin_page_setup (surface);
    return CAIRO_TEST_SUCCESS;
}

#endif /* CAIRO_HAS_PS_SURFACE */

#if CAIRO_HAS_QUARTZ_SURFACE

static cairo_test_status_t
test_cairo_quartz_surface_get_cg_context (cairo_surface_t *surface)
{
    CGContextRef context = cairo_quartz_surface_get_cg_context (surface);
    return context == NULL || surface_has_type (surface, CAIRO_SURFACE_TYPE_QUARTZ) ? CAIRO_TEST_SUCCESS : CAIRO_TEST_ERROR;
}

#endif /* CAIRO_HAS_QUARTZ_SURFACE */

#if CAIRO_HAS_SVG_SURFACE

static cairo_test_status_t
test_cairo_svg_surface_restrict_to_version (cairo_surface_t *surface)
{
    cairo_svg_surface_restrict_to_version (surface, CAIRO_SVG_VERSION_1_1);
    return CAIRO_TEST_SUCCESS;
}

#endif /* CAIRO_HAS_SVG_SURFACE */

#if CAIRO_HAS_XCB_SURFACE

static cairo_test_status_t
test_cairo_xcb_surface_set_size (cairo_surface_t *surface)
{
    cairo_xcb_surface_set_size (surface, 5, 5);
    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_xcb_surface_set_drawable (cairo_surface_t *surface)
{
    cairo_xcb_surface_set_drawable (surface, 0, 5, 5);
    return CAIRO_TEST_SUCCESS;
}

#endif

#if CAIRO_HAS_XLIB_SURFACE

static cairo_test_status_t
test_cairo_xlib_surface_set_size (cairo_surface_t *surface)
{
    cairo_xlib_surface_set_size (surface, 5, 5);
    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_xlib_surface_set_drawable (cairo_surface_t *surface)
{
    cairo_xlib_surface_set_drawable (surface, 0, 5, 5);
    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_cairo_xlib_surface_get_display (cairo_surface_t *surface)
{
    Display *display = cairo_xlib_surface_get_display (surface);
    return display == NULL || surface_has_type (surface, CAIRO_SURFACE_TYPE_XLIB) ? CAIRO_TEST_SUCCESS : CAIRO_TEST_ERROR;
}

static cairo_test_status_t
test_cairo_xlib_surface_get_screen (cairo_surface_t *surface)
{
    Screen *screen = cairo_xlib_surface_get_screen (surface);
    return screen == NULL || surface_has_type (surface, CAIRO_SURFACE_TYPE_XLIB) ? CAIRO_TEST_SUCCESS : CAIRO_TEST_ERROR;
}

static cairo_test_status_t
test_cairo_xlib_surface_get_visual (cairo_surface_t *surface)
{
    Visual *visual = cairo_xlib_surface_get_visual (surface);
    return visual == NULL || surface_has_type (surface, CAIRO_SURFACE_TYPE_XLIB) ? CAIRO_TEST_SUCCESS : CAIRO_TEST_ERROR;
}

static cairo_test_status_t
test_cairo_xlib_surface_get_drawable (cairo_surface_t *surface)
{
    Drawable drawable = cairo_xlib_surface_get_drawable (surface);
    return drawable == 0 || surface_has_type (surface, CAIRO_SURFACE_TYPE_XLIB) ? CAIRO_TEST_SUCCESS : CAIRO_TEST_ERROR;
}

static cairo_test_status_t
test_cairo_xlib_surface_get_depth (cairo_surface_t *surface)
{
    int depth = cairo_xlib_surface_get_depth (surface);
    return depth == 0 || surface_has_type (surface, CAIRO_SURFACE_TYPE_XLIB) ? CAIRO_TEST_SUCCESS : CAIRO_TEST_ERROR;
}

static cairo_test_status_t
test_cairo_xlib_surface_get_width (cairo_surface_t *surface)
{
    int width = cairo_xlib_surface_get_width (surface);
    return width == 0 || surface_has_type (surface, CAIRO_SURFACE_TYPE_XLIB) ? CAIRO_TEST_SUCCESS : CAIRO_TEST_ERROR;
}

static cairo_test_status_t
test_cairo_xlib_surface_get_height (cairo_surface_t *surface)
{
    int height = cairo_xlib_surface_get_height (surface);
    return height == 0 || surface_has_type (surface, CAIRO_SURFACE_TYPE_XLIB) ? CAIRO_TEST_SUCCESS : CAIRO_TEST_ERROR;
}

#endif

#define TEST(name) { #name, test_ ## name }

struct {
    const char *name;
    context_test_func_t func;
} context_tests[] = {
    TEST (cairo_reference),
    TEST (cairo_get_reference_count),
    TEST (cairo_set_user_data),
    TEST (cairo_save),
    TEST (cairo_push_group),
    TEST (cairo_push_group_with_content),
    TEST (cairo_set_operator),
    TEST (cairo_set_source),
    TEST (cairo_set_source_rgb),
    TEST (cairo_set_source_rgba),
    TEST (cairo_set_source_surface),
    TEST (cairo_set_tolerance),
    TEST (cairo_set_antialias),
    TEST (cairo_set_fill_rule),
    TEST (cairo_set_line_width),
    TEST (cairo_set_line_cap),
    TEST (cairo_set_line_join),
    TEST (cairo_set_dash),
    TEST (cairo_set_miter_limit),
    TEST (cairo_translate),
    TEST (cairo_scale),
    TEST (cairo_rotate),
    TEST (cairo_transform),
    TEST (cairo_set_matrix),
    TEST (cairo_identity_matrix),
    TEST (cairo_user_to_device),
    TEST (cairo_user_to_device_distance),
    TEST (cairo_device_to_user),
    TEST (cairo_device_to_user_distance),
    TEST (cairo_new_path),
    TEST (cairo_move_to),
    TEST (cairo_new_sub_path),
    TEST (cairo_line_to),
    TEST (cairo_curve_to),
    TEST (cairo_arc),
    TEST (cairo_arc_negative),
    TEST (cairo_rel_move_to),
    TEST (cairo_rel_line_to),
    TEST (cairo_rel_curve_to),
    TEST (cairo_rectangle),
    TEST (cairo_close_path),
    TEST (cairo_path_extents),
    TEST (cairo_paint),
    TEST (cairo_paint_with_alpha),
    TEST (cairo_mask),
    TEST (cairo_mask_surface),
    TEST (cairo_stroke),
    TEST (cairo_stroke_preserve),
    TEST (cairo_fill),
    TEST (cairo_fill_preserve),
    TEST (cairo_copy_page),
    TEST (cairo_show_page),
    TEST (cairo_in_stroke),
    TEST (cairo_in_fill),
    TEST (cairo_in_clip),
    TEST (cairo_stroke_extents),
    TEST (cairo_fill_extents),
    TEST (cairo_reset_clip),
    TEST (cairo_clip),
    TEST (cairo_clip_preserve),
    TEST (cairo_clip_extents),
    TEST (cairo_copy_clip_rectangle_list),
    TEST (cairo_select_font_face),
    TEST (cairo_set_font_size),
    TEST (cairo_set_font_matrix),
    TEST (cairo_get_font_matrix),
    TEST (cairo_set_font_options),
    TEST (cairo_get_font_options),
    TEST (cairo_set_font_face),
    TEST (cairo_set_scaled_font),
    TEST (cairo_show_text),
    TEST (cairo_show_glyphs),
    TEST (cairo_show_text_glyphs),
    TEST (cairo_text_path),
    TEST (cairo_glyph_path),
    TEST (cairo_text_extents),
    TEST (cairo_glyph_extents),
    TEST (cairo_font_extents),
    TEST (cairo_get_operator),
    TEST (cairo_get_source),
    TEST (cairo_get_tolerance),
    TEST (cairo_get_antialias),
    TEST (cairo_has_current_point),
    TEST (cairo_get_current_point),
    TEST (cairo_get_fill_rule),
    TEST (cairo_get_line_width),
    TEST (cairo_get_line_cap),
    TEST (cairo_get_line_join),
    TEST (cairo_get_miter_limit),
    TEST (cairo_get_dash_count),
    TEST (cairo_get_dash),
    TEST (cairo_get_matrix),
    TEST (cairo_get_target),
    TEST (cairo_get_group_target),
    TEST (cairo_copy_path),
    TEST (cairo_copy_path_flat),
    TEST (cairo_append_path),
};

#undef TEST

#define TEST(name, surface_type, sets_status) { #name, test_ ## name, surface_type, sets_status }

struct {
    const char *name;
    surface_test_func_t func;
    int surface_type; /* cairo_surface_type_t or -1 */
    cairo_bool_t modifies_surface;
} surface_tests[] = {
    TEST (cairo_surface_create_similar, -1, FALSE),
    TEST (cairo_surface_create_for_rectangle, -1, FALSE),
    TEST (cairo_surface_reference, -1, FALSE),
    TEST (cairo_surface_finish, -1, TRUE),
    TEST (cairo_surface_get_device, -1, FALSE),
    TEST (cairo_surface_get_reference_count, -1, FALSE),
    TEST (cairo_surface_status, -1, FALSE),
    TEST (cairo_surface_get_type, -1, FALSE),
    TEST (cairo_surface_get_content, -1, FALSE),
    TEST (cairo_surface_set_user_data, -1, FALSE),
    TEST (cairo_surface_set_mime_data, -1, TRUE),
    TEST (cairo_surface_get_mime_data, -1, FALSE),
    TEST (cairo_surface_get_font_options, -1, FALSE),
    TEST (cairo_surface_flush, -1, TRUE),
    TEST (cairo_surface_mark_dirty, -1, TRUE),
    TEST (cairo_surface_mark_dirty_rectangle, -1, TRUE),
    TEST (cairo_surface_set_device_offset, -1, TRUE),
    TEST (cairo_surface_get_device_offset, -1, FALSE),
    TEST (cairo_surface_set_fallback_resolution, -1, TRUE),
    TEST (cairo_surface_get_fallback_resolution, -1, FALSE),
    TEST (cairo_surface_copy_page, -1, TRUE),
    TEST (cairo_surface_show_page, -1, TRUE),
    TEST (cairo_surface_has_show_text_glyphs, -1, FALSE),
    TEST (cairo_image_surface_get_data, CAIRO_SURFACE_TYPE_IMAGE, FALSE),
    TEST (cairo_image_surface_get_format, CAIRO_SURFACE_TYPE_IMAGE, FALSE),
    TEST (cairo_image_surface_get_width, CAIRO_SURFACE_TYPE_IMAGE, FALSE),
    TEST (cairo_image_surface_get_height, CAIRO_SURFACE_TYPE_IMAGE, FALSE),
    TEST (cairo_image_surface_get_stride, CAIRO_SURFACE_TYPE_IMAGE, FALSE),
#if CAIRO_HAS_PNG_FUNCTIONS
    TEST (cairo_surface_write_to_png, -1, FALSE),
    TEST (cairo_surface_write_to_png_stream, -1, FALSE),
#endif
    TEST (cairo_recording_surface_ink_extents, CAIRO_SURFACE_TYPE_RECORDING, FALSE),
#if CAIRO_HAS_TEE_SURFACE
    TEST (cairo_tee_surface_add, CAIRO_SURFACE_TYPE_TEE, TRUE),
    TEST (cairo_tee_surface_remove, CAIRO_SURFACE_TYPE_TEE, TRUE),
    TEST (cairo_tee_surface_index, CAIRO_SURFACE_TYPE_TEE, FALSE),
#endif
#if CAIRO_HAS_PDF_SURFACE
    TEST (cairo_pdf_surface_restrict_to_version, CAIRO_SURFACE_TYPE_PDF, TRUE),
    TEST (cairo_pdf_surface_set_size, CAIRO_SURFACE_TYPE_PDF, TRUE),
#endif
#if CAIRO_HAS_PS_SURFACE
    TEST (cairo_ps_surface_restrict_to_level, CAIRO_SURFACE_TYPE_PS, TRUE),
    TEST (cairo_ps_surface_set_eps, CAIRO_SURFACE_TYPE_PS, TRUE),
    TEST (cairo_ps_surface_get_eps, CAIRO_SURFACE_TYPE_PS, FALSE),
    TEST (cairo_ps_surface_set_size, CAIRO_SURFACE_TYPE_PS, TRUE),
    TEST (cairo_ps_surface_dsc_comment, CAIRO_SURFACE_TYPE_PS, TRUE),
    TEST (cairo_ps_surface_dsc_begin_setup, CAIRO_SURFACE_TYPE_PS, TRUE),
    TEST (cairo_ps_surface_dsc_begin_page_setup, CAIRO_SURFACE_TYPE_PS, TRUE),
#endif
#if CAIRO_HAS_QUARTZ_SURFACE
    TEST (cairo_quartz_surface_get_cg_context, CAIRO_SURFACE_TYPE_QUARTZ, FALSE),
#endif
#if CAIRO_HAS_SVG_SURFACE
    TEST (cairo_svg_surface_restrict_to_version, CAIRO_SURFACE_TYPE_SVG, TRUE),
#endif
#if CAIRO_HAS_XCB_SURFACE
    TEST (cairo_xcb_surface_set_size, CAIRO_SURFACE_TYPE_XCB, TRUE),
    TEST (cairo_xcb_surface_set_drawable, CAIRO_SURFACE_TYPE_XCB, TRUE),
#endif
#if CAIRO_HAS_XLIB_SURFACE
    TEST (cairo_xlib_surface_set_size, CAIRO_SURFACE_TYPE_XLIB, TRUE),
    TEST (cairo_xlib_surface_set_drawable, CAIRO_SURFACE_TYPE_XLIB, TRUE),
    TEST (cairo_xlib_surface_get_display, CAIRO_SURFACE_TYPE_XLIB, FALSE),
    TEST (cairo_xlib_surface_get_drawable, CAIRO_SURFACE_TYPE_XLIB, FALSE),
    TEST (cairo_xlib_surface_get_screen, CAIRO_SURFACE_TYPE_XLIB, FALSE),
    TEST (cairo_xlib_surface_get_visual, CAIRO_SURFACE_TYPE_XLIB, FALSE),
    TEST (cairo_xlib_surface_get_depth, CAIRO_SURFACE_TYPE_XLIB, FALSE),
    TEST (cairo_xlib_surface_get_width, CAIRO_SURFACE_TYPE_XLIB, FALSE),
    TEST (cairo_xlib_surface_get_height, CAIRO_SURFACE_TYPE_XLIB, FALSE),
#endif
};

static cairo_test_status_t
preamble (cairo_test_context_t *ctx)
{
    cairo_surface_t *surface;
    cairo_t *cr;
    cairo_test_status_t test_status;
    cairo_status_t status_before, status_after;
    unsigned int i;

    /* Test an error surface */
    for (i = 0; i < ARRAY_LENGTH (surface_tests); i++) {
        surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, INT_MAX, INT_MAX);
        status_before = cairo_surface_status (surface);
        assert (status_before);

        test_status = surface_tests[i].func (surface);

        status_after = cairo_surface_status (surface);
        cairo_surface_destroy (surface);

        if (test_status != CAIRO_TEST_SUCCESS) {
            cairo_test_log (ctx,
                            "Failed test %s with %d\n",
                            surface_tests[i].name, (int) test_status);
            return test_status;
        }

        if (status_before != status_after) {
            cairo_test_log (ctx,
                            "Failed test %s: Modified surface status from %u (%s) to %u (%s)\n",
                            surface_tests[i].name,
                            status_before, cairo_status_to_string (status_before),
                            status_after, cairo_status_to_string (status_after));
            return CAIRO_TEST_ERROR;
        }
    }

    /* Test an error context */
    for (i = 0; i < ARRAY_LENGTH (context_tests); i++) {
        cr = cairo_create (NULL);
        status_before = cairo_status (cr);
        assert (status_before);

        test_status = context_tests[i].func (cr);

        status_after = cairo_status (cr);
        cairo_destroy (cr);

        if (test_status != CAIRO_TEST_SUCCESS) {
            cairo_test_log (ctx,
                            "Failed test %s with %d\n",
                            context_tests[i].name, (int) test_status);
            return test_status;
        }

        if (status_before != status_after) {
            cairo_test_log (ctx,
                            "Failed test %s: Modified context status from %u (%s) to %u (%s)\n",
                            context_tests[i].name,
                            status_before, cairo_status_to_string (status_before),
                            status_after, cairo_status_to_string (status_after));
            return CAIRO_TEST_ERROR;
        }
    }

    /* Test a context for an error surface */
    for (i = 0; i < ARRAY_LENGTH (context_tests); i++) {
        surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, INT_MAX, INT_MAX);
        cr = cairo_create (surface);
        cairo_surface_destroy (surface);
        status_before = cairo_status (cr);
        assert (status_before);

        test_status = context_tests[i].func (cr);

        status_after = cairo_status (cr);
        cairo_destroy (cr);

        if (test_status != CAIRO_TEST_SUCCESS) {
            cairo_test_log (ctx,
                            "Failed test %s with %d\n",
                            context_tests[i].name, (int) test_status);
            return test_status;
        }

        if (status_before != status_after) {
            cairo_test_log (ctx,
                            "Failed test %s: Modified context status from %u (%s) to %u (%s)\n",
                            context_tests[i].name,
                            status_before, cairo_status_to_string (status_before),
                            status_after, cairo_status_to_string (status_after));
            return CAIRO_TEST_ERROR;
        }
    }

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
test_context (const cairo_test_context_t *ctx, cairo_t *cr, const char *name, unsigned int i)
{
    cairo_test_status_t test_status;
    cairo_status_t status_before, status_after;

    /* Make sure that there is a current point */
    cairo_move_to (cr, 0, 0);

    status_before = cairo_status (cr);
    test_status = context_tests[i].func (cr);
    status_after = cairo_status (cr);

    if (test_status != CAIRO_TEST_SUCCESS) {
        cairo_test_log (ctx,
                        "Failed test %s on %s with %d\n",
                        context_tests[i].name, name, (int) test_status);
        return test_status;
    }

    if (status_after != CAIRO_STATUS_SURFACE_FINISHED && status_before != status_after) {
        cairo_test_log (ctx,
                        "Failed test %s on %s: Modified context status from %u (%s) to %u (%s)\n",
                        context_tests[i].name, name,
                        status_before, cairo_status_to_string (status_before),
                        status_after, cairo_status_to_string (status_after));
        return CAIRO_TEST_ERROR;
    }

    return CAIRO_TEST_SUCCESS;
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    const cairo_test_context_t *ctx = cairo_test_get_context (cr);
    cairo_surface_t *similar, *target;
    cairo_test_status_t test_status;
    cairo_status_t status;
    cairo_t *cr2;
    unsigned int i;

    target = cairo_get_target (cr);

    /* Test a finished similar surface */
    for (i = 0; i < ARRAY_LENGTH (surface_tests); i++) {
        similar = cairo_surface_create_similar (target,
                                                cairo_surface_get_content (target),
                                                10, 10);
        cairo_surface_finish (similar);
        test_status = surface_tests[i].func (similar);
        status = cairo_surface_status (similar);
        cairo_surface_destroy (similar);

        if (test_status != CAIRO_TEST_SUCCESS) {
            cairo_test_log (ctx,
                            "Failed test %s with %d\n",
                            surface_tests[i].name, (int) test_status);
            return test_status;
        }

        if (surface_tests[i].modifies_surface &&
            strcmp (surface_tests[i].name, "cairo_surface_finish") &&
            strcmp (surface_tests[i].name, "cairo_surface_flush") &&
            status != CAIRO_STATUS_SURFACE_FINISHED) {
            cairo_test_log (ctx,
                            "Failed test %s: Finished surface not set into error state\n",
                            surface_tests[i].name);
            return CAIRO_TEST_ERROR;
        }
    }

    /* Test a context for a finished similar surface */
    for (i = 0; i < ARRAY_LENGTH (context_tests); i++) {
        similar = cairo_surface_create_similar (target,
                                                cairo_surface_get_content (target),
                                                10, 10);
        cairo_surface_finish (similar);
        cr2 = cairo_create (similar);
        test_status = test_context (ctx, cr2, "finished surface", i);
        cairo_surface_destroy (similar);
        cairo_destroy (cr2);

        if (test_status != CAIRO_TEST_SUCCESS)
            return test_status;
    }

    /* Test a context for a similar surface finished later */
    for (i = 0; i < ARRAY_LENGTH (context_tests); i++) {
        similar = cairo_surface_create_similar (target,
                                                cairo_surface_get_content (target),
                                                10, 10);
        cr2 = cairo_create (similar);
        cairo_surface_finish (similar);
        test_status = test_context (ctx, cr2, "finished surface after create", i);
        cairo_surface_destroy (similar);
        cairo_destroy (cr2);

        if (test_status != CAIRO_TEST_SUCCESS)
            return test_status;
    }

    /* Test a context for a similar surface finished later with a path */
    for (i = 0; i < ARRAY_LENGTH (context_tests); i++) {
        similar = cairo_surface_create_similar (target,
                                                cairo_surface_get_content (target),
                                                10, 10);
        cr2 = cairo_create (similar);
        cairo_rectangle (cr2, 2, 2, 4, 4);
        cairo_surface_finish (similar);
        test_status = test_context (ctx, cr2, "finished surface with path", i);
        cairo_surface_destroy (similar);
        cairo_destroy (cr2);

        if (test_status != CAIRO_TEST_SUCCESS)
            return test_status;
    }

    /* Test a normal surface for functions that have the wrong type */
    for (i = 0; i < ARRAY_LENGTH (surface_tests); i++) {
        cairo_status_t desired_status;

        if (surface_tests[i].surface_type == -1)
            continue;
        similar = cairo_surface_create_similar (target,
                                                cairo_surface_get_content (target),
                                                10, 10);
        if (cairo_surface_get_type (similar) == (cairo_surface_type_t) surface_tests[i].surface_type) {
            cairo_surface_destroy (similar);
            continue;
        }

        test_status = surface_tests[i].func (similar);
        status = cairo_surface_status (similar);
        cairo_surface_destroy (similar);

        if (test_status != CAIRO_TEST_SUCCESS) {
            cairo_test_log (ctx,
                            "Failed test %s with %d\n",
                            surface_tests[i].name, (int) test_status);
            return test_status;
        }

        desired_status = surface_tests[i].modifies_surface ? CAIRO_STATUS_SURFACE_TYPE_MISMATCH : CAIRO_STATUS_SUCCESS;
        if (status != desired_status) {
            cairo_test_log (ctx,
                            "Failed test %s: Surface status should be %u (%s), but is %u (%s)\n",
                            surface_tests[i].name,
                            desired_status, cairo_status_to_string (desired_status),
                            status, cairo_status_to_string (status));
            return CAIRO_TEST_ERROR;
        }
    }

    /* 565-compatible gray background */
    cairo_set_source_rgb (cr, 0.51613, 0.55555, 0.51613);
    cairo_paint (cr);

    return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (api_special_cases,
	    "Check surface functions properly handle wrong surface arguments",
	    "api", /* keywords */
	    NULL, /* requirements */
	    10, 10,
	    preamble, draw)
