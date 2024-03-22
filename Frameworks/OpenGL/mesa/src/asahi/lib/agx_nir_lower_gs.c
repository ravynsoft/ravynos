/*
 * Copyright 2023 Alyssa Rosenzweig
 * Copyright 2023 Valve Corporation
 * SPDX-License-Identifier: MIT
 */

#include "agx_nir_lower_gs.h"
#include "asahi/compiler/agx_compile.h"
#include "compiler/nir/nir_builder.h"
#include "shaders/geometry.h"
#include "util/bitscan.h"
#include "util/macros.h"
#include "util/ralloc.h"
#include "util/u_math.h"
#include "libagx_shaders.h"
#include "nir.h"
#include "nir_builder_opcodes.h"
#include "nir_xfb_info.h"

enum gs_counter {
   GS_COUNTER_VERTICES = 0,
   GS_COUNTER_PRIMITIVES,
   GS_COUNTER_XFB_PRIMITIVES,
   GS_NUM_COUNTERS
};

#define MAX_PRIM_OUT_SIZE 3

struct lower_gs_state {
   int static_count[GS_NUM_COUNTERS][MAX_VERTEX_STREAMS];
   nir_variable *outputs[NUM_TOTAL_VARYING_SLOTS][MAX_PRIM_OUT_SIZE];

   /* For the geometry output buffer */
   unsigned stride_B;
   unsigned offset_B[NUM_TOTAL_VARYING_SLOTS];

   /* The count buffer contains `count_stride_el` 32-bit words in a row for each
    * input primitive, for `input_primitives * count_stride_el * 4` total bytes.
    */
   unsigned count_stride_el;

   /* The index of each counter in the count buffer, or -1 if it's not in the
    * count buffer.
    *
    * Invariant: count_stride_el == sum(count_index[i][j] >= 0).
    */
   int count_index[MAX_VERTEX_STREAMS][GS_NUM_COUNTERS];

   /* Provoking vertex mode, required for transform feedback calculations */
   nir_def *flatshade_first;

   bool rasterizer_discard;
};

static uint64_t
outputs_rasterized(nir_shader *s)
{
   uint64_t outputs = s->info.outputs_written;

   /* Optimize out pointless gl_PointSize outputs. Bizarrely, these occur. We
    * need to preserve the transform feedback portion of the write, but we don't
    * bother saving for rasterization.
    */
   if (s->info.gs.output_primitive != MESA_PRIM_POINTS)
      outputs &= ~VARYING_BIT_PSIZ;

   return outputs;
}

/* Helpers for loading from the geometry state buffer */
static nir_def *
load_geometry_param_offset(nir_builder *b, uint32_t offset, uint8_t bytes)
{
   nir_def *base = nir_load_geometry_param_buffer_agx(b);
   nir_def *addr = nir_iadd_imm(b, base, offset);

   assert((offset % bytes) == 0 && "must be naturally aligned");

   return nir_load_global_constant(b, addr, bytes, 1, bytes * 8);
}

static void
store_geometry_param_offset(nir_builder *b, nir_def *def, uint32_t offset,
                            uint8_t bytes)
{
   nir_def *base = nir_load_geometry_param_buffer_agx(b);
   nir_def *addr = nir_iadd_imm(b, base, offset);

   assert((offset % bytes) == 0 && "must be naturally aligned");

   nir_store_global(b, addr, 4, def, nir_component_mask(def->num_components));
}

#define store_geometry_param(b, field, def)                                    \
   store_geometry_param_offset(                                                \
      b, def, offsetof(struct agx_geometry_params, field),                     \
      sizeof(((struct agx_geometry_params *)0)->field))

#define load_geometry_param(b, field)                                          \
   load_geometry_param_offset(                                                 \
      b, offsetof(struct agx_geometry_params, field),                          \
      sizeof(((struct agx_geometry_params *)0)->field))

static nir_def *
load_geometry_state_offset(nir_builder *b, uint32_t offset, uint8_t bytes)
{
   nir_def *base = load_geometry_param(b, state);
   nir_def *addr = nir_iadd_imm(b, base, offset);

   assert((offset % bytes) == 0 && "must be naturally aligned");

   return nir_load_global_constant(b, addr, bytes, 1, bytes * 8);
}

#define load_geometry_state(b, field)                                          \
   load_geometry_state_offset(b, offsetof(struct agx_geometry_state, field),   \
                              sizeof(((struct agx_geometry_state *)0)->field))

/* Helper for updating counters */
static void
add_counter(nir_builder *b, nir_def *counter, nir_def *increment)
{
   /* If the counter is NULL, the counter is disabled. Skip the update. */
   nir_if *nif = nir_push_if(b, nir_ine_imm(b, counter, 0));
   {
      nir_def *old = nir_load_global(b, counter, 4, 1, 32);
      nir_def *new_ = nir_iadd(b, old, increment);
      nir_store_global(b, counter, 4, new_, nir_component_mask(1));
   }
   nir_pop_if(b, nif);
}

/* Helpers for lowering I/O to variables */
struct lower_output_to_var_state {
   nir_variable *outputs[NUM_TOTAL_VARYING_SLOTS];
   bool arrayed;
};

static bool
lower_output_to_var(nir_builder *b, nir_instr *instr, void *data)
{
   struct lower_output_to_var_state *state = data;
   if (instr->type != nir_instr_type_intrinsic)
      return false;

   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
   if (intr->intrinsic != nir_intrinsic_store_output)
      return false;

   b->cursor = nir_instr_remove(instr);
   nir_io_semantics sem = nir_intrinsic_io_semantics(intr);
   unsigned component = nir_intrinsic_component(intr);
   nir_def *value = intr->src[0].ssa;

   assert(nir_src_is_const(intr->src[1]) && "no indirect outputs");
   assert(nir_intrinsic_write_mask(intr) == nir_component_mask(1) &&
          "should be scalarized");

   nir_variable *var =
      state->outputs[sem.location + nir_src_as_uint(intr->src[1])];
   if (!var) {
      assert(sem.location == VARYING_SLOT_PSIZ &&
             "otherwise in outputs_written");
      return true;
   }

   unsigned nr_components = glsl_get_components(glsl_without_array(var->type));
   assert(component < nr_components);

   /* Turn it into a vec4 write like NIR expects */
   value = nir_vector_insert_imm(b, nir_undef(b, nr_components, 32), value,
                                 component);

   if (state->arrayed) {
      nir_def *index = nir_load_vertex_id_in_primitive_agx(b);
      nir_store_array_var(b, var, index, value, BITFIELD_BIT(component));
   } else {
      nir_store_var(b, var, value, BITFIELD_BIT(component));
   }

   return true;
}

/*
 * Geometry shader invocations are compute-like:
 *
 * (primitive ID, instance ID, 1)
 */
static nir_def *
load_primitive_id(nir_builder *b)
{
   return nir_channel(b, nir_load_global_invocation_id(b, 32), 0);
}

static nir_def *
load_instance_id(nir_builder *b)
{
   return nir_channel(b, nir_load_global_invocation_id(b, 32), 1);
}

static bool
lower_gs_inputs(nir_builder *b, nir_intrinsic_instr *intr, void *data)
{
   struct lower_output_to_var_state *vs_state = data;
   if (intr->intrinsic != nir_intrinsic_load_per_vertex_input)
      return false;

   /* I suppose we could support indirect GS inputs, but it would be more
    * complicated and probably pointless (versus the lowering the frontend would
    * otherwise do). GS lowering is hard enough as it is.
    */
   assert(nir_src_is_const(intr->src[1]) && "no indirect GS inputs");

   b->cursor = nir_instr_remove(&intr->instr);
   nir_def *vertex = intr->src[0].ssa;
   nir_io_semantics sem = nir_intrinsic_io_semantics(intr);

   nir_variable *var =
      vs_state->outputs[sem.location + nir_src_as_uint(intr->src[1])];

   nir_def *val = nir_load_array_var(b, var, vertex);

   assert(intr->def.bit_size == 32);
   unsigned start = nir_intrinsic_component(intr);
   unsigned count = intr->def.num_components;
   val = nir_channels(b, val, nir_component_mask(count) << start);

   nir_def_rewrite_uses(&intr->def, val);
   return true;
}

static bool
lower_id_in_prim(nir_builder *b, nir_instr *instr, void *data)
{
   if (instr->type != nir_instr_type_intrinsic)
      return false;

   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
   if (intr->intrinsic != nir_intrinsic_load_vertex_id_in_primitive_agx)
      return false;

   /* The ID in the primitive is passed as a function parameter */
   b->cursor = nir_instr_remove(instr);
   nir_def *id = nir_load_param(b, 0);
   nir_def_rewrite_uses(&intr->def, nir_u2uN(b, id, intr->def.bit_size));
   return true;
}

static void
agx_nir_link_vs_gs(nir_shader *vs, nir_shader *gs)
{
   struct lower_output_to_var_state state = {.arrayed = true};

   /* Vertex shader outputs will be placed in arrays. Create those arrays. */
   u_foreach_bit64(slot, vs->info.outputs_written) {
      state.outputs[slot] = nir_variable_create(
         gs, nir_var_shader_temp,
         glsl_array_type(glsl_uvec4_type(), gs->info.gs.vertices_in, 0),
         gl_varying_slot_name_for_stage(slot, MESA_SHADER_VERTEX));
   }

   /* Rewrite geometry shader inputs to read from those arrays */
   NIR_PASS_V(gs, nir_shader_intrinsics_pass, lower_gs_inputs,
              nir_metadata_block_index | nir_metadata_dominance, &state);

   /* Link the vertex shader with the geometry shader. This assumes that
    * all functions have been inlined in the vertex shader.
    */
   nir_function_impl *vs_entry = nir_shader_get_entrypoint(vs);
   nir_function *vs_function = nir_function_create(gs, "vertex");
   vs_function->impl = nir_function_impl_clone(gs, vs_entry);
   vs_function->impl->function = vs_function;

   /* The vertex shader needs to be passed its index in the input primitive */
   vs_function->num_params = 1;
   vs_function->params = rzalloc_array(gs, nir_parameter, 1);
   vs_function->params[0] = (nir_parameter){1, 16};

   /* The vertex shader needs to be expressed in terms of that index */
   nir_function_instructions_pass(
      vs_function->impl, lower_output_to_var,
      nir_metadata_block_index | nir_metadata_dominance, &state);

   nir_function_instructions_pass(
      vs_function->impl, lower_id_in_prim,
      nir_metadata_block_index | nir_metadata_dominance, NULL);

   /* Run the vertex shader for each vertex in the input primitive */
   nir_function_impl *gs_entry = nir_shader_get_entrypoint(gs);
   nir_builder b = nir_builder_at(nir_before_impl(gs_entry));

   for (unsigned i = 0; i < gs->info.gs.vertices_in; ++i) {
      nir_call(&b, vs_function, nir_imm_intN_t(&b, i, 16));
   }

   /* Copy texture info. We force bindless on GS for now. */
   gs->info.num_textures = vs->info.num_textures;

   /* Inline the VS into the GS */
   nir_inline_functions(gs);
   exec_node_remove(&vs_function->node);
   nir_lower_global_vars_to_local(gs);

   /* Do some optimization to get rid of indirects */
   bool progress;

   do {
      progress = false;
      NIR_PASS(progress, gs, nir_opt_constant_folding);
      NIR_PASS(progress, gs, nir_opt_dce);
   } while (progress);

   /* If any indirects hung around, lower them */
   nir_lower_indirect_derefs(gs, nir_var_function_temp, UINT32_MAX);
}

/*
 * Unrolled ID is the index of the primitive in the count buffer, given as
 * (instance ID * # vertices/instance) + vertex ID
 */
static nir_def *
calc_unrolled_id(nir_builder *b)
{
   return nir_iadd(b,
                   nir_imul(b, load_instance_id(b), nir_load_num_vertices(b)),
                   load_primitive_id(b));
}

static nir_def *
load_count_address(nir_builder *b, struct lower_gs_state *state,
                   nir_def *unrolled_id, unsigned stream,
                   enum gs_counter counter)
{
   int index = state->count_index[stream][counter];
   if (index < 0)
      return NULL;

   nir_def *prim_offset_el =
      nir_imul_imm(b, unrolled_id, state->count_stride_el);

   nir_def *offset_el = nir_iadd_imm(b, prim_offset_el, index);

   return nir_iadd(b, load_geometry_param(b, count_buffer),
                   nir_u2u64(b, nir_imul_imm(b, offset_el, 4)));
}

static void
write_counts(nir_builder *b, nir_intrinsic_instr *intr,
             struct lower_gs_state *state)
{
   /* Store each required counter */
   nir_def *counts[GS_NUM_COUNTERS] = {
      [GS_COUNTER_VERTICES] = intr->src[0].ssa,
      [GS_COUNTER_PRIMITIVES] = intr->src[1].ssa,
      [GS_COUNTER_XFB_PRIMITIVES] = intr->src[2].ssa,
   };

   for (unsigned i = 0; i < GS_NUM_COUNTERS; ++i) {
      nir_def *addr = load_count_address(b, state, calc_unrolled_id(b),
                                         nir_intrinsic_stream_id(intr), i);

      if (addr)
         nir_store_global(b, addr, 4, counts[i], nir_component_mask(1));
   }
}

static bool
lower_gs_count_instr(nir_builder *b, nir_intrinsic_instr *intr, void *data)
{
   switch (intr->intrinsic) {
   case nir_intrinsic_emit_vertex_with_counter:
   case nir_intrinsic_end_primitive_with_counter:
   case nir_intrinsic_store_output:
      /* These are for the main shader, just remove them */
      nir_instr_remove(&intr->instr);
      return true;

   case nir_intrinsic_set_vertex_and_primitive_count:
      b->cursor = nir_instr_remove(&intr->instr);
      write_counts(b, intr, data);
      return true;

   default:
      return false;
   }
}

static bool
lower_id(nir_builder *b, nir_intrinsic_instr *intr, void *data)
{
   b->cursor = nir_before_instr(&intr->instr);

   nir_def *id;
   if (intr->intrinsic == nir_intrinsic_load_primitive_id)
      id = load_primitive_id(b);
   else if (intr->intrinsic == nir_intrinsic_load_instance_id)
      id = load_instance_id(b);
   else if (intr->intrinsic == nir_intrinsic_load_num_vertices)
      id = nir_channel(b, nir_load_num_workgroups(b), 0);
   else if (intr->intrinsic == nir_intrinsic_load_flat_mask)
      id = load_geometry_param(b, flat_outputs);
   else if (intr->intrinsic == nir_intrinsic_load_provoking_last) {
      id = nir_b2b32(
         b, libagx_is_provoking_last(b, nir_load_input_assembly_buffer_agx(b)));
   } else
      return false;

   b->cursor = nir_instr_remove(&intr->instr);
   nir_def_rewrite_uses(&intr->def, id);
   return true;
}

/*
 * Create a "Geometry count" shader. This is a stripped down geometry shader
 * that just write its number of emitted vertices / primitives / transform
 * feedback primitives to a count buffer. That count buffer will be prefix
 * summed prior to running the real geometry shader. This is skipped if the
 * counts are statically known.
 */
static nir_shader *
agx_nir_create_geometry_count_shader(nir_shader *gs, const nir_shader *libagx,
                                     struct lower_gs_state *state)
{
   /* Don't muck up the original shader */
   nir_shader *shader = nir_shader_clone(NULL, gs);

   if (shader->info.name) {
      shader->info.name =
         ralloc_asprintf(shader, "%s_count", shader->info.name);
   } else {
      shader->info.name = "count";
   }

   NIR_PASS_V(shader, nir_shader_intrinsics_pass, lower_gs_count_instr,
              nir_metadata_block_index | nir_metadata_dominance, state);

   NIR_PASS_V(shader, nir_shader_intrinsics_pass, lower_id,
              nir_metadata_block_index | nir_metadata_dominance, NULL);

   /* Preprocess it */
   UNUSED struct agx_uncompiled_shader_info info;
   agx_preprocess_nir(shader, libagx, false, &info);

   return shader;
}

/*
 * Create a GS copy shader. This is a hardware vertex shader that copies each
 * vertex from the geometry output buffer to the Unified Vertex Store.
 */
static nir_shader *
agx_nir_create_gs_copy_shader(struct lower_gs_state *state,
                              uint64_t outputs_written,
                              unsigned clip_distance_array_size,
                              unsigned cull_distance_array_size,
                              enum mesa_prim output_primitive)
{
   nir_builder b_ = nir_builder_init_simple_shader(MESA_SHADER_VERTEX,
                                                   &agx_nir_options, "GS copy");
   nir_builder *b = &b_;

   b->shader->info.clip_distance_array_size = clip_distance_array_size;
   b->shader->info.cull_distance_array_size = cull_distance_array_size;

   /* Get the base for this vertex */
   nir_def *vert_offs = nir_imul_imm(b, nir_load_vertex_id(b), state->stride_B);

   nir_def *state_buffer = load_geometry_param(b, output_buffer);

   /* Each output must be copied */
   u_foreach_bit64(slot, outputs_written) {
      assert(state->outputs[slot][0] != NULL);

      nir_def *addr = nir_iadd(
         b, state_buffer,
         nir_u2u64(b, nir_iadd_imm(b, vert_offs, state->offset_B[slot])));

      unsigned components = glsl_get_components(state->outputs[slot][0]->type);

      nir_def *value = nir_load_global_constant(b, addr, 4, components, 32);

      /* We set NIR_COMPACT_ARRAYS so clip/cull distance needs to come all in
       * DIST0. Undo the offset if we need to.
       */
      unsigned offset = 0;
      if (slot == VARYING_SLOT_CULL_DIST1 || slot == VARYING_SLOT_CLIP_DIST1)
         offset = 1;

      nir_store_output(b, value, nir_imm_int(b, offset),
                       .io_semantics.location = slot - offset,
                       .io_semantics.num_slots = 1,
                       .write_mask = nir_component_mask(components));

      b->shader->info.outputs_written |= BITFIELD64_BIT(slot);
   }

   /* In OpenGL ES, it is legal to omit the point size write from the geometry
    * shader when drawing points. In this case, the point size is
    * implicitly 1.0. We implement this by inserting this synthetic
    * `gl_PointSize = 1.0` write into the GS copy shader, if the GS does not
    * export a point size while drawing points.
    *
    * This should not be load bearing for other APIs, but should be harmless.
    */
   bool is_points = output_primitive == MESA_PRIM_POINTS;

   if (!(outputs_written & VARYING_BIT_PSIZ) && is_points) {
      nir_store_output(b, nir_imm_float(b, 1.0), nir_imm_int(b, 0),
                       .io_semantics.location = VARYING_SLOT_PSIZ,
                       .io_semantics.num_slots = 1,
                       .write_mask = nir_component_mask(1));

      b->shader->info.outputs_written |= VARYING_BIT_PSIZ;
   }

   UNUSED struct agx_uncompiled_shader_info info;
   agx_preprocess_nir(b->shader, NULL, false, &info);

   return b->shader;
}

static nir_def *
previous_count(nir_builder *b, struct lower_gs_state *state, unsigned stream,
               nir_def *unrolled_id, enum gs_counter counter)
{
   assert(stream < MAX_VERTEX_STREAMS);
   assert(counter < GS_NUM_COUNTERS);
   int static_count = state->static_count[counter][stream];

   if (static_count >= 0) {
      /* If the number of outputted vertices per invocation is known statically,
       * we can calculate the base.
       */
      return nir_imul_imm(b, unrolled_id, static_count);
   } else {
      /* Otherwise, we need to load from the prefix sum buffer. Note that the
       * sums are inclusive, so index 0 is nonzero. This requires a little
       * fixup here. We use a saturating unsigned subtraction so we don't read
       * out-of-bounds for zero.
       *
       * TODO: Optimize this.
       */
      nir_def *prim_minus_1 = nir_usub_sat(b, unrolled_id, nir_imm_int(b, 1));
      nir_def *addr =
         load_count_address(b, state, prim_minus_1, stream, counter);

      return nir_bcsel(b, nir_ieq_imm(b, unrolled_id, 0), nir_imm_int(b, 0),
                       nir_load_global_constant(b, addr, 4, 1, 32));
   }
}

static nir_def *
previous_vertices(nir_builder *b, struct lower_gs_state *state, unsigned stream,
                  nir_def *unrolled_id)
{
   return previous_count(b, state, stream, unrolled_id, GS_COUNTER_VERTICES);
}

static nir_def *
previous_primitives(nir_builder *b, struct lower_gs_state *state,
                    unsigned stream, nir_def *unrolled_id)
{
   return previous_count(b, state, stream, unrolled_id, GS_COUNTER_PRIMITIVES);
}

static nir_def *
previous_xfb_primitives(nir_builder *b, struct lower_gs_state *state,
                        unsigned stream, nir_def *unrolled_id)
{
   return previous_count(b, state, stream, unrolled_id,
                         GS_COUNTER_XFB_PRIMITIVES);
}

static void
lower_end_primitive(nir_builder *b, nir_intrinsic_instr *intr,
                    struct lower_gs_state *state)
{
   assert(b->shader->info.gs.output_primitive != MESA_PRIM_POINTS &&
          "should've been removed");

   /* The GS is the last stage before rasterization, so if we discard the
    * rasterization, we don't output an index buffer, nothing will read it.
    * Index buffer is only for the rasterization stream.
    */
   unsigned stream = nir_intrinsic_stream_id(intr);
   if (state->rasterizer_discard || stream != 0)
      return;

   libagx_end_primitive(b, load_geometry_param(b, output_index_buffer),
                        intr->src[0].ssa, intr->src[1].ssa, intr->src[2].ssa,
                        previous_vertices(b, state, 0, calc_unrolled_id(b)),
                        previous_primitives(b, state, 0, calc_unrolled_id(b)));
}

static unsigned
verts_in_output_prim(nir_shader *gs)
{
   return mesa_vertices_per_prim(gs->info.gs.output_primitive);
}

static void
write_xfb(nir_builder *b, struct lower_gs_state *state, unsigned stream,
          nir_def *index_in_strip, nir_def *prim_id_in_invocation)
{
   /* If there is no XFB info, there is no XFB */
   struct nir_xfb_info *xfb = b->shader->xfb_info;
   if (!xfb)
      return;

   unsigned verts = verts_in_output_prim(b->shader);

   /* Get the index of this primitive in the XFB buffer. That is, the base for
    * this invocation for the stream plus the offset within this invocation.
    */
   nir_def *invocation_base =
      previous_xfb_primitives(b, state, stream, calc_unrolled_id(b));

   nir_def *prim_index = nir_iadd(b, invocation_base, prim_id_in_invocation);
   nir_def *base_index = nir_imul_imm(b, prim_index, verts);

   nir_def *xfb_prims = load_geometry_param(b, xfb_prims[stream]);
   nir_push_if(b, nir_ult(b, prim_index, xfb_prims));

   /* Write XFB for each output */
   for (unsigned i = 0; i < xfb->output_count; ++i) {
      nir_xfb_output_info output = xfb->outputs[i];

      /* Only write to the selected stream */
      if (xfb->buffer_to_stream[output.buffer] != stream)
         continue;

      unsigned buffer = output.buffer;
      unsigned stride = xfb->buffers[buffer].stride;
      unsigned count = util_bitcount(output.component_mask);

      for (unsigned vert = 0; vert < verts; ++vert) {
         /* We write out the vertices backwards, since 0 is the current
          * emitted vertex (which is actually the last vertex).
          *
          * We handle NULL var for
          * KHR-Single-GL44.enhanced_layouts.xfb_capture_struct.
          */
         unsigned v = (verts - 1) - vert;
         nir_variable *var = state->outputs[output.location][v];
         nir_def *value = var ? nir_load_var(b, var) : nir_undef(b, 4, 32);

         /* In case output.component_mask contains invalid components, write
          * out zeroes instead of blowing up validation.
          *
          * KHR-Single-GL44.enhanced_layouts.xfb_capture_inactive_output_component
          * hits this.
          */
         value = nir_pad_vector_imm_int(b, value, 0, 4);

         nir_def *rotated_vert = nir_imm_int(b, vert);
         if (verts == 3) {
            /* Map vertices for output so we get consistent winding order. For
             * the primitive index, we use the index_in_strip. This is actually
             * the vertex index in the strip, hence
             * offset by 2 relative to the true primitive index (#2 for the
             * first triangle in the strip, #3 for the second). That's ok
             * because only the parity matters.
             */
            rotated_vert = libagx_map_vertex_in_tri_strip(
               b, index_in_strip, rotated_vert, state->flatshade_first);
         }

         nir_def *addr = libagx_xfb_vertex_address(
            b, nir_load_geometry_param_buffer_agx(b), base_index, rotated_vert,
            nir_imm_int(b, buffer), nir_imm_int(b, stride),
            nir_imm_int(b, output.offset));

         nir_store_global(b, addr, 4,
                          nir_channels(b, value, output.component_mask),
                          nir_component_mask(count));
      }
   }

   nir_pop_if(b, NULL);
}

/*
 * Lower emit_vertex_with_counter to a copy of vertex outputs to the geometry
 * output buffer and (possibly) transform feedback buffers.
 */
static void
lower_emit_vertex(nir_builder *b, nir_intrinsic_instr *intr,
                  struct lower_gs_state *state)
{
   nir_def *total_vertices = intr->src[0].ssa;

   /* All previous invocations are first in the geometry output buffer */
   unsigned stream = nir_intrinsic_stream_id(intr);
   nir_def *invocation_vertex_base =
      previous_vertices(b, state, stream, calc_unrolled_id(b));

   /* Calculate the number of vertices that this invocation will produce. This
    * is calculated by the count shader and then prefix summed, so calculate the
    * difference to undo the action of the prefix sum.
    */
   nir_def *next = previous_vertices(b, state, stream,
                                     nir_iadd_imm(b, calc_unrolled_id(b), 1));
   nir_def *our_num_verts = nir_isub(b, next, invocation_vertex_base);

   /* We can only emit vertices within bounds, since other entries in the
    * geometry state buffer might belong to other invocations. This is required
    * to pass glsl-1.50-geometry-end-primitive (without geometry shaders racing
    * each other).
    *
    * TODO: This could be optimized many ways.
    */
   if (!state->rasterizer_discard && stream == 0) {
      nir_if *nif = nir_push_if(b, nir_ult(b, total_vertices, our_num_verts));
      {
         /* The index into the geometry output buffer */
         nir_def *vertex_id =
            nir_iadd(b, invocation_vertex_base, total_vertices);

         nir_def *buffer = load_geometry_param(b, output_buffer);
         nir_def *vertex_offset = nir_imul_imm(b, vertex_id, state->stride_B);
         nir_def *vertex_addr =
            nir_iadd(b, buffer, nir_u2u64(b, vertex_offset));

         /* Copy each output where it belongs */
         u_foreach_bit64(slot, outputs_rasterized(b->shader)) {
            nir_def *addr = nir_iadd_imm(b, vertex_addr, state->offset_B[slot]);
            nir_def *value = nir_load_var(b, state->outputs[slot][0]);
            unsigned comps = glsl_get_components(state->outputs[slot][0]->type);

            nir_store_global(b, addr, 4, value, nir_component_mask(comps));
         }
      }
      nir_pop_if(b, nif);
   }

   /* Transform feedback is written for each decomposed output primitive. Since
    * we're writing strips, that means we output XFB for each vertex after the
    * first complete primitive is formed.
    */
   unsigned first_prim = verts_in_output_prim(b->shader) - 1;
   nir_def *index_in_strip = intr->src[1].ssa;

   nir_push_if(b, nir_uge_imm(b, index_in_strip, first_prim));
   {
      write_xfb(b, state, nir_intrinsic_stream_id(intr), index_in_strip,
                intr->src[3].ssa);
   }
   nir_pop_if(b, NULL);

   /* Transform feedback writes out entire primitives during the emit_vertex. To
    * do that, we store the values at all vertices in the strip in a little ring
    * buffer. Index #0 is always the most recent primitive (so non-XFB code can
    * just grab index #0 without any checking). Index #1 is the previous vertex,
    * and index #2 is the vertex before that. Now that we've written XFB, since
    * we've emitted a vertex we need to cycle the ringbuffer, freeing up index
    * #0 for the next vertex that we are about to emit. We do that by copying
    * the first n - 1 vertices forward one slot, which has to happen with a
    * backwards copy implemented here.
    *
    * If we're lucky, all of these copies will be propagated away. If we're
    * unlucky, this involves at most 2 copies per component per XFB output per
    * vertex.
    */
   struct nir_xfb_info *xfb = b->shader->xfb_info;
   if (xfb) {
      u_foreach_bit64(slot, b->shader->info.outputs_written) {
         /* Note: if we're outputting points, verts_in_output_prim will be 1, so
          * this loop will not execute. This is intended: points are
          * self-contained primitives and do not need these copies.
          */
         for (int v = verts_in_output_prim(b->shader) - 1; v >= 1; --v) {
            nir_def *value = nir_load_var(b, state->outputs[slot][v - 1]);

            nir_store_var(b, state->outputs[slot][v], value,
                          nir_component_mask(value->num_components));
         }
      }
   }
}

static bool
lower_gs_instr(nir_builder *b, nir_intrinsic_instr *intr, void *state)
{
   b->cursor = nir_before_instr(&intr->instr);

   switch (intr->intrinsic) {
   case nir_intrinsic_set_vertex_and_primitive_count:
      /* This instruction is only for the count shader, so just remove */
      break;

   case nir_intrinsic_end_primitive_with_counter: {
      unsigned min = verts_in_output_prim(b->shader);

      /* We only write out complete primitives */
      nir_push_if(b, nir_uge_imm(b, intr->src[1].ssa, min));
      {
         lower_end_primitive(b, intr, state);
      }
      nir_pop_if(b, NULL);
      break;
   }

   case nir_intrinsic_emit_vertex_with_counter:
      lower_emit_vertex(b, intr, state);
      break;

   default:
      return false;
   }

   nir_instr_remove(&intr->instr);
   return true;
}

static bool
collect_components(nir_builder *b, nir_intrinsic_instr *intr, void *data)
{
   uint8_t *counts = data;
   if (intr->intrinsic != nir_intrinsic_store_output)
      return false;

   unsigned count = nir_intrinsic_component(intr) +
                    util_last_bit(nir_intrinsic_write_mask(intr));

   unsigned loc =
      nir_intrinsic_io_semantics(intr).location + nir_src_as_uint(intr->src[1]);

   uint8_t *total_count = &counts[loc];

   *total_count = MAX2(*total_count, count);
   return true;
}

/*
 * Create the pre-GS shader. This is a small compute 1x1x1 kernel that patches
 * up the VDM Index List command from the draw to read the produced geometry, as
 * well as updates transform feedack offsets and counters as applicable (TODO).
 */
static nir_shader *
agx_nir_create_pre_gs(struct lower_gs_state *state, const nir_shader *libagx,
                      bool indexed, struct nir_xfb_info *xfb,
                      unsigned vertices_per_prim, uint8_t streams)
{
   nir_builder b_ = nir_builder_init_simple_shader(
      MESA_SHADER_COMPUTE, &agx_nir_options, "Pre-GS patch up");
   nir_builder *b = &b_;

   /* Load the number of primitives input to the GS */
   nir_def *unrolled_in_prims = load_geometry_param(b, input_primitives);

   /* Setup the draw from the rasterization stream (0). */
   if (!state->rasterizer_discard) {
      libagx_build_gs_draw(b, nir_load_geometry_param_buffer_agx(b),
                           nir_imm_bool(b, indexed),
                           previous_vertices(b, state, 0, unrolled_in_prims),
                           previous_primitives(b, state, 0, unrolled_in_prims),
                           nir_imm_int(b, state->stride_B));
   }

   /* Determine the number of primitives generated in each stream */
   nir_def *in_prims[MAX_VERTEX_STREAMS], *prims[MAX_VERTEX_STREAMS];

   u_foreach_bit(i, streams) {
      in_prims[i] = previous_xfb_primitives(b, state, i, unrolled_in_prims);
      prims[i] = in_prims[i];

      add_counter(b, load_geometry_param(b, prims_generated_counter[i]),
                  prims[i]);
   }

   if (xfb) {
      /* Write XFB addresses */
      nir_def *offsets[4] = {NULL};
      u_foreach_bit(i, xfb->buffers_written) {
         offsets[i] = libagx_setup_xfb_buffer(
            b, nir_load_geometry_param_buffer_agx(b), nir_imm_int(b, i));
      }

      /* Now clamp to the number that XFB captures */
      for (unsigned i = 0; i < xfb->output_count; ++i) {
         nir_xfb_output_info output = xfb->outputs[i];

         unsigned buffer = output.buffer;
         unsigned stream = xfb->buffer_to_stream[buffer];
         unsigned stride = xfb->buffers[buffer].stride;
         unsigned words_written = util_bitcount(output.component_mask);
         unsigned bytes_written = words_written * 4;

         /* Primitive P will write up to (but not including) offset:
          *
          *    xfb_offset + ((P - 1) * (verts_per_prim * stride))
          *               + ((verts_per_prim - 1) * stride)
          *               + output_offset
          *               + output_size
          *
          * Given an XFB buffer of size xfb_size, we get the inequality:
          *
          *    floor(P) <= (stride + xfb_size - xfb_offset - output_offset -
          *                     output_size) // (stride * verts_per_prim)
          */
         nir_def *size = load_geometry_param(b, xfb_size[buffer]);
         size = nir_iadd_imm(b, size, stride - output.offset - bytes_written);
         size = nir_isub(b, size, offsets[buffer]);
         size = nir_imax(b, size, nir_imm_int(b, 0));
         nir_def *max_prims = nir_udiv_imm(b, size, stride * vertices_per_prim);

         prims[stream] = nir_umin(b, prims[stream], max_prims);
      }

      nir_def *any_overflow = nir_imm_false(b);

      u_foreach_bit(i, streams) {
         nir_def *overflow = nir_ult(b, prims[i], in_prims[i]);
         any_overflow = nir_ior(b, any_overflow, overflow);

         store_geometry_param(b, xfb_prims[i], prims[i]);

         add_counter(b, load_geometry_param(b, xfb_overflow[i]),
                     nir_b2i32(b, overflow));

         add_counter(b, load_geometry_param(b, xfb_prims_generated_counter[i]),
                     prims[i]);
      }

      add_counter(b, load_geometry_param(b, xfb_any_overflow),
                  nir_b2i32(b, any_overflow));

      /* Update XFB counters */
      u_foreach_bit(i, xfb->buffers_written) {
         uint32_t prim_stride_B = xfb->buffers[i].stride * vertices_per_prim;
         unsigned stream = xfb->buffer_to_stream[i];

         nir_def *off_ptr = load_geometry_param(b, xfb_offs_ptrs[i]);
         nir_def *size = nir_imul_imm(b, prims[stream], prim_stride_B);
         add_counter(b, off_ptr, size);
      }
   }

   /* Preprocess it */
   UNUSED struct agx_uncompiled_shader_info info;
   agx_preprocess_nir(b->shader, libagx, false, &info);

   return b->shader;
}

static bool
rewrite_invocation_id(nir_builder *b, nir_intrinsic_instr *intr, void *data)
{
   if (intr->intrinsic != nir_intrinsic_load_invocation_id)
      return false;

   b->cursor = nir_instr_remove(&intr->instr);
   nir_def_rewrite_uses(&intr->def, nir_u2uN(b, data, intr->def.bit_size));
   return true;
}

/*
 * Geometry shader instancing allows a GS to run multiple times. The number of
 * times is statically known and small. It's easiest to turn this into a loop
 * inside the GS, to avoid the feature "leaking" outside and affecting e.g. the
 * counts.
 */
static void
agx_nir_lower_gs_instancing(nir_shader *gs)
{
   unsigned nr_invocations = gs->info.gs.invocations;
   nir_function_impl *impl = nir_shader_get_entrypoint(gs);

   /* Each invocation can produce up to the shader-declared max_vertices, so
    * multiply it up for proper bounds check. Emitting more than the declared
    * max_vertices per invocation results in undefined behaviour, so erroneously
    * emitting more as asked on early invocations is a perfectly cromulent
    * behvaiour.
    */
   gs->info.gs.vertices_out *= gs->info.gs.invocations;

   /* Get the original function */
   nir_cf_list list;
   nir_cf_extract(&list, nir_before_impl(impl), nir_after_impl(impl));

   /* Create a builder for the wrapped function */
   nir_builder b = nir_builder_at(nir_after_block(nir_start_block(impl)));

   nir_variable *i =
      nir_local_variable_create(impl, glsl_uintN_t_type(16), NULL);
   nir_store_var(&b, i, nir_imm_intN_t(&b, 0, 16), ~0);
   nir_def *index = NULL;

   /* Create a loop in the wrapped function */
   nir_loop *loop = nir_push_loop(&b);
   {
      index = nir_load_var(&b, i);
      nir_push_if(&b, nir_uge_imm(&b, index, nr_invocations));
      {
         nir_jump(&b, nir_jump_break);
      }
      nir_pop_if(&b, NULL);

      b.cursor = nir_cf_reinsert(&list, b.cursor);
      nir_store_var(&b, i, nir_iadd_imm(&b, index, 1), ~0);

      /* Make sure we end the primitive between invocations. If the geometry
       * shader already ended the primitive, this will get optimized out.
       */
      nir_end_primitive(&b);
   }
   nir_pop_loop(&b, loop);

   /* We've mucked about with control flow */
   nir_metadata_preserve(impl, nir_metadata_none);

   /* Use the loop counter as the invocation ID each iteration */
   nir_shader_intrinsics_pass(gs, rewrite_invocation_id,
                              nir_metadata_block_index | nir_metadata_dominance,
                              index);
}

static void
link_libagx(nir_shader *nir, const nir_shader *libagx)
{
   nir_link_shader_functions(nir, libagx);
   NIR_PASS_V(nir, nir_inline_functions);
   NIR_PASS_V(nir, nir_remove_non_entrypoints);
   NIR_PASS_V(nir, nir_lower_indirect_derefs, nir_var_function_temp, 64);
   NIR_PASS_V(nir, nir_lower_vars_to_explicit_types,
              nir_var_shader_temp | nir_var_function_temp | nir_var_mem_shared |
                 nir_var_mem_global,
              glsl_get_cl_type_size_align);
   NIR_PASS_V(nir, nir_opt_deref);
   NIR_PASS_V(nir, nir_lower_vars_to_ssa);
   NIR_PASS_V(nir, nir_lower_explicit_io,
              nir_var_shader_temp | nir_var_function_temp | nir_var_mem_shared |
                 nir_var_mem_global,
              nir_address_format_62bit_generic);
}

void
agx_nir_lower_gs(nir_shader *gs, nir_shader *vs, const nir_shader *libagx,
                 struct agx_ia_key *ia, bool rasterizer_discard,
                 nir_shader **gs_count, nir_shader **gs_copy,
                 nir_shader **pre_gs, enum mesa_prim *out_mode,
                 unsigned *out_count_words)
{
   link_libagx(vs, libagx);

   /* Collect output component counts so we can size the geometry output buffer
    * appropriately, instead of assuming everything is vec4.
    */
   uint8_t component_counts[NUM_TOTAL_VARYING_SLOTS] = {0};
   nir_shader_intrinsics_pass(gs, collect_components, nir_metadata_all,
                              component_counts);

   /* If geometry shader instancing is used, lower it away before linking
    * anything. Otherwise, smash the invocation ID to zero.
    */
   if (gs->info.gs.invocations != 1) {
      NIR_PASS_V(gs, agx_nir_lower_gs_instancing);
   } else {
      nir_function_impl *impl = nir_shader_get_entrypoint(gs);
      nir_builder b = nir_builder_at(nir_before_impl(impl));

      NIR_PASS_V(gs, nir_shader_intrinsics_pass, rewrite_invocation_id,
                 nir_metadata_block_index | nir_metadata_dominance,
                 nir_imm_int(&b, 0));
   }

   /* Link VS into the GS */
   agx_nir_link_vs_gs(vs, gs);

   /* Lower geometry shader writes to contain all of the required counts, so we
    * know where in the various buffers we should write vertices.
    */
   NIR_PASS_V(gs, nir_lower_gs_intrinsics,
              nir_lower_gs_intrinsics_count_primitives |
                 nir_lower_gs_intrinsics_per_stream |
                 nir_lower_gs_intrinsics_count_vertices_per_primitive |
                 nir_lower_gs_intrinsics_overwrite_incomplete |
                 nir_lower_gs_intrinsics_always_end_primitive |
                 nir_lower_gs_intrinsics_count_decomposed_primitives);

   /* Clean up after all that lowering we did */
   bool progress = false;
   do {
      progress = false;
      NIR_PASS(progress, gs, nir_lower_var_copies);
      NIR_PASS(progress, gs, nir_lower_variable_initializers,
               nir_var_shader_temp);
      NIR_PASS(progress, gs, nir_lower_vars_to_ssa);
      NIR_PASS(progress, gs, nir_copy_prop);
      NIR_PASS(progress, gs, nir_opt_constant_folding);
      NIR_PASS(progress, gs, nir_opt_algebraic);
      NIR_PASS(progress, gs, nir_opt_cse);
      NIR_PASS(progress, gs, nir_opt_dead_cf);
      NIR_PASS(progress, gs, nir_opt_dce);

      /* Unrolling lets us statically determine counts more often, which
       * otherwise would not be possible with multiple invocations even in the
       * simplest of cases.
       */
      NIR_PASS(progress, gs, nir_opt_loop_unroll);
   } while (progress);

   if (ia->indirect_multidraw)
      NIR_PASS_V(gs, agx_nir_lower_multidraw, ia);

   NIR_PASS_V(gs, nir_shader_intrinsics_pass, lower_id,
              nir_metadata_block_index | nir_metadata_dominance, NULL);

   link_libagx(gs, libagx);

   NIR_PASS_V(gs, nir_lower_idiv,
              &(const nir_lower_idiv_options){.allow_fp16 = true});

   /* All those variables we created should've gone away by now */
   NIR_PASS_V(gs, nir_remove_dead_variables, nir_var_function_temp, NULL);

   /* If we know counts at compile-time we can simplify, so try to figure out
    * the counts statically.
    */
   struct lower_gs_state gs_state = {
      .stride_B = 0,
      .rasterizer_discard = rasterizer_discard,
   };

   nir_gs_count_vertices_and_primitives(
      gs, gs_state.static_count[GS_COUNTER_VERTICES],
      gs_state.static_count[GS_COUNTER_PRIMITIVES],
      gs_state.static_count[GS_COUNTER_XFB_PRIMITIVES], 4);

   /* Anything we don't know statically will be tracked by the count buffer.
    * Determine the layout for it.
    */
   for (unsigned i = 0; i < MAX_VERTEX_STREAMS; ++i) {
      for (unsigned c = 0; c < GS_NUM_COUNTERS; ++c) {
         gs_state.count_index[i][c] =
            (gs_state.static_count[c][i] < 0) ? gs_state.count_stride_el++ : -1;
      }
   }

   /* If there is any unknown count, we need a geometry count shader */
   if (gs_state.count_stride_el > 0)
      *gs_count = agx_nir_create_geometry_count_shader(gs, libagx, &gs_state);
   else
      *gs_count = NULL;

   /* Geometry shader outputs are staged to temporaries */
   struct lower_output_to_var_state state = {.arrayed = false};

   u_foreach_bit64(slot, gs->info.outputs_written) {
      const char *slot_name =
         gl_varying_slot_name_for_stage(slot, MESA_SHADER_GEOMETRY);

      for (unsigned i = 0; i < MAX_PRIM_OUT_SIZE; ++i) {
         gs_state.outputs[slot][i] = nir_variable_create(
            gs, nir_var_shader_temp,
            glsl_vector_type(GLSL_TYPE_UINT, component_counts[slot]),
            ralloc_asprintf(gs, "%s-%u", slot_name, i));
      }

      state.outputs[slot] = gs_state.outputs[slot][0];

      /* Assume fp32 output */
      unsigned size_B = 4 * component_counts[slot];
      gs_state.offset_B[slot] = gs_state.stride_B;
      gs_state.stride_B += size_B;
   }

   NIR_PASS_V(gs, nir_shader_instructions_pass, lower_output_to_var,
              nir_metadata_block_index | nir_metadata_dominance, &state);

   /* Set flatshade_first. For now this is always a constant, but in the future
    * we will want this to be dynamic.
    */
   {
      nir_builder b =
         nir_builder_at(nir_before_impl(nir_shader_get_entrypoint(gs)));

      gs_state.flatshade_first = nir_imm_bool(&b, ia->flatshade_first);
   }

   NIR_PASS_V(gs, nir_shader_intrinsics_pass, lower_gs_instr, nir_metadata_none,
              &gs_state);

   /* Clean up after all that lowering we did */
   nir_lower_global_vars_to_local(gs);
   do {
      progress = false;
      NIR_PASS(progress, gs, nir_lower_var_copies);
      NIR_PASS(progress, gs, nir_lower_variable_initializers,
               nir_var_shader_temp);
      NIR_PASS(progress, gs, nir_lower_vars_to_ssa);
      NIR_PASS(progress, gs, nir_copy_prop);
      NIR_PASS(progress, gs, nir_opt_constant_folding);
      NIR_PASS(progress, gs, nir_opt_algebraic);
      NIR_PASS(progress, gs, nir_opt_cse);
      NIR_PASS(progress, gs, nir_opt_dead_cf);
      NIR_PASS(progress, gs, nir_opt_dce);
      NIR_PASS(progress, gs, nir_opt_loop_unroll);
   } while (progress);

   /* All those variables we created should've gone away by now */
   NIR_PASS_V(gs, nir_remove_dead_variables, nir_var_function_temp, NULL);

   NIR_PASS_V(gs, nir_opt_sink, ~0);
   NIR_PASS_V(gs, nir_opt_move, ~0);
   NIR_PASS_V(gs, nir_shader_intrinsics_pass, lower_id,
              nir_metadata_block_index | nir_metadata_dominance, NULL);

   /* Create auxiliary programs */
   *gs_copy = agx_nir_create_gs_copy_shader(
      &gs_state, outputs_rasterized(gs), gs->info.clip_distance_array_size,
      gs->info.cull_distance_array_size, gs->info.gs.output_primitive);

   *pre_gs = agx_nir_create_pre_gs(
      &gs_state, libagx, gs->info.gs.output_primitive != MESA_PRIM_POINTS,
      gs->xfb_info, verts_in_output_prim(gs), gs->info.gs.active_stream_mask);

   /* Signal what primitive we want to draw the GS Copy VS with */
   *out_mode = gs->info.gs.output_primitive;
   *out_count_words = gs_state.count_stride_el;
}

nir_shader *
agx_nir_prefix_sum_gs(const nir_shader *libagx, unsigned words)
{
   nir_builder b = nir_builder_init_simple_shader(
      MESA_SHADER_COMPUTE, &agx_nir_options, "GS prefix sum");

   uint32_t subgroup_size = 32;
   b.shader->info.workgroup_size[0] = subgroup_size;
   b.shader->info.workgroup_size[1] = words;

   libagx_prefix_sum(&b, load_geometry_param(&b, count_buffer),
                     load_geometry_param(&b, input_primitives),
                     nir_imm_int(&b, words),
                     nir_trim_vector(&b, nir_load_local_invocation_id(&b), 2));

   UNUSED struct agx_uncompiled_shader_info info;
   agx_preprocess_nir(b.shader, libagx, false, &info);
   return b.shader;
}

nir_shader *
agx_nir_gs_setup_indirect(const nir_shader *libagx, enum mesa_prim prim,
                          bool multidraw)
{
   nir_builder b = nir_builder_init_simple_shader(
      MESA_SHADER_COMPUTE, &agx_nir_options, "GS indirect setup");

   if (multidraw) {
      uint32_t subgroup_size = 32;
      b.shader->info.workgroup_size[0] = subgroup_size;
   }

   libagx_gs_setup_indirect(
      &b, nir_load_geometry_param_buffer_agx(&b),
      nir_load_input_assembly_buffer_agx(&b), nir_imm_int(&b, prim),
      nir_channel(&b, nir_load_local_invocation_id(&b), 0),
      nir_imm_bool(&b, multidraw));

   UNUSED struct agx_uncompiled_shader_info info;
   agx_preprocess_nir(b.shader, libagx, false, &info);
   return b.shader;
}

nir_shader *
agx_nir_unroll_restart(const nir_shader *libagx, enum mesa_prim prim,
                       unsigned index_size_B)
{
   nir_builder b = nir_builder_init_simple_shader(
      MESA_SHADER_COMPUTE, &agx_nir_options, "Primitive restart unroll");

   nir_def *ia = nir_load_input_assembly_buffer_agx(&b);
   nir_def *draw = nir_channel(&b, nir_load_workgroup_id(&b), 0);
   nir_def *mode = nir_imm_int(&b, prim);

   if (index_size_B == 1)
      libagx_unroll_restart_u8(&b, ia, mode, draw);
   else if (index_size_B == 2)
      libagx_unroll_restart_u16(&b, ia, mode, draw);
   else if (index_size_B == 4)
      libagx_unroll_restart_u32(&b, ia, mode, draw);
   else
      unreachable("invalid index size");

   UNUSED struct agx_uncompiled_shader_info info;
   agx_preprocess_nir(b.shader, libagx, false, &info);
   return b.shader;
}
