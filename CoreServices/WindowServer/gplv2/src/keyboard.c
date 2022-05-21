// SPDX-License-Identifier: GPL-2.0-only
#include <wlr/backend/multi.h>
#include <wlr/backend/session.h>
#include "action.h"
#include "key-state.h"
#include "labwc.h"

static void
change_vt(struct server *server, unsigned int vt)
{
	if (!wlr_backend_is_multi(server->backend)) {
		return;
	}
	struct wlr_session *session = wlr_backend_get_session(server->backend);
	if (session) {
		wlr_session_change_vt(session, vt);
	}
}

static bool
any_modifiers_pressed(struct wlr_keyboard *keyboard)
{
	xkb_mod_index_t i;
	for (i = 0; i < xkb_keymap_num_mods(keyboard->keymap); i++) {
		if (xkb_state_mod_index_is_active
				(keyboard->xkb_state, i,
				 XKB_STATE_MODS_DEPRESSED)) {
			return true;
		}
	}
	return false;
}


static void
keyboard_modifiers_notify(struct wl_listener *listener, void *data)
{
	struct seat *seat = wl_container_of(listener, seat, keyboard_modifiers);
	struct server *server = seat->server;

	if (server->cycle_view) {
		struct wlr_event_keyboard_key *event = data;
		struct wlr_input_device *device = seat->keyboard_group->input_device;
		damage_all_outputs(server);
		if ((event->state == WL_KEYBOARD_KEY_STATE_RELEASED)
				&& !any_modifiers_pressed(device->keyboard))  {
			/* end cycle */
			desktop_focus_and_activate_view(&server->seat,
				server->cycle_view);
			desktop_move_to_front(server->cycle_view);
			server->cycle_view = NULL;
		}
	}

	wlr_seat_keyboard_notify_modifiers(seat->seat,
		&seat->keyboard_group->keyboard.modifiers);
}

static bool
handle_keybinding(struct server *server, uint32_t modifiers, xkb_keysym_t sym)
{
	struct keybind *keybind;
	struct wlr_keyboard *kb = &server->seat.keyboard_group->keyboard;
	wl_list_for_each_reverse (keybind, &rc.keybinds, link) {
		if (modifiers ^ keybind->modifiers) {
			continue;
		}
		for (size_t i = 0; i < keybind->keysyms_len; i++) {
			if (xkb_keysym_to_lower(sym) == keybind->keysyms[i]) {
				wlr_keyboard_set_repeat_info(kb, 0, 0);
				action(NULL, server, &keybind->actions, 0);
				return true;
			}
		}
	}
	return false;
}

static bool is_modifier_key(xkb_keysym_t sym)
{
	return sym == XKB_KEY_Shift_L ||
		   sym == XKB_KEY_Shift_R ||
		   sym == XKB_KEY_Alt_L ||
		   sym == XKB_KEY_Alt_R ||
		   sym == XKB_KEY_Control_L ||
		   sym == XKB_KEY_Control_R ||
		   sym == XKB_KEY_Super_L ||
		   sym == XKB_KEY_Super_R;
}

static bool
handle_compositor_keybindings(struct wl_listener *listener,
		struct wlr_event_keyboard_key *event)
{
	struct seat *seat = wl_container_of(listener, seat, keyboard_key);
	struct server *server = seat->server;
	struct wlr_input_device *device = seat->keyboard_group->input_device;

	/* Translate libinput keycode -> xkbcommon */
	uint32_t keycode = event->keycode + 8;
	/* Get a list of keysyms based on the keymap for this keyboard */
	const xkb_keysym_t *syms;
	int nsyms = xkb_state_key_get_syms(device->keyboard->xkb_state, keycode, &syms);

	bool handled = false;

	key_state_set_pressed(keycode,
		event->state == WL_KEYBOARD_KEY_STATE_PRESSED);

	/*
	 * If a press event was handled by a compositor binding, then do not
	 * forward the corresponding release event to clients
	 */
	if (key_state_corresponding_press_event_was_bound(keycode)
			&& event->state == WL_KEYBOARD_KEY_STATE_RELEASED) {
		int nr_bound_keys = key_state_bound_key_remove(keycode);
		if (!nr_bound_keys) {
			wlr_keyboard_set_repeat_info(device->keyboard,
				rc.repeat_rate, rc.repeat_delay);
		}
		return true;
	}

	uint32_t modifiers =
		wlr_keyboard_get_modifiers(device->keyboard);

	/* Catch C-A-F1 to C-A-F12 to change tty */
	if (event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
		for (int i = 0; i < nsyms; i++) {
			unsigned int vt = syms[i] - XKB_KEY_XF86Switch_VT_1 + 1;
			if (vt >= 1 && vt <= 12) {
				change_vt(server, vt);
				/* don't send any key events to clients when changing tty */
				return true;
			}
		}
	}

	if (server->cycle_view) {
		damage_all_outputs(server);
		if (event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
			for (int i = 0; i < nsyms; i++) {
				if (syms[i] == XKB_KEY_Escape) {
					/* cancel */
					server->cycle_view = NULL;
					return true;
				}
			}

			/* cycle to next */
			bool backwards = modifiers & WLR_MODIFIER_SHIFT;
			/* ignore if this is a modifier key being pressed */
			bool ignore = false;
			for (int i = 0; i < nsyms; i++) {
				ignore |= is_modifier_key(syms[i]);
			}

			if (!ignore) {
				server->cycle_view =
					desktop_cycle_view(server, server->cycle_view,
						backwards ? LAB_CYCLE_DIR_BACKWARD : LAB_CYCLE_DIR_FORWARD);
				osd_update(server);
			}
		}
		/* don't send any key events to clients when osd onscreen */
		return true;
	}

	/* Handle compositor key bindings */
	if (event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
		for (int i = 0; i < nsyms; i++) {
			handled |= handle_keybinding(server, modifiers, syms[i]);
		}
	}

	if (handled) {
		key_state_store_pressed_keys_as_bound();
	}

	return handled;
}

static void
keyboard_key_notify(struct wl_listener *listener, void *data)
{
	/* This event is raised when a key is pressed or released. */
	struct seat *seat = wl_container_of(listener, seat, keyboard_key);
	struct server *server = seat->server;
	struct wlr_event_keyboard_key *event = data;
	struct wlr_seat *wlr_seat = server->seat.seat;
	struct wlr_input_device *device = seat->keyboard_group->input_device;
	wlr_idle_notify_activity(seat->wlr_idle, seat->seat);

	bool handled = false;

	/* ignore labwc keybindings if input is inhibited */
	if (!seat->active_client_while_inhibited) {
		handled = handle_compositor_keybindings(listener, event);
	}

	if (!handled) {
		wlr_seat_set_keyboard(wlr_seat, device);
		wlr_seat_keyboard_notify_key(wlr_seat, event->time_msec,
					     event->keycode, event->state);
	}
}

void
keyboard_init(struct seat *seat)
{
	seat->keyboard_group = wlr_keyboard_group_create();
	struct wlr_keyboard *kb = &seat->keyboard_group->keyboard;
	struct xkb_rule_names rules = { 0 };
	struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	struct xkb_keymap *keymap = xkb_map_new_from_names(context, &rules,
		XKB_KEYMAP_COMPILE_NO_FLAGS);
	wlr_keyboard_set_keymap(kb, keymap);
	xkb_keymap_unref(keymap);
	xkb_context_unref(context);
	wlr_keyboard_set_repeat_info(kb, rc.repeat_rate, rc.repeat_delay);

	seat->keyboard_key.notify = keyboard_key_notify;
	wl_signal_add(&kb->events.key, &seat->keyboard_key);
	seat->keyboard_modifiers.notify = keyboard_modifiers_notify;
	wl_signal_add(&kb->events.modifiers, &seat->keyboard_modifiers);
}

void
keyboard_finish(struct seat *seat)
{
	if (seat->keyboard_group) {
		wlr_keyboard_group_destroy(seat->keyboard_group);
	}
	wl_list_remove(&seat->keyboard_key.link);
	wl_list_remove(&seat->keyboard_modifiers.link);
}
