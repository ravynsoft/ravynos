/*
 * Copyright 2023 Valve Corporation
 * Copyright 2014 Intel Corporation
 * SPDX-License-Identifier: MIT
 */

#include "nir.h"
#include "nir_builder.h"
#include "nir_intrinsics.h"
#include "nir_intrinsics_indices.h"
#include "nir_phi_builder.h"
#include "nir_vla.h"

static bool
should_lower_reg(nir_intrinsic_instr *decl)
{
   /* This pass only really works on "plain" registers. In particular,
    * base/indirects are not handled. If it's a packed or array register,
    * just set the value to NULL so that the rewrite portion of the pass
    * will know to ignore it.
    */
   return nir_intrinsic_num_array_elems(decl) == 0;
}

struct regs_to_ssa_state {
   nir_builder b;

   /* Scratch bitset for use in setup_reg */
   unsigned defs_words;
   BITSET_WORD *defs;

   struct nir_phi_builder *phi_builder;
   struct nir_phi_builder_value **values;
};

static void
setup_reg(nir_intrinsic_instr *decl, struct regs_to_ssa_state *state)
{
   assert(state->values[decl->def.index] == NULL);
   if (!should_lower_reg(decl))
      return;

   const unsigned num_components = nir_intrinsic_num_components(decl);
   const unsigned bit_size = nir_intrinsic_bit_size(decl);

   memset(state->defs, 0, state->defs_words * sizeof(*state->defs));

   nir_foreach_reg_store(store, decl)
      BITSET_SET(state->defs, nir_src_parent_instr(store)->block->index);

   state->values[decl->def.index] =
      nir_phi_builder_add_value(state->phi_builder, num_components,
                                bit_size, state->defs);
}

static void
rewrite_load(nir_intrinsic_instr *load, struct regs_to_ssa_state *state)
{
   nir_block *block = load->instr.block;
   nir_def *reg = load->src[0].ssa;

   struct nir_phi_builder_value *value = state->values[reg->index];
   if (!value)
      return;

   nir_intrinsic_instr *decl = nir_instr_as_intrinsic(reg->parent_instr);
   nir_def *def = nir_phi_builder_value_get_block_def(value, block);

   nir_def_rewrite_uses(&load->def, def);
   nir_instr_remove(&load->instr);

   if (nir_def_is_unused(&decl->def))
      nir_instr_remove(&decl->instr);
}

static void
rewrite_store(nir_intrinsic_instr *store, struct regs_to_ssa_state *state)
{
   nir_block *block = store->instr.block;
   nir_def *new_value = store->src[0].ssa;
   nir_def *reg = store->src[1].ssa;

   struct nir_phi_builder_value *value = state->values[reg->index];
   if (!value)
      return;

   nir_intrinsic_instr *decl = nir_instr_as_intrinsic(reg->parent_instr);
   unsigned num_components = nir_intrinsic_num_components(decl);
   unsigned write_mask = nir_intrinsic_write_mask(store);

   /* Implement write masks by combining together the old/new values */
   if (write_mask != BITFIELD_MASK(num_components)) {
      nir_def *old_value =
         nir_phi_builder_value_get_block_def(value, block);

      nir_def *channels[NIR_MAX_VEC_COMPONENTS] = { NULL };
      state->b.cursor = nir_before_instr(&store->instr);

      for (unsigned i = 0; i < num_components; ++i) {
         if (write_mask & BITFIELD_BIT(i))
            channels[i] = nir_channel(&state->b, new_value, i);
         else
            channels[i] = nir_channel(&state->b, old_value, i);
      }

      new_value = nir_vec(&state->b, channels, num_components);
   }

   nir_phi_builder_value_set_block_def(value, block, new_value);
   nir_instr_remove(&store->instr);

   if (nir_def_is_unused(&decl->def))
      nir_instr_remove(&decl->instr);
}

bool
nir_lower_reg_intrinsics_to_ssa_impl(nir_function_impl *impl)
{
   bool need_lower_reg = false;
   nir_foreach_reg_decl(reg, impl) {
      if (should_lower_reg(reg)) {
         need_lower_reg = true;
         break;
      }
   }
   if (!need_lower_reg) {
      nir_metadata_preserve(impl, nir_metadata_all);
      return false;
   }

   nir_metadata_require(impl, nir_metadata_block_index |
                                 nir_metadata_dominance);
   nir_index_ssa_defs(impl);

   void *dead_ctx = ralloc_context(NULL);
   struct regs_to_ssa_state state;
   state.b = nir_builder_create(impl);
   state.defs_words = BITSET_WORDS(impl->num_blocks);
   state.defs = ralloc_array(dead_ctx, BITSET_WORD, state.defs_words);
   state.phi_builder = nir_phi_builder_create(state.b.impl);
   state.values = rzalloc_array(dead_ctx, struct nir_phi_builder_value *,
                                impl->ssa_alloc);

   nir_foreach_block(block, impl) {
      nir_foreach_instr_safe(instr, block) {
         if (instr->type != nir_instr_type_intrinsic)
            continue;

         nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
         switch (intr->intrinsic) {
         case nir_intrinsic_decl_reg:
            setup_reg(intr, &state);
            break;
         case nir_intrinsic_load_reg:
            rewrite_load(intr, &state);
            break;
         case nir_intrinsic_store_reg:
            rewrite_store(intr, &state);
            break;
         default:
            break;
         }
      }
   }

   nir_phi_builder_finish(state.phi_builder);

   ralloc_free(dead_ctx);

   nir_metadata_preserve(impl, nir_metadata_block_index |
                                  nir_metadata_dominance);
   return true;
}

bool
nir_lower_reg_intrinsics_to_ssa(nir_shader *shader)
{
   bool progress = false;

   nir_foreach_function_impl(impl, shader) {
      progress |= nir_lower_reg_intrinsics_to_ssa_impl(impl);
   }

   return progress;
}
