/*
 * Copyright © 2017 Intel Corporation
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

#include "compiler/nir/nir_builder.h"

static inline void
blorp_nir_init_shader(nir_builder *b,
                      void *mem_ctx,
                      gl_shader_stage stage,
                      const char *name)
{
   *b = nir_builder_init_simple_shader(stage, NULL, "%s", name ? name : "");
   ralloc_steal(mem_ctx, b->shader);
   if (stage == MESA_SHADER_FRAGMENT)
      b->shader->info.fs.origin_upper_left = true;
}

static inline nir_def *
blorp_nir_txf_ms_mcs(nir_builder *b, nir_def *xy_pos, nir_def *layer)
{
   nir_tex_instr *tex = nir_tex_instr_create(b->shader, 1);
   tex->op = nir_texop_txf_ms_mcs_intel;
   tex->sampler_dim = GLSL_SAMPLER_DIM_MS;
   tex->dest_type = nir_type_int32;

   nir_def *coord;
   if (layer) {
      tex->is_array = true;
      tex->coord_components = 3;
      coord = nir_vec3(b, nir_channel(b, xy_pos, 0),
                          nir_channel(b, xy_pos, 1),
                          layer);
   } else {
      tex->is_array = false;
      tex->coord_components = 2;
      coord = nir_trim_vector(b, xy_pos, 2);
   }
   tex->src[0] = nir_tex_src_for_ssa(nir_tex_src_coord, coord);

   /* Blorp only has one texture and it's bound at unit 0 */
   tex->texture_index = 0;
   tex->sampler_index = 0;

   nir_def_init(&tex->instr, &tex->def, 4, 32);
   nir_builder_instr_insert(b, &tex->instr);

   return &tex->def;
}

static inline nir_def *
blorp_nir_mcs_is_clear_color(nir_builder *b,
                             nir_def *mcs,
                             uint32_t samples)
{
   switch (samples) {
   case 2:
      /* Empirical evidence suggests that the value returned from the
       * sampler is not always 0x3 for clear color so we need to mask it.
       */
      return nir_ieq_imm(b, nir_iand(b, nir_channel(b, mcs, 0),
                                        nir_imm_int(b, 0x3)),
                            0x3);

   case 4:
      return nir_ieq_imm(b, nir_channel(b, mcs, 0), 0xff);

   case 8:
      return nir_ieq_imm(b, nir_channel(b, mcs, 0), ~0);

   case 16:
      /* For 16x MSAA, the MCS is actually an ivec2 */
      return nir_iand(b, nir_ieq_imm(b, nir_channel(b, mcs, 0), ~0),
                         nir_ieq_imm(b, nir_channel(b, mcs, 1), ~0));

   default:
      unreachable("Invalid sample count");
   }
}

static inline nir_def *
blorp_check_in_bounds(nir_builder *b,
                      nir_def *bounds_rect,
                      nir_def *pos)
{
   nir_def *x0 = nir_channel(b, bounds_rect, 0);
   nir_def *x1 = nir_channel(b, bounds_rect, 1);
   nir_def *y0 = nir_channel(b, bounds_rect, 2);
   nir_def *y1 = nir_channel(b, bounds_rect, 3);

   nir_def *c0 = nir_uge(b, nir_channel(b, pos, 0), x0);
   nir_def *c1 = nir_ult(b, nir_channel(b, pos, 0), x1);
   nir_def *c2 = nir_uge(b, nir_channel(b, pos, 1), y0);
   nir_def *c3 = nir_ult(b, nir_channel(b, pos, 1), y1);

   nir_def *in_bounds =
      nir_iand(b, nir_iand(b, c0, c1), nir_iand(b, c2, c3));

   return in_bounds;
}
