/**************************************************************************
 *
 * Copyright 2003 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#include "util/log.h"
#include "util/ralloc.h"
#include "util/u_debug.h"
#include "i915_debug.h"
#include "i915_debug_private.h"
#include "i915_reg.h"

#define PRINTF ralloc_asprintf_append

static const char *opcodes[0x20] = {
   "NOP",     "ADD", "MOV",  "MUL",  "MAD",  "DP2ADD", "DP3",    "DP4",
   "FRC",     "RCP", "RSQ",  "EXP",  "LOG",  "CMP",    "MIN",    "MAX",
   "FLR",     "MOD", "TRC",  "SGE",  "SLT",  "TEXLD",  "TEXLDP", "TEXLDB",
   "TEXKILL", "DCL", "0x1a", "0x1b", "0x1c", "0x1d",   "0x1e",   "0x1f",
};

static const int args[0x20] = {
   0, /* 0 nop */
   2, /* 1 add */
   1, /* 2 mov */
   2, /* 3 m ul */
   3, /* 4 mad */
   3, /* 5 dp2add */
   2, /* 6 dp3 */
   2, /* 7 dp4 */
   1, /* 8 frc */
   1, /* 9 rcp */
   1, /* a rsq */
   1, /* b exp */
   1, /* c log */
   3, /* d cmp */
   2, /* e min */
   2, /* f max */
   1, /* 10 flr */
   1, /* 11 mod */
   1, /* 12 trc */
   2, /* 13 sge */
   2, /* 14 slt */
   1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
};

static const char *regname[0x8] = {
   "R", "T", "CONST", "S", "OC", "OD", "U", "UNKNOWN",
};

static void
print_reg_type_nr(char **stream, unsigned type, unsigned nr)
{
   switch (type) {
   case REG_TYPE_T:
      switch (nr) {
      case T_DIFFUSE:
         PRINTF(stream, "T_DIFFUSE");
         return;
      case T_SPECULAR:
         PRINTF(stream, "T_SPECULAR");
         return;
      case T_FOG_W:
         PRINTF(stream, "T_FOG_W");
         return;
      default:
         PRINTF(stream, "T_TEX%d", nr);
         return;
      }
   case REG_TYPE_OC:
      if (nr == 0) {
         PRINTF(stream, "oC");
         return;
      }
      break;
   case REG_TYPE_OD:
      if (nr == 0) {
         PRINTF(stream, "oD");
         return;
      }
      break;
   default:
      break;
   }

   PRINTF(stream, "%s[%d]", regname[type], nr);
}

#define REG_SWIZZLE_MASK 0x7777
#define REG_NEGATE_MASK  0x8888

#define REG_SWIZZLE_XYZW                                                       \
   ((SRC_X << A2_SRC2_CHANNEL_X_SHIFT) | (SRC_Y << A2_SRC2_CHANNEL_Y_SHIFT) |  \
    (SRC_Z << A2_SRC2_CHANNEL_Z_SHIFT) | (SRC_W << A2_SRC2_CHANNEL_W_SHIFT))

static void
print_reg_neg_swizzle(char **stream, unsigned reg)
{
   int i;

   if ((reg & REG_SWIZZLE_MASK) == REG_SWIZZLE_XYZW &&
       (reg & REG_NEGATE_MASK) == 0)
      return;

   PRINTF(stream, ".");

   for (i = 3; i >= 0; i--) {
      if (reg & (1 << ((i * 4) + 3)))
         PRINTF(stream, "-");

      switch ((reg >> (i * 4)) & 0x7) {
      case 0:
         PRINTF(stream, "x");
         break;
      case 1:
         PRINTF(stream, "y");
         break;
      case 2:
         PRINTF(stream, "z");
         break;
      case 3:
         PRINTF(stream, "w");
         break;
      case 4:
         PRINTF(stream, "0");
         break;
      case 5:
         PRINTF(stream, "1");
         break;
      default:
         PRINTF(stream, "?");
         break;
      }
   }
}

static void
print_src_reg(char **stream, unsigned dword)
{
   unsigned nr = (dword >> A2_SRC2_NR_SHIFT) & REG_NR_MASK;
   unsigned type = (dword >> A2_SRC2_TYPE_SHIFT) & REG_TYPE_MASK;
   print_reg_type_nr(stream, type, nr);
   print_reg_neg_swizzle(stream, dword);
}

static void
print_dest_reg(char **stream, unsigned dword)
{
   unsigned nr = (dword >> A0_DEST_NR_SHIFT) & REG_NR_MASK;
   unsigned type = (dword >> A0_DEST_TYPE_SHIFT) & REG_TYPE_MASK;
   print_reg_type_nr(stream, type, nr);
   if ((dword & A0_DEST_CHANNEL_ALL) == A0_DEST_CHANNEL_ALL)
      return;
   PRINTF(stream, ".");
   if (dword & A0_DEST_CHANNEL_X)
      PRINTF(stream, "x");
   if (dword & A0_DEST_CHANNEL_Y)
      PRINTF(stream, "y");
   if (dword & A0_DEST_CHANNEL_Z)
      PRINTF(stream, "z");
   if (dword & A0_DEST_CHANNEL_W)
      PRINTF(stream, "w");
}

#define GET_SRC0_REG(r0, r1) ((r0 << 14) | (r1 >> A1_SRC0_CHANNEL_W_SHIFT))
#define GET_SRC1_REG(r0, r1) ((r0 << 8) | (r1 >> A2_SRC1_CHANNEL_W_SHIFT))
#define GET_SRC2_REG(r)      (r)

static void
print_arith_op(char **stream, unsigned opcode, const unsigned *program)
{
   if (opcode != A0_NOP) {
      print_dest_reg(stream, program[0]);
      if (program[0] & A0_DEST_SATURATE)
         PRINTF(stream, " = SATURATE ");
      else
         PRINTF(stream, " = ");
   }

   PRINTF(stream, "%s ", opcodes[opcode]);

   print_src_reg(stream, GET_SRC0_REG(program[0], program[1]));
   if (args[opcode] == 1)
      return;

   PRINTF(stream, ", ");
   print_src_reg(stream, GET_SRC1_REG(program[1], program[2]));
   if (args[opcode] == 2)
      return;

   PRINTF(stream, ", ");
   print_src_reg(stream, GET_SRC2_REG(program[2]));
   return;
}

static void
print_tex_op(char **stream, unsigned opcode, const unsigned *program)
{
   print_dest_reg(stream, program[0] | A0_DEST_CHANNEL_ALL);
   PRINTF(stream, " = ");

   PRINTF(stream, "%s ", opcodes[opcode]);

   PRINTF(stream, "S[%d],", program[0] & T0_SAMPLER_NR_MASK);

   print_reg_type_nr(stream,
                     (program[1] >> T1_ADDRESS_REG_TYPE_SHIFT) & REG_TYPE_MASK,
                     (program[1] >> T1_ADDRESS_REG_NR_SHIFT) & REG_NR_MASK);
}

static void
print_texkil_op(char **stream, unsigned opcode, const unsigned *program)
{
   PRINTF(stream, "TEXKIL ");

   print_reg_type_nr(stream,
                     (program[1] >> T1_ADDRESS_REG_TYPE_SHIFT) & REG_TYPE_MASK,
                     (program[1] >> T1_ADDRESS_REG_NR_SHIFT) & REG_NR_MASK);
}

static void
print_dcl_op(char **stream, unsigned opcode, const unsigned *program)
{
   unsigned type = (program[0] >> D0_TYPE_SHIFT) & REG_TYPE_MASK;

   PRINTF(stream, "%s ", opcodes[opcode]);

   unsigned dest_dword = program[0];
   if (type == REG_TYPE_S)
      dest_dword |= A0_DEST_CHANNEL_ALL;
   print_dest_reg(stream, dest_dword);

   if (type == REG_TYPE_S) {
      switch (program[0] & D0_SAMPLE_TYPE_MASK) {
      case D0_SAMPLE_TYPE_2D:
         PRINTF(stream, " 2D");
         break;
      case D0_SAMPLE_TYPE_VOLUME:
         PRINTF(stream, " 3D");
         break;
      case D0_SAMPLE_TYPE_CUBE:
         PRINTF(stream, " CUBE");
         break;
      default:
         PRINTF(stream, " XXX bad type");
         break;
      }
   }
}

void
i915_disassemble_program(const unsigned *program, unsigned sz)
{
   unsigned i;

   mesa_logi("\t\tBEGIN");

   assert((program[0] & 0x1ff) + 2 == sz);

   program++;
   for (i = 1; i < sz; i += 3, program += 3) {
      unsigned opcode = program[0] & (0x1f << 24);

      char *stream = ralloc_strdup(NULL, "");
      if ((int)opcode >= A0_NOP && opcode <= A0_SLT)
         print_arith_op(&stream, opcode >> 24, program);
      else if (opcode >= T0_TEXLD && opcode < T0_TEXKILL)
         print_tex_op(&stream, opcode >> 24, program);
      else if (opcode == T0_TEXKILL)
         print_texkil_op(&stream, opcode >> 24, program);
      else if (opcode == D0_DCL)
         print_dcl_op(&stream, opcode >> 24, program);
      else
         ralloc_asprintf_append(&stream, "\t\t Unknown opcode 0x%x\n", opcode);

      mesa_logi("\t\t %s ", stream);
      ralloc_free(stream);
   }

   mesa_logi("\t\tEND");
}
