/*
 * Copyright 2023 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

/* Utility for gathering context rolls for performance bottleneck analysis.
 *
 * Usage for radeonsi:
 *    AMD_ROLLS=filename app1
 *    AMD_ROLLS=filename app2
 *    ...
 *    AMD_ROLLS=filename appN
 *
 *    sort filename | uniq -c | sort -n > rolls_sorted.txt
 *
 *    Then try to reduce the most frequent context rolls.
 */

#include "ac_debug.h"
#include "sid.h"
#include "sid_tables.h"

#include "util/bitset.h"
#include "util/u_dynarray.h"
#include "util/u_memory.h"

struct ac_context_reg_deltas {
   uint32_t changed_masks[1024];    /* changes masks of context registers */
   BITSET_DECLARE(changed, 1024);   /* which context register was set */
   bool acquire_mem;                /* whether ACQUIRE_MEM rolled the context */
};

struct ac_context_reg_state {
   uint32_t regs[1024];
   struct ac_context_reg_deltas deltas;
};

struct ac_context_roll_ctx {
   struct ac_context_reg_state *cur;
   bool context_busy;

   unsigned num_busy_contexts;
   struct util_dynarray rolls;

   const struct radeon_info *info;
};

static void ac_roll_context(struct ac_context_roll_ctx *ctx)
{
   if (!ctx->context_busy)
      return;

   struct ac_context_reg_state *last = ctx->cur;
   ctx->cur = CALLOC_STRUCT(ac_context_reg_state);
   memcpy(ctx->cur->regs, last->regs, sizeof(last->regs));
   ctx->context_busy = false;
   ctx->num_busy_contexts++;

   /* Ignore the first context at the beginning or after waiting for idle. */
   if (ctx->num_busy_contexts > 1) {
      util_dynarray_append(&ctx->rolls, struct ac_context_reg_state *, last);
   } else {
      FREE(last);
   }
}

static void ac_record_wait_idle(struct ac_context_roll_ctx *ctx)
{
   ctx->num_busy_contexts = 0;
   ctx->context_busy = false;
   memset(&ctx->cur->deltas, 0, sizeof(ctx->cur->deltas));
}

static void ac_record_set_context_reg(struct ac_context_roll_ctx *ctx,
                                      unsigned reg_rel_dw_offset, unsigned value)
{
   if (!ac_register_exists(ctx->info->gfx_level, ctx->info->family,
                           SI_CONTEXT_REG_OFFSET + reg_rel_dw_offset * 4)) {
      fprintf(stderr, "This register is not supported by this chip: 0x%X\n",
              SI_CONTEXT_REG_OFFSET + reg_rel_dw_offset * 4);
      abort();
   }

   assert(reg_rel_dw_offset < 1024);
   BITSET_SET(ctx->cur->deltas.changed, reg_rel_dw_offset);
   ctx->cur->deltas.changed_masks[reg_rel_dw_offset] |= ctx->cur->regs[reg_rel_dw_offset] ^ value;
   ctx->cur->regs[reg_rel_dw_offset] = value;
}

static unsigned get_reg_index(unsigned reg)
{
   return (reg - SI_CONTEXT_REG_OFFSET) / 4;
}

static void ac_ib_gather_context_rolls(struct ac_context_roll_ctx *ctx, uint32_t *ib, int num_dw)
{
   for (unsigned cur_dw = 0; cur_dw < num_dw;) {
      uint32_t header = ib[cur_dw++];
      unsigned type = PKT_TYPE_G(header);

      if (type != 3) {
         fprintf(stderr, "Unexpected type %u packet\n", type);
         abort();
      }

      int count = PKT_COUNT_G(header);
      unsigned op = PKT3_IT_OPCODE_G(header);

      switch (op) {
      /* Record context register changes. */
      case PKT3_SET_CONTEXT_REG: {
         ac_roll_context(ctx);

         unsigned reg_dw = ib[cur_dw++];
         unsigned reg_rel_dw_offset = reg_dw & 0xFFFF;

         for (int i = 0; i < count; i++)
            ac_record_set_context_reg(ctx, reg_rel_dw_offset + i, ib[cur_dw++]);
         continue;
      }

      case PKT3_SET_CONTEXT_REG_PAIRS:
         ac_roll_context(ctx);

         for (int i = 0; i < (count + 1) / 2; i++) {
            unsigned reg_rel_dw_offset = ib[cur_dw++];
            ac_record_set_context_reg(ctx, reg_rel_dw_offset, ib[cur_dw++]);
         }
         continue;

      case PKT3_SET_CONTEXT_REG_PAIRS_PACKED: {
         ac_roll_context(ctx);

         unsigned reg_rel_dw_offset0 = 0, reg_rel_dw_offset1 = 0;
         cur_dw++;

         for (int i = 0; i < count; i++) {
            if (i % 3 == 0) {
               unsigned tmp = ib[cur_dw++];
               reg_rel_dw_offset0 = tmp & 0xffff;
               reg_rel_dw_offset1 = tmp >> 16;
            } else if (i % 3 == 1) {
               ac_record_set_context_reg(ctx, reg_rel_dw_offset0, ib[cur_dw++]);
            } else {
               ac_record_set_context_reg(ctx, reg_rel_dw_offset1, ib[cur_dw++]);
            }
         }
         continue;
      }

      case PKT3_CLEAR_STATE:
         ac_roll_context(ctx);

         ac_record_set_context_reg(ctx, get_reg_index(R_028000_DB_RENDER_CONTROL), 0);
         ac_record_set_context_reg(ctx, get_reg_index(R_028004_DB_COUNT_CONTROL), 0);

         ac_record_set_context_reg(ctx, get_reg_index(R_028BDC_PA_SC_LINE_CNTL), 0x1000);
         ac_record_set_context_reg(ctx, get_reg_index(R_028BE0_PA_SC_AA_CONFIG), 0);

         ac_record_set_context_reg(ctx, get_reg_index(R_028BE4_PA_SU_VTX_CNTL), 0x5);
         ac_record_set_context_reg(ctx, get_reg_index(R_028BE8_PA_CL_GB_VERT_CLIP_ADJ), 0x3f800000);
         ac_record_set_context_reg(ctx, get_reg_index(R_028BEC_PA_CL_GB_VERT_DISC_ADJ), 0x3f800000);
         ac_record_set_context_reg(ctx, get_reg_index(R_028BF0_PA_CL_GB_HORZ_CLIP_ADJ), 0x3f800000);
         ac_record_set_context_reg(ctx, get_reg_index(R_028BF4_PA_CL_GB_HORZ_DISC_ADJ), 0x3f800000);

         ac_record_set_context_reg(ctx, get_reg_index(R_02870C_SPI_SHADER_POS_FORMAT), 0);

         ac_record_set_context_reg(ctx, get_reg_index(R_028710_SPI_SHADER_Z_FORMAT), 0);
         ac_record_set_context_reg(ctx, get_reg_index(R_028714_SPI_SHADER_COL_FORMAT), 0);
         ac_record_set_context_reg(ctx, get_reg_index(R_0286E0_SPI_BARYC_CNTL), 0);
         ac_record_set_context_reg(ctx, get_reg_index(R_0286CC_SPI_PS_INPUT_ENA), 0);
         ac_record_set_context_reg(ctx, get_reg_index(R_0286D0_SPI_PS_INPUT_ADDR), 0);

         ac_record_set_context_reg(ctx, get_reg_index(R_028804_DB_EQAA), 0);
         ac_record_set_context_reg(ctx, get_reg_index(R_02880C_DB_SHADER_CONTROL), 0);
         ac_record_set_context_reg(ctx, get_reg_index(R_02823C_CB_SHADER_MASK), 0xffffffff);
         ac_record_set_context_reg(ctx, get_reg_index(R_028238_CB_TARGET_MASK), 0xffffffff);
         ac_record_set_context_reg(ctx, get_reg_index(R_028810_PA_CL_CLIP_CNTL), 0x90000);
         ac_record_set_context_reg(ctx, get_reg_index(R_02881C_PA_CL_VS_OUT_CNTL), 0);
         ac_record_set_context_reg(ctx, get_reg_index(R_028818_PA_CL_VTE_CNTL), 0);
         ac_record_set_context_reg(ctx, get_reg_index(R_02820C_PA_SC_CLIPRECT_RULE), 0xffff);
         ac_record_set_context_reg(ctx, get_reg_index(R_028A0C_PA_SC_LINE_STIPPLE), 0);
         ac_record_set_context_reg(ctx, get_reg_index(R_028A4C_PA_SC_MODE_CNTL_1), 0);
         ac_record_set_context_reg(ctx, get_reg_index(R_028234_PA_SU_HARDWARE_SCREEN_OFFSET), 0);
         ac_record_set_context_reg(ctx, get_reg_index(R_0286D8_SPI_PS_IN_CONTROL), 0x2);
         ac_record_set_context_reg(ctx, get_reg_index(R_028B90_VGT_GS_INSTANCE_CNT), 0);
         ac_record_set_context_reg(ctx, get_reg_index(R_028B38_VGT_GS_MAX_VERT_OUT), 0);
         ac_record_set_context_reg(ctx, get_reg_index(R_028B54_VGT_SHADER_STAGES_EN), 0);
         ac_record_set_context_reg(ctx, get_reg_index(R_028B58_VGT_LS_HS_CONFIG), 0);
         ac_record_set_context_reg(ctx, get_reg_index(R_028B6C_VGT_TF_PARAM), 0);
         ac_record_set_context_reg(ctx, get_reg_index(R_028830_PA_SU_SMALL_PRIM_FILTER_CNTL), 0);
         ac_record_set_context_reg(ctx, get_reg_index(R_028C44_PA_SC_BINNER_CNTL_0), 0x3);
         if (ctx->info->gfx_level >= GFX10) {
            ac_record_set_context_reg(ctx, get_reg_index(R_0287FC_GE_MAX_OUTPUT_PER_SUBGROUP), 0);
            ac_record_set_context_reg(ctx, get_reg_index(R_028B4C_GE_NGG_SUBGRP_CNTL), 0);
         }
         if (ctx->info->gfx_level >= GFX11)
            ac_record_set_context_reg(ctx, get_reg_index(R_0283D0_PA_SC_VRS_OVERRIDE_CNTL), 0);
         else if (ctx->info->gfx_level == GFX10_3)
            ac_record_set_context_reg(ctx, get_reg_index(R_028064_DB_VRS_OVERRIDE_CNTL), 0);

         ac_record_set_context_reg(ctx, get_reg_index(R_028754_SX_PS_DOWNCONVERT), 0);
         ac_record_set_context_reg(ctx, get_reg_index(R_028758_SX_BLEND_OPT_EPSILON), 0);
         ac_record_set_context_reg(ctx, get_reg_index(R_02875C_SX_BLEND_OPT_CONTROL), 0);

         ac_record_set_context_reg(ctx, get_reg_index(R_028AAC_VGT_ESGS_RING_ITEMSIZE), 0);
         ac_record_set_context_reg(ctx, get_reg_index(R_028AB4_VGT_REUSE_OFF), 0);
         if (ctx->info->gfx_level <= GFX9)
            ac_record_set_context_reg(ctx, get_reg_index(R_028AA8_IA_MULTI_VGT_PARAM), 0xff);

         if (ctx->info->gfx_level == GFX9)
            ac_record_set_context_reg(ctx, get_reg_index(R_028A94_VGT_GS_MAX_PRIMS_PER_SUBGROUP), 0);
         if (ctx->info->gfx_level <= GFX10_3) {
            ac_record_set_context_reg(ctx, get_reg_index(R_028A44_VGT_GS_ONCHIP_CNTL), 0);
            ac_record_set_context_reg(ctx, get_reg_index(R_028AB0_VGT_GSVS_RING_ITEMSIZE), 0);
            ac_record_set_context_reg(ctx, get_reg_index(R_028A40_VGT_GS_MODE), 0);
            ac_record_set_context_reg(ctx, get_reg_index(R_028C58_VGT_VERTEX_REUSE_BLOCK_CNTL), 0x1e);
            ac_record_set_context_reg(ctx, get_reg_index(R_028A6C_VGT_GS_OUT_PRIM_TYPE), 0);

            ac_record_set_context_reg(ctx, get_reg_index(R_028A60_VGT_GSVS_RING_OFFSET_1), 0);
            ac_record_set_context_reg(ctx, get_reg_index(R_028A64_VGT_GSVS_RING_OFFSET_2), 0);
            ac_record_set_context_reg(ctx, get_reg_index(R_028A68_VGT_GSVS_RING_OFFSET_3), 0);

            ac_record_set_context_reg(ctx, get_reg_index(R_028B5C_VGT_GS_VERT_ITEMSIZE), 0);
            ac_record_set_context_reg(ctx, get_reg_index(R_028B60_VGT_GS_VERT_ITEMSIZE_1), 0);
            ac_record_set_context_reg(ctx, get_reg_index(R_028B64_VGT_GS_VERT_ITEMSIZE_2), 0);
            ac_record_set_context_reg(ctx, get_reg_index(R_028B68_VGT_GS_VERT_ITEMSIZE_3), 0);
         }

         ac_record_set_context_reg(ctx, get_reg_index(R_028010_DB_RENDER_OVERRIDE2), 0);
         ac_record_set_context_reg(ctx, get_reg_index(R_0286C4_SPI_VS_OUT_CONFIG), 0);
         ac_record_set_context_reg(ctx, get_reg_index(R_028A84_VGT_PRIMITIVEID_EN), 0);
         ac_record_set_context_reg(ctx, get_reg_index(R_028424_CB_DCC_CONTROL), 0);
         break;

      case PKT3_LOAD_CONTEXT_REG_INDEX:
      case PKT3_COPY_DATA:
         /* TODO */
         break;

      case PKT3_ACQUIRE_MEM:
         if (G_580_PWS_ENA2(ib[cur_dw])) {
            ac_record_wait_idle(ctx);
         } else {
            ac_roll_context(ctx);
            ctx->cur->deltas.acquire_mem = true;
         }
         break;

      case PKT3_WAIT_REG_MEM:
         ac_record_wait_idle(ctx);
         break;

      case PKT3_EVENT_WRITE:
         if (G_490_EVENT_TYPE(ib[cur_dw]) == V_028A90_PS_PARTIAL_FLUSH)
            ac_record_wait_idle(ctx);
         break;

      /* Record draws. */
      case PKT3_DRAW_INDEX_AUTO:
      case PKT3_DRAW_INDEX_IMMD:
      case PKT3_DRAW_INDEX_MULTI_AUTO:
      case PKT3_DRAW_INDEX_2:
      case PKT3_DRAW_INDEX_OFFSET_2:
      case PKT3_DRAW_INDIRECT:
      case PKT3_DRAW_INDEX_INDIRECT:
      case PKT3_DRAW_INDIRECT_MULTI:
      case PKT3_DRAW_INDEX_INDIRECT_MULTI:
      case PKT3_DISPATCH_MESH_DIRECT:
      case PKT3_DISPATCH_MESH_INDIRECT_MULTI:
      case PKT3_DISPATCH_TASKMESH_GFX:
         ctx->context_busy = true;
         break;

      case PKT3_INDIRECT_BUFFER:
         /* Chaining. Note that the CHAIN bit is not set at this point, so we can't distinguish
          * between chaining and IB2.
          */
         return;

      case PKT3_CONTEXT_REG_RMW:
      case PKT3_INDIRECT_BUFFER_SI:
      case PKT3_SURFACE_SYNC:
         fprintf(stderr, "Unhandled packet: 0x%x\n", op);
         abort();
         break;
      }

      cur_dw += count + 1;
   }
}

void ac_gather_context_rolls(FILE *f, uint32_t **ibs, uint32_t *ib_dw_sizes, unsigned num_ibs,
                             const struct radeon_info *info)
{
   struct ac_context_roll_ctx ctx;

   /* Initialize. */
   memset(&ctx, 0, sizeof(ctx));
   ctx.info = info;
   ctx.cur = CALLOC_STRUCT(ac_context_reg_state);
   util_dynarray_init(&ctx.rolls, NULL);

   /* Parse the IBs. */
   for (unsigned i = 0; i < num_ibs; i++)
      ac_ib_gather_context_rolls(&ctx, ibs[i], ib_dw_sizes[i]);

   /* Roll the last context to add it to the list. */
   ac_roll_context(&ctx);

   /* Print context rolls. */
   if (util_dynarray_num_elements(&ctx.rolls, struct ac_context_reg_state *)) {
      /* Print the context rolls starting with the most frequent one. */
      util_dynarray_foreach(&ctx.rolls, struct ac_context_reg_state *, iter) {
         struct ac_context_reg_state *state = *iter;

         unsigned i;
         BITSET_FOREACH_SET(i, state->deltas.changed, 1024) {
            unsigned reg_offset = SI_CONTEXT_REG_OFFSET + i * 4;
            const struct si_reg *reg = ac_find_register(info->gfx_level, info->family,
                                                        reg_offset);

            if (!reg) {
               fprintf(f, "0x%X(0x%x) ", reg_offset, state->deltas.changed_masks[i]);
            } else {
               fprintf(f, "%s(0x%x) ", sid_strings + reg->name_offset,
                       state->deltas.changed_masks[i]);
            }
         }

         if (state->deltas.acquire_mem)
            fprintf(f, "ACQUIRE_MEM");

         fprintf(f, "\n");
      }
   }

   /* Free. */
   FREE(ctx.cur);
   util_dynarray_foreach(&ctx.rolls, struct ac_context_reg_state *, iter) {
      FREE(*iter);
   }
   util_dynarray_fini(&ctx.rolls);
}
