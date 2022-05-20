#ifndef WLR_BACKEND_X11_H
#define WLR_BACKEND_X11_H

#include <stdbool.h>

#include <wayland-server-core.h>

#include <wlr/backend.h>
#include <wlr/types/wlr_output.h>

struct wlr_input_device;

/**
 * Creates a new wlr_x11_backend. This backend will be created with no outputs;
 * you must use wlr_x11_output_create to add them.
 *
 * The `x11_display` argument is the name of the X Display socket. Set
 * to NULL for the default behaviour of XOpenDisplay.
 */
struct wlr_backend *wlr_x11_backend_create(struct wl_display *display,
	const char *x11_display);

/**
 * Adds a new output to this backend. You may remove outputs by destroying them.
 * Note that if called before initializing the backend, this will return NULL
 * and your outputs will be created during initialization (and given to you via
 * the output_add signal).
 */
struct wlr_output *wlr_x11_output_create(struct wlr_backend *backend);

/**
 * True if the given backend is a wlr_x11_backend.
 */
bool wlr_backend_is_x11(struct wlr_backend *backend);

/**
 * True if the given input device is a wlr_x11_input_device.
 */
bool wlr_input_device_is_x11(struct wlr_input_device *device);

/**
 * True if the given output is a wlr_x11_output.
 */
bool wlr_output_is_x11(struct wlr_output *output);

/**
 * Sets the title of a wlr_output which is an X11 window.
 */
void wlr_x11_output_set_title(struct wlr_output *output, const char *title);

#endif
