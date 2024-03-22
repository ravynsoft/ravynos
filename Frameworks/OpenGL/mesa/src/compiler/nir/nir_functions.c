/*
 * Copyright © 2015 Intel Corporation
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

#include "nir.h"
#include "nir_builder.h"
#include "nir_control_flow.h"
#include "nir_vla.h"

/*
 * TODO: write a proper inliner for GPUs.
 * This heuristic just inlines small functions,
 * and tail calls get inlined as well.
 */
static bool
nir_function_can_inline(nir_function *function)
{
   bool can_inline = true;
   if (!function->should_inline) {
      if (function->impl) {
         if (function->impl->num_blocks > 2)
            can_inline = false;
         if (function->impl->ssa_alloc > 45)
            can_inline = false;
      }
   }
   return can_inline;
}

static bool
function_ends_in_jump(nir_function_impl *impl)
{
   nir_block *last_block = nir_impl_last_block(impl);
   return nir_block_ends_in_jump(last_block);
}

void
nir_inline_function_impl(struct nir_builder *b,
                         const nir_function_impl *impl,
                         nir_def **params,
                         struct hash_table *shader_var_remap)
{
   nir_function_impl *copy = nir_function_impl_clone(b->shader, impl);

   exec_list_append(&b->impl->locals, &copy->locals);

   nir_foreach_block(block, copy) {
      nir_foreach_instr_safe(instr, block) {
         switch (instr->type) {
         case nir_instr_type_deref: {
            nir_deref_instr *deref = nir_instr_as_deref(instr);
            if (deref->deref_type != nir_deref_type_var)
               break;

            /* We don't need to remap function variables.  We already cloned
             * them as part of nir_function_impl_clone and appended them to
             * b->impl->locals.
             */
            if (deref->var->data.mode == nir_var_function_temp)
               break;

            /* If no map is provided, we assume that there are either no
             * shader variables or they already live b->shader (this is the
             * case for function inlining within a single shader.
             */
            if (shader_var_remap == NULL)
               break;

            struct hash_entry *entry =
               _mesa_hash_table_search(shader_var_remap, deref->var);
            if (entry == NULL) {
               nir_variable *nvar = nir_variable_clone(deref->var, b->shader);
               nir_shader_add_variable(b->shader, nvar);
               entry = _mesa_hash_table_insert(shader_var_remap,
                                               deref->var, nvar);
            }
            deref->var = entry->data;
            break;
         }

         case nir_instr_type_intrinsic: {
            nir_intrinsic_instr *load = nir_instr_as_intrinsic(instr);
            if (load->intrinsic != nir_intrinsic_load_param)
               break;

            unsigned param_idx = nir_intrinsic_param_idx(load);
            assert(param_idx < impl->function->num_params);
            nir_def_rewrite_uses(&load->def,
                                 params[param_idx]);

            /* Remove any left-over load_param intrinsics because they're soon
             * to be in another function and therefore no longer valid.
             */
            nir_instr_remove(&load->instr);
            break;
         }

         case nir_instr_type_jump:
            /* Returns have to be lowered for this to work */
            assert(nir_instr_as_jump(instr)->type != nir_jump_return);
            break;

         default:
            break;
         }
      }
   }

   bool nest_if = function_ends_in_jump(copy);

   /* Pluck the body out of the function and place it here */
   nir_cf_list body;
   nir_cf_list_extract(&body, &copy->body);

   if (nest_if) {
      nir_if *cf = nir_push_if(b, nir_imm_true(b));
      nir_cf_reinsert(&body, nir_after_cf_list(&cf->then_list));
      nir_pop_if(b, cf);
   } else {
      /* Insert a nop at the cursor so we can keep track of where things are as
       * we add/remove stuff from the CFG.
       */
      nir_intrinsic_instr *nop = nir_nop(b);
      nir_cf_reinsert(&body, nir_before_instr(&nop->instr));
      b->cursor = nir_instr_remove(&nop->instr);
   }
}

static bool inline_function_impl(nir_function_impl *impl, struct set *inlined);

static bool inline_functions_pass(nir_builder *b,
                                  nir_instr *instr,
                                  void *cb_data)
{
   struct set *inlined = cb_data;
   if (instr->type != nir_instr_type_call)
      return false;

   nir_call_instr *call = nir_instr_as_call(instr);
   assert(call->callee->impl);

   if (b->shader->options->driver_functions &&
       b->shader->info.stage == MESA_SHADER_KERNEL) {
      bool last_instr = (instr == nir_block_last_instr(instr->block));
      if (!nir_function_can_inline(call->callee) && !last_instr) {
         return false;
      }
   }

   /* Make sure that the function we're calling is already inlined */
   inline_function_impl(call->callee->impl, inlined);

   b->cursor = nir_instr_remove(&call->instr);

   /* Rewrite all of the uses of the callee's parameters to use the call
    * instructions sources.  In order to ensure that the "load" happens
    * here and not later (for register sources), we make sure to convert it
    * to an SSA value first.
    */
   const unsigned num_params = call->num_params;
   NIR_VLA(nir_def *, params, num_params);
   for (unsigned i = 0; i < num_params; i++) {
      params[i] = call->params[i].ssa;
   }

   nir_inline_function_impl(b, call->callee->impl, params, NULL);
   return true;
}

static bool
inline_function_impl(nir_function_impl *impl, struct set *inlined)
{
   if (_mesa_set_search(inlined, impl))
      return false; /* Already inlined */

   bool progress;
   progress = nir_function_instructions_pass(impl, inline_functions_pass,
                                             nir_metadata_none, inlined);
   if (progress) {
      /* Indices are completely messed up now */
      nir_index_ssa_defs(impl);
   }

   _mesa_set_add(inlined, impl);

   return progress;
}

/** A pass to inline all functions in a shader into their callers
 *
 * For most use-cases, function inlining is a multi-step process.  The general
 * pattern employed by SPIR-V consumers and others is as follows:
 *
 *  1. nir_lower_variable_initializers(shader, nir_var_function_temp)
 *
 *     This is needed because local variables from the callee are simply added
 *     to the locals list for the caller and the information about where the
 *     constant initializer logically happens is lost.  If the callee is
 *     called in a loop, this can cause the variable to go from being
 *     initialized once per loop iteration to being initialized once at the
 *     top of the caller and values to persist from one invocation of the
 *     callee to the next.  The simple solution to this problem is to get rid
 *     of constant initializers before function inlining.
 *
 *  2. nir_lower_returns(shader)
 *
 *     nir_inline_functions assumes that all functions end "naturally" by
 *     execution reaching the end of the function without any return
 *     instructions causing instant jumps to the end.  Thanks to NIR being
 *     structured, we can't represent arbitrary jumps to various points in the
 *     program which is what an early return in the callee would have to turn
 *     into when we inline it into the caller.  Instead, we require returns to
 *     be lowered which lets us just copy+paste the callee directly into the
 *     caller.
 *
 *  3. nir_inline_functions(shader)
 *
 *     This does the actual function inlining and the resulting shader will
 *     contain no call instructions.
 *
 *  4. nir_opt_deref(shader)
 *
 *     Most functions contain pointer parameters where the result of a deref
 *     instruction is passed in as a parameter, loaded via a load_param
 *     intrinsic, and then turned back into a deref via a cast.  Function
 *     inlining will get rid of the load_param but we are still left with a
 *     cast.  Running nir_opt_deref gets rid of the intermediate cast and
 *     results in a whole deref chain again.  This is currently required by a
 *     number of optimizations and lowering passes at least for certain
 *     variable modes.
 *
 *  5. Loop over the functions and delete all but the main entrypoint.
 *
 *     In the Intel Vulkan driver this looks like this:
 *
 *        nir_remove_non_entrypoints(nir);
 *
 *    While nir_inline_functions does get rid of all call instructions, it
 *    doesn't get rid of any functions because it doesn't know what the "root
 *    function" is.  Instead, it's up to the individual driver to know how to
 *    decide on a root function and delete the rest.  With SPIR-V,
 *    spirv_to_nir returns the root function and so we can just use == whereas
 *    with GL, you may have to look for a function named "main".
 *
 *  6. nir_lower_variable_initializers(shader, ~nir_var_function_temp)
 *
 *     Lowering constant initializers on inputs, outputs, global variables,
 *     etc. requires that we know the main entrypoint so that we know where to
 *     initialize them.  Otherwise, we would have to assume that anything
 *     could be a main entrypoint and initialize them at the start of every
 *     function but that would clearly be wrong if any of those functions were
 *     ever called within another function.  Simply requiring a single-
 *     entrypoint function shader is the best way to make it well-defined.
 */
bool
nir_inline_functions(nir_shader *shader)
{
   struct set *inlined = _mesa_pointer_set_create(NULL);
   bool progress = false;

   nir_foreach_function_impl(impl, shader) {
      progress = inline_function_impl(impl, inlined) || progress;
   }

   _mesa_set_destroy(inlined, NULL);

   return progress;
}

struct lower_link_state {
   struct hash_table *shader_var_remap;
   const nir_shader *link_shader;
   unsigned printf_index_offset;
};

static bool
lower_calls_vars_instr(struct nir_builder *b,
                       nir_instr *instr,
                       void *cb_data)
{
   struct lower_link_state *state = cb_data;

   switch (instr->type) {
   case nir_instr_type_deref: {
      nir_deref_instr *deref = nir_instr_as_deref(instr);
      if (deref->deref_type != nir_deref_type_var)
         return false;
      if (deref->var->data.mode == nir_var_function_temp)
         return false;

      assert(state->shader_var_remap);
      struct hash_entry *entry =
         _mesa_hash_table_search(state->shader_var_remap, deref->var);
      if (entry == NULL) {
         nir_variable *nvar = nir_variable_clone(deref->var, b->shader);
         nir_shader_add_variable(b->shader, nvar);
         entry = _mesa_hash_table_insert(state->shader_var_remap,
                                         deref->var, nvar);
      }
      deref->var = entry->data;
      break;
   }
   case nir_instr_type_call: {
      nir_call_instr *ncall = nir_instr_as_call(instr);
      if (!ncall->callee->name)
         return false;

      nir_function *func = nir_shader_get_function_for_name(b->shader, ncall->callee->name);
      if (func) {
         ncall->callee = func;
         break;
      }

      nir_function *new_func;
      new_func = nir_shader_get_function_for_name(state->link_shader, ncall->callee->name);
      if (new_func)
         ncall->callee = nir_function_clone(b->shader, new_func);
      break;
   }
   case nir_instr_type_intrinsic: {
      /* Reindex the offset of the printf intrinsic by the number of already
       * present printfs in the shader where functions are linked into.
       */
      if (state->printf_index_offset == 0)
         return false;

      nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
      if (intrin->intrinsic != nir_intrinsic_printf)
         return false;

      b->cursor = nir_before_instr(instr);
      nir_src_rewrite(&intrin->src[0],
                      nir_iadd_imm(b, intrin->src[0].ssa,
                                      state->printf_index_offset));
      break;
   }
   default:
      break;
   }
   return true;
}

static bool
lower_call_function_impl(struct nir_builder *b,
                         nir_function *callee,
                         const nir_function_impl *impl,
                         struct lower_link_state *state)
{
   nir_function_impl *copy = nir_function_impl_clone(b->shader, impl);
   copy->function = callee;
   callee->impl = copy;

   return nir_function_instructions_pass(copy,
                                         lower_calls_vars_instr,
                                         nir_metadata_none,
                                         state);
}

static bool
function_link_pass(struct nir_builder *b,
                   nir_instr *instr,
                   void *cb_data)
{
   struct lower_link_state *state = cb_data;

   if (instr->type != nir_instr_type_call)
      return false;

   nir_call_instr *call = nir_instr_as_call(instr);
   nir_function *func = NULL;

   if (!call->callee->name)
      return false;

   if (call->callee->impl)
      return false;

   func = nir_shader_get_function_for_name(state->link_shader, call->callee->name);
   if (!func || !func->impl) {
      return false;
   }
   return lower_call_function_impl(b, call->callee,
                                   func->impl,
                                   state);
}

bool
nir_link_shader_functions(nir_shader *shader,
                          const nir_shader *link_shader)
{
   void *ra_ctx = ralloc_context(NULL);
   struct hash_table *copy_vars = _mesa_pointer_hash_table_create(ra_ctx);
   bool progress = false, overall_progress = false;

   struct lower_link_state state = {
      .shader_var_remap = copy_vars,
      .link_shader = link_shader,
      .printf_index_offset = shader->printf_info_count,
   };
   /* do progress passes inside the pass */
   do {
      progress = false;
      nir_foreach_function_impl(impl, shader) {
         bool this_progress = nir_function_instructions_pass(impl,
                                                             function_link_pass,
                                                             nir_metadata_none,
                                                             &state);
         if (this_progress)
            nir_index_ssa_defs(impl);
         progress |= this_progress;
      }
      overall_progress |= progress;
   } while (progress);

   if (overall_progress && link_shader->printf_info_count > 0) {
      shader->printf_info = reralloc(shader, shader->printf_info,
                                     u_printf_info,
                                     shader->printf_info_count +
                                     link_shader->printf_info_count);

      for (unsigned i = 0; i < link_shader->printf_info_count; i++){
         const u_printf_info *src_info = &link_shader->printf_info[i];
         u_printf_info *dst_info = &shader->printf_info[shader->printf_info_count++];

         dst_info->num_args = src_info->num_args;
         dst_info->arg_sizes = ralloc_array(shader, unsigned, dst_info->num_args);
         memcpy(dst_info->arg_sizes, src_info->arg_sizes,
                sizeof(dst_info->arg_sizes[0]) * dst_info->num_args);

         dst_info->string_size = src_info->string_size;
         dst_info->strings = ralloc_size(shader, dst_info->string_size);
         memcpy(dst_info->strings, src_info->strings, dst_info->string_size);
      }
   }

   ralloc_free(ra_ctx);

   return overall_progress;
}

static void
nir_mark_used_functions(struct nir_function *func, struct set *used_funcs);

static bool mark_used_pass_cb(struct nir_builder *b,
                              nir_instr *instr, void *data)
{
   struct set *used_funcs = data;
   if (instr->type != nir_instr_type_call)
      return false;
   nir_call_instr *call = nir_instr_as_call(instr);

   _mesa_set_add(used_funcs, call->callee);

   nir_mark_used_functions(call->callee, used_funcs);
   return true;
}

static void
nir_mark_used_functions(struct nir_function *func, struct set *used_funcs)
{
   if (func->impl) {
      nir_function_instructions_pass(func->impl,
                                     mark_used_pass_cb,
                                     nir_metadata_none,
                                     used_funcs);
   }
}

void
nir_cleanup_functions(nir_shader *nir)
{
   if (!nir->options->driver_functions) {
      nir_remove_non_entrypoints(nir);
      return;
   }

   struct set *used_funcs = _mesa_set_create(NULL, _mesa_hash_pointer,
                                             _mesa_key_pointer_equal);
   foreach_list_typed_safe(nir_function, func, node, &nir->functions) {
      if (func->is_entrypoint) {
         _mesa_set_add(used_funcs, func);
         nir_mark_used_functions(func, used_funcs);
      }
   }
   foreach_list_typed_safe(nir_function, func, node, &nir->functions) {
      if (!_mesa_set_search(used_funcs, func))
         exec_node_remove(&func->node);
   }
   _mesa_set_destroy(used_funcs, NULL);
}
