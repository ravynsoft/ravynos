/*
 * Copyright 2023 Pavel Ondračka <pavel.ondracka@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE. */

#ifndef R300_NIR_H
#define R300_NIR_H

#include "pipe/p_screen.h"
#include "compiler/nir/nir.h"

static inline bool
is_ubo_or_input(UNUSED struct hash_table *ht, const nir_alu_instr *instr,
                unsigned src, unsigned num_components,
                const uint8_t *swizzle)
{
   nir_instr *parent = instr->src[src].src.ssa->parent_instr;
   if (parent->type != nir_instr_type_intrinsic)
      return false;

   nir_intrinsic_instr *intrinsic = nir_instr_as_intrinsic(parent);

   switch (intrinsic->intrinsic) {
   case nir_intrinsic_load_ubo_vec4:
   case nir_intrinsic_load_input:
   case nir_intrinsic_load_interpolated_input:
      return true;
   default:
      return false;
   }
}

static inline bool
is_only_used_by_load_ubo_vec4(const nir_alu_instr *instr)
{
   nir_foreach_use(src, &instr->def) {
      if (nir_src_is_if(src))
         return false;
      nir_instr *user_instr = nir_src_parent_instr(src);
      if (user_instr->type != nir_instr_type_intrinsic)
         return false;

      const nir_intrinsic_instr *const user_intrinsic = nir_instr_as_intrinsic(user_instr);

      if (user_intrinsic->intrinsic != nir_intrinsic_load_ubo_vec4)
            return false;
   }
   return true;
}

char *r300_finalize_nir(struct pipe_screen *pscreen, void *nir);

extern bool r300_transform_vs_trig_input(struct nir_shader *shader);

extern bool r300_transform_fs_trig_input(struct nir_shader *shader);

extern bool r300_nir_fuse_fround_d3d9(struct nir_shader *shader);

extern bool r300_nir_lower_bool_to_float(struct nir_shader *shader);

extern bool r300_nir_prepare_presubtract(struct nir_shader *shader);

extern bool r300_nir_clean_double_fneg(struct nir_shader *shader);

extern bool r300_nir_post_integer_lowering(struct nir_shader *shader);

extern bool r300_nir_lower_fcsel_r500(nir_shader *shader);

extern bool r300_nir_lower_fcsel_r300(nir_shader *shader);

extern bool r300_nir_lower_flrp(nir_shader *shader);

#endif /* R300_NIR_H */
