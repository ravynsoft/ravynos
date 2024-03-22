/*
 * Copyright 2010 Jerome Glisse <glisse@freedesktop.org>
 * Copyright 2018 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#include "drm-uapi/drm_fourcc.h"
#include "si_pipe.h"
#include "si_query.h"
#include "sid.h"
#include "frontend/drm_driver.h"
#include "util/format/u_format.h"
#include "util/os_time.h"
#include "util/u_log.h"
#include "util/u_memory.h"
#include "util/u_pack_color.h"
#include "util/u_resource.h"
#include "util/u_surface.h"
#include "util/u_transfer.h"

#include <errno.h>
#include <inttypes.h>

#include "amd/addrlib/inc/addrinterface.h"

static enum radeon_surf_mode si_choose_tiling(struct si_screen *sscreen,
                                              const struct pipe_resource *templ,
                                              bool tc_compatible_htile);

static bool si_texture_is_aux_plane(const struct pipe_resource *resource);

/* Same as resource_copy_region, except that both upsampling and downsampling are allowed. */
static void si_copy_region_with_blit(struct pipe_context *pipe, struct pipe_resource *dst,
                                     unsigned dst_level, unsigned dst_sample, unsigned dstx, unsigned dsty,
                                     unsigned dstz, struct pipe_resource *src, unsigned src_level,
                                     const struct pipe_box *src_box)
{
   struct pipe_blit_info blit;

   memset(&blit, 0, sizeof(blit));
   blit.src.resource = src;
   blit.src.format = src->format;
   blit.src.level = src_level;
   blit.src.box = *src_box;
   blit.dst.resource = dst;
   blit.dst.format = dst->format;
   blit.dst.level = dst_level;
   blit.dst.box.x = dstx;
   blit.dst.box.y = dsty;
   blit.dst.box.z = dstz;
   blit.dst.box.width = src_box->width;
   blit.dst.box.height = src_box->height;
   blit.dst.box.depth = src_box->depth;
   blit.mask = util_format_get_mask(dst->format);
   blit.filter = PIPE_TEX_FILTER_NEAREST;
   blit.dst_sample = dst_sample;

   if (blit.mask) {
      /* Only the gfx blit handles dst_sample. */
      if (dst_sample)
         si_gfx_blit(pipe, &blit);
      else
         pipe->blit(pipe, &blit);
   }
}

/* Copy all planes of multi-plane texture */
static bool si_copy_multi_plane_texture(struct pipe_context *ctx, struct pipe_resource *dst,
                                        unsigned dst_level, unsigned dstx, unsigned dsty, unsigned dstz,
                                        struct pipe_resource *src, unsigned src_level,
                                        const struct pipe_box *src_box)
{
   unsigned i, dx, dy;
   struct si_texture *src_tex = (struct si_texture *)src;
   struct si_texture *dst_tex = (struct si_texture *)dst;
   struct pipe_box sbox;

   if (src_tex->multi_plane_format == PIPE_FORMAT_NONE || src_tex->plane_index != 0)
      return false;

   assert(src_tex->multi_plane_format == dst_tex->multi_plane_format);
   assert(dst_tex->plane_index == 0 && src_tex->num_planes == dst_tex->num_planes);

   sbox = *src_box;

   for (i = 0; i < src_tex->num_planes && src && dst; ++i) {
      dx = util_format_get_plane_width(src_tex->multi_plane_format, i, dstx);
      dy = util_format_get_plane_height(src_tex->multi_plane_format, i, dsty);
      sbox.x = util_format_get_plane_width(src_tex->multi_plane_format, i, src_box->x);
      sbox.y = util_format_get_plane_height(src_tex->multi_plane_format, i, src_box->y);
      sbox.width = util_format_get_plane_width(src_tex->multi_plane_format, i, src_box->width);
      sbox.height = util_format_get_plane_height(src_tex->multi_plane_format, i, src_box->height);

      si_resource_copy_region(ctx, dst, dst_level, dx, dy, dstz, src, src_level, &sbox);

      src = src->next;
      dst = dst->next;
   }

   return true;
}

/* Copy from a full GPU texture to a transfer's staging one. */
static void si_copy_to_staging_texture(struct pipe_context *ctx, struct si_transfer *stransfer)
{
   struct pipe_transfer *transfer = (struct pipe_transfer *)stransfer;
   struct pipe_resource *dst = &stransfer->staging->b.b;
   struct pipe_resource *src = transfer->resource;
   /* level means sample_index - 1 with MSAA. Used by texture uploads. */
   unsigned src_level = src->nr_samples > 1 ? 0 : transfer->level;

   if (src->nr_samples > 1 || ((struct si_texture *)src)->is_depth) {
      si_copy_region_with_blit(ctx, dst, 0, 0, 0, 0, 0, src, src_level, &transfer->box);
      return;
   }

   if (si_copy_multi_plane_texture(ctx, dst, 0, 0, 0, 0, src, src_level, &transfer->box))
      return;

   si_resource_copy_region(ctx, dst, 0, 0, 0, 0, src, src_level, &transfer->box);
}

/* Copy from a transfer's staging texture to a full GPU one. */
static void si_copy_from_staging_texture(struct pipe_context *ctx, struct si_transfer *stransfer)
{
   struct pipe_transfer *transfer = (struct pipe_transfer *)stransfer;
   struct pipe_resource *dst = transfer->resource;
   struct pipe_resource *src = &stransfer->staging->b.b;
   struct pipe_box sbox;

   u_box_3d(0, 0, 0, transfer->box.width, transfer->box.height, transfer->box.depth, &sbox);

   if (dst->nr_samples > 1 || ((struct si_texture *)dst)->is_depth) {
      unsigned dst_level = dst->nr_samples > 1 ? 0 : transfer->level;
      unsigned dst_sample = dst->nr_samples > 1 ? transfer->level : 0;

      si_copy_region_with_blit(ctx, dst, dst_level, dst_sample, transfer->box.x, transfer->box.y,
                               transfer->box.z, src, 0, &sbox);
      return;
   }

   if (si_copy_multi_plane_texture(ctx, dst, transfer->level, transfer->box.x, transfer->box.y,
                                   transfer->box.z, src, 0, &sbox))
      return;

   if (util_format_is_compressed(dst->format)) {
      sbox.width = util_format_get_nblocksx(dst->format, sbox.width);
      sbox.height = util_format_get_nblocksx(dst->format, sbox.height);
   }

   si_resource_copy_region(ctx, dst, transfer->level, transfer->box.x, transfer->box.y,
                           transfer->box.z, src, 0, &sbox);
}

static uint64_t si_texture_get_offset(struct si_screen *sscreen, struct si_texture *tex,
                                      unsigned level, const struct pipe_box *box, unsigned *stride,
                                      uintptr_t *layer_stride)
{
   if (sscreen->info.gfx_level >= GFX9) {
      unsigned pitch;
      if (tex->surface.is_linear) {
         pitch = tex->surface.u.gfx9.pitch[level];
      } else {
         pitch = tex->surface.u.gfx9.surf_pitch;
      }

      *stride = pitch * tex->surface.bpe;
      *layer_stride = tex->surface.u.gfx9.surf_slice_size;

      if (!box)
         return 0;

      /* Each texture is an array of slices. Each slice is an array
       * of mipmap levels. */
      return tex->surface.u.gfx9.surf_offset + box->z * tex->surface.u.gfx9.surf_slice_size +
             tex->surface.u.gfx9.offset[level] +
             (box->y / tex->surface.blk_h * (uint64_t)pitch + box->x / tex->surface.blk_w) *
             tex->surface.bpe;
   } else {
      *stride = tex->surface.u.legacy.level[level].nblk_x * tex->surface.bpe;
      assert((uint64_t)tex->surface.u.legacy.level[level].slice_size_dw * 4 <= UINT_MAX);
      *layer_stride = (uint64_t)tex->surface.u.legacy.level[level].slice_size_dw * 4;

      if (!box)
         return (uint64_t)tex->surface.u.legacy.level[level].offset_256B * 256;

      /* Each texture is an array of mipmap levels. Each level is
       * an array of slices. */
      return (uint64_t)tex->surface.u.legacy.level[level].offset_256B * 256 +
             box->z * (uint64_t)tex->surface.u.legacy.level[level].slice_size_dw * 4 +
             (box->y / tex->surface.blk_h * tex->surface.u.legacy.level[level].nblk_x +
              box->x / tex->surface.blk_w) *
                tex->surface.bpe;
   }
}

static int si_init_surface(struct si_screen *sscreen, struct radeon_surf *surface,
                           const struct pipe_resource *ptex, enum radeon_surf_mode array_mode,
                           uint64_t modifier, bool is_imported, bool is_scanout,
                           bool is_flushed_depth, bool tc_compatible_htile)
{
   const struct util_format_description *desc = util_format_description(ptex->format);
   bool is_depth = util_format_has_depth(desc);
   bool is_stencil = util_format_has_stencil(desc);
   int r;
   unsigned bpe;
   uint64_t flags = 0;

   if (!is_flushed_depth && ptex->format == PIPE_FORMAT_Z32_FLOAT_S8X24_UINT) {
      bpe = 4; /* stencil is allocated separately */
   } else {
      bpe = util_format_get_blocksize(ptex->format);
      assert(util_is_power_of_two_or_zero(bpe));
   }

   if (!is_flushed_depth && is_depth) {
      flags |= RADEON_SURF_ZBUFFER;

      if ((sscreen->debug_flags & DBG(NO_HYPERZ)) ||
          (ptex->bind & PIPE_BIND_SHARED) || is_imported) {
         flags |= RADEON_SURF_NO_HTILE;
      } else if (tc_compatible_htile &&
                 (sscreen->info.gfx_level >= GFX9 || array_mode == RADEON_SURF_MODE_2D)) {
         /* TC-compatible HTILE only supports Z32_FLOAT.
          * GFX9 also supports Z16_UNORM.
          * On GFX8, promote Z16 to Z32. DB->CB copies will convert
          * the format for transfers.
          */
         if (sscreen->info.gfx_level == GFX8)
            bpe = 4;

         flags |= RADEON_SURF_TC_COMPATIBLE_HTILE;
      }

      if (is_stencil)
         flags |= RADEON_SURF_SBUFFER;
   }

   /* Disable DCC? (it can't be disabled if modifiers are used) */
   if (sscreen->info.gfx_level >= GFX8 && modifier == DRM_FORMAT_MOD_INVALID && !is_imported) {
      /* Global options that disable DCC. */
      if (ptex->flags & SI_RESOURCE_FLAG_DISABLE_DCC)
         flags |= RADEON_SURF_DISABLE_DCC;

      if (ptex->nr_samples >= 2 && sscreen->debug_flags & DBG(NO_DCC_MSAA))
         flags |= RADEON_SURF_DISABLE_DCC;

      /* Shared textures must always set up DCC. If it's not present, it will be disabled by
       * si_get_opaque_metadata later.
       */
      if (!is_imported && sscreen->debug_flags & DBG(NO_DCC))
         flags |= RADEON_SURF_DISABLE_DCC;

      /* R9G9B9E5 isn't supported for rendering by older generations. */
      if (sscreen->info.gfx_level < GFX10_3 &&
          ptex->format == PIPE_FORMAT_R9G9B9E5_FLOAT)
         flags |= RADEON_SURF_DISABLE_DCC;

      /* If constant (non-data-dependent) format is requested, disable DCC: */
      if (ptex->bind & PIPE_BIND_CONST_BW)
         flags |= RADEON_SURF_DISABLE_DCC;

      switch (sscreen->info.gfx_level) {
      case GFX8:
         /* Stoney: 128bpp MSAA textures randomly fail piglit tests with DCC. */
         if (sscreen->info.family == CHIP_STONEY && bpe == 16 && ptex->nr_samples >= 2)
            flags |= RADEON_SURF_DISABLE_DCC;

         /* DCC clear for 4x and 8x MSAA array textures unimplemented. */
         if (ptex->nr_storage_samples >= 4 && ptex->array_size > 1)
            flags |= RADEON_SURF_DISABLE_DCC;
         break;

      case GFX9:
         /* DCC MSAA fails this on Raven:
          *    https://www.khronos.org/registry/webgl/sdk/tests/deqp/functional/gles3/fbomultisample.2_samples.html
          * and this on Picasso:
          *    https://www.khronos.org/registry/webgl/sdk/tests/deqp/functional/gles3/fbomultisample.4_samples.html
          */
         if (sscreen->info.family == CHIP_RAVEN && ptex->nr_storage_samples >= 2 && bpe < 4)
            flags |= RADEON_SURF_DISABLE_DCC;
         break;

      case GFX10:
      case GFX10_3:
         if (ptex->nr_storage_samples >= 2 && !sscreen->options.dcc_msaa)
            flags |= RADEON_SURF_DISABLE_DCC;
         break;

      case GFX11:
      case GFX11_5:
         break;

      default:
         assert(0);
      }
   }

   if (is_scanout) {
      /* This should catch bugs in gallium users setting incorrect flags. */
      assert(ptex->nr_samples <= 1 && ptex->depth0 == 1 &&
             ptex->last_level == 0 && !(flags & RADEON_SURF_Z_OR_SBUFFER));

      flags |= RADEON_SURF_SCANOUT;
   }

   if (ptex->bind & PIPE_BIND_SHARED)
      flags |= RADEON_SURF_SHAREABLE;
   if (is_imported)
      flags |= RADEON_SURF_IMPORTED | RADEON_SURF_SHAREABLE;
   if (sscreen->debug_flags & DBG(NO_FMASK))
      flags |= RADEON_SURF_NO_FMASK;

   if (sscreen->info.gfx_level == GFX9 && (ptex->flags & SI_RESOURCE_FLAG_FORCE_MICRO_TILE_MODE)) {
      flags |= RADEON_SURF_FORCE_MICRO_TILE_MODE;
      surface->micro_tile_mode = SI_RESOURCE_FLAG_MICRO_TILE_MODE_GET(ptex->flags);
   }

   if (ptex->flags & SI_RESOURCE_FLAG_FORCE_MSAA_TILING) {
      /* GFX11 shouldn't get here because the flag is only used by the CB MSAA resolving
       * that GFX11 doesn't have.
       */
      assert(sscreen->info.gfx_level <= GFX10_3);

      flags |= RADEON_SURF_FORCE_SWIZZLE_MODE;

      if (sscreen->info.gfx_level >= GFX10)
         surface->u.gfx9.swizzle_mode = ADDR_SW_64KB_R_X;
   }

   if (ptex->flags & PIPE_RESOURCE_FLAG_SPARSE) {
      flags |=
         RADEON_SURF_PRT |
         RADEON_SURF_NO_FMASK |
         RADEON_SURF_NO_HTILE |
         RADEON_SURF_DISABLE_DCC;
   }

   surface->modifier = modifier;

   r = sscreen->ws->surface_init(sscreen->ws, &sscreen->info, ptex, flags, bpe, array_mode,
                                 surface);
   if (r) {
      return r;
   }

   return 0;
}

void si_eliminate_fast_color_clear(struct si_context *sctx, struct si_texture *tex,
                                   bool *ctx_flushed)
{
   struct pipe_context *ctx = &sctx->b;

   unsigned n = sctx->num_decompress_calls;
   ctx->flush_resource(ctx, &tex->buffer.b.b);

   /* Flush only if any fast clear elimination took place. */
   bool flushed = false;
   if (n != sctx->num_decompress_calls)
   {
      ctx->flush(ctx, NULL, 0);
      flushed = true;
   }
   if (ctx_flushed)
      *ctx_flushed = flushed;
}

void si_texture_discard_cmask(struct si_screen *sscreen, struct si_texture *tex)
{
   if (!tex->cmask_buffer)
      return;

   assert(tex->buffer.b.b.nr_samples <= 1);

   /* Disable CMASK. */
   tex->cmask_base_address_reg = tex->buffer.gpu_address >> 8;
   tex->dirty_level_mask = 0;

   tex->cb_color_info &= ~S_028C70_FAST_CLEAR(1);

   if (tex->cmask_buffer != &tex->buffer)
      si_resource_reference(&tex->cmask_buffer, NULL);

   tex->cmask_buffer = NULL;

   /* Notify all contexts about the change. */
   p_atomic_inc(&sscreen->dirty_tex_counter);
   p_atomic_inc(&sscreen->compressed_colortex_counter);
}

static bool si_can_disable_dcc(struct si_texture *tex)
{
   /* We can't disable DCC if it can be written by another process. */
   return !tex->is_depth &&
          tex->surface.meta_offset &&
          (!tex->buffer.b.is_shared ||
           !(tex->buffer.external_usage & PIPE_HANDLE_USAGE_FRAMEBUFFER_WRITE)) &&
          !ac_modifier_has_dcc(tex->surface.modifier);
}

static bool si_texture_discard_dcc(struct si_screen *sscreen, struct si_texture *tex)
{
   if (!si_can_disable_dcc(tex))
      return false;

   /* Disable DCC. */
   ac_surface_zero_dcc_fields(&tex->surface);

   /* Notify all contexts about the change. */
   p_atomic_inc(&sscreen->dirty_tex_counter);
   return true;
}

/**
 * Disable DCC for the texture. (first decompress, then discard metadata).
 *
 * There is unresolved multi-context synchronization issue between
 * screen::aux_context and the current context. If applications do this with
 * multiple contexts, it's already undefined behavior for them and we don't
 * have to worry about that. The scenario is:
 *
 * If context 1 disables DCC and context 2 has queued commands that write
 * to the texture via CB with DCC enabled, and the order of operations is
 * as follows:
 *   context 2 queues draw calls rendering to the texture, but doesn't flush
 *   context 1 disables DCC and flushes
 *   context 1 & 2 reset descriptors and FB state
 *   context 2 flushes (new compressed tiles written by the draw calls)
 *   context 1 & 2 read garbage, because DCC is disabled, yet there are
 *   compressed tiled
 *
 * \param sctx  the current context if you have one, or sscreen->aux_context
 *              if you don't.
 */
bool si_texture_disable_dcc(struct si_context *sctx, struct si_texture *tex)
{
   struct si_screen *sscreen = sctx->screen;

   if (!sctx->has_graphics)
      return si_texture_discard_dcc(sscreen, tex);

   if (!si_can_disable_dcc(tex))
      return false;

   /* Decompress DCC. */
   si_decompress_dcc(sctx, tex);
   sctx->b.flush(&sctx->b, NULL, 0);

   return si_texture_discard_dcc(sscreen, tex);
}

static void si_reallocate_texture_inplace(struct si_context *sctx, struct si_texture *tex,
                                          unsigned new_bind_flag, bool invalidate_storage)
{
   struct pipe_screen *screen = sctx->b.screen;
   struct si_texture *new_tex;
   struct pipe_resource templ = tex->buffer.b.b;
   unsigned i;

   templ.bind |= new_bind_flag;

   if (tex->buffer.b.is_shared || tex->num_planes > 1)
      return;

   if (new_bind_flag == PIPE_BIND_LINEAR) {
      if (tex->surface.is_linear)
         return;

      /* This fails with MSAA, depth, and compressed textures. */
      if (si_choose_tiling(sctx->screen, &templ, false) != RADEON_SURF_MODE_LINEAR_ALIGNED)
         return;
   }

   /* Inherit the modifier from the old texture. */
   if (tex->surface.modifier != DRM_FORMAT_MOD_INVALID && screen->resource_create_with_modifiers)
      new_tex = (struct si_texture *)screen->resource_create_with_modifiers(screen, &templ,
                                                                            &tex->surface.modifier, 1);
   else
      new_tex = (struct si_texture *)screen->resource_create(screen, &templ);

   if (!new_tex)
      return;

   /* Copy the pixels to the new texture. */
   if (!invalidate_storage) {
      for (i = 0; i <= templ.last_level; i++) {
         struct pipe_box box;

         u_box_3d(0, 0, 0, u_minify(templ.width0, i), u_minify(templ.height0, i),
                  util_num_layers(&templ, i), &box);

         si_resource_copy_region(&sctx->b, &new_tex->buffer.b.b,
                                 i, 0, 0, 0, &tex->buffer.b.b, i, &box);
      }
   }

   if (new_bind_flag == PIPE_BIND_LINEAR) {
      si_texture_discard_cmask(sctx->screen, tex);
      si_texture_discard_dcc(sctx->screen, tex);
   }

   /* Replace the structure fields of tex. */
   tex->buffer.b.b.bind = templ.bind;
   radeon_bo_reference(sctx->screen->ws, &tex->buffer.buf, new_tex->buffer.buf);
   tex->buffer.gpu_address = new_tex->buffer.gpu_address;
   tex->buffer.bo_size = new_tex->buffer.bo_size;
   tex->buffer.bo_alignment_log2 = new_tex->buffer.bo_alignment_log2;
   tex->buffer.domains = new_tex->buffer.domains;
   tex->buffer.flags = new_tex->buffer.flags;

   tex->surface = new_tex->surface;
   si_texture_reference(&tex->flushed_depth_texture, new_tex->flushed_depth_texture);

   tex->surface.fmask_offset = new_tex->surface.fmask_offset;
   tex->surface.cmask_offset = new_tex->surface.cmask_offset;
   tex->cmask_base_address_reg = new_tex->cmask_base_address_reg;

   if (tex->cmask_buffer == &tex->buffer)
      tex->cmask_buffer = NULL;
   else
      si_resource_reference(&tex->cmask_buffer, NULL);

   if (new_tex->cmask_buffer == &new_tex->buffer)
      tex->cmask_buffer = &tex->buffer;
   else
      si_resource_reference(&tex->cmask_buffer, new_tex->cmask_buffer);

   tex->surface.meta_offset = new_tex->surface.meta_offset;
   tex->cb_color_info = new_tex->cb_color_info;
   memcpy(tex->color_clear_value, new_tex->color_clear_value, sizeof(tex->color_clear_value));
   tex->last_msaa_resolve_target_micro_mode = new_tex->last_msaa_resolve_target_micro_mode;

   memcpy(tex->depth_clear_value, new_tex->depth_clear_value, sizeof(tex->depth_clear_value));
   tex->dirty_level_mask = new_tex->dirty_level_mask;
   tex->stencil_dirty_level_mask = new_tex->stencil_dirty_level_mask;
   tex->db_render_format = new_tex->db_render_format;
   memcpy(tex->stencil_clear_value, new_tex->stencil_clear_value, sizeof(tex->stencil_clear_value));
   tex->tc_compatible_htile = new_tex->tc_compatible_htile;
   tex->depth_cleared_level_mask_once = new_tex->depth_cleared_level_mask_once;
   tex->stencil_cleared_level_mask_once = new_tex->stencil_cleared_level_mask_once;
   tex->upgraded_depth = new_tex->upgraded_depth;
   tex->db_compatible = new_tex->db_compatible;
   tex->can_sample_z = new_tex->can_sample_z;
   tex->can_sample_s = new_tex->can_sample_s;

   tex->displayable_dcc_dirty = new_tex->displayable_dcc_dirty;

   if (new_bind_flag == PIPE_BIND_LINEAR) {
      assert(!tex->surface.meta_offset);
      assert(!tex->cmask_buffer);
      assert(!tex->surface.fmask_size);
      assert(!tex->is_depth);
   }

   si_texture_reference(&new_tex, NULL);

   p_atomic_inc(&sctx->screen->dirty_tex_counter);
}

static void si_set_tex_bo_metadata(struct si_screen *sscreen, struct si_texture *tex)
{
   struct pipe_resource *res = &tex->buffer.b.b;
   struct radeon_bo_metadata md;

   memset(&md, 0, sizeof(md));

   assert(tex->surface.fmask_size == 0);

   static const unsigned char swizzle[] = {PIPE_SWIZZLE_X, PIPE_SWIZZLE_Y, PIPE_SWIZZLE_Z,
                                           PIPE_SWIZZLE_W};
   bool is_array = util_texture_is_array(res->target);
   uint32_t desc[8];

   sscreen->make_texture_descriptor(sscreen, tex, true, res->target, res->format, swizzle, 0,
                                    res->last_level, 0, is_array ? res->array_size - 1 : 0,
                                    res->width0, res->height0, res->depth0, true, desc, NULL);
   si_set_mutable_tex_desc_fields(sscreen, tex, &tex->surface.u.legacy.level[0], 0, 0,
                                  tex->surface.blk_w, false, 0, desc);

   ac_surface_compute_umd_metadata(&sscreen->info, &tex->surface,
                                   tex->buffer.b.b.last_level + 1,
                                   desc, &md.size_metadata, md.metadata,
                                   sscreen->debug_flags & DBG(EXTRA_METADATA));
   sscreen->ws->buffer_set_metadata(sscreen->ws, tex->buffer.buf, &md, &tex->surface);
}

static bool si_displayable_dcc_needs_explicit_flush(struct si_texture *tex)
{
   struct si_screen *sscreen = (struct si_screen *)tex->buffer.b.b.screen;

   if (sscreen->info.gfx_level <= GFX8)
      return false;

   /* With modifiers and > 1 planes any applications will know that they
    * cannot do frontbuffer rendering with the texture. */
   if (ac_surface_get_nplanes(&tex->surface) > 1)
      return false;

   return tex->surface.is_displayable && tex->surface.meta_offset;
}

static bool si_resource_get_param(struct pipe_screen *screen, struct pipe_context *context,
                                  struct pipe_resource *resource, unsigned plane, unsigned layer,
                                  unsigned level,
                                  enum pipe_resource_param param, unsigned handle_usage,
                                  uint64_t *value)
{
   while (plane && resource->next && !si_texture_is_aux_plane(resource->next)) {
      --plane;
      resource = resource->next;
   }

   struct si_screen *sscreen = (struct si_screen *)screen;
   struct si_texture *tex = (struct si_texture *)resource;
   struct winsys_handle whandle;

   switch (param) {
   case PIPE_RESOURCE_PARAM_NPLANES:
      if (resource->target == PIPE_BUFFER)
         *value = 1;
      else if (tex->num_planes > 1)
         *value = tex->num_planes;
      else
         *value = ac_surface_get_nplanes(&tex->surface);
      return true;

   case PIPE_RESOURCE_PARAM_STRIDE:
      if (resource->target == PIPE_BUFFER)
         *value = 0;
      else
         *value = ac_surface_get_plane_stride(sscreen->info.gfx_level,
                                              &tex->surface, plane, level);
      return true;

   case PIPE_RESOURCE_PARAM_OFFSET:
      if (resource->target == PIPE_BUFFER) {
         *value = 0;
      } else {
         uint64_t level_offset = 0;
         if (sscreen->info.gfx_level >= GFX9 && tex->surface.is_linear)
            level_offset = tex->surface.u.gfx9.offset[level];
         *value = ac_surface_get_plane_offset(sscreen->info.gfx_level,
                                              &tex->surface, plane, layer)  + level_offset;
      }
      return true;

   case PIPE_RESOURCE_PARAM_MODIFIER:
      *value = tex->surface.modifier;
      return true;

   case PIPE_RESOURCE_PARAM_HANDLE_TYPE_SHARED:
   case PIPE_RESOURCE_PARAM_HANDLE_TYPE_KMS:
   case PIPE_RESOURCE_PARAM_HANDLE_TYPE_FD:
      memset(&whandle, 0, sizeof(whandle));

      if (param == PIPE_RESOURCE_PARAM_HANDLE_TYPE_SHARED)
         whandle.type = WINSYS_HANDLE_TYPE_SHARED;
      else if (param == PIPE_RESOURCE_PARAM_HANDLE_TYPE_KMS)
         whandle.type = WINSYS_HANDLE_TYPE_KMS;
      else if (param == PIPE_RESOURCE_PARAM_HANDLE_TYPE_FD)
         whandle.type = WINSYS_HANDLE_TYPE_FD;

      if (!screen->resource_get_handle(screen, context, resource, &whandle, handle_usage))
         return false;

      *value = whandle.handle;
      return true;
   case PIPE_RESOURCE_PARAM_LAYER_STRIDE:
      break;
   }
   return false;
}

static void si_texture_get_info(struct pipe_screen *screen, struct pipe_resource *resource,
                                unsigned *pstride, unsigned *poffset)
{
   uint64_t value;

   if (pstride) {
      si_resource_get_param(screen, NULL, resource, 0, 0, 0, PIPE_RESOURCE_PARAM_STRIDE, 0, &value);
      *pstride = value;
   }

   if (poffset) {
      si_resource_get_param(screen, NULL, resource, 0, 0, 0, PIPE_RESOURCE_PARAM_OFFSET, 0, &value);
      *poffset = value;
   }
}

static bool si_texture_get_handle(struct pipe_screen *screen, struct pipe_context *ctx,
                                  struct pipe_resource *resource, struct winsys_handle *whandle,
                                  unsigned usage)
{
   struct si_screen *sscreen = (struct si_screen *)screen;
   struct si_context *sctx;
   struct si_resource *res = si_resource(resource);
   struct si_texture *tex = (struct si_texture *)resource;
   bool update_metadata = false;
   unsigned stride, offset, slice_size;
   uint64_t modifier = DRM_FORMAT_MOD_INVALID;
   bool flush = false;

   ctx = threaded_context_unwrap_sync(ctx);
   sctx = ctx ? (struct si_context *)ctx : si_get_aux_context(&sscreen->aux_context.general);

   if (resource->target != PIPE_BUFFER) {
      unsigned plane = whandle->plane;

      /* Individual planes are chained pipe_resource instances. */
      while (plane && resource->next && !si_texture_is_aux_plane(resource->next)) {
         resource = resource->next;
         --plane;
      }

      res = si_resource(resource);
      tex = (struct si_texture *)resource;

      /* This is not supported now, but it might be required for OpenCL
       * interop in the future.
       */
      if (resource->nr_samples > 1 || tex->is_depth) {
         if (!ctx)
            si_put_aux_context_flush(&sscreen->aux_context.general);
         return false;
      }

      whandle->size = tex->buffer.bo_size;

      if (plane) {
         if (!ctx)
            si_put_aux_context_flush(&sscreen->aux_context.general);
         whandle->offset = ac_surface_get_plane_offset(sscreen->info.gfx_level,
                                                       &tex->surface, plane, 0);
         whandle->stride = ac_surface_get_plane_stride(sscreen->info.gfx_level,
                                                       &tex->surface, plane, 0);
         whandle->modifier = tex->surface.modifier;
         return sscreen->ws->buffer_get_handle(sscreen->ws, res->buf, whandle);
      }

      /* Move a suballocated texture into a non-suballocated allocation. */
      if (sscreen->ws->buffer_is_suballocated(res->buf) || tex->surface.tile_swizzle ||
          (tex->buffer.flags & RADEON_FLAG_NO_INTERPROCESS_SHARING &&
           sscreen->info.has_local_buffers)) {
         assert(!res->b.is_shared);
         si_reallocate_texture_inplace(sctx, tex, PIPE_BIND_SHARED, false);
         flush = true;
         assert(res->b.b.bind & PIPE_BIND_SHARED);
         assert(res->flags & RADEON_FLAG_NO_SUBALLOC);
         assert(!(res->flags & RADEON_FLAG_NO_INTERPROCESS_SHARING));
         assert(tex->surface.tile_swizzle == 0);
      }

      /* Since shader image stores don't support DCC on GFX8,
       * disable it for external clients that want write
       * access.
       */
      if (sscreen->debug_flags & DBG(NO_EXPORTED_DCC) ||
          (usage & PIPE_HANDLE_USAGE_SHADER_WRITE && !tex->is_depth && tex->surface.meta_offset) ||
          /* Displayable DCC requires an explicit flush. */
          (!(usage & PIPE_HANDLE_USAGE_EXPLICIT_FLUSH) &&
           si_displayable_dcc_needs_explicit_flush(tex))) {
         if (si_texture_disable_dcc(sctx, tex)) {
            update_metadata = true;
            /* si_texture_disable_dcc flushes the context */
            flush = false;
         }
      }

      if (!(usage & PIPE_HANDLE_USAGE_EXPLICIT_FLUSH) &&
          (tex->cmask_buffer || (!tex->is_depth && tex->surface.meta_offset))) {
         /* Eliminate fast clear (both CMASK and DCC) */
         bool flushed;
         si_eliminate_fast_color_clear(sctx, tex, &flushed);
         /* eliminate_fast_color_clear sometimes flushes the context */
         flush = !flushed;

         /* Disable CMASK if flush_resource isn't going
          * to be called.
          */
         if (tex->cmask_buffer)
            si_texture_discard_cmask(sscreen, tex);
      }

      /* Set metadata. */
      if ((!res->b.is_shared || update_metadata) && whandle->offset == 0)
         si_set_tex_bo_metadata(sscreen, tex);

      if (sscreen->info.gfx_level >= GFX9) {
         slice_size = tex->surface.u.gfx9.surf_slice_size;
      } else {
         slice_size = (uint64_t)tex->surface.u.legacy.level[0].slice_size_dw * 4;
      }

      modifier = tex->surface.modifier;
   } else {
      tc_buffer_disable_cpu_storage(&res->b.b);

      /* Buffer exports are for the OpenCL interop. */
      /* Move a suballocated buffer into a non-suballocated allocation. */
      if (sscreen->ws->buffer_is_suballocated(res->buf) ||
          /* A DMABUF export always fails if the BO is local. */
          (tex->buffer.flags & RADEON_FLAG_NO_INTERPROCESS_SHARING &&
           sscreen->info.has_local_buffers)) {
         assert(!res->b.is_shared);

         /* Allocate a new buffer with PIPE_BIND_SHARED. */
         struct pipe_resource templ = res->b.b;
         templ.bind |= PIPE_BIND_SHARED;

         struct pipe_resource *newb = screen->resource_create(screen, &templ);
         if (!newb) {
            if (!ctx)
               si_put_aux_context_flush(&sscreen->aux_context.general);
            return false;
         }

         /* Copy the old buffer contents to the new one. */
         struct pipe_box box;
         u_box_1d(0, newb->width0, &box);
         sctx->b.resource_copy_region(&sctx->b, newb, 0, 0, 0, 0, &res->b.b, 0, &box);
         flush = true;
         /* Move the new buffer storage to the old pipe_resource. */
         si_replace_buffer_storage(&sctx->b, &res->b.b, newb, 0, 0, 0);
         pipe_resource_reference(&newb, NULL);

         assert(res->b.b.bind & PIPE_BIND_SHARED);
         assert(res->flags & RADEON_FLAG_NO_SUBALLOC);
      }

      /* Buffers */
      slice_size = 0;
   }

   si_texture_get_info(screen, resource, &stride, &offset);

   if (res->b.is_shared) {
      /* USAGE_EXPLICIT_FLUSH must be cleared if at least one user
       * doesn't set it.
       */
      res->external_usage |= usage & ~PIPE_HANDLE_USAGE_EXPLICIT_FLUSH;
      if (!(usage & PIPE_HANDLE_USAGE_EXPLICIT_FLUSH))
         res->external_usage &= ~PIPE_HANDLE_USAGE_EXPLICIT_FLUSH;
   } else {
      res->b.is_shared = true;
      res->external_usage = usage;
   }

   if (flush && ctx)
      sctx->b.flush(&sctx->b, NULL, 0);
   if (!ctx)
      si_put_aux_context_flush(&sscreen->aux_context.general);

   whandle->stride = stride;
   whandle->offset = offset + slice_size * whandle->layer;
   whandle->modifier = modifier;

   return sscreen->ws->buffer_get_handle(sscreen->ws, res->buf, whandle);
}

void si_print_texture_info(struct si_screen *sscreen, struct si_texture *tex,
                           struct u_log_context *log)
{
   int i;
   FILE *f;
   char *surf_info = NULL;
   size_t surf_info_size;

   /* Common parameters. */
   u_log_printf(log,
                "  Info: npix_x=%u, npix_y=%u, npix_z=%u, "
                "array_size=%u, last_level=%u, nsamples=%u",
                tex->buffer.b.b.width0, tex->buffer.b.b.height0,
                tex->buffer.b.b.depth0, tex->buffer.b.b.array_size,
                tex->buffer.b.b.last_level, tex->buffer.b.b.nr_samples);

   if (tex->is_depth && tex->surface.meta_offset)
      u_log_printf(log, ", tc_compatible_htile=%u", tex->tc_compatible_htile);

   u_log_printf(log, ", %s\n",
                util_format_short_name(tex->buffer.b.b.format));

   f = open_memstream(&surf_info, &surf_info_size);
   if (!f)
      return;
   ac_surface_print_info(f, &sscreen->info, &tex->surface);
   fclose(f);
   u_log_printf(log, "%s", surf_info);
   free(surf_info);

   if (sscreen->info.gfx_level >= GFX9) {
      return;
   }

   if (!tex->is_depth && tex->surface.meta_offset) {
      for (i = 0; i <= tex->buffer.b.b.last_level; i++)
         u_log_printf(log,
                      "    DCCLevel[%i]: enabled=%u, offset=%u, "
                      "fast_clear_size=%u\n",
                      i, i < tex->surface.num_meta_levels, tex->surface.u.legacy.color.dcc_level[i].dcc_offset,
                      tex->surface.u.legacy.color.dcc_level[i].dcc_fast_clear_size);
   }

   for (i = 0; i <= tex->buffer.b.b.last_level; i++)
      u_log_printf(log,
                   "    Level[%i]: offset=%" PRIu64 ", slice_size=%" PRIu64 ", "
                   "npix_x=%u, npix_y=%u, npix_z=%u, nblk_x=%u, nblk_y=%u, "
                   "mode=%u, tiling_index = %u\n",
                   i, (uint64_t)tex->surface.u.legacy.level[i].offset_256B * 256,
                   (uint64_t)tex->surface.u.legacy.level[i].slice_size_dw * 4,
                   u_minify(tex->buffer.b.b.width0, i), u_minify(tex->buffer.b.b.height0, i),
                   u_minify(tex->buffer.b.b.depth0, i), tex->surface.u.legacy.level[i].nblk_x,
                   tex->surface.u.legacy.level[i].nblk_y, tex->surface.u.legacy.level[i].mode,
                   tex->surface.u.legacy.tiling_index[i]);

   if (tex->surface.has_stencil) {
      for (i = 0; i <= tex->buffer.b.b.last_level; i++) {
         u_log_printf(log,
                      "    StencilLevel[%i]: offset=%" PRIu64 ", "
                      "slice_size=%" PRIu64 ", npix_x=%u, "
                      "npix_y=%u, npix_z=%u, nblk_x=%u, nblk_y=%u, "
                      "mode=%u, tiling_index = %u\n",
                      i, (uint64_t)tex->surface.u.legacy.zs.stencil_level[i].offset_256B * 256,
                      (uint64_t)tex->surface.u.legacy.zs.stencil_level[i].slice_size_dw * 4,
                      u_minify(tex->buffer.b.b.width0, i), u_minify(tex->buffer.b.b.height0, i),
                      u_minify(tex->buffer.b.b.depth0, i),
                      tex->surface.u.legacy.zs.stencil_level[i].nblk_x,
                      tex->surface.u.legacy.zs.stencil_level[i].nblk_y,
                      tex->surface.u.legacy.zs.stencil_level[i].mode,
                      tex->surface.u.legacy.zs.stencil_tiling_index[i]);
      }
   }
}

/**
 * Common function for si_texture_create and si_texture_from_handle.
 *
 * \param screen       screen
 * \param base         resource template
 * \param surface      radeon_surf
 * \param plane0       if a non-zero plane is being created, this is the first plane
 * \param imported_buf from si_texture_from_handle
 * \param offset       offset for non-zero planes or imported buffers
 * \param alloc_size   the size to allocate if plane0 != NULL
 * \param alignment    alignment for the allocation
 */
static struct si_texture *si_texture_create_object(struct pipe_screen *screen,
                                                   const struct pipe_resource *base,
                                                   const struct radeon_surf *surface,
                                                   const struct si_texture *plane0,
                                                   struct pb_buffer_lean *imported_buf,
                                                   uint64_t offset, unsigned pitch_in_bytes,
                                                   uint64_t alloc_size, unsigned alignment)
{
   struct si_texture *tex;
   struct si_resource *resource;
   struct si_screen *sscreen = (struct si_screen *)screen;

   if (!sscreen->info.has_3d_cube_border_color_mipmap &&
       (base->last_level > 0 ||
        base->target == PIPE_TEXTURE_3D ||
        base->target == PIPE_TEXTURE_CUBE)) {
      assert(0);
      return NULL;
   }

   tex = CALLOC_STRUCT_CL(si_texture);
   if (!tex)
      goto error;

   resource = &tex->buffer;
   resource->b.b = *base;
   pipe_reference_init(&resource->b.b.reference, 1);
   resource->b.b.screen = screen;

   /* don't include stencil-only formats which we don't support for rendering */
   tex->is_depth = util_format_has_depth(util_format_description(tex->buffer.b.b.format));
   tex->surface = *surface;

   if (!ac_surface_override_offset_stride(&sscreen->info, &tex->surface,
                                          tex->buffer.b.b.array_size,
                                          tex->buffer.b.b.last_level + 1,
                                          offset, pitch_in_bytes / tex->surface.bpe))
      goto error;

   if (plane0) {
      /* The buffer is shared with the first plane. */
      resource->bo_size = plane0->buffer.bo_size;
      resource->bo_alignment_log2 = plane0->buffer.bo_alignment_log2;
      resource->flags = plane0->buffer.flags;
      resource->domains = plane0->buffer.domains;

      radeon_bo_reference(sscreen->ws, &resource->buf, plane0->buffer.buf);
      resource->gpu_address = plane0->buffer.gpu_address;
   } else if (!(surface->flags & RADEON_SURF_IMPORTED)) {
      if (base->flags & PIPE_RESOURCE_FLAG_SPARSE)
         resource->b.b.flags |= PIPE_RESOURCE_FLAG_UNMAPPABLE;
      if (base->bind & PIPE_BIND_PRIME_BLIT_DST)
         resource->b.b.flags |= SI_RESOURCE_FLAG_GL2_BYPASS;

      /* Create the backing buffer. */
      si_init_resource_fields(sscreen, resource, alloc_size, alignment);

      if (!si_alloc_resource(sscreen, resource))
         goto error;
   } else {
      resource->buf = imported_buf;
      resource->gpu_address = sscreen->ws->buffer_get_virtual_address(resource->buf);
      resource->bo_size = imported_buf->size;
      resource->bo_alignment_log2 = imported_buf->alignment_log2;
      resource->domains = sscreen->ws->buffer_get_initial_domain(resource->buf);
      if (sscreen->ws->buffer_get_flags)
         resource->flags = sscreen->ws->buffer_get_flags(resource->buf);
   }

   if (sscreen->debug_flags & DBG(VM)) {
      fprintf(stderr,
              "VM start=0x%" PRIX64 "  end=0x%" PRIX64
              " | Texture %ix%ix%i, %i levels, %i samples, %s | Flags: ",
              tex->buffer.gpu_address, tex->buffer.gpu_address + tex->buffer.buf->size,
              base->width0, base->height0, util_num_layers(base, 0), base->last_level + 1,
              base->nr_samples ? base->nr_samples : 1, util_format_short_name(base->format));
      si_res_print_flags(tex->buffer.flags);
      fprintf(stderr, "\n");
   }

   if (sscreen->debug_flags & DBG(TEX)) {
      puts("Texture:");
      struct u_log_context log;
      u_log_context_init(&log);
      si_print_texture_info(sscreen, tex, &log);
      u_log_new_page_print(&log, stdout);
      fflush(stdout);
      u_log_context_destroy(&log);
   }

   /* Use 1.0 as the default clear value to get optimal ZRANGE_PRECISION if we don't
    * get a fast clear.
    */
   for (unsigned i = 0; i < ARRAY_SIZE(tex->depth_clear_value); i++)
      tex->depth_clear_value[i] = 1.0;

   /* On GFX8, HTILE uses different tiling depending on the TC_COMPATIBLE_HTILE
    * setting, so we have to enable it if we enabled it at allocation.
    *
    * GFX9 and later use the same tiling for both, so TC-compatible HTILE can be
    * enabled on demand.
    */
   tex->tc_compatible_htile = (sscreen->info.gfx_level == GFX8 &&
                               tex->surface.flags & RADEON_SURF_TC_COMPATIBLE_HTILE) ||
                              /* Mipmapping always starts TC-compatible. */
                              (sscreen->info.gfx_level >= GFX8 &&
                               tex->surface.flags & RADEON_SURF_TC_COMPATIBLE_HTILE &&
                               tex->buffer.b.b.last_level > 0);

   /* TC-compatible HTILE:
    * - GFX8 only supports Z32_FLOAT.
    * - GFX9 only supports Z32_FLOAT and Z16_UNORM. */
   if (tex->surface.flags & RADEON_SURF_TC_COMPATIBLE_HTILE) {
      if (sscreen->info.gfx_level >= GFX9 && base->format == PIPE_FORMAT_Z16_UNORM)
         tex->db_render_format = base->format;
      else {
         tex->db_render_format = PIPE_FORMAT_Z32_FLOAT;
         tex->upgraded_depth = base->format != PIPE_FORMAT_Z32_FLOAT &&
                               base->format != PIPE_FORMAT_Z32_FLOAT_S8X24_UINT;
      }
   } else {
      tex->db_render_format = base->format;
   }

   /* Applies to GCN. */
   tex->last_msaa_resolve_target_micro_mode = tex->surface.micro_tile_mode;

   if (tex->is_depth) {
      tex->htile_stencil_disabled = !tex->surface.has_stencil;

      if (sscreen->info.gfx_level >= GFX9) {
         tex->can_sample_z = true;
         tex->can_sample_s = true;

         /* Stencil texturing with HTILE doesn't work
          * with mipmapping on Navi10-14. */
         if (sscreen->info.gfx_level == GFX10 && base->last_level > 0)
            tex->htile_stencil_disabled = true;
      } else {
         tex->can_sample_z = !tex->surface.u.legacy.depth_adjusted;
         tex->can_sample_s = !tex->surface.u.legacy.stencil_adjusted;

         /* GFX8 must keep stencil enabled because it can't use Z-only TC-compatible
          * HTILE because of a hw bug. This has only a small effect on performance
          * because we lose a little bit of Z precision in order to make space for
          * stencil in HTILE.
          */
         if (sscreen->info.gfx_level == GFX8 &&
             tex->surface.flags & RADEON_SURF_TC_COMPATIBLE_HTILE)
            tex->htile_stencil_disabled = false;
      }

      tex->db_compatible = surface->flags & RADEON_SURF_ZBUFFER;
   } else {
      if (tex->surface.cmask_offset) {
         assert(sscreen->info.gfx_level < GFX11);
         tex->cb_color_info |= S_028C70_FAST_CLEAR(1);
         tex->cmask_buffer = &tex->buffer;
      }
   }

   /* Prepare metadata clears.  */
   struct si_clear_info clears[4];
   unsigned num_clears = 0;

   if (tex->cmask_buffer) {
      /* Initialize the cmask to 0xCC (= compressed state). */
      assert(num_clears < ARRAY_SIZE(clears));
      si_init_buffer_clear(&clears[num_clears++], &tex->cmask_buffer->b.b,
                           tex->surface.cmask_offset, tex->surface.cmask_size,
                           0xCCCCCCCC);
   }
   if (tex->is_depth && tex->surface.meta_offset) {
      uint32_t clear_value = 0;

      if (sscreen->info.gfx_level >= GFX9 || tex->tc_compatible_htile)
         clear_value = 0x0000030F;

      assert(num_clears < ARRAY_SIZE(clears));
      si_init_buffer_clear(&clears[num_clears++], &tex->buffer.b.b, tex->surface.meta_offset,
                           tex->surface.meta_size, clear_value);
   }

   /* Initialize DCC only if the texture is not being imported. */
   if (!(surface->flags & RADEON_SURF_IMPORTED) && !tex->is_depth && tex->surface.meta_offset) {
      /* Clear DCC to black for all tiles with DCC enabled.
       *
       * This fixes corruption in 3DMark Slingshot Extreme, which
       * uses uninitialized textures, causing corruption.
       */
      if (tex->surface.num_meta_levels == tex->buffer.b.b.last_level + 1 &&
          tex->buffer.b.b.nr_samples <= 2) {
         /* Simple case - all tiles have DCC enabled. */
         assert(num_clears < ARRAY_SIZE(clears));
         si_init_buffer_clear(&clears[num_clears++], &tex->buffer.b.b, tex->surface.meta_offset,
                              tex->surface.meta_size, DCC_CLEAR_0000);
      } else if (sscreen->info.gfx_level >= GFX9) {
         /* Clear to uncompressed. Clearing this to black is complicated. */
         assert(num_clears < ARRAY_SIZE(clears));
         si_init_buffer_clear(&clears[num_clears++], &tex->buffer.b.b, tex->surface.meta_offset,
                              tex->surface.meta_size, DCC_UNCOMPRESSED);
      } else {
         /* GFX8: Initialize mipmap levels and multisamples separately. */
         if (tex->buffer.b.b.nr_samples >= 2) {
            /* Clearing this to black is complicated. */
            assert(num_clears < ARRAY_SIZE(clears));
            si_init_buffer_clear(&clears[num_clears++], &tex->buffer.b.b, tex->surface.meta_offset,
                                 tex->surface.meta_size, DCC_UNCOMPRESSED);
         } else {
            /* Clear the enabled mipmap levels to black. */
            unsigned size = 0;

            for (unsigned i = 0; i < tex->surface.num_meta_levels; i++) {
               if (!tex->surface.u.legacy.color.dcc_level[i].dcc_fast_clear_size)
                  break;

               size = tex->surface.u.legacy.color.dcc_level[i].dcc_offset +
                      tex->surface.u.legacy.color.dcc_level[i].dcc_fast_clear_size;
            }

            /* Mipmap levels with DCC. */
            if (size) {
               assert(num_clears < ARRAY_SIZE(clears));
               si_init_buffer_clear(&clears[num_clears++], &tex->buffer.b.b, tex->surface.meta_offset, size,
                                    DCC_CLEAR_0000);
            }
            /* Mipmap levels without DCC. */
            if (size != tex->surface.meta_size) {
               assert(num_clears < ARRAY_SIZE(clears));
               si_init_buffer_clear(&clears[num_clears++], &tex->buffer.b.b, tex->surface.meta_offset + size,
                                    tex->surface.meta_size - size, DCC_UNCOMPRESSED);
            }
         }
      }
   }

   /* Initialize displayable DCC that requires the retile blit. */
   if (tex->surface.display_dcc_offset && !(surface->flags & RADEON_SURF_IMPORTED)) {
      /* Uninitialized DCC can hang the display hw.
       * Clear to white to indicate that. */
      assert(num_clears < ARRAY_SIZE(clears));
      si_init_buffer_clear(&clears[num_clears++], &tex->buffer.b.b, tex->surface.display_dcc_offset,
                           tex->surface.u.gfx9.color.display_dcc_size,
                           sscreen->info.gfx_level >= GFX11 ? GFX11_DCC_CLEAR_1111_UNORM
                                                             : GFX8_DCC_CLEAR_1111);
   }

   /* Execute the clears. */
   if (num_clears) {
      si_execute_clears(si_get_aux_context(&sscreen->aux_context.general), clears, num_clears, 0);
      si_put_aux_context_flush(&sscreen->aux_context.general);
   }

   /* Initialize the CMASK base register value. */
   tex->cmask_base_address_reg = (tex->buffer.gpu_address + tex->surface.cmask_offset) >> 8;

   return tex;

error:
   FREE_CL(tex);
   return NULL;
}

static enum radeon_surf_mode si_choose_tiling(struct si_screen *sscreen,
                                              const struct pipe_resource *templ,
                                              bool tc_compatible_htile)
{
   const struct util_format_description *desc = util_format_description(templ->format);
   bool force_tiling = templ->flags & SI_RESOURCE_FLAG_FORCE_MSAA_TILING;
   bool is_depth_stencil = util_format_is_depth_or_stencil(templ->format) &&
                           !(templ->flags & SI_RESOURCE_FLAG_FLUSHED_DEPTH);

   /* MSAA resources must be 2D tiled. */
   if (templ->nr_samples > 1)
      return RADEON_SURF_MODE_2D;

   /* Transfer resources should be linear. */
   if (templ->flags & SI_RESOURCE_FLAG_FORCE_LINEAR)
      return RADEON_SURF_MODE_LINEAR_ALIGNED;

   /* Avoid Z/S decompress blits by forcing TC-compatible HTILE on GFX8,
    * which requires 2D tiling.
    */
   if (sscreen->info.gfx_level == GFX8 && tc_compatible_htile)
      return RADEON_SURF_MODE_2D;

   /* Handle common candidates for the linear mode.
    * Compressed textures and DB surfaces must always be tiled.
    */
   if (!force_tiling && !is_depth_stencil && !util_format_is_compressed(templ->format)) {
      if (sscreen->debug_flags & DBG(NO_TILING) ||
          (templ->bind & PIPE_BIND_SCANOUT && sscreen->debug_flags & DBG(NO_DISPLAY_TILING)))
         return RADEON_SURF_MODE_LINEAR_ALIGNED;

      /* Tiling doesn't work with the 422 (SUBSAMPLED) formats. */
      if (desc->layout == UTIL_FORMAT_LAYOUT_SUBSAMPLED)
         return RADEON_SURF_MODE_LINEAR_ALIGNED;

      /* Cursors are linear on AMD GCN.
       * (XXX double-check, maybe also use RADEON_SURF_SCANOUT) */
      if (templ->bind & PIPE_BIND_CURSOR)
         return RADEON_SURF_MODE_LINEAR_ALIGNED;

      if (templ->bind & PIPE_BIND_LINEAR)
         return RADEON_SURF_MODE_LINEAR_ALIGNED;

      /* Textures with a very small height are recommended to be linear. */
      if (templ->target == PIPE_TEXTURE_1D || templ->target == PIPE_TEXTURE_1D_ARRAY ||
          /* Only very thin and long 2D textures should benefit from
           * linear_aligned. */
          templ->height0 <= 2)
         return RADEON_SURF_MODE_LINEAR_ALIGNED;

      /* Textures likely to be mapped often. */
      if (templ->usage == PIPE_USAGE_STAGING || templ->usage == PIPE_USAGE_STREAM)
         return RADEON_SURF_MODE_LINEAR_ALIGNED;
   }

   /* Make small textures 1D tiled. */
   if (templ->width0 <= 16 || templ->height0 <= 16 || (sscreen->debug_flags & DBG(NO_2D_TILING)))
      return RADEON_SURF_MODE_1D;

   /* The allocator will switch to 1D if needed. */
   return RADEON_SURF_MODE_2D;
}

static struct pipe_resource *
si_texture_create_with_modifier(struct pipe_screen *screen,
                                const struct pipe_resource *templ,
                                uint64_t modifier)
{
   struct si_screen *sscreen = (struct si_screen *)screen;
   bool is_zs = util_format_is_depth_or_stencil(templ->format);

   if (templ->nr_samples >= 2) {
      /* This is hackish (overwriting the const pipe_resource template),
       * but should be harmless and gallium frontends can also see
       * the overridden number of samples in the created pipe_resource.
       */
      if (is_zs && sscreen->eqaa_force_z_samples) {
         ((struct pipe_resource *)templ)->nr_samples =
            ((struct pipe_resource *)templ)->nr_storage_samples = sscreen->eqaa_force_z_samples;
      } else if (!is_zs && sscreen->eqaa_force_color_samples) {
         ((struct pipe_resource *)templ)->nr_samples = sscreen->eqaa_force_coverage_samples;
         ((struct pipe_resource *)templ)->nr_storage_samples = sscreen->eqaa_force_color_samples;
      }
   }

   bool is_flushed_depth = templ->flags & SI_RESOURCE_FLAG_FLUSHED_DEPTH ||
                           templ->flags & SI_RESOURCE_FLAG_FORCE_LINEAR;
   bool tc_compatible_htile =
      sscreen->info.gfx_level >= GFX8 &&
      /* There are issues with TC-compatible HTILE on Tonga (and
       * Iceland is the same design), and documented bug workarounds
       * don't help. For example, this fails:
       *   piglit/bin/tex-miplevel-selection 'texture()' 2DShadow -auto
       */
      sscreen->info.family != CHIP_TONGA && sscreen->info.family != CHIP_ICELAND &&
      (templ->flags & PIPE_RESOURCE_FLAG_TEXTURING_MORE_LIKELY) &&
      !(sscreen->debug_flags & DBG(NO_HYPERZ)) && !is_flushed_depth &&
      is_zs;
   enum radeon_surf_mode tile_mode = si_choose_tiling(sscreen, templ, tc_compatible_htile);

   /* This allocates textures with multiple planes like NV12 in 1 buffer. */
   enum
   {
      SI_TEXTURE_MAX_PLANES = 3
   };
   struct radeon_surf surface[SI_TEXTURE_MAX_PLANES] = {};
   struct pipe_resource plane_templ[SI_TEXTURE_MAX_PLANES];
   uint64_t plane_offset[SI_TEXTURE_MAX_PLANES] = {};
   uint64_t total_size = 0;
   unsigned max_alignment = 0;
   unsigned num_planes = util_format_get_num_planes(templ->format);
   assert(num_planes <= SI_TEXTURE_MAX_PLANES);

   /* Compute texture or plane layouts and offsets. */
   for (unsigned i = 0; i < num_planes; i++) {
      plane_templ[i] = *templ;
      plane_templ[i].format = util_format_get_plane_format(templ->format, i);
      plane_templ[i].width0 = util_format_get_plane_width(templ->format, i, templ->width0);
      plane_templ[i].height0 = util_format_get_plane_height(templ->format, i, templ->height0);

      /* Multi-plane allocations need PIPE_BIND_SHARED, because we can't
       * reallocate the storage to add PIPE_BIND_SHARED, because it's
       * shared by 3 pipe_resources.
       */
      if (num_planes > 1)
         plane_templ[i].bind |= PIPE_BIND_SHARED;
      /* Setting metadata on suballocated buffers is impossible. So use PIPE_BIND_CUSTOM to
       * request a non-suballocated buffer.
       */
      if (!is_zs && sscreen->debug_flags & DBG(EXTRA_METADATA))
         plane_templ[i].bind |= PIPE_BIND_CUSTOM;

      if (si_init_surface(sscreen, &surface[i], &plane_templ[i], tile_mode, modifier,
                          false, plane_templ[i].bind & PIPE_BIND_SCANOUT,
                          is_flushed_depth, tc_compatible_htile))
         return NULL;

      plane_templ[i].nr_sparse_levels = surface[i].first_mip_tail_level;

      plane_offset[i] = align64(total_size, 1 << surface[i].surf_alignment_log2);
      total_size = plane_offset[i] + surface[i].total_size;
      max_alignment = MAX2(max_alignment, 1 << surface[i].surf_alignment_log2);
   }

   struct si_texture *plane0 = NULL, *last_plane = NULL;

   for (unsigned i = 0; i < num_planes; i++) {
      struct si_texture *tex =
         si_texture_create_object(screen, &plane_templ[i], &surface[i], plane0, NULL,
                                  plane_offset[i], 0, total_size, max_alignment);
      if (!tex) {
         si_texture_reference(&plane0, NULL);
         return NULL;
      }

      tex->plane_index = i;
      tex->num_planes = num_planes;

      if (!plane0) {
         plane0 = last_plane = tex;
      } else {
         last_plane->buffer.b.b.next = &tex->buffer.b.b;
         last_plane = tex;
      }
      if (i == 0 && !is_zs && tex->surface.fmask_size == 0 &&
          sscreen->debug_flags & DBG(EXTRA_METADATA))
         si_set_tex_bo_metadata(sscreen, tex);
   }

   if (num_planes >= 2)
      plane0->multi_plane_format = templ->format;

   return (struct pipe_resource *)plane0;
}

struct pipe_resource *si_texture_create(struct pipe_screen *screen,
                                        const struct pipe_resource *templ)
{
   return si_texture_create_with_modifier(screen, templ, DRM_FORMAT_MOD_INVALID);
}

bool si_texture_commit(struct si_context *ctx, struct si_resource *res, unsigned level,
                       struct pipe_box *box, bool commit)
{
   struct si_texture *tex = (struct si_texture *)res;
   struct radeon_surf *surface = &tex->surface;
   enum pipe_format format = res->b.b.format;
   unsigned blks = util_format_get_blocksize(format);
   unsigned samples = MAX2(1, res->b.b.nr_samples);

   assert(ctx->gfx_level >= GFX9);

   unsigned row_pitch = surface->u.gfx9.prt_level_pitch[level] *
      surface->prt_tile_height * surface->prt_tile_depth * blks * samples;
   uint64_t depth_pitch = surface->u.gfx9.surf_slice_size * surface->prt_tile_depth;

   unsigned x = box->x / surface->prt_tile_width;
   unsigned y = box->y / surface->prt_tile_height;
   unsigned z = box->z / surface->prt_tile_depth;

   unsigned w = DIV_ROUND_UP(box->width, surface->prt_tile_width);
   unsigned h = DIV_ROUND_UP(box->height, surface->prt_tile_height);
   unsigned d = DIV_ROUND_UP(box->depth, surface->prt_tile_depth);

   /* Align to tile block base, for levels in mip tail whose offset is inside
    * a tile block.
    */
   uint64_t level_base = ROUND_DOWN_TO(surface->u.gfx9.prt_level_offset[level],
                                       RADEON_SPARSE_PAGE_SIZE);
   uint64_t commit_base = level_base +
      x * RADEON_SPARSE_PAGE_SIZE + y * (uint64_t)row_pitch + z * depth_pitch;

   uint64_t size = (uint64_t)w * RADEON_SPARSE_PAGE_SIZE;
   for (int i = 0; i < d; i++) {
      uint64_t base = commit_base + i * depth_pitch;
      for (int j = 0; j < h; j++) {
         uint64_t offset = base + j * row_pitch;
         if (!ctx->ws->buffer_commit(ctx->ws, res->buf, offset, size, commit))
            return false;
      }
   }

   return true;
}

static void si_query_dmabuf_modifiers(struct pipe_screen *screen,
                                      enum pipe_format format,
                                      int max,
                                      uint64_t *modifiers,
                                      unsigned int *external_only,
                                      int *count)
{
   struct si_screen *sscreen = (struct si_screen *)screen;

   unsigned ac_mod_count = max;
   ac_get_supported_modifiers(&sscreen->info, &(struct ac_modifier_options) {
         .dcc = !(sscreen->debug_flags & DBG(NO_DCC)),
         /* Do not support DCC with retiling yet. This needs explicit
          * resource flushes, but the app has no way to promise doing
          * flushes with modifiers. */
         .dcc_retile = !(sscreen->debug_flags & DBG(NO_DCC)),
      }, format, &ac_mod_count,  max ? modifiers : NULL);
   if (max && external_only) {
      for (unsigned i = 0; i < ac_mod_count; ++i)
         external_only[i] = util_format_is_yuv(format);
   }
   *count = ac_mod_count;
}

static bool
si_is_dmabuf_modifier_supported(struct pipe_screen *screen,
                               uint64_t modifier,
                               enum pipe_format format,
                               bool *external_only)
{
   int allowed_mod_count;
   si_query_dmabuf_modifiers(screen, format, 0, NULL, NULL, &allowed_mod_count);

   uint64_t *allowed_modifiers = (uint64_t *)calloc(allowed_mod_count, sizeof(uint64_t));
   if (!allowed_modifiers)
      return false;

   unsigned *external_array = NULL;
   if (external_only) {
      external_array = (unsigned *)calloc(allowed_mod_count, sizeof(unsigned));
      if (!external_array) {
         free(allowed_modifiers);
         return false;
      }
   }

   si_query_dmabuf_modifiers(screen, format, allowed_mod_count, allowed_modifiers,
                            external_array, &allowed_mod_count);

   bool supported = false;
   for (int i = 0; i < allowed_mod_count && !supported; ++i) {
      if (allowed_modifiers[i] != modifier)
         continue;

      supported = true;
      if (external_only)
         *external_only = external_array[i];
   }

   free(allowed_modifiers);
   free(external_array);
   return supported;
}

static unsigned
si_get_dmabuf_modifier_planes(struct pipe_screen *pscreen, uint64_t modifier,
                             enum pipe_format format)
{
   unsigned planes = util_format_get_num_planes(format);

   if (IS_AMD_FMT_MOD(modifier) && planes == 1) {
      if (AMD_FMT_MOD_GET(DCC_RETILE, modifier))
         return 3;
      else if (AMD_FMT_MOD_GET(DCC, modifier))
         return 2;
      else
         return 1;
   }

   return planes;
}

static bool
si_modifier_supports_resource(struct pipe_screen *screen,
                              uint64_t modifier,
                              const struct pipe_resource *templ)
{
   struct si_screen *sscreen = (struct si_screen *)screen;
   uint32_t max_width, max_height;

   ac_modifier_max_extent(&sscreen->info, modifier, &max_width, &max_height);
   return templ->width0 <= max_width && templ->height0 <= max_height;
}

static struct pipe_resource *
si_texture_create_with_modifiers(struct pipe_screen *screen,
                                 const struct pipe_resource *templ,
                                 const uint64_t *modifiers,
                                 int modifier_count)
{
   /* Buffers with modifiers make zero sense. */
   assert(templ->target != PIPE_BUFFER);

   /* Select modifier. */
   int allowed_mod_count;
   si_query_dmabuf_modifiers(screen, templ->format, 0, NULL, NULL, &allowed_mod_count);

   uint64_t *allowed_modifiers = (uint64_t *)calloc(allowed_mod_count, sizeof(uint64_t));
   if (!allowed_modifiers) {
      return NULL;
   }

   /* This does not take external_only into account. We assume it is the same for all modifiers. */
   si_query_dmabuf_modifiers(screen, templ->format, allowed_mod_count, allowed_modifiers, NULL, &allowed_mod_count);

   uint64_t modifier = DRM_FORMAT_MOD_INVALID;

   /* Try to find the first allowed modifier that is in the application provided
    * list. We assume that the allowed modifiers are ordered in descending
    * preference in the list provided by si_query_dmabuf_modifiers. */
   for (int i = 0; i < allowed_mod_count; ++i) {
      bool found = false;
      for (int j = 0; j < modifier_count && !found; ++j)
         if (modifiers[j] == allowed_modifiers[i] && si_modifier_supports_resource(screen, modifiers[j], templ))
            found = true;

      if (found) {
         modifier = allowed_modifiers[i];
         break;
      }
   }

   free(allowed_modifiers);

   if (modifier == DRM_FORMAT_MOD_INVALID) {
      return NULL;
   }
   return si_texture_create_with_modifier(screen, templ, modifier);
}

static bool si_texture_is_aux_plane(const struct pipe_resource *resource)
{
   return resource->flags & SI_RESOURCE_AUX_PLANE;
}

static struct pipe_resource *si_texture_from_winsys_buffer(struct si_screen *sscreen,
                                                           const struct pipe_resource *templ,
                                                           struct pb_buffer_lean *buf, unsigned stride,
                                                           uint64_t offset, uint64_t modifier,
                                                           unsigned usage, bool dedicated)
{
   struct radeon_surf surface = {};
   struct radeon_bo_metadata metadata = {};
   struct si_texture *tex;
   int r;

   /* Ignore metadata for non-zero planes. */
   if (offset != 0)
      dedicated = false;

   if (dedicated) {
      sscreen->ws->buffer_get_metadata(sscreen->ws, buf, &metadata, &surface);
   } else {
      /**
       * The bo metadata is unset for un-dedicated images. So we fall
       * back to linear. See answer to question 5 of the
       * VK_KHX_external_memory spec for some details.
       *
       * It is possible that this case isn't going to work if the
       * surface pitch isn't correctly aligned by default.
       *
       * In order to support it correctly we require multi-image
       * metadata to be synchronized between radv and radeonsi. The
       * semantics of associating multiple image metadata to a memory
       * object on the vulkan export side are not concretely defined
       * either.
       *
       * All the use cases we are aware of at the moment for memory
       * objects use dedicated allocations. So lets keep the initial
       * implementation simple.
       *
       * A possible alternative is to attempt to reconstruct the
       * tiling information when the TexParameter TEXTURE_TILING_EXT
       * is set.
       */
      metadata.mode = RADEON_SURF_MODE_LINEAR_ALIGNED;
   }

   r = si_init_surface(sscreen, &surface, templ, metadata.mode, modifier, true,
                       surface.flags & RADEON_SURF_SCANOUT, false, false);
   if (r)
      return NULL;

   /* This is a hack to skip alignment checking for 3D textures */
   if (templ->target == PIPE_TEXTURE_3D)
      stride = 0;

   tex = si_texture_create_object(&sscreen->b, templ, &surface, NULL, buf,
                                  offset, stride, 0, 0);
   if (!tex)
      return NULL;

   tex->buffer.b.is_shared = true;
   tex->buffer.external_usage = usage;
   tex->num_planes = 1;
   if (tex->buffer.flags & RADEON_FLAG_ENCRYPTED)
      tex->buffer.b.b.bind |= PIPE_BIND_PROTECTED;

   /* Account for multiple planes with lowered yuv import. */
   struct pipe_resource *next_plane = tex->buffer.b.b.next;
   while (next_plane && !si_texture_is_aux_plane(next_plane)) {
      struct si_texture *next_tex = (struct si_texture *)next_plane;
      ++next_tex->num_planes;
      ++tex->num_planes;
      next_plane = next_plane->next;
   }

   unsigned nplanes = ac_surface_get_nplanes(&tex->surface);
   unsigned plane = 1;
   while (next_plane) {
      struct si_auxiliary_texture *ptex = (struct si_auxiliary_texture *)next_plane;
      if (plane >= nplanes || ptex->buffer != tex->buffer.buf ||
          ptex->offset != ac_surface_get_plane_offset(sscreen->info.gfx_level,
                                                      &tex->surface, plane, 0) ||
          ptex->stride != ac_surface_get_plane_stride(sscreen->info.gfx_level,
                                                      &tex->surface, plane, 0)) {
         si_texture_reference(&tex, NULL);
         return NULL;
      }
      ++plane;
      next_plane = next_plane->next;
   }

   if (plane != nplanes && tex->num_planes == 1) {
      si_texture_reference(&tex, NULL);
      return NULL;
   }

   if (!ac_surface_apply_umd_metadata(&sscreen->info, &tex->surface,
                                      tex->buffer.b.b.nr_storage_samples,
                                      tex->buffer.b.b.last_level + 1,
                                      metadata.size_metadata,
                                      metadata.metadata)) {
      si_texture_reference(&tex, NULL);
      return NULL;
   }

   if (ac_surface_get_plane_offset(sscreen->info.gfx_level, &tex->surface, 0, 0) +
        tex->surface.total_size > buf->size) {
      si_texture_reference(&tex, NULL);
      return NULL;
   }

   /* Displayable DCC requires an explicit flush. */
   if (dedicated && offset == 0 && !(usage & PIPE_HANDLE_USAGE_EXPLICIT_FLUSH) &&
       si_displayable_dcc_needs_explicit_flush(tex)) {
      /* TODO: do we need to decompress DCC? */
      if (si_texture_discard_dcc(sscreen, tex)) {
         /* Update BO metadata after disabling DCC. */
         si_set_tex_bo_metadata(sscreen, tex);
      }
   }

   assert(tex->surface.tile_swizzle == 0);
   return &tex->buffer.b.b;
}

static struct pipe_resource *si_texture_from_handle(struct pipe_screen *screen,
                                                    const struct pipe_resource *templ,
                                                    struct winsys_handle *whandle, unsigned usage)
{
   struct si_screen *sscreen = (struct si_screen *)screen;
   struct pb_buffer_lean *buf = NULL;

   buf = sscreen->ws->buffer_from_handle(sscreen->ws, whandle,
                                         sscreen->info.max_alignment,
                                         templ->bind & PIPE_BIND_PRIME_BLIT_DST);
   if (!buf)
      return NULL;

   if (templ->target == PIPE_BUFFER)
      return si_buffer_from_winsys_buffer(screen, templ, buf, 0);

   if (whandle->plane >= util_format_get_num_planes(whandle->format)) {
      struct si_auxiliary_texture *tex = CALLOC_STRUCT_CL(si_auxiliary_texture);
      if (!tex)
         return NULL;
      tex->b.b = *templ;
      tex->b.b.flags |= SI_RESOURCE_AUX_PLANE;
      tex->stride = whandle->stride;
      tex->offset = whandle->offset;
      tex->buffer = buf;
      pipe_reference_init(&tex->b.b.reference, 1);
      tex->b.b.screen = screen;

      return &tex->b.b;
   }

   return si_texture_from_winsys_buffer(sscreen, templ, buf, whandle->stride, whandle->offset,
                                        whandle->modifier, usage, true);
}

bool si_init_flushed_depth_texture(struct pipe_context *ctx, struct pipe_resource *texture)
{
   struct si_texture *tex = (struct si_texture *)texture;
   struct pipe_resource resource;
   enum pipe_format pipe_format = texture->format;

   assert(!tex->flushed_depth_texture);

   if (!tex->can_sample_z && tex->can_sample_s) {
      switch (pipe_format) {
      case PIPE_FORMAT_Z32_FLOAT_S8X24_UINT:
         /* Save memory by not allocating the S plane. */
         pipe_format = PIPE_FORMAT_Z32_FLOAT;
         break;
      case PIPE_FORMAT_Z24_UNORM_S8_UINT:
      case PIPE_FORMAT_S8_UINT_Z24_UNORM:
         /* Save memory bandwidth by not copying the
          * stencil part during flush.
          *
          * This potentially increases memory bandwidth
          * if an application uses both Z and S texturing
          * simultaneously (a flushed Z24S8 texture
          * would be stored compactly), but how often
          * does that really happen?
          */
         pipe_format = PIPE_FORMAT_Z24X8_UNORM;
         break;
      default:;
      }
   } else if (!tex->can_sample_s && tex->can_sample_z) {
      assert(util_format_has_stencil(util_format_description(pipe_format)));

      /* DB->CB copies to an 8bpp surface don't work. */
      pipe_format = PIPE_FORMAT_X24S8_UINT;
   }

   memset(&resource, 0, sizeof(resource));
   resource.target = texture->target;
   resource.format = pipe_format;
   resource.width0 = texture->width0;
   resource.height0 = texture->height0;
   resource.depth0 = texture->depth0;
   resource.array_size = texture->array_size;
   resource.last_level = texture->last_level;
   resource.nr_samples = texture->nr_samples;
   resource.nr_storage_samples = texture->nr_storage_samples;
   resource.usage = PIPE_USAGE_DEFAULT;
   resource.bind = texture->bind & ~PIPE_BIND_DEPTH_STENCIL;
   resource.flags = texture->flags | SI_RESOURCE_FLAG_FLUSHED_DEPTH;

   tex->flushed_depth_texture =
      (struct si_texture *)ctx->screen->resource_create(ctx->screen, &resource);
   if (!tex->flushed_depth_texture) {
      PRINT_ERR("failed to create temporary texture to hold flushed depth\n");
      return false;
   }
   return true;
}

/**
 * Initialize the pipe_resource descriptor to be of the same size as the box,
 * which is supposed to hold a subregion of the texture "orig" at the given
 * mipmap level.
 */
static void si_init_temp_resource_from_box(struct pipe_resource *res, struct pipe_resource *orig,
                                           const struct pipe_box *box, unsigned level,
                                           unsigned usage, unsigned flags)
{
   struct si_texture *tex = (struct si_texture *)orig;
   enum pipe_format orig_format = tex->multi_plane_format != PIPE_FORMAT_NONE ?
      tex->multi_plane_format : orig->format;

   memset(res, 0, sizeof(*res));
   res->format = orig_format;
   res->width0 = box->width;
   res->height0 = box->height;
   res->depth0 = 1;
   res->array_size = 1;
   res->usage = usage;
   res->flags = flags;

   if (flags & SI_RESOURCE_FLAG_FORCE_LINEAR && util_format_is_compressed(orig_format)) {
      /* Transfer resources are allocated with linear tiling, which is
       * not supported for compressed formats.
       */
      unsigned blocksize = util_format_get_blocksize(orig_format);

      if (blocksize == 8) {
         res->format = PIPE_FORMAT_R16G16B16A16_UINT;
      } else {
         assert(blocksize == 16);
         res->format = PIPE_FORMAT_R32G32B32A32_UINT;
      }

      res->width0 = util_format_get_nblocksx(orig_format, box->width);
      res->height0 = util_format_get_nblocksy(orig_format, box->height);
   }

   /* We must set the correct texture target and dimensions for a 3D box. */
   if (box->depth > 1 && util_max_layer(orig, level) > 0) {
      res->target = PIPE_TEXTURE_2D_ARRAY;
      res->array_size = box->depth;
   } else {
      res->target = PIPE_TEXTURE_2D;
   }
}

static bool si_can_invalidate_texture(struct si_screen *sscreen, struct si_texture *tex,
                                      unsigned transfer_usage, const struct pipe_box *box)
{
   return !tex->buffer.b.is_shared && !(tex->surface.flags & RADEON_SURF_IMPORTED) &&
          !(transfer_usage & PIPE_MAP_READ) && tex->buffer.b.b.last_level == 0 &&
          util_texrange_covers_whole_level(&tex->buffer.b.b, 0, box->x, box->y, box->z, box->width,
                                           box->height, box->depth);
}

static void si_texture_invalidate_storage(struct si_context *sctx, struct si_texture *tex)
{
   struct si_screen *sscreen = sctx->screen;

   /* There is no point in discarding depth and tiled buffers. */
   assert(!tex->is_depth);
   assert(tex->surface.is_linear);

   /* Reallocate the buffer in the same pipe_resource. */
   si_alloc_resource(sscreen, &tex->buffer);

   /* Initialize the CMASK base address (needed even without CMASK). */
   tex->cmask_base_address_reg = (tex->buffer.gpu_address + tex->surface.cmask_offset) >> 8;

   p_atomic_inc(&sscreen->dirty_tex_counter);

   sctx->num_alloc_tex_transfer_bytes += tex->surface.total_size;
}

static void *si_texture_transfer_map(struct pipe_context *ctx, struct pipe_resource *texture,
                                     unsigned level, unsigned usage, const struct pipe_box *box,
                                     struct pipe_transfer **ptransfer)
{
   struct si_context *sctx = (struct si_context *)ctx;
   struct si_texture *tex = (struct si_texture *)texture;
   struct si_transfer *trans;
   struct si_resource *buf;
   uint64_t offset = 0;
   char *map;
   bool use_staging_texture = tex->buffer.flags & RADEON_FLAG_ENCRYPTED;
   unsigned real_level = texture->nr_samples > 1 ? 0 : level;

   assert(texture->target != PIPE_BUFFER);
   assert(!(texture->flags & SI_RESOURCE_FLAG_FORCE_LINEAR));
   assert(box->width && box->height && box->depth);

   if (tex->buffer.b.b.flags & SI_RESOURCE_AUX_PLANE)
      return NULL;

   if ((tex->buffer.flags & RADEON_FLAG_ENCRYPTED) && usage & PIPE_MAP_READ)
      return NULL;

   if (tex->is_depth || tex->buffer.flags & RADEON_FLAG_SPARSE) {
      /* Depth and sparse textures use staging unconditionally. */
      use_staging_texture = true;
   } else {
      /* Degrade the tile mode if we get too many transfers on APUs.
       * On dGPUs, the staging texture is always faster.
       * Only count uploads that are at least 4x4 pixels large.
       */
      if (!sctx->screen->info.has_dedicated_vram && real_level == 0 && box->width >= 4 &&
          box->height >= 4 && p_atomic_inc_return(&tex->num_level0_transfers) == 10) {
         bool can_invalidate = si_can_invalidate_texture(sctx->screen, tex, usage, box);

         si_reallocate_texture_inplace(sctx, tex, PIPE_BIND_LINEAR, can_invalidate);
      }

      /* Tiled textures need to be converted into a linear texture for CPU
       * access. The staging texture is always linear and is placed in GART.
       *
       * dGPU use a staging texture for VRAM, so that we don't map it and
       * don't relocate it to GTT.
       *
       * Reading from VRAM or GTT WC is slow, always use the staging
       * texture in this case.
       *
       * Use the staging texture for uploads if the underlying BO
       * is busy.
       */
      if (!tex->surface.is_linear || (tex->buffer.flags & RADEON_FLAG_ENCRYPTED) ||
          (tex->buffer.domains & RADEON_DOMAIN_VRAM && sctx->screen->info.has_dedicated_vram))
         use_staging_texture = true;
      else if (usage & PIPE_MAP_READ)
         use_staging_texture =
            tex->buffer.domains & RADEON_DOMAIN_VRAM || tex->buffer.flags & RADEON_FLAG_GTT_WC;
      /* Write & linear only: */
      else if (si_cs_is_buffer_referenced(sctx, tex->buffer.buf, RADEON_USAGE_READWRITE) ||
               !sctx->ws->buffer_wait(sctx->ws, tex->buffer.buf, 0, RADEON_USAGE_READWRITE)) {
         /* It's busy. */
         if (si_can_invalidate_texture(sctx->screen, tex, usage, box))
            si_texture_invalidate_storage(sctx, tex);
         else
            use_staging_texture = true;
      }
   }

   trans = CALLOC_STRUCT(si_transfer);
   if (!trans)
      return NULL;
   pipe_resource_reference(&trans->b.b.resource, texture);
   trans->b.b.level = level;
   trans->b.b.usage = usage;
   trans->b.b.box = *box;

   if (use_staging_texture) {
      struct pipe_resource resource;
      struct si_texture *staging;
      unsigned bo_usage = usage & PIPE_MAP_READ ? PIPE_USAGE_STAGING : PIPE_USAGE_STREAM;
      unsigned bo_flags = SI_RESOURCE_FLAG_FORCE_LINEAR | SI_RESOURCE_FLAG_DRIVER_INTERNAL;

      si_init_temp_resource_from_box(&resource, texture, box, real_level, bo_usage,
                                     bo_flags);

      /* Since depth-stencil textures don't support linear tiling,
       * blit from ZS to color and vice versa. u_blitter will do
       * the packing for these formats.
       */
      if (tex->is_depth)
         resource.format = util_blitter_get_color_format_for_zs(resource.format);

      /* Create the temporary texture. */
      staging = (struct si_texture *)ctx->screen->resource_create(ctx->screen, &resource);
      if (!staging) {
         PRINT_ERR("failed to create temporary texture to hold untiled copy\n");
         goto fail_trans;
      }
      trans->staging = &staging->buffer;

      /* Just get the strides. */
      si_texture_get_offset(sctx->screen, staging, 0, NULL, &trans->b.b.stride,
                            &trans->b.b.layer_stride);

      if (usage & PIPE_MAP_READ)
         si_copy_to_staging_texture(ctx, trans);
      else
         usage |= PIPE_MAP_UNSYNCHRONIZED;

      buf = trans->staging;
   } else {
      /* the resource is mapped directly */
      offset = si_texture_get_offset(sctx->screen, tex, real_level, box, &trans->b.b.stride,
                                     &trans->b.b.layer_stride);
      buf = &tex->buffer;
   }

   /* Always unmap texture CPU mappings on 32-bit architectures, so that
    * we don't run out of the CPU address space.
    */
   if (sizeof(void *) == 4)
      usage |= RADEON_MAP_TEMPORARY;

   if (!(map = si_buffer_map(sctx, buf, usage)))
      goto fail_trans;

   *ptransfer = &trans->b.b;
   return map + offset;

fail_trans:
   si_resource_reference(&trans->staging, NULL);
   pipe_resource_reference(&trans->b.b.resource, NULL);
   FREE(trans);
   return NULL;
}

static void si_texture_transfer_unmap(struct pipe_context *ctx, struct pipe_transfer *transfer)
{
   struct si_context *sctx = (struct si_context *)ctx;
   struct si_transfer *stransfer = (struct si_transfer *)transfer;
   struct pipe_resource *texture = transfer->resource;
   struct si_texture *tex = (struct si_texture *)texture;

   /* Always unmap texture CPU mappings on 32-bit architectures, so that
    * we don't run out of the CPU address space.
    */
   if (sizeof(void *) == 4) {
      struct si_resource *buf = stransfer->staging ? stransfer->staging : &tex->buffer;

      sctx->ws->buffer_unmap(sctx->ws, buf->buf);
   }

   if ((transfer->usage & PIPE_MAP_WRITE) && stransfer->staging)
      si_copy_from_staging_texture(ctx, stransfer);

   if (stransfer->staging) {
      sctx->num_alloc_tex_transfer_bytes += stransfer->staging->buf->size;
      si_resource_reference(&stransfer->staging, NULL);
   }

   /* Heuristic for {upload, draw, upload, draw, ..}:
    *
    * Flush the gfx IB if we've allocated too much texture storage.
    *
    * The idea is that we don't want to build IBs that use too much
    * memory and put pressure on the kernel memory manager and we also
    * want to make temporary and invalidated buffers go idle ASAP to
    * decrease the total memory usage or make them reusable. The memory
    * usage will be slightly higher than given here because of the buffer
    * cache in the winsys.
    *
    * The result is that the kernel memory manager is never a bottleneck.
    */
   if (sctx->num_alloc_tex_transfer_bytes > (uint64_t)sctx->screen->info.gart_size_kb * 1024 / 4) {
      si_flush_gfx_cs(sctx, RADEON_FLUSH_ASYNC_START_NEXT_GFX_IB_NOW, NULL);
      sctx->num_alloc_tex_transfer_bytes = 0;
   }

   pipe_resource_reference(&transfer->resource, NULL);
   FREE(transfer);
}

/* Return if it's allowed to reinterpret one format as another with DCC enabled.
 */
bool vi_dcc_formats_compatible(struct si_screen *sscreen, enum pipe_format format1,
                               enum pipe_format format2)
{
   const struct util_format_description *desc1, *desc2;

   /* All formats are compatible on GFX11. */
   if (sscreen->info.gfx_level >= GFX11)
      return true;

   /* No format change - exit early. */
   if (format1 == format2)
      return true;

   format1 = si_simplify_cb_format(format1);
   format2 = si_simplify_cb_format(format2);

   /* Check again after format adjustments. */
   if (format1 == format2)
      return true;

   desc1 = util_format_description(format1);
   desc2 = util_format_description(format2);

   if (desc1->layout != UTIL_FORMAT_LAYOUT_PLAIN || desc2->layout != UTIL_FORMAT_LAYOUT_PLAIN)
      return false;

   /* Float and non-float are totally incompatible. */
   if ((desc1->channel[0].type == UTIL_FORMAT_TYPE_FLOAT) !=
       (desc2->channel[0].type == UTIL_FORMAT_TYPE_FLOAT))
      return false;

   /* Channel sizes must match across DCC formats.
    * Comparing just the first 2 channels should be enough.
    */
   if (desc1->channel[0].size != desc2->channel[0].size ||
       (desc1->nr_channels >= 2 && desc1->channel[1].size != desc2->channel[1].size))
      return false;

   /* Everything below is not needed if the driver never uses the DCC
    * clear code with the value of 1.
    */

   /* If the clear values are all 1 or all 0, this constraint can be
    * ignored. */
   if (vi_alpha_is_on_msb(sscreen, format1) != vi_alpha_is_on_msb(sscreen, format2))
      return false;

   /* Channel types must match if the clear value of 1 is used.
    * The type categories are only float, signed, unsigned.
    * NORM and INT are always compatible.
    */
   if (desc1->channel[0].type != desc2->channel[0].type ||
       (desc1->nr_channels >= 2 && desc1->channel[1].type != desc2->channel[1].type))
      return false;

   return true;
}

bool vi_dcc_formats_are_incompatible(struct pipe_resource *tex, unsigned level,
                                     enum pipe_format view_format)
{
   struct si_texture *stex = (struct si_texture *)tex;

   return vi_dcc_enabled(stex, level) &&
          !vi_dcc_formats_compatible((struct si_screen *)tex->screen, tex->format, view_format);
}

/* This can't be merged with the above function, because
 * vi_dcc_formats_compatible should be called only when DCC is enabled. */
void vi_disable_dcc_if_incompatible_format(struct si_context *sctx, struct pipe_resource *tex,
                                           unsigned level, enum pipe_format view_format)
{
   struct si_texture *stex = (struct si_texture *)tex;

   if (vi_dcc_formats_are_incompatible(tex, level, view_format))
      if (!si_texture_disable_dcc(sctx, stex))
         si_decompress_dcc(sctx, stex);
}

static struct pipe_surface *si_create_surface(struct pipe_context *pipe, struct pipe_resource *tex,
                                              const struct pipe_surface *templ)
{
   unsigned level = templ->u.tex.level;
   unsigned width = u_minify(tex->width0, level);
   unsigned height = u_minify(tex->height0, level);
   unsigned width0 = tex->width0;
   unsigned height0 = tex->height0;

   if (tex->target != PIPE_BUFFER && templ->format != tex->format) {
      const struct util_format_description *tex_desc = util_format_description(tex->format);
      const struct util_format_description *templ_desc = util_format_description(templ->format);

      assert(tex_desc->block.bits == templ_desc->block.bits);

      /* Adjust size of surface if and only if the block width or
       * height is changed. */
      if (tex_desc->block.width != templ_desc->block.width ||
          tex_desc->block.height != templ_desc->block.height) {
         unsigned nblks_x = util_format_get_nblocksx(tex->format, width);
         unsigned nblks_y = util_format_get_nblocksy(tex->format, height);

         width = nblks_x * templ_desc->block.width;
         height = nblks_y * templ_desc->block.height;

         width0 = util_format_get_nblocksx(tex->format, width0);
         height0 = util_format_get_nblocksy(tex->format, height0);
      }
   }

   struct si_surface *surface = CALLOC_STRUCT(si_surface);

   if (!surface)
      return NULL;

   assert(templ->u.tex.first_layer <= util_max_layer(tex, templ->u.tex.level));
   assert(templ->u.tex.last_layer <= util_max_layer(tex, templ->u.tex.level));

   pipe_reference_init(&surface->base.reference, 1);
   pipe_resource_reference(&surface->base.texture, tex);
   surface->base.context = pipe;
   surface->base.format = templ->format;
   surface->base.width = width;
   surface->base.height = height;
   surface->base.u = templ->u;

   surface->width0 = width0;
   surface->height0 = height0;

   surface->dcc_incompatible =
      tex->target != PIPE_BUFFER &&
      vi_dcc_formats_are_incompatible(tex, templ->u.tex.level, templ->format);
   return &surface->base;
}

static void si_surface_destroy(struct pipe_context *pipe, struct pipe_surface *surface)
{
   pipe_resource_reference(&surface->texture, NULL);
   FREE(surface);
}

unsigned si_translate_colorswap(enum amd_gfx_level gfx_level, enum pipe_format format,
                                bool do_endian_swap)
{
   const struct util_format_description *desc = util_format_description(format);

#define HAS_SWIZZLE(chan, swz) (desc->swizzle[chan] == PIPE_SWIZZLE_##swz)

   if (format == PIPE_FORMAT_R11G11B10_FLOAT) /* isn't plain */
      return V_028C70_SWAP_STD;

   if (gfx_level >= GFX10_3 &&
       format == PIPE_FORMAT_R9G9B9E5_FLOAT) /* isn't plain */
      return V_028C70_SWAP_STD;

   if (desc->layout != UTIL_FORMAT_LAYOUT_PLAIN)
      return ~0U;

   switch (desc->nr_channels) {
   case 1:
      if (HAS_SWIZZLE(0, X))
         return V_028C70_SWAP_STD; /* X___ */
      else if (HAS_SWIZZLE(3, X))
         return V_028C70_SWAP_ALT_REV; /* ___X */
      break;
   case 2:
      if ((HAS_SWIZZLE(0, X) && HAS_SWIZZLE(1, Y)) || (HAS_SWIZZLE(0, X) && HAS_SWIZZLE(1, NONE)) ||
          (HAS_SWIZZLE(0, NONE) && HAS_SWIZZLE(1, Y)))
         return V_028C70_SWAP_STD; /* XY__ */
      else if ((HAS_SWIZZLE(0, Y) && HAS_SWIZZLE(1, X)) ||
               (HAS_SWIZZLE(0, Y) && HAS_SWIZZLE(1, NONE)) ||
               (HAS_SWIZZLE(0, NONE) && HAS_SWIZZLE(1, X)))
         /* YX__ */
         return (do_endian_swap ? V_028C70_SWAP_STD : V_028C70_SWAP_STD_REV);
      else if (HAS_SWIZZLE(0, X) && HAS_SWIZZLE(3, Y))
         return V_028C70_SWAP_ALT; /* X__Y */
      else if (HAS_SWIZZLE(0, Y) && HAS_SWIZZLE(3, X))
         return V_028C70_SWAP_ALT_REV; /* Y__X */
      break;
   case 3:
      if (HAS_SWIZZLE(0, X))
         return (do_endian_swap ? V_028C70_SWAP_STD_REV : V_028C70_SWAP_STD);
      else if (HAS_SWIZZLE(0, Z))
         return V_028C70_SWAP_STD_REV; /* ZYX */
      break;
   case 4:
      /* check the middle channels, the 1st and 4th channel can be NONE */
      if (HAS_SWIZZLE(1, Y) && HAS_SWIZZLE(2, Z)) {
         return V_028C70_SWAP_STD; /* XYZW */
      } else if (HAS_SWIZZLE(1, Z) && HAS_SWIZZLE(2, Y)) {
         return V_028C70_SWAP_STD_REV; /* WZYX */
      } else if (HAS_SWIZZLE(1, Y) && HAS_SWIZZLE(2, X)) {
         return V_028C70_SWAP_ALT; /* ZYXW */
      } else if (HAS_SWIZZLE(1, Z) && HAS_SWIZZLE(2, W)) {
         /* YZWX */
         if (desc->is_array)
            return V_028C70_SWAP_ALT_REV;
         else
            return (do_endian_swap ? V_028C70_SWAP_ALT : V_028C70_SWAP_ALT_REV);
      }
      break;
   }
   return ~0U;
}

static struct pipe_memory_object *
si_memobj_from_handle(struct pipe_screen *screen, struct winsys_handle *whandle, bool dedicated)
{
   struct si_screen *sscreen = (struct si_screen *)screen;
   struct si_memory_object *memobj = CALLOC_STRUCT(si_memory_object);
   struct pb_buffer_lean *buf = NULL;

   if (!memobj)
      return NULL;

   buf = sscreen->ws->buffer_from_handle(sscreen->ws, whandle, sscreen->info.max_alignment, false);
   if (!buf) {
      free(memobj);
      return NULL;
   }

   memobj->b.dedicated = dedicated;
   memobj->buf = buf;
   memobj->stride = whandle->stride;

   return (struct pipe_memory_object *)memobj;
}

static void si_memobj_destroy(struct pipe_screen *screen, struct pipe_memory_object *_memobj)
{
   struct si_memory_object *memobj = (struct si_memory_object *)_memobj;

   radeon_bo_reference(((struct si_screen*)screen)->ws, &memobj->buf, NULL);
   free(memobj);
}

static struct pipe_resource *si_resource_from_memobj(struct pipe_screen *screen,
                                                    const struct pipe_resource *templ,
                                                    struct pipe_memory_object *_memobj,
                                                    uint64_t offset)
{
   struct si_screen *sscreen = (struct si_screen *)screen;
   struct si_memory_object *memobj = (struct si_memory_object *)_memobj;
   struct pipe_resource *res;

   if (templ->target == PIPE_BUFFER)
      res = si_buffer_from_winsys_buffer(screen, templ, memobj->buf, offset);
   else
      res = si_texture_from_winsys_buffer(sscreen, templ, memobj->buf,
                                          memobj->stride,
                                          offset, DRM_FORMAT_MOD_INVALID,
                                          PIPE_HANDLE_USAGE_FRAMEBUFFER_WRITE | PIPE_HANDLE_USAGE_SHADER_WRITE,
                                          memobj->b.dedicated);

   if (!res)
      return NULL;

   /* si_texture_from_winsys_buffer doesn't increment refcount of
    * memobj->buf, so increment it here.
    */
   struct pb_buffer_lean *buf = NULL;
   radeon_bo_reference(sscreen->ws, &buf, memobj->buf);
   return res;
}

static bool si_check_resource_capability(struct pipe_screen *screen, struct pipe_resource *resource,
                                         unsigned bind)
{
   struct si_texture *tex = (struct si_texture *)resource;

   /* Buffers only support the linear flag. */
   if (resource->target == PIPE_BUFFER)
      return (bind & ~PIPE_BIND_LINEAR) == 0;

   if (bind & PIPE_BIND_LINEAR && !tex->surface.is_linear)
      return false;

   if (bind & PIPE_BIND_SCANOUT && !tex->surface.is_displayable)
      return false;

   /* TODO: PIPE_BIND_CURSOR - do we care? */
   return true;
}

static int si_get_sparse_texture_virtual_page_size(struct pipe_screen *screen,
                                                   enum pipe_texture_target target,
                                                   bool multi_sample,
                                                   enum pipe_format format,
                                                   unsigned offset, unsigned size,
                                                   int *x, int *y, int *z)
{
   struct si_screen *sscreen = (struct si_screen *)screen;

   /* Only support one type of page size. */
   if (offset != 0)
      return 0;

   static const int page_size_2d[][3] = {
      { 256, 256, 1 }, /* 8bpp   */
      { 256, 128, 1 }, /* 16bpp  */
      { 128, 128, 1 }, /* 32bpp  */
      { 128, 64,  1 }, /* 64bpp  */
      { 64,  64,  1 }, /* 128bpp */
   };
   static const int page_size_3d[][3] = {
      { 64,  32,  32 }, /* 8bpp   */
      { 32,  32,  32 }, /* 16bpp  */
      { 32,  32,  16 }, /* 32bpp  */
      { 32,  16,  16 }, /* 64bpp  */
      { 16,  16,  16 }, /* 128bpp */
   };

   const int (*page_sizes)[3];

   /* Supported targets. */
   switch (target) {
   case PIPE_TEXTURE_2D:
   case PIPE_TEXTURE_CUBE:
   case PIPE_TEXTURE_RECT:
   case PIPE_TEXTURE_2D_ARRAY:
   case PIPE_TEXTURE_CUBE_ARRAY:
      page_sizes = page_size_2d;
      break;
   case PIPE_TEXTURE_3D:
      page_sizes = page_size_3d;
      break;
   default:
      return 0;
   }

   /* ARB_sparse_texture2 need to query supported virtual page x/y/z without
    * knowing the actual sample count. So we need to return a fixed virtual page
    * x/y/z for all sample count which means the virtual page size can not be fixed
    * to 64KB.
    *
    * Only enabled for GFX9. GFX10+ removed MS texture support. By specification
    * ARB_sparse_texture2 need MS texture support, but we relax it by just return
    * no page size for GFX10+ to keep shader query capbility.
    */
   if (multi_sample && sscreen->info.gfx_level != GFX9)
      return 0;

   /* Unsupported formats. */
   /* TODO: support these formats. */
   if (util_format_is_depth_or_stencil(format) ||
       util_format_get_num_planes(format) > 1 ||
       util_format_is_compressed(format))
      return 0;

   int blk_size = util_format_get_blocksize(format);
   /* We don't support any non-power-of-two bpp formats, so
    * pipe_screen->is_format_supported() should already filter out these formats.
    */
   assert(util_is_power_of_two_nonzero(blk_size));

   if (size) {
      unsigned index = util_logbase2(blk_size);
      if (x) *x = page_sizes[index][0];
      if (y) *y = page_sizes[index][1];
      if (z) *z = page_sizes[index][2];
   }

   return 1;
}

void si_init_screen_texture_functions(struct si_screen *sscreen)
{
   sscreen->b.resource_from_handle = si_texture_from_handle;
   sscreen->b.resource_get_handle = si_texture_get_handle;
   sscreen->b.resource_get_param = si_resource_get_param;
   sscreen->b.resource_get_info = si_texture_get_info;
   sscreen->b.resource_from_memobj = si_resource_from_memobj;
   sscreen->b.memobj_create_from_handle = si_memobj_from_handle;
   sscreen->b.memobj_destroy = si_memobj_destroy;
   sscreen->b.check_resource_capability = si_check_resource_capability;
   sscreen->b.get_sparse_texture_virtual_page_size =
      si_get_sparse_texture_virtual_page_size;

   /* By not setting it the frontend will fall back to non-modifier create,
    * which works around some applications using modifiers that are not
    * allowed in combination with lack of error reporting in
    * gbm_dri_surface_create */
   if (sscreen->info.gfx_level >= GFX9 && sscreen->info.kernel_has_modifiers) {
      sscreen->b.resource_create_with_modifiers = si_texture_create_with_modifiers;
      sscreen->b.query_dmabuf_modifiers = si_query_dmabuf_modifiers;
      sscreen->b.is_dmabuf_modifier_supported = si_is_dmabuf_modifier_supported;
      sscreen->b.get_dmabuf_modifier_planes = si_get_dmabuf_modifier_planes;
   }
}

void si_init_context_texture_functions(struct si_context *sctx)
{
   sctx->b.texture_map = si_texture_transfer_map;
   sctx->b.texture_unmap = si_texture_transfer_unmap;
   sctx->b.create_surface = si_create_surface;
   sctx->b.surface_destroy = si_surface_destroy;
}
