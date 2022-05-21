// SPDX-License-Identifier: GPL-2.0-only
#include <cairo.h>
#include <drm_fourcc.h>
#include <pango/pangocairo.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/util/box.h>
#include <wlr/util/log.h>
#include "common/font.h"
#include "labwc.h"

static PangoRectangle
font_extents(struct font *font, const char *string)
{
	PangoRectangle rect;
	cairo_surface_t *surface;
	cairo_t *c;
	PangoLayout *layout;

	surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1);
	c = cairo_create(surface);
	layout = pango_cairo_create_layout(c);
	PangoFontDescription *desc = pango_font_description_new();
	pango_font_description_set_family(desc, font->name);
	pango_font_description_set_size(desc, font->size * PANGO_SCALE);

	pango_layout_set_font_description(layout, desc);
	pango_layout_set_text(layout, string, -1);
	pango_layout_set_single_paragraph_mode(layout, TRUE);
	pango_layout_set_width(layout, -1);
	pango_layout_set_ellipsize(layout, PANGO_ELLIPSIZE_MIDDLE);
	pango_layout_get_extents(layout, NULL, &rect);
	pango_extents_to_pixels(&rect, NULL);

	/* we put a 2 px edge on each side - because Openbox does it :) */
	rect.width += 4;

	cairo_destroy(c);
	cairo_surface_destroy(surface);
	pango_font_description_free(desc);
	g_object_unref(layout);
	return rect;
}

int
font_height(struct font *font)
{
	PangoRectangle rectangle = font_extents(font, "abcdefg");
	return rectangle.height;
}

void
font_texture_create(struct server *server, struct wlr_texture **texture,
		int max_width, const char *text, struct font *font, float *color)
{
	if (!text || !*text) {
		return;
	}
	if (*texture) {
		wlr_texture_destroy(*texture);
		*texture = NULL;
	}

	PangoRectangle rect = font_extents(font, text);
	if (max_width && rect.width > max_width) {
		rect.width = max_width;
	}

	cairo_surface_t *surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
		rect.width, rect.height);
	cairo_t *cairo = cairo_create(surf);

	cairo_set_source_rgba(cairo, color[0], color[1], color[2], color[3]);
	cairo_move_to(cairo, 0, 0);

	PangoLayout *layout = pango_cairo_create_layout(cairo);
	pango_layout_set_width(layout, rect.width * PANGO_SCALE);
	pango_layout_set_text(layout, text, -1);
	pango_layout_set_ellipsize(layout, PANGO_ELLIPSIZE_END);

	PangoFontDescription *desc = pango_font_description_new();
	pango_font_description_set_family(desc, font->name);
	pango_font_description_set_size(desc, font->size * PANGO_SCALE);
	pango_layout_set_font_description(layout, desc);
	pango_font_description_free(desc);
	pango_cairo_update_layout(cairo, layout);

	pango_cairo_show_layout(cairo, layout);
	g_object_unref(layout);

	cairo_surface_flush(surf);
	unsigned char *data = cairo_image_surface_get_data(surf);
	*texture = wlr_texture_from_pixels(server->renderer, DRM_FORMAT_ARGB8888,
			cairo_image_surface_get_stride(surf), rect.width,
			rect.height, data);

	cairo_destroy(cairo);
	cairo_surface_destroy(surf);
}

void
font_finish(void)
{
	pango_cairo_font_map_set_default(NULL);
}
