#ifndef TYPES_WLR_TABLET_V2_H
#define TYPES_WLR_TABLET_V2_H

#include "tablet-unstable-v2-protocol.h"
#include <wayland-server-core.h>
#include <wlr/types/wlr_tablet_v2.h>

struct wlr_tablet_seat_v2 {
	struct wl_list link; // wlr_tablet_manager_v2::seats
	struct wlr_seat *wlr_seat;
	struct wlr_tablet_manager_v2 *manager;

	struct wl_list tablets; // wlr_tablet_v2_tablet::link
	struct wl_list tools;
	struct wl_list pads;

	struct wl_list clients; // wlr_tablet_seat_v2_client::link

	struct wl_listener seat_destroy;
};

struct wlr_tablet_seat_client_v2 {
	struct wl_list seat_link;
	struct wl_list client_link;
	struct wl_client *wl_client;
	struct wl_resource *resource;

	struct wlr_tablet_manager_client_v2 *client;
	struct wlr_seat_client *seat_client;

	struct wl_listener seat_client_destroy;

	struct wl_list tools;   //wlr_tablet_tool_client_v2::link
	struct wl_list tablets; //wlr_tablet_client_v2::link
	struct wl_list pads;    //wlr_tablet_pad_client_v2::link
};

struct wlr_tablet_client_v2 {
	struct wl_list seat_link; // wlr_tablet_seat_client_v2::tablet
	struct wl_list tablet_link; // wlr_tablet_v2_tablet::clients
	struct wl_client *client;
	struct wl_resource *resource;
};

struct wlr_tablet_pad_client_v2 {
	struct wl_list seat_link;
	struct wl_list pad_link;
	struct wl_client *client;
	struct wl_resource *resource;
	struct wlr_tablet_v2_tablet_pad *pad;
	struct wlr_tablet_seat_client_v2 *seat;

	size_t button_count;

	size_t group_count;
	struct wl_resource **groups;

	size_t ring_count;
	struct wl_resource **rings;

	size_t strip_count;
	struct wl_resource **strips;
};

struct wlr_tablet_tool_client_v2 {
	struct wl_list seat_link;
	struct wl_list tool_link;
	struct wl_client *client;
	struct wl_resource *resource;
	struct wlr_tablet_v2_tablet_tool *tool;
	struct wlr_tablet_seat_client_v2 *seat;

	struct wl_event_source *frame_source;
};

struct wlr_tablet_client_v2 *tablet_client_from_resource(struct wl_resource *resource);
void destroy_tablet_v2(struct wl_resource *resource);
void add_tablet_client(struct wlr_tablet_seat_client_v2 *seat, struct wlr_tablet_v2_tablet *tablet);

void destroy_tablet_pad_v2(struct wl_resource *resource);
struct wlr_tablet_pad_client_v2 *tablet_pad_client_from_resource(struct wl_resource *resource);
void add_tablet_pad_client(struct wlr_tablet_seat_client_v2 *seat, struct wlr_tablet_v2_tablet_pad *pad);

void destroy_tablet_tool_v2(struct wl_resource *resource);
struct wlr_tablet_tool_client_v2 *tablet_tool_client_from_resource(struct wl_resource *resource);
void add_tablet_tool_client(struct wlr_tablet_seat_client_v2 *seat, struct wlr_tablet_v2_tablet_tool *tool);

struct wlr_tablet_seat_client_v2 *tablet_seat_client_from_resource(struct wl_resource *resource);
void tablet_seat_client_v2_destroy(struct wl_resource *resource);
struct wlr_tablet_seat_v2 *get_or_create_tablet_seat(
	struct wlr_tablet_manager_v2 *manager,
	struct wlr_seat *wlr_seat);

#endif /* TYPES_WLR_TABLET_V2_H */
