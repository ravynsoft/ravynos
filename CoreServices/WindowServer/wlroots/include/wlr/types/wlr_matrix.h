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

#ifndef WLR_TYPES_WLR_MATRIX_H
#define WLR_TYPES_WLR_MATRIX_H

#include <wayland-server-protocol.h>

struct wlr_box;

/** Writes the identity matrix into mat */
void wlr_matrix_identity(float mat[static 9]);

/** mat ← a × b */
void wlr_matrix_multiply(float mat[static 9], const float a[static 9],
	const float b[static 9]);

void wlr_matrix_transpose(float mat[static 9], const float a[static 9]);

/** Writes a 2D translation matrix to mat of magnitude (x, y) */
void wlr_matrix_translate(float mat[static 9], float x, float y);

/** Writes a 2D scale matrix to mat of magnitude (x, y) */
void wlr_matrix_scale(float mat[static 9], float x, float y);

/** Writes a 2D rotation matrix to mat at an angle of rad radians */
void wlr_matrix_rotate(float mat[static 9], float rad);

/** Writes a transformation matrix which applies the specified
 *  wl_output_transform to mat */
void wlr_matrix_transform(float mat[static 9],
	enum wl_output_transform transform);

/** Writes a 2D orthographic projection matrix to mat of (width, height) with a
 *  specified wl_output_transform.
 *
 *  Deprecated: this function is deprecated and will be removed in a future
 *  version of wlroots. */
void wlr_matrix_projection(float mat[static 9], int width, int height,
	enum wl_output_transform transform);

/** Shortcut for the various matrix operations involved in projecting the
 *  specified wlr_box onto a given orthographic projection with a given
 *  rotation. The result is written to mat, which can be applied to each
 *  coordinate of the box to get a new coordinate from [-1,1]. */
void wlr_matrix_project_box(float mat[static 9], const struct wlr_box *box,
	enum wl_output_transform transform, float rotation,
	const float projection[static 9]);

#endif
