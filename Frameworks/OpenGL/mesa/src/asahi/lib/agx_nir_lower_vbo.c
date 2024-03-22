/*
 * Copyright 2022 Alyssa Rosenzweig
 * SPDX-License-Identifier: MIT
 */

#include "agx_nir_lower_vbo.h"
#include "asahi/compiler/agx_internal_formats.h"
#include "compiler/nir/nir_builder.h"
#include "compiler/nir/nir_format_convert.h"
#include "util/bitset.h"
#include "util/u_math.h"
#include "shader_enums.h"

static bool
is_rgb10_a2(const struct util_format_description *desc)
{
   return desc->channel[0].shift == 0 && desc->channel[0].size == 10 &&
          desc->channel[1].shift == 10 && desc->channel[1].size == 10 &&
          desc->channel[2].shift == 20 && desc->channel[2].size == 10 &&
          desc->channel[3].shift == 30 && desc->channel[3].size == 2;
}

static enum pipe_format
agx_vbo_internal_format(enum pipe_format format)
{
   const struct util_format_description *desc = util_format_description(format);

   /* RGB10A2 formats are native for UNORM and unpacked otherwise */
   if (is_rgb10_a2(desc)) {
      if (desc->is_unorm)
         return PIPE_FORMAT_R10G10B10A2_UNORM;
      else
         return PIPE_FORMAT_R32_UINT;
   }

   /* R11G11B10F is native and special */
   if (format == PIPE_FORMAT_R11G11B10_FLOAT)
      return format;

   /* No other non-array formats handled */
   if (!desc->is_array)
      return PIPE_FORMAT_NONE;

   /* Otherwise look at one (any) channel */
   int idx = util_format_get_first_non_void_channel(format);
   if (idx < 0)
      return PIPE_FORMAT_NONE;

   /* We only handle RGB formats (we could do SRGB if we wanted though?) */
   if ((desc->colorspace != UTIL_FORMAT_COLORSPACE_RGB) ||
       (desc->layout != UTIL_FORMAT_LAYOUT_PLAIN))
      return PIPE_FORMAT_NONE;

   /* We have native 8-bit and 16-bit normalized formats */
   struct util_format_channel_description chan = desc->channel[idx];

   if (chan.normalized) {
      if (chan.size == 8)
         return desc->is_unorm ? PIPE_FORMAT_R8_UNORM : PIPE_FORMAT_R8_SNORM;
      else if (chan.size == 16)
         return desc->is_unorm ? PIPE_FORMAT_R16_UNORM : PIPE_FORMAT_R16_SNORM;
   }

   /* Otherwise map to the corresponding integer format */
   switch (chan.size) {
   case 32:
      return PIPE_FORMAT_R32_UINT;
   case 16:
      return PIPE_FORMAT_R16_UINT;
   case 8:
      return PIPE_FORMAT_R8_UINT;
   default:
      return PIPE_FORMAT_NONE;
   }
}

bool
agx_vbo_supports_format(enum pipe_format format)
{
   return agx_vbo_internal_format(format) != PIPE_FORMAT_NONE;
}

static nir_def *
apply_swizzle_channel(nir_builder *b, nir_def *vec, unsigned swizzle,
                      bool is_int)
{
   switch (swizzle) {
   case PIPE_SWIZZLE_X:
      return nir_channel(b, vec, 0);
   case PIPE_SWIZZLE_Y:
      return nir_channel(b, vec, 1);
   case PIPE_SWIZZLE_Z:
      return nir_channel(b, vec, 2);
   case PIPE_SWIZZLE_W:
      return nir_channel(b, vec, 3);
   case PIPE_SWIZZLE_0:
      return nir_imm_intN_t(b, 0, vec->bit_size);
   case PIPE_SWIZZLE_1:
      return is_int ? nir_imm_intN_t(b, 1, vec->bit_size)
                    : nir_imm_floatN_t(b, 1.0, vec->bit_size);
   default:
      unreachable("Invalid swizzle channel");
   }
}

static bool
pass(struct nir_builder *b, nir_instr *instr, void *data)
{
   if (instr->type != nir_instr_type_intrinsic)
      return false;

   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
   if (intr->intrinsic != nir_intrinsic_load_input)
      return false;

   struct agx_attribute *attribs = data;
   b->cursor = nir_before_instr(instr);

   nir_src *offset_src = nir_get_io_offset_src(intr);
   assert(nir_src_is_const(*offset_src) && "no attribute indirects");
   unsigned index = nir_intrinsic_base(intr) + nir_src_as_uint(*offset_src);

   struct agx_attribute attrib = attribs[index];
   uint32_t stride = attrib.stride;
   uint16_t offset = attrib.src_offset;

   const struct util_format_description *desc =
      util_format_description(attrib.format);
   int chan = util_format_get_first_non_void_channel(attrib.format);
   assert(chan >= 0);

   bool is_float = desc->channel[chan].type == UTIL_FORMAT_TYPE_FLOAT;
   bool is_unsigned = desc->channel[chan].type == UTIL_FORMAT_TYPE_UNSIGNED;
   bool is_signed = desc->channel[chan].type == UTIL_FORMAT_TYPE_SIGNED;
   bool is_fixed = desc->channel[chan].type == UTIL_FORMAT_TYPE_FIXED;
   bool is_int = util_format_is_pure_integer(attrib.format);

   assert((is_float ^ is_unsigned ^ is_signed ^ is_fixed) && "Invalid format");

   enum pipe_format interchange_format = agx_vbo_internal_format(attrib.format);
   assert(interchange_format != PIPE_FORMAT_NONE);

   unsigned interchange_align = util_format_get_blocksize(interchange_format);
   unsigned interchange_comps = util_format_get_nr_components(attrib.format);

   /* In the hardware, uint formats zero-extend and float formats convert.
    * However, non-uint formats using a uint interchange format shouldn't be
    * zero extended.
    */
   unsigned interchange_register_size =
      util_format_is_pure_uint(interchange_format) &&
            !util_format_is_pure_uint(attrib.format)
         ? (interchange_align * 8)
         : intr->def.bit_size;

   /* Non-UNORM R10G10B10A2 loaded as a scalar and unpacked */
   if (interchange_format == PIPE_FORMAT_R32_UINT && !desc->is_array)
      interchange_comps = 1;

   /* Calculate the element to fetch the vertex for. Divide the instance ID by
    * the divisor for per-instance data. Divisor=0 specifies per-vertex data.
    */
   nir_def *el;
   if (attrib.divisor) {
      el = nir_udiv_imm(b, nir_load_instance_id(b), attrib.divisor);
      el = nir_iadd(b, el, nir_load_base_instance(b));

      BITSET_SET(b->shader->info.system_values_read,
                 SYSTEM_VALUE_BASE_INSTANCE);
   } else {
      el = nir_load_vertex_id(b);
   }

   nir_def *base = nir_load_vbo_base_agx(b, nir_imm_int(b, attrib.buf));

   assert((stride % interchange_align) == 0 && "must be aligned");
   assert((offset % interchange_align) == 0 && "must be aligned");

   unsigned stride_el = stride / interchange_align;
   unsigned offset_el = offset / interchange_align;
   unsigned shift = 0;

   /* Try to use the small shift on the load itself when possible. This can save
    * an instruction. Shifts are only available for regular interchange formats,
    * i.e. the set of formats that support masking.
    */
   if (offset_el == 0 && (stride_el == 2 || stride_el == 4) &&
       agx_internal_format_supports_mask(
          (enum agx_internal_formats)interchange_format)) {

      shift = util_logbase2(stride_el);
      stride_el = 1;
   }

   nir_def *stride_offset_el =
      nir_iadd_imm(b, nir_imul_imm(b, el, stride_el), offset_el);

   /* Load the raw vector */
   nir_def *memory = nir_load_constant_agx(
      b, interchange_comps, interchange_register_size, base, stride_offset_el,
      .format = interchange_format, .base = shift);

   unsigned dest_size = intr->def.bit_size;

   /* Unpack but do not convert non-native non-array formats */
   if (is_rgb10_a2(desc) && interchange_format == PIPE_FORMAT_R32_UINT) {
      unsigned bits[] = {10, 10, 10, 2};

      if (is_signed)
         memory = nir_format_unpack_sint(b, memory, bits, 4);
      else
         memory = nir_format_unpack_uint(b, memory, bits, 4);
   }

   if (desc->channel[chan].normalized) {
      /* 8/16-bit normalized formats are native, others converted here */
      if (is_rgb10_a2(desc) && is_signed) {
         unsigned bits[] = {10, 10, 10, 2};
         memory = nir_format_snorm_to_float(b, memory, bits);
      } else if (desc->channel[chan].size == 32) {
         assert(desc->is_array && "no non-array 32-bit norm formats");
         unsigned bits[] = {32, 32, 32, 32};

         if (is_signed)
            memory = nir_format_snorm_to_float(b, memory, bits);
         else
            memory = nir_format_unorm_to_float(b, memory, bits);
      }
   } else if (desc->channel[chan].pure_integer) {
      /* Zero-extension is native, may need to sign extend */
      if (is_signed)
         memory = nir_i2iN(b, memory, dest_size);
   } else {
      if (is_unsigned)
         memory = nir_u2fN(b, memory, dest_size);
      else if (is_signed || is_fixed)
         memory = nir_i2fN(b, memory, dest_size);
      else
         memory = nir_f2fN(b, memory, dest_size);

      /* 16.16 fixed-point weirdo GL formats need to be scaled */
      if (is_fixed) {
         assert(desc->is_array && desc->channel[chan].size == 32);
         assert(dest_size == 32 && "overflow if smaller");
         memory = nir_fmul_imm(b, memory, 1.0 / 65536.0);
      }
   }

   /* We now have a properly formatted vector of the components in memory. Apply
    * the format swizzle forwards to trim/pad/reorder as needed.
    */
   nir_def *channels[4] = {NULL};

   for (unsigned i = 0; i < intr->num_components; ++i) {
      unsigned c = nir_intrinsic_component(intr) + i;
      channels[i] = apply_swizzle_channel(b, memory, desc->swizzle[c], is_int);
   }

   nir_def *logical = nir_vec(b, channels, intr->num_components);
   nir_def_rewrite_uses(&intr->def, logical);
   return true;
}

bool
agx_nir_lower_vbo(nir_shader *shader, struct agx_attribute *attribs)
{
   assert(shader->info.stage == MESA_SHADER_VERTEX);
   return nir_shader_instructions_pass(
      shader, pass, nir_metadata_block_index | nir_metadata_dominance, attribs);
}
