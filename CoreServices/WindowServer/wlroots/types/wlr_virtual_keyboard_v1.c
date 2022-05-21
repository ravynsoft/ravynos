#define _POSIX_C_SOURCE 199309L
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_virtual_keyboard_v1.h>
#include <wlr/util/log.h>
#include <xkbcommon/xkbcommon.h>
#include "util/signal.h"
#include "util/time.h"
#include "virtual-keyboard-unstable-v1-protocol.h"


static void keyboard_led_update(struct wlr_keyboard *wlr_kb, uint32_t leds) {
	// unsupported by virtual keyboard protocol
}

static void keyboard_destroy(struct wlr_keyboard *wlr_kb) {
	// safe to ignore - keyboard will be destroyed only iff associated virtual
	// keyboard is torn down, no need to tear down the keyboard separately
}

static const struct wlr_keyboard_impl keyboard_impl = {
	.destroy = keyboard_destroy,
	.led_update = keyboard_led_update
};

static void input_device_destroy(struct wlr_input_device *dev) {
}

static const struct wlr_input_device_impl input_device_impl = {
	.destroy = input_device_destroy
};

static const struct zwp_virtual_keyboard_v1_interface virtual_keyboard_impl;

static struct wlr_virtual_keyboard_v1 *virtual_keyboard_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource,
	   &zwp_virtual_keyboard_v1_interface, &virtual_keyboard_impl));
	return wl_resource_get_user_data(resource);
}

struct wlr_virtual_keyboard_v1 *wlr_input_device_get_virtual_keyboard(
		struct wlr_input_device *wlr_dev) {
	if (wlr_dev->impl != &input_device_impl) {
		return NULL;
	}
	return (struct wlr_virtual_keyboard_v1 *)wlr_dev;
}

static void virtual_keyboard_keymap(struct wl_client *client,
		struct wl_resource *resource, uint32_t format, int32_t fd,
		uint32_t size) {
	struct wlr_virtual_keyboard_v1 *keyboard =
		virtual_keyboard_from_resource(resource);

	struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	if (!context) {
		goto context_fail;
	}
	void *data = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (data == MAP_FAILED) {
		goto fd_fail;
	}
	struct xkb_keymap *keymap = xkb_keymap_new_from_string(context, data,
		XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
	munmap(data, size);
	if (!keymap) {
		goto keymap_fail;
	}
	wlr_keyboard_set_keymap(keyboard->input_device.keyboard, keymap);
	keyboard->has_keymap = true;
	xkb_keymap_unref(keymap);
	xkb_context_unref(context);
	close(fd);
	return;
keymap_fail:
fd_fail:
	xkb_context_unref(context);
context_fail:
	wl_client_post_no_memory(client);
	close(fd);
}

static void virtual_keyboard_key(struct wl_client *client,
		struct wl_resource *resource, uint32_t time, uint32_t key,
		uint32_t state) {
	struct wlr_virtual_keyboard_v1 *keyboard =
		virtual_keyboard_from_resource(resource);
	if (!keyboard->has_keymap) {
		wl_resource_post_error(resource,
			ZWP_VIRTUAL_KEYBOARD_V1_ERROR_NO_KEYMAP,
			"Cannot send a keypress before defining a keymap");
		return;
	}
	struct wlr_event_keyboard_key event = {
		.time_msec = time,
		.keycode = key,
		.update_state = false,
		.state = state,
	};
	wlr_keyboard_notify_key(keyboard->input_device.keyboard, &event);
}

static void virtual_keyboard_modifiers(struct wl_client *client,
		struct wl_resource *resource, uint32_t mods_depressed,
		uint32_t mods_latched, uint32_t mods_locked, uint32_t group) {
	struct wlr_virtual_keyboard_v1 *keyboard =
		virtual_keyboard_from_resource(resource);
	if (!keyboard->has_keymap) {
		wl_resource_post_error(resource,
			ZWP_VIRTUAL_KEYBOARD_V1_ERROR_NO_KEYMAP,
			"Cannot send a modifier state before defining a keymap");
		return;
	}
	wlr_keyboard_notify_modifiers(keyboard->input_device.keyboard,
		mods_depressed, mods_latched, mods_locked, group);
}

/**
 * Send release event for each pressed key to bring the keyboard back to
 * neutral state.
 *
 * This may be needed for virtual keyboards. For physical devices, kernel
 * or libinput will deal with the removal of devices.
 */
static void keyboard_release_pressed_keys(struct wlr_keyboard *keyboard) {
	size_t orig_num_keycodes = keyboard->num_keycodes;
	for (size_t i = 0; i < orig_num_keycodes; ++i) {
		assert(keyboard->num_keycodes == orig_num_keycodes - i);
		struct wlr_event_keyboard_key event = {
			.time_msec = get_current_time_msec(),
			.keycode = keyboard->keycodes[orig_num_keycodes - i - 1],
			.update_state = false,
			.state = WL_KEYBOARD_KEY_STATE_RELEASED,
		};
		wlr_keyboard_notify_key(keyboard, &event);  // updates num_keycodes
	}
}

static void virtual_keyboard_destroy_resource(struct wl_resource *resource) {
	struct wlr_virtual_keyboard_v1 *keyboard =
		virtual_keyboard_from_resource(resource);
	keyboard_release_pressed_keys(keyboard->input_device.keyboard);
	wlr_signal_emit_safe(&keyboard->events.destroy, keyboard);
	wl_list_remove(&keyboard->link);
	wlr_input_device_destroy(&keyboard->input_device);
	free(keyboard);
}

static void virtual_keyboard_destroy(struct wl_client *client,
		struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static const struct zwp_virtual_keyboard_v1_interface virtual_keyboard_impl = {
	.keymap = virtual_keyboard_keymap,
	.key = virtual_keyboard_key,
	.modifiers = virtual_keyboard_modifiers,
	.destroy = virtual_keyboard_destroy,
};

static const struct zwp_virtual_keyboard_manager_v1_interface manager_impl;

static struct wlr_virtual_keyboard_manager_v1 *manager_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource,
		&zwp_virtual_keyboard_manager_v1_interface, &manager_impl));
	return wl_resource_get_user_data(resource);
}

static void virtual_keyboard_manager_create_virtual_keyboard(
		struct wl_client *client, struct wl_resource *resource,
		struct wl_resource *seat, uint32_t id) {
	struct wlr_virtual_keyboard_manager_v1 *manager =
		manager_from_resource(resource);

	struct wlr_virtual_keyboard_v1 *virtual_keyboard = calloc(1,
		sizeof(struct wlr_virtual_keyboard_v1));
	if (!virtual_keyboard) {
		wl_client_post_no_memory(client);
		return;
	}

	struct wlr_keyboard* keyboard = calloc(1, sizeof(struct wlr_keyboard));
	if (!keyboard) {
		wlr_log(WLR_ERROR, "Cannot allocate wlr_keyboard");
		free(virtual_keyboard);
		wl_client_post_no_memory(client);
		return;
	}
	wlr_keyboard_init(keyboard, &keyboard_impl);

	struct wl_resource *keyboard_resource = wl_resource_create(client,
		&zwp_virtual_keyboard_v1_interface, wl_resource_get_version(resource),
		id);
	if (!keyboard_resource) {
		free(keyboard);
		free(virtual_keyboard);
		wl_client_post_no_memory(client);
		return;
	}

	wl_resource_set_implementation(keyboard_resource, &virtual_keyboard_impl,
		virtual_keyboard, virtual_keyboard_destroy_resource);

	wlr_input_device_init(&virtual_keyboard->input_device,
		WLR_INPUT_DEVICE_KEYBOARD, &input_device_impl, "virtual keyboard",
		0x0, 0x0);

	struct wlr_seat_client *seat_client = wlr_seat_client_from_resource(seat);

	virtual_keyboard->input_device.keyboard = keyboard;
	virtual_keyboard->resource = keyboard_resource;
	virtual_keyboard->seat = seat_client->seat;
	wl_signal_init(&virtual_keyboard->events.destroy);

	wl_list_insert(&manager->virtual_keyboards, &virtual_keyboard->link);

	wlr_signal_emit_safe(&manager->events.new_virtual_keyboard,
		virtual_keyboard);
}

static const struct zwp_virtual_keyboard_manager_v1_interface manager_impl = {
	.create_virtual_keyboard = virtual_keyboard_manager_create_virtual_keyboard,
};

static void virtual_keyboard_manager_bind(struct wl_client *client, void *data,
		uint32_t version, uint32_t id) {
	struct wlr_virtual_keyboard_manager_v1 *manager = data;

	struct wl_resource *resource = wl_resource_create(client,
		&zwp_virtual_keyboard_manager_v1_interface, version, id);
	if (!resource) {
		wl_client_post_no_memory(client);
		return;
	}

	wl_resource_set_implementation(resource, &manager_impl, manager, NULL);
}

static void handle_display_destroy(struct wl_listener *listener, void *data) {
	struct wlr_virtual_keyboard_manager_v1 *manager =
		wl_container_of(listener, manager, display_destroy);
	wlr_signal_emit_safe(&manager->events.destroy, manager);
	wl_list_remove(&manager->display_destroy.link);
	wl_global_destroy(manager->global);
	free(manager);
}

struct wlr_virtual_keyboard_manager_v1*
		wlr_virtual_keyboard_manager_v1_create(
		struct wl_display *display) {
	struct wlr_virtual_keyboard_manager_v1 *manager = calloc(1,
		sizeof(struct wlr_virtual_keyboard_manager_v1));
	if (!manager) {
		return NULL;
	}

	manager->global = wl_global_create(display,
		&zwp_virtual_keyboard_manager_v1_interface, 1, manager,
		virtual_keyboard_manager_bind);
	if (!manager->global) {
		free(manager);
		return NULL;
	}

	manager->display_destroy.notify = handle_display_destroy;
	wl_display_add_destroy_listener(display, &manager->display_destroy);

	wl_list_init(&manager->virtual_keyboards);

	wl_signal_init(&manager->events.new_virtual_keyboard);
	wl_signal_init(&manager->events.destroy);

	return manager;
}
