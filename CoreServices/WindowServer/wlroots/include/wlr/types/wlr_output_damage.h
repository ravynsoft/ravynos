/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_TYPES_WLR_OUTPUT_DAMAGE_H
#define WLR_TYPES_WLR_OUTPUT_DAMAGE_H

#include <pixman.h>
#include <time.h>
#include <wlr/types/wlr_output.h>

/**
 * Damage tracking requires to keep track of previous frames' damage. To allow
 * damage tracking to work with triple buffering, a history of two frames is
 * required.
 */
#define WLR_OUTPUT_DAMAGE_PREVIOUS_LEN 2

struct wlr_box;

/**
 * Tracks damage for an output.
 *
 * The `frame` event will be emitted when it is a good time for the compositor
 * to submit a new frame.
 *
 * To render a new frame, compositors should call
 * `wlr_output_damage_attach_render`, render and call `wlr_output_commit`. No
 * rendering should happen outside a `frame` event handler or before
 * `wlr_output_damage_attach_render`.
 */
struct wlr_output_damage {
	struct wlr_output *output;
	int max_rects; // max number of damaged rectangles

	pixman_region32_t current; // in output-local coordinates

	// circular queue for previous damage
	pixman_region32_t previous[WLR_OUTPUT_DAMAGE_PREVIOUS_LEN];
	size_t previous_idx;

	bool pending_attach_render;

	struct {
		struct wl_signal frame;
		struct wl_signal destroy;
	} events;

	struct wl_listener output_destroy;
	struct wl_listener output_mode;
	struct wl_listener output_needs_frame;
	struct wl_listener output_damage;
	struct wl_listener output_frame;
	struct wl_listener output_precommit;
	struct wl_listener output_commit;
};

struct wlr_output_damage *wlr_output_damage_create(struct wlr_output *output);
void wlr_output_damage_destroy(struct wlr_output_damage *output_damage);
/**
 * Attach the renderer's buffer to the output. Compositors must call this
 * function before rendering. After they are done rendering, they should call
 * `wlr_output_set_damage` and `wlr_output_commit` to submit the new frame.
 *
 * `needs_frame` will be set to true if a frame should be submitted. `damage`
 * will be set to the region of the output that needs to be repainted, in
 * output-buffer-local coordinates.
 *
 * The buffer damage region accumulates all damage since the buffer has last
 * been swapped. This is not to be confused with the output surface damage,
 * which only contains the changes between two frames.
 */
bool wlr_output_damage_attach_render(struct wlr_output_damage *output_damage,
	bool *needs_frame, pixman_region32_t *buffer_damage);
/**
 * Accumulates damage and schedules a `frame` event.
 */
void wlr_output_damage_add(struct wlr_output_damage *output_damage,
	pixman_region32_t *damage);
/**
 * Damages the whole output and schedules a `frame` event.
 */
void wlr_output_damage_add_whole(struct wlr_output_damage *output_damage);
/**
 * Accumulates damage from a box and schedules a `frame` event.
 */
void wlr_output_damage_add_box(struct wlr_output_damage *output_damage,
	struct wlr_box *box);

#endif
