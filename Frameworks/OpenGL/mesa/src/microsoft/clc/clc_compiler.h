/*
 * Copyright © Microsoft Corporation
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef CLC_COMPILER_H
#define CLC_COMPILER_H

#include "clc/clc.h"
#include "dxil_versions.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CLC_MAX_CONSTS 32
#define CLC_MAX_BINDINGS_PER_ARG 3
#define CLC_MAX_SAMPLERS 16

struct clc_printf_info {
   unsigned num_args;
   unsigned *arg_sizes;
   char *str;
};

struct clc_dxil_metadata {
   struct {
      unsigned offset;
      unsigned size;
      union {
         struct {
            unsigned buf_ids[CLC_MAX_BINDINGS_PER_ARG];
            unsigned num_buf_ids;
         } image;
         struct {
            unsigned sampler_id;
         } sampler;
         struct {
            unsigned buf_id;
         } globconstptr;
         struct {
            unsigned sharedmem_offset;
         } localptr;
      };
   } *args;
   unsigned kernel_inputs_cbv_id;
   unsigned kernel_inputs_buf_size;
   unsigned work_properties_cbv_id;
   size_t num_uavs;
   size_t num_srvs;
   size_t num_samplers;

   struct {
      void *data;
      size_t size;
      unsigned uav_id;
   } consts[CLC_MAX_CONSTS];
   size_t num_consts;

   struct {
      unsigned sampler_id;
      unsigned addressing_mode;
      unsigned normalized_coords;
      unsigned filter_mode;
   } const_samplers[CLC_MAX_SAMPLERS];
   size_t num_const_samplers;
   size_t local_mem_size;
   size_t priv_mem_size;

   uint16_t local_size[3];
   uint16_t local_size_hint[3];

   struct {
      unsigned info_count;
      struct clc_printf_info *infos;
      int uav_id;
   } printf;
};

struct clc_dxil_object {
   const struct clc_kernel_info *kernel;
   struct clc_dxil_metadata metadata;
   struct {
      void *data;
      size_t size;
   } binary;
};

struct clc_runtime_arg_info {
   union {
      struct {
         unsigned size;
      } localptr;
      struct {
         unsigned normalized_coords;
         unsigned addressing_mode; /* See SPIR-V spec for value meanings */
         unsigned linear_filtering;
      } sampler;
   };
};

struct clc_runtime_kernel_conf {
   uint16_t local_size[3];
   struct clc_runtime_arg_info *args;
   unsigned lower_bit_size;
   unsigned support_global_work_id_offsets;
   unsigned support_workgroup_id_offsets;

   enum dxil_shader_model max_shader_model;
   enum dxil_validator_version validator_version;
};

struct clc_libclc_dxil_options {
   unsigned optimize;
};

struct clc_libclc *
clc_libclc_new_dxil(const struct clc_logger *logger,
                    const struct clc_libclc_dxil_options *dxil_options);

bool
clc_spirv_to_dxil(struct clc_libclc *lib,
                  const struct clc_binary *linked_spirv,
                  const struct clc_parsed_spirv *parsed_data,
                  const char *entrypoint,
                  const struct clc_runtime_kernel_conf *conf,
                  const struct clc_spirv_specialization_consts *consts,
                  const struct clc_logger *logger,
                  struct clc_dxil_object *out_dxil);

void clc_free_dxil_object(struct clc_dxil_object *dxil);

/* This struct describes the layout of data expected in the CB bound at global_work_offset_cbv_id */
struct clc_work_properties_data {
   /* Returned from get_global_offset(), and added into get_global_id() */
   unsigned global_offset_x;
   unsigned global_offset_y;
   unsigned global_offset_z;
   /* Returned from get_work_dim() */
   unsigned work_dim;
   /* The number of work groups being launched (i.e. the parameters to Dispatch).
    * If the requested global size doesn't fit in a single Dispatch, these values should
    * indicate the total number of groups that *should* have been launched. */
   unsigned group_count_total_x;
   unsigned group_count_total_y;
   unsigned group_count_total_z;
   unsigned padding;
   /* If the requested global size doesn't fit in a single Dispatch, subsequent dispatches
    * should fill out these offsets to indicate how many groups have already been launched */
   unsigned group_id_offset_x;
   unsigned group_id_offset_y;
   unsigned group_id_offset_z;
};

uint64_t clc_compiler_get_version(void);

#ifdef __cplusplus
}
#endif

#endif
