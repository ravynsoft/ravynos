/*
 * Copyright (C) 2018 Alyssa Rosenzweig
 * Copyright (C) 2019-2021 Collabora, Ltd.
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

#include "pan_shader.h"
#include "pan_device.h"
#include "pan_format.h"

#if PAN_ARCH <= 5
#include "panfrost/midgard/midgard_compile.h"
#else
#include "panfrost/compiler/bifrost_compile.h"
#endif

const nir_shader_compiler_options *
GENX(pan_shader_get_compiler_options)(void)
{
#if PAN_ARCH >= 9
   return &bifrost_nir_options_v9;
#elif PAN_ARCH >= 6
   return &bifrost_nir_options_v6;
#else
   return &midgard_nir_options;
#endif
}

#if PAN_ARCH >= 6
static enum mali_register_file_format
bifrost_blend_type_from_nir(nir_alu_type nir_type)
{
   switch (nir_type) {
   case 0: /* Render target not in use */
      return 0;
   case nir_type_float16:
      return MALI_REGISTER_FILE_FORMAT_F16;
   case nir_type_float32:
      return MALI_REGISTER_FILE_FORMAT_F32;
   case nir_type_int32:
      return MALI_REGISTER_FILE_FORMAT_I32;
   case nir_type_uint32:
      return MALI_REGISTER_FILE_FORMAT_U32;
   case nir_type_int16:
      return MALI_REGISTER_FILE_FORMAT_I16;
   case nir_type_uint16:
      return MALI_REGISTER_FILE_FORMAT_U16;
   default:
      unreachable("Unsupported blend shader type for NIR alu type");
      return 0;
   }
}

#if PAN_ARCH <= 7
enum mali_register_file_format
GENX(pan_fixup_blend_type)(nir_alu_type T_size, enum pipe_format format)
{
   const struct util_format_description *desc = util_format_description(format);
   unsigned size = nir_alu_type_get_type_size(T_size);
   nir_alu_type T_format = pan_unpacked_type_for_format(desc);
   nir_alu_type T = nir_alu_type_get_base_type(T_format) | size;

   return bifrost_blend_type_from_nir(T);
}
#endif
#endif

/* This is only needed on Midgard. It's the same on both v4 and v5, so only
 * compile once to avoid the GenXML dependency for calls.
 */
#if PAN_ARCH == 5
uint8_t
pan_raw_format_mask_midgard(enum pipe_format *formats)
{
   uint8_t out = 0;

   for (unsigned i = 0; i < 8; i++) {
      enum pipe_format fmt = formats[i];
      unsigned wb_fmt = panfrost_blendable_formats_v6[fmt].writeback;

      if (wb_fmt < MALI_COLOR_FORMAT_R8)
         out |= BITFIELD_BIT(i);
   }

   return out;
}
#endif

void
GENX(pan_shader_compile)(nir_shader *s, struct panfrost_compile_inputs *inputs,
                         struct util_dynarray *binary,
                         struct pan_shader_info *info)
{
   memset(info, 0, sizeof(*info));

#if PAN_ARCH >= 6
   bifrost_compile_shader_nir(s, inputs, binary, info);
#else
   midgard_compile_shader_nir(s, inputs, binary, info);
#endif

   info->stage = s->info.stage;
   info->contains_barrier =
      s->info.uses_memory_barrier || s->info.uses_control_barrier;
   info->separable = s->info.separate_shader;

   switch (info->stage) {
   case MESA_SHADER_VERTEX:
      info->attributes_read = s->info.inputs_read;
      info->attributes_read_count = util_bitcount64(info->attributes_read);
      info->attribute_count = info->attributes_read_count;

#if PAN_ARCH <= 5
      bool vertex_id = BITSET_TEST(s->info.system_values_read,
                                   SYSTEM_VALUE_VERTEX_ID_ZERO_BASE);
      if (vertex_id)
         info->attribute_count = MAX2(info->attribute_count, PAN_VERTEX_ID + 1);

      bool instance_id =
         BITSET_TEST(s->info.system_values_read, SYSTEM_VALUE_INSTANCE_ID);
      if (instance_id)
         info->attribute_count =
            MAX2(info->attribute_count, PAN_INSTANCE_ID + 1);
#endif

      info->vs.writes_point_size =
         s->info.outputs_written & (1 << VARYING_SLOT_PSIZ);

#if PAN_ARCH >= 9
      info->varyings.output_count =
         util_last_bit(s->info.outputs_written >> VARYING_SLOT_VAR0);
#endif
      break;
   case MESA_SHADER_FRAGMENT:
      if (s->info.outputs_written & BITFIELD64_BIT(FRAG_RESULT_DEPTH))
         info->fs.writes_depth = true;
      if (s->info.outputs_written & BITFIELD64_BIT(FRAG_RESULT_STENCIL))
         info->fs.writes_stencil = true;
      if (s->info.outputs_written & BITFIELD64_BIT(FRAG_RESULT_SAMPLE_MASK))
         info->fs.writes_coverage = true;

      info->fs.outputs_read = s->info.outputs_read >> FRAG_RESULT_DATA0;
      info->fs.outputs_written = s->info.outputs_written >> FRAG_RESULT_DATA0;
      info->fs.sample_shading = s->info.fs.uses_sample_shading;
      info->fs.untyped_color_outputs = s->info.fs.untyped_color_outputs;

      info->fs.can_discard = s->info.fs.uses_discard;
      info->fs.early_fragment_tests = s->info.fs.early_fragment_tests;

      /* List of reasons we need to execute frag shaders when things
       * are masked off */

      info->fs.sidefx = s->info.writes_memory || s->info.fs.uses_discard ||
                        s->info.fs.uses_demote;

      /* With suitable ZSA/blend, is early-z possible? */
      info->fs.can_early_z = !info->fs.sidefx && !info->fs.writes_depth &&
                             !info->fs.writes_stencil &&
                             !info->fs.writes_coverage;

      /* Similiarly with suitable state, is FPK possible? */
      info->fs.can_fpk = !info->fs.writes_depth && !info->fs.writes_stencil &&
                         !info->fs.writes_coverage && !info->fs.can_discard &&
                         !info->fs.outputs_read;

      /* Requires the same hardware guarantees, so grouped as one bit
       * in the hardware.
       */
      info->contains_barrier |= s->info.fs.needs_quad_helper_invocations;

      info->fs.reads_frag_coord =
         (s->info.inputs_read & (1 << VARYING_SLOT_POS)) ||
         BITSET_TEST(s->info.system_values_read, SYSTEM_VALUE_FRAG_COORD);
      info->fs.reads_point_coord =
         s->info.inputs_read & (1 << VARYING_SLOT_PNTC);
      info->fs.reads_face =
         (s->info.inputs_read & (1 << VARYING_SLOT_FACE)) ||
         BITSET_TEST(s->info.system_values_read, SYSTEM_VALUE_FRONT_FACE);
#if PAN_ARCH >= 9
      info->varyings.output_count =
         util_last_bit(s->info.outputs_read >> VARYING_SLOT_VAR0);
#endif
      break;
   default:
      /* Everything else treated as compute */
      info->wls_size = s->info.shared_size;
      break;
   }

   info->outputs_written = s->info.outputs_written;
   info->attribute_count += BITSET_LAST_BIT(s->info.images_used);
   info->writes_global = s->info.writes_memory;
   info->ubo_count = s->info.num_ubos;

   info->sampler_count = info->texture_count =
      BITSET_LAST_BIT(s->info.textures_used);

   unsigned execution_mode = s->info.float_controls_execution_mode;
   info->ftz_fp16 = nir_is_denorm_flush_to_zero(execution_mode, 16);
   info->ftz_fp32 = nir_is_denorm_flush_to_zero(execution_mode, 32);

#if PAN_ARCH >= 6
   /* This is "redundant" information, but is needed in a draw-time hot path */
   for (unsigned i = 0; i < ARRAY_SIZE(info->bifrost.blend); ++i) {
      info->bifrost.blend[i].format =
         bifrost_blend_type_from_nir(info->bifrost.blend[i].type);
   }
#endif
}
