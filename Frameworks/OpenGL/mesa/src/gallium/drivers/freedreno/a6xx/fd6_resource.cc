/*
 * Copyright (C) 2018 Rob Clark <robclark@freedesktop.org>
 * Copyright © 2018 Google, Inc.
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
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 */

#define FD_BO_NO_HARDPIN 1

#include "drm-uapi/drm_fourcc.h"

#include "a6xx/fd6_blitter.h"
#include "fd6_resource.h"
#include "fdl/fd6_format_table.h"

#include "a6xx.xml.h"

/* A subset of the valid tiled formats can be compressed.  We do
 * already require tiled in order to be compressed, but just because
 * it can be tiled doesn't mean it can be compressed.
 */
static bool
ok_ubwc_format(struct pipe_screen *pscreen, enum pipe_format pfmt)
{
   const struct fd_dev_info *info = fd_screen(pscreen)->info;

   switch (pfmt) {
   case PIPE_FORMAT_X24S8_UINT:
   case PIPE_FORMAT_Z24_UNORM_S8_UINT:
      /* We can't sample stencil with UBWC on a630, and we may need to be able
       * to sample stencil at some point.  We can't just use
       * fd_resource_uncompress() at the point of stencil sampling because
       * that itself uses stencil sampling in the fd_blitter_blit path.
       */
      return info->a6xx.has_z24uint_s8uint;

   case PIPE_FORMAT_R8_G8B8_420_UNORM:
      /* The difference between NV12 and R8_G8B8_420_UNORM is only where the
       * conversion to RGB happens, with the latter it happens _after_ the
       * texture samp instruction.  But dri2_get_mapping_by_fourcc() doesn't
       * know this, so it asks for NV12 when it really meant to ask for
       * R8_G8B8_420_UNORM.  Just treat them the same here to work around it:
       */
   case PIPE_FORMAT_NV12:
      return true;

   default:
      break;
   }

   /* A690 seem to have broken UBWC for depth/stencil, it requires
    * depth flushing where we cannot realistically place it, like between
    * ordinary draw calls writing read/depth. WSL blob seem to use ubwc
    * sometimes for depth/stencil.
    */
   if (info->a6xx.broken_ds_ubwc_quirk &&
       util_format_is_depth_or_stencil(pfmt))
      return false;

   switch (fd6_color_format(pfmt, TILE6_LINEAR)) {
   case FMT6_10_10_10_2_UINT:
   case FMT6_10_10_10_2_UNORM_DEST:
   case FMT6_11_11_10_FLOAT:
   case FMT6_16_FLOAT:
   case FMT6_16_16_16_16_FLOAT:
   case FMT6_16_16_16_16_SINT:
   case FMT6_16_16_16_16_UINT:
   case FMT6_16_16_FLOAT:
   case FMT6_16_16_SINT:
   case FMT6_16_16_UINT:
   case FMT6_16_SINT:
   case FMT6_16_UINT:
   case FMT6_32_32_32_32_SINT:
   case FMT6_32_32_32_32_UINT:
   case FMT6_32_32_SINT:
   case FMT6_32_32_UINT:
   case FMT6_5_6_5_UNORM:
   case FMT6_5_5_5_1_UNORM:
   case FMT6_8_8_8_8_SINT:
   case FMT6_8_8_8_8_UINT:
   case FMT6_8_8_8_8_UNORM:
   case FMT6_8_8_8_X8_UNORM:
   case FMT6_8_8_SINT:
   case FMT6_8_8_UINT:
   case FMT6_8_8_UNORM:
   case FMT6_Z24_UNORM_S8_UINT:
   case FMT6_Z24_UNORM_S8_UINT_AS_R8G8B8A8:
      return true;
   case FMT6_8_UNORM:
      return info->a6xx.has_8bpp_ubwc;
   default:
      return false;
   }
}

static bool
can_do_ubwc(struct pipe_resource *prsc)
{
   /* limit things to simple single level 2d for now: */
   if ((prsc->depth0 != 1) || (prsc->array_size != 1) ||
       (prsc->last_level != 0))
      return false;
   if (prsc->target != PIPE_TEXTURE_2D)
      return false;
   if (!ok_ubwc_format(prsc->screen, prsc->format))
      return false;
   return true;
}

static bool
is_norm(enum pipe_format format)
{
   const struct util_format_description *desc = util_format_description(format);

   return desc->is_snorm || desc->is_unorm;
}

static bool
is_z24s8(enum pipe_format format)
{
   switch (format) {
   case PIPE_FORMAT_Z24_UNORM_S8_UINT:
   case PIPE_FORMAT_Z24X8_UNORM:
   case PIPE_FORMAT_X24S8_UINT:
   case PIPE_FORMAT_Z24_UNORM_S8_UINT_AS_R8G8B8A8:
      return true;
   default:
      return false;
   }
}

static bool
valid_format_cast(struct fd_resource *rsc, enum pipe_format format)
{
   /* Special case "casting" format in hw: */
   if (format == PIPE_FORMAT_Z24_UNORM_S8_UINT_AS_R8G8B8A8)
      return true;

   /* If we support z24s8 ubwc then allow casts between the various
    * permutations of z24s8:
    */
   if (fd_screen(rsc->b.b.screen)->info->a6xx.has_z24uint_s8uint &&
         is_z24s8(format) && is_z24s8(rsc->b.b.format))
      return true;

   /* For some color values (just "solid white") compression metadata maps to
    * different pixel values for uint/sint vs unorm/snorm, so we can't reliably
    * "cast" u/snorm to u/sint and visa versa:
    */
   if (is_norm(format) != is_norm(rsc->b.b.format))
      return false;

   /* The UBWC formats can be re-interpreted so long as the components
    * have the same # of bits
    */
   for (unsigned i = 0; i < 4; i++) {
      unsigned sb, db;

      sb = util_format_get_component_bits(rsc->b.b.format, UTIL_FORMAT_COLORSPACE_RGB, i);
      db = util_format_get_component_bits(format, UTIL_FORMAT_COLORSPACE_RGB, i);

      if (sb != db)
         return false;
   }

   return true;
}

/**
 * R8G8 have a different block width/height and height alignment from other
 * formats that would normally be compatible (like R16), and so if we are
 * trying to, for example, sample R16 as R8G8 we need to demote to linear.
 */
static bool
is_r8g8(enum pipe_format format)
{
   return (util_format_get_blocksize(format) == 2) &&
         (util_format_get_nr_components(format) == 2);
}

/**
 * Can a rsc as it is currently laid out be accessed as the specified format.
 * Returns whether the access is ok or whether the rsc needs to be demoted
 * to uncompressed tiled or linear.
 */
enum fd6_format_status
fd6_check_valid_format(struct fd_resource *rsc, enum pipe_format format)
{
   enum pipe_format orig_format = rsc->b.b.format;

   if (orig_format == format)
      return FORMAT_OK;

   if (rsc->layout.tile_mode && (is_r8g8(orig_format) != is_r8g8(format)))
      return DEMOTE_TO_LINEAR;

   if (!rsc->layout.ubwc)
      return FORMAT_OK;

   if (ok_ubwc_format(rsc->b.b.screen, format) && valid_format_cast(rsc, format))
      return FORMAT_OK;

   return DEMOTE_TO_TILED;
}

/**
 * Ensure the rsc is in an ok state to be used with the specified format.
 * This handles the case of UBWC buffers used with non-UBWC compatible
 * formats, by triggering an uncompress.
 */
void
fd6_validate_format(struct fd_context *ctx, struct fd_resource *rsc,
                    enum pipe_format format)
{
   tc_assert_driver_thread(ctx->tc);

   switch (fd6_check_valid_format(rsc, format)) {
   case FORMAT_OK:
      return;
   case DEMOTE_TO_LINEAR:
      perf_debug_ctx(ctx,
                     "%" PRSC_FMT ": demoted to linear+uncompressed due to use as %s",
                     PRSC_ARGS(&rsc->b.b), util_format_short_name(format));

      fd_resource_uncompress(ctx, rsc, true);
      return;
   case DEMOTE_TO_TILED:
      perf_debug_ctx(ctx,
                     "%" PRSC_FMT ": demoted to uncompressed due to use as %s",
                     PRSC_ARGS(&rsc->b.b), util_format_short_name(format));

      fd_resource_uncompress(ctx, rsc, false);
      return;
   }
}

static void
setup_lrz(struct fd_resource *rsc)
{
   struct fd_screen *screen = fd_screen(rsc->b.b.screen);
   unsigned width0 = rsc->b.b.width0;
   unsigned height0 = rsc->b.b.height0;

   /* LRZ buffer is super-sampled: */
   switch (rsc->b.b.nr_samples) {
   case 4:
      width0 *= 2;
      FALLTHROUGH;
   case 2:
      height0 *= 2;
   }

   unsigned lrz_pitch = align(DIV_ROUND_UP(width0, 8), 32);
   unsigned lrz_height = align(DIV_ROUND_UP(height0, 8), 16);

   unsigned size = lrz_pitch * lrz_height * 2;

   rsc->lrz_height = lrz_height;
   rsc->lrz_width = lrz_pitch;
   rsc->lrz_pitch = lrz_pitch;
   rsc->lrz = fd_bo_new(screen->dev, size, FD_BO_NOMAP, "lrz");
}

static uint32_t
fd6_setup_slices(struct fd_resource *rsc)
{
   struct pipe_resource *prsc = &rsc->b.b;

   if (!FD_DBG(NOLRZ) && has_depth(prsc->format) && !is_z32(prsc->format))
      setup_lrz(rsc);

   if (rsc->layout.ubwc && !ok_ubwc_format(prsc->screen, prsc->format))
      rsc->layout.ubwc = false;

   fdl6_layout(&rsc->layout, prsc->format, fd_resource_nr_samples(prsc),
               prsc->width0, prsc->height0, prsc->depth0, prsc->last_level + 1,
               prsc->array_size, prsc->target == PIPE_TEXTURE_3D, NULL);

   return rsc->layout.size;
}

static int
fill_ubwc_buffer_sizes(struct fd_resource *rsc)
{
   struct pipe_resource *prsc = &rsc->b.b;
   struct fdl_explicit_layout l = {
      .offset = rsc->layout.slices[0].offset,
      .pitch = rsc->layout.pitch0,
   };

   if (!can_do_ubwc(prsc))
      return -1;

   rsc->layout.ubwc = true;
   rsc->layout.tile_mode = TILE6_3;

   if (!fdl6_layout(&rsc->layout, prsc->format, fd_resource_nr_samples(prsc),
                    prsc->width0, prsc->height0, prsc->depth0,
                    prsc->last_level + 1, prsc->array_size, false, &l))
      return -1;

   if (rsc->layout.size > fd_bo_size(rsc->bo))
      return -1;

   return 0;
}

static int
fd6_layout_resource_for_modifier(struct fd_resource *rsc, uint64_t modifier)
{
   switch (modifier) {
   case DRM_FORMAT_MOD_QCOM_COMPRESSED:
      return fill_ubwc_buffer_sizes(rsc);
   case DRM_FORMAT_MOD_LINEAR:
      if (can_do_ubwc(&rsc->b.b)) {
         perf_debug("%" PRSC_FMT
                    ": not UBWC: imported with DRM_FORMAT_MOD_LINEAR!",
                    PRSC_ARGS(&rsc->b.b));
      }
      return 0;
   case DRM_FORMAT_MOD_QCOM_TILED3:
      rsc->layout.tile_mode = fd6_tile_mode(&rsc->b.b);
      FALLTHROUGH;
   case DRM_FORMAT_MOD_INVALID:
      /* For now, without buffer metadata, we must assume that buffers
       * imported with INVALID modifier are linear
       */
      if (can_do_ubwc(&rsc->b.b)) {
         perf_debug("%" PRSC_FMT
                    ": not UBWC: imported with DRM_FORMAT_MOD_INVALID!",
                    PRSC_ARGS(&rsc->b.b));
      }
      return 0;
   default:
      return -1;
   }
}

static bool
fd6_is_format_supported(struct pipe_screen *pscreen,
                        enum pipe_format fmt,
                        uint64_t modifier)
{
   switch (modifier) {
   case DRM_FORMAT_MOD_LINEAR:
      return true;
   case DRM_FORMAT_MOD_QCOM_COMPRESSED:
      return ok_ubwc_format(pscreen, fmt);
   case DRM_FORMAT_MOD_QCOM_TILED3:
      return fd6_tile_mode_for_format(fmt) == TILE6_3;
   default:
      return false;
   }
}

void
fd6_resource_screen_init(struct pipe_screen *pscreen)
{
   struct fd_screen *screen = fd_screen(pscreen);

   screen->setup_slices = fd6_setup_slices;
   screen->layout_resource_for_modifier = fd6_layout_resource_for_modifier;
   screen->is_format_supported = fd6_is_format_supported;
}
