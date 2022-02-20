#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <wlr/backend/drm.h>
#include <wlr/backend/multi.h>
#include <wlr/types/wlr_drm_lease_v1.h>
#include <wlr/util/log.h>
#include "backend/drm/drm.h"
#include "drm-lease-v1-protocol.h"
#include "util/signal.h"
#include "util/global.h"

#define DRM_LEASE_DEVICE_V1_VERSION 1

static struct wp_drm_lease_device_v1_interface lease_device_impl;
static struct wp_drm_lease_connector_v1_interface lease_connector_impl;
static struct wp_drm_lease_request_v1_interface lease_request_impl;
static struct wp_drm_lease_v1_interface lease_impl;

static struct wlr_drm_lease_device_v1 *drm_lease_device_v1_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource,
				&wp_drm_lease_device_v1_interface, &lease_device_impl));
	return wl_resource_get_user_data(resource);
}

static struct wlr_drm_lease_connector_v1 *
drm_lease_connector_v1_from_resource(struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource,
				&wp_drm_lease_connector_v1_interface, &lease_connector_impl));
	return wl_resource_get_user_data(resource);
}

static struct wlr_drm_lease_request_v1 *drm_lease_request_v1_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource,
				&wp_drm_lease_request_v1_interface, &lease_request_impl));
	return wl_resource_get_user_data(resource);
}

static struct wlr_drm_lease_v1 *drm_lease_v1_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource,
				&wp_drm_lease_v1_interface, &lease_impl));
	return wl_resource_get_user_data(resource);
}

static void drm_lease_request_v1_destroy(
		struct wlr_drm_lease_request_v1 *request) {
	if (!request) {
		return;
	}

	wlr_log(WLR_DEBUG, "Destroying request %p", request);

	wl_list_remove(&request->link);
	wl_resource_set_user_data(request->resource, NULL);

	free(request->connectors);
	free(request);
}

static void drm_lease_connector_v1_destroy(
		struct wlr_drm_lease_connector_v1 *connector) {
	if (!connector) {
		return;
	}

	wlr_log(WLR_DEBUG, "Destroying connector %s", connector->output->name);

	if (connector->active_lease) {
		wlr_drm_lease_terminate(connector->active_lease->drm_lease);
	}

	struct wl_resource *resource, *tmp;
	wl_resource_for_each_safe(resource, tmp, &connector->resources) {
		wp_drm_lease_connector_v1_send_withdrawn(resource);

		wl_resource_set_user_data(resource, NULL);
		wl_list_remove(wl_resource_get_link(resource));
		wl_list_init(wl_resource_get_link(resource));
	}

	struct wl_resource *device_resource;
	wl_resource_for_each(device_resource, &connector->device->resources) {
		wp_drm_lease_device_v1_send_done(device_resource);
	}

	wl_list_remove(&connector->link);
	wl_list_remove(&connector->destroy.link);
	free(connector);
}

static void drm_lease_device_v1_destroy(
		struct wlr_drm_lease_device_v1 *device) {
	if (!device) {
		return;
	}

	struct wlr_drm_backend *backend =
			get_drm_backend_from_backend(device->backend);
	wlr_log(WLR_DEBUG, "Destroying wlr_drm_lease_device_v1 for %s",
			backend->name);

	struct wl_resource *resource, *tmp_resource;
	wl_resource_for_each_safe(resource, tmp_resource, &device->resources) {
		wl_list_remove(wl_resource_get_link(resource));
		wl_list_init(wl_resource_get_link(resource));
		wl_resource_set_user_data(resource, NULL);
	}

	struct wlr_drm_lease_request_v1 *request, *tmp_request;
	wl_list_for_each_safe(request, tmp_request, &device->requests, link) {
		drm_lease_request_v1_destroy(request);
	}

	struct wlr_drm_lease_v1 *lease, *tmp_lease;
	wl_list_for_each_safe(lease, tmp_lease, &device->leases, link) {
		wlr_drm_lease_terminate(lease->drm_lease);
	}

	struct wlr_drm_lease_connector_v1 *connector, *tmp_connector;
	wl_list_for_each_safe(connector, tmp_connector, &device->connectors, link) {
		drm_lease_connector_v1_destroy(connector);
	}

	wl_list_remove(&device->link);
	wlr_global_destroy_safe(device->global);

	free(device);
}

static void lease_handle_destroy(struct wl_listener *listener, void *data) {
	struct wlr_drm_lease_v1 *lease = wl_container_of(listener, lease, destroy);

	wlr_log(WLR_DEBUG, "Destroying lease %"PRIu32, lease->drm_lease->lessee_id);

	wp_drm_lease_v1_send_finished(lease->resource);

	wl_list_remove(&lease->destroy.link);

	for (size_t i = 0; i < lease->n_connectors; ++i) {
		lease->connectors[i]->active_lease = NULL;
	}

	wl_list_remove(&lease->link);
	wl_resource_set_user_data(lease->resource, NULL);

	free(lease->connectors);
	free(lease);
}

struct wlr_drm_lease_v1 *wlr_drm_lease_request_v1_grant(
		struct wlr_drm_lease_request_v1 *request) {
	assert(request->lease);

	wlr_log(WLR_DEBUG, "Attempting to grant request %p", request);

	struct wlr_drm_lease_v1 *lease = request->lease;
	assert(!request->invalid);

	/* Transform connectors list into wlr_output for leasing */
	struct wlr_output *outputs[request->n_connectors + 1];
	for(size_t i = 0; i < request->n_connectors; ++i) {
		outputs[i] = request->connectors[i]->output;
	}

	int fd;
	lease->drm_lease = wlr_drm_create_lease(outputs, request->n_connectors, &fd);
	if (!lease->drm_lease) {
		wlr_log(WLR_ERROR, "wlr_drm_create_lease failed");
		wp_drm_lease_v1_send_finished(lease->resource);
		return NULL;
	}

	lease->connectors = calloc(request->n_connectors,
			sizeof(struct wlr_drm_lease_connector_v1 *));
	if (!lease->connectors) {
		wlr_log(WLR_ERROR, "Failed to allocate lease connectors list");
		close(fd);
		wp_drm_lease_v1_send_finished(lease->resource);
		return NULL;
	}
	lease->n_connectors = request->n_connectors;
	for (size_t i = 0; i < request->n_connectors; ++i) {
		lease->connectors[i] = request->connectors[i];
		lease->connectors[i]->active_lease = lease;
	}

	lease->destroy.notify = lease_handle_destroy;
	wl_signal_add(&lease->drm_lease->events.destroy, &lease->destroy);

	wlr_log(WLR_DEBUG, "Granting request %p", request);

	wp_drm_lease_v1_send_lease_fd(lease->resource, fd);
	close(fd);

	return lease;
}

void wlr_drm_lease_request_v1_reject(
		struct wlr_drm_lease_request_v1 *request) {
	assert(request && request->lease);

	wlr_log(WLR_DEBUG, "Rejecting request %p", request);

	request->invalid = true;
	wp_drm_lease_v1_send_finished(request->lease->resource);
}

void wlr_drm_lease_v1_revoke(struct wlr_drm_lease_v1 *lease) {
	assert(lease);
	wlr_log(WLR_DEBUG, "Revoking lease %"PRIu32, lease->drm_lease->lessee_id);
	wlr_drm_lease_terminate(lease->drm_lease);
}

static void drm_lease_v1_handle_resource_destroy(struct wl_resource *resource) {
	struct wlr_drm_lease_v1 *lease = drm_lease_v1_from_resource(resource);
	if (lease != NULL) {
		wlr_drm_lease_terminate(lease->drm_lease);
	}
}

static void drm_lease_v1_handle_destroy(
		struct wl_client *client, struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static struct wp_drm_lease_v1_interface lease_impl = {
	.destroy = drm_lease_v1_handle_destroy,
};

static void drm_lease_request_v1_handle_resource_destroy(
		struct wl_resource *resource) {
	struct wlr_drm_lease_request_v1 *request =
		drm_lease_request_v1_from_resource(resource);
	drm_lease_request_v1_destroy(request);
}

static void drm_lease_request_v1_handle_request_connector(
		struct wl_client *client, struct wl_resource *request_resource,
		struct wl_resource *connector_resource) {
	struct wlr_drm_lease_request_v1 *request =
		drm_lease_request_v1_from_resource(request_resource);
	if (!request) {
		wlr_log(WLR_ERROR, "Request has been destroyed");
		return;
	}

	struct wlr_drm_lease_connector_v1 *connector =
		drm_lease_connector_v1_from_resource(connector_resource);

	if (!connector) {
		/* This connector offer has been withdrawn or is leased */
		wlr_log(WLR_ERROR, "Failed to request connector");
		request->invalid = true;
		return;
	}

	wlr_log(WLR_DEBUG, "Requesting connector %s", connector->output->name);

	if (request->device != connector->device) {
		wlr_log(WLR_ERROR, "The connector belongs to another device");
		wl_resource_post_error(request_resource,
				WP_DRM_LEASE_REQUEST_V1_ERROR_WRONG_DEVICE,
				"The requested connector belongs to another device");
		return;
	}

	for (size_t i = 0; i < request->n_connectors; ++i) {
		struct wlr_drm_lease_connector_v1 *tmp = request->connectors[i];

		if (connector == tmp) {
			wlr_log(WLR_ERROR, "The connector has already been requested");
			wl_resource_post_error(request_resource,
					WP_DRM_LEASE_REQUEST_V1_ERROR_DUPLICATE_CONNECTOR,
					"The connector has already been requested");
			return;
		}
	}

	size_t n_connectors = request->n_connectors + 1;

	struct wlr_drm_lease_connector_v1 **tmp_connectors =
			realloc(request->connectors,
			n_connectors * sizeof(struct wlr_drm_lease_connector_v1 *));
	if (!tmp_connectors) {
		wlr_log(WLR_ERROR, "Failed to grow connectors request array");
		return;
	}

	request->connectors = tmp_connectors;
	request->connectors[request->n_connectors] = connector;
	request->n_connectors = n_connectors;
}

static void drm_lease_request_v1_handle_submit(
		struct wl_client *client, struct wl_resource *resource, uint32_t id) {
	uint32_t version = wl_resource_get_version(resource);
	struct wl_resource *lease_resource = wl_resource_create(client,
			&wp_drm_lease_v1_interface, version, id);
	if (!lease_resource) {
		wlr_log(WLR_ERROR, "Failed to allocate wl_resource");
		wl_resource_post_no_memory(resource);
		return;
	}

	wl_resource_set_implementation(lease_resource, &lease_impl, NULL,
			drm_lease_v1_handle_resource_destroy);

	struct wlr_drm_lease_request_v1 *request =
			drm_lease_request_v1_from_resource(resource);
	if (!request) {
		wlr_log(WLR_DEBUG, "Request has been destroyed");
		return;
	}

	/* Pre-emptively reject invalid lease requests */
	if (request->invalid) {
		wlr_log(WLR_ERROR, "Invalid request");
		return;
	} else if (request->n_connectors == 0) {
		wl_resource_post_error(lease_resource,
				WP_DRM_LEASE_REQUEST_V1_ERROR_EMPTY_LEASE,
				"Lease request has no connectors");
		return;
	}

	for (size_t i = 0; i < request->n_connectors; ++i) {
		struct wlr_drm_lease_connector_v1 *conn = request->connectors[i];
		if (conn->active_lease) {
			wlr_log(WLR_ERROR, "Failed to create lease, connector %s has "
					"already been leased", conn->output->name);
			return;
		}
	}

	struct wlr_drm_lease_v1 *lease = calloc(1, sizeof(struct wlr_drm_lease_v1));
	if (!lease) {
		wlr_log(WLR_ERROR, "Failed to allocate wlr_drm_lease_v1");
		wl_resource_post_no_memory(resource);
		return;
	}

	lease->device = request->device;
	wl_list_insert(&lease->device->leases, &lease->link);

	lease->resource = lease_resource;
	wl_resource_set_user_data(lease_resource, lease);

	request->lease = lease;

	/* TODO: reject the request if the user does not grant it */
	wlr_signal_emit_safe(&request->device->manager->events.request,
			request);

	/* Request is done */
	wl_resource_destroy(resource);
}

static struct wp_drm_lease_request_v1_interface lease_request_impl = {
	.request_connector = drm_lease_request_v1_handle_request_connector,
	.submit = drm_lease_request_v1_handle_submit,
};

static void drm_lease_device_v1_handle_resource_destroy(
		struct wl_resource *resource) {
	wl_list_remove(wl_resource_get_link(resource));
}

static void drm_lease_device_v1_handle_release(
		struct wl_client *client, struct wl_resource *resource) {
	wp_drm_lease_device_v1_send_released(resource);
	wl_resource_destroy(resource);
}

static void drm_lease_device_v1_handle_create_lease_request(
		struct wl_client *client, struct wl_resource *resource, uint32_t id) {
	uint32_t version = wl_resource_get_version(resource);
	struct wl_resource *request_resource = wl_resource_create(client,
		&wp_drm_lease_request_v1_interface, version, id);
	if (!request_resource) {
		wlr_log(WLR_ERROR, "Failed to allocate wl_resource");
		return;
	}

	wl_resource_set_implementation(request_resource, &lease_request_impl,
		NULL, drm_lease_request_v1_handle_resource_destroy);

	struct wlr_drm_lease_device_v1 *device =
		drm_lease_device_v1_from_resource(resource);
	if (!device) {
		wlr_log(WLR_DEBUG, "Failed to create lease request, "
				"wlr_drm_lease_device_v1 has been destroyed");
		return;
	}

	struct wlr_drm_lease_request_v1 *req =
		calloc(1, sizeof(struct wlr_drm_lease_request_v1));
	if (!req) {
		wlr_log(WLR_ERROR, "Failed to allocate wlr_drm_lease_request_v1");
		wl_resource_post_no_memory(resource);
		return;
	}

	wlr_log(WLR_DEBUG, "Created request %p", req);

	req->device = device;
	req->resource = request_resource;
	req->connectors = NULL;
	req->n_connectors = 0;

	wl_resource_set_user_data(request_resource, req);

	wl_list_insert(&device->requests, &req->link);
}

static struct wp_drm_lease_device_v1_interface lease_device_impl = {
	.release = drm_lease_device_v1_handle_release,
	.create_lease_request = drm_lease_device_v1_handle_create_lease_request,
};

static void drm_connector_v1_handle_resource_destroy(
		struct wl_resource *resource) {
	wl_list_remove(wl_resource_get_link(resource));
}

static void drm_connector_v1_handle_destroy(
		struct wl_client *client, struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static struct wp_drm_lease_connector_v1_interface lease_connector_impl = {
	.destroy = drm_connector_v1_handle_destroy,
};

static void drm_lease_connector_v1_send_to_client(
		struct wlr_drm_lease_connector_v1 *connector,
		struct wl_resource *resource) {
	if (connector->active_lease) {
		return;
	}

	struct wl_client *client = wl_resource_get_client(resource);

	uint32_t version = wl_resource_get_version(resource);
	struct wl_resource *connector_resource = wl_resource_create(client,
			&wp_drm_lease_connector_v1_interface, version, 0);
	if (!connector_resource) {
		wl_client_post_no_memory(client);
		return;
	}

	wl_resource_set_implementation(connector_resource, &lease_connector_impl,
			connector, drm_connector_v1_handle_resource_destroy);
	wp_drm_lease_device_v1_send_connector(resource, connector_resource);

	struct wlr_output *output = connector->output;
	wp_drm_lease_connector_v1_send_name(connector_resource, output->name);

	// TODO: re-send the description when it's updated
	wp_drm_lease_connector_v1_send_description(connector_resource,
			output->description);

	wp_drm_lease_connector_v1_send_connector_id(connector_resource,
			wlr_drm_connector_get_id(output));

	wp_drm_lease_connector_v1_send_done(connector_resource);

	wl_list_insert(&connector->resources,
			wl_resource_get_link(connector_resource));
}

static void lease_device_bind(struct wl_client *wl_client, void *data,
		uint32_t version, uint32_t id) {
	struct wl_resource *device_resource  = wl_resource_create(wl_client,
		&wp_drm_lease_device_v1_interface, version, id);
	if (!device_resource) {
		wl_client_post_no_memory(wl_client);
		return;
	}

	wl_resource_set_implementation(device_resource, &lease_device_impl, NULL,
			drm_lease_device_v1_handle_resource_destroy);

	struct wlr_drm_lease_device_v1 *device = data;
	if (!device) {
		wlr_log(WLR_DEBUG, "Failed to bind lease device, "
				"the wlr_drm_lease_device_v1 has been destroyed");
		return;
	}

	wl_resource_set_user_data(device_resource, device);

	int fd = wlr_drm_backend_get_non_master_fd(device->backend);
	if (fd < 0) {
		wlr_log(WLR_ERROR, "Unable to get read only DRM fd for leasing");
		wl_client_post_no_memory(wl_client);
		return;
	}

	wp_drm_lease_device_v1_send_drm_fd(device_resource, fd);
	close(fd);

	wl_list_insert(&device->resources, wl_resource_get_link(device_resource));

	struct wlr_drm_lease_connector_v1 *connector;
	wl_list_for_each(connector, &device->connectors, link) {
		drm_lease_connector_v1_send_to_client(connector, device_resource);
	}

	wp_drm_lease_device_v1_send_done(device_resource);
}

static void handle_output_destroy(struct wl_listener *listener, void *data) {
	struct wlr_drm_lease_connector_v1 *conn = wl_container_of(listener, conn,
			destroy);
	wlr_log(WLR_DEBUG, "Handle destruction of output %s", conn->output->name);
	wlr_drm_lease_v1_manager_withdraw_output(conn->device->manager, conn->output);
}

bool wlr_drm_lease_v1_manager_offer_output(
		struct wlr_drm_lease_v1_manager *manager, struct wlr_output *output) {
	assert(manager && output);
	assert(wlr_output_is_drm(output));

	wlr_log(WLR_DEBUG, "Offering output %s", output->name);

	struct wlr_drm_lease_device_v1 *device = NULL, *tmp_device;
	wl_list_for_each(tmp_device, &manager->devices, link) {
		if (tmp_device->backend == output->backend) {
			device = tmp_device;
			break;
		}
	}

	if (!device) {
		wlr_log(WLR_ERROR, "No wlr_drm_lease_device_v1 associated with the "
				"offered output");
		return false;
	}

	struct wlr_drm_lease_connector_v1 *tmp_connector;
	wl_list_for_each(tmp_connector, &device->connectors, link) {
		if (tmp_connector->output == output) {
			wlr_log(WLR_ERROR, "Output %s has already been offered",
					output->name);
			return false;
		}
	}

	struct wlr_drm_lease_connector_v1 *connector =
			calloc(1, sizeof(struct wlr_drm_lease_connector_v1));
	if (!connector) {
		wlr_log(WLR_ERROR, "Failed to allocate wlr_drm_lease_connector_v1");
		return false;
	}

	connector->output = output;
	connector->device = device;

	connector->destroy.notify = handle_output_destroy;
	wl_signal_add(&output->events.destroy, &connector->destroy);

	wl_list_init(&connector->resources);
	wl_list_insert(&device->connectors, &connector->link);

	struct wl_resource *resource;
	wl_resource_for_each(resource, &device->resources) {
		drm_lease_connector_v1_send_to_client(connector, resource);
		wp_drm_lease_device_v1_send_done(resource);
	}

	return true;
}

void wlr_drm_lease_v1_manager_withdraw_output(
	struct wlr_drm_lease_v1_manager *manager, struct wlr_output *output) {
	assert(manager && output);

	wlr_log(WLR_DEBUG, "Withdrawing output %s", output->name);

	struct wlr_drm_lease_device_v1 *device = NULL, *tmp_device;
	wl_list_for_each(tmp_device, &manager->devices, link) {
		if (tmp_device->backend == output->backend) {
			device = tmp_device;
			break;
		}
	}

	if (!device) {
		wlr_log(WLR_ERROR, "No wlr_drm_lease_device_v1 associated with the "
				"given output");
		return;
	}

	struct wlr_drm_lease_connector_v1 *connector = NULL, *tmp_conn;
	wl_list_for_each(tmp_conn, &device->connectors, link) {
		if (tmp_conn->output == output) {
			connector = tmp_conn;
			break;
		}
	}

	if (!connector) {
		wlr_log(WLR_DEBUG, "No wlr_drm_connector_v1 associated with the given "
				"output");
		return;
	}

	drm_lease_connector_v1_destroy(connector);
}

static void handle_backend_destroy(struct wl_listener *listener, void *data) {
	struct wlr_drm_lease_device_v1 *device =
		wl_container_of(listener, device, backend_destroy);
	drm_lease_device_v1_destroy(device);
}

static void drm_lease_device_v1_create(struct wlr_drm_lease_v1_manager *manager,
		struct wlr_backend *backend) {
	assert(backend);

	struct wlr_drm_backend *drm_backend =
			get_drm_backend_from_backend(backend);
	wlr_log(WLR_DEBUG, "Creating wlr_drm_lease_device_v1 for %s",
			drm_backend->name);

	struct wlr_drm_lease_device_v1 *lease_device =
		calloc(1, sizeof(struct wlr_drm_lease_device_v1));

	if (!lease_device) {
		wlr_log(WLR_ERROR, "Failed to allocate wlr_drm_lease_device_v1");
		return;
	}

	lease_device->manager = manager;
	lease_device->backend = backend;
	wl_list_init(&lease_device->resources);
	wl_list_init(&lease_device->connectors);
	wl_list_init(&lease_device->requests);
	wl_list_init(&lease_device->leases);
	wl_list_init(&lease_device->link);

	lease_device->global = wl_global_create(manager->display,
		&wp_drm_lease_device_v1_interface, DRM_LEASE_DEVICE_V1_VERSION,
		lease_device, lease_device_bind);

	if (!lease_device->global) {
		wlr_log(WLR_ERROR, "Failed to allocate wp_drm_lease_device_v1 global");
		free(lease_device);
		return;
	}

	lease_device->backend_destroy.notify = handle_backend_destroy;
	wl_signal_add(&backend->events.destroy, &lease_device->backend_destroy);

	wl_list_insert(&manager->devices, &lease_device->link);
}

static void multi_backend_cb(struct wlr_backend *backend, void *data) {
	if (!wlr_backend_is_drm(backend)) {
		return;
	}

	struct wlr_drm_lease_v1_manager *manager = data;
	drm_lease_device_v1_create(manager, backend);
}

static void handle_display_destroy(struct wl_listener *listener, void *data) {
	struct wlr_drm_lease_v1_manager *manager = wl_container_of(listener, manager,
			display_destroy);
	wlr_log(WLR_DEBUG, "Destroying wlr_drm_lease_v1_manager");

	struct wlr_drm_lease_device_v1 *device, *tmp;
	wl_list_for_each_safe(device, tmp, &manager->devices, link) {
		drm_lease_device_v1_destroy(device);
	}

	free(manager);
}

struct wlr_drm_lease_v1_manager *wlr_drm_lease_v1_manager_create(
		struct wl_display *display, struct wlr_backend *backend) {
	struct wlr_drm_lease_v1_manager *manager = calloc(1,
			sizeof(struct wlr_drm_lease_v1_manager));
	if (!manager) {
		wlr_log(WLR_ERROR, "Failed to allocate wlr_drm_lease_v1_manager");
		return NULL;
	}

	wl_list_init(&manager->devices);
	manager->display = display;

	if (wlr_backend_is_multi(backend)) {
		/* TODO: handle backends added after the manager is created */
		wlr_multi_for_each_backend(backend, multi_backend_cb, manager);
	} else if (wlr_backend_is_drm(backend)) {
		drm_lease_device_v1_create(manager, backend);
	}

	if (wl_list_empty(&manager->devices)) {
		wlr_log(WLR_ERROR, "No DRM backend supplied, failed to create "
				"wlr_drm_lease_v1_manager");
		free(manager);
		return NULL;
	}

	manager->display_destroy.notify = handle_display_destroy;
	wl_display_add_destroy_listener(display, &manager->display_destroy);

	wl_signal_init(&manager->events.request);

	return manager;
}
