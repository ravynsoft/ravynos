/*
 * Copyright 2020 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#include "amd_family.h"
#include "si_build_pm4.h"
#include "si_pipe.h"

#include "tgsi/tgsi_from_mesa.h"
#include "util/hash_table.h"
#include "util/u_debug.h"
#include "util/u_memory.h"
#include "ac_rgp.h"
#include "ac_sqtt.h"

static void
si_emit_spi_config_cntl(struct si_context *sctx,
                        struct radeon_cmdbuf *cs, bool enable);

static bool si_sqtt_init_bo(struct si_context *sctx)
{
   unsigned max_se = sctx->screen->info.max_se;
   struct radeon_winsys *ws = sctx->ws;
   uint64_t size;

   /* The buffer size and address need to be aligned in HW regs. Align the
    * size as early as possible so that we do all the allocation & addressing
    * correctly. */
   sctx->sqtt->buffer_size =
      align64(sctx->sqtt->buffer_size, 1u << SQTT_BUFFER_ALIGN_SHIFT);

   /* Compute total size of the thread trace BO for all SEs. */
   size = align64(sizeof(struct ac_sqtt_data_info) * max_se,
                  1 << SQTT_BUFFER_ALIGN_SHIFT);
   size += sctx->sqtt->buffer_size * (uint64_t)max_se;

   sctx->sqtt->pipeline_bos = _mesa_hash_table_u64_create(NULL);

   sctx->sqtt->bo =
      ws->buffer_create(ws, size, 4096, RADEON_DOMAIN_VRAM,
                        RADEON_FLAG_NO_INTERPROCESS_SHARING |
                           RADEON_FLAG_GTT_WC | RADEON_FLAG_NO_SUBALLOC);
   if (!sctx->sqtt->bo)
      return false;

   return true;
}

static void si_emit_sqtt_start(struct si_context *sctx,
                               struct radeon_cmdbuf *cs,
                               enum amd_ip_type ip_type)
{
   struct si_screen *sscreen = sctx->screen;
   uint32_t shifted_size = sctx->sqtt->buffer_size >> SQTT_BUFFER_ALIGN_SHIFT;
   unsigned max_se = sscreen->info.max_se;

   radeon_begin(cs);

   for (unsigned se = 0; se < max_se; se++) {
      uint64_t va = sctx->ws->buffer_get_virtual_address(sctx->sqtt->bo);
      uint64_t data_va =
         ac_sqtt_get_data_va(&sctx->screen->info, sctx->sqtt, va, se);
      uint64_t shifted_va = data_va >> SQTT_BUFFER_ALIGN_SHIFT;

      if (ac_sqtt_se_is_disabled(&sctx->screen->info, se))
         continue;

      /* Target SEx and SH0. */
      radeon_set_uconfig_perfctr_reg_seq(R_030800_GRBM_GFX_INDEX, 1);
      radeon_emit(S_030800_SE_INDEX(se) | S_030800_SH_INDEX(0) |
                  S_030800_INSTANCE_BROADCAST_WRITES(1));

      /* Select the first active CUs */
      int first_active_cu = ffs(sctx->screen->info.cu_mask[se][0]);

      if (sctx->gfx_level >= GFX10) {
         uint32_t token_mask =
            V_008D18_REG_INCLUDE_SQDEC | V_008D18_REG_INCLUDE_SHDEC |
            V_008D18_REG_INCLUDE_GFXUDEC | V_008D18_REG_INCLUDE_CONTEXT |
            V_008D18_REG_INCLUDE_COMP | V_008D18_REG_INCLUDE_CONFIG;
         int wgp = first_active_cu / 2;
         unsigned shader_mask = 0x7f; /* all shader stages */

         /* Order seems important for the following 2 registers. */
         if (sctx->gfx_level >= GFX11) {
            /* Disable unsupported hw shader stages */
            shader_mask &= ~(0x02 /* VS */ | 0x08 /* ES */ | 0x20 /* LS */);

            radeon_set_uconfig_perfctr_reg_seq(R_0367A0_SQ_THREAD_TRACE_BUF0_BASE, 2);
            radeon_emit(shifted_va);
            radeon_emit(S_0367A4_SIZE(shifted_size) |
                        S_0367A4_BASE_HI(shifted_va >> 32));

            radeon_set_uconfig_perfctr_reg_seq(R_0367B4_SQ_THREAD_TRACE_MASK, 2);
            radeon_emit(S_0367B4_WTYPE_INCLUDE(shader_mask) |
                        S_0367B4_SA_SEL(0) | S_0367B4_WGP_SEL(wgp) |
                        S_0367B4_SIMD_SEL(0));
            radeon_emit(S_0367B8_REG_INCLUDE(token_mask) |
                        S_0367B8_TOKEN_EXCLUDE(V_008D18_TOKEN_EXCLUDE_PERF));
         } else {
            radeon_set_privileged_config_reg(
               R_008D04_SQ_THREAD_TRACE_BUF0_SIZE,
               S_008D04_SIZE(shifted_size) | S_008D04_BASE_HI(shifted_va >> 32));

            radeon_set_privileged_config_reg(R_008D00_SQ_THREAD_TRACE_BUF0_BASE,
                                             shifted_va);

            radeon_set_privileged_config_reg(
               R_008D14_SQ_THREAD_TRACE_MASK,
               S_008D14_WTYPE_INCLUDE(shader_mask) | S_008D14_SA_SEL(0) |
               S_008D14_WGP_SEL(wgp) | S_008D14_SIMD_SEL(0));

            radeon_set_privileged_config_reg(
               R_008D18_SQ_THREAD_TRACE_TOKEN_MASK,
               S_008D18_REG_INCLUDE(token_mask) |
               S_008D18_TOKEN_EXCLUDE(V_008D18_TOKEN_EXCLUDE_PERF));
         }

         /* Should be emitted last (it enables thread traces). */
         uint32_t ctrl = S_008D1C_MODE(1) | S_008D1C_HIWATER(5) |
                         S_008D1C_UTIL_TIMER(1) | S_008D1C_RT_FREQ(2) /* 4096 clk */ |
                         S_008D1C_DRAW_EVENT_EN(1);

         if (sctx->gfx_level == GFX10_3)
            ctrl |= S_008D1C_LOWATER_OFFSET(4);

         ctrl |= S_008D1C_AUTO_FLUSH_MODE(
            sctx->screen->info.has_sqtt_auto_flush_mode_bug);

         switch (sctx->gfx_level) {
            case GFX10:
            case GFX10_3:
               ctrl |= S_008D1C_REG_STALL_EN(1) |
                       S_008D1C_SPI_STALL_EN(1) |
                       S_008D1C_SQ_STALL_EN(1) |
                       S_008D1C_REG_DROP_ON_STALL(0);
               radeon_set_privileged_config_reg(R_008D1C_SQ_THREAD_TRACE_CTRL, ctrl);
               break;
            case GFX11:
               ctrl |= S_0367B0_SPI_STALL_EN(1) |
                       S_0367B0_SQ_STALL_EN(1) |
                       S_0367B0_REG_AT_HWM(2);
               radeon_set_uconfig_perfctr_reg_seq(R_0367B0_SQ_THREAD_TRACE_CTRL, 1);
               radeon_emit(ctrl);
               break;
            default:
               assert(false);
         }
      } else {
         /* Order seems important for the following 4 registers. */
         radeon_set_uconfig_reg(R_030CDC_SQ_THREAD_TRACE_BASE2,
                                S_030CDC_ADDR_HI(shifted_va >> 32));

         radeon_set_uconfig_reg(R_030CC0_SQ_THREAD_TRACE_BASE, shifted_va);

         radeon_set_uconfig_reg(R_030CC4_SQ_THREAD_TRACE_SIZE,
                                S_030CC4_SIZE(shifted_size));

         radeon_set_uconfig_reg(R_030CD4_SQ_THREAD_TRACE_CTRL,
                                S_030CD4_RESET_BUFFER(1));

         uint32_t sqtt_mask = S_030CC8_CU_SEL(first_active_cu) |
                              S_030CC8_SH_SEL(0) | S_030CC8_SIMD_EN(0xf) |
                              S_030CC8_VM_ID_MASK(0) | S_030CC8_REG_STALL_EN(1) |
                              S_030CC8_SPI_STALL_EN(1) | S_030CC8_SQ_STALL_EN(1);

         radeon_set_uconfig_reg(R_030CC8_SQ_THREAD_TRACE_MASK, sqtt_mask);

         /* Trace all tokens and registers. */
         radeon_set_uconfig_reg(R_030CCC_SQ_THREAD_TRACE_TOKEN_MASK,
                                S_030CCC_TOKEN_MASK(0xbfff) |
                                S_030CCC_REG_MASK(0xff) |
                                S_030CCC_REG_DROP_ON_STALL(0));

         /* Enable SQTT perf counters for all CUs. */
         radeon_set_uconfig_reg(R_030CD0_SQ_THREAD_TRACE_PERF_MASK,
                                S_030CD0_SH0_MASK(0xffff) |
                                S_030CD0_SH1_MASK(0xffff));

         radeon_set_uconfig_reg(R_030CE0_SQ_THREAD_TRACE_TOKEN_MASK2, 0xffffffff);

         radeon_set_uconfig_reg(R_030CEC_SQ_THREAD_TRACE_HIWATER,
                                S_030CEC_HIWATER(4));

         if (sctx->gfx_level == GFX9) {
            /* Reset thread trace status errors. */
            radeon_set_uconfig_reg(R_030CE8_SQ_THREAD_TRACE_STATUS,
                                   S_030CE8_UTC_ERROR(0));
         }

         /* Enable the thread trace mode. */
         uint32_t sqtt_mode = S_030CD8_MASK_PS(1) |
                              S_030CD8_MASK_VS(1) |
                              S_030CD8_MASK_GS(1) |
                              S_030CD8_MASK_ES(1) |
                              S_030CD8_MASK_HS(1) |
                              S_030CD8_MASK_LS(1) |
                              S_030CD8_MASK_CS(1) |
                              S_030CD8_AUTOFLUSH_EN(1) | /* periodically flush SQTT data to memory */
                              S_030CD8_MODE(1);

         if (sctx->gfx_level == GFX9) {
            /* Count SQTT traffic in TCC perf counters. */
            sqtt_mode |= S_030CD8_TC_PERF_EN(1);
         }

         radeon_set_uconfig_reg(R_030CD8_SQ_THREAD_TRACE_MODE, sqtt_mode);
      }
   }

   /* Restore global broadcasting. */
   radeon_set_uconfig_reg(R_030800_GRBM_GFX_INDEX,
                          S_030800_SE_BROADCAST_WRITES(1) |
                          S_030800_SH_BROADCAST_WRITES(1) |
                          S_030800_INSTANCE_BROADCAST_WRITES(1));

   /* Start the thread trace with a different event based on the queue. */
   if (ip_type == AMD_IP_COMPUTE) {
      radeon_set_sh_reg(R_00B878_COMPUTE_THREAD_TRACE_ENABLE,
                        S_00B878_THREAD_TRACE_ENABLE(1));
   } else {
      radeon_emit(PKT3(PKT3_EVENT_WRITE, 0, 0));
      radeon_emit(EVENT_TYPE(V_028A90_THREAD_TRACE_START) | EVENT_INDEX(0));
   }
   radeon_end();
}

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

static void si_copy_sqtt_info_regs(struct si_context *sctx,
                                   struct radeon_cmdbuf *cs,
                                   unsigned se_index)
{
   const uint32_t *sqtt_info_regs = NULL;

   switch (sctx->gfx_level) {
      case GFX10_3:
      case GFX10:
         sqtt_info_regs = gfx10_sqtt_info_regs;
         break;
      case GFX11:
         sqtt_info_regs = gfx11_sqtt_info_regs;
         break;
      case GFX9:
         sqtt_info_regs = gfx9_sqtt_info_regs;
         break;
      default:
         unreachable("Unsupported gfx_level");
   }

   /* Get the VA where the info struct is stored for this SE. */
   uint64_t va = sctx->ws->buffer_get_virtual_address(sctx->sqtt->bo);
   uint64_t info_va = ac_sqtt_get_info_va(va, se_index);

   radeon_begin(cs);

   /* Copy back the info struct one DWORD at a time. */
   for (unsigned i = 0; i < 3; i++) {
      radeon_emit(PKT3(PKT3_COPY_DATA, 4, 0));
      radeon_emit(COPY_DATA_SRC_SEL(COPY_DATA_PERF) |
                  COPY_DATA_DST_SEL(COPY_DATA_TC_L2) | COPY_DATA_WR_CONFIRM);
      radeon_emit(sqtt_info_regs[i] >> 2);
      radeon_emit(0); /* unused */
      radeon_emit((info_va + i * 4));
      radeon_emit((info_va + i * 4) >> 32);
   }

   if (sctx->gfx_level == GFX11) {
      /* On GFX11, WPTR is incremented from the offset of the current buffer base
       * address and it needs to be subtracted to get the correct offset:
       *
       * 1) get the current buffer base address for this SE
       * 2) shift right by 5 bits because SQ_THREAD_TRACE_WPTR is 32-byte aligned
       * 3) mask off the higher 3 bits because WPTR.OFFSET is 29 bits
       */
      uint64_t data_va =
         ac_sqtt_get_data_va(&sctx->screen->info, sctx->sqtt, va, se_index);
      uint64_t shifted_data_va = (data_va >> 5);
      uint64_t init_wptr_value = shifted_data_va & 0x1fffffff;

      radeon_emit(PKT3(PKT3_ATOMIC_MEM, 7, 0));
      radeon_emit(ATOMIC_OP(TC_OP_ATOMIC_SUB_32));
      radeon_emit(info_va);
      radeon_emit(info_va >> 32);
      radeon_emit(init_wptr_value);
      radeon_emit(init_wptr_value >> 32);
      radeon_emit(0);
      radeon_emit(0);
      radeon_emit(0);
   }

   radeon_end();
}

static void si_emit_sqtt_stop(struct si_context *sctx, struct radeon_cmdbuf *cs,
                              enum amd_ip_type ip_type)
{
   unsigned max_se = sctx->screen->info.max_se;
   radeon_begin(cs);

   /* Stop the thread trace with a different event based on the queue. */
   if (ip_type == AMD_IP_COMPUTE) {
      radeon_set_sh_reg(R_00B878_COMPUTE_THREAD_TRACE_ENABLE,
                        S_00B878_THREAD_TRACE_ENABLE(0));
   } else {
      radeon_emit(PKT3(PKT3_EVENT_WRITE, 0, 0));
      radeon_emit(EVENT_TYPE(V_028A90_THREAD_TRACE_STOP) | EVENT_INDEX(0));
   }

   radeon_emit(PKT3(PKT3_EVENT_WRITE, 0, 0));
   radeon_emit(EVENT_TYPE(V_028A90_THREAD_TRACE_FINISH) | EVENT_INDEX(0));
   radeon_end();

   if (sctx->screen->info.has_sqtt_rb_harvest_bug) {
      /* Some chips with disabled RBs should wait for idle because FINISH_DONE
       * doesn't work. */
      sctx->flags |= SI_CONTEXT_FLUSH_AND_INV_CB | SI_CONTEXT_FLUSH_AND_INV_DB |
                     SI_CONTEXT_CS_PARTIAL_FLUSH;
      sctx->emit_cache_flush(sctx, cs);
   }

   for (unsigned se = 0; se < max_se; se++) {
      if (ac_sqtt_se_is_disabled(&sctx->screen->info, se))
         continue;

      radeon_begin(cs);

      /* Target SEi and SH0. */
      radeon_set_uconfig_reg(R_030800_GRBM_GFX_INDEX,
                             S_030800_SE_INDEX(se) | S_030800_SH_INDEX(0) |
                             S_030800_INSTANCE_BROADCAST_WRITES(1));

      if (sctx->gfx_level >= GFX10) {
         uint32_t tt_status_reg = sctx->gfx_level >= GFX11 ?
                                     R_0367D0_SQ_THREAD_TRACE_STATUS:
                                     R_008D20_SQ_THREAD_TRACE_STATUS;
         if (!sctx->screen->info.has_sqtt_rb_harvest_bug) {
            /* Make sure to wait for the trace buffer. */
            radeon_emit(PKT3(PKT3_WAIT_REG_MEM, 5, 0));
            radeon_emit(WAIT_REG_MEM_NOT_EQUAL); /* wait until the register is equal
                                                    to the reference value */
            radeon_emit(tt_status_reg >> 2);     /* register */
            radeon_emit(0);
            radeon_emit(0);                      /* reference value */
            radeon_emit(sctx->gfx_level >= GFX11 ? ~C_0367D0_FINISH_DONE:
                                                   ~C_008D20_FINISH_DONE); /* mask */
            radeon_emit(4);                          /* poll interval */
         }

         /* Disable the thread trace mode. */
         if (sctx->gfx_level >= GFX11) {
            radeon_set_uconfig_perfctr_reg_seq(R_0367B0_SQ_THREAD_TRACE_CTRL, 1);
            radeon_emit(S_008D1C_MODE(0));
         } else {
            radeon_set_privileged_config_reg(R_008D1C_SQ_THREAD_TRACE_CTRL,
                                             S_008D1C_MODE(0));
         }

         /* Wait for thread trace completion. */
         radeon_emit(PKT3(PKT3_WAIT_REG_MEM, 5, 0));
         radeon_emit(WAIT_REG_MEM_EQUAL);                        /* wait until the register is equal to
                                                                    the reference value */
         radeon_emit(tt_status_reg >> 2);                        /* register */
         radeon_emit(0);
         radeon_emit(0);                                         /* reference value */
         radeon_emit(sctx->gfx_level >= GFX11 ? ~C_0367D0_BUSY:
                                                ~C_008D20_BUSY); /* mask */
         radeon_emit(4);                                         /* poll interval */
      } else {
         /* Disable the thread trace mode. */
         radeon_set_uconfig_reg(R_030CD8_SQ_THREAD_TRACE_MODE, S_030CD8_MODE(0));

         /* Wait for thread trace completion. */
         radeon_emit(PKT3(PKT3_WAIT_REG_MEM, 5, 0));
         radeon_emit(WAIT_REG_MEM_EQUAL);                   /* wait until the register is equal to
                                                               the reference value */
         radeon_emit(R_030CE8_SQ_THREAD_TRACE_STATUS >> 2); /* register */
         radeon_emit(0);
         radeon_emit(0);                                    /* reference value */
         radeon_emit(~C_030CE8_BUSY);                       /* mask */
         radeon_emit(4);                                    /* poll interval */
      }
      radeon_end();

      si_copy_sqtt_info_regs(sctx, cs, se);
   }

   /* Restore global broadcasting. */
   radeon_begin_again(cs);
   radeon_set_uconfig_reg(R_030800_GRBM_GFX_INDEX,
                          S_030800_SE_BROADCAST_WRITES(1) |
                          S_030800_SH_BROADCAST_WRITES(1) |
                          S_030800_INSTANCE_BROADCAST_WRITES(1));
   radeon_end();
}

static void si_sqtt_start(struct si_context *sctx, struct radeon_cmdbuf *cs)
{
   struct radeon_winsys *ws = sctx->ws;
   enum amd_ip_type ip_type = sctx->ws->cs_get_ip_type(cs);

   radeon_begin(cs);

   switch (ip_type) {
      case AMD_IP_GFX:
         radeon_emit(PKT3(PKT3_CONTEXT_CONTROL, 1, 0));
         radeon_emit(CC0_UPDATE_LOAD_ENABLES(1));
         radeon_emit(CC1_UPDATE_SHADOW_ENABLES(1));
         break;
      case AMD_IP_COMPUTE:
         radeon_emit(PKT3(PKT3_NOP, 0, 0));
         radeon_emit(0);
         break;
      default:
        /* Unsupported. */
        assert(false);
   }
   radeon_end();

   ws->cs_add_buffer(cs, sctx->sqtt->bo, RADEON_USAGE_READWRITE,
                     RADEON_DOMAIN_VRAM);
   if (sctx->spm.bo)
      ws->cs_add_buffer(cs, sctx->spm.bo, RADEON_USAGE_READWRITE,
                        RADEON_DOMAIN_VRAM);

   si_cp_dma_wait_for_idle(sctx, cs);

   /* Make sure to wait-for-idle before starting SQTT. */
   sctx->flags |= SI_CONTEXT_PS_PARTIAL_FLUSH | SI_CONTEXT_CS_PARTIAL_FLUSH |
                  SI_CONTEXT_INV_ICACHE | SI_CONTEXT_INV_SCACHE |
                  SI_CONTEXT_INV_VCACHE | SI_CONTEXT_INV_L2 |
                  SI_CONTEXT_PFP_SYNC_ME;
   sctx->emit_cache_flush(sctx, cs);

   si_inhibit_clockgating(sctx, cs, true);

   /* Enable SQG events that collects thread trace data. */
   si_emit_spi_config_cntl(sctx, cs, true);

   if (sctx->spm.bo) {
      si_pc_emit_spm_reset(cs);
      si_pc_emit_shaders(cs, 0x7f);
      si_emit_spm_setup(sctx, cs);
   }

   si_emit_sqtt_start(sctx, cs, ip_type);

   if (sctx->spm.bo)
      si_pc_emit_spm_start(cs);
}

static void si_sqtt_stop(struct si_context *sctx, struct radeon_cmdbuf *cs)
{
   struct radeon_winsys *ws = sctx->ws;
   enum amd_ip_type ip_type = sctx->ws->cs_get_ip_type(cs);

   radeon_begin(cs);

   switch (ip_type) {
      case AMD_IP_GFX:
         radeon_emit(PKT3(PKT3_CONTEXT_CONTROL, 1, 0));
         radeon_emit(CC0_UPDATE_LOAD_ENABLES(1));
         radeon_emit(CC1_UPDATE_SHADOW_ENABLES(1));
         break;
      case AMD_IP_COMPUTE:
         radeon_emit(PKT3(PKT3_NOP, 0, 0));
         radeon_emit(0);
         break;
      default:
        /* Unsupported. */
        assert(false);
   }
   radeon_end();

   ws->cs_add_buffer(cs, sctx->sqtt->bo, RADEON_USAGE_READWRITE,
                     RADEON_DOMAIN_VRAM);

   if (sctx->spm.bo)
      ws->cs_add_buffer(cs, sctx->spm.bo, RADEON_USAGE_READWRITE,
                        RADEON_DOMAIN_VRAM);

   si_cp_dma_wait_for_idle(sctx, cs);

   if (sctx->spm.bo)
      si_pc_emit_spm_stop(cs, sctx->screen->info.never_stop_sq_perf_counters,
                          sctx->screen->info.never_send_perfcounter_stop);

   /* Make sure to wait-for-idle before stopping SQTT. */
   sctx->flags |= SI_CONTEXT_PS_PARTIAL_FLUSH | SI_CONTEXT_CS_PARTIAL_FLUSH |
                  SI_CONTEXT_INV_ICACHE | SI_CONTEXT_INV_SCACHE |
                  SI_CONTEXT_INV_VCACHE | SI_CONTEXT_INV_L2 |
                  SI_CONTEXT_PFP_SYNC_ME;
   sctx->emit_cache_flush(sctx, cs);

   si_emit_sqtt_stop(sctx, cs, ip_type);

   if (sctx->spm.bo)
      si_pc_emit_spm_reset(cs);

   /* Restore previous state by disabling SQG events. */
   si_emit_spi_config_cntl(sctx, cs, false);

   si_inhibit_clockgating(sctx, cs, false);
}

static void si_sqtt_init_cs(struct si_context *sctx)
{
   struct radeon_winsys *ws = sctx->ws;

   for (unsigned i = 0; i < ARRAY_SIZE(sctx->sqtt->start_cs); i++) {
      sctx->sqtt->start_cs[i] = CALLOC_STRUCT(radeon_cmdbuf);
      if (!ws->cs_create(sctx->sqtt->start_cs[i], sctx->ctx, (enum amd_ip_type)i,
                         NULL, NULL)) {
         free(sctx->sqtt->start_cs[i]);
         sctx->sqtt->start_cs[i] = NULL;
         return;
      }
      si_sqtt_start(sctx, sctx->sqtt->start_cs[i]);

      sctx->sqtt->stop_cs[i] = CALLOC_STRUCT(radeon_cmdbuf);
      if (!ws->cs_create(sctx->sqtt->stop_cs[i], sctx->ctx, (enum amd_ip_type)i,
                         NULL, NULL)) {
         ws->cs_destroy(sctx->sqtt->start_cs[i]);
         free(sctx->sqtt->start_cs[i]);
         sctx->sqtt->start_cs[i] = NULL;
         free(sctx->sqtt->stop_cs[i]);
         sctx->sqtt->stop_cs[i] = NULL;
         return;
      }

      si_sqtt_stop(sctx, sctx->sqtt->stop_cs[i]);
   }
}

static void si_begin_sqtt(struct si_context *sctx, struct radeon_cmdbuf *rcs)
{
   struct radeon_cmdbuf *cs = sctx->sqtt->start_cs[sctx->ws->cs_get_ip_type(rcs)];
   sctx->ws->cs_flush(cs, 0, NULL);
}

static void si_end_sqtt(struct si_context *sctx, struct radeon_cmdbuf *rcs)
{
   struct radeon_cmdbuf *cs = sctx->sqtt->stop_cs[sctx->ws->cs_get_ip_type(rcs)];
   sctx->ws->cs_flush(cs, 0, &sctx->last_sqtt_fence);
}

static bool si_get_sqtt_trace(struct si_context *sctx,
                              struct ac_sqtt_trace *sqtt)
{
   unsigned max_se = sctx->screen->info.max_se;

   memset(sqtt, 0, sizeof(*sqtt));

   sctx->sqtt->ptr =
      sctx->ws->buffer_map(sctx->ws, sctx->sqtt->bo, NULL, PIPE_MAP_READ);

   if (!sctx->sqtt->ptr)
      return false;

   if (!ac_sqtt_get_trace(sctx->sqtt, &sctx->screen->info, sqtt)) {
      void *sqtt_ptr = sctx->sqtt->ptr;

      for (unsigned se = 0; se < max_se; se++) {
         uint64_t info_offset = ac_sqtt_get_info_offset(se);
         void *info_ptr = sqtt_ptr + info_offset;
         struct ac_sqtt_data_info *info = (struct ac_sqtt_data_info *)info_ptr;

         if (ac_sqtt_se_is_disabled(&sctx->screen->info, se))
            continue;

         if (!ac_is_sqtt_complete(&sctx->screen->info, sctx->sqtt, info)) {
            uint32_t expected_size =
               ac_get_expected_buffer_size(&sctx->screen->info, info);
            uint32_t available_size = (info->cur_offset * 32) / 1024;

            fprintf(stderr,
                    "Failed to get the thread trace "
                    "because the buffer is too small. The "
                    "hardware needs %d KB but the "
                    "buffer size is %d KB.\n",
                    expected_size, available_size);
            fprintf(stderr, "Please update the buffer size with "
                            "AMD_THREAD_TRACE_BUFFER_SIZE=<size_in_kbytes>\n");
            return false;
         }
      }
   }

   return true;
}

bool si_init_sqtt(struct si_context *sctx)
{
   static bool warn_once = true;
   if (warn_once) {
      fprintf(stderr, "*************************************************\n");
      fprintf(stderr, "* WARNING: Thread trace support is experimental *\n");
      fprintf(stderr, "*************************************************\n");
      warn_once = false;
   }

   sctx->sqtt = CALLOC_STRUCT(ac_sqtt);

   if (sctx->gfx_level < GFX8) {
      fprintf(stderr, "GPU hardware not supported: refer to "
                      "the RGP documentation for the list of "
                      "supported GPUs!\n");
      return false;
   }

   if (sctx->gfx_level > GFX11) {
      fprintf(stderr, "radeonsi: Thread trace is not supported "
                      "for that GPU!\n");
      return false;
   }

   /* Default buffer size set to 32MB per SE. */
   sctx->sqtt->buffer_size =
      debug_get_num_option("AMD_THREAD_TRACE_BUFFER_SIZE", 32 * 1024) * 1024;
   sctx->sqtt->start_frame = 10;

   const char *trigger = getenv("AMD_THREAD_TRACE_TRIGGER");
   if (trigger) {
      sctx->sqtt->start_frame = atoi(trigger);
      if (sctx->sqtt->start_frame <= 0) {
         /* This isn't a frame number, must be a file */
         sctx->sqtt->trigger_file = strdup(trigger);
         sctx->sqtt->start_frame = -1;
      }
   }

   if (!si_sqtt_init_bo(sctx))
      return false;

   ac_sqtt_init(sctx->sqtt);

   if (sctx->gfx_level >= GFX10 &&
       debug_get_bool_option("AMD_THREAD_TRACE_SPM", sctx->gfx_level < GFX11)) {
      /* Limit SPM counters to GFX10 and GFX10_3 for now */
      ASSERTED bool r = si_spm_init(sctx);
      assert(r);
   }

   si_sqtt_init_cs(sctx);

   sctx->sqtt_next_event = EventInvalid;

   return true;
}

void si_destroy_sqtt(struct si_context *sctx)
{
   struct si_screen *sscreen = sctx->screen;
   struct pb_buffer_lean *bo = sctx->sqtt->bo;
   radeon_bo_reference(sctx->screen->ws, &bo, NULL);

   if (sctx->sqtt->trigger_file)
      free(sctx->sqtt->trigger_file);

   for (int i = 0; i < ARRAY_SIZE(sctx->sqtt->start_cs); i++) {
      sscreen->ws->cs_destroy(sctx->sqtt->start_cs[i]);
      sscreen->ws->cs_destroy(sctx->sqtt->stop_cs[i]);
   }

   struct rgp_pso_correlation *pso_correlation =
      &sctx->sqtt->rgp_pso_correlation;
   struct rgp_loader_events *loader_events = &sctx->sqtt->rgp_loader_events;
   struct rgp_code_object *code_object = &sctx->sqtt->rgp_code_object;
   list_for_each_entry_safe (struct rgp_pso_correlation_record, record,
                             &pso_correlation->record, list) {
      list_del(&record->list);
      pso_correlation->record_count--;
      free(record);
   }

   list_for_each_entry_safe (struct rgp_loader_events_record, record,
                             &loader_events->record, list) {
      list_del(&record->list);
      loader_events->record_count--;
      free(record);
   }

   list_for_each_entry_safe (struct rgp_code_object_record, record,
                             &code_object->record, list) {
      uint32_t mask = record->shader_stages_mask;
      int i;

      /* Free the disassembly. */
      while (mask) {
         i = u_bit_scan(&mask);
         free(record->shader_data[i].code);
      }
      list_del(&record->list);
      free(record);
      code_object->record_count--;
   }

   ac_sqtt_finish(sctx->sqtt);

   hash_table_foreach (sctx->sqtt->pipeline_bos->table, entry) {
      struct si_sqtt_fake_pipeline *pipeline =
         (struct si_sqtt_fake_pipeline *)entry->data;
      si_resource_reference(&pipeline->bo, NULL);
      FREE(pipeline);
   }

   free(sctx->sqtt);
   sctx->sqtt = NULL;

   if (sctx->spm.bo)
      si_spm_finish(sctx);
}

static uint64_t num_frames = 0;

void si_handle_sqtt(struct si_context *sctx, struct radeon_cmdbuf *rcs)
{
   /* Should we enable SQTT yet? */
   if (!sctx->sqtt_enabled) {
      bool frame_trigger = num_frames == sctx->sqtt->start_frame;
      bool file_trigger = false;
      if (sctx->sqtt->trigger_file &&
          access(sctx->sqtt->trigger_file, W_OK) == 0) {
         if (unlink(sctx->sqtt->trigger_file) == 0) {
            file_trigger = true;
         } else {
            /* Do not enable tracing if we cannot remove the file,
             * because by then we'll trace every frame.
             */
            fprintf(stderr, "radeonsi: could not remove thread "
                            "trace trigger file, ignoring\n");
         }
      }

      if (frame_trigger || file_trigger) {
         /* Wait for last submission */
         sctx->ws->fence_wait(sctx->ws, sctx->last_gfx_fence,
                              OS_TIMEOUT_INFINITE);

         /* Start SQTT */
         si_begin_sqtt(sctx, rcs);

         sctx->sqtt_enabled = true;
         sctx->sqtt->start_frame = -1;

         /* Force shader update to make sure si_sqtt_describe_pipeline_bind is
          * called for the current "pipeline".
          */
         sctx->do_update_shaders = true;
      }
   } else {
      struct ac_sqtt_trace sqtt_trace = {0};

      /* Stop SQTT */
      si_end_sqtt(sctx, rcs);
      sctx->sqtt_enabled = false;
      sctx->sqtt->start_frame = -1;
      assert(sctx->last_sqtt_fence);

      /* Wait for SQTT to finish and read back the bo */
      if (sctx->ws->fence_wait(sctx->ws, sctx->last_sqtt_fence,
                               OS_TIMEOUT_INFINITE) &&
          si_get_sqtt_trace(sctx, &sqtt_trace)) {
         struct ac_spm_trace spm_trace;

         /* Map the SPM counter buffer */
         if (sctx->spm.bo) {
            sctx->spm.ptr = sctx->ws->buffer_map(
               sctx->ws, sctx->spm.bo, NULL, PIPE_MAP_READ | RADEON_MAP_TEMPORARY);
            ac_spm_get_trace(&sctx->spm, &spm_trace);
         }

         ac_dump_rgp_capture(&sctx->screen->info, &sqtt_trace,
                             sctx->spm.bo ? &spm_trace : NULL);

         if (sctx->spm.ptr)
            sctx->ws->buffer_unmap(sctx->ws, sctx->spm.bo);
      } else {
         fprintf(stderr, "Failed to read the trace\n");
      }
   }

   num_frames++;
}

static void si_emit_sqtt_userdata(struct si_context *sctx,
                                  struct radeon_cmdbuf *cs, const void *data,
                                  uint32_t num_dwords)
{
   const uint32_t *dwords = (uint32_t *)data;

   radeon_begin(cs);

   while (num_dwords > 0) {
      uint32_t count = MIN2(num_dwords, 2);

      radeon_set_uconfig_perfctr_reg_seq(R_030D08_SQ_THREAD_TRACE_USERDATA_2, count);
      radeon_emit_array(dwords, count);

      dwords += count;
      num_dwords -= count;
   }
   radeon_end();
}

static void
si_emit_spi_config_cntl(struct si_context *sctx,
                        struct radeon_cmdbuf *cs, bool enable)
{
   radeon_begin(cs);

   if (sctx->gfx_level >= GFX9) {
      uint32_t spi_config_cntl = S_031100_GPR_WRITE_PRIORITY(0x2c688) |
                                 S_031100_EXP_PRIORITY_ORDER(3) |
                                 S_031100_ENABLE_SQG_TOP_EVENTS(enable) |
                                 S_031100_ENABLE_SQG_BOP_EVENTS(enable);

      if (sctx->gfx_level >= GFX10)
         spi_config_cntl |= S_031100_PS_PKR_PRIORITY_CNTL(3);

      radeon_set_uconfig_reg(R_031100_SPI_CONFIG_CNTL, spi_config_cntl);
   } else {
      /* SPI_CONFIG_CNTL is a protected register on GFX6-GFX8. */
      radeon_set_privileged_config_reg(R_009100_SPI_CONFIG_CNTL,
                                       S_009100_ENABLE_SQG_TOP_EVENTS(enable) |
                                       S_009100_ENABLE_SQG_BOP_EVENTS(enable));
   }
   radeon_end();
}

static uint32_t num_events = 0;
void si_sqtt_write_event_marker(struct si_context *sctx, struct radeon_cmdbuf *rcs,
                                enum rgp_sqtt_marker_event_type api_type,
                                uint32_t vertex_offset_user_data,
                                uint32_t instance_offset_user_data,
                                uint32_t draw_index_user_data)
{
   struct rgp_sqtt_marker_event marker = {0};

   marker.identifier = RGP_SQTT_MARKER_IDENTIFIER_EVENT;
   marker.api_type = api_type == EventInvalid ? EventCmdDraw : api_type;
   marker.cmd_id = num_events++;
   marker.cb_id = 0;

   if (vertex_offset_user_data == UINT_MAX ||
       instance_offset_user_data == UINT_MAX) {
      vertex_offset_user_data = 0;
      instance_offset_user_data = 0;
   }

   if (draw_index_user_data == UINT_MAX)
      draw_index_user_data = vertex_offset_user_data;

   marker.vertex_offset_reg_idx = vertex_offset_user_data;
   marker.instance_offset_reg_idx = instance_offset_user_data;
   marker.draw_index_reg_idx = draw_index_user_data;

   si_emit_sqtt_userdata(sctx, rcs, &marker, sizeof(marker) / 4);

   sctx->sqtt_next_event = EventInvalid;
}

void si_write_event_with_dims_marker(struct si_context *sctx, struct radeon_cmdbuf *rcs,
                                     enum rgp_sqtt_marker_event_type api_type,
                                     uint32_t x, uint32_t y, uint32_t z)
{
   struct rgp_sqtt_marker_event_with_dims marker = {0};

   marker.event.identifier = RGP_SQTT_MARKER_IDENTIFIER_EVENT;
   marker.event.api_type = api_type;
   marker.event.cmd_id = num_events++;
   marker.event.cb_id = 0;
   marker.event.has_thread_dims = 1;

   marker.thread_x = x;
   marker.thread_y = y;
   marker.thread_z = z;

   si_emit_sqtt_userdata(sctx, rcs, &marker, sizeof(marker) / 4);
   sctx->sqtt_next_event = EventInvalid;
}

void si_sqtt_describe_barrier_start(struct si_context *sctx, struct radeon_cmdbuf *rcs)
{
   struct rgp_sqtt_marker_barrier_start marker = {0};

   marker.identifier = RGP_SQTT_MARKER_IDENTIFIER_BARRIER_START;
   marker.cb_id = 0;
   marker.dword02 = 0xC0000000 + 10; /* RGP_BARRIER_INTERNAL_BASE */

   si_emit_sqtt_userdata(sctx, rcs, &marker, sizeof(marker) / 4);
}

void si_sqtt_describe_barrier_end(struct si_context *sctx, struct radeon_cmdbuf *rcs,
                                  unsigned flags)
{
   struct rgp_sqtt_marker_barrier_end marker = {0};

   marker.identifier = RGP_SQTT_MARKER_IDENTIFIER_BARRIER_END;
   marker.cb_id = 0;

   if (flags & SI_CONTEXT_VS_PARTIAL_FLUSH)
      marker.vs_partial_flush = true;
   if (flags & SI_CONTEXT_PS_PARTIAL_FLUSH)
      marker.ps_partial_flush = true;
   if (flags & SI_CONTEXT_CS_PARTIAL_FLUSH)
      marker.cs_partial_flush = true;

   if (flags & SI_CONTEXT_PFP_SYNC_ME)
      marker.pfp_sync_me = true;

   if (flags & SI_CONTEXT_INV_VCACHE)
      marker.inval_tcp = true;
   if (flags & SI_CONTEXT_INV_ICACHE)
      marker.inval_sqI = true;
   if (flags & SI_CONTEXT_INV_SCACHE)
      marker.inval_sqK = true;
   if (flags & SI_CONTEXT_INV_L2)
      marker.inval_tcc = true;

   if (flags & SI_CONTEXT_FLUSH_AND_INV_CB) {
      marker.inval_cb = true;
      marker.flush_cb = true;
   }
   if (flags & SI_CONTEXT_FLUSH_AND_INV_DB) {
      marker.inval_db = true;
      marker.flush_db = true;
   }

   si_emit_sqtt_userdata(sctx, rcs, &marker, sizeof(marker) / 4);
}

void si_write_user_event(struct si_context *sctx, struct radeon_cmdbuf *rcs,
                         enum rgp_sqtt_marker_user_event_type type,
                         const char *str, int len)
{
   if (type == UserEventPop) {
      assert(str == NULL);
      struct rgp_sqtt_marker_user_event marker = {0};
      marker.identifier = RGP_SQTT_MARKER_IDENTIFIER_USER_EVENT;
      marker.data_type = type;

      si_emit_sqtt_userdata(sctx, rcs, &marker, sizeof(marker) / 4);
   } else {
      assert(str != NULL);
      struct rgp_sqtt_marker_user_event_with_length marker = {0};
      marker.user_event.identifier = RGP_SQTT_MARKER_IDENTIFIER_USER_EVENT;
      marker.user_event.data_type = type;
      len = MIN2(1024, len);
      marker.length = align(len, 4);

      uint8_t *buffer = alloca(sizeof(marker) + marker.length);
      memcpy(buffer, &marker, sizeof(marker));
      memcpy(buffer + sizeof(marker), str, len);
      buffer[sizeof(marker) + len - 1] = '\0';

      si_emit_sqtt_userdata(sctx, rcs, buffer,
                            sizeof(marker) / 4 + marker.length / 4);
   }
}

bool si_sqtt_pipeline_is_registered(struct ac_sqtt *sqtt,
                                    uint64_t pipeline_hash)
{
   simple_mtx_lock(&sqtt->rgp_pso_correlation.lock);
   list_for_each_entry_safe (struct rgp_pso_correlation_record, record,
                             &sqtt->rgp_pso_correlation.record, list) {
      if (record->pipeline_hash[0] == pipeline_hash) {
         simple_mtx_unlock(&sqtt->rgp_pso_correlation.lock);
         return true;
      }
   }
   simple_mtx_unlock(&sqtt->rgp_pso_correlation.lock);

   return false;
}

static enum rgp_hardware_stages
si_sqtt_pipe_to_rgp_shader_stage(union si_shader_key *key, enum pipe_shader_type stage)
{
   switch (stage) {
      case PIPE_SHADER_VERTEX:
         if (key->ge.as_ls)
            return RGP_HW_STAGE_LS;
         else if (key->ge.as_es)
            return RGP_HW_STAGE_ES;
         else if (key->ge.as_ngg)
            return RGP_HW_STAGE_GS;
         else
            return RGP_HW_STAGE_VS;
      case PIPE_SHADER_TESS_CTRL:
         return RGP_HW_STAGE_HS;
      case PIPE_SHADER_TESS_EVAL:
         if (key->ge.as_es)
            return RGP_HW_STAGE_ES;
         else if (key->ge.as_ngg)
            return RGP_HW_STAGE_GS;
         else
            return RGP_HW_STAGE_VS;
      case PIPE_SHADER_GEOMETRY:
         return RGP_HW_STAGE_GS;
      case PIPE_SHADER_FRAGMENT:
         return RGP_HW_STAGE_PS;
      case PIPE_SHADER_COMPUTE:
         return RGP_HW_STAGE_CS;
      default:
         unreachable("invalid mesa shader stage");
   }
}

static bool
si_sqtt_add_code_object(struct si_context *sctx,
                        struct si_sqtt_fake_pipeline *pipeline,
                        bool is_compute)
{
   struct rgp_code_object *code_object = &sctx->sqtt->rgp_code_object;
   struct rgp_code_object_record *record;

   record = calloc(1, sizeof(struct rgp_code_object_record));
   if (!record)
      return false;

   record->shader_stages_mask = 0;
   record->num_shaders_combined = 0;
   record->pipeline_hash[0] = pipeline->code_hash;
   record->pipeline_hash[1] = pipeline->code_hash;

   for (unsigned i = 0; i < PIPE_SHADER_TYPES; i++) {
      struct si_shader *shader;
      enum rgp_hardware_stages hw_stage;

      if (is_compute) {
         if (i != PIPE_SHADER_COMPUTE)
            continue;
         shader = &sctx->cs_shader_state.program->shader;
         hw_stage = RGP_HW_STAGE_CS;
      } else if (i != PIPE_SHADER_COMPUTE) {
         if (!sctx->shaders[i].cso || !sctx->shaders[i].current)
            continue;
         shader = sctx->shaders[i].current;
         hw_stage = si_sqtt_pipe_to_rgp_shader_stage(&shader->key, i);
      } else {
         continue;
      }

      uint8_t *code = malloc(shader->binary.uploaded_code_size);
      if (!code) {
         free(record);
         return false;
      }
      memcpy(code, shader->binary.uploaded_code, shader->binary.uploaded_code_size);

      uint64_t va = pipeline->bo->gpu_address + pipeline->offset[i];
      unsigned gl_shader_stage = tgsi_processor_to_shader_stage(i);
      record->shader_data[gl_shader_stage].hash[0] = _mesa_hash_data(code, shader->binary.uploaded_code_size);
      record->shader_data[gl_shader_stage].hash[1] = record->shader_data[gl_shader_stage].hash[0];
      record->shader_data[gl_shader_stage].code_size = shader->binary.uploaded_code_size;
      record->shader_data[gl_shader_stage].code = code;
      record->shader_data[gl_shader_stage].vgpr_count = shader->config.num_vgprs;
      record->shader_data[gl_shader_stage].sgpr_count = shader->config.num_sgprs;
      record->shader_data[gl_shader_stage].base_address = va & 0xffffffffffff;
      record->shader_data[gl_shader_stage].elf_symbol_offset = 0;
      record->shader_data[gl_shader_stage].hw_stage = hw_stage;
      record->shader_data[gl_shader_stage].is_combined = false;
      record->shader_data[gl_shader_stage].scratch_memory_size = shader->config.scratch_bytes_per_wave;
      record->shader_data[gl_shader_stage].wavefront_size = shader->wave_size;

      record->shader_stages_mask |= 1 << gl_shader_stage;
      record->num_shaders_combined++;
   }

   simple_mtx_lock(&code_object->lock);
   list_addtail(&record->list, &code_object->record);
   code_object->record_count++;
   simple_mtx_unlock(&code_object->lock);

   return true;
}

bool si_sqtt_register_pipeline(struct si_context *sctx, struct si_sqtt_fake_pipeline *pipeline, bool is_compute)
{
   assert(!si_sqtt_pipeline_is_registered(sctx->sqtt, pipeline->code_hash));

   bool result = ac_sqtt_add_pso_correlation(sctx->sqtt, pipeline->code_hash, pipeline->code_hash);
   if (!result)
      return false;

   result = ac_sqtt_add_code_object_loader_event(
      sctx->sqtt, pipeline->code_hash, pipeline->bo->gpu_address);
   if (!result)
      return false;

   return si_sqtt_add_code_object(sctx, pipeline, is_compute);
}

void si_sqtt_describe_pipeline_bind(struct si_context *sctx,
                                    uint64_t pipeline_hash,
                                    int bind_point)
{
   struct rgp_sqtt_marker_pipeline_bind marker = {0};
   struct radeon_cmdbuf *cs = &sctx->gfx_cs;

   if (likely(!sctx->sqtt_enabled)) {
      return;
   }

   marker.identifier = RGP_SQTT_MARKER_IDENTIFIER_BIND_PIPELINE;
   marker.cb_id = 0;
   marker.bind_point = bind_point;
   marker.api_pso_hash[0] = pipeline_hash;
   marker.api_pso_hash[1] = pipeline_hash >> 32;

   si_emit_sqtt_userdata(sctx, cs, &marker, sizeof(marker) / 4);
}
