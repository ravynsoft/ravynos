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

static cairo_bool_t
check_count (const cairo_test_context_t *ctx,
	     const char *message,
             cairo_rectangle_list_t *list, int expected)
{
    if (list->status != CAIRO_STATUS_SUCCESS) {
        cairo_test_log (ctx, "Error: %s; cairo_copy_clip_rectangle_list failed with \"%s\"\n",
                        message, cairo_status_to_string(list->status));
        return 0;
    }

    if (list->num_rectangles == expected)
        return 1;
    cairo_test_log (ctx, "Error: %s; expected %d rectangles, got %d\n", message,
                    expected, list->num_rectangles);
    return 0;
}

static cairo_bool_t
check_unrepresentable (const cairo_test_context_t *ctx, const char *message, cairo_rectangle_list_t *list)
{
    if (list->status != CAIRO_STATUS_CLIP_NOT_REPRESENTABLE) {
        cairo_test_log (ctx, "Error: %s; cairo_copy_clip_rectangle_list got unexpected result \"%s\"\n"
                        " (we expected CAIRO_STATUS_CLIP_NOT_REPRESENTABLE)",
                        message, cairo_status_to_string(list->status));
        return 0;
    }
    return 1;
}

static cairo_bool_t
check_rectangles_contain (const cairo_test_context_t *ctx,
			  const char *message,
                          cairo_rectangle_list_t *list,
                          double x, double y, double width, double height)
{
    int i;

    for (i = 0; i < list->num_rectangles; ++i) {
        if (list->rectangles[i].x == x && list->rectangles[i].y == y &&
            list->rectangles[i].width == width && list->rectangles[i].height == height)
            return 1;
    }
    cairo_test_log (ctx, "Error: %s; rectangle list does not contain rectangle %f,%f,%f,%f\n",
                    message, x, y, width, height);
    return 0;
}

static cairo_bool_t
check_clip_extents (const cairo_test_context_t *ctx,
		    const char *message, cairo_t *cr,
                    double x, double y, double width, double height)
{
    double ext_x1, ext_y1, ext_x2, ext_y2;
    cairo_clip_extents (cr, &ext_x1, &ext_y1, &ext_x2, &ext_y2);
    if (ext_x1 == x && ext_y1 == y && ext_x2 == x + width && ext_y2 == y + height)
        return 1;
    if (width == 0.0 && height == 0.0 && ext_x1 == ext_x2 && ext_y1 == ext_y2)
        return 1;
    cairo_test_log (ctx, "Error: %s; clip extents %f,%f,%f,%f should be %f,%f,%f,%f\n",
                    message, ext_x1, ext_y1, ext_x2 - ext_x1, ext_y2 - ext_y1,
                    x, y, width, height);
    return 0;
}

#define SIZE 100

static cairo_test_status_t
preamble (cairo_test_context_t *ctx)
{
    cairo_surface_t        *surface;
    cairo_t                *cr;
    cairo_rectangle_list_t *rectangle_list;
    const char             *phase;
    cairo_bool_t            completed = 0;
    cairo_status_t          status;

    surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, SIZE, SIZE);
    cr = cairo_create (surface);
    cairo_surface_destroy (surface);


    /* first, test basic stuff. This should not be clipped, it should
       return the surface rectangle. */
    phase = "No clip set";
    rectangle_list = cairo_copy_clip_rectangle_list (cr);
    if (! check_count (ctx, phase, rectangle_list, 1) ||
        ! check_clip_extents (ctx, phase, cr, 0, 0, SIZE, SIZE) ||
        ! check_rectangles_contain (ctx, phase, rectangle_list, 0, 0, SIZE, SIZE))
    {
	goto FAIL;
    }
    cairo_rectangle_list_destroy (rectangle_list);

    /* We should get the same results after applying a clip that contains the
       existing clip. */
    phase = "Clip beyond surface extents";
    cairo_save (cr);
    cairo_rectangle (cr, -10, -10, SIZE + 20 , SIZE + 20);
    cairo_clip (cr);
    rectangle_list = cairo_copy_clip_rectangle_list (cr);
    if (! check_count (ctx, phase, rectangle_list, 1) ||
        ! check_clip_extents (ctx, phase, cr, 0, 0, SIZE, SIZE) ||
        ! check_rectangles_contain (ctx, phase, rectangle_list, 0, 0, SIZE, SIZE))
    {
	goto FAIL;
    }
    cairo_rectangle_list_destroy (rectangle_list);
    cairo_restore (cr);

    /* Test simple clip rect. */
    phase = "Simple clip rect";
    cairo_save (cr);
    cairo_rectangle (cr, 10, 10, 80, 80);
    cairo_clip (cr);
    rectangle_list = cairo_copy_clip_rectangle_list (cr);
    if (! check_count (ctx, phase, rectangle_list, 1) ||
        ! check_clip_extents (ctx, phase, cr, 10, 10, 80, 80) ||
        ! check_rectangles_contain (ctx, phase, rectangle_list, 10, 10, 80, 80))
    {
	goto FAIL;
    }
    cairo_rectangle_list_destroy (rectangle_list);
    cairo_restore (cr);

    /* Test everything clipped out. */
    phase = "All clipped out";
    cairo_save (cr);
    cairo_clip (cr);
    rectangle_list = cairo_copy_clip_rectangle_list (cr);
    if (! check_count (ctx, phase, rectangle_list, 0) ||
        ! check_clip_extents (ctx, phase, cr, 0, 0, 0, 0))
    {
	goto FAIL;
    }
    cairo_rectangle_list_destroy (rectangle_list);
    cairo_restore (cr);

    /* test two clip rects */
    phase = "Two clip rects";
    cairo_save (cr);
    cairo_rectangle (cr, 10, 10, 10, 10);
    cairo_rectangle (cr, 20, 20, 10, 10);
    cairo_clip (cr);
    cairo_rectangle (cr, 15, 15, 10, 10);
    cairo_clip (cr);
    rectangle_list = cairo_copy_clip_rectangle_list (cr);
    if (! check_count (ctx, phase, rectangle_list, 2) ||
        ! check_clip_extents (ctx, phase, cr, 15, 15, 10, 10) ||
        ! check_rectangles_contain (ctx, phase, rectangle_list, 15, 15, 5, 5) ||
        ! check_rectangles_contain (ctx, phase, rectangle_list, 20, 20, 5, 5))
    {
	goto FAIL;
    }
    cairo_rectangle_list_destroy (rectangle_list);
    cairo_restore (cr);

    /* test non-rectangular clip */
    phase = "Nonrectangular clip";
    cairo_save (cr);
    cairo_move_to (cr, 0, 0);
    cairo_line_to (cr, 100, 100);
    cairo_line_to (cr, 100, 0);
    cairo_close_path (cr);
    cairo_clip (cr);
    rectangle_list = cairo_copy_clip_rectangle_list (cr);
     /* can't get this in one tight user-space rectangle */
    if (! check_unrepresentable (ctx, phase, rectangle_list) ||
        ! check_clip_extents (ctx, phase, cr, 0, 0, 100, 100))
    {
	goto FAIL;
    }
    cairo_rectangle_list_destroy (rectangle_list);
    cairo_restore (cr);

    phase = "User space, simple scale, getting clip with same transform";
    cairo_save (cr);
    cairo_scale (cr, 2, 2);
    cairo_rectangle (cr, 5, 5, 40, 40);
    cairo_clip (cr);
    rectangle_list = cairo_copy_clip_rectangle_list (cr);
    if (! check_count (ctx, phase, rectangle_list, 1) ||
        ! check_clip_extents (ctx, phase, cr, 5, 5, 40, 40) ||
        ! check_rectangles_contain (ctx, phase, rectangle_list, 5, 5, 40, 40))
    {
	goto FAIL;
    }
    cairo_rectangle_list_destroy (rectangle_list);
    cairo_restore (cr);

    phase = "User space, simple scale, getting clip with no transform";
    cairo_save (cr);
    cairo_save (cr);
    cairo_scale (cr, 2, 2);
    cairo_rectangle (cr, 5, 5, 40, 40);
    cairo_restore (cr);
    cairo_clip (cr);
    rectangle_list = cairo_copy_clip_rectangle_list (cr);
    if (! check_count (ctx, phase, rectangle_list, 1) ||
        ! check_clip_extents (ctx, phase, cr, 10, 10, 80, 80) ||
        ! check_rectangles_contain (ctx, phase, rectangle_list, 10, 10, 80, 80))
    {
	goto FAIL;
    }
    cairo_rectangle_list_destroy (rectangle_list);
    cairo_restore (cr);

    phase = "User space, rotation, getting clip with no transform";
    cairo_save (cr);
    cairo_save (cr);
    cairo_rotate (cr, 12);
    cairo_rectangle (cr, 5, 5, 40, 40);
    cairo_restore (cr);
    cairo_clip (cr);
    rectangle_list = cairo_copy_clip_rectangle_list (cr);
    if (! check_unrepresentable (ctx, phase, rectangle_list))
	goto FAIL;

    completed = 1;
FAIL:
    cairo_rectangle_list_destroy (rectangle_list);
    status = cairo_status (cr);
    cairo_destroy (cr);

    if (!completed)
        return CAIRO_TEST_FAILURE;

    return cairo_test_status_from_status (ctx, status);
}

CAIRO_TEST (get_clip,
	    "Test cairo_copy_clip_rectangle_list and cairo_clip_extents",
	    "clip, extents", /* keywords */
	    NULL, /* requirements */
	    0, 0,
	    preamble, NULL)
