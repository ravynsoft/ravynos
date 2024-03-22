/*
 * Copyright © 2021 Valve Corporation
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

#include <inttypes.h>

#include "radv_cs.h"
#include "radv_private.h"
#include "sid.h"

#define SPM_RING_BASE_ALIGN 32

static bool
radv_spm_init_bo(struct radv_device *device)
{
   struct radeon_winsys *ws = device->ws;
   uint64_t size = 32 * 1024 * 1024; /* Default to 1MB. */
   uint16_t sample_interval = 4096;  /* Default to 4096 clk. */
   VkResult result;

   device->spm.buffer_size = size;
   device->spm.sample_interval = sample_interval;

   struct radeon_winsys_bo *bo = NULL;
   result = ws->buffer_create(ws, size, 4096, RADEON_DOMAIN_VRAM,
                              RADEON_FLAG_CPU_ACCESS | RADEON_FLAG_NO_INTERPROCESS_SHARING | RADEON_FLAG_ZERO_VRAM,
                              RADV_BO_PRIORITY_SCRATCH, 0, &bo);
   device->spm.bo = bo;
   if (result != VK_SUCCESS)
      return false;

   result = ws->buffer_make_resident(ws, device->spm.bo, true);
   if (result != VK_SUCCESS)
      return false;

   device->spm.ptr = ws->buffer_map(device->spm.bo);
   if (!device->spm.ptr)
      return false;

   return true;
}

static void
radv_emit_spm_counters(struct radv_device *device, struct radeon_cmdbuf *cs, enum radv_queue_family qf)
{
   const enum amd_gfx_level gfx_level = device->physical_device->rad_info.gfx_level;
   struct ac_spm *spm = &device->spm;

   if (gfx_level >= GFX11) {
      for (uint32_t instance = 0; instance < ARRAY_SIZE(spm->sq_wgp); instance++) {
         uint32_t num_counters = spm->sq_wgp[instance].num_counters;

         if (!num_counters)
            continue;

         radeon_check_space(device->ws, cs, 3 + num_counters * 3);

         radeon_set_uconfig_reg(cs, R_030800_GRBM_GFX_INDEX, spm->sq_wgp[instance].grbm_gfx_index);

         for (uint32_t b = 0; b < num_counters; b++) {
            const struct ac_spm_counter_select *cntr_sel = &spm->sq_wgp[instance].counters[b];
            uint32_t reg_base = R_036700_SQ_PERFCOUNTER0_SELECT;

            radeon_set_uconfig_reg_seq_perfctr(gfx_level, qf, cs, reg_base + b * 4, 1);
            radeon_emit(cs, cntr_sel->sel0);
         }
      }
   }

   for (uint32_t instance = 0; instance < ARRAY_SIZE(spm->sqg); instance++) {
      uint32_t num_counters = spm->sqg[instance].num_counters;

      if (!num_counters)
         continue;

      radeon_check_space(device->ws, cs, 3 + num_counters * 3);

      radeon_set_uconfig_reg(
         cs, R_030800_GRBM_GFX_INDEX,
         S_030800_SH_BROADCAST_WRITES(1) | S_030800_INSTANCE_BROADCAST_WRITES(1) | S_030800_SE_INDEX(instance));

      for (uint32_t b = 0; b < num_counters; b++) {
         const struct ac_spm_counter_select *cntr_sel = &spm->sqg[instance].counters[b];
         uint32_t reg_base = R_036700_SQ_PERFCOUNTER0_SELECT;

         radeon_set_uconfig_reg_seq_perfctr(gfx_level, qf, cs, reg_base + b * 4, 1);
         radeon_emit(cs, cntr_sel->sel0 | S_036700_SQC_BANK_MASK(0xf)); /* SQC_BANK_MASK only gfx10 */
      }
   }

   for (uint32_t b = 0; b < spm->num_block_sel; b++) {
      struct ac_spm_block_select *block_sel = &spm->block_sel[b];
      struct ac_pc_block_base *regs = block_sel->b->b->b;

      for (unsigned i = 0; i < block_sel->num_instances; i++) {
         struct ac_spm_block_instance *block_instance = &block_sel->instances[i];

         radeon_check_space(device->ws, cs, 3 + (AC_SPM_MAX_COUNTER_PER_BLOCK * 6));

         radeon_set_uconfig_reg(cs, R_030800_GRBM_GFX_INDEX, block_instance->grbm_gfx_index);

         for (unsigned c = 0; c < block_instance->num_counters; c++) {
            const struct ac_spm_counter_select *cntr_sel = &block_instance->counters[c];

            if (!cntr_sel->active)
               continue;

            radeon_set_uconfig_reg_seq_perfctr(gfx_level, qf, cs, regs->select0[c], 1);
            radeon_emit(cs, cntr_sel->sel0);

            radeon_set_uconfig_reg_seq_perfctr(gfx_level, qf, cs, regs->select1[c], 1);
            radeon_emit(cs, cntr_sel->sel1);
         }
      }
   }

   /* Restore global broadcasting. */
   radeon_set_uconfig_reg(
      cs, R_030800_GRBM_GFX_INDEX,
      S_030800_SE_BROADCAST_WRITES(1) | S_030800_SH_BROADCAST_WRITES(1) | S_030800_INSTANCE_BROADCAST_WRITES(1));
}

void
radv_emit_spm_setup(struct radv_device *device, struct radeon_cmdbuf *cs, enum radv_queue_family qf)
{
   const enum amd_gfx_level gfx_level = device->physical_device->rad_info.gfx_level;
   struct ac_spm *spm = &device->spm;
   uint64_t va = radv_buffer_get_va(spm->bo);
   uint64_t ring_size = spm->buffer_size;

   /* It's required that the ring VA and the size are correctly aligned. */
   assert(!(va & (SPM_RING_BASE_ALIGN - 1)));
   assert(!(ring_size & (SPM_RING_BASE_ALIGN - 1)));
   assert(spm->sample_interval >= 32);

   radeon_check_space(device->ws, cs, 27);

   /* Configure the SPM ring buffer. */
   radeon_set_uconfig_reg(cs, R_037200_RLC_SPM_PERFMON_CNTL,
                          S_037200_PERFMON_RING_MODE(0) | /* no stall and no interrupt on overflow */
                             S_037200_PERFMON_SAMPLE_INTERVAL(spm->sample_interval)); /* in sclk */
   radeon_set_uconfig_reg(cs, R_037204_RLC_SPM_PERFMON_RING_BASE_LO, va);
   radeon_set_uconfig_reg(cs, R_037208_RLC_SPM_PERFMON_RING_BASE_HI, S_037208_RING_BASE_HI(va >> 32));
   radeon_set_uconfig_reg(cs, R_03720C_RLC_SPM_PERFMON_RING_SIZE, ring_size);

   /* Configure the muxsel. */
   uint32_t total_muxsel_lines = 0;
   for (unsigned s = 0; s < AC_SPM_SEGMENT_TYPE_COUNT; s++) {
      total_muxsel_lines += spm->num_muxsel_lines[s];
   }

   radeon_set_uconfig_reg(cs, R_03726C_RLC_SPM_ACCUM_MODE, 0);

   if (device->physical_device->rad_info.gfx_level >= GFX11) {
      radeon_set_uconfig_reg(cs, R_03721C_RLC_SPM_PERFMON_SEGMENT_SIZE,
                             S_03721C_TOTAL_NUM_SEGMENT(total_muxsel_lines) |
                                S_03721C_GLOBAL_NUM_SEGMENT(spm->num_muxsel_lines[AC_SPM_SEGMENT_TYPE_GLOBAL]) |
                                S_03721C_SE_NUM_SEGMENT(spm->max_se_muxsel_lines));

      radeon_set_uconfig_reg(cs, R_037210_RLC_SPM_RING_WRPTR, 0);
   } else {
      radeon_set_uconfig_reg(cs, R_037210_RLC_SPM_PERFMON_SEGMENT_SIZE, 0);
      radeon_set_uconfig_reg(cs, R_03727C_RLC_SPM_PERFMON_SE3TO0_SEGMENT_SIZE,
                             S_03727C_SE0_NUM_LINE(spm->num_muxsel_lines[AC_SPM_SEGMENT_TYPE_SE0]) |
                                S_03727C_SE1_NUM_LINE(spm->num_muxsel_lines[AC_SPM_SEGMENT_TYPE_SE1]) |
                                S_03727C_SE2_NUM_LINE(spm->num_muxsel_lines[AC_SPM_SEGMENT_TYPE_SE2]) |
                                S_03727C_SE3_NUM_LINE(spm->num_muxsel_lines[AC_SPM_SEGMENT_TYPE_SE3]));
      radeon_set_uconfig_reg(cs, R_037280_RLC_SPM_PERFMON_GLB_SEGMENT_SIZE,
                             S_037280_PERFMON_SEGMENT_SIZE(total_muxsel_lines) |
                                S_037280_GLOBAL_NUM_LINE(spm->num_muxsel_lines[AC_SPM_SEGMENT_TYPE_GLOBAL]));
   }

   /* Upload each muxsel ram to the RLC. */
   for (unsigned s = 0; s < AC_SPM_SEGMENT_TYPE_COUNT; s++) {
      unsigned rlc_muxsel_addr, rlc_muxsel_data;
      unsigned grbm_gfx_index = S_030800_SH_BROADCAST_WRITES(1) | S_030800_INSTANCE_BROADCAST_WRITES(1);

      if (!spm->num_muxsel_lines[s])
         continue;

      if (s == AC_SPM_SEGMENT_TYPE_GLOBAL) {
         grbm_gfx_index |= S_030800_SE_BROADCAST_WRITES(1);

         rlc_muxsel_addr =
            gfx_level >= GFX11 ? R_037220_RLC_SPM_GLOBAL_MUXSEL_ADDR : R_037224_RLC_SPM_GLOBAL_MUXSEL_ADDR;
         rlc_muxsel_data =
            gfx_level >= GFX11 ? R_037224_RLC_SPM_GLOBAL_MUXSEL_DATA : R_037228_RLC_SPM_GLOBAL_MUXSEL_DATA;
      } else {
         grbm_gfx_index |= S_030800_SE_INDEX(s);

         rlc_muxsel_addr = gfx_level >= GFX11 ? R_037228_RLC_SPM_SE_MUXSEL_ADDR : R_03721C_RLC_SPM_SE_MUXSEL_ADDR;
         rlc_muxsel_data = gfx_level >= GFX11 ? R_03722C_RLC_SPM_SE_MUXSEL_DATA : R_037220_RLC_SPM_SE_MUXSEL_DATA;
      }

      radeon_check_space(device->ws, cs, 3 + spm->num_muxsel_lines[s] * (7 + AC_SPM_MUXSEL_LINE_SIZE));

      radeon_set_uconfig_reg(cs, R_030800_GRBM_GFX_INDEX, grbm_gfx_index);

      for (unsigned l = 0; l < spm->num_muxsel_lines[s]; l++) {
         uint32_t *data = (uint32_t *)spm->muxsel_lines[s][l].muxsel;

         /* Select MUXSEL_ADDR to point to the next muxsel. */
         radeon_set_uconfig_reg_perfctr(gfx_level, qf, cs, rlc_muxsel_addr, l * AC_SPM_MUXSEL_LINE_SIZE);

         /* Write the muxsel line configuration with MUXSEL_DATA. */
         radeon_emit(cs, PKT3(PKT3_WRITE_DATA, 2 + AC_SPM_MUXSEL_LINE_SIZE, 0));
         radeon_emit(cs, S_370_DST_SEL(V_370_MEM_MAPPED_REGISTER) | S_370_WR_CONFIRM(1) | S_370_ENGINE_SEL(V_370_ME) |
                            S_370_WR_ONE_ADDR(1));
         radeon_emit(cs, rlc_muxsel_data >> 2);
         radeon_emit(cs, 0);
         radeon_emit_array(cs, data, AC_SPM_MUXSEL_LINE_SIZE);
      }
   }

   /* Select SPM counters. */
   radv_emit_spm_counters(device, cs, qf);
}

bool
radv_spm_init(struct radv_device *device)
{
   const struct radeon_info *info = &device->physical_device->rad_info;
   struct ac_perfcounters *pc = &device->physical_device->ac_perfcounters;

   /* We failed to initialize the performance counters. */
   if (!pc->blocks)
      return false;

   if (!ac_init_spm(info, pc, &device->spm))
      return false;

   if (!radv_spm_init_bo(device))
      return false;

   return true;
}

void
radv_spm_finish(struct radv_device *device)
{
   struct radeon_winsys *ws = device->ws;

   if (device->spm.bo) {
      ws->buffer_make_resident(ws, device->spm.bo, false);
      ws->buffer_destroy(ws, device->spm.bo);
   }

   ac_destroy_spm(&device->spm);
}
