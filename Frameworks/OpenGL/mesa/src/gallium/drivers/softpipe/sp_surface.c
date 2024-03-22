/**************************************************************************
 * 
 * Copyright 2007 VMware, Inc.
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

#include "util/format/u_format.h"
#include "util/u_surface.h"
#include "sp_context.h"
#include "sp_surface.h"
#include "sp_query.h"

static void sp_blit(struct pipe_context *pipe,
                    const struct pipe_blit_info *info)
{
   struct softpipe_context *sp = softpipe_context(pipe);

   if (info->render_condition_enable && !softpipe_check_render_cond(sp))
      return;

   if (info->src.resource->nr_samples > 1 &&
       info->dst.resource->nr_samples <= 1 &&
       !util_format_is_depth_or_stencil(info->src.resource->format) &&
       !util_format_is_pure_integer(info->src.resource->format)) {
      debug_printf("softpipe: color resolve unimplemented\n");
      return;
   }

   if (util_try_blit_via_copy_region(pipe, info, sp->render_cond_query != NULL)) {
      return; /* done */
   }

   if (!util_blitter_is_blit_supported(sp->blitter, info)) {
      debug_printf("softpipe: blit unsupported %s -> %s\n",
                   util_format_short_name(info->src.resource->format),
                   util_format_short_name(info->dst.resource->format));
      return;
   }

   /* XXX turn off occlusion and streamout queries */

   util_blitter_save_vertex_buffer_slot(sp->blitter, sp->vertex_buffer);
   util_blitter_save_vertex_elements(sp->blitter, sp->velems);
   util_blitter_save_vertex_shader(sp->blitter, sp->vs);
   util_blitter_save_geometry_shader(sp->blitter, sp->gs);
   util_blitter_save_so_targets(sp->blitter, sp->num_so_targets,
                     (struct pipe_stream_output_target**)sp->so_targets);
   util_blitter_save_rasterizer(sp->blitter, sp->rasterizer);
   util_blitter_save_viewport(sp->blitter, &sp->viewports[0]);
   util_blitter_save_scissor(sp->blitter, &sp->scissors[0]);
   util_blitter_save_fragment_shader(sp->blitter, sp->fs);
   util_blitter_save_blend(sp->blitter, sp->blend);
   util_blitter_save_depth_stencil_alpha(sp->blitter, sp->depth_stencil);
   util_blitter_save_stencil_ref(sp->blitter, &sp->stencil_ref);
   /*util_blitter_save_sample_mask(sp->blitter, sp->sample_mask);*/
   util_blitter_save_framebuffer(sp->blitter, &sp->framebuffer);
   util_blitter_save_fragment_sampler_states(sp->blitter,
                     sp->num_samplers[PIPE_SHADER_FRAGMENT],
                     (void**)sp->samplers[PIPE_SHADER_FRAGMENT]);
   util_blitter_save_fragment_sampler_views(sp->blitter,
                     sp->num_sampler_views[PIPE_SHADER_FRAGMENT],
                     sp->sampler_views[PIPE_SHADER_FRAGMENT]);
   util_blitter_save_render_condition(sp->blitter, sp->render_cond_query,
                                      sp->render_cond_cond, sp->render_cond_mode);
   util_blitter_blit(sp->blitter, info);
}

static void
sp_flush_resource(struct pipe_context *pipe,
                  struct pipe_resource *resource)
{
}

static void
softpipe_clear_render_target(struct pipe_context *pipe,
                             struct pipe_surface *dst,
                             const union pipe_color_union *color,
                             unsigned dstx, unsigned dsty,
                             unsigned width, unsigned height,
                             bool render_condition_enabled)
{
   struct softpipe_context *softpipe = softpipe_context(pipe);

   if (render_condition_enabled && !softpipe_check_render_cond(softpipe))
      return;

   util_clear_render_target(pipe, dst, color,
                            dstx, dsty, width, height);
}


static void
softpipe_clear_depth_stencil(struct pipe_context *pipe,
                             struct pipe_surface *dst,
                             unsigned clear_flags,
                             double depth,
                             unsigned stencil,
                             unsigned dstx, unsigned dsty,
                             unsigned width, unsigned height,
                             bool render_condition_enabled)
{
   struct softpipe_context *softpipe = softpipe_context(pipe);

   if (render_condition_enabled && !softpipe_check_render_cond(softpipe))
      return;

   util_clear_depth_stencil(pipe, dst, clear_flags,
                            depth, stencil,
                            dstx, dsty, width, height);
}


void
sp_init_surface_functions(struct softpipe_context *sp)
{
   sp->pipe.resource_copy_region = util_resource_copy_region;
   sp->pipe.clear_render_target = softpipe_clear_render_target;
   sp->pipe.clear_depth_stencil = softpipe_clear_depth_stencil;
   sp->pipe.blit = sp_blit;
   sp->pipe.flush_resource = sp_flush_resource;
}
