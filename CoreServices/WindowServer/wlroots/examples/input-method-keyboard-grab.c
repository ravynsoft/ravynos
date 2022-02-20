#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>
#include "input-method-unstable-v2-client-protocol.h"

static struct wl_display *display = NULL;
static struct wl_seat *seat = NULL;
static struct zwp_input_method_manager_v2 *input_method_manager = NULL;
static struct zwp_input_method_v2 *input_method = NULL;
static struct zwp_input_method_keyboard_grab_v2 *kb_grab = NULL;

static bool active = false;
static bool pending_active = false;

static struct xkb_context *xkb_context = NULL;
static struct xkb_keymap *keymap = NULL;
static struct xkb_state *xkb_state = NULL;

static void handle_key(void *data,
		struct zwp_input_method_keyboard_grab_v2 *im_keyboard_grab,
		uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
	printf("handle_key %u %u %u %u\n", serial, time, key, state);
	xkb_keysym_t keysym = xkb_state_key_get_one_sym(xkb_state, key + 8);
	char keysym_name[64];
	xkb_keysym_get_name(keysym, keysym_name, sizeof(keysym_name));
	printf("xkb translated to %s\n", keysym_name);
	if (state == WL_KEYBOARD_KEY_STATE_PRESSED) {
		if (keysym == XKB_KEY_KP_Enter || keysym == XKB_KEY_Return) {
			printf("Stopping grab\n");
			zwp_input_method_keyboard_grab_v2_release(kb_grab);
			kb_grab = NULL;
		}
	}

}

static void handle_modifiers(void *data,
		struct zwp_input_method_keyboard_grab_v2 *im_keyboard_grab,
		uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched,
		uint32_t mods_locked, uint32_t group) {
	printf("handle_modifiers %u %u %u %u %u\n", serial, mods_depressed,
		mods_latched, mods_locked, group);
	xkb_state_update_mask(xkb_state, mods_depressed, mods_latched,
		mods_locked, 0, 0, group);
}

static void handle_keymap(void *data,
		struct zwp_input_method_keyboard_grab_v2 *im_keyboard_grab,
		uint32_t format, int32_t fd, uint32_t size) {
	printf("handle_keymap\n");
	char *keymap_string = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
	xkb_keymap_unref(keymap);
	keymap = xkb_keymap_new_from_string(xkb_context, keymap_string,
		XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
	munmap(keymap_string, size);
	close(fd);
	xkb_state_unref(xkb_state);
	xkb_state = xkb_state_new(keymap);
}

static void handle_repeat_info(void *data,
		struct zwp_input_method_keyboard_grab_v2 *im_keyboard_grab,
		int32_t rate, int32_t delay) {
	printf("handle_repeat_info %d %d", rate, delay);
}


static const struct zwp_input_method_keyboard_grab_v2_listener grab_listener = {
	.key = handle_key,
	.modifiers = handle_modifiers,
	.keymap = handle_keymap,
	.repeat_info = handle_repeat_info,
};

static void handle_activate(void *data,
		struct zwp_input_method_v2 *zwp_input_method_v2) {
	pending_active = true;
}

static void handle_deactivate(void *data,
		struct zwp_input_method_v2 *zwp_input_method_v2) {
	pending_active = false;
}

static void handle_unavailable(void *data,
		struct zwp_input_method_v2 *zwp_input_method_v2) {
	printf("IM unavailable\n");
	zwp_input_method_v2_destroy(zwp_input_method_v2);
	input_method = NULL;
}

static void im_activate(void *data,
		struct zwp_input_method_v2 *id) {
	kb_grab = zwp_input_method_v2_grab_keyboard(input_method);
	if (kb_grab == NULL) {
		fprintf(stderr, "Failed to grab\n");
		exit(EXIT_FAILURE);
	}
	zwp_input_method_keyboard_grab_v2_add_listener(kb_grab, &grab_listener,
		NULL);
	printf("Started grab, press enter to stop grab\n");
}

static void im_deactivate(void *data,
		struct zwp_input_method_v2 *context) {
	if (kb_grab != NULL) {
		zwp_input_method_keyboard_grab_v2_release(kb_grab);
		kb_grab = NULL;
	}
}

static void handle_done(void *data,
		struct zwp_input_method_v2 *zwp_input_method_v2) {
	bool prev_active = active;
	if (active != pending_active) {
		printf("Now %s\n", pending_active ? "active" : "inactive");
	}
	active = pending_active;
	if (active && !prev_active) {
		im_activate(data, zwp_input_method_v2);
	} else if (!active && prev_active) {
		im_deactivate(data, zwp_input_method_v2);
	}
}

static void handle_surrounding_text(void *data,
		struct zwp_input_method_v2 *zwp_input_method_v2,
		const char *text, uint32_t cursor, uint32_t anchor) {
	// not for this test
}

static void handle_text_change_cause(void *data,
		struct zwp_input_method_v2 *zwp_input_method_v2,
		uint32_t cause) {
	// not for this test
}

static void handle_content_type(void *data,
		struct zwp_input_method_v2 *zwp_input_method_v2,
		uint32_t hint, uint32_t purpose) {
	// not for this test
}

static const struct zwp_input_method_v2_listener im_listener = {
	.activate = handle_activate,
	.deactivate = handle_deactivate,
	.surrounding_text = handle_surrounding_text,
	.text_change_cause = handle_text_change_cause,
	.content_type = handle_content_type,
	.done = handle_done,
	.unavailable = handle_unavailable,
};

static void handle_global(void *data, struct wl_registry *registry,
		uint32_t name, const char *interface, uint32_t version) {
	if (strcmp(interface, zwp_input_method_manager_v2_interface.name) == 0) {
		input_method_manager = wl_registry_bind(registry, name,
			&zwp_input_method_manager_v2_interface, 1);
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
	xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	if (xkb_context == NULL) {
		fprintf(stderr, "Failed to create xkb context\n");
		return EXIT_FAILURE;
	}

	display = wl_display_connect(NULL);
	if (display == NULL) {
		fprintf(stderr, "Failed to create display\n");
		return EXIT_FAILURE;
	}

	struct wl_registry *registry = wl_display_get_registry(display);
	wl_registry_add_listener(registry, &registry_listener, NULL);
	wl_display_roundtrip(display);

	if (input_method_manager == NULL) {
		fprintf(stderr, "input-method not available\n");
		return EXIT_FAILURE;
	}
	if (seat == NULL) {
		fprintf(stderr, "seat not available\n");
		return EXIT_FAILURE;
	}

	input_method = zwp_input_method_manager_v2_get_input_method(
		input_method_manager, seat);
	zwp_input_method_v2_add_listener(input_method, &im_listener, NULL);

	wl_display_roundtrip(display);

	while (wl_display_dispatch(display) != -1) {
		// This space is intentionally left blank
	};

	return EXIT_SUCCESS;
}
