#include <stdlib.h>
#include <wlr/util/log.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include "backend/drm/drm.h"
#include "backend/drm/iface.h"
#include "backend/drm/util.h"

struct atomic {
	drmModeAtomicReq *req;
	bool failed;
};

static void atomic_begin(struct atomic *atom) {
	memset(atom, 0, sizeof(*atom));

	atom->req = drmModeAtomicAlloc();
	if (!atom->req) {
		wlr_log_errno(WLR_ERROR, "Allocation failed");
		atom->failed = true;
		return;
	}
}

static bool atomic_commit(struct atomic *atom,
		struct wlr_drm_connector *conn, uint32_t flags) {
	struct wlr_drm_backend *drm = conn->backend;
	if (atom->failed) {
		return false;
	}

	int ret = drmModeAtomicCommit(drm->fd, atom->req, flags, drm);
	if (ret != 0) {
		wlr_drm_conn_log_errno(conn,
			(flags & DRM_MODE_ATOMIC_TEST_ONLY) ? WLR_DEBUG : WLR_ERROR,
			"Atomic %s failed (%s)",
			(flags & DRM_MODE_ATOMIC_TEST_ONLY) ? "test" : "commit",
			(flags & DRM_MODE_ATOMIC_ALLOW_MODESET) ? "modeset" : "pageflip");
		return false;
	}

	return true;
}

static void atomic_finish(struct atomic *atom) {
	drmModeAtomicFree(atom->req);
}

static void atomic_add(struct atomic *atom, uint32_t id, uint32_t prop, uint64_t val) {
	if (!atom->failed && drmModeAtomicAddProperty(atom->req, id, prop, val) < 0) {
		wlr_log_errno(WLR_ERROR, "Failed to add atomic DRM property");
		atom->failed = true;
	}
}

static bool create_mode_blob(struct wlr_drm_backend *drm,
		struct wlr_drm_connector *conn,
		const struct wlr_drm_connector_state *state, uint32_t *blob_id) {
	if (!state->active) {
		*blob_id = 0;
		return true;
	}

	if (drmModeCreatePropertyBlob(drm->fd, &state->mode,
			sizeof(drmModeModeInfo), blob_id)) {
		wlr_log_errno(WLR_ERROR, "Unable to create mode property blob");
		return false;
	}

	return true;
}

static bool create_gamma_lut_blob(struct wlr_drm_backend *drm,
		size_t size, const uint16_t *lut, uint32_t *blob_id) {
	if (size == 0) {
		*blob_id = 0;
		return true;
	}

	struct drm_color_lut *gamma = malloc(size * sizeof(struct drm_color_lut));
	if (gamma == NULL) {
		wlr_log(WLR_ERROR, "Failed to allocate gamma table");
		return false;
	}

	const uint16_t *r = lut;
	const uint16_t *g = lut + size;
	const uint16_t *b = lut + 2 * size;
	for (size_t i = 0; i < size; i++) {
		gamma[i].red = r[i];
		gamma[i].green = g[i];
		gamma[i].blue = b[i];
	}

	if (drmModeCreatePropertyBlob(drm->fd, gamma,
			size * sizeof(struct drm_color_lut), blob_id) != 0) {
		wlr_log_errno(WLR_ERROR, "Unable to create gamma LUT property blob");
		free(gamma);
		return false;
	}
	free(gamma);

	return true;
}

static void commit_blob(struct wlr_drm_backend *drm,
		uint32_t *current, uint32_t next) {
	if (*current == next) {
		return;
	}
	if (*current != 0) {
		drmModeDestroyPropertyBlob(drm->fd, *current);
	}
	*current = next;
}

static void rollback_blob(struct wlr_drm_backend *drm,
		uint32_t *current, uint32_t next) {
	if (*current == next) {
		return;
	}
	if (next != 0) {
		drmModeDestroyPropertyBlob(drm->fd, next);
	}
}

static void plane_disable(struct atomic *atom, struct wlr_drm_plane *plane) {
	uint32_t id = plane->id;
	const union wlr_drm_plane_props *props = &plane->props;
	atomic_add(atom, id, props->fb_id, 0);
	atomic_add(atom, id, props->crtc_id, 0);
}

static void set_plane_props(struct atomic *atom, struct wlr_drm_backend *drm,
		struct wlr_drm_plane *plane, uint32_t crtc_id, int32_t x, int32_t y) {
	uint32_t id = plane->id;
	const union wlr_drm_plane_props *props = &plane->props;
	struct wlr_drm_fb *fb = plane_get_next_fb(plane);
	if (fb == NULL) {
		wlr_log(WLR_ERROR, "Failed to acquire FB");
		goto error;
	}

	uint32_t width = fb->wlr_buf->width;
	uint32_t height = fb->wlr_buf->height;

	// The src_* properties are in 16.16 fixed point
	atomic_add(atom, id, props->src_x, 0);
	atomic_add(atom, id, props->src_y, 0);
	atomic_add(atom, id, props->src_w, (uint64_t)width << 16);
	atomic_add(atom, id, props->src_h, (uint64_t)height << 16);
	atomic_add(atom, id, props->crtc_w, width);
	atomic_add(atom, id, props->crtc_h, height);
	atomic_add(atom, id, props->fb_id, fb->id);
	atomic_add(atom, id, props->crtc_id, crtc_id);
	atomic_add(atom, id, props->crtc_x, (uint64_t)x);
	atomic_add(atom, id, props->crtc_y, (uint64_t)y);

	return;

error:
	wlr_log(WLR_ERROR, "Failed to set plane %"PRIu32" properties", plane->id);
	atom->failed = true;
}

static bool atomic_crtc_commit(struct wlr_drm_connector *conn,
		const struct wlr_drm_connector_state *state, uint32_t flags,
		bool test_only) {
	struct wlr_drm_backend *drm = conn->backend;
	struct wlr_output *output = &conn->output;
	struct wlr_drm_crtc *crtc = conn->crtc;

	bool modeset = state->modeset;
	bool active = state->active;

	uint32_t mode_id = crtc->mode_id;
	if (modeset) {
		if (!create_mode_blob(drm, conn, state, &mode_id)) {
			return false;
		}
	}

	uint32_t gamma_lut = crtc->gamma_lut;
	if (state->base->committed & WLR_OUTPUT_STATE_GAMMA_LUT) {
		// Fallback to legacy gamma interface when gamma properties are not
		// available (can happen on older Intel GPUs that support gamma but not
		// degamma).
		if (crtc->props.gamma_lut == 0) {
			if (!drm_legacy_crtc_set_gamma(drm, crtc,
					state->base->gamma_lut_size,
					state->base->gamma_lut)) {
				return false;
			}
		} else {
			if (!create_gamma_lut_blob(drm, state->base->gamma_lut_size,
					state->base->gamma_lut, &gamma_lut)) {
				return false;
			}
		}
	}

	uint32_t fb_damage_clips = 0;
	if ((state->base->committed & WLR_OUTPUT_STATE_DAMAGE) &&
			pixman_region32_not_empty((pixman_region32_t *)&state->base->damage) &&
			crtc->primary->props.fb_damage_clips != 0) {
		int rects_len;
		const pixman_box32_t *rects = pixman_region32_rectangles(
			(pixman_region32_t *)&state->base->damage, &rects_len);
		if (drmModeCreatePropertyBlob(drm->fd, rects,
				sizeof(*rects) * rects_len, &fb_damage_clips) != 0) {
			wlr_log_errno(WLR_ERROR, "Failed to create FB_DAMAGE_CLIPS property blob");
		}
	}

	bool prev_vrr_enabled =
		output->adaptive_sync_status == WLR_OUTPUT_ADAPTIVE_SYNC_ENABLED;
	bool vrr_enabled = prev_vrr_enabled;
	if ((state->base->committed & WLR_OUTPUT_STATE_ADAPTIVE_SYNC_ENABLED) &&
			drm_connector_supports_vrr(conn)) {
		vrr_enabled = state->base->adaptive_sync_enabled;
	}

	if (test_only) {
		flags |= DRM_MODE_ATOMIC_TEST_ONLY;
	}
	if (modeset) {
		flags |= DRM_MODE_ATOMIC_ALLOW_MODESET;
	} else if (!test_only) {
		flags |= DRM_MODE_ATOMIC_NONBLOCK;
	}

	struct atomic atom;
	atomic_begin(&atom);
	atomic_add(&atom, conn->id, conn->props.crtc_id, active ? crtc->id : 0);
	if (modeset && active && conn->props.link_status != 0) {
		atomic_add(&atom, conn->id, conn->props.link_status,
			DRM_MODE_LINK_STATUS_GOOD);
	}
	atomic_add(&atom, crtc->id, crtc->props.mode_id, mode_id);
	atomic_add(&atom, crtc->id, crtc->props.active, active);
	if (active) {
		if (crtc->props.gamma_lut != 0) {
			atomic_add(&atom, crtc->id, crtc->props.gamma_lut, gamma_lut);
		}
		if (crtc->props.vrr_enabled != 0) {
			atomic_add(&atom, crtc->id, crtc->props.vrr_enabled, vrr_enabled);
		}
		set_plane_props(&atom, drm, crtc->primary, crtc->id, 0, 0);
		if (crtc->primary->props.fb_damage_clips != 0) {
			atomic_add(&atom, crtc->primary->id,
				crtc->primary->props.fb_damage_clips, fb_damage_clips);
		}
		if (crtc->cursor) {
			if (drm_connector_is_cursor_visible(conn)) {
				set_plane_props(&atom, drm, crtc->cursor, crtc->id,
					conn->cursor_x, conn->cursor_y);
			} else {
				plane_disable(&atom, crtc->cursor);
			}
		}
	} else {
		plane_disable(&atom, crtc->primary);
		if (crtc->cursor) {
			plane_disable(&atom, crtc->cursor);
		}
	}

	bool ok = atomic_commit(&atom, conn, flags);
	atomic_finish(&atom);

	if (ok && !test_only) {
		commit_blob(drm, &crtc->mode_id, mode_id);
		commit_blob(drm, &crtc->gamma_lut, gamma_lut);

		if (vrr_enabled != prev_vrr_enabled) {
			output->adaptive_sync_status = vrr_enabled ?
				WLR_OUTPUT_ADAPTIVE_SYNC_ENABLED :
				WLR_OUTPUT_ADAPTIVE_SYNC_DISABLED;
			wlr_drm_conn_log(conn, WLR_DEBUG, "VRR %s",
				vrr_enabled ? "enabled" : "disabled");
		}
	} else {
		rollback_blob(drm, &crtc->mode_id, mode_id);
		rollback_blob(drm, &crtc->gamma_lut, gamma_lut);
	}

	if (fb_damage_clips != 0 &&
			drmModeDestroyPropertyBlob(drm->fd, fb_damage_clips) != 0) {
		wlr_log_errno(WLR_ERROR, "Failed to destroy FB_DAMAGE_CLIPS property blob");
	}

	return ok;
}

const struct wlr_drm_interface atomic_iface = {
	.crtc_commit = atomic_crtc_commit,
};
