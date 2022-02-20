#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <libudev.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <wayland-server-core.h>
#include <wlr/backend/session.h>
#include <wlr/config.h>
#include <wlr/util/log.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include "backend/session/session.h"
#include "util/signal.h"

#include <libseat.h>

#define WAIT_GPU_TIMEOUT 10000 // ms

static void handle_enable_seat(struct libseat *seat, void *data) {
	struct wlr_session *session = data;
	session->active = true;
	wlr_signal_emit_safe(&session->events.active, NULL);
}

static void handle_disable_seat(struct libseat *seat, void *data) {
	struct wlr_session *session = data;
	session->active = false;
	wlr_signal_emit_safe(&session->events.active, NULL);
	libseat_disable_seat(session->seat_handle);
}

static int libseat_event(int fd, uint32_t mask, void *data) {
	struct wlr_session *session = data;
	if (libseat_dispatch(session->seat_handle, 0) == -1) {
		wlr_log_errno(WLR_ERROR, "Failed to dispatch libseat");
		wl_display_terminate(session->display);
	}
	return 1;
}

static struct libseat_seat_listener seat_listener = {
	.enable_seat = handle_enable_seat,
	.disable_seat = handle_disable_seat,
};

static enum wlr_log_importance libseat_log_level_to_wlr(
		enum libseat_log_level level) {
	switch (level) {
	case LIBSEAT_LOG_LEVEL_ERROR:
		return WLR_ERROR;
	case LIBSEAT_LOG_LEVEL_INFO:
		return WLR_INFO;
	default:
		return WLR_DEBUG;
	}
}

static void log_libseat(enum libseat_log_level level,
		const char *fmt, va_list args) {
	enum wlr_log_importance importance = libseat_log_level_to_wlr(level);

	static char wlr_fmt[1024];
	snprintf(wlr_fmt, sizeof(wlr_fmt), "[libseat] %s", fmt);

	_wlr_vlog(importance, wlr_fmt, args);
}

static int libseat_session_init(struct wlr_session *session, struct wl_display *disp) {
	libseat_set_log_handler(log_libseat);
	libseat_set_log_level(LIBSEAT_LOG_LEVEL_INFO);

	// libseat will take care of updating the logind state if necessary
	setenv("XDG_SESSION_TYPE", "wayland", 1);

	session->seat_handle = libseat_open_seat(&seat_listener, session);
	if (session->seat_handle == NULL) {
		wlr_log_errno(WLR_ERROR, "Unable to create seat");
		return -1;
	}

	const char *seat_name = libseat_seat_name(session->seat_handle);
	if (seat_name == NULL) {
		wlr_log_errno(WLR_ERROR, "Unable to get seat info");
		goto error;
	}
	snprintf(session->seat, sizeof(session->seat), "%s", seat_name);

	struct wl_event_loop *event_loop = wl_display_get_event_loop(disp);
	session->libseat_event = wl_event_loop_add_fd(event_loop, libseat_get_fd(session->seat_handle),
		WL_EVENT_READABLE, libseat_event, session);
	if (session->libseat_event == NULL) {
		wlr_log(WLR_ERROR, "Failed to create libseat event source");
		goto error;
	}

	// We may have received enable_seat immediately after the open_seat result,
	// so, dispatch once without timeout to speed up activation.
	if (libseat_dispatch(session->seat_handle, 0) == -1) {
		wlr_log_errno(WLR_ERROR, "libseat dispatch failed");
		goto error_dispatch;
	}

	wlr_log(WLR_INFO, "Successfully loaded libseat session");
	return 0;

error_dispatch:
	wl_event_source_remove(session->libseat_event);
	session->libseat_event = NULL;
error:
	libseat_close_seat(session->seat_handle);
	session->seat_handle = NULL;
	return -1;
}

static void libseat_session_finish(struct wlr_session *session) {
	libseat_close_seat(session->seat_handle);
	wl_event_source_remove(session->libseat_event);
	session->seat_handle = NULL;
	session->libseat_event = NULL;
}

static bool is_drm_card(const char *sysname) {
	const char prefix[] = DRM_PRIMARY_MINOR_NAME;
	if (strncmp(sysname, prefix, strlen(prefix)) != 0) {
		return false;
	}
	for (size_t i = strlen(prefix); sysname[i] != '\0'; i++) {
		if (sysname[i] < '0' || sysname[i] > '9') {
			return false;
		}
	}
	return true;
}

static void read_udev_change_event(struct wlr_device_change_event *event,
		struct udev_device *udev_dev) {
	const char *hotplug = udev_device_get_property_value(udev_dev, "HOTPLUG");
	if (hotplug != NULL && strcmp(hotplug, "1") == 0) {
		event->type = WLR_DEVICE_HOTPLUG;
		struct wlr_device_hotplug_event *hotplug = &event->hotplug;

		const char *connector =
			udev_device_get_property_value(udev_dev, "CONNECTOR");
		if (connector != NULL) {
			hotplug->connector_id = strtoul(connector, NULL, 10);
		}

		const char *prop =
			udev_device_get_property_value(udev_dev, "PROPERTY");
		if (prop != NULL) {
			hotplug->prop_id = strtoul(prop, NULL, 10);
		}

		return;
	}

	const char *lease = udev_device_get_property_value(udev_dev, "LEASE");
	if (lease != NULL && strcmp(lease, "1") == 0) {
		event->type = WLR_DEVICE_LEASE;
		return;
	}
}

static int handle_udev_event(int fd, uint32_t mask, void *data) {
	struct wlr_session *session = data;

	struct udev_device *udev_dev = udev_monitor_receive_device(session->mon);
	if (!udev_dev) {
		return 1;
	}

	const char *sysname = udev_device_get_sysname(udev_dev);
	const char *devnode = udev_device_get_devnode(udev_dev);
	const char *action = udev_device_get_action(udev_dev);
	wlr_log(WLR_DEBUG, "udev event for %s (%s)", sysname, action);

	if (!is_drm_card(sysname) || !action || !devnode) {
		goto out;
	}

	const char *seat = udev_device_get_property_value(udev_dev, "ID_SEAT");
	if (!seat) {
		seat = "seat0";
	}
	if (session->seat[0] != '\0' && strcmp(session->seat, seat) != 0) {
		goto out;
	}

	if (strcmp(action, "add") == 0) {
		wlr_log(WLR_DEBUG, "DRM device %s added", sysname);
		struct wlr_session_add_event event = {
			.path = devnode,
		};
		wlr_signal_emit_safe(&session->events.add_drm_card, &event);
	} else if (strcmp(action, "change") == 0 || strcmp(action, "remove") == 0) {
		dev_t devnum = udev_device_get_devnum(udev_dev);
		struct wlr_device *dev;
		wl_list_for_each(dev, &session->devices, link) {
			if (dev->dev != devnum) {
				continue;
			}

			if (strcmp(action, "change") == 0) {
				wlr_log(WLR_DEBUG, "DRM device %s changed", sysname);
				struct wlr_device_change_event event = {0};
				read_udev_change_event(&event, udev_dev);
				wlr_signal_emit_safe(&dev->events.change, &event);
			} else if (strcmp(action, "remove") == 0) {
				wlr_log(WLR_DEBUG, "DRM device %s removed", sysname);
				wlr_signal_emit_safe(&dev->events.remove, NULL);
			} else {
				assert(0);
			}
			break;
		}
	}

out:
	udev_device_unref(udev_dev);
	return 1;
}

static void handle_display_destroy(struct wl_listener *listener, void *data) {
	struct wlr_session *session =
		wl_container_of(listener, session, display_destroy);
	wlr_session_destroy(session);
}

struct wlr_session *wlr_session_create(struct wl_display *disp) {
	struct wlr_session *session = calloc(1, sizeof(*session));
	if (!session) {
		wlr_log_errno(WLR_ERROR, "Allocation failed");
		return NULL;
	}

	wl_signal_init(&session->events.active);
	wl_signal_init(&session->events.add_drm_card);
	wl_signal_init(&session->events.destroy);
	wl_list_init(&session->devices);

	if (libseat_session_init(session, disp) == -1) {
		wlr_log(WLR_ERROR, "Failed to load session backend");
		goto error_open;
	}

	session->udev = udev_new();
	if (!session->udev) {
		wlr_log_errno(WLR_ERROR, "Failed to create udev context");
		goto error_session;
	}

	session->mon = udev_monitor_new_from_netlink(session->udev, "udev");
	if (!session->mon) {
		wlr_log_errno(WLR_ERROR, "Failed to create udev monitor");
		goto error_udev;
	}

	udev_monitor_filter_add_match_subsystem_devtype(session->mon, "drm", NULL);
	udev_monitor_enable_receiving(session->mon);

	struct wl_event_loop *event_loop = wl_display_get_event_loop(disp);
	int fd = udev_monitor_get_fd(session->mon);

	session->udev_event = wl_event_loop_add_fd(event_loop, fd,
		WL_EVENT_READABLE, handle_udev_event, session);
	if (!session->udev_event) {
		wlr_log_errno(WLR_ERROR, "Failed to create udev event source");
		goto error_mon;
	}

	session->display = disp;

	session->display_destroy.notify = handle_display_destroy;
	wl_display_add_destroy_listener(disp, &session->display_destroy);

	return session;

error_mon:
	udev_monitor_unref(session->mon);
error_udev:
	udev_unref(session->udev);
error_session:
	libseat_session_finish(session);
error_open:
	free(session);
	return NULL;
}

void wlr_session_destroy(struct wlr_session *session) {
	if (!session) {
		return;
	}

	wlr_signal_emit_safe(&session->events.destroy, session);
	wl_list_remove(&session->display_destroy.link);

	wl_event_source_remove(session->udev_event);
	udev_monitor_unref(session->mon);
	udev_unref(session->udev);

	struct wlr_device *dev, *tmp_dev;
	wl_list_for_each_safe(dev, tmp_dev, &session->devices, link) {
		wlr_session_close_file(session, dev);
	}

	libseat_session_finish(session);
	free(session);
}

struct wlr_device *wlr_session_open_file(struct wlr_session *session,
		const char *path) {
	int fd;
	int device_id = libseat_open_device(session->seat_handle, path, &fd);
	if (device_id == -1) {
		wlr_log_errno(WLR_ERROR, "Failed to open device: '%s'", path);
		return NULL;
	}

	struct wlr_device *dev = malloc(sizeof(*dev));
	if (!dev) {
		wlr_log_errno(WLR_ERROR, "Allocation failed");
		goto error;
	}

	struct stat st;
	if (fstat(fd, &st) < 0) {
		wlr_log_errno(WLR_ERROR, "Stat failed");
		goto error;
	}

	dev->fd = fd;
	dev->dev = st.st_rdev;
	dev->device_id = device_id;
	wl_signal_init(&dev->events.change);
	wl_signal_init(&dev->events.remove);
	wl_list_insert(&session->devices, &dev->link);

	return dev;

error:
	libseat_close_device(session->seat_handle, device_id);
	free(dev);
	close(fd);
	return NULL;
}

void wlr_session_close_file(struct wlr_session *session,
		struct wlr_device *dev) {
	if (libseat_close_device(session->seat_handle, dev->device_id) == -1) {
		wlr_log_errno(WLR_ERROR, "Failed to close device %d", dev->device_id);
	}
	close(dev->fd);
	wl_list_remove(&dev->link);
	free(dev);
}

bool wlr_session_change_vt(struct wlr_session *session, unsigned vt) {
	if (!session) {
		return false;
	}
	return libseat_switch_session(session->seat_handle, vt) == 0;
}

/* Tests if 'path' is KMS compatible by trying to open it. Returns the opened
 * device on success. */
struct wlr_device *session_open_if_kms(struct wlr_session *restrict session,
		const char *restrict path) {
	if (!path) {
		return NULL;
	}

	struct wlr_device *dev = wlr_session_open_file(session, path);
	if (!dev) {
		return NULL;
	}

	if (!drmIsKMS(dev->fd)) {
		wlr_log(WLR_DEBUG, "Ignoring '%s': not a KMS device", path);
		wlr_session_close_file(session, dev);
		return NULL;
	}

	return dev;
}

static ssize_t explicit_find_gpus(struct wlr_session *session,
		size_t ret_len, struct wlr_device *ret[static ret_len], const char *str) {
	char *gpus = strdup(str);
	if (!gpus) {
		wlr_log_errno(WLR_ERROR, "Allocation failed");
		return -1;
	}

	size_t i = 0;
	char *save;
	char *ptr = strtok_r(gpus, ":", &save);
	do {
		if (i >= ret_len) {
			break;
		}

		ret[i] = session_open_if_kms(session, ptr);
		if (!ret[i]) {
			wlr_log(WLR_ERROR, "Unable to open %s as DRM device", ptr);
		} else {
			++i;
		}
	} while ((ptr = strtok_r(NULL, ":", &save)));

	free(gpus);
	return i;
}

static struct udev_enumerate *enumerate_drm_cards(struct udev *udev) {
	struct udev_enumerate *en = udev_enumerate_new(udev);
	if (!en) {
		wlr_log(WLR_ERROR, "udev_enumerate_new failed");
		return NULL;
	}

	udev_enumerate_add_match_subsystem(en, "drm");
	udev_enumerate_add_match_sysname(en, DRM_PRIMARY_MINOR_NAME "[0-9]*");

	if (udev_enumerate_scan_devices(en) != 0) {
		wlr_log(WLR_ERROR, "udev_enumerate_scan_devices failed");
		udev_enumerate_unref(en);
		return NULL;
	}

	return en;
}

static uint64_t get_current_time_ms(void) {
	struct timespec ts = {0};
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

struct find_gpus_add_handler {
	bool added;
	struct wl_listener listener;
};

static void find_gpus_handle_add(struct wl_listener *listener, void *data) {
	struct find_gpus_add_handler *handler =
		wl_container_of(listener, handler, listener);
	handler->added = true;
}

/* Tries to find the primary GPU by checking for the "boot_vga" attribute.
 * If it's not found, it returns the first valid GPU it finds.
 */
ssize_t wlr_session_find_gpus(struct wlr_session *session,
		size_t ret_len, struct wlr_device **ret) {
	const char *explicit = getenv("WLR_DRM_DEVICES");
	if (explicit) {
		return explicit_find_gpus(session, ret_len, ret, explicit);
	}

	struct udev_enumerate *en = enumerate_drm_cards(session->udev);
	if (!en) {
		return -1;
	}

	if (udev_enumerate_get_list_entry(en) == NULL) {
		udev_enumerate_unref(en);
		wlr_log(WLR_INFO, "Waiting for a DRM card device");

		struct find_gpus_add_handler handler = {0};
		handler.listener.notify = find_gpus_handle_add;
		wl_signal_add(&session->events.add_drm_card, &handler.listener);

		uint64_t started_at = get_current_time_ms();
		uint64_t timeout = WAIT_GPU_TIMEOUT;
		struct wl_event_loop *event_loop =
			wl_display_get_event_loop(session->display);
		while (!handler.added) {
			int ret = wl_event_loop_dispatch(event_loop, (int)timeout);
			if (ret < 0) {
				wlr_log_errno(WLR_ERROR, "Failed to wait for DRM card device: "
					"wl_event_loop_dispatch failed");
				udev_enumerate_unref(en);
				return -1;
			}

			uint64_t now = get_current_time_ms();
			if (now >= started_at + WAIT_GPU_TIMEOUT) {
				break;
			}
			timeout = started_at + WAIT_GPU_TIMEOUT - now;
		}

		wl_list_remove(&handler.listener.link);

		en = enumerate_drm_cards(session->udev);
		if (!en) {
			return -1;
		}
	}

	struct udev_list_entry *entry;
	size_t i = 0;

	udev_list_entry_foreach(entry, udev_enumerate_get_list_entry(en)) {
		if (i == ret_len) {
			break;
		}

		bool is_boot_vga = false;

		const char *path = udev_list_entry_get_name(entry);
		struct udev_device *dev = udev_device_new_from_syspath(session->udev, path);
		if (!dev) {
			continue;
		}

		const char *seat = udev_device_get_property_value(dev, "ID_SEAT");
		if (!seat) {
			seat = "seat0";
		}
		if (session->seat[0] && strcmp(session->seat, seat) != 0) {
			udev_device_unref(dev);
			continue;
		}

		// This is owned by 'dev', so we don't need to free it
		struct udev_device *pci =
			udev_device_get_parent_with_subsystem_devtype(dev, "pci", NULL);

		if (pci) {
			const char *id = udev_device_get_sysattr_value(pci, "boot_vga");
			if (id && strcmp(id, "1") == 0) {
				is_boot_vga = true;
			}
		}

		struct wlr_device *wlr_dev =
			session_open_if_kms(session, udev_device_get_devnode(dev));
		if (!wlr_dev) {
			udev_device_unref(dev);
			continue;
		}

		udev_device_unref(dev);

		ret[i] = wlr_dev;
		if (is_boot_vga) {
			struct wlr_device *tmp = ret[0];
			ret[0] = ret[i];
			ret[i] = tmp;
		}

		++i;
	}

	udev_enumerate_unref(en);

	return i;
}
