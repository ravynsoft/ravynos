/*
 * Copyright 2022 Alyssa Rosenzweig
 * SPDX-License-Identifier: MIT
 */

#include "compiler/nir/nir_builder.h"
#include "util/bitset.h"
#include "util/u_dynarray.h"
#include "agx_state.h"
#include "nir.h"
#include "nir_builder_opcodes.h"
#include "nir_intrinsics.h"
#include "nir_intrinsics_indices.h"

#define AGX_TEXTURE_DESC_STRIDE 24

/*
 * Lower all system values to uniform loads. This pass tries to compact ranges
 * of contiguous uploaded uniforms to reduce the draw-time overhead of uploading
 * many tiny ranges. To do so, it works in 4 steps:
 *
 * 1. Lower NIR sysvals to loads from the system value buffers.
 * 2. Walk the NIR, recording loads from system value buffers.
 * 2. Walk the ranges of uniforms needed, compacting into contiguous ranges.
 * 3. Fill in the load_preamble instructions with the real uniforms.
 */

#define MAX_TABLE_SIZE sizeof(struct agx_stage_uniforms)
static_assert(sizeof(struct agx_draw_uniforms) <= MAX_TABLE_SIZE, "packed");

struct table_state {
   /* Bitset of 16-bit uniforms pushed */
   BITSET_DECLARE(pushed, MAX_TABLE_SIZE / 2);

   /* Element size in 16-bit units, so we may split ranges of different sizes
    * to guarantee natural alignment.
    */
   uint8_t element_size[MAX_TABLE_SIZE / 2];
};

struct state {
   /* Array of nir_intrinsic_instr's to fix up at the end */
   struct util_dynarray loads;

   struct table_state tables[AGX_NUM_SYSVAL_TABLES];
};

static nir_def *
load_sysval(nir_builder *b, unsigned dim, unsigned bitsize, uint8_t table,
            uint16_t offset)
{
   return nir_load_sysval_agx(b, dim, bitsize, .desc_set = table,
                              .binding = offset);
}

static nir_def *
load_sysval_root(nir_builder *b, unsigned dim, unsigned bitsize, void *ptr)
{
   return load_sysval(b, dim, bitsize, AGX_SYSVAL_TABLE_ROOT, (uintptr_t)ptr);
}

static nir_def *
load_sysval_indirect(nir_builder *b, unsigned dim, unsigned bitsize,
                     uint8_t table, void *base, nir_def *offset_el)
{
   nir_scalar scalar = {offset_el, 0};
   unsigned stride = (dim * bitsize) / 8;

   if (nir_scalar_is_const(scalar)) {
      /* Load the sysval directly */
      return load_sysval(
         b, dim, bitsize, table,
         (uintptr_t)base + (nir_scalar_as_uint(scalar) * stride));
   } else {
      /* Load the base address of the table */
      struct agx_draw_uniforms *u = NULL;
      nir_def *table_base = load_sysval_root(b, 1, 64, &u->tables[table]);

      /* Load address of the array in the table */
      nir_def *array_base = nir_iadd_imm(b, table_base, (uintptr_t)base);

      /* Index into the table and load */
      nir_def *address = nir_iadd(
         b, array_base, nir_u2u64(b, nir_imul_imm(b, offset_el, stride)));
      return nir_load_global_constant(b, address, bitsize / 8, dim, bitsize);
   }
}

static unsigned
stage_table(nir_builder *b)
{
   assert(b->shader->info.stage < PIPE_SHADER_TYPES);
   return AGX_SYSVAL_STAGE(b->shader->info.stage);
}

static nir_def *
load_ubo(nir_builder *b, nir_intrinsic_instr *intr, void *bases)
{
   nir_def *base =
      load_sysval_indirect(b, 1, 64, stage_table(b), bases, intr->src[0].ssa);

   nir_def *address = nir_iadd(b, base, nir_u2u64(b, intr->src[1].ssa));

   return nir_load_global_constant(b, address, nir_intrinsic_align(intr),
                                   intr->num_components, intr->def.bit_size);
}

static nir_def *
load_texture_handle(nir_builder *b, nir_intrinsic_instr *intr, void *base)
{
   nir_def *uniform =
      nir_load_sysval_agx(b, 1, 64, .desc_set = stage_table(b),
                          .binding = (uintptr_t)base, .flags = ~0);

   return nir_vec2(b, nir_u2u32(b, uniform),
                   nir_imul_imm(b, intr->src[0].ssa, AGX_TEXTURE_DESC_STRIDE));
}

static nir_def *
lower_intrinsic(nir_builder *b, nir_intrinsic_instr *intr,
                bool lower_draw_params)
{
   struct agx_draw_uniforms *u = NULL;
   struct agx_stage_uniforms *s = NULL;

   switch (intr->intrinsic) {
   case nir_intrinsic_load_ubo:
      return load_ubo(b, intr, s->ubo_base);
   case nir_intrinsic_load_texture_handle_agx:
      return load_texture_handle(b, intr, &s->texture_base);
   case nir_intrinsic_load_sampler_handle_agx:
      return load_sysval_indirect(b, 1, 16, stage_table(b), &s->sampler_handle,
                                  intr->src[0].ssa);
   case nir_intrinsic_load_vbo_base_agx:
      return load_sysval_indirect(b, 1, 64, AGX_SYSVAL_TABLE_ROOT, &u->vbo_base,
                                  intr->src[0].ssa);
   case nir_intrinsic_load_blend_const_color_r_float:
      return load_sysval_root(b, 1, 32, &u->blend_constant[0]);
   case nir_intrinsic_load_blend_const_color_g_float:
      return load_sysval_root(b, 1, 32, &u->blend_constant[1]);
   case nir_intrinsic_load_blend_const_color_b_float:
      return load_sysval_root(b, 1, 32, &u->blend_constant[2]);
   case nir_intrinsic_load_blend_const_color_a_float:
      return load_sysval_root(b, 1, 32, &u->blend_constant[3]);
   case nir_intrinsic_load_api_sample_mask_agx:
      return load_sysval_root(b, 1, 16, &u->sample_mask);
   case nir_intrinsic_load_sample_positions_agx:
      return load_sysval_root(b, 1, 32, &u->ppp_multisamplectl);
   case nir_intrinsic_load_ssbo_address:
      return load_sysval_indirect(b, 1, 64, stage_table(b), &s->ssbo_base,
                                  intr->src[0].ssa);
   case nir_intrinsic_get_ubo_size:
      return load_sysval_indirect(b, 1, 32, stage_table(b), &s->ubo_size,
                                  intr->src[0].ssa);
   case nir_intrinsic_get_ssbo_size:
      return load_sysval_indirect(b, 1, 32, stage_table(b), &s->ssbo_size,
                                  intr->src[0].ssa);
   case nir_intrinsic_load_layer_id_written_agx:
      return load_sysval_root(b, 1, 16, &u->layer_id_written);
   case nir_intrinsic_load_input_assembly_buffer_agx:
      return load_sysval_root(b, 1, 64, &u->input_assembly);
   case nir_intrinsic_load_geometry_param_buffer_agx:
      return load_sysval_root(b, 1, 64, &u->geometry_params);
   case nir_intrinsic_load_fixed_point_size_agx:
      return load_sysval_root(b, 1, 32, &u->fixed_point_size);
   case nir_intrinsic_load_tex_sprite_mask_agx:
      return load_sysval_root(b, 1, 16, &u->sprite_mask);
   default:
      break;
   }

   if (!lower_draw_params)
      return NULL;

   switch (intr->intrinsic) {
   case nir_intrinsic_load_num_workgroups:
      return load_sysval(b, 3, 32, AGX_SYSVAL_TABLE_GRID, 0);
   case nir_intrinsic_load_first_vertex:
      return load_sysval(b, 1, 32, AGX_SYSVAL_TABLE_PARAMS, 0);
   case nir_intrinsic_load_base_instance:
      return load_sysval(b, 1, 32, AGX_SYSVAL_TABLE_PARAMS, 4);
   case nir_intrinsic_load_base_vertex:
      /* first vertex if indexed, 0 otherwise. More efficient for our hw than
       * the lowering in NIR.
       */
      return nir_bcsel(
         b, nir_i2b(b, load_sysval_root(b, 1, 16, &u->is_indexed_draw)),
         load_sysval(b, 1, 32, AGX_SYSVAL_TABLE_PARAMS, 0), nir_imm_int(b, 0));
   case nir_intrinsic_load_draw_id:
      return load_sysval_root(b, 1, 32, &u->draw_id);
   default:
      return NULL;
   }
}

/* Step 1. Lower NIR sysvals */
static bool
lower_sysvals(nir_builder *b, nir_instr *instr, void *data)
{
   bool *lower_draw_params = data;
   b->cursor = nir_before_instr(instr);
   nir_def *old;
   nir_def *replacement = NULL;

   if (instr->type == nir_instr_type_intrinsic) {
      nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
      old = &intr->def;
      replacement = lower_intrinsic(b, intr, *lower_draw_params);
   } else if (instr->type == nir_instr_type_tex) {
      nir_tex_instr *tex = nir_instr_as_tex(instr);
      old = &tex->def;

      if (tex->op != nir_texop_lod_bias_agx)
         return false;

      struct agx_stage_uniforms *s = NULL;

      int src_idx = nir_tex_instr_src_index(tex, nir_tex_src_texture_offset);
      if (src_idx >= 0) {
         replacement = load_sysval_indirect(
            b, 1, 16, stage_table(b), s->lod_bias, tex->src[src_idx].src.ssa);
      } else {
         replacement = load_sysval(b, 1, 16, stage_table(b),
                                   (uintptr_t)&s->lod_bias[tex->sampler_index]);
      }
   }

   if (replacement != NULL) {
      nir_def_rewrite_uses(old, replacement);
      return true;
   } else {
      return false;
   }
}

/* Step 2: Record system value loads */
static bool
record_loads(nir_builder *b, nir_intrinsic_instr *intr, void *data)
{
   if (intr->intrinsic != nir_intrinsic_load_sysval_agx)
      return false;

   assert(intr->def.bit_size >= 16 && "no 8-bit sysvals");
   unsigned dim = intr->def.num_components;
   unsigned element_size = intr->def.bit_size / 16;
   unsigned length = dim * element_size;

   struct state *state = data;
   struct table_state *table = &state->tables[nir_intrinsic_desc_set(intr)];
   unsigned offset = nir_intrinsic_binding(intr);
   assert((offset % 2) == 0 && "all entries are aligned by ABI");

   BITSET_SET_RANGE(table->pushed, (offset / 2), (offset / 2) + length - 1);

   for (unsigned i = 0; i < length; ++i) {
      if (table->element_size[(offset / 2) + i])
         assert((table->element_size[(offset / 2) + i]) == element_size);
      else
         table->element_size[(offset / 2) + i] = element_size;
   }

   util_dynarray_append(&state->loads, nir_intrinsic_instr *, intr);
   return false;
}

/* Step 3: Decide where to push the system values */
static struct agx_push_range *
find_push_range_containing(struct agx_compiled_shader *shader, uint8_t table,
                           uint16_t offset)
{
   for (unsigned i = 0; i < shader->push_range_count; ++i) {
      struct agx_push_range *range = &shader->push[i];

      if (range->table != table)
         continue;

      /* range->length is 16-bit words, need to convert. offset is bytes. */
      uint16_t length_B = range->length * 2;

      if (range->offset <= offset && offset < (range->offset + length_B))
         return range;
   }

   unreachable("no containing range");
}

static unsigned
lay_out_table(struct agx_compiled_shader *shader, struct table_state *state,
              unsigned table_index, unsigned uniform)
{
   unsigned start, end;
   BITSET_FOREACH_RANGE(start, end, state->pushed, sizeof(state->pushed) * 8) {
      unsigned range_start = start;

      do {
         uint8_t size = state->element_size[range_start];

         /* Find a range of constant element size. [range_start, range_end).
          * Ranges may be at most 64 halfs.
          */
         unsigned range_end;
         for (range_end = range_start + 1;
              range_end < end && state->element_size[range_end] == size &&
              range_end < range_start + 64;
              ++range_end)
            ;

         /* Now make the range with the given size (naturally aligned) */
         uniform = ALIGN_POT(uniform, size);

         assert((shader->push_range_count < ARRAY_SIZE(shader->push)) &&
                "AGX_MAX_PUSH_RANGES must be an upper bound");

         /* Offsets must be aligned to 4 bytes, this may require pushing a
          * little more than intended (otherwise we would need extra copies)
          */
         range_start = ROUND_DOWN_TO(range_start, 4 / 2);

         shader->push[shader->push_range_count++] = (struct agx_push_range){
            .uniform = uniform,
            .table = table_index,
            .offset = range_start * 2 /* bytes, not elements */,
            .length = (range_end - range_start),
         };

         uniform += (range_end - range_start);
         range_start = range_end;
      } while (range_start < end);
   }

   return uniform;
}

static unsigned
lay_out_uniforms(struct agx_compiled_shader *shader, struct state *state)
{
   unsigned uniform = 0;

   /* Lay out each system value table. We do this backwards to ensure the first
    * uniform goes to the bindless texture base.
    */
   for (int t = AGX_NUM_SYSVAL_TABLES - 1; t >= 0; --t)
      uniform = lay_out_table(shader, &state->tables[t], t, uniform);

   /* Step 4: Fill in the loads */
   util_dynarray_foreach(&state->loads, nir_intrinsic_instr *, intr_) {
      nir_intrinsic_instr *intr = *intr_;
      uint8_t table = nir_intrinsic_desc_set(intr);
      uint16_t offset = nir_intrinsic_binding(intr);
      bool load_uniform_location = nir_intrinsic_flags(intr);

      struct agx_push_range *range =
         find_push_range_containing(shader, table, offset);
      unsigned base = range->uniform + ((offset - range->offset) / 2);

      nir_builder b = nir_builder_at(nir_instr_remove(&(intr->instr)));
      nir_def *repl;

      if (load_uniform_location) {
         repl = nir_imm_int(&b, base);
      } else {
         repl = nir_load_preamble(&b, intr->def.num_components,
                                  intr->def.bit_size, .base = base);
      }

      nir_def_rewrite_uses(&intr->def, repl);
   }

   return uniform;
}

bool
agx_nir_lower_sysvals(nir_shader *shader, bool lower_draw_params)
{
   return nir_shader_instructions_pass(
      shader, lower_sysvals, nir_metadata_block_index | nir_metadata_dominance,
      &lower_draw_params);
}

bool
agx_nir_layout_uniforms(nir_shader *shader,
                        struct agx_compiled_shader *compiled,
                        unsigned *push_size)
{
   struct state state = {0};
   nir_shader_intrinsics_pass(shader, record_loads,
                              nir_metadata_block_index | nir_metadata_dominance,
                              &state);

   *push_size = lay_out_uniforms(compiled, &state);

   util_dynarray_fini(&state.loads);

   /* Make sure texture handles have constants associated */
   nir_opt_constant_folding(shader);

   return true;
}
