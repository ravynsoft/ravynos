#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <wayland-server-core.h>
#include <wlr/interfaces/wlr_output.h>
#include <wlr/types/wlr_output_power_management_v1.h>
#include <wlr/types/wlr_output.h>
#include <wlr/util/log.h>
#include "util/signal.h"
#include "wlr-output-power-management-unstable-v1-protocol.h"

#define OUTPUT_POWER_MANAGER_V1_VERSION 1

static void output_power_handle_destroy(struct wl_client *client,
		struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static void output_power_destroy(struct wlr_output_power_v1 *output_power) {
	if (output_power == NULL) {
		return;
	}
	wl_resource_set_user_data(output_power->resource, NULL);
	wl_list_remove(&output_power->output_destroy_listener.link);
	wl_list_remove(&output_power->output_commit_listener.link);
	wl_list_remove(&output_power->link);
	free(output_power);
}

static const struct zwlr_output_power_v1_interface output_power_impl;

static struct wlr_output_power_v1 *output_power_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource, &zwlr_output_power_v1_interface,
		&output_power_impl));
	return wl_resource_get_user_data(resource);
}

static void output_power_handle_resource_destroy(struct wl_resource *resource) {
	struct wlr_output_power_v1 *output_power =
		output_power_from_resource(resource);
	output_power_destroy(output_power);
}

static void output_power_handle_output_destroy(struct wl_listener *listener,
		void *data) {
	struct wlr_output_power_v1 *output_power =
		wl_container_of(listener, output_power, output_destroy_listener);
	output_power_destroy(output_power);
}

static void output_power_v1_send_mode(struct wlr_output_power_v1 *output_power) {
	enum zwlr_output_power_v1_mode mode;

	mode = output_power->output->enabled ? ZWLR_OUTPUT_POWER_V1_MODE_ON :
		ZWLR_OUTPUT_POWER_V1_MODE_OFF;
	zwlr_output_power_v1_send_mode(output_power->resource, mode);
}

static void output_power_handle_output_commit(struct wl_listener *listener,
		void *data) {
	struct wlr_output_power_v1 *output_power =
		wl_container_of(listener, output_power, output_commit_listener);
	struct wlr_output_event_commit *event = data;
	if (event->committed & WLR_OUTPUT_STATE_ENABLED) {
		output_power_v1_send_mode(output_power);
	}
}

static void output_power_handle_set_mode(struct wl_client *client,
		struct wl_resource *output_power_resource,
		enum zwlr_output_power_v1_mode mode) {
	struct wlr_output_power_v1 *output_power =
		output_power_from_resource(output_power_resource);
	if (output_power == NULL) {
		return;
	}

	switch (mode) {
	case ZWLR_OUTPUT_POWER_V1_MODE_OFF:
	case ZWLR_OUTPUT_POWER_V1_MODE_ON:
		break;
	default:
		wlr_log(WLR_ERROR, "Invalid power mode %d", mode);
		wl_resource_post_error(output_power_resource,
			ZWLR_OUTPUT_POWER_V1_ERROR_INVALID_MODE,
			"Invalid power mode");
		return;
	}

	struct wlr_output_power_v1_set_mode_event event = {
		.output = output_power->output,
		.mode = mode,
	};
	wlr_signal_emit_safe(&output_power->manager->events.set_mode, &event);
}

static const struct zwlr_output_power_v1_interface output_power_impl = {
	.destroy = output_power_handle_destroy,
	.set_mode = output_power_handle_set_mode,
};

static const struct zwlr_output_power_manager_v1_interface
	output_power_manager_impl;

static struct wlr_output_power_manager_v1 *output_power_manager_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource,
		&zwlr_output_power_manager_v1_interface, &output_power_manager_impl));
	return wl_resource_get_user_data(resource);
}

static void output_power_manager_get_output_power(struct wl_client *client,
		struct wl_resource *manager_resource, uint32_t id,
		struct wl_resource *output_resource) {
	struct wlr_output_power_manager_v1 *manager =
		output_power_manager_from_resource(manager_resource);
	struct wlr_output *output = wlr_output_from_resource(output_resource);

	struct wlr_output_power_v1 *output_power =
		calloc(1, sizeof(struct wlr_output_power_v1));
	if (output_power == NULL) {
		wl_client_post_no_memory(client);
		return;
	}
	output_power->output = output;
	output_power->manager = manager;
	wl_list_init(&output_power->link);

	uint32_t version = wl_resource_get_version(manager_resource);
	output_power->resource = wl_resource_create(client,
		&zwlr_output_power_v1_interface, version, id);
	if (output_power->resource == NULL) {
		free(output_power);
		wl_client_post_no_memory(client);
		return;
	}
	wl_resource_set_implementation(output_power->resource, &output_power_impl,
		output_power, output_power_handle_resource_destroy);

	if (!output) {
		wl_resource_set_user_data(output_power->resource, NULL);
		zwlr_output_power_v1_send_failed(output_power->resource);
		free(output_power);
		return;
	}

	wl_signal_add(&output->events.destroy,
		&output_power->output_destroy_listener);
	output_power->output_destroy_listener.notify =
		output_power_handle_output_destroy;
	wl_signal_add(&output->events.commit,
		&output_power->output_commit_listener);
	output_power->output_commit_listener.notify =
		output_power_handle_output_commit;

	struct wlr_output_power_v1 *mgmt;
	wl_list_for_each(mgmt, &manager->output_powers, link) {
		if (mgmt->output == output) {
			zwlr_output_power_v1_send_failed(output_power->resource);
			output_power_destroy(output_power);
			return;
		}
	}

	wl_list_insert(&manager->output_powers, &output_power->link);
	output_power_v1_send_mode(output_power);
}

static void output_power_manager_destroy(struct wl_client *client,
		struct wl_resource *manager_resource) {
	wl_resource_destroy(manager_resource);
}

static const struct zwlr_output_power_manager_v1_interface
		output_power_manager_impl = {
	.get_output_power = output_power_manager_get_output_power,
	.destroy = output_power_manager_destroy,
};

static void output_power_manager_bind(struct wl_client *client, void *data,
		uint32_t version, uint32_t id) {
	struct wlr_output_power_manager_v1 *manager = data;

	struct wl_resource *resource = wl_resource_create(client,
		&zwlr_output_power_manager_v1_interface, version, id);
	if (resource == NULL) {
		wl_client_post_no_memory(client);
		return;
	}
	wl_resource_set_implementation(resource, &output_power_manager_impl,
		manager, NULL);
}

static void handle_display_destroy(struct wl_listener *listener, void *data) {
	struct wlr_output_power_manager_v1 *manager =
		wl_container_of(listener, manager, display_destroy);
	wlr_signal_emit_safe(&manager->events.destroy, manager);
	wl_global_destroy(manager->global);
	free(manager);
}

struct wlr_output_power_manager_v1 *wlr_output_power_manager_v1_create(
		struct wl_display *display) {
	struct wlr_output_power_manager_v1 *manager =
		calloc(1, sizeof(struct wlr_output_power_manager_v1));
	if (!manager) {
		return NULL;
	}

	manager->global = wl_global_create(display,
		&zwlr_output_power_manager_v1_interface,
		OUTPUT_POWER_MANAGER_V1_VERSION, manager, output_power_manager_bind);
	if (manager->global == NULL) {
		free(manager);
		return NULL;
	}

	wl_signal_init(&manager->events.set_mode);
	wl_signal_init(&manager->events.destroy);
	wl_list_init(&manager->output_powers);

	manager->display_destroy.notify = handle_display_destroy;
	wl_display_add_destroy_listener(display, &manager->display_destroy);

	return manager;
}
