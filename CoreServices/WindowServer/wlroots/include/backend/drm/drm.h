#ifndef BACKEND_DRM_DRM_H
#define BACKEND_DRM_DRM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wlr/backend/drm.h>
#include <wlr/backend/session.h>
#include <wlr/render/drm_format_set.h>
#include <xf86drmMode.h>
#include "backend/drm/iface.h"
#include "backend/drm/properties.h"
#include "backend/drm/renderer.h"

struct wlr_drm_plane {
	uint32_t type;
	uint32_t id;

	/* Only initialized on multi-GPU setups */
	struct wlr_drm_surface mgpu_surf;

	/* Buffer to be submitted to the kernel on the next page-flip */
	struct wlr_drm_fb *pending_fb;
	/* Buffer submitted to the kernel, will be presented on next vblank */
	struct wlr_drm_fb *queued_fb;
	/* Buffer currently displayed on screen */
	struct wlr_drm_fb *current_fb;

	struct wlr_drm_format_set formats;

	union wlr_drm_plane_props props;
};

struct wlr_drm_crtc {
	uint32_t id;
	struct wlr_drm_lease *lease;

	// Atomic modesetting only
	uint32_t mode_id;
	uint32_t gamma_lut;

	// Legacy only
	drmModeCrtc *legacy_crtc;

	struct wlr_drm_plane *primary;
	struct wlr_drm_plane *cursor;

	union wlr_drm_crtc_props props;
};

struct wlr_drm_backend {
	struct wlr_backend backend;

	struct wlr_drm_backend *parent;
	const struct wlr_drm_interface *iface;
	clockid_t clock;
	bool addfb2_modifiers;

	int fd;
	char *name;
	struct wlr_device *dev;

	size_t num_crtcs;
	struct wlr_drm_crtc *crtcs;

	struct wl_display *display;
	struct wl_event_source *drm_event;

	struct wl_listener display_destroy;
	struct wl_listener session_destroy;
	struct wl_listener session_active;
	struct wl_listener parent_destroy;
	struct wl_listener dev_change;
	struct wl_listener dev_remove;

	struct wl_list fbs; // wlr_drm_fb.link
	struct wl_list outputs;

	/* Only initialized on multi-GPU setups */
	struct wlr_drm_renderer mgpu_renderer;

	struct wlr_session *session;

	uint64_t cursor_width, cursor_height;

	struct wlr_drm_format_set mgpu_formats;
};

enum wlr_drm_connector_status {
	// Connector is available but no output is plugged in
	WLR_DRM_CONN_DISCONNECTED,
	// An output just has been plugged in and is waiting for a modeset
	WLR_DRM_CONN_NEEDS_MODESET,
	WLR_DRM_CONN_CLEANUP,
	WLR_DRM_CONN_CONNECTED,
};

struct wlr_drm_mode {
	struct wlr_output_mode wlr_mode;
	drmModeModeInfo drm_mode;
};

struct wlr_drm_connector_state {
	const struct wlr_output_state *base;
	bool modeset;
	bool active;
	drmModeModeInfo mode;
};

struct wlr_drm_connector {
	struct wlr_output output; // only valid if status != DISCONNECTED

	struct wlr_drm_backend *backend;
	char name[24];
	enum wlr_drm_connector_status status;
	bool desired_enabled;
	uint32_t id;
	struct wlr_drm_lease *lease;

	struct wlr_drm_crtc *crtc;
	uint32_t possible_crtcs;

	union wlr_drm_connector_props props;

	bool cursor_enabled;
	int cursor_x, cursor_y;
	int cursor_width, cursor_height;
	int cursor_hotspot_x, cursor_hotspot_y;

	struct wl_list link;

	/* CRTC ID if a page-flip is pending, zero otherwise.
	 *
	 * We've asked for a state change in the kernel, and yet to receive a
	 * notification for its completion. Currently, the kernel only has a
	 * queue length of 1, and no way to modify your submissions after
	 * they're sent.
	 */
	uint32_t pending_page_flip_crtc;
};

struct wlr_drm_backend *get_drm_backend_from_backend(
	struct wlr_backend *wlr_backend);
bool check_drm_features(struct wlr_drm_backend *drm);
bool init_drm_resources(struct wlr_drm_backend *drm);
void finish_drm_resources(struct wlr_drm_backend *drm);
void scan_drm_connectors(struct wlr_drm_backend *state,
	struct wlr_device_hotplug_event *event);
void scan_drm_leases(struct wlr_drm_backend *drm);
int handle_drm_event(int fd, uint32_t mask, void *data);
void destroy_drm_connector(struct wlr_drm_connector *conn);
bool drm_connector_commit_state(struct wlr_drm_connector *conn,
	const struct wlr_output_state *state);
bool drm_connector_is_cursor_visible(struct wlr_drm_connector *conn);
bool drm_connector_supports_vrr(struct wlr_drm_connector *conn);
size_t drm_crtc_get_gamma_lut_size(struct wlr_drm_backend *drm,
	struct wlr_drm_crtc *crtc);
void drm_lease_destroy(struct wlr_drm_lease *lease);

struct wlr_drm_fb *plane_get_next_fb(struct wlr_drm_plane *plane);

#define wlr_drm_conn_log(conn, verb, fmt, ...) \
	wlr_log(verb, "connector %s: " fmt, conn->name, ##__VA_ARGS__)
#define wlr_drm_conn_log_errno(conn, verb, fmt, ...) \
	wlr_log_errno(verb, "connector %s: " fmt, conn->name, ##__VA_ARGS__)

#endif
