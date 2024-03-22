/*
 * Copyright © Microsoft Corporation
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#pragma once

#include <pipe/p_state.h>
#include "stw_context.h"

#include <GL/gl.h>

struct st_egl_image;

enum stw_image_error
{
   STW_IMAGE_ERROR_SUCCESS,
   STW_IMAGE_ERROR_BAD_ALLOC,
   STW_IMAGE_ERROR_BAD_PARAMETER,
   STW_IMAGE_ERROR_BAD_MATCH,
   STW_IMAGE_ERROR_BAD_ACCESS,
};

struct stw_image
{
   struct pipe_resource *pres;
   unsigned level;
   unsigned layer;
   enum pipe_format format;
};

struct stw_image *
stw_create_image_from_texture(struct stw_context *ctx, GLenum gl_target, GLuint texture,
                              GLuint depth, GLint level, enum stw_image_error *error);

struct stw_image *
stw_create_image_from_renderbuffer(struct stw_context *ctx, GLuint renderbuffer,
                                   enum stw_image_error *error);

void
stw_destroy_image(struct stw_image *img);

void
stw_translate_image(struct stw_image *in, struct st_egl_image *out);
