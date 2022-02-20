#include <GLES2/gl2.h>
#include <linux/input-event-codes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-client.h>
#include <wayland-egl.h>
#include "egl_common.h"
#include "xdg-shell-client-protocol.h"
#include "pointer-constraints-unstable-v1-client-protocol.h"

static int width = 512, height = 512;

static struct wl_compositor *compositor = NULL;
static struct wl_seat *seat = NULL;
static struct xdg_wm_base *wm_base = NULL;
static struct zwp_pointer_constraints_v1 *pointer_constraints = NULL;

struct wl_egl_window *egl_window;
struct wlr_egl_surface *egl_surface;
struct zwp_locked_pointer_v1* locked_pointer;
struct zwp_confined_pointer_v1* confined_pointer;

enum {
	REGION_TYPE_NONE,
	REGION_TYPE_DISJOINT,
	REGION_TYPE_JOINT,
	REGION_TYPE_MAX
} region_type = REGION_TYPE_NONE;

struct wl_region *regions[3];

static void draw(void) {
	eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);

	float color[] = {1.0, 1.0, 0.0, 1.0};

	glViewport(0, 0, width, height);
	glClearColor(color[0], color[1], color[2], 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	eglSwapBuffers(egl_display, egl_surface);
}

static void pointer_handle_button(void *data, struct wl_pointer *pointer,
		uint32_t serial, uint32_t time, uint32_t button, uint32_t state_w) {
	struct wl_surface *surface = data;

	if (button == BTN_LEFT && state_w == WL_POINTER_BUTTON_STATE_PRESSED) {
		region_type = (region_type + 1) % REGION_TYPE_MAX;

		if (locked_pointer) {
			zwp_locked_pointer_v1_set_region(locked_pointer,
				regions[region_type]);
		} else if (confined_pointer) {
			zwp_confined_pointer_v1_set_region(confined_pointer,
				regions[region_type]);
		}

		wl_surface_commit(surface);
	}
}

static void pointer_handle_enter(void *data, struct wl_pointer *wl_pointer,
		uint32_t serial, struct wl_surface *surface,
		wl_fixed_t surface_x, wl_fixed_t surface_y) {
	// This space intentionally left blank
}

static void pointer_handle_leave(void *data, struct wl_pointer *wl_pointer,
		uint32_t serial, struct wl_surface *surface) {
	// This space intentionally left blank
}

static void pointer_handle_motion(void *data, struct wl_pointer *wl_pointer,
		uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y) {
	// This space intentionally left blank
}

static void pointer_handle_axis(void *data, struct wl_pointer *wl_pointer,
		uint32_t time, uint32_t axis, wl_fixed_t value) {
	// This space intentionally left blank
}

static void pointer_handle_frame(void *data, struct wl_pointer *wl_pointer) {
	// This space intentionally left blank
}

static void pointer_handle_axis_source(void *data,
		struct wl_pointer *wl_pointer, uint32_t axis_source) {
	// This space intentionally left blank
}

static void pointer_handle_axis_stop(void *data,
		struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis) {
	// This space intentionally left blank
}

static void pointer_handle_axis_discrete(void *data,
		struct wl_pointer *wl_pointer, uint32_t axis, int32_t discrete) {
	// This space intentionally left blank
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

static void xdg_surface_handle_configure(void *data,
		struct xdg_surface *xdg_surface, uint32_t serial) {
	xdg_surface_ack_configure(xdg_surface, serial);
	wl_egl_window_resize(egl_window, width, height, 0, 0);
	draw();
}

static const struct xdg_surface_listener xdg_surface_listener = {
	.configure = xdg_surface_handle_configure,
};

static void xdg_toplevel_handle_configure(void *data,
		struct xdg_toplevel *xdg_toplevel, int32_t w, int32_t h,
		struct wl_array *states) {
	width = w;
	height = h;
}

static void xdg_toplevel_handle_close(void *data,
		struct xdg_toplevel *xdg_toplevel) {
	exit(EXIT_SUCCESS);
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
	.configure = xdg_toplevel_handle_configure,
	.close = xdg_toplevel_handle_close,
};

static void handle_global(void *data, struct wl_registry *registry,
		uint32_t name, const char *interface, uint32_t version) {
	if (strcmp(interface, wl_compositor_interface.name) == 0) {
		compositor = wl_registry_bind(registry, name,
			&wl_compositor_interface, 1);
	} else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
		wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
	} else if (strcmp(interface, wl_seat_interface.name) == 0) {
		seat = wl_registry_bind(registry, name, &wl_seat_interface, version);
	} else if (strcmp(interface,
			zwp_pointer_constraints_v1_interface.name) == 0) {
		pointer_constraints = wl_registry_bind(registry, name,
			&zwp_pointer_constraints_v1_interface, version);
	}
}

static const struct wl_registry_listener registry_listener = {
	.global = handle_global,
	.global_remove = NULL,
};

int main(int argc, char **argv) {
	if (argc != 4) {
		goto invalid_args;
	}

	bool lock;
	if (strcmp(argv[1], "lock") == 0) {
		lock = true;
	} else if (strcmp(argv[1], "confine") == 0) {
		lock = false;
	} else {
		goto invalid_args;
	}

	enum zwp_pointer_constraints_v1_lifetime lifetime;
	if (strcmp(argv[2], "oneshot") == 0) {
		lifetime = ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_ONESHOT;
	} else if (strcmp(argv[2], "persistent") == 0) {
		lifetime = ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT;
	} else {
		goto invalid_args;
	}

	if (strcmp(argv[3], "no-region") == 0) {
		region_type = REGION_TYPE_NONE;
	} else if (strcmp(argv[3], "disjoint-region") == 0) {
		region_type = REGION_TYPE_DISJOINT;
	} else if (strcmp(argv[3], "joint-region") == 0) {
		region_type = REGION_TYPE_JOINT;
	}

	struct wl_display *display = wl_display_connect(NULL);

	struct wl_registry *registry = wl_display_get_registry(display);
	wl_registry_add_listener(registry, &registry_listener, NULL);
	wl_display_roundtrip(display);

	struct wl_region *disjoint_region = wl_compositor_create_region(compositor);
	wl_region_add(disjoint_region, 0, 0, 255, 256);
	wl_region_add(disjoint_region, 257, 0, 255, 256);
	regions[REGION_TYPE_DISJOINT] = disjoint_region;

	struct wl_region *joint_region = wl_compositor_create_region(compositor);
	wl_region_add(joint_region, 0, 0, 256, 256);
	wl_region_add(joint_region, 256, 0, 256, 256);
	wl_region_add(joint_region, 256, 256, 256, 256);
	regions[REGION_TYPE_JOINT] = joint_region;

	egl_init(display);

	struct wl_surface *surface = wl_compositor_create_surface(compositor);
	struct xdg_surface *xdg_surface =
		xdg_wm_base_get_xdg_surface(wm_base, surface);
	struct xdg_toplevel *xdg_toplevel = xdg_surface_get_toplevel(xdg_surface);

	xdg_surface_add_listener(xdg_surface, &xdg_surface_listener, NULL);
	xdg_toplevel_add_listener(xdg_toplevel, &xdg_toplevel_listener, NULL);

	struct wl_pointer *pointer = wl_seat_get_pointer(seat);
	wl_pointer_add_listener(pointer, &pointer_listener, surface);

	if (lock) {
		locked_pointer = zwp_pointer_constraints_v1_lock_pointer(
			pointer_constraints, surface, pointer,
			regions[region_type], lifetime);

		zwp_locked_pointer_v1_set_cursor_position_hint(locked_pointer,
			wl_fixed_from_int(128), wl_fixed_from_int(128));
	} else {
		confined_pointer = zwp_pointer_constraints_v1_confine_pointer(
			pointer_constraints, surface, pointer,
			regions[region_type], lifetime);
	}

	wl_surface_commit(surface);

	egl_window = wl_egl_window_create(surface, width, height);
	egl_surface = eglCreatePlatformWindowSurfaceEXT(
		egl_display, egl_config, egl_window, NULL);

	wl_display_roundtrip(display);

	draw();

	while (wl_display_dispatch(display) != -1) {}

	return EXIT_SUCCESS;

invalid_args:
	fprintf(stderr, "pointer-constraints <lock | confine> "
		"<oneshot | persistent> "
		"<no-region | disjoint-rejoin | joint-region>\n");
	return EXIT_FAILURE;
}
