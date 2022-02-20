#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdlib.h>
#include <wayland-server-core.h>
#include <wlr/types/wlr_session_lock_v1.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/util/log.h>
#include "util/signal.h"
#include "ext-session-lock-v1-protocol.h"

#define SESSION_LOCK_VERSION 1

static void resource_handle_destroy(struct wl_client *client,
		struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static const struct ext_session_lock_manager_v1_interface lock_manager_implementation;
static const struct ext_session_lock_v1_interface lock_implementation;
static const struct ext_session_lock_surface_v1_interface lock_surface_implementation;

static struct wlr_session_lock_manager_v1 *lock_manager_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource,
		&ext_session_lock_manager_v1_interface, &lock_manager_implementation));
	struct wlr_session_lock_manager_v1 *lock_manager =
		wl_resource_get_user_data(resource);
	assert(lock_manager != NULL);
	return lock_manager;
}

static struct wlr_session_lock_v1 *lock_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource,
		&ext_session_lock_v1_interface, &lock_implementation));
	return wl_resource_get_user_data(resource);
}

static struct wlr_session_lock_surface_v1 *lock_surface_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource,
		&ext_session_lock_surface_v1_interface, &lock_surface_implementation));
	return wl_resource_get_user_data(resource);
}

static const struct wlr_surface_role lock_surface_role;

bool wlr_surface_is_session_lock_surface_v1(struct wlr_surface *surface) {
	return surface->role == &lock_surface_role;
}

struct wlr_session_lock_surface_v1 *wlr_session_lock_surface_v1_from_wlr_surface(
		struct wlr_surface *surface) {
	assert(wlr_surface_is_session_lock_surface_v1(surface));
	return (struct wlr_session_lock_surface_v1 *)surface->role_data;
}

uint32_t wlr_session_lock_surface_v1_configure(
		struct wlr_session_lock_surface_v1 *lock_surface,
		uint32_t width, uint32_t height) {
	struct wlr_session_lock_surface_v1_configure *configure =
		calloc(1, sizeof(struct wlr_session_lock_surface_v1_configure));
	if (configure == NULL) {
		wl_resource_post_no_memory(lock_surface->resource);
		return lock_surface->pending.configure_serial;
	}

	struct wl_display *display =
		wl_client_get_display(wl_resource_get_client(lock_surface->resource));

	configure->width = width;
	configure->height = height;
	configure->serial = wl_display_next_serial(display);

	wl_list_insert(lock_surface->configure_list.prev, &configure->link);

	ext_session_lock_surface_v1_send_configure(lock_surface->resource,
		configure->serial, configure->width, configure->height);

	return configure->serial;
}

static void lock_surface_handle_ack_configure(struct wl_client *client,
		struct wl_resource *resource, uint32_t serial) {
	struct wlr_session_lock_surface_v1 *lock_surface =
		lock_surface_from_resource(resource);
	if (lock_surface == NULL) {
		return;
	}

	// First find the ack'ed configure
	bool found = false;
	struct wlr_session_lock_surface_v1_configure *configure, *tmp;
	wl_list_for_each(configure, &lock_surface->configure_list, link) {
		if (configure->serial == serial) {
			found = true;
			break;
		}
	}
	if (!found) {
		wl_resource_post_error(resource,
			EXT_SESSION_LOCK_SURFACE_V1_ERROR_INVALID_SERIAL,
			"ack_configure serial %" PRIu32
			" does not match any configure serial", serial);
		return;
	}
	// Then remove old configures from the list
	wl_list_for_each_safe(configure, tmp, &lock_surface->configure_list, link) {
		if (configure->serial == serial) {
			break;
		}
		wl_list_remove(&configure->link);
		free(configure);
	}

	lock_surface->pending.configure_serial = configure->serial;
	lock_surface->pending.width = configure->width;
	lock_surface->pending.height = configure->height;

	lock_surface->configured = true;

	wl_list_remove(&configure->link);
	free(configure);
}


static const struct ext_session_lock_surface_v1_interface lock_surface_implementation = {
	.destroy = resource_handle_destroy,
	.ack_configure = lock_surface_handle_ack_configure,
};

static void lock_surface_role_commit(struct wlr_surface *surface) {
	struct wlr_session_lock_surface_v1 *lock_surface =
		wlr_session_lock_surface_v1_from_wlr_surface(surface);
	if (lock_surface == NULL) {
		return;
	}

	if (!lock_surface->configured) {
		wl_resource_post_error(lock_surface->resource,
			EXT_SESSION_LOCK_SURFACE_V1_ERROR_COMMIT_BEFORE_FIRST_ACK,
			"session lock surface has never been configured");
		return;
	}

	if (surface->current.width < 0 || surface->current.height < 0 ||
			(uint32_t)surface->current.width != lock_surface->pending.width ||
			(uint32_t)surface->current.height != lock_surface->pending.height) {
		wl_resource_post_error(lock_surface->resource,
			EXT_SESSION_LOCK_SURFACE_V1_ERROR_DIMENSIONS_MISMATCH,
			"committed surface dimensions do not match last acked configure");
		return;
	}

	lock_surface->current = lock_surface->pending;

	if (!lock_surface->mapped) {
		lock_surface->mapped = true;
		wlr_signal_emit_safe(&lock_surface->events.map, NULL);
	}
}

static void lock_surface_role_precommit(struct wlr_surface *surface,
		const struct wlr_surface_state *state) {
	struct wlr_session_lock_surface_v1 *lock_surface =
		wlr_session_lock_surface_v1_from_wlr_surface(surface);
	if (lock_surface == NULL) {
		return;
	}

	if (state->committed & WLR_SURFACE_STATE_BUFFER && state->buffer == NULL) {
		wl_resource_post_error(lock_surface->resource,
			EXT_SESSION_LOCK_SURFACE_V1_ERROR_NULL_BUFFER,
			"session lock surfaces committed with null buffer");
		return;
	}
}

static const struct wlr_surface_role lock_surface_role = {
	.name = "ext_session_lock_surface_v1",
	.commit = lock_surface_role_commit,
	.precommit = lock_surface_role_precommit,
};

static void lock_surface_destroy(
		struct wlr_session_lock_surface_v1 *lock_surface) {
	wlr_signal_emit_safe(&lock_surface->events.destroy, NULL);

	wl_list_remove(&lock_surface->link);

	struct wlr_session_lock_surface_v1_configure *configure, *tmp;
	wl_list_for_each_safe(configure, tmp, &lock_surface->configure_list, link) {
		wl_list_remove(&configure->link);
		free(configure);
	}

	assert(wl_list_empty(&lock_surface->events.map.listener_list));
	assert(wl_list_empty(&lock_surface->events.destroy.listener_list));

	wl_list_remove(&lock_surface->output_destroy.link);
	wl_list_remove(&lock_surface->surface_destroy.link);

	lock_surface->surface->role_data = NULL;
	wl_resource_set_user_data(lock_surface->resource, NULL);
	free(lock_surface);
}

static void lock_surface_handle_output_destroy(struct wl_listener *listener,
		void *data) {
	struct wlr_session_lock_surface_v1 *lock_surface =
		wl_container_of(listener, lock_surface, output_destroy);
	lock_surface_destroy(lock_surface);
}

static void lock_surface_handle_surface_destroy(struct wl_listener *listener,
		void *data) {
	struct wlr_session_lock_surface_v1 *lock_surface =
		wl_container_of(listener, lock_surface, surface_destroy);
	lock_surface_destroy(lock_surface);
}

static void lock_surface_resource_destroy(struct wl_resource *resource) {
	struct wlr_session_lock_surface_v1 *lock_surface =
		lock_surface_from_resource(resource);
	if (lock_surface != NULL) {
		lock_surface_destroy(lock_surface);
	}
}

static void lock_handle_get_lock_surface(struct wl_client *client,
		struct wl_resource *lock_resource, uint32_t id,
		struct wl_resource *surface_resource,
		struct wl_resource *output_resource) {
	// We always need to create a lock surface resource to stay in sync
	// with the client, even if the lock resource or output resource is
	// inert. For example, if the compositor denies the lock and immediately
	// calls wlr_session_lock_v1_destroy() the client may have already sent
	// get_lock_surface requests.
	struct wl_resource *lock_surface_resource = wl_resource_create(
		client, &ext_session_lock_surface_v1_interface,
		wl_resource_get_version(lock_resource), id);
	if (lock_surface_resource == NULL) {
		wl_client_post_no_memory(client);
		return;
	}

	// Leave the lock surface resource inert for now, we will set the
	// user data at the end of this function if everything is successful.
	wl_resource_set_implementation(lock_surface_resource,
		&lock_surface_implementation, NULL,
		lock_surface_resource_destroy);

	struct wlr_session_lock_v1 *lock = lock_from_resource(lock_resource);
	if (lock == NULL) {
		return;
	}

	struct wlr_output *output = wlr_output_from_resource(output_resource);
	if (output == NULL) {
		return;
	}

	struct wlr_session_lock_surface_v1 *other;
	wl_list_for_each(other, &lock->surfaces, link) {
		if (other->output == output) {
			wl_resource_post_error(lock_resource,
				EXT_SESSION_LOCK_V1_ERROR_DUPLICATE_OUTPUT,
				"session lock surface already created for the given output");
			return;
		}
	}

	struct wlr_surface *surface = wlr_surface_from_resource(surface_resource);

	if (wlr_surface_has_buffer(surface)) {
		wl_resource_post_error(lock_resource,
			EXT_SESSION_LOCK_V1_ERROR_ALREADY_CONSTRUCTED,
			"surface already has a buffer attached");
		return;
	}

	struct wlr_session_lock_surface_v1 *lock_surface =
		calloc(1, sizeof(struct wlr_session_lock_surface_v1));
	if (lock_surface == NULL) {
		wl_client_post_no_memory(client);
		return;
	}

	if (!wlr_surface_set_role(surface, &lock_surface_role, lock_surface,
			lock_resource, EXT_SESSION_LOCK_V1_ERROR_ROLE)) {
		free(lock_surface);
		return;
	}

	lock_surface->resource = lock_surface_resource;
	wl_resource_set_user_data(lock_surface_resource, lock_surface);

	wl_list_insert(&lock->surfaces, &lock_surface->link);

	lock_surface->output = output;
	lock_surface->surface = surface;

	wl_list_init(&lock_surface->configure_list);

	wl_signal_init(&lock_surface->events.map);
	wl_signal_init(&lock_surface->events.destroy);

	wl_signal_add(&output->events.destroy, &lock_surface->output_destroy);
	lock_surface->output_destroy.notify = lock_surface_handle_output_destroy;

	wl_signal_add(&surface->events.destroy, &lock_surface->surface_destroy);
	lock_surface->surface_destroy.notify = lock_surface_handle_surface_destroy;

	wlr_signal_emit_safe(&lock->events.new_surface, lock_surface);
}

static void lock_handle_unlock_and_destroy(struct wl_client *client,
		struct wl_resource *lock_resource) {
	struct wlr_session_lock_v1 *lock = lock_from_resource(lock_resource);
	if (lock == NULL) {
		return;
	}

	wlr_signal_emit_safe(&lock->events.unlock, NULL);

	wl_resource_destroy(lock_resource);
}

static const struct ext_session_lock_v1_interface lock_implementation = {
	.destroy = resource_handle_destroy,
	.get_lock_surface = lock_handle_get_lock_surface,
	.unlock_and_destroy = lock_handle_unlock_and_destroy,
};

void wlr_session_lock_v1_send_locked(struct wlr_session_lock_v1 *lock) {
	ext_session_lock_v1_send_locked(lock->resource);
}

static void lock_destroy(struct wlr_session_lock_v1 *lock) {
	struct wlr_session_lock_surface_v1 *lock_surface, *tmp;
	wl_list_for_each_safe(lock_surface, tmp, &lock->surfaces, link) {
		lock_surface_destroy(lock_surface);
	}
	assert(wl_list_empty(&lock->surfaces));

	wlr_signal_emit_safe(&lock->events.destroy, NULL);

	assert(wl_list_empty(&lock->events.new_surface.listener_list));
	assert(wl_list_empty(&lock->events.unlock.listener_list));
	assert(wl_list_empty(&lock->events.destroy.listener_list));

	wl_resource_set_user_data(lock->resource, NULL);
	free(lock);
}

void wlr_session_lock_v1_destroy(struct wlr_session_lock_v1 *lock) {
	ext_session_lock_v1_send_finished(lock->resource);
	lock_destroy(lock);
}

static void lock_resource_destroy(struct wl_resource *lock_resource) {
	struct wlr_session_lock_v1 *lock = lock_from_resource(lock_resource);
	if (lock != NULL) {
		lock_destroy(lock);
	}
}

static void lock_manager_handle_lock(struct wl_client *client,
		struct wl_resource *manager_resource, uint32_t id) {
	struct wlr_session_lock_manager_v1 *lock_manager =
		lock_manager_from_resource(manager_resource);

	struct wlr_session_lock_v1 *lock =
		calloc(1, sizeof(struct wlr_session_lock_v1));
	if (lock == NULL) {
		wl_client_post_no_memory(client);
		return;
	}

	lock->resource = wl_resource_create(client, &ext_session_lock_v1_interface,
		wl_resource_get_version(manager_resource), id);
	if (lock->resource == NULL) {
		free(lock);
		wl_client_post_no_memory(client);
		return;
	}

	wl_list_init(&lock->surfaces);

	wl_signal_init(&lock->events.new_surface);
	wl_signal_init(&lock->events.unlock);
	wl_signal_init(&lock->events.destroy);

	wl_resource_set_implementation(lock->resource, &lock_implementation,
		lock, lock_resource_destroy);

	wlr_signal_emit_safe(&lock_manager->events.new_lock, lock);
}

static const struct ext_session_lock_manager_v1_interface lock_manager_implementation = {
	.destroy = resource_handle_destroy,
	.lock = lock_manager_handle_lock,
};

static void lock_manager_bind(struct wl_client *client, void *data,
		uint32_t version, uint32_t id) {
	struct wlr_session_lock_manager_v1 *lock_manager = data;

	struct wl_resource *resource = wl_resource_create(
		client, &ext_session_lock_manager_v1_interface, version, id);
	if (resource == NULL) {
		wl_client_post_no_memory(client);
		return;
	}
	wl_resource_set_implementation(resource, &lock_manager_implementation,
		lock_manager, NULL);
}

static void handle_display_destroy(struct wl_listener *listener, void *data) {
	struct wlr_session_lock_manager_v1 *lock_manager =
		wl_container_of(listener, lock_manager, display_destroy);
	wlr_signal_emit_safe(&lock_manager->events.destroy, NULL);
	wl_list_remove(&lock_manager->display_destroy.link);

	wl_global_destroy(lock_manager->global);

	assert(wl_list_empty(&lock_manager->events.new_lock.listener_list));
	assert(wl_list_empty(&lock_manager->events.destroy.listener_list));

	free(lock_manager);
}

struct wlr_session_lock_manager_v1 *wlr_session_lock_manager_v1_create(struct wl_display *display) {
	struct wlr_session_lock_manager_v1 *lock_manager =
		calloc(1, sizeof(struct wlr_session_lock_manager_v1));
	if (lock_manager == NULL) {
		return NULL;
	}

	struct wl_global *global = wl_global_create(display,
		&ext_session_lock_manager_v1_interface, SESSION_LOCK_VERSION,
		lock_manager, lock_manager_bind);
	if (global == NULL) {
		free(lock_manager);
		return NULL;
	}
	lock_manager->global = global;

	wl_signal_init(&lock_manager->events.new_lock);
	wl_signal_init(&lock_manager->events.destroy);

	lock_manager->display_destroy.notify = handle_display_destroy;
	wl_display_add_destroy_listener(display, &lock_manager->display_destroy);

	return lock_manager;
}
