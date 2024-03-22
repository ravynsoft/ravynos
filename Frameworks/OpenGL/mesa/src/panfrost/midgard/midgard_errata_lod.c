/*
 * Copyright (C) 2019 Collabora, Ltd.
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

#include "compiler/nir/nir.h"
#include "compiler/nir/nir_builder.h"

bool midgard_nir_lod_errata(nir_shader *shader);

/* Workarounds errata pertaining to early Midgard chips where the settings for
 * min_lod/max_lod/lod_bias are ignored in the sampler descriptor when
 * texturing with a textureLod instruction. The workaround is to load these
 * constants in as system values and perform the bias/clamp in the shader.
 */

static bool
nir_lod_errata_instr(nir_builder *b, nir_instr *instr, void *data)
{
   if (instr->type != nir_instr_type_tex)
      return false;

   nir_tex_instr *tex = nir_instr_as_tex(instr);
   b->cursor = nir_before_instr(instr);

   /* The errata only applies to textureLod ("TEXGRD") */
   if (tex->op != nir_texop_txl)
      return false;

   /* TODO: Indirect samplers, separate sampler objects XXX */
   nir_def *sampler = nir_imm_int(b, tex->texture_index);

   /* Let's grab the sampler parameters */
   nir_def *params = nir_load_sampler_lod_parameters_pan(b, 3, 32, sampler);

   /* Extract the individual components */
   nir_def *min_lod = nir_channel(b, params, 0);
   nir_def *max_lod = nir_channel(b, params, 1);
   nir_def *lod_bias = nir_channel(b, params, 2);

   /* Rewrite the LOD with bias/clamps. Order sensitive. */
   for (unsigned i = 0; i < tex->num_srcs; i++) {
      if (tex->src[i].src_type != nir_tex_src_lod)
         continue;

      nir_def *lod = tex->src[i].src.ssa;

      nir_def *biased = nir_fadd(b, lod, lod_bias);
      nir_def *clamped = nir_fmin(b, nir_fmax(b, biased, min_lod), max_lod);

      nir_src_rewrite(&tex->src[i].src, clamped);
   }

   return true;
}

bool
midgard_nir_lod_errata(nir_shader *shader)
{
   return nir_shader_instructions_pass(
      shader, nir_lod_errata_instr,
      nir_metadata_block_index | nir_metadata_dominance, NULL);
}
