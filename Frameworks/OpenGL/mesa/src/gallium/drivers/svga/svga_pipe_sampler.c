/**********************************************************
 * Copyright 2008-2009 VMware, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **********************************************************/

#include "pipe/p_defines.h"
#include "util/u_bitmask.h"
#include "util/format/u_format.h"
#include "util/u_inlines.h"
#include "util/u_math.h"
#include "util/u_memory.h"

#include "svga_context.h"
#include "svga_cmd.h"
#include "svga_debug.h"
#include "svga_resource_texture.h"
#include "svga_surface.h"
#include "svga_sampler_view.h"


static inline unsigned
translate_wrap_mode(unsigned wrap)
{
   switch (wrap) {
   case PIPE_TEX_WRAP_REPEAT:
      return SVGA3D_TEX_ADDRESS_WRAP;
   case PIPE_TEX_WRAP_CLAMP:
      return SVGA3D_TEX_ADDRESS_CLAMP;
   case PIPE_TEX_WRAP_CLAMP_TO_EDGE:
      /* Unfortunately SVGA3D_TEX_ADDRESS_EDGE not respected by
       * hardware.
       */
      return SVGA3D_TEX_ADDRESS_CLAMP;
   case PIPE_TEX_WRAP_CLAMP_TO_BORDER:
      return SVGA3D_TEX_ADDRESS_BORDER;
   case PIPE_TEX_WRAP_MIRROR_REPEAT:
      return SVGA3D_TEX_ADDRESS_MIRROR;
   case PIPE_TEX_WRAP_MIRROR_CLAMP:
   case PIPE_TEX_WRAP_MIRROR_CLAMP_TO_EDGE:
   case PIPE_TEX_WRAP_MIRROR_CLAMP_TO_BORDER:
      return SVGA3D_TEX_ADDRESS_MIRRORONCE;
   default:
      assert(0);
      return SVGA3D_TEX_ADDRESS_WRAP;
   }
}


static inline unsigned
translate_img_filter(unsigned filter)
{
   switch (filter) {
   case PIPE_TEX_FILTER_NEAREST:
      return SVGA3D_TEX_FILTER_NEAREST;
   case PIPE_TEX_FILTER_LINEAR:
      return SVGA3D_TEX_FILTER_LINEAR;
   default:
      assert(0);
      return SVGA3D_TEX_FILTER_NEAREST;
   }
}


static inline unsigned
translate_mip_filter(unsigned filter)
{
   switch (filter) {
   case PIPE_TEX_MIPFILTER_NONE:
      return SVGA3D_TEX_FILTER_NONE;
   case PIPE_TEX_MIPFILTER_NEAREST:
      return SVGA3D_TEX_FILTER_NEAREST;
   case PIPE_TEX_MIPFILTER_LINEAR:
      return SVGA3D_TEX_FILTER_LINEAR;
   default:
      assert(0);
      return SVGA3D_TEX_FILTER_NONE;
   }
}


static uint8
translate_comparison_func(unsigned func)
{
   switch (func) {
   case PIPE_FUNC_NEVER:
      return SVGA3D_COMPARISON_NEVER;
   case PIPE_FUNC_LESS:
      return SVGA3D_COMPARISON_LESS;
   case PIPE_FUNC_EQUAL:
      return SVGA3D_COMPARISON_EQUAL;
   case PIPE_FUNC_LEQUAL:
      return SVGA3D_COMPARISON_LESS_EQUAL;
   case PIPE_FUNC_GREATER:
      return SVGA3D_COMPARISON_GREATER;
   case PIPE_FUNC_NOTEQUAL:
      return SVGA3D_COMPARISON_NOT_EQUAL;
   case PIPE_FUNC_GEQUAL:
      return SVGA3D_COMPARISON_GREATER_EQUAL;
   case PIPE_FUNC_ALWAYS:
      return SVGA3D_COMPARISON_ALWAYS;
   default:
      assert(!"Invalid comparison function");
      return SVGA3D_COMPARISON_ALWAYS;
   }
}


/**
 * Translate filtering state to vgpu10 format.
 */
static SVGA3dFilter
translate_filter_mode(unsigned img_filter,
                      unsigned min_filter,
                      unsigned mag_filter,
                      bool anisotropic,
                      bool compare)
{
   SVGA3dFilter mode = 0;

   if (img_filter == PIPE_TEX_FILTER_LINEAR)
      mode |= SVGA3D_FILTER_MIP_LINEAR;
   if (min_filter == PIPE_TEX_FILTER_LINEAR)
      mode |= SVGA3D_FILTER_MIN_LINEAR;
   if (mag_filter == PIPE_TEX_FILTER_LINEAR)
      mode |= SVGA3D_FILTER_MAG_LINEAR;
   if (anisotropic)
      mode |= SVGA3D_FILTER_ANISOTROPIC;
   if (compare)
      mode |= SVGA3D_FILTER_COMPARE;

   return mode;
}


/**
 * Define a vgpu10 sampler state.
 */
static void
define_sampler_state_object(struct svga_context *svga,
                            struct svga_sampler_state *ss,
                            const struct pipe_sampler_state *ps)
{
   uint8_t max_aniso = (uint8_t) 255; /* XXX fix me */
   bool anisotropic;
   uint8 compare_func;
   SVGA3dFilter filter;
   SVGA3dRGBAFloat bcolor;
   float min_lod, max_lod;

   assert(svga_have_vgpu10(svga));

   anisotropic = ss->aniso_level > 1.0f;

   filter = translate_filter_mode(ps->min_mip_filter,
                                  ps->min_img_filter,
                                  ps->mag_img_filter,
                                  anisotropic,
                                  ss->compare_mode);

   compare_func = translate_comparison_func(ss->compare_func);

   COPY_4V(bcolor.value, ps->border_color.f);

   assert(ps->min_lod <= ps->max_lod);

   if (ps->min_mip_filter == PIPE_TEX_MIPFILTER_NONE) {
      /* just use the base level image */
      min_lod = max_lod = 0.0f;
   }
   else {
      min_lod = ps->min_lod;
      max_lod = ps->max_lod;
   }

   /* If shadow comparisons are enabled, create two sampler states: one
    * with the given shadow compare mode, another with shadow comparison off.
    * We need the later because in some cases, we have to do the shadow
    * compare in the shader.  So, we don't want to do it twice.
    */
   STATIC_ASSERT(PIPE_TEX_COMPARE_NONE == 0);
   STATIC_ASSERT(PIPE_TEX_COMPARE_R_TO_TEXTURE == 1);
   ss->id[1] = SVGA3D_INVALID_ID;

   unsigned i;
   for (i = 0; i <= ss->compare_mode; i++) {
      ss->id[i] = util_bitmask_add(svga->sampler_object_id_bm);

      SVGA_RETRY(svga, SVGA3D_vgpu10_DefineSamplerState
                 (svga->swc,
                  ss->id[i],
                  filter,
                  ss->addressu,
                  ss->addressv,
                  ss->addressw,
                  ss->lod_bias, /* float */
                  max_aniso,
                  compare_func,
                  bcolor,
                  min_lod,       /* float */
                  max_lod));      /* float */

      /* turn off the shadow compare option for second iteration */
      filter &= ~SVGA3D_FILTER_COMPARE;
   }
}


static void *
svga_create_sampler_state(struct pipe_context *pipe,
                          const struct pipe_sampler_state *sampler)
{
   struct svga_context *svga = svga_context(pipe);
   struct svga_sampler_state *cso = CALLOC_STRUCT( svga_sampler_state );

   if (!cso)
      return NULL;

   cso->mipfilter = translate_mip_filter(sampler->min_mip_filter);
   cso->magfilter = translate_img_filter( sampler->mag_img_filter );
   cso->minfilter = translate_img_filter( sampler->min_img_filter );
   cso->aniso_level = MAX2( sampler->max_anisotropy, 1 );
   if (sampler->max_anisotropy)
      cso->magfilter = cso->minfilter = SVGA3D_TEX_FILTER_ANISOTROPIC;
   cso->lod_bias = sampler->lod_bias;
   cso->addressu = translate_wrap_mode(sampler->wrap_s);
   cso->addressv = translate_wrap_mode(sampler->wrap_t);
   cso->addressw = translate_wrap_mode(sampler->wrap_r);
   cso->normalized_coords = !sampler->unnormalized_coords;
   cso->compare_mode = sampler->compare_mode;
   cso->compare_func = sampler->compare_func;

   {
      uint32 r = float_to_ubyte(sampler->border_color.f[0]);
      uint32 g = float_to_ubyte(sampler->border_color.f[1]);
      uint32 b = float_to_ubyte(sampler->border_color.f[2]);
      uint32 a = float_to_ubyte(sampler->border_color.f[3]);

      cso->bordercolor = (a << 24) | (r << 16) | (g << 8) | b;
   }

   /* No SVGA3D support for:
    *    - min/max LOD clamping
    */
   cso->min_lod = 0;
   cso->view_min_lod = MAX2((int) (sampler->min_lod + 0.5), 0);
   cso->view_max_lod = MAX2((int) (sampler->max_lod + 0.5), 0);

   /* Use min_mipmap */
   if (svga->debug.use_min_mipmap) {
      if (cso->view_min_lod == cso->view_max_lod) {
         cso->min_lod = cso->view_min_lod;
         cso->view_min_lod = 0;
         cso->view_max_lod = 1000; /* Just a high number */
         cso->mipfilter = SVGA3D_TEX_FILTER_NONE;
      }
   }

   if (svga_have_vgpu10(svga)) {
      define_sampler_state_object(svga, cso, sampler);
   }

   SVGA_DBG(DEBUG_SAMPLERS,
            "New sampler: min %u, view(min %u, max %u) lod, mipfilter %s\n",
            cso->min_lod, cso->view_min_lod, cso->view_max_lod,
            cso->mipfilter == SVGA3D_TEX_FILTER_NONE ? "SVGA3D_TEX_FILTER_NONE" : "SOMETHING");

   svga->hud.num_sampler_objects++;
   SVGA_STATS_COUNT_INC(svga_screen(svga->pipe.screen)->sws,
                        SVGA_STATS_COUNT_SAMPLER);

   return cso;
}


static void
svga_bind_sampler_states(struct pipe_context *pipe,
                         enum pipe_shader_type shader,
                         unsigned start,
                         unsigned num,
                         void **samplers)
{
   struct svga_context *svga = svga_context(pipe);
   unsigned i;
   bool any_change = false;

   assert(shader < PIPE_SHADER_TYPES);
   assert(start + num <= PIPE_MAX_SAMPLERS);

   /* Pre-VGPU10 only supports FS textures */
   if (!svga_have_vgpu10(svga) && shader != PIPE_SHADER_FRAGMENT)
      return;

   for (i = 0; i < num; i++) {
      if (svga->curr.sampler[shader][start + i] != samplers[i])
         any_change = true;
      svga->curr.sampler[shader][start + i] = samplers[i];
   }

   if (!any_change) {
      return;
   }

   /* find highest non-null sampler[] entry */
   {
      unsigned j = MAX2(svga->curr.num_samplers[shader], start + num);
      while (j > 0 && svga->curr.sampler[shader][j - 1] == NULL)
         j--;
      svga->curr.num_samplers[shader] = j;
   }

   svga->dirty |= SVGA_NEW_SAMPLER;
}


static void
svga_delete_sampler_state(struct pipe_context *pipe, void *sampler)
{
   struct svga_sampler_state *ss = (struct svga_sampler_state *) sampler;
   struct svga_context *svga = svga_context(pipe);

   if (svga_have_vgpu10(svga)) {
      unsigned i;
      for (i = 0; i < ARRAY_SIZE(ss->id); i++) {
         if (ss->id[i] != SVGA3D_INVALID_ID) {
            svga_hwtnl_flush_retry(svga);

            SVGA_RETRY(svga, SVGA3D_vgpu10_DestroySamplerState(svga->swc,
                                                               ss->id[i]));
            util_bitmask_clear(svga->sampler_object_id_bm, ss->id[i]);
         }
      }
   }

   FREE(sampler);
   svga->hud.num_sampler_objects--;
}


static struct pipe_sampler_view *
svga_create_sampler_view(struct pipe_context *pipe,
                         struct pipe_resource *texture,
                         const struct pipe_sampler_view *templ)
{
   struct svga_context *svga = svga_context(pipe);
   struct svga_pipe_sampler_view *sv = CALLOC_STRUCT(svga_pipe_sampler_view);

   if (!sv) {
      return NULL;
   }

   sv->base = *templ;
   sv->base.reference.count = 1;
   sv->base.texture = NULL;
   pipe_resource_reference(&sv->base.texture, texture);

   sv->base.context = pipe;
   sv->id = SVGA3D_INVALID_ID;

   svga->hud.num_samplerview_objects++;
   SVGA_STATS_COUNT_INC(svga_screen(svga->pipe.screen)->sws,
                        SVGA_STATS_COUNT_SAMPLERVIEW);

   return &sv->base;
}


static void
svga_sampler_view_destroy(struct pipe_context *pipe,
                          struct pipe_sampler_view *view)
{
   struct svga_context *svga = svga_context(pipe);
   struct svga_pipe_sampler_view *sv = svga_pipe_sampler_view(view);

   if (svga_have_vgpu10(svga) && sv->id != SVGA3D_INVALID_ID) {
      assert(view->context == pipe);

      svga_hwtnl_flush_retry(svga);

      SVGA_RETRY(svga, SVGA3D_vgpu10_DestroyShaderResourceView(svga->swc,
                                                               sv->id));
      util_bitmask_clear(svga->sampler_view_id_bm, sv->id);
   }

   pipe_resource_reference(&sv->base.texture, NULL);

   FREE(sv);
   svga->hud.num_samplerview_objects--;
}


static void
svga_set_sampler_views(struct pipe_context *pipe,
                       enum pipe_shader_type shader,
                       unsigned start,
                       unsigned num,
                       unsigned unbind_num_trailing_slots,
                       bool take_ownership,
                       struct pipe_sampler_view **views)
{
   struct svga_context *svga = svga_context(pipe);
   unsigned flag_1d = 0;
   unsigned flag_srgb = 0;
   uint i;
   bool any_change = false;

   assert(shader < PIPE_SHADER_TYPES);
   assert(start + num <= ARRAY_SIZE(svga->curr.sampler_views[shader]));

   /* Pre-VGPU10 only supports FS textures */
   if (!svga_have_vgpu10(svga) && shader != PIPE_SHADER_FRAGMENT) {
      for (unsigned i = 0; i < num; i++) {
         struct pipe_sampler_view *view = views[i];
         pipe_sampler_view_reference(&view, NULL);
      }
      return;
   }

   SVGA_STATS_TIME_PUSH(svga_sws(svga), SVGA_STATS_TIME_SETSAMPLERVIEWS);

   /* This bit of code works around a quirk in the CSO module.
    * If start=num=0 it means all sampler views should be released.
    * Note that the CSO module treats sampler views for fragment shaders
    * differently than other shader types.
    */
   if (start == 0 && num == 0 && svga->curr.num_sampler_views[shader] > 0) {
      for (i = 0; i < svga->curr.num_sampler_views[shader]; i++) {
         pipe_sampler_view_reference(&svga->curr.sampler_views[shader][i],
                                     NULL);
      }
      any_change = true;
   }

   for (i = 0; i < num; i++) {
      enum pipe_texture_target target;

      any_change |= svga->curr.sampler_views[shader][start + i] != views[i];

      if (take_ownership) {
         pipe_sampler_view_reference(&svga->curr.sampler_views[shader][start + i],
               NULL);
         svga->curr.sampler_views[shader][start + i] = views[i];
      } else if (svga->curr.sampler_views[shader][start + i] != views[i]) {
         pipe_sampler_view_reference(&svga->curr.sampler_views[shader][start + i],
                                     views[i]);
      }

      if (!views[i])
         continue;

      if (util_format_is_srgb(views[i]->format))
         flag_srgb |= 1 << (start + i);

      target = views[i]->target;
      if (target == PIPE_TEXTURE_1D) {
         flag_1d |= 1 << (start + i);
      } else if (target == PIPE_TEXTURE_RECT) {
         /* If the size of the bound texture changes, we need to emit new
          * const buffer values.
          */
         svga->dirty |= SVGA_NEW_TEXTURE_CONSTS;
      } else if (target == PIPE_BUFFER) {
         /* If the size of the bound buffer changes, we need to emit new
          * const buffer values.
          */
         svga->dirty |= SVGA_NEW_TEXTURE_CONSTS;
      }
   }

   for (; i < num + unbind_num_trailing_slots; i++) {
      if (svga->curr.sampler_views[shader][start + i]) {
         pipe_sampler_view_reference(&svga->curr.sampler_views[shader][start + i],
                                     NULL);
         any_change = true;
      }
   }

   if (!any_change) {
      goto done;
   }

   /* find highest non-null sampler_views[] entry */
   {
      unsigned j = MAX2(svga->curr.num_sampler_views[shader], start + num);
      while (j > 0 && svga->curr.sampler_views[shader][j - 1] == NULL)
         j--;
      svga->curr.num_sampler_views[shader] = j;
   }

   svga->dirty |= SVGA_NEW_TEXTURE_BINDING;

   if (flag_srgb != svga->curr.tex_flags.flag_srgb ||
       flag_1d != svga->curr.tex_flags.flag_1d) {
      svga->dirty |= SVGA_NEW_TEXTURE_FLAGS;
      svga->curr.tex_flags.flag_1d = flag_1d;
      svga->curr.tex_flags.flag_srgb = flag_srgb;
   }

   /* Check if any of the sampler view resources collide with the framebuffer
    * color buffers or depth stencil resource. If so, set the NEW_FRAME_BUFFER
    * dirty bit so that emit_framebuffer can be invoked to create backed view
    * for the conflicted surface view.
    */
   if (svga_check_sampler_framebuffer_resource_collision(svga, shader)) {
      svga->dirty |= SVGA_NEW_FRAME_BUFFER;
   }

done:
   SVGA_STATS_TIME_POP(svga_sws(svga));
}

/**
 * Clean up sampler, sampler view state at context destruction time
 */
void
svga_cleanup_sampler_state(struct svga_context *svga)
{
   enum pipe_shader_type shader;

   for (shader = 0; shader <= PIPE_SHADER_COMPUTE; shader++) {
      unsigned i;

      for (i = 0; i < svga->state.hw_draw.num_sampler_views[shader]; i++) {
         pipe_sampler_view_reference(&svga->state.hw_draw.sampler_views[shader][i],
                                     NULL);
      }
   }
   
   /* free polygon stipple state */
   if (svga->polygon_stipple.sampler) {
      svga->pipe.delete_sampler_state(&svga->pipe, svga->polygon_stipple.sampler);
   }

   if (svga->polygon_stipple.sampler_view) {
      svga->pipe.sampler_view_destroy(&svga->pipe,
                                      &svga->polygon_stipple.sampler_view->base);
   }
   pipe_resource_reference(&svga->polygon_stipple.texture, NULL);
}

void
svga_init_sampler_functions( struct svga_context *svga )
{
   svga->pipe.create_sampler_state = svga_create_sampler_state;
   svga->pipe.bind_sampler_states = svga_bind_sampler_states;
   svga->pipe.delete_sampler_state = svga_delete_sampler_state;
   svga->pipe.set_sampler_views = svga_set_sampler_views;
   svga->pipe.create_sampler_view = svga_create_sampler_view;
   svga->pipe.sampler_view_destroy = svga_sampler_view_destroy;
}
