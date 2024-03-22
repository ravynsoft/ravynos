/*
 * Copyright 2023 Valve Corporation
 * Copyright 2014 Intel Corporation
 * SPDX-License-Identifier: MIT
 */

#include "nir.h"
#include "nir_builder.h"

/*
 * This file implements a simple pass that lowers vecN instructions to a series
 * of partial register stores with partial writes.
 */

struct data {
   nir_instr_writemask_filter_cb cb;
   const void *data;
};

/**
 * For a given starting writemask channel and corresponding source index in
 * the vec instruction, insert a store_reg to the vector register with a
 * writemask with all the channels that get read from the same src reg.
 *
 * Returns the writemask of the store, so the parent loop calling this knows
 * which ones have been processed.
 */
static unsigned
insert_store(nir_builder *b, nir_def *reg, nir_alu_instr *vec,
             unsigned start_idx)
{
   assert(start_idx < nir_op_infos[vec->op].num_inputs);
   nir_def *src = vec->src[start_idx].src.ssa;

   unsigned num_components = vec->def.num_components;
   assert(num_components == nir_op_infos[vec->op].num_inputs);
   unsigned write_mask = 0;
   unsigned swiz[NIR_MAX_VEC_COMPONENTS] = { 0 };

   for (unsigned i = start_idx; i < num_components; i++) {
      if (vec->src[i].src.ssa == src) {
         write_mask |= BITFIELD_BIT(i);
         swiz[i] = vec->src[i].swizzle[0];
      }
   }

   /* No sense storing from undef, just return the write mask */
   if (src->parent_instr->type == nir_instr_type_undef)
      return write_mask;

   b->cursor = nir_before_instr(&vec->instr);
   nir_build_store_reg(b, nir_swizzle(b, src, swiz, num_components), reg,
                       .write_mask = write_mask);
   return write_mask;
}

static bool
has_replicated_dest(nir_alu_instr *alu)
{
   return alu->op == nir_op_fdot2_replicated ||
          alu->op == nir_op_fdot3_replicated ||
          alu->op == nir_op_fdot4_replicated ||
          alu->op == nir_op_fdph_replicated;
}

/* Attempts to coalesce the "move" from the given source of the vec to the
 * destination of the instruction generating the value. If, for whatever
 * reason, we cannot coalesce the move, it does nothing and returns 0.  We
 * can then call insert_mov as normal.
 */
static unsigned
try_coalesce(nir_builder *b, nir_def *reg, nir_alu_instr *vec,
             unsigned start_idx, struct data *data)
{
   assert(start_idx < nir_op_infos[vec->op].num_inputs);

   /* If we are going to do a reswizzle, then the vecN operation must be the
    * only use of the source value.
    */
   nir_foreach_use_including_if(src, vec->src[start_idx].src.ssa) {
      if (nir_src_is_if(src))
         return 0;

      if (nir_src_parent_instr(src) != &vec->instr)
         return 0;
   }

   if (vec->src[start_idx].src.ssa->parent_instr->type != nir_instr_type_alu)
      return 0;

   nir_alu_instr *src_alu =
      nir_instr_as_alu(vec->src[start_idx].src.ssa->parent_instr);

   if (has_replicated_dest(src_alu)) {
      /* The fdot instruction is special: It replicates its result to all
       * components.  This means that we can always rewrite its destination
       * and we don't need to swizzle anything.
       */
   } else {
      /* We only care about being able to re-swizzle the instruction if it is
       * something that we can reswizzle.  It must be per-component.  The one
       * exception to this is the fdotN instructions which implicitly splat
       * their result out to all channels.
       */
      if (nir_op_infos[src_alu->op].output_size != 0)
         return 0;

      /* If we are going to reswizzle the instruction, we can't have any
       * non-per-component sources either.
       */
      for (unsigned j = 0; j < nir_op_infos[src_alu->op].num_inputs; j++)
         if (nir_op_infos[src_alu->op].input_sizes[j] != 0)
            return 0;
   }

   /* Only vecN instructions have more than 4 sources and those are disallowed
    * by the above check for non-per-component sources.  This assumption saves
    * us a bit of stack memory.
    */
   assert(nir_op_infos[src_alu->op].num_inputs <= 4);

   /* Stash off all of the ALU instruction's swizzles. */
   uint8_t swizzles[4][NIR_MAX_VEC_COMPONENTS];
   for (unsigned j = 0; j < nir_op_infos[src_alu->op].num_inputs; j++)
      for (unsigned i = 0; i < NIR_MAX_VEC_COMPONENTS; i++)
         swizzles[j][i] = src_alu->src[j].swizzle[i];

   unsigned dest_components = vec->def.num_components;
   assert(dest_components == nir_op_infos[vec->op].num_inputs);

   /* Generate the final write mask */
   nir_component_mask_t write_mask = 0;
   for (unsigned i = start_idx; i < dest_components; i++) {
      if (vec->src[i].src.ssa != &src_alu->def)
         continue;

      write_mask |= BITFIELD_BIT(i);
   }

   /* If the instruction would be vectorized but the backend
    * doesn't support vectorizing this op, abort. */
   if (data->cb && !data->cb(&src_alu->instr, write_mask, data->data))
      return 0;

   for (unsigned i = 0; i < dest_components; i++) {
      bool valid = write_mask & BITFIELD_BIT(i);

      /* At this point, the given vec source matches up with the ALU
       * instruction so we can re-swizzle that component to match.
       */
      if (has_replicated_dest(src_alu)) {
         /* Since the destination is a single replicated value, we don't need
          * to do any reswizzling
          */
      } else {
         for (unsigned j = 0; j < nir_op_infos[src_alu->op].num_inputs; j++) {
            /* For channels we're extending out of nowhere, use a benign swizzle
             * so we don't read invalid components and trip nir_validate.
             */
            unsigned c = valid ? vec->src[i].swizzle[0] : 0;

            src_alu->src[j].swizzle[i] = swizzles[j][c];
         }
      }

      /* Clear the no longer needed vec source */
      if (valid)
         nir_instr_clear_src(&vec->instr, &vec->src[i].src);
   }

   /* We've cleared the only use of the destination */
   assert(list_is_empty(&src_alu->def.uses));

   /* ... so we can replace it with the bigger destination accommodating the
    * whole vector that will be masked for the store.
    */
   unsigned bit_size = vec->def.bit_size;
   assert(bit_size == src_alu->def.bit_size);
   nir_def_init(&src_alu->instr, &src_alu->def, dest_components,
                bit_size);

   /* Then we can store that ALU result directly into the register */
   b->cursor = nir_after_instr(&src_alu->instr);
   nir_build_store_reg(b, &src_alu->def,
                       reg, .write_mask = write_mask);

   return write_mask;
}

static bool
lower(nir_builder *b, nir_instr *instr, void *data_)
{
   struct data *data = data_;
   if (instr->type != nir_instr_type_alu)
      return false;

   nir_alu_instr *vec = nir_instr_as_alu(instr);
   if (!nir_op_is_vec(vec->op))
      return false;

   unsigned num_components = vec->def.num_components;

   /* Special case: if all sources are the same, just swizzle instead to avoid
    * the extra copies from a register.
    */
   bool need_reg = false;
   for (unsigned i = 1; i < num_components; ++i) {
      if (!nir_srcs_equal(vec->src[0].src, vec->src[i].src)) {
         need_reg = true;
         break;
      }
   }

   if (need_reg) {
      /* We'll replace with a register. Declare one for the purpose. */
      nir_def *reg = nir_decl_reg(b, num_components,
                                  vec->def.bit_size, 0);

      unsigned finished_write_mask = 0;
      for (unsigned i = 0; i < num_components; i++) {
         /* Try to coalesce the move */
         if (!(finished_write_mask & BITFIELD_BIT(i)))
            finished_write_mask |= try_coalesce(b, reg, vec, i, data);

         /* Otherwise fall back on the simple path */
         if (!(finished_write_mask & BITFIELD_BIT(i)))
            finished_write_mask |= insert_store(b, reg, vec, i);
      }

      nir_rewrite_uses_to_load_reg(b, &vec->def, reg);
   } else {
      /* Otherwise, we replace with a swizzle */
      unsigned swiz[NIR_MAX_VEC_COMPONENTS] = { 0 };

      for (unsigned i = 0; i < num_components; ++i) {
         swiz[i] = vec->src[i].swizzle[0];
      }

      b->cursor = nir_before_instr(instr);
      nir_def *swizzled = nir_swizzle(b, vec->src[0].src.ssa, swiz,
                                      num_components);
      nir_def_rewrite_uses(&vec->def, swizzled);
   }

   nir_instr_remove(&vec->instr);
   nir_instr_free(&vec->instr);
   return true;
}

bool
nir_lower_vec_to_regs(nir_shader *shader, nir_instr_writemask_filter_cb cb,
                      const void *_data)
{
   struct data data = {
      .cb = cb,
      .data = _data
   };

   return nir_shader_instructions_pass(shader, lower,
                                       nir_metadata_block_index |
                                          nir_metadata_dominance,
                                       &data);
}
