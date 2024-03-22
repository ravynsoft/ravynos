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

#ifndef U_DRAW_H
#define U_DRAW_H


#include "util/compiler.h"
#include "pipe/p_context.h"
#include "pipe/p_state.h"


#ifdef __cplusplus
extern "C" {
#endif


static inline void
util_draw_init_info(struct pipe_draw_info *info)
{
   memset(info, 0, sizeof(*info));
   info->instance_count = 1;
   info->max_index = 0xffffffff;
}


static inline void
util_draw_arrays(struct pipe_context *pipe,
                 enum mesa_prim mode,
                 unsigned start,
                 unsigned count)
{
   struct pipe_draw_info info;
   struct pipe_draw_start_count_bias draw;

   util_draw_init_info(&info);
   info.mode = mode;
   info.min_index = start;
   info.max_index = start + count - 1;

   draw.start = start;
   draw.count = count;
   draw.index_bias = 0;

   pipe->draw_vbo(pipe, &info, 0, NULL, &draw, 1);
}

static inline void
util_draw_elements(struct pipe_context *pipe,
                   void *indices,
                   unsigned index_size,
                   int index_bias, enum mesa_prim mode,
                   unsigned start,
                   unsigned count)
{
   struct pipe_draw_info info;
   struct pipe_draw_start_count_bias draw;

   util_draw_init_info(&info);
   info.index.user = indices;
   info.has_user_indices = true;
   info.index_size = index_size;
   info.mode = mode;
   draw.index_bias = index_bias;

   draw.start = start;
   draw.count = count;

   pipe->draw_vbo(pipe, &info, 0, NULL, &draw, 1);
}

static inline void
util_draw_arrays_instanced(struct pipe_context *pipe,
                           enum mesa_prim mode,
                           unsigned start,
                           unsigned count,
                           unsigned start_instance,
                           unsigned instance_count)
{
   struct pipe_draw_info info;
   struct pipe_draw_start_count_bias draw;

   util_draw_init_info(&info);
   info.mode = mode;
   info.start_instance = start_instance;
   info.instance_count = instance_count;
   info.index_bounds_valid = true;
   info.min_index = start;
   info.max_index = start + count - 1;

   draw.start = start;
   draw.count = count;
   draw.index_bias = 0;

   pipe->draw_vbo(pipe, &info, 0, NULL, &draw, 1);
}

static inline void
util_draw_elements_instanced(struct pipe_context *pipe,
                             void *indices,
                             unsigned index_size,
                             int index_bias,
                             enum mesa_prim mode,
                             unsigned start,
                             unsigned count,
                             unsigned start_instance,
                             unsigned instance_count)
{
   struct pipe_draw_info info;
   struct pipe_draw_start_count_bias draw;

   util_draw_init_info(&info);
   info.index.user = indices;
   info.has_user_indices = true;
   info.index_size = index_size;
   info.mode = mode;
   draw.index_bias = index_bias;
   info.start_instance = start_instance;
   info.instance_count = instance_count;

   draw.start = start;
   draw.count = count;

   pipe->draw_vbo(pipe, &info, 0, NULL, &draw, 1);
}

struct u_indirect_params {
   struct pipe_draw_info info;
   struct pipe_draw_start_count_bias draw;
};

/* caller must free the return value */
struct u_indirect_params *
util_draw_indirect_read(struct pipe_context *pipe,
                        const struct pipe_draw_info *info_in,
                        const struct pipe_draw_indirect_info *indirect,
                        unsigned *num_draws);

/* This converts an indirect draw into a direct draw by mapping the indirect
 * buffer, extracting its arguments, and calling pipe->draw_vbo.
 */
void
util_draw_indirect(struct pipe_context *pipe,
                   const struct pipe_draw_info *info,
                   const struct pipe_draw_indirect_info *indirect);

/* Helper to handle multi-draw by splitting into individual draws.  You
 * don't want to call this if num_draws==1
 */
void
util_draw_multi(struct pipe_context *pctx, const struct pipe_draw_info *info,
                unsigned drawid_offset,
                const struct pipe_draw_indirect_info *indirect,
                const struct pipe_draw_start_count_bias *draws,
                unsigned num_draws);

unsigned
util_draw_max_index(
      const struct pipe_vertex_buffer *vertex_buffers,
      const struct pipe_vertex_element *vertex_elements,
      unsigned nr_vertex_elements,
      const struct pipe_draw_info *info);


#ifdef __cplusplus
}
#endif

#endif /* !U_DRAW_H */
