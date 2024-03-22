/*
 * Copyright © 2021 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

/* This helps separate shaders because the next shader doesn't have to be known.
 *
 * It optimizes VS and TES outputs before FS as follows:
 * - Eliminate and merge equal outputs, and treat undef as equal to everything, e.g.
 *   (x,y,undef,undef) == (undef,y,z,undef) --> (x,y,z,undef) regardless of the interpolation
 *   qualifier (AMD can map 1 output to multiple PS inputs and interpolate each differently).
 * - Remove constant outputs that match AMD DEFAULT_VAL options, e.g. (0,0,0,1),
 *   treat undef as whatever.
 *
 * It requires that there is no indirect indexing and all output stores must be scalar.
 */

#include "ac_nir.h"
#include "nir_builder.h"

struct ac_chan_info {
   nir_instr *value;
   nir_intrinsic_instr *store_intr; /* The intrinsic writing the value. */
};

struct ac_out_info {
   unsigned base; /* nir_intrinsic_base */
   nir_alu_type types;
   bool duplicated;
   bool constant;

   /* Channels 0-3 are 32-bit channels or low bits of 16-bit channels.
    * Channels 4-7 are high bits of 16-bit channels.
    */
   struct ac_chan_info chan[8];
};

static void ac_remove_varying(struct ac_out_info *out)
{
   /* Remove the output. (all channels) */
   for (unsigned i = 0; i < ARRAY_SIZE(out->chan); i++) {
      if (out->chan[i].store_intr) {
         nir_remove_varying(out->chan[i].store_intr, MESA_SHADER_FRAGMENT);
         out->chan[i].store_intr = NULL;
         out->chan[i].value = NULL;
      }
   }
}

/* Return true if the output matches DEFAULT_VAL and has been eliminated. */
static bool ac_eliminate_const_output(struct ac_out_info *out,
                                      gl_varying_slot semantic,
                                      uint8_t *param_export_index)
{
   if (!(out->types & 32))
      return false;

   bool is_zero[4] = {0}, is_one[4] = {0};

   for (unsigned i = 0; i < 4; i++) {
      /* NULL means undef. */
      if (!out->chan[i].value) {
         is_zero[i] = true;
         is_one[i] = true;
      } else if (out->chan[i].value->type == nir_instr_type_load_const) {
         if (nir_instr_as_load_const(out->chan[i].value)->value[0].f32 == 0)
            is_zero[i] = true;
         else if (nir_instr_as_load_const(out->chan[i].value)->value[0].f32 == 1)
            is_one[i] = true;
         else
            return false; /* other constant */
      } else
         return false;
   }

   /* Only certain combinations of 0 and 1 are supported. */
   unsigned default_val; /* SPI_PS_INPUT_CNTL_i.DEFAULT_VAL */

   if (is_zero[0] && is_zero[1] && is_zero[2]) {
      if (is_zero[3])
         default_val = AC_EXP_PARAM_DEFAULT_VAL_0000;
      else if (is_one[3])
         default_val = AC_EXP_PARAM_DEFAULT_VAL_0001;
      else
         return false;
   } else if (is_one[0] && is_one[1] && is_one[2]) {
      if (is_zero[3])
         default_val = AC_EXP_PARAM_DEFAULT_VAL_1110;
      else if (is_one[3])
         default_val = AC_EXP_PARAM_DEFAULT_VAL_1111;
      else
         return false;
   } else {
      return false;
   }

   /* Change OFFSET to DEFAULT_VAL. */
   param_export_index[semantic] = default_val;
   out->constant = true;
   ac_remove_varying(out);
   return true;
}

static bool ac_eliminate_duplicated_output(struct ac_out_info *outputs,
                                           BITSET_DECLARE(outputs_optimized, NUM_TOTAL_VARYING_SLOTS),
                                           gl_varying_slot current, struct nir_builder *b,
                                           int8_t slot_remap[NUM_TOTAL_VARYING_SLOTS])
{
   struct ac_out_info *cur = &outputs[current];
   unsigned p, copy_back_channels = 0;

   /* Check all outputs before current. */
   BITSET_FOREACH_SET(p, outputs_optimized, current) {
      struct ac_out_info *prev = &outputs[p];

      /* Only compare with real outputs. */
      if (prev->constant || prev->duplicated)
         continue;

      /* The types must match (only 16-bit and 32-bit types are allowed). */
      if ((prev->types & 16) != (cur->types & 16))
         continue;

      bool different = false;

      /* Iterate over all channels, including 16-bit channels in chan_hi. */
      for (unsigned j = 0; j < 8; j++) {
         nir_instr *prev_chan = prev->chan[j].value;
         nir_instr *cur_chan = cur->chan[j].value;

         /* Treat undef as a match. */
         if (!cur_chan)
            continue;

         /* If prev is undef but cur isn't, we can merge the outputs
          * and consider the output duplicated.
          */
         if (!prev_chan) {
            copy_back_channels |= 1 << j;
            continue;
         }

         /* Test whether the values are different. */
         if (prev_chan != cur_chan &&
             (prev_chan->type != nir_instr_type_load_const ||
              cur_chan->type != nir_instr_type_load_const ||
              nir_instr_as_load_const(prev_chan)->value[0].u32 !=
              nir_instr_as_load_const(cur_chan)->value[0].u32)) {
            different = true;
            break;
         }
      }
      if (!different)
         break;

      copy_back_channels = 0;
   }
   if (p == current)
      return false;

   /* An equal output already exists. Make FS use the existing one instead.
    * This effectively disables the current output and the param export shouldn't
    * be generated.
    */
   cur->duplicated = true;

   /* p is gl_varying_slot in addition to being an index into outputs. */
   slot_remap[current] = p;

   /* If the matching preceding output has undef where the current one has a proper value,
    * move the value to the preceding output.
    */
   struct ac_out_info *prev = &outputs[p];

   while (copy_back_channels) {
      unsigned i = u_bit_scan(&copy_back_channels);
      struct ac_chan_info *prev_chan = &prev->chan[i];
      struct ac_chan_info *cur_chan = &cur->chan[i];

      b->cursor = nir_after_instr(&cur_chan->store_intr->instr);

      /* The store intrinsic doesn't exist for this channel. Create a new one. */
      nir_alu_type src_type = nir_intrinsic_src_type(cur_chan->store_intr);
      struct nir_io_semantics sem = nir_intrinsic_io_semantics(cur_chan->store_intr);
      struct nir_io_xfb xfb = nir_intrinsic_io_xfb(cur_chan->store_intr);
      struct nir_io_xfb xfb2 = nir_intrinsic_io_xfb2(cur_chan->store_intr);

      /* p is gl_varying_slot in addition to being an index into outputs. */
      sem.location = p;
      assert(sem.high_16bits == i / 4);

      /* If it's a sysval output (such as CLIPDIST), we move the varying portion but keep
       * the system value output. This is just the varying portion.
       */
      sem.no_sysval_output = 1;

      /* Write just one component. */
      prev_chan->store_intr = nir_store_output(b, nir_instr_def(cur_chan->value),
                                               nir_imm_int(b, 0),
                                               .base = prev->base,
                                               .component = i % 4,
                                               .io_semantics = sem,
                                               .src_type = src_type,
                                               .write_mask = 0x1,
                                               .io_xfb = xfb,
                                               .io_xfb2 = xfb2);

      /* Update the undef channels in the output info. */
      assert(!prev_chan->value);
      prev_chan->value = cur_chan->value;

      /* Remove transform feedback info from the current instruction because
       * we moved it too. The instruction might not be removed if it's a system
       * value output.
       */
      static struct nir_io_xfb zero_xfb;
      nir_intrinsic_set_io_xfb(cur->chan[i].store_intr, zero_xfb);
      nir_intrinsic_set_io_xfb2(cur->chan[i].store_intr, zero_xfb);
   }

   ac_remove_varying(cur);
   return true;
}

bool ac_nir_optimize_outputs(nir_shader *nir, bool sprite_tex_disallowed,
                             int8_t slot_remap[NUM_TOTAL_VARYING_SLOTS],
                             uint8_t param_export_index[NUM_TOTAL_VARYING_SLOTS])
{
   nir_function_impl *impl = nir_shader_get_entrypoint(nir);
   assert(impl);

   if (nir->info.stage != MESA_SHADER_VERTEX &&
       nir->info.stage != MESA_SHADER_TESS_EVAL) {
      nir_metadata_preserve(impl, nir_metadata_all);
      return false;
   }

   struct ac_out_info outputs[NUM_TOTAL_VARYING_SLOTS] = { 0 };

   BITSET_DECLARE(outputs_optimized, NUM_TOTAL_VARYING_SLOTS);
   BITSET_ZERO(outputs_optimized);

   /* Gather outputs. */
   nir_foreach_block(block, impl) {
      nir_foreach_instr_safe(instr, block) {
         if (instr->type != nir_instr_type_intrinsic)
            continue;

         nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
         if (intr->intrinsic != nir_intrinsic_store_output)
            continue;

         nir_io_semantics sem = nir_intrinsic_io_semantics(intr);

         /* Only process varyings that appear as param exports. */
         if (!nir_slot_is_varying(sem.location) || sem.no_varying)
            continue;

         /* We can't optimize texture coordinates if sprite_coord_enable can override them. */
         if (sem.location >= VARYING_SLOT_TEX0 && sem.location <= VARYING_SLOT_TEX7 &&
             !sprite_tex_disallowed)
            continue;

         BITSET_SET(outputs_optimized, sem.location);

         /* No indirect indexing allowed. */
         ASSERTED nir_src offset = *nir_get_io_offset_src(intr);
         assert(nir_src_is_const(offset) && nir_src_as_uint(offset) == 0);

         /* nir_lower_io_to_scalar is required before this */
         assert(intr->src[0].ssa->num_components == 1);
         /* No intrinsic should store undef. */
         assert(intr->src[0].ssa->parent_instr->type != nir_instr_type_undef);

         /* Gather the output. */
         struct ac_out_info *out_info = &outputs[sem.location];
         if (!out_info->types)
            out_info->base = nir_intrinsic_base(intr);
         else
            assert(out_info->base == nir_intrinsic_base(intr));

         out_info->types |= nir_intrinsic_src_type(intr);

         unsigned chan = sem.high_16bits * 4 + nir_intrinsic_component(intr);
         out_info->chan[chan].store_intr = intr;
         out_info->chan[chan].value = intr->src[0].ssa->parent_instr;
      }
   }

   unsigned i;
   bool progress = false;

   struct nir_builder b = nir_builder_create(impl);

   /* Optimize outputs. */
   BITSET_FOREACH_SET(i, outputs_optimized, NUM_TOTAL_VARYING_SLOTS) {
      progress |=
         ac_eliminate_const_output(&outputs[i], i, param_export_index) ||
         ac_eliminate_duplicated_output(outputs, outputs_optimized, i, &b, slot_remap);
   }

   if (progress) {
      nir_metadata_preserve(impl, nir_metadata_dominance |
                                  nir_metadata_block_index);
   } else {
      nir_metadata_preserve(impl, nir_metadata_all);
   }
   return progress;
}
