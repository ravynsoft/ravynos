#include <assert.h>
#include <drm_fourcc.h>
#include <stdlib.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_buffer.h>
#include <wlr/types/wlr_drm.h>
#include <wlr/types/wlr_linux_dmabuf_v1.h>
#include <wlr/util/log.h>
#include "render/pixel_format.h"
#include "types/wlr_buffer.h"
#include "util/signal.h"

void wlr_buffer_init(struct wlr_buffer *buffer,
		const struct wlr_buffer_impl *impl, int width, int height) {
	assert(impl->destroy);
	if (impl->begin_data_ptr_access || impl->end_data_ptr_access) {
		assert(impl->begin_data_ptr_access && impl->end_data_ptr_access);
	}
	buffer->impl = impl;
	buffer->width = width;
	buffer->height = height;
	wl_signal_init(&buffer->events.destroy);
	wl_signal_init(&buffer->events.release);
	wlr_addon_set_init(&buffer->addons);
}

static void buffer_consider_destroy(struct wlr_buffer *buffer) {
	if (!buffer->dropped || buffer->n_locks > 0) {
		return;
	}

	assert(!buffer->accessing_data_ptr);

	wlr_signal_emit_safe(&buffer->events.destroy, NULL);
	wlr_addon_set_finish(&buffer->addons);

	buffer->impl->destroy(buffer);
}

void wlr_buffer_drop(struct wlr_buffer *buffer) {
	if (buffer == NULL) {
		return;
	}

	assert(!buffer->dropped);
	buffer->dropped = true;
	buffer_consider_destroy(buffer);
}

struct wlr_buffer *wlr_buffer_lock(struct wlr_buffer *buffer) {
	buffer->n_locks++;
	return buffer;
}

void wlr_buffer_unlock(struct wlr_buffer *buffer) {
	if (buffer == NULL) {
		return;
	}

	assert(buffer->n_locks > 0);
	buffer->n_locks--;

	if (buffer->n_locks == 0) {
		wl_signal_emit(&buffer->events.release, NULL);
	}

	buffer_consider_destroy(buffer);
}

bool wlr_buffer_get_dmabuf(struct wlr_buffer *buffer,
		struct wlr_dmabuf_attributes *attribs) {
	if (!buffer->impl->get_dmabuf) {
		return false;
	}
	return buffer->impl->get_dmabuf(buffer, attribs);
}

bool wlr_buffer_begin_data_ptr_access(struct wlr_buffer *buffer, uint32_t flags,
		void **data, uint32_t *format, size_t *stride) {
	assert(!buffer->accessing_data_ptr);
	if (!buffer->impl->begin_data_ptr_access) {
		return false;
	}
	if (!buffer->impl->begin_data_ptr_access(buffer, flags, data, format, stride)) {
		return false;
	}
	buffer->accessing_data_ptr = true;
	return true;
}

void wlr_buffer_end_data_ptr_access(struct wlr_buffer *buffer) {
	assert(buffer->accessing_data_ptr);
	buffer->impl->end_data_ptr_access(buffer);
	buffer->accessing_data_ptr = false;
}

bool wlr_buffer_get_shm(struct wlr_buffer *buffer,
		struct wlr_shm_attributes *attribs) {
	if (!buffer->impl->get_shm) {
		return false;
	}
	return buffer->impl->get_shm(buffer, attribs);
}

bool wlr_resource_is_buffer(struct wl_resource *resource) {
	return strcmp(wl_resource_get_class(resource), wl_buffer_interface.name) == 0;
}

static const struct wlr_buffer_impl client_buffer_impl;

struct wlr_client_buffer *wlr_client_buffer_get(struct wlr_buffer *buffer) {
	if (buffer->impl != &client_buffer_impl) {
		return NULL;
	}
	return (struct wlr_client_buffer *)buffer;
}

static struct wlr_client_buffer *client_buffer_from_buffer(
		struct wlr_buffer *buffer) {
	struct wlr_client_buffer *client_buffer = wlr_client_buffer_get(buffer);
	assert(client_buffer != NULL);
	return client_buffer;
}

static void client_buffer_destroy(struct wlr_buffer *buffer) {
	struct wlr_client_buffer *client_buffer = client_buffer_from_buffer(buffer);
	wl_list_remove(&client_buffer->source_destroy.link);
	wlr_texture_destroy(client_buffer->texture);
	free(client_buffer);
}

static bool client_buffer_get_dmabuf(struct wlr_buffer *buffer,
		struct wlr_dmabuf_attributes *attribs) {
	struct wlr_client_buffer *client_buffer = client_buffer_from_buffer(buffer);

	if (client_buffer->source == NULL) {
		return false;
	}

	return wlr_buffer_get_dmabuf(client_buffer->source, attribs);
}

static const struct wlr_buffer_impl client_buffer_impl = {
	.destroy = client_buffer_destroy,
	.get_dmabuf = client_buffer_get_dmabuf,
};

static void client_buffer_handle_source_destroy(struct wl_listener *listener,
		void *data) {
	struct wlr_client_buffer *client_buffer =
		wl_container_of(listener, client_buffer, source_destroy);
	wl_list_remove(&client_buffer->source_destroy.link);
	wl_list_init(&client_buffer->source_destroy.link);
	client_buffer->source = NULL;
}

static struct wlr_shm_client_buffer *shm_client_buffer_get_or_create(
	struct wl_resource *resource);
static bool buffer_is_shm_client_buffer(struct wlr_buffer *buffer);
static struct wlr_shm_client_buffer *shm_client_buffer_from_buffer(
	struct wlr_buffer *buffer);

/* struct wlr_buffer_resource_interface */
static struct wl_array buffer_resource_interfaces = {0};

void wlr_buffer_register_resource_interface(
		const struct wlr_buffer_resource_interface *iface) {
	assert(iface);
	assert(iface->is_instance);
	assert(iface->from_resource);

	const struct wlr_buffer_resource_interface **iface_ptr;
	wl_array_for_each(iface_ptr, &buffer_resource_interfaces) {
		if (*iface_ptr == iface) {
			wlr_log(WLR_DEBUG, "wlr_resource_buffer_interface %s has already"
					"been registered", iface->name);
			return;
		}
	}

	iface_ptr = wl_array_add(&buffer_resource_interfaces, sizeof(iface));
	*iface_ptr = iface;
}

static const struct wlr_buffer_resource_interface *get_buffer_resource_iface(
		struct wl_resource *resource) {
	struct wlr_buffer_resource_interface **iface_ptr;
	wl_array_for_each(iface_ptr, &buffer_resource_interfaces) {
		if ((*iface_ptr)->is_instance(resource)) {
			return *iface_ptr;
		}
	}

	return NULL;
}

struct wlr_buffer *wlr_buffer_from_resource(struct wl_resource *resource) {
	assert(resource && wlr_resource_is_buffer(resource));

	struct wlr_buffer *buffer;
	if (wl_shm_buffer_get(resource) != NULL) {
		struct wlr_shm_client_buffer *shm_client_buffer =
			shm_client_buffer_get_or_create(resource);
		if (shm_client_buffer == NULL) {
			wlr_log(WLR_ERROR, "Failed to create shm client buffer");
			return NULL;
		}
		buffer = wlr_buffer_lock(&shm_client_buffer->base);
	} else if (wlr_dmabuf_v1_resource_is_buffer(resource)) {
		struct wlr_dmabuf_v1_buffer *dmabuf =
			wlr_dmabuf_v1_buffer_from_buffer_resource(resource);
		buffer = wlr_buffer_lock(&dmabuf->base);
	} else if (wlr_drm_buffer_is_resource(resource)) {
		struct wlr_drm_buffer *drm_buffer =
			wlr_drm_buffer_from_resource(resource);
		buffer = wlr_buffer_lock(&drm_buffer->base);
	} else {
		const struct wlr_buffer_resource_interface *iface =
				get_buffer_resource_iface(resource);
		if (!iface) {
			wlr_log(WLR_ERROR, "Unknown buffer type");
			return NULL;
		}

		struct wlr_buffer *custom_buffer = iface->from_resource(resource);
		if (!custom_buffer) {
			wlr_log(WLR_ERROR, "Failed to create %s buffer", iface->name);
			return NULL;
		}

		buffer = wlr_buffer_lock(custom_buffer);
	}

	return buffer;
}

struct wlr_client_buffer *wlr_client_buffer_create(struct wlr_buffer *buffer,
		struct wlr_renderer *renderer) {
	struct wlr_texture *texture = wlr_texture_from_buffer(renderer, buffer);
	if (texture == NULL) {
		wlr_log(WLR_ERROR, "Failed to create texture");
		return NULL;
	}

	struct wlr_client_buffer *client_buffer =
		calloc(1, sizeof(struct wlr_client_buffer));
	if (client_buffer == NULL) {
		wlr_texture_destroy(texture);
		return NULL;
	}
	wlr_buffer_init(&client_buffer->base, &client_buffer_impl,
		texture->width, texture->height);
	client_buffer->source = buffer;
	client_buffer->texture = texture;

	wl_signal_add(&buffer->events.destroy, &client_buffer->source_destroy);
	client_buffer->source_destroy.notify = client_buffer_handle_source_destroy;

	if (buffer_is_shm_client_buffer(buffer)) {
		struct wlr_shm_client_buffer *shm_client_buffer =
			shm_client_buffer_from_buffer(buffer);
		client_buffer->shm_source_format = shm_client_buffer->format;
	} else {
		client_buffer->shm_source_format = DRM_FORMAT_INVALID;
	}

	// Ensure the buffer will be released before being destroyed
	wlr_buffer_lock(&client_buffer->base);
	wlr_buffer_drop(&client_buffer->base);

	return client_buffer;
}

bool wlr_client_buffer_apply_damage(struct wlr_client_buffer *client_buffer,
		struct wlr_buffer *next, pixman_region32_t *damage) {
	if (client_buffer->base.n_locks > 1) {
		// Someone else still has a reference to the buffer
		return false;
	}

	if ((uint32_t)next->width != client_buffer->texture->width ||
			(uint32_t)next->height != client_buffer->texture->height) {
		return false;
	}

	if (client_buffer->shm_source_format == DRM_FORMAT_INVALID) {
		// Uploading only damaged regions only works for wl_shm buffers and
		// mutable textures (created from wl_shm buffer)
		return false;
	}

	void *data;
	uint32_t format;
	size_t stride;
	if (!wlr_buffer_begin_data_ptr_access(next, WLR_BUFFER_DATA_PTR_ACCESS_READ,
			&data, &format, &stride)) {
		return false;
	}

	if (format != client_buffer->shm_source_format) {
		// Uploading to textures can't change the format
		wlr_buffer_end_data_ptr_access(next);
		return false;
	}

	int n;
	pixman_box32_t *rects = pixman_region32_rectangles(damage, &n);
	for (int i = 0; i < n; ++i) {
		pixman_box32_t *r = &rects[i];
		if (!wlr_texture_write_pixels(client_buffer->texture, stride,
				r->x2 - r->x1, r->y2 - r->y1, r->x1, r->y1,
				r->x1, r->y1, data)) {
			wlr_buffer_end_data_ptr_access(next);
			return false;
		}
	}

	wlr_buffer_end_data_ptr_access(next);

	return true;
}

static const struct wlr_buffer_impl shm_client_buffer_impl;

static bool buffer_is_shm_client_buffer(struct wlr_buffer *buffer) {
	return buffer->impl == &shm_client_buffer_impl;
}

static struct wlr_shm_client_buffer *shm_client_buffer_from_buffer(
		struct wlr_buffer *buffer) {
	assert(buffer_is_shm_client_buffer(buffer));
	return (struct wlr_shm_client_buffer *)buffer;
}

static void shm_client_buffer_destroy(struct wlr_buffer *wlr_buffer) {
	struct wlr_shm_client_buffer *buffer =
		shm_client_buffer_from_buffer(wlr_buffer);
	wl_list_remove(&buffer->resource_destroy.link);
	wl_list_remove(&buffer->release.link);
	if (buffer->saved_shm_pool != NULL) {
		wl_shm_pool_unref(buffer->saved_shm_pool);
	}
	free(buffer);
}

static bool shm_client_buffer_begin_data_ptr_access(struct wlr_buffer *wlr_buffer,
		uint32_t flags, void **data, uint32_t *format, size_t *stride) {
	struct wlr_shm_client_buffer *buffer =
		shm_client_buffer_from_buffer(wlr_buffer);
	*format = buffer->format;
	*stride = buffer->stride;
	if (buffer->shm_buffer != NULL) {
		*data = wl_shm_buffer_get_data(buffer->shm_buffer);
		wl_shm_buffer_begin_access(buffer->shm_buffer);
	} else {
		*data = buffer->saved_data;
	}
	return true;
}

static void shm_client_buffer_end_data_ptr_access(struct wlr_buffer *wlr_buffer) {
	struct wlr_shm_client_buffer *buffer =
		shm_client_buffer_from_buffer(wlr_buffer);
	if (buffer->shm_buffer != NULL) {
		wl_shm_buffer_end_access(buffer->shm_buffer);
	}
}

static const struct wlr_buffer_impl shm_client_buffer_impl = {
	.destroy = shm_client_buffer_destroy,
	.begin_data_ptr_access = shm_client_buffer_begin_data_ptr_access,
	.end_data_ptr_access = shm_client_buffer_end_data_ptr_access,
};

static void shm_client_buffer_resource_handle_destroy(
		struct wl_listener *listener, void *data) {
	struct wlr_shm_client_buffer *buffer =
		wl_container_of(listener, buffer, resource_destroy);

	// In order to still be able to access the shared memory region, we need to
	// keep a reference to the wl_shm_pool
	buffer->saved_shm_pool = wl_shm_buffer_ref_pool(buffer->shm_buffer);
	buffer->saved_data = wl_shm_buffer_get_data(buffer->shm_buffer);

	// The wl_shm_buffer destroys itself with the wl_resource
	buffer->resource = NULL;
	buffer->shm_buffer = NULL;
	wl_list_remove(&buffer->resource_destroy.link);
	wl_list_init(&buffer->resource_destroy.link);

	// This might destroy the buffer
	wlr_buffer_drop(&buffer->base);
}

static void shm_client_buffer_handle_release(struct wl_listener *listener,
		void *data) {
	struct wlr_shm_client_buffer *buffer =
		wl_container_of(listener, buffer, release);
	if (buffer->resource != NULL) {
		wl_buffer_send_release(buffer->resource);
	}
}

static struct wlr_shm_client_buffer *shm_client_buffer_get_or_create(
		struct wl_resource *resource) {
	struct wl_shm_buffer *shm_buffer = wl_shm_buffer_get(resource);
	assert(shm_buffer != NULL);

	struct wl_listener *resource_destroy_listener =
		wl_resource_get_destroy_listener(resource,
		shm_client_buffer_resource_handle_destroy);
	if (resource_destroy_listener != NULL) {
		struct wlr_shm_client_buffer *buffer =
			wl_container_of(resource_destroy_listener, buffer, resource_destroy);
		return buffer;
	}

	int32_t width = wl_shm_buffer_get_width(shm_buffer);
	int32_t height = wl_shm_buffer_get_height(shm_buffer);

	struct wlr_shm_client_buffer *buffer = calloc(1, sizeof(*buffer));
	if (buffer == NULL) {
		return NULL;
	}
	wlr_buffer_init(&buffer->base, &shm_client_buffer_impl, width, height);
	buffer->resource = resource;
	buffer->shm_buffer = shm_buffer;

	enum wl_shm_format wl_shm_format = wl_shm_buffer_get_format(shm_buffer);
	buffer->format = convert_wl_shm_format_to_drm(wl_shm_format);
	buffer->stride = wl_shm_buffer_get_stride(shm_buffer);

	buffer->resource_destroy.notify = shm_client_buffer_resource_handle_destroy;
	wl_resource_add_destroy_listener(resource, &buffer->resource_destroy);

	buffer->release.notify = shm_client_buffer_handle_release;
	wl_signal_add(&buffer->base.events.release, &buffer->release);

	return buffer;
}

static const struct wlr_buffer_impl readonly_data_buffer_impl;

static struct wlr_readonly_data_buffer *readonly_data_buffer_from_buffer(
		struct wlr_buffer *buffer) {
	assert(buffer->impl == &readonly_data_buffer_impl);
	return (struct wlr_readonly_data_buffer *)buffer;
}

static void readonly_data_buffer_destroy(struct wlr_buffer *wlr_buffer) {
	struct wlr_readonly_data_buffer *buffer =
		readonly_data_buffer_from_buffer(wlr_buffer);
	free(buffer->saved_data);
	free(buffer);
}

static bool readonly_data_buffer_begin_data_ptr_access(struct wlr_buffer *wlr_buffer,
		uint32_t flags, void **data, uint32_t *format, size_t *stride) {
	struct wlr_readonly_data_buffer *buffer =
		readonly_data_buffer_from_buffer(wlr_buffer);
	if (buffer->data == NULL) {
		return false;
	}
	if (flags & WLR_BUFFER_DATA_PTR_ACCESS_WRITE) {
		return false;
	}
	*data = (void *)buffer->data;
	*format = buffer->format;
	*stride = buffer->stride;
	return true;
}

static void readonly_data_buffer_end_data_ptr_access(struct wlr_buffer *wlr_buffer) {
	// This space is intentionally left blank
}

static const struct wlr_buffer_impl readonly_data_buffer_impl = {
	.destroy = readonly_data_buffer_destroy,
	.begin_data_ptr_access = readonly_data_buffer_begin_data_ptr_access,
	.end_data_ptr_access = readonly_data_buffer_end_data_ptr_access,
};

struct wlr_readonly_data_buffer *readonly_data_buffer_create(uint32_t format,
		size_t stride, uint32_t width, uint32_t height, const void *data) {
	struct wlr_readonly_data_buffer *buffer = calloc(1, sizeof(*buffer));
	if (buffer == NULL) {
		return NULL;
	}
	wlr_buffer_init(&buffer->base, &readonly_data_buffer_impl, width, height);

	buffer->data = data;
	buffer->format = format;
	buffer->stride = stride;

	return buffer;
}

bool readonly_data_buffer_drop(struct wlr_readonly_data_buffer *buffer) {
	bool ok = true;

	if (buffer->base.n_locks > 0) {
		size_t size = buffer->stride * buffer->base.height;
		buffer->saved_data = malloc(size);
		if (buffer->saved_data == NULL) {
			wlr_log_errno(WLR_ERROR, "Allocation failed");
			ok = false;
			buffer->data = NULL;
			// We can't destroy the buffer, or we risk use-after-free in the
			// consumers. We can't allow accesses to buffer->data anymore, so
			// set it to NULL and make subsequent begin_data_ptr_access()
			// calls fail.
		} else {
			memcpy(buffer->saved_data, buffer->data, size);
			buffer->data = buffer->saved_data;
		}
	}

	wlr_buffer_drop(&buffer->base);
	return ok;
}

static const struct wlr_buffer_impl dmabuf_buffer_impl;

static struct wlr_dmabuf_buffer *dmabuf_buffer_from_buffer(
		struct wlr_buffer *buffer) {
	assert(buffer->impl == &dmabuf_buffer_impl);
	return (struct wlr_dmabuf_buffer *)buffer;
}

static void dmabuf_buffer_destroy(struct wlr_buffer *wlr_buffer) {
	struct wlr_dmabuf_buffer *buffer = dmabuf_buffer_from_buffer(wlr_buffer);
	if (buffer->saved) {
		wlr_dmabuf_attributes_finish(&buffer->dmabuf);
	}
	free(buffer);
}

static bool dmabuf_buffer_get_dmabuf(struct wlr_buffer *wlr_buffer,
		struct wlr_dmabuf_attributes *dmabuf) {
	struct wlr_dmabuf_buffer *buffer = dmabuf_buffer_from_buffer(wlr_buffer);
	if (buffer->dmabuf.n_planes == 0) {
		return false;
	}
	*dmabuf = buffer->dmabuf;
	return true;
}

static const struct wlr_buffer_impl dmabuf_buffer_impl = {
	.destroy = dmabuf_buffer_destroy,
	.get_dmabuf = dmabuf_buffer_get_dmabuf,
};

struct wlr_dmabuf_buffer *dmabuf_buffer_create(
		struct wlr_dmabuf_attributes *dmabuf) {
	struct wlr_dmabuf_buffer *buffer = calloc(1, sizeof(*buffer));
	if (buffer == NULL) {
		return NULL;
	}
	wlr_buffer_init(&buffer->base, &dmabuf_buffer_impl,
		dmabuf->width, dmabuf->height);

	buffer->dmabuf = *dmabuf;

	return buffer;
}

bool dmabuf_buffer_drop(struct wlr_dmabuf_buffer *buffer) {
	bool ok = true;

	if (buffer->base.n_locks > 0) {
		struct wlr_dmabuf_attributes saved_dmabuf = {0};
		if (!wlr_dmabuf_attributes_copy(&saved_dmabuf, &buffer->dmabuf)) {
			wlr_log(WLR_ERROR, "Failed to save DMA-BUF");
			ok = false;
			memset(&buffer->dmabuf, 0, sizeof(buffer->dmabuf));
		} else {
			buffer->dmabuf = saved_dmabuf;
			buffer->saved = true;
		}
	}

	wlr_buffer_drop(&buffer->base);
	return ok;
}
