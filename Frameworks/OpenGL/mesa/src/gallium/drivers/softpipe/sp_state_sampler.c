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

/* Authors:
 *  Brian Paul
 */

#include "util/u_memory.h"
#include "util/u_inlines.h"
#include "util/format/u_format.h"

#include "draw/draw_context.h"

#include "sp_context.h"
#include "sp_state.h"
#include "sp_texture.h"
#include "sp_tex_sample.h"
#include "sp_tex_tile_cache.h"
#include "sp_screen.h"
#include "frontend/sw_winsys.h"


/**
 * Bind a range [start, start+num-1] of samplers for a shader stage.
 */
static void
softpipe_bind_sampler_states(struct pipe_context *pipe,
                             enum pipe_shader_type shader,
                             unsigned start,
                             unsigned num,
                             void **samplers)
{
   struct softpipe_context *softpipe = softpipe_context(pipe);
   unsigned i;

   assert(shader < PIPE_SHADER_TYPES);
   assert(start + num <= ARRAY_SIZE(softpipe->samplers[shader]));

   draw_flush(softpipe->draw);

   /* set the new samplers */
   for (i = 0; i < num; i++) {
      softpipe->samplers[shader][start + i] = samplers[i];
   }

   /* find highest non-null samplers[] entry */
   {
      unsigned j = MAX2(softpipe->num_samplers[shader], start + num);
      while (j > 0 && softpipe->samplers[shader][j - 1] == NULL)
         j--;
      softpipe->num_samplers[shader] = j;
   }

   if (shader == PIPE_SHADER_VERTEX || shader == PIPE_SHADER_GEOMETRY) {
      draw_set_samplers(softpipe->draw,
                        shader,
                        softpipe->samplers[shader],
                        softpipe->num_samplers[shader]);
   }

   softpipe->dirty |= SP_NEW_SAMPLER;
}


static void
softpipe_sampler_view_destroy(struct pipe_context *pipe,
                              struct pipe_sampler_view *view)
{
   pipe_resource_reference(&view->texture, NULL);
   FREE(view);
}


void
softpipe_set_sampler_views(struct pipe_context *pipe,
                           enum pipe_shader_type shader,
                           unsigned start,
                           unsigned num,
                           unsigned unbind_num_trailing_slots,
                           bool take_ownership,
                           struct pipe_sampler_view **views)
{
   struct softpipe_context *softpipe = softpipe_context(pipe);
   uint i;

   assert(shader < PIPE_SHADER_TYPES);
   assert(start + num <= ARRAY_SIZE(softpipe->sampler_views[shader]));

   draw_flush(softpipe->draw);

   /* set the new sampler views */
   for (i = 0; i < num; i++) {
      struct sp_sampler_view *sp_sviewsrc;
      struct sp_sampler_view *sp_sviewdst =
         &softpipe->tgsi.sampler[shader]->sp_sview[start + i];
      struct pipe_sampler_view **pview = &softpipe->sampler_views[shader][start + i];

      if (take_ownership) {
         pipe_sampler_view_reference(pview, NULL);
         *pview = views[i];
      } else {
         pipe_sampler_view_reference(pview, views[i]);
      }
      sp_tex_tile_cache_set_sampler_view(softpipe->tex_cache[shader][start + i],
                                         views[i]);
      /*
       * We don't really have variants, however some bits are different per shader,
       * so just copy?
       */
      sp_sviewsrc = (struct sp_sampler_view *)*pview;
      if (sp_sviewsrc) {
         memcpy(sp_sviewdst, sp_sviewsrc, sizeof(*sp_sviewsrc));
         sp_sviewdst->compute_lambda = softpipe_get_lambda_func(&sp_sviewdst->base, shader);
         sp_sviewdst->compute_lambda_from_grad = softpipe_get_lambda_from_grad_func(&sp_sviewdst->base, shader);
         sp_sviewdst->cache = softpipe->tex_cache[shader][start + i];
      }
      else {
         memset(sp_sviewdst, 0,  sizeof(*sp_sviewsrc));
      }
   }
   for (; i < num + unbind_num_trailing_slots; i++) {
      struct pipe_sampler_view **pview = &softpipe->sampler_views[shader][start + i];
      pipe_sampler_view_reference(pview, NULL);
      sp_tex_tile_cache_set_sampler_view(softpipe->tex_cache[shader][start + i],
                                         NULL);
   }


   /* find highest non-null sampler_views[] entry */
   {
      unsigned j = MAX2(softpipe->num_sampler_views[shader], start + num);
      while (j > 0 && softpipe->sampler_views[shader][j - 1] == NULL)
         j--;
      softpipe->num_sampler_views[shader] = j;
   }

   if (shader == PIPE_SHADER_VERTEX || shader == PIPE_SHADER_GEOMETRY) {
      draw_set_sampler_views(softpipe->draw,
                             shader,
                             softpipe->sampler_views[shader],
                             softpipe->num_sampler_views[shader]);
   }

   softpipe->dirty |= SP_NEW_TEXTURE;
}


static void
softpipe_delete_sampler_state(struct pipe_context *pipe,
                              void *sampler)
{
   FREE( sampler );
}


static void
prepare_shader_sampling(
   struct softpipe_context *sp,
   unsigned num,
   struct pipe_sampler_view **views,
   enum pipe_shader_type shader_type,
   struct pipe_resource *mapped_tex[PIPE_MAX_SHADER_SAMPLER_VIEWS])
{

   unsigned i;
   uint32_t row_stride[PIPE_MAX_TEXTURE_LEVELS];
   uint32_t img_stride[PIPE_MAX_TEXTURE_LEVELS];
   uint32_t mip_offsets[PIPE_MAX_TEXTURE_LEVELS];
   const void *addr;

   assert(num <= PIPE_MAX_SHADER_SAMPLER_VIEWS);
   if (!num)
      return;

   for (i = 0; i < num; i++) {
      struct pipe_sampler_view *view = views[i];

      if (view) {
         struct pipe_resource *tex = view->texture;
         struct softpipe_resource *sp_tex = softpipe_resource(tex);
         unsigned width0 = tex->width0;
         unsigned num_layers = tex->depth0;
         unsigned first_level = 0;
         unsigned last_level = 0;

         /* We're referencing the texture's internal data, so save a
          * reference to it.
          */
         pipe_resource_reference(&mapped_tex[i], tex);

         if (!sp_tex->dt) {
            /* regular texture - setup array of mipmap level offsets */
            ASSERTED struct pipe_resource *res = view->texture;
            int j;

            if (view->target != PIPE_BUFFER) {
               first_level = view->u.tex.first_level;
               last_level = view->u.tex.last_level;
               assert(first_level <= last_level);
               assert(last_level <= res->last_level);
               addr = sp_tex->data;

               for (j = first_level; j <= last_level; j++) {
                  mip_offsets[j] = sp_tex->level_offset[j];
                  row_stride[j] = sp_tex->stride[j];
                  img_stride[j] = sp_tex->img_stride[j];
               }
               if (tex->target == PIPE_TEXTURE_1D_ARRAY ||
                   tex->target == PIPE_TEXTURE_2D_ARRAY ||
                   tex->target == PIPE_TEXTURE_CUBE ||
                   tex->target == PIPE_TEXTURE_CUBE_ARRAY) {
                  num_layers = view->u.tex.last_layer - view->u.tex.first_layer + 1;
                  for (j = first_level; j <= last_level; j++) {
                     mip_offsets[j] += view->u.tex.first_layer *
                                       sp_tex->img_stride[j];
                  }
                  if (view->target == PIPE_TEXTURE_CUBE ||
                      view->target == PIPE_TEXTURE_CUBE_ARRAY) {
                     assert(num_layers % 6 == 0);
                  }
                  assert(view->u.tex.first_layer <= view->u.tex.last_layer);
                  assert(view->u.tex.last_layer < res->array_size);
               }
            }
            else {
               unsigned view_blocksize = util_format_get_blocksize(view->format);
               addr = sp_tex->data;
               /* probably don't really need to fill that out */
               mip_offsets[0] = 0;
               row_stride[0] = 0;
               img_stride[0] = 0;

               /* everything specified in number of elements here. */
               width0 = view->u.buf.size / view_blocksize;
               addr = (uint8_t *)addr + view->u.buf.offset;
               assert(view->u.buf.offset + view->u.buf.size <= res->width0);
            }
         }
         else {
            /* display target texture/surface */
            struct softpipe_screen *screen = softpipe_screen(tex->screen);
            struct sw_winsys *winsys = screen->winsys;
            addr = winsys->displaytarget_map(winsys, sp_tex->dt,
                                             PIPE_MAP_READ);
            row_stride[0] = sp_tex->stride[0];
            img_stride[0] = sp_tex->img_stride[0];
            mip_offsets[0] = 0;
            assert(addr);
         }
         draw_set_mapped_texture(sp->draw,
                                 shader_type,
                                 i,
                                 width0, tex->height0, num_layers,
                                 first_level, last_level, 0, 0,
                                 addr,
                                 row_stride, img_stride, mip_offsets);
      }
   }
}

static void
sp_sampler_view_display_target_unmap(struct softpipe_context *sp,
                                     struct pipe_sampler_view *view)
{
   if (view) {
      struct pipe_resource *tex = view->texture;
      struct softpipe_resource *sp_tex = softpipe_resource(tex);
      if (sp_tex->dt) {
         struct softpipe_screen *screen = softpipe_screen(tex->screen);
         struct sw_winsys *winsys = screen->winsys;
         winsys->displaytarget_unmap(winsys, sp_tex->dt);
      }
   }
}

/**
 * Called during state validation when SP_NEW_TEXTURE is set.
 */
void
softpipe_prepare_vertex_sampling(struct softpipe_context *sp,
                                 unsigned num,
                                 struct pipe_sampler_view **views)
{
   prepare_shader_sampling(sp, num, views, PIPE_SHADER_VERTEX,
                           sp->mapped_vs_tex);
}

void
softpipe_cleanup_vertex_sampling(struct softpipe_context *ctx)
{
   unsigned i;
   for (i = 0; i < ARRAY_SIZE(ctx->mapped_vs_tex); i++) {
      sp_sampler_view_display_target_unmap(
         ctx, ctx->sampler_views[PIPE_SHADER_VERTEX][i]);
      pipe_resource_reference(&ctx->mapped_vs_tex[i], NULL);
   }
}


/**
 * Called during state validation when SP_NEW_TEXTURE is set.
 */
void
softpipe_prepare_geometry_sampling(struct softpipe_context *sp,
                                   unsigned num,
                                   struct pipe_sampler_view **views)
{
   prepare_shader_sampling(sp, num, views, PIPE_SHADER_GEOMETRY,
                           sp->mapped_gs_tex);
}

void
softpipe_cleanup_geometry_sampling(struct softpipe_context *ctx)
{
   unsigned i;
   for (i = 0; i < ARRAY_SIZE(ctx->mapped_gs_tex); i++) {
      sp_sampler_view_display_target_unmap(
         ctx, ctx->sampler_views[PIPE_SHADER_GEOMETRY][i]);
      pipe_resource_reference(&ctx->mapped_gs_tex[i], NULL);
   }
}


void
softpipe_init_sampler_funcs(struct pipe_context *pipe)
{
   pipe->create_sampler_state = softpipe_create_sampler_state;
   pipe->bind_sampler_states = softpipe_bind_sampler_states;
   pipe->delete_sampler_state = softpipe_delete_sampler_state;

   pipe->create_sampler_view = softpipe_create_sampler_view;
   pipe->set_sampler_views = softpipe_set_sampler_views;
   pipe->sampler_view_destroy = softpipe_sampler_view_destroy;
}

