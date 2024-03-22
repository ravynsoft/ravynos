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

#include "pan_indirect_dispatch.h"
#include <stdio.h>
#include "compiler/nir/nir_builder.h"
#include "util/macros.h"
#include "util/u_memory.h"
#include "pan_bo.h"
#include "pan_encoder.h"
#include "pan_jc.h"
#include "pan_pool.h"
#include "pan_shader.h"
#include "pan_util.h"

#define get_input_field(b, name)                                               \
   nir_load_push_constant(                                                     \
      b, 1, sizeof(((struct pan_indirect_dispatch_info *)0)->name) * 8,        \
      nir_imm_int(b, 0),                                                       \
      .base = offsetof(struct pan_indirect_dispatch_info, name))

static mali_ptr
get_rsd(const struct panfrost_device *dev)
{
   return dev->indirect_dispatch.descs->ptr.gpu;
}

static mali_ptr
get_tls(const struct panfrost_device *dev)
{
   return dev->indirect_dispatch.descs->ptr.gpu + pan_size(RENDERER_STATE);
}

static void
pan_indirect_dispatch_init(struct panfrost_device *dev)
{
   nir_builder b = nir_builder_init_simple_shader(
      MESA_SHADER_COMPUTE, GENX(pan_shader_get_compiler_options)(), "%s",
      "indirect_dispatch");
   nir_def *zero = nir_imm_int(&b, 0);
   nir_def *one = nir_imm_int(&b, 1);
   nir_def *num_wg =
      nir_load_global(&b, get_input_field(&b, indirect_dim), 4, 3, 32);
   nir_def *num_wg_x = nir_channel(&b, num_wg, 0);
   nir_def *num_wg_y = nir_channel(&b, num_wg, 1);
   nir_def *num_wg_z = nir_channel(&b, num_wg, 2);

   nir_def *job_hdr_ptr = get_input_field(&b, job);
   nir_def *num_wg_flat =
      nir_imul(&b, num_wg_x, nir_imul(&b, num_wg_y, num_wg_z));

   nir_push_if(&b, nir_ieq(&b, num_wg_flat, zero));
   {
      nir_def *type_ptr = nir_iadd(&b, job_hdr_ptr, nir_imm_int64(&b, 4 * 4));
      nir_def *ntype = nir_imm_intN_t(&b, (MALI_JOB_TYPE_NULL << 1) | 1, 8);
      nir_store_global(&b, type_ptr, 1, ntype, 1);
   }
   nir_push_else(&b, NULL);
   {
      nir_def *job_dim_ptr = nir_iadd(
         &b, job_hdr_ptr,
         nir_imm_int64(&b, pan_section_offset(COMPUTE_JOB, INVOCATION)));
      nir_def *num_wg_x_m1 = nir_isub(&b, num_wg_x, one);
      nir_def *num_wg_y_m1 = nir_isub(&b, num_wg_y, one);
      nir_def *num_wg_z_m1 = nir_isub(&b, num_wg_z, one);
      nir_def *job_dim = nir_load_global(&b, job_dim_ptr, 8, 2, 32);
      nir_def *dims = nir_channel(&b, job_dim, 0);
      nir_def *split = nir_channel(&b, job_dim, 1);
      nir_def *num_wg_x_split =
         nir_iand_imm(&b, nir_ushr_imm(&b, split, 10), 0x3f);
      nir_def *num_wg_y_split = nir_iadd(
         &b, num_wg_x_split, nir_isub_imm(&b, 32, nir_uclz(&b, num_wg_x_m1)));
      nir_def *num_wg_z_split = nir_iadd(
         &b, num_wg_y_split, nir_isub_imm(&b, 32, nir_uclz(&b, num_wg_y_m1)));
      split =
         nir_ior(&b, split,
                 nir_ior(&b, nir_ishl(&b, num_wg_y_split, nir_imm_int(&b, 16)),
                         nir_ishl(&b, num_wg_z_split, nir_imm_int(&b, 22))));
      dims =
         nir_ior(&b, dims,
                 nir_ior(&b, nir_ishl(&b, num_wg_x_m1, num_wg_x_split),
                         nir_ior(&b, nir_ishl(&b, num_wg_y_m1, num_wg_y_split),
                                 nir_ishl(&b, num_wg_z_m1, num_wg_z_split))));

      nir_store_global(&b, job_dim_ptr, 8, nir_vec2(&b, dims, split), 3);

      nir_def *num_wg_x_ptr = get_input_field(&b, num_wg_sysval[0]);

      nir_push_if(&b, nir_ine_imm(&b, num_wg_x_ptr, 0));
      {
         nir_store_global(&b, num_wg_x_ptr, 8, num_wg_x, 1);
         nir_store_global(&b, get_input_field(&b, num_wg_sysval[1]), 8,
                          num_wg_y, 1);
         nir_store_global(&b, get_input_field(&b, num_wg_sysval[2]), 8,
                          num_wg_z, 1);
      }
      nir_pop_if(&b, NULL);
   }

   nir_pop_if(&b, NULL);

   struct panfrost_compile_inputs inputs = {
      .gpu_id = panfrost_device_gpu_id(dev),
      .no_ubo_to_push = true,
   };
   struct pan_shader_info shader_info;
   struct util_dynarray binary;

   util_dynarray_init(&binary, NULL);
   pan_shader_preprocess(b.shader, inputs.gpu_id);
   GENX(pan_shader_compile)(b.shader, &inputs, &binary, &shader_info);

   ralloc_free(b.shader);

   assert(!shader_info.tls_size);
   assert(!shader_info.wls_size);

   shader_info.push.count =
      DIV_ROUND_UP(sizeof(struct pan_indirect_dispatch_info), 4);

   dev->indirect_dispatch.bin = panfrost_bo_create(
      dev, binary.size, PAN_BO_EXECUTE, "Indirect dispatch shader");

   memcpy(dev->indirect_dispatch.bin->ptr.cpu, binary.data, binary.size);
   util_dynarray_fini(&binary);

   dev->indirect_dispatch.descs = panfrost_bo_create(
      dev, pan_size(RENDERER_STATE) + pan_size(LOCAL_STORAGE), 0,
      "Indirect dispatch descriptors");

   mali_ptr address = dev->indirect_dispatch.bin->ptr.gpu;

   void *rsd = dev->indirect_dispatch.descs->ptr.cpu;
   pan_pack(rsd, RENDERER_STATE, cfg) {
      pan_shader_prepare_rsd(&shader_info, address, &cfg);
   }

   void *tsd = dev->indirect_dispatch.descs->ptr.cpu + pan_size(RENDERER_STATE);
   pan_pack(tsd, LOCAL_STORAGE, ls) {
      ls.wls_instances = MALI_LOCAL_STORAGE_NO_WORKGROUP_MEM;
   };
}

unsigned
GENX(pan_indirect_dispatch_emit)(struct pan_pool *pool, struct pan_jc *jc,
                                 const struct pan_indirect_dispatch_info *inputs)
{
   struct panfrost_device *dev = pool->dev;
   struct panfrost_ptr job = pan_pool_alloc_desc(pool, COMPUTE_JOB);
   void *invocation = pan_section_ptr(job.cpu, COMPUTE_JOB, INVOCATION);

   /* If we haven't compiled the indirect dispatch shader yet, do it now */
   if (!dev->indirect_dispatch.bin)
      pan_indirect_dispatch_init(dev);

   panfrost_pack_work_groups_compute(invocation, 1, 1, 1, 1, 1, 1, false,
                                     false);

   pan_section_pack(job.cpu, COMPUTE_JOB, PARAMETERS, cfg) {
      cfg.job_task_split = 2;
   }

   pan_section_pack(job.cpu, COMPUTE_JOB, DRAW, cfg) {
      cfg.state = get_rsd(dev);
      cfg.thread_storage = get_tls(pool->dev);
      cfg.push_uniforms =
         pan_pool_upload_aligned(pool, inputs, sizeof(*inputs), 16);
   }

   return pan_jc_add_job(pool, jc, MALI_JOB_TYPE_COMPUTE, false, true, 0, 0,
                         &job, false);
}

void
GENX(pan_indirect_dispatch_cleanup)(struct panfrost_device *dev)
{
   panfrost_bo_unreference(dev->indirect_dispatch.bin);
   panfrost_bo_unreference(dev->indirect_dispatch.descs);
}
