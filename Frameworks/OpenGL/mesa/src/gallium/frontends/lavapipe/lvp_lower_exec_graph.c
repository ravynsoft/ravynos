/*
 * Copyright © 2023 Valve Corporation
 *
 * SPDX-License-Identifier: MIT
 */

#include "lvp_private.h"

#include "nir.h"
#include "nir_builder.h"

#define lvp_load_internal_field(b, bit_size, field)                                                \
   nir_load_ssbo(b, 1, bit_size, nir_imm_int(b, 0),                                                \
                 nir_imm_int(b, offsetof(struct lvp_exec_graph_internal_data, field)))

#define lvp_store_internal_field(b, value, field, scope)                                           \
   nir_store_ssbo(b, value, nir_imm_int(b, 0),                                                     \
                  nir_iadd_imm(b,                                                                  \
                               nir_imul_imm(b, nir_load_local_invocation_index(b),                 \
                                            scope == SCOPE_INVOCATION                              \
                                               ? sizeof(struct lvp_exec_graph_shader_output)       \
                                               : 0),                                               \
                               offsetof(struct lvp_exec_graph_internal_data, outputs) +            \
                                  offsetof(struct lvp_exec_graph_shader_output, field)))

static bool
lvp_lower_node_payload_deref(nir_builder *b, nir_instr *instr, void *data)
{
   if (instr->type != nir_instr_type_deref)
      return false;

   nir_deref_instr *deref = nir_instr_as_deref(instr);

   bool is_payload = nir_deref_mode_is(deref, nir_var_mem_node_payload);
   bool is_payload_in = nir_deref_mode_is(deref, nir_var_mem_node_payload_in);
   if (!is_payload && !is_payload_in)
      return false;

   deref->modes = nir_var_mem_global;

   if (deref->deref_type != nir_deref_type_var)
      return true;

   if (is_payload_in) {
      b->cursor = nir_after_instr(instr);
      nir_def *payload = lvp_load_internal_field(b, 64, payload_in);
      nir_deref_instr *cast = nir_build_deref_cast(b, payload, nir_var_mem_global, deref->type, 0);
      nir_def_rewrite_uses(&deref->def, &cast->def);
   } else {
      nir_foreach_use_safe(use, &deref->def) {
         b->cursor = nir_before_instr(nir_src_parent_instr(use));
         nir_def *payload = nir_load_var(b, deref->var);
         nir_deref_instr *cast =
            nir_build_deref_cast(b, payload, nir_var_mem_global, deref->type, 0);
         nir_src_rewrite(use, &cast->def);
      }
   }

   nir_instr_remove(instr);

   return true;
}

static bool
lvp_lower_node_payload_derefs(nir_shader *nir)
{
   return nir_shader_instructions_pass(nir, lvp_lower_node_payload_deref,
                                       nir_metadata_block_index | nir_metadata_dominance, NULL);
}

static void
lvp_build_initialize_node_payloads(nir_builder *b, nir_intrinsic_instr *intr)
{
   mesa_scope scope = nir_intrinsic_execution_scope(intr);
   assert(scope == SCOPE_INVOCATION || scope == SCOPE_WORKGROUP);

   nir_deref_instr *payloads_deref = nir_src_as_deref(intr->src[0]);
   assert(payloads_deref->deref_type == nir_deref_type_var);
   nir_variable *payloads_var = payloads_deref->var;

   nir_def *addr = lvp_load_internal_field(b, 64, payloads);
   if (scope == SCOPE_INVOCATION) {
      nir_def *payloads_offset =
         nir_imul_imm(b, nir_load_local_invocation_index(b), b->shader->info.cs.node_payloads_size);
      addr = nir_iadd(b, addr, nir_u2u64(b, payloads_offset));
   }
   nir_store_var(b, payloads_var, addr, 0x1);

   nir_def *payload_count = intr->src[1].ssa;
   lvp_store_internal_field(b, payload_count, payload_count, scope);

   nir_def *node_index = intr->src[1].ssa;
   lvp_store_internal_field(b, node_index, node_index, scope);
}

static bool
lvp_lower_node_payload_intrinsic(nir_builder *b, nir_intrinsic_instr *intr,
                                 void *data)
{
   if (intr->intrinsic == nir_intrinsic_enqueue_node_payloads) {
      nir_instr_remove(&intr->instr);
      return false;
   }

   b->cursor = nir_after_instr(&intr->instr);

   switch (intr->intrinsic) {
   case nir_intrinsic_initialize_node_payloads:
      lvp_build_initialize_node_payloads(b, intr);
      nir_instr_remove(&intr->instr);
      return true;
   case nir_intrinsic_finalize_incoming_node_payload:
      nir_def_rewrite_uses(&intr->def, nir_imm_true(b));
      nir_instr_remove(&intr->instr);
      return true;
   case nir_intrinsic_load_coalesced_input_count:
      nir_def_rewrite_uses(&intr->def, nir_imm_int(b, 1));
      nir_instr_remove(&intr->instr);
      return true;
   default:
      return false;
   }
}

static bool
lvp_lower_exec_graph_intrinsics(nir_shader *nir)
{
   return nir_shader_intrinsics_pass(nir, lvp_lower_node_payload_intrinsic,
                                       nir_metadata_block_index | nir_metadata_dominance, NULL);
}

static void
lvp_lower_node_payload_vars(struct lvp_pipeline *pipeline, nir_shader *nir)
{
   nir_foreach_variable_in_shader(var, nir) {
      if (var->data.mode != nir_var_mem_node_payload &&
          var->data.mode != nir_var_mem_node_payload_in)
         continue;

      if (var->data.mode == nir_var_mem_node_payload) {
         assert(var->data.node_name);
         assert(!pipeline->exec_graph.next_name);
         pipeline->exec_graph.next_name = var->data.node_name;
      }

      var->data.mode = nir_var_shader_temp;
      var->type = glsl_uint64_t_type();
   }
}

bool
lvp_lower_exec_graph(struct lvp_pipeline *pipeline, nir_shader *nir)
{
   bool progress = false;
   NIR_PASS(progress, nir, nir_lower_vars_to_explicit_types,
            nir_var_mem_node_payload | nir_var_mem_node_payload_in,
            glsl_get_natural_size_align_bytes);

   if (!progress)
      return false;

   /* Lower node payload variables to 64-bit addresses. */
   lvp_lower_node_payload_vars(pipeline, nir);

   /* Lower exec graph intrinsics to their actual implementation. */
   lvp_lower_exec_graph_intrinsics(nir);

   /* Lower node payloads to load/store_global intructions. */
   lvp_lower_node_payload_derefs(nir);
   NIR_PASS(_, nir, nir_lower_explicit_io, nir_var_mem_global, nir_address_format_64bit_global);

   /* Cleanup passes */
   NIR_PASS(_, nir, nir_lower_global_vars_to_local);
   NIR_PASS(_, nir, nir_lower_vars_to_ssa);
   NIR_PASS(_, nir, nir_opt_constant_folding);
   NIR_PASS(_, nir, nir_opt_dce);

   return true;
}
