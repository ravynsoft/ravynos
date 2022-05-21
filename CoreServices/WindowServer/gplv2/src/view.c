// SPDX-License-Identifier: GPL-2.0-only
#include <stdio.h>
#include <strings.h>
#include "labwc.h"
#include "ssd.h"

void
view_set_activated(struct view *view, bool activated)
{
	if (view->impl->set_activated) {
		view->impl->set_activated(view, activated);
	}
	if (view->toplevel_handle) {
		wlr_foreign_toplevel_handle_v1_set_activated(
			view->toplevel_handle, activated);
	}
}

void
view_close(struct view *view)
{
	if (view->impl->close) {
		view->impl->close(view);
	}
}

void
view_move(struct view *view, double x, double y)
{
	if (view->impl->move) {
		view->impl->move(view, x, y);
	}
	view_discover_output(view);
}

void
view_move_resize(struct view *view, struct wlr_box geo)
{
	if (view->w == geo.width && view->h == geo.height) {
		wlr_log(WLR_ERROR, "use view_move() if not resizing");
	}
	if (view->impl->configure) {
		view->impl->configure(view, geo);
	}
	ssd_update_title(view);
	view_discover_output(view);
}

#define MIN_VIEW_WIDTH (100)
#define MIN_VIEW_HEIGHT (60)

void
view_min_size(struct view *view, int *w, int *h)
{
	int min_width = MIN_VIEW_WIDTH;
	int min_height = MIN_VIEW_HEIGHT;
#if HAVE_XWAYLAND
	if (view->type != LAB_XWAYLAND_VIEW) {
		goto out;
	}
	if (!view->xwayland_surface->size_hints) {
		goto out;
	}
	if (view->xwayland_surface->size_hints->min_width > 0
			|| view->xwayland_surface->size_hints->min_height > 0) {
		min_width = view->xwayland_surface->size_hints->min_width;
		min_height = view->xwayland_surface->size_hints->min_height;
	}
out:
#endif

	if (w) {
		*w = min_width;
	}
	if (h) {
		*h = min_height;
	}
}

void
view_minimize(struct view *view, bool minimized)
{
	if (view->minimized == minimized) {
		return;
	}
	if (view->toplevel_handle) {
		wlr_foreign_toplevel_handle_v1_set_minimized(
			view->toplevel_handle, minimized);
	}
	view->minimized = minimized;
	if (minimized) {
		view->impl->unmap(view);
		desktop_move_to_back(view);
		view_set_activated(view, false);
	} else {
		view->impl->map(view);
	}
}

/* view_wlr_output - return the output that a view is mostly on */
struct wlr_output *
view_wlr_output(struct view *view)
{
	double closest_x, closest_y;
	struct wlr_output *wlr_output = NULL;
	wlr_output_layout_closest_point(view->server->output_layout, wlr_output,
		view->x + view->w / 2, view->y + view->h / 2, &closest_x,
		&closest_y);
	wlr_output = wlr_output_layout_output_at(view->server->output_layout,
		closest_x, closest_y);
	return wlr_output;
}

static struct output *
view_output(struct view *view)
{
	struct wlr_output *wlr_output = view_wlr_output(view);
	return output_from_wlr_output(view->server, wlr_output);
}

static bool
view_compute_centered_position(struct view *view, int w, int h, int *x, int *y)
{
	struct wlr_output *wlr_output = view_wlr_output(view);
	if (!wlr_output) {
		return false;
	}

	struct wlr_output_layout *layout = view->server->output_layout;
	struct wlr_output_layout_output *ol_output =
		wlr_output_layout_get(layout, wlr_output);
	if (!ol_output) {
		return false;
	}

	*x = ol_output->x + wlr_output->width / wlr_output->scale / 2 - w / 2;
	*y = ol_output->y + wlr_output->height / wlr_output->scale / 2 - h / 2;
	return true;
}

void
view_center(struct view *view)
{
	int x, y;
	if (view_compute_centered_position(view, view->w, view->h, &x, &y)) {
		view_move(view, x, y);
	}
}

static void
view_apply_fullscreen_geometry(struct view *view, struct wlr_output *wlr_output)
{
	struct output *output =
		output_from_wlr_output(view->server, wlr_output);
	struct wlr_box box = { 0 };
	wlr_output_effective_resolution(wlr_output, &box.width, &box.height);
	double ox = 0, oy = 0;
	wlr_output_layout_output_coords(output->server->output_layout,
		output->wlr_output, &ox, &oy);
	box.x -= ox;
	box.y -= oy;
	view_move_resize(view, box);
}

static void
view_apply_maximized_geometry(struct view *view)
{
	/*
	 * The same code handles both initial maximize and re-maximize
	 * to account for layout changes.  In either case, view_output()
	 * gives the output closest to the current geometry (which may
	 * be different from the output originally maximized onto).
	 * view_output() can return NULL if all outputs are disabled.
	 */
	struct output *output = view_output(view);
	if (!output) {
		return;
	}
	struct wlr_box box = output_usable_area_in_layout_coords(output);
	if (box.height == output->wlr_output->height && output->wlr_output->scale != 1) {
		box.height /= output->wlr_output->scale;
	}
	if (box.width == output->wlr_output->width && output->wlr_output->scale != 1) {
		box.width /= output->wlr_output->scale;
	}

	if (view->ssd.enabled) {
		struct border border = ssd_thickness(view);
		box.x += border.left;
		box.y += border.top;
		box.width -= border.right + border.left;
		box.height -= border.top + border.bottom;
	}
	view_move_resize(view, box);
}

static void
view_apply_unmaximized_geometry(struct view *view)
{
	struct wlr_output_layout *layout = view->server->output_layout;
	if (wlr_output_layout_intersects(layout, NULL, &view->unmaximized_geometry)) {
		/* restore to original geometry */
		view_move_resize(view, view->unmaximized_geometry);
	} else {
		/* reposition if original geometry is offscreen */
		struct wlr_box box = view->unmaximized_geometry;
		if (view_compute_centered_position(view, box.width, box.height,
				&box.x, &box.y)) {
			view_move_resize(view, box);
		}
	}
}

void
view_maximize(struct view *view, bool maximize)
{
	if (view->maximized == maximize) {
		return;
	}
	if (view->fullscreen) {
		return;
	}
	if (view->impl->maximize) {
		view->impl->maximize(view, maximize);
	}
	if (view->toplevel_handle) {
		wlr_foreign_toplevel_handle_v1_set_maximized(
			view->toplevel_handle, maximize);
	}
	if (maximize) {
		interactive_end(view);
		view->unmaximized_geometry.x = view->x;
		view->unmaximized_geometry.y = view->y;
		view->unmaximized_geometry.width = view->w;
		view->unmaximized_geometry.height = view->h;

		view_apply_maximized_geometry(view);
		view->maximized = true;
	} else {
		/* unmaximize */
		view_apply_unmaximized_geometry(view);
		view->maximized = false;
	}
}

void
view_toggle_maximize(struct view *view)
{
	view_maximize(view, !view->maximized);
}

void
view_toggle_decorations(struct view *view)
{
	if (!view->fullscreen) {
		view->ssd.enabled = !view->ssd.enabled;
		ssd_update_geometry(view, true);
		if (view->maximized) {
			view_apply_maximized_geometry(view);
		}
	}
}

void
view_set_decorations(struct view *view, bool decorations)
{
	if (view->ssd.enabled != decorations && !view->fullscreen) {
		view->ssd.enabled = decorations;
		ssd_update_geometry(view, true);
		if (view->maximized) {
			view_apply_maximized_geometry(view);
		}
	}
}

void
view_toggle_fullscreen(struct view *view)
{
	view_set_fullscreen(view, !view->fullscreen, NULL);
}

void
view_set_fullscreen(struct view *view, bool fullscreen,
		struct wlr_output *wlr_output)
{
	if (fullscreen == (view->fullscreen != NULL)) {
		return;
	}
	if (view->impl->set_fullscreen) {
		view->impl->set_fullscreen(view, fullscreen);
	}
	if (view->toplevel_handle) {
		wlr_foreign_toplevel_handle_v1_set_fullscreen(
			view->toplevel_handle, fullscreen);
	}
	if (fullscreen) {
		if (!view->maximized) {
			view->unmaximized_geometry.x = view->x;
			view->unmaximized_geometry.y = view->y;
			view->unmaximized_geometry.width = view->w;
			view->unmaximized_geometry.height = view->h;
		}

		if (!wlr_output) {
			wlr_output = view_wlr_output(view);
		}
		view->fullscreen = wlr_output;
		view_apply_fullscreen_geometry(view, view->fullscreen);
	} else {
		/* restore to normal */
		if (view->maximized) {
			view_apply_maximized_geometry(view);
		} else {
			view_apply_unmaximized_geometry(view);
		}
		view->fullscreen = false;
	}
}

void
view_adjust_for_layout_change(struct view *view)
{
	struct wlr_output_layout *layout = view->server->output_layout;
	if (view->fullscreen) {
		if (wlr_output_layout_get(layout, view->fullscreen)) {
			/* recompute fullscreen geometry */
			view_apply_fullscreen_geometry(view, view->fullscreen);
		} else {
			/* output is gone, exit fullscreen */
			view_set_fullscreen(view, false, NULL);
		}
	} else if (view->maximized) {
		/* recompute maximized geometry */
		view_apply_maximized_geometry(view);
	} else {
		/* reposition view if it's offscreen */
		struct wlr_box box = { view->x, view->y, view->w, view->h };
		if (!wlr_output_layout_intersects(layout, NULL, &box)) {
			view_center(view);
		}
	}
}

void
view_for_each_surface(struct view *view, wlr_surface_iterator_func_t iterator,
		void *user_data)
{
	if (view->impl->for_each_surface) {
		view->impl->for_each_surface(view, iterator, user_data);
	}
}

void
view_for_each_popup_surface(struct view *view,
		wlr_surface_iterator_func_t iterator, void *data)
{
	if (view->impl->for_each_popup_surface) {
		view->impl->for_each_popup_surface(view, iterator, data);
	}
}

struct border
view_border(struct view *view)
{
	struct border border = {
		.left = view->margin.left - view->padding.left,
		.top = view->margin.top - view->padding.top,
		.right = view->margin.right + view->padding.right,
		.bottom = view->margin.bottom + view->padding.bottom,
	};
	return border;
}

static void
surface_enter_for_each_surface(struct wlr_surface *surface, int sx, int sy,
		void *user_data)
{
	struct wlr_output *wlr_output = user_data;
	wlr_surface_send_enter(surface, wlr_output);
}

static void
surface_leave_for_each_surface(struct wlr_surface *surface, int sx, int sy,
		void *user_data)
{
	struct wlr_output *wlr_output = user_data;
	wlr_surface_send_leave(surface, wlr_output);
}

static void
view_output_enter(struct view *view, struct wlr_output *wlr_output)
{
	view_for_each_surface(view, surface_enter_for_each_surface,
		wlr_output);
	if (view->toplevel_handle) {
		wlr_foreign_toplevel_handle_v1_output_enter(
			view->toplevel_handle, wlr_output);
	}
}

static void
view_output_leave(struct view *view, struct wlr_output *wlr_output)
{
	view_for_each_surface(view, surface_leave_for_each_surface,
		wlr_output);
	if (view->toplevel_handle) {
		wlr_foreign_toplevel_handle_v1_output_leave(
			view->toplevel_handle, wlr_output);
	}
}

/*
 * At present, a view can only 'enter' one output at a time, although the view
 * may span multiple outputs. Ideally we would handle multiple outputs, but
 * this method is the simplest form of what we want.
 */
void
view_discover_output(struct view *view)
{
	struct output *old_output = view->output;
	struct output *new_output = view_output(view);
	if (old_output != new_output) {
		view->output = new_output;
		view_output_enter(view, new_output->wlr_output);
		if (old_output) {
			view_output_leave(view, old_output->wlr_output);
		}
	}
}

void
view_move_to_edge(struct view *view, const char *direction)
{
	if (!view) {
		wlr_log(WLR_ERROR, "no view");
		return;
	}
	struct output *output = view_output(view);
	if (!output) {
		wlr_log(WLR_ERROR, "no output");
		return;
	}
	struct border border = view_border(view);
	struct wlr_box usable = output_usable_area_in_layout_coords(output);
	if (usable.height == output->wlr_output->height && output->wlr_output->scale != 1) {
		usable.height /= output->wlr_output->scale;
	}
	if (usable.width == output->wlr_output->width && output->wlr_output->scale != 1) {
		usable.width /= output->wlr_output->scale;
	}

	int x = 0, y = 0;
	if (!strcasecmp(direction, "left")) {
		x = usable.x + border.left + rc.gap;
		y = view->y;
	} else if (!strcasecmp(direction, "up")) {
		x = view->x;
		y = usable.y + border.top + rc.gap;
	} else if (!strcasecmp(direction, "right")) {
		x = usable.x + usable.width - view->w - border.right - rc.gap;
		y = view->y;
	} else if (!strcasecmp(direction, "down")) {
		x = view->x;
		y = usable.y + usable.height - view->h - border.bottom - rc.gap;
	}
	view_move(view, x, y);
}

enum view_edge {
	VIEW_EDGE_INVALID,

	VIEW_EDGE_LEFT,
	VIEW_EDGE_RIGHT,
	VIEW_EDGE_UP,
	VIEW_EDGE_DOWN,
	VIEW_EDGE_CENTER,
};

static enum view_edge
view_edge_invert(enum view_edge edge)
{
	switch (edge) {
	case VIEW_EDGE_LEFT:
		return VIEW_EDGE_RIGHT;
	case VIEW_EDGE_RIGHT:
		return VIEW_EDGE_LEFT;
	case VIEW_EDGE_UP:
		return VIEW_EDGE_DOWN;
	case VIEW_EDGE_DOWN:
		return VIEW_EDGE_UP;
	case VIEW_EDGE_CENTER:
	case VIEW_EDGE_INVALID:
	default:
		return VIEW_EDGE_INVALID;
	}
}

static enum view_edge
view_edge_parse(const char *direction)
{
	if (!strcasecmp(direction, "left")) {
		return VIEW_EDGE_LEFT;
	} else if (!strcasecmp(direction, "up")) {
		return VIEW_EDGE_UP;
	} else if (!strcasecmp(direction, "right")) {
		return VIEW_EDGE_RIGHT;
	} else if (!strcasecmp(direction, "down")) {
		return VIEW_EDGE_DOWN;
	} else if (!strcasecmp(direction, "center")) {
		return VIEW_EDGE_CENTER;
	} else {
		return VIEW_EDGE_INVALID;
	}
}

static struct wlr_box
view_get_edge_snap_box(struct view *view, struct output *output,
		enum view_edge edge)
{
	struct border border = view_border(view);
	struct wlr_box usable = output_usable_area_in_layout_coords(output);
	if (usable.height == output->wlr_output->height && output->wlr_output->scale != 1) {
		usable.height /= output->wlr_output->scale;
	}
	if (usable.width == output->wlr_output->width && output->wlr_output->scale != 1) {
		usable.width /= output->wlr_output->scale;
	}

	int x_offset = edge == VIEW_EDGE_RIGHT
		? (usable.width + rc.gap) / 2 : rc.gap;
	int y_offset = edge == VIEW_EDGE_DOWN
		? (usable.height + rc.gap) / 2 : rc.gap;

	int base_width, base_height;
	switch (edge) {
	case VIEW_EDGE_LEFT:
	case VIEW_EDGE_RIGHT:
		base_width = (usable.width - 3 * rc.gap) / 2;
		base_height = usable.height - 2 * rc.gap;
		break;
	case VIEW_EDGE_UP:
	case VIEW_EDGE_DOWN:
		base_width = usable.width - 2 * rc.gap;
		base_height = (usable.height - 3 * rc.gap) / 2;
		break;
	default:
	case VIEW_EDGE_CENTER:
		base_width = usable.width - 2 * rc.gap;
		base_height = usable.height - 2 * rc.gap;
		break;
	}
	struct wlr_box dst = {
		.x = x_offset + usable.x + border.left,
		.y = y_offset + usable.y + border.top,
		.width = base_width - border.left - border.right,
		.height = base_height - border.top - border.bottom,
	};

	return dst;
}

void
view_snap_to_edge(struct view *view, const char *direction)
{
	if (!view) {
		wlr_log(WLR_ERROR, "no view");
		return;
	}
	struct output *output = view_output(view);
	if (!output) {
		wlr_log(WLR_ERROR, "no output");
		return;
	}
	enum view_edge edge = view_edge_parse(direction);
	if (edge == VIEW_EDGE_INVALID) {
		wlr_log(WLR_ERROR, "invalid edge");
		return;
	}

	struct wlr_box dst = view_get_edge_snap_box(view, output, edge);

	if (view->x == dst.x && view->y == dst.y && view->w == dst.width
			&& view->h == dst.height) {
		/* Move over to the next screen if this is already snapped. */
		struct wlr_box usable =
			output_usable_area_in_layout_coords(output);
		switch (edge) {
		case VIEW_EDGE_LEFT:
			dst.x -= (usable.width / 2) + 1;
			break;
		case VIEW_EDGE_RIGHT:
			dst.x += (usable.width / 2) + 1;
			break;
		case VIEW_EDGE_UP:
			dst.y -= (usable.height / 2) + 1;
			break;
		case VIEW_EDGE_DOWN:
			dst.y += (usable.height / 2) + 1;
			break;
		default:
			break;
		}

		struct wlr_output *new_wlr_output = wlr_output_layout_output_at(
			view->server->output_layout, dst.x, dst.y);
		struct output *new_output =
			output_from_wlr_output(view->server, new_wlr_output);

		if (new_output == output || !new_output
				|| edge == VIEW_EDGE_CENTER) {
			return;
		}

		dst = view_get_edge_snap_box(view, new_output,
			view_edge_invert(edge));
	}

	if (view->w == dst.width && view->h == dst.height) {
		/* move horizontally/vertically without changing size */
		view_move(view, dst.x, dst.y);
	} else {
		view_move_resize(view, dst);
	}
}

const char *
view_get_string_prop(struct view *view, const char *prop)
{
	if (view->impl->get_string_prop) {
		return view->impl->get_string_prop(view, prop);
	}
	return "";
}

void
view_update_title(struct view *view)
{
	const char *title = view_get_string_prop(view, "title");
	if (!view->toplevel_handle || !title) {
		return;
	}
	ssd_update_title(view);
	wlr_foreign_toplevel_handle_v1_set_title(view->toplevel_handle, title);
	damage_all_outputs(view->server);
}

void
view_update_app_id(struct view *view)
{
	const char *app_id = view_get_string_prop(view, "app_id");
	if (!view->toplevel_handle || !app_id) {
		return;
	}
	wlr_foreign_toplevel_handle_v1_set_app_id(
		view->toplevel_handle, app_id);
}
