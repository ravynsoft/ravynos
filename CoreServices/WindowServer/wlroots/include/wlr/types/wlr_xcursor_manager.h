/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_TYPES_WLR_XCURSOR_MANAGER_H
#define WLR_TYPES_WLR_XCURSOR_MANAGER_H

#include <wayland-server-core.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/xcursor.h>

/**
 * An XCursor theme at a particular scale factor of the base size.
 */
struct wlr_xcursor_manager_theme {
	float scale;
	struct wlr_xcursor_theme *theme;
	struct wl_list link;
};

/**
 * wlr_xcursor_manager dynamically loads xcursor themes at sizes necessary for
 * use on outputs at arbitrary scale factors. You should call
 * wlr_xcursor_manager_load for each output you will show your cursor on, with
 * the scale factor parameter set to that output's scale factor.
 */
struct wlr_xcursor_manager {
	char *name;
	uint32_t size;
	struct wl_list scaled_themes; // wlr_xcursor_manager_theme::link
};

/**
 * Creates a new XCursor manager with the given xcursor theme name and base size
 * (for use when scale=1).
 */
struct wlr_xcursor_manager *wlr_xcursor_manager_create(const char *name,
	uint32_t size);

void wlr_xcursor_manager_destroy(struct wlr_xcursor_manager *manager);

/**
 * Ensures an xcursor theme at the given scale factor is loaded in the manager.
 */
bool wlr_xcursor_manager_load(struct wlr_xcursor_manager *manager,
	float scale);

/**
 * Retrieves a wlr_xcursor reference for the given cursor name at the given
 * scale factor, or NULL if this wlr_xcursor_manager has not loaded a cursor
 * theme at the requested scale.
 */
struct wlr_xcursor *wlr_xcursor_manager_get_xcursor(
	struct wlr_xcursor_manager *manager, const char *name, float scale);

/**
 * Set a wlr_cursor's cursor image to the specified cursor name for all scale
 * factors. wlr_cursor will take over from this point and ensure the correct
 * cursor is used on each output, assuming a wlr_output_layout is attached to
 * it.
 */
void wlr_xcursor_manager_set_cursor_image(struct wlr_xcursor_manager *manager,
	const char *name, struct wlr_cursor *cursor);

#endif
