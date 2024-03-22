/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 2010 LunarG Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Chia-I Wu <olv@lunarg.com>
 */

#ifndef _XM_ST_H_
#define _XM_ST_H_

#include "util/compiler.h"
#include "frontend/api.h"

#include "xm_api.h"

struct pipe_frontend_drawable *
xmesa_create_st_framebuffer(XMesaDisplay xmdpy, XMesaBuffer b);

void
xmesa_destroy_st_framebuffer(struct pipe_frontend_drawable *drawable);

struct pipe_resource *
xmesa_get_framebuffer_resource(struct pipe_frontend_drawable *drawable,
                               enum st_attachment_type att);

void
xmesa_swap_st_framebuffer(struct pipe_frontend_drawable *drawable);

void
xmesa_copy_st_framebuffer(struct pipe_frontend_drawable *drawable,
                          enum st_attachment_type src,
                          enum st_attachment_type dst,
                          int x, int y, int w, int h);

struct pipe_resource*
xmesa_get_attachment(struct pipe_frontend_drawable *drawable,
                     enum st_attachment_type st_attachment);

struct pipe_context*
xmesa_get_context(struct pipe_frontend_drawable* drawable);

bool
xmesa_st_framebuffer_validate_textures(struct pipe_frontend_drawable *drawable,
                                       unsigned width, unsigned height,
                                       unsigned mask);
#endif /* _XM_ST_H_ */
