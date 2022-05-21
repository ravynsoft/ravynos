/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef __LABWC_FONT_H
#define __LABWC_FONT_H

struct server;
struct wlr_texture;
struct wlr_box;

struct font {
	char *name;
	int size;
};

/**
 * font_height - get font vertical extents
 * @font: description of font including family name and size
 */
int font_height(struct font *font);

/**
 * texture_create - Create ARGB8888 texture using pango
 * @server: context (for wlr_renderer)
 * @texture: texture pointer; existing pointer will be freed
 * @max_width: max allowable width; will be ellipsized if longer
 * @text: text to be generated as texture
 * @font: font description
 * @color: foreground color in rgba format
 */
void font_texture_create(struct server *server, struct wlr_texture **texture,
	int max_width, const char *text, struct font *font, float *color);

/**
 * font_finish - free some font related resources
 * Note: use on exit
 */
void font_finish(void);

#endif /* __LABWC_FONT_H */
