/*
 * Copyright (C) 2020-2023 Collabora, Ltd.
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "compiler/nir/nir_builder.h"
#include "pan_context.h"

struct ctx {
   struct panfrost_sysvals *sysvals;
   struct hash_table_u64 *sysval_to_id;
   unsigned sysval_ubo;
};

static unsigned
lookup_sysval(struct hash_table_u64 *sysval_to_id,
              struct panfrost_sysvals *sysvals, int sysval)
{
   /* Try to lookup */
   void *cached = _mesa_hash_table_u64_search(sysval_to_id, sysval);

   if (cached) {
      unsigned id = ((uintptr_t)cached) - 1;
      assert(id < MAX_SYSVAL_COUNT);
      assert(sysvals->sysvals[id] == sysval);
      return id;
   }

   /* Else assign */
   unsigned id = sysvals->sysval_count++;
   assert(id < MAX_SYSVAL_COUNT);
   _mesa_hash_table_u64_insert(sysval_to_id, sysval,
                               (void *)((uintptr_t)id + 1));
   sysvals->sysvals[id] = sysval;
   return id;
}

static unsigned
sysval_for_intrinsic(nir_intrinsic_instr *intr, unsigned *offset)
{
   switch (intr->intrinsic) {
   case nir_intrinsic_load_ssbo_address:
      return PAN_SYSVAL(SSBO, nir_src_as_uint(intr->src[0]));
   case nir_intrinsic_get_ssbo_size:
      *offset = 8;
      return PAN_SYSVAL(SSBO, nir_src_as_uint(intr->src[0]));

   case nir_intrinsic_load_sampler_lod_parameters_pan:
      /* This is only used for a workaround on Mali-T720, where we don't
       * support dynamic samplers.
       */
      return PAN_SYSVAL(SAMPLER, nir_src_as_uint(intr->src[0]));

   case nir_intrinsic_load_xfb_address:
      return PAN_SYSVAL(XFB, nir_intrinsic_base(intr));

   case nir_intrinsic_load_work_dim:
      return PAN_SYSVAL_WORK_DIM;

   case nir_intrinsic_load_sample_positions_pan:
      return PAN_SYSVAL_SAMPLE_POSITIONS;

   case nir_intrinsic_load_num_vertices:
      return PAN_SYSVAL_NUM_VERTICES;

   case nir_intrinsic_load_first_vertex:
      return PAN_SYSVAL_VERTEX_INSTANCE_OFFSETS;
   case nir_intrinsic_load_base_vertex:
      *offset = 4;
      return PAN_SYSVAL_VERTEX_INSTANCE_OFFSETS;
   case nir_intrinsic_load_base_instance:
      *offset = 8;
      return PAN_SYSVAL_VERTEX_INSTANCE_OFFSETS;

   case nir_intrinsic_load_draw_id:
      return PAN_SYSVAL_DRAWID;

   case nir_intrinsic_load_multisampled_pan:
      return PAN_SYSVAL_MULTISAMPLED;

   case nir_intrinsic_load_viewport_scale:
      return PAN_SYSVAL_VIEWPORT_SCALE;

   case nir_intrinsic_load_viewport_offset:
      return PAN_SYSVAL_VIEWPORT_OFFSET;

   case nir_intrinsic_load_num_workgroups:
      return PAN_SYSVAL_NUM_WORK_GROUPS;

   case nir_intrinsic_load_workgroup_size:
      return PAN_SYSVAL_LOCAL_GROUP_SIZE;

   case nir_intrinsic_load_rt_conversion_pan: {
      unsigned size = nir_alu_type_get_type_size(nir_intrinsic_src_type(intr));
      unsigned rt = nir_intrinsic_base(intr);

      return PAN_SYSVAL(RT_CONVERSION, rt | (size << 4));
   }

   case nir_intrinsic_image_size: {
      uint32_t uindex = nir_src_as_uint(intr->src[0]);
      bool is_array = nir_intrinsic_image_array(intr);
      unsigned dim = nir_intrinsic_dest_components(intr) - is_array;

      return PAN_SYSVAL(IMAGE_SIZE, PAN_TXS_SYSVAL_ID(uindex, dim, is_array));
   }

   default:
      return ~0;
   }
}

static bool
lower(nir_builder *b, nir_instr *instr, void *data)
{
   struct ctx *ctx = data;
   nir_def *old = NULL;
   unsigned sysval = ~0, offset = 0;
   b->cursor = nir_before_instr(instr);

   if (instr->type == nir_instr_type_intrinsic) {
      nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
      old = &intr->def;
      sysval = sysval_for_intrinsic(intr, &offset);

      if (sysval == ~0)
         return false;
   } else if (instr->type == nir_instr_type_tex) {
      nir_tex_instr *tex = nir_instr_as_tex(instr);
      old = &tex->def;

      if (tex->op != nir_texop_txs)
         return false;

      /* XXX: This is broken for dynamic indexing */
      sysval = PAN_SYSVAL(TEXTURE_SIZE,
                          PAN_TXS_SYSVAL_ID(tex->texture_index,
                                            nir_tex_instr_dest_size(tex) -
                                               (tex->is_array ? 1 : 0),
                                            tex->is_array));
   } else {
      return false;
   }

   /* Allocate a UBO for the sysvals if we haven't yet */
   if (ctx->sysvals->sysval_count == 0)
      ctx->sysval_ubo = b->shader->info.num_ubos++;

   unsigned vec4_index = lookup_sysval(ctx->sysval_to_id, ctx->sysvals, sysval);
   unsigned ubo_offset = (vec4_index * 16) + offset;

   b->cursor = nir_after_instr(instr);
   nir_def *val = nir_load_ubo(
      b, old->num_components, old->bit_size, nir_imm_int(b, ctx->sysval_ubo),
      nir_imm_int(b, ubo_offset), .align_mul = old->bit_size / 8,
      .align_offset = 0, .range_base = offset, .range = old->bit_size / 8);
   nir_def_rewrite_uses(old, val);
   return true;
}

bool
panfrost_nir_lower_sysvals(nir_shader *shader, struct panfrost_sysvals *sysvals)
{
   bool progress = false;

   /* The lowerings for SSBOs, etc require constants, so fold now */
   do {
      progress = false;

      NIR_PASS(progress, shader, nir_copy_prop);
      NIR_PASS(progress, shader, nir_opt_constant_folding);
      NIR_PASS(progress, shader, nir_opt_dce);
   } while (progress);

   struct ctx ctx = {
      .sysvals = sysvals,
      .sysval_to_id = _mesa_hash_table_u64_create(NULL),
   };

   memset(sysvals, 0, sizeof(*sysvals));

   nir_shader_instructions_pass(
      shader, lower, nir_metadata_block_index | nir_metadata_dominance, &ctx);

   _mesa_hash_table_u64_destroy(ctx.sysval_to_id);
   return true;
}
