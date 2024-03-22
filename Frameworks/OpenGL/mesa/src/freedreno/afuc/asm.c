/*
 * Copyright (c) 2017 Rob Clark <robdclark@gmail.com>
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

#include <assert.h>
#include <err.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util/macros.h"
#include "util/log.h"
#include "afuc.h"
#include "asm.h"
#include "parser.h"
#include "util.h"

struct encode_state {
	unsigned gen;
};

static afuc_opc
__instruction_case(struct encode_state *s, struct afuc_instr *instr)
{
   switch (instr->opc) {
#define ALU(name) \
   case OPC_##name: \
      if (instr->has_immed) \
         return OPC_##name##I; \
      break;

   ALU(ADD)
   ALU(ADDHI)
   ALU(SUB)
   ALU(SUBHI)
   ALU(AND)
   ALU(OR)
   ALU(XOR)
   ALU(NOT)
   ALU(SHL)
   ALU(USHR)
   ALU(ISHR)
   ALU(ROT)
   ALU(MUL8)
   ALU(MIN)
   ALU(MAX)
   ALU(CMP)
#undef ALU

   default:
      break;
   }

   return instr->opc;
}

#include "encode.h"

int gpuver;

/* bit lame to hard-code max but fw sizes are small */
static struct afuc_instr instructions[0x2000];
static unsigned num_instructions;

static struct asm_label labels[0x512];
static unsigned num_labels;

struct afuc_instr *
next_instr(afuc_opc opc)
{
   struct afuc_instr *ai = &instructions[num_instructions++];
   assert(num_instructions < ARRAY_SIZE(instructions));
   ai->opc = opc;
   return ai;
}

void
decl_label(const char *str)
{
   struct asm_label *label = &labels[num_labels++];

   assert(num_labels < ARRAY_SIZE(labels));

   label->offset = num_instructions;
   label->label = str;
}

static int
resolve_label(const char *str)
{
   int i;

   for (i = 0; i < num_labels; i++) {
      struct asm_label *label = &labels[i];

      if (!strcmp(str, label->label)) {
         return label->offset;
      }
   }

   fprintf(stderr, "Undeclared label: %s\n", str);
   exit(2);
}

static void
emit_instructions(int outfd)
{
   int i;

   struct encode_state s = {
      .gen = gpuver,
   };

   /* there is an extra 0x00000000 which kernel strips off.. we could
    * perhaps use it for versioning.
    */
   i = 0;
   write(outfd, &i, 4);

   /* Expand some meta opcodes, and resolve branch targets */
   for (i = 0; i < num_instructions; i++) {
      struct afuc_instr *ai = &instructions[i];

      switch (ai->opc) {
      case OPC_BREQ:
         ai->offset = resolve_label(ai->label) - i;
         if (ai->has_bit)
            ai->opc = OPC_BREQB;
         else
            ai->opc = OPC_BREQI;
         break;

      case OPC_BRNE:
         ai->offset = resolve_label(ai->label) - i;
         if (ai->has_bit)
            ai->opc = OPC_BRNEB;
         else
            ai->opc = OPC_BRNEI;
         break;

      case OPC_JUMP:
         ai->offset = resolve_label(ai->label) - i;
         ai->opc = OPC_BRNEB;
         ai->src1 = 0;
         ai->bit = 0;
         break;

      case OPC_CALL:
      case OPC_PREEMPTLEAVE:
         ai->literal = resolve_label(ai->label);
         break;

      case OPC_MOVI:
         if (ai->label)
            ai->immed = resolve_label(ai->label);
         break;

      default:
         break;
      }

      /* special case, 2nd dword is patched up w/ # of instructions
       * (ie. offset of jmptbl)
       */
      if (i == 1) {
         assert(ai->opc == OPC_RAW_LITERAL);
         ai->literal &= ~0xffff;
         ai->literal |= num_instructions;
      }

      if (ai->opc == OPC_RAW_LITERAL) {
         write(outfd, &ai->literal, 4);
         continue;
      }

      uint32_t encoded = bitmask_to_uint64_t(encode__instruction(&s, NULL, ai));
      write(outfd, &encoded, 4);
   }
}

unsigned
parse_control_reg(const char *name)
{
   /* skip leading "@" */
   return afuc_control_reg(name + 1);
}

unsigned
parse_sqe_reg(const char *name)
{
   /* skip leading "%" */
   return afuc_sqe_reg(name + 1);
}

static void
emit_jumptable(int outfd)
{
   uint32_t jmptable[0x80] = {0};
   int i;

   for (i = 0; i < num_labels; i++) {
      struct asm_label *label = &labels[i];
      int id = afuc_pm4_id(label->label);

      /* if it doesn't match a known PM4 packet-id, try to match UNKN%d: */
      if (id < 0) {
         if (sscanf(label->label, "UNKN%d", &id) != 1) {
            /* if still not found, must not belong in jump-table: */
            continue;
         }
      }

      jmptable[id] = label->offset;
   }

   write(outfd, jmptable, sizeof(jmptable));
}

static void
usage(void)
{
   fprintf(stderr, "Usage:\n"
                   "\tasm [-g GPUVER] filename.asm filename.fw\n"
                   "\t\t-g - specify GPU version (5, etc)\n");
   exit(2);
}

int
main(int argc, char **argv)
{
   FILE *in;
   char *file, *outfile;
   int c, ret, outfd;

   /* Argument parsing: */
   while ((c = getopt(argc, argv, "g:")) != -1) {
      switch (c) {
      case 'g':
         gpuver = atoi(optarg);
         break;
      default:
         usage();
      }
   }

   if (optind >= (argc + 1)) {
      fprintf(stderr, "no file specified!\n");
      usage();
   }

   file = argv[optind];
   outfile = argv[optind + 1];

   outfd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
   if (outfd < 0) {
      fprintf(stderr, "could not open \"%s\"\n", outfile);
      usage();
   }

   in = fopen(file, "r");
   if (!in) {
      fprintf(stderr, "could not open \"%s\"\n", file);
      usage();
   }

   yyset_in(in);

   /* if gpu version not specified, infer from filename: */
   if (!gpuver) {
      if (strstr(file, "a5")) {
         gpuver = 5;
      } else if (strstr(file, "a6")) {
         gpuver = 6;
      } else if (strstr(file, "a7")) {
         gpuver = 7;
      }
   }

   ret = afuc_util_init(gpuver, false);
   if (ret < 0) {
      usage();
   }

   ret = yyparse();
   if (ret) {
      fprintf(stderr, "parse failed: %d\n", ret);
      return ret;
   }

   emit_instructions(outfd);
   emit_jumptable(outfd);

   close(outfd);

   return 0;
}
