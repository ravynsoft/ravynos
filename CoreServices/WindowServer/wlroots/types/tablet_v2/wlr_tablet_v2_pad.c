#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "tablet-unstable-v2-protocol.h"
#include <assert.h>
#include <stdlib.h>
#include <types/wlr_tablet_v2.h>
#include <wayland-util.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_tablet_tool.h>
#include <wlr/types/wlr_tablet_pad.h>
#include <wlr/types/wlr_tablet_v2.h>
#include <wlr/util/log.h>

static const struct wlr_tablet_pad_v2_grab_interface default_pad_grab_interface;

struct tablet_pad_auxiliary_user_data {
	struct wlr_tablet_pad_client_v2 *pad;
	size_t index;
};

static void handle_tablet_pad_v2_destroy(struct wl_client *client,
		struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static void destroy_tablet_pad_ring_v2(struct wl_resource *resource) {
	struct tablet_pad_auxiliary_user_data *aux = wl_resource_get_user_data(resource);

	if (!aux) {
		return;
	}

	aux->pad->rings[aux->index] = NULL;
	free(aux);
	wl_resource_set_user_data(resource, NULL);
}

static void handle_tablet_pad_ring_v2_set_feedback(struct wl_client *client,
		struct wl_resource *resource, const char *description,
		uint32_t serial) {
	struct tablet_pad_auxiliary_user_data *aux = wl_resource_get_user_data(resource);
	if (!aux) {
		return;
	}

	struct wlr_tablet_v2_event_feedback evt = {
		.serial = serial,
		.description = description,
		.index = aux->index
		};

	wl_signal_emit(&aux->pad->pad->events.ring_feedback, &evt);
}

static void handle_tablet_pad_ring_v2_destroy(struct wl_client *client,
		struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static const struct zwp_tablet_pad_ring_v2_interface tablet_pad_ring_impl = {
	.set_feedback = handle_tablet_pad_ring_v2_set_feedback,
	.destroy = handle_tablet_pad_ring_v2_destroy,
};

static void destroy_tablet_pad_strip_v2(struct wl_resource *resource) {
	struct tablet_pad_auxiliary_user_data *aux = wl_resource_get_user_data(resource);
	if (!aux) {
		return;
	}

	aux->pad->strips[aux->index] = NULL;
	free(aux);
	wl_resource_set_user_data(resource, NULL);
}

static void handle_tablet_pad_strip_v2_set_feedback(struct wl_client *client,
		struct wl_resource *resource, const char *description,
		uint32_t serial) {
	struct tablet_pad_auxiliary_user_data *aux = wl_resource_get_user_data(resource);
	if (!aux) {
		return;
	}

	struct wlr_tablet_v2_event_feedback evt = {
		.serial = serial,
		.description = description,
		.index = aux->index
		};

	wl_signal_emit(&aux->pad->pad->events.strip_feedback, &evt);
}

static void handle_tablet_pad_strip_v2_destroy(struct wl_client *client,
		struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static const struct zwp_tablet_pad_strip_v2_interface tablet_pad_strip_impl = {
	.set_feedback = handle_tablet_pad_strip_v2_set_feedback,
	.destroy = handle_tablet_pad_strip_v2_destroy,
};

static void handle_tablet_pad_v2_set_feedback( struct wl_client *client,
		struct wl_resource *resource, uint32_t button,
		const char *description, uint32_t serial) {
	struct wlr_tablet_pad_client_v2 *pad = tablet_pad_client_from_resource(resource);
	if (!pad) {
		return;
	}

	struct wlr_tablet_v2_event_feedback evt = {
		.serial = serial,
		.index = button,
		.description = description,
		};

	wl_signal_emit(&pad->pad->events.button_feedback, &evt);
}

static const struct zwp_tablet_pad_v2_interface tablet_pad_impl = {
	.set_feedback = handle_tablet_pad_v2_set_feedback,
	.destroy = handle_tablet_pad_v2_destroy,
};

static void destroy_tablet_pad_group_v2(struct wl_resource *resource) {
	struct tablet_pad_auxiliary_user_data *aux = wl_resource_get_user_data(resource);

	if (!aux) {
		return;
	}

	aux->pad->groups[aux->index] = NULL;
	free(aux);
	wl_resource_set_user_data(resource, NULL);
}

void destroy_tablet_pad_v2(struct wl_resource *resource) {
	struct wlr_tablet_pad_client_v2 *pad =
		tablet_pad_client_from_resource(resource);

	if (!pad) {
		return;
	}

	wl_list_remove(&pad->seat_link);
	wl_list_remove(&pad->pad_link);

	/* This isn't optimal, if the client destroys the resources in another
	 * order, it will be disconnected.
	 * But this makes things *way* easier for us, and (untested) I doubt
	 * clients will destroy it in another order.
	 */
	for (size_t i = 0; i < pad->group_count; ++i) {
		if (pad->groups[i]) {
			destroy_tablet_pad_group_v2(pad->groups[i]);
		}
	}
	free(pad->groups);

	for (size_t i = 0; i < pad->ring_count; ++i) {
		if (pad->rings[i]) {
			destroy_tablet_pad_ring_v2(pad->rings[i]);
		}
	}
	free(pad->rings);

	for (size_t i = 0; i < pad->strip_count; ++i) {
		if (pad->strips[i]) {
			destroy_tablet_pad_strip_v2(pad->strips[i]);
		}
	}
	free(pad->strips);

	if (pad->pad->current_client == pad) {
		pad->pad->current_client = NULL;
	}
	free(pad);
	wl_resource_set_user_data(resource, NULL);
}

static void handle_tablet_pad_group_v2_destroy(struct wl_client *client,
		struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static const struct zwp_tablet_pad_group_v2_interface tablet_pad_group_impl = {
	.destroy = handle_tablet_pad_group_v2_destroy,
};

static void add_tablet_pad_group(struct wlr_tablet_v2_tablet_pad *pad,
		struct wlr_tablet_pad_client_v2 *client,
		struct wlr_tablet_pad_group *group, size_t index) {

	uint32_t version = wl_resource_get_version(client->resource);
	client->groups[index] = wl_resource_create(client->client,
		&zwp_tablet_pad_group_v2_interface, version, 0);
	if (!client->groups[index]) {
		wl_client_post_no_memory(client->client);
		return;
	}
	struct tablet_pad_auxiliary_user_data *user_data =
		calloc(1, sizeof(struct tablet_pad_auxiliary_user_data));
	if (!user_data) {
		wl_client_post_no_memory(client->client);
		return;
	}
	user_data->pad = client;
	user_data->index = index;
	wl_resource_set_implementation(client->groups[index], &tablet_pad_group_impl,
		user_data, destroy_tablet_pad_group_v2);

	zwp_tablet_pad_v2_send_group(client->resource, client->groups[index]);
	zwp_tablet_pad_group_v2_send_modes(client->groups[index], group->mode_count);

	struct wl_array button_array;
	wl_array_init(&button_array);
	wl_array_add(&button_array, group->button_count * sizeof(int));
	memcpy(button_array.data, group->buttons, group->button_count * sizeof(int));
	zwp_tablet_pad_group_v2_send_buttons(client->groups[index], &button_array);
	wl_array_release(&button_array);

	client->strip_count = group->strip_count;
	for (size_t i = 0; i < group->strip_count; ++i) {
		size_t strip = group->strips[i];
		struct tablet_pad_auxiliary_user_data *user_data =
			calloc(1, sizeof(struct tablet_pad_auxiliary_user_data));
		if (!user_data) {
			wl_client_post_no_memory(client->client);
			return;
		}
		user_data->pad = client;
		user_data->index = strip;
		client->strips[strip] = wl_resource_create(client->client,
			&zwp_tablet_pad_strip_v2_interface, version, 0);
		if (!client->strips[strip]) {
			free(user_data);
			wl_client_post_no_memory(client->client);
			return;
		}
		wl_resource_set_implementation(client->strips[strip],
			&tablet_pad_strip_impl, user_data, destroy_tablet_pad_strip_v2);
		zwp_tablet_pad_group_v2_send_strip(client->groups[index],
			client->strips[strip]);
	}

	client->ring_count = group->ring_count;
	for (size_t i = 0; i < group->ring_count; ++i) {
		size_t ring = group->rings[i];
		struct tablet_pad_auxiliary_user_data *user_data =
			calloc(1, sizeof(struct tablet_pad_auxiliary_user_data));
		if (!user_data) {
			wl_client_post_no_memory(client->client);
			return;
		}
		user_data->pad = client;
		user_data->index = ring;
		client->rings[ring] = wl_resource_create(client->client,
			&zwp_tablet_pad_ring_v2_interface, version, 0);
		if (!client->rings[ring]) {
			free(user_data);
			wl_client_post_no_memory(client->client);
			return;
		}
		wl_resource_set_implementation(client->rings[ring],
			&tablet_pad_ring_impl, user_data, destroy_tablet_pad_ring_v2);
		zwp_tablet_pad_group_v2_send_ring(client->groups[index],
			client->rings[ring]);
	}

	zwp_tablet_pad_group_v2_send_done(client->groups[index]);
}

void add_tablet_pad_client(struct wlr_tablet_seat_client_v2 *seat,
		struct wlr_tablet_v2_tablet_pad *pad) {
	struct wlr_tablet_pad_client_v2 *client =
		calloc(1, sizeof(struct wlr_tablet_pad_client_v2));
	if (!client) {
		wl_client_post_no_memory(seat->wl_client);
		return;
	}
	client->pad = pad;
	client->seat = seat;

	client->groups = calloc(wl_list_length(&pad->wlr_pad->groups), sizeof(struct wl_resource*));
	if (!client->groups) {
		wl_client_post_no_memory(seat->wl_client);
		free(client);
		return;
	}

	client->rings = calloc(pad->wlr_pad->ring_count, sizeof(struct wl_resource*));
	if (!client->rings) {
		wl_client_post_no_memory(seat->wl_client);
		free(client->groups);
		free(client);
		return;
	}

	client->strips = calloc(pad->wlr_pad->strip_count, sizeof(struct wl_resource*));
	if (!client->strips) {
		wl_client_post_no_memory(seat->wl_client);
		free(client->groups);
		free(client->rings);
		free(client);
		return;
	}

	uint32_t version = wl_resource_get_version(seat->resource);
	client->resource = wl_resource_create(seat->wl_client,
		&zwp_tablet_pad_v2_interface, version, 0);
	if (!client->resource) {
		wl_client_post_no_memory(seat->wl_client);
		free(client->groups);
		free(client->rings);
		free(client->strips);
		free(client);
		return;
	}
	wl_resource_set_implementation(client->resource, &tablet_pad_impl,
		client, destroy_tablet_pad_v2);
	zwp_tablet_seat_v2_send_pad_added(seat->resource, client->resource);
	client->client = seat->wl_client;

	// Send the expected events
	if (pad->wlr_pad->button_count) {
		zwp_tablet_pad_v2_send_buttons(client->resource, pad->wlr_pad->button_count);
	}

	const char **path_ptr;
	wl_array_for_each(path_ptr, &pad->wlr_pad->paths) {
		zwp_tablet_pad_v2_send_path(client->resource, *path_ptr);
	}

	size_t i = 0;
	struct wlr_tablet_pad_group *group;
	client->group_count = pad->group_count;
	wl_list_for_each(group, &pad->wlr_pad->groups, link) {
		add_tablet_pad_group(pad, client, group, i++);
	}

	zwp_tablet_pad_v2_send_done(client->resource);

	wl_list_insert(&seat->pads, &client->seat_link);
	wl_list_insert(&pad->clients, &client->pad_link);
}

static void handle_wlr_tablet_pad_destroy(struct wl_listener *listener, void *data) {
	struct wlr_tablet_v2_tablet_pad *pad =
		wl_container_of(listener, pad, pad_destroy);

	struct wlr_tablet_pad_client_v2 *client;
	struct wlr_tablet_pad_client_v2 *tmp_client;
	wl_list_for_each_safe(client, tmp_client, &pad->clients, pad_link) {
		zwp_tablet_pad_v2_send_removed(client->resource);
		destroy_tablet_pad_v2(client->resource);
	}

	wl_list_remove(&pad->clients);
	wl_list_remove(&pad->link);
	wl_list_remove(&pad->pad_destroy.link);
	wl_list_remove(&pad->events.button_feedback.listener_list);
	wl_list_remove(&pad->events.strip_feedback.listener_list);
	wl_list_remove(&pad->events.ring_feedback.listener_list);
	free(pad);
}

struct wlr_tablet_v2_tablet_pad *wlr_tablet_pad_create(
		struct wlr_tablet_manager_v2 *manager,
		struct wlr_seat *wlr_seat,
		struct wlr_input_device *wlr_device) {
	assert(wlr_device->type == WLR_INPUT_DEVICE_TABLET_PAD);
	struct wlr_tablet_seat_v2 *seat = get_or_create_tablet_seat(manager, wlr_seat);
	if (!seat) {
		return NULL;
	}
	struct wlr_tablet_pad *wlr_pad = wlr_device->tablet_pad;
	struct wlr_tablet_v2_tablet_pad *pad = calloc(1, sizeof(struct wlr_tablet_v2_tablet_pad));
	if (!pad) {
		return NULL;
	}
	pad->default_grab.interface = &default_pad_grab_interface;
	pad->default_grab.pad = pad;
	pad->grab = &pad->default_grab;

	pad->group_count = wl_list_length(&wlr_pad->groups);
	pad->groups = calloc(pad->group_count, sizeof(uint32_t));
	if (!pad->groups) {
		free(pad);
		return NULL;
	}

	pad->wlr_pad = wlr_pad;
	wl_list_init(&pad->clients);

	pad->pad_destroy.notify = handle_wlr_tablet_pad_destroy;
	wl_signal_add(&wlr_device->events.destroy, &pad->pad_destroy);
	wl_list_insert(&seat->pads, &pad->link);

	// We need to create a tablet client for all clients on the seat
	struct wlr_tablet_seat_client_v2 *pos;
	wl_list_for_each(pos, &seat->clients, seat_link) {
		// Tell the clients about the new tool
		add_tablet_pad_client(pos, pad);
	}

	wl_signal_init(&pad->events.button_feedback);
	wl_signal_init(&pad->events.strip_feedback);
	wl_signal_init(&pad->events.ring_feedback);

	return pad;
}

struct wlr_tablet_pad_client_v2 *tablet_pad_client_from_resource(struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource, &zwp_tablet_pad_v2_interface,
		&tablet_pad_impl));
	return wl_resource_get_user_data(resource);
}

/* Actual protocol foo */
uint32_t wlr_send_tablet_v2_tablet_pad_enter(
		struct wlr_tablet_v2_tablet_pad *pad,
		struct wlr_tablet_v2_tablet *tablet,
		struct wlr_surface *surface) {
	struct wl_client *client = wl_resource_get_client(surface->resource);

	struct wlr_tablet_client_v2 *tablet_tmp;
	struct wlr_tablet_client_v2 *tablet_client = NULL;
	wl_list_for_each(tablet_tmp, &tablet->clients, tablet_link) {
		if (tablet_tmp->client == client) {
			tablet_client = tablet_tmp;
			break;
		}
	}

	// Couldn't find the client binding for the surface's client. Either
	// the client didn't bind tablet_v2 at all, or not for the relevant
	// seat
	if (!tablet_client) {
		return 0;
	}

	struct wlr_tablet_pad_client_v2 *pad_tmp = NULL;
	struct wlr_tablet_pad_client_v2 *pad_client = NULL;
	wl_list_for_each(pad_tmp, &pad->clients, pad_link) {
		if (pad_tmp->client == client) {
			pad_client = pad_tmp;
			break;
		}
	}

	// Couldn't find the client binding for the surface's client. Either
	// the client didn't bind tablet_v2 at all, or not for the relevant
	// seat
	if (!pad_client) {
		return 0;
	}

	pad->current_client = pad_client;

	uint32_t serial = wlr_seat_client_next_serial(
		pad_client->seat->seat_client);

	zwp_tablet_pad_v2_send_enter(pad_client->resource, serial,
		tablet_client->resource, surface->resource);

	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	uint32_t time = now.tv_nsec / 1000;

	for (size_t i = 0; i < pad->group_count; ++i) {
		if (pad_client->groups[i]) {
			zwp_tablet_pad_group_v2_send_mode_switch(
				pad_client->groups[i], time, serial, pad->groups[i]);
		}
	}

	return serial;
}

void wlr_send_tablet_v2_tablet_pad_button(
		struct wlr_tablet_v2_tablet_pad *pad, size_t button,
		uint32_t time, enum zwp_tablet_pad_v2_button_state state) {

	if (pad->current_client) {
		zwp_tablet_pad_v2_send_button(pad->current_client->resource,
				time, button, state);
	}
}

void wlr_send_tablet_v2_tablet_pad_strip(struct wlr_tablet_v2_tablet_pad *pad,
		uint32_t strip, double position, bool finger, uint32_t time) {
	if (!pad->current_client ||
			!pad->current_client->strips ||
			!pad->current_client->strips[strip]) {
		return;
	}
	struct wl_resource *resource = pad->current_client->strips[strip];

	if (finger) {
		zwp_tablet_pad_strip_v2_send_source(resource, ZWP_TABLET_PAD_STRIP_V2_SOURCE_FINGER);
	}

	if (position < 0) {
		zwp_tablet_pad_strip_v2_send_stop(resource);
	} else {
		zwp_tablet_pad_strip_v2_send_position(resource, position * 65535);
	}
	zwp_tablet_pad_strip_v2_send_frame(resource, time);
}

void wlr_send_tablet_v2_tablet_pad_ring(struct wlr_tablet_v2_tablet_pad *pad,
		uint32_t ring, double position, bool finger, uint32_t time) {
	if (!pad->current_client ||
			!pad->current_client->rings ||
			!pad->current_client->rings[ring]) {
		return;
	}
	struct wl_resource *resource = pad->current_client->rings[ring];

	if (finger) {
		zwp_tablet_pad_ring_v2_send_source(resource, ZWP_TABLET_PAD_RING_V2_SOURCE_FINGER);
	}

	if (position < 0) {
		zwp_tablet_pad_ring_v2_send_stop(resource);
	} else {
		zwp_tablet_pad_ring_v2_send_angle(resource, position);
	}
	zwp_tablet_pad_ring_v2_send_frame(resource, time);
}

uint32_t wlr_send_tablet_v2_tablet_pad_leave(struct wlr_tablet_v2_tablet_pad *pad,
		struct wlr_surface *surface) {
	struct wl_client *client = wl_resource_get_client(surface->resource);
	if (!pad->current_client || client != pad->current_client->client) {
		return 0;
	}


	uint32_t serial = wlr_seat_client_next_serial(
		pad->current_client->seat->seat_client);

	zwp_tablet_pad_v2_send_leave(pad->current_client->resource, serial, surface->resource);
	return serial;
}

uint32_t wlr_send_tablet_v2_tablet_pad_mode(struct wlr_tablet_v2_tablet_pad *pad,
		size_t group, uint32_t mode, uint32_t time) {
	if (!pad->current_client ||
			!pad->current_client->groups ||
			!pad->current_client->groups[group] ) {
		return 0;
	}

	if (pad->groups[group] == mode) {
		return 0;
	}

	pad->groups[group] = mode;

	uint32_t serial = wlr_seat_client_next_serial(
		pad->current_client->seat->seat_client);

	zwp_tablet_pad_group_v2_send_mode_switch(
		pad->current_client->groups[group], time, serial, mode);
	return serial;
}

bool wlr_surface_accepts_tablet_v2(struct wlr_tablet_v2_tablet *tablet,
		struct wlr_surface *surface) {
	struct wl_client *client = wl_resource_get_client(surface->resource);

	if (tablet->current_client &&
			tablet->current_client->client == client) {
		return true;
	}

	struct wlr_tablet_client_v2 *tablet_tmp;
	wl_list_for_each(tablet_tmp, &tablet->clients, tablet_link) {
		if (tablet_tmp->client == client) {
			return true;
		}
	}

	return false;
}


uint32_t wlr_tablet_v2_tablet_pad_notify_enter(
	struct wlr_tablet_v2_tablet_pad *pad,
	struct wlr_tablet_v2_tablet *tablet,
	struct wlr_surface *surface) {
	if (pad->grab && pad->grab->interface->enter) {
		return pad->grab->interface->enter(pad->grab, tablet, surface);
	}

	return 0;
}

void wlr_tablet_v2_tablet_pad_notify_button(
	struct wlr_tablet_v2_tablet_pad *pad, size_t button,
	uint32_t time, enum zwp_tablet_pad_v2_button_state state) {
	if (pad->grab && pad->grab->interface->button) {
		pad->grab->interface->button(pad->grab, button, time, state);
	}
}

void wlr_tablet_v2_tablet_pad_notify_strip(
	struct wlr_tablet_v2_tablet_pad *pad,
	uint32_t strip, double position, bool finger, uint32_t time) {
	if (pad->grab && pad->grab->interface->strip) {
		pad->grab->interface->strip(pad->grab, strip, position, finger, time);
	}
}

void wlr_tablet_v2_tablet_pad_notify_ring(
	struct wlr_tablet_v2_tablet_pad *pad,
	uint32_t ring, double position, bool finger, uint32_t time) {
	if (pad->grab && pad->grab->interface->ring) {
		pad->grab->interface->ring(pad->grab, ring, position, finger, time);
	}
}

uint32_t wlr_tablet_v2_tablet_pad_notify_leave(
	struct wlr_tablet_v2_tablet_pad *pad, struct wlr_surface *surface) {
	if (pad->grab && pad->grab->interface->leave) {
		return pad->grab->interface->leave(pad->grab, surface);
	}

	return 0;
}

uint32_t wlr_tablet_v2_tablet_pad_notify_mode(
	struct wlr_tablet_v2_tablet_pad *pad,
	size_t group, uint32_t mode, uint32_t time) {
	if (pad->grab && pad->grab->interface->mode) {
		return pad->grab->interface->mode(pad->grab, group, mode, time);
	}

	return 0;
}

void wlr_tablet_v2_start_grab(struct wlr_tablet_v2_tablet_pad *pad,
		struct wlr_tablet_pad_v2_grab *grab) {
	if (grab != &pad->default_grab) {
		struct wlr_tablet_pad_v2_grab *prev = pad->grab;
		grab->pad = pad;
		pad->grab = grab;
		if (prev && prev->interface->cancel) {
			prev->interface->cancel(prev);
		}
	}
}

void wlr_tablet_v2_end_grab(struct wlr_tablet_v2_tablet_pad *pad) {
	struct wlr_tablet_pad_v2_grab *grab = pad->grab;
	if (grab && grab != &pad->default_grab) {
		pad->grab = &pad->default_grab;
		if (grab->interface->cancel) {
			grab->interface->cancel(grab);
		}
	}
}

static uint32_t default_pad_enter(
		struct wlr_tablet_pad_v2_grab *grab,
		struct wlr_tablet_v2_tablet *tablet,
		struct wlr_surface *surface) {
	return wlr_send_tablet_v2_tablet_pad_enter(grab->pad, tablet, surface);
}

static void default_pad_button(struct wlr_tablet_pad_v2_grab *grab,size_t button,
		uint32_t time, enum zwp_tablet_pad_v2_button_state state) {
	wlr_send_tablet_v2_tablet_pad_button(grab->pad, button, time, state);
}

static void default_pad_strip(struct wlr_tablet_pad_v2_grab *grab,
		uint32_t strip, double position, bool finger, uint32_t time) {
	wlr_send_tablet_v2_tablet_pad_strip(grab->pad, strip, position, finger, time);
}

static void default_pad_ring(struct wlr_tablet_pad_v2_grab *grab,
		uint32_t ring, double position, bool finger, uint32_t time) {
	wlr_send_tablet_v2_tablet_pad_ring(grab->pad, ring, position, finger, time);
}

static uint32_t default_pad_leave(struct wlr_tablet_pad_v2_grab *grab,
		struct wlr_surface *surface) {
	return wlr_send_tablet_v2_tablet_pad_leave(grab->pad, surface);
}

static uint32_t default_pad_mode(struct wlr_tablet_pad_v2_grab *grab,
		size_t group, uint32_t mode, uint32_t time) {
	return wlr_send_tablet_v2_tablet_pad_mode(grab->pad, group, mode, time);
}

static void default_pad_cancel(struct wlr_tablet_pad_v2_grab *grab) {
	// Do nothing, the default cancel can be ignored.
}

static const struct wlr_tablet_pad_v2_grab_interface default_pad_grab_interface  = {
	.enter = default_pad_enter,
	.button = default_pad_button,
	.strip = default_pad_strip,
	.ring = default_pad_ring,
	.leave = default_pad_leave,
	.mode = default_pad_mode,
	.cancel = default_pad_cancel,
};
