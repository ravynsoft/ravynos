/*
 * Copyright (c) 2022 Intel Corporation
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef INTEL_GENX_STATE_H
#define INTEL_GENX_STATE_H

#ifndef GFX_VERx10
#error This file should only be included by genX files.
#endif

#include <stdbool.h>

#include "dev/intel_device_info.h"
#include "genxml/gen_macros.h"

#ifdef __cplusplus
extern "C" {
#endif

#if GFX_VER >= 7

static inline void
intel_set_ps_dispatch_state(struct GENX(3DSTATE_PS) *ps,
                            const struct intel_device_info *devinfo,
                            const struct brw_wm_prog_data *prog_data,
                            unsigned rasterization_samples,
                            enum brw_wm_msaa_flags msaa_flags)
{
   assert(rasterization_samples != 0);

   bool enable_8  = prog_data->dispatch_8;
   bool enable_16 = prog_data->dispatch_16;
   bool enable_32 = prog_data->dispatch_32;

#if GFX_VER >= 9
   /* SKL PRMs, Volume 2a: Command Reference: Instructions:
    *    3DSTATE_PS_BODY::8 Pixel Dispatch Enable:
    *
    *    "When Render Target Fast Clear Enable is ENABLED or Render Target
    *     Resolve Type = RESOLVE_PARTIAL or RESOLVE_FULL, this bit must be
    *     DISABLED."
    */
   if (ps->RenderTargetFastClearEnable ||
       ps->RenderTargetResolveType == RESOLVE_PARTIAL ||
       ps->RenderTargetResolveType == RESOLVE_FULL)
      enable_8 = false;
#elif GFX_VER >= 8
   /* BDW has the same wording as SKL, except some of the fields mentioned
    * don't exist...
    */
   if (ps->RenderTargetFastClearEnable ||
       ps->RenderTargetResolveEnable)
      enable_8 = false;
#endif

   const bool is_persample_dispatch =
      brw_wm_prog_data_is_persample(prog_data, msaa_flags);

   if (is_persample_dispatch) {
      /* TGL PRMs, Volume 2d: Command Reference: Structures:
       *    3DSTATE_PS_BODY::32 Pixel Dispatch Enable:
       *
       *    "Must not be enabled when dispatch rate is sample AND NUM_MULTISAMPLES > 1."
       */
      if (GFX_VER >= 12 && rasterization_samples > 1)
         enable_32 = false;

      /* Starting with SandyBridge (where we first get MSAA), the different
       * pixel dispatch combinations are grouped into classifications A
       * through F (SNB PRM Vol. 2 Part 1 Section 7.7.1).  On most hardware
       * generations, the only configurations supporting persample dispatch
       * are those in which only one dispatch width is enabled.
       *
       * The Gfx12 hardware spec has a similar dispatch grouping table, but
       * the following conflicting restriction applies (from the page on
       * "Structure_3DSTATE_PS_BODY"), so we need to keep the SIMD16 shader:
       *
       *  "SIMD32 may only be enabled if SIMD16 or (dual)SIMD8 is also
       *   enabled."
       */
      if (enable_32 || enable_16)
         enable_8 = false;
      if (GFX_VER < 12 && enable_32)
         enable_16 = false;
   }

   /* The docs for 3DSTATE_PS::32 Pixel Dispatch Enable say:
    *
    *    "When NUM_MULTISAMPLES = 16 or FORCE_SAMPLE_COUNT = 16,
    *     SIMD32 Dispatch must not be enabled for PER_PIXEL dispatch
    *     mode."
    *
    * 16x MSAA only exists on Gfx9+, so we can skip this on Gfx8.
    */
   if (GFX_VER >= 9 && rasterization_samples == 16 && !is_persample_dispatch) {
      assert(enable_8 || enable_16);
      enable_32 = false;
   }

   assert(enable_8 || enable_16 || enable_32 ||
          (GFX_VER >= 12 && prog_data->dispatch_multi));
   assert(!prog_data->dispatch_multi ||
          (GFX_VER >= 12 && !enable_8));

#if GFX_VER >= 20
   if (prog_data->dispatch_multi) {
      ps->Kernel0Enable = true;
      ps->Kernel0SIMDWidth = (prog_data->dispatch_multi == 32 ?
                              PS_SIMD32 : PS_SIMD16);
      ps->Kernel0MaximumPolysperThread =
         prog_data->max_polygons - 1;
      switch (prog_data->dispatch_multi /
              prog_data->max_polygons) {
      case 8:
         ps->Kernel0PolyPackingPolicy = POLY_PACK8_FIXED;
         break;
      case 16:
         ps->Kernel0PolyPackingPolicy = POLY_PACK16_FIXED;
         break;
      default:
         unreachable("Invalid polygon width");
      }

   } else if (enable_16) {
      ps->Kernel0Enable = true;
      ps->Kernel0SIMDWidth = PS_SIMD16;
      ps->Kernel0PolyPackingPolicy = POLY_PACK16_FIXED;
   }

   if (enable_32) {
      ps->Kernel1Enable = true;
      ps->Kernel1SIMDWidth = PS_SIMD32;

   } else if (enable_16 && prog_data->dispatch_multi == 16) {
      ps->Kernel1Enable = true;
      ps->Kernel1SIMDWidth = PS_SIMD16;
   }
#else
   ps->_8PixelDispatchEnable = enable_8 ||
      (GFX_VER == 12 && prog_data->dispatch_multi);
   ps->_16PixelDispatchEnable = enable_16;
   ps->_32PixelDispatchEnable = enable_32;
#endif
}

#endif

#if GFX_VERx10 >= 125

UNUSED static int
preferred_slm_allocation_size(const struct intel_device_info *devinfo)
{
   if (devinfo->platform == INTEL_PLATFORM_LNL && devinfo->revision == 0)
      return SLM_ENCODES_128K;

   return 0;
}

#endif

#ifdef __cplusplus
}
#endif

#endif /* INTEL_GENX_STATE_H */
