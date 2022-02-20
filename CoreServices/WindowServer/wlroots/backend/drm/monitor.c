#include <wlr/util/log.h>
#include <stdlib.h>
#include "backend/drm/monitor.h"
#include "backend/multi.h"
#include "backend/session/session.h"

static void drm_backend_monitor_destroy(struct wlr_drm_backend_monitor* monitor) {
	wl_list_remove(&monitor->session_add_drm_card.link);
	wl_list_remove(&monitor->session_destroy.link);
	wl_list_remove(&monitor->primary_drm_destroy.link);
	wl_list_remove(&monitor->multi_destroy.link);
	free(monitor);
}

static void handle_add_drm_card(struct wl_listener *listener, void *data) {
	struct wlr_session_add_event *event = data;
	struct wlr_drm_backend_monitor *backend_monitor =
		wl_container_of(listener, backend_monitor, session_add_drm_card);

	struct wlr_device *dev =
		session_open_if_kms(backend_monitor->session, event->path);
	if (!dev) {
		wlr_log(WLR_ERROR, "Unable to open %s as DRM device", event->path);
		return;
	}

	wlr_log(WLR_DEBUG, "Creating DRM backend for %s after hotplug", event->path);
	struct wlr_backend *child_drm = wlr_drm_backend_create(
		backend_monitor->session->display, backend_monitor->session,
		dev, backend_monitor->primary_drm);
	if (!child_drm) {
		wlr_log(WLR_ERROR, "Failed to create DRM backend after hotplug");
		return;
	}

	if (!wlr_multi_backend_add(backend_monitor->multi, child_drm)) {
		wlr_log(WLR_ERROR, "Failed to add new drm backend to multi backend");
		wlr_backend_destroy(child_drm);
		return;
	}

	if (!wlr_backend_start(child_drm)) {
		wlr_log(WLR_ERROR, "Failed to start new child DRM backend");
		wlr_backend_destroy(child_drm);
	}
}

static void handle_session_destroy(struct wl_listener *listener, void *data) {
	struct wlr_drm_backend_monitor *backend_monitor =
		wl_container_of(listener, backend_monitor, session_destroy);
	drm_backend_monitor_destroy(backend_monitor);
}

static void handle_primary_drm_destroy(struct wl_listener *listener, void *data) {
	struct wlr_drm_backend_monitor *backend_monitor =
		wl_container_of(listener, backend_monitor, primary_drm_destroy);
	drm_backend_monitor_destroy(backend_monitor);
}

static void handle_multi_destroy(struct wl_listener *listener, void *data) {
	struct wlr_drm_backend_monitor *backend_monitor =
		wl_container_of(listener, backend_monitor, multi_destroy);
	drm_backend_monitor_destroy(backend_monitor);
}

struct wlr_drm_backend_monitor *drm_backend_monitor_create(
		struct wlr_backend *multi,
		struct wlr_backend *primary_drm,
		struct wlr_session *session) {
	struct wlr_drm_backend_monitor *monitor =
		calloc(1, sizeof(struct wlr_drm_backend_monitor));
	if (!monitor) {
		wlr_log_errno(WLR_ERROR, "Allocation failed");
		return NULL;
	}

	monitor->multi = multi;
	monitor->primary_drm = primary_drm;
	monitor->session = session;

	monitor->session_add_drm_card.notify = handle_add_drm_card;
	wl_signal_add(&session->events.add_drm_card, &monitor->session_add_drm_card);

	monitor->session_destroy.notify = handle_session_destroy;
	wl_signal_add(&session->events.destroy, &monitor->session_destroy);

	monitor->primary_drm_destroy.notify = handle_primary_drm_destroy;
	wl_signal_add(&primary_drm->events.destroy, &monitor->primary_drm_destroy);

	monitor->multi_destroy.notify = handle_multi_destroy;
	wl_signal_add(&multi->events.destroy, &monitor->multi_destroy);

	return monitor;
}
