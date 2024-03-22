/*
 * Copyright (C) 2021 Collabora, Ltd.
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
 *   Alyssa Rosenzweig <alyssa.rosenzweig@collabora.com>
 *   Boris Brezillon <boris.brezillon@collabora.com>
 */

#include "util/macros.h"

#include "genxml/gen_macros.h"

#include "pan_desc.h"
#include "pan_encoder.h"
#include "pan_texture.h"

static unsigned
mod_to_block_fmt(uint64_t mod)
{
   switch (mod) {
   case DRM_FORMAT_MOD_LINEAR:
      return MALI_BLOCK_FORMAT_LINEAR;
   case DRM_FORMAT_MOD_ARM_16X16_BLOCK_U_INTERLEAVED:
      return MALI_BLOCK_FORMAT_TILED_U_INTERLEAVED;
   default:
#if PAN_ARCH >= 5
      if (drm_is_afbc(mod) && !(mod & AFBC_FORMAT_MOD_TILED))
         return MALI_BLOCK_FORMAT_AFBC;
#endif
#if PAN_ARCH >= 7
      if (drm_is_afbc(mod) && (mod & AFBC_FORMAT_MOD_TILED))
         return MALI_BLOCK_FORMAT_AFBC_TILED;
#endif

      unreachable("Unsupported modifer");
   }
}

static enum mali_msaa
mali_sampling_mode(const struct pan_image_view *view)
{
   unsigned nr_samples = pan_image_view_get_nr_samples(view);

   if (nr_samples > 1) {
      assert(view->nr_samples == nr_samples);
      assert(view->planes[0]->layout.slices[0].surface_stride != 0);
      return MALI_MSAA_LAYERED;
   }

   if (view->nr_samples > nr_samples) {
      assert(nr_samples == 1);
      return MALI_MSAA_AVERAGE;
   }

   assert(view->nr_samples == nr_samples);
   assert(view->nr_samples == 1);

   return MALI_MSAA_SINGLE;
}

int
GENX(pan_select_crc_rt)(const struct pan_fb_info *fb, unsigned tile_size)
{
   /* Disable CRC when the tile size is not 16x16. In the hardware, CRC
    * tiles are the same size as the tiles of the framebuffer. However,
    * our code only handles 16x16 tiles. Therefore under the current
    * implementation, we must disable CRC when 16x16 tiles are not used.
    *
    * This may hurt performance. However, smaller tile sizes are rare, and
    * CRCs are more expensive at smaller tile sizes, reducing the benefit.
    * Restricting CRC to 16x16 should work in practice.
    */
   if (tile_size != 16 * 16) {
      assert(tile_size < 16 * 16);
      return -1;
   }

#if PAN_ARCH <= 6
   if (fb->rt_count == 1 && fb->rts[0].view && !fb->rts[0].discard &&
       pan_image_view_has_crc(fb->rts[0].view))
      return 0;

   return -1;
#else
   bool best_rt_valid = false;
   int best_rt = -1;

   for (unsigned i = 0; i < fb->rt_count; i++) {
      if (!fb->rts[i].view || fb->rts[0].discard ||
          !pan_image_view_has_crc(fb->rts[i].view))
         continue;

      bool valid = *(fb->rts[i].crc_valid);
      bool full = !fb->extent.minx && !fb->extent.miny &&
                  fb->extent.maxx == (fb->width - 1) &&
                  fb->extent.maxy == (fb->height - 1);
      if (!full && !valid)
         continue;

      if (best_rt < 0 || (valid && !best_rt_valid)) {
         best_rt = i;
         best_rt_valid = valid;
      }

      if (valid)
         break;
   }

   return best_rt;
#endif
}

static enum mali_zs_format
translate_zs_format(enum pipe_format in)
{
   switch (in) {
   case PIPE_FORMAT_Z16_UNORM:
      return MALI_ZS_FORMAT_D16;
   case PIPE_FORMAT_Z24_UNORM_S8_UINT:
      return MALI_ZS_FORMAT_D24S8;
   case PIPE_FORMAT_Z24X8_UNORM:
      return MALI_ZS_FORMAT_D24X8;
   case PIPE_FORMAT_Z32_FLOAT:
      return MALI_ZS_FORMAT_D32;
#if PAN_ARCH <= 7
   case PIPE_FORMAT_Z32_FLOAT_S8X24_UINT:
      return MALI_ZS_FORMAT_D32_S8X24;
#endif
   default:
      unreachable("Unsupported depth/stencil format.");
   }
}

#if PAN_ARCH >= 5
static enum mali_s_format
translate_s_format(enum pipe_format in)
{
   switch (in) {
   case PIPE_FORMAT_S8_UINT:
      return MALI_S_FORMAT_S8;
   case PIPE_FORMAT_Z24_UNORM_S8_UINT:
   case PIPE_FORMAT_X24S8_UINT:
      return MALI_S_FORMAT_X24S8;

#if PAN_ARCH <= 7
   case PIPE_FORMAT_S8_UINT_Z24_UNORM:
   case PIPE_FORMAT_S8X24_UINT:
      return MALI_S_FORMAT_S8X24;
   case PIPE_FORMAT_Z32_FLOAT_S8X24_UINT:
      return MALI_S_FORMAT_X32_S8X24;
#endif

   default:
      unreachable("Unsupported stencil format.");
   }
}

static void
pan_prepare_s(const struct pan_fb_info *fb, struct MALI_ZS_CRC_EXTENSION *ext)
{
   const struct pan_image_view *s = fb->zs.view.s;

   if (!s)
      return;

   const struct pan_image *image = pan_image_view_get_zs_image(s);
   unsigned level = s->first_level;

   ext->s_msaa = mali_sampling_mode(s);

   struct pan_surface surf;
   pan_iview_get_surface(s, 0, 0, 0, &surf);

   assert(image->layout.modifier ==
             DRM_FORMAT_MOD_ARM_16X16_BLOCK_U_INTERLEAVED ||
          image->layout.modifier == DRM_FORMAT_MOD_LINEAR);
   ext->s_writeback_base = surf.data;
   ext->s_writeback_row_stride = image->layout.slices[level].row_stride;
   ext->s_writeback_surface_stride =
      (pan_image_view_get_nr_samples(s) > 1)
         ? image->layout.slices[level].surface_stride
         : 0;
   ext->s_block_format = mod_to_block_fmt(image->layout.modifier);
   ext->s_write_format = translate_s_format(s->format);
}

static void
pan_prepare_zs(const struct pan_fb_info *fb, struct MALI_ZS_CRC_EXTENSION *ext)
{
   const struct pan_image_view *zs = fb->zs.view.zs;

   if (!zs)
      return;

   const struct pan_image *image = pan_image_view_get_zs_image(zs);
   unsigned level = zs->first_level;

   ext->zs_msaa = mali_sampling_mode(zs);

   struct pan_surface surf;
   pan_iview_get_surface(zs, 0, 0, 0, &surf);
   UNUSED const struct pan_image_slice_layout *slice =
      &image->layout.slices[level];

   if (drm_is_afbc(image->layout.modifier)) {
#if PAN_ARCH >= 9
      ext->zs_writeback_base = surf.afbc.header;
      ext->zs_writeback_row_stride = slice->row_stride;
      /* TODO: surface stride? */
      ext->zs_afbc_body_offset = surf.afbc.body - surf.afbc.header;

      /* TODO: stencil AFBC? */
#else
#if PAN_ARCH >= 6
      ext->zs_afbc_row_stride =
         pan_afbc_stride_blocks(image->layout.modifier, slice->row_stride);
#else
      ext->zs_block_format = MALI_BLOCK_FORMAT_AFBC;
      ext->zs_afbc_body_size = 0x1000;
      ext->zs_afbc_chunk_size = 9;
      ext->zs_afbc_sparse = true;
#endif

      ext->zs_afbc_header = surf.afbc.header;
      ext->zs_afbc_body = surf.afbc.body;
#endif
   } else {
      assert(image->layout.modifier ==
                DRM_FORMAT_MOD_ARM_16X16_BLOCK_U_INTERLEAVED ||
             image->layout.modifier == DRM_FORMAT_MOD_LINEAR);

      /* TODO: Z32F(S8) support, which is always linear */

      ext->zs_writeback_base = surf.data;
      ext->zs_writeback_row_stride = image->layout.slices[level].row_stride;
      ext->zs_writeback_surface_stride =
         (pan_image_view_get_nr_samples(zs) > 1)
            ? image->layout.slices[level].surface_stride
            : 0;
   }

   ext->zs_block_format = mod_to_block_fmt(image->layout.modifier);
   ext->zs_write_format = translate_zs_format(zs->format);
   if (ext->zs_write_format == MALI_ZS_FORMAT_D24S8)
      ext->s_writeback_base = ext->zs_writeback_base;
}

static void
pan_prepare_crc(const struct pan_fb_info *fb, int rt_crc,
                struct MALI_ZS_CRC_EXTENSION *ext)
{
   if (rt_crc < 0)
      return;

   assert(rt_crc < fb->rt_count);

   const struct pan_image_view *rt = fb->rts[rt_crc].view;
   const struct pan_image *image = pan_image_view_get_rt_image(rt);
   const struct pan_image_slice_layout *slice =
      &image->layout.slices[rt->first_level];

   ext->crc_base =
      image->data.bo->ptr.gpu + image->data.offset + slice->crc.offset;
   ext->crc_row_stride = slice->crc.stride;

#if PAN_ARCH >= 7
   ext->crc_render_target = rt_crc;

   if (fb->rts[rt_crc].clear) {
      uint32_t clear_val = fb->rts[rt_crc].clear_value[0];
      ext->crc_clear_color = clear_val | 0xc000000000000000 |
                             (((uint64_t)clear_val & 0xffff) << 32);
   }
#endif
}

static void
pan_emit_zs_crc_ext(const struct pan_fb_info *fb, int rt_crc, void *zs_crc_ext)
{
   pan_pack(zs_crc_ext, ZS_CRC_EXTENSION, cfg) {
      pan_prepare_crc(fb, rt_crc, &cfg);
      cfg.zs_clean_pixel_write_enable = fb->zs.clear.z || fb->zs.clear.s;
      pan_prepare_zs(fb, &cfg);
      pan_prepare_s(fb, &cfg);
   }
}

/* Measure format as it appears in the tile buffer */

static unsigned
pan_bytes_per_pixel_tib(const struct panfrost_device *dev,
                        enum pipe_format format)
{
   if (dev->blendable_formats[format].internal) {
      /* Blendable formats are always 32-bits in the tile buffer,
       * extra bits are used as padding or to dither */
      return 4;
   } else {
      /* Non-blendable formats are raw, rounded up to the nearest
       * power-of-two size */
      unsigned bytes = util_format_get_blocksize(format);
      return util_next_power_of_two(bytes);
   }
}

static unsigned
pan_cbuf_bytes_per_pixel(const struct panfrost_device *dev,
                         const struct pan_fb_info *fb)
{
   unsigned sum = 0;

   for (int cb = 0; cb < fb->rt_count; ++cb) {
      const struct pan_image_view *rt = fb->rts[cb].view;

      if (!rt)
         continue;

      sum += pan_bytes_per_pixel_tib(dev, rt->format) * rt->nr_samples;
   }

   return sum;
}

/*
 * Select the largest tile size that fits within the tilebuffer budget.
 * Formally, maximize (pixels per tile) such that it is a power of two and
 *
 *      (bytes per pixel) (pixels per tile) <= (max bytes per tile)
 *
 * A bit of algebra gives the following formula.
 */
static unsigned
pan_select_max_tile_size(unsigned tile_buffer_bytes, unsigned bytes_per_pixel)
{
   assert(util_is_power_of_two_nonzero(tile_buffer_bytes));
   assert(tile_buffer_bytes >= 1024);

   return tile_buffer_bytes >> util_logbase2_ceil(bytes_per_pixel);
}

static enum mali_color_format
pan_mfbd_raw_format(unsigned bits)
{
   /* clang-format off */
   switch (bits) {
   case    8: return MALI_COLOR_FORMAT_RAW8;
   case   16: return MALI_COLOR_FORMAT_RAW16;
   case   24: return MALI_COLOR_FORMAT_RAW24;
   case   32: return MALI_COLOR_FORMAT_RAW32;
   case   48: return MALI_COLOR_FORMAT_RAW48;
   case   64: return MALI_COLOR_FORMAT_RAW64;
   case   96: return MALI_COLOR_FORMAT_RAW96;
   case  128: return MALI_COLOR_FORMAT_RAW128;
   case  192: return MALI_COLOR_FORMAT_RAW192;
   case  256: return MALI_COLOR_FORMAT_RAW256;
   case  384: return MALI_COLOR_FORMAT_RAW384;
   case  512: return MALI_COLOR_FORMAT_RAW512;
   case  768: return MALI_COLOR_FORMAT_RAW768;
   case 1024: return MALI_COLOR_FORMAT_RAW1024;
   case 1536: return MALI_COLOR_FORMAT_RAW1536;
   case 2048: return MALI_COLOR_FORMAT_RAW2048;
   default: unreachable("invalid raw bpp");
   }
   /* clang-format on */
}

static void
pan_rt_init_format(const struct panfrost_device *dev,
                   const struct pan_image_view *rt,
                   struct MALI_RENDER_TARGET *cfg)
{
   /* Explode details on the format */

   const struct util_format_description *desc =
      util_format_description(rt->format);

   /* The swizzle for rendering is inverted from texturing */

   unsigned char swizzle[4] = {
      PIPE_SWIZZLE_X,
      PIPE_SWIZZLE_Y,
      PIPE_SWIZZLE_Z,
      PIPE_SWIZZLE_W,
   };

   /* Fill in accordingly, defaulting to 8-bit UNORM */

   if (desc->colorspace == UTIL_FORMAT_COLORSPACE_SRGB)
      cfg->srgb = true;

   struct pan_blendable_format fmt = dev->blendable_formats[rt->format];

   if (fmt.internal) {
      cfg->internal_format = fmt.internal;
      cfg->writeback_format = fmt.writeback;
      panfrost_invert_swizzle(desc->swizzle, swizzle);
   } else {
      /* Construct RAW internal/writeback, where internal is
       * specified logarithmically (round to next power-of-two).
       * Offset specified from RAW8, where 8 = 2^3 */

      unsigned bits = desc->block.bits;
      unsigned offset = util_logbase2_ceil(bits) - 3;
      assert(offset <= 4);

      cfg->internal_format = MALI_COLOR_BUFFER_INTERNAL_FORMAT_RAW8 + offset;

      cfg->writeback_format = pan_mfbd_raw_format(bits);
   }

   cfg->swizzle = panfrost_translate_swizzle_4(swizzle);
}

static void
pan_prepare_rt(const struct panfrost_device *dev, const struct pan_fb_info *fb,
               unsigned idx, unsigned cbuf_offset,
               struct MALI_RENDER_TARGET *cfg)
{
   cfg->clean_pixel_write_enable = fb->rts[idx].clear;
   cfg->internal_buffer_offset = cbuf_offset;
   if (fb->rts[idx].clear) {
      cfg->clear.color_0 = fb->rts[idx].clear_value[0];
      cfg->clear.color_1 = fb->rts[idx].clear_value[1];
      cfg->clear.color_2 = fb->rts[idx].clear_value[2];
      cfg->clear.color_3 = fb->rts[idx].clear_value[3];
   }

   const struct pan_image_view *rt = fb->rts[idx].view;
   if (!rt || fb->rts[idx].discard) {
      cfg->internal_format = MALI_COLOR_BUFFER_INTERNAL_FORMAT_R8G8B8A8;
      cfg->internal_buffer_offset = cbuf_offset;
#if PAN_ARCH >= 7
      cfg->writeback_block_format = MALI_BLOCK_FORMAT_TILED_U_INTERLEAVED;
      cfg->dithering_enable = true;
#endif
      return;
   }

   const struct pan_image *image = pan_image_view_get_rt_image(rt);

   cfg->write_enable = true;
   cfg->dithering_enable = true;

   unsigned level = rt->first_level;
   assert(rt->last_level == rt->first_level);
   assert(rt->last_layer == rt->first_layer);

   int row_stride = image->layout.slices[level].row_stride;

   /* Only set layer_stride for layered MSAA rendering  */

   unsigned layer_stride = (pan_image_view_get_nr_samples(rt) > 1)
                              ? image->layout.slices[level].surface_stride
                              : 0;

   cfg->writeback_msaa = mali_sampling_mode(rt);

   pan_rt_init_format(dev, rt, cfg);

   cfg->writeback_block_format = mod_to_block_fmt(image->layout.modifier);

   struct pan_surface surf;
   pan_iview_get_surface(rt, 0, 0, 0, &surf);

   if (drm_is_afbc(image->layout.modifier)) {
#if PAN_ARCH >= 9
      if (image->layout.modifier & AFBC_FORMAT_MOD_YTR)
         cfg->afbc.yuv_transform = true;

      cfg->afbc.wide_block = panfrost_afbc_is_wide(image->layout.modifier);
      cfg->afbc.header = surf.afbc.header;
      cfg->afbc.body_offset = surf.afbc.body - surf.afbc.header;
      assert(surf.afbc.body >= surf.afbc.header);

      cfg->afbc.compression_mode = GENX(pan_afbc_compression_mode)(rt->format);
      cfg->afbc.row_stride = row_stride;
#else
      const struct pan_image_slice_layout *slice = &image->layout.slices[level];

#if PAN_ARCH >= 6
      cfg->afbc.row_stride =
         pan_afbc_stride_blocks(image->layout.modifier, slice->row_stride);
      cfg->afbc.afbc_wide_block_enable =
         panfrost_afbc_is_wide(image->layout.modifier);
#else
      cfg->afbc.chunk_size = 9;
      cfg->afbc.sparse = true;
      cfg->afbc.body_size = slice->afbc.body_size;
#endif

      cfg->afbc.header = surf.afbc.header;
      cfg->afbc.body = surf.afbc.body;

      if (image->layout.modifier & AFBC_FORMAT_MOD_YTR)
         cfg->afbc.yuv_transform_enable = true;
#endif
   } else {
      assert(image->layout.modifier == DRM_FORMAT_MOD_LINEAR ||
             image->layout.modifier ==
                DRM_FORMAT_MOD_ARM_16X16_BLOCK_U_INTERLEAVED);
      cfg->rgb.base = surf.data;
      cfg->rgb.row_stride = row_stride;
      cfg->rgb.surface_stride = layer_stride;
   }
}
#endif

void
GENX(pan_emit_tls)(const struct pan_tls_info *info, void *out)
{
   pan_pack(out, LOCAL_STORAGE, cfg) {
      if (info->tls.size) {
         unsigned shift = panfrost_get_stack_shift(info->tls.size);

         cfg.tls_size = shift;
#if PAN_ARCH >= 9
         /* For now, always use packed TLS addressing. This is
          * better for the cache and requires no fix up code in
          * the shader. We may need to revisit this someday for
          * OpenCL generic pointer support.
          */
         cfg.tls_address_mode = MALI_ADDRESS_MODE_PACKED;

         assert((info->tls.ptr & 4095) == 0);
         cfg.tls_base_pointer = info->tls.ptr >> 8;
#else
         cfg.tls_base_pointer = info->tls.ptr;
#endif
      }

      if (info->wls.size) {
         assert(!(info->wls.ptr & 4095));
         assert((info->wls.ptr & 0xffffffff00000000ULL) ==
                ((info->wls.ptr + info->wls.size - 1) & 0xffffffff00000000ULL));
         cfg.wls_base_pointer = info->wls.ptr;
         unsigned wls_size = pan_wls_adjust_size(info->wls.size);
         cfg.wls_instances = info->wls.instances;
         cfg.wls_size_scale = util_logbase2(wls_size) + 1;
      } else {
         cfg.wls_instances = MALI_LOCAL_STORAGE_NO_WORKGROUP_MEM;
      }
   }
}

#if PAN_ARCH <= 5
static void
pan_emit_midgard_tiler(const struct panfrost_device *dev,
                       const struct pan_fb_info *fb,
                       const struct pan_tiler_context *tiler_ctx, void *out)
{
   bool hierarchy = !dev->model->quirks.no_hierarchical_tiling;

   assert(tiler_ctx->midgard.polygon_list->ptr.gpu);

   pan_pack(out, TILER_CONTEXT, cfg) {
      unsigned header_size;

      if (tiler_ctx->midgard.disable) {
         cfg.hierarchy_mask =
            hierarchy ? MALI_MIDGARD_TILER_DISABLED : MALI_MIDGARD_TILER_USER;
         header_size = MALI_MIDGARD_TILER_MINIMUM_HEADER_SIZE;
         cfg.polygon_list_size = header_size + (hierarchy ? 0 : 4);
         cfg.heap_start = tiler_ctx->midgard.polygon_list->ptr.gpu;
         cfg.heap_end = tiler_ctx->midgard.polygon_list->ptr.gpu;
      } else {
         cfg.hierarchy_mask = panfrost_choose_hierarchy_mask(
            fb->width, fb->height, tiler_ctx->vertex_count, hierarchy);
         header_size = panfrost_tiler_header_size(
            fb->width, fb->height, cfg.hierarchy_mask, hierarchy);
         cfg.polygon_list_size = panfrost_tiler_full_size(
            fb->width, fb->height, cfg.hierarchy_mask, hierarchy);
         cfg.heap_start = dev->tiler_heap->ptr.gpu;
         cfg.heap_end =
            dev->tiler_heap->ptr.gpu + panfrost_bo_size(dev->tiler_heap);
      }

      cfg.polygon_list = tiler_ctx->midgard.polygon_list->ptr.gpu;
      cfg.polygon_list_body = cfg.polygon_list + header_size;
   }
}
#endif

#if PAN_ARCH >= 5
static void
pan_emit_rt(const struct panfrost_device *dev, const struct pan_fb_info *fb,
            unsigned idx, unsigned cbuf_offset, void *out)
{
   pan_pack(out, RENDER_TARGET, cfg) {
      pan_prepare_rt(dev, fb, idx, cbuf_offset, &cfg);
   }
}

#if PAN_ARCH >= 6
/* All Bifrost and Valhall GPUs are affected by issue TSIX-2033:
 *
 *      Forcing clean_tile_writes breaks INTERSECT readbacks
 *
 * To workaround, use the frame shader mode ALWAYS instead of INTERSECT if
 * clean tile writes is forced. Since INTERSECT is a hint that the hardware may
 * ignore, this cannot affect correctness, only performance */

static enum mali_pre_post_frame_shader_mode
pan_fix_frame_shader_mode(enum mali_pre_post_frame_shader_mode mode,
                          bool force_clean_tile)
{
   if (force_clean_tile && mode == MALI_PRE_POST_FRAME_SHADER_MODE_INTERSECT)
      return MALI_PRE_POST_FRAME_SHADER_MODE_ALWAYS;
   else
      return mode;
}

/* Regardless of clean_tile_write_enable, the hardware writes clean tiles if
 * the effective tile size differs from the superblock size of any enabled AFBC
 * render target. Check this condition. */

static bool
pan_force_clean_write_rt(const struct pan_image_view *rt, unsigned tile_size)
{
   const struct pan_image *image = pan_image_view_get_rt_image(rt);
   if (!drm_is_afbc(image->layout.modifier))
      return false;

   unsigned superblock = panfrost_afbc_superblock_width(image->layout.modifier);

   assert(superblock >= 16);
   assert(tile_size <= 16 * 16);

   /* Tile size and superblock differ unless they are both 16x16 */
   return !(superblock == 16 && tile_size == 16 * 16);
}

static bool
pan_force_clean_write(const struct pan_fb_info *fb, unsigned tile_size)
{
   /* Maximum tile size */
   assert(tile_size <= 16 * 16);

   for (unsigned i = 0; i < fb->rt_count; ++i) {
      if (fb->rts[i].view && !fb->rts[i].discard &&
          pan_force_clean_write_rt(fb->rts[i].view, tile_size))
         return true;
   }

   if (fb->zs.view.zs && !fb->zs.discard.z &&
       pan_force_clean_write_rt(fb->zs.view.zs, tile_size))
      return true;

   if (fb->zs.view.s && !fb->zs.discard.s &&
       pan_force_clean_write_rt(fb->zs.view.s, tile_size))
      return true;

   return false;
}

#endif

unsigned
GENX(pan_emit_fbd)(const struct panfrost_device *dev,
                   const struct pan_fb_info *fb, const struct pan_tls_info *tls,
                   const struct pan_tiler_context *tiler_ctx, void *out)
{
   void *fbd = out;
   void *rtd = out + pan_size(FRAMEBUFFER);

#if PAN_ARCH <= 5
   GENX(pan_emit_tls)(tls, pan_section_ptr(fbd, FRAMEBUFFER, LOCAL_STORAGE));
#endif

   unsigned bytes_per_pixel = pan_cbuf_bytes_per_pixel(dev, fb);
   unsigned tile_size =
      pan_select_max_tile_size(dev->optimal_tib_size, bytes_per_pixel);

   /* Clamp tile size to hardware limits */
   tile_size = MIN2(tile_size, 16 * 16);
   assert(tile_size >= 4 * 4);

   /* Colour buffer allocations must be 1K aligned. */
   unsigned cbuf_allocation = ALIGN_POT(bytes_per_pixel * tile_size, 1024);
   assert(cbuf_allocation <= dev->optimal_tib_size && "tile too big");

   int crc_rt = GENX(pan_select_crc_rt)(fb, tile_size);
   bool has_zs_crc_ext = (fb->zs.view.zs || fb->zs.view.s || crc_rt >= 0);

   pan_section_pack(fbd, FRAMEBUFFER, PARAMETERS, cfg) {
#if PAN_ARCH >= 6
      bool force_clean_write = pan_force_clean_write(fb, tile_size);

      cfg.sample_locations =
         panfrost_sample_positions(dev, pan_sample_pattern(fb->nr_samples));
      cfg.pre_frame_0 = pan_fix_frame_shader_mode(fb->bifrost.pre_post.modes[0],
                                                  force_clean_write);
      cfg.pre_frame_1 = pan_fix_frame_shader_mode(fb->bifrost.pre_post.modes[1],
                                                  force_clean_write);
      cfg.post_frame = pan_fix_frame_shader_mode(fb->bifrost.pre_post.modes[2],
                                                 force_clean_write);
      cfg.frame_shader_dcds = fb->bifrost.pre_post.dcds.gpu;
      cfg.tiler = tiler_ctx->bifrost;
#endif
      cfg.width = fb->width;
      cfg.height = fb->height;
      cfg.bound_max_x = fb->width - 1;
      cfg.bound_max_y = fb->height - 1;

      cfg.effective_tile_size = tile_size;
      cfg.tie_break_rule = MALI_TIE_BREAK_RULE_MINUS_180_IN_0_OUT;
      cfg.render_target_count = MAX2(fb->rt_count, 1);

      /* Default to 24 bit depth if there's no surface. */
      cfg.z_internal_format =
         fb->zs.view.zs ? panfrost_get_z_internal_format(fb->zs.view.zs->format)
                        : MALI_Z_INTERNAL_FORMAT_D24;

      cfg.z_clear = fb->zs.clear_value.depth;
      cfg.s_clear = fb->zs.clear_value.stencil;
      cfg.color_buffer_allocation = cbuf_allocation;
      cfg.sample_count = fb->nr_samples;
      cfg.sample_pattern = pan_sample_pattern(fb->nr_samples);
      cfg.z_write_enable = (fb->zs.view.zs && !fb->zs.discard.z);
      cfg.s_write_enable = (fb->zs.view.s && !fb->zs.discard.s);
      cfg.has_zs_crc_extension = has_zs_crc_ext;

      if (crc_rt >= 0) {
         bool *valid = fb->rts[crc_rt].crc_valid;
         bool full = !fb->extent.minx && !fb->extent.miny &&
                     fb->extent.maxx == (fb->width - 1) &&
                     fb->extent.maxy == (fb->height - 1);

         cfg.crc_read_enable = *valid;

         /* If the data is currently invalid, still write CRC
          * data if we are doing a full write, so that it is
          * valid for next time. */
         cfg.crc_write_enable = *valid || full;

         *valid |= full;
      }

#if PAN_ARCH >= 9
      cfg.point_sprite_coord_origin_max_y = fb->sprite_coord_origin;
      cfg.first_provoking_vertex = fb->first_provoking_vertex;
#endif
   }

#if PAN_ARCH >= 6
   pan_section_pack(fbd, FRAMEBUFFER, PADDING, padding)
      ;
#else
   pan_emit_midgard_tiler(dev, fb, tiler_ctx,
                          pan_section_ptr(fbd, FRAMEBUFFER, TILER));

   /* All weights set to 0, nothing to do here */
   pan_section_pack(fbd, FRAMEBUFFER, TILER_WEIGHTS, w)
      ;
#endif

   if (has_zs_crc_ext) {
      pan_emit_zs_crc_ext(fb, crc_rt, out + pan_size(FRAMEBUFFER));
      rtd += pan_size(ZS_CRC_EXTENSION);
   }

   unsigned rt_count = MAX2(fb->rt_count, 1);
   unsigned cbuf_offset = 0;
   for (unsigned i = 0; i < rt_count; i++) {
      pan_emit_rt(dev, fb, i, cbuf_offset, rtd);
      rtd += pan_size(RENDER_TARGET);
      if (!fb->rts[i].view)
         continue;

      cbuf_offset += pan_bytes_per_pixel_tib(dev, fb->rts[i].view->format) *
                     tile_size * pan_image_view_get_nr_samples(fb->rts[i].view);

      if (i != crc_rt)
         *(fb->rts[i].crc_valid) = false;
   }

   struct mali_framebuffer_pointer_packed tag;
   pan_pack(tag.opaque, FRAMEBUFFER_POINTER, cfg) {
      cfg.zs_crc_extension_present = has_zs_crc_ext;
      cfg.render_target_count = MAX2(fb->rt_count, 1);
   }
   return tag.opaque[0];
}
#else /* PAN_ARCH == 4 */
unsigned
GENX(pan_emit_fbd)(const struct panfrost_device *dev,
                   const struct pan_fb_info *fb, const struct pan_tls_info *tls,
                   const struct pan_tiler_context *tiler_ctx, void *fbd)
{
   assert(fb->rt_count <= 1);

   GENX(pan_emit_tls)(tls, pan_section_ptr(fbd, FRAMEBUFFER, LOCAL_STORAGE));
   pan_section_pack(fbd, FRAMEBUFFER, PARAMETERS, cfg) {
      cfg.bound_max_x = fb->width - 1;
      cfg.bound_max_y = fb->height - 1;
      cfg.dithering_enable = true;
      cfg.clean_pixel_write_enable = true;
      cfg.tie_break_rule = MALI_TIE_BREAK_RULE_MINUS_180_IN_0_OUT;
      if (fb->rts[0].clear) {
         cfg.clear_color_0 = fb->rts[0].clear_value[0];
         cfg.clear_color_1 = fb->rts[0].clear_value[1];
         cfg.clear_color_2 = fb->rts[0].clear_value[2];
         cfg.clear_color_3 = fb->rts[0].clear_value[3];
      }

      if (fb->zs.clear.z)
         cfg.z_clear = fb->zs.clear_value.depth;

      if (fb->zs.clear.s)
         cfg.s_clear = fb->zs.clear_value.stencil;

      if (fb->rt_count && fb->rts[0].view) {
         const struct pan_image_view *rt = fb->rts[0].view;
         const struct pan_image *image = pan_image_view_get_rt_image(rt);

         const struct util_format_description *desc =
            util_format_description(rt->format);

         /* The swizzle for rendering is inverted from texturing */
         unsigned char swizzle[4];
         panfrost_invert_swizzle(desc->swizzle, swizzle);
         cfg.swizzle = panfrost_translate_swizzle_4(swizzle);

         struct pan_blendable_format fmt = dev->blendable_formats[rt->format];
         if (fmt.internal) {
            cfg.internal_format = fmt.internal;
            cfg.color_writeback_format = fmt.writeback;
         } else {
            unreachable("raw formats not finished for SFBD");
         }

         unsigned level = rt->first_level;
         struct pan_surface surf;

         pan_iview_get_surface(rt, 0, 0, 0, &surf);

         cfg.color_write_enable = !fb->rts[0].discard;
         cfg.color_writeback.base = surf.data;
         cfg.color_writeback.row_stride =
            image->layout.slices[level].row_stride;

         cfg.color_block_format = mod_to_block_fmt(image->layout.modifier);
         assert(cfg.color_block_format == MALI_BLOCK_FORMAT_LINEAR ||
                cfg.color_block_format ==
                   MALI_BLOCK_FORMAT_TILED_U_INTERLEAVED);

         if (pan_image_view_has_crc(rt)) {
            const struct pan_image_slice_layout *slice =
               &image->layout.slices[level];

            cfg.crc_buffer.row_stride = slice->crc.stride;
            cfg.crc_buffer.base =
               image->data.bo->ptr.gpu + image->data.offset + slice->crc.offset;
         }
      }

      if (fb->zs.view.zs) {
         const struct pan_image_view *zs = fb->zs.view.zs;
         const struct pan_image *image = pan_image_view_get_zs_image(zs);
         unsigned level = zs->first_level;
         struct pan_surface surf;

         pan_iview_get_surface(zs, 0, 0, 0, &surf);

         cfg.zs_write_enable = !fb->zs.discard.z;
         cfg.zs_writeback.base = surf.data;
         cfg.zs_writeback.row_stride = image->layout.slices[level].row_stride;
         cfg.zs_block_format = mod_to_block_fmt(image->layout.modifier);
         assert(cfg.zs_block_format == MALI_BLOCK_FORMAT_LINEAR ||
                cfg.zs_block_format == MALI_BLOCK_FORMAT_TILED_U_INTERLEAVED);

         cfg.zs_format = translate_zs_format(zs->format);
      }

      cfg.sample_count = fb->nr_samples;

      if (fb->rt_count)
         cfg.msaa = mali_sampling_mode(fb->rts[0].view);
   }

   pan_emit_midgard_tiler(dev, fb, tiler_ctx,
                          pan_section_ptr(fbd, FRAMEBUFFER, TILER));

   /* All weights set to 0, nothing to do here */
   pan_section_pack(fbd, FRAMEBUFFER, TILER_WEIGHTS, w)
      ;

   pan_section_pack(fbd, FRAMEBUFFER, PADDING_1, padding)
      ;
   pan_section_pack(fbd, FRAMEBUFFER, PADDING_2, padding)
      ;
   return 0;
}
#endif

#if PAN_ARCH <= 9
void
GENX(pan_emit_fragment_job)(const struct pan_fb_info *fb, mali_ptr fbd,
                            void *out)
{
   pan_section_pack(out, FRAGMENT_JOB, HEADER, header) {
      header.type = MALI_JOB_TYPE_FRAGMENT;
      header.index = 1;
   }

   pan_section_pack(out, FRAGMENT_JOB, PAYLOAD, payload) {
      payload.bound_min_x = fb->extent.minx >> MALI_TILE_SHIFT;
      payload.bound_min_y = fb->extent.miny >> MALI_TILE_SHIFT;
      payload.bound_max_x = fb->extent.maxx >> MALI_TILE_SHIFT;
      payload.bound_max_y = fb->extent.maxy >> MALI_TILE_SHIFT;
      payload.framebuffer = fbd;

#if PAN_ARCH >= 5
      if (fb->tile_map.base) {
         payload.has_tile_enable_map = true;
         payload.tile_enable_map = fb->tile_map.base;
         payload.tile_enable_map_row_stride = fb->tile_map.stride;
      }
#endif
   }
}
#endif
