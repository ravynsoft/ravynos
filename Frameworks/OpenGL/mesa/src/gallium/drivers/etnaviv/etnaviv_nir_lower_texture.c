/*
 * Copyright © 2023 Igalia S.L.
 * SPDX-License-Identifier: MIT
 */

#include "etnaviv_nir.h"

static bool
lower_txs(nir_builder *b, nir_instr *instr, UNUSED void *data)
{
   if (instr->type != nir_instr_type_tex)
      return false;

   nir_tex_instr *tex = nir_instr_as_tex(instr);

   if (tex->op != nir_texop_txs)
      return false;

   b->cursor = nir_instr_remove(instr);

   nir_def *idx = nir_imm_int(b, tex->texture_index);
   nir_def *sizes = nir_load_texture_size_etna(b, 32, idx);

   nir_def_rewrite_uses(&tex->def, sizes);

   return true;
}

bool
etna_nir_lower_texture(nir_shader *s, struct etna_shader_key *key)
{
   bool progress = false;

   nir_lower_tex_options lower_tex_options = {
      .lower_txp = ~0u,
      .lower_txs_lod = true,
      .lower_invalid_implicit_lod = true,
   };

   NIR_PASS(progress, s, nir_lower_tex, &lower_tex_options);

   if (key->has_sample_tex_compare)
      NIR_PASS(progress, s, nir_lower_tex_shadow, key->num_texture_states,
                                                  key->tex_compare_func,
                                                  key->tex_swizzle);

   NIR_PASS(progress, s, nir_shader_instructions_pass, lower_txs,
         nir_metadata_block_index | nir_metadata_dominance, NULL);

   return progress;
}
