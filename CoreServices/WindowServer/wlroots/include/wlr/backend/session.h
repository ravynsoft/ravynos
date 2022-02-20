#ifndef WLR_BACKEND_SESSION_H
#define WLR_BACKEND_SESSION_H

#include <libudev.h>
#include <stdbool.h>
#include <sys/types.h>
#include <wayland-server-core.h>

struct libseat;

struct wlr_device {
	int fd;
	int device_id;
	dev_t dev;
	struct wl_list link;

	struct {
		struct wl_signal change; // struct wlr_device_change_event
		struct wl_signal remove;
	} events;
};

struct wlr_session {
	/*
	 * Signal for when the session becomes active/inactive.
	 * It's called when we swap virtual terminal.
	 */
	bool active;

	/*
	 * 0 if virtual terminals are not supported
	 * i.e. seat != "seat0"
	 */
	unsigned vtnr;
	char seat[256];

	struct udev *udev;
	struct udev_monitor *mon;
	struct wl_event_source *udev_event;

	struct libseat *seat_handle;
	struct wl_event_source *libseat_event;

	struct wl_list devices;

	struct wl_display *display;
	struct wl_listener display_destroy;

	struct {
		struct wl_signal active;
		struct wl_signal add_drm_card; // struct wlr_session_add_event
		struct wl_signal destroy;
	} events;
};

struct wlr_session_add_event {
	const char *path;
};

enum wlr_device_change_type {
	WLR_DEVICE_HOTPLUG = 1,
	WLR_DEVICE_LEASE,
};

struct wlr_device_hotplug_event {
	uint32_t connector_id;
	uint32_t prop_id;
};

struct wlr_device_change_event {
	enum wlr_device_change_type type;
	union {
		struct wlr_device_hotplug_event hotplug;
	};
};

/*
 * Opens a session, taking control of the current virtual terminal.
 * This should not be called if another program is already in control
 * of the terminal (Xorg, another Wayland compositor, etc.).
 *
 * If libseat support is not enabled, or if a standalone backend is to be used,
 * then you must have CAP_SYS_ADMIN or be root. It is safe to drop privileges
 * after this is called.
 *
 * Returns NULL on error.
 */
struct wlr_session *wlr_session_create(struct wl_display *disp);

/*
 * Closes a previously opened session and restores the virtual terminal.
 * You should call wlr_session_close_file on each files you opened
 * with wlr_session_open_file before you call this.
 */
void wlr_session_destroy(struct wlr_session *session);

/*
 * Opens the file at path.
 * This can only be used to open DRM or evdev (input) devices.
 *
 * When the session becomes inactive:
 * - DRM files lose their DRM master status
 * - evdev files become invalid and should be closed
 *
 * Returns -errno on error.
 */
struct wlr_device *wlr_session_open_file(struct wlr_session *session,
	const char *path);

/*
 * Closes a file previously opened with wlr_session_open_file.
 */
void wlr_session_close_file(struct wlr_session *session,
	struct wlr_device *device);

/*
 * Changes the virtual terminal.
 */
bool wlr_session_change_vt(struct wlr_session *session, unsigned vt);

ssize_t wlr_session_find_gpus(struct wlr_session *session,
	size_t ret_len, struct wlr_device **ret);

#endif
