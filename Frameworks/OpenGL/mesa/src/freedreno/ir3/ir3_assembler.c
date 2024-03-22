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

#include "ir3_assembler.h"
#include "ir3_parser.h"
#include "ir3_shader.h"

/**
 * A helper to go from ir3 assembly to assembled shader.  The shader has a
 * single variant.
 */
struct ir3_shader *
ir3_parse_asm(struct ir3_compiler *c, struct ir3_kernel_info *info, FILE *in)
{
   struct ir3_shader *shader = rzalloc_size(NULL, sizeof(*shader));
   shader->compiler = c;
   shader->type = MESA_SHADER_COMPUTE;
   mtx_init(&shader->variants_lock, mtx_plain);

   struct ir3_shader_variant *v = rzalloc_size(shader, sizeof(*v));
   v->type = MESA_SHADER_COMPUTE;
   v->compiler = c;
   v->const_state = rzalloc_size(v, sizeof(*v->const_state));

   if (c->gen >= 6)
      v->mergedregs = true;

   shader->variants = v;
   shader->variant_count = 1;

   info->numwg = INVALID_REG;

   for (int i = 0; i < MAX_BUFS; i++) {
      info->buf_addr_regs[i] = INVALID_REG;
   }

   /* Provide a default local_size in case the shader doesn't set it, so that
    * we don't crash at least.
    */
   v->local_size[0] = v->local_size[1] = v->local_size[2] = 1;

   v->ir = ir3_parse(v, info, in);
   if (!v->ir)
      goto error;

   ir3_debug_print(v->ir, "AFTER PARSING");

   v->bin = ir3_shader_assemble(v);
   if (!v->bin)
      goto error;

   return shader;

error:
   ralloc_free(shader);
   return NULL;
}
