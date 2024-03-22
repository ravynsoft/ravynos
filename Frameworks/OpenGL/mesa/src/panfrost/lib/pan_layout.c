/*
 * Copyright (C) 2019-2022 Collabora, Ltd.
 * Copyright (C) 2018-2019 Alyssa Rosenzweig
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
 */

#include "util/macros.h"
#include "util/u_math.h"
#include "pan_texture.h"

/*
 * List of supported modifiers, in descending order of preference. AFBC is
 * faster than u-interleaved tiling which is faster than linear. Within AFBC,
 * enabling the YUV-like transform is typically a win where possible.
 */
uint64_t pan_best_modifiers[PAN_MODIFIER_COUNT] = {
   DRM_FORMAT_MOD_ARM_AFBC(AFBC_FORMAT_MOD_BLOCK_SIZE_16x16 |
                           AFBC_FORMAT_MOD_TILED | AFBC_FORMAT_MOD_SC |
                           AFBC_FORMAT_MOD_SPARSE | AFBC_FORMAT_MOD_YTR),

   DRM_FORMAT_MOD_ARM_AFBC(AFBC_FORMAT_MOD_BLOCK_SIZE_16x16 |
                           AFBC_FORMAT_MOD_TILED | AFBC_FORMAT_MOD_SC |
                           AFBC_FORMAT_MOD_SPARSE),

   DRM_FORMAT_MOD_ARM_AFBC(AFBC_FORMAT_MOD_BLOCK_SIZE_16x16 |
                           AFBC_FORMAT_MOD_SPARSE | AFBC_FORMAT_MOD_YTR),

   DRM_FORMAT_MOD_ARM_AFBC(AFBC_FORMAT_MOD_BLOCK_SIZE_16x16 |
                           AFBC_FORMAT_MOD_SPARSE),

   DRM_FORMAT_MOD_ARM_16X16_BLOCK_U_INTERLEAVED,
   DRM_FORMAT_MOD_LINEAR};

/* Table of AFBC superblock sizes */
static const struct pan_block_size afbc_superblock_sizes[] = {
   [AFBC_FORMAT_MOD_BLOCK_SIZE_16x16] = {16, 16},
   [AFBC_FORMAT_MOD_BLOCK_SIZE_32x8] = {32, 8},
   [AFBC_FORMAT_MOD_BLOCK_SIZE_64x4] = {64, 4},
};

/*
 * Given an AFBC modifier, return the superblock size.
 *
 * We do not yet have any use cases for multiplanar YCBCr formats with different
 * superblock sizes on the luma and chroma planes. These formats are unsupported
 * for now.
 */
struct pan_block_size
panfrost_afbc_superblock_size(uint64_t modifier)
{
   unsigned index = (modifier & AFBC_FORMAT_MOD_BLOCK_SIZE_MASK);

   assert(drm_is_afbc(modifier));
   assert(index < ARRAY_SIZE(afbc_superblock_sizes));

   return afbc_superblock_sizes[index];
}

/*
 * Given an AFBC modifier, return the width of the superblock.
 */
unsigned
panfrost_afbc_superblock_width(uint64_t modifier)
{
   return panfrost_afbc_superblock_size(modifier).width;
}

/*
 * Given an AFBC modifier, return the height of the superblock.
 */
unsigned
panfrost_afbc_superblock_height(uint64_t modifier)
{
   return panfrost_afbc_superblock_size(modifier).height;
}

/*
 * Given an AFBC modifier, return if "wide blocks" are used. Wide blocks are
 * defined as superblocks wider than 16 pixels, the minimum (and default) super
 * block width.
 */
bool
panfrost_afbc_is_wide(uint64_t modifier)
{
   return panfrost_afbc_superblock_width(modifier) > 16;
}

/*
 * Given an AFBC modifier, return the subblock size (subdivision of a
 * superblock). This is always 4x4 for now as we only support one AFBC
 * superblock layout.
 */
struct pan_block_size
panfrost_afbc_subblock_size(uint64_t modifier)
{
   return (struct pan_block_size){4, 4};
}

/*
 * Given a format, determine the tile size used for u-interleaving. For formats
 * that are already block compressed, this is 4x4. For all other formats, this
 * is 16x16, hence the modifier name.
 */
static inline struct pan_block_size
panfrost_u_interleaved_tile_size(enum pipe_format format)
{
   if (util_format_is_compressed(format))
      return (struct pan_block_size){4, 4};
   else
      return (struct pan_block_size){16, 16};
}

/*
 * Determine the block size used for interleaving. For u-interleaving, this is
 * the tile size. For AFBC, this is the superblock size. For linear textures,
 * this is trivially 1x1.
 */
struct pan_block_size
panfrost_block_size(uint64_t modifier, enum pipe_format format)
{
   if (modifier == DRM_FORMAT_MOD_ARM_16X16_BLOCK_U_INTERLEAVED)
      return panfrost_u_interleaved_tile_size(format);
   else if (drm_is_afbc(modifier))
      return panfrost_afbc_superblock_size(modifier);
   else
      return (struct pan_block_size){1, 1};
}

/*
 * Determine the tile size used by AFBC. This tiles superblocks themselves.
 * Current GPUs support either 8x8 tiling or no tiling (1x1)
 */
static inline unsigned
pan_afbc_tile_size(uint64_t modifier)
{
   return (modifier & AFBC_FORMAT_MOD_TILED) ? 8 : 1;
}

/*
 * Determine the number of bytes between header rows for an AFBC image. For an
 * image with linear headers, this is simply the number of header blocks
 * (=superblocks) per row times the numbers of bytes per header block. For an
 * image with tiled headers, this is multipled by the number of rows of
 * header blocks are in a tile together.
 */
uint32_t
pan_afbc_row_stride(uint64_t modifier, uint32_t width)
{
   unsigned block_width = panfrost_afbc_superblock_width(modifier);

   return (width / block_width) * pan_afbc_tile_size(modifier) *
          AFBC_HEADER_BYTES_PER_TILE;
}

/*
 * Determine the number of header blocks between header rows. This is equal to
 * the number of bytes between header rows divided by the bytes per blocks of a
 * header tile. This is also divided by the tile size to give a "line stride" in
 * blocks, rather than a real row stride. This is required by Bifrost.
 */
uint32_t
pan_afbc_stride_blocks(uint64_t modifier, uint32_t row_stride_bytes)
{
   return row_stride_bytes /
          (AFBC_HEADER_BYTES_PER_TILE * pan_afbc_tile_size(modifier));
}

/*
 * Determine the required alignment for the slice offset of an image. For
 * now, this is always aligned on 64-byte boundaries. */
uint32_t
pan_slice_align(uint64_t modifier)
{
   return 64;
}

/*
 * Determine the required alignment for the body offset of an AFBC image. For
 * now, this depends only on whether tiling is in use. These minimum alignments
 * are required on all current GPUs.
 */
uint32_t
pan_afbc_body_align(uint64_t modifier)
{
   return (modifier & AFBC_FORMAT_MOD_TILED) ? 4096 : 64;
}

static inline unsigned
format_minimum_alignment(const struct panfrost_device *dev,
                         enum pipe_format format, bool afbc)
{
   if (afbc)
      return 16;

   if (dev->arch < 7)
      return 64;

   switch (format) {
   /* For v7+, NV12/NV21/I420 have a looser alignment requirement of 16 bytes */
   case PIPE_FORMAT_R8_G8B8_420_UNORM:
   case PIPE_FORMAT_G8_B8R8_420_UNORM:
   case PIPE_FORMAT_R8_G8_B8_420_UNORM:
   case PIPE_FORMAT_R8_B8_G8_420_UNORM:
      return 16;
   default:
      return 64;
   }
}

/* Computes sizes for checksumming, which is 8 bytes per 16x16 tile.
 * Checksumming is believed to be a CRC variant (CRC64 based on the size?).
 * This feature is also known as "transaction elimination". */

#define CHECKSUM_TILE_WIDTH     16
#define CHECKSUM_TILE_HEIGHT    16
#define CHECKSUM_BYTES_PER_TILE 8

unsigned
panfrost_compute_checksum_size(struct pan_image_slice_layout *slice,
                               unsigned width, unsigned height)
{
   unsigned tile_count_x = DIV_ROUND_UP(width, CHECKSUM_TILE_WIDTH);
   unsigned tile_count_y = DIV_ROUND_UP(height, CHECKSUM_TILE_HEIGHT);

   slice->crc.stride = tile_count_x * CHECKSUM_BYTES_PER_TILE;

   return slice->crc.stride * tile_count_y;
}

unsigned
panfrost_get_layer_stride(const struct pan_image_layout *layout, unsigned level)
{
   if (layout->dim != MALI_TEXTURE_DIMENSION_3D)
      return layout->array_stride;
   else if (drm_is_afbc(layout->modifier))
      return layout->slices[level].afbc.surface_stride;
   else
      return layout->slices[level].surface_stride;
}

unsigned
panfrost_get_legacy_stride(const struct pan_image_layout *layout,
                           unsigned level)
{
   unsigned row_stride = layout->slices[level].row_stride;
   struct pan_block_size block_size =
      panfrost_block_size(layout->modifier, layout->format);

   if (drm_is_afbc(layout->modifier)) {
      unsigned width = u_minify(layout->width, level);
      unsigned alignment =
         block_size.width * pan_afbc_tile_size(layout->modifier);

      width = ALIGN_POT(width, alignment);
      return width * util_format_get_blocksize(layout->format);
   } else {
      return row_stride / block_size.height;
   }
}

unsigned
panfrost_from_legacy_stride(unsigned legacy_stride, enum pipe_format format,
                            uint64_t modifier)
{
   struct pan_block_size block_size = panfrost_block_size(modifier, format);

   if (drm_is_afbc(modifier)) {
      unsigned width = legacy_stride / util_format_get_blocksize(format);

      return pan_afbc_row_stride(modifier, width);
   } else {
      return legacy_stride * block_size.height;
   }
}

/* Computes the offset into a texture at a particular level/face. Add to
 * the base address of a texture to get the address to that level/face */

unsigned
panfrost_texture_offset(const struct pan_image_layout *layout, unsigned level,
                        unsigned array_idx, unsigned surface_idx)
{
   return layout->slices[level].offset + (array_idx * layout->array_stride) +
          (surface_idx * layout->slices[level].surface_stride);
}

bool
pan_image_layout_init(const struct panfrost_device *dev,
                      struct pan_image_layout *layout,
                      const struct pan_image_explicit_layout *explicit_layout)
{
   /* Explicit stride only work with non-mipmap, non-array, single-sample
    * 2D image without CRC.
    */
   if (explicit_layout &&
       (layout->depth > 1 || layout->nr_samples > 1 || layout->array_size > 1 ||
        layout->dim != MALI_TEXTURE_DIMENSION_2D || layout->nr_slices > 1 ||
        layout->crc))
      return false;

   bool afbc = drm_is_afbc(layout->modifier);
   int align_req = format_minimum_alignment(dev, layout->format, afbc);

   /* Mandate alignment */
   if (explicit_layout) {
      bool rejected = false;

      int align_mask = align_req - 1;

      if (dev->arch >= 7) {
         rejected = ((explicit_layout->offset & align_mask) ||
                     (explicit_layout->row_stride & align_mask));
      } else {
         rejected = (explicit_layout->offset & align_mask);
      }

      if (rejected) {
         mesa_loge(
            "panfrost: rejecting image due to unsupported offset or stride "
            "alignment.\n");
         return false;
      }
   }

   unsigned fmt_blocksize = util_format_get_blocksize(layout->format);

   /* MSAA is implemented as a 3D texture with z corresponding to the
    * sample #, horrifyingly enough */

   assert(layout->depth == 1 || layout->nr_samples == 1);

   bool linear = layout->modifier == DRM_FORMAT_MOD_LINEAR;
   bool is_3d = layout->dim == MALI_TEXTURE_DIMENSION_3D;

   unsigned offset = explicit_layout ? explicit_layout->offset : 0;
   struct pan_block_size block_size =
      panfrost_block_size(layout->modifier, layout->format);

   unsigned width = layout->width;
   unsigned height = layout->height;
   unsigned depth = layout->depth;

   unsigned align_w = block_size.width;
   unsigned align_h = block_size.height;

   /* For tiled AFBC, align to tiles of superblocks (this can be large) */
   if (afbc) {
      align_w *= pan_afbc_tile_size(layout->modifier);
      align_h *= pan_afbc_tile_size(layout->modifier);
   }

   for (unsigned l = 0; l < layout->nr_slices; ++l) {
      struct pan_image_slice_layout *slice = &layout->slices[l];

      unsigned effective_width =
         ALIGN_POT(util_format_get_nblocksx(layout->format, width), align_w);
      unsigned effective_height =
         ALIGN_POT(util_format_get_nblocksy(layout->format, height), align_h);

      /* Align levels to cache-line as a performance improvement for
       * linear/tiled and as a requirement for AFBC */

      offset = ALIGN_POT(offset, pan_slice_align(layout->modifier));

      slice->offset = offset;

      unsigned row_stride = fmt_blocksize * effective_width * block_size.height;

      /* On v7+ row_stride and offset alignment requirement are equal */
      if (dev->arch >= 7) {
         row_stride = ALIGN_POT(row_stride, align_req);
      }

      if (explicit_layout && !afbc) {
         /* Make sure the explicit stride is valid */
         if (explicit_layout->row_stride < row_stride) {
            mesa_loge("panfrost: rejecting image due to invalid row stride.\n");
            return false;
         }

         row_stride = explicit_layout->row_stride;
      } else if (linear) {
         /* Keep lines alignment on 64 byte for performance */
         row_stride = ALIGN_POT(row_stride, 64);
      }

      unsigned slice_one_size =
         row_stride * (effective_height / block_size.height);

      /* Compute AFBC sizes if necessary */
      if (afbc) {
         slice->row_stride =
            pan_afbc_row_stride(layout->modifier, effective_width);
         slice->afbc.stride = effective_width / block_size.width;
         slice->afbc.nr_blocks =
            slice->afbc.stride * (effective_height / block_size.height);
         slice->afbc.header_size =
            ALIGN_POT(slice->row_stride * (effective_height / align_h),
                      pan_afbc_body_align(layout->modifier));

         if (explicit_layout &&
             explicit_layout->row_stride < slice->row_stride) {
            mesa_loge("panfrost: rejecting image due to invalid row stride.\n");
            return false;
         }

         /* AFBC body size */
         slice->afbc.body_size = slice_one_size;

         /* 3D AFBC resources have all headers placed at the
          * beginning instead of having them split per depth
          * level
          */
         if (is_3d) {
            slice->afbc.surface_stride = slice->afbc.header_size;
            slice->afbc.header_size *= depth;
            slice->afbc.body_size *= depth;
            offset += slice->afbc.header_size;
         } else {
            slice_one_size += slice->afbc.header_size;
            slice->afbc.surface_stride = slice_one_size;
         }
      } else {
         slice->row_stride = row_stride;
      }

      unsigned slice_full_size = slice_one_size * depth * layout->nr_samples;

      slice->surface_stride = slice_one_size;

      /* Compute AFBC sizes if necessary */

      offset += slice_full_size;
      slice->size = slice_full_size;

      /* Add a checksum region if necessary */
      if (layout->crc) {
         slice->crc.size = panfrost_compute_checksum_size(slice, width, height);

         slice->crc.offset = offset;
         offset += slice->crc.size;
         slice->size += slice->crc.size;
      }

      width = u_minify(width, 1);
      height = u_minify(height, 1);
      depth = u_minify(depth, 1);
   }

   /* Arrays and cubemaps have the entire miptree duplicated */
   layout->array_stride = ALIGN_POT(offset, 64);
   if (explicit_layout)
      layout->data_size = offset;
   else
      layout->data_size =
         ALIGN_POT(layout->array_stride * layout->array_size, 4096);

   return true;
}

void
pan_iview_get_surface(const struct pan_image_view *iview, unsigned level,
                      unsigned layer, unsigned sample, struct pan_surface *surf)
{
   const struct pan_image *image = pan_image_view_get_plane(iview, 0);

   level += iview->first_level;
   assert(level < image->layout.nr_slices);

   layer += iview->first_layer;

   bool is_3d = image->layout.dim == MALI_TEXTURE_DIMENSION_3D;
   const struct pan_image_slice_layout *slice = &image->layout.slices[level];
   mali_ptr base = image->data.bo->ptr.gpu + image->data.offset;

   if (drm_is_afbc(image->layout.modifier)) {
      assert(!sample);

      if (is_3d) {
         ASSERTED unsigned depth = u_minify(image->layout.depth, level);
         assert(layer < depth);
         surf->afbc.header =
            base + slice->offset + (layer * slice->afbc.surface_stride);
         surf->afbc.body = base + slice->offset + slice->afbc.header_size +
                           (slice->surface_stride * layer);
      } else {
         assert(layer < image->layout.array_size);
         surf->afbc.header =
            base + panfrost_texture_offset(&image->layout, level, layer, 0);
         surf->afbc.body = surf->afbc.header + slice->afbc.header_size;
      }
   } else {
      unsigned array_idx = is_3d ? 0 : layer;
      unsigned surface_idx = is_3d ? layer : sample;

      surf->data = base + panfrost_texture_offset(&image->layout, level,
                                                  array_idx, surface_idx);
   }
}
