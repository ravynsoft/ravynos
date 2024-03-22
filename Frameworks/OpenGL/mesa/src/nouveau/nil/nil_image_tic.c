/*
 * Copyright © 2022 Collabora Ltd.
 * SPDX-License-Identifier: MIT
 */
#include "nil_image.h"

#include "nil_format.h"
#include "util/bitpack_helpers.h"

#include "nouveau_device.h"

#include "cl9097.h"
#include "cl9097tex.h"
#include "clb097.h"
#include "clb097tex.h"
#include "drf.h"

ALWAYS_INLINE static void
__set_u32(uint32_t *o, uint32_t v, unsigned lo, unsigned hi)
{
   assert(lo <= hi && (lo / 32) == (hi / 32));
   o[lo / 32] |= util_bitpack_uint(v, lo % 32, hi % 32);
}

#define FIXED_FRAC_BITS 8

ALWAYS_INLINE static void
__set_ufixed(uint32_t *o, float v, unsigned lo, unsigned hi)
{
   assert(lo <= hi && (lo / 32) == (hi / 32));
   o[lo / 32] |= util_bitpack_ufixed_clamp(v, lo % 32, hi % 32,
                                           FIXED_FRAC_BITS);
}

ALWAYS_INLINE static void
__set_i32(uint32_t *o, int32_t v, unsigned lo, unsigned hi)
{
   assert(lo <= hi && (lo / 32) == (hi / 32));
   o[lo / 32] |= util_bitpack_sint(v, lo % 32, hi % 32);
}

ALWAYS_INLINE static void
__set_bool(uint32_t *o, bool b, unsigned lo, unsigned hi)
{
   assert(lo == hi);
   o[lo / 32] |= util_bitpack_uint(b, lo % 32, hi % 32);
}

#define MW(x) x

#define TH_SET_U(o, NV, VER, FIELD, val) \
   __set_u32((o), (val), DRF_LO(NV##_TEXHEAD##VER##_##FIELD),\
                         DRF_HI(NV##_TEXHEAD##VER##_##FIELD))

#define TH_SET_UF(o, NV, VER, FIELD, val) \
   __set_ufixed((o), (val), DRF_LO(NV##_TEXHEAD##VER##_##FIELD),\
                            DRF_HI(NV##_TEXHEAD##VER##_##FIELD))

#define TH_SET_I(o, NV, VER, FIELD, val) \
   __set_i32((o), (val), DRF_LO(NV##_TEXHEAD##VER##_##FIELD),\
                         DRF_HI(NV##_TEXHEAD##VER##_##FIELD))

#define TH_SET_B(o, NV, VER, FIELD, b) \
   __set_bool((o), (b), DRF_LO(NV##_TEXHEAD##VER##_##FIELD),\
                        DRF_HI(NV##_TEXHEAD##VER##_##FIELD))

#define TH_SET_E(o, NV, VER, FIELD, E) \
   TH_SET_U((o), NV, VER, FIELD, NV##_TEXHEAD##VER##_##FIELD##_##E)

#define TH_NV9097_SET_U(o, IDX, FIELD, val) \
   TH_SET_U(&(o)[IDX], NV9097, V2_##IDX, FIELD, (val));
#define TH_NV9097_SET_UF(o, IDX, FIELD, val) \
   TH_SET_UF(&(o)[IDX], NV9097, V2_##IDX, FIELD, (val));
#define TH_NV9097_SET_I(o, IDX, FIELD, val) \
   TH_SET_I(&(o)[IDX], NV9097, V2_##IDX, FIELD, (val));
#define TH_NV9097_SET_B(o, IDX, FIELD, b) \
   TH_SET_B(&(o)[IDX], NV9097, V2_##IDX, FIELD, (b));
#define TH_NV9097_SET_E(o, IDX, FIELD, E) \
   TH_SET_E(&(o)[IDX], NV9097, V2_##IDX, FIELD, E);

#define TH_NVB097_SET_U(o, VER, FIELD, val) \
   TH_SET_U((o), NVB097, _##VER, FIELD, (val));
#define TH_NVB097_SET_UF(o, VER, FIELD, val) \
   TH_SET_UF((o), NVB097, _##VER, FIELD, (val));
#define TH_NVB097_SET_I(o, VER, FIELD, val) \
   TH_SET_I((o), NVB097, _##VER, FIELD, (val));
#define TH_NVB097_SET_B(o, VER, FIELD, b) \
   TH_SET_B((o), NVB097, _##VER, FIELD, (b));
#define TH_NVB097_SET_E(o, VER, FIELD, E) \
   TH_SET_E((o), NVB097, _##VER, FIELD, E);

static inline uint32_t
nv9097_th_bl_source(const struct nil_tic_format *fmt,
                    enum pipe_swizzle swz, bool is_int)
{
   switch (swz) {
   case PIPE_SWIZZLE_X: return fmt->src_x;
   case PIPE_SWIZZLE_Y: return fmt->src_y;
   case PIPE_SWIZZLE_Z: return fmt->src_z;
   case PIPE_SWIZZLE_W: return fmt->src_w;
   case PIPE_SWIZZLE_0:
      return NV9097_TEXHEADV2_0_X_SOURCE_IN_ZERO;
   case PIPE_SWIZZLE_1:
      return is_int ? NV9097_TEXHEADV2_0_X_SOURCE_IN_ONE_INT :
                      NV9097_TEXHEADV2_0_X_SOURCE_IN_ONE_FLOAT;
   default:
      unreachable("Invalid component swizzle");
   }
}

static inline uint32_t
nvb097_th_bl_source(const struct nil_tic_format *fmt,
                    enum pipe_swizzle swz, bool is_int)
{
   switch (swz) {
   case PIPE_SWIZZLE_X: return fmt->src_x;
   case PIPE_SWIZZLE_Y: return fmt->src_y;
   case PIPE_SWIZZLE_Z: return fmt->src_z;
   case PIPE_SWIZZLE_W: return fmt->src_w;
   case PIPE_SWIZZLE_0:
      return NVB097_TEXHEAD_BL_X_SOURCE_IN_ZERO;
   case PIPE_SWIZZLE_1:
      return is_int ? NVB097_TEXHEAD_BL_X_SOURCE_IN_ONE_INT :
                      NVB097_TEXHEAD_BL_X_SOURCE_IN_ONE_FLOAT;
   default:
      unreachable("Invalid component swizzle");
   }
}

static uint32_t
nv9097_th_bl_0(enum pipe_format format, const enum pipe_swizzle swizzle[4])
{
   const struct nil_tic_format *fmt = nil_tic_format_for_pipe(format);
   const bool is_int = util_format_is_pure_integer(format);

   uint32_t source[4];
   for (unsigned i = 0; i < 4; i++)
      source[i] = nvb097_th_bl_source(fmt, swizzle[i], is_int);

   uint32_t th_0 = 0;
   TH_NV9097_SET_U(&th_0, 0, COMPONENT_SIZES, fmt->comp_sizes);
   TH_NV9097_SET_U(&th_0, 0, R_DATA_TYPE, fmt->type_r);
   TH_NV9097_SET_U(&th_0, 0, G_DATA_TYPE, fmt->type_g);
   TH_NV9097_SET_U(&th_0, 0, B_DATA_TYPE, fmt->type_b);
   TH_NV9097_SET_U(&th_0, 0, A_DATA_TYPE, fmt->type_a);
   TH_NV9097_SET_U(&th_0, 0, X_SOURCE, source[0]);
   TH_NV9097_SET_U(&th_0, 0, Y_SOURCE, source[1]);
   TH_NV9097_SET_U(&th_0, 0, Z_SOURCE, source[2]);
   TH_NV9097_SET_U(&th_0, 0, W_SOURCE, source[3]);

   return th_0;
}

static uint32_t
nvb097_th_bl_0(enum pipe_format format, const enum pipe_swizzle swizzle[4])
{
   const struct nil_tic_format *fmt = nil_tic_format_for_pipe(format);
   const bool is_int = util_format_is_pure_integer(format);

   uint32_t source[4];
   for (unsigned i = 0; i < 4; i++)
      source[i] = nvb097_th_bl_source(fmt, swizzle[i], is_int);

   uint32_t th_0 = 0;
   TH_NVB097_SET_U(&th_0, BL, COMPONENTS, fmt->comp_sizes);
   TH_NVB097_SET_U(&th_0, BL, R_DATA_TYPE, fmt->type_r);
   TH_NVB097_SET_U(&th_0, BL, G_DATA_TYPE, fmt->type_g);
   TH_NVB097_SET_U(&th_0, BL, B_DATA_TYPE, fmt->type_b);
   TH_NVB097_SET_U(&th_0, BL, A_DATA_TYPE, fmt->type_a);
   TH_NVB097_SET_U(&th_0, BL, X_SOURCE, source[0]);
   TH_NVB097_SET_U(&th_0, BL, Y_SOURCE, source[1]);
   TH_NVB097_SET_U(&th_0, BL, Z_SOURCE, source[2]);
   TH_NVB097_SET_U(&th_0, BL, W_SOURCE, source[3]);

   return th_0;
}

static uint32_t
pipe_to_nv_texture_type(enum nil_view_type type)
{
#define CASE(NIL, NV) \
   case NIL_VIEW_TYPE_##NIL: return NVB097_TEXHEAD_BL_TEXTURE_TYPE_##NV; \
   STATIC_ASSERT(NVB097_TEXHEAD_BL_TEXTURE_TYPE_##NV == NV9097_TEXHEADV2_2_TEXTURE_TYPE_##NV);

   switch (type) {
   CASE(1D,             ONE_D);
   CASE(2D,             TWO_D);
   CASE(3D,             THREE_D);
   CASE(CUBE,           CUBEMAP);
   CASE(1D_ARRAY,       ONE_D_ARRAY);
   CASE(2D_ARRAY,       TWO_D_ARRAY);
   CASE(CUBE_ARRAY,     CUBEMAP_ARRAY);
   default: unreachable("Invalid image view type");
   }

#undef CASE
}

static uint32_t
nil_to_nv9097_multi_sample_count(enum nil_sample_layout sample_layout)
{
#define CASE(SIZE) \
   case NIL_SAMPLE_LAYOUT_##SIZE: \
      return NV9097_TEXHEADV2_7_MULTI_SAMPLE_COUNT_MODE_##SIZE

   switch (sample_layout) {
   CASE(1X1);
   CASE(2X1);
   CASE(2X2);
   CASE(4X2);
   CASE(4X4);
   default:
      unreachable("Invalid sample layout");
   }

#undef CASE
}

static uint32_t
nil_to_nvb097_multi_sample_count(enum nil_sample_layout sample_layout)
{
#define CASE(SIZE) \
   case NIL_SAMPLE_LAYOUT_##SIZE: \
      return NVB097_TEXHEAD_BL_MULTI_SAMPLE_COUNT_MODE_##SIZE

   switch (sample_layout) {
   CASE(1X1);
   CASE(2X1);
   CASE(2X2);
   CASE(4X2);
   CASE(4X4);
   default:
      unreachable("Invalid sample layout");
   }

#undef CASE
}

static inline uint32_t
nil_max_mip_level(const struct nil_image *image,
                  const struct nil_view *view)
{
   if (view->type != NIL_VIEW_TYPE_3D && view->array_len == 1 &&
       view->base_level == 0 && view->num_levels == 1) {
      /* The Unnormalized coordinates bit in the sampler gets ignored if the
       * referenced image has more than one miplevel.  Fortunately, Vulkan has
       * restrictions requiring the view to be a single-layer single-LOD view
       * in order to use nonnormalizedCoordinates = VK_TRUE in the sampler.
       * From the Vulkan 1.3.255 spec:
       *
       *    "When unnormalizedCoordinates is VK_TRUE, images the sampler is
       *    used with in the shader have the following requirements:
       *
       *     - The viewType must be either VK_IMAGE_VIEW_TYPE_1D or
       *       VK_IMAGE_VIEW_TYPE_2D.
       *     - The image view must have a single layer and a single mip
       *       level."
       *
       * Under these conditions, the view is simply LOD 0 of a single array
       * slice so we don't need to care about aray stride between slices so
       * it's safe to set the number of miplevels to 0 regardless of how many
       * the image actually has.
       */
      return 0;
   } else {
      return image->num_levels - 1;
   }
}

static struct nil_extent4d
nil_normalize_extent(const struct nil_image *image,
                     const struct nil_view *view)
{
   struct nil_extent4d extent;

   extent.width = image->extent_px.width;
   extent.height = image->extent_px.height;
   extent.array_len = 0;

   switch (view->type) {
   case NIL_VIEW_TYPE_1D:
   case NIL_VIEW_TYPE_1D_ARRAY:
   case NIL_VIEW_TYPE_2D:
   case NIL_VIEW_TYPE_2D_ARRAY:
      assert(image->extent_px.depth == 1);
      extent.depth = view->array_len;
      break;
   case NIL_VIEW_TYPE_CUBE:
   case NIL_VIEW_TYPE_CUBE_ARRAY:
      assert(image->dim == NIL_IMAGE_DIM_2D);
      assert(view->array_len % 6 == 0);
      extent.depth = view->array_len / 6;
      break;
   case NIL_VIEW_TYPE_3D:
      assert(image->dim == NIL_IMAGE_DIM_3D);
      extent.depth = image->extent_px.depth;
      break;
   default:
      unreachable("Unsupported image view target");
   };

   return extent;
}

static void
nv9097_nil_image_fill_tic(const struct nil_image *image,
                          const struct nil_view *view,
                          uint64_t base_address,
                          void *desc_out)
{
   assert(util_format_get_blocksize(image->format) ==
          util_format_get_blocksize(view->format));
   assert(view->base_level + view->num_levels <= image->num_levels);
   assert(view->base_array_layer + view->array_len <= image->extent_px.a);

   uint32_t th[8] = { };

   TH_NV9097_SET_B(th, 4, USE_TEXTURE_HEADER_VERSION2, true);

   th[0] = nv9097_th_bl_0(view->format, view->swizzle);

   /* There's no base layer field in the texture header */
   const uint64_t layer_address =
      base_address + view->base_array_layer * image->array_stride_B;
   TH_NV9097_SET_U(th, 1, OFFSET_LOWER, layer_address & 0xffffffff);
   TH_NV9097_SET_U(th, 2, OFFSET_UPPER, layer_address >> 32);

   const struct nil_tiling *tiling = &image->levels[0].tiling;
   if (tiling->is_tiled) {
      TH_NV9097_SET_E(th, 2, MEMORY_LAYOUT, BLOCKLINEAR);

      assert(tiling->gob_height_8);
      TH_NV9097_SET_E(th, 2, GOBS_PER_BLOCK_WIDTH, ONE_GOB);
      TH_NV9097_SET_U(th, 2, GOBS_PER_BLOCK_HEIGHT, tiling->y_log2);
      TH_NV9097_SET_U(th, 2, GOBS_PER_BLOCK_DEPTH, tiling->z_log2);

      TH_NV9097_SET_U(th, 2, TEXTURE_TYPE, pipe_to_nv_texture_type(view->type));
   } else {
      TH_NV9097_SET_E(th, 2, MEMORY_LAYOUT, PITCH);

      uint32_t pitch = image->levels[0].row_stride_B;
      TH_NV9097_SET_U(th, 3, PITCH, pitch);

      assert(view->type == NIL_VIEW_TYPE_2D ||
             view->type == NIL_VIEW_TYPE_2D_ARRAY);
      assert(image->sample_layout == NIL_SAMPLE_LAYOUT_1X1);
      assert(view->num_levels == 1);
      TH_NV9097_SET_E(th, 2, TEXTURE_TYPE, TWO_D_NO_MIPMAP);
   }

   TH_NV9097_SET_E(th, 3, LOD_ANISO_QUALITY, LOD_QUALITY_HIGH);
   TH_NV9097_SET_E(th, 3, LOD_ISO_QUALITY, LOD_QUALITY_HIGH);
   TH_NV9097_SET_E(th, 3, ANISO_COARSE_SPREAD_MODIFIER, SPREAD_MODIFIER_NONE);

   const struct nil_extent4d extent = nil_normalize_extent(image, view);
   TH_NV9097_SET_U(th, 4, WIDTH, extent.width);
   TH_NV9097_SET_U(th, 5, HEIGHT, extent.height);
   TH_NV9097_SET_U(th, 5, DEPTH, extent.depth);

   TH_NV9097_SET_U(th, 5, MAX_MIP_LEVEL, nil_max_mip_level(image, view));

   TH_NV9097_SET_B(th, 2, S_R_G_B_CONVERSION,
                   util_format_is_srgb(view->format));

   TH_NV9097_SET_E(th, 2, BORDER_SOURCE, BORDER_COLOR);

   /* In the sampler, the two options for FLOAT_COORD_NORMALIZATION are:
      *
      *  - FORCE_UNNORMALIZED_COORDS
      *  - USE_HEADER_SETTING
      *
      * So we set it to normalized in the header and let the sampler select
      * that or force non-normalized.
      */
   TH_NV9097_SET_B(th, 2, NORMALIZED_COORDS, true);

   TH_NV9097_SET_E(th, 6, ANISO_FINE_SPREAD_FUNC, SPREAD_FUNC_TWO);
   TH_NV9097_SET_E(th, 6, ANISO_COARSE_SPREAD_FUNC, SPREAD_FUNC_ONE);

   TH_NV9097_SET_U(th, 7, RES_VIEW_MIN_MIP_LEVEL, view->base_level);
   TH_NV9097_SET_U(th, 7, RES_VIEW_MAX_MIP_LEVEL,
                   view->num_levels + view->base_level - 1);

   TH_NV9097_SET_U(th, 7, MULTI_SAMPLE_COUNT,
                   nil_to_nv9097_multi_sample_count(image->sample_layout));

   TH_NV9097_SET_UF(th, 7, MIN_LOD_CLAMP,
                    view->min_lod_clamp - view->base_level);

   memcpy(desc_out, th, sizeof(th));
}

static void
nvb097_nil_image_fill_tic(const struct nil_image *image,
                          const struct nil_view *view,
                          uint64_t base_address,
                          void *desc_out)
{
   assert(util_format_get_blocksize(image->format) ==
          util_format_get_blocksize(view->format));
   assert(view->base_level + view->num_levels <= image->num_levels);
   assert(view->base_array_layer + view->array_len <= image->extent_px.a);

   uint32_t th[8] = { };

   th[0] = nvb097_th_bl_0(view->format, view->swizzle);

   /* There's no base layer field in the texture header */
   const uint64_t layer_address =
      base_address + view->base_array_layer * image->array_stride_B;
   const struct nil_tiling *tiling = &image->levels[0].tiling;

   if (tiling->is_tiled) {
      TH_NVB097_SET_E(th, BL, HEADER_VERSION, SELECT_BLOCKLINEAR);
      
      assert((layer_address & BITFIELD_MASK(9)) == 0);
      TH_NVB097_SET_U(th, BL, ADDRESS_BITS31TO9, (uint32_t)layer_address >> 9);
      TH_NVB097_SET_U(th, BL, ADDRESS_BITS47TO32, layer_address >> 32);

      assert(tiling->gob_height_8);
      TH_NVB097_SET_E(th, BL, GOBS_PER_BLOCK_WIDTH, ONE_GOB);
      TH_NVB097_SET_U(th, BL, GOBS_PER_BLOCK_HEIGHT, tiling->y_log2);
      TH_NVB097_SET_U(th, BL, GOBS_PER_BLOCK_DEPTH, tiling->z_log2);

      TH_NVB097_SET_U(th, BL, TEXTURE_TYPE, pipe_to_nv_texture_type(view->type));
   } else {
      TH_NVB097_SET_E(th, PITCH, HEADER_VERSION, SELECT_PITCH);
      
      assert((layer_address & BITFIELD_MASK(5)) == 0);
      TH_NVB097_SET_U(th, PITCH, ADDRESS_BITS31TO5,
                      (uint32_t)layer_address >> 5);
      TH_NVB097_SET_U(th, PITCH, ADDRESS_BITS47TO32,
                      layer_address >> 32);

      uint32_t pitch = image->levels[0].row_stride_B;
      assert((pitch & BITFIELD_MASK(5)) == 0);
      TH_NVB097_SET_U(th, PITCH, PITCH_BITS20TO5, pitch >> 5);

      assert(view->type == NIL_VIEW_TYPE_2D ||
             view->type == NIL_VIEW_TYPE_2D_ARRAY);
      assert(image->sample_layout == NIL_SAMPLE_LAYOUT_1X1);
      assert(view->num_levels == 1);
      TH_NVB097_SET_E(th, PITCH, TEXTURE_TYPE, TWO_D_NO_MIPMAP);
   }

   TH_NVB097_SET_B(th, BL, LOD_ANISO_QUALITY2, true);
   TH_NVB097_SET_E(th, BL, LOD_ANISO_QUALITY, LOD_QUALITY_HIGH);
   TH_NVB097_SET_E(th, BL, LOD_ISO_QUALITY, LOD_QUALITY_HIGH);
   TH_NVB097_SET_E(th, BL, ANISO_COARSE_SPREAD_MODIFIER, SPREAD_MODIFIER_NONE);

   const struct nil_extent4d extent = nil_normalize_extent(image, view);
   TH_NVB097_SET_U(th, BL, WIDTH_MINUS_ONE, extent.width - 1);
   TH_NVB097_SET_U(th, BL, HEIGHT_MINUS_ONE, extent.height - 1);
   TH_NVB097_SET_U(th, BL, DEPTH_MINUS_ONE, extent.depth - 1);

   TH_NVB097_SET_U(th, BL, MAX_MIP_LEVEL, nil_max_mip_level(image, view));

   TH_NVB097_SET_B(th, BL, S_R_G_B_CONVERSION,
                   util_format_is_srgb(view->format));

   TH_NVB097_SET_E(th, BL, SECTOR_PROMOTION, PROMOTE_TO_2_V);
   TH_NVB097_SET_E(th, BL, BORDER_SIZE, BORDER_SAMPLER_COLOR);

   /* In the sampler, the two options for FLOAT_COORD_NORMALIZATION are:
      *
      *  - FORCE_UNNORMALIZED_COORDS
      *  - USE_HEADER_SETTING
      *
      * So we set it to normalized in the header and let the sampler select
      * that or force non-normalized.
      */
   TH_NVB097_SET_B(th, BL, NORMALIZED_COORDS, true);

   TH_NVB097_SET_E(th, BL, ANISO_FINE_SPREAD_FUNC, SPREAD_FUNC_TWO);
   TH_NVB097_SET_E(th, BL, ANISO_COARSE_SPREAD_FUNC, SPREAD_FUNC_ONE);

   TH_NVB097_SET_U(th, BL, RES_VIEW_MIN_MIP_LEVEL, view->base_level);
   TH_NVB097_SET_U(th, BL, RES_VIEW_MAX_MIP_LEVEL,
                   view->num_levels + view->base_level - 1);

   TH_NVB097_SET_U(th, BL, MULTI_SAMPLE_COUNT,
                   nil_to_nvb097_multi_sample_count(image->sample_layout));

   TH_NVB097_SET_UF(th, BL, MIN_LOD_CLAMP,
                    view->min_lod_clamp - view->base_level);

   memcpy(desc_out, th, sizeof(th));
}

static const enum pipe_swizzle IDENTITY_SWIZZLE[4] = {
   PIPE_SWIZZLE_X,
   PIPE_SWIZZLE_Y,
   PIPE_SWIZZLE_Z,
   PIPE_SWIZZLE_W,
};

static void
nv9097_nil_buffer_fill_tic(uint64_t base_address,
                           enum pipe_format format,
                           uint32_t num_elements,
                           void *desc_out)
{
   uint32_t th[8] = { };

   TH_NV9097_SET_B(th, 4, USE_TEXTURE_HEADER_VERSION2, true);

   assert(!util_format_is_compressed(format));
   th[0] = nv9097_th_bl_0(format, IDENTITY_SWIZZLE);

   TH_NV9097_SET_U(th, 1, OFFSET_LOWER, base_address);
   TH_NV9097_SET_U(th, 2, OFFSET_UPPER, base_address >> 32);
   TH_NV9097_SET_E(th, 2, MEMORY_LAYOUT, PITCH);

   TH_NV9097_SET_U(th, 4, WIDTH, num_elements);
   TH_NV9097_SET_E(th, 2, TEXTURE_TYPE, ONE_D_BUFFER);


   memcpy(desc_out, th, sizeof(th));
}

static void
nvb097_nil_buffer_fill_tic(uint64_t base_address,
                           enum pipe_format format,
                           uint32_t num_elements,
                           void *desc_out)
{
   uint32_t th[8] = { };

   assert(!util_format_is_compressed(format));
   th[0] = nvb097_th_bl_0(format, IDENTITY_SWIZZLE);

   TH_NVB097_SET_U(th, 1D, ADDRESS_BITS31TO0, base_address);
   TH_NVB097_SET_U(th, 1D, ADDRESS_BITS47TO32, base_address >> 32);
   TH_NVB097_SET_E(th, 1D, HEADER_VERSION, SELECT_ONE_D_BUFFER);

   TH_NVB097_SET_U(th, 1D, WIDTH_MINUS_ONE_BITS15TO0,
            (num_elements - 1) & 0xffff);
   TH_NVB097_SET_U(th, 1D, WIDTH_MINUS_ONE_BITS31TO16,
            (num_elements - 1) >> 16);

   TH_NVB097_SET_E(th, 1D, TEXTURE_TYPE, ONE_D_BUFFER);

   /* TODO: Do we need this? */
   TH_NVB097_SET_E(th, 1D, SECTOR_PROMOTION, PROMOTE_TO_2_V);

   memcpy(desc_out, th, sizeof(th));
}

void
nil_image_fill_tic(struct nv_device_info *dev,
                   const struct nil_image *image,
                   const struct nil_view *view,
                   uint64_t base_address,
                   void *desc_out)
{
   if (dev->cls_eng3d >= MAXWELL_A) {
      nvb097_nil_image_fill_tic(image, view, base_address, desc_out);
   } else if (dev->cls_eng3d >= FERMI_A) {
      nv9097_nil_image_fill_tic(image, view, base_address, desc_out);
   } else {
      unreachable("Tesla and older not supported");
   }
}

void
nil_buffer_fill_tic(struct nv_device_info *dev,
                    uint64_t base_address,
                    enum pipe_format format,
                    uint32_t num_elements,
                    void *desc_out)
{
   if (dev->cls_eng3d >= MAXWELL_A) {
      nvb097_nil_buffer_fill_tic(base_address, format, num_elements, desc_out);
   } else if (dev->cls_eng3d >= FERMI_A) {
      nv9097_nil_buffer_fill_tic(base_address, format, num_elements, desc_out);
   } else {
      unreachable("Tesla and older not supported");
   }
}
