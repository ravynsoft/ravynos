#ifndef WLR_BACKEND_WAYLAND_H
#define WLR_BACKEND_WAYLAND_H
#include <stdbool.h>
#include <wayland-client.h>
#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/types/wlr_output.h>

struct wlr_input_device;

/**
 * Creates a new wlr_wl_backend. This backend will be created with no outputs;
 * you must use wlr_wl_output_create to add them.
 *
 * The `remote` argument is the name of the host compositor wayland socket. Set
 * to NULL for the default behaviour (WAYLAND_DISPLAY env variable or wayland-0
 * default)
 */
struct wlr_backend *wlr_wl_backend_create(struct wl_display *display,
		const char *remote);

/**
 * Returns the remote wl_display used by the Wayland backend.
 */
struct wl_display *wlr_wl_backend_get_remote_display(struct wlr_backend *backend);

/**
 * Adds a new output to this backend. You may remove outputs by destroying them.
 * Note that if called before initializing the backend, this will return NULL
 * and your outputs will be created during initialization (and given to you via
 * the output_add signal).
 */
struct wlr_output *wlr_wl_output_create(struct wlr_backend *backend);

/**
 * True if the given backend is a wlr_wl_backend.
 */
bool wlr_backend_is_wl(struct wlr_backend *backend);

/**
 * True if the given input device is a wlr_wl_input_device.
 */
bool wlr_input_device_is_wl(struct wlr_input_device *device);

/**
 * True if the given output is a wlr_wl_output.
 */
bool wlr_output_is_wl(struct wlr_output *output);

/**
 * Sets the title of a wlr_output which is a Wayland window.
 */
void wlr_wl_output_set_title(struct wlr_output *output, const char *title);

/**
 * Returns the remote wl_surface used by the Wayland output.
 */
struct wl_surface *wlr_wl_output_get_surface(struct wlr_output *output);

/**
 * Returns the remote wl_seat for a Wayland input device.
 */
struct wl_seat *wlr_wl_input_device_get_seat(struct wlr_input_device *dev);

#endif
