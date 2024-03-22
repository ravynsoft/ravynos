/*
 * Copyright (C) 2008 VMware, Inc.
 * Copyright (C) 2012 Rob Clark <robclark@freedesktop.org>
 * Copyright (C) 2014-2017 Broadcom
 * Copyright (C) 2018-2019 Alyssa Rosenzweig
 * Copyright (C) 2019 Collabora, Ltd.
 * Copyright (C) 2023 Amazon.com, Inc. or its affiliates
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors (Collabora):
 *   Tomeu Vizoso <tomeu.vizoso@collabora.com>
 *   Alyssa Rosenzweig <alyssa.rosenzweig@collabora.com>
 *
 */

#include <fcntl.h>
#include <xf86drm.h>
#include "drm-uapi/drm_fourcc.h"

#include "frontend/winsys_handle.h"
#include "util/format/u_format.h"
#include "util/u_debug_image.h"
#include "util/u_drm.h"
#include "util/u_gen_mipmap.h"
#include "util/u_memory.h"
#include "util/u_resource.h"
#include "util/u_surface.h"
#include "util/u_transfer.h"
#include "util/u_transfer_helper.h"

#include "decode.h"
#include "pan_bo.h"
#include "pan_context.h"
#include "pan_resource.h"
#include "pan_screen.h"
#include "pan_tiling.h"
#include "pan_util.h"

static void
panfrost_clear_depth_stencil(struct pipe_context *pipe,
                             struct pipe_surface *dst, unsigned clear_flags,
                             double depth, unsigned stencil, unsigned dstx,
                             unsigned dsty, unsigned width, unsigned height,
                             bool render_condition_enabled)
{
   struct panfrost_context *ctx = pan_context(pipe);

   if (render_condition_enabled && !panfrost_render_condition_check(ctx))
      return;

   panfrost_blitter_save(
      ctx, render_condition_enabled ? PAN_RENDER_COND : PAN_RENDER_BASE);
   util_blitter_clear_depth_stencil(ctx->blitter, dst, clear_flags, depth,
                                    stencil, dstx, dsty, width, height);
}

static void
panfrost_clear_render_target(struct pipe_context *pipe,
                             struct pipe_surface *dst,
                             const union pipe_color_union *color, unsigned dstx,
                             unsigned dsty, unsigned width, unsigned height,
                             bool render_condition_enabled)
{
   struct panfrost_context *ctx = pan_context(pipe);

   if (render_condition_enabled && !panfrost_render_condition_check(ctx))
      return;

   panfrost_blitter_save(
      ctx, render_condition_enabled ? PAN_RENDER_COND : PAN_RENDER_BASE);
   util_blitter_clear_render_target(ctx->blitter, dst, color, dstx, dsty, width,
                                    height);
}

static struct pipe_resource *
panfrost_resource_from_handle(struct pipe_screen *pscreen,
                              const struct pipe_resource *templat,
                              struct winsys_handle *whandle, unsigned usage)
{
   struct panfrost_device *dev = pan_device(pscreen);
   struct panfrost_resource *rsc;
   struct pipe_resource *prsc;

   assert(whandle->type == WINSYS_HANDLE_TYPE_FD);

   rsc = CALLOC_STRUCT(panfrost_resource);
   if (!rsc)
      return NULL;

   prsc = &rsc->base;

   *prsc = *templat;

   pipe_reference_init(&prsc->reference, 1);
   prsc->screen = pscreen;

   uint64_t mod = whandle->modifier == DRM_FORMAT_MOD_INVALID
                     ? DRM_FORMAT_MOD_LINEAR
                     : whandle->modifier;
   enum mali_texture_dimension dim =
      panfrost_translate_texture_dimension(templat->target);
   struct pan_image_explicit_layout explicit_layout = {
      .offset = whandle->offset,
      .row_stride =
         panfrost_from_legacy_stride(whandle->stride, templat->format, mod),
   };

   rsc->image.layout = (struct pan_image_layout){
      .modifier = mod,
      .format = templat->format,
      .dim = dim,
      .width = prsc->width0,
      .height = prsc->height0,
      .depth = prsc->depth0,
      .array_size = prsc->array_size,
      .nr_samples = MAX2(prsc->nr_samples, 1),
      .nr_slices = 1,
   };

   bool valid =
      pan_image_layout_init(dev, &rsc->image.layout, &explicit_layout);

   if (!valid) {
      FREE(rsc);
      return NULL;
   }

   rsc->image.data.bo = panfrost_bo_import(dev, whandle->handle);
   /* Sometimes an import can fail e.g. on an invalid buffer fd, out of
    * memory space to mmap it etc.
    */
   if (!rsc->image.data.bo) {
      FREE(rsc);
      return NULL;
   }

   rsc->modifier_constant = true;

   BITSET_SET(rsc->valid.data, 0);
   panfrost_resource_set_damage_region(pscreen, &rsc->base, 0, NULL);

   if (dev->ro) {
      rsc->scanout =
         renderonly_create_gpu_import_for_resource(prsc, dev->ro, NULL);
      /* failure is expected in some cases.. */
   }

   return prsc;
}

static bool
panfrost_resource_get_handle(struct pipe_screen *pscreen,
                             struct pipe_context *ctx, struct pipe_resource *pt,
                             struct winsys_handle *handle, unsigned usage)
{
   struct panfrost_device *dev = pan_device(pscreen);
   struct panfrost_resource *rsrc;
   struct renderonly_scanout *scanout;
   struct pipe_resource *cur = pt;

   /* Even though panfrost doesn't support multi-planar formats, we
    * can get here through GBM, which does. Walk the list of planes
    * to find the right one.
    */
   for (int i = 0; i < handle->plane; i++) {
      cur = cur->next;
      if (!cur)
         return false;
   }
   rsrc = pan_resource(cur);
   scanout = rsrc->scanout;

   handle->modifier = rsrc->image.layout.modifier;
   rsrc->modifier_constant = true;

   if (handle->type == WINSYS_HANDLE_TYPE_KMS && dev->ro) {
      return renderonly_get_handle(scanout, handle);
   } else if (handle->type == WINSYS_HANDLE_TYPE_KMS) {
      handle->handle = panfrost_bo_handle(rsrc->image.data.bo);
   } else if (handle->type == WINSYS_HANDLE_TYPE_FD) {
      int fd = panfrost_bo_export(rsrc->image.data.bo);

      if (fd < 0)
         return false;

      handle->handle = fd;
   } else {
      /* Other handle types not supported */
      return false;
   }

   handle->stride = panfrost_get_legacy_stride(&rsrc->image.layout, 0);
   handle->offset = rsrc->image.layout.slices[0].offset;
   return true;
}

static bool
panfrost_resource_get_param(struct pipe_screen *pscreen,
                            struct pipe_context *pctx,
                            struct pipe_resource *prsc, unsigned plane,
                            unsigned layer, unsigned level,
                            enum pipe_resource_param param, unsigned usage,
                            uint64_t *value)
{
   struct panfrost_resource *rsrc =
      (struct panfrost_resource *)util_resource_at_index(prsc, plane);

   switch (param) {
   case PIPE_RESOURCE_PARAM_STRIDE:
      *value = panfrost_get_legacy_stride(&rsrc->image.layout, level);
      return true;
   case PIPE_RESOURCE_PARAM_OFFSET:
      *value = rsrc->image.layout.slices[level].offset;
      return true;
   case PIPE_RESOURCE_PARAM_MODIFIER:
      *value = rsrc->image.layout.modifier;
      return true;
   case PIPE_RESOURCE_PARAM_NPLANES:
      *value = util_resource_num(prsc);
      return true;
   default:
      return false;
   }
}

static void
panfrost_flush_resource(struct pipe_context *pctx, struct pipe_resource *prsc)
{
   /* TODO */
}

static struct pipe_surface *
panfrost_create_surface(struct pipe_context *pipe, struct pipe_resource *pt,
                        const struct pipe_surface *surf_tmpl)
{
   struct pipe_surface *ps = NULL;

   ps = CALLOC_STRUCT(pipe_surface);

   if (ps) {
      pipe_reference_init(&ps->reference, 1);
      pipe_resource_reference(&ps->texture, pt);
      ps->context = pipe;
      ps->format = surf_tmpl->format;

      if (pt->target != PIPE_BUFFER) {
         assert(surf_tmpl->u.tex.level <= pt->last_level);
         ps->width = u_minify(pt->width0, surf_tmpl->u.tex.level);
         ps->height = u_minify(pt->height0, surf_tmpl->u.tex.level);
         ps->nr_samples = surf_tmpl->nr_samples;
         ps->u.tex.level = surf_tmpl->u.tex.level;
         ps->u.tex.first_layer = surf_tmpl->u.tex.first_layer;
         ps->u.tex.last_layer = surf_tmpl->u.tex.last_layer;
      } else {
         /* setting width as number of elements should get us correct
          * renderbuffer width */
         ps->width =
            surf_tmpl->u.buf.last_element - surf_tmpl->u.buf.first_element + 1;
         ps->height = pt->height0;
         ps->u.buf.first_element = surf_tmpl->u.buf.first_element;
         ps->u.buf.last_element = surf_tmpl->u.buf.last_element;
         assert(ps->u.buf.first_element <= ps->u.buf.last_element);
         assert(ps->u.buf.last_element < ps->width);
      }
   }

   return ps;
}

static void
panfrost_surface_destroy(struct pipe_context *pipe, struct pipe_surface *surf)
{
   assert(surf->texture);
   pipe_resource_reference(&surf->texture, NULL);
   free(surf);
}

static inline bool
panfrost_is_2d(const struct panfrost_resource *pres)
{
   return (pres->base.target == PIPE_TEXTURE_2D) ||
          (pres->base.target == PIPE_TEXTURE_RECT);
}

/* Based on the usage, determine if it makes sense to use u-inteleaved tiling.
 * We only have routines to tile 2D textures of sane bpps. On the hardware
 * level, not all usages are valid for tiling. Finally, if the app is hinting
 * that the contents frequently change, tiling will be a loss.
 *
 * On platforms where it is supported, AFBC is even better. */

static bool
panfrost_should_afbc(struct panfrost_device *dev,
                     const struct panfrost_resource *pres, enum pipe_format fmt)
{
   /* AFBC resources may be rendered to, textured from, or shared across
    * processes, but may not be used as e.g buffers */
   const unsigned valid_binding =
      PIPE_BIND_DEPTH_STENCIL | PIPE_BIND_RENDER_TARGET | PIPE_BIND_BLENDABLE |
      PIPE_BIND_SAMPLER_VIEW | PIPE_BIND_DISPLAY_TARGET | PIPE_BIND_SCANOUT |
      PIPE_BIND_SHARED;

   if (pres->base.bind & ~valid_binding)
      return false;

   /* AFBC support is optional */
   if (!dev->has_afbc)
      return false;

   /* AFBC<-->staging is expensive */
   if (pres->base.usage == PIPE_USAGE_STREAM)
      return false;

   /* If constant (non-data-dependent) format is requested, don't AFBC: */
   if (pres->base.bind & PIPE_BIND_CONST_BW)
      return false;

   /* Only a small selection of formats are AFBC'able */
   if (!panfrost_format_supports_afbc(dev, fmt))
      return false;

   /* AFBC does not support layered (GLES3 style) multisampling. Use
    * EXT_multisampled_render_to_texture instead */
   if (pres->base.nr_samples > 1)
      return false;

   switch (pres->base.target) {
   case PIPE_TEXTURE_2D:
   case PIPE_TEXTURE_RECT:
   case PIPE_TEXTURE_2D_ARRAY:
   case PIPE_TEXTURE_CUBE:
   case PIPE_TEXTURE_CUBE_ARRAY:
      break;

   case PIPE_TEXTURE_3D:
      /* 3D AFBC is only supported on Bifrost v7+. It's supposed to
       * be supported on Midgard but it doesn't seem to work */
      if (dev->arch != 7)
         return false;

      break;

   default:
      return false;
   }

   /* For one tile, AFBC is a loss compared to u-interleaved */
   if (pres->base.width0 <= 16 && pres->base.height0 <= 16)
      return false;

   /* Otherwise, we'd prefer AFBC as it is dramatically more efficient
    * than linear or usually even u-interleaved */
   return true;
}

/*
 * For a resource we want to use AFBC with, should we use AFBC with tiled
 * headers? On GPUs that support it, this is believed to be beneficial for
 * images that are at least 128x128.
 */
static bool
panfrost_should_tile_afbc(const struct panfrost_device *dev,
                          const struct panfrost_resource *pres)
{
   return panfrost_afbc_can_tile(dev) && pres->base.width0 >= 128 &&
          pres->base.height0 >= 128 && !(dev->debug & PAN_DBG_FORCE_PACK);
}

bool
panfrost_should_pack_afbc(struct panfrost_device *dev,
                          const struct panfrost_resource *prsrc)
{
   const unsigned valid_binding = PIPE_BIND_DEPTH_STENCIL |
                                  PIPE_BIND_RENDER_TARGET |
                                  PIPE_BIND_SAMPLER_VIEW;

   return panfrost_afbc_can_pack(prsrc->base.format) && panfrost_is_2d(prsrc) &&
          drm_is_afbc(prsrc->image.layout.modifier) &&
          (prsrc->image.layout.modifier & AFBC_FORMAT_MOD_SPARSE) &&
          (prsrc->base.bind & ~valid_binding) == 0 &&
          !prsrc->modifier_constant && prsrc->base.width0 >= 32 &&
          prsrc->base.height0 >= 32;
}

static bool
panfrost_should_tile(struct panfrost_device *dev,
                     const struct panfrost_resource *pres, enum pipe_format fmt)
{
   const unsigned valid_binding =
      PIPE_BIND_DEPTH_STENCIL | PIPE_BIND_RENDER_TARGET | PIPE_BIND_BLENDABLE |
      PIPE_BIND_SAMPLER_VIEW | PIPE_BIND_DISPLAY_TARGET | PIPE_BIND_SCANOUT |
      PIPE_BIND_SHARED;

   /* The purpose of tiling is improving locality in both X- and
    * Y-directions. If there is only a single pixel in either direction,
    * tiling does not make sense; using a linear layout instead is optimal
    * for both memory usage and performance.
    */
   if (MIN2(pres->base.width0, pres->base.height0) < 2)
      return false;

   bool can_tile = (pres->base.target != PIPE_BUFFER) &&
                   ((pres->base.bind & ~valid_binding) == 0);

   return can_tile && (pres->base.usage != PIPE_USAGE_STREAM);
}

static uint64_t
panfrost_best_modifier(struct panfrost_device *dev,
                       const struct panfrost_resource *pres,
                       enum pipe_format fmt)
{
   /* Force linear textures when debugging tiling/compression */
   if (unlikely(dev->debug & PAN_DBG_LINEAR))
      return DRM_FORMAT_MOD_LINEAR;

   if (panfrost_should_afbc(dev, pres, fmt)) {
      uint64_t afbc = AFBC_FORMAT_MOD_BLOCK_SIZE_16x16 | AFBC_FORMAT_MOD_SPARSE;

      if (panfrost_afbc_can_ytr(pres->base.format))
         afbc |= AFBC_FORMAT_MOD_YTR;

      if (panfrost_should_tile_afbc(dev, pres))
         afbc |= AFBC_FORMAT_MOD_TILED | AFBC_FORMAT_MOD_SC;

      return DRM_FORMAT_MOD_ARM_AFBC(afbc);
   } else if (panfrost_should_tile(dev, pres, fmt))
      return DRM_FORMAT_MOD_ARM_16X16_BLOCK_U_INTERLEAVED;
   else
      return DRM_FORMAT_MOD_LINEAR;
}

static bool
panfrost_should_checksum(const struct panfrost_device *dev,
                         const struct panfrost_resource *pres)
{
   /* Checksumming is disabled by default due to fundamental unsoundness */
   if (!(dev->debug & PAN_DBG_CRC))
      return false;

   /* When checksumming is enabled, the tile data must fit in the
    * size of the writeback buffer, so don't checksum formats
    * that use too much space. */

   unsigned bytes_per_pixel_max = (dev->arch == 6) ? 6 : 4;

   unsigned bytes_per_pixel = MAX2(pres->base.nr_samples, 1) *
                              util_format_get_blocksize(pres->base.format);

   return pres->base.bind & PIPE_BIND_RENDER_TARGET && panfrost_is_2d(pres) &&
          bytes_per_pixel <= bytes_per_pixel_max && pres->base.last_level == 0;
}

static void
panfrost_resource_setup(struct panfrost_device *dev,
                        struct panfrost_resource *pres, uint64_t modifier,
                        enum pipe_format fmt)
{
   uint64_t chosen_mod = modifier != DRM_FORMAT_MOD_INVALID
                            ? modifier
                            : panfrost_best_modifier(dev, pres, fmt);
   enum mali_texture_dimension dim =
      panfrost_translate_texture_dimension(pres->base.target);

   /* We can only switch tiled->linear if the resource isn't already
    * linear and if we control the modifier */
   pres->modifier_constant = !(chosen_mod != DRM_FORMAT_MOD_LINEAR &&
                               modifier == DRM_FORMAT_MOD_INVALID);

   /* Z32_S8X24 variants are actually stored in 2 planes (one per
    * component), we have to adjust the format on the first plane.
    */
   if (fmt == PIPE_FORMAT_Z32_FLOAT_S8X24_UINT)
      fmt = PIPE_FORMAT_Z32_FLOAT;

   pres->image.layout = (struct pan_image_layout){
      .modifier = chosen_mod,
      .format = fmt,
      .dim = dim,
      .width = pres->base.width0,
      .height = pres->base.height0,
      .depth = pres->base.depth0,
      .array_size = pres->base.array_size,
      .nr_samples = MAX2(pres->base.nr_samples, 1),
      .nr_slices = pres->base.last_level + 1,
      .crc = panfrost_should_checksum(dev, pres),
   };

   ASSERTED bool valid = pan_image_layout_init(dev, &pres->image.layout, NULL);
   assert(valid);
}

static void
panfrost_resource_init_afbc_headers(struct panfrost_resource *pres)
{
   panfrost_bo_mmap(pres->image.data.bo);

   unsigned nr_samples = MAX2(pres->base.nr_samples, 1);

   for (unsigned i = 0; i < pres->base.array_size; ++i) {
      for (unsigned l = 0; l <= pres->base.last_level; ++l) {
         struct pan_image_slice_layout *slice = &pres->image.layout.slices[l];

         for (unsigned s = 0; s < nr_samples; ++s) {
            void *ptr = pres->image.data.bo->ptr.cpu +
                        (i * pres->image.layout.array_stride) + slice->offset +
                        (s * slice->afbc.surface_stride);

            /* Zero-ed AFBC headers seem to encode a plain
             * black. Let's use this pattern to keep the
             * initialization simple.
             */
            memset(ptr, 0, slice->afbc.header_size);
         }
      }
   }
}

void
panfrost_resource_set_damage_region(struct pipe_screen *screen,
                                    struct pipe_resource *res,
                                    unsigned int nrects,
                                    const struct pipe_box *rects)
{
   struct panfrost_device *dev = pan_device(screen);
   struct panfrost_resource *pres = pan_resource(res);
   struct pipe_scissor_state *damage_extent = &pres->damage.extent;
   unsigned int i;

   /* Partial updates are implemented with a tile enable map only on v5.
    * Later architectures have a more efficient method of implementing
    * partial updates (frame shaders), while earlier architectures lack
    * tile enable maps altogether.
    */
   if (dev->arch == 5 && nrects > 1) {
      if (!pres->damage.tile_map.data) {
         pres->damage.tile_map.stride =
            ALIGN_POT(DIV_ROUND_UP(res->width0, 32 * 8), 64);
         pres->damage.tile_map.size =
            pres->damage.tile_map.stride * DIV_ROUND_UP(res->height0, 32);
         pres->damage.tile_map.data = malloc(pres->damage.tile_map.size);
      }

      memset(pres->damage.tile_map.data, 0, pres->damage.tile_map.size);
      pres->damage.tile_map.enable = true;
   } else {
      pres->damage.tile_map.enable = false;
   }

   /* Track the damage extent: the quad including all damage regions. Will
    * be used restrict the rendering area */

   damage_extent->minx = 0xffff;
   damage_extent->miny = 0xffff;

   unsigned enable_count = 0;

   for (i = 0; i < nrects; i++) {
      int x = rects[i].x, w = rects[i].width, h = rects[i].height;
      int y = res->height0 - (rects[i].y + h);

      damage_extent->minx = MIN2(damage_extent->minx, x);
      damage_extent->miny = MIN2(damage_extent->miny, y);
      damage_extent->maxx = MAX2(damage_extent->maxx, MIN2(x + w, res->width0));
      damage_extent->maxy =
         MAX2(damage_extent->maxy, MIN2(y + h, res->height0));

      if (!pres->damage.tile_map.enable)
         continue;

      unsigned t_x_start = x / 32;
      unsigned t_x_end = (x + w - 1) / 32;
      unsigned t_y_start = y / 32;
      unsigned t_y_end = (y + h - 1) / 32;

      for (unsigned t_y = t_y_start; t_y <= t_y_end; t_y++) {
         for (unsigned t_x = t_x_start; t_x <= t_x_end; t_x++) {
            unsigned b = (t_y * pres->damage.tile_map.stride * 8) + t_x;

            if (BITSET_TEST(pres->damage.tile_map.data, b))
               continue;

            BITSET_SET(pres->damage.tile_map.data, b);
            enable_count++;
         }
      }
   }

   if (nrects == 0) {
      damage_extent->minx = 0;
      damage_extent->miny = 0;
      damage_extent->maxx = res->width0;
      damage_extent->maxy = res->height0;
   }

   if (pres->damage.tile_map.enable) {
      unsigned t_x_start = damage_extent->minx / 32;
      unsigned t_x_end = damage_extent->maxx / 32;
      unsigned t_y_start = damage_extent->miny / 32;
      unsigned t_y_end = damage_extent->maxy / 32;
      unsigned tile_count =
         (t_x_end - t_x_start + 1) * (t_y_end - t_y_start + 1);

      /* Don't bother passing a tile-enable-map if the amount of
       * tiles to reload is to close to the total number of tiles.
       */
      if (tile_count - enable_count < 10)
         pres->damage.tile_map.enable = false;
   }
}

struct pipe_resource *
panfrost_resource_create_with_modifier(struct pipe_screen *screen,
                                       const struct pipe_resource *template,
                                       uint64_t modifier)
{
   struct panfrost_device *dev = pan_device(screen);

   struct panfrost_resource *so = CALLOC_STRUCT(panfrost_resource);
   so->base = *template;
   so->base.screen = screen;

   pipe_reference_init(&so->base.reference, 1);

   util_range_init(&so->valid_buffer_range);

   if (template->bind & PAN_BIND_SHARED_MASK) {
      /* For compatibility with older consumers that may not be
       * modifiers aware, treat INVALID as LINEAR for shared
       * resources.
       */
      if (modifier == DRM_FORMAT_MOD_INVALID)
         modifier = DRM_FORMAT_MOD_LINEAR;

      /* At any rate, we can't change the modifier later for shared
       * resources, since we have no way to propagate the modifier
       * change.
       */
      so->modifier_constant = true;
   }

   panfrost_resource_setup(dev, so, modifier, template->format);

   /* Guess a label based on the bind */
   unsigned bind = template->bind;
   const char *label = (bind & PIPE_BIND_INDEX_BUFFER)     ? "Index buffer"
                       : (bind & PIPE_BIND_SCANOUT)        ? "Scanout"
                       : (bind & PIPE_BIND_DISPLAY_TARGET) ? "Display target"
                       : (bind & PIPE_BIND_SHARED)         ? "Shared resource"
                       : (bind & PIPE_BIND_RENDER_TARGET)  ? "Render target"
                       : (bind & PIPE_BIND_DEPTH_STENCIL)
                          ? "Depth/stencil buffer"
                       : (bind & PIPE_BIND_SAMPLER_VIEW)    ? "Texture"
                       : (bind & PIPE_BIND_VERTEX_BUFFER)   ? "Vertex buffer"
                       : (bind & PIPE_BIND_CONSTANT_BUFFER) ? "Constant buffer"
                       : (bind & PIPE_BIND_GLOBAL)          ? "Global memory"
                       : (bind & PIPE_BIND_SHADER_BUFFER)   ? "Shader buffer"
                       : (bind & PIPE_BIND_SHADER_IMAGE)    ? "Shader image"
                                                            : "Other resource";

   if (dev->ro && (template->bind & PIPE_BIND_SCANOUT)) {
      struct winsys_handle handle;
      struct pan_block_size blocksize =
         panfrost_block_size(modifier, template->format);

      /* Block-based texture formats are only used for texture
       * compression (not framebuffer compression!), which doesn't
       * make sense to share across processes.
       */
      assert(util_format_get_blockwidth(template->format) == 1);
      assert(util_format_get_blockheight(template->format) == 1);

      /* Present a resource with similar dimensions that, if allocated
       * as a linear image, is big enough to fit the resource in the
       * actual layout. For linear images, this is a no-op. For 16x16
       * tiling, this aligns the dimensions to 16x16.
       *
       * For AFBC, this aligns the width to the superblock width (as
       * expected) and adds extra rows to account for the header. This
       * is a bit of a lie, but it's the best we can do with dumb
       * buffers, which are extremely not meant for AFBC. And yet this
       * has to work anyway...
       *
       * Moral of the story: if you're reading this comment, that
       * means you're working on WSI and so it's already too late for
       * you. I'm sorry.
       */
      unsigned width = ALIGN_POT(template->width0, blocksize.width);
      unsigned stride = ALIGN_POT(template->width0, blocksize.width) *
                        util_format_get_blocksize(template->format);
      unsigned size = so->image.layout.data_size;
      unsigned effective_rows = DIV_ROUND_UP(size, stride);

      struct pipe_resource scanout_tmpl = {
         .target = so->base.target,
         .format = template->format,
         .width0 = width,
         .height0 = effective_rows,
         .depth0 = 1,
         .array_size = 1,
      };

      so->scanout =
         renderonly_scanout_for_resource(&scanout_tmpl, dev->ro, &handle);

      if (!so->scanout) {
         fprintf(stderr, "Failed to create scanout resource\n");
         free(so);
         return NULL;
      }
      assert(handle.type == WINSYS_HANDLE_TYPE_FD);
      so->image.data.bo = panfrost_bo_import(dev, handle.handle);
      close(handle.handle);

      if (!so->image.data.bo) {
         free(so);
         return NULL;
      }
   } else {
      /* We create a BO immediately but don't bother mapping, since we don't
       * care to map e.g. FBOs which the CPU probably won't touch */
      uint32_t flags = PAN_BO_DELAY_MMAP;

      /* If the resource is never exported, we can make the BO private. */
      if (template->bind & PIPE_BIND_SHARED)
         flags |= PAN_BO_SHAREABLE;

      so->image.data.bo =
         panfrost_bo_create(dev, so->image.layout.data_size, flags, label);

      so->constant_stencil = true;
   }

   if (drm_is_afbc(so->image.layout.modifier))
      panfrost_resource_init_afbc_headers(so);

   panfrost_resource_set_damage_region(screen, &so->base, 0, NULL);

   if (template->bind & PIPE_BIND_INDEX_BUFFER)
      so->index_cache = CALLOC_STRUCT(panfrost_minmax_cache);

   return (struct pipe_resource *)so;
}

/* Default is to create a resource as don't care */

static struct pipe_resource *
panfrost_resource_create(struct pipe_screen *screen,
                         const struct pipe_resource *template)
{
   return panfrost_resource_create_with_modifier(screen, template,
                                                 DRM_FORMAT_MOD_INVALID);
}

/* If no modifier is specified, we'll choose. Otherwise, the order of
 * preference is compressed, tiled, linear. */

static struct pipe_resource *
panfrost_resource_create_with_modifiers(struct pipe_screen *screen,
                                        const struct pipe_resource *template,
                                        const uint64_t *modifiers, int count)
{
   for (unsigned i = 0; i < PAN_MODIFIER_COUNT; ++i) {
      if (drm_find_modifier(pan_best_modifiers[i], modifiers, count)) {
         return panfrost_resource_create_with_modifier(screen, template,
                                                       pan_best_modifiers[i]);
      }
   }

   /* If we didn't find one, app specified invalid */
   assert(count == 1 && modifiers[0] == DRM_FORMAT_MOD_INVALID);
   return panfrost_resource_create(screen, template);
}

static void
panfrost_resource_destroy(struct pipe_screen *screen, struct pipe_resource *pt)
{
   struct panfrost_device *dev = pan_device(screen);
   struct panfrost_resource *rsrc = (struct panfrost_resource *)pt;

   if (rsrc->scanout)
      renderonly_scanout_destroy(rsrc->scanout, dev->ro);

   if (rsrc->image.data.bo)
      panfrost_bo_unreference(rsrc->image.data.bo);

   free(rsrc->index_cache);
   free(rsrc->damage.tile_map.data);

   util_range_destroy(&rsrc->valid_buffer_range);
   free(rsrc);
}

/* Most of the time we can do CPU-side transfers, but sometimes we need to use
 * the 3D pipe for this. Let's wrap u_blitter to blit to/from staging textures.
 * Code adapted from freedreno */

static struct panfrost_resource *
pan_alloc_staging(struct panfrost_context *ctx, struct panfrost_resource *rsc,
                  unsigned level, const struct pipe_box *box)
{
   struct pipe_context *pctx = &ctx->base;
   struct pipe_resource tmpl = rsc->base;

   tmpl.width0 = box->width;
   tmpl.height0 = box->height;
   /* for array textures, box->depth is the array_size, otherwise
    * for 3d textures, it is the depth:
    */
   if (tmpl.array_size > 1) {
      if (tmpl.target == PIPE_TEXTURE_CUBE)
         tmpl.target = PIPE_TEXTURE_2D_ARRAY;
      tmpl.array_size = box->depth;
      tmpl.depth0 = 1;
   } else {
      tmpl.array_size = 1;
      tmpl.depth0 = box->depth;
   }
   tmpl.last_level = 0;
   tmpl.bind |= PIPE_BIND_LINEAR;
   tmpl.bind &= ~PAN_BIND_SHARED_MASK;

   struct pipe_resource *pstaging =
      pctx->screen->resource_create(pctx->screen, &tmpl);
   if (!pstaging)
      return NULL;

   return pan_resource(pstaging);
}

static void
pan_blit_from_staging(struct pipe_context *pctx,
                      struct panfrost_transfer *trans)
{
   struct pipe_resource *dst = trans->base.resource;
   struct pipe_blit_info blit = {0};

   blit.dst.resource = dst;
   blit.dst.format = dst->format;
   blit.dst.level = trans->base.level;
   blit.dst.box = trans->base.box;
   blit.src.resource = trans->staging.rsrc;
   blit.src.format = trans->staging.rsrc->format;
   blit.src.level = 0;
   blit.src.box = trans->staging.box;
   blit.mask = util_format_get_mask(blit.src.format);
   blit.filter = PIPE_TEX_FILTER_NEAREST;

   panfrost_blit_no_afbc_legalization(pctx, &blit);
}

static void
pan_blit_to_staging(struct pipe_context *pctx, struct panfrost_transfer *trans)
{
   struct pipe_resource *src = trans->base.resource;
   struct pipe_blit_info blit = {0};

   blit.src.resource = src;
   blit.src.format = src->format;
   blit.src.level = trans->base.level;
   blit.src.box = trans->base.box;
   blit.dst.resource = trans->staging.rsrc;
   blit.dst.format = trans->staging.rsrc->format;
   blit.dst.level = 0;
   blit.dst.box = trans->staging.box;
   blit.mask = util_format_get_mask(blit.dst.format);
   blit.filter = PIPE_TEX_FILTER_NEAREST;

   panfrost_blit_no_afbc_legalization(pctx, &blit);
}

static void
panfrost_load_tiled_images(struct panfrost_transfer *transfer,
                           struct panfrost_resource *rsrc)
{
   struct pipe_transfer *ptrans = &transfer->base;
   unsigned level = ptrans->level;

   /* If the requested level of the image is uninitialized, it's not
    * necessary to copy it. Leave the result unintiialized too.
    */
   if (!BITSET_TEST(rsrc->valid.data, level))
      return;

   struct panfrost_bo *bo = rsrc->image.data.bo;
   unsigned stride = panfrost_get_layer_stride(&rsrc->image.layout, level);

   /* Otherwise, load each layer separately, required to load from 3D and
    * array textures.
    */
   for (unsigned z = 0; z < ptrans->box.depth; ++z) {
      void *dst = transfer->map + (ptrans->layer_stride * z);
      uint8_t *map = bo->ptr.cpu + rsrc->image.layout.slices[level].offset +
                     (z + ptrans->box.z) * stride;

      panfrost_load_tiled_image(dst, map, ptrans->box.x, ptrans->box.y,
                                ptrans->box.width, ptrans->box.height,
                                ptrans->stride,
                                rsrc->image.layout.slices[level].row_stride,
                                rsrc->image.layout.format);
   }
}

#ifdef DEBUG

static unsigned
get_superblock_size(uint32_t *hdr, unsigned uncompressed_size)
{
   /* AFBC superblock layout 0 */
   unsigned body_base_ptr_len = 32;
   unsigned nr_subblocks = 16;
   unsigned sz_len = 6; /* bits */
   unsigned mask = (1 << sz_len) - 1;
   unsigned size = 0;

   /* Sum up all of the subblock sizes */
   for (int i = 0; i < nr_subblocks; i++) {
      unsigned bitoffset = body_base_ptr_len + (i * sz_len);
      unsigned start = bitoffset / 32;
      unsigned end = (bitoffset + (sz_len - 1)) / 32;
      unsigned offset = bitoffset % 32;
      unsigned subblock_size;

      if (start != end)
         subblock_size = (hdr[start] >> offset) | (hdr[end] << (32 - offset));
      else
         subblock_size = hdr[start] >> offset;
      subblock_size = (subblock_size == 1) ? uncompressed_size : subblock_size;
      size += subblock_size & mask;

      if (i == 0 && size == 0)
         return 0;
   }

   return size;
}

static void
dump_block(struct panfrost_resource *rsrc, uint32_t idx)
{
   panfrost_bo_wait(rsrc->image.data.bo, INT64_MAX, false);

   uint8_t *ptr = rsrc->image.data.bo->ptr.cpu;
   uint32_t *header = (uint32_t *)(ptr + (idx * AFBC_HEADER_BYTES_PER_TILE));
   uint32_t body_base_ptr = header[0];
   uint32_t *body = (uint32_t *)(ptr + body_base_ptr);
   struct pan_block_size block_sz =
      panfrost_afbc_subblock_size(rsrc->image.layout.modifier);
   unsigned pixel_sz = util_format_get_blocksize(rsrc->base.format);
   unsigned uncompressed_size = pixel_sz * block_sz.width * block_sz.height;
   unsigned size = get_superblock_size(header, uncompressed_size);

   fprintf(stderr, "  Header: %08x %08x %08x %08x (size: %u bytes)\n",
           header[0], header[1], header[2], header[3], size);
   if (size > 0) {
      fprintf(stderr, "  Body:   %08x %08x %08x %08x\n", body[0], body[1],
              body[2], body[3]);
   } else {
      uint8_t *comp = (uint8_t *)(header + 2);
      fprintf(stderr, "  Color:  0x%02x%02x%02x%02x\n", comp[0], comp[1],
              comp[2], comp[3]);
   }
   fprintf(stderr, "\n");
}

void
pan_dump_resource(struct panfrost_context *ctx, struct panfrost_resource *rsc)
{
   struct pipe_context *pctx = &ctx->base;
   struct pipe_resource tmpl = rsc->base;
   struct pipe_resource *plinear = NULL;
   struct panfrost_resource *linear = rsc;
   struct pipe_blit_info blit = {0};
   struct pipe_box box;
   char buffer[1024];

   if (rsc->image.layout.modifier != DRM_FORMAT_MOD_LINEAR) {
      tmpl.bind |= PIPE_BIND_LINEAR;
      tmpl.bind &= ~PAN_BIND_SHARED_MASK;

      plinear = pctx->screen->resource_create(pctx->screen, &tmpl);
      u_box_2d(0, 0, rsc->base.width0, rsc->base.height0, &box);

      blit.src.resource = &rsc->base;
      blit.src.format = rsc->base.format;
      blit.src.level = 0;
      blit.src.box = box;
      blit.dst.resource = plinear;
      blit.dst.format = rsc->base.format;
      blit.dst.level = 0;
      blit.dst.box = box;
      blit.mask = util_format_get_mask(blit.dst.format);
      blit.filter = PIPE_TEX_FILTER_NEAREST;

      panfrost_blit(pctx, &blit);

      linear = pan_resource(plinear);
   }

   panfrost_flush_writer(ctx, linear, "dump image");
   panfrost_bo_wait(linear->image.data.bo, INT64_MAX, false);
   panfrost_bo_mmap(linear->image.data.bo);

   static unsigned frame_count = 0;
   frame_count++;
   snprintf(buffer, sizeof(buffer), "dump_image.%04d", frame_count);

   debug_dump_image(buffer, rsc->base.format, 0 /* UNUSED */, rsc->base.width0,
                    rsc->base.height0,
                    linear->image.layout.slices[0].row_stride,
                    linear->image.data.bo->ptr.cpu);

   if (plinear)
      pipe_resource_reference(&plinear, NULL);
}

#endif

/* Get scan-order index from (x, y) position when blocks are
 * arranged in z-order in 8x8 tiles */
static unsigned
get_morton_index(unsigned x, unsigned y, unsigned stride)
{
   unsigned i = ((x << 0) & 1) | ((y << 1) & 2) | ((x << 1) & 4) |
                ((y << 2) & 8) | ((x << 2) & 16) | ((y << 3) & 32);
   return (((y & ~7) * stride) + ((x & ~7) << 3)) + i;
}

static void
panfrost_store_tiled_images(struct panfrost_transfer *transfer,
                            struct panfrost_resource *rsrc)
{
   struct panfrost_bo *bo = rsrc->image.data.bo;
   struct pipe_transfer *ptrans = &transfer->base;
   unsigned level = ptrans->level;
   unsigned stride = panfrost_get_layer_stride(&rsrc->image.layout, level);

   /* Otherwise, store each layer separately, required to store to 3D and
    * array textures.
    */
   for (unsigned z = 0; z < ptrans->box.depth; ++z) {
      void *src = transfer->map + (ptrans->layer_stride * z);
      uint8_t *map = bo->ptr.cpu + rsrc->image.layout.slices[level].offset +
                     (z + ptrans->box.z) * stride;

      panfrost_store_tiled_image(map, src, ptrans->box.x, ptrans->box.y,
                                 ptrans->box.width, ptrans->box.height,
                                 rsrc->image.layout.slices[level].row_stride,
                                 ptrans->stride, rsrc->image.layout.format);
   }
}

static bool
panfrost_box_covers_resource(const struct pipe_resource *resource,
                             const struct pipe_box *box)
{
   return resource->last_level == 0 &&
          util_texrange_covers_whole_level(resource, 0, box->x, box->y, box->z,
                                           box->width, box->height, box->depth);
}

static bool
panfrost_can_discard(struct pipe_resource *resource, const struct pipe_box *box,
                     unsigned usage)
{
   struct panfrost_resource *rsrc = pan_resource(resource);

   return ((usage & PIPE_MAP_DISCARD_RANGE) &&
           !(usage & PIPE_MAP_UNSYNCHRONIZED) &&
           !(resource->flags & PIPE_RESOURCE_FLAG_MAP_PERSISTENT) &&
           panfrost_box_covers_resource(resource, box) &&
           !(rsrc->image.data.bo->flags & PAN_BO_SHARED));
}

static void *
panfrost_ptr_map(struct pipe_context *pctx, struct pipe_resource *resource,
                 unsigned level,
                 unsigned usage, /* a combination of PIPE_MAP_x */
                 const struct pipe_box *box,
                 struct pipe_transfer **out_transfer)
{
   struct panfrost_context *ctx = pan_context(pctx);
   struct panfrost_device *dev = pan_device(pctx->screen);
   struct panfrost_resource *rsrc = pan_resource(resource);
   enum pipe_format format = rsrc->image.layout.format;
   int bytes_per_block = util_format_get_blocksize(format);
   struct panfrost_bo *bo = rsrc->image.data.bo;

   /* Can't map tiled/compressed directly */
   if ((usage & PIPE_MAP_DIRECTLY) &&
       rsrc->image.layout.modifier != DRM_FORMAT_MOD_LINEAR)
      return NULL;

   struct panfrost_transfer *transfer = rzalloc(pctx, struct panfrost_transfer);
   transfer->base.level = level;
   transfer->base.usage = usage;
   transfer->base.box = *box;

   pipe_resource_reference(&transfer->base.resource, resource);
   *out_transfer = &transfer->base;

   if (usage & PIPE_MAP_WRITE)
      rsrc->constant_stencil = false;

   /* We don't have s/w routines for AFBC, so use a staging texture */
   if (drm_is_afbc(rsrc->image.layout.modifier)) {
      struct panfrost_resource *staging =
         pan_alloc_staging(ctx, rsrc, level, box);
      assert(staging);

      /* Staging resources have one LOD: level 0. Query the strides
       * on this LOD.
       */
      transfer->base.stride = staging->image.layout.slices[0].row_stride;
      transfer->base.layer_stride =
         panfrost_get_layer_stride(&staging->image.layout, 0);

      transfer->staging.rsrc = &staging->base;

      transfer->staging.box = *box;
      transfer->staging.box.x = 0;
      transfer->staging.box.y = 0;
      transfer->staging.box.z = 0;

      assert(transfer->staging.rsrc != NULL);

      bool valid = BITSET_TEST(rsrc->valid.data, level);

      if ((usage & PIPE_MAP_READ) &&
          (valid || panfrost_any_batch_writes_rsrc(ctx, rsrc))) {
         pan_blit_to_staging(pctx, transfer);
         panfrost_flush_writer(ctx, staging, "AFBC read staging blit");
         panfrost_bo_wait(staging->image.data.bo, INT64_MAX, false);
      }

      panfrost_bo_mmap(staging->image.data.bo);
      return staging->image.data.bo->ptr.cpu;
   }

   /* If we haven't already mmaped, now's the time */
   panfrost_bo_mmap(bo);

   if (dev->debug & (PAN_DBG_TRACE | PAN_DBG_SYNC)) {
      pandecode_inject_mmap(dev->decode_ctx, bo->ptr.gpu, bo->ptr.cpu,
                            panfrost_bo_size(bo), NULL);
   }

   /* Upgrade writes to uninitialized ranges to UNSYNCHRONIZED */
   if ((usage & PIPE_MAP_WRITE) && resource->target == PIPE_BUFFER &&
       !util_ranges_intersect(&rsrc->valid_buffer_range, box->x,
                              box->x + box->width)) {

      usage |= PIPE_MAP_UNSYNCHRONIZED;
   }

   /* Upgrade DISCARD_RANGE to WHOLE_RESOURCE if the whole resource is
    * being mapped.
    */
   if (panfrost_can_discard(resource, box, usage)) {
      usage |= PIPE_MAP_DISCARD_WHOLE_RESOURCE;
   }

   bool create_new_bo = usage & PIPE_MAP_DISCARD_WHOLE_RESOURCE;
   bool copy_resource = false;

   if (!create_new_bo && !(usage & PIPE_MAP_UNSYNCHRONIZED) &&
       !(resource->flags & PIPE_RESOURCE_FLAG_MAP_PERSISTENT) &&
       (usage & PIPE_MAP_WRITE) && panfrost_any_batch_reads_rsrc(ctx, rsrc)) {
      /* When a resource to be modified is already being used by a
       * pending batch, it is often faster to copy the whole BO than
       * to flush and split the frame in two.
       */

      panfrost_flush_writer(ctx, rsrc, "Shadow resource creation");
      panfrost_bo_wait(bo, INT64_MAX, false);

      create_new_bo = true;
      copy_resource = !(usage & PIPE_MAP_DISCARD_WHOLE_RESOURCE);
   }

   /* Shadowing with separate stencil may require additional accounting.
    * Bail in these exotic cases.
    */
   if (rsrc->separate_stencil) {
      create_new_bo = false;
      copy_resource = false;
   }

   if (create_new_bo) {
      /* Make sure we re-emit any descriptors using this resource */
      panfrost_dirty_state_all(ctx);

      /* If the BO is used by one of the pending batches or if it's
       * not ready yet (still accessed by one of the already flushed
       * batches), we try to allocate a new one to avoid waiting.
       */
      if (panfrost_any_batch_reads_rsrc(ctx, rsrc) ||
          !panfrost_bo_wait(bo, 0, true)) {
         /* We want the BO to be MMAPed. */
         uint32_t flags = bo->flags & ~PAN_BO_DELAY_MMAP;
         struct panfrost_bo *newbo = NULL;

         /* When the BO has been imported/exported, we can't
          * replace it by another one, otherwise the
          * importer/exporter wouldn't see the change we're
          * doing to it.
          */
         if (!(bo->flags & PAN_BO_SHARED)) {
            newbo =
               panfrost_bo_create(dev, panfrost_bo_size(bo), flags, bo->label);
         }

         if (newbo) {
            if (copy_resource) {
               memcpy(newbo->ptr.cpu, rsrc->image.data.bo->ptr.cpu,
                      panfrost_bo_size(bo));
            }

            /* Swap the pointers, dropping a reference to
             * the old BO which is no long referenced from
             * the resource.
             */
            panfrost_bo_unreference(rsrc->image.data.bo);
            rsrc->image.data.bo = newbo;

            if (!copy_resource && drm_is_afbc(rsrc->image.layout.modifier))
               panfrost_resource_init_afbc_headers(rsrc);

            bo = newbo;
         } else {
            /* Allocation failed or was impossible, let's
             * fall back on a flush+wait.
             */
            panfrost_flush_batches_accessing_rsrc(
               ctx, rsrc, "Resource access with high memory pressure");
            panfrost_bo_wait(bo, INT64_MAX, true);
         }
      }
   } else if (!(usage & PIPE_MAP_UNSYNCHRONIZED)) {
      if (usage & PIPE_MAP_WRITE) {
         panfrost_flush_batches_accessing_rsrc(ctx, rsrc, "Synchronized write");
         panfrost_bo_wait(bo, INT64_MAX, true);
      } else if (usage & PIPE_MAP_READ) {
         panfrost_flush_writer(ctx, rsrc, "Synchronized read");
         panfrost_bo_wait(bo, INT64_MAX, false);
      }
   }

   /* For access to compressed textures, we want the (x, y, w, h)
    * region-of-interest in blocks, not pixels. Then we compute the stride
    * between rows of blocks as the width in blocks times the width per
    * block, etc.
    */
   struct pipe_box box_blocks;
   u_box_pixels_to_blocks(&box_blocks, box, format);

   if (rsrc->image.layout.modifier ==
       DRM_FORMAT_MOD_ARM_16X16_BLOCK_U_INTERLEAVED) {
      transfer->base.stride = box_blocks.width * bytes_per_block;
      transfer->base.layer_stride = transfer->base.stride * box_blocks.height;
      transfer->map =
         ralloc_size(transfer, transfer->base.layer_stride * box->depth);

      if (usage & PIPE_MAP_READ)
         panfrost_load_tiled_images(transfer, rsrc);

      return transfer->map;
   } else {
      assert(rsrc->image.layout.modifier == DRM_FORMAT_MOD_LINEAR);

      /* Direct, persistent writes create holes in time for
       * caching... I don't know if this is actually possible but we
       * should still get it right */

      unsigned dpw = PIPE_MAP_DIRECTLY | PIPE_MAP_WRITE | PIPE_MAP_PERSISTENT;

      if ((usage & dpw) == dpw && rsrc->index_cache)
         return NULL;

      transfer->base.stride = rsrc->image.layout.slices[level].row_stride;
      transfer->base.layer_stride =
         panfrost_get_layer_stride(&rsrc->image.layout, level);

      /* By mapping direct-write, we're implicitly already
       * initialized (maybe), so be conservative */

      if (usage & PIPE_MAP_WRITE) {
         BITSET_SET(rsrc->valid.data, level);
         panfrost_minmax_cache_invalidate(rsrc->index_cache, &transfer->base);
      }

      return bo->ptr.cpu + rsrc->image.layout.slices[level].offset +
             box->z * transfer->base.layer_stride +
             box_blocks.y * rsrc->image.layout.slices[level].row_stride +
             box_blocks.x * bytes_per_block;
   }
}

void
pan_resource_modifier_convert(struct panfrost_context *ctx,
                              struct panfrost_resource *rsrc, uint64_t modifier,
                              bool copy_resource, const char *reason)
{
   assert(!rsrc->modifier_constant);

   struct pipe_resource *tmp_prsrc = panfrost_resource_create_with_modifier(
      ctx->base.screen, &rsrc->base, modifier);
   struct panfrost_resource *tmp_rsrc = pan_resource(tmp_prsrc);

   if (copy_resource) {
      struct pipe_blit_info blit = {
         .dst.resource = &tmp_rsrc->base,
         .dst.format = tmp_rsrc->base.format,
         .src.resource = &rsrc->base,
         .src.format = rsrc->base.format,
         .mask = util_format_get_mask(tmp_rsrc->base.format),
         .filter = PIPE_TEX_FILTER_NEAREST,
      };

      /* data_valid is not valid until flushed */
      panfrost_flush_writer(ctx, rsrc, "AFBC decompressing blit");

      for (int i = 0; i <= rsrc->base.last_level; i++) {
         if (BITSET_TEST(rsrc->valid.data, i)) {
            blit.dst.level = blit.src.level = i;

            u_box_3d(0, 0, 0, u_minify(rsrc->base.width0, i),
                     u_minify(rsrc->base.height0, i),
                     util_num_layers(&rsrc->base, i), &blit.dst.box);
            blit.src.box = blit.dst.box;

            panfrost_blit_no_afbc_legalization(&ctx->base, &blit);
         }
      }

      /* we lose track of tmp_rsrc after this point, and the BO migration
       * (from tmp_rsrc to rsrc) doesn't transfer the last_writer to rsrc
       */
      panfrost_flush_writer(ctx, tmp_rsrc, "AFBC decompressing blit");
   }

   panfrost_bo_unreference(rsrc->image.data.bo);

   rsrc->image.data.bo = tmp_rsrc->image.data.bo;
   panfrost_bo_reference(rsrc->image.data.bo);

   panfrost_resource_setup(pan_device(ctx->base.screen), rsrc, modifier,
                           tmp_rsrc->base.format);
   /* panfrost_resource_setup will force the modifier to stay constant when
    * called with a specific modifier. We don't want that here, we want to
    * be able to convert back to another modifier if needed */
   rsrc->modifier_constant = false;
   pipe_resource_reference(&tmp_prsrc, NULL);
}

/* Validate that an AFBC resource may be used as a particular format. If it may
 * not, decompress it on the fly. Failure to do so can produce wrong results or
 * invalid data faults when sampling or rendering to AFBC */

void
pan_legalize_afbc_format(struct panfrost_context *ctx,
                         struct panfrost_resource *rsrc,
                         enum pipe_format format, bool write, bool discard)
{
   struct panfrost_device *dev = pan_device(ctx->base.screen);

   if (!drm_is_afbc(rsrc->image.layout.modifier))
      return;

   if (panfrost_afbc_format(dev->arch, rsrc->base.format) !=
       panfrost_afbc_format(dev->arch, format)) {
      pan_resource_modifier_convert(
         ctx, rsrc, DRM_FORMAT_MOD_ARM_16X16_BLOCK_U_INTERLEAVED, !discard,
         "Reinterpreting AFBC surface as incompatible format");
      return;
   }

   if (write && (rsrc->image.layout.modifier & AFBC_FORMAT_MOD_SPARSE) == 0)
      pan_resource_modifier_convert(
         ctx, rsrc, rsrc->image.layout.modifier | AFBC_FORMAT_MOD_SPARSE,
         !discard, "Legalizing resource to allow writing");
}

static bool
panfrost_should_linear_convert(struct panfrost_device *dev,
                               struct panfrost_resource *prsrc,
                               struct pipe_transfer *transfer)
{
   if (prsrc->modifier_constant)
      return false;

   /* Overwriting the entire resource indicates streaming, for which
    * linear layout is most efficient due to the lack of expensive
    * conversion.
    *
    * For now we just switch to linear after a number of complete
    * overwrites to keep things simple, but we could do better.
    *
    * This mechanism is only implemented for 2D resources. This suffices
    * for video players, its intended use case.
    */

   bool entire_overwrite = panfrost_is_2d(prsrc) &&
                           prsrc->base.last_level == 0 &&
                           transfer->box.width == prsrc->base.width0 &&
                           transfer->box.height == prsrc->base.height0 &&
                           transfer->box.x == 0 && transfer->box.y == 0;

   if (entire_overwrite)
      ++prsrc->modifier_updates;

   if (prsrc->modifier_updates >= LAYOUT_CONVERT_THRESHOLD) {
      perf_debug(dev, "Transitioning to linear due to streaming usage");
      return true;
   } else {
      return false;
   }
}

struct panfrost_bo *
panfrost_get_afbc_superblock_sizes(struct panfrost_context *ctx,
                                   struct panfrost_resource *rsrc,
                                   unsigned first_level, unsigned last_level,
                                   unsigned *out_offsets)
{
   struct panfrost_screen *screen = pan_screen(ctx->base.screen);
   struct panfrost_device *dev = pan_device(ctx->base.screen);
   struct panfrost_batch *batch;
   struct panfrost_bo *bo;
   unsigned metadata_size = 0;

   for (int level = first_level; level <= last_level; ++level) {
      struct pan_image_slice_layout *slice = &rsrc->image.layout.slices[level];
      unsigned sz = slice->afbc.nr_blocks * sizeof(struct pan_afbc_block_info);
      out_offsets[level - first_level] = metadata_size;
      metadata_size += sz;
   }

   panfrost_flush_batches_accessing_rsrc(ctx, rsrc, "AFBC before size flush");
   batch = panfrost_get_fresh_batch_for_fbo(ctx, "AFBC superblock sizes");
   bo = panfrost_bo_create(dev, metadata_size, 0, "AFBC superblock sizes");

   for (int level = first_level; level <= last_level; ++level) {
      unsigned offset = out_offsets[level - first_level];
      screen->vtbl.afbc_size(batch, rsrc, bo, offset, level);
   }

   panfrost_flush_batches_accessing_rsrc(ctx, rsrc, "AFBC after size flush");

   return bo;
}

void
panfrost_pack_afbc(struct panfrost_context *ctx,
                   struct panfrost_resource *prsrc)
{
   struct panfrost_screen *screen = pan_screen(ctx->base.screen);
   struct panfrost_device *dev = pan_device(ctx->base.screen);
   struct panfrost_bo *metadata_bo;
   unsigned metadata_offsets[PIPE_MAX_TEXTURE_LEVELS];

   uint64_t src_modifier = prsrc->image.layout.modifier;
   uint64_t dst_modifier =
      src_modifier & ~(AFBC_FORMAT_MOD_TILED | AFBC_FORMAT_MOD_SPARSE);
   bool is_tiled = src_modifier & AFBC_FORMAT_MOD_TILED;
   unsigned last_level = prsrc->base.last_level;
   struct pan_image_slice_layout slice_infos[PIPE_MAX_TEXTURE_LEVELS] = {0};
   unsigned total_size = 0;

   /* It doesn't make sense to pack everything if we need to unpack right
    * away to upload data to another level */
   for (int i = 0; i <= last_level; i++) {
      if (!BITSET_TEST(prsrc->valid.data, i))
         return;
   }

   metadata_bo = panfrost_get_afbc_superblock_sizes(ctx, prsrc, 0, last_level,
                                                    metadata_offsets);
   panfrost_bo_wait(metadata_bo, INT64_MAX, false);

   for (unsigned level = 0; level <= last_level; ++level) {
      struct pan_image_slice_layout *src_slice =
         &prsrc->image.layout.slices[level];
      struct pan_image_slice_layout *dst_slice = &slice_infos[level];

      unsigned width = u_minify(prsrc->base.width0, level);
      unsigned height = u_minify(prsrc->base.height0, level);
      unsigned src_stride =
         pan_afbc_stride_blocks(src_modifier, src_slice->row_stride);
      unsigned dst_stride =
         DIV_ROUND_UP(width, panfrost_afbc_superblock_width(dst_modifier));
      unsigned dst_height =
         DIV_ROUND_UP(height, panfrost_afbc_superblock_height(dst_modifier));

      uint32_t offset = 0;
      struct pan_afbc_block_info *meta =
         metadata_bo->ptr.cpu + metadata_offsets[level];

      for (unsigned y = 0, i = 0; y < dst_height; ++y) {
         for (unsigned x = 0; x < dst_stride; ++x, ++i) {
            unsigned idx = is_tiled ? get_morton_index(x, y, src_stride) : i;
            uint32_t size = meta[idx].size;
            meta[idx].offset = offset; /* write the start offset */
            offset += size;
         }
      }

      total_size = ALIGN_POT(total_size, pan_slice_align(dst_modifier));
      {
         dst_slice->afbc.stride = dst_stride;
         dst_slice->afbc.nr_blocks = dst_stride * dst_height;
         dst_slice->afbc.header_size =
            ALIGN_POT(dst_stride * dst_height * AFBC_HEADER_BYTES_PER_TILE,
                      pan_afbc_body_align(dst_modifier));
         dst_slice->afbc.body_size = offset;
         dst_slice->afbc.surface_stride = dst_slice->afbc.header_size + offset;

         dst_slice->offset = total_size;
         dst_slice->row_stride = dst_stride * AFBC_HEADER_BYTES_PER_TILE;
         dst_slice->surface_stride = dst_slice->afbc.surface_stride;
         dst_slice->size = dst_slice->afbc.surface_stride;
      }
      total_size += dst_slice->afbc.surface_stride;
   }

   unsigned new_size = ALIGN_POT(total_size, 4096); // FIXME
   unsigned old_size = panfrost_bo_size(prsrc->image.data.bo);
   unsigned ratio = 100 * new_size / old_size;

   if (ratio > screen->max_afbc_packing_ratio)
      return;

   perf_debug(dev, "%i%%: %i KB -> %i KB\n", ratio, old_size / 1024,
              new_size / 1024);

   struct panfrost_bo *dst =
      panfrost_bo_create(dev, new_size, 0, "AFBC compact texture");
   struct panfrost_batch *batch =
      panfrost_get_fresh_batch_for_fbo(ctx, "AFBC compaction");

   for (unsigned level = 0; level <= last_level; ++level) {
      struct pan_image_slice_layout *slice = &slice_infos[level];
      screen->vtbl.afbc_pack(batch, prsrc, dst, slice, metadata_bo,
                             metadata_offsets[level], level);
      prsrc->image.layout.slices[level] = *slice;
   }

   panfrost_flush_batches_accessing_rsrc(ctx, prsrc, "AFBC compaction flush");

   prsrc->image.layout.modifier = dst_modifier;
   panfrost_bo_unreference(prsrc->image.data.bo);
   prsrc->image.data.bo = dst;
   panfrost_bo_unreference(metadata_bo);
}

static void
panfrost_ptr_unmap(struct pipe_context *pctx, struct pipe_transfer *transfer)
{
   /* Gallium expects writeback here, so we tile */

   struct panfrost_context *ctx = pan_context(pctx);
   struct panfrost_transfer *trans = pan_transfer(transfer);
   struct panfrost_resource *prsrc =
      (struct panfrost_resource *)transfer->resource;
   struct panfrost_device *dev = pan_device(pctx->screen);

   if (transfer->usage & PIPE_MAP_WRITE)
      prsrc->valid.crc = false;

   /* AFBC will use a staging resource. `initialized` will be set when the
    * fragment job is created; this is deferred to prevent useless surface
    * reloads that can cascade into DATA_INVALID_FAULTs due to reading
    * malformed AFBC data if uninitialized */

   if (trans->staging.rsrc) {
      if (transfer->usage & PIPE_MAP_WRITE) {
         if (panfrost_should_linear_convert(dev, prsrc, transfer)) {

            panfrost_bo_unreference(prsrc->image.data.bo);

            panfrost_resource_setup(dev, prsrc, DRM_FORMAT_MOD_LINEAR,
                                    prsrc->image.layout.format);

            prsrc->image.data.bo =
               pan_resource(trans->staging.rsrc)->image.data.bo;
            panfrost_bo_reference(prsrc->image.data.bo);
         } else {
            bool discard = panfrost_can_discard(&prsrc->base, &transfer->box,
                                                transfer->usage);
            pan_legalize_afbc_format(ctx, prsrc, prsrc->image.layout.format,
                                     true, discard);
            pan_blit_from_staging(pctx, trans);
            panfrost_flush_batches_accessing_rsrc(
               ctx, pan_resource(trans->staging.rsrc),
               "AFBC write staging blit");

            if (dev->debug & PAN_DBG_FORCE_PACK) {
               if (panfrost_should_pack_afbc(dev, prsrc))
                  panfrost_pack_afbc(ctx, prsrc);
            }
         }
      }

      pipe_resource_reference(&trans->staging.rsrc, NULL);
   }

   /* Tiling will occur in software from a staging cpu buffer */
   if (trans->map) {
      struct panfrost_bo *bo = prsrc->image.data.bo;

      if (transfer->usage & PIPE_MAP_WRITE) {
         BITSET_SET(prsrc->valid.data, transfer->level);

         if (prsrc->image.layout.modifier ==
             DRM_FORMAT_MOD_ARM_16X16_BLOCK_U_INTERLEAVED) {
            if (panfrost_should_linear_convert(dev, prsrc, transfer)) {
               panfrost_resource_setup(dev, prsrc, DRM_FORMAT_MOD_LINEAR,
                                       prsrc->image.layout.format);
               if (prsrc->image.layout.data_size > panfrost_bo_size(bo)) {
                  const char *label = bo->label;
                  panfrost_bo_unreference(bo);
                  bo = prsrc->image.data.bo = panfrost_bo_create(
                     dev, prsrc->image.layout.data_size, 0, label);
                  assert(bo);
               }

               util_copy_rect(
                  bo->ptr.cpu + prsrc->image.layout.slices[0].offset,
                  prsrc->base.format, prsrc->image.layout.slices[0].row_stride,
                  0, 0, transfer->box.width, transfer->box.height, trans->map,
                  transfer->stride, 0, 0);
            } else {
               panfrost_store_tiled_images(trans, prsrc);
            }
         }
      }
   }

   util_range_add(&prsrc->base, &prsrc->valid_buffer_range, transfer->box.x,
                  transfer->box.x + transfer->box.width);

   panfrost_minmax_cache_invalidate(prsrc->index_cache, transfer);

   /* Derefence the resource */
   pipe_resource_reference(&transfer->resource, NULL);

   /* Transfer itself is RALLOCed at the moment */
   ralloc_free(transfer);
}

static void
panfrost_ptr_flush_region(struct pipe_context *pctx,
                          struct pipe_transfer *transfer,
                          const struct pipe_box *box)
{
   struct panfrost_resource *rsc = pan_resource(transfer->resource);

   if (transfer->resource->target == PIPE_BUFFER) {
      util_range_add(&rsc->base, &rsc->valid_buffer_range,
                     transfer->box.x + box->x,
                     transfer->box.x + box->x + box->width);
   } else {
      BITSET_SET(rsc->valid.data, transfer->level);
   }
}

static void
panfrost_invalidate_resource(struct pipe_context *pctx,
                             struct pipe_resource *prsrc)
{
   struct panfrost_context *ctx = pan_context(pctx);
   struct panfrost_batch *batch = panfrost_get_batch_for_fbo(ctx);
   struct panfrost_resource *rsrc = pan_resource(prsrc);

   rsrc->constant_stencil = true;

   /* Handle the glInvalidateFramebuffer case */
   if (batch->key.zsbuf && batch->key.zsbuf->texture == prsrc)
      batch->resolve &= ~PIPE_CLEAR_DEPTHSTENCIL;

   for (unsigned i = 0; i < batch->key.nr_cbufs; ++i) {
      struct pipe_surface *surf = batch->key.cbufs[i];

      if (surf && surf->texture == prsrc)
         batch->resolve &= ~(PIPE_CLEAR_COLOR0 << i);
   }
}

static enum pipe_format
panfrost_resource_get_internal_format(struct pipe_resource *rsrc)
{
   struct panfrost_resource *prsrc = (struct panfrost_resource *)rsrc;
   return prsrc->image.layout.format;
}

void
panfrost_set_image_view_planes(struct pan_image_view *iview,
                               struct pipe_resource *texture)
{
   struct panfrost_resource *prsrc_plane = (struct panfrost_resource *)texture;

   for (int i = 0; i < MAX_IMAGE_PLANES && prsrc_plane; i++) {
      iview->planes[i] = &prsrc_plane->image;
      prsrc_plane = (struct panfrost_resource *)prsrc_plane->base.next;
   }
}

static bool
panfrost_generate_mipmap(struct pipe_context *pctx, struct pipe_resource *prsrc,
                         enum pipe_format format, unsigned base_level,
                         unsigned last_level, unsigned first_layer,
                         unsigned last_layer)
{
   struct panfrost_resource *rsrc = pan_resource(prsrc);

   perf_debug_ctx(pan_context(pctx), "Unoptimized mipmap generation");

   /* Generating a mipmap invalidates the written levels, so make that
    * explicit so we don't try to wallpaper them back and end up with
    * u_blitter recursion */

   assert(rsrc->image.data.bo);
   for (unsigned l = base_level + 1; l <= last_level; ++l)
      BITSET_CLEAR(rsrc->valid.data, l);

   /* Beyond that, we just delegate the hard stuff. */

   bool blit_res =
      util_gen_mipmap(pctx, prsrc, format, base_level, last_level, first_layer,
                      last_layer, PIPE_TEX_FILTER_LINEAR);

   return blit_res;
}

static void
panfrost_resource_set_stencil(struct pipe_resource *prsrc,
                              struct pipe_resource *stencil)
{
   pan_resource(prsrc)->separate_stencil = pan_resource(stencil);
}

static struct pipe_resource *
panfrost_resource_get_stencil(struct pipe_resource *prsrc)
{
   if (!pan_resource(prsrc)->separate_stencil)
      return NULL;

   return &pan_resource(prsrc)->separate_stencil->base;
}

static const struct u_transfer_vtbl transfer_vtbl = {
   .resource_create = panfrost_resource_create,
   .resource_destroy = panfrost_resource_destroy,
   .transfer_map = panfrost_ptr_map,
   .transfer_unmap = panfrost_ptr_unmap,
   .transfer_flush_region = panfrost_ptr_flush_region,
   .get_internal_format = panfrost_resource_get_internal_format,
   .set_stencil = panfrost_resource_set_stencil,
   .get_stencil = panfrost_resource_get_stencil,
};

void
panfrost_resource_screen_init(struct pipe_screen *pscreen)
{
   pscreen->resource_create_with_modifiers =
      panfrost_resource_create_with_modifiers;
   pscreen->resource_create = u_transfer_helper_resource_create;
   pscreen->resource_destroy = u_transfer_helper_resource_destroy;
   pscreen->resource_from_handle = panfrost_resource_from_handle;
   pscreen->resource_get_handle = panfrost_resource_get_handle;
   pscreen->resource_get_param = panfrost_resource_get_param;
   pscreen->transfer_helper = u_transfer_helper_create(
      &transfer_vtbl,
      U_TRANSFER_HELPER_SEPARATE_Z32S8 | U_TRANSFER_HELPER_MSAA_MAP);
}
void
panfrost_resource_screen_destroy(struct pipe_screen *pscreen)
{
   u_transfer_helper_destroy(pscreen->transfer_helper);
}

void
panfrost_resource_context_init(struct pipe_context *pctx)
{
   pctx->buffer_map = u_transfer_helper_transfer_map;
   pctx->buffer_unmap = u_transfer_helper_transfer_unmap;
   pctx->texture_map = u_transfer_helper_transfer_map;
   pctx->texture_unmap = u_transfer_helper_transfer_unmap;
   pctx->create_surface = panfrost_create_surface;
   pctx->surface_destroy = panfrost_surface_destroy;
   pctx->resource_copy_region = util_resource_copy_region;
   pctx->blit = panfrost_blit;
   pctx->generate_mipmap = panfrost_generate_mipmap;
   pctx->flush_resource = panfrost_flush_resource;
   pctx->invalidate_resource = panfrost_invalidate_resource;
   pctx->transfer_flush_region = u_transfer_helper_transfer_flush_region;
   pctx->buffer_subdata = u_default_buffer_subdata;
   pctx->texture_subdata = u_default_texture_subdata;
   pctx->clear_buffer = u_default_clear_buffer;
   pctx->clear_render_target = panfrost_clear_render_target;
   pctx->clear_depth_stencil = panfrost_clear_depth_stencil;
}
