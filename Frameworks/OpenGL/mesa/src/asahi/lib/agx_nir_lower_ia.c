/*
 * Copyright 2023 Valve Corporation
 * SPDX-License-Identifier: MIT
 */

#include "asahi/compiler/agx_compile.h"
#include "compiler/nir/nir_builder.h"
#include "shaders/geometry.h"
#include "util/compiler.h"
#include "agx_nir_lower_gs.h"
#include "libagx_shaders.h"
#include "nir.h"
#include "nir_builder_opcodes.h"
#include "nir_intrinsics.h"

/*
 * This file implements input assembly in software for geometry/tessellation
 * shaders. load_vertex_id is lowered based on the topology. Most of the logic
 * lives in CL library routines.
 *
 * When geom/tess is used, multidraw indirect is implemented by:
 *
 * 1. Prefix summing the vertex counts across draws.
 * 2. Issuing a single indirect draw for the summed vertices.
 * 3. Binary searching the prefix sum buffer in software index fetch.
 *
 * This multidraw implementation kicks off the prefix sum and lowered draw.
 */

static nir_def *
load_vertex_id(nir_builder *b, struct agx_ia_key *key)
{
   /* Tessellate by primitive mode */
   nir_def *id = libagx_vertex_id_for_topology(
      b, nir_imm_int(b, key->mode), nir_imm_bool(b, key->flatshade_first),
      nir_load_primitive_id(b), nir_load_vertex_id_in_primitive_agx(b),
      nir_load_num_vertices(b));

   /* If drawing with an index buffer, pull the vertex ID. Otherwise, the
    * vertex ID is just the index as-is.
    */
   if (key->index_size) {
      nir_def *ia = nir_load_input_assembly_buffer_agx(b);

      /*
       * For multidraw, apply the index buffer offset. For !multidraw, this is
       * handled ahead-of-time and baked into the index buffer pointer.
       */
      if (key->indirect_multidraw) {
         nir_def *first = libagx_multidraw_param(b, ia, nir_load_draw_id(b),
                                                 nir_imm_int(b, 2));
         id = nir_iadd(b, id, first);
      }

      nir_def *address =
         libagx_index_buffer(b, ia, id, nir_imm_int(b, key->index_size));

      nir_def *index = nir_load_global_constant(b, address, key->index_size, 1,
                                                key->index_size * 8);

      id = nir_u2uN(b, index, id->bit_size);
   }

   /* Add the "start", either an index bias or a base vertex. This must happen
    * after indexing for proper index bias behaviour.
    */
   return nir_iadd(b, id, nir_load_first_vertex(b));
}

static bool
lower_vertex_id(nir_builder *b, nir_intrinsic_instr *intr, void *data)
{
   if (intr->intrinsic != nir_intrinsic_load_vertex_id)
      return false;

   b->cursor = nir_instr_remove(&intr->instr);
   assert(intr->def.bit_size == 32);
   nir_def_rewrite_uses(&intr->def, load_vertex_id(b, data));
   return true;
}

void
agx_nir_lower_ia(nir_shader *s, struct agx_ia_key *key)
{
   nir_shader_intrinsics_pass(s, lower_vertex_id,
                              nir_metadata_block_index | nir_metadata_dominance,
                              key);
}

struct multidraw_state {
   nir_def *raw_id, *draw, *primitive, *first_vertex, *base_instance;
   nir_def *num_vertices;

   bool indexed;
};

static nir_def *
map_multidraw_param(nir_builder *b, nir_intrinsic_op intrin,
                    struct multidraw_state *state)
{
   switch (intrin) {
   case nir_intrinsic_load_draw_id:
      return state->draw;

   case nir_intrinsic_load_primitive_id:
      return state->primitive;

   case nir_intrinsic_load_base_vertex:
      return state->indexed ? state->first_vertex : nir_imm_int(b, 0);

   case nir_intrinsic_load_first_vertex:
      return state->first_vertex;

   case nir_intrinsic_load_base_instance:
      return state->base_instance;

   case nir_intrinsic_load_num_vertices:
      return state->num_vertices;

   default:
      return NULL;
   }
}

static bool
lower_multidraw(nir_builder *b, nir_intrinsic_instr *intr, void *data)
{
   b->cursor = nir_before_instr(&intr->instr);
   nir_def *id = map_multidraw_param(b, intr->intrinsic, data);
   if (!id)
      return false;

   nir_instr_remove(&intr->instr);
   nir_def_rewrite_uses(&intr->def, id);
   return true;
}

void
agx_nir_lower_multidraw(nir_shader *s, struct agx_ia_key *key)
{
   assert(key->indirect_multidraw);

   nir_builder b_ =
      nir_builder_at(nir_before_impl(nir_shader_get_entrypoint(s)));
   nir_builder *b = &b_;

   struct multidraw_state state = {
      /* Filled in at the end to avoid recursion */
      .raw_id = nir_undef(b, 1, 32),
      .indexed = key->index_size > 0,
   };

   nir_def *ia = nir_load_input_assembly_buffer_agx(b);
   state.draw = libagx_multidraw_draw_id(b, ia, state.raw_id);

   state.primitive = libagx_multidraw_primitive_id(
      b, ia, state.draw, state.raw_id, nir_imm_int(b, key->mode));

   state.num_vertices =
      libagx_multidraw_param(b, ia, state.draw, nir_imm_int(b, 0));

   state.first_vertex = libagx_multidraw_param(
      b, ia, state.draw, nir_imm_int(b, state.indexed ? 3 : 2));

   state.base_instance = libagx_multidraw_param(
      b, ia, state.draw, nir_imm_int(b, state.indexed ? 4 : 3));

   nir_shader_intrinsics_pass(b->shader, lower_multidraw,
                              nir_metadata_block_index | nir_metadata_dominance,
                              &state);

   b->cursor = nir_before_impl(b->impl);
   nir_def_rewrite_uses(state.raw_id, nir_load_primitive_id(b));
}
