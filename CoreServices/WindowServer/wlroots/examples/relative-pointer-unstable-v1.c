#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GLES2/gl2.h>
#include <linux/input-event-codes.h>
#include <wayland-egl.h>
#include "egl_common.h"
#include "pointer-constraints-unstable-v1-client-protocol.h"
#include "relative-pointer-unstable-v1-client-protocol.h"
#include "xdg-shell-client-protocol.h"


/**
 * structs storing global state
 *
 * These are passed around as user data in the wayland globals.
 */

struct egl_info {
	struct wl_egl_window *egl_window;
	struct wlr_egl_surface *egl_surface;
	uint32_t width;
	uint32_t height;
	struct wl_surface *surface;
	struct wl_callback *frame_callback;
};

struct window {
	struct egl_info *egl_info;
	struct wl_pointer *pointer;
	struct zwp_locked_pointer_v1 *locked_pointer;
	struct zwp_relative_pointer_v1 *relative_pointer;
	int32_t pointer_x, pointer_y;
	uint32_t last_draw;
};


/**
 * surface handling and helpers
 *
 * draw_cursor and draw_relative_cursor draw a small 5px by 5px box around the
 * cursor and relative motion coordinates respectively. draw_background colors
 * the background black.
 *
 * The functions are somewhat duplicated, but it doesn't really matter.
 */

static void surface_callback_handle_done(void *data,
		struct wl_callback *callback, uint32_t time) {
	wl_callback_destroy(callback);
	struct egl_info *e = data;
	e->frame_callback = NULL;
}

static struct wl_callback_listener surface_callback_listener = {
	.done = surface_callback_handle_done,
};

static void draw_init(struct egl_info *e) {
	eglMakeCurrent(egl_display, e->egl_surface,
		e->egl_surface, egl_context);
	glViewport(0, 0, e->width, e->height);
}

static void draw_cursor(struct egl_info *e, int32_t x, int32_t y) {
	glEnable(GL_SCISSOR_TEST);
	glScissor(x, e->height - y, 5, 5);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f); /* white */
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_SCISSOR_TEST);
}

static void draw_relative_cursor(struct egl_info *e, int32_t x, int32_t y) {
	glEnable(GL_SCISSOR_TEST);
	glScissor(x, e->height - y, 5, 5);
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f); /* red */
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_SCISSOR_TEST);
}

static void draw_background(struct egl_info *e) {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); /* black */
	glClear(GL_COLOR_BUFFER_BIT);
}

static void draw_end(struct egl_info *e) {
	e->frame_callback = wl_surface_frame(e->surface);
	wl_callback_add_listener(e->frame_callback, &surface_callback_listener, e);

	eglSwapBuffers(egl_display, e->egl_surface);
}


/**
 * registry and globals handling
 */

static struct wl_compositor *compositor = NULL;
static struct wl_seat *seat = NULL;
static struct xdg_wm_base *wm_base = NULL;
static struct zwp_pointer_constraints_v1 *pointer_constraints = NULL;
static struct zwp_relative_pointer_manager_v1 *relative_pointer_manager = NULL;

static void registry_handle_global(void *data, struct wl_registry *registry,
		uint32_t name, const char *interface, uint32_t version) {
	if (strcmp(interface, wl_compositor_interface.name) == 0) {
		compositor = wl_registry_bind(registry, name,
			&wl_compositor_interface, version);
	} else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
		wm_base = wl_registry_bind(registry, name,
			&xdg_wm_base_interface, version);
	} else if (strcmp(interface, wl_seat_interface.name) == 0) {
		seat = wl_registry_bind(registry, name,
			&wl_seat_interface, version);
	} else if (strcmp(interface, zwp_pointer_constraints_v1_interface.name) == 0) {
		pointer_constraints = wl_registry_bind(registry, name,
			&zwp_pointer_constraints_v1_interface, version);
	} else if (strcmp(interface, zwp_relative_pointer_manager_v1_interface.name) == 0) {
		relative_pointer_manager = wl_registry_bind(registry, name,
			&zwp_relative_pointer_manager_v1_interface, version);
	}
}

static void registry_handle_global_remove(void *data, struct wl_registry *registry,
		uint32_t time) {
	/* This space intentionally left blank */
}

static const struct wl_registry_listener registry_listener = {
	.global = registry_handle_global,
	.global_remove = registry_handle_global_remove,
};


/**
 * xdg_surface handling
 */

static void xdg_surface_handle_configure(void *data,
		struct xdg_surface *xdg_surface, uint32_t serial) {
	struct egl_info *e = data;
	xdg_surface_ack_configure(xdg_surface, serial);
	wl_egl_window_resize(e->egl_window, e->width, e->height, 0, 0);
	draw_init(e);
	draw_background(e);
	draw_end(e);
}

static const struct xdg_surface_listener xdg_surface_listener = {
	.configure = xdg_surface_handle_configure,
};


/**
 * xdg_toplevel handling
 */

static void xdg_toplevel_handle_configure(void *data,
		struct xdg_toplevel *xdg_toplevel, int32_t width, int32_t height,
		struct wl_array *states) {
	struct egl_info *e = data;
	// TODO: Why do we get 0,0 on initialization here? (in rootston)
	if (width == 0 && height == 0) {
		return;
	}
	e->width = width;
	e->height = height;
}

static void xdg_toplevel_handle_close(void *data,
		struct xdg_toplevel *xdg_toplevel) {
	egl_finish();
	exit(EXIT_SUCCESS);
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
	.configure = xdg_toplevel_handle_configure,
	.close = xdg_toplevel_handle_close,
};


/**
 * zwp_locked_pointer handling
 *
 * Pointer unlocks need to be handled properly since the relative pointer needs
 * to be released as well. Unlocks happen when the focus is changed, for
 * example.
 */

static void locked_pointer_handle_locked(void *data,
		struct zwp_locked_pointer_v1 *zwp_locked_pointer_v1) {
	/* This space intentionally left blank */
}

static void locked_pointer_handle_unlocked(void *data,
		struct zwp_locked_pointer_v1 *zwp_locked_pointer_v1) {
	struct window *w = data;
	/* The locked pointer doesn't need to be destroyed since it was oneshot */
	w->locked_pointer = NULL;
	if (w->relative_pointer) {
		/* Destroy the relative pointer */
		zwp_relative_pointer_v1_destroy(w->relative_pointer);
		w->relative_pointer = NULL;
	}
	draw_init(w->egl_info);
	draw_background(w->egl_info);
	draw_end(w->egl_info);
}

static const struct zwp_locked_pointer_v1_listener locked_pointer_listener = {
	.locked = locked_pointer_handle_locked,
	.unlocked = locked_pointer_handle_unlocked,
};


/**
 * zwp_relative_pointer handling
 *
 * Handling relative_motion events is the meat of the code. The handler simply
 * tries to indicate what the deltas look like.
 */

static void relative_pointer_handle_relative_motion(void *data,
		struct zwp_relative_pointer_v1 *zwp_relative_pointer_v1,
		uint32_t utime_hi, uint32_t utime_lo,
		wl_fixed_t dx, wl_fixed_t dy,
		wl_fixed_t dx_unaccel, wl_fixed_t dy_unaccel) {
	struct window *w = data;

	/**
	 * This renders the last location of the pointer (before it as locked), as
	 * well as what the position would have been after the given relative
	 * motion. Note that, if the device sends absolute motion events, the
	 * cursor location after relative motion is always identical to the actual
	 * cursor position.
	 */
	uint64_t utime = (((uint64_t) utime_hi << 32) + utime_lo) / 1000;
	if (utime - w->last_draw > 30 && w->egl_info->frame_callback == NULL) {
		w->last_draw = utime;

		struct egl_info *e = w->egl_info;
		draw_init(e);
		draw_background(e);
		draw_cursor(e, w->pointer_x, w->pointer_y);
		draw_relative_cursor(e, w->pointer_x + wl_fixed_to_int(dx),
			w->pointer_y + wl_fixed_to_int(dy));
		draw_end(e);
	}
}

static const struct zwp_relative_pointer_v1_listener relative_pointer_listener = {
	.relative_motion = relative_pointer_handle_relative_motion,
};


/**
 * wl_pointer handling
 *
 * The client toggles between locking the pointer and receiving relative motion
 * events, and releasing the locked pointer and falling back to normal motion
 * events, on a mouse button one (left mouse button) click.
 *
 * It additionally removes the cursor image, and indicates the pointer location
 * via a small white box.
 */

static void pointer_handle_button(void *data, struct wl_pointer *pointer,
		uint32_t serial, uint32_t time, uint32_t button, uint32_t state_w) {

	struct window *w = data;
	struct egl_info *e = w->egl_info;

	if (button == BTN_LEFT && state_w == WL_POINTER_BUTTON_STATE_PRESSED) {
		if (w->locked_pointer == NULL && w->relative_pointer == NULL) {
			/* Get a locked pointer and add listener */
			w->locked_pointer = zwp_pointer_constraints_v1_lock_pointer(
				pointer_constraints, w->egl_info->surface, w->pointer, NULL,
				ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_ONESHOT);
			zwp_locked_pointer_v1_add_listener(w->locked_pointer,
				&locked_pointer_listener, w);

			/* Get relative pointer and add listener */
			w->relative_pointer = zwp_relative_pointer_manager_v1_get_relative_pointer(
				relative_pointer_manager, w->pointer);
			zwp_relative_pointer_v1_add_listener(w->relative_pointer,
				&relative_pointer_listener, w);

			/* Commit the surface and render */
			wl_surface_commit(e->surface);

			draw_init(e);
			draw_background(e);
			draw_cursor(e, w->pointer_x, w->pointer_y);
			draw_end(e);
		} else if (w->locked_pointer && w->relative_pointer) {
			/* Destroy the locked pointer */
			zwp_locked_pointer_v1_destroy(w->locked_pointer);
			w->locked_pointer = NULL;

			/* Destroy the relative pointer */
			zwp_relative_pointer_v1_destroy(w->relative_pointer);
			w->relative_pointer = NULL;

			/* Render */
			draw_init(e);
			draw_background(e);
			draw_cursor(e, w->pointer_x, w->pointer_y);
			draw_end(e);
		} else {
			fprintf(stderr, "Unknown state!\n");
			exit(EXIT_FAILURE);
		}
	}
}

static void pointer_handle_enter(void *data, struct wl_pointer *wl_pointer,
		uint32_t serial, struct wl_surface *surface,
		wl_fixed_t surface_x, wl_fixed_t surface_y) {
	struct window *w = data;
	wl_pointer_set_cursor(w->pointer, serial, NULL, 0, 0);
}

static void pointer_handle_leave(void *data, struct wl_pointer *wl_pointer,
		uint32_t serial, struct wl_surface *surface) {
	/* This space intentionally left blank */
}

static void pointer_handle_motion(void *data, struct wl_pointer *wl_pointer,
		uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y) {
	struct window *w = data;
	struct egl_info *e = w->egl_info;
	w->pointer_x = wl_fixed_to_int(surface_x);
	w->pointer_y = wl_fixed_to_int(surface_y);
	if (time - w->last_draw > 30 && e->frame_callback == NULL) {
		w->last_draw = time;
		draw_init(e);
		draw_background(e);
		draw_cursor(e, w->pointer_x, w->pointer_y);
		draw_end(e);
	}
}

static void pointer_handle_axis(void *data, struct wl_pointer *wl_pointer,
		uint32_t time, uint32_t axis, wl_fixed_t value) {
	/* This space intentionally left blank */
}

static void pointer_handle_frame(void *data, struct wl_pointer *wl_pointer) {
	/* This space intentionally left blank */
}

static void pointer_handle_axis_source(void *data,
		struct wl_pointer *wl_pointer, uint32_t axis_source) {
	/* This space intentionally left blank */
}

static void pointer_handle_axis_stop(void *data,
		struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis) {
	/* This space intentionally left blank */
}

static void pointer_handle_axis_discrete(void *data,
		struct wl_pointer *wl_pointer, uint32_t axis, int32_t discrete) {
	/* This space intentionally left blank */
}

static const struct wl_pointer_listener pointer_listener = {
	.enter = pointer_handle_enter,
	.leave = pointer_handle_leave,
	.motion = pointer_handle_motion,
	.button = pointer_handle_button,
	.axis = pointer_handle_axis,
	.frame = pointer_handle_frame,
	.axis_source = pointer_handle_axis_source,
	.axis_stop = pointer_handle_axis_stop,
	.axis_discrete = pointer_handle_axis_discrete,
};


/**
 * relative-pointer:
 *
 * This client servers as an example for the relative-pointer-v1 protocol, and
 * to some extent the pointer-constraints protocol as well (the locked_pointer
 * interface).
 *
 * The intended behavior is the following. In the default state, the client
 * shows a black background, and renders the pointer location as a small white
 * box. No cursor is shown. In the locked state, the pointer is locked and the
 * client only listens for relative motion events, which are rendered relative
 * to the last unlocked pointer location by a small red box. Clicking with the
 * left mouse button toggles the state.
 *
 * Most of the code is standard. The interesting stuff happens in "wl_pointer
 * handling" (toggling states), and "zwp_relative_pointer handling" (rendering
 * relative motion events).
 */

int main(int argc, char **argv) {

	/* Connect to the display */

	struct wl_display *display = wl_display_connect(NULL);
	if (display == NULL) {
		fprintf(stderr, "Could not connect to a Wayland display\n");
		return EXIT_FAILURE;
	}

	/* Get the registry and set listeners */

	struct wl_registry *registry = wl_display_get_registry(display);
	wl_registry_add_listener(registry, &registry_listener, NULL);
	wl_display_roundtrip(display);

	/* Check that all the global interfaces were captured */

	if (compositor == NULL) {
		fprintf(stderr, "wl_compositor not available\n");
		return EXIT_FAILURE;
	}
	if (seat == NULL) {
		fprintf(stderr, "wl_seat not available\n");
		return EXIT_FAILURE;
	}
	if (wm_base == NULL) {
		fprintf(stderr, "xdg_wm_base not available\n");
		return EXIT_FAILURE;
	}
	if (pointer_constraints == NULL) {
		fprintf(stderr, "zwp_pointer_constraints_v1 not available\n");
		return EXIT_FAILURE;
	}
	if (relative_pointer_manager == NULL) {
		fprintf(stderr, "zwp_relative_pointer_manager_v1 not available\n");
		return EXIT_FAILURE;
	}

	/* Initialize EGL context */

	struct egl_info *e = calloc(1, sizeof(struct egl_info));
	e->width = e->height = 512;

	egl_init(display);

	/* Create the surface and xdg_toplevels, and set listeners */

	struct wl_surface *surface = wl_compositor_create_surface(compositor);
	struct xdg_surface *xdg_surface =
		xdg_wm_base_get_xdg_surface(wm_base, surface);
	struct xdg_toplevel *xdg_toplevel = xdg_surface_get_toplevel(xdg_surface);

	xdg_surface_add_listener(xdg_surface, &xdg_surface_listener, e);
	xdg_toplevel_add_listener(xdg_toplevel, &xdg_toplevel_listener, e);

	/* Create the egl window and surface */

	wl_surface_commit(surface);

	e->egl_window = wl_egl_window_create(surface, e->width, e->height);
	e->egl_surface = eglCreatePlatformWindowSurfaceEXT(
		egl_display, egl_config, e->egl_window, NULL);
	e->surface = surface;

	wl_display_roundtrip(display);

	/* Setup global state and render */

	struct window *w = calloc(1, sizeof(struct window));
	w->egl_info = e;

	draw_init(e);
	draw_background(e);
	draw_end(e);

	/* Setup the pointer */

	struct wl_pointer *pointer = wl_seat_get_pointer(seat);
	wl_pointer_add_listener(pointer, &pointer_listener, w);

	w->pointer = pointer;

	/* Run display */

	while (wl_display_dispatch(display) != -1) {
		/* This space intentionally left blank */
	}

	return EXIT_SUCCESS;
}
