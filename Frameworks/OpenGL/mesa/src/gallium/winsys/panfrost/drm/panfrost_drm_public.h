/*
 * Copyright © 2014 Broadcom
 * Copyright © 208 Alyssa Rosenzweig
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef __PAN_DRM_PUBLIC_H__
#define __PAN_DRM_PUBLIC_H__

#include <stdbool.h>

struct pipe_resource;
struct pipe_screen;
struct pipe_screen_config;
struct renderonly;
struct renderonly_scanout;
struct winsys_handle;

struct pipe_screen *panfrost_drm_screen_create(int drmFD);
struct pipe_screen *
panfrost_drm_screen_create_renderonly(int fd, struct renderonly *ro,
                                      const struct pipe_screen_config *config);
struct renderonly_scanout *
panfrost_create_kms_dumb_buffer_for_resource(struct pipe_resource *rsc,
                                             struct renderonly *ro,
                                             struct winsys_handle *out_handle);

#endif /* __PAN_DRM_PUBLIC_H__ */
