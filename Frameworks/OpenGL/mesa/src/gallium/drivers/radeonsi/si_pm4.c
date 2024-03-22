/*
 * Copyright 2012 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#include "si_pm4.h"
#include "si_pipe.h"
#include "si_build_pm4.h"
#include "sid.h"
#include "util/u_memory.h"
#include "ac_debug.h"

static void si_pm4_set_reg_custom(struct si_pm4_state *state, unsigned reg, uint32_t val,
                                  unsigned opcode, unsigned idx);

static bool opcode_is_pairs_packed(unsigned opcode)
{
   return opcode == PKT3_SET_CONTEXT_REG_PAIRS_PACKED ||
          opcode == PKT3_SET_SH_REG_PAIRS_PACKED ||
          opcode == PKT3_SET_SH_REG_PAIRS_PACKED_N;
}

static unsigned pairs_packed_opcode_to_regular(unsigned opcode)
{
   switch (opcode) {
   case PKT3_SET_CONTEXT_REG_PAIRS_PACKED:
      return PKT3_SET_CONTEXT_REG;
   case PKT3_SET_SH_REG_PAIRS_PACKED:
      return PKT3_SET_SH_REG;
   default:
      unreachable("invalid packed opcode");
   }
}

static unsigned regular_opcode_to_pairs(struct si_pm4_state *state, unsigned opcode)
{
   const struct radeon_info *info = &state->screen->info;

   switch (opcode) {
   case PKT3_SET_CONTEXT_REG:
      return info->has_set_context_pairs_packed ? PKT3_SET_CONTEXT_REG_PAIRS_PACKED : opcode;
   case PKT3_SET_SH_REG:
      return info->has_set_sh_pairs_packed ? PKT3_SET_SH_REG_PAIRS_PACKED : opcode;
   }

   return opcode;
}

static bool packed_next_is_reg_offset_pair(struct si_pm4_state *state)
{
   return (state->ndw - state->last_pm4) % 3 == 2;
}

static bool packed_next_is_reg_value1(struct si_pm4_state *state)
{
   return (state->ndw - state->last_pm4) % 3 == 1;
}

static bool packed_prev_is_reg_value0(struct si_pm4_state *state)
{
   return packed_next_is_reg_value1(state);
}

static unsigned get_packed_reg_dw_offsetN(struct si_pm4_state *state, unsigned index)
{
   unsigned i = state->last_pm4 + 2 + (index / 2) * 3;
   assert(i < state->ndw);
   return (state->pm4[i] >> ((index % 2) * 16)) & 0xffff;
}

static unsigned get_packed_reg_valueN_idx(struct si_pm4_state *state, unsigned index)
{
   unsigned i = state->last_pm4 + 2 + (index / 2) * 3 + 1 + (index % 2);
   assert(i < state->ndw);
   return i;
}

static unsigned get_packed_reg_valueN(struct si_pm4_state *state, unsigned index)
{
   return state->pm4[get_packed_reg_valueN_idx(state, index)];
}

static unsigned get_packed_reg_count(struct si_pm4_state *state)
{
   int body_size = state->ndw - state->last_pm4 - 2;
   assert(body_size > 0 && body_size % 3 == 0);
   return (body_size / 3) * 2;
}

void si_pm4_finalize(struct si_pm4_state *state)
{
   if (opcode_is_pairs_packed(state->last_opcode)) {
      unsigned reg_count = get_packed_reg_count(state);
      unsigned reg_dw_offset0 = get_packed_reg_dw_offsetN(state, 0);

      if (state->packed_is_padded)
         reg_count--;

      bool all_consecutive = true;

      /* If the whole packed SET packet only sets consecutive registers, rewrite the packet
       * to be unpacked to make it shorter.
       *
       * This also eliminates the invalid scenario when the packed SET packet sets only
       * 2 registers and the register offsets are equal due to padding.
       */
      for (unsigned i = 1; i < reg_count; i++) {
         if (reg_dw_offset0 != get_packed_reg_dw_offsetN(state, i) - i) {
            all_consecutive = false;
            break;
         }
      }

      if (all_consecutive) {
         assert(state->ndw - state->last_pm4 == 2 + 3 * (reg_count + state->packed_is_padded) / 2);
         state->pm4[state->last_pm4] = PKT3(pairs_packed_opcode_to_regular(state->last_opcode),
                                            reg_count, 0);
         state->pm4[state->last_pm4 + 1] = reg_dw_offset0;
         for (unsigned i = 0; i < reg_count; i++)
            state->pm4[state->last_pm4 + 2 + i] = get_packed_reg_valueN(state, i);
         state->ndw = state->last_pm4 + 2 + reg_count;
         state->last_opcode = PKT3_SET_SH_REG;
      } else {
         /* Set reg_va_low_idx to where the shader address is stored in the pm4 state. */
         if (state->screen->debug_flags & DBG(SQTT) &&
             (state->last_opcode == PKT3_SET_SH_REG_PAIRS_PACKED ||
              state->last_opcode == PKT3_SET_SH_REG_PAIRS_PACKED_N)) {
            if (state->packed_is_padded)
               reg_count++; /* Add this back because we only need to record the last write. */

            for (int i = reg_count - 1; i >= 0; i--) {
               unsigned reg_offset = SI_SH_REG_OFFSET + get_packed_reg_dw_offsetN(state, i) * 4;

               if (strstr(ac_get_register_name(state->screen->info.gfx_level,
                                               state->screen->info.family, reg_offset),
                          "SPI_SHADER_PGM_LO_")) {
                  state->spi_shader_pgm_lo_reg = reg_offset;
                  break;
               }
            }
         }

         /* If it's a packed SET_SH packet, use the *_N variant when possible. */
         if (state->last_opcode == PKT3_SET_SH_REG_PAIRS_PACKED && reg_count <= 14) {
            state->pm4[state->last_pm4] &= PKT3_IT_OPCODE_C;
            state->pm4[state->last_pm4] |= PKT3_IT_OPCODE_S(PKT3_SET_SH_REG_PAIRS_PACKED_N);
         }
      }
   }

   if (state->screen->debug_flags & DBG(SQTT) && state->last_opcode == PKT3_SET_SH_REG) {
      /* Set reg_va_low_idx to where the shader address is stored in the pm4 state. */
      unsigned reg_count = PKT_COUNT_G(state->pm4[state->last_pm4]);
      unsigned reg_base_offset = SI_SH_REG_OFFSET + state->pm4[state->last_pm4 + 1] * 4;

      for (unsigned i = 0; i < reg_count; i++) {
         if (strstr(ac_get_register_name(state->screen->info.gfx_level,
                                         state->screen->info.family, reg_base_offset + i * 4),
                    "SPI_SHADER_PGM_LO_")) {
            state->spi_shader_pgm_lo_reg = reg_base_offset + i * 4;

            break;
         }
      }
   }
}

static void si_pm4_cmd_begin(struct si_pm4_state *state, unsigned opcode)
{
   si_pm4_finalize(state);

   assert(state->max_dw);
   assert(state->ndw < state->max_dw);
   assert(opcode <= 254);
   state->last_opcode = opcode;
   state->last_pm4 = state->ndw++;
   state->packed_is_padded = false;
}

void si_pm4_cmd_add(struct si_pm4_state *state, uint32_t dw)
{
   assert(state->max_dw);
   assert(state->ndw < state->max_dw);
   state->pm4[state->ndw++] = dw;
   state->last_opcode = 255; /* invalid opcode */
}

static void si_pm4_cmd_end(struct si_pm4_state *state, bool predicate)
{
   unsigned count;
   count = state->ndw - state->last_pm4 - 2;
   /* All SET_*_PAIRS* packets on the gfx queue must set RESET_FILTER_CAM. */
   bool reset_filter_cam = !state->is_compute_queue &&
                           opcode_is_pairs_packed(state->last_opcode);

   state->pm4[state->last_pm4] = PKT3(state->last_opcode, count, predicate) |
                                 PKT3_RESET_FILTER_CAM_S(reset_filter_cam);

   if (opcode_is_pairs_packed(state->last_opcode)) {
      if (packed_prev_is_reg_value0(state)) {
         /* Duplicate the first register at the end to make the number of registers aligned to 2. */
         si_pm4_set_reg_custom(state, get_packed_reg_dw_offsetN(state, 0) * 4,
                               get_packed_reg_valueN(state, 0),
                               state->last_opcode, 0);
         state->packed_is_padded = true;
      }

      state->pm4[state->last_pm4 + 1] = get_packed_reg_count(state);
   }
}

static void si_pm4_set_reg_custom(struct si_pm4_state *state, unsigned reg, uint32_t val,
                                  unsigned opcode, unsigned idx)
{
   bool is_packed = opcode_is_pairs_packed(opcode);
   reg >>= 2;

   assert(state->max_dw);
   assert(state->ndw + 2 <= state->max_dw);

   if (is_packed) {
      assert(idx == 0);

      if (opcode != state->last_opcode) {
         si_pm4_cmd_begin(state, opcode); /* reserve space for the header */
         state->ndw++; /* reserve space for the register count, it will be set at the end */
      }
   } else if (opcode != state->last_opcode || reg != (state->last_reg + 1) ||
              idx != state->last_idx) {
      si_pm4_cmd_begin(state, opcode);
      state->pm4[state->ndw++] = reg | (idx << 28);
   }

   assert(reg <= UINT16_MAX);
   state->last_reg = reg;
   state->last_idx = idx;

   if (is_packed) {
      if (state->packed_is_padded) {
         /* The packet is padded, which means the first register is written redundantly again
          * at the end. Remove it, so that we can replace it with this register.
          */
         state->packed_is_padded = false;
         state->ndw--;
      }

      if (packed_next_is_reg_offset_pair(state)) {
         state->pm4[state->ndw++] = reg;
      } else if (packed_next_is_reg_value1(state)) {
         /* Set the second register offset in the high 16 bits. */
         state->pm4[state->ndw - 2] &= 0x0000ffff;
         state->pm4[state->ndw - 2] |= reg << 16;
      }
   }

   state->pm4[state->ndw++] = val;
   si_pm4_cmd_end(state, false);
}

void si_pm4_set_reg(struct si_pm4_state *state, unsigned reg, uint32_t val)
{
   unsigned opcode;

   if (reg >= SI_CONFIG_REG_OFFSET && reg < SI_CONFIG_REG_END) {
      opcode = PKT3_SET_CONFIG_REG;
      reg -= SI_CONFIG_REG_OFFSET;

   } else if (reg >= SI_SH_REG_OFFSET && reg < SI_SH_REG_END) {
      opcode = PKT3_SET_SH_REG;
      reg -= SI_SH_REG_OFFSET;

   } else if (reg >= SI_CONTEXT_REG_OFFSET && reg < SI_CONTEXT_REG_END) {
      opcode = PKT3_SET_CONTEXT_REG;
      reg -= SI_CONTEXT_REG_OFFSET;

   } else if (reg >= CIK_UCONFIG_REG_OFFSET && reg < CIK_UCONFIG_REG_END) {
      opcode = PKT3_SET_UCONFIG_REG;
      reg -= CIK_UCONFIG_REG_OFFSET;

   } else {
      PRINT_ERR("Invalid register offset %08x!\n", reg);
      return;
   }

   opcode = regular_opcode_to_pairs(state, opcode);

   si_pm4_set_reg_custom(state, reg, val, opcode, 0);
}

void si_pm4_set_reg_idx3(struct si_pm4_state *state, unsigned reg, uint32_t val)
{
   if (state->screen->info.uses_kernel_cu_mask) {
      assert(state->screen->info.gfx_level >= GFX10);
      si_pm4_set_reg_custom(state, reg - SI_SH_REG_OFFSET, val, PKT3_SET_SH_REG_INDEX, 3);
   } else {
      si_pm4_set_reg(state, reg, val);
   }
}

void si_pm4_clear_state(struct si_pm4_state *state, struct si_screen *sscreen,
                        bool is_compute_queue)
{
   state->screen = sscreen;
   state->ndw = 0;
   state->is_compute_queue = is_compute_queue;

   if (!state->max_dw)
      state->max_dw = ARRAY_SIZE(state->pm4);
}

void si_pm4_free_state(struct si_context *sctx, struct si_pm4_state *state, unsigned idx)
{
   if (!state)
      return;

   if (idx != ~0) {
      if (sctx->emitted.array[idx] == state)
         sctx->emitted.array[idx] = NULL;

      if (sctx->queued.array[idx] == state) {
         sctx->queued.array[idx] = NULL;
         sctx->dirty_atoms &= ~BITFIELD64_BIT(idx);
      }
   }

   FREE(state);
}

void si_pm4_emit_commands(struct si_context *sctx, struct si_pm4_state *state)
{
   struct radeon_cmdbuf *cs = &sctx->gfx_cs;

   radeon_begin(cs);
   radeon_emit_array(state->pm4, state->ndw);
   radeon_end();
}

void si_pm4_emit_state(struct si_context *sctx, unsigned index)
{
   struct si_pm4_state *state = sctx->queued.array[index];
   struct radeon_cmdbuf *cs = &sctx->gfx_cs;

   /* All places should unset dirty_states if this doesn't pass. */
   assert(state && state != sctx->emitted.array[index]);

   radeon_begin(cs);
   radeon_emit_array(state->pm4, state->ndw);
   radeon_end();

   sctx->emitted.array[index] = state;
}

void si_pm4_emit_shader(struct si_context *sctx, unsigned index)
{
   struct si_pm4_state *state = sctx->queued.array[index];

   si_pm4_emit_state(sctx, index);

   radeon_add_to_buffer_list(sctx, &sctx->gfx_cs, ((struct si_shader*)state)->bo,
                             RADEON_USAGE_READ | RADEON_PRIO_SHADER_BINARY);
   if (state->atom.emit)
      state->atom.emit(sctx, -1);
}

void si_pm4_reset_emitted(struct si_context *sctx)
{
   memset(&sctx->emitted, 0, sizeof(sctx->emitted));

   for (unsigned i = 0; i < SI_NUM_STATES; i++) {
      if (sctx->queued.array[i])
         sctx->dirty_atoms |= BITFIELD64_BIT(i);
   }
}

struct si_pm4_state *si_pm4_create_sized(struct si_screen *sscreen, unsigned max_dw,
                                         bool is_compute_queue)
{
   struct si_pm4_state *pm4;
   unsigned size = sizeof(*pm4) + 4 * (max_dw - ARRAY_SIZE(pm4->pm4));

   pm4 = (struct si_pm4_state *)calloc(1, size);
   if (pm4) {
      pm4->max_dw = max_dw;
      si_pm4_clear_state(pm4, sscreen, is_compute_queue);
   }
   return pm4;
}

struct si_pm4_state *si_pm4_clone(struct si_pm4_state *orig)
{
   struct si_pm4_state *pm4 = si_pm4_create_sized(orig->screen, orig->max_dw,
                                                  orig->is_compute_queue);
   if (pm4)
      memcpy(pm4, orig, sizeof(*pm4) + 4 * (pm4->max_dw - ARRAY_SIZE(pm4->pm4)));
   return pm4;
}
