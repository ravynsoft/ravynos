#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <wayland-server-core.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/util/log.h>
#include "types/wlr_seat.h"
#include "util/signal.h"

static uint32_t default_touch_down(struct wlr_seat_touch_grab *grab,
		uint32_t time, struct wlr_touch_point *point) {
	return wlr_seat_touch_send_down(grab->seat, point->surface, time,
			point->touch_id, point->sx, point->sy);
}

static void default_touch_up(struct wlr_seat_touch_grab *grab, uint32_t time,
		struct wlr_touch_point *point) {
	wlr_seat_touch_send_up(grab->seat, time, point->touch_id);
}

static void default_touch_motion(struct wlr_seat_touch_grab *grab,
		uint32_t time, struct wlr_touch_point *point) {
	if (!point->focus_surface || point->focus_surface == point->surface) {
		wlr_seat_touch_send_motion(grab->seat, time, point->touch_id, point->sx,
			point->sy);
	}
}

static void default_touch_enter(struct wlr_seat_touch_grab *grab,
		uint32_t time, struct wlr_touch_point *point) {
	// not handled by default
}

static void default_touch_frame(struct wlr_seat_touch_grab *grab) {
	wlr_seat_touch_send_frame(grab->seat);
}

static void default_touch_cancel(struct wlr_seat_touch_grab *grab) {
	// cannot be cancelled
}

const struct wlr_touch_grab_interface default_touch_grab_impl = {
	.down = default_touch_down,
	.up = default_touch_up,
	.motion = default_touch_motion,
	.enter = default_touch_enter,
	.frame = default_touch_frame,
	.cancel = default_touch_cancel,
};


static void touch_release(struct wl_client *client,
		struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static const struct wl_touch_interface touch_impl = {
	.release = touch_release,
};

static void touch_handle_resource_destroy(struct wl_resource *resource) {
	wl_list_remove(wl_resource_get_link(resource));
	seat_client_destroy_touch(resource);
}

static struct wlr_seat_client *seat_client_from_touch_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource, &wl_touch_interface,
		&touch_impl));
	return wl_resource_get_user_data(resource);
}


void wlr_seat_touch_start_grab(struct wlr_seat *wlr_seat,
		struct wlr_seat_touch_grab *grab) {
	grab->seat = wlr_seat;
	wlr_seat->touch_state.grab = grab;

	wlr_signal_emit_safe(&wlr_seat->events.touch_grab_begin, grab);
}

void wlr_seat_touch_end_grab(struct wlr_seat *wlr_seat) {
	struct wlr_seat_touch_grab *grab = wlr_seat->touch_state.grab;

	if (grab != wlr_seat->touch_state.default_grab) {
		wlr_seat->touch_state.grab = wlr_seat->touch_state.default_grab;
		wlr_signal_emit_safe(&wlr_seat->events.touch_grab_end, grab);
		if (grab->interface->cancel) {
			grab->interface->cancel(grab);
		}
	}
}

static void touch_point_clear_focus(struct wlr_touch_point *point) {
	if (point->focus_surface) {
		wl_list_remove(&point->focus_surface_destroy.link);
		point->focus_client = NULL;
		point->focus_surface = NULL;
	}
}

static void touch_point_destroy(struct wlr_touch_point *point) {
	wlr_signal_emit_safe(&point->events.destroy, point);

	touch_point_clear_focus(point);
	wl_list_remove(&point->surface_destroy.link);
	wl_list_remove(&point->client_destroy.link);
	wl_list_remove(&point->link);
	free(point);
}

static void touch_point_handle_surface_destroy(struct wl_listener *listener,
		void *data) {
	struct wlr_touch_point *point =
		wl_container_of(listener, point, surface_destroy);
	// Touch point itself is destroyed on up event
	point->surface = NULL;
	wl_list_remove(&point->surface_destroy.link);
	wl_list_init(&point->surface_destroy.link);
}

static void touch_point_handle_client_destroy(struct wl_listener *listener,
		void *data) {
	struct wlr_touch_point *point =
		wl_container_of(listener, point, client_destroy);
	touch_point_destroy(point);
}

static struct wlr_touch_point *touch_point_create(
		struct wlr_seat *seat, int32_t touch_id,
		struct wlr_surface *surface, double sx, double sy) {
	struct wl_client *wl_client = wl_resource_get_client(surface->resource);
	struct wlr_seat_client *client =
		wlr_seat_client_for_wl_client(seat, wl_client);

	if (client == NULL || wl_list_empty(&client->touches)) {
		// touch points are not valid without a connected client with touch
		return NULL;
	}

	struct wlr_touch_point *point = calloc(1, sizeof(struct wlr_touch_point));
	if (!point) {
		return NULL;
	}

	point->touch_id = touch_id;
	point->surface = surface;
	point->client = client;

	point->sx = sx;
	point->sy = sy;

	wl_signal_init(&point->events.destroy);

	wl_signal_add(&surface->events.destroy, &point->surface_destroy);
	point->surface_destroy.notify = touch_point_handle_surface_destroy;
	wl_signal_add(&client->events.destroy, &point->client_destroy);
	point->client_destroy.notify = touch_point_handle_client_destroy;
	wl_list_insert(&seat->touch_state.touch_points, &point->link);

	return point;
}

struct wlr_touch_point *wlr_seat_touch_get_point(
		struct wlr_seat *seat, int32_t touch_id) {
	struct wlr_touch_point *point = NULL;
	wl_list_for_each(point, &seat->touch_state.touch_points, link) {
		if (point->touch_id == touch_id) {
			return point;
		}
	}

	return NULL;
}

uint32_t wlr_seat_touch_notify_down(struct wlr_seat *seat,
		struct wlr_surface *surface, uint32_t time, int32_t touch_id, double sx,
		double sy) {
	clock_gettime(CLOCK_MONOTONIC, &seat->last_event);
	struct wlr_seat_touch_grab *grab = seat->touch_state.grab;
	struct wlr_touch_point *point =
		touch_point_create(seat, touch_id, surface, sx, sy);
	if (!point) {
		wlr_log(WLR_ERROR, "could not create touch point");
		return 0;
	}

	uint32_t serial = grab->interface->down(grab, time, point);

	if (!serial) {
		touch_point_destroy(point);
		return 0;
	}

	if (serial && wlr_seat_touch_num_points(seat) == 1) {
		seat->touch_state.grab_serial = serial;
		seat->touch_state.grab_id = touch_id;
	}

	return serial;
}

void wlr_seat_touch_notify_up(struct wlr_seat *seat, uint32_t time,
		int32_t touch_id) {
	clock_gettime(CLOCK_MONOTONIC, &seat->last_event);
	struct wlr_seat_touch_grab *grab = seat->touch_state.grab;
	struct wlr_touch_point *point = wlr_seat_touch_get_point(seat, touch_id);
	if (!point) {
		return;
	}

	grab->interface->up(grab, time, point);

	touch_point_destroy(point);
}

void wlr_seat_touch_notify_motion(struct wlr_seat *seat, uint32_t time,
		int32_t touch_id, double sx, double sy) {
	clock_gettime(CLOCK_MONOTONIC, &seat->last_event);
	struct wlr_seat_touch_grab *grab = seat->touch_state.grab;
	struct wlr_touch_point *point = wlr_seat_touch_get_point(seat, touch_id);
	if (!point) {
		return;
	}

	point->sx = sx;
	point->sy = sy;

	grab->interface->motion(grab, time, point);
}

void wlr_seat_touch_notify_frame(struct wlr_seat *seat) {
	struct wlr_seat_touch_grab *grab = seat->touch_state.grab;
	if (grab->interface->frame) {
		grab->interface->frame(grab);
	}
}

static void handle_point_focus_destroy(struct wl_listener *listener,
		void *data) {
	struct wlr_touch_point *point =
		wl_container_of(listener, point, focus_surface_destroy);
	touch_point_clear_focus(point);
}

static void touch_point_set_focus(struct wlr_touch_point *point,
		struct wlr_surface *surface, double sx, double sy) {
	if (point->focus_surface == surface) {
		return;
	}

	touch_point_clear_focus(point);

	if (surface && surface->resource) {
		struct wlr_seat_client *client =
			wlr_seat_client_for_wl_client(point->client->seat,
				wl_resource_get_client(surface->resource));

		if (client && !wl_list_empty(&client->touches)) {
			wl_signal_add(&surface->events.destroy, &point->focus_surface_destroy);
			point->focus_surface_destroy.notify = handle_point_focus_destroy;
			point->focus_surface = surface;
			point->focus_client = client;
			point->sx = sx;
			point->sy = sy;
		}
	}
}

void wlr_seat_touch_point_focus(struct wlr_seat *seat,
		struct wlr_surface *surface, uint32_t time, int32_t touch_id, double sx,
		double sy) {
	assert(surface);
	struct wlr_touch_point *point = wlr_seat_touch_get_point(seat, touch_id);
	if (!point) {
		wlr_log(WLR_ERROR, "got touch point focus for unknown touch point");
		return;
	}
	struct wlr_surface *focus = point->focus_surface;
	touch_point_set_focus(point, surface, sx, sy);

	if (focus != point->focus_surface) {
		struct wlr_seat_touch_grab *grab = seat->touch_state.grab;
		grab->interface->enter(grab, time, point);
	}
}

void wlr_seat_touch_point_clear_focus(struct wlr_seat *seat, uint32_t time,
		int32_t touch_id) {
	struct wlr_touch_point *point = wlr_seat_touch_get_point(seat, touch_id);
	if (!point) {
		wlr_log(WLR_ERROR, "got touch point focus for unknown touch point");
		return;
	}

	touch_point_clear_focus(point);
}

uint32_t wlr_seat_touch_send_down(struct wlr_seat *seat,
		struct wlr_surface *surface, uint32_t time, int32_t touch_id, double sx,
		double sy) {
	struct wlr_touch_point *point = wlr_seat_touch_get_point(seat, touch_id);
	if (!point) {
		wlr_log(WLR_ERROR, "got touch down for unknown touch point");
		return 0;
	}

	uint32_t serial = wlr_seat_client_next_serial(point->client);
	struct wl_resource *resource;
	wl_resource_for_each(resource, &point->client->touches) {
		if (seat_client_from_touch_resource(resource) == NULL) {
			continue;
		}
		wl_touch_send_down(resource, serial, time, surface->resource,
			touch_id, wl_fixed_from_double(sx), wl_fixed_from_double(sy));
	}

	point->client->needs_touch_frame = true;

	return serial;
}

void wlr_seat_touch_send_up(struct wlr_seat *seat, uint32_t time, int32_t touch_id) {
	struct wlr_touch_point *point = wlr_seat_touch_get_point(seat, touch_id);
	if (!point) {
		wlr_log(WLR_ERROR, "got touch up for unknown touch point");
		return;
	}

	uint32_t serial = wlr_seat_client_next_serial(point->client);
	struct wl_resource *resource;
	wl_resource_for_each(resource, &point->client->touches) {
		if (seat_client_from_touch_resource(resource) == NULL) {
			continue;
		}
		wl_touch_send_up(resource, serial, time, touch_id);
	}

	point->client->needs_touch_frame = true;
}

void wlr_seat_touch_send_motion(struct wlr_seat *seat, uint32_t time, int32_t touch_id,
		double sx, double sy) {
	struct wlr_touch_point *point = wlr_seat_touch_get_point(seat, touch_id);
	if (!point) {
		wlr_log(WLR_ERROR, "got touch motion for unknown touch point");
		return;
	}

	struct wl_resource *resource;
	wl_resource_for_each(resource, &point->client->touches) {
		if (seat_client_from_touch_resource(resource) == NULL) {
			continue;
		}
		wl_touch_send_motion(resource, time, touch_id, wl_fixed_from_double(sx),
			wl_fixed_from_double(sy));
	}

	point->client->needs_touch_frame = true;
}

void wlr_seat_touch_send_frame(struct wlr_seat *seat) {
	struct wlr_seat_client *seat_client;
	wl_list_for_each(seat_client, &seat->clients, link) {
		if (!seat_client->needs_touch_frame) {
			continue;
		}

		struct wl_resource *resource;
		wl_resource_for_each(resource, &seat_client->touches) {
			wl_touch_send_frame(resource);
		}
		seat_client->needs_touch_frame = false;
	}
}

int wlr_seat_touch_num_points(struct wlr_seat *seat) {
	return wl_list_length(&seat->touch_state.touch_points);
}

bool wlr_seat_touch_has_grab(struct wlr_seat *seat) {
	return seat->touch_state.grab->interface != &default_touch_grab_impl;
}


void seat_client_create_touch(struct wlr_seat_client *seat_client,
		uint32_t version, uint32_t id) {
	struct wl_resource *resource = wl_resource_create(seat_client->client,
		&wl_touch_interface, version, id);
	if (resource == NULL) {
		wl_client_post_no_memory(seat_client->client);
		return;
	}
	wl_resource_set_implementation(resource, &touch_impl, seat_client,
		&touch_handle_resource_destroy);
	wl_list_insert(&seat_client->touches, wl_resource_get_link(resource));

	if ((seat_client->seat->capabilities & WL_SEAT_CAPABILITY_TOUCH) == 0) {
		wl_resource_set_user_data(resource, NULL);
	}
}

void seat_client_destroy_touch(struct wl_resource *resource) {
	struct wlr_seat_client *seat_client =
		seat_client_from_touch_resource(resource);
	if (seat_client == NULL) {
		return;
	}
	wl_resource_set_user_data(resource, NULL);
}

bool wlr_seat_validate_touch_grab_serial(struct wlr_seat *seat,
		struct wlr_surface *origin, uint32_t serial,
		struct wlr_touch_point **point_ptr) {
	if (wlr_seat_touch_num_points(seat) != 1 ||
			seat->touch_state.grab_serial != serial) {
		wlr_log(WLR_DEBUG, "Touch grab serial validation failed: "
			"num_points=%d grab_serial=%"PRIu32" (got %"PRIu32")",
			wlr_seat_touch_num_points(seat),
			seat->touch_state.grab_serial, serial);
		return false;
	}

	struct wlr_touch_point *point;
	wl_list_for_each(point, &seat->touch_state.touch_points, link) {
		if (origin == NULL || point->surface == origin) {
			if (point_ptr != NULL) {
				*point_ptr = point;
			}
			return true;
		}
	}

	wlr_log(WLR_DEBUG, "Touch grab serial validation failed: "
		"invalid origin surface");
	return false;
}

bool wlr_surface_accepts_touch(struct wlr_seat *wlr_seat, struct wlr_surface *surface) {
	struct wl_client *client = wl_resource_get_client(surface->resource);
	struct wlr_seat_client *seat_client = wlr_seat_client_for_wl_client(wlr_seat, client);
	if (!seat_client) {
		return false;
	}
	return !wl_list_empty(&seat_client->touches);
}
