/*
 * Copyright (C) 2018-2019 Alyssa Rosenzweig <alyssa@rosenzweig.io>
 * Copyright (C) 2019-2020 Collabora, Ltd.
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

#include <math.h>

#include "util/bitscan.h"
#include "util/half_float.h"
#include "compiler.h"
#include "helpers.h"
#include "midgard_ops.h"

/* Pretty printer for Midgard IR, for use debugging compiler-internal
 * passes like register allocation. The output superficially resembles
 * Midgard assembly, with the exception that unit information and such is
 * (normally) omitted, and generic indices are usually used instead of
 * registers */

static void
mir_print_index(int source)
{
   if (source == ~0) {
      printf("_");
      return;
   }

   if (source >= SSA_FIXED_MINIMUM) {
      /* Specific register */
      int reg = SSA_REG_FROM_FIXED(source);

      /* TODO: Moving threshold */
      if (reg > 16 && reg < 24)
         printf("U%d", 23 - reg);
      else
         printf("R%d", reg);
   } else if (source & PAN_IS_REG) {
      printf("r%d", source >> 1);
   } else {
      printf("%d", source >> 1);
   }
}

static const char components[16] = "xyzwefghijklmnop";

static void
mir_print_mask(unsigned mask)
{
   printf(".");

   for (unsigned i = 0; i < 16; ++i) {
      if (mask & (1 << i))
         putchar(components[i]);
   }
}

/*
 * Print a swizzle. We only print the components enabled by the corresponding
 * writemask, as the other components will be ignored by the hardware and so
 * don't matter.
 */
static void
mir_print_swizzle(unsigned mask, unsigned *swizzle)
{
   printf(".");

   for (unsigned i = 0; i < 16; ++i) {
      if (mask & BITFIELD_BIT(i)) {
         unsigned C = swizzle[i];
         putchar(components[C]);
      }
   }
}

static const char *
mir_get_unit(unsigned unit)
{
   switch (unit) {
   case ALU_ENAB_VEC_MUL:
      return "vmul";
   case ALU_ENAB_SCAL_ADD:
      return "sadd";
   case ALU_ENAB_VEC_ADD:
      return "vadd";
   case ALU_ENAB_SCAL_MUL:
      return "smul";
   case ALU_ENAB_VEC_LUT:
      return "lut";
   case ALU_ENAB_BR_COMPACT:
      return "br";
   case ALU_ENAB_BRANCH:
      return "brx";
   default:
      return "???";
   }
}

static void
mir_print_embedded_constant(midgard_instruction *ins, unsigned src_idx)
{
   assert(src_idx <= 1);

   unsigned base_size = max_bitsize_for_alu(ins);
   unsigned sz = nir_alu_type_get_type_size(ins->src_types[src_idx]);
   bool half = (sz == (base_size >> 1));
   unsigned mod = mir_pack_mod(ins, src_idx, false);
   unsigned *swizzle = ins->swizzle[src_idx];
   midgard_reg_mode reg_mode = reg_mode_for_bitsize(max_bitsize_for_alu(ins));
   unsigned comp_mask = effective_writemask(ins->op, ins->mask);
   unsigned num_comp = util_bitcount(comp_mask);
   unsigned max_comp = mir_components_for_type(ins->dest_type);
   bool first = true;

   printf("#");

   if (num_comp > 1)
      printf("vec%d(", num_comp);

   for (unsigned comp = 0; comp < max_comp; comp++) {
      if (!(comp_mask & (1 << comp)))
         continue;

      if (first)
         first = false;
      else
         printf(", ");

      mir_print_constant_component(stdout, &ins->constants, swizzle[comp],
                                   reg_mode, half, mod, ins->op);
   }

   if (num_comp > 1)
      printf(")");
}

static void
mir_print_src(midgard_instruction *ins, unsigned c)
{
   mir_print_index(ins->src[c]);

   if (ins->src[c] != ~0 && ins->src_types[c] != nir_type_invalid) {
      pan_print_alu_type(ins->src_types[c], stdout);
      mir_print_swizzle(ins->mask, ins->swizzle[c]);
   }
}

void
mir_print_instruction(midgard_instruction *ins)
{
   printf("\t");

   if (midgard_is_branch_unit(ins->unit)) {
      const char *branch_target_names[] = {"goto", "break", "continue",
                                           "discard"};

      printf("%s.", mir_get_unit(ins->unit));
      if (ins->branch.target_type == TARGET_DISCARD)
         printf("discard.");
      else if (ins->writeout)
         printf("write.");
      else if (ins->unit == ALU_ENAB_BR_COMPACT && !ins->branch.conditional)
         printf("uncond.");
      else
         printf("cond.");

      if (!ins->branch.conditional)
         printf("always");
      else if (ins->branch.invert_conditional)
         printf("false");
      else
         printf("true");

      if (ins->writeout) {
         printf(" (c: ");
         mir_print_src(ins, 0);
         printf(", z: ");
         mir_print_src(ins, 2);
         printf(", s: ");
         mir_print_src(ins, 3);
         printf(")");
      }

      if (ins->branch.target_type != TARGET_DISCARD)
         printf(" %s -> block(%d)\n",
                ins->branch.target_type < 4
                   ? branch_target_names[ins->branch.target_type]
                   : "??",
                ins->branch.target_block);

      return;
   }

   switch (ins->type) {
   case TAG_ALU_4: {
      midgard_alu_op op = ins->op;
      const char *name = alu_opcode_props[op].name;

      if (ins->unit)
         printf("%s.", mir_get_unit(ins->unit));

      printf("%s", name ? name : "??");

      if (!(midgard_is_integer_out_op(ins->op) &&
            ins->outmod == midgard_outmod_keeplo)) {
         mir_print_outmod(stdout, ins->outmod,
                          midgard_is_integer_out_op(ins->op));
      }

      break;
   }

   case TAG_LOAD_STORE_4: {
      midgard_load_store_op op = ins->op;
      const char *name = load_store_opcode_props[op].name;

      assert(name);
      printf("%s", name);
      break;
   }

   case TAG_TEXTURE_4: {
      printf("TEX");

      if (ins->helper_terminate)
         printf(".terminate");

      if (ins->helper_execute)
         printf(".execute");

      break;
   }

   default:
      assert(0);
   }

   if (ins->compact_branch && ins->branch.invert_conditional)
      printf(".not");

   printf(" ");
   mir_print_index(ins->dest);

   if (ins->dest != ~0) {
      pan_print_alu_type(ins->dest_type, stdout);
      mir_print_mask(ins->mask);
   }

   printf(", ");

   /* Only ALU can have an embedded constant, r26 as read on load/store is
    * something else entirely */
   bool is_alu = ins->type == TAG_ALU_4;
   unsigned r_constant = SSA_FIXED_REGISTER(REGISTER_CONSTANT);

   if (is_alu && alu_opcode_props[ins->op].props & QUIRK_FLIPPED_R24) {
      /* Moves (indicated by QUIRK_FLIPPED_R24) are 1-src, with their
       * one source in the second slot
       */
      assert(ins->src[0] == ~0);
   } else {
      if (ins->src[0] == r_constant && is_alu)
         mir_print_embedded_constant(ins, 0);
      else
         mir_print_src(ins, 0);

      printf(", ");
   }

   if (ins->has_inline_constant)
      printf("#%d", ins->inline_constant);
   else if (ins->src[1] == r_constant && is_alu)
      mir_print_embedded_constant(ins, 1);
   else
      mir_print_src(ins, 1);

   if (is_alu) {
      /* ALU ops are all 2-src, though CSEL is treated like a 3-src
       * pseudo op with the third source scheduler lowered
       */
      switch (ins->op) {
      case midgard_alu_op_icsel:
      case midgard_alu_op_fcsel:
      case midgard_alu_op_icsel_v:
      case midgard_alu_op_fcsel_v:
         printf(", ");
         mir_print_src(ins, 2);
         break;
      default:
         assert(ins->src[2] == ~0);
         break;
      }

      assert(ins->src[3] == ~0);
   } else {
      for (unsigned c = 2; c <= 3; ++c) {
         printf(", ");
         mir_print_src(ins, c);
      }
   }

   if (ins->no_spill)
      printf(" /* no spill */");

   printf("\n");
}

/* Dumps MIR for a block or entire shader respective */

void
mir_print_block(midgard_block *block)
{
   printf("block%u: {\n", block->base.name);

   if (block->scheduled) {
      mir_foreach_bundle_in_block(block, bundle) {
         for (unsigned i = 0; i < bundle->instruction_count; ++i)
            mir_print_instruction(bundle->instructions[i]);

         printf("\n");
      }
   } else {
      mir_foreach_instr_in_block(block, ins) {
         mir_print_instruction(ins);
      }
   }

   printf("}");

   if (block->base.successors[0]) {
      printf(" -> ");
      pan_foreach_successor((&block->base), succ)
         printf(" block%u ", succ->name);
   }

   printf(" from { ");
   mir_foreach_predecessor(block, pred)
      printf("block%u ", pred->base.name);
   printf("}");

   printf("\n\n");
}

void
mir_print_shader(compiler_context *ctx)
{
   mir_foreach_block(ctx, block) {
      mir_print_block((midgard_block *)block);
   }
}
