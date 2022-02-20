/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_INTERFACES_WLR_OUTPUT_H
#define WLR_INTERFACES_WLR_OUTPUT_H

#include <stdbool.h>
#include <wlr/backend.h>
#include <wlr/types/wlr_output.h>

/**
 * Output state fields that don't require backend support. Backends can ignore
 * them without breaking the API contract.
 */
#define WLR_OUTPUT_STATE_BACKEND_OPTIONAL \
	(WLR_OUTPUT_STATE_DAMAGE | \
	WLR_OUTPUT_STATE_SCALE | \
	WLR_OUTPUT_STATE_TRANSFORM | \
	WLR_OUTPUT_STATE_RENDER_FORMAT | \
	WLR_OUTPUT_STATE_ADAPTIVE_SYNC_ENABLED)

/**
 * A backend implementation of wlr_output.
 *
 * The commit function is mandatory. Other functions are optional.
 */
struct wlr_output_impl {
	/**
	 * Set the output cursor plane image.
	 *
	 * If buffer is NULL, the cursor should be hidden.
	 *
	 * The hotspot indicates the offset that needs to be applied to the
	 * top-left corner of the image to match the cursor position. In other
	 * words, the image should be displayed at (x - hotspot_x, y - hotspot_y).
	 * The hotspot is given in the texture's coordinate space.
	 */
	bool (*set_cursor)(struct wlr_output *output, struct wlr_buffer *buffer,
		int hotspot_x, int hotspot_y);
	/**
	 * Set the output cursor plane position.
	 *
	 * The position is relative to the cursor hotspot, see set_cursor.
	 */
	bool (*move_cursor)(struct wlr_output *output, int x, int y);
	/**
	 * Cleanup backend-specific resources tied to the output.
	 */
	void (*destroy)(struct wlr_output *output);
	/**
	 * Check that the pending output state is a valid configuration.
	 *
	 * If this function returns true, commit can only fail due to a runtime
	 * error.
	 */
	bool (*test)(struct wlr_output *output);
	/**
	 * Commit the pending output state.
	 *
	 * If a buffer has been attached, a frame event is scheduled.
	 */
	bool (*commit)(struct wlr_output *output);
	/**
	 * Get the maximum number of gamma LUT elements for each channel.
	 *
	 * Zero can be returned if the output doesn't support gamma LUTs.
	 */
	size_t (*get_gamma_size)(struct wlr_output *output);
	/**
	 * Get the list of formats suitable for the cursor, assuming a buffer with
	 * the specified capabilities.
	 *
	 * If unimplemented, the cursor buffer has no format constraint. If NULL is
	 * returned, no format is suitable.
	 */
	const struct wlr_drm_format_set *(*get_cursor_formats)(
		struct wlr_output *output, uint32_t buffer_caps);
	/**
	 * Get the size suitable for the cursor buffer. Attempts to use a different
	 * size for the cursor may fail.
	 */
	void (*get_cursor_size)(struct wlr_output *output, int *width, int *height);
	/**
	 * Get the list of DRM formats suitable for the primary buffer,
	 * assuming a buffer with the specified capabilities.
	 *
	 * If unimplemented, the primary buffer has no format constraint. If NULL
	 * is returned, no format is suitable.
	 */
	const struct wlr_drm_format_set *(*get_primary_formats)(
		struct wlr_output *output, uint32_t buffer_caps);
};

/**
 * Initialize a new output.
 */
void wlr_output_init(struct wlr_output *output, struct wlr_backend *backend,
	const struct wlr_output_impl *impl, struct wl_display *display);
/**
 * Update the current output mode.
 *
 * The backend must call this function when the mode is updated to notify
 * compositors about the change.
 */
void wlr_output_update_mode(struct wlr_output *output,
	struct wlr_output_mode *mode);
/**
 * Update the current output custom mode.
 *
 * The backend must call this function when the mode is updated to notify
 * compositors about the change.
 */
void wlr_output_update_custom_mode(struct wlr_output *output, int32_t width,
	int32_t height, int32_t refresh);
/**
 * Update the current output status.
 *
 * The backend must call this function when the status is updated to notify
 * compositors about the change.
 */
void wlr_output_update_enabled(struct wlr_output *output, bool enabled);
/**
 * Notify compositors that they need to submit a new frame in order to apply
 * output changes.
 */
void wlr_output_update_needs_frame(struct wlr_output *output);
/**
 * Notify compositors that the output needs to be fully repainted.
 */
void wlr_output_damage_whole(struct wlr_output *output);
/**
 * Send a frame event.
 *
 * See wlr_output.events.frame.
 */
void wlr_output_send_frame(struct wlr_output *output);
/**
 * Send a present event.
 *
 * See wlr_output.events.present.
 */
void wlr_output_send_present(struct wlr_output *output,
	struct wlr_output_event_present *event);

#endif
