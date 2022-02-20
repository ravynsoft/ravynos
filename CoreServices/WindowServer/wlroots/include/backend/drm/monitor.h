#ifndef BACKEND_DRM_MONITOR_H
#define BACKEND_DRM_MONITOR_H

#include <wlr/backend/drm.h>

/**
 * Helper to create new DRM sub-backends on GPU hotplug.
 */
struct wlr_drm_backend_monitor {
	struct wlr_backend *multi;
	struct wlr_backend *primary_drm;
	struct wlr_session *session;

	struct wl_listener multi_destroy;
	struct wl_listener primary_drm_destroy;
	struct wl_listener session_destroy;
	struct wl_listener session_add_drm_card;
};

struct wlr_drm_backend_monitor *drm_backend_monitor_create(
	struct wlr_backend *multi, struct wlr_backend *primary_drm,
	struct wlr_session *session);

#endif
