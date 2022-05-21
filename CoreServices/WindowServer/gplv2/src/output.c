// SPDX-License-Identifier: GPL-2.0-only
/*
 * output.c: labwc output and rendering
 *
 * Copyright (C) 2019-2021 Johan Malm
 * Copyright (C) 2020 The Sway authors
 */

#define _POSIX_C_SOURCE 200809L
#include "config.h"
#include <assert.h>
#include <wlr/types/wlr_xdg_output_v1.h>
#include <wlr/types/wlr_output_damage.h>
#include <wlr/util/region.h>
#include <wlr/util/log.h>
#include "labwc.h"
#include "layers.h"
#include "menu/menu.h"
#include "ssd.h"
#include "theme.h"

#define DEBUG (0)

typedef void (*surface_iterator_func_t)(struct output *output,
		struct wlr_surface *surface, struct wlr_box *box,
		void *user_data);

struct surface_iterator_data {
	surface_iterator_func_t user_iterator;
	void *user_data;
	struct output *output;
	struct view *view;
	double ox, oy;
	int width, height;
};

static bool
intersects_with_output(struct output *output,
		struct wlr_output_layout *output_layout,
		struct wlr_box *surface_box)
{
	/* The resolution can change if outputs are rotated */
	struct wlr_box output_box = {0};
	wlr_output_effective_resolution(output->wlr_output, &output_box.width,
		&output_box.height);
	struct wlr_box intersection;
	return wlr_box_intersection(&intersection, &output_box, surface_box);
}

static void
output_for_each_surface_iterator(struct wlr_surface *surface, int sx, int sy,
		void *user_data)
{
	struct surface_iterator_data *data = user_data;
	struct output *output = data->output;
	if (!wlr_surface_has_buffer(surface)) {
		return;
	}
	struct wlr_box surface_box = {
		.x = data->ox + sx + surface->sx,
		.y = data->oy + sy + surface->sy,
		.width = surface->current.width,
		.height = surface->current.height,
	};

	if (!intersects_with_output(output, output->server->output_layout,
				    &surface_box)) {
		return;
	}
	data->user_iterator(data->output, surface, &surface_box,
		data->user_data);
}

void
output_surface_for_each_surface(struct output *output,
		struct wlr_surface *surface, double ox, double oy,
		surface_iterator_func_t iterator, void *user_data)
{
	struct surface_iterator_data data = {
		.user_iterator = iterator,
		.user_data = user_data,
		.output = output,
		.ox = ox,
		.oy = oy,
	};
	assert(surface);
	wlr_surface_for_each_surface(surface,
		output_for_each_surface_iterator, &data);
}

struct render_data {
	pixman_region32_t *damage;
};

int
scale_length(int length, int offset, float scale)
{
	return round((offset + length) * scale) - round(offset * scale);
}

void
scale_box(struct wlr_box *box, float scale)
{
	box->width = scale_length(box->width, box->x, scale);
	box->height = scale_length(box->height, box->y, scale);
	box->x = round(box->x * scale);
	box->y = round(box->y * scale);
}

static void
scissor_output(struct wlr_output *output, pixman_box32_t *rect)
{
	struct wlr_renderer *renderer = output->renderer;

	struct wlr_box box = {
		.x = rect->x1,
		.y = rect->y1,
		.width = rect->x2 - rect->x1,
		.height = rect->y2 - rect->y1,
	};

	int output_width, output_height;
	wlr_output_transformed_resolution(output, &output_width, &output_height);
	enum wl_output_transform transform =
		wlr_output_transform_invert(output->transform);
	wlr_box_transform(&box, &box, transform, output_width, output_height);

	wlr_renderer_scissor(renderer, &box);
}

static void
render_texture(struct wlr_output *wlr_output,
		pixman_region32_t *output_damage, struct wlr_texture *texture,
		const struct wlr_fbox *src_box, const struct wlr_box *dst_box,
		const float matrix[static 9])
{
	struct wlr_renderer *renderer = wlr_output->renderer;

	pixman_region32_t damage;
	pixman_region32_init(&damage);
	pixman_region32_union_rect(&damage, &damage, dst_box->x, dst_box->y,
		dst_box->width, dst_box->height);
	pixman_region32_intersect(&damage, &damage, output_damage);
	bool damaged = pixman_region32_not_empty(&damage);
	if (!damaged) {
		goto damage_finish;
	}

	int nrects;
	pixman_box32_t *rects = pixman_region32_rectangles(&damage, &nrects);
	for (int i = 0; i < nrects; i++) {
		scissor_output(wlr_output, &rects[i]);
		const float alpha = 1.0f;
		if (src_box != NULL) {
			wlr_render_subtexture_with_matrix(renderer, texture, src_box, matrix, alpha);
		} else {
			wlr_render_texture_with_matrix(renderer, texture, matrix, alpha);
		}
	}

damage_finish:
	pixman_region32_fini(&damage);
}

static void
render_surface_iterator(struct output *output, struct wlr_surface *surface,
		struct wlr_box *_box, void *user_data)
{
	struct render_data *data = user_data;
	struct wlr_output *wlr_output = output->wlr_output;
	pixman_region32_t *output_damage = data->damage;

	struct wlr_texture *texture = wlr_surface_get_texture(surface);
	if (!texture) {
		wlr_log(WLR_DEBUG, "Cannot obtain surface texture");
		return;
	}

	struct wlr_fbox src_box;
	wlr_surface_get_buffer_source_box(surface, &src_box);

	struct wlr_box proj_box = *_box;
	scale_box(&proj_box, wlr_output->scale);

	float matrix[9];
	enum wl_output_transform transform =
		wlr_output_transform_invert(surface->current.transform);
	wlr_matrix_project_box(matrix, &proj_box, transform, 0.0,
		wlr_output->transform_matrix);

	struct wlr_box dst_box = *_box;
	scale_box(&dst_box, wlr_output->scale);

	render_texture(wlr_output, output_damage, texture, &src_box, &dst_box, matrix);
}

void
output_drag_icon_for_each_surface(struct output *output, struct seat *seat,
		surface_iterator_func_t iterator, void *user_data)
{
	if (!seat->drag_icon || !seat->drag_icon->mapped) {
		return;
	}
	double ox = seat->cursor->x, oy = seat->cursor->y;
	wlr_output_layout_output_coords(output->server->output_layout,
			output->wlr_output, &ox, &oy);
	output_surface_for_each_surface(output, seat->drag_icon->surface,
			ox, oy, iterator, user_data);
}

static void
render_drag_icon(struct output *output, pixman_region32_t *damage)
{
	struct render_data data = {
		.damage = damage,
	};
	output_drag_icon_for_each_surface(output, &output->server->seat, render_surface_iterator, &data);
}

#if HAVE_XWAYLAND
void
output_unmanaged_for_each_surface(struct output *output,
		struct wl_list *unmanaged, surface_iterator_func_t iterator,
		void *user_data)
{
	struct xwayland_unmanaged *unmanaged_surface;
	wl_list_for_each(unmanaged_surface, unmanaged, link) {
		struct wlr_xwayland_surface *xsurface =
			unmanaged_surface->xwayland_surface;
		double ox = unmanaged_surface->lx, oy = unmanaged_surface->ly;
		wlr_output_layout_output_coords(
			output->server->output_layout, output->wlr_output, &ox, &oy);
		output_surface_for_each_surface(output, xsurface->surface, ox, oy,
			iterator, user_data);
	}
}

static void
render_unmanaged(struct output *output, pixman_region32_t *damage,
		struct wl_list *unmanaged)
{
	struct render_data data = {
		.damage = damage,
	};
	output_unmanaged_for_each_surface(output, unmanaged,
		render_surface_iterator, &data);
}
#endif

static void
output_view_for_each_surface(struct output *output, struct view *view,
		surface_iterator_func_t iterator, void *user_data)
{
	struct surface_iterator_data data = {
		.user_iterator = iterator,
		.user_data = user_data,
		.output = output,
		.ox = view->x,
		.oy = view->y,
	};

	wlr_output_layout_output_coords(output->server->output_layout,
		output->wlr_output, &data.ox, &data.oy);
	view_for_each_surface(view, output_for_each_surface_iterator, &data);
}

void
output_view_for_each_popup_surface(struct output *output, struct view *view,
		surface_iterator_func_t iterator, void *user_data)
{
	struct surface_iterator_data data = {
		.user_iterator = iterator,
		.user_data = user_data,
		.output = output,
		.ox = view->x,
		.oy = view->y,
	};

	wlr_output_layout_output_coords(output->server->output_layout,
		output->wlr_output, &data.ox, &data.oy);
	view_for_each_popup_surface(view, output_for_each_surface_iterator, &data);
}

/* for sending frame done */
void
output_layer_for_each_surface(struct output *output,
		struct wl_list *layer_surfaces, surface_iterator_func_t iterator,
		void *user_data)
{
	struct lab_layer_surface *layer_surface;
	wl_list_for_each(layer_surface, layer_surfaces, link) {
		struct wlr_layer_surface_v1 *wlr_layer_surface_v1 =
			layer_surface->layer_surface;
		struct wlr_surface *surface = wlr_layer_surface_v1->surface;
		struct surface_iterator_data data = {
			.user_iterator = iterator,
			.user_data = user_data,
			.output = output,
			.view = NULL,
			.ox = layer_surface->geo.x,
			.oy = layer_surface->geo.y,
			.width = surface->current.width,
			.height = surface->current.height,
		};
		wlr_layer_surface_v1_for_each_surface(wlr_layer_surface_v1,
			output_for_each_surface_iterator, &data);
	}
}

static void
output_for_each_surface(struct output *output, surface_iterator_func_t iterator,
		void *user_data)
{
	output_layer_for_each_surface(output,
		&output->layers[ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND],
		iterator, user_data);
	output_layer_for_each_surface(output,
		&output->layers[ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM],
		iterator, user_data);

	struct view *view;
	wl_list_for_each_reverse(view, &output->server->views, link) {
		if (!view->mapped) {
			continue;
		}
		output_view_for_each_surface(output, view, iterator, user_data);
	}

#if HAVE_XWAYLAND
	output_unmanaged_for_each_surface(output,
		&output->server->unmanaged_surfaces, iterator, user_data);
#endif

	output_layer_for_each_surface(output,
		&output->layers[ZWLR_LAYER_SHELL_V1_LAYER_TOP],
		iterator, user_data);
	output_layer_for_each_surface(output,
		&output->layers[ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY],
		iterator, user_data);
	output_drag_icon_for_each_surface(output, &output->server->seat, iterator, user_data);
}

struct send_frame_done_data {
	struct timespec when;
};

static void
send_frame_done_iterator(struct output *output, struct wlr_surface *surface,
		struct wlr_box *box, void *user_data)
{
	struct send_frame_done_data *data = user_data;
	wlr_surface_send_frame_done(surface, &data->when);
}

static void
send_frame_done(struct output *output, struct send_frame_done_data *data)
{
	output_for_each_surface(output, send_frame_done_iterator, data);
}

void
render_rect(struct output *output, pixman_region32_t *output_damage,
		const struct wlr_box *_box, float color[static 4])
{
	struct wlr_output *wlr_output = output->wlr_output;
	struct wlr_renderer *renderer = wlr_output->renderer;

	struct wlr_box box;
	memcpy(&box, _box, sizeof(struct wlr_box));

	double ox = 0, oy = 0;
	wlr_output_layout_output_coords(output->server->output_layout,
		output->wlr_output, &ox, &oy);
	box.x += ox;
	box.y += oy;
	scale_box(&box, wlr_output->scale);

	pixman_region32_t damage;
	pixman_region32_init(&damage);
	pixman_region32_union_rect(&damage, &damage, box.x, box.y,
		box.width, box.height);
	pixman_region32_intersect(&damage, &damage, output_damage);
	bool damaged = pixman_region32_not_empty(&damage);
	if (!damaged) {
		goto damage_finish;
	}

	int nrects;
	pixman_box32_t *rects = pixman_region32_rectangles(&damage, &nrects);
	for (int i = 0; i < nrects; ++i) {
		scissor_output(wlr_output, &rects[i]);
		wlr_render_rect(renderer, &box, color,
			wlr_output->transform_matrix);
	}

damage_finish:
	pixman_region32_fini(&damage);
}

void
render_rect_unfilled(struct output *output, pixman_region32_t *output_damage,
		const struct wlr_box *_box, float color[static 4])
{
	struct wlr_box box;
	memcpy(&box, _box, sizeof(struct wlr_box));
	box.height = 1;
	render_rect(output, output_damage, &box, color);
	box.y += _box->height - 1;
	render_rect(output, output_damage, &box, color);
	memcpy(&box, _box, sizeof(struct wlr_box));
	box.width = 1;
	render_rect(output, output_damage, &box, color);
	box.x += _box->width - 1;
	render_rect(output, output_damage, &box, color);
}

static void
shrink(struct wlr_box *box, int size)
{
	box->x += size;
	box->y += size;
	box->width -= 2 * size;
	box->height -= 2 * size;
}

static void
render_cycle_box(struct output *output, pixman_region32_t *output_damage,
		struct view *view)
{
	struct wlr_box box = {
		.x = view->x,
		.y = view->y,
		.width = view->w,
		.height = view->h,
	};
	box.x -= view->margin.left;
	box.y -= view->margin.top;
	box.width += view->margin.left + view->margin.right;
	box.height += view->margin.top + view->margin.bottom;
	box.x += view->padding.left;
	box.y += view->padding.top;

	float white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float black[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	render_rect_unfilled(output, output_damage, &box, white);

	for (int i = 0; i < 4; i++) {
		shrink(&box, 1);
		render_rect_unfilled(output, output_damage, &box, black);
	}
	shrink(&box, 1);
	render_rect_unfilled(output, output_damage, &box, white);
}

static void
render_icon(struct output *output, pixman_region32_t *output_damage,
		struct wlr_box *box, struct wlr_texture *texture)
{
	/* centre-align icon if smaller than designated box */
	struct wlr_box button = {
		.width = texture->width,
		.height = texture->height,
	};
	if (box->width > button.width) {
		button.x = box->x + (box->width - button.width) / 2;
	} else {
		button.x = box->x;
		button.width = box->width;
	}
	if (box->height > button.height) {
		button.y = box->y + (box->height - button.height) / 2;
	} else {
		button.y = box->y;
		button.height = box->height;
	}

	double ox = 0, oy = 0;
	wlr_output_layout_output_coords(output->server->output_layout,
		output->wlr_output, &ox, &oy);
	button.x += ox;
	button.y += oy;
	scale_box(&button, output->wlr_output->scale);

	float matrix[9];
	wlr_matrix_project_box(matrix, &button, WL_OUTPUT_TRANSFORM_NORMAL, 0,
		output->wlr_output->transform_matrix);
	render_texture(output->wlr_output, output_damage, texture, NULL, &button,
		matrix);
}

void
render_texture_helper(struct output *output, pixman_region32_t *output_damage,
		struct wlr_box *_box, struct wlr_texture *texture)
{
	if (!texture) {
		return;
	}
	struct wlr_box box;
	memcpy(&box, _box, sizeof(struct wlr_box));

	double ox = 0, oy = 0;
	wlr_output_layout_output_coords(output->server->output_layout,
		output->wlr_output, &ox, &oy);
	box.x += ox;
	box.y += oy;
	scale_box(&box, output->wlr_output->scale);

	float matrix[9];
	wlr_matrix_project_box(matrix, &box, WL_OUTPUT_TRANSFORM_NORMAL, 0,
		output->wlr_output->transform_matrix);
	render_texture(output->wlr_output, output_damage, texture, NULL, &box,
		matrix);
}

static void
render_osd(struct output *output, pixman_region32_t *damage,
		struct server *server)
{
	/* show on screen display (osd) on all outputs */
	struct output *o;
	struct wlr_output_layout *layout = server->output_layout;
	struct wlr_output_layout_output *ol_output;
	wl_list_for_each(o, &server->outputs, link) {
		ol_output = wlr_output_layout_get(layout, o->wlr_output);
		if (!ol_output) {
			continue;
		}

		if (!output->osd) {
			continue;
		}
		struct wlr_box box = {
			.x = ol_output->x + o->wlr_output->width
				/ 2,
			.y = ol_output->y + o->wlr_output->height
				/ 2,
			.width = output->osd->width,
			.height = output->osd->height,
		};
		box.x -= box.width  / 2;
		box.y -= box.height / 2;

		double ox = 0, oy = 0;
		wlr_output_layout_output_coords(output->server->output_layout,
			output->wlr_output, &ox, &oy);
		box.x += ox;
		box.y += oy;
		float matrix[9];
		wlr_matrix_project_box(matrix, &box, WL_OUTPUT_TRANSFORM_NORMAL,
			0, output->wlr_output->transform_matrix);
		render_texture(o->wlr_output, damage, output->osd, NULL, &box,
			matrix);
	}
}

static void
render_deco(struct view *view, struct output *output,
		pixman_region32_t *output_damage)
{
	if (!view->ssd.enabled || view->fullscreen) {
		return;
	}

	struct wlr_seat *seat = view->server->seat.seat;
	bool focused = view->surface == seat->keyboard_state.focused_surface;

	/* render texture or rectangle */
	struct ssd_part *part;
	wl_list_for_each_reverse(part, &view->ssd.parts, link) {
		if (part->texture.active && *(part->texture.active)) {
			struct wlr_texture *texture = focused ?
				*(part->texture.active) :
				*(part->texture.inactive);
			render_texture_helper(output, output_damage, &part->box,
					      texture);
		} else if (part->color.active && part->color.inactive) {
			float *color = focused ?
				part->color.active :
				part->color.inactive;
			render_rect(output, output_damage, &part->box, color);
		}
	}

	/* button background */
	struct wlr_cursor *cur = view->server->seat.cursor;
	enum ssd_part_type type = ssd_at(view, cur->x, cur->y);
	struct wlr_box box = ssd_visible_box(view, type);
	if (ssd_is_button(type) &&
			wlr_box_contains_point(&box, cur->x, cur->y)) {
		float *color = (float[4]) { 0.5, 0.5, 0.5, 0.5 };
		render_rect(output, output_damage, &box, color);
	}

	/* buttons */
	struct theme *theme = view->server->theme;
	if (view->surface == seat->keyboard_state.focused_surface) {
		box = ssd_visible_box(view, LAB_SSD_BUTTON_CLOSE);
		render_icon(output, output_damage, &box,
			theme->xbm_close_active_unpressed);
		box = ssd_visible_box(view, LAB_SSD_BUTTON_MAXIMIZE);
		render_icon(output, output_damage, &box,
			theme->xbm_maximize_active_unpressed);
		box = ssd_visible_box(view, LAB_SSD_BUTTON_ICONIFY);
		render_icon(output, output_damage, &box,
			theme->xbm_iconify_active_unpressed);
		box = ssd_visible_box(view, LAB_SSD_BUTTON_WINDOW_MENU);
		render_icon(output, output_damage, &box,
			theme->xbm_menu_active_unpressed);
	} else {
		box = ssd_visible_box(view, LAB_SSD_BUTTON_CLOSE);
		render_icon(output, output_damage, &box,
			theme->xbm_close_inactive_unpressed);
		box = ssd_visible_box(view, LAB_SSD_BUTTON_MAXIMIZE);
		render_icon(output, output_damage, &box,
			theme->xbm_maximize_inactive_unpressed);
		box = ssd_visible_box(view, LAB_SSD_BUTTON_ICONIFY);
		render_icon(output, output_damage, &box,
			theme->xbm_iconify_inactive_unpressed);
		box = ssd_visible_box(view, LAB_SSD_BUTTON_WINDOW_MENU);
		render_icon(output, output_damage, &box,
			theme->xbm_menu_inactive_unpressed);
	}
}

static void
render_menu(struct output *output, pixman_region32_t *damage, struct menu *menu)
{
	if (!menu->visible) {
		return;
	}
	struct server *server = output->server;
	struct theme *theme = server->theme;
	float matrix[9];

	struct wlr_output_layout *output_layout = server->output_layout;
	double ox = 0, oy = 0;
	wlr_output_layout_output_coords(output_layout, output->wlr_output,
		&ox, &oy);

	/* background */
	render_rect(output, damage, &menu->box, theme->menu_items_bg_color);

	/* items */
	struct menuitem *menuitem;
	wl_list_for_each (menuitem, &menu->menuitems, link) {
		struct wlr_box box = {
			.x = menuitem->box.x + menuitem->texture.offset_x + ox,
			.y = menuitem->box.y + menuitem->texture.offset_y + oy,
			.width = menuitem->texture.active->width,
			.height = menuitem->texture.active->height,
		};
		scale_box(&box, output->wlr_output->scale);
		wlr_matrix_project_box(matrix, &box, WL_OUTPUT_TRANSFORM_NORMAL,
			0, output->wlr_output->transform_matrix);
		if (menuitem->selected) {
			render_rect(output, damage, &menuitem->box,
				theme->menu_items_active_bg_color);
			render_texture(output->wlr_output, damage,
				menuitem->texture.active, NULL, &box, matrix);
		} else {
			render_texture(output->wlr_output, damage,
				menuitem->texture.inactive, NULL, &box, matrix);
		}
		/* render submenus */
		if (menuitem->submenu) {
			render_menu(output, damage, menuitem->submenu);
		}
	}
}

static void
output_layer_for_each_popup_surface(struct output *output,
		struct wl_list *layer_surfaces,
		surface_iterator_func_t iterator,
		void *user_data)
{
	struct lab_layer_surface *layer_surface;
	wl_list_for_each(layer_surface, layer_surfaces, link) {
		struct wlr_layer_surface_v1 *wlr_layer_surface_v1 =
			layer_surface->layer_surface;
		struct wlr_surface *surface = wlr_layer_surface_v1->surface;
		struct surface_iterator_data data = {
			.user_iterator = iterator,
			.user_data = user_data,
			.output = output,
			.view = NULL,
			.ox = layer_surface->geo.x,
			.oy = layer_surface->geo.y,
			.width = surface->current.width,
			.height = surface->current.height,
		};
		wlr_layer_surface_v1_for_each_popup_surface(wlr_layer_surface_v1,
			output_for_each_surface_iterator, &data);
	}
}

static void
render_layer_popups(struct output *output, pixman_region32_t *damage,
		struct wl_list *layer_surfaces)
{
	struct render_data data = {
		.damage = damage,
	};
	output_layer_for_each_popup_surface(output, layer_surfaces,
		render_surface_iterator, &data);
}

void
output_layer_for_each_surface_toplevel(struct output *output,
		struct wl_list *layer_surfaces, surface_iterator_func_t iterator,
		void *user_data)
{
	struct lab_layer_surface *layer_surface;
	wl_list_for_each(layer_surface, layer_surfaces, link) {
		struct wlr_layer_surface_v1 *wlr_layer_surface_v1 =
			layer_surface->layer_surface;
		output_surface_for_each_surface(output,
			wlr_layer_surface_v1->surface, layer_surface->geo.x,
			layer_surface->geo.y, iterator, user_data);
	}
}

static void
render_layer_toplevel(struct output *output, pixman_region32_t *damage,
		struct wl_list *layer_surfaces)
{
	struct render_data data = {
		.damage = damage,
	};
	output_layer_for_each_surface_toplevel(output, layer_surfaces,
		render_surface_iterator, &data);
}

static void
render_view_toplevels(struct view *view, struct output *output,
		pixman_region32_t *damage)
{
	struct render_data data = {
		.damage = damage,
	};
	double ox = view->x;
	double oy = view->y;
	wlr_output_layout_output_coords(output->server->output_layout,
		output->wlr_output, &ox, &oy);
	output_surface_for_each_surface(output, view->surface, ox, oy,
		render_surface_iterator, &data);
}

static void
render_view_popups(struct view *view, struct output *output,
		pixman_region32_t *damage)
{
	struct render_data data = {
		.damage = damage,
	};
	output_view_for_each_popup_surface(output, view,
		render_surface_iterator, &data);
}

void
output_render(struct output *output, pixman_region32_t *damage)
{
	struct server *server = output->server;
	struct wlr_output *wlr_output = output->wlr_output;

	struct wlr_renderer *renderer = wlr_output->renderer;
	if (!renderer) {
		wlr_log(WLR_DEBUG, "no renderer");
		return;
	}

	/* Calls glViewport and some other GL sanity checks */
	wlr_renderer_begin(renderer, wlr_output->width, wlr_output->height);

	if (!pixman_region32_not_empty(damage)) {
		goto renderer_end;
	}

#if DEBUG
	wlr_renderer_clear(renderer, (float[]){0.2f, 0.0f, 0.0f, 1.0f});
#endif

	float color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
	int nrects;
	pixman_box32_t *rects = pixman_region32_rectangles(damage, &nrects);
	for (int i = 0; i < nrects; i++) {
		scissor_output(wlr_output, &rects[i]);
		wlr_renderer_clear(renderer, color);
	}

	render_layer_toplevel(output, damage,
		&output->layers[ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND]);
	render_layer_toplevel(output, damage,
		&output->layers[ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM]);

	struct view *view;
	wl_list_for_each_reverse (view, &server->views, link) {
		if (!view->mapped) {
			continue;
		}
		render_deco(view, output, damage);
		render_view_toplevels(view, output, damage);
		render_view_popups(view, output, damage);
	}

#if HAVE_XWAYLAND
	render_unmanaged(output, damage, &output->server->unmanaged_surfaces);
#endif

	/* 'alt-tab' border */
	if (output->server->cycle_view) {
		/* If the 'cycle_preview_contents' option is set in
		 * rc.xml, render the contents of the cycle_view over
		 * all other views (except for the OSD)
		 */
		if (rc.cycle_preview_contents) {
			render_deco(output->server->cycle_view, output, damage);
			render_view_toplevels(output->server->cycle_view, output, damage);
			render_view_popups(output->server->cycle_view, output, damage);
		}

		render_cycle_box(output, damage, output->server->cycle_view);
		render_osd(output, damage, output->server);
	}

	render_drag_icon(output, damage);

	render_layer_toplevel(output, damage,
		&output->layers[ZWLR_LAYER_SHELL_V1_LAYER_TOP]);
	render_layer_popups(output, damage,
		&output->layers[ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND]);
	render_layer_popups(output, damage,
		&output->layers[ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM]);
	render_layer_popups(output, damage,
		&output->layers[ZWLR_LAYER_SHELL_V1_LAYER_TOP]);

	render_layer_toplevel(output, damage,
		&output->layers[ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY]);
	render_layer_popups(output, damage,
		&output->layers[ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY]);

	if (output->server->input_mode == LAB_INPUT_STATE_MENU) {
		render_menu(output, damage, server->rootmenu);
		render_menu(output, damage, server->windowmenu);
	}

renderer_end:
	/* Just in case hardware cursors not supported by GPU */
	wlr_output_render_software_cursors(wlr_output, damage);
	wlr_renderer_scissor(renderer, NULL);
	wlr_renderer_end(renderer);

	int output_width, output_height;
	wlr_output_transformed_resolution(wlr_output, &output_width,
		&output_height);

	pixman_region32_t frame_damage;
	pixman_region32_init(&frame_damage);

	enum wl_output_transform transform =
		wlr_output_transform_invert(wlr_output->transform);
	wlr_region_transform(&frame_damage, &output->damage->current,
		transform, output_width, output_height);

#if DEBUG
	pixman_region32_union_rect(&frame_damage, &frame_damage, 0, 0,
		output_width, output_height);
#endif

	wlr_output_set_damage(wlr_output, &frame_damage);
	pixman_region32_fini(&frame_damage);

	if (!wlr_output_commit(wlr_output)) {
		wlr_log(WLR_ERROR, "could not commit output");
	}
}

static void
damage_surface_iterator(struct output *output, struct wlr_surface *surface,
		struct wlr_box *box, void *user_data)
{
	struct wlr_output *wlr_output = output->wlr_output;
	bool whole = *(bool *) user_data;

	scale_box(box, output->wlr_output->scale);

	if (whole) {
		wlr_output_damage_add_box(output->damage, box);
	} else if (pixman_region32_not_empty(&surface->buffer_damage)) {
		pixman_region32_t damage;
		pixman_region32_init(&damage);
		wlr_surface_get_effective_damage(surface, &damage);

		wlr_region_scale(&damage, &damage, wlr_output->scale);
		if (ceil(wlr_output->scale) > surface->current.scale) {
			wlr_region_expand(&damage, &damage,
				ceil(wlr_output->scale) - surface->current.scale);
		}
		pixman_region32_translate(&damage, box->x, box->y);
		wlr_output_damage_add(output->damage, &damage);
		pixman_region32_fini(&damage);
	}
}

void
output_damage_surface(struct output *output, struct wlr_surface *surface,
		double lx, double ly, bool whole)
{
	if (!output->wlr_output->enabled) {
		return;
	}

	double ox = lx, oy = ly;
	wlr_output_layout_output_coords(output->server->output_layout,
		output->wlr_output, &ox, &oy);
	output_surface_for_each_surface(output, surface, ox, oy,
		damage_surface_iterator, &whole);
}

static void
output_damage_frame_notify(struct wl_listener *listener, void *data)
{
	struct output *output = wl_container_of(listener, output, damage_frame);

	if (!output->wlr_output->enabled) {
		return;
	}

	bool needs_frame;
	pixman_region32_t damage;
	pixman_region32_init(&damage);
	if (!wlr_output_damage_attach_render(output->damage,
			&needs_frame, &damage)) {
		return;
	}

	if (needs_frame) {
		output_render(output, &damage);
	} else {
		wlr_output_rollback(output->wlr_output);
	}
	pixman_region32_fini(&damage);

	struct send_frame_done_data frame_data = {0};
	clock_gettime(CLOCK_MONOTONIC, &frame_data.when);
	send_frame_done(output, &frame_data);
}

static void
output_damage_destroy_notify(struct wl_listener *listener, void *data)
{
	struct output *output = wl_container_of(listener, output, damage_destroy);
	wl_list_remove(&output->damage_frame.link);
	wl_list_remove(&output->damage_destroy.link);
}

static void
output_destroy_notify(struct wl_listener *listener, void *data)
{
	struct output *output = wl_container_of(listener, output, destroy);
	wl_list_remove(&output->link);
	wl_list_remove(&output->destroy.link);
}

static void
new_output_notify(struct wl_listener *listener, void *data)
{
	/*
	 * This event is rasied by the backend when a new output (aka display
	 * or monitor) becomes available.
	 */
	struct server *server = wl_container_of(listener, server, new_output);
	struct wlr_output *wlr_output = data;

	/*
	 * Configures the output created by the backend to use our allocator
	 * and our renderer. Must be done once, before commiting the output
	 */
	if (!wlr_output_init_render(wlr_output, server->allocator,
			server->renderer)) {
		wlr_log(WLR_ERROR, "unable to init output renderer");
		return;
	}

	wlr_log(WLR_DEBUG, "enable output");
	wlr_output_enable(wlr_output, true);

	/* The mode is a tuple of (width, height, refresh rate). */
	wlr_log(WLR_DEBUG, "set preferred mode");
	struct wlr_output_mode *preferred_mode =
		wlr_output_preferred_mode(wlr_output);
	wlr_output_set_mode(wlr_output, preferred_mode);

	/*
	 * Sometimes the preferred mode is not available due to hardware
	 * constraints (e.g. GPU or cable bandwidth limitations). In these
	 * cases it's better to fallback to lower modes than to end up with
	 * a black screen. See sway@4cdc4ac6
	 */
	if (!wlr_output_test(wlr_output)) {
		wlr_log(WLR_DEBUG,
			"preferred mode rejected, falling back to another mode");
		struct wlr_output_mode *mode;
		wl_list_for_each(mode, &wlr_output->modes, link) {
			if (mode == preferred_mode) {
				continue;
			}
			wlr_output_set_mode(wlr_output, mode);
			if (wlr_output_test(wlr_output)) {
				break;
			}
		}
	}

	wlr_output_commit(wlr_output);

	struct output *output = calloc(1, sizeof(struct output));
	output->wlr_output = wlr_output;
	wlr_output->data = output;
	output->server = server;
	output->damage = wlr_output_damage_create(wlr_output);
	wlr_output_effective_resolution(wlr_output,
		&output->usable_area.width, &output->usable_area.height);
	wl_list_insert(&server->outputs, &output->link);

	output->destroy.notify = output_destroy_notify;
	wl_signal_add(&wlr_output->events.destroy, &output->destroy);

	output->damage_frame.notify = output_damage_frame_notify;
	wl_signal_add(&output->damage->events.frame, &output->damage_frame);
	output->damage_destroy.notify = output_damage_destroy_notify;
	wl_signal_add(&output->damage->events.destroy, &output->damage_destroy);

	wl_list_init(&output->layers[0]);
	wl_list_init(&output->layers[1]);
	wl_list_init(&output->layers[2]);
	wl_list_init(&output->layers[3]);
	/*
	 * Arrange outputs from left-to-right in the order they appear.
	 * TODO: support configuration in run-time
	 */

	if (rc.adaptive_sync) {
		wlr_log(WLR_INFO, "enable adaptive sync on %s",
			wlr_output->name);
		wlr_output_enable_adaptive_sync(wlr_output, true);
	}

	output->osd = NULL;

	wlr_output_layout_add_auto(server->output_layout, wlr_output);
}

void
output_init(struct server *server)
{
	server->new_output.notify = new_output_notify;
	wl_signal_add(&server->backend->events.new_output, &server->new_output);

	/*
	 * Create an output layout, which is a wlroots utility for working with
	 * an arrangement of screens in a physical layout.
	 */
	server->output_layout = wlr_output_layout_create();
	if (!server->output_layout) {
		wlr_log(WLR_ERROR, "unable to create output layout");
		exit(EXIT_FAILURE);
	}

	/* Enable screen recording with wf-recorder */
	wlr_xdg_output_manager_v1_create(server->wl_display,
		server->output_layout);

	wl_list_init(&server->outputs);

	output_manager_init(server);
}

static void
output_update_for_layout_change(struct server *server)
{
	/* Adjust window positions/sizes */
	struct view *view;
	wl_list_for_each(view, &server->views, link) {
		view_adjust_for_layout_change(view);
	}

	/*
	 * "Move" each wlr_output_cursor (in per-output coordinates) to
	 * align with the seat cursor.  Set a default cursor image so
	 * that the cursor isn't invisible on new outputs.
	 *
	 * TODO: remember the most recent cursor image (see cursor.c)
	 * and set that rather than XCURSOR_DEFAULT
	 */
	wlr_cursor_move(server->seat.cursor, NULL, 0, 0);
	wlr_xcursor_manager_set_cursor_image(server->seat.xcursor_manager,
		XCURSOR_DEFAULT, server->seat.cursor);

	/* Redraw everything */
	damage_all_outputs(server);
}

static void
output_config_apply(struct server *server,
		struct wlr_output_configuration_v1 *config)
{
	server->pending_output_config = config;

	struct wlr_output_configuration_head_v1 *head;
	wl_list_for_each(head, &config->heads, link) {
		struct wlr_output *o = head->state.output;
		bool need_to_add = head->state.enabled && !o->enabled;
		if (need_to_add) {
			wlr_output_layout_add_auto(server->output_layout, o);
		}

		bool need_to_remove = !head->state.enabled && o->enabled;
		if (need_to_remove) {
			wlr_output_layout_remove(server->output_layout, o);
		}

		wlr_output_enable(o, head->state.enabled);
		if (head->state.enabled) {
			if (head->state.mode) {
				wlr_output_set_mode(o, head->state.mode);
			} else {
				int32_t width = head->state.custom_mode.width;
				int32_t height = head->state.custom_mode.height;
				int32_t refresh = head->state.custom_mode.refresh;
				wlr_output_set_custom_mode(o, width,
					height, refresh);
			}
			wlr_output_layout_move(server->output_layout, o,
				head->state.x, head->state.y);
			wlr_output_set_scale(o, head->state.scale);
			wlr_output_set_transform(o, head->state.transform);
		}
		wlr_output_commit(o);
	}

	server->pending_output_config = NULL;
	output_update_for_layout_change(server);
}

static bool
verify_output_config_v1(const struct wlr_output_configuration_v1 *config)
{
	/* TODO implement */
	return true;
}

static void
handle_output_manager_apply(struct wl_listener *listener, void *data)
{
	struct server *server =
		wl_container_of(listener, server, output_manager_apply);
	struct wlr_output_configuration_v1 *config = data;

	bool config_is_good = verify_output_config_v1(config);

	if (config_is_good) {
		output_config_apply(server, config);
		wlr_output_configuration_v1_send_succeeded(config);
	} else {
		wlr_output_configuration_v1_send_failed(config);
	}
	wlr_output_configuration_v1_destroy(config);
	struct output *output;
	wl_list_for_each(output, &server->outputs, link) {
		wlr_xcursor_manager_load(server->seat.xcursor_manager,
			output->wlr_output->scale);
	}
}

/*
 * Take the way outputs are currently configured/layed out and turn that into
 * a struct that we send to clients via the wlr_output_configuration v1
 * interface
 */
static struct
wlr_output_configuration_v1 *create_output_config(struct server *server)
{
	struct wlr_output_configuration_v1 *config =
		wlr_output_configuration_v1_create();
	if (!config) {
		wlr_log(WLR_ERROR, "wlr_output_configuration_v1_create()");
		return NULL;
	}

	struct output *output;
	wl_list_for_each(output, &server->outputs, link) {
		struct wlr_output_configuration_head_v1 *head =
			wlr_output_configuration_head_v1_create(config,
				output->wlr_output);
		if (!head) {
			wlr_log(WLR_ERROR,
				"wlr_output_configuration_head_v1_create()");
			wlr_output_configuration_v1_destroy(config);
			return NULL;
		}
		struct wlr_box *box =
			wlr_output_layout_get_box(server->output_layout,
				output->wlr_output);
		if (box) {
			head->state.x = box->x;
			head->state.y = box->y;
		} else {
			wlr_log(WLR_ERROR, "failed to get output layout box");
		}
	}
	return config;
}

static void
handle_output_layout_change(struct wl_listener *listener, void *data)
{
	struct server *server =
		wl_container_of(listener, server, output_layout_change);

	bool done_changing = server->pending_output_config == NULL;
	if (done_changing) {
		struct wlr_output_configuration_v1 *config =
			create_output_config(server);
		if (config) {
			wlr_output_manager_v1_set_configuration(
				server->output_manager, config);
		} else {
			wlr_log(WLR_ERROR,
				"wlr_output_manager_v1_set_configuration()");
		}
		struct output *output;
		wl_list_for_each(output, &server->outputs, link) {
			if (output) {
				arrange_layers(output);
			}
		}
		output_update_for_layout_change(server);
	}
}

void
output_manager_init(struct server *server)
{
	server->output_manager = wlr_output_manager_v1_create(server->wl_display);

	server->output_layout_change.notify = handle_output_layout_change;
	wl_signal_add(&server->output_layout->events.change,
		&server->output_layout_change);

	server->output_manager_apply.notify = handle_output_manager_apply;
	wl_signal_add(&server->output_manager->events.apply,
		&server->output_manager_apply);
}

struct output *
output_from_wlr_output(struct server *server, struct wlr_output *wlr_output)
{
	struct output *output;
	wl_list_for_each (output, &server->outputs, link) {
		if (output->wlr_output == wlr_output) {
			return output;
		}
	}
	return NULL;
}

struct wlr_box
output_usable_area_in_layout_coords(struct output *output)
{
	struct wlr_box box = output->usable_area;
	double ox = 0, oy = 0;
	wlr_output_layout_output_coords(output->server->output_layout,
		output->wlr_output, &ox, &oy);
	box.x -= ox;
	box.y -= oy;
	return box;
}

struct wlr_box
output_usable_area_from_cursor_coords(struct server *server)
{
	struct wlr_output *wlr_output;
	wlr_output = wlr_output_layout_output_at(server->output_layout,
		server->seat.cursor->x, server->seat.cursor->y);
	struct output *output = output_from_wlr_output(server, wlr_output);
	return output_usable_area_in_layout_coords(output);
}
