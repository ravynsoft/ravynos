/* moxie-opc.c -- Definitions for moxie opcodes.
   Copyright (C) 2009-2023 Free Software Foundation, Inc.
   Contributed by Anthony Green (green@moxielogic.com).

   This file is part of the GNU opcodes library.

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   It is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with this file; see the file COPYING.  If not, write to the
   Free Software Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"
#include "opcode/moxie.h"

/* The moxie processor's 16-bit instructions come in two forms:

  FORM 1 instructions start with a 0 bit...

    0oooooooaaaabbbb
    0              F

   ooooooo - form 1 opcode number
   aaaa    - operand A
   bbbb    - operand B

  FORM 2 instructions start with bits "10"...

    10ooaaaavvvvvvvv
    0              F

   oo       - form 2 opcode number
   aaaa     - operand A
   vvvvvvvv - 8-bit immediate value

  FORM 3 instructions start with a bits "11"...

    11oooovvvvvvvvvv
    0              F

   oooo         - form 3 opcode number
   vvvvvvvvvv   - 10-bit immediate value.  */

const moxie_opc_info_t moxie_form1_opc_info[128] =
  {
    { 0x00, MOXIE_BAD,     "bad" },  /* Reserved as bad.  */
    { 0x01, MOXIE_F1_A4,   "ldi.l" },
    { 0x02, MOXIE_F1_AB,   "mov" },
    { 0x03, MOXIE_F1_M,    "jsra" },
    { 0x04, MOXIE_F1_NARG, "ret" },
    { 0x05, MOXIE_F1_AB,   "add" },
    { 0x06, MOXIE_F1_AB,   "push" },
    { 0x07, MOXIE_F1_AB,   "pop" },
    { 0x08, MOXIE_F1_A4,   "lda.l" },
    { 0x09, MOXIE_F1_4A,   "sta.l" },
    { 0x0a, MOXIE_F1_ABi,  "ld.l" },
    { 0x0b, MOXIE_F1_AiB,  "st.l" },
    { 0x0c, MOXIE_F1_ABi2, "ldo.l" },
    { 0x0d, MOXIE_F1_AiB2, "sto.l" },
    { 0x0e, MOXIE_F1_AB,   "cmp" },
    { 0x0f, MOXIE_F1_NARG, "nop" },
    { 0x10, MOXIE_F1_AB,   "sex.b" },
    { 0x11, MOXIE_F1_AB,   "sex.s" },
    { 0x12, MOXIE_F1_AB,   "zex.b" },
    { 0x13, MOXIE_F1_AB,   "zex.s" },
    { 0x14, MOXIE_F1_AB,   "umul.x" },
    { 0x15, MOXIE_F1_AB,   "mul.x" },
    { 0x16, MOXIE_BAD,     "bad" },
    { 0x17, MOXIE_BAD,     "bad" },
    { 0x18, MOXIE_BAD,     "bad" },
    { 0x19, MOXIE_F1_A,    "jsr" },
    { 0x1a, MOXIE_F1_M,    "jmpa" },
    { 0x1b, MOXIE_F1_A4,   "ldi.b" },
    { 0x1c, MOXIE_F1_ABi,  "ld.b" },
    { 0x1d, MOXIE_F1_A4,   "lda.b" },
    { 0x1e, MOXIE_F1_AiB,  "st.b" },
    { 0x1f, MOXIE_F1_4A,   "sta.b" },
    { 0x20, MOXIE_F1_A4,   "ldi.s" },
    { 0x21, MOXIE_F1_ABi,  "ld.s" },
    { 0x22, MOXIE_F1_A4,   "lda.s" },
    { 0x23, MOXIE_F1_AiB,  "st.s" },
    { 0x24, MOXIE_F1_4A,   "sta.s" },
    { 0x25, MOXIE_F1_A,    "jmp" },
    { 0x26, MOXIE_F1_AB,   "and" },
    { 0x27, MOXIE_F1_AB,   "lshr" },
    { 0x28, MOXIE_F1_AB,   "ashl" },
    { 0x29, MOXIE_F1_AB,   "sub" },
    { 0x2a, MOXIE_F1_AB,   "neg" },
    { 0x2b, MOXIE_F1_AB,   "or" },
    { 0x2c, MOXIE_F1_AB,   "not" },
    { 0x2d, MOXIE_F1_AB,   "ashr" },
    { 0x2e, MOXIE_F1_AB,   "xor" },
    { 0x2f, MOXIE_F1_AB,   "mul" },
    { 0x30, MOXIE_F1_4,    "swi" },
    { 0x31, MOXIE_F1_AB,   "div" },
    { 0x32, MOXIE_F1_AB,   "udiv" },
    { 0x33, MOXIE_F1_AB,   "mod" },
    { 0x34, MOXIE_F1_AB,   "umod" },
    { 0x35, MOXIE_F1_NARG, "brk" },
    { 0x36, MOXIE_F1_ABi2, "ldo.b" },
    { 0x37, MOXIE_F1_AiB2, "sto.b" },
    { 0x38, MOXIE_F1_ABi2, "ldo.s" },
    { 0x39, MOXIE_F1_AiB2, "sto.s" },
    { 0x3a, MOXIE_BAD,     "bad" },
    { 0x3b, MOXIE_BAD,     "bad" },
    { 0x3c, MOXIE_BAD,     "bad" },
    { 0x3d, MOXIE_BAD,     "bad" },
    { 0x3e, MOXIE_BAD,     "bad" },
    { 0x3f, MOXIE_BAD,     "bad" },
    { 0x40, MOXIE_BAD,     "bad" },
    { 0x41, MOXIE_BAD,     "bad" },
    { 0x42, MOXIE_BAD,     "bad" },
    { 0x43, MOXIE_BAD,     "bad" },
    { 0x44, MOXIE_BAD,     "bad" },
    { 0x45, MOXIE_BAD,     "bad" },
    { 0x46, MOXIE_BAD,     "bad" },
    { 0x47, MOXIE_BAD,     "bad" },
    { 0x48, MOXIE_BAD,     "bad" },
    { 0x49, MOXIE_BAD,     "bad" },
    { 0x4a, MOXIE_BAD,     "bad" },
    { 0x4b, MOXIE_BAD,     "bad" },
    { 0x4c, MOXIE_BAD,     "bad" },
    { 0x4d, MOXIE_BAD,     "bad" },
    { 0x4e, MOXIE_BAD,     "bad" },
    { 0x4f, MOXIE_BAD,     "bad" },
    { 0x50, MOXIE_BAD,     "bad" },
    { 0x51, MOXIE_BAD,     "bad" },
    { 0x52, MOXIE_BAD,     "bad" },
    { 0x53, MOXIE_BAD,     "bad" },
    { 0x54, MOXIE_BAD,     "bad" },
    { 0x55, MOXIE_BAD,     "bad" },
    { 0x56, MOXIE_BAD,     "bad" },
    { 0x57, MOXIE_BAD,     "bad" },
    { 0x58, MOXIE_BAD,     "bad" },
    { 0x59, MOXIE_BAD,     "bad" },
    { 0x5a, MOXIE_BAD,     "bad" },
    { 0x5b, MOXIE_BAD,     "bad" },
    { 0x5c, MOXIE_BAD,     "bad" },
    { 0x5d, MOXIE_BAD,     "bad" },
    { 0x5e, MOXIE_BAD,     "bad" },
    { 0x5f, MOXIE_BAD,     "bad" },
    { 0x60, MOXIE_BAD,     "bad" },
    { 0x61, MOXIE_BAD,     "bad" },
    { 0x62, MOXIE_BAD,     "bad" },
    { 0x63, MOXIE_BAD,     "bad" },
    { 0x64, MOXIE_BAD,     "bad" },
    { 0x65, MOXIE_BAD,     "bad" },
    { 0x66, MOXIE_BAD,     "bad" },
    { 0x67, MOXIE_BAD,     "bad" },
    { 0x68, MOXIE_BAD,     "bad" },
    { 0x69, MOXIE_BAD,     "bad" },
    { 0x6a, MOXIE_BAD,     "bad" },
    { 0x6b, MOXIE_BAD,     "bad" },
    { 0x6c, MOXIE_BAD,     "bad" },
    { 0x6d, MOXIE_BAD,     "bad" },
    { 0x6e, MOXIE_BAD,     "bad" },
    { 0x6f, MOXIE_BAD,     "bad" },
    { 0x70, MOXIE_BAD,     "bad" },
    { 0x71, MOXIE_BAD,     "bad" },
    { 0x72, MOXIE_BAD,     "bad" },
    { 0x73, MOXIE_BAD,     "bad" },
    { 0x74, MOXIE_BAD,     "bad" },
    { 0x75, MOXIE_BAD,     "bad" },
    { 0x76, MOXIE_BAD,     "bad" },
    { 0x77, MOXIE_BAD,     "bad" },
    { 0x78, MOXIE_BAD,     "bad" },
    { 0x79, MOXIE_BAD,     "bad" },
    { 0x7a, MOXIE_BAD,     "bad" },
    { 0x7b, MOXIE_BAD,     "bad" },
    { 0x7c, MOXIE_BAD,     "bad" },
    { 0x7d, MOXIE_BAD,     "bad" },
    { 0x7e, MOXIE_BAD,     "bad" },
    { 0x7f, MOXIE_BAD,     "bad" }
  };

const moxie_opc_info_t moxie_form2_opc_info[4] =
  {
    { 0x00, MOXIE_F2_A8V,  "inc" },
    { 0x01, MOXIE_F2_A8V,  "dec" },
    { 0x02, MOXIE_F2_A8V,  "gsr" },
    { 0x03, MOXIE_F2_A8V,  "ssr" }
  };

const moxie_opc_info_t moxie_form3_opc_info[16] =
  {
    { 0x00, MOXIE_F3_PCREL,"beq" },
    { 0x01, MOXIE_F3_PCREL,"bne" },
    { 0x02, MOXIE_F3_PCREL,"blt" },
    { 0x03, MOXIE_F3_PCREL,"bgt" },
    { 0x04, MOXIE_F3_PCREL,"bltu" },
    { 0x05, MOXIE_F3_PCREL,"bgtu" },
    { 0x06, MOXIE_F3_PCREL,"bge" },
    { 0x07, MOXIE_F3_PCREL,"ble" },
    { 0x08, MOXIE_F3_PCREL,"bgeu" },
    { 0x09, MOXIE_F3_PCREL,"bleu" },
    { 0x0a, MOXIE_BAD,     "bad" },
    { 0x0b, MOXIE_BAD,     "bad" },
    { 0x0c, MOXIE_BAD,     "bad" },
    { 0x0d, MOXIE_BAD,     "bad" },
    { 0x0e, MOXIE_BAD,     "bad" },
    { 0x0f, MOXIE_BAD,     "bad" }  /* Reserved as bad.  */
  };
