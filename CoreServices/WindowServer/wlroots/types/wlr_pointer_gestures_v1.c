#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include <assert.h>
#include <stdlib.h>
#include <wayland-server-core.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_pointer_gestures_v1.h>
#include <wlr/util/log.h>
#include "util/signal.h"
#include "pointer-gestures-unstable-v1-protocol.h"

#define POINTER_GESTURES_VERSION 3

static void resource_handle_destroy(struct wl_client *client,
		struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static void resource_remove_from_list(struct wl_resource *resource) {
	wl_list_remove(wl_resource_get_link(resource));
}

static const struct zwp_pointer_gestures_v1_interface gestures_impl;
static const struct zwp_pointer_gesture_swipe_v1_interface swipe_impl;
static const struct zwp_pointer_gesture_pinch_v1_interface pinch_impl;
static const struct zwp_pointer_gesture_hold_v1_interface hold_impl;

static struct wlr_pointer_gestures_v1 *pointer_gestures_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource,
				&zwp_pointer_gestures_v1_interface, &gestures_impl));
	return wl_resource_get_user_data(resource);
}

static struct wlr_seat *seat_from_pointer_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource,
				&zwp_pointer_gesture_swipe_v1_interface, &swipe_impl) ||
			wl_resource_instance_of(resource,
				&zwp_pointer_gesture_pinch_v1_interface, &pinch_impl) ||
			wl_resource_instance_of(resource,
				&zwp_pointer_gesture_hold_v1_interface, &hold_impl));
	return wl_resource_get_user_data(resource);
}

void wlr_pointer_gestures_v1_send_swipe_begin(
		struct wlr_pointer_gestures_v1 *gestures,
		struct wlr_seat *seat,
		uint32_t time_msec,
		uint32_t fingers) {
	struct wlr_surface *focus = seat->pointer_state.focused_surface;
	if (focus == NULL) {
		return;
	}

	struct wl_client *focus_client = wl_resource_get_client(focus->resource);
	uint32_t serial = wlr_seat_client_next_serial(
		seat->pointer_state.focused_client);

	struct wl_resource *gesture;
	wl_resource_for_each(gesture, &gestures->swipes) {
		struct wlr_seat *gesture_seat = seat_from_pointer_resource(gesture);
		struct wl_client *gesture_client = wl_resource_get_client(gesture);
		if (gesture_seat != seat || gesture_client != focus_client) {
			continue;
		}
		zwp_pointer_gesture_swipe_v1_send_begin(gesture, serial,
				time_msec, focus->resource, fingers);
	}
}

void wlr_pointer_gestures_v1_send_swipe_update(
		struct wlr_pointer_gestures_v1 *gestures,
		struct wlr_seat *seat,
		uint32_t time_msec,
		double dx,
		double dy) {
	struct wlr_surface *focus = seat->pointer_state.focused_surface;
	if (focus == NULL) {
		return;
	}

	struct wl_client *focus_client = wl_resource_get_client(focus->resource);

	struct wl_resource *gesture;
	wl_resource_for_each(gesture, &gestures->swipes) {
		struct wlr_seat *gesture_seat = seat_from_pointer_resource(gesture);
		struct wl_client *gesture_client = wl_resource_get_client(gesture);
		if (gesture_seat != seat || gesture_client != focus_client) {
			continue;
		}
		zwp_pointer_gesture_swipe_v1_send_update(gesture, time_msec,
				wl_fixed_from_double(dx), wl_fixed_from_double(dy));
	}
}

void wlr_pointer_gestures_v1_send_swipe_end(
		struct wlr_pointer_gestures_v1 *gestures,
		struct wlr_seat *seat,
		uint32_t time_msec,
		bool cancelled) {
	struct wlr_surface *focus = seat->pointer_state.focused_surface;
	if (focus == NULL) {
		return;
	}

	struct wl_client *focus_client = wl_resource_get_client(focus->resource);
	uint32_t serial = wlr_seat_client_next_serial(
		seat->pointer_state.focused_client);

	struct wl_resource *gesture;
	wl_resource_for_each(gesture, &gestures->swipes) {
		struct wlr_seat *gesture_seat = seat_from_pointer_resource(gesture);
		struct wl_client *gesture_client = wl_resource_get_client(gesture);
		if (gesture_seat != seat || gesture_client != focus_client) {
			continue;
		}
		zwp_pointer_gesture_swipe_v1_send_end(gesture, serial,
				time_msec, cancelled);
	}
}

static const struct zwp_pointer_gesture_swipe_v1_interface swipe_impl = {
	.destroy = resource_handle_destroy,
};

static void get_swipe_gesture(struct wl_client *client,
		struct wl_resource *gestures_resource,
		uint32_t id,
		struct wl_resource *pointer_resource) {
	struct wlr_seat_client *seat_client =
		wlr_seat_client_from_pointer_resource(pointer_resource);
	struct wlr_seat *seat = NULL;

	if (seat_client != NULL) {
		seat = seat_client->seat;
	}
	// Otherwise, the resource will be inert
	// (NULL seat, so all seat comparisons will fail)

	struct wlr_pointer_gestures_v1 *gestures =
		pointer_gestures_from_resource(gestures_resource);

	struct wl_resource *gesture = wl_resource_create(client,
		&zwp_pointer_gesture_swipe_v1_interface,
		wl_resource_get_version(gestures_resource),
		id);
	if (gesture == NULL) {
		wl_client_post_no_memory(client);
		return;
	}

	wl_resource_set_implementation(gesture, &swipe_impl, seat,
			resource_remove_from_list);
	wl_list_insert(&gestures->swipes, wl_resource_get_link(gesture));
}

void wlr_pointer_gestures_v1_send_pinch_begin(
		struct wlr_pointer_gestures_v1 *gestures,
		struct wlr_seat *seat,
		uint32_t time_msec,
		uint32_t fingers) {
	struct wlr_surface *focus = seat->pointer_state.focused_surface;
	if (focus == NULL) {
		return;
	}

	struct wl_client *focus_client = wl_resource_get_client(focus->resource);
	uint32_t serial = wlr_seat_client_next_serial(
		seat->pointer_state.focused_client);

	struct wl_resource *gesture;
	wl_resource_for_each(gesture, &gestures->pinches) {
		struct wlr_seat *gesture_seat = seat_from_pointer_resource(gesture);
		struct wl_client *gesture_client = wl_resource_get_client(gesture);
		if (gesture_seat != seat || gesture_client != focus_client) {
			continue;
		}
		zwp_pointer_gesture_pinch_v1_send_begin(gesture, serial,
				time_msec, focus->resource, fingers);
	}
}

void wlr_pointer_gestures_v1_send_pinch_update(
		struct wlr_pointer_gestures_v1 *gestures,
		struct wlr_seat *seat,
		uint32_t time_msec,
		double dx,
		double dy,
		double scale,
		double rotation) {
	struct wlr_surface *focus = seat->pointer_state.focused_surface;
	if (focus == NULL) {
		return;
	}

	struct wl_client *focus_client = wl_resource_get_client(focus->resource);

	struct wl_resource *gesture;
	wl_resource_for_each(gesture, &gestures->pinches) {
		struct wlr_seat *gesture_seat = seat_from_pointer_resource(gesture);
		struct wl_client *gesture_client = wl_resource_get_client(gesture);
		if (gesture_seat != seat || gesture_client != focus_client) {
			continue;
		}
		zwp_pointer_gesture_pinch_v1_send_update(gesture, time_msec,
				wl_fixed_from_double(dx), wl_fixed_from_double(dy),
				wl_fixed_from_double(scale),
				wl_fixed_from_double(rotation));
	}
}

void wlr_pointer_gestures_v1_send_pinch_end(
		struct wlr_pointer_gestures_v1 *gestures,
		struct wlr_seat *seat,
		uint32_t time_msec,
		bool cancelled) {
	struct wlr_surface *focus = seat->pointer_state.focused_surface;
	if (focus == NULL) {
		return;
	}

	struct wl_client *focus_client = wl_resource_get_client(focus->resource);
	uint32_t serial = wlr_seat_client_next_serial(
		seat->pointer_state.focused_client);

	struct wl_resource *gesture;
	wl_resource_for_each(gesture, &gestures->pinches) {
		struct wlr_seat *gesture_seat = seat_from_pointer_resource(gesture);
		struct wl_client *gesture_client = wl_resource_get_client(gesture);
		if (gesture_seat != seat || gesture_client != focus_client) {
			continue;
		}
		zwp_pointer_gesture_pinch_v1_send_end(gesture, serial,
				time_msec, cancelled);
	}
}

static const struct zwp_pointer_gesture_pinch_v1_interface pinch_impl = {
	.destroy = resource_handle_destroy,
};

static void get_pinch_gesture(struct wl_client *client,
		struct wl_resource *gestures_resource,
		uint32_t id,
		struct wl_resource *pointer_resource) {
	struct wlr_seat_client *seat_client =
		wlr_seat_client_from_pointer_resource(pointer_resource);
	struct wlr_seat *seat = NULL;

	if (seat_client != NULL) {
		seat = seat_client->seat;
	}
	// Otherwise, the resource will be inert
	// (NULL seat, so all seat comparisons will fail)

	struct wlr_pointer_gestures_v1 *gestures =
		pointer_gestures_from_resource(gestures_resource);

	struct wl_resource *gesture = wl_resource_create(client,
		&zwp_pointer_gesture_pinch_v1_interface,
		wl_resource_get_version(gestures_resource),
		id);
	if (gesture == NULL) {
		wl_client_post_no_memory(client);
		return;
	}

	wl_resource_set_implementation(gesture, &pinch_impl, seat,
			resource_remove_from_list);
	wl_list_insert(&gestures->pinches, wl_resource_get_link(gesture));
}

static void pointer_gestures_release(struct wl_client *client,
		struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

void wlr_pointer_gestures_v1_send_hold_begin(
		struct wlr_pointer_gestures_v1 *gestures,
		struct wlr_seat *seat,
		uint32_t time_msec,
		uint32_t fingers) {
	struct wlr_surface *focus = seat->pointer_state.focused_surface;
	if (focus == NULL) {
		return;
	}

	struct wl_client *focus_client = wl_resource_get_client(focus->resource);
	uint32_t serial = wlr_seat_client_next_serial(
		seat->pointer_state.focused_client);

	struct wl_resource *gesture;
	wl_resource_for_each(gesture, &gestures->holds) {
		struct wlr_seat *gesture_seat = seat_from_pointer_resource(gesture);
		struct wl_client *gesture_client = wl_resource_get_client(gesture);
		if (gesture_seat != seat || gesture_client != focus_client) {
			continue;
		}
		zwp_pointer_gesture_hold_v1_send_begin(gesture, serial,
				time_msec, focus->resource, fingers);
	}
}

void wlr_pointer_gestures_v1_send_hold_end(
		struct wlr_pointer_gestures_v1 *gestures,
		struct wlr_seat *seat,
		uint32_t time_msec,
		bool cancelled) {
	struct wlr_surface *focus = seat->pointer_state.focused_surface;
	if (focus == NULL) {
		return;
	}

	struct wl_client *focus_client = wl_resource_get_client(focus->resource);
	uint32_t serial = wlr_seat_client_next_serial(
		seat->pointer_state.focused_client);

	struct wl_resource *gesture;
	wl_resource_for_each(gesture, &gestures->holds) {
		struct wlr_seat *gesture_seat = seat_from_pointer_resource(gesture);
		struct wl_client *gesture_client = wl_resource_get_client(gesture);
		if (gesture_seat != seat || gesture_client != focus_client) {
			continue;
		}
		zwp_pointer_gesture_hold_v1_send_end(gesture, serial,
				time_msec, cancelled);
	}
}

static const struct zwp_pointer_gesture_hold_v1_interface hold_impl = {
	.destroy = resource_handle_destroy,
};

static void get_hold_gesture(struct wl_client *client,
		struct wl_resource *gestures_resource,
		uint32_t id,
		struct wl_resource *pointer_resource) {
	struct wlr_seat_client *seat_client =
		wlr_seat_client_from_pointer_resource(pointer_resource);
	struct wlr_seat *seat = NULL;

	if (seat_client != NULL) {
		seat = seat_client->seat;
	}
	// Otherwise, the resource will be inert
	// (NULL seat, so all seat comparisons will fail)

	struct wlr_pointer_gestures_v1 *gestures =
		pointer_gestures_from_resource(gestures_resource);

	struct wl_resource *gesture = wl_resource_create(client,
		&zwp_pointer_gesture_hold_v1_interface,
		wl_resource_get_version(gestures_resource),
		id);
	if (gesture == NULL) {
		wl_client_post_no_memory(client);
		return;
	}

	wl_resource_set_implementation(gesture, &hold_impl, seat,
			resource_remove_from_list);
	wl_list_insert(&gestures->holds, wl_resource_get_link(gesture));
}

static const struct zwp_pointer_gestures_v1_interface gestures_impl = {
	.get_swipe_gesture = get_swipe_gesture,
	.get_pinch_gesture = get_pinch_gesture,
	.release = pointer_gestures_release,
	.get_hold_gesture = get_hold_gesture,
};

static void pointer_gestures_v1_bind(struct wl_client *wl_client, void *data,
		uint32_t version, uint32_t id) {
	struct wlr_pointer_gestures_v1 *gestures = data;

	struct wl_resource *resource = wl_resource_create(wl_client,
			&zwp_pointer_gestures_v1_interface, version, id);
	if (resource == NULL) {
		wl_client_post_no_memory(wl_client);
		return;
	}

	wl_resource_set_implementation(resource,
			&gestures_impl, gestures, NULL);
}

static void handle_display_destroy(struct wl_listener *listener, void *data) {
	struct wlr_pointer_gestures_v1 *gestures =
		wl_container_of(listener, gestures, display_destroy);
	wl_list_remove(&gestures->display_destroy.link);
	wl_global_destroy(gestures->global);
	free(gestures);
}

struct wlr_pointer_gestures_v1 *wlr_pointer_gestures_v1_create(
		struct wl_display *display) {
	struct wlr_pointer_gestures_v1 *gestures =
		calloc(1, sizeof(struct wlr_pointer_gestures_v1));
	if (!gestures) {
		return NULL;
	}

	wl_list_init(&gestures->swipes);
	wl_list_init(&gestures->pinches);
	wl_list_init(&gestures->holds);

	gestures->global = wl_global_create(display,
			&zwp_pointer_gestures_v1_interface, POINTER_GESTURES_VERSION,
			gestures, pointer_gestures_v1_bind);
	if (gestures->global == NULL) {
		free(gestures);
		return NULL;
	}

	gestures->display_destroy.notify = handle_display_destroy;
	wl_display_add_destroy_listener(display, &gestures->display_destroy);

	return gestures;
}
