/*
 * Copyright © 2023 Valve Corporation
 * Copyright © 2015 Broadcom
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

/*
 * This lowering pass supports (as configured via nir_lower_tex_options)
 * various texture related conversions:
 *   + texture projector lowering: converts the coordinate division for
 *     texture projection to be done in ALU instructions instead of
 *     asking the texture operation to do so.
 *   + lowering RECT: converts the un-normalized RECT texture coordinates
 *     to normalized coordinates with txs plus ALU instructions
 *   + saturate s/t/r coords: to emulate certain texture clamp/wrap modes,
 *     inserts instructions to clamp specified coordinates to [0.0, 1.0].
 *     Note that this automatically triggers texture projector lowering if
 *     needed, since clamping must happen after projector lowering.
 *   + YUV-to-RGB conversion: to allow sampling YUV values as RGB values
 *     according to a specific YUV color space and range.
 */

#include "nir.h"
#include "nir_builder.h"
#include "nir_builtin_builder.h"
#include "nir_format_convert.h"

typedef struct nir_const_value_3_4 {
   nir_const_value v[3][4];
} nir_const_value_3_4;

static const nir_const_value_3_4 bt601_limited_range_csc_coeffs = { {
   { { .f32 = 1.16438356f }, { .f32 = 1.16438356f }, { .f32 = 1.16438356f } },
   { { .f32 = 0.0f }, { .f32 = -0.39176229f }, { .f32 = 2.01723214f } },
   { { .f32 = 1.59602678f }, { .f32 = -0.81296764f }, { .f32 = 0.0f } },
} };
static const nir_const_value_3_4 bt601_full_range_csc_coeffs = { {
   { { .f32 = 1.0f }, { .f32 = 1.0f }, { .f32 = 1.0f } },
   { { .f32 = 0.0f }, { .f32 = -0.34413629f }, { .f32 = 1.772f } },
   { { .f32 = 1.402f }, { .f32 = -0.71413629f }, { .f32 = 0.0f } },
} };
static const nir_const_value_3_4 bt709_limited_range_csc_coeffs = { {
   { { .f32 = 1.16438356f }, { .f32 = 1.16438356f }, { .f32 = 1.16438356f } },
   { { .f32 = 0.0f }, { .f32 = -0.21324861f }, { .f32 = 2.11240179f } },
   { { .f32 = 1.79274107f }, { .f32 = -0.53290933f }, { .f32 = 0.0f } },
} };
static const nir_const_value_3_4 bt709_full_range_csc_coeffs = { {
   { { .f32 = 1.0f }, { .f32 = 1.0f }, { .f32 = 1.0f } },
   { { .f32 = 0.0f }, { .f32 = -0.18732427f }, { .f32 = 1.8556f } },
   { { .f32 = 1.5748f }, { .f32 = -0.46812427f }, { .f32 = 0.0f } },
} };
static const nir_const_value_3_4 bt2020_limited_range_csc_coeffs = { {
   { { .f32 = 1.16438356f }, { .f32 = 1.16438356f }, { .f32 = 1.16438356f } },
   { { .f32 = 0.0f }, { .f32 = -0.18732610f }, { .f32 = 2.14177232f } },
   { { .f32 = 1.67878795f }, { .f32 = -0.65046843f }, { .f32 = 0.0f } },
} };
static const nir_const_value_3_4 bt2020_full_range_csc_coeffs = { {
   { { .f32 = 1.0f }, { .f32 = 1.0f }, { .f32 = 1.0f } },
   { { .f32 = 0.0f }, { .f32 = -0.16455313f }, { .f32 = 1.88140000f } },
   { { .f32 = 1.4747f }, { .f32 = -0.57139187f }, { .f32 = 0.0f } },
} };

static const float bt601_limited_range_csc_offsets[3] = {
   -0.874202218f, 0.531667823f, -1.085630789f
};
static const float bt601_full_range_csc_offsets[3] = {
   -0.701000000f, 0.529136286f, -0.886000000f
};
static const float bt709_limited_range_csc_offsets[3] = {
   -0.972945075f, 0.301482665f, -1.133402218f
};
static const float bt709_full_range_csc_offsets[3] = {
   -0.787400000f, 0.327724273f, -0.927800000f
};
static const float bt2020_limited_range_csc_offsets[3] = {
   -0.915745075f, 0.347480639f, -1.148145075f
};
static const float bt2020_full_range_csc_offsets[3] = {
   -0.737350000f, 0.367972500f, -0.940700000f
};

static bool
project_src(nir_builder *b, nir_tex_instr *tex)
{
   nir_def *proj = nir_steal_tex_src(tex, nir_tex_src_projector);
   if (!proj)
      return false;

   b->cursor = nir_before_instr(&tex->instr);
   nir_def *inv_proj = nir_frcp(b, proj);

   /* Walk through the sources projecting the arguments. */
   for (unsigned i = 0; i < tex->num_srcs; i++) {
      switch (tex->src[i].src_type) {
      case nir_tex_src_coord:
      case nir_tex_src_comparator:
         break;
      default:
         continue;
      }
      nir_def *unprojected =
         tex->src[i].src.ssa;
      nir_def *projected = nir_fmul(b, unprojected, inv_proj);

      /* Array indices don't get projected, so make an new vector with the
       * coordinate's array index untouched.
       */
      if (tex->is_array && tex->src[i].src_type == nir_tex_src_coord) {
         switch (tex->coord_components) {
         case 4:
            projected = nir_vec4(b,
                                 nir_channel(b, projected, 0),
                                 nir_channel(b, projected, 1),
                                 nir_channel(b, projected, 2),
                                 nir_channel(b, unprojected, 3));
            break;
         case 3:
            projected = nir_vec3(b,
                                 nir_channel(b, projected, 0),
                                 nir_channel(b, projected, 1),
                                 nir_channel(b, unprojected, 2));
            break;
         case 2:
            projected = nir_vec2(b,
                                 nir_channel(b, projected, 0),
                                 nir_channel(b, unprojected, 1));
            break;
         default:
            unreachable("bad texture coord count for array");
            break;
         }
      }

      nir_src_rewrite(&tex->src[i].src, projected);
   }

   return true;
}

static bool
lower_offset(nir_builder *b, nir_tex_instr *tex)
{
   nir_def *offset = nir_steal_tex_src(tex, nir_tex_src_offset);
   if (!offset)
      return false;

   int coord_index = nir_tex_instr_src_index(tex, nir_tex_src_coord);
   assert(coord_index >= 0);

   nir_def *coord = tex->src[coord_index].src.ssa;

   b->cursor = nir_before_instr(&tex->instr);

   nir_def *offset_coord;
   if (nir_tex_instr_src_type(tex, coord_index) == nir_type_float) {
      if (tex->sampler_dim == GLSL_SAMPLER_DIM_RECT) {
         offset_coord = nir_fadd(b, coord, nir_i2f32(b, offset));
      } else {
         nir_def *scale = NULL;

         if (b->shader->options->has_texture_scaling) {
            nir_def *idx = nir_imm_int(b, tex->texture_index);
            scale = nir_load_texture_scale(b, 32, idx);
         } else {
            nir_def *txs = nir_i2f32(b, nir_get_texture_size(b, tex));
            scale = nir_frcp(b, txs);
         }

         offset_coord = nir_fadd(b, coord,
                                 nir_fmul(b,
                                          nir_i2f32(b, offset),
                                          scale));
      }
   } else {
      offset_coord = nir_iadd(b, coord, offset);
   }

   if (tex->is_array) {
      /* The offset is not applied to the array index */
      if (tex->coord_components == 2) {
         offset_coord = nir_vec2(b, nir_channel(b, offset_coord, 0),
                                 nir_channel(b, coord, 1));
      } else if (tex->coord_components == 3) {
         offset_coord = nir_vec3(b, nir_channel(b, offset_coord, 0),
                                 nir_channel(b, offset_coord, 1),
                                 nir_channel(b, coord, 2));
      } else {
         unreachable("Invalid number of components");
      }
   }

   nir_src_rewrite(&tex->src[coord_index].src, offset_coord);

   return true;
}

static void
lower_rect(nir_builder *b, nir_tex_instr *tex)
{
   /* Set the sampler_dim to 2D here so that get_texture_size picks up the
    * right dimensionality.
    */
   tex->sampler_dim = GLSL_SAMPLER_DIM_2D;

   nir_def *txs = nir_i2f32(b, nir_get_texture_size(b, tex));
   nir_def *scale = nir_frcp(b, txs);
   int coord_index = nir_tex_instr_src_index(tex, nir_tex_src_coord);

   if (coord_index != -1) {
      nir_def *coords =
         tex->src[coord_index].src.ssa;
      nir_src_rewrite(&tex->src[coord_index].src, nir_fmul(b, coords, scale));
   }
}

static void
lower_rect_tex_scale(nir_builder *b, nir_tex_instr *tex)
{
   b->cursor = nir_before_instr(&tex->instr);

   nir_def *idx = nir_imm_int(b, tex->texture_index);
   nir_def *scale = nir_load_texture_scale(b, 32, idx);
   int coord_index = nir_tex_instr_src_index(tex, nir_tex_src_coord);

   if (coord_index != -1) {
      nir_def *coords =
         tex->src[coord_index].src.ssa;
      nir_src_rewrite(&tex->src[coord_index].src, nir_fmul(b, coords, scale));
   }
}

static void
lower_1d(nir_builder *b, nir_tex_instr *tex)
{
   b->cursor = nir_before_instr(&tex->instr);

   nir_def *coords = nir_steal_tex_src(tex, nir_tex_src_coord);
   nir_def *offset = nir_steal_tex_src(tex, nir_tex_src_offset);
   nir_def *ddx = nir_steal_tex_src(tex, nir_tex_src_ddx);
   nir_def *ddy = nir_steal_tex_src(tex, nir_tex_src_ddy);

   /* Add in 2D sources to become a 2D operation */
   tex->sampler_dim = GLSL_SAMPLER_DIM_2D;

   if (coords) {
      /* We want to fetch texel 0 along the Y-axis. To do so, we sample at 0.5
       * to get texel 0 with correct handling of wrap modes.
       */
      nir_def *y = nir_imm_floatN_t(b, tex->op == nir_texop_txf ? 0.0 : 0.5,
                                    coords->bit_size);

      tex->coord_components++;

      if (tex->is_array && tex->op != nir_texop_lod) {
         assert(tex->coord_components == 3);

         nir_def *x = nir_channel(b, coords, 0);
         nir_def *idx = nir_channel(b, coords, 1);
         coords = nir_vec3(b, x, y, idx);
      } else {
         assert(tex->coord_components == 2);
         coords = nir_vec2(b, coords, y);
      }

      nir_tex_instr_add_src(tex, nir_tex_src_coord, coords);
   }

   if (offset) {
      nir_tex_instr_add_src(tex, nir_tex_src_offset,
                            nir_pad_vector_imm_int(b, offset, 0, 2));
   }

   if (ddx || ddy) {
      nir_tex_instr_add_src(tex, nir_tex_src_ddx,
                            nir_pad_vector_imm_int(b, ddx, 0, 2));

      nir_tex_instr_add_src(tex, nir_tex_src_ddy,
                            nir_pad_vector_imm_int(b, ddy, 0, 2));
   }

   /* Handle destination component mismatch for txs. */
   if (tex->op == nir_texop_txs) {
      b->cursor = nir_after_instr(&tex->instr);

      nir_def *dst;
      if (tex->is_array) {
         assert(tex->def.num_components == 2);
         tex->def.num_components = 3;

         /* For array, we take .xz to skip the newly added height */
         dst = nir_channels(b, &tex->def, (1 << 0) | (1 << 2));
      } else {
         assert(tex->def.num_components == 1);
         tex->def.num_components = 2;

         dst = nir_channel(b, &tex->def, 0);
      }

      nir_def_rewrite_uses_after(&tex->def, dst, dst->parent_instr);
   }
}

static void
lower_lod(nir_builder *b, nir_tex_instr *tex, nir_def *lod)
{
   assert(tex->op == nir_texop_tex || tex->op == nir_texop_txb);
   assert(nir_tex_instr_src_index(tex, nir_tex_src_lod) < 0);
   assert(nir_tex_instr_src_index(tex, nir_tex_src_ddx) < 0);
   assert(nir_tex_instr_src_index(tex, nir_tex_src_ddy) < 0);

   /* If we have a bias, add it in */
   nir_def *bias = nir_steal_tex_src(tex, nir_tex_src_bias);
   if (bias)
      lod = nir_fadd(b, lod, bias);

   /* If we have a minimum LOD, clamp LOD accordingly */
   nir_def *min_lod = nir_steal_tex_src(tex, nir_tex_src_min_lod);
   if (min_lod)
      lod = nir_fmax(b, lod, min_lod);

   nir_tex_instr_add_src(tex, nir_tex_src_lod, lod);
   tex->op = nir_texop_txl;
}

static void
lower_implicit_lod(nir_builder *b, nir_tex_instr *tex)
{
   b->cursor = nir_before_instr(&tex->instr);
   lower_lod(b, tex, nir_get_texture_lod(b, tex));
}

static void
lower_zero_lod(nir_builder *b, nir_tex_instr *tex)
{
   b->cursor = nir_before_instr(&tex->instr);

   if (tex->op == nir_texop_lod) {
      nir_def_rewrite_uses(&tex->def, nir_imm_int(b, 0));
      nir_instr_remove(&tex->instr);
      return;
   }

   lower_lod(b, tex, nir_imm_int(b, 0));
}

static nir_def *
sample_plane(nir_builder *b, nir_tex_instr *tex, int plane,
             const nir_lower_tex_options *options)
{
   assert(nir_tex_instr_dest_size(tex) == 4);
   assert(nir_alu_type_get_base_type(tex->dest_type) == nir_type_float);
   assert(tex->op == nir_texop_tex);
   assert(tex->coord_components == 2);

   nir_tex_instr *plane_tex =
      nir_tex_instr_create(b->shader, tex->num_srcs + 1);
   for (unsigned i = 0; i < tex->num_srcs; i++) {
      plane_tex->src[i].src = nir_src_for_ssa(tex->src[i].src.ssa);
      plane_tex->src[i].src_type = tex->src[i].src_type;
   }
   plane_tex->src[tex->num_srcs] = nir_tex_src_for_ssa(nir_tex_src_plane,
                                                       nir_imm_int(b, plane));
   plane_tex->op = nir_texop_tex;
   plane_tex->sampler_dim = GLSL_SAMPLER_DIM_2D;
   plane_tex->dest_type = nir_type_float | tex->def.bit_size;
   plane_tex->coord_components = 2;

   plane_tex->texture_index = tex->texture_index;
   plane_tex->sampler_index = tex->sampler_index;

   nir_def_init(&plane_tex->instr, &plane_tex->def, 4,
                tex->def.bit_size);

   nir_builder_instr_insert(b, &plane_tex->instr);

   /* If scaling_factor is set, return a scaled value. */
   if (options->scale_factors[tex->texture_index])
      return nir_fmul_imm(b, &plane_tex->def,
                          options->scale_factors[tex->texture_index]);

   return &plane_tex->def;
}

static void
convert_yuv_to_rgb(nir_builder *b, nir_tex_instr *tex,
                   nir_def *y, nir_def *u, nir_def *v,
                   nir_def *a,
                   const nir_lower_tex_options *options,
                   unsigned texture_index)
{

   const float *offset_vals;
   const nir_const_value_3_4 *m;
   assert((options->bt709_external & options->bt2020_external) == 0);
   if (options->yuv_full_range_external & (1u << texture_index)) {
      if (options->bt709_external & (1u << texture_index)) {
         m = &bt709_full_range_csc_coeffs;
         offset_vals = bt709_full_range_csc_offsets;
      } else if (options->bt2020_external & (1u << texture_index)) {
         m = &bt2020_full_range_csc_coeffs;
         offset_vals = bt2020_full_range_csc_offsets;
      } else {
         m = &bt601_full_range_csc_coeffs;
         offset_vals = bt601_full_range_csc_offsets;
      }
   } else {
      if (options->bt709_external & (1u << texture_index)) {
         m = &bt709_limited_range_csc_coeffs;
         offset_vals = bt709_limited_range_csc_offsets;
      } else if (options->bt2020_external & (1u << texture_index)) {
         m = &bt2020_limited_range_csc_coeffs;
         offset_vals = bt2020_limited_range_csc_offsets;
      } else {
         m = &bt601_limited_range_csc_coeffs;
         offset_vals = bt601_limited_range_csc_offsets;
      }
   }

   unsigned bit_size = tex->def.bit_size;

   nir_def *offset =
      nir_vec4(b,
               nir_imm_floatN_t(b, offset_vals[0], a->bit_size),
               nir_imm_floatN_t(b, offset_vals[1], a->bit_size),
               nir_imm_floatN_t(b, offset_vals[2], a->bit_size),
               a);

   offset = nir_f2fN(b, offset, bit_size);

   nir_def *m0 = nir_f2fN(b, nir_build_imm(b, 4, 32, m->v[0]), bit_size);
   nir_def *m1 = nir_f2fN(b, nir_build_imm(b, 4, 32, m->v[1]), bit_size);
   nir_def *m2 = nir_f2fN(b, nir_build_imm(b, 4, 32, m->v[2]), bit_size);

   nir_def *result =
      nir_ffma(b, y, m0, nir_ffma(b, u, m1, nir_ffma(b, v, m2, offset)));

   nir_def_rewrite_uses(&tex->def, result);
}

static void
lower_y_uv_external(nir_builder *b, nir_tex_instr *tex,
                    const nir_lower_tex_options *options,
                    unsigned texture_index)
{
   b->cursor = nir_after_instr(&tex->instr);

   nir_def *y = sample_plane(b, tex, 0, options);
   nir_def *uv = sample_plane(b, tex, 1, options);

   convert_yuv_to_rgb(b, tex,
                      nir_channel(b, y, 0),
                      nir_channel(b, uv, 0),
                      nir_channel(b, uv, 1),
                      nir_imm_float(b, 1.0f),
                      options,
                      texture_index);
}

static void
lower_y_vu_external(nir_builder *b, nir_tex_instr *tex,
                    const nir_lower_tex_options *options,
                    unsigned texture_index)
{
   b->cursor = nir_after_instr(&tex->instr);

   nir_def *y = sample_plane(b, tex, 0, options);
   nir_def *vu = sample_plane(b, tex, 1, options);

   convert_yuv_to_rgb(b, tex,
                      nir_channel(b, y, 0),
                      nir_channel(b, vu, 1),
                      nir_channel(b, vu, 0),
                      nir_imm_float(b, 1.0f),
                      options,
                      texture_index);
}

static void
lower_y_u_v_external(nir_builder *b, nir_tex_instr *tex,
                     const nir_lower_tex_options *options,
                     unsigned texture_index)
{
   b->cursor = nir_after_instr(&tex->instr);

   nir_def *y = sample_plane(b, tex, 0, options);
   nir_def *u = sample_plane(b, tex, 1, options);
   nir_def *v = sample_plane(b, tex, 2, options);

   convert_yuv_to_rgb(b, tex,
                      nir_channel(b, y, 0),
                      nir_channel(b, u, 0),
                      nir_channel(b, v, 0),
                      nir_imm_float(b, 1.0f),
                      options,
                      texture_index);
}

static void
lower_yx_xuxv_external(nir_builder *b, nir_tex_instr *tex,
                       const nir_lower_tex_options *options,
                       unsigned texture_index)
{
   b->cursor = nir_after_instr(&tex->instr);

   nir_def *y = sample_plane(b, tex, 0, options);
   nir_def *xuxv = sample_plane(b, tex, 1, options);

   convert_yuv_to_rgb(b, tex,
                      nir_channel(b, y, 0),
                      nir_channel(b, xuxv, 1),
                      nir_channel(b, xuxv, 3),
                      nir_imm_float(b, 1.0f),
                      options,
                      texture_index);
}

static void
lower_yx_xvxu_external(nir_builder *b, nir_tex_instr *tex,
                       const nir_lower_tex_options *options,
                       unsigned texture_index)
{
   b->cursor = nir_after_instr(&tex->instr);

   nir_def *y = sample_plane(b, tex, 0, options);
   nir_def *xvxu = sample_plane(b, tex, 1, options);

   convert_yuv_to_rgb(b, tex,
                      nir_channel(b, y, 0),
                      nir_channel(b, xvxu, 3),
                      nir_channel(b, xvxu, 1),
                      nir_imm_float(b, 1.0f),
                      options,
                      texture_index);
}

static void
lower_xy_uxvx_external(nir_builder *b, nir_tex_instr *tex,
                       const nir_lower_tex_options *options,
                       unsigned texture_index)
{
   b->cursor = nir_after_instr(&tex->instr);

   nir_def *y = sample_plane(b, tex, 0, options);
   nir_def *uxvx = sample_plane(b, tex, 1, options);

   convert_yuv_to_rgb(b, tex,
                      nir_channel(b, y, 1),
                      nir_channel(b, uxvx, 0),
                      nir_channel(b, uxvx, 2),
                      nir_imm_float(b, 1.0f),
                      options,
                      texture_index);
}

static void
lower_xy_vxux_external(nir_builder *b, nir_tex_instr *tex,
                       const nir_lower_tex_options *options,
                       unsigned texture_index)
{
   b->cursor = nir_after_instr(&tex->instr);

   nir_def *y = sample_plane(b, tex, 0, options);
   nir_def *vxux = sample_plane(b, tex, 1, options);

   convert_yuv_to_rgb(b, tex,
                      nir_channel(b, y, 1),
                      nir_channel(b, vxux, 2),
                      nir_channel(b, vxux, 0),
                      nir_imm_float(b, 1.0f),
                      options,
                      texture_index);
}

static void
lower_ayuv_external(nir_builder *b, nir_tex_instr *tex,
                    const nir_lower_tex_options *options,
                    unsigned texture_index)
{
   b->cursor = nir_after_instr(&tex->instr);

   nir_def *ayuv = sample_plane(b, tex, 0, options);

   convert_yuv_to_rgb(b, tex,
                      nir_channel(b, ayuv, 2),
                      nir_channel(b, ayuv, 1),
                      nir_channel(b, ayuv, 0),
                      nir_channel(b, ayuv, 3),
                      options,
                      texture_index);
}

static void
lower_y41x_external(nir_builder *b, nir_tex_instr *tex,
                    const nir_lower_tex_options *options,
                    unsigned texture_index)
{
   b->cursor = nir_after_instr(&tex->instr);

   nir_def *y41x = sample_plane(b, tex, 0, options);

   convert_yuv_to_rgb(b, tex,
                      nir_channel(b, y41x, 1),
                      nir_channel(b, y41x, 0),
                      nir_channel(b, y41x, 2),
                      nir_channel(b, y41x, 3),
                      options,
                      texture_index);
}

static void
lower_xyuv_external(nir_builder *b, nir_tex_instr *tex,
                    const nir_lower_tex_options *options,
                    unsigned texture_index)
{
   b->cursor = nir_after_instr(&tex->instr);

   nir_def *xyuv = sample_plane(b, tex, 0, options);

   convert_yuv_to_rgb(b, tex,
                      nir_channel(b, xyuv, 2),
                      nir_channel(b, xyuv, 1),
                      nir_channel(b, xyuv, 0),
                      nir_imm_float(b, 1.0f),
                      options,
                      texture_index);
}

static void
lower_yuv_external(nir_builder *b, nir_tex_instr *tex,
                   const nir_lower_tex_options *options,
                   unsigned texture_index)
{
   b->cursor = nir_after_instr(&tex->instr);

   nir_def *yuv = sample_plane(b, tex, 0, options);

   convert_yuv_to_rgb(b, tex,
                      nir_channel(b, yuv, 0),
                      nir_channel(b, yuv, 1),
                      nir_channel(b, yuv, 2),
                      nir_imm_float(b, 1.0f),
                      options,
                      texture_index);
}

static void
lower_yu_yv_external(nir_builder *b, nir_tex_instr *tex,
                     const nir_lower_tex_options *options,
                     unsigned texture_index)
{
   b->cursor = nir_after_instr(&tex->instr);

   nir_def *yuv = sample_plane(b, tex, 0, options);

   convert_yuv_to_rgb(b, tex,
                      nir_channel(b, yuv, 1),
                      nir_channel(b, yuv, 2),
                      nir_channel(b, yuv, 0),
                      nir_imm_float(b, 1.0f),
                      options,
                      texture_index);
}

static void
lower_yv_yu_external(nir_builder *b, nir_tex_instr *tex,
                     const nir_lower_tex_options *options,
                     unsigned texture_index)
{
   b->cursor = nir_after_instr(&tex->instr);

   nir_def *yuv = sample_plane(b, tex, 0, options);

   convert_yuv_to_rgb(b, tex,
                      nir_channel(b, yuv, 2),
                      nir_channel(b, yuv, 1),
                      nir_channel(b, yuv, 0),
                      nir_imm_float(b, 1.0f),
                      options,
                      texture_index);
}

/*
 * Converts a nir_texop_txd instruction to nir_texop_txl with the given lod
 * computed from the gradients.
 */
static void
replace_gradient_with_lod(nir_builder *b, nir_def *lod, nir_tex_instr *tex)
{
   assert(tex->op == nir_texop_txd);

   nir_tex_instr_remove_src(tex, nir_tex_instr_src_index(tex, nir_tex_src_ddx));
   nir_tex_instr_remove_src(tex, nir_tex_instr_src_index(tex, nir_tex_src_ddy));

   /* If we have a minimum LOD, clamp LOD accordingly */
   nir_def *min_lod = nir_steal_tex_src(tex, nir_tex_src_min_lod);
   if (min_lod)
      lod = nir_fmax(b, lod, min_lod);

   nir_tex_instr_add_src(tex, nir_tex_src_lod, lod);
   tex->op = nir_texop_txl;
}

static void
lower_gradient_cube_map(nir_builder *b, nir_tex_instr *tex)
{
   assert(tex->sampler_dim == GLSL_SAMPLER_DIM_CUBE);
   assert(tex->op == nir_texop_txd);

   /* Use textureSize() to get the width and height of LOD 0 */
   nir_def *size = nir_i2f32(b, nir_get_texture_size(b, tex));

   /* Cubemap texture lookups first generate a texture coordinate normalized
    * to [-1, 1] on the appropiate face. The appropiate face is determined
    * by which component has largest magnitude and its sign. The texture
    * coordinate is the quotient of the remaining texture coordinates against
    * that absolute value of the component of largest magnitude. This
    * division requires that the computing of the derivative of the texel
    * coordinate must use the quotient rule. The high level GLSL code is as
    * follows:
    *
    * Step 1: selection
    *
    * vec3 abs_p, Q, dQdx, dQdy;
    * abs_p = abs(ir->coordinate);
    * if (abs_p.x >= max(abs_p.y, abs_p.z)) {
    *    Q = ir->coordinate.yzx;
    *    dQdx = ir->lod_info.grad.dPdx.yzx;
    *    dQdy = ir->lod_info.grad.dPdy.yzx;
    * }
    * if (abs_p.y >= max(abs_p.x, abs_p.z)) {
    *    Q = ir->coordinate.xzy;
    *    dQdx = ir->lod_info.grad.dPdx.xzy;
    *    dQdy = ir->lod_info.grad.dPdy.xzy;
    * }
    * if (abs_p.z >= max(abs_p.x, abs_p.y)) {
    *    Q = ir->coordinate;
    *    dQdx = ir->lod_info.grad.dPdx;
    *    dQdy = ir->lod_info.grad.dPdy;
    * }
    *
    * Step 2: use quotient rule to compute derivative. The normalized to
    * [-1, 1] texel coordinate is given by Q.xy / (sign(Q.z) * Q.z). We are
    * only concerned with the magnitudes of the derivatives whose values are
    * not affected by the sign. We drop the sign from the computation.
    *
    * vec2 dx, dy;
    * float recip;
    *
    * recip = 1.0 / Q.z;
    * dx = recip * ( dQdx.xy - Q.xy * (dQdx.z * recip) );
    * dy = recip * ( dQdy.xy - Q.xy * (dQdy.z * recip) );
    *
    * Step 3: compute LOD. At this point we have the derivatives of the
    * texture coordinates normalized to [-1,1]. We take the LOD to be
    *  result = log2(max(sqrt(dot(dx, dx)), sqrt(dy, dy)) * 0.5 * L)
    *         = -1.0 + log2(max(sqrt(dot(dx, dx)), sqrt(dy, dy)) * L)
    *         = -1.0 + log2(sqrt(max(dot(dx, dx), dot(dy,dy))) * L)
    *         = -1.0 + log2(sqrt(L * L * max(dot(dx, dx), dot(dy,dy))))
    *         = -1.0 + 0.5 * log2(L * L * max(dot(dx, dx), dot(dy,dy)))
    * where L is the dimension of the cubemap. The code is:
    *
    * float M, result;
    * M = max(dot(dx, dx), dot(dy, dy));
    * L = textureSize(sampler, 0).x;
    * result = -1.0 + 0.5 * log2(L * L * M);
    */

   /* coordinate */
   nir_def *p =
      tex->src[nir_tex_instr_src_index(tex, nir_tex_src_coord)].src.ssa;

   /* unmodified dPdx, dPdy values */
   nir_def *dPdx =
      tex->src[nir_tex_instr_src_index(tex, nir_tex_src_ddx)].src.ssa;
   nir_def *dPdy =
      tex->src[nir_tex_instr_src_index(tex, nir_tex_src_ddy)].src.ssa;

   nir_def *abs_p = nir_fabs(b, p);
   nir_def *abs_p_x = nir_channel(b, abs_p, 0);
   nir_def *abs_p_y = nir_channel(b, abs_p, 1);
   nir_def *abs_p_z = nir_channel(b, abs_p, 2);

   /* 1. compute selector */
   nir_def *Q, *dQdx, *dQdy;

   nir_def *cond_z = nir_fge(b, abs_p_z, nir_fmax(b, abs_p_x, abs_p_y));
   nir_def *cond_y = nir_fge(b, abs_p_y, nir_fmax(b, abs_p_x, abs_p_z));

   unsigned yzx[3] = { 1, 2, 0 };
   unsigned xzy[3] = { 0, 2, 1 };

   Q = nir_bcsel(b, cond_z,
                 p,
                 nir_bcsel(b, cond_y,
                           nir_swizzle(b, p, xzy, 3),
                           nir_swizzle(b, p, yzx, 3)));

   dQdx = nir_bcsel(b, cond_z,
                    dPdx,
                    nir_bcsel(b, cond_y,
                              nir_swizzle(b, dPdx, xzy, 3),
                              nir_swizzle(b, dPdx, yzx, 3)));

   dQdy = nir_bcsel(b, cond_z,
                    dPdy,
                    nir_bcsel(b, cond_y,
                              nir_swizzle(b, dPdy, xzy, 3),
                              nir_swizzle(b, dPdy, yzx, 3)));

   /* 2. quotient rule */

   /* tmp = Q.xy * recip;
    * dx = recip * ( dQdx.xy - (tmp * dQdx.z) );
    * dy = recip * ( dQdy.xy - (tmp * dQdy.z) );
    */
   nir_def *rcp_Q_z = nir_frcp(b, nir_channel(b, Q, 2));

   nir_def *Q_xy = nir_trim_vector(b, Q, 2);
   nir_def *tmp = nir_fmul(b, Q_xy, rcp_Q_z);

   nir_def *dQdx_xy = nir_trim_vector(b, dQdx, 2);
   nir_def *dQdx_z = nir_channel(b, dQdx, 2);
   nir_def *dx =
      nir_fmul(b, rcp_Q_z, nir_fsub(b, dQdx_xy, nir_fmul(b, tmp, dQdx_z)));

   nir_def *dQdy_xy = nir_trim_vector(b, dQdy, 2);
   nir_def *dQdy_z = nir_channel(b, dQdy, 2);
   nir_def *dy =
      nir_fmul(b, rcp_Q_z, nir_fsub(b, dQdy_xy, nir_fmul(b, tmp, dQdy_z)));

   /* M = max(dot(dx, dx), dot(dy, dy)); */
   nir_def *M = nir_fmax(b, nir_fdot(b, dx, dx), nir_fdot(b, dy, dy));

   /* size has textureSize() of LOD 0 */
   nir_def *L = nir_channel(b, size, 0);

   /* lod = -1.0 + 0.5 * log2(L * L * M); */
   nir_def *lod =
      nir_fadd(b,
               nir_imm_float(b, -1.0f),
               nir_fmul(b,
                        nir_imm_float(b, 0.5f),
                        nir_flog2(b, nir_fmul(b, L, nir_fmul(b, L, M)))));

   /* 3. Replace the gradient instruction with an equivalent lod instruction */
   replace_gradient_with_lod(b, lod, tex);
}

static void
lower_gradient(nir_builder *b, nir_tex_instr *tex)
{
   /* Cubes are more complicated and have their own function */
   if (tex->sampler_dim == GLSL_SAMPLER_DIM_CUBE) {
      lower_gradient_cube_map(b, tex);
      return;
   }

   assert(tex->sampler_dim != GLSL_SAMPLER_DIM_CUBE);
   assert(tex->op == nir_texop_txd);

   /* Use textureSize() to get the width and height of LOD 0 */
   unsigned component_mask;
   switch (tex->sampler_dim) {
   case GLSL_SAMPLER_DIM_3D:
      component_mask = 7;
      break;
   case GLSL_SAMPLER_DIM_1D:
      component_mask = 1;
      break;
   default:
      component_mask = 3;
      break;
   }

   nir_def *size =
      nir_channels(b, nir_i2f32(b, nir_get_texture_size(b, tex)),
                   component_mask);

   /* Scale the gradients by width and height.  Effectively, the incoming
    * gradients are s'(x,y), t'(x,y), and r'(x,y) from equation 3.19 in the
    * GL 3.0 spec; we want u'(x,y), which is w_t * s'(x,y).
    */
   nir_def *ddx =
      tex->src[nir_tex_instr_src_index(tex, nir_tex_src_ddx)].src.ssa;
   nir_def *ddy =
      tex->src[nir_tex_instr_src_index(tex, nir_tex_src_ddy)].src.ssa;

   nir_def *dPdx = nir_fmul(b, ddx, size);
   nir_def *dPdy = nir_fmul(b, ddy, size);

   nir_def *rho;
   if (dPdx->num_components == 1) {
      rho = nir_fmax(b, nir_fabs(b, dPdx), nir_fabs(b, dPdy));
   } else {
      rho = nir_fmax(b,
                     nir_fsqrt(b, nir_fdot(b, dPdx, dPdx)),
                     nir_fsqrt(b, nir_fdot(b, dPdy, dPdy)));
   }

   /* lod = log2(rho).  We're ignoring GL state biases for now. */
   nir_def *lod = nir_flog2(b, rho);

   /* Replace the gradient instruction with an equivalent lod instruction */
   replace_gradient_with_lod(b, lod, tex);
}

/* tex(s, coord) = txd(s, coord, dfdx(coord), dfdy(coord)) */
static nir_tex_instr *
lower_tex_to_txd(nir_builder *b, nir_tex_instr *tex)
{
   b->cursor = nir_after_instr(&tex->instr);
   nir_tex_instr *txd = nir_tex_instr_create(b->shader, tex->num_srcs + 2);

   txd->op = nir_texop_txd;
   txd->sampler_dim = tex->sampler_dim;
   txd->dest_type = tex->dest_type;
   txd->coord_components = tex->coord_components;
   txd->texture_index = tex->texture_index;
   txd->sampler_index = tex->sampler_index;
   txd->is_array = tex->is_array;
   txd->is_shadow = tex->is_shadow;
   txd->is_new_style_shadow = tex->is_new_style_shadow;

   /* reuse existing srcs */
   for (unsigned i = 0; i < tex->num_srcs; i++) {
      txd->src[i].src = nir_src_for_ssa(tex->src[i].src.ssa);
      txd->src[i].src_type = tex->src[i].src_type;
   }
   int coord_idx = nir_tex_instr_src_index(tex, nir_tex_src_coord);
   assert(coord_idx >= 0);
   nir_def *coord = tex->src[coord_idx].src.ssa;
   /* don't take the derivative of the array index */
   if (tex->is_array)
      coord = nir_channels(b, coord, nir_component_mask(coord->num_components - 1));
   nir_def *dfdx = nir_fddx(b, coord);
   nir_def *dfdy = nir_fddy(b, coord);
   txd->src[tex->num_srcs] = nir_tex_src_for_ssa(nir_tex_src_ddx, dfdx);
   txd->src[tex->num_srcs + 1] = nir_tex_src_for_ssa(nir_tex_src_ddy, dfdy);

   nir_def_init(&txd->instr, &txd->def,
                tex->def.num_components,
                tex->def.bit_size);
   nir_builder_instr_insert(b, &txd->instr);
   nir_def_rewrite_uses(&tex->def, &txd->def);
   nir_instr_remove(&tex->instr);
   return txd;
}

/* txb(s, coord, bias) = txl(s, coord, lod(s, coord).y + bias) */
static nir_tex_instr *
lower_txb_to_txl(nir_builder *b, nir_tex_instr *tex)
{
   b->cursor = nir_after_instr(&tex->instr);
   nir_tex_instr *txl = nir_tex_instr_create(b->shader, tex->num_srcs);

   txl->op = nir_texop_txl;
   txl->sampler_dim = tex->sampler_dim;
   txl->dest_type = tex->dest_type;
   txl->coord_components = tex->coord_components;
   txl->texture_index = tex->texture_index;
   txl->sampler_index = tex->sampler_index;
   txl->is_array = tex->is_array;
   txl->is_shadow = tex->is_shadow;
   txl->is_new_style_shadow = tex->is_new_style_shadow;

   /* reuse all but bias src */
   for (int i = 0; i < tex->num_srcs; i++) {
      if (tex->src[i].src_type != nir_tex_src_bias) {
         txl->src[i].src = nir_src_for_ssa(tex->src[i].src.ssa);
         txl->src[i].src_type = tex->src[i].src_type;
      }
   }
   nir_def *lod = nir_get_texture_lod(b, tex);

   int bias_idx = nir_tex_instr_src_index(tex, nir_tex_src_bias);
   assert(bias_idx >= 0);
   lod = nir_fadd(b, lod, tex->src[bias_idx].src.ssa);
   txl->src[tex->num_srcs - 1] = nir_tex_src_for_ssa(nir_tex_src_lod, lod);

   nir_def_init(&txl->instr, &txl->def,
                tex->def.num_components,
                tex->def.bit_size);
   nir_builder_instr_insert(b, &txl->instr);
   nir_def_rewrite_uses(&tex->def, &txl->def);
   nir_instr_remove(&tex->instr);
   return txl;
}

static nir_tex_instr *
saturate_src(nir_builder *b, nir_tex_instr *tex, unsigned sat_mask)
{
   if (tex->op == nir_texop_tex)
      tex = lower_tex_to_txd(b, tex);
   else if (tex->op == nir_texop_txb)
      tex = lower_txb_to_txl(b, tex);

   b->cursor = nir_before_instr(&tex->instr);
   int coord_index = nir_tex_instr_src_index(tex, nir_tex_src_coord);

   if (coord_index != -1) {
      nir_def *src =
         tex->src[coord_index].src.ssa;

      /* split src into components: */
      nir_def *comp[4];

      assume(tex->coord_components >= 1);

      for (unsigned j = 0; j < tex->coord_components; j++)
         comp[j] = nir_channel(b, src, j);

      /* clamp requested components, array index does not get clamped: */
      unsigned ncomp = tex->coord_components;
      if (tex->is_array)
         ncomp--;

      for (unsigned j = 0; j < ncomp; j++) {
         if ((1 << j) & sat_mask) {
            if (tex->sampler_dim == GLSL_SAMPLER_DIM_RECT) {
               /* non-normalized texture coords, so clamp to texture
                * size rather than [0.0, 1.0]
                */
               nir_def *txs = nir_i2f32(b, nir_get_texture_size(b, tex));
               comp[j] = nir_fmax(b, comp[j], nir_imm_float(b, 0.0));
               comp[j] = nir_fmin(b, comp[j], nir_channel(b, txs, j));
            } else {
               comp[j] = nir_fsat(b, comp[j]);
            }
         }
      }

      /* and move the result back into a single vecN: */
      src = nir_vec(b, comp, tex->coord_components);

      nir_src_rewrite(&tex->src[coord_index].src, src);
   }
   return tex;
}

static nir_def *
get_zero_or_one(nir_builder *b, nir_alu_type type, uint8_t swizzle_val)
{
   nir_const_value v[4];

   memset(&v, 0, sizeof(v));

   if (swizzle_val == 4) {
      v[0].u32 = v[1].u32 = v[2].u32 = v[3].u32 = 0;
   } else {
      assert(swizzle_val == 5);
      if (type == nir_type_float32)
         v[0].f32 = v[1].f32 = v[2].f32 = v[3].f32 = 1.0;
      else
         v[0].u32 = v[1].u32 = v[2].u32 = v[3].u32 = 1;
   }

   return nir_build_imm(b, 4, 32, v);
}

static void
swizzle_tg4_broadcom(nir_builder *b, nir_tex_instr *tex)
{
   b->cursor = nir_after_instr(&tex->instr);

   assert(nir_tex_instr_dest_size(tex) == 4);
   unsigned swiz[4] = { 2, 3, 1, 0 };
   nir_def *swizzled = nir_swizzle(b, &tex->def, swiz, 4);

   nir_def_rewrite_uses_after(&tex->def, swizzled,
                              swizzled->parent_instr);
}

static void
swizzle_result(nir_builder *b, nir_tex_instr *tex, const uint8_t swizzle[4])
{
   b->cursor = nir_after_instr(&tex->instr);

   nir_def *swizzled;
   if (tex->op == nir_texop_tg4) {
      if (swizzle[tex->component] < 4) {
         /* This one's easy */
         tex->component = swizzle[tex->component];
         return;
      } else {
         swizzled = get_zero_or_one(b, tex->dest_type, swizzle[tex->component]);
      }
   } else {
      assert(nir_tex_instr_dest_size(tex) == 4);
      if (swizzle[0] < 4 && swizzle[1] < 4 &&
          swizzle[2] < 4 && swizzle[3] < 4) {
         unsigned swiz[4] = { swizzle[0], swizzle[1], swizzle[2], swizzle[3] };
         /* We have no 0s or 1s, just emit a swizzling MOV */
         swizzled = nir_swizzle(b, &tex->def, swiz, 4);
      } else {
         nir_scalar srcs[4];
         for (unsigned i = 0; i < 4; i++) {
            if (swizzle[i] < 4) {
               srcs[i] = nir_get_scalar(&tex->def, swizzle[i]);
            } else {
               srcs[i] = nir_get_scalar(get_zero_or_one(b, tex->dest_type, swizzle[i]), 0);
            }
         }
         swizzled = nir_vec_scalars(b, srcs, 4);
      }
   }

   nir_def_rewrite_uses_after(&tex->def, swizzled,
                              swizzled->parent_instr);
}

static void
linearize_srgb_result(nir_builder *b, nir_tex_instr *tex)
{
   assert(nir_tex_instr_dest_size(tex) == 4);
   assert(nir_alu_type_get_base_type(tex->dest_type) == nir_type_float);

   b->cursor = nir_after_instr(&tex->instr);

   nir_def *rgb =
      nir_format_srgb_to_linear(b, nir_trim_vector(b, &tex->def, 3));

   /* alpha is untouched: */
   nir_def *result = nir_vec4(b,
                              nir_channel(b, rgb, 0),
                              nir_channel(b, rgb, 1),
                              nir_channel(b, rgb, 2),
                              nir_channel(b, &tex->def, 3));

   nir_def_rewrite_uses_after(&tex->def, result,
                              result->parent_instr);
}

/**
 * Lowers texture instructions from giving a vec4 result to a vec2 of f16,
 * i16, or u16, or a single unorm4x8 value.
 *
 * Note that we don't change the destination num_components, because
 * nir_tex_instr_dest_size() will still return 4.  The driver is just expected
 * to not store the other channels, given that nothing at the NIR level will
 * read them.
 */
static bool
lower_tex_packing(nir_builder *b, nir_tex_instr *tex,
                  const nir_lower_tex_options *options)
{
   nir_def *color = &tex->def;

   b->cursor = nir_after_instr(&tex->instr);

   assert(options->lower_tex_packing_cb);
   enum nir_lower_tex_packing packing =
      options->lower_tex_packing_cb(tex, options->lower_tex_packing_data);

   switch (packing) {
   case nir_lower_tex_packing_none:
      return false;

   case nir_lower_tex_packing_16: {
      static const unsigned bits[4] = { 16, 16, 16, 16 };

      switch (nir_alu_type_get_base_type(tex->dest_type)) {
      case nir_type_float:
         switch (nir_tex_instr_dest_size(tex)) {
         case 1:
            assert(tex->is_shadow && tex->is_new_style_shadow);
            color = nir_unpack_half_2x16_split_x(b, nir_channel(b, color, 0));
            break;
         case 2: {
            nir_def *rg = nir_channel(b, color, 0);
            color = nir_vec2(b,
                             nir_unpack_half_2x16_split_x(b, rg),
                             nir_unpack_half_2x16_split_y(b, rg));
            break;
         }
         case 4: {
            nir_def *rg = nir_channel(b, color, 0);
            nir_def *ba = nir_channel(b, color, 1);
            color = nir_vec4(b,
                             nir_unpack_half_2x16_split_x(b, rg),
                             nir_unpack_half_2x16_split_y(b, rg),
                             nir_unpack_half_2x16_split_x(b, ba),
                             nir_unpack_half_2x16_split_y(b, ba));
            break;
         }
         default:
            unreachable("wrong dest_size");
         }
         break;

      case nir_type_int:
         color = nir_format_unpack_sint(b, color, bits, 4);
         break;

      case nir_type_uint:
         color = nir_format_unpack_uint(b, color, bits, 4);
         break;

      default:
         unreachable("unknown base type");
      }
      break;
   }

   case nir_lower_tex_packing_8:
      assert(nir_alu_type_get_base_type(tex->dest_type) == nir_type_float);
      color = nir_unpack_unorm_4x8(b, nir_channel(b, color, 0));
      break;
   }

   nir_def_rewrite_uses_after(&tex->def, color,
                              color->parent_instr);
   return true;
}

static bool
sampler_index_lt(nir_tex_instr *tex, unsigned max)
{
   assert(nir_tex_instr_src_index(tex, nir_tex_src_sampler_deref) == -1);

   unsigned sampler_index = tex->sampler_index;

   int sampler_offset_idx =
      nir_tex_instr_src_index(tex, nir_tex_src_sampler_offset);
   if (sampler_offset_idx >= 0) {
      if (!nir_src_is_const(tex->src[sampler_offset_idx].src))
         return false;

      sampler_index += nir_src_as_uint(tex->src[sampler_offset_idx].src);
   }

   return sampler_index < max;
}

static bool
lower_tg4_offsets(nir_builder *b, nir_tex_instr *tex)
{
   assert(tex->op == nir_texop_tg4);
   assert(nir_tex_instr_has_explicit_tg4_offsets(tex));
   assert(nir_tex_instr_src_index(tex, nir_tex_src_offset) == -1);

   b->cursor = nir_after_instr(&tex->instr);

   nir_scalar dest[5] = { 0 };
   nir_def *residency = NULL;
   for (unsigned i = 0; i < 4; ++i) {
      nir_tex_instr *tex_copy = nir_tex_instr_create(b->shader, tex->num_srcs + 1);
      tex_copy->op = tex->op;
      tex_copy->coord_components = tex->coord_components;
      tex_copy->sampler_dim = tex->sampler_dim;
      tex_copy->is_array = tex->is_array;
      tex_copy->is_shadow = tex->is_shadow;
      tex_copy->is_new_style_shadow = tex->is_new_style_shadow;
      tex_copy->is_sparse = tex->is_sparse;
      tex_copy->is_gather_implicit_lod = tex->is_gather_implicit_lod;
      tex_copy->component = tex->component;
      tex_copy->dest_type = tex->dest_type;
      tex_copy->texture_index = tex->texture_index;
      tex_copy->sampler_index = tex->sampler_index;
      tex_copy->backend_flags = tex->backend_flags;

      for (unsigned j = 0; j < tex->num_srcs; ++j) {
         tex_copy->src[j].src = nir_src_for_ssa(tex->src[j].src.ssa);
         tex_copy->src[j].src_type = tex->src[j].src_type;
      }

      nir_def *offset = nir_imm_ivec2(b, tex->tg4_offsets[i][0],
                                      tex->tg4_offsets[i][1]);
      nir_tex_src src = nir_tex_src_for_ssa(nir_tex_src_offset, offset);
      tex_copy->src[tex_copy->num_srcs - 1] = src;

      nir_def_init(&tex_copy->instr, &tex_copy->def,
                   nir_tex_instr_dest_size(tex), 32);

      nir_builder_instr_insert(b, &tex_copy->instr);

      dest[i] = nir_get_scalar(&tex_copy->def, 3);
      if (tex->is_sparse) {
         nir_def *code = nir_channel(b, &tex_copy->def, 4);
         if (residency)
            residency = nir_sparse_residency_code_and(b, residency, code);
         else
            residency = code;
      }
   }
   dest[4] = nir_get_scalar(residency, 0);

   nir_def *res = nir_vec_scalars(b, dest, tex->def.num_components);
   nir_def_rewrite_uses(&tex->def, res);
   nir_instr_remove(&tex->instr);

   return true;
}

static bool
nir_lower_txs_lod(nir_builder *b, nir_tex_instr *tex)
{
   int lod_idx = nir_tex_instr_src_index(tex, nir_tex_src_lod);
   if (lod_idx < 0 ||
       (nir_src_is_const(tex->src[lod_idx].src) &&
        nir_src_as_int(tex->src[lod_idx].src) == 0))
      return false;

   unsigned dest_size = nir_tex_instr_dest_size(tex);

   b->cursor = nir_before_instr(&tex->instr);
   nir_def *lod = tex->src[lod_idx].src.ssa;

   /* Replace the non-0-LOD in the initial TXS operation by a 0-LOD. */
   nir_src_rewrite(&tex->src[lod_idx].src, nir_imm_int(b, 0));

   /* TXS(LOD) = max(TXS(0) >> LOD, 1)
    * But we do min(TXS(0), TXS(LOD)) to catch the case of a null surface,
    * which should return 0, not 1.
    */
   b->cursor = nir_after_instr(&tex->instr);
   nir_def *minified = nir_imin(b, &tex->def,
                                nir_imax(b, nir_ushr(b, &tex->def, lod),
                                         nir_imm_int(b, 1)));

   /* Make sure the component encoding the array size (if any) is not
    * minified.
    */
   if (tex->is_array) {
      nir_def *comp[3];

      assert(dest_size <= ARRAY_SIZE(comp));
      for (unsigned i = 0; i < dest_size - 1; i++)
         comp[i] = nir_channel(b, minified, i);

      comp[dest_size - 1] = nir_channel(b, &tex->def, dest_size - 1);
      minified = nir_vec(b, comp, dest_size);
   }

   nir_def_rewrite_uses_after(&tex->def, minified,
                              minified->parent_instr);
   return true;
}

static void
nir_lower_txs_cube_array(nir_builder *b, nir_tex_instr *tex)
{
   assert(tex->sampler_dim == GLSL_SAMPLER_DIM_CUBE && tex->is_array);
   tex->sampler_dim = GLSL_SAMPLER_DIM_2D;

   b->cursor = nir_after_instr(&tex->instr);

   assert(tex->def.num_components == 3);
   nir_def *size = &tex->def;
   size = nir_vec3(b, nir_channel(b, size, 1),
                   nir_channel(b, size, 1),
                   nir_idiv(b, nir_channel(b, size, 2),
                            nir_imm_int(b, 6)));

   nir_def_rewrite_uses_after(&tex->def, size, size->parent_instr);
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
nir_lower_ms_txf_to_fragment_fetch(nir_builder *b, nir_tex_instr *tex)
{
   lower_offset(b, tex);

   b->cursor = nir_before_instr(&tex->instr);

   /* Create FMASK fetch. */
   assert(tex->texture_index == 0);
   nir_tex_instr *fmask_fetch = nir_tex_instr_create(b->shader, tex->num_srcs - 1);
   fmask_fetch->op = nir_texop_fragment_mask_fetch_amd;
   fmask_fetch->coord_components = tex->coord_components;
   fmask_fetch->sampler_dim = tex->sampler_dim;
   fmask_fetch->is_array = tex->is_array;
   fmask_fetch->texture_non_uniform = tex->texture_non_uniform;
   fmask_fetch->dest_type = nir_type_uint32;
   nir_def_init(&fmask_fetch->instr, &fmask_fetch->def, 1, 32);

   fmask_fetch->num_srcs = 0;
   for (unsigned i = 0; i < tex->num_srcs; i++) {
      if (tex->src[i].src_type == nir_tex_src_ms_index)
         continue;
      nir_tex_src *src = &fmask_fetch->src[fmask_fetch->num_srcs++];
      src->src = nir_src_for_ssa(tex->src[i].src.ssa);
      src->src_type = tex->src[i].src_type;
   }

   nir_builder_instr_insert(b, &fmask_fetch->instr);

   /* Obtain new sample index. */
   int ms_index = nir_tex_instr_src_index(tex, nir_tex_src_ms_index);
   assert(ms_index >= 0);
   nir_src sample = tex->src[ms_index].src;
   nir_def *new_sample = nir_ubfe(b, &fmask_fetch->def,
                                  nir_ishl_imm(b, sample.ssa, 2), nir_imm_int(b, 3));

   /* Update instruction. */
   tex->op = nir_texop_fragment_fetch_amd;
   nir_src_rewrite(&tex->src[ms_index].src, new_sample);
}

static void
nir_lower_samples_identical_to_fragment_fetch(nir_builder *b, nir_tex_instr *tex)
{
   b->cursor = nir_after_instr(&tex->instr);

   nir_tex_instr *fmask_fetch = nir_instr_as_tex(nir_instr_clone(b->shader, &tex->instr));
   fmask_fetch->op = nir_texop_fragment_mask_fetch_amd;
   fmask_fetch->dest_type = nir_type_uint32;
   nir_def_init(&fmask_fetch->instr, &fmask_fetch->def, 1, 32);
   nir_builder_instr_insert(b, &fmask_fetch->instr);

   nir_def_rewrite_uses(&tex->def, nir_ieq_imm(b, &fmask_fetch->def, 0));
   nir_instr_remove_v(&tex->instr);
}

static void
nir_lower_lod_zero_width(nir_builder *b, nir_tex_instr *tex)
{
   int coord_index = nir_tex_instr_src_index(tex, nir_tex_src_coord);
   assert(coord_index >= 0);

   b->cursor = nir_after_instr(&tex->instr);

   nir_def *is_zero = nir_imm_true(b);
   for (unsigned i = 0; i < tex->coord_components; i++) {
      nir_def *coord = nir_channel(b, tex->src[coord_index].src.ssa, i);

      /* Compute the sum of the absolute values of derivatives. */
      nir_def *dfdx = nir_fddx(b, coord);
      nir_def *dfdy = nir_fddy(b, coord);
      nir_def *fwidth = nir_fadd(b, nir_fabs(b, dfdx), nir_fabs(b, dfdy));

      /* Check if the sum is 0. */
      is_zero = nir_iand(b, is_zero, nir_feq_imm(b, fwidth, 0.0));
   }

   /* Replace the raw LOD by -FLT_MAX if the sum is 0 for all coordinates. */
   nir_def *adjusted_lod =
      nir_bcsel(b, is_zero, nir_imm_float(b, -FLT_MAX),
                nir_channel(b, &tex->def, 1));

   nir_def *def =
      nir_vec2(b, nir_channel(b, &tex->def, 0), adjusted_lod);

   nir_def_rewrite_uses_after(&tex->def, def, def->parent_instr);
}

static bool
lower_index_to_offset(nir_builder *b, nir_tex_instr *tex)
{
   bool progress = false;
   b->cursor = nir_before_instr(&tex->instr);

   for (unsigned i = 0; i < tex->num_srcs; i++) {
      unsigned *index;
      switch (tex->src[i].src_type) {
      case nir_tex_src_texture_offset:
         index = &tex->texture_index;
         break;
      case nir_tex_src_sampler_offset:
         index = &tex->sampler_index;
         break;
      default:
         continue;
      }

      /* If there's no base index, there's nothing to lower */
      if ((*index) == 0)
         continue;

      nir_def *sum = nir_iadd_imm(b, tex->src[i].src.ssa, *index);
      nir_src_rewrite(&tex->src[i].src, sum);
      *index = 0;
      progress = true;
   }

   return progress;
}

static bool
nir_lower_tex_block(nir_block *block, nir_builder *b,
                    const nir_lower_tex_options *options,
                    const struct nir_shader_compiler_options *compiler_options)
{
   bool progress = false;

   nir_foreach_instr_safe(instr, block) {
      if (instr->type != nir_instr_type_tex)
         continue;

      nir_tex_instr *tex = nir_instr_as_tex(instr);
      bool lower_txp = !!(options->lower_txp & (1 << tex->sampler_dim));

      /* mask of src coords to saturate (clamp): */
      unsigned sat_mask = 0;
      /* ignore saturate for txf ops: these don't use samplers and can't GL_CLAMP */
      if (nir_tex_instr_need_sampler(tex)) {
         if ((1 << tex->sampler_index) & options->saturate_r)
            sat_mask |= (1 << 2); /* .z */
         if ((1 << tex->sampler_index) & options->saturate_t)
            sat_mask |= (1 << 1); /* .y */
         if ((1 << tex->sampler_index) & options->saturate_s)
            sat_mask |= (1 << 0); /* .x */
      }

      if (options->lower_index_to_offset)
         progress |= lower_index_to_offset(b, tex);

      /* If we are clamping any coords, we must lower projector first
       * as clamping happens *after* projection:
       */
      if (lower_txp || sat_mask ||
          (options->lower_txp_array && tex->is_array)) {
         progress |= project_src(b, tex);
      }

      if ((tex->op == nir_texop_txf && options->lower_txf_offset) ||
          (sat_mask && nir_tex_instr_src_index(tex, nir_tex_src_coord) >= 0) ||
          (tex->sampler_dim == GLSL_SAMPLER_DIM_RECT &&
           options->lower_rect_offset) ||
          (options->lower_offset_filter &&
           options->lower_offset_filter(instr, options->callback_data))) {
         progress = lower_offset(b, tex) || progress;
      }

      if ((tex->sampler_dim == GLSL_SAMPLER_DIM_RECT) && options->lower_rect &&
          tex->op != nir_texop_txf) {
         if (nir_tex_instr_is_query(tex))
            tex->sampler_dim = GLSL_SAMPLER_DIM_2D;
         else if (compiler_options->has_texture_scaling)
            lower_rect_tex_scale(b, tex);
         else
            lower_rect(b, tex);

         progress = true;
      }

      if (tex->sampler_dim == GLSL_SAMPLER_DIM_1D &&
          (options->lower_1d || (tex->is_shadow && options->lower_1d_shadow))) {
         lower_1d(b, tex);
         progress = true;
      }

      unsigned texture_index = tex->texture_index;
      uint32_t texture_mask = 1u << texture_index;
      int tex_index = nir_tex_instr_src_index(tex, nir_tex_src_texture_deref);
      if (tex_index >= 0) {
         nir_deref_instr *deref = nir_src_as_deref(tex->src[tex_index].src);
         nir_variable *var = nir_deref_instr_get_variable(deref);
         texture_index = var ? var->data.binding : 0;
         texture_mask = var && texture_index < 32 ? (1u << texture_index) : 0u;
      }

      if (texture_mask & options->lower_y_uv_external) {
         lower_y_uv_external(b, tex, options, texture_index);
         progress = true;
      }

      if (texture_mask & options->lower_y_vu_external) {
         lower_y_vu_external(b, tex, options, texture_index);
         progress = true;
      }

      if (texture_mask & options->lower_y_u_v_external) {
         lower_y_u_v_external(b, tex, options, texture_index);
         progress = true;
      }

      if (texture_mask & options->lower_yx_xuxv_external) {
         lower_yx_xuxv_external(b, tex, options, texture_index);
         progress = true;
      }

      if (texture_mask & options->lower_yx_xvxu_external) {
         lower_yx_xvxu_external(b, tex, options, texture_index);
         progress = true;
      }

      if (texture_mask & options->lower_xy_uxvx_external) {
         lower_xy_uxvx_external(b, tex, options, texture_index);
         progress = true;
      }

      if (texture_mask & options->lower_xy_vxux_external) {
         lower_xy_vxux_external(b, tex, options, texture_index);
         progress = true;
      }

      if (texture_mask & options->lower_ayuv_external) {
         lower_ayuv_external(b, tex, options, texture_index);
         progress = true;
      }

      if (texture_mask & options->lower_xyuv_external) {
         lower_xyuv_external(b, tex, options, texture_index);
         progress = true;
      }

      if (texture_mask & options->lower_yuv_external) {
         lower_yuv_external(b, tex, options, texture_index);
         progress = true;
      }

      if ((1 << tex->texture_index) & options->lower_yu_yv_external) {
         lower_yu_yv_external(b, tex, options, texture_index);
         progress = true;
      }

      if ((1 << tex->texture_index) & options->lower_yv_yu_external) {
         lower_yv_yu_external(b, tex, options, texture_index);
         progress = true;
      }

      if ((1 << tex->texture_index) & options->lower_y41x_external) {
         lower_y41x_external(b, tex, options, texture_index);
         progress = true;
      }

      if (sat_mask) {
         tex = saturate_src(b, tex, sat_mask);
         progress = true;
      }

      if (tex->op == nir_texop_tg4 && options->lower_tg4_broadcom_swizzle) {
         swizzle_tg4_broadcom(b, tex);
         progress = true;
      }

      if ((texture_mask & options->swizzle_result) &&
          !nir_tex_instr_is_query(tex) &&
          !(tex->is_shadow && tex->is_new_style_shadow)) {
         swizzle_result(b, tex, options->swizzles[tex->texture_index]);
         progress = true;
      }

      /* should be after swizzle so we know which channels are rgb: */
      if ((texture_mask & options->lower_srgb) &&
          !nir_tex_instr_is_query(tex) && !tex->is_shadow) {
         linearize_srgb_result(b, tex);
         progress = true;
      }

      const bool has_min_lod =
         nir_tex_instr_src_index(tex, nir_tex_src_min_lod) >= 0;
      const bool has_offset =
         nir_tex_instr_src_index(tex, nir_tex_src_offset) >= 0;

      if (tex->op == nir_texop_txb && tex->is_shadow && has_min_lod &&
          options->lower_txb_shadow_clamp) {
         lower_implicit_lod(b, tex);
         progress = true;
      }

      if (options->lower_tex_packing_cb &&
          tex->op != nir_texop_txs &&
          tex->op != nir_texop_query_levels &&
          tex->op != nir_texop_texture_samples) {
         progress |= lower_tex_packing(b, tex, options);
      }

      if (tex->op == nir_texop_txd &&
          (options->lower_txd ||
           (options->lower_txd_clamp && has_min_lod) ||
           (options->lower_txd_shadow && tex->is_shadow) ||
           (options->lower_txd_shadow_clamp && tex->is_shadow && has_min_lod) ||
           (options->lower_txd_offset_clamp && has_offset && has_min_lod) ||
           (options->lower_txd_clamp_bindless_sampler && has_min_lod &&
            nir_tex_instr_src_index(tex, nir_tex_src_sampler_handle) != -1) ||
           (options->lower_txd_clamp_if_sampler_index_not_lt_16 &&
            has_min_lod && !sampler_index_lt(tex, 16)) ||
           (options->lower_txd_cube_map &&
            tex->sampler_dim == GLSL_SAMPLER_DIM_CUBE) ||
           (options->lower_txd_3d &&
            tex->sampler_dim == GLSL_SAMPLER_DIM_3D) ||
           (options->lower_txd_array && tex->is_array))) {
         lower_gradient(b, tex);
         progress = true;
         continue;
      }

      /* TXF, TXS and TXL require a LOD but not everything we implement using those
       * three opcodes provides one.  Provide a default LOD of 0.
       */
      if ((nir_tex_instr_src_index(tex, nir_tex_src_lod) == -1) &&
          (tex->op == nir_texop_txf || tex->op == nir_texop_txs ||
           tex->op == nir_texop_txl || tex->op == nir_texop_query_levels)) {
         b->cursor = nir_before_instr(&tex->instr);
         nir_tex_instr_add_src(tex, nir_tex_src_lod, nir_imm_int(b, 0));
         progress = true;
         continue;
      }

      /* Only fragment and compute (in some cases) support implicit
       * derivatives.  Lower those opcodes which use implicit derivatives to
       * use an explicit LOD of 0.
       * But don't touch RECT samplers because they don't have mips.
       */
      if (options->lower_invalid_implicit_lod &&
          nir_tex_instr_has_implicit_derivative(tex) &&
          tex->sampler_dim != GLSL_SAMPLER_DIM_RECT &&
          !nir_shader_supports_implicit_lod(b->shader)) {
         lower_zero_lod(b, tex);
         progress = true;
      }

      if (options->lower_txs_lod && tex->op == nir_texop_txs) {
         progress |= nir_lower_txs_lod(b, tex);
         continue;
      }

      if (options->lower_txs_cube_array && tex->op == nir_texop_txs &&
          tex->sampler_dim == GLSL_SAMPLER_DIM_CUBE && tex->is_array) {
         nir_lower_txs_cube_array(b, tex);
         progress = true;
         continue;
      }

      /* has to happen after all the other lowerings as the original tg4 gets
       * replaced by 4 tg4 instructions.
       */
      if (tex->op == nir_texop_tg4 &&
          nir_tex_instr_has_explicit_tg4_offsets(tex) &&
          options->lower_tg4_offsets) {
         progress |= lower_tg4_offsets(b, tex);
         continue;
      }

      if (options->lower_to_fragment_fetch_amd && tex->op == nir_texop_txf_ms) {
         nir_lower_ms_txf_to_fragment_fetch(b, tex);
         progress = true;
         continue;
      }

      if (options->lower_to_fragment_fetch_amd && tex->op == nir_texop_samples_identical) {
         nir_lower_samples_identical_to_fragment_fetch(b, tex);
         progress = true;
         continue;
      }

      if (options->lower_lod_zero_width && tex->op == nir_texop_lod) {
         nir_lower_lod_zero_width(b, tex);
         progress = true;
         continue;
      }
   }

   return progress;
}

static bool
nir_lower_tex_impl(nir_function_impl *impl,
                   const nir_lower_tex_options *options,
                   const struct nir_shader_compiler_options *compiler_options)
{
   bool progress = false;
   nir_builder builder = nir_builder_create(impl);

   nir_foreach_block(block, impl) {
      progress |= nir_lower_tex_block(block, &builder, options, compiler_options);
   }

   nir_metadata_preserve(impl, nir_metadata_block_index |
                                  nir_metadata_dominance);
   return progress;
}

bool
nir_lower_tex(nir_shader *shader, const nir_lower_tex_options *options)
{
   bool progress = false;

   /* lower_tg4_offsets injects new tg4 instructions that won't be lowered
    * if lower_tg4_broadcom_swizzle is also requested so when both are set
    * we want to run lower_tg4_offsets in a separate pass first.
    */
   if (options->lower_tg4_offsets && options->lower_tg4_broadcom_swizzle) {
      nir_lower_tex_options _options = {
         .lower_tg4_offsets = true,
      };
      progress = nir_lower_tex(shader, &_options);
   }

   nir_foreach_function_impl(impl, shader) {
      progress |= nir_lower_tex_impl(impl, options, shader->options);
   }

   return progress;
}
