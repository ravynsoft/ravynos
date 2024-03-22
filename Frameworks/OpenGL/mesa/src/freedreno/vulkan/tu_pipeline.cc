/*
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 * SPDX-License-Identifier: MIT
 *
 * based in part on anv driver which is:
 * Copyright © 2015 Intel Corporation
 */

#include "tu_pipeline.h"

#include "common/freedreno_guardband.h"

#include "ir3/ir3_nir.h"
#include "nir/nir.h"
#include "nir/nir_builder.h"
#include "nir/nir_serialize.h"
#include "spirv/nir_spirv.h"
#include "util/u_debug.h"
#include "util/mesa-sha1.h"
#include "vk_nir.h"
#include "vk_pipeline.h"
#include "vk_render_pass.h"
#include "vk_util.h"

#include "tu_cmd_buffer.h"
#include "tu_cs.h"
#include "tu_device.h"
#include "tu_knl.h"
#include "tu_formats.h"
#include "tu_lrz.h"
#include "tu_pass.h"

/* Emit IB that preloads the descriptors that the shader uses */

static void
emit_load_state(struct tu_cs *cs, unsigned opcode, enum a6xx_state_type st,
                enum a6xx_state_block sb, unsigned base, unsigned offset,
                unsigned count)
{
   /* Note: just emit one packet, even if count overflows NUM_UNIT. It's not
    * clear if emitting more packets will even help anything. Presumably the
    * descriptor cache is relatively small, and these packets stop doing
    * anything when there are too many descriptors.
    */
   tu_cs_emit_pkt7(cs, opcode, 3);
   tu_cs_emit(cs,
              CP_LOAD_STATE6_0_STATE_TYPE(st) |
              CP_LOAD_STATE6_0_STATE_SRC(SS6_BINDLESS) |
              CP_LOAD_STATE6_0_STATE_BLOCK(sb) |
              CP_LOAD_STATE6_0_NUM_UNIT(MIN2(count, 1024-1)));
   tu_cs_emit_qw(cs, offset | (base << 28));
}

static unsigned
tu6_load_state_size(struct tu_pipeline *pipeline,
                    struct tu_pipeline_layout *layout)
{
   const unsigned load_state_size = 4;
   unsigned size = 0;
   for (unsigned i = 0; i < layout->num_sets; i++) {
      if (!(pipeline->active_desc_sets & (1u << i)))
         continue;

      struct tu_descriptor_set_layout *set_layout = layout->set[i].layout;
      for (unsigned j = 0; j < set_layout->binding_count; j++) {
         struct tu_descriptor_set_binding_layout *binding = &set_layout->binding[j];
         unsigned count = 0;
         /* See comment in tu6_emit_load_state(). */
         VkShaderStageFlags stages = pipeline->active_stages & binding->shader_stages;
         unsigned stage_count = util_bitcount(stages);

         if (!binding->array_size)
            continue;

         switch (binding->type) {
         case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
         case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
         case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
         case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            /* IBO-backed resources only need one packet for all graphics stages */
            if (stage_count)
               count += 1;
            break;
         case VK_DESCRIPTOR_TYPE_SAMPLER:
         case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
         case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
         case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
         case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
            /* Textures and UBO's needs a packet for each stage */
            count = stage_count;
            break;
         case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            /* Because of how we pack combined images and samplers, we
             * currently can't use one packet for the whole array.
             */
            count = stage_count * binding->array_size * 2;
            break;
         case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
         case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK:
         case VK_DESCRIPTOR_TYPE_MUTABLE_EXT:
            break;
         default:
            unreachable("bad descriptor type");
         }
         size += count * load_state_size;
      }
   }
   return size;
}

static void
tu6_emit_load_state(struct tu_device *device,
                    struct tu_pipeline *pipeline,
                    struct tu_pipeline_layout *layout)
{
   unsigned size = tu6_load_state_size(pipeline, layout);
   if (size == 0)
      return;

   struct tu_cs cs;
   tu_cs_begin_sub_stream(&pipeline->cs, size, &cs);

   for (unsigned i = 0; i < layout->num_sets; i++) {
      /* From 13.2.7. Descriptor Set Binding:
       *
       *    A compatible descriptor set must be bound for all set numbers that
       *    any shaders in a pipeline access, at the time that a draw or
       *    dispatch command is recorded to execute using that pipeline.
       *    However, if none of the shaders in a pipeline statically use any
       *    bindings with a particular set number, then no descriptor set need
       *    be bound for that set number, even if the pipeline layout includes
       *    a non-trivial descriptor set layout for that set number.
       *
       * This means that descriptor sets unused by the pipeline may have a
       * garbage or 0 BINDLESS_BASE register, which will cause context faults
       * when prefetching descriptors from these sets. Skip prefetching for
       * descriptors from them to avoid this. This is also an optimization,
       * since these prefetches would be useless.
       */
      if (!(pipeline->active_desc_sets & (1u << i)))
         continue;

      struct tu_descriptor_set_layout *set_layout = layout->set[i].layout;
      for (unsigned j = 0; j < set_layout->binding_count; j++) {
         struct tu_descriptor_set_binding_layout *binding = &set_layout->binding[j];
         unsigned base = i;
         unsigned offset = binding->offset / 4;
         /* Note: amber sets VK_SHADER_STAGE_ALL for its descriptor layout, and
          * zink has descriptors for each stage in the push layout even if some
          * stages aren't present in a used pipeline.  We don't want to emit
          * loads for unused descriptors.
          */
         VkShaderStageFlags stages = pipeline->active_stages & binding->shader_stages;
         unsigned count = binding->array_size;

         /* If this is a variable-count descriptor, then the array_size is an
          * upper bound on the size, but we don't know how many descriptors
          * will actually be used. Therefore we can't pre-load them here.
          */
         if (j == set_layout->binding_count - 1 &&
             set_layout->has_variable_descriptors)
            continue;

         if (count == 0 || stages == 0)
            continue;
         switch (binding->type) {
         case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
            assert(device->physical_device->reserved_set_idx >= 0);
            base = device->physical_device->reserved_set_idx;
            offset = (pipeline->program.dynamic_descriptor_offsets[i] +
                      binding->dynamic_offset_offset) / 4;
            FALLTHROUGH;
         case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
         case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
         case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: {
            unsigned mul = binding->size / (A6XX_TEX_CONST_DWORDS * 4);
            /* IBO-backed resources only need one packet for all graphics stages */
            if (stages & ~VK_SHADER_STAGE_COMPUTE_BIT) {
               emit_load_state(&cs, CP_LOAD_STATE6, ST6_SHADER, SB6_IBO,
                               base, offset, count * mul);
            }
            if (stages & VK_SHADER_STAGE_COMPUTE_BIT) {
               emit_load_state(&cs, CP_LOAD_STATE6_FRAG, ST6_IBO, SB6_CS_SHADER,
                               base, offset, count * mul);
            }
            break;
         }
         case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
         case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK:
         case VK_DESCRIPTOR_TYPE_MUTABLE_EXT:
            /* nothing - input attachments and inline uniforms don't use bindless */
            break;
         case VK_DESCRIPTOR_TYPE_SAMPLER:
         case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
         case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER: {
            tu_foreach_stage(stage, stages) {
               emit_load_state(&cs, tu6_stage2opcode(stage),
                               binding->type == VK_DESCRIPTOR_TYPE_SAMPLER ?
                               ST6_SHADER : ST6_CONSTANTS,
                               tu6_stage2texsb(stage), base, offset, count);
            }
            break;
         }
         case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
            assert(device->physical_device->reserved_set_idx >= 0);
            base = device->physical_device->reserved_set_idx;
            offset = (pipeline->program.dynamic_descriptor_offsets[i] +
                      binding->dynamic_offset_offset) / 4;
            FALLTHROUGH;
         case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER: {
            tu_foreach_stage(stage, stages) {
               emit_load_state(&cs, tu6_stage2opcode(stage), ST6_UBO,
                               tu6_stage2shadersb(stage), base, offset, count);
            }
            break;
         }
         case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: {
            tu_foreach_stage(stage, stages) {
               /* TODO: We could emit less CP_LOAD_STATE6 if we used
                * struct-of-arrays instead of array-of-structs.
                */
               for (unsigned i = 0; i < count; i++) {
                  unsigned tex_offset = offset + 2 * i * A6XX_TEX_CONST_DWORDS;
                  unsigned sam_offset = offset + (2 * i + 1) * A6XX_TEX_CONST_DWORDS;
                  emit_load_state(&cs, tu6_stage2opcode(stage),
                                  ST6_CONSTANTS, tu6_stage2texsb(stage),
                                  base, tex_offset, 1);
                  emit_load_state(&cs, tu6_stage2opcode(stage),
                                  ST6_SHADER, tu6_stage2texsb(stage),
                                  base, sam_offset, 1);
               }
            }
            break;
         }
         default:
            unreachable("bad descriptor type");
         }
      }
   }

   pipeline->load_state = tu_cs_end_draw_state(&pipeline->cs, &cs);
}

struct tu_pipeline_builder
{
   struct tu_device *device;
   void *mem_ctx;
   struct vk_pipeline_cache *cache;
   const VkAllocationCallbacks *alloc;
   const VkGraphicsPipelineCreateInfo *create_info;
   VkPipelineCreateFlags2KHR create_flags;

   struct tu_pipeline_layout layout;

   struct tu_pvtmem_config pvtmem;

   bool rasterizer_discard;
   /* these states are affectd by rasterizer_discard */
   uint8_t unscaled_input_fragcoord;

   /* Each library defines at least one piece of state in
    * VkGraphicsPipelineLibraryFlagsEXT, and libraries cannot overlap, so
    * there can be at most as many libraries as pieces of state, of which
    * there are currently 4.
    */
#define MAX_LIBRARIES 4

   unsigned num_libraries;
   struct tu_graphics_lib_pipeline *libraries[MAX_LIBRARIES];

   /* This is just the state that we are compiling now, whereas the final
    * pipeline will include the state from the libraries.
    */
   VkGraphicsPipelineLibraryFlagsEXT state;

   /* The stages we are compiling now. */
   VkShaderStageFlags active_stages;

   bool fragment_density_map;

   struct vk_graphics_pipeline_all_state all_state;
   struct vk_graphics_pipeline_state graphics_state;
};

static bool
tu_logic_op_reads_dst(VkLogicOp op)
{
   switch (op) {
   case VK_LOGIC_OP_CLEAR:
   case VK_LOGIC_OP_COPY:
   case VK_LOGIC_OP_COPY_INVERTED:
   case VK_LOGIC_OP_SET:
      return false;
   default:
      return true;
   }
}

static bool
tu_blend_state_is_dual_src(const struct vk_color_blend_state *cb)
{
   for (unsigned i = 0; i < cb->attachment_count; i++) {
      if (tu_blend_factor_is_dual_src((VkBlendFactor)cb->attachments[i].src_color_blend_factor) ||
          tu_blend_factor_is_dual_src((VkBlendFactor)cb->attachments[i].dst_color_blend_factor) ||
          tu_blend_factor_is_dual_src((VkBlendFactor)cb->attachments[i].src_alpha_blend_factor) ||
          tu_blend_factor_is_dual_src((VkBlendFactor)cb->attachments[i].dst_alpha_blend_factor))
         return true;
   }

   return false;
}

enum ir3_push_consts_type
tu_push_consts_type(const struct tu_pipeline_layout *layout,
                    const struct ir3_compiler *compiler)
{
   if (!layout->push_constant_size)
      return IR3_PUSH_CONSTS_NONE;

   if (TU_DEBUG(PUSH_CONSTS_PER_STAGE))
      return IR3_PUSH_CONSTS_PER_STAGE;

   if (tu6_shared_constants_enable(layout, compiler)) {
      return IR3_PUSH_CONSTS_SHARED;
   } else {
      if (compiler->gen >= 7) {
         return IR3_PUSH_CONSTS_SHARED_PREAMBLE;
      } else {
         return IR3_PUSH_CONSTS_PER_STAGE;
      }
   }
}

template <chip CHIP>
struct xs_config {
   uint16_t reg_sp_xs_config;
   uint16_t reg_hlsq_xs_ctrl;
};

template <chip CHIP>
static const xs_config<CHIP> xs_configs[] = {
   [MESA_SHADER_VERTEX] = {
      REG_A6XX_SP_VS_CONFIG,
      CHIP == A6XX ? REG_A6XX_HLSQ_VS_CNTL : REG_A7XX_HLSQ_VS_CNTL,
   },
   [MESA_SHADER_TESS_CTRL] = {
      REG_A6XX_SP_HS_CONFIG,
      CHIP == A6XX ? REG_A6XX_HLSQ_HS_CNTL : REG_A7XX_HLSQ_HS_CNTL,
   },
   [MESA_SHADER_TESS_EVAL] = {
      REG_A6XX_SP_DS_CONFIG,
      CHIP == A6XX ? REG_A6XX_HLSQ_DS_CNTL : REG_A7XX_HLSQ_DS_CNTL,
   },
   [MESA_SHADER_GEOMETRY] = {
      REG_A6XX_SP_GS_CONFIG,
      CHIP == A6XX ? REG_A6XX_HLSQ_GS_CNTL : REG_A7XX_HLSQ_GS_CNTL,
   },
   [MESA_SHADER_FRAGMENT] = {
      REG_A6XX_SP_FS_CONFIG,
      CHIP == A6XX ? REG_A6XX_HLSQ_FS_CNTL : REG_A7XX_HLSQ_FS_CNTL,
   },
   [MESA_SHADER_COMPUTE] = {
      REG_A6XX_SP_CS_CONFIG,
      CHIP == A6XX ? REG_A6XX_HLSQ_CS_CNTL : REG_A7XX_HLSQ_CS_CNTL,
   },
};

template <chip CHIP>
void
tu6_emit_xs_config(struct tu_cs *cs,
                   gl_shader_stage stage, /* xs->type, but xs may be NULL */
                   const struct ir3_shader_variant *xs)
{
   const struct xs_config<CHIP> *cfg = &xs_configs<CHIP>[stage];

   if (!xs) {
      /* shader stage disabled */
      tu_cs_emit_pkt4(cs, cfg->reg_sp_xs_config, 1);
      tu_cs_emit(cs, 0);

      tu_cs_emit_pkt4(cs, cfg->reg_hlsq_xs_ctrl, 1);
      tu_cs_emit(cs, 0);
      return;
   }

   tu_cs_emit_pkt4(cs, cfg->reg_sp_xs_config, 1);
   tu_cs_emit(cs, A6XX_SP_VS_CONFIG_ENABLED |
                  COND(xs->bindless_tex, A6XX_SP_VS_CONFIG_BINDLESS_TEX) |
                  COND(xs->bindless_samp, A6XX_SP_VS_CONFIG_BINDLESS_SAMP) |
                  COND(xs->bindless_ibo, A6XX_SP_VS_CONFIG_BINDLESS_IBO) |
                  COND(xs->bindless_ubo, A6XX_SP_VS_CONFIG_BINDLESS_UBO) |
                  A6XX_SP_VS_CONFIG_NTEX(xs->num_samp) |
                  A6XX_SP_VS_CONFIG_NSAMP(xs->num_samp));

   tu_cs_emit_pkt4(cs, cfg->reg_hlsq_xs_ctrl, 1);
   tu_cs_emit(cs, A6XX_HLSQ_VS_CNTL_CONSTLEN(xs->constlen) |
                     A6XX_HLSQ_VS_CNTL_ENABLED |
                     COND(xs->shader_options.push_consts_type == IR3_PUSH_CONSTS_SHARED_PREAMBLE,
                          A7XX_HLSQ_VS_CNTL_READ_IMM_SHARED_CONSTS));
}
TU_GENX(tu6_emit_xs_config);

static void
tu6_emit_dynamic_offset(struct tu_cs *cs,
                        const struct ir3_shader_variant *xs,
                        const struct tu_shader *shader,
                        const struct tu_program_state *program)
{
   const struct tu_physical_device *phys_dev = cs->device->physical_device;
   if (!xs || shader->const_state.dynamic_offset_loc == UINT32_MAX)
      return;

   tu_cs_emit_pkt7(cs, tu6_stage2opcode(xs->type), 3 + phys_dev->usable_sets);
   tu_cs_emit(cs, CP_LOAD_STATE6_0_DST_OFF(shader->const_state.dynamic_offset_loc / 4) |
              CP_LOAD_STATE6_0_STATE_TYPE(ST6_CONSTANTS) |
              CP_LOAD_STATE6_0_STATE_SRC(SS6_DIRECT) |
              CP_LOAD_STATE6_0_STATE_BLOCK(tu6_stage2shadersb(xs->type)) |
              CP_LOAD_STATE6_0_NUM_UNIT(DIV_ROUND_UP(phys_dev->usable_sets, 4)));
   tu_cs_emit(cs, CP_LOAD_STATE6_1_EXT_SRC_ADDR(0));
   tu_cs_emit(cs, CP_LOAD_STATE6_2_EXT_SRC_ADDR_HI(0));

   for (unsigned i = 0; i < phys_dev->usable_sets; i++) {
      unsigned dynamic_offset_start =
         program->dynamic_descriptor_offsets[i] / (A6XX_TEX_CONST_DWORDS * 4);
      tu_cs_emit(cs, dynamic_offset_start);
   }
}

template <chip CHIP>
void
tu6_emit_shared_consts_enable(struct tu_cs *cs, bool enable)
{
   if (CHIP == A6XX) {
      /* Enable/disable shared constants */
      tu_cs_emit_regs(cs, A6XX_HLSQ_SHARED_CONSTS(.enable = enable));
   } else {
      assert(!enable);
   }

   tu_cs_emit_regs(cs, A6XX_SP_MODE_CONTROL(.constant_demotion_enable = true,
                                            .isammode = ISAMMODE_GL,
                                            .shared_consts_enable = enable));
}
TU_GENX(tu6_emit_shared_consts_enable);

template <chip CHIP>
static void
tu6_setup_streamout(struct tu_cs *cs,
                    const struct ir3_shader_variant *v,
                    const struct ir3_shader_linkage *l)
{
   const struct ir3_stream_output_info *info = &v->stream_output;
   /* Note: 64 here comes from the HW layout of the program RAM. The program
    * for stream N is at DWORD 64 * N.
    */
#define A6XX_SO_PROG_DWORDS 64
   uint32_t prog[A6XX_SO_PROG_DWORDS * IR3_MAX_SO_STREAMS] = {};
   BITSET_DECLARE(valid_dwords, A6XX_SO_PROG_DWORDS * IR3_MAX_SO_STREAMS) = {0};

   /* TODO: streamout state should be in a non-GMEM draw state */

   /* no streamout: */
   if (info->num_outputs == 0) {
      unsigned sizedw = 4;
      if (cs->device->physical_device->info->a6xx.tess_use_shared)
         sizedw += 2;

      tu_cs_emit_pkt7(cs, CP_CONTEXT_REG_BUNCH, sizedw);
      tu_cs_emit(cs, REG_A6XX_VPC_SO_CNTL);
      tu_cs_emit(cs, 0);
      tu_cs_emit(cs, REG_A6XX_VPC_SO_STREAM_CNTL);
      tu_cs_emit(cs, 0);

      if (cs->device->physical_device->info->a6xx.tess_use_shared) {
         tu_cs_emit(cs, REG_A6XX_PC_SO_STREAM_CNTL);
         tu_cs_emit(cs, 0);
      }

      return;
   }

   for (unsigned i = 0; i < info->num_outputs; i++) {
      const struct ir3_stream_output *out = &info->output[i];
      unsigned k = out->register_index;
      unsigned idx;

      /* Skip it, if it's an output that was never assigned a register. */
      if (k >= v->outputs_count || v->outputs[k].regid == INVALID_REG)
         continue;

      /* linkage map sorted by order frag shader wants things, so
       * a bit less ideal here..
       */
      for (idx = 0; idx < l->cnt; idx++)
         if (l->var[idx].slot == v->outputs[k].slot)
            break;

      assert(idx < l->cnt);

      for (unsigned j = 0; j < out->num_components; j++) {
         unsigned c   = j + out->start_component;
         unsigned loc = l->var[idx].loc + c;
         unsigned off = j + out->dst_offset;  /* in dwords */

         assert(loc < A6XX_SO_PROG_DWORDS * 2);
         unsigned dword = out->stream * A6XX_SO_PROG_DWORDS + loc/2;
         if (loc & 1) {
            prog[dword] |= A6XX_VPC_SO_PROG_B_EN |
                           A6XX_VPC_SO_PROG_B_BUF(out->output_buffer) |
                           A6XX_VPC_SO_PROG_B_OFF(off * 4);
         } else {
            prog[dword] |= A6XX_VPC_SO_PROG_A_EN |
                           A6XX_VPC_SO_PROG_A_BUF(out->output_buffer) |
                           A6XX_VPC_SO_PROG_A_OFF(off * 4);
         }
         BITSET_SET(valid_dwords, dword);
      }
   }

   unsigned prog_count = 0;
   unsigned start, end;
   BITSET_FOREACH_RANGE(start, end, valid_dwords,
                        A6XX_SO_PROG_DWORDS * IR3_MAX_SO_STREAMS) {
      prog_count += end - start + 1;
   }

   const bool emit_pc_so_stream_cntl =
      cs->device->physical_device->info->a6xx.tess_use_shared &&
      v->type == MESA_SHADER_TESS_EVAL;

   if (emit_pc_so_stream_cntl)
      prog_count += 1;

   tu_cs_emit_pkt7(cs, CP_CONTEXT_REG_BUNCH, 10 + 2 * prog_count);
   tu_cs_emit(cs, REG_A6XX_VPC_SO_STREAM_CNTL);
   tu_cs_emit(cs, A6XX_VPC_SO_STREAM_CNTL_STREAM_ENABLE(info->streams_written) |
                  COND(info->stride[0] > 0,
                       A6XX_VPC_SO_STREAM_CNTL_BUF0_STREAM(1 + info->buffer_to_stream[0])) |
                  COND(info->stride[1] > 0,
                       A6XX_VPC_SO_STREAM_CNTL_BUF1_STREAM(1 + info->buffer_to_stream[1])) |
                  COND(info->stride[2] > 0,
                       A6XX_VPC_SO_STREAM_CNTL_BUF2_STREAM(1 + info->buffer_to_stream[2])) |
                  COND(info->stride[3] > 0,
                       A6XX_VPC_SO_STREAM_CNTL_BUF3_STREAM(1 + info->buffer_to_stream[3])));
   for (uint32_t i = 0; i < 4; i++) {
      tu_cs_emit(cs, REG_A6XX_VPC_SO_BUFFER_STRIDE(i));
      tu_cs_emit(cs, info->stride[i]);
   }
   bool first = true;
   BITSET_FOREACH_RANGE(start, end, valid_dwords,
                        A6XX_SO_PROG_DWORDS * IR3_MAX_SO_STREAMS) {
      tu_cs_emit(cs, REG_A6XX_VPC_SO_CNTL);
      tu_cs_emit(cs, COND(first, A6XX_VPC_SO_CNTL_RESET) |
                     A6XX_VPC_SO_CNTL_ADDR(start));
      for (unsigned i = start; i < end; i++) {
         tu_cs_emit(cs, REG_A6XX_VPC_SO_PROG);
         tu_cs_emit(cs, prog[i]);
      }
      first = false;
   }

   if (emit_pc_so_stream_cntl) {
      /* Possibly not tess_use_shared related, but the combination of
       * tess + xfb fails some tests if we don't emit this.
       */
      tu_cs_emit(cs, REG_A6XX_PC_SO_STREAM_CNTL);
      tu_cs_emit(cs, A6XX_PC_SO_STREAM_CNTL_STREAM_ENABLE(info->streams_written));
   }
}

static void
tu6_emit_const(struct tu_cs *cs, uint32_t opcode, uint32_t base,
               enum a6xx_state_block block, uint32_t offset,
               uint32_t size, const uint32_t *dwords) {
   assert(size % 4 == 0);

   tu_cs_emit_pkt7(cs, opcode, 3 + size);
   tu_cs_emit(cs, CP_LOAD_STATE6_0_DST_OFF(base) |
         CP_LOAD_STATE6_0_STATE_TYPE(ST6_CONSTANTS) |
         CP_LOAD_STATE6_0_STATE_SRC(SS6_DIRECT) |
         CP_LOAD_STATE6_0_STATE_BLOCK(block) |
         CP_LOAD_STATE6_0_NUM_UNIT(size / 4));

   tu_cs_emit(cs, CP_LOAD_STATE6_1_EXT_SRC_ADDR(0));
   tu_cs_emit(cs, CP_LOAD_STATE6_2_EXT_SRC_ADDR_HI(0));
   dwords = (uint32_t *)&((uint8_t *)dwords)[offset];

   tu_cs_emit_array(cs, dwords, size);
}

static void
tu6_emit_link_map(struct tu_cs *cs,
                  const struct ir3_shader_variant *producer,
                  const struct ir3_shader_variant *consumer,
                  enum a6xx_state_block sb)
{
   const struct ir3_const_state *const_state = ir3_const_state(consumer);
   uint32_t base = const_state->offsets.primitive_map;
   int size = DIV_ROUND_UP(consumer->input_size, 4);

   size = (MIN2(size + base, consumer->constlen) - base) * 4;
   if (size <= 0)
      return;

   tu6_emit_const(cs, CP_LOAD_STATE6_GEOM, base, sb, 0, size,
                         producer->output_loc);
}

static int
tu6_vpc_varying_mode(const struct ir3_shader_variant *fs,
                     const struct ir3_shader_variant *last_shader,
                     uint32_t index,
                     uint8_t *interp_mode,
                     uint8_t *ps_repl_mode)
{
   enum
   {
      INTERP_SMOOTH = 0,
      INTERP_FLAT = 1,
      INTERP_ZERO = 2,
      INTERP_ONE = 3,
   };
   enum
   {
      PS_REPL_NONE = 0,
      PS_REPL_S = 1,
      PS_REPL_T = 2,
      PS_REPL_ONE_MINUS_T = 3,
   };

   const uint32_t compmask = fs->inputs[index].compmask;

   /* NOTE: varyings are packed, so if compmask is 0xb then first, second, and
    * fourth component occupy three consecutive varying slots
    */
   int shift = 0;
   *interp_mode = 0;
   *ps_repl_mode = 0;
   if (fs->inputs[index].slot == VARYING_SLOT_PNTC) {
      if (compmask & 0x1) {
         *ps_repl_mode |= PS_REPL_S << shift;
         shift += 2;
      }
      if (compmask & 0x2) {
         *ps_repl_mode |= PS_REPL_T << shift;
         shift += 2;
      }
      if (compmask & 0x4) {
         *interp_mode |= INTERP_ZERO << shift;
         shift += 2;
      }
      if (compmask & 0x8) {
         *interp_mode |= INTERP_ONE << 6;
         shift += 2;
      }
   } else if (fs->inputs[index].slot == VARYING_SLOT_LAYER ||
              fs->inputs[index].slot == VARYING_SLOT_VIEWPORT) {
      /* If the last geometry shader doesn't statically write these, they're
       * implicitly zero and the FS is supposed to read zero.
       */
      const gl_varying_slot slot = (gl_varying_slot) fs->inputs[index].slot;
      if (ir3_find_output(last_shader, slot) < 0 &&
          (compmask & 0x1)) {
         *interp_mode |= INTERP_ZERO;
      } else {
         *interp_mode |= INTERP_FLAT;
      }
   } else if (fs->inputs[index].flat) {
      for (int i = 0; i < 4; i++) {
         if (compmask & (1 << i)) {
            *interp_mode |= INTERP_FLAT << shift;
            shift += 2;
         }
      }
   }

   return util_bitcount(compmask) * 2;
}

static void
tu6_emit_vpc_varying_modes(struct tu_cs *cs,
                           const struct ir3_shader_variant *fs,
                           const struct ir3_shader_variant *last_shader)
{
   uint32_t interp_modes[8] = { 0 };
   uint32_t ps_repl_modes[8] = { 0 };
   uint32_t interp_regs = 0;

   if (fs) {
      for (int i = -1;
           (i = ir3_next_varying(fs, i)) < (int) fs->inputs_count;) {

         /* get the mode for input i */
         uint8_t interp_mode;
         uint8_t ps_repl_mode;
         const int bits =
            tu6_vpc_varying_mode(fs, last_shader, i, &interp_mode, &ps_repl_mode);

         /* OR the mode into the array */
         const uint32_t inloc = fs->inputs[i].inloc * 2;
         uint32_t n = inloc / 32;
         uint32_t shift = inloc % 32;
         interp_modes[n] |= interp_mode << shift;
         ps_repl_modes[n] |= ps_repl_mode << shift;
         if (shift + bits > 32) {
            n++;
            shift = 32 - shift;

            interp_modes[n] |= interp_mode >> shift;
            ps_repl_modes[n] |= ps_repl_mode >> shift;
         }
         interp_regs = MAX2(interp_regs, n + 1);
      }
   }

   if (interp_regs) {
      tu_cs_emit_pkt4(cs, REG_A6XX_VPC_VARYING_INTERP_MODE(0), interp_regs);
      tu_cs_emit_array(cs, interp_modes, interp_regs);

      tu_cs_emit_pkt4(cs, REG_A6XX_VPC_VARYING_PS_REPL_MODE(0), interp_regs);
      tu_cs_emit_array(cs, ps_repl_modes, interp_regs);
   }
}

template <chip CHIP>
void
tu6_emit_vpc(struct tu_cs *cs,
             const struct ir3_shader_variant *vs,
             const struct ir3_shader_variant *hs,
             const struct ir3_shader_variant *ds,
             const struct ir3_shader_variant *gs,
             const struct ir3_shader_variant *fs)
{
   /* note: doesn't compile as static because of the array regs.. */
   const struct reg_config {
      uint16_t reg_sp_xs_out_reg;
      uint16_t reg_sp_xs_vpc_dst_reg;
      uint16_t reg_vpc_xs_pack;
      uint16_t reg_vpc_xs_clip_cntl;
      uint16_t reg_gras_xs_cl_cntl;
      uint16_t reg_pc_xs_out_cntl;
      uint16_t reg_sp_xs_primitive_cntl;
      uint16_t reg_vpc_xs_layer_cntl;
      uint16_t reg_gras_xs_layer_cntl;
   } reg_config[] = {
      [MESA_SHADER_VERTEX] = {
         REG_A6XX_SP_VS_OUT_REG(0),
         REG_A6XX_SP_VS_VPC_DST_REG(0),
         REG_A6XX_VPC_VS_PACK,
         REG_A6XX_VPC_VS_CLIP_CNTL,
         REG_A6XX_GRAS_VS_CL_CNTL,
         REG_A6XX_PC_VS_OUT_CNTL,
         REG_A6XX_SP_VS_PRIMITIVE_CNTL,
         REG_A6XX_VPC_VS_LAYER_CNTL,
         REG_A6XX_GRAS_VS_LAYER_CNTL
      },
      [MESA_SHADER_TESS_CTRL] = {
         0,
         0,
         0,
         0,
         0,
         REG_A6XX_PC_HS_OUT_CNTL,
         0,
         0,
         0
      },
      [MESA_SHADER_TESS_EVAL] = {
         REG_A6XX_SP_DS_OUT_REG(0),
         REG_A6XX_SP_DS_VPC_DST_REG(0),
         REG_A6XX_VPC_DS_PACK,
         REG_A6XX_VPC_DS_CLIP_CNTL,
         REG_A6XX_GRAS_DS_CL_CNTL,
         REG_A6XX_PC_DS_OUT_CNTL,
         REG_A6XX_SP_DS_PRIMITIVE_CNTL,
         REG_A6XX_VPC_DS_LAYER_CNTL,
         REG_A6XX_GRAS_DS_LAYER_CNTL
      },
      [MESA_SHADER_GEOMETRY] = {
         REG_A6XX_SP_GS_OUT_REG(0),
         REG_A6XX_SP_GS_VPC_DST_REG(0),
         REG_A6XX_VPC_GS_PACK,
         REG_A6XX_VPC_GS_CLIP_CNTL,
         REG_A6XX_GRAS_GS_CL_CNTL,
         REG_A6XX_PC_GS_OUT_CNTL,
         REG_A6XX_SP_GS_PRIMITIVE_CNTL,
         REG_A6XX_VPC_GS_LAYER_CNTL,
         REG_A6XX_GRAS_GS_LAYER_CNTL
      },
   };

   const struct ir3_shader_variant *last_shader;
   if (gs) {
      last_shader = gs;
   } else if (hs) {
      last_shader = ds;
   } else {
      last_shader = vs;
   }

   const struct reg_config *cfg = &reg_config[last_shader->type];

   struct ir3_shader_linkage linkage = {
      .primid_loc = 0xff,
      .clip0_loc = 0xff,
      .clip1_loc = 0xff,
   };
   if (fs)
      ir3_link_shaders(&linkage, last_shader, fs, true);

   if (last_shader->stream_output.num_outputs)
      ir3_link_stream_out(&linkage, last_shader);

   /* a6xx finds position/pointsize at the end */
   const uint32_t pointsize_regid =
      ir3_find_output_regid(last_shader, VARYING_SLOT_PSIZ);
   const uint32_t layer_regid =
      ir3_find_output_regid(last_shader, VARYING_SLOT_LAYER);
   const uint32_t view_regid =
      ir3_find_output_regid(last_shader, VARYING_SLOT_VIEWPORT);
   const uint32_t clip0_regid =
      ir3_find_output_regid(last_shader, VARYING_SLOT_CLIP_DIST0);
   const uint32_t clip1_regid =
      ir3_find_output_regid(last_shader, VARYING_SLOT_CLIP_DIST1);
   uint32_t flags_regid = gs ?
      ir3_find_output_regid(gs, VARYING_SLOT_GS_VERTEX_FLAGS_IR3) : 0;

   uint32_t pointsize_loc = 0xff, position_loc = 0xff, layer_loc = 0xff, view_loc = 0xff;

   if (layer_regid != regid(63, 0)) {
      layer_loc = linkage.max_loc;
      ir3_link_add(&linkage, VARYING_SLOT_LAYER, layer_regid, 0x1, linkage.max_loc);
   }

   if (view_regid != regid(63, 0)) {
      view_loc = linkage.max_loc;
      ir3_link_add(&linkage, VARYING_SLOT_VIEWPORT, view_regid, 0x1, linkage.max_loc);
   }

   unsigned extra_pos = 0;

   for (unsigned i = 0; i < last_shader->outputs_count; i++) {
      if (last_shader->outputs[i].slot != VARYING_SLOT_POS)
         continue;

      if (position_loc == 0xff)
         position_loc = linkage.max_loc;

      ir3_link_add(&linkage, last_shader->outputs[i].slot,
                   last_shader->outputs[i].regid,
                   0xf, position_loc + 4 * last_shader->outputs[i].view);
      extra_pos = MAX2(extra_pos, last_shader->outputs[i].view);
   }

   if (pointsize_regid != regid(63, 0)) {
      pointsize_loc = linkage.max_loc;
      ir3_link_add(&linkage, VARYING_SLOT_PSIZ, pointsize_regid, 0x1, linkage.max_loc);
   }

   uint8_t clip_cull_mask = last_shader->clip_mask | last_shader->cull_mask;

   /* Handle the case where clip/cull distances aren't read by the FS */
   uint32_t clip0_loc = linkage.clip0_loc, clip1_loc = linkage.clip1_loc;
   if (clip0_loc == 0xff && clip0_regid != regid(63, 0)) {
      clip0_loc = linkage.max_loc;
      ir3_link_add(&linkage, VARYING_SLOT_CLIP_DIST0, clip0_regid,
                   clip_cull_mask & 0xf, linkage.max_loc);
   }
   if (clip1_loc == 0xff && clip1_regid != regid(63, 0)) {
      clip1_loc = linkage.max_loc;
      ir3_link_add(&linkage, VARYING_SLOT_CLIP_DIST1, clip1_regid,
                   clip_cull_mask >> 4, linkage.max_loc);
   }

   tu6_setup_streamout<CHIP>(cs, last_shader, &linkage);

   /* The GPU hangs on some models when there are no outputs (xs_pack::CNT),
    * at least when a DS is the last stage, so add a dummy output to keep it
    * happy if there aren't any. We do this late in order to avoid emitting
    * any unused code and make sure that optimizations don't remove it.
    */
   if (linkage.cnt == 0)
      ir3_link_add(&linkage, 0, 0, 0x1, linkage.max_loc);

   /* map outputs of the last shader to VPC */
   assert(linkage.cnt <= 32);
   const uint32_t sp_out_count = DIV_ROUND_UP(linkage.cnt, 2);
   const uint32_t sp_vpc_dst_count = DIV_ROUND_UP(linkage.cnt, 4);
   uint32_t sp_out[16] = {0};
   uint32_t sp_vpc_dst[8] = {0};
   for (uint32_t i = 0; i < linkage.cnt; i++) {
      ((uint16_t *) sp_out)[i] =
         A6XX_SP_VS_OUT_REG_A_REGID(linkage.var[i].regid) |
         A6XX_SP_VS_OUT_REG_A_COMPMASK(linkage.var[i].compmask);
      ((uint8_t *) sp_vpc_dst)[i] =
         A6XX_SP_VS_VPC_DST_REG_OUTLOC0(linkage.var[i].loc);
   }

   tu_cs_emit_pkt4(cs, cfg->reg_sp_xs_out_reg, sp_out_count);
   tu_cs_emit_array(cs, sp_out, sp_out_count);

   tu_cs_emit_pkt4(cs, cfg->reg_sp_xs_vpc_dst_reg, sp_vpc_dst_count);
   tu_cs_emit_array(cs, sp_vpc_dst, sp_vpc_dst_count);

   tu_cs_emit_pkt4(cs, cfg->reg_vpc_xs_pack, 1);
   tu_cs_emit(cs, A6XX_VPC_VS_PACK_POSITIONLOC(position_loc) |
                  A6XX_VPC_VS_PACK_PSIZELOC(pointsize_loc) |
                  A6XX_VPC_VS_PACK_STRIDE_IN_VPC(linkage.max_loc) |
                  A6XX_VPC_VS_PACK_EXTRAPOS(extra_pos));

   tu_cs_emit_pkt4(cs, cfg->reg_vpc_xs_clip_cntl, 1);
   tu_cs_emit(cs, A6XX_VPC_VS_CLIP_CNTL_CLIP_MASK(clip_cull_mask) |
                  A6XX_VPC_VS_CLIP_CNTL_CLIP_DIST_03_LOC(clip0_loc) |
                  A6XX_VPC_VS_CLIP_CNTL_CLIP_DIST_47_LOC(clip1_loc));

   tu_cs_emit_pkt4(cs, cfg->reg_gras_xs_cl_cntl, 1);
   tu_cs_emit(cs, A6XX_GRAS_VS_CL_CNTL_CLIP_MASK(last_shader->clip_mask) |
                  A6XX_GRAS_VS_CL_CNTL_CULL_MASK(last_shader->cull_mask));

   const struct ir3_shader_variant *geom_shaders[] = { vs, hs, ds, gs };

   for (unsigned i = 0; i < ARRAY_SIZE(geom_shaders); i++) {
      const struct ir3_shader_variant *shader = geom_shaders[i];
      if (!shader)
         continue;

      bool primid = shader->type != MESA_SHADER_VERTEX &&
         VALIDREG(ir3_find_sysval_regid(shader, SYSTEM_VALUE_PRIMITIVE_ID));

      tu_cs_emit_pkt4(cs, reg_config[shader->type].reg_pc_xs_out_cntl, 1);
      if (shader == last_shader) {
         tu_cs_emit(cs, A6XX_PC_VS_OUT_CNTL_STRIDE_IN_VPC(linkage.max_loc) |
                        CONDREG(pointsize_regid, A6XX_PC_VS_OUT_CNTL_PSIZE) |
                        CONDREG(layer_regid, A6XX_PC_VS_OUT_CNTL_LAYER) |
                        CONDREG(view_regid, A6XX_PC_VS_OUT_CNTL_VIEW) |
                        COND(primid, A6XX_PC_VS_OUT_CNTL_PRIMITIVE_ID) |
                        A6XX_PC_VS_OUT_CNTL_CLIP_MASK(clip_cull_mask));
      } else {
         tu_cs_emit(cs, COND(primid, A6XX_PC_VS_OUT_CNTL_PRIMITIVE_ID));
      }
   }

   /* if vertex_flags somehow gets optimized out, your gonna have a bad time: */
   if (gs)
      assert(flags_regid != INVALID_REG);

   tu_cs_emit_pkt4(cs, cfg->reg_sp_xs_primitive_cntl, 1);
   tu_cs_emit(cs, A6XX_SP_VS_PRIMITIVE_CNTL_OUT(linkage.cnt) |
                  A6XX_SP_GS_PRIMITIVE_CNTL_FLAGS_REGID(flags_regid));

   tu_cs_emit_pkt4(cs, cfg->reg_vpc_xs_layer_cntl, 1);
   tu_cs_emit(cs, A6XX_VPC_VS_LAYER_CNTL_LAYERLOC(layer_loc) |
                  A6XX_VPC_VS_LAYER_CNTL_VIEWLOC(view_loc));

   tu_cs_emit_pkt4(cs, cfg->reg_gras_xs_layer_cntl, 1);
   tu_cs_emit(cs, CONDREG(layer_regid, A6XX_GRAS_GS_LAYER_CNTL_WRITES_LAYER) |
                  CONDREG(view_regid, A6XX_GRAS_GS_LAYER_CNTL_WRITES_VIEW));

   tu6_emit_vpc_varying_modes(cs, fs, last_shader);
}
TU_GENX(tu6_emit_vpc);

static void
tu6_emit_vs_params(struct tu_cs *cs,
                   const struct ir3_const_state *const_state,
                   unsigned constlen,
                   unsigned param_stride,
                   unsigned num_vertices)
{
   uint32_t vs_params[4] = {
      param_stride * num_vertices * 4,  /* vs primitive stride */
      param_stride * 4,                 /* vs vertex stride */
      0,
      0,
   };
   uint32_t vs_base = const_state->offsets.primitive_param;
   tu6_emit_const(cs, CP_LOAD_STATE6_GEOM, vs_base, SB6_VS_SHADER, 0,
                  ARRAY_SIZE(vs_params), vs_params);
}

static void
tu_get_tess_iova(struct tu_device *dev,
                 uint64_t *tess_factor_iova,
                 uint64_t *tess_param_iova)
{
   /* Create the shared tess factor BO the first time tess is used on the device. */
   if (!dev->tess_bo) {
      mtx_lock(&dev->mutex);
      if (!dev->tess_bo)
         tu_bo_init_new(dev, &dev->tess_bo, TU_TESS_BO_SIZE, TU_BO_ALLOC_NO_FLAGS, "tess");
      mtx_unlock(&dev->mutex);
   }

   *tess_factor_iova = dev->tess_bo->iova;
   *tess_param_iova = dev->tess_bo->iova + TU_TESS_FACTOR_SIZE;
}

static const enum mesa_vk_dynamic_graphics_state tu_patch_control_points_state[] = {
   MESA_VK_DYNAMIC_TS_PATCH_CONTROL_POINTS,
};

template <chip CHIP>
static unsigned
tu6_patch_control_points_size(struct tu_device *dev,
                              const struct tu_shader *vs,
                              const struct tu_shader *tcs,
                              const struct tu_shader *tes,
                              const struct tu_program_state *program,
                              uint32_t patch_control_points)
{
#define EMIT_CONST_DWORDS(const_dwords) (4 + const_dwords)
   return EMIT_CONST_DWORDS(4) +
      EMIT_CONST_DWORDS(program->hs_param_dwords) + 2 + 2 + 2;
#undef EMIT_CONST_DWORDS
}

template <chip CHIP>
void
tu6_emit_patch_control_points(struct tu_cs *cs,
                              const struct tu_shader *vs,
                              const struct tu_shader *tcs,
                              const struct tu_shader *tes,
                              const struct tu_program_state *program,
                              uint32_t patch_control_points)
{
   if (!tcs->variant)
      return;

   struct tu_device *dev = cs->device;

   tu6_emit_vs_params(cs,
                      &program->link[MESA_SHADER_VERTEX].const_state,
                      program->link[MESA_SHADER_VERTEX].constlen,
                      vs->variant->output_size,
                      patch_control_points);

   uint64_t tess_factor_iova, tess_param_iova;
   tu_get_tess_iova(dev, &tess_factor_iova, &tess_param_iova);

   uint32_t hs_params[8] = {
      vs->variant->output_size * patch_control_points * 4,  /* hs primitive stride */
      vs->variant->output_size * 4,                         /* hs vertex stride */
      tcs->variant->output_size,
      patch_control_points,
      tess_param_iova,
      tess_param_iova >> 32,
      tess_factor_iova,
      tess_factor_iova >> 32,
   };

   const struct ir3_const_state *hs_const =
      &program->link[MESA_SHADER_TESS_CTRL].const_state;
   uint32_t hs_base = hs_const->offsets.primitive_param;
   tu6_emit_const(cs, CP_LOAD_STATE6_GEOM, hs_base, SB6_HS_SHADER, 0,
                  program->hs_param_dwords, hs_params);

   uint32_t patch_local_mem_size_16b =
      patch_control_points * vs->variant->output_size / 4;

   /* Total attribute slots in HS incoming patch. */
   tu_cs_emit_pkt4(cs, REG_A6XX_PC_HS_INPUT_SIZE, 1);
   tu_cs_emit(cs, patch_local_mem_size_16b);

   const uint32_t wavesize = 64;
   const uint32_t vs_hs_local_mem_size = 16384;

   uint32_t max_patches_per_wave;
   if (dev->physical_device->info->a6xx.tess_use_shared) {
      /* HS invocations for a patch are always within the same wave,
       * making barriers less expensive. VS can't have barriers so we
       * don't care about VS invocations being in the same wave.
       */
      max_patches_per_wave = wavesize / tcs->variant->tess.tcs_vertices_out;
   } else {
      /* VS is also in the same wave */
      max_patches_per_wave =
         wavesize / MAX2(patch_control_points,
                         tcs->variant->tess.tcs_vertices_out);
   }

   uint32_t patches_per_wave =
      MIN2(vs_hs_local_mem_size / (patch_local_mem_size_16b * 16),
           max_patches_per_wave);

   uint32_t wave_input_size = DIV_ROUND_UP(
      patches_per_wave * patch_local_mem_size_16b * 16, 256);

   tu_cs_emit_pkt4(cs, REG_A6XX_SP_HS_WAVE_INPUT_SIZE, 1);
   tu_cs_emit(cs, wave_input_size);

   /* maximum number of patches that can fit in tess factor/param buffers */
   uint32_t subdraw_size = MIN2(TU_TESS_FACTOR_SIZE / ir3_tess_factor_stride(tes->variant->key.tessellation),
                        TU_TESS_PARAM_SIZE / (tcs->variant->output_size * 4));
   /* convert from # of patches to draw count */
   subdraw_size *= patch_control_points;

   tu_cs_emit_pkt7(cs, CP_SET_SUBDRAW_SIZE, 1);
   tu_cs_emit(cs, subdraw_size);
}

static void
tu6_emit_geom_tess_consts(struct tu_cs *cs,
                          const struct ir3_shader_variant *vs,
                          const struct ir3_shader_variant *hs,
                          const struct ir3_shader_variant *ds,
                          const struct ir3_shader_variant *gs)
{
   struct tu_device *dev = cs->device;

   if (gs && !hs) {
      tu6_emit_vs_params(cs, ir3_const_state(vs), vs->constlen,
                         vs->output_size, gs->gs.vertices_in);
   }

   if (hs) {
      uint64_t tess_factor_iova, tess_param_iova;
      tu_get_tess_iova(dev, &tess_factor_iova, &tess_param_iova);

      uint32_t ds_params[8] = {
         gs ? ds->output_size * gs->gs.vertices_in * 4 : 0,  /* ds primitive stride */
         ds->output_size * 4,                                /* ds vertex stride */
         hs->output_size,                                    /* hs vertex stride (dwords) */
         hs->tess.tcs_vertices_out,
         tess_param_iova,
         tess_param_iova >> 32,
         tess_factor_iova,
         tess_factor_iova >> 32,
      };

      uint32_t ds_base = ds->const_state->offsets.primitive_param;
      uint32_t ds_param_dwords = MIN2((ds->constlen - ds_base) * 4, ARRAY_SIZE(ds_params));
      tu6_emit_const(cs, CP_LOAD_STATE6_GEOM, ds_base, SB6_DS_SHADER, 0,
                     ds_param_dwords, ds_params);
   }

   if (gs) {
      const struct ir3_shader_variant *prev = ds ? ds : vs;
      uint32_t gs_params[4] = {
         prev->output_size * gs->gs.vertices_in * 4,  /* gs primitive stride */
         prev->output_size * 4,                 /* gs vertex stride */
         0,
         0,
      };
      uint32_t gs_base = gs->const_state->offsets.primitive_param;
      tu6_emit_const(cs, CP_LOAD_STATE6_GEOM, gs_base, SB6_GS_SHADER, 0,
                     ARRAY_SIZE(gs_params), gs_params);
   }
}

template <chip CHIP>
static void
tu6_emit_program_config(struct tu_cs *cs,
                        const struct tu_program_state *prog,
                        struct tu_shader **shaders,
                        const struct ir3_shader_variant **variants)
{
   STATIC_ASSERT(MESA_SHADER_VERTEX == 0);

   bool shared_consts_enable =
      prog->shared_consts.type == IR3_PUSH_CONSTS_SHARED;
   tu6_emit_shared_consts_enable<CHIP>(cs, shared_consts_enable);

   tu_cs_emit_regs(cs, HLSQ_INVALIDATE_CMD(CHIP,
         .vs_state = true,
         .hs_state = true,
         .ds_state = true,
         .gs_state = true,
         .fs_state = true,
         .gfx_ibo = true,
         .gfx_shared_const = shared_consts_enable));
   for (size_t stage_idx = MESA_SHADER_VERTEX;
        stage_idx <= MESA_SHADER_FRAGMENT; stage_idx++) {
      gl_shader_stage stage = (gl_shader_stage) stage_idx;
      tu6_emit_xs_config<CHIP>(cs, stage, variants[stage]);
   }

   for (size_t stage_idx = MESA_SHADER_VERTEX;
        stage_idx <= MESA_SHADER_FRAGMENT; stage_idx++) {
      gl_shader_stage stage = (gl_shader_stage) stage_idx;
      tu6_emit_dynamic_offset(cs, variants[stage], shaders[stage], prog);
   }

   const struct ir3_shader_variant *vs = variants[MESA_SHADER_VERTEX];
   const struct ir3_shader_variant *hs = variants[MESA_SHADER_TESS_CTRL];
   const struct ir3_shader_variant *ds = variants[MESA_SHADER_TESS_EVAL];
   const struct ir3_shader_variant *gs = variants[MESA_SHADER_GEOMETRY];

   if (hs) {
      tu6_emit_link_map(cs, vs, hs, SB6_HS_SHADER);
      tu6_emit_link_map(cs, hs, ds, SB6_DS_SHADER);
   }

   if (gs) {
      if (hs) {
         tu6_emit_link_map(cs, ds, gs, SB6_GS_SHADER);
      } else {
         tu6_emit_link_map(cs, vs, gs, SB6_GS_SHADER);
      }

      uint32_t prev_stage_output_size = ds ? ds->output_size : vs->output_size;

      if (CHIP == A6XX) {
         /* Size of per-primitive alloction in ldlw memory in vec4s. */
         uint32_t vec4_size = gs->gs.vertices_in *
                              DIV_ROUND_UP(prev_stage_output_size, 4);

         tu_cs_emit_pkt4(cs, REG_A6XX_PC_PRIMITIVE_CNTL_6, 1);
         tu_cs_emit(cs, A6XX_PC_PRIMITIVE_CNTL_6_STRIDE_IN_VPC(vec4_size));
      }

      uint32_t prim_size = prev_stage_output_size;
      if (prim_size > 64)
         prim_size = 64;
      else if (prim_size == 64)
         prim_size = 63;
      tu_cs_emit_pkt4(cs, REG_A6XX_SP_GS_PRIM_SIZE, 1);
      tu_cs_emit(cs, prim_size);
   }

   if (gs || hs) {
      tu6_emit_geom_tess_consts(cs, vs, hs, ds, gs);
   }
}

static bool
contains_all_shader_state(VkGraphicsPipelineLibraryFlagsEXT state)
{
   return (state &
      (VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT |
       VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT)) ==
      (VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT |
       VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT);
}

static bool
pipeline_contains_all_shader_state(struct tu_pipeline *pipeline)
{
   return pipeline->type == TU_PIPELINE_GRAPHICS ||
      pipeline->type == TU_PIPELINE_COMPUTE ||
      contains_all_shader_state(tu_pipeline_to_graphics_lib(pipeline)->state);
}

/* Return true if this pipeline contains all of the GPL stages listed but none
 * of the libraries it uses do, so this is "the first time" that all of them
 * are defined together. This is useful for state that needs to be combined
 * from multiple GPL stages.
 */

static bool
set_combined_state(struct tu_pipeline_builder *builder,
                   struct tu_pipeline *pipeline,
                   VkGraphicsPipelineLibraryFlagsEXT state)
{
   if (pipeline->type == TU_PIPELINE_GRAPHICS_LIB &&
       (tu_pipeline_to_graphics_lib(pipeline)->state & state) != state)
      return false;

   for (unsigned i = 0; i < builder->num_libraries; i++) {
      if ((builder->libraries[i]->state & state) == state)
         return false;
   }

   return true;
}

#define TU6_EMIT_VERTEX_INPUT_MAX_DWORDS (MAX_VERTEX_ATTRIBS * 2 + 1)

static VkResult
tu_pipeline_allocate_cs(struct tu_device *dev,
                        struct tu_pipeline *pipeline,
                        struct tu_pipeline_layout *layout,
                        struct tu_pipeline_builder *builder,
                        const struct ir3_shader_variant *compute)
{
   uint32_t size = 1024;

   /* graphics case: */
   if (builder) {
      if (builder->state &
          VK_GRAPHICS_PIPELINE_LIBRARY_VERTEX_INPUT_INTERFACE_BIT_EXT) {
         size += TU6_EMIT_VERTEX_INPUT_MAX_DWORDS;
      }

      if (set_combined_state(builder, pipeline,
                             VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT |
                             VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT)) {
         size += tu6_load_state_size(pipeline, layout);
      }
   } else {
      size += tu6_load_state_size(pipeline, layout);
   }

   /* Allocate the space for the pipeline out of the device's RO suballocator.
    *
    * Sub-allocating BOs saves memory and also kernel overhead in refcounting of
    * BOs at exec time.
    *
    * The pipeline cache would seem like a natural place to stick the
    * suballocator, except that it is not guaranteed to outlive the pipelines
    * created from it, so you can't store any long-lived state there, and you
    * can't use its EXTERNALLY_SYNCHRONIZED flag to avoid atomics because
    * pipeline destroy isn't synchronized by the cache.
    */
   mtx_lock(&dev->pipeline_mutex);
   VkResult result = tu_suballoc_bo_alloc(&pipeline->bo, &dev->pipeline_suballoc,
                                          size * 4, 128);
   mtx_unlock(&dev->pipeline_mutex);
   if (result != VK_SUCCESS)
      return result;

   tu_cs_init_suballoc(&pipeline->cs, dev, &pipeline->bo);

   return VK_SUCCESS;
}

static void
tu_append_executable(struct tu_pipeline *pipeline,
                     const struct ir3_shader_variant *variant,
                     char *nir_from_spirv)
{
   struct tu_pipeline_executable exe = {
      .stage = variant->type,
      .stats = variant->info,
      .is_binning = variant->binning_pass,
      .nir_from_spirv = nir_from_spirv,
      .nir_final = ralloc_strdup(pipeline->executables_mem_ctx, variant->disasm_info.nir),
      .disasm = ralloc_strdup(pipeline->executables_mem_ctx, variant->disasm_info.disasm),
   };

   util_dynarray_append(&pipeline->executables, struct tu_pipeline_executable, exe);
}

static void
tu_hash_stage(struct mesa_sha1 *ctx,
              const VkPipelineShaderStageCreateInfo *stage,
              const nir_shader *nir,
              const struct tu_shader_key *key)
{

   if (nir) {
      struct blob blob;
      blob_init(&blob);
      nir_serialize(&blob, nir, true);
      _mesa_sha1_update(ctx, blob.data, blob.size);
      blob_finish(&blob);
   } else {
      unsigned char stage_hash[SHA1_DIGEST_LENGTH];
      vk_pipeline_hash_shader_stage(stage, NULL, stage_hash);
      _mesa_sha1_update(ctx, stage_hash, sizeof(stage_hash));
   }
   _mesa_sha1_update(ctx, key, sizeof(*key));
}

/* Hash flags which can affect ir3 shader compilation which aren't known until
 * logical device creation.
 */
static void
tu_hash_compiler(struct mesa_sha1 *ctx, const struct ir3_compiler *compiler)
{
   _mesa_sha1_update(ctx, &compiler->options.robust_buffer_access2,
                     sizeof(compiler->options.robust_buffer_access2));
   _mesa_sha1_update(ctx, &ir3_shader_debug, sizeof(ir3_shader_debug));
}

static void
tu_hash_shaders(unsigned char *hash,
                const VkPipelineShaderStageCreateInfo **stages,
                nir_shader *const *nir,
                const struct tu_pipeline_layout *layout,
                const struct tu_shader_key *keys,
                VkGraphicsPipelineLibraryFlagsEXT state,
                const struct ir3_compiler *compiler)
{
   struct mesa_sha1 ctx;

   _mesa_sha1_init(&ctx);

   if (layout)
      _mesa_sha1_update(&ctx, layout->sha1, sizeof(layout->sha1));

   for (int i = 0; i < MESA_SHADER_STAGES; ++i) {
      if (stages[i] || nir[i]) {
         tu_hash_stage(&ctx, stages[i], nir[i], &keys[i]);
      }
   }
   _mesa_sha1_update(&ctx, &state, sizeof(state));
   tu_hash_compiler(&ctx, compiler);
   _mesa_sha1_final(&ctx, hash);
}

static void
tu_hash_compute(unsigned char *hash,
                const VkPipelineShaderStageCreateInfo *stage,
                const struct tu_pipeline_layout *layout,
                const struct tu_shader_key *key,
                const struct ir3_compiler *compiler)
{
   struct mesa_sha1 ctx;

   _mesa_sha1_init(&ctx);

   if (layout)
      _mesa_sha1_update(&ctx, layout->sha1, sizeof(layout->sha1));

   tu_hash_stage(&ctx, stage, NULL, key);

   tu_hash_compiler(&ctx, compiler);
   _mesa_sha1_final(&ctx, hash);
}

static struct tu_shader *
tu_pipeline_cache_lookup(struct vk_pipeline_cache *cache,
                         const void *key_data, size_t key_size,
                         bool *application_cache_hit)
{
   struct vk_pipeline_cache_object *object =
      vk_pipeline_cache_lookup_object(cache, key_data, key_size,
                                      &tu_shader_ops, application_cache_hit);
   if (object)
      return container_of(object, struct tu_shader, base);
   else
      return NULL;
}

static struct tu_shader *
tu_pipeline_cache_insert(struct vk_pipeline_cache *cache,
                         struct tu_shader *shader)
{
   struct vk_pipeline_cache_object *object =
      vk_pipeline_cache_add_object(cache, &shader->base);
   return container_of(object, struct tu_shader, base);
}

static bool
tu_nir_shaders_serialize(struct vk_pipeline_cache_object *object,
                         struct blob *blob);

static struct vk_pipeline_cache_object *
tu_nir_shaders_deserialize(struct vk_pipeline_cache *cache,
                           const void *key_data,
                           size_t key_size,
                           struct blob_reader *blob);

static void
tu_nir_shaders_destroy(struct vk_device *device,
                       struct vk_pipeline_cache_object *object)
{
   struct tu_nir_shaders *shaders =
      container_of(object, struct tu_nir_shaders, base);

   for (unsigned i = 0; i < ARRAY_SIZE(shaders->nir); i++)
      ralloc_free(shaders->nir[i]);

   vk_pipeline_cache_object_finish(&shaders->base);
   vk_free(&device->alloc, shaders);
}

const struct vk_pipeline_cache_object_ops tu_nir_shaders_ops = {
   .serialize = tu_nir_shaders_serialize,
   .deserialize = tu_nir_shaders_deserialize,
   .destroy = tu_nir_shaders_destroy,
};

static struct tu_nir_shaders *
tu_nir_shaders_init(struct tu_device *dev, const void *key_data, size_t key_size)
{
   VK_MULTIALLOC(ma);
   VK_MULTIALLOC_DECL(&ma, struct tu_nir_shaders, shaders, 1);
   VK_MULTIALLOC_DECL_SIZE(&ma, char, obj_key_data, key_size);

   if (!vk_multialloc_zalloc(&ma, &dev->vk.alloc,
                             VK_SYSTEM_ALLOCATION_SCOPE_DEVICE))
      return NULL;

   memcpy(obj_key_data, key_data, key_size);
   vk_pipeline_cache_object_init(&dev->vk, &shaders->base,
                                 &tu_nir_shaders_ops, obj_key_data, key_size);

   return shaders;
}

static bool
tu_nir_shaders_serialize(struct vk_pipeline_cache_object *object,
                         struct blob *blob)
{
   struct tu_nir_shaders *shaders =
      container_of(object, struct tu_nir_shaders, base);

   for (unsigned i = 0; i < ARRAY_SIZE(shaders->nir); i++) {
      if (shaders->nir[i]) {
         blob_write_uint8(blob, 1);
         nir_serialize(blob, shaders->nir[i], true);
      } else {
         blob_write_uint8(blob, 0);
      }
   }

   return true;
}

static struct vk_pipeline_cache_object *
tu_nir_shaders_deserialize(struct vk_pipeline_cache *cache,
                           const void *key_data,
                           size_t key_size,
                           struct blob_reader *blob)
{
   struct tu_device *dev =
      container_of(cache->base.device, struct tu_device, vk);
   struct tu_nir_shaders *shaders =
      tu_nir_shaders_init(dev, key_data, key_size);

   if (!shaders)
      return NULL;

   for (unsigned i = 0; i < ARRAY_SIZE(shaders->nir); i++) {
      if (blob_read_uint8(blob)) {
         shaders->nir[i] =
            nir_deserialize(NULL, ir3_get_compiler_options(dev->compiler), blob);
      }
   }

   return &shaders->base;
}

static struct tu_nir_shaders *
tu_nir_cache_lookup(struct vk_pipeline_cache *cache,
                    const void *key_data, size_t key_size,
                    bool *application_cache_hit)
{
   struct vk_pipeline_cache_object *object =
      vk_pipeline_cache_lookup_object(cache, key_data, key_size,
                                      &tu_nir_shaders_ops, application_cache_hit);
   if (object)
      return container_of(object, struct tu_nir_shaders, base);
   else
      return NULL;
}

static struct tu_nir_shaders *
tu_nir_cache_insert(struct vk_pipeline_cache *cache,
                    struct tu_nir_shaders *shaders)
{
   struct vk_pipeline_cache_object *object =
      vk_pipeline_cache_add_object(cache, &shaders->base);
   return container_of(object, struct tu_nir_shaders, base);
}

static VkResult
tu_pipeline_builder_compile_shaders(struct tu_pipeline_builder *builder,
                                    struct tu_pipeline *pipeline)
{
   VkResult result = VK_SUCCESS;
   const struct ir3_compiler *compiler = builder->device->compiler;
   const VkPipelineShaderStageCreateInfo *stage_infos[MESA_SHADER_STAGES] = {
      NULL
   };
   VkPipelineCreationFeedback pipeline_feedback = {
      .flags = VK_PIPELINE_CREATION_FEEDBACK_VALID_BIT,
   };
   VkPipelineCreationFeedback stage_feedbacks[MESA_SHADER_STAGES] = { 0 };

   const bool executable_info =
      builder->create_flags &
      VK_PIPELINE_CREATE_2_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR;

   bool retain_nir =
      builder->create_flags &
      VK_PIPELINE_CREATE_2_RETAIN_LINK_TIME_OPTIMIZATION_INFO_BIT_EXT;

   int64_t pipeline_start = os_time_get_nano();

   const VkPipelineCreationFeedbackCreateInfo *creation_feedback =
      vk_find_struct_const(builder->create_info->pNext, PIPELINE_CREATION_FEEDBACK_CREATE_INFO);

   bool must_compile = false;
   for (uint32_t i = 0; i < builder->create_info->stageCount; i++) {
      if (!(builder->active_stages & builder->create_info->pStages[i].stage))
         continue;

      gl_shader_stage stage =
         vk_to_mesa_shader_stage(builder->create_info->pStages[i].stage);
      stage_infos[stage] = &builder->create_info->pStages[i];
      must_compile = true;
   }

   /* Forward declare everything due to the goto usage */
   nir_shader *nir[ARRAY_SIZE(stage_infos)] = { NULL };
   struct tu_shader *shaders[ARRAY_SIZE(stage_infos)] = { NULL };
   nir_shader *post_link_nir[ARRAY_SIZE(nir)] = { NULL };
   char *nir_initial_disasm[ARRAY_SIZE(stage_infos)] = { NULL };
   bool cache_hit = false;

   struct tu_shader_key keys[ARRAY_SIZE(stage_infos)] = { };
   for (gl_shader_stage stage = MESA_SHADER_VERTEX;
        stage < ARRAY_SIZE(keys); stage = (gl_shader_stage) (stage+1)) {
      const VkPipelineShaderStageRequiredSubgroupSizeCreateInfo *subgroup_info = NULL;
      if (stage_infos[stage])
         subgroup_info = vk_find_struct_const(stage_infos[stage],
                                              PIPELINE_SHADER_STAGE_REQUIRED_SUBGROUP_SIZE_CREATE_INFO);
      bool allow_varying_subgroup_size =
         !stage_infos[stage] ||
         (stage_infos[stage]->flags &
          VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT);
      bool require_full_subgroups =
         stage_infos[stage] &&
         (stage_infos[stage]->flags &
          VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT);
      tu_shader_key_subgroup_size(&keys[stage], allow_varying_subgroup_size,
                                  require_full_subgroups, subgroup_info,
                                  builder->device);
   }

   if (builder->create_flags &
       VK_PIPELINE_CREATE_2_LINK_TIME_OPTIMIZATION_BIT_EXT) {
      for (unsigned i = 0; i < builder->num_libraries; i++) {
         struct tu_graphics_lib_pipeline *library = builder->libraries[i];

         for (unsigned j = 0; j < ARRAY_SIZE(library->shaders); j++) {
            if (library->shaders[j].nir) {
               assert(!nir[j]);
               nir[j] = nir_shader_clone(builder->mem_ctx,
                     library->shaders[j].nir);
               keys[j] = library->shaders[j].key;
               must_compile = true;
            }
         }
      }
   }

   struct tu_nir_shaders *nir_shaders = NULL;
   if (!must_compile)
      goto done;

   if (builder->state &
       VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT) {
      keys[MESA_SHADER_VERTEX].multiview_mask =
         builder->graphics_state.rp->view_mask;
   }

   if (builder->state & VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT) {
      keys[MESA_SHADER_FRAGMENT].multiview_mask =
         builder->graphics_state.rp->view_mask;
      keys[MESA_SHADER_FRAGMENT].fragment_density_map =
         builder->fragment_density_map;
      keys[MESA_SHADER_FRAGMENT].unscaled_input_fragcoord =
         builder->unscaled_input_fragcoord;

      const VkPipelineMultisampleStateCreateInfo *msaa_info =
         builder->create_info->pMultisampleState;

      /* The 1.3.215 spec says:
       *
       *    Sample shading can be used to specify a minimum number of unique
       *    samples to process for each fragment. If sample shading is enabled,
       *    an implementation must provide a minimum of
       *
       *       max(ceil(minSampleShadingFactor * totalSamples), 1)
       *
       *    unique associated data for each fragment, where
       *    minSampleShadingFactor is the minimum fraction of sample shading.
       *
       * The definition is pretty much the same as OpenGL's GL_SAMPLE_SHADING.
       * They both require unique associated data.
       *
       * There are discussions to change the definition, such that
       * sampleShadingEnable does not imply unique associated data.  Before the
       * discussions are settled and before apps (i.e., ANGLE) are fixed to
       * follow the new and incompatible definition, we should stick to the
       * current definition.
       *
       * Note that ir3_shader_key::sample_shading is not actually used by ir3,
       * just checked in tu6_emit_fs_inputs.  We will also copy the value to
       * tu_shader_key::force_sample_interp in a bit.
       */
      keys[MESA_SHADER_FRAGMENT].force_sample_interp =
         !builder->rasterizer_discard && msaa_info && msaa_info->sampleShadingEnable;
   }

   unsigned char pipeline_sha1[20];
   tu_hash_shaders(pipeline_sha1, stage_infos, nir, &builder->layout, keys,
                   builder->state, compiler);

   unsigned char nir_sha1[21];
   memcpy(nir_sha1, pipeline_sha1, sizeof(pipeline_sha1));
   nir_sha1[20] = 'N';

   if (!executable_info) {
      cache_hit = true;
      bool application_cache_hit = false;

      unsigned char shader_sha1[21];
      memcpy(shader_sha1, pipeline_sha1, sizeof(pipeline_sha1));
      
      for (gl_shader_stage stage = MESA_SHADER_VERTEX; stage < ARRAY_SIZE(nir);
           stage = (gl_shader_stage) (stage + 1)) {
         if (stage_infos[stage] || nir[stage]) {
            bool shader_application_cache_hit;
            shader_sha1[20] = (unsigned char) stage;
            shaders[stage] =
               tu_pipeline_cache_lookup(builder->cache, &shader_sha1,
                                        sizeof(shader_sha1),
                                        &shader_application_cache_hit);
            if (!shaders[stage]) {
               cache_hit = false;
               break;
            }
            application_cache_hit &= shader_application_cache_hit;
         }
      }

      /* If the user asks us to keep the NIR around, we need to have it for a
       * successful cache hit. If we only have a "partial" cache hit, then we
       * still need to recompile in order to get the NIR.
       */
      if (cache_hit &&
          (builder->create_flags &
           VK_PIPELINE_CREATE_2_RETAIN_LINK_TIME_OPTIMIZATION_INFO_BIT_EXT)) {
         bool nir_application_cache_hit = false;
         nir_shaders =
            tu_nir_cache_lookup(builder->cache, &nir_sha1,
                                sizeof(nir_sha1),
                                &nir_application_cache_hit);

         application_cache_hit &= nir_application_cache_hit;
         cache_hit &= !!nir_shaders;
      }

      if (application_cache_hit && builder->cache != builder->device->mem_cache) {
         pipeline_feedback.flags |=
            VK_PIPELINE_CREATION_FEEDBACK_APPLICATION_PIPELINE_CACHE_HIT_BIT;
      }
   }

   if (!cache_hit) {
      if (builder->create_flags &
          VK_PIPELINE_CREATE_2_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT_KHR) {
         return VK_PIPELINE_COMPILE_REQUIRED;
      }

      result = tu_compile_shaders(builder->device,
                                  stage_infos,
                                  nir,
                                  keys,
                                  &builder->layout,
                                  pipeline_sha1,
                                  shaders,
                                  executable_info ? nir_initial_disasm : NULL,
                                  pipeline->executables_mem_ctx,
                                  retain_nir ? post_link_nir : NULL,
                                  stage_feedbacks);

      if (result != VK_SUCCESS)
         goto fail;

      if (retain_nir) {
         nir_shaders =
            tu_nir_shaders_init(builder->device, &nir_sha1, sizeof(nir_sha1));
         for (gl_shader_stage stage = MESA_SHADER_VERTEX;
              stage < ARRAY_SIZE(nir); stage = (gl_shader_stage) (stage + 1)) {
            if (!post_link_nir[stage])
               continue;

            nir_shaders->nir[stage] = post_link_nir[stage];
         }

         nir_shaders = tu_nir_cache_insert(builder->cache, nir_shaders);
      }

      for (gl_shader_stage stage = MESA_SHADER_VERTEX; stage < ARRAY_SIZE(nir);
           stage = (gl_shader_stage) (stage + 1)) {
         if (!nir[stage])
            continue;

         shaders[stage] = tu_pipeline_cache_insert(builder->cache, shaders[stage]);
      }
   }

done:

   /* Create empty shaders which contain the draw states to initialize
    * registers for unused shader stages.
    */
   if (builder->state &
       VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT) {
      if (!shaders[MESA_SHADER_TESS_CTRL]) {
         shaders[MESA_SHADER_TESS_CTRL] = builder->device->empty_tcs;
         vk_pipeline_cache_object_ref(&shaders[MESA_SHADER_TESS_CTRL]->base);
      }
      if (!shaders[MESA_SHADER_TESS_EVAL]) {
         shaders[MESA_SHADER_TESS_EVAL] = builder->device->empty_tes;
         vk_pipeline_cache_object_ref(&shaders[MESA_SHADER_TESS_EVAL]->base);
      }
      if (!shaders[MESA_SHADER_GEOMETRY]) {
         shaders[MESA_SHADER_GEOMETRY] = builder->device->empty_gs;
         vk_pipeline_cache_object_ref(&shaders[MESA_SHADER_GEOMETRY]->base);
      }
   }

   if (builder->state &
       VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT) {
      if (!shaders[MESA_SHADER_FRAGMENT]) {
         shaders[MESA_SHADER_FRAGMENT] =
            builder->fragment_density_map ?
            builder->device->empty_fs_fdm : builder->device->empty_fs;
         vk_pipeline_cache_object_ref(&shaders[MESA_SHADER_FRAGMENT]->base);
      }
   }

   for (gl_shader_stage stage = MESA_SHADER_VERTEX;
        stage < ARRAY_SIZE(nir); stage = (gl_shader_stage) (stage + 1)) {
      if (shaders[stage] && shaders[stage]->variant) {
         tu_append_executable(pipeline, shaders[stage]->variant,
                              nir_initial_disasm[stage]);
      }
   }

   /* We may have deduplicated a cache entry, in which case our original
    * post_link_nir may be gone.
    */
   if (nir_shaders) {
      for (gl_shader_stage stage = MESA_SHADER_VERTEX;
           stage < ARRAY_SIZE(nir); stage = (gl_shader_stage) (stage + 1)) {
         if (nir_shaders->nir[stage]) {
            post_link_nir[stage] = nir_shaders->nir[stage];
         }
      }
   }
   
   /* In the case where we're building a library without link-time
    * optimization but with sub-libraries that retain LTO info, we should
    * retain it ourselves in case another pipeline includes us with LTO.
    */
   for (unsigned i = 0; i < builder->num_libraries; i++) {
      struct tu_graphics_lib_pipeline *library = builder->libraries[i];
      for (gl_shader_stage stage = MESA_SHADER_VERTEX;
           stage < ARRAY_SIZE(library->shaders);
           stage = (gl_shader_stage) (stage + 1)) {
         if (!post_link_nir[stage] && library->shaders[stage].nir) {
            post_link_nir[stage] = library->shaders[stage].nir;
            keys[stage] = library->shaders[stage].key;
         }

         if (!shaders[stage] && library->base.shaders[stage]) {
            shaders[stage] = library->base.shaders[stage];
            vk_pipeline_cache_object_ref(&shaders[stage]->base);
         }
      }
   }

   if (shaders[MESA_SHADER_VERTEX]) {
      const struct ir3_shader_variant *vs =
         shaders[MESA_SHADER_VERTEX]->variant;

      if (!vs->stream_output.num_outputs && ir3_has_binning_vs(&vs->key)) {
         tu_append_executable(pipeline, vs->binning, NULL);
      }
   }

   if (pipeline_contains_all_shader_state(pipeline)) {
      /* It doesn't make much sense to use RETAIN_LINK_TIME_OPTIMIZATION_INFO
       * when compiling all stages, but make sure we don't leak.
       */
      if (nir_shaders)
         vk_pipeline_cache_object_unref(&builder->device->vk,
                                        &nir_shaders->base);
   } else {
      struct tu_graphics_lib_pipeline *library =
         tu_pipeline_to_graphics_lib(pipeline);
      library->nir_shaders = nir_shaders;
      for (gl_shader_stage stage = MESA_SHADER_VERTEX;
           stage < ARRAY_SIZE(library->shaders);
           stage = (gl_shader_stage) (stage + 1)) {
         library->shaders[stage].nir = post_link_nir[stage];
         library->shaders[stage].key = keys[stage];
      }
   }

   for (gl_shader_stage stage = MESA_SHADER_VERTEX;
        stage < ARRAY_SIZE(shaders); stage = (gl_shader_stage) (stage + 1)) {
      pipeline->shaders[stage] = shaders[stage];
      if (shaders[stage])
         pipeline->active_desc_sets |= shaders[stage]->active_desc_sets;
   }

   pipeline_feedback.duration = os_time_get_nano() - pipeline_start;
   if (creation_feedback) {
      *creation_feedback->pPipelineCreationFeedback = pipeline_feedback;

      for (uint32_t i = 0; i < builder->create_info->stageCount; i++) {
         gl_shader_stage s =
            vk_to_mesa_shader_stage(builder->create_info->pStages[i].stage);
         creation_feedback->pPipelineStageCreationFeedbacks[i] = stage_feedbacks[s];
      }
   }

   return VK_SUCCESS;

fail:
   if (nir_shaders)
      vk_pipeline_cache_object_unref(&builder->device->vk,
                                     &nir_shaders->base);

   return result;
}

static void
tu_pipeline_builder_parse_libraries(struct tu_pipeline_builder *builder,
                                    struct tu_pipeline *pipeline)
{
   const VkPipelineLibraryCreateInfoKHR *library_info =
      vk_find_struct_const(builder->create_info->pNext,
                           PIPELINE_LIBRARY_CREATE_INFO_KHR);

   if (library_info) {
      assert(library_info->libraryCount <= MAX_LIBRARIES);
      builder->num_libraries = library_info->libraryCount;
      for (unsigned i = 0; i < library_info->libraryCount; i++) {
         TU_FROM_HANDLE(tu_pipeline, library, library_info->pLibraries[i]);
         builder->libraries[i] = tu_pipeline_to_graphics_lib(library);
      }
   }

   /* Merge in the state from libraries. The program state is a bit special
    * and is handled separately.
    */
   if (pipeline->type == TU_PIPELINE_GRAPHICS_LIB)
      tu_pipeline_to_graphics_lib(pipeline)->state = builder->state;
   for (unsigned i = 0; i < builder->num_libraries; i++) {
      struct tu_graphics_lib_pipeline *library = builder->libraries[i];
      if (pipeline->type == TU_PIPELINE_GRAPHICS_LIB)
         tu_pipeline_to_graphics_lib(pipeline)->state |= library->state;

      if (library->state &
          VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT) {
         pipeline->output = library->base.output;
         pipeline->lrz_blend.reads_dest |= library->base.lrz_blend.reads_dest;
         pipeline->lrz_blend.valid |= library->base.lrz_blend.valid;
         pipeline->prim_order = library->base.prim_order;
      }

      if ((library->state &
           VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT) &&
          (library->state &
           VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT)) {
         pipeline->prim_order = library->base.prim_order;
      }

      pipeline->set_state_mask |= library->base.set_state_mask;

      u_foreach_bit (i, library->base.set_state_mask) {
         pipeline->dynamic_state[i] = library->base.dynamic_state[i];
      }

      if (contains_all_shader_state(library->state)) {
         pipeline->program = library->base.program;
         pipeline->load_state = library->base.load_state;
         for (unsigned i = 0; i < ARRAY_SIZE(pipeline->shaders); i++) {
            if (library->base.shaders[i]) {
               pipeline->shaders[i] = library->base.shaders[i];
               vk_pipeline_cache_object_ref(&pipeline->shaders[i]->base);
            }
         }
      }

      BITSET_OR(pipeline->static_state_mask, pipeline->static_state_mask,
                library->base.static_state_mask);

      vk_graphics_pipeline_state_merge(&builder->graphics_state,
                                       &library->graphics_state);
   }
}

static void
tu_pipeline_builder_parse_layout(struct tu_pipeline_builder *builder,
                                 struct tu_pipeline *pipeline)
{
   TU_FROM_HANDLE(tu_pipeline_layout, layout, builder->create_info->layout);

   if (layout) {
      /* Note: it's still valid to have a layout even if there are libraries.
       * This allows the app to e.g. overwrite an INDEPENDENT_SET layout with
       * a non-INDEPENDENT_SET layout which may make us use a faster path,
       * currently this just affects dynamic offset descriptors.
       */
      builder->layout = *layout;
   } else {
      for (unsigned i = 0; i < builder->num_libraries; i++) {
         struct tu_graphics_lib_pipeline *library = builder->libraries[i];
         builder->layout.num_sets = MAX2(builder->layout.num_sets,
                                         library->num_sets);
         assert(builder->layout.num_sets <= builder->device->physical_device->usable_sets);
         for (unsigned j = 0; j < library->num_sets; j++) {
            builder->layout.set[i].layout = library->layouts[i];
         }

         builder->layout.push_constant_size = library->push_constant_size;
      }

      tu_pipeline_layout_init(&builder->layout);
   }

   if (pipeline->type == TU_PIPELINE_GRAPHICS_LIB) {
      struct tu_graphics_lib_pipeline *library =
         tu_pipeline_to_graphics_lib(pipeline);
      library->num_sets = builder->layout.num_sets;
      for (unsigned i = 0; i < library->num_sets; i++) {
         library->layouts[i] = builder->layout.set[i].layout;
         if (library->layouts[i])
            vk_descriptor_set_layout_ref(&library->layouts[i]->vk);
      }
      library->push_constant_size = builder->layout.push_constant_size;
   }
}

static void
tu_pipeline_set_linkage(struct tu_program_descriptor_linkage *link,
                        struct tu_const_state *const_state,
                        const struct ir3_shader_variant *v)
{
   link->const_state = *ir3_const_state(v);
   link->tu_const_state = *const_state;
   link->constlen = v->constlen;
}

template <chip CHIP>
static void
tu_emit_program_state(struct tu_cs *sub_cs,
                      struct tu_program_state *prog,
                      struct tu_shader **shaders)
{
   struct tu_device *dev = sub_cs->device;
   struct tu_cs prog_cs;

   const struct ir3_shader_variant *variants[MESA_SHADER_STAGES];
   struct tu_draw_state draw_states[MESA_SHADER_STAGES];
   
   for (gl_shader_stage stage = MESA_SHADER_VERTEX;
        stage < ARRAY_SIZE(variants); stage = (gl_shader_stage) (stage+1)) {
      variants[stage] = shaders[stage] ? shaders[stage]->variant : NULL;
   }

   uint32_t safe_variants =
      ir3_trim_constlen(variants, dev->compiler);

   unsigned dynamic_descriptor_sizes[MAX_SETS] = { };

   for (gl_shader_stage stage = MESA_SHADER_VERTEX;
        stage < ARRAY_SIZE(variants); stage = (gl_shader_stage) (stage+1)) {
      if (shaders[stage]) {
         if (safe_variants & (1u << stage)) {
            variants[stage] = shaders[stage]->safe_const_variant;
            draw_states[stage] = shaders[stage]->safe_const_state;
         } else {
            draw_states[stage] = shaders[stage]->state;
         }

         for (unsigned i = 0; i < MAX_SETS; i++) {
            if (shaders[stage]->dynamic_descriptor_sizes[i] >= 0) {
               dynamic_descriptor_sizes[i] =
                  shaders[stage]->dynamic_descriptor_sizes[i];
            }
         }
      }
   }

   for (unsigned i = 0; i < ARRAY_SIZE(variants); i++) {
      if (!variants[i])
         continue;

      tu_pipeline_set_linkage(&prog->link[i],
                              &shaders[i]->const_state,
                              variants[i]);

      struct tu_push_constant_range *push_consts =
         &shaders[i]->const_state.push_consts;
      if (push_consts->type == IR3_PUSH_CONSTS_SHARED ||
          push_consts->type == IR3_PUSH_CONSTS_SHARED_PREAMBLE) {
         prog->shared_consts = *push_consts;
      }
   }

   unsigned dynamic_descriptor_offset = 0;
   for (unsigned i = 0; i < MAX_SETS; i++) {
      prog->dynamic_descriptor_offsets[i] = dynamic_descriptor_offset;
      dynamic_descriptor_offset += dynamic_descriptor_sizes[i];
   }

   /* Emit HLSQ_xS_CNTL/HLSQ_SP_xS_CONFIG *first*, before emitting anything
    * else that could depend on that state (like push constants)
    *
    * Note also that this always uses the full VS even in binning pass.  The
    * binning pass variant has the same const layout as the full VS, and
    * the constlen for the VS will be the same or greater than the constlen
    * for the binning pass variant.  It is required that the constlen state
    * matches between binning and draw passes, as some parts of the push
    * consts are emitted in state groups that are shared between the binning
    * and draw passes.
    */
   tu_cs_begin_sub_stream(sub_cs, 512, &prog_cs);
   tu6_emit_program_config<CHIP>(&prog_cs, prog, shaders, variants);
   prog->config_state = tu_cs_end_draw_state(sub_cs, &prog_cs);

   prog->vs_state = draw_states[MESA_SHADER_VERTEX];

  /* Don't use the binning pass variant when GS is present because we don't
   * support compiling correct binning pass variants with GS.
   */
   if (variants[MESA_SHADER_GEOMETRY]) {
      prog->vs_binning_state = prog->vs_state;
   } else {
      prog->vs_binning_state =
         shaders[MESA_SHADER_VERTEX]->binning_state;
   }

   prog->hs_state = draw_states[MESA_SHADER_TESS_CTRL];
   prog->ds_state = draw_states[MESA_SHADER_TESS_EVAL];
   prog->gs_state = draw_states[MESA_SHADER_GEOMETRY];
   prog->gs_binning_state =
      shaders[MESA_SHADER_GEOMETRY]->binning_state;
   prog->fs_state = draw_states[MESA_SHADER_FRAGMENT];

   const struct ir3_shader_variant *vs = variants[MESA_SHADER_VERTEX];
   const struct ir3_shader_variant *hs = variants[MESA_SHADER_TESS_CTRL];
   const struct ir3_shader_variant *ds = variants[MESA_SHADER_TESS_EVAL];
   const struct ir3_shader_variant *gs = variants[MESA_SHADER_GEOMETRY];
   const struct ir3_shader_variant *fs = variants[MESA_SHADER_FRAGMENT];

   tu_cs_begin_sub_stream(sub_cs, 512, &prog_cs);
   tu6_emit_vpc<CHIP>(&prog_cs, vs, hs, ds, gs, fs);
   prog->vpc_state = tu_cs_end_draw_state(sub_cs, &prog_cs);
   
   if (hs) {
      const struct ir3_const_state *hs_const =
         &prog->link[MESA_SHADER_TESS_CTRL].const_state;
      unsigned hs_constlen =
         prog->link[MESA_SHADER_TESS_CTRL].constlen;
      uint32_t hs_base = hs_const->offsets.primitive_param;
      prog->hs_param_dwords = MIN2((hs_constlen - hs_base) * 4, 8);
   }

   const struct ir3_shader_variant *last_shader;
   if (gs)
      last_shader = gs;
   else if (ds)
      last_shader = ds;
   else
      last_shader = vs;

   prog->per_view_viewport =
      !last_shader->writes_viewport &&
      shaders[MESA_SHADER_FRAGMENT]->fs.has_fdm &&
      dev->physical_device->info->a6xx.has_per_view_viewport;
}

static const enum mesa_vk_dynamic_graphics_state tu_vertex_input_state[] = {
   MESA_VK_DYNAMIC_VI,
};

template <chip CHIP>
static unsigned
tu6_vertex_input_size(struct tu_device *dev,
                      const struct vk_vertex_input_state *vi)
{
   return 1 + 2 * util_last_bit(vi->attributes_valid);
}

template <chip CHIP>
static void
tu6_emit_vertex_input(struct tu_cs *cs,
                      const struct vk_vertex_input_state *vi)
{
   unsigned attr_count = util_last_bit(vi->attributes_valid);
   if (attr_count != 0)
      tu_cs_emit_pkt4(cs, REG_A6XX_VFD_DECODE_INSTR(0), attr_count * 2);

   for (uint32_t loc = 0; loc < attr_count; loc++) {
      const struct vk_vertex_attribute_state *attr = &vi->attributes[loc];

      if (vi->attributes_valid & (1u << loc)) {
         const struct vk_vertex_binding_state *binding =
            &vi->bindings[attr->binding];

         enum pipe_format pipe_format = vk_format_to_pipe_format(attr->format);
         const struct tu_native_format format = tu6_format_vtx(pipe_format);
         tu_cs_emit(cs, A6XX_VFD_DECODE_INSTR(0,
                          .idx = attr->binding,
                          .offset = attr->offset,
                          .instanced = binding->input_rate == VK_VERTEX_INPUT_RATE_INSTANCE,
                          .format = format.fmt,
                          .swap = format.swap,
                          .unk30 = 1,
                          ._float = !util_format_is_pure_integer(pipe_format)).value);
         tu_cs_emit(cs, A6XX_VFD_DECODE_STEP_RATE(0, binding->divisor).value);
      } else {
         tu_cs_emit(cs, 0);
         tu_cs_emit(cs, 0);
      }
   }
}

static const enum mesa_vk_dynamic_graphics_state tu_vertex_stride_state[] = {
   MESA_VK_DYNAMIC_VI_BINDINGS_VALID,
   MESA_VK_DYNAMIC_VI_BINDING_STRIDES,
};

template <chip CHIP>
static unsigned
tu6_vertex_stride_size(struct tu_device *dev,
                       const struct vk_vertex_input_state *vi)
{
   return 1 + 2 * util_last_bit(vi->bindings_valid);
}

template <chip CHIP>
static void
tu6_emit_vertex_stride(struct tu_cs *cs, const struct vk_vertex_input_state *vi)
{
   if (vi->bindings_valid) {
      unsigned bindings_count = util_last_bit(vi->bindings_valid);
      tu_cs_emit_pkt7(cs, CP_CONTEXT_REG_BUNCH, 2 * bindings_count);
      for (unsigned i = 0; i < bindings_count; i++) {
         tu_cs_emit(cs, REG_A6XX_VFD_FETCH_STRIDE(i));
         tu_cs_emit(cs, vi->bindings[i].stride);
      }
   }
}

template <chip CHIP>
static unsigned
tu6_vertex_stride_size_dyn(struct tu_device *dev,
                           const uint16_t *vi_binding_stride,
                           uint32_t bindings_valid)
{
   return 1 + 2 * util_last_bit(bindings_valid);
}

template <chip CHIP>
static void
tu6_emit_vertex_stride_dyn(struct tu_cs *cs, const uint16_t *vi_binding_stride,
                           uint32_t bindings_valid)
{
   if (bindings_valid) {
      unsigned bindings_count = util_last_bit(bindings_valid);
      tu_cs_emit_pkt7(cs, CP_CONTEXT_REG_BUNCH, 2 * bindings_count);
      for (unsigned i = 0; i < bindings_count; i++) {
         tu_cs_emit(cs, REG_A6XX_VFD_FETCH_STRIDE(i));
         tu_cs_emit(cs, vi_binding_stride[i]);
      }
   }
}

static const enum mesa_vk_dynamic_graphics_state tu_viewport_state[] = {
   MESA_VK_DYNAMIC_VP_VIEWPORTS,
   MESA_VK_DYNAMIC_VP_VIEWPORT_COUNT,
   MESA_VK_DYNAMIC_VP_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE,
};

template <chip CHIP>
static unsigned
tu6_viewport_size(struct tu_device *dev, const struct vk_viewport_state *vp)
{
   return 1 + vp->viewport_count * 6 + 1 + vp->viewport_count * 2 +
      1 + vp->viewport_count * 2 + 5;
}

template <chip CHIP>
static void
tu6_emit_viewport(struct tu_cs *cs, const struct vk_viewport_state *vp)
{
   VkExtent2D guardband = {511, 511};

   tu_cs_emit_pkt4(cs, REG_A6XX_GRAS_CL_VPORT_XOFFSET(0), vp->viewport_count * 6);
   for (uint32_t i = 0; i < vp->viewport_count; i++) {
      const VkViewport *viewport = &vp->viewports[i];
      float offsets[3];
      float scales[3];
      scales[0] = viewport->width / 2.0f;
      scales[1] = viewport->height / 2.0f;
      if (vp->depth_clip_negative_one_to_one) {
         scales[2] = 0.5 * (viewport->maxDepth - viewport->minDepth);
      } else {
         scales[2] = viewport->maxDepth - viewport->minDepth;
      }

      offsets[0] = viewport->x + scales[0];
      offsets[1] = viewport->y + scales[1];
      if (vp->depth_clip_negative_one_to_one) {
         offsets[2] = 0.5 * (viewport->minDepth + viewport->maxDepth);
      } else {
         offsets[2] = viewport->minDepth;
      }

      for (uint32_t j = 0; j < 3; j++) {
         tu_cs_emit(cs, fui(offsets[j]));
         tu_cs_emit(cs, fui(scales[j]));
      }

      guardband.width =
         MIN2(guardband.width, fd_calc_guardband(offsets[0], scales[0], false));
      guardband.height =
         MIN2(guardband.height, fd_calc_guardband(offsets[1], scales[1], false));
   }

   tu_cs_emit_pkt4(cs, REG_A6XX_GRAS_SC_VIEWPORT_SCISSOR_TL(0), vp->viewport_count * 2);
   for (uint32_t i = 0; i < vp->viewport_count; i++) {
      const VkViewport *viewport = &vp->viewports[i];
      VkOffset2D min;
      VkOffset2D max;
      min.x = (int32_t) viewport->x;
      max.x = (int32_t) ceilf(viewport->x + viewport->width);
      if (viewport->height >= 0.0f) {
         min.y = (int32_t) viewport->y;
         max.y = (int32_t) ceilf(viewport->y + viewport->height);
      } else {
         min.y = (int32_t)(viewport->y + viewport->height);
         max.y = (int32_t) ceilf(viewport->y);
      }
      /* the spec allows viewport->height to be 0.0f */
      if (min.y == max.y)
         max.y++;
      /* allow viewport->width = 0.0f for un-initialized viewports: */
      if (min.x == max.x)
         max.x++;

      min.x = MAX2(min.x, 0);
      min.y = MAX2(min.y, 0);
      max.x = MAX2(max.x, 1);
      max.y = MAX2(max.y, 1);

      assert(min.x < max.x);
      assert(min.y < max.y);

      tu_cs_emit(cs, A6XX_GRAS_SC_VIEWPORT_SCISSOR_TL_X(min.x) |
                     A6XX_GRAS_SC_VIEWPORT_SCISSOR_TL_Y(min.y));
      tu_cs_emit(cs, A6XX_GRAS_SC_VIEWPORT_SCISSOR_BR_X(max.x - 1) |
                     A6XX_GRAS_SC_VIEWPORT_SCISSOR_BR_Y(max.y - 1));
   }

   tu_cs_emit_pkt4(cs, REG_A6XX_GRAS_CL_Z_CLAMP(0), vp->viewport_count * 2);
   for (uint32_t i = 0; i < vp->viewport_count; i++) {
      const VkViewport *viewport = &vp->viewports[i];
      tu_cs_emit(cs, fui(MIN2(viewport->minDepth, viewport->maxDepth)));
      tu_cs_emit(cs, fui(MAX2(viewport->minDepth, viewport->maxDepth)));
   }
   tu_cs_emit_pkt4(cs, REG_A6XX_GRAS_CL_GUARDBAND_CLIP_ADJ, 1);
   tu_cs_emit(cs, A6XX_GRAS_CL_GUARDBAND_CLIP_ADJ_HORZ(guardband.width) |
                  A6XX_GRAS_CL_GUARDBAND_CLIP_ADJ_VERT(guardband.height));

   /* TODO: what to do about this and multi viewport ? */
   float z_clamp_min = vp->viewport_count ? MIN2(vp->viewports[0].minDepth, vp->viewports[0].maxDepth) : 0;
   float z_clamp_max = vp->viewport_count ? MAX2(vp->viewports[0].minDepth, vp->viewports[0].maxDepth) : 0;

   tu_cs_emit_regs(cs,
                   A6XX_RB_Z_CLAMP_MIN(z_clamp_min),
                   A6XX_RB_Z_CLAMP_MAX(z_clamp_max));
}

struct apply_viewport_state {
   struct vk_viewport_state vp;
   bool share_scale;
};

/* It's a hardware restriction that the window offset (i.e. bin.offset) must
 * be the same for all views. This means that GMEM coordinates cannot be a
 * simple scaling of framebuffer coordinates, because this would require us to
 * scale the window offset and the scale may be different per view. Instead we
 * have to apply a per-bin offset to the GMEM coordinate transform to make
 * sure that the window offset maps to itself. Specifically we need an offset
 * o to the transform:
 *
 * x' = s * x + o
 *
 * so that when we plug in the bin start b_s:
 * 
 * b_s = s * b_s + o
 *
 * and we get:
 *
 * o = b_s - s * b_s
 *
 * We use this form exactly, because we know the bin offset is a multiple of
 * the frag area so s * b_s is an integer and we can compute an exact result
 * easily.
 */

VkOffset2D
tu_fdm_per_bin_offset(VkExtent2D frag_area, VkRect2D bin)
{
   assert(bin.offset.x % frag_area.width == 0);
   assert(bin.offset.y % frag_area.height == 0);

   return (VkOffset2D) {
      bin.offset.x - bin.offset.x / frag_area.width,
      bin.offset.y - bin.offset.y / frag_area.height
   };
}

static void
fdm_apply_viewports(struct tu_cs *cs, void *data, VkRect2D bin, unsigned views,
                    VkExtent2D *frag_areas)
{
   const struct apply_viewport_state *state =
      (const struct apply_viewport_state *)data;

   struct vk_viewport_state vp = state->vp;

   for (unsigned i = 0; i < state->vp.viewport_count; i++) {
      /* Note: If we're using shared scaling, the scale should already be the
       * same across all views, we can pick any view. However the number
       * of viewports and number of views is not guaranteed the same, so we
       * need to pick the 0'th view which always exists to be safe.
       *
       * Conversly, if we're not using shared scaling then the rasterizer in
       * the original pipeline is using only the first viewport, so we need to
       * replicate it across all viewports.
       */
      VkExtent2D frag_area = state->share_scale ? frag_areas[0] : frag_areas[i];
      VkViewport viewport =
         state->share_scale ? state->vp.viewports[i] : state->vp.viewports[0];
      if (frag_area.width == 1 && frag_area.height == 1) {
         vp.viewports[i] = viewport;
         continue;
      }

      float scale_x = (float) 1.0f / frag_area.width;
      float scale_y = (float) 1.0f / frag_area.height;

      vp.viewports[i].minDepth = viewport.minDepth;
      vp.viewports[i].maxDepth = viewport.maxDepth;
      vp.viewports[i].width = viewport.width * scale_x;
      vp.viewports[i].height = viewport.height * scale_y;

      VkOffset2D offset = tu_fdm_per_bin_offset(frag_area, bin);

      vp.viewports[i].x = scale_x * viewport.x + offset.x;
      vp.viewports[i].y = scale_y * viewport.y + offset.y;
   }

   TU_CALLX(cs->device, tu6_emit_viewport)(cs, &vp);
}

static void
tu6_emit_viewport_fdm(struct tu_cs *cs, struct tu_cmd_buffer *cmd,
                      const struct vk_viewport_state *vp)
{
   unsigned num_views = MAX2(cmd->state.pass->num_views, 1);
   struct apply_viewport_state state = {
      .vp = *vp,
      .share_scale = !cmd->state.per_view_viewport,
   };
   if (!state.share_scale)
      state.vp.viewport_count = num_views;
   unsigned size = TU_CALLX(cmd->device, tu6_viewport_size)(cmd->device, &state.vp);
   tu_cs_begin_sub_stream(&cmd->sub_cs, size, cs);
   tu_create_fdm_bin_patchpoint(cmd, cs, size, fdm_apply_viewports, state);
}

static const enum mesa_vk_dynamic_graphics_state tu_scissor_state[] = {
   MESA_VK_DYNAMIC_VP_SCISSORS,
   MESA_VK_DYNAMIC_VP_SCISSOR_COUNT,
};

template <chip CHIP>
static unsigned
tu6_scissor_size(struct tu_device *dev, const struct vk_viewport_state *vp)
{
   return 1 + vp->scissor_count * 2;
}

template <chip CHIP>
void
tu6_emit_scissor(struct tu_cs *cs, const struct vk_viewport_state *vp)
{
   tu_cs_emit_pkt4(cs, REG_A6XX_GRAS_SC_SCREEN_SCISSOR_TL(0), vp->scissor_count * 2);

   for (uint32_t i = 0; i < vp->scissor_count; i++) {
      const VkRect2D *scissor = &vp->scissors[i];

      uint32_t min_x = scissor->offset.x;
      uint32_t min_y = scissor->offset.y;
      uint32_t max_x = min_x + scissor->extent.width - 1;
      uint32_t max_y = min_y + scissor->extent.height - 1;

      if (!scissor->extent.width || !scissor->extent.height) {
         min_x = min_y = 1;
         max_x = max_y = 0;
      } else {
         /* avoid overflow */
         uint32_t scissor_max = BITFIELD_MASK(15);
         min_x = MIN2(scissor_max, min_x);
         min_y = MIN2(scissor_max, min_y);
         max_x = MIN2(scissor_max, max_x);
         max_y = MIN2(scissor_max, max_y);
      }

      tu_cs_emit(cs, A6XX_GRAS_SC_SCREEN_SCISSOR_TL_X(min_x) |
                     A6XX_GRAS_SC_SCREEN_SCISSOR_TL_Y(min_y));
      tu_cs_emit(cs, A6XX_GRAS_SC_SCREEN_SCISSOR_BR_X(max_x) |
                     A6XX_GRAS_SC_SCREEN_SCISSOR_BR_Y(max_y));
   }
}

static void
fdm_apply_scissors(struct tu_cs *cs, void *data, VkRect2D bin, unsigned views,
                   VkExtent2D *frag_areas)
{
   const struct apply_viewport_state *state =
      (const struct apply_viewport_state *)data;

   struct vk_viewport_state vp = state->vp;

   for (unsigned i = 0; i < vp.scissor_count; i++) {
      VkExtent2D frag_area = state->share_scale ? frag_areas[0] : frag_areas[i];
      VkRect2D scissor =
         state->share_scale ? state->vp.scissors[i] : state->vp.scissors[0];
      if (frag_area.width == 1 && frag_area.height == 1) {
         vp.scissors[i] = scissor;
         continue;
      }

      /* Transform the scissor following the viewport. It's unclear how this
       * is supposed to handle cases where the scissor isn't aligned to the
       * fragment area, but we round outwards to always render partial
       * fragments if the scissor size equals the framebuffer size and it
       * isn't aligned to the fragment area.
       */
      VkOffset2D offset = tu_fdm_per_bin_offset(frag_area, bin);
      VkOffset2D min = {
         scissor.offset.x / frag_area.width + offset.x,
         scissor.offset.y / frag_area.width + offset.y,
      };
      VkOffset2D max = {
         DIV_ROUND_UP(scissor.offset.x + scissor.extent.width, frag_area.width) + offset.x,
         DIV_ROUND_UP(scissor.offset.y + scissor.extent.height, frag_area.height) + offset.y,
      };

      /* Intersect scissor with the scaled bin, this essentially replaces the
       * window scissor.
       */
      uint32_t scaled_width = bin.extent.width / frag_area.width;
      uint32_t scaled_height = bin.extent.height / frag_area.height;
      vp.scissors[i].offset.x = MAX2(min.x, bin.offset.x);
      vp.scissors[i].offset.y = MAX2(min.y, bin.offset.y);
      vp.scissors[i].extent.width =
         MIN2(max.x, bin.offset.x + scaled_width) - vp.scissors[i].offset.x;
      vp.scissors[i].extent.height =
         MIN2(max.y, bin.offset.y + scaled_height) - vp.scissors[i].offset.y;
   }

   TU_CALLX(cs->device, tu6_emit_scissor)(cs, &vp);
}

static void
tu6_emit_scissor_fdm(struct tu_cs *cs, struct tu_cmd_buffer *cmd,
                     const struct vk_viewport_state *vp)
{
   unsigned num_views = MAX2(cmd->state.pass->num_views, 1);
   struct apply_viewport_state state = {
      .vp = *vp,
      .share_scale = !cmd->state.per_view_viewport,
   };
   if (!state.share_scale)
      state.vp.scissor_count = num_views;
   unsigned size = TU_CALLX(cmd->device, tu6_scissor_size)(cmd->device, &state.vp);
   tu_cs_begin_sub_stream(&cmd->sub_cs, size, cs);
   tu_create_fdm_bin_patchpoint(cmd, cs, size, fdm_apply_scissors, state);
}

static const enum mesa_vk_dynamic_graphics_state tu_sample_locations_state[] = {
   MESA_VK_DYNAMIC_MS_SAMPLE_LOCATIONS_ENABLE,
   MESA_VK_DYNAMIC_MS_SAMPLE_LOCATIONS,
};

template <chip CHIP>
static unsigned
tu6_sample_locations_size(struct tu_device *dev, bool enable,
                          const struct vk_sample_locations_state *samp_loc)
{
   return 6 + (enable ? 6 : 0);
}

template <chip CHIP>
void
tu6_emit_sample_locations(struct tu_cs *cs, bool enable,
                          const struct vk_sample_locations_state *samp_loc)
{
   uint32_t sample_config =
      COND(enable, A6XX_RB_SAMPLE_CONFIG_LOCATION_ENABLE);

   tu_cs_emit_pkt4(cs, REG_A6XX_GRAS_SAMPLE_CONFIG, 1);
   tu_cs_emit(cs, sample_config);

   tu_cs_emit_pkt4(cs, REG_A6XX_RB_SAMPLE_CONFIG, 1);
   tu_cs_emit(cs, sample_config);

   tu_cs_emit_pkt4(cs, REG_A6XX_SP_TP_SAMPLE_CONFIG, 1);
   tu_cs_emit(cs, sample_config);

   if (!enable)
      return;

   assert(samp_loc->grid_size.width == 1);
   assert(samp_loc->grid_size.height == 1);

   uint32_t sample_locations = 0;
   for (uint32_t i = 0; i < samp_loc->per_pixel; i++) {
      /* From VkSampleLocationEXT:
       *
       *    The values specified in a VkSampleLocationEXT structure are always
       *    clamped to the implementation-dependent sample location coordinate
       *    range
       *    [sampleLocationCoordinateRange[0],sampleLocationCoordinateRange[1]]
       */
      float x = CLAMP(samp_loc->locations[i].x, SAMPLE_LOCATION_MIN,
                      SAMPLE_LOCATION_MAX);
      float y = CLAMP(samp_loc->locations[i].y, SAMPLE_LOCATION_MIN,
                      SAMPLE_LOCATION_MAX);

      sample_locations |=
         (A6XX_RB_SAMPLE_LOCATION_0_SAMPLE_0_X(x) |
          A6XX_RB_SAMPLE_LOCATION_0_SAMPLE_0_Y(y)) << i*8;
   }

   tu_cs_emit_pkt4(cs, REG_A6XX_GRAS_SAMPLE_LOCATION_0, 1);
   tu_cs_emit(cs, sample_locations);

   tu_cs_emit_pkt4(cs, REG_A6XX_RB_SAMPLE_LOCATION_0, 1);
   tu_cs_emit(cs, sample_locations);

   tu_cs_emit_pkt4(cs, REG_A6XX_SP_TP_SAMPLE_LOCATION_0, 1);
   tu_cs_emit(cs, sample_locations);
}

static const enum mesa_vk_dynamic_graphics_state tu_depth_bias_state[] = {
   MESA_VK_DYNAMIC_RS_DEPTH_BIAS_FACTORS,
};

template <chip CHIP>
static unsigned
tu6_depth_bias_size(struct tu_device *dev,
                    const struct vk_rasterization_state *rs)
{
   return 4;
}

template <chip CHIP>
void
tu6_emit_depth_bias(struct tu_cs *cs, const struct vk_rasterization_state *rs)
{
   tu_cs_emit_pkt4(cs, REG_A6XX_GRAS_SU_POLY_OFFSET_SCALE, 3);
   tu_cs_emit(cs, A6XX_GRAS_SU_POLY_OFFSET_SCALE(rs->depth_bias.slope).value);
   tu_cs_emit(cs, A6XX_GRAS_SU_POLY_OFFSET_OFFSET(rs->depth_bias.constant).value);
   tu_cs_emit(cs, A6XX_GRAS_SU_POLY_OFFSET_OFFSET_CLAMP(rs->depth_bias.clamp).value);
}

static const enum mesa_vk_dynamic_graphics_state tu_bandwidth_state[] = {
   MESA_VK_DYNAMIC_CB_LOGIC_OP_ENABLE,
   MESA_VK_DYNAMIC_CB_LOGIC_OP,
   MESA_VK_DYNAMIC_CB_ATTACHMENT_COUNT,
   MESA_VK_DYNAMIC_CB_COLOR_WRITE_ENABLES,
   MESA_VK_DYNAMIC_CB_BLEND_ENABLES,
   MESA_VK_DYNAMIC_CB_WRITE_MASKS,
};

static void
tu_calc_bandwidth(struct tu_bandwidth *bandwidth,
                  const struct vk_color_blend_state *cb,
                  const struct vk_render_pass_state *rp)
{
   bool rop_reads_dst = cb->logic_op_enable && tu_logic_op_reads_dst((VkLogicOp)cb->logic_op);

   uint32_t total_bpp = 0;
   for (unsigned i = 0; i < cb->attachment_count; i++) {
      const struct vk_color_blend_attachment_state *att = &cb->attachments[i];
      if (!(cb->color_write_enables & (1u << i)))
         continue;

      const VkFormat format = rp->color_attachment_formats[i];

      uint32_t write_bpp = 0;
      if (format == VK_FORMAT_UNDEFINED) {
         /* do nothing */
      } else if (att->write_mask == 0xf) {
         write_bpp = vk_format_get_blocksizebits(format);
      } else {
         const enum pipe_format pipe_format = vk_format_to_pipe_format(format);
         for (uint32_t i = 0; i < 4; i++) {
            if (att->write_mask & (1 << i)) {
               write_bpp += util_format_get_component_bits(pipe_format,
                     UTIL_FORMAT_COLORSPACE_RGB, i);
            }
         }
      }
      total_bpp += write_bpp;

      if (rop_reads_dst || att->blend_enable) {
         total_bpp += write_bpp;
      }
   }

   bandwidth->color_bandwidth_per_sample = total_bpp / 8;

   if (rp->attachment_aspects & VK_IMAGE_ASPECT_DEPTH_BIT) {
      bandwidth->depth_cpp_per_sample = util_format_get_component_bits(
            vk_format_to_pipe_format(rp->depth_attachment_format),
            UTIL_FORMAT_COLORSPACE_ZS, 0) / 8;
   }

   if (rp->attachment_aspects & VK_IMAGE_ASPECT_STENCIL_BIT) {
      bandwidth->stencil_cpp_per_sample = util_format_get_component_bits(
            vk_format_to_pipe_format(rp->stencil_attachment_format),
            UTIL_FORMAT_COLORSPACE_ZS, 1) / 8;
   }
}

/* Return true if the blend state reads the color attachments. */
static bool
tu6_calc_blend_lrz(const struct vk_color_blend_state *cb,
                   const struct vk_render_pass_state *rp)
{
   if (cb->logic_op_enable && tu_logic_op_reads_dst((VkLogicOp)cb->logic_op))
      return true;

   for (unsigned i = 0; i < cb->attachment_count; i++) {
      if (rp->color_attachment_formats[i] == VK_FORMAT_UNDEFINED)
         continue;

      const struct vk_color_blend_attachment_state *att = &cb->attachments[i];
      if (att->blend_enable)
         return true;
      if (!(cb->color_write_enables & (1u << i)))
         return true;
      unsigned mask =
         MASK(vk_format_get_nr_components(rp->color_attachment_formats[i]));
      if ((att->write_mask & mask) != mask)
         return true;
   }

   return false;
}

static const enum mesa_vk_dynamic_graphics_state tu_blend_lrz_state[] = {
   MESA_VK_DYNAMIC_CB_LOGIC_OP_ENABLE,
   MESA_VK_DYNAMIC_CB_LOGIC_OP,
   MESA_VK_DYNAMIC_CB_ATTACHMENT_COUNT,
   MESA_VK_DYNAMIC_CB_COLOR_WRITE_ENABLES,
   MESA_VK_DYNAMIC_CB_BLEND_ENABLES,
   MESA_VK_DYNAMIC_CB_WRITE_MASKS,
};

static void
tu_emit_blend_lrz(struct tu_lrz_blend *lrz,
                  const struct vk_color_blend_state *cb,
                  const struct vk_render_pass_state *rp)
{
   lrz->reads_dest = tu6_calc_blend_lrz(cb, rp);
   lrz->valid = true;
}

static const enum mesa_vk_dynamic_graphics_state tu_blend_state[] = {
   MESA_VK_DYNAMIC_CB_LOGIC_OP_ENABLE,
   MESA_VK_DYNAMIC_CB_LOGIC_OP,
   MESA_VK_DYNAMIC_CB_ATTACHMENT_COUNT,
   MESA_VK_DYNAMIC_CB_COLOR_WRITE_ENABLES,
   MESA_VK_DYNAMIC_CB_BLEND_ENABLES,
   MESA_VK_DYNAMIC_CB_BLEND_EQUATIONS,
   MESA_VK_DYNAMIC_CB_WRITE_MASKS,
   MESA_VK_DYNAMIC_MS_ALPHA_TO_COVERAGE_ENABLE,
   MESA_VK_DYNAMIC_MS_ALPHA_TO_ONE_ENABLE,
   MESA_VK_DYNAMIC_MS_SAMPLE_MASK,
};

template <chip CHIP>
static unsigned
tu6_blend_size(struct tu_device *dev,
               const struct vk_color_blend_state *cb,
               bool alpha_to_coverage_enable,
               bool alpha_to_one_enable,
               uint32_t sample_mask)
{
   unsigned num_rts = alpha_to_coverage_enable ?
      MAX2(cb->attachment_count, 1) : cb->attachment_count;
   return 8 + 3 * num_rts;
}

template <chip CHIP>
static void
tu6_emit_blend(struct tu_cs *cs,
               const struct vk_color_blend_state *cb,
               bool alpha_to_coverage_enable,
               bool alpha_to_one_enable,
               uint32_t sample_mask)
{
   bool rop_reads_dst = cb->logic_op_enable && tu_logic_op_reads_dst((VkLogicOp)cb->logic_op);
   enum a3xx_rop_code rop = tu6_rop((VkLogicOp)cb->logic_op);

   uint32_t blend_enable_mask = 0;
   for (unsigned i = 0; i < cb->attachment_count; i++) {
      const struct vk_color_blend_attachment_state *att = &cb->attachments[i];
      if (!(cb->color_write_enables & (1u << i)))
         continue;

      if (rop_reads_dst || att->blend_enable) {
         blend_enable_mask |= 1u << i;
      }
   }

   /* This will emit a dummy RB_MRT_*_CONTROL below if alpha-to-coverage is
    * enabled but there are no color attachments, in addition to changing
    * *_FS_OUTPUT_CNTL1.
    */
   unsigned num_rts = alpha_to_coverage_enable ?
      MAX2(cb->attachment_count, 1) : cb->attachment_count;

   bool dual_src_blend = tu_blend_state_is_dual_src(cb);

   tu_cs_emit_regs(cs, A6XX_SP_FS_OUTPUT_CNTL1(.mrt = num_rts));
   tu_cs_emit_regs(cs, A6XX_RB_FS_OUTPUT_CNTL1(.mrt = num_rts));
   tu_cs_emit_regs(cs, A6XX_SP_BLEND_CNTL(.enable_blend = blend_enable_mask,
                                          .unk8 = true,
                                          .dual_color_in_enable =
                                             dual_src_blend,
                                          .alpha_to_coverage =
                                             alpha_to_coverage_enable));
   /* set A6XX_RB_BLEND_CNTL_INDEPENDENT_BLEND only when enabled? */
   tu_cs_emit_regs(cs, A6XX_RB_BLEND_CNTL(.enable_blend = blend_enable_mask,
                                          .independent_blend = true,
                                          .dual_color_in_enable =
                                             dual_src_blend,
                                          .alpha_to_coverage =
                                             alpha_to_coverage_enable,
                                          .alpha_to_one = alpha_to_one_enable,
                                          .sample_mask = sample_mask));

   for (unsigned i = 0; i < num_rts; i++) {
      const struct vk_color_blend_attachment_state *att = &cb->attachments[i];
      if ((cb->color_write_enables & (1u << i)) && i < cb->attachment_count) {
         const enum a3xx_rb_blend_opcode color_op = tu6_blend_op(att->color_blend_op);
         const enum adreno_rb_blend_factor src_color_factor =
            tu6_blend_factor((VkBlendFactor)att->src_color_blend_factor);
         const enum adreno_rb_blend_factor dst_color_factor =
            tu6_blend_factor((VkBlendFactor)att->dst_color_blend_factor);
         const enum a3xx_rb_blend_opcode alpha_op =
            tu6_blend_op(att->alpha_blend_op);
         const enum adreno_rb_blend_factor src_alpha_factor =
            tu6_blend_factor((VkBlendFactor)att->src_alpha_blend_factor);
         const enum adreno_rb_blend_factor dst_alpha_factor =
            tu6_blend_factor((VkBlendFactor)att->dst_alpha_blend_factor);

         tu_cs_emit_regs(cs,
                         A6XX_RB_MRT_CONTROL(i,
                                             .blend = att->blend_enable,
                                             .blend2 = att->blend_enable,
                                             .rop_enable = cb->logic_op_enable,
                                             .rop_code = rop,
                                             .component_enable = att->write_mask),
                         A6XX_RB_MRT_BLEND_CONTROL(i,
                                                   .rgb_src_factor = src_color_factor,
                                                   .rgb_blend_opcode = color_op,
                                                   .rgb_dest_factor = dst_color_factor,
                                                   .alpha_src_factor = src_alpha_factor,
                                                   .alpha_blend_opcode = alpha_op,
                                                   .alpha_dest_factor = dst_alpha_factor));
      } else {
            tu_cs_emit_regs(cs,
                            A6XX_RB_MRT_CONTROL(i,),
                            A6XX_RB_MRT_BLEND_CONTROL(i,));
      }
   }
}

static const enum mesa_vk_dynamic_graphics_state tu_blend_constants_state[] = {
   MESA_VK_DYNAMIC_CB_BLEND_CONSTANTS,
};

template <chip CHIP>
static unsigned
tu6_blend_constants_size(struct tu_device *dev,
                         const struct vk_color_blend_state *cb)
{
   return 5;
}

template <chip CHIP>
static void
tu6_emit_blend_constants(struct tu_cs *cs, const struct vk_color_blend_state *cb)
{
   tu_cs_emit_pkt4(cs, REG_A6XX_RB_BLEND_RED_F32, 4);
   tu_cs_emit_array(cs, (const uint32_t *) cb->blend_constants, 4);
}

static const enum mesa_vk_dynamic_graphics_state tu_rast_state[] = {
   MESA_VK_DYNAMIC_RS_DEPTH_CLAMP_ENABLE,
   MESA_VK_DYNAMIC_RS_DEPTH_CLIP_ENABLE,
   MESA_VK_DYNAMIC_RS_POLYGON_MODE,
   MESA_VK_DYNAMIC_RS_CULL_MODE,
   MESA_VK_DYNAMIC_RS_FRONT_FACE,
   MESA_VK_DYNAMIC_RS_DEPTH_BIAS_ENABLE,
   MESA_VK_DYNAMIC_RS_LINE_MODE,
   MESA_VK_DYNAMIC_RS_RASTERIZER_DISCARD_ENABLE,
   MESA_VK_DYNAMIC_RS_RASTERIZATION_STREAM,
   MESA_VK_DYNAMIC_VP_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE,
};

template <chip CHIP>
uint32_t
tu6_rast_size(struct tu_device *dev,
              const struct vk_rasterization_state *rs,
              const struct vk_viewport_state *vp,
              bool multiview,
              bool per_view_viewport)
{
   if (CHIP == A6XX) {
      return 15 + (dev->physical_device->info->a6xx.has_shading_rate ? 8 : 0);
   } else {
      return 15;
   }
}

template <chip CHIP>
void
tu6_emit_rast(struct tu_cs *cs,
              const struct vk_rasterization_state *rs,
              const struct vk_viewport_state *vp,
              bool multiview,
              bool per_view_viewport)
{
   enum a5xx_line_mode line_mode =
      rs->line.mode == VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT ?
      BRESENHAM : RECTANGULAR;
   tu_cs_emit_regs(cs,
                   A6XX_GRAS_SU_CNTL(
                     .cull_front = rs->cull_mode & VK_CULL_MODE_FRONT_BIT,
                     .cull_back = rs->cull_mode & VK_CULL_MODE_BACK_BIT,
                     .front_cw = rs->front_face == VK_FRONT_FACE_CLOCKWISE,
                     .linehalfwidth = rs->line.width / 2.0f,
                     .poly_offset = rs->depth_bias.enable,
                     .line_mode = line_mode,
                     .multiview_enable = multiview,
                     .rendertargetindexincr = multiview,
                     .viewportindexincr = multiview && per_view_viewport));

   bool depth_clip_enable = vk_rasterization_state_depth_clip_enable(rs);

   tu_cs_emit_regs(cs,
                   A6XX_GRAS_CL_CNTL(
                     .znear_clip_disable = !depth_clip_enable,
                     .zfar_clip_disable = !depth_clip_enable,
                     .z_clamp_enable = rs->depth_clamp_enable,
                     .zero_gb_scale_z = vp->depth_clip_negative_one_to_one ? 0 : 1,
                     .vp_clip_code_ignore = 1));;

   enum a6xx_polygon_mode polygon_mode = tu6_polygon_mode(rs->polygon_mode);

   tu_cs_emit_regs(cs,
                   A6XX_VPC_POLYGON_MODE(polygon_mode));

   tu_cs_emit_regs(cs,
                   PC_POLYGON_MODE(CHIP, polygon_mode));

   if (CHIP == A7XX) {
      tu_cs_emit_regs(cs,
                     A7XX_VPC_POLYGON_MODE2(polygon_mode));
   }

   tu_cs_emit_regs(cs, PC_RASTER_CNTL(CHIP,
      .stream = rs->rasterization_stream,
      .discard = rs->rasterizer_discard_enable));
   if (CHIP == A6XX) {
      tu_cs_emit_regs(cs, A6XX_VPC_UNKNOWN_9107(
         .raster_discard = rs->rasterizer_discard_enable));
   }

   /* move to hw ctx init? */
   tu_cs_emit_regs(cs,
                   A6XX_GRAS_SU_POINT_MINMAX(.min = 1.0f / 16.0f, .max = 4092.0f),
                   A6XX_GRAS_SU_POINT_SIZE(1.0f));

   if (CHIP == A6XX && cs->device->physical_device->info->a6xx.has_shading_rate) {
      tu_cs_emit_regs(cs, A6XX_RB_UNKNOWN_8A00());
      tu_cs_emit_regs(cs, A6XX_RB_UNKNOWN_8A10());
      tu_cs_emit_regs(cs, A6XX_RB_UNKNOWN_8A20());
      tu_cs_emit_regs(cs, A6XX_RB_UNKNOWN_8A30());
   }
}

static const enum mesa_vk_dynamic_graphics_state tu_ds_state[] = {
   MESA_VK_DYNAMIC_DS_STENCIL_TEST_ENABLE,
   MESA_VK_DYNAMIC_DS_STENCIL_OP,
   MESA_VK_DYNAMIC_DS_STENCIL_COMPARE_MASK,
   MESA_VK_DYNAMIC_DS_STENCIL_WRITE_MASK,
   MESA_VK_DYNAMIC_DS_STENCIL_REFERENCE,
   MESA_VK_DYNAMIC_DS_DEPTH_BOUNDS_TEST_BOUNDS,
};

template <chip CHIP>
static unsigned
tu6_ds_size(struct tu_device *dev,
                 const struct vk_depth_stencil_state *ds)
{
   return 11;
}

template <chip CHIP>
static void
tu6_emit_ds(struct tu_cs *cs,
            const struct vk_depth_stencil_state *ds)
{
   tu_cs_emit_regs(cs, A6XX_RB_STENCIL_CONTROL(
      .stencil_enable = ds->stencil.test_enable,
      .stencil_enable_bf = ds->stencil.test_enable,
      .stencil_read = ds->stencil.test_enable,
      .func = tu6_compare_func((VkCompareOp)ds->stencil.front.op.compare),
      .fail = tu6_stencil_op((VkStencilOp)ds->stencil.front.op.fail),
      .zpass = tu6_stencil_op((VkStencilOp)ds->stencil.front.op.pass),
      .zfail = tu6_stencil_op((VkStencilOp)ds->stencil.front.op.depth_fail),
      .func_bf = tu6_compare_func((VkCompareOp)ds->stencil.back.op.compare),
      .fail_bf = tu6_stencil_op((VkStencilOp)ds->stencil.back.op.fail),
      .zpass_bf = tu6_stencil_op((VkStencilOp)ds->stencil.back.op.pass),
      .zfail_bf = tu6_stencil_op((VkStencilOp)ds->stencil.back.op.depth_fail)));

   tu_cs_emit_regs(cs, A6XX_RB_STENCILMASK(
      .mask = ds->stencil.front.compare_mask,
      .bfmask = ds->stencil.back.compare_mask));

   tu_cs_emit_regs(cs, A6XX_RB_STENCILWRMASK(
      .wrmask = ds->stencil.front.write_mask,
      .bfwrmask = ds->stencil.back.write_mask));

   tu_cs_emit_regs(cs, A6XX_RB_STENCILREF(
      .ref = ds->stencil.front.reference,
      .bfref = ds->stencil.back.reference));

   tu_cs_emit_regs(cs,
                   A6XX_RB_Z_BOUNDS_MIN(ds->depth.bounds_test.min),
                   A6XX_RB_Z_BOUNDS_MAX(ds->depth.bounds_test.max));
}

static const enum mesa_vk_dynamic_graphics_state tu_rb_depth_cntl_state[] = {
   MESA_VK_DYNAMIC_DS_DEPTH_TEST_ENABLE,
   MESA_VK_DYNAMIC_DS_DEPTH_WRITE_ENABLE,
   MESA_VK_DYNAMIC_DS_DEPTH_COMPARE_OP,
   MESA_VK_DYNAMIC_DS_DEPTH_BOUNDS_TEST_ENABLE,
   MESA_VK_DYNAMIC_RS_DEPTH_CLAMP_ENABLE,
};

template <chip CHIP>
static unsigned
tu6_rb_depth_cntl_size(struct tu_device *dev,
                       const struct vk_depth_stencil_state *ds,
                       const struct vk_render_pass_state *rp,
                       const struct vk_rasterization_state *rs)
{
   return 2;
}

template <chip CHIP>
static void
tu6_emit_rb_depth_cntl(struct tu_cs *cs,
                       const struct vk_depth_stencil_state *ds,
                       const struct vk_render_pass_state *rp,
                       const struct vk_rasterization_state *rs)
{
   if (rp->attachment_aspects & VK_IMAGE_ASPECT_DEPTH_BIT) {
      bool depth_test = ds->depth.test_enable;
      enum adreno_compare_func zfunc = tu6_compare_func(ds->depth.compare_op);

      /* On some GPUs it is necessary to enable z test for depth bounds test
       * when UBWC is enabled. Otherwise, the GPU would hang. FUNC_ALWAYS is
       * required to pass z test. Relevant tests:
       *  dEQP-VK.pipeline.extended_dynamic_state.two_draws_dynamic.depth_bounds_test_disable
       *  dEQP-VK.dynamic_state.ds_state.depth_bounds_1
       */
      if (ds->depth.bounds_test.enable &&
          !ds->depth.test_enable &&
          cs->device->physical_device->info->a6xx.depth_bounds_require_depth_test_quirk) {
         depth_test = true;
         zfunc = FUNC_ALWAYS;
      }

      tu_cs_emit_regs(cs, A6XX_RB_DEPTH_CNTL(
         .z_test_enable = depth_test,
         .z_write_enable = ds->depth.test_enable && ds->depth.write_enable,
         .zfunc = zfunc,
         .z_clamp_enable = rs->depth_clamp_enable,
         /* TODO don't set for ALWAYS/NEVER */
         .z_read_enable = ds->depth.test_enable || ds->depth.bounds_test.enable,
         .z_bounds_enable = ds->depth.bounds_test.enable));
   } else {
      tu_cs_emit_regs(cs, A6XX_RB_DEPTH_CNTL());
   }
}

static inline bool
emit_pipeline_state(BITSET_WORD *keep, BITSET_WORD *remove,
                    BITSET_WORD *pipeline_set,
                    const enum mesa_vk_dynamic_graphics_state *state_array,
                    unsigned num_states, bool extra_cond,
                    struct tu_pipeline_builder *builder)
{
   BITSET_DECLARE(state, MESA_VK_DYNAMIC_GRAPHICS_STATE_ENUM_MAX) = {};

   /* Unrolling this loop should produce a constant value once the function is
    * inlined, because state_array and num_states are a per-draw-state
    * constant, but GCC seems to need a little encouragement. clang does a
    * little better but still needs a pragma when there are a large number of
    * states.
    */
#if defined(__clang__)
#pragma clang loop unroll(full)
#elif defined(__GNUC__) && __GNUC__ >= 8
#pragma GCC unroll MESA_VK_DYNAMIC_GRAPHICS_STATE_ENUM_MAX
#endif
   for (unsigned i = 0; i < num_states; i++) {
      BITSET_SET(state, state_array[i]);
   }

   /* If all of the state is set, then after we emit it we can tentatively
    * remove it from the states to set for the pipeline by making it dynamic.
    * If we can't emit it, though, we need to keep around the partial state so
    * that we can emit it later, even if another draw state consumes it. That
    * is, we have to cancel any tentative removal.
    */
   BITSET_DECLARE(temp, MESA_VK_DYNAMIC_GRAPHICS_STATE_ENUM_MAX);
   memcpy(temp, pipeline_set, sizeof(temp));
   BITSET_AND(temp, temp, state);
   if (!BITSET_EQUAL(temp, state) || !extra_cond) {
      __bitset_or(keep, keep, temp, ARRAY_SIZE(temp));
      return false;
   }
   __bitset_or(remove, remove, state, ARRAY_SIZE(state));
   return true;
}

template <chip CHIP>
static void
tu_pipeline_builder_emit_state(struct tu_pipeline_builder *builder,
                               struct tu_pipeline *pipeline)
{
   struct tu_cs cs;
   BITSET_DECLARE(keep, MESA_VK_DYNAMIC_GRAPHICS_STATE_ENUM_MAX) = {};
   BITSET_DECLARE(remove, MESA_VK_DYNAMIC_GRAPHICS_STATE_ENUM_MAX) = {};
   BITSET_DECLARE(pipeline_set, MESA_VK_DYNAMIC_GRAPHICS_STATE_ENUM_MAX) = {};

   vk_graphics_pipeline_get_state(&builder->graphics_state, pipeline_set);

#define EMIT_STATE(name, extra_cond)                                          \
   emit_pipeline_state(keep, remove, pipeline_set, tu_##name##_state,         \
                       ARRAY_SIZE(tu_##name##_state), extra_cond, builder)

#define DRAW_STATE_COND(name, id, extra_cond, ...)                            \
   if (EMIT_STATE(name, extra_cond)) {                                        \
      unsigned size = tu6_##name##_size<CHIP>(builder->device, __VA_ARGS__);  \
      if (size > 0) {                                                         \
         tu_cs_begin_sub_stream(&pipeline->cs, size, &cs);                    \
         tu6_emit_##name<CHIP>(&cs, __VA_ARGS__);                             \
         pipeline->dynamic_state[id] =                                        \
            tu_cs_end_draw_state(&pipeline->cs, &cs);                         \
      }                                                                       \
      pipeline->set_state_mask |= (1u << id);                                 \
   }
#define DRAW_STATE(name, id, ...) DRAW_STATE_COND(name, id, true, __VA_ARGS__)

   DRAW_STATE(vertex_input, TU_DYNAMIC_STATE_VERTEX_INPUT,
              builder->graphics_state.vi);
   DRAW_STATE(vertex_stride, TU_DYNAMIC_STATE_VB_STRIDE,
              builder->graphics_state.vi);
   /* If (a) per-view viewport is used or (b) we don't know yet, then we need
    * to set viewport and stencil state dynamically.
    */
   bool no_per_view_viewport = pipeline_contains_all_shader_state(pipeline) &&
      !pipeline->program.per_view_viewport;
   DRAW_STATE_COND(viewport, TU_DYNAMIC_STATE_VIEWPORT, no_per_view_viewport,
                   builder->graphics_state.vp);
   DRAW_STATE_COND(scissor, TU_DYNAMIC_STATE_SCISSOR, no_per_view_viewport,
              builder->graphics_state.vp);
   DRAW_STATE(sample_locations,
              TU_DYNAMIC_STATE_SAMPLE_LOCATIONS,
              builder->graphics_state.ms->sample_locations_enable,
              builder->graphics_state.ms->sample_locations);
   DRAW_STATE(depth_bias, TU_DYNAMIC_STATE_DEPTH_BIAS,
              builder->graphics_state.rs);
   bool attachments_valid =
      builder->graphics_state.rp &&
      !(builder->graphics_state.rp->attachment_aspects &
                              VK_IMAGE_ASPECT_METADATA_BIT);
   struct vk_color_blend_state dummy_cb = {};
   const struct vk_color_blend_state *cb = builder->graphics_state.cb;
   if (attachments_valid &&
       !(builder->graphics_state.rp->attachment_aspects &
         VK_IMAGE_ASPECT_COLOR_BIT)) {
      /* If there are no color attachments, then the original blend state may
       * be NULL and the common code sanitizes it to always be NULL. In this
       * case we want to emit an empty blend/bandwidth/etc.  rather than
       * letting it be dynamic (and potentially garbage).
       */
      cb = &dummy_cb;
      BITSET_SET(pipeline_set, MESA_VK_DYNAMIC_CB_LOGIC_OP_ENABLE);
      BITSET_SET(pipeline_set, MESA_VK_DYNAMIC_CB_LOGIC_OP);
      BITSET_SET(pipeline_set, MESA_VK_DYNAMIC_CB_ATTACHMENT_COUNT);
      BITSET_SET(pipeline_set, MESA_VK_DYNAMIC_CB_COLOR_WRITE_ENABLES);
      BITSET_SET(pipeline_set, MESA_VK_DYNAMIC_CB_BLEND_ENABLES);
      BITSET_SET(pipeline_set, MESA_VK_DYNAMIC_CB_BLEND_EQUATIONS);
      BITSET_SET(pipeline_set, MESA_VK_DYNAMIC_CB_WRITE_MASKS);
      BITSET_SET(pipeline_set, MESA_VK_DYNAMIC_CB_BLEND_CONSTANTS);
   }
   DRAW_STATE(blend, TU_DYNAMIC_STATE_BLEND, cb,
              builder->graphics_state.ms->alpha_to_coverage_enable,
              builder->graphics_state.ms->alpha_to_one_enable,
              builder->graphics_state.ms->sample_mask);
   if (EMIT_STATE(blend_lrz, attachments_valid))
      tu_emit_blend_lrz(&pipeline->lrz_blend, cb,
                        builder->graphics_state.rp);
   if (EMIT_STATE(bandwidth, attachments_valid))
      tu_calc_bandwidth(&pipeline->bandwidth, cb,
                        builder->graphics_state.rp);
   DRAW_STATE(blend_constants, TU_DYNAMIC_STATE_BLEND_CONSTANTS, cb);
   if (attachments_valid &&
       !(builder->graphics_state.rp->attachment_aspects &
         VK_IMAGE_ASPECT_COLOR_BIT)) {
      /* Don't actually make anything dynamic as that may mean a partially-set
       * state group where the group is NULL which angers common code.
       */
      BITSET_CLEAR(remove, MESA_VK_DYNAMIC_CB_LOGIC_OP_ENABLE);
      BITSET_CLEAR(remove, MESA_VK_DYNAMIC_CB_LOGIC_OP);
      BITSET_CLEAR(remove, MESA_VK_DYNAMIC_CB_ATTACHMENT_COUNT);
      BITSET_CLEAR(remove, MESA_VK_DYNAMIC_CB_COLOR_WRITE_ENABLES);
      BITSET_CLEAR(remove, MESA_VK_DYNAMIC_CB_BLEND_ENABLES);
      BITSET_CLEAR(remove, MESA_VK_DYNAMIC_CB_BLEND_EQUATIONS);
      BITSET_CLEAR(remove, MESA_VK_DYNAMIC_CB_WRITE_MASKS);
      BITSET_CLEAR(remove, MESA_VK_DYNAMIC_CB_BLEND_CONSTANTS);
   }
   DRAW_STATE_COND(rast, TU_DYNAMIC_STATE_RAST,
                   pipeline_contains_all_shader_state(pipeline),
                   builder->graphics_state.rs,
                   builder->graphics_state.vp,
                   builder->graphics_state.rp->view_mask != 0,
                   pipeline->program.per_view_viewport);
   DRAW_STATE(ds, TU_DYNAMIC_STATE_DS,
              builder->graphics_state.ds);
   DRAW_STATE_COND(rb_depth_cntl, TU_DYNAMIC_STATE_RB_DEPTH_CNTL,
                   attachments_valid,
                   builder->graphics_state.ds,
                   builder->graphics_state.rp,
                   builder->graphics_state.rs);
   DRAW_STATE_COND(patch_control_points,
                   TU_DYNAMIC_STATE_PATCH_CONTROL_POINTS,
                   pipeline_contains_all_shader_state(pipeline),
                   pipeline->shaders[MESA_SHADER_VERTEX],
                   pipeline->shaders[MESA_SHADER_TESS_CTRL],
                   pipeline->shaders[MESA_SHADER_TESS_EVAL],
                   &pipeline->program,
                   builder->graphics_state.ts->patch_control_points);
#undef DRAW_STATE
#undef DRAW_STATE_COND
#undef EMIT_STATE

   /* LRZ always needs depth/stencil state at draw time */
   BITSET_SET(keep, MESA_VK_DYNAMIC_DS_DEPTH_TEST_ENABLE);
   BITSET_SET(keep, MESA_VK_DYNAMIC_DS_DEPTH_WRITE_ENABLE);
   BITSET_SET(keep, MESA_VK_DYNAMIC_DS_DEPTH_BOUNDS_TEST_ENABLE);
   BITSET_SET(keep, MESA_VK_DYNAMIC_DS_DEPTH_COMPARE_OP);
   BITSET_SET(keep, MESA_VK_DYNAMIC_DS_STENCIL_TEST_ENABLE);
   BITSET_SET(keep, MESA_VK_DYNAMIC_DS_STENCIL_OP);
   BITSET_SET(keep, MESA_VK_DYNAMIC_DS_STENCIL_WRITE_MASK);
   BITSET_SET(keep, MESA_VK_DYNAMIC_MS_ALPHA_TO_COVERAGE_ENABLE);

   /* MSAA needs line mode */
   BITSET_SET(keep, MESA_VK_DYNAMIC_RS_LINE_MODE);

   /* The patch control points is part of the draw */
   BITSET_SET(keep, MESA_VK_DYNAMIC_TS_PATCH_CONTROL_POINTS);

   /* Vertex buffer state needs to know the max valid binding */
   BITSET_SET(keep, MESA_VK_DYNAMIC_VI_BINDINGS_VALID);

   /* Remove state which has been emitted and we no longer need to set when
    * binding the pipeline by making it "dynamic".
    */
   BITSET_ANDNOT(remove, remove, keep);

   BITSET_OR(pipeline->static_state_mask, pipeline->static_state_mask, remove);

   BITSET_OR(builder->graphics_state.dynamic, builder->graphics_state.dynamic,
             remove);
}

static inline bool
emit_draw_state(const struct vk_dynamic_graphics_state *dynamic_state,
                const enum mesa_vk_dynamic_graphics_state *state_array,
                unsigned num_states)
{
   BITSET_DECLARE(state, MESA_VK_DYNAMIC_GRAPHICS_STATE_ENUM_MAX) = {};

   /* Unrolling this loop should produce a constant value once the function is
    * inlined, because state_array and num_states are a per-draw-state
    * constant, but GCC seems to need a little encouragement. clang does a
    * little better but still needs a pragma when there are a large number of
    * states.
    */
#if defined(__clang__)
#pragma clang loop unroll(full)
#elif defined(__GNUC__) && __GNUC__ >= 8
#pragma GCC unroll MESA_VK_DYNAMIC_GRAPHICS_STATE_ENUM_MAX
#endif
   for (unsigned i = 0; i < num_states; i++) {
      BITSET_SET(state, state_array[i]);
   }

   BITSET_DECLARE(temp, MESA_VK_DYNAMIC_GRAPHICS_STATE_ENUM_MAX);
   BITSET_AND(temp, state, dynamic_state->dirty);
   return !BITSET_IS_EMPTY(temp);
}

template <chip CHIP>
uint32_t
tu_emit_draw_state(struct tu_cmd_buffer *cmd)
{
   struct tu_cs cs;
   uint32_t dirty_draw_states = 0;

#define EMIT_STATE(name)                                                      \
   emit_draw_state(&cmd->vk.dynamic_graphics_state, tu_##name##_state,        \
                   ARRAY_SIZE(tu_##name##_state))
#define DRAW_STATE_COND(name, id, extra_cond, ...)                            \
   if ((EMIT_STATE(name) || extra_cond) &&                                    \
       !(cmd->state.pipeline_draw_states & (1u << id))) {                     \
      unsigned size = tu6_##name##_size<CHIP>(cmd->device, __VA_ARGS__);      \
      if (size > 0) {                                                         \
         tu_cs_begin_sub_stream(&cmd->sub_cs, size, &cs);                     \
         tu6_emit_##name<CHIP>(&cs, __VA_ARGS__);                             \
         cmd->state.dynamic_state[id] =                                       \
            tu_cs_end_draw_state(&cmd->sub_cs, &cs);                          \
      } else {                                                                \
         cmd->state.dynamic_state[id] = {};                                   \
      }                                                                       \
      dirty_draw_states |= (1u << id);                                        \
   }
#define DRAW_STATE_FDM(name, id, ...)                                         \
   if ((EMIT_STATE(name) || (cmd->state.dirty & TU_CMD_DIRTY_FDM)) &&         \
       !(cmd->state.pipeline_draw_states & (1u << id))) {                     \
      if (cmd->state.shaders[MESA_SHADER_FRAGMENT]->fs.has_fdm) {             \
         tu_cs_set_writeable(&cmd->sub_cs, true);                             \
         tu6_emit_##name##_fdm(&cs, cmd, __VA_ARGS__);                        \
         cmd->state.dynamic_state[id] =                                       \
            tu_cs_end_draw_state(&cmd->sub_cs, &cs);                          \
         tu_cs_set_writeable(&cmd->sub_cs, false);                            \
      } else {                                                                \
         unsigned size = tu6_##name##_size<CHIP>(cmd->device, __VA_ARGS__);   \
         if (size > 0) {                                                      \
            tu_cs_begin_sub_stream(&cmd->sub_cs, size, &cs);                  \
            tu6_emit_##name<CHIP>(&cs, __VA_ARGS__);                          \
            cmd->state.dynamic_state[id] =                                    \
               tu_cs_end_draw_state(&cmd->sub_cs, &cs);                       \
         } else {                                                             \
            cmd->state.dynamic_state[id] = {};                                \
         }                                                                    \
         tu_cs_begin_sub_stream(&cmd->sub_cs,                                 \
                                tu6_##name##_size<CHIP>(cmd->device, __VA_ARGS__),  \
                                &cs);                                         \
         tu6_emit_##name<CHIP>(&cs, __VA_ARGS__);                             \
         cmd->state.dynamic_state[id] =                                       \
            tu_cs_end_draw_state(&cmd->sub_cs, &cs);                          \
      }                                                                       \
      dirty_draw_states |= (1u << id);                                        \
   }
#define DRAW_STATE(name, id, ...) DRAW_STATE_COND(name, id, false, __VA_ARGS__)

   DRAW_STATE(vertex_input, TU_DYNAMIC_STATE_VERTEX_INPUT,
              cmd->vk.dynamic_graphics_state.vi);

   /* Vertex input stride is special because it's part of the vertex input in
    * the pipeline but a separate array when it's dynamic state so we have to
    * use two separate functions.
    */
#define tu6_emit_vertex_stride tu6_emit_vertex_stride_dyn
#define tu6_vertex_stride_size tu6_vertex_stride_size_dyn

   DRAW_STATE(vertex_stride, TU_DYNAMIC_STATE_VB_STRIDE,
              cmd->vk.dynamic_graphics_state.vi_binding_strides,
              cmd->vk.dynamic_graphics_state.vi_bindings_valid);

#undef tu6_emit_vertex_stride
#undef tu6_vertex_stride_size

   DRAW_STATE_FDM(viewport, TU_DYNAMIC_STATE_VIEWPORT,
                  &cmd->vk.dynamic_graphics_state.vp);
   DRAW_STATE_FDM(scissor, TU_DYNAMIC_STATE_SCISSOR,
                  &cmd->vk.dynamic_graphics_state.vp);
   DRAW_STATE(sample_locations,
              TU_DYNAMIC_STATE_SAMPLE_LOCATIONS,
              cmd->vk.dynamic_graphics_state.ms.sample_locations_enable,
              cmd->vk.dynamic_graphics_state.ms.sample_locations);
   DRAW_STATE(depth_bias, TU_DYNAMIC_STATE_DEPTH_BIAS,
              &cmd->vk.dynamic_graphics_state.rs);
   DRAW_STATE(blend, TU_DYNAMIC_STATE_BLEND,
              &cmd->vk.dynamic_graphics_state.cb,
              cmd->vk.dynamic_graphics_state.ms.alpha_to_coverage_enable,
              cmd->vk.dynamic_graphics_state.ms.alpha_to_one_enable,
              cmd->vk.dynamic_graphics_state.ms.sample_mask);
   if (EMIT_STATE(blend_lrz) ||
       ((cmd->state.dirty & TU_CMD_DIRTY_SUBPASS) &&
        !cmd->state.pipeline_blend_lrz)) {
      bool blend_reads_dest = tu6_calc_blend_lrz(&cmd->vk.dynamic_graphics_state.cb,
                                                 &cmd->state.vk_rp);
      if (blend_reads_dest != cmd->state.blend_reads_dest) {
         cmd->state.blend_reads_dest = blend_reads_dest;
         cmd->state.dirty |= TU_CMD_DIRTY_LRZ;
      }
   }
   if (EMIT_STATE(bandwidth) ||
       ((cmd->state.dirty & TU_CMD_DIRTY_SUBPASS) &&
        !cmd->state.pipeline_bandwidth))
      tu_calc_bandwidth(&cmd->state.bandwidth, &cmd->vk.dynamic_graphics_state.cb,
                        &cmd->state.vk_rp);
   DRAW_STATE(blend_constants, VK_DYNAMIC_STATE_BLEND_CONSTANTS,
              &cmd->vk.dynamic_graphics_state.cb);
   DRAW_STATE_COND(rast, TU_DYNAMIC_STATE_RAST,
                   cmd->state.dirty & (TU_CMD_DIRTY_SUBPASS |
                                       TU_CMD_DIRTY_PER_VIEW_VIEWPORT),
                   &cmd->vk.dynamic_graphics_state.rs,
                   &cmd->vk.dynamic_graphics_state.vp,
                   cmd->state.vk_rp.view_mask != 0,
                   cmd->state.per_view_viewport);
   DRAW_STATE(ds, TU_DYNAMIC_STATE_DS,
              &cmd->vk.dynamic_graphics_state.ds);
   DRAW_STATE_COND(rb_depth_cntl, TU_DYNAMIC_STATE_RB_DEPTH_CNTL,
                   cmd->state.dirty & TU_CMD_DIRTY_SUBPASS,
                   &cmd->vk.dynamic_graphics_state.ds,
                   &cmd->state.vk_rp,
                   &cmd->vk.dynamic_graphics_state.rs);
   DRAW_STATE_COND(patch_control_points,
                   TU_DYNAMIC_STATE_PATCH_CONTROL_POINTS,
                   cmd->state.dirty & TU_CMD_DIRTY_PROGRAM,
                   cmd->state.shaders[MESA_SHADER_VERTEX],
                   cmd->state.shaders[MESA_SHADER_TESS_CTRL],
                   cmd->state.shaders[MESA_SHADER_TESS_EVAL],
                   &cmd->state.program,
                   cmd->vk.dynamic_graphics_state.ts.patch_control_points);
#undef DRAW_STATE
#undef DRAW_STATE_COND
#undef EMIT_STATE

   return dirty_draw_states;
}
TU_GENX(tu_emit_draw_state);

static void
tu_pipeline_builder_parse_depth_stencil(
   struct tu_pipeline_builder *builder, struct tu_pipeline *pipeline)
{
   const VkPipelineDepthStencilStateCreateInfo *ds_info =
      builder->create_info->pDepthStencilState;

   if ((builder->graphics_state.rp->attachment_aspects &
        VK_IMAGE_ASPECT_METADATA_BIT) ||
       (builder->graphics_state.rp->attachment_aspects &
        VK_IMAGE_ASPECT_DEPTH_BIT)) {
      pipeline->ds.raster_order_attachment_access =
         ds_info && (ds_info->flags &
         (VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_DEPTH_ACCESS_BIT_EXT |
          VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_STENCIL_ACCESS_BIT_EXT));
   }
}

static void
tu_pipeline_builder_parse_multisample_and_color_blend(
   struct tu_pipeline_builder *builder, struct tu_pipeline *pipeline)
{
   /* The spec says:
    *
    *    pMultisampleState is a pointer to an instance of the
    *    VkPipelineMultisampleStateCreateInfo, and is ignored if the pipeline
    *    has rasterization disabled.
    *
    * Also,
    *
    *    pColorBlendState is a pointer to an instance of the
    *    VkPipelineColorBlendStateCreateInfo structure, and is ignored if the
    *    pipeline has rasterization disabled or if the subpass of the render
    *    pass the pipeline is created against does not use any color
    *    attachments.
    *
    * We leave the relevant registers stale when rasterization is disabled.
    */
   if (builder->rasterizer_discard) {
      return;
   }

   static const VkPipelineColorBlendStateCreateInfo dummy_blend_info = {};

   const VkPipelineColorBlendStateCreateInfo *blend_info =
      (builder->graphics_state.rp->attachment_aspects &
       VK_IMAGE_ASPECT_COLOR_BIT) ? builder->create_info->pColorBlendState :
      &dummy_blend_info;

   if (builder->graphics_state.rp->attachment_aspects & VK_IMAGE_ASPECT_COLOR_BIT) {
      pipeline->output.raster_order_attachment_access =
         blend_info && (blend_info->flags &
            VK_PIPELINE_COLOR_BLEND_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_BIT_EXT);
   }
}

static void
tu_pipeline_builder_parse_rasterization_order(
   struct tu_pipeline_builder *builder, struct tu_pipeline *pipeline)
{
   if (builder->rasterizer_discard)
      return;

   bool raster_order_attachment_access =
      pipeline->output.raster_order_attachment_access ||
      pipeline->ds.raster_order_attachment_access ||
      TU_DEBUG(RAST_ORDER);

   /* VK_EXT_blend_operation_advanced would also require ordered access
    * when implemented in the future.
    */

   enum a6xx_single_prim_mode sysmem_prim_mode = NO_FLUSH;
   enum a6xx_single_prim_mode gmem_prim_mode = NO_FLUSH;

   if (raster_order_attachment_access) {
      /* VK_EXT_rasterization_order_attachment_access:
       *
       * This extension allow access to framebuffer attachments when used as
       * both input and color attachments from one fragment to the next,
       * in rasterization order, without explicit synchronization.
       */
      sysmem_prim_mode = FLUSH_PER_OVERLAP_AND_OVERWRITE;
      gmem_prim_mode = FLUSH_PER_OVERLAP;
      pipeline->prim_order.sysmem_single_prim_mode = true;
   } else {
      /* If there is a feedback loop, then the shader can read the previous value
       * of a pixel being written out. It can also write some components and then
       * read different components without a barrier in between. This is a
       * problem in sysmem mode with UBWC, because the main buffer and flags
       * buffer can get out-of-sync if only one is flushed. We fix this by
       * setting the SINGLE_PRIM_MODE field to the same value that the blob does
       * for advanced_blend in sysmem mode if a feedback loop is detected.
       */
      if (builder->graphics_state.pipeline_flags &
          (VK_PIPELINE_CREATE_2_COLOR_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT |
           VK_PIPELINE_CREATE_2_DEPTH_STENCIL_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT)) {
         sysmem_prim_mode = FLUSH_PER_OVERLAP_AND_OVERWRITE;
         pipeline->prim_order.sysmem_single_prim_mode = true;
      }
   }

   struct tu_cs cs;

   pipeline->prim_order.state_gmem = tu_cs_draw_state(&pipeline->cs, &cs, 2);
   tu_cs_emit_write_reg(&cs, REG_A6XX_GRAS_SC_CNTL,
                        A6XX_GRAS_SC_CNTL_CCUSINGLECACHELINESIZE(2) |
                        A6XX_GRAS_SC_CNTL_SINGLE_PRIM_MODE(gmem_prim_mode));

   pipeline->prim_order.state_sysmem = tu_cs_draw_state(&pipeline->cs, &cs, 2);
   tu_cs_emit_write_reg(&cs, REG_A6XX_GRAS_SC_CNTL,
                        A6XX_GRAS_SC_CNTL_CCUSINGLECACHELINESIZE(2) |
                        A6XX_GRAS_SC_CNTL_SINGLE_PRIM_MODE(sysmem_prim_mode));
}

static void
tu_pipeline_finish(struct tu_pipeline *pipeline,
                   struct tu_device *dev,
                   const VkAllocationCallbacks *alloc)
{
   tu_cs_finish(&pipeline->cs);
   mtx_lock(&dev->pipeline_mutex);
   tu_suballoc_bo_free(&dev->pipeline_suballoc, &pipeline->bo);
   mtx_unlock(&dev->pipeline_mutex);

   if (pipeline->type == TU_PIPELINE_GRAPHICS_LIB) {
      struct tu_graphics_lib_pipeline *library =
         tu_pipeline_to_graphics_lib(pipeline);

      if (library->nir_shaders)
         vk_pipeline_cache_object_unref(&dev->vk,
                                        &library->nir_shaders->base);

      for (unsigned i = 0; i < library->num_sets; i++) {
         if (library->layouts[i])
            vk_descriptor_set_layout_unref(&dev->vk, &library->layouts[i]->vk);
      }

      vk_free2(&dev->vk.alloc, alloc, library->state_data);
   }

   for (unsigned i = 0; i < ARRAY_SIZE(pipeline->shaders); i++) {
      if (pipeline->shaders[i])
         vk_pipeline_cache_object_unref(&dev->vk,
                                        &pipeline->shaders[i]->base);
   }

   ralloc_free(pipeline->executables_mem_ctx);
}

static VkGraphicsPipelineLibraryFlagBitsEXT
vk_shader_stage_to_pipeline_library_flags(VkShaderStageFlagBits stage)
{
   assert(util_bitcount(stage) == 1);
   switch (stage) {
   case VK_SHADER_STAGE_VERTEX_BIT:
   case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
   case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
   case VK_SHADER_STAGE_GEOMETRY_BIT:
   case VK_SHADER_STAGE_TASK_BIT_EXT:
   case VK_SHADER_STAGE_MESH_BIT_EXT:
      return VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT;
   case VK_SHADER_STAGE_FRAGMENT_BIT:
      return VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT;
   default:
      unreachable("Invalid shader stage");
   }
}

template <chip CHIP>
static VkResult
tu_pipeline_builder_build(struct tu_pipeline_builder *builder,
                          struct tu_pipeline **pipeline)
{
   VkResult result;

   if (builder->create_flags & VK_PIPELINE_CREATE_2_LIBRARY_BIT_KHR) {
      *pipeline = (struct tu_pipeline *) vk_object_zalloc(
         &builder->device->vk, builder->alloc,
         sizeof(struct tu_graphics_lib_pipeline),
         VK_OBJECT_TYPE_PIPELINE);
      if (!*pipeline)
         return VK_ERROR_OUT_OF_HOST_MEMORY;
      (*pipeline)->type = TU_PIPELINE_GRAPHICS_LIB;
   } else {
      *pipeline = (struct tu_pipeline *) vk_object_zalloc(
         &builder->device->vk, builder->alloc,
         sizeof(struct tu_graphics_pipeline),
         VK_OBJECT_TYPE_PIPELINE);
      if (!*pipeline)
         return VK_ERROR_OUT_OF_HOST_MEMORY;
      (*pipeline)->type = TU_PIPELINE_GRAPHICS;
   }

   (*pipeline)->executables_mem_ctx = ralloc_context(NULL);
   util_dynarray_init(&(*pipeline)->executables, (*pipeline)->executables_mem_ctx);

   tu_pipeline_builder_parse_libraries(builder, *pipeline);

   VkShaderStageFlags stages = 0;
   for (unsigned i = 0; i < builder->create_info->stageCount; i++) {
      VkShaderStageFlagBits stage = builder->create_info->pStages[i].stage;

      /* Ignore shader stages that don't need to be imported. */
      if (!(vk_shader_stage_to_pipeline_library_flags(stage) & builder->state))
         continue;

      stages |= stage;
   }
   builder->active_stages = stages;

   (*pipeline)->active_stages = stages;
   for (unsigned i = 0; i < builder->num_libraries; i++)
      (*pipeline)->active_stages |= builder->libraries[i]->base.active_stages;

   /* Compile and upload shaders unless a library has already done that. */
   if ((*pipeline)->program.vs_state.size == 0) {
      tu_pipeline_builder_parse_layout(builder, *pipeline);

      result = tu_pipeline_builder_compile_shaders(builder, *pipeline);
      if (result != VK_SUCCESS) {
         vk_object_free(&builder->device->vk, builder->alloc, *pipeline);
         return result;
      }
   }

   result = tu_pipeline_allocate_cs(builder->device, *pipeline,
                                    &builder->layout, builder, NULL);


   if (set_combined_state(builder, *pipeline,
                          VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT |
                          VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT)) {
      if (result != VK_SUCCESS) {
         vk_object_free(&builder->device->vk, builder->alloc, *pipeline);
         return result;
      }

      tu_emit_program_state<CHIP>(&(*pipeline)->cs, &(*pipeline)->program,
                                  (*pipeline)->shaders);

      if (CHIP == A6XX) {
         /* Blob doesn't preload state on A7XX, likely preloading either
          * doesn't work or doesn't provide benefits.
          */
         tu6_emit_load_state(builder->device, *pipeline, &builder->layout);
      }
   }

   if (builder->state & VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT) {
      tu_pipeline_builder_parse_depth_stencil(builder, *pipeline);
   }

   if (builder->state &
       VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT) {
      tu_pipeline_builder_parse_multisample_and_color_blend(builder, *pipeline);
   }

   if (set_combined_state(builder, *pipeline,
                          VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT |
                          VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT)) {
      tu_pipeline_builder_parse_rasterization_order(builder, *pipeline);
   }

   tu_pipeline_builder_emit_state<CHIP>(builder, *pipeline);

   if ((*pipeline)->type == TU_PIPELINE_GRAPHICS_LIB) {
      struct tu_graphics_lib_pipeline *library =
         tu_pipeline_to_graphics_lib(*pipeline);
      result = vk_graphics_pipeline_state_copy(&builder->device->vk,
                                               &library->graphics_state,
                                               &builder->graphics_state,
                                               builder->alloc,
                                               VK_SYSTEM_ALLOCATION_SCOPE_OBJECT,
                                               &library->state_data);
      if (result != VK_SUCCESS) {
         tu_pipeline_finish(*pipeline, builder->device, builder->alloc);
         return result;
      }
   } else {
      struct tu_graphics_pipeline *gfx_pipeline =
         tu_pipeline_to_graphics(*pipeline);
      gfx_pipeline->dynamic_state.ms.sample_locations =
         &gfx_pipeline->sample_locations;
      vk_dynamic_graphics_state_fill(&gfx_pipeline->dynamic_state,
                                     &builder->graphics_state);
      gfx_pipeline->feedback_loop_color =
         (builder->graphics_state.pipeline_flags &
          VK_PIPELINE_CREATE_2_COLOR_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT);
      gfx_pipeline->feedback_loop_ds =
         (builder->graphics_state.pipeline_flags &
          VK_PIPELINE_CREATE_2_DEPTH_STENCIL_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT);
      gfx_pipeline->feedback_loop_may_involve_textures =
         builder->graphics_state.feedback_loop_not_input_only;
   }

   return VK_SUCCESS;
}

static void
tu_pipeline_builder_finish(struct tu_pipeline_builder *builder)
{
   ralloc_free(builder->mem_ctx);
}

void
tu_fill_render_pass_state(struct vk_render_pass_state *rp,
                          const struct tu_render_pass *pass,
                          const struct tu_subpass *subpass)
{
   rp->view_mask = subpass->multiview_mask;
   rp->color_attachment_count = subpass->color_count;

   const uint32_t a = subpass->depth_stencil_attachment.attachment;
   rp->depth_attachment_format = VK_FORMAT_UNDEFINED;
   rp->stencil_attachment_format = VK_FORMAT_UNDEFINED;
   rp->attachment_aspects = 0;
   if (a != VK_ATTACHMENT_UNUSED) {
      VkFormat ds_format = pass->attachments[a].format;
      if (vk_format_has_depth(ds_format)) {
         rp->depth_attachment_format = ds_format;
         rp->attachment_aspects |= VK_IMAGE_ASPECT_DEPTH_BIT;
      }
      if (vk_format_has_stencil(ds_format)) {
         rp->stencil_attachment_format = ds_format;
         rp->attachment_aspects |= VK_IMAGE_ASPECT_STENCIL_BIT;
      }
   }

   for (uint32_t i = 0; i < subpass->color_count; i++) {
      const uint32_t a = subpass->color_attachments[i].attachment;
      if (a == VK_ATTACHMENT_UNUSED) {
         rp->color_attachment_formats[i] = VK_FORMAT_UNDEFINED;
         continue;
      }

      rp->color_attachment_formats[i] = pass->attachments[a].format;
      rp->attachment_aspects |= VK_IMAGE_ASPECT_COLOR_BIT;
   }
}

static void
tu_pipeline_builder_init_graphics(
   struct tu_pipeline_builder *builder,
   struct tu_device *dev,
   struct vk_pipeline_cache *cache,
   const VkGraphicsPipelineCreateInfo *create_info,
   VkPipelineCreateFlags2KHR flags,
   const VkAllocationCallbacks *alloc)
{
   *builder = (struct tu_pipeline_builder) {
      .device = dev,
      .mem_ctx = ralloc_context(NULL),
      .cache = cache,
      .alloc = alloc,
      .create_info = create_info,
      .create_flags = flags,
   };

   const VkGraphicsPipelineLibraryCreateInfoEXT *gpl_info =
      vk_find_struct_const(builder->create_info->pNext, 
                           GRAPHICS_PIPELINE_LIBRARY_CREATE_INFO_EXT);

   const VkPipelineLibraryCreateInfoKHR *library_info =
      vk_find_struct_const(builder->create_info->pNext, 
                           PIPELINE_LIBRARY_CREATE_INFO_KHR);

   if (gpl_info) {
      builder->state = gpl_info->flags;
   } else {
      /* Implement this bit of spec text:
       *
       *    If this structure is omitted, and either
       *    VkGraphicsPipelineCreateInfo::flags includes
       *    VK_PIPELINE_CREATE_LIBRARY_BIT_KHR or the
       *    VkGraphicsPipelineCreateInfo::pNext chain includes a
       *    VkPipelineLibraryCreateInfoKHR structure with a libraryCount
       *    greater than 0, it is as if flags is 0. Otherwise if this
       *    structure is omitted, it is as if flags includes all possible
       *    subsets of the graphics pipeline (i.e. a complete graphics
       *    pipeline).
       */
      if ((library_info && library_info->libraryCount > 0) ||
          (builder->create_flags & VK_PIPELINE_CREATE_2_LIBRARY_BIT_KHR)) {
         builder->state = 0;
      } else {
         builder->state =
            VK_GRAPHICS_PIPELINE_LIBRARY_VERTEX_INPUT_INTERFACE_BIT_EXT |
            VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT |
            VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT |
            VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT;
      }
   }

   bool rasterizer_discard_dynamic = false;
   if (create_info->pDynamicState) {
      for (uint32_t i = 0; i < create_info->pDynamicState->dynamicStateCount; i++) {
         if (create_info->pDynamicState->pDynamicStates[i] ==
               VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE) {
            rasterizer_discard_dynamic = true;
            break;
         }
      }
   }

   builder->rasterizer_discard =
      (builder->state & VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT) &&
      !rasterizer_discard_dynamic &&
      builder->create_info->pRasterizationState->rasterizerDiscardEnable;

   struct vk_render_pass_state rp_state = {};
   const struct vk_render_pass_state *driver_rp = NULL;
   VkPipelineCreateFlags2KHR rp_flags = 0;

   builder->unscaled_input_fragcoord = 0;

   /* Extract information we need from the turnip renderpass. This will be
    * filled out automatically if the app is using dynamic rendering or
    * renderpasses are emulated.
    */
   if (!TU_DEBUG(DYNAMIC) &&
       (builder->state &
        (VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT |
         VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT |
         VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT)) &&
       builder->create_info->renderPass) {
      const struct tu_render_pass *pass =
         tu_render_pass_from_handle(create_info->renderPass);
      const struct tu_subpass *subpass =
         &pass->subpasses[create_info->subpass];

      tu_fill_render_pass_state(&rp_state, pass, subpass);

      for (unsigned i = 0; i < subpass->input_count; i++) {
         /* Input attachments stored in GMEM must be loaded with unscaled
          * FragCoord.
          */
         if (subpass->input_attachments[i].patch_input_gmem)
            builder->unscaled_input_fragcoord |= 1u << i;
      }

      if (subpass->feedback_loop_color) {
         rp_flags |=
            VK_PIPELINE_CREATE_2_COLOR_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT;
      }

      if (subpass->feedback_loop_ds) {
         rp_flags |=
            VK_PIPELINE_CREATE_2_DEPTH_STENCIL_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT;
      }

      if (pass->fragment_density_map.attachment != VK_ATTACHMENT_UNUSED) {
         rp_flags |=
            VK_PIPELINE_CREATE_2_RENDERING_FRAGMENT_DENSITY_MAP_ATTACHMENT_BIT_EXT;
      }

      builder->unscaled_input_fragcoord = 0;
      for (unsigned i = 0; i < subpass->input_count; i++) {
         /* Input attachments stored in GMEM must be loaded with unscaled
          * FragCoord.
          */
         if (subpass->input_attachments[i].patch_input_gmem)
            builder->unscaled_input_fragcoord |= 1u << i;
      }

      driver_rp = &rp_state;
   }

   vk_graphics_pipeline_state_fill(&dev->vk,
                                   &builder->graphics_state,
                                   builder->create_info,
                                   driver_rp,
                                   rp_flags,
                                   &builder->all_state,
                                   NULL, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT,
                                   NULL);

   if (builder->graphics_state.rp) {
      builder->fragment_density_map = (builder->graphics_state.pipeline_flags &
         VK_PIPELINE_CREATE_2_RENDERING_FRAGMENT_DENSITY_MAP_ATTACHMENT_BIT_EXT) ||
         TU_DEBUG(FDM);
   }
}

template <chip CHIP>
static VkResult
tu_graphics_pipeline_create(VkDevice device,
                            VkPipelineCache pipelineCache,
                            const VkGraphicsPipelineCreateInfo *pCreateInfo,
                            VkPipelineCreateFlags2KHR flags,
                            const VkAllocationCallbacks *pAllocator,
                            VkPipeline *pPipeline)
{
   TU_FROM_HANDLE(tu_device, dev, device);
   TU_FROM_HANDLE(vk_pipeline_cache, cache, pipelineCache);

   cache = cache ? cache : dev->mem_cache;

   struct tu_pipeline_builder builder;
   tu_pipeline_builder_init_graphics(&builder, dev, cache,
                                     pCreateInfo, flags, pAllocator);

   struct tu_pipeline *pipeline = NULL;
   VkResult result = tu_pipeline_builder_build<CHIP>(&builder, &pipeline);
   tu_pipeline_builder_finish(&builder);

   if (result == VK_SUCCESS)
      *pPipeline = tu_pipeline_to_handle(pipeline);
   else
      *pPipeline = VK_NULL_HANDLE;

   return result;
}

template <chip CHIP>
VKAPI_ATTR VkResult VKAPI_CALL
tu_CreateGraphicsPipelines(VkDevice device,
                           VkPipelineCache pipelineCache,
                           uint32_t count,
                           const VkGraphicsPipelineCreateInfo *pCreateInfos,
                           const VkAllocationCallbacks *pAllocator,
                           VkPipeline *pPipelines)
{
   MESA_TRACE_FUNC();
   VkResult final_result = VK_SUCCESS;
   uint32_t i = 0;

   for (; i < count; i++) {
      VkPipelineCreateFlags2KHR flags =
         vk_graphics_pipeline_create_flags(&pCreateInfos[i]);

      VkResult result =
         tu_graphics_pipeline_create<CHIP>(device, pipelineCache,
                                           &pCreateInfos[i], flags,
                                           pAllocator, &pPipelines[i]);

      if (result != VK_SUCCESS) {
         final_result = result;
         pPipelines[i] = VK_NULL_HANDLE;

         if (flags & VK_PIPELINE_CREATE_2_EARLY_RETURN_ON_FAILURE_BIT_KHR)
            break;
      }
   }

   for (; i < count; i++)
      pPipelines[i] = VK_NULL_HANDLE;

   return final_result;
}
TU_GENX(tu_CreateGraphicsPipelines);

template <chip CHIP>
static VkResult
tu_compute_pipeline_create(VkDevice device,
                           VkPipelineCache pipelineCache,
                           const VkComputePipelineCreateInfo *pCreateInfo,
                           VkPipelineCreateFlags2KHR flags,
                           const VkAllocationCallbacks *pAllocator,
                           VkPipeline *pPipeline)
{
   TU_FROM_HANDLE(tu_device, dev, device);
   TU_FROM_HANDLE(vk_pipeline_cache, cache, pipelineCache);
   TU_FROM_HANDLE(tu_pipeline_layout, layout, pCreateInfo->layout);
   const VkPipelineShaderStageCreateInfo *stage_info = &pCreateInfo->stage;
   VkResult result;
   const struct ir3_shader_variant *v = NULL;

   cache = cache ? cache : dev->mem_cache;

   struct tu_compute_pipeline *pipeline;

   *pPipeline = VK_NULL_HANDLE;

   VkPipelineCreationFeedback pipeline_feedback = {
      .flags = VK_PIPELINE_CREATION_FEEDBACK_VALID_BIT,
   };

   const VkPipelineCreationFeedbackCreateInfo *creation_feedback =
      vk_find_struct_const(pCreateInfo->pNext, PIPELINE_CREATION_FEEDBACK_CREATE_INFO);

   int64_t pipeline_start = os_time_get_nano();

   pipeline = (struct tu_compute_pipeline *) vk_object_zalloc(
      &dev->vk, pAllocator, sizeof(*pipeline), VK_OBJECT_TYPE_PIPELINE);
   if (!pipeline)
      return VK_ERROR_OUT_OF_HOST_MEMORY;
   pipeline->base.type = TU_PIPELINE_COMPUTE;

   pipeline->base.executables_mem_ctx = ralloc_context(NULL);
   util_dynarray_init(&pipeline->base.executables, pipeline->base.executables_mem_ctx);
   pipeline->base.active_stages = VK_SHADER_STAGE_COMPUTE_BIT;

   struct tu_shader_key key = { };
   bool allow_varying_subgroup_size =
      (stage_info->flags &
       VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT_EXT);
   bool require_full_subgroups =
      stage_info->flags &
      VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT;
   const VkPipelineShaderStageRequiredSubgroupSizeCreateInfo *subgroup_info =
      vk_find_struct_const(stage_info,
                           PIPELINE_SHADER_STAGE_REQUIRED_SUBGROUP_SIZE_CREATE_INFO);
   tu_shader_key_subgroup_size(&key, allow_varying_subgroup_size,
                               require_full_subgroups, subgroup_info,
                               dev);

   void *pipeline_mem_ctx = ralloc_context(NULL);

   unsigned char pipeline_sha1[20];
   tu_hash_compute(pipeline_sha1, stage_info, layout, &key, dev->compiler);

   struct tu_shader *shader = NULL;

   const bool executable_info = flags &
      VK_PIPELINE_CREATE_2_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR;

   bool application_cache_hit = false;

   if (!executable_info) {
      shader =
         tu_pipeline_cache_lookup(cache, pipeline_sha1, sizeof(pipeline_sha1),
                                  &application_cache_hit);
   }

   if (application_cache_hit && cache != dev->mem_cache) {
      pipeline_feedback.flags |=
         VK_PIPELINE_CREATION_FEEDBACK_APPLICATION_PIPELINE_CACHE_HIT_BIT;
   }

   char *nir_initial_disasm = NULL;

   if (!shader) {
      if (flags &
          VK_PIPELINE_CREATE_2_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT_KHR) {
         result = VK_PIPELINE_COMPILE_REQUIRED;
         goto fail;
      }

      struct ir3_shader_key ir3_key = {};

      nir_shader *nir = tu_spirv_to_nir(dev, pipeline_mem_ctx, stage_info,
                                        MESA_SHADER_COMPUTE);

      nir_initial_disasm = executable_info ?
         nir_shader_as_str(nir, pipeline->base.executables_mem_ctx) : NULL;

      result = tu_shader_create(dev, &shader, nir, &key, &ir3_key,
                                pipeline_sha1, sizeof(pipeline_sha1), layout,
                                executable_info);
      if (!shader) {
         goto fail;
      }

      shader = tu_pipeline_cache_insert(cache, shader);
   }

   pipeline_feedback.duration = os_time_get_nano() - pipeline_start;

   if (creation_feedback) {
      *creation_feedback->pPipelineCreationFeedback = pipeline_feedback;
      assert(creation_feedback->pipelineStageCreationFeedbackCount == 1);
      creation_feedback->pPipelineStageCreationFeedbacks[0] = pipeline_feedback;
   }

   pipeline->base.active_desc_sets = shader->active_desc_sets;

   v = shader->variant;

   tu_pipeline_set_linkage(&pipeline->base.program.link[MESA_SHADER_COMPUTE],
                           &shader->const_state, v);

   result = tu_pipeline_allocate_cs(dev, &pipeline->base, layout, NULL, v);
   if (result != VK_SUCCESS)
      goto fail;

   for (int i = 0; i < 3; i++)
      pipeline->local_size[i] = v->local_size[i];

   if (CHIP == A6XX) {
      tu6_emit_load_state(dev, &pipeline->base, layout);
   }

   tu_append_executable(&pipeline->base, v, nir_initial_disasm);

   pipeline->instrlen = v->instrlen;

   pipeline->base.shaders[MESA_SHADER_COMPUTE] = shader;

   ralloc_free(pipeline_mem_ctx);

   *pPipeline = tu_pipeline_to_handle(&pipeline->base);

   return VK_SUCCESS;

fail:
   if (shader)
      vk_pipeline_cache_object_unref(&dev->vk, &shader->base);

   ralloc_free(pipeline_mem_ctx);

   vk_object_free(&dev->vk, pAllocator, pipeline);

   return result;
}

template <chip CHIP>
VKAPI_ATTR VkResult VKAPI_CALL
tu_CreateComputePipelines(VkDevice device,
                          VkPipelineCache pipelineCache,
                          uint32_t count,
                          const VkComputePipelineCreateInfo *pCreateInfos,
                          const VkAllocationCallbacks *pAllocator,
                          VkPipeline *pPipelines)
{
   MESA_TRACE_FUNC();
   VkResult final_result = VK_SUCCESS;
   uint32_t i = 0;

   for (; i < count; i++) {
      VkPipelineCreateFlags2KHR flags =
         vk_compute_pipeline_create_flags(&pCreateInfos[i]);

      VkResult result =
         tu_compute_pipeline_create<CHIP>(device, pipelineCache,
                                          &pCreateInfos[i], flags,
                                          pAllocator, &pPipelines[i]);
      if (result != VK_SUCCESS) {
         final_result = result;
         pPipelines[i] = VK_NULL_HANDLE;

         if (flags &
             VK_PIPELINE_CREATE_2_EARLY_RETURN_ON_FAILURE_BIT_KHR)
            break;
      }
   }

   for (; i < count; i++)
      pPipelines[i] = VK_NULL_HANDLE;

   return final_result;
}
TU_GENX(tu_CreateComputePipelines);

VKAPI_ATTR void VKAPI_CALL
tu_DestroyPipeline(VkDevice _device,
                   VkPipeline _pipeline,
                   const VkAllocationCallbacks *pAllocator)
{
   TU_FROM_HANDLE(tu_device, dev, _device);
   TU_FROM_HANDLE(tu_pipeline, pipeline, _pipeline);

   if (!_pipeline)
      return;

   tu_pipeline_finish(pipeline, dev, pAllocator);
   vk_object_free(&dev->vk, pAllocator, pipeline);
}

#define WRITE_STR(field, ...) ({                                \
   memset(field, 0, sizeof(field));                             \
   UNUSED int _i = snprintf(field, sizeof(field), __VA_ARGS__); \
   assert(_i > 0 && _i < sizeof(field));                        \
})

static const struct tu_pipeline_executable *
tu_pipeline_get_executable(struct tu_pipeline *pipeline, uint32_t index)
{
   assert(index < util_dynarray_num_elements(&pipeline->executables,
                                             struct tu_pipeline_executable));
   return util_dynarray_element(
      &pipeline->executables, struct tu_pipeline_executable, index);
}

VKAPI_ATTR VkResult VKAPI_CALL
tu_GetPipelineExecutablePropertiesKHR(
      VkDevice _device,
      const VkPipelineInfoKHR* pPipelineInfo,
      uint32_t* pExecutableCount,
      VkPipelineExecutablePropertiesKHR* pProperties)
{
   TU_FROM_HANDLE(tu_device, dev, _device);
   TU_FROM_HANDLE(tu_pipeline, pipeline, pPipelineInfo->pipeline);
   VK_OUTARRAY_MAKE_TYPED(VkPipelineExecutablePropertiesKHR, out,
                          pProperties, pExecutableCount);

   util_dynarray_foreach (&pipeline->executables, struct tu_pipeline_executable, exe) {
      vk_outarray_append_typed(VkPipelineExecutablePropertiesKHR, &out, props) {
         gl_shader_stage stage = exe->stage;
         props->stages = mesa_to_vk_shader_stage(stage);

         if (!exe->is_binning)
            WRITE_STR(props->name, "%s", _mesa_shader_stage_to_abbrev(stage));
         else
            WRITE_STR(props->name, "Binning VS");

         WRITE_STR(props->description, "%s", _mesa_shader_stage_to_string(stage));

         props->subgroupSize =
            dev->compiler->threadsize_base * (exe->stats.double_threadsize ? 2 : 1);
      }
   }

   return vk_outarray_status(&out);
}

VKAPI_ATTR VkResult VKAPI_CALL
tu_GetPipelineExecutableStatisticsKHR(
      VkDevice _device,
      const VkPipelineExecutableInfoKHR* pExecutableInfo,
      uint32_t* pStatisticCount,
      VkPipelineExecutableStatisticKHR* pStatistics)
{
   TU_FROM_HANDLE(tu_pipeline, pipeline, pExecutableInfo->pipeline);
   VK_OUTARRAY_MAKE_TYPED(VkPipelineExecutableStatisticKHR, out,
                          pStatistics, pStatisticCount);

   const struct tu_pipeline_executable *exe =
      tu_pipeline_get_executable(pipeline, pExecutableInfo->executableIndex);

   vk_outarray_append_typed(VkPipelineExecutableStatisticKHR, &out, stat) {
      WRITE_STR(stat->name, "Max Waves Per Core");
      WRITE_STR(stat->description,
                "Maximum number of simultaneous waves per core.");
      stat->format = VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR;
      stat->value.u64 = exe->stats.max_waves;
   }

   vk_outarray_append_typed(VkPipelineExecutableStatisticKHR, &out, stat) {
      WRITE_STR(stat->name, "Instruction Count");
      WRITE_STR(stat->description,
                "Total number of IR3 instructions in the final generated "
                "shader executable.");
      stat->format = VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR;
      stat->value.u64 = exe->stats.instrs_count;
   }

   vk_outarray_append_typed(VkPipelineExecutableStatisticKHR, &out, stat) {
      WRITE_STR(stat->name, "Code size");
      WRITE_STR(stat->description,
                "Total number of dwords in the final generated "
                "shader executable.");
      stat->format = VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR;
      stat->value.u64 = exe->stats.sizedwords;
   }

   vk_outarray_append_typed(VkPipelineExecutableStatisticKHR, &out, stat) {
      WRITE_STR(stat->name, "NOPs Count");
      WRITE_STR(stat->description,
                "Number of NOP instructions in the final generated "
                "shader executable.");
      stat->format = VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR;
      stat->value.u64 = exe->stats.nops_count;
   }

   vk_outarray_append_typed(VkPipelineExecutableStatisticKHR, &out, stat) {
      WRITE_STR(stat->name, "MOV Count");
      WRITE_STR(stat->description,
                "Number of MOV instructions in the final generated "
                "shader executable.");
      stat->format = VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR;
      stat->value.u64 = exe->stats.mov_count;
   }

   vk_outarray_append_typed(VkPipelineExecutableStatisticKHR, &out, stat) {
      WRITE_STR(stat->name, "COV Count");
      WRITE_STR(stat->description,
                "Number of COV instructions in the final generated "
                "shader executable.");
      stat->format = VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR;
      stat->value.u64 = exe->stats.cov_count;
   }

   vk_outarray_append_typed(VkPipelineExecutableStatisticKHR, &out, stat) {
      WRITE_STR(stat->name, "Registers used");
      WRITE_STR(stat->description,
                "Number of registers used in the final generated "
                "shader executable.");
      stat->format = VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR;
      stat->value.u64 = exe->stats.max_reg + 1;
   }

   vk_outarray_append_typed(VkPipelineExecutableStatisticKHR, &out, stat) {
      WRITE_STR(stat->name, "Half-registers used");
      WRITE_STR(stat->description,
                "Number of half-registers used in the final generated "
                "shader executable.");
      stat->format = VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR;
      stat->value.u64 = exe->stats.max_half_reg + 1;
   }

   vk_outarray_append_typed(VkPipelineExecutableStatisticKHR, &out, stat) {
      WRITE_STR(stat->name, "Last interpolation instruction");
      WRITE_STR(stat->description,
                "The instruction where varying storage in Local Memory is released");
      stat->format = VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR;
      stat->value.u64 = exe->stats.last_baryf;
   }

   vk_outarray_append_typed(VkPipelineExecutableStatisticKHR, &out, stat) {
      WRITE_STR(stat->name, "Last helper instruction");
      WRITE_STR(stat->description,
                "The instruction where helper invocations are killed");
      stat->format = VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR;
      stat->value.u64 = exe->stats.last_helper;
   }

   vk_outarray_append_typed(VkPipelineExecutableStatisticKHR, &out, stat) {
      WRITE_STR(stat->name, "Instructions with SS sync bit");
      WRITE_STR(stat->description,
                "SS bit is set for instructions which depend on a result "
                "of \"long\" instructions to prevent RAW hazard.");
      stat->format = VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR;
      stat->value.u64 = exe->stats.ss;
   }

   vk_outarray_append_typed(VkPipelineExecutableStatisticKHR, &out, stat) {
      WRITE_STR(stat->name, "Instructions with SY sync bit");
      WRITE_STR(stat->description,
                "SY bit is set for instructions which depend on a result "
                "of loads from global memory to prevent RAW hazard.");
      stat->format = VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR;
      stat->value.u64 = exe->stats.sy;
   }

   vk_outarray_append_typed(VkPipelineExecutableStatisticKHR, &out, stat) {
      WRITE_STR(stat->name, "Estimated cycles stalled on SS");
      WRITE_STR(stat->description,
                "A better metric to estimate the impact of SS syncs.");
      stat->format = VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR;
      stat->value.u64 = exe->stats.sstall;
   }

   vk_outarray_append_typed(VkPipelineExecutableStatisticKHR, &out, stat) {
      WRITE_STR(stat->name, "Estimated cycles stalled on SY");
      WRITE_STR(stat->description,
                "A better metric to estimate the impact of SY syncs.");
      stat->format = VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR;
      stat->value.u64 = exe->stats.systall;
   }

   for (int i = 0; i < ARRAY_SIZE(exe->stats.instrs_per_cat); i++) {
      vk_outarray_append_typed(VkPipelineExecutableStatisticKHR, &out, stat) {
         WRITE_STR(stat->name, "cat%d instructions", i);
         WRITE_STR(stat->description,
                  "Number of cat%d instructions.", i);
         stat->format = VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR;
         stat->value.u64 = exe->stats.instrs_per_cat[i];
      }
   }

   vk_outarray_append_typed(VkPipelineExecutableStatisticKHR, &out, stat) {
      WRITE_STR(stat->name, "STP Count");
      WRITE_STR(stat->description,
                "Number of STore Private instructions in the final generated "
                "shader executable.");
      stat->format = VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR;
      stat->value.u64 = exe->stats.stp_count;
   }

   vk_outarray_append_typed(VkPipelineExecutableStatisticKHR, &out, stat) {
      WRITE_STR(stat->name, "LDP Count");
      WRITE_STR(stat->description,
                "Number of LoaD Private instructions in the final generated "
                "shader executable.");
      stat->format = VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR;
      stat->value.u64 = exe->stats.ldp_count;
   }

   return vk_outarray_status(&out);
}

static bool
write_ir_text(VkPipelineExecutableInternalRepresentationKHR* ir,
              const char *data)
{
   ir->isText = VK_TRUE;

   size_t data_len = strlen(data) + 1;

   if (ir->pData == NULL) {
      ir->dataSize = data_len;
      return true;
   }

   strncpy((char *) ir->pData, data, ir->dataSize);
   if (ir->dataSize < data_len)
      return false;

   ir->dataSize = data_len;
   return true;
}

VKAPI_ATTR VkResult VKAPI_CALL
tu_GetPipelineExecutableInternalRepresentationsKHR(
    VkDevice _device,
    const VkPipelineExecutableInfoKHR* pExecutableInfo,
    uint32_t* pInternalRepresentationCount,
    VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations)
{
   TU_FROM_HANDLE(tu_pipeline, pipeline, pExecutableInfo->pipeline);
   VK_OUTARRAY_MAKE_TYPED(VkPipelineExecutableInternalRepresentationKHR, out,
                          pInternalRepresentations, pInternalRepresentationCount);
   bool incomplete_text = false;

   const struct tu_pipeline_executable *exe =
      tu_pipeline_get_executable(pipeline, pExecutableInfo->executableIndex);

   if (exe->nir_from_spirv) {
      vk_outarray_append_typed(VkPipelineExecutableInternalRepresentationKHR, &out, ir) {
         WRITE_STR(ir->name, "NIR from SPIRV");
         WRITE_STR(ir->description,
                   "Initial NIR before any optimizations");

         if (!write_ir_text(ir, exe->nir_from_spirv))
            incomplete_text = true;
      }
   }

   if (exe->nir_final) {
      vk_outarray_append_typed(VkPipelineExecutableInternalRepresentationKHR, &out, ir) {
         WRITE_STR(ir->name, "Final NIR");
         WRITE_STR(ir->description,
                   "Final NIR before going into the back-end compiler");

         if (!write_ir_text(ir, exe->nir_final))
            incomplete_text = true;
      }
   }

   if (exe->disasm) {
      vk_outarray_append_typed(VkPipelineExecutableInternalRepresentationKHR, &out, ir) {
         WRITE_STR(ir->name, "IR3 Assembly");
         WRITE_STR(ir->description,
                   "Final IR3 assembly for the generated shader binary");

         if (!write_ir_text(ir, exe->disasm))
            incomplete_text = true;
      }
   }

   return incomplete_text ? VK_INCOMPLETE : vk_outarray_status(&out);
}
