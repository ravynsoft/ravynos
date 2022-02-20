#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <wlr/types/wlr_output_management_v1.h>
#include <wlr/util/log.h>
#include "util/signal.h"
#include "wlr-output-management-unstable-v1-protocol.h"

#define OUTPUT_MANAGER_VERSION 2

enum {
	HEAD_STATE_ENABLED = 1 << 0,
	HEAD_STATE_MODE = 1 << 1,
	HEAD_STATE_POSITION = 1 << 2,
	HEAD_STATE_TRANSFORM = 1 << 3,
	HEAD_STATE_SCALE = 1 << 4,
};

static const uint32_t HEAD_STATE_ALL = HEAD_STATE_ENABLED | HEAD_STATE_MODE |
	HEAD_STATE_POSITION | HEAD_STATE_TRANSFORM | HEAD_STATE_SCALE;


// Can return NULL if the head is inert
static struct wlr_output_head_v1 *head_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource,
		&zwlr_output_head_v1_interface, NULL));
	return wl_resource_get_user_data(resource);
}

static struct wlr_output_mode *mode_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource,
		&zwlr_output_mode_v1_interface, NULL));
	return wl_resource_get_user_data(resource);
}

static void head_destroy(struct wlr_output_head_v1 *head) {
	if (head == NULL) {
		return;
	}
	struct wl_resource *resource, *tmp;
	wl_resource_for_each_safe(resource, tmp, &head->mode_resources) {
		zwlr_output_mode_v1_send_finished(resource);
		wl_resource_destroy(resource);
	}
	wl_resource_for_each_safe(resource, tmp, &head->resources) {
		zwlr_output_head_v1_send_finished(resource);
		wl_resource_destroy(resource);
	}
	wl_list_remove(&head->link);
	wl_list_remove(&head->output_destroy.link);
	free(head);
}

static void head_handle_output_destroy(struct wl_listener *listener,
		void *data) {
	struct wlr_output_head_v1 *head =
		wl_container_of(listener, head, output_destroy);
	head->manager->current_configuration_dirty = true;
	head_destroy(head);
}

static struct wlr_output_head_v1 *head_create(
		struct wlr_output_manager_v1 *manager, struct wlr_output *output) {
	struct wlr_output_head_v1 *head = calloc(1, sizeof(*head));
	if (head == NULL) {
		return NULL;
	}
	head->manager = manager;
	head->state.output = output;
	wl_list_init(&head->resources);
	wl_list_init(&head->mode_resources);
	wl_list_insert(&manager->heads, &head->link);
	head->output_destroy.notify = head_handle_output_destroy;
	wl_signal_add(&output->events.destroy, &head->output_destroy);
	return head;
}


static void config_head_destroy(
		struct wlr_output_configuration_head_v1 *config_head) {
	if (config_head == NULL) {
		return;
	}
	if (config_head->resource != NULL) {
		wl_resource_set_user_data(config_head->resource, NULL); // make inert
	}
	wl_list_remove(&config_head->link);
	wl_list_remove(&config_head->output_destroy.link);
	free(config_head);
}

static void config_head_handle_output_destroy(struct wl_listener *listener,
		void *data) {
	struct wlr_output_configuration_head_v1 *config_head =
		wl_container_of(listener, config_head, output_destroy);
	config_head_destroy(config_head);
}

static struct wlr_output_configuration_head_v1 *config_head_create(
		struct wlr_output_configuration_v1 *config, struct wlr_output *output) {
	struct wlr_output_configuration_head_v1 *config_head =
		calloc(1, sizeof(*config_head));
	if (config_head == NULL) {
		return NULL;
	}
	config_head->config = config;
	config_head->state.output = output;
	wl_list_insert(&config->heads, &config_head->link);
	config_head->output_destroy.notify = config_head_handle_output_destroy;
	wl_signal_add(&output->events.destroy, &config_head->output_destroy);
	return config_head;
}

struct wlr_output_configuration_head_v1 *
		wlr_output_configuration_head_v1_create(
		struct wlr_output_configuration_v1 *config, struct wlr_output *output) {
	struct wlr_output_configuration_head_v1 *config_head =
		config_head_create(config, output);
	if (config_head == NULL) {
		return NULL;
	}
	config_head->state.enabled = output->enabled;
	config_head->state.mode = output->current_mode;
	config_head->state.custom_mode.width = output->width;
	config_head->state.custom_mode.height = output->height;
	config_head->state.custom_mode.refresh = output->refresh;
	config_head->state.transform = output->transform;
	config_head->state.scale = output->scale;
	return config_head;
}

static const struct zwlr_output_configuration_head_v1_interface config_head_impl;

// Can return NULL if the configuration head is inert
static struct wlr_output_configuration_head_v1 *config_head_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource,
		&zwlr_output_configuration_head_v1_interface, &config_head_impl));
	return wl_resource_get_user_data(resource);
}

static void config_head_handle_set_mode(struct wl_client *client,
		struct wl_resource *config_head_resource,
		struct wl_resource *mode_resource) {
	struct wlr_output_configuration_head_v1 *config_head =
		config_head_from_resource(config_head_resource);
	if (config_head == NULL) {
		return;
	}

	// Mode can be NULL if the output doesn't support modes (in which case we
	// expose only one "virtual" mode, the current mode)
	struct wlr_output_mode *mode = mode_from_resource(mode_resource);
	struct wlr_output *output = config_head->state.output;

	bool found = (mode == NULL && wl_list_empty(&output->modes));
	struct wlr_output_mode *m;
	wl_list_for_each(m, &output->modes, link) {
		if (mode == m) {
			found = true;
			break;
		}
	}

	if (!found) {
		wl_resource_post_error(config_head_resource,
			ZWLR_OUTPUT_CONFIGURATION_HEAD_V1_ERROR_INVALID_MODE,
			"mode doesn't belong to head");
		return;
	}

	config_head->state.mode = mode;
	if (mode != NULL) {
		config_head->state.custom_mode.width = 0;
		config_head->state.custom_mode.height = 0;
		config_head->state.custom_mode.refresh = 0;
	}
}

static void config_head_handle_set_custom_mode(struct wl_client *client,
		struct wl_resource *config_head_resource, int32_t width, int32_t height,
		int32_t refresh) {
	struct wlr_output_configuration_head_v1 *config_head =
		config_head_from_resource(config_head_resource);
	if (config_head == NULL) {
		return;
	}

	if (width <= 0 || height <= 0 || refresh < 0) {
		wl_resource_post_error(config_head_resource,
			ZWLR_OUTPUT_CONFIGURATION_HEAD_V1_ERROR_INVALID_CUSTOM_MODE,
			"invalid custom mode");
		return;
	}

	config_head->state.mode = NULL;
	config_head->state.custom_mode.width = width;
	config_head->state.custom_mode.height = height;
	config_head->state.custom_mode.refresh = refresh;
}

static void config_head_handle_set_position(struct wl_client *client,
		struct wl_resource *config_head_resource, int32_t x, int32_t y) {
	struct wlr_output_configuration_head_v1 *config_head =
		config_head_from_resource(config_head_resource);
	if (config_head == NULL) {
		return;
	}

	config_head->state.x = x;
	config_head->state.y = y;
}

static void config_head_handle_set_transform(struct wl_client *client,
		struct wl_resource *config_head_resource, int32_t transform) {
	struct wlr_output_configuration_head_v1 *config_head =
		config_head_from_resource(config_head_resource);
	if (config_head == NULL) {
		return;
	}

	if (transform < WL_OUTPUT_TRANSFORM_NORMAL ||
			transform > WL_OUTPUT_TRANSFORM_FLIPPED_270) {
		wl_resource_post_error(config_head_resource,
			ZWLR_OUTPUT_CONFIGURATION_HEAD_V1_ERROR_INVALID_TRANSFORM,
			"invalid transform");
		return;
	}

	config_head->state.transform = transform;
}

static void config_head_handle_set_scale(struct wl_client *client,
		struct wl_resource *config_head_resource, wl_fixed_t scale_fixed) {
	struct wlr_output_configuration_head_v1 *config_head =
		config_head_from_resource(config_head_resource);
	if (config_head == NULL) {
		return;
	}

	float scale =  wl_fixed_to_double(scale_fixed);
	if (scale <= 0) {
		wl_resource_post_error(config_head_resource,
			ZWLR_OUTPUT_CONFIGURATION_HEAD_V1_ERROR_INVALID_SCALE,
			"invalid scale");
		return;
	}

	config_head->state.scale = scale;
}

static const struct zwlr_output_configuration_head_v1_interface config_head_impl = {
	.set_mode = config_head_handle_set_mode,
	.set_custom_mode = config_head_handle_set_custom_mode,
	.set_position = config_head_handle_set_position,
	.set_transform = config_head_handle_set_transform,
	.set_scale = config_head_handle_set_scale,
};

static void config_head_handle_resource_destroy(struct wl_resource *resource) {
	struct wlr_output_configuration_head_v1 *config_head =
		config_head_from_resource(resource);
	config_head_destroy(config_head);
}


static const struct zwlr_output_configuration_v1_interface config_impl;

// Can return NULL if the config has been used
static struct wlr_output_configuration_v1 *config_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource,
		&zwlr_output_configuration_v1_interface, &config_impl));
	return wl_resource_get_user_data(resource);
}

// Checks that the head is unconfigured (ie. no enable_head/disable_head request
// has yet been sent for this head), if not sends a protocol error.
static bool config_check_head_is_unconfigured(
		struct wlr_output_configuration_v1 *config, struct wlr_output *output) {
	struct wlr_output_configuration_head_v1 *head;
	wl_list_for_each(head, &config->heads, link) {
		if (head->state.output == output) {
			wl_resource_post_error(config->resource,
				ZWLR_OUTPUT_CONFIGURATION_V1_ERROR_ALREADY_CONFIGURED_HEAD,
				"head has already been configured");
			return false;
		}
	}
	return true;
}

static void config_handle_enable_head(struct wl_client *client,
		struct wl_resource *config_resource, uint32_t id,
		struct wl_resource *head_resource) {
	struct wlr_output_configuration_v1 *config =
		config_from_resource(config_resource);
	if (config == NULL || config->finalized) {
		wl_resource_post_error(config_resource,
			ZWLR_OUTPUT_CONFIGURATION_V1_ERROR_ALREADY_USED,
			"configuration object has already been used");
		return;
	}
	struct wlr_output_head_v1 *head = head_from_resource(head_resource);

	// Create an inert resource if the head no longer exists
	struct wlr_output_configuration_head_v1 *config_head = NULL;
	if (head != NULL) {
		if (!config_check_head_is_unconfigured(config, head->state.output)) {
			return;
		}
		config_head = config_head_create(config, head->state.output);
		if (config_head == NULL) {
			wl_resource_post_no_memory(config_resource);
			return;
		}
		config_head->state = head->state;
	}

	uint32_t version = wl_resource_get_version(config_resource);
	struct wl_resource *resource = wl_resource_create(client,
		&zwlr_output_configuration_head_v1_interface, version, id);
	if (resource == NULL) {
		wl_client_post_no_memory(client);
		return;
	}
	wl_resource_set_implementation(resource, &config_head_impl,
		config_head, config_head_handle_resource_destroy);

	if (config_head != NULL) {
		config_head->resource = resource;
		config_head->state.enabled = true;
	}
}

static void config_handle_disable_head(struct wl_client *client,
		struct wl_resource *config_resource,
		struct wl_resource *head_resource) {
	struct wlr_output_configuration_v1 *config =
		config_from_resource(config_resource);
	if (config == NULL || config->finalized) {
		wl_resource_post_error(config_resource,
			ZWLR_OUTPUT_CONFIGURATION_V1_ERROR_ALREADY_USED,
			"configuration object has already been used");
		return;
	}
	struct wlr_output_head_v1 *head = head_from_resource(head_resource);
	if (head == NULL) {
		return;
	}

	if (!config_check_head_is_unconfigured(config, head->state.output)) {
		return;
	}
	struct wlr_output_configuration_head_v1 *config_head =
		config_head_create(config, head->state.output);
	if (config_head == NULL) {
		wl_resource_post_no_memory(config_resource);
		return;
	}

	config_head->state.enabled = false;
}

// Finalizes a configuration. This prevents the same config from being used
// multiple times.
static void config_finalize(struct wlr_output_configuration_v1 *config) {
	if (config->finalized) {
		return;
	}

	// Destroy config head resources now, the client is forbidden to use them at
	// this point anyway
	struct wlr_output_configuration_head_v1 *config_head, *tmp;
	wl_list_for_each_safe(config_head, tmp, &config->heads, link) {
		// Resource is NULL if head has been disabled
		if (config_head->resource != NULL) {
			wl_resource_set_user_data(config_head->resource, NULL);
			wl_resource_destroy(config_head->resource);
			config_head->resource = NULL;
		}
	}

	config->finalized = true;
}

// Destroys the config if serial is invalid
static bool config_validate_serial(struct wlr_output_configuration_v1 *config) {
	if (config->serial != config->manager->serial) {
		wlr_log(WLR_DEBUG, "Ignored configuration request: invalid serial");
		zwlr_output_configuration_v1_send_cancelled(config->resource);
		wlr_output_configuration_v1_destroy(config);
		return false;
	}
	return true;
}

static void config_handle_apply(struct wl_client *client,
		struct wl_resource *config_resource) {
	struct wlr_output_configuration_v1 *config =
		config_from_resource(config_resource);
	if (config == NULL || config->finalized) {
		wl_resource_post_error(config_resource,
			ZWLR_OUTPUT_CONFIGURATION_V1_ERROR_ALREADY_USED,
			"configuration object has already been used");
		return;
	}

	config_finalize(config);
	if (!config_validate_serial(config)) {
		return;
	}

	wlr_signal_emit_safe(&config->manager->events.apply, config);
}

static void config_handle_test(struct wl_client *client,
		struct wl_resource *config_resource) {
	struct wlr_output_configuration_v1 *config =
		config_from_resource(config_resource);
	if (config == NULL || config->finalized) {
		wl_resource_post_error(config_resource,
			ZWLR_OUTPUT_CONFIGURATION_V1_ERROR_ALREADY_USED,
			"configuration object has already been used");
		return;
	}

	config_finalize(config);
	if (!config_validate_serial(config)) {
		return;
	}

	wlr_signal_emit_safe(&config->manager->events.test, config);
}

static void config_handle_destroy(struct wl_client *client,
		struct wl_resource *config_resource) {
	wl_resource_destroy(config_resource);
}

static const struct zwlr_output_configuration_v1_interface config_impl = {
	.enable_head = config_handle_enable_head,
	.disable_head = config_handle_disable_head,
	.apply = config_handle_apply,
	.test = config_handle_test,
	.destroy = config_handle_destroy,
};

static struct wlr_output_configuration_v1 *config_create(bool finalized) {
	struct wlr_output_configuration_v1 *config = calloc(1, sizeof(*config));
	if (config == NULL) {
		return NULL;
	}
	wl_list_init(&config->heads);
	config->finalized = finalized;
	return config;
}

struct wlr_output_configuration_v1 *wlr_output_configuration_v1_create(void) {
	return config_create(true);
}

void wlr_output_configuration_v1_destroy(
		struct wlr_output_configuration_v1 *config) {
	if (config == NULL) {
		return;
	}
	config_finalize(config);
	if (config->resource != NULL) {
		wl_resource_set_user_data(config->resource, NULL); // make inert
	}
	struct wlr_output_configuration_head_v1 *config_head, *tmp;
	wl_list_for_each_safe(config_head, tmp, &config->heads, link) {
		config_head_destroy(config_head);
	}
	free(config);
}

static void config_handle_resource_destroy(struct wl_resource *resource) {
	struct wlr_output_configuration_v1 *config = config_from_resource(resource);
	if (config == NULL) {
		return;
	}
	if (config->finalized) {
		config->resource = NULL; // we no longer own the config
	} else {
		wlr_output_configuration_v1_destroy(config);
	}
}

void wlr_output_configuration_v1_send_succeeded(
		struct wlr_output_configuration_v1 *config) {
	assert(!config->finished);
	if (config->resource == NULL) {
		return; // client destroyed the resource early
	}
	zwlr_output_configuration_v1_send_succeeded(config->resource);
	config->finished = true;
}

void wlr_output_configuration_v1_send_failed(
		struct wlr_output_configuration_v1 *config) {
	assert(!config->finished);
	if (config->resource == NULL) {
		return; // client destroyed the resource early
	}
	zwlr_output_configuration_v1_send_failed(config->resource);
	config->finished = true;
}


static const struct zwlr_output_manager_v1_interface manager_impl;

static struct wlr_output_manager_v1 *manager_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource,
		&zwlr_output_manager_v1_interface, &manager_impl));
	return wl_resource_get_user_data(resource);
}

static void manager_handle_create_configuration(struct wl_client *client,
		struct wl_resource *manager_resource, uint32_t id, uint32_t serial) {
	struct wlr_output_manager_v1 *manager =
		manager_from_resource(manager_resource);

	struct wlr_output_configuration_v1 *config = config_create(false);
	if (config == NULL) {
		wl_resource_post_no_memory(manager_resource);
		return;
	}
	config->manager = manager;
	config->serial = serial;

	uint32_t version = wl_resource_get_version(manager_resource);
	config->resource = wl_resource_create(client,
		&zwlr_output_configuration_v1_interface, version, id);
	if (config->resource == NULL) {
		wl_client_post_no_memory(client);
		return;
	}
	wl_resource_set_implementation(config->resource, &config_impl,
		config, config_handle_resource_destroy);
}

static void manager_handle_stop(struct wl_client *client,
		struct wl_resource *manager_resource) {
	zwlr_output_manager_v1_send_finished(manager_resource);
	wl_resource_destroy(manager_resource);
}

static const struct zwlr_output_manager_v1_interface manager_impl = {
	.create_configuration = manager_handle_create_configuration,
	.stop = manager_handle_stop,
};

static void manager_handle_resource_destroy(struct wl_resource *resource) {
	wl_list_remove(wl_resource_get_link(resource));
}

static void manager_send_head(struct wlr_output_manager_v1 *manager,
	struct wlr_output_head_v1 *head, struct wl_resource *manager_resource);

static void manager_bind(struct wl_client *client, void *data, uint32_t version,
		uint32_t id) {
	struct wlr_output_manager_v1 *manager = data;

	struct wl_resource *resource = wl_resource_create(client,
		&zwlr_output_manager_v1_interface, version, id);
	if (resource == NULL) {
		wl_client_post_no_memory(client);
		return;
	}
	wl_resource_set_implementation(resource, &manager_impl, manager,
		manager_handle_resource_destroy);

	wl_list_insert(&manager->resources, wl_resource_get_link(resource));

	struct wlr_output_head_v1 *head;
	wl_list_for_each(head, &manager->heads, link) {
		manager_send_head(manager, head, resource);
	}

	zwlr_output_manager_v1_send_done(resource, manager->serial);
}

static void manager_handle_display_destroy(struct wl_listener *listener,
		void *data) {
	struct wlr_output_manager_v1 *manager =
		wl_container_of(listener, manager, display_destroy);
	wlr_signal_emit_safe(&manager->events.destroy, manager);
	wl_list_remove(&manager->display_destroy.link);
	struct wlr_output_head_v1 *head, *tmp;
	wl_list_for_each_safe(head, tmp, &manager->heads, link) {
		head_destroy(head);
	}
	wl_global_destroy(manager->global);
	free(manager);
}

struct wlr_output_manager_v1 *wlr_output_manager_v1_create(
		struct wl_display *display) {
	struct wlr_output_manager_v1 *manager = calloc(1, sizeof(*manager));
	if (manager == NULL) {
		return NULL;
	}
	manager->display = display;

	wl_list_init(&manager->resources);
	wl_list_init(&manager->heads);
	wl_signal_init(&manager->events.destroy);
	wl_signal_init(&manager->events.apply);
	wl_signal_init(&manager->events.test);

	manager->global = wl_global_create(display,
		&zwlr_output_manager_v1_interface, OUTPUT_MANAGER_VERSION,
		manager, manager_bind);
	if (manager->global == NULL) {
		free(manager);
		return NULL;
	}

	manager->display_destroy.notify = manager_handle_display_destroy;
	wl_display_add_destroy_listener(display, &manager->display_destroy);

	return manager;
}

static struct wlr_output_configuration_head_v1 *configuration_get_head(
		struct wlr_output_configuration_v1 *config, struct wlr_output *output) {
	struct wlr_output_configuration_head_v1 *head;
	wl_list_for_each(head, &config->heads, link) {
		if (head->state.output == output) {
			return head;
		}
	}
	return NULL;
}

static void send_mode_state(struct wl_resource *mode_resource,
		struct wlr_output_mode *mode) {
	zwlr_output_mode_v1_send_size(mode_resource, mode->width, mode->height);
	if (mode->refresh > 0) {
		zwlr_output_mode_v1_send_refresh(mode_resource, mode->refresh);
	}
	if (mode->preferred) {
		zwlr_output_mode_v1_send_preferred(mode_resource);
	}
}

static void mode_handle_resource_destroy(struct wl_resource *resource) {
	wl_list_remove(wl_resource_get_link(resource));
}

static struct wl_resource *head_send_mode(struct wlr_output_head_v1 *head,
		struct wl_resource *head_resource, struct wlr_output_mode *mode) {
	struct wl_client *client = wl_resource_get_client(head_resource);
	uint32_t version = wl_resource_get_version(head_resource);
	struct wl_resource *mode_resource =
		wl_resource_create(client, &zwlr_output_mode_v1_interface, version, 0);
	if (mode_resource == NULL) {
		wl_resource_post_no_memory(head_resource);
		return NULL;
	}
	wl_resource_set_implementation(mode_resource, NULL, mode,
		mode_handle_resource_destroy);
	wl_list_insert(&head->mode_resources, wl_resource_get_link(mode_resource));

	zwlr_output_head_v1_send_mode(head_resource, mode_resource);

	if (mode != NULL) {
		send_mode_state(mode_resource, mode);
	}

	return mode_resource;
}

// Sends new head state to a client.
static void head_send_state(struct wlr_output_head_v1 *head,
		struct wl_resource *head_resource, uint32_t state) {
	struct wl_client *client = wl_resource_get_client(head_resource);

	if (state & HEAD_STATE_ENABLED) {
		zwlr_output_head_v1_send_enabled(head_resource, head->state.enabled);
		// On enabling we send all current data since clients have not been
		// notified about potential data changes while the head was disabled.
		state = HEAD_STATE_ALL;
	}

	if (!head->state.enabled) {
		return;
	}

	if (state & HEAD_STATE_MODE) {
		assert(head->state.mode != NULL ||
			wl_list_empty(&head->state.output->modes));

		bool found = false;
		struct wl_resource *mode_resource;
		wl_resource_for_each(mode_resource, &head->mode_resources) {
			if (wl_resource_get_client(mode_resource) == client &&
					mode_from_resource(mode_resource) == head->state.mode) {
				found = true;
				break;
			}
		}
		assert(found);

		if (head->state.mode == NULL) {
			// Fake a single output mode if output doesn't support modes
			struct wlr_output_mode virtual_mode = {
				.width = head->state.custom_mode.width,
				.height = head->state.custom_mode.height,
				.refresh = head->state.custom_mode.refresh,
			};
			send_mode_state(mode_resource, &virtual_mode);
		}

		zwlr_output_head_v1_send_current_mode(head_resource, mode_resource);
	}

	if (state & HEAD_STATE_POSITION) {
		zwlr_output_head_v1_send_position(head_resource,
			head->state.x, head->state.y);
	}

	if (state & HEAD_STATE_TRANSFORM) {
		zwlr_output_head_v1_send_transform(head_resource,
			head->state.transform);
	}

	if (state & HEAD_STATE_SCALE) {
		zwlr_output_head_v1_send_scale(head_resource,
			wl_fixed_from_double(head->state.scale));
	}
}

static void head_handle_resource_destroy(struct wl_resource *resource) {
	wl_list_remove(wl_resource_get_link(resource));
}

static void manager_send_head(struct wlr_output_manager_v1 *manager,
		struct wlr_output_head_v1 *head, struct wl_resource *manager_resource) {
	struct wlr_output *output = head->state.output;

	struct wl_client *client = wl_resource_get_client(manager_resource);
	uint32_t version = wl_resource_get_version(manager_resource);
	struct wl_resource *head_resource = wl_resource_create(client,
		&zwlr_output_head_v1_interface, version, 0);
	if (head_resource == NULL) {
		wl_resource_post_no_memory(manager_resource);
		return;
	}
	wl_resource_set_implementation(head_resource, NULL, head,
		head_handle_resource_destroy);
	wl_list_insert(&head->resources, wl_resource_get_link(head_resource));

	zwlr_output_manager_v1_send_head(manager_resource, head_resource);

	zwlr_output_head_v1_send_name(head_resource, output->name);
	zwlr_output_head_v1_send_description(head_resource,
		output->description ? output->description : "Unknown");

	if (output->phys_width > 0 && output->phys_height > 0) {
		zwlr_output_head_v1_send_physical_size(head_resource,
			output->phys_width, output->phys_height);
	}

	if (version >= ZWLR_OUTPUT_HEAD_V1_MAKE_SINCE_VERSION && output->make[0] != '\0') {
		zwlr_output_head_v1_send_make(head_resource, output->make);
	}
	if (version >= ZWLR_OUTPUT_HEAD_V1_MODEL_SINCE_VERSION && output->model[0] != '\0') {
		zwlr_output_head_v1_send_model(head_resource, output->model);
	}
	if (version >= ZWLR_OUTPUT_HEAD_V1_SERIAL_NUMBER_SINCE_VERSION && output->serial[0] != '\0') {
		zwlr_output_head_v1_send_serial_number(head_resource, output->serial);
	}

	struct wlr_output_mode *mode;
	wl_list_for_each(mode, &output->modes, link) {
		head_send_mode(head, head_resource, mode);
	}

	if (wl_list_empty(&output->modes)) {
		// Output doesn't support modes. Send a virtual one.
		head_send_mode(head, head_resource, NULL);
	}

	head_send_state(head, head_resource, HEAD_STATE_ALL);
}

// Compute state that has changed and sends it to all clients. Then writes the
// new state to the head.
static bool manager_update_head(struct wlr_output_manager_v1 *manager,
		struct wlr_output_head_v1 *head,
		struct wlr_output_head_v1_state *next) {
	struct wlr_output_head_v1_state *current = &head->state;

	uint32_t state = 0;
	if (current->enabled != next->enabled) {
		state |= HEAD_STATE_ENABLED;
	}
	if (current->mode != next->mode) {
		state |= HEAD_STATE_MODE;
	}
	if (current->custom_mode.width != next->custom_mode.width ||
			current->custom_mode.height != next->custom_mode.height ||
			current->custom_mode.refresh != next->custom_mode.refresh) {
		state |= HEAD_STATE_MODE;
	}
	if (current->x != next->x || current->y != next->y) {
		state |= HEAD_STATE_POSITION;
	}
	if (current->transform != next->transform) {
		state |= HEAD_STATE_TRANSFORM;
	}
	if (current->scale != next->scale) {
		state |= HEAD_STATE_SCALE;
	}

	// If  a mode was added to wlr_output.modes we need to add the new mode
	// to the wlr_output_head
	struct wlr_output_mode *mode;
	wl_list_for_each(mode, &head->state.output->modes, link) {
		bool found = false;
		struct wl_resource *mode_resource;
		wl_resource_for_each(mode_resource, &head->mode_resources) {
			if (mode_from_resource(mode_resource) == mode) {
				found = true;
				break;
			}
		}
		if (!found) {
			struct wl_resource *resource;
			wl_resource_for_each(resource, &head->resources) {
				head_send_mode(head, resource, mode);
			}
		}
	}

	if (state != 0) {
		*current = *next;

		struct wl_resource *resource;
		wl_resource_for_each(resource, &head->resources) {
			head_send_state(head, resource, state);
		}
	}

	return state != 0;
}

void wlr_output_manager_v1_set_configuration(
		struct wlr_output_manager_v1 *manager,
		struct wlr_output_configuration_v1 *config) {
	bool changed = manager->current_configuration_dirty;

	// Either update or destroy existing heads
	struct wlr_output_head_v1 *existing_head, *head_tmp;
	wl_list_for_each_safe(existing_head, head_tmp, &manager->heads, link) {
		struct wlr_output_configuration_head_v1 *updated_head =
			configuration_get_head(config, existing_head->state.output);
		if (updated_head != NULL) {
			changed |= manager_update_head(manager,
				existing_head, &updated_head->state);
			config_head_destroy(updated_head);
		} else {
			head_destroy(existing_head);
			changed = true;
		}
	}

	// Heads remaining in `config` are new heads

	// Move new heads to current config
	struct wlr_output_configuration_head_v1 *config_head, *config_head_tmp;
	wl_list_for_each_safe(config_head, config_head_tmp, &config->heads, link) {
		struct wlr_output_head_v1 *head =
			head_create(manager, config_head->state.output);
		if (head == NULL) {
			wlr_log_errno(WLR_ERROR, "Allocation failed");
			continue;
		}

		head->state = config_head->state;

		struct wl_resource *manager_resource;
		wl_resource_for_each(manager_resource, &manager->resources) {
			manager_send_head(manager, head, manager_resource);
		}

		changed = true;
	}

	wlr_output_configuration_v1_destroy(config);

	if (!changed) {
		return;
	}

	manager->serial = wl_display_next_serial(manager->display);
	struct wl_resource *manager_resource;
	wl_resource_for_each(manager_resource, &manager->resources) {
		zwlr_output_manager_v1_send_done(manager_resource,
			manager->serial);
	}
	manager->current_configuration_dirty = false;
}
