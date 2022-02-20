/*
* This is a stable interface of wlroots. Future changes will be limited to:
 *
 * - New functions
 * - New struct members
 * - New enum members
 *
 * Note that wlroots does not make an ABI compatibility promise - in the future,
 * the layout and size of structs used by wlroots may change, requiring code
 * depending on this header to be recompiled (but not edited).
 *
 * Breaking changes are announced in the release notes and follow a 1-year
 * deprecation schedule.
 */
#ifndef WLR_UTIL_BOX_H
#define WLR_UTIL_BOX_H

#include <stdbool.h>
#include <wayland-server-protocol.h>

/**
 * A box representing a rectangle region in a 2D space.
 *
 * The x and y coordinates are inclusive, and the width and height lengths are
 * exclusive. In other words, the box starts from the coordinates (x, y), and
 * goes up to but not including (x + width, y + height)
 */
struct wlr_box {
	int x, y;
	int width, height;
};

/**
 * A floating-point box representing a rectangle region in a 2D space.
 *
 * wlr_fbox has the same semantics as wlr_box
 */
struct wlr_fbox {
	double x, y;
	double width, height;
};

/**
 * Finds the closest point within the box bounds
 *
 * Returns NAN if the box is empty
 */
void wlr_box_closest_point(const struct wlr_box *box, double x, double y,
	double *dest_x, double *dest_y);

/**
 * Gives the intersecting box between two wlr_box.
 *
 * Returns an empty wlr_box if the provided wlr_box don't intersect.
 */
bool wlr_box_intersection(struct wlr_box *dest, const struct wlr_box *box_a,
	const struct wlr_box *box_b);

/**
 * Verifies if a point is contained within the bounds of a given wlr_box.
 *
 * For example:
 *   - A point at (100, 50) is not contained in the box (0, 0, 100, 50).
 *   - A point at (10, 10) is contained in the box (10, 0, 50, 50).
 */
bool wlr_box_contains_point(const struct wlr_box *box, double x, double y);

/**
 * Checks whether a box is empty or not.
 *
 * A wlr_box is considered empty if its width and/or height is zero or negative.
 */
bool wlr_box_empty(const struct wlr_box *box);

/**
 * Transforms a box inside a (0, 0, width, height) box.
 */
void wlr_box_transform(struct wlr_box *dest, const struct wlr_box *box,
	enum wl_output_transform transform, int width, int height);

/**
 * Checks whether a box is empty or not.
 *
 * A wlr_box is considered empty if its width and/or height is zero or negative.
 */
bool wlr_fbox_empty(const struct wlr_fbox *box);

/**
 * Transforms a floating-point box inside a (0, 0, width, height) box.
 */
void wlr_fbox_transform(struct wlr_fbox *dest, const struct wlr_fbox *box,
	enum wl_output_transform transform, double width, double height);

#endif
