/*
 * Copyright © 2020 Intel Corporation
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

#ifndef BRW_NIR_RT_H
#define BRW_NIR_RT_H

#include "brw_nir.h"
#include "brw_rt.h"

#ifdef __cplusplus
extern "C" {
#endif

void brw_nir_lower_raygen(nir_shader *nir);
void brw_nir_lower_any_hit(nir_shader *nir,
                           const struct intel_device_info *devinfo);
void brw_nir_lower_closest_hit(nir_shader *nir);
void brw_nir_lower_miss(nir_shader *nir);
void brw_nir_lower_callable(nir_shader *nir);
void brw_nir_lower_combined_intersection_any_hit(nir_shader *intersection,
                                                 const nir_shader *any_hit,
                                                 const struct intel_device_info *devinfo);

/* We reserve the first 16B of the stack for callee data pointers */
#define BRW_BTD_STACK_RESUME_BSR_ADDR_OFFSET 0
#define BRW_BTD_STACK_CALL_DATA_PTR_OFFSET 8
#define BRW_BTD_STACK_CALLEE_DATA_SIZE 16

/* We require the stack to be 8B aligned at the start of a shader */
#define BRW_BTD_STACK_ALIGN 8

bool brw_nir_lower_ray_queries(nir_shader *shader,
                               const struct intel_device_info *devinfo);

void brw_nir_lower_shader_returns(nir_shader *shader);

bool brw_nir_lower_shader_calls(nir_shader *shader, struct brw_bs_prog_key *key);

void brw_nir_lower_rt_intrinsics(nir_shader *shader,
                                 const struct intel_device_info *devinfo);
void brw_nir_lower_intersection_shader(nir_shader *intersection,
                                       const nir_shader *any_hit,
                                       const struct intel_device_info *devinfo);

nir_shader *
brw_nir_create_raygen_trampoline(const struct brw_compiler *compiler,
                                 void *mem_ctx);
nir_shader *
brw_nir_create_trivial_return_shader(const struct brw_compiler *compiler,
                                     void *mem_ctx);

#ifdef __cplusplus
}
#endif

#endif /* BRW_NIR_RT_H */
