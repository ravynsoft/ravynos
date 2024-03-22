/*
 * Copyright (C) 2019 Connor Abbott <cwabbott0@gmail.com>
 * Copyright (C) 2019 Lyude Paul <thatslyude@gmail.com>
 * Copyright (C) 2019 Ryan Houdek <Sonicadvance1@gmail.com>
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

#include "bi_print_common.h"
#include "compiler.h"

static const char *
bi_reg_op_name(enum bifrost_reg_op op)
{
   switch (op) {
   case BIFROST_OP_IDLE:
      return "idle";
   case BIFROST_OP_READ:
      return "read";
   case BIFROST_OP_WRITE:
      return "write";
   case BIFROST_OP_WRITE_LO:
      return "write lo";
   case BIFROST_OP_WRITE_HI:
      return "write hi";
   default:
      return "invalid";
   }
}

void
bi_print_slots(bi_registers *regs, FILE *fp)
{
   for (unsigned i = 0; i < 2; ++i) {
      if (regs->enabled[i])
         fprintf(fp, "slot %u: %u\n", i, regs->slot[i]);
   }

   if (regs->slot23.slot2) {
      fprintf(fp, "slot 2 (%s%s): %u\n", bi_reg_op_name(regs->slot23.slot2),
              regs->slot23.slot2 >= BIFROST_OP_WRITE ? " FMA" : "",
              regs->slot[2]);
   }

   if (regs->slot23.slot3) {
      fprintf(fp, "slot 3 (%s %s): %u\n", bi_reg_op_name(regs->slot23.slot3),
              regs->slot23.slot3_fma ? "FMA" : "ADD", regs->slot[3]);
   }
}

void
bi_print_tuple(bi_tuple *tuple, FILE *fp)
{
   bi_instr *ins[2] = {tuple->fma, tuple->add};

   for (unsigned i = 0; i < 2; ++i) {
      fprintf(fp, (i == 0) ? "\t* " : "\t+ ");

      if (ins[i])
         bi_print_instr(ins[i], fp);
      else
         fprintf(fp, "NOP\n");
   }
}

void
bi_print_clause(bi_clause *clause, FILE *fp)
{
   fprintf(fp, "id(%u)", clause->scoreboard_id);

   if (clause->dependencies) {
      fprintf(fp, " wait(");

      for (unsigned i = 0; i < 8; ++i) {
         if (clause->dependencies & (1 << i))
            fprintf(fp, "%u ", i);
      }

      fprintf(fp, ")");
   }

   fprintf(fp, " %s", bi_flow_control_name(clause->flow_control));

   if (!clause->next_clause_prefetch)
      fprintf(fp, " no_prefetch");

   if (clause->staging_barrier)
      fprintf(fp, " osrb");

   if (clause->td)
      fprintf(fp, " td");

   if (clause->pcrel_idx != ~0)
      fprintf(fp, " pcrel(%u)", clause->pcrel_idx);

   fprintf(fp, "\n");

   for (unsigned i = 0; i < clause->tuple_count; ++i)
      bi_print_tuple(&clause->tuples[i], fp);

   if (clause->constant_count) {
      for (unsigned i = 0; i < clause->constant_count; ++i)
         fprintf(fp, "%" PRIx64 " ", clause->constants[i]);

      if (clause->branch_constant)
         fprintf(fp, "*");

      fprintf(fp, "\n");
   }

   fprintf(fp, "\n");
}

static void
bi_print_scoreboard_line(unsigned slot, const char *name, uint64_t mask,
                         FILE *fp)
{
   if (!mask)
      return;

   fprintf(fp, "slot %u %s:", slot, name);

   u_foreach_bit64(reg, mask)
      fprintf(fp, " r%" PRId64, reg);

   fprintf(fp, "\n");
}

static void
bi_print_scoreboard(struct bi_scoreboard_state *state, FILE *fp)
{
   for (unsigned i = 0; i < BI_NUM_SLOTS; ++i) {
      bi_print_scoreboard_line(i, "reads", state->read[i], fp);
      bi_print_scoreboard_line(i, "writes", state->write[i], fp);
   }
}

void
bi_print_block(bi_block *block, FILE *fp)
{
   if (block->scheduled) {
      bi_print_scoreboard(&block->scoreboard_in, fp);
      fprintf(fp, "\n");
   }

   fprintf(fp, "block%u {\n", block->index);

   if (block->scheduled) {
      bi_foreach_clause_in_block(block, clause)
         bi_print_clause(clause, fp);
   } else {
      bi_foreach_instr_in_block(block, ins)
         bi_print_instr((bi_instr *)ins, fp);
   }

   fprintf(fp, "}");

   if (block->successors[0]) {
      fprintf(fp, " -> ");

      bi_foreach_successor((block), succ)
         fprintf(fp, "block%u ", succ->index);
   }

   if (bi_num_predecessors(block)) {
      fprintf(fp, " from");

      bi_foreach_predecessor(block, pred)
         fprintf(fp, " block%u", (*pred)->index);
   }

   if (block->scheduled) {
      fprintf(fp, "\n");
      bi_print_scoreboard(&block->scoreboard_out, fp);
   }

   fprintf(fp, "\n\n");
}

void
bi_print_shader(bi_context *ctx, FILE *fp)
{
   bi_foreach_block(ctx, block)
      bi_print_block(block, fp);
}
