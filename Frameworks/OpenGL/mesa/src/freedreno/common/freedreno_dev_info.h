/*
 * Copyright © 2020 Valve Corporation
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
 *
 */

#ifndef FREEDRENO_DEVICE_INFO_H
#define FREEDRENO_DEVICE_INFO_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Freedreno hardware description and quirks
 */

struct fd_dev_info {
   uint8_t chip;

   /* alignment for size of tiles */
   uint32_t tile_align_w, tile_align_h;
   /* gmem load/store granularity */
   uint32_t gmem_align_w, gmem_align_h;
   /* max tile size */
   uint32_t tile_max_w, tile_max_h;

   uint32_t num_vsc_pipes;

   uint32_t cs_shared_mem_size;

   int wave_granularity;

   /* Information for private memory calculations */
   uint32_t fibers_per_sp;

   /* number of CCU is always equal to the number of SP */
   union {
      uint32_t num_sp_cores;
      uint32_t num_ccu;
   };

   struct {
      uint32_t reg_size_vec4;

      /* The size (in instrlen units (128 bytes)) of instruction cache where
       * we preload a shader. Loading more than this could trigger a hang
       * on gen3 and later.
       */
      uint32_t instr_cache_size;

      bool has_hw_multiview;

      bool has_fs_tex_prefetch;

      /* Whether the PC_MULTIVIEW_MASK register exists. */
      bool supports_multiview_mask;

      /* info for setting RB_CCU_CNTL */
      bool concurrent_resolve;
      bool has_z24uint_s8uint;

      bool tess_use_shared;

      /* Does the hw support GL_QCOM_shading_rate? */
      bool has_shading_rate;

      /* newer a6xx allows using 16-bit descriptor for both 16-bit
       * and 32-bit access
       */
      bool storage_16bit;

      /* The latest known a630_sqe.fw fails to wait for WFI before
       * reading the indirect buffer when using CP_DRAW_INDIRECT_MULTI,
       * so we have to fall back to CP_WAIT_FOR_ME except for a650
       * which has a fixed firmware.
       *
       * TODO: There may be newer a630_sqe.fw released in the future
       * which fixes this, if so we should detect it and avoid this
       * workaround.  Once we have uapi to query fw version, we can
       * replace this with minimum fw version.
       */
      bool indirect_draw_wfm_quirk;

      /* On some GPUs, the depth test needs to be enabled when the
       * depth bounds test is enabled and the depth attachment uses UBWC.
       */
      bool depth_bounds_require_depth_test_quirk;

      bool has_tex_filter_cubic;
      bool has_separate_chroma_filter;

      bool has_sample_locations;

      /* The firmware on newer a6xx drops CP_REG_WRITE support as we
       * can now use direct register writes for these regs.
       */
      bool has_cp_reg_write;

      bool has_8bpp_ubwc;

      bool has_lpac;

      bool has_getfiberid;

      bool has_dp2acc;
      bool has_dp4acc;

      /* LRZ fast-clear works on all gens, however blob disables it on
       * gen1 and gen2. We also elect to disable fast-clear on these gens
       * because for close to none gains it adds complexity and seem to work
       * a bit differently from gen3+. Which creates at least one edge case:
       * if first draw which uses LRZ fast-clear doesn't lock LRZ direction
       * the fast-clear value is undefined. For details see
       * https://gitlab.freedesktop.org/mesa/mesa/-/issues/6829
       */
      bool enable_lrz_fast_clear;
      bool has_lrz_dir_tracking;
      bool lrz_track_quirk;

      /* Some generations have a bit to add the multiview index to the
       * viewport index, which lets us implement different scaling for
       * different views.
       */
      bool has_per_view_viewport;
      bool has_gmem_fast_clear;

      /* Per CCU GMEM amount reserved for each of DEPTH and COLOR caches
       * in sysmem rendering. */
      uint32_t sysmem_per_ccu_cache_size;
      /* Per CCU GMEM amount reserved for color cache used by GMEM resolves
       * which require color cache (non-BLIT event case).
       * The size is expressed as a fraction of ccu cache used by sysmem
       * rendering. If a GMEM resolve requires color cache, the driver needs
       * to make sure it will not overwrite pixel data in GMEM that is still
       * needed.
       */
      /* see enum a6xx_ccu_color_cache_size */
      uint32_t gmem_ccu_color_cache_fraction;

      /* Corresponds to HLSQ_CONTROL_1_REG::PRIMALLOCTHRESHOLD */
      uint32_t prim_alloc_threshold;

      uint32_t vs_max_inputs_count;

      bool supports_double_threadsize;

      bool has_sampler_minmax;

      bool broken_ds_ubwc_quirk;

      struct {
         uint32_t PC_POWER_CNTL;
         uint32_t TPL1_DBG_ECO_CNTL;
         uint32_t GRAS_DBG_ECO_CNTL;
         uint32_t SP_CHICKEN_BITS;
         uint32_t UCHE_CLIENT_PF;
         uint32_t PC_MODE_CNTL;
         uint32_t SP_DBG_ECO_CNTL;
         uint32_t RB_DBG_ECO_CNTL;
         uint32_t RB_DBG_ECO_CNTL_blit;
         uint32_t HLSQ_DBG_ECO_CNTL;
         uint32_t RB_UNKNOWN_8E01;
         uint32_t VPC_DBG_ECO_CNTL;
         uint32_t UCHE_UNKNOWN_0E12;
      } magic;

      struct {
            uint32_t reg;
            uint32_t value;
      } magic_raw[32];

      /* maximum number of descriptor sets */
      uint32_t max_sets;
   } a6xx;

   struct {
      /* stsc may need to be done twice for the same range to workaround
       * _something_, observed in blob's disassembly.
       */
      bool stsc_duplication_quirk;

      /* Whether there is CP_EVENT_WRITE7::WRITE_SAMPLE_COUNT */
      bool has_event_write_sample_count;

      /* Blob executes a special compute dispatch at the start of each
       * command buffers. We copy this dispatch as is.
       */
      bool cmdbuf_start_a725_quirk;
   } a7xx;
};

struct fd_dev_id {
   uint32_t gpu_id;
   uint64_t chip_id;
};

/**
 * Note that gpu-id should be considered deprecated.  For newer a6xx, if
 * there is no gpu-id, this attempts to generate one from the chip-id.
 * But that may not work forever, so avoid depending on this for newer
 * gens
 */
static inline uint32_t
fd_dev_gpu_id(const struct fd_dev_id *id)
{
   assert(id->gpu_id || id->chip_id);
   if (!id->gpu_id) {
      return ((id->chip_id >> 24) & 0xff) * 100 +
             ((id->chip_id >> 16) & 0xff) * 10 +
             ((id->chip_id >>  8) & 0xff);

   }
   return id->gpu_id;
}

/* Unmodified dev info as defined in freedreno_devices.py */
const struct fd_dev_info *fd_dev_info_raw(const struct fd_dev_id *id);

/* Final dev info with dbg options and everything else applied.  */
const struct fd_dev_info fd_dev_info(const struct fd_dev_id *id);

static uint8_t
fd_dev_gen(const struct fd_dev_id *id)
{
   return fd_dev_info_raw(id)->chip;
}

static inline bool
fd_dev_64b(const struct fd_dev_id *id)
{
   return fd_dev_gen(id) >= 5;
}

/* per CCU GMEM amount reserved for depth cache for direct rendering */
#define A6XX_CCU_DEPTH_SIZE (64 * 1024)
/* per CCU GMEM amount reserved for color cache used by GMEM resolves
 * which require color cache (non-BLIT event case).
 * this is smaller than what is normally used by direct rendering
 * (RB_CCU_CNTL.GMEM bit enables this smaller size)
 * if a GMEM resolve requires color cache, the driver needs to make sure
 * it will not overwrite pixel data in GMEM that is still needed
 */
#define A6XX_CCU_GMEM_COLOR_SIZE (16 * 1024)

const char * fd_dev_name(const struct fd_dev_id *id);

void
fd_dev_info_apply_dbg_options(struct fd_dev_info *info);

#ifdef __cplusplus
} /* end of extern "C" */
#endif

#endif /* FREEDRENO_DEVICE_INFO_H */
