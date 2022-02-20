#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wlr/types/wlr_primary_selection_v1.h>
#include <wlr/types/wlr_primary_selection.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/util/log.h>
#include "primary-selection-unstable-v1-protocol.h"
#include "util/signal.h"

#define DEVICE_MANAGER_VERSION 1

static const struct zwp_primary_selection_offer_v1_interface offer_impl;

static struct wlr_primary_selection_v1_device *device_from_offer_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource,
		&zwp_primary_selection_offer_v1_interface, &offer_impl));
	return wl_resource_get_user_data(resource);
}

static void offer_handle_receive(struct wl_client *client,
		struct wl_resource *resource, const char *mime_type, int32_t fd) {
	struct wlr_primary_selection_v1_device *device =
		device_from_offer_resource(resource);
	if (device == NULL || device->seat->primary_selection_source == NULL) {
		close(fd);
		return;
	}

	wlr_primary_selection_source_send(device->seat->primary_selection_source,
		mime_type, fd);
}

static void offer_handle_destroy(struct wl_client *client,
		struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static const struct zwp_primary_selection_offer_v1_interface offer_impl = {
	.receive = offer_handle_receive,
	.destroy = offer_handle_destroy,
};

static void offer_handle_resource_destroy(struct wl_resource *resource) {
	wl_list_remove(wl_resource_get_link(resource));
}

static struct wlr_primary_selection_v1_device *device_from_resource(
	struct wl_resource *resource);

static void create_offer(struct wl_resource *device_resource,
		struct wlr_primary_selection_source *source) {
	struct wlr_primary_selection_v1_device *device =
		device_from_resource(device_resource);
	assert(device != NULL);

	struct wl_client *client = wl_resource_get_client(device_resource);
	uint32_t version = wl_resource_get_version(device_resource);
	struct wl_resource *resource = wl_resource_create(client,
		&zwp_primary_selection_offer_v1_interface, version, 0);
	if (resource == NULL) {
		wl_resource_post_no_memory(device_resource);
		return;
	}
	wl_resource_set_implementation(resource, &offer_impl, device,
		offer_handle_resource_destroy);

	wl_list_insert(&device->offers, wl_resource_get_link(resource));

	zwp_primary_selection_device_v1_send_data_offer(device_resource, resource);

	char **p;
	wl_array_for_each(p, &source->mime_types) {
		zwp_primary_selection_offer_v1_send_offer(resource, *p);
	}

	zwp_primary_selection_device_v1_send_selection(device_resource, resource);
}

static void destroy_offer(struct wl_resource *resource) {
	if (device_from_offer_resource(resource) == NULL) {
		return;
	}

	// Make the offer inert
	wl_resource_set_user_data(resource, NULL);

	struct wl_list *link = wl_resource_get_link(resource);
	wl_list_remove(link);
	wl_list_init(link);
}


struct client_data_source {
	struct wlr_primary_selection_source source;
	struct wl_resource *resource;
	bool finalized;
};

static void client_source_send(
		struct wlr_primary_selection_source *wlr_source,
		const char *mime_type, int fd) {
	struct client_data_source *source = (struct client_data_source *)wlr_source;
	zwp_primary_selection_source_v1_send_send(source->resource, mime_type, fd);
	close(fd);
}

static void client_source_destroy(
		struct wlr_primary_selection_source *wlr_source) {
	struct client_data_source *source = (struct client_data_source *)wlr_source;
	zwp_primary_selection_source_v1_send_cancelled(source->resource);
	// Make the source resource inert
	wl_resource_set_user_data(source->resource, NULL);
	free(source);
}

static const struct wlr_primary_selection_source_impl client_source_impl = {
	.send = client_source_send,
	.destroy = client_source_destroy,
};

static const struct zwp_primary_selection_source_v1_interface source_impl;

static struct client_data_source *client_data_source_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource,
		&zwp_primary_selection_source_v1_interface, &source_impl));
	return wl_resource_get_user_data(resource);
}

static void source_handle_offer(struct wl_client *client,
		struct wl_resource *resource, const char *mime_type) {
	struct client_data_source *source =
		client_data_source_from_resource(resource);
	if (source == NULL) {
		return;
	}
	if (source->finalized) {
		wlr_log(WLR_DEBUG, "Offering additional MIME type after set_selection");
	}

	const char **mime_type_ptr;
	wl_array_for_each(mime_type_ptr, &source->source.mime_types) {
		if (strcmp(*mime_type_ptr, mime_type) == 0) {
			wlr_log(WLR_DEBUG, "Ignoring duplicate MIME type offer %s",
				mime_type);
			return;
		}
	}

	char *dup_mime_type = strdup(mime_type);
	if (dup_mime_type == NULL) {
		wl_resource_post_no_memory(resource);
		return;
	}

	char **p = wl_array_add(&source->source.mime_types, sizeof(*p));
	if (p == NULL) {
		free(dup_mime_type);
		wl_resource_post_no_memory(resource);
		return;
	}

	*p = dup_mime_type;
}

static void source_handle_destroy(struct wl_client *client,
		struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static const struct zwp_primary_selection_source_v1_interface source_impl = {
	.offer = source_handle_offer,
	.destroy = source_handle_destroy,
};

static void source_resource_handle_destroy(struct wl_resource *resource) {
	struct client_data_source *source =
		client_data_source_from_resource(resource);
	if (source == NULL) {
		return;
	}
	wlr_primary_selection_source_destroy(&source->source);
}


static const struct zwp_primary_selection_device_v1_interface device_impl;

static struct wlr_primary_selection_v1_device *device_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource,
		&zwp_primary_selection_device_v1_interface, &device_impl));
	return wl_resource_get_user_data(resource);
}

static void device_handle_set_selection(struct wl_client *client,
		struct wl_resource *resource, struct wl_resource *source_resource,
		uint32_t serial) {
	struct wlr_primary_selection_v1_device *device =
		device_from_resource(resource);
	if (device == NULL) {
		return;
	}

	struct client_data_source *client_source = NULL;
	if (source_resource != NULL) {
		client_source = client_data_source_from_resource(source_resource);
	}

	struct wlr_primary_selection_source *source = NULL;
	if (client_source != NULL) {
		client_source->finalized = true;
		source = &client_source->source;
	}

	struct wlr_seat_client *seat_client =
		wlr_seat_client_for_wl_client(device->seat, client);

	wlr_seat_request_set_primary_selection(device->seat, seat_client, source, serial);
}

static void device_handle_destroy(struct wl_client *client,
		struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static const struct zwp_primary_selection_device_v1_interface device_impl = {
	.set_selection = device_handle_set_selection,
	.destroy = device_handle_destroy,
};

static void device_handle_resource_destroy(struct wl_resource *resource) {
	wl_list_remove(wl_resource_get_link(resource));
}


static void device_resource_send_selection(struct wl_resource *resource,
		struct wlr_primary_selection_source *source) {
	assert(device_from_resource(resource) != NULL);

	if (source != NULL) {
		create_offer(resource, source);
	} else {
		zwp_primary_selection_device_v1_send_selection(resource, NULL);
	}
}

static void device_send_selection(
		struct wlr_primary_selection_v1_device *device) {
	struct wlr_seat_client *seat_client =
		device->seat->keyboard_state.focused_client;
	if (seat_client == NULL) {
		return;
	}

	struct wl_resource *resource;
	wl_resource_for_each(resource, &device->resources) {
		if (wl_resource_get_client(resource) == seat_client->client) {
			device_resource_send_selection(resource,
				device->seat->primary_selection_source);
		}
	}
}

static void device_destroy(struct wlr_primary_selection_v1_device *device);

static void device_handle_seat_destroy(struct wl_listener *listener,
		void *data) {
	struct wlr_primary_selection_v1_device *device =
		wl_container_of(listener, device, seat_destroy);
	device_destroy(device);
}

static void device_handle_seat_focus_change(struct wl_listener *listener,
		void *data) {
	struct wlr_primary_selection_v1_device *device =
		wl_container_of(listener, device, seat_focus_change);
	// TODO: maybe make previous offers inert, or set a NULL selection for
	// previous client?
	device_send_selection(device);
}

static void device_handle_seat_set_primary_selection(
		struct wl_listener *listener, void *data) {
	struct wlr_primary_selection_v1_device *device =
		wl_container_of(listener, device, seat_set_primary_selection);

	struct wl_resource *resource, *tmp;
	wl_resource_for_each_safe(resource, tmp, &device->offers) {
		destroy_offer(resource);
	}

	device_send_selection(device);
}

static struct wlr_primary_selection_v1_device *get_or_create_device(
		struct wlr_primary_selection_v1_device_manager *manager,
		struct wlr_seat *seat) {
	struct wlr_primary_selection_v1_device *device;
	wl_list_for_each(device, &manager->devices, link) {
		if (device->seat == seat) {
			return device;
		}
	}

	device = calloc(1, sizeof(struct wlr_primary_selection_v1_device));
	if (device == NULL) {
		return NULL;
	}
	device->manager = manager;
	device->seat = seat;

	wl_list_init(&device->resources);
	wl_list_insert(&manager->devices, &device->link);

	wl_list_init(&device->offers);

	device->seat_destroy.notify = device_handle_seat_destroy;
	wl_signal_add(&seat->events.destroy, &device->seat_destroy);

	device->seat_focus_change.notify = device_handle_seat_focus_change;
	wl_signal_add(&seat->keyboard_state.events.focus_change,
		&device->seat_focus_change);

	device->seat_set_primary_selection.notify =
		device_handle_seat_set_primary_selection;
	wl_signal_add(&seat->events.set_primary_selection,
		&device->seat_set_primary_selection);

	return device;
}

static void device_destroy(struct wlr_primary_selection_v1_device *device) {
	if (device == NULL) {
		return;
	}
	wl_list_remove(&device->link);
	wl_list_remove(&device->seat_destroy.link);
	wl_list_remove(&device->seat_focus_change.link);
	wl_list_remove(&device->seat_set_primary_selection.link);
	struct wl_resource *resource, *resource_tmp;
	wl_resource_for_each_safe(resource, resource_tmp, &device->offers) {
		destroy_offer(resource);
	}
	wl_resource_for_each_safe(resource, resource_tmp, &device->resources) {
		// Make the resource inert
		wl_resource_set_user_data(resource, NULL);

		struct wl_list *link = wl_resource_get_link(resource);
		wl_list_remove(link);
		wl_list_init(link);
	}
	free(device);
}


static const struct zwp_primary_selection_device_manager_v1_interface
	device_manager_impl;

static struct wlr_primary_selection_v1_device_manager *manager_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource,
		&zwp_primary_selection_device_manager_v1_interface, &device_manager_impl));
	return wl_resource_get_user_data(resource);
}

static void device_manager_handle_create_source(struct wl_client *client,
		struct wl_resource *manager_resource, uint32_t id) {
	struct client_data_source *source =
		calloc(1, sizeof(struct client_data_source));
	if (source == NULL) {
		wl_client_post_no_memory(client);
		return;
	}
	wlr_primary_selection_source_init(&source->source, &client_source_impl);

	uint32_t version = wl_resource_get_version(manager_resource);
	source->resource = wl_resource_create(client,
		&zwp_primary_selection_source_v1_interface, version, id);
	if (source->resource == NULL) {
		free(source);
		wl_client_post_no_memory(client);
		return;
	}
	wl_resource_set_implementation(source->resource, &source_impl, source,
		source_resource_handle_destroy);
}

static void device_manager_handle_get_device(struct wl_client *client,
		struct wl_resource *manager_resource, uint32_t id,
		struct wl_resource *seat_resource) {
	struct wlr_seat_client *seat_client =
		wlr_seat_client_from_resource(seat_resource);
	struct wlr_primary_selection_v1_device_manager *manager =
		manager_from_resource(manager_resource);

	struct wlr_primary_selection_v1_device *device =
		get_or_create_device(manager, seat_client->seat);
	if (device == NULL) {
		wl_resource_post_no_memory(manager_resource);
		return;
	}

	uint32_t version = wl_resource_get_version(manager_resource);
	struct wl_resource *resource = wl_resource_create(client,
		&zwp_primary_selection_device_v1_interface, version, id);
	if (resource == NULL) {
		wl_resource_post_no_memory(manager_resource);
		return;
	}
	wl_resource_set_implementation(resource, &device_impl, device,
		device_handle_resource_destroy);
	wl_list_insert(&device->resources, wl_resource_get_link(resource));

	if (device->seat->keyboard_state.focused_client == seat_client) {
		device_resource_send_selection(resource,
			device->seat->primary_selection_source);
	}
}

static void device_manager_handle_destroy(struct wl_client *client,
		struct wl_resource *manager_resource) {
	wl_resource_destroy(manager_resource);
}

static const struct zwp_primary_selection_device_manager_v1_interface
		device_manager_impl = {
	.create_source = device_manager_handle_create_source,
	.get_device = device_manager_handle_get_device,
	.destroy = device_manager_handle_destroy,
};


static void primary_selection_device_manager_bind(struct wl_client *client,
		void *data, uint32_t version, uint32_t id) {
	struct wlr_primary_selection_v1_device_manager *manager = data;

	struct wl_resource *resource = wl_resource_create(client,
		&zwp_primary_selection_device_manager_v1_interface, version, id);
	if (resource == NULL) {
		wl_client_post_no_memory(client);
		return;
	}
	wl_resource_set_implementation(resource, &device_manager_impl, manager,
		NULL);
}

static void handle_display_destroy(struct wl_listener *listener, void *data) {
	struct wlr_primary_selection_v1_device_manager *manager =
		wl_container_of(listener, manager, display_destroy);

	struct wlr_primary_selection_v1_device *device, *tmp;
	wl_list_for_each_safe(device, tmp, &manager->devices, link) {
		device_destroy(device);
	}

	wlr_signal_emit_safe(&manager->events.destroy, manager);
	wl_list_remove(&manager->display_destroy.link);
	wl_global_destroy(manager->global);
	free(manager);
}

struct wlr_primary_selection_v1_device_manager *
		wlr_primary_selection_v1_device_manager_create(
		struct wl_display *display) {
	struct wlr_primary_selection_v1_device_manager *manager =
		calloc(1, sizeof(struct wlr_primary_selection_v1_device_manager));
	if (manager == NULL) {
		return NULL;
	}
	manager->global = wl_global_create(display,
		&zwp_primary_selection_device_manager_v1_interface, DEVICE_MANAGER_VERSION,
		manager, primary_selection_device_manager_bind);
	if (manager->global == NULL) {
		free(manager);
		return NULL;
	}

	wl_list_init(&manager->devices);
	wl_signal_init(&manager->events.destroy);

	manager->display_destroy.notify = handle_display_destroy;
	wl_display_add_destroy_listener(display, &manager->display_destroy);

	return manager;
}
