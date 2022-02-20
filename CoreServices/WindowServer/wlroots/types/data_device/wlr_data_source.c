#define _POSIX_C_SOURCE 200809L
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

void wlr_data_source_init(struct wlr_data_source *source,
		const struct wlr_data_source_impl *impl) {
	assert(impl->send);

	source->impl = impl;
	wl_array_init(&source->mime_types);
	wl_signal_init(&source->events.destroy);
	source->actions = -1;
}

void wlr_data_source_send(struct wlr_data_source *source, const char *mime_type,
		int32_t fd) {
	source->impl->send(source, mime_type, fd);
}

void wlr_data_source_accept(struct wlr_data_source *source, uint32_t serial,
		const char *mime_type) {
	source->accepted = (mime_type != NULL);
	if (source->impl->accept) {
		source->impl->accept(source, serial, mime_type);
	}
}

void wlr_data_source_destroy(struct wlr_data_source *source) {
	if (source == NULL) {
		return;
	}

	wlr_signal_emit_safe(&source->events.destroy, source);

	char **p;
	wl_array_for_each(p, &source->mime_types) {
		free(*p);
	}
	wl_array_release(&source->mime_types);

	if (source->impl->destroy) {
		source->impl->destroy(source);
	} else {
		free(source);
	}
}

void wlr_data_source_dnd_drop(struct wlr_data_source *source) {
	if (source->impl->dnd_drop) {
		source->impl->dnd_drop(source);
	}
}

void wlr_data_source_dnd_finish(struct wlr_data_source *source) {
	if (source->impl->dnd_finish) {
		source->impl->dnd_finish(source);
	}
}

void wlr_data_source_dnd_action(struct wlr_data_source *source,
		enum wl_data_device_manager_dnd_action action) {
	source->current_dnd_action = action;
	if (source->impl->dnd_action) {
		source->impl->dnd_action(source, action);
	}
}


static const struct wl_data_source_interface data_source_impl;

struct wlr_client_data_source *client_data_source_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource, &wl_data_source_interface,
		&data_source_impl));
	return wl_resource_get_user_data(resource);
}

static void client_data_source_accept(struct wlr_data_source *wlr_source,
	uint32_t serial, const char *mime_type);

static struct wlr_client_data_source *client_data_source_from_wlr_data_source(
		struct wlr_data_source *wlr_source) {
	assert(wlr_source->impl->accept == client_data_source_accept);
	return (struct wlr_client_data_source *)wlr_source;
}

static void client_data_source_accept(struct wlr_data_source *wlr_source,
		uint32_t serial, const char *mime_type) {
	struct wlr_client_data_source *source =
		client_data_source_from_wlr_data_source(wlr_source);
	wl_data_source_send_target(source->resource, mime_type);
}

static void client_data_source_send(struct wlr_data_source *wlr_source,
		const char *mime_type, int32_t fd) {
	struct wlr_client_data_source *source =
		client_data_source_from_wlr_data_source(wlr_source);
	wl_data_source_send_send(source->resource, mime_type, fd);
	close(fd);
}

static void client_data_source_destroy(struct wlr_data_source *wlr_source) {
	struct wlr_client_data_source *source =
		client_data_source_from_wlr_data_source(wlr_source);
	wl_data_source_send_cancelled(source->resource);
	wl_resource_set_user_data(source->resource, NULL);
	free(source);
}

static void client_data_source_dnd_drop(struct wlr_data_source *wlr_source) {
	struct wlr_client_data_source *source =
		client_data_source_from_wlr_data_source(wlr_source);
	assert(wl_resource_get_version(source->resource) >=
		WL_DATA_SOURCE_DND_DROP_PERFORMED_SINCE_VERSION);
	wl_data_source_send_dnd_drop_performed(source->resource);
}

static void client_data_source_dnd_finish(struct wlr_data_source *wlr_source) {
	struct wlr_client_data_source *source =
		client_data_source_from_wlr_data_source(wlr_source);
	assert(wl_resource_get_version(source->resource) >=
		WL_DATA_SOURCE_DND_FINISHED_SINCE_VERSION);
	wl_data_source_send_dnd_finished(source->resource);
}

static void client_data_source_dnd_action(struct wlr_data_source *wlr_source,
		enum wl_data_device_manager_dnd_action action) {
	struct wlr_client_data_source *source =
		client_data_source_from_wlr_data_source(wlr_source);
	assert(wl_resource_get_version(source->resource) >=
		WL_DATA_SOURCE_ACTION_SINCE_VERSION);
	wl_data_source_send_action(source->resource, action);
}

static void data_source_destroy(struct wl_client *client,
		struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static void data_source_set_actions(struct wl_client *client,
		struct wl_resource *resource, uint32_t dnd_actions) {
	struct wlr_client_data_source *source =
		client_data_source_from_resource(resource);
	if (source == NULL) {
		return;
	}

	if (source->source.actions >= 0) {
		wl_resource_post_error(source->resource,
			WL_DATA_SOURCE_ERROR_INVALID_ACTION_MASK,
			"cannot set actions more than once");
		return;
	}

	if (dnd_actions & ~DATA_DEVICE_ALL_ACTIONS) {
		wl_resource_post_error(source->resource,
			WL_DATA_SOURCE_ERROR_INVALID_ACTION_MASK,
			"invalid action mask %x", dnd_actions);
		return;
	}

	if (source->finalized) {
		wl_resource_post_error(source->resource,
			WL_DATA_SOURCE_ERROR_INVALID_ACTION_MASK,
			"invalid action change after wl_data_device.start_drag");
		return;
	}

	source->source.actions = dnd_actions;
}

static void data_source_offer(struct wl_client *client,
		struct wl_resource *resource, const char *mime_type) {
	struct wlr_client_data_source *source =
		client_data_source_from_resource(resource);
	if (source == NULL) {
		return;
	}
	if (source->finalized) {
		wlr_log(WLR_DEBUG, "Offering additional MIME type after "
			"wl_data_device.set_selection");
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

static const struct wl_data_source_interface data_source_impl = {
	.destroy = data_source_destroy,
	.offer = data_source_offer,
	.set_actions = data_source_set_actions,
};

static void data_source_handle_resource_destroy(struct wl_resource *resource) {
	struct wlr_client_data_source *source =
		client_data_source_from_resource(resource);
	if (source != NULL) {
		wlr_data_source_destroy(&source->source);
	}
	wl_list_remove(wl_resource_get_link(resource));
}

struct wlr_client_data_source *client_data_source_create(
		struct wl_client *client, uint32_t version, uint32_t id,
		struct wl_list *resource_list) {
	struct wlr_client_data_source *source =
		calloc(1, sizeof(struct wlr_client_data_source));
	if (source == NULL) {
		return NULL;
	}

	source->resource = wl_resource_create(client, &wl_data_source_interface,
		version, id);
	if (source->resource == NULL) {
		wl_resource_post_no_memory(source->resource);
		free(source);
		return NULL;
	}
	wl_resource_set_implementation(source->resource, &data_source_impl,
		source, data_source_handle_resource_destroy);
	wl_list_insert(resource_list, wl_resource_get_link(source->resource));

	source->impl.accept = client_data_source_accept;
	source->impl.send = client_data_source_send;
	source->impl.destroy = client_data_source_destroy;

	if (wl_resource_get_version(source->resource) >=
			WL_DATA_SOURCE_DND_DROP_PERFORMED_SINCE_VERSION) {
		source->impl.dnd_drop = client_data_source_dnd_drop;
	}
	if (wl_resource_get_version(source->resource) >=
			WL_DATA_SOURCE_DND_FINISHED_SINCE_VERSION) {
		source->impl.dnd_finish = client_data_source_dnd_finish;
	}
	if (wl_resource_get_version(source->resource) >=
			WL_DATA_SOURCE_ACTION_SINCE_VERSION) {
		source->impl.dnd_action = client_data_source_dnd_action;
	}

	wlr_data_source_init(&source->source, &source->impl);
	return source;
}
