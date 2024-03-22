/*
 * Copyright © Microsoft Corporation
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "d3d12_compiler.h"
#include "d3d12_context.h"
#include "d3d12_debug.h"
#include "d3d12_screen.h"

#include "nir.h"
#include "compiler/nir/nir_builder.h"
#include "compiler/nir/nir_builtin_builder.h"

#include "util/u_memory.h"
#include "util/u_simple_shaders.h"

static nir_def *
nir_cull_face(nir_builder *b, nir_variable *vertices, bool ccw)
{
   nir_def *v0 =
       nir_load_deref(b, nir_build_deref_array(b, nir_build_deref_var(b, vertices), nir_imm_int(b, 0)));
   nir_def *v1 =
       nir_load_deref(b, nir_build_deref_array(b, nir_build_deref_var(b, vertices), nir_imm_int(b, 1)));
   nir_def *v2 =
       nir_load_deref(b, nir_build_deref_array(b, nir_build_deref_var(b, vertices), nir_imm_int(b, 2)));

   nir_def *dir = nir_fdot(b, nir_cross4(b, nir_fsub(b, v1, v0),
                                               nir_fsub(b, v2, v0)),
                                   nir_imm_vec4(b, 0.0, 0.0, -1.0, 0.0));
   if (ccw)
       return nir_fle_imm(b, dir, 0.0f);
   else
       return nir_fgt_imm(b, dir, 0.0f);
}

static void
copy_vars(nir_builder *b, nir_deref_instr *dst, nir_deref_instr *src)
{
   assert(glsl_get_bare_type(dst->type) == glsl_get_bare_type(src->type));
   if (glsl_type_is_struct(dst->type)) {
      for (unsigned i = 0; i < glsl_get_length(dst->type); ++i) {
         copy_vars(b, nir_build_deref_struct(b, dst, i), nir_build_deref_struct(b, src, i));
      }
   } else if (glsl_type_is_array_or_matrix(dst->type)) {
      copy_vars(b, nir_build_deref_array_wildcard(b, dst), nir_build_deref_array_wildcard(b, src));
   } else {
      nir_copy_deref(b, dst, src);
   }
}

static d3d12_shader_selector*
d3d12_make_passthrough_gs(struct d3d12_context *ctx, struct d3d12_gs_variant_key *key)
{
   struct d3d12_shader_selector *gs;
   uint64_t varyings = key->varyings->mask;
   nir_shader *nir;
   struct pipe_shader_state templ;

   nir_builder b = nir_builder_init_simple_shader(MESA_SHADER_GEOMETRY,
                                                  &d3d12_screen(ctx->base.screen)->nir_options,
                                                  "passthrough");

   nir = b.shader;
   nir->info.inputs_read = varyings;
   nir->info.outputs_written = varyings;
   nir->info.gs.input_primitive = MESA_PRIM_POINTS;
   nir->info.gs.output_primitive = MESA_PRIM_POINTS;
   nir->info.gs.vertices_in = 1;
   nir->info.gs.vertices_out = 1;
   nir->info.gs.invocations = 1;
   nir->info.gs.active_stream_mask = 1;

   /* Copy inputs to outputs. */
   while (varyings) {
      char tmp[100];
      const int i = u_bit_scan64(&varyings);

      unsigned frac_slots = key->varyings->slots[i].location_frac_mask;
      while (frac_slots) {
         nir_variable *in, *out;
         int j = u_bit_scan(&frac_slots);

         snprintf(tmp, ARRAY_SIZE(tmp), "in_%d", key->varyings->slots[i].vars[j].driver_location);
         in = nir_variable_create(nir,
                                  nir_var_shader_in,
                                  glsl_array_type(key->varyings->slots[i].types[j], 1, false),
                                  tmp);
         in->data.location = i;
         in->data.location_frac = j;
         in->data.driver_location = key->varyings->slots[i].vars[j].driver_location;
         in->data.interpolation = key->varyings->slots[i].vars[j].interpolation;
         in->data.compact = key->varyings->slots[i].vars[j].compact;

         snprintf(tmp, ARRAY_SIZE(tmp), "out_%d", key->varyings->slots[i].vars[j].driver_location);
         out = nir_variable_create(nir,
                                   nir_var_shader_out,
                                   key->varyings->slots[i].types[j],
                                   tmp);
         out->data.location = i;
         out->data.location_frac = j;
         out->data.driver_location = key->varyings->slots[i].vars[j].driver_location;
         out->data.interpolation = key->varyings->slots[i].vars[j].interpolation;
         out->data.compact = key->varyings->slots[i].vars[j].compact;

         nir_deref_instr *in_value = nir_build_deref_array(&b, nir_build_deref_var(&b, in),
                                                               nir_imm_int(&b, 0));
         copy_vars(&b, nir_build_deref_var(&b, out), in_value);
      }
   }

   nir_emit_vertex(&b, 0);
   nir_end_primitive(&b, 0);

   NIR_PASS_V(nir, nir_lower_var_copies);
   nir_validate_shader(nir, "in d3d12_create_passthrough_gs");

   templ.type = PIPE_SHADER_IR_NIR;
   templ.ir.nir = nir;
   templ.stream_output.num_outputs = 0;

   gs = d3d12_create_shader(ctx, PIPE_SHADER_GEOMETRY, &templ);

   return gs;
}

struct emit_primitives_context
{
   struct d3d12_context *ctx;
   nir_builder b;

   unsigned num_vars;
   nir_variable *in[VARYING_SLOT_MAX * 4];
   nir_variable *out[VARYING_SLOT_MAX * 4];
   nir_variable *front_facing_var;

   nir_loop *loop;
   nir_deref_instr *loop_index_deref;
   nir_def *loop_index;
   nir_def *edgeflag_cmp;
   nir_def *front_facing;
};

static bool
d3d12_begin_emit_primitives_gs(struct emit_primitives_context *emit_ctx,
                               struct d3d12_context *ctx,
                               struct d3d12_gs_variant_key *key,
                               enum mesa_prim output_primitive,
                               unsigned vertices_out)
{
   nir_builder *b = &emit_ctx->b;
   nir_variable *edgeflag_var = NULL;
   nir_variable *pos_var = NULL;
   uint64_t varyings = key->varyings->mask;

   emit_ctx->ctx = ctx;

   emit_ctx->b = nir_builder_init_simple_shader(MESA_SHADER_GEOMETRY,
                                                &d3d12_screen(ctx->base.screen)->nir_options,
                                                "edgeflags");

   nir_shader *nir = b->shader;
   nir->info.inputs_read = varyings;
   nir->info.outputs_written = varyings;
   nir->info.gs.input_primitive = MESA_PRIM_TRIANGLES;
   nir->info.gs.output_primitive = output_primitive;
   nir->info.gs.vertices_in = 3;
   nir->info.gs.vertices_out = vertices_out;
   nir->info.gs.invocations = 1;
   nir->info.gs.active_stream_mask = 1;

   while (varyings) {
      char tmp[100];
      const int i = u_bit_scan64(&varyings);

      unsigned frac_slots = key->varyings->slots[i].location_frac_mask;
      while (frac_slots) {
         int j = u_bit_scan(&frac_slots);
         snprintf(tmp, ARRAY_SIZE(tmp), "in_%d", emit_ctx->num_vars);
         emit_ctx->in[emit_ctx->num_vars] = nir_variable_create(nir,
                                                                nir_var_shader_in,
                                                                glsl_array_type(key->varyings->slots[i].types[j], 3, 0),
                                                                tmp);
         emit_ctx->in[emit_ctx->num_vars]->data.location = i;
         emit_ctx->in[emit_ctx->num_vars]->data.location_frac = j;
         emit_ctx->in[emit_ctx->num_vars]->data.driver_location = key->varyings->slots[i].vars[j].driver_location;
         emit_ctx->in[emit_ctx->num_vars]->data.interpolation = key->varyings->slots[i].vars[j].interpolation;
         emit_ctx->in[emit_ctx->num_vars]->data.compact = key->varyings->slots[i].vars[j].compact;

         /* Don't create an output for the edge flag variable */
         if (i == VARYING_SLOT_EDGE) {
            edgeflag_var = emit_ctx->in[emit_ctx->num_vars];
            continue;
         } else if (i == VARYING_SLOT_POS) {
             pos_var = emit_ctx->in[emit_ctx->num_vars];
         }

         snprintf(tmp, ARRAY_SIZE(tmp), "out_%d", emit_ctx->num_vars);
         emit_ctx->out[emit_ctx->num_vars] = nir_variable_create(nir,
                                                                 nir_var_shader_out,
                                                                 key->varyings->slots[i].types[j],
                                                                 tmp);
         emit_ctx->out[emit_ctx->num_vars]->data.location = i;
         emit_ctx->out[emit_ctx->num_vars]->data.location_frac = j;
         emit_ctx->out[emit_ctx->num_vars]->data.driver_location = key->varyings->slots[i].vars[j].driver_location;
         emit_ctx->out[emit_ctx->num_vars]->data.interpolation = key->varyings->slots[i].vars[j].interpolation;
         emit_ctx->out[emit_ctx->num_vars]->data.compact = key->varyings->slots[i].vars[j].compact;

         emit_ctx->num_vars++;
      }
   }

   if (key->has_front_face) {
      emit_ctx->front_facing_var = nir_variable_create(nir,
                                                       nir_var_shader_out,
                                                       glsl_uint_type(),
                                                       "gl_FrontFacing");
      emit_ctx->front_facing_var->data.location = VARYING_SLOT_VAR12;
      emit_ctx->front_facing_var->data.driver_location = emit_ctx->num_vars;
      emit_ctx->front_facing_var->data.interpolation = INTERP_MODE_FLAT;
   }

   /* Temporary variable "loop_index" to loop over input vertices */
   nir_function_impl *impl = nir_shader_get_entrypoint(nir);
   nir_variable *loop_index_var =
      nir_local_variable_create(impl, glsl_uint_type(), "loop_index");
   emit_ctx->loop_index_deref = nir_build_deref_var(b, loop_index_var);
   nir_store_deref(b, emit_ctx->loop_index_deref, nir_imm_int(b, 0), 1);

   nir_def *diagonal_vertex = NULL;
   if (key->edge_flag_fix) {
      nir_def *prim_id = nir_load_primitive_id(b);
      nir_def *odd = nir_build_alu(b, nir_op_imod,
                                       prim_id,
                                       nir_imm_int(b, 2),
                                       NULL, NULL);
      diagonal_vertex = nir_bcsel(b, nir_i2b(b, odd),
                                  nir_imm_int(b, 2),
                                  nir_imm_int(b, 1));
   }

   if (key->cull_mode != PIPE_FACE_NONE || key->has_front_face) {
      if (key->cull_mode == PIPE_FACE_BACK)
         emit_ctx->edgeflag_cmp = nir_cull_face(b, pos_var, key->front_ccw);
      else if (key->cull_mode == PIPE_FACE_FRONT)
         emit_ctx->edgeflag_cmp = nir_cull_face(b, pos_var, !key->front_ccw);

      if (key->has_front_face) {
         if (key->cull_mode == PIPE_FACE_BACK)
            emit_ctx->front_facing = emit_ctx->edgeflag_cmp;
         else
            emit_ctx->front_facing = nir_cull_face(b, pos_var, key->front_ccw);
         emit_ctx->front_facing = nir_i2i32(b, emit_ctx->front_facing);
      }
   }

   /**
    *  while {
    *     if (loop_index >= 3)
    *        break;
    */
   emit_ctx->loop = nir_push_loop(b);

   emit_ctx->loop_index = nir_load_deref(b, emit_ctx->loop_index_deref);
   nir_def *cmp = nir_ige_imm(b, emit_ctx->loop_index, 3);
   nir_if *loop_check = nir_push_if(b, cmp);
   nir_jump(b, nir_jump_break);
   nir_pop_if(b, loop_check);

   if (edgeflag_var) {
      nir_def *edge_flag =
         nir_load_deref(b, nir_build_deref_array(b, nir_build_deref_var(b, edgeflag_var), emit_ctx->loop_index));
      nir_def *is_edge = nir_feq_imm(b, nir_channel(b, edge_flag, 0), 1.0);
      if (emit_ctx->edgeflag_cmp)
         emit_ctx->edgeflag_cmp = nir_iand(b, emit_ctx->edgeflag_cmp, is_edge);
      else
         emit_ctx->edgeflag_cmp = is_edge;
   }

   if (key->edge_flag_fix) {
      nir_def *is_edge = nir_ine(b, emit_ctx->loop_index, diagonal_vertex);
      if (emit_ctx->edgeflag_cmp)
         emit_ctx->edgeflag_cmp = nir_iand(b, emit_ctx->edgeflag_cmp, is_edge);
      else
         emit_ctx->edgeflag_cmp = is_edge;
   }

   return true;
}

static struct d3d12_shader_selector *
d3d12_finish_emit_primitives_gs(struct emit_primitives_context *emit_ctx, bool end_primitive)
{
   struct pipe_shader_state templ;
   nir_builder *b = &emit_ctx->b;
   nir_shader *nir = b->shader;

   /**
    *     loop_index++;
    *  }
    */
   nir_store_deref(b, emit_ctx->loop_index_deref, nir_iadd_imm(b, emit_ctx->loop_index, 1), 1);
   nir_pop_loop(b, emit_ctx->loop);

   if (end_primitive)
      nir_end_primitive(b, 0);

   nir_validate_shader(nir, "in d3d12_lower_edge_flags");

   NIR_PASS_V(nir, nir_lower_var_copies);

   templ.type = PIPE_SHADER_IR_NIR;
   templ.ir.nir = nir;
   templ.stream_output.num_outputs = 0;

   return d3d12_create_shader(emit_ctx->ctx, PIPE_SHADER_GEOMETRY, &templ);
}

static d3d12_shader_selector*
d3d12_emit_points(struct d3d12_context *ctx, struct d3d12_gs_variant_key *key)
{
   struct emit_primitives_context emit_ctx = {0};
   nir_builder *b = &emit_ctx.b;

   d3d12_begin_emit_primitives_gs(&emit_ctx, ctx, key, MESA_PRIM_POINTS, 3);

   /**
    *  if (edge_flag)
    *     out_position = in_position;
    *  else
    *     out_position = vec4(-2.0, -2.0, 0.0, 1.0); // Invalid position
    *
    *  [...] // Copy other variables
    *
    *  EmitVertex();
    */
   for (unsigned i = 0; i < emit_ctx.num_vars; ++i) {
      nir_def *index = (key->flat_varyings & (1ull << emit_ctx.in[i]->data.location))  ?
                              nir_imm_int(b, (key->flatshade_first ? 0 : 2)) : emit_ctx.loop_index;
      nir_deref_instr *in_value = nir_build_deref_array(b, nir_build_deref_var(b, emit_ctx.in[i]), index);
      if (emit_ctx.in[i]->data.location == VARYING_SLOT_POS && emit_ctx.edgeflag_cmp) {
         nir_if *edge_check = nir_push_if(b, emit_ctx.edgeflag_cmp);
         copy_vars(b, nir_build_deref_var(b, emit_ctx.out[i]), in_value);
         nir_if *edge_else = nir_push_else(b, edge_check);
         nir_store_deref(b, nir_build_deref_var(b, emit_ctx.out[i]),
                         nir_imm_vec4(b, -2.0, -2.0, 0.0, 1.0), 0xf);
         nir_pop_if(b, edge_else);
      } else {
         copy_vars(b, nir_build_deref_var(b, emit_ctx.out[i]), in_value);
      }
   }
   if (key->has_front_face)
       nir_store_var(b, emit_ctx.front_facing_var, emit_ctx.front_facing, 0x1);
   nir_emit_vertex(b, 0);

   return d3d12_finish_emit_primitives_gs(&emit_ctx, false);
}

static d3d12_shader_selector*
d3d12_emit_lines(struct d3d12_context *ctx, struct d3d12_gs_variant_key *key)
{
   struct emit_primitives_context emit_ctx = {0};
   nir_builder *b = &emit_ctx.b;

   d3d12_begin_emit_primitives_gs(&emit_ctx, ctx, key, MESA_PRIM_LINE_STRIP, 6);

   nir_def *next_index = nir_imod_imm(b, nir_iadd_imm(b, emit_ctx.loop_index, 1), 3);

   /* First vertex */
   for (unsigned i = 0; i < emit_ctx.num_vars; ++i) {
      nir_def *index = (key->flat_varyings & (1ull << emit_ctx.in[i]->data.location)) ?
                              nir_imm_int(b, (key->flatshade_first ? 0 : 2)) : emit_ctx.loop_index;
      nir_deref_instr *in_value = nir_build_deref_array(b, nir_build_deref_var(b, emit_ctx.in[i]), index);
      copy_vars(b, nir_build_deref_var(b, emit_ctx.out[i]), in_value);
   }
   if (key->has_front_face)
       nir_store_var(b, emit_ctx.front_facing_var, emit_ctx.front_facing, 0x1);
   nir_emit_vertex(b, 0);

   /* Second vertex. If not an edge, use same position as first vertex */
   for (unsigned i = 0; i < emit_ctx.num_vars; ++i) {
      nir_def *index = next_index;
      if (emit_ctx.in[i]->data.location == VARYING_SLOT_POS)
         index = nir_bcsel(b, emit_ctx.edgeflag_cmp, next_index, emit_ctx.loop_index);
      else if (key->flat_varyings & (1ull << emit_ctx.in[i]->data.location))
         index = nir_imm_int(b, 2);
      copy_vars(b, nir_build_deref_var(b, emit_ctx.out[i]),
                nir_build_deref_array(b, nir_build_deref_var(b, emit_ctx.in[i]), index));
   }
   if (key->has_front_face)
       nir_store_var(b, emit_ctx.front_facing_var, emit_ctx.front_facing, 0x1);
   nir_emit_vertex(b, 0);

   nir_end_primitive(b, 0);

   return d3d12_finish_emit_primitives_gs(&emit_ctx, false);
}

static d3d12_shader_selector*
d3d12_emit_triangles(struct d3d12_context *ctx, struct d3d12_gs_variant_key *key)
{
   struct emit_primitives_context emit_ctx = {0};
   nir_builder *b = &emit_ctx.b;

   d3d12_begin_emit_primitives_gs(&emit_ctx, ctx, key, MESA_PRIM_TRIANGLE_STRIP, 3);

   /**
    *  [...] // Copy variables
    *
    *  EmitVertex();
    */

   nir_def *incr = NULL;

   if (key->provoking_vertex > 0)
      incr = nir_imm_int(b, key->provoking_vertex);
   else
      incr = nir_imm_int(b, 3);

   if (key->alternate_tri) {
      nir_def *odd = nir_imod_imm(b, nir_load_primitive_id(b), 2);
      incr = nir_isub(b, incr, odd);
   }

   assert(incr != NULL);
   nir_def *index = nir_imod_imm(b, nir_iadd(b, emit_ctx.loop_index, incr), 3);
   for (unsigned i = 0; i < emit_ctx.num_vars; ++i) {
      nir_deref_instr *in_value = nir_build_deref_array(b, nir_build_deref_var(b, emit_ctx.in[i]), index);
      copy_vars(b, nir_build_deref_var(b, emit_ctx.out[i]), in_value);
   }
   nir_emit_vertex(b, 0);

   return d3d12_finish_emit_primitives_gs(&emit_ctx, true);
}

static uint32_t
hash_gs_variant_key(const void *key)
{
   d3d12_gs_variant_key *v = (d3d12_gs_variant_key*)key;
   uint32_t hash = _mesa_hash_data(v, offsetof(d3d12_gs_variant_key, varyings));
   if (v->varyings)
      hash = _mesa_hash_data_with_seed(v->varyings->slots, sizeof(v->varyings->slots[0]) * v->varyings->max, hash);
   return hash;
}

static bool
equals_gs_variant_key(const void *a, const void *b)
{
   return memcmp(a, b, sizeof(d3d12_gs_variant_key)) == 0;
}

void
d3d12_gs_variant_cache_init(struct d3d12_context *ctx)
{
   ctx->gs_variant_cache = _mesa_hash_table_create(NULL, NULL, equals_gs_variant_key);
}

static void
delete_entry(struct hash_entry *entry)
{
   d3d12_shader_free((d3d12_shader_selector *)entry->data);
}

void
d3d12_gs_variant_cache_destroy(struct d3d12_context *ctx)
{
   _mesa_hash_table_destroy(ctx->gs_variant_cache, delete_entry);
}

static struct d3d12_shader_selector *
create_geometry_shader_variant(struct d3d12_context *ctx, struct d3d12_gs_variant_key *key)
{
   d3d12_shader_selector *gs = NULL;

   if (key->passthrough)
      gs = d3d12_make_passthrough_gs(ctx, key);
   else if (key->provoking_vertex > 0 || key->alternate_tri)
      gs = d3d12_emit_triangles(ctx, key);
   else if (key->fill_mode == PIPE_POLYGON_MODE_POINT)
      gs = d3d12_emit_points(ctx, key);
   else if (key->fill_mode == PIPE_POLYGON_MODE_LINE)
      gs = d3d12_emit_lines(ctx, key);

   if (gs) {
      gs->is_variant = true;
      gs->gs_key = *key;
   }

   return gs;
}

d3d12_shader_selector *
d3d12_get_gs_variant(struct d3d12_context *ctx, struct d3d12_gs_variant_key *key)
{
   uint32_t hash = hash_gs_variant_key(key);
   struct hash_entry *entry = _mesa_hash_table_search_pre_hashed(ctx->gs_variant_cache,
                                                                 hash, key);
   if (!entry) {
      d3d12_shader_selector *gs = create_geometry_shader_variant(ctx, key);
      entry = _mesa_hash_table_insert_pre_hashed(ctx->gs_variant_cache,
                                                 hash, &gs->gs_key, gs);
      assert(entry);
   }

   return (d3d12_shader_selector *)entry->data;
}
