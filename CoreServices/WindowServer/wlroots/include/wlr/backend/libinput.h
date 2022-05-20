/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_BACKEND_LIBINPUT_H
#define WLR_BACKEND_LIBINPUT_H

#include <libinput.h>
#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/backend/session.h>
#include <wlr/types/wlr_input_device.h>

struct wlr_backend *wlr_libinput_backend_create(struct wl_display *display,
		struct wlr_session *session);
/** Gets the underlying libinput_device handle for the given wlr_input_device */
struct libinput_device *wlr_libinput_get_device_handle(
		struct wlr_input_device *dev);

bool wlr_backend_is_libinput(struct wlr_backend *backend);
bool wlr_input_device_is_libinput(struct wlr_input_device *device);

#endif
