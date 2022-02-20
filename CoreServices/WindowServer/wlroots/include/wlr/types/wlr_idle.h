/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_TYPES_WLR_IDLE_H
#define WLR_TYPES_WLR_IDLE_H

#include <wayland-server-core.h>
#include <wlr/types/wlr_seat.h>

/**
 * Idle protocol is used to create timers which will notify the client when the
 * compositor does not receive any input for a given time(in milliseconds). Also
 * the client will be notified when the timer receives an activity notify and already
 * was in idle state. Besides this, the client is able to simulate user activity
 * which will reset the timers and at any time can destroy the timer.
 */


struct wlr_idle {
	struct wl_global *global;
	struct wl_list idle_timers; // wlr_idle_timeout::link
	struct wl_event_loop *event_loop;
	bool enabled;

	struct wl_listener display_destroy;
	struct {
		struct wl_signal activity_notify;
		struct wl_signal destroy;
	} events;

	void *data;
};

struct wlr_idle_timeout {
	struct wl_resource *resource;
	struct wl_list link;
	struct wlr_seat *seat;

	struct wl_event_source *idle_source;
	bool idle_state;
	bool enabled;
	uint32_t timeout; // milliseconds

	struct {
		struct wl_signal idle;
		struct wl_signal resume;
		struct wl_signal destroy;
	} events;

	struct wl_listener input_listener;
	struct wl_listener seat_destroy;

	void *data;
};

struct wlr_idle *wlr_idle_create(struct wl_display *display);

/**
 * Send notification to restart all timers for the given seat. Called by
 * compositor when there is an user activity event on that seat.
 */
void wlr_idle_notify_activity(struct wlr_idle *idle, struct wlr_seat *seat);

/**
 * Enable or disable timers for a given idle resource by seat.
 * Passing a NULL seat means update timers for all seats.
 */
void wlr_idle_set_enabled(struct wlr_idle *idle, struct wlr_seat *seat,
	bool enabled);

/**
 * Create a new timer on the given seat. The idle event will be called after
 * the given amount of milliseconds of inactivity, and the resumed event will
 * be sent at the first user activity after the fired event.
 */
struct wlr_idle_timeout *wlr_idle_timeout_create(struct wlr_idle *idle,
	struct wlr_seat *seat, uint32_t timeout);

void wlr_idle_timeout_destroy(struct wlr_idle_timeout *timeout);

#endif
