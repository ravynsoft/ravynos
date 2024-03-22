/*
 * Copyright (c) 2021 Lima Project
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
#include "lima_ir.h"

static nir_def *
get_proj_index(nir_instr *coord_instr, nir_instr *proj_instr,
               int coord_components, int *proj_idx)
{
   *proj_idx = -1;
   if (coord_instr->type != nir_instr_type_alu ||
       proj_instr->type != nir_instr_type_alu)
      return NULL;

   nir_alu_instr *coord_alu = nir_instr_as_alu(coord_instr);
   nir_alu_instr *proj_alu = nir_instr_as_alu(proj_instr);

   if (coord_alu->op != nir_op_mov ||
       proj_alu->op != nir_op_mov)
      return NULL;

   nir_def *coord_src_ssa = coord_alu->src[0].src.ssa;
   nir_def *proj_src_ssa = proj_alu->src[0].src.ssa;

   if (coord_src_ssa != proj_src_ssa)
      return NULL;

   if (coord_src_ssa->parent_instr->type != nir_instr_type_intrinsic)
      return NULL;

   nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(coord_src_ssa->parent_instr);
   if (intrin->intrinsic != nir_intrinsic_load_input)
      return NULL;

   if (intrin->def.num_components != 4)
      return NULL;

   /* Coords must be in .xyz */
   for (int i = 0; i < coord_components; i++) {
      if (coord_alu->src[0].swizzle[i] != i)
         return NULL;
   }

   *proj_idx = proj_alu->src[0].swizzle[0];

   return coord_src_ssa;
}

static bool
lima_nir_lower_txp_instr(nir_builder *b, nir_instr *instr,
                         UNUSED void *cb_data)
{
   if (instr->type != nir_instr_type_tex)
      return false;

   nir_tex_instr *tex = nir_instr_as_tex(instr);

   int proj_idx = nir_tex_instr_src_index(tex, nir_tex_src_projector);
   int coords_idx = nir_tex_instr_src_index(tex, nir_tex_src_coord);

   if (proj_idx < 0)
      return false;

   switch (tex->sampler_dim) {
   case GLSL_SAMPLER_DIM_RECT:
   case GLSL_SAMPLER_DIM_1D:
   case GLSL_SAMPLER_DIM_2D:
   case GLSL_SAMPLER_DIM_3D:
      break;
   default:
      return false;
   }

   b->cursor = nir_before_instr(&tex->instr);

   /* Merge coords and projector into single backend-specific source.
    * It's easy if texture2DProj argument is vec3, it's more tricky with
    * vec4 since NIR just drops Z component that we need, so we have to
    * step back and use load_input SSA instead of mov as a source for
    * newly constructed vec4
    */
   nir_def *proj_ssa = tex->src[proj_idx].src.ssa;
   nir_def *coords_ssa = tex->src[coords_idx].src.ssa;

   int proj_idx_in_vec = -1;
   nir_def *load_input = get_proj_index(coords_ssa->parent_instr,
                                            proj_ssa->parent_instr,
                                            tex->coord_components,
                                            &proj_idx_in_vec);
   nir_def *combined;
   if (load_input && proj_idx_in_vec == 3) {
      unsigned xyzw[] = { 0, 1, 2, 3 };
      combined = nir_swizzle(b, load_input, xyzw, 4);
      tex->coord_components = 4;
   } else if (load_input && proj_idx_in_vec == 2) {
      unsigned xyz[] = { 0, 1, 2 };
      combined = nir_swizzle(b, load_input, xyz, 3);
      tex->coord_components = 3;
   } else {
      switch (tex->coord_components) {
      default:
      case 1:
         /* We still need vec3 for 1D textures, so duplicate coordinate */
         combined = nir_vec3(b,
                             nir_channel(b, coords_ssa, 0),
                             nir_channel(b, coords_ssa, 0),
                             nir_channel(b, proj_ssa, 0));
         tex->coord_components = 3;
         break;
      case 2:
         combined = nir_vec3(b,
                             nir_channel(b, coords_ssa, 0),
                             nir_channel(b, coords_ssa, 1),
                             nir_channel(b, proj_ssa, 0));
         tex->coord_components = 3;
         break;
      case 3:
         combined = nir_vec4(b,
                             nir_channel(b, coords_ssa, 0),
                             nir_channel(b, coords_ssa, 1),
                             nir_channel(b, coords_ssa, 2),
                             nir_channel(b, proj_ssa, 0));
         tex->coord_components = 4;
      }
   }

   nir_tex_instr_remove_src(tex, nir_tex_instr_src_index(tex, nir_tex_src_coord));
   nir_tex_instr_remove_src(tex, nir_tex_instr_src_index(tex, nir_tex_src_projector));
   nir_tex_instr_add_src(tex, nir_tex_src_backend1, combined);

   return true;
}

bool
lima_nir_lower_txp(nir_shader *shader)
{
   return nir_shader_instructions_pass(shader, lima_nir_lower_txp_instr,
                                       nir_metadata_block_index |
                                       nir_metadata_dominance,
                                       NULL);
}
