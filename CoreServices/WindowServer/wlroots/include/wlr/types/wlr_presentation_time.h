/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_TYPES_WLR_PRESENTATION_TIME_H
#define WLR_TYPES_WLR_PRESENTATION_TIME_H

#include <stdbool.h>
#include <stddef.h>
#include <time.h>
#include <wayland-server-core.h>

struct wlr_surface;

struct wlr_output;
struct wlr_output_event_present;

struct wlr_presentation {
	struct wl_global *global;
	clockid_t clock;

	struct {
		struct wl_signal destroy;
	} events;

	struct wl_listener display_destroy;
};

struct wlr_presentation_feedback {
	struct wl_list resources; // wl_resource_get_link

	// Only when the wlr_presentation_surface_sampled_on_output helper has been
	// called
	struct wlr_output *output;
	bool output_committed;
	uint32_t output_commit_seq;

	struct wl_listener output_commit;
	struct wl_listener output_present;
	struct wl_listener output_destroy;
};

struct wlr_presentation_event {
	struct wlr_output *output;
	uint64_t tv_sec;
	uint32_t tv_nsec;
	uint32_t refresh;
	uint64_t seq;
	uint32_t flags; // wp_presentation_feedback_kind
};

struct wlr_backend;

struct wlr_presentation *wlr_presentation_create(struct wl_display *display,
	struct wlr_backend *backend);
/**
 * Mark the current surface's buffer as sampled.
 *
 * The compositor must call this function when it uses the surface's current
 * contents (e.g. when rendering the surface's current texture, when
 * referencing its current buffer, or when directly scanning out its current
 * buffer). A wlr_presentation_feedback is returned. The compositor should call
 * wlr_presentation_feedback_send_presented if this content has been displayed,
 * then wlr_presentation_feedback_destroy.
 *
 * NULL is returned if the client hasn't requested presentation feedback for
 * this surface.
 */
struct wlr_presentation_feedback *wlr_presentation_surface_sampled(
	struct wlr_presentation *presentation, struct wlr_surface *surface);
void wlr_presentation_feedback_send_presented(
	struct wlr_presentation_feedback *feedback,
	struct wlr_presentation_event *event);
void wlr_presentation_feedback_destroy(
	struct wlr_presentation_feedback *feedback);

/**
 * Fill a wlr_presentation_event from a wlr_output_event_present.
 */
void wlr_presentation_event_from_output(struct wlr_presentation_event *event,
		const struct wlr_output_event_present *output_event);

/**
 * Mark the current surface's buffer as sampled on the given output.
 *
 * Instead of calling wlr_presentation_surface_sampled and managing the
 * wlr_presentation_feedback itself, the compositor can call this function
 * before a wlr_output_commit call to indicate that the surface's current
 * contents will be displayed on the output.
 */
void wlr_presentation_surface_sampled_on_output(
	struct wlr_presentation *presentation, struct wlr_surface *surface,
	struct wlr_output *output);

#endif
