#include <assert.h>
#include <stdlib.h>
#include <wlr/util/log.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include "backend/drm/drm.h"
#include "backend/drm/iface.h"
#include "backend/drm/util.h"

static bool legacy_fb_props_match(struct wlr_drm_fb *fb1,
		struct wlr_drm_fb *fb2) {
	struct wlr_dmabuf_attributes dmabuf1 = {0}, dmabuf2 = {0};
	if (!wlr_buffer_get_dmabuf(fb1->wlr_buf, &dmabuf1) ||
			!wlr_buffer_get_dmabuf(fb2->wlr_buf, &dmabuf2)) {
		return false;
	}

	if (dmabuf1.width != dmabuf2.width ||
			dmabuf1.height != dmabuf2.height ||
			dmabuf1.format != dmabuf2.format ||
			dmabuf1.modifier != dmabuf2.modifier ||
			dmabuf1.n_planes != dmabuf2.n_planes) {
		return false;
	}

	for (int i = 0; i < dmabuf1.n_planes; i++) {
		if (dmabuf1.stride[i] != dmabuf2.stride[i] ||
				dmabuf1.offset[i] != dmabuf2.offset[i]) {
			return false;
		}
	}

	return true;
}

static bool legacy_crtc_test(struct wlr_drm_connector *conn,
		const struct wlr_drm_connector_state *state) {
	struct wlr_drm_crtc *crtc = conn->crtc;

	if ((state->base->committed & WLR_OUTPUT_STATE_BUFFER) && !state->modeset) {
		struct wlr_drm_fb *pending_fb = crtc->primary->pending_fb;

		struct wlr_drm_fb *prev_fb = crtc->primary->queued_fb;
		if (!prev_fb) {
			prev_fb = crtc->primary->current_fb;
		}

		/* Legacy is only guaranteed to be able to display a FB if it's been
		 * allocated the same way as the previous one. */
		if (prev_fb != NULL && !legacy_fb_props_match(prev_fb, pending_fb)) {
			wlr_drm_conn_log(conn, WLR_DEBUG,
				"Cannot change scan-out buffer parameters with legacy KMS API");
			return false;
		}
	}

	return true;
}

static bool legacy_crtc_commit(struct wlr_drm_connector *conn,
		const struct wlr_drm_connector_state *state,
		uint32_t flags, bool test_only) {
	if (!legacy_crtc_test(conn, state)) {
		return false;
	}
	if (test_only) {
		return true;
	}

	struct wlr_drm_backend *drm = conn->backend;
	struct wlr_output *output = &conn->output;
	struct wlr_drm_crtc *crtc = conn->crtc;
	struct wlr_drm_plane *cursor = crtc->cursor;

	uint32_t fb_id = 0;
	if (state->active) {
		struct wlr_drm_fb *fb = plane_get_next_fb(crtc->primary);
		if (fb == NULL) {
			wlr_log(WLR_ERROR, "%s: failed to acquire primary FB",
				conn->output.name);
			return false;
		}
		fb_id = fb->id;
	}

	if (state->modeset) {
		uint32_t *conns = NULL;
		size_t conns_len = 0;
		drmModeModeInfo *mode = NULL;
		if (state->active) {
			conns = &conn->id;
			conns_len = 1;
			mode = (drmModeModeInfo *)&state->mode;
		}

		uint32_t dpms = state->active ? DRM_MODE_DPMS_ON : DRM_MODE_DPMS_OFF;
		if (drmModeConnectorSetProperty(drm->fd, conn->id, conn->props.dpms,
				dpms) != 0) {
			wlr_drm_conn_log_errno(conn, WLR_ERROR,
				"Failed to set DPMS property");
			return false;
		}

		if (drmModeSetCrtc(drm->fd, crtc->id, fb_id, 0, 0,
				conns, conns_len, mode)) {
			wlr_drm_conn_log_errno(conn, WLR_ERROR, "Failed to set CRTC");
			return false;
		}
	}

	if (state->base->committed & WLR_OUTPUT_STATE_GAMMA_LUT) {
		if (!drm_legacy_crtc_set_gamma(drm, crtc,
				state->base->gamma_lut_size, state->base->gamma_lut)) {
			return false;
		}
	}

	if ((state->base->committed & WLR_OUTPUT_STATE_ADAPTIVE_SYNC_ENABLED) &&
			drm_connector_supports_vrr(conn)) {
		if (drmModeObjectSetProperty(drm->fd, crtc->id, DRM_MODE_OBJECT_CRTC,
				crtc->props.vrr_enabled,
				state->base->adaptive_sync_enabled) != 0) {
			wlr_drm_conn_log_errno(conn, WLR_ERROR,
				"drmModeObjectSetProperty(VRR_ENABLED) failed");
			return false;
		}
		output->adaptive_sync_status = state->base->adaptive_sync_enabled ?
			WLR_OUTPUT_ADAPTIVE_SYNC_ENABLED :
			WLR_OUTPUT_ADAPTIVE_SYNC_DISABLED;
		wlr_drm_conn_log(conn, WLR_DEBUG, "VRR %s",
			state->base->adaptive_sync_enabled ? "enabled" : "disabled");
	}

	if (cursor != NULL && drm_connector_is_cursor_visible(conn)) {
		struct wlr_drm_fb *cursor_fb = plane_get_next_fb(cursor);
		if (cursor_fb == NULL) {
			wlr_drm_conn_log(conn, WLR_DEBUG, "Failed to acquire cursor FB");
			return false;
		}

		drmModeFB *drm_fb = drmModeGetFB(drm->fd, cursor_fb->id);
		if (drm_fb == NULL) {
			wlr_drm_conn_log_errno(conn, WLR_DEBUG, "Failed to get cursor "
				"BO handle: drmModeGetFB failed");
			return false;
		}
		uint32_t cursor_handle = drm_fb->handle;
		uint32_t cursor_width = drm_fb->width;
		uint32_t cursor_height = drm_fb->height;
		drmModeFreeFB(drm_fb);

		int ret = drmModeSetCursor(drm->fd, crtc->id, cursor_handle,
			cursor_width, cursor_height);
		int set_cursor_errno = errno;
		if (drmCloseBufferHandle(drm->fd, cursor_handle) != 0) {
			wlr_log_errno(WLR_ERROR, "drmCloseBufferHandle failed");
		}
		if (ret != 0) {
			wlr_drm_conn_log(conn, WLR_DEBUG, "drmModeSetCursor failed: %s",
				strerror(set_cursor_errno));
			return false;
		}

		if (drmModeMoveCursor(drm->fd,
			crtc->id, conn->cursor_x, conn->cursor_y) != 0) {
			wlr_drm_conn_log_errno(conn, WLR_ERROR, "drmModeMoveCursor failed");
			return false;
		}
	} else {
		if (drmModeSetCursor(drm->fd, crtc->id, 0, 0, 0)) {
			wlr_drm_conn_log_errno(conn, WLR_DEBUG, "drmModeSetCursor failed");
			return false;
		}
	}

	if (flags & DRM_MODE_PAGE_FLIP_EVENT) {
		if (drmModePageFlip(drm->fd, crtc->id, fb_id,
				DRM_MODE_PAGE_FLIP_EVENT, drm)) {
			wlr_drm_conn_log_errno(conn, WLR_ERROR, "drmModePageFlip failed");
			return false;
		}
	}

	return true;
}

static void fill_empty_gamma_table(size_t size,
		uint16_t *r, uint16_t *g, uint16_t *b) {
	assert(0xFFFF < UINT64_MAX / (size - 1));
	for (uint32_t i = 0; i < size; ++i) {
		uint16_t val = (uint64_t)0xFFFF * i / (size - 1);
		r[i] = g[i] = b[i] = val;
	}
}

bool drm_legacy_crtc_set_gamma(struct wlr_drm_backend *drm,
		struct wlr_drm_crtc *crtc, size_t size, uint16_t *lut) {
	uint16_t *linear_lut = NULL;
	if (size == 0) {
		// The legacy interface doesn't offer a way to reset the gamma LUT
		size = drm_crtc_get_gamma_lut_size(drm, crtc);
		if (size == 0) {
			return false;
		}

		linear_lut = malloc(3 * size * sizeof(uint16_t));
		if (linear_lut == NULL) {
			wlr_log_errno(WLR_ERROR, "Allocation failed");
			return false;
		}
		fill_empty_gamma_table(size, linear_lut, linear_lut + size,
			linear_lut + 2 * size);

		lut = linear_lut;
	}

	uint16_t *r = lut, *g = lut + size, *b = lut + 2 * size;
	if (drmModeCrtcSetGamma(drm->fd, crtc->id, size, r, g, b) != 0) {
		wlr_log_errno(WLR_ERROR, "Failed to set gamma LUT on CRTC %"PRIu32,
			crtc->id);
		free(linear_lut);
		return false;
	}

	free(linear_lut);
	return true;
}

const struct wlr_drm_interface legacy_iface = {
	.crtc_commit = legacy_crtc_commit,
};
