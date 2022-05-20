#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "tablet-unstable-v2-protocol.h"
#include "util/array.h"
#include "util/time.h"
#include <assert.h>
#include <stdlib.h>
#include <types/wlr_tablet_v2.h>
#include <wayland-util.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_tablet_tool.h>
#include <wlr/types/wlr_tablet_v2.h>
#include <wlr/util/log.h>

static const struct wlr_tablet_tool_v2_grab_interface default_tool_grab_interface;

static const struct wlr_surface_role tablet_tool_cursor_surface_role = {
	.name = "wp_tablet_tool-cursor",
};

static void handle_tablet_tool_v2_set_cursor(struct wl_client *client,
		struct wl_resource *resource, uint32_t serial,
		struct wl_resource *surface_resource,
		int32_t hotspot_x, int32_t hotspot_y) {
	struct wlr_tablet_tool_client_v2 *tool = tablet_tool_client_from_resource(resource);
	if (!tool || !tool->tool) {
		return;
	}

	struct wlr_surface *surface = NULL;
	if (surface_resource != NULL) {
		surface = wlr_surface_from_resource(surface_resource);
		if (!wlr_surface_set_role(surface, &tablet_tool_cursor_surface_role, NULL,
				surface_resource, ZWP_TABLET_TOOL_V2_ERROR_ROLE)) {
			return;
		}
	}

	struct wlr_tablet_v2_event_cursor evt = {
		.surface = surface,
		.serial = serial,
		.hotspot_x = hotspot_x,
		.hotspot_y = hotspot_y,
		.seat_client = tool->seat->seat_client,
	};

	wl_signal_emit(&tool->tool->events.set_cursor, &evt);
}

static void handle_tablet_tool_v2_destroy(struct wl_client *client,
		struct wl_resource *resource) {
	wl_resource_destroy(resource);
}
static const struct zwp_tablet_tool_v2_interface tablet_tool_impl = {
	.set_cursor = handle_tablet_tool_v2_set_cursor,
	.destroy = handle_tablet_tool_v2_destroy,
};

static enum zwp_tablet_tool_v2_type tablet_type_from_wlr_type(
		enum wlr_tablet_tool_type wlr_type) {
	switch(wlr_type) {
	case WLR_TABLET_TOOL_TYPE_PEN:
		return ZWP_TABLET_TOOL_V2_TYPE_PEN;
	case WLR_TABLET_TOOL_TYPE_ERASER:
		return ZWP_TABLET_TOOL_V2_TYPE_ERASER;
	case WLR_TABLET_TOOL_TYPE_BRUSH:
		return ZWP_TABLET_TOOL_V2_TYPE_BRUSH;
	case WLR_TABLET_TOOL_TYPE_PENCIL:
		return ZWP_TABLET_TOOL_V2_TYPE_PENCIL;
	case WLR_TABLET_TOOL_TYPE_AIRBRUSH:
		return ZWP_TABLET_TOOL_V2_TYPE_AIRBRUSH;
	case WLR_TABLET_TOOL_TYPE_MOUSE:
		return ZWP_TABLET_TOOL_V2_TYPE_MOUSE;
	case WLR_TABLET_TOOL_TYPE_LENS:
		return ZWP_TABLET_TOOL_V2_TYPE_LENS;
	case WLR_TABLET_TOOL_TYPE_TOTEM:
		// missing, see:
		// https://gitlab.freedesktop.org/wayland/wayland-protocols/-/issues/19
		abort();
	}
	abort(); // unreachable
}

void destroy_tablet_tool_v2(struct wl_resource *resource) {
	struct wlr_tablet_tool_client_v2 *client =
		tablet_tool_client_from_resource(resource);

	if (!client) {
		return;
	}

	if (client->frame_source) {
		wl_event_source_remove(client->frame_source);
	}

	if (client->tool && client->tool->current_client == client) {
		client->tool->current_client = NULL;
	}

	wl_list_remove(&client->seat_link);
	wl_list_remove(&client->tool_link);
	free(client);

	wl_resource_set_user_data(resource, NULL);
}

void add_tablet_tool_client(struct wlr_tablet_seat_client_v2 *seat,
		struct wlr_tablet_v2_tablet_tool *tool) {
	struct wlr_tablet_tool_client_v2 *client =
		calloc(1, sizeof(struct wlr_tablet_tool_client_v2));
	if (!client) {
		return;
	}
	client->tool = tool;
	client->seat = seat;

	uint32_t version = wl_resource_get_version(seat->resource);
	client->resource = wl_resource_create(seat->wl_client,
		&zwp_tablet_tool_v2_interface, version, 0);
	if (!client->resource) {
		free(client);
		return;
	}
	wl_resource_set_implementation(client->resource, &tablet_tool_impl,
		client, destroy_tablet_tool_v2);
	zwp_tablet_seat_v2_send_tool_added(seat->resource, client->resource);

	// Send the expected events
	if (tool->wlr_tool->hardware_serial) {
			zwp_tablet_tool_v2_send_hardware_serial(
			client->resource,
			tool->wlr_tool->hardware_serial >> 32,
			tool->wlr_tool->hardware_serial & 0xFFFFFFFF);
	}
	if (tool->wlr_tool->hardware_wacom) {
			zwp_tablet_tool_v2_send_hardware_id_wacom(
			client->resource,
			tool->wlr_tool->hardware_wacom >> 32,
			tool->wlr_tool->hardware_wacom & 0xFFFFFFFF);
	}
	zwp_tablet_tool_v2_send_type(client->resource,
		tablet_type_from_wlr_type(tool->wlr_tool->type));

	if (tool->wlr_tool->tilt) {
		zwp_tablet_tool_v2_send_capability(client->resource,
			ZWP_TABLET_TOOL_V2_CAPABILITY_TILT);
	}

	if (tool->wlr_tool->pressure) {
		zwp_tablet_tool_v2_send_capability(client->resource,
			ZWP_TABLET_TOOL_V2_CAPABILITY_PRESSURE);
	}

	if (tool->wlr_tool->distance) {
		zwp_tablet_tool_v2_send_capability(client->resource,
			ZWP_TABLET_TOOL_V2_CAPABILITY_DISTANCE);
	}

	if (tool->wlr_tool->rotation) {
		zwp_tablet_tool_v2_send_capability(client->resource,
			ZWP_TABLET_TOOL_V2_CAPABILITY_ROTATION);
	}

	if (tool->wlr_tool->slider) {
		zwp_tablet_tool_v2_send_capability(client->resource,
			ZWP_TABLET_TOOL_V2_CAPABILITY_SLIDER);
	}

	if (tool->wlr_tool->wheel) {
		zwp_tablet_tool_v2_send_capability(client->resource,
			ZWP_TABLET_TOOL_V2_CAPABILITY_WHEEL);
	}

	zwp_tablet_tool_v2_send_done(client->resource);

	client->client = seat->wl_client;
	wl_list_insert(&seat->tools, &client->seat_link);
	wl_list_insert(&tool->clients, &client->tool_link);
}

static void handle_wlr_tablet_tool_destroy(struct wl_listener *listener, void *data) {
	struct wlr_tablet_v2_tablet_tool *tool =
		wl_container_of(listener, tool, tool_destroy);

	struct wlr_tablet_tool_client_v2 *pos;
	struct wlr_tablet_tool_client_v2 *tmp;
	wl_list_for_each_safe(pos, tmp, &tool->clients, tool_link) {
		zwp_tablet_tool_v2_send_removed(pos->resource);
		pos->tool = NULL;
	}

	wl_list_remove(&tool->clients);
	wl_list_remove(&tool->link);
	wl_list_remove(&tool->tool_destroy.link);
	wl_list_remove(&tool->events.set_cursor.listener_list);
	wl_list_remove(&tool->surface_destroy.link);
	free(tool);
}

struct wlr_tablet_v2_tablet_tool *wlr_tablet_tool_create(
		struct wlr_tablet_manager_v2 *manager,
		struct wlr_seat *wlr_seat,
		struct wlr_tablet_tool *wlr_tool) {
	switch (wlr_tool->type) {
	case WLR_TABLET_TOOL_TYPE_PEN:
	case WLR_TABLET_TOOL_TYPE_ERASER:
	case WLR_TABLET_TOOL_TYPE_BRUSH:
	case WLR_TABLET_TOOL_TYPE_PENCIL:
	case WLR_TABLET_TOOL_TYPE_AIRBRUSH:
	case WLR_TABLET_TOOL_TYPE_MOUSE:
	case WLR_TABLET_TOOL_TYPE_LENS:
		/* supported */
		break;
	default:
		/* Unsupported */
		return NULL;
	}

	struct wlr_tablet_seat_v2 *seat = get_or_create_tablet_seat(manager, wlr_seat);
	if (!seat) {
		return NULL;
	}
	struct wlr_tablet_v2_tablet_tool *tool =
		calloc(1, sizeof(struct wlr_tablet_v2_tablet_tool));
	if (!tool) {
		return NULL;
	}

	tool->wlr_tool = wlr_tool;
	wl_list_init(&tool->clients);
	wl_list_init(&tool->surface_destroy.link);
	tool->default_grab.tool = tool;
	tool->default_grab.interface = &default_tool_grab_interface;
	tool->grab = &tool->default_grab;

	tool->tool_destroy.notify = handle_wlr_tablet_tool_destroy;
	wl_signal_add(&wlr_tool->events.destroy, &tool->tool_destroy);
	wl_list_insert(&seat->tools, &tool->link);

	// We need to create a tablet client for all clients on the seat
	struct wlr_tablet_seat_client_v2 *pos;
	wl_list_for_each(pos, &seat->clients, seat_link) {
		// Tell the clients about the new tool
		add_tablet_tool_client(pos, tool);
	}

	wl_signal_init(&tool->events.set_cursor);

	return tool;
}

struct wlr_tablet_tool_client_v2 *tablet_tool_client_from_resource(struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource, &zwp_tablet_tool_v2_interface,
		&tablet_tool_impl));
	return wl_resource_get_user_data(resource);
}


/* Actual protocol foo */

// Button 0 is KEY_RESERVED in input-event-codes on linux (and freebsd)
static ssize_t tablet_tool_button_update(struct wlr_tablet_v2_tablet_tool *tool,
		uint32_t button, enum zwp_tablet_pad_v2_button_state state) {
	bool found = false;
	size_t i = 0;
	for (; i < tool->num_buttons; ++i) {
		if (tool->pressed_buttons[i] == button) {
			found = true;
			wlr_log(WLR_DEBUG, "Found the button \\o/: %u", button);
			break;

		}
	}

	if (state == ZWP_TABLET_PAD_V2_BUTTON_STATE_PRESSED && found) {
		/* Already have the button saved, durr */
		return -1;
	}

	if (state == ZWP_TABLET_PAD_V2_BUTTON_STATE_PRESSED && !found) {
		if (tool->num_buttons < WLR_TABLET_V2_TOOL_BUTTONS_CAP) {
			i = tool->num_buttons++;
			tool->pressed_buttons[i] = button;
			tool->pressed_serials[i] = -1;
		} else {
			i = -1;
			wlr_log(WLR_ERROR, "You pressed more than %d tablet tool buttons. "
				"This is currently not supported by wlroots. Please report this "
				"with a description of your tablet, since this is either a "
				"bug, or fancy hardware", WLR_TABLET_V2_TOOL_BUTTONS_CAP);
		}
	}
	if (state == ZWP_TABLET_PAD_V2_BUTTON_STATE_RELEASED && found) {
		wlr_log(WLR_DEBUG, "Removed the button \\o/: %u", button);
		tool->pressed_buttons[i] = 0;
		tool->pressed_serials[i] = 0;
		tool->num_buttons = push_zeroes_to_end(tool->pressed_buttons, WLR_TABLET_V2_TOOL_BUTTONS_CAP);
		tool->num_buttons = push_zeroes_to_end(tool->pressed_serials, WLR_TABLET_V2_TOOL_BUTTONS_CAP);
	}

	assert(tool->num_buttons <= WLR_TABLET_V2_TOOL_BUTTONS_CAP);
	return i;
}

static void send_tool_frame(void *data) {
	struct wlr_tablet_tool_client_v2 *tool = data;

	zwp_tablet_tool_v2_send_frame(tool->resource, get_current_time_msec());
	tool->frame_source = NULL;
}

static void queue_tool_frame(struct wlr_tablet_tool_client_v2 *tool) {
	struct wl_display *display = wl_client_get_display(tool->client);
	struct wl_event_loop *loop = wl_display_get_event_loop(display);
	if (!tool->frame_source) {
		tool->frame_source =
			wl_event_loop_add_idle(loop, send_tool_frame, tool);
	}
}

static void handle_tablet_tool_surface_destroy(struct wl_listener *listener,
		void *data) {
	struct wlr_tablet_v2_tablet_tool *tool =
		wl_container_of(listener, tool, surface_destroy);
	wlr_send_tablet_v2_tablet_tool_proximity_out(tool);
}

void wlr_send_tablet_v2_tablet_tool_proximity_in(
		struct wlr_tablet_v2_tablet_tool *tool,
		struct wlr_tablet_v2_tablet *tablet,
		struct wlr_surface *surface) {
	struct wl_client *client = wl_resource_get_client(surface->resource);

	if (tool->focused_surface == surface) {
		return;
	}

	wlr_send_tablet_v2_tablet_tool_proximity_out(tool);

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
		return;
	}

	struct wlr_tablet_tool_client_v2 *tool_tmp = NULL;
	struct wlr_tablet_tool_client_v2 *tool_client = NULL;
	wl_list_for_each(tool_tmp, &tool->clients, tool_link) {
		if (tool_tmp->client == client) {
			tool_client = tool_tmp;
			break;
		}
	}

	// Couldn't find the client binding for the surface's client. Either
	// the client didn't bind tablet_v2 at all, or not for the relevant
	// seat
	if (!tool_client) {
		return;
	}

	// Reinitialize the focus destroy events
	wl_list_remove(&tool->surface_destroy.link);
	wl_signal_add(&surface->events.destroy, &tool->surface_destroy);
	tool->surface_destroy.notify = handle_tablet_tool_surface_destroy;

	tool->current_client = tool_client;

	uint32_t serial = wlr_seat_client_next_serial(tool_client->seat->seat_client);
	tool->focused_surface = surface;
	tool->proximity_serial = serial;

	zwp_tablet_tool_v2_send_proximity_in(tool_client->resource, serial,
		tablet_client->resource, surface->resource);
	/* Send all the pressed buttons */
	for (size_t i = 0; i < tool->num_buttons; ++i) {
		wlr_send_tablet_v2_tablet_tool_button(tool,
			tool->pressed_buttons[i],
			ZWP_TABLET_PAD_V2_BUTTON_STATE_PRESSED);
	}
	if (tool->is_down) {
		wlr_send_tablet_v2_tablet_tool_down(tool);
	}

	queue_tool_frame(tool_client);
}

void wlr_send_tablet_v2_tablet_tool_motion(
		struct wlr_tablet_v2_tablet_tool *tool, double x, double y) {
	if (!tool->current_client) {
		return;
	}

	zwp_tablet_tool_v2_send_motion(tool->current_client->resource,
		wl_fixed_from_double(x), wl_fixed_from_double(y));

	queue_tool_frame(tool->current_client);
}

void wlr_send_tablet_v2_tablet_tool_proximity_out(
		struct wlr_tablet_v2_tablet_tool *tool) {
	if (tool->current_client) {
		for (size_t i = 0; i < tool->num_buttons; ++i) {
			zwp_tablet_tool_v2_send_button(tool->current_client->resource,
				tool->pressed_serials[i],
				tool->pressed_buttons[i],
				ZWP_TABLET_PAD_V2_BUTTON_STATE_RELEASED);
		}
		if (tool->is_down) {
			zwp_tablet_tool_v2_send_up(tool->current_client->resource);
		}
		if (tool->current_client->frame_source) {
			wl_event_source_remove(tool->current_client->frame_source);
			send_tool_frame(tool->current_client);
		}
		zwp_tablet_tool_v2_send_proximity_out(tool->current_client->resource);
		send_tool_frame(tool->current_client);

		wl_list_remove(&tool->surface_destroy.link);
		wl_list_init(&tool->surface_destroy.link);
		tool->current_client = NULL;
		tool->focused_surface = NULL;
	}
}

void wlr_send_tablet_v2_tablet_tool_pressure(
		struct wlr_tablet_v2_tablet_tool *tool, double pressure) {
	if (tool->current_client) {
		zwp_tablet_tool_v2_send_pressure(tool->current_client->resource,
			pressure * 65535);

		queue_tool_frame(tool->current_client);
	}
}

void wlr_send_tablet_v2_tablet_tool_distance(
		struct wlr_tablet_v2_tablet_tool *tool, double distance) {
	if (tool->current_client) {
		zwp_tablet_tool_v2_send_distance(tool->current_client->resource,
			distance * 65535);

		queue_tool_frame(tool->current_client);
	}
}

void wlr_send_tablet_v2_tablet_tool_tilt(
		struct wlr_tablet_v2_tablet_tool *tool, double x, double y) {
	if (!tool->current_client) {
		return;
	}

	zwp_tablet_tool_v2_send_tilt(tool->current_client->resource,
		wl_fixed_from_double(x), wl_fixed_from_double(y));

	queue_tool_frame(tool->current_client);
}

void wlr_send_tablet_v2_tablet_tool_rotation(
		struct wlr_tablet_v2_tablet_tool *tool, double degrees) {
	if (!tool->current_client) {
		return;
	}

	zwp_tablet_tool_v2_send_rotation(tool->current_client->resource,
		wl_fixed_from_double(degrees));

	queue_tool_frame(tool->current_client);
}

void wlr_send_tablet_v2_tablet_tool_slider(
		struct wlr_tablet_v2_tablet_tool *tool, double position) {
	if (!tool->current_client) {
		return;
	}

	zwp_tablet_tool_v2_send_slider(tool->current_client->resource,
		position * 65535);

	queue_tool_frame(tool->current_client);
}

void wlr_send_tablet_v2_tablet_tool_button(
		struct wlr_tablet_v2_tablet_tool *tool, uint32_t button,
		enum zwp_tablet_pad_v2_button_state state) {
	ssize_t index = tablet_tool_button_update(tool, button, state);

	if (tool->current_client) {
		uint32_t serial = wlr_seat_client_next_serial(
			tool->current_client->seat->seat_client);
		if (index >= 0) {
			tool->pressed_serials[index] = serial;
		}

		zwp_tablet_tool_v2_send_button(tool->current_client->resource,
			serial, button, state);
		queue_tool_frame(tool->current_client);
	}
}

void wlr_send_tablet_v2_tablet_tool_wheel(
	struct wlr_tablet_v2_tablet_tool *tool, double degrees, int32_t clicks) {
	if (tool->current_client) {
		zwp_tablet_tool_v2_send_wheel(tool->current_client->resource,
			clicks, degrees);

		queue_tool_frame(tool->current_client);
	}
}

void wlr_send_tablet_v2_tablet_tool_down(struct wlr_tablet_v2_tablet_tool *tool) {
	if (tool->is_down) {
		return;
	}

	tool->is_down = true;
	if (tool->current_client) {
		uint32_t serial = wlr_seat_client_next_serial(
			tool->current_client->seat->seat_client);

		zwp_tablet_tool_v2_send_down(tool->current_client->resource,
			serial);
		queue_tool_frame(tool->current_client);

		tool->down_serial = serial;
	}
}

void wlr_send_tablet_v2_tablet_tool_up(struct wlr_tablet_v2_tablet_tool *tool) {
	if (!tool->is_down) {
		return;
	}
	tool->is_down = false;
	tool->down_serial = 0;

	if (tool->current_client) {
		zwp_tablet_tool_v2_send_up(tool->current_client->resource);
		queue_tool_frame(tool->current_client);
	}
}


void wlr_tablet_v2_tablet_tool_notify_proximity_in(
	struct wlr_tablet_v2_tablet_tool *tool,
	struct wlr_tablet_v2_tablet *tablet,
	struct wlr_surface *surface) {
	if (tool->grab->interface->proximity_in) {
		tool->grab->interface->proximity_in(tool->grab, tablet, surface);
	}
}

void wlr_tablet_v2_tablet_tool_notify_down(struct wlr_tablet_v2_tablet_tool *tool) {
	if (tool->grab->interface->down) {
		tool->grab->interface->down(tool->grab);
	}
}
void wlr_tablet_v2_tablet_tool_notify_up(struct wlr_tablet_v2_tablet_tool *tool) {
	if (tool->grab->interface->up) {
		tool->grab->interface->up(tool->grab);
	}
}

void wlr_tablet_v2_tablet_tool_notify_motion(
	struct wlr_tablet_v2_tablet_tool *tool, double x, double y) {
	if (tool->grab->interface->motion) {
		tool->grab->interface->motion(tool->grab, x, y);
	}
}

void wlr_tablet_v2_tablet_tool_notify_pressure(
	struct wlr_tablet_v2_tablet_tool *tool, double pressure) {
	if (tool->grab->interface->pressure) {
		tool->grab->interface->pressure(tool->grab, pressure);
	}
}

void wlr_tablet_v2_tablet_tool_notify_distance(
	struct wlr_tablet_v2_tablet_tool *tool, double distance) {
	if (tool->grab->interface->distance) {
		tool->grab->interface->distance(tool->grab, distance);
	}
}

void wlr_tablet_v2_tablet_tool_notify_tilt(
	struct wlr_tablet_v2_tablet_tool *tool, double x, double y) {
	if (tool->grab->interface->tilt) {
		tool->grab->interface->tilt(tool->grab, x, y);
	}
}

void wlr_tablet_v2_tablet_tool_notify_rotation(
	struct wlr_tablet_v2_tablet_tool *tool, double degrees) {
	if (tool->grab->interface->rotation) {
		tool->grab->interface->rotation(tool->grab, degrees);
	}
}

void wlr_tablet_v2_tablet_tool_notify_slider(
	struct wlr_tablet_v2_tablet_tool *tool, double position) {
	if (tool->grab->interface->slider) {
		tool->grab->interface->slider(tool->grab, position);
	}
}

void wlr_tablet_v2_tablet_tool_notify_wheel(
	struct wlr_tablet_v2_tablet_tool *tool, double degrees, int32_t clicks) {
	if (tool->grab->interface->wheel) {
		tool->grab->interface->wheel(tool->grab, degrees, clicks);
	}
}

void wlr_tablet_v2_tablet_tool_notify_proximity_out(
	struct wlr_tablet_v2_tablet_tool *tool) {
	if (tool->grab->interface->proximity_out) {
		tool->grab->interface->proximity_out(tool->grab);
	}
}

void wlr_tablet_v2_tablet_tool_notify_button(
	struct wlr_tablet_v2_tablet_tool *tool, uint32_t button,
	enum zwp_tablet_pad_v2_button_state state) {
	if (tool->grab->interface->button) {
		tool->grab->interface->button(tool->grab, button, state);
	}
}

void wlr_tablet_tool_v2_start_grab(struct wlr_tablet_v2_tablet_tool *tool,
		struct wlr_tablet_tool_v2_grab *grab) {
	wlr_tablet_tool_v2_end_grab(tool);
	tool->grab = grab;
}

void wlr_tablet_tool_v2_end_grab(struct wlr_tablet_v2_tablet_tool *tool) {
	if (tool->grab->interface->cancel) {
		tool->grab->interface->cancel(tool->grab);
	}
	tool->grab = &tool->default_grab;
}


static void default_tool_proximity_in(
	struct wlr_tablet_tool_v2_grab *grab,
	struct wlr_tablet_v2_tablet *tablet,
	struct wlr_surface *surface) {
	wlr_send_tablet_v2_tablet_tool_proximity_in(grab->tool, tablet, surface);
}

static void default_tool_down(struct wlr_tablet_tool_v2_grab *grab) {
	wlr_send_tablet_v2_tablet_tool_down(grab->tool);
}
static void default_tool_up(struct wlr_tablet_tool_v2_grab *grab) {
	wlr_send_tablet_v2_tablet_tool_up(grab->tool);
}

static void default_tool_motion(
		struct wlr_tablet_tool_v2_grab *grab, double x, double y) {
	wlr_send_tablet_v2_tablet_tool_motion(grab->tool, x, y);
}

static void default_tool_pressure(
	struct wlr_tablet_tool_v2_grab *grab, double pressure) {
	wlr_send_tablet_v2_tablet_tool_pressure(grab->tool, pressure);
}

static void default_tool_distance(
	struct wlr_tablet_tool_v2_grab *grab, double distance) {
	wlr_send_tablet_v2_tablet_tool_distance(grab->tool, distance);
}

static void default_tool_tilt(
	struct wlr_tablet_tool_v2_grab *grab, double x, double y) {
	wlr_send_tablet_v2_tablet_tool_tilt(grab->tool, x, y);
}

static void default_tool_rotation(
	struct wlr_tablet_tool_v2_grab *grab, double degrees) {
	wlr_send_tablet_v2_tablet_tool_rotation(grab->tool, degrees);
}

static void default_tool_slider(
	struct wlr_tablet_tool_v2_grab *grab, double position) {
	wlr_send_tablet_v2_tablet_tool_slider(grab->tool, position);
}

static void default_tool_wheel(
	struct wlr_tablet_tool_v2_grab *grab, double degrees, int32_t clicks) {
	wlr_send_tablet_v2_tablet_tool_wheel(grab->tool, degrees, clicks);
}

static void default_tool_proximity_out(struct wlr_tablet_tool_v2_grab *grab) {
	wlr_send_tablet_v2_tablet_tool_proximity_out(grab->tool);
}

static void default_tool_button(
	struct wlr_tablet_tool_v2_grab *grab, uint32_t button,
	enum zwp_tablet_pad_v2_button_state state) {
	wlr_send_tablet_v2_tablet_tool_button(grab->tool, button, state);
}

static void default_tool_cancel(struct wlr_tablet_tool_v2_grab *grab) {
	/* Do nothing. Default grab can't be canceled */
}

static const struct wlr_tablet_tool_v2_grab_interface
		default_tool_grab_interface = {
	.proximity_in = default_tool_proximity_in,
	.down = default_tool_down,
	.up = default_tool_up,
	.motion = default_tool_motion,
	.pressure = default_tool_pressure,
	.distance = default_tool_distance,
	.tilt = default_tool_tilt,
	.rotation = default_tool_rotation,
	.slider = default_tool_slider,
	.wheel = default_tool_wheel,
	.proximity_out = default_tool_proximity_out,
	.button = default_tool_button,
	.cancel = default_tool_cancel,
};

struct implicit_grab_state {
	struct wlr_surface *original;
	bool released;

	struct wlr_surface *focused;
	struct wlr_tablet_v2_tablet *tablet;
};

static void check_and_release_implicit_grab(struct wlr_tablet_tool_v2_grab *grab) {
	struct implicit_grab_state *state = grab->data;
	/* Still button or tip pressed. We should hold the grab */
	if (grab->tool->is_down || grab->tool->num_buttons > 0 || state->released) {
		return;
	}

	state->released = true;

	/* We should still focus the same surface. Do nothing */
	if (state->original == state->focused) {
		wlr_tablet_tool_v2_end_grab(grab->tool);
		return;
	}

	wlr_send_tablet_v2_tablet_tool_proximity_out(grab->tool);
	if (state->focused) {
		wlr_send_tablet_v2_tablet_tool_proximity_in(grab->tool,
			state->tablet, state->focused);
	}

	wlr_tablet_tool_v2_end_grab(grab->tool);
}

static void implicit_tool_proximity_in(
	struct wlr_tablet_tool_v2_grab *grab,
	struct wlr_tablet_v2_tablet *tablet,
	struct wlr_surface *surface) {

	/* As long as we got an implicit grab, proximity won't change
	 * But should track the currently focused surface to change to it when
	 * the grab is released.
	 */
	struct implicit_grab_state *state = grab->data;
	state->focused = surface;
	state->tablet = tablet;
}

static void implicit_tool_proximity_out(struct wlr_tablet_tool_v2_grab *grab) {
	struct implicit_grab_state *state = grab->data;
	state->focused = NULL;
}

static void implicit_tool_down(struct wlr_tablet_tool_v2_grab *grab) {
	wlr_send_tablet_v2_tablet_tool_down(grab->tool);
}

static void implicit_tool_up(struct wlr_tablet_tool_v2_grab *grab) {
	wlr_send_tablet_v2_tablet_tool_up(grab->tool);
	check_and_release_implicit_grab(grab);
}

static void implicit_tool_button(
	struct wlr_tablet_tool_v2_grab *grab, uint32_t button,
	enum zwp_tablet_pad_v2_button_state state) {
	wlr_send_tablet_v2_tablet_tool_button(grab->tool, button, state);
	check_and_release_implicit_grab(grab);
}

static void implicit_tool_cancel(struct wlr_tablet_tool_v2_grab *grab) {
	check_and_release_implicit_grab(grab);
	free(grab->data);
	free(grab);
}

static const struct wlr_tablet_tool_v2_grab_interface
		implicit_tool_grab_interface = {
	.proximity_in = implicit_tool_proximity_in,
	.down = implicit_tool_down,
	.up = implicit_tool_up,
	.motion = default_tool_motion,
	.pressure = default_tool_pressure,
	.distance = default_tool_distance,
	.tilt = default_tool_tilt,
	.rotation = default_tool_rotation,
	.slider = default_tool_slider,
	.wheel = default_tool_wheel,
	.proximity_out = implicit_tool_proximity_out,
	.button = implicit_tool_button,
	.cancel = implicit_tool_cancel,
};

bool wlr_tablet_tool_v2_has_implicit_grab(
		struct wlr_tablet_v2_tablet_tool *tool) {
	return tool->grab->interface == &implicit_tool_grab_interface;
}

void wlr_tablet_tool_v2_start_implicit_grab(
		struct wlr_tablet_v2_tablet_tool *tool) {
	if (wlr_tablet_tool_v2_has_implicit_grab(tool) || !tool->focused_surface) {
		return;
	}

	/* No current implicit grab */
	if (!(tool->is_down || tool->num_buttons > 0)) {
		return;
	}

	struct wlr_tablet_tool_v2_grab *grab =
		calloc(1, sizeof(struct wlr_tablet_tool_v2_grab));
	if (!grab) {
		return;
	}

	grab->interface = &implicit_tool_grab_interface;
	grab->tool = tool;
	struct implicit_grab_state *state = calloc(1, sizeof(struct implicit_grab_state));
	if (!state) {
		free(grab);
		return;
	}

	state->original = tool->focused_surface;
	state->focused = tool->focused_surface;
	grab->data = state;

	wlr_tablet_tool_v2_start_grab(tool, grab);
}
