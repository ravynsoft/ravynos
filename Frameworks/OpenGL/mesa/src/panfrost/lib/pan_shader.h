/*
 * Copyright (C) 2021 Collabora, Ltd.
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
 *
 */

#ifndef __PAN_SHADER_H__
#define __PAN_SHADER_H__

#include "compiler/nir/nir.h"
#include "panfrost/util/pan_ir.h"
#include "panfrost/util/pan_lower_framebuffer.h"

#include "genxml/gen_macros.h"
#include "pan_device.h"

struct panfrost_device;

void bifrost_preprocess_nir(nir_shader *nir, unsigned gpu_id);
void midgard_preprocess_nir(nir_shader *nir, unsigned gpu_id);

static inline void
pan_shader_preprocess(nir_shader *nir, unsigned gpu_id)
{
   if (pan_arch(gpu_id) >= 6)
      bifrost_preprocess_nir(nir, gpu_id);
   else
      midgard_preprocess_nir(nir, gpu_id);
}

uint8_t pan_raw_format_mask_midgard(enum pipe_format *formats);

#ifdef PAN_ARCH
const nir_shader_compiler_options *GENX(pan_shader_get_compiler_options)(void);

void GENX(pan_shader_compile)(nir_shader *nir,
                              struct panfrost_compile_inputs *inputs,
                              struct util_dynarray *binary,
                              struct pan_shader_info *info);

#if PAN_ARCH >= 6 && PAN_ARCH <= 7
enum mali_register_file_format
   GENX(pan_fixup_blend_type)(nir_alu_type T_size, enum pipe_format format);
#endif

#if PAN_ARCH >= 9
static inline enum mali_shader_stage
pan_shader_stage(const struct pan_shader_info *info)
{
   switch (info->stage) {
   case MESA_SHADER_VERTEX:
      return MALI_SHADER_STAGE_VERTEX;
   case MESA_SHADER_FRAGMENT:
      return MALI_SHADER_STAGE_FRAGMENT;
   default:
      return MALI_SHADER_STAGE_COMPUTE;
   }
}
#endif

#if PAN_ARCH >= 7
static inline enum mali_shader_register_allocation
pan_register_allocation(unsigned work_reg_count)
{
   return (work_reg_count <= 32)
             ? MALI_SHADER_REGISTER_ALLOCATION_32_PER_THREAD
             : MALI_SHADER_REGISTER_ALLOCATION_64_PER_THREAD;
}
#endif

static inline enum mali_depth_source
pan_depth_source(const struct pan_shader_info *info)
{
   return info->fs.writes_depth ? MALI_DEPTH_SOURCE_SHADER
                                : MALI_DEPTH_SOURCE_FIXED_FUNCTION;
}

#if PAN_ARCH <= 7
#if PAN_ARCH <= 5
static inline void
pan_shader_prepare_midgard_rsd(const struct pan_shader_info *info,
                               struct MALI_RENDERER_STATE *rsd)
{
   assert((info->push.count & 3) == 0);

   rsd->properties.uniform_count = info->push.count / 4;
   rsd->properties.shader_has_side_effects = info->writes_global;
   rsd->properties.fp_mode = MALI_FP_MODE_GL_INF_NAN_ALLOWED;

   /* For fragment shaders, work register count, early-z, reads at draw-time */

   if (info->stage != MESA_SHADER_FRAGMENT) {
      rsd->properties.work_register_count = info->work_reg_count;
   } else {
      rsd->properties.shader_reads_tilebuffer = info->fs.outputs_read;

      /* However, forcing early-z in the shader overrides draw-time */
      rsd->properties.force_early_z = info->fs.early_fragment_tests;
   }
}

#else

#define pan_preloads(reg) (preload & BITFIELD64_BIT(reg))

static void
pan_make_preload(gl_shader_stage stage, uint64_t preload,
                 struct MALI_PRELOAD *out)
{
   switch (stage) {
   case MESA_SHADER_VERTEX:
      out->vertex.position_result_address_lo = pan_preloads(58);
      out->vertex.position_result_address_hi = pan_preloads(59);
      out->vertex.vertex_id = pan_preloads(61);
      out->vertex.instance_id = pan_preloads(62);
      break;

   case MESA_SHADER_FRAGMENT:
      out->fragment.primitive_id = pan_preloads(57);
      out->fragment.primitive_flags = pan_preloads(58);
      out->fragment.fragment_position = pan_preloads(59);
      out->fragment.sample_mask_id = pan_preloads(61);
      out->fragment.coverage = true;
      break;

   default:
      out->compute.local_invocation_xy = pan_preloads(55);
      out->compute.local_invocation_z = pan_preloads(56);
      out->compute.work_group_x = pan_preloads(57);
      out->compute.work_group_y = pan_preloads(58);
      out->compute.work_group_z = pan_preloads(59);
      out->compute.global_invocation_x = pan_preloads(60);
      out->compute.global_invocation_y = pan_preloads(61);
      out->compute.global_invocation_z = pan_preloads(62);
      break;
   }
}

#if PAN_ARCH == 7
static inline void
pan_pack_message_preload(struct MALI_MESSAGE_PRELOAD *cfg,
                         const struct bifrost_message_preload *msg)
{
   enum mali_message_preload_register_format regfmt =
      msg->fp16 ? MALI_MESSAGE_PRELOAD_REGISTER_FORMAT_F16
                : MALI_MESSAGE_PRELOAD_REGISTER_FORMAT_F32;

   if (msg->enabled && msg->texture) {
      cfg->type = MALI_MESSAGE_TYPE_VAR_TEX;
      cfg->var_tex.varying_index = msg->varying_index;
      cfg->var_tex.texture_index = msg->texture_index;
      cfg->var_tex.register_format = regfmt;
      cfg->var_tex.skip = msg->skip;
      cfg->var_tex.zero_lod = msg->zero_lod;
   } else if (msg->enabled) {
      cfg->type = MALI_MESSAGE_TYPE_LD_VAR;
      cfg->ld_var.varying_index = msg->varying_index;
      cfg->ld_var.register_format = regfmt;
      cfg->ld_var.num_components = msg->num_components;
   } else {
      cfg->type = MALI_MESSAGE_TYPE_DISABLED;
   }
}
#endif

static inline void
pan_shader_prepare_bifrost_rsd(const struct pan_shader_info *info,
                               struct MALI_RENDERER_STATE *rsd)
{
   unsigned fau_count = DIV_ROUND_UP(info->push.count, 2);
   rsd->preload.uniform_count = fau_count;

#if PAN_ARCH >= 7
   rsd->properties.shader_register_allocation =
      pan_register_allocation(info->work_reg_count);
#endif

   pan_make_preload(info->stage, info->preload, &rsd->preload);

   if (info->stage == MESA_SHADER_FRAGMENT) {
      rsd->properties.shader_modifies_coverage =
         info->fs.writes_coverage || info->fs.can_discard;

      rsd->properties.allow_forward_pixel_to_be_killed = !info->writes_global;

#if PAN_ARCH >= 7
      rsd->properties.shader_wait_dependency_6 = info->bifrost.wait_6;
      rsd->properties.shader_wait_dependency_7 = info->bifrost.wait_7;

      pan_pack_message_preload(&rsd->message_preload_1,
                               &info->bifrost.messages[0]);
      pan_pack_message_preload(&rsd->message_preload_2,
                               &info->bifrost.messages[1]);
#endif
   } else if (info->stage == MESA_SHADER_VERTEX && info->vs.secondary_enable) {
      rsd->secondary_preload.uniform_count = fau_count;

      pan_make_preload(info->stage, info->vs.secondary_preload,
                       &rsd->secondary_preload);

      rsd->secondary_shader = rsd->shader.shader + info->vs.secondary_offset;

#if PAN_ARCH >= 7
      rsd->properties.secondary_shader_register_allocation =
         pan_register_allocation(info->vs.secondary_work_reg_count);
#endif
   }
}

#endif

static inline void
pan_shader_prepare_rsd(const struct pan_shader_info *shader_info,
                       mali_ptr shader_ptr, struct MALI_RENDERER_STATE *rsd)
{
#if PAN_ARCH <= 5
   shader_ptr |= shader_info->midgard.first_tag;
#endif

   rsd->shader.shader = shader_ptr;
   rsd->shader.attribute_count = shader_info->attribute_count;
   rsd->shader.varying_count =
      shader_info->varyings.input_count + shader_info->varyings.output_count;
   rsd->shader.texture_count = shader_info->texture_count;
   rsd->shader.sampler_count = shader_info->sampler_count;
   rsd->properties.shader_contains_barrier = shader_info->contains_barrier;
   rsd->properties.uniform_buffer_count = shader_info->ubo_count;

   if (shader_info->stage == MESA_SHADER_FRAGMENT) {
      rsd->properties.stencil_from_shader = shader_info->fs.writes_stencil;
      rsd->properties.depth_source = pan_depth_source(shader_info);

      /* This also needs to be set if the API forces per-sample
       * shading, but that'll just got ORed in */
      rsd->multisample_misc.evaluate_per_sample =
         shader_info->fs.sample_shading;
   }

#if PAN_ARCH >= 6
   pan_shader_prepare_bifrost_rsd(shader_info, rsd);
#else
   pan_shader_prepare_midgard_rsd(shader_info, rsd);
#endif
}
#endif /* PAN_ARCH */
#endif

#endif
