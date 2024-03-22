/*
 * Copyright © 2014-2015 Broadcom
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

struct alu_width_data {
   nir_vectorize_cb cb;
   const void *data;
};

/** @file nir_lower_alu_width.c
 *
 * Replaces nir_alu_instr operations with more than one channel used in the
 * arguments with individual per-channel operations.
 *
 * Optionally, a callback function which returns the max vectorization width
 * per instruction can be provided.
 *
 * The max vectorization width must be a power of 2.
 */

static bool
inst_is_vector_alu(const nir_instr *instr, const void *_state)
{
   if (instr->type != nir_instr_type_alu)
      return false;

   nir_alu_instr *alu = nir_instr_as_alu(instr);

   /* There is no ALU instruction which has a scalar destination, scalar
    * src[0], and some other vector source.
    */
   return alu->def.num_components > 1 ||
          nir_op_infos[alu->op].input_sizes[0] > 1;
}

/* Checks whether all operands of an ALU instruction are swizzled
 * within the targeted vectorization width.
 *
 * The assumption here is that a vecN instruction can only swizzle
 * within the first N channels of the values it consumes, irrespective
 * of the capabilities of the instruction which produced those values.
 * If we assume values are packed consistently (i.e., they always start
 * at the beginning of a hardware register), we can actually access any
 * aligned group of N channels so long as we stay within the group.
 * This means for a vectorization width of 4 that only swizzles from
 * either [xyzw] or [abcd] etc are allowed.  For a width of 2 these are
 * swizzles from either [xy] or [zw] etc.
 */
static bool
alu_is_swizzled_in_bounds(const nir_alu_instr *alu, unsigned width)
{
   for (unsigned i = 0; i < nir_op_infos[alu->op].num_inputs; i++) {
      if (nir_op_infos[alu->op].input_sizes[i] == 1)
         continue;

      unsigned mask = ~(width - 1);
      for (unsigned j = 1; j < alu->def.num_components; j++) {
         if ((alu->src[i].swizzle[0] & mask) != (alu->src[i].swizzle[j] & mask))
            return false;
      }
   }

   return true;
}

static void
nir_alu_ssa_dest_init(nir_alu_instr *alu, unsigned num_components,
                      unsigned bit_size)
{
   nir_def_init(&alu->instr, &alu->def, num_components, bit_size);
}

static nir_def *
lower_reduction(nir_alu_instr *alu, nir_op chan_op, nir_op merge_op,
                nir_builder *builder, bool reverse_order)
{
   unsigned num_components = nir_op_infos[alu->op].input_sizes[0];

   nir_def *last = NULL;
   for (int i = 0; i < num_components; i++) {
      int channel = reverse_order ? num_components - 1 - i : i;
      nir_alu_instr *chan = nir_alu_instr_create(builder->shader, chan_op);
      nir_alu_ssa_dest_init(chan, 1, alu->def.bit_size);
      nir_alu_src_copy(&chan->src[0], &alu->src[0]);
      chan->src[0].swizzle[0] = chan->src[0].swizzle[channel];
      if (nir_op_infos[chan_op].num_inputs > 1) {
         assert(nir_op_infos[chan_op].num_inputs == 2);
         nir_alu_src_copy(&chan->src[1], &alu->src[1]);
         chan->src[1].swizzle[0] = chan->src[1].swizzle[channel];
      }
      chan->exact = alu->exact;

      nir_builder_instr_insert(builder, &chan->instr);

      if (i == 0) {
         last = &chan->def;
      } else {
         last = nir_build_alu(builder, merge_op,
                              last, &chan->def, NULL, NULL);
      }
   }

   return last;
}

static inline bool
will_lower_ffma(nir_shader *shader, unsigned bit_size)
{
   switch (bit_size) {
   case 16:
      return shader->options->lower_ffma16;
   case 32:
      return shader->options->lower_ffma32;
   case 64:
      return shader->options->lower_ffma64;
   }
   unreachable("bad bit size");
}

static nir_def *
lower_fdot(nir_alu_instr *alu, nir_builder *builder)
{
   /* Reversed order can result in lower instruction count because it
    * creates more MAD/FMA in the case of fdot(a, vec4(b, 1.0)).
    * Some games expect xyzw order, so only reverse the order for imprecise fdot.
    */
   bool reverse_order = !builder->exact;

   /* If we don't want to lower ffma, create several ffma instead of fmul+fadd
    * and fusing later because fusing is not possible for exact fdot instructions.
    */
   if (will_lower_ffma(builder->shader, alu->def.bit_size))
      return lower_reduction(alu, nir_op_fmul, nir_op_fadd, builder, reverse_order);

   unsigned num_components = nir_op_infos[alu->op].input_sizes[0];

   nir_def *prev = NULL;
   for (int i = 0; i < num_components; i++) {
      int channel = reverse_order ? num_components - 1 - i : i;
      nir_alu_instr *instr = nir_alu_instr_create(
         builder->shader, prev ? nir_op_ffma : nir_op_fmul);
      nir_alu_ssa_dest_init(instr, 1, alu->def.bit_size);
      for (unsigned j = 0; j < 2; j++) {
         nir_alu_src_copy(&instr->src[j], &alu->src[j]);
         instr->src[j].swizzle[0] = alu->src[j].swizzle[channel];
      }
      if (i != 0)
         instr->src[2].src = nir_src_for_ssa(prev);
      instr->exact = builder->exact;

      nir_builder_instr_insert(builder, &instr->instr);

      prev = &instr->def;
   }

   return prev;
}

static nir_def *
lower_alu_instr_width(nir_builder *b, nir_instr *instr, void *_data)
{
   struct alu_width_data *data = _data;
   nir_alu_instr *alu = nir_instr_as_alu(instr);
   unsigned num_src = nir_op_infos[alu->op].num_inputs;
   unsigned i, chan;

   b->exact = alu->exact;

   unsigned num_components = alu->def.num_components;
   unsigned target_width = 1;

   if (data->cb) {
      target_width = data->cb(instr, data->data);
      assert(util_is_power_of_two_or_zero(target_width));
      if (target_width == 0)
         return NULL;
   }

#define LOWER_REDUCTION(name, chan, merge) \
   case name##2:                           \
   case name##3:                           \
   case name##4:                           \
   case name##8:                           \
   case name##16:                          \
      return lower_reduction(alu, chan, merge, b, true);

   switch (alu->op) {
   case nir_op_vec16:
   case nir_op_vec8:
   case nir_op_vec5:
   case nir_op_vec4:
   case nir_op_vec3:
   case nir_op_vec2:
   case nir_op_cube_amd:
      /* We don't need to scalarize these ops, they're the ones generated to
       * group up outputs into a value that can be SSAed.
       */
      return NULL;

   case nir_op_pack_half_2x16: {
      if (!b->shader->options->lower_pack_half_2x16)
         return NULL;

      nir_def *src_vec2 = nir_ssa_for_alu_src(b, alu, 0);
      return nir_pack_half_2x16_split(b, nir_channel(b, src_vec2, 0),
                                      nir_channel(b, src_vec2, 1));
   }

   case nir_op_unpack_unorm_4x8:
   case nir_op_unpack_snorm_4x8:
   case nir_op_unpack_unorm_2x16:
   case nir_op_unpack_snorm_2x16:
      /* There is no scalar version of these ops, unless we were to break it
       * down to bitshifts and math (which is definitely not intended).
       */
      return NULL;

   case nir_op_unpack_half_2x16_flush_to_zero:
   case nir_op_unpack_half_2x16: {
      if (!b->shader->options->lower_unpack_half_2x16)
         return NULL;

      nir_def *packed = nir_ssa_for_alu_src(b, alu, 0);
      if (alu->op == nir_op_unpack_half_2x16_flush_to_zero) {
         return nir_vec2(b,
                         nir_unpack_half_2x16_split_x_flush_to_zero(b,
                                                                    packed),
                         nir_unpack_half_2x16_split_y_flush_to_zero(b,
                                                                    packed));
      } else {
         return nir_vec2(b,
                         nir_unpack_half_2x16_split_x(b, packed),
                         nir_unpack_half_2x16_split_y(b, packed));
      }
   }

   case nir_op_pack_uvec2_to_uint: {
      assert(b->shader->options->lower_pack_snorm_2x16 ||
             b->shader->options->lower_pack_unorm_2x16);

      nir_def *word = nir_extract_u16(b, nir_ssa_for_alu_src(b, alu, 0),
                                      nir_imm_int(b, 0));
      return nir_ior(b, nir_ishl(b, nir_channel(b, word, 1), nir_imm_int(b, 16)),
                     nir_channel(b, word, 0));
   }

   case nir_op_pack_uvec4_to_uint: {
      assert(b->shader->options->lower_pack_snorm_4x8 ||
             b->shader->options->lower_pack_unorm_4x8);

      nir_def *byte = nir_extract_u8(b, nir_ssa_for_alu_src(b, alu, 0),
                                     nir_imm_int(b, 0));
      return nir_ior(b, nir_ior(b, nir_ishl(b, nir_channel(b, byte, 3), nir_imm_int(b, 24)), nir_ishl(b, nir_channel(b, byte, 2), nir_imm_int(b, 16))),
                     nir_ior(b, nir_ishl(b, nir_channel(b, byte, 1), nir_imm_int(b, 8)),
                             nir_channel(b, byte, 0)));
   }

   case nir_op_fdph: {
      nir_def *src0_vec = nir_ssa_for_alu_src(b, alu, 0);
      nir_def *src1_vec = nir_ssa_for_alu_src(b, alu, 1);

      /* Only use reverse order for imprecise fdph, see explanation in lower_fdot. */
      bool reverse_order = !b->exact;
      if (will_lower_ffma(b->shader, alu->def.bit_size)) {
         nir_def *sum[4];
         for (unsigned i = 0; i < 3; i++) {
            int dest = reverse_order ? 3 - i : i;
            sum[dest] = nir_fmul(b, nir_channel(b, src0_vec, i),
                                 nir_channel(b, src1_vec, i));
         }
         sum[reverse_order ? 0 : 3] = nir_channel(b, src1_vec, 3);

         return nir_fadd(b, nir_fadd(b, nir_fadd(b, sum[0], sum[1]), sum[2]), sum[3]);
      } else if (reverse_order) {
         nir_def *sum = nir_channel(b, src1_vec, 3);
         for (int i = 2; i >= 0; i--)
            sum = nir_ffma(b, nir_channel(b, src0_vec, i), nir_channel(b, src1_vec, i), sum);
         return sum;
      } else {
         nir_def *sum = nir_fmul(b, nir_channel(b, src0_vec, 0), nir_channel(b, src1_vec, 0));
         sum = nir_ffma(b, nir_channel(b, src0_vec, 1), nir_channel(b, src1_vec, 1), sum);
         sum = nir_ffma(b, nir_channel(b, src0_vec, 2), nir_channel(b, src1_vec, 2), sum);
         return nir_fadd(b, sum, nir_channel(b, src1_vec, 3));
      }
   }

   case nir_op_pack_64_2x32: {
      if (!b->shader->options->lower_pack_64_2x32)
         return NULL;

      nir_def *src_vec2 = nir_ssa_for_alu_src(b, alu, 0);
      return nir_pack_64_2x32_split(b, nir_channel(b, src_vec2, 0),
                                    nir_channel(b, src_vec2, 1));
   }
   case nir_op_pack_64_4x16: {
      if (!b->shader->options->lower_pack_64_4x16)
         return NULL;

      nir_def *src_vec4 = nir_ssa_for_alu_src(b, alu, 0);
      nir_def *xy = nir_pack_32_2x16_split(b, nir_channel(b, src_vec4, 0),
                                           nir_channel(b, src_vec4, 1));
      nir_def *zw = nir_pack_32_2x16_split(b, nir_channel(b, src_vec4, 2),
                                           nir_channel(b, src_vec4, 3));

      return nir_pack_64_2x32_split(b, xy, zw);
   }
   case nir_op_pack_32_2x16: {
      if (!b->shader->options->lower_pack_32_2x16)
         return NULL;

      nir_def *src_vec2 = nir_ssa_for_alu_src(b, alu, 0);
      return nir_pack_32_2x16_split(b, nir_channel(b, src_vec2, 0),
                                    nir_channel(b, src_vec2, 1));
   }
   case nir_op_unpack_64_2x32:
   case nir_op_unpack_64_4x16:
   case nir_op_unpack_32_2x16:
   case nir_op_unpack_32_4x8:
   case nir_op_unpack_double_2x32_dxil:
      return NULL;

   case nir_op_fdot2:
   case nir_op_fdot3:
   case nir_op_fdot4:
   case nir_op_fdot8:
   case nir_op_fdot16:
      return lower_fdot(alu, b);

      LOWER_REDUCTION(nir_op_ball_fequal, nir_op_feq, nir_op_iand);
      LOWER_REDUCTION(nir_op_ball_iequal, nir_op_ieq, nir_op_iand);
      LOWER_REDUCTION(nir_op_bany_fnequal, nir_op_fneu, nir_op_ior);
      LOWER_REDUCTION(nir_op_bany_inequal, nir_op_ine, nir_op_ior);
      LOWER_REDUCTION(nir_op_b8all_fequal, nir_op_feq8, nir_op_iand);
      LOWER_REDUCTION(nir_op_b8all_iequal, nir_op_ieq8, nir_op_iand);
      LOWER_REDUCTION(nir_op_b8any_fnequal, nir_op_fneu8, nir_op_ior);
      LOWER_REDUCTION(nir_op_b8any_inequal, nir_op_ine8, nir_op_ior);
      LOWER_REDUCTION(nir_op_b16all_fequal, nir_op_feq16, nir_op_iand);
      LOWER_REDUCTION(nir_op_b16all_iequal, nir_op_ieq16, nir_op_iand);
      LOWER_REDUCTION(nir_op_b16any_fnequal, nir_op_fneu16, nir_op_ior);
      LOWER_REDUCTION(nir_op_b16any_inequal, nir_op_ine16, nir_op_ior);
      LOWER_REDUCTION(nir_op_b32all_fequal, nir_op_feq32, nir_op_iand);
      LOWER_REDUCTION(nir_op_b32all_iequal, nir_op_ieq32, nir_op_iand);
      LOWER_REDUCTION(nir_op_b32any_fnequal, nir_op_fneu32, nir_op_ior);
      LOWER_REDUCTION(nir_op_b32any_inequal, nir_op_ine32, nir_op_ior);
      LOWER_REDUCTION(nir_op_fall_equal, nir_op_seq, nir_op_fmin);
      LOWER_REDUCTION(nir_op_fany_nequal, nir_op_sne, nir_op_fmax);

   default:
      break;
   }

   if (num_components == 1)
      return NULL;

   if (num_components <= target_width) {
      /* If the ALU instr is swizzled outside the target width,
       * reduce the target width.
       */
      if (alu_is_swizzled_in_bounds(alu, target_width))
         return NULL;
      else
         target_width = DIV_ROUND_UP(num_components, 2);
   }

   nir_alu_instr *vec = nir_alu_instr_create(b->shader, nir_op_vec(num_components));

   for (chan = 0; chan < num_components; chan += target_width) {
      unsigned components = MIN2(target_width, num_components - chan);
      nir_alu_instr *lower = nir_alu_instr_create(b->shader, alu->op);

      for (i = 0; i < num_src; i++) {
         nir_alu_src_copy(&lower->src[i], &alu->src[i]);

         /* We only handle same-size-as-dest (input_sizes[] == 0) or scalar
          * args (input_sizes[] == 1).
          */
         assert(nir_op_infos[alu->op].input_sizes[i] < 2);
         for (int j = 0; j < components; j++) {
            unsigned src_chan = nir_op_infos[alu->op].input_sizes[i] == 1 ? 0 : chan + j;
            lower->src[i].swizzle[j] = alu->src[i].swizzle[src_chan];
         }
      }

      nir_alu_ssa_dest_init(lower, components, alu->def.bit_size);
      lower->exact = alu->exact;

      for (i = 0; i < components; i++) {
         vec->src[chan + i].src = nir_src_for_ssa(&lower->def);
         vec->src[chan + i].swizzle[0] = i;
      }

      nir_builder_instr_insert(b, &lower->instr);
   }

   return nir_builder_alu_instr_finish_and_insert(b, vec);
}

bool
nir_lower_alu_width(nir_shader *shader, nir_vectorize_cb cb, const void *_data)
{
   struct alu_width_data data = {
      .cb = cb,
      .data = _data,
   };

   return nir_shader_lower_instructions(shader,
                                        inst_is_vector_alu,
                                        lower_alu_instr_width,
                                        &data);
}

struct alu_to_scalar_data {
   nir_instr_filter_cb cb;
   const void *data;
};

static uint8_t
scalar_cb(const nir_instr *instr, const void *data)
{
   /* return vectorization-width = 1 for filtered instructions */
   const struct alu_to_scalar_data *filter = data;
   return filter->cb(instr, filter->data) ? 1 : 0;
}

bool
nir_lower_alu_to_scalar(nir_shader *shader, nir_instr_filter_cb cb, const void *_data)
{
   struct alu_to_scalar_data data = {
      .cb = cb,
      .data = _data,
   };

   return nir_lower_alu_width(shader, cb ? scalar_cb : NULL, &data);
}

static bool
lower_alu_vec8_16_src(nir_builder *b, nir_instr *instr, void *_data)
{
   if (instr->type != nir_instr_type_alu)
      return false;

   nir_alu_instr *alu = nir_instr_as_alu(instr);
   const nir_op_info *info = &nir_op_infos[alu->op];

   bool changed = false;
   b->cursor = nir_before_instr(instr);
   for (int i = 0; i < info->num_inputs; i++) {
      if (alu->src[i].src.ssa->num_components < 8 || info->input_sizes[i])
         continue;

      changed = true;
      nir_def *comps[4];
      for (int c = 0; c < alu->def.num_components; c++) {
         unsigned swizzle = alu->src[i].swizzle[c];
         alu->src[i].swizzle[c] = c;

         nir_const_value *const_val = nir_src_as_const_value(alu->src[i].src);
         if (const_val) {
            comps[c] = nir_build_imm(b, 1, alu->src[i].src.ssa->bit_size, &const_val[swizzle]);
         } else {
            comps[c] = nir_swizzle(b, alu->src[i].src.ssa, &swizzle, 1);
         }
      }
      nir_def *src = nir_vec(b, comps, alu->def.num_components);
      nir_src_rewrite(&alu->src[i].src, src);
   }

   return changed;
}

bool
nir_lower_alu_vec8_16_srcs(nir_shader *shader)
{
   return nir_shader_instructions_pass(shader, lower_alu_vec8_16_src,
      nir_metadata_block_index | nir_metadata_dominance,
      NULL);
}
