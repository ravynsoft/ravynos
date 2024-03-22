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

#include "vtn_private.h"
#include "spirv_info.h"
#include "nir/nir_vla.h"
#include "util/u_debug.h"

static unsigned
glsl_type_count_function_params(const struct glsl_type *type)
{
   if (glsl_type_is_vector_or_scalar(type)) {
      return 1;
   } else if (glsl_type_is_array_or_matrix(type)) {
      return glsl_get_length(type) *
             glsl_type_count_function_params(glsl_get_array_element(type));
   } else {
      assert(glsl_type_is_struct_or_ifc(type));
      unsigned count = 0;
      unsigned elems = glsl_get_length(type);
      for (unsigned i = 0; i < elems; i++) {
         const struct glsl_type *elem_type = glsl_get_struct_field(type, i);
         count += glsl_type_count_function_params(elem_type);
      }
      return count;
   }
}

static void
glsl_type_add_to_function_params(const struct glsl_type *type,
                                 nir_function *func,
                                 unsigned *param_idx)
{
   if (glsl_type_is_vector_or_scalar(type)) {
      func->params[(*param_idx)++] = (nir_parameter) {
         .num_components = glsl_get_vector_elements(type),
         .bit_size = glsl_get_bit_size(type),
      };
   } else if (glsl_type_is_array_or_matrix(type)) {
      unsigned elems = glsl_get_length(type);
      const struct glsl_type *elem_type = glsl_get_array_element(type);
      for (unsigned i = 0; i < elems; i++)
         glsl_type_add_to_function_params(elem_type,func, param_idx);
   } else {
      assert(glsl_type_is_struct_or_ifc(type));
      unsigned elems = glsl_get_length(type);
      for (unsigned i = 0; i < elems; i++) {
         const struct glsl_type *elem_type = glsl_get_struct_field(type, i);
         glsl_type_add_to_function_params(elem_type, func, param_idx);
      }
   }
}

static void
vtn_ssa_value_add_to_call_params(struct vtn_builder *b,
                                 struct vtn_ssa_value *value,
                                 nir_call_instr *call,
                                 unsigned *param_idx)
{
   if (glsl_type_is_vector_or_scalar(value->type)) {
      call->params[(*param_idx)++] = nir_src_for_ssa(value->def);
   } else {
      unsigned elems = glsl_get_length(value->type);
      for (unsigned i = 0; i < elems; i++) {
         vtn_ssa_value_add_to_call_params(b, value->elems[i],
                                          call, param_idx);
      }
   }
}

static void
vtn_ssa_value_load_function_param(struct vtn_builder *b,
                                  struct vtn_ssa_value *value,
                                  unsigned *param_idx)
{
   if (glsl_type_is_vector_or_scalar(value->type)) {
      value->def = nir_load_param(&b->nb, (*param_idx)++);
   } else {
      unsigned elems = glsl_get_length(value->type);
      for (unsigned i = 0; i < elems; i++)
         vtn_ssa_value_load_function_param(b, value->elems[i], param_idx);
   }
}

void
vtn_handle_function_call(struct vtn_builder *b, SpvOp opcode,
                         const uint32_t *w, unsigned count)
{
   struct vtn_function *vtn_callee =
      vtn_value(b, w[3], vtn_value_type_function)->func;

   vtn_callee->referenced = true;

   nir_call_instr *call = nir_call_instr_create(b->nb.shader,
                                                vtn_callee->nir_func);

   unsigned param_idx = 0;

   nir_deref_instr *ret_deref = NULL;
   struct vtn_type *ret_type = vtn_callee->type->return_type;
   if (ret_type->base_type != vtn_base_type_void) {
      nir_variable *ret_tmp =
         nir_local_variable_create(b->nb.impl,
                                   glsl_get_bare_type(ret_type->type),
                                   "return_tmp");
      ret_deref = nir_build_deref_var(&b->nb, ret_tmp);
      call->params[param_idx++] = nir_src_for_ssa(&ret_deref->def);
   }

   for (unsigned i = 0; i < vtn_callee->type->length; i++) {
      vtn_ssa_value_add_to_call_params(b, vtn_ssa_value(b, w[4 + i]),
                                       call, &param_idx);
   }
   assert(param_idx == call->num_params);

   nir_builder_instr_insert(&b->nb, &call->instr);

   if (ret_type->base_type == vtn_base_type_void) {
      vtn_push_value(b, w[2], vtn_value_type_undef);
   } else {
      vtn_push_ssa_value(b, w[2], vtn_local_load(b, ret_deref, 0));
   }
}

static void
function_decoration_cb(struct vtn_builder *b, struct vtn_value *val, int member,
                       const struct vtn_decoration *dec, void *void_func)
{
   struct vtn_function *func = void_func;

   switch (dec->decoration) {
   case SpvDecorationLinkageAttributes: {
      unsigned name_words;
      const char *name =
         vtn_string_literal(b, dec->operands, dec->num_operands, &name_words);
      vtn_fail_if(name_words >= dec->num_operands,
                  "Malformed LinkageAttributes decoration");
      (void)name; /* TODO: What is this? */
      func->linkage = dec->operands[name_words];
      break;
   }

   default:
      break;
   }
}

bool
vtn_cfg_handle_prepass_instruction(struct vtn_builder *b, SpvOp opcode,
                                   const uint32_t *w, unsigned count)
{
   switch (opcode) {
   case SpvOpFunction: {
      vtn_assert(b->func == NULL);
      b->func = vtn_zalloc(b, struct vtn_function);

      list_inithead(&b->func->body);
      b->func->linkage = SpvLinkageTypeMax;
      b->func->control = w[3];
      list_inithead(&b->func->constructs);

      UNUSED const struct glsl_type *result_type = vtn_get_type(b, w[1])->type;
      struct vtn_value *val = vtn_push_value(b, w[2], vtn_value_type_function);
      val->func = b->func;

      vtn_foreach_decoration(b, val, function_decoration_cb, b->func);

      b->func->type = vtn_get_type(b, w[4]);
      const struct vtn_type *func_type = b->func->type;

      vtn_assert(func_type->return_type->type == result_type);

      nir_function *func =
         nir_function_create(b->shader, ralloc_strdup(b->shader, val->name));

      unsigned num_params = 0;
      for (unsigned i = 0; i < func_type->length; i++)
         num_params += glsl_type_count_function_params(func_type->params[i]->type);

      /* Add one parameter for the function return value */
      if (func_type->return_type->base_type != vtn_base_type_void)
         num_params++;

      func->should_inline = b->func->control & SpvFunctionControlInlineMask;
      func->dont_inline = b->func->control & SpvFunctionControlDontInlineMask;
      func->is_exported = b->func->linkage == SpvLinkageTypeExport;

      func->num_params = num_params;
      func->params = ralloc_array(b->shader, nir_parameter, num_params);

      unsigned idx = 0;
      if (func_type->return_type->base_type != vtn_base_type_void) {
         nir_address_format addr_format =
            vtn_mode_to_address_format(b, vtn_variable_mode_function);
         /* The return value is a regular pointer */
         func->params[idx++] = (nir_parameter) {
            .num_components = nir_address_format_num_components(addr_format),
            .bit_size = nir_address_format_bit_size(addr_format),
         };
      }

      for (unsigned i = 0; i < func_type->length; i++)
         glsl_type_add_to_function_params(func_type->params[i]->type, func, &idx);
      assert(idx == num_params);

      b->func->nir_func = func;

      /* Set up a nir_function_impl and the builder so we can load arguments
       * directly in our OpFunctionParameter handler.
       */
      nir_function_impl *impl = nir_function_impl_create(func);
      b->nb = nir_builder_at(nir_before_impl(impl));
      b->nb.exact = b->exact;

      b->func_param_idx = 0;

      /* The return value is the first parameter */
      if (func_type->return_type->base_type != vtn_base_type_void)
         b->func_param_idx++;
      break;
   }

   case SpvOpFunctionEnd:
      b->func->end = w;
      if (b->func->start_block == NULL) {
         vtn_fail_if(b->func->linkage != SpvLinkageTypeImport,
                     "A function declaration (an OpFunction with no basic "
                     "blocks), must have a Linkage Attributes Decoration "
                     "with the Import Linkage Type.");

         /* In this case, the function didn't have any actual blocks.  It's
          * just a prototype so delete the function_impl.
          */
         b->func->nir_func->impl = NULL;
      } else {
         vtn_fail_if(b->func->linkage == SpvLinkageTypeImport,
                     "A function definition (an OpFunction with basic blocks) "
                     "cannot be decorated with the Import Linkage Type.");
      }
      b->func = NULL;
      break;

   case SpvOpFunctionParameter: {
      vtn_assert(b->func_param_idx < b->func->nir_func->num_params);
      struct vtn_type *type = vtn_get_type(b, w[1]);
      struct vtn_ssa_value *value = vtn_create_ssa_value(b, type->type);
      vtn_ssa_value_load_function_param(b, value, &b->func_param_idx);
      vtn_push_ssa_value(b, w[2], value);
      break;
   }

   case SpvOpLabel: {
      vtn_assert(b->block == NULL);
      b->block = vtn_zalloc(b, struct vtn_block);
      b->block->label = w;
      vtn_push_value(b, w[1], vtn_value_type_block)->block = b->block;

      b->func->block_count++;

      if (b->func->start_block == NULL) {
         /* This is the first block encountered for this function.  In this
          * case, we set the start block and add it to the list of
          * implemented functions that we'll walk later.
          */
         b->func->start_block = b->block;
         list_addtail(&b->func->link, &b->functions);
      }
      break;
   }

   case SpvOpSelectionMerge:
   case SpvOpLoopMerge:
      vtn_assert(b->block && b->block->merge == NULL);
      b->block->merge = w;
      break;

   case SpvOpBranch:
   case SpvOpBranchConditional:
   case SpvOpSwitch:
   case SpvOpKill:
   case SpvOpTerminateInvocation:
   case SpvOpIgnoreIntersectionKHR:
   case SpvOpTerminateRayKHR:
   case SpvOpEmitMeshTasksEXT:
   case SpvOpReturn:
   case SpvOpReturnValue:
   case SpvOpUnreachable:
      if (b->wa_ignore_return_after_emit_mesh_tasks &&
          opcode == SpvOpReturn && !b->block) {
            /* At this point block was already reset by
             * SpvOpEmitMeshTasksEXT. */
            break;
      }
      vtn_assert(b->block && b->block->branch == NULL);
      b->block->branch = w;
      b->block = NULL;
      break;

   default:
      /* Continue on as per normal */
      return true;
   }

   return true;
}

/* returns the default block */
void
vtn_parse_switch(struct vtn_builder *b,
                 const uint32_t *branch,
                 struct list_head *case_list)
{
   const uint32_t *branch_end = branch + (branch[0] >> SpvWordCountShift);

   struct vtn_value *sel_val = vtn_untyped_value(b, branch[1]);
   vtn_fail_if(!sel_val->type ||
               sel_val->type->base_type != vtn_base_type_scalar,
               "Selector of OpSwitch must have a type of OpTypeInt");

   nir_alu_type sel_type =
      nir_get_nir_type_for_glsl_type(sel_val->type->type);
   vtn_fail_if(nir_alu_type_get_base_type(sel_type) != nir_type_int &&
               nir_alu_type_get_base_type(sel_type) != nir_type_uint,
               "Selector of OpSwitch must have a type of OpTypeInt");

   struct hash_table *block_to_case = _mesa_pointer_hash_table_create(b);

   bool is_default = true;
   const unsigned bitsize = nir_alu_type_get_type_size(sel_type);
   for (const uint32_t *w = branch + 2; w < branch_end;) {
      uint64_t literal = 0;
      if (!is_default) {
         if (bitsize <= 32) {
            literal = *(w++);
         } else {
            assert(bitsize == 64);
            literal = vtn_u64_literal(w);
            w += 2;
         }
      }
      struct vtn_block *case_block = vtn_block(b, *(w++));

      struct hash_entry *case_entry =
         _mesa_hash_table_search(block_to_case, case_block);

      struct vtn_case *cse;
      if (case_entry) {
         cse = case_entry->data;
      } else {
         cse = vtn_zalloc(b, struct vtn_case);
         cse->block = case_block;
         cse->block->switch_case = cse;
         util_dynarray_init(&cse->values, b);

         list_addtail(&cse->link, case_list);
         _mesa_hash_table_insert(block_to_case, case_block, cse);
      }

      if (is_default) {
         cse->is_default = true;
      } else {
         util_dynarray_append(&cse->values, uint64_t, literal);
      }

      is_default = false;
   }

   _mesa_hash_table_destroy(block_to_case, NULL);
}

void
vtn_build_cfg(struct vtn_builder *b, const uint32_t *words, const uint32_t *end)
{
   vtn_foreach_instruction(b, words, end,
                           vtn_cfg_handle_prepass_instruction);

   if (b->shader->info.stage == MESA_SHADER_KERNEL)
      return;

   vtn_build_structured_cfg(b, words, end);
}

bool
vtn_handle_phis_first_pass(struct vtn_builder *b, SpvOp opcode,
                           const uint32_t *w, unsigned count)
{
   if (opcode == SpvOpLabel)
      return true; /* Nothing to do */

   /* If this isn't a phi node, stop. */
   if (opcode != SpvOpPhi)
      return false;

   /* For handling phi nodes, we do a poor-man's out-of-ssa on the spot.
    * For each phi, we create a variable with the appropreate type and
    * do a load from that variable.  Then, in a second pass, we add
    * stores to that variable to each of the predecessor blocks.
    *
    * We could do something more intelligent here.  However, in order to
    * handle loops and things properly, we really need dominance
    * information.  It would end up basically being the into-SSA
    * algorithm all over again.  It's easier if we just let
    * lower_vars_to_ssa do that for us instead of repeating it here.
    */
   struct vtn_type *type = vtn_get_type(b, w[1]);
   nir_variable *phi_var =
      nir_local_variable_create(b->nb.impl, type->type, "phi");

   struct vtn_value *phi_val = vtn_untyped_value(b, w[2]);
   if (vtn_value_is_relaxed_precision(b, phi_val))
      phi_var->data.precision = GLSL_PRECISION_MEDIUM;

   _mesa_hash_table_insert(b->phi_table, w, phi_var);

   vtn_push_ssa_value(b, w[2],
      vtn_local_load(b, nir_build_deref_var(&b->nb, phi_var), 0));

   return true;
}

static bool
vtn_handle_phi_second_pass(struct vtn_builder *b, SpvOp opcode,
                           const uint32_t *w, unsigned count)
{
   if (opcode != SpvOpPhi)
      return true;

   struct hash_entry *phi_entry = _mesa_hash_table_search(b->phi_table, w);

   /* It's possible that this phi is in an unreachable block in which case it
    * may never have been emitted and therefore may not be in the hash table.
    * In this case, there's no var for it and it's safe to just bail.
    */
   if (phi_entry == NULL)
      return true;

   nir_variable *phi_var = phi_entry->data;

   for (unsigned i = 3; i < count; i += 2) {
      struct vtn_block *pred = vtn_block(b, w[i + 1]);

      /* If block does not have end_nop, that is because it is an unreacheable
       * block, and hence it is not worth to handle it */
      if (!pred->end_nop)
         continue;

      b->nb.cursor = nir_after_instr(&pred->end_nop->instr);

      struct vtn_ssa_value *src = vtn_ssa_value(b, w[i]);

      vtn_local_store(b, src, nir_build_deref_var(&b->nb, phi_var), 0);
   }

   return true;
}

void
vtn_emit_ret_store(struct vtn_builder *b, const struct vtn_block *block)
{
   if ((*block->branch & SpvOpCodeMask) != SpvOpReturnValue)
      return;

   vtn_fail_if(b->func->type->return_type->base_type == vtn_base_type_void,
               "Return with a value from a function returning void");
   struct vtn_ssa_value *src = vtn_ssa_value(b, block->branch[1]);
   const struct glsl_type *ret_type =
      glsl_get_bare_type(b->func->type->return_type->type);
   nir_deref_instr *ret_deref =
      nir_build_deref_cast(&b->nb, nir_load_param(&b->nb, 0),
                           nir_var_function_temp, ret_type, 0);
   vtn_local_store(b, src, ret_deref, 0);
}

static struct nir_block *
vtn_new_unstructured_block(struct vtn_builder *b, struct vtn_function *func)
{
   struct nir_block *n = nir_block_create(b->shader);
   exec_list_push_tail(&func->nir_func->impl->body, &n->cf_node.node);
   n->cf_node.parent = &func->nir_func->impl->cf_node;
   return n;
}

static void
vtn_add_unstructured_block(struct vtn_builder *b,
                           struct vtn_function *func,
                           struct list_head *work_list,
                           struct vtn_block *block)
{
   if (!block->block) {
      block->block = vtn_new_unstructured_block(b, func);
      list_addtail(&block->link, work_list);
   }
}

static void
vtn_emit_cf_func_unstructured(struct vtn_builder *b, struct vtn_function *func,
                              vtn_instruction_handler handler)
{
   struct list_head work_list;
   list_inithead(&work_list);

   func->start_block->block = nir_start_block(func->nir_func->impl);
   list_addtail(&func->start_block->link, &work_list);
   while (!list_is_empty(&work_list)) {
      struct vtn_block *block =
         list_first_entry(&work_list, struct vtn_block, link);
      list_del(&block->link);

      vtn_assert(block->block);

      const uint32_t *block_start = block->label;
      const uint32_t *block_end = block->branch;

      b->nb.cursor = nir_after_block(block->block);
      block_start = vtn_foreach_instruction(b, block_start, block_end,
                                            vtn_handle_phis_first_pass);
      vtn_foreach_instruction(b, block_start, block_end, handler);
      block->end_nop = nir_nop(&b->nb);

      SpvOp op = *block_end & SpvOpCodeMask;
      switch (op) {
      case SpvOpBranch: {
         struct vtn_block *branch_block = vtn_block(b, block->branch[1]);
         vtn_add_unstructured_block(b, func, &work_list, branch_block);
         nir_goto(&b->nb, branch_block->block);
         break;
      }

      case SpvOpBranchConditional: {
         nir_def *cond = vtn_ssa_value(b, block->branch[1])->def;
         struct vtn_block *then_block = vtn_block(b, block->branch[2]);
         struct vtn_block *else_block = vtn_block(b, block->branch[3]);

         vtn_add_unstructured_block(b, func, &work_list, then_block);
         if (then_block == else_block) {
            nir_goto(&b->nb, then_block->block);
         } else {
            vtn_add_unstructured_block(b, func, &work_list, else_block);
            nir_goto_if(&b->nb, then_block->block, nir_src_for_ssa(cond),
                                else_block->block);
         }

         break;
      }

      case SpvOpSwitch: {
         struct list_head cases;
         list_inithead(&cases);
         vtn_parse_switch(b, block->branch, &cases);

         nir_def *sel = vtn_get_nir_ssa(b, block->branch[1]);

         struct vtn_case *def = NULL;
         vtn_foreach_case(cse, &cases) {
            if (cse->is_default) {
               assert(def == NULL);
               def = cse;
               continue;
            }

            nir_def *cond = nir_imm_false(&b->nb);
            util_dynarray_foreach(&cse->values, uint64_t, val)
               cond = nir_ior(&b->nb, cond, nir_ieq_imm(&b->nb, sel, *val));

            /* block for the next check */
            nir_block *e = vtn_new_unstructured_block(b, func);
            vtn_add_unstructured_block(b, func, &work_list, cse->block);

            /* add branching */
            nir_goto_if(&b->nb, cse->block->block, nir_src_for_ssa(cond), e);
            b->nb.cursor = nir_after_block(e);
         }

         vtn_assert(def != NULL);
         vtn_add_unstructured_block(b, func, &work_list, def->block);

         /* now that all cases are handled, branch into the default block */
         nir_goto(&b->nb, def->block->block);
         break;
      }

      case SpvOpKill: {
         nir_discard(&b->nb);
         nir_goto(&b->nb, b->func->nir_func->impl->end_block);
         break;
      }

      case SpvOpUnreachable:
      case SpvOpReturn:
      case SpvOpReturnValue: {
         vtn_emit_ret_store(b, block);
         nir_goto(&b->nb, b->func->nir_func->impl->end_block);
         break;
      }

      default:
         vtn_fail("Unhandled opcode %s", spirv_op_to_string(op));
      }
   }
}

void
vtn_function_emit(struct vtn_builder *b, struct vtn_function *func,
                  vtn_instruction_handler instruction_handler)
{
   static int force_unstructured = -1;
   if (force_unstructured < 0) {
      force_unstructured =
         debug_get_bool_option("MESA_SPIRV_FORCE_UNSTRUCTURED", false);
   }

   nir_function_impl *impl = func->nir_func->impl;
   b->nb = nir_builder_at(nir_after_impl(impl));
   b->func = func;
   b->nb.exact = b->exact;
   b->phi_table = _mesa_pointer_hash_table_create(b);

   if (b->shader->info.stage == MESA_SHADER_KERNEL || force_unstructured) {
      impl->structured = false;
      vtn_emit_cf_func_unstructured(b, func, instruction_handler);
   } else {
      vtn_emit_cf_func_structured(b, func, instruction_handler);
   }

   vtn_foreach_instruction(b, func->start_block->label, func->end,
                           vtn_handle_phi_second_pass);

   if (func->nir_func->impl->structured)
      nir_copy_prop_impl(impl);
   nir_rematerialize_derefs_in_use_blocks_impl(impl);

   /*
    * There are some cases where we need to repair SSA to insert
    * the needed phi nodes:
    *
    * - Early termination instructions `OpKill` and `OpTerminateInvocation`,
    *   in NIR. They're represented by regular intrinsics with no control-flow
    *   semantics. This means that the SSA form from the SPIR-V may not
    *   100% match NIR.
    *
    * - Switches with only default case may also define SSA which may
    *   subsequently be used out of the switch.
    */
   if (func->nir_func->impl->structured)
      nir_repair_ssa_impl(impl);

   func->emitted = true;
}
