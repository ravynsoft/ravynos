/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_TYPES_WLR_OUTPUT_H
#define WLR_TYPES_WLR_OUTPUT_H

#include <pixman.h>
#include <stdbool.h>
#include <time.h>
#include <wayland-server-protocol.h>
#include <wayland-util.h>
#include <wlr/types/wlr_buffer.h>
#include <wlr/util/addon.h>

struct wlr_output_mode {
	int32_t width, height;
	int32_t refresh; // mHz
	bool preferred;
	struct wl_list link;
};

struct wlr_output_cursor {
	struct wlr_output *output;
	double x, y;
	bool enabled;
	bool visible;
	uint32_t width, height;
	int32_t hotspot_x, hotspot_y;
	struct wl_list link;

	// only when using a software cursor without a surface
	struct wlr_texture *texture;

	// only when using a cursor surface
	struct wlr_surface *surface;
	struct wl_listener surface_commit;
	struct wl_listener surface_destroy;

	struct {
		struct wl_signal destroy;
	} events;
};

enum wlr_output_adaptive_sync_status {
	WLR_OUTPUT_ADAPTIVE_SYNC_DISABLED,
	WLR_OUTPUT_ADAPTIVE_SYNC_ENABLED,
	WLR_OUTPUT_ADAPTIVE_SYNC_UNKNOWN, // requested, but maybe disabled
};

enum wlr_output_state_field {
	WLR_OUTPUT_STATE_BUFFER = 1 << 0,
	WLR_OUTPUT_STATE_DAMAGE = 1 << 1,
	WLR_OUTPUT_STATE_MODE = 1 << 2,
	WLR_OUTPUT_STATE_ENABLED = 1 << 3,
	WLR_OUTPUT_STATE_SCALE = 1 << 4,
	WLR_OUTPUT_STATE_TRANSFORM = 1 << 5,
	WLR_OUTPUT_STATE_ADAPTIVE_SYNC_ENABLED = 1 << 6,
	WLR_OUTPUT_STATE_GAMMA_LUT = 1 << 7,
	WLR_OUTPUT_STATE_RENDER_FORMAT = 1 << 8,
};

enum wlr_output_state_mode_type {
	WLR_OUTPUT_STATE_MODE_FIXED,
	WLR_OUTPUT_STATE_MODE_CUSTOM,
};

/**
 * Holds the double-buffered output state.
 */
struct wlr_output_state {
	uint32_t committed; // enum wlr_output_state_field
	pixman_region32_t damage; // output-buffer-local coordinates
	bool enabled;
	float scale;
	enum wl_output_transform transform;
	bool adaptive_sync_enabled;
	uint32_t render_format;

	// only valid if WLR_OUTPUT_STATE_BUFFER
	struct wlr_buffer *buffer;

	// only valid if WLR_OUTPUT_STATE_MODE
	enum wlr_output_state_mode_type mode_type;
	struct wlr_output_mode *mode;
	struct {
		int32_t width, height;
		int32_t refresh; // mHz, may be zero
	} custom_mode;

	// only valid if WLR_OUTPUT_STATE_GAMMA_LUT
	uint16_t *gamma_lut;
	size_t gamma_lut_size;
};

struct wlr_output_impl;

/**
 * A compositor output region. This typically corresponds to a monitor that
 * displays part of the compositor space.
 *
 * The `frame` event will be emitted when it is a good time for the compositor
 * to submit a new frame.
 *
 * To render a new frame, compositors should call `wlr_output_attach_render`,
 * render and call `wlr_output_commit`. No rendering should happen outside a
 * `frame` event handler or before `wlr_output_attach_render`.
 */
struct wlr_output {
	const struct wlr_output_impl *impl;
	struct wlr_backend *backend;
	struct wl_display *display;

	struct wl_global *global;
	struct wl_list resources;

	char *name;
	char *description; // may be NULL
	char make[56];
	char model[16];
	char serial[16];
	int32_t phys_width, phys_height; // mm

	// Note: some backends may have zero modes
	struct wl_list modes; // wlr_output_mode::link
	struct wlr_output_mode *current_mode;
	int32_t width, height;
	int32_t refresh; // mHz, may be zero

	bool enabled;
	float scale;
	enum wl_output_subpixel subpixel;
	enum wl_output_transform transform;
	enum wlr_output_adaptive_sync_status adaptive_sync_status;
	uint32_t render_format;

	bool needs_frame;
	// damage for cursors and fullscreen surface, in output-local coordinates
	bool frame_pending;
	float transform_matrix[9];

	// true for example with VR headsets
	bool non_desktop;

	struct wlr_output_state pending;

	// Commit sequence number. Incremented on each commit, may overflow.
	uint32_t commit_seq;

	struct {
		// Request to render a frame
		struct wl_signal frame;
		// Emitted when software cursors or backend-specific logic damage the
		// output
		struct wl_signal damage; // wlr_output_event_damage
		// Emitted when a new frame needs to be committed (because of
		// backend-specific logic)
		struct wl_signal needs_frame;
		// Emitted right before commit
		struct wl_signal precommit; // wlr_output_event_precommit
		// Emitted right after commit
		struct wl_signal commit; // wlr_output_event_commit
		// Emitted right after the buffer has been presented to the user
		struct wl_signal present; // wlr_output_event_present
		// Emitted after a client bound the wl_output global
		struct wl_signal bind; // wlr_output_event_bind
		struct wl_signal enable;
		struct wl_signal mode;
		struct wl_signal description;
		struct wl_signal destroy;
	} events;

	struct wl_event_source *idle_frame;
	struct wl_event_source *idle_done;

	int attach_render_locks; // number of locks forcing rendering

	struct wl_list cursors; // wlr_output_cursor::link
	struct wlr_output_cursor *hardware_cursor;
	struct wlr_swapchain *cursor_swapchain;
	struct wlr_buffer *cursor_front_buffer;
	int software_cursor_locks; // number of locks forcing software cursors

	struct wlr_allocator *allocator;
	struct wlr_renderer *renderer;
	struct wlr_swapchain *swapchain;
	struct wlr_buffer *back_buffer;

	struct wl_listener display_destroy;

	struct wlr_addon_set addons;

	void *data;
};

struct wlr_output_event_damage {
	struct wlr_output *output;
	pixman_region32_t *damage; // output-buffer-local coordinates
};

struct wlr_output_event_precommit {
	struct wlr_output *output;
	struct timespec *when;
};

struct wlr_output_event_commit {
	struct wlr_output *output;
	uint32_t committed; // bitmask of enum wlr_output_state_field
	struct timespec *when;
	struct wlr_buffer *buffer; // NULL if no buffer is committed
};

enum wlr_output_present_flag {
	// The presentation was synchronized to the "vertical retrace" by the
	// display hardware such that tearing does not happen.
	WLR_OUTPUT_PRESENT_VSYNC = 0x1,
	// The display hardware provided measurements that the hardware driver
	// converted into a presentation timestamp.
	WLR_OUTPUT_PRESENT_HW_CLOCK = 0x2,
	// The display hardware signalled that it started using the new image
	// content.
	WLR_OUTPUT_PRESENT_HW_COMPLETION = 0x4,
	// The presentation of this update was done zero-copy.
	WLR_OUTPUT_PRESENT_ZERO_COPY = 0x8,
};

struct wlr_output_event_present {
	struct wlr_output *output;
	// Frame submission for which this presentation event is for (see
	// wlr_output.commit_seq).
	uint32_t commit_seq;
	// Whether the frame was presented at all.
	bool presented;
	// Time when the content update turned into light the first time.
	struct timespec *when;
	// Vertical retrace counter. Zero if unavailable.
	unsigned seq;
	// Prediction of how many nanoseconds after `when` the very next output
	// refresh may occur. Zero if unknown.
	int refresh; // nsec
	uint32_t flags; // enum wlr_output_present_flag
};

struct wlr_output_event_bind {
	struct wlr_output *output;
	struct wl_resource *resource;
};

struct wlr_surface;

/**
 * Enables or disables the output. A disabled output is turned off and doesn't
 * emit `frame` events.
 *
 * Whether an output is enabled is double-buffered state, see
 * `wlr_output_commit`.
 */
void wlr_output_enable(struct wlr_output *output, bool enable);
void wlr_output_create_global(struct wlr_output *output);
void wlr_output_destroy_global(struct wlr_output *output);
/**
 * Initialize the output's rendering subsystem with the provided allocator and
 * renderer. Can only be called once.
 *
 * Call this function prior to any call to wlr_output_attach_render,
 * wlr_output_commit or wlr_output_cursor_create.
 *
 * The buffer capabilities of the provided must match the capabilities of the
 * output's backend. Returns false otherwise.
 */
bool wlr_output_init_render(struct wlr_output *output,
	struct wlr_allocator *allocator, struct wlr_renderer *renderer);
/**
 * Returns the preferred mode for this output. If the output doesn't support
 * modes, returns NULL.
 */
struct wlr_output_mode *wlr_output_preferred_mode(struct wlr_output *output);
/**
 * Sets the output mode. The output needs to be enabled.
 *
 * Mode is double-buffered state, see `wlr_output_commit`.
 */
void wlr_output_set_mode(struct wlr_output *output,
	struct wlr_output_mode *mode);
/**
 * Sets a custom mode on the output. If modes are available, they are preferred.
 * Setting `refresh` to zero lets the backend pick a preferred value. The
 * output needs to be enabled.
 *
 * Custom mode is double-buffered state, see `wlr_output_commit`.
 */
void wlr_output_set_custom_mode(struct wlr_output *output, int32_t width,
	int32_t height, int32_t refresh);
/**
 * Sets a transform for the output.
 *
 * Transform is double-buffered state, see `wlr_output_commit`.
 */
void wlr_output_set_transform(struct wlr_output *output,
	enum wl_output_transform transform);
/**
 * Enables or disables adaptive sync (ie. variable refresh rate) on this
 * output. This is just a hint, the backend is free to ignore this setting.
 *
 * When enabled, compositors can submit frames a little bit later than the
 * deadline without dropping a frame.
 *
 * Adaptive sync is double-buffered state, see `wlr_output_commit`.
 */
void wlr_output_enable_adaptive_sync(struct wlr_output *output, bool enabled);
/**
 * Set the output buffer render format. Default value: DRM_FORMAT_XRGB8888
 *
 * While high bit depth render formats are necessary for a monitor to receive
 * useful high bit data, they do not guarantee it; a DRM_FORMAT_XBGR2101010
 * buffer will only lead to sending 10-bpc image data to the monitor if
 * hardware and software permit this.
 *
 * This only affects the format of the output buffer used when rendering,
 * as with `wlr_output_attach_render`. It has no impact on the cursor buffer
 * format, or on the formats supported for direct scan-out (see also
 * `wlr_output_attach_buffer`).
 *
 * This format is double-buffered state, see `wlr_output_commit`.
 */
void wlr_output_set_render_format(struct wlr_output *output, uint32_t format);
/**
 * Sets a scale for the output.
 *
 * Scale is double-buffered state, see `wlr_output_commit`.
 */
void wlr_output_set_scale(struct wlr_output *output, float scale);
void wlr_output_set_subpixel(struct wlr_output *output,
	enum wl_output_subpixel subpixel);
/**
 * Set the output name.
 *
 * Output names are subject to the following rules:
 *
 * - Each output name must be unique.
 * - The name cannot change after the output has been advertised to clients.
 *
 * For more details, see the protocol documentation for wl_output.name.
 */
void wlr_output_set_name(struct wlr_output *output, const char *name);
void wlr_output_set_description(struct wlr_output *output, const char *desc);
/**
 * Schedule a done event.
 *
 * This is intended to be used by wl_output add-on interfaces.
 */
void wlr_output_schedule_done(struct wlr_output *output);
void wlr_output_destroy(struct wlr_output *output);
/**
 * Computes the transformed output resolution.
 */
void wlr_output_transformed_resolution(struct wlr_output *output,
	int *width, int *height);
/**
 * Computes the transformed and scaled output resolution.
 */
void wlr_output_effective_resolution(struct wlr_output *output,
	int *width, int *height);
/**
 * Attach the renderer's buffer to the output. Compositors must call this
 * function before rendering. After they are done rendering, they should call
 * `wlr_output_commit` to submit the new frame. The output needs to be
 * enabled.
 *
 * If non-NULL, `buffer_age` is set to the drawing buffer age in number of
 * frames or -1 if unknown. This is useful for damage tracking.
 *
 * If the compositor decides not to render after calling this function, it
 * must call wlr_output_rollback.
 */
bool wlr_output_attach_render(struct wlr_output *output, int *buffer_age);
/**
 * Attach a buffer to the output. Compositors should call `wlr_output_commit`
 * to submit the new frame. The output needs to be enabled.
 *
 * Not all backends support direct scan-out on all buffers. Compositors can
 * check whether a buffer is supported by calling `wlr_output_test`.
 */
void wlr_output_attach_buffer(struct wlr_output *output,
	struct wlr_buffer *buffer);
/**
 * Get the preferred format for reading pixels.
 * This function might change the current rendering context.
 */
uint32_t wlr_output_preferred_read_format(struct wlr_output *output);
/**
 * Set the damage region for the frame to be submitted. This is the region of
 * the screen that has changed since the last frame.
 *
 * Compositors implementing damage tracking should call this function with the
 * damaged region in output-buffer-local coordinates.
 *
 * This region is not to be confused with the renderer's buffer damage, ie. the
 * region compositors need to repaint. Compositors usually need to repaint more
 * than what changed since last frame since multiple render buffers are used.
 */
void wlr_output_set_damage(struct wlr_output *output,
	pixman_region32_t *damage);
/**
 * Test whether the pending output state would be accepted by the backend. If
 * this function returns true, `wlr_output_commit` can only fail due to a
 * runtime error.
 *
 * This function doesn't mutate the pending state.
 */
bool wlr_output_test(struct wlr_output *output);
/**
 * Commit the pending output state. If `wlr_output_attach_render` has been
 * called, the pending frame will be submitted for display and a `frame` event
 * will be scheduled.
 *
 * On failure, the pending changes are rolled back.
 */
bool wlr_output_commit(struct wlr_output *output);
/**
 * Discard the pending output state.
 */
void wlr_output_rollback(struct wlr_output *output);
/**
 * Manually schedules a `frame` event. If a `frame` event is already pending,
 * it is a no-op.
 */
void wlr_output_schedule_frame(struct wlr_output *output);
/**
 * Returns the maximum length of each gamma ramp, or 0 if unsupported.
 */
size_t wlr_output_get_gamma_size(struct wlr_output *output);
/**
 * Sets the gamma table for this output. `r`, `g` and `b` are gamma ramps for
 * red, green and blue. `size` is the length of the ramps and must not exceed
 * the value returned by `wlr_output_get_gamma_size`.
 *
 * Providing zero-sized ramps resets the gamma table.
 *
 * The gamma table is double-buffered state, see `wlr_output_commit`.
 */
void wlr_output_set_gamma(struct wlr_output *output, size_t size,
	const uint16_t *r, const uint16_t *g, const uint16_t *b);
/**
 * Returns the wlr_output matching the provided wl_output resource. If the
 * resource isn't a wl_output, it aborts. If the resource is inert (because the
 * wlr_output has been destroyed), NULL is returned.
 */
struct wlr_output *wlr_output_from_resource(struct wl_resource *resource);
/**
 * Locks the output to only use rendering instead of direct scan-out. This is
 * useful if direct scan-out needs to be temporarily disabled (e.g. during
 * screen capture). There must be as many unlocks as there have been locks to
 * restore the original state. There should never be an unlock before a lock.
 */
void wlr_output_lock_attach_render(struct wlr_output *output, bool lock);
/**
 * Locks the output to only use software cursors instead of hardware cursors.
 * This is useful if hardware cursors need to be temporarily disabled (e.g.
 * during screen capture). There must be as many unlocks as there have been
 * locks to restore the original state. There should never be an unlock before
 * a lock.
 */
void wlr_output_lock_software_cursors(struct wlr_output *output, bool lock);
/**
 * Renders software cursors. This is a utility function that can be called when
 * compositors render.
 */
void wlr_output_render_software_cursors(struct wlr_output *output,
	pixman_region32_t *damage);
/**
 * Get the set of DRM formats suitable for the primary buffer, assuming a
 * buffer with the specified capabilities.
 *
 * NULL is returned if the backend doesn't have any format constraint, ie. all
 * formats are supported. An empty set is returned if the backend doesn't
 * support any format.
 */
const struct wlr_drm_format_set *wlr_output_get_primary_formats(
	struct wlr_output *output, uint32_t buffer_caps);


struct wlr_output_cursor *wlr_output_cursor_create(struct wlr_output *output);
/**
 * Sets the cursor image. The image must be already scaled for the output.
 */
bool wlr_output_cursor_set_image(struct wlr_output_cursor *cursor,
	const uint8_t *pixels, int32_t stride, uint32_t width, uint32_t height,
	int32_t hotspot_x, int32_t hotspot_y);
void wlr_output_cursor_set_surface(struct wlr_output_cursor *cursor,
	struct wlr_surface *surface, int32_t hotspot_x, int32_t hotspot_y);
bool wlr_output_cursor_move(struct wlr_output_cursor *cursor,
	double x, double y);
void wlr_output_cursor_destroy(struct wlr_output_cursor *cursor);


/**
 * Returns the transform that, when composed with `tr`, gives
 * `WL_OUTPUT_TRANSFORM_NORMAL`.
 */
enum wl_output_transform wlr_output_transform_invert(
	enum wl_output_transform tr);

/**
 * Returns a transform that, when applied, has the same effect as applying
 * sequentially `tr_a` and `tr_b`.
 */
enum wl_output_transform wlr_output_transform_compose(
	enum wl_output_transform tr_a, enum wl_output_transform tr_b);

#endif
