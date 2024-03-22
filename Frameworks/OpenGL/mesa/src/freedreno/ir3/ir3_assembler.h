/*
 * Copyright © 2020 Google, Inc.
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

#ifndef __IR3_ASSEMBLER_H__
#define __IR3_ASSEMBLER_H__

#include <stdint.h>
#include <stdio.h>

#define MAX_BUFS 4

#ifdef __cplusplus
extern "C" {
#endif

struct ir3_kernel_info {
   uint32_t num_bufs;
   uint32_t buf_sizes[MAX_BUFS]; /* size in dwords */
   uint32_t buf_addr_regs[MAX_BUFS];

   uint64_t shader_print_buffer_iova;

   /* driver-param / replaced uniforms: */
   unsigned numwg;
   unsigned wgid;
   unsigned early_preamble;
};

struct ir3_shader;
struct ir3_compiler;

struct ir3_shader *ir3_parse_asm(struct ir3_compiler *c,
                                 struct ir3_kernel_info *info, FILE *in);

#ifdef __cplusplus
}
#endif

#endif /* __IR3_ASSEMBLER_H__ */
