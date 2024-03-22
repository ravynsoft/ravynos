/*
 * Copyright © 2014 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#include "radeon_drm_winsys.h"
#include "util/format/u_format.h"
#include <radeon_surface.h>

static unsigned cik_get_macro_tile_index(struct radeon_surf *surf)
{
   unsigned index, tileb;

   tileb = 8 * 8 * surf->bpe;
   tileb = MIN2(surf->u.legacy.tile_split, tileb);

   for (index = 0; tileb > 64; index++)
      tileb >>= 1;

   assert(index < 16);
   return index;
}

#define   G_009910_MICRO_TILE_MODE(x)          (((x) >> 0) & 0x03)
#define   G_009910_MICRO_TILE_MODE_NEW(x)      (((x) >> 22) & 0x07)

static void set_micro_tile_mode(struct radeon_surf *surf,
                                struct radeon_info *info)
{
   uint32_t tile_mode;

   if (info->gfx_level < GFX6) {
      surf->micro_tile_mode = 0;
      return;
   }

   tile_mode = info->si_tile_mode_array[surf->u.legacy.tiling_index[0]];

   if (info->gfx_level >= GFX7)
      surf->micro_tile_mode = G_009910_MICRO_TILE_MODE_NEW(tile_mode);
   else
      surf->micro_tile_mode = G_009910_MICRO_TILE_MODE(tile_mode);
}

static void surf_level_winsys_to_drm(struct radeon_surface_level *level_drm,
                                     const struct legacy_surf_level *level_ws,
                                     unsigned bpe)
{
   level_drm->offset = (uint64_t)level_ws->offset_256B * 256;
   level_drm->slice_size = (uint64_t)level_ws->slice_size_dw * 4;
   level_drm->nblk_x = level_ws->nblk_x;
   level_drm->nblk_y = level_ws->nblk_y;
   level_drm->pitch_bytes = level_ws->nblk_x * bpe;
   level_drm->mode = level_ws->mode;
}

static void surf_level_drm_to_winsys(struct legacy_surf_level *level_ws,
                                     const struct radeon_surface_level *level_drm,
                                     unsigned bpe)
{
   level_ws->offset_256B = level_drm->offset / 256;
   level_ws->slice_size_dw = level_drm->slice_size / 4;
   level_ws->nblk_x = level_drm->nblk_x;
   level_ws->nblk_y = level_drm->nblk_y;
   level_ws->mode = level_drm->mode;
   assert(level_drm->nblk_x * bpe == level_drm->pitch_bytes);
}

static void surf_winsys_to_drm(struct radeon_surface *surf_drm,
                               const struct pipe_resource *tex,
                               unsigned flags, unsigned bpe,
                               enum radeon_surf_mode mode,
                               const struct radeon_surf *surf_ws)
{
   int i;

   memset(surf_drm, 0, sizeof(*surf_drm));

   surf_drm->npix_x = tex->width0;
   surf_drm->npix_y = tex->height0;
   surf_drm->npix_z = tex->depth0;
   surf_drm->blk_w = util_format_get_blockwidth(tex->format);
   surf_drm->blk_h = util_format_get_blockheight(tex->format);
   surf_drm->blk_d = 1;
   surf_drm->array_size = 1;
   surf_drm->last_level = tex->last_level;
   surf_drm->bpe = bpe;
   surf_drm->nsamples = tex->nr_samples ? tex->nr_samples : 1;

   surf_drm->flags = flags;
   surf_drm->flags = RADEON_SURF_CLR(surf_drm->flags, TYPE);
   surf_drm->flags = RADEON_SURF_CLR(surf_drm->flags, MODE);
   surf_drm->flags |= RADEON_SURF_SET(mode, MODE) |
                      RADEON_SURF_HAS_SBUFFER_MIPTREE |
                      RADEON_SURF_HAS_TILE_MODE_INDEX;

   switch (tex->target) {
   case PIPE_TEXTURE_1D:
      surf_drm->flags |= RADEON_SURF_SET(RADEON_SURF_TYPE_1D, TYPE);
      break;
   case PIPE_TEXTURE_RECT:
   case PIPE_TEXTURE_2D:
      surf_drm->flags |= RADEON_SURF_SET(RADEON_SURF_TYPE_2D, TYPE);
      break;
   case PIPE_TEXTURE_3D:
      surf_drm->flags |= RADEON_SURF_SET(RADEON_SURF_TYPE_3D, TYPE);
      break;
   case PIPE_TEXTURE_1D_ARRAY:
      surf_drm->flags |= RADEON_SURF_SET(RADEON_SURF_TYPE_1D_ARRAY, TYPE);
      surf_drm->array_size = tex->array_size;
      break;
   case PIPE_TEXTURE_CUBE_ARRAY: /* cube array layout like 2d array */
      assert(tex->array_size % 6 == 0);
      FALLTHROUGH;
   case PIPE_TEXTURE_2D_ARRAY:
      surf_drm->flags |= RADEON_SURF_SET(RADEON_SURF_TYPE_2D_ARRAY, TYPE);
      surf_drm->array_size = tex->array_size;
      break;
   case PIPE_TEXTURE_CUBE:
      surf_drm->flags |= RADEON_SURF_SET(RADEON_SURF_TYPE_CUBEMAP, TYPE);
      break;
   case PIPE_BUFFER:
   default:
      assert(0);
   }

   surf_drm->bo_size = surf_ws->surf_size;
   surf_drm->bo_alignment = 1 << surf_ws->surf_alignment_log2;

   surf_drm->bankw = surf_ws->u.legacy.bankw;
   surf_drm->bankh = surf_ws->u.legacy.bankh;
   surf_drm->mtilea = surf_ws->u.legacy.mtilea;
   surf_drm->tile_split = surf_ws->u.legacy.tile_split;

   for (i = 0; i <= surf_drm->last_level; i++) {
      surf_level_winsys_to_drm(&surf_drm->level[i], &surf_ws->u.legacy.level[i],
                               bpe * surf_drm->nsamples);

      surf_drm->tiling_index[i] = surf_ws->u.legacy.tiling_index[i];
   }

   if (flags & RADEON_SURF_SBUFFER) {
      surf_drm->stencil_tile_split = surf_ws->u.legacy.stencil_tile_split;

      for (i = 0; i <= surf_drm->last_level; i++) {
         surf_level_winsys_to_drm(&surf_drm->stencil_level[i],
                                  &surf_ws->u.legacy.zs.stencil_level[i],
                                  surf_drm->nsamples);
         surf_drm->stencil_tiling_index[i] = surf_ws->u.legacy.zs.stencil_tiling_index[i];
      }
   }
}

static void surf_drm_to_winsys(struct radeon_drm_winsys *ws,
                               struct radeon_surf *surf_ws,
                               const struct radeon_surface *surf_drm)
{
   int i;

   memset(surf_ws, 0, sizeof(*surf_ws));

   surf_ws->blk_w = surf_drm->blk_w;
   surf_ws->blk_h = surf_drm->blk_h;
   surf_ws->bpe = surf_drm->bpe;
   surf_ws->is_linear = surf_drm->level[0].mode <= RADEON_SURF_MODE_LINEAR_ALIGNED;
   surf_ws->has_stencil = !!(surf_drm->flags & RADEON_SURF_SBUFFER);
   surf_ws->flags = surf_drm->flags;

   surf_ws->surf_size = surf_drm->bo_size;
   surf_ws->surf_alignment_log2 = util_logbase2(surf_drm->bo_alignment);

   surf_ws->u.legacy.bankw = surf_drm->bankw;
   surf_ws->u.legacy.bankh = surf_drm->bankh;
   surf_ws->u.legacy.mtilea = surf_drm->mtilea;
   surf_ws->u.legacy.tile_split = surf_drm->tile_split;

   surf_ws->u.legacy.macro_tile_index = cik_get_macro_tile_index(surf_ws);

   for (i = 0; i <= surf_drm->last_level; i++) {
      surf_level_drm_to_winsys(&surf_ws->u.legacy.level[i], &surf_drm->level[i],
                               surf_drm->bpe * surf_drm->nsamples);
      surf_ws->u.legacy.tiling_index[i] = surf_drm->tiling_index[i];
   }

   if (surf_ws->flags & RADEON_SURF_SBUFFER) {
      surf_ws->u.legacy.stencil_tile_split = surf_drm->stencil_tile_split;

      for (i = 0; i <= surf_drm->last_level; i++) {
         surf_level_drm_to_winsys(&surf_ws->u.legacy.zs.stencil_level[i],
                                  &surf_drm->stencil_level[i],
                                  surf_drm->nsamples);
         surf_ws->u.legacy.zs.stencil_tiling_index[i] = surf_drm->stencil_tiling_index[i];
      }
   }

   set_micro_tile_mode(surf_ws, &ws->info);
   surf_ws->is_displayable = surf_ws->is_linear ||
                             surf_ws->micro_tile_mode == RADEON_MICRO_MODE_DISPLAY ||
                             surf_ws->micro_tile_mode == RADEON_MICRO_MODE_RENDER;
}

static void si_compute_cmask(const struct radeon_info *info,
                             const struct ac_surf_config *config,
                             struct radeon_surf *surf)
{
   unsigned pipe_interleave_bytes = info->pipe_interleave_bytes;
   unsigned num_pipes = info->num_tile_pipes;
   unsigned cl_width, cl_height;

   if (surf->flags & RADEON_SURF_Z_OR_SBUFFER)
      return;

   assert(info->gfx_level <= GFX8);

   switch (num_pipes) {
   case 2:
      cl_width = 32;
      cl_height = 16;
      break;
   case 4:
      cl_width = 32;
      cl_height = 32;
      break;
   case 8:
      cl_width = 64;
      cl_height = 32;
      break;
   case 16: /* Hawaii */
      cl_width = 64;
      cl_height = 64;
      break;
   default:
      assert(0);
      return;
   }

   unsigned base_align = num_pipes * pipe_interleave_bytes;

   unsigned width = align(surf->u.legacy.level[0].nblk_x, cl_width*8);
   unsigned height = align(surf->u.legacy.level[0].nblk_y, cl_height*8);
   unsigned slice_elements = (width * height) / (8*8);

   /* Each element of CMASK is a nibble. */
   unsigned slice_bytes = slice_elements / 2;

   surf->u.legacy.color.cmask_slice_tile_max = (width * height) / (128*128);
   if (surf->u.legacy.color.cmask_slice_tile_max)
      surf->u.legacy.color.cmask_slice_tile_max -= 1;

   unsigned num_layers;
   if (config->is_3d)
      num_layers = config->info.depth;
   else if (config->is_cube)
      num_layers = 6;
   else
      num_layers = config->info.array_size;

   surf->cmask_alignment_log2 = util_logbase2(MAX2(256, base_align));
   surf->cmask_size = align(slice_bytes, base_align) * num_layers;
}

static void si_compute_htile(const struct radeon_info *info,
                             struct radeon_surf *surf, unsigned num_layers)
{
   unsigned cl_width, cl_height, width, height;
   unsigned slice_elements, slice_bytes, pipe_interleave_bytes, base_align;
   unsigned num_pipes = info->num_tile_pipes;

   surf->meta_size = 0;

   if (!(surf->flags & RADEON_SURF_Z_OR_SBUFFER) ||
       surf->flags & RADEON_SURF_NO_HTILE)
      return;

   /* Overalign HTILE on P2 configs to work around GPU hangs in
     * piglit/depthstencil-render-miplevels 585.
     *
     * This has been confirmed to help Kabini & Stoney, where the hangs
     * are always reproducible. I think I have seen the test hang
     * on Carrizo too, though it was very rare there.
     */
   if (info->gfx_level >= GFX7 && num_pipes < 4)
      num_pipes = 4;

   switch (num_pipes) {
   case 1:
      cl_width = 32;
      cl_height = 16;
      break;
   case 2:
      cl_width = 32;
      cl_height = 32;
      break;
   case 4:
      cl_width = 64;
      cl_height = 32;
      break;
   case 8:
      cl_width = 64;
      cl_height = 64;
      break;
   case 16:
      cl_width = 128;
      cl_height = 64;
      break;
   default:
      assert(0);
      return;
   }

   width = align(surf->u.legacy.level[0].nblk_x, cl_width * 8);
   height = align(surf->u.legacy.level[0].nblk_y, cl_height * 8);

   slice_elements = (width * height) / (8 * 8);
   slice_bytes = slice_elements * 4;

   pipe_interleave_bytes = info->pipe_interleave_bytes;
   base_align = num_pipes * pipe_interleave_bytes;

   surf->meta_alignment_log2 = util_logbase2(base_align);
   surf->meta_size = num_layers * align(slice_bytes, base_align);
}

static int radeon_winsys_surface_init(struct radeon_winsys *rws,
                                      const struct radeon_info *info,
                                      const struct pipe_resource *tex,
                                      uint64_t flags, unsigned bpe,
                                      enum radeon_surf_mode mode,
                                      struct radeon_surf *surf_ws)
{
   struct radeon_drm_winsys *ws = (struct radeon_drm_winsys*)rws;
   struct radeon_surface surf_drm;
   int r;

   surf_winsys_to_drm(&surf_drm, tex, flags, bpe, mode, surf_ws);

   if (!(flags & (RADEON_SURF_IMPORTED | RADEON_SURF_FMASK))) {
      r = radeon_surface_best(ws->surf_man, &surf_drm);
      if (r)
         return r;
   }

   r = radeon_surface_init(ws->surf_man, &surf_drm);
   if (r)
      return r;

   surf_drm_to_winsys(ws, surf_ws, &surf_drm);

   /* Compute FMASK. */
   if (ws->gen == DRV_SI &&
       tex->nr_samples >= 2 &&
       !(flags & (RADEON_SURF_Z_OR_SBUFFER | RADEON_SURF_FMASK | RADEON_SURF_NO_FMASK))) {
      /* FMASK is allocated like an ordinary texture. */
      struct pipe_resource templ = *tex;
      struct radeon_surf fmask = {};
      unsigned fmask_flags, bpe;

      templ.nr_samples = 1;
      fmask_flags = flags | RADEON_SURF_FMASK;

      switch (tex->nr_samples) {
      case 2:
      case 4:
         bpe = 1;
         break;
      case 8:
         bpe = 4;
         break;
      default:
         fprintf(stderr, "radeon: Invalid sample count for FMASK allocation.\n");
         return -1;
      }

      if (radeon_winsys_surface_init(rws, info, &templ, fmask_flags, bpe,
                                     RADEON_SURF_MODE_2D, &fmask)) {
         fprintf(stderr, "Got error in surface_init while allocating FMASK.\n");
         return -1;
      }

      assert(fmask.u.legacy.level[0].mode == RADEON_SURF_MODE_2D);

      surf_ws->fmask_size = fmask.surf_size;
      surf_ws->fmask_alignment_log2 = util_logbase2(MAX2(256, 1 << fmask.surf_alignment_log2));
      surf_ws->fmask_tile_swizzle = fmask.tile_swizzle;

      surf_ws->u.legacy.color.fmask.slice_tile_max =
            (fmask.u.legacy.level[0].nblk_x * fmask.u.legacy.level[0].nblk_y) / 64;
      if (surf_ws->u.legacy.color.fmask.slice_tile_max)
         surf_ws->u.legacy.color.fmask.slice_tile_max -= 1;

      surf_ws->u.legacy.color.fmask.tiling_index = fmask.u.legacy.tiling_index[0];
      surf_ws->u.legacy.color.fmask.bankh = fmask.u.legacy.bankh;
      surf_ws->u.legacy.color.fmask.pitch_in_pixels = fmask.u.legacy.level[0].nblk_x;
   }

   if (ws->gen == DRV_SI &&
       (tex->nr_samples <= 1 || surf_ws->fmask_size)) {
      struct ac_surf_config config;

      /* Only these fields need to be set for the CMASK computation. */
      config.info.width = tex->width0;
      config.info.height = tex->height0;
      config.info.depth = tex->depth0;
      config.info.array_size = tex->array_size;
      config.is_3d = !!(tex->target == PIPE_TEXTURE_3D);
      config.is_cube = !!(tex->target == PIPE_TEXTURE_CUBE);
      config.is_array = tex->target == PIPE_TEXTURE_1D_ARRAY ||
                        tex->target == PIPE_TEXTURE_2D_ARRAY ||
                        tex->target == PIPE_TEXTURE_CUBE_ARRAY;

      si_compute_cmask(&ws->info, &config, surf_ws);
   }

   if (ws->gen == DRV_SI) {
      si_compute_htile(&ws->info, surf_ws, util_num_layers(tex, 0));

      /* Determine the memory layout of multiple allocations in one buffer. */
      surf_ws->total_size = surf_ws->surf_size;

      if (surf_ws->meta_size) {
         surf_ws->meta_offset = align64(surf_ws->total_size, 1 << surf_ws->meta_alignment_log2);
         surf_ws->total_size = surf_ws->meta_offset + surf_ws->meta_size;
      }

      if (surf_ws->fmask_size) {
         assert(tex->nr_samples >= 2);
         surf_ws->fmask_offset = align64(surf_ws->total_size, 1 << surf_ws->fmask_alignment_log2);
         surf_ws->total_size = surf_ws->fmask_offset + surf_ws->fmask_size;
      }

      /* Single-sample CMASK is in a separate buffer. */
      if (surf_ws->cmask_size && tex->nr_samples >= 2) {
         surf_ws->cmask_offset = align64(surf_ws->total_size, 1 << surf_ws->cmask_alignment_log2);
         surf_ws->total_size = surf_ws->cmask_offset + surf_ws->cmask_size;
      }
   }

   return 0;
}

void radeon_surface_init_functions(struct radeon_drm_winsys *ws)
{
   ws->base.surface_init = radeon_winsys_surface_init;
}
