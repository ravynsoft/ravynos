/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef __LABWC_SSD_H
#define __LABWC_SSD_H

/*
 * Sequence these according to the order they should be processed for
 * press and hover events. Bear in mind that some of their respective
 * interactive areas overlap, so for example buttons need to come before title.
 */
enum ssd_part_type {
	LAB_SSD_NONE = 0,
	LAB_SSD_BUTTON_CLOSE,
	LAB_SSD_BUTTON_MAXIMIZE,
	LAB_SSD_BUTTON_ICONIFY,
	LAB_SSD_BUTTON_WINDOW_MENU,
	LAB_SSD_PART_TITLEBAR,
	LAB_SSD_PART_TITLE,
	LAB_SSD_PART_CORNER_TOP_LEFT,
	LAB_SSD_PART_CORNER_TOP_RIGHT,
	LAB_SSD_PART_CORNER_BOTTOM_RIGHT,
	LAB_SSD_PART_CORNER_BOTTOM_LEFT,
	LAB_SSD_PART_TOP,
	LAB_SSD_PART_RIGHT,
	LAB_SSD_PART_BOTTOM,
	LAB_SSD_PART_LEFT,
	LAB_SSD_CLIENT,
	LAB_SSD_FRAME,
	LAB_SSD_ROOT,
	LAB_SSD_END_MARKER
};

/*
 * Defer including labwc.h because it is using enum ssd_part_type.
 * This is an issue for headers like mousebind.h which only includes
 * ssd.h but does not include labwc.h.
 */
#include "labwc.h"

struct ssd_part {
	struct wlr_box box;
	enum ssd_part_type type;

	/*
	 * The texture pointers are often held in other places such as the
	 * theme struct, so here we use ** in order to keep the code
	 * simple and avoid updating pointers as textures change.
	 */
	struct {
		struct wlr_texture **active;
		struct wlr_texture **inactive;
	} texture;

	/*
	 * If a part does not contain textures, it'll just be rendered as a
	 * rectangle with the following colors.
	 */
	struct {
		float *active;
		float *inactive;
	} color;

	struct wl_list link;
};

struct view;

struct border ssd_thickness(struct view *view);
struct wlr_box ssd_max_extents(struct view *view);
struct wlr_box ssd_visible_box(struct view *view, enum ssd_part_type type);
enum ssd_part_type ssd_at(struct view *view, double lx, double ly);
uint32_t ssd_resize_edges(enum ssd_part_type type);
void ssd_update_title(struct view *view);
void ssd_create(struct view *view);
void ssd_destroy(struct view *view);
void ssd_update_geometry(struct view *view, bool force);
bool ssd_part_contains(enum ssd_part_type whole, enum ssd_part_type candidate);
bool ssd_is_button(enum ssd_part_type type);

#endif /* __LABWC_SSD_H */
