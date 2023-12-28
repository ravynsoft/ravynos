/* Opcode table header for Visium.

   Copyright (C) 2003-2023 Free Software Foundation, Inc.

   This file is part of GDB, GAS, and GNU binutils.

   GDB, GAS and the GNU binutils are free software; you can redistribute
   them and/or modify them under the terms of the GNU General Public
   License as published by the Free Software Foundation; either version 3,
   or (at your option) any later version.

   GDB, GAS, and the GNU binutils are distributed in the hope that they
   will be useful, but WITHOUT ANY WARRANTY; without even the implied
   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
   the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this file; see the file COPYING3.  If not, write to the Free
   Software Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

enum visium_opcode_arch_val
{
  VISIUM_OPCODE_ARCH_DEF = 0,
  VISIUM_OPCODE_ARCH_GR5,
  VISIUM_OPCODE_ARCH_GR6,
  VISIUM_OPCODE_ARCH_BAD
};

/* The highest architecture in the table.  */
#define VISIUM_OPCODE_ARCH_MAX (VISIUM_OPCODE_ARCH_BAD - 1)

/* Given an enum visium_opcode_arch_val, return the bitmask to use in
   insn encoding/decoding.  */
#define VISIUM_OPCODE_ARCH_MASK(arch) (1 << (arch))

/* Some defines to make life easy.  */
#define MASK_DEF VISIUM_OPCODE_ARCH_MASK (VISIUM_OPCODE_ARCH_DEF)
#define MASK_GR5 VISIUM_OPCODE_ARCH_MASK (VISIUM_OPCODE_ARCH_GR5)
#define MASK_GR6 VISIUM_OPCODE_ARCH_MASK (VISIUM_OPCODE_ARCH_GR6)

/* Bit masks of architectures supporting the insn.  */
#define def (MASK_DEF | MASK_GR5 | MASK_GR6)
#define gr5 (MASK_GR5 | MASK_GR6)
#define gr6 (MASK_GR6)

/* The condition code field is not used (zero) for most instructions.
   BRR and BRA make normal use of it. Floating point instructions use
   it as a sub-opcode.  */
#define CC_MASK (0xf << 27)

/* It seems a shame not to use these bits in a class 0 instruction,
   since they could be used to extend the range of the branch.  */
#define CLASS0_UNUSED_MASK (0x1f << 16)

/* For class 1 instructions the following bit is unused.  */
#define CLASS1_UNUSED_MASK (1 << 9)

/* For class 1 instructions this field gives the index for a write
   instruction, the specific operation for an EAM instruction, or
   the floating point destination register for a floating point
   instruction.  */
#define CLASS1_INDEX_MASK (0x1f << 10)

/* For class 3 instructions the following field gives the destination
   general register.  */
#define CLASS3_DEST_MASK (0x1f << 10)

/* For class 1 and class 3 instructions the following bit selects an
   EAM write/read rather than a memory write/read.  */
#define EAM_SELECT_MASK (1 << 15)

/* Floating point instructions are distinguished from general EAM
   instructions by the following bit.  */
#define FP_SELECT_MASK (1 << 3)

/* For both class 1 and class 3 the following fields give, where
   appropriate the srcA and srcB registers whether floating point
   or general.  */
#define SRCA_MASK (0x1f << 16)
#define SRCB_MASK (0x1f << 4)

/* The class 3 interrupt bit. It turns a BRA into a SYS1, and an
   RFLAG into a SYS2. This bit should not be set in the user's
   class 3 instructions. This bit is also used in class 3
   to distinguish between floating point and other EAM operations.
   (see FP_SELECT_MASK).  */
#define CLASS3_INT (1 << 3)

/* Class 3 shift instructions use this bit to indicate that the
   srcB field is a 5 bit immediate shift count rather than a
   register number.  */
#define CLASS3_SOURCEB_IMMED (1 << 9)

#define BMD 0x02630004
#define BMI 0x82230004
#define DSI 0x82800004
#define ENI 0x02a00004
#define RFI 0x82fe01d4

struct reg_entry
{
  const char *name;
  unsigned char code;
};

static const struct reg_entry gen_reg_table[] ATTRIBUTE_UNUSED =
{
  {"fp", 0x16},
  {"r0", 0x0},
  {"r1", 0x1},
  {"r10", 0xA},
  {"r11", 0xB},
  {"r12", 0xC},
  {"r13", 0xD},
  {"r14", 0xE},
  {"r15", 0xF},
  {"r16", 0x10},
  {"r17", 0x11},
  {"r18", 0x12},
  {"r19", 0x13},
  {"r2", 0x2},
  {"r20", 0x14},
  {"r21", 0x15},
  {"r22", 0x16},
  {"r23", 0x17},
  {"r24", 0x18},
  {"r25", 0x19},
  {"r26", 0x1a},
  {"r27", 0x1b},
  {"r28", 0x1c},
  {"r29", 0x1d},
  {"r3", 0x3},
  {"r30", 0x1e},
  {"r31", 0x1f},
  {"r4", 0x4},
  {"r5", 0x5},
  {"r6", 0x6},
  {"r7", 0x7},
  {"r8", 0x8},
  {"r9", 0x9},
  {"sp", 0x17},
};

static const struct reg_entry fp_reg_table[] ATTRIBUTE_UNUSED =
{
  {"f0", 0x0},
  {"f1", 0x1},
  {"f10", 0xa},
  {"f11", 0xb},
  {"f12", 0xc},
  {"f13", 0xd},
  {"f14", 0xe},
  {"f15", 0xf},
  {"f2", 0x2},
  {"f3", 0x3},
  {"f4", 0x4},
  {"f5", 0x5},
  {"f6", 0x6},
  {"f7", 0x7},
  {"f8", 0x8},
  {"f9", 0x9},
};

static const struct cc_entry
{
  const char *name;
  int code;
} cc_table [] ATTRIBUTE_UNUSED =
{
  {"cc", 6},
  {"cs", 2},
  {"eq", 1},
  {"fa", 0},
  {"ge", 9},
  {"gt", 10},
  {"hi", 11},
  {"le", 12},
  {"ls", 13},
  {"lt", 14},
  {"nc", 8},
  {"ne", 5},
  {"ns", 4},
  {"oc", 7},
  {"os", 3},
  {"tr", 15},
};

enum addressing_mode
{
  mode_d,	/* register := */
  mode_a,	/* op= register */
  mode_da,	/* register := register */
  mode_ab,	/* register * register */
  mode_dab,	/* register := register * register */
  mode_iab,	/* 5-bit immediate * register * register */
  mode_0ab,	/* zero * register * register */
  mode_da0,	/* register := register * zero */
  mode_cad,	/* condition * register * register */
  mode_das,	/* register := register * 5-bit immed/register shift count */
  mode_di,	/* register := 5-bit immediate */
  mode_ir,	/* 5-bit immediate * register */
  mode_ai,	/* register 16-bit unsigned immediate */
  mode_i,	/* 16-bit unsigned immediate */
  mode_bax,	/* register * register * 5-bit immediate */
  mode_dax,	/* register := register * 5-bit immediate */
  mode_s,	/* special mode */
  mode_sr,	/* special mode with register */
  mode_ci,	/* condition * 16-bit signed word displacement */
  mode_fdab,	/* float := float * float */
  mode_ifdab,	/* fpinst: 4-bit immediate * float * float * float */
  mode_idfab,	/* fpuread: 4-bit immediate * register * float * float */
  mode_fda,	/* float := float */
  mode_fdra,	/* float := register */
  mode_rdfab,	/* register := float * float */
  mode_rdfa,	/* register := float */
  mode_rrr,	/* 3 register sources and destinations (block move) */
};

#define class0 (0<<25)
#define class1 (1<<25)
#define class2 (2<<25)
#define class3 (3<<25)

static const struct opcode_entry
{
  const char *mnem;
  enum addressing_mode mode;
  unsigned code;
  char flags;
}
opcode_table[] ATTRIBUTE_UNUSED =
{
  { "adc.b",    mode_dab,  class3|(1<<21)|(1), def },
  { "adc.l",    mode_dab,  class3|(1<<21)|(4), def },
  { "adc.w",    mode_dab,  class3|(1<<21)|(2), def },
  { "add.b",    mode_dab,  class3|(0<<21)|(1), def },
  { "add.l",    mode_dab,  class3|(0<<21)|(4), def },
  { "add.w",    mode_dab,  class3|(0<<21)|(2), def },
  { "addi",     mode_ai,   class2, def },
  { "and.b",    mode_dab,  class3|(10<<21)|(1), def},
  { "and.l",    mode_dab,  class3|(10<<21)|(4), def },
  { "and.w",    mode_dab,  class3|(10<<21)|(2), def },
  { "asl.b",    mode_das,  class3|(7<<21)|(1), def },
  { "asl.l",    mode_das,  class3|(7<<21)|(4), def },
  { "asl.w",    mode_das,  class3|(7<<21)|(2), def },
  { "asld",     mode_a,    class1|(15<<21)|(1<<15)|(11<<10)|(4), def },
  { "asr.b",    mode_das,  class3|(5<<21)|(1), def },
  { "asr.l",    mode_das,  class3|(5<<21)|(4), def },
  { "asr.w",    mode_das,  class3|(5<<21)|(2), def },
  { "asrd",     mode_a,    class1|(15<<21)|(1<<15)|(9<<10)|(4), def },
  { "bmd",      mode_rrr,  class1|(3<<21)|(3<<16)|(4), gr6 },
  { "bmi",      mode_rrr,  class1|(1<<21)|(3<<16)|(4), gr6 },
  { "bra",      mode_cad,  class3|(12<<21)|(4), def },
  { "brr",      mode_ci,   class0, def },
  { "cmp.b",    mode_0ab,  class3|(2<<21)|(1), def },
  { "cmp.l",    mode_0ab,  class3|(2<<21)|(4), def },
  { "cmp.w",    mode_0ab,  class3|(2<<21)|(2), def },
  { "cmpc.b",   mode_0ab,  class3|(3<<21)|(1), def },
  { "cmpc.l",   mode_0ab,  class3|(3<<21)|(4), def },
  { "cmpc.w",   mode_0ab,  class3|(3<<21)|(2), def },
  { "divds",    mode_a,    class1|(15<<21)|(1<<15)|(6<<10)|(4), def },
  { "divdu",    mode_a,    class1|(15<<21)|(1<<15)|(7<<10)|(4), def },
  { "divs",     mode_a,    class1|(15<<21)|(1<<15)|(2<<10)|(4), def },
  { "divu",     mode_a,    class1|(15<<21)|(1<<15)|(3<<10)|(4), def },
  { "dsi",      mode_s,    class1|(4<<21)|(4), def },
  { "eamread",  mode_di,   class3|(15<<21)|(1<<15)|(1<<9)|(4), def },
  { "eamwrite", mode_iab,  class1|(15<<21)|(1<<15)|(4), def },
  { "eni",      mode_s,    class1|(5<<21)|(4), def },
  { "extb.b",   mode_da,   class3|(14<<21)|(1), def },
  { "extb.l",   mode_da,   class3|(14<<21)|(4), def },
  { "extb.w",   mode_da,   class3|(14<<21)|(2), def },
  { "extw.l",   mode_da,   class3|(4<<21)|(4), def },
  { "extw.w",   mode_da,   class3|(4<<21)|(2), def },
  { "fabs",     mode_fda,  class1|(7<<27)|(15<<21)|(1<<15)|(1<<3)|(4), gr5 },
  { "fadd",     mode_fdab, class1|(1<<27)|(15<<21)|(1<<15)|(1<<3)|(4), gr5 },
  { "fcmp",     mode_rdfab,class3|(10<<27)|(15<<21)|(1<<15)|(1<<9)|(1<<3)|(4), gr5 },
  { "fcmpe",    mode_rdfab,class3|(11<<27)|(15<<21)|(1<<15)|(1<<9)|(1<<3)|(4), gr5 },
  { "fdiv",     mode_fdab, class1|(4<<27)|(15<<21)|(1<<15)|(1<<3)|(4), gr5 },
  { "fload",    mode_fdra, class1|(15<<21)|(1<<15)|(1<<3)|(4), gr5 },
  { "fmove",    mode_fda,  class1|(12<<27)|(15<<21)|(1<<15)|(1<<3)|(4), gr5},
  { "fmult",    mode_fdab, class1|(3<<27)|(15<<21)|(1<<15)|(1<<3)|(4), gr5 },
  { "fneg",     mode_fda,  class1|(6<<27)|(15<<21)|(1<<15)|(1<<3)|(4), gr5 },
  { "fpinst",   mode_ifdab,class1|(15<<21)|(1<<15)|(1<<3)|(4), gr5 },
  { "fpuread",  mode_idfab,class3|(15<<21)|(1<<15)|(1<<9)|(1<<3)|(4), gr5 },
  { "fsqrt",    mode_fda,  class1|(5<<27)|(15<<21)|(1<<15)|(1<<3)|(4), gr5 },
  { "fstore",   mode_rdfa, class3|(15<<21)|(1<<15)|(1<<9)|(1<<3)|(4), gr5 },
  { "fsub",     mode_fdab, class1|(2<<27)|(15<<21)|(1<<15)|(1<<3)|(4), gr5 },
  { "ftoi",     mode_fda,  class1|(8<<27)|(15<<21)|(1<<15)|(1<<3)|(4), gr5 },
  { "itof",     mode_fda,  class1|(9<<27)|(15<<21)|(1<<15)|(1<<3)|(4), gr5 },
  { "lsr.b",    mode_das,  class3|(6<<21)|(1), def },
  { "lsr.l",    mode_das,  class3|(6<<21)|(4), def },
  { "lsr.w",    mode_das,  class3|(6<<21)|(2), def },
  { "lsrd",     mode_a,    class1|(15<<21)|(1<<15)|(10<<10)|(4), def },
  { "move.b",   mode_da0,  class3|(9<<21)|(1), def },
  { "move.l",   mode_da0,  class3|(9<<21)|(4), def },
  { "move.w",   mode_da0,  class3|(9<<21)|(2), def },
  { "movil",    mode_ai,   class2|(4<<21), def },
  { "moviq",    mode_ai,   class2|(6<<21), def },
  { "moviu",    mode_ai,   class2|(5<<21), def },
  { "mults",    mode_ab,   class1|(15<<21)|(1<<15)|(0<<10)|(4), def },
  { "multu",    mode_ab,   class1|(15<<21)|(1<<15)|(1<<10)|(4), def },
  { "nop",      mode_s,    class0, def },
  { "not.b",    mode_da,   class3|(11<<21)|(1), def },
  { "not.l",    mode_da,   class3|(11<<21)|(4), def },
  { "not.w",    mode_da,   class3|(11<<21)|(2), def },
  { "or.b",     mode_dab,  class3|(9<<21)|(1), def },
  { "or.l",     mode_dab,  class3|(9<<21)|(4), def },
  { "or.w",     mode_dab,  class3|(9<<21)|(2), def },
  { "read.b",   mode_dax,  class3|(15<<21)|(1<<9)|(1), def },
  { "read.l",   mode_dax,  class3|(15<<21)|(1<<9)|(4), def },
  { "read.w",   mode_dax,  class3|(15<<21)|(1<<9)|(2), def },
  { "readmda",  mode_d,    class3|(15<<21)|(1<<15)|(1<<9)|(4), def },
  { "readmdb",  mode_d,    class3|(15<<21)|(1<<15)|(1<<9)|(1<<4)|(4), def },
  { "readmdc",  mode_d,    class3|(15<<21)|(1<<15)|(1<<9)|(2<<4)|(4), def },
  { "rfi",      mode_s,    class1|(7<<21)|(30<<16)|(29<<4)|(4), def },
  { "rflag",    mode_d,    class3|(13<<21)|(4), def },
  { "stop",     mode_ir,   class1|(0<<21)|(4), def },
  { "sub.b",    mode_dab,  class3|(2<<21)|(1), def },
  { "sub.l",    mode_dab,  class3|(2<<21)|(4), def },
  { "sub.w",    mode_dab,  class3|(2<<21)|(2), def },
  { "subc.b",   mode_dab,  class3|(3<<21)|(1), def },
  { "subc.l",   mode_dab,  class3|(3<<21)|(4), def },
  { "subc.w",   mode_dab,  class3|(3<<21)|(2), def },
  { "subi",     mode_ai,   class2|(2<<21), def },
  { "trace",    mode_ir,   class1|(13<<21), def },
  { "write.b",  mode_bax,  class1|(15<<21)|(1), def },
  { "write.l",  mode_bax,  class1|(15<<21)|(4), def },
  { "write.w",  mode_bax,  class1|(15<<21)|(2), def },
  { "writemd",  mode_ab,   class1|(15<<21)|(1<<15)|(4<<10)|(4), def },
  { "writemdc", mode_a,    class1|(15<<21)|(1<<15)|(5<<10)|(4), def },
  { "wrtl",     mode_i,    class2|(8<<21), gr6 },
  { "wrtu",     mode_i,    class2|(9<<21), gr6 },
  { "xor.b",    mode_dab,  class3|(8<<21)|(1), def },
  { "xor.l",    mode_dab,  class3|(8<<21)|(4), def },
  { "xor.w",    mode_dab,  class3|(8<<21)|(2), def },
};

