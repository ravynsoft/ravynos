/*
 * Copyright 2017 Intel Corporation
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice (including the next
 *  paragraph) shall be included in all copies or substantial portions of the
 *  Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 */

#include <assert.h>
#include <stdlib.h>

#include "drm-uapi/drm_fourcc.h"
#include "drm-uapi/i915_drm.h"

#include "isl.h"
#include "dev/intel_device_info.h"
#include "dev/intel_debug.h"

uint32_t
isl_tiling_to_i915_tiling(enum isl_tiling tiling)
{
   switch (tiling) {
   case ISL_TILING_LINEAR:
      return I915_TILING_NONE;

   case ISL_TILING_X:
      return I915_TILING_X;

   case ISL_TILING_Y0:
   case ISL_TILING_HIZ:
   case ISL_TILING_CCS:
      return I915_TILING_Y;

   case ISL_TILING_W:
   case ISL_TILING_SKL_Yf:
   case ISL_TILING_SKL_Ys:
   case ISL_TILING_ICL_Yf:
   case ISL_TILING_ICL_Ys:
   case ISL_TILING_4:
   case ISL_TILING_64:
   case ISL_TILING_GFX12_CCS:
      return I915_TILING_NONE;
   }

   unreachable("Invalid ISL tiling");
}

enum isl_tiling
isl_tiling_from_i915_tiling(uint32_t tiling)
{
   switch (tiling) {
   case I915_TILING_NONE:
      return ISL_TILING_LINEAR;

   case I915_TILING_X:
      return ISL_TILING_X;

   case I915_TILING_Y:
      return ISL_TILING_Y0;
   }

   unreachable("Invalid i915 tiling");
}

/** Sentinel is DRM_FORMAT_MOD_INVALID. */
const struct isl_drm_modifier_info
isl_drm_modifier_info_list[] = {
   {
      .modifier = DRM_FORMAT_MOD_NONE,
      .name = "DRM_FORMAT_MOD_NONE",
      .tiling = ISL_TILING_LINEAR,
   },
   {
      .modifier = I915_FORMAT_MOD_X_TILED,
      .name = "I915_FORMAT_MOD_X_TILED",
      .tiling = ISL_TILING_X,
   },
   {
      .modifier = I915_FORMAT_MOD_Y_TILED,
      .name = "I915_FORMAT_MOD_Y_TILED",
      .tiling = ISL_TILING_Y0,
   },
   {
      .modifier = I915_FORMAT_MOD_Y_TILED_CCS,
      .name = "I915_FORMAT_MOD_Y_TILED_CCS",
      .tiling = ISL_TILING_Y0,
      .supports_render_compression = true,
      .supports_clear_color = false,
   },
   {
      .modifier = I915_FORMAT_MOD_Y_TILED_GEN12_RC_CCS,
      .name = "I915_FORMAT_MOD_Y_TILED_GEN12_RC_CCS",
      .tiling = ISL_TILING_Y0,
      .supports_render_compression = true,
      .supports_clear_color = false,
   },
   {
      .modifier = I915_FORMAT_MOD_Y_TILED_GEN12_MC_CCS,
      .name = "I915_FORMAT_MOD_Y_TILED_GEN12_MC_CCS",
      .tiling = ISL_TILING_Y0,
      .supports_media_compression = true,
      .supports_clear_color = false,
   },
   {
      .modifier = I915_FORMAT_MOD_Y_TILED_GEN12_RC_CCS_CC,
      .name = "I915_FORMAT_MOD_Y_TILED_GEN12_RC_CCS_CC",
      .tiling = ISL_TILING_Y0,
      .supports_render_compression = true,
      .supports_clear_color = true,
   },
   {
      .modifier = I915_FORMAT_MOD_4_TILED,
      .name = "I915_FORMAT_MOD_4_TILED",
      .tiling = ISL_TILING_4,
   },
   {
      .modifier = I915_FORMAT_MOD_4_TILED_DG2_RC_CCS,
      .name = "I915_FORMAT_MOD_4_TILED_DG2_RC_CCS",
      .tiling = ISL_TILING_4,
      .supports_render_compression = true,
      .supports_clear_color = false,
   },
   {
      .modifier = I915_FORMAT_MOD_4_TILED_DG2_MC_CCS,
      .name = "I915_FORMAT_MOD_4_TILED_DG2_MC_CCS",
      .tiling = ISL_TILING_4,
      .supports_media_compression = true,
      .supports_clear_color = false,
   },
   {
      .modifier = I915_FORMAT_MOD_4_TILED_DG2_RC_CCS_CC,
      .name = "I915_FORMAT_MOD_4_TILED_DG2_RC_CCS_CC",
      .tiling = ISL_TILING_4,
      .supports_render_compression = true,
      .supports_clear_color = true,
   },
   {
      .modifier = I915_FORMAT_MOD_4_TILED_MTL_RC_CCS,
      .name = "I915_FORMAT_MOD_4_TILED_MTL_RC_CCS",
      .tiling = ISL_TILING_4,
      .supports_render_compression = true,
      .supports_clear_color = false,
   },
   {
      .modifier = I915_FORMAT_MOD_4_TILED_MTL_RC_CCS_CC,
      .name = "I915_FORMAT_MOD_4_TILED_MTL_RC_CCS_CC",
      .tiling = ISL_TILING_4,
      .supports_render_compression = true,
      .supports_clear_color = true,
   },
   {
      .modifier = I915_FORMAT_MOD_4_TILED_MTL_MC_CCS,
      .name = "I915_FORMAT_MOD_4_TILED_MTL_MC_CCS",
      .tiling = ISL_TILING_4,
      .supports_media_compression = true,
      .supports_clear_color = false,
   },
   {
      .modifier = DRM_FORMAT_MOD_INVALID,
   },
};

const struct isl_drm_modifier_info *
isl_drm_modifier_get_info(uint64_t modifier)
{
   isl_drm_modifier_info_for_each(info) {
      if (info->modifier == modifier)
         return info;
   }

   return NULL;
}

uint32_t
isl_drm_modifier_get_score(const struct intel_device_info *devinfo,
                           uint64_t modifier)
{
   /* We want to know the absence of the debug environment variable
    * and don't want to provide a default value either, so we don't
    * use debug_get_num_option() here.
    */
   const char *mod_str = getenv("INTEL_MODIFIER_OVERRIDE");
   if (mod_str != NULL) {
      return modifier == strtoul(mod_str, NULL, 0);
   }
   /* FINISHME: Add gfx12 modifiers */
   switch (modifier) {
   default:
      return 0;
   case DRM_FORMAT_MOD_LINEAR:
      return 1;
   case I915_FORMAT_MOD_X_TILED:
      return 2;
   case I915_FORMAT_MOD_Y_TILED:
      /* Gfx12.5 doesn't have Y-tiling. */
      if (devinfo->verx10 >= 125)
         return 0;

      return 3;
   case I915_FORMAT_MOD_Y_TILED_CCS:
      /* Not supported before Gfx9 and also Gfx12's CCS layout differs from
       * Gfx9-11.
       */
      if (devinfo->ver <= 8 || devinfo->ver >= 12)
         return 0;

      if (INTEL_DEBUG(DEBUG_NO_CCS))
         return 0;

      return 4;
   case I915_FORMAT_MOD_Y_TILED_GEN12_RC_CCS:
      if (devinfo->verx10 != 120)
         return 0;

      if (INTEL_DEBUG(DEBUG_NO_CCS))
         return 0;

      return 4;
   case I915_FORMAT_MOD_Y_TILED_GEN12_RC_CCS_CC:
      if (devinfo->verx10 != 120)
         return 0;

      if (INTEL_DEBUG(DEBUG_NO_CCS) || INTEL_DEBUG(DEBUG_NO_FAST_CLEAR))
         return 0;

      return 5;
   case I915_FORMAT_MOD_4_TILED:
      /* Gfx12.5 introduces Tile4. */
      if (devinfo->verx10 < 125)
         return 0;

      return 3;
   case I915_FORMAT_MOD_4_TILED_DG2_RC_CCS:
      if (!intel_device_info_is_dg2(devinfo))
         return 0;

      if (INTEL_DEBUG(DEBUG_NO_CCS))
         return 0;

      return 4;
   case I915_FORMAT_MOD_4_TILED_DG2_RC_CCS_CC:
      if (!intel_device_info_is_dg2(devinfo))
         return 0;

      if (INTEL_DEBUG(DEBUG_NO_CCS) || INTEL_DEBUG(DEBUG_NO_FAST_CLEAR))
         return 0;

      return 5;
   case I915_FORMAT_MOD_4_TILED_MTL_RC_CCS:
      if (!intel_device_info_is_mtl(devinfo))
         return 0;

      if (INTEL_DEBUG(DEBUG_NO_CCS))
         return 0;

      return 4;
   case I915_FORMAT_MOD_4_TILED_MTL_RC_CCS_CC:
      if (!intel_device_info_is_mtl(devinfo))
         return 0;

      if (INTEL_DEBUG(DEBUG_NO_CCS) || INTEL_DEBUG(DEBUG_NO_FAST_CLEAR))
         return 0;

      return 5;
   }
}

uint32_t
isl_drm_modifier_get_plane_count(const struct intel_device_info *devinfo,
                                 uint64_t modifier,
                                 uint32_t fmt_planes)
{
   /* This function could return the wrong value if the modifier is not
    * supported by the device.
    */
   assert(isl_drm_modifier_get_score(devinfo, modifier) > 0);

   /* Planar images don't support clear color. */
   if (isl_drm_modifier_get_info(modifier)->supports_clear_color)
      assert(fmt_planes == 1);

   if (devinfo->has_flat_ccs) {
      if (isl_drm_modifier_get_info(modifier)->supports_clear_color)
         return 2 * fmt_planes;
      else
         return 1 * fmt_planes;
   } else {
      if (isl_drm_modifier_get_info(modifier)->supports_clear_color)
         return 3 * fmt_planes;
      else if (isl_drm_modifier_has_aux(modifier))
         return 2 * fmt_planes;
      else
         return 1 * fmt_planes;
   }
}
