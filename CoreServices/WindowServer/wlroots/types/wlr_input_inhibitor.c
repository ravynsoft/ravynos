#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <wayland-server-core.h>
#include "wlr/types/wlr_input_inhibitor.h"
#include "wlr-input-inhibitor-unstable-v1-protocol.h"
#include "util/signal.h"

static const struct zwlr_input_inhibit_manager_v1_interface inhibit_manager_implementation;
static const struct zwlr_input_inhibitor_v1_interface input_inhibitor_implementation;

static struct wlr_input_inhibit_manager *input_inhibit_manager_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource,
			&zwlr_input_inhibit_manager_v1_interface,
			&inhibit_manager_implementation)
		|| wl_resource_instance_of(resource,
			&zwlr_input_inhibitor_v1_interface,
			&input_inhibitor_implementation));
	return wl_resource_get_user_data(resource);
}

static void input_inhibit_manager_deactivate(
		struct wlr_input_inhibit_manager *manager) {
	if (manager->active_client == NULL && manager->active_inhibitor == NULL) {
		return;
	}
	manager->active_client = NULL;
	manager->active_inhibitor = NULL;
	wlr_signal_emit_safe(&manager->events.deactivate, manager);
}

static void input_inhibitor_destroy(struct wl_client *client,
			struct wl_resource *resource) {
	struct wlr_input_inhibit_manager *manager =
		input_inhibit_manager_from_resource(resource);
	input_inhibit_manager_deactivate(manager);
	wl_resource_destroy(resource);
}

static void input_inhibitor_resource_destroy(struct wl_resource *resource) {
	struct wlr_input_inhibit_manager *manager =
		input_inhibit_manager_from_resource(resource);
	input_inhibit_manager_deactivate(manager);
}

static const struct zwlr_input_inhibitor_v1_interface input_inhibitor_implementation = {
	.destroy = input_inhibitor_destroy,
};

static void inhibit_manager_get_inhibitor(struct wl_client *client,
		struct wl_resource *resource, uint32_t id) {
	struct wlr_input_inhibit_manager *manager =
		input_inhibit_manager_from_resource(resource);
	if (manager->active_client || manager->active_inhibitor) {
		wl_resource_post_error(resource,
			ZWLR_INPUT_INHIBIT_MANAGER_V1_ERROR_ALREADY_INHIBITED,
			"this compositor already has input inhibited");
		return;
	}

	struct wl_resource *wl_resource = wl_resource_create(client,
			&zwlr_input_inhibitor_v1_interface,
			wl_resource_get_version(resource), id);
	if (!wl_resource) {
		wl_client_post_no_memory(client);
	}
	wl_resource_set_implementation(wl_resource, &input_inhibitor_implementation,
			manager, input_inhibitor_resource_destroy);

	manager->active_client = client;
	manager->active_inhibitor = wl_resource;

	wlr_signal_emit_safe(&manager->events.activate, manager);
}

static const struct zwlr_input_inhibit_manager_v1_interface inhibit_manager_implementation = {
	.get_inhibitor = inhibit_manager_get_inhibitor
};

static void input_manager_resource_destroy(struct wl_resource *resource) {
	struct wlr_input_inhibit_manager *manager =
		input_inhibit_manager_from_resource(resource);
	struct wl_client *client = wl_resource_get_client(resource);
	if (manager->active_client == client) {
		input_inhibit_manager_deactivate(manager);
	}
}

static void inhibit_manager_bind(struct wl_client *wl_client, void *data,
		uint32_t version, uint32_t id) {
	struct wlr_input_inhibit_manager *manager = data;
	assert(wl_client && manager);

	struct wl_resource *wl_resource = wl_resource_create(wl_client,
		&zwlr_input_inhibit_manager_v1_interface, version, id);
	if (wl_resource == NULL) {
		wl_client_post_no_memory(wl_client);
		return;
	}
	wl_resource_set_implementation(wl_resource,
			&inhibit_manager_implementation, manager,
			input_manager_resource_destroy);
}

static void handle_display_destroy(struct wl_listener *listener, void *data) {
	struct wlr_input_inhibit_manager *manager =
		wl_container_of(listener, manager, display_destroy);
	wlr_signal_emit_safe(&manager->events.destroy, manager);
	wl_list_remove(&manager->display_destroy.link);
	wl_global_destroy(manager->global);
	free(manager);
}

struct wlr_input_inhibit_manager *wlr_input_inhibit_manager_create(
		struct wl_display *display) {
	// TODO: Client destroy
	struct wlr_input_inhibit_manager *manager =
		calloc(1, sizeof(struct wlr_input_inhibit_manager));
	if (!manager) {
		return NULL;
	}

	manager->global = wl_global_create(display,
			&zwlr_input_inhibit_manager_v1_interface,
			1, manager, inhibit_manager_bind);
	if (manager->global == NULL){
		free(manager);
		return NULL;
	}

	wl_signal_init(&manager->events.activate);
	wl_signal_init(&manager->events.deactivate);
	wl_signal_init(&manager->events.destroy);

	manager->display_destroy.notify = handle_display_destroy;
	wl_display_add_destroy_listener(display, &manager->display_destroy);

	return manager;
}
