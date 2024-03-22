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

/* This file is based on rgxdefs.h and should only contain function-like macros
 * and inline functions. Any object-like macros should instead appear in
 * rogue_hw_defs.h.
 */

#ifndef ROGUE_HW_UTILS_H
#define ROGUE_HW_UTILS_H

#include <stdint.h>

#include "pvr_types.h"

#define __pvr_address_type pvr_dev_addr_t
#define __pvr_get_address(pvr_dev_addr) (pvr_dev_addr).addr
/* clang-format off */
#define __pvr_make_address(addr_u64) PVR_DEV_ADDR(addr_u64)
/* clang-format on */

#include "csbgen/rogue_cdm.h"
#include "csbgen/rogue_lls.h"

#undef __pvr_make_address
#undef __pvr_get_address
#undef __pvr_address_type

#include "rogue_hw_defs.h"
#include "pvr_device_info.h"
#include "util/compiler.h"
#include "util/macros.h"

static inline void
rogue_get_isp_samples_per_tile_xy(const struct pvr_device_info *dev_info,
                                  uint32_t samples,
                                  uint32_t *const x_out,
                                  uint32_t *const y_out)
{
   const uint32_t tile_size_x =
      PVR_GET_FEATURE_VALUE(dev_info, tile_size_x, 0U);
   const uint32_t tile_size_y =
      PVR_GET_FEATURE_VALUE(dev_info, tile_size_y, 0U);
   const uint32_t samples_per_pixel =
      PVR_GET_FEATURE_VALUE(dev_info, isp_samples_per_pixel, 0U);

#if !defined(NDEBUG)
   switch (samples_per_pixel) {
   case 1:
   case 2:
   case 4:
      break;
   default:
      assert(!"Unsupported ISP samples per pixel");
   }
#endif

   *x_out = tile_size_x;
   *y_out = tile_size_y;

   switch (samples) {
   case 1:
      break;
   case 2:
      if (samples_per_pixel == 2 || samples_per_pixel == 4)
         *y_out *= 2;

      break;
   case 4:
      if (samples_per_pixel == 2 || samples_per_pixel == 4)
         *x_out *= 2;

      if (samples_per_pixel == 2)
         *y_out *= 2;

      break;
   case 8:
      *y_out *= 2;
      break;
   default:
      assert(!"Unsupported number of samples");
   }
}

static void rogue_get_isp_scale_xy_from_samples(const uint32_t samples,
                                                uint32_t *const x_scale_out,
                                                uint32_t *const y_scale_out)
{
   switch (samples) {
   case 1:
      *x_scale_out = 1;
      *y_scale_out = 1;
      break;
   case 2:
      *x_scale_out = 1;
      *y_scale_out = 2;
      break;
   case 4:
      *x_scale_out = 2;
      *y_scale_out = 2;
      break;
   case 8:
      *x_scale_out = 2;
      *y_scale_out = 4;
      break;
   default:
      unreachable("Unsupported number of samples");
   }
}

static inline void
rogue_get_isp_num_tiles_xy(const struct pvr_device_info *dev_info,
                           uint32_t samples,
                           uint32_t width,
                           uint32_t height,
                           uint32_t *const x_out,
                           uint32_t *const y_out)
{
   uint32_t tile_samples_x;
   uint32_t tile_samples_y;
   uint32_t scale_x;
   uint32_t scale_y;

   rogue_get_isp_samples_per_tile_xy(dev_info,
                                     samples,
                                     &tile_samples_x,
                                     &tile_samples_y);

   rogue_get_isp_scale_xy_from_samples(samples, &scale_x, &scale_y);

   *x_out = DIV_ROUND_UP(width * scale_x, tile_samples_x);
   *y_out = DIV_ROUND_UP(height * scale_y, tile_samples_y);

   if (PVR_HAS_FEATURE(dev_info, simple_internal_parameter_format)) {
      assert(PVR_GET_FEATURE_VALUE(dev_info,
                                   simple_parameter_format_version,
                                   0U) == 2U);
      /* Align to a 2x2 tile block. */
      *x_out = ALIGN_POT(*x_out, 2);
      *y_out = ALIGN_POT(*y_out, 2);
   }
}

static inline void
rogue_get_zls_tile_size_xy(const struct pvr_device_info *dev_info,
                           uint32_t *const x_out,
                           uint32_t *const y_out)
{
   uint32_t version = 0;
   bool has_version;

   has_version =
      !PVR_FEATURE_VALUE(dev_info, simple_parameter_format_version, &version);

   *x_out = PVR_GET_FEATURE_VALUE(dev_info, tile_size_x, 0U);
   *y_out = PVR_GET_FEATURE_VALUE(dev_info, tile_size_y, 0U);

   if (PVR_HAS_FEATURE(dev_info, simple_internal_parameter_format) &&
       has_version && version == 2) {
      *x_out *= 2;
      *y_out *= 2;
   }
}

static inline uint32_t
rogue_get_max_output_regs_per_pixel(const struct pvr_device_info *dev_info)
{
   if (PVR_HAS_FEATURE(dev_info, eight_output_registers))
      return 8U;

   return 4U;
}

static inline void
rogue_get_num_macrotiles_xy(const struct pvr_device_info *dev_info,
                            uint32_t *const x_out,
                            uint32_t *const y_out)
{
   uint32_t version;

   if (PVR_FEATURE_VALUE(dev_info, simple_parameter_format_version, &version))
      version = 0;

   if (!PVR_HAS_FEATURE(dev_info, simple_internal_parameter_format) ||
       version == 2) {
      *x_out = 4;
      *y_out = 4;
   } else {
      *x_out = 1;
      *y_out = 1;
   }
}

static inline uint32_t
rogue_get_macrotile_array_size(const struct pvr_device_info *dev_info)
{
   uint32_t num_macrotiles_x;
   uint32_t num_macrotiles_y;

   if (PVR_HAS_FEATURE(dev_info, simple_internal_parameter_format))
      return 0;

   rogue_get_num_macrotiles_xy(dev_info, &num_macrotiles_x, &num_macrotiles_y);

   return num_macrotiles_x * num_macrotiles_y * 8U;
}

/* Region header size in bytes. */
static inline uint32_t
rogue_get_region_header_size(const struct pvr_device_info *dev_info)
{
   uint32_t version;

   if (PVR_FEATURE_VALUE(dev_info, simple_parameter_format_version, &version))
      version = 0;

   if (PVR_HAS_FEATURE(dev_info, simple_internal_parameter_format) &&
       version == 2) {
      return 6;
   }

   return 5;
}

static inline uint32_t
rogue_get_render_size_max(const struct pvr_device_info *dev_info)
{
   if (PVR_HAS_FEATURE(dev_info, simple_internal_parameter_format))
      if (!PVR_HAS_FEATURE(dev_info, screen_size8K))
         return 4096U;

   return 8192U;
}

#define rogue_get_render_size_max_x(dev_info) \
   rogue_get_render_size_max(dev_info)

#define rogue_get_render_size_max_y(dev_info) \
   rogue_get_render_size_max(dev_info)

static inline uint32_t
rogue_get_slc_cache_line_size(const struct pvr_device_info *dev_info)
{
   return PVR_GET_FEATURE_VALUE(dev_info, slc_cache_line_size_bits, 8U) / 8U;
}

static inline uint32_t pvr_get_max_user_vertex_output_components(
   const struct pvr_device_info *dev_info)
{
   const uint32_t uvs_pba_entries =
      PVR_GET_FEATURE_VALUE(dev_info, uvs_pba_entries, 0U);
   const uint32_t uvs_banks = PVR_GET_FEATURE_VALUE(dev_info, uvs_banks, 0U);

   if (uvs_banks <= 8U && uvs_pba_entries == 160U)
      return 64U;

   return 128U;
}

static inline uint32_t
rogue_max_compute_shared_registers(const struct pvr_device_info *dev_info)
{
   if (PVR_HAS_FEATURE(dev_info, compute))
      return 1024U;

   return 0U;
}

static inline uint32_t
rogue_get_max_num_cores(const struct pvr_device_info *dev_info)
{
   if (PVR_HAS_FEATURE(dev_info, gpu_multicore_support) &&
       PVR_HAS_FEATURE(dev_info, xpu_max_slaves)) {
      return PVR_GET_FEATURE_VALUE(dev_info, xpu_max_slaves, 0U) + 1U;
   }

   return 1U;
}

static inline uint32_t
rogue_get_cdm_context_resume_buffer_size(const struct pvr_device_info *dev_info)
{
   if (PVR_HAS_FEATURE(dev_info, gpu_multicore_support)) {
      const uint32_t max_num_cores = rogue_get_max_num_cores(dev_info);
      const uint32_t cache_line_size = rogue_get_slc_cache_line_size(dev_info);
      const uint32_t cdm_context_resume_buffer_stride =
         ALIGN_POT(ROGUE_LLS_CDM_CONTEXT_RESUME_BUFFER_SIZE, cache_line_size);

      return cdm_context_resume_buffer_stride * max_num_cores;
   }

   return ROGUE_LLS_CDM_CONTEXT_RESUME_BUFFER_SIZE;
}

static inline uint32_t rogue_get_cdm_context_resume_buffer_alignment(
   const struct pvr_device_info *dev_info)
{
   if (PVR_HAS_FEATURE(dev_info, gpu_multicore_support))
      return rogue_get_slc_cache_line_size(dev_info);

   return ROGUE_LLS_CDM_CONTEXT_RESUME_BUFFER_ALIGNMENT;
}

static inline uint32_t
rogue_get_compute_max_work_group_size(const struct pvr_device_info *dev_info)
{
   /* The number of tasks which can be executed per USC - Limited to 16U by the
    * CDM.
    */
   const uint32_t max_tasks_per_usc = 16U;

   if (!PVR_HAS_ERN(dev_info, 35421)) {
      /* Barriers on work-groups > 32 instances aren't supported. */
      return ROGUE_MAX_INSTANCES_PER_TASK;
   }

   return ROGUE_MAX_INSTANCES_PER_TASK * max_tasks_per_usc;
}

/* Don't use this directly. Use the x and y define macros. */
static inline uint32_t
__rogue_get_param_vf_max(const struct pvr_device_info *dev_info)
{
   return (rogue_get_render_size_max(dev_info) * 3 / 2) - 1;
}

#define rogue_get_param_vf_max_x(dev_info) __rogue_get_param_vf_max(dev_info)
#define rogue_get_param_vf_max_y(dev_info) __rogue_get_param_vf_max(dev_info)

#endif /* ROGUE_HW_UTILS_H */
