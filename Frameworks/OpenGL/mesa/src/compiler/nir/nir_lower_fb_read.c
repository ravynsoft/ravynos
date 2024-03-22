/*
 * Copyright © 2019 Google, Inc.
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

#include "nir.h"
#include "nir_builder.h"

/* Lowing for fragment shader load_output.
 *
 * This pass supports the blend_equation_advanced, where a fragment
 * shader loads the output (fragcolor) to read the current framebuffer.
 * It does this by lowering the output read to a txf_ms_fb instruction.
 * This instruction works similarly to a normal txf_ms except without
 * taking a texture source argument.  (The driver backend is expected
 * to wire this up to a free texture slot which is configured to read
 * from the framebuffer.)
 *
 * This should be run after lower_wpos_ytransform, because the tex
 * coordinates should be the physical fragcoord, not the logical
 * y-flipped coord.
 *
 * Note that this pass explicitly does *not* add a sampler uniform
 * (as txf_ms_fb does not reference a texture).  The driver backend
 * is going to want nif->info.num_textures to include the count of
 * number of textures *not* including the one it inserts to sample
 * from the framebuffer, so it more easily knows where to insert the
 * hidden texture to read from the fb.
 */

static bool
nir_lower_fb_read_instr(nir_builder *b, nir_intrinsic_instr *intr,
                        UNUSED void *cb_data)
{
   if (intr->intrinsic != nir_intrinsic_load_output)
      return false;

   b->cursor = nir_before_instr(&intr->instr);

   nir_def *fragcoord = nir_load_frag_coord(b);
   nir_def *sampid = nir_load_sample_id(b);
   nir_def *layer = nir_load_layer_id(b);
   fragcoord = nir_f2i32(b, fragcoord);

   nir_tex_instr *tex = nir_tex_instr_create(b->shader, 3);
   tex->op = nir_texop_txf_ms_fb;
   tex->sampler_dim = GLSL_SAMPLER_DIM_2D;
   tex->coord_components = 3;
   tex->dest_type = nir_type_float32;
   tex->is_array = true;
   tex->src[0] = nir_tex_src_for_ssa(nir_tex_src_coord,
                                     nir_vec3(b, nir_channel(b, fragcoord, 0), nir_channel(b, fragcoord, 1), layer));
   tex->src[1] = nir_tex_src_for_ssa(nir_tex_src_ms_index, sampid);
   struct nir_io_semantics io = nir_intrinsic_io_semantics(intr);
   tex->src[2] = nir_tex_src_for_ssa(nir_tex_src_texture_handle,
                                     nir_imm_intN_t(b, io.location - FRAG_RESULT_DATA0, 32));

   nir_def_init(&tex->instr, &tex->def, 4, 32);
   nir_builder_instr_insert(b, &tex->instr);

   nir_def_rewrite_uses(&intr->def, &tex->def);

   return true;
}

bool
nir_lower_fb_read(nir_shader *shader)
{
   assert(shader->info.stage == MESA_SHADER_FRAGMENT);

   return nir_shader_intrinsics_pass(shader, nir_lower_fb_read_instr,
                                       nir_metadata_block_index |
                                          nir_metadata_dominance,
                                       NULL);
}
