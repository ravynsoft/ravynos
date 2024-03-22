/*
 * Copyright © 2021 Intel Corporation
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

/**
 * This lowering pass supports (as configured via nir_lower_image_options)
 * image related conversions:
 *   + cube array size lowering. The size operation is converted from cube
 *     size to a 2d-array with the z component divided by 6.
 */

#include "nir.h"
#include "nir_builder.h"

static void
lower_cube_size(nir_builder *b, nir_intrinsic_instr *intrin)
{
   assert(nir_intrinsic_image_dim(intrin) == GLSL_SAMPLER_DIM_CUBE);

   b->cursor = nir_before_instr(&intrin->instr);

   nir_intrinsic_instr *_2darray_size =
      nir_instr_as_intrinsic(nir_instr_clone(b->shader, &intrin->instr));
   nir_intrinsic_set_image_dim(_2darray_size, GLSL_SAMPLER_DIM_2D);
   nir_intrinsic_set_image_array(_2darray_size, true);
   nir_builder_instr_insert(b, &_2darray_size->instr);

   nir_def *size = nir_instr_def(&_2darray_size->instr);
   nir_scalar comps[NIR_MAX_VEC_COMPONENTS] = { 0 };
   unsigned coord_comps = intrin->def.num_components;
   for (unsigned c = 0; c < coord_comps; c++) {
      if (c == 2) {
         comps[2] = nir_get_scalar(nir_idiv(b, nir_channel(b, size, 2), nir_imm_int(b, 6)), 0);
      } else {
         comps[c] = nir_get_scalar(size, c);
      }
   }

   nir_def *vec = nir_vec_scalars(b, comps, intrin->def.num_components);
   nir_def_rewrite_uses(&intrin->def, vec);
   nir_instr_remove(&intrin->instr);
   nir_instr_free(&intrin->instr);
}

/* Adjust the sample index according to AMD FMASK (fragment mask).
 *
 * For uncompressed MSAA surfaces, FMASK should return 0x76543210,
 * which is the identity mapping. Each nibble says which physical sample
 * should be fetched to get that sample.
 *
 * For example, 0x11111100 means there are only 2 samples stored and
 * the second sample covers 3/4 of the pixel. When reading samples 0
 * and 1, return physical sample 0 (determined by the first two 0s
 * in FMASK), otherwise return physical sample 1.
 *
 * The sample index should be adjusted as follows:
 *   sample_index = ubfe(fmask, sample_index * 4, 3);
 *
 * Only extract 3 bits because EQAA can generate number 8 in FMASK, which
 * means the physical sample index is unknown. We can map 8 to any valid
 * sample index, and extracting only 3 bits will map it to 0, which works
 * with all MSAA modes.
 */
static void
lower_image_to_fragment_mask_load(nir_builder *b, nir_intrinsic_instr *intrin)
{
   b->cursor = nir_before_instr(&intrin->instr);

   nir_intrinsic_op fmask_op;
   switch (intrin->intrinsic) {
   case nir_intrinsic_image_load:
      fmask_op = nir_intrinsic_image_fragment_mask_load_amd;
      break;
   case nir_intrinsic_image_deref_load:
      fmask_op = nir_intrinsic_image_deref_fragment_mask_load_amd;
      break;
   case nir_intrinsic_bindless_image_load:
      fmask_op = nir_intrinsic_bindless_image_fragment_mask_load_amd;
      break;
   default:
      unreachable("bad intrinsic");
      break;
   }

   nir_def *fmask =
      nir_image_fragment_mask_load_amd(b, intrin->src[0].ssa, intrin->src[1].ssa,
                                       .image_dim = nir_intrinsic_image_dim(intrin),
                                       .image_array = nir_intrinsic_image_array(intrin),
                                       .format = nir_intrinsic_format(intrin),
                                       .access = nir_intrinsic_access(intrin));

   /* fix intrinsic op */
   nir_intrinsic_instr *fmask_load = nir_instr_as_intrinsic(fmask->parent_instr);
   fmask_load->intrinsic = fmask_op;

   /* extract real color buffer index from fmask buffer */
   nir_def *sample_index_old = intrin->src[2].ssa;
   nir_def *fmask_offset = nir_ishl_imm(b, sample_index_old, 2);
   nir_def *fmask_width = nir_imm_int(b, 3);
   nir_def *sample_index_new = nir_ubfe(b, fmask, fmask_offset, fmask_width);

   /* fix color buffer load */
   nir_src_rewrite(&intrin->src[2], sample_index_new);

   /* Mark uses fmask to prevent lower this intrinsic again. */
   enum gl_access_qualifier access = nir_intrinsic_access(intrin);
   nir_intrinsic_set_access(intrin, access | ACCESS_FMASK_LOWERED_AMD);
}

static void
lower_image_samples_identical_to_fragment_mask_load(nir_builder *b, nir_intrinsic_instr *intrin)
{
   b->cursor = nir_before_instr(&intrin->instr);

   nir_intrinsic_instr *fmask_load =
      nir_instr_as_intrinsic(nir_instr_clone(b->shader, &intrin->instr));

   switch (intrin->intrinsic) {
   case nir_intrinsic_image_samples_identical:
      fmask_load->intrinsic = nir_intrinsic_image_fragment_mask_load_amd;
      break;
   case nir_intrinsic_image_deref_samples_identical:
      fmask_load->intrinsic = nir_intrinsic_image_deref_fragment_mask_load_amd;
      break;
   case nir_intrinsic_bindless_image_samples_identical:
      fmask_load->intrinsic = nir_intrinsic_bindless_image_fragment_mask_load_amd;
      break;
   default:
      unreachable("bad intrinsic");
      break;
   }

   nir_def_init(&fmask_load->instr, &fmask_load->def, 1, 32);
   nir_builder_instr_insert(b, &fmask_load->instr);

   nir_def *samples_identical = nir_ieq_imm(b, &fmask_load->def, 0);
   nir_def_rewrite_uses(&intrin->def, samples_identical);

   nir_instr_remove(&intrin->instr);
   nir_instr_free(&intrin->instr);
}

static bool
lower_image_intrin(nir_builder *b, nir_intrinsic_instr *intrin, void *state)
{
   const nir_lower_image_options *options = state;

   switch (intrin->intrinsic) {
   case nir_intrinsic_image_size:
   case nir_intrinsic_image_deref_size:
   case nir_intrinsic_bindless_image_size:
      if (options->lower_cube_size &&
          nir_intrinsic_image_dim(intrin) == GLSL_SAMPLER_DIM_CUBE) {
         lower_cube_size(b, intrin);
         return true;
      }
      return false;

   case nir_intrinsic_image_load:
   case nir_intrinsic_image_deref_load:
   case nir_intrinsic_bindless_image_load:
      if (options->lower_to_fragment_mask_load_amd &&
          nir_intrinsic_image_dim(intrin) == GLSL_SAMPLER_DIM_MS &&
          /* Don't lower again. */
          !(nir_intrinsic_access(intrin) & ACCESS_FMASK_LOWERED_AMD)) {
         lower_image_to_fragment_mask_load(b, intrin);
         return true;
      }
      return false;

   case nir_intrinsic_image_samples_identical:
   case nir_intrinsic_image_deref_samples_identical:
   case nir_intrinsic_bindless_image_samples_identical:
      if (options->lower_to_fragment_mask_load_amd &&
          nir_intrinsic_image_dim(intrin) == GLSL_SAMPLER_DIM_MS) {
         lower_image_samples_identical_to_fragment_mask_load(b, intrin);
         return true;
      }
      return false;

   case nir_intrinsic_image_samples:
   case nir_intrinsic_image_deref_samples:
   case nir_intrinsic_bindless_image_samples: {
      if (options->lower_image_samples_to_one) {
         b->cursor = nir_after_instr(&intrin->instr);
         nir_def *samples = nir_imm_intN_t(b, 1, intrin->def.bit_size);
         nir_def_rewrite_uses(&intrin->def, samples);
         return true;
      }
      return false;
   }
   default:
      return false;
   }
}

bool
nir_lower_image(nir_shader *nir, const nir_lower_image_options *options)
{
   return nir_shader_intrinsics_pass(nir, lower_image_intrin,
                                     nir_metadata_block_index |
                                        nir_metadata_dominance,
                                     (void *)options);
}
