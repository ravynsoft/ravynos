#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-server-core.h>
#include <wlr/types/wlr_idle.h>
#include <wlr/util/log.h>
#include "idle-protocol.h"
#include "util/signal.h"

static const struct org_kde_kwin_idle_timeout_interface idle_timeout_impl;

static struct wlr_idle_timeout *idle_timeout_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource,
		&org_kde_kwin_idle_timeout_interface, &idle_timeout_impl));
	return wl_resource_get_user_data(resource);
}

static int idle_notify(void *data) {
	struct wlr_idle_timeout *timer = data;
	if (timer->idle_state) {
		return 0;
	}
	timer->idle_state = true;
	wlr_signal_emit_safe(&timer->events.idle, timer);

	if (timer->resource) {
		org_kde_kwin_idle_timeout_send_idle(timer->resource);
	}
	return 1;
}

static void handle_activity(struct wlr_idle_timeout *timer) {
	if (!timer->enabled) {
		return;
	}

	// in case the previous state was sleeping send a resume event and switch state
	if (timer->idle_state) {
		timer->idle_state = false;
		wlr_signal_emit_safe(&timer->events.resume, timer);

		if (timer->resource) {
			org_kde_kwin_idle_timeout_send_resumed(timer->resource);
		}
	}

	// rearm the timer
	wl_event_source_timer_update(timer->idle_source, timer->timeout);
	if (timer->timeout == 0) {
		idle_notify(timer);
	}
}

static void handle_timer_resource_destroy(struct wl_resource *timer_resource) {
	struct wlr_idle_timeout *timer = idle_timeout_from_resource(timer_resource);
	if (timer != NULL) {
		wlr_idle_timeout_destroy(timer);
	}
}

static void handle_seat_destroy(struct wl_listener *listener, void *data) {
	struct wlr_idle_timeout *timer = wl_container_of(listener, timer, seat_destroy);
	if (timer != NULL) {
		wlr_idle_timeout_destroy(timer);
	}
}

static void release_idle_timeout(struct wl_client *client,
		struct wl_resource *resource){
	handle_timer_resource_destroy(resource);
}

static void simulate_activity(struct wl_client *client,
		struct wl_resource *resource){
	struct wlr_idle_timeout *timer = idle_timeout_from_resource(resource);
	handle_activity(timer);
}

static const struct org_kde_kwin_idle_timeout_interface idle_timeout_impl = {
	.release = release_idle_timeout,
	.simulate_user_activity = simulate_activity,
};

static const struct org_kde_kwin_idle_interface idle_impl;

static struct wlr_idle *idle_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource, &org_kde_kwin_idle_interface,
		&idle_impl));
	return wl_resource_get_user_data(resource);
}

static void handle_input_notification(struct wl_listener *listener, void *data) {
	struct wlr_idle_timeout *timer =
		wl_container_of(listener, timer, input_listener);
	struct wlr_seat *seat = data;
	if (timer->seat == seat) {
		handle_activity(timer);
	}
}

static struct wlr_idle_timeout *create_timer(struct wlr_idle *idle,
		struct wlr_seat *seat, uint32_t timeout, struct wl_resource *resource) {
	struct wlr_idle_timeout *timer =
		calloc(1, sizeof(struct wlr_idle_timeout));
	if (!timer) {
		return NULL;
	}

	timer->seat = seat;
	timer->timeout = timeout;
	timer->idle_state = false;
	timer->enabled = idle->enabled;

	wl_list_insert(&idle->idle_timers, &timer->link);
	wl_signal_init(&timer->events.idle);
	wl_signal_init(&timer->events.resume);
	wl_signal_init(&timer->events.destroy);

	timer->seat_destroy.notify = handle_seat_destroy;
	wl_signal_add(&timer->seat->events.destroy, &timer->seat_destroy);

	timer->input_listener.notify = handle_input_notification;
	wl_signal_add(&idle->events.activity_notify, &timer->input_listener);
	// create the timer
	timer->idle_source =
		wl_event_loop_add_timer(idle->event_loop, idle_notify, timer);
	if (timer->idle_source == NULL) {
		wl_list_remove(&timer->link);
		wl_list_remove(&timer->input_listener.link);
		wl_list_remove(&timer->seat_destroy.link);
		free(timer);
		return NULL;
	}

	if (resource) {
		timer->resource = resource;
		wl_resource_set_user_data(resource, timer);
	}

	if (timer->enabled) {
		// arm the timer
		wl_event_source_timer_update(timer->idle_source, timer->timeout);
		if (timer->timeout == 0) {
			idle_notify(timer);
		}
	}

	return timer;
}

static void create_idle_timer(struct wl_client *client,
		struct wl_resource *idle_resource, uint32_t id,
		struct wl_resource *seat_resource, uint32_t timeout) {
	struct wlr_idle *idle = idle_from_resource(idle_resource);
	struct wlr_seat_client *client_seat =
		wlr_seat_client_from_resource(seat_resource);

	struct wl_resource *resource = wl_resource_create(client,
		&org_kde_kwin_idle_timeout_interface,
		wl_resource_get_version(idle_resource), id);
	if (resource == NULL) {
		wl_resource_post_no_memory(idle_resource);
		return;
	}
	wl_resource_set_implementation(resource, &idle_timeout_impl,
		NULL, handle_timer_resource_destroy);

	if (!create_timer(idle, client_seat->seat, timeout, resource)) {
		wl_resource_post_no_memory(resource);
	}
}

static const struct org_kde_kwin_idle_interface idle_impl = {
	.get_idle_timeout = create_idle_timer,
};

void wlr_idle_set_enabled(struct wlr_idle *idle, struct wlr_seat *seat,
		bool enabled) {
	if (idle->enabled == enabled) {
		return;
	}
	wlr_log(WLR_DEBUG, "%s idle timers for %s",
		enabled ? "Enabling" : "Disabling",
		seat ? seat->name : "all seats");
	idle->enabled = enabled;
	struct wlr_idle_timeout *timer;
	wl_list_for_each(timer, &idle->idle_timers, link) {
		if (seat != NULL && timer->seat != seat) {
			continue;
		}
		int timeout = enabled ? timer->timeout : 0;
		wl_event_source_timer_update(timer->idle_source, timeout);
		timer->enabled = enabled;
	}
}

static void idle_bind(struct wl_client *wl_client, void *data,
		uint32_t version, uint32_t id) {
	struct wlr_idle *idle = data;
	assert(wl_client && idle);

	struct wl_resource *wl_resource = wl_resource_create(wl_client,
		&org_kde_kwin_idle_interface, version, id);
	if (wl_resource == NULL) {
		wl_client_post_no_memory(wl_client);
		return;
	}
	wl_resource_set_implementation(wl_resource, &idle_impl, idle, NULL);
}

static void handle_display_destroy(struct wl_listener *listener, void *data) {
	struct wlr_idle *idle = wl_container_of(listener, idle, display_destroy);
	wlr_signal_emit_safe(&idle->events.destroy, idle);
	wl_list_remove(&idle->display_destroy.link);
	wl_global_destroy(idle->global);
	free(idle);
}

struct wlr_idle *wlr_idle_create(struct wl_display *display) {
	struct wlr_idle *idle = calloc(1, sizeof(struct wlr_idle));
	if (!idle) {
		return NULL;
	}
	wl_list_init(&idle->idle_timers);
	wl_signal_init(&idle->events.activity_notify);
	wl_signal_init(&idle->events.destroy);
	idle->enabled = true;

	idle->event_loop = wl_display_get_event_loop(display);
	if (idle->event_loop == NULL) {
		free(idle);
		return NULL;
	}

	idle->display_destroy.notify = handle_display_destroy;
	wl_display_add_destroy_listener(display, &idle->display_destroy);

	idle->global = wl_global_create(display, &org_kde_kwin_idle_interface,
		1, idle, idle_bind);
	if (idle->global == NULL) {
		wl_list_remove(&idle->display_destroy.link);
		free(idle);
		return NULL;
	}
	wlr_log(WLR_DEBUG, "idle manager created");
	return idle;
}

void wlr_idle_notify_activity(struct wlr_idle *idle, struct wlr_seat *seat) {
	wlr_signal_emit_safe(&idle->events.activity_notify, seat);
}

struct wlr_idle_timeout *wlr_idle_timeout_create(struct wlr_idle *idle,
		struct wlr_seat *seat, uint32_t timeout) {
	return create_timer(idle, seat, timeout, NULL);
}

void wlr_idle_timeout_destroy(struct wlr_idle_timeout *timer) {
	wlr_signal_emit_safe(&timer->events.destroy, NULL);

	wl_list_remove(&timer->input_listener.link);
	wl_list_remove(&timer->seat_destroy.link);
	wl_event_source_remove(timer->idle_source);
	wl_list_remove(&timer->link);

	if (timer->resource) {
		wl_resource_set_user_data(timer->resource, NULL);
	}

	free(timer);
}
