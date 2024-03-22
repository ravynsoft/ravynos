/*
 * Copyright 2015 Intel Corporation
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
#include <stdarg.h>
#include <stdio.h>
#include <inttypes.h>

#include "dev/intel_debug.h"
#include "genxml/genX_bits.h"
#include "util/log.h"

#include "isl.h"
#include "isl_gfx4.h"
#include "isl_gfx6.h"
#include "isl_gfx7.h"
#include "isl_gfx8.h"
#include "isl_gfx9.h"
#include "isl_gfx12.h"
#include "isl_priv.h"

isl_genX_declare_get_func(surf_fill_state_s)
isl_genX_declare_get_func(buffer_fill_state_s)
isl_genX_declare_get_func(emit_depth_stencil_hiz_s)
isl_genX_declare_get_func(null_fill_state_s)
isl_genX_declare_get_func(emit_cpb_control_s)

void
isl_memcpy_linear_to_tiled(uint32_t xt1, uint32_t xt2,
                           uint32_t yt1, uint32_t yt2,
                           char *dst, const char *src,
                           uint32_t dst_pitch, int32_t src_pitch,
                           bool has_swizzling,
                           enum isl_tiling tiling,
                           isl_memcpy_type copy_type)
{
#ifdef USE_SSE41
   if (copy_type == ISL_MEMCPY_STREAMING_LOAD) {
      _isl_memcpy_linear_to_tiled_sse41(
         xt1, xt2, yt1, yt2, dst, src, dst_pitch, src_pitch, has_swizzling,
         tiling, copy_type);
      return;
   }
#endif

   _isl_memcpy_linear_to_tiled(
      xt1, xt2, yt1, yt2, dst, src, dst_pitch, src_pitch, has_swizzling,
      tiling, copy_type);
}

void
isl_memcpy_tiled_to_linear(uint32_t xt1, uint32_t xt2,
                           uint32_t yt1, uint32_t yt2,
                           char *dst, const char *src,
                           int32_t dst_pitch, uint32_t src_pitch,
                           bool has_swizzling,
                           enum isl_tiling tiling,
                           isl_memcpy_type copy_type)
{
#ifdef USE_SSE41
   if (copy_type == ISL_MEMCPY_STREAMING_LOAD) {
      _isl_memcpy_tiled_to_linear_sse41(
         xt1, xt2, yt1, yt2, dst, src, dst_pitch, src_pitch, has_swizzling,
         tiling, copy_type);
      return;
   }
#endif

   _isl_memcpy_tiled_to_linear(
      xt1, xt2, yt1, yt2, dst, src, dst_pitch, src_pitch, has_swizzling,
      tiling, copy_type);
}

void PRINTFLIKE(3, 4) UNUSED
__isl_finishme(const char *file, int line, const char *fmt, ...)
{
   va_list ap;
   char buf[512];

   va_start(ap, fmt);
   vsnprintf(buf, sizeof(buf), fmt, ap);
   va_end(ap);

   fprintf(stderr, "%s:%d: FINISHME: %s\n", file, line, buf);
}

static void
isl_device_setup_mocs(struct isl_device *dev)
{
   dev->mocs.protected_mask = 0;

   if (dev->info->ver >= 20) {
      /* L3+L4=WB; BSpec: 71582 */
      dev->mocs.internal = 1 << 1;
      dev->mocs.external = 1 << 1;
      dev->mocs.protected_mask = 3 << 0;
      /* TODO: Setting to uncached
       * WA 14018443005:
       *  Ensure that any compression-enabled resource from gfx memory subject
       *  to app recycling (e.g. OGL sparse resource backing memory or
       *  Vulkan heaps) is never PAT/MOCS'ed as L3:UC.
       */
      dev->mocs.blitter_dst = 1 << 1;
      dev->mocs.blitter_src = 1 << 1;
   } else if (dev->info->ver >= 12) {
      if (intel_device_info_is_mtl(dev->info)) {
         /* Cached L3+L4; BSpec: 45101 */
         dev->mocs.internal = 1 << 1;
         /* Displayables cached to L3+L4:WT */
         dev->mocs.external = 14 << 1;
         /* Uncached - GO:Mem */
         dev->mocs.uncached = 5 << 1;
         /* TODO: XY_BLOCK_COPY_BLT don't mention what should be the L4 cache
          * mode so for now it is setting L4 as uncached following what is
          * asked for L3
          */
         dev->mocs.blitter_dst = 9 << 1;
         dev->mocs.blitter_src = 9 << 1;
      } else if (intel_device_info_is_dg2(dev->info)) {
         /* L3CC=WB; BSpec: 45101 */
         dev->mocs.internal = 3 << 1;
         dev->mocs.external = 3 << 1;
         /* UC - Coherent; GO:Memory */
         dev->mocs.uncached = 1 << 1;

         /* XY_BLOCK_COPY_BLT MOCS fields have programming notes which say:
          *
          *    "Destination MOCS value, which is used to program MOCS index
          *     for writing to memory, should select a MOCS register having
          *     "L3 Cacheability Control" programmed as uncacheable(UC) and
          *     "Global GO" parameter set as GOMemory (pushes GO point to
          *     memory). The MOCS Register may have L3 Lookup programmed as
          *     UCL3LKDIS for better efficiency."
          *
          * The GO:Memory setting requires us to use MOCS 1 or 2.  MOCS 2
          * has LKUP set to 0 and is marked "Non-Coherent", which we assume
          * is probably the "better efficiency" they mention...
          *
          *   "Source MOCS value, which is used to program MOCS index for
          *    reading from memory, should select a MOCS register having
          *    "L3 Cacheability Control" programmed as uncacheable(UC).
          *    The MOCS Register may have L3 Lookup programmed as UCL3LKDIS
          *    for better efficiency."
          *
          * Any MOCS except 3 should work.  We use MOCS 2...
          */
         dev->mocs.blitter_dst = 2 << 1;
         dev->mocs.blitter_src = 2 << 1;
      } else if (dev->info->platform == INTEL_PLATFORM_DG1) {
         /* L3CC=WB */
         dev->mocs.internal = 5 << 1;
         /* Displayables on DG1 are free to cache in L3 since L3 is transient
          * and flushed at bottom of each submission.
          */
         dev->mocs.external = 5 << 1;
         /* UC */
         dev->mocs.uncached = 1 << 1;
      } else {
         /* TC=1/LLC Only, LeCC=1/UC, LRUM=0, L3CC=3/WB */
         dev->mocs.external = 61 << 1;
         /* TC=LLC/eLLC, LeCC=WB, LRUM=3, L3CC=WB */
         dev->mocs.internal = 2 << 1;
         /* Uncached */
         dev->mocs.uncached = 3 << 1;

         /* L1 - HDC:L1 + L3 + LLC */
         dev->mocs.l1_hdc_l3_llc = 48 << 1;
      }
      /* Protected is just an additional flag. */
      dev->mocs.protected_mask = 1 << 0;
   } else if (dev->info->ver >= 9) {
      /* TC=LLC/eLLC, LeCC=PTE, LRUM=3, L3CC=WB */
      dev->mocs.external = 1 << 1;
      /* TC=LLC/eLLC, LeCC=WB, LRUM=3, L3CC=WB */
      dev->mocs.internal = 2 << 1;
      /* Uncached */
      dev->mocs.uncached = (dev->info->ver >= 11 ? 3 : 0) << 1;
   } else if (dev->info->ver >= 8) {
      /* MEMORY_OBJECT_CONTROL_STATE:
       * .MemoryTypeLLCeLLCCacheabilityControl = UCwithFenceifcoherentcycle,
       * .TargetCache = L3DefertoPATforLLCeLLCselection,
       * .AgeforQUADLRU = 0
       */
      dev->mocs.external = 0x18;
      /* MEMORY_OBJECT_CONTROL_STATE:
       * .MemoryTypeLLCeLLCCacheabilityControl = WB,
       * .TargetCache = L3DefertoPATforLLCeLLCselection,
       * .AgeforQUADLRU = 0
       */
      dev->mocs.internal = 0x78;
      if (dev->info->platform == INTEL_PLATFORM_CHV) {
         /* MEMORY_OBJECT_CONTROL_STATE:
          * .MemoryType = UC,
          * .TargetCache = NoCaching,
          */
         dev->mocs.uncached = 0;
      } else {
         /* MEMORY_OBJECT_CONTROL_STATE:
          * .MemoryTypeLLCeLLCCacheabilityControl = UCUncacheable,
          * .TargetCache = eLLCOnlywheneDRAMispresentelsegetsallocatedinLLC,
          * .AgeforQUADLRU = 0
          */
         dev->mocs.uncached = 0x20;
      }
   } else if (dev->info->ver >= 7) {
      if (dev->info->platform == INTEL_PLATFORM_HSW) {
         /* MEMORY_OBJECT_CONTROL_STATE:
          * .LLCeLLCCacheabilityControlLLCCC             = 0,
          * .L3CacheabilityControlL3CC                   = 1,
          */
         dev->mocs.internal = 1;
         dev->mocs.external = 1;
         /* MEMORY_OBJECT_CONTROL_STATE:
          * .LLCeLLCCacheabilityControlLLCCC             = 1,
          * .L3CacheabilityControlL3CC                   = 0,
          */
         dev->mocs.uncached = 2;
      } else {
         /* MEMORY_OBJECT_CONTROL_STATE:
          * .GraphicsDataTypeGFDT                        = 0,
          * .LLCCacheabilityControlLLCCC                 = 0,
          * .L3CacheabilityControlL3CC                   = 1,
          */
         dev->mocs.internal = 1;
         dev->mocs.external = 1;
         /* MEMORY_OBJECT_CONTROL_STATE:
          * .GraphicsDataTypeGFDT                        = 0,
          * .LLCCacheabilityControlLLCCC                 = 0,
          * .L3CacheabilityControlL3CC                   = 0,
          */
         dev->mocs.uncached = 0;
      }
   } else {
      dev->mocs.internal = 0;
      dev->mocs.external = 0;
      dev->mocs.uncached = 0;
   }
}

/**
 * Return an appropriate MOCS entry for the given usage flags.
 */
uint32_t
isl_mocs(const struct isl_device *dev, isl_surf_usage_flags_t usage,
         bool external)
{
   uint32_t mask = (usage & ISL_SURF_USAGE_PROTECTED_BIT) ?
      dev->mocs.protected_mask : 0;

   if (usage & ISL_SURF_USAGE_BLITTER_SRC_BIT)
      return dev->mocs.blitter_src | mask;

   if (usage & ISL_SURF_USAGE_BLITTER_DST_BIT)
      return dev->mocs.blitter_dst | mask;

   if (external)
      return dev->mocs.external | mask;

   if (intel_device_info_is_mtl(dev->info) &&
       (usage & ISL_SURF_USAGE_STREAM_OUT_BIT))
      return dev->mocs.uncached | mask;

   if (dev->info->verx10 == 120 && dev->info->platform != INTEL_PLATFORM_DG1) {
      if (usage & ISL_SURF_USAGE_STAGING_BIT)
         return dev->mocs.internal | mask;

      if (usage & ISL_SURF_USAGE_CPB_BIT)
         return dev->mocs.internal | mask;

      /* Using L1:HDC for storage buffers breaks Vulkan memory model
       * tests that use shader atomics.  This isn't likely to work out,
       * and we can't know a priori whether they'll be used.  So just
       * continue with ordinary internal MOCS for now.
       */
      if (usage & ISL_SURF_USAGE_STORAGE_BIT)
         return dev->mocs.internal | mask;

      if (usage & (ISL_SURF_USAGE_CONSTANT_BUFFER_BIT |
                   ISL_SURF_USAGE_RENDER_TARGET_BIT |
                   ISL_SURF_USAGE_TEXTURE_BIT))
         return dev->mocs.l1_hdc_l3_llc | mask;
   }

   return dev->mocs.internal | mask;
}

void
isl_device_init(struct isl_device *dev,
                const struct intel_device_info *info)
{
   /* Gfx8+ don't have bit6 swizzling, ensure callsite is not confused. */
   assert(!(info->has_bit6_swizzle && info->ver >= 8));

   dev->info = info;
   dev->use_separate_stencil = ISL_GFX_VER(dev) >= 6;
   dev->has_bit6_swizzling = info->has_bit6_swizzle;
   dev->buffer_length_in_aux_addr = false;

   /* The ISL_DEV macros may be defined in the CFLAGS, thus hardcoding some
    * device properties at buildtime. Verify that the macros with the device
    * properties chosen during runtime.
    */
   ISL_GFX_VER_SANITIZE(dev);
   ISL_DEV_USE_SEPARATE_STENCIL_SANITIZE(dev);

   /* Did we break hiz or stencil? */
   if (ISL_DEV_USE_SEPARATE_STENCIL(dev))
      assert(info->has_hiz_and_separate_stencil);
   if (info->must_use_separate_stencil)
      assert(ISL_DEV_USE_SEPARATE_STENCIL(dev));

   dev->ss.size = RENDER_SURFACE_STATE_length(info) * 4;
   dev->ss.align = isl_align(dev->ss.size, 32);

   dev->ss.clear_color_state_size = CLEAR_COLOR_length(info) * 4;
   dev->ss.clear_color_state_offset =
      RENDER_SURFACE_STATE_ClearValueAddress_start(info) / 32 * 4;

   dev->ss.clear_value_size =
      isl_align(RENDER_SURFACE_STATE_RedClearColor_bits(info) +
                RENDER_SURFACE_STATE_GreenClearColor_bits(info) +
                RENDER_SURFACE_STATE_BlueClearColor_bits(info) +
                RENDER_SURFACE_STATE_AlphaClearColor_bits(info), 32) / 8;

   dev->ss.clear_value_offset =
      RENDER_SURFACE_STATE_RedClearColor_start(info) / 32 * 4;

   assert(RENDER_SURFACE_STATE_SurfaceBaseAddress_start(info) % 8 == 0);
   dev->ss.addr_offset =
      RENDER_SURFACE_STATE_SurfaceBaseAddress_start(info) / 8;

   /* The "Auxiliary Surface Base Address" field starts a bit higher up
    * because the bottom 12 bits are used for other things.  Round down to
    * the nearest dword before.
    */
   dev->ss.aux_addr_offset =
      (RENDER_SURFACE_STATE_AuxiliarySurfaceBaseAddress_start(info) & ~31) / 8;

   dev->ds.size = _3DSTATE_DEPTH_BUFFER_length(info) * 4;
   assert(_3DSTATE_DEPTH_BUFFER_SurfaceBaseAddress_start(info) % 8 == 0);
   dev->ds.depth_offset =
      _3DSTATE_DEPTH_BUFFER_SurfaceBaseAddress_start(info) / 8;

   if (dev->use_separate_stencil) {
      dev->ds.size += _3DSTATE_STENCIL_BUFFER_length(info) * 4 +
                      _3DSTATE_HIER_DEPTH_BUFFER_length(info) * 4 +
                      _3DSTATE_CLEAR_PARAMS_length(info) * 4;

      assert(_3DSTATE_STENCIL_BUFFER_SurfaceBaseAddress_start(info) % 8 == 0);
      dev->ds.stencil_offset =
         _3DSTATE_DEPTH_BUFFER_length(info) * 4 +
         _3DSTATE_STENCIL_BUFFER_SurfaceBaseAddress_start(info) / 8;

      assert(_3DSTATE_HIER_DEPTH_BUFFER_SurfaceBaseAddress_start(info) % 8 == 0);
      dev->ds.hiz_offset =
         _3DSTATE_DEPTH_BUFFER_length(info) * 4 +
         _3DSTATE_STENCIL_BUFFER_length(info) * 4 +
         _3DSTATE_HIER_DEPTH_BUFFER_SurfaceBaseAddress_start(info) / 8;
   } else {
      dev->ds.stencil_offset = 0;
      dev->ds.hiz_offset = 0;
   }

   /* From the IVB PRM, SURFACE_STATE::Height,
    *
    *    For typed buffer and structured buffer surfaces, the number
    *    of entries in the buffer ranges from 1 to 2^27. For raw buffer
    *    surfaces, the number of entries in the buffer is the number of bytes
    *    which can range from 1 to 2^30.
    *
    * From the SKL PRM, SURFACE_STATE::Width/Height/Depth for RAW buffers,
    *
    *    Width  : bits [6:0]
    *    Height : bits [20:7]
    *    Depth  : bits [31:21]
    *
    *    So we can address 4Gb
    *
    * This limit is only concerned with raw buffers.
    */
   if (ISL_GFX_VER(dev) >= 9) {
      dev->max_buffer_size = 1ull << 32;
   } else if (ISL_GFX_VER(dev) >= 7) {
      dev->max_buffer_size = 1ull << 30;
   } else {
      dev->max_buffer_size = 1ull << 27;
   }

   dev->cpb.size = _3DSTATE_CPSIZE_CONTROL_BUFFER_length(info) * 4;
   dev->cpb.offset =
      _3DSTATE_CPSIZE_CONTROL_BUFFER_SurfaceBaseAddress_start(info) / 8;

   isl_device_setup_mocs(dev);

   dev->surf_fill_state_s = isl_surf_fill_state_s_get_func(dev);
   dev->buffer_fill_state_s = isl_buffer_fill_state_s_get_func(dev);
   dev->emit_depth_stencil_hiz_s = isl_emit_depth_stencil_hiz_s_get_func(dev);
   dev->null_fill_state_s = isl_null_fill_state_s_get_func(dev);
   dev->emit_cpb_control_s = isl_emit_cpb_control_s_get_func(dev);
}

/**
 * @brief Query the set of multisamples supported by the device.
 *
 * This function always returns non-zero, as ISL_SAMPLE_COUNT_1_BIT is always
 * supported.
 */
isl_sample_count_mask_t ATTRIBUTE_CONST
isl_device_get_sample_counts(const struct isl_device *dev)
{
   if (ISL_GFX_VER(dev) >= 9) {
      return ISL_SAMPLE_COUNT_1_BIT |
             ISL_SAMPLE_COUNT_2_BIT |
             ISL_SAMPLE_COUNT_4_BIT |
             ISL_SAMPLE_COUNT_8_BIT |
             ISL_SAMPLE_COUNT_16_BIT;
   } else if (ISL_GFX_VER(dev) >= 8) {
      return ISL_SAMPLE_COUNT_1_BIT |
             ISL_SAMPLE_COUNT_2_BIT |
             ISL_SAMPLE_COUNT_4_BIT |
             ISL_SAMPLE_COUNT_8_BIT;
   } else if (ISL_GFX_VER(dev) >= 7) {
      return ISL_SAMPLE_COUNT_1_BIT |
             ISL_SAMPLE_COUNT_4_BIT |
             ISL_SAMPLE_COUNT_8_BIT;
   } else if (ISL_GFX_VER(dev) >= 6) {
      return ISL_SAMPLE_COUNT_1_BIT |
             ISL_SAMPLE_COUNT_4_BIT;
   } else {
      return ISL_SAMPLE_COUNT_1_BIT;
   }
}

static uint32_t
isl_get_miptail_base_row(enum isl_tiling tiling)
{
   /* Miptails base levels can depend on the number of samples, but since we
    * don't support levels > 1 with multisampling, the base miptail level is
    * really simple :
    */
   if (tiling == ISL_TILING_SKL_Yf ||
       tiling == ISL_TILING_ICL_Yf)
      return 4;
   else
      return 0;
}

static const uint8_t skl_std_y_2d_miptail_offset_el[][5][2] = {
/*   128 bpb    64 bpb    32 bpb    16 bpb      8 bpb     */
   { {32,  0}, {64,  0}, {64,  0}, {128,  0}, {128,  0} },
   { { 0, 32}, { 0, 32}, { 0, 64}, {  0, 64}, {  0,128} },
   { {16,  0}, {32,  0}, {32,  0}, { 64,  0}, { 64,  0} },
   { { 0, 16}, { 0, 16}, { 0, 32}, {  0, 32}, {  0, 64} },
   { { 8,  0}, {16,  0}, {16,  0}, { 32,  0}, { 32,  0} },
   { { 4,  8}, { 8,  8}, { 8, 16}, { 16, 16}, { 16, 32} },
   { { 0, 12}, { 0, 12}, { 0, 24}, {  0, 24}, {  0, 48} },
   { { 0,  8}, { 0,  8}, { 0, 16}, {  0, 16}, {  0, 32} },
   { { 4,  4}, { 8,  4}, { 8,  8}, { 16,  8}, { 16, 16} },
   { { 4,  0}, { 8,  0}, { 8,  0}, { 16,  0}, { 16,  0} },
   { { 0,  4}, { 0,  4}, { 0,  8}, {  0,  8}, {  0, 16} },
   { { 3,  0}, { 6,  0}, { 4,  4}, {  8,  4}, {  0, 12} },
   { { 2,  0}, { 4,  0}, { 4,  0}, {  8,  0}, {  0,  8} },
   { { 1,  0}, { 2,  0}, { 0,  4}, {  0,  4}, {  0,  4} },
   { { 0,  0}, { 0,  0}, { 0,  0}, {  0,  0}, {  0,  0} },
};

static const uint8_t icl_std_y_2d_miptail_offset_el[][5][2] = {
/*   128 bpb    64 bpb    32 bpb    16 bpb      8 bpb     */
   { {32,  0}, {64,  0}, {64,  0}, {128,  0}, {128,   0} },
   { { 0, 32}, { 0, 32}, { 0, 64}, {  0, 64}, {  0, 128} },
   { {16,  0}, {32,  0}, {32,  0}, { 64,  0}, { 64,   0} },
   { { 0, 16}, { 0, 16}, { 0, 32}, {  0, 32}, {  0,  64} },
   { { 8,  0}, {16,  0}, {16,  0}, { 32,  0}, { 32,   0} },
   { { 4,  8}, { 8,  8}, { 8, 16}, { 16, 16}, { 16,  32} },
   { { 0, 12}, { 0, 12}, { 0, 24}, {  0, 24}, {  0,  48} },
   { { 0,  8}, { 0,  8}, { 0, 16}, {  0, 16}, {  0,  32} },
   { { 4,  4}, { 8,  4}, { 8,  8}, { 16,  8}, { 16,  16} },
   { { 4,  0}, { 8,  0}, { 8,  0}, { 16,  0}, { 16,   0} },
   { { 0,  4}, { 0,  4}, { 0,  8}, {  0,  8}, {  0,  16} },
   { { 0,  0}, { 0,  0}, { 0,  0}, {  0,  0}, {  0,   0} },
   { { 1,  0}, { 2,  0}, { 0,  4}, {  0,  4}, {  0,   4} },
   { { 2,  0}, { 4,  0}, { 4,  0}, {  8,  0}, {  0,   8} },
   { { 3,  0}, { 6,  0}, { 4,  4}, {  8,  4}, {  0,  12} },
};

static const uint8_t skl_std_y_3d_miptail_offset_el[][5][3] = {
/*    128 bpb     64 bpb      32 bpb        16 bpb        8 bpb      */
   { {8, 0, 0}, {16, 0, 0}, {16,  0, 0}, {16,  0,  0}, {32,  0,  0} },
   { {0, 8, 0}, { 0, 8, 0}, { 0, 16, 0}, { 0, 16,  0}, { 0, 16,  0} },
   { {0, 0, 8}, { 0, 0, 8}, { 0,  0, 8}, { 0,  0, 16}, { 0,  0, 16} },
   { {4, 0, 0}, { 8, 0, 0}, { 8,  0, 0}, { 8,  0,  0}, {16,  0,  0} },
   { {0, 4, 0}, { 0, 4, 0}, { 0,  8, 0}, { 0,  8,  0}, { 0,  8,  0} },
   { {0, 0, 4}, { 0, 0, 4}, { 0,  0, 4}, { 0,  0,  8}, { 0,  0,  8} },
   { {3, 0, 0}, { 6, 0, 0}, { 4,  4, 0}, { 0,  4,  4}, { 0,  4,  4} },
   { {2, 0, 0}, { 4, 0, 0}, { 0,  4, 0}, { 0,  4,  0}, { 0,  4,  0} },
   { {1, 0, 3}, { 2, 0, 3}, { 4,  0, 3}, { 0,  0,  7}, { 0,  0,  7} },
   { {1, 0, 2}, { 2, 0, 2}, { 4,  0, 2}, { 0,  0,  6}, { 0,  0,  6} },
   { {1, 0, 1}, { 2, 0, 1}, { 4,  0, 1}, { 0,  0,  5}, { 0,  0,  5} },
   { {1, 0, 0}, { 2, 0, 0}, { 4,  0, 0}, { 0,  0,  4}, { 0,  0,  4} },
   { {0, 0, 3}, { 0, 0, 3}, { 0,  0, 3}, { 0,  0,  3}, { 0,  0,  3} },
   { {0, 0, 2}, { 0, 0, 2}, { 0,  0, 2}, { 0,  0,  2}, { 0,  0,  2} },
   { {0, 0, 1}, { 0, 0, 1}, { 0,  0, 1}, { 0,  0,  1}, { 0,  0,  1} },
   { {0, 0, 0}, { 0, 0, 0}, { 0,  0, 0}, { 0,  0,  0}, { 0,  0,  0} },
};

static const uint8_t icl_std_y_3d_miptail_offset_el[][5][3] = {
/*    128 bpb     64 bpb      32 bpb        16 bpb        8 bpb      */
   { {8, 0, 0}, {16, 0, 0}, {16,  0, 0}, {16,  0,  0}, {32,  0,  0} },
   { {0, 8, 0}, { 0, 8, 0}, { 0, 16, 0}, { 0, 16,  0}, { 0, 16,  0} },
   { {0, 0, 8}, { 0, 0, 8}, { 0,  0, 8}, { 0,  0, 16}, { 0,  0, 16} },
   { {4, 0, 0}, { 8, 0, 0}, { 8,  0, 0}, { 8,  0,  0}, {16,  0,  0} },
   { {0, 4, 0}, { 0, 4, 0}, { 0,  8, 0}, { 0,  8,  0}, { 0,  8,  0} },
   { {2, 0, 4}, { 4, 0, 4}, { 4,  0, 4}, { 4,  0,  8}, { 8,  0,  8} },
   { {0, 2, 4}, { 0, 2, 4}, { 0,  4, 4}, { 0,  4,  8}, { 0,  4,  8} },
   { {0, 0, 4}, { 0, 0, 4}, { 0,  0, 4}, { 0,  0,  8}, { 0,  0,  8} },
   { {2, 2, 0}, { 4, 2, 0}, { 4,  4, 0}, { 4,  4,  0}, { 8,  4,  0} },
   { {2, 0, 0}, { 4, 0, 0}, { 4,  0, 0}, { 4,  0,  0}, { 8,  0,  0} },
   { {0, 2, 0}, { 0, 2, 0}, { 0,  4, 0}, { 0,  4,  0}, { 0,  4,  0} },
   { {1, 0, 2}, { 2, 0, 2}, { 2,  0, 2}, { 2,  0,  4}, { 4,  0,  4} },
   { {0, 0, 2}, { 0, 0, 2}, { 0,  0, 2}, { 0,  0,  4}, { 0,  0,  4} },
   { {1, 0, 0}, { 2, 0, 0}, { 2,  0, 0}, { 2,  0,  0}, { 4,  0,  0} },
   { {0, 0, 0}, { 0, 0, 0}, { 0,  0, 0}, { 0,  0,  0}, { 0,  0,  0} },
};

static const uint8_t acm_tile64_3d_miptail_offset_el[][5][3] = {
/*    128 bpb     64 bpb      32 bpb        16 bpb        8 bpb      */
   { {8, 0, 0}, {16, 0, 0}, {16,  0, 0}, {16,  0,  0}, {32,  0,  0}, },
   { {0, 8, 0}, { 0, 8, 0}, { 0, 16, 0}, { 0, 16,  0}, { 0, 16,  0}, },
   { {0, 0, 8}, { 0, 0, 8}, { 0,  0, 8}, { 0,  0, 16}, { 0,  0, 16}, },
   { {4, 0, 0}, { 8, 0, 0}, { 8,  0, 0}, { 8,  0,  0}, {16,  0,  0}, },
   { {0, 4, 0}, { 0, 4, 0}, { 0,  8, 0}, { 0,  8,  0}, { 0,  8,  0}, },
   { {2, 0, 4}, { 4, 0, 4}, { 4,  0, 4}, { 0,  4,  8}, { 0,  4,  8}, },
   { {1, 0, 4}, { 2, 0, 4}, { 0,  4, 4}, { 0,  0, 12}, { 0,  0, 12}, },
   { {0, 0, 4}, { 0, 0, 4}, { 0,  0, 4}, { 0,  0,  8}, { 0,  0,  8}, },
   { {3, 0, 0}, { 6, 0, 0}, { 4,  4, 0}, { 0,  4,  4}, { 0,  4,  4}, },
   { {2, 0, 0}, { 4, 0, 0}, { 4,  0, 0}, { 0,  4,  0}, { 0,  4,  0}, },
   { {1, 0, 0}, { 2, 0, 0}, { 0,  4, 0}, { 0,  0,  4}, { 0,  0,  4}, },
   { {0, 0, 0}, { 0, 0, 0}, { 0,  0, 0}, { 0,  0,  0}, { 0,  0,  0}, },
   { {0, 0, 1}, { 0, 0, 1}, { 0,  0, 1}, { 0,  0,  1}, { 0,  0,  1}, },
   { {0, 0, 2}, { 0, 0, 2}, { 0,  0, 2}, { 0,  0,  2}, { 0,  0,  2}, },
   { {0, 0, 3}, { 0, 0, 3}, { 0,  0, 3}, { 0,  0,  3}, { 0,  0,  3}, },
};

static uint32_t
tiling_max_mip_tail(enum isl_tiling tiling,
                    enum isl_surf_dim dim,
                    uint32_t samples)
{
   /* In theory, miptails work for multisampled images, but we don't support
    * mipmapped multisampling.
    */
   if (samples > 1)
      return 0;

   int num_2d_table_rows;
   int num_3d_table_rows;

   switch (tiling) {
   case ISL_TILING_LINEAR:
   case ISL_TILING_X:
   case ISL_TILING_Y0:
   case ISL_TILING_4:
   case ISL_TILING_W:
   case ISL_TILING_HIZ:
   case ISL_TILING_CCS:
   case ISL_TILING_GFX12_CCS:
      /* There is no miptail for those tilings */
      return 0;

   case ISL_TILING_SKL_Yf:
   case ISL_TILING_SKL_Ys:
      /* SKL PRMs, Volume 5: Memory Views :
       *
       * Given by the last row of the table in the following sections:
       *
       *    - Tiling and Mip Tail for 1D Surfaces
       *    - Tiling and Mip Tail for 2D Surfaces
       *    - Tiling and Mip Tail for 3D Surfaces
       */
      num_2d_table_rows = ARRAY_SIZE(skl_std_y_2d_miptail_offset_el);
      num_3d_table_rows = ARRAY_SIZE(skl_std_y_3d_miptail_offset_el);
      break;

   case ISL_TILING_ICL_Yf:
   case ISL_TILING_ICL_Ys:
      /* ICL PRMs, Volume 5: Memory Views :
       *
       *    - Tiling and Mip Tail for 1D Surfaces :
       *        "There is no MIP Tail allowed for 1D surfaces because they are
       *         not allowed to be tiled. They must be declared as linear."
       *    - Tiling and Mip Tail for 2D Surfaces
       *    - Tiling and Mip Tail for 3D Surfaces
       */
      num_2d_table_rows = ARRAY_SIZE(icl_std_y_2d_miptail_offset_el);
      num_3d_table_rows = ARRAY_SIZE(icl_std_y_3d_miptail_offset_el);
      break;

   case ISL_TILING_64:
      /* ATS-M PRMS, Volume 5: Memory Data Formats :
       *
       *    - Tiling and Mip Tail for 1D Surfaces :
       *       "There is no MIP Tail allowed for 1D surfaces because they are
       *        not allowed to be tiled. They must be declared as linear."
       *    - Tiling and Mip Tail for 2D Surfaces
       *    - Tiling and Mip Tail for 3D Surfaces
       */
      num_2d_table_rows = ARRAY_SIZE(icl_std_y_2d_miptail_offset_el);
      num_3d_table_rows = ARRAY_SIZE(acm_tile64_3d_miptail_offset_el);
      break;

   default:
      unreachable("Invalid tiling");
   }

   assert(dim != ISL_SURF_DIM_1D);
   const int num_rows = dim == ISL_SURF_DIM_2D ? num_2d_table_rows :
                                                 num_3d_table_rows;
   return num_rows - isl_get_miptail_base_row(tiling);
}

/**
 * Returns an isl_tile_info representation of the given isl_tiling when
 * combined when used in the given configuration.
 *
 * :param tiling:       |in|  The tiling format to introspect
 * :param dim:          |in|  The dimensionality of the surface being tiled
 * :param msaa_layout:  |in|  The layout of samples in the surface being tiled
 * :param format_bpb:   |in|  The number of bits per surface element (block) for
 *                            the surface being tiled
 * :param samples:      |in|  The samples in the surface being tiled
 * :param tile_info:    |out| Return parameter for the tiling information
 */
void
isl_tiling_get_info(enum isl_tiling tiling,
                    enum isl_surf_dim dim,
                    enum isl_msaa_layout msaa_layout,
                    uint32_t format_bpb,
                    uint32_t samples,
                    struct isl_tile_info *tile_info)
{
   const uint32_t bs = format_bpb / 8;
   struct isl_extent4d logical_el;
   struct isl_extent2d phys_B;

   if (tiling != ISL_TILING_LINEAR && !isl_is_pow2(format_bpb)) {
      /* It is possible to have non-power-of-two formats in a tiled buffer.
       * The easiest way to handle this is to treat the tile as if it is three
       * times as wide.  This way no pixel will ever cross a tile boundary.
       * This really only works on a subset of tiling formats.
       */
      assert(tiling == ISL_TILING_X || tiling == ISL_TILING_Y0 ||
             tiling == ISL_TILING_4);
      assert(bs % 3 == 0 && isl_is_pow2(format_bpb / 3));
      isl_tiling_get_info(tiling, dim, msaa_layout, format_bpb / 3, samples,
                          tile_info);
      return;
   }

   switch (tiling) {
   case ISL_TILING_LINEAR:
      assert(bs > 0);
      logical_el = isl_extent4d(1, 1, 1, 1);
      phys_B = isl_extent2d(bs, 1);
      break;

   case ISL_TILING_X:
      assert(bs > 0);
      logical_el = isl_extent4d(512 / bs, 8, 1, 1);
      phys_B = isl_extent2d(512, 8);
      break;

   case ISL_TILING_Y0:
   case ISL_TILING_4:
      assert(bs > 0);
      logical_el = isl_extent4d(128 / bs, 32, 1, 1);
      phys_B = isl_extent2d(128, 32);
      break;

   case ISL_TILING_W:
      assert(bs == 1);
      logical_el = isl_extent4d(64, 64, 1, 1);
      /* From the Broadwell PRM Vol 2d, RENDER_SURFACE_STATE::SurfacePitch:
       *
       *    "If the surface is a stencil buffer (and thus has Tile Mode set
       *    to TILEMODE_WMAJOR), the pitch must be set to 2x the value
       *    computed based on width, as the stencil buffer is stored with two
       *    rows interleaved."
       *
       * This, together with the fact that stencil buffers are referred to as
       * being Y-tiled in the PRMs for older hardware implies that the
       * physical size of a W-tile is actually the same as for a Y-tile.
       */
      phys_B = isl_extent2d(128, 32);
      break;

   case ISL_TILING_SKL_Yf:
   case ISL_TILING_SKL_Ys:
   case ISL_TILING_ICL_Yf:
   case ISL_TILING_ICL_Ys: {
      bool is_Ys = tiling == ISL_TILING_SKL_Ys ||
                   tiling == ISL_TILING_ICL_Ys;
      assert(format_bpb >= 8);

      switch (dim) {
      case ISL_SURF_DIM_2D:
         /* See the BSpec Memory Data Formats » Common Surface Formats »
          * Surface Layout and Tiling [SKL+] » 2D Surfaces SKL+ » 2D/CUBE
          * Alignment Requirement [SKL+]
          *
          * Or, look in the SKL PRM under Memory Views > Common Surface
          * Formats > Surface Layout and Tiling > 2D Surfaces > 2D/CUBE
          * Alignment Requirements.
          */
         logical_el = (struct isl_extent4d) {
            .w = 1 << (6 - ((ffs(format_bpb) - 4) / 2) + (2 * is_Ys)),
            .h = 1 << (6 - ((ffs(format_bpb) - 3) / 2) + (2 * is_Ys)),
            .d = 1,
            .a = 1,
         };

         if (samples > 1 && tiling != ISL_TILING_SKL_Yf) {
            /* SKL PRMs, Volume 5: Memory Views, 2D/CUBE Alignment
             * Requirement:
             *
             *    "For MSFMT_MSS type multi-sampled TileYS surfaces, the
             *     alignments given above must be divided by the appropriate
             *     value from the table below."
             *
             * The formulas below reproduce those values.
             */
            if (msaa_layout == ISL_MSAA_LAYOUT_ARRAY) {
               logical_el.w >>= (ffs(samples) - 0) / 2;
               logical_el.h >>= (ffs(samples) - 1) / 2;
               logical_el.a = samples;
            }
         }
         break;

      case ISL_SURF_DIM_3D:
         /* See the BSpec Memory Data Formats » Common Surface Formats »
          * Surface Layout and Tiling [SKL+] » 3D Surfaces SKL+ » 3D Alignment
          * Requirements [SKL+]
          *
          * Or, look in the SKL PRM under Memory Views > Common Surface
          * Formats > Surface Layout and Tiling > 3D Surfaces > 3D Alignment
          * Requirements.
          */
         logical_el = (struct isl_extent4d) {
            .w = 1 << (4 - ((ffs(format_bpb) - 2) / 3) + (2 * is_Ys)),
            .h = 1 << (4 - ((ffs(format_bpb) - 4) / 3) + (1 * is_Ys)),
            .d = 1 << (4 - ((ffs(format_bpb) - 3) / 3) + (1 * is_Ys)),
            .a = 1,
         };
         break;
      default:
         unreachable("Invalid dimension");
      }

      uint32_t tile_size_B = is_Ys ? (1 << 16) : (1 << 12);

      phys_B.w = logical_el.width * bs;
      phys_B.h = tile_size_B / phys_B.w;
      break;
   }
   case ISL_TILING_64:
      /* The tables below are taken from the "2D Surfaces" & "3D Surfaces"
       * pages in the Bspec which are formulated in terms of the Cv and Cu
       * constants. This is different from the tables in the "Tile64 Format"
       * page which should be equivalent but are usually in terms of pixels.
       * Also note that Cv and Cu are HxW order to match the Bspec table, not
       * WxH order like you might expect.
       *
       * From the Bspec's or ATS-M PRMs Volume 5: Memory Data Formats, "Tile64
       * Format" :
       *
       *    MSAA Depth/Stencil surface use IMS (Interleaved Multi Samples)
       *    which means:
       *
       *    - Use the 1X MSAA (non-MSRT) version of the Tile64 equations and
       *      let the client unit do the swizzling internally
       *
       * Surfaces using the IMS layout will use the mapping for 1x MSAA.
       */
#define tile_extent2d(bs, cv, cu, a) \
      isl_extent4d((1 << cu) / bs, 1 << cv, 1, a)
#define tile_extent3d(bs, cr, cv, cu) \
      isl_extent4d((1 << cu) / bs, 1 << cv, 1 << cr, 1)

      if (dim == ISL_SURF_DIM_3D) {
          switch (format_bpb) {
          case 128: logical_el = tile_extent3d(bs, 4, 4, 8); break;
          case  64: logical_el = tile_extent3d(bs, 4, 4, 8); break;
          case  32: logical_el = tile_extent3d(bs, 4, 5, 7); break;
          case  16: logical_el = tile_extent3d(bs, 5, 5, 6); break;
          case   8: logical_el = tile_extent3d(bs, 5, 5, 6); break;
          default: unreachable("Unsupported format size for 3D");
          }
      } else {
          if (samples == 1 || msaa_layout == ISL_MSAA_LAYOUT_INTERLEAVED) {
              switch (format_bpb) {
              case 128: logical_el = tile_extent2d(bs, 6, 10, 1); break;
              case  64: logical_el = tile_extent2d(bs, 6, 10, 1); break;
              case  32: logical_el = tile_extent2d(bs, 7,  9, 1); break;
              case  16: logical_el = tile_extent2d(bs, 7,  9, 1); break;
              case   8: logical_el = tile_extent2d(bs, 8,  8, 1); break;
              default: unreachable("Unsupported format size.");
              }
          } else if (samples == 2) {
              switch (format_bpb) {
              case 128: logical_el = tile_extent2d(bs, 6,  9, 2); break;
              case  64: logical_el = tile_extent2d(bs, 6,  9, 2); break;
              case  32: logical_el = tile_extent2d(bs, 7,  8, 2); break;
              case  16: logical_el = tile_extent2d(bs, 7,  8, 2); break;
              case   8: logical_el = tile_extent2d(bs, 8,  7, 2); break;
              default: unreachable("Unsupported format size.");
              }
          } else {
              switch (format_bpb) {
              case 128: logical_el = tile_extent2d(bs, 5,  9, 4); break;
              case  64: logical_el = tile_extent2d(bs, 5,  9, 4); break;
              case  32: logical_el = tile_extent2d(bs, 6,  8, 4); break;
              case  16: logical_el = tile_extent2d(bs, 6,  8, 4); break;
              case   8: logical_el = tile_extent2d(bs, 7,  7, 4); break;
              default: unreachable("Unsupported format size.");
              }
          }
      }

#undef tile_extent2d
#undef tile_extent3d

      phys_B.w = logical_el.w * bs;
      phys_B.h = 64 * 1024 / phys_B.w;
      break;

   case ISL_TILING_HIZ:
      /* HiZ buffers are required to have a 128bpb HiZ format. The tiling has
       * the same physical dimensions as Y-tiling but actually has two HiZ
       * columns per Y-tiled column.
       */
      assert(bs == 16);
      logical_el = isl_extent4d(16, 16, 1, 1);
      phys_B = isl_extent2d(128, 32);
      break;

   case ISL_TILING_CCS:
      /* CCS surfaces are required to have one of the GENX_CCS_* formats which
       * have a block size of 1 or 2 bits per block and each CCS element
       * corresponds to one cache-line pair in the main surface.  From the Sky
       * Lake PRM Vol. 12 in the section on planes:
       *
       *    "The Color Control Surface (CCS) contains the compression status
       *    of the cache-line pairs. The compression state of the cache-line
       *    pair is specified by 2 bits in the CCS.  Each CCS cache-line
       *    represents an area on the main surface of 16x16 sets of 128 byte
       *    Y-tiled cache-line-pairs. CCS is always Y tiled."
       *
       * The CCS being Y-tiled implies that it's an 8x8 grid of cache-lines.
       * Since each cache line corresponds to a 16x16 set of cache-line pairs,
       * that yields total tile area of 128x128 cache-line pairs or CCS
       * elements.  On older hardware, each CCS element is 1 bit and the tile
       * is 128x256 elements.
       */
      assert(format_bpb == 1 || format_bpb == 2);
      logical_el = isl_extent4d(128, 256 / format_bpb, 1, 1);
      phys_B = isl_extent2d(128, 32);
      break;

   case ISL_TILING_GFX12_CCS:
      /* From the Bspec, Gen Graphics > Gfx12 > Memory Data Formats > Memory
       * Compression > Memory Compression - Gfx12:
       *
       *    4 bits of auxiliary plane data are required for 2 cachelines of
       *    main surface data. This results in a single cacheline of auxiliary
       *    plane data mapping to 4 4K pages of main surface data for the 4K
       *    pages (tile Y ) and 1 64K Tile Ys page.
       *
       * The Y-tiled pairing bit of 9 shown in the table below that Bspec
       * section expresses that the 2 cachelines of main surface data are
       * horizontally adjacent.
       *
       * TODO: Handle Ys, Yf and their pairing bits.
       *
       * Therefore, each CCS cacheline represents a 512Bx32 row area and each
       * element represents a 32Bx4 row area.
       */
      assert(format_bpb == 4);
      logical_el = isl_extent4d(16, 8, 1, 1);
      phys_B = isl_extent2d(64, 1);
      break;

   default:
      unreachable("not reached");
   } /* end switch */

   *tile_info = (struct isl_tile_info) {
      .tiling = tiling,
      .format_bpb = format_bpb,
      .logical_extent_el = logical_el,
      .phys_extent_B = phys_B,
      .max_miptail_levels = tiling_max_mip_tail(tiling, dim, samples),
   };
}

bool
isl_color_value_is_zero(union isl_color_value value,
                        enum isl_format format)
{
   const struct isl_format_layout *fmtl = isl_format_get_layout(format);

#define RETURN_FALSE_IF_NOT_0(c, i) \
   if (fmtl->channels.c.bits && value.u32[i] != 0) \
      return false

   RETURN_FALSE_IF_NOT_0(r, 0);
   RETURN_FALSE_IF_NOT_0(g, 1);
   RETURN_FALSE_IF_NOT_0(b, 2);
   RETURN_FALSE_IF_NOT_0(a, 3);

#undef RETURN_FALSE_IF_NOT_0

   return true;
}

bool
isl_color_value_is_zero_one(union isl_color_value value,
                            enum isl_format format)
{
   const struct isl_format_layout *fmtl = isl_format_get_layout(format);

#define RETURN_FALSE_IF_NOT_0_1(c, i, field) \
   if (fmtl->channels.c.bits && value.field[i] != 0 && value.field[i] != 1) \
      return false

   if (isl_format_has_int_channel(format)) {
      RETURN_FALSE_IF_NOT_0_1(r, 0, u32);
      RETURN_FALSE_IF_NOT_0_1(g, 1, u32);
      RETURN_FALSE_IF_NOT_0_1(b, 2, u32);
      RETURN_FALSE_IF_NOT_0_1(a, 3, u32);
   } else {
      RETURN_FALSE_IF_NOT_0_1(r, 0, f32);
      RETURN_FALSE_IF_NOT_0_1(g, 1, f32);
      RETURN_FALSE_IF_NOT_0_1(b, 2, f32);
      RETURN_FALSE_IF_NOT_0_1(a, 3, f32);
   }

#undef RETURN_FALSE_IF_NOT_0_1

   return true;
}

/**
 * @param[out] tiling is set only on success
 */
static bool
isl_surf_choose_tiling(const struct isl_device *dev,
                       const struct isl_surf_init_info *restrict info,
                       enum isl_tiling *tiling)
{
   isl_tiling_flags_t tiling_flags = info->tiling_flags;

   /* HiZ surfaces always use the HiZ tiling */
   if (info->usage & ISL_SURF_USAGE_HIZ_BIT) {
      assert(isl_format_is_hiz(info->format));
      assert(tiling_flags == ISL_TILING_HIZ_BIT);
      *tiling = isl_tiling_flag_to_enum(tiling_flags);
      return true;
   }

   /* CCS surfaces always use the CCS tiling */
   if (info->usage & ISL_SURF_USAGE_CCS_BIT) {
      assert(isl_format_get_layout(info->format)->txc == ISL_TXC_CCS);
      UNUSED bool ivb_ccs = ISL_GFX_VER(dev) < 12 &&
                            tiling_flags == ISL_TILING_CCS_BIT;
      UNUSED bool tgl_ccs = ISL_GFX_VER(dev) >= 12 &&
                            tiling_flags == ISL_TILING_GFX12_CCS_BIT;
      assert(ivb_ccs != tgl_ccs);
      *tiling = isl_tiling_flag_to_enum(tiling_flags);
      return true;
   }

   if (ISL_GFX_VERX10(dev) >= 125) {
      isl_gfx125_filter_tiling(dev, info, &tiling_flags);
   } else if (ISL_GFX_VER(dev) >= 6) {
      isl_gfx6_filter_tiling(dev, info, &tiling_flags);
   } else {
      isl_gfx4_filter_tiling(dev, info, &tiling_flags);
   }

   #define CHOOSE(__tiling) \
      do { \
         if (tiling_flags & (1u << (__tiling))) { \
            *tiling = (__tiling); \
            return true; \
          } \
      } while (0)

   /* Of the tiling modes remaining, choose the one that offers the best
    * performance.
    */

   if (info->dim == ISL_SURF_DIM_1D) {
      /* Prefer linear for 1D surfaces because they do not benefit from
       * tiling. To the contrary, tiling leads to wasted memory and poor
       * memory locality due to the swizzling and alignment restrictions
       * required in tiled surfaces.
       */
      CHOOSE(ISL_TILING_LINEAR);
   }

   /* For sparse images, prefer the formats that use the standard block
    * shapes.
    */
   if (info->usage & ISL_SURF_USAGE_SPARSE_BIT) {
      CHOOSE(ISL_TILING_64);
      CHOOSE(ISL_TILING_ICL_Ys);
      CHOOSE(ISL_TILING_SKL_Ys);
   }

   /* Choose suggested 4K tilings first, then 64K tilings:
    *
    * Then following quotes can be found in the SKL PRMs,
    *   Volume 5: Memory Views, Address Tiling Function Introduction
    * and from the ATS-M PRMs,
    *   Volume 5: Memory Data Formats, Address Tiling Function Introduction
    *
    *    "TileY: Used for most tiled surfaces when TR_MODE=TR_NONE."
    *    "Tile4: 4KB tiling mode based on previously-supported TileY"
    *    "TileYF: 4KB tiling mode based on TileY"
    *    "TileYS: 64KB tiling mode based on TileY"
    *    "Tile64: 64KB tiling mode which support standard-tiling including
    *     Mip Tails"
    *
    * When TileYF and TileYS are used TR_MODE != TR_NONE.
    */
   CHOOSE(ISL_TILING_Y0);
   CHOOSE(ISL_TILING_4);
   CHOOSE(ISL_TILING_SKL_Yf);
   CHOOSE(ISL_TILING_ICL_Yf);
   CHOOSE(ISL_TILING_SKL_Ys);
   CHOOSE(ISL_TILING_ICL_Ys);
   CHOOSE(ISL_TILING_64);

   CHOOSE(ISL_TILING_X);
   CHOOSE(ISL_TILING_W);
   CHOOSE(ISL_TILING_LINEAR);

   #undef CHOOSE

   /* No tiling mode accommodates the inputs. */
   assert(tiling_flags == 0);
   return notify_failure(info, "no supported tiling");
}

static bool
isl_choose_msaa_layout(const struct isl_device *dev,
                 const struct isl_surf_init_info *info,
                 enum isl_tiling tiling,
                 enum isl_msaa_layout *msaa_layout)
{
   if (ISL_GFX_VER(dev) >= 8) {
      return isl_gfx8_choose_msaa_layout(dev, info, tiling, msaa_layout);
   } else if (ISL_GFX_VER(dev) >= 7) {
      return isl_gfx7_choose_msaa_layout(dev, info, tiling, msaa_layout);
   } else if (ISL_GFX_VER(dev) >= 6) {
      return isl_gfx6_choose_msaa_layout(dev, info, tiling, msaa_layout);
   } else {
      return isl_gfx4_choose_msaa_layout(dev, info, tiling, msaa_layout);
   }
}

struct isl_extent2d
isl_get_interleaved_msaa_px_size_sa(uint32_t samples)
{
   assert(isl_is_pow2(samples));

   /* From the Broadwell PRM >> Volume 5: Memory Views >> Computing Mip Level
    * Sizes (p133):
    *
    *    If the surface is multisampled and it is a depth or stencil surface
    *    or Multisampled Surface StorageFormat in SURFACE_STATE is
    *    MSFMT_DEPTH_STENCIL, W_L and H_L must be adjusted as follows before
    *    proceeding: [...]
    */
   return (struct isl_extent2d) {
      .width = 1 << ((ffs(samples) - 0) / 2),
      .height = 1 << ((ffs(samples) - 1) / 2),
   };
}

static void
isl_msaa_interleaved_scale_px_to_sa(uint32_t samples,
                                    uint32_t *width, uint32_t *height)
{
   const struct isl_extent2d px_size_sa =
      isl_get_interleaved_msaa_px_size_sa(samples);

   if (width)
      *width = isl_align(*width, 2) * px_size_sa.width;
   if (height)
      *height = isl_align(*height, 2) * px_size_sa.height;
}

static enum isl_array_pitch_span
isl_choose_array_pitch_span(const struct isl_device *dev,
                            const struct isl_surf_init_info *restrict info,
                            enum isl_dim_layout dim_layout,
                            const struct isl_extent4d *phys_level0_sa)
{
   switch (dim_layout) {
   case ISL_DIM_LAYOUT_GFX9_1D:
   case ISL_DIM_LAYOUT_GFX4_2D:
      if (ISL_GFX_VER(dev) >= 8) {
         /* QPitch becomes programmable in Broadwell. So choose the
          * most compact QPitch possible in order to conserve memory.
          *
          * From the Broadwell PRM >> Volume 2d: Command Reference: Structures
          * >> RENDER_SURFACE_STATE Surface QPitch (p325):
          *
          *    - Software must ensure that this field is set to a value
          *      sufficiently large such that the array slices in the surface
          *      do not overlap. Refer to the Memory Data Formats section for
          *      information on how surfaces are stored in memory.
          *
          *    - This field specifies the distance in rows between array
          *      slices.  It is used only in the following cases:
          *
          *          - Surface Array is enabled OR
          *          - Number of Mulitsamples is not NUMSAMPLES_1 and
          *            Multisampled Surface Storage Format set to MSFMT_MSS OR
          *          - Surface Type is SURFTYPE_CUBE
          */
         return ISL_ARRAY_PITCH_SPAN_COMPACT;
      } else if (ISL_GFX_VER(dev) >= 7) {
         /* Note that Ivybridge introduces
          * RENDER_SURFACE_STATE.SurfaceArraySpacing, which provides the
          * driver more control over the QPitch.
          */

         if (phys_level0_sa->array_len == 1) {
            /* The hardware will never use the QPitch. So choose the most
             * compact QPitch possible in order to conserve memory.
             */
            return ISL_ARRAY_PITCH_SPAN_COMPACT;
         }

         if (isl_surf_usage_is_depth_or_stencil(info->usage) ||
             (info->usage & ISL_SURF_USAGE_HIZ_BIT)) {
            /* From the Ivybridge PRM >> Volume 1 Part 1: Graphics Core >>
             * Section 6.18.4.7: Surface Arrays (p112):
             *
             *    If Surface Array Spacing is set to ARYSPC_FULL (note that
             *    the depth buffer and stencil buffer have an implied value of
             *    ARYSPC_FULL):
             */
            return ISL_ARRAY_PITCH_SPAN_FULL;
         }

         if (info->levels == 1) {
            /* We are able to set RENDER_SURFACE_STATE.SurfaceArraySpacing
             * to ARYSPC_LOD0.
             */
            return ISL_ARRAY_PITCH_SPAN_COMPACT;
         }

         return ISL_ARRAY_PITCH_SPAN_FULL;
      } else if ((ISL_GFX_VER(dev) == 5 || ISL_GFX_VER(dev) == 6) &&
                 ISL_DEV_USE_SEPARATE_STENCIL(dev) &&
                 isl_surf_usage_is_stencil(info->usage)) {
         /* [ILK-SNB] Errata from the Sandy Bridge PRM >> Volume 4 Part 1:
          * Graphics Core >> Section 7.18.3.7: Surface Arrays:
          *
          *    The separate stencil buffer does not support mip mapping, thus
          *    the storage for LODs other than LOD 0 is not needed.
          */
         assert(info->levels == 1);
         return ISL_ARRAY_PITCH_SPAN_COMPACT;
      } else {
         if ((ISL_GFX_VER(dev) == 5 || ISL_GFX_VER(dev) == 6) &&
             ISL_DEV_USE_SEPARATE_STENCIL(dev) &&
             isl_surf_usage_is_stencil(info->usage)) {
            /* [ILK-SNB] Errata from the Sandy Bridge PRM >> Volume 4 Part 1:
             * Graphics Core >> Section 7.18.3.7: Surface Arrays:
             *
             *    The separate stencil buffer does not support mip mapping,
             *    thus the storage for LODs other than LOD 0 is not needed.
             */
            assert(info->levels == 1);
            assert(phys_level0_sa->array_len == 1);
            return ISL_ARRAY_PITCH_SPAN_COMPACT;
         }

         if (phys_level0_sa->array_len == 1) {
            /* The hardware will never use the QPitch. So choose the most
             * compact QPitch possible in order to conserve memory.
             */
            return ISL_ARRAY_PITCH_SPAN_COMPACT;
         }

         return ISL_ARRAY_PITCH_SPAN_FULL;
      }

   case ISL_DIM_LAYOUT_GFX4_3D:
      /* The hardware will never use the QPitch. So choose the most
       * compact QPitch possible in order to conserve memory.
       */
      return ISL_ARRAY_PITCH_SPAN_COMPACT;

   case ISL_DIM_LAYOUT_GFX6_STENCIL_HIZ:
      /* Each array image in the gfx6 stencil of HiZ surface is compact in the
       * sense that every LOD is a compact array of the same size as LOD0.
       */
      return ISL_ARRAY_PITCH_SPAN_COMPACT;
   }

   unreachable("bad isl_dim_layout");
   return ISL_ARRAY_PITCH_SPAN_FULL;
}

static void
isl_choose_image_alignment_el(const struct isl_device *dev,
                              const struct isl_surf_init_info *restrict info,
                              enum isl_tiling tiling,
                              enum isl_dim_layout dim_layout,
                              enum isl_msaa_layout msaa_layout,
                              struct isl_extent3d *image_align_el)
{
   const struct isl_format_layout *fmtl = isl_format_get_layout(info->format);
   if (fmtl->txc == ISL_TXC_MCS) {
      /*
       * IvyBrigde PRM Vol 2, Part 1, "11.7 MCS Buffer for Render Target(s)":
       *
       * Height, width, and layout of MCS buffer in this case must match with
       * Render Target height, width, and layout. MCS buffer is tiledY.
       *
       * Pick a vertical and horizontal alignment that matches the main render
       * target. Vertical alignment is important for properly spacing an array
       * of MCS images. Horizontal alignment is not expected to matter because
       * MCS is not mipmapped. Regardless, we pick a valid value here.
       */
      if (ISL_GFX_VERX10(dev) >= 125) {
         *image_align_el = isl_extent3d(128 * 8 / fmtl->bpb, 4, 1);
      } else if (ISL_GFX_VER(dev) >= 8) {
         *image_align_el = isl_extent3d(16, 4, 1);
      } else {
         *image_align_el = isl_extent3d(4, 4, 1);
      }
      return;
   } else if (fmtl->txc == ISL_TXC_HIZ) {
      assert(ISL_GFX_VER(dev) >= 6);
      if (ISL_GFX_VER(dev) == 6) {
         /* HiZ surfaces on Sandy Bridge are packed tightly. */
         *image_align_el = isl_extent3d(1, 1, 1);
      } else if (ISL_GFX_VER(dev) < 12) {
         /* On gfx7+, HiZ surfaces are always aligned to 16x8 pixels in the
          * primary surface which works out to 2x2 HiZ elements.
          */
         *image_align_el = isl_extent3d(2, 2, 1);
      } else {
         /* We choose the alignments based on the docs and what we've seen on
          * prior platforms. From the TGL PRM Vol. 9, "Hierarchical Depth
          * Buffer":
          *
          *    The height and width of the hierarchical depth buffer that must
          *    be allocated are computed by the following formulas, where HZ
          *    is the hierarchical depth buffer and Z is the depth buffer. The
          *    Z_Height, Z_Width, and Z_Depth values given in these formulas
          *    are those present in 3DSTATE_DEPTH_BUFFER incremented by one.
          *
          * The note about 3DSTATE_DEPTH_BUFFER tells us that the dimensions
          * in the following formula refers to the base level. The key formula
          * for the horizontal alignment is:
          *
          *    HZ_Width (bytes) [=]
          *    ceiling(Z_Width / 16) * 16
          *
          * This type of formula is used when sizing compression blocks. So,
          * the docs seem to say that the HiZ format has a block width of 16,
          * and thus, the surface has a minimum horizontal alignment of 16
          * pixels. This formula hasn't changed from prior platforms (where
          * we've chosen a horizontal alignment of 16), so we should be on the
          * right track. As for the vertical alignment, we're told:
          *
          *    To compute the minimum QPitch for the HZ surface, the height of
          *    each LOD in pixels is determined using the equations for hL in
          *    the GPU Overview volume, using a vertical alignment j=16.
          *
          * We're not calculating the QPitch right now, but the vertical
          * alignment is plainly given as 16 rows in the depth buffer.
          *
          * As a result, we believe that HiZ surfaces are aligned to 16x16
          * pixels in the primary surface. We divide this area by the HiZ
          * block dimensions to get the alignment in terms of HiZ blocks.
          */
         *image_align_el = isl_extent3d(16 / fmtl->bw, 16 / fmtl->bh, 1);
      }
      return;
   }

   if (ISL_GFX_VERX10(dev) >= 125) {
      isl_gfx125_choose_image_alignment_el(dev, info, tiling, dim_layout,
                                           msaa_layout, image_align_el);
   } else if (ISL_GFX_VER(dev) >= 12) {
      isl_gfx12_choose_image_alignment_el(dev, info, tiling, dim_layout,
                                          msaa_layout, image_align_el);
   } else if (ISL_GFX_VER(dev) >= 9) {
      isl_gfx9_choose_image_alignment_el(dev, info, tiling, dim_layout,
                                         msaa_layout, image_align_el);
   } else if (ISL_GFX_VER(dev) >= 8) {
      isl_gfx8_choose_image_alignment_el(dev, info, tiling, dim_layout,
                                         msaa_layout, image_align_el);
   } else if (ISL_GFX_VER(dev) >= 7) {
      isl_gfx7_choose_image_alignment_el(dev, info, tiling, dim_layout,
                                          msaa_layout, image_align_el);
   } else if (ISL_GFX_VER(dev) >= 6) {
      isl_gfx6_choose_image_alignment_el(dev, info, tiling, dim_layout,
                                         msaa_layout, image_align_el);
   } else {
      isl_gfx4_choose_image_alignment_el(dev, info, tiling, dim_layout,
                                         msaa_layout, image_align_el);
   }
}

static enum isl_dim_layout
isl_surf_choose_dim_layout(const struct isl_device *dev,
                           enum isl_surf_dim logical_dim,
                           enum isl_tiling tiling,
                           isl_surf_usage_flags_t usage)
{
   /* Sandy bridge needs a special layout for HiZ and stencil. */
   if (ISL_GFX_VER(dev) == 6 &&
       (tiling == ISL_TILING_W || tiling == ISL_TILING_HIZ))
      return ISL_DIM_LAYOUT_GFX6_STENCIL_HIZ;

   if (ISL_GFX_VER(dev) >= 9) {
      switch (logical_dim) {
      case ISL_SURF_DIM_1D:
         /* From the Sky Lake PRM Vol. 5, "1D Surfaces":
          *
          *    One-dimensional surfaces use a tiling mode of linear.
          *    Technically, they are not tiled resources, but the Tiled
          *    Resource Mode field in RENDER_SURFACE_STATE is still used to
          *    indicate the alignment requirements for this linear surface
          *    (See 1D Alignment requirements for how 4K and 64KB Tiled
          *    Resource Modes impact alignment). Alternatively, a 1D surface
          *    can be defined as a 2D tiled surface (e.g. TileY or TileX) with
          *    a height of 0.
          *
          * In other words, ISL_DIM_LAYOUT_GFX9_1D is only used for linear
          * surfaces and, for tiled surfaces, ISL_DIM_LAYOUT_GFX4_2D is used.
          */
         if (tiling == ISL_TILING_LINEAR)
            return ISL_DIM_LAYOUT_GFX9_1D;
         else
            return ISL_DIM_LAYOUT_GFX4_2D;
      case ISL_SURF_DIM_2D:
      case ISL_SURF_DIM_3D:
         return ISL_DIM_LAYOUT_GFX4_2D;
      }
   } else {
      switch (logical_dim) {
      case ISL_SURF_DIM_1D:
      case ISL_SURF_DIM_2D:
         /* From the G45 PRM Vol. 1a, "6.17.4.1 Hardware Cube Map Layout":
          *
          * The cube face textures are stored in the same way as 3D surfaces
          * are stored (see section 6.17.5 for details).  For cube surfaces,
          * however, the depth is equal to the number of faces (always 6) and
          * is not reduced for each MIP.
          */
         if (ISL_GFX_VER(dev) == 4 && (usage & ISL_SURF_USAGE_CUBE_BIT))
            return ISL_DIM_LAYOUT_GFX4_3D;

         return ISL_DIM_LAYOUT_GFX4_2D;
      case ISL_SURF_DIM_3D:
         return ISL_DIM_LAYOUT_GFX4_3D;
      }
   }

   unreachable("bad isl_surf_dim");
   return ISL_DIM_LAYOUT_GFX4_2D;
}

/**
 * Calculate the physical extent of the surface's first level, in units of
 * surface samples.
 */
static void
isl_calc_phys_level0_extent_sa(const struct isl_device *dev,
                               const struct isl_surf_init_info *restrict info,
                               enum isl_dim_layout dim_layout,
                               enum isl_tiling tiling,
                               enum isl_msaa_layout msaa_layout,
                               struct isl_extent4d *phys_level0_sa)
{
   const struct isl_format_layout *fmtl = isl_format_get_layout(info->format);

   if (isl_format_is_planar(info->format))
      unreachable("Planar formats unsupported");

   switch (info->dim) {
   case ISL_SURF_DIM_1D:
      assert(info->height == 1);
      assert(info->depth == 1);
      assert(info->samples == 1);

      switch (dim_layout) {
      case ISL_DIM_LAYOUT_GFX4_3D:
         unreachable("bad isl_dim_layout");

      case ISL_DIM_LAYOUT_GFX9_1D:
      case ISL_DIM_LAYOUT_GFX4_2D:
      case ISL_DIM_LAYOUT_GFX6_STENCIL_HIZ:
         *phys_level0_sa = (struct isl_extent4d) {
            .w = info->width,
            .h = 1,
            .d = 1,
            .a = info->array_len,
         };
         break;
      }
      break;

   case ISL_SURF_DIM_2D:
      if (ISL_GFX_VER(dev) == 4 && (info->usage & ISL_SURF_USAGE_CUBE_BIT))
         assert(dim_layout == ISL_DIM_LAYOUT_GFX4_3D);
      else
         assert(dim_layout == ISL_DIM_LAYOUT_GFX4_2D ||
                dim_layout == ISL_DIM_LAYOUT_GFX6_STENCIL_HIZ);

      switch (msaa_layout) {
      case ISL_MSAA_LAYOUT_NONE:
         assert(info->depth == 1);
         assert(info->samples == 1);

         *phys_level0_sa = (struct isl_extent4d) {
            .w = info->width,
            .h = info->height,
            .d = 1,
            .a = info->array_len,
         };
         break;

      case ISL_MSAA_LAYOUT_ARRAY:
         assert(info->depth == 1);
         assert(info->levels == 1);
         assert(isl_format_supports_multisampling(dev->info, info->format));
         assert(fmtl->bw == 1 && fmtl->bh == 1);

         *phys_level0_sa = (struct isl_extent4d) {
            .w = info->width,
            .h = info->height,
            .d = 1,
            .a = info->array_len * info->samples,
         };
         break;

      case ISL_MSAA_LAYOUT_INTERLEAVED:
         assert(info->depth == 1);
         assert(info->levels == 1);
         assert(isl_format_supports_multisampling(dev->info, info->format));

         *phys_level0_sa = (struct isl_extent4d) {
            .w = info->width,
            .h = info->height,
            .d = 1,
            .a = info->array_len,
         };

         isl_msaa_interleaved_scale_px_to_sa(info->samples,
                                             &phys_level0_sa->w,
                                             &phys_level0_sa->h);
         break;
      }
      break;

   case ISL_SURF_DIM_3D:
      assert(info->array_len == 1);
      assert(info->samples == 1);

      if (fmtl->bd > 1) {
         isl_finishme("%s:%s: compression block with depth > 1",
                      __FILE__, __func__);
      }

      switch (dim_layout) {
      case ISL_DIM_LAYOUT_GFX9_1D:
      case ISL_DIM_LAYOUT_GFX6_STENCIL_HIZ:
         unreachable("bad isl_dim_layout");

      case ISL_DIM_LAYOUT_GFX4_2D:
      case ISL_DIM_LAYOUT_GFX4_3D:
         *phys_level0_sa = (struct isl_extent4d) {
            .w = info->width,
            .h = info->height,
            .d = info->depth,
            .a = 1,
         };
         break;
      }
      break;
   }
}

static void
isl_get_miptail_level_offset_el(enum isl_tiling tiling,
                                enum isl_surf_dim dim,
                                uint32_t format_bpb,
                                uint32_t level,
                                uint32_t *x_offset_el,
                                uint32_t *y_offset_el,
                                uint32_t *z_offset_el)
{
   uint32_t row = isl_get_miptail_base_row(tiling) + level;
   uint32_t col = 8 - ffs(format_bpb);

   switch (dim) {
   case ISL_SURF_DIM_2D:
      switch (tiling) {
      case ISL_TILING_64:
      case ISL_TILING_ICL_Yf:
      case ISL_TILING_ICL_Ys:
         assert(row < ARRAY_SIZE(icl_std_y_2d_miptail_offset_el));
         assert(col < ARRAY_SIZE(icl_std_y_2d_miptail_offset_el[0]));
         *x_offset_el = icl_std_y_2d_miptail_offset_el[row][col][0];
         *y_offset_el = icl_std_y_2d_miptail_offset_el[row][col][1];
         break;
      case ISL_TILING_SKL_Yf:
      case ISL_TILING_SKL_Ys:
         assert(row < ARRAY_SIZE(skl_std_y_2d_miptail_offset_el));
         assert(col < ARRAY_SIZE(skl_std_y_2d_miptail_offset_el[0]));
         *x_offset_el = skl_std_y_2d_miptail_offset_el[row][col][0];
         *y_offset_el = skl_std_y_2d_miptail_offset_el[row][col][1];
         break;
      default:
         unreachable("invalid tiling");
      }
      *z_offset_el = 0;
      break;

   case ISL_SURF_DIM_3D:
      switch (tiling) {
      case ISL_TILING_64:
         assert(row < ARRAY_SIZE(acm_tile64_3d_miptail_offset_el));
         assert(col < ARRAY_SIZE(acm_tile64_3d_miptail_offset_el[0]));
         *x_offset_el = acm_tile64_3d_miptail_offset_el[row][col][0];
         *y_offset_el = acm_tile64_3d_miptail_offset_el[row][col][1];
         *z_offset_el = acm_tile64_3d_miptail_offset_el[row][col][2];
         break;
      case ISL_TILING_ICL_Yf:
      case ISL_TILING_ICL_Ys:
         assert(row < ARRAY_SIZE(icl_std_y_3d_miptail_offset_el));
         assert(col < ARRAY_SIZE(icl_std_y_3d_miptail_offset_el[0]));
         *x_offset_el = icl_std_y_3d_miptail_offset_el[row][col][0];
         *y_offset_el = icl_std_y_3d_miptail_offset_el[row][col][1];
         *z_offset_el = icl_std_y_3d_miptail_offset_el[row][col][2];
         break;
      case ISL_TILING_SKL_Yf:
      case ISL_TILING_SKL_Ys:
         assert(row < ARRAY_SIZE(skl_std_y_3d_miptail_offset_el));
         assert(col < ARRAY_SIZE(skl_std_y_3d_miptail_offset_el[0]));
         *x_offset_el = skl_std_y_3d_miptail_offset_el[row][col][0];
         *y_offset_el = skl_std_y_3d_miptail_offset_el[row][col][1];
         *z_offset_el = skl_std_y_3d_miptail_offset_el[row][col][2];
         break;
      default:
         unreachable("invalid tiling");
      }
      break;

   case ISL_SURF_DIM_1D:
      unreachable("invalid dimension");
   }
}

static uint32_t
isl_choose_miptail_start_level(const struct isl_device *dev,
                               const struct isl_surf_init_info *restrict info,
                               const struct isl_tile_info *tile_info)
{
   const struct isl_format_layout *fmtl = isl_format_get_layout(info->format);

   if (tile_info->max_miptail_levels == 0)
      return info->levels;

   /* SKL PRMs, Volume 5: Memory Views, YUV 4:2:0 Format Memory Organization :
    *
    *    "Planar YUV does not support MIP Tails as part of Standard Tiling.
    *     The MIP Tail Start field in RENDER_SURFACE_STATE must be programmed
    *     to 15."
    */
   if (isl_format_is_planar(info->format))
      return 15;

   /* TODO: figure out why having YUV formats in the miptail on Gfx12 does not
    *       work.
    */
   if (ISL_GFX_VER(dev) == 12 && isl_format_is_yuv(info->format))
      return 15;

   assert(tile_info->tiling == ISL_TILING_64 || isl_tiling_is_std_y(tile_info->tiling));
   assert(info->samples == 1);

   uint32_t max_miptail_levels = tile_info->max_miptail_levels;

   /* Start with the minimum number of levels that will fit in the tile */
   uint32_t min_miptail_start =
      info->levels > max_miptail_levels ? info->levels - max_miptail_levels : 0;

   /* Account for the specified minimum */
   min_miptail_start = MAX(min_miptail_start, info->min_miptail_start_level);

   struct isl_extent3d level0_extent_el = {
      .w = isl_align_div_npot(info->width, fmtl->bw),
      .h = isl_align_div_npot(info->height, fmtl->bh),
      .d = isl_align_div_npot(info->depth, fmtl->bd),
   };

   /* The first miptail slot takes up the entire right side of the tile. So,
    * the extent is just the distance from the offset of the first level to
    * the corner of the tile.
    */
   uint32_t level0_x_offset_el, level0_y_offset_el, level0_z_offset_el;
   isl_get_miptail_level_offset_el(tile_info->tiling, info->dim,
                                   fmtl->bpb, 0, /* level */
                                   &level0_x_offset_el,
                                   &level0_y_offset_el,
                                   &level0_z_offset_el);
   struct isl_extent3d miptail_level0_extent_el = {
      .w = tile_info->logical_extent_el.w - level0_x_offset_el,
      .h = tile_info->logical_extent_el.h - level0_y_offset_el,
      .d = tile_info->logical_extent_el.d - level0_z_offset_el,
   };

   /* Now find the first level that fits the maximum miptail size requirement.
    */
   for (uint32_t s = min_miptail_start; s < info->levels; s++) {
      if (isl_minify(level0_extent_el.w, s) <= miptail_level0_extent_el.w &&
          isl_minify(level0_extent_el.h, s) <= miptail_level0_extent_el.h &&
          isl_minify(level0_extent_el.d, s) <= miptail_level0_extent_el.d)
         return s;
   }

   return info->levels;
}

/**
 * Calculate the pitch between physical array slices, in units of rows of
 * surface elements.
 */
static uint32_t
isl_calc_array_pitch_el_rows_gfx4_2d(
      const struct isl_device *dev,
      const struct isl_surf_init_info *restrict info,
      const struct isl_tile_info *tile_info,
      const struct isl_extent3d *image_align_sa,
      const struct isl_extent4d *phys_level0_sa,
      enum isl_array_pitch_span array_pitch_span,
      const struct isl_extent2d *phys_slice0_sa)
{
   const struct isl_format_layout *fmtl = isl_format_get_layout(info->format);
   uint32_t pitch_sa_rows = 0;

   switch (array_pitch_span) {
   case ISL_ARRAY_PITCH_SPAN_COMPACT:
      pitch_sa_rows = isl_align_npot(phys_slice0_sa->h, image_align_sa->h);
      break;
   case ISL_ARRAY_PITCH_SPAN_FULL: {
      /* The QPitch equation is found in the Broadwell PRM >> Volume 5:
       * Memory Views >> Common Surface Formats >> Surface Layout >> 2D
       * Surfaces >> Surface Arrays.
       */
      uint32_t H0_sa = phys_level0_sa->h;
      uint32_t H1_sa = isl_minify(H0_sa, 1);

      uint32_t h0_sa = isl_align_npot(H0_sa, image_align_sa->h);
      uint32_t h1_sa = isl_align_npot(H1_sa, image_align_sa->h);

      uint32_t m;
      if (ISL_GFX_VER(dev) >= 7) {
         /* The QPitch equation changed slightly in Ivybridge. */
         m = 12;
      } else {
         m = 11;
      }

      pitch_sa_rows = h0_sa + h1_sa + (m * image_align_sa->h);

      if (ISL_GFX_VER(dev) == 6 && info->samples > 1 &&
          (info->height % 4 == 1)) {
         /* [SNB] Errata from the Sandy Bridge PRM >> Volume 4 Part 1:
          * Graphics Core >> Section 7.18.3.7: Surface Arrays:
          *
          *    [SNB] Errata: Sampler MSAA Qpitch will be 4 greater than
          *    the value calculated in the equation above , for every
          *    other odd Surface Height starting from 1 i.e. 1,5,9,13.
          *
          * XXX(chadv): Is the errata natural corollary of the physical
          * layout of interleaved samples?
          */
         pitch_sa_rows += 4;
      }

      pitch_sa_rows = isl_align_npot(pitch_sa_rows, fmtl->bh);
      } /* end case */
      break;
   }

   assert(pitch_sa_rows % fmtl->bh == 0);
   uint32_t pitch_el_rows = pitch_sa_rows / fmtl->bh;

   if (ISL_GFX_VER(dev) >= 9 && ISL_GFX_VER(dev) <= 11 &&
       fmtl->txc == ISL_TXC_CCS) {
      /*
       * From the Sky Lake PRM Vol 7, "MCS Buffer for Render Target(s)" (p. 632):
       *
       *    "Mip-mapped and arrayed surfaces are supported with MCS buffer
       *    layout with these alignments in the RT space: Horizontal
       *    Alignment = 128 and Vertical Alignment = 64."
       *
       * From the Sky Lake PRM Vol. 2d, "RENDER_SURFACE_STATE" (p. 435):
       *
       *    "For non-multisampled render target's CCS auxiliary surface,
       *    QPitch must be computed with Horizontal Alignment = 128 and
       *    Surface Vertical Alignment = 256. These alignments are only for
       *    CCS buffer and not for associated render target."
       *
       * The first restriction is already handled by isl_choose_image_alignment_el
       * but the second restriction, which is an extension of the first, only
       * applies to qpitch and must be applied here.
       *
       * The second restriction disappears on Gfx12.
       */
      assert(fmtl->bh == 4);
      pitch_el_rows = isl_align(pitch_el_rows, 256 / 4);
   }

   if (ISL_GFX_VER(dev) >= 9 &&
       info->dim == ISL_SURF_DIM_3D &&
       tile_info->tiling != ISL_TILING_LINEAR) {
      /* From the Skylake BSpec >> RENDER_SURFACE_STATE >> Surface QPitch:
       *
       *    Tile Mode != Linear: This field must be set to an integer multiple
       *    of the tile height
       */
      pitch_el_rows = isl_align(pitch_el_rows, tile_info->logical_extent_el.height);
   }

   return pitch_el_rows;
}

/**
 * A variant of isl_calc_phys_slice0_extent_sa() specific to
 * ISL_DIM_LAYOUT_GFX4_2D.
 */
static void
isl_calc_phys_slice0_extent_sa_gfx4_2d(
      const struct isl_device *dev,
      const struct isl_surf_init_info *restrict info,
      const struct isl_tile_info *tile_info,
      enum isl_msaa_layout msaa_layout,
      const struct isl_extent3d *image_align_sa,
      const struct isl_extent4d *phys_level0_sa,
      uint32_t miptail_start_level,
      struct isl_extent2d *phys_slice0_sa)
{
   ASSERTED const struct isl_format_layout *fmtl =
      isl_format_get_layout(info->format);

   if (info->levels == 1 && miptail_start_level > 0) {
      /* Do not pad the surface to the image alignment.
       *
       * For tiled surfaces, using a reduced alignment here avoids wasting CPU
       * cycles on the below mipmap layout caluclations. Reducing the
       * alignment here is safe because we later align the row pitch and array
       * pitch to the tile boundary. It is safe even for
       * ISL_MSAA_LAYOUT_INTERLEAVED, because phys_level0_sa is already scaled
       * to accommodate the interleaved samples.
       *
       * For linear surfaces, reducing the alignment here permits us to later
       * choose an arbitrary, non-aligned row pitch. If the surface backs
       * a VkBuffer, then an arbitrary pitch may be needed to accommodate
       * VkBufferImageCopy::bufferRowLength.
       */
      *phys_slice0_sa = (struct isl_extent2d) {
         .w = phys_level0_sa->w,
         .h = phys_level0_sa->h,
      };
      return;
   }

   uint32_t slice_top_w = 0;
   uint32_t slice_bottom_w = 0;
   uint32_t slice_left_h = 0;
   uint32_t slice_right_h = 0;

   uint32_t W0 = phys_level0_sa->w;
   uint32_t H0 = phys_level0_sa->h;

   for (uint32_t l = 0; l < info->levels; ++l) {
      uint32_t W = isl_minify(W0, l);
      uint32_t H = isl_minify(H0, l);

      uint32_t w = isl_align_npot(W, image_align_sa->w);
      uint32_t h = isl_align_npot(H, image_align_sa->h);

      if (l == 0) {
         slice_top_w = w;
         slice_left_h = h;
         slice_right_h = h;
      } else if (l == 1) {
         slice_bottom_w = w;
         slice_left_h += h;
      } else if (l == 2) {
         slice_bottom_w += w;
         slice_right_h += h;
      } else {
         slice_right_h += h;
      }

      if (l >= miptail_start_level) {
         assert(l == miptail_start_level);
         assert(tile_info->tiling == ISL_TILING_64 ||
                isl_tiling_is_std_y(tile_info->tiling));
         assert(w == tile_info->logical_extent_el.w * fmtl->bw);
         assert(h == tile_info->logical_extent_el.h * fmtl->bh);
         /* If we've gone into the miptail, we're done.  All higher miplevels
          * will be tucked into the same tile as this one.
          */
         break;
      }
   }

   *phys_slice0_sa = (struct isl_extent2d) {
      .w = MAX(slice_top_w, slice_bottom_w),
      .h = MAX(slice_left_h, slice_right_h),
   };
}

static void
isl_calc_phys_total_extent_el_gfx4_2d(
      const struct isl_device *dev,
      const struct isl_surf_init_info *restrict info,
      const struct isl_tile_info *tile_info,
      enum isl_msaa_layout msaa_layout,
      const struct isl_extent3d *image_align_sa,
      const struct isl_extent4d *phys_level0_sa,
      enum isl_array_pitch_span array_pitch_span,
      uint32_t miptail_start_level,
      uint32_t *array_pitch_el_rows,
      struct isl_extent4d *phys_total_el)
{
   const struct isl_format_layout *fmtl = isl_format_get_layout(info->format);

   struct isl_extent2d phys_slice0_sa;
   isl_calc_phys_slice0_extent_sa_gfx4_2d(dev, info, tile_info, msaa_layout,
                                          image_align_sa, phys_level0_sa,
                                          miptail_start_level,
                                          &phys_slice0_sa);
   *array_pitch_el_rows =
      isl_calc_array_pitch_el_rows_gfx4_2d(dev, info, tile_info,
                                           image_align_sa, phys_level0_sa,
                                           array_pitch_span,
                                           &phys_slice0_sa);

   if (tile_info->tiling == ISL_TILING_64 ||
       isl_tiling_is_std_y(tile_info->tiling)) {
      *phys_total_el = (struct isl_extent4d) {
         .w = isl_align_div_npot(phys_slice0_sa.w, fmtl->bw),
         .h = isl_align_div_npot(phys_slice0_sa.h, fmtl->bh),
         .d = isl_align_div_npot(phys_level0_sa->d, fmtl->bd),
         .a = phys_level0_sa->array_len,
      };
   } else {
      uint32_t array_len = MAX(phys_level0_sa->d, phys_level0_sa->a);
      *phys_total_el = (struct isl_extent4d) {
         .w = isl_align_div_npot(phys_slice0_sa.w, fmtl->bw),
         .h = *array_pitch_el_rows * (array_len - 1) +
              isl_align_div_npot(phys_slice0_sa.h, fmtl->bh),
         .d = 1,
         .a = 1,
      };
   }
}

/**
 * A variant of isl_calc_phys_slice0_extent_sa() specific to
 * ISL_DIM_LAYOUT_GFX4_3D.
 */
static void
isl_calc_phys_total_extent_el_gfx4_3d(
      const struct isl_device *dev,
      const struct isl_surf_init_info *restrict info,
      const struct isl_extent3d *image_align_sa,
      const struct isl_extent4d *phys_level0_sa,
      uint32_t *array_pitch_el_rows,
      struct isl_extent4d *phys_total_el)
{
   const struct isl_format_layout *fmtl = isl_format_get_layout(info->format);

   assert(info->samples == 1);

   if (info->dim != ISL_SURF_DIM_3D) {
      /* From the G45 PRM Vol. 1a, "6.17.4.1 Hardware Cube Map Layout":
       *
       * The cube face textures are stored in the same way as 3D surfaces
       * are stored (see section 6.17.5 for details).  For cube surfaces,
       * however, the depth is equal to the number of faces (always 6) and
       * is not reduced for each MIP.
       */
      assert(ISL_GFX_VER(dev) == 4);
      assert(info->usage & ISL_SURF_USAGE_CUBE_BIT);
      assert(phys_level0_sa->array_len == 6);
   } else {
      assert(phys_level0_sa->array_len == 1);
   }

   uint32_t total_w = 0;
   uint32_t total_h = 0;

   uint32_t W0 = phys_level0_sa->w;
   uint32_t H0 = phys_level0_sa->h;
   uint32_t D0 = phys_level0_sa->d;
   uint32_t A0 = phys_level0_sa->a;

   for (uint32_t l = 0; l < info->levels; ++l) {
      uint32_t level_w = isl_align_npot(isl_minify(W0, l), image_align_sa->w);
      uint32_t level_h = isl_align_npot(isl_minify(H0, l), image_align_sa->h);
      uint32_t level_d = info->dim == ISL_SURF_DIM_3D ? isl_minify(D0, l) : A0;

      uint32_t max_layers_horiz = MIN(level_d, 1u << l);
      uint32_t max_layers_vert = isl_align(level_d, 1u << l) / (1u << l);

      total_w = MAX(total_w, level_w * max_layers_horiz);
      total_h += level_h * max_layers_vert;
   }

   /* GFX4_3D layouts don't really have an array pitch since each LOD has a
    * different number of horizontal and vertical layers.  We have to set it
    * to something, so at least make it true for LOD0.
    */
   *array_pitch_el_rows =
      isl_align_npot(phys_level0_sa->h, image_align_sa->h) / fmtl->bw;
   *phys_total_el = (struct isl_extent4d) {
      .w = isl_assert_div(total_w, fmtl->bw),
      .h = isl_assert_div(total_h, fmtl->bh),
      .d = 1,
      .a = 1,
   };
}

/**
 * A variant of isl_calc_phys_slice0_extent_sa() specific to
 * ISL_DIM_LAYOUT_GFX6_STENCIL_HIZ.
 */
static void
isl_calc_phys_total_extent_el_gfx6_stencil_hiz(
      const struct isl_device *dev,
      const struct isl_surf_init_info *restrict info,
      const struct isl_tile_info *tile_info,
      const struct isl_extent3d *image_align_sa,
      const struct isl_extent4d *phys_level0_sa,
      uint32_t *array_pitch_el_rows,
      struct isl_extent4d *phys_total_el)
{
   const struct isl_format_layout *fmtl = isl_format_get_layout(info->format);

   const struct isl_extent2d tile_extent_sa = {
      .w = tile_info->logical_extent_el.w * fmtl->bw,
      .h = tile_info->logical_extent_el.h * fmtl->bh,
   };
   /* Tile size is a multiple of image alignment */
   assert(tile_extent_sa.w % image_align_sa->w == 0);
   assert(tile_extent_sa.h % image_align_sa->h == 0);

   const uint32_t W0 = phys_level0_sa->w;
   const uint32_t H0 = phys_level0_sa->h;

   /* Each image has the same height as LOD0 because the hardware thinks
    * everything is LOD0
    */
   const uint32_t H = isl_align(H0, image_align_sa->h) * phys_level0_sa->a;

   uint32_t total_top_w = 0;
   uint32_t total_bottom_w = 0;
   uint32_t total_h = 0;

   for (uint32_t l = 0; l < info->levels; ++l) {
      const uint32_t W = isl_minify(W0, l);

      const uint32_t w = isl_align(W, tile_extent_sa.w);
      const uint32_t h = isl_align(H, tile_extent_sa.h);

      if (l == 0) {
         total_top_w = w;
         total_h = h;
      } else if (l == 1) {
         total_bottom_w = w;
         total_h += h;
      } else {
         total_bottom_w += w;
      }
   }

   *array_pitch_el_rows =
      isl_assert_div(isl_align(H0, image_align_sa->h), fmtl->bh);
   *phys_total_el = (struct isl_extent4d) {
      .w = isl_assert_div(MAX(total_top_w, total_bottom_w), fmtl->bw),
      .h = isl_assert_div(total_h, fmtl->bh),
      .d = 1,
      .a = 1,
   };
}

/**
 * A variant of isl_calc_phys_slice0_extent_sa() specific to
 * ISL_DIM_LAYOUT_GFX9_1D.
 */
static void
isl_calc_phys_total_extent_el_gfx9_1d(
      const struct isl_device *dev,
      const struct isl_surf_init_info *restrict info,
      const struct isl_extent3d *image_align_sa,
      const struct isl_extent4d *phys_level0_sa,
      uint32_t *array_pitch_el_rows,
      struct isl_extent4d *phys_total_el)
{
   const struct isl_format_layout *fmtl = isl_format_get_layout(info->format);

   assert(phys_level0_sa->height == 1);
   assert(phys_level0_sa->depth == 1);
   assert(info->samples == 1);
   assert(image_align_sa->w >= fmtl->bw);

   uint32_t slice_w = 0;
   const uint32_t W0 = phys_level0_sa->w;

   for (uint32_t l = 0; l < info->levels; ++l) {
      uint32_t W = isl_minify(W0, l);
      uint32_t w = isl_align_npot(W, image_align_sa->w);

      slice_w += w;
   }

   *array_pitch_el_rows = 1;
   *phys_total_el = (struct isl_extent4d) {
      .w = isl_assert_div(slice_w, fmtl->bw),
      .h = phys_level0_sa->array_len,
      .d = 1,
      .a = 1,
   };
}

/**
 * Calculate the two-dimensional total physical extent of the surface, in
 * units of surface elements.
 */
static void
isl_calc_phys_total_extent_el(const struct isl_device *dev,
                              const struct isl_surf_init_info *restrict info,
                              const struct isl_tile_info *tile_info,
                              enum isl_dim_layout dim_layout,
                              enum isl_msaa_layout msaa_layout,
                              const struct isl_extent3d *image_align_sa,
                              const struct isl_extent4d *phys_level0_sa,
                              enum isl_array_pitch_span array_pitch_span,
                              uint32_t miptail_start_level,
                              uint32_t *array_pitch_el_rows,
                              struct isl_extent4d *phys_total_el)
{
   switch (dim_layout) {
   case ISL_DIM_LAYOUT_GFX9_1D:
      assert(array_pitch_span == ISL_ARRAY_PITCH_SPAN_COMPACT);
      isl_calc_phys_total_extent_el_gfx9_1d(dev, info,
                                            image_align_sa, phys_level0_sa,
                                            array_pitch_el_rows,
                                            phys_total_el);
      return;
   case ISL_DIM_LAYOUT_GFX4_2D:
      isl_calc_phys_total_extent_el_gfx4_2d(dev, info, tile_info, msaa_layout,
                                            image_align_sa, phys_level0_sa,
                                            array_pitch_span,
                                            miptail_start_level,
                                            array_pitch_el_rows,
                                            phys_total_el);
      return;
   case ISL_DIM_LAYOUT_GFX6_STENCIL_HIZ:
      assert(array_pitch_span == ISL_ARRAY_PITCH_SPAN_COMPACT);
      isl_calc_phys_total_extent_el_gfx6_stencil_hiz(dev, info, tile_info,
                                                     image_align_sa,
                                                     phys_level0_sa,
                                                     array_pitch_el_rows,
                                                     phys_total_el);
      return;
   case ISL_DIM_LAYOUT_GFX4_3D:
      assert(array_pitch_span == ISL_ARRAY_PITCH_SPAN_COMPACT);
      isl_calc_phys_total_extent_el_gfx4_3d(dev, info,
                                            image_align_sa, phys_level0_sa,
                                            array_pitch_el_rows,
                                            phys_total_el);
      return;
   }

   unreachable("invalid value for dim_layout");
}

static uint32_t
isl_calc_row_pitch_alignment(const struct isl_device *dev,
                             const struct isl_surf_init_info *surf_info,
                             const struct isl_tile_info *tile_info)
{
   if (tile_info->tiling != ISL_TILING_LINEAR) {
      /* According to BSpec: 44930, Gfx12's CCS-compressed surface pitches must
       * be 512B-aligned. CCS is only support on Y tilings.
       *
       * Only consider 512B alignment when :
       *    - AUX is not explicitly disabled
       *    - the caller has specified no pitch
       *
       * isl_surf_get_ccs_surf() will check that the main surface alignment
       * matches CCS expectations.
       */
      if (ISL_GFX_VER(dev) >= 12 &&
          isl_format_supports_ccs_e(dev->info, surf_info->format) &&
          tile_info->tiling != ISL_TILING_X &&
          !(surf_info->usage & ISL_SURF_USAGE_DISABLE_AUX_BIT) &&
          surf_info->row_pitch_B == 0) {
         return isl_align(tile_info->phys_extent_B.width, 512);
      }

      return tile_info->phys_extent_B.width;
   }

   /* We only support tiled fragment shading rate buffers. */
   assert((surf_info->usage & ISL_SURF_USAGE_CPB_BIT) == 0);

   /* From the Broadwel PRM >> Volume 2d: Command Reference: Structures >>
    * RENDER_SURFACE_STATE Surface Pitch (p349):
    *
    *    - For linear render target surfaces and surfaces accessed with the
    *      typed data port messages, the pitch must be a multiple of the
    *      element size for non-YUV surface formats.  Pitch must be
    *      a multiple of 2 * element size for YUV surface formats.
    *
    *    - [Requirements for SURFTYPE_BUFFER and SURFTYPE_STRBUF, which we
    *      ignore because isl doesn't do buffers.]
    *
    *    - For other linear surfaces, the pitch can be any multiple of
    *      bytes.
    */
   const struct isl_format_layout *fmtl = isl_format_get_layout(surf_info->format);
   const uint32_t bs = fmtl->bpb / 8;
   uint32_t alignment;

   if (surf_info->usage & ISL_SURF_USAGE_RENDER_TARGET_BIT) {
      if (isl_format_is_yuv(surf_info->format)) {
         alignment = 2 * bs;
      } else  {
         alignment = bs;
      }
   } else {
      alignment = 1;
   }

   /* From the Broadwell PRM >> Volume 2c: Command Reference: Registers >>
    * PRI_STRIDE Stride (p1254):
    *
    *    "When using linear memory, this must be at least 64 byte aligned."
    *
    * However, when displaying on NVIDIA and recent AMD GPUs via PRIME,
    * we need a larger pitch of 256 bytes.
    *
    * If the ISL caller didn't specify a row_pitch_B, then we should assume
    * the NVIDIA/AMD requirements. Otherwise, if we have a specified
    * row_pitch_B, this is probably because the caller is trying to import a
    * buffer. In that case we limit the minimum row pitch to the Intel HW
    * requirement.
    */
   if (surf_info->usage & ISL_SURF_USAGE_DISPLAY_BIT) {
      if (surf_info->row_pitch_B == 0)
         alignment = isl_align(alignment, 256);
      else
         alignment = isl_align(alignment, 64);
   }

   return alignment;
}

static uint32_t
isl_calc_linear_min_row_pitch(const struct isl_device *dev,
                              const struct isl_surf_init_info *info,
                              const struct isl_extent4d *phys_total_el,
                              uint32_t alignment_B)
{
   const struct isl_format_layout *fmtl = isl_format_get_layout(info->format);
   const uint32_t bs = fmtl->bpb / 8;

   return isl_align_npot(bs * phys_total_el->w, alignment_B);
}

static uint32_t
isl_calc_tiled_min_row_pitch(const struct isl_device *dev,
                             const struct isl_surf_init_info *surf_info,
                             const struct isl_tile_info *tile_info,
                             const struct isl_extent4d *phys_total_el,
                             uint32_t alignment_B)
{
   const struct isl_format_layout *fmtl = isl_format_get_layout(surf_info->format);

   assert(fmtl->bpb % tile_info->format_bpb == 0);

   const uint32_t tile_el_scale = fmtl->bpb / tile_info->format_bpb;
   const uint32_t total_w_tl =
      isl_align_div(phys_total_el->w * tile_el_scale,
                    tile_info->logical_extent_el.width);

   /* In some cases the alignment of the pitch might be > to the tile size
    * (for example Gfx12 CCS requires 512B alignment while the tile's width
    * can be 128B), so align the row pitch to the alignment.
    */
   assert(alignment_B >= tile_info->phys_extent_B.width);
   return isl_align(total_w_tl * tile_info->phys_extent_B.width, alignment_B);
}

static uint32_t
isl_calc_min_row_pitch(const struct isl_device *dev,
                       const struct isl_surf_init_info *surf_info,
                       const struct isl_tile_info *tile_info,
                       const struct isl_extent4d *phys_total_el,
                       uint32_t alignment_B)
{
   if (tile_info->tiling == ISL_TILING_LINEAR) {
      return isl_calc_linear_min_row_pitch(dev, surf_info, phys_total_el,
                                           alignment_B);
   } else {
      return isl_calc_tiled_min_row_pitch(dev, surf_info, tile_info,
                                          phys_total_el, alignment_B);
   }
}

/**
 * Is `pitch` in the valid range for a hardware bitfield, if the bitfield's
 * size is `bits` bits?
 *
 * Hardware pitch fields are offset by 1. For example, if the size of
 * RENDER_SURFACE_STATE::SurfacePitch is B bits, then the range of valid
 * pitches is [1, 2^b] inclusive.  If the surface pitch is N, then
 * RENDER_SURFACE_STATE::SurfacePitch must be set to N-1.
 */
static bool
pitch_in_range(uint32_t n, uint32_t bits)
{
   assert(n != 0);
   return likely(bits != 0 && 1 <= n && n <= (1 << bits));
}

void PRINTFLIKE(4, 5)
_isl_notify_failure(const struct isl_surf_init_info *surf_info,
                    const char *file, int line, const char *fmt, ...)
{
   if (!INTEL_DEBUG(DEBUG_ISL))
      return;

   char msg[512];
   va_list ap;
   va_start(ap, fmt);
   int ret = vsnprintf(msg, sizeof(msg), fmt, ap);
   assert(ret < sizeof(msg));
   va_end(ap);

#define PRINT_USAGE(bit, str) \
            (surf_info->usage & ISL_SURF_USAGE_##bit##_BIT) ? ("+"str) : ""
#define PRINT_TILING(bit, str) \
            (surf_info->tiling_flags & ISL_TILING_##bit##_BIT) ? ("+"str) : ""

   snprintf(msg + ret, sizeof(msg) - ret,
            " extent=%ux%ux%u dim=%s msaa=%ux levels=%u rpitch=%u fmt=%s "
            "usages=%s%s%s%s%s%s%s%s%s%s%s%s%s%s "
            "tiling_flags=%s%s%s%s%s%s%s%s%s%s%s%s%s",
            surf_info->width, surf_info->height,
            surf_info->dim == ISL_SURF_DIM_3D ?
            surf_info->depth : surf_info->array_len,
            surf_info->dim == ISL_SURF_DIM_1D ? "1d" :
            surf_info->dim == ISL_SURF_DIM_2D ? "2d" : "3d",
            surf_info->samples, surf_info->levels,
            surf_info->row_pitch_B,
            isl_format_get_name(surf_info->format) + strlen("ISL_FORMAT_"),

            PRINT_USAGE(RENDER_TARGET,   "rt"),
            PRINT_USAGE(DEPTH,           "depth"),
            PRINT_USAGE(STENCIL,         "stenc"),
            PRINT_USAGE(TEXTURE,         "tex"),
            PRINT_USAGE(CUBE,            "cube"),
            PRINT_USAGE(DISABLE_AUX,     "noaux"),
            PRINT_USAGE(DISPLAY,         "disp"),
            PRINT_USAGE(HIZ,             "hiz"),
            PRINT_USAGE(MCS,             "mcs"),
            PRINT_USAGE(CCS,             "ccs"),
            PRINT_USAGE(VERTEX_BUFFER,   "vb"),
            PRINT_USAGE(INDEX_BUFFER,    "ib"),
            PRINT_USAGE(CONSTANT_BUFFER, "const"),
            PRINT_USAGE(STAGING,         "stage"),

            PRINT_TILING(LINEAR,         "linear"),
            PRINT_TILING(W,              "W"),
            PRINT_TILING(X,              "X"),
            PRINT_TILING(Y0,             "Y0"),
            PRINT_TILING(SKL_Yf,         "skl-Yf"),
            PRINT_TILING(SKL_Ys,         "skl-Ys"),
            PRINT_TILING(ICL_Yf,         "icl-Yf"),
            PRINT_TILING(ICL_Ys,         "icl-Ys"),
            PRINT_TILING(4,              "4"),
            PRINT_TILING(64,             "64"),
            PRINT_TILING(HIZ,            "hiz"),
            PRINT_TILING(CCS,            "ccs"),
            PRINT_TILING(GFX12_CCS,      "ccs12"));

#undef PRINT_USAGE
#undef PRINT_TILING

   mesa_logd("%s:%i: %s", file, line, msg);
}

static bool
isl_calc_row_pitch(const struct isl_device *dev,
                   const struct isl_surf_init_info *surf_info,
                   const struct isl_tile_info *tile_info,
                   enum isl_dim_layout dim_layout,
                   const struct isl_extent4d *phys_total_el,
                   uint32_t *out_row_pitch_B)
{
   uint32_t alignment_B =
      isl_calc_row_pitch_alignment(dev, surf_info, tile_info);

   const uint32_t min_row_pitch_B =
      isl_calc_min_row_pitch(dev, surf_info, tile_info, phys_total_el,
                             alignment_B);

   if (surf_info->row_pitch_B != 0) {
      if (surf_info->row_pitch_B < min_row_pitch_B) {
         return notify_failure(surf_info,
                               "requested row pitch (%uB) less than minimum "
                               "allowed (%uB)",
                               surf_info->row_pitch_B, min_row_pitch_B);
      }

      if (surf_info->row_pitch_B % alignment_B != 0) {
         return notify_failure(surf_info,
                               "requested row pitch (%uB) doesn't satisfy the "
                               "minimum alignment requirement (%uB)",
                               surf_info->row_pitch_B, alignment_B);
      }
   }

   const uint32_t row_pitch_B =
      surf_info->row_pitch_B != 0 ? surf_info->row_pitch_B : min_row_pitch_B;

   const uint32_t row_pitch_tl = row_pitch_B / tile_info->phys_extent_B.width;

   if (row_pitch_B == 0)
      return notify_failure(surf_info, "calculated row pitch is zero");

   if (dim_layout == ISL_DIM_LAYOUT_GFX9_1D) {
      /* SurfacePitch is ignored for this layout. */
      goto done;
   }

   if ((surf_info->usage & (ISL_SURF_USAGE_RENDER_TARGET_BIT |
                            ISL_SURF_USAGE_TEXTURE_BIT |
                            ISL_SURF_USAGE_STORAGE_BIT)) &&
       !pitch_in_range(row_pitch_B, RENDER_SURFACE_STATE_SurfacePitch_bits(dev->info))) {
      return notify_failure(surf_info,
                            "row pitch (%uB) not in range of "
                            "RENDER_SURFACE_STATE::SurfacePitch",
                            row_pitch_B);
   }

   if ((surf_info->usage & (ISL_SURF_USAGE_CCS_BIT |
                            ISL_SURF_USAGE_MCS_BIT)) &&
       !pitch_in_range(row_pitch_tl, RENDER_SURFACE_STATE_AuxiliarySurfacePitch_bits(dev->info))) {
      return notify_failure(surf_info,
                            "row_pitch_tl=%u not in range of "
                            "RENDER_SURFACE_STATE::AuxiliarySurfacePitch",
                            row_pitch_tl);
   }

   if ((surf_info->usage & ISL_SURF_USAGE_DEPTH_BIT) &&
       !pitch_in_range(row_pitch_B, _3DSTATE_DEPTH_BUFFER_SurfacePitch_bits(dev->info))) {
      return notify_failure(surf_info,
                            "row pitch (%uB) not in range of "
                            "3DSTATE_DEPTH_BUFFER::SurfacePitch",
                            row_pitch_B);
   }

   if ((surf_info->usage & ISL_SURF_USAGE_HIZ_BIT) &&
       !pitch_in_range(row_pitch_B, _3DSTATE_HIER_DEPTH_BUFFER_SurfacePitch_bits(dev->info))) {
      return notify_failure(surf_info,
                            "row pitch (%uB) not in range of "
                            "3DSTATE_HIER_DEPTH_BUFFER::SurfacePitch",
                            row_pitch_B);
   }

   const uint32_t stencil_pitch_bits = dev->use_separate_stencil ?
      _3DSTATE_STENCIL_BUFFER_SurfacePitch_bits(dev->info) :
      _3DSTATE_DEPTH_BUFFER_SurfacePitch_bits(dev->info);

   if ((surf_info->usage & ISL_SURF_USAGE_STENCIL_BIT) &&
       !pitch_in_range(row_pitch_B, stencil_pitch_bits)) {
      return notify_failure(surf_info,
                            "row pitch (%uB) not in range of "
                            "3DSTATE_STENCIL_BUFFER/3DSTATE_DEPTH_BUFFER::SurfacePitch",
                            row_pitch_B);
   }

   if ((surf_info->usage & ISL_SURF_USAGE_CPB_BIT) &&
       !pitch_in_range(row_pitch_B, _3DSTATE_CPSIZE_CONTROL_BUFFER_SurfacePitch_bits(dev->info)))
      return false;

 done:
   *out_row_pitch_B = row_pitch_B;
   return true;
}

static bool
isl_calc_size(const struct isl_device *dev,
              const struct isl_surf_init_info *info,
              const struct isl_tile_info *tile_info,
              const struct isl_extent4d *phys_total_el,
              uint32_t array_pitch_el_rows,
              uint32_t row_pitch_B,
              uint64_t *out_size_B)
{
   uint64_t size_B;
   if (tile_info->tiling == ISL_TILING_LINEAR) {
      /* LINEAR tiling has no concept of intra-tile arrays */
      assert(phys_total_el->d == 1 && phys_total_el->a == 1);

      size_B = (uint64_t) row_pitch_B * phys_total_el->h;

   } else {
      /* Pitches must make sense with the tiling */
      assert(row_pitch_B % tile_info->phys_extent_B.width == 0);

      uint32_t array_slices, array_pitch_tl_rows;
      if (phys_total_el->d > 1) {
         assert(phys_total_el->a == 1);
         array_pitch_tl_rows = isl_assert_div(array_pitch_el_rows,
                                              tile_info->logical_extent_el.h);
         array_slices = isl_align_div(phys_total_el->d,
                                      tile_info->logical_extent_el.d);
      } else if (phys_total_el->a > 1) {
         assert(phys_total_el->d == 1);
         array_pitch_tl_rows = isl_assert_div(array_pitch_el_rows,
                                              tile_info->logical_extent_el.h);
         array_slices = isl_align_div(phys_total_el->a,
                                      tile_info->logical_extent_el.a);
      } else {
         assert(phys_total_el->d == 1 && phys_total_el->a == 1);
         array_pitch_tl_rows = 0;
         array_slices = 1;
      }

      const uint32_t total_h_tl =
         (array_slices - 1) * array_pitch_tl_rows +
         isl_align_div(phys_total_el->h, tile_info->logical_extent_el.height);

      size_B = (uint64_t) total_h_tl * tile_info->phys_extent_B.height *
               row_pitch_B;
   }

   /* If for some reason we can't support the appropriate tiling format and
    * end up falling to linear or some other format, make sure the image size
    * and alignment are aligned to the expected block size so we can at least
    * do opaque binds.
    */
   if (info->usage & ISL_SURF_USAGE_SPARSE_BIT)
      size_B = isl_align(size_B, 64 * 1024);

   /* Pre-gfx9: from the Broadwell PRM Vol 5, Surface Layout:
    *    "In addition to restrictions on maximum height, width, and depth,
    *     surfaces are also restricted to a maximum size in bytes. This
    *     maximum is 2 GB for all products and all surface types."
    *
    * gfx9-10: from the Skylake PRM Vol 5, Maximum Surface Size in Bytes:
    *    "In addition to restrictions on maximum height, width, and depth,
    *     surfaces are also restricted to a maximum size of 2^38 bytes.
    *     All pixels within the surface must be contained within 2^38 bytes
    *     of the base address."
    *
    * gfx11+ platforms raised this limit to 2^44 bytes.
    */
   uint64_t max_surface_B = 1ull << (ISL_GFX_VER(dev) >= 11 ? 44 :
                                     ISL_GFX_VER(dev) >= 9 ? 38 : 31);
   if (size_B > max_surface_B) {
      return notify_failure(
         info,
         "calculated size (%"PRIu64"B) exceeds platform limit of %"PRIu64"B",
         size_B, max_surface_B);
   }

   *out_size_B = size_B;
   return true;
}

static uint32_t
isl_calc_base_alignment(const struct isl_device *dev,
                        const struct isl_surf_init_info *info,
                        const struct isl_tile_info *tile_info)
{
   uint32_t base_alignment_B;
   if (tile_info->tiling == ISL_TILING_LINEAR) {
      /* From the Broadwell PRM Vol 2d,
       * RENDER_SURFACE_STATE::SurfaceBaseAddress:
       *
       *    "The Base Address for linear render target surfaces and surfaces
       *    accessed with the typed surface read/write data port messages must
       *    be element-size aligned, for non-YUV surface formats, or a
       *    multiple of 2 element-sizes for YUV surface formats. Other linear
       *    surfaces have no alignment requirements (byte alignment is
       *    sufficient.)"
       */
      base_alignment_B = MAX(1, info->min_alignment_B);
      if (info->usage & ISL_SURF_USAGE_RENDER_TARGET_BIT) {
         if (isl_format_is_yuv(info->format)) {
            base_alignment_B =
               MAX(base_alignment_B, tile_info->format_bpb / 4);
         } else {
            base_alignment_B =
               MAX(base_alignment_B, tile_info->format_bpb / 8);
         }
      }
      base_alignment_B = isl_round_up_to_power_of_two(base_alignment_B);

      /* From the Skylake PRM Vol 2c, PLANE_STRIDE::Stride:
       *
       *     "For Linear memory, this field specifies the stride in chunks of
       *     64 bytes (1 cache line)."
       */
      if (isl_surf_usage_is_display(info->usage))
         base_alignment_B = MAX(base_alignment_B, 64);
   } else {
      const uint32_t tile_size_B = tile_info->phys_extent_B.width *
                                   tile_info->phys_extent_B.height;
      assert(isl_is_pow2(info->min_alignment_B) && isl_is_pow2(tile_size_B));
      base_alignment_B = MAX(info->min_alignment_B, tile_size_B);

      /* The diagram in the Bspec section Memory Compression - Gfx12, shows
       * that the CCS is indexed in 256B chunks. However, the
       * PLANE_AUX_DIST::Auxiliary Surface Distance field is in units of 4K
       * pages. We currently don't assign the usage field like we do for main
       * surfaces, so just use 4K for now.
       */
      if (tile_info->tiling == ISL_TILING_GFX12_CCS)
         base_alignment_B = MAX(base_alignment_B, 4096);

      if (dev->info->has_aux_map &&
          (isl_format_supports_ccs_d(dev->info, info->format) ||
           isl_format_supports_ccs_e(dev->info, info->format)) &&
          !INTEL_DEBUG(DEBUG_NO_CCS) &&
          !(info->usage & ISL_SURF_USAGE_DISABLE_AUX_BIT)) {
         /* Wa_22015614752:
          *
          * Due to L3 cache being tagged with (engineID, vaID) and the CCS
          * block/cacheline being 256 bytes, 2 engines accessing a 64Kb range
          * with compression will generate 2 different CCS cacheline entries
          * in L3, this will lead to corruptions. To avoid this, we need to
          * ensure 2 images do not share a 256 bytes CCS cacheline. With a
          * ratio of compression of 1/256, this is 64Kb alignment (even for
          * Tile4...)
          *
          * ATS-M PRMS, Vol 2a: Command Reference: Instructions,
          * XY_CTRL_SURF_COPY_BLT, "Size of Control Surface Copy" field, the
          * CCS blocks are 256 bytes :
          *
          *    "This field indicates size of the Control Surface or CCS copy.
          *     It is expressed in terms of number of 256B block of CCS, where
          *     each 256B block of CCS corresponds to 64KB of main surface."
          */
         if (intel_needs_workaround(dev->info, 22015614752)) {
            base_alignment_B = MAX(base_alignment_B,
                                   256 /* cacheline */ * 256 /* AUX ratio */);
         }

         /* Platforms using an aux map require that images be
          * granularity-aligned if they're going to used with CCS. This is
          * because the Aux translation table maps main surface addresses to
          * aux addresses at a granularity in the main surface. Because we
          * don't know for sure in ISL if a surface will use CCS, we have to
          * guess based on the DISABLE_AUX usage bit. The one thing we do know
          * is that we haven't enable CCS on linear images yet so we can avoid
          * the extra alignment there.
          */
         base_alignment_B = MAX(base_alignment_B, dev->info->verx10 >= 125 ?
                                1024 * 1024 : 64 * 1024);
      }
   }

   /* If for some reason we can't support the appropriate tiling format and
    * end up falling to linear or some other format, make sure the image size
    * and alignment are aligned to the expected block size so we can at least
    * do opaque binds.
    */
   if (info->usage & ISL_SURF_USAGE_SPARSE_BIT)
      base_alignment_B = MAX(base_alignment_B, 64 * 1024);

   return base_alignment_B;
}

bool
isl_surf_init_s(const struct isl_device *dev,
                struct isl_surf *surf,
                const struct isl_surf_init_info *restrict info)
{
   /* Some sanity checks */
   assert(!(info->usage & ISL_SURF_USAGE_CPB_BIT) ||
          dev->info->has_coarse_pixel_primitive_and_cb);

   const struct isl_format_layout *fmtl = isl_format_get_layout(info->format);

   const struct isl_extent4d logical_level0_px = {
      .w = info->width,
      .h = info->height,
      .d = info->depth,
      .a = info->array_len,
   };

   enum isl_tiling tiling;
   if (!isl_surf_choose_tiling(dev, info, &tiling))
      return false;

   const enum isl_dim_layout dim_layout =
      isl_surf_choose_dim_layout(dev, info->dim, tiling, info->usage);

   enum isl_msaa_layout msaa_layout;
   if (!isl_choose_msaa_layout(dev, info, tiling, &msaa_layout))
       return false;

   struct isl_tile_info tile_info;
   isl_tiling_get_info(tiling, info->dim, msaa_layout, fmtl->bpb,
                       info->samples, &tile_info);

   struct isl_extent3d image_align_el;
   isl_choose_image_alignment_el(dev, info, tiling, dim_layout, msaa_layout,
                                 &image_align_el);

   struct isl_extent3d image_align_sa =
      isl_extent3d_el_to_sa(info->format, image_align_el);

   struct isl_extent4d phys_level0_sa;
   isl_calc_phys_level0_extent_sa(dev, info, dim_layout, tiling, msaa_layout,
                                  &phys_level0_sa);

   enum isl_array_pitch_span array_pitch_span =
      isl_choose_array_pitch_span(dev, info, dim_layout, &phys_level0_sa);

   uint32_t miptail_start_level =
      isl_choose_miptail_start_level(dev, info, &tile_info);

   uint32_t array_pitch_el_rows;
   struct isl_extent4d phys_total_el;
   isl_calc_phys_total_extent_el(dev, info, &tile_info,
                                 dim_layout, msaa_layout,
                                 &image_align_sa, &phys_level0_sa,
                                 array_pitch_span, miptail_start_level,
                                 &array_pitch_el_rows,
                                 &phys_total_el);

   uint32_t row_pitch_B;
   if (!isl_calc_row_pitch(dev, info, &tile_info, dim_layout,
                           &phys_total_el, &row_pitch_B))
      return false;

   uint64_t size_B;
   if (!isl_calc_size(dev, info, &tile_info, &phys_total_el,
                      array_pitch_el_rows, row_pitch_B, &size_B))
      return false;

   const uint32_t base_alignment_B =
      isl_calc_base_alignment(dev, info, &tile_info);

   *surf = (struct isl_surf) {
      .dim = info->dim,
      .dim_layout = dim_layout,
      .msaa_layout = msaa_layout,
      .tiling = tiling,
      .format = info->format,

      .levels = info->levels,
      .samples = info->samples,

      .image_alignment_el = image_align_el,
      .logical_level0_px = logical_level0_px,
      .phys_level0_sa = phys_level0_sa,

      .size_B = size_B,
      .alignment_B = base_alignment_B,
      .row_pitch_B = row_pitch_B,
      .array_pitch_el_rows = array_pitch_el_rows,
      .array_pitch_span = array_pitch_span,
      .miptail_start_level = miptail_start_level,

      .usage = info->usage,
   };

   return true;
}

void
isl_surf_get_tile_info(const struct isl_surf *surf,
                       struct isl_tile_info *tile_info)
{
   const struct isl_format_layout *fmtl = isl_format_get_layout(surf->format);
   isl_tiling_get_info(surf->tiling, surf->dim, surf->msaa_layout, fmtl->bpb,
                       surf->samples, tile_info);
}

bool
isl_surf_get_hiz_surf(const struct isl_device *dev,
                      const struct isl_surf *surf,
                      struct isl_surf *hiz_surf)
{
   if (INTEL_DEBUG(DEBUG_NO_HIZ))
      return false;

   /* HiZ support does not exist prior to Gfx5 */
   if (ISL_GFX_VER(dev) < 5)
      return false;

   if (!isl_surf_usage_is_depth(surf->usage))
      return false;

   /* From the Sandy Bridge PRM, Vol 2 Part 1,
    * 3DSTATE_DEPTH_BUFFER::Hierarchical Depth Buffer Enable,
    *
    *    If this field is enabled, the Surface Format of the depth buffer
    *    cannot be D32_FLOAT_S8X24_UINT or D24_UNORM_S8_UINT. Use of stencil
    *    requires the separate stencil buffer.
    *
    * On SNB+, HiZ can't be used with combined depth-stencil buffers.
    */
   if (isl_surf_usage_is_stencil(surf->usage))
      return false;

   /* Multisampled depth is always interleaved */
   assert(surf->msaa_layout == ISL_MSAA_LAYOUT_NONE ||
          surf->msaa_layout == ISL_MSAA_LAYOUT_INTERLEAVED);

   /* From the Broadwell PRM Vol. 7, "Hierarchical Depth Buffer":
    *
    *    "The Surface Type, Height, Width, Depth, Minimum Array Element, Render
    *    Target View Extent, and Depth Coordinate Offset X/Y of the
    *    hierarchical depth buffer are inherited from the depth buffer. The
    *    height and width of the hierarchical depth buffer that must be
    *    allocated are computed by the following formulas, where HZ is the
    *    hierarchical depth buffer and Z is the depth buffer. The Z_Height,
    *    Z_Width, and Z_Depth values given in these formulas are those present
    *    in 3DSTATE_DEPTH_BUFFER incremented by one.
    *
    *    "The value of Z_Height and Z_Width must each be multiplied by 2 before
    *    being applied to the table below if Number of Multisamples is set to
    *    NUMSAMPLES_4. The value of Z_Height must be multiplied by 2 and
    *    Z_Width must be multiplied by 4 before being applied to the table
    *    below if Number of Multisamples is set to NUMSAMPLES_8."
    *
    * In the Sky Lake PRM, the second paragraph is gone.  This means that,
    * from Sandy Bridge through Broadwell, HiZ compresses samples in the
    * primary depth surface.  On Sky Lake and onward, HiZ compresses pixels.
    *
    * There are a number of different ways that this discrepancy could be
    * handled.  The way we have chosen is to simply make MSAA HiZ have the
    * same number of samples as the parent surface pre-Sky Lake and always be
    * single-sampled on Sky Lake and above.  Since the block sizes of
    * compressed formats are given in samples, this neatly handles everything
    * without the need for additional HiZ formats with different block sizes
    * on SKL+.
    */
   const unsigned samples = ISL_GFX_VER(dev) >= 9 ? 1 : surf->samples;

   const enum isl_format format =
      ISL_GFX_VERX10(dev) >= 125 ? ISL_FORMAT_GFX125_HIZ : ISL_FORMAT_HIZ;

   return isl_surf_init(dev, hiz_surf,
                        .dim = surf->dim,
                        .format = format,
                        .width = surf->logical_level0_px.width,
                        .height = surf->logical_level0_px.height,
                        .depth = surf->logical_level0_px.depth,
                        .levels = surf->levels,
                        .array_len = surf->logical_level0_px.array_len,
                        .samples = samples,
                        .usage = ISL_SURF_USAGE_HIZ_BIT,
                        .tiling_flags = ISL_TILING_HIZ_BIT);
}

bool
isl_surf_get_mcs_surf(const struct isl_device *dev,
                      const struct isl_surf *surf,
                      struct isl_surf *mcs_surf)
{
   /* It must be multisampled with an array layout */
   if (surf->msaa_layout != ISL_MSAA_LAYOUT_ARRAY)
      return false;

   /* On Gfx12+ this format is not listed in TGL PRMs, Volume 2b: Command
    * Reference: Enumerations, RenderCompressionFormat
    */
   if (ISL_GFX_VER(dev) >= 12 &&
       surf->format == ISL_FORMAT_R9G9B9E5_SHAREDEXP)
      return false;

   /* The following are true of all multisampled surfaces */
   assert(surf->samples > 1);
   assert(surf->dim == ISL_SURF_DIM_2D);
   assert(surf->levels == 1);
   assert(surf->logical_level0_px.depth == 1);
   assert(isl_format_supports_multisampling(dev->info, surf->format));

   enum isl_format mcs_format;
   switch (surf->samples) {
   case 2:  mcs_format = ISL_FORMAT_MCS_2X;  break;
   case 4:  mcs_format = ISL_FORMAT_MCS_4X;  break;
   case 8:  mcs_format = ISL_FORMAT_MCS_8X;  break;
   case 16: mcs_format = ISL_FORMAT_MCS_16X; break;
   default:
      unreachable("Invalid sample count");
   }

   return isl_surf_init(dev, mcs_surf,
                        .dim = ISL_SURF_DIM_2D,
                        .format = mcs_format,
                        .width = surf->logical_level0_px.width,
                        .height = surf->logical_level0_px.height,
                        .depth = 1,
                        .levels = 1,
                        .array_len = surf->logical_level0_px.array_len,
                        .samples = 1, /* MCS surfaces are really single-sampled */
                        .usage = ISL_SURF_USAGE_MCS_BIT,
                        .tiling_flags = ISL_TILING_ANY_MASK);
}

bool
isl_surf_supports_ccs(const struct isl_device *dev,
                      const struct isl_surf *surf,
                      const struct isl_surf *hiz_or_mcs_surf)
{
   if (INTEL_DEBUG(DEBUG_NO_CCS))
      return false;

   if (surf->usage & ISL_SURF_USAGE_DISABLE_AUX_BIT)
      return false;

   if (!isl_format_supports_ccs_d(dev->info, surf->format) &&
       !isl_format_supports_ccs_e(dev->info, surf->format))
      return false;

   /* From the Ivy Bridge PRM, Vol2 Part1 11.7 "MCS Buffer for Render
    * Target(s)", beneath the "Fast Color Clear" bullet (p326):
    *
    *     - Support is limited to tiled render targets.
    *
    * From the Skylake documentation, it is made clear that X-tiling is no
    * longer supported:
    *
    *     - MCS and Lossless compression is supported for
    *       TiledY/TileYs/TileYf non-MSRTs only.
    *
    * From the BSpec (44930) for Gfx12:
    *
    *    Linear CCS is only allowed for Untyped Buffers but only via HDC
    *    Data-Port messages.
    *
    * We never use untyped messages on surfaces created by ISL on Gfx9+ so
    * this means linear is out on Gfx12+ as well.
    */
   if (surf->tiling == ISL_TILING_LINEAR)
      return false;

   /* TODO: Disable for now, as we're not sure about the meaning of
    * 3DSTATE_CPSIZE_CONTROL_BUFFER::CPCBCompressionEnable
    */
   if (isl_surf_usage_is_cpb(surf->usage))
      return false;

   /* SKL PRMs, Volume 5: Memory Views, Tiling and Mip Tails for 2D Surfaces:
    *
    *    "Lossless compression must not be used on surfaces which have MIP
    *     Tail which contains MIPs for Slots greater than 11."
    */
   if (surf->miptail_start_level < surf->levels) {
      const uint32_t miptail_levels = surf->levels - surf->miptail_start_level;
      if (miptail_levels + isl_get_miptail_base_row(surf->tiling) > 11) {
         assert(surf->tiling == ISL_TILING_64 || isl_tiling_is_std_y(surf->tiling));
         return false;
      }
   }

   /* From the workarounds section in the SKL PRM:
    *
    *    "RCC cacheline is composed of X-adjacent 64B fragments instead of
    *     memory adjacent. This causes a single 128B cacheline to straddle
    *     multiple LODs inside the TYF MIPtail for 3D surfaces (beyond a
    *     certain slot number), leading to corruption when CCS is enabled
    *     for these LODs and RT is later bound as texture. WA: If
    *     RENDER_SURFACE_STATE.Surface Type = 3D and
    *     RENDER_SURFACE_STATE.Auxiliary Surface Mode != AUX_NONE and
    *     RENDER_SURFACE_STATE.Tiled ResourceMode is TYF or TYS, Set the
    *     value of RENDER_SURFACE_STATE.Mip Tail Start LOD to a mip that
    *     larger than those present in the surface (i.e. 15)"
    *
    * We simply disallow CCS on 3D surfaces with miptails.
    *
    * Referred to as Wa_1207137018 on ICL+
    */
   if (ISL_GFX_VERX10(dev) <= 120 &&
       surf->dim == ISL_SURF_DIM_3D &&
       surf->miptail_start_level < surf->levels) {
      assert(isl_tiling_is_std_y(surf->tiling));
      return false;
   }

   /* TODO: add CCS support for Ys/Yf */
   if (isl_tiling_is_std_y(surf->tiling))
      return false;

   if (ISL_GFX_VER(dev) >= 12) {
      if (isl_surf_usage_is_stencil(surf->usage)) {
         /* HiZ and MCS aren't allowed with stencil */
         assert(hiz_or_mcs_surf == NULL || hiz_or_mcs_surf->size_B == 0);

         /* Multi-sampled stencil cannot have CCS */
         if (surf->samples > 1)
            return false;
      } else if (isl_surf_usage_is_depth(surf->usage)) {
         const struct isl_surf *hiz_surf = hiz_or_mcs_surf;

         /* With depth surfaces, HIZ is required for CCS. */
         if (hiz_surf == NULL || hiz_surf->size_B == 0)
            return false;

         assert(hiz_surf->usage & ISL_SURF_USAGE_HIZ_BIT);
         assert(hiz_surf->tiling == ISL_TILING_HIZ);
         assert(isl_format_is_hiz(hiz_surf->format));
      } else if (surf->samples > 1) {
         const struct isl_surf *mcs_surf = hiz_or_mcs_surf;

         /* With multisampled color, CCS requires MCS */
         if (mcs_surf == NULL || mcs_surf->size_B == 0)
            return false;

         assert(mcs_surf->usage & ISL_SURF_USAGE_MCS_BIT);
         assert(isl_format_is_mcs(mcs_surf->format));
      } else {
         /* Single-sampled color can't have MCS or HiZ */
         assert(hiz_or_mcs_surf == NULL || hiz_or_mcs_surf->size_B == 0);
      }

      /* On Gfx12, all CCS-compressed surface pitches must be multiples of
       * 512B.
       */
      if (surf->row_pitch_B % 512 != 0)
         return false;

      /* TODO: According to Wa_1406738321, 3D textures need a blit to a new
       * surface in order to perform a resolve. For now, just disable CCS.
       */
      if (surf->dim == ISL_SURF_DIM_3D)
         return false;

      /* BSpec 44930: (Gfx12, Gfx12.5)
       *
       *    "Compression of 3D Ys surfaces with 64 or 128 bpp is not supported
       *     in Gen12. Moreover, "Render Target Fast-clear Enable" command is
       *     not supported for any 3D Ys surfaces. except when Surface is a
       *     Procdural Texture."
       *
       * Since the note applies to MTL, we apply this to TILE64 too.
       */
      uint32_t format_bpb = isl_format_get_layout(surf->format)->bpb;
      if (ISL_GFX_VER(dev) == 12 &&
          surf->dim == ISL_SURF_DIM_3D &&
          (surf->tiling == ISL_TILING_ICL_Ys ||
           surf->tiling == ISL_TILING_64) &&
          (format_bpb == 64 || format_bpb == 128))
         return false;

      /* TODO: Handle the other tiling formats */
      if (surf->tiling != ISL_TILING_Y0 && surf->tiling != ISL_TILING_4 &&
          surf->tiling != ISL_TILING_64)
         return false;

      /* TODO: Handle single-sampled Tile64. */
      if (surf->samples == 1 && surf->tiling == ISL_TILING_64)
         return false;
   } else {
      /* ISL_GFX_VER(dev) < 12 */
      if (surf->samples > 1)
         return false;

      /* CCS is only for color images on Gfx7-11 */
      if (isl_surf_usage_is_depth_or_stencil(surf->usage))
         return false;

      /* We're single-sampled color so having HiZ or MCS makes no sense */
      assert(hiz_or_mcs_surf == NULL || hiz_or_mcs_surf->size_B == 0);

      /* The PRM doesn't say this explicitly, but fast-clears don't appear to
       * work for 3D textures until gfx9 where the layout of 3D textures
       * changes to match 2D array textures.
       */
      if (ISL_GFX_VER(dev) <= 8 && surf->dim != ISL_SURF_DIM_2D)
         return false;

      /* From the HSW PRM Volume 7: 3D-Media-GPGPU, page 652 (Color Clear of
       * Non-MultiSampler Render Target Restrictions):
       *
       *    "Support is for non-mip-mapped and non-array surface types only."
       *
       * This restriction is lifted on gfx8+.  Technically, it may be possible
       * to create a CCS for an arrayed or mipmapped image and only enable
       * CCS_D when rendering to the base slice.  However, there is no
       * documentation tell us what the hardware would do in that case or what
       * it does if you walk off the bases slice.  (Does it ignore CCS or does
       * it start scribbling over random memory?)  We play it safe and just
       * follow the docs and don't allow CCS_D for arrayed or mip-mapped
       * surfaces.
       */
      if (ISL_GFX_VER(dev) <= 7 &&
          (surf->levels > 1 || surf->logical_level0_px.array_len > 1))
         return false;

      /* From the Skylake documentation, it is made clear that X-tiling is no
       * longer supported:
       *
       *     - MCS and Lossless compression is supported for
       *     TiledY/TileYs/TileYf non-MSRTs only.
       */
      if (ISL_GFX_VER(dev) >= 9 && !isl_tiling_is_any_y(surf->tiling))
         return false;
   }

   return true;
}

bool
isl_surf_get_ccs_surf(const struct isl_device *dev,
                      const struct isl_surf *surf,
                      const struct isl_surf *hiz_or_mcs_surf,
                      struct isl_surf *ccs_surf,
                      uint32_t row_pitch_B)
{
   if (!isl_surf_supports_ccs(dev, surf, hiz_or_mcs_surf))
      return false;

   if (ISL_GFX_VER(dev) >= 12) {
      enum isl_format ccs_format;
      switch (isl_format_get_layout(surf->format)->bpb) {
      case 8:     ccs_format = ISL_FORMAT_GFX12_CCS_8BPP_Y0;    break;
      case 16:    ccs_format = ISL_FORMAT_GFX12_CCS_16BPP_Y0;   break;
      case 32:    ccs_format = ISL_FORMAT_GFX12_CCS_32BPP_Y0;   break;
      case 64:    ccs_format = ISL_FORMAT_GFX12_CCS_64BPP_Y0;   break;
      case 128:   ccs_format = ISL_FORMAT_GFX12_CCS_128BPP_Y0;  break;
      default:
         return false;
      }

      /* On Gfx12, the CCS is a scaled-down version of the main surface. We
       * model this as the CCS compressing a 2D-view of the entire surface.
       */
      const bool ok =
         isl_surf_init(dev, ccs_surf,
                       .dim = ISL_SURF_DIM_2D,
                       .format = ccs_format,
                       .width = isl_surf_get_row_pitch_el(surf),
                       .height = surf->size_B / surf->row_pitch_B,
                       .depth = 1,
                       .levels = 1,
                       .array_len = 1,
                       .samples = 1,
                       .row_pitch_B = row_pitch_B,
                       .usage = ISL_SURF_USAGE_CCS_BIT,
                       .tiling_flags = ISL_TILING_GFX12_CCS_BIT);
      assert(!ok || ccs_surf->size_B == surf->size_B / 256);
      return ok;
   } else {
      enum isl_format ccs_format;
      if (ISL_GFX_VER(dev) >= 9) {
         switch (isl_format_get_layout(surf->format)->bpb) {
         case 32:    ccs_format = ISL_FORMAT_GFX9_CCS_32BPP;   break;
         case 64:    ccs_format = ISL_FORMAT_GFX9_CCS_64BPP;   break;
         case 128:   ccs_format = ISL_FORMAT_GFX9_CCS_128BPP;  break;
         default:    unreachable("Unsupported CCS format");
            return false;
         }
      } else if (surf->tiling == ISL_TILING_Y0) {
         switch (isl_format_get_layout(surf->format)->bpb) {
         case 32:    ccs_format = ISL_FORMAT_GFX7_CCS_32BPP_Y;    break;
         case 64:    ccs_format = ISL_FORMAT_GFX7_CCS_64BPP_Y;    break;
         case 128:   ccs_format = ISL_FORMAT_GFX7_CCS_128BPP_Y;   break;
         default:    unreachable("Unsupported CCS format");
         }
      } else if (surf->tiling == ISL_TILING_X) {
         switch (isl_format_get_layout(surf->format)->bpb) {
         case 32:    ccs_format = ISL_FORMAT_GFX7_CCS_32BPP_X;    break;
         case 64:    ccs_format = ISL_FORMAT_GFX7_CCS_64BPP_X;    break;
         case 128:   ccs_format = ISL_FORMAT_GFX7_CCS_128BPP_X;   break;
         default:    unreachable("Unsupported CCS format");
         }
      } else {
         unreachable("Invalid tiling format");
      }

      return isl_surf_init(dev, ccs_surf,
                           .dim = surf->dim,
                           .format = ccs_format,
                           .width = surf->logical_level0_px.width,
                           .height = surf->logical_level0_px.height,
                           .depth = surf->logical_level0_px.depth,
                           .levels = surf->levels,
                           .array_len = surf->logical_level0_px.array_len,
                           .samples = 1,
                           .row_pitch_B = row_pitch_B,
                           .usage = ISL_SURF_USAGE_CCS_BIT,
                           .tiling_flags = ISL_TILING_CCS_BIT);
   }
}

#define isl_genX_call(dev, func, ...)              \
   switch (ISL_GFX_VERX10(dev)) {                  \
   case 40:                                        \
      isl_gfx4_##func(__VA_ARGS__);                \
      break;                                       \
   case 45:                                        \
      /* G45 surface state is the same as gfx5 */  \
   case 50:                                        \
      isl_gfx5_##func(__VA_ARGS__);                \
      break;                                       \
   case 60:                                        \
      isl_gfx6_##func(__VA_ARGS__);                \
      break;                                       \
   case 70:                                        \
      isl_gfx7_##func(__VA_ARGS__);                \
      break;                                       \
   case 75:                                        \
      isl_gfx75_##func(__VA_ARGS__);               \
      break;                                       \
   case 80:                                        \
      isl_gfx8_##func(__VA_ARGS__);                \
      break;                                       \
   case 90:                                        \
      isl_gfx9_##func(__VA_ARGS__);                \
      break;                                       \
   case 110:                                       \
      isl_gfx11_##func(__VA_ARGS__);               \
      break;                                       \
   case 120:                                       \
      isl_gfx12_##func(__VA_ARGS__);               \
      break;                                       \
   case 125:                                       \
      isl_gfx125_##func(__VA_ARGS__);              \
      break;                                       \
   case 200:                                       \
      isl_gfx20_##func(__VA_ARGS__);               \
      break;                                       \
   default:                                        \
      assert(!"Unknown hardware generation");      \
   }

/**
 * A variant of isl_surf_get_image_offset_sa() specific to
 * ISL_DIM_LAYOUT_GFX4_2D.
 */
static void
get_image_offset_sa_gfx4_2d(const struct isl_surf *surf,
                            uint32_t level, uint32_t logical_array_layer,
                            uint32_t *x_offset_sa,
                            uint32_t *y_offset_sa,
                            uint32_t *z_offset_sa,
                            uint32_t *array_offset)
{
   assert(level < surf->levels);
   if (surf->dim == ISL_SURF_DIM_3D)
      assert(logical_array_layer < surf->logical_level0_px.depth);
   else
      assert(logical_array_layer < surf->logical_level0_px.array_len);

   const struct isl_extent3d image_align_sa =
      isl_surf_get_image_alignment_sa(surf);

   const uint32_t W0 = surf->phys_level0_sa.width;
   const uint32_t H0 = surf->phys_level0_sa.height;

   const uint32_t phys_layer = logical_array_layer *
      (surf->msaa_layout == ISL_MSAA_LAYOUT_ARRAY ? surf->samples : 1);

   uint32_t x = 0, y;
   if (isl_tiling_is_std_y(surf->tiling) || surf->tiling == ISL_TILING_64) {
      y = 0;
      if (surf->dim == ISL_SURF_DIM_3D) {
         *z_offset_sa = logical_array_layer;
         *array_offset = 0;
      } else {
         *z_offset_sa = 0;
         *array_offset = phys_layer;
      }
   } else {
      y = phys_layer * isl_surf_get_array_pitch_sa_rows(surf);
      *z_offset_sa = 0;
      *array_offset = 0;
   }

   for (uint32_t l = 0; l < MIN(level, surf->miptail_start_level); ++l) {
      if (l == 1) {
         uint32_t W = isl_minify(W0, l);
         x += isl_align_npot(W, image_align_sa.w);
      } else {
         uint32_t H = isl_minify(H0, l);
         y += isl_align_npot(H, image_align_sa.h);
      }
   }

   *x_offset_sa = x;
   *y_offset_sa = y;

   if (level >= surf->miptail_start_level) {
      const struct isl_format_layout *fmtl =
         isl_format_get_layout(surf->format);

      uint32_t tail_offset_x_el, tail_offset_y_el, tail_offset_z_el;
      isl_get_miptail_level_offset_el(surf->tiling, surf->dim,
                                      fmtl->bpb,
                                      level - surf->miptail_start_level,
                                      &tail_offset_x_el,
                                      &tail_offset_y_el,
                                      &tail_offset_z_el);
      *x_offset_sa += tail_offset_x_el * fmtl->bw;
      *y_offset_sa += tail_offset_y_el * fmtl->bh;
      *z_offset_sa += tail_offset_z_el * fmtl->bd;
   }
}

/**
 * A variant of isl_surf_get_image_offset_sa() specific to
 * ISL_DIM_LAYOUT_GFX4_3D.
 */
static void
get_image_offset_sa_gfx4_3d(const struct isl_surf *surf,
                            uint32_t level, uint32_t logical_z_offset_px,
                            uint32_t *x_offset_sa,
                            uint32_t *y_offset_sa)
{
   assert(level < surf->levels);
   if (surf->dim == ISL_SURF_DIM_3D) {
      assert(surf->phys_level0_sa.array_len == 1);
      assert(logical_z_offset_px < isl_minify(surf->phys_level0_sa.depth, level));
   } else {
      assert(surf->dim == ISL_SURF_DIM_2D);
      assert(surf->usage & ISL_SURF_USAGE_CUBE_BIT);
      assert(surf->phys_level0_sa.array_len == 6);
      assert(logical_z_offset_px < surf->phys_level0_sa.array_len);
   }

   const struct isl_extent3d image_align_sa =
      isl_surf_get_image_alignment_sa(surf);

   const uint32_t W0 = surf->phys_level0_sa.width;
   const uint32_t H0 = surf->phys_level0_sa.height;
   const uint32_t D0 = surf->phys_level0_sa.depth;
   const uint32_t AL = surf->phys_level0_sa.array_len;

   uint32_t x = 0;
   uint32_t y = 0;

   for (uint32_t l = 0; l < level; ++l) {
      const uint32_t level_h = isl_align_npot(isl_minify(H0, l), image_align_sa.h);
      const uint32_t level_d =
         isl_align_npot(surf->dim == ISL_SURF_DIM_3D ? isl_minify(D0, l) : AL,
                        image_align_sa.d);
      const uint32_t max_layers_vert = isl_align(level_d, 1u << l) / (1u << l);

      y += level_h * max_layers_vert;
   }

   const uint32_t level_w = isl_align_npot(isl_minify(W0, level), image_align_sa.w);
   const uint32_t level_h = isl_align_npot(isl_minify(H0, level), image_align_sa.h);
   const uint32_t level_d =
      isl_align_npot(surf->dim == ISL_SURF_DIM_3D ? isl_minify(D0, level) : AL,
                     image_align_sa.d);

   const uint32_t max_layers_horiz = MIN(level_d, 1u << level);

   x += level_w * (logical_z_offset_px % max_layers_horiz);
   y += level_h * (logical_z_offset_px / max_layers_horiz);

   *x_offset_sa = x;
   *y_offset_sa = y;
}

static void
get_image_offset_sa_gfx6_stencil_hiz(const struct isl_surf *surf,
                                     uint32_t level,
                                     uint32_t logical_array_layer,
                                     uint32_t *x_offset_sa,
                                     uint32_t *y_offset_sa)
{
   assert(level < surf->levels);
   assert(surf->logical_level0_px.depth == 1);
   assert(logical_array_layer < surf->logical_level0_px.array_len);

   const struct isl_format_layout *fmtl = isl_format_get_layout(surf->format);

   const struct isl_extent3d image_align_sa =
      isl_surf_get_image_alignment_sa(surf);

   struct isl_tile_info tile_info;
   isl_surf_get_tile_info(surf, &tile_info);
   const struct isl_extent2d tile_extent_sa = {
      .w = tile_info.logical_extent_el.w * fmtl->bw,
      .h = tile_info.logical_extent_el.h * fmtl->bh,
   };
   /* Tile size is a multiple of image alignment */
   assert(tile_extent_sa.w % image_align_sa.w == 0);
   assert(tile_extent_sa.h % image_align_sa.h == 0);

   const uint32_t W0 = surf->phys_level0_sa.w;
   const uint32_t H0 = surf->phys_level0_sa.h;

   /* Each image has the same height as LOD0 because the hardware thinks
    * everything is LOD0
    */
   const uint32_t H = isl_align(H0, image_align_sa.h);

   /* Quick sanity check for consistency */
   if (surf->phys_level0_sa.array_len > 1)
      assert(surf->array_pitch_el_rows == isl_assert_div(H, fmtl->bh));

   uint32_t x = 0, y = 0;
   for (uint32_t l = 0; l < level; ++l) {
      const uint32_t W = isl_minify(W0, l);

      const uint32_t w = isl_align(W, tile_extent_sa.w);
      const uint32_t h = isl_align(H * surf->phys_level0_sa.a,
                                   tile_extent_sa.h);

      if (l == 0) {
         y += h;
      } else {
         x += w;
      }
   }

   y += H * logical_array_layer;

   *x_offset_sa = x;
   *y_offset_sa = y;
}

/**
 * A variant of isl_surf_get_image_offset_sa() specific to
 * ISL_DIM_LAYOUT_GFX9_1D.
 */
static void
get_image_offset_sa_gfx9_1d(const struct isl_surf *surf,
                            uint32_t level, uint32_t layer,
                            uint32_t *x_offset_sa,
                            uint32_t *y_offset_sa)
{
   assert(level < surf->levels);
   assert(layer < surf->phys_level0_sa.array_len);
   assert(surf->phys_level0_sa.height == 1);
   assert(surf->phys_level0_sa.depth == 1);
   assert(surf->samples == 1);

   const uint32_t W0 = surf->phys_level0_sa.width;
   const struct isl_extent3d image_align_sa =
      isl_surf_get_image_alignment_sa(surf);

   uint32_t x = 0;

   for (uint32_t l = 0; l < level; ++l) {
      uint32_t W = isl_minify(W0, l);
      uint32_t w = isl_align_npot(W, image_align_sa.w);

      x += w;
   }

   *x_offset_sa = x;
   *y_offset_sa = layer * isl_surf_get_array_pitch_sa_rows(surf);
}

/**
 * Calculate the offset, in units of surface samples, to a subimage in the
 * surface.
 *
 * @invariant level < surface levels
 * @invariant logical_array_layer < logical array length of surface
 * @invariant logical_z_offset_px < logical depth of surface at level
 */
void
isl_surf_get_image_offset_sa(const struct isl_surf *surf,
                             uint32_t level,
                             uint32_t logical_array_layer,
                             uint32_t logical_z_offset_px,
                             uint32_t *x_offset_sa,
                             uint32_t *y_offset_sa,
                             uint32_t *z_offset_sa,
                             uint32_t *array_offset)
{
   assert(level < surf->levels);
   assert(logical_array_layer < surf->logical_level0_px.array_len);
   assert(logical_z_offset_px
          < isl_minify(surf->logical_level0_px.depth, level));

   switch (surf->dim_layout) {
   case ISL_DIM_LAYOUT_GFX9_1D:
      get_image_offset_sa_gfx9_1d(surf, level, logical_array_layer,
                                  x_offset_sa, y_offset_sa);
      *z_offset_sa = 0;
      *array_offset = 0;
      break;
   case ISL_DIM_LAYOUT_GFX4_2D:
      get_image_offset_sa_gfx4_2d(surf, level, logical_array_layer
                                  + logical_z_offset_px,
                                  x_offset_sa, y_offset_sa,
                                  z_offset_sa, array_offset);
      break;
   case ISL_DIM_LAYOUT_GFX4_3D:
      get_image_offset_sa_gfx4_3d(surf, level, logical_array_layer +
                                  logical_z_offset_px,
                                  x_offset_sa, y_offset_sa);
      *z_offset_sa = 0;
      *array_offset = 0;
      break;
   case ISL_DIM_LAYOUT_GFX6_STENCIL_HIZ:
      get_image_offset_sa_gfx6_stencil_hiz(surf, level, logical_array_layer +
                                           logical_z_offset_px,
                                           x_offset_sa, y_offset_sa);
      *z_offset_sa = 0;
      *array_offset = 0;
      break;

   default:
      unreachable("not reached");
   }
}

void
isl_surf_get_image_offset_el(const struct isl_surf *surf,
                             uint32_t level,
                             uint32_t logical_array_layer,
                             uint32_t logical_z_offset_px,
                             uint32_t *x_offset_el,
                             uint32_t *y_offset_el,
                             uint32_t *z_offset_el,
                             uint32_t *array_offset)
{
   const struct isl_format_layout *fmtl = isl_format_get_layout(surf->format);

   assert(level < surf->levels);
   assert(logical_array_layer < surf->logical_level0_px.array_len);
   assert(logical_z_offset_px
          < isl_minify(surf->logical_level0_px.depth, level));

   uint32_t x_offset_sa, y_offset_sa, z_offset_sa;
   isl_surf_get_image_offset_sa(surf, level,
                                logical_array_layer,
                                logical_z_offset_px,
                                &x_offset_sa,
                                &y_offset_sa,
                                &z_offset_sa,
                                array_offset);

   *x_offset_el = x_offset_sa / fmtl->bw;
   *y_offset_el = y_offset_sa / fmtl->bh;
   *z_offset_el = z_offset_sa / fmtl->bd;
}

void
isl_surf_get_image_offset_B_tile_sa(const struct isl_surf *surf,
                                    uint32_t level,
                                    uint32_t logical_array_layer,
                                    uint32_t logical_z_offset_px,
                                    uint64_t *offset_B,
                                    uint32_t *x_offset_sa,
                                    uint32_t *y_offset_sa)
{
   const struct isl_format_layout *fmtl = isl_format_get_layout(surf->format);

   uint32_t x_offset_el, y_offset_el;
   isl_surf_get_image_offset_B_tile_el(surf, level,
                                       logical_array_layer,
                                       logical_z_offset_px,
                                       offset_B,
                                       &x_offset_el,
                                       &y_offset_el);

   if (x_offset_sa) {
      *x_offset_sa = x_offset_el * fmtl->bw;
   } else {
      assert(x_offset_el == 0);
   }

   if (y_offset_sa) {
      *y_offset_sa = y_offset_el * fmtl->bh;
   } else {
      assert(y_offset_el == 0);
   }
}

void
isl_surf_get_image_offset_B_tile_el(const struct isl_surf *surf,
                                    uint32_t level,
                                    uint32_t logical_array_layer,
                                    uint32_t logical_z_offset_px,
                                    uint64_t *offset_B,
                                    uint32_t *x_offset_el,
                                    uint32_t *y_offset_el)
{
   const struct isl_format_layout *fmtl = isl_format_get_layout(surf->format);

   uint32_t total_x_offset_el, total_y_offset_el;
   uint32_t total_z_offset_el, total_array_offset;
   isl_surf_get_image_offset_el(surf, level, logical_array_layer,
                                logical_z_offset_px,
                                &total_x_offset_el,
                                &total_y_offset_el,
                                &total_z_offset_el,
                                &total_array_offset);

   uint32_t z_offset_el, array_offset;
   isl_tiling_get_intratile_offset_el(surf->tiling, surf->dim,
                                      surf->msaa_layout, fmtl->bpb,
                                      surf->samples,
                                      surf->row_pitch_B,
                                      surf->array_pitch_el_rows,
                                      total_x_offset_el,
                                      total_y_offset_el,
                                      total_z_offset_el,
                                      total_array_offset,
                                      offset_B,
                                      x_offset_el,
                                      y_offset_el,
                                      &z_offset_el,
                                      &array_offset);
   if (level >= surf->miptail_start_level) {
      /* We can do a byte offset to the first level of a miptail but we cannot
       * offset into a miptail.
       */
      assert(level == surf->miptail_start_level);

      /* The byte offset will get us to the miptail page.  The other offsets
       * are to the actual level within the miptail.  It is assumed that the
       * caller will set up a texture with a miptail and use the hardware to
       * handle offseting inside the miptail.
       */
      *x_offset_el = 0;
      *y_offset_el = 0;
   } else {
      assert(z_offset_el == 0);
      assert(array_offset == 0);
   }
}

void
isl_surf_get_image_range_B_tile(const struct isl_surf *surf,
                                uint32_t level,
                                uint32_t logical_array_layer,
                                uint32_t logical_z_offset_px,
                                uint64_t *start_tile_B,
                                uint64_t *end_tile_B)
{
   uint32_t start_x_offset_el, start_y_offset_el;
   uint32_t start_z_offset_el, start_array_slice;
   isl_surf_get_image_offset_el(surf, level, logical_array_layer,
                                logical_z_offset_px,
                                &start_x_offset_el,
                                &start_y_offset_el,
                                &start_z_offset_el,
                                &start_array_slice);

   /* Compute the size of the subimage in surface elements */
   const uint32_t subimage_w_sa = isl_minify(surf->phys_level0_sa.w, level);
   const uint32_t subimage_h_sa = isl_minify(surf->phys_level0_sa.h, level);
   const struct isl_format_layout *fmtl = isl_format_get_layout(surf->format);
   const uint32_t subimage_w_el = isl_align_div_npot(subimage_w_sa, fmtl->bw);
   const uint32_t subimage_h_el = isl_align_div_npot(subimage_h_sa, fmtl->bh);

   /* Find the last pixel */
   uint32_t end_x_offset_el = start_x_offset_el + subimage_w_el - 1;
   uint32_t end_y_offset_el = start_y_offset_el + subimage_h_el - 1;

   /* We only consider one Z or array slice */
   const uint32_t end_z_offset_el = start_z_offset_el;
   const uint32_t end_array_slice = start_array_slice;

   UNUSED uint32_t x_offset_el, y_offset_el, z_offset_el, array_slice;
   isl_tiling_get_intratile_offset_el(surf->tiling, surf->dim,
                                      surf->msaa_layout, fmtl->bpb,
                                      surf->samples,
                                      surf->row_pitch_B,
                                      surf->array_pitch_el_rows,
                                      start_x_offset_el,
                                      start_y_offset_el,
                                      start_z_offset_el,
                                      start_array_slice,
                                      start_tile_B,
                                      &x_offset_el,
                                      &y_offset_el,
                                      &z_offset_el,
                                      &array_slice);

   isl_tiling_get_intratile_offset_el(surf->tiling, surf->dim,
                                      surf->msaa_layout, fmtl->bpb,
                                      surf->samples,
                                      surf->row_pitch_B,
                                      surf->array_pitch_el_rows,
                                      end_x_offset_el,
                                      end_y_offset_el,
                                      end_z_offset_el,
                                      end_array_slice,
                                      end_tile_B,
                                      &x_offset_el,
                                      &y_offset_el,
                                      &z_offset_el,
                                      &array_slice);

   /* We want the range we return to be exclusive but the tile containing the
    * last pixel (what we just calculated) is inclusive.  Add one.
    */
   (*end_tile_B)++;

   assert(*end_tile_B <= surf->size_B);
}

void
isl_surf_get_image_surf(const struct isl_device *dev,
                        const struct isl_surf *surf,
                        uint32_t level,
                        uint32_t logical_array_layer,
                        uint32_t logical_z_offset_px,
                        struct isl_surf *image_surf,
                        uint64_t *offset_B,
                        uint32_t *x_offset_sa,
                        uint32_t *y_offset_sa)
{
   isl_surf_get_image_offset_B_tile_sa(surf,
                                       level,
                                       logical_array_layer,
                                       logical_z_offset_px,
                                       offset_B,
                                       x_offset_sa,
                                       y_offset_sa);

   /* Even for cube maps there will be only single face, therefore drop the
    * corresponding flag if present.
    */
   const isl_surf_usage_flags_t usage =
      surf->usage & (~ISL_SURF_USAGE_CUBE_BIT);

   bool ok UNUSED;
   ok = isl_surf_init(dev, image_surf,
                      .dim = ISL_SURF_DIM_2D,
                      .format = surf->format,
                      .width = isl_minify(surf->logical_level0_px.w, level),
                      .height = isl_minify(surf->logical_level0_px.h, level),
                      .depth = 1,
                      .levels = 1,
                      .array_len = 1,
                      .samples = surf->samples,
                      .row_pitch_B = surf->row_pitch_B,
                      .usage = usage,
                      .tiling_flags = (1 << surf->tiling));
   assert(ok);
}

bool
isl_surf_get_uncompressed_surf(const struct isl_device *dev,
                               const struct isl_surf *_surf,
                               const struct isl_view *_view,
                               struct isl_surf *ucompr_surf,
                               struct isl_view *ucompr_view,
                               uint64_t *offset_B,
                               uint32_t *x_offset_el,
                               uint32_t *y_offset_el)
{
   /* Input and output pointers may be the same, save the input contents now. */
   const struct isl_surf __surf = *_surf, *surf = &__surf;
   const struct isl_view __view = *_view, *view = &__view;
   const struct isl_format_layout *fmtl =
      isl_format_get_layout(surf->format);
   const enum isl_format view_format = view->format;

   assert(fmtl->bw > 1 || fmtl->bh > 1 || fmtl->bd > 1);
   assert(isl_format_is_compressed(surf->format));
   assert(!isl_format_is_compressed(view->format));
   assert(isl_format_get_layout(view->format)->bpb == fmtl->bpb);
   assert(view->levels == 1);

   const uint32_t view_width_px =
      isl_minify(surf->logical_level0_px.width, view->base_level);
   const uint32_t view_height_px =
      isl_minify(surf->logical_level0_px.height, view->base_level);

   assert(surf->samples == 1);
   const uint32_t view_width_el = isl_align_div_npot(view_width_px, fmtl->bw);
   const uint32_t view_height_el = isl_align_div_npot(view_height_px, fmtl->bh);

   /* If we ever enable 3D block formats, we'll need to re-think this */
   assert(fmtl->bd == 1);

   if (isl_tiling_is_std_y(surf->tiling) || surf->tiling == ISL_TILING_64) {
      /* If the requested level is not part of the miptail, we just offset to
       * the requested level. Because we're using standard tilings and aren't
       * in the miptail, arrays and 3D textures should just work so long as we
       * have the right array stride in the end.
       *
       * If the requested level is in the miptail, we instead offset to the
       * base of the miptail.  Because offsets into the miptail are fixed by
       * the tiling and don't depend on the actual size of the image, we can
       * set the level in the view to offset into the miptail regardless of
       * the fact minification yields different results for the compressed and
       * uncompressed surface.
       */
      const uint32_t base_level =
         MIN(view->base_level, surf->miptail_start_level);

      isl_surf_get_image_offset_B_tile_el(surf, base_level, 0, 0,
                                          offset_B, x_offset_el, y_offset_el);
      /* Tile64, Ys and Yf should have no intratile X or Y offset */
      assert(*x_offset_el == 0 && *y_offset_el == 0);

      /* Save off the array pitch */
      const uint32_t array_pitch_el_rows = surf->array_pitch_el_rows;

      const uint32_t view_depth_px =
         isl_minify(surf->logical_level0_px.depth, view->base_level);
      const uint32_t view_depth_el =
         isl_align_div_npot(view_depth_px, fmtl->bd);

      /* We need to compute the size of the uncompressed surface we will
       * create. If we're not in the miptail, it is just the view size in
       * surface elements. If we are in a miptail, we need a size that will
       * minify to the view size in surface elements. This may not be the same
       * as the size of base_level, but that's not a problem. Slot offsets are
       * fixed in HW (see the tables used in isl_get_miptail_level_offset_el).
       */
      const uint32_t ucompr_level = view->base_level - base_level;

      /* The > 1 check is here to prevent a change in the surface's overall
       * dimension (e.g. 2D->3D).
       *
       * Also having a base_level dimension = 1 doesn´t mean the HW will
       * ignore higher mip level. Once the dimension has reached 1, it'll stay
       * at 1 in the higher mip levels.
       */
      struct isl_extent3d ucompr_surf_extent_el = {
         .w = view_width_el  > 1 ? view_width_el  << ucompr_level : 1,
         .h = view_height_el > 1 ? view_height_el << ucompr_level : 1,
         .d = view_depth_el  > 1 ? view_depth_el  << ucompr_level : 1,
      };

      bool ok UNUSED;
      ok = isl_surf_init(dev, ucompr_surf,
                         .dim = surf->dim,
                         .format = view->format,
                         .width = ucompr_surf_extent_el.width,
                         .height = ucompr_surf_extent_el.height,
                         .depth = ucompr_surf_extent_el.depth,
                         .levels = ucompr_level + 1,
                         .array_len = surf->logical_level0_px.array_len,
                         .samples = surf->samples,
                         .min_miptail_start_level =
                            (int) (view->base_level < surf->miptail_start_level),
                         .row_pitch_B = surf->row_pitch_B,
                         .usage = surf->usage,
                         .tiling_flags = (1u << surf->tiling));
      assert(ok);

      /* Use the array pitch from the original surface.  This way 2D arrays
       * and 3D textures should work properly, just with one LOD.
       */
      assert(ucompr_surf->array_pitch_el_rows <= array_pitch_el_rows);
      ucompr_surf->array_pitch_el_rows = array_pitch_el_rows;

      /* The newly created image represents only the one miplevel so we
       * need to adjust the view accordingly.  Because we offset it to
       * miplevel but used a Z and array slice of 0, the array range can be
       * left alone.
       */
      *ucompr_view = *view;
      ucompr_view->base_level -= base_level;
   } else {
      if (view->array_len > 1) {
         /* The Skylake PRM Vol. 2d, "RENDER_SURFACE_STATE::X Offset" says:
          *
          *    "If Surface Array is enabled, this field must be zero."
          *
          * The PRMs for other hardware have similar text. This is also tricky
          * to handle with things like BLORP's SW offsetting because the
          * increased surface size required for the offset may result in an
          * image height greater than qpitch.
          */
         if (view->base_level > 0)
            return false;

         /* On Haswell and earlier, RENDER_SURFACE_STATE doesn't have a QPitch
          * field; it only has "array pitch span" which means the QPitch is
          * automatically calculated. Since we're smashing the surface format
          * (block formats are subtly different) and the number of miplevels,
          * that calculation will get thrown off. This means we can't do
          * arrays even at LOD0
          *
          * On Broadwell, we do have a QPitch field which we can control.
          * However, HALIGN and VALIGN are specified in pixels and are
          * hard-coded to align to exactly the block size of the compressed
          * texture. This means that, when reinterpreted as a non-compressed
          * the QPitch may be anything but the HW requires it to be properly
          * aligned.
          */
         if (ISL_GFX_VER(dev) < 9)
            return false;

         *ucompr_surf = *surf;
         ucompr_surf->levels = 1;
         ucompr_surf->format = view_format;

         /* We're making an uncompressed view here. The image dimensions need
          * to be scaled down by the block size.
          */
         assert(ucompr_surf->logical_level0_px.width == view_width_px);
         assert(ucompr_surf->logical_level0_px.height == view_height_px);
         ucompr_surf->logical_level0_px.width = view_width_el;
         ucompr_surf->logical_level0_px.height = view_height_el;
         ucompr_surf->phys_level0_sa = isl_surf_get_phys_level0_el(surf);

         /* The surface mostly stays as-is; there is no offset */
         *offset_B = 0;
         *x_offset_el = 0;
         *y_offset_el = 0;

         /* The view remains the same */
         *ucompr_view = *view;
      } else {
         /* If only one array slice is requested, directly offset to that
          * slice. We could, in theory, still use arrays in some cases but
          * BLORP isn't prepared for this and everyone who calls this function
          * should be prepared to handle an X/Y offset.
          */
         isl_surf_get_image_offset_B_tile_el(surf,
                                             view->base_level,
                                             surf->dim == ISL_SURF_DIM_3D ?
                                             0 : view->base_array_layer,
                                             surf->dim == ISL_SURF_DIM_3D ?
                                             view->base_array_layer : 0,
                                             offset_B,
                                             x_offset_el,
                                             y_offset_el);

         /* Even for cube maps there will be only single face, therefore drop
          * the corresponding flag if present.
          */
         const isl_surf_usage_flags_t usage =
            surf->usage & (~ISL_SURF_USAGE_CUBE_BIT);

         bool ok UNUSED;
         ok = isl_surf_init(dev, ucompr_surf,
                            .dim = ISL_SURF_DIM_2D,
                            .format = view_format,
                            .width = view_width_el,
                            .height = view_height_el,
                            .depth = 1,
                            .levels = 1,
                            .array_len = 1,
                            .samples = 1,
                            .row_pitch_B = surf->row_pitch_B,
                            .usage = usage,
                            .tiling_flags = (1 << surf->tiling));
         assert(ok);

         /* The newly created image represents the one subimage we're
          * referencing with this view so it only has one array slice and
          * miplevel.
          */
         *ucompr_view = *view;
         ucompr_view->base_array_layer = 0;
         ucompr_view->base_level = 0;
      }
   }

   return true;
}

void
isl_tiling_get_intratile_offset_el(enum isl_tiling tiling,
                                   enum isl_surf_dim dim,
                                   enum isl_msaa_layout msaa_layout,
                                   uint32_t bpb,
                                   uint32_t samples,
                                   uint32_t row_pitch_B,
                                   uint32_t array_pitch_el_rows,
                                   uint32_t total_x_offset_el,
                                   uint32_t total_y_offset_el,
                                   uint32_t total_z_offset_el,
                                   uint32_t total_array_offset,
                                   uint64_t *tile_offset_B,
                                   uint32_t *x_offset_el,
                                   uint32_t *y_offset_el,
                                   uint32_t *z_offset_el,
                                   uint32_t *array_offset)
{
   if (tiling == ISL_TILING_LINEAR) {
      assert(bpb % 8 == 0);
      assert(samples == 1);
      assert(total_z_offset_el == 0 && total_array_offset == 0);
      *tile_offset_B = (uint64_t)total_y_offset_el * row_pitch_B +
                       (uint64_t)total_x_offset_el * (bpb / 8);
      *x_offset_el = 0;
      *y_offset_el = 0;
      *z_offset_el = 0;
      *array_offset = 0;
      return;
   }

   struct isl_tile_info tile_info;
   isl_tiling_get_info(tiling, dim, msaa_layout, bpb, samples, &tile_info);

   /* Pitches must make sense with the tiling */
   assert(row_pitch_B % tile_info.phys_extent_B.width == 0);
   if (tile_info.logical_extent_el.d > 1 || tile_info.logical_extent_el.a > 1)
      assert(array_pitch_el_rows % tile_info.logical_extent_el.h == 0);

   /* For non-power-of-two formats, we need the address to be both tile and
    * element-aligned.  The easiest way to achieve this is to work with a tile
    * that is three times as wide as the regular tile.
    *
    * The tile info returned by get_tile_info has a logical size that is an
    * integer number of tile_info.format_bpb size elements.  To scale the
    * tile, we scale up the physical width and then treat the logical tile
    * size as if it has bpb size elements.
    */
   const uint32_t tile_el_scale = bpb / tile_info.format_bpb;
   tile_info.phys_extent_B.width *= tile_el_scale;

   /* Compute the offset into the tile */
   *x_offset_el = total_x_offset_el % tile_info.logical_extent_el.w;
   *y_offset_el = total_y_offset_el % tile_info.logical_extent_el.h;
   *z_offset_el = total_z_offset_el % tile_info.logical_extent_el.d;
   *array_offset = total_array_offset % tile_info.logical_extent_el.a;

   /* Compute the offset of the tile in units of whole tiles */
   uint32_t x_offset_tl = total_x_offset_el / tile_info.logical_extent_el.w;
   uint32_t y_offset_tl = total_y_offset_el / tile_info.logical_extent_el.h;
   uint32_t z_offset_tl = total_z_offset_el / tile_info.logical_extent_el.d;
   uint32_t a_offset_tl = total_array_offset / tile_info.logical_extent_el.a;

   /* Compute an array pitch in number of tiles */
   uint32_t array_pitch_tl_rows =
      array_pitch_el_rows / tile_info.logical_extent_el.h;

   /* Add the Z and array offset to the Y offset to get a 2D offset */
   y_offset_tl += (z_offset_tl + a_offset_tl) * array_pitch_tl_rows;

   *tile_offset_B =
      (uint64_t)y_offset_tl * tile_info.phys_extent_B.h * row_pitch_B +
      (uint64_t)x_offset_tl * tile_info.phys_extent_B.h * tile_info.phys_extent_B.w;
}

uint32_t
isl_surf_get_depth_format(const struct isl_device *dev,
                          const struct isl_surf *surf)
{
   /* Support for separate stencil buffers began in gfx5. Support for
    * interleaved depthstencil buffers ceased in gfx7. The intermediate gens,
    * those that supported separate and interleaved stencil, were gfx5 and
    * gfx6.
    *
    * For a list of all available formats, see the Sandybridge PRM >> Volume
    * 2 Part 1: 3D/Media - 3D Pipeline >> 3DSTATE_DEPTH_BUFFER >> Surface
    * Format (p321).
    */

   bool has_stencil = surf->usage & ISL_SURF_USAGE_STENCIL_BIT;

   assert(surf->usage & ISL_SURF_USAGE_DEPTH_BIT);

   if (has_stencil)
      assert(ISL_GFX_VER(dev) < 7);

   switch (surf->format) {
   default:
      unreachable("bad isl depth format");
   case ISL_FORMAT_R32_FLOAT_X8X24_TYPELESS:
      assert(ISL_GFX_VER(dev) < 7);
      return 0; /* D32_FLOAT_S8X24_UINT */
   case ISL_FORMAT_R32_FLOAT:
      assert(!has_stencil);
      return 1; /* D32_FLOAT */
   case ISL_FORMAT_R24_UNORM_X8_TYPELESS:
      if (has_stencil) {
         assert(ISL_GFX_VER(dev) < 7);
         return 2; /* D24_UNORM_S8_UINT */
      } else {
         assert(ISL_GFX_VER(dev) >= 5);
         return 3; /* D24_UNORM_X8_UINT */
      }
   case ISL_FORMAT_R16_UNORM:
      assert(!has_stencil);
      return 5; /* D16_UNORM */
   }
}

bool
isl_swizzle_supports_rendering(const struct intel_device_info *devinfo,
                               struct isl_swizzle swizzle)
{
   if (devinfo->platform == INTEL_PLATFORM_HSW) {
      /* From the Haswell PRM,
       * RENDER_SURFACE_STATE::Shader Channel Select Red
       *
       *    "The Shader channel selects also define which shader channels are
       *    written to which surface channel. If the Shader channel select is
       *    SCS_ZERO or SCS_ONE then it is not written to the surface. If the
       *    shader channel select is SCS_RED it is written to the surface red
       *    channel and so on. If more than one shader channel select is set
       *    to the same surface channel only the first shader channel in RGBA
       *    order will be written."
       */
      return true;
   } else if (devinfo->ver <= 7) {
      /* Ivy Bridge and early doesn't have any swizzling */
      return isl_swizzle_is_identity(swizzle);
   } else {
      /* From the Sky Lake PRM Vol. 2d,
       * RENDER_SURFACE_STATE::Shader Channel Select Red
       *
       *    "For Render Target, Red, Green and Blue Shader Channel Selects
       *    MUST be such that only valid components can be swapped i.e. only
       *    change the order of components in the pixel. Any other values for
       *    these Shader Channel Select fields are not valid for Render
       *    Targets. This also means that there MUST not be multiple shader
       *    channels mapped to the same RT channel."
       *
       * From the Sky Lake PRM Vol. 2d,
       * RENDER_SURFACE_STATE::Shader Channel Select Alpha
       *
       *    "For Render Target, this field MUST be programmed to
       *    value = SCS_ALPHA."
       */
      return (swizzle.r == ISL_CHANNEL_SELECT_RED ||
              swizzle.r == ISL_CHANNEL_SELECT_GREEN ||
              swizzle.r == ISL_CHANNEL_SELECT_BLUE) &&
             (swizzle.g == ISL_CHANNEL_SELECT_RED ||
              swizzle.g == ISL_CHANNEL_SELECT_GREEN ||
              swizzle.g == ISL_CHANNEL_SELECT_BLUE) &&
             (swizzle.b == ISL_CHANNEL_SELECT_RED ||
              swizzle.b == ISL_CHANNEL_SELECT_GREEN ||
              swizzle.b == ISL_CHANNEL_SELECT_BLUE) &&
             swizzle.r != swizzle.g &&
             swizzle.r != swizzle.b &&
             swizzle.g != swizzle.b &&
             swizzle.a == ISL_CHANNEL_SELECT_ALPHA;
   }
}

static enum isl_channel_select
swizzle_select(enum isl_channel_select chan, struct isl_swizzle swizzle)
{
   switch (chan) {
   case ISL_CHANNEL_SELECT_ZERO:
   case ISL_CHANNEL_SELECT_ONE:
      return chan;
   case ISL_CHANNEL_SELECT_RED:
      return swizzle.r;
   case ISL_CHANNEL_SELECT_GREEN:
      return swizzle.g;
   case ISL_CHANNEL_SELECT_BLUE:
      return swizzle.b;
   case ISL_CHANNEL_SELECT_ALPHA:
      return swizzle.a;
   default:
      unreachable("Invalid swizzle component");
   }
}

/**
 * Returns the single swizzle that is equivalent to applying the two given
 * swizzles in sequence.
 */
struct isl_swizzle
isl_swizzle_compose(struct isl_swizzle first, struct isl_swizzle second)
{
   return (struct isl_swizzle) {
      .r = swizzle_select(first.r, second),
      .g = swizzle_select(first.g, second),
      .b = swizzle_select(first.b, second),
      .a = swizzle_select(first.a, second),
   };
}

/**
 * Returns a swizzle that is the pseudo-inverse of this swizzle.
 */
struct isl_swizzle
isl_swizzle_invert(struct isl_swizzle swizzle)
{
   /* Default to zero for channels which do not show up in the swizzle */
   enum isl_channel_select chans[4] = {
      ISL_CHANNEL_SELECT_ZERO,
      ISL_CHANNEL_SELECT_ZERO,
      ISL_CHANNEL_SELECT_ZERO,
      ISL_CHANNEL_SELECT_ZERO,
   };

   /* We go in ABGR order so that, if there are any duplicates, the first one
    * is taken if you look at it in RGBA order.  This is what Haswell hardware
    * does for render target swizzles.
    */
   if ((unsigned)(swizzle.a - ISL_CHANNEL_SELECT_RED) < 4)
      chans[swizzle.a - ISL_CHANNEL_SELECT_RED] = ISL_CHANNEL_SELECT_ALPHA;
   if ((unsigned)(swizzle.b - ISL_CHANNEL_SELECT_RED) < 4)
      chans[swizzle.b - ISL_CHANNEL_SELECT_RED] = ISL_CHANNEL_SELECT_BLUE;
   if ((unsigned)(swizzle.g - ISL_CHANNEL_SELECT_RED) < 4)
      chans[swizzle.g - ISL_CHANNEL_SELECT_RED] = ISL_CHANNEL_SELECT_GREEN;
   if ((unsigned)(swizzle.r - ISL_CHANNEL_SELECT_RED) < 4)
      chans[swizzle.r - ISL_CHANNEL_SELECT_RED] = ISL_CHANNEL_SELECT_RED;

   return (struct isl_swizzle) { chans[0], chans[1], chans[2], chans[3] };
}

static uint32_t
isl_color_value_channel(union isl_color_value src,
                        enum isl_channel_select chan,
                        uint32_t one)
{
   if (chan == ISL_CHANNEL_SELECT_ZERO)
      return 0;
   if (chan == ISL_CHANNEL_SELECT_ONE)
      return one;

   assert(chan >= ISL_CHANNEL_SELECT_RED);
   assert(chan < ISL_CHANNEL_SELECT_RED + 4);

   return src.u32[chan - ISL_CHANNEL_SELECT_RED];
}

/** Applies an inverse swizzle to a color value */
union isl_color_value
isl_color_value_swizzle(union isl_color_value src,
                        struct isl_swizzle swizzle,
                        bool is_float)
{
   uint32_t one = is_float ? 0x3f800000 : 1;

   return (union isl_color_value) { .u32 = {
      isl_color_value_channel(src, swizzle.r, one),
      isl_color_value_channel(src, swizzle.g, one),
      isl_color_value_channel(src, swizzle.b, one),
      isl_color_value_channel(src, swizzle.a, one),
   } };
}

/** Applies an inverse swizzle to a color value */
union isl_color_value
isl_color_value_swizzle_inv(union isl_color_value src,
                            struct isl_swizzle swizzle)
{
   union isl_color_value dst = { .u32 = { 0, } };

   /* We assign colors in ABGR order so that the first one will be taken in
    * RGBA precedence order.  According to the PRM docs for shader channel
    * select, this matches Haswell hardware behavior.
    */
   if ((unsigned)(swizzle.a - ISL_CHANNEL_SELECT_RED) < 4)
      dst.u32[swizzle.a - ISL_CHANNEL_SELECT_RED] = src.u32[3];
   if ((unsigned)(swizzle.b - ISL_CHANNEL_SELECT_RED) < 4)
      dst.u32[swizzle.b - ISL_CHANNEL_SELECT_RED] = src.u32[2];
   if ((unsigned)(swizzle.g - ISL_CHANNEL_SELECT_RED) < 4)
      dst.u32[swizzle.g - ISL_CHANNEL_SELECT_RED] = src.u32[1];
   if ((unsigned)(swizzle.r - ISL_CHANNEL_SELECT_RED) < 4)
      dst.u32[swizzle.r - ISL_CHANNEL_SELECT_RED] = src.u32[0];

   return dst;
}

uint8_t
isl_format_get_aux_map_encoding(enum isl_format format)
{
   switch(format) {
   case ISL_FORMAT_R32G32B32A32_FLOAT: return 0x11;
   case ISL_FORMAT_R32G32B32X32_FLOAT: return 0x11;
   case ISL_FORMAT_R32G32B32A32_SINT: return 0x12;
   case ISL_FORMAT_R32G32B32A32_UINT: return 0x13;
   case ISL_FORMAT_R16G16B16A16_UNORM: return 0x14;
   case ISL_FORMAT_R16G16B16A16_SNORM: return 0x15;
   case ISL_FORMAT_R16G16B16A16_SINT: return 0x16;
   case ISL_FORMAT_R16G16B16A16_UINT: return 0x17;
   case ISL_FORMAT_R16G16B16A16_FLOAT: return 0x10;
   case ISL_FORMAT_R16G16B16X16_FLOAT: return 0x10;
   case ISL_FORMAT_R32G32_FLOAT: return 0x11;
   case ISL_FORMAT_R32G32_SINT: return 0x12;
   case ISL_FORMAT_R32G32_UINT: return 0x13;
   case ISL_FORMAT_B8G8R8A8_UNORM: return 0xA;
   case ISL_FORMAT_B8G8R8X8_UNORM: return 0xA;
   case ISL_FORMAT_B8G8R8A8_UNORM_SRGB: return 0xA;
   case ISL_FORMAT_B8G8R8X8_UNORM_SRGB: return 0xA;
   case ISL_FORMAT_R10G10B10A2_UNORM: return 0x18;
   case ISL_FORMAT_R10G10B10A2_UNORM_SRGB: return 0x18;
   case ISL_FORMAT_R10G10B10_FLOAT_A2_UNORM: return 0x19;
   case ISL_FORMAT_R10G10B10A2_UINT: return 0x1A;
   case ISL_FORMAT_R8G8B8A8_UNORM: return 0xA;
   case ISL_FORMAT_R8G8B8A8_UNORM_SRGB: return 0xA;
   case ISL_FORMAT_R8G8B8A8_SNORM: return 0x1B;
   case ISL_FORMAT_R8G8B8A8_SINT: return 0x1C;
   case ISL_FORMAT_R8G8B8A8_UINT: return 0x1D;
   case ISL_FORMAT_R16G16_UNORM: return 0x14;
   case ISL_FORMAT_R16G16_SNORM: return 0x15;
   case ISL_FORMAT_R16G16_SINT: return 0x16;
   case ISL_FORMAT_R16G16_UINT: return 0x17;
   case ISL_FORMAT_R16G16_FLOAT: return 0x10;
   case ISL_FORMAT_B10G10R10A2_UNORM: return 0x18;
   case ISL_FORMAT_B10G10R10A2_UNORM_SRGB: return 0x18;
   case ISL_FORMAT_R11G11B10_FLOAT: return 0x1E;
   case ISL_FORMAT_R32_SINT: return 0x12;
   case ISL_FORMAT_R32_UINT: return 0x13;
   case ISL_FORMAT_R32_FLOAT: return 0x11;
   case ISL_FORMAT_R24_UNORM_X8_TYPELESS: return 0x13;
   case ISL_FORMAT_B5G6R5_UNORM: return 0xA;
   case ISL_FORMAT_B5G6R5_UNORM_SRGB: return 0xA;
   case ISL_FORMAT_B5G5R5A1_UNORM: return 0xA;
   case ISL_FORMAT_B5G5R5A1_UNORM_SRGB: return 0xA;
   case ISL_FORMAT_B4G4R4A4_UNORM: return 0xA;
   case ISL_FORMAT_B4G4R4A4_UNORM_SRGB: return 0xA;
   case ISL_FORMAT_R8G8_UNORM: return 0xA;
   case ISL_FORMAT_R8G8_SNORM: return 0x1B;
   case ISL_FORMAT_R8G8_SINT: return 0x1C;
   case ISL_FORMAT_R8G8_UINT: return 0x1D;
   case ISL_FORMAT_R16_UNORM: return 0x14;
   case ISL_FORMAT_R16_SNORM: return 0x15;
   case ISL_FORMAT_R16_SINT: return 0x16;
   case ISL_FORMAT_R16_UINT: return 0x17;
   case ISL_FORMAT_R16_FLOAT: return 0x10;
   case ISL_FORMAT_B5G5R5X1_UNORM: return 0xA;
   case ISL_FORMAT_B5G5R5X1_UNORM_SRGB: return 0xA;
   case ISL_FORMAT_A1B5G5R5_UNORM: return 0xA;
   case ISL_FORMAT_A4B4G4R4_UNORM: return 0xA;
   case ISL_FORMAT_R8_UNORM: return 0xA;
   case ISL_FORMAT_R8_SNORM: return 0x1B;
   case ISL_FORMAT_R8_SINT: return 0x1C;
   case ISL_FORMAT_R8_UINT: return 0x1D;
   case ISL_FORMAT_A8_UNORM: return 0xA;
   case ISL_FORMAT_PLANAR_420_8: return 0xF;
   case ISL_FORMAT_PLANAR_420_10: return 0x7;
   case ISL_FORMAT_PLANAR_420_12: return 0x8;
   case ISL_FORMAT_PLANAR_420_16: return 0x8;
   case ISL_FORMAT_YCRCB_NORMAL: return 0x3;
   case ISL_FORMAT_YCRCB_SWAPY: return 0xB;
   default:
      unreachable("Unsupported aux-map format!");
      return 0;
   }
}

/*
 * Returns compression format encoding for Unified Lossless Compression
 */
uint8_t
isl_get_render_compression_format(enum isl_format format)
{
   /* From the Bspec, Enumeration_RenderCompressionFormat section (53726): */
   switch(format) {
   case ISL_FORMAT_R32G32B32A32_FLOAT:
   case ISL_FORMAT_R32G32B32X32_FLOAT:
   case ISL_FORMAT_R32G32B32A32_SINT:
      return 0x0;
   case ISL_FORMAT_R32G32B32A32_UINT:
      return 0x1;
   case ISL_FORMAT_R32G32_FLOAT:
   case ISL_FORMAT_R32G32_SINT:
      return 0x2;
   case ISL_FORMAT_R32G32_UINT:
      return 0x3;
   case ISL_FORMAT_R16G16B16A16_UNORM:
   case ISL_FORMAT_R16G16B16X16_UNORM:
   case ISL_FORMAT_R16G16B16A16_UINT:
      return 0x4;
   case ISL_FORMAT_R16G16B16A16_SNORM:
   case ISL_FORMAT_R16G16B16A16_SINT:
   case ISL_FORMAT_R16G16B16A16_FLOAT:
   case ISL_FORMAT_R16G16B16X16_FLOAT:
      return 0x5;
   case ISL_FORMAT_R16G16_UNORM:
   case ISL_FORMAT_R16G16_UINT:
      return 0x6;
   case ISL_FORMAT_R16G16_SNORM:
   case ISL_FORMAT_R16G16_SINT:
   case ISL_FORMAT_R16G16_FLOAT:
      return 0x7;
   case ISL_FORMAT_B8G8R8A8_UNORM:
   case ISL_FORMAT_B8G8R8X8_UNORM:
   case ISL_FORMAT_B8G8R8A8_UNORM_SRGB:
   case ISL_FORMAT_B8G8R8X8_UNORM_SRGB:
   case ISL_FORMAT_R8G8B8A8_UNORM:
   case ISL_FORMAT_R8G8B8X8_UNORM:
   case ISL_FORMAT_R8G8B8A8_UNORM_SRGB:
   case ISL_FORMAT_R8G8B8X8_UNORM_SRGB:
   case ISL_FORMAT_R8G8B8A8_UINT:
      return 0x8;
   case ISL_FORMAT_R8G8B8A8_SNORM:
   case ISL_FORMAT_R8G8B8A8_SINT:
      return 0x9;
   case ISL_FORMAT_B5G6R5_UNORM:
   case ISL_FORMAT_B5G6R5_UNORM_SRGB:
   case ISL_FORMAT_B5G5R5A1_UNORM:
   case ISL_FORMAT_B5G5R5A1_UNORM_SRGB:
   case ISL_FORMAT_B4G4R4A4_UNORM:
   case ISL_FORMAT_B4G4R4A4_UNORM_SRGB:
   case ISL_FORMAT_B5G5R5X1_UNORM:
   case ISL_FORMAT_B5G5R5X1_UNORM_SRGB:
   case ISL_FORMAT_A1B5G5R5_UNORM:
   case ISL_FORMAT_A4B4G4R4_UNORM:
   case ISL_FORMAT_R8G8_UNORM:
   case ISL_FORMAT_R8G8_UINT:
      return 0xA;
   case ISL_FORMAT_R8G8_SNORM:
   case ISL_FORMAT_R8G8_SINT:
      return 0xB;
   case ISL_FORMAT_R10G10B10A2_UNORM:
   case ISL_FORMAT_R10G10B10A2_UNORM_SRGB:
   case ISL_FORMAT_R10G10B10_FLOAT_A2_UNORM:
   case ISL_FORMAT_R10G10B10A2_UINT:
   case ISL_FORMAT_B10G10R10A2_UNORM:
   case ISL_FORMAT_B10G10R10X2_UNORM:
   case ISL_FORMAT_B10G10R10A2_UNORM_SRGB:
      return 0xC;
   case ISL_FORMAT_R11G11B10_FLOAT:
      return 0xD;
   case ISL_FORMAT_R32_SINT:
   case ISL_FORMAT_R32_FLOAT:
      return 0x10;
   case ISL_FORMAT_R32_UINT:
   case ISL_FORMAT_R24_UNORM_X8_TYPELESS:
      return 0x11;
   case ISL_FORMAT_R16_UNORM:
   case ISL_FORMAT_R16_UINT:
      return 0x14;
   case ISL_FORMAT_R16_SNORM:
   case ISL_FORMAT_R16_SINT:
   case ISL_FORMAT_R16_FLOAT:
      return 0x15;
   case ISL_FORMAT_R8_UNORM:
   case ISL_FORMAT_R8_UINT:
   case ISL_FORMAT_A8_UNORM:
      return 0x18;
   case ISL_FORMAT_R8_SNORM:
   case ISL_FORMAT_R8_SINT:
      return 0x19;
   default:
      unreachable("Unsupported render compression format!");
      return 0;
   }
}

const char *
isl_aux_op_to_name(enum isl_aux_op op)
{
   static const char *names[] = {
      [ISL_AUX_OP_NONE]            = "none",
      [ISL_AUX_OP_FAST_CLEAR]      = "fast-clear",
      [ISL_AUX_OP_FULL_RESOLVE]    = "full-resolve",
      [ISL_AUX_OP_PARTIAL_RESOLVE] = "partial-resolve",
      [ISL_AUX_OP_AMBIGUATE]       = "ambiguate",
   };
   assert(op < ARRAY_SIZE(names));
   return names[op];
}

const char *
isl_tiling_to_name(enum isl_tiling tiling)
{
   static const char *names[] = {
      [ISL_TILING_LINEAR]    = "linear",
      [ISL_TILING_W]         = "W",
      [ISL_TILING_X]         = "X",
      [ISL_TILING_Y0]        = "Y0",
      [ISL_TILING_SKL_Yf]    = "SKL-Yf",
      [ISL_TILING_SKL_Ys]    = "SKL-Ys",
      [ISL_TILING_ICL_Yf]    = "ICL-Yf",
      [ISL_TILING_ICL_Ys]    = "ICL-Ys",
      [ISL_TILING_4]         = "4",
      [ISL_TILING_64]        = "64",
      [ISL_TILING_HIZ]       = "hiz",
      [ISL_TILING_CCS]       = "ccs",
      [ISL_TILING_GFX12_CCS] = "gfx12-ccs",
   };
   assert(tiling < ARRAY_SIZE(names));
   return names[tiling];
}
