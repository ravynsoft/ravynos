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

#include "config.h"
#include "xcursor.h"
#include "wayland-cursor.h"
#include "wayland-client.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>

#include "os-compatibility.h"

#define ARRAY_LENGTH(a) (sizeof (a) / sizeof (a)[0])

struct shm_pool {
	struct wl_shm_pool *pool;
	int fd;
	unsigned int size;
	unsigned int used;
	char *data;
};

static struct shm_pool *
shm_pool_create(struct wl_shm *shm, int size)
{
	struct shm_pool *pool;

	pool = malloc(sizeof *pool);
	if (!pool)
		return NULL;

	pool->fd = os_create_anonymous_file(size);
	if (pool->fd < 0)
		goto err_free;

	pool->data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED,
			  pool->fd, 0);

	if (pool->data == MAP_FAILED)
		goto err_close;

	pool->pool = wl_shm_create_pool(shm, pool->fd, size);
	pool->size = size;
	pool->used = 0;

	return pool;

err_close:
	close(pool->fd);
err_free:
	free(pool);
	return NULL;
}

static int
shm_pool_resize(struct shm_pool *pool, int size)
{
	if (os_resize_anonymous_file(pool->fd, size) < 0)
		return 0;

	wl_shm_pool_resize(pool->pool, size);

	munmap(pool->data, pool->size);

	pool->data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED,
			  pool->fd, 0);
	if (pool->data == MAP_FAILED)
		return 0;
	pool->size = size;

	return 1;
}

static int
shm_pool_allocate(struct shm_pool *pool, int size)
{
	int offset;

	if (pool->used + size > pool->size)
		if (!shm_pool_resize(pool, 2 * pool->size + size))
			return -1;

	offset = pool->used;
	pool->used += size;

	return offset;
}

static void
shm_pool_destroy(struct shm_pool *pool)
{
	munmap(pool->data, pool->size);
	wl_shm_pool_destroy(pool->pool);
	close(pool->fd);
	free(pool);
}


struct wl_cursor_theme {
	unsigned int cursor_count;
	struct wl_cursor **cursors;
	struct wl_shm *shm;
	struct shm_pool *pool;
	int size;
};

struct cursor_image {
	struct wl_cursor_image image;
	struct wl_cursor_theme *theme;
	struct wl_buffer *buffer;
	int offset; /* data offset of this image in the shm pool */
};

struct cursor {
	struct wl_cursor cursor;
	uint32_t total_delay; /* length of the animation in ms */
};

/** Get an shm buffer for a cursor image
 *
 * \param image The cursor image
 * \return An shm buffer for the cursor image. The user should not destroy
 * the returned buffer.
 */
WL_EXPORT struct wl_buffer *
wl_cursor_image_get_buffer(struct wl_cursor_image *image)
{
	struct cursor_image *img = (struct cursor_image *) image;
	struct wl_cursor_theme *theme = img->theme;

	if (!img->buffer) {
		img->buffer =
			wl_shm_pool_create_buffer(theme->pool->pool,
						  img->offset,
						  image->width, image->height,
						  image->width * 4,
						  WL_SHM_FORMAT_ARGB8888);
	};

	return img->buffer;
}

static void
wl_cursor_image_destroy(struct wl_cursor_image *image)
{
	struct cursor_image *img = (struct cursor_image *) image;

	if (img->buffer)
		wl_buffer_destroy(img->buffer);

	free(img);
}

static void
wl_cursor_destroy(struct wl_cursor *cursor)
{
	unsigned int i;

	for (i = 0; i < cursor->image_count; i++)
		wl_cursor_image_destroy(cursor->images[i]);

	free(cursor->images);
	free(cursor->name);
	free(cursor);
}

#include "cursor-data.h"

static struct wl_cursor *
wl_cursor_create_from_data(struct cursor_metadata *metadata,
			   struct wl_cursor_theme *theme)
{
	struct cursor *cursor;
	struct cursor_image *image;
	int size;

	cursor = malloc(sizeof *cursor);
	if (!cursor)
		return NULL;

	cursor->cursor.image_count = 1;
	cursor->cursor.images = malloc(sizeof *cursor->cursor.images);
	if (!cursor->cursor.images)
		goto err_free_cursor;

	cursor->cursor.name = strdup(metadata->name);
	cursor->total_delay = 0;

	image = malloc(sizeof *image);
	if (!image)
		goto err_free_images;

	cursor->cursor.images[0] = (struct wl_cursor_image *) image;
	image->theme = theme;
	image->buffer = NULL;
	image->image.width = metadata->width;
	image->image.height = metadata->height;
	image->image.hotspot_x = metadata->hotspot_x;
	image->image.hotspot_y = metadata->hotspot_y;
	image->image.delay = 0;

	size = metadata->width * metadata->height * sizeof(uint32_t);
	image->offset = shm_pool_allocate(theme->pool, size);

	if (image->offset < 0)
		goto err_free_image;

	memcpy(theme->pool->data + image->offset,
	       cursor_data + metadata->offset, size);

	return &cursor->cursor;

err_free_image:
	free(image);

err_free_images:
	free(cursor->cursor.name);
	free(cursor->cursor.images);

err_free_cursor:
	free(cursor);
	return NULL;
}

static void
load_fallback_theme(struct wl_cursor_theme *theme)
{
	uint32_t i;

	theme->cursor_count = ARRAY_LENGTH(cursor_metadata);
	theme->cursors = malloc(theme->cursor_count * sizeof(*theme->cursors));

	if (theme->cursors == NULL) {
		theme->cursor_count = 0;
		return;
	}

	for (i = 0; i < theme->cursor_count; ++i) {
		theme->cursors[i] =
			wl_cursor_create_from_data(&cursor_metadata[i], theme);

		if (theme->cursors[i] == NULL)
			break;
	}
	theme->cursor_count = i;
}

static struct wl_cursor *
wl_cursor_create_from_xcursor_images(struct xcursor_images *images,
				     struct wl_cursor_theme *theme)
{
	struct cursor *cursor;
	struct cursor_image *image;
	int i, size;

	cursor = malloc(sizeof *cursor);
	if (!cursor)
		return NULL;

	cursor->cursor.images =
		malloc(images->nimage * sizeof cursor->cursor.images[0]);
	if (!cursor->cursor.images) {
		free(cursor);
		return NULL;
	}

	cursor->cursor.name = strdup(images->name);
	cursor->total_delay = 0;

	for (i = 0; i < images->nimage; i++) {
		image = malloc(sizeof *image);
		if (image == NULL)
			break;

		image->theme = theme;
		image->buffer = NULL;

		image->image.width = images->images[i]->width;
		image->image.height = images->images[i]->height;
		image->image.hotspot_x = images->images[i]->xhot;
		image->image.hotspot_y = images->images[i]->yhot;
		image->image.delay = images->images[i]->delay;

		size = image->image.width * image->image.height * 4;
		image->offset = shm_pool_allocate(theme->pool, size);
		if (image->offset < 0) {
			free(image);
			break;
		}

		/* copy pixels to shm pool */
		memcpy(theme->pool->data + image->offset,
		       images->images[i]->pixels, size);
		cursor->total_delay += image->image.delay;
		cursor->cursor.images[i] = (struct wl_cursor_image *) image;
	}
	cursor->cursor.image_count = i;

	if (cursor->cursor.image_count == 0) {
		free(cursor->cursor.name);
		free(cursor->cursor.images);
		free(cursor);
		return NULL;
	}

	return &cursor->cursor;
}

static void
load_callback(struct xcursor_images *images, void *data)
{
	struct wl_cursor_theme *theme = data;
	struct wl_cursor *cursor;

	if (wl_cursor_theme_get_cursor(theme, images->name)) {
		xcursor_images_destroy(images);
		return;
	}

	cursor = wl_cursor_create_from_xcursor_images(images, theme);

	if (cursor) {
		theme->cursor_count++;
		theme->cursors =
			realloc(theme->cursors,
				theme->cursor_count * sizeof theme->cursors[0]);

		if (theme->cursors == NULL) {
			theme->cursor_count--;
			free(cursor);
		} else {
			theme->cursors[theme->cursor_count - 1] = cursor;
		}
	}

	xcursor_images_destroy(images);
}

/** Load a cursor theme to memory shared with the compositor
 *
 * \param name The name of the cursor theme to load. If %NULL, the default
 * theme will be loaded.
 * \param size Desired size of the cursor images.
 * \param shm The compositor's shm interface.
 *
 * \return An object representing the theme that should be destroyed with
 * wl_cursor_theme_destroy() or %NULL on error. If no theme with the given
 * name exists, a default theme will be loaded.
 */
WL_EXPORT struct wl_cursor_theme *
wl_cursor_theme_load(const char *name, int size, struct wl_shm *shm)
{
	struct wl_cursor_theme *theme;

	theme = malloc(sizeof *theme);
	if (!theme)
		return NULL;

	if (!name)
		name = "default";

	theme->size = size;
	theme->cursor_count = 0;
	theme->cursors = NULL;

	theme->pool = shm_pool_create(shm, size * size * 4);
	if (!theme->pool)
		goto out_error_pool;

	xcursor_load_theme(name, size, load_callback, theme);

	if (theme->cursor_count == 0)
		xcursor_load_theme(NULL, size, load_callback, theme);

	if (theme->cursor_count == 0)
		load_fallback_theme(theme);

	return theme;

out_error_pool:
	free(theme);
	return NULL;
}

/** Destroys a cursor theme object
 *
 * \param theme The cursor theme to be destroyed
 */
WL_EXPORT void
wl_cursor_theme_destroy(struct wl_cursor_theme *theme)
{
	unsigned int i;

	for (i = 0; i < theme->cursor_count; i++)
		wl_cursor_destroy(theme->cursors[i]);

	shm_pool_destroy(theme->pool);

	free(theme->cursors);
	free(theme);
}

/** Get the cursor for a given name from a cursor theme
 *
 * \param theme The cursor theme
 * \param name Name of the desired cursor
 * \return The theme's cursor of the given name or %NULL if there is no
 * such cursor
 */
WL_EXPORT struct wl_cursor *
wl_cursor_theme_get_cursor(struct wl_cursor_theme *theme,
			   const char *name)
{
	unsigned int i;

	for (i = 0; i < theme->cursor_count; i++) {
		if (strcmp(name, theme->cursors[i]->name) == 0)
			return theme->cursors[i];
	}

	return NULL;
}

/** Find the frame for a given elapsed time in a cursor animation
 *  as well as the time left until next cursor change.
 *
 * \param cursor The cursor
 * \param time Elapsed time in ms since the beginning of the animation
 * \param duration pointer to uint32_t to store time left for this image or
 *                 zero if the cursor won't change.
 *
 * \return The index of the image that should be displayed for the
 * given time in the cursor animation.
 */
WL_EXPORT int
wl_cursor_frame_and_duration(struct wl_cursor *cursor, uint32_t time,
			     uint32_t *duration)
{
	struct cursor *cur = (struct cursor *) cursor;
	uint32_t t;
	int i;

	if (cur->cursor.image_count == 1 || cur->total_delay == 0) {
		if (duration)
			*duration = 0;
		return 0;
	}

	i = 0;
	t = time % cur->total_delay;

	/* If there is a 0 delay in the image set then this
	 * loop breaks on it and we display that cursor until
	 * time % cursor->total_delay wraps again.
	 * Since a 0 delay is silly, and we've never actually
	 * seen one in a cursor file, we haven't bothered to
	 * "fix" this.
	 */
	while (t - cur->cursor.images[i]->delay < t)
		t -= cur->cursor.images[i++]->delay;

	if (!duration)
		return i;

	/* Make sure we don't accidentally tell the caller this is
	 * a static cursor image.
	 */
	if (t >= cur->cursor.images[i]->delay)
		*duration = 1;
	else
		*duration = cur->cursor.images[i]->delay - t;

	return i;
}

/** Find the frame for a given elapsed time in a cursor animation
 *
 * \param cursor The cursor
 * \param time Elapsed time in ms since the beginning of the animation
 *
 * \return The index of the image that should be displayed for the
 * given time in the cursor animation.
 */
WL_EXPORT int
wl_cursor_frame(struct wl_cursor *cursor, uint32_t time)
{
	return wl_cursor_frame_and_duration(cursor, time, NULL);
}
