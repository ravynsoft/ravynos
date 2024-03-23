/*
 * Copyright 2009 Chris Wilson
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Chris Wilson not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. Chris Wilson makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * CHRIS WILSON DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL CHRIS WILSON BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Chris Wilson <chris@chris-wilson.co.uk>
 */

#include "cairo-test.h"

#include <assert.h>

static const char *png_filename = "romedalen.png";

#define WIDTH 800
#define HEIGHT 600

static cairo_status_t
_image_to_glyphs (cairo_surface_t *image,
		  int channel,
		  int level,
		  cairo_scaled_font_t *scaled_font,
		  double tx, double ty,
		  cairo_glyph_t *glyphs,
		  int *num_glyphs)
{
    int width, height, stride;
    const unsigned char *data;
    int x, y, z, n;

    width = cairo_image_surface_get_width (image);
    height = cairo_image_surface_get_height (image);
    stride = cairo_image_surface_get_stride (image);
    data = cairo_image_surface_get_data (image);

    n = 0;
    for (y = 0; y < height; y++) {
	const uint32_t *row = (uint32_t *) (data + y * stride);

	for (x = 0; x < width; x++) {
	    z = (row[x] >> channel) & 0xff;
	    if (z == level) {
		double xx, yy, zz;
		char c = n % 26 + 'a';
		int count = 1;
		cairo_glyph_t *glyphs_p = &glyphs[n];
		cairo_status_t status;

		xx = 4 * (x - width/2.) + width/2.;
		yy = 4 * (y - height/2.) + height/2.;

		zz = z / 1000.;
		xx = xx + zz*(width/2. - xx);
		yy = yy + zz*(height/2. - yy);

		cairo_scaled_font_text_to_glyphs (scaled_font,
						  tx + xx, ty + yy,
						  &c, 1,
						  &glyphs_p, &count,
						  NULL, NULL,
						  NULL);
		status = cairo_scaled_font_status (scaled_font);
		if (status)
		    return status;

		assert (glyphs_p == &glyphs[n]);
		assert (count == 1);
		n++;
	    }
	}
    }

    *num_glyphs = n;
    return CAIRO_STATUS_SUCCESS;
}

static cairo_status_t
_render_image (cairo_t *cr,
	       int width, int height,
	       cairo_surface_t *image)
{
    int ww, hh;
    cairo_glyph_t *glyphs;
    cairo_pattern_t *mask;
    cairo_scaled_font_t *scaled_font;
    double tx, ty;
    const struct {
	int shift;
	double red;
	double green;
	double blue;
    } channel[3] = {
	{  0, 0.9, 0.3, 0.4 },
	{  8, 0.4, 0.9, 0.3 },
	{ 16, 0.3, 0.4, 0.9 },
    };
    unsigned int n, i;

    ww = cairo_image_surface_get_width (image);
    hh = cairo_image_surface_get_height (image);

    glyphs = cairo_glyph_allocate (ww * hh);
    if (glyphs == NULL)
	return CAIRO_STATUS_NO_MEMORY;

    tx = (width - ww) / 2.;
    ty = (height - hh) / 2.;

    cairo_set_font_size (cr, 5);
    scaled_font = cairo_get_scaled_font (cr);

    for (i = 0; i < ARRAY_LENGTH (channel); i++) {
	cairo_push_group_with_content (cr, CAIRO_CONTENT_ALPHA);
	for (n = 0; n < 256; n++) {
	    cairo_status_t status;
	    int num_glyphs;

	    status = _image_to_glyphs (image, channel[i].shift, n,
				       scaled_font,
				       tx, ty, glyphs, &num_glyphs);
	    if (status) {
		cairo_glyph_free (glyphs);
		return status;
	    }

	    cairo_set_source_rgba (cr,
				   0, 0, 0,
				   .15 + .85 * n / 255.);
	    cairo_show_glyphs (cr, glyphs, num_glyphs);
	}
	mask = cairo_pop_group (cr);
	cairo_set_source_rgb (cr,
			      channel[i].red,
			      channel[i].green,
			      channel[i].blue);
	cairo_mask (cr, mask);
	cairo_pattern_destroy (mask);
    }

    cairo_glyph_free (glyphs);
    return CAIRO_STATUS_SUCCESS;
}

static cairo_test_status_t
draw (cairo_t *cr, int width, int height)
{
    const cairo_test_context_t *ctx = cairo_test_get_context (cr);
    cairo_surface_t *image;
    cairo_status_t status;

    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_paint (cr);

    image = cairo_test_create_surface_from_png (ctx, png_filename);
    status = cairo_surface_status (image);
    if (status)
	return cairo_test_status_from_status (ctx, status);

    status = _render_image (cr, width, height, image);
    cairo_surface_destroy (image);

    return cairo_test_status_from_status (ctx, status);
}

CAIRO_TEST (mask_glyphs,
	    "Creates a mask using a distorted array of overlapping glyphs",
	    "mask, glyphs", /* keywords */
	    "slow", /* requirements */
	    WIDTH, HEIGHT,
	    NULL, draw)
