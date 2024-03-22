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

#ifndef ST_MANAGER_H
#define ST_MANAGER_H

#include "main/menums.h"
#include "main/fbobject.h"

#include "util/compiler.h"


struct st_context;
struct gl_renderbuffer;
struct pipe_surface;

void
st_manager_flush_frontbuffer(struct st_context *st);

void
st_manager_validate_framebuffers(struct st_context *st);

bool
st_manager_add_color_renderbuffer(struct gl_context *ctx, struct gl_framebuffer *fb,
                                  gl_buffer_index idx);

void
st_manager_flush_swapbuffers(void);

void
st_set_ws_renderbuffer_surface(struct gl_renderbuffer *rb,
                               struct pipe_surface *surf);

void st_manager_invalidate_drawables(struct gl_context *ctx);
#endif /* ST_MANAGER_H */
