/*
 * Copyright 2023 Valve Corporation
 * SPDX-License-Identifier: MIT
 */

#include "compiler/nir/nir.h"
#include "compiler/nir/nir_builder.h"
#include "agx_nir.h"
#include "nir_builder_opcodes.h"
#include "nir_intrinsics.h"

static bool
lower(nir_builder *b, nir_intrinsic_instr *intr, void *data)
{
   b->cursor = nir_before_instr(&intr->instr);

   switch (intr->intrinsic) {
   case nir_intrinsic_vote_any: {
      /* We don't have vote instructions, but we have efficient ballots */
      nir_def *ballot = nir_ballot(b, 1, 32, intr->src[0].ssa);
      nir_def_rewrite_uses(&intr->def, nir_ine_imm(b, ballot, 0));
      return true;
   }

   case nir_intrinsic_vote_all: {
      nir_def *ballot = nir_ballot(b, 1, 32, nir_inot(b, intr->src[0].ssa));
      nir_def_rewrite_uses(&intr->def, nir_ieq_imm(b, ballot, 0));
      return true;
   }

   default:
      return false;
   }
}

void
agx_nir_lower_subgroups(nir_shader *s)
{
   /* First, do as much common lowering as we can */
   nir_lower_subgroups_options opts = {
      .lower_vote_eq = true,
      .lower_vote_bool_eq = true,
      .lower_read_first_invocation = true,
      .lower_first_invocation_to_ballot = true,
      .lower_to_scalar = true,
      .ballot_components = 1,
      .ballot_bit_size = 32,
   };

   NIR_PASS_V(s, nir_lower_subgroups, &opts);

   /* Then do AGX-only lowerings on top */
   nir_shader_intrinsics_pass(
      s, lower, nir_metadata_block_index | nir_metadata_dominance, NULL);
}
