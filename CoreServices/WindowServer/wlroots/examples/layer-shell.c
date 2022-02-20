#define _POSIX_C_SOURCE 200112L
#include <linux/input-event-codes.h>
#include <assert.h>
#include <GLES2/gl2.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <wayland-client.h>
#include <wayland-cursor.h>
#include <wayland-egl.h>
#include "egl_common.h"
#include "wlr-layer-shell-unstable-v1-client-protocol.h"
#include "xdg-shell-client-protocol.h"

static struct wl_display *display;
static struct wl_compositor *compositor;
static struct wl_seat *seat;
static struct wl_shm *shm;
static struct wl_pointer *pointer;
static struct wl_keyboard *keyboard;
static struct xdg_wm_base *xdg_wm_base;
static struct zwlr_layer_shell_v1 *layer_shell;

struct zwlr_layer_surface_v1 *layer_surface;
static struct wl_output *wl_output;

struct wl_surface *wl_surface;
struct wl_egl_window *egl_window;
struct wlr_egl_surface *egl_surface;
struct wl_callback *frame_callback;

static uint32_t output = UINT32_MAX;
struct xdg_popup *popup;
struct wl_surface *popup_wl_surface;
struct wl_egl_window *popup_egl_window;
static uint32_t popup_width = 256, popup_height = 256;
struct wlr_egl_surface *popup_egl_surface;
struct wl_callback *popup_frame_callback;
float popup_alpha = 1.0, popup_red = 0.5f;

static uint32_t layer = ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND;
static uint32_t anchor = 0;
static uint32_t width = 256, height = 256;
static int32_t margin_top = 0;
static double alpha = 1.0;
static bool run_display = true;
static bool animate = false;
static enum zwlr_layer_surface_v1_keyboard_interactivity keyboard_interactive =
	ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_NONE;
static double frame = 0;
static int cur_x = -1, cur_y = -1;
static int buttons = 0;

struct wl_cursor_image *cursor_image;
struct wl_cursor_image *popup_cursor_image;
struct wl_surface *cursor_surface, *input_surface;

static struct {
	struct timespec last_frame;
	float color[3];
	int dec;
} demo;

static void draw(void);
static void draw_popup(void);

static void surface_frame_callback(
		void *data, struct wl_callback *cb, uint32_t time) {
	wl_callback_destroy(cb);
	frame_callback = NULL;
	draw();
}

static struct wl_callback_listener frame_listener = {
	.done = surface_frame_callback
};

static void popup_surface_frame_callback(
		void *data, struct wl_callback *cb, uint32_t time) {
	wl_callback_destroy(cb);
	popup_frame_callback = NULL;
	if (popup) {
		draw_popup();
	}
}

static struct wl_callback_listener popup_frame_listener = {
	.done = popup_surface_frame_callback
};

static void draw(void) {
	eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);

	long ms = (ts.tv_sec - demo.last_frame.tv_sec) * 1000 +
		(ts.tv_nsec - demo.last_frame.tv_nsec) / 1000000;
	int inc = (demo.dec + 1) % 3;

	if (!buttons) {
		demo.color[inc] += ms / 2000.0f;
		demo.color[demo.dec] -= ms / 2000.0f;

		if (demo.color[demo.dec] < 0.0f) {
			demo.color[inc] = 1.0f;
			demo.color[demo.dec] = 0.0f;
			demo.dec = inc;
		}
	}

	if (animate) {
		frame += ms / 50.0;
		int32_t old_top = margin_top;
		margin_top = -(20 - ((int)frame % 20));
		if (old_top != margin_top) {
			zwlr_layer_surface_v1_set_margin(layer_surface,
					margin_top, 0, 0, 0);
			wl_surface_commit(wl_surface);
		}
	}

	glViewport(0, 0, width, height);
	if (buttons) {
		glClearColor(1, 1, 1, alpha);
	} else {
		glClearColor(demo.color[0], demo.color[1], demo.color[2], alpha);
	}
	glClear(GL_COLOR_BUFFER_BIT);

	if (cur_x != -1 && cur_y != -1) {
		glEnable(GL_SCISSOR_TEST);
		glScissor(cur_x, height - cur_y, 5, 5);
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_SCISSOR_TEST);
	}

	frame_callback = wl_surface_frame(wl_surface);
	wl_callback_add_listener(frame_callback, &frame_listener, NULL);

	eglSwapBuffers(egl_display, egl_surface);

	demo.last_frame = ts;
}

static void draw_popup(void) {
	static float alpha_mod = -0.01;

	eglMakeCurrent(egl_display, popup_egl_surface, popup_egl_surface, egl_context);
	glViewport(0, 0, popup_width, popup_height);
	glClearColor(popup_red, 0.5f, 0.5f, popup_alpha);
	popup_alpha += alpha_mod;
	if (popup_alpha < 0.01 || popup_alpha >= 1.0f) {
		alpha_mod *= -1.0;
	}
	glClear(GL_COLOR_BUFFER_BIT);

	popup_frame_callback = wl_surface_frame(popup_wl_surface);
	assert(popup_frame_callback);
	wl_callback_add_listener(popup_frame_callback, &popup_frame_listener, NULL);
	eglSwapBuffers(egl_display, popup_egl_surface);
	wl_surface_commit(popup_wl_surface);
}

static void xdg_surface_handle_configure(void *data,
		struct xdg_surface *xdg_surface, uint32_t serial) {
	xdg_surface_ack_configure(xdg_surface, serial);
}

static const struct xdg_surface_listener xdg_surface_listener = {
	.configure = xdg_surface_handle_configure,
};

static void xdg_popup_configure(void *data, struct xdg_popup *xdg_popup,
		int32_t x, int32_t y, int32_t width, int32_t height) {
	fprintf(stderr, "Popup configured %dx%d@%d,%d\n", width, height, x, y);
	popup_width = width;
	popup_height = height;
	if (popup_egl_window) {
		wl_egl_window_resize(popup_egl_window, width, height, 0, 0);
	}
}

static void popup_destroy(void) {
	eglDestroySurface(egl_display, popup_egl_surface);
	wl_egl_window_destroy(popup_egl_window);
	xdg_popup_destroy(popup);
	wl_surface_destroy(popup_wl_surface);
	popup_wl_surface = NULL;
	popup = NULL;
	popup_egl_window = NULL;
}

static void xdg_popup_done(void *data, struct xdg_popup *xdg_popup) {
	fprintf(stderr, "Popup done\n");
	popup_destroy();
}

static const struct xdg_popup_listener xdg_popup_listener = {
	.configure = xdg_popup_configure,
	.popup_done = xdg_popup_done,
};

static void create_popup(uint32_t serial) {
	if (popup) {
		return;
	}
	struct wl_surface *surface = wl_compositor_create_surface(compositor);
	assert(xdg_wm_base && surface);
	struct xdg_surface *xdg_surface =
		xdg_wm_base_get_xdg_surface(xdg_wm_base, surface);
	struct xdg_positioner *xdg_positioner =
		xdg_wm_base_create_positioner(xdg_wm_base);
	assert(xdg_surface && xdg_positioner);

	xdg_positioner_set_size(xdg_positioner, popup_width, popup_height);
	xdg_positioner_set_offset(xdg_positioner, 0, 0);
	xdg_positioner_set_anchor_rect(xdg_positioner, cur_x, cur_y, 1, 1);
	xdg_positioner_set_anchor(xdg_positioner, XDG_POSITIONER_ANCHOR_BOTTOM_RIGHT);

	popup = xdg_surface_get_popup(xdg_surface, NULL, xdg_positioner);
	xdg_popup_grab(popup, seat, serial);

	assert(popup);

	zwlr_layer_surface_v1_get_popup(layer_surface, popup);

	xdg_surface_add_listener(xdg_surface, &xdg_surface_listener, NULL);
	xdg_popup_add_listener(popup, &xdg_popup_listener, NULL);

	wl_surface_commit(surface);
	wl_display_roundtrip(display);

	xdg_positioner_destroy(xdg_positioner);

	popup_wl_surface = surface;
	popup_egl_window = wl_egl_window_create(surface, popup_width, popup_height);
	assert(popup_egl_window);
	popup_egl_surface = eglCreatePlatformWindowSurfaceEXT(
			egl_display, egl_config, popup_egl_window, NULL);
	assert(popup_egl_surface != EGL_NO_SURFACE);
	draw_popup();
}

static void layer_surface_configure(void *data,
		struct zwlr_layer_surface_v1 *surface,
		uint32_t serial, uint32_t w, uint32_t h) {
	width = w;
	height = h;
	if (egl_window) {
		wl_egl_window_resize(egl_window, width, height, 0, 0);
	}
	zwlr_layer_surface_v1_ack_configure(surface, serial);
}

static void layer_surface_closed(void *data,
		struct zwlr_layer_surface_v1 *surface) {
	eglDestroySurface(egl_display, egl_surface);
	wl_egl_window_destroy(egl_window);
	zwlr_layer_surface_v1_destroy(surface);
	wl_surface_destroy(wl_surface);
	run_display = false;
}

struct zwlr_layer_surface_v1_listener layer_surface_listener = {
	.configure = layer_surface_configure,
	.closed = layer_surface_closed,
};

static void wl_pointer_enter(void *data, struct wl_pointer *wl_pointer,
		uint32_t serial, struct wl_surface *surface,
		wl_fixed_t surface_x, wl_fixed_t surface_y) {
	struct wl_cursor_image *image;
	if (surface == popup_wl_surface) {
		image = popup_cursor_image;
	} else {
		image = cursor_image;
	}
	wl_surface_attach(cursor_surface,
		wl_cursor_image_get_buffer(image), 0, 0);
	wl_surface_damage(cursor_surface, 1, 0,
		image->width, image->height);
	wl_surface_commit(cursor_surface);
	wl_pointer_set_cursor(wl_pointer, serial, cursor_surface,
		image->hotspot_x, image->hotspot_y);
	input_surface = surface;
}

static void wl_pointer_leave(void *data, struct wl_pointer *wl_pointer,
		uint32_t serial, struct wl_surface *surface) {
	cur_x = cur_y = -1;
	buttons = 0;
}

static void wl_pointer_motion(void *data, struct wl_pointer *wl_pointer,
		uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y) {
	cur_x = wl_fixed_to_int(surface_x);
	cur_y = wl_fixed_to_int(surface_y);
}

static void wl_pointer_button(void *data, struct wl_pointer *wl_pointer,
		uint32_t serial, uint32_t time, uint32_t button, uint32_t state) {
	if (input_surface == wl_surface) {
		if (state == WL_POINTER_BUTTON_STATE_PRESSED) {
			if (button == BTN_RIGHT) {
				if (popup_wl_surface) {
					popup_destroy();
				} else {
					create_popup(serial);
				}
			} else {
				buttons++;
			}
		} else {
			if (button != BTN_RIGHT) {
				buttons--;
			}
		}
	} else if (input_surface == popup_wl_surface) {
		if (state == WL_POINTER_BUTTON_STATE_PRESSED) {
			if (button == BTN_LEFT && popup_red <= 0.9f) {
				popup_red += 0.1;
			} else if (button == BTN_RIGHT && popup_red >= 0.1f) {
				popup_red -= 0.1;
			}
		}
	} else {
		assert(false && "Unknown surface");
	}
}

static void wl_pointer_axis(void *data, struct wl_pointer *wl_pointer,
		uint32_t time, uint32_t axis, wl_fixed_t value) {
	// Who cares
}

static void wl_pointer_frame(void *data, struct wl_pointer *wl_pointer) {
	// Who cares
}

static void wl_pointer_axis_source(void *data, struct wl_pointer *wl_pointer,
		uint32_t axis_source) {
	// Who cares
}

static void wl_pointer_axis_stop(void *data, struct wl_pointer *wl_pointer,
		uint32_t time, uint32_t axis) {
	// Who cares
}

static void wl_pointer_axis_discrete(void *data, struct wl_pointer *wl_pointer,
		uint32_t axis, int32_t discrete) {
	// Who cares
}

struct wl_pointer_listener pointer_listener = {
	.enter = wl_pointer_enter,
	.leave = wl_pointer_leave,
	.motion = wl_pointer_motion,
	.button = wl_pointer_button,
	.axis = wl_pointer_axis,
	.frame = wl_pointer_frame,
	.axis_source = wl_pointer_axis_source,
	.axis_stop = wl_pointer_axis_stop,
	.axis_discrete = wl_pointer_axis_discrete,
};

static void wl_keyboard_keymap(void *data, struct wl_keyboard *wl_keyboard,
		uint32_t format, int32_t fd, uint32_t size) {
	// Who cares
}

static void wl_keyboard_enter(void *data, struct wl_keyboard *wl_keyboard,
		uint32_t serial, struct wl_surface *surface, struct wl_array *keys) {
	fprintf(stderr, "Keyboard enter\n");
}

static void wl_keyboard_leave(void *data, struct wl_keyboard *wl_keyboard,
		uint32_t serial, struct wl_surface *surface) {
	fprintf(stderr, "Keyboard leave\n");
}

static void wl_keyboard_key(void *data, struct wl_keyboard *wl_keyboard,
		uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
	fprintf(stderr, "Key event: %d %d\n", key, state);
}

static void wl_keyboard_modifiers(void *data, struct wl_keyboard *wl_keyboard,
		uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched,
		uint32_t mods_locked, uint32_t group) {
	// Who cares
}

static void wl_keyboard_repeat_info(void *data, struct wl_keyboard *wl_keyboard,
		int32_t rate, int32_t delay) {
	// Who cares
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
	if ((caps & WL_SEAT_CAPABILITY_POINTER)) {
		pointer = wl_seat_get_pointer(wl_seat);
		wl_pointer_add_listener(pointer, &pointer_listener, NULL);
	}
	if ((caps & WL_SEAT_CAPABILITY_KEYBOARD)) {
		keyboard = wl_seat_get_keyboard(wl_seat);
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
	} else if (strcmp(interface, wl_shm_interface.name) == 0) {
		shm = wl_registry_bind(registry, name,
				&wl_shm_interface, 1);
	} else if (strcmp(interface, "wl_output") == 0) {
		if (output != UINT32_MAX) {
			if (!wl_output) {
				wl_output = wl_registry_bind(registry, name,
						&wl_output_interface, 1);
			} else {
				output--;
			}
		}
	} else if (strcmp(interface, wl_seat_interface.name) == 0) {
		seat = wl_registry_bind(registry, name,
				&wl_seat_interface, 1);
		wl_seat_add_listener(seat, &seat_listener, NULL);
	} else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
		layer_shell = wl_registry_bind(registry, name,
			&zwlr_layer_shell_v1_interface, version < 4 ? version : 4);
	} else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
		xdg_wm_base = wl_registry_bind(
				registry, name, &xdg_wm_base_interface, 1);
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
	char *namespace = "wlroots";
	int exclusive_zone = 0;
	int32_t margin_right = 0, margin_bottom = 0, margin_left = 0;
	bool found;
	int c;
	while ((c = getopt(argc, argv, "k:nw:h:o:l:a:x:m:t:")) != -1) {
		switch (c) {
		case 'o':
			output = atoi(optarg);
			break;
		case 'w':
			width = atoi(optarg);
			break;
		case 'h':
			height = atoi(optarg);
			break;
		case 'x':
			exclusive_zone = atoi(optarg);
			break;
		case 'l': {
			struct {
				char *name;
				enum zwlr_layer_shell_v1_layer value;
			} layers[] = {
				{ "background", ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND },
				{ "bottom", ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM },
				{ "top", ZWLR_LAYER_SHELL_V1_LAYER_TOP },
				{ "overlay", ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY },
			};
			found = false;
			for (size_t i = 0; i < sizeof(layers) / sizeof(layers[0]); ++i) {
				if (strcmp(optarg, layers[i].name) == 0) {
					layer = layers[i].value;
					found = true;
					break;
				}
			}
			if (!found) {
				fprintf(stderr, "invalid layer %s\n", optarg);
				return 1;
			}
			break;
		}
		case 'a': {
			struct {
				char *name;
				uint32_t value;
			} anchors[] = {
				{ "top", ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP },
				{ "bottom", ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM },
				{ "left", ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT },
				{ "right", ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT },
			};
			found = false;
			for (size_t i = 0; i < sizeof(anchors) / sizeof(anchors[0]); ++i) {
				if (strcmp(optarg, anchors[i].name) == 0) {
					anchor |= anchors[i].value;
					found = true;
					break;
				}
			}
			if (!found) {
				fprintf(stderr, "invalid anchor %s\n", optarg);
				return 1;
			}
			break;
		}
		case 't':
			alpha = atof(optarg);
			break;
		case 'm': {
			char *endptr = optarg;
			margin_top = strtol(endptr, &endptr, 10);
			assert(*endptr == ',');
			margin_right = strtol(endptr + 1, &endptr, 10);
			assert(*endptr == ',');
			margin_bottom = strtol(endptr + 1, &endptr, 10);
			assert(*endptr == ',');
			margin_left = strtol(endptr + 1, &endptr, 10);
			assert(!*endptr);
			break;
		}
		case 'n':
			animate = true;
			break;
		case 'k': {
			const struct {
				const char *name;
				enum zwlr_layer_surface_v1_keyboard_interactivity value;
			} kb_int[] = {
				{ "none", ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_NONE },
				{ "exclusive", ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_EXCLUSIVE },
				{ "on_demand", ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_ON_DEMAND }
			};
			found = false;
			for (size_t i = 0; i < sizeof(kb_int) / sizeof(kb_int[0]); ++i) {
				if (strcmp(optarg, kb_int[i].name) == 0) {
					keyboard_interactive = kb_int[i].value;
					found = true;
					break;
				}
			}
			if (!found) {
				fprintf(stderr, "invalid keyboard interactivity setting %s\n", optarg);
				return 1;
			}
			break;
		}
		default:
			break;
		}
	}

	display = wl_display_connect(NULL);
	if (display == NULL) {
		fprintf(stderr, "Failed to create display\n");
		return 1;
	}

	struct wl_registry *registry = wl_display_get_registry(display);
	wl_registry_add_listener(registry, &registry_listener, NULL);
	wl_display_roundtrip(display);

	if (compositor == NULL) {
		fprintf(stderr, "wl_compositor not available\n");
		return 1;
	}
	if (shm == NULL) {
		fprintf(stderr, "wl_shm not available\n");
		return 1;
	}
	if (layer_shell == NULL) {
		fprintf(stderr, "layer_shell not available\n");
		return 1;
	}

	struct wl_cursor_theme *cursor_theme =
		wl_cursor_theme_load(NULL, 16, shm);
	assert(cursor_theme);
	struct wl_cursor *cursor =
		wl_cursor_theme_get_cursor(cursor_theme, "crosshair");
	if (cursor == NULL) {
		cursor = wl_cursor_theme_get_cursor(cursor_theme, "left_ptr");
	}
	assert(cursor);
	cursor_image = cursor->images[0];

	cursor = wl_cursor_theme_get_cursor(cursor_theme, "tcross");
	if (cursor == NULL) {
		cursor = wl_cursor_theme_get_cursor(cursor_theme, "left_ptr");
	}
	assert(cursor);
	popup_cursor_image = cursor->images[0];

	cursor_surface = wl_compositor_create_surface(compositor);
	assert(cursor_surface);

	egl_init(display);

	wl_surface = wl_compositor_create_surface(compositor);
	assert(wl_surface);

	layer_surface = zwlr_layer_shell_v1_get_layer_surface(layer_shell,
				wl_surface, wl_output, layer, namespace);
	assert(layer_surface);
	zwlr_layer_surface_v1_set_size(layer_surface, width, height);
	zwlr_layer_surface_v1_set_anchor(layer_surface, anchor);
	zwlr_layer_surface_v1_set_exclusive_zone(layer_surface, exclusive_zone);
	zwlr_layer_surface_v1_set_margin(layer_surface,
			margin_top, margin_right, margin_bottom, margin_left);
	zwlr_layer_surface_v1_set_keyboard_interactivity(
			layer_surface, keyboard_interactive);
	zwlr_layer_surface_v1_add_listener(layer_surface,
			&layer_surface_listener, layer_surface);
	wl_surface_commit(wl_surface);
	wl_display_roundtrip(display);

	egl_window = wl_egl_window_create(wl_surface, width, height);
	assert(egl_window);
	egl_surface = eglCreatePlatformWindowSurfaceEXT(
		egl_display, egl_config, egl_window, NULL);
	assert(egl_surface != EGL_NO_SURFACE);

	wl_display_roundtrip(display);
	draw();

	while (wl_display_dispatch(display) != -1 && run_display) {
		// This space intentionally left blank
	}

	wl_cursor_theme_destroy(cursor_theme);
	return 0;
}
