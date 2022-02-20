#include <GLES2/gl2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-client.h>
#include <wayland-egl.h>
#include "egl_common.h"
#include "xdg-shell-client-protocol.h"
#include "xdg-decoration-unstable-v1-client-protocol.h"

/**
 * Usage: toplevel-decoration [mode]
 * Creates an xdg-toplevel supporting decoration negotiation. If `mode` is
 * specified, the client will prefer this decoration mode.
 */

static int width = 500, height = 300;

static struct wl_compositor *compositor = NULL;
static struct xdg_wm_base *wm_base = NULL;
static struct zxdg_decoration_manager_v1 *decoration_manager = NULL;

struct wl_egl_window *egl_window;
struct wlr_egl_surface *egl_surface;

struct zxdg_toplevel_decoration_v1 *decoration;
enum zxdg_toplevel_decoration_v1_mode client_preferred_mode, current_mode;

static const char *get_mode_name(enum zxdg_toplevel_decoration_v1_mode mode) {
	switch (mode) {
	case ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE:
		return "client-side decorations";
	case ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE:
		return "server-side decorations";
	}
	abort();
}

static void request_preferred_mode(void) {
	enum zxdg_toplevel_decoration_v1_mode mode = client_preferred_mode;
	if (mode == 0) {
		printf("Requesting compositor preferred mode\n");
		zxdg_toplevel_decoration_v1_unset_mode(decoration);
		return;
	}
	if (mode == current_mode) {
		return;
	}

	printf("Requesting %s\n", get_mode_name(mode));
	zxdg_toplevel_decoration_v1_set_mode(decoration, mode);
}

static void draw(void) {
	eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);

	float color[] = {1.0, 1.0, 0.0, 1.0};
	if (current_mode == ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE) {
		color[0] = 0.0;
	}

	glViewport(0, 0, width, height);
	glClearColor(color[0], color[1], color[2], 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	eglSwapBuffers(egl_display, egl_surface);
}

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

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
	.configure = xdg_toplevel_handle_configure,
};

static void decoration_handle_configure(void *data,
		struct zxdg_toplevel_decoration_v1 *decoration,
		enum zxdg_toplevel_decoration_v1_mode mode) {
	printf("Using %s\n", get_mode_name(mode));
	current_mode = mode;
}

static const struct zxdg_toplevel_decoration_v1_listener decoration_listener = {
	.configure = decoration_handle_configure,
};

static void pointer_handle_enter(void *data, struct wl_pointer *pointer,
		uint32_t serial, struct wl_surface *surface,
		wl_fixed_t sx, wl_fixed_t sy) {
	// No-op
}

static void pointer_handle_leave(void *data, struct wl_pointer *pointer,
		uint32_t serial, struct wl_surface *surface) {
	// No-op
}

static void pointer_handle_motion(void *data, struct wl_pointer *pointer,
		uint32_t time, wl_fixed_t sx, wl_fixed_t sy) {
	// No-op
}

static void pointer_handle_button(void *data, struct wl_pointer *wl_pointer,
		uint32_t serial, uint32_t time, uint32_t button,
		enum wl_pointer_button_state state) {
	if (state == WL_POINTER_BUTTON_STATE_PRESSED) {
		// Toggle mode
		if (client_preferred_mode == ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE) {
			client_preferred_mode = ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE;
		} else {
			client_preferred_mode = ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE;
		}
		request_preferred_mode();
	}
}

static void pointer_handle_axis(void *data, struct wl_pointer *wl_pointer,
		uint32_t time, enum wl_pointer_axis axis, wl_fixed_t value) {
	// No-op
}

static const struct wl_pointer_listener pointer_listener = {
	.enter = pointer_handle_enter,
	.leave = pointer_handle_leave,
	.motion = pointer_handle_motion,
	.button = pointer_handle_button,
	.axis = pointer_handle_axis,
};

static void seat_handle_capabilities(void *data, struct wl_seat *seat,
		enum wl_seat_capability caps) {
	if (caps & WL_SEAT_CAPABILITY_POINTER) {
		struct wl_pointer *pointer = wl_seat_get_pointer(seat);
		wl_pointer_add_listener(pointer, &pointer_listener, NULL);
	}
}

static const struct wl_seat_listener seat_listener = {
	.capabilities = seat_handle_capabilities,
};

static void handle_global(void *data, struct wl_registry *registry,
		uint32_t name, const char *interface, uint32_t version) {
	if (strcmp(interface, wl_compositor_interface.name) == 0) {
		compositor = wl_registry_bind(registry, name, &wl_compositor_interface,
			1);
	} else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
		wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
	} else if (strcmp(interface, zxdg_decoration_manager_v1_interface.name) == 0) {
		decoration_manager = wl_registry_bind(registry, name,
			&zxdg_decoration_manager_v1_interface, 1);
	} else if (strcmp(interface, wl_seat_interface.name) == 0) {
		struct wl_seat *seat =
			wl_registry_bind(registry, name, &wl_seat_interface, 1);
		wl_seat_add_listener(seat, &seat_listener, NULL);
	}
}

static void handle_global_remove(void *data, struct wl_registry *registry,
		uint32_t name) {
	// Who cares?
}

static const struct wl_registry_listener registry_listener = {
	.global = handle_global,
	.global_remove = handle_global_remove,
};

int main(int argc, char **argv) {
	if (argc == 2) {
		char *mode = argv[1];
		if (strcmp(mode, "client") == 0) {
			client_preferred_mode = ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE;
		} else if (strcmp(mode, "server") == 0) {
			client_preferred_mode = ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE;
		} else {
			fprintf(stderr, "Invalid decoration mode\n");
			return EXIT_FAILURE;
		}
	}

	struct wl_display *display = wl_display_connect(NULL);
	if (display == NULL) {
		fprintf(stderr, "Failed to create display\n");
		return EXIT_FAILURE;
	}

	struct wl_registry *registry = wl_display_get_registry(display);
	wl_registry_add_listener(registry, &registry_listener, NULL);
	wl_display_roundtrip(display);

	if (compositor == NULL) {
		fprintf(stderr, "wl-compositor not available\n");
		return EXIT_FAILURE;
	}
	if (wm_base == NULL) {
		fprintf(stderr, "xdg-shell not available\n");
		return EXIT_FAILURE;
	}
	if (decoration_manager == NULL) {
		fprintf(stderr, "xdg-decoration not available\n");
		return EXIT_FAILURE;
	}

	egl_init(display);

	struct wl_surface *surface = wl_compositor_create_surface(compositor);
	struct xdg_surface *xdg_surface =
		xdg_wm_base_get_xdg_surface(wm_base, surface);
	struct xdg_toplevel *xdg_toplevel = xdg_surface_get_toplevel(xdg_surface);
	decoration = zxdg_decoration_manager_v1_get_toplevel_decoration(
		decoration_manager, xdg_toplevel);

	wl_display_roundtrip(display);
	request_preferred_mode();

	xdg_surface_add_listener(xdg_surface, &xdg_surface_listener, NULL);
	xdg_toplevel_add_listener(xdg_toplevel, &xdg_toplevel_listener, NULL);
	zxdg_toplevel_decoration_v1_add_listener(decoration, &decoration_listener,
		NULL);
	wl_surface_commit(surface);

	egl_window = wl_egl_window_create(surface, width, height);
	egl_surface = eglCreatePlatformWindowSurfaceEXT(
		egl_display, egl_config, egl_window, NULL);

	wl_display_roundtrip(display);

	draw();

	while (wl_display_dispatch(display) != -1) {
		// This space is intentionally left blank
	}

	return EXIT_SUCCESS;
}
