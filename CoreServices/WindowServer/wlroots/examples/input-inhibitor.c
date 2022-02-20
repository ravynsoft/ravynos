#include <GLES2/gl2.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wayland-client.h>
#include <wayland-egl.h>
#include "egl_common.h"
#include "wlr-input-inhibitor-unstable-v1-client-protocol.h"
#include "xdg-shell-client-protocol.h"

static int width = 500, height = 300;
static int keys = 0;

static struct wl_compositor *compositor = NULL;
static struct wl_seat *seat = NULL;
static struct xdg_wm_base *wm_base = NULL;
static struct zwlr_input_inhibit_manager_v1 *input_inhibit_manager = NULL;
static struct zwlr_input_inhibitor_v1 *input_inhibitor = NULL;

struct wl_egl_window *egl_window;
struct wlr_egl_surface *egl_surface;

static void render_frame(void) {
	eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);

	glViewport(0, 0, width, height);
	if (keys) {
		glClearColor(1.0, 1.0, 1.0, 1.0);
	} else {
		glClearColor(0.8, 0.4, 1.0, 1.0);
	}
	glClear(GL_COLOR_BUFFER_BIT);

	eglSwapBuffers(egl_display, egl_surface);
}

static void xdg_surface_handle_configure(void *data,
		struct xdg_surface *xdg_surface, uint32_t serial) {
	xdg_surface_ack_configure(xdg_surface, serial);
	wl_egl_window_resize(egl_window, width, height, 0, 0);
	render_frame();
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
	exit(0);
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
	.configure = xdg_toplevel_handle_configure,
	.close = xdg_toplevel_handle_close,
};

static void wl_keyboard_keymap(void *data, struct wl_keyboard *wl_keyboard,
		uint32_t format, int32_t fd, uint32_t size) {
}
static void wl_keyboard_enter(void *data, struct wl_keyboard *wl_keyboard,
		uint32_t serial, struct wl_surface *surface, struct wl_array *keys) {
}
static void wl_keyboard_leave(void *data, struct wl_keyboard *wl_keyboard,
		uint32_t serial, struct wl_surface *surface) {
}
static void wl_keyboard_modifiers(void *data, struct wl_keyboard *wl_keyboard,
		uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched,
		uint32_t mods_locked, uint32_t group) {
}
static void wl_keyboard_repeat_info(void *data, struct wl_keyboard *wl_keyboard,
		int32_t rate, int32_t delay) {
}

static void wl_keyboard_key(void *data, struct wl_keyboard *wl_keyboard,
		uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
	if (state == WL_KEYBOARD_KEY_STATE_PRESSED) {
		++keys;
	} else {
		--keys;
	}
	render_frame();
}

static struct wl_keyboard_listener keyboard_listener = {
	.keymap = wl_keyboard_keymap,
	.enter = wl_keyboard_enter,
	.leave = wl_keyboard_leave,
	.key = wl_keyboard_key,
	.modifiers = wl_keyboard_modifiers,
	.repeat_info = wl_keyboard_repeat_info,
};

static void seat_handle_capabilities(void *data, struct wl_seat *wl_seat,
		enum wl_seat_capability caps) {
	if ((caps & WL_SEAT_CAPABILITY_KEYBOARD)) {
		struct wl_keyboard *keyboard = wl_seat_get_keyboard(wl_seat);
		wl_keyboard_add_listener(keyboard, &keyboard_listener, NULL);
	}
}

static void seat_handle_name(void *data, struct wl_seat *wl_seat,
		const char *name) {
	// Who cares
}

const struct wl_seat_listener seat_listener = {
	.capabilities = seat_handle_capabilities,
	.name = seat_handle_name,
};

static void handle_global(void *data, struct wl_registry *registry,
		uint32_t name, const char *interface, uint32_t version) {
	if (strcmp(interface, wl_compositor_interface.name) == 0) {
		compositor = wl_registry_bind(registry, name,
			&wl_compositor_interface, 1);
	} else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
		wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
	} else if (strcmp(interface, zwlr_input_inhibit_manager_v1_interface.name) == 0) {
		input_inhibit_manager = wl_registry_bind(registry, name,
			&zwlr_input_inhibit_manager_v1_interface, 1);
	} else if (strcmp(interface, wl_seat_interface.name) == 0) {
		seat = wl_registry_bind(registry, name, &wl_seat_interface, version);
		wl_seat_add_listener(seat, &seat_listener, seat);
	}
}

static void handle_global_remove(void *data, struct wl_registry *registry,
		uint32_t name) {
	// who cares
}

static const struct wl_registry_listener registry_listener = {
	.global = handle_global,
	.global_remove = handle_global_remove,
};

int main(int argc, char **argv) {
	struct wl_display *display = wl_display_connect(NULL);
	assert(display);
	struct wl_registry *registry = wl_display_get_registry(display);
	assert(registry);
	wl_registry_add_listener(registry, &registry_listener, NULL);
	wl_display_roundtrip(display);
	assert(compositor && seat && wm_base && input_inhibit_manager);

	input_inhibitor = zwlr_input_inhibit_manager_v1_get_inhibitor(
			input_inhibit_manager);
	assert(input_inhibitor);

	egl_init(display);

	struct wl_surface *surface = wl_compositor_create_surface(compositor);
	assert(surface);
	struct xdg_surface *xdg_surface =
		xdg_wm_base_get_xdg_surface(wm_base, surface);
	assert(xdg_surface);
	struct xdg_toplevel *xdg_toplevel = xdg_surface_get_toplevel(xdg_surface);
	assert(xdg_toplevel);

	xdg_surface_add_listener(xdg_surface, &xdg_surface_listener, NULL);
	xdg_toplevel_add_listener(xdg_toplevel, &xdg_toplevel_listener, NULL);

	wl_surface_commit(surface);

	egl_window = wl_egl_window_create(surface, width, height);
	egl_surface = eglCreatePlatformWindowSurfaceEXT(
		egl_display, egl_config, egl_window, NULL);

	wl_display_roundtrip(display);

	render_frame();

	while (wl_display_dispatch(display) != -1) {
		// This space intentionally left blank
	}

	return 0;
}
