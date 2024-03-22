/*
 * Copyright 2013 VMware, Inc.  All rights reserved.
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
 */


/**
 * VGPU10 sampler and sampler view functions.
 */


#include "pipe/p_defines.h"
#include "util/u_bitmask.h"
#include "util/format/u_format.h"
#include "util/u_inlines.h"
#include "util/u_math.h"
#include "util/u_memory.h"

#include "svga_cmd.h"
#include "svga_context.h"
#include "svga_format.h"
#include "svga_resource_buffer.h"
#include "svga_resource_texture.h"
#include "svga_sampler_view.h"
#include "svga_shader.h"
#include "svga_state.h"
#include "svga_surface.h"
#include "svga3d_surfacedefs.h"

/** Get resource handle for a texture or buffer */
static inline struct svga_winsys_surface *
svga_resource_handle(struct pipe_resource *res)
{
   if (res->target == PIPE_BUFFER) {
      return svga_buffer(res)->handle;
   }
   else {
      return svga_texture(res)->handle;
   }
}


/**
 * This helper function returns TRUE if the specified resource collides with
 * any of the resources bound to any of the currently bound sampler views.
 */
bool
svga_check_sampler_view_resource_collision(const struct svga_context *svga,
                                           const struct svga_winsys_surface *res,
                                           enum pipe_shader_type shader)
{
   struct pipe_screen *screen = svga->pipe.screen;
   unsigned i;

   if (svga_screen(screen)->debug.no_surface_view) {
      return false;
   }

   if (!svga_curr_shader_use_samplers(svga, shader))
      return false;

   for (i = 0; i < svga->curr.num_sampler_views[shader]; i++) {
      struct svga_pipe_sampler_view *sv =
         svga_pipe_sampler_view(svga->curr.sampler_views[shader][i]);

      if (sv && res == svga_resource_handle(sv->base.texture)) {
         return true;
      }
   }

   return false;
}


/**
 * Check if there are any resources that are both bound to a render target
 * and bound as a shader resource for the given type of shader.
 */
bool
svga_check_sampler_framebuffer_resource_collision(struct svga_context *svga,
                                                  enum pipe_shader_type shader)
{
   struct svga_surface *surf;
   unsigned i;

   for (i = 0; i < svga->curr.framebuffer.nr_cbufs; i++) {
      surf = svga_surface(svga->curr.framebuffer.cbufs[i]);
      if (surf &&
          svga_check_sampler_view_resource_collision(svga, surf->handle,
                                                     shader)) {
         return true;
      }
   }

   surf = svga_surface(svga->curr.framebuffer.zsbuf);
   if (surf &&
       svga_check_sampler_view_resource_collision(svga, surf->handle, shader)) {
      return true;
   }

   return false;
}


/**
 * Create a DX ShaderResourceSamplerView for the given pipe_sampler_view,
 * if needed.
 */
enum pipe_error
svga_validate_pipe_sampler_view(struct svga_context *svga,
                                struct svga_pipe_sampler_view *sv)
{
   enum pipe_error ret = PIPE_OK;

   if (sv->id == SVGA3D_INVALID_ID) {
      struct svga_screen *ss = svga_screen(svga->pipe.screen);
      struct pipe_resource *texture = sv->base.texture;
      struct svga_winsys_surface *surface;
      SVGA3dSurfaceFormat format;
      SVGA3dResourceType resourceDim;
      SVGA3dShaderResourceViewDesc viewDesc;
      enum pipe_format viewFormat = sv->base.format;
      enum pipe_texture_target target = sv->base.target;

      /* vgpu10 cannot create a BGRX view for a BGRA resource, so force it to
       * create a BGRA view (and vice versa).
       */
      if (viewFormat == PIPE_FORMAT_B8G8R8X8_UNORM &&
          svga_texture_device_format_has_alpha(texture)) {
         viewFormat = PIPE_FORMAT_B8G8R8A8_UNORM;
      }
      else if (viewFormat == PIPE_FORMAT_B8G8R8A8_UNORM &&
               !svga_texture_device_format_has_alpha(texture)) {
         viewFormat = PIPE_FORMAT_B8G8R8X8_UNORM;
      }

      if (target == PIPE_BUFFER) {
         unsigned pf_flags;
         assert(texture->target == PIPE_BUFFER);
         svga_translate_texture_buffer_view_format(viewFormat,
                                                   &format,
                                                   &pf_flags);
         surface = svga_buffer_handle(svga, texture, PIPE_BIND_SAMPLER_VIEW);
      }
      else {
         format = svga_translate_format(ss, viewFormat,
                                        PIPE_BIND_SAMPLER_VIEW);

         /* Convert the format to a sampler-friendly format, if needed */
         format = svga_sampler_format(format);

         surface = svga_texture(texture)->handle;
      }

      assert(format != SVGA3D_FORMAT_INVALID);

      if (target == PIPE_BUFFER) {
         unsigned elem_size = util_format_get_blocksize(sv->base.format);

         viewDesc.buffer.firstElement = sv->base.u.buf.offset / elem_size;
         viewDesc.buffer.numElements = sv->base.u.buf.size / elem_size;
      }
      else {
         viewDesc.tex.mostDetailedMip = sv->base.u.tex.first_level;
         viewDesc.tex.firstArraySlice = sv->base.u.tex.first_layer;
         viewDesc.tex.mipLevels = (sv->base.u.tex.last_level -
                                   sv->base.u.tex.first_level + 1);
      }

      /* arraySize in viewDesc specifies the number of array slices in a
       * texture array. For 3D texture, last_layer in
       * pipe_sampler_view specifies the last slice of the texture
       * which is different from the last slice in a texture array,
       * hence we need to set arraySize to 1 explicitly.
       */
      viewDesc.tex.arraySize =
         (target == PIPE_TEXTURE_3D || target == PIPE_BUFFER) ? 1 :
            (sv->base.u.tex.last_layer - sv->base.u.tex.first_layer + 1);

      switch (target) {
      case PIPE_BUFFER:
         resourceDim = SVGA3D_RESOURCE_BUFFER;
         break;
      case PIPE_TEXTURE_1D:
      case PIPE_TEXTURE_1D_ARRAY:
         resourceDim = SVGA3D_RESOURCE_TEXTURE1D;
         break;
      case PIPE_TEXTURE_RECT:
      case PIPE_TEXTURE_2D:
      case PIPE_TEXTURE_2D_ARRAY:
         resourceDim = SVGA3D_RESOURCE_TEXTURE2D;
         break;
      case PIPE_TEXTURE_3D:
         resourceDim = SVGA3D_RESOURCE_TEXTURE3D;
         break;
      case PIPE_TEXTURE_CUBE:
      case PIPE_TEXTURE_CUBE_ARRAY:
         resourceDim = SVGA3D_RESOURCE_TEXTURECUBE;
         break;

      default:
         assert(!"Unexpected texture type");
         resourceDim = SVGA3D_RESOURCE_TEXTURE2D;
      }

      sv->id = util_bitmask_add(svga->sampler_view_id_bm);

      ret = SVGA3D_vgpu10_DefineShaderResourceView(svga->swc,
                                                   sv->id,
                                                   surface,
                                                   format,
                                                   resourceDim,
                                                   &viewDesc);
      if (ret != PIPE_OK) {
         util_bitmask_clear(svga->sampler_view_id_bm, sv->id);
         sv->id = SVGA3D_INVALID_ID;
      }
   }

   return ret;
}


static enum pipe_error
update_sampler_resources(struct svga_context *svga, uint64_t dirty)
{
   enum pipe_error ret = PIPE_OK;
   enum pipe_shader_type shader;

   assert(svga_have_vgpu10(svga));

   for (shader = PIPE_SHADER_VERTEX; shader < PIPE_SHADER_COMPUTE; shader++) {
      SVGA3dShaderResourceViewId ids[PIPE_MAX_SAMPLERS];
      struct svga_winsys_surface *surfaces[PIPE_MAX_SAMPLERS];
      struct pipe_sampler_view *sampler_views[PIPE_MAX_SAMPLERS];
      unsigned count;
      unsigned nviews;
      unsigned i;

      count = svga->curr.num_sampler_views[shader];
      for (i = 0; i < count; i++) {
         struct svga_pipe_sampler_view *sv =
            svga_pipe_sampler_view(svga->curr.sampler_views[shader][i]);

         if (sv) {
            surfaces[i] = svga_resource_handle(sv->base.texture);

            ret = svga_validate_pipe_sampler_view(svga, sv);
            if (ret != PIPE_OK)
               return ret;

            assert(sv->id != SVGA3D_INVALID_ID);
            ids[i] = sv->id;
            sampler_views[i] = &sv->base;
         }
         else {
            surfaces[i] = NULL;
            ids[i] = SVGA3D_INVALID_ID;
            sampler_views[i] = NULL;
         }
      }

      for (; i < svga->state.hw_draw.num_sampler_views[shader]; i++) {
         ids[i] = SVGA3D_INVALID_ID;
         surfaces[i] = NULL;
         sampler_views[i] = NULL;
      }

      /* Number of ShaderResources that need to be modified. This includes
       * the one that need to be unbound.
       */
      nviews = MAX2(svga->state.hw_draw.num_sampler_views[shader], count);
      if (nviews > 0) {
         if (count != svga->state.hw_draw.num_sampler_views[shader] ||
             memcmp(sampler_views, svga->state.hw_draw.sampler_views[shader],
                    count * sizeof(sampler_views[0])) != 0) {
            SVGA3dShaderResourceViewId *pIds = ids;
            struct svga_winsys_surface **pSurf = surfaces;
            unsigned numSR = 0;

            /* Loop through the sampler view list to only emit
             * the sampler views that are not already in the
             * corresponding entries in the device's
             * shader resource list.
             */
            for (i = 0; i < nviews; i++) {
                bool emit;

                emit = sampler_views[i] ==
                       svga->state.hw_draw.sampler_views[shader][i];

                if (!emit && i == nviews-1) {
                   /* Include the last sampler view in the next emit
                    * if it is different.
                    */
                   emit = true;
                   numSR++;
                   i++;
                }
 
                if (emit) {
                   /* numSR can only be 0 if the first entry of the list
                    * is the same as the one in the device list.
                    * In this case, * there is nothing to send yet.
                    */
                   if (numSR) {
                      ret = SVGA3D_vgpu10_SetShaderResources(
                               svga->swc,
                               svga_shader_type(shader),
                               i - numSR, /* startView */
                               numSR,
                               pIds,
                               pSurf);

                      if (ret != PIPE_OK)
                         return ret;
                   }
                   pIds += (numSR + 1);
                   pSurf += (numSR + 1);
                   numSR = 0;
                }
                else
                   numSR++;
            }

            /* Save referenced sampler views in the hw draw state.  */
            svga->state.hw_draw.num_sampler_views[shader] = count;
            for (i = 0; i < nviews; i++) {
               pipe_sampler_view_reference(
                  &svga->state.hw_draw.sampler_views[shader][i],
                  sampler_views[i]);
            }
         }
      }
   }

   /* Handle polygon stipple sampler view */
   if (svga->curr.rast->templ.poly_stipple_enable) {
      const unsigned unit =
         svga_fs_variant(svga->state.hw_draw.fs)->pstipple_sampler_unit;
      struct svga_pipe_sampler_view *sv = svga->polygon_stipple.sampler_view;
      struct svga_winsys_surface *surface;

      assert(sv);
      if (!sv) {
         return PIPE_OK;  /* probably out of memory */
      }

      ret = svga_validate_pipe_sampler_view(svga, sv);
      if (ret != PIPE_OK)
         return ret;

      surface = svga_resource_handle(sv->base.texture);
      ret = SVGA3D_vgpu10_SetShaderResources(
               svga->swc,
               svga_shader_type(PIPE_SHADER_FRAGMENT),
               unit, /* startView */
               1,
               &sv->id,
               &surface);
   }
   return ret;
}


struct svga_tracked_state svga_hw_sampler_bindings = {
   "shader resources emit",
   SVGA_NEW_STIPPLE |
   SVGA_NEW_TEXTURE_BINDING,
   update_sampler_resources
};



static enum pipe_error
update_samplers(struct svga_context *svga, uint64_t dirty )
{
   enum pipe_error ret = PIPE_OK;
   enum pipe_shader_type shader;

   assert(svga_have_vgpu10(svga));

   for (shader = PIPE_SHADER_VERTEX; shader < PIPE_SHADER_COMPUTE; shader++) {
      const unsigned count = svga->curr.num_samplers[shader];
      SVGA3dSamplerId ids[PIPE_MAX_SAMPLERS*2];
      unsigned i;
      unsigned nsamplers = 0;
      bool sampler_state_mapping =
         svga_use_sampler_state_mapping(svga, count);

      for (i = 0; i < count; i++) {
         bool fs_shadow = false;
         const struct svga_sampler_state *sampler = svga->curr.sampler[shader][i];

         /* _NEW_FS */
         if (shader == PIPE_SHADER_FRAGMENT) {
            struct svga_fs_variant *fs =
               svga_fs_variant(svga->state.hw_draw.fs);

            if (fs && (fs->fs_shadow_compare_units & (1 << i))) {

               /* Use the alternate sampler state with the compare
                * bit disabled when comparison is done in the shader and
                * sampler state mapping is not enabled.
                */
               fs_shadow = true;
            }
         }

         if (!sampler_state_mapping) {
            if (sampler) {
               SVGA3dSamplerId id = sampler->id[fs_shadow];
               assert(id != SVGA3D_INVALID_ID);
               ids[i] = id;
            }
            else {
               ids[i] = SVGA3D_INVALID_ID;
            }
            nsamplers++;
         }
         else {
            if (sampler) {
               SVGA3dSamplerId id = sampler->id[0];
               assert(id != SVGA3D_INVALID_ID);

               /* Check if the sampler id is already on the ids list */
               unsigned k;
               for (k = 0; k < nsamplers; k++) {
                   if (ids[k] == id)
                      break;
               }

               /* add the id to the list if it is not already on the list */
               if (k == nsamplers) {
                  ids[nsamplers++] = id;

                  if (sampler->compare_mode == PIPE_TEX_COMPARE_R_TO_TEXTURE) {
                     /*
                      * add the alternate sampler state as well as the shader
                      * might use this alternate sampler state which has comparison
                      * disabled when the comparison is done in the shader.
                      */
                     ids[nsamplers++] = sampler->id[1];
                  }
               }
            }
         }
      }

      for (i = nsamplers; i < svga->state.hw_draw.num_samplers[shader]; i++) {
         ids[i] = SVGA3D_INVALID_ID;
      }

      unsigned nsamplerIds =
         MAX2(nsamplers, svga->state.hw_draw.num_samplers[shader]);

      if (nsamplerIds > 0) {

         if (nsamplers > SVGA3D_DX_MAX_SAMPLERS) {
            debug_warn_once("Too many sampler states");
            nsamplers = SVGA3D_DX_MAX_SAMPLERS;
         }

         if (nsamplers != svga->state.hw_draw.num_samplers[shader] ||
             memcmp(ids, svga->state.hw_draw.samplers[shader],
                    nsamplerIds * sizeof(ids[0])) != 0) {

            /* HW state is really changing */
            ret = SVGA3D_vgpu10_SetSamplers(svga->swc,
                                            nsamplerIds,
                                            0,                       /* start */
                                            svga_shader_type(shader), /* type */
                                            ids);
            if (ret != PIPE_OK)
               return ret;
            memcpy(svga->state.hw_draw.samplers[shader], ids,
                   nsamplerIds * sizeof(ids[0]));
            svga->state.hw_draw.num_samplers[shader] = nsamplers;
         }
      }
   }

   /* Handle polygon stipple sampler texture */
   if (svga->curr.rast->templ.poly_stipple_enable) {
      const unsigned unit =
         svga_fs_variant(svga->state.hw_draw.fs)->pstipple_sampler_state_index;
      struct svga_sampler_state *sampler = svga->polygon_stipple.sampler;

      assert(sampler);
      if (!sampler) {
         return PIPE_OK; /* probably out of memory */
      }

      if (svga->state.hw_draw.samplers[PIPE_SHADER_FRAGMENT][unit]
          != sampler->id[0]) {
         ret = SVGA3D_vgpu10_SetSamplers(svga->swc,
                                         1, /* count */
                                         unit, /* start */
                                         SVGA3D_SHADERTYPE_PS,
                                         &sampler->id[0]);
         if (ret != PIPE_OK)
            return ret;

         /* save the polygon stipple sampler in the hw draw state */
         svga->state.hw_draw.samplers[PIPE_SHADER_FRAGMENT][unit] =
            sampler->id[0];
      }
      svga->state.hw_draw.num_samplers[PIPE_SHADER_FRAGMENT]++;
   }

   return ret;
}


struct svga_tracked_state svga_hw_sampler = {
   "texture sampler emit",
   (SVGA_NEW_FS |
    SVGA_NEW_SAMPLER |
    SVGA_NEW_STIPPLE),
   update_samplers
};


static enum pipe_error
update_cs_sampler_resources(struct svga_context *svga, uint64_t dirty)
{
   enum pipe_error ret = PIPE_OK;
   enum pipe_shader_type shader = PIPE_SHADER_COMPUTE;

   assert(svga_have_sm5(svga));

   SVGA3dShaderResourceViewId ids[PIPE_MAX_SAMPLERS];
   struct svga_winsys_surface *surfaces[PIPE_MAX_SAMPLERS];
   struct pipe_sampler_view *sampler_views[PIPE_MAX_SAMPLERS];
   unsigned count;
   unsigned nviews;
   unsigned i;
   struct svga_compute_shader *cs = svga->curr.cs;

   count = svga->curr.num_sampler_views[shader];
   if (!cs || !cs->base.info.uses_samplers)
      count = 0;

   for (i = 0; i < count; i++) {
      struct svga_pipe_sampler_view *sv =
         svga_pipe_sampler_view(svga->curr.sampler_views[shader][i]);

      if (sv) {
         surfaces[i] = svga_resource_handle(sv->base.texture);

         ret = svga_validate_pipe_sampler_view(svga, sv);
         if (ret != PIPE_OK)
            return ret;

         assert(sv->id != SVGA3D_INVALID_ID);
         ids[i] = sv->id;
         sampler_views[i] = &sv->base;
      }
      else {
         surfaces[i] = NULL;
         ids[i] = SVGA3D_INVALID_ID;
         sampler_views[i] = NULL;
      }
   }

   for (; i < svga->state.hw_draw.num_sampler_views[shader]; i++) {
      ids[i] = SVGA3D_INVALID_ID;
      surfaces[i] = NULL;
      sampler_views[i] = NULL;
   }

   /* Number of ShaderResources that need to be modified. This includes
    * the one that need to be unbound.
    */
   nviews = MAX2(svga->state.hw_draw.num_sampler_views[shader], count);
   if (nviews > 0) {
      if (count != svga->state.hw_draw.num_sampler_views[shader] ||
          memcmp(sampler_views, svga->state.hw_draw.sampler_views[shader],
                 count * sizeof(sampler_views[0])) != 0) {
         SVGA3dShaderResourceViewId *pIds = ids;
         struct svga_winsys_surface **pSurf = surfaces;
         unsigned numSR = 0;

         /* Loop through the sampler view list to only emit the sampler views
          * that are not already in the corresponding entries in the device's
          * shader resource list.
          */
         for (i = 0; i < nviews; i++) {
            bool emit;

            emit = sampler_views[i] ==
                   svga->state.hw_draw.sampler_views[shader][i];

            if (!emit && i == nviews - 1) {
               /* Include the last sampler view in the next emit
                * if it is different.
                */
               emit = true;
               numSR++;
               i++;
            }

            if (emit) {
               /* numSR can only be 0 if the first entry of the list
                * is the same as the one in the device list.
                * In this case, * there is nothing to send yet.
                */
               if (numSR) {
                  ret = SVGA3D_vgpu10_SetShaderResources(svga->swc,
                           svga_shader_type(shader),
                           i - numSR, /* startView */
                           numSR,
                           pIds,
                           pSurf);

                  if (ret != PIPE_OK)
                     return ret;
               }
               pIds += (numSR + 1);
               pSurf += (numSR + 1);
               numSR = 0;
            }
            else
               numSR++;
         }

         /* Save referenced sampler views in the hw draw state.  */
         svga->state.hw_draw.num_sampler_views[shader] = count;
         for (i = 0; i < nviews; i++) {
            pipe_sampler_view_reference(
               &svga->state.hw_draw.sampler_views[shader][i],
               sampler_views[i]);
         }
      }
   }
   return ret;
}


struct svga_tracked_state svga_hw_cs_sampler_bindings = {
   "cs shader resources emit",
   SVGA_NEW_TEXTURE_BINDING,
   update_cs_sampler_resources
};

static enum pipe_error
update_cs_samplers(struct svga_context *svga, uint64_t dirty )
{
   enum pipe_error ret = PIPE_OK;
   enum pipe_shader_type shader = PIPE_SHADER_COMPUTE;

   assert(svga_have_sm5(svga));

   const unsigned count = svga->curr.num_samplers[shader];
   SVGA3dSamplerId ids[PIPE_MAX_SAMPLERS];
   unsigned i;
   unsigned nsamplers;

   for (i = 0; i < count; i++) {
      if (svga->curr.sampler[shader][i]) {
         ids[i] = svga->curr.sampler[shader][i]->id[0];
         assert(ids[i] != SVGA3D_INVALID_ID);
      }
      else {
         ids[i] = SVGA3D_INVALID_ID;
      }
   }

   for (; i < svga->state.hw_draw.num_samplers[shader]; i++) {
      ids[i] = SVGA3D_INVALID_ID;
   }

   nsamplers = MAX2(svga->state.hw_draw.num_samplers[shader], count);
   if (nsamplers > 0) {
      if (count != svga->state.hw_draw.num_samplers[shader] ||
          memcmp(ids, svga->state.hw_draw.samplers[shader],
                 count * sizeof(ids[0])) != 0) {
         /* HW state is really changing */
         ret = SVGA3D_vgpu10_SetSamplers(svga->swc,
                                         nsamplers,
                                         0,                        /* start */
                                         svga_shader_type(shader), /* type */
                                         ids);
         if (ret != PIPE_OK)
            return ret;

         memcpy(svga->state.hw_draw.samplers[shader], ids,
                nsamplers * sizeof(ids[0]));
         svga->state.hw_draw.num_samplers[shader] = count;
      }
   }

   return ret;
}


struct svga_tracked_state svga_hw_cs_sampler = {
   "texture cs sampler emit",
   (SVGA_NEW_CS |
    SVGA_NEW_SAMPLER),
   update_cs_samplers
};
