#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-server-core.h>
#include <types/wlr_tablet_v2.h>
#include <wlr/config.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_tablet_tool.h>
#include <wlr/types/wlr_tablet_v2.h>
#include <wlr/util/log.h>
#include "tablet-unstable-v2-protocol.h"
#include "util/signal.h"

#define TABLET_MANAGER_VERSION 1

struct wlr_tablet_manager_client_v2 {
	struct wl_list link;
	struct wl_client *client;
	struct wl_resource *resource;
	struct wlr_tablet_manager_v2 *manager;

	struct wl_list tablet_seats; // wlr_tablet_seat_client_v2::link
};

static void tablet_seat_destroy(struct wlr_tablet_seat_v2 *seat) {
	struct wlr_tablet_seat_client_v2 *client, *client_tmp;
	wl_list_for_each_safe(client, client_tmp, &seat->clients, seat_link) {
		tablet_seat_client_v2_destroy(client->resource);
	}

	wl_list_remove(&seat->link);
	wl_list_remove(&seat->seat_destroy.link);
	free(seat);
}

static void handle_wlr_seat_destroy(struct wl_listener *listener, void *data) {
	struct wlr_tablet_seat_v2 *seat =
		wl_container_of(listener, seat, seat_destroy);
	tablet_seat_destroy(seat);
}

static struct wlr_tablet_seat_v2 *create_tablet_seat(
		struct wlr_tablet_manager_v2 *manager,
		struct wlr_seat *wlr_seat) {
	struct wlr_tablet_seat_v2 *tablet_seat =
		calloc(1, sizeof(struct wlr_tablet_seat_v2));
	if (!tablet_seat) {
		return NULL;
	}

	tablet_seat->manager = manager;
	tablet_seat->wlr_seat = wlr_seat;

	wl_list_init(&tablet_seat->clients);

	wl_list_init(&tablet_seat->tablets);
	wl_list_init(&tablet_seat->tools);
	wl_list_init(&tablet_seat->pads);

	tablet_seat->seat_destroy.notify = handle_wlr_seat_destroy;
	wl_signal_add(&wlr_seat->events.destroy, &tablet_seat->seat_destroy);

	wl_list_insert(&manager->seats, &tablet_seat->link);
	return tablet_seat;
}

struct wlr_tablet_seat_v2 *get_or_create_tablet_seat(
		struct wlr_tablet_manager_v2 *manager,
		struct wlr_seat *wlr_seat) {
	struct wlr_tablet_seat_v2 *pos;
	wl_list_for_each(pos, &manager->seats, link) {
		if (pos->wlr_seat == wlr_seat) {
			return pos;
		}
	}

	return create_tablet_seat(manager, wlr_seat);
}

static void tablet_seat_handle_destroy(struct wl_client *client,
		struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static const struct zwp_tablet_seat_v2_interface seat_impl = {
	.destroy = tablet_seat_handle_destroy,
};

struct wlr_tablet_seat_client_v2 *tablet_seat_client_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource, &zwp_tablet_seat_v2_interface,
		&seat_impl));
	return wl_resource_get_user_data(resource);
}

void tablet_seat_client_v2_destroy(struct wl_resource *resource) {
	struct wlr_tablet_seat_client_v2 *seat = tablet_seat_client_from_resource(resource);
	if (!seat) {
		return;
	}

	struct wlr_tablet_client_v2 *tablet;
	struct wlr_tablet_client_v2 *tmp_tablet;
	wl_list_for_each_safe(tablet, tmp_tablet, &seat->tablets, seat_link) {
		destroy_tablet_v2(tablet->resource);
	}

	struct wlr_tablet_pad_client_v2 *pad;
	struct wlr_tablet_pad_client_v2 *tmp_pad;
	wl_list_for_each_safe(pad, tmp_pad, &seat->pads, seat_link) {
		destroy_tablet_pad_v2(pad->resource);
	}

	struct wlr_tablet_tool_client_v2 *tool;
	struct wlr_tablet_tool_client_v2 *tmp_tool;
	wl_list_for_each_safe(tool, tmp_tool, &seat->tools, seat_link) {
		destroy_tablet_tool_v2(tool->resource);
	}

	wl_list_remove(&seat->seat_link);
	wl_list_remove(&seat->client_link);
	wl_list_remove(&seat->seat_client_destroy.link);

	free(seat);
	wl_resource_set_user_data(resource, NULL);
}

static void handle_seat_client_destroy(struct wl_listener *listener, void *data) {
	struct wlr_tablet_seat_client_v2 *seat =
		wl_container_of(listener, seat, seat_client_destroy);
	tablet_seat_client_v2_destroy(seat->resource);
}

static void tablet_manager_destroy(struct wl_client *client,
		struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static struct wlr_tablet_manager_client_v2 *tablet_manager_client_from_resource(
	struct wl_resource *resource);

static void get_tablet_seat(struct wl_client *wl_client, struct wl_resource *resource,
		uint32_t id, struct wl_resource *seat_resource) {
	struct wlr_tablet_manager_client_v2 *manager =
		tablet_manager_client_from_resource(resource);
	if (!manager) {
		/* Inert manager, just set up the resource for later
		 * destruction, without allocations or advertising things
		 */
		wl_resource_set_implementation(seat_resource, &seat_impl, NULL,
			tablet_seat_client_v2_destroy);
		return;
	}
	struct wlr_seat_client *seat = wlr_seat_client_from_resource(seat_resource);
	struct wlr_tablet_seat_v2 *tablet_seat =
		get_or_create_tablet_seat(manager->manager, seat->seat);

	if (!tablet_seat) { // This can only happen when we ran out of memory
		wl_client_post_no_memory(wl_client);
		return;
	}

	struct wlr_tablet_seat_client_v2 *seat_client =
		calloc(1, sizeof(struct wlr_tablet_seat_client_v2));
	if (seat_client == NULL) {
		wl_client_post_no_memory(wl_client);
		return;
	}

	seat_client->resource =
		wl_resource_create(wl_client, &zwp_tablet_seat_v2_interface, TABLET_MANAGER_VERSION, id);
	if (seat_client->resource == NULL) {
		free(seat_client);
		wl_client_post_no_memory(wl_client);
		return;
	}
	wl_resource_set_implementation(seat_client->resource, &seat_impl, seat_client,
		tablet_seat_client_v2_destroy);


	seat_client->seat_client = seat;
	seat_client->client = manager;
	seat_client->wl_client = wl_client;
	wl_list_init(&seat_client->tools);
	wl_list_init(&seat_client->tablets);
	wl_list_init(&seat_client->pads);

	seat_client->seat_client_destroy.notify = handle_seat_client_destroy;
	wl_signal_add(&seat->events.destroy, &seat_client->seat_client_destroy);

	wl_list_insert(&manager->tablet_seats, &seat_client->client_link);
	wl_list_insert(&tablet_seat->clients, &seat_client->seat_link);

	// We need to emit the devices already on the seat
	struct wlr_tablet_v2_tablet *tablet_pos;
	wl_list_for_each(tablet_pos, &tablet_seat->tablets, link) {
		add_tablet_client(seat_client, tablet_pos);
	}

	struct wlr_tablet_v2_tablet_pad *pad_pos;
	wl_list_for_each(pad_pos, &tablet_seat->pads, link) {
		add_tablet_pad_client(seat_client, pad_pos);
	}

	struct wlr_tablet_v2_tablet_tool *tool_pos;
	wl_list_for_each(tool_pos, &tablet_seat->tools, link) {
		add_tablet_tool_client(seat_client, tool_pos);
	}
}

static const struct zwp_tablet_manager_v2_interface manager_impl = {
	.get_tablet_seat = get_tablet_seat,
	.destroy = tablet_manager_destroy,
};

static struct wlr_tablet_manager_client_v2 *tablet_manager_client_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource, &zwp_tablet_manager_v2_interface,
		&manager_impl));
	return wl_resource_get_user_data(resource);
}

static void wlr_tablet_manager_v2_destroy(struct wl_resource *resource) {
	struct wlr_tablet_manager_client_v2 *client =
		tablet_manager_client_from_resource(resource);
	if (!client) {
		return;
	}

	struct wlr_tablet_seat_client_v2 *pos;
	struct wlr_tablet_seat_client_v2 *tmp;
	wl_list_for_each_safe(pos, tmp, &client->tablet_seats, client_link) {
		tablet_seat_client_v2_destroy(pos->resource);
	}

	wl_list_remove(&client->link);

	free(client);
	wl_resource_set_user_data(resource, NULL);
}

static void tablet_v2_bind(struct wl_client *wl_client, void *data,
		uint32_t version, uint32_t id) {
	struct wlr_tablet_manager_v2 *manager = data;
	assert(wl_client && manager);

	struct wlr_tablet_manager_client_v2 *client =
		calloc(1, sizeof(struct wlr_tablet_manager_client_v2));
	if (client == NULL) {
		wl_client_post_no_memory(wl_client);
		return;
	}

	wl_list_init(&client->tablet_seats);

	client->resource =
		wl_resource_create(wl_client, &zwp_tablet_manager_v2_interface, version, id);
	if (client->resource == NULL) {
		free(client);
		wl_client_post_no_memory(wl_client);
		return;
	}
	client->client = wl_client;
	client->manager = manager;

	wl_resource_set_implementation(client->resource, &manager_impl, client,
		wlr_tablet_manager_v2_destroy);
	wl_list_insert(&manager->clients, &client->link);
}

static void handle_display_destroy(struct wl_listener *listener, void *data) {
	struct wlr_tablet_manager_v2 *manager =
		wl_container_of(listener, manager, display_destroy);
	wlr_signal_emit_safe(&manager->events.destroy, manager);
	wl_list_remove(&manager->display_destroy.link);
	wl_global_destroy(manager->wl_global);
	free(manager);
}

struct wlr_tablet_manager_v2 *wlr_tablet_v2_create(struct wl_display *display) {
	struct wlr_tablet_manager_v2 *tablet =
		calloc(1, sizeof(struct wlr_tablet_manager_v2));
	if (!tablet) {
		return NULL;
	}

	tablet->wl_global = wl_global_create(display,
		&zwp_tablet_manager_v2_interface, TABLET_MANAGER_VERSION,
		tablet, tablet_v2_bind);
	if (tablet->wl_global == NULL) {
		free(tablet);
		return NULL;
	}

	wl_signal_init(&tablet->events.destroy);
	wl_list_init(&tablet->clients);
	wl_list_init(&tablet->seats);

	tablet->display_destroy.notify = handle_display_destroy;
	wl_display_add_destroy_listener(display, &tablet->display_destroy);

	return tablet;
}
