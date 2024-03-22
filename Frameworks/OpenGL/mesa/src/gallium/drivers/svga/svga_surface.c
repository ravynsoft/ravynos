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

#include "svga_cmd.h"

#include "pipe/p_state.h"
#include "pipe/p_defines.h"
#include "util/u_inlines.h"
#include "util/u_thread.h"
#include "util/u_bitmask.h"
#include "util/format/u_format.h"
#include "util/u_math.h"
#include "util/u_memory.h"

#include "svga_format.h"
#include "svga_screen.h"
#include "svga_context.h"
#include "svga_sampler_view.h"
#include "svga_resource_texture.h"
#include "svga_surface.h"
#include "svga_debug.h"

static void svga_mark_surface_dirty(struct pipe_surface *surf);

void
svga_texture_copy_region(struct svga_context *svga,
                         struct svga_winsys_surface *src_handle,
                         unsigned srcSubResource,
                         unsigned src_x, unsigned src_y, unsigned src_z,
                         struct svga_winsys_surface *dst_handle,
                         unsigned dstSubResource,
                         unsigned dst_x, unsigned dst_y, unsigned dst_z,
                         unsigned width, unsigned height, unsigned depth)
{
   SVGA3dCopyBox box;

   assert(svga_have_vgpu10(svga));

   box.x = dst_x;
   box.y = dst_y;
   box.z = dst_z;
   box.w = width;
   box.h = height;
   box.d = depth;
   box.srcx = src_x;
   box.srcy = src_y;
   box.srcz = src_z;

   SVGA_RETRY(svga, SVGA3D_vgpu10_PredCopyRegion
              (svga->swc, dst_handle, dstSubResource,
               src_handle, srcSubResource, &box));
}


void
svga_texture_copy_handle(struct svga_context *svga,
                         struct svga_winsys_surface *src_handle,
                         unsigned src_x, unsigned src_y, unsigned src_z,
                         unsigned src_level, unsigned src_layer,
                         struct svga_winsys_surface *dst_handle,
                         unsigned dst_x, unsigned dst_y, unsigned dst_z,
                         unsigned dst_level, unsigned dst_layer,
                         unsigned width, unsigned height, unsigned depth)
{
   struct svga_surface dst, src;
   SVGA3dCopyBox box, *boxes;

   assert(svga);

   src.handle = src_handle;
   src.real_level = src_level;
   src.real_layer = src_layer;
   src.real_zslice = 0;

   dst.handle = dst_handle;
   dst.real_level = dst_level;
   dst.real_layer = dst_layer;
   dst.real_zslice = 0;

   box.x = dst_x;
   box.y = dst_y;
   box.z = dst_z;
   box.w = width;
   box.h = height;
   box.d = depth;
   box.srcx = src_x;
   box.srcy = src_y;
   box.srcz = src_z;

/*
   SVGA_DBG(DEBUG_VIEWS, "mipcopy src: %p %u (%ux%ux%u), dst: %p %u (%ux%ux%u)\n",
            src_handle, src_level, src_x, src_y, src_z,
            dst_handle, dst_level, dst_x, dst_y, dst_z);
*/

   SVGA_RETRY(svga, SVGA3D_BeginSurfaceCopy(svga->swc,
                                            &src.base,
                                            &dst.base,
                                            &boxes, 1));

   *boxes = box;
   SVGA_FIFOCommitAll(svga->swc);
}


/* A helper function to sync up the two surface handles.
 */
static void
svga_texture_copy_handle_resource(struct svga_context *svga,
                                  struct svga_texture *src_tex,
                                  struct svga_winsys_surface *dst,
                                  unsigned int numMipLevels,
                                  unsigned int numLayers,
                                  int zslice_pick,
                                  unsigned int mipoffset,
                                  unsigned int layeroffset)
{
   unsigned int i, j;
   unsigned int zoffset = 0;

   /* A negative zslice_pick implies zoffset at 0, and depth to copy is
    * from the depth of the texture at the particular mipmap level.
    */
   if (zslice_pick >= 0)
      zoffset = zslice_pick;

   for (i = 0; i < numMipLevels; i++) {
      unsigned int miplevel = i + mipoffset;

      for (j = 0; j < numLayers; j++) {
         if (svga_is_texture_level_defined(src_tex, j+layeroffset, miplevel)) {
            unsigned depth = (zslice_pick < 0 ?
                              u_minify(src_tex->b.depth0, miplevel) : 1);

            if (src_tex->b.nr_samples > 1) {
               unsigned subResource = j * numMipLevels + i;
               svga_texture_copy_region(svga, src_tex->handle,
                                        subResource, 0, 0, zoffset,
                                        dst, subResource, 0, 0, 0,
                                        src_tex->b.width0, src_tex->b.height0, depth);
            }
            else {
               svga_texture_copy_handle(svga,
                                        src_tex->handle,
                                        0, 0, zoffset,
                                        miplevel,
                                        j + layeroffset,
                                        dst, 0, 0, 0, i, j,
                                        u_minify(src_tex->b.width0, miplevel),
                                        u_minify(src_tex->b.height0, miplevel),
                                        depth);
            }
         }
      }
   }
}


struct svga_winsys_surface *
svga_texture_view_surface(struct svga_context *svga,
                          struct svga_texture *tex,
                          unsigned bind_flags,
                          SVGA3dSurfaceAllFlags flags,
                          SVGA3dSurfaceFormat format,
                          unsigned start_mip,
                          unsigned num_mip,
                          int layer_pick,
                          unsigned num_layers,
                          int zslice_pick,
                          bool cacheable,
                          struct svga_host_surface_cache_key *key) /* OUT */
{
   struct svga_screen *ss = svga_screen(svga->pipe.screen);
   struct svga_winsys_surface *handle = NULL;
   bool invalidated;
   bool needCopyResource;

   SVGA_DBG(DEBUG_PERF,
            "svga: Create surface view: layer %d zslice %d mips %d..%d\n",
            layer_pick, zslice_pick, start_mip, start_mip+num_mip-1);

   SVGA_STATS_TIME_PUSH(ss->sws, SVGA_STATS_TIME_EMULATESURFACEVIEW);

   key->flags = flags;
   key->format = format;
   key->numMipLevels = num_mip;
   key->size.width = u_minify(tex->b.width0, start_mip);
   key->size.height = u_minify(tex->b.height0, start_mip);
   key->size.depth = zslice_pick < 0 ? u_minify(tex->b.depth0, start_mip) : 1;
   key->cachable = 1;
   key->arraySize = 1;
   key->numFaces = 1;

   /* single sample surface can be treated as non-multisamples surface */
   key->sampleCount = tex->b.nr_samples > 1 ? tex->b.nr_samples : 0;

   if (key->sampleCount > 1) {
      assert(ss->sws->have_sm4_1);
      key->flags |= SVGA3D_SURFACE_MULTISAMPLE;
   }

   if (tex->b.target == PIPE_TEXTURE_CUBE && layer_pick < 0) {
      key->flags |= SVGA3D_SURFACE_CUBEMAP;
      key->numFaces = 6;
   } else if (tex->b.target == PIPE_TEXTURE_1D_ARRAY ||
              tex->b.target == PIPE_TEXTURE_2D_ARRAY) {
      key->arraySize = num_layers;
   }

   if (key->format == SVGA3D_FORMAT_INVALID) {
      key->cachable = 0;
      goto done;
   }

   if (cacheable && tex->backed_handle &&
       memcmp(key, &tex->backed_key, sizeof *key) == 0) {
      handle = tex->backed_handle;
      needCopyResource = tex->backed_age < tex->age;
   } else {
      SVGA_DBG(DEBUG_DMA, "surface_create for texture view\n");
      handle = svga_screen_surface_create(ss, bind_flags, PIPE_USAGE_DEFAULT,
                                          &invalidated, key);
      needCopyResource = true;

      if (cacheable && !tex->backed_handle) {
         tex->backed_handle = handle;
         memcpy(&tex->backed_key, key, sizeof *key);
      }
   }

   if (!handle) {
      key->cachable = 0;
      goto done;
   }

   SVGA_DBG(DEBUG_DMA, " --> got sid %p (texture view)\n", handle);

   if (layer_pick < 0)
      layer_pick = 0;

   if (needCopyResource) {
      svga_texture_copy_handle_resource(svga, tex, handle,
                                        key->numMipLevels,
                                        key->numFaces * key->arraySize,
                                        zslice_pick, start_mip, layer_pick);
      tex->backed_age = tex->age;
   }

done:
   SVGA_STATS_TIME_POP(ss->sws);

   return handle;
}


/**
 * A helper function to create a surface view.
 * The clone_resource boolean flag specifies whether to clone the resource
 * for the surface view.
 */
static struct pipe_surface *
svga_create_surface_view(struct pipe_context *pipe,
                         struct pipe_resource *pt,
                         const struct pipe_surface *surf_tmpl,
                         bool clone_resource)
{
   struct svga_context *svga = svga_context(pipe);
   struct svga_texture *tex = svga_texture(pt);
   struct pipe_screen *screen = pipe->screen;
   struct svga_screen *ss = svga_screen(screen);
   struct svga_surface *s;
   unsigned layer, zslice, bind;
   unsigned nlayers = 1;
   SVGA3dSurfaceAllFlags flags = 0;
   SVGA3dSurfaceFormat format;
   struct pipe_surface *retVal = NULL;

   s = CALLOC_STRUCT(svga_surface);
   if (!s)
      return NULL;

   SVGA_STATS_TIME_PUSH(ss->sws, SVGA_STATS_TIME_CREATESURFACEVIEW);

   if (pt->target == PIPE_TEXTURE_CUBE) {
      layer = surf_tmpl->u.tex.first_layer;
      zslice = 0;
   }
   else if (pt->target == PIPE_TEXTURE_1D_ARRAY ||
            pt->target == PIPE_TEXTURE_2D_ARRAY ||
            pt->target == PIPE_TEXTURE_CUBE_ARRAY) {
      layer = surf_tmpl->u.tex.first_layer;
      zslice = 0;
      nlayers = surf_tmpl->u.tex.last_layer - surf_tmpl->u.tex.first_layer + 1;
   }
   else {
      layer = 0;
      zslice = surf_tmpl->u.tex.first_layer;
   }

   pipe_reference_init(&s->base.reference, 1);
   pipe_resource_reference(&s->base.texture, pt);
   s->base.context = pipe;
   s->base.format = surf_tmpl->format;
   s->base.width = u_minify(pt->width0, surf_tmpl->u.tex.level);
   s->base.height = u_minify(pt->height0, surf_tmpl->u.tex.level);
   s->base.u.tex.level = surf_tmpl->u.tex.level;
   s->base.u.tex.first_layer = surf_tmpl->u.tex.first_layer;
   s->base.u.tex.last_layer = surf_tmpl->u.tex.last_layer;
   s->view_id = SVGA3D_INVALID_ID;

   s->backed = NULL;

   if (util_format_is_depth_or_stencil(surf_tmpl->format)) {
      flags = SVGA3D_SURFACE_HINT_DEPTHSTENCIL |
              SVGA3D_SURFACE_BIND_DEPTH_STENCIL;
      bind = PIPE_BIND_DEPTH_STENCIL;
   }
   else {
      flags = SVGA3D_SURFACE_HINT_RENDERTARGET |
              SVGA3D_SURFACE_BIND_RENDER_TARGET;
      bind = PIPE_BIND_RENDER_TARGET;
   }

   if (tex->imported) {
      /* imported resource (a window) */
      format = tex->key.format;
      if (util_format_is_srgb(surf_tmpl->format)) {
         /* sRGB rendering to window */
         format = svga_linear_to_srgb(format);
      }
   }
   else {
      format = svga_translate_format(ss, surf_tmpl->format, bind);
   }

   assert(format != SVGA3D_FORMAT_INVALID);

   if (clone_resource) {
      SVGA_DBG(DEBUG_VIEWS,
               "New backed surface view: resource %p, level %u layer %u z %u, %p\n",
               pt, surf_tmpl->u.tex.level, layer, zslice, s);

      if (svga_have_vgpu10(svga)) {
         switch (pt->target) {
         case PIPE_TEXTURE_1D:
            flags |= SVGA3D_SURFACE_1D;
            break;
         case PIPE_TEXTURE_1D_ARRAY:
            flags |= SVGA3D_SURFACE_1D | SVGA3D_SURFACE_ARRAY;
            break;
         case PIPE_TEXTURE_2D_ARRAY:
            flags |= SVGA3D_SURFACE_ARRAY;
            break;
         case PIPE_TEXTURE_3D:
            flags |= SVGA3D_SURFACE_VOLUME;
            break;
         case PIPE_TEXTURE_CUBE:
            if (nlayers == 6)
               flags |= SVGA3D_SURFACE_CUBEMAP;
            break;
         case PIPE_TEXTURE_CUBE_ARRAY:
            if (nlayers % 6 == 0)
               flags |= SVGA3D_SURFACE_CUBEMAP | SVGA3D_SURFACE_ARRAY;
            break;   
         default:
            break;
         }
      }

      /* When we clone the surface view resource, use the format used in
       * the creation of the original resource.
       */
      s->handle = svga_texture_view_surface(svga, tex, bind, flags,
                                            tex->key.format,
                                            surf_tmpl->u.tex.level, 1,
                                            layer, nlayers, zslice,
                                            true, &s->key);
      if (!s->handle) {
         FREE(s);
         goto done;
      }

      s->key.format = format;
      s->real_layer = 0;
      s->real_level = 0;
      s->real_zslice = 0;
   } else {
      SVGA_DBG(DEBUG_VIEWS,
               "New surface view: resource %p, level %u, layer %u, z %u, %p\n",
               pt, surf_tmpl->u.tex.level, layer, zslice, s);

      memset(&s->key, 0, sizeof s->key);
      s->key.format = format;
      s->handle = tex->handle;
      s->real_layer = layer;
      s->real_zslice = zslice;
      s->real_level = surf_tmpl->u.tex.level;
   }

   svga->hud.num_surface_views++;
   retVal = &s->base;

done:
   SVGA_STATS_TIME_POP(ss->sws);
   return retVal;
}


static struct pipe_surface *
svga_create_surface(struct pipe_context *pipe,
                    struct pipe_resource *pt,
                    const struct pipe_surface *surf_tmpl)
{
   struct svga_context *svga = svga_context(pipe);
   struct pipe_screen *screen = pipe->screen;
   struct pipe_surface *surf = NULL;
   bool view = false;

   SVGA_STATS_TIME_PUSH(svga_sws(svga), SVGA_STATS_TIME_CREATESURFACE);

   if (svga_screen(screen)->debug.force_surface_view)
      view = true;

   if (surf_tmpl->u.tex.level != 0 &&
       svga_screen(screen)->debug.force_level_surface_view)
      view = true;

   if (pt->target == PIPE_TEXTURE_3D)
      view = true;

   if (svga_have_vgpu10(svga) || svga_screen(screen)->debug.no_surface_view)
      view = false;

   surf = svga_create_surface_view(pipe, pt, surf_tmpl, view);

   SVGA_STATS_TIME_POP(svga_sws(svga));

   return surf;
}


/**
 * Create an alternate surface view and clone the resource if specified
 */
static struct svga_surface *
create_backed_surface_view(struct svga_context *svga, struct svga_surface *s,
                           bool clone_resource)
{
   struct svga_texture *tex = svga_texture(s->base.texture);

   if (!s->backed) {
      struct pipe_surface *backed_view;

      SVGA_STATS_TIME_PUSH(svga_sws(svga),
                           SVGA_STATS_TIME_CREATEBACKEDSURFACEVIEW);

      backed_view = svga_create_surface_view(&svga->pipe,
                                             &tex->b,
                                             &s->base,
                                             clone_resource);
      if (!backed_view)
         goto done;

      s->backed = svga_surface(backed_view);

      SVGA_STATS_TIME_POP(svga_sws(svga));
   }
   else if (s->backed->handle != tex->handle &&
            s->backed->age < tex->age) {
      /*
       * There is already an existing backing surface, but we still need to
       * sync the backing resource if the original resource has been modified
       * since the last copy.
       */
      struct svga_surface *bs = s->backed;
      unsigned int layer, zslice;

      assert(bs->handle);

      switch (tex->b.target) {
      case PIPE_TEXTURE_CUBE:
      case PIPE_TEXTURE_CUBE_ARRAY:
      case PIPE_TEXTURE_1D_ARRAY:
      case PIPE_TEXTURE_2D_ARRAY:
         layer = s->base.u.tex.first_layer;
         zslice = 0;
         break;
      default:
         layer = 0;
         zslice = s->base.u.tex.first_layer;
      }

      svga_texture_copy_handle_resource(svga, tex, bs->handle,
                                        bs->key.numMipLevels,
                                        bs->key.numFaces * bs->key.arraySize,
                                        zslice, s->base.u.tex.level, layer);
   }

   svga_mark_surface_dirty(&s->backed->base);
   s->backed->age = tex->age;

   assert(s->backed->base.context == &svga->pipe);

done:
   return s->backed;
}

/**
 * Create a DX RenderTarget/DepthStencil View for the given surface,
 * if needed.
 */
struct pipe_surface *
svga_validate_surface_view(struct svga_context *svga, struct svga_surface *s)
{
   enum pipe_error ret = PIPE_OK;
   enum pipe_shader_type shader;

   assert(svga_have_vgpu10(svga));
   assert(s);

   SVGA_STATS_TIME_PUSH(svga_sws(svga),
                        SVGA_STATS_TIME_VALIDATESURFACEVIEW);

   /**
    * DX spec explicitly specifies that no resource can be bound to a render
    * target view and a shader resource view simultaneously.
    * So first check if the resource bound to this surface view collides with
    * a sampler view. If so, then we will clone this surface view and its
    * associated resource. We will then use the cloned surface view for
    * render target.
    */
   for (shader = PIPE_SHADER_VERTEX; shader <= PIPE_SHADER_COMPUTE; shader++) {
      if (svga_check_sampler_view_resource_collision(svga, s->handle, shader)) {
         SVGA_DBG(DEBUG_VIEWS,
                  "same resource used in shaderResource and renderTarget 0x%x\n",
                  s->handle);
         s = create_backed_surface_view(svga, s, true);

         if (s)
            svga->state.hw_draw.has_backed_views = true;

         /* s may be null here if the function failed */
         break;
      }
   }

   /**
    * Create an alternate surface view for the specified context if the
    * view was created for another context.
    */
   if (s && s->base.context != &svga->pipe) {
      s = create_backed_surface_view(svga, s, false);

      if (s)
         svga->state.hw_draw.has_backed_views = true;
   }

   if (s && s->view_id == SVGA3D_INVALID_ID) {
      SVGA3dResourceType resType;
      SVGA3dRenderTargetViewDesc desc;
      struct svga_texture *stex = svga_texture(s->base.texture);

      if (stex->surface_state < SVGA_SURFACE_STATE_INVALIDATED) {
         assert(stex->handle);

         /* We are about to render into a surface that has not been validated.
          * First invalidate the surface so that the device does not
          * need to update the host-side copy with the invalid
          * content when the associated mob is first bound to the surface.
          */
         SVGA_RETRY(svga, SVGA3D_InvalidateGBSurface(svga->swc, stex->handle));
         stex->surface_state = SVGA_SURFACE_STATE_INVALIDATED;
      }

      desc.tex.mipSlice = s->real_level;
      desc.tex.firstArraySlice = s->real_layer + s->real_zslice;
      desc.tex.arraySize =
         s->base.u.tex.last_layer - s->base.u.tex.first_layer + 1;

      resType = svga_resource_type(s->base.texture->target);

      if (util_format_is_depth_or_stencil(s->base.format)) {

         /* Create depth stencil view only if the resource is created
          * with depth stencil bind flag.
          */
         if (stex->key.flags & SVGA3D_SURFACE_BIND_DEPTH_STENCIL) {
            s->view_id = util_bitmask_add(svga->surface_view_id_bm);
            ret = SVGA3D_vgpu10_DefineDepthStencilView(svga->swc,
                                                       s->view_id,
                                                       s->handle,
                                                       s->key.format,
                                                       resType,
                                                       &desc);
         }
      }
      else {
         /* Create render target view only if the resource is created
          * with render target bind flag.
          */
         if (stex->key.flags & SVGA3D_SURFACE_BIND_RENDER_TARGET) {
            SVGA3dSurfaceFormat view_format = s->key.format;

            /* Can't create RGBA render target view of a RGBX surface so adjust
             * the view format.  We do something similar for texture samplers in
             * svga_validate_pipe_sampler_view().
             */
            if (view_format == SVGA3D_B8G8R8A8_UNORM &&
                (stex->key.format == SVGA3D_B8G8R8X8_UNORM ||
                 stex->key.format == SVGA3D_B8G8R8X8_TYPELESS)) {
               view_format = SVGA3D_B8G8R8X8_UNORM;
            }

            s->view_id = util_bitmask_add(svga->surface_view_id_bm);
            ret = SVGA3D_vgpu10_DefineRenderTargetView(svga->swc,
                                                       s->view_id,
                                                       s->handle,
                                                       view_format,
                                                       resType,
                                                       &desc);
         }
      }

      if (ret != PIPE_OK) {
         util_bitmask_clear(svga->surface_view_id_bm, s->view_id);
         s->view_id = SVGA3D_INVALID_ID;
         s = NULL;
      }
   }
   
   SVGA_STATS_TIME_POP(svga_sws(svga));

   return s ? &s->base : NULL;
}



static void
svga_surface_destroy(struct pipe_context *pipe,
                     struct pipe_surface *surf)
{
   struct svga_context *svga = svga_context(pipe);
   struct svga_surface *s = svga_surface(surf);
   struct svga_texture *t = svga_texture(surf->texture);
   struct svga_screen *ss = svga_screen(surf->texture->screen);

   SVGA_STATS_TIME_PUSH(ss->sws, SVGA_STATS_TIME_DESTROYSURFACE);

   /* Destroy the backed view surface if it exists */
   if (s->backed) {
      svga_surface_destroy(pipe, &s->backed->base);
      s->backed = NULL;
   }

   /* Destroy the surface handle if this is a backed handle and
    * it is not being cached in the texture.
    */
   if (s->handle != t->handle && s->handle != t->backed_handle) {
      SVGA_DBG(DEBUG_DMA, "unref sid %p (tex surface)\n", s->handle);
      svga_screen_surface_destroy(ss, &s->key,
                                  svga_was_texture_rendered_to(t),
                                  &s->handle);
   }

   if (s->view_id != SVGA3D_INVALID_ID) {
      /* The SVGA3D device will generate a device error if the
       * render target view or depth stencil view is destroyed from
       * a context other than the one it was created with.
       * Similar to shader resource view, in this case, we will skip
       * the destroy for now.
       */
      if (surf->context != pipe) {
         _debug_printf("context mismatch in %s\n", __func__);
      }
      else {
         assert(svga_have_vgpu10(svga));
         if (util_format_is_depth_or_stencil(s->base.format)) {
            SVGA_RETRY(svga, SVGA3D_vgpu10_DestroyDepthStencilView(svga->swc,
                                                                   s->view_id));
         }
         else {
            SVGA_RETRY(svga, SVGA3D_vgpu10_DestroyRenderTargetView(svga->swc,
                                                                   s->view_id));
         }
         util_bitmask_clear(svga->surface_view_id_bm, s->view_id);
      }
   }

   pipe_resource_reference(&surf->texture, NULL);
   FREE(surf);

   svga->hud.num_surface_views--;
   SVGA_STATS_TIME_POP(ss->sws);
}


static void
svga_mark_surface_dirty(struct pipe_surface *surf)
{
   struct svga_surface *s = svga_surface(surf);
   struct svga_texture *tex = svga_texture(surf->texture);

   if (!s->dirty) {
      s->dirty = true;

      if (s->handle == tex->handle) {
         /* hmm so 3d textures always have all their slices marked ? */
         svga_define_texture_level(tex, surf->u.tex.first_layer,
                                   surf->u.tex.level);
      }
      else {
         /* this will happen later in svga_propagate_surface */
      }
   }

   /* Increment the view_age and texture age for this surface's mipmap
    * level so that any sampler views into the texture are re-validated too.
    * Note: we age the texture for backed surface view only when the
    *       backed surface is propagated to the original surface.
    */
   if (s->handle == tex->handle)
      svga_age_texture_view(tex, surf->u.tex.level);
}


void
svga_mark_surfaces_dirty(struct svga_context *svga)
{
   unsigned i;
   struct svga_hw_clear_state *hw = &svga->state.hw_clear;

   if (svga_have_vgpu10(svga)) {

      /* For VGPU10, mark the dirty bit in the rendertarget/depth stencil view surface.
       * This surface can be the backed surface.
       */
      for (i = 0; i < hw->num_rendertargets; i++) {
         if (hw->rtv[i])
            svga_mark_surface_dirty(hw->rtv[i]);
      }
      if (hw->dsv)
         svga_mark_surface_dirty(hw->dsv);
   } else {
      for (i = 0; i < svga->curr.framebuffer.nr_cbufs; i++) {
         if (svga->curr.framebuffer.cbufs[i])
            svga_mark_surface_dirty(svga->curr.framebuffer.cbufs[i]);
      }
      if (svga->curr.framebuffer.zsbuf)
         svga_mark_surface_dirty(svga->curr.framebuffer.zsbuf);
   }
}


/**
 * Progagate any changes from surfaces to texture.
 * pipe is optional context to inline the blit command in.
 */
void
svga_propagate_surface(struct svga_context *svga, struct pipe_surface *surf,
                       bool reset)
{
   struct svga_surface *s = svga_surface(surf);
   struct svga_texture *tex = svga_texture(surf->texture);
   struct svga_screen *ss = svga_screen(surf->texture->screen);

   if (!s->dirty)
      return;

   SVGA_STATS_TIME_PUSH(ss->sws, SVGA_STATS_TIME_PROPAGATESURFACE);

   /* Reset the dirty flag if specified. This is to ensure that
    * the dirty flag will not be reset and stay unset when the backing
    * surface is still being bound and rendered to.
    * The reset flag will be set to TRUE when the surface is propagated
    * and will be unbound.
    */
   s->dirty = !reset;

   ss->texture_timestamp++;
   svga_age_texture_view(tex, surf->u.tex.level);

   if (s->handle != tex->handle) {
      unsigned zslice, layer;
      unsigned nlayers = 1;
      unsigned i;
      unsigned numMipLevels = tex->b.last_level + 1;
      unsigned srcLevel = s->real_level;
      unsigned dstLevel = surf->u.tex.level;
      unsigned width = u_minify(tex->b.width0, dstLevel);
      unsigned height = u_minify(tex->b.height0, dstLevel);

      if (surf->texture->target == PIPE_TEXTURE_CUBE) {
         zslice = 0;
         layer = surf->u.tex.first_layer;
      }
      else if (surf->texture->target == PIPE_TEXTURE_1D_ARRAY ||
               surf->texture->target == PIPE_TEXTURE_2D_ARRAY ||
               surf->texture->target == PIPE_TEXTURE_CUBE_ARRAY) {
         zslice = 0;
         layer = surf->u.tex.first_layer;
         nlayers = surf->u.tex.last_layer - surf->u.tex.first_layer + 1;
      }
      else {
         zslice = surf->u.tex.first_layer;
         layer = 0;
      }

      SVGA_DBG(DEBUG_VIEWS,
               "Propagate surface %p to resource %p, level %u\n",
               surf, tex, surf->u.tex.level);

      if (svga_have_vgpu10(svga)) {
         unsigned srcSubResource, dstSubResource;

         for (i = 0; i < nlayers; i++) {
            srcSubResource = (s->real_layer + i) * numMipLevels + srcLevel;
            dstSubResource = (layer + i) * numMipLevels + dstLevel;

            svga_texture_copy_region(svga,
                                     s->handle, srcSubResource, 0, 0, 0,
                                     tex->handle, dstSubResource, 0, 0, zslice,
                                     width, height, 1);
            svga_define_texture_level(tex, layer + i, dstLevel);
         }
      }
      else {
         for (i = 0; i < nlayers; i++) {
            svga_texture_copy_handle(svga,
                                     s->handle, 0, 0, 0, srcLevel,
                                     s->real_layer + i,
                                     tex->handle, 0, 0, zslice, dstLevel,
                                     layer + i,
                                     width, height, 1);
        
            svga_define_texture_level(tex, layer + i, dstLevel);
         }
      }

      /* Sync the surface view age with the texture age */
      s->age = tex->age;

      /* If this backed surface is cached in the texture,
       * update the backed age as well.
       */
      if (tex->backed_handle == s->handle) {
         tex->backed_age = tex->age;
      }
   }

   SVGA_STATS_TIME_POP(ss->sws);
}


/**
 * If any of the render targets are in backing texture views, propagate any
 * changes to them back to the original texture.
 */
void
svga_propagate_rendertargets(struct svga_context *svga)
{
   unsigned i;

   /* Early exit if there is no backing texture views in use */
   if (!svga->state.hw_draw.has_backed_views)
      return;

   /* Note that we examine the svga->state.hw_draw.framebuffer surfaces,
    * not the svga->curr.framebuffer surfaces, because it's the former
    * surfaces which may be backing surface views (the actual render targets).
    */
   for (i = 0; i < svga->state.hw_clear.num_rendertargets; i++) {
      struct pipe_surface *s = svga->state.hw_clear.rtv[i];
      if (s) {
         svga_propagate_surface(svga, s, false);
      }
   }

   if (svga->state.hw_clear.dsv) {
      svga_propagate_surface(svga, svga->state.hw_clear.dsv, false);
   }
}


/**
 * Check if we should call svga_propagate_surface on the surface.
 */
bool
svga_surface_needs_propagation(const struct pipe_surface *surf)
{
   const struct svga_surface *s = svga_surface_const(surf);
   struct svga_texture *tex = svga_texture(surf->texture);

   return s->dirty && s->handle != tex->handle;
}


static void
svga_get_sample_position(struct pipe_context *context,
                         unsigned sample_count, unsigned sample_index,
                         float *pos_out)
{
   /* We can't actually query the device to learn the sample positions.
    * These were grabbed from nvidia's driver.
    */
   static const float pos1[1][2] = {
      { 0.5, 0.5 }
   };
   static const float pos2[2][2] = {
      { 0.75, 0.75 },
      { 0.25, 0.25 }
   };
   static const float pos4[4][2] = {
      { 0.375000, 0.125000 },
      { 0.875000, 0.375000 },
      { 0.125000, 0.625000 },
      { 0.625000, 0.875000 }
   };
   static const float pos8[8][2] = {
      { 0.562500, 0.312500 },
      { 0.437500, 0.687500 },
      { 0.812500, 0.562500 },
      { 0.312500, 0.187500 },
      { 0.187500, 0.812500 },
      { 0.062500, 0.437500 },
      { 0.687500, 0.937500 },
      { 0.937500, 0.062500 }
   };
   static const float pos16[16][2] = {
      { 0.187500, 0.062500 },
      { 0.437500, 0.187500 },
      { 0.062500, 0.312500 },
      { 0.312500, 0.437500 },
      { 0.687500, 0.062500 },
      { 0.937500, 0.187500 },
      { 0.562500, 0.312500 },
      { 0.812500, 0.437500 },
      { 0.187500, 0.562500 },
      { 0.437500, 0.687500 },
      { 0.062500, 0.812500 },
      { 0.312500, 0.937500 },
      { 0.687500, 0.562500 },
      { 0.937500, 0.687500 },
      { 0.562500, 0.812500 },
      { 0.812500, 0.937500 }
   };
   const float (*positions)[2];

   switch (sample_count) {
   case 2:
      positions = pos2;
      break;
   case 4:
      positions = pos4;
      break;
   case 8:
      positions = pos8;
      break;
   case 16:
      positions = pos16;
      break;
   default:
      positions = pos1;
   }

   pos_out[0] = positions[sample_index][0];
   pos_out[1] = positions[sample_index][1];
}


void
svga_init_surface_functions(struct svga_context *svga)
{
   svga->pipe.create_surface = svga_create_surface;
   svga->pipe.surface_destroy = svga_surface_destroy;
   svga->pipe.get_sample_position = svga_get_sample_position;
}
