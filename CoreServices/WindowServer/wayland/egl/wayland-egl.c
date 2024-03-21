/*
 * Copyright © 2011 Kristian Høgsberg
 * Copyright © 2011 Benjamin Franzke
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Kristian Høgsberg <krh@bitplanet.net>
 *    Benjamin Franzke <benjaminfranzke@googlemail.com>
 */

#include <stdlib.h>
#include <string.h>

#include "wayland-egl.h"
#include "wayland-egl-backend.h"
#include "wayland-util.h"


/** Resize the EGL window
 *
 * \param egl_window A pointer to a struct wl_egl_window
 * \param width The new width
 * \param height The new height
 * \param dx Offset on the X axis
 * \param dy Offset on the Y axis
 *
 * Note that applications should prefer using the wl_surface.offset request if
 * the associated wl_surface has the interface version 5 or higher.
 *
 * If the wl_surface.offset request is used, applications MUST pass 0 to both
 * dx and dy.
 */
WL_EXPORT void
wl_egl_window_resize(struct wl_egl_window *egl_window,
		     int width, int height,
		     int dx, int dy)
{
	if (width <= 0 || height <= 0)
		return;

	egl_window->width  = width;
	egl_window->height = height;
	egl_window->dx     = dx;
	egl_window->dy     = dy;

	if (egl_window->resize_callback)
		egl_window->resize_callback(egl_window, egl_window->driver_private);
}

WL_EXPORT struct wl_egl_window *
wl_egl_window_create(struct wl_surface *surface,
		     int width, int height)
{
	struct wl_egl_window *egl_window;

	if (width <= 0 || height <= 0)
		return NULL;

	egl_window = calloc(1, sizeof *egl_window);
	if (!egl_window)
		return NULL;

	/* Cast away the constness to set the version number.
	 *
	 * We want the const notation since it gives an explicit
	 * feedback to the backend implementation, should it try to
	 * change it.
	 *
	 * The latter in itself is not too surprising as these days APIs
	 * tend to provide bidirectional version field.
	 */
	intptr_t *version = (intptr_t *)&egl_window->version;
	*version = WL_EGL_WINDOW_VERSION;

	egl_window->surface = surface;

	egl_window->width  = width;
	egl_window->height = height;

	return egl_window;
}

WL_EXPORT void
wl_egl_window_destroy(struct wl_egl_window *egl_window)
{
	if (egl_window->destroy_window_callback)
		egl_window->destroy_window_callback(egl_window->driver_private);
	free(egl_window);
}

WL_EXPORT void
wl_egl_window_get_attached_size(struct wl_egl_window *egl_window,
				int *width, int *height)
{
	if (width)
		*width = egl_window->attached_width;
	if (height)
		*height = egl_window->attached_height;
}
