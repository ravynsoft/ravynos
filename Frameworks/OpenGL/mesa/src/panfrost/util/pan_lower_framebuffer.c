/*
 * Copyright (C) 2020 Collabora, Ltd.
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
 *
 * Authors (Collabora):
 *      Alyssa Rosenzweig <alyssa.rosenzweig@collabora.com>
 */

/**
 * Implements framebuffer format conversions in software for Midgard/Bifrost
 * blend shaders. This pass is designed for a single render target; Midgard
 * duplicates blend shaders for MRT to simplify everything. A particular
 * framebuffer format may be categorized as 1) typed load available, 2) typed
 * unpack available, or 3) software unpack only, and likewise for stores. The
 * first two types are handled in the compiler backend directly, so this module
 * is responsible for identifying type 3 formats (hardware dependent) and
 * inserting appropriate ALU code to perform the conversion from the packed
 * type to a designated unpacked type, and vice versa.
 *
 * The unpacked type depends on the format:
 *
 *      - For 32-bit float formats or >8-bit UNORM, 32-bit floats.
 *      - For other floats, 16-bit floats.
 *      - For 32-bit ints, 32-bit ints.
 *      - For 8-bit ints, 8-bit ints.
 *      - For other ints, 16-bit ints.
 *
 * The rationale is to optimize blending and logic op instructions by using the
 * smallest precision necessary to store the pixel losslessly.
 */

#include "pan_lower_framebuffer.h"
#include "compiler/nir/nir.h"
#include "compiler/nir/nir_builder.h"
#include "compiler/nir/nir_format_convert.h"
#include "util/format/u_format.h"

/* Determines the unpacked type best suiting a given format, so the rest of the
 * pipeline may be adjusted accordingly */

nir_alu_type
pan_unpacked_type_for_format(const struct util_format_description *desc)
{
   int c = util_format_get_first_non_void_channel(desc->format);

   if (c == -1)
      unreachable("Void format not renderable");

   bool large = (desc->channel[c].size > 16);
   bool large_norm = (desc->channel[c].size > 8);
   bool bit8 = (desc->channel[c].size == 8);
   assert(desc->channel[c].size <= 32);

   if (desc->channel[c].normalized)
      return large_norm ? nir_type_float32 : nir_type_float16;

   switch (desc->channel[c].type) {
   case UTIL_FORMAT_TYPE_UNSIGNED:
      return bit8 ? nir_type_uint8 : large ? nir_type_uint32 : nir_type_uint16;
   case UTIL_FORMAT_TYPE_SIGNED:
      return bit8 ? nir_type_int8 : large ? nir_type_int32 : nir_type_int16;
   case UTIL_FORMAT_TYPE_FLOAT:
      return large ? nir_type_float32 : nir_type_float16;
   default:
      unreachable("Format not renderable");
   }
}

static bool
pan_is_format_native(const struct util_format_description *desc,
                     bool broken_ld_special, bool is_store)
{
   if (is_store || broken_ld_special)
      return false;

   if (util_format_is_pure_integer(desc->format) ||
       util_format_is_float(desc->format))
      return false;

   /* Some formats are missing as typed but have unpacks */
   if (desc->format == PIPE_FORMAT_R11G11B10_FLOAT)
      return false;

   if (desc->is_array) {
      int c = util_format_get_first_non_void_channel(desc->format);
      assert(c >= 0);
      if (desc->channel[c].size > 8)
         return false;
   }

   return true;
}

/* Software packs/unpacks, by format class. Packs take in the pixel value typed
 * as `pan_unpacked_type_for_format` of the format and return an i32vec4
 * suitable for storing (with components replicated to fill). Unpacks do the
 * reverse but cannot rely on replication. */

static nir_def *
pan_replicate(nir_builder *b, nir_def *v, unsigned num_components)
{
   nir_def *replicated[4];

   for (unsigned i = 0; i < 4; ++i)
      replicated[i] = nir_channel(b, v, i % num_components);

   return nir_vec(b, replicated, 4);
}

/* Pure x16 formats are x16 unpacked, so it's similar, but we need to pack
 * upper/lower halves of course */

static nir_def *
pan_pack_pure_16(nir_builder *b, nir_def *v, unsigned num_components)
{
   nir_def *v4 = pan_replicate(b, v, num_components);

   nir_def *lo = nir_pack_32_2x16(b, nir_channels(b, v4, 0x3 << 0));
   nir_def *hi = nir_pack_32_2x16(b, nir_channels(b, v4, 0x3 << 2));

   return nir_vec4(b, lo, hi, lo, hi);
}

static nir_def *
pan_unpack_pure_16(nir_builder *b, nir_def *pack, unsigned num_components)
{
   nir_def *unpacked[4];

   assert(num_components <= 4);

   for (unsigned i = 0; i < num_components; i += 2) {
      nir_def *halves = nir_unpack_32_2x16(b, nir_channel(b, pack, i >> 1));

      unpacked[i + 0] = nir_channel(b, halves, 0);
      unpacked[i + 1] = nir_channel(b, halves, 1);
   }

   return nir_pad_vec4(b, nir_vec(b, unpacked, num_components));
}

static nir_def *
pan_pack_reorder(nir_builder *b, const struct util_format_description *desc,
                 nir_def *v)
{
   unsigned swizzle[4] = {0, 1, 2, 3};

   for (unsigned i = 0; i < v->num_components; i++) {
      if (desc->swizzle[i] <= PIPE_SWIZZLE_W)
         swizzle[i] = desc->swizzle[i];
   }

   return nir_swizzle(b, v, swizzle, v->num_components);
}

static nir_def *
pan_unpack_reorder(nir_builder *b, const struct util_format_description *desc,
                   nir_def *v)
{
   unsigned swizzle[4] = {0, 1, 2, 3};

   for (unsigned i = 0; i < v->num_components; i++) {
      if (desc->swizzle[i] <= PIPE_SWIZZLE_W)
         swizzle[desc->swizzle[i]] = i;
   }

   return nir_swizzle(b, v, swizzle, v->num_components);
}

static nir_def *
pan_pack_pure_8(nir_builder *b, nir_def *v, unsigned num_components)
{
   return nir_replicate(
      b, nir_pack_32_4x8(b, pan_replicate(b, v, num_components)), 4);
}

static nir_def *
pan_unpack_pure_8(nir_builder *b, nir_def *pack, unsigned num_components)
{
   nir_def *unpacked = nir_unpack_32_4x8(b, nir_channel(b, pack, 0));
   return nir_trim_vector(b, unpacked, num_components);
}

static nir_def *
pan_fsat(nir_builder *b, nir_def *v, bool is_signed)
{
   if (is_signed)
      return nir_fsat_signed_mali(b, v);
   else
      return nir_fsat(b, v);
}

static float
norm_scale(bool snorm, unsigned bits)
{
   if (snorm)
      return (1 << (bits - 1)) - 1;
   else
      return (1 << bits) - 1;
}

/* For <= 8-bits per channel, [U,S]NORM formats are packed like [U,S]NORM 8,
 * with zeroes spacing out each component as needed */

static nir_def *
pan_pack_norm(nir_builder *b, nir_def *v, unsigned x, unsigned y, unsigned z,
              unsigned w, bool is_signed)
{
   /* If a channel has N bits, 1.0 is encoded as 2^N - 1 for UNORMs and
    * 2^(N-1) - 1 for SNORMs */
   nir_def *scales =
      is_signed ? nir_imm_vec4_16(b, (1 << (x - 1)) - 1, (1 << (y - 1)) - 1,
                                  (1 << (z - 1)) - 1, (1 << (w - 1)) - 1)
                : nir_imm_vec4_16(b, (1 << x) - 1, (1 << y) - 1, (1 << z) - 1,
                                  (1 << w) - 1);

   /* If a channel has N bits, we pad out to the byte by (8 - N) bits */
   nir_def *shifts = nir_imm_ivec4(b, 8 - x, 8 - y, 8 - z, 8 - w);
   nir_def *clamped = pan_fsat(b, nir_pad_vec4(b, v), is_signed);

   nir_def *f = nir_fmul(b, clamped, scales);
   nir_def *u8 = nir_f2u8(b, nir_fround_even(b, f));
   nir_def *s = nir_ishl(b, u8, shifts);
   nir_def *repl = nir_pack_32_4x8(b, s);

   return nir_replicate(b, repl, 4);
}

static nir_def *
pan_pack_unorm(nir_builder *b, nir_def *v, unsigned x, unsigned y, unsigned z,
               unsigned w)
{
   return pan_pack_norm(b, v, x, y, z, w, false);
}

/* RGB10_A2 is packed in the tilebuffer as the bottom 3 bytes being the top
 * 8-bits of RGB and the top byte being RGBA as 2-bits packed. As imirkin
 * pointed out, this means free conversion to RGBX8 */

static nir_def *
pan_pack_unorm_1010102(nir_builder *b, nir_def *v)
{
   nir_def *scale = nir_imm_vec4(b, 1023.0, 1023.0, 1023.0, 3.0);
   nir_def *s =
      nir_f2u32(b, nir_fround_even(b, nir_fmul(b, nir_fsat(b, v), scale)));

   nir_def *top8 = nir_ushr(b, s, nir_imm_ivec4(b, 0x2, 0x2, 0x2, 0x2));
   nir_def *top8_rgb = nir_pack_32_4x8(b, nir_u2u8(b, top8));

   nir_def *bottom2 = nir_iand(b, s, nir_imm_ivec4(b, 0x3, 0x3, 0x3, 0x3));

   nir_def *top =
      nir_ior(b,
              nir_ior(b, nir_ishl_imm(b, nir_channel(b, bottom2, 0), 24 + 0),
                      nir_ishl_imm(b, nir_channel(b, bottom2, 1), 24 + 2)),
              nir_ior(b, nir_ishl_imm(b, nir_channel(b, bottom2, 2), 24 + 4),
                      nir_ishl_imm(b, nir_channel(b, bottom2, 3), 24 + 6)));

   nir_def *p = nir_ior(b, top, top8_rgb);
   return nir_replicate(b, p, 4);
}

/* On the other hand, the pure int RGB10_A2 is identical to the spec */

static nir_def *
pan_pack_int_1010102(nir_builder *b, nir_def *v, bool is_signed)
{
   v = nir_u2u32(b, v);

   /* Clamp the values */
   if (is_signed) {
      v = nir_imin(b, v, nir_imm_ivec4(b, 511, 511, 511, 1));
      v = nir_imax(b, v, nir_imm_ivec4(b, -512, -512, -512, -2));
   } else {
      v = nir_umin(b, v, nir_imm_ivec4(b, 1023, 1023, 1023, 3));
   }

   v = nir_ishl(b, v, nir_imm_ivec4(b, 0, 10, 20, 30));
   v = nir_ior(b, nir_ior(b, nir_channel(b, v, 0), nir_channel(b, v, 1)),
               nir_ior(b, nir_channel(b, v, 2), nir_channel(b, v, 3)));

   return nir_replicate(b, v, 4);
}

static nir_def *
pan_unpack_int_1010102(nir_builder *b, nir_def *packed, bool is_signed)
{
   nir_def *v = nir_replicate(b, nir_channel(b, packed, 0), 4);

   /* Left shift all components so the sign bit is on the MSB, and
    * can be extended by ishr(). The ishl()+[u,i]shr() combination
    * sets all unused bits to 0 without requiring a mask.
    */
   v = nir_ishl(b, v, nir_imm_ivec4(b, 22, 12, 2, 0));

   if (is_signed)
      v = nir_ishr(b, v, nir_imm_ivec4(b, 22, 22, 22, 30));
   else
      v = nir_ushr(b, v, nir_imm_ivec4(b, 22, 22, 22, 30));

   return nir_i2i16(b, v);
}

/* NIR means we can *finally* catch a break */

static nir_def *
pan_pack_r11g11b10(nir_builder *b, nir_def *v)
{
   return nir_replicate(b, nir_format_pack_11f11f10f(b, nir_f2f32(b, v)), 4);
}

static nir_def *
pan_unpack_r11g11b10(nir_builder *b, nir_def *v)
{
   nir_def *f32 = nir_format_unpack_11f11f10f(b, nir_channel(b, v, 0));
   nir_def *f16 = nir_f2fmp(b, f32);

   /* Extend to vec4 with alpha */
   nir_def *components[4] = {nir_channel(b, f16, 0), nir_channel(b, f16, 1),
                             nir_channel(b, f16, 2), nir_imm_float16(b, 1.0)};

   return nir_vec(b, components, 4);
}

/* Wrapper around sRGB conversion */

static nir_def *
pan_linear_to_srgb(nir_builder *b, nir_def *linear)
{
   nir_def *rgb = nir_trim_vector(b, linear, 3);

   /* TODO: fp16 native conversion */
   nir_def *srgb =
      nir_f2fmp(b, nir_format_linear_to_srgb(b, nir_f2f32(b, rgb)));

   nir_def *comp[4] = {
      nir_channel(b, srgb, 0),
      nir_channel(b, srgb, 1),
      nir_channel(b, srgb, 2),
      nir_channel(b, linear, 3),
   };

   return nir_vec(b, comp, 4);
}

static nir_def *
pan_unpack_pure(nir_builder *b, nir_def *packed, unsigned size, unsigned nr)
{
   switch (size) {
   case 32:
      return nir_trim_vector(b, packed, nr);
   case 16:
      return pan_unpack_pure_16(b, packed, nr);
   case 8:
      return pan_unpack_pure_8(b, packed, nr);
   default:
      unreachable("Unrenderable size");
   }
}

/* Generic dispatches for un/pack regardless of format */

static nir_def *
pan_unpack(nir_builder *b, const struct util_format_description *desc,
           nir_def *packed)
{
   if (desc->is_array) {
      int c = util_format_get_first_non_void_channel(desc->format);
      assert(c >= 0);
      struct util_format_channel_description d = desc->channel[c];
      nir_def *unpacked = pan_unpack_pure(b, packed, d.size, desc->nr_channels);

      /* Normalized formats are unpacked as integers. We need to
       * convert to float for the final result.
       */
      if (d.normalized) {
         bool snorm = desc->is_snorm;
         unsigned float_sz = (d.size <= 8 ? 16 : 32);
         float multiplier = norm_scale(snorm, d.size);

         nir_def *as_float = snorm ? nir_i2fN(b, unpacked, float_sz)
                                   : nir_u2fN(b, unpacked, float_sz);

         return nir_fmul_imm(b, as_float, 1.0 / multiplier);
      } else {
         return unpacked;
      }
   }

   switch (desc->format) {
   case PIPE_FORMAT_R10G10B10A2_UINT:
   case PIPE_FORMAT_B10G10R10A2_UINT:
      return pan_unpack_int_1010102(b, packed, false);
   case PIPE_FORMAT_R10G10B10A2_SINT:
   case PIPE_FORMAT_B10G10R10A2_SINT:
      return pan_unpack_int_1010102(b, packed, true);
   case PIPE_FORMAT_R11G11B10_FLOAT:
      return pan_unpack_r11g11b10(b, packed);
   default:
      break;
   }

   fprintf(stderr, "%s\n", desc->name);
   unreachable("Unknown format");
}

static nir_def *pan_pack(nir_builder *b,
                         const struct util_format_description *desc,
                         nir_def * unpacked)
{
   if (desc->colorspace == UTIL_FORMAT_COLORSPACE_SRGB)
      unpacked = pan_linear_to_srgb(b, unpacked);

   if (desc->is_array) {
      int c = util_format_get_first_non_void_channel(desc->format);
      assert(c >= 0);
      struct util_format_channel_description d = desc->channel[c];

      /* Pure formats are packed as-is */
      nir_def *raw = unpacked;

      /* Normalized formats get normalized first */
      if (d.normalized) {
         bool snorm = desc->is_snorm;
         float multiplier = norm_scale(snorm, d.size);
         nir_def *clamped = pan_fsat(b, unpacked, snorm);
         nir_def *normed = nir_fmul_imm(b, clamped, multiplier);

         raw = nir_f2uN(b, normed, d.size);
      }

      /* Pack the raw format */
      switch (d.size) {
      case 32:
         return pan_replicate(b, raw, desc->nr_channels);
      case 16:
         return pan_pack_pure_16(b, raw, desc->nr_channels);
      case 8:
         return pan_pack_pure_8(b, raw, desc->nr_channels);
      default:
         unreachable("Unrenderable size");
      }
   }

   switch (desc->format) {
   case PIPE_FORMAT_B4G4R4A4_UNORM:
   case PIPE_FORMAT_B4G4R4X4_UNORM:
   case PIPE_FORMAT_A4R4_UNORM:
   case PIPE_FORMAT_R4A4_UNORM:
   case PIPE_FORMAT_A4B4G4R4_UNORM:
   case PIPE_FORMAT_R4G4B4A4_UNORM:
      return pan_pack_unorm(b, unpacked, 4, 4, 4, 4);
   case PIPE_FORMAT_B5G5R5A1_UNORM:
   case PIPE_FORMAT_R5G5B5A1_UNORM:
      return pan_pack_unorm(b, unpacked, 5, 6, 5, 1);
   case PIPE_FORMAT_R5G6B5_UNORM:
   case PIPE_FORMAT_B5G6R5_UNORM:
      return pan_pack_unorm(b, unpacked, 5, 6, 5, 0);
   case PIPE_FORMAT_R10G10B10A2_UNORM:
   case PIPE_FORMAT_B10G10R10A2_UNORM:
      return pan_pack_unorm_1010102(b, unpacked);
   case PIPE_FORMAT_R10G10B10A2_UINT:
   case PIPE_FORMAT_B10G10R10A2_UINT:
      return pan_pack_int_1010102(b, unpacked, false);
   case PIPE_FORMAT_R10G10B10A2_SINT:
   case PIPE_FORMAT_B10G10R10A2_SINT:
      return pan_pack_int_1010102(b, unpacked, true);
   case PIPE_FORMAT_R11G11B10_FLOAT:
      return pan_pack_r11g11b10(b, unpacked);
   default:
      break;
   }

   fprintf(stderr, "%s\n", desc->name);
   unreachable("Unknown format");
}

static void
pan_lower_fb_store(nir_builder *b, nir_intrinsic_instr *intr,
                   const struct util_format_description *desc,
                   bool reorder_comps, unsigned nr_samples)
{
   /* For stores, add conversion before */
   nir_def *unpacked = intr->src[0].ssa;
   unpacked = nir_pad_vec4(b, unpacked);

   /* Re-order the components */
   if (reorder_comps)
      unpacked = pan_pack_reorder(b, desc, unpacked);

   nir_def *packed = pan_pack(b, desc, unpacked);

   /* We have to split writeout in 128 bit chunks */
   unsigned iterations = DIV_ROUND_UP(desc->block.bits * nr_samples, 128);

   for (unsigned s = 0; s < iterations; ++s) {
      nir_store_raw_output_pan(b, packed,
                               .io_semantics = nir_intrinsic_io_semantics(intr),
                               .base = s);
   }
}

static nir_def *
pan_sample_id(nir_builder *b, int sample)
{
   return (sample >= 0) ? nir_imm_int(b, sample) : nir_load_sample_id(b);
}

static void
pan_lower_fb_load(nir_builder *b, nir_intrinsic_instr *intr,
                  const struct util_format_description *desc,
                  bool reorder_comps, int sample)
{
   nir_def *packed =
      nir_load_raw_output_pan(b, 4, 32, pan_sample_id(b, sample),
                              .io_semantics = nir_intrinsic_io_semantics(intr));

   /* Convert the raw value */
   nir_def *unpacked = pan_unpack(b, desc, packed);

   /* Convert to the size of the load intrinsic.
    *
    * We can assume that the type will match with the framebuffer format:
    *
    * Page 170 of the PDF of the OpenGL ES 3.0.6 spec says:
    *
    * If [UNORM or SNORM, convert to fixed-point]; otherwise no type
    * conversion is applied. If the values written by the fragment shader
    * do not match the format(s) of the corresponding color buffer(s),
    * the result is undefined.
    */

   unsigned bits = intr->def.bit_size;

   nir_alu_type src_type =
      nir_alu_type_get_base_type(pan_unpacked_type_for_format(desc));

   unpacked = nir_convert_to_bit_size(b, unpacked, src_type, bits);
   unpacked = nir_resize_vector(b, unpacked, intr->def.num_components);

   /* Reorder the components */
   if (reorder_comps)
      unpacked = pan_unpack_reorder(b, desc, unpacked);

   nir_def_rewrite_uses_after(&intr->def, unpacked, &intr->instr);
}

struct inputs {
   const enum pipe_format *rt_fmts;
   uint8_t raw_fmt_mask;
   bool is_blend;
   bool broken_ld_special;
   unsigned nr_samples;
};

static bool
lower(nir_builder *b, nir_instr *instr, void *data)
{
   struct inputs *inputs = data;
   if (instr->type != nir_instr_type_intrinsic)
      return false;

   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
   bool is_load = intr->intrinsic == nir_intrinsic_load_output;
   bool is_store = intr->intrinsic == nir_intrinsic_store_output;

   if (!(is_load || (is_store && inputs->is_blend)))
      return false;

   nir_io_semantics sem = nir_intrinsic_io_semantics(intr);
   if (sem.location < FRAG_RESULT_DATA0)
      return false;

   unsigned rt = sem.location - FRAG_RESULT_DATA0;
   if (inputs->rt_fmts[rt] == PIPE_FORMAT_NONE)
      return false;

   const struct util_format_description *desc =
      util_format_description(inputs->rt_fmts[rt]);

   /* Don't lower */
   if (pan_is_format_native(desc, inputs->broken_ld_special, is_store))
      return false;

   /* EXT_shader_framebuffer_fetch requires per-sample loads. MSAA blend
    * shaders are not yet handled, so for now always load sample 0.
    */
   int sample = inputs->is_blend ? 0 : -1;
   bool reorder_comps = inputs->raw_fmt_mask & BITFIELD_BIT(rt);

   if (is_store) {
      b->cursor = nir_before_instr(instr);
      pan_lower_fb_store(b, intr, desc, reorder_comps, inputs->nr_samples);
   } else {
      b->cursor = nir_after_instr(instr);
      pan_lower_fb_load(b, intr, desc, reorder_comps, sample);
   }

   nir_instr_remove(instr);
   return true;
}

bool
pan_lower_framebuffer(nir_shader *shader, const enum pipe_format *rt_fmts,
                      uint8_t raw_fmt_mask, unsigned blend_shader_nr_samples,
                      bool broken_ld_special)
{
   assert(shader->info.stage == MESA_SHADER_FRAGMENT);

   return nir_shader_instructions_pass(
      shader, lower, nir_metadata_block_index | nir_metadata_dominance,
      &(struct inputs){
         .rt_fmts = rt_fmts,
         .raw_fmt_mask = raw_fmt_mask,
         .nr_samples = blend_shader_nr_samples,
         .is_blend = blend_shader_nr_samples > 0,
         .broken_ld_special = broken_ld_special,
      });
}
