/*
 * Copyright © 2006 Novell, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Novell, Inc. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. Novell, Inc. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * NOVELL, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL RED HAT, INC. BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Robert O'Callahan <rocallahan@novell.com>
 */

#include "cairo-test.h"
#include <stddef.h>
#include <math.h>

enum ExtentsType { FILL, STROKE, PATH };

enum Relation { EQUALS, APPROX_EQUALS, CONTAINS };


static cairo_bool_t within_tolerance(double x1, double y1,
				     double x2, double y2,
				     double expected_x1, double expected_y1,
				     double expected_x2, double expected_y2,
				     double tolerance)
{
    return (fabs (expected_x1 - x1) < tolerance &&
	    fabs (expected_y1 - y1) < tolerance &&
	    fabs (expected_x2 - x2) < tolerance &&
	    fabs (expected_y2 - y2) < tolerance);
}

static cairo_bool_t
check_extents (const cairo_test_context_t *ctx,
	       const char *message, cairo_t *cr, enum ExtentsType type,
               enum Relation relation,
               double x, double y, double width, double height)
{
    double ext_x1, ext_y1, ext_x2, ext_y2;
    const char *type_string;
    const char *relation_string;

    switch (type) {
    default:
    case FILL:
        type_string = "fill";
        cairo_fill_extents (cr, &ext_x1, &ext_y1, &ext_x2, &ext_y2);
        break;
    case STROKE:
        type_string = "stroke";
        cairo_stroke_extents (cr, &ext_x1, &ext_y1, &ext_x2, &ext_y2);
        break;
    case PATH:
        type_string = "path";
        cairo_path_extents (cr, &ext_x1, &ext_y1, &ext_x2, &ext_y2);
        break;
    }

    /* ignore results after an error occurs */
    if (cairo_status (cr))
	return 1;

    switch (relation) {
    default:
    case EQUALS:
        relation_string = "equal";
	if (within_tolerance(x, y, x + width, y + height,
			     ext_x1, ext_y1, ext_x2, ext_y2,
			     cairo_get_tolerance(cr)))
	    return 1;
        break;
    case APPROX_EQUALS:
        relation_string = "approx. equal";
	if (within_tolerance(x, y, x + width, y + height,
			     ext_x1, ext_y1, ext_x2, ext_y2,
			     1.))
	    return 1;
        break;
    case CONTAINS:
        relation_string = "contain";
        if (width == 0 || height == 0) {
            /* odd test that doesn't really test anything... */
            return 1;
        }
        if (ext_x1 <= x && ext_y1 <= y && ext_x2 >= x + width && ext_y2 >= y + height)
            return 1;
        break;
    }

    cairo_test_log (ctx, "Error: %s; %s extents (%g, %g) x (%g, %g) should %s (%g, %g) x (%g, %g)\n",
                    message, type_string,
                    ext_x1, ext_y1, ext_x2 - ext_x1, ext_y2 - ext_y1,
                    relation_string,
                    x, y, width, height);
    return 0;
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    const cairo_test_context_t *ctx = cairo_test_get_context (cr);
    cairo_surface_t *surface;
    cairo_t         *cr2;
    const char      *phase;
    const char	     string[] = "The quick brown fox jumps over the lazy dog.";
    cairo_text_extents_t extents, scaled_font_extents;
    cairo_status_t   status;
    int              errors = 0;

    surface = cairo_surface_create_similar (cairo_get_group_target (cr),
                                            CAIRO_CONTENT_COLOR, 1000, 1000);
    /* don't use cr accidentally */
    cr = NULL;
    cr2 = cairo_create (surface);
    cairo_surface_destroy (surface);

    cairo_set_line_width (cr2, 10);
    cairo_set_line_join (cr2, CAIRO_LINE_JOIN_MITER);
    cairo_set_miter_limit (cr2, 100);

    phase = "No path";
    errors += !check_extents (ctx, phase, cr2, FILL, EQUALS, 0, 0, 0, 0);
    errors += !check_extents (ctx, phase, cr2, STROKE, EQUALS, 0, 0, 0, 0);
    errors += !check_extents (ctx, phase, cr2, PATH, EQUALS, 0, 0, 0, 0);

    cairo_save (cr2);

    cairo_new_path (cr2);
    cairo_move_to (cr2, 200, 400);
    cairo_close_path (cr2);
    phase = "Degenerate closed path";
    errors += !check_extents (ctx, phase, cr2, FILL, EQUALS, 0, 0, 0, 0);
    errors += !check_extents (ctx, phase, cr2, STROKE, EQUALS, 0, 0, 0, 0);
    errors += !check_extents (ctx, phase, cr2, PATH, EQUALS, 200, 400, 0, 0);

    cairo_new_path (cr2);
    cairo_move_to (cr2, 200, 400);
    cairo_rel_line_to (cr2, 0., 0.);
    phase = "Degenerate line";
    errors += !check_extents (ctx, phase, cr2, FILL, EQUALS, 0, 0, 0, 0);
    errors += !check_extents (ctx, phase, cr2, STROKE, EQUALS, 0, 0, 0, 0);
    errors += !check_extents (ctx, phase, cr2, PATH, EQUALS, 200, 400, 0, 0);

    cairo_new_path (cr2);
    cairo_move_to (cr2, 200, 400);
    cairo_rel_curve_to (cr2, 0., 0., 0., 0., 0., 0.);
    phase = "Degenerate curve";
    errors += !check_extents (ctx, phase, cr2, FILL, EQUALS, 0, 0, 0, 0);
    errors += !check_extents (ctx, phase, cr2, STROKE, EQUALS, 0, 0, 0, 0);
    errors += !check_extents (ctx, phase, cr2, PATH, EQUALS, 200, 400, 0, 0);

    cairo_new_path (cr2);
    cairo_arc (cr2, 200, 400, 0., 0, 2 * M_PI);
    phase = "Degenerate arc (R=0)";
    errors += !check_extents (ctx, phase, cr2, FILL, EQUALS, 0, 0, 0, 0);
    errors += !check_extents (ctx, phase, cr2, STROKE, EQUALS, 0, 0, 0, 0);
    errors += !check_extents (ctx, phase, cr2, PATH, EQUALS, 200, 400, 0, 0);

    cairo_new_path (cr2);
    cairo_arc_negative (cr2, 200, 400, 0., 0, 2 * M_PI);
    phase = "Degenerate negative arc (R=0)";
    errors += !check_extents (ctx, phase, cr2, FILL, EQUALS, 0, 0, 0, 0);
    errors += !check_extents (ctx, phase, cr2, STROKE, EQUALS, 0, 0, 0, 0);
    errors += !check_extents (ctx, phase, cr2, PATH, EQUALS, 200, 400, 0, 0);

    cairo_new_path (cr2);
    cairo_arc (cr2, 200, 400, 10., 0, 0);
    phase = "Degenerate arc (Θ=0)";
    errors += !check_extents (ctx, phase, cr2, FILL, EQUALS, 0, 0, 0, 0);
    errors += !check_extents (ctx, phase, cr2, STROKE, EQUALS, 0, 0, 0, 0);
    errors += !check_extents (ctx, phase, cr2, PATH, EQUALS, 210, 400, 0, 0);

    cairo_new_path (cr2);
    cairo_arc_negative (cr2, 200, 400, 10., 0, 0);
    phase = "Degenerate negative arc (Θ=0)";
    errors += !check_extents (ctx, phase, cr2, FILL, EQUALS, 0, 0, 0, 0);
    errors += !check_extents (ctx, phase, cr2, STROKE, EQUALS, 0, 0, 0, 0);
    errors += !check_extents (ctx, phase, cr2, PATH, EQUALS, 210, 400, 0, 0);

    cairo_new_path (cr2);
    cairo_restore (cr2);

    /* Test that with CAIRO_LINE_CAP_ROUND, we get "dots" from
     * cairo_move_to; cairo_rel_line_to(0,0) */
    cairo_save (cr2);

    cairo_set_line_cap (cr2, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_width (cr2, 20);

    cairo_move_to (cr2, 200, 400);
    cairo_rel_line_to (cr2, 0, 0);
    phase = "Single 'dot'";
    errors += !check_extents (ctx, phase, cr2, FILL, EQUALS, 0, 0, 0, 0);
    errors += !check_extents (ctx, phase, cr2, STROKE, EQUALS, 190, 390, 20, 20);
    errors += !check_extents (ctx, phase, cr2, PATH, EQUALS, 200, 400, 0, 0);

    /* Add another dot without starting a new path */
    cairo_move_to (cr2, 100, 500);
    cairo_rel_line_to (cr2, 0, 0);
    phase = "Multiple 'dots'";
    errors += !check_extents (ctx, phase, cr2, FILL, EQUALS, 0, 0, 0, 0);
    errors += !check_extents (ctx, phase, cr2, STROKE, EQUALS, 90, 390, 120, 120);
    errors += !check_extents (ctx, phase, cr2, PATH, EQUALS, 100, 400, 100, 100);

    cairo_new_path (cr2);

    cairo_restore (cr2);

    /* https://bugs.freedesktop.org/show_bug.cgi?id=7965 */
    phase = "A horizontal, open path";
    cairo_save (cr2);
    cairo_set_line_cap (cr2, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join (cr2, CAIRO_LINE_JOIN_ROUND);
    cairo_move_to (cr2, 0, 180);
    cairo_line_to (cr2, 750, 180);
    errors += !check_extents (ctx, phase, cr2, FILL, EQUALS, 0, 0, 0, 0);
    errors += !check_extents (ctx, phase, cr2, STROKE, EQUALS, -5, 175, 760, 10);
    errors += !check_extents (ctx, phase, cr2, PATH, EQUALS, 0, 180, 750, 0);
    cairo_new_path (cr2);
    cairo_restore (cr2);

    phase = "A vertical, open path";
    cairo_save (cr2);
    cairo_set_line_cap (cr2, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join (cr2, CAIRO_LINE_JOIN_ROUND);
    cairo_new_path (cr2);
    cairo_move_to (cr2, 180, 0);
    cairo_line_to (cr2, 180, 750);
    errors += !check_extents (ctx, phase, cr2, FILL, EQUALS, 0, 0, 0, 0);
    errors += !check_extents (ctx, phase, cr2, STROKE, EQUALS, 175, -5, 10, 760);
    errors += !check_extents (ctx, phase, cr2, PATH, EQUALS, 180, 0, 0, 750);
    cairo_new_path (cr2);
    cairo_restore (cr2);

    phase = "A degenerate open path";
    cairo_save (cr2);
    cairo_set_line_cap (cr2, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join (cr2, CAIRO_LINE_JOIN_ROUND);
    cairo_new_path (cr2);
    cairo_move_to (cr2, 180, 0);
    cairo_line_to (cr2, 180, 0);
    errors += !check_extents (ctx, phase, cr2, FILL, EQUALS, 0, 0, 0, 0);
    errors += !check_extents (ctx, phase, cr2, STROKE, EQUALS, 175, -5, 10, 10);
    errors += !check_extents (ctx, phase, cr2, PATH, EQUALS, 180, 0, 0, 0);
    cairo_new_path (cr2);
    cairo_restore (cr2);

    phase = "Simple rect";
    cairo_save (cr2);
    cairo_rectangle (cr2, 10, 10, 80, 80);
    errors += !check_extents (ctx, phase, cr2, FILL, EQUALS, 10, 10, 80, 80);
    errors += !check_extents (ctx, phase, cr2, STROKE, EQUALS, 5, 5, 90, 90);
    errors += !check_extents (ctx, phase, cr2, PATH, EQUALS, 10, 10, 80, 80);
    cairo_new_path (cr2);
    cairo_restore (cr2);

    phase = "Two rects";
    cairo_save (cr2);
    cairo_rectangle (cr2, 10, 10, 10, 10);
    cairo_rectangle (cr2, 20, 20, 10, 10);
    errors += !check_extents (ctx, phase, cr2, FILL, EQUALS, 10, 10, 20, 20);
    errors += !check_extents (ctx, phase, cr2, STROKE, EQUALS, 5, 5, 30, 30);
    errors += !check_extents (ctx, phase, cr2, PATH, EQUALS, 10, 10, 20, 20);
    cairo_new_path (cr2);
    cairo_restore (cr2);

    phase = "Triangle";
    cairo_save (cr2);
    cairo_move_to (cr2, 10, 10);
    cairo_line_to (cr2, 90, 90);
    cairo_line_to (cr2, 90, 10);
    cairo_close_path (cr2);
    /* miter joins protrude 5*(1+sqrt(2)) above the top-left corner and to
       the right of the bottom-right corner */
    errors += !check_extents (ctx, phase, cr2, FILL, EQUALS, 10, 10, 80, 80);
    errors += !check_extents (ctx, phase, cr2, STROKE, CONTAINS, 0, 5, 95, 95);
    errors += !check_extents (ctx, phase, cr2, PATH, CONTAINS, 10, 10, 80, 80);
    cairo_new_path (cr2);
    cairo_restore (cr2);

    cairo_save (cr2);

    cairo_set_line_width (cr2, 4);

    cairo_rectangle (cr2, 10, 10, 30, 30);
    cairo_rectangle (cr2, 25, 10, 15, 30);

    cairo_set_fill_rule (cr2, CAIRO_FILL_RULE_EVEN_ODD);
    phase = "EVEN_ODD overlapping rectangles";
    errors += !check_extents (ctx, phase, cr2, FILL, EQUALS, 10, 10, 15, 30);
    errors += !check_extents (ctx, phase, cr2, STROKE, EQUALS, 8, 8, 34, 34);
    errors += !check_extents (ctx, phase, cr2, PATH, EQUALS, 10, 10, 30, 30);

    /* Test other fill rule with the same path. */

    cairo_set_fill_rule (cr2, CAIRO_FILL_RULE_WINDING);
    phase = "WINDING overlapping rectangles";
    errors += !check_extents (ctx, phase, cr2, FILL, EQUALS, 10, 10, 30, 30);
    errors += !check_extents (ctx, phase, cr2, STROKE, EQUALS, 8, 8, 34, 34);
    errors += !check_extents (ctx, phase, cr2, PATH, EQUALS, 10, 10, 30, 30);

    /* Now, change the direction of the second rectangle and test both
     * fill rules again. */
    cairo_new_path (cr2);
    cairo_rectangle (cr2, 10, 10, 30, 30);
    cairo_rectangle (cr2, 25, 40, 15, -30);

    cairo_set_fill_rule (cr2, CAIRO_FILL_RULE_EVEN_ODD);
    phase = "EVEN_ODD overlapping rectangles";
    errors += !check_extents (ctx, phase, cr2, FILL, EQUALS, 10, 10, 15, 30);
    errors += !check_extents (ctx, phase, cr2, STROKE, EQUALS, 8, 8, 34, 34);
    errors += !check_extents (ctx, phase, cr2, PATH, EQUALS, 10, 10, 30, 30);

    /* Test other fill rule with the same path. */

    cairo_set_fill_rule (cr2, CAIRO_FILL_RULE_WINDING);
    phase = "WINDING overlapping rectangles";
    errors += !check_extents (ctx, phase, cr2, FILL, EQUALS, 10, 10, 15, 30);
    errors += !check_extents (ctx, phase, cr2, STROKE, EQUALS, 8, 8, 34, 34);
    errors += !check_extents (ctx, phase, cr2, PATH, EQUALS, 10, 10, 30, 30);

    cairo_new_path (cr2);

    cairo_restore (cr2);

    /* https://bugs.freedesktop.org/show_bug.cgi?id=7245 */
    phase = "Arc";
    cairo_save (cr2);
    cairo_arc (cr2, 250.0, 250.0, 157.0, 5.147, 3.432);
    cairo_set_line_width (cr2, 154.0);
    errors += !check_extents (ctx, phase, cr2, STROKE, APPROX_EQUALS, 16, 38, 468, 446);
    cairo_new_path (cr2);
    cairo_restore (cr2);

    phase = "Text";
    cairo_save (cr2);
    cairo_select_font_face (cr2, CAIRO_TEST_FONT_FAMILY " Sans",
			    CAIRO_FONT_SLANT_NORMAL,
			    CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size (cr2, 12);
    cairo_text_extents (cr2, string, &extents);
    /* double check that the two methods of measuring the text agree... */
    cairo_scaled_font_text_extents (cairo_get_scaled_font (cr2),
				    string,
				    &scaled_font_extents);
    if (memcmp (&extents, &scaled_font_extents, sizeof (extents))) {
	cairo_test_log (ctx, "Error: cairo_text_extents() does not match cairo_scaled_font_text_extents() - font extents (%f, %f) x (%f, %f) should be (%f, %f) x (%f, %f)\n",
		        scaled_font_extents.x_bearing,
			scaled_font_extents.y_bearing,
			scaled_font_extents.width,
			scaled_font_extents.height,
			extents.x_bearing,
			extents.y_bearing,
			extents.width,
			extents.height);
	errors++;
    }

    cairo_move_to (cr2, -extents.x_bearing, -extents.y_bearing);
    cairo_text_path (cr2, string);
    cairo_set_line_width (cr2, 2.0);
    /* XXX: We'd like to be able to use EQUALS here, but currently
     * when hinting is enabled freetype returns integer extents. See
     * https://cairographics.org/todo */
    errors += !check_extents (ctx, phase, cr2, FILL, APPROX_EQUALS,
			      0, 0, extents.width, extents.height);
    errors += !check_extents (ctx, phase, cr2, STROKE, APPROX_EQUALS,
			      -1, -1, extents.width+2, extents.height+2);
    errors += !check_extents (ctx, phase, cr2, PATH, APPROX_EQUALS,
			      0, 0, extents.width, extents.height);
    cairo_new_path (cr2);
    cairo_restore (cr2);

    phase = "User space, simple scale, getting extents with same transform";
    cairo_save (cr2);
    cairo_scale (cr2, 2, 2);
    cairo_rectangle (cr2, 5, 5, 40, 40);
    errors += !check_extents (ctx, phase, cr2, FILL, EQUALS, 5, 5, 40, 40);
    errors += !check_extents (ctx, phase, cr2, STROKE, EQUALS, 0, 0, 50, 50);
    errors += !check_extents (ctx, phase, cr2, PATH, EQUALS, 5, 5, 40, 40);
    cairo_new_path (cr2);
    cairo_restore (cr2);

    phase = "User space, simple scale, getting extents with no transform";
    cairo_save (cr2);
    cairo_save (cr2);
    cairo_scale (cr2, 2, 2);
    cairo_rectangle (cr2, 5, 5, 40, 40);
    cairo_restore (cr2);
    errors += !check_extents (ctx, phase, cr2, FILL, EQUALS, 10, 10, 80, 80);
    errors += !check_extents (ctx, phase, cr2, STROKE, EQUALS, 5, 5, 90, 90);
    errors += !check_extents (ctx, phase, cr2, PATH, EQUALS, 10, 10, 80, 80);
    cairo_new_path (cr2);
    cairo_restore (cr2);

    phase = "User space, rotation, getting extents with transform";
    cairo_save (cr2);
    cairo_rectangle (cr2, -50, -50, 50, 50);
    cairo_rotate (cr2, -M_PI/4);
    /* the path in user space is now (nearly) the square rotated by
       45 degrees about the origin. Thus its x1 and x2 are both nearly 0.
       This should show any bugs where we just transform device-space
       x1,y1 and x2,y2 to get the extents. */
    /* The largest axis-aligned square inside the rotated path has
       side lengths 50*sqrt(2), so a bit over 35 on either side of
       the axes. With the stroke width added to the rotated path,
       the largest axis-aligned square is a bit over 38 on either side of
       the axes. */
    errors += !check_extents (ctx, phase, cr2, FILL, CONTAINS, -35, -35, 35, 35);
    errors += !check_extents (ctx, phase, cr2, STROKE, CONTAINS, -38, -38, 38, 38);
    errors += !check_extents (ctx, phase, cr2, PATH, CONTAINS, -35, -35, 35, 35);
    cairo_new_path (cr2);
    cairo_restore (cr2);

    status = cairo_status (cr2);
    cairo_destroy (cr2);

    if (status)
	return cairo_test_status_from_status (ctx, status);

    return errors == 0 ? CAIRO_TEST_SUCCESS : CAIRO_TEST_FAILURE;
}

CAIRO_TEST (get_path_extents,
	    "Test cairo_fill_extents and cairo_stroke_extents",
	    "extents, path", /* keywords */
	    NULL, /* requirements */
	    0, 0,
	    NULL, draw)
