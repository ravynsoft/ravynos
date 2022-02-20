/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_BACKEND_DRM_H
#define WLR_BACKEND_DRM_H

#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/backend/session.h>
#include <wlr/types/wlr_output.h>

struct wlr_drm_backend;

struct wlr_drm_lease {
	int fd;
	uint32_t lessee_id;
	struct wlr_drm_backend *backend;

	struct {
		struct wl_signal destroy;
	} events;

	void *data;
};

/**
 * Creates a DRM backend using the specified GPU file descriptor (typically from
 * a device node in /dev/dri).
 *
 * To slave this to another DRM backend, pass it as the parent (which _must_ be
 * a DRM backend, other kinds of backends raise SIGABRT).
 */
struct wlr_backend *wlr_drm_backend_create(struct wl_display *display,
	struct wlr_session *session, struct wlr_device *dev,
	struct wlr_backend *parent);

bool wlr_backend_is_drm(struct wlr_backend *backend);
bool wlr_output_is_drm(struct wlr_output *output);

/**
 * Get the KMS connector object ID.
 */
uint32_t wlr_drm_connector_get_id(struct wlr_output *output);

/**
 * Tries to open non-master DRM FD. The compositor must not call `drmSetMaster`
 * on the returned FD.
 * Returns a valid opened DRM FD, or -1 on error.
 */
int wlr_drm_backend_get_non_master_fd(struct wlr_backend *backend);

/**
 * Leases the given outputs to the caller. The outputs must be from the
 * associated DRM backend.
 *
 * Returns NULL on error.
 */
struct wlr_drm_lease *wlr_drm_create_lease(struct wlr_output **outputs,
	size_t n_outputs, int *lease_fd);

/**
 * Terminates and destroys a given lease.
 *
 * The outputs will be owned again by the backend.
 */
void wlr_drm_lease_terminate(struct wlr_drm_lease *lease);

/**
 * Add mode to the list of available modes
 */
typedef struct _drmModeModeInfo drmModeModeInfo;
struct wlr_output_mode *wlr_drm_connector_add_mode(struct wlr_output *output,
	const drmModeModeInfo *mode);

/**
 * Get the connector's panel orientation.
 *
 * On some devices the panel is mounted in the casing in such a way that the
 * top side of the panel does not match with the top side of the device. This
 * function returns the output transform which needs to be applied to compensate
 * for this.
 */
enum wl_output_transform wlr_drm_connector_get_panel_orientation(
	struct wlr_output *output);

#endif
