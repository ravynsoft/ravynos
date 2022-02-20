#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wlr/types/wlr_data_control_v1.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_primary_selection.h>
#include <wlr/util/log.h>
#include "util/signal.h"
#include "wlr-data-control-unstable-v1-protocol.h"

#define DATA_CONTROL_MANAGER_VERSION 2

struct data_control_source {
	struct wl_resource *resource;
	struct wl_array mime_types;
	bool finalized;

	// Only one of these is non-NULL.
	struct wlr_data_source *active_source;
	struct wlr_primary_selection_source *active_primary_source;
};

static const struct zwlr_data_control_source_v1_interface source_impl;

static struct data_control_source *source_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource,
		&zwlr_data_control_source_v1_interface, &source_impl));
	return wl_resource_get_user_data(resource);
}

static void source_handle_offer(struct wl_client *client,
		struct wl_resource *resource, const char *mime_type) {
	struct data_control_source *source = source_from_resource(resource);
	if (source == NULL) {
		return;
	}

	if (source->finalized) {
		wl_resource_post_error(resource,
			ZWLR_DATA_CONTROL_SOURCE_V1_ERROR_INVALID_OFFER,
			"cannot mutate offer after set_selection or "
			"set_primary_selection");
		return;
	}

	const char **mime_type_ptr;
	wl_array_for_each(mime_type_ptr, &source->mime_types) {
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

	char **p = wl_array_add(&source->mime_types, sizeof(char *));
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

static const struct zwlr_data_control_source_v1_interface source_impl = {
	.offer = source_handle_offer,
	.destroy = source_handle_destroy,
};

static void data_control_source_destroy(struct data_control_source *source) {
	if (source == NULL) {
		return;
	}

	char **p;
	wl_array_for_each(p, &source->mime_types) {
		free(*p);
	}
	wl_array_release(&source->mime_types);

	// Prevent destructors below from calling this recursively.
	wl_resource_set_user_data(source->resource, NULL);

	if (source->active_source != NULL) {
		wlr_data_source_destroy(source->active_source);
	} else if (source->active_primary_source != NULL) {
		wlr_primary_selection_source_destroy(
			source->active_primary_source);
	}

	free(source);
}

static void source_handle_resource_destroy(struct wl_resource *resource) {
	struct data_control_source *source = source_from_resource(resource);
	data_control_source_destroy(source);
}


struct client_data_source {
	struct wlr_data_source source;
	struct wl_resource *resource;
};

static const struct wlr_data_source_impl client_source_impl;

static struct client_data_source *
		client_data_source_from_source(struct wlr_data_source *wlr_source) {
	assert(wlr_source->impl == &client_source_impl);
	return (struct client_data_source *)wlr_source;
}

static void client_source_send(struct wlr_data_source *wlr_source,
		const char *mime_type, int fd) {
	struct client_data_source *source =
		client_data_source_from_source(wlr_source);
	zwlr_data_control_source_v1_send_send(source->resource, mime_type, fd);
	close(fd);
}

static void client_source_destroy(struct wlr_data_source *wlr_source) {
	struct client_data_source *client_source =
		client_data_source_from_source(wlr_source);
	struct data_control_source *source =
		source_from_resource(client_source->resource);
	free(client_source);

	if (source == NULL) {
		return;
	}

	source->active_source = NULL;

	zwlr_data_control_source_v1_send_cancelled(source->resource);
	data_control_source_destroy(source);
}

static const struct wlr_data_source_impl client_source_impl = {
	.send = client_source_send,
	.destroy = client_source_destroy,
};


struct client_primary_selection_source {
	struct wlr_primary_selection_source source;
	struct wl_resource *resource;
};

static const struct wlr_primary_selection_source_impl
client_primary_selection_source_impl;

static struct client_primary_selection_source *
		client_primary_selection_source_from_source(
			struct wlr_primary_selection_source *wlr_source) {
	assert(wlr_source->impl == &client_primary_selection_source_impl);
	return (struct client_primary_selection_source *)wlr_source;
}

static void client_primary_selection_source_send(
		struct wlr_primary_selection_source *wlr_source,
		const char *mime_type, int fd) {
	struct client_primary_selection_source *source =
		client_primary_selection_source_from_source(wlr_source);
	zwlr_data_control_source_v1_send_send(source->resource, mime_type, fd);
	close(fd);
}

static void client_primary_selection_source_destroy(
		struct wlr_primary_selection_source *wlr_source) {
	struct client_primary_selection_source *client_source =
		client_primary_selection_source_from_source(wlr_source);
	struct data_control_source *source =
		source_from_resource(client_source->resource);
	free(client_source);

	if (source == NULL) {
		return;
	}

	source->active_primary_source = NULL;

	zwlr_data_control_source_v1_send_cancelled(source->resource);
	data_control_source_destroy(source);
}

static const struct wlr_primary_selection_source_impl
client_primary_selection_source_impl = {
	.send = client_primary_selection_source_send,
	.destroy = client_primary_selection_source_destroy,
};


struct data_offer {
	struct wl_resource *resource;
	struct wlr_data_control_device_v1 *device;
	bool is_primary;
};

static void data_offer_destroy(struct data_offer *offer) {
	if (offer == NULL) {
		return;
	}

	struct wlr_data_control_device_v1 *device = offer->device;
	if (device != NULL) {
		if (offer->is_primary) {
			device->primary_selection_offer_resource = NULL;
		} else {
			device->selection_offer_resource = NULL;
		}
	}

	wl_resource_set_user_data(offer->resource, NULL);
	free(offer);
}

static const struct zwlr_data_control_offer_v1_interface offer_impl;

static struct data_offer *data_offer_from_offer_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource,
		&zwlr_data_control_offer_v1_interface, &offer_impl));
	return wl_resource_get_user_data(resource);
}

static void offer_handle_receive(struct wl_client *client,
		struct wl_resource *resource, const char *mime_type, int fd) {
	struct data_offer *offer = data_offer_from_offer_resource(resource);
	if (offer == NULL) {
		close(fd);
		return;
	}

	struct wlr_data_control_device_v1 *device = offer->device;
	if (device == NULL) {
		close(fd);
		return;
	}

	if (offer->is_primary) {
		if (device->seat->primary_selection_source == NULL) {
			close(fd);
			return;
		}
		wlr_primary_selection_source_send(
			device->seat->primary_selection_source,
			mime_type, fd);
	} else {
		if (device->seat->selection_source == NULL) {
			close(fd);
			return;
		}
		wlr_data_source_send(device->seat->selection_source, mime_type, fd);
	}
}

static void offer_handle_destroy(struct wl_client *client,
		struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static const struct zwlr_data_control_offer_v1_interface offer_impl = {
	.receive = offer_handle_receive,
	.destroy = offer_handle_destroy,
};

static void offer_handle_resource_destroy(struct wl_resource *resource) {
	struct data_offer *offer = data_offer_from_offer_resource(resource);
	data_offer_destroy(offer);
}

static struct wl_resource *create_offer(struct wlr_data_control_device_v1 *device,
		struct wl_array *mime_types, bool is_primary) {
	struct wl_client *client = wl_resource_get_client(device->resource);

	struct data_offer *offer = calloc(1, sizeof(struct data_offer));
	if (offer == NULL) {
		wl_client_post_no_memory(client);
		return NULL;
	}

	offer->device = device;
	offer->is_primary = is_primary;

	uint32_t version = wl_resource_get_version(device->resource);
	struct wl_resource *resource = wl_resource_create(client,
		&zwlr_data_control_offer_v1_interface, version, 0);
	if (resource == NULL) {
		free(offer);
		return NULL;
	}

	offer->resource = resource;

	wl_resource_set_implementation(resource, &offer_impl, offer,
		offer_handle_resource_destroy);

	zwlr_data_control_device_v1_send_data_offer(device->resource, resource);

	char **p;
	wl_array_for_each(p, mime_types) {
		zwlr_data_control_offer_v1_send_offer(resource, *p);
	}

	return resource;
}


static const struct zwlr_data_control_device_v1_interface control_impl;

static struct wlr_data_control_device_v1 *control_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource,
		&zwlr_data_control_device_v1_interface, &control_impl));
	return wl_resource_get_user_data(resource);
}

static void control_handle_set_selection(struct wl_client *client,
		struct wl_resource *control_resource,
		struct wl_resource *source_resource) {
	struct wlr_data_control_device_v1 *device =
		control_from_resource(control_resource);
	if (device == NULL) {
		return;
	}

	struct data_control_source *source = NULL;
	if (source_resource != NULL) {
		source = source_from_resource(source_resource);
	}

	if (source == NULL) {
		wlr_seat_request_set_selection(device->seat, NULL, NULL,
			wl_display_next_serial(device->seat->display));

		return;
	}

	if (source->active_source != NULL ||
			source->active_primary_source != NULL) {
		wl_resource_post_error(control_resource,
			ZWLR_DATA_CONTROL_DEVICE_V1_ERROR_USED_SOURCE,
			"cannot use a data source in set_selection or "
			"set_primary_selection more than once");

		return;
	}

	struct client_data_source *client_source =
		calloc(1, sizeof(struct client_data_source));
	if (client_source == NULL) {
		wl_client_post_no_memory(client);
		return;
	}
	client_source->resource = source_resource;

	struct wlr_data_source *wlr_source = &client_source->source;
	wlr_data_source_init(wlr_source, &client_source_impl);
	source->active_source = wlr_source;

	wl_array_release(&wlr_source->mime_types);
	wlr_source->mime_types = source->mime_types;
	wl_array_init(&source->mime_types);

	source->finalized = true;

	wlr_seat_request_set_selection(device->seat, NULL, wlr_source,
		wl_display_next_serial(device->seat->display));
}

static void control_handle_set_primary_selection(struct wl_client *client,
		struct wl_resource *control_resource,
		struct wl_resource *source_resource) {
	struct wlr_data_control_device_v1 *device =
		control_from_resource(control_resource);
	if (device == NULL) {
		return;
	}

	struct data_control_source *source = NULL;
	if (source_resource != NULL) {
		source = source_from_resource(source_resource);
	}

	if (source == NULL) {
		wlr_seat_request_set_primary_selection(device->seat, NULL, NULL,
			wl_display_next_serial(device->seat->display));

		return;
	}

	if (source->active_source != NULL ||
			source->active_primary_source != NULL) {
		wl_resource_post_error(control_resource,
			ZWLR_DATA_CONTROL_DEVICE_V1_ERROR_USED_SOURCE,
			"cannot use a data source in set_selection or "
			"set_primary_selection more than once");

		return;
	}

	struct client_primary_selection_source *client_source =
		calloc(1, sizeof(struct client_primary_selection_source));
	if (client_source == NULL) {
		wl_client_post_no_memory(client);
		return;
	}
	client_source->resource = source_resource;

	struct wlr_primary_selection_source *wlr_source = &client_source->source;
	wlr_primary_selection_source_init(wlr_source, &client_primary_selection_source_impl);
	source->active_primary_source = wlr_source;

	wl_array_release(&wlr_source->mime_types);
	wlr_source->mime_types = source->mime_types;
	wl_array_init(&source->mime_types);

	source->finalized = true;

	wlr_seat_request_set_primary_selection(device->seat, NULL, wlr_source,
		wl_display_next_serial(device->seat->display));
}

static void control_handle_destroy(struct wl_client *client,
		struct wl_resource *control_resource) {
	wl_resource_destroy(control_resource);
}

static const struct zwlr_data_control_device_v1_interface control_impl = {
	.set_selection = control_handle_set_selection,
	.set_primary_selection = control_handle_set_primary_selection,
	.destroy = control_handle_destroy,
};

static void control_send_selection(struct wlr_data_control_device_v1 *device) {
	struct wlr_data_source *source = device->seat->selection_source;

	if (device->selection_offer_resource != NULL) {
		// Make the offer inert
		struct data_offer *offer = data_offer_from_offer_resource(
			device->selection_offer_resource);
		data_offer_destroy(offer);
	}

	device->selection_offer_resource = NULL;
	if (source != NULL) {
		device->selection_offer_resource =
			create_offer(device, &source->mime_types, false);
		if (device->selection_offer_resource == NULL) {
			wl_resource_post_no_memory(device->resource);
			return;
		}
	}

	zwlr_data_control_device_v1_send_selection(device->resource,
		device->selection_offer_resource);
}

static void control_send_primary_selection(
		struct wlr_data_control_device_v1 *device) {
	uint32_t version = wl_resource_get_version(device->resource);
	if (version < ZWLR_DATA_CONTROL_DEVICE_V1_PRIMARY_SELECTION_SINCE_VERSION) {
		return;
	}

	struct wlr_primary_selection_source *source =
		device->seat->primary_selection_source;

	if (device->primary_selection_offer_resource != NULL) {
		// Make the offer inert
		struct data_offer *offer = data_offer_from_offer_resource(
			device->primary_selection_offer_resource);
		data_offer_destroy(offer);
	}

	device->primary_selection_offer_resource = NULL;
	if (source != NULL) {
		device->primary_selection_offer_resource =
			create_offer(device, &source->mime_types, true);
		if (device->primary_selection_offer_resource == NULL) {
			wl_resource_post_no_memory(device->resource);
			return;
		}
	}

	zwlr_data_control_device_v1_send_primary_selection(device->resource,
		device->primary_selection_offer_resource);
}

static void control_handle_resource_destroy(struct wl_resource *resource) {
	struct wlr_data_control_device_v1 *device = control_from_resource(resource);
	wlr_data_control_device_v1_destroy(device);
}

static void control_handle_seat_destroy(struct wl_listener *listener,
		void *data) {
	struct wlr_data_control_device_v1 *device =
		wl_container_of(listener, device, seat_destroy);
	wlr_data_control_device_v1_destroy(device);
}

static void control_handle_seat_set_selection(struct wl_listener *listener,
		void *data) {
	struct wlr_data_control_device_v1 *device =
		wl_container_of(listener, device, seat_set_selection);
	control_send_selection(device);
}

static void control_handle_seat_set_primary_selection(
		struct wl_listener *listener,
		void *data) {
	struct wlr_data_control_device_v1 *device =
		wl_container_of(listener, device, seat_set_primary_selection);
	control_send_primary_selection(device);
}

void wlr_data_control_device_v1_destroy(struct wlr_data_control_device_v1 *device) {
	if (device == NULL) {
		return;
	}
	zwlr_data_control_device_v1_send_finished(device->resource);
	// Make the resources inert
	wl_resource_set_user_data(device->resource, NULL);
	if (device->selection_offer_resource != NULL) {
		struct data_offer *offer = data_offer_from_offer_resource(
			device->selection_offer_resource);
		data_offer_destroy(offer);
	}
	if (device->primary_selection_offer_resource != NULL) {
		struct data_offer *offer = data_offer_from_offer_resource(
			device->primary_selection_offer_resource);
		data_offer_destroy(offer);
	}
	wl_list_remove(&device->seat_destroy.link);
	wl_list_remove(&device->seat_set_selection.link);
	wl_list_remove(&device->seat_set_primary_selection.link);
	wl_list_remove(&device->link);
	free(device);
}


static const struct zwlr_data_control_manager_v1_interface manager_impl;

static struct wlr_data_control_manager_v1 *manager_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource,
		&zwlr_data_control_manager_v1_interface, &manager_impl));
	return wl_resource_get_user_data(resource);
}

static void manager_handle_create_data_source(struct wl_client *client,
		struct wl_resource *manager_resource, uint32_t id) {
	struct data_control_source *source =
		calloc(1, sizeof(struct data_control_source));
	if (source == NULL) {
		wl_resource_post_no_memory(manager_resource);
		return;
	}

	wl_array_init(&source->mime_types);

	uint32_t version = wl_resource_get_version(manager_resource);
	source->resource = wl_resource_create(client,
		&zwlr_data_control_source_v1_interface, version, id);
	if (source->resource == NULL) {
		wl_resource_post_no_memory(manager_resource);
		wl_array_release(&source->mime_types);
		free(source);
		return;
	}
	wl_resource_set_implementation(source->resource, &source_impl, source,
		source_handle_resource_destroy);
}

static void manager_handle_get_data_device(struct wl_client *client,
		struct wl_resource *manager_resource, uint32_t id,
		struct wl_resource *seat_resource) {
	struct wlr_data_control_manager_v1 *manager =
		manager_from_resource(manager_resource);
	struct wlr_seat_client *seat_client =
		wlr_seat_client_from_resource(seat_resource);

	struct wlr_data_control_device_v1 *device =
		calloc(1, sizeof(struct wlr_data_control_device_v1));
	if (device == NULL) {
		wl_resource_post_no_memory(manager_resource);
		return;
	}
	device->manager = manager;
	device->seat = seat_client->seat;

	uint32_t version = wl_resource_get_version(manager_resource);
	device->resource = wl_resource_create(client,
		&zwlr_data_control_device_v1_interface, version, id);
	if (device->resource == NULL) {
		wl_resource_post_no_memory(manager_resource);
		free(device);
		return;
	}
	wl_resource_set_implementation(device->resource, &control_impl, device,
		control_handle_resource_destroy);
	struct wl_resource *resource = device->resource;

	device->seat_destroy.notify = control_handle_seat_destroy;
	wl_signal_add(&device->seat->events.destroy, &device->seat_destroy);

	device->seat_set_selection.notify = control_handle_seat_set_selection;
	wl_signal_add(&device->seat->events.set_selection,
		&device->seat_set_selection);

	device->seat_set_primary_selection.notify =
		control_handle_seat_set_primary_selection;
	wl_signal_add(&device->seat->events.set_primary_selection,
		&device->seat_set_primary_selection);

	wl_list_insert(&manager->devices, &device->link);
	wlr_signal_emit_safe(&manager->events.new_device, device);

	// At this point maybe the compositor decided to destroy the device. If
	// it's the case then the resource will be inert.
	device = control_from_resource(resource);
	if (device != NULL) {
		control_send_selection(device);
		control_send_primary_selection(device);
	}
}

static void manager_handle_destroy(struct wl_client *client,
		struct wl_resource *manager_resource) {
	wl_resource_destroy(manager_resource);
}

static const struct zwlr_data_control_manager_v1_interface manager_impl = {
	.create_data_source = manager_handle_create_data_source,
	.get_data_device = manager_handle_get_data_device,
	.destroy = manager_handle_destroy,
};

static void manager_bind(struct wl_client *client, void *data, uint32_t version,
		uint32_t id) {
	struct wlr_data_control_manager_v1 *manager = data;

	struct wl_resource *resource = wl_resource_create(client,
		&zwlr_data_control_manager_v1_interface, version, id);
	if (resource == NULL) {
		wl_client_post_no_memory(client);
		return;
	}
	wl_resource_set_implementation(resource, &manager_impl, manager, NULL);
}

static void handle_display_destroy(struct wl_listener *listener, void *data) {
	struct wlr_data_control_manager_v1 *manager =
		wl_container_of(listener, manager, display_destroy);
	wlr_signal_emit_safe(&manager->events.destroy, manager);
	wl_list_remove(&manager->display_destroy.link);
	wl_global_destroy(manager->global);
	free(manager);
}

struct wlr_data_control_manager_v1 *wlr_data_control_manager_v1_create(
		struct wl_display *display) {
	struct wlr_data_control_manager_v1 *manager =
		calloc(1, sizeof(struct wlr_data_control_manager_v1));
	if (manager == NULL) {
		return NULL;
	}
	wl_list_init(&manager->devices);
	wl_signal_init(&manager->events.destroy);
	wl_signal_init(&manager->events.new_device);

	manager->global = wl_global_create(display,
		&zwlr_data_control_manager_v1_interface,
		DATA_CONTROL_MANAGER_VERSION, manager, manager_bind);
	if (manager->global == NULL) {
		free(manager);
		return NULL;
	}

	manager->display_destroy.notify = handle_display_destroy;
	wl_display_add_destroy_listener(display, &manager->display_destroy);

	return manager;
}
