/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_TYPES_WLR_LAYER_SHELL_V1_H
#define WLR_TYPES_WLR_LAYER_SHELL_V1_H

#include <stdbool.h>
#include <stdint.h>
#include <wayland-server-core.h>
#include <wlr/types/wlr_compositor.h>
#include "wlr-layer-shell-unstable-v1-protocol.h"

/**
 * wlr_layer_shell_v1 allows clients to arrange themselves in "layers" on the
 * desktop in accordance with the wlr-layer-shell protocol. When a client is
 * added, the new_surface signal will be raised and passed a reference to our
 * wlr_layer_surface_v1. At this time, the client will have configured the
 * surface as it desires, including information like desired anchors and
 * margins. The compositor should use this information to decide how to arrange
 * the layer on-screen, then determine the dimensions of the layer and call
 * wlr_layer_surface_v1_configure. The client will then attach a buffer and
 * commit the surface, at which point the wlr_layer_surface_v1 map signal is
 * raised and the compositor should begin rendering the surface.
 */
struct wlr_layer_shell_v1 {
	struct wl_global *global;

	struct wl_listener display_destroy;

	struct {
		// struct wlr_layer_surface_v1 *
		// Note: the output may be NULL. In this case, it is your
		// responsibility to assign an output before returning.
		struct wl_signal new_surface;
		struct wl_signal destroy;
	} events;

	void *data;
};

enum wlr_layer_surface_v1_state_field {
	WLR_LAYER_SURFACE_V1_STATE_DESIRED_SIZE = 1 << 0,
	WLR_LAYER_SURFACE_V1_STATE_ANCHOR = 1 << 1,
	WLR_LAYER_SURFACE_V1_STATE_EXCLUSIVE_ZONE = 1 << 2,
	WLR_LAYER_SURFACE_V1_STATE_MARGIN = 1 << 3,
	WLR_LAYER_SURFACE_V1_STATE_KEYBOARD_INTERACTIVITY = 1 << 4,
	WLR_LAYER_SURFACE_V1_STATE_LAYER = 1 << 5,
};

struct wlr_layer_surface_v1_state {
	uint32_t committed; // enum wlr_layer_surface_v1_state_field

	uint32_t anchor;
	int32_t exclusive_zone;
	struct {
		int32_t top, right, bottom, left;
	} margin;
	enum zwlr_layer_surface_v1_keyboard_interactivity keyboard_interactive;
	uint32_t desired_width, desired_height;
	enum zwlr_layer_shell_v1_layer layer;

	uint32_t configure_serial;
	uint32_t actual_width, actual_height;
};

struct wlr_layer_surface_v1_configure {
	struct wl_list link; // wlr_layer_surface_v1::configure_list
	uint32_t serial;

	uint32_t width, height;
};

struct wlr_layer_surface_v1 {
	struct wlr_surface *surface;
	struct wlr_output *output;
	struct wl_resource *resource;
	struct wlr_layer_shell_v1 *shell;
	struct wl_list popups; // wlr_xdg_popup::link

	char *namespace;

	bool added, configured, mapped;
	struct wl_list configure_list;

	struct wlr_layer_surface_v1_state current, pending;

	struct wl_listener surface_destroy;

	struct {
		/**
		 * The destroy signal indicates that the wlr_layer_surface is about to be
		 * freed. It is guaranteed that the unmap signal is raised before the destroy
		 * signal if the layer surface is destroyed while mapped.
		 */
		struct wl_signal destroy;
		/**
		 * The map signal indicates that the client has configured itself and is
		 * ready to be rendered by the compositor.
		 */
		struct wl_signal map;
		/**
		 * The unmap signal indicates that the surface is no longer in a state where
		 * it should be rendered by the compositor. This might happen if the surface
		 * no longer has a displayable buffer because either the surface has been
		 * hidden or is about to be destroyed. It is guaranteed that the unmap signal
		 * is raised before the destroy signal if the layer surface is destroyed
		 * while mapped.
		 */
		struct wl_signal unmap;
		/**
		 * The new_popup signal is raised when a new popup is created. The data
		 * parameter passed to the listener is a pointer to the new wlr_xdg_popup.
		 */
		struct wl_signal new_popup;
	} events;

	void *data;
};

struct wlr_layer_shell_v1 *wlr_layer_shell_v1_create(struct wl_display *display);

/**
 * Notifies the layer surface to configure itself with this width/height. The
 * layer_surface will signal its map event when the surface is ready to assume
 * this size. Returns the associated configure serial.
 */
uint32_t wlr_layer_surface_v1_configure(struct wlr_layer_surface_v1 *surface,
		uint32_t width, uint32_t height);

/**
 * Notify the client that the surface has been closed and destroy the
 * wlr_layer_surface_v1, rendering the resource inert.
 */
void wlr_layer_surface_v1_destroy(struct wlr_layer_surface_v1 *surface);

bool wlr_surface_is_layer_surface(struct wlr_surface *surface);

struct wlr_layer_surface_v1 *wlr_layer_surface_v1_from_wlr_surface(
		struct wlr_surface *surface);

/**
 * Calls the iterator function for each mapped sub-surface and popup of this
 * surface (whether or not this surface is mapped).
 */
void wlr_layer_surface_v1_for_each_surface(struct wlr_layer_surface_v1 *surface,
		wlr_surface_iterator_func_t iterator, void *user_data);

/**
 * Call `iterator` on each popup's surface and popup's subsurface in the
 * layer surface's tree, with the surfaces's position relative to the root
 * layer surface. The function is called from root to leaves (in rendering
 * order).
 */
void wlr_layer_surface_v1_for_each_popup_surface(
		struct wlr_layer_surface_v1 *surface,
		wlr_surface_iterator_func_t iterator, void *user_data);

/**
 * Find a surface within this layer-surface tree at the given surface-local
 * coordinates. Returns the surface and coordinates in the leaf surface
 * coordinate system or NULL if no surface is found at that location.
 */
struct wlr_surface *wlr_layer_surface_v1_surface_at(
		struct wlr_layer_surface_v1 *surface, double sx, double sy,
		double *sub_x, double *sub_y);

/**
 * Find a surface within this layer-surface's popup tree at the given
 * surface-local coordinates. Returns the surface and coordinates in the leaf
 * surface coordinate system or NULL if no surface is found at that location.
 */
struct wlr_surface *wlr_layer_surface_v1_popup_surface_at(
		struct wlr_layer_surface_v1 *surface, double sx, double sy,
		double *sub_x, double *sub_y);

#endif
