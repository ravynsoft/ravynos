/*
 * Copyright 2022 Alyssa Rosenzweig
 * SPDX-License-Identifier: MIT
 */

#include "compiler/nir/nir_builder.h"
#include "agx_compiler.h"

/*
 * Lower load_interpolated_input instructions with unused components of their
 * destination, duplicating the intrinsic and shrinking to avoid the holes.
 * load_interpolated_input becomes iter instructions, which lack a write mask.
 */
static bool
pass(struct nir_builder *b, nir_instr *instr, UNUSED void *data)
{
   if (instr->type != nir_instr_type_intrinsic)
      return false;

   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
   if (intr->intrinsic != nir_intrinsic_load_interpolated_input)
      return false;

   unsigned mask = nir_def_components_read(&intr->def);
   if (mask == 0 || mask == nir_component_mask(intr->num_components))
      return false;

   b->cursor = nir_before_instr(instr);
   unsigned bit_size = intr->def.bit_size;
   nir_def *comps[4] = {NULL};

   for (unsigned c = 0; c < intr->num_components; ++c) {
      if (mask & BITFIELD_BIT(c)) {
         /* Count contiguous components to combine with */
         unsigned next_mask = mask >> c;
         unsigned next_zero = ffs(~next_mask);
         unsigned count = next_zero - 1;

         assert(next_zero >= 2);
         assert(count >= 1);

         nir_instr *clone = nir_instr_clone(b->shader, instr);
         nir_intrinsic_instr *clone_intr = nir_instr_as_intrinsic(clone);

         /* Shrink the load to count contiguous components */
         nir_def_init(clone, &clone_intr->def, count, bit_size);
         nir_def *clone_vec = &clone_intr->def;
         clone_intr->num_components = count;

         /* The load starts from component c relative to the original load */
         nir_intrinsic_set_component(clone_intr,
                                     nir_intrinsic_component(intr) + c);

         nir_builder_instr_insert(b, &clone_intr->instr);

         /* The destination is a vector with `count` components, extract the
          * components so we can recombine into the final vector.
          */
         for (unsigned d = 0; d < count; ++d)
            comps[c + d] = nir_channel(b, clone_vec, d);

         c += (count - 1);
      } else {
         /* The value of unused components is irrelevant, but use an undef for
          * semantics. It will be eliminated by DCE after copyprop.
          */
         comps[c] = nir_undef(b, 1, bit_size);
      }
   }

   nir_def_rewrite_uses(&intr->def, nir_vec(b, comps, intr->num_components));
   return true;
}

bool
agx_nir_lower_load_mask(nir_shader *shader)
{
   return nir_shader_instructions_pass(
      shader, pass, nir_metadata_block_index | nir_metadata_dominance, NULL);
}
