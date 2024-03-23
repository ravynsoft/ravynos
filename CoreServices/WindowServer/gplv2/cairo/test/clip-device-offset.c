/*
 * Copyright © 2009 Benjamin Otte
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
 * Authors: Benjamin Otte <otte@gnome.org>
 */

#include "cairo-test.h"

#define WIDTH 50
#define HEIGHT 50

static cairo_pattern_t *
create_green_source (void)
{
  cairo_surface_t *image;
  cairo_pattern_t *pattern;
  cairo_t *cr;

  image = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, WIDTH, HEIGHT);
  cr = cairo_create (image);
  cairo_surface_destroy (image);

  cairo_set_source_rgb (cr, 0, 1, 0);
  cairo_paint (cr);

  pattern = cairo_pattern_create_for_surface (cairo_get_target (cr));
  cairo_destroy (cr);

  return pattern;
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
  cairo_pattern_t *source;
  double old_x, old_y;

  cairo_surface_get_device_offset (cairo_get_group_target (cr), &old_x, &old_y);
  cairo_surface_set_device_offset (cairo_get_group_target (cr), old_x+5, old_y+5);

  source = create_green_source ();
  cairo_set_source (cr, source);
  cairo_pattern_destroy (source);

  cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
  cairo_rectangle (cr, 0, 0, WIDTH, HEIGHT);
  cairo_clip (cr);
  cairo_paint (cr);

  cairo_surface_set_device_offset (cairo_get_group_target (cr), old_x, old_y);

  return CAIRO_TEST_SUCCESS;
}

CAIRO_TEST (clip_device_offset,
	    "Test clipping on surfaces with device offsets",
	    "clip", /* keywords */
	    NULL, /* requirements */
	    WIDTH+10, HEIGHT+10,
	    NULL, draw)
