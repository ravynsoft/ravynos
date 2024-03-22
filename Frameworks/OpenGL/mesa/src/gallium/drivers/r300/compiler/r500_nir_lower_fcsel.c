#include <stdbool.h>
#include "r300_nir.h"
#include "nir_builder.h"

static int
follow_modifiers(nir_instr *instr)
{
   /* We don't have texturing so the only other options besides alus are
    * just load input, load ubo or phi. We can copy propagate the first two
    * in most cases. The cases when the copy propagate is not guaranteed
    * to work is with indirect ubo load and in the presence of control flow.
    * So just be safe and count this as a separate tmp.
    */
   if (instr->type == nir_instr_type_intrinsic) {
      nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
      /* It should be enough to check if any of the uses is in phi. */
      if (intrin->intrinsic == nir_intrinsic_load_ubo_vec4 ||
          intrin->intrinsic == nir_intrinsic_load_constant ||
          intrin->intrinsic == nir_intrinsic_load_input) {
          nir_foreach_use(use, &intrin->def) {
              if (nir_src_parent_instr(use)->type == nir_instr_type_phi)
                 return intrin->def.index;
          }
      }
      if (intrin->intrinsic == nir_intrinsic_load_ubo_vec4 &&
          !nir_src_is_const(intrin->src[1]))
      return intrin->def.index;
   }
   /* Assume the worst when we see a phi. */
   if (instr->type == nir_instr_type_phi)
      return nir_instr_as_phi(instr)->def.index;

   if (instr->type != nir_instr_type_alu)
      return -1;

   nir_alu_instr *alu = nir_instr_as_alu(instr);

   if (alu->op == nir_op_fneg || alu->op == nir_op_fabs) {
      return follow_modifiers(alu->src[0].src.ssa->parent_instr);
   }
   return alu->def.index;
}

static bool
has_three_different_tmp_sources(nir_alu_instr *fcsel)
{
   unsigned src_def_index[3];
   for (unsigned i = 0; i < 3; i++) {
      int index = follow_modifiers(fcsel->src[i].src.ssa->parent_instr);
      if (index == -1)
         return false;
      else
	 src_def_index[i] = index;
   }
   return src_def_index[0] != src_def_index[1] &&
          src_def_index[0] != src_def_index[2] &&
          src_def_index[1] != src_def_index[2];
}

static bool
is_comparison(nir_instr *instr)
{
   if (instr->type != nir_instr_type_alu)
      return false;

   nir_alu_instr *alu = nir_instr_as_alu(instr);

   switch (alu->op) {
   case nir_op_sge:
   case nir_op_slt:
   case nir_op_seq:
   case nir_op_sne:
      return true;
   default:
      return false;
   }
}

static bool
r300_nir_lower_fcsel_instr(nir_builder *b, nir_instr *instr, void *data)
{
   if (instr->type != nir_instr_type_alu)
      return false;

   nir_alu_instr *alu = nir_instr_as_alu(instr);

   if (alu->op != nir_op_fcsel && alu->op != nir_op_fcsel_ge && alu->op != nir_op_fcsel_gt)
      return false;

   if (has_three_different_tmp_sources(alu)) {
      nir_def *lrp;
      b->cursor = nir_before_instr(&alu->instr);
      /* Lower to LRP.
       * At this point there are no fcsels as all bcsels were converted to
       * fcsel_gt by nir_lower_bool_to_float, however we can save on the slt
       * even for nir_op_fcsel_gt if the source is 0 or 1 anyway.
       */
      nir_instr *src0_instr = alu->src[0].src.ssa->parent_instr;
      if (alu->op == nir_op_fcsel ||
          (alu->op == nir_op_fcsel_gt && is_comparison(src0_instr))) {
         lrp = nir_flrp(b, nir_ssa_for_alu_src(b, alu, 2),
                        nir_ssa_for_alu_src(b, alu, 1),
                        nir_ssa_for_alu_src(b, alu, 0));
      } else if (alu->op == nir_op_fcsel_ge) {
         nir_def *sge = nir_sge(b, nir_ssa_for_alu_src(b, alu, 0), nir_imm_float(b, 0.0));
         lrp = nir_flrp(b, nir_ssa_for_alu_src(b, alu, 2),
                        nir_ssa_for_alu_src(b, alu, 1), sge);
      } else {
         nir_def *slt = nir_slt(b, nir_fneg(b, nir_ssa_for_alu_src(b, alu, 0)),
                                nir_imm_float(b, 0.0));
         lrp = nir_flrp(b, nir_ssa_for_alu_src(b, alu, 2),
                        nir_ssa_for_alu_src(b, alu, 1), slt);
      }

      nir_def_rewrite_uses(&alu->def, lrp);
      nir_instr_remove(&alu->instr);
      return true;
   }
   return false;
}

bool
r300_nir_lower_fcsel_r500(nir_shader *shader)
{
   bool progress = nir_shader_instructions_pass(shader,
                                                r300_nir_lower_fcsel_instr,
                                                nir_metadata_block_index |
                                                   nir_metadata_dominance,
                                                NULL);
   return progress;
}
