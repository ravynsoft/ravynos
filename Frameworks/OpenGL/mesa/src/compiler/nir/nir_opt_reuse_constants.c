/*
 * Copyright 2023 Intel Corporation
 * SPDX-License-Identifier: MIT
 */

#include "nir.h"
#include "nir_instr_set.h"

bool
nir_opt_reuse_constants(nir_shader *shader)
{
   bool progress = false;

   struct set *consts = nir_instr_set_create(NULL);
   nir_foreach_function_impl(impl, shader) {
      _mesa_set_clear(consts, NULL);

      nir_block *start_block = nir_start_block(impl);
      bool func_progress = false;

      nir_foreach_block_safe(block, impl) {
         const bool in_start_block = start_block == block;
         nir_foreach_instr_safe(instr, block) {
            if (instr->type != nir_instr_type_load_const)
               continue;

            struct set_entry *entry = _mesa_set_search(consts, instr);
            if (!entry) {
               if (!in_start_block)
                  nir_instr_move(nir_after_block_before_jump(start_block), instr);
               _mesa_set_add(consts, instr);
            }

            func_progress |= nir_instr_set_add_or_rewrite(consts, instr, nir_instrs_equal);
         }
      }

      if (func_progress) {
         nir_metadata_preserve(impl, nir_metadata_block_index |
                                        nir_metadata_dominance);
         progress = true;
      } else {
         nir_metadata_preserve(impl, nir_metadata_all);
      }
   }

   nir_instr_set_destroy(consts);
   return progress;
}
