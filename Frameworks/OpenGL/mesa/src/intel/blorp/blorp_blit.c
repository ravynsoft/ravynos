/*
 * Copyright © 2012 Intel Corporation
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

#include "blorp_nir_builder.h"
#include "compiler/nir/nir_format_convert.h"

#include "blorp_priv.h"
#include "dev/intel_debug.h"

#include "util/format_rgb9e5.h"
#include "util/u_math.h"

#define FILE_DEBUG_FLAG DEBUG_BLORP

static const bool split_blorp_blit_debug = false;

struct brw_blorp_blit_vars {
   /* Input values from brw_blorp_wm_inputs */
   nir_variable *v_bounds_rect;
   nir_variable *v_rect_grid;
   nir_variable *v_coord_transform;
   nir_variable *v_src_z;
   nir_variable *v_src_offset;
   nir_variable *v_dst_offset;
   nir_variable *v_src_inv_size;
};

static void
brw_blorp_blit_vars_init(nir_builder *b, struct brw_blorp_blit_vars *v,
                         const struct brw_blorp_blit_prog_key *key)
{
#define LOAD_INPUT(name, type)\
   v->v_##name = BLORP_CREATE_NIR_INPUT(b->shader, name, type);

   LOAD_INPUT(bounds_rect, glsl_vec4_type())
   LOAD_INPUT(rect_grid, glsl_vec4_type())
   LOAD_INPUT(coord_transform, glsl_vec4_type())
   LOAD_INPUT(src_z, glsl_float_type())
   LOAD_INPUT(src_offset, glsl_vector_type(GLSL_TYPE_UINT, 2))
   LOAD_INPUT(dst_offset, glsl_vector_type(GLSL_TYPE_UINT, 2))
   LOAD_INPUT(src_inv_size, glsl_vector_type(GLSL_TYPE_FLOAT, 2))

#undef LOAD_INPUT
}

static nir_def *
blorp_blit_get_frag_coords(nir_builder *b,
                           const struct brw_blorp_blit_prog_key *key,
                           struct brw_blorp_blit_vars *v)
{
   nir_def *coord = nir_f2i32(b, nir_load_frag_coord(b));

   /* Account for destination surface intratile offset
    *
    * Transformation parameters giving translation from destination to source
    * coordinates don't take into account possible intra-tile destination
    * offset.  Therefore it has to be first subtracted from the incoming
    * coordinates.  Vertices are set up based on coordinates containing the
    * intra-tile offset.
    */
   if (key->need_dst_offset)
      coord = nir_isub(b, coord, nir_load_var(b, v->v_dst_offset));

   if (key->persample_msaa_dispatch) {
      b->shader->info.fs.uses_sample_shading = true;
      return nir_vec3(b, nir_channel(b, coord, 0), nir_channel(b, coord, 1),
                      nir_load_sample_id(b));
   } else {
      return nir_trim_vector(b, coord, 2);
   }
}

static nir_def *
blorp_blit_get_cs_dst_coords(nir_builder *b,
                             const struct brw_blorp_blit_prog_key *key,
                             struct brw_blorp_blit_vars *v)
{
   nir_def *coord = nir_load_global_invocation_id(b, 32);

   /* Account for destination surface intratile offset
    *
    * Transformation parameters giving translation from destination to source
    * coordinates don't take into account possible intra-tile destination
    * offset.  Therefore it has to be first subtracted from the incoming
    * coordinates.  Vertices are set up based on coordinates containing the
    * intra-tile offset.
    */
   if (key->need_dst_offset)
      coord = nir_isub(b, coord, nir_load_var(b, v->v_dst_offset));

   assert(!key->persample_msaa_dispatch);
   return nir_trim_vector(b, coord, 2);
}

/**
 * Emit code to translate from destination (X, Y) coordinates to source (X, Y)
 * coordinates.
 */
static nir_def *
blorp_blit_apply_transform(nir_builder *b, nir_def *src_pos,
                           struct brw_blorp_blit_vars *v)
{
   nir_def *coord_transform = nir_load_var(b, v->v_coord_transform);

   nir_def *offset = nir_vec2(b, nir_channel(b, coord_transform, 1),
                                     nir_channel(b, coord_transform, 3));
   nir_def *mul = nir_vec2(b, nir_channel(b, coord_transform, 0),
                                  nir_channel(b, coord_transform, 2));

   return nir_fadd(b, nir_fmul(b, src_pos, mul), offset);
}

static nir_tex_instr *
blorp_create_nir_tex_instr(nir_builder *b, struct brw_blorp_blit_vars *v,
                           nir_texop op, nir_def *pos, unsigned num_srcs,
                           nir_alu_type dst_type)
{
   nir_tex_instr *tex = nir_tex_instr_create(b->shader, num_srcs);

   tex->op = op;

   tex->dest_type = dst_type | 32;
   tex->is_array = false;
   tex->is_shadow = false;

   tex->texture_index = BLORP_TEXTURE_BT_INDEX;
   tex->sampler_index = BLORP_SAMPLER_INDEX;

   /* To properly handle 3-D and 2-D array textures, we pull the Z component
    * from an input.  TODO: This is a bit magic; we should probably make this
    * more explicit in the future.
    */
   assert(pos->num_components >= 2);
   if (op == nir_texop_txf || op == nir_texop_txf_ms ||
       op == nir_texop_txf_ms_mcs_intel) {
      pos = nir_vec3(b, nir_channel(b, pos, 0), nir_channel(b, pos, 1),
                        nir_f2i32(b, nir_load_var(b, v->v_src_z)));
   } else {
      pos = nir_vec3(b, nir_channel(b, pos, 0), nir_channel(b, pos, 1),
                        nir_load_var(b, v->v_src_z));
   }

   tex->src[0] = nir_tex_src_for_ssa(nir_tex_src_coord, pos);
   tex->coord_components = 3;

   nir_def_init(&tex->instr, &tex->def, 4, 32);

   return tex;
}

static nir_def *
blorp_nir_tex(nir_builder *b, struct brw_blorp_blit_vars *v,
              const struct brw_blorp_blit_prog_key *key, nir_def *pos)
{
   if (key->need_src_offset)
      pos = nir_fadd(b, pos, nir_i2f32(b, nir_load_var(b, v->v_src_offset)));

   /* If the sampler requires normalized coordinates, we need to compensate. */
   if (key->src_coords_normalized)
      pos = nir_fmul(b, pos, nir_load_var(b, v->v_src_inv_size));

   nir_tex_instr *tex =
      blorp_create_nir_tex_instr(b, v, nir_texop_txl, pos, 2,
                                 key->texture_data_type);

   assert(pos->num_components == 2);
   tex->sampler_dim = GLSL_SAMPLER_DIM_2D;
   tex->src[1] = nir_tex_src_for_ssa(nir_tex_src_lod, nir_imm_int(b, 0));

   nir_builder_instr_insert(b, &tex->instr);

   return &tex->def;
}

static nir_def *
blorp_nir_txf(nir_builder *b, struct brw_blorp_blit_vars *v,
              nir_def *pos, nir_alu_type dst_type)
{
   nir_tex_instr *tex =
      blorp_create_nir_tex_instr(b, v, nir_texop_txf, pos, 2, dst_type);

   tex->sampler_dim = GLSL_SAMPLER_DIM_3D;
   tex->src[1] = nir_tex_src_for_ssa(nir_tex_src_lod, nir_imm_int(b, 0));

   nir_builder_instr_insert(b, &tex->instr);

   return &tex->def;
}

static nir_def *
blorp_nir_txf_ms(nir_builder *b, struct brw_blorp_blit_vars *v,
                 nir_def *pos, nir_def *mcs, nir_alu_type dst_type)
{
   nir_tex_instr *tex =
      blorp_create_nir_tex_instr(b, v, nir_texop_txf_ms, pos, 3, dst_type);

   tex->sampler_dim = GLSL_SAMPLER_DIM_MS;

   tex->src[1].src_type = nir_tex_src_ms_index;
   if (pos->num_components == 2) {
      tex->src[1].src = nir_src_for_ssa(nir_imm_int(b, 0));
   } else {
      assert(pos->num_components == 3);
      tex->src[1].src = nir_src_for_ssa(nir_channel(b, pos, 2));
   }

   if (!mcs)
      mcs = nir_imm_zero(b, 4, 32);

   tex->src[2] = nir_tex_src_for_ssa(nir_tex_src_ms_mcs_intel, mcs);

   nir_builder_instr_insert(b, &tex->instr);

   return &tex->def;
}

static nir_def *
blorp_blit_txf_ms_mcs(nir_builder *b, struct brw_blorp_blit_vars *v,
                      nir_def *pos)
{
   nir_tex_instr *tex =
      blorp_create_nir_tex_instr(b, v, nir_texop_txf_ms_mcs_intel,
                                 pos, 1, nir_type_int);

   tex->sampler_dim = GLSL_SAMPLER_DIM_MS;

   nir_builder_instr_insert(b, &tex->instr);

   return &tex->def;
}

/**
 * Emit code to compensate for the difference between Y and W tiling.
 *
 * This code modifies the X and Y coordinates according to the formula:
 *
 *   (X', Y', S') = detile(W-MAJOR, tile(Y-MAJOR, X, Y, S))
 *
 * (See brw_blorp_build_nir_shader).
 */
static inline nir_def *
blorp_nir_retile_y_to_w(nir_builder *b, nir_def *pos)
{
   assert(pos->num_components == 2);
   nir_def *x_Y = nir_channel(b, pos, 0);
   nir_def *y_Y = nir_channel(b, pos, 1);

   /* Given X and Y coordinates that describe an address using Y tiling,
    * translate to the X and Y coordinates that describe the same address
    * using W tiling.
    *
    * If we break down the low order bits of X and Y, using a
    * single letter to represent each low-order bit:
    *
    *   X = A << 7 | 0bBCDEFGH
    *   Y = J << 5 | 0bKLMNP                                       (1)
    *
    * Then we can apply the Y tiling formula to see the memory offset being
    * addressed:
    *
    *   offset = (J * tile_pitch + A) << 12 | 0bBCDKLMNPEFGH       (2)
    *
    * If we apply the W detiling formula to this memory location, that the
    * corresponding X' and Y' coordinates are:
    *
    *   X' = A << 6 | 0bBCDPFH                                     (3)
    *   Y' = J << 6 | 0bKLMNEG
    *
    * Combining (1) and (3), we see that to transform (X, Y) to (X', Y'),
    * we need to make the following computation:
    *
    *   X' = (X & ~0b1011) >> 1 | (Y & 0b1) << 2 | X & 0b1         (4)
    *   Y' = (Y & ~0b1) << 1 | (X & 0b1000) >> 2 | (X & 0b10) >> 1
    */
   nir_def *x_W = nir_imm_int(b, 0);
   x_W = nir_mask_shift_or(b, x_W, x_Y, 0xfffffff4, -1);
   x_W = nir_mask_shift_or(b, x_W, y_Y, 0x1, 2);
   x_W = nir_mask_shift_or(b, x_W, x_Y, 0x1, 0);

   nir_def *y_W = nir_imm_int(b, 0);
   y_W = nir_mask_shift_or(b, y_W, y_Y, 0xfffffffe, 1);
   y_W = nir_mask_shift_or(b, y_W, x_Y, 0x8, -2);
   y_W = nir_mask_shift_or(b, y_W, x_Y, 0x2, -1);

   return nir_vec2(b, x_W, y_W);
}

/**
 * Emit code to compensate for the difference between Y and W tiling.
 *
 * This code modifies the X and Y coordinates according to the formula:
 *
 *   (X', Y', S') = detile(Y-MAJOR, tile(W-MAJOR, X, Y, S))
 *
 * (See brw_blorp_build_nir_shader).
 */
static inline nir_def *
blorp_nir_retile_w_to_y(nir_builder *b, nir_def *pos)
{
   assert(pos->num_components == 2);
   nir_def *x_W = nir_channel(b, pos, 0);
   nir_def *y_W = nir_channel(b, pos, 1);

   /* Applying the same logic as above, but in reverse, we obtain the
    * formulas:
    *
    * X' = (X & ~0b101) << 1 | (Y & 0b10) << 2 | (Y & 0b1) << 1 | X & 0b1
    * Y' = (Y & ~0b11) >> 1 | (X & 0b100) >> 2
    */
   nir_def *x_Y = nir_imm_int(b, 0);
   x_Y = nir_mask_shift_or(b, x_Y, x_W, 0xfffffffa, 1);
   x_Y = nir_mask_shift_or(b, x_Y, y_W, 0x2, 2);
   x_Y = nir_mask_shift_or(b, x_Y, y_W, 0x1, 1);
   x_Y = nir_mask_shift_or(b, x_Y, x_W, 0x1, 0);

   nir_def *y_Y = nir_imm_int(b, 0);
   y_Y = nir_mask_shift_or(b, y_Y, y_W, 0xfffffffc, -1);
   y_Y = nir_mask_shift_or(b, y_Y, x_W, 0x4, -2);

   return nir_vec2(b, x_Y, y_Y);
}

/**
 * Emit code to compensate for the difference between MSAA and non-MSAA
 * surfaces.
 *
 * This code modifies the X and Y coordinates according to the formula:
 *
 *   (X', Y', S') = encode_msaa(num_samples, IMS, X, Y, S)
 *
 * (See brw_blorp_blit_program).
 */
static inline nir_def *
blorp_nir_encode_msaa(nir_builder *b, nir_def *pos,
                      unsigned num_samples, enum isl_msaa_layout layout)
{
   assert(pos->num_components == 2 || pos->num_components == 3);

   switch (layout) {
   case ISL_MSAA_LAYOUT_NONE:
      assert(pos->num_components == 2);
      return pos;
   case ISL_MSAA_LAYOUT_ARRAY:
      /* No translation needed */
      return pos;
   case ISL_MSAA_LAYOUT_INTERLEAVED: {
      nir_def *x_in = nir_channel(b, pos, 0);
      nir_def *y_in = nir_channel(b, pos, 1);
      nir_def *s_in = pos->num_components == 2 ? nir_imm_int(b, 0) :
                                                     nir_channel(b, pos, 2);

      nir_def *x_out = nir_imm_int(b, 0);
      nir_def *y_out = nir_imm_int(b, 0);
      switch (num_samples) {
      case 2:
      case 4:
         /* encode_msaa(2, IMS, X, Y, S) = (X', Y', 0)
          *   where X' = (X & ~0b1) << 1 | (S & 0b1) << 1 | (X & 0b1)
          *         Y' = Y
          *
          * encode_msaa(4, IMS, X, Y, S) = (X', Y', 0)
          *   where X' = (X & ~0b1) << 1 | (S & 0b1) << 1 | (X & 0b1)
          *         Y' = (Y & ~0b1) << 1 | (S & 0b10) | (Y & 0b1)
          */
         x_out = nir_mask_shift_or(b, x_out, x_in, 0xfffffffe, 1);
         x_out = nir_mask_shift_or(b, x_out, s_in, 0x1, 1);
         x_out = nir_mask_shift_or(b, x_out, x_in, 0x1, 0);
         if (num_samples == 2) {
            y_out = y_in;
         } else {
            y_out = nir_mask_shift_or(b, y_out, y_in, 0xfffffffe, 1);
            y_out = nir_mask_shift_or(b, y_out, s_in, 0x2, 0);
            y_out = nir_mask_shift_or(b, y_out, y_in, 0x1, 0);
         }
         break;

      case 8:
         /* encode_msaa(8, IMS, X, Y, S) = (X', Y', 0)
          *   where X' = (X & ~0b1) << 2 | (S & 0b100) | (S & 0b1) << 1
          *              | (X & 0b1)
          *         Y' = (Y & ~0b1) << 1 | (S & 0b10) | (Y & 0b1)
          */
         x_out = nir_mask_shift_or(b, x_out, x_in, 0xfffffffe, 2);
         x_out = nir_mask_shift_or(b, x_out, s_in, 0x4, 0);
         x_out = nir_mask_shift_or(b, x_out, s_in, 0x1, 1);
         x_out = nir_mask_shift_or(b, x_out, x_in, 0x1, 0);
         y_out = nir_mask_shift_or(b, y_out, y_in, 0xfffffffe, 1);
         y_out = nir_mask_shift_or(b, y_out, s_in, 0x2, 0);
         y_out = nir_mask_shift_or(b, y_out, y_in, 0x1, 0);
         break;

      case 16:
         /* encode_msaa(16, IMS, X, Y, S) = (X', Y', 0)
          *   where X' = (X & ~0b1) << 2 | (S & 0b100) | (S & 0b1) << 1
          *              | (X & 0b1)
          *         Y' = (Y & ~0b1) << 2 | (S & 0b1000) >> 1 (S & 0b10)
          *              | (Y & 0b1)
          */
         x_out = nir_mask_shift_or(b, x_out, x_in, 0xfffffffe, 2);
         x_out = nir_mask_shift_or(b, x_out, s_in, 0x4, 0);
         x_out = nir_mask_shift_or(b, x_out, s_in, 0x1, 1);
         x_out = nir_mask_shift_or(b, x_out, x_in, 0x1, 0);
         y_out = nir_mask_shift_or(b, y_out, y_in, 0xfffffffe, 2);
         y_out = nir_mask_shift_or(b, y_out, s_in, 0x8, -1);
         y_out = nir_mask_shift_or(b, y_out, s_in, 0x2, 0);
         y_out = nir_mask_shift_or(b, y_out, y_in, 0x1, 0);
         break;

      default:
         unreachable("Invalid number of samples for IMS layout");
      }

      return nir_vec2(b, x_out, y_out);
   }

   default:
      unreachable("Invalid MSAA layout");
   }
}

/**
 * Emit code to compensate for the difference between MSAA and non-MSAA
 * surfaces.
 *
 * This code modifies the X and Y coordinates according to the formula:
 *
 *   (X', Y', S) = decode_msaa(num_samples, IMS, X, Y, S)
 *
 * (See brw_blorp_blit_program).
 */
static inline nir_def *
blorp_nir_decode_msaa(nir_builder *b, nir_def *pos,
                      unsigned num_samples, enum isl_msaa_layout layout)
{
   assert(pos->num_components == 2 || pos->num_components == 3);

   switch (layout) {
   case ISL_MSAA_LAYOUT_NONE:
      /* No translation necessary, and S should already be zero. */
      assert(pos->num_components == 2);
      return pos;
   case ISL_MSAA_LAYOUT_ARRAY:
      /* No translation necessary. */
      return pos;
   case ISL_MSAA_LAYOUT_INTERLEAVED: {
      assert(pos->num_components == 2);

      nir_def *x_in = nir_channel(b, pos, 0);
      nir_def *y_in = nir_channel(b, pos, 1);

      nir_def *x_out = nir_imm_int(b, 0);
      nir_def *y_out = nir_imm_int(b, 0);
      nir_def *s_out = nir_imm_int(b, 0);
      switch (num_samples) {
      case 2:
      case 4:
         /* decode_msaa(2, IMS, X, Y, 0) = (X', Y', S)
          *   where X' = (X & ~0b11) >> 1 | (X & 0b1)
          *         S = (X & 0b10) >> 1
          *
          * decode_msaa(4, IMS, X, Y, 0) = (X', Y', S)
          *   where X' = (X & ~0b11) >> 1 | (X & 0b1)
          *         Y' = (Y & ~0b11) >> 1 | (Y & 0b1)
          *         S = (Y & 0b10) | (X & 0b10) >> 1
          */
         x_out = nir_mask_shift_or(b, x_out, x_in, 0xfffffffc, -1);
         x_out = nir_mask_shift_or(b, x_out, x_in, 0x1, 0);
         if (num_samples == 2) {
            y_out = y_in;
            s_out = nir_mask_shift_or(b, s_out, x_in, 0x2, -1);
         } else {
            y_out = nir_mask_shift_or(b, y_out, y_in, 0xfffffffc, -1);
            y_out = nir_mask_shift_or(b, y_out, y_in, 0x1, 0);
            s_out = nir_mask_shift_or(b, s_out, x_in, 0x2, -1);
            s_out = nir_mask_shift_or(b, s_out, y_in, 0x2, 0);
         }
         break;

      case 8:
         /* decode_msaa(8, IMS, X, Y, 0) = (X', Y', S)
          *   where X' = (X & ~0b111) >> 2 | (X & 0b1)
          *         Y' = (Y & ~0b11) >> 1 | (Y & 0b1)
          *         S = (X & 0b100) | (Y & 0b10) | (X & 0b10) >> 1
          */
         x_out = nir_mask_shift_or(b, x_out, x_in, 0xfffffff8, -2);
         x_out = nir_mask_shift_or(b, x_out, x_in, 0x1, 0);
         y_out = nir_mask_shift_or(b, y_out, y_in, 0xfffffffc, -1);
         y_out = nir_mask_shift_or(b, y_out, y_in, 0x1, 0);
         s_out = nir_mask_shift_or(b, s_out, x_in, 0x4, 0);
         s_out = nir_mask_shift_or(b, s_out, y_in, 0x2, 0);
         s_out = nir_mask_shift_or(b, s_out, x_in, 0x2, -1);
         break;

      case 16:
         /* decode_msaa(16, IMS, X, Y, 0) = (X', Y', S)
          *   where X' = (X & ~0b111) >> 2 | (X & 0b1)
          *         Y' = (Y & ~0b111) >> 2 | (Y & 0b1)
          *         S = (Y & 0b100) << 1 | (X & 0b100) |
          *             (Y & 0b10) | (X & 0b10) >> 1
          */
         x_out = nir_mask_shift_or(b, x_out, x_in, 0xfffffff8, -2);
         x_out = nir_mask_shift_or(b, x_out, x_in, 0x1, 0);
         y_out = nir_mask_shift_or(b, y_out, y_in, 0xfffffff8, -2);
         y_out = nir_mask_shift_or(b, y_out, y_in, 0x1, 0);
         s_out = nir_mask_shift_or(b, s_out, y_in, 0x4, 1);
         s_out = nir_mask_shift_or(b, s_out, x_in, 0x4, 0);
         s_out = nir_mask_shift_or(b, s_out, y_in, 0x2, 0);
         s_out = nir_mask_shift_or(b, s_out, x_in, 0x2, -1);
         break;

      default:
         unreachable("Invalid number of samples for IMS layout");
      }

      return nir_vec3(b, x_out, y_out, s_out);
   }

   default:
      unreachable("Invalid MSAA layout");
   }
}

/**
 * Count the number of trailing 1 bits in the given value.  For example:
 *
 * count_trailing_one_bits(0) == 0
 * count_trailing_one_bits(7) == 3
 * count_trailing_one_bits(11) == 2
 */
static inline int count_trailing_one_bits(unsigned value)
{
#ifdef HAVE___BUILTIN_CTZ
   return __builtin_ctz(~value);
#else
   return util_bitcount(value & ~(value + 1));
#endif
}

static nir_def *
blorp_nir_combine_samples(nir_builder *b, struct brw_blorp_blit_vars *v,
                          nir_def *pos, unsigned tex_samples,
                          enum isl_aux_usage tex_aux_usage,
                          nir_alu_type dst_type,
                          enum blorp_filter filter)
{
   nir_variable *color =
      nir_local_variable_create(b->impl, glsl_vec4_type(), "color");

   nir_def *mcs = NULL;
   if (isl_aux_usage_has_mcs(tex_aux_usage))
      mcs = blorp_blit_txf_ms_mcs(b, v, pos);

   nir_op combine_op;
   switch (filter) {
   case BLORP_FILTER_AVERAGE:
      assert(dst_type == nir_type_float);
      combine_op = nir_op_fadd;
      break;

   case BLORP_FILTER_MIN_SAMPLE:
      switch (dst_type) {
      case nir_type_int:   combine_op = nir_op_imin;  break;
      case nir_type_uint:  combine_op = nir_op_umin;  break;
      case nir_type_float: combine_op = nir_op_fmin;  break;
      default: unreachable("Invalid dst_type");
      }
      break;

   case BLORP_FILTER_MAX_SAMPLE:
      switch (dst_type) {
      case nir_type_int:   combine_op = nir_op_imax;  break;
      case nir_type_uint:  combine_op = nir_op_umax;  break;
      case nir_type_float: combine_op = nir_op_fmax;  break;
      default: unreachable("Invalid dst_type");
      }
      break;

   default:
      unreachable("Invalid filter");
   }

   /* If true, we inserted an if statement that we need to pop at at the end.
    */
   bool inserted_if = false;

   /* We add together samples using a binary tree structure, e.g. for 4x MSAA:
    *
    *   result = ((sample[0] + sample[1]) + (sample[2] + sample[3])) / 4
    *
    * This ensures that when all samples have the same value, no numerical
    * precision is lost, since each addition operation always adds two equal
    * values, and summing two equal floating point values does not lose
    * precision.
    *
    * We perform this computation by treating the texture_data array as a
    * stack and performing the following operations:
    *
    * - push sample 0 onto stack
    * - push sample 1 onto stack
    * - add top two stack entries
    * - push sample 2 onto stack
    * - push sample 3 onto stack
    * - add top two stack entries
    * - add top two stack entries
    * - divide top stack entry by 4
    *
    * Note that after pushing sample i onto the stack, the number of add
    * operations we do is equal to the number of trailing 1 bits in i.  This
    * works provided the total number of samples is a power of two, which it
    * always is for i965.
    *
    * For integer formats, we replace the add operations with average
    * operations and skip the final division.
    */
   nir_def *texture_data[5];
   texture_data[0] = NULL; /* Avoid maybe-uninitialized warning with GCC 10 */
   unsigned stack_depth = 0;
   for (unsigned i = 0; i < tex_samples; ++i) {
      assert(stack_depth == util_bitcount(i)); /* Loop invariant */

      /* Push sample i onto the stack */
      assert(stack_depth < ARRAY_SIZE(texture_data));

      nir_def *ms_pos = nir_vec3(b, nir_channel(b, pos, 0),
                                        nir_channel(b, pos, 1),
                                        nir_imm_int(b, i));
      texture_data[stack_depth++] = blorp_nir_txf_ms(b, v, ms_pos, mcs, dst_type);

      if (i == 0 && isl_aux_usage_has_mcs(tex_aux_usage)) {
         /* The Ivy Bridge PRM, Vol4 Part1 p27 (Multisample Control Surface)
          * suggests an optimization:
          *
          *     "A simple optimization with probable large return in
          *     performance is to compare the MCS value to zero (indicating
          *     all samples are on sample slice 0), and sample only from
          *     sample slice 0 using ld2dss if MCS is zero."
          *
          * Note that in the case where the MCS value is zero, sampling from
          * sample slice 0 using ld2dss and sampling from sample 0 using
          * ld2dms are equivalent (since all samples are on sample slice 0).
          * Since we have already sampled from sample 0, all we need to do is
          * skip the remaining fetches and averaging if MCS is zero.
          *
          * It's also trivial to detect when the MCS has the magic clear color
          * value.  In this case, the txf we did on sample 0 will return the
          * clear color and we can skip the remaining fetches just like we do
          * when MCS == 0.
          */
         nir_def *mcs_zero = nir_ieq_imm(b, nir_channel(b, mcs, 0), 0);
         if (tex_samples == 16) {
            mcs_zero = nir_iand(b, mcs_zero,
               nir_ieq_imm(b, nir_channel(b, mcs, 1), 0));
         }
         nir_def *mcs_clear =
            blorp_nir_mcs_is_clear_color(b, mcs, tex_samples);

         nir_push_if(b, nir_ior(b, mcs_zero, mcs_clear));
         nir_store_var(b, color, texture_data[0], 0xf);

         nir_push_else(b, NULL);
         inserted_if = true;
      }

      for (int j = 0; j < count_trailing_one_bits(i); j++) {
         assert(stack_depth >= 2);
         --stack_depth;

         texture_data[stack_depth - 1] =
            nir_build_alu(b, combine_op,
                             texture_data[stack_depth - 1],
                             texture_data[stack_depth],
                             NULL, NULL);
      }
   }

   /* We should have just 1 sample on the stack now. */
   assert(stack_depth == 1);

   if (filter == BLORP_FILTER_AVERAGE) {
      assert(dst_type == nir_type_float);
      texture_data[0] = nir_fmul_imm(b, texture_data[0],
                                     1.0 / tex_samples);
   }

   nir_store_var(b, color, texture_data[0], 0xf);

   if (inserted_if)
      nir_pop_if(b, NULL);

   return nir_load_var(b, color);
}

static nir_def *
blorp_nir_manual_blend_bilinear(nir_builder *b, nir_def *pos,
                                unsigned tex_samples,
                                const struct brw_blorp_blit_prog_key *key,
                                struct brw_blorp_blit_vars *v)
{
   nir_def *pos_xy = nir_trim_vector(b, pos, 2);
   nir_def *rect_grid = nir_load_var(b, v->v_rect_grid);
   nir_def *scale = nir_imm_vec2(b, key->x_scale, key->y_scale);

   /* Translate coordinates to lay out the samples in a rectangular  grid
    * roughly corresponding to sample locations.
    */
   pos_xy = nir_fmul(b, pos_xy, scale);
   /* Adjust coordinates so that integers represent pixel centers rather
    * than pixel edges.
    */
   pos_xy = nir_fadd_imm(b, pos_xy, -0.5);
   /* Clamp the X, Y texture coordinates to properly handle the sampling of
    * texels on texture edges.
    */
   pos_xy = nir_fmin(b, nir_fmax(b, pos_xy, nir_imm_float(b, 0.0)),
                        nir_trim_vector(b, rect_grid, 2));

   /* Store the fractional parts to be used as bilinear interpolation
    * coefficients.
    */
   nir_def *frac_xy = nir_ffract(b, pos_xy);
   /* Round the float coordinates down to nearest integer */
   pos_xy = nir_fdiv(b, nir_ftrunc(b, pos_xy), scale);

   nir_def *tex_data[4];
   for (unsigned i = 0; i < 4; ++i) {
      float sample_off_x = (float)(i & 0x1) / key->x_scale;
      float sample_off_y = (float)((i >> 1) & 0x1) / key->y_scale;
      nir_def *sample_off = nir_imm_vec2(b, sample_off_x, sample_off_y);

      nir_def *sample_coords = nir_fadd(b, pos_xy, sample_off);
      nir_def *sample_coords_int = nir_f2i32(b, sample_coords);

      /* The MCS value we fetch has to match up with the pixel that we're
       * sampling from. Since we sample from different pixels in each
       * iteration of this "for" loop, the call to mcs_fetch() should be
       * here inside the loop after computing the pixel coordinates.
       */
      nir_def *mcs = NULL;
      if (isl_aux_usage_has_mcs(key->tex_aux_usage))
         mcs = blorp_blit_txf_ms_mcs(b, v, sample_coords_int);

      /* Compute sample index and map the sample index to a sample number.
       * Sample index layout shows the numbering of slots in a rectangular
       * grid of samples with in a pixel. Sample number layout shows the
       * rectangular grid of samples roughly corresponding to the real sample
       * locations with in a pixel.
       *
       * In the case of 2x MSAA, the layout of sample indices is reversed from
       * the layout of sample numbers:
       *
       * sample index layout :  ---------    sample number layout :  ---------
       *                        | 0 | 1 |                            | 1 | 0 |
       *                        ---------                            ---------
       *
       * In case of 4x MSAA, layout of sample indices matches the layout of
       * sample numbers:
       *           ---------
       *           | 0 | 1 |
       *           ---------
       *           | 2 | 3 |
       *           ---------
       *
       * In case of 8x MSAA the two layouts don't match.
       * sample index layout :  ---------    sample number layout :  ---------
       *                        | 0 | 1 |                            | 3 | 7 |
       *                        ---------                            ---------
       *                        | 2 | 3 |                            | 5 | 0 |
       *                        ---------                            ---------
       *                        | 4 | 5 |                            | 1 | 2 |
       *                        ---------                            ---------
       *                        | 6 | 7 |                            | 4 | 6 |
       *                        ---------                            ---------
       *
       * Fortunately, this can be done fairly easily as:
       * S' = (0x17306425 >> (S * 4)) & 0xf
       *
       * In the case of 16x MSAA the two layouts don't match.
       * Sample index layout:                Sample number layout:
       * ---------------------               ---------------------
       * |  0 |  1 |  2 |  3 |               | 15 | 10 |  9 |  7 |
       * ---------------------               ---------------------
       * |  4 |  5 |  6 |  7 |               |  4 |  1 |  3 | 13 |
       * ---------------------               ---------------------
       * |  8 |  9 | 10 | 11 |               | 12 |  2 |  0 |  6 |
       * ---------------------               ---------------------
       * | 12 | 13 | 14 | 15 |               | 11 |  8 |  5 | 14 |
       * ---------------------               ---------------------
       *
       * This is equivalent to
       * S' = (0xe58b602cd31479af >> (S * 4)) & 0xf
       */
      nir_def *frac = nir_ffract(b, sample_coords);
      nir_def *sample =
         nir_fdot2(b, frac, nir_imm_vec2(b, key->x_scale,
                                            key->x_scale * key->y_scale));
      sample = nir_f2i32(b, sample);

      if (tex_samples == 2) {
         sample = nir_isub_imm(b, 1, sample);
      } else if (tex_samples == 8) {
         sample = nir_iand_imm(b, nir_ishr(b, nir_imm_int(b, 0x64210573),
                                           nir_ishl_imm(b, sample, 2)),
                               0xf);
      } else if (tex_samples == 16) {
         nir_def *sample_low =
            nir_iand_imm(b, nir_ishr(b, nir_imm_int(b, 0xd31479af),
                                     nir_ishl_imm(b, sample, 2)),
                         0xf);
         nir_def *sample_high =
            nir_iand_imm(b, nir_ishr(b, nir_imm_int(b, 0xe58b602c),
                                     nir_ishl_imm(b, nir_iadd_imm(b, sample, -8),
                                                  2)),
                         0xf);

         sample = nir_bcsel(b, nir_ilt_imm(b, sample, 8),
                            sample_low, sample_high);
      }
      nir_def *pos_ms = nir_vec3(b, nir_channel(b, sample_coords_int, 0),
                                        nir_channel(b, sample_coords_int, 1),
                                        sample);
      tex_data[i] = blorp_nir_txf_ms(b, v, pos_ms, mcs, key->texture_data_type);
   }

   nir_def *frac_x = nir_channel(b, frac_xy, 0);
   nir_def *frac_y = nir_channel(b, frac_xy, 1);
   return nir_flrp(b, nir_flrp(b, tex_data[0], tex_data[1], frac_x),
                      nir_flrp(b, tex_data[2], tex_data[3], frac_x),
                      frac_y);
}

/** Perform a color bit-cast operation
 *
 * For copy operations involving CCS, we may need to use different formats for
 * the source and destination surfaces.  The two formats must both be UINT
 * formats and must have the same size but may have different bit layouts.
 * For instance, we may be copying from R8G8B8A8_UINT to R32_UINT or R32_UINT
 * to R16G16_UINT.  This function generates code to shuffle bits around to get
 * us from one to the other.
 */
static nir_def *
bit_cast_color(struct nir_builder *b, nir_def *color,
               const struct brw_blorp_blit_prog_key *key)
{
   if (key->src_format == key->dst_format)
      return color;

   const struct isl_format_layout *src_fmtl =
      isl_format_get_layout(key->src_format);
   const struct isl_format_layout *dst_fmtl =
      isl_format_get_layout(key->dst_format);

   /* They must be formats with the same bit size */
   assert(src_fmtl->bpb == dst_fmtl->bpb);

   if (src_fmtl->bpb <= 32) {
      assert(src_fmtl->channels.r.type == ISL_UINT ||
             src_fmtl->channels.r.type == ISL_UNORM);
      assert(dst_fmtl->channels.r.type == ISL_UINT ||
             dst_fmtl->channels.r.type == ISL_UNORM);

      nir_def *packed = nir_imm_int(b, 0);
      for (unsigned c = 0; c < 4; c++) {
         if (src_fmtl->channels_array[c].bits == 0)
            continue;

         const unsigned chan_start_bit = src_fmtl->channels_array[c].start_bit;
         const unsigned chan_bits = src_fmtl->channels_array[c].bits;

         nir_def *chan =  nir_channel(b, color, c);
         if (src_fmtl->channels_array[c].type == ISL_UNORM)
            chan = nir_format_float_to_unorm(b, chan, &chan_bits);

         packed = nir_ior(b, packed, nir_shift_imm(b, chan, chan_start_bit));
      }

      nir_def *chans[4] = { };
      for (unsigned c = 0; c < 4; c++) {
         if (dst_fmtl->channels_array[c].bits == 0) {
            chans[c] = nir_imm_int(b, 0);
            continue;
         }

         const unsigned chan_start_bit = dst_fmtl->channels_array[c].start_bit;
         const unsigned chan_bits = dst_fmtl->channels_array[c].bits;
         chans[c] = nir_iand_imm(b, nir_shift_imm(b, packed, -(int)chan_start_bit),
                                    BITFIELD_MASK(chan_bits));

         if (dst_fmtl->channels_array[c].type == ISL_UNORM)
            chans[c] = nir_format_unorm_to_float(b, chans[c], &chan_bits);
      }
      color = nir_vec(b, chans, 4);
   } else {
      /* This path only supports UINT formats */
      assert(src_fmtl->channels.r.type == ISL_UINT);
      assert(dst_fmtl->channels.r.type == ISL_UINT);

      const unsigned src_bpc = src_fmtl->channels.r.bits;
      const unsigned dst_bpc = dst_fmtl->channels.r.bits;

      assert(src_fmtl->channels.g.bits == 0 ||
             src_fmtl->channels.g.bits == src_fmtl->channels.r.bits);
      assert(src_fmtl->channels.b.bits == 0 ||
             src_fmtl->channels.b.bits == src_fmtl->channels.r.bits);
      assert(src_fmtl->channels.a.bits == 0 ||
             src_fmtl->channels.a.bits == src_fmtl->channels.r.bits);
      assert(dst_fmtl->channels.g.bits == 0 ||
             dst_fmtl->channels.g.bits == dst_fmtl->channels.r.bits);
      assert(dst_fmtl->channels.b.bits == 0 ||
             dst_fmtl->channels.b.bits == dst_fmtl->channels.r.bits);
      assert(dst_fmtl->channels.a.bits == 0 ||
             dst_fmtl->channels.a.bits == dst_fmtl->channels.r.bits);

      /* Restrict to only the channels we actually have */
      const unsigned src_channels =
         isl_format_get_num_channels(key->src_format);
      color = nir_trim_vector(b, color, src_channels);

      color = nir_format_bitcast_uvec_unmasked(b, color, src_bpc, dst_bpc);
   }

   /* Blorp likes to assume that colors are vec4s */
   nir_def *u = nir_undef(b, 1, 32);
   nir_def *chans[4] = { u, u, u, u };
   for (unsigned i = 0; i < color->num_components; i++)
      chans[i] = nir_channel(b, color, i);
   return nir_vec4(b, chans[0], chans[1], chans[2], chans[3]);
}

static nir_def *
select_color_channel(struct nir_builder *b, nir_def *color,
                     nir_alu_type data_type,
                     enum isl_channel_select chan)
{
   if (chan == ISL_CHANNEL_SELECT_ZERO) {
      return nir_imm_int(b, 0);
   } else if (chan == ISL_CHANNEL_SELECT_ONE) {
      switch (data_type) {
      case nir_type_int:
      case nir_type_uint:
         return nir_imm_int(b, 1);
      case nir_type_float:
         return nir_imm_float(b, 1);
      default:
         unreachable("Invalid data type");
      }
   } else {
      assert((unsigned)(chan - ISL_CHANNEL_SELECT_RED) < 4);
      return nir_channel(b, color, chan - ISL_CHANNEL_SELECT_RED);
   }
}

static nir_def *
swizzle_color(struct nir_builder *b, nir_def *color,
              struct isl_swizzle swizzle, nir_alu_type data_type)
{
   return nir_vec4(b,
                   select_color_channel(b, color, data_type, swizzle.r),
                   select_color_channel(b, color, data_type, swizzle.g),
                   select_color_channel(b, color, data_type, swizzle.b),
                   select_color_channel(b, color, data_type, swizzle.a));
}

static nir_def *
convert_color(struct nir_builder *b, nir_def *color,
              const struct brw_blorp_blit_prog_key *key)
{
   /* All of our color conversions end up generating a single-channel color
    * value that we need to write out.
    */
   nir_def *value;

   if (key->dst_format == ISL_FORMAT_R24_UNORM_X8_TYPELESS) {
      /* The destination image is bound as R32_UINT but the data needs to be
       * in R24_UNORM_X8_TYPELESS.  The bottom 24 are the actual data and the
       * top 8 need to be zero.  We can accomplish this by simply multiplying
       * by a factor to scale things down.
       */
      unsigned factor = (1 << 24) - 1;
      value = nir_fsat(b, nir_channel(b, color, 0));
      value = nir_f2i32(b, nir_fmul_imm(b, value, factor));
   } else if (key->dst_format == ISL_FORMAT_L8_UNORM_SRGB) {
      value = nir_format_linear_to_srgb(b, nir_channel(b, color, 0));
   } else if (key->dst_format == ISL_FORMAT_R8G8B8_UNORM_SRGB) {
      value = nir_format_linear_to_srgb(b, color);
   } else if (key->dst_format == ISL_FORMAT_R9G9B9E5_SHAREDEXP) {
      value = nir_format_pack_r9g9b9e5(b, color);
   } else {
      unreachable("Unsupported format conversion");
   }

   nir_def *out_comps[4];
   for (unsigned i = 0; i < 4; i++) {
      if (i < value->num_components)
         out_comps[i] = nir_channel(b, value, i);
      else
         out_comps[i] = nir_undef(b, 1, 32);
   }
   return nir_vec(b, out_comps, 4);
}

/**
 * Generator for WM programs used in BLORP blits.
 *
 * The bulk of the work done by the WM program is to wrap and unwrap the
 * coordinate transformations used by the hardware to store surfaces in
 * memory.  The hardware transforms a pixel location (X, Y, S) (where S is the
 * sample index for a multisampled surface) to a memory offset by the
 * following formulas:
 *
 *   offset = tile(tiling_format, encode_msaa(num_samples, layout, X, Y, S))
 *   (X, Y, S) = decode_msaa(num_samples, layout, detile(tiling_format, offset))
 *
 * For a single-sampled surface, or for a multisampled surface using
 * INTEL_MSAA_LAYOUT_UMS, encode_msaa() and decode_msaa are the identity
 * function:
 *
 *   encode_msaa(1, NONE, X, Y, 0) = (X, Y, 0)
 *   decode_msaa(1, NONE, X, Y, 0) = (X, Y, 0)
 *   encode_msaa(n, UMS, X, Y, S) = (X, Y, S)
 *   decode_msaa(n, UMS, X, Y, S) = (X, Y, S)
 *
 * For a 4x multisampled surface using INTEL_MSAA_LAYOUT_IMS, encode_msaa()
 * embeds the sample number into bit 1 of the X and Y coordinates:
 *
 *   encode_msaa(4, IMS, X, Y, S) = (X', Y', 0)
 *     where X' = (X & ~0b1) << 1 | (S & 0b1) << 1 | (X & 0b1)
 *           Y' = (Y & ~0b1 ) << 1 | (S & 0b10) | (Y & 0b1)
 *   decode_msaa(4, IMS, X, Y, 0) = (X', Y', S)
 *     where X' = (X & ~0b11) >> 1 | (X & 0b1)
 *           Y' = (Y & ~0b11) >> 1 | (Y & 0b1)
 *           S = (Y & 0b10) | (X & 0b10) >> 1
 *
 * For an 8x multisampled surface using INTEL_MSAA_LAYOUT_IMS, encode_msaa()
 * embeds the sample number into bits 1 and 2 of the X coordinate and bit 1 of
 * the Y coordinate:
 *
 *   encode_msaa(8, IMS, X, Y, S) = (X', Y', 0)
 *     where X' = (X & ~0b1) << 2 | (S & 0b100) | (S & 0b1) << 1 | (X & 0b1)
 *           Y' = (Y & ~0b1) << 1 | (S & 0b10) | (Y & 0b1)
 *   decode_msaa(8, IMS, X, Y, 0) = (X', Y', S)
 *     where X' = (X & ~0b111) >> 2 | (X & 0b1)
 *           Y' = (Y & ~0b11) >> 1 | (Y & 0b1)
 *           S = (X & 0b100) | (Y & 0b10) | (X & 0b10) >> 1
 *
 * For X tiling, tile() combines together the low-order bits of the X and Y
 * coordinates in the pattern 0byyyxxxxxxxxx, creating 4k tiles that are 512
 * bytes wide and 8 rows high:
 *
 *   tile(x_tiled, X, Y, S) = A
 *     where A = tile_num << 12 | offset
 *           tile_num = (Y' >> 3) * tile_pitch + (X' >> 9)
 *           offset = (Y' & 0b111) << 9
 *                    | (X & 0b111111111)
 *           X' = X * cpp
 *           Y' = Y + S * qpitch
 *   detile(x_tiled, A) = (X, Y, S)
 *     where X = X' / cpp
 *           Y = Y' % qpitch
 *           S = Y' / qpitch
 *           Y' = (tile_num / tile_pitch) << 3
 *                | (A & 0b111000000000) >> 9
 *           X' = (tile_num % tile_pitch) << 9
 *                | (A & 0b111111111)
 *
 * (In all tiling formulas, cpp is the number of bytes occupied by a single
 * sample ("chars per pixel"), tile_pitch is the number of 4k tiles required
 * to fill the width of the surface, and qpitch is the spacing (in rows)
 * between array slices).
 *
 * For Y tiling, tile() combines together the low-order bits of the X and Y
 * coordinates in the pattern 0bxxxyyyyyxxxx, creating 4k tiles that are 128
 * bytes wide and 32 rows high:
 *
 *   tile(y_tiled, X, Y, S) = A
 *     where A = tile_num << 12 | offset
 *           tile_num = (Y' >> 5) * tile_pitch + (X' >> 7)
 *           offset = (X' & 0b1110000) << 5
 *                    | (Y' & 0b11111) << 4
 *                    | (X' & 0b1111)
 *           X' = X * cpp
 *           Y' = Y + S * qpitch
 *   detile(y_tiled, A) = (X, Y, S)
 *     where X = X' / cpp
 *           Y = Y' % qpitch
 *           S = Y' / qpitch
 *           Y' = (tile_num / tile_pitch) << 5
 *                | (A & 0b111110000) >> 4
 *           X' = (tile_num % tile_pitch) << 7
 *                | (A & 0b111000000000) >> 5
 *                | (A & 0b1111)
 *
 * For W tiling, tile() combines together the low-order bits of the X and Y
 * coordinates in the pattern 0bxxxyyyyxyxyx, creating 4k tiles that are 64
 * bytes wide and 64 rows high (note that W tiling is only used for stencil
 * buffers, which always have cpp = 1 and S=0):
 *
 *   tile(w_tiled, X, Y, S) = A
 *     where A = tile_num << 12 | offset
 *           tile_num = (Y' >> 6) * tile_pitch + (X' >> 6)
 *           offset = (X' & 0b111000) << 6
 *                    | (Y' & 0b111100) << 3
 *                    | (X' & 0b100) << 2
 *                    | (Y' & 0b10) << 2
 *                    | (X' & 0b10) << 1
 *                    | (Y' & 0b1) << 1
 *                    | (X' & 0b1)
 *           X' = X * cpp = X
 *           Y' = Y + S * qpitch
 *   detile(w_tiled, A) = (X, Y, S)
 *     where X = X' / cpp = X'
 *           Y = Y' % qpitch = Y'
 *           S = Y / qpitch = 0
 *           Y' = (tile_num / tile_pitch) << 6
 *                | (A & 0b111100000) >> 3
 *                | (A & 0b1000) >> 2
 *                | (A & 0b10) >> 1
 *           X' = (tile_num % tile_pitch) << 6
 *                | (A & 0b111000000000) >> 6
 *                | (A & 0b10000) >> 2
 *                | (A & 0b100) >> 1
 *                | (A & 0b1)
 *
 * Finally, for a non-tiled surface, tile() simply combines together the X and
 * Y coordinates in the natural way:
 *
 *   tile(untiled, X, Y, S) = A
 *     where A = Y * pitch + X'
 *           X' = X * cpp
 *           Y' = Y + S * qpitch
 *   detile(untiled, A) = (X, Y, S)
 *     where X = X' / cpp
 *           Y = Y' % qpitch
 *           S = Y' / qpitch
 *           X' = A % pitch
 *           Y' = A / pitch
 *
 * (In these formulas, pitch is the number of bytes occupied by a single row
 * of samples).
 */
static nir_shader *
brw_blorp_build_nir_shader(struct blorp_context *blorp,
                           struct blorp_batch *batch, void *mem_ctx,
                           const struct brw_blorp_blit_prog_key *key)
{
   const struct intel_device_info *devinfo = blorp->isl_dev->info;
   nir_def *src_pos, *dst_pos, *color;

   /* Sanity checks */
   if (key->dst_tiled_w && key->rt_samples > 1) {
      /* If the destination image is W tiled and multisampled, then the thread
       * must be dispatched once per sample, not once per pixel.  This is
       * necessary because after conversion between W and Y tiling, there's no
       * guarantee that all samples corresponding to a single pixel will still
       * be together.
       */
      assert(key->persample_msaa_dispatch);
   }

   if (key->persample_msaa_dispatch) {
      /* It only makes sense to do persample dispatch if the render target is
       * configured as multisampled.
       */
      assert(key->rt_samples > 0);
   }

   /* Make sure layout is consistent with sample count */
   assert((key->tex_layout == ISL_MSAA_LAYOUT_NONE) ==
          (key->tex_samples <= 1));
   assert((key->rt_layout == ISL_MSAA_LAYOUT_NONE) ==
          (key->rt_samples <= 1));
   assert((key->src_layout == ISL_MSAA_LAYOUT_NONE) ==
          (key->src_samples <= 1));
   assert((key->dst_layout == ISL_MSAA_LAYOUT_NONE) ==
          (key->dst_samples <= 1));

   nir_builder b;
   const bool compute =
      key->base.shader_pipeline == BLORP_SHADER_PIPELINE_COMPUTE;
   gl_shader_stage stage =
      compute ? MESA_SHADER_COMPUTE : MESA_SHADER_FRAGMENT;
   blorp_nir_init_shader(&b, mem_ctx, stage, NULL);

   struct brw_blorp_blit_vars v;
   brw_blorp_blit_vars_init(&b, &v, key);

   dst_pos = compute ?
      blorp_blit_get_cs_dst_coords(&b, key, &v) :
      blorp_blit_get_frag_coords(&b, key, &v);

   /* Render target and texture hardware don't support W tiling until Gfx8. */
   const bool rt_tiled_w = false;
   const bool tex_tiled_w = devinfo->ver >= 8 && key->src_tiled_w;

   /* The address that data will be written to is determined by the
    * coordinates supplied to the WM thread and the tiling and sample count of
    * the render target, according to the formula:
    *
    * (X, Y, S) = decode_msaa(rt_samples, detile(rt_tiling, offset))
    *
    * If the actual tiling and sample count of the destination surface are not
    * the same as the configuration of the render target, then these
    * coordinates are wrong and we have to adjust them to compensate for the
    * difference.
    */
   if (rt_tiled_w != key->dst_tiled_w ||
       key->rt_samples != key->dst_samples ||
       key->rt_layout != key->dst_layout) {
      dst_pos = blorp_nir_encode_msaa(&b, dst_pos, key->rt_samples,
                                      key->rt_layout);
      /* Now (X, Y, S) = detile(rt_tiling, offset) */
      if (rt_tiled_w != key->dst_tiled_w)
         dst_pos = blorp_nir_retile_y_to_w(&b, dst_pos);
      /* Now (X, Y, S) = detile(rt_tiling, offset) */
      dst_pos = blorp_nir_decode_msaa(&b, dst_pos, key->dst_samples,
                                      key->dst_layout);
   }

   nir_def *comp = NULL;
   if (key->dst_rgb) {
      /* The destination image is bound as a red texture three times as wide
       * as the actual image.  Our shader is effectively running one color
       * component at a time.  We need to save off the component and adjust
       * the destination position.
       */
      assert(dst_pos->num_components == 2);
      nir_def *dst_x = nir_channel(&b, dst_pos, 0);
      comp = nir_umod_imm(&b, dst_x, 3);
      dst_pos = nir_vec2(&b, nir_idiv(&b, dst_x, nir_imm_int(&b, 3)),
                             nir_channel(&b, dst_pos, 1));
   }

   /* Now (X, Y, S) = decode_msaa(dst_samples, detile(dst_tiling, offset)).
    *
    * That is: X, Y and S now contain the true coordinates and sample index of
    * the data that the WM thread should output.
    *
    * If we need to kill pixels that are outside the destination rectangle,
    * now is the time to do it.
    */
   nir_if *bounds_if = NULL;
   if (key->use_kill) {
      nir_def *bounds_rect = nir_load_var(&b, v.v_bounds_rect);
      nir_def *in_bounds = blorp_check_in_bounds(&b, bounds_rect,
                                                     dst_pos);
      if (!compute)
         nir_discard_if(&b, nir_inot(&b, in_bounds));
      else
         bounds_if = nir_push_if(&b, in_bounds);
   }

   src_pos = blorp_blit_apply_transform(&b, nir_i2f32(&b, dst_pos), &v);
   if (dst_pos->num_components == 3) {
      /* The sample coordinate is an integer that we want left alone but
       * blorp_blit_apply_transform() blindly applies the transform to all
       * three coordinates.  Grab the original sample index.
       */
      src_pos = nir_vec3(&b, nir_channel(&b, src_pos, 0),
                             nir_channel(&b, src_pos, 1),
                             nir_channel(&b, dst_pos, 2));
   }

   /* If the source image is not multisampled, then we want to fetch sample
    * number 0, because that's the only sample there is.
    */
   if (key->src_samples == 1)
      src_pos = nir_trim_vector(&b, src_pos, 2);

   /* X, Y, and S are now the coordinates of the pixel in the source image
    * that we want to texture from.  Exception: if we are blending, then S is
    * irrelevant, because we are going to fetch all samples.
    */
   switch (key->filter) {
   case BLORP_FILTER_NONE:
   case BLORP_FILTER_NEAREST:
   case BLORP_FILTER_SAMPLE_0:
      /* We're going to use texelFetch, so we need integers */
      if (src_pos->num_components == 2) {
         src_pos = nir_f2i32(&b, src_pos);
      } else {
         assert(src_pos->num_components == 3);
         src_pos = nir_vec3(&b, nir_channel(&b, nir_f2i32(&b, src_pos), 0),
                                nir_channel(&b, nir_f2i32(&b, src_pos), 1),
                                nir_channel(&b, src_pos, 2));
      }

      /* We aren't blending, which means we just want to fetch a single
       * sample from the source surface.  The address that we want to fetch
       * from is related to the X, Y and S values according to the formula:
       *
       * (X, Y, S) = decode_msaa(src_samples, detile(src_tiling, offset)).
       *
       * If the actual tiling and sample count of the source surface are
       * not the same as the configuration of the texture, then we need to
       * adjust the coordinates to compensate for the difference.
       */
      if (tex_tiled_w != key->src_tiled_w ||
          key->tex_samples != key->src_samples ||
          key->tex_layout != key->src_layout) {
         src_pos = blorp_nir_encode_msaa(&b, src_pos, key->src_samples,
                                         key->src_layout);
         /* Now (X, Y, S) = detile(src_tiling, offset) */
         if (tex_tiled_w != key->src_tiled_w)
            src_pos = blorp_nir_retile_w_to_y(&b, src_pos);
         /* Now (X, Y, S) = detile(tex_tiling, offset) */
         src_pos = blorp_nir_decode_msaa(&b, src_pos, key->tex_samples,
                                         key->tex_layout);
      }

      if (key->need_src_offset)
         src_pos = nir_iadd(&b, src_pos, nir_load_var(&b, v.v_src_offset));

      /* Now (X, Y, S) = decode_msaa(tex_samples, detile(tex_tiling, offset)).
       *
       * In other words: X, Y, and S now contain values which, when passed to
       * the texturing unit, will cause data to be read from the correct
       * memory location.  So we can fetch the texel now.
       */
      if (key->src_samples == 1) {
         color = blorp_nir_txf(&b, &v, src_pos, key->texture_data_type);
      } else {
         nir_def *mcs = NULL;
         if (isl_aux_usage_has_mcs(key->tex_aux_usage))
            mcs = blorp_blit_txf_ms_mcs(&b, &v, src_pos);

         color = blorp_nir_txf_ms(&b, &v, src_pos, mcs, key->texture_data_type);
      }
      break;

   case BLORP_FILTER_BILINEAR:
      assert(!key->src_tiled_w);
      assert(key->tex_samples == key->src_samples);
      assert(key->tex_layout == key->src_layout);

      if (key->src_samples == 1) {
         color = blorp_nir_tex(&b, &v, key, src_pos);
      } else {
         assert(!key->use_kill);
         color = blorp_nir_manual_blend_bilinear(&b, src_pos, key->src_samples,
                                                 key, &v);
      }
      break;

   case BLORP_FILTER_AVERAGE:
   case BLORP_FILTER_MIN_SAMPLE:
   case BLORP_FILTER_MAX_SAMPLE:
      assert(!key->src_tiled_w);
      assert(key->tex_samples == key->src_samples);
      assert(key->tex_layout == key->src_layout);

      /* Resolves (effecively) use texelFetch, so we need integers and we
       * don't care about the sample index if we got one.
       */
      src_pos = nir_f2i32(&b, nir_trim_vector(&b, src_pos, 2));

      if (devinfo->ver == 6) {
         /* Because gfx6 only supports 4x interleved MSAA, we can do all the
          * blending we need with a single linear-interpolated texture lookup
          * at the center of the sample. The texture coordinates to be odd
          * integers so that they correspond to the center of a 2x2 block
          * representing the four samples that maxe up a pixel.  So we need
          * to multiply our X and Y coordinates each by 2 and then add 1.
          */
         assert(key->src_coords_normalized);
         assert(key->filter == BLORP_FILTER_AVERAGE);
         src_pos = nir_fadd_imm(&b,
                                nir_i2f32(&b, src_pos),
                                0.5f);
         color = blorp_nir_tex(&b, &v, key, src_pos);
      } else {
         /* Gfx7+ hardware doesn't automatically blend. */
         color = blorp_nir_combine_samples(&b, &v, src_pos, key->src_samples,
                                           key->tex_aux_usage,
                                           key->texture_data_type,
                                           key->filter);
      }
      break;

   default:
      unreachable("Invalid blorp filter");
   }

   if (!isl_swizzle_is_identity(key->src_swizzle)) {
      color = swizzle_color(&b, color, key->src_swizzle,
                            key->texture_data_type);
   }

   if (!isl_swizzle_is_identity(key->dst_swizzle)) {
      color = swizzle_color(&b, color, isl_swizzle_invert(key->dst_swizzle),
                            nir_type_int);
   }

   if (key->format_bit_cast) {
      assert(isl_swizzle_is_identity(key->src_swizzle));
      assert(isl_swizzle_is_identity(key->dst_swizzle));
      color = bit_cast_color(&b, color, key);
   } else if (key->dst_format) {
      color = convert_color(&b, color, key);
   } else if (key->uint32_to_sint) {
      /* Normally the hardware will take care of converting values from/to
       * the source and destination formats.  But a few cases need help.
       *
       * The Skylake PRM, volume 07, page 658 has a programming note:
       *
       *    "When using SINT or UINT rendertarget surface formats, Blending
       *     must be DISABLED. The Pre-Blend Color Clamp Enable and Color
       *     Clamp Range fields are ignored, and an implied clamp to the
       *     rendertarget surface format is performed."
       *
       * For UINT to SINT blits, our sample operation gives us a uint32_t,
       * but our render target write expects a signed int32_t number.  If we
       * simply passed the value along, the hardware would interpret a value
       * with bit 31 set as a negative value, clamping it to the largest
       * negative number the destination format could represent.  But the
       * actual source value is a positive number, so we want to clamp it
       * to INT_MAX.  To fix this, we explicitly take min(color, INT_MAX).
       */
      color = nir_umin(&b, color, nir_imm_int(&b, INT32_MAX));
   } else if (key->sint32_to_uint) {
      /* Similar to above, but clamping negative numbers to zero. */
      color = nir_imax(&b, color, nir_imm_int(&b, 0));
   }

   if (key->dst_rgb) {
      /* The destination image is bound as a red texture three times as wide
       * as the actual image.  Our shader is effectively running one color
       * component at a time.  We need to pick off the appropriate component
       * from the source color and write that to destination red.
       */
      assert(dst_pos->num_components == 2);

      nir_def *color_component =
         nir_bcsel(&b, nir_ieq_imm(&b, comp, 0),
                       nir_channel(&b, color, 0),
                       nir_bcsel(&b, nir_ieq_imm(&b, comp, 1),
                                     nir_channel(&b, color, 1),
                                     nir_channel(&b, color, 2)));

      nir_def *u = nir_undef(&b, 1, 32);
      color = nir_vec4(&b, color_component, u, u, u);
   }

   if (compute) {
      nir_def *store_pos = nir_load_global_invocation_id(&b, 32);
      nir_image_store(&b, nir_imm_int(&b, 0),
                      nir_pad_vector_imm_int(&b, store_pos, 0, 4),
                      nir_imm_int(&b, 0),
                      nir_pad_vector_imm_int(&b, color, 0, 4),
                      nir_imm_int(&b, 0),
                      .image_dim = GLSL_SAMPLER_DIM_2D,
                      .image_array = true,
                      .access = ACCESS_NON_READABLE);
   } else if (key->dst_usage == ISL_SURF_USAGE_RENDER_TARGET_BIT) {
      nir_variable *color_out =
         nir_variable_create(b.shader, nir_var_shader_out,
                             glsl_vec4_type(), "gl_FragColor");
      color_out->data.location = FRAG_RESULT_COLOR;
      nir_store_var(&b, color_out, color, 0xf);
   } else if (key->dst_usage == ISL_SURF_USAGE_DEPTH_BIT) {
      nir_variable *depth_out =
         nir_variable_create(b.shader, nir_var_shader_out,
                             glsl_float_type(), "gl_FragDepth");
      depth_out->data.location = FRAG_RESULT_DEPTH;
      nir_store_var(&b, depth_out, nir_channel(&b, color, 0), 0x1);
   } else if (key->dst_usage == ISL_SURF_USAGE_STENCIL_BIT) {
      nir_variable *stencil_out =
         nir_variable_create(b.shader, nir_var_shader_out,
                             glsl_int_type(), "gl_FragStencilRef");
      stencil_out->data.location = FRAG_RESULT_STENCIL;
      nir_store_var(&b, stencil_out, nir_channel(&b, color, 0), 0x1);
   } else {
      unreachable("Invalid destination usage");
   }

   if (bounds_if)
      nir_pop_if(&b, bounds_if);

   return b.shader;
}

static bool
brw_blorp_get_blit_kernel_fs(struct blorp_batch *batch,
                             struct blorp_params *params,
                             const struct brw_blorp_blit_prog_key *key)
{
   struct blorp_context *blorp = batch->blorp;

   if (blorp->lookup_shader(batch, key, sizeof(*key),
                            &params->wm_prog_kernel, &params->wm_prog_data))
      return true;

   void *mem_ctx = ralloc_context(NULL);

   const unsigned *program;
   struct brw_wm_prog_data prog_data;

   nir_shader *nir = brw_blorp_build_nir_shader(blorp, batch, mem_ctx, key);
   nir->info.name =
      ralloc_strdup(nir, blorp_shader_type_to_name(key->base.shader_type));

   struct brw_wm_prog_key wm_key;
   brw_blorp_init_wm_prog_key(&wm_key);
   wm_key.multisample_fbo = key->rt_samples > 1 ? BRW_ALWAYS : BRW_NEVER;

   program = blorp_compile_fs(blorp, mem_ctx, nir, &wm_key, false,
                              &prog_data);

   bool result =
      blorp->upload_shader(batch, MESA_SHADER_FRAGMENT,
                           key, sizeof(*key),
                           program, prog_data.base.program_size,
                           &prog_data.base, sizeof(prog_data),
                           &params->wm_prog_kernel, &params->wm_prog_data);

   ralloc_free(mem_ctx);
   return result;
}

static bool
brw_blorp_get_blit_kernel_cs(struct blorp_batch *batch,
                             struct blorp_params *params,
                             const struct brw_blorp_blit_prog_key *prog_key)
{
   struct blorp_context *blorp = batch->blorp;

   if (blorp->lookup_shader(batch, prog_key, sizeof(*prog_key),
                            &params->cs_prog_kernel, &params->cs_prog_data))
      return true;

   void *mem_ctx = ralloc_context(NULL);

   const unsigned *program;
   struct brw_cs_prog_data prog_data;

   nir_shader *nir = brw_blorp_build_nir_shader(blorp, batch, mem_ctx,
                                                prog_key);
   nir->info.name = ralloc_strdup(nir, "BLORP-gpgpu-blit");
   blorp_set_cs_dims(nir, prog_key->local_y);

   struct brw_cs_prog_key cs_key;
   brw_blorp_init_cs_prog_key(&cs_key);
   assert(prog_key->rt_samples == 1);

   program = blorp_compile_cs(blorp, mem_ctx, nir, &cs_key, &prog_data);

   bool result =
      blorp->upload_shader(batch, MESA_SHADER_COMPUTE,
                           prog_key, sizeof(*prog_key),
                           program, prog_data.base.program_size,
                           &prog_data.base, sizeof(prog_data),
                           &params->cs_prog_kernel, &params->cs_prog_data);

   ralloc_free(mem_ctx);
   return result;
}

static void
brw_blorp_setup_coord_transform(struct brw_blorp_coord_transform *xform,
                                float src0, float src1,
                                float dst0, float dst1,
                                bool mirror)
{
   double scale = (double)(src1 - src0) / (double)(dst1 - dst0);
   if (!mirror) {
      /* When not mirroring a coordinate (say, X), we need:
       *   src_x - src_x0 = (dst_x - dst_x0 + 0.5) * scale
       * Therefore:
       *   src_x = src_x0 + (dst_x - dst_x0 + 0.5) * scale
       *
       * blorp program uses "round toward zero" to convert the
       * transformed floating point coordinates to integer coordinates,
       * whereas the behaviour we actually want is "round to nearest",
       * so 0.5 provides the necessary correction.
       */
      xform->multiplier = scale;
      xform->offset = src0 + (-(double)dst0 + 0.5) * scale;
   } else {
      /* When mirroring X we need:
       *   src_x - src_x0 = dst_x1 - dst_x - 0.5
       * Therefore:
       *   src_x = src_x0 + (dst_x1 -dst_x - 0.5) * scale
       */
      xform->multiplier = -scale;
      xform->offset = src0 + ((double)dst1 - 0.5) * scale;
   }
}

static inline void
surf_get_intratile_offset_px(struct brw_blorp_surface_info *info,
                             uint32_t *tile_x_px, uint32_t *tile_y_px)
{
   if (info->surf.msaa_layout == ISL_MSAA_LAYOUT_INTERLEAVED) {
      struct isl_extent2d px_size_sa =
         isl_get_interleaved_msaa_px_size_sa(info->surf.samples);
      assert(info->tile_x_sa % px_size_sa.width == 0);
      assert(info->tile_y_sa % px_size_sa.height == 0);
      *tile_x_px = info->tile_x_sa / px_size_sa.width;
      *tile_y_px = info->tile_y_sa / px_size_sa.height;
   } else {
      *tile_x_px = info->tile_x_sa;
      *tile_y_px = info->tile_y_sa;
   }
}

void
blorp_surf_convert_to_single_slice(const struct isl_device *isl_dev,
                                   struct brw_blorp_surface_info *info)
{
   bool ok UNUSED;

   /* It would be insane to try and do this on a compressed surface */
   assert(info->aux_usage == ISL_AUX_USAGE_NONE);

   /* Just bail if we have nothing to do. */
   if (info->surf.dim == ISL_SURF_DIM_2D &&
       info->view.base_level == 0 && info->view.base_array_layer == 0 &&
       info->surf.levels == 1 && info->surf.logical_level0_px.array_len == 1)
      return;

   /* If this gets triggered then we've gotten here twice which.  This
    * shouldn't happen thanks to the above early return.
    */
   assert(info->tile_x_sa == 0 && info->tile_y_sa == 0);

   uint32_t layer = 0, z = 0;
   if (info->surf.dim == ISL_SURF_DIM_3D)
      z = info->view.base_array_layer + info->z_offset;
   else
      layer = info->view.base_array_layer;

   uint64_t offset_B;
   isl_surf_get_image_surf(isl_dev, &info->surf,
                           info->view.base_level, layer, z,
                           &info->surf,
                           &offset_B, &info->tile_x_sa, &info->tile_y_sa);
   info->addr.offset += offset_B;

   uint32_t tile_x_px, tile_y_px;
   surf_get_intratile_offset_px(info, &tile_x_px, &tile_y_px);

   /* Instead of using the X/Y Offset fields in RENDER_SURFACE_STATE, we place
    * the image at the tile boundary and offset our sampling or rendering.
    * For this reason, we need to grow the image by the offset to ensure that
    * the hardware doesn't think we've gone past the edge.
    */
   info->surf.logical_level0_px.w += tile_x_px;
   info->surf.logical_level0_px.h += tile_y_px;
   info->surf.phys_level0_sa.w += info->tile_x_sa;
   info->surf.phys_level0_sa.h += info->tile_y_sa;

   /* The view is also different now. */
   info->view.base_level = 0;
   info->view.levels = 1;
   info->view.base_array_layer = 0;
   info->view.array_len = 1;
   info->z_offset = 0;
}

void
blorp_surf_fake_interleaved_msaa(const struct isl_device *isl_dev,
                                 struct brw_blorp_surface_info *info)
{
   assert(info->surf.msaa_layout == ISL_MSAA_LAYOUT_INTERLEAVED);

   /* First, we need to convert it to a simple 1-level 1-layer 2-D surface */
   blorp_surf_convert_to_single_slice(isl_dev, info);

   info->surf.logical_level0_px = info->surf.phys_level0_sa;
   info->surf.samples = 1;
   info->surf.msaa_layout = ISL_MSAA_LAYOUT_NONE;
}

void
blorp_surf_retile_w_to_y(const struct isl_device *isl_dev,
                         struct brw_blorp_surface_info *info)
{
   assert(info->surf.tiling == ISL_TILING_W);

   /* First, we need to convert it to a simple 1-level 1-layer 2-D surface */
   blorp_surf_convert_to_single_slice(isl_dev, info);

   /* On gfx7+, we don't have interleaved multisampling for color render
    * targets so we have to fake it.
    *
    * TODO: Are we sure we don't also need to fake it on gfx6?
    */
   if (isl_dev->info->ver > 6 &&
       info->surf.msaa_layout == ISL_MSAA_LAYOUT_INTERLEAVED) {
      blorp_surf_fake_interleaved_msaa(isl_dev, info);
   }

   if (isl_dev->info->ver == 6 || isl_dev->info->ver == 7) {
      /* Gfx6-7 stencil buffers have a very large alignment coming in from the
       * miptree.  It's out-of-bounds for what the surface state can handle.
       * Since we have a single layer and level, it doesn't really matter as
       * long as we don't pass a bogus value into isl_surf_fill_state().
       */
      info->surf.image_alignment_el = isl_extent3d(4, 2, 1);
   }

   /* Now that we've converted everything to a simple 2-D surface with only
    * one miplevel, we can go about retiling it.
    */
   const unsigned x_align = 8, y_align = info->surf.samples != 0 ? 8 : 4;
   info->surf.tiling = ISL_TILING_Y0;
   info->surf.logical_level0_px.width =
      ALIGN(info->surf.logical_level0_px.width, x_align) * 2;
   info->surf.logical_level0_px.height =
      ALIGN(info->surf.logical_level0_px.height, y_align) / 2;
   info->tile_x_sa *= 2;
   info->tile_y_sa /= 2;
}

static bool
can_shrink_surface(const struct brw_blorp_surface_info *surf)
{
   /* The current code doesn't support offsets into the aux buffers. This
    * should be possible, but we need to make sure the offset is page
    * aligned for both the surface and the aux buffer surface. Generally
    * this mean using the page aligned offset for the aux buffer.
    *
    * Currently the cases where we must split the blit are limited to cases
    * where we don't have a aux buffer.
    */
   if (surf->aux_addr.buffer != NULL)
      return false;

   /* We can't support splitting the blit for gen <= 7, because the qpitch
    * size is calculated by the hardware based on the surface height for
    * gen <= 7. In gen >= 8, the qpitch is controlled by the driver.
    */
   if (surf->surf.msaa_layout == ISL_MSAA_LAYOUT_ARRAY)
      return false;

   return true;
}

static unsigned
get_max_surface_size(const struct intel_device_info *devinfo,
                     const struct brw_blorp_surface_info *surf)
{
   const unsigned max = devinfo->ver >= 7 ? 16384 : 8192;
   if (split_blorp_blit_debug && can_shrink_surface(surf))
      return max >> 4; /* A smaller restriction when debug is enabled */
   else
      return max;
}

struct blt_axis {
   double src0, src1, dst0, dst1;
   bool mirror;
};

struct blt_coords {
   struct blt_axis x, y;
};

static enum isl_format
get_red_format_for_rgb_format(enum isl_format format)
{
   const struct isl_format_layout *fmtl = isl_format_get_layout(format);

   switch (fmtl->channels.r.bits) {
   case 8:
      switch (fmtl->channels.r.type) {
      case ISL_UNORM:
         return ISL_FORMAT_R8_UNORM;
      case ISL_SNORM:
         return ISL_FORMAT_R8_SNORM;
      case ISL_UINT:
         return ISL_FORMAT_R8_UINT;
      case ISL_SINT:
         return ISL_FORMAT_R8_SINT;
      default:
         unreachable("Invalid 8-bit RGB channel type");
      }
   case 16:
      switch (fmtl->channels.r.type) {
      case ISL_UNORM:
         return ISL_FORMAT_R16_UNORM;
      case ISL_SNORM:
         return ISL_FORMAT_R16_SNORM;
      case ISL_SFLOAT:
         return ISL_FORMAT_R16_FLOAT;
      case ISL_UINT:
         return ISL_FORMAT_R16_UINT;
      case ISL_SINT:
         return ISL_FORMAT_R16_SINT;
      default:
         unreachable("Invalid 8-bit RGB channel type");
      }
   case 32:
      switch (fmtl->channels.r.type) {
      case ISL_SFLOAT:
         return ISL_FORMAT_R32_FLOAT;
      case ISL_UINT:
         return ISL_FORMAT_R32_UINT;
      case ISL_SINT:
         return ISL_FORMAT_R32_SINT;
      default:
         unreachable("Invalid 8-bit RGB channel type");
      }
   default:
      unreachable("Invalid number of red channel bits");
   }
}

void
surf_fake_rgb_with_red(const struct isl_device *isl_dev,
                       struct brw_blorp_surface_info *info)
{
   blorp_surf_convert_to_single_slice(isl_dev, info);

   info->surf.logical_level0_px.width *= 3;
   info->surf.phys_level0_sa.width *= 3;
   info->tile_x_sa *= 3;

   enum isl_format red_format =
      get_red_format_for_rgb_format(info->view.format);

   assert(isl_format_get_layout(red_format)->channels.r.type ==
          isl_format_get_layout(info->view.format)->channels.r.type);
   assert(isl_format_get_layout(red_format)->channels.r.bits ==
          isl_format_get_layout(info->view.format)->channels.r.bits);

   info->surf.format = info->view.format = red_format;

   if (isl_dev->info->verx10 >= 125) {
      /* The horizontal alignment is in units of texels for NPOT formats, and
       * bytes for other formats. Since the only allowed alignment units are
       * powers of two, there's no way to convert the alignment.
       *
       * Thankfully, the value doesn't matter since we're only a single slice.
       * Pick one allowed by isl_gfx125_choose_image_alignment_el.
       */
      info->surf.image_alignment_el.w =
         128 / (isl_format_get_layout(red_format)->bpb / 8);
   }
}

enum blit_shrink_status {
   BLIT_NO_SHRINK = 0,
   BLIT_SRC_WIDTH_SHRINK   = (1 << 0),
   BLIT_DST_WIDTH_SHRINK   = (1 << 1),
   BLIT_SRC_HEIGHT_SHRINK  = (1 << 2),
   BLIT_DST_HEIGHT_SHRINK  = (1 << 3),
};

/* Try to blit. If the surface parameters exceed the size allowed by hardware,
 * then enum blit_shrink_status will be returned. If BLIT_NO_SHRINK is
 * returned, then the blit was successful.
 */
static enum blit_shrink_status
try_blorp_blit(struct blorp_batch *batch,
               struct blorp_params *params,
               struct brw_blorp_blit_prog_key *key,
               struct blt_coords *coords)
{
   const struct intel_device_info *devinfo = batch->blorp->isl_dev->info;

   if (params->dst.surf.usage & ISL_SURF_USAGE_DEPTH_BIT) {
      if (devinfo->ver >= 7) {
         /* We can render as depth on Gfx5 but there's no real advantage since
          * it doesn't support MSAA or HiZ.  On Gfx4, we can't always render
          * to depth due to issues with depth buffers and mip-mapping.  On
          * Gfx6, we can do everything but we have weird offsetting for HiZ
          * and stencil.  It's easier to just render using the color pipe
          * on those platforms.
          */
         key->dst_usage = ISL_SURF_USAGE_DEPTH_BIT;
      } else {
         key->dst_usage = ISL_SURF_USAGE_RENDER_TARGET_BIT;
      }
   } else if (params->dst.surf.usage & ISL_SURF_USAGE_STENCIL_BIT) {
      assert(params->dst.surf.format == ISL_FORMAT_R8_UINT);
      if (devinfo->ver >= 9 && !(batch->flags & BLORP_BATCH_USE_COMPUTE)) {
         key->dst_usage = ISL_SURF_USAGE_STENCIL_BIT;
      } else {
         key->dst_usage = ISL_SURF_USAGE_RENDER_TARGET_BIT;
      }
   } else {
      key->dst_usage = ISL_SURF_USAGE_RENDER_TARGET_BIT;
   }

   if (isl_format_has_sint_channel(params->src.view.format)) {
      key->texture_data_type = nir_type_int;
   } else if (isl_format_has_uint_channel(params->src.view.format)) {
      key->texture_data_type = nir_type_uint;
   } else {
      key->texture_data_type = nir_type_float;
   }

   /* src_samples and dst_samples are the true sample counts */
   key->src_samples = params->src.surf.samples;
   key->dst_samples = params->dst.surf.samples;

   key->tex_aux_usage = params->src.aux_usage;

   /* src_layout and dst_layout indicate the true MSAA layout used by src and
    * dst.
    */
   key->src_layout = params->src.surf.msaa_layout;
   key->dst_layout = params->dst.surf.msaa_layout;

   /* Round floating point values to nearest integer to avoid "off by one texel"
    * kind of errors when blitting.
    */
   params->x0 = params->wm_inputs.bounds_rect.x0 = round(coords->x.dst0);
   params->y0 = params->wm_inputs.bounds_rect.y0 = round(coords->y.dst0);
   params->x1 = params->wm_inputs.bounds_rect.x1 = round(coords->x.dst1);
   params->y1 = params->wm_inputs.bounds_rect.y1 = round(coords->y.dst1);

   brw_blorp_setup_coord_transform(&params->wm_inputs.coord_transform[0],
                                   coords->x.src0, coords->x.src1,
                                   coords->x.dst0, coords->x.dst1,
                                   coords->x.mirror);
   brw_blorp_setup_coord_transform(&params->wm_inputs.coord_transform[1],
                                   coords->y.src0, coords->y.src1,
                                   coords->y.dst0, coords->y.dst1,
                                   coords->y.mirror);


   if (devinfo->ver == 4) {
      /* The MinLOD and MinimumArrayElement don't work properly for cube maps.
       * Convert them to a single slice on gfx4.
       */
      if (params->dst.surf.usage & ISL_SURF_USAGE_CUBE_BIT) {
         blorp_surf_convert_to_single_slice(batch->blorp->isl_dev, &params->dst);
         key->need_dst_offset = true;
      }

      if (params->src.surf.usage & ISL_SURF_USAGE_CUBE_BIT) {
         blorp_surf_convert_to_single_slice(batch->blorp->isl_dev, &params->src);
         key->need_src_offset = true;
      }
   }

   if (devinfo->ver > 6 &&
       !isl_surf_usage_is_depth_or_stencil(key->dst_usage) &&
       params->dst.surf.msaa_layout == ISL_MSAA_LAYOUT_INTERLEAVED) {
      assert(params->dst.surf.samples > 1);

      /* We must expand the rectangle we send through the rendering pipeline,
       * to account for the fact that we are mapping the destination region as
       * single-sampled when it is in fact multisampled.  We must also align
       * it to a multiple of the multisampling pattern, because the
       * differences between multisampled and single-sampled surface formats
       * will mean that pixels are scrambled within the multisampling pattern.
       * TODO: what if this makes the coordinates too large?
       *
       * Note: this only works if the destination surface uses the IMS layout.
       * If it's UMS, then we have no choice but to set up the rendering
       * pipeline as multisampled.
       */
      struct isl_extent2d px_size_sa =
         isl_get_interleaved_msaa_px_size_sa(params->dst.surf.samples);
      params->x0 = ROUND_DOWN_TO(params->x0, 2) * px_size_sa.width;
      params->y0 = ROUND_DOWN_TO(params->y0, 2) * px_size_sa.height;
      params->x1 = ALIGN(params->x1, 2) * px_size_sa.width;
      params->y1 = ALIGN(params->y1, 2) * px_size_sa.height;

      blorp_surf_fake_interleaved_msaa(batch->blorp->isl_dev, &params->dst);

      key->use_kill = true;
      key->need_dst_offset = true;
   }

   if (params->dst.surf.tiling == ISL_TILING_W &&
       key->dst_usage != ISL_SURF_USAGE_STENCIL_BIT) {
      /* We must modify the rectangle we send through the rendering pipeline
       * (and the size and x/y offset of the destination surface), to account
       * for the fact that we are mapping it as Y-tiled when it is in fact
       * W-tiled.
       *
       * Both Y tiling and W tiling can be understood as organizations of
       * 32-byte sub-tiles; within each 32-byte sub-tile, the layout of pixels
       * is different, but the layout of the 32-byte sub-tiles within the 4k
       * tile is the same (8 sub-tiles across by 16 sub-tiles down, in
       * column-major order).  In Y tiling, the sub-tiles are 16 bytes wide
       * and 2 rows high; in W tiling, they are 8 bytes wide and 4 rows high.
       *
       * Therefore, to account for the layout differences within the 32-byte
       * sub-tiles, we must expand the rectangle so the X coordinates of its
       * edges are multiples of 8 (the W sub-tile width), and its Y
       * coordinates of its edges are multiples of 4 (the W sub-tile height).
       * Then we need to scale the X and Y coordinates of the rectangle to
       * account for the differences in aspect ratio between the Y and W
       * sub-tiles.  We need to modify the layer width and height similarly.
       *
       * A correction needs to be applied when MSAA is in use: since
       * INTEL_MSAA_LAYOUT_IMS uses an interleaving pattern whose height is 4,
       * we need to align the Y coordinates to multiples of 8, so that when
       * they are divided by two they are still multiples of 4.
       *
       * Note: Since the x/y offset of the surface will be applied using the
       * SURFACE_STATE command packet, it will be invisible to the swizzling
       * code in the shader; therefore it needs to be in a multiple of the
       * 32-byte sub-tile size.  Fortunately it is, since the sub-tile is 8
       * pixels wide and 4 pixels high (when viewed as a W-tiled stencil
       * buffer), and the miplevel alignment used for stencil buffers is 8
       * pixels horizontally and either 4 or 8 pixels vertically (see
       * intel_horizontal_texture_alignment_unit() and
       * intel_vertical_texture_alignment_unit()).
       *
       * Note: Also, since the SURFACE_STATE command packet can only apply
       * offsets that are multiples of 4 pixels horizontally and 2 pixels
       * vertically, it is important that the offsets will be multiples of
       * these sizes after they are converted into Y-tiled coordinates.
       * Fortunately they will be, since we know from above that the offsets
       * are a multiple of the 32-byte sub-tile size, and in Y-tiled
       * coordinates the sub-tile is 16 pixels wide and 2 pixels high.
       *
       * TODO: what if this makes the coordinates (or the texture size) too
       * large?
       */
      const unsigned x_align = 8;
      const unsigned y_align = params->dst.surf.samples != 0 ? 8 : 4;
      params->x0 = ROUND_DOWN_TO(params->x0, x_align) * 2;
      params->y0 = ROUND_DOWN_TO(params->y0, y_align) / 2;
      params->x1 = ALIGN(params->x1, x_align) * 2;
      params->y1 = ALIGN(params->y1, y_align) / 2;

      /* Retile the surface to Y-tiled */
      blorp_surf_retile_w_to_y(batch->blorp->isl_dev, &params->dst);

      key->dst_tiled_w = true;
      key->use_kill = true;
      key->need_dst_offset = true;

      if (params->dst.surf.samples > 1) {
         /* If the destination surface is a W-tiled multisampled stencil
          * buffer that we're mapping as Y tiled, then we need to arrange for
          * the WM program to run once per sample rather than once per pixel,
          * because the memory layout of related samples doesn't match between
          * W and Y tiling.
          */
         key->persample_msaa_dispatch = true;
      }
   }

   if (devinfo->ver < 8 && params->src.surf.tiling == ISL_TILING_W) {
      /* On Haswell and earlier, we have to fake W-tiled sources as Y-tiled.
       * Broadwell adds support for sampling from stencil.
       *
       * See the comments above concerning x/y offset alignment for the
       * destination surface.
       *
       * TODO: what if this makes the texture size too large?
       */
      blorp_surf_retile_w_to_y(batch->blorp->isl_dev, &params->src);

      key->src_tiled_w = true;
      key->need_src_offset = true;
   }

   /* tex_samples and rt_samples are the sample counts that are set up in
    * SURFACE_STATE.
    */
   key->tex_samples = params->src.surf.samples;
   key->rt_samples  = params->dst.surf.samples;

   /* tex_layout and rt_layout indicate the MSAA layout the GPU pipeline will
    * use to access the source and destination surfaces.
    */
   key->tex_layout = params->src.surf.msaa_layout;
   key->rt_layout = params->dst.surf.msaa_layout;

   if (params->src.surf.samples > 0 && params->dst.surf.samples > 1) {
      /* We are blitting from a multisample buffer to a multisample buffer, so
       * we must preserve samples within a pixel.  This means we have to
       * arrange for the WM program to run once per sample rather than once
       * per pixel.
       */
      key->persample_msaa_dispatch = true;
   }

   params->num_samples = params->dst.surf.samples;

   if ((key->filter == BLORP_FILTER_AVERAGE ||
        key->filter == BLORP_FILTER_BILINEAR) &&
       batch->blorp->isl_dev->info->ver <= 6) {
      /* Gfx4-5 don't support non-normalized texture coordinates */
      key->src_coords_normalized = true;
      params->wm_inputs.src_inv_size[0] =
         1.0f / u_minify(params->src.surf.logical_level0_px.width,
                         params->src.view.base_level);
      params->wm_inputs.src_inv_size[1] =
         1.0f / u_minify(params->src.surf.logical_level0_px.height,
                         params->src.view.base_level);
   }

   if (isl_format_get_layout(params->dst.view.format)->bpb % 3 == 0) {
      /* We can't render to  RGB formats natively because they aren't a
       * power-of-two size.  Instead, we fake them by using a red format
       * with the same channel type and size and emitting shader code to
       * only write one channel at a time.
       */
      params->x0 *= 3;
      params->x1 *= 3;

      /* If it happens to be sRGB, we need to force a conversion */
      if (params->dst.view.format == ISL_FORMAT_R8G8B8_UNORM_SRGB)
         key->dst_format = ISL_FORMAT_R8G8B8_UNORM_SRGB;

      surf_fake_rgb_with_red(batch->blorp->isl_dev, &params->dst);

      key->dst_rgb = true;
      key->need_dst_offset = true;
   } else if (isl_format_is_rgbx(params->dst.view.format)) {
      /* We can handle RGBX formats easily enough by treating them as RGBA */
      params->dst.view.format =
         isl_format_rgbx_to_rgba(params->dst.view.format);
   } else if (params->dst.view.format == ISL_FORMAT_R24_UNORM_X8_TYPELESS &&
              key->dst_usage != ISL_SURF_USAGE_DEPTH_BIT) {
      key->dst_format = params->dst.view.format;
      params->dst.view.format = ISL_FORMAT_R32_UINT;
   } else if (params->dst.view.format == ISL_FORMAT_A4B4G4R4_UNORM) {
      params->dst.view.swizzle =
         isl_swizzle_compose(params->dst.view.swizzle,
                             ISL_SWIZZLE(ALPHA, RED, GREEN, BLUE));
      params->dst.view.format = ISL_FORMAT_B4G4R4A4_UNORM;
   } else if (params->dst.view.format == ISL_FORMAT_L8_UNORM_SRGB) {
      key->dst_format = params->dst.view.format;
      params->dst.view.format = ISL_FORMAT_R8_UNORM;
   } else if (params->dst.view.format == ISL_FORMAT_R9G9B9E5_SHAREDEXP) {
      key->dst_format = params->dst.view.format;
      params->dst.view.format = ISL_FORMAT_R32_UINT;
   }

   if (devinfo->verx10 <= 70 &&
       !isl_swizzle_is_identity(params->src.view.swizzle)) {
      key->src_swizzle = params->src.view.swizzle;
      params->src.view.swizzle = ISL_SWIZZLE_IDENTITY;
   } else {
      key->src_swizzle = ISL_SWIZZLE_IDENTITY;
   }

   if (!isl_swizzle_supports_rendering(devinfo, params->dst.view.swizzle)) {
      key->dst_swizzle = params->dst.view.swizzle;
      params->dst.view.swizzle = ISL_SWIZZLE_IDENTITY;
   } else {
      key->dst_swizzle = ISL_SWIZZLE_IDENTITY;
   }

   if (params->src.tile_x_sa || params->src.tile_y_sa) {
      assert(key->need_src_offset);
      surf_get_intratile_offset_px(&params->src,
                                   &params->wm_inputs.src_offset.x,
                                   &params->wm_inputs.src_offset.y);
   }

   if (params->dst.tile_x_sa || params->dst.tile_y_sa) {
      assert(key->need_dst_offset);
      surf_get_intratile_offset_px(&params->dst,
                                   &params->wm_inputs.dst_offset.x,
                                   &params->wm_inputs.dst_offset.y);
      params->x0 += params->wm_inputs.dst_offset.x;
      params->y0 += params->wm_inputs.dst_offset.y;
      params->x1 += params->wm_inputs.dst_offset.x;
      params->y1 += params->wm_inputs.dst_offset.y;
   }

   /* For some texture types, we need to pass the layer through the sampler. */
   params->wm_inputs.src_z = params->src.z_offset;

   const bool compute =
      key->base.shader_pipeline == BLORP_SHADER_PIPELINE_COMPUTE;
   if (compute) {
      key->local_y = blorp_get_cs_local_y(params);

      unsigned workgroup_width = 16 / key->local_y;
      unsigned workgroup_height = key->local_y;

      /* If the rectangle being drawn isn't an exact multiple of the
       * workgroup size, we'll get extra invocations that should not
       * perform blits.  We need to set use_kill to bounds check and
       * prevent those invocations from blitting.
       */
      if ((params->x0 % workgroup_width) != 0 ||
          (params->x1 % workgroup_width) != 0 ||
          (params->y0 % workgroup_height) != 0 ||
          (params->y1 % workgroup_height) != 0)
         key->use_kill = true;
   }

   if (compute) {
      if (!brw_blorp_get_blit_kernel_cs(batch, params, key))
         return 0;
   } else {
      if (!brw_blorp_get_blit_kernel_fs(batch, params, key))
         return 0;

      if (!blorp_ensure_sf_program(batch, params))
         return 0;
   }

   unsigned result = 0;
   unsigned max_src_surface_size = get_max_surface_size(devinfo, &params->src);
   if (params->src.surf.logical_level0_px.width > max_src_surface_size)
      result |= BLIT_SRC_WIDTH_SHRINK;
   if (params->src.surf.logical_level0_px.height > max_src_surface_size)
      result |= BLIT_SRC_HEIGHT_SHRINK;

   unsigned max_dst_surface_size = get_max_surface_size(devinfo, &params->dst);
   if (params->dst.surf.logical_level0_px.width > max_dst_surface_size)
      result |= BLIT_DST_WIDTH_SHRINK;
   if (params->dst.surf.logical_level0_px.height > max_dst_surface_size)
      result |= BLIT_DST_HEIGHT_SHRINK;

   if (result == 0) {
      if (key->dst_usage == ISL_SURF_USAGE_DEPTH_BIT) {
         params->depth = params->dst;
         memset(&params->dst, 0, sizeof(params->dst));
      } else if (key->dst_usage == ISL_SURF_USAGE_STENCIL_BIT) {
         params->stencil = params->dst;
         params->stencil_mask = 0xff;
         memset(&params->dst, 0, sizeof(params->dst));
      }

      batch->blorp->exec(batch, params);
   }

   return result;
}

/* Adjust split blit source coordinates for the current destination
 * coordinates.
 */
static void
adjust_split_source_coords(const struct blt_axis *orig,
                           struct blt_axis *split_coords,
                           double scale)
{
   /* When scale is greater than 0, then we are growing from the start, so
    * src0 uses delta0, and src1 uses delta1. When scale is less than 0, the
    * source range shrinks from the end. In that case src0 is adjusted by
    * delta1, and src1 is adjusted by delta0.
    */
   double delta0 = scale * (split_coords->dst0 - orig->dst0);
   double delta1 = scale * (split_coords->dst1 - orig->dst1);
   split_coords->src0 = orig->src0 + (scale >= 0.0 ? delta0 : delta1);
   split_coords->src1 = orig->src1 + (scale >= 0.0 ? delta1 : delta0);
}

static struct isl_extent2d
get_px_size_sa(const struct isl_surf *surf)
{
   static const struct isl_extent2d one_to_one = { .w = 1, .h = 1 };

   if (surf->msaa_layout != ISL_MSAA_LAYOUT_INTERLEAVED)
      return one_to_one;
   else
      return isl_get_interleaved_msaa_px_size_sa(surf->samples);
}

static void
shrink_surface_params(const struct isl_device *dev,
                      struct brw_blorp_surface_info *info,
                      double *x0, double *x1, double *y0, double *y1)
{
   uint64_t offset_B;
   uint32_t x_offset_sa, y_offset_sa, size;
   struct isl_extent2d px_size_sa;
   int adjust;

   blorp_surf_convert_to_single_slice(dev, info);

   px_size_sa = get_px_size_sa(&info->surf);

   /* Because this gets called after we lower compressed images, the tile
    * offsets may be non-zero and we need to incorporate them in our
    * calculations.
    */
   x_offset_sa = (uint32_t)*x0 * px_size_sa.w + info->tile_x_sa;
   y_offset_sa = (uint32_t)*y0 * px_size_sa.h + info->tile_y_sa;
   uint32_t tile_z_sa, tile_a;
   isl_tiling_get_intratile_offset_sa(info->surf.tiling, info->surf.dim,
                                      info->surf.msaa_layout,
                                      info->surf.format, info->surf.samples,
                                      info->surf.row_pitch_B,
                                      info->surf.array_pitch_el_rows,
                                      x_offset_sa, y_offset_sa, 0, 0,
                                      &offset_B,
                                      &info->tile_x_sa, &info->tile_y_sa,
                                      &tile_z_sa, &tile_a);
   assert(tile_z_sa == 0 && tile_a == 0);

   info->addr.offset += offset_B;

   adjust = (int)info->tile_x_sa / px_size_sa.w - (int)*x0;
   *x0 += adjust;
   *x1 += adjust;
   info->tile_x_sa = 0;

   adjust = (int)info->tile_y_sa / px_size_sa.h - (int)*y0;
   *y0 += adjust;
   *y1 += adjust;
   info->tile_y_sa = 0;

   size = MIN2((uint32_t)ceil(*x1), info->surf.logical_level0_px.width);
   info->surf.logical_level0_px.width = size;
   info->surf.phys_level0_sa.width = size * px_size_sa.w;

   size = MIN2((uint32_t)ceil(*y1), info->surf.logical_level0_px.height);
   info->surf.logical_level0_px.height = size;
   info->surf.phys_level0_sa.height = size * px_size_sa.h;
}

static void
do_blorp_blit(struct blorp_batch *batch,
              const struct blorp_params *orig_params,
              struct brw_blorp_blit_prog_key *key,
              const struct blt_coords *orig)
{
   struct blorp_params params;
   struct blt_coords blit_coords;
   struct blt_coords split_coords = *orig;
   double w = orig->x.dst1 - orig->x.dst0;
   double h = orig->y.dst1 - orig->y.dst0;
   double x_scale = (orig->x.src1 - orig->x.src0) / w;
   double y_scale = (orig->y.src1 - orig->y.src0) / h;
   if (orig->x.mirror)
      x_scale = -x_scale;
   if (orig->y.mirror)
      y_scale = -y_scale;

   enum blit_shrink_status shrink = BLIT_NO_SHRINK;
   if (split_blorp_blit_debug) {
      if (can_shrink_surface(&orig_params->src))
         shrink |= BLIT_SRC_WIDTH_SHRINK | BLIT_SRC_HEIGHT_SHRINK;
      if (can_shrink_surface(&orig_params->dst))
         shrink |= BLIT_DST_WIDTH_SHRINK | BLIT_DST_HEIGHT_SHRINK;
   }

   bool x_done, y_done;
   do {
      params = *orig_params;
      blit_coords = split_coords;

      if (shrink & (BLIT_SRC_WIDTH_SHRINK | BLIT_SRC_HEIGHT_SHRINK)) {
         shrink_surface_params(batch->blorp->isl_dev, &params.src,
                               &blit_coords.x.src0, &blit_coords.x.src1,
                               &blit_coords.y.src0, &blit_coords.y.src1);
         key->need_src_offset = false;
      }

      if (shrink & (BLIT_DST_WIDTH_SHRINK | BLIT_DST_HEIGHT_SHRINK)) {
         shrink_surface_params(batch->blorp->isl_dev, &params.dst,
                               &blit_coords.x.dst0, &blit_coords.x.dst1,
                               &blit_coords.y.dst0, &blit_coords.y.dst1);
         key->need_dst_offset = false;
      }

      enum blit_shrink_status result =
         try_blorp_blit(batch, &params, key, &blit_coords);

      if (result & (BLIT_SRC_WIDTH_SHRINK | BLIT_SRC_HEIGHT_SHRINK))
         assert(can_shrink_surface(&orig_params->src));

      if (result & (BLIT_DST_WIDTH_SHRINK | BLIT_DST_HEIGHT_SHRINK))
         assert(can_shrink_surface(&orig_params->dst));

      if (result & (BLIT_SRC_WIDTH_SHRINK | BLIT_DST_WIDTH_SHRINK)) {
         w /= 2.0;
         assert(w >= 1.0);
         split_coords.x.dst1 = MIN2(split_coords.x.dst0 + w, orig->x.dst1);
         adjust_split_source_coords(&orig->x, &split_coords.x, x_scale);
      }
      if (result & (BLIT_SRC_HEIGHT_SHRINK | BLIT_DST_HEIGHT_SHRINK)) {
         h /= 2.0;
         assert(h >= 1.0);
         split_coords.y.dst1 = MIN2(split_coords.y.dst0 + h, orig->y.dst1);
         adjust_split_source_coords(&orig->y, &split_coords.y, y_scale);
      }

      if (result) {
         /* We may get less bits set on result than we had already, so make
          * sure we remember all the ways in which a resize is required.
          */
         shrink |= result;
         continue;
      }

      y_done = (orig->y.dst1 - split_coords.y.dst1 < 0.5);
      x_done = y_done && (orig->x.dst1 - split_coords.x.dst1 < 0.5);
      if (x_done) {
         break;
      } else if (y_done) {
         split_coords.x.dst0 += w;
         split_coords.x.dst1 = MIN2(split_coords.x.dst0 + w, orig->x.dst1);
         split_coords.y.dst0 = orig->y.dst0;
         split_coords.y.dst1 = MIN2(split_coords.y.dst0 + h, orig->y.dst1);
         adjust_split_source_coords(&orig->x, &split_coords.x, x_scale);
      } else {
         split_coords.y.dst0 += h;
         split_coords.y.dst1 = MIN2(split_coords.y.dst0 + h, orig->y.dst1);
         adjust_split_source_coords(&orig->y, &split_coords.y, y_scale);
      }
   } while (true);
}

bool
blorp_blit_supports_compute(struct blorp_context *blorp,
                            const struct isl_surf *src_surf,
                            const struct isl_surf *dst_surf,
                            enum isl_aux_usage dst_aux_usage)
{
   /* Our compiler doesn't currently support typed image writes with MSAA.
    * Also, our BLORP compute shaders don't handle multisampling cases.
    */
   if (dst_surf->samples > 1 || src_surf->samples > 1)
      return false;

   if (blorp->isl_dev->info->ver >= 12) {
      return dst_aux_usage == ISL_AUX_USAGE_FCV_CCS_E ||
             dst_aux_usage == ISL_AUX_USAGE_CCS_E ||
             dst_aux_usage == ISL_AUX_USAGE_NONE;
   } else if (blorp->isl_dev->info->ver >= 7) {
      return dst_aux_usage == ISL_AUX_USAGE_NONE;
   } else {
      /* No compute shader support */
      return false;
   }
}

bool
blorp_blitter_supports_aux(const struct intel_device_info *devinfo,
                           enum isl_aux_usage aux_usage)
{
   switch (aux_usage) {
   case ISL_AUX_USAGE_NONE:
      return true;
   case ISL_AUX_USAGE_CCS_E:
   case ISL_AUX_USAGE_FCV_CCS_E:
   case ISL_AUX_USAGE_STC_CCS:
      return devinfo->verx10 >= 125;
   default:
      return false;
   }
}

bool
blorp_copy_supports_blitter(struct blorp_context *blorp,
                            const struct isl_surf *src_surf,
                            const struct isl_surf *dst_surf,
                            enum isl_aux_usage src_aux_usage,
                            enum isl_aux_usage dst_aux_usage)
{
   const struct intel_device_info *devinfo = blorp->isl_dev->info;

   if (devinfo->ver < 12)
      return false;

   if (dst_surf->samples > 1 || src_surf->samples > 1)
      return false;

   if (!blorp_blitter_supports_aux(devinfo, dst_aux_usage))
      return false;

   if (!blorp_blitter_supports_aux(devinfo, src_aux_usage))
      return false;

   const struct isl_format_layout *fmtl =
      isl_format_get_layout(dst_surf->format);

   if (fmtl->bpb == 96) {
      /* XY_BLOCK_COPY_BLT mentions it doesn't support clear colors for 96bpp
       * formats, but none of them support CCS anyway, so it's a moot point.
       */
      assert(src_aux_usage == ISL_AUX_USAGE_NONE);
      assert(dst_aux_usage == ISL_AUX_USAGE_NONE);

      /* We can only support linear mode for 96bpp. */
      if (src_surf->tiling != ISL_TILING_LINEAR ||
          dst_surf->tiling != ISL_TILING_LINEAR)
         return false;
   }

   return true;
}

void
blorp_blit(struct blorp_batch *batch,
           const struct blorp_surf *src_surf,
           unsigned src_level, float src_layer,
           enum isl_format src_format, struct isl_swizzle src_swizzle,
           const struct blorp_surf *dst_surf,
           unsigned dst_level, unsigned dst_layer,
           enum isl_format dst_format, struct isl_swizzle dst_swizzle,
           float src_x0, float src_y0,
           float src_x1, float src_y1,
           float dst_x0, float dst_y0,
           float dst_x1, float dst_y1,
           enum blorp_filter filter,
           bool mirror_x, bool mirror_y)
{
   struct blorp_params params;
   blorp_params_init(&params);
   params.op = BLORP_OP_BLIT;
   const bool compute = batch->flags & BLORP_BATCH_USE_COMPUTE;
   if (compute) {
      assert(blorp_blit_supports_compute(batch->blorp,
                                         src_surf->surf, dst_surf->surf,
                                         dst_surf->aux_usage));
   }

   /* We cannot handle combined depth and stencil. */
   if (src_surf->surf->usage & ISL_SURF_USAGE_STENCIL_BIT)
      assert(src_surf->surf->format == ISL_FORMAT_R8_UINT);
   if (dst_surf->surf->usage & ISL_SURF_USAGE_STENCIL_BIT)
      assert(dst_surf->surf->format == ISL_FORMAT_R8_UINT);

   if (dst_surf->surf->usage & ISL_SURF_USAGE_STENCIL_BIT) {
      assert(src_surf->surf->usage & ISL_SURF_USAGE_STENCIL_BIT);
      /* Prior to Broadwell, we can't render to R8_UINT */
      if (batch->blorp->isl_dev->info->ver < 8) {
         src_format = ISL_FORMAT_R8_UNORM;
         dst_format = ISL_FORMAT_R8_UNORM;
      }
   }

   brw_blorp_surface_info_init(batch, &params.src, src_surf, src_level,
                               src_layer, src_format, false);
   brw_blorp_surface_info_init(batch, &params.dst, dst_surf, dst_level,
                               dst_layer, dst_format, true);

   params.src.view.swizzle = src_swizzle;
   params.dst.view.swizzle = dst_swizzle;

   const struct isl_format_layout *src_fmtl =
      isl_format_get_layout(params.src.view.format);

   struct brw_blorp_blit_prog_key key = {
      .base = BRW_BLORP_BASE_KEY_INIT(BLORP_SHADER_TYPE_BLIT),
      .base.shader_pipeline = compute ? BLORP_SHADER_PIPELINE_COMPUTE :
                                        BLORP_SHADER_PIPELINE_RENDER,
      .filter = filter,
      .sint32_to_uint = src_fmtl->channels.r.bits == 32 &&
                        isl_format_has_sint_channel(params.src.view.format) &&
                        isl_format_has_uint_channel(params.dst.view.format),
      .uint32_to_sint = src_fmtl->channels.r.bits == 32 &&
                        isl_format_has_uint_channel(params.src.view.format) &&
                        isl_format_has_sint_channel(params.dst.view.format),
   };

   params.shader_type = key.base.shader_type;
   params.shader_pipeline = key.base.shader_pipeline;

   /* Scaling factors used for bilinear filtering in multisample scaled
    * blits.
    */
   if (params.src.surf.samples == 16)
      key.x_scale = 4.0f;
   else
      key.x_scale = 2.0f;
   key.y_scale = params.src.surf.samples / key.x_scale;

   params.wm_inputs.rect_grid.x1 =
      u_minify(params.src.surf.logical_level0_px.width, src_level) *
      key.x_scale - 1.0f;
   params.wm_inputs.rect_grid.y1 =
      u_minify(params.src.surf.logical_level0_px.height, src_level) *
      key.y_scale - 1.0f;

   struct blt_coords coords = {
      .x = {
         .src0 = src_x0,
         .src1 = src_x1,
         .dst0 = dst_x0,
         .dst1 = dst_x1,
         .mirror = mirror_x
      },
      .y = {
         .src0 = src_y0,
         .src1 = src_y1,
         .dst0 = dst_y0,
         .dst1 = dst_y1,
         .mirror = mirror_y
      }
   };

   do_blorp_blit(batch, &params, &key, &coords);
}

static enum isl_format
get_copy_format_for_bpb(const struct isl_device *isl_dev, unsigned bpb)
{
   /* The choice of UNORM and UINT formats is very intentional here.  Most
    * of the time, we want to use a UINT format to avoid any rounding error
    * in the blit.  For stencil blits, R8_UINT is required by the hardware.
    * (It's the only format allowed in conjunction with W-tiling.)  Also we
    * intentionally use the 4-channel formats whenever we can.  This is so
    * that, when we do a RGB <-> RGBX copy, the two formats will line up
    * even though one of them is 3/4 the size of the other.  The choice of
    * UNORM vs. UINT is also very intentional because we don't have 8 or
    * 16-bit RGB UINT formats until Sky Lake so we have to use UNORM there.
    * Fortunately, the only time we should ever use two different formats in
    * the table below is for RGB -> RGBA blits and so we will never have any
    * UNORM/UINT mismatch.
    */
   if (ISL_GFX_VER(isl_dev) >= 9) {
      switch (bpb) {
      case 8:  return ISL_FORMAT_R8_UINT;
      case 16: return ISL_FORMAT_R8G8_UINT;
      case 24: return ISL_FORMAT_R8G8B8_UINT;
      case 32: return ISL_FORMAT_R8G8B8A8_UINT;
      case 48: return ISL_FORMAT_R16G16B16_UINT;
      case 64: return ISL_FORMAT_R16G16B16A16_UINT;
      case 96: return ISL_FORMAT_R32G32B32_UINT;
      case 128:return ISL_FORMAT_R32G32B32A32_UINT;
      default:
         unreachable("Unknown format bpb");
      }
   } else {
      switch (bpb) {
      case 8:  return ISL_FORMAT_R8_UINT;
      case 16: return ISL_FORMAT_R8G8_UINT;
      case 24: return ISL_FORMAT_R8G8B8_UNORM;
      case 32: return ISL_FORMAT_R8G8B8A8_UNORM;
      case 48: return ISL_FORMAT_R16G16B16_UNORM;
      case 64: return ISL_FORMAT_R16G16B16A16_UNORM;
      case 96: return ISL_FORMAT_R32G32B32_UINT;
      case 128:return ISL_FORMAT_R32G32B32A32_UINT;
      default:
         unreachable("Unknown format bpb");
      }
   }
}

/** Returns a UINT format that is CCS-compatible with the given format
 *
 * The PRM's say absolutely nothing about how render compression works.  The
 * only thing they provide is a list of formats on which it is and is not
 * supported.  Empirical testing indicates that the compression is only based
 * on the bit-layout of the format and the channel encoding doesn't matter.
 * So, while texture views don't work in general, you can create a view as
 * long as the bit-layout of the formats are the same.
 *
 * Fortunately, for every render compression capable format, the UINT format
 * with the same bit layout also supports render compression.  This means that
 * we only need to handle UINT formats for copy operations.  In order to do
 * copies between formats with different bit layouts, we attach both with a
 * UINT format and use bit_cast_color() to generate code to do the bit-cast
 * operation between the two bit layouts.
 */
static enum isl_format
get_ccs_compatible_copy_format(const struct isl_format_layout *fmtl)
{
   switch (fmtl->format) {
   case ISL_FORMAT_R32G32B32A32_FLOAT:
   case ISL_FORMAT_R32G32B32A32_SINT:
   case ISL_FORMAT_R32G32B32A32_UINT:
   case ISL_FORMAT_R32G32B32A32_UNORM:
   case ISL_FORMAT_R32G32B32A32_SNORM:
   case ISL_FORMAT_R32G32B32X32_FLOAT:
      return ISL_FORMAT_R32G32B32A32_UINT;

   case ISL_FORMAT_R16G16B16A16_UNORM:
   case ISL_FORMAT_R16G16B16A16_SNORM:
   case ISL_FORMAT_R16G16B16A16_SINT:
   case ISL_FORMAT_R16G16B16A16_UINT:
   case ISL_FORMAT_R16G16B16A16_FLOAT:
   case ISL_FORMAT_R16G16B16X16_UNORM:
   case ISL_FORMAT_R16G16B16X16_FLOAT:
      return ISL_FORMAT_R16G16B16A16_UINT;

   case ISL_FORMAT_R32G32_FLOAT:
   case ISL_FORMAT_R32G32_SINT:
   case ISL_FORMAT_R32G32_UINT:
   case ISL_FORMAT_R32G32_UNORM:
   case ISL_FORMAT_R32G32_SNORM:
      return ISL_FORMAT_R32G32_UINT;

   case ISL_FORMAT_B8G8R8A8_UNORM:
   case ISL_FORMAT_B8G8R8A8_UNORM_SRGB:
   case ISL_FORMAT_R8G8B8A8_UNORM:
   case ISL_FORMAT_R8G8B8A8_UNORM_SRGB:
   case ISL_FORMAT_R8G8B8A8_SNORM:
   case ISL_FORMAT_R8G8B8A8_SINT:
   case ISL_FORMAT_R8G8B8A8_UINT:
   case ISL_FORMAT_B8G8R8X8_UNORM:
   case ISL_FORMAT_B8G8R8X8_UNORM_SRGB:
   case ISL_FORMAT_R8G8B8X8_UNORM:
   case ISL_FORMAT_R8G8B8X8_UNORM_SRGB:
      return ISL_FORMAT_R8G8B8A8_UINT;

   case ISL_FORMAT_R16G16_UNORM:
   case ISL_FORMAT_R16G16_SNORM:
   case ISL_FORMAT_R16G16_SINT:
   case ISL_FORMAT_R16G16_UINT:
   case ISL_FORMAT_R16G16_FLOAT:
      return ISL_FORMAT_R16G16_UINT;

   case ISL_FORMAT_R32_SINT:
   case ISL_FORMAT_R32_UINT:
   case ISL_FORMAT_R32_FLOAT:
   case ISL_FORMAT_R32_UNORM:
   case ISL_FORMAT_R32_SNORM:
      return ISL_FORMAT_R32_UINT;

   case ISL_FORMAT_R11G11B10_FLOAT:
      return ISL_FORMAT_R8G8B8A8_UINT;

   case ISL_FORMAT_B10G10R10A2_UNORM:
   case ISL_FORMAT_B10G10R10A2_UNORM_SRGB:
   case ISL_FORMAT_R10G10B10A2_UNORM:
   case ISL_FORMAT_R10G10B10A2_UNORM_SRGB:
   case ISL_FORMAT_R10G10B10_FLOAT_A2_UNORM:
   case ISL_FORMAT_R10G10B10A2_UINT:
      return ISL_FORMAT_R10G10B10A2_UINT;

   case ISL_FORMAT_R16_UNORM:
   case ISL_FORMAT_R16_SNORM:
   case ISL_FORMAT_R16_SINT:
   case ISL_FORMAT_R16_UINT:
   case ISL_FORMAT_R16_FLOAT:
      return ISL_FORMAT_R16_UINT;

   case ISL_FORMAT_R8G8_UNORM:
   case ISL_FORMAT_R8G8_SNORM:
   case ISL_FORMAT_R8G8_SINT:
   case ISL_FORMAT_R8G8_UINT:
      return ISL_FORMAT_R8G8_UINT;

   case ISL_FORMAT_YCRCB_NORMAL:
   case ISL_FORMAT_YCRCB_SWAPY:
   case ISL_FORMAT_YCRCB_SWAPUV:
   case ISL_FORMAT_YCRCB_SWAPUVY:
      /* Tiger Lake starts claiming CCS_E support for certain YCRCB formats.
       * BLORP chooses to take the CCS-compatible format path whenever ISL
       * claims CCS_E support on a format, not when CCS_E is actually used.
       * Therefore, if these formats are going to be used with BLORP, we need
       * a CCS-compatible format. R8G8_UINT seems as good as any.
       */
      return ISL_FORMAT_R8G8_UINT;

   case ISL_FORMAT_B5G5R5X1_UNORM:
   case ISL_FORMAT_B5G5R5X1_UNORM_SRGB:
   case ISL_FORMAT_B5G5R5A1_UNORM:
   case ISL_FORMAT_B5G5R5A1_UNORM_SRGB:
      return ISL_FORMAT_B5G5R5A1_UNORM;

   case ISL_FORMAT_A4B4G4R4_UNORM:
   case ISL_FORMAT_B4G4R4A4_UNORM:
   case ISL_FORMAT_B4G4R4A4_UNORM_SRGB:
      return ISL_FORMAT_B4G4R4A4_UNORM;

   case ISL_FORMAT_B5G6R5_UNORM:
   case ISL_FORMAT_B5G6R5_UNORM_SRGB:
      return ISL_FORMAT_B5G6R5_UNORM;

   case ISL_FORMAT_A1B5G5R5_UNORM:
      return ISL_FORMAT_A1B5G5R5_UNORM;

   case ISL_FORMAT_A8_UNORM:
   case ISL_FORMAT_R8_UNORM:
   case ISL_FORMAT_R8_SNORM:
   case ISL_FORMAT_R8_SINT:
   case ISL_FORMAT_R8_UINT:
      return ISL_FORMAT_R8_UINT;

   default:
      unreachable("Not a compressible format");
   }
}

void
blorp_surf_convert_to_uncompressed(const struct isl_device *isl_dev,
                                   struct brw_blorp_surface_info *info,
                                   uint32_t *x, uint32_t *y,
                                   uint32_t *width, uint32_t *height)
{
   const struct isl_format_layout *fmtl =
      isl_format_get_layout(info->surf.format);

   assert(fmtl->bw > 1 || fmtl->bh > 1);

   /* This should be the first modification made to the surface */
   assert(info->tile_x_sa == 0 && info->tile_y_sa == 0);

   if (width && height) {
      ASSERTED const uint32_t level_width =
         u_minify(info->surf.logical_level0_px.width, info->view.base_level);
      ASSERTED const uint32_t level_height =
         u_minify(info->surf.logical_level0_px.height, info->view.base_level);
      assert(*width % fmtl->bw == 0 || *x + *width == level_width);
      assert(*height % fmtl->bh == 0 || *y + *height == level_height);
      *width = DIV_ROUND_UP(*width, fmtl->bw);
      *height = DIV_ROUND_UP(*height, fmtl->bh);
   }

   if (x && y) {
      assert(*x % fmtl->bw == 0);
      assert(*y % fmtl->bh == 0);
      *x /= fmtl->bw;
      *y /= fmtl->bh;
   }

   /* We only want one level and slice */
   info->view.levels = 1;
   info->view.array_len = 1;

   if (info->surf.dim == ISL_SURF_DIM_3D) {
      /* Roll the Z offset into the image view */
      info->view.base_array_layer += info->z_offset;
      info->z_offset = 0;
   }

   uint64_t offset_B;
   ASSERTED bool ok =
      isl_surf_get_uncompressed_surf(isl_dev, &info->surf, &info->view,
                                     &info->surf, &info->view, &offset_B,
                                     &info->tile_x_sa, &info->tile_y_sa);
   assert(ok);
   info->addr.offset += offset_B;

   /* BLORP doesn't use the actual intratile offsets.  Instead, it needs the
    * surface to be a bit bigger and we offset the vertices instead. Standard
    * tilings don't need intratile offsets because each subresource is aligned
    * to a bpb-based tile boundary or miptail slot offset.
    */
  if (info->surf.tiling == ISL_TILING_64 ||
      isl_tiling_is_std_y(info->surf.tiling)) {
      assert(info->tile_x_sa == 0 && info->tile_y_sa == 0);
   } else {
      assert(info->surf.dim == ISL_SURF_DIM_2D);
      assert(info->surf.logical_level0_px.array_len == 1);
      info->surf.logical_level0_px.w += info->tile_x_sa;
      info->surf.logical_level0_px.h += info->tile_y_sa;
      info->surf.phys_level0_sa.w += info->tile_x_sa;
      info->surf.phys_level0_sa.h += info->tile_y_sa;
   }
}

bool
blorp_copy_supports_compute(struct blorp_context *blorp,
                            const struct isl_surf *src_surf,
                            const struct isl_surf *dst_surf,
                            enum isl_aux_usage dst_aux_usage)
{
   return blorp_blit_supports_compute(blorp, src_surf, dst_surf, dst_aux_usage);
}

void
blorp_copy_get_formats(const struct isl_device *isl_dev,
                       const struct isl_surf *src_surf,
                       const struct isl_surf *dst_surf,
                       enum isl_format *src_view_format,
                       enum isl_format *dst_view_format)
{
   const struct isl_format_layout *src_fmtl =
      isl_format_get_layout(src_surf->format);
   const struct isl_format_layout *dst_fmtl =
      isl_format_get_layout(dst_surf->format);

   if (ISL_GFX_VER(isl_dev) >= 8 &&
       isl_surf_usage_is_depth(src_surf->usage)) {
      /* In order to use HiZ, we have to use the real format for the source.
       * Depth <-> Color copies are not allowed.
       */
      *src_view_format = src_surf->format;
      *dst_view_format = src_surf->format;
   } else if (ISL_GFX_VER(isl_dev) >= 7 &&
              isl_surf_usage_is_depth(dst_surf->usage)) {
      /* On Gfx7 and higher, we use actual depth writes for blits into depth
       * buffers so we need the real format.
       */
      *src_view_format = dst_surf->format;
      *dst_view_format = dst_surf->format;
   } else if (isl_surf_usage_is_depth(src_surf->usage) ||
              isl_surf_usage_is_depth(dst_surf->usage)) {
      assert(src_fmtl->bpb == dst_fmtl->bpb);
      *src_view_format =
      *dst_view_format =
         get_copy_format_for_bpb(isl_dev, dst_fmtl->bpb);
   } else if (isl_format_supports_ccs_e(isl_dev->info, dst_surf->format)) {
      *dst_view_format = get_ccs_compatible_copy_format(dst_fmtl);
      if (isl_format_supports_ccs_e(isl_dev->info, src_surf->format)) {
         *src_view_format = get_ccs_compatible_copy_format(src_fmtl);
      } else if (src_fmtl->bpb == dst_fmtl->bpb) {
         *src_view_format = *dst_view_format;
      } else {
         *src_view_format = get_copy_format_for_bpb(isl_dev, src_fmtl->bpb);
      }
   } else if (isl_format_supports_ccs_e(isl_dev->info, src_surf->format)) {
      *src_view_format = get_ccs_compatible_copy_format(src_fmtl);
      if (src_fmtl->bpb == dst_fmtl->bpb) {
         *dst_view_format = *src_view_format;
      } else {
         *dst_view_format = get_copy_format_for_bpb(isl_dev, dst_fmtl->bpb);
      }
   } else {
      *dst_view_format = get_copy_format_for_bpb(isl_dev, dst_fmtl->bpb);
      *src_view_format = get_copy_format_for_bpb(isl_dev, src_fmtl->bpb);
   }
}


void
blorp_copy(struct blorp_batch *batch,
           const struct blorp_surf *src_surf,
           unsigned src_level, unsigned src_layer,
           const struct blorp_surf *dst_surf,
           unsigned dst_level, unsigned dst_layer,
           uint32_t src_x, uint32_t src_y,
           uint32_t dst_x, uint32_t dst_y,
           uint32_t src_width, uint32_t src_height)
{
   const struct isl_device *isl_dev = batch->blorp->isl_dev;
   const struct intel_device_info *devinfo = isl_dev->info;
   struct blorp_params params;

   if (src_width == 0 || src_height == 0)
      return;

   blorp_params_init(&params);
   params.op = BLORP_OP_COPY;

   const bool compute = batch->flags & BLORP_BATCH_USE_COMPUTE;
   if (compute) {
      assert(blorp_copy_supports_compute(batch->blorp,
                                         src_surf->surf, dst_surf->surf,
                                         dst_surf->aux_usage));
   } else if (batch->flags & BLORP_BATCH_USE_BLITTER) {
      assert(blorp_copy_supports_blitter(batch->blorp,
                                         src_surf->surf, dst_surf->surf,
                                         src_surf->aux_usage,
                                         dst_surf->aux_usage));
   }

   brw_blorp_surface_info_init(batch, &params.src, src_surf, src_level,
                               src_layer, ISL_FORMAT_UNSUPPORTED, false);
   brw_blorp_surface_info_init(batch, &params.dst, dst_surf, dst_level,
                               dst_layer, ISL_FORMAT_UNSUPPORTED, true);

   struct brw_blorp_blit_prog_key key = {
      .base = BRW_BLORP_BASE_KEY_INIT(BLORP_SHADER_TYPE_COPY),
      .base.shader_pipeline = compute ? BLORP_SHADER_PIPELINE_COMPUTE :
                                        BLORP_SHADER_PIPELINE_RENDER,
      .filter = BLORP_FILTER_NONE,
      .need_src_offset = src_surf->tile_x_sa || src_surf->tile_y_sa,
      .need_dst_offset = dst_surf->tile_x_sa || dst_surf->tile_y_sa,
   };

   params.shader_type = key.base.shader_type;
   params.shader_pipeline = key.base.shader_pipeline;

   const struct isl_format_layout *src_fmtl =
      isl_format_get_layout(params.src.surf.format);
   const struct isl_format_layout *dst_fmtl =
      isl_format_get_layout(params.dst.surf.format);

   assert(params.src.aux_usage == ISL_AUX_USAGE_NONE ||
          params.src.aux_usage == ISL_AUX_USAGE_HIZ ||
          params.src.aux_usage == ISL_AUX_USAGE_HIZ_CCS_WT ||
          params.src.aux_usage == ISL_AUX_USAGE_MCS ||
          params.src.aux_usage == ISL_AUX_USAGE_MCS_CCS ||
          params.src.aux_usage == ISL_AUX_USAGE_CCS_E ||
          params.src.aux_usage == ISL_AUX_USAGE_FCV_CCS_E ||
          params.src.aux_usage == ISL_AUX_USAGE_STC_CCS);

   blorp_copy_get_formats(isl_dev, &params.src.surf, &params.dst.surf,
                          &params.src.view.format, &params.dst.view.format);

   if (isl_aux_usage_has_fast_clears(params.src.aux_usage) &&
       isl_dev->ss.clear_color_state_size > 0) {
      /* For 32bpc formats, the sampler fetches the raw clear color dwords
       * used for rendering instead of the converted pixel dwords typically
       * used for sampling. The CLEAR_COLOR struct page documents this for
       * 128bpp formats, but not for 32bpp and 64bpp formats.
       *
       * Note that although the sampler doesn't use the converted clear color
       * field with 32bpc formats, the Clear Color Conversion hardware feature
       * still occurs when the format sizes are less than 128bpp.
       *
       * The sampler changing its clear color fetching location can be a
       * problem in some cases, but we won't run into them here. When using an
       * indirect clear color, we won't create 32bpc views of non-32bpc
       * surfaces (and vice-versa).
       */
      const struct isl_format_layout *src_view_fmtl =
         isl_format_get_layout(params.src.view.format);
      assert((src_fmtl->channels.r.bits == 32) ==
             (src_view_fmtl->channels.r.bits == 32));
   }

   if (params.src.view.format != params.dst.view.format) {
      enum isl_format src_cast_format = params.src.view.format;
      enum isl_format dst_cast_format = params.dst.view.format;

      /* The BLORP bitcast code gets confused by RGB formats.  Just treat them
       * as RGBA and then everything will be happy.  This is perfectly safe
       * because BLORP likes to treat things as if they have vec4 colors all
       * the time anyway.
       */
      if (isl_format_get_layout(src_cast_format)->bpb % 3 == 0)
         src_cast_format = isl_format_rgb_to_rgba(src_cast_format);
      if (isl_format_get_layout(dst_cast_format)->bpb % 3 == 0)
         dst_cast_format = isl_format_rgb_to_rgba(dst_cast_format);

      if (src_cast_format != dst_cast_format) {
         key.format_bit_cast = true;
         key.src_format = src_cast_format;
         key.dst_format = dst_cast_format;
      }
   }

   if (src_fmtl->bw > 1 || src_fmtl->bh > 1) {
      blorp_surf_convert_to_uncompressed(batch->blorp->isl_dev, &params.src,
                                         &src_x, &src_y,
                                         &src_width, &src_height);
      key.need_src_offset = true;
   }

   if (dst_fmtl->bw > 1 || dst_fmtl->bh > 1) {
      blorp_surf_convert_to_uncompressed(batch->blorp->isl_dev, &params.dst,
                                         &dst_x, &dst_y, NULL, NULL);
      key.need_dst_offset = true;
   }

   /* Once both surfaces are stompped to uncompressed as needed, the
    * destination size is the same as the source size.
    */
   uint32_t dst_width = src_width;
   uint32_t dst_height = src_height;

   if (batch->flags & BLORP_BATCH_USE_BLITTER) {
      if (devinfo->verx10 < 125) {
         blorp_surf_convert_to_single_slice(isl_dev, &params.dst);
         blorp_surf_convert_to_single_slice(isl_dev, &params.src);
      }

      params.x0 = dst_x;
      params.x1 = dst_x + dst_width;
      params.y0 = dst_y;
      params.y1 = dst_y + dst_height;
      params.wm_inputs.coord_transform[0].offset = dst_x - (float)src_x;
      params.wm_inputs.coord_transform[1].offset = dst_y - (float)src_y;
      params.wm_inputs.coord_transform[0].multiplier = 1.0f;
      params.wm_inputs.coord_transform[1].multiplier = 1.0f;

      batch->blorp->exec(batch, &params);
      return;
   }

   struct blt_coords coords = {
      .x = {
         .src0 = src_x,
         .src1 = src_x + src_width,
         .dst0 = dst_x,
         .dst1 = dst_x + dst_width,
         .mirror = false
      },
      .y = {
         .src0 = src_y,
         .src1 = src_y + src_height,
         .dst0 = dst_y,
         .dst1 = dst_y + dst_height,
         .mirror = false
      }
   };

   do_blorp_blit(batch, &params, &key, &coords);
}

static enum isl_format
isl_format_for_size(unsigned size_B)
{
   switch (size_B) {
   case 1:  return ISL_FORMAT_R8_UINT;
   case 2:  return ISL_FORMAT_R8G8_UINT;
   case 4:  return ISL_FORMAT_R8G8B8A8_UINT;
   case 8:  return ISL_FORMAT_R16G16B16A16_UINT;
   case 16: return ISL_FORMAT_R32G32B32A32_UINT;
   default:
      unreachable("Not a power-of-two format size");
   }
}

/**
 * Returns the greatest common divisor of a and b that is a power of two.
 */
static uint64_t
gcd_pow2_u64(uint64_t a, uint64_t b)
{
   assert(a > 0 || b > 0);

   unsigned a_log2 = ffsll(a) - 1;
   unsigned b_log2 = ffsll(b) - 1;

   /* If either a or b is 0, then a_log2 or b_log2 till be UINT_MAX in which
    * case, the MIN2() will take the other one.  If both are 0 then we will
    * hit the assert above.
    */
   return 1 << MIN2(a_log2, b_log2);
}

static void
do_buffer_copy(struct blorp_batch *batch,
               struct blorp_address *src,
               struct blorp_address *dst,
               int width, int height, int block_size)
{
   /* The actual format we pick doesn't matter as blorp will throw it away.
    * The only thing that actually matters is the size.
    */
   enum isl_format format = isl_format_for_size(block_size);

   UNUSED bool ok;
   struct isl_surf surf;
   ok = isl_surf_init(batch->blorp->isl_dev, &surf,
                      .dim = ISL_SURF_DIM_2D,
                      .format = format,
                      .width = width,
                      .height = height,
                      .depth = 1,
                      .levels = 1,
                      .array_len = 1,
                      .samples = 1,
                      .row_pitch_B = width * block_size,
                      .usage = ISL_SURF_USAGE_TEXTURE_BIT |
                               ISL_SURF_USAGE_RENDER_TARGET_BIT,
                      .tiling_flags = ISL_TILING_LINEAR_BIT);
   assert(ok);

   struct blorp_surf src_blorp_surf = {
      .surf = &surf,
      .addr = *src,
   };

   struct blorp_surf dst_blorp_surf = {
      .surf = &surf,
      .addr = *dst,
   };

   blorp_copy(batch, &src_blorp_surf, 0, 0, &dst_blorp_surf, 0, 0,
              0, 0, 0, 0, width, height);
}

void
blorp_buffer_copy(struct blorp_batch *batch,
                  struct blorp_address src,
                  struct blorp_address dst,
                  uint64_t size)
{
   const struct intel_device_info *devinfo = batch->blorp->isl_dev->info;
   uint64_t copy_size = size;

   /* This is maximum possible width/height our HW can handle */
   uint64_t max_surface_dim = 1 << (devinfo->ver >= 7 ? 14 : 13);

   /* First, we compute the biggest format that can be used with the
    * given offsets and size.
    */
   int bs = 16;
   bs = gcd_pow2_u64(bs, src.offset);
   bs = gcd_pow2_u64(bs, dst.offset);
   bs = gcd_pow2_u64(bs, size);

   /* First, we make a bunch of max-sized copies */
   uint64_t max_copy_size = max_surface_dim * max_surface_dim * bs;
   while (copy_size >= max_copy_size) {
      do_buffer_copy(batch, &src, &dst, max_surface_dim, max_surface_dim, bs);
      copy_size -= max_copy_size;
      src.offset += max_copy_size;
      dst.offset += max_copy_size;
   }

   /* Now make a max-width copy */
   uint64_t height = copy_size / (max_surface_dim * bs);
   assert(height < max_surface_dim);
   if (height != 0) {
      uint64_t rect_copy_size = height * max_surface_dim * bs;
      do_buffer_copy(batch, &src, &dst, max_surface_dim, height, bs);
      copy_size -= rect_copy_size;
      src.offset += rect_copy_size;
      dst.offset += rect_copy_size;
   }

   /* Finally, make a small copy to finish it off */
   if (copy_size != 0) {
      do_buffer_copy(batch, &src, &dst, copy_size / bs, 1, bs);
   }
}
