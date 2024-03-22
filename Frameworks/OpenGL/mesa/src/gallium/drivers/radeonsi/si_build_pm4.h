/*
 * Copyright 2013 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * This file contains helpers for writing commands to commands streams.
 */

#ifndef SI_BUILD_PM4_H
#define SI_BUILD_PM4_H

#include "si_pipe.h"
#include "sid.h"

#define radeon_begin(cs) struct radeon_cmdbuf *__cs = (cs); \
                         unsigned __cs_num = __cs->current.cdw; \
                         UNUSED unsigned __cs_num_initial = __cs_num; \
                         uint32_t *__cs_buf = __cs->current.buf

#define radeon_begin_again(cs) do { \
   assert(__cs == NULL); \
   __cs = (cs); \
   __cs_num = __cs->current.cdw; \
   __cs_num_initial = __cs_num; \
   __cs_buf = __cs->current.buf; \
} while (0)

#define radeon_end() do { \
   __cs->current.cdw = __cs_num; \
   assert(__cs->current.cdw <= __cs->current.max_dw); \
   __cs = NULL; \
} while (0)

#define radeon_emit(value)  __cs_buf[__cs_num++] = (value)
#define radeon_packets_added()  (__cs_num != __cs_num_initial)

#define radeon_end_update_context_roll(_unused) do { \
   radeon_end(); \
   if (radeon_packets_added()) \
      sctx->context_roll = true; \
} while (0)

#define radeon_emit_array(values, num) do { \
   unsigned __n = (num); \
   memcpy(__cs_buf + __cs_num, (values), __n * 4); \
   __cs_num += __n; \
} while (0)

/* Instead of writing into the command buffer, return the pointer to the command buffer and
 * assume that the caller will fill the specified number of elements.
 */
#define radeon_emit_array_get_ptr(num, ptr) do { \
   *(ptr) = __cs_buf + __cs_num; \
   __cs_num += (num); \
} while (0)

/* Packet building helpers. Don't use directly. */
#define radeon_set_reg_seq(reg, num, idx, prefix_name, packet, reset_filter_cam) do { \
   assert((reg) >= prefix_name##_REG_OFFSET && (reg) < prefix_name##_REG_END); \
   radeon_emit(PKT3(packet, num, 0) | PKT3_RESET_FILTER_CAM_S(reset_filter_cam)); \
   radeon_emit((((reg) - prefix_name##_REG_OFFSET) >> 2) | ((idx) << 28)); \
} while (0)

#define radeon_set_reg(reg, idx, value, prefix_name, packet) do { \
   radeon_set_reg_seq(reg, 1, idx, prefix_name, packet, 0); \
   radeon_emit(value); \
} while (0)

#define radeon_opt_set_reg(reg, reg_enum, idx, value, prefix_name, packet) do { \
   unsigned __value = (value); \
   if (!BITSET_TEST(sctx->tracked_regs.reg_saved_mask, (reg_enum)) || \
       sctx->tracked_regs.reg_value[(reg_enum)] != __value) { \
      radeon_set_reg(reg, idx, __value, prefix_name, packet); \
      BITSET_SET(sctx->tracked_regs.reg_saved_mask, (reg_enum)); \
      sctx->tracked_regs.reg_value[(reg_enum)] = __value; \
   } \
} while (0)

/* Set consecutive registers if any value is different. */
#define radeon_opt_set_reg2(reg, reg_enum, v1, v2, prefix_name, packet) do { \
   unsigned __v1 = (v1), __v2 = (v2); \
   if (!BITSET_TEST_RANGE_INSIDE_WORD(sctx->tracked_regs.reg_saved_mask, \
                                      (reg_enum), (reg_enum) + 1, 0x3) || \
       sctx->tracked_regs.reg_value[(reg_enum)] != __v1 || \
       sctx->tracked_regs.reg_value[(reg_enum) + 1] != __v2) { \
      radeon_set_reg_seq(reg, 2, 0, prefix_name, packet, 0); \
      radeon_emit(__v1); \
      radeon_emit(__v2); \
      BITSET_SET_RANGE_INSIDE_WORD(sctx->tracked_regs.reg_saved_mask, \
                                   (reg_enum), (reg_enum) + 1); \
      sctx->tracked_regs.reg_value[(reg_enum)] = __v1; \
      sctx->tracked_regs.reg_value[(reg_enum) + 1] = __v2; \
   } \
} while (0)

#define radeon_opt_set_reg3(reg, reg_enum, v1, v2, v3, prefix_name, packet) do { \
   unsigned __v1 = (v1), __v2 = (v2), __v3 = (v3); \
   if (!BITSET_TEST_RANGE_INSIDE_WORD(sctx->tracked_regs.reg_saved_mask, \
                                      (reg_enum), (reg_enum) + 2, 0x7) || \
       sctx->tracked_regs.reg_value[(reg_enum)] != __v1 || \
       sctx->tracked_regs.reg_value[(reg_enum) + 1] != __v2 || \
       sctx->tracked_regs.reg_value[(reg_enum) + 2] != __v3) { \
      radeon_set_reg_seq(reg, 3, 0, prefix_name, packet, 0); \
      radeon_emit(__v1); \
      radeon_emit(__v2); \
      radeon_emit(__v3); \
      BITSET_SET_RANGE_INSIDE_WORD(sctx->tracked_regs.reg_saved_mask, \
                                   (reg_enum), (reg_enum) + 2); \
      sctx->tracked_regs.reg_value[(reg_enum)] = __v1; \
      sctx->tracked_regs.reg_value[(reg_enum) + 1] = __v2; \
      sctx->tracked_regs.reg_value[(reg_enum) + 2] = __v3; \
   } \
} while (0)

#define radeon_opt_set_reg4(reg, reg_enum, v1, v2, v3, v4, prefix_name, packet) do { \
   unsigned __v1 = (v1), __v2 = (v2), __v3 = (v3), __v4 = (v4); \
   if (!BITSET_TEST_RANGE_INSIDE_WORD(sctx->tracked_regs.reg_saved_mask, \
                                      (reg_enum), (reg_enum) + 3, 0xf) || \
       sctx->tracked_regs.reg_value[(reg_enum)] != __v1 || \
       sctx->tracked_regs.reg_value[(reg_enum) + 1] != __v2 || \
       sctx->tracked_regs.reg_value[(reg_enum) + 2] != __v3 || \
       sctx->tracked_regs.reg_value[(reg_enum) + 3] != __v4) { \
      radeon_set_reg_seq(reg, 4, 0, prefix_name, packet, 0); \
      radeon_emit(__v1); \
      radeon_emit(__v2); \
      radeon_emit(__v3); \
      radeon_emit(__v4); \
      BITSET_SET_RANGE_INSIDE_WORD(sctx->tracked_regs.reg_saved_mask, \
                                   (reg_enum), (reg_enum) + 3); \
      sctx->tracked_regs.reg_value[(reg_enum)] = __v1; \
      sctx->tracked_regs.reg_value[(reg_enum) + 1] = __v2; \
      sctx->tracked_regs.reg_value[(reg_enum) + 2] = __v3; \
      sctx->tracked_regs.reg_value[(reg_enum) + 3] = __v4; \
   } \
} while (0)

#define radeon_opt_set_reg5(reg, reg_enum, v1, v2, v3, v4, v5, prefix_name, packet) do { \
   unsigned __v1 = (v1), __v2 = (v2), __v3 = (v3), __v4 = (v4), __v5 = (v5); \
   if (!BITSET_TEST_RANGE_INSIDE_WORD(sctx->tracked_regs.reg_saved_mask, \
                                      (reg_enum), (reg_enum) + 4, 0x1f) || \
       sctx->tracked_regs.reg_value[(reg_enum)] != __v1 || \
       sctx->tracked_regs.reg_value[(reg_enum) + 1] != __v2 || \
       sctx->tracked_regs.reg_value[(reg_enum) + 2] != __v3 || \
       sctx->tracked_regs.reg_value[(reg_enum) + 3] != __v4 || \
       sctx->tracked_regs.reg_value[(reg_enum) + 4] != __v5) { \
      radeon_set_reg_seq(reg, 5, 0, prefix_name, packet, 0); \
      radeon_emit(__v1); \
      radeon_emit(__v2); \
      radeon_emit(__v3); \
      radeon_emit(__v4); \
      radeon_emit(__v5); \
      BITSET_SET_RANGE_INSIDE_WORD(sctx->tracked_regs.reg_saved_mask, \
                                   (reg_enum), (reg_enum) + 4); \
      sctx->tracked_regs.reg_value[(reg_enum)] = __v1; \
      sctx->tracked_regs.reg_value[(reg_enum) + 1] = __v2; \
      sctx->tracked_regs.reg_value[(reg_enum) + 2] = __v3; \
      sctx->tracked_regs.reg_value[(reg_enum) + 3] = __v4; \
      sctx->tracked_regs.reg_value[(reg_enum) + 4] = __v5; \
   } \
} while (0)

#define radeon_opt_set_reg6(reg, reg_enum, v1, v2, v3, v4, v5, v6, prefix_name, packet) do { \
   unsigned __v1 = (v1), __v2 = (v2), __v3 = (v3), __v4 = (v4), __v5 = (v5), __v6 = (v6); \
   if (!BITSET_TEST_RANGE_INSIDE_WORD(sctx->tracked_regs.reg_saved_mask, \
                                      (reg_enum), (reg_enum) + 5, 0x3f) || \
       sctx->tracked_regs.reg_value[(reg_enum)] != __v1 || \
       sctx->tracked_regs.reg_value[(reg_enum) + 1] != __v2 || \
       sctx->tracked_regs.reg_value[(reg_enum) + 2] != __v3 || \
       sctx->tracked_regs.reg_value[(reg_enum) + 3] != __v4 || \
       sctx->tracked_regs.reg_value[(reg_enum) + 4] != __v5 || \
       sctx->tracked_regs.reg_value[(reg_enum) + 5] != __v6) { \
      radeon_set_reg_seq(reg, 6, 0, prefix_name, packet, 0); \
      radeon_emit(__v1); \
      radeon_emit(__v2); \
      radeon_emit(__v3); \
      radeon_emit(__v4); \
      radeon_emit(__v5); \
      radeon_emit(__v6); \
      BITSET_SET_RANGE_INSIDE_WORD(sctx->tracked_regs.reg_saved_mask, \
                                   (reg_enum), (reg_enum) + 5); \
      sctx->tracked_regs.reg_value[(reg_enum)] = __v1; \
      sctx->tracked_regs.reg_value[(reg_enum) + 1] = __v2; \
      sctx->tracked_regs.reg_value[(reg_enum) + 2] = __v3; \
      sctx->tracked_regs.reg_value[(reg_enum) + 3] = __v4; \
      sctx->tracked_regs.reg_value[(reg_enum) + 4] = __v5; \
      sctx->tracked_regs.reg_value[(reg_enum) + 5] = __v6; \
   } \
} while (0)

#define radeon_opt_set_regn(reg, values, saved_values, num, prefix_name, packet) do { \
   if (memcmp(values, saved_values, sizeof(uint32_t) * (num))) { \
      radeon_set_reg_seq(reg, num, 0, prefix_name, packet, 0); \
      radeon_emit_array(values, num); \
      memcpy(saved_values, values, sizeof(uint32_t) * (num)); \
   } \
} while (0)

/* Packet building helpers for CONFIG registers. */
#define radeon_set_config_reg(reg, value) \
   radeon_set_reg(reg, 0, value, SI_CONFIG, PKT3_SET_CONFIG_REG)

/* Packet building helpers for CONTEXT registers. */
/* TODO: Remove the _unused parameters everywhere. */
#define radeon_set_context_reg_seq(reg, num) \
   radeon_set_reg_seq(reg, num, 0, SI_CONTEXT, PKT3_SET_CONTEXT_REG, 0)

#define radeon_set_context_reg(reg, value) \
   radeon_set_reg(reg, 0, value, SI_CONTEXT, PKT3_SET_CONTEXT_REG)

#define radeon_opt_set_context_reg(_unused, reg, reg_enum, value) \
   radeon_opt_set_reg(reg, reg_enum, 0, value, SI_CONTEXT, PKT3_SET_CONTEXT_REG)

#define radeon_opt_set_context_reg_idx(_unused, reg, reg_enum, idx, value) \
   radeon_opt_set_reg(reg, reg_enum, idx, value, SI_CONTEXT, PKT3_SET_CONTEXT_REG)

#define radeon_opt_set_context_reg2(_unused, reg, reg_enum, v1, v2) \
   radeon_opt_set_reg2(reg, reg_enum, v1, v2, SI_CONTEXT, PKT3_SET_CONTEXT_REG)

#define radeon_opt_set_context_reg3(_unused, reg, reg_enum, v1, v2, v3) \
   radeon_opt_set_reg3(reg, reg_enum, v1, v2, v3, SI_CONTEXT, PKT3_SET_CONTEXT_REG)

#define radeon_opt_set_context_reg4(_unused, reg, reg_enum, v1, v2, v3, v4) \
   radeon_opt_set_reg4(reg, reg_enum, v1, v2, v3, v4, SI_CONTEXT, PKT3_SET_CONTEXT_REG)

#define radeon_opt_set_context_reg5(_unused, reg, reg_enum, v1, v2, v3, v4, v5) \
   radeon_opt_set_reg5(reg, reg_enum, v1, v2, v3, v4, v5, SI_CONTEXT, PKT3_SET_CONTEXT_REG)

#define radeon_opt_set_context_reg6(reg, reg_enum, v1, v2, v3, v4, v5, v6) \
   radeon_opt_set_reg6(reg, reg_enum, v1, v2, v3, v4, v5, v6, SI_CONTEXT, PKT3_SET_CONTEXT_REG)

#define radeon_opt_set_context_regn(_unused, reg, values, saved_values, num) \
   radeon_opt_set_regn(reg, values, saved_values, num, SI_CONTEXT, PKT3_SET_CONTEXT_REG)

/* Packet building helpers for SH registers. */
#define radeon_set_sh_reg_seq(reg, num) \
   radeon_set_reg_seq(reg, num, 0, SI_SH, PKT3_SET_SH_REG, 0)

#define radeon_set_sh_reg(reg, value) \
   radeon_set_reg(reg, 0, value, SI_SH, PKT3_SET_SH_REG)

#define radeon_opt_set_sh_reg(_unused, reg, reg_enum, value) \
   radeon_opt_set_reg(reg, reg_enum, 0, value, SI_SH, PKT3_SET_SH_REG)

#define radeon_opt_set_sh_reg2(_unused, reg, reg_enum, v1, v2) \
   radeon_opt_set_reg2(reg, reg_enum, v1, v2, SI_SH, PKT3_SET_SH_REG)

#define radeon_opt_set_sh_reg3(_unused, reg, reg_enum, v1, v2, v3) \
   radeon_opt_set_reg3(reg, reg_enum, v1, v2, v3, SI_SH, PKT3_SET_SH_REG)

#define radeon_opt_set_sh_reg_idx(_unused, reg, reg_enum, idx, value) do { \
   assert(sctx->gfx_level >= GFX10); \
   radeon_opt_set_reg(reg, reg_enum, idx, value, SI_SH, PKT3_SET_SH_REG_INDEX); \
} while (0)

#define radeon_emit_32bit_pointer(_unused, va) do { \
   assert((va) == 0 || ((va) >> 32) == sctx->screen->info.address32_hi); \
   radeon_emit(va); \
} while (0)

#define radeon_emit_one_32bit_pointer(_unused, desc, sh_base) do { \
   radeon_set_sh_reg_seq((sh_base) + (desc)->shader_userdata_offset, 1); \
   radeon_emit_32bit_pointer(_unused, (desc)->gpu_address); \
} while (0)

/* Packet building helpers for UCONFIG registers. */
#define radeon_set_uconfig_reg_seq(reg, num) \
   radeon_set_reg_seq(reg, num, 0, CIK_UCONFIG, PKT3_SET_UCONFIG_REG, 0)

#define radeon_set_uconfig_perfctr_reg_seq(reg, num) \
   radeon_set_reg_seq(reg, num, 0, CIK_UCONFIG, PKT3_SET_UCONFIG_REG, \
                      sctx->gfx_level >= GFX10 && \
                      sctx->ws->cs_get_ip_type(__cs) == AMD_IP_GFX)

#define radeon_set_uconfig_reg(reg, value) \
   radeon_set_reg(reg, 0, value, CIK_UCONFIG, PKT3_SET_UCONFIG_REG)

#define radeon_opt_set_uconfig_reg(_unused, reg, reg_enum, value) \
   radeon_opt_set_reg(reg, reg_enum, 0, value, CIK_UCONFIG, PKT3_SET_UCONFIG_REG)

#define RESOLVE_PKT3_SET_UCONFIG_REG_INDEX \
   (GFX_VERSION >= GFX10 || (GFX_VERSION == GFX9 && sctx->screen->info.me_fw_version >= 26) ? \
    PKT3_SET_UCONFIG_REG_INDEX : PKT3_SET_UCONFIG_REG)

#define radeon_set_uconfig_reg_idx(_unused, _unused2, reg, idx, value) \
   radeon_set_reg(reg, idx, value, CIK_UCONFIG, RESOLVE_PKT3_SET_UCONFIG_REG_INDEX)

#define radeon_opt_set_uconfig_reg_idx(_unused, _unused2, reg, reg_enum, idx, value) \
   radeon_opt_set_reg(reg, reg_enum, idx, value, CIK_UCONFIG, RESOLVE_PKT3_SET_UCONFIG_REG_INDEX)

#define radeon_set_privileged_config_reg(reg, value) do { \
   assert((reg) < CIK_UCONFIG_REG_OFFSET); \
   radeon_emit(PKT3(PKT3_COPY_DATA, 4, 0)); \
   radeon_emit(COPY_DATA_SRC_SEL(COPY_DATA_IMM) | \
               COPY_DATA_DST_SEL(COPY_DATA_PERF)); \
   radeon_emit(value); \
   radeon_emit(0); /* unused */ \
   radeon_emit((reg) >> 2); \
   radeon_emit(0); /* unused */ \
} while (0)

/* GFX11 generic packet building helpers for buffered SH registers. Don't use these directly. */
#define gfx11_push_reg(reg, value, prefix_name, buffer, reg_count) do { \
   unsigned __i = (reg_count)++; \
   assert((reg) >= prefix_name##_REG_OFFSET && (reg) < prefix_name##_REG_END); \
   assert(__i / 2 < ARRAY_SIZE(buffer)); \
   buffer[__i / 2].reg_offset[__i % 2] = ((reg) - prefix_name##_REG_OFFSET) >> 2; \
   buffer[__i / 2].reg_value[__i % 2] = value; \
} while (0)

#define gfx11_opt_push_reg(reg, reg_enum, value, prefix_name, buffer, reg_count) do { \
   unsigned __value = value; \
   if (!BITSET_TEST(sctx->tracked_regs.reg_saved_mask, (reg_enum)) || \
       sctx->tracked_regs.reg_value[reg_enum] != __value) { \
      gfx11_push_reg(reg, __value, prefix_name, buffer, reg_count); \
      BITSET_SET(sctx->tracked_regs.reg_saved_mask, (reg_enum)); \
      sctx->tracked_regs.reg_value[reg_enum] = __value; \
   } \
} while (0)

#define gfx11_opt_push_reg4(reg, reg_enum, v1, v2, v3, v4, prefix_name, buffer, reg_count) do { \
   unsigned __v1 = (v1); \
   unsigned __v2 = (v2); \
   unsigned __v3 = (v3); \
   unsigned __v4 = (v4); \
   if (!BITSET_TEST_RANGE_INSIDE_WORD(sctx->tracked_regs.reg_saved_mask, \
                                      (reg_enum), (reg_enum) + 3, 0xf) || \
       sctx->tracked_regs.reg_value[(reg_enum)] != __v1 || \
       sctx->tracked_regs.reg_value[(reg_enum) + 1] != __v2 || \
       sctx->tracked_regs.reg_value[(reg_enum) + 2] != __v3 || \
       sctx->tracked_regs.reg_value[(reg_enum) + 3] != __v4) { \
      gfx11_push_reg((reg), __v1, prefix_name, buffer, reg_count); \
      gfx11_push_reg((reg) + 4, __v2, prefix_name, buffer, reg_count); \
      gfx11_push_reg((reg) + 8, __v3, prefix_name, buffer, reg_count); \
      gfx11_push_reg((reg) + 12, __v4, prefix_name, buffer, reg_count); \
      BITSET_SET_RANGE_INSIDE_WORD(sctx->tracked_regs.reg_saved_mask, \
                                   (reg_enum), (reg_enum) + 3); \
      sctx->tracked_regs.reg_value[(reg_enum)] = __v1; \
      sctx->tracked_regs.reg_value[(reg_enum) + 1] = __v2; \
      sctx->tracked_regs.reg_value[(reg_enum) + 2] = __v3; \
      sctx->tracked_regs.reg_value[(reg_enum) + 3] = __v4; \
   } \
} while (0)

/* GFX11 packet building helpers for buffered SH registers. */
#define gfx11_push_gfx_sh_reg(reg, value) \
   gfx11_push_reg(reg, value, SI_SH, sctx->gfx11.buffered_gfx_sh_regs, \
                  sctx->num_buffered_gfx_sh_regs)

#define gfx11_push_compute_sh_reg(reg, value) \
   gfx11_push_reg(reg, value, SI_SH, sctx->gfx11.buffered_compute_sh_regs, \
                  sctx->num_buffered_compute_sh_regs)

#define gfx11_opt_push_gfx_sh_reg(reg, reg_enum, value) \
   gfx11_opt_push_reg(reg, reg_enum, value, SI_SH, sctx->gfx11.buffered_gfx_sh_regs, \
                      sctx->num_buffered_gfx_sh_regs)

#define gfx11_opt_push_compute_sh_reg(reg, reg_enum, value) \
   gfx11_opt_push_reg(reg, reg_enum, value, SI_SH, sctx->gfx11.buffered_compute_sh_regs, \
                      sctx->num_buffered_compute_sh_regs)

/* GFX11 packet building helpers for SET_CONTEXT_REG_PAIRS_PACKED.
 * Registers are buffered on the stack and then copied to the command buffer at the end.
 */
#define gfx11_begin_packed_context_regs() \
   struct gfx11_reg_pair __cs_context_regs[50]; \
   unsigned __cs_context_reg_count = 0;

#define gfx11_set_context_reg(reg, value) \
   gfx11_push_reg(reg, value, SI_CONTEXT, __cs_context_regs, __cs_context_reg_count)

#define gfx11_opt_set_context_reg(reg, reg_enum, value) \
   gfx11_opt_push_reg(reg, reg_enum, value, SI_CONTEXT, __cs_context_regs, \
                      __cs_context_reg_count)

#define gfx11_opt_set_context_reg4(reg, reg_enum, v1, v2, v3, v4) \
   gfx11_opt_push_reg4(reg, reg_enum, v1, v2, v3, v4, SI_CONTEXT, __cs_context_regs, \
                       __cs_context_reg_count)

#define gfx11_end_packed_context_regs() do { \
   if (__cs_context_reg_count >= 2) { \
      /* Align the count to 2 by duplicating the first register. */ \
      if (__cs_context_reg_count % 2 == 1) { \
         gfx11_set_context_reg(SI_CONTEXT_REG_OFFSET + __cs_context_regs[0].reg_offset[0] * 4, \
                               __cs_context_regs[0].reg_value[0]); \
      } \
      assert(__cs_context_reg_count % 2 == 0); \
      unsigned __num_dw = (__cs_context_reg_count / 2) * 3; \
      radeon_emit(PKT3(PKT3_SET_CONTEXT_REG_PAIRS_PACKED, __num_dw, 0) | PKT3_RESET_FILTER_CAM_S(1)); \
      radeon_emit(__cs_context_reg_count); \
      radeon_emit_array(__cs_context_regs, __num_dw); \
   } else if (__cs_context_reg_count == 1) { \
      radeon_emit(PKT3(PKT3_SET_CONTEXT_REG, 1, 0)); \
      radeon_emit(__cs_context_regs[0].reg_offset[0]); \
      radeon_emit(__cs_context_regs[0].reg_value[0]); \
   } \
} while (0)

#define radeon_set_or_push_gfx_sh_reg(reg, value) do { \
   if (GFX_VERSION >= GFX11 && HAS_SH_PAIRS_PACKED) { \
      gfx11_push_gfx_sh_reg(reg, value); \
   } else { \
      radeon_set_sh_reg_seq(reg, 1); \
      radeon_emit(value); \
   } \
} while (0)

/* This should be evaluated at compile time if all parameters are constants. */
static ALWAYS_INLINE unsigned
si_get_user_data_base(enum amd_gfx_level gfx_level, enum si_has_tess has_tess,
                      enum si_has_gs has_gs, enum si_has_ngg ngg,
                      enum pipe_shader_type shader)
{
   switch (shader) {
   case PIPE_SHADER_VERTEX:
      /* VS can be bound as VS, ES, LS, or GS. */
      if (has_tess) {
         if (gfx_level >= GFX10) {
            return R_00B430_SPI_SHADER_USER_DATA_HS_0;
         } else if (gfx_level == GFX9) {
            return R_00B430_SPI_SHADER_USER_DATA_LS_0;
         } else {
            return R_00B530_SPI_SHADER_USER_DATA_LS_0;
         }
      } else if (gfx_level >= GFX10) {
         if (ngg || has_gs) {
            return R_00B230_SPI_SHADER_USER_DATA_GS_0;
         } else {
            return R_00B130_SPI_SHADER_USER_DATA_VS_0;
         }
      } else if (has_gs) {
         return R_00B330_SPI_SHADER_USER_DATA_ES_0;
      } else {
         return R_00B130_SPI_SHADER_USER_DATA_VS_0;
      }

   case PIPE_SHADER_TESS_CTRL:
      if (gfx_level == GFX9) {
         return R_00B430_SPI_SHADER_USER_DATA_LS_0;
      } else {
         return R_00B430_SPI_SHADER_USER_DATA_HS_0;
      }

   case PIPE_SHADER_TESS_EVAL:
      /* TES can be bound as ES, VS, or not bound. */
      if (has_tess) {
         if (gfx_level >= GFX10) {
            if (ngg || has_gs) {
               return R_00B230_SPI_SHADER_USER_DATA_GS_0;
            } else {
               return R_00B130_SPI_SHADER_USER_DATA_VS_0;
            }
         } else if (has_gs) {
            return R_00B330_SPI_SHADER_USER_DATA_ES_0;
         } else {
            return R_00B130_SPI_SHADER_USER_DATA_VS_0;
         }
      } else {
         return 0;
      }

   case PIPE_SHADER_GEOMETRY:
      if (gfx_level == GFX9) {
         return R_00B330_SPI_SHADER_USER_DATA_ES_0;
      } else {
         return R_00B230_SPI_SHADER_USER_DATA_GS_0;
      }

   default:
      assert(0);
      return 0;
   }
}

#endif
