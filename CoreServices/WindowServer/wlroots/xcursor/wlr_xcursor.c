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

#define _POSIX_C_SOURCE 200809L
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wlr/util/log.h>
#include <wlr/xcursor.h>
#include "xcursor/xcursor.h"

static void xcursor_destroy(struct wlr_xcursor *cursor) {
	for (size_t i = 0; i < cursor->image_count; i++) {
		free(cursor->images[i]->buffer);
		free(cursor->images[i]);
	}

	free(cursor->images);
	free(cursor->name);
	free(cursor);
}

#include "xcursor/cursor_data.h"

static struct wlr_xcursor *xcursor_create_from_data(
		const struct cursor_metadata *metadata, struct wlr_xcursor_theme *theme) {
	struct wlr_xcursor *cursor;
	struct wlr_xcursor_image *image;
	int size;

	cursor = malloc(sizeof(*cursor));
	if (!cursor) {
		return NULL;
	}

	cursor->image_count = 1;
	cursor->images = malloc(sizeof(*cursor->images));
	if (!cursor->images) {
		goto err_free_cursor;
	}

	cursor->name = strdup(metadata->name);
	cursor->total_delay = 0;

	image = malloc(sizeof(*image));
	if (!image) {
		goto err_free_images;
	}

	cursor->images[0] = image;
	image->buffer = NULL;
	image->width = metadata->width;
	image->height = metadata->height;
	image->hotspot_x = metadata->hotspot_x;
	image->hotspot_y = metadata->hotspot_y;
	image->delay = 0;

	size = metadata->width * metadata->height * sizeof(uint32_t);
	image->buffer = malloc(size);

	if (!image->buffer) {
		goto err_free_image;
	}

	memcpy(image->buffer, cursor_data + metadata->offset, size);

	return cursor;

err_free_image:
	free(image);

err_free_images:
	free(cursor->name);
	free(cursor->images);

err_free_cursor:
	free(cursor);
	return NULL;
}

static void load_default_theme(struct wlr_xcursor_theme *theme) {
	uint32_t i;

	free(theme->name);
	theme->name = strdup("default");

	theme->cursor_count = sizeof(cursor_metadata) / sizeof(cursor_metadata[0]);
	theme->cursors = malloc(theme->cursor_count * sizeof(*theme->cursors));

	if (theme->cursors == NULL) {
		theme->cursor_count = 0;
		return;
	}

	for (i = 0; i < theme->cursor_count; ++i) {
		theme->cursors[i] =
			xcursor_create_from_data(&cursor_metadata[i], theme);

		if (theme->cursors[i] == NULL) {
			break;
		}
	}
	theme->cursor_count = i;
}

static struct wlr_xcursor *xcursor_create_from_xcursor_images(
		XcursorImages *images, struct wlr_xcursor_theme *theme) {
	struct wlr_xcursor *cursor;
	struct wlr_xcursor_image *image;
	int i, size;

	cursor = malloc(sizeof(*cursor));
	if (!cursor) {
		return NULL;
	}

	cursor->images = malloc(images->nimage * sizeof(cursor->images[0]));
	if (!cursor->images) {
		free(cursor);
		return NULL;
	}

	cursor->name = strdup(images->name);
	cursor->total_delay = 0;

	for (i = 0; i < images->nimage; i++) {
		image = malloc(sizeof(*image));
		if (image == NULL) {
			break;
		}

		image->buffer = NULL;

		image->width = images->images[i]->width;
		image->height = images->images[i]->height;
		image->hotspot_x = images->images[i]->xhot;
		image->hotspot_y = images->images[i]->yhot;
		image->delay = images->images[i]->delay;

		size = image->width * image->height * 4;
		image->buffer = malloc(size);
		if (!image->buffer) {
			free(image);
			break;
		}

		/* copy pixels to shm pool */
		memcpy(image->buffer, images->images[i]->pixels, size);
		cursor->total_delay += image->delay;
		cursor->images[i] = image;
	}
	cursor->image_count = i;

	if (cursor->image_count == 0) {
		free(cursor->name);
		free(cursor->images);
		free(cursor);
		return NULL;
	}

	return cursor;
}

static void load_callback(XcursorImages *images, void *data) {
	struct wlr_xcursor_theme *theme = data;
	struct wlr_xcursor *cursor;

	if (wlr_xcursor_theme_get_cursor(theme, images->name)) {
		XcursorImagesDestroy(images);
		return;
	}

	cursor = xcursor_create_from_xcursor_images(images, theme);

	if (cursor) {
		struct wlr_xcursor **cursors;
		theme->cursor_count++;
		cursors =
			realloc(theme->cursors,
				theme->cursor_count * sizeof(theme->cursors[0]));

		if (cursors == NULL) {
			theme->cursor_count--;
			free(cursor);
		} else {
			theme->cursors = cursors;
			theme->cursors[theme->cursor_count - 1] = cursor;
		}
	}

	XcursorImagesDestroy(images);
}

struct wlr_xcursor_theme *wlr_xcursor_theme_load(const char *name, int size) {
	struct wlr_xcursor_theme *theme;

	theme = malloc(sizeof(*theme));
	if (!theme) {
		return NULL;
	}

	if (!name) {
		name = "default";
	}

	theme->name = strdup(name);
	if (!theme->name) {
		goto out_error_name;
	}
	theme->size = size;
	theme->cursor_count = 0;
	theme->cursors = NULL;

	xcursor_load_theme(name, size, load_callback, theme);

	if (theme->cursor_count == 0) {
		load_default_theme(theme);
	}

	wlr_log(WLR_DEBUG, "Loaded cursor theme '%s' at size %d (%d available cursors)",
			theme->name, size, theme->cursor_count);

	return theme;

out_error_name:
	free(theme);
	return NULL;
}

void wlr_xcursor_theme_destroy(struct wlr_xcursor_theme *theme) {
	unsigned int i;

	for (i = 0; i < theme->cursor_count; i++) {
		xcursor_destroy(theme->cursors[i]);
	}

	free(theme->name);
	free(theme->cursors);
	free(theme);
}

struct wlr_xcursor *wlr_xcursor_theme_get_cursor(struct wlr_xcursor_theme *theme,
		const char *name) {
	unsigned int i;

	for (i = 0; i < theme->cursor_count; i++) {
		if (strcmp(name, theme->cursors[i]->name) == 0) {
			return theme->cursors[i];
		}
	}

	return NULL;
}

static int xcursor_frame_and_duration(struct wlr_xcursor *cursor,
		uint32_t time, uint32_t *duration) {
	uint32_t t;
	int i;

	if (cursor->image_count == 1) {
		if (duration) {
			*duration = 0;
		}
		return 0;
	}

	i = 0;
	t = time % cursor->total_delay;

	/* If there is a 0 delay in the image set then this
	 * loop breaks on it and we display that cursor until
	 * time % cursor->total_delay wraps again.
	 * Since a 0 delay is silly, and we've never actually
	 * seen one in a cursor file, we haven't bothered to
	 * "fix" this.
	 */
	while (t - cursor->images[i]->delay < t) {
		t -= cursor->images[i++]->delay;
	}

	if (!duration) {
		return i;
	}

	/* Make sure we don't accidentally tell the caller this is
	 * a static cursor image.
	 */
	if (t >= cursor->images[i]->delay) {
		*duration = 1;
	} else {
		*duration = cursor->images[i]->delay - t;
	}

	return i;
}

int wlr_xcursor_frame(struct wlr_xcursor *_cursor, uint32_t time) {
	return xcursor_frame_and_duration(_cursor, time, NULL);
}

const char *wlr_xcursor_get_resize_name(enum wlr_edges edges) {
	if (edges & WLR_EDGE_TOP) {
		if (edges & WLR_EDGE_RIGHT) {
			return "ne-resize";
		} else if (edges & WLR_EDGE_LEFT) {
			return "nw-resize";
		}
		return "n-resize";
	} else if (edges & WLR_EDGE_BOTTOM) {
		if (edges & WLR_EDGE_RIGHT) {
			return "se-resize";
		} else if (edges & WLR_EDGE_LEFT) {
			return "sw-resize";
		}
		return "s-resize";
	} else if (edges & WLR_EDGE_RIGHT) {
		return "e-resize";
	} else if (edges & WLR_EDGE_LEFT) {
		return "w-resize";
	}
	return "se-resize"; // fallback
}
