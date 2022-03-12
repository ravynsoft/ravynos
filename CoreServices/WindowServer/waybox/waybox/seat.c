#include <unistd.h>

#include <wlr/types/wlr_primary_selection.h>
#include <wlr/types/wlr_primary_selection_v1.h>

#include "waybox/seat.h"
#include "waybox/xdg_shell.h"

static void deiconify_view(struct wb_view *view) {
	if (view->xdg_toplevel->requested.minimized) {
		view->xdg_toplevel->requested.minimized = false;
		wl_signal_emit(&view->xdg_toplevel->events.request_minimize, NULL);
	}
}

static void cycle_views(struct wb_server *server) {
	/* Cycle to the next view */
	if (wl_list_length(&server->views) < 1) {
		return;
	}

	struct wb_view *current_view = wl_container_of(
		server->views.prev, current_view, link);
	deiconify_view(current_view);
	focus_view(current_view, current_view->xdg_toplevel->base->surface);

	/* Move the current view to the beginning of the list */
	wl_list_remove(&current_view->link);
	wl_list_insert(&server->views, &current_view->link);
}

static void cycle_views_reverse(struct wb_server *server) {
	/* Cycle to the previous view */
	if (wl_list_length(&server->views) < 1) {
		return;
	}

	struct wb_view *current_view = wl_container_of(
		server->views.next, current_view, link);
	struct wb_view *next_view = wl_container_of(
		current_view->link.next, next_view, link);
	deiconify_view(next_view);
	focus_view(next_view, next_view->xdg_toplevel->base->surface);

	/* Move the current view to after the previous view in the list */
	wl_list_remove(&current_view->link);
	wl_list_insert(server->views.prev, &current_view->link);
}

static bool handle_keybinding(struct wb_server *server, xkb_keysym_t sym, uint32_t modifiers) {
	/*
	 * Here we handle compositor keybindings. This is when the compositor is
	 * processing keys, rather than passing them on to the client for its own
	 * processing.
	 *
	 * Returns true if the keybinding is handled, false to send it to the
	 * client.
	 */
	if (!server->config) {
		/* Some default key bindings, when the rc.xml file can't be
		 * parsed. */
		if (modifiers & WLR_MODIFIER_ALT && sym == XKB_KEY_Tab)
			cycle_views(server);
		else if (modifiers & (WLR_MODIFIER_ALT|WLR_MODIFIER_SHIFT) &&
				sym == XKB_KEY_Tab)
			cycle_views_reverse(server);
		else if (sym == XKB_KEY_Escape && modifiers & WLR_MODIFIER_CTRL)
			wl_display_terminate(server->wl_display);
		else
			return false;
		return true;
	}

	struct wb_key_binding *key_binding;
	wl_list_for_each(key_binding, &server->config->key_bindings, link) {
		if (sym == key_binding->sym && modifiers == key_binding->modifiers) {
			if (key_binding->action & ACTION_NEXT_WINDOW)
				cycle_views(server);
			if (key_binding->action & ACTION_PREVIOUS_WINDOW)
				cycle_views_reverse(server);
			if (key_binding->action & ACTION_CLOSE) {
				struct wb_view *current_view = wl_container_of(
						server->views.next, current_view, link);
				if (current_view->scene_node->state.enabled)
#if WLR_CHECK_VERSION(0, 16, 0)
					wlr_xdg_toplevel_send_close(current_view->xdg_toplevel);
#else
					wlr_xdg_toplevel_send_close(current_view->xdg_surface);
#endif
			 }
			if (key_binding->action & ACTION_EXECUTE) {
				if (fork() == 0) {
					execl("/bin/sh", "/bin/sh", "-c", key_binding->cmd, (char *) NULL);
				}
			}
			if (key_binding->action & ACTION_TOGGLE_MAXIMIZE) {
				struct wb_view *view = wl_container_of(server->views.next, view, link);
				if (view->scene_node->state.enabled)
					wl_signal_emit(&view->xdg_toplevel->events.request_maximize, NULL);
			}
			if (key_binding->action & ACTION_ICONIFY) {
				struct wb_view *view = wl_container_of(server->views.next, view, link);
				if (view->scene_node->state.enabled) {
					view->xdg_toplevel->requested.minimized = true;
					wl_signal_emit(&view->xdg_toplevel->events.request_minimize, NULL);
				}
			}
			if (key_binding->action & ACTION_SHADE) {
				struct wb_view *view = wl_container_of(server->views.next, view, link);
				if (view->scene_node->state.enabled) {
					struct wlr_box geo_box;
					wlr_xdg_surface_get_geometry(view->xdg_toplevel->base, &geo_box);
					int decoration_height = MAX(geo_box.y - view->current_position.y, TITLEBAR_HEIGHT);

					view->previous_position = view->current_position;
#if WLR_CHECK_VERSION(0, 16, 0)
					wlr_xdg_toplevel_set_size(view->xdg_toplevel,
							view->current_position.width, decoration_height);
#else
					wlr_xdg_toplevel_set_size(view->xdg_surface,
							view->current_position.width, decoration_height);
#endif
				}
			}
			if (key_binding->action & ACTION_UNSHADE) {
				struct wb_view *view = wl_container_of(server->views.next, view, link);
				if (view->scene_node->state.enabled) {
#if WLR_CHECK_VERSION(0, 16, 0)
					wlr_xdg_toplevel_set_size(view->xdg_toplevel,
							view->previous_position.width, view->previous_position.height);
#else
					wlr_xdg_toplevel_set_size(view->xdg_surface,
							view->previous_position.width, view->previous_position.height);
#endif
				}
			}
			if (key_binding->action & ACTION_RECONFIGURE) {
				deinit_config(server->config);
				init_config(server);
			}
			if (key_binding->action & ACTION_EXIT)
				wl_display_terminate(server->wl_display);
			return true;
		}
	}
	return false;
}

static void keyboard_handle_destroy(struct wl_listener *listener, void *data) {
	/* This event is raised by the keyboard base wlr_input_device to signal
	 * the destruction of the wlr_keyboard. It will no longer receive events
	 * and should be destroyed.
	 */
	struct wb_keyboard *keyboard = wl_container_of(listener, keyboard, destroy);
	wl_list_remove(&keyboard->destroy.link);
	wl_list_remove(&keyboard->key.link);
	wl_list_remove(&keyboard->modifiers.link);
	wl_list_remove(&keyboard->link);
	free(keyboard);
}

static void keyboard_handle_modifiers(
		struct wl_listener *listener, void *data) {
	/* This event is raised when a modifier key, such as shift or alt, is
	 * pressed. We simply communicate this to the client. */
	struct wb_keyboard *keyboard =
		wl_container_of(listener, keyboard, modifiers);
	/*
	 * A seat can only have one keyboard, but this is a limitation of the
	 * Wayland protocol - not wlroots. We assign all connected keyboards to the
	 * same seat. You can swap out the underlying wlr_keyboard like this and
	 * wlr_seat handles this transparently.
	 */
	wlr_seat_set_keyboard(keyboard->server->seat->seat, keyboard->device);
	/* Send modifiers to the client. */
	wlr_seat_keyboard_notify_modifiers(keyboard->server->seat->seat,
		&keyboard->device->keyboard->modifiers);
}

static void keyboard_handle_key(
		struct wl_listener *listener, void *data) {
	/* This event is raised when a key is pressed or released. */
	struct wb_keyboard *keyboard =
		wl_container_of(listener, keyboard, key);
	struct wb_server *server = keyboard->server;
	struct wlr_event_keyboard_key *event = data;
	struct wlr_seat *seat = server->seat->seat;

	/* Translate libinput keycode -> xkbcommon */
	uint32_t keycode = event->keycode + 8;
	/* Get a list of keysyms based on the keymap for this keyboard */
	const xkb_keysym_t *syms;
	int nsyms = xkb_state_key_get_syms(
			keyboard->device->keyboard->xkb_state, keycode, &syms);

	bool handled = false;
	uint32_t modifiers = wlr_keyboard_get_modifiers(keyboard->device->keyboard);
	if (event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
		for (int i = 0; i < nsyms; i++) {
			handled = handle_keybinding(server, syms[i], modifiers);
		}
	}

	if (!handled) {
		/* Otherwise, we pass it along to the client. */
		wlr_seat_set_keyboard(seat, keyboard->device);
		wlr_seat_keyboard_notify_key(seat, event->time_msec,
			event->keycode, event->state);
	}
}

static void handle_new_keyboard(struct wb_server *server,
		struct wlr_input_device *device) {
	struct wb_keyboard *keyboard =
		calloc(1, sizeof(struct wb_keyboard));
	keyboard->server = server;
	keyboard->device = device;

	/* We need to prepare an XKB keymap and assign it to the keyboard. */
	struct xkb_rule_names *rules = malloc(sizeof(struct xkb_rule_names));
	if (server->config && server->config->keyboard_layout.use_config) {
		if (server->config->keyboard_layout.layout)
			rules->layout = server->config->keyboard_layout.layout;
		if (server->config->keyboard_layout.model)
			rules->model = server->config->keyboard_layout.model;
		if (server->config->keyboard_layout.options)
			rules->options = server->config->keyboard_layout.options;
		if (server->config->keyboard_layout.rules)
			rules->rules = server->config->keyboard_layout.rules;
		if (server->config->keyboard_layout.variant)
			rules->variant = server->config->keyboard_layout.variant;
	}
	else
		/* If a NULL xkb_rule_names pointer is passed to
		   xkb_keymap_new_from_names, libxkbcommon will default to reading
		   the XKB_* env variables. So there's no need to do it ourselves. */
		rules = NULL;

	struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	struct xkb_keymap *keymap = xkb_keymap_new_from_names(context, rules,
		XKB_KEYMAP_COMPILE_NO_FLAGS);

	if (keymap != NULL) {
		wlr_keyboard_set_keymap(device->keyboard, keymap);
		wlr_keyboard_set_repeat_info(device->keyboard, 25, 600);
	}
	free(rules);
	xkb_keymap_unref(keymap);
	xkb_context_unref(context);

	/* Here we set up listeners for keyboard events. */
	keyboard->destroy.notify = keyboard_handle_destroy;
	wl_signal_add(&device->events.destroy, &keyboard->destroy);
	keyboard->modifiers.notify = keyboard_handle_modifiers;
	wl_signal_add(&device->keyboard->events.modifiers, &keyboard->modifiers);
	keyboard->key.notify = keyboard_handle_key;
	wl_signal_add(&device->keyboard->events.key, &keyboard->key);

	wlr_seat_set_keyboard(server->seat->seat, device);

	/* And add the keyboard to our list of keyboards */
	wl_list_insert(&server->seat->keyboards, &keyboard->link);
}

static void new_input_notify(struct wl_listener *listener, void *data) {
	struct wlr_input_device *device = data;
	struct wb_server *server = wl_container_of(listener, server, new_input);
	switch (device->type) {
		case WLR_INPUT_DEVICE_KEYBOARD:
			wlr_log(WLR_INFO, "%s: %s", _("New keyboard detected"), device->name);
			handle_new_keyboard(server, device);
			break;
		case WLR_INPUT_DEVICE_POINTER:
			wlr_log(WLR_INFO, "%s: %s", _("New pointer detected"), device->name);
			wlr_cursor_attach_input_device(server->cursor->cursor, device);
			break;
		default:
			wlr_log(WLR_INFO, "%s: %s", _("Unsupported input device detected"), device->name);
			break;
	}

	uint32_t caps = WL_SEAT_CAPABILITY_POINTER;
	if (!wl_list_empty(&server->seat->keyboards)) {
		caps |= WL_SEAT_CAPABILITY_KEYBOARD;
	}
	wlr_seat_set_capabilities(server->seat->seat, caps);
}

void seat_focus_surface(struct wb_seat *seat, struct wlr_surface *surface) {
	if (!surface) {
		wlr_seat_keyboard_notify_clear_focus(seat->seat);
		return;
	}

	struct wlr_keyboard *kb = wlr_seat_get_keyboard(seat->seat);
	wlr_seat_keyboard_notify_enter(seat->seat, surface, kb->keycodes,
		kb->num_keycodes, &kb->modifiers);
}

void seat_set_focus_layer(struct wb_seat *seat, struct wlr_layer_surface_v1 *layer) {
	if (!layer) {
		seat->focused_layer = NULL;
		return;
	}
	seat_focus_surface(seat, layer->surface);
	if (layer->current.layer >= ZWLR_LAYER_SHELL_V1_LAYER_TOP) {
		seat->focused_layer = layer;
	}
}

static void handle_request_set_primary_selection(struct wl_listener *listener,
		void *data) {
	struct wb_seat *seat =
		wl_container_of(listener, seat, request_set_primary_selection);
	struct wlr_seat_request_set_primary_selection_event *event = data;
	wlr_seat_set_primary_selection(seat->seat, event->source, event->serial);
}

static void handle_request_set_selection(struct wl_listener *listener, void
		*data) {
	struct wb_seat *seat =
		wl_container_of(listener, seat, request_set_selection);
	struct wlr_seat_request_set_selection_event *event = data;
	wlr_seat_set_selection(seat->seat, event->source, event->serial);
}

struct wb_seat *wb_seat_create(struct wb_server *server) {
	struct wb_seat *seat = malloc(sizeof(struct wb_seat));

	wl_list_init(&seat->keyboards);
	server->new_input.notify = new_input_notify;
	wl_signal_add(&server->backend->events.new_input, &server->new_input);
	seat->seat = wlr_seat_create(server->wl_display, "seat0");

	wlr_primary_selection_v1_device_manager_create(server->wl_display);
	seat->request_set_primary_selection.notify =
		handle_request_set_primary_selection;
	wl_signal_add(&seat->seat->events.request_set_primary_selection,
			&seat->request_set_primary_selection);
	seat->request_set_selection.notify = handle_request_set_selection;
	wl_signal_add(&seat->seat->events.request_set_selection,
			&seat->request_set_selection);

	return seat;
}

void wb_seat_destroy(struct wb_seat *seat) {
	wl_list_remove(&seat->keyboards);
	wl_list_remove(&seat->request_set_primary_selection.link);
	wl_list_remove(&seat->request_set_selection.link);
	wlr_seat_destroy(seat->seat);
	free(seat);
}
