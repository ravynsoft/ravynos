#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include <assert.h>
#include <stdlib.h>
#include <types/wlr_tablet_v2.h>
#include <wayland-util.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_tablet_tool.h>
#include <wlr/types/wlr_tablet_v2.h>
#include <wlr/util/log.h>

#include "tablet-unstable-v2-protocol.h"

void destroy_tablet_v2(struct wl_resource *resource) {
	struct wlr_tablet_client_v2 *tablet = tablet_client_from_resource(resource);

	if (!tablet) {
		return;
	}

	wl_list_remove(&tablet->seat_link);
	wl_list_remove(&tablet->tablet_link);
	free(tablet);
	wl_resource_set_user_data(resource, NULL);
}

static void handle_tablet_v2_destroy(struct wl_client *client,
		struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static const struct zwp_tablet_v2_interface tablet_impl = {
	.destroy = handle_tablet_v2_destroy,
};

static void handle_wlr_tablet_destroy(struct wl_listener *listener, void *data) {
	struct wlr_tablet_v2_tablet *tablet =
		wl_container_of(listener, tablet, tool_destroy);

	struct wlr_tablet_client_v2 *pos;
	struct wlr_tablet_client_v2 *tmp;
	wl_list_for_each_safe(pos, tmp, &tablet->clients, tablet_link) {
		zwp_tablet_v2_send_removed(pos->resource);
	}

	wl_list_remove(&tablet->clients);
	wl_list_remove(&tablet->link);
	wl_list_remove(&tablet->tool_destroy.link);
	free(tablet);
}

struct wlr_tablet_v2_tablet *wlr_tablet_create(
		struct wlr_tablet_manager_v2 *manager,
		struct wlr_seat *wlr_seat,
		struct wlr_input_device *wlr_device) {
	assert(wlr_device->type == WLR_INPUT_DEVICE_TABLET_TOOL);
	struct wlr_tablet_seat_v2 *seat = get_or_create_tablet_seat(manager, wlr_seat);
	if (!seat) {
		return NULL;
	}
	struct wlr_tablet *wlr_tablet = wlr_device->tablet;
	struct wlr_tablet_v2_tablet *tablet = calloc(1, sizeof(struct wlr_tablet_v2_tablet));
	if (!tablet) {
		return NULL;
	}

	tablet->wlr_tablet = wlr_tablet;
	tablet->wlr_device = wlr_device;
	wl_list_init(&tablet->clients);


	tablet->tool_destroy.notify = handle_wlr_tablet_destroy;
	wl_signal_add(&wlr_device->events.destroy, &tablet->tool_destroy);
	wl_list_insert(&seat->tablets, &tablet->link);

	// We need to create a tablet client for all clients on the seat
	struct wlr_tablet_seat_client_v2 *pos;
	wl_list_for_each(pos, &seat->clients, seat_link) {
		// Tell the clients about the new tool
		add_tablet_client(pos, tablet);
	}

	return tablet;
}


void add_tablet_client(struct wlr_tablet_seat_client_v2 *seat,
		struct wlr_tablet_v2_tablet *tablet) {
	struct wlr_tablet_client_v2 *client =
		calloc(1, sizeof(struct wlr_tablet_client_v2));
	if (!client) {
		return;
	}

	uint32_t version = wl_resource_get_version(seat->resource);
	client->resource =
		wl_resource_create(seat->wl_client, &zwp_tablet_v2_interface,
			version, 0);
	if (!client->resource) {
		wl_resource_post_no_memory(seat->resource);
		free(client);
		return;
	}
	wl_resource_set_implementation(client->resource, &tablet_impl,
		client, destroy_tablet_v2);
	zwp_tablet_seat_v2_send_tablet_added(seat->resource, client->resource);

	// Send the expected events
	if (tablet->wlr_tablet->name) {
		zwp_tablet_v2_send_name(client->resource,
			tablet->wlr_tablet->name);
	}
	zwp_tablet_v2_send_id(client->resource,
		tablet->wlr_device->vendor, tablet->wlr_device->product);

	const char **path_ptr;
	wl_array_for_each(path_ptr, &tablet->wlr_tablet->paths) {
		zwp_tablet_v2_send_path(client->resource, *path_ptr);
	}

	zwp_tablet_v2_send_done(client->resource);

	client->client = seat->wl_client;
	wl_list_insert(&seat->tablets, &client->seat_link);
	wl_list_insert(&tablet->clients, &client->tablet_link);
}


struct wlr_tablet_client_v2 *tablet_client_from_resource(struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource, &zwp_tablet_v2_interface,
		&tablet_impl));
	return wl_resource_get_user_data(resource);
}
