/*
 * Copyright (c) 2012-2015 Etnaviv Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Wladimir J. van der Laan <laanwj@gmail.com>
 */

#include "etnaviv_resource.h"

#include "hw/common.xml.h"

#include "etnaviv_context.h"
#include "etnaviv_debug.h"
#include "etnaviv_screen.h"
#include "etnaviv_translate.h"

#include "util/hash_table.h"
#include "util/u_inlines.h"
#include "util/u_memory.h"

static enum etna_surface_layout modifier_to_layout(uint64_t modifier)
{
   switch (modifier & ~VIVANTE_MOD_EXT_MASK) {
   case DRM_FORMAT_MOD_VIVANTE_TILED:
      return ETNA_LAYOUT_TILED;
   case DRM_FORMAT_MOD_VIVANTE_SUPER_TILED:
      return ETNA_LAYOUT_SUPER_TILED;
   case DRM_FORMAT_MOD_VIVANTE_SPLIT_TILED:
      return ETNA_LAYOUT_MULTI_TILED;
   case DRM_FORMAT_MOD_VIVANTE_SPLIT_SUPER_TILED:
      return ETNA_LAYOUT_MULTI_SUPERTILED;
   case DRM_FORMAT_MOD_LINEAR:
      return ETNA_LAYOUT_LINEAR;
   default:
      unreachable("unhandled modifier");
   }
}

static uint64_t layout_to_modifier(enum etna_surface_layout layout)
{
   switch (layout) {
   case ETNA_LAYOUT_TILED:
      return DRM_FORMAT_MOD_VIVANTE_TILED;
   case ETNA_LAYOUT_SUPER_TILED:
      return DRM_FORMAT_MOD_VIVANTE_SUPER_TILED;
   case ETNA_LAYOUT_MULTI_TILED:
      return DRM_FORMAT_MOD_VIVANTE_SPLIT_TILED;
   case ETNA_LAYOUT_MULTI_SUPERTILED:
      return DRM_FORMAT_MOD_VIVANTE_SPLIT_SUPER_TILED;
   case ETNA_LAYOUT_LINEAR:
      return DRM_FORMAT_MOD_LINEAR;
   default:
      return DRM_FORMAT_MOD_INVALID;
   }
}

static uint64_t etna_resource_modifier(struct etna_resource *rsc)
{
   if (etna_resource_ext_ts(rsc))
      return rsc->modifier;

   return layout_to_modifier(rsc->layout);
}

/* A tile is either 64 bytes or, when the GPU has the CACHE128B256BPERLINE
 * feature, 128/256 bytes of color/depth data, tracked by
 * 'screen->specs.bits_per_tile' bits of tile status.
 */
bool
etna_screen_resource_alloc_ts(struct pipe_screen *pscreen,
                              struct etna_resource *rsc, uint64_t modifier)
{
   struct etna_screen *screen = etna_screen(pscreen);
   struct etna_resource_level *lvl = &rsc->levels[0];
   struct pipe_resource *prsc = &rsc->base;
   size_t tile_size, ts_size, ts_bo_size, ts_layer_stride, ts_data_offset = 0;
   uint8_t ts_mode = TS_MODE_128B;
   int8_t ts_compress_fmt = -1;
   unsigned layers;

   assert(!rsc->ts_bo);

   /* pre-v4 compression is largely useless, so disable it when not wanted for MSAA
    * v4 compression can be enabled everywhere without any known drawback,
    * except that in-place resolve must go through a slower path
    */
   if ((screen->specs.v4_compression &&
        (!modifier || (modifier & VIVANTE_MOD_COMP_DEC400))) ||
       (!modifier && prsc->nr_samples > 1))
      ts_compress_fmt = translate_ts_format(prsc->format);

   /* enable 256B ts mode with compression, as it improves performance
    * the size of the resource might also determine if we want to use it or not
    */
   if (VIV_FEATURE(screen, chipMinorFeatures6, CACHE128B256BPERLINE)) {
      if ((modifier & VIVANTE_MOD_TS_MASK) == VIVANTE_MOD_TS_128_4)
         ts_mode = TS_MODE_128B;
      else if ((modifier & VIVANTE_MOD_TS_MASK) == VIVANTE_MOD_TS_256_4)
         ts_mode = TS_MODE_256B;
      else {
         /* Without a TS modifier TS is only internal, so we can choose the
          * mode to use freely. Enable 256B ts mode with compression, as it
          * improves performance. The size of the resource might also determine
          * if we want to use it or not.
          */
         if (ts_compress_fmt >= 0 &&
             (rsc->layout != ETNA_LAYOUT_LINEAR || lvl->stride % 256 == 0))
            ts_mode = TS_MODE_256B;
         else
            ts_mode = TS_MODE_128B;
      }
   }

   tile_size = etna_screen_get_tile_size(screen, ts_mode, prsc->nr_samples > 1);
   layers = prsc->target == PIPE_TEXTURE_3D ? prsc->depth0 : prsc->array_size;
   ts_layer_stride = align(DIV_ROUND_UP(lvl->layer_stride,
                                        tile_size * 8 / screen->specs.bits_per_tile),
                           0x100 * screen->specs.pixel_pipes);
   ts_size = ts_bo_size = ts_layer_stride * layers;
   if (ts_size == 0)
      return true;

   /* add space for the software meta */
   if (modifier & VIVANTE_MOD_TS_MASK) {
      /* The offset is a educated guess for a safe value based on the experience
       * that various object pointers on the GPU need to be 64B aligned. Maybe
       * some GPU needs even more alignment, or we could drop this to 32B. 64B
       * has worked well across various GPU generations so far.
       */
      ts_data_offset = 64;
      assert(ts_data_offset >= sizeof(struct etna_ts_sw_meta));
      ts_bo_size += ts_data_offset;
   }

   DBG_F(ETNA_DBG_RESOURCE_MSGS, "%p: Allocating tile status of size %zu",
         rsc, ts_bo_size);

   if ((rsc->base.bind & PIPE_BIND_SCANOUT) && screen->ro) {
      struct pipe_resource scanout_templat;
      struct winsys_handle handle;

      scanout_templat.format = PIPE_FORMAT_R8_UNORM;
      scanout_templat.width0 = align(ts_bo_size, 4096);
      scanout_templat.height0 = 1;

      rsc->ts_scanout = renderonly_scanout_for_resource(&scanout_templat,
                                                     screen->ro, &handle);
      if (!rsc->ts_scanout) {
         BUG("Problem allocating kms memory for TS resource");
         return false;
      }

      assert(handle.type == WINSYS_HANDLE_TYPE_FD);
      rsc->ts_bo = etna_screen_bo_from_handle(pscreen, &handle);
      close(handle.handle);
   } else {
      rsc->ts_bo = etna_bo_new(screen->dev, ts_bo_size, DRM_ETNA_GEM_CACHE_WC);
   }

   if (unlikely(!rsc->ts_bo)) {
      BUG("Problem allocating tile status for resource");
      return false;
   }

   lvl->ts_offset = ts_data_offset;
   lvl->ts_layer_stride = ts_layer_stride;
   lvl->ts_size = ts_size;
   lvl->ts_mode = ts_mode;
   lvl->ts_compress_fmt = ts_compress_fmt;

   /* fill software meta */
   if (modifier & VIVANTE_MOD_TS_MASK) {
      lvl->ts_meta = etna_bo_map(rsc->ts_bo);
      memset(lvl->ts_meta, 0, sizeof(struct etna_ts_sw_meta));
      lvl->ts_meta->version = 0;
      lvl->ts_meta->v0.data_size = ts_size;
      lvl->ts_meta->v0.data_offset = ts_data_offset;
      lvl->ts_meta->v0.layer_stride = ts_layer_stride;
      lvl->ts_meta->v0.comp_format = ts_format_to_drmfourcc(ts_compress_fmt);
   }
   return true;
}

static bool
etna_screen_can_create_resource(struct pipe_screen *pscreen,
                                const struct pipe_resource *templat)
{
   struct etna_screen *screen = etna_screen(pscreen);
   if (!translate_samples_to_xyscale(templat->nr_samples, NULL, NULL))
      return false;

   /* templat->bind is not set here, so we must use the minimum sizes */
   uint max_size =
      MIN2(screen->specs.max_rendertarget_size, screen->specs.max_texture_size);

   if (templat->width0 > max_size || templat->height0 > max_size)
      return false;

   return true;
}

static unsigned
setup_miptree(struct etna_resource *rsc, unsigned paddingX, unsigned paddingY,
              unsigned msaa_xscale, unsigned msaa_yscale)
{
   struct pipe_resource *prsc = &rsc->base;
   unsigned level, size = 0;
   unsigned width = prsc->width0;
   unsigned height = prsc->height0;
   unsigned depth = prsc->depth0;

   for (level = 0; level <= prsc->last_level; level++) {
      struct etna_resource_level *mip = &rsc->levels[level];

      mip->width = width;
      mip->height = height;
      mip->depth = depth;
      mip->padded_width = align(width * msaa_xscale, paddingX);
      mip->padded_height = align(height * msaa_yscale, paddingY);
      mip->stride = util_format_get_stride(prsc->format, mip->padded_width);
      mip->offset = size;
      mip->layer_stride = mip->stride * util_format_get_nblocksy(prsc->format, mip->padded_height);
      mip->size = prsc->array_size * mip->layer_stride;

      /* align levels to 64 bytes to be able to render to them */
      size += align(mip->size, ETNA_PE_ALIGNMENT) * depth;

      width = u_minify(width, 1);
      height = u_minify(height, 1);
      depth = u_minify(depth, 1);
   }

   return size;
}

/* Compute the slice/miplevel alignment (in pixels) and the texture sampler
 * HALIGN parameter from the resource parameters and the target layout.
 */
static void
etna_layout_multiple(const struct etna_screen *screen,
                     const struct pipe_resource *templat, unsigned layout,
                     unsigned *paddingX, unsigned *paddingY, unsigned *halign)
{
   const struct etna_specs *specs = &screen->specs;
   /* If we have the TEXTURE_HALIGN feature, we can always align to the resolve
    * engine's width.  If not, we must not align resources used only for
    * textures. If this GPU uses the BLT engine, never do RS align.
    */
   bool rs_align = !specs->use_blt && (!etna_resource_sampler_only(templat) ||
                   VIV_FEATURE(screen, chipMinorFeatures1, TEXTURE_HALIGN));
   int msaa_xscale = 1, msaa_yscale = 1;

   /* Compressed textures are padded to their block size, but we don't have
    * to do anything special for that.
    */
   if (unlikely(util_format_is_compressed(templat->format))) {
      assert(layout == ETNA_LAYOUT_LINEAR);
      *paddingX = 1;
      *paddingY = 1;
      *halign = TEXTURE_HALIGN_FOUR;
      return;
   }

   translate_samples_to_xyscale(templat->nr_samples, &msaa_xscale, &msaa_yscale);

   switch (layout) {
   case ETNA_LAYOUT_LINEAR:
      *paddingX = rs_align ? 16 : 4;
      *paddingY = !specs->use_blt && templat->target != PIPE_BUFFER ? 4 : 1;
      *halign = rs_align ? TEXTURE_HALIGN_SIXTEEN : TEXTURE_HALIGN_FOUR;
      break;
   case ETNA_LAYOUT_TILED:
      *paddingX = rs_align ? 16 * msaa_xscale : 4;
      *paddingY = 4 * msaa_yscale;
      *halign = rs_align ? TEXTURE_HALIGN_SIXTEEN : TEXTURE_HALIGN_FOUR;
      break;
   case ETNA_LAYOUT_SUPER_TILED:
      *paddingX = 64;
      *paddingY = 64;
      *halign = TEXTURE_HALIGN_SUPER_TILED;
      break;
   case ETNA_LAYOUT_MULTI_TILED:
      *paddingX = 16 * msaa_xscale;
      *paddingY = 4 * msaa_yscale * specs->pixel_pipes;
      *halign = TEXTURE_HALIGN_SPLIT_TILED;
      break;
   case ETNA_LAYOUT_MULTI_SUPERTILED:
      *paddingX = 64;
      *paddingY = 64 * specs->pixel_pipes;
      *halign = TEXTURE_HALIGN_SPLIT_SUPER_TILED;
      break;
   default:
      unreachable("Unhandled layout");
   }
}

/* Create a new resource object, using the given template info */
struct pipe_resource *
etna_resource_alloc(struct pipe_screen *pscreen, unsigned layout,
                    uint64_t modifier, const struct pipe_resource *templat)
{
   struct etna_screen *screen = etna_screen(pscreen);
   struct etna_resource *rsc;
   unsigned size;

   DBG_F(ETNA_DBG_RESOURCE_MSGS,
         "target=%d, format=%s, %ux%ux%u, array_size=%u, "
         "last_level=%u, nr_samples=%u, usage=%u, bind=%x, flags=%x",
         templat->target, util_format_name(templat->format), templat->width0,
         templat->height0, templat->depth0, templat->array_size,
         templat->last_level, templat->nr_samples, templat->usage,
         templat->bind, templat->flags);

   /* Determine scaling for antialiasing */
   int msaa_xscale = 1, msaa_yscale = 1;
   if (!translate_samples_to_xyscale(templat->nr_samples, &msaa_xscale, &msaa_yscale)) {
      /* Number of samples not supported */
      return NULL;
   }

   /* Determine needed padding (alignment of height/width) */
   unsigned paddingX, paddingY, halign;
   etna_layout_multiple(screen, templat, layout, &paddingX, &paddingY, &halign);

   rsc = CALLOC_STRUCT(etna_resource);
   if (!rsc)
      return NULL;

   rsc->base = *templat;
   rsc->base.screen = pscreen;
   rsc->base.nr_samples = templat->nr_samples;
   rsc->layout = layout;
   rsc->modifier = modifier;
   rsc->halign = halign;
   rsc->explicit_flush = true;

   pipe_reference_init(&rsc->base.reference, 1);
   util_range_init(&rsc->valid_buffer_range);

   size = setup_miptree(rsc, paddingX, paddingY, msaa_xscale, msaa_yscale);

   if (unlikely(templat->bind & PIPE_BIND_SCANOUT) && screen->ro) {
      struct pipe_resource scanout_templat = *templat;
      struct winsys_handle handle;

      scanout_templat.width0 = align(scanout_templat.width0, paddingX);
      scanout_templat.height0 = align(scanout_templat.height0, paddingY);

      rsc->scanout = renderonly_scanout_for_resource(&scanout_templat,
                                                     screen->ro, &handle);
      if (!rsc->scanout) {
         BUG("Problem allocating kms memory for resource");
         goto free_rsc;
      }

      assert(handle.type == WINSYS_HANDLE_TYPE_FD);
      rsc->levels[0].stride = handle.stride;
      rsc->bo = etna_screen_bo_from_handle(pscreen, &handle);
      close(handle.handle);
      if (unlikely(!rsc->bo))
         goto free_rsc;
   } else {
      uint32_t flags = DRM_ETNA_GEM_CACHE_WC;

      if (templat->bind & PIPE_BIND_VERTEX_BUFFER)
         flags |= DRM_ETNA_GEM_FORCE_MMU;

      rsc->bo = etna_bo_new(screen->dev, size, flags);
      if (unlikely(!rsc->bo)) {
         BUG("Problem allocating video memory for resource");
         goto free_rsc;
      }
   }

   /* If TS is externally visible set it up now, so it can be exported before
    * the first rendering to a surface.
    */
   if (etna_resource_ext_ts(rsc))
      etna_screen_resource_alloc_ts(pscreen, rsc, modifier);

   if (DBG_ENABLED(ETNA_DBG_ZERO)) {
      void *map = etna_bo_map(rsc->bo);
      etna_bo_cpu_prep(rsc->bo, DRM_ETNA_PREP_WRITE);
      memset(map, 0, size);
      etna_bo_cpu_fini(rsc->bo);
   }

   return &rsc->base;

free_rsc:
   FREE(rsc);
   return NULL;
}

static struct pipe_resource *
etna_resource_create(struct pipe_screen *pscreen,
                     const struct pipe_resource *templat)
{
   struct etna_screen *screen = etna_screen(pscreen);
   unsigned layout = ETNA_LAYOUT_TILED;

   /* At this point we don't know if the resource will be used as a texture,
    * render target, or both, because gallium sets the bits whenever possible
    * This matters because on some GPUs (GC2000) there is no tiling that is
    * compatible with both TE and PE.
    *
    * We expect that depth/stencil buffers will always be used by PE (rendering),
    * and any other non-scanout resource will be used as a texture at some point,
    * So allocate a render-compatible base buffer for scanout/depthstencil buffers,
    * and a texture-compatible base buffer in other cases
    *
    */
   if (templat->bind & PIPE_BIND_DEPTH_STENCIL) {
      if (screen->specs.pixel_pipes > 1 && !screen->specs.single_buffer)
         layout |= ETNA_LAYOUT_BIT_MULTI;
      if (screen->specs.can_supertile)
         layout |= ETNA_LAYOUT_BIT_SUPER;
   } else if (screen->specs.can_supertile &&
              VIV_FEATURE(screen, chipMinorFeatures2, SUPERTILED_TEXTURE) &&
              etna_resource_hw_tileable(screen->specs.use_blt, templat)) {
      layout |= ETNA_LAYOUT_BIT_SUPER;
   }

   if (/* MSAA render target */
       (templat->nr_samples > 1) &&
       (templat->bind & (PIPE_BIND_RENDER_TARGET | PIPE_BIND_DEPTH_STENCIL))) {
      if (screen->specs.pixel_pipes > 1 && !screen->specs.single_buffer)
         layout |= ETNA_LAYOUT_BIT_MULTI;
      if (screen->specs.can_supertile)
         layout |= ETNA_LAYOUT_BIT_SUPER;
   }

   if (/* linear base or scanout without modifier requested */
       (templat->bind & (PIPE_BIND_LINEAR | PIPE_BIND_SCANOUT)) ||
       templat->target == PIPE_BUFFER || /* buffer always linear */
       /* compressed textures don't use tiling, they have their own "tiles" */
       util_format_is_compressed(templat->format)) {
      layout = ETNA_LAYOUT_LINEAR;
   }

   /* modifier is only used for scanout surfaces, so safe to use LINEAR here */
   return etna_resource_alloc(pscreen, layout, DRM_FORMAT_MOD_LINEAR, templat);
}

enum modifier_priority {
   MODIFIER_PRIORITY_INVALID = 0,
   MODIFIER_PRIORITY_LINEAR,
   MODIFIER_PRIORITY_SPLIT_TILED,
   MODIFIER_PRIORITY_SPLIT_SUPER_TILED,
   MODIFIER_PRIORITY_TILED,
   MODIFIER_PRIORITY_SUPER_TILED,
};

static const uint64_t priority_to_modifier[] = {
   [MODIFIER_PRIORITY_INVALID] = DRM_FORMAT_MOD_INVALID,
   [MODIFIER_PRIORITY_LINEAR] = DRM_FORMAT_MOD_LINEAR,
   [MODIFIER_PRIORITY_SPLIT_TILED] = DRM_FORMAT_MOD_VIVANTE_SPLIT_TILED,
   [MODIFIER_PRIORITY_SPLIT_SUPER_TILED] = DRM_FORMAT_MOD_VIVANTE_SPLIT_SUPER_TILED,
   [MODIFIER_PRIORITY_TILED] = DRM_FORMAT_MOD_VIVANTE_TILED,
   [MODIFIER_PRIORITY_SUPER_TILED] = DRM_FORMAT_MOD_VIVANTE_SUPER_TILED,
};

static uint64_t
select_best_modifier(const struct etna_screen * screen,
                     const uint64_t *modifiers, const unsigned count)
{
   enum modifier_priority prio = MODIFIER_PRIORITY_INVALID;
   uint64_t best_modifier, base_modifier;

   for (int i = 0; i < count; i++) {
      switch (modifiers[i] & ~VIVANTE_MOD_EXT_MASK) {
      case DRM_FORMAT_MOD_VIVANTE_SUPER_TILED:
         if ((screen->specs.pixel_pipes > 1 && !screen->specs.single_buffer) ||
             !screen->specs.can_supertile)
            break;
         prio = MAX2(prio, MODIFIER_PRIORITY_SUPER_TILED);
         break;
      case DRM_FORMAT_MOD_VIVANTE_TILED:
         if (screen->specs.pixel_pipes > 1 && !screen->specs.single_buffer)
            break;
         prio = MAX2(prio, MODIFIER_PRIORITY_TILED);
         break;
      case DRM_FORMAT_MOD_VIVANTE_SPLIT_SUPER_TILED:
         if ((screen->specs.pixel_pipes < 2) || !screen->specs.can_supertile)
            break;
         prio = MAX2(prio, MODIFIER_PRIORITY_SPLIT_SUPER_TILED);
         break;
      case DRM_FORMAT_MOD_VIVANTE_SPLIT_TILED:
         if (screen->specs.pixel_pipes < 2)
            break;
         prio = MAX2(prio, MODIFIER_PRIORITY_SPLIT_TILED);
         break;
      case DRM_FORMAT_MOD_LINEAR:
         prio = MAX2(prio, MODIFIER_PRIORITY_LINEAR);
         break;
      case DRM_FORMAT_MOD_INVALID:
      default:
         break;
      }
   }

   best_modifier = base_modifier = priority_to_modifier[prio];

   if (!DBG_ENABLED(ETNA_DBG_SHARED_TS) ||
       !VIV_FEATURE(screen, chipFeatures, FAST_CLEAR))
      return best_modifier;

   /* Make a second pass to try and find the best TS modifier if any. */
   for (int i = 0; i < count; i++) {
      if ((modifiers[i] & ~VIVANTE_MOD_EXT_MASK) == base_modifier)
         if ((modifiers[i] & VIVANTE_MOD_TS_MASK) >
             (best_modifier & VIVANTE_MOD_TS_MASK))
            best_modifier = modifiers[i];
   }

   /* If no modifier with TS was found, there is no point in looking further,
    * as compression depends on TS.
    */
   if (best_modifier == base_modifier)
      return best_modifier;

   /* Make a third pass to find a modifier allowing compression. */
   base_modifier = best_modifier;
   for (int i = 0; i < count; i++) {
      if ((modifiers[i] & ~VIVANTE_MOD_COMP_MASK) == base_modifier)
         if ((modifiers[i] & VIVANTE_MOD_COMP_MASK) >
             (best_modifier & VIVANTE_MOD_COMP_MASK))
            best_modifier = modifiers[i];
   }

   return best_modifier;
}

static struct pipe_resource *
etna_resource_create_modifiers(struct pipe_screen *pscreen,
                               const struct pipe_resource *templat,
                               const uint64_t *modifiers, int count)
{
   struct etna_screen *screen = etna_screen(pscreen);
   struct pipe_resource tmpl = *templat;
   uint64_t modifier = select_best_modifier(screen, modifiers, count);

   if (modifier == DRM_FORMAT_MOD_INVALID)
      return NULL;

   return etna_resource_alloc(pscreen, modifier_to_layout(modifier), modifier, &tmpl);
}

static void
etna_resource_changed(struct pipe_screen *pscreen, struct pipe_resource *prsc)
{
   struct etna_resource *rsc = etna_resource(prsc);

   for (int level = 0; level <= prsc->last_level; level++)
      etna_resource_level_mark_changed(&rsc->levels[level]);
}

static void
etna_resource_destroy(struct pipe_screen *pscreen, struct pipe_resource *prsc)
{
   struct etna_resource *rsc = etna_resource(prsc);

   if (rsc->bo)
      etna_bo_del(rsc->bo);

   if (rsc->ts_bo)
      etna_bo_del(rsc->ts_bo);

   if (rsc->scanout)
      renderonly_scanout_destroy(rsc->scanout, etna_screen(pscreen)->ro);

   if (rsc->ts_scanout)
      renderonly_scanout_destroy(rsc->ts_scanout, etna_screen(pscreen)->ro);

   util_range_destroy(&rsc->valid_buffer_range);

   pipe_resource_reference(&rsc->texture, NULL);
   pipe_resource_reference(&rsc->render, NULL);

   for (unsigned i = 0; i < ETNA_NUM_LOD; i++)
      FREE(rsc->levels[i].patch_offsets);

   FREE(rsc);
}

static void etna_resource_finish_ts_import(struct etna_screen *screen,
                                           struct etna_resource *rsc)
{
   struct etna_resource *ts_rsc = etna_resource(rsc->base.next);
   uint64_t ts_modifier = rsc->modifier & VIVANTE_MOD_TS_MASK;
   struct etna_resource_level *lvl = &rsc->levels[0];
   uint8_t ts_mode = 0;

   if (ts_rsc->bo == rsc->bo)
      fprintf(stderr, "etnaviv: application bug: importing shared TS resource "
              "with TS BO matching color BO, expect rendering corruption!\n");

   if (ts_modifier == VIVANTE_MOD_TS_256_4)
      ts_mode = TS_MODE_256B;

   rsc->ts_bo = etna_bo_ref(ts_rsc->bo);

   rsc->ts_scanout = ts_rsc->scanout;
   ts_rsc->scanout = NULL;

   /* Get TS layout/usage information from the SW meta. FIXME: clear color may
    * change over the lifetime of the resource, so might need to look this up
    * at a few other places. For now it's not an issue, as buffers with shared
    * TS get re-imported all the time. */
   lvl->ts_meta = etna_bo_map(rsc->ts_bo) + ts_rsc->levels[0].offset;
   lvl->ts_compress_fmt = drmfourcc_to_ts_format(lvl->ts_meta->v0.comp_format);
   lvl->ts_offset = ts_rsc->levels[0].offset + lvl->ts_meta->v0.data_offset;
   lvl->ts_layer_stride = lvl->ts_meta->v0.layer_stride;
   lvl->clear_value = lvl->ts_meta->v0.clear_value;
   lvl->ts_size = lvl->ts_meta->v0.data_size;
   lvl->ts_mode = ts_mode;

   etna_resource_destroy(&screen->base, rsc->base.next);
   rsc->base.next = NULL;
}

static struct pipe_resource *
etna_resource_from_handle(struct pipe_screen *pscreen,
                          const struct pipe_resource *tmpl,
                          struct winsys_handle *handle, unsigned usage)
{
   struct etna_screen *screen = etna_screen(pscreen);
   struct etna_resource *rsc;
   struct etna_resource_level *level;
   struct pipe_resource *prsc;
   uint64_t modifier = handle->modifier;

   DBG("target=%d, format=%s, %ux%ux%u, array_size=%u, last_level=%u, "
       "nr_samples=%u, usage=%u, bind=%x, flags=%x",
       tmpl->target, util_format_name(tmpl->format), tmpl->width0,
       tmpl->height0, tmpl->depth0, tmpl->array_size, tmpl->last_level,
       tmpl->nr_samples, tmpl->usage, tmpl->bind, tmpl->flags);

   rsc = CALLOC_STRUCT(etna_resource);
   if (!rsc)
      return NULL;

   level = &rsc->levels[0];
   prsc = &rsc->base;

   *prsc = *tmpl;

   pipe_reference_init(&prsc->reference, 1);
   util_range_init(&rsc->valid_buffer_range);
   prsc->screen = pscreen;

   rsc->bo = etna_screen_bo_from_handle(pscreen, handle);
   if (!rsc->bo)
      goto fail;

   if (modifier == DRM_FORMAT_MOD_INVALID)
      modifier = DRM_FORMAT_MOD_LINEAR;

   rsc->layout = modifier_to_layout(modifier);
   rsc->modifier = modifier;

   if (usage & PIPE_HANDLE_USAGE_EXPLICIT_FLUSH)
      rsc->explicit_flush = true;

   level->width = tmpl->width0;
   level->height = tmpl->height0;
   level->depth = tmpl->depth0;
   level->stride = handle->stride;
   level->offset = handle->offset;
   level->seqno = 1;

   /* Determine padding of the imported resource. */
   unsigned paddingX, paddingY;
   etna_layout_multiple(screen, tmpl, rsc->layout,
                        &paddingX, &paddingY, &rsc->halign);

   level->padded_width = align(level->width, paddingX);
   level->padded_height = align(level->height, paddingY);

   level->layer_stride = level->stride * util_format_get_nblocksy(prsc->format,
                                                                  level->padded_height);
   level->size = level->layer_stride;

   if (screen->ro)
      rsc->scanout = renderonly_create_gpu_import_for_resource(prsc, screen->ro,
                                                               NULL);

   /* If the buffer is for a TS plane, skip the RS compatible checks */
   if (handle->plane >= util_format_get_num_planes(prsc->format))
      return prsc;

   /* The DDX must give us a BO which conforms to our padding size.
    * The stride of the BO must be greater or equal to our padded
    * stride. The size of the BO must accomodate the padded height. */
   if (level->stride < util_format_get_stride(tmpl->format, level->padded_width)) {
      BUG("BO stride %u is too small for RS engine width padding (%u, format %s)",
          level->stride, util_format_get_stride(tmpl->format, level->padded_width),
          util_format_name(tmpl->format));
      goto fail;
   }
   if (etna_bo_size(rsc->bo) < level->stride * level->padded_height) {
      BUG("BO size %u is too small for RS engine height padding (%u, format %s)",
          etna_bo_size(rsc->bo), level->stride * level->padded_height,
          util_format_name(tmpl->format));
      goto fail;
   }

   if (handle->plane == 0 && etna_resource_ext_ts(rsc))
      etna_resource_finish_ts_import(screen, rsc);

   return prsc;

fail:
   etna_resource_destroy(pscreen, prsc);

   return NULL;
}

static bool
etna_resource_get_handle(struct pipe_screen *pscreen,
                         struct pipe_context *pctx,
                         struct pipe_resource *prsc,
                         struct winsys_handle *handle, unsigned usage)
{
   struct etna_screen *screen = etna_screen(pscreen);
   struct etna_resource *rsc = etna_resource(prsc);
   bool wants_ts = etna_resource_ext_ts(rsc) &&
                      handle->plane >= util_format_get_num_planes(prsc->format);
   struct renderonly_scanout *scanout;
   struct etna_resource_level *lvl;
   struct etna_bo *bo;

   if (handle->plane && !wants_ts) {
      struct pipe_resource *cur = prsc;

      for (int i = 0; i < handle->plane; i++) {
         cur = cur->next;
         if (!cur)
            return false;
      }
      rsc = etna_resource(cur);
   }

   /* Scanout is always attached to the base resource */
   scanout = rsc->scanout;

   lvl = &rsc->levels[0];

   if (wants_ts) {
      handle->stride = DIV_ROUND_UP(lvl->stride,
                          etna_screen_get_tile_size(screen, lvl->ts_mode, false)
                          * 8 / screen->specs.bits_per_tile);
      handle->offset = lvl->ts_offset - lvl->ts_meta->v0.data_offset;
      scanout = rsc->ts_scanout;
      bo = rsc->ts_bo;
   } else {
      handle->stride = lvl->stride;
      handle->offset = lvl->offset;
      scanout = rsc->scanout;
      bo = rsc->bo;
   }
   handle->modifier = etna_resource_modifier(rsc);

   if (!(usage & PIPE_HANDLE_USAGE_EXPLICIT_FLUSH))
      rsc->explicit_flush = false;

   if (handle->type == WINSYS_HANDLE_TYPE_SHARED) {
      return etna_bo_get_name(bo, &handle->handle) == 0;
   } else if (handle->type == WINSYS_HANDLE_TYPE_KMS) {
      if (screen->ro) {
         return renderonly_get_handle(scanout, handle);
      } else {
         handle->handle = etna_bo_handle(bo);
         return true;
      }
   } else if (handle->type == WINSYS_HANDLE_TYPE_FD) {
      handle->handle = etna_bo_dmabuf(bo);
      return true;
   } else {
      return false;
   }
}

static bool
etna_resource_get_param(struct pipe_screen *pscreen,
                        struct pipe_context *pctx, struct pipe_resource *prsc,
                        unsigned plane, unsigned layer, unsigned level,
                        enum pipe_resource_param param,
                        unsigned usage, uint64_t *value)
{
   struct etna_screen *screen = etna_screen(pscreen);
   struct etna_resource *rsc = etna_resource(prsc);
   bool wants_ts = etna_resource_ext_ts(rsc) &&
                      plane >= util_format_get_num_planes(prsc->format);
   struct etna_resource_level *lvl;

   if (param == PIPE_RESOURCE_PARAM_NPLANES) {
      if (etna_resource_ext_ts(rsc)) {
               *value = 2;
      } else {
         unsigned count = 0;

         for (struct pipe_resource *cur = prsc; cur; cur = cur->next)
            count++;
         *value = count;
      }
      return true;
   }

   if (!wants_ts) {
      struct pipe_resource *cur = prsc;
      for (int i = 0; i < plane; i++) {
         cur = cur->next;
         if (!cur)
            return false;
      }
      rsc = etna_resource(cur);
   }

   lvl = &rsc->levels[0];

   switch (param) {
   case PIPE_RESOURCE_PARAM_STRIDE:
      if (wants_ts) {
         *value = DIV_ROUND_UP(lvl->stride,
                               etna_screen_get_tile_size(screen, lvl->ts_mode,
                                                         prsc->nr_samples > 1)
                               * 8 / screen->specs.bits_per_tile);
      } else {
         *value = lvl->stride;
      }
      return true;
   case PIPE_RESOURCE_PARAM_OFFSET:
      if (wants_ts)
         *value = lvl->ts_offset - lvl->ts_meta->v0.data_offset;
      else
         *value = lvl->offset;
      return true;
   case PIPE_RESOURCE_PARAM_MODIFIER:
      *value = etna_resource_modifier(rsc);
      return true;
   default:
      return false;
   }
}

void
etna_resource_used(struct etna_context *ctx, struct pipe_resource *prsc,
                   enum etna_resource_status status)
{
   struct etna_resource *rsc;
   struct hash_entry *entry;
   uint32_t hash;

   if (!prsc)
      return;

   rsc = etna_resource(prsc);
   hash = _mesa_hash_pointer(rsc);
   entry = _mesa_hash_table_search_pre_hashed(ctx->pending_resources,
                                              hash, rsc);

   if (entry) {
      enum etna_resource_status tmp = (uintptr_t)entry->data;
      tmp |= status;
      entry->data = (void *)(uintptr_t)tmp;
   } else {
      _mesa_hash_table_insert_pre_hashed(ctx->pending_resources, hash, rsc,
                                         (void *)(uintptr_t)status);
   }
}

enum etna_resource_status
etna_resource_status(struct etna_context *ctx, struct etna_resource *res)
{
   struct hash_entry *entry = _mesa_hash_table_search(ctx->pending_resources, res);

   if (entry)
      return (enum etna_resource_status)(uintptr_t)entry->data;
   else
      return 0;
}

bool
etna_resource_needs_flush(struct etna_resource *rsc)
{
   if (!rsc->ts_bo)
      return false;

   for (int level = 0; level <= rsc->base.last_level; level++)
      if (etna_resource_level_needs_flush(&rsc->levels[level]))
         return true;

   return false;
}

void
etna_resource_screen_init(struct pipe_screen *pscreen)
{
   pscreen->can_create_resource = etna_screen_can_create_resource;
   pscreen->resource_create = etna_resource_create;
   pscreen->resource_create_with_modifiers = etna_resource_create_modifiers;
   pscreen->resource_from_handle = etna_resource_from_handle;
   pscreen->resource_get_handle = etna_resource_get_handle;
   pscreen->resource_get_param = etna_resource_get_param;
   pscreen->resource_changed = etna_resource_changed;
   pscreen->resource_destroy = etna_resource_destroy;
}
