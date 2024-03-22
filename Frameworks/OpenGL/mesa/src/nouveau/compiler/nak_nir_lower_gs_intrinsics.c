/*
 * Copyright © 2023 Collabora, Ltd.
 * SPDX-License-Identifier: MIT
 */

#include "nak_private.h"
#include "nir_builder.h"
#include "nir_format_convert.h"


struct state {
   nir_builder *builder;
   nir_variable *handle_var;
   bool progress;
};

static void
rewrite_emit_vertex(nir_intrinsic_instr *intr, struct state *state)
{
   nir_builder *b = state->builder;
   const unsigned stream = nir_intrinsic_stream_id(intr);

   b->cursor = nir_before_instr(&intr->instr);

   nir_def *gs_handle = nir_load_var(b, state->handle_var);
   nir_def *gs_handle_out = nir_emit_vertex_nv(b, 32, gs_handle, stream);

   nir_store_var(b, state->handle_var, gs_handle_out, 0x1);

   nir_instr_remove(&intr->instr);

   state->progress = true;
}

static void
rewrite_end_primitive(nir_intrinsic_instr *intr, struct state *state)
{
   nir_builder *b = state->builder;
   const unsigned stream = nir_intrinsic_stream_id(intr);

   b->cursor = nir_before_instr(&intr->instr);

   nir_def *gs_handle = nir_load_var(b, state->handle_var);
   nir_def *gs_handle_out = nir_end_primitive_nv(b, 32, gs_handle, stream);

   nir_store_var(b, state->handle_var, gs_handle_out, 0x1);

   nir_instr_remove(&intr->instr);

   state->progress = true;
}

static void
rewrite_ast_nv(nir_intrinsic_instr *intr, struct state *state)
{
   nir_builder *b = state->builder;

   b->cursor = nir_before_instr(&intr->instr);

   nir_src *vtx = &intr->src[1];
   nir_def *gs_handle = nir_load_var(b, state->handle_var);
   nir_src_rewrite(vtx, gs_handle);

   state->progress = true;
}

static void
append_final_primitive_nv(nir_block *end_block, struct state *state)
{
   nir_builder *b = state->builder;

   set_foreach(end_block->predecessors, entry) {
      nir_block *pred = (nir_block *)entry->key;
      b->cursor = nir_after_block_before_jump(pred);

      nir_def *gs_handle = nir_load_var(b, state->handle_var);
      nir_final_primitive_nv(b, gs_handle);

      state->progress = true;
   }
}

bool
nak_nir_lower_gs_intrinsics(nir_shader *nir)
{
   struct state state;

   state.progress = false;
   nir_function_impl *impl = nir_shader_get_entrypoint(nir);
   nir_builder builder = nir_builder_at(nir_before_impl(impl));

   state.builder = &builder;

   state.handle_var = nir_local_variable_create(impl, glsl_uint_type(), "gs_handle");
   nir_store_var(&builder, state.handle_var, nir_imm_int(&builder, 0), 0x1);

   nir_foreach_block_safe(block, impl) {
      nir_foreach_instr_safe(instr, block) {
         if (instr->type != nir_instr_type_intrinsic)
            continue;

         nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);

         switch (intr->intrinsic) {
         case nir_intrinsic_emit_vertex:
            rewrite_emit_vertex(intr, &state);
            break;
         case nir_intrinsic_end_primitive:
            rewrite_end_primitive(intr, &state);
            break;
         case nir_intrinsic_ast_nv:
            rewrite_ast_nv(intr, &state);
            break;
         default:
            break;
         }
      }
   }

   append_final_primitive_nv(impl->end_block, &state);

   nir_metadata_preserve(impl, nir_metadata_none);

   return state.progress;
}