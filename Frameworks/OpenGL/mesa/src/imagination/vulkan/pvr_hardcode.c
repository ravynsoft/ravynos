/*
 * Copyright © 2022 Imagination Technologies Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

#include "compiler/shader_enums.h"
#include "hwdef/rogue_hw_utils.h"
#include "pvr_device_info.h"
#include "pvr_hardcode.h"
#include "pvr_private.h"
#include "rogue/rogue.h"
#include "usc/hardcoded_apps/pvr_simple_compute.h"
#include "util/macros.h"
#include "util/u_dynarray.h"
#include "util/u_process.h"

/**
 * \file pvr_hardcode.c
 *
 * \brief Contains hard coding functions.
 * This should eventually be deleted as the compiler becomes more capable.
 */

#define PVR_AXE_1_16M_BVNC PVR_BVNC_PACK(33, 15, 11, 3)
#define PVR_GX6250_BVNC PVR_BVNC_PACK(4, 40, 2, 51)

#define util_dynarray_append_mem(buf, size, mem) \
   memcpy(util_dynarray_grow_bytes((buf), 1, size), mem, size)

enum pvr_hard_code_shader_type {
   PVR_HARD_CODE_SHADER_TYPE_COMPUTE,
   PVR_HARD_CODE_SHADER_TYPE_GRAPHICS,
};

static const struct pvr_hard_coding_data {
   const char *const name;
   uint64_t bvnc;
   enum pvr_hard_code_shader_type type;

   union {
      struct {
         const uint8_t *const shader;
         size_t shader_size;

         /* Note that the bo field will be unused. */
         const struct pvr_compute_shader_state shader_info;

         const struct pvr_hard_code_compute_build_info build_info;
      } compute;

      struct {
         /* Mask of MESA_SHADER_* (gl_shader_stage). */
         uint32_t flags;

         uint8_t *const *const vert_shaders;
         unsigned *vert_shader_sizes;
         uint8_t *const *const frag_shaders;
         unsigned *frag_shader_sizes;

         const struct pvr_vertex_shader_state *const *const vert_shader_states;
         const struct pvr_fragment_shader_state *const *const frag_shader_states;

         const struct pvr_hard_code_graphics_build_info *const
            *const build_infos;

         uint32_t shader_count;
      } graphics;
   };

} hard_coding_table[] = {
   {
      .name = "simple-compute",
      .bvnc = PVR_GX6250_BVNC,
      .type = PVR_HARD_CODE_SHADER_TYPE_COMPUTE,

      .compute = {
         .shader = pvr_simple_compute_shader,
         .shader_size = sizeof(pvr_simple_compute_shader),

         .shader_info = {
            .uses_atomic_ops = false,
            .uses_barrier = false,
            .uses_num_workgroups = false,

            .const_shared_reg_count = 4,
            .input_register_count = 8,
            .work_size = 1 * 1 * 1,
            .coefficient_register_count = 4,
         },

         .build_info = {
            .ubo_data = { 0 },
            .compile_time_consts_data = {
               .static_consts = { 0 },
            },

            .local_invocation_regs = { 0, 1 },
            .work_group_regs = { 0, 1, 2 },
            .barrier_reg = ROGUE_REG_UNUSED,
            .usc_temps = 0,

            .explicit_conts_usage = {
               .start_offset = 0,
            },
         },
      }
   },
};

static inline uint64_t
pvr_device_get_bvnc(const struct pvr_device_info *const dev_info)
{
   const struct pvr_device_ident *const ident = &dev_info->ident;

   return PVR_BVNC_PACK(ident->b, ident->v, ident->n, ident->c);
}

bool pvr_has_hard_coded_shaders(const struct pvr_device_info *const dev_info)
{
   const char *const program = util_get_process_name();
   const uint64_t bvnc = pvr_device_get_bvnc(dev_info);

   for (uint32_t i = 0; i < ARRAY_SIZE(hard_coding_table); i++) {
      if (bvnc != hard_coding_table[i].bvnc)
         continue;

      if (strcmp(program, hard_coding_table[i].name) == 0)
         return true;
   }

   return false;
}

static const struct pvr_hard_coding_data *
pvr_get_hard_coding_data(const struct pvr_device_info *const dev_info)
{
   const char *const program = util_get_process_name();
   const uint64_t bvnc = pvr_device_get_bvnc(dev_info);

   for (uint32_t i = 0; i < ARRAY_SIZE(hard_coding_table); i++) {
      if (bvnc != hard_coding_table[i].bvnc)
         continue;

      if (strcmp(program, hard_coding_table[i].name) == 0)
         return &hard_coding_table[i];
   }

   mesa_loge("Could not find hard coding data for %s", program);

   return NULL;
}

VkResult pvr_hard_code_compute_pipeline(
   struct pvr_device *const device,
   struct pvr_compute_shader_state *const shader_state_out,
   struct pvr_hard_code_compute_build_info *const build_info_out)
{
   const uint32_t cache_line_size =
      rogue_get_slc_cache_line_size(&device->pdevice->dev_info);
   const struct pvr_hard_coding_data *const data =
      pvr_get_hard_coding_data(&device->pdevice->dev_info);

   assert(data->type == PVR_HARD_CODE_SHADER_TYPE_COMPUTE);

   mesa_logd("Hard coding compute pipeline for %s", data->name);

   *build_info_out = data->compute.build_info;
   *shader_state_out = data->compute.shader_info;

   return pvr_gpu_upload_usc(device,
                             data->compute.shader,
                             data->compute.shader_size,
                             cache_line_size,
                             &shader_state_out->bo);
}

uint32_t
pvr_hard_code_graphics_get_flags(const struct pvr_device_info *const dev_info)
{
   const struct pvr_hard_coding_data *const data =
      pvr_get_hard_coding_data(dev_info);

   assert(data->type == PVR_HARD_CODE_SHADER_TYPE_GRAPHICS);

   return data->graphics.flags;
}

void pvr_hard_code_graphics_shader(const struct pvr_device_info *const dev_info,
                                   uint32_t pipeline_n,
                                   gl_shader_stage stage,
                                   struct util_dynarray *shader_out)
{
   const struct pvr_hard_coding_data *const data =
      pvr_get_hard_coding_data(dev_info);

   assert(data->type == PVR_HARD_CODE_SHADER_TYPE_GRAPHICS);
   assert(pipeline_n < data->graphics.shader_count);
   assert(data->graphics.flags & BITFIELD_BIT(stage));

   mesa_logd("Hard coding %s stage shader for \"%s\" demo.",
             _mesa_shader_stage_to_string(stage),
             data->name);

   switch (stage) {
   case MESA_SHADER_VERTEX:
      util_dynarray_append_mem(shader_out,
                               data->graphics.vert_shader_sizes[pipeline_n],
                               data->graphics.vert_shaders[pipeline_n]);
      break;

   case MESA_SHADER_FRAGMENT:
      util_dynarray_append_mem(shader_out,
                               data->graphics.frag_shader_sizes[pipeline_n],
                               data->graphics.frag_shaders[pipeline_n]);
      break;

   default:
      unreachable("Unsupported stage.");
   }
}

void pvr_hard_code_graphics_vertex_state(
   const struct pvr_device_info *const dev_info,
   uint32_t pipeline_n,
   struct pvr_vertex_shader_state *const vert_state_out)
{
   const struct pvr_hard_coding_data *const data =
      pvr_get_hard_coding_data(dev_info);

   assert(data->type == PVR_HARD_CODE_SHADER_TYPE_GRAPHICS);
   assert(pipeline_n < data->graphics.shader_count);
   assert(data->graphics.flags & BITFIELD_BIT(MESA_SHADER_VERTEX));

   *vert_state_out = *data->graphics.vert_shader_states[0];
}

void pvr_hard_code_graphics_fragment_state(
   const struct pvr_device_info *const dev_info,
   uint32_t pipeline_n,
   struct pvr_fragment_shader_state *const frag_state_out)
{
   const struct pvr_hard_coding_data *const data =
      pvr_get_hard_coding_data(dev_info);

   assert(data->type == PVR_HARD_CODE_SHADER_TYPE_GRAPHICS);
   assert(pipeline_n < data->graphics.shader_count);
   assert(data->graphics.flags & BITFIELD_BIT(MESA_SHADER_FRAGMENT));

   *frag_state_out = *data->graphics.frag_shader_states[0];
}

void pvr_hard_code_graphics_get_build_info(
   const struct pvr_device_info *const dev_info,
   uint32_t pipeline_n,
   gl_shader_stage stage,
   struct rogue_common_build_data *const common_build_data,
   struct rogue_build_data *const build_data,
   struct pvr_explicit_constant_usage *const explicit_const_usage)
{
   const struct pvr_hard_coding_data *const data =
      pvr_get_hard_coding_data(dev_info);

   assert(data->type == PVR_HARD_CODE_SHADER_TYPE_GRAPHICS);
   assert(pipeline_n < data->graphics.shader_count);
   assert(data->graphics.flags & BITFIELD_BIT(stage));

   switch (stage) {
   case MESA_SHADER_VERTEX:
      assert(data->graphics.build_infos[pipeline_n]->vert_common_data.temps ==
             data->graphics.vert_shader_states[pipeline_n]
                ->stage_state.pds_temps_count);

      assert(data->graphics.build_infos[pipeline_n]->vert_common_data.coeffs ==
             data->graphics.vert_shader_states[pipeline_n]
                ->stage_state.coefficient_size);

      build_data->vs = data->graphics.build_infos[pipeline_n]->stage_data.vs;
      *common_build_data =
         data->graphics.build_infos[pipeline_n]->vert_common_data;
      *explicit_const_usage =
         data->graphics.build_infos[pipeline_n]->vert_explicit_conts_usage;

      break;

   case MESA_SHADER_FRAGMENT:
      assert(data->graphics.build_infos[pipeline_n]->frag_common_data.temps ==
             data->graphics.frag_shader_states[pipeline_n]
                ->stage_state.pds_temps_count);

      assert(data->graphics.build_infos[pipeline_n]->frag_common_data.coeffs ==
             data->graphics.frag_shader_states[pipeline_n]
                ->stage_state.coefficient_size);

      build_data->fs = data->graphics.build_infos[pipeline_n]->stage_data.fs;
      *common_build_data =
         data->graphics.build_infos[pipeline_n]->frag_common_data;
      *explicit_const_usage =
         data->graphics.build_infos[pipeline_n]->frag_explicit_conts_usage;

      break;

   default:
      unreachable("Unsupported stage.");
   }
}

void pvr_hard_code_get_idfwdf_program(
   const struct pvr_device_info *const dev_info,
   struct util_dynarray *program_out,
   uint32_t *usc_shareds_out,
   uint32_t *usc_temps_out)
{
   static const uint8_t shader[] = { 0, 0, 0, 0, 0, 0, 0, 0 };

   mesa_loge("No hard coded idfwdf program. Returning empty program.");

   util_dynarray_append_mem(program_out, ARRAY_SIZE(shader), &shader[0]);

   *usc_shareds_out = 12U;
   *usc_temps_out = 4U;
}

void pvr_hard_code_get_passthrough_vertex_shader(
   const struct pvr_device_info *const dev_info,
   struct util_dynarray *program_out)
{
   static const uint8_t shader[] = { 0, 0, 0, 0, 0, 0, 0, 0 };

   mesa_loge(
      "No hard coded passthrough vertex shader. Returning empty shader.");

   util_dynarray_append_mem(program_out, ARRAY_SIZE(shader), &shader[0]);
};

/* Render target array (RTA). */
void pvr_hard_code_get_passthrough_rta_vertex_shader(
   const struct pvr_device_info *const dev_info,
   struct util_dynarray *program_out)
{
   uint32_t shader[] = { 0, 0, 0, 0, 0, 0, 0, 0 };

   util_dynarray_append_mem(program_out, ARRAY_SIZE(shader), &shader);

   mesa_loge("No hard coded passthrough rta vertex shader. Returning "
             "empty shader.");
}
