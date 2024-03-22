/*
 * Copyright (C) 2008 VMware, Inc.
 * Copyright (C) 2014 Broadcom
 * Copyright (C) 2018-2019 Alyssa Rosenzweig
 * Copyright (C) 2019-2020 Collabora, Ltd.
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

#ifndef __PAN_TEXTURE_H
#define __PAN_TEXTURE_H

#include "genxml/gen_macros.h"

#include <stdbool.h>
#include "compiler/shader_enums.h"
#include "drm-uapi/drm_fourcc.h"
#include "genxml/gen_macros.h"
#include "util/format/u_format.h"
#include "pan_bo.h"
#include "pan_device.h"
#include "pan_format.h"
#include "pan_util.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PAN_MODIFIER_COUNT 6
extern uint64_t pan_best_modifiers[PAN_MODIFIER_COUNT];

struct pan_image_slice_layout {
   unsigned offset;

   /* For AFBC images, the number of bytes between two rows of AFBC
    * headers.
    *
    * For non-AFBC images, the number of bytes between two rows of texels.
    * For linear images, this will equal the logical stride. For
    * images that are compressed or interleaved, this will be greater than
    * the logical stride.
    */
   unsigned row_stride;

   unsigned surface_stride;

   struct {
      /* Stride in number of superblocks */
      unsigned stride;

      /* Number of superblocks */
      unsigned nr_blocks;

      /* Size of the AFBC header preceding each slice */
      unsigned header_size;

      /* Size of the AFBC body */
      unsigned body_size;

      /* Stride between AFBC headers of two consecutive surfaces.
       * For 3D textures, this must be set to header size since
       * AFBC headers are allocated together, for 2D arrays this
       * should be set to size0, since AFBC headers are placed at
       * the beginning of each layer
       */
      unsigned surface_stride;
   } afbc;

   /* If checksumming is enabled following the slice, what
    * is its offset/stride? */
   struct {
      unsigned offset;
      unsigned stride;
      unsigned size;
   } crc;

   unsigned size;
};

struct pan_image_layout {
   uint64_t modifier;
   enum pipe_format format;
   unsigned width, height, depth;
   unsigned nr_samples;
   enum mali_texture_dimension dim;
   unsigned nr_slices;
   unsigned array_size;
   bool crc;

   /* The remaining fields may be derived from the above by calling
    * pan_image_layout_init
    */

   struct pan_image_slice_layout slices[MAX_MIP_LEVELS];

   unsigned data_size;
   unsigned array_stride;
};

struct pan_image_mem {
   struct panfrost_bo *bo;
   unsigned offset;
};

struct pan_image {
   struct pan_image_mem data;
   struct pan_image_layout layout;
};

struct pan_image_view {
   /* Format, dimension and sample count of the view might differ from
    * those of the image (2D view of a 3D image surface for instance).
    */
   enum pipe_format format;
   enum mali_texture_dimension dim;
   unsigned first_level, last_level;
   unsigned first_layer, last_layer;
   unsigned char swizzle[4];

   /* planes 1 and 2 are NULL for single plane formats */
   const struct pan_image *planes[MAX_IMAGE_PLANES];

   /* If EXT_multisampled_render_to_texture is used, this may be
    * greater than image->layout.nr_samples. */
   unsigned nr_samples;

   /* Only valid if dim == 1D, needed to implement buffer views */
   struct {
      unsigned offset;
      unsigned size;
   } buf;
};

static inline const struct pan_image *
pan_image_view_get_plane(const struct pan_image_view *iview, uint32_t idx)
{
   if (idx >= ARRAY_SIZE(iview->planes))
      return NULL;

   return iview->planes[idx];
}

static inline uint32_t
pan_image_view_get_nr_samples(const struct pan_image_view *iview)
{
   /* All planes should have the same nr_samples value, so we
    * just pick the first plane. */
   const struct pan_image *image = pan_image_view_get_plane(iview, 0);

   if (!image)
      return 0;

   return image->layout.nr_samples;
}

static inline const struct pan_image *
pan_image_view_get_rt_image(const struct pan_image_view *iview)
{
   /* We only support rendering to plane 0 */
   assert(pan_image_view_get_plane(iview, 1) == NULL);
   return pan_image_view_get_plane(iview, 0);
}

static inline bool
pan_image_view_has_crc(const struct pan_image_view *iview)
{
   const struct pan_image *image = pan_image_view_get_rt_image(iview);

   if (!image)
      return false;

   return image->layout.crc;
}

static inline const struct pan_image *
pan_image_view_get_zs_image(const struct pan_image_view *iview)
{
   /* We split depth/stencil combined formats, and end up with only
    * singleplanar depth and stencil formats. */
   assert(util_format_is_depth_or_stencil(iview->format));
   assert(pan_image_view_get_plane(iview, 1) == NULL);
   return pan_image_view_get_plane(iview, 0);
}

unsigned panfrost_compute_checksum_size(struct pan_image_slice_layout *slice,
                                        unsigned width, unsigned height);

/* AFBC format mode. The ordering is intended to match the Valhall hardware enum
 * ("AFBC Compression Mode"), but this enum is required in software on older
 * hardware for correct handling of texture views. Defining the enum lets us
 * unify these code paths.
 */
enum pan_afbc_mode {
   PAN_AFBC_MODE_R8,
   PAN_AFBC_MODE_R8G8,
   PAN_AFBC_MODE_R5G6B5,
   PAN_AFBC_MODE_R4G4B4A4,
   PAN_AFBC_MODE_R5G5B5A1,
   PAN_AFBC_MODE_R8G8B8,
   PAN_AFBC_MODE_R8G8B8A8,
   PAN_AFBC_MODE_R10G10B10A2,
   PAN_AFBC_MODE_R11G11B10,
   PAN_AFBC_MODE_S8,

   /* Sentintel signalling a format that cannot be compressed */
   PAN_AFBC_MODE_INVALID
};

bool panfrost_format_supports_afbc(const struct panfrost_device *dev,
                                   enum pipe_format format);

enum pan_afbc_mode panfrost_afbc_format(unsigned arch, enum pipe_format format);

#define AFBC_HEADER_BYTES_PER_TILE 16

bool panfrost_afbc_can_ytr(enum pipe_format format);

bool panfrost_afbc_can_pack(enum pipe_format format);

bool panfrost_afbc_can_tile(const struct panfrost_device *dev);

/*
 * Represents the block size of a single plane. For AFBC, this represents the
 * superblock size. For u-interleaving, this represents the tile size.
 */
struct pan_block_size {
   /** Width of block */
   unsigned width;

   /** Height of blocks */
   unsigned height;
};

struct pan_block_size panfrost_afbc_superblock_size(uint64_t modifier);

unsigned panfrost_afbc_superblock_width(uint64_t modifier);

unsigned panfrost_afbc_superblock_height(uint64_t modifier);

bool panfrost_afbc_is_wide(uint64_t modifier);

struct pan_block_size panfrost_afbc_subblock_size(uint64_t modifier);

uint32_t pan_afbc_row_stride(uint64_t modifier, uint32_t width);

uint32_t pan_afbc_stride_blocks(uint64_t modifier, uint32_t row_stride_bytes);

uint32_t pan_slice_align(uint64_t modifier);

uint32_t pan_afbc_body_align(uint64_t modifier);

struct pan_block_size panfrost_block_size(uint64_t modifier,
                                          enum pipe_format format);

#ifdef PAN_ARCH
unsigned GENX(panfrost_estimate_texture_payload_size)(
   const struct pan_image_view *iview);

void GENX(panfrost_new_texture)(const struct panfrost_device *dev,
                                const struct pan_image_view *iview, void *out,
                                const struct panfrost_ptr *payload);
#endif

unsigned panfrost_get_layer_stride(const struct pan_image_layout *layout,
                                   unsigned level);

unsigned panfrost_texture_offset(const struct pan_image_layout *layout,
                                 unsigned level, unsigned array_idx,
                                 unsigned surface_idx);

struct pan_pool;
struct pan_jc;

/* DRM modifier helper */

#define drm_is_afbc(mod)                                                       \
   ((mod >> 52) ==                                                             \
    (DRM_FORMAT_MOD_ARM_TYPE_AFBC | (DRM_FORMAT_MOD_VENDOR_ARM << 4)))

struct pan_image_explicit_layout {
   unsigned offset;
   unsigned row_stride;
};

bool
pan_image_layout_init(const struct panfrost_device *dev,
                      struct pan_image_layout *layout,
                      const struct pan_image_explicit_layout *explicit_layout);

unsigned panfrost_get_legacy_stride(const struct pan_image_layout *layout,
                                    unsigned level);

unsigned panfrost_from_legacy_stride(unsigned legacy_stride,
                                     enum pipe_format format,
                                     uint64_t modifier);

struct pan_surface {
   union {
      mali_ptr data;
      struct {
         mali_ptr header;
         mali_ptr body;
      } afbc;
   };
};

void pan_iview_get_surface(const struct pan_image_view *iview, unsigned level,
                           unsigned layer, unsigned sample,
                           struct pan_surface *surf);

#if PAN_ARCH >= 9
enum mali_afbc_compression_mode
GENX(pan_afbc_compression_mode)(enum pipe_format format);
#endif

#ifdef __cplusplus
} /* extern C */
#endif

#endif
