// SPDX-License-Identifier: GPL-2.0-only
/*
 * Theme engine for labwc
 *
 * Copyright (C) Johan Malm 2020-2021
 */

#define _POSIX_C_SOURCE 200809L
#include <cairo.h>
#include <ctype.h>
#include <drm_fourcc.h>
#include <glib.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wlr/util/box.h>
#include <wlr/util/log.h>
#include <strings.h>
#include "common/dir.h"
#include "common/font.h"
#include "common/string-helpers.h"
#include "common/zfree.h"
#include "config/rcxml.h"
#include "theme.h"
#include "xbm/xbm.h"

static int
hex_to_dec(char c)
{
	if (c >= '0' && c <= '9') {
		return c - '0';
	}
	if (c >= 'a' && c <= 'f') {
		return c - 'a' + 10;
	}
	if (c >= 'A' && c <= 'F') {
		return c - 'A' + 10;
	}
	return 0;
}

/**
 * parse_hexstr - parse #rrggbb
 * @hex: hex string to be parsed
 * @rgba: pointer to float[4] for return value
 */
static void
parse_hexstr(const char *hex, float *rgba)
{
	if (!hex || hex[0] != '#' || strlen(hex) < 7) {
		return;
	}
	rgba[0] = (hex_to_dec(hex[1]) * 16 + hex_to_dec(hex[2])) / 255.0;
	rgba[1] = (hex_to_dec(hex[3]) * 16 + hex_to_dec(hex[4])) / 255.0;
	rgba[2] = (hex_to_dec(hex[5]) * 16 + hex_to_dec(hex[6])) / 255.0;
	if (strlen(hex) > 7) {
		rgba[3] = atoi(hex + 7) / 100.0;
	} else {
		rgba[3] = 1.0;
	}
}

static enum lab_justification
parse_justification(const char *str)
{
	if (!strcasecmp(str, "Center")) {
		return LAB_JUSTIFY_CENTER;
	} else if (!strcasecmp(str, "Right")) {
		return LAB_JUSTIFY_RIGHT;
	} else {
		return LAB_JUSTIFY_LEFT;
	}
}

/*
 * We generally use Openbox defaults, but if no theme file can be found it's
 * better to populate the theme variables with some sane values as no-one
 * wants to use openbox without a theme - it'll all just be black and white.
 *
 * Openbox doesn't actual start if it can't find a theme. As it's normally
 * packaged with Clearlooks, this is not a problem, but for labwc I thought
 * this was a bit hard-line. People might want to try labwc without having
 * Openbox (and associated themes) installed.
 *
 * theme_builtin() applies a theme that is similar to vanilla GTK
 */
static void
theme_builtin(struct theme *theme)
{
	theme->border_width = 1;
	theme->padding_height = 3;
	theme->menu_overlap_x = 0;
	theme->menu_overlap_y = 0;

	parse_hexstr("#dddad6", theme->window_active_border_color);
	parse_hexstr("#f6f5f4", theme->window_inactive_border_color);

	parse_hexstr("#dddad6", theme->window_active_title_bg_color);
	parse_hexstr("#f6f5f4", theme->window_inactive_title_bg_color);

	parse_hexstr("#000000", theme->window_active_label_text_color);
	parse_hexstr("#000000", theme->window_inactive_label_text_color);
	theme->window_label_text_justify = parse_justification("Left");

	parse_hexstr("#000000",
		theme->window_active_button_menu_unpressed_image_color);
	parse_hexstr("#000000",
		theme->window_active_button_iconify_unpressed_image_color);
	parse_hexstr("#000000",
		theme->window_active_button_max_unpressed_image_color);
	parse_hexstr("#000000",
		theme->window_active_button_close_unpressed_image_color);
	parse_hexstr("#000000",
		theme->window_inactive_button_menu_unpressed_image_color);
	parse_hexstr("#000000",
		theme->window_inactive_button_iconify_unpressed_image_color);
	parse_hexstr("#000000",
		theme->window_inactive_button_max_unpressed_image_color);
	parse_hexstr("#000000",
		theme->window_inactive_button_close_unpressed_image_color);

	parse_hexstr("#fcfbfa", theme->menu_items_bg_color);
	parse_hexstr("#000000", theme->menu_items_text_color);
	parse_hexstr("#dddad6", theme->menu_items_active_bg_color);
	parse_hexstr("#000000", theme->menu_items_active_text_color);

	/* inherit colors in post_processing() if not set elsewhere */
	theme->osd_bg_color[0] = FLT_MIN;
	theme->osd_label_text_color[0] = FLT_MIN;
}

static bool
match(const gchar *pattern, const gchar *string)
{
	GString *p = g_string_new(pattern);
	g_string_ascii_down(p);
	bool ret = (bool)g_pattern_match_simple(p->str, string);
	g_string_free(p, true);
	return ret;
}

static void
entry(struct theme *theme, const char *key, const char *value)
{
	if (!key || !value) {
		return;
	}

	/*
	 * Note that in order for the pattern match to apply to more than just
	 * the first instance, "else if" cannot be used throughout this function
	 */
	if (match(key, "border.width")) {
		theme->border_width = atoi(value);
	}
	if (match(key, "padding.height")) {
		theme->padding_height = atoi(value);
	}
	if (match(key, "menu.overlap.x")) {
		theme->menu_overlap_x = atoi(value);
	}
	if (match(key, "menu.overlap.y")) {
		theme->menu_overlap_y = atoi(value);
	}

	if (match(key, "window.active.border.color")) {
		parse_hexstr(value, theme->window_active_border_color);
	}
	if (match(key, "window.inactive.border.color")) {
		parse_hexstr(value, theme->window_inactive_border_color);
	}
	/* border.color is obsolete, but handled for backward compatibility */
	if (match(key, "border.color")) {
		parse_hexstr(value, theme->window_active_border_color);
		parse_hexstr(value, theme->window_inactive_border_color);
	}

	if (match(key, "window.active.title.bg.color")) {
		parse_hexstr(value, theme->window_active_title_bg_color);
	}
	if (match(key, "window.inactive.title.bg.color")) {
		parse_hexstr(value, theme->window_inactive_title_bg_color);
	}

	if (match(key, "window.active.label.text.color")) {
		parse_hexstr(value, theme->window_active_label_text_color);
	}
	if (match(key, "window.inactive.label.text.color")) {
		parse_hexstr(value, theme->window_inactive_label_text_color);
	}
	if (match(key, "window.label.text.justify")) {
		theme->window_label_text_justify = parse_justification(value);
	}

	/* universal button */
	if (match(key, "window.active.button.unpressed.image.color")) {
		parse_hexstr(value,
			theme->window_active_button_menu_unpressed_image_color);
		parse_hexstr(value,
			theme->window_active_button_iconify_unpressed_image_color);
		parse_hexstr(value,
			theme->window_active_button_max_unpressed_image_color);
		parse_hexstr(value,
			theme->window_active_button_close_unpressed_image_color);
	}
	if (match(key, "window.inactive.button.unpressed.image.color")) {
		parse_hexstr(value,
			theme->window_inactive_button_menu_unpressed_image_color);
		parse_hexstr(value,
			theme->window_inactive_button_iconify_unpressed_image_color);
		parse_hexstr(value,
			theme->window_inactive_button_max_unpressed_image_color);
		parse_hexstr(value,
			theme->window_inactive_button_close_unpressed_image_color);
	}

	/* individual buttons */
	if (match(key, "window.active.button.iconify.unpressed.image.color")) {
		parse_hexstr(value,
			theme->window_active_button_iconify_unpressed_image_color);
	}
	if (match(key, "window.active.button.max.unpressed.image.color")) {
		parse_hexstr(value,
			theme->window_active_button_max_unpressed_image_color);
	}
	if (match(key, "window.active.button.close.unpressed.image.color")) {
		parse_hexstr(value,
			theme->window_active_button_close_unpressed_image_color);
	}
	if (match(key, "window.inactive.button.iconify.unpressed.image.color")) {
		parse_hexstr(value,
			theme->window_inactive_button_iconify_unpressed_image_color);
	}
	if (match(key, "window.inactive.button.max.unpressed.image.color")) {
		parse_hexstr(value,
			theme->window_inactive_button_max_unpressed_image_color);
	}
	if (match(key, "window.inactive.button.close.unpressed.image.color")) {
		parse_hexstr(value,
			theme->window_inactive_button_close_unpressed_image_color);
	}

	if (match(key, "menu.items.bg.color")) {
		parse_hexstr(value, theme->menu_items_bg_color);
	}
	if (match(key, "menu.items.text.color")) {
		parse_hexstr(value, theme->menu_items_text_color);
	}
	if (match(key, "menu.items.active.bg.color")) {
		parse_hexstr(value, theme->menu_items_active_bg_color);
	}
	if (match(key, "menu.items.active.text.color")) {
		parse_hexstr(value, theme->menu_items_active_text_color);
	}

	if (match(key, "osd.bg.color")) {
		parse_hexstr(value, theme->osd_bg_color);
	}
	if (match(key, "osd.label.text.color")) {
		parse_hexstr(value, theme->osd_label_text_color);
	}
}

static void
parse_config_line(char *line, char **key, char **value)
{
	char *p = strchr(line, ':');
	if (!p) {
		return;
	}
	*p = '\0';
	*key = string_strip(line);
	*value = string_strip(++p);
}

static void
process_line(struct theme *theme, char *line)
{
	if (line[0] == '\0' || line[0] == '#') {
		return;
	}
	char *key = NULL, *value = NULL;
	parse_config_line(line, &key, &value);
	entry(theme, key, value);
}

static void
theme_read(struct theme *theme, const char *theme_name)
{
	FILE *stream = NULL;
	char *line = NULL;
	size_t len = 0;
	char themerc[4096];

	if (strlen(theme_dir(theme_name))) {
		snprintf(themerc, sizeof(themerc), "%s/themerc",
			 theme_dir(theme_name));
		stream = fopen(themerc, "r");
	}
	if (!stream) {
		if (theme_name) {
			wlr_log(WLR_INFO, "cannot find theme %s", theme_name);
		}
		return;
	}
	wlr_log(WLR_INFO, "read theme %s", themerc);
	while (getline(&line, &len, stream) != -1) {
		char *p = strrchr(line, '\n');
		if (p) {
			*p = '\0';
		}
		process_line(theme, line);
	}
	free(line);
	fclose(stream);
}

struct rounded_corner_ctx {
	struct wlr_box *box;
	double radius;
	double line_width;
	float *fill_color;
	float *border_color;
	enum {
		LAB_CORNER_UNKNOWN = 0,
		LAB_CORNER_TOP_LEFT,
		LAB_CORNER_TOP_RIGHT,
	} corner;
};

static void
set_source(cairo_t *cairo, float *c)
{
	cairo_set_source_rgba(cairo, c[0], c[1], c[2], c[3]);
}

static struct wlr_texture *
rounded_rect(struct wlr_renderer *renderer, struct rounded_corner_ctx *ctx)
{
	/* 1 degree in radians (=2π/360) */
	double deg = 0.017453292519943295;

	if (ctx->corner == LAB_CORNER_UNKNOWN) {
		return NULL;
	}

	double w = ctx->box->width;
	double h = ctx->box->height;
	double r = ctx->radius;

	cairo_surface_t *surf =
		cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
	cairo_t *cairo = cairo_create(surf);

	/* set transparent background */
	cairo_set_operator(cairo, CAIRO_OPERATOR_CLEAR);
	cairo_paint(cairo);

	/* fill */
	cairo_set_line_width(cairo, 0.0);
	cairo_new_sub_path(cairo);
	switch (ctx->corner) {
	case LAB_CORNER_TOP_LEFT:
		cairo_arc(cairo, r, r, r, 180 * deg, 270 * deg);
		cairo_line_to(cairo, w, 0);
		cairo_line_to(cairo, w, h);
		cairo_line_to(cairo, 0, h);
		break;
	case LAB_CORNER_TOP_RIGHT:
		cairo_arc(cairo, w - r, r, r, -90 * deg, 0 * deg);
		cairo_line_to(cairo, w, h);
		cairo_line_to(cairo, 0, h);
		cairo_line_to(cairo, 0, 0);
		break;
	default:
		wlr_log(WLR_ERROR, "unknown corner type");
	}
	cairo_close_path(cairo);
	cairo_set_operator(cairo, CAIRO_OPERATOR_SOURCE);
	set_source(cairo, ctx->fill_color);
	cairo_fill_preserve(cairo);
	cairo_stroke(cairo);

	/* border */
	cairo_set_line_cap(cairo, CAIRO_LINE_CAP_ROUND);
	set_source(cairo, ctx->border_color);
	cairo_set_line_width(cairo, ctx->line_width);
	double half_line_width = ctx->line_width / 2.0;
	switch (ctx->corner) {
	case LAB_CORNER_TOP_LEFT:
		cairo_move_to(cairo, half_line_width, h);
		cairo_line_to(cairo, half_line_width, r + half_line_width);
		cairo_arc(cairo, r, r, r - half_line_width, 180 * deg, 270 * deg);
		cairo_line_to(cairo, w, half_line_width);
		break;
	case LAB_CORNER_TOP_RIGHT:
		cairo_move_to(cairo, 0, half_line_width);
		cairo_line_to(cairo, w - r, half_line_width);
		cairo_arc(cairo, w - r, r, r - half_line_width, -90 * deg, 0 * deg);
		cairo_line_to(cairo, w - half_line_width, h);
		break;
	default:
		wlr_log(WLR_ERROR, "unknown corner type");
	}
	cairo_stroke(cairo);

	/* convert to wlr_texture */
	cairo_surface_flush(surf);
	unsigned char *data = cairo_image_surface_get_data(surf);
	struct wlr_texture *texture = wlr_texture_from_pixels(renderer,
		DRM_FORMAT_ARGB8888, cairo_image_surface_get_stride(surf),
		w, h, data);

	cairo_destroy(cairo);
	cairo_surface_destroy(surf);
	return texture;
}

static void
create_corners(struct theme *theme, struct wlr_renderer *renderer)
{
	int corner_square = theme->title_height + theme->border_width;
	struct wlr_box box = {
		.x = 0,
		.y = 0,
		.width = corner_square,
		.height = corner_square,
	};

	struct rounded_corner_ctx ctx = {
		.box = &box,
		.radius = rc.corner_radius,
		.line_width = theme->border_width,
		.fill_color = theme->window_active_title_bg_color,
		.border_color = theme->window_active_border_color,
		.corner = LAB_CORNER_TOP_LEFT,
	};
	theme->corner_top_left_active_normal = rounded_rect(renderer, &ctx);

	ctx.fill_color = theme->window_inactive_title_bg_color,
	ctx.border_color = theme->window_inactive_border_color,
	theme->corner_top_left_inactive_normal = rounded_rect(renderer, &ctx);

	ctx.corner = LAB_CORNER_TOP_RIGHT;
	ctx.fill_color = theme->window_active_title_bg_color,
	ctx.border_color = theme->window_active_border_color,
	theme->corner_top_right_active_normal = rounded_rect(renderer, &ctx);

	ctx.fill_color = theme->window_inactive_title_bg_color,
	ctx.border_color = theme->window_inactive_border_color,
	theme->corner_top_right_inactive_normal = rounded_rect(renderer, &ctx);
}

static void
post_processing(struct theme *theme)
{
	struct font font = {
		.name = rc.font_name_activewindow,
		.size = rc.font_size_activewindow,
	};
	theme->title_height = font_height(&font) + 2 * theme->padding_height;

	if (rc.corner_radius >= theme->title_height) {
		theme->title_height = rc.corner_radius + 1;
	}

	/* Inherit colors if not set */
	if (theme->osd_bg_color[0] == FLT_MIN) {
		memcpy(theme->osd_bg_color,
			theme->window_active_title_bg_color,
			sizeof(theme->osd_bg_color));
	}
	if (theme->osd_label_text_color[0] == FLT_MIN) {
		memcpy(theme->osd_label_text_color,
			theme->window_active_label_text_color,
			sizeof(theme->osd_label_text_color));
	}
}

void
theme_init(struct theme *theme, struct wlr_renderer *renderer,
		const char *theme_name)
{
	/*
	 * Set some default values. This is particularly important on
	 * reconfigure as not all themes set all options
	 */
	theme_builtin(theme);

	theme_read(theme, theme_name);
	post_processing(theme);
	create_corners(theme, renderer);
	xbm_load(theme, renderer);
}

void
theme_finish(struct theme *theme)
{
	; /* nothing to free */
}
