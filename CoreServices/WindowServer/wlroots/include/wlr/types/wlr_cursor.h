/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_TYPES_WLR_CURSOR_H
#define WLR_TYPES_WLR_CURSOR_H

#include <wayland-server-core.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_output.h>

struct wlr_input_device;

/**
 * wlr_cursor implements the behavior of the "cursor", that is, the image on the
 * screen typically moved about with a mouse or so. It provides tracking for
 * this in global coordinates, and integrates with wlr_output,
 * wlr_output_layout, and wlr_input_device. You can use it to abstract multiple
 * input devices over a single cursor, constrain cursor movement to the usable
 * area of a wlr_output_layout and communicate position updates to the hardware
 * cursor, constrain specific input devices to specific outputs or regions of
 * the screen, and so on.
 */

struct wlr_box;
struct wlr_cursor_state;

struct wlr_cursor {
	struct wlr_cursor_state *state;
	double x, y;

	/**
	 * The interpretation of these signals is the responsibility of the
	 * compositor, but some helpers are provided for your benefit. If you
	 * receive a relative motion event, for example, you may want to call
	 * wlr_cursor_move. If you receive an absolute event, call
	 * wlr_cursor_warp_absolute. If you pass an input device into these
	 * functions, it will apply the region/output constraints associated with
	 * that device to the resulting cursor motion. If an output layout is
	 * attached, these functions will constrain the resulting cursor motion to
	 * within the usable space of the output layout.
	 *
	 * Re-broadcasting these signals to, for example, a wlr_seat, is also your
	 * responsibility.
	 */
	struct {
		struct wl_signal motion;
		struct wl_signal motion_absolute;
		struct wl_signal button;
		struct wl_signal axis;
		struct wl_signal frame;
		struct wl_signal swipe_begin;
		struct wl_signal swipe_update;
		struct wl_signal swipe_end;
		struct wl_signal pinch_begin;
		struct wl_signal pinch_update;
		struct wl_signal pinch_end;
		struct wl_signal hold_begin;
		struct wl_signal hold_end;

		struct wl_signal touch_up;
		struct wl_signal touch_down;
		struct wl_signal touch_motion;
		struct wl_signal touch_cancel;
		struct wl_signal touch_frame;

		struct wl_signal tablet_tool_axis;
		struct wl_signal tablet_tool_proximity;
		struct wl_signal tablet_tool_tip;
		struct wl_signal tablet_tool_button;
	} events;

	void *data;
};

struct wlr_cursor *wlr_cursor_create(void);

void wlr_cursor_destroy(struct wlr_cursor *cur);

/**
 * Warp the cursor to the given x and y in layout coordinates. If x and y are
 * out of the layout boundaries or constraints, no warp will happen.
 *
 * `dev` may be passed to respect device mapping constraints. If `dev` is NULL,
 * device mapping constraints will be ignored.
 *
 * Returns true when the cursor warp was successful.
 */
bool wlr_cursor_warp(struct wlr_cursor *cur, struct wlr_input_device *dev,
	double lx, double ly);

/**
 * Convert absolute 0..1 coordinates to layout coordinates.
 *
 * `dev` may be passed to respect device mapping constraints. If `dev` is NULL,
 * device mapping constraints will be ignored.
 */
void wlr_cursor_absolute_to_layout_coords(struct wlr_cursor *cur,
	struct wlr_input_device *dev, double x, double y, double *lx, double *ly);


/**
 * Warp the cursor to the given x and y coordinates. If the given point is out
 * of the layout boundaries or constraints, the closest point will be used.
 * If one coordinate is NAN, it will be ignored.
 *
 * `dev` may be passed to respect device mapping constraints. If `dev` is NULL,
 * device mapping constraints will be ignored.
 */
void wlr_cursor_warp_closest(struct wlr_cursor *cur,
	struct wlr_input_device *dev, double x, double y);

/**
 * Warp the cursor to the given x and y in absolute 0..1 coordinates. If the
 * given point is out of the layout boundaries or constraints, the closest point
 * will be used. If one coordinate is NAN, it will be ignored.
 *
 * `dev` may be passed to respect device mapping constraints. If `dev` is NULL,
 * device mapping constraints will be ignored.
 */
void wlr_cursor_warp_absolute(struct wlr_cursor *cur,
	struct wlr_input_device *dev, double x, double y);

/**
 * Move the cursor in the direction of the given x and y layout coordinates. If
 * one coordinate is NAN, it will be ignored.
 *
 * `dev` may be passed to respect device mapping constraints. If `dev` is NULL,
 * device mapping constraints will be ignored.
 */
void wlr_cursor_move(struct wlr_cursor *cur, struct wlr_input_device *dev,
	double delta_x, double delta_y);

/**
 * Set the cursor image. stride is given in bytes. If pixels is NULL, hides the
 * cursor.
 *
 * If scale isn't zero, the image is only set on outputs having the provided
 * scale.
 */
void wlr_cursor_set_image(struct wlr_cursor *cur, const uint8_t *pixels,
	int32_t stride, uint32_t width, uint32_t height, int32_t hotspot_x,
	int32_t hotspot_y, float scale);

/**
 * Set the cursor surface. The surface can be committed to update the cursor
 * image. The surface position is subtracted from the hotspot. A NULL surface
 * commit hides the cursor.
 */
void wlr_cursor_set_surface(struct wlr_cursor *cur, struct wlr_surface *surface,
	int32_t hotspot_x, int32_t hotspot_y);

/**
 * Attaches this input device to this cursor. The input device must be one of:
 *
 * - WLR_INPUT_DEVICE_POINTER
 * - WLR_INPUT_DEVICE_TOUCH
 * - WLR_INPUT_DEVICE_TABLET_TOOL
 */
void wlr_cursor_attach_input_device(struct wlr_cursor *cur,
		struct wlr_input_device *dev);

void wlr_cursor_detach_input_device(struct wlr_cursor *cur,
		struct wlr_input_device *dev);
/**
 * Uses the given layout to establish the boundaries and movement semantics of
 * this cursor. Cursors without an output layout allow infinite movement in any
 * direction and do not support absolute input events.
 */
void wlr_cursor_attach_output_layout(struct wlr_cursor *cur,
	struct wlr_output_layout *l);

/**
 * Attaches this cursor to the given output, which must be among the outputs in
 * the current output_layout for this cursor. This call is invalid for a cursor
 * without an associated output layout.
 */
void wlr_cursor_map_to_output(struct wlr_cursor *cur,
	struct wlr_output *output);

/**
 * Maps all input from a specific input device to a given output. The input
 * device must be attached to this cursor and the output must be among the
 * outputs in the attached output layout.
 */
void wlr_cursor_map_input_to_output(struct wlr_cursor *cur,
	struct wlr_input_device *dev, struct wlr_output *output);

/**
 * Maps this cursor to an arbitrary region on the associated wlr_output_layout.
 */
void wlr_cursor_map_to_region(struct wlr_cursor *cur, const struct wlr_box *box);

/**
 * Maps inputs from this input device to an arbitrary region on the associated
 * wlr_output_layout.
 */
void wlr_cursor_map_input_to_region(struct wlr_cursor *cur,
	struct wlr_input_device *dev, const struct wlr_box *box);

#endif
