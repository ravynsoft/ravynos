/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_TYPES_WLR_TABLET_V2_H
#define WLR_TYPES_WLR_TABLET_V2_H

#include <wayland-server-core.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_input_device.h>

#include "tablet-unstable-v2-protocol.h"

/* This can probably be even lower,the tools don't have a lot of buttons */
#define WLR_TABLET_V2_TOOL_BUTTONS_CAP 16

struct wlr_tablet_pad_v2_grab_interface;

struct wlr_tablet_pad_v2_grab {
	const struct wlr_tablet_pad_v2_grab_interface *interface;
	struct wlr_tablet_v2_tablet_pad *pad;
	void *data;
};

struct wlr_tablet_tool_v2_grab_interface;

struct wlr_tablet_tool_v2_grab {
	const struct wlr_tablet_tool_v2_grab_interface *interface;
	struct wlr_tablet_v2_tablet_tool *tool;
	void *data;
};

struct wlr_tablet_client_v2;
struct wlr_tablet_tool_client_v2;
struct wlr_tablet_pad_client_v2;

struct wlr_tablet_manager_v2 {
	struct wl_global *wl_global;
	struct wl_list clients; // wlr_tablet_manager_client_v2::link
	struct wl_list seats; // wlr_tablet_seat_v2::link

	struct wl_listener display_destroy;

	struct {
		struct wl_signal destroy;
	} events;

	void *data;
};

struct wlr_tablet_v2_tablet {
	struct wl_list link; // wlr_tablet_seat_v2::tablets
	struct wlr_tablet *wlr_tablet;
	struct wlr_input_device *wlr_device;
	struct wl_list clients; // wlr_tablet_client_v2::tablet_link

	struct wl_listener tool_destroy;

	struct wlr_tablet_client_v2 *current_client;
};

struct wlr_tablet_v2_tablet_tool {
	struct wl_list link; // wlr_tablet_seat_v2::tablets
	struct wlr_tablet_tool *wlr_tool;
	struct wl_list clients; // wlr_tablet_tool_client_v2::tool_link

	struct wl_listener tool_destroy;

	struct wlr_tablet_tool_client_v2 *current_client;
	struct wlr_surface *focused_surface;
	struct wl_listener surface_destroy;

	struct wlr_tablet_tool_v2_grab *grab;
	struct wlr_tablet_tool_v2_grab default_grab;

	uint32_t proximity_serial;
	bool is_down;
	uint32_t down_serial;
	size_t num_buttons;
	uint32_t pressed_buttons[WLR_TABLET_V2_TOOL_BUTTONS_CAP];
	uint32_t pressed_serials[WLR_TABLET_V2_TOOL_BUTTONS_CAP];

	struct {
		struct wl_signal set_cursor; // struct wlr_tablet_v2_event_cursor
	} events;
};

struct wlr_tablet_v2_tablet_pad {
	struct wl_list link; // wlr_tablet_seat_v2::pads
	struct wlr_tablet_pad *wlr_pad;
	struct wlr_input_device *wlr_device;
	struct wl_list clients; // wlr_tablet_pad_client_v2::pad_link

	size_t group_count;
	uint32_t *groups;

	struct wl_listener pad_destroy;

	struct wlr_tablet_pad_client_v2 *current_client;
	struct wlr_tablet_pad_v2_grab *grab;
	struct wlr_tablet_pad_v2_grab default_grab;

	struct {
		struct wl_signal button_feedback; // struct wlr_tablet_v2_event_feedback
		struct wl_signal strip_feedback; // struct wlr_tablet_v2_event_feedback
		struct wl_signal ring_feedback; // struct wlr_tablet_v2_event_feedback
	} events;
};

struct wlr_tablet_v2_event_cursor {
	struct wlr_surface *surface;
	uint32_t serial;
	int32_t hotspot_x;
	int32_t hotspot_y;
	struct wlr_seat_client *seat_client;
};

struct wlr_tablet_v2_event_feedback {
	const char *description;
	size_t index;
	uint32_t serial;
};

struct wlr_tablet_v2_tablet *wlr_tablet_create(
	struct wlr_tablet_manager_v2 *manager,
	struct wlr_seat *wlr_seat,
	struct wlr_input_device *wlr_device);

struct wlr_tablet_v2_tablet_pad *wlr_tablet_pad_create(
	struct wlr_tablet_manager_v2 *manager,
	struct wlr_seat *wlr_seat,
	struct wlr_input_device *wlr_device);

struct wlr_tablet_v2_tablet_tool *wlr_tablet_tool_create(
	struct wlr_tablet_manager_v2 *manager,
	struct wlr_seat *wlr_seat,
	struct wlr_tablet_tool *wlr_tool);

struct wlr_tablet_manager_v2 *wlr_tablet_v2_create(struct wl_display *display);

void wlr_send_tablet_v2_tablet_tool_proximity_in(
	struct wlr_tablet_v2_tablet_tool *tool,
	struct wlr_tablet_v2_tablet *tablet,
	struct wlr_surface *surface);

void wlr_send_tablet_v2_tablet_tool_down(struct wlr_tablet_v2_tablet_tool *tool);
void wlr_send_tablet_v2_tablet_tool_up(struct wlr_tablet_v2_tablet_tool *tool);

void wlr_send_tablet_v2_tablet_tool_motion(
	struct wlr_tablet_v2_tablet_tool *tool, double x, double y);

void wlr_send_tablet_v2_tablet_tool_pressure(
	struct wlr_tablet_v2_tablet_tool *tool, double pressure);

void wlr_send_tablet_v2_tablet_tool_distance(
	struct wlr_tablet_v2_tablet_tool *tool, double distance);

void wlr_send_tablet_v2_tablet_tool_tilt(
	struct wlr_tablet_v2_tablet_tool *tool, double x, double y);

void wlr_send_tablet_v2_tablet_tool_rotation(
	struct wlr_tablet_v2_tablet_tool *tool, double degrees);

void wlr_send_tablet_v2_tablet_tool_slider(
	struct wlr_tablet_v2_tablet_tool *tool, double position);

void wlr_send_tablet_v2_tablet_tool_wheel(
	struct wlr_tablet_v2_tablet_tool *tool, double degrees, int32_t clicks);

void wlr_send_tablet_v2_tablet_tool_proximity_out(
	struct wlr_tablet_v2_tablet_tool *tool);

void wlr_send_tablet_v2_tablet_tool_button(
	struct wlr_tablet_v2_tablet_tool *tool, uint32_t button,
	enum zwp_tablet_pad_v2_button_state state);



void wlr_tablet_v2_tablet_tool_notify_proximity_in(
	struct wlr_tablet_v2_tablet_tool *tool,
	struct wlr_tablet_v2_tablet *tablet,
	struct wlr_surface *surface);

void wlr_tablet_v2_tablet_tool_notify_down(struct wlr_tablet_v2_tablet_tool *tool);
void wlr_tablet_v2_tablet_tool_notify_up(struct wlr_tablet_v2_tablet_tool *tool);

void wlr_tablet_v2_tablet_tool_notify_motion(
	struct wlr_tablet_v2_tablet_tool *tool, double x, double y);

void wlr_tablet_v2_tablet_tool_notify_pressure(
	struct wlr_tablet_v2_tablet_tool *tool, double pressure);

void wlr_tablet_v2_tablet_tool_notify_distance(
	struct wlr_tablet_v2_tablet_tool *tool, double distance);

void wlr_tablet_v2_tablet_tool_notify_tilt(
	struct wlr_tablet_v2_tablet_tool *tool, double x, double y);

void wlr_tablet_v2_tablet_tool_notify_rotation(
	struct wlr_tablet_v2_tablet_tool *tool, double degrees);

void wlr_tablet_v2_tablet_tool_notify_slider(
	struct wlr_tablet_v2_tablet_tool *tool, double position);

void wlr_tablet_v2_tablet_tool_notify_wheel(
	struct wlr_tablet_v2_tablet_tool *tool, double degrees, int32_t clicks);

void wlr_tablet_v2_tablet_tool_notify_proximity_out(
	struct wlr_tablet_v2_tablet_tool *tool);

void wlr_tablet_v2_tablet_tool_notify_button(
	struct wlr_tablet_v2_tablet_tool *tool, uint32_t button,
	enum zwp_tablet_pad_v2_button_state state);


struct wlr_tablet_tool_v2_grab_interface {
	void (*proximity_in)(
		struct wlr_tablet_tool_v2_grab *grab,
		struct wlr_tablet_v2_tablet *tablet,
		struct wlr_surface *surface);

	void (*down)(struct wlr_tablet_tool_v2_grab *grab);
	void (*up)(struct wlr_tablet_tool_v2_grab *grab);

	void (*motion)(struct wlr_tablet_tool_v2_grab *grab, double x, double y);

	void (*pressure)(struct wlr_tablet_tool_v2_grab *grab, double pressure);

	void (*distance)(struct wlr_tablet_tool_v2_grab *grab, double distance);

	void (*tilt)(struct wlr_tablet_tool_v2_grab *grab, double x, double y);

	void (*rotation)(struct wlr_tablet_tool_v2_grab *grab, double degrees);

	void (*slider)(struct wlr_tablet_tool_v2_grab *grab, double position);

	void (*wheel)(struct wlr_tablet_tool_v2_grab *grab, double degrees, int32_t clicks);

	void (*proximity_out)(struct wlr_tablet_tool_v2_grab *grab);

	void (*button)(
		struct wlr_tablet_tool_v2_grab *grab, uint32_t button,
		enum zwp_tablet_pad_v2_button_state state);
	void (*cancel)(struct wlr_tablet_tool_v2_grab *grab);
};

void wlr_tablet_tool_v2_start_grab(struct wlr_tablet_v2_tablet_tool *tool, struct wlr_tablet_tool_v2_grab *grab);
void wlr_tablet_tool_v2_end_grab(struct wlr_tablet_v2_tablet_tool *tool);

void wlr_tablet_tool_v2_start_implicit_grab(struct wlr_tablet_v2_tablet_tool *tool);

bool wlr_tablet_tool_v2_has_implicit_grab(
	struct wlr_tablet_v2_tablet_tool *tool);

uint32_t wlr_send_tablet_v2_tablet_pad_enter(
	struct wlr_tablet_v2_tablet_pad *pad,
	struct wlr_tablet_v2_tablet *tablet,
	struct wlr_surface *surface);

void wlr_send_tablet_v2_tablet_pad_button(
	struct wlr_tablet_v2_tablet_pad *pad, size_t button,
	uint32_t time, enum zwp_tablet_pad_v2_button_state state);

void wlr_send_tablet_v2_tablet_pad_strip(struct wlr_tablet_v2_tablet_pad *pad,
	uint32_t strip, double position, bool finger, uint32_t time);
void wlr_send_tablet_v2_tablet_pad_ring(struct wlr_tablet_v2_tablet_pad *pad,
	uint32_t ring, double position, bool finger, uint32_t time);

uint32_t wlr_send_tablet_v2_tablet_pad_leave(struct wlr_tablet_v2_tablet_pad *pad,
	struct wlr_surface *surface);

uint32_t wlr_send_tablet_v2_tablet_pad_mode(struct wlr_tablet_v2_tablet_pad *pad,
	size_t group, uint32_t mode, uint32_t time);


uint32_t wlr_tablet_v2_tablet_pad_notify_enter(
	struct wlr_tablet_v2_tablet_pad *pad,
	struct wlr_tablet_v2_tablet *tablet,
	struct wlr_surface *surface);

void wlr_tablet_v2_tablet_pad_notify_button(
	struct wlr_tablet_v2_tablet_pad *pad, size_t button,
	uint32_t time, enum zwp_tablet_pad_v2_button_state state);

void wlr_tablet_v2_tablet_pad_notify_strip(
	struct wlr_tablet_v2_tablet_pad *pad,
	uint32_t strip, double position, bool finger, uint32_t time);
void wlr_tablet_v2_tablet_pad_notify_ring(
	struct wlr_tablet_v2_tablet_pad *pad,
	uint32_t ring, double position, bool finger, uint32_t time);

uint32_t wlr_tablet_v2_tablet_pad_notify_leave(
	struct wlr_tablet_v2_tablet_pad *pad, struct wlr_surface *surface);

uint32_t wlr_tablet_v2_tablet_pad_notify_mode(
	struct wlr_tablet_v2_tablet_pad *pad,
	size_t group, uint32_t mode, uint32_t time);

struct wlr_tablet_pad_v2_grab_interface {
	uint32_t (*enter)(
		struct wlr_tablet_pad_v2_grab *grab,
		struct wlr_tablet_v2_tablet *tablet,
		struct wlr_surface *surface);

	void (*button)(struct wlr_tablet_pad_v2_grab *grab,size_t button,
		uint32_t time, enum zwp_tablet_pad_v2_button_state state);

	void (*strip)(struct wlr_tablet_pad_v2_grab *grab,
		uint32_t strip, double position, bool finger, uint32_t time);
	void (*ring)(struct wlr_tablet_pad_v2_grab *grab,
		uint32_t ring, double position, bool finger, uint32_t time);

	uint32_t (*leave)(struct wlr_tablet_pad_v2_grab *grab,
		struct wlr_surface *surface);

	uint32_t (*mode)(struct wlr_tablet_pad_v2_grab *grab,
		size_t group, uint32_t mode, uint32_t time);

	void (*cancel)(struct wlr_tablet_pad_v2_grab *grab);
};

void wlr_tablet_v2_end_grab(struct wlr_tablet_v2_tablet_pad *pad);
void wlr_tablet_v2_start_grab(struct wlr_tablet_v2_tablet_pad *pad, struct wlr_tablet_pad_v2_grab *grab);

bool wlr_surface_accepts_tablet_v2(struct wlr_tablet_v2_tablet *tablet,
	struct wlr_surface *surface);
#endif /* WLR_TYPES_WLR_TABLET_V2_H */
