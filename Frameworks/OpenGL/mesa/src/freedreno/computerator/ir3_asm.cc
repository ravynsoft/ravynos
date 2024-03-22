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

#include "ir3/ir3_assembler.h"
#include "ir3/ir3_compiler.h"

#include "ir3_asm.h"

struct ir3_kernel *
ir3_asm_assemble(struct ir3_compiler *c, FILE *in)
{
   struct ir3_kernel *kernel = (struct ir3_kernel *)calloc(1, sizeof(*kernel));
   struct ir3_shader *shader = ir3_parse_asm(c, &kernel->info, in);
   if (!shader)
      errx(-1, "assembler failed");
   struct ir3_shader_variant *v = shader->variants;

   kernel->v = v;
   kernel->bin = v->bin;

   kernel->base.local_size[0] = v->local_size[0];
   kernel->base.local_size[1] = v->local_size[1];
   kernel->base.local_size[2] = v->local_size[2];
   kernel->base.num_bufs = kernel->info.num_bufs;
   memcpy(kernel->base.buf_sizes, kernel->info.buf_sizes,
          sizeof(kernel->base.buf_sizes));
   memcpy(kernel->base.buf_addr_regs, kernel->info.buf_addr_regs,
          sizeof(kernel->base.buf_addr_regs));

   unsigned sz = v->info.size;

   v->bo = fd_bo_new(c->dev, sz, 0, "%s", ir3_shader_stage(v));

   memcpy(fd_bo_map(v->bo), kernel->bin, sz);

   /* Always include shaders in kernel crash dumps. */
   fd_bo_mark_for_dump(v->bo);

   return kernel;
}

void
ir3_asm_disassemble(struct ir3_kernel *k, FILE *out)
{
   ir3_shader_disasm(k->v, (uint32_t *)k->bin, out);
}
