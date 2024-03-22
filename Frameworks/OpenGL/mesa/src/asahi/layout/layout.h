/*
 * Copyright 2022 Alyssa Rosenzweig
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef __AIL_LAYOUT_H_
#define __AIL_LAYOUT_H_

#include "util/format/u_format.h"
#include "util/macros.h"
#include "util/u_math.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AIL_CACHELINE      0x80
#define AIL_PAGESIZE       0x4000
#define AIL_MAX_MIP_LEVELS 16

enum ail_tiling {
   /**
    * Strided linear (raster order). Only allowed for 1D or 2D, without
    * mipmapping, multisampling, block-compression, or arrays.
    */
   AIL_TILING_LINEAR,

   /**
    * Twiddled (Morton order). Always allowed.
    */
   AIL_TILING_TWIDDLED,

   /**
    * Twiddled (Morton order) with compression.
    */
   AIL_TILING_TWIDDLED_COMPRESSED,
};

/*
 * Represents the dimensions of a single tile. Used to describe tiled layouts.
 * Width and height are in units of elements, not pixels, to model compressed
 * textures corrects.
 *
 * Invariant: width_el and height_el are powers of two.
 */
struct ail_tile {
   unsigned width_el, height_el;
};

/*
 * An AGX image layout.
 */
struct ail_layout {
   /** Width, height, and depth in pixels at level 0 */
   uint32_t width_px, height_px, depth_px;

   /** Number of samples per pixel. 1 if multisampling is disabled. */
   uint8_t sample_count_sa;

   /** Number of miplevels. 1 if no mipmapping is used. */
   uint8_t levels;

   /** Should this image be mipmapped along the Z-axis in addition to the X- and
    * Y-axes? This should be set for API-level 3D images, but not 2D arrays or
    * cubes.
    */
   bool mipmapped_z;

   /** Tiling mode used */
   enum ail_tiling tiling;

   /** Texture format */
   enum pipe_format format;

   /**
    * If tiling is LINEAR, the number of bytes between adjacent rows of
    * elements. Otherwise, this field is zero.
    */
   uint32_t linear_stride_B;

   /**
    * Stride between layers of an array texture, including a cube map. Layer i
    * begins at offset (i * layer_stride_B) from the beginning of the texture.
    *
    * If depth_px = 1, the value of this field is UNDEFINED.
    */
   uint64_t layer_stride_B;

   /**
    * Whether the layer stride is aligned to the page size or not. The hardware
    * needs this flag to compute the implicit layer stride.
    */
   bool page_aligned_layers;

   /**
    * Offsets of mip levels within a layer.
    */
   uint64_t level_offsets_B[AIL_MAX_MIP_LEVELS];

   /**
    * For the compressed buffer, offsets of mip levels within a layer.
    */
   uint64_t level_offsets_compressed_B[AIL_MAX_MIP_LEVELS];

   /**
    * If tiling is TWIDDLED, the tile size used for each mip level within a
    * layer. Calculating tile sizes is the sole responsibility of
    * ail_initialized_twiddled.
    */
   struct ail_tile tilesize_el[AIL_MAX_MIP_LEVELS];

   /**
    * If tiling is TWIDDLED, the stride in elements used for each mip level
    * within a layer. Calculating level strides is the sole responsibility of
    * ail_initialized_twiddled. This is necessary because compressed pixel
    * formats may add extra stride padding.
    */
   uint32_t stride_el[AIL_MAX_MIP_LEVELS];

   /* Offset of the start of the compression metadata buffer */
   uint32_t metadata_offset_B;

   /* Stride between subsequent layers in the compression metadata buffer */
   uint64_t compression_layer_stride_B;

   /* Size of entire texture */
   uint64_t size_B;

   /* Must the layout support writeable images? If false, the layout MUST NOT be
    * used as a writeable image (either PBE or image atomics).
    */
   bool writeable_image;

   /* Must the layout support rendering? If false, the layout MUST NOT be used
    * for rendering, either PBE or ZLS.
    */
   bool renderable;
};

static inline uint32_t
ail_get_linear_stride_B(struct ail_layout *layout, ASSERTED uint8_t level)
{
   assert(layout->tiling == AIL_TILING_LINEAR && "Invalid usage");
   assert(level == 0 && "Strided linear mipmapped textures are unsupported");

   return layout->linear_stride_B;
}

/*
 * For WSI purposes, we need to associate a stride with all layouts. In the
 * hardware, only strided linear images have an associated stride, there is no
 * natural stride associated with twiddled images. However, various clients
 * assert that the stride is valid for the image if it were linear (even if it
 * is in fact not linear). In those cases, by convention we use the minimum
 * valid such stride.
 */
static inline uint32_t
ail_get_wsi_stride_B(struct ail_layout *layout, unsigned level)
{
   assert(level == 0 && "Mipmaps cannot be shared as WSI");

   if (layout->tiling == AIL_TILING_LINEAR)
      return ail_get_linear_stride_B(layout, level);
   else
      return util_format_get_stride(layout->format, layout->width_px);
}

static inline uint32_t
ail_get_layer_offset_B(struct ail_layout *layout, unsigned z_px)
{
   return z_px * layout->layer_stride_B;
}

static inline uint32_t
ail_get_level_offset_B(struct ail_layout *layout, unsigned level)
{
   return layout->level_offsets_B[level];
}

static inline uint32_t
ail_get_layer_level_B(struct ail_layout *layout, unsigned z_px, unsigned level)
{
   return ail_get_layer_offset_B(layout, z_px) +
          ail_get_level_offset_B(layout, level);
}

static inline uint32_t
ail_get_linear_pixel_B(struct ail_layout *layout, ASSERTED unsigned level,
                       uint32_t x_px, uint32_t y_px, uint32_t z_px)
{
   assert(level == 0 && "Strided linear mipmapped textures are unsupported");
   assert(util_format_get_blockwidth(layout->format) == 1 &&
          "Strided linear block formats unsupported");
   assert(util_format_get_blockheight(layout->format) == 1 &&
          "Strided linear block formats unsupported");
   assert(layout->sample_count_sa == 1 &&
          "Strided linear multisampling unsupported");

   return ail_get_layer_offset_B(layout, z_px) +
          (y_px * ail_get_linear_stride_B(layout, level)) +
          (x_px * util_format_get_blocksize(layout->format));
}

static inline unsigned
ail_effective_width_sa(unsigned width_px, unsigned sample_count_sa)
{
   return width_px * (sample_count_sa == 4 ? 2 : 1);
}

static inline unsigned
ail_effective_height_sa(unsigned height_px, unsigned sample_count_sa)
{
   return height_px * (sample_count_sa >= 2 ? 2 : 1);
}

static inline bool
ail_can_compress(unsigned w_px, unsigned h_px, unsigned sample_count_sa)
{
   assert(sample_count_sa == 1 || sample_count_sa == 2 || sample_count_sa == 4);

   /* Small textures cannot be compressed */
   return ail_effective_width_sa(w_px, sample_count_sa) >= 16 &&
          ail_effective_height_sa(h_px, sample_count_sa) >= 16;
}

static inline bool
ail_is_compressed(struct ail_layout *layout)
{
   return layout->tiling == AIL_TILING_TWIDDLED_COMPRESSED;
}

/*
 * Even when the base mip level is compressed, high levels of the miptree
 * (smaller than 16 pixels on either axis) are not compressed as it would be
 * pointless. This queries this case.
 */
static inline bool
ail_is_level_compressed(struct ail_layout *layout, unsigned level)
{
   unsigned width_sa = ALIGN(
      ail_effective_width_sa(layout->width_px, layout->sample_count_sa), 16);

   unsigned height_sa = ALIGN(
      ail_effective_height_sa(layout->height_px, layout->sample_count_sa), 16);

   return ail_is_compressed(layout) &&
          u_minify(MAX2(width_sa, height_sa), level) >= 16;
}

static inline bool
ail_is_level_twiddled_uncompressed(struct ail_layout *layout, unsigned level)
{
   switch (layout->tiling) {
   case AIL_TILING_TWIDDLED:
      return true;
   case AIL_TILING_TWIDDLED_COMPRESSED:
      return !ail_is_level_compressed(layout, level);
   default:
      return false;
   }
}

void ail_make_miptree(struct ail_layout *layout);

void ail_detile(void *_tiled, void *_linear, struct ail_layout *tiled_layout,
                unsigned level, unsigned linear_pitch_B, unsigned sx_px,
                unsigned sy_px, unsigned width_px, unsigned height_px);

void ail_tile(void *_tiled, void *_linear, struct ail_layout *tiled_layout,
              unsigned level, unsigned linear_pitch_B, unsigned sx_px,
              unsigned sy_px, unsigned width_px, unsigned height_px);

#ifdef __cplusplus
} /* extern C */
#endif

#endif
