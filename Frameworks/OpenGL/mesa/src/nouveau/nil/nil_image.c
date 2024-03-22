/*
 * Copyright © 2022 Collabora Ltd.
 * SPDX-License-Identifier: MIT
 */
#include "nil_image.h"

#include "util/u_math.h"

#include "nouveau_device.h"

#include "cl9097.h"
#include "clc597.h"

static struct nil_extent4d
nil_minify_extent4d(struct nil_extent4d extent, uint32_t level)
{
   return (struct nil_extent4d) {
      .w = u_minify(extent.w, level),
      .h = u_minify(extent.h, level),
      .d = u_minify(extent.d, level),
      .a = extent.a,
   };
}

static struct nil_extent4d
nil_extent4d_div_round_up(struct nil_extent4d num, struct nil_extent4d denom)
{
   return (struct nil_extent4d) {
      .w = DIV_ROUND_UP(num.w, denom.w),
      .h = DIV_ROUND_UP(num.h, denom.h),
      .d = DIV_ROUND_UP(num.d, denom.d),
      .a = DIV_ROUND_UP(num.a, denom.a),
   };
}

static struct nil_extent4d
nil_extent4d_mul(struct nil_extent4d a, struct nil_extent4d b)
{
   return (struct nil_extent4d) {
      .w = a.w * b.w,
      .h = a.h * b.h,
      .d = a.d * b.d,
      .a = a.a * b.a,
   };
}

static struct nil_offset4d
nil_offset4d_div_round_down(struct nil_offset4d num, struct nil_extent4d denom)
{
   return (struct nil_offset4d) {
      .x = num.x / denom.w,
      .y = num.y / denom.h,
      .z = num.z / denom.d,
      .a = num.a / denom.a,
   };
}

static struct nil_offset4d
nil_offset4d_mul(struct nil_offset4d a, struct nil_extent4d b)
{
   return (struct nil_offset4d) {
      .x = a.x * b.w,
      .y = a.y * b.h,
      .z = a.z * b.d,
      .a = a.a * b.a,
   };
}

static struct nil_extent4d
nil_extent4d_align(struct nil_extent4d ext, struct nil_extent4d alignment)
{
   return (struct nil_extent4d) {
      .w = align(ext.w, alignment.w),
      .h = align(ext.h, alignment.h),
      .d = align(ext.d, alignment.d),
      .a = align(ext.a, alignment.a),
   };
}

static inline struct nil_extent4d
nil_px_extent_sa(enum nil_sample_layout sample_layout)
{
   switch (sample_layout) {
   case NIL_SAMPLE_LAYOUT_1X1: return nil_extent4d(1, 1, 1, 1);
   case NIL_SAMPLE_LAYOUT_2X1: return nil_extent4d(2, 1, 1, 1);
   case NIL_SAMPLE_LAYOUT_2X2: return nil_extent4d(2, 2, 1, 1);
   case NIL_SAMPLE_LAYOUT_4X2: return nil_extent4d(4, 2, 1, 1);
   case NIL_SAMPLE_LAYOUT_4X4: return nil_extent4d(4, 4, 1, 1);
   default: unreachable("Invalid sample layout");
   }
}

static inline struct nil_extent4d
nil_el_extent_sa(enum pipe_format format)
{
   const struct util_format_description *fmt =
      util_format_description(format);

   return (struct nil_extent4d) {
      .w = fmt->block.width,
      .h = fmt->block.height,
      .d = fmt->block.depth,
      .a = 1,
   };
}

static struct nil_extent4d
nil_extent4d_px_to_sa(struct nil_extent4d extent_px,
                      enum nil_sample_layout sample_layout)
{
   return nil_extent4d_mul(extent_px, nil_px_extent_sa(sample_layout));
}

struct nil_extent4d
nil_extent4d_px_to_el(struct nil_extent4d extent_px,
                      enum pipe_format format,
                      enum nil_sample_layout sample_layout)
{
   const struct nil_extent4d extent_sa =
      nil_extent4d_px_to_sa(extent_px, sample_layout);

   return nil_extent4d_div_round_up(extent_sa, nil_el_extent_sa(format));
}

struct nil_offset4d
nil_offset4d_px_to_el(struct nil_offset4d offset_px,
                      enum pipe_format format,
                      enum nil_sample_layout sample_layout)
{
   const struct nil_offset4d offset_sa =
      nil_offset4d_mul(offset_px, nil_px_extent_sa(sample_layout));

   return nil_offset4d_div_round_down(offset_sa, nil_el_extent_sa(format));
}

static struct nil_extent4d
nil_extent4d_el_to_B(struct nil_extent4d extent_el,
                     uint32_t B_per_el)
{
   struct nil_extent4d extent_B = extent_el;
   extent_B.w *= B_per_el;
   return extent_B;
}

static struct nil_extent4d
nil_extent4d_B_to_GOB(struct nil_extent4d extent_B,
                      bool gob_height_8)
{
   const struct nil_extent4d gob_extent_B = {
      .w = NIL_GOB_WIDTH_B,
      .h = NIL_GOB_HEIGHT(gob_height_8),
      .d = NIL_GOB_DEPTH,
      .a = 1,
   };

   return nil_extent4d_div_round_up(extent_B, gob_extent_B);
}

static struct nil_extent4d
nil_tiling_extent_B(struct nil_tiling tiling)
{
   if (tiling.is_tiled) {
      return (struct nil_extent4d) {
         .w = NIL_GOB_WIDTH_B, /* Tiles are always 1 GOB wide */
         .h = NIL_GOB_HEIGHT(tiling.gob_height_8) << tiling.y_log2,
         .d = NIL_GOB_DEPTH << tiling.z_log2,
         .a = 1,
      };
   } else {
      /* We handle linear images in nil_image_create */
      return nil_extent4d(1, 1, 1, 1);
   }
}

enum nil_sample_layout
nil_choose_sample_layout(uint32_t samples)
{
   switch (samples) {
   case 1:  return NIL_SAMPLE_LAYOUT_1X1;
   case 2:  return NIL_SAMPLE_LAYOUT_2X1;
   case 4:  return NIL_SAMPLE_LAYOUT_2X2;
   case 8:  return NIL_SAMPLE_LAYOUT_4X2;
   case 16: return NIL_SAMPLE_LAYOUT_4X4;
   default:
      unreachable("Unsupported sample count");
   }
}

static struct nil_tiling
choose_tiling(struct nil_extent4d extent_B,
              enum nil_image_usage_flags usage)
{
   if (usage & NIL_IMAGE_USAGE_LINEAR_BIT)
      return (struct nil_tiling) { .is_tiled = false };

   struct nil_tiling tiling = {
      .is_tiled = true,
      .gob_height_8 = true,
   };

   const struct nil_extent4d extent_GOB =
      nil_extent4d_B_to_GOB(extent_B, tiling.gob_height_8);

   const uint32_t height_log2 = util_logbase2_ceil(extent_GOB.height);
   const uint32_t depth_log2 = util_logbase2_ceil(extent_GOB.depth);

   tiling.y_log2 = MIN2(height_log2, 5);
   tiling.z_log2 = MIN2(depth_log2, 5);

   if (usage & NIL_IMAGE_USAGE_2D_VIEW_BIT)
      tiling.z_log2 = 0;

   return tiling;
}

static uint32_t
nil_tiling_size_B(struct nil_tiling tiling)
{
   const struct nil_extent4d extent_B = nil_tiling_extent_B(tiling);
   return extent_B.w * extent_B.h * extent_B.d * extent_B.a;
}

static struct nil_extent4d
nil_extent4d_B_to_tl(struct nil_extent4d extent_B,
                     struct nil_tiling tiling)
{
   return nil_extent4d_div_round_up(extent_B, nil_tiling_extent_B(tiling));
}

struct nil_extent4d
nil_image_level_extent_px(const struct nil_image *image, uint32_t level)
{
   assert(level == 0 || image->sample_layout == NIL_SAMPLE_LAYOUT_1X1);

   return nil_minify_extent4d(image->extent_px, level);
}

struct nil_extent4d
nil_image_level_extent_sa(const struct nil_image *image, uint32_t level)
{
   const struct nil_extent4d level_extent_px =
      nil_image_level_extent_px(image, level);

   return nil_extent4d_px_to_sa(level_extent_px, image->sample_layout);
}

static struct nil_extent4d
image_level_extent_B(const struct nil_image *image, uint32_t level)
{
   const struct nil_extent4d level_extent_px =
      nil_image_level_extent_px(image, level);
   const struct nil_extent4d level_extent_el =
      nil_extent4d_px_to_el(level_extent_px, image->format,
                            image->sample_layout);
   const uint32_t B_per_el = util_format_get_blocksize(image->format);
   return nil_extent4d_el_to_B(level_extent_el, B_per_el);
}

static uint8_t
tu102_choose_pte_kind(enum pipe_format format, bool compressed)
{
   switch (format) {
   case PIPE_FORMAT_Z16_UNORM:
      if (compressed)
         return 0x0b; // NV_MMU_PTE_KIND_Z16_COMPRESSIBLE_DISABLE_PLC
      else
         return 0x01; // NV_MMU_PTE_KIND_Z16
   case PIPE_FORMAT_X8Z24_UNORM:
   case PIPE_FORMAT_S8X24_UINT:
   case PIPE_FORMAT_S8_UINT_Z24_UNORM:
      if (compressed)
         return 0x0e; // NV_MMU_PTE_KIND_Z24S8_COMPRESSIBLE_DISABLE_PLC
      else
         return 0x05; // NV_MMU_PTE_KIND_Z24S8
   case PIPE_FORMAT_X24S8_UINT:
   case PIPE_FORMAT_Z24X8_UNORM:
   case PIPE_FORMAT_Z24_UNORM_S8_UINT:
      if (compressed)
         return 0x0c; // NV_MMU_PTE_KIND_S8Z24_COMPRESSIBLE_DISABLE_PLC
      else
         return 0x03; // NV_MMU_PTE_KIND_S8Z24
   case PIPE_FORMAT_X32_S8X24_UINT:
   case PIPE_FORMAT_Z32_FLOAT_S8X24_UINT:
      if (compressed)
         return 0x0d; // NV_MMU_PTE_KIND_ZF32_X24S8_COMPRESSIBLE_DISABLE_PLC
      else
         return 0x04; // NV_MMU_PTE_KIND_ZF32_X24S8
   case PIPE_FORMAT_Z32_FLOAT:
      return 0x06;
   default:
      return 0;
   }
}

static uint8_t
nvc0_choose_pte_kind(enum pipe_format format,
                     uint32_t samples, bool compressed)
{
   const unsigned ms = util_logbase2(samples);

   switch (format) {
   case PIPE_FORMAT_Z16_UNORM:
      if (compressed)
         return 0x02 + ms;
      else
         return 0x01;
   case PIPE_FORMAT_X8Z24_UNORM:
   case PIPE_FORMAT_S8X24_UINT:
   case PIPE_FORMAT_S8_UINT_Z24_UNORM:
      if (compressed)
         return 0x51 + ms;
      else
         return 0x46;
   case PIPE_FORMAT_X24S8_UINT:
   case PIPE_FORMAT_Z24X8_UNORM:
   case PIPE_FORMAT_Z24_UNORM_S8_UINT:
      if (compressed)
         return 0x17 + ms;
      else
         return 0x11;
      break;
   case PIPE_FORMAT_Z32_FLOAT:
      if (compressed)
         return 0x86 + ms;
      else
         return 0x7b;
      break;
   case PIPE_FORMAT_X32_S8X24_UINT:
   case PIPE_FORMAT_Z32_FLOAT_S8X24_UINT:
      if (compressed)
         return 0xce + ms;
      else
         return 0xc3;
   default:
      switch (util_format_get_blocksizebits(format)) {
      case 128:
         if (compressed)
            return 0xf4 + ms * 2;
         else
            return 0xfe;
         break;
      case 64:
         if (compressed) {
            switch (samples) {
            case 1:  return 0xe6;
            case 2:  return 0xeb;
            case 4:  return 0xed;
            case 8:  return 0xf2;
            default: return 0;
            }
         } else {
            return 0xfe;
         }
         break;
      case 32:
         if (compressed && ms) {
            switch (samples) {
               /* This one makes things blurry:
            case 1:  return 0xdb;
               */
            case 2:  return 0xdd;
            case 4:  return 0xdf;
            case 8:  return 0xe4;
            default: return 0;
            }
         } else {
            return 0xfe;
         }
         break;
      case 16:
      case 8:
         return 0xfe;
      default:
         return 0;
      }
   }
}

static uint8_t
nil_choose_pte_kind(struct nv_device_info *dev,
                    enum pipe_format format,
                    uint32_t samples, bool compressed)
{
   if (dev->cls_eng3d >= TURING_A)
      return tu102_choose_pte_kind(format, compressed);
   else if (dev->cls_eng3d >= FERMI_A)
      return nvc0_choose_pte_kind(format, samples, compressed);
   else
      unreachable("Unsupported 3D engine class");
}

bool
nil_image_init(struct nv_device_info *dev,
               struct nil_image *image,
               const struct nil_image_init_info *restrict info)
{
   switch (info->dim) {
   case NIL_IMAGE_DIM_1D:
      assert(info->extent_px.h == 1);
      assert(info->extent_px.d == 1);
      assert(info->samples == 1);
      break;
   case NIL_IMAGE_DIM_2D:
      assert(info->extent_px.d == 1);
      break;
   case NIL_IMAGE_DIM_3D:
      assert(info->extent_px.a == 1);
      assert(info->samples == 1);
      break;
   }

   *image = (struct nil_image) {
      .dim = info->dim,
      .format = info->format,
      .extent_px = info->extent_px,
      .sample_layout = nil_choose_sample_layout(info->samples),
      .num_levels = info->levels,
   };

   uint64_t layer_size_B = 0;
   for (uint32_t l = 0; l < info->levels; l++) {
      struct nil_extent4d lvl_ext_B = image_level_extent_B(image, l);

      /* Tiling is chosen per-level with LOD0 acting as a maximum */
      struct nil_tiling lvl_tiling = choose_tiling(lvl_ext_B, info->usage);

      if (lvl_tiling.is_tiled) {
         /* Align the size to tiles */
         struct nil_extent4d lvl_tiling_ext_B = nil_tiling_extent_B(lvl_tiling);
         lvl_ext_B = nil_extent4d_align(lvl_ext_B, lvl_tiling_ext_B);

         image->levels[l] = (struct nil_image_level) {
            .offset_B = layer_size_B,
            .tiling = lvl_tiling,
            .row_stride_B = lvl_ext_B.width,
         };

         layer_size_B += (uint64_t)lvl_ext_B.w *
                         (uint64_t)lvl_ext_B.h *
                         (uint64_t)lvl_ext_B.d;
      } else {
         /* Linear images need to be 2D */
         assert(image->dim == NIL_IMAGE_DIM_2D);
         /* NVIDIA can't do linear and mipmapping */
         assert(image->num_levels == 1);
         /* NVIDIA can't do linear and multisampling*/
         assert(image->sample_layout == NIL_SAMPLE_LAYOUT_1X1);

         image->levels[l] = (struct nil_image_level) {
            .offset_B = layer_size_B,
            .tiling = lvl_tiling,
            /* Row stride needs to be aligned to 128B for render to work */
            .row_stride_B = align(lvl_ext_B.width, 128),
         };

         assert(lvl_ext_B.d == 1);
         layer_size_B += (uint64_t)image->levels[l].row_stride_B * 
                         (uint64_t)lvl_ext_B.h;

      }
   }

   /* Align the image and array stride to a single level0 tile */
   image->align_B = nil_tiling_size_B(image->levels[0].tiling);

   /* I have no idea why but hardware seems to align layer strides */
   image->array_stride_B = ALIGN(layer_size_B, image->align_B);

   image->size_B = (uint64_t)image->array_stride_B * image->extent_px.a;

   if (image->levels[0].tiling.is_tiled) {
      image->tile_mode = (uint16_t)image->levels[0].tiling.y_log2 << 4 |
                         (uint16_t)image->levels[0].tiling.z_log2 << 8;

      image->pte_kind = nil_choose_pte_kind(dev, info->format, info->samples,
                                            false /* TODO: compressed */);

      image->align_B = MAX2(image->align_B, 4096);
      if (image->pte_kind >= 0xb && image->pte_kind <= 0xe)
         image->align_B = MAX2(image->align_B, (1 << 16));
   } else {
      /* Linear images need to be aligned to 128B for render to work */
      image->align_B = MAX2(image->align_B, 128);
   }

   image->size_B = align64(image->size_B, image->align_B);
   return true;
}

uint64_t
nil_image_level_size_B(const struct nil_image *image, uint32_t level)
{
   assert(level < image->num_levels);

   /* See the nil_image::levels[] computations */
   struct nil_extent4d lvl_ext_B = image_level_extent_B(image, level);
   struct nil_extent4d lvl_tiling_ext_B =
      nil_tiling_extent_B(image->levels[level].tiling);
   lvl_ext_B = nil_extent4d_align(lvl_ext_B, lvl_tiling_ext_B);

   return (uint64_t)lvl_ext_B.w *
          (uint64_t)lvl_ext_B.h *
          (uint64_t)lvl_ext_B.d;
}

uint64_t
nil_image_level_depth_stride_B(const struct nil_image *image, uint32_t level)
{
   assert(level < image->num_levels);

   /* See the nil_image::levels[] computations */
   struct nil_extent4d lvl_ext_B = image_level_extent_B(image, level);
   struct nil_extent4d lvl_tiling_ext_B =
      nil_tiling_extent_B(image->levels[level].tiling);
   lvl_ext_B = nil_extent4d_align(lvl_ext_B, lvl_tiling_ext_B);

   return (uint64_t)lvl_ext_B.w * (uint64_t)lvl_ext_B.h;
}

void
nil_image_for_level(const struct nil_image *image_in,
                    uint32_t level,
                    struct nil_image *lvl_image_out,
                    uint64_t *offset_B_out)
{
   assert(level < image_in->num_levels);

   const struct nil_extent4d lvl_extent_px =
      nil_image_level_extent_px(image_in, level);
   const struct nil_image_level lvl = image_in->levels[level];
   const uint32_t align_B = nil_tiling_size_B(lvl.tiling);

   uint64_t size_B = image_in->size_B - lvl.offset_B;
   if (level + 1 < image_in->num_levels) {
      /* This assumes levels are sequential, tightly packed, and that each
       * level has a higher alignment than the next one.  All of this is
       * currently true
       */
      const uint64_t next_lvl_offset_B = image_in->levels[level + 1].offset_B;
      assert(next_lvl_offset_B > lvl.offset_B);
      size_B -= next_lvl_offset_B - lvl.offset_B;
   }

   *offset_B_out = lvl.offset_B;
   *lvl_image_out = (struct nil_image) {
      .dim = image_in->dim,
      .format = image_in->format,
      .extent_px = lvl_extent_px,
      .sample_layout = image_in->sample_layout,
      .num_levels = 1,
      .levels[0] = lvl,
      .array_stride_B = image_in->array_stride_B,
      .align_B = align_B,
      .size_B = size_B,
      .tile_mode = image_in->tile_mode,
      .pte_kind = image_in->pte_kind,
   };
}

static enum pipe_format
pipe_format_for_bits(uint32_t bits)
{
   switch (bits) {
   case 32:    return PIPE_FORMAT_R32_UINT;
   case 64:    return PIPE_FORMAT_R32G32_UINT;
   case 128:   return PIPE_FORMAT_R32G32B32A32_UINT;
   default:
      unreachable("No PIPE_FORMAT with this size");
   }
}

void
nil_image_level_as_uncompressed(const struct nil_image *image_in,
                                uint32_t level,
                                struct nil_image *uc_image_out,
                                uint64_t *offset_B_out)
{
   assert(image_in->sample_layout == NIL_SAMPLE_LAYOUT_1X1);

   /* Format is arbitrary. Pick one that has the right number of bits. */
   const enum pipe_format uc_format =
      pipe_format_for_bits(util_format_get_blocksizebits(image_in->format));

   struct nil_image lvl_image;
   nil_image_for_level(image_in, level, &lvl_image, offset_B_out);

   *uc_image_out = lvl_image;
   uc_image_out->format = uc_format;
   uc_image_out->extent_px =
      nil_extent4d_px_to_el(lvl_image.extent_px, lvl_image.format,
                            lvl_image.sample_layout);
}

void
nil_image_3d_level_as_2d_array(const struct nil_image *image_3d,
                               uint32_t level,
                               struct nil_image *image_2d_out,
                               uint64_t *offset_B_out)
{
   assert(image_3d->dim == NIL_IMAGE_DIM_3D);
   assert(image_3d->extent_px.array_len == 1);
   assert(image_3d->sample_layout == NIL_SAMPLE_LAYOUT_1X1);

   struct nil_image lvl_image;
   nil_image_for_level(image_3d, level, &lvl_image, offset_B_out);

   assert(lvl_image.num_levels == 1);
   assert(!lvl_image.levels[0].tiling.is_tiled ||
          lvl_image.levels[0].tiling.z_log2 == 0);

   struct nil_extent4d lvl_tiling_ext_B =
      nil_tiling_extent_B(lvl_image.levels[0].tiling);
   struct nil_extent4d lvl_ext_B = image_level_extent_B(&lvl_image, 0);
   lvl_ext_B = nil_extent4d_align(lvl_ext_B, lvl_tiling_ext_B);
   uint64_t z_stride = (uint64_t)lvl_ext_B.w * (uint64_t)lvl_ext_B.h;

   *image_2d_out = lvl_image;
   image_2d_out->dim = NIL_IMAGE_DIM_2D;
   image_2d_out->extent_px.d = 1;
   image_2d_out->extent_px.a = lvl_image.extent_px.d;
   image_2d_out->array_stride_B = z_stride;
}
