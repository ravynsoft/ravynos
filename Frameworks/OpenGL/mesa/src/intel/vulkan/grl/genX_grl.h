/*
 * Copyright © 2021 Corporation
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

#ifndef ANV_GRL_H
#define ANV_GRL_H

#include "grl/grl_cl_kernel.h"
#include "genxml/gen_macros.h"

#ifdef __cplusplus
extern "C" {
#endif

struct anv_cmd_buffer;
struct anv_kernel_arg;

void
genX(grl_dispatch)(struct anv_cmd_buffer *cmd_buffer,
                   enum grl_cl_kernel kernel,
                   const uint32_t *global_size,
                   uint32_t arg_count,
                   const struct anv_kernel_arg *args);

void
genX(grl_load_rt_uuid)(uint8_t *out_uuid);

uint32_t
genX(grl_max_scratch_size)(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ANV_GRL_H */
