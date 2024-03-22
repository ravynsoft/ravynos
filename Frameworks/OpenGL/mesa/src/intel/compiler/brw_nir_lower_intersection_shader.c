/*
 * Copyright (c) 2020 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "brw_nir_rt.h"
#include "brw_nir_rt_builder.h"

static nir_function_impl *
lower_any_hit_for_intersection(nir_shader *any_hit)
{
   nir_function_impl *impl = nir_shader_get_entrypoint(any_hit);

   /* Any-hit shaders need three parameters */
   assert(impl->function->num_params == 0);
   nir_parameter params[] = {
      {
         /* A pointer to a boolean value for whether or not the hit was
          * accepted.
          */
         .num_components = 1,
         .bit_size = 32,
      },
      {
         /* The hit T value */
         .num_components = 1,
         .bit_size = 32,
      },
      {
         /* The hit kind */
         .num_components = 1,
         .bit_size = 32,
      },
   };
   impl->function->num_params = ARRAY_SIZE(params);
   impl->function->params =
      ralloc_array(any_hit, nir_parameter, ARRAY_SIZE(params));
   memcpy(impl->function->params, params, sizeof(params));

   nir_builder build = nir_builder_at(nir_before_impl(impl));
   nir_builder *b = &build;

   nir_def *commit_ptr = nir_load_param(b, 0);
   nir_def *hit_t = nir_load_param(b, 1);
   nir_def *hit_kind = nir_load_param(b, 2);

   nir_deref_instr *commit =
      nir_build_deref_cast(b, commit_ptr, nir_var_function_temp,
                           glsl_bool_type(), 0);

   nir_foreach_block_safe(block, impl) {
      nir_foreach_instr_safe(instr, block) {
         switch (instr->type) {
         case nir_instr_type_intrinsic: {
            nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
            switch (intrin->intrinsic) {
            case nir_intrinsic_ignore_ray_intersection:
               b->cursor = nir_instr_remove(&intrin->instr);
               /* We put the newly emitted code inside a dummy if because it's
                * going to contain a jump instruction and we don't want to
                * deal with that mess here.  It'll get dealt with by our
                * control-flow optimization passes.
                */
               nir_store_deref(b, commit, nir_imm_false(b), 0x1);
               nir_push_if(b, nir_imm_true(b));
               nir_jump(b, nir_jump_return);
               nir_pop_if(b, NULL);
               break;

            case nir_intrinsic_terminate_ray:
               /* The "normal" handling of terminateRay works fine in
                * intersection shaders.
                */
               break;

            case nir_intrinsic_load_ray_t_max:
               nir_def_rewrite_uses(&intrin->def,
                                        hit_t);
               nir_instr_remove(&intrin->instr);
               break;

            case nir_intrinsic_load_ray_hit_kind:
               nir_def_rewrite_uses(&intrin->def,
                                        hit_kind);
               nir_instr_remove(&intrin->instr);
               break;

            default:
               break;
            }
            break;
         }

         case nir_instr_type_jump: {
            /* Stomp any halts to returns since they only return from the
             * any-hit shader and not necessarily from the intersection
             * shader.  This is safe to do because we've already asserted
             * that we only have the one function.
             */
            nir_jump_instr *jump = nir_instr_as_jump(instr);
            if (jump->type == nir_jump_halt)
               jump->type = nir_jump_return;
            break;
         }

         default:
            break;
         }
      }
   }

   nir_validate_shader(any_hit, "after initial any-hit lowering");

   nir_lower_returns_impl(impl);

   nir_validate_shader(any_hit, "after lowering returns");

   return impl;
}

void
brw_nir_lower_intersection_shader(nir_shader *intersection,
                                  const nir_shader *any_hit,
                                  const struct intel_device_info *devinfo)
{
   void *dead_ctx = ralloc_context(intersection);

   nir_function_impl *any_hit_impl = NULL;
   struct hash_table *any_hit_var_remap = NULL;
   if (any_hit) {
      nir_shader *any_hit_tmp = nir_shader_clone(dead_ctx, any_hit);
      NIR_PASS_V(any_hit_tmp, nir_opt_dce);
      any_hit_impl = lower_any_hit_for_intersection(any_hit_tmp);
      any_hit_var_remap = _mesa_pointer_hash_table_create(dead_ctx);
   }

   nir_function_impl *impl = nir_shader_get_entrypoint(intersection);

   nir_builder build = nir_builder_at(nir_before_impl(impl));
   nir_builder *b = &build;

   nir_def *t_addr = brw_nir_rt_mem_hit_addr(b, false /* committed */);
   nir_variable *commit =
      nir_local_variable_create(impl, glsl_bool_type(), "ray_commit");
   nir_store_var(b, commit, nir_imm_false(b), 0x1);

   assert(impl->end_block->predecessors->entries == 1);
   set_foreach(impl->end_block->predecessors, block_entry) {
      struct nir_block *block = (void *)block_entry->key;
      b->cursor = nir_after_block_before_jump(block);
      nir_push_if(b, nir_load_var(b, commit));
      {
         /* Set the "valid" bit in mem_hit */
         nir_def *ray_addr = brw_nir_rt_mem_hit_addr(b, false /* committed */);
         nir_def *flags_dw_addr = nir_iadd_imm(b, ray_addr, 12);
         nir_store_global(b, flags_dw_addr, 4,
            nir_ior(b, nir_load_global(b, flags_dw_addr, 4, 1, 32),
                       nir_imm_int(b, 1 << 16)), 0x1 /* write_mask */);

         nir_accept_ray_intersection(b);
      }
      nir_push_else(b, NULL);
      {
         nir_ignore_ray_intersection(b);
      }
      nir_pop_if(b, NULL);
      break;
   }

   nir_foreach_block_safe(block, impl) {
      nir_foreach_instr_safe(instr, block) {
         switch (instr->type) {
         case nir_instr_type_intrinsic: {
            nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
            switch (intrin->intrinsic) {
            case nir_intrinsic_report_ray_intersection: {
               b->cursor = nir_instr_remove(&intrin->instr);
               nir_def *hit_t = intrin->src[0].ssa;
               nir_def *hit_kind = intrin->src[1].ssa;
               nir_def *min_t = nir_load_ray_t_min(b);

               struct brw_nir_rt_mem_ray_defs ray_def;
               brw_nir_rt_load_mem_ray(b, &ray_def, BRW_RT_BVH_LEVEL_WORLD);

               struct brw_nir_rt_mem_hit_defs hit_in = {};
               brw_nir_rt_load_mem_hit(b, &hit_in, false);

               nir_def *max_t = ray_def.t_far;

               /* bool commit_tmp = false; */
               nir_variable *commit_tmp =
                  nir_local_variable_create(impl, glsl_bool_type(),
                                            "commit_tmp");
               nir_store_var(b, commit_tmp, nir_imm_false(b), 0x1);

               nir_push_if(b, nir_iand(b, nir_fge(b, hit_t, min_t),
                                          nir_fge(b, max_t, hit_t)));
               {
                  /* Any-hit defaults to commit */
                  nir_store_var(b, commit_tmp, nir_imm_true(b), 0x1);

                  if (any_hit_impl != NULL) {
                     nir_push_if(b, nir_inot(b, nir_load_leaf_opaque_intel(b)));
                     {
                        nir_def *params[] = {
                           &nir_build_deref_var(b, commit_tmp)->def,
                           hit_t,
                           hit_kind,
                        };
                        nir_inline_function_impl(b, any_hit_impl, params,
                                                 any_hit_var_remap);
                     }
                     nir_pop_if(b, NULL);
                  }

                  nir_push_if(b, nir_load_var(b, commit_tmp));
                  {
                     nir_store_var(b, commit, nir_imm_true(b), 0x1);

                     nir_def *ray_addr =
                        brw_nir_rt_mem_ray_addr(b, brw_nir_rt_stack_addr(b), BRW_RT_BVH_LEVEL_WORLD);

                     nir_store_global(b, nir_iadd_imm(b, ray_addr, 16 + 12), 4,  hit_t, 0x1);
                     nir_store_global(b, t_addr, 4,
                                      nir_vec2(b, nir_fmin(b, hit_t, hit_in.t), hit_kind),
                                      0x3);
                  }
                  nir_pop_if(b, NULL);
               }
               nir_pop_if(b, NULL);

               nir_def *accepted = nir_load_var(b, commit_tmp);
               nir_def_rewrite_uses(&intrin->def,
                                        accepted);
               break;
            }

            default:
               break;
            }
            break;
         }

         default:
            break;
         }
      }
   }
   nir_metadata_preserve(impl, nir_metadata_none);

   /* We did some inlining; have to re-index SSA defs */
   nir_index_ssa_defs(impl);

   ralloc_free(dead_ctx);
}
