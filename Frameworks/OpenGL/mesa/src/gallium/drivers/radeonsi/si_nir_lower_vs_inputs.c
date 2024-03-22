/*
 * Copyright 2023 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#include "nir_builder.h"

#include "ac_nir.h"
#include "si_shader_internal.h"
#include "si_state.h"
#include "si_pipe.h"

struct lower_vs_inputs_state {
   struct si_shader *shader;
   struct si_shader_args *args;

   nir_def *instance_divisor_constbuf;
   nir_def *vertex_index[16];
};

/* See fast_idiv_by_const.h. */
/* If num != UINT_MAX, this more efficient version can be used. */
/* Set: increment = util_fast_udiv_info::increment; */
static nir_def *
fast_udiv_nuw(nir_builder *b, nir_def *num, nir_def *divisor)
{
   nir_def *multiplier = nir_channel(b, divisor, 0);
   nir_def *pre_shift = nir_channel(b, divisor, 1);
   nir_def *post_shift = nir_channel(b, divisor, 2);
   nir_def *increment = nir_channel(b, divisor, 3);

   num = nir_ushr(b, num, pre_shift);
   num = nir_iadd_nuw(b, num, increment);
   num = nir_umul_high(b, num, multiplier);
   return nir_ushr(b, num, post_shift);
}

static nir_def *
get_vertex_index_for_mono_shader(nir_builder *b, int input_index,
                                 struct lower_vs_inputs_state *s)
{
   const union si_shader_key *key = &s->shader->key;

   bool divisor_is_one =
      key->ge.part.vs.prolog.instance_divisor_is_one & (1u << input_index);
   bool divisor_is_fetched =
      key->ge.part.vs.prolog.instance_divisor_is_fetched & (1u << input_index);

   if (divisor_is_one || divisor_is_fetched) {
      nir_def *instance_id = nir_load_instance_id(b);

      /* This is used to determine vs vgpr count in si_get_vs_vgpr_comp_cnt(). */
      s->shader->info.uses_instanceid = true;

      nir_def *index = NULL;
      if (divisor_is_one) {
         index = instance_id;
      } else {
         nir_def *offset = nir_imm_int(b, input_index * 16);
         nir_def *divisor = nir_load_ubo(b, 4, 32, s->instance_divisor_constbuf, offset,
                                             .range = ~0);

         /* The faster NUW version doesn't work when InstanceID == UINT_MAX.
          * Such InstanceID might not be achievable in a reasonable time though.
          */
         index = fast_udiv_nuw(b, instance_id, divisor);
      }

      nir_def *start_instance = nir_load_base_instance(b);
      return nir_iadd(b, index, start_instance);
   } else {
      nir_def *vertex_id = nir_load_vertex_id_zero_base(b);
      nir_def *base_vertex = nir_load_first_vertex(b);

      return nir_iadd(b, vertex_id, base_vertex);
   }
}

static nir_def *
get_vertex_index_for_part_shader(nir_builder *b, int input_index,
                                 struct lower_vs_inputs_state *s)
{
   return ac_nir_load_arg_at_offset(b, &s->args->ac, s->args->vertex_index0, input_index);
}

static void
get_vertex_index_for_all_inputs(nir_shader *nir, struct lower_vs_inputs_state *s)
{
   nir_function_impl *impl = nir_shader_get_entrypoint(nir);

   nir_builder builder = nir_builder_at(nir_before_impl(impl));
   nir_builder *b = &builder;

   const struct si_shader_selector *sel = s->shader->selector;
   const union si_shader_key *key = &s->shader->key;

   if (key->ge.part.vs.prolog.instance_divisor_is_fetched) {
      s->instance_divisor_constbuf =
         si_nir_load_internal_binding(b, s->args, SI_VS_CONST_INSTANCE_DIVISORS, 4);
   }

   for (int i = 0; i < sel->info.num_inputs; i++) {
      s->vertex_index[i] = s->shader->is_monolithic ?
         get_vertex_index_for_mono_shader(b, i, s) :
         get_vertex_index_for_part_shader(b, i, s);
   }
}

static void
load_vs_input_from_blit_sgpr(nir_builder *b, unsigned input_index,
                             struct lower_vs_inputs_state *s,
                             nir_def *out[4])
{
   nir_def *vertex_id = nir_load_vertex_id_zero_base(b);
   nir_def *sel_x1 = nir_ule_imm(b, vertex_id, 1);
   /* Use nir_ine, because we have 3 vertices and only
    * the middle one should use y2.
    */
   nir_def *sel_y1 = nir_ine_imm(b, vertex_id, 1);

   if (input_index == 0) {
      /* Position: */
      nir_def *x1y1 = ac_nir_load_arg_at_offset(b, &s->args->ac, s->args->vs_blit_inputs, 0);
      nir_def *x2y2 = ac_nir_load_arg_at_offset(b, &s->args->ac, s->args->vs_blit_inputs, 1);

      x1y1 = nir_i2i32(b, nir_unpack_32_2x16(b, x1y1));
      x2y2 = nir_i2i32(b, nir_unpack_32_2x16(b, x2y2));

      nir_def *x1 = nir_channel(b, x1y1, 0);
      nir_def *y1 = nir_channel(b, x1y1, 1);
      nir_def *x2 = nir_channel(b, x2y2, 0);
      nir_def *y2 = nir_channel(b, x2y2, 1);

      out[0] = nir_i2f32(b, nir_bcsel(b, sel_x1, x1, x2));
      out[1] = nir_i2f32(b, nir_bcsel(b, sel_y1, y1, y2));
      out[2] = ac_nir_load_arg_at_offset(b, &s->args->ac, s->args->vs_blit_inputs, 2);
      out[3] = nir_imm_float(b, 1);
   } else {
      bool has_attribute_ring_address = s->shader->selector->screen->info.gfx_level >= GFX11;

      /* Color or texture coordinates: */
      assert(input_index == 1);

      unsigned vs_blit_property = s->shader->selector->info.base.vs.blit_sgprs_amd;
      if (vs_blit_property == SI_VS_BLIT_SGPRS_POS_COLOR + has_attribute_ring_address) {
         for (int i = 0; i < 4; i++)
            out[i] = ac_nir_load_arg_at_offset(b, &s->args->ac, s->args->vs_blit_inputs, 3 + i);
      } else {
         assert(vs_blit_property == SI_VS_BLIT_SGPRS_POS_TEXCOORD + has_attribute_ring_address);

         nir_def *x1 = ac_nir_load_arg_at_offset(b, &s->args->ac, s->args->vs_blit_inputs, 3);
         nir_def *y1 = ac_nir_load_arg_at_offset(b, &s->args->ac, s->args->vs_blit_inputs, 4);
         nir_def *x2 = ac_nir_load_arg_at_offset(b, &s->args->ac, s->args->vs_blit_inputs, 5);
         nir_def *y2 = ac_nir_load_arg_at_offset(b, &s->args->ac, s->args->vs_blit_inputs, 6);

         out[0] = nir_bcsel(b, sel_x1, x1, x2);
         out[1] = nir_bcsel(b, sel_y1, y1, y2);
         out[2] = ac_nir_load_arg_at_offset(b, &s->args->ac, s->args->vs_blit_inputs, 7);
         out[3] = ac_nir_load_arg_at_offset(b, &s->args->ac, s->args->vs_blit_inputs, 8);
      }
   }
}

/**
 * Convert an 11- or 10-bit unsigned floating point number to an f32.
 *
 * The input exponent is expected to be biased analogous to IEEE-754, i.e. by
 * 2^(exp_bits-1) - 1 (as defined in OpenGL and other graphics APIs).
 */
static nir_def *
ufN_to_float(nir_builder *b, nir_def *src, unsigned exp_bits, unsigned mant_bits)
{
   assert(src->bit_size == 32);

   nir_def *mantissa = nir_iand_imm(b, src, (1 << mant_bits) - 1);

   /* Converting normal numbers is just a shift + correcting the exponent bias */
   unsigned normal_shift = 23 - mant_bits;
   unsigned bias_shift = 127 - ((1 << (exp_bits - 1)) - 1);

   nir_def *shifted = nir_ishl_imm(b, src, normal_shift);
   nir_def *normal = nir_iadd_imm(b, shifted, bias_shift << 23);

   /* Converting nan/inf numbers is the same, but with a different exponent update */
   nir_def *naninf = nir_ior_imm(b, normal, 0xff << 23);

   /* Converting denormals is the complex case: determine the leading zeros of the
    * mantissa to obtain the correct shift for the mantissa and exponent correction.
    */
   nir_def *ctlz = nir_uclz(b, mantissa);
   /* Shift such that the leading 1 ends up as the LSB of the exponent field. */
   nir_def *denormal = nir_ishl(b, mantissa, nir_iadd_imm(b, ctlz, -8));

   unsigned denormal_exp = bias_shift + (32 - mant_bits) - 1;
   nir_def *tmp = nir_isub_imm(b, denormal_exp, ctlz);
   denormal = nir_iadd(b, denormal, nir_ishl_imm(b, tmp, 23));

   /* Select the final result. */
   nir_def *cond = nir_uge_imm(b, src, ((1ULL << exp_bits) - 1) << mant_bits);
   nir_def *result = nir_bcsel(b, cond, naninf, normal);

   cond = nir_uge_imm(b, src, 1ULL << mant_bits);
   result = nir_bcsel(b, cond, result, denormal);

   cond = nir_ine_imm(b, src, 0);
   result = nir_bcsel(b, cond, result, nir_imm_int(b, 0));

   return result;
}

/**
 * Generate a fully general open coded buffer format fetch with all required
 * fixups suitable for vertex fetch, using non-format buffer loads.
 *
 * Some combinations of argument values have special interpretations:
 * - size = 8 bytes, format = fixed indicates PIPE_FORMAT_R11G11B10_FLOAT
 * - size = 8 bytes, format != {float,fixed} indicates a 2_10_10_10 data format
 */
static void
opencoded_load_format(nir_builder *b, nir_def *rsrc, nir_def *vindex,
                      union si_vs_fix_fetch fix_fetch, bool known_aligned,
                      enum amd_gfx_level gfx_level, nir_def *out[4])
{
   unsigned log_size = fix_fetch.u.log_size;
   unsigned num_channels = fix_fetch.u.num_channels_m1 + 1;
   unsigned format = fix_fetch.u.format;
   bool reverse = fix_fetch.u.reverse;

   unsigned load_log_size = log_size;
   unsigned load_num_channels = num_channels;
   if (log_size == 3) {
      load_log_size = 2;
      if (format == AC_FETCH_FORMAT_FLOAT) {
         load_num_channels = 2 * num_channels;
      } else {
         load_num_channels = 1; /* 10_11_11 or 2_10_10_10 */
      }
   }

   int log_recombine = 0;
   if ((gfx_level == GFX6 || gfx_level >= GFX10) && !known_aligned) {
      /* Avoid alignment restrictions by loading one byte at a time. */
      load_num_channels <<= load_log_size;
      log_recombine = load_log_size;
      load_log_size = 0;
   } else if (load_num_channels == 2 || load_num_channels == 4) {
      log_recombine = -util_logbase2(load_num_channels);
      load_num_channels = 1;
      load_log_size += -log_recombine;
   }

   nir_def *loads[32]; /* up to 32 bytes */
   for (unsigned i = 0; i < load_num_channels; ++i) {
      nir_def *soffset = nir_imm_int(b, i << load_log_size);
      unsigned num_channels = 1 << (MAX2(load_log_size, 2) - 2);
      unsigned bit_size = 8 << MIN2(load_log_size, 2);
      nir_def *zero = nir_imm_int(b, 0);

      loads[i] = nir_load_buffer_amd(b, num_channels, bit_size, rsrc, zero, soffset, vindex);
   }

   if (log_recombine > 0) {
      /* Recombine bytes if necessary (GFX6 only) */
      unsigned dst_bitsize = log_recombine == 2 ? 32 : 16;

      for (unsigned src = 0, dst = 0; src < load_num_channels; ++dst) {
         nir_def *accum = NULL;
         for (unsigned i = 0; i < (1 << log_recombine); ++i, ++src) {
            nir_def *tmp = nir_u2uN(b, loads[src], dst_bitsize);
            if (i == 0) {
               accum = tmp;
            } else {
               tmp = nir_ishl_imm(b, tmp, 8 * i);
               accum = nir_ior(b, accum, tmp);
            }
         }
         loads[dst] = accum;
      }
   } else if (log_recombine < 0) {
      /* Split vectors of dwords */
      if (load_log_size > 2) {
         assert(load_num_channels == 1);
         nir_def *loaded = loads[0];
         unsigned log_split = load_log_size - 2;
         log_recombine += log_split;
         load_num_channels = 1 << log_split;
         load_log_size = 2;
         for (unsigned i = 0; i < load_num_channels; ++i)
            loads[i] = nir_channel(b, loaded, i);
      }

      /* Further split dwords and shorts if required */
      if (log_recombine < 0) {
         for (unsigned src = load_num_channels, dst = load_num_channels << -log_recombine;
              src > 0; --src) {
            unsigned dst_bits = 1 << (3 + load_log_size + log_recombine);
            nir_def *loaded = loads[src - 1];
            for (unsigned i = 1 << -log_recombine; i > 0; --i, --dst) {
               nir_def *tmp = nir_ushr_imm(b, loaded, dst_bits * (i - 1));
               loads[dst - 1] = nir_u2uN(b, tmp, dst_bits);
            }
         }
      }
   }

   if (log_size == 3) {
      switch (format) {
      case AC_FETCH_FORMAT_FLOAT: {
         for (unsigned i = 0; i < num_channels; ++i)
            loads[i] = nir_pack_64_2x32_split(b, loads[2 * i], loads[2 * i + 1]);
         break;
      }
      case AC_FETCH_FORMAT_FIXED: {
         /* 10_11_11_FLOAT */
         nir_def *data = loads[0];
         nir_def *red = nir_iand_imm(b, data, 2047);
         nir_def *green = nir_iand_imm(b, nir_ushr_imm(b, data, 11), 2047);
         nir_def *blue = nir_ushr_imm(b, data, 22);

         loads[0] = ufN_to_float(b, red, 5, 6);
         loads[1] = ufN_to_float(b, green, 5, 6);
         loads[2] = ufN_to_float(b, blue, 5, 5);

         num_channels = 3;
         log_size = 2;
         format = AC_FETCH_FORMAT_FLOAT;
         break;
      }
      case AC_FETCH_FORMAT_UINT:
      case AC_FETCH_FORMAT_UNORM:
      case AC_FETCH_FORMAT_USCALED: {
         /* 2_10_10_10 data formats */
         nir_def *data = loads[0];

         loads[0] = nir_ubfe_imm(b, data, 0, 10);
         loads[1] = nir_ubfe_imm(b, data, 10, 10);
         loads[2] = nir_ubfe_imm(b, data, 20, 10);
         loads[3] = nir_ubfe_imm(b, data, 30, 2);

         num_channels = 4;
         break;
      }
      case AC_FETCH_FORMAT_SINT:
      case AC_FETCH_FORMAT_SNORM:
      case AC_FETCH_FORMAT_SSCALED: {
         /* 2_10_10_10 data formats */
         nir_def *data = loads[0];

         loads[0] = nir_ibfe_imm(b, data, 0, 10);
         loads[1] = nir_ibfe_imm(b, data, 10, 10);
         loads[2] = nir_ibfe_imm(b, data, 20, 10);
         loads[3] = nir_ibfe_imm(b, data, 30, 2);

         num_channels = 4;
         break;
      }
      default:
         unreachable("invalid fetch format");
         break;
      }
   }

   switch (format) {
   case AC_FETCH_FORMAT_FLOAT:
      if (log_size != 2) {
         for (unsigned chan = 0; chan < num_channels; ++chan)
            loads[chan] = nir_f2f32(b, loads[chan]);
      }
      break;
   case AC_FETCH_FORMAT_UINT:
      if (log_size != 2) {
         for (unsigned chan = 0; chan < num_channels; ++chan)
            loads[chan] = nir_u2u32(b, loads[chan]);
      }
      break;
   case AC_FETCH_FORMAT_SINT:
      if (log_size != 2) {
         for (unsigned chan = 0; chan < num_channels; ++chan)
            loads[chan] = nir_i2i32(b, loads[chan]);
      }
      break;
   case AC_FETCH_FORMAT_USCALED:
      for (unsigned chan = 0; chan < num_channels; ++chan)
         loads[chan] = nir_u2f32(b, loads[chan]);
      break;
   case AC_FETCH_FORMAT_SSCALED:
      for (unsigned chan = 0; chan < num_channels; ++chan)
         loads[chan] = nir_i2f32(b, loads[chan]);
      break;
   case AC_FETCH_FORMAT_FIXED:
      for (unsigned chan = 0; chan < num_channels; ++chan) {
         nir_def *tmp = nir_i2f32(b, loads[chan]);
         loads[chan] = nir_fmul_imm(b, tmp, 1.0 / 0x10000);
      }
      break;
   case AC_FETCH_FORMAT_UNORM:
      for (unsigned chan = 0; chan < num_channels; ++chan) {
         /* 2_10_10_10 data formats */
         unsigned bits = log_size == 3 ? (chan == 3 ? 2 : 10) : (8 << log_size);
         nir_def *tmp = nir_u2f32(b, loads[chan]);
         loads[chan] = nir_fmul_imm(b, tmp, 1.0 / (double)BITFIELD64_MASK(bits));
      }
      break;
   case AC_FETCH_FORMAT_SNORM:
      for (unsigned chan = 0; chan < num_channels; ++chan) {
         /* 2_10_10_10 data formats */
         unsigned bits = log_size == 3 ? (chan == 3 ? 2 : 10) : (8 << log_size);
         nir_def *tmp = nir_i2f32(b, loads[chan]);
         tmp = nir_fmul_imm(b, tmp, 1.0 / (double)BITFIELD64_MASK(bits - 1));
         /* Clamp to [-1, 1] */
         tmp = nir_fmax(b, tmp, nir_imm_float(b, -1));
         loads[chan] = nir_fmin(b, tmp, nir_imm_float(b, 1));
      }
      break;
   default:
      unreachable("invalid fetch format");
      break;
   }

   while (num_channels < 4) {
      unsigned pad_value = num_channels == 3 ? 1 : 0;
      loads[num_channels] =
         format == AC_FETCH_FORMAT_UINT || format == AC_FETCH_FORMAT_SINT ?
         nir_imm_int(b, pad_value) : nir_imm_float(b, pad_value);
      num_channels++;
   }

   if (reverse) {
      nir_def *tmp = loads[0];
      loads[0] = loads[2];
      loads[2] = tmp;
   }

   memcpy(out, loads, 4 * sizeof(out[0]));
}

static void
load_vs_input_from_vertex_buffer(nir_builder *b, unsigned input_index,
                                 struct lower_vs_inputs_state *s,
                                 unsigned bit_size, nir_def *out[4])
{
   const struct si_shader_selector *sel = s->shader->selector;
   const union si_shader_key *key = &s->shader->key;

   nir_def *vb_desc;
   if (input_index < sel->info.num_vbos_in_user_sgprs) {
      vb_desc = ac_nir_load_arg(b, &s->args->ac, s->args->vb_descriptors[input_index]);
   } else {
      unsigned index = input_index - sel->info.num_vbos_in_user_sgprs;
      nir_def *addr = ac_nir_load_arg(b, &s->args->ac, s->args->ac.vertex_buffers);
      vb_desc = nir_load_smem_amd(b, 4, addr, nir_imm_int(b, index * 16));
   }

   nir_def *vertex_index = s->vertex_index[input_index];

   /* Use the open-coded implementation for all loads of doubles and
    * of dword-sized data that needs fixups. We need to insert conversion
    * code anyway.
    */
   bool opencode = key->ge.mono.vs_fetch_opencode & (1 << input_index);
   union si_vs_fix_fetch fix_fetch = key->ge.mono.vs_fix_fetch[input_index];
   if (opencode ||
       (fix_fetch.u.log_size == 3 && fix_fetch.u.format == AC_FETCH_FORMAT_FLOAT) ||
       fix_fetch.u.log_size == 2) {
      opencoded_load_format(b, vb_desc, vertex_index, fix_fetch, !opencode,
                            sel->screen->info.gfx_level, out);

      if (bit_size == 16) {
         if (fix_fetch.u.format == AC_FETCH_FORMAT_UINT ||
             fix_fetch.u.format == AC_FETCH_FORMAT_SINT) {
            for (unsigned i = 0; i < 4; i++)
               out[i] = nir_u2u16(b, out[i]);
         } else {
            for (unsigned i = 0; i < 4; i++)
               out[i] = nir_f2f16(b, out[i]);
         }
      }
      return;
   }

   unsigned required_channels = util_last_bit(sel->info.input[input_index].usage_mask);
   if (required_channels == 0) {
      for (unsigned i = 0; i < 4; ++i)
         out[i] = nir_undef(b, 1, bit_size);
      return;
   }

   /* Do multiple loads for special formats. */
   nir_def *fetches[4];
   unsigned num_fetches;
   unsigned fetch_stride;
   unsigned channels_per_fetch;

   if (fix_fetch.u.log_size <= 1 && fix_fetch.u.num_channels_m1 == 2) {
      num_fetches = MIN2(required_channels, 3);
      fetch_stride = 1 << fix_fetch.u.log_size;
      channels_per_fetch = 1;
   } else {
      num_fetches = 1;
      fetch_stride = 0;
      channels_per_fetch = required_channels;
   }

   for (unsigned i = 0; i < num_fetches; ++i) {
      nir_def *zero = nir_imm_int(b, 0);
      fetches[i] = nir_load_buffer_amd(b, channels_per_fetch, bit_size, vb_desc,
                                       zero, zero, vertex_index,
                                       .base = fetch_stride * i,
                                       .access = ACCESS_USES_FORMAT_AMD);
   }

   if (num_fetches == 1 && channels_per_fetch > 1) {
      nir_def *fetch = fetches[0];
      for (unsigned i = 0; i < channels_per_fetch; ++i)
         fetches[i] = nir_channel(b, fetch, i);

      num_fetches = channels_per_fetch;
      channels_per_fetch = 1;
   }

   for (unsigned i = num_fetches; i < 4; ++i)
      fetches[i] = nir_undef(b, 1, bit_size);

   if (fix_fetch.u.log_size <= 1 && fix_fetch.u.num_channels_m1 == 2 && required_channels == 4) {
      if (fix_fetch.u.format == AC_FETCH_FORMAT_UINT || fix_fetch.u.format == AC_FETCH_FORMAT_SINT)
         fetches[3] = nir_imm_intN_t(b, 1, bit_size);
      else
         fetches[3] = nir_imm_floatN_t(b, 1, bit_size);
   } else if (fix_fetch.u.log_size == 3 &&
              (fix_fetch.u.format == AC_FETCH_FORMAT_SNORM ||
               fix_fetch.u.format == AC_FETCH_FORMAT_SSCALED ||
               fix_fetch.u.format == AC_FETCH_FORMAT_SINT) &&
              required_channels == 4) {

      /* For 2_10_10_10, the hardware returns an unsigned value;
       * convert it to a signed one.
       */
      nir_def *tmp = fetches[3];

      /* First, recover the sign-extended signed integer value. */
      if (fix_fetch.u.format == AC_FETCH_FORMAT_SSCALED)
         tmp = nir_f2uN(b, tmp, bit_size);

      /* For the integer-like cases, do a natural sign extension.
       *
       * For the SNORM case, the values are 0.0, 0.333, 0.666, 1.0
       * and happen to contain 0, 1, 2, 3 as the two LSBs of the
       * exponent.
       */
      tmp = nir_ishl_imm(b, tmp, fix_fetch.u.format == AC_FETCH_FORMAT_SNORM ? 7 : 30);
      tmp = nir_ishr_imm(b, tmp, 30);

      /* Convert back to the right type. */
      if (fix_fetch.u.format == AC_FETCH_FORMAT_SNORM) {
         tmp = nir_i2fN(b, tmp, bit_size);
         /* Clamp to [-1, 1] */
         tmp = nir_fmax(b, tmp, nir_imm_float(b, -1));
         tmp = nir_fmin(b, tmp, nir_imm_float(b, 1));
      } else if (fix_fetch.u.format == AC_FETCH_FORMAT_SSCALED) {
         tmp = nir_i2fN(b, tmp, bit_size);
      }

      fetches[3] = tmp;
   }

   memcpy(out, fetches, 4 * sizeof(out[0]));
}

static bool
lower_vs_input_instr(nir_builder *b, nir_intrinsic_instr *intrin, void *state)
{
   if (intrin->intrinsic != nir_intrinsic_load_input)
      return false;

   struct lower_vs_inputs_state *s = (struct lower_vs_inputs_state *)state;

   b->cursor = nir_before_instr(&intrin->instr);

   unsigned input_index = nir_intrinsic_base(intrin);
   unsigned component = nir_intrinsic_component(intrin);
   unsigned num_components = intrin->def.num_components;

   nir_def *comp[4];
   if (s->shader->selector->info.base.vs.blit_sgprs_amd)
      load_vs_input_from_blit_sgpr(b, input_index, s, comp);
   else
      load_vs_input_from_vertex_buffer(b, input_index, s, intrin->def.bit_size, comp);

   nir_def *replacement = nir_vec(b, &comp[component], num_components);

   nir_def_rewrite_uses(&intrin->def, replacement);
   nir_instr_remove(&intrin->instr);
   nir_instr_free(&intrin->instr);

   return true;
}

bool
si_nir_lower_vs_inputs(nir_shader *nir, struct si_shader *shader, struct si_shader_args *args)
{
   const struct si_shader_selector *sel = shader->selector;

   /* no inputs to lower */
   if (!sel->info.num_inputs)
      return false;

   struct lower_vs_inputs_state state = {
      .shader = shader,
      .args = args,
   };

   if (!sel->info.base.vs.blit_sgprs_amd)
      get_vertex_index_for_all_inputs(nir, &state);

   return nir_shader_intrinsics_pass(nir, lower_vs_input_instr,
                                       nir_metadata_dominance | nir_metadata_block_index,
                                       &state);
}
