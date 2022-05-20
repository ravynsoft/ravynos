#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <wlr/render/allocator.h>
#include <wlr/util/log.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include "backend/backend.h"
#include "render/allocator/allocator.h"
#include "render/allocator/drm_dumb.h"
#include "render/allocator/gbm.h"
#include "render/allocator/shm.h"
#include "render/wlr_renderer.h"

void wlr_allocator_init(struct wlr_allocator *alloc,
		const struct wlr_allocator_interface *impl, uint32_t buffer_caps) {
	assert(impl && impl->destroy && impl->create_buffer);
	alloc->impl = impl;
	alloc->buffer_caps = buffer_caps;
	wl_signal_init(&alloc->events.destroy);
}

/* Re-open the DRM node to avoid GEM handle ref'counting issues. See:
 * https://gitlab.freedesktop.org/mesa/drm/-/merge_requests/110
 */
static int reopen_drm_node(int drm_fd, bool allow_render_node) {
	if (drmIsMaster(drm_fd)) {
		// Only recent kernels support empty leases
		uint32_t lessee_id;
		int lease_fd = drmModeCreateLease(drm_fd, NULL, 0, 0, &lessee_id);
		if (lease_fd >= 0) {
			return lease_fd;
		} else if (lease_fd != -EINVAL && lease_fd != -EOPNOTSUPP) {
			wlr_log_errno(WLR_ERROR, "drmModeCreateLease failed");
			return -1;
		}
		wlr_log(WLR_DEBUG, "drmModeCreateLease failed, "
			"falling back to plain open");
	}

	char *name = NULL;
	if (allow_render_node) {
		name = drmGetRenderDeviceNameFromFd(drm_fd);
	}
	if (name == NULL) {
		// Either the DRM device has no render node, either the caller wants
		// a primary node
		name = drmGetDeviceNameFromFd2(drm_fd);
		if (name == NULL) {
			wlr_log(WLR_ERROR, "drmGetDeviceNameFromFd2 failed");
			return -1;
		}
	}

	int new_fd = open(name, O_RDWR | O_CLOEXEC);
	if (new_fd < 0) {
		wlr_log_errno(WLR_ERROR, "Failed to open DRM node '%s'", name);
		free(name);
		return -1;
	}

	free(name);

	// If we're using a DRM primary node (e.g. because we're running under the
	// DRM backend, or because we're on split render/display machine), we need
	// to use the legacy DRM authentication mechanism to have the permission to
	// manipulate buffers.
	if (drmGetNodeTypeFromFd(new_fd) == DRM_NODE_PRIMARY) {
		drm_magic_t magic;
		if (drmGetMagic(new_fd, &magic) < 0) {
			wlr_log_errno(WLR_ERROR, "drmGetMagic failed");
			close(new_fd);
			return -1;
		}

		if (drmAuthMagic(drm_fd, magic) < 0) {
			wlr_log_errno(WLR_ERROR, "drmAuthMagic failed");
			close(new_fd);
			return -1;
		}
	}

	return new_fd;
}

struct wlr_allocator *allocator_autocreate_with_drm_fd(
		struct wlr_backend *backend, struct wlr_renderer *renderer,
		int drm_fd) {
	uint32_t backend_caps = backend_get_buffer_caps(backend);
	uint32_t renderer_caps = renderer_get_render_buffer_caps(renderer);

	struct wlr_allocator *alloc = NULL;
	uint32_t gbm_caps = WLR_BUFFER_CAP_DMABUF;
	if ((backend_caps & gbm_caps) && (renderer_caps & gbm_caps)
			&& drm_fd >= 0) {
		wlr_log(WLR_DEBUG, "Trying to create gbm allocator");
		int gbm_fd = reopen_drm_node(drm_fd, true);
		if (gbm_fd < 0) {
			return NULL;
		}
		if ((alloc = wlr_gbm_allocator_create(gbm_fd)) != NULL) {
			return alloc;
		}
		close(gbm_fd);
		wlr_log(WLR_DEBUG, "Failed to create gbm allocator");
	}

	uint32_t shm_caps = WLR_BUFFER_CAP_SHM | WLR_BUFFER_CAP_DATA_PTR;
	if ((backend_caps & shm_caps) && (renderer_caps & shm_caps)) {
		wlr_log(WLR_DEBUG, "Trying to create shm allocator");
		if ((alloc = wlr_shm_allocator_create()) != NULL) {
			return alloc;
		}
		wlr_log(WLR_DEBUG, "Failed to create shm allocator");
	}

	uint32_t drm_caps = WLR_BUFFER_CAP_DMABUF | WLR_BUFFER_CAP_DATA_PTR;
	if ((backend_caps & drm_caps) && (renderer_caps & drm_caps)
			&& drm_fd >= 0 && drmIsMaster(drm_fd)) {
		wlr_log(WLR_DEBUG, "Trying to create drm dumb allocator");
		int dumb_fd = reopen_drm_node(drm_fd, false);
		if (dumb_fd < 0) {
			return NULL;
		}
		if ((alloc = wlr_drm_dumb_allocator_create(dumb_fd)) != NULL) {
			return alloc;
		}
		close(dumb_fd);
		wlr_log(WLR_DEBUG, "Failed to create drm dumb allocator");
	}

	wlr_log(WLR_ERROR, "Failed to create allocator");
	return NULL;
}

struct wlr_allocator *wlr_allocator_autocreate(struct wlr_backend *backend,
		struct wlr_renderer *renderer) {
	// Note, drm_fd may be negative if unavailable
	int drm_fd = wlr_backend_get_drm_fd(backend);
	if (drm_fd < 0) {
		drm_fd = wlr_renderer_get_drm_fd(renderer);
	}
	return allocator_autocreate_with_drm_fd(backend, renderer, drm_fd);
}

void wlr_allocator_destroy(struct wlr_allocator *alloc) {
	if (alloc == NULL) {
		return;
	}
	wl_signal_emit(&alloc->events.destroy, NULL);
	alloc->impl->destroy(alloc);
}

struct wlr_buffer *wlr_allocator_create_buffer(struct wlr_allocator *alloc,
		int width, int height, const struct wlr_drm_format *format) {
	struct wlr_buffer *buffer =
		alloc->impl->create_buffer(alloc, width, height, format);
	if (buffer == NULL) {
		return NULL;
	}
	if (alloc->buffer_caps & WLR_BUFFER_CAP_DATA_PTR) {
		assert(buffer->impl->begin_data_ptr_access &&
			buffer->impl->end_data_ptr_access);
	}
	if (alloc->buffer_caps & WLR_BUFFER_CAP_DMABUF) {
		assert(buffer->impl->get_dmabuf);
	}
	if (alloc->buffer_caps & WLR_BUFFER_CAP_SHM) {
		assert(buffer->impl->get_shm);
	}
	return buffer;
}
