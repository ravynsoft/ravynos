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

#include "r300_nir.h"

#include "compiler/nir/nir_builder.h"
#include "r300_screen.h"

static unsigned char
r300_should_vectorize_instr(const nir_instr *instr, const void *data)
{
   if (instr->type != nir_instr_type_alu)
      return 0;

   return 4;
}

static bool
r300_should_vectorize_io(unsigned align, unsigned bit_size,
                        unsigned num_components, unsigned high_offset,
                        nir_intrinsic_instr *low, nir_intrinsic_instr *high,
                        void *data)
{
   if (bit_size != 32)
      return false;

   /* Our offset alignment should aways be at least 4 bytes */
   if (align < 4)
      return false;

   /* No wrapping off the end of a TGSI reg.  We could do a bit better by
    * looking at low's actual offset.  XXX: With LOAD_CONSTBUF maybe we don't
    * need this restriction.
    */
   unsigned worst_start_component = align == 4 ? 3 : align / 4;
   if (worst_start_component + num_components > 4)
      return false;

   return true;
}

static bool
set_speculate(nir_builder *b, nir_intrinsic_instr *intr, UNUSED void *_)
{
   if (intr->intrinsic == nir_intrinsic_load_ubo_vec4) {
      nir_intrinsic_set_access(intr, nir_intrinsic_access(intr) | ACCESS_CAN_SPECULATE);
      return true;
   }
   return false;
}

static void
r300_optimize_nir(struct nir_shader *s, struct pipe_screen *screen)
{
   bool is_r500 = r300_screen(screen)->caps.is_r500;

   bool progress;
   do {
      progress = false;

      NIR_PASS_V(s, nir_lower_vars_to_ssa);

      NIR_PASS(progress, s, nir_copy_prop);
      NIR_PASS(progress, s, r300_nir_lower_flrp);
      NIR_PASS(progress, s, nir_opt_algebraic);
      if (s->info.stage == MESA_SHADER_VERTEX) {
         if (!is_r500)
            NIR_PASS(progress, s, r300_nir_lower_bool_to_float);
         NIR_PASS(progress, s, r300_nir_fuse_fround_d3d9);
      }
      NIR_PASS(progress, s, nir_opt_constant_folding);
      NIR_PASS(progress, s, nir_opt_remove_phis);
      NIR_PASS(progress, s, nir_opt_conditional_discard);
      NIR_PASS(progress, s, nir_opt_dce);
      NIR_PASS(progress, s, nir_opt_dead_cf);
      NIR_PASS(progress, s, nir_opt_cse);
      NIR_PASS(progress, s, nir_opt_find_array_copies);
      NIR_PASS(progress, s, nir_opt_copy_prop_vars);
      NIR_PASS(progress, s, nir_opt_dead_write_vars);

      NIR_PASS(progress, s, nir_opt_if, nir_opt_if_optimize_phi_true_false);
      if (is_r500)
         nir_shader_intrinsics_pass(s, set_speculate,
                                    nir_metadata_block_index |
                                    nir_metadata_dominance, NULL);
      NIR_PASS(progress, s, nir_opt_peephole_select, is_r500 ? 8 : ~0, true, true);
      NIR_PASS(progress, s, nir_opt_algebraic);
      NIR_PASS(progress, s, nir_opt_constant_folding);
      nir_load_store_vectorize_options vectorize_opts = {
         .modes = nir_var_mem_ubo,
         .callback = r300_should_vectorize_io,
         .robust_modes = 0,
      };
      NIR_PASS(progress, s, nir_opt_load_store_vectorize, &vectorize_opts);
      NIR_PASS(progress, s, nir_opt_shrink_stores, true);
      NIR_PASS(progress, s, nir_opt_shrink_vectors);
      NIR_PASS(progress, s, nir_opt_loop);
      NIR_PASS(progress, s, nir_opt_vectorize, r300_should_vectorize_instr, NULL);
      NIR_PASS(progress, s, nir_opt_undef);
      if(!progress)
         NIR_PASS(progress, s, nir_lower_undef_to_zero);
      NIR_PASS(progress, s, nir_opt_loop_unroll);

      /* Try to fold addressing math into ubo_vec4's base to avoid load_consts
       * and ALU ops for it.
       */
      nir_opt_offsets_options offset_options = {
         .ubo_vec4_max = 255,

         /* No const offset in TGSI for shared accesses. */
         .shared_max = 0,

         /* unused intrinsics */
         .uniform_max = 0,
         .buffer_max = 0,
      };

      NIR_PASS(progress, s, nir_opt_offsets, &offset_options);
   } while (progress);

   NIR_PASS_V(s, nir_lower_var_copies);
   NIR_PASS(progress, s, nir_remove_dead_variables, nir_var_function_temp,
			NULL);
}

static char *r300_check_control_flow(nir_shader *s)
{
   nir_function_impl *impl = nir_shader_get_entrypoint(s);
   nir_block *first = nir_start_block(impl);
   nir_cf_node *next = nir_cf_node_next(&first->cf_node);

   if (next) {
      switch (next->type) {
         case nir_cf_node_if:
            return "If/then statements not supported by R300/R400 shaders, should have been flattened by peephole_select.";
         case nir_cf_node_loop:
            return "Looping not supported R300/R400 shaders, all loops must be statically unrollable.";
         default:
            return "Unknown control flow type";
      }
   }

   return NULL;
}

char *
r300_finalize_nir(struct pipe_screen *pscreen, void *nir)
{
   nir_shader *s = nir;

   r300_optimize_nir(s, pscreen);

   /* st_program.c's parameter list optimization requires that future nir
    * variants don't reallocate the uniform storage, so we have to remove
    * uniforms that occupy storage.  But we don't want to remove samplers,
    * because they're needed for YUV variant lowering.
    */
   nir_remove_dead_derefs(s);
   nir_foreach_uniform_variable_safe(var, s) {
      if (var->data.mode == nir_var_uniform &&
          (glsl_type_get_image_count(var->type) ||
           glsl_type_get_sampler_count(var->type)))
         continue;

      exec_node_remove(&var->node);
   }
   nir_validate_shader(s, "after uniform var removal");

   nir_sweep(s);

   if (!r300_screen(pscreen)->caps.is_r500 &&
       (r300_screen(pscreen)->caps.has_tcl || s->info.stage == MESA_SHADER_FRAGMENT)) {
      char *msg = r300_check_control_flow(s);
      if (msg)
         return strdup(msg);
   }

   return NULL;
}
