#define _POSIX_C_SOURCE 200809L
#include <GLES2/gl2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wayland-client.h>
#include <wayland-egl.h>
#include "egl_common.h"
#include "text-input-unstable-v3-client-protocol.h"
#include "xdg-shell-client-protocol.h"

const char usage[] = "Usage: text-input [seconds [width height]]\n\
\n\
Creates a xdg-toplevel using the text-input protocol.\n\
It will be solid black when it has no text input focus, yellow when it\n\
has focus, and red when it was notified that the focus moved away\n\
but still didn't give up the text input ability.\n\
\n\
The \"seconds\" argument is optional and defines the delay between getting\n\
notified of lost focus and releasing text input.\n\
\n\
The \"width\" and \"height\" arguments define the window shape.\n\
\n\
The console will print the internal state of the text field:\n\
- the text in the 1st line\n\
- \".\" under each preedit character\n\
- \"_\" under each selected preedit character\n\
- \"|\" at the cursor position if there are no selected characters in the\n\
preedit.\n\
\n\
The cursor positions may be inaccurate, especially in presence of zero-width\n\
characters or non-monospaced fonts.\n";

struct text_input_state {
	char *commit;
	struct {
		char *text;
		int32_t cursor_begin;
		int32_t cursor_end;
	} preedit;
	struct {
		uint32_t after_length;
		uint32_t before_length;
	} delete_surrounding;
};

static struct text_input_state pending = {0};
static struct text_input_state current = {0};
static bool entered = false;
static uint32_t serial;
static char *buffer; // text buffer
// cursor is not present, there's no way to move it outside of preedit

static int sleeptime = 0;
static int width = 100, height = 200;
static int enabled = 0;

static struct wl_display *display = NULL;
static struct wl_compositor *compositor = NULL;
static struct wl_seat *seat = NULL;
static struct xdg_wm_base *wm_base = NULL;
static struct zwp_text_input_manager_v3 *text_input_manager = NULL;
static struct zwp_text_input_v3 *text_input	= NULL;

struct wl_egl_window *egl_window;
struct wlr_egl_surface *egl_surface;

static void draw(void) {
	eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);

	float color[] = {1.0, 1.0, 0.0, 1.0};
	color[0] = enabled * 1.0;
	color[1] = entered * 1.0;

	glViewport(0, 0, width, height);
	glClearColor(color[0], color[1], color[2], 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	eglSwapBuffers(egl_display, egl_surface);
}

static size_t utf8_strlen(char *str) {
	size_t cp_count = 0;
	for (; *str != '\0'; str++) {
		if ((*str & 0xc0) != 0x80) {
			cp_count++;
		}
	}
	return cp_count;
}

static size_t utf8_offset(char *utf8_str, size_t byte_offset) {
	size_t cp_count = 0;
	for (char *c = utf8_str; c < utf8_str + byte_offset; c++) {
		if ((*c & 0xc0) != 0x80) {
			cp_count++;
		}
	}
	return cp_count;
}

// TODO: would be nicer to have this text display inside the window
static void show_status(void) {
	printf("State %d:", serial);
	if (!enabled) {
		printf(" disabled");
	}

	char *preedit_text = current.preedit.text;
	if (!preedit_text) {
		preedit_text = "";
	}

	printf("\n");
	printf("%s", buffer);
	printf("%s\n", preedit_text);

	// Positioning of the cursor requires UTF8 offsets to match monospaced
	// glyphs
	for (unsigned i = 0; i < utf8_strlen(buffer); i++) {
		printf(" ");
	}
	char *cursor_mark = calloc(utf8_strlen(preedit_text) + 2, sizeof(char));
	for (unsigned i = 0; i < utf8_strlen(preedit_text); i++) {
		cursor_mark[i] = '.';
	}
	if (current.preedit.cursor_begin == -1
			&& current.preedit.cursor_end == -1) {
		goto end;
	}
	if (current.preedit.cursor_begin == -1
			|| current.preedit.cursor_end == -1) {
		printf("Only one cursor side is defined: %d to %d\n",
			current.preedit.cursor_begin, current.preedit.cursor_end);
		goto end;
	}

	if ((unsigned)current.preedit.cursor_begin > strlen(preedit_text)) {
		printf("Cursor out of bounds\n");
		goto end;
	}

	if (current.preedit.cursor_begin == current.preedit.cursor_end) {
		cursor_mark[utf8_offset(preedit_text, current.preedit.cursor_begin)]
			= '|';
		goto print;
	}

	if (current.preedit.cursor_begin > current.preedit.cursor_end) {
		printf("End cursor is before start cursor\n");
		goto end;
	}

	// negative offsets already checked before
	for (unsigned i = utf8_offset(preedit_text, current.preedit.cursor_begin);
			i < utf8_offset(preedit_text, current.preedit.cursor_end); i++) {
		cursor_mark[i] = '_';
	}
print:
	printf("%s\n", cursor_mark);
end:
	free(cursor_mark);
}

static void commit(struct zwp_text_input_v3 *text_input) {
	zwp_text_input_v3_commit(text_input);
	serial++;
}

static void send_status_update(struct zwp_text_input_v3 *text_input) {
	zwp_text_input_v3_set_surrounding_text(text_input, buffer, strlen(buffer), strlen(buffer));
	zwp_text_input_v3_set_text_change_cause(text_input, ZWP_TEXT_INPUT_V3_CHANGE_CAUSE_INPUT_METHOD);
	commit(text_input);
}

static void text_input_handle_enter(void *data,
		struct zwp_text_input_v3 *zwp_text_input_v3,
		struct wl_surface *surface) {
	entered = true;
	zwp_text_input_v3_enable(zwp_text_input_v3);
	commit(zwp_text_input_v3);
	enabled = true;
	draw();
	show_status();
}

static void text_input_handle_leave(void *data,
		struct zwp_text_input_v3 *zwp_text_input_v3,
		struct wl_surface *surface) {
	entered = false;
	draw();
	wl_display_roundtrip(display);
	sleep(sleeptime);
	zwp_text_input_v3_disable(zwp_text_input_v3);
	commit(zwp_text_input_v3);
	enabled = false;
	draw();
	show_status();
}

static void text_input_commit_string(void *data,
		struct zwp_text_input_v3 *zwp_text_input_v3,
		const char *text) {
	free(pending.commit);
	pending.commit = strdup(text);
}

static void text_input_delete_surrounding_text(void *data,
		struct zwp_text_input_v3 *zwp_text_input_v3,
		uint32_t before_length, uint32_t after_length) {
	pending.delete_surrounding.before_length = before_length;
	pending.delete_surrounding.after_length = after_length;
}

static void text_input_preedit_string(void *data,
		struct zwp_text_input_v3 *zwp_text_input_v3,
		const char *text, int32_t cursor_begin, int32_t cursor_end) {
	free(pending.preedit.text);
	pending.preedit.text = strdup(text);
	pending.preedit.cursor_begin = cursor_begin;
	pending.preedit.cursor_end = cursor_end;
}

static void text_input_handle_done(void *data,
		 struct zwp_text_input_v3 *zwp_text_input_v3,
		 uint32_t incoming_serial) {
	if (serial != incoming_serial) {
		fprintf(stderr, "Received serial %d while expecting %d\n", incoming_serial, serial);
		return;
	}
	free(current.preedit.text);
	free(current.commit);
	current = pending;
	struct text_input_state empty = {0};
	pending = empty;

	if (current.delete_surrounding.after_length + current.delete_surrounding.before_length > 0) {
		// cursor is always after committed text, after_length != 0 will never happen
		unsigned delete_before = current.delete_surrounding.before_length;
		if (delete_before > strlen(buffer)) {
			delete_before = strlen(buffer);
		}
		buffer[strlen(buffer) - delete_before] = '\0';
	}

	char *commit_string = current.commit;
	if (!commit_string) {
		commit_string = "";
	}
	char *old_buffer = buffer;
	buffer = calloc(strlen(buffer) + strlen(commit_string) + 1, sizeof(char)); // realloc may fail anyway
	strcpy(buffer, old_buffer);
	free(old_buffer);
	strcat(buffer, commit_string);

	send_status_update(zwp_text_input_v3);
	show_status();
}

static const struct zwp_text_input_v3_listener text_input_listener = {
	.enter = text_input_handle_enter,
	.leave = text_input_handle_leave,
	.commit_string = text_input_commit_string,
	.delete_surrounding_text = text_input_delete_surrounding_text,
	.preedit_string = text_input_preedit_string,
	.done = text_input_handle_done,
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
	if (strcmp(interface, "wl_compositor") == 0) {
		compositor = wl_registry_bind(registry, name,
			&wl_compositor_interface, 1);
	} else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
		wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
	} else if (strcmp(interface, zwp_text_input_manager_v3_interface.name) == 0) {
		text_input_manager = wl_registry_bind(registry, name,
			&zwp_text_input_manager_v3_interface, 1);
	} else if (strcmp(interface, wl_seat_interface.name) == 0) {
		seat = wl_registry_bind(registry, name, &wl_seat_interface, version);
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
	if (argc > 1) {
		if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
			printf(usage);
			return 0;
		}
		sleeptime = atoi(argv[1]);
		if (argc > 3) {
			width = atoi(argv[2]);
			height = atoi(argv[3]);
		}
	}

	buffer = calloc(1, sizeof(char));

	display = wl_display_connect(NULL);
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
	if (text_input_manager == NULL) {
		fprintf(stderr, "text-input not available\n");
		return EXIT_FAILURE;
	}

	text_input = zwp_text_input_manager_v3_get_text_input(text_input_manager, seat);

	zwp_text_input_v3_add_listener(text_input, &text_input_listener, NULL);

	egl_init(display);

	struct wl_surface *surface = wl_compositor_create_surface(compositor);
	struct xdg_surface *xdg_surface =
		xdg_wm_base_get_xdg_surface(wm_base, surface);
	struct xdg_toplevel *xdg_toplevel = xdg_surface_get_toplevel(xdg_surface);

	xdg_surface_add_listener(xdg_surface, &xdg_surface_listener, NULL);
	xdg_toplevel_add_listener(xdg_toplevel, &xdg_toplevel_listener, NULL);

	wl_surface_commit(surface);

	egl_window = wl_egl_window_create(surface, width, height);
	egl_surface = eglCreatePlatformWindowSurfaceEXT(
		egl_display, egl_config, egl_window, NULL);

	wl_display_roundtrip(display);

	draw();

	while (wl_display_dispatch(display) != -1) {
		// This space intentionally left blank
	}

	return EXIT_SUCCESS;
}
