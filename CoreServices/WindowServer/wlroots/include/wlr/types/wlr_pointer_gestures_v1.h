/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_TYPES_WLR_POINTER_GESTURES_V1_H
#define WLR_TYPES_WLR_POINTER_GESTURES_V1_H

#include <wayland-server-core.h>
#include <wlr/types/wlr_seat.h>

struct wlr_surface;

struct wlr_pointer_gestures_v1 {
	struct wl_global *global;
	struct wl_list swipes; // wl_resource_get_link
	struct wl_list pinches; // wl_resource_get_link
	struct wl_list holds; // wl_resource_get_link

	struct wl_listener display_destroy;

	struct {
		struct wl_signal destroy;
	} events;

	void *data;
};

struct wlr_pointer_gestures_v1 *wlr_pointer_gestures_v1_create(
	struct wl_display *display);

void wlr_pointer_gestures_v1_send_swipe_begin(
	struct wlr_pointer_gestures_v1 *gestures,
	struct wlr_seat *seat,
	uint32_t time_msec,
	uint32_t fingers);
void wlr_pointer_gestures_v1_send_swipe_update(
	struct wlr_pointer_gestures_v1 *gestures,
	struct wlr_seat *seat,
	uint32_t time_msec,
	double dx,
	double dy);
void wlr_pointer_gestures_v1_send_swipe_end(
	struct wlr_pointer_gestures_v1 *gestures,
	struct wlr_seat *seat,
	uint32_t time_msec,
	bool cancelled);

void wlr_pointer_gestures_v1_send_pinch_begin(
	struct wlr_pointer_gestures_v1 *gestures,
	struct wlr_seat *seat,
	uint32_t time_msec,
	uint32_t fingers);
void wlr_pointer_gestures_v1_send_pinch_update(
	struct wlr_pointer_gestures_v1 *gestures,
	struct wlr_seat *seat,
	uint32_t time_msec,
	double dx,
	double dy,
	double scale,
	double rotation);
void wlr_pointer_gestures_v1_send_pinch_end(
	struct wlr_pointer_gestures_v1 *gestures,
	struct wlr_seat *seat,
	uint32_t time_msec,
	bool cancelled);

void wlr_pointer_gestures_v1_send_hold_begin(
	struct wlr_pointer_gestures_v1 *gestures,
	struct wlr_seat *seat,
	uint32_t time_msec,
	uint32_t fingers);
void wlr_pointer_gestures_v1_send_hold_end(
	struct wlr_pointer_gestures_v1 *gestures,
	struct wlr_seat *seat,
	uint32_t time_msec,
	bool cancelled);

#endif
