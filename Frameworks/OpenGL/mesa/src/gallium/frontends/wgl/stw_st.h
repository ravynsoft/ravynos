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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Chia-I Wu <olv@lunarg.com>
 */

#ifndef STW_ST_H
#define STW_ST_H

#include <windows.h>

#include "frontend/api.h"

struct st_context;
struct stw_framebuffer;

bool
stw_own_mutex(const CRITICAL_SECTION *cs);

struct pipe_frontend_drawable *
stw_st_create_framebuffer(struct stw_framebuffer *fb, struct pipe_frontend_screen *fscreen);

void
stw_st_destroy_framebuffer_locked(struct pipe_frontend_drawable *drawable);

void
stw_st_flush(struct st_context *st, struct pipe_frontend_drawable *drawable,
             unsigned flags);

bool
stw_st_swap_framebuffer_locked(HDC hdc, struct st_context *st,
                               struct pipe_frontend_drawable *drawable);

struct pipe_resource *
stw_get_framebuffer_resource(struct pipe_frontend_drawable *drawable,
                             enum st_attachment_type att);

#endif /* STW_ST_H */
