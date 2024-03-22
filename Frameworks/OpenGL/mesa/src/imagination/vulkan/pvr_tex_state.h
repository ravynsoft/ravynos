/*
 * Copyright © 2022 Imagination Technologies Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef PVR_TEX_STATE_H
#define PVR_TEX_STATE_H

#include <stdint.h>
#include <vulkan/vulkan.h>

#include "hwdef/rogue_hw_defs.h"
#include "pvr_private.h"
#include "pvr_types.h"
#include "util/macros.h"

/**
 * Texture requires 32bit index lookups instead of texture coordinate access.
 */
#define PVR_TEXFLAGS_INDEX_LOOKUP BITFIELD_BIT(0U)

/** Texture has border texels present. */
#define PVR_TEXFLAGS_BORDER BITFIELD_BIT(1U)

/**
 * Resource is actually a buffer, not a texture, and therefore LOD is ignored.
 * Coordinates are integers.
 */
#define PVR_TEXFLAGS_BUFFER BITFIELD_BIT(2U)

/** Parameters for #pvr_pack_tex_state(). */
struct pvr_texture_state_info {
   VkFormat format;
   enum pvr_memlayout mem_layout;
   uint32_t flags;
   VkImageViewType type;
   VkImageAspectFlags aspect_mask;
   bool is_cube;
   enum pvr_texture_state tex_state_type;
   VkExtent3D extent;

   /**
    * For array textures, this holds the array dimension, in elements. This can
    * be zero if texture is not an array.
    */
   uint32_t array_size;

   /** Base mipmap level. This is the miplevel you want as the top level. */
   uint32_t base_level;

   /**
    * Number of mipmap levels that should be accessed by HW. This is not
    * necessarily the number of levels that are in memory. (See
    * mipmaps_present)
    */
   uint32_t mip_levels;

   /**
    * True if the texture is mipmapped.
    * Note: This is based on the number of mip levels the texture contains, not
    * on the mip levels that are being used i.e. mip_levels.
    */
   bool mipmaps_present;

   /**
    * Number of samples per texel for multisampling. This should be 1 for none
    * multisampled textures.
    */
   uint32_t sample_count;

   /** Stride, in pixels. Only valid if mem_layout is stride or tiled. */
   uint32_t stride;

   /**
    * For buffers, where TPU_BUFFER_LOOKUP is present, this defines
    * the offset for the buffer, in texels.
    */
   uint32_t offset;

   /**
    * Precomputed (composed from createinfo->components and format swizzle)
    * swizzles to pass in to the texture state.
    */
   uint8_t swizzle[4];

   /** Address of texture, which must be aligned to at least 32bits. */
   pvr_dev_addr_t addr;
};

VkResult
pvr_pack_tex_state(struct pvr_device *device,
                   const struct pvr_texture_state_info *info,
                   uint64_t state[static const ROGUE_NUM_TEXSTATE_IMAGE_WORDS]);

#endif /* PVR_TEX_STATE_H */
