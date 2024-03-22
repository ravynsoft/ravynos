/*
 * Copyright © 2023 Valve Corporation
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

#include "nir/nir.h"
#include "nir/nir_builder.h"
#include "radv_nir.h"
#include "radv_private.h"

typedef struct {
   bool dynamic_rasterization_samples;
   unsigned num_rasterization_samples;
   unsigned rast_prim;
} lower_fs_barycentric_state;

static nir_def *
lower_interp_center_smooth(nir_builder *b, nir_def *offset)
{
   nir_def *pull_model = nir_load_barycentric_model(b, 32);

   nir_def *deriv_x =
      nir_vec3(b, nir_fddx_fine(b, nir_channel(b, pull_model, 0)), nir_fddx_fine(b, nir_channel(b, pull_model, 1)),
               nir_fddx_fine(b, nir_channel(b, pull_model, 2)));
   nir_def *deriv_y =
      nir_vec3(b, nir_fddy_fine(b, nir_channel(b, pull_model, 0)), nir_fddy_fine(b, nir_channel(b, pull_model, 1)),
               nir_fddy_fine(b, nir_channel(b, pull_model, 2)));

   nir_def *offset_x = nir_channel(b, offset, 0);
   nir_def *offset_y = nir_channel(b, offset, 1);

   nir_def *adjusted_x = nir_fadd(b, pull_model, nir_fmul(b, deriv_x, offset_x));
   nir_def *adjusted = nir_fadd(b, adjusted_x, nir_fmul(b, deriv_y, offset_y));

   nir_def *ij = nir_vec2(b, nir_channel(b, adjusted, 0), nir_channel(b, adjusted, 1));

   /* Get W by using the reciprocal of 1/W. */
   nir_def *w = nir_frcp(b, nir_channel(b, adjusted, 2));

   return nir_fmul(b, ij, w);
}

static nir_def *
lower_barycentric_coord_at_offset(nir_builder *b, nir_def *src, enum glsl_interp_mode mode)
{
   if (mode == INTERP_MODE_SMOOTH)
      return lower_interp_center_smooth(b, src);

   return nir_load_barycentric_at_offset(b, 32, src, .interp_mode = mode);
}

static nir_def *
lower_barycentric_coord_at_sample(nir_builder *b, lower_fs_barycentric_state *state, nir_intrinsic_instr *intrin)
{
   const enum glsl_interp_mode mode = (enum glsl_interp_mode)nir_intrinsic_interp_mode(intrin);
   nir_def *num_samples = nir_load_rasterization_samples_amd(b);
   nir_def *new_dest;

   if (state->dynamic_rasterization_samples) {
      nir_def *res1, *res2;

      nir_push_if(b, nir_ieq_imm(b, num_samples, 1));
      {
         res1 = nir_load_barycentric_pixel(b, 32, .interp_mode = nir_intrinsic_interp_mode(intrin));
      }
      nir_push_else(b, NULL);
      {
         nir_def *sample_pos = nir_load_sample_positions_amd(b, 32, intrin->src[0].ssa, num_samples);

         /* sample_pos -= 0.5 */
         sample_pos = nir_fadd_imm(b, sample_pos, -0.5f);

         res2 = lower_barycentric_coord_at_offset(b, sample_pos, mode);
      }
      nir_pop_if(b, NULL);

      new_dest = nir_if_phi(b, res1, res2);
   } else {
      if (!state->num_rasterization_samples) {
         new_dest = nir_load_barycentric_pixel(b, 32, .interp_mode = nir_intrinsic_interp_mode(intrin));
      } else {
         nir_def *sample_pos = nir_load_sample_positions_amd(b, 32, intrin->src[0].ssa, num_samples);

         /* sample_pos -= 0.5 */
         sample_pos = nir_fadd_imm(b, sample_pos, -0.5f);

         new_dest = lower_barycentric_coord_at_offset(b, sample_pos, mode);
      }
   }

   return new_dest;
}

static nir_def *
get_interp_param(nir_builder *b, lower_fs_barycentric_state *state, nir_intrinsic_instr *intrin)
{
   const enum glsl_interp_mode mode = (enum glsl_interp_mode)nir_intrinsic_interp_mode(intrin);

   if (intrin->intrinsic == nir_intrinsic_load_barycentric_coord_pixel) {
      return nir_load_barycentric_pixel(b, 32, .interp_mode = mode);
   } else if (intrin->intrinsic == nir_intrinsic_load_barycentric_coord_at_offset) {
      return lower_barycentric_coord_at_offset(b, intrin->src[0].ssa, mode);
   } else if (intrin->intrinsic == nir_intrinsic_load_barycentric_coord_at_sample) {
      return lower_barycentric_coord_at_sample(b, state, intrin);
   } else if (intrin->intrinsic == nir_intrinsic_load_barycentric_coord_centroid) {
      return nir_load_barycentric_centroid(b, 32, .interp_mode = mode);
   } else {
      assert(intrin->intrinsic == nir_intrinsic_load_barycentric_coord_sample);
      return nir_load_barycentric_sample(b, 32, .interp_mode = mode);
   }

   return NULL;
}

static nir_def *
lower_point(nir_builder *b)
{
   nir_def *coords[3];

   coords[0] = nir_imm_float(b, 1.0f);
   coords[1] = nir_imm_float(b, 0.0f);
   coords[2] = nir_imm_float(b, 0.0f);

   return nir_vec(b, coords, 3);
}

static nir_def *
lower_line(nir_builder *b, nir_def *p1, nir_def *p2)
{
   nir_def *coords[3];

   coords[1] = nir_fadd(b, p1, p2);
   coords[0] = nir_fsub_imm(b, 1.0f, coords[1]);
   coords[2] = nir_imm_float(b, 0.0f);

   return nir_vec(b, coords, 3);
}

static nir_def *
lower_triangle(nir_builder *b, nir_def *p1, nir_def *p2)
{
   nir_def *v0_bary[3], *v1_bary[3], *v2_bary[3];
   nir_def *coords[3];

   /* Compute the provoking vertex ID:
    *
    * quad_id = thread_id >> 2
    * provoking_vtx_id = (provoking_vtx >> (quad_id << 1)) & 3
    */
   nir_def *quad_id = nir_ushr_imm(b, nir_load_subgroup_invocation(b), 2);
   nir_def *provoking_vtx = nir_load_provoking_vtx_amd(b);
   nir_def *provoking_vtx_id = nir_ubfe(b, provoking_vtx, nir_ishl_imm(b, quad_id, 1), nir_imm_int(b, 2));

   /* Compute barycentrics. */
   v0_bary[0] = nir_fsub(b, nir_fsub_imm(b, 1.0f, p2), p1);
   v0_bary[1] = p1;
   v0_bary[2] = p2;

   v1_bary[0] = p1;
   v1_bary[1] = p2;
   v1_bary[2] = nir_fsub(b, nir_fsub_imm(b, 1.0f, p2), p1);

   v2_bary[0] = p2;
   v2_bary[1] = nir_fsub(b, nir_fsub_imm(b, 1.0f, p2), p1);
   v2_bary[2] = p1;

   /* Select barycentrics for the given provoking vertex ID. */
   for (unsigned i = 0; i < 3; i++) {
      coords[i] = nir_bcsel(b, nir_ieq_imm(b, provoking_vtx_id, 2), v2_bary[i],
                            nir_bcsel(b, nir_ieq_imm(b, provoking_vtx_id, 1), v1_bary[i], v0_bary[i]));
   }

   return nir_vec(b, coords, 3);
}

static bool
lower_load_barycentric_coord(nir_builder *b, lower_fs_barycentric_state *state, nir_intrinsic_instr *intrin)
{
   nir_def *interp, *p1, *p2;
   nir_def *new_dest;

   b->cursor = nir_after_instr(&intrin->instr);

   /* When the rasterization primitive isn't known at compile time (GPL), load it. */
   if (state->rast_prim == -1) {
      nir_def *rast_prim = nir_load_rasterization_primitive_amd(b);
      nir_def *res1, *res2;

      nir_def *is_point = nir_ieq_imm(b, rast_prim, V_028A6C_POINTLIST);
      nir_if *if_point = nir_push_if(b, is_point);
      {
         res1 = lower_point(b);
      }
      nir_push_else(b, if_point);
      {
         nir_def *res_line, *res_triangle;

         interp = get_interp_param(b, state, intrin);
         p1 = nir_channel(b, interp, 0);
         p2 = nir_channel(b, interp, 1);

         nir_def *is_line = nir_ieq_imm(b, rast_prim, V_028A6C_LINESTRIP);
         nir_if *if_line = nir_push_if(b, is_line);
         {
            res_line = lower_line(b, p1, p2);
         }
         nir_push_else(b, if_line);
         {
            res_triangle = lower_triangle(b, p1, p2);
         }
         nir_pop_if(b, if_line);

         res2 = nir_if_phi(b, res_line, res_triangle);
      }
      nir_pop_if(b, if_point);

      new_dest = nir_if_phi(b, res1, res2);
   } else {
      if (state->rast_prim == V_028A6C_POINTLIST) {
         new_dest = lower_point(b);
      } else {
         interp = get_interp_param(b, state, intrin);
         p1 = nir_channel(b, interp, 0);
         p2 = nir_channel(b, interp, 1);

         if (state->rast_prim == V_028A6C_LINESTRIP) {
            new_dest = lower_line(b, p1, p2);
         } else {
            assert(state->rast_prim == V_028A6C_TRISTRIP);
            new_dest = lower_triangle(b, p1, p2);
         }
      }
   }

   nir_def_rewrite_uses(&intrin->def, new_dest);
   nir_instr_remove(&intrin->instr);

   return true;
}

bool
radv_nir_lower_fs_barycentric(nir_shader *shader, const struct radv_pipeline_key *key, unsigned rast_prim)
{
   nir_function_impl *impl = nir_shader_get_entrypoint(shader);
   bool progress = false;

   nir_builder b;

   lower_fs_barycentric_state state = {
      .dynamic_rasterization_samples = key->dynamic_rasterization_samples,
      .num_rasterization_samples = key->ps.num_samples,
      .rast_prim = rast_prim,
   };

   nir_foreach_function (function, shader) {
      if (!function->impl)
         continue;

      b = nir_builder_create(function->impl);

      nir_foreach_block (block, impl) {
         nir_foreach_instr_safe (instr, block) {
            if (instr->type != nir_instr_type_intrinsic)
               continue;

            nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
            if (intrin->intrinsic != nir_intrinsic_load_barycentric_coord_pixel &&
                intrin->intrinsic != nir_intrinsic_load_barycentric_coord_centroid &&
                intrin->intrinsic != nir_intrinsic_load_barycentric_coord_sample &&
                intrin->intrinsic != nir_intrinsic_load_barycentric_coord_at_offset &&
                intrin->intrinsic != nir_intrinsic_load_barycentric_coord_at_sample)
               continue;

            progress |= lower_load_barycentric_coord(&b, &state, intrin);
         }
      }
   }

   nir_metadata_preserve(impl, progress ? nir_metadata_none : nir_metadata_all);

   return progress;
}
