#ifndef _WB_SEAT_H
#define _WB_SEAT_H

#include <wlr/types/wlr_seat.h>

struct wb_seat {
	struct wlr_seat *seat;

	struct wlr_layer_surface_v1 *focused_layer;

	struct wl_list keyboards;

	struct wl_listener request_set_primary_selection;
	struct wl_listener request_set_selection;
};

struct wb_keyboard {
	struct wl_list link;
	struct wb_server *server;
	struct wlr_input_device *device;

	struct wl_listener destroy;
	struct wl_listener modifiers;
	struct wl_listener key;
};

struct wb_server;
struct wb_seat *wb_seat_create(struct wb_server *server);
void seat_focus_surface(struct wb_seat *seat, struct wlr_surface *surface);
void seat_set_focus_layer(struct wb_seat *seat, struct wlr_layer_surface_v1 *layer);
void wb_seat_destroy(struct wb_seat *seat);
#endif
