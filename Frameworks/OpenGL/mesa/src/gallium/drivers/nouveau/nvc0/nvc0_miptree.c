/*
 * Copyright 2008 Ben Skeggs
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "drm-uapi/drm_fourcc.h"

#include "pipe/p_state.h"
#include "pipe/p_defines.h"
#include "frontend/drm_driver.h"
#include "util/u_inlines.h"
#include "util/format/u_format.h"

#include "nvc0/nvc0_context.h"
#include "nvc0/nvc0_resource.h"

static uint32_t
nvc0_tex_choose_tile_dims(unsigned nx, unsigned ny, unsigned nz, bool is_3d)
{
   return nv50_tex_choose_tile_dims_helper(nx, ny, nz, is_3d);
}

static uint32_t
tu102_choose_tiled_storage_type(enum pipe_format format,
                                unsigned ms,
                                bool compressed)

{
   uint32_t kind;

   switch (format) {
   case PIPE_FORMAT_Z16_UNORM:
      if (compressed)
         kind = 0x0b; // NV_MMU_PTE_KIND_Z16_COMPRESSIBLE_DISABLE_PLC
      else
         kind = 0x01; // NV_MMU_PTE_KIND_Z16
      break;
   case PIPE_FORMAT_X8Z24_UNORM:
   case PIPE_FORMAT_S8X24_UINT:
   case PIPE_FORMAT_S8_UINT_Z24_UNORM:
      if (compressed)
         kind = 0x0e; // NV_MMU_PTE_KIND_Z24S8_COMPRESSIBLE_DISABLE_PLC
      else
         kind = 0x05; // NV_MMU_PTE_KIND_Z24S8
      break;
   case PIPE_FORMAT_X24S8_UINT:
   case PIPE_FORMAT_Z24X8_UNORM:
   case PIPE_FORMAT_Z24_UNORM_S8_UINT:
      if (compressed)
         kind = 0x0c; // NV_MMU_PTE_KIND_S8Z24_COMPRESSIBLE_DISABLE_PLC
      else
         kind = 0x03; // NV_MMU_PTE_KIND_S8Z24
      break;
   case PIPE_FORMAT_X32_S8X24_UINT:
   case PIPE_FORMAT_Z32_FLOAT_S8X24_UINT:
      if (compressed)
         kind = 0x0d; // NV_MMU_PTE_KIND_ZF32_X24S8_COMPRESSIBLE_DISABLE_PLC
      else
         kind = 0x04; // NV_MMU_PTE_KIND_ZF32_X24S8
      break;
   case PIPE_FORMAT_Z32_FLOAT:
   default:
      kind = 0x06;
      break;
   }

   return kind;
}

uint32_t
nvc0_choose_tiled_storage_type(struct pipe_screen *pscreen,
                               enum pipe_format format,
                               unsigned ms,
                               bool compressed)
{
   uint32_t tile_flags;

   if (nouveau_screen(pscreen)->device->chipset >= 0x160)
      return tu102_choose_tiled_storage_type(format, ms, compressed);

   switch (format) {
   case PIPE_FORMAT_Z16_UNORM:
      if (compressed)
         tile_flags = 0x02 + ms;
      else
         tile_flags = 0x01;
      break;
   case PIPE_FORMAT_X8Z24_UNORM:
   case PIPE_FORMAT_S8X24_UINT:
   case PIPE_FORMAT_S8_UINT_Z24_UNORM:
      if (compressed)
         tile_flags = 0x51 + ms;
      else
         tile_flags = 0x46;
      break;
   case PIPE_FORMAT_X24S8_UINT:
   case PIPE_FORMAT_Z24X8_UNORM:
   case PIPE_FORMAT_Z24_UNORM_S8_UINT:
      if (compressed)
         tile_flags = 0x17 + ms;
      else
         tile_flags = 0x11;
      break;
   case PIPE_FORMAT_Z32_FLOAT:
      if (compressed)
         tile_flags = 0x86 + ms;
      else
         tile_flags = 0x7b;
      break;
   case PIPE_FORMAT_X32_S8X24_UINT:
   case PIPE_FORMAT_Z32_FLOAT_S8X24_UINT:
      if (compressed)
         tile_flags = 0xce + ms;
      else
         tile_flags = 0xc3;
      break;
   default:
      switch (util_format_get_blocksizebits(format)) {
      case 128:
         if (compressed)
            tile_flags = 0xf4 + ms * 2;
         else
            tile_flags = 0xfe;
         break;
      case 64:
         if (compressed) {
            switch (ms) {
            case 0: tile_flags = 0xe6; break;
            case 1: tile_flags = 0xeb; break;
            case 2: tile_flags = 0xed; break;
            case 3: tile_flags = 0xf2; break;
            default:
               return 0;
            }
         } else {
            tile_flags = 0xfe;
         }
         break;
      case 32:
         if (compressed && ms) {
            switch (ms) {
               /* This one makes things blurry:
            case 0: tile_flags = 0xdb; break;
               */
            case 1: tile_flags = 0xdd; break;
            case 2: tile_flags = 0xdf; break;
            case 3: tile_flags = 0xe4; break;
            default:
               return 0;
            }
         } else {
            tile_flags = 0xfe;
         }
         break;
      case 16:
      case 8:
         tile_flags = 0xfe;
         break;
      default:
         return 0;
      }
      break;
   }

   return tile_flags;
}

static uint32_t
nvc0_mt_choose_storage_type(struct pipe_screen *pscreen,
                            const struct nv50_miptree *mt,
                            bool compressed)
{
   const unsigned ms = util_logbase2(mt->base.base.nr_samples);

   if (unlikely(mt->base.base.bind & PIPE_BIND_CURSOR))
      return 0;
   if (unlikely(mt->base.base.flags & NOUVEAU_RESOURCE_FLAG_LINEAR))
      return 0;

   return nvc0_choose_tiled_storage_type(pscreen, mt->base.base.format, ms, compressed);
}

static inline bool
nvc0_miptree_init_ms_mode(struct nv50_miptree *mt)
{
   switch (mt->base.base.nr_samples) {
   case 8:
      mt->ms_mode = NVC0_3D_MULTISAMPLE_MODE_MS8;
      mt->ms_x = 2;
      mt->ms_y = 1;
      break;
   case 4:
      mt->ms_mode = NVC0_3D_MULTISAMPLE_MODE_MS4;
      mt->ms_x = 1;
      mt->ms_y = 1;
      break;
   case 2:
      mt->ms_mode = NVC0_3D_MULTISAMPLE_MODE_MS2;
      mt->ms_x = 1;
      break;
   case 1:
   case 0:
      mt->ms_mode = NVC0_3D_MULTISAMPLE_MODE_MS1;
      break;
   default:
      NOUVEAU_ERR("invalid nr_samples: %u\n", mt->base.base.nr_samples);
      return false;
   }
   return true;
}

static void
nvc0_miptree_init_layout_video(struct nv50_miptree *mt)
{
   const struct pipe_resource *pt = &mt->base.base;
   const unsigned blocksize = util_format_get_blocksize(pt->format);

   assert(pt->last_level == 0);
   assert(mt->ms_x == 0 && mt->ms_y == 0);
   assert(!util_format_is_compressed(pt->format));

   mt->layout_3d = pt->target == PIPE_TEXTURE_3D;

   mt->level[0].tile_mode = 0x10;
   mt->level[0].pitch = align(pt->width0 * blocksize, 64);
   mt->total_size = align(pt->height0, 16) * mt->level[0].pitch * (mt->layout_3d ? pt->depth0 : 1);

   if (pt->array_size > 1) {
      mt->layer_stride = align(mt->total_size, NVC0_TILE_SIZE(0x10));
      mt->total_size = mt->layer_stride * pt->array_size;
   }
}

static void
nvc0_miptree_init_layout_tiled(struct nv50_miptree *mt, uint64_t modifier)
{
   struct pipe_resource *pt = &mt->base.base;
   unsigned w, h, d, l;
   const unsigned blocksize = util_format_get_blocksize(pt->format);

   mt->layout_3d = pt->target == PIPE_TEXTURE_3D;

   w = pt->width0 << mt->ms_x;
   h = pt->height0 << mt->ms_y;

   /* For 3D textures, a mipmap is spanned by all the layers, for array
    * textures and cube maps, each layer contains its own mipmaps.
    */
   d = mt->layout_3d ? pt->depth0 : 1;

   assert(!mt->ms_mode || !pt->last_level);
   assert(modifier == DRM_FORMAT_MOD_INVALID ||
          (!pt->last_level && !mt->layout_3d));
   assert(modifier != DRM_FORMAT_MOD_LINEAR);

   for (l = 0; l <= pt->last_level; ++l) {
      struct nv50_miptree_level *lvl = &mt->level[l];
      unsigned tsx, tsy, tsz;
      unsigned nbx = util_format_get_nblocksx(pt->format, w);
      unsigned nby = util_format_get_nblocksy(pt->format, h);

      lvl->offset = mt->total_size;

      if (modifier != DRM_FORMAT_MOD_INVALID)
         /* Extract the log2(block height) field from the modifier and pack it
          * into tile_mode's y field. Other tile dimensions are always 1
          * (represented using 0 here) for 2D surfaces, and non-2D surfaces are
          * not supported by the current modifiers (asserted above). Note the
          * modifier must be validated prior to calling this function.
          */
         lvl->tile_mode = ((uint32_t)modifier & 0xf) << 4;
      else
         lvl->tile_mode = nvc0_tex_choose_tile_dims(nbx, nby, d, mt->layout_3d);

      tsx = NVC0_TILE_SIZE_X(lvl->tile_mode); /* x is tile row pitch in bytes */
      tsy = NVC0_TILE_SIZE_Y(lvl->tile_mode);
      tsz = NVC0_TILE_SIZE_Z(lvl->tile_mode);

      lvl->pitch = align(nbx * blocksize, tsx);

      mt->total_size += lvl->pitch * align(nby, tsy) * align(d, tsz);

      w = u_minify(w, 1);
      h = u_minify(h, 1);
      d = u_minify(d, 1);
   }

   if (pt->array_size > 1) {
      mt->layer_stride = align(mt->total_size,
                               NVC0_TILE_SIZE(mt->level[0].tile_mode));
      mt->total_size = mt->layer_stride * pt->array_size;
   }
}

static uint64_t
nvc0_miptree_get_modifier(struct pipe_screen *pscreen, struct nv50_miptree *mt)
{
   const union nouveau_bo_config *config = &mt->base.bo->config;
   const uint32_t uc_kind =
      nvc0_choose_tiled_storage_type(pscreen,
                                     mt->base.base.format,
                                     mt->base.base.nr_samples,
                                     false);
   const uint32_t kind_gen = nvc0_get_kind_generation(pscreen);

   if (mt->layout_3d)
      return DRM_FORMAT_MOD_INVALID;
   if (mt->base.base.nr_samples > 1)
      return DRM_FORMAT_MOD_INVALID;
   if (config->nvc0.memtype == 0x00)
      return DRM_FORMAT_MOD_LINEAR;
   if (NVC0_TILE_MODE_Y(config->nvc0.tile_mode) > 5)
      return DRM_FORMAT_MOD_INVALID;
   if (config->nvc0.memtype != uc_kind)
      return DRM_FORMAT_MOD_INVALID;

   return DRM_FORMAT_MOD_NVIDIA_BLOCK_LINEAR_2D(
             0,
             nouveau_screen(pscreen)->tegra_sector_layout ? 0 : 1,
             kind_gen,
             config->nvc0.memtype,
             NVC0_TILE_MODE_Y(config->nvc0.tile_mode));
}

bool
nvc0_miptree_get_handle(struct pipe_screen *pscreen,
                        struct pipe_context *context,
                        struct pipe_resource *pt,
                        struct winsys_handle *whandle,
                        unsigned usage)
{
   struct nv50_miptree *mt = nv50_miptree(pt);
   bool ret;

   ret = nv50_miptree_get_handle(pscreen, context, pt, whandle, usage);
   if (!ret)
      return ret;

   whandle->modifier = nvc0_miptree_get_modifier(pscreen, mt);

   return true;
}

static uint64_t
nvc0_miptree_select_best_modifier(struct pipe_screen *pscreen,
                                  const struct nv50_miptree *mt,
                                  const uint64_t *modifiers,
                                  unsigned int count)
{
   /*
    * Supported block heights are 1,2,4,8,16,32, stored as log2() their
    * value. Reserve one slot for each, as well as the linear modifier.
    */
   uint64_t prio_supported_mods[] = {
      DRM_FORMAT_MOD_INVALID,
      DRM_FORMAT_MOD_INVALID,
      DRM_FORMAT_MOD_INVALID,
      DRM_FORMAT_MOD_INVALID,
      DRM_FORMAT_MOD_INVALID,
      DRM_FORMAT_MOD_INVALID,
      DRM_FORMAT_MOD_LINEAR,
   };
   const uint32_t uc_kind = nvc0_mt_choose_storage_type(pscreen, mt, false);
   int top_mod_slot = ARRAY_SIZE(prio_supported_mods);
   const uint32_t kind_gen = nvc0_get_kind_generation(pscreen);
   unsigned int i;
   int p;

   if (uc_kind != 0u) {
      const struct pipe_resource *pt = &mt->base.base;
      const unsigned nbx = util_format_get_nblocksx(pt->format, pt->width0);
      const unsigned nby = util_format_get_nblocksy(pt->format, pt->height0);
      const uint32_t lbh_preferred =
         NVC0_TILE_MODE_Y(nvc0_tex_choose_tile_dims(nbx, nby, 1u, false));
      uint32_t lbh = lbh_preferred;
      bool dec_lbh = true;
      const uint8_t s = nouveau_screen(pscreen)->tegra_sector_layout ? 0 : 1;

      for (i = 0; i < ARRAY_SIZE(prio_supported_mods) - 1; i++) {
         assert(lbh <= 5u);
         prio_supported_mods[i] =
            DRM_FORMAT_MOD_NVIDIA_BLOCK_LINEAR_2D(0, s, kind_gen, uc_kind, lbh);

         /*
          * The preferred block height is the largest block size that doesn't
          * waste excessive space with unused padding bytes relative to the
          * height of the image.  Construct the priority array such that
          * the preferred block height is highest priority, followed by
          * progressively smaller block sizes down to a block height of one,
          * followed by progressively larger (more wasteful) block sizes up
          * to 5.
          */
         if (lbh == 0u) {
            lbh = lbh_preferred + 1u;
            dec_lbh = false;
         } else if (dec_lbh) {
            lbh--;
         } else {
            lbh++;
         }
      }
   }

   assert(prio_supported_mods[ARRAY_SIZE(prio_supported_mods) - 1] ==
          DRM_FORMAT_MOD_LINEAR);

   for (i = 0u; i < count; i++) {
      for (p = 0; p < ARRAY_SIZE(prio_supported_mods); p++) {
         if (prio_supported_mods[p] != DRM_FORMAT_MOD_INVALID) {
            if (modifiers[i] == DRM_FORMAT_MOD_INVALID ||
                prio_supported_mods[p] == modifiers[i]) {
               if (top_mod_slot > p) top_mod_slot = p;
               break;
            }
         }
      }
   }

   if (top_mod_slot >= ARRAY_SIZE(prio_supported_mods))
       return DRM_FORMAT_MOD_INVALID;

   return prio_supported_mods[top_mod_slot];
}

struct pipe_resource *
nvc0_miptree_create(struct pipe_screen *pscreen,
                    const struct pipe_resource *templ,
                    const uint64_t *modifiers, unsigned int count)
{
   struct nouveau_device *dev = nouveau_screen(pscreen)->device;
   struct nouveau_drm *drm = nouveau_screen(pscreen)->drm;
   struct nv50_miptree *mt = CALLOC_STRUCT(nv50_miptree);
   struct pipe_resource *pt = &mt->base.base;
   bool compressed = drm->version >= 0x01000101;
   int ret;
   union nouveau_bo_config bo_config;
   uint32_t bo_flags;
   unsigned pitch_align;
   uint64_t modifier = DRM_FORMAT_MOD_INVALID;

   if (!mt)
      return NULL;

   *pt = *templ;
   pipe_reference_init(&pt->reference, 1);
   pt->screen = pscreen;

   if (pt->usage == PIPE_USAGE_STAGING) {
      /* PIPE_USAGE_STAGING, and usage in general, should not be specified when
       * modifiers are used. */
      assert(count == 0);
      switch (pt->target) {
      case PIPE_TEXTURE_2D:
      case PIPE_TEXTURE_RECT:
         if (pt->last_level == 0 &&
             !util_format_is_depth_or_stencil(pt->format) &&
             pt->nr_samples <= 1)
            pt->flags |= NOUVEAU_RESOURCE_FLAG_LINEAR;
         break;
      default:
         break;
      }
   }

   if (pt->bind & PIPE_BIND_LINEAR)
      pt->flags |= NOUVEAU_RESOURCE_FLAG_LINEAR;

   if (count > 0) {
      modifier = nvc0_miptree_select_best_modifier(pscreen, mt,
                                                   modifiers, count);

      if (modifier == DRM_FORMAT_MOD_INVALID) {
         FREE(mt);
         return NULL;
      }

      if (modifier == DRM_FORMAT_MOD_LINEAR) {
         pt->flags |= NOUVEAU_RESOURCE_FLAG_LINEAR;
         bo_config.nvc0.memtype = 0;
      } else {
         bo_config.nvc0.memtype = (modifier >> 12) & 0xff;
      }
   } else {
      bo_config.nvc0.memtype = nvc0_mt_choose_storage_type(pscreen, mt, compressed);
   }

   if (!nvc0_miptree_init_ms_mode(mt)) {
      FREE(mt);
      return NULL;
   }

   if (unlikely(pt->flags & NVC0_RESOURCE_FLAG_VIDEO)) {
      assert(modifier == DRM_FORMAT_MOD_INVALID);
      nvc0_miptree_init_layout_video(mt);
   } else
   if (likely(bo_config.nvc0.memtype)) {
      nvc0_miptree_init_layout_tiled(mt, modifier);
   } else {
      /* When modifiers are supplied, usage is zero. TODO: detect the
       * modifiers+cursor case. */
      if (pt->usage & PIPE_BIND_CURSOR)
         pitch_align = 1;
      else if ((pt->usage & PIPE_BIND_SCANOUT) || count > 0)
         pitch_align = 256;
      else
         pitch_align = 128;
      if (!nv50_miptree_init_layout_linear(mt, pitch_align)) {
         FREE(mt);
         return NULL;
      }
   }
   bo_config.nvc0.tile_mode = mt->level[0].tile_mode;

   if (!bo_config.nvc0.memtype && (pt->usage == PIPE_USAGE_STAGING || pt->bind & PIPE_BIND_SHARED))
      mt->base.domain = NOUVEAU_BO_GART;
   else
      mt->base.domain = NV_VRAM_DOMAIN(nouveau_screen(pscreen));

   bo_flags = mt->base.domain | NOUVEAU_BO_NOSNOOP;

   if (mt->base.base.bind & (PIPE_BIND_CURSOR | PIPE_BIND_DISPLAY_TARGET))
      bo_flags |= NOUVEAU_BO_CONTIG;

   ret = nouveau_bo_new(dev, bo_flags, 4096, mt->total_size, &bo_config,
                        &mt->base.bo);
   if (ret) {
      FREE(mt);
      return NULL;
   }
   mt->base.address = mt->base.bo->offset;

   NOUVEAU_DRV_STAT(nouveau_screen(pscreen), tex_obj_current_count, 1);
   NOUVEAU_DRV_STAT(nouveau_screen(pscreen), tex_obj_current_bytes,
                    mt->total_size);

   return pt;
}

/* Offset of zslice @z from start of level @l. */
inline unsigned
nvc0_mt_zslice_offset(const struct nv50_miptree *mt, unsigned l, unsigned z)
{
   const struct pipe_resource *pt = &mt->base.base;

   unsigned tds = NVC0_TILE_SHIFT_Z(mt->level[l].tile_mode);
   unsigned ths = NVC0_TILE_SHIFT_Y(mt->level[l].tile_mode);

   unsigned nby = util_format_get_nblocksy(pt->format,
                                           u_minify(pt->height0, l));

   /* to next 2D tile slice within a 3D tile */
   unsigned stride_2d = NVC0_TILE_SIZE_2D(mt->level[l].tile_mode);

   /* to slice in the next (in z direction) 3D tile */
   unsigned stride_3d = (align(nby, (1 << ths)) * mt->level[l].pitch) << tds;

   return (z & (1 << (tds - 1))) * stride_2d + (z >> tds) * stride_3d;
}

/* Surface functions.
 */

struct pipe_surface *
nvc0_miptree_surface_new(struct pipe_context *pipe,
                         struct pipe_resource *pt,
                         const struct pipe_surface *templ)
{
   struct nv50_surface *ns = nv50_surface_from_miptree(nv50_miptree(pt), templ);
   if (!ns)
      return NULL;
   ns->base.context = pipe;
   return &ns->base;
}
