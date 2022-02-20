/*
 * Copyright © 2012 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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

#ifndef WLR_XCURSOR_H
#define WLR_XCURSOR_H

#include <stdint.h>
#include <wlr/util/edges.h>

/**
 * A still cursor image.
 *
 * The buffer contains pixels layed out in a packed DRM_FORMAT_ARGB8888 format.
 */
struct wlr_xcursor_image {
	uint32_t width; /* actual width */
	uint32_t height; /* actual height */
	uint32_t hotspot_x; /* hot-spot x (must be inside image) */
	uint32_t hotspot_y; /* hot-spot y (must be inside image) */
	uint32_t delay; /* animation delay to next frame (ms) */
	uint8_t *buffer; /* pixel data */
};

/**
 * A cursor.
 *
 * If the cursor is animated, it may contain more than a single image.
 */
struct wlr_xcursor {
	unsigned int image_count;
	struct wlr_xcursor_image **images;
	char *name;
	uint32_t total_delay; /* total duration of the animation in ms */
};

/**
 * Container for an Xcursor theme.
 */
struct wlr_xcursor_theme {
	unsigned int cursor_count;
	struct wlr_xcursor **cursors;
	char *name;
	int size;
};

/**
 * Loads the named Xcursor theme.
 *
 * This is useful if you need cursor images for your compositor to use when a
 * client-side cursor is not available or you wish to override client-side
 * cursors for a particular UI interaction (such as using a grab cursor when
 * moving a window around).
 *
 * The size is given in pixels.
 *
 * If a cursor theme with the given name couldn't be loaded, a fallback theme
 * is loaded.
 *
 * On error, NULL is returned.
 */
struct wlr_xcursor_theme *wlr_xcursor_theme_load(const char *name, int size);

/**
 * Destroy a cursor theme.
 *
 * This implicitly destroys all child cursors and cursor images.
 */
void wlr_xcursor_theme_destroy(struct wlr_xcursor_theme *theme);

/**
 * Obtain a cursor for the specified name (e.g. "left_ptr").
 *
 * If the cursor could not be found, NULL is returned.
 */
struct wlr_xcursor *wlr_xcursor_theme_get_cursor(
	struct wlr_xcursor_theme *theme, const char *name);

/**
 * Find the frame for a given elapsed time in a cursor animation.
 *
 * This function converts a timestamp (in ms) to a cursor image index.
 */
int wlr_xcursor_frame(struct wlr_xcursor *cursor, uint32_t time);

/**
 * Get the name of the resize cursor for the given edges.
 */
const char *wlr_xcursor_get_resize_name(enum wlr_edges edges);

#endif
