/*
 * Copyright © 2019 Igalia S.L.
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

#include "ir3_nir.h"

/**
 * A pass which detects tex instructions which are candidate to be executed
 * prior to FS shader start, and change them to nir_texop_tex_prefetch.
 */

static int
coord_offset(nir_def *ssa)
{
   nir_instr *parent_instr = ssa->parent_instr;

   /* The coordinate of a texture sampling instruction eligible for
    * pre-fetch is either going to be a load_interpolated_input/
    * load_input, or a vec2 assembling non-swizzled components of
    * a load_interpolated_input/load_input (due to varying packing)
    */

   if (parent_instr->type == nir_instr_type_alu) {
      nir_alu_instr *alu = nir_instr_as_alu(parent_instr);

      if (alu->op != nir_op_vec2)
         return -1;

      int base_src_offset = coord_offset(alu->src[0].src.ssa);
      if (base_src_offset < 0)
         return -1;

      int base_offset = base_src_offset + alu->src[0].swizzle[0];

      /* NOTE it might be possible to support more than 2D? */
      for (int i = 1; i < 2; i++) {
         int nth_src_offset = coord_offset(alu->src[i].src.ssa);
         if (nth_src_offset < 0)
            return -1;
         int nth_offset = nth_src_offset + alu->src[i].swizzle[0];

         if (nth_offset != (base_offset + i))
            return -1;
      }

      return base_offset;
   }

   if (parent_instr->type != nir_instr_type_intrinsic)
      return -1;

   nir_intrinsic_instr *input = nir_instr_as_intrinsic(parent_instr);

   if (input->intrinsic != nir_intrinsic_load_interpolated_input)
      return -1;

   /* Happens with lowered load_barycentric_at_offset */
   if (input->src[0].ssa->parent_instr->type != nir_instr_type_intrinsic)
      return -1;

   nir_intrinsic_instr *interp =
      nir_instr_as_intrinsic(input->src[0].ssa->parent_instr);

   if (interp->intrinsic != nir_intrinsic_load_barycentric_pixel)
      return -1;

   /* interpolation modes such as noperspective aren't covered by the other
    * test, we need to explicitly check for them here.
    */
   unsigned interp_mode = nir_intrinsic_interp_mode(interp);
   if (interp_mode != INTERP_MODE_NONE && interp_mode != INTERP_MODE_SMOOTH)
      return -1;

   /* we also need a const input offset: */
   if (!nir_src_is_const(input->src[1]))
      return -1;

   unsigned base = nir_src_as_uint(input->src[1]) + nir_intrinsic_base(input);
   unsigned comp = nir_intrinsic_component(input);

   return (4 * base) + comp;
}

int
ir3_nir_coord_offset(nir_def *ssa)
{

   assert(ssa->num_components == 2);
   return coord_offset(ssa);
}

static bool
has_src(nir_tex_instr *tex, nir_tex_src_type type)
{
   return nir_tex_instr_src_index(tex, type) >= 0;
}

static bool
ok_bindless_src(nir_tex_instr *tex, nir_tex_src_type type)
{
   int idx = nir_tex_instr_src_index(tex, type);
   assert(idx >= 0);
   nir_intrinsic_instr *bindless = ir3_bindless_resource(tex->src[idx].src);

   /* TODO from SP_FS_BINDLESS_PREFETCH[n] it looks like this limit should
    * be 1<<8 ?
    */
   return nir_src_is_const(bindless->src[0]) &&
          (nir_src_as_uint(bindless->src[0]) < (1 << 16));
}

/**
 * Check that we will be able to encode the tex/samp parameters
 * successfully.  These limits are based on the layout of
 * SP_FS_PREFETCH[n] and SP_FS_BINDLESS_PREFETCH[n], so at some
 * point (if those regs changes) they may become generation
 * specific.
 */
static bool
ok_tex_samp(nir_tex_instr *tex)
{
   if (has_src(tex, nir_tex_src_texture_handle)) {
      /* bindless case: */

      assert(has_src(tex, nir_tex_src_sampler_handle));

      return ok_bindless_src(tex, nir_tex_src_texture_handle) &&
             ok_bindless_src(tex, nir_tex_src_sampler_handle);
   } else {
      assert(!has_src(tex, nir_tex_src_texture_offset));
      assert(!has_src(tex, nir_tex_src_sampler_offset));

      return (tex->texture_index <= 0x1f) && (tex->sampler_index <= 0xf);
   }
}

static bool
lower_tex_prefetch_block(nir_block *block)
{
   bool progress = false;

   nir_foreach_instr_safe (instr, block) {
      if (instr->type != nir_instr_type_tex)
         continue;

      nir_tex_instr *tex = nir_instr_as_tex(instr);
      if (tex->op != nir_texop_tex)
         continue;

      if (has_src(tex, nir_tex_src_bias) || has_src(tex, nir_tex_src_lod) ||
          has_src(tex, nir_tex_src_comparator) ||
          has_src(tex, nir_tex_src_projector) ||
          has_src(tex, nir_tex_src_offset) || has_src(tex, nir_tex_src_ddx) ||
          has_src(tex, nir_tex_src_ddy) || has_src(tex, nir_tex_src_ms_index) ||
          has_src(tex, nir_tex_src_texture_offset) ||
          has_src(tex, nir_tex_src_sampler_offset))
         continue;

      /* only prefetch for simple 2d tex fetch case */
      if (tex->sampler_dim != GLSL_SAMPLER_DIM_2D || tex->is_array)
         continue;

      if (!ok_tex_samp(tex))
         continue;

      int idx = nir_tex_instr_src_index(tex, nir_tex_src_coord);
      /* First source should be the sampling coordinate. */
      nir_tex_src *coord = &tex->src[idx];

      if (ir3_nir_coord_offset(coord->src.ssa) >= 0) {
         tex->op = nir_texop_tex_prefetch;

         progress |= true;
      }
   }

   return progress;
}

static bool
lower_tex_prefetch_func(nir_function_impl *impl)
{
   /* Only instructions in the the outer-most block are considered eligible for
    * pre-dispatch, because they need to be move-able to the beginning of the
    * shader to avoid locking down the register holding the pre-fetched result
    * for too long. However if there is a preamble we should skip the preamble
    * and only look in the first block after the preamble instead, because that
    * corresponds to the first block in the original program and texture fetches
    * in the preamble are never pre-dispatchable.
    */
   nir_block *block = nir_start_block(impl);

   nir_if *nif = nir_block_get_following_if(block);
   if (nif) {
      nir_instr *cond = nif->condition.ssa->parent_instr;
      if (cond->type == nir_instr_type_intrinsic &&
          nir_instr_as_intrinsic(cond)->intrinsic ==
          nir_intrinsic_preamble_start_ir3) {
         block = nir_cf_node_as_block(nir_cf_node_next(&nif->cf_node));
      }
   }

   bool progress = lower_tex_prefetch_block(block);

   if (progress) {
      nir_metadata_preserve(impl,
                            nir_metadata_block_index | nir_metadata_dominance);
   }

   return progress;
}

bool
ir3_nir_lower_tex_prefetch(nir_shader *shader)
{
   bool progress = false;

   assert(shader->info.stage == MESA_SHADER_FRAGMENT);

   nir_foreach_function (function, shader) {
      /* Only texture sampling instructions inside the main function
       * are eligible for pre-dispatch.
       */
      if (!function->impl || !function->is_entrypoint)
         continue;

      progress |= lower_tex_prefetch_func(function->impl);
   }

   return progress;
}
