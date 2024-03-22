/*
 * Copyright © 2020 Corporation
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

#ifndef BRW_KERNEL_H
#define BRW_KERNEL_H

#include "brw_compiler.h"

struct disk_cache;

#ifdef __cplusplus
extern "C" {
#endif

/** Software interface for system values in kernels
 *
 * These are intended to go at the start of the kernel argument buffer.
 */
struct brw_kernel_sysvals {
   uint32_t num_work_groups[3];
   uint32_t pad[5];
};

struct brw_kernel_arg_desc {
   uint16_t offset;
   uint16_t size;
};

struct brw_kernel {
   struct brw_cs_prog_data prog_data;

   struct brw_compile_stats stats[3];

   uint16_t args_size;
   uint16_t arg_count;
   const struct brw_kernel_arg_desc *args;

   const void *code;
};

bool
brw_kernel_from_spirv(struct brw_compiler *compiler,
                      struct disk_cache *disk_cache,
                      struct brw_kernel *kernel,
                      void *log_data, void *mem_ctx,
                      const uint32_t *spirv, size_t spirv_size,
                      const char *entrypoint_name,
                      char **error_str);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* BRW_KERNEL_H */
