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
#include "nir_xfb_info.h"

/**
 * \file nir_lower_gs_intrinsics.c
 *
 * Geometry Shaders can call EmitVertex()/EmitStreamVertex() to output an
 * arbitrary number of vertices.  However, the shader must declare the maximum
 * number of vertices that it will ever output - further attempts to emit
 * vertices result in undefined behavior according to the GLSL specification.
 *
 * Drivers might use this maximum number of vertices to allocate enough space
 * to hold the geometry shader's output.  Some drivers (such as i965) need to
 * implement "safety checks" which ensure that the shader hasn't emitted too
 * many vertices, to avoid overflowing that space and trashing other memory.
 *
 * The count of emitted vertices can also be useful in buffer offset
 * calculations, so drivers know where to write the GS output.
 *
 * However, for simple geometry shaders that emit a statically determinable
 * number of vertices, this extra bookkeeping is unnecessary and inefficient.
 * By tracking the vertex count in NIR, we allow constant folding/propagation
 * and dead control flow optimizations to eliminate most of it where possible.
 *
 * This pass introduces a new global variable which stores the current vertex
 * count (initialized to 0), and converts emit_vertex/end_primitive intrinsics
 * to their *_with_counter variants.  emit_vertex is also wrapped in a safety
 * check to avoid buffer overflows.  Finally, it adds a set_vertex_count
 * intrinsic at the end of the program, informing the driver of the final
 * vertex count.
 */

struct state {
   nir_builder *builder;
   nir_variable *vertex_count_vars[NIR_MAX_XFB_STREAMS];
   nir_variable *vtxcnt_per_prim_vars[NIR_MAX_XFB_STREAMS];
   nir_variable *primitive_count_vars[NIR_MAX_XFB_STREAMS];
   nir_variable *decomposed_primitive_count_vars[NIR_MAX_XFB_STREAMS];
   bool per_stream;
   bool count_prims;
   bool count_vtx_per_prim;
   bool count_decomposed_prims;
   bool overwrite_incomplete;
   bool is_points;
   bool progress;
};

/**
 * Replace emit_vertex intrinsics with:
 *
 * if (vertex_count < max_vertices) {
 *    emit_vertex_with_counter vertex_count, vertex_count_per_primitive (optional) ...
 *    vertex_count += 1
 *    vertex_count_per_primitive += 1
 * }
 */
static void
rewrite_emit_vertex(nir_intrinsic_instr *intrin, struct state *state)
{
   nir_builder *b = state->builder;
   unsigned stream = nir_intrinsic_stream_id(intrin);

   /* Load the vertex count */
   b->cursor = nir_before_instr(&intrin->instr);
   assert(state->vertex_count_vars[stream] != NULL);
   nir_def *count = nir_load_var(b, state->vertex_count_vars[stream]);
   nir_def *count_per_primitive;
   nir_def *primitive_count;
   nir_def *decomposed_primitive_count;

   if (state->count_vtx_per_prim)
      count_per_primitive = nir_load_var(b, state->vtxcnt_per_prim_vars[stream]);
   else if (state->is_points)
      count_per_primitive = nir_imm_int(b, 0);
   else
      count_per_primitive = nir_undef(b, 1, 32);

   if (state->count_prims)
      primitive_count = nir_load_var(b, state->primitive_count_vars[stream]);
   else
      primitive_count = nir_undef(b, 1, 32);

   if (state->count_decomposed_prims) {
      decomposed_primitive_count =
         nir_load_var(b, state->decomposed_primitive_count_vars[stream]);
   } else {
      decomposed_primitive_count = nir_undef(b, 1, 32);
   }

   /* Create: if (vertex_count < max_vertices) and insert it.
    *
    * The new if statement needs to be hooked up to the control flow graph
    * before we start inserting instructions into it.
    */
   nir_push_if(b, nir_ilt_imm(b, count, b->shader->info.gs.vertices_out));

   nir_emit_vertex_with_counter(b, count, count_per_primitive, primitive_count,
                                decomposed_primitive_count, stream);

   /* Increment the vertex count by 1 */
   nir_store_var(b, state->vertex_count_vars[stream],
                 nir_iadd_imm(b, count, 1),
                 0x1); /* .x */

   if (state->count_vtx_per_prim) {
      /* Increment the per-primitive vertex count by 1 */
      nir_variable *var = state->vtxcnt_per_prim_vars[stream];
      nir_def *vtx_per_prim_cnt = nir_load_var(b, var);
      nir_store_var(b, var,
                    nir_iadd_imm(b, vtx_per_prim_cnt, 1),
                    0x1); /* .x */
   }

   if (state->count_decomposed_prims) {
      nir_variable *vtx_var = state->vtxcnt_per_prim_vars[stream];
      nir_def *vtx_per_prim_cnt = state->is_points ? nir_imm_int(b, 1) :
                                                     nir_load_var(b, vtx_var);

      /* We form a new primitive for every vertex emitted after the first
       * complete primitive (since we're outputting strips).
       */
      unsigned min_verts =
         mesa_vertices_per_prim(b->shader->info.gs.output_primitive);
      nir_def *new_prim = nir_uge_imm(b, vtx_per_prim_cnt, min_verts);

      /* Increment the decomposed primitive count by 1 if we formed a complete
       * primitive.
       */
      nir_variable *var = state->decomposed_primitive_count_vars[stream];
      nir_def *cnt = nir_load_var(b, var);
      nir_store_var(b, var,
                    nir_iadd(b, cnt, nir_b2i32(b, new_prim)),
                    0x1); /* .x */
   }

   nir_pop_if(b, NULL);

   nir_instr_remove(&intrin->instr);

   state->progress = true;
}

/**
 * Emits code that overwrites incomplete primitives and their vertices.
 *
 * A primitive is considered incomplete when it doesn't have enough vertices.
 * For example, a triangle strip that has 2 or fewer vertices, or a line strip
 * with 1 vertex are considered incomplete.
 *
 * After each end_primitive and at the end of the shader before emitting
 * set_vertex_and_primitive_count, we check if the primitive that is being
 * emitted has enough vertices or not, and we adjust the vertex and primitive
 * counters accordingly.
 *
 * This means that the following emit_vertex can reuse the vertex index of
 * a previous vertex, if the previous primitive was incomplete, so the compiler
 * backend is expected to simply overwrite any data that belonged to those.
 */
static void
overwrite_incomplete_primitives(struct state *state, unsigned stream)
{
   assert(state->count_vtx_per_prim);

   nir_builder *b = state->builder;
   unsigned outprim_min_vertices =
      mesa_vertices_per_prim(b->shader->info.gs.output_primitive);

   /* Total count of vertices emitted so far. */
   nir_def *vtxcnt_total =
      nir_load_var(b, state->vertex_count_vars[stream]);

   /* Number of vertices emitted for the last primitive */
   nir_def *vtxcnt_per_primitive =
      nir_load_var(b, state->vtxcnt_per_prim_vars[stream]);

   /* See if the current primitive is a incomplete */
   nir_def *is_inc_prim =
      nir_ilt_imm(b, vtxcnt_per_primitive, outprim_min_vertices);

   /* Number of vertices in the incomplete primitive */
   nir_def *num_inc_vtx =
      nir_bcsel(b, is_inc_prim, vtxcnt_per_primitive, nir_imm_int(b, 0));

   /* Store corrected total vertex count */
   nir_store_var(b, state->vertex_count_vars[stream],
                 nir_isub(b, vtxcnt_total, num_inc_vtx),
                 0x1); /* .x */

   if (state->count_prims) {
      /* Number of incomplete primitives (0 or 1) */
      nir_def *num_inc_prim = nir_b2i32(b, is_inc_prim);

      /* Store corrected primitive count */
      nir_def *prim_cnt = nir_load_var(b, state->primitive_count_vars[stream]);
      nir_store_var(b, state->primitive_count_vars[stream],
                    nir_isub(b, prim_cnt, num_inc_prim),
                    0x1); /* .x */
   }
}

/**
 * Replace end_primitive with end_primitive_with_counter.
 */
static void
rewrite_end_primitive(nir_intrinsic_instr *intrin, struct state *state)
{
   nir_builder *b = state->builder;
   unsigned stream = nir_intrinsic_stream_id(intrin);

   b->cursor = nir_instr_remove(&intrin->instr);
   state->progress = true;

   /* end_primitive doesn't do anything for points, remove without replacing */
   if (state->is_points) {
      b->shader->info.gs.uses_end_primitive = false;
      return;
   }

   assert(state->vertex_count_vars[stream] != NULL);
   nir_def *count = nir_load_var(b, state->vertex_count_vars[stream]);
   nir_def *count_per_primitive;
   nir_def *primitive_count;
   nir_def *decomposed_primitive_count;

   if (state->count_vtx_per_prim)
      count_per_primitive = nir_load_var(b, state->vtxcnt_per_prim_vars[stream]);
   else
      count_per_primitive = nir_undef(b, count->num_components, count->bit_size);

   if (state->count_prims)
      primitive_count = nir_load_var(b, state->primitive_count_vars[stream]);
   else
      primitive_count = nir_undef(b, 1, 32);

   if (state->count_decomposed_prims) {
      decomposed_primitive_count =
         nir_load_var(b, state->decomposed_primitive_count_vars[stream]);
   } else {
      decomposed_primitive_count = nir_undef(b, 1, 32);
   }

   nir_end_primitive_with_counter(b, count, count_per_primitive,
                                  primitive_count,
                                  decomposed_primitive_count, stream);

   if (state->count_prims) {
      /* Increment the primitive count by 1 */
      nir_def *prim_cnt = nir_load_var(b, state->primitive_count_vars[stream]);
      nir_store_var(b, state->primitive_count_vars[stream],
                    nir_iadd_imm(b, prim_cnt, 1),
                    0x1); /* .x */
   }

   if (state->count_vtx_per_prim) {
      if (state->overwrite_incomplete)
         overwrite_incomplete_primitives(state, stream);

      /* Store 0 to per-primitive vertex count */
      nir_store_var(b, state->vtxcnt_per_prim_vars[stream],
                    nir_imm_int(b, 0),
                    0x1); /* .x */
   }
}

static bool
rewrite_intrinsics(nir_block *block, struct state *state)
{
   nir_foreach_instr_safe(instr, block) {
      if (instr->type != nir_instr_type_intrinsic)
         continue;

      nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
      switch (intrin->intrinsic) {
      case nir_intrinsic_emit_vertex:
      case nir_intrinsic_emit_vertex_with_counter:
         rewrite_emit_vertex(intrin, state);
         break;
      case nir_intrinsic_end_primitive:
      case nir_intrinsic_end_primitive_with_counter:
         rewrite_end_primitive(intrin, state);
         break;
      default:
         /* not interesting; skip this */
         break;
      }
   }

   return true;
}

/**
 * Add a set_vertex_and_primitive_count intrinsic at the end of the program
 * (representing the final total vertex and primitive count).
 */
static void
append_set_vertex_and_primitive_count(nir_block *end_block, struct state *state)
{
   nir_builder *b = state->builder;
   nir_shader *shader = state->builder->shader;

   /* Insert the new intrinsic in all of the predecessors of the end block,
    * but before any jump instructions (return).
    */
   set_foreach(end_block->predecessors, entry) {
      nir_block *pred = (nir_block *)entry->key;
      b->cursor = nir_after_block_before_jump(pred);

      for (unsigned stream = 0; stream < NIR_MAX_XFB_STREAMS; ++stream) {
         /* When it's not per-stream, we only need to write one variable. */
         if (!state->per_stream && stream != 0)
            continue;

         nir_def *vtx_cnt;
         nir_def *prim_cnt;
         nir_def *decomposed_prim_cnt;

         if (state->per_stream && !(shader->info.gs.active_stream_mask & (1 << stream))) {
            /* Inactive stream: vertex count is 0, primitive count is 0 or undef. */
            vtx_cnt = nir_imm_int(b, 0);
            prim_cnt = state->count_prims || state->is_points
                          ? nir_imm_int(b, 0)
                          : nir_undef(b, 1, 32);
            decomposed_prim_cnt = prim_cnt;
         } else {
            if (state->overwrite_incomplete)
               overwrite_incomplete_primitives(state, stream);

            vtx_cnt = nir_load_var(b, state->vertex_count_vars[stream]);

            if (state->count_prims)
               prim_cnt = nir_load_var(b, state->primitive_count_vars[stream]);
            else if (state->is_points)
               /* EndPrimitive does not affect primitive count for points,
                * just use vertex count instead
                */
               prim_cnt = vtx_cnt;
            else
               prim_cnt = nir_undef(b, 1, 32);

            if (state->count_decomposed_prims) {
               decomposed_prim_cnt =
                  nir_load_var(b, state->decomposed_primitive_count_vars[stream]);
            } else {
               decomposed_prim_cnt = nir_undef(b, 1, 32);
            }
         }

         nir_set_vertex_and_primitive_count(b, vtx_cnt, prim_cnt,
                                            decomposed_prim_cnt, stream);
         state->progress = true;
      }
   }
}

/*
 * Append an EndPrimitive intrinsic to the end of the geometry shader. This
 * allows the backend to emit primitives only when EndPrimitive is used. If this
 * EndPrimitive is not needed, it will be predicated out via
 * overwrite_incomplete_primitives.
 */
static void
append_end_primitive(nir_block *end_block, struct state *state)
{
   nir_builder *b = state->builder;

   /* Only end a primitive if there is a primitive to end */
   if (b->shader->info.gs.active_stream_mask == 0)
      return;

   /* Insert the new intrinsic in all of the predecessors of the end block,
    * but before any jump instructions (return).
    */
   set_foreach(end_block->predecessors, entry) {
      nir_block *pred = (nir_block *) entry->key;
      b->cursor = nir_after_block_before_jump(pred);

      nir_end_primitive(b);
   }
}

/**
 * Check to see if there are any blocks that need set_vertex_and_primitive_count
 *
 * If every block that could need the set_vertex_and_primitive_count intrinsic
 * already has one, there is nothing for this pass to do.
 */
static bool
a_block_needs_set_vertex_and_primitive_count(nir_block *end_block, bool per_stream)
{
   set_foreach(end_block->predecessors, entry) {
      nir_block *pred = (nir_block *)entry->key;

      for (unsigned stream = 0; stream < NIR_MAX_XFB_STREAMS; ++stream) {
         /* When it's not per-stream, we only need to write one variable. */
         if (!per_stream && stream != 0)
            continue;

         bool found = false;

         nir_foreach_instr_reverse(instr, pred) {
            if (instr->type != nir_instr_type_intrinsic)
               continue;

            const nir_intrinsic_instr *const intrin =
               nir_instr_as_intrinsic(instr);

            if (intrin->intrinsic == nir_intrinsic_set_vertex_and_primitive_count &&
                intrin->const_index[0] == stream) {
               found = true;
               break;
            }
         }

         if (!found)
            return true;
      }
   }

   return false;
}

bool
nir_lower_gs_intrinsics(nir_shader *shader, nir_lower_gs_intrinsics_flags options)
{
   bool per_stream = options & nir_lower_gs_intrinsics_per_stream;
   bool count_primitives = options & nir_lower_gs_intrinsics_count_primitives;
   bool overwrite_incomplete = options & nir_lower_gs_intrinsics_overwrite_incomplete;
   bool always_end_primitive_non_points = options & nir_lower_gs_intrinsics_always_end_primitive;
   bool count_vtx_per_prim =
      overwrite_incomplete ||
      (options & nir_lower_gs_intrinsics_count_vertices_per_primitive);
   bool count_decomposed_prims = options & nir_lower_gs_intrinsics_count_decomposed_primitives;

   bool is_points = shader->info.gs.output_primitive == MESA_PRIM_POINTS;
   /* points are always complete primitives with a single vertex, so these are
    * not needed when primitive is points.
    */
   if (is_points) {
      count_primitives = false;
      overwrite_incomplete = false;
      count_vtx_per_prim = false;
   }

   struct state state;
   state.progress = false;
   state.count_prims = count_primitives;
   state.count_vtx_per_prim = count_vtx_per_prim;
   state.count_decomposed_prims = count_decomposed_prims;
   state.overwrite_incomplete = overwrite_incomplete;
   state.per_stream = per_stream;
   state.is_points = is_points;

   nir_function_impl *impl = nir_shader_get_entrypoint(shader);
   assert(impl);

   if (!a_block_needs_set_vertex_and_primitive_count(impl->end_block, per_stream))
      return false;

   nir_builder b = nir_builder_at(nir_before_impl(impl));
   state.builder = &b;

   for (unsigned i = 0; i < NIR_MAX_XFB_STREAMS; i++) {
      if (per_stream && !(shader->info.gs.active_stream_mask & (1 << i)))
         continue;

      if (i == 0 || per_stream) {
         state.vertex_count_vars[i] =
            nir_local_variable_create(impl, glsl_uint_type(), "vertex_count");
         /* initialize to 0 */
         nir_store_var(&b, state.vertex_count_vars[i], nir_imm_int(&b, 0), 0x1);

         if (count_primitives) {
            state.primitive_count_vars[i] =
               nir_local_variable_create(impl, glsl_uint_type(), "primitive_count");
            /* initialize to 1 */
            nir_store_var(&b, state.primitive_count_vars[i], nir_imm_int(&b, 1), 0x1);
         }
         if (count_vtx_per_prim) {
            state.vtxcnt_per_prim_vars[i] =
               nir_local_variable_create(impl, glsl_uint_type(), "vertices_per_primitive");
            /* initialize to 0 */
            nir_store_var(&b, state.vtxcnt_per_prim_vars[i], nir_imm_int(&b, 0), 0x1);
         }
         if (count_decomposed_prims) {
            state.decomposed_primitive_count_vars[i] =
               nir_local_variable_create(impl, glsl_uint_type(), "decomposed_primitive_count");
            /* initialize to 0 */
            nir_store_var(&b, state.decomposed_primitive_count_vars[i],
                           nir_imm_int(&b, 0), 0x1);
         }
      } else {
         /* If per_stream is false, we only have one counter of each kind which we
          * want to use for all streams. Duplicate the counter pointers so all
          * streams use the same counters.
          */
         state.vertex_count_vars[i] = state.vertex_count_vars[0];

         if (count_primitives)
            state.primitive_count_vars[i] = state.primitive_count_vars[0];
         if (count_vtx_per_prim)
            state.vtxcnt_per_prim_vars[i] = state.vtxcnt_per_prim_vars[0];
         if (count_decomposed_prims)
            state.decomposed_primitive_count_vars[i] = state.decomposed_primitive_count_vars[0];
      }
   }

   if (always_end_primitive_non_points && !is_points)
      append_end_primitive(impl->end_block, &state);

   nir_foreach_block_safe(block, impl)
      rewrite_intrinsics(block, &state);

   /* This only works because we have a single main() function. */
   append_set_vertex_and_primitive_count(impl->end_block, &state);

   nir_metadata_preserve(impl, nir_metadata_none);

   return state.progress;
}
