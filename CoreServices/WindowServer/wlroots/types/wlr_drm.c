#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <drm_fourcc.h>
#include <stdlib.h>
#include <unistd.h>
#include <xf86drm.h>
#include <wlr/interfaces/wlr_buffer.h>
#include <wlr/render/drm_format_set.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_buffer.h>
#include <wlr/types/wlr_drm.h>
#include <wlr/util/log.h>
#include "drm-protocol.h"
#include "util/signal.h"

#define WLR_DRM_VERSION 2

static void buffer_handle_destroy(struct wl_client *client,
		struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static const struct wl_buffer_interface wl_buffer_impl = {
	.destroy = buffer_handle_destroy,
};

static const struct wlr_buffer_impl buffer_impl;

static struct wlr_drm_buffer *drm_buffer_from_buffer(
		struct wlr_buffer *buffer) {
	assert(buffer->impl == &buffer_impl);
	return (struct wlr_drm_buffer *)buffer;
}

static void buffer_destroy(struct wlr_buffer *wlr_buffer) {
	struct wlr_drm_buffer *buffer = drm_buffer_from_buffer(wlr_buffer);
	if (buffer->resource != NULL) {
		wl_resource_set_user_data(buffer->resource, NULL);
	}
	wlr_dmabuf_attributes_finish(&buffer->dmabuf);
	wl_list_remove(&buffer->release.link);
	free(buffer);
}

static bool buffer_get_dmabuf(struct wlr_buffer *wlr_buffer,
		struct wlr_dmabuf_attributes *dmabuf) {
	struct wlr_drm_buffer *buffer = drm_buffer_from_buffer(wlr_buffer);
	*dmabuf = buffer->dmabuf;
	return true;
}

static const struct wlr_buffer_impl buffer_impl = {
	.destroy = buffer_destroy,
	.get_dmabuf = buffer_get_dmabuf,
};

bool wlr_drm_buffer_is_resource(struct wl_resource *resource) {
	return wl_resource_instance_of(resource, &wl_buffer_interface,
		&wl_buffer_impl);
}

struct wlr_drm_buffer *wlr_drm_buffer_from_resource(
		struct wl_resource *resource) {
	assert(wlr_drm_buffer_is_resource(resource));
	return wl_resource_get_user_data(resource);
}

static void buffer_handle_resource_destroy(struct wl_resource *resource) {
	struct wlr_drm_buffer *buffer = wlr_drm_buffer_from_resource(resource);
	buffer->resource = NULL;
	wlr_buffer_drop(&buffer->base);
}

static void buffer_handle_release(struct wl_listener *listener, void *data) {
	struct wlr_drm_buffer *buffer = wl_container_of(listener, buffer, release);
	if (buffer->resource != NULL) {
		wl_buffer_send_release(buffer->resource);
	}
}

static void drm_handle_authenticate(struct wl_client *client,
		struct wl_resource *resource, uint32_t id) {
	// We only use render nodes, which don't need authentication
	wl_drm_send_authenticated(resource);
}

static void drm_handle_create_buffer(struct wl_client *client,
		struct wl_resource *resource, uint32_t id, uint32_t name, int32_t width,
		int32_t height, uint32_t stride, uint32_t format) {
	wl_resource_post_error(resource, WL_DRM_ERROR_INVALID_NAME,
		"Flink handles are not supported, use DMA-BUF instead");
}

static void drm_handle_create_planar_buffer(struct wl_client *client,
		struct wl_resource *resource, uint32_t id, uint32_t name, int32_t width,
		int32_t height, uint32_t format, int32_t offset0, int32_t stride0,
		int32_t offset1, int32_t stride1, int32_t offset2, int32_t stride2) {
	wl_resource_post_error(resource, WL_DRM_ERROR_INVALID_NAME,
		"Flink handles are not supported, use DMA-BUF instead");
}

static void drm_handle_create_prime_buffer(struct wl_client *client,
		struct wl_resource *resource, uint32_t id, int fd, int32_t width,
		int32_t height, uint32_t format, int32_t offset0, int32_t stride0,
		int32_t offset1, int32_t stride1, int32_t offset2, int32_t stride2) {
	struct wlr_dmabuf_attributes dmabuf = {
		.width = width,
		.height = height,
		.format = format,
		.modifier = DRM_FORMAT_MOD_INVALID,
		.n_planes = 1,
		.offset[0] = offset0,
		.stride[0] = stride0,
		.fd[0] = fd,
	};

	struct wlr_drm_buffer *buffer = calloc(1, sizeof(*buffer));
	if (buffer == NULL) {
		close(fd);
		wl_resource_post_no_memory(resource);
		return;
	}
	wlr_buffer_init(&buffer->base, &buffer_impl, width, height);

	buffer->resource = wl_resource_create(client, &wl_buffer_interface, 1, id);
	if (buffer->resource == NULL) {
		free(buffer);
		close(fd);
		wl_resource_post_no_memory(resource);
		return;
	}
	wl_resource_set_implementation(buffer->resource, &wl_buffer_impl, buffer,
		buffer_handle_resource_destroy);

	buffer->dmabuf = dmabuf;

	buffer->release.notify = buffer_handle_release;
	wl_signal_add(&buffer->base.events.release, &buffer->release);
}

static const struct wl_drm_interface drm_impl = {
	.authenticate = drm_handle_authenticate,
	.create_buffer = drm_handle_create_buffer,
	.create_planar_buffer = drm_handle_create_planar_buffer,
	.create_prime_buffer = drm_handle_create_prime_buffer,
};

static void drm_bind(struct wl_client *client, void *data,
		uint32_t version, uint32_t id) {
	struct wlr_drm *drm = data;

	struct wl_resource *resource = wl_resource_create(client,
		&wl_drm_interface, version, id);
	if (resource == NULL) {
		wl_client_post_no_memory(client);
		return;
	}
	wl_resource_set_implementation(resource, &drm_impl, drm, NULL);

	wl_drm_send_device(resource, drm->node_name);
	wl_drm_send_capabilities(resource, WL_DRM_CAPABILITY_PRIME);

	const struct wlr_drm_format_set *formats =
		wlr_renderer_get_dmabuf_texture_formats(drm->renderer);
	if (formats == NULL) {
		return;
	}

	for (size_t i = 0; i < formats->len; i++) {
		wl_drm_send_format(resource, formats->formats[i]->format);
	}
}

static void drm_destroy(struct wlr_drm *drm) {
	wlr_signal_emit_safe(&drm->events.destroy, NULL);

	wl_list_remove(&drm->display_destroy.link);
	wl_list_remove(&drm->renderer_destroy.link);

	free(drm->node_name);
	wl_global_destroy(drm->global);
	free(drm);
}

static void handle_display_destroy(struct wl_listener *listener, void *data) {
	struct wlr_drm *drm = wl_container_of(listener, drm, display_destroy);
	drm_destroy(drm);
}

static void handle_renderer_destroy(struct wl_listener *listener, void *data) {
	struct wlr_drm *drm = wl_container_of(listener, drm, renderer_destroy);
	drm_destroy(drm);
}

struct wlr_drm *wlr_drm_create(struct wl_display *display,
		struct wlr_renderer *renderer) {
	int drm_fd = wlr_renderer_get_drm_fd(renderer);
	if (drm_fd < 0) {
		wlr_log(WLR_ERROR, "Failed to get DRM FD from renderer");
		return NULL;
	}

	drmDevice *dev = NULL;
	if (drmGetDevice2(drm_fd, 0, &dev) != 0) {
		wlr_log(WLR_ERROR, "drmGetDevice2 failed");
		return NULL;
	}

	char *node_name = NULL;
	if (dev->available_nodes & (1 << DRM_NODE_RENDER)) {
		node_name = strdup(dev->nodes[DRM_NODE_RENDER]);
	} else {
		assert(dev->available_nodes & (1 << DRM_NODE_PRIMARY));
		wlr_log(WLR_DEBUG, "No DRM render node available, "
			"falling back to primary node '%s'", dev->nodes[DRM_NODE_PRIMARY]);
		node_name = strdup(dev->nodes[DRM_NODE_PRIMARY]);
	}
	drmFreeDevice(&dev);
	if (node_name == NULL) {
		return NULL;
	}

	struct wlr_drm *drm = calloc(1, sizeof(*drm));
	if (drm == NULL) {
		free(node_name);
		return NULL;
	}

	drm->node_name = node_name;
	drm->renderer = renderer;
	wl_signal_init(&drm->events.destroy);

	drm->global = wl_global_create(display, &wl_drm_interface, WLR_DRM_VERSION,
		drm, drm_bind);
	if (drm->global == NULL) {
		free(drm->node_name);
		free(drm);
		return NULL;
	}

	drm->display_destroy.notify = handle_display_destroy;
	wl_display_add_destroy_listener(display, &drm->display_destroy);

	drm->renderer_destroy.notify = handle_renderer_destroy;
	wl_signal_add(&renderer->events.destroy, &drm->renderer_destroy);

	return drm;
}
