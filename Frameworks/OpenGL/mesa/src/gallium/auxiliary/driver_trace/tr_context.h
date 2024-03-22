/**************************************************************************
 *
 * Copyright 2008 VMware, Inc.
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
 *
 **************************************************************************/

#ifndef TR_CONTEXT_H_
#define TR_CONTEXT_H_


#include "util/compiler.h"
#include "util/u_debug.h"
#include "util/hash_table.h"
#include "pipe/p_context.h"
#include "pipe/p_video_codec.h"
#include "util/u_threaded_context.h"

#include "tr_screen.h"

#ifdef __cplusplus
extern "C" {
#endif


struct trace_screen;
   
struct trace_context
{
   struct pipe_context base;

   struct hash_table blend_states;
   struct hash_table rasterizer_states;
   struct hash_table depth_stencil_alpha_states;

   struct pipe_context *pipe;
   tc_replace_buffer_storage_func replace_buffer_storage;
   tc_create_fence_func create_fence;

   struct pipe_framebuffer_state unwrapped_state;
   bool seen_fb_state;

   bool threaded;
};


void
trace_context_check(const struct pipe_context *pipe);
struct pipe_context *
trace_get_possibly_threaded_context(struct pipe_context *pipe);

static inline struct trace_context *
trace_context(struct pipe_context *pipe)
{
   assert(pipe);
#ifdef DEBUG
   trace_context_check(pipe);
#endif
   return (struct trace_context *)pipe;
}


struct pipe_context *
trace_context_create(struct trace_screen *tr_scr,
                     struct pipe_context *pipe);

struct pipe_context *
trace_context_create_threaded(struct pipe_screen *screen, struct pipe_context *pipe,
                              tc_replace_buffer_storage_func *replace_buffer,
                              struct threaded_context_options *options);
#ifdef __cplusplus
}
#endif

#endif /* TR_CONTEXT_H_ */
