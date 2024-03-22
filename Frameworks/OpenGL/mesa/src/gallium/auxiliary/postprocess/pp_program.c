/**************************************************************************
 *
 * Copyright 2010 Jakob Bornecrantz
 * Copyright 2011 Lauri Kasanen
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
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#include "postprocess/postprocess.h"
#include "postprocess/pp_private.h"

#include "cso_cache/cso_context.h"
#include "pipe/p_screen.h"
#include "pipe/p_context.h"
#include "pipe/p_state.h"
#include "pipe/p_shader_tokens.h"
#include "util/u_inlines.h"
#include "util/u_simple_shaders.h"
#include "util/u_memory.h"

/** Initialize the internal details */
struct pp_program *
pp_init_prog(struct pp_queue_t *ppq, struct pipe_context *pipe,
             struct cso_context *cso, struct st_context *st,
             pp_st_invalidate_state_func st_invalidate_state)
{
   struct pp_program *p;

   pp_debug("Initializing program\n");
   if (!pipe)
      return NULL;

   p = CALLOC(1, sizeof(struct pp_program));
   if (!p)
      return NULL;

   p->screen = pipe->screen;
   p->pipe = pipe;
   p->cso = cso;
   p->st = st;
   p->st_invalidate_state = st_invalidate_state;

   {
      static const float verts[4][2][4] = {
         {
          {1.0f, 1.0f, 0.0f, 1.0f},
          {1.0f, 1.0f, 0.0f, 1.0f}
          },
         {
          {-1.0f, 1.0f, 0.0f, 1.0f},
          {0.0f, 1.0f, 0.0f, 1.0f}
          },
         {
          {-1.0f, -1.0f, 0.0f, 1.0f},
          {0.0f, 0.0f, 0.0f, 1.0f}
          },
         {
          {1.0f, -1.0f, 0.0f, 1.0f},
          {1.0f, 0.0f, 0.0f, 1.0f}
          }
      };

      p->vbuf = pipe_buffer_create(pipe->screen, PIPE_BIND_VERTEX_BUFFER,
                                   PIPE_USAGE_DEFAULT, sizeof(verts));
      pipe_buffer_write(p->pipe, p->vbuf, 0, sizeof(verts), verts);
   }

   p->blend.rt[0].colormask = PIPE_MASK_RGBA;
   p->blend.rt[0].rgb_src_factor = p->blend.rt[0].alpha_src_factor =
      PIPE_BLENDFACTOR_SRC_ALPHA;
   p->blend.rt[0].rgb_dst_factor = p->blend.rt[0].alpha_dst_factor =
      PIPE_BLENDFACTOR_INV_SRC_ALPHA;

   p->rasterizer.cull_face = PIPE_FACE_NONE;
   p->rasterizer.half_pixel_center = 1;
   p->rasterizer.bottom_edge_rule = 1;
   p->rasterizer.depth_clip_near = 1;
   p->rasterizer.depth_clip_far = 1;

   p->sampler.wrap_s = p->sampler.wrap_t = p->sampler.wrap_r =
      PIPE_TEX_WRAP_CLAMP_TO_EDGE;

   p->sampler.min_mip_filter = PIPE_TEX_MIPFILTER_NONE;
   p->sampler.min_img_filter = p->sampler.mag_img_filter =
      PIPE_TEX_FILTER_LINEAR;

   p->sampler_point.wrap_s = p->sampler_point.wrap_t =
      p->sampler_point.wrap_r = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
   p->sampler_point.min_mip_filter = PIPE_TEX_MIPFILTER_NONE;
   p->sampler_point.min_img_filter = p->sampler_point.mag_img_filter =
      PIPE_TEX_FILTER_NEAREST;

   p->velem.count = 2;
   p->velem.velems[0].src_offset = 0;
   p->velem.velems[0].instance_divisor = 0;
   p->velem.velems[0].vertex_buffer_index = 0;
   p->velem.velems[0].src_format = PIPE_FORMAT_R32G32B32A32_FLOAT;
   p->velem.velems[0].src_stride = 2 * 4 * sizeof(float);
   p->velem.velems[1].src_offset = 1 * 4 * sizeof(float);
   p->velem.velems[1].instance_divisor = 0;
   p->velem.velems[1].vertex_buffer_index = 0;
   p->velem.velems[1].src_format = PIPE_FORMAT_R32G32B32A32_FLOAT;
   p->velem.velems[1].src_stride = 2 * 4 * sizeof(float);

   if (!p->screen->is_format_supported(p->screen,
                                       PIPE_FORMAT_R32G32B32A32_FLOAT,
                                       PIPE_BUFFER, 1, 1,
                                       PIPE_BIND_VERTEX_BUFFER))
      pp_debug("Vertex buf format fail\n");


   {
      const enum tgsi_semantic semantic_names[] = { TGSI_SEMANTIC_POSITION,
         TGSI_SEMANTIC_GENERIC
      };
      const unsigned semantic_indexes[] = { 0, 0 };
      p->passvs = util_make_vertex_passthrough_shader(p->pipe, 2,
                                                      semantic_names,
                                                      semantic_indexes, false);
   }

   p->framebuffer.nr_cbufs = 1;

   p->surf.format = PIPE_FORMAT_B8G8R8A8_UNORM;

   return p;
}
