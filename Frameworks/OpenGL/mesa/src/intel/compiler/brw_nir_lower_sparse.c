/*
 * Copyright (c) 2023 Intel Corporation
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

#include "brw_nir.h"
#include "compiler/nir/nir_builder.h"

/*
 * This pass lowers a few of the sparse instructions to something HW can
 * handle.
 *
 * The image_*_sparse_load intrinsics are lowered into 2 instructions, a
 * regular image_*_load intrinsic and a sparse texture txf operation and
 * reconstructs the sparse vector of the original intrinsic using the 2 new
 * values. We need to do this because our backend implements image load/store
 * using the dataport and the dataport unit doesn't provide residency
 * information. We need to use the sampler for residency.
 *
 * The is_sparse_texels_resident intrinsic is lowered to a bit checking
 * operation as the data reported by the sampler is a single bit per lane in
 * the first component.
 *
 * The tex_* instructions with a compare value need to be lower into 2
 * instructions due to a HW limitation :
 *
 * SKL PRMs, Volume 7: 3D-Media-GPGPU, Messages, SIMD Payloads :
 *
 *    "The Pixel Null Mask field, when enabled via the Pixel Null Mask Enable
 *     will be incorect for sample_c when applied to a surface with 64-bit per
 *     texel format such as R16G16BA16_UNORM. Pixel Null mask Enable may
 *     incorrectly report pixels as referencing a Null surface."
 */

static void
lower_is_sparse_texels_resident(nir_builder *b, nir_intrinsic_instr *intrin)
{
   b->cursor = nir_instr_remove(&intrin->instr);

   nir_def_rewrite_uses(
      &intrin->def,
      nir_i2b(b, nir_iand(b, intrin->src[0].ssa,
                              nir_ishl(b, nir_imm_int(b, 1),
                                          nir_load_subgroup_invocation(b)))));
}

static void
lower_sparse_residency_code_and(nir_builder *b, nir_intrinsic_instr *intrin)
{
   b->cursor = nir_instr_remove(&intrin->instr);

   nir_def_rewrite_uses(
      &intrin->def,
      nir_iand(b, intrin->src[0].ssa, intrin->src[1].ssa));
}

static void
lower_sparse_image_load(nir_builder *b, nir_intrinsic_instr *intrin)
{
   b->cursor = nir_instr_remove(&intrin->instr);

   nir_def *img_load;
   nir_intrinsic_instr *new_intrin;
   if (intrin->intrinsic == nir_intrinsic_image_sparse_load) {
      img_load = nir_image_load(b,
                                intrin->num_components - 1,
                                intrin->def.bit_size,
                                intrin->src[0].ssa,
                                intrin->src[1].ssa,
                                intrin->src[2].ssa,
                                intrin->src[3].ssa);
      new_intrin = nir_instr_as_intrinsic(img_load->parent_instr);
      nir_intrinsic_set_range_base(new_intrin, nir_intrinsic_range_base(intrin));
   } else {
      img_load = nir_bindless_image_load(b,
                                         intrin->num_components - 1,
                                         intrin->def.bit_size,
                                         intrin->src[0].ssa,
                                         intrin->src[1].ssa,
                                         intrin->src[2].ssa,
                                         intrin->src[3].ssa);
      new_intrin = nir_instr_as_intrinsic(img_load->parent_instr);
   }

   nir_intrinsic_set_image_array(new_intrin, nir_intrinsic_image_array(intrin));
   nir_intrinsic_set_image_dim(new_intrin, nir_intrinsic_image_dim(intrin));
   nir_intrinsic_set_format(new_intrin, nir_intrinsic_format(intrin));
   nir_intrinsic_set_access(new_intrin, nir_intrinsic_access(intrin));
   nir_intrinsic_set_dest_type(new_intrin, nir_intrinsic_dest_type(intrin));

   nir_def *dests[NIR_MAX_VEC_COMPONENTS];
   for (unsigned i = 0; i < intrin->num_components - 1; i++) {
      dests[i] = nir_channel(b, img_load, i);
   }

   /* Use texture instruction to compute residency */
   nir_tex_instr *tex = nir_tex_instr_create(b->shader, 3);

   tex->op = nir_texop_txf;
   /* We don't care about the dest type since we're not using any of that
    * data.
    */
   tex->dest_type = nir_type_float32;
   tex->is_array = nir_intrinsic_image_array(intrin);
   tex->is_shadow = false;
   tex->sampler_index = 0;
   tex->is_sparse = true;

   tex->src[0].src_type = intrin->intrinsic == nir_intrinsic_image_sparse_load ?
                          nir_tex_src_texture_offset :
                          nir_tex_src_texture_handle;
   tex->src[0].src = nir_src_for_ssa(intrin->src[0].ssa);

   tex->coord_components = nir_image_intrinsic_coord_components(intrin);
   nir_def *coord;
   if (nir_intrinsic_image_dim(intrin) == GLSL_SAMPLER_DIM_CUBE &&
       nir_intrinsic_image_array(intrin)) {
      tex->coord_components++;

      nir_def *img_layer = nir_channel(b, intrin->src[1].ssa, 2);
      nir_def *tex_slice = nir_idiv(b, img_layer, nir_imm_int(b, 6));
      nir_def *tex_face =
         nir_iadd(b, img_layer, nir_ineg(b, nir_imul_imm(b, tex_slice, 6)));
      nir_def *comps[4] = {
         nir_channel(b, intrin->src[1].ssa, 0),
         nir_channel(b, intrin->src[1].ssa, 1),
         tex_face,
         tex_slice
      };
      coord = nir_vec(b, comps, 4);
   } else {
      coord = nir_channels(b, intrin->src[1].ssa,
                           nir_component_mask(tex->coord_components));
   }
   tex->src[1].src_type = nir_tex_src_coord;
   tex->src[1].src = nir_src_for_ssa(coord);

   tex->src[2].src_type = nir_tex_src_lod;
   tex->src[2].src = nir_src_for_ssa(nir_imm_int(b, 0));

   nir_def_init(&tex->instr, &tex->def, 5,
                intrin->def.bit_size);

   nir_builder_instr_insert(b, &tex->instr);

   dests[intrin->num_components - 1] = nir_channel(b, &tex->def, 4);

   nir_def_rewrite_uses(
      &intrin->def,
      nir_vec(b, dests, intrin->num_components));
}

static void
lower_tex_compare(nir_builder *b, nir_tex_instr *tex, int compare_idx)
{
   b->cursor = nir_after_instr(&tex->instr);

   /* Clone the original instruction */
   nir_tex_instr *sparse_tex = nir_instr_as_tex(nir_instr_clone(b->shader, &tex->instr));
   nir_def_init(&sparse_tex->instr, &sparse_tex->def,
                tex->def.num_components, tex->def.bit_size);
   nir_builder_instr_insert(b, &sparse_tex->instr);

   /* Drop the compare source on the cloned instruction */
   nir_tex_instr_remove_src(sparse_tex, compare_idx);

   /* Drop the residency query on the original tex instruction */
   tex->is_sparse = false;
   tex->def.num_components = tex->def.num_components - 1;

   nir_def *new_comps[NIR_MAX_VEC_COMPONENTS];
   for (unsigned i = 0; i < tex->def.num_components; i++)
      new_comps[i] = nir_channel(b, &tex->def, i);
   new_comps[tex->def.num_components] =
      nir_channel(b, &sparse_tex->def, tex->def.num_components);

   nir_def *new_vec = nir_vec(b, new_comps, sparse_tex->def.num_components);

   nir_def_rewrite_uses_after(&tex->def, new_vec, new_vec->parent_instr);
}

static bool
lower_sparse_intrinsics(nir_builder *b, nir_instr *instr, void *cb_data)
{
   switch (instr->type) {
   case nir_instr_type_intrinsic: {
      nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
      switch (intrin->intrinsic) {
      case nir_intrinsic_image_sparse_load:
      case nir_intrinsic_bindless_image_sparse_load:
         lower_sparse_image_load(b, intrin);
         return true;

      case nir_intrinsic_is_sparse_texels_resident:
         lower_is_sparse_texels_resident(b, intrin);
         return true;

      case nir_intrinsic_sparse_residency_code_and:
         lower_sparse_residency_code_and(b, intrin);
         return true;

      default:
         return false;
      }
   }

   case nir_instr_type_tex: {
      nir_tex_instr *tex = nir_instr_as_tex(instr);
      int comp_idx = nir_tex_instr_src_index(tex, nir_tex_src_comparator);
      if (comp_idx != -1 && tex->is_sparse) {
         lower_tex_compare(b, tex, comp_idx);
         return true;
      }
      return false;
   }

   default:
      return false;
   }
}

bool
brw_nir_lower_sparse_intrinsics(nir_shader *nir)
{
   return nir_shader_instructions_pass(nir, lower_sparse_intrinsics,
                                       nir_metadata_block_index |
                                       nir_metadata_dominance,
                                       NULL);
}
