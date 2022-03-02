/*
 * Copyright (C) 2022 Zoe Knox <zoe@pixin.net>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Large portions of this file were taken from wlroots/tinywl/tinywl.c which
 * was released to the public domain under the CC0 1.0 Universal license.
 * Thank you for the excellent starting point and reference!
 */

#define _POSIX_C_SOURCE 200112L
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "wlr-screencopy-unstable-v1.h"
#include "WindowServer.h"

static void focus_view(struct WSView *view, struct wlr_surface *surface) {
	/* Note: this function only deals with keyboard focus. */
	if (view == NULL) {
		return;
	}
	struct WindowServer *server = view->server;
	struct wlr_seat *seat = server->seat;
	struct wlr_surface *prev_surface = seat->keyboard_state.focused_surface;
	if (prev_surface == surface) {
		/* Don't re-focus an already focused surface. */
		return;
	}
	if (prev_surface) {
		/*
		 * Deactivate the previously focused surface. This lets the client know
		 * it no longer has focus and the client will repaint accordingly, e.g.
		 * stop displaying a caret.
		 */
		struct wlr_xdg_surface *previous = wlr_xdg_surface_from_wlr_surface(
					seat->keyboard_state.focused_surface);
		assert(previous->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL);
		wlr_xdg_toplevel_set_activated(previous->toplevel, false);
	}
	struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(seat);
	/* Move the view to the front */
	wlr_scene_node_raise_to_top(view->scene_node);
	wl_list_remove(&view->link);
	wl_list_insert(&server->views, &view->link);
	/* Activate the new surface */
	wlr_xdg_toplevel_set_activated(view->xdg_toplevel, true);
	/*
	 * Tell the seat to have the keyboard enter this surface. wlroots will keep
	 * track of this and automatically send key events to the appropriate
	 * clients without additional work on your part.
	 */
	wlr_seat_keyboard_notify_enter(seat, view->xdg_toplevel->base->surface,
		keyboard->keycodes, keyboard->num_keycodes, &keyboard->modifiers);
}

static void keyboard_handle_modifiers(
		struct wl_listener *listener, void *data) {
	/* This event is raised when a modifier key, such as shift or alt, is
	 * pressed. We simply communicate this to the client. */
	struct WSKeyboard *keyboard =
		wl_container_of(listener, keyboard, modifiers);
	/*
	 * A seat can only have one keyboard, but this is a limitation of the
	 * Wayland protocol - not wlroots. We assign all connected keyboards to the
	 * same seat. You can swap out the underlying wlr_keyboard like this and
	 * wlr_seat handles this transparently.
	 */
	wlr_seat_set_keyboard(keyboard->server->seat, keyboard->device);
	/* Send modifiers to the client. */
	wlr_seat_keyboard_notify_modifiers(keyboard->server->seat,
		&keyboard->device->keyboard->modifiers);
}

static bool handle_keybinding(struct WindowServer *server, xkb_keysym_t sym) {
	/*
	 * Here we handle compositor keybindings. This is when the compositor is
	 * processing keys, rather than passing them on to the client for its own
	 * processing.
	 *
	 * This function assumes Alt is held down.
	 */
	switch (sym) {
	case XKB_KEY_Escape:
		wl_display_terminate(server->wl_display);
		break;
	case XKB_KEY_F1:
		/* Cycle to the next view */
		if (wl_list_length(&server->views) < 2) {
			break;
		}
		struct WSView *next_view = wl_container_of(
			server->views.prev, next_view, link);
		focus_view(next_view, next_view->xdg_toplevel->base->surface);
		break;
	default:
		return false;
	}
	return true;
}

static void keyboard_handle_key(
		struct wl_listener *listener, void *data) {
	/* This event is raised when a key is pressed or released. */
	struct WSKeyboard *keyboard =
		wl_container_of(listener, keyboard, key);
	struct WindowServer *server = keyboard->server;
	struct wlr_event_keyboard_key *event = data;
	struct wlr_seat *seat = server->seat;

	/* Translate libinput keycode -> xkbcommon */
	uint32_t keycode = event->keycode + 8;
	/* Get a list of keysyms based on the keymap for this keyboard */
	const xkb_keysym_t *syms;
	int nsyms = xkb_state_key_get_syms(
			keyboard->device->keyboard->xkb_state, keycode, &syms);

	bool handled = false;
	uint32_t modifiers = wlr_keyboard_get_modifiers(keyboard->device->keyboard);
	if ((modifiers & WLR_MODIFIER_ALT) &&
			event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
		/* If alt is held down and this button was _pressed_, we attempt to
		 * process it as a compositor keybinding. */
		for (int i = 0; i < nsyms; i++) {
			handled = handle_keybinding(server, syms[i]);
		}
	}

	if (!handled) {
		/* Otherwise, we pass it along to the client. */
		wlr_seat_set_keyboard(seat, keyboard->device);
		wlr_seat_keyboard_notify_key(seat, event->time_msec,
			event->keycode, event->state);
	}
}

static void server_new_keyboard(struct WindowServer *server,
		struct wlr_input_device *device) {
	struct WSKeyboard *keyboard =
		calloc(1, sizeof(struct WSKeyboard));
	keyboard->server = server;
	keyboard->device = device;

	/* We need to prepare an XKB keymap and assign it to the keyboard. This
	 * assumes the defaults (e.g. layout = "us"). */
	struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	struct xkb_keymap *keymap = xkb_keymap_new_from_names(context, NULL,
		XKB_KEYMAP_COMPILE_NO_FLAGS);

	wlr_keyboard_set_keymap(device->keyboard, keymap);
	xkb_keymap_unref(keymap);
	xkb_context_unref(context);
	wlr_keyboard_set_repeat_info(device->keyboard, 25, 600);

	/* Here we set up listeners for keyboard events. */
	keyboard->modifiers.notify = keyboard_handle_modifiers;
	wl_signal_add(&device->keyboard->events.modifiers, &keyboard->modifiers);
	keyboard->key.notify = keyboard_handle_key;
	wl_signal_add(&device->keyboard->events.key, &keyboard->key);

	wlr_seat_set_keyboard(server->seat, device);

	/* And add the keyboard to our list of keyboards */
	wl_list_insert(&server->keyboards, &keyboard->link);
}

static void server_new_pointer(struct WindowServer *server,
		struct wlr_input_device *device) {
	/* We don't do anything special with pointers. All of our pointer handling
	 * is proxied through wlr_cursor. On another compositor, you might take this
	 * opportunity to do libinput configuration on the device to set
	 * acceleration, etc. */
	wlr_cursor_attach_input_device(server->cursor, device);
}

void server_new_input(struct wl_listener *listener, void *data) {
	/* This event is raised by the backend when a new input device becomes
	 * available. */
	struct WindowServer *server =
		wl_container_of(listener, server, new_input);
	struct wlr_input_device *device = data;
	switch (device->type) {
	case WLR_INPUT_DEVICE_KEYBOARD:
		server_new_keyboard(server, device);
		break;
	case WLR_INPUT_DEVICE_POINTER:
		server_new_pointer(server, device);
		break;
	default:
		break;
	}
	/* We need to let the wlr_seat know what our capabilities are, which is
	 * communiciated to the client. In TinyWL we always have a cursor, even if
	 * there are no pointer devices, so we always include that capability. */
	uint32_t caps = WL_SEAT_CAPABILITY_POINTER;
	if (!wl_list_empty(&server->keyboards)) {
		caps |= WL_SEAT_CAPABILITY_KEYBOARD;
	}
	wlr_seat_set_capabilities(server->seat, caps);
}

void seat_request_cursor(struct wl_listener *listener, void *data) {
	struct WindowServer *server = wl_container_of(
			listener, server, request_cursor);
	/* This event is raised by the seat when a client provides a cursor image */
	struct wlr_seat_pointer_request_set_cursor_event *event = data;
	struct wlr_seat_client *focused_client =
		server->seat->pointer_state.focused_client;
	/* This can be sent by any client, so we check to make sure this one is
	 * actually has pointer focus first. */
	if (focused_client == event->seat_client) {
		/* Once we've vetted the client, we can tell the cursor to use the
		 * provided surface as the cursor image. It will set the hardware cursor
		 * on the output that it's currently on and continue to do so as the
		 * cursor moves between outputs. */
		wlr_cursor_set_surface(server->cursor, event->surface,
				event->hotspot_x, event->hotspot_y);
	}
}

void seat_request_set_selection(struct wl_listener *listener, void *data) {
	/* This event is raised by the seat when a client wants to set the selection,
	 * usually when the user copies something. wlroots allows compositors to
	 * ignore such requests if they so choose, but in tinywl we always honor
	 */
	struct WindowServer *server = wl_container_of(
			listener, server, request_set_selection);
	struct wlr_seat_request_set_selection_event *event = data;
	wlr_seat_set_selection(server->seat, event->source, event->serial);
}

static struct WSView *desktop_view_at(
		struct WindowServer *server, double lx, double ly,
		struct wlr_surface **surface, double *sx, double *sy) {
	/* This returns the topmost node in the scene at the given layout coords.
	 * we only care about surface nodes as we are specifically looking for a
	 * surface in the surface tree of a WSView. */
	struct wlr_scene_node *node = wlr_scene_node_at(
		&server->scene->node, lx, ly, sx, sy);
	if (node == NULL || node->type != WLR_SCENE_NODE_SURFACE) {
		return NULL;
	}
	*surface = wlr_scene_surface_from_node(node)->surface;
	/* Find the node corresponding to the WSView at the root of this
	 * surface tree, it is the only one for which we set the data field. */
	while (node != NULL && node->data == NULL) {
		node = node->parent;
	}
	return node->data;
}

static void process_cursor_move(struct WindowServer *server, uint32_t time) {
	/* Move the grabbed view to the new position. */
	struct WSView *view = server->grabbed_view;
	view->x = server->cursor->x - server->grab_x;
	view->y = server->cursor->y - server->grab_y;
        WSLog("process_cursor_move: x=%d y=%d view=%p node=%p\n",view->x,view->y,view,view->scene_node);
	wlr_scene_node_set_position(view->scene_node, view->x, view->y);
}

static void process_cursor_resize(struct WindowServer *server, uint32_t time) {
	/*
	 * Resizing the grabbed view can be a little bit complicated, because we
	 * could be resizing from any corner or edge. This not only resizes the view
	 * on one or two axes, but can also move the view if you resize from the top
	 * or left edges (or top-left corner).
	 *
	 * Note that I took some shortcuts here. In a more fleshed-out compositor,
	 * you'd wait for the client to prepare a buffer at the new size, then
	 * commit any movement that was prepared.
	 */
	struct WSView *view = server->grabbed_view;
	double border_x = server->cursor->x - server->grab_x;
	double border_y = server->cursor->y - server->grab_y;
	int new_left = server->grab_geobox.x;
	int new_right = server->grab_geobox.x + server->grab_geobox.width;
	int new_top = server->grab_geobox.y;
	int new_bottom = server->grab_geobox.y + server->grab_geobox.height;

	if (server->resize_edges & WLR_EDGE_TOP) {
		new_top = border_y;
		if (new_top >= new_bottom) {
			new_top = new_bottom - 1;
		}
	} else if (server->resize_edges & WLR_EDGE_BOTTOM) {
		new_bottom = border_y;
		if (new_bottom <= new_top) {
			new_bottom = new_top + 1;
		}
	}
	if (server->resize_edges & WLR_EDGE_LEFT) {
		new_left = border_x;
		if (new_left >= new_right) {
			new_left = new_right - 1;
		}
	} else if (server->resize_edges & WLR_EDGE_RIGHT) {
		new_right = border_x;
		if (new_right <= new_left) {
			new_right = new_left + 1;
		}
	}

	struct wlr_box geo_box;
	wlr_xdg_surface_get_geometry(view->xdg_toplevel->base, &geo_box);
	view->x = new_left - geo_box.x;
	view->y = new_top - geo_box.y;
	wlr_scene_node_set_position(view->scene_node, view->x, view->y);

	int new_width = new_right - new_left;
	int new_height = new_bottom - new_top;
	wlr_xdg_toplevel_set_size(view->xdg_toplevel, new_width, new_height);
}

static void process_cursor_motion(struct WindowServer *server, uint32_t time) {
	/* If the mode is non-passthrough, delegate to those functions. */
	if (server->cursor_mode == WINDOWSERVER_CURSOR_MOVE) {
		process_cursor_move(server, time);
		return;
	} else if (server->cursor_mode == WINDOWSERVER_CURSOR_RESIZE) {
		process_cursor_resize(server, time);
		return;
	}

	/* Otherwise, find the view under the pointer and send the event along. */
	double sx, sy;
	struct wlr_seat *seat = server->seat;
	struct wlr_surface *surface = NULL;
	struct WSView *view = desktop_view_at(server,
			server->cursor->x, server->cursor->y, &surface, &sx, &sy);
	if (!view) {
		/* If there's no view under the cursor, set the cursor image to a
		 * default. This is what makes the cursor image appear when you move it
		 * around the screen, not over any views. */
		wlr_xcursor_manager_set_cursor_image(
				server->cursor_mgr, "left_ptr", server->cursor);
	}
	if (surface) {
		/*
		 * Send pointer enter and motion events.
		 *
		 * The enter event gives the surface "pointer focus", which is distinct
		 * from keyboard focus. You get pointer focus by moving the pointer over
		 * a window.
		 *
		 * Note that wlroots will avoid sending duplicate enter/motion events if
		 * the surface has already has pointer focus or if the client is already
		 * aware of the coordinates passed.
		 */
		wlr_seat_pointer_notify_enter(seat, surface, sx, sy);
		wlr_seat_pointer_notify_motion(seat, time, sx, sy);
	} else {
		/* Clear pointer focus so future button events and such are not sent to
		 * the last client to have the cursor over it. */
		wlr_seat_pointer_clear_focus(seat);
	}
}

void server_cursor_motion(struct wl_listener *listener, void *data) {
	/* This event is forwarded by the cursor when a pointer emits a _relative_
	 * pointer motion event (i.e. a delta) */
	struct WindowServer *server =
		wl_container_of(listener, server, cursor_motion);
	struct wlr_event_pointer_motion *event = data;
	/* The cursor doesn't move unless we tell it to. The cursor automatically
	 * handles constraining the motion to the output layout, as well as any
	 * special configuration applied for the specific input device which
	 * generated the event. You can pass NULL for the device if you want to move
	 * the cursor around without any input. */
	wlr_cursor_move(server->cursor, event->device,
			event->delta_x, event->delta_y);
	process_cursor_motion(server, event->time_msec);
}

void server_cursor_motion_absolute(
		struct wl_listener *listener, void *data) {
	/* This event is forwarded by the cursor when a pointer emits an _absolute_
	 * motion event, from 0..1 on each axis. This happens, for example, when
	 * wlroots is running under a Wayland window rather than KMS+DRM, and you
	 * move the mouse over the window. You could enter the window from any edge,
	 * so we have to warp the mouse there. There is also some hardware which
	 * emits these events. */
	struct WindowServer *server =
		wl_container_of(listener, server, cursor_motion_absolute);
	struct wlr_event_pointer_motion_absolute *event = data;
	wlr_cursor_warp_absolute(server->cursor, event->device, event->x, event->y);
	process_cursor_motion(server, event->time_msec);
}

void server_cursor_button(struct wl_listener *listener, void *data) {
	/* This event is forwarded by the cursor when a pointer emits a button
	 * event. */
	struct WindowServer *server =
		wl_container_of(listener, server, cursor_button);
	struct wlr_event_pointer_button *event = data;
	/* Notify the client with pointer focus that a button press has occurred */
	wlr_seat_pointer_notify_button(server->seat,
			event->time_msec, event->button, event->state);
	double sx, sy;
	struct wlr_surface *surface = NULL;
	struct WSView *view = desktop_view_at(server,
			server->cursor->x, server->cursor->y, &surface, &sx, &sy);
	if (event->state == WLR_BUTTON_RELEASED) {
		/* If you released any buttons, we exit interactive move/resize mode. */
		server->cursor_mode = WINDOWSERVER_CURSOR_PASSTHROUGH;
	} else {
		/* Focus that client if the button was _pressed_ */
		focus_view(view, surface);
	}
}

void server_cursor_axis(struct wl_listener *listener, void *data) {
	/* This event is forwarded by the cursor when a pointer emits an axis event,
	 * for example when you move the scroll wheel. */
	struct WindowServer *server =
		wl_container_of(listener, server, cursor_axis);
	struct wlr_event_pointer_axis *event = data;
	/* Notify the client with pointer focus of the axis event. */
	wlr_seat_pointer_notify_axis(server->seat,
			event->time_msec, event->orientation, event->delta,
			event->delta_discrete, event->source);
}

void server_cursor_frame(struct wl_listener *listener, void *data) {
	/* This event is forwarded by the cursor when a pointer emits an frame
	 * event. Frame events are sent after regular pointer events to group
	 * multiple events together. For instance, two axis events may happen at the
	 * same time, in which case a frame event won't be sent in between. */
	struct WindowServer *server =
		wl_container_of(listener, server, cursor_frame);
	/* Notify the client with pointer focus of the frame event. */
	wlr_seat_pointer_notify_frame(server->seat);
}

static void output_frame(struct wl_listener *listener, void *data) {
	/* This function is called every time an output is ready to display a frame,
	 * generally at the output's refresh rate (e.g. 60Hz). */
	struct WSOutput *output = wl_container_of(listener, output, frame);
	struct wlr_scene *scene = output->server->scene;

	struct wlr_scene_output *scene_output = wlr_scene_get_scene_output(
		scene, output->wlr_output);

	/* Render the scene if needed and commit the output */
	wlr_scene_output_commit(scene_output);

	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	wlr_scene_output_send_frame_done(scene_output, &now);
}

void server_new_output(struct wl_listener *listener, void *data) {
	/* This event is raised by the backend when a new output (aka a display or
	 * monitor) becomes available. */
	struct WindowServer *server =
		wl_container_of(listener, server, new_output);
	struct wlr_output *wlr_output = data;

	/* Configures the output created by the backend to use our allocator
	 * and our renderer. Must be done once, before commiting the output */
	wlr_output_init_render(wlr_output, server->allocator, server->renderer);

	/* Some backends don't have modes. DRM+KMS does, and we need to set a mode
	 * before we can use the output. The mode is a tuple of (width, height,
	 * refresh rate), and each monitor supports only a specific set of modes. We
	 * just pick the monitor's preferred mode, a more sophisticated compositor
	 * would let the user configure it. */
	if (!wl_list_empty(&wlr_output->modes)) {
		struct wlr_output_mode *mode = wlr_output_preferred_mode(wlr_output);
		wlr_output_set_mode(wlr_output, mode);
		wlr_output_enable(wlr_output, true);
		if (!wlr_output_commit(wlr_output)) {
			return;
		}
	}

	/* Allocates and configures our state for this output */
	struct WSOutput *output =
		calloc(1, sizeof(struct WSOutput));
	output->wlr_output = wlr_output;
	output->server = server;
	/* Sets up a listener for the frame notify event. */
	output->frame.notify = output_frame;
	wl_signal_add(&wlr_output->events.frame, &output->frame);
	wl_list_insert(&server->outputs, &output->link);

	/* Adds this to the output layout. The add_auto function arranges outputs
	 * from left-to-right in the order they appear. A more sophisticated
	 * compositor would let the user configure the arrangement of outputs in the
	 * layout.
	 *
	 * The output layout utility automatically adds a wl_output global to the
	 * display, which Wayland clients can see to find out information about the
	 * output (such as DPI, scale factor, manufacturer, etc).
	 */
	wlr_output_layout_add_auto(server->output_layout, wlr_output);
}

static void xdg_toplevel_map(struct wl_listener *listener, void *data) {
	/* Called when the surface is mapped, or ready to display on-screen. */
	struct WSView *view = wl_container_of(listener, view, map);

	wl_list_insert(&view->server->views, &view->link);

	focus_view(view, view->xdg_toplevel->base->surface);
}

static void xdg_toplevel_unmap(struct wl_listener *listener, void *data) {
	/* Called when the surface is unmapped, and should no longer be shown. */
	struct WSView *view = wl_container_of(listener, view, unmap);

	wl_list_remove(&view->link);
}

static void xdg_toplevel_destroy(struct wl_listener *listener, void *data) {
	/* Called when the surface is destroyed and should never be shown again. */
	struct WSView *view = wl_container_of(listener, view, destroy);

	wl_list_remove(&view->map.link);
	wl_list_remove(&view->unmap.link);
	wl_list_remove(&view->destroy.link);
	wl_list_remove(&view->request_move.link);
	wl_list_remove(&view->request_resize.link);

	free(view);
}

static void begin_interactive(struct WSView *view,
		enum WSCursorMode mode, uint32_t edges) {
	/* This function sets up an interactive move or resize operation, where the
	 * compositor stops propegating pointer events to clients and instead
	 * consumes them itself, to move or resize windows. */
	struct WindowServer *server = view->server;
	struct wlr_surface *focused_surface =
		server->seat->pointer_state.focused_surface;
	if (view->xdg_toplevel->base->surface !=
			wlr_surface_get_root_surface(focused_surface)) {
		/* Deny move/resize requests from unfocused clients. */
		return;
	}
	server->grabbed_view = view;
	server->cursor_mode = mode;

	if (mode == WINDOWSERVER_CURSOR_MOVE) {
		server->grab_x = server->cursor->x - view->x;
		server->grab_y = server->cursor->y - view->y;
	} else {
		struct wlr_box geo_box;
		wlr_xdg_surface_get_geometry(view->xdg_toplevel->base, &geo_box);

		double border_x = (view->x + geo_box.x) +
			((edges & WLR_EDGE_RIGHT) ? geo_box.width : 0);
		double border_y = (view->y + geo_box.y) +
			((edges & WLR_EDGE_BOTTOM) ? geo_box.height : 0);
		server->grab_x = server->cursor->x - border_x;
		server->grab_y = server->cursor->y - border_y;

		server->grab_geobox = geo_box;
		server->grab_geobox.x += view->x;
		server->grab_geobox.y += view->y;

		server->resize_edges = edges;
	}
}

static void xdg_toplevel_request_move(
		struct wl_listener *listener, void *data) {
	/* This event is raised when a client would like to begin an interactive
	 * move, typically because the user clicked on their client-side
	 * decorations. Note that a more sophisticated compositor should check the
	 * provided serial against a list of button press serials sent to this
	 * client, to prevent the client from requesting this whenever they want. */
	struct WSView *view = wl_container_of(listener, view, request_move);
	begin_interactive(view, WINDOWSERVER_CURSOR_MOVE, 0);
}

static void xdg_toplevel_request_resize(
		struct wl_listener *listener, void *data) {
	/* This event is raised when a client would like to begin an interactive
	 * resize, typically because the user clicked on their client-side
	 * decorations. Note that a more sophisticated compositor should check the
	 * provided serial against a list of button press serials sent to this
	 * client, to prevent the client from requesting this whenever they want. */
	struct wlr_xdg_toplevel_resize_event *event = data;
	struct WSView *view = wl_container_of(listener, view, request_resize);
	begin_interactive(view, WINDOWSERVER_CURSOR_RESIZE, event->edges);
}

void server_new_xdg_surface(struct wl_listener *listener, void *data) {
	/* This event is raised when wlr_xdg_shell receives a new xdg surface from a
	 * client, either a toplevel (application window) or popup. */
	struct WindowServer *server =
		wl_container_of(listener, server, new_xdg_surface);
	struct wlr_xdg_surface *xdg_surface = data;

	/* We must add xdg popups to the scene graph so they get rendered. The
	 * wlroots scene graph provides a helper for this, but to use it we must
	 * provide the proper parent scene node of the xdg popup. To enable this,
	 * we always set the user data field of xdg_surfaces to the corresponding
	 * scene node. */
	if (xdg_surface->role == WLR_XDG_SURFACE_ROLE_POPUP) {
		struct wlr_xdg_surface *parent = wlr_xdg_surface_from_wlr_surface(
			xdg_surface->popup->parent);
		struct wlr_scene_node *parent_node = parent->data;
		xdg_surface->data = wlr_scene_xdg_surface_create(
			parent_node, xdg_surface);
		return;
	}
	assert(xdg_surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL);

	/* Allocate a WSView for this surface */
	struct WSView *view =
		calloc(1, sizeof(struct WSView));
	view->server = server;
	view->xdg_toplevel = xdg_surface->toplevel;
	view->scene_node = wlr_scene_xdg_surface_create(
			&view->server->scene->node, view->xdg_toplevel->base);
	view->scene_node->data = view;
	xdg_surface->data = view->scene_node;

	/* Listen to the various events it can emit */
	view->map.notify = xdg_toplevel_map;
	wl_signal_add(&xdg_surface->events.map, &view->map);
	view->unmap.notify = xdg_toplevel_unmap;
	wl_signal_add(&xdg_surface->events.unmap, &view->unmap);
	view->destroy.notify = xdg_toplevel_destroy;
	wl_signal_add(&xdg_surface->events.destroy, &view->destroy);

	/* cotd */
	struct wlr_xdg_toplevel *toplevel = xdg_surface->toplevel;
	view->request_move.notify = xdg_toplevel_request_move;
	wl_signal_add(&toplevel->events.request_move, &view->request_move);
	view->request_resize.notify = xdg_toplevel_request_resize;
	wl_signal_add(&toplevel->events.request_resize, &view->request_resize);
}

