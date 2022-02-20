#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <drm_fourcc.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <wlr/render/allocator.h>
#include <wlr/render/drm_format_set.h>
#include <wlr/util/log.h>
#include <xf86drm.h>

#include "render/allocator/gbm.h"
#include "render/drm_format_set.h"

static const struct wlr_buffer_impl buffer_impl;

static struct wlr_gbm_buffer *get_gbm_buffer_from_buffer(
		struct wlr_buffer *buffer) {
	assert(buffer->impl == &buffer_impl);
	return (struct wlr_gbm_buffer *)buffer;
}

static bool export_gbm_bo(struct gbm_bo *bo,
		struct wlr_dmabuf_attributes *out) {
	struct wlr_dmabuf_attributes attribs = {0};

	attribs.n_planes = gbm_bo_get_plane_count(bo);
	if (attribs.n_planes > WLR_DMABUF_MAX_PLANES) {
		wlr_log(WLR_ERROR, "GBM BO contains too many planes (%d)",
			attribs.n_planes);
		return false;
	}

	attribs.width = gbm_bo_get_width(bo);
	attribs.height = gbm_bo_get_height(bo);
	attribs.format = gbm_bo_get_format(bo);
	attribs.modifier = gbm_bo_get_modifier(bo);

	int i;
	int32_t handle = -1;
	for (i = 0; i < attribs.n_planes; ++i) {
#if HAS_GBM_BO_GET_FD_FOR_PLANE
		(void)handle;

		attribs.fd[i] = gbm_bo_get_fd_for_plane(bo, i);
		if (attribs.fd[i] < 0) {
			wlr_log(WLR_ERROR, "gbm_bo_get_fd_for_plane failed");
			goto error_fd;
		}
#else
		// GBM is lacking a function to get a FD for a given plane. Instead,
		// check all planes have the same handle. We can't use
		// drmPrimeHandleToFD because that messes up handle ref'counting in
		// the user-space driver.
		union gbm_bo_handle plane_handle = gbm_bo_get_handle_for_plane(bo, i);
		if (plane_handle.s32 < 0) {
			wlr_log(WLR_ERROR, "gbm_bo_get_handle_for_plane failed");
			goto error_fd;
		}
		if (i == 0) {
			handle = plane_handle.s32;
		} else if (plane_handle.s32 != handle) {
			wlr_log(WLR_ERROR, "Failed to export GBM BO: "
				"all planes don't have the same GEM handle");
			goto error_fd;
		}

		attribs.fd[i] = gbm_bo_get_fd(bo);
		if (attribs.fd[i] < 0) {
			wlr_log(WLR_ERROR, "gbm_bo_get_fd failed");
			goto error_fd;
		}
#endif

		attribs.offset[i] = gbm_bo_get_offset(bo, i);
		attribs.stride[i] = gbm_bo_get_stride_for_plane(bo, i);
	}

	memcpy(out, &attribs, sizeof(attribs));
	return true;

error_fd:
	for (int j = 0; j < i; ++j) {
		close(attribs.fd[j]);
	}
	return false;
}

static struct wlr_gbm_buffer *create_buffer(struct wlr_gbm_allocator *alloc,
		int width, int height, const struct wlr_drm_format *format) {
	struct gbm_device *gbm_device = alloc->gbm_device;

	assert(format->len > 0);

	bool has_modifier = true;
	uint64_t fallback_modifier = DRM_FORMAT_MOD_INVALID;
	struct gbm_bo *bo = gbm_bo_create_with_modifiers(gbm_device, width, height,
		format->format, format->modifiers, format->len);
	if (bo == NULL) {
		uint32_t usage = GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING;
		if (format->len == 1 &&
				format->modifiers[0] == DRM_FORMAT_MOD_LINEAR) {
			usage |= GBM_BO_USE_LINEAR;
			fallback_modifier = DRM_FORMAT_MOD_LINEAR;
		} else if (!wlr_drm_format_has(format, DRM_FORMAT_MOD_INVALID)) {
			// If the format doesn't accept an implicit modifier, bail out.
			wlr_log(WLR_ERROR, "gbm_bo_create_with_modifiers failed");
			return NULL;
		}
		bo = gbm_bo_create(gbm_device, width, height, format->format, usage);
		has_modifier = false;
	}
	if (bo == NULL) {
		wlr_log(WLR_ERROR, "gbm_bo_create failed");
		return NULL;
	}

	struct wlr_gbm_buffer *buffer = calloc(1, sizeof(*buffer));
	if (buffer == NULL) {
		gbm_bo_destroy(bo);
		return NULL;
	}
	wlr_buffer_init(&buffer->base, &buffer_impl, width, height);
	buffer->gbm_bo = bo;
	wl_list_insert(&alloc->buffers, &buffer->link);

	if (!export_gbm_bo(bo, &buffer->dmabuf)) {
		free(buffer);
		gbm_bo_destroy(bo);
		return NULL;
	}

	// If the buffer has been allocated with an implicit modifier, make sure we
	// don't populate the modifier field: other parts of the stack may not
	// understand modifiers, and they can't strip the modifier.
	if (!has_modifier) {
		buffer->dmabuf.modifier = fallback_modifier;
	}

	wlr_log(WLR_DEBUG, "Allocated %dx%d GBM buffer (format 0x%"PRIX32", "
		"modifier 0x%"PRIX64")", buffer->base.width, buffer->base.height,
		buffer->dmabuf.format, buffer->dmabuf.modifier);

	return buffer;
}

static void buffer_destroy(struct wlr_buffer *wlr_buffer) {
	struct wlr_gbm_buffer *buffer =
		get_gbm_buffer_from_buffer(wlr_buffer);
	wlr_dmabuf_attributes_finish(&buffer->dmabuf);
	if (buffer->gbm_bo != NULL) {
		gbm_bo_destroy(buffer->gbm_bo);
	}
	wl_list_remove(&buffer->link);
	free(buffer);
}

static bool buffer_get_dmabuf(struct wlr_buffer *wlr_buffer,
		struct wlr_dmabuf_attributes *attribs) {
	struct wlr_gbm_buffer *buffer =
		get_gbm_buffer_from_buffer(wlr_buffer);
	memcpy(attribs, &buffer->dmabuf, sizeof(buffer->dmabuf));
	return true;
}

static const struct wlr_buffer_impl buffer_impl = {
	.destroy = buffer_destroy,
	.get_dmabuf = buffer_get_dmabuf,
};

static const struct wlr_allocator_interface allocator_impl;

static struct wlr_gbm_allocator *get_gbm_alloc_from_alloc(
		struct wlr_allocator *alloc) {
	assert(alloc->impl == &allocator_impl);
	return (struct wlr_gbm_allocator *)alloc;
}

struct wlr_allocator *wlr_gbm_allocator_create(int fd) {
	uint64_t cap;
	if (drmGetCap(fd, DRM_CAP_PRIME, &cap) ||
			!(cap & DRM_PRIME_CAP_EXPORT)) {
		wlr_log(WLR_ERROR, "PRIME export not supported");
		return NULL;
	}

	struct wlr_gbm_allocator *alloc = calloc(1, sizeof(*alloc));
	if (alloc == NULL) {
		return NULL;
	}
	wlr_allocator_init(&alloc->base, &allocator_impl, WLR_BUFFER_CAP_DMABUF);

	alloc->fd = fd;
	wl_list_init(&alloc->buffers);

	alloc->gbm_device = gbm_create_device(fd);
	if (alloc->gbm_device == NULL) {
		wlr_log(WLR_ERROR, "gbm_create_device failed");
		free(alloc);
		return NULL;
	}

	wlr_log(WLR_DEBUG, "Created GBM allocator with backend %s",
		gbm_device_get_backend_name(alloc->gbm_device));
	char *drm_name = drmGetDeviceNameFromFd2(fd);
	wlr_log(WLR_DEBUG, "Using DRM node %s", drm_name);
	free(drm_name);

	return &alloc->base;
}

static void allocator_destroy(struct wlr_allocator *wlr_alloc) {
	struct wlr_gbm_allocator *alloc = get_gbm_alloc_from_alloc(wlr_alloc);

	// The gbm_bo objects need to be destroyed before the gbm_device
	struct wlr_gbm_buffer *buf, *buf_tmp;
	wl_list_for_each_safe(buf, buf_tmp, &alloc->buffers, link) {
		gbm_bo_destroy(buf->gbm_bo);
		buf->gbm_bo = NULL;
		wl_list_remove(&buf->link);
		wl_list_init(&buf->link);
	}

	gbm_device_destroy(alloc->gbm_device);
	close(alloc->fd);
	free(alloc);
}

static struct wlr_buffer *allocator_create_buffer(
		struct wlr_allocator *wlr_alloc, int width, int height,
		const struct wlr_drm_format *format) {
	struct wlr_gbm_allocator *alloc = get_gbm_alloc_from_alloc(wlr_alloc);
	struct wlr_gbm_buffer *buffer = create_buffer(alloc, width, height, format);
	if (buffer == NULL) {
		return NULL;
	}
	return &buffer->base;
}

static const struct wlr_allocator_interface allocator_impl = {
	.destroy = allocator_destroy,
	.create_buffer = allocator_create_buffer,
};
