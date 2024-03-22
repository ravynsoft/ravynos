/*
 * Copyright 2023 Valve Corporation
 * Copyright 2020 Collabora, Ltd.
 * SPDX-License-Identifier: MIT
 */

#include "nir_legacy.h"
#include "nir.h"
#include "nir_builder.h"

bool
nir_legacy_float_mod_folds(nir_alu_instr *mod)
{
   assert(mod->op == nir_op_fabs || mod->op == nir_op_fneg);

   /* No legacy user supports fp64 modifiers */
   if (mod->def.bit_size == 64)
      return false;

   nir_foreach_use_including_if(src, &mod->def) {
      if (nir_src_is_if(src))
         return false;

      nir_instr *parent = nir_src_parent_instr(src);
      if (parent->type != nir_instr_type_alu)
         return false;

      nir_alu_instr *alu = nir_instr_as_alu(parent);
      nir_alu_src *alu_src = list_entry(src, nir_alu_src, src);
      unsigned src_index = alu_src - alu->src;

      assert(src_index < nir_op_infos[alu->op].num_inputs);
      nir_alu_type src_type = nir_op_infos[alu->op].input_types[src_index];

      if (nir_alu_type_get_base_type(src_type) != nir_type_float)
         return false;
   }

   return true;
}

static nir_legacy_alu_src
chase_alu_src_helper(const nir_src *src)
{
   nir_intrinsic_instr *load = nir_load_reg_for_def(src->ssa);

   if (load) {
      bool indirect = (load->intrinsic == nir_intrinsic_load_reg_indirect);

      return (nir_legacy_alu_src){
         .src.is_ssa = false,
         .src.reg = {
            .handle = load->src[0].ssa,
            .base_offset = nir_intrinsic_base(load),
            .indirect = indirect ? load->src[1].ssa : NULL },
         .fabs = nir_intrinsic_legacy_fabs(load),
         .fneg = nir_intrinsic_legacy_fneg(load),
      };
   } else {
      return (nir_legacy_alu_src){
         .src.is_ssa = true,
         .src.ssa = src->ssa,
      };
   }
}

static inline bool
chase_source_mod(nir_def **ssa, nir_op op, uint8_t *swizzle)
{
   if ((*ssa)->parent_instr->type != nir_instr_type_alu)
      return false;

   nir_alu_instr *alu = nir_instr_as_alu((*ssa)->parent_instr);
   if (alu->op != op)
      return false;

   /* If there are other uses of the modifier that don't fold, we can't fold it
    * here either, in case of it's reading from a load_reg that won't be
    * emitted.
    */
   if (!nir_legacy_float_mod_folds(alu))
      return false;

   /* This only works for unary ops */
   assert(nir_op_infos[op].num_inputs == 1);

   /* To fuse the source mod in, we need to compose the swizzles and string
    * through the source.
    */
   for (unsigned i = 0; i < NIR_MAX_VEC_COMPONENTS; ++i)
      swizzle[i] = alu->src[0].swizzle[swizzle[i]];

   *ssa = alu->src[0].src.ssa;
   return true;
}

nir_legacy_alu_src
nir_legacy_chase_alu_src(const nir_alu_src *src, bool fuse_fabs)
{
   if (src->src.ssa->parent_instr->type == nir_instr_type_alu) {
      nir_legacy_alu_src out = {
         .src.is_ssa = true,
         .src.ssa = src->src.ssa,
      };
      STATIC_ASSERT(sizeof(src->swizzle) == sizeof(out.swizzle));
      memcpy(out.swizzle, src->swizzle, sizeof(src->swizzle));

      /* To properly handle foo(fneg(fabs(x))), we first chase fneg and then
       * fabs, since we chase from bottom-up. We don't handle fabs(fneg(x))
       * since nir_opt_algebraic should have eliminated that.
       */
      out.fneg = chase_source_mod(&out.src.ssa, nir_op_fneg, out.swizzle);
      if (fuse_fabs)
         out.fabs = chase_source_mod(&out.src.ssa, nir_op_fabs, out.swizzle);

      return out;
   } else {
      nir_legacy_alu_src out = chase_alu_src_helper(&src->src);
      memcpy(out.swizzle, src->swizzle, sizeof(src->swizzle));
      return out;
   }
}

static nir_legacy_alu_dest
chase_alu_dest_helper(nir_def *def)
{
   nir_intrinsic_instr *store = nir_store_reg_for_def(def);

   if (store) {
      bool indirect = (store->intrinsic == nir_intrinsic_store_reg_indirect);

      return (nir_legacy_alu_dest){
         .dest.is_ssa = false,
         .dest.reg = {
            .handle = store->src[1].ssa,
            .base_offset = nir_intrinsic_base(store),
            .indirect = indirect ? store->src[2].ssa : NULL },
         .fsat = nir_intrinsic_legacy_fsat(store),
         .write_mask = nir_intrinsic_write_mask(store),
      };
   } else {
      return (nir_legacy_alu_dest){
         .dest.is_ssa = true,
         .dest.ssa = def,
         .write_mask = nir_component_mask(def->num_components),
      };
   }
}

bool
nir_legacy_fsat_folds(nir_alu_instr *fsat)
{
   assert(fsat->op == nir_op_fsat);
   nir_def *def = fsat->src[0].src.ssa;

   /* No legacy user supports fp64 modifiers */
   if (def->bit_size == 64)
      return false;

   /* Must be the only use */
   if (!list_is_singular(&def->uses))
      return false;

   assert(&fsat->src[0].src ==
          list_first_entry(&def->uses, nir_src, use_link));

   nir_instr *generate = def->parent_instr;
   if (generate->type != nir_instr_type_alu)
      return false;

   nir_alu_instr *generate_alu = nir_instr_as_alu(generate);
   nir_alu_type dest_type = nir_op_infos[generate_alu->op].output_type;
   if (dest_type != nir_type_float)
      return false;

   /* If we are a saturating a source modifier fsat(fabs(x)), we need to emit
    * either the fsat or the modifier or else the sequence disappears.
    */
   if (generate_alu->op == nir_op_fabs || generate_alu->op == nir_op_fneg)
      return false;

   /* We can't do expansions without a move in the middle */
   unsigned nr_components = generate_alu->def.num_components;
   if (fsat->def.num_components != nr_components)
      return false;

   /* We don't handle swizzles here, so check for the identity */
   for (unsigned i = 0; i < nr_components; ++i) {
      if (fsat->src[0].swizzle[i] != i)
         return false;
   }

   return true;
}

static inline bool
chase_fsat(nir_def **def)
{
   /* No legacy user supports fp64 modifiers */
   if ((*def)->bit_size == 64)
      return false;

   if (!list_is_singular(&(*def)->uses))
      return false;

   nir_src *use = list_first_entry(&(*def)->uses, nir_src, use_link);
   if (nir_src_is_if(use) || nir_src_parent_instr(use)->type != nir_instr_type_alu)
      return false;

   nir_alu_instr *fsat = nir_instr_as_alu(nir_src_parent_instr(use));
   if (fsat->op != nir_op_fsat || !nir_legacy_fsat_folds(fsat))
      return false;

   /* Otherwise, we're good */
   nir_alu_instr *alu = nir_instr_as_alu(nir_src_parent_instr(use));
   *def = &alu->def;
   return true;
}

nir_legacy_alu_dest
nir_legacy_chase_alu_dest(nir_def *def)
{
   /* Try SSA fsat. No users support 64-bit modifiers. */
   if (chase_fsat(&def)) {
      return (nir_legacy_alu_dest){
         .dest.is_ssa = true,
         .dest.ssa = def,
         .fsat = true,
         .write_mask = nir_component_mask(def->num_components),
      };
   } else {
      return chase_alu_dest_helper(def);
   }
}

nir_legacy_src
nir_legacy_chase_src(const nir_src *src)
{
   nir_legacy_alu_src alu_src = chase_alu_src_helper(src);
   assert(!alu_src.fabs && !alu_src.fneg);
   return alu_src.src;
}

nir_legacy_dest
nir_legacy_chase_dest(nir_def *def)
{
   nir_legacy_alu_dest alu_dest = chase_alu_dest_helper(def);
   assert(!alu_dest.fsat);
   assert(alu_dest.write_mask == nir_component_mask(def->num_components));

   return alu_dest.dest;
}

static bool
fuse_mods_with_registers(nir_builder *b, nir_instr *instr, void *fuse_fabs_)
{
   bool *fuse_fabs = fuse_fabs_;
   if (instr->type != nir_instr_type_alu)
      return false;

   nir_alu_instr *alu = nir_instr_as_alu(instr);
   if ((alu->op == nir_op_fneg || (*fuse_fabs && alu->op == nir_op_fabs)) &&
       nir_legacy_float_mod_folds(alu)) {
      /* Try to fold this instruction into the load, if possible.  We only do
       * this for loads in the same block as the use because uses of loads
       * which cross block boundaries aren't trivial anyway.
       */
      nir_intrinsic_instr *load = nir_load_reg_for_def(alu->src[0].src.ssa);
      if (load != NULL) {
         /* Duplicate the load before changing it in case there are other
          * users. We assume someone has run CSE so there should be at most
          * four load instructions generated (one for each possible modifier
          * combination), but likely only one or two.
          */
         b->cursor = nir_before_instr(&load->instr);
         load = nir_instr_as_intrinsic(nir_instr_clone(b->shader, &load->instr));
         nir_builder_instr_insert(b, &load->instr);

         if (alu->op == nir_op_fabs) {
            nir_intrinsic_set_legacy_fabs(load, true);
            nir_intrinsic_set_legacy_fneg(load, false);
         } else {
            assert(alu->op == nir_op_fneg);
            bool old_fneg = nir_intrinsic_legacy_fneg(load);
            nir_intrinsic_set_legacy_fneg(load, !old_fneg);
         }

         /* Rewrite all the users to use the modified load instruction.  We
          * already know that they're all float ALU instructions because
          * nir_legacy_float_mod_folds() returned true.
          */
         nir_foreach_use_including_if_safe(use, &alu->def) {
            assert(!nir_src_is_if(use));
            assert(nir_src_parent_instr(use)->type == nir_instr_type_alu);
            nir_alu_src *alu_use = list_entry(use, nir_alu_src, src);
            nir_src_rewrite(&alu_use->src, &load->def);
            for (unsigned i = 0; i < NIR_MAX_VEC_COMPONENTS; ++i)
               alu_use->swizzle[i] = alu->src[0].swizzle[alu_use->swizzle[i]];
         }

         nir_instr_remove(&alu->instr);

         return true;
      } else {
         /* We don't want to attempt to add saturate to foldable mod ops */
         return false;
      }
   }

   nir_legacy_alu_dest dest = nir_legacy_chase_alu_dest(&alu->def);
   if (dest.fsat) {
      nir_intrinsic_instr *store = nir_store_reg_for_def(dest.dest.ssa);

      if (store) {
         nir_intrinsic_set_legacy_fsat(store, true);
         nir_src_rewrite(&store->src[0], &alu->def);
         return true;
      }
   }

   return false;
}

void
nir_legacy_trivialize(nir_shader *s, bool fuse_fabs)
{
   /* First, fuse modifiers with registers. This ensures that the helpers do not
    * chase registers recursively, allowing registers to be trivialized easier.
    */
   if (nir_shader_instructions_pass(s, fuse_mods_with_registers,
                                    nir_metadata_block_index |
                                       nir_metadata_dominance,
                                    &fuse_fabs)) {
      /* If we made progress, we likely left dead loads. Clean them up. */
      NIR_PASS_V(s, nir_opt_dce);
   }

   /* Now that modifiers are dealt with, we can trivialize the regular way. */
   NIR_PASS_V(s, nir_trivialize_registers);
}
