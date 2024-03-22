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

#ifndef PVR_JOB_COMMON_H
#define PVR_JOB_COMMON_H

#include <stdbool.h>
#include <stdint.h>
#include <vulkan/vulkan.h>

#include "hwdef/rogue_hw_defs.h"
#include "pvr_csb_enum_helpers.h"
#include "pvr_private.h"
#include "pvr_types.h"

enum pvr_pbe_gamma {
   PVR_PBE_GAMMA_NONE,
   /* For two-channel pack formats. */
   PVR_PBE_GAMMA_RED,
   PVR_PBE_GAMMA_REDGREEN,
   /* For all other pack formats. */
   PVR_PBE_GAMMA_ENABLED,
};

/**
 * These are parameters specific to the surface being set up and hence can be
 * typically set up at surface creation time.
 */
struct pvr_pbe_surf_params {
   /* Swizzle for a format can be retrieved using pvr_get_format_swizzle(). */
   uint8_t swizzle[4];
   /* is_normalized can be retrieved using vk_format_is_normalized(). */
   bool is_normalized;
   /* pbe_packmode can be retrieved using pvr_get_pbe_packmode(). */
   uint32_t pbe_packmode;
   /* source_format and gamma can be retrieved using
    * pvr_pbe_get_src_format_and_gamma().
    */
   uint32_t source_format;
   enum pvr_pbe_gamma gamma;
   /* nr_components can be retrieved using vk_format_get_nr_components(). */
   uint32_t nr_components;

   /* When an RT of MRT is packed using less USC outputs, this flag needs to be
    * setup to true.
    *
    * Currently, this flag is only considered when has_usc_f16_sop is enabled.
    * And it needs to be true when a render target by default should use F16
    * USC channel but uses U8 channel instead for squeezing into on-chip MRT.
    *
    * It is better to make this member with FF_ACCUMFORMAT type or, at least,
    * describe USC channel size. But for now, only use this flag for
    * simplicity.
    */

   pvr_dev_addr_t addr;
   enum pvr_memlayout mem_layout;
   uint32_t stride;

   /* Depth size for renders */
   uint32_t depth;

   /* Pre-rotation dimensions of surface */
   uint32_t width;
   uint32_t height;

   bool z_only_render;
   bool down_scale;
};

/**
 * These parameters are generally render-specific and need to be set up at the
 * time #pvr_pbe_pack_state() is called.
 */
struct pvr_pbe_render_params {
   /* Clipping params are in terms of pixels and are inclusive. */
   uint32_t min_x_clip;
   uint32_t max_x_clip;

   uint32_t min_y_clip;
   uint32_t max_y_clip;

   /* Start position of pixels to be read within 128bit USC output buffer. */
   enum pvr_pbe_source_start_pos source_start;

   /* 9-bit slice number to be used when memlayout is 3D twiddle. */
   uint32_t slice;

   /* Index */
   uint32_t mrt_index;
};

void pvr_pbe_pack_state(
   const struct pvr_device_info *dev_info,
   const struct pvr_pbe_surf_params *surface_params,
   const struct pvr_pbe_render_params *render_params,
   uint32_t pbe_cs_words[static const ROGUE_NUM_PBESTATE_STATE_WORDS],
   uint64_t pbe_reg_words[static const ROGUE_NUM_PBESTATE_REG_WORDS]);

/* Helper to calculate pvr_pbe_surf_params::gamma and
 * pvr_pbe_surf_params::source_format.
 */
void pvr_pbe_get_src_format_and_gamma(VkFormat vk_format,
                                      enum pvr_pbe_gamma default_gamma,
                                      bool with_packed_usc_channel,
                                      uint32_t *const src_format_out,
                                      enum pvr_pbe_gamma *const gamma_out);

void pvr_setup_tiles_in_flight(
   const struct pvr_device_info *dev_info,
   const struct pvr_device_runtime_info *dev_runtime_info,
   uint32_t msaa_mode,
   uint32_t pixel_width,
   bool paired_tiles,
   uint32_t max_tiles_in_flight,
   uint32_t *const isp_ctl_out,
   uint32_t *const pixel_ctl_out);

#endif /* PVR_JOB_COMMON_H */
