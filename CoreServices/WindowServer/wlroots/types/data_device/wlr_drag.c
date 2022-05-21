#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <wayland-server-core.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/util/log.h>
#include "types/wlr_data_device.h"
#include "util/signal.h"

static void drag_handle_seat_client_destroy(struct wl_listener *listener,
		void *data) {
	struct wlr_drag *drag =
		wl_container_of(listener, drag, seat_client_destroy);

	drag->focus_client = NULL;
	wl_list_remove(&drag->seat_client_destroy.link);
}

static void drag_set_focus(struct wlr_drag *drag,
		struct wlr_surface *surface, double sx, double sy) {
	if (drag->focus == surface) {
		return;
	}

	if (drag->focus_client) {
		wl_list_remove(&drag->seat_client_destroy.link);

		// If we're switching focus to another client, we want to destroy all
		// offers without destroying the source. If the drag operation ends, we
		// want to keep the offer around for the data transfer.
		struct wlr_data_offer *offer, *tmp;
		wl_list_for_each_safe(offer, tmp,
				&drag->focus_client->seat->drag_offers, link) {
			struct wl_client *client = wl_resource_get_client(offer->resource);
			if (!drag->dropped && offer->source == drag->source &&
					client == drag->focus_client->client) {
				offer->source = NULL;
				data_offer_destroy(offer);
			}
		}

		struct wl_resource *resource;
		wl_resource_for_each(resource, &drag->focus_client->data_devices) {
			wl_data_device_send_leave(resource);
		}

		drag->focus_client = NULL;
		drag->focus = NULL;
	}

	if (!surface) {
		goto out;
	}

	if (!drag->source &&
			wl_resource_get_client(surface->resource) !=
			drag->seat_client->client) {
		goto out;
	}

	struct wlr_seat_client *focus_client = wlr_seat_client_for_wl_client(
		drag->seat_client->seat, wl_resource_get_client(surface->resource));
	if (!focus_client) {
		goto out;
	}

	if (drag->source != NULL) {
		drag->source->accepted = false;

		uint32_t serial =
			wl_display_next_serial(drag->seat_client->seat->display);

		struct wl_resource *device_resource;
		wl_resource_for_each(device_resource, &focus_client->data_devices) {
			struct wlr_data_offer *offer = data_offer_create(device_resource,
				drag->source, WLR_DATA_OFFER_DRAG);
			if (offer == NULL) {
				wl_resource_post_no_memory(device_resource);
				return;
			}

			data_offer_update_action(offer);

			if (wl_resource_get_version(offer->resource) >=
					WL_DATA_OFFER_SOURCE_ACTIONS_SINCE_VERSION) {
				wl_data_offer_send_source_actions(offer->resource,
					drag->source->actions);
			}

			wl_data_device_send_enter(device_resource, serial,
				surface->resource,
				wl_fixed_from_double(sx), wl_fixed_from_double(sy),
				offer->resource);
		}
	}

	drag->focus = surface;
	drag->focus_client = focus_client;
	drag->seat_client_destroy.notify = drag_handle_seat_client_destroy;
	wl_signal_add(&focus_client->events.destroy, &drag->seat_client_destroy);

out:
	wlr_signal_emit_safe(&drag->events.focus, drag);
}

static void drag_icon_set_mapped(struct wlr_drag_icon *icon, bool mapped) {
	if (mapped && !icon->mapped) {
		icon->mapped = true;
		wlr_signal_emit_safe(&icon->events.map, icon);
	} else if (!mapped && icon->mapped) {
		wlr_signal_emit_safe(&icon->events.unmap, icon);
		icon->mapped = false;
	}
}

static void drag_icon_destroy(struct wlr_drag_icon *icon);

static void drag_destroy(struct wlr_drag *drag) {
	if (drag->cancelling) {
		return;
	}
	drag->cancelling = true;

	if (drag->started) {
		wlr_seat_keyboard_end_grab(drag->seat);
		switch (drag->grab_type) {
		case WLR_DRAG_GRAB_KEYBOARD:
			break;
		case WLR_DRAG_GRAB_KEYBOARD_POINTER:
			wlr_seat_pointer_end_grab(drag->seat);
			break;
		case WLR_DRAG_GRAB_KEYBOARD_TOUCH:
			wlr_seat_touch_end_grab(drag->seat);
			break;
		}
	}

	if (drag->started) {
		drag_set_focus(drag, NULL, 0, 0);

		assert(drag->seat->drag == drag);
		drag->seat->drag = NULL;
	}

	// We issue destroy after ending the grab to allow focus changes.
	// Furthermore, we wait until after clearing the drag focus in order
	// to ensure that the wl_data_device.leave is sent before emitting the
	// signal. This allows e.g. wl_pointer.enter to be sent in the destroy
	// signal handler.
	wlr_signal_emit_safe(&drag->events.destroy, drag);

	if (drag->source) {
		wl_list_remove(&drag->source_destroy.link);
	}

	drag_icon_destroy(drag->icon);
	free(drag);
}

static void drag_handle_pointer_enter(struct wlr_seat_pointer_grab *grab,
		struct wlr_surface *surface, double sx, double sy) {
	struct wlr_drag *drag = grab->data;
	drag_set_focus(drag, surface, sx, sy);
}

static void drag_handle_pointer_clear_focus(struct wlr_seat_pointer_grab *grab) {
	struct wlr_drag *drag = grab->data;
	drag_set_focus(drag, NULL, 0, 0);
}

static void drag_handle_pointer_motion(struct wlr_seat_pointer_grab *grab,
		uint32_t time, double sx, double sy) {
	struct wlr_drag *drag = grab->data;
	if (drag->focus != NULL && drag->focus_client != NULL) {
		struct wl_resource *resource;
		wl_resource_for_each(resource, &drag->focus_client->data_devices) {
			wl_data_device_send_motion(resource, time, wl_fixed_from_double(sx),
				wl_fixed_from_double(sy));
		}

		struct wlr_drag_motion_event event = {
			.drag = drag,
			.time = time,
			.sx = sx,
			.sy = sy,
		};
		wlr_signal_emit_safe(&drag->events.motion, &event);
	}
}

static void drag_drop(struct wlr_drag *drag, uint32_t time) {
	assert(drag->focus_client);

	drag->dropped = true;

	struct wl_resource *resource;
	wl_resource_for_each(resource, &drag->focus_client->data_devices) {
		wl_data_device_send_drop(resource);
	}
	if (drag->source) {
		wlr_data_source_dnd_drop(drag->source);
	}

	struct wlr_drag_drop_event event = {
		.drag = drag,
		.time = time,
	};
	wlr_signal_emit_safe(&drag->events.drop, &event);
}

static uint32_t drag_handle_pointer_button(struct wlr_seat_pointer_grab *grab,
		uint32_t time, uint32_t button, uint32_t state) {
	struct wlr_drag *drag = grab->data;

	if (drag->source &&
			grab->seat->pointer_state.grab_button == button &&
			state == WL_POINTER_BUTTON_STATE_RELEASED) {
		if (drag->focus_client && drag->source->current_dnd_action &&
				drag->source->accepted) {
			drag_drop(drag, time);
		} else if (drag->source->impl->dnd_finish) {
			// This will end the grab and free `drag`
			wlr_data_source_destroy(drag->source);
			return 0;
		}
	}

	if (grab->seat->pointer_state.button_count == 0 &&
			state == WL_POINTER_BUTTON_STATE_RELEASED) {
		drag_destroy(drag);
	}

	return 0;
}

static void drag_handle_pointer_axis(struct wlr_seat_pointer_grab *grab,
		uint32_t time, enum wlr_axis_orientation orientation, double value,
		int32_t value_discrete, enum wlr_axis_source source) {
	// This space is intentionally left blank
}

static void drag_handle_pointer_cancel(struct wlr_seat_pointer_grab *grab) {
	struct wlr_drag *drag = grab->data;
	drag_destroy(drag);
}

static const struct wlr_pointer_grab_interface
		data_device_pointer_drag_interface = {
	.enter = drag_handle_pointer_enter,
	.clear_focus = drag_handle_pointer_clear_focus,
	.motion = drag_handle_pointer_motion,
	.button = drag_handle_pointer_button,
	.axis = drag_handle_pointer_axis,
	.cancel = drag_handle_pointer_cancel,
};

static uint32_t drag_handle_touch_down(struct wlr_seat_touch_grab *grab,
		uint32_t time, struct wlr_touch_point *point) {
	// eat the event
	return 0;
}

static void drag_handle_touch_up(struct wlr_seat_touch_grab *grab,
		uint32_t time, struct wlr_touch_point *point) {
	struct wlr_drag *drag = grab->data;
	if (drag->grab_touch_id != point->touch_id) {
		return;
	}

	if (drag->focus_client) {
		drag_drop(drag, time);
	}

	drag_destroy(drag);
}

static void drag_handle_touch_motion(struct wlr_seat_touch_grab *grab,
		uint32_t time, struct wlr_touch_point *point) {
	struct wlr_drag *drag = grab->data;
	if (drag->focus && drag->focus_client) {
		struct wl_resource *resource;
		wl_resource_for_each(resource, &drag->focus_client->data_devices) {
			wl_data_device_send_motion(resource, time,
				wl_fixed_from_double(point->sx),
				wl_fixed_from_double(point->sy));
		}
	}
}

static void drag_handle_touch_enter(struct wlr_seat_touch_grab *grab,
		uint32_t time, struct wlr_touch_point *point) {
	struct wlr_drag *drag = grab->data;
	drag_set_focus(drag, point->focus_surface, point->sx, point->sy);
}

static void drag_handle_touch_cancel(struct wlr_seat_touch_grab *grab) {
	struct wlr_drag *drag = grab->data;
	drag_destroy(drag);
}

static const struct wlr_touch_grab_interface
		data_device_touch_drag_interface = {
	.down = drag_handle_touch_down,
	.up = drag_handle_touch_up,
	.motion = drag_handle_touch_motion,
	.enter = drag_handle_touch_enter,
	.cancel = drag_handle_touch_cancel,
};

static void drag_handle_keyboard_enter(struct wlr_seat_keyboard_grab *grab,
		struct wlr_surface *surface, uint32_t keycodes[], size_t num_keycodes,
		struct wlr_keyboard_modifiers *modifiers) {
	// nothing has keyboard focus during drags
}

static void drag_handle_keyboard_clear_focus(struct wlr_seat_keyboard_grab *grab) {
	// nothing has keyboard focus during drags
}

static void drag_handle_keyboard_key(struct wlr_seat_keyboard_grab *grab,
		uint32_t time, uint32_t key, uint32_t state) {
	// no keyboard input during drags
}

static void drag_handle_keyboard_modifiers(struct wlr_seat_keyboard_grab *grab,
		struct wlr_keyboard_modifiers *modifiers) {
	//struct wlr_keyboard *keyboard = grab->seat->keyboard_state.keyboard;
	// TODO change the dnd action based on what modifier is pressed on the
	// keyboard
}

static void drag_handle_keyboard_cancel(struct wlr_seat_keyboard_grab *grab) {
	struct wlr_drag *drag = grab->data;
	drag_destroy(drag);
}

static const struct wlr_keyboard_grab_interface
		data_device_keyboard_drag_interface = {
	.enter = drag_handle_keyboard_enter,
	.clear_focus = drag_handle_keyboard_clear_focus,
	.key = drag_handle_keyboard_key,
	.modifiers = drag_handle_keyboard_modifiers,
	.cancel = drag_handle_keyboard_cancel,
};

static void drag_handle_icon_destroy(struct wl_listener *listener, void *data) {
	struct wlr_drag *drag = wl_container_of(listener, drag, icon_destroy);
	drag->icon = NULL;
}

static void drag_handle_drag_source_destroy(struct wl_listener *listener,
		void *data) {
	struct wlr_drag *drag = wl_container_of(listener, drag, source_destroy);
	drag_destroy(drag);
}


static void drag_icon_destroy(struct wlr_drag_icon *icon) {
	if (icon == NULL) {
		return;
	}
	drag_icon_set_mapped(icon, false);
	wlr_signal_emit_safe(&icon->events.destroy, icon);
	icon->surface->role_data = NULL;
	wl_list_remove(&icon->surface_destroy.link);
	free(icon);
}

static void drag_icon_handle_surface_destroy(struct wl_listener *listener,
		void *data) {
	struct wlr_drag_icon *icon =
		wl_container_of(listener, icon, surface_destroy);
	drag_icon_destroy(icon);
}

static void drag_icon_surface_role_commit(struct wlr_surface *surface) {
	assert(surface->role == &drag_icon_surface_role);
	struct wlr_drag_icon *icon = surface->role_data;
	if (icon == NULL) {
		return;
	}

	drag_icon_set_mapped(icon, wlr_surface_has_buffer(surface));
}

const struct wlr_surface_role drag_icon_surface_role = {
	.name = "wl_data_device-icon",
	.commit = drag_icon_surface_role_commit,
};

static struct wlr_drag_icon *drag_icon_create(struct wlr_drag *drag,
		struct wlr_surface *surface) {
	struct wlr_drag_icon *icon = calloc(1, sizeof(struct wlr_drag_icon));
	if (!icon) {
		return NULL;
	}

	icon->drag = drag;
	icon->surface = surface;

	wl_signal_init(&icon->events.map);
	wl_signal_init(&icon->events.unmap);
	wl_signal_init(&icon->events.destroy);

	wl_signal_add(&icon->surface->events.destroy, &icon->surface_destroy);
	icon->surface_destroy.notify = drag_icon_handle_surface_destroy;

	icon->surface->role_data = icon;

	if (wlr_surface_has_buffer(surface)) {
		drag_icon_set_mapped(icon, true);
	}

	return icon;
}


struct wlr_drag *wlr_drag_create(struct wlr_seat_client *seat_client,
		struct wlr_data_source *source, struct wlr_surface *icon_surface) {
	struct wlr_drag *drag = calloc(1, sizeof(struct wlr_drag));
	if (drag == NULL) {
		return NULL;
	}

	wl_signal_init(&drag->events.focus);
	wl_signal_init(&drag->events.motion);
	wl_signal_init(&drag->events.drop);
	wl_signal_init(&drag->events.destroy);

	drag->seat = seat_client->seat;
	drag->seat_client = seat_client;

	if (icon_surface) {
		struct wlr_drag_icon *icon = drag_icon_create(drag, icon_surface);
		if (icon == NULL) {
			free(drag);
			return NULL;
		}

		drag->icon = icon;

		drag->icon_destroy.notify = drag_handle_icon_destroy;
		wl_signal_add(&icon->events.destroy, &drag->icon_destroy);
	}

	drag->source = source;
	if (source != NULL) {
		drag->source_destroy.notify = drag_handle_drag_source_destroy;
		wl_signal_add(&source->events.destroy, &drag->source_destroy);
	}

	drag->pointer_grab.data = drag;
	drag->pointer_grab.interface = &data_device_pointer_drag_interface;

	drag->touch_grab.data = drag;
	drag->touch_grab.interface = &data_device_touch_drag_interface;

	drag->keyboard_grab.data = drag;
	drag->keyboard_grab.interface = &data_device_keyboard_drag_interface;

	return drag;
}

void wlr_seat_request_start_drag(struct wlr_seat *seat, struct wlr_drag *drag,
		struct wlr_surface *origin, uint32_t serial) {
	assert(drag->seat == seat);

	if (seat->drag != NULL) {
		wlr_log(WLR_DEBUG, "Rejecting start_drag request, "
			"another drag-and-drop operation is already in progress");
		return;
	}

	struct wlr_seat_request_start_drag_event event = {
		.drag = drag,
		.origin = origin,
		.serial = serial,
	};
	wlr_signal_emit_safe(&seat->events.request_start_drag, &event);
}

static void seat_handle_drag_source_destroy(struct wl_listener *listener,
		void *data) {
	struct wlr_seat *seat =
		wl_container_of(listener, seat, drag_source_destroy);
	wl_list_remove(&seat->drag_source_destroy.link);
	seat->drag_source = NULL;
}

void wlr_seat_start_drag(struct wlr_seat *seat, struct wlr_drag *drag,
		uint32_t serial) {
	assert(drag->seat == seat);
	assert(!drag->started);
	drag->started = true;

	wlr_seat_keyboard_start_grab(seat, &drag->keyboard_grab);

	seat->drag = drag;
	seat->drag_serial = serial;

	// We need to destroy the previous source, because listeners only expect one
	// active drag source at a time.
	wlr_data_source_destroy(seat->drag_source);
	seat->drag_source = drag->source;
	if (drag->source != NULL) {
		seat->drag_source_destroy.notify = seat_handle_drag_source_destroy;
		wl_signal_add(&drag->source->events.destroy, &seat->drag_source_destroy);
	}

	wlr_signal_emit_safe(&seat->events.start_drag, drag);
}

void wlr_seat_start_pointer_drag(struct wlr_seat *seat, struct wlr_drag *drag,
		uint32_t serial) {
	drag->grab_type = WLR_DRAG_GRAB_KEYBOARD_POINTER;

	wlr_seat_pointer_clear_focus(seat);
	wlr_seat_pointer_start_grab(seat, &drag->pointer_grab);

	wlr_seat_start_drag(seat, drag, serial);
}

void wlr_seat_start_touch_drag(struct wlr_seat *seat, struct wlr_drag *drag,
		uint32_t serial, struct wlr_touch_point *point) {
	drag->grab_type = WLR_DRAG_GRAB_KEYBOARD_TOUCH;
	drag->grab_touch_id = seat->touch_state.grab_id;
	drag->touch_id = point->touch_id;

	wlr_seat_touch_start_grab(seat, &drag->touch_grab);
	drag_set_focus(drag, point->surface, point->sx, point->sy);

	wlr_seat_start_drag(seat, drag, serial);
}
