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
 */

#include <inttypes.h>

#include "radv_cs.h"
#include "radv_debug.h"
#include "radv_private.h"
#include "sid.h"

#include "vk_common_entrypoints.h"

#define SQTT_BUFFER_ALIGN_SHIFT 12

bool
radv_is_instruction_timing_enabled(void)
{
   return debug_get_bool_option("RADV_THREAD_TRACE_INSTRUCTION_TIMING", true);
}

bool
radv_sqtt_queue_events_enabled(void)
{
   return debug_get_bool_option("RADV_THREAD_TRACE_QUEUE_EVENTS", true);
}

static uint32_t
gfx11_get_sqtt_ctrl(const struct radv_device *device, bool enable)
{
   return S_0367B0_MODE(enable) | S_0367B0_HIWATER(5) | S_0367B0_UTIL_TIMER(1) | S_0367B0_RT_FREQ(2) | /* 4096 clk */
          S_0367B0_DRAW_EVENT_EN(1) | S_0367B0_SPI_STALL_EN(1) | S_0367B0_SQ_STALL_EN(1) | S_0367B0_REG_AT_HWM(2);
}

static uint32_t
gfx10_get_sqtt_ctrl(const struct radv_device *device, bool enable)
{
   uint32_t sqtt_ctrl = S_008D1C_MODE(enable) | S_008D1C_HIWATER(5) | S_008D1C_UTIL_TIMER(1) |
                        S_008D1C_RT_FREQ(2) | /* 4096 clk */
                        S_008D1C_DRAW_EVENT_EN(1) | S_008D1C_REG_STALL_EN(1) | S_008D1C_SPI_STALL_EN(1) |
                        S_008D1C_SQ_STALL_EN(1) | S_008D1C_REG_DROP_ON_STALL(0);

   if (device->physical_device->rad_info.gfx_level == GFX10_3)
      sqtt_ctrl |= S_008D1C_LOWATER_OFFSET(4);

   if (device->physical_device->rad_info.has_sqtt_auto_flush_mode_bug)
      sqtt_ctrl |= S_008D1C_AUTO_FLUSH_MODE(1);

   return sqtt_ctrl;
}

static enum radv_queue_family
radv_ip_to_queue_family(enum amd_ip_type t)
{
   switch (t) {
   case AMD_IP_GFX:
      return RADV_QUEUE_GENERAL;
   case AMD_IP_COMPUTE:
      return RADV_QUEUE_COMPUTE;
   case AMD_IP_SDMA:
      return RADV_QUEUE_TRANSFER;
   default:
      unreachable("Unknown IP type");
   }
}

static void
radv_emit_wait_for_idle(const struct radv_device *device, struct radeon_cmdbuf *cs, int family)
{
   const enum radv_queue_family qf = radv_ip_to_queue_family(family);
   enum rgp_flush_bits sqtt_flush_bits = 0;
   radv_cs_emit_cache_flush(
      device->ws, cs, device->physical_device->rad_info.gfx_level, NULL, 0, qf,
      (family == RADV_QUEUE_COMPUTE ? RADV_CMD_FLAG_CS_PARTIAL_FLUSH
                                    : (RADV_CMD_FLAG_CS_PARTIAL_FLUSH | RADV_CMD_FLAG_PS_PARTIAL_FLUSH)) |
         RADV_CMD_FLAG_INV_ICACHE | RADV_CMD_FLAG_INV_SCACHE | RADV_CMD_FLAG_INV_VCACHE | RADV_CMD_FLAG_INV_L2,
      &sqtt_flush_bits, 0);
}

static void
radv_emit_sqtt_start(const struct radv_device *device, struct radeon_cmdbuf *cs, enum radv_queue_family qf)
{
   const enum amd_gfx_level gfx_level = device->physical_device->rad_info.gfx_level;
   uint32_t shifted_size = device->sqtt.buffer_size >> SQTT_BUFFER_ALIGN_SHIFT;
   const struct radeon_info *rad_info = &device->physical_device->rad_info;
   const unsigned shader_mask = ac_sqtt_get_shader_mask(rad_info);
   unsigned max_se = rad_info->max_se;

   radeon_check_space(device->ws, cs, 6 + max_se * 33);

   for (unsigned se = 0; se < max_se; se++) {
      uint64_t va = radv_buffer_get_va(device->sqtt.bo);
      uint64_t data_va = ac_sqtt_get_data_va(rad_info, &device->sqtt, va, se);
      uint64_t shifted_va = data_va >> SQTT_BUFFER_ALIGN_SHIFT;
      int active_cu = ac_sqtt_get_active_cu(&device->physical_device->rad_info, se);

      if (ac_sqtt_se_is_disabled(rad_info, se))
         continue;

      /* Target SEx and SH0. */
      radeon_set_uconfig_reg(cs, R_030800_GRBM_GFX_INDEX,
                             S_030800_SE_INDEX(se) | S_030800_SH_INDEX(0) | S_030800_INSTANCE_BROADCAST_WRITES(1));

      if (device->physical_device->rad_info.gfx_level >= GFX11) {
         /* Order seems important for the following 2 registers. */
         radeon_set_perfctr_reg(gfx_level, qf, cs, R_0367A4_SQ_THREAD_TRACE_BUF0_SIZE,
                                S_0367A4_SIZE(shifted_size) | S_0367A4_BASE_HI(shifted_va >> 32));

         radeon_set_perfctr_reg(gfx_level, qf, cs, R_0367A0_SQ_THREAD_TRACE_BUF0_BASE, shifted_va);

         radeon_set_perfctr_reg(gfx_level, qf, cs, R_0367B4_SQ_THREAD_TRACE_MASK,
                                S_0367B4_WTYPE_INCLUDE(shader_mask) | S_0367B4_SA_SEL(0) |
                                   S_0367B4_WGP_SEL(active_cu / 2) | S_0367B4_SIMD_SEL(0));

         uint32_t sqtt_token_mask = S_0367B8_REG_INCLUDE(V_0367B8_REG_INCLUDE_SQDEC | V_0367B8_REG_INCLUDE_SHDEC |
                                                         V_0367B8_REG_INCLUDE_GFXUDEC | V_0367B8_REG_INCLUDE_COMP |
                                                         V_0367B8_REG_INCLUDE_CONTEXT | V_0367B8_REG_INCLUDE_CONFIG);

         /* Performance counters with SQTT are considered deprecated. */
         uint32_t token_exclude = V_0367B8_TOKEN_EXCLUDE_PERF;

         if (!radv_is_instruction_timing_enabled()) {
            /* Reduce SQTT traffic when instruction timing isn't enabled. */
            token_exclude |= V_0367B8_TOKEN_EXCLUDE_VMEMEXEC | V_0367B8_TOKEN_EXCLUDE_ALUEXEC |
                             V_0367B8_TOKEN_EXCLUDE_VALUINST | V_0367B8_TOKEN_EXCLUDE_IMMEDIATE |
                             V_0367B8_TOKEN_EXCLUDE_INST;
         }
         sqtt_token_mask |= S_0367B8_TOKEN_EXCLUDE(token_exclude) | S_0367B8_BOP_EVENTS_TOKEN_INCLUDE(1);

         radeon_set_perfctr_reg(gfx_level, qf, cs, R_0367B8_SQ_THREAD_TRACE_TOKEN_MASK, sqtt_token_mask);

         /* Should be emitted last (it enables thread traces). */
         radeon_set_perfctr_reg(gfx_level, qf, cs, R_0367B0_SQ_THREAD_TRACE_CTRL, gfx11_get_sqtt_ctrl(device, true));

      } else if (device->physical_device->rad_info.gfx_level >= GFX10) {
         /* Order seems important for the following 2 registers. */
         radeon_set_privileged_config_reg(cs, R_008D04_SQ_THREAD_TRACE_BUF0_SIZE,
                                          S_008D04_SIZE(shifted_size) | S_008D04_BASE_HI(shifted_va >> 32));

         radeon_set_privileged_config_reg(cs, R_008D00_SQ_THREAD_TRACE_BUF0_BASE, shifted_va);

         radeon_set_privileged_config_reg(cs, R_008D14_SQ_THREAD_TRACE_MASK,
                                          S_008D14_WTYPE_INCLUDE(shader_mask) | S_008D14_SA_SEL(0) |
                                             S_008D14_WGP_SEL(active_cu / 2) | S_008D14_SIMD_SEL(0));

         uint32_t sqtt_token_mask = S_008D18_REG_INCLUDE(V_008D18_REG_INCLUDE_SQDEC | V_008D18_REG_INCLUDE_SHDEC |
                                                         V_008D18_REG_INCLUDE_GFXUDEC | V_008D18_REG_INCLUDE_COMP |
                                                         V_008D18_REG_INCLUDE_CONTEXT | V_008D18_REG_INCLUDE_CONFIG);

         /* Performance counters with SQTT are considered deprecated. */
         uint32_t token_exclude = V_008D18_TOKEN_EXCLUDE_PERF;

         if (!radv_is_instruction_timing_enabled()) {
            /* Reduce SQTT traffic when instruction timing isn't enabled. */
            token_exclude |= V_008D18_TOKEN_EXCLUDE_VMEMEXEC | V_008D18_TOKEN_EXCLUDE_ALUEXEC |
                             V_008D18_TOKEN_EXCLUDE_VALUINST | V_008D18_TOKEN_EXCLUDE_IMMEDIATE |
                             V_008D18_TOKEN_EXCLUDE_INST;
         }
         sqtt_token_mask |=
            S_008D18_TOKEN_EXCLUDE(token_exclude) | S_008D18_BOP_EVENTS_TOKEN_INCLUDE(gfx_level == GFX10_3);

         radeon_set_privileged_config_reg(cs, R_008D18_SQ_THREAD_TRACE_TOKEN_MASK, sqtt_token_mask);

         /* Should be emitted last (it enables thread traces). */
         radeon_set_privileged_config_reg(cs, R_008D1C_SQ_THREAD_TRACE_CTRL, gfx10_get_sqtt_ctrl(device, true));
      } else {
         /* Order seems important for the following 4 registers. */
         radeon_set_uconfig_reg(cs, R_030CDC_SQ_THREAD_TRACE_BASE2, S_030CDC_ADDR_HI(shifted_va >> 32));

         radeon_set_uconfig_reg(cs, R_030CC0_SQ_THREAD_TRACE_BASE, shifted_va);

         radeon_set_uconfig_reg(cs, R_030CC4_SQ_THREAD_TRACE_SIZE, S_030CC4_SIZE(shifted_size));

         radeon_set_uconfig_reg(cs, R_030CD4_SQ_THREAD_TRACE_CTRL, S_030CD4_RESET_BUFFER(1));

         uint32_t sqtt_mask = S_030CC8_CU_SEL(active_cu) | S_030CC8_SH_SEL(0) | S_030CC8_SIMD_EN(0xf) |
                              S_030CC8_VM_ID_MASK(0) | S_030CC8_REG_STALL_EN(1) | S_030CC8_SPI_STALL_EN(1) |
                              S_030CC8_SQ_STALL_EN(1);

         if (device->physical_device->rad_info.gfx_level < GFX9) {
            sqtt_mask |= S_030CC8_RANDOM_SEED(0xffff);
         }

         radeon_set_uconfig_reg(cs, R_030CC8_SQ_THREAD_TRACE_MASK, sqtt_mask);

         /* Trace all tokens and registers. */
         radeon_set_uconfig_reg(cs, R_030CCC_SQ_THREAD_TRACE_TOKEN_MASK,
                                S_030CCC_TOKEN_MASK(0xbfff) | S_030CCC_REG_MASK(0xff) | S_030CCC_REG_DROP_ON_STALL(0));

         /* Enable SQTT perf counters for all CUs. */
         radeon_set_uconfig_reg(cs, R_030CD0_SQ_THREAD_TRACE_PERF_MASK,
                                S_030CD0_SH0_MASK(0xffff) | S_030CD0_SH1_MASK(0xffff));

         radeon_set_uconfig_reg(cs, R_030CE0_SQ_THREAD_TRACE_TOKEN_MASK2, 0xffffffff);

         radeon_set_uconfig_reg(cs, R_030CEC_SQ_THREAD_TRACE_HIWATER, S_030CEC_HIWATER(4));

         if (device->physical_device->rad_info.gfx_level == GFX9) {
            /* Reset thread trace status errors. */
            radeon_set_uconfig_reg(cs, R_030CE8_SQ_THREAD_TRACE_STATUS, S_030CE8_UTC_ERROR(0));
         }

         /* Enable the thread trace mode. */
         uint32_t sqtt_mode = S_030CD8_MASK_PS(1) | S_030CD8_MASK_VS(1) | S_030CD8_MASK_GS(1) | S_030CD8_MASK_ES(1) |
                              S_030CD8_MASK_HS(1) | S_030CD8_MASK_LS(1) | S_030CD8_MASK_CS(1) |
                              S_030CD8_AUTOFLUSH_EN(1) | /* periodically flush SQTT data to memory */
                              S_030CD8_MODE(1);

         if (device->physical_device->rad_info.gfx_level == GFX9) {
            /* Count SQTT traffic in TCC perf counters. */
            sqtt_mode |= S_030CD8_TC_PERF_EN(1);
         }

         radeon_set_uconfig_reg(cs, R_030CD8_SQ_THREAD_TRACE_MODE, sqtt_mode);
      }
   }

   /* Restore global broadcasting. */
   radeon_set_uconfig_reg(
      cs, R_030800_GRBM_GFX_INDEX,
      S_030800_SE_BROADCAST_WRITES(1) | S_030800_SH_BROADCAST_WRITES(1) | S_030800_INSTANCE_BROADCAST_WRITES(1));

   /* Start the thread trace with a different event based on the queue. */
   if (qf == RADV_QUEUE_COMPUTE) {
      radeon_set_sh_reg(cs, R_00B878_COMPUTE_THREAD_TRACE_ENABLE, S_00B878_THREAD_TRACE_ENABLE(1));
   } else {
      radeon_emit(cs, PKT3(PKT3_EVENT_WRITE, 0, 0));
      radeon_emit(cs, EVENT_TYPE(V_028A90_THREAD_TRACE_START) | EVENT_INDEX(0));
   }
}

static const uint32_t gfx8_sqtt_info_regs[] = {
   R_030CE4_SQ_THREAD_TRACE_WPTR,
   R_030CE8_SQ_THREAD_TRACE_STATUS,
   R_008E40_SQ_THREAD_TRACE_CNTR,
};

static const uint32_t gfx9_sqtt_info_regs[] = {
   R_030CE4_SQ_THREAD_TRACE_WPTR,
   R_030CE8_SQ_THREAD_TRACE_STATUS,
   R_030CF0_SQ_THREAD_TRACE_CNTR,
};

static const uint32_t gfx10_sqtt_info_regs[] = {
   R_008D10_SQ_THREAD_TRACE_WPTR,
   R_008D20_SQ_THREAD_TRACE_STATUS,
   R_008D24_SQ_THREAD_TRACE_DROPPED_CNTR,
};

static const uint32_t gfx11_sqtt_info_regs[] = {
   R_0367BC_SQ_THREAD_TRACE_WPTR,
   R_0367D0_SQ_THREAD_TRACE_STATUS,
   R_0367E8_SQ_THREAD_TRACE_DROPPED_CNTR,
};
static void
radv_copy_sqtt_info_regs(const struct radv_device *device, struct radeon_cmdbuf *cs, unsigned se_index)
{
   const struct radv_physical_device *pdevice = device->physical_device;
   const uint32_t *sqtt_info_regs = NULL;

   if (device->physical_device->rad_info.gfx_level >= GFX11) {
      sqtt_info_regs = gfx11_sqtt_info_regs;
   } else if (device->physical_device->rad_info.gfx_level >= GFX10) {
      sqtt_info_regs = gfx10_sqtt_info_regs;
   } else if (device->physical_device->rad_info.gfx_level == GFX9) {
      sqtt_info_regs = gfx9_sqtt_info_regs;
   } else {
      assert(device->physical_device->rad_info.gfx_level == GFX8);
      sqtt_info_regs = gfx8_sqtt_info_regs;
   }

   /* Get the VA where the info struct is stored for this SE. */
   uint64_t va = radv_buffer_get_va(device->sqtt.bo);
   uint64_t info_va = ac_sqtt_get_info_va(va, se_index);

   /* Copy back the info struct one DWORD at a time. */
   for (unsigned i = 0; i < 3; i++) {
      radeon_emit(cs, PKT3(PKT3_COPY_DATA, 4, 0));
      radeon_emit(cs, COPY_DATA_SRC_SEL(COPY_DATA_PERF) | COPY_DATA_DST_SEL(COPY_DATA_TC_L2) | COPY_DATA_WR_CONFIRM);
      radeon_emit(cs, sqtt_info_regs[i] >> 2);
      radeon_emit(cs, 0); /* unused */
      radeon_emit(cs, (info_va + i * 4));
      radeon_emit(cs, (info_va + i * 4) >> 32);
   }

   if (pdevice->rad_info.gfx_level >= GFX11) {
      /* On GFX11, SQ_THREAD_TRACE_WPTR is incremented from the "initial WPTR address" instead of 0.
       * To get the number of bytes (in units of 32 bytes) written by SQTT, the workaround is to
       * subtract SQ_THREAD_TRACE_WPTR from the "initial WPTR address" as follow:
       *
       * 1) get the current buffer base address for this SE
       * 2) shift right by 5 bits because SQ_THREAD_TRACE_WPTR is 32-byte aligned
       * 3) mask off the higher 3 bits because WPTR.OFFSET is 29 bits
       */
      uint64_t data_va = ac_sqtt_get_data_va(&pdevice->rad_info, &device->sqtt, va, se_index);
      uint64_t shifted_data_va = (data_va >> 5);
      uint32_t init_wptr_value = shifted_data_va & 0x1fffffff;

      radeon_emit(cs, PKT3(PKT3_ATOMIC_MEM, 7, 0));
      radeon_emit(cs, ATOMIC_OP(TC_OP_ATOMIC_SUB_32));
      radeon_emit(cs, info_va);         /* addr lo */
      radeon_emit(cs, info_va >> 32);   /* addr hi */
      radeon_emit(cs, init_wptr_value); /* data lo */
      radeon_emit(cs, 0);               /* data hi */
      radeon_emit(cs, 0);               /* compare data lo */
      radeon_emit(cs, 0);               /* compare data hi */
      radeon_emit(cs, 0);               /* loop interval */
   }
}

static void
radv_emit_sqtt_stop(const struct radv_device *device, struct radeon_cmdbuf *cs, enum radv_queue_family qf)
{
   const enum amd_gfx_level gfx_level = device->physical_device->rad_info.gfx_level;
   unsigned max_se = device->physical_device->rad_info.max_se;

   radeon_check_space(device->ws, cs, 8 + max_se * 64);

   /* Stop the thread trace with a different event based on the queue. */
   if (qf == RADV_QUEUE_COMPUTE) {
      radeon_set_sh_reg(cs, R_00B878_COMPUTE_THREAD_TRACE_ENABLE, S_00B878_THREAD_TRACE_ENABLE(0));
   } else {
      radeon_emit(cs, PKT3(PKT3_EVENT_WRITE, 0, 0));
      radeon_emit(cs, EVENT_TYPE(V_028A90_THREAD_TRACE_STOP) | EVENT_INDEX(0));
   }

   radeon_emit(cs, PKT3(PKT3_EVENT_WRITE, 0, 0));
   radeon_emit(cs, EVENT_TYPE(V_028A90_THREAD_TRACE_FINISH) | EVENT_INDEX(0));

   if (device->physical_device->rad_info.has_sqtt_rb_harvest_bug) {
      /* Some chips with disabled RBs should wait for idle because FINISH_DONE doesn't work. */
      radv_emit_wait_for_idle(device, cs, qf);
   }

   for (unsigned se = 0; se < max_se; se++) {
      if (ac_sqtt_se_is_disabled(&device->physical_device->rad_info, se))
         continue;

      /* Target SEi and SH0. */
      radeon_set_uconfig_reg(cs, R_030800_GRBM_GFX_INDEX,
                             S_030800_SE_INDEX(se) | S_030800_SH_INDEX(0) | S_030800_INSTANCE_BROADCAST_WRITES(1));

      if (device->physical_device->rad_info.gfx_level >= GFX11) {
         /* Make sure to wait for the trace buffer. */
         radeon_emit(cs, PKT3(PKT3_WAIT_REG_MEM, 5, 0));
         radeon_emit(cs, WAIT_REG_MEM_NOT_EQUAL); /* wait until the register is equal to the reference value */
         radeon_emit(cs, R_0367D0_SQ_THREAD_TRACE_STATUS >> 2); /* register */
         radeon_emit(cs, 0);
         radeon_emit(cs, 0); /* reference value */
         radeon_emit(cs, ~C_0367D0_FINISH_DONE);
         radeon_emit(cs, 4); /* poll interval */

         /* Disable the thread trace mode. */
         radeon_set_perfctr_reg(gfx_level, qf, cs, R_0367B0_SQ_THREAD_TRACE_CTRL, gfx11_get_sqtt_ctrl(device, false));

         /* Wait for thread trace completion. */
         radeon_emit(cs, PKT3(PKT3_WAIT_REG_MEM, 5, 0));
         radeon_emit(cs, WAIT_REG_MEM_EQUAL); /* wait until the register is equal to the reference value */
         radeon_emit(cs, R_0367D0_SQ_THREAD_TRACE_STATUS >> 2); /* register */
         radeon_emit(cs, 0);
         radeon_emit(cs, 0);              /* reference value */
         radeon_emit(cs, ~C_0367D0_BUSY); /* mask */
         radeon_emit(cs, 4);              /* poll interval */
      } else if (device->physical_device->rad_info.gfx_level >= GFX10) {
         if (!device->physical_device->rad_info.has_sqtt_rb_harvest_bug) {
            /* Make sure to wait for the trace buffer. */
            radeon_emit(cs, PKT3(PKT3_WAIT_REG_MEM, 5, 0));
            radeon_emit(cs, WAIT_REG_MEM_NOT_EQUAL); /* wait until the register is equal to the reference value */
            radeon_emit(cs, R_008D20_SQ_THREAD_TRACE_STATUS >> 2); /* register */
            radeon_emit(cs, 0);
            radeon_emit(cs, 0); /* reference value */
            radeon_emit(cs, ~C_008D20_FINISH_DONE);
            radeon_emit(cs, 4); /* poll interval */
         }

         /* Disable the thread trace mode. */
         radeon_set_privileged_config_reg(cs, R_008D1C_SQ_THREAD_TRACE_CTRL, gfx10_get_sqtt_ctrl(device, false));

         /* Wait for thread trace completion. */
         radeon_emit(cs, PKT3(PKT3_WAIT_REG_MEM, 5, 0));
         radeon_emit(cs, WAIT_REG_MEM_EQUAL); /* wait until the register is equal to the reference value */
         radeon_emit(cs, R_008D20_SQ_THREAD_TRACE_STATUS >> 2); /* register */
         radeon_emit(cs, 0);
         radeon_emit(cs, 0);              /* reference value */
         radeon_emit(cs, ~C_008D20_BUSY); /* mask */
         radeon_emit(cs, 4);              /* poll interval */
      } else {
         /* Disable the thread trace mode. */
         radeon_set_uconfig_reg(cs, R_030CD8_SQ_THREAD_TRACE_MODE, S_030CD8_MODE(0));

         /* Wait for thread trace completion. */
         radeon_emit(cs, PKT3(PKT3_WAIT_REG_MEM, 5, 0));
         radeon_emit(cs, WAIT_REG_MEM_EQUAL); /* wait until the register is equal to the reference value */
         radeon_emit(cs, R_030CE8_SQ_THREAD_TRACE_STATUS >> 2); /* register */
         radeon_emit(cs, 0);
         radeon_emit(cs, 0);              /* reference value */
         radeon_emit(cs, ~C_030CE8_BUSY); /* mask */
         radeon_emit(cs, 4);              /* poll interval */
      }

      radv_copy_sqtt_info_regs(device, cs, se);
   }

   /* Restore global broadcasting. */
   radeon_set_uconfig_reg(
      cs, R_030800_GRBM_GFX_INDEX,
      S_030800_SE_BROADCAST_WRITES(1) | S_030800_SH_BROADCAST_WRITES(1) | S_030800_INSTANCE_BROADCAST_WRITES(1));
}

void
radv_emit_sqtt_userdata(const struct radv_cmd_buffer *cmd_buffer, const void *data, uint32_t num_dwords)
{
   const enum amd_gfx_level gfx_level = cmd_buffer->device->physical_device->rad_info.gfx_level;
   const enum radv_queue_family qf = cmd_buffer->qf;
   struct radv_device *device = cmd_buffer->device;
   struct radeon_cmdbuf *cs = cmd_buffer->cs;
   const uint32_t *dwords = (uint32_t *)data;

   /* SQTT user data packets aren't supported on SDMA queues. */
   if (cmd_buffer->qf == RADV_QUEUE_TRANSFER)
      return;

   while (num_dwords > 0) {
      uint32_t count = MIN2(num_dwords, 2);

      radeon_check_space(device->ws, cs, 2 + count);

      /* Without the perfctr bit the CP might not always pass the
       * write on correctly. */
      if (device->physical_device->rad_info.gfx_level >= GFX10)
         radeon_set_uconfig_reg_seq_perfctr(gfx_level, qf, cs, R_030D08_SQ_THREAD_TRACE_USERDATA_2, count);
      else
         radeon_set_uconfig_reg_seq(cs, R_030D08_SQ_THREAD_TRACE_USERDATA_2, count);
      radeon_emit_array(cs, dwords, count);

      dwords += count;
      num_dwords -= count;
   }
}

void
radv_emit_spi_config_cntl(const struct radv_device *device, struct radeon_cmdbuf *cs, bool enable)
{
   if (device->physical_device->rad_info.gfx_level >= GFX9) {
      uint32_t spi_config_cntl = S_031100_GPR_WRITE_PRIORITY(0x2c688) | S_031100_EXP_PRIORITY_ORDER(3) |
                                 S_031100_ENABLE_SQG_TOP_EVENTS(enable) | S_031100_ENABLE_SQG_BOP_EVENTS(enable);

      if (device->physical_device->rad_info.gfx_level >= GFX10)
         spi_config_cntl |= S_031100_PS_PKR_PRIORITY_CNTL(3);

      radeon_set_uconfig_reg(cs, R_031100_SPI_CONFIG_CNTL, spi_config_cntl);
   } else {
      /* SPI_CONFIG_CNTL is a protected register on GFX6-GFX8. */
      radeon_set_privileged_config_reg(cs, R_009100_SPI_CONFIG_CNTL,
                                       S_009100_ENABLE_SQG_TOP_EVENTS(enable) | S_009100_ENABLE_SQG_BOP_EVENTS(enable));
   }
}

void
radv_emit_inhibit_clockgating(const struct radv_device *device, struct radeon_cmdbuf *cs, bool inhibit)
{
   if (device->physical_device->rad_info.gfx_level >= GFX11)
      return; /* not needed */

   if (device->physical_device->rad_info.gfx_level >= GFX10) {
      radeon_set_uconfig_reg(cs, R_037390_RLC_PERFMON_CLK_CNTL, S_037390_PERFMON_CLOCK_STATE(inhibit));
   } else if (device->physical_device->rad_info.gfx_level >= GFX8) {
      radeon_set_uconfig_reg(cs, R_0372FC_RLC_PERFMON_CLK_CNTL, S_0372FC_PERFMON_CLOCK_STATE(inhibit));
   }
}

VkResult
radv_sqtt_acquire_gpu_timestamp(struct radv_device *device, struct radeon_winsys_bo **gpu_timestamp_bo,
                                uint32_t *gpu_timestamp_offset, void **gpu_timestamp_ptr)
{
   struct radeon_winsys *ws = device->ws;

   simple_mtx_lock(&device->sqtt_timestamp_mtx);

   if (device->sqtt_timestamp.offset + 8 > device->sqtt_timestamp.size) {
      struct radeon_winsys_bo *bo;
      uint64_t new_size;
      VkResult result;
      uint8_t *map;

      new_size = MAX2(4096, 2 * device->sqtt_timestamp.size);

      result = ws->buffer_create(ws, new_size, 8, RADEON_DOMAIN_GTT,
                                 RADEON_FLAG_CPU_ACCESS | RADEON_FLAG_NO_INTERPROCESS_SHARING, RADV_BO_PRIORITY_SCRATCH,
                                 0, &bo);
      if (result != VK_SUCCESS) {
         simple_mtx_unlock(&device->sqtt_timestamp_mtx);
         return result;
      }

      map = device->ws->buffer_map(bo);
      if (!map) {
         ws->buffer_destroy(ws, bo);
         simple_mtx_unlock(&device->sqtt_timestamp_mtx);
         return VK_ERROR_OUT_OF_DEVICE_MEMORY;
      }

      if (device->sqtt_timestamp.bo) {
         struct radv_sqtt_timestamp *new_timestamp;

         new_timestamp = malloc(sizeof(*new_timestamp));
         if (!new_timestamp) {
            ws->buffer_destroy(ws, bo);
            simple_mtx_unlock(&device->sqtt_timestamp_mtx);
            return VK_ERROR_OUT_OF_HOST_MEMORY;
         }

         memcpy(new_timestamp, &device->sqtt_timestamp, sizeof(*new_timestamp));
         list_add(&new_timestamp->list, &device->sqtt_timestamp.list);
      }

      device->sqtt_timestamp.bo = bo;
      device->sqtt_timestamp.size = new_size;
      device->sqtt_timestamp.offset = 0;
      device->sqtt_timestamp.map = map;
   }

   *gpu_timestamp_bo = device->sqtt_timestamp.bo;
   *gpu_timestamp_offset = device->sqtt_timestamp.offset;
   *gpu_timestamp_ptr = device->sqtt_timestamp.map + device->sqtt_timestamp.offset;

   device->sqtt_timestamp.offset += 8;

   simple_mtx_unlock(&device->sqtt_timestamp_mtx);

   return VK_SUCCESS;
}

static void
radv_sqtt_reset_timestamp(struct radv_device *device)
{
   struct radeon_winsys *ws = device->ws;

   simple_mtx_lock(&device->sqtt_timestamp_mtx);

   list_for_each_entry_safe (struct radv_sqtt_timestamp, ts, &device->sqtt_timestamp.list, list) {
      ws->buffer_destroy(ws, ts->bo);
      list_del(&ts->list);
      free(ts);
   }

   device->sqtt_timestamp.offset = 0;

   simple_mtx_unlock(&device->sqtt_timestamp_mtx);
}

static bool
radv_sqtt_init_queue_event(struct radv_device *device)
{
   VkCommandPool cmd_pool;
   VkResult result;

   const VkCommandPoolCreateInfo create_gfx_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .queueFamilyIndex = RADV_QUEUE_GENERAL, /* Graphics queue is always the first queue. */
   };

   result = vk_common_CreateCommandPool(radv_device_to_handle(device), &create_gfx_info, NULL, &cmd_pool);
   if (result != VK_SUCCESS)
      return false;

   device->sqtt_command_pool[0] = vk_command_pool_from_handle(cmd_pool);

   if (!(device->instance->debug_flags & RADV_DEBUG_NO_COMPUTE_QUEUE)) {
      const VkCommandPoolCreateInfo create_comp_info = {
         .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
         .queueFamilyIndex = RADV_QUEUE_COMPUTE,
      };

      result = vk_common_CreateCommandPool(radv_device_to_handle(device), &create_comp_info, NULL, &cmd_pool);
      if (result != VK_SUCCESS)
         return false;

      device->sqtt_command_pool[1] = vk_command_pool_from_handle(cmd_pool);
   }

   simple_mtx_init(&device->sqtt_command_pool_mtx, mtx_plain);

   simple_mtx_init(&device->sqtt_timestamp_mtx, mtx_plain);
   list_inithead(&device->sqtt_timestamp.list);

   return true;
}

static void
radv_sqtt_finish_queue_event(struct radv_device *device)
{
   struct radeon_winsys *ws = device->ws;

   if (device->sqtt_timestamp.bo)
      ws->buffer_destroy(ws, device->sqtt_timestamp.bo);

   simple_mtx_destroy(&device->sqtt_timestamp_mtx);

   for (unsigned i = 0; i < ARRAY_SIZE(device->sqtt_command_pool); i++)
      vk_common_DestroyCommandPool(radv_device_to_handle(device),
                                   vk_command_pool_to_handle(device->sqtt_command_pool[i]), NULL);

   simple_mtx_destroy(&device->sqtt_command_pool_mtx);
}

static bool
radv_sqtt_init_bo(struct radv_device *device)
{
   unsigned max_se = device->physical_device->rad_info.max_se;
   struct radeon_winsys *ws = device->ws;
   VkResult result;
   uint64_t size;

   /* The buffer size and address need to be aligned in HW regs. Align the
    * size as early as possible so that we do all the allocation & addressing
    * correctly. */
   device->sqtt.buffer_size = align64(device->sqtt.buffer_size, 1u << SQTT_BUFFER_ALIGN_SHIFT);

   /* Compute total size of the thread trace BO for all SEs. */
   size = align64(sizeof(struct ac_sqtt_data_info) * max_se, 1 << SQTT_BUFFER_ALIGN_SHIFT);
   size += device->sqtt.buffer_size * (uint64_t)max_se;

   struct radeon_winsys_bo *bo = NULL;
   result = ws->buffer_create(ws, size, 4096, RADEON_DOMAIN_VRAM,
                              RADEON_FLAG_CPU_ACCESS | RADEON_FLAG_NO_INTERPROCESS_SHARING | RADEON_FLAG_ZERO_VRAM,
                              RADV_BO_PRIORITY_SCRATCH, 0, &bo);
   device->sqtt.bo = bo;
   if (result != VK_SUCCESS)
      return false;

   result = ws->buffer_make_resident(ws, device->sqtt.bo, true);
   if (result != VK_SUCCESS)
      return false;

   device->sqtt.ptr = ws->buffer_map(device->sqtt.bo);
   if (!device->sqtt.ptr)
      return false;

   return true;
}

static void
radv_sqtt_finish_bo(struct radv_device *device)
{
   struct radeon_winsys *ws = device->ws;

   if (unlikely(device->sqtt.bo)) {
      ws->buffer_make_resident(ws, device->sqtt.bo, false);
      ws->buffer_destroy(ws, device->sqtt.bo);
   }
}

static VkResult
radv_register_queue(struct radv_device *device, struct radv_queue *queue)
{
   struct ac_sqtt *sqtt = &device->sqtt;
   struct rgp_queue_info *queue_info = &sqtt->rgp_queue_info;
   struct rgp_queue_info_record *record;

   record = malloc(sizeof(struct rgp_queue_info_record));
   if (!record)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   record->queue_id = (uintptr_t)queue;
   record->queue_context = (uintptr_t)queue->hw_ctx;
   if (queue->vk.queue_family_index == RADV_QUEUE_GENERAL) {
      record->hardware_info.queue_type = SQTT_QUEUE_TYPE_UNIVERSAL;
      record->hardware_info.engine_type = SQTT_ENGINE_TYPE_UNIVERSAL;
   } else {
      record->hardware_info.queue_type = SQTT_QUEUE_TYPE_COMPUTE;
      record->hardware_info.engine_type = SQTT_ENGINE_TYPE_COMPUTE;
   }

   simple_mtx_lock(&queue_info->lock);
   list_addtail(&record->list, &queue_info->record);
   queue_info->record_count++;
   simple_mtx_unlock(&queue_info->lock);

   return VK_SUCCESS;
}

static void
radv_unregister_queue(struct radv_device *device, struct radv_queue *queue)
{
   struct ac_sqtt *sqtt = &device->sqtt;
   struct rgp_queue_info *queue_info = &sqtt->rgp_queue_info;

   /* Destroy queue info record. */
   simple_mtx_lock(&queue_info->lock);
   if (queue_info->record_count > 0) {
      list_for_each_entry_safe (struct rgp_queue_info_record, record, &queue_info->record, list) {
         if (record->queue_id == (uintptr_t)queue) {
            queue_info->record_count--;
            list_del(&record->list);
            free(record);
            break;
         }
      }
   }
   simple_mtx_unlock(&queue_info->lock);
}

static void
radv_register_queues(struct radv_device *device, struct ac_sqtt *sqtt)
{
   if (device->queue_count[RADV_QUEUE_GENERAL] == 1)
      radv_register_queue(device, &device->queues[RADV_QUEUE_GENERAL][0]);

   for (uint32_t i = 0; i < device->queue_count[RADV_QUEUE_COMPUTE]; i++)
      radv_register_queue(device, &device->queues[RADV_QUEUE_COMPUTE][i]);
}

static void
radv_unregister_queues(struct radv_device *device, struct ac_sqtt *sqtt)
{
   if (device->queue_count[RADV_QUEUE_GENERAL] == 1)
      radv_unregister_queue(device, &device->queues[RADV_QUEUE_GENERAL][0]);

   for (uint32_t i = 0; i < device->queue_count[RADV_QUEUE_COMPUTE]; i++)
      radv_unregister_queue(device, &device->queues[RADV_QUEUE_COMPUTE][i]);
}

bool
radv_sqtt_init(struct radv_device *device)
{
   struct ac_sqtt *sqtt = &device->sqtt;

   /* Default buffer size set to 32MB per SE. */
   device->sqtt.buffer_size = (uint32_t)debug_get_num_option("RADV_THREAD_TRACE_BUFFER_SIZE", 32 * 1024 * 1024);

   if (!radv_sqtt_init_bo(device))
      return false;

   if (!radv_sqtt_init_queue_event(device))
      return false;

   if (!radv_device_acquire_performance_counters(device))
      return false;

   ac_sqtt_init(sqtt);

   radv_register_queues(device, sqtt);

   return true;
}

void
radv_sqtt_finish(struct radv_device *device)
{
   struct ac_sqtt *sqtt = &device->sqtt;
   struct radeon_winsys *ws = device->ws;

   radv_sqtt_finish_bo(device);
   radv_sqtt_finish_queue_event(device);

   for (unsigned i = 0; i < 2; i++) {
      if (device->sqtt.start_cs[i])
         ws->cs_destroy(device->sqtt.start_cs[i]);
      if (device->sqtt.stop_cs[i])
         ws->cs_destroy(device->sqtt.stop_cs[i]);
   }

   radv_unregister_queues(device, sqtt);

   ac_sqtt_finish(sqtt);
}

static bool
radv_sqtt_resize_bo(struct radv_device *device)
{
   /* Destroy the previous thread trace BO. */
   radv_sqtt_finish_bo(device);

   /* Double the size of the thread trace buffer per SE. */
   device->sqtt.buffer_size *= 2;

   fprintf(stderr,
           "Failed to get the thread trace because the buffer "
           "was too small, resizing to %d KB\n",
           device->sqtt.buffer_size / 1024);

   /* Re-create the thread trace BO. */
   return radv_sqtt_init_bo(device);
}

bool
radv_begin_sqtt(struct radv_queue *queue)
{
   struct radv_device *device = queue->device;
   enum radv_queue_family family = queue->state.qf;
   struct radeon_winsys *ws = device->ws;
   struct radeon_cmdbuf *cs;
   VkResult result;

   /* Destroy the previous start CS and create a new one. */
   if (device->sqtt.start_cs[family]) {
      ws->cs_destroy(device->sqtt.start_cs[family]);
      device->sqtt.start_cs[family] = NULL;
   }

   cs = ws->cs_create(ws, radv_queue_ring(queue), false);
   if (!cs)
      return false;

   radeon_check_space(ws, cs, 512);

   switch (family) {
   case RADV_QUEUE_GENERAL:
      radeon_emit(cs, PKT3(PKT3_CONTEXT_CONTROL, 1, 0));
      radeon_emit(cs, CC0_UPDATE_LOAD_ENABLES(1));
      radeon_emit(cs, CC1_UPDATE_SHADOW_ENABLES(1));
      break;
   case RADV_QUEUE_COMPUTE:
      radeon_emit(cs, PKT3(PKT3_NOP, 0, 0));
      radeon_emit(cs, 0);
      break;
   default:
      unreachable("Incorrect queue family");
      break;
   }

   /* Make sure to wait-for-idle before starting SQTT. */
   radv_emit_wait_for_idle(device, cs, family);

   /* Disable clock gating before starting SQTT. */
   radv_emit_inhibit_clockgating(device, cs, true);

   /* Enable SQG events that collects thread trace data. */
   radv_emit_spi_config_cntl(device, cs, true);

   radv_perfcounter_emit_spm_reset(cs);

   if (device->spm.bo) {
      /* Enable all shader stages by default. */
      radv_perfcounter_emit_shaders(device, cs, ac_sqtt_get_shader_mask(&device->physical_device->rad_info));

      radv_emit_spm_setup(device, cs, family);
   }

   /* Start SQTT. */
   radv_emit_sqtt_start(device, cs, family);

   if (device->spm.bo)
      radv_perfcounter_emit_spm_start(device, cs, family);

   result = ws->cs_finalize(cs);
   if (result != VK_SUCCESS) {
      ws->cs_destroy(cs);
      return false;
   }

   device->sqtt.start_cs[family] = cs;

   return radv_queue_internal_submit(queue, cs);
}

bool
radv_end_sqtt(struct radv_queue *queue)
{
   struct radv_device *device = queue->device;
   enum radv_queue_family family = queue->state.qf;
   struct radeon_winsys *ws = device->ws;
   struct radeon_cmdbuf *cs;
   VkResult result;

   /* Destroy the previous stop CS and create a new one. */
   if (queue->device->sqtt.stop_cs[family]) {
      ws->cs_destroy(device->sqtt.stop_cs[family]);
      device->sqtt.stop_cs[family] = NULL;
   }

   cs = ws->cs_create(ws, radv_queue_ring(queue), false);
   if (!cs)
      return false;

   radeon_check_space(ws, cs, 512);

   switch (family) {
   case RADV_QUEUE_GENERAL:
      radeon_emit(cs, PKT3(PKT3_CONTEXT_CONTROL, 1, 0));
      radeon_emit(cs, CC0_UPDATE_LOAD_ENABLES(1));
      radeon_emit(cs, CC1_UPDATE_SHADOW_ENABLES(1));
      break;
   case RADV_QUEUE_COMPUTE:
      radeon_emit(cs, PKT3(PKT3_NOP, 0, 0));
      radeon_emit(cs, 0);
      break;
   default:
      unreachable("Incorrect queue family");
      break;
   }

   /* Make sure to wait-for-idle before stopping SQTT. */
   radv_emit_wait_for_idle(device, cs, family);

   if (device->spm.bo)
      radv_perfcounter_emit_spm_stop(device, cs, family);

   /* Stop SQTT. */
   radv_emit_sqtt_stop(device, cs, family);

   radv_perfcounter_emit_spm_reset(cs);

   /* Restore previous state by disabling SQG events. */
   radv_emit_spi_config_cntl(device, cs, false);

   /* Restore previous state by re-enabling clock gating. */
   radv_emit_inhibit_clockgating(device, cs, false);

   result = ws->cs_finalize(cs);
   if (result != VK_SUCCESS) {
      ws->cs_destroy(cs);
      return false;
   }

   device->sqtt.stop_cs[family] = cs;

   return radv_queue_internal_submit(queue, cs);
}

bool
radv_get_sqtt_trace(struct radv_queue *queue, struct ac_sqtt_trace *sqtt_trace)
{
   struct radv_device *device = queue->device;
   const struct radeon_info *rad_info = &device->physical_device->rad_info;

   if (!ac_sqtt_get_trace(&device->sqtt, rad_info, sqtt_trace)) {
      if (!radv_sqtt_resize_bo(device))
         fprintf(stderr, "radv: Failed to resize the SQTT buffer.\n");
      return false;
   }

   return true;
}

void
radv_reset_sqtt_trace(struct radv_device *device)
{
   struct ac_sqtt *sqtt = &device->sqtt;
   struct rgp_clock_calibration *clock_calibration = &sqtt->rgp_clock_calibration;
   struct rgp_queue_event *queue_event = &sqtt->rgp_queue_event;

   /* Clear clock calibration records. */
   simple_mtx_lock(&clock_calibration->lock);
   list_for_each_entry_safe (struct rgp_clock_calibration_record, record, &clock_calibration->record, list) {
      clock_calibration->record_count--;
      list_del(&record->list);
      free(record);
   }
   simple_mtx_unlock(&clock_calibration->lock);

   /* Clear queue event records. */
   simple_mtx_lock(&queue_event->lock);
   list_for_each_entry_safe (struct rgp_queue_event_record, record, &queue_event->record, list) {
      list_del(&record->list);
      free(record);
   }
   queue_event->record_count = 0;
   simple_mtx_unlock(&queue_event->lock);

   /* Clear timestamps. */
   radv_sqtt_reset_timestamp(device);

   /* Clear timed cmdbufs. */
   simple_mtx_lock(&device->sqtt_command_pool_mtx);
   for (unsigned i = 0; i < ARRAY_SIZE(device->sqtt_command_pool); i++) {
      vk_common_TrimCommandPool(radv_device_to_handle(device), vk_command_pool_to_handle(device->sqtt_command_pool[i]),
                                0);
   }
   simple_mtx_unlock(&device->sqtt_command_pool_mtx);
}

static VkResult
radv_get_calibrated_timestamps(struct radv_device *device, uint64_t *cpu_timestamp, uint64_t *gpu_timestamp)
{
   uint64_t timestamps[2];
   uint64_t max_deviation;
   VkResult result;

   const VkCalibratedTimestampInfoKHR timestamp_infos[2] = {{
                                                               .sType = VK_STRUCTURE_TYPE_CALIBRATED_TIMESTAMP_INFO_KHR,
                                                               .timeDomain = VK_TIME_DOMAIN_CLOCK_MONOTONIC_KHR,
                                                            },
                                                            {
                                                               .sType = VK_STRUCTURE_TYPE_CALIBRATED_TIMESTAMP_INFO_KHR,
                                                               .timeDomain = VK_TIME_DOMAIN_DEVICE_KHR,
                                                            }};

   result =
      radv_GetCalibratedTimestampsKHR(radv_device_to_handle(device), 2, timestamp_infos, timestamps, &max_deviation);
   if (result != VK_SUCCESS)
      return result;

   *cpu_timestamp = timestamps[0];
   *gpu_timestamp = timestamps[1];

   return result;
}

bool
radv_sqtt_sample_clocks(struct radv_device *device)
{
   uint64_t cpu_timestamp = 0, gpu_timestamp = 0;
   VkResult result;

   result = radv_get_calibrated_timestamps(device, &cpu_timestamp, &gpu_timestamp);
   if (result != VK_SUCCESS)
      return false;

   return ac_sqtt_add_clock_calibration(&device->sqtt, cpu_timestamp, gpu_timestamp);
}

VkResult
radv_sqtt_get_timed_cmdbuf(struct radv_queue *queue, struct radeon_winsys_bo *timestamp_bo, uint32_t timestamp_offset,
                           VkPipelineStageFlags2 timestamp_stage, VkCommandBuffer *pcmdbuf)
{
   struct radv_device *device = queue->device;
   enum radv_queue_family queue_family = queue->state.qf;
   VkCommandBuffer cmdbuf;
   uint64_t timestamp_va;
   VkResult result;

   assert(queue_family == RADV_QUEUE_GENERAL || queue_family == RADV_QUEUE_COMPUTE);

   simple_mtx_lock(&device->sqtt_command_pool_mtx);

   const VkCommandBufferAllocateInfo alloc_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = vk_command_pool_to_handle(device->sqtt_command_pool[queue_family]),
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1,
   };

   result = vk_common_AllocateCommandBuffers(radv_device_to_handle(device), &alloc_info, &cmdbuf);
   if (result != VK_SUCCESS)
      goto fail;

   const VkCommandBufferBeginInfo begin_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
   };

   result = radv_BeginCommandBuffer(cmdbuf, &begin_info);
   if (result != VK_SUCCESS)
      goto fail;

   radeon_check_space(device->ws, radv_cmd_buffer_from_handle(cmdbuf)->cs, 28);

   timestamp_va = radv_buffer_get_va(timestamp_bo) + timestamp_offset;

   radv_cs_add_buffer(device->ws, radv_cmd_buffer_from_handle(cmdbuf)->cs, timestamp_bo);

   radv_write_timestamp(radv_cmd_buffer_from_handle(cmdbuf), timestamp_va, timestamp_stage);

   result = radv_EndCommandBuffer(cmdbuf);
   if (result != VK_SUCCESS)
      goto fail;

   *pcmdbuf = cmdbuf;

fail:
   simple_mtx_unlock(&device->sqtt_command_pool_mtx);
   return result;
}
