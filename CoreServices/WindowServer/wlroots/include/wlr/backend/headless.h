/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_BACKEND_HEADLESS_H
#define WLR_BACKEND_HEADLESS_H

#include <wlr/backend.h>
#include <wlr/types/wlr_output.h>

/**
 * Creates a headless backend. A headless backend has no outputs or inputs by
 * default.
 */
struct wlr_backend *wlr_headless_backend_create(struct wl_display *display);
/**
 * Create a new headless output backed by an in-memory EGL framebuffer. You can
 * read pixels from this framebuffer via wlr_renderer_read_pixels but it is
 * otherwise not displayed.
 */
struct wlr_output *wlr_headless_add_output(struct wlr_backend *backend,
	unsigned int width, unsigned int height);

bool wlr_backend_is_headless(struct wlr_backend *backend);
bool wlr_output_is_headless(struct wlr_output *output);

#endif
