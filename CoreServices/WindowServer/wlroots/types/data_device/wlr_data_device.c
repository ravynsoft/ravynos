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

#define DATA_DEVICE_MANAGER_VERSION 3

static const struct wl_data_device_interface data_device_impl;

struct wlr_seat_client *seat_client_from_data_device_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource, &wl_data_device_interface,
		&data_device_impl));
	return wl_resource_get_user_data(resource);
}

static void data_device_set_selection(struct wl_client *client,
		struct wl_resource *device_resource,
		struct wl_resource *source_resource, uint32_t serial) {
	struct wlr_seat_client *seat_client =
		seat_client_from_data_device_resource(device_resource);
	if (seat_client == NULL) {
		return;
	}

	struct wlr_client_data_source *source = NULL;
	if (source_resource != NULL) {
		source = client_data_source_from_resource(source_resource);
	}

	if (source != NULL) {
		source->finalized = true;
	}

	struct wlr_data_source *wlr_source =
		source != NULL ? &source->source : NULL;
	wlr_seat_request_set_selection(seat_client->seat, seat_client, wlr_source, serial);
}

static void data_device_start_drag(struct wl_client *client,
		struct wl_resource *device_resource,
		struct wl_resource *source_resource,
		struct wl_resource *origin_resource, struct wl_resource *icon_resource,
		uint32_t serial) {
	struct wlr_seat_client *seat_client =
		seat_client_from_data_device_resource(device_resource);
	if (seat_client == NULL) {
		return;
	}

	struct wlr_surface *origin = wlr_surface_from_resource(origin_resource);

	struct wlr_client_data_source *source = NULL;
	if (source_resource != NULL) {
		source = client_data_source_from_resource(source_resource);
	}

	struct wlr_surface *icon = NULL;
	if (icon_resource) {
		icon = wlr_surface_from_resource(icon_resource);
		if (!wlr_surface_set_role(icon, &drag_icon_surface_role, NULL,
				icon_resource, WL_DATA_DEVICE_ERROR_ROLE)) {
			return;
		}
	}

	struct wlr_data_source *wlr_source =
		source != NULL ? &source->source : NULL;
	struct wlr_drag *drag = wlr_drag_create(seat_client, wlr_source, icon);
	if (drag == NULL) {
		wl_resource_post_no_memory(device_resource);
		return;
	}

	if (source != NULL) {
		source->finalized = true;
	}

	wlr_seat_request_start_drag(seat_client->seat, drag, origin, serial);
}

static void data_device_release(struct wl_client *client,
		struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static const struct wl_data_device_interface data_device_impl = {
	.start_drag = data_device_start_drag,
	.set_selection = data_device_set_selection,
	.release = data_device_release,
};

static void data_device_handle_resource_destroy(struct wl_resource *resource) {
	wl_list_remove(wl_resource_get_link(resource));
}


static void device_resource_send_selection(struct wl_resource *device_resource) {
	struct wlr_seat_client *seat_client =
		seat_client_from_data_device_resource(device_resource);
	assert(seat_client != NULL);

	struct wlr_data_source *source = seat_client->seat->selection_source;
	if (source != NULL) {
		struct wlr_data_offer *offer = data_offer_create(device_resource,
			source, WLR_DATA_OFFER_SELECTION);
		if (offer == NULL) {
			wl_client_post_no_memory(seat_client->client);
			return;
		}

		wl_data_device_send_selection(device_resource, offer->resource);
	} else {
		wl_data_device_send_selection(device_resource, NULL);
	}
}

void seat_client_send_selection(struct wlr_seat_client *seat_client) {
	struct wlr_data_source *source = seat_client->seat->selection_source;
	if (source != NULL) {
		source->accepted = false;
	}

	// Make all current offers inert
	struct wlr_data_offer *offer, *tmp;
	wl_list_for_each_safe(offer, tmp,
			&seat_client->seat->selection_offers, link) {
		data_offer_destroy(offer);
	}

	struct wl_resource *device_resource;
	wl_resource_for_each(device_resource, &seat_client->data_devices) {
		device_resource_send_selection(device_resource);
	}
}

void wlr_seat_request_set_selection(struct wlr_seat *seat,
		struct wlr_seat_client *client,
		struct wlr_data_source *source, uint32_t serial) {
	if (client && !wlr_seat_client_validate_event_serial(client, serial)) {
		wlr_log(WLR_DEBUG, "Rejecting set_selection request, "
			"serial %"PRIu32" was never given to client", serial);
		return;
	}

	if (seat->selection_source &&
			serial - seat->selection_serial > UINT32_MAX / 2) {
		wlr_log(WLR_DEBUG, "Rejecting set_selection request, serial indicates superseded "
			"(%"PRIu32" < %"PRIu32")", serial, seat->selection_serial);
		return;
	}

	struct wlr_seat_request_set_selection_event event = {
		.source = source,
		.serial = serial,
	};
	wlr_signal_emit_safe(&seat->events.request_set_selection, &event);
}

static void seat_handle_selection_source_destroy(
		struct wl_listener *listener, void *data) {
	struct wlr_seat *seat =
		wl_container_of(listener, seat, selection_source_destroy);

	wl_list_remove(&seat->selection_source_destroy.link);
	seat->selection_source = NULL;

	struct wlr_seat_client *focused_client =
		seat->keyboard_state.focused_client;
	if (focused_client != NULL) {
		seat_client_send_selection(focused_client);
	}

	wlr_signal_emit_safe(&seat->events.set_selection, seat);
}

void wlr_seat_set_selection(struct wlr_seat *seat,
		struct wlr_data_source *source, uint32_t serial) {
	if (seat->selection_source == source) {
		seat->selection_serial = serial;
		return;
	}

	if (seat->selection_source) {
		wl_list_remove(&seat->selection_source_destroy.link);
		wlr_data_source_destroy(seat->selection_source);
		seat->selection_source = NULL;
	}

	seat->selection_source = source;
	seat->selection_serial = serial;

	if (source) {
		seat->selection_source_destroy.notify =
			seat_handle_selection_source_destroy;
		wl_signal_add(&source->events.destroy,
			&seat->selection_source_destroy);
	}

	struct wlr_seat_client *focused_client =
		seat->keyboard_state.focused_client;
	if (focused_client) {
		seat_client_send_selection(focused_client);
	}

	wlr_signal_emit_safe(&seat->events.set_selection, seat);
}


static const struct wl_data_device_manager_interface data_device_manager_impl;

static struct wlr_data_device_manager *data_device_manager_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource, &wl_data_device_manager_interface,
		&data_device_manager_impl));
	return wl_resource_get_user_data(resource);
}

static void data_device_manager_get_data_device(struct wl_client *client,
		struct wl_resource *manager_resource, uint32_t id,
		struct wl_resource *seat_resource) {
	struct wlr_seat_client *seat_client =
		wlr_seat_client_from_resource(seat_resource);

	uint32_t version = wl_resource_get_version(manager_resource);
	struct wl_resource *resource = wl_resource_create(client,
		&wl_data_device_interface, version, id);
	if (resource == NULL) {
		wl_resource_post_no_memory(manager_resource);
		return;
	}
	wl_resource_set_implementation(resource, &data_device_impl, seat_client,
		data_device_handle_resource_destroy);
	wl_list_insert(&seat_client->data_devices, wl_resource_get_link(resource));

	struct wlr_seat_client *focused_client =
		seat_client->seat->keyboard_state.focused_client;
	if (focused_client == seat_client) {
		device_resource_send_selection(resource);
	}
}

static void data_device_manager_create_data_source(struct wl_client *client,
		struct wl_resource *manager_resource, uint32_t id) {
	struct wlr_data_device_manager *manager =
		data_device_manager_from_resource(manager_resource);

	client_data_source_create(client, wl_resource_get_version(manager_resource),
		id, &manager->data_sources);
}

static const struct wl_data_device_manager_interface
		data_device_manager_impl = {
	.create_data_source = data_device_manager_create_data_source,
	.get_data_device = data_device_manager_get_data_device,
};

static void data_device_manager_bind(struct wl_client *client,
		void *data, uint32_t version, uint32_t id) {
	struct wlr_data_device_manager *manager = data;

	struct wl_resource *resource = wl_resource_create(client,
		&wl_data_device_manager_interface, version, id);
	if (resource == NULL) {
		wl_client_post_no_memory(client);
		return;
	}
	wl_resource_set_implementation(resource, &data_device_manager_impl,
		manager, NULL);
}

static void handle_display_destroy(struct wl_listener *listener, void *data) {
	struct wlr_data_device_manager *manager =
		wl_container_of(listener, manager, display_destroy);
	wlr_signal_emit_safe(&manager->events.destroy, manager);
	wl_list_remove(&manager->display_destroy.link);
	wl_global_destroy(manager->global);
	free(manager);
}

struct wlr_data_device_manager *wlr_data_device_manager_create(
		struct wl_display *display) {
	struct wlr_data_device_manager *manager =
		calloc(1, sizeof(struct wlr_data_device_manager));
	if (manager == NULL) {
		wlr_log(WLR_ERROR, "could not create data device manager");
		return NULL;
	}

	wl_list_init(&manager->data_sources);
	wl_signal_init(&manager->events.destroy);

	manager->global =
		wl_global_create(display, &wl_data_device_manager_interface,
			DATA_DEVICE_MANAGER_VERSION, manager, data_device_manager_bind);
	if (!manager->global) {
		wlr_log(WLR_ERROR, "could not create data device manager wl_global");
		free(manager);
		return NULL;
	}

	manager->display_destroy.notify = handle_display_destroy;
	wl_display_add_destroy_listener(display, &manager->display_destroy);

	return manager;
}
