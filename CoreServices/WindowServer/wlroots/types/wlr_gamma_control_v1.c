#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <wayland-server-core.h>
#include <wlr/interfaces/wlr_output.h>
#include <wlr/types/wlr_gamma_control_v1.h>
#include <wlr/types/wlr_output.h>
#include <wlr/util/log.h>
#include "util/signal.h"
#include "wlr-gamma-control-unstable-v1-protocol.h"

#define GAMMA_CONTROL_MANAGER_V1_VERSION 1

static void gamma_control_handle_destroy(struct wl_client *client,
		struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static void gamma_control_destroy(struct wlr_gamma_control_v1 *gamma_control) {
	if (gamma_control == NULL) {
		return;
	}

	wlr_output_set_gamma(gamma_control->output, 0, NULL, NULL, NULL);
	// Gamma LUT will be applied on next output commit
	wlr_output_schedule_frame(gamma_control->output);

	wl_resource_set_user_data(gamma_control->resource, NULL);
	wl_list_remove(&gamma_control->output_destroy_listener.link);
	wl_list_remove(&gamma_control->output_commit_listener.link);
	wl_list_remove(&gamma_control->link);
	free(gamma_control->table);
	free(gamma_control);
}

static void gamma_control_send_failed(
		struct wlr_gamma_control_v1 *gamma_control) {
	zwlr_gamma_control_v1_send_failed(gamma_control->resource);
	gamma_control_destroy(gamma_control);
}

static void gamma_control_apply(struct wlr_gamma_control_v1 *gamma_control) {
	uint16_t *r = gamma_control->table;
	uint16_t *g = gamma_control->table + gamma_control->ramp_size;
	uint16_t *b = gamma_control->table + 2 * gamma_control->ramp_size;

	wlr_output_set_gamma(gamma_control->output, gamma_control->ramp_size, r, g, b);
	if (!wlr_output_test(gamma_control->output)) {
		wlr_output_rollback(gamma_control->output);
		gamma_control_send_failed(gamma_control);
		return;
	}

	// Gamma LUT will be applied on next output commit
	wlr_output_schedule_frame(gamma_control->output);
}

static const struct zwlr_gamma_control_v1_interface gamma_control_impl;

static struct wlr_gamma_control_v1 *gamma_control_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource, &zwlr_gamma_control_v1_interface,
		&gamma_control_impl));
	return wl_resource_get_user_data(resource);
}

static void gamma_control_handle_resource_destroy(struct wl_resource *resource) {
	struct wlr_gamma_control_v1 *gamma_control =
		gamma_control_from_resource(resource);
	gamma_control_destroy(gamma_control);
}

static void gamma_control_handle_output_destroy(struct wl_listener *listener,
		void *data) {
	struct wlr_gamma_control_v1 *gamma_control =
		wl_container_of(listener, gamma_control, output_destroy_listener);
	gamma_control_destroy(gamma_control);
}

static void gamma_control_handle_output_commit(struct wl_listener *listener,
		void *data) {
	struct wlr_gamma_control_v1 *gamma_control =
		wl_container_of(listener, gamma_control, output_commit_listener);
	struct wlr_output_event_commit *event = data;
	if ((event->committed & WLR_OUTPUT_STATE_ENABLED) &&
			gamma_control->output->enabled) {
		gamma_control_apply(gamma_control);
	}
}

static void gamma_control_handle_set_gamma(struct wl_client *client,
		struct wl_resource *gamma_control_resource, int fd) {
	struct wlr_gamma_control_v1 *gamma_control =
		gamma_control_from_resource(gamma_control_resource);
	if (gamma_control == NULL) {
		goto error_fd;
	}

	uint32_t ramp_size = wlr_output_get_gamma_size(gamma_control->output);
	size_t table_size = ramp_size * 3 * sizeof(uint16_t);

	// Refuse to block when reading
	int fd_flags = fcntl(fd, F_GETFL, 0);
	if (fd_flags == -1) {
		wlr_log_errno(WLR_ERROR, "failed to get FD flags");
		gamma_control_send_failed(gamma_control);
		goto error_fd;
	}
	if (fcntl(fd, F_SETFL, fd_flags | O_NONBLOCK) == -1) {
		wlr_log_errno(WLR_ERROR, "failed to set FD flags");
		gamma_control_send_failed(gamma_control);
		goto error_fd;
	}

	// Use the heap since gamma tables can be large
	uint16_t *table = malloc(table_size);
	if (table == NULL) {
		wl_resource_post_no_memory(gamma_control_resource);
		goto error_fd;
	}

	ssize_t n_read = read(fd, table, table_size);
	if (n_read < 0) {
		wlr_log_errno(WLR_ERROR, "failed to read gamma table");
		gamma_control_send_failed(gamma_control);
		goto error_table;
	} else if ((size_t)n_read != table_size) {
		wl_resource_post_error(gamma_control_resource,
			ZWLR_GAMMA_CONTROL_V1_ERROR_INVALID_GAMMA,
			"The gamma ramps don't have the correct size");
		goto error_table;
	}
	close(fd);
	fd = -1;

	free(gamma_control->table);
	gamma_control->table = table;
	gamma_control->ramp_size = ramp_size;

	if (gamma_control->output->enabled) {
		gamma_control_apply(gamma_control);
	}

	return;

error_table:
	free(table);
error_fd:
	close(fd);
}

static const struct zwlr_gamma_control_v1_interface gamma_control_impl = {
	.destroy = gamma_control_handle_destroy,
	.set_gamma = gamma_control_handle_set_gamma,
};

static const struct zwlr_gamma_control_manager_v1_interface
	gamma_control_manager_impl;

static struct wlr_gamma_control_manager_v1 *gamma_control_manager_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource,
		&zwlr_gamma_control_manager_v1_interface, &gamma_control_manager_impl));
	return wl_resource_get_user_data(resource);
}

static void gamma_control_manager_get_gamma_control(struct wl_client *client,
		struct wl_resource *manager_resource, uint32_t id,
		struct wl_resource *output_resource) {
	struct wlr_gamma_control_manager_v1 *manager =
		gamma_control_manager_from_resource(manager_resource);
	struct wlr_output *output = wlr_output_from_resource(output_resource);

	struct wlr_gamma_control_v1 *gamma_control =
		calloc(1, sizeof(struct wlr_gamma_control_v1));
	if (gamma_control == NULL) {
		wl_client_post_no_memory(client);
		return;
	}
	gamma_control->output = output;

	uint32_t version = wl_resource_get_version(manager_resource);
	gamma_control->resource = wl_resource_create(client,
		&zwlr_gamma_control_v1_interface, version, id);
	if (gamma_control->resource == NULL) {
		free(gamma_control);
		wl_client_post_no_memory(client);
		return;
	}
	wl_resource_set_implementation(gamma_control->resource, &gamma_control_impl,
		gamma_control, gamma_control_handle_resource_destroy);

	if (output == NULL) {
		wl_resource_set_user_data(gamma_control->resource, NULL);
		zwlr_gamma_control_v1_send_failed(gamma_control->resource);
		free(gamma_control);
		return;
	}

	wl_signal_add(&output->events.destroy,
		&gamma_control->output_destroy_listener);
	gamma_control->output_destroy_listener.notify =
		gamma_control_handle_output_destroy;

	wl_signal_add(&output->events.commit,
		&gamma_control->output_commit_listener);
	gamma_control->output_commit_listener.notify =
		gamma_control_handle_output_commit;

	wl_list_init(&gamma_control->link);

	size_t gamma_size = wlr_output_get_gamma_size(output);
	if (gamma_size == 0) {
		zwlr_gamma_control_v1_send_failed(gamma_control->resource);
		gamma_control_destroy(gamma_control);
		return;
	}

	struct wlr_gamma_control_v1 *gc;
	wl_list_for_each(gc, &manager->controls, link) {
		if (gc->output == output) {
			zwlr_gamma_control_v1_send_failed(gc->resource);
			gamma_control_destroy(gc);
			return;
		}
	}

	wl_list_remove(&gamma_control->link);
	wl_list_insert(&manager->controls, &gamma_control->link);
	zwlr_gamma_control_v1_send_gamma_size(gamma_control->resource, gamma_size);
}

static void gamma_control_manager_destroy(struct wl_client *client,
		struct wl_resource *manager_resource) {
	wl_resource_destroy(manager_resource);
}

static const struct zwlr_gamma_control_manager_v1_interface
		gamma_control_manager_impl = {
	.get_gamma_control = gamma_control_manager_get_gamma_control,
	.destroy = gamma_control_manager_destroy,
};

static void gamma_control_manager_bind(struct wl_client *client, void *data,
		uint32_t version, uint32_t id) {
	struct wlr_gamma_control_manager_v1 *manager = data;

	struct wl_resource *resource = wl_resource_create(client,
		&zwlr_gamma_control_manager_v1_interface, version, id);
	if (resource == NULL) {
		wl_client_post_no_memory(client);
		return;
	}
	wl_resource_set_implementation(resource, &gamma_control_manager_impl,
		manager, NULL);
}

static void handle_display_destroy(struct wl_listener *listener, void *data) {
	struct wlr_gamma_control_manager_v1 *manager =
		wl_container_of(listener, manager, display_destroy);
	wlr_signal_emit_safe(&manager->events.destroy, manager);
	wl_list_remove(&manager->display_destroy.link);
	wl_global_destroy(manager->global);
	free(manager);
}

struct wlr_gamma_control_manager_v1 *wlr_gamma_control_manager_v1_create(
		struct wl_display *display) {
	struct wlr_gamma_control_manager_v1 *manager =
		calloc(1, sizeof(struct wlr_gamma_control_manager_v1));
	if (!manager) {
		return NULL;
	}

	manager->global = wl_global_create(display,
		&zwlr_gamma_control_manager_v1_interface,
		GAMMA_CONTROL_MANAGER_V1_VERSION, manager, gamma_control_manager_bind);
	if (manager->global == NULL) {
		free(manager);
		return NULL;
	}

	wl_signal_init(&manager->events.destroy);
	wl_list_init(&manager->controls);

	manager->display_destroy.notify = handle_display_destroy;
	wl_display_add_destroy_listener(display, &manager->display_destroy);

	return manager;
}
