/*
 * Copyright 2016 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#ifndef ST_SAMPLER_VIEW_H
#define ST_SAMPLER_VIEW_H

#include "util/compiler.h"
#include "pipe/p_context.h"
#include "pipe/p_state.h"
#include "util/u_sampler.h"

static inline struct pipe_sampler_view *
st_create_texture_sampler_view_format(struct pipe_context *pipe,
                                      struct pipe_resource *texture,
                                      enum pipe_format format)
{
   struct pipe_sampler_view templ;

   u_sampler_view_default_template(&templ, texture, format);

   return pipe->create_sampler_view(pipe, texture, &templ);
}


static inline struct pipe_sampler_view *
st_create_texture_sampler_view(struct pipe_context *pipe,
                               struct pipe_resource *texture)
{
   return st_create_texture_sampler_view_format(pipe, texture,
                                                texture->format);
}


extern void
st_texture_release_context_sampler_view(struct st_context *st,
                                        struct gl_texture_object *stObj);

extern void
st_texture_release_all_sampler_views(struct st_context *st,
                                     struct gl_texture_object *stObj);

void
st_delete_texture_sampler_views(struct st_context *st,
                                struct gl_texture_object *stObj);

struct st_sampler_view *
st_texture_get_current_sampler_view(const struct st_context *st,
                                    const struct gl_texture_object *stObj);

struct pipe_sampler_view *
st_get_texture_sampler_view_from_stobj(struct st_context *st,
                                       struct gl_texture_object *stObj,
                                       const struct gl_sampler_object *samp,
                                       bool glsl130_or_later,
                                       bool ignore_srgb_decode,
                                       bool get_reference);

struct pipe_sampler_view *
st_get_buffer_sampler_view_from_stobj(struct st_context *st,
                                      struct gl_texture_object *stObj,
                                      bool get_reference);

enum pipe_format
st_get_sampler_view_format(const struct st_context *st,
                           const struct gl_texture_object *texObj,
                           bool srgb_skip_decode);

#endif /* ST_SAMPLER_VIEW_H */
