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

#ifndef __IR3_ASM_H__
#define __IR3_ASM_H__

#include "main.h"

#include "ir3/ir3_parser.h"
#include "ir3/ir3_shader.h"

struct ir3_kernel {
   struct kernel base;
   struct ir3_kernel_info info;
   struct backend *backend;
   struct ir3_shader_variant *v;
   void *bin;
};
define_cast(kernel, ir3_kernel);

struct ir3_kernel *ir3_asm_assemble(struct ir3_compiler *c, FILE *in);
void ir3_asm_disassemble(struct ir3_kernel *k, FILE *out);

#endif /* __IR3_ASM_H__ */
