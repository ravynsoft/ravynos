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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <err.h>
#include <stdio.h>

#include "ir3.h"
#include "ir3_assembler.h"
#include "ir3_shader.h"

/*
 * A test for delay-slot calculation.  Each test specifies ir3 assembly
 * for one or more instructions and the last instruction that consumes
 * the previously produced values.  And the expected number of delay
 * slots that would be needed before that last instruction.  Any source
 * registers in the last instruction which are not written in a previous
 * instruction are not counted.
 */

/* clang-format off */
#define TEST(n, ...) { # __VA_ARGS__, n }
/* clang-format on */

static const struct test {
   const char *asmstr;
   unsigned expected_delay;
} tests[] = {
   /* clang-format off */
   TEST(6,
      add.f r0.x, r2.x, r2.y
      rsq r0.x, r0.x
   ),
   TEST(3,
      mov.f32f32 r0.x, c0.x
      mov.f32f32 r0.y, c0.y
      add.f r0.x, r0.x, r0.y
   ),
   TEST(2,
      mov.f32f32 r0.x, c0.x
      mov.f32f32 r0.y, c0.y
      mov.f32f32 r0.z, c0.z
      mad.f32 r0.x, r0.x, r0.y, r0.z
   ),
   TEST(0,
      mov.f32f32 r0.x, c0.x
      rcp r0.x, r0.y
      add.f r0.x, r0.x, c0.x
   ),
   TEST(2,
      mov.f32f32 r0.x, c0.x
      mov.f32f32 r0.y, c0.y
      (rpt1)add.f r0.x, (r)r0.x, (r)c0.x
   ),
   TEST(2,
      (rpt1)mov.f32f32 r0.x, c0.x
      (rpt1)add.f r0.x, (r)r0.x, (r)c0.x
   ),
   TEST(3,
      mov.f32f32 r0.y, c0.y
      mov.f32f32 r0.x, c0.x
      (rpt1)add.f r0.x, (r)r0.x, (r)c0.x
   ),
   TEST(1,
      (rpt2)mov.f32f32 r0.x, (r)c0.x
      add.f r0.x, r0.x, c0.x
   ),
   TEST(2,
      (rpt2)mov.f32f32 r0.x, (r)c0.x
      add.f r0.x, r0.x, r0.y
   ),
   TEST(2,
      (rpt1)mov.f32f32 r0.x, (r)c0.x
      (rpt1)add.f r0.x, (r)r0.x, c0.x
   ),
   TEST(1,
      (rpt1)mov.f32f32 r0.y, (r)c0.x
      (rpt1)add.f r0.x, (r)r0.x, c0.x
   ),
   TEST(3,
      (rpt1)mov.f32f32 r0.x, (r)c0.x
      (rpt1)add.f r0.x, (r)r0.y, c0.x
   ),
   /* clang-format on */
};

static struct ir3_shader *
parse_asm(struct ir3_compiler *c, const char *asmstr)
{
   struct ir3_kernel_info info = {};
   FILE *in = fmemopen((void *)asmstr, strlen(asmstr), "r");
   struct ir3_shader *shader = ir3_parse_asm(c, &info, in);

   fclose(in);

   if (!shader)
      errx(-1, "assembler failed");

   return shader;
}

/**
 * ir3_delay_calc_* relies on the src/dst wrmask being correct even for ALU
 * instructions, so this sets it here.
 *
 * Note that this is not clever enough to know how many src/dst there are
 * for various tex/mem instructions.  But the rules for tex consuming alu
 * are the same as sfu consuming alu.
 */
static void
fixup_wrmask(struct ir3 *ir)
{
   struct ir3_block *block = ir3_start_block(ir);

   foreach_instr_safe (instr, &block->instr_list) {
      instr->dsts[0]->wrmask = MASK(instr->repeat + 1);
      foreach_src (reg, instr) {
         if (reg->flags & (IR3_REG_CONST | IR3_REG_IMMED))
            continue;

         if (reg->flags & IR3_REG_R)
            reg->wrmask = MASK(instr->repeat + 1);
         else
            reg->wrmask = 1;
      }
   }
}

int
main(int argc, char **argv)
{
   struct ir3_compiler *c;
   int result = 0;

   struct fd_dev_id dev_id = {
         .gpu_id = 630,
   };

   c = ir3_compiler_create(NULL, &dev_id, fd_dev_info_raw(&dev_id), &(struct ir3_compiler_options){});

   for (int i = 0; i < ARRAY_SIZE(tests); i++) {
      const struct test *test = &tests[i];
      struct ir3_shader *shader = parse_asm(c, test->asmstr);
      struct ir3 *ir = shader->variants->ir;

      fixup_wrmask(ir);

      ir3_debug_print(ir, "AFTER fixup_wrmask");

      struct ir3_block *block =
         list_first_entry(&ir->block_list, struct ir3_block, node);
      struct ir3_instruction *last = NULL;

      foreach_instr_rev (instr, &block->instr_list) {
         if (is_meta(instr))
            continue;
         last = instr;
         break;
      }

      /* The delay calc is expecting the instr to not yet be added to the
       * block, so remove it from the block so that it doesn't get counted
       * in the distance from assigner:
       */
      list_delinit(&last->node);

      unsigned n = ir3_delay_calc(block, last, true);

      if (n != test->expected_delay) {
         printf("%d: FAIL: Expected delay %u, but got %u, for:\n%s\n", i,
                test->expected_delay, n, test->asmstr);
         result = -1;
      } else {
         printf("%d: PASS\n", i);
      }

      ir3_shader_destroy(shader);
   }

   ir3_compiler_destroy(c);

   return result;
}
