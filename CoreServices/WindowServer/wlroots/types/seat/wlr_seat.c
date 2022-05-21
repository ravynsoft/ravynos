#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <wayland-server-core.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_primary_selection.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/util/log.h>
#include "types/wlr_seat.h"
#include "util/global.h"
#include "util/signal.h"

#define SEAT_VERSION 7

static void seat_handle_get_pointer(struct wl_client *client,
		struct wl_resource *seat_resource, uint32_t id) {
	struct wlr_seat_client *seat_client =
		wlr_seat_client_from_resource(seat_resource);
	if (!(seat_client->seat->accumulated_capabilities & WL_SEAT_CAPABILITY_POINTER)) {
		wl_resource_post_error(seat_resource, WL_SEAT_ERROR_MISSING_CAPABILITY,
				"wl_seat.get_pointer called when no pointer capability has existed");
		return;
	}

	uint32_t version = wl_resource_get_version(seat_resource);
	seat_client_create_pointer(seat_client, version, id);
}

static void seat_handle_get_keyboard(struct wl_client *client,
		struct wl_resource *seat_resource, uint32_t id) {
	struct wlr_seat_client *seat_client =
		wlr_seat_client_from_resource(seat_resource);
	if (!(seat_client->seat->accumulated_capabilities & WL_SEAT_CAPABILITY_KEYBOARD)) {
		wl_resource_post_error(seat_resource, WL_SEAT_ERROR_MISSING_CAPABILITY,
				"wl_seat.get_keyboard called when no keyboard capability has existed");
		return;
	}

	uint32_t version = wl_resource_get_version(seat_resource);
	seat_client_create_keyboard(seat_client, version, id);
}

static void seat_handle_get_touch(struct wl_client *client,
		struct wl_resource *seat_resource, uint32_t id) {
	struct wlr_seat_client *seat_client =
		wlr_seat_client_from_resource(seat_resource);
	if (!(seat_client->seat->accumulated_capabilities & WL_SEAT_CAPABILITY_TOUCH)) {
		wl_resource_post_error(seat_resource, WL_SEAT_ERROR_MISSING_CAPABILITY,
				"wl_seat.get_touch called when no touch capability has existed");
		return;
	}

	uint32_t version = wl_resource_get_version(seat_resource);
	seat_client_create_touch(seat_client, version, id);
}

static void seat_client_handle_resource_destroy(
		struct wl_resource *seat_resource) {
	struct wlr_seat_client *client =
		wlr_seat_client_from_resource(seat_resource);

	wl_list_remove(wl_resource_get_link(seat_resource));
	if (!wl_list_empty(&client->resources)) {
		return;
	}

	wlr_signal_emit_safe(&client->events.destroy, client);

	if (client == client->seat->pointer_state.focused_client) {
		client->seat->pointer_state.focused_client = NULL;
	}
	if (client == client->seat->keyboard_state.focused_client) {
		client->seat->keyboard_state.focused_client = NULL;
	}

	struct wl_resource *resource, *tmp;
	wl_resource_for_each_safe(resource, tmp, &client->pointers) {
		wl_resource_destroy(resource);
	}
	wl_resource_for_each_safe(resource, tmp, &client->keyboards) {
		wl_resource_destroy(resource);
	}
	wl_resource_for_each_safe(resource, tmp, &client->touches) {
		wl_resource_destroy(resource);
	}
	wl_resource_for_each_safe(resource, tmp, &client->data_devices) {
		// Make the data device inert
		wl_resource_set_user_data(resource, NULL);

		struct wl_list *link = wl_resource_get_link(resource);
		wl_list_remove(link);
		wl_list_init(link);
	}

	wl_list_remove(&client->link);
	free(client);
}

static void seat_handle_release(struct wl_client *client,
		struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static const struct wl_seat_interface seat_impl = {
	.get_pointer = seat_handle_get_pointer,
	.get_keyboard = seat_handle_get_keyboard,
	.get_touch = seat_handle_get_touch,
	.release = seat_handle_release,
};

static void seat_handle_bind(struct wl_client *client, void *_wlr_seat,
		uint32_t version, uint32_t id) {
	struct wlr_seat *wlr_seat = _wlr_seat;
	assert(client && wlr_seat);

	struct wl_resource *wl_resource =
		wl_resource_create(client, &wl_seat_interface, version, id);
	if (wl_resource == NULL) {
		wl_client_post_no_memory(client);
		return;
	}

	struct wlr_seat_client *seat_client =
		wlr_seat_client_for_wl_client(wlr_seat, client);
	if (seat_client == NULL) {
		seat_client = calloc(1, sizeof(struct wlr_seat_client));
		if (seat_client == NULL) {
			wl_resource_destroy(wl_resource);
			wl_client_post_no_memory(client);
			return;
		}

		seat_client->client = client;
		seat_client->seat = wlr_seat;
		wl_list_init(&seat_client->resources);
		wl_list_init(&seat_client->pointers);
		wl_list_init(&seat_client->keyboards);
		wl_list_init(&seat_client->touches);
		wl_list_init(&seat_client->data_devices);
		wl_signal_init(&seat_client->events.destroy);

		wl_list_insert(&wlr_seat->clients, &seat_client->link);
	}

	wl_resource_set_implementation(wl_resource, &seat_impl,
		seat_client, seat_client_handle_resource_destroy);
	wl_list_insert(&seat_client->resources, wl_resource_get_link(wl_resource));
	if (version >= WL_SEAT_NAME_SINCE_VERSION) {
		wl_seat_send_name(wl_resource, wlr_seat->name);
	}
	wl_seat_send_capabilities(wl_resource, wlr_seat->capabilities);
}

void wlr_seat_destroy(struct wlr_seat *seat) {
	if (!seat) {
		return;
	}

	wlr_seat_pointer_clear_focus(seat);
	wlr_seat_keyboard_clear_focus(seat);

	struct wlr_touch_point *point;
	wl_list_for_each(point, &seat->touch_state.touch_points, link) {
		wlr_seat_touch_point_clear_focus(seat, 0, point->touch_id);
	}

	wlr_signal_emit_safe(&seat->events.destroy, seat);

	wl_list_remove(&seat->display_destroy.link);

	wlr_data_source_destroy(seat->selection_source);
	wlr_primary_selection_source_destroy(seat->primary_selection_source);

	struct wlr_seat_client *client, *tmp;
	wl_list_for_each_safe(client, tmp, &seat->clients, link) {
		struct wl_resource *resource, *next;
		/* wl_resource_for_each_safe isn't safe to use here, because the last
		 * wl_resource_destroy will also destroy the head we cannot do the last
		 * 'next' update that usually is harmless here.
		 * Work around this by breaking one step ahead
		 */
		wl_resource_for_each_safe(resource, next, &client->resources) {
			// will destroy other resources as well
			wl_resource_destroy(resource);
			if (wl_resource_get_link(next) == &client->resources) {
				break;
			}
		}
	}

	wlr_global_destroy_safe(seat->global);
	free(seat->pointer_state.default_grab);
	free(seat->keyboard_state.default_grab);
	free(seat->touch_state.default_grab);
	free(seat->name);
	free(seat);
}

static void handle_display_destroy(struct wl_listener *listener, void *data) {
	struct wlr_seat *seat =
		wl_container_of(listener, seat, display_destroy);
	wlr_seat_destroy(seat);
}

struct wlr_seat *wlr_seat_create(struct wl_display *display, const char *name) {
	struct wlr_seat *seat = calloc(1, sizeof(struct wlr_seat));
	if (!seat) {
		return NULL;
	}

	// pointer state
	seat->pointer_state.seat = seat;
	wl_list_init(&seat->pointer_state.surface_destroy.link);

	struct wlr_seat_pointer_grab *pointer_grab =
		calloc(1, sizeof(struct wlr_seat_pointer_grab));
	if (!pointer_grab) {
		free(seat);
		return NULL;
	}
	pointer_grab->interface = &default_pointer_grab_impl;
	pointer_grab->seat = seat;
	seat->pointer_state.default_grab = pointer_grab;
	seat->pointer_state.grab = pointer_grab;

	wl_signal_init(&seat->pointer_state.events.focus_change);

	// keyboard state
	struct wlr_seat_keyboard_grab *keyboard_grab =
		calloc(1, sizeof(struct wlr_seat_keyboard_grab));
	if (!keyboard_grab) {
		free(pointer_grab);
		free(seat);
		return NULL;
	}
	keyboard_grab->interface = &default_keyboard_grab_impl;
	keyboard_grab->seat = seat;
	seat->keyboard_state.default_grab = keyboard_grab;
	seat->keyboard_state.grab = keyboard_grab;

	seat->keyboard_state.seat = seat;
	wl_list_init(&seat->keyboard_state.surface_destroy.link);

	wl_signal_init(&seat->keyboard_state.events.focus_change);

	// touch state
	struct wlr_seat_touch_grab *touch_grab =
		calloc(1, sizeof(struct wlr_seat_touch_grab));
	if (!touch_grab) {
		free(pointer_grab);
		free(keyboard_grab);
		free(seat);
		return NULL;
	}
	touch_grab->interface = &default_touch_grab_impl;
	touch_grab->seat = seat;
	seat->touch_state.default_grab = touch_grab;
	seat->touch_state.grab = touch_grab;

	seat->touch_state.seat = seat;
	wl_list_init(&seat->touch_state.touch_points);

	seat->global = wl_global_create(display, &wl_seat_interface,
		SEAT_VERSION, seat, seat_handle_bind);
	if (seat->global == NULL) {
		free(touch_grab);
		free(pointer_grab);
		free(keyboard_grab);
		free(seat);
		return NULL;
	}
	seat->display = display;
	seat->name = strdup(name);
	wl_list_init(&seat->clients);
	wl_list_init(&seat->selection_offers);
	wl_list_init(&seat->drag_offers);

	wl_signal_init(&seat->events.request_start_drag);
	wl_signal_init(&seat->events.start_drag);

	wl_signal_init(&seat->events.request_set_cursor);

	wl_signal_init(&seat->events.request_set_selection);
	wl_signal_init(&seat->events.set_selection);
	wl_signal_init(&seat->events.request_set_primary_selection);
	wl_signal_init(&seat->events.set_primary_selection);

	wl_signal_init(&seat->events.pointer_grab_begin);
	wl_signal_init(&seat->events.pointer_grab_end);

	wl_signal_init(&seat->events.keyboard_grab_begin);
	wl_signal_init(&seat->events.keyboard_grab_end);

	wl_signal_init(&seat->events.touch_grab_begin);
	wl_signal_init(&seat->events.touch_grab_end);

	wl_signal_init(&seat->events.destroy);

	seat->display_destroy.notify = handle_display_destroy;
	wl_display_add_destroy_listener(display, &seat->display_destroy);

	return seat;
}

struct wlr_seat_client *wlr_seat_client_for_wl_client(struct wlr_seat *wlr_seat,
		struct wl_client *wl_client) {
	struct wlr_seat_client *seat_client;
	wl_list_for_each(seat_client, &wlr_seat->clients, link) {
		if (seat_client->client == wl_client) {
			return seat_client;
		}
	}
	return NULL;
}

void wlr_seat_set_capabilities(struct wlr_seat *wlr_seat,
		uint32_t capabilities) {
	// if the capabilities haven't changed (i.e a redundant mouse was removed),
	// we don't actually have to do anything
	if (capabilities == wlr_seat->capabilities) {
		return;
	}
	wlr_seat->capabilities = capabilities;
	wlr_seat->accumulated_capabilities |= capabilities;

	struct wlr_seat_client *client;
	wl_list_for_each(client, &wlr_seat->clients, link) {
		// Make resources inert if necessary
		if ((capabilities & WL_SEAT_CAPABILITY_POINTER) == 0) {
			struct wlr_seat_client *focused_client =
				wlr_seat->pointer_state.focused_client;
			struct wlr_surface *focused_surface =
				wlr_seat->pointer_state.focused_surface;

			if (focused_client != NULL && focused_surface != NULL) {
				seat_client_send_pointer_leave_raw(focused_client, focused_surface);
			}

			// Note: we don't set focused client/surface to NULL since we need
			// them to send the enter event if the pointer is recreated

			struct wl_resource *resource, *tmp;
			wl_resource_for_each_safe(resource, tmp, &client->pointers) {
				seat_client_destroy_pointer(resource);
			}
		}
		if ((capabilities & WL_SEAT_CAPABILITY_KEYBOARD) == 0) {
			struct wlr_seat_client *focused_client =
				wlr_seat->keyboard_state.focused_client;
			struct wlr_surface *focused_surface =
				wlr_seat->keyboard_state.focused_surface;

			if (focused_client != NULL && focused_surface != NULL) {
				seat_client_send_keyboard_leave_raw(focused_client,
						focused_surface);
			}

			// Note: we don't set focused client/surface to NULL since we need
			// them to send the enter event if the keyboard is recreated

			struct wl_resource *resource, *tmp;
			wl_resource_for_each_safe(resource, tmp, &client->keyboards) {
				seat_client_destroy_keyboard(resource);
			}
		}
		if ((capabilities & WL_SEAT_CAPABILITY_TOUCH) == 0) {
			struct wl_resource *resource, *tmp;
			wl_resource_for_each_safe(resource, tmp, &client->touches) {
				seat_client_destroy_touch(resource);
			}
		}

		struct wl_resource *resource;
		wl_resource_for_each(resource, &client->resources) {
			wl_seat_send_capabilities(resource, capabilities);
		}
	}
}

void wlr_seat_set_name(struct wlr_seat *wlr_seat, const char *name) {
	free(wlr_seat->name);
	wlr_seat->name = strdup(name);
	struct wlr_seat_client *client;
	wl_list_for_each(client, &wlr_seat->clients, link) {
		struct wl_resource *resource;
		wl_resource_for_each(resource, &client->resources) {
			wl_seat_send_name(resource, name);
		}
	}
}

struct wlr_seat_client *wlr_seat_client_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource, &wl_seat_interface,
		&seat_impl));
	return wl_resource_get_user_data(resource);
}

bool wlr_seat_validate_grab_serial(struct wlr_seat *seat, uint32_t serial) {
	// TODO
	//return serial == seat->pointer_state.grab_serial ||
	//	serial == seat->touch_state.grab_serial;
	return true;
}

uint32_t wlr_seat_client_next_serial(struct wlr_seat_client *client) {
	uint32_t serial = wl_display_next_serial(wl_client_get_display(client->client));
	struct wlr_serial_ringset *set = &client->serials;

	if (set->count == 0) {
		set->data[0].min_incl = serial;
		set->data[0].max_incl = serial;
		set->count = 1;
		set->end = 0;
	} else if (set->data[set->end].max_incl + 1 != serial) {
		if (set->count < WLR_SERIAL_RINGSET_SIZE) {
			set->count++;
		}
		set->end = (set->end + 1) % WLR_SERIAL_RINGSET_SIZE;
		set->data[set->end].min_incl = serial;
		set->data[set->end].max_incl = serial;
	} else {
		set->data[set->end].max_incl = serial;
	}

	return serial;
}

bool wlr_seat_client_validate_event_serial(struct wlr_seat_client *client, uint32_t serial) {
	uint32_t cur = wl_display_get_serial(wl_client_get_display(client->client));
	struct wlr_serial_ringset *set = &client->serials;
	uint32_t rev_dist = cur - serial;

	if (rev_dist >= UINT32_MAX / 2) {
		// serial is closer to being 'newer' instead of 'older' than
		// the current serial, so it's either invalid or incredibly old
		return false;
	}

	for (int i = 0; i < set->count; i++) {
		int j = (set->end - i + WLR_SERIAL_RINGSET_SIZE) % WLR_SERIAL_RINGSET_SIZE;
		if (rev_dist < cur - set->data[j].max_incl) {
			return false;
		}
		if (rev_dist <= cur - set->data[j].min_incl) {
			return true;
		}
	}

	// Iff the set is full, then `rev_dist` is large enough that serial
	// could already have been recycled out of the set.
	return set->count == WLR_SERIAL_RINGSET_SIZE;
}
