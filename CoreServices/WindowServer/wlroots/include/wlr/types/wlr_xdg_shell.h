/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_TYPES_WLR_XDG_SHELL_H
#define WLR_TYPES_WLR_XDG_SHELL_H

#include <wayland-server-core.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/util/box.h>
#include "xdg-shell-protocol.h"

struct wlr_xdg_shell {
	struct wl_global *global;
	struct wl_list clients;
	struct wl_list popup_grabs;
	uint32_t ping_timeout;

	struct wl_listener display_destroy;

	struct {
		/**
		 * The `new_surface` event signals that a client has requested to
		 * create a new shell surface. At this point, the surface is ready to
		 * be configured but is not mapped or ready receive input events. The
		 * surface will be ready to be managed on the `map` event.
		 */
		struct wl_signal new_surface;
		struct wl_signal destroy;
	} events;

	void *data;
};

struct wlr_xdg_client {
	struct wlr_xdg_shell *shell;
	struct wl_resource *resource;
	struct wl_client *client;
	struct wl_list surfaces;

	struct wl_list link; // wlr_xdg_shell::clients

	uint32_t ping_serial;
	struct wl_event_source *ping_timer;
};

struct wlr_xdg_positioner {
	struct wlr_box anchor_rect;
	enum xdg_positioner_anchor anchor;
	enum xdg_positioner_gravity gravity;
	enum xdg_positioner_constraint_adjustment constraint_adjustment;

	struct {
		int32_t width, height;
	} size;

	struct {
		int32_t x, y;
	} offset;
};

struct wlr_xdg_popup {
	struct wlr_xdg_surface *base;
	struct wl_list link;

	struct wl_resource *resource;
	bool committed;
	struct wlr_surface *parent;
	struct wlr_seat *seat;

	// Position of the popup relative to the upper left corner of the window
	// geometry of the parent surface
	struct wlr_box geometry;

	struct wlr_xdg_positioner positioner;

	struct wl_list grab_link; // wlr_xdg_popup_grab::popups
};

// each seat gets a popup grab
struct wlr_xdg_popup_grab {
	struct wl_client *client;
	struct wlr_seat_pointer_grab pointer_grab;
	struct wlr_seat_keyboard_grab keyboard_grab;
	struct wlr_seat_touch_grab touch_grab;
	struct wlr_seat *seat;
	struct wl_list popups;
	struct wl_list link; // wlr_xdg_shell::popup_grabs
	struct wl_listener seat_destroy;
};

enum wlr_xdg_surface_role {
	WLR_XDG_SURFACE_ROLE_NONE,
	WLR_XDG_SURFACE_ROLE_TOPLEVEL,
	WLR_XDG_SURFACE_ROLE_POPUP,
};

struct wlr_xdg_toplevel_state {
	bool maximized, fullscreen, resizing, activated;
	uint32_t tiled; // enum wlr_edges
	uint32_t width, height;
	uint32_t max_width, max_height;
	uint32_t min_width, min_height;
};

struct wlr_xdg_toplevel_configure {
	bool maximized, fullscreen, resizing, activated;
	uint32_t tiled; // enum wlr_edges
	uint32_t width, height;
};

struct wlr_xdg_toplevel_requested {
	bool maximized, minimized, fullscreen;
	struct wlr_output *fullscreen_output;
	struct wl_listener fullscreen_output_destroy;
};

struct wlr_xdg_toplevel {
	struct wl_resource *resource;
	struct wlr_xdg_surface *base;
	bool added;

	struct wlr_xdg_toplevel *parent;
	struct wl_listener parent_unmap;

	struct wlr_xdg_toplevel_state current, pending;

	// Properties to be sent to the client in the next configure event.
	struct wlr_xdg_toplevel_configure scheduled;

	// Properties that the client has requested. Intended to be checked
	// by the compositor on surface map and state change requests (such as
	// xdg_toplevel::set_fullscreen) and handled accordingly.
	struct wlr_xdg_toplevel_requested requested;

	char *title;
	char *app_id;

	struct {
		struct wl_signal request_maximize;
		struct wl_signal request_fullscreen;
		struct wl_signal request_minimize;
		struct wl_signal request_move;
		struct wl_signal request_resize;
		struct wl_signal request_show_window_menu;
		struct wl_signal set_parent;
		struct wl_signal set_title;
		struct wl_signal set_app_id;
	} events;
};

struct wlr_xdg_surface_configure {
	struct wlr_xdg_surface *surface;
	struct wl_list link; // wlr_xdg_surface::configure_list
	uint32_t serial;

	struct wlr_xdg_toplevel_configure *toplevel_configure;
};

struct wlr_xdg_surface_state {
	uint32_t configure_serial;
	struct wlr_box geometry;
};

/**
 * An xdg-surface is a user interface element requiring management by the
 * compositor. An xdg-surface alone isn't useful, a role should be assigned to
 * it in order to map it.
 *
 * When a surface has a role and is ready to be displayed, the `map` event is
 * emitted. When a surface should no longer be displayed, the `unmap` event is
 * emitted. The `unmap` event is guaranteed to be emitted before the `destroy`
 * event if the view is destroyed when mapped.
 */
struct wlr_xdg_surface {
	struct wlr_xdg_client *client;
	struct wl_resource *resource;
	struct wlr_surface *surface;
	struct wl_list link; // wlr_xdg_client::surfaces
	enum wlr_xdg_surface_role role;

	union {
		struct wlr_xdg_toplevel *toplevel;
		struct wlr_xdg_popup *popup;
	};

	struct wl_list popups; // wlr_xdg_popup::link

	bool added, configured, mapped;
	struct wl_event_source *configure_idle;
	uint32_t scheduled_serial;
	struct wl_list configure_list;

	struct wlr_xdg_surface_state current, pending;

	struct wl_listener surface_destroy;
	struct wl_listener surface_commit;

	struct {
		struct wl_signal destroy;
		struct wl_signal ping_timeout;
		struct wl_signal new_popup;
		/**
		 * The `map` event signals that the shell surface is ready to be
		 * managed by the compositor and rendered on the screen. At this point,
		 * the surface has configured its properties, has had the opportunity
		 * to bind to the seat to receive input events, and has a buffer that
		 * is ready to be rendered. You can now safely add this surface to a
		 * list of views.
		 */
		struct wl_signal map;
		/**
		 * The `unmap` event signals that the surface is no longer in a state
		 * where it should be shown on the screen. This might happen if the
		 * surface no longer has a displayable buffer because either the
		 * surface has been hidden or is about to be destroyed.
		 */
		struct wl_signal unmap;

		// for protocol extensions
		struct wl_signal configure; // wlr_xdg_surface_configure
		struct wl_signal ack_configure; // wlr_xdg_surface_configure
	} events;

	void *data;
};

struct wlr_xdg_toplevel_move_event {
	struct wlr_xdg_toplevel *toplevel;
	struct wlr_seat_client *seat;
	uint32_t serial;
};

struct wlr_xdg_toplevel_resize_event {
	struct wlr_xdg_toplevel *toplevel;
	struct wlr_seat_client *seat;
	uint32_t serial;
	uint32_t edges;
};

struct wlr_xdg_toplevel_show_window_menu_event {
	struct wlr_xdg_toplevel *toplevel;
	struct wlr_seat_client *seat;
	uint32_t serial;
	uint32_t x, y;
};

struct wlr_xdg_shell *wlr_xdg_shell_create(struct wl_display *display);

/** Get the corresponding wlr_xdg_surface from a resource.
 *
 * Aborts if the resource doesn't have the correct type. Returns NULL if the
 * resource is inert.
 */
struct wlr_xdg_surface *wlr_xdg_surface_from_resource(
		struct wl_resource *resource);

/** Get the corresponding wlr_xdg_popup from a resource.
 *
 * Aborts if the resource doesn't have the correct type. Returns NULL if the
 * resource is inert.
 */
struct wlr_xdg_popup *wlr_xdg_popup_from_resource(
		struct wl_resource *resource);

/** Get the corresponding wlr_xdg_toplevel from a resource.
 *
 * Aborts if the resource doesn't have the correct type. Returns NULL if the
 * resource is inert.
 */
struct wlr_xdg_toplevel *wlr_xdg_toplevel_from_resource(
		struct wl_resource *resource);

/**
 * Send a ping to the surface. If the surface does not respond in a reasonable
 * amount of time, the ping_timeout event will be emitted.
 */
void wlr_xdg_surface_ping(struct wlr_xdg_surface *surface);

/**
 * Request that this toplevel surface be the given size. Returns the associated
 * configure serial.
 */
uint32_t wlr_xdg_toplevel_set_size(struct wlr_xdg_toplevel *toplevel,
		uint32_t width, uint32_t height);

/**
 * Request that this toplevel show itself in an activated or deactivated
 * state. Returns the associated configure serial.
 */
uint32_t wlr_xdg_toplevel_set_activated(struct wlr_xdg_toplevel *toplevel,
		bool activated);

/**
 * Request that this toplevel consider itself maximized or not
 * maximized. Returns the associated configure serial.
 */
uint32_t wlr_xdg_toplevel_set_maximized(struct wlr_xdg_toplevel *toplevel,
		bool maximized);

/**
 * Request that this toplevel consider itself fullscreen or not
 * fullscreen. Returns the associated configure serial.
 */
uint32_t wlr_xdg_toplevel_set_fullscreen(struct wlr_xdg_toplevel *toplevel,
		bool fullscreen);

/**
 * Request that this toplevel consider itself to be resizing or not
 * resizing. Returns the associated configure serial.
 */
uint32_t wlr_xdg_toplevel_set_resizing(struct wlr_xdg_toplevel *toplevel,
		bool resizing);

/**
 * Request that this toplevel consider itself in a tiled layout and some
 * edges are adjacent to another part of the tiling grid. `tiled_edges` is a
 * bitfield of `enum wlr_edges`. Returns the associated configure serial.
 */
uint32_t wlr_xdg_toplevel_set_tiled(struct wlr_xdg_toplevel *toplevel,
		uint32_t tiled_edges);

/**
 * Request that this toplevel closes.
 */
void wlr_xdg_toplevel_send_close(struct wlr_xdg_toplevel *toplevel);

/**
 * Sets the parent of this toplevel. Parent can be NULL.
 */
void wlr_xdg_toplevel_set_parent(struct wlr_xdg_toplevel *toplevel,
		struct wlr_xdg_toplevel *parent);

/**
 * Request that this popup closes.
 **/
void wlr_xdg_popup_destroy(struct wlr_xdg_popup *popup);

/**
 * Get the position for this popup in the surface parent's coordinate system.
 */
void wlr_xdg_popup_get_position(struct wlr_xdg_popup *popup,
		double *popup_sx, double *popup_sy);
/**
 * Get the geometry for this positioner based on the anchor rect, gravity, and
 * size of this positioner.
 */
struct wlr_box wlr_xdg_positioner_get_geometry(
		struct wlr_xdg_positioner *positioner);

/**
 * Get the anchor point for this popup in the toplevel parent's coordinate system.
 */
void wlr_xdg_popup_get_anchor_point(struct wlr_xdg_popup *popup,
		int *toplevel_sx, int *toplevel_sy);

/**
 * Convert the given coordinates in the popup coordinate system to the toplevel
 * surface coordinate system.
 */
void wlr_xdg_popup_get_toplevel_coords(struct wlr_xdg_popup *popup,
		int popup_sx, int popup_sy, int *toplevel_sx, int *toplevel_sy);

/**
 * Set the geometry of this popup to unconstrain it according to its
 * xdg-positioner rules. The box should be in the popup's root toplevel parent
 * surface coordinate system.
 */
void wlr_xdg_popup_unconstrain_from_box(struct wlr_xdg_popup *popup,
		const struct wlr_box *toplevel_sx_box);

/**
  Invert the right/left anchor and gravity for this positioner. This can be
  used to "flip" the positioner around the anchor rect in the x direction.
 */
void wlr_positioner_invert_x(struct wlr_xdg_positioner *positioner);

/**
  Invert the top/bottom anchor and gravity for this positioner. This can be
  used to "flip" the positioner around the anchor rect in the y direction.
 */
void wlr_positioner_invert_y(struct wlr_xdg_positioner *positioner);

/**
 * Find a surface within this xdg-surface tree at the given surface-local
 * coordinates. Returns the surface and coordinates in the leaf surface
 * coordinate system or NULL if no surface is found at that location.
 */
struct wlr_surface *wlr_xdg_surface_surface_at(
		struct wlr_xdg_surface *surface, double sx, double sy,
		double *sub_x, double *sub_y);

/**
 * Find a surface within this xdg-surface's popup tree at the given
 * surface-local coordinates. Returns the surface and coordinates in the leaf
 * surface coordinate system or NULL if no surface is found at that location.
 */
struct wlr_surface *wlr_xdg_surface_popup_surface_at(
		struct wlr_xdg_surface *surface, double sx, double sy,
		double *sub_x, double *sub_y);

bool wlr_surface_is_xdg_surface(struct wlr_surface *surface);

struct wlr_xdg_surface *wlr_xdg_surface_from_wlr_surface(
		struct wlr_surface *surface);

/**
 * Get the surface geometry.
 * This is either the geometry as set by the client, or defaulted to the bounds
 * of the surface + the subsurfaces (as specified by the protocol).
 *
 * The x and y value can be <0
 */
void wlr_xdg_surface_get_geometry(struct wlr_xdg_surface *surface,
		struct wlr_box *box);

/**
 * Call `iterator` on each mapped surface and popup in the xdg-surface tree
 * (whether or not this xdg-surface is mapped), with the surface's position
 * relative to the root xdg-surface. The function is called from root to leaves
 * (in rendering order).
 */
void wlr_xdg_surface_for_each_surface(struct wlr_xdg_surface *surface,
		wlr_surface_iterator_func_t iterator, void *user_data);

/**
 * Call `iterator` on each mapped popup's surface and popup's subsurface in the
 * xdg-surface tree (whether or not this xdg-surface is mapped), with the
 * surfaces's position relative to the root xdg-surface. The function is called
 * from root to leaves (in rendering order).
 */
void wlr_xdg_surface_for_each_popup_surface(struct wlr_xdg_surface *surface,
		wlr_surface_iterator_func_t iterator, void *user_data);

/**
 * Schedule a surface configuration. This should only be called by protocols
 * extending the shell.
 */
uint32_t wlr_xdg_surface_schedule_configure(struct wlr_xdg_surface *surface);

#endif
