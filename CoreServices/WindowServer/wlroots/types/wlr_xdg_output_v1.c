#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_xdg_output_v1.h>
#include <wlr/util/log.h>
#include "xdg-output-unstable-v1-protocol.h"
#include "util/signal.h"

#define OUTPUT_MANAGER_VERSION 3
#define OUTPUT_DONE_DEPRECATED_SINCE_VERSION 3
#define OUTPUT_DESCRIPTION_MUTABLE_SINCE_VERSION 3

static void output_handle_destroy(struct wl_client *client,
		struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static const struct zxdg_output_v1_interface output_implementation = {
	.destroy = output_handle_destroy,
};

static void output_handle_resource_destroy(struct wl_resource *resource) {
	wl_list_remove(wl_resource_get_link(resource));
}

static void output_send_details(struct wlr_xdg_output_v1 *xdg_output,
		struct wl_resource *resource) {
	zxdg_output_v1_send_logical_position(resource,
		xdg_output->x, xdg_output->y);
	zxdg_output_v1_send_logical_size(resource,
		xdg_output->width, xdg_output->height);
	if (wl_resource_get_version(resource) < OUTPUT_DONE_DEPRECATED_SINCE_VERSION) {
		zxdg_output_v1_send_done(resource);
	}
}

static void output_update(struct wlr_xdg_output_v1 *xdg_output) {
	struct wlr_output_layout_output *layout_output = xdg_output->layout_output;
	bool updated = false;

	if (layout_output->x != xdg_output->x || layout_output->y != xdg_output->y) {
		xdg_output->x = layout_output->x;
		xdg_output->y = layout_output->y;
		updated = true;
	}

	int width, height;
	wlr_output_effective_resolution(layout_output->output, &width, &height);
	if (xdg_output->width != width || xdg_output->height != height) {
		xdg_output->width = width;
		xdg_output->height = height;
		updated = true;
	}

	if (updated) {
		struct wl_resource *resource;
		wl_resource_for_each(resource, &xdg_output->resources) {
			output_send_details(xdg_output, resource);
		}

		wlr_output_schedule_done(xdg_output->layout_output->output);
	}
}

static void output_destroy(struct wlr_xdg_output_v1 *output) {
	struct wl_resource *resource, *tmp;
	wl_resource_for_each_safe(resource, tmp, &output->resources) {
		wl_list_remove(wl_resource_get_link(resource));
		wl_list_init(wl_resource_get_link(resource));
	}
	wl_list_remove(&output->destroy.link);
	wl_list_remove(&output->description.link);
	wl_list_remove(&output->link);
	free(output);
}


static void output_manager_handle_destroy(struct wl_client *client,
		struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static const struct zxdg_output_manager_v1_interface
	output_manager_implementation;

static void output_manager_handle_get_xdg_output(struct wl_client *client,
		struct wl_resource *resource, uint32_t id,
		struct wl_resource *output_resource) {
	assert(wl_resource_instance_of(resource, &zxdg_output_manager_v1_interface,
		&output_manager_implementation));

	struct wlr_xdg_output_manager_v1 *manager =
		wl_resource_get_user_data(resource);
	struct wlr_output_layout *layout = manager->layout;
	struct wlr_output *output = wlr_output_from_resource(output_resource);

	struct wl_resource *xdg_output_resource = wl_resource_create(client,
			&zxdg_output_v1_interface, wl_resource_get_version(resource), id);
	if (!xdg_output_resource) {
		wl_client_post_no_memory(client);
		return;
	}
	wl_resource_set_implementation(xdg_output_resource, &output_implementation,
		NULL, output_handle_resource_destroy);

	if (output == NULL) {
		wl_list_init(wl_resource_get_link(xdg_output_resource));
		return;
	}

	struct wlr_output_layout_output *layout_output =
		wlr_output_layout_get(layout, output);
	assert(layout_output);

	struct wlr_xdg_output_v1 *_xdg_output, *xdg_output = NULL;
	wl_list_for_each(_xdg_output, &manager->outputs, link) {
		if (_xdg_output->layout_output == layout_output) {
			xdg_output = _xdg_output;
			break;
		}
	}
	assert(xdg_output);


	wl_list_insert(&xdg_output->resources,
		wl_resource_get_link(xdg_output_resource));

	// Name and description should only be sent once per output
	uint32_t version = wl_resource_get_version(xdg_output_resource);
	if (version >= ZXDG_OUTPUT_V1_NAME_SINCE_VERSION) {
		zxdg_output_v1_send_name(xdg_output_resource, output->name);
	}
	if (version >= ZXDG_OUTPUT_V1_DESCRIPTION_SINCE_VERSION &&
			output->description != NULL) {
		zxdg_output_v1_send_description(xdg_output_resource,
			output->description);
	}

	output_send_details(xdg_output, xdg_output_resource);

	wl_output_send_done(output_resource);
}

static const struct zxdg_output_manager_v1_interface
		output_manager_implementation = {
	.destroy = output_manager_handle_destroy,
	.get_xdg_output = output_manager_handle_get_xdg_output,
};

static void output_manager_bind(struct wl_client *wl_client, void *data,
		uint32_t version, uint32_t id) {
	struct wlr_xdg_output_manager_v1 *manager = data;

	struct wl_resource *resource = wl_resource_create(wl_client,
		&zxdg_output_manager_v1_interface, version, id);
	if (resource == NULL) {
		wl_client_post_no_memory(wl_client);
		return;
	}
	wl_resource_set_implementation(resource, &output_manager_implementation,
		manager, NULL);
}

static void handle_output_destroy(struct wl_listener *listener, void *data) {
	struct wlr_xdg_output_v1 *output = wl_container_of(listener, output, destroy);
	output_destroy(output);
}

static void handle_output_description(struct wl_listener *listener,
		void *data) {
	struct wlr_xdg_output_v1 *xdg_output =
		wl_container_of(listener, xdg_output, description);
	struct wlr_output *output = xdg_output->layout_output->output;

	if (output->description == NULL) {
		return;
	}

	struct wl_resource *resource;
	wl_resource_for_each(resource, &xdg_output->resources) {
		if (wl_resource_get_version(resource) >=
				OUTPUT_DESCRIPTION_MUTABLE_SINCE_VERSION) {
			zxdg_output_v1_send_description(resource, output->description);
		}
	}
}

static void add_output(struct wlr_xdg_output_manager_v1 *manager,
		struct wlr_output_layout_output *layout_output) {
	struct wlr_xdg_output_v1 *output = calloc(1, sizeof(struct wlr_xdg_output_v1));
	if (output == NULL) {
		return;
	}
	wl_list_init(&output->resources);
	output->manager = manager;
	output->layout_output = layout_output;
	output->destroy.notify = handle_output_destroy;
	wl_signal_add(&layout_output->events.destroy, &output->destroy);
	output->description.notify = handle_output_description;
	wl_signal_add(&layout_output->output->events.description,
		&output->description);
	wl_list_insert(&manager->outputs, &output->link);
	output_update(output);
}

static void output_manager_send_details(
		struct wlr_xdg_output_manager_v1 *manager) {
	struct wlr_xdg_output_v1 *output;
	wl_list_for_each(output, &manager->outputs, link) {
		output_update(output);
	}
}

static void handle_layout_add(struct wl_listener *listener, void *data) {
	struct wlr_xdg_output_manager_v1 *manager =
		wl_container_of(listener, manager, layout_add);
	struct wlr_output_layout_output *layout_output = data;
	add_output(manager, layout_output);
}

static void handle_layout_change(struct wl_listener *listener, void *data) {
	struct wlr_xdg_output_manager_v1 *manager =
		wl_container_of(listener, manager, layout_change);
	output_manager_send_details(manager);
}

static void manager_destroy(struct wlr_xdg_output_manager_v1 *manager) {
	struct wlr_xdg_output_v1 *output, *tmp;
	wl_list_for_each_safe(output, tmp, &manager->outputs, link) {
		output_destroy(output);
	}
	wlr_signal_emit_safe(&manager->events.destroy, manager);
	wl_list_remove(&manager->display_destroy.link);
	wl_list_remove(&manager->layout_add.link);
	wl_list_remove(&manager->layout_change.link);
	wl_list_remove(&manager->layout_destroy.link);
	free(manager);
}

static void handle_layout_destroy(struct wl_listener *listener, void *data) {
	struct wlr_xdg_output_manager_v1 *manager =
		wl_container_of(listener, manager, layout_destroy);
	manager_destroy(manager);
}

static void handle_display_destroy(struct wl_listener *listener, void *data) {
	struct wlr_xdg_output_manager_v1 *manager =
		wl_container_of(listener, manager, display_destroy);
	manager_destroy(manager);
}

struct wlr_xdg_output_manager_v1 *wlr_xdg_output_manager_v1_create(
		struct wl_display *display, struct wlr_output_layout *layout) {
	struct wlr_xdg_output_manager_v1 *manager =
		calloc(1, sizeof(struct wlr_xdg_output_manager_v1));
	if (manager == NULL) {
		return NULL;
	}
	manager->layout = layout;
	manager->global = wl_global_create(display,
		&zxdg_output_manager_v1_interface, OUTPUT_MANAGER_VERSION, manager,
		output_manager_bind);
	if (!manager->global) {
		free(manager);
		return NULL;
	}

	wl_list_init(&manager->outputs);
	struct wlr_output_layout_output *layout_output;
	wl_list_for_each(layout_output, &layout->outputs, link) {
		add_output(manager, layout_output);
	}

	wl_signal_init(&manager->events.destroy);

	manager->layout_add.notify = handle_layout_add;
	wl_signal_add(&layout->events.add, &manager->layout_add);
	manager->layout_change.notify = handle_layout_change;
	wl_signal_add(&layout->events.change, &manager->layout_change);
	manager->layout_destroy.notify = handle_layout_destroy;
	wl_signal_add(&layout->events.destroy, &manager->layout_destroy);

	manager->display_destroy.notify = handle_display_destroy;
	wl_display_add_destroy_listener(display, &manager->display_destroy);

	return manager;
}
