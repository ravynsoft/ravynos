/* m68hc11-opc.c -- Motorola 68HC11, 68HC12, 9S12X and XGATE opcode list
   Copyright (C) 1999-2023 Free Software Foundation, Inc.
   Written by Stephane Carrez (stcarrez@nerim.fr)
   XGATE and S12X added by James Murray (jsm@jsm-net.demon.co.uk)
   Note: min/max cycles not updated for S12X opcodes.

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

#include <stdio.h>
#include "ansidecl.h"
#include "opcode/m68hc11.h"

#define TABLE_SIZE(X)       (sizeof(X) / sizeof(X[0]))

/* Combination of CCR flags.  */
#define M6811_ZC_BIT    M6811_Z_BIT|M6811_C_BIT
#define M6811_NZ_BIT    M6811_N_BIT|M6811_Z_BIT
#define M6811_NZV_BIT   M6811_N_BIT|M6811_Z_BIT|M6811_V_BIT
#define M6811_NZC_BIT   M6811_N_BIT|M6811_Z_BIT|M6811_C_BIT
#define M6811_NVC_BIT   M6811_N_BIT|M6811_V_BIT|M6811_C_BIT
#define M6811_ZVC_BIT   M6811_Z_BIT|M6811_V_BIT|M6811_C_BIT
#define M6811_NZVC_BIT  M6811_ZVC_BIT|M6811_N_BIT
#define M6811_HNZVC_BIT M6811_NZVC_BIT|M6811_H_BIT
#define M6811_HNVC_BIT  M6811_NVC_BIT|M6811_H_BIT
#define M6811_VC_BIT    M6811_V_BIT|M6811_C_BIT

/* Flags when the insn only changes some CCR flags.  */
#define CHG_NONE        0,0,0
#define CHG_Z           0,0,M6811_Z_BIT
#define CHG_C           0,0,M6811_C_BIT
#define CHG_ZVC         0,0,M6811_ZVC_BIT
#define CHG_NZC         0,0,M6811_NZC_BIT
#define CHG_NZV         0,0,M6811_NZV_BIT
#define CHG_NZVC        0,0,M6811_NZVC_BIT
#define CHG_HNZVC       0,0,M6811_HNZVC_BIT
#define CHG_ALL         0,0,0xff

/* The insn clears and changes some flags.  */
#define CLR_I           0,M6811_I_BIT,0
#define CLR_C           0,M6811_C_BIT,0
#define CLR_V           0,M6811_V_BIT,0
#define CLR_V_CHG_ZC    0,M6811_V_BIT,M6811_ZC_BIT
#define CLR_V_CHG_NZ    0,M6811_V_BIT,M6811_NZ_BIT
#define CLR_V_CHG_ZVC   0,M6811_V_BIT,M6811_ZVC_BIT
#define CLR_N_CHG_ZVC   0,M6811_N_BIT,M6811_ZVC_BIT /* Used by lsr */
#define CLR_VC_CHG_NZ   0,M6811_VC_BIT,M6811_NZ_BIT

/* The insn sets some flags.  */
#define SET_I           M6811_I_BIT,0,0
#define SET_C           M6811_C_BIT,0,0
#define SET_V           M6811_V_BIT,0,0
#define SET_Z_CLR_NVC   M6811_Z_BIT,M6811_NVC_BIT,0
#define SET_C_CLR_V_CHG_NZ M6811_C_BIT,M6811_V_BIT,M6811_NZ_BIT
#define SET_Z_CHG_HNVC  M6811_Z_BIT,0,M6811_HNVC_BIT

#define _M 0xff
#define OP_NONE         M6811_OP_NONE
#define OP_PAGE2        M6811_OP_PAGE2
#define OP_PAGE3        M6811_OP_PAGE3
#define OP_PAGE4        M6811_OP_PAGE4
#define OP_IMM8         M6811_OP_IMM8
#define OP_IMM16        M6811_OP_IMM16
#define OP_IX           M6811_OP_IX
#define OP_IY           M6811_OP_IY
#define OP_IND16        M6811_OP_IND16
#define OP_PAGE         M6812_OP_PAGE
#define OP_IDX          M6812_OP_IDX
#define OP_IDX_1        M6812_OP_IDX_1
#define OP_IDX_2        M6812_OP_IDX_2
#define OP_D_IDX        M6812_OP_D_IDX
#define OP_D_IDX_2      M6812_OP_D_IDX_2
#define OP_DIRECT       M6811_OP_DIRECT
#define OP_BITMASK      M6811_OP_BITMASK
#define OP_BRANCH       M6811_OP_BRANCH
#define OP_JUMP_REL     (M6811_OP_JUMP_REL|OP_BRANCH)
#define OP_JUMP_REL16   (M6812_OP_JUMP_REL16|OP_BRANCH)
#define OP_REG          M6812_OP_REG
#define OP_REG_1        M6812_OP_REG
#define OP_REG_2        M6812_OP_REG_2
#define OP_IDX_p2       M6812_OP_IDX_P2
#define OP_IND16_p2     M6812_OP_IND16_P2
#define OP_TRAP_ID      M6812_OP_TRAP_ID
#define OP_EXG_MARKER   M6812_OP_EXG_MARKER
#define OP_TFR_MARKER   M6812_OP_TFR_MARKER
#define OP_DBEQ_MARKER  (M6812_OP_DBCC_MARKER|M6812_OP_EQ_MARKER)
#define OP_DBNE_MARKER  (M6812_OP_DBCC_MARKER)
#define OP_TBEQ_MARKER  (M6812_OP_TBCC_MARKER|M6812_OP_EQ_MARKER)
#define OP_TBNE_MARKER  (M6812_OP_TBCC_MARKER)
#define OP_IBEQ_MARKER  (M6812_OP_IBCC_MARKER|M6812_OP_EQ_MARKER)
#define OP_IBNE_MARKER  (M6812_OP_IBCC_MARKER)

/*
   { "test", OP_NONE,          1, 0x00,  5, _M,  CHG_NONE,  cpu6811, 0 },
                                                            +-- cpu  +-- XGATE opcode mask
  Name -+                                        +------- Insn CCR changes
  Format  ------+                            +----------- Max # cycles
  Size     --------------------+         +--------------- Min # cycles
                                   +--------------------- Opcode
*/
const struct m68hc11_opcode m68hc11_opcodes[] = {
  { "aba",  OP_NONE,           1, 0x1b,  2,  2,  CHG_HNZVC, cpu6811, 0 },
  { "aba",  OP_NONE | OP_PAGE2,2, 0x06,  2,  2,  CHG_HNZVC, cpu6812|cpu9s12x, 0 },
  { "abx",  OP_NONE,           1, 0x3a,  3,  3,  CHG_NONE,  cpu6811, 0 },
  { "aby",  OP_NONE | OP_PAGE2,2, 0x3a,  4,  4,  CHG_NONE,  cpu6811, 0 },

  { "adca", OP_IMM8,           2, 0x89,  1,  1,  CHG_HNZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "adca", OP_DIRECT,         2, 0x99,  3,  3,  CHG_HNZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "adca", OP_IND16,          3, 0xb9,  3,  3,  CHG_HNZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "adca", OP_IX,             2, 0xa9,  4,  4,  CHG_HNZVC, cpu6811, 0 },
  { "adca", OP_IY | OP_PAGE2,  3, 0xa9,  5,  5,  CHG_HNZVC, cpu6811, 0 },
  { "adca", OP_IDX,            2, 0xa9,  3,  3,  CHG_HNZVC, cpu6812|cpu9s12x, 0 },
  { "adca", OP_IDX_1,          3, 0xa9,  3,  3,  CHG_HNZVC, cpu6812|cpu9s12x, 0 },
  { "adca", OP_IDX_2,          4, 0xa9,  4,  4,  CHG_HNZVC, cpu6812|cpu9s12x, 0 },
  { "adca", OP_D_IDX,          2, 0xa9,  6,  6,  CHG_HNZVC, cpu6812|cpu9s12x, 0 },
  { "adca", OP_D_IDX_2,        4, 0xa9,  6,  6,  CHG_HNZVC, cpu6812|cpu9s12x, 0 },

  { "adcb", OP_IMM8,           2, 0xc9,  1,  1,  CHG_HNZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "adcb", OP_DIRECT,         2, 0xd9,  3,  3,  CHG_HNZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "adcb", OP_IND16,          3, 0xf9,  3,  3,  CHG_HNZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "adcb", OP_IX,             2, 0xe9,  4,  4,  CHG_HNZVC, cpu6811, 0 },
  { "adcb", OP_IY | OP_PAGE2,  3, 0xe9,  5,  5,  CHG_HNZVC, cpu6811, 0 },
  { "adcb", OP_IDX,            2, 0xe9,  3,  3,  CHG_HNZVC, cpu6812|cpu9s12x, 0 },
  { "adcb", OP_IDX_1,          3, 0xe9,  3,  3,  CHG_HNZVC, cpu6812|cpu9s12x, 0 },
  { "adcb", OP_IDX_2,          4, 0xe9,  4,  4,  CHG_HNZVC, cpu6812|cpu9s12x, 0 },
  { "adcb", OP_D_IDX,          2, 0xe9,  6,  6,  CHG_HNZVC, cpu6812|cpu9s12x, 0 },
  { "adcb", OP_D_IDX_2,        4, 0xe9,  6,  6,  CHG_HNZVC, cpu6812|cpu9s12x, 0 },

  { "adda", OP_IMM8,           2, 0x8b,  1,  1,  CHG_HNZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "adda", OP_DIRECT,         2, 0x9b,  3,  3,  CHG_HNZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "adda", OP_IND16,          3, 0xbb,  3,  3,  CHG_HNZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "adda", OP_IX,             2, 0xab,  4,  4,  CHG_HNZVC, cpu6811, 0 },
  { "adda", OP_IY | OP_PAGE2,  3, 0xab,  5,  5,  CHG_HNZVC, cpu6811, 0 },
  { "adda", OP_IDX,            2, 0xab,  3,  3,  CHG_HNZVC, cpu6812|cpu9s12x, 0 },
  { "adda", OP_IDX_1,          3, 0xab,  3,  3,  CHG_HNZVC, cpu6812|cpu9s12x, 0 },
  { "adda", OP_IDX_2,          4, 0xab,  4,  4,  CHG_HNZVC, cpu6812|cpu9s12x, 0 },
  { "adda", OP_D_IDX,          2, 0xab,  6,  6,  CHG_HNZVC, cpu6812|cpu9s12x, 0 },
  { "adda", OP_D_IDX_2,        4, 0xab,  6,  6,  CHG_HNZVC, cpu6812|cpu9s12x, 0 },

  { "addb", OP_IMM8,           2, 0xcb,  1,  1,  CHG_HNZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "addb", OP_DIRECT,         2, 0xdb,  3,  3,  CHG_HNZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "addb", OP_IND16,          3, 0xfb,  3,  3,  CHG_HNZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "addb", OP_IX,             2, 0xeb,  4,  4,  CHG_HNZVC, cpu6811, 0 },
  { "addb", OP_IY | OP_PAGE2,  3, 0xeb,  5,  5,  CHG_HNZVC, cpu6811, 0 },
  { "addb", OP_IDX,            2, 0xeb,  3,  3,  CHG_HNZVC, cpu6812|cpu9s12x, 0 },
  { "addb", OP_IDX_1,          3, 0xeb,  3,  3,  CHG_HNZVC, cpu6812|cpu9s12x, 0 },
  { "addb", OP_IDX_2,          4, 0xeb,  4,  4,  CHG_HNZVC, cpu6812|cpu9s12x, 0 },
  { "addb", OP_D_IDX,          2, 0xeb,  6,  6,  CHG_HNZVC, cpu6812|cpu9s12x, 0 },
  { "addb", OP_D_IDX_2,        4, 0xeb,  6,  6,  CHG_HNZVC, cpu6812|cpu9s12x, 0 },

  { "addd", OP_IMM16,          3, 0xc3,  2,  2,  CHG_NZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "addd", OP_DIRECT,         2, 0xd3,  3,  3,  CHG_NZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "addd", OP_IND16,          3, 0xf3,  3,  3,  CHG_NZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "addd", OP_IX,             2, 0xe3,  6,  6,  CHG_NZVC, cpu6811, 0 },
  { "addd", OP_IY | OP_PAGE2,  3, 0xe3,  7,  7,  CHG_NZVC, cpu6811, 0 },
  { "addd", OP_IDX,            2, 0xe3,  3,  3,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "addd", OP_IDX_1,          3, 0xe3,  3,  3,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "addd", OP_IDX_2,          4, 0xe3,  4,  4,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "addd", OP_D_IDX,          2, 0xe3,  6,  6,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "addd", OP_D_IDX_2,        4, 0xe3,  6,  6,  CHG_NZVC, cpu6812|cpu9s12x, 0 },

  { "addx", OP_IMM16 | OP_PAGE2,          3, 0x8b,  2,  2,  CHG_NZVC, cpu9s12x, 0 },
  { "addx", OP_DIRECT | OP_PAGE2,         2, 0x9b,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "addx", OP_IND16 | OP_PAGE2,          3, 0xbb,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "addx", OP_IDX | OP_PAGE2,            2, 0xab,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "addx", OP_IDX_1 | OP_PAGE2,          3, 0xab,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "addx", OP_IDX_2 | OP_PAGE2,          4, 0xab,  4,  4,  CHG_NZVC, cpu9s12x, 0 },
  { "addx", OP_D_IDX | OP_PAGE2,          2, 0xab,  6,  6,  CHG_NZVC, cpu9s12x, 0 },
  { "addx", OP_D_IDX_2 | OP_PAGE2,        4, 0xab,  6,  6,  CHG_NZVC, cpu9s12x, 0 },

  { "addy", OP_IMM16 | OP_PAGE2,          3, 0xcb,  2,  2,  CHG_NZVC, cpu9s12x, 0 },
  { "addy", OP_DIRECT | OP_PAGE2,         2, 0xdb,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "addy", OP_IND16 | OP_PAGE2,          3, 0xfb,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "addy", OP_IDX | OP_PAGE2,            2, 0xeb,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "addy", OP_IDX_1 | OP_PAGE2,          3, 0xeb,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "addy", OP_IDX_2 | OP_PAGE2,          4, 0xeb,  4,  4,  CHG_NZVC, cpu9s12x, 0 },
  { "addy", OP_D_IDX | OP_PAGE2,          2, 0xeb,  6,  6,  CHG_NZVC, cpu9s12x, 0 },
  { "addy", OP_D_IDX_2 | OP_PAGE2,        4, 0xeb,  6,  6,  CHG_NZVC, cpu9s12x, 0 },

  { "aded", OP_IMM16 | OP_PAGE2,          3, 0xc3,  2,  2,  CHG_NZVC, cpu9s12x, 0 },
  { "aded", OP_DIRECT | OP_PAGE2,         2, 0xd3,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "aded", OP_IND16 | OP_PAGE2,          3, 0xf3,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "aded", OP_IDX | OP_PAGE2,            2, 0xe3,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "aded", OP_IDX_1 | OP_PAGE2,          3, 0xe3,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "aded", OP_IDX_2 | OP_PAGE2,          4, 0xe3,  4,  4,  CHG_NZVC, cpu9s12x, 0 },
  { "aded", OP_D_IDX | OP_PAGE2,          2, 0xe3,  6,  6,  CHG_NZVC, cpu9s12x, 0 },
  { "aded", OP_D_IDX_2 | OP_PAGE2,        4, 0xe3,  6,  6,  CHG_NZVC, cpu9s12x, 0 },

  { "adex", OP_IMM16 | OP_PAGE2,          3, 0x89,  2,  2,  CHG_NZVC, cpu9s12x, 0 },
  { "adex", OP_DIRECT | OP_PAGE2,         2, 0x99,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "adex", OP_IND16 | OP_PAGE2,          3, 0xb9,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "adex", OP_IDX | OP_PAGE2,            2, 0xa9,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "adex", OP_IDX_1 | OP_PAGE2,          3, 0xa9,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "adex", OP_IDX_2 | OP_PAGE2,          4, 0xa9,  4,  4,  CHG_NZVC, cpu9s12x, 0 },
  { "adex", OP_D_IDX | OP_PAGE2,          2, 0xa9,  6,  6,  CHG_NZVC, cpu9s12x, 0 },
  { "adex", OP_D_IDX_2 | OP_PAGE2,        4, 0xa9,  6,  6,  CHG_NZVC, cpu9s12x, 0 },

  { "adey", OP_IMM16 | OP_PAGE2,          3, 0xc9,  2,  2,  CHG_NZVC, cpu9s12x, 0 },
  { "adey", OP_DIRECT | OP_PAGE2,         2, 0xd9,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "adey", OP_IND16 | OP_PAGE2,          3, 0xf9,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "adey", OP_IDX | OP_PAGE2,            2, 0xe9,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "adey", OP_IDX_1 | OP_PAGE2,          3, 0xe9,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "adey", OP_IDX_2 | OP_PAGE2,          4, 0xe9,  4,  4,  CHG_NZVC, cpu9s12x, 0 },
  { "adey", OP_D_IDX | OP_PAGE2,          2, 0xe9,  6,  6,  CHG_NZVC, cpu9s12x, 0 },
  { "adey", OP_D_IDX_2 | OP_PAGE2,        4, 0xe9,  6,  6,  CHG_NZVC, cpu9s12x, 0 },

  { "anda", OP_IMM8,         2, 0x84,  1,  1,  CLR_V_CHG_NZ, cpu6811|cpu6812|cpu9s12x, 0 },
  { "anda", OP_DIRECT,       2, 0x94,  3,  3,  CLR_V_CHG_NZ, cpu6811|cpu6812|cpu9s12x, 0 },
  { "anda", OP_IND16,        3, 0xb4,  3,  3,  CLR_V_CHG_NZ, cpu6811|cpu6812|cpu9s12x, 0 },
  { "anda", OP_IX,             2, 0xa4,  4,  4,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "anda", OP_IY | OP_PAGE2,  3, 0xa4,  5,  5,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "anda", OP_IDX,            2, 0xa4,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "anda", OP_IDX_1,          3, 0xa4,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "anda", OP_IDX_2,          4, 0xa4,  4,  4,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "anda", OP_D_IDX,          2, 0xa4,  6,  6,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "anda", OP_D_IDX_2,        4, 0xa4,  6,  6,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },

  { "andb", OP_IMM8,         2, 0xc4,  1,  1,  CLR_V_CHG_NZ, cpu6811|cpu6812|cpu9s12x, 0 },
  { "andb", OP_DIRECT,       2, 0xd4,  3,  3,  CLR_V_CHG_NZ, cpu6811|cpu6812|cpu9s12x, 0 },
  { "andb", OP_IND16,        3, 0xf4,  3,  3,  CLR_V_CHG_NZ, cpu6811|cpu6812|cpu9s12x, 0 },
  { "andb", OP_IX,             2, 0xe4,  4,  4,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "andb", OP_IY | OP_PAGE2,  3, 0xe4,  5,  5,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "andb", OP_IDX,            2, 0xe4,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "andb", OP_IDX_1,          3, 0xe4,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "andb", OP_IDX_2,          4, 0xe4,  4,  4,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "andb", OP_D_IDX,          2, 0xe4,  6,  6,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "andb", OP_D_IDX_2,        4, 0xe4,  6,  6,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },

  { "andcc", OP_IMM8,          2, 0x10,  1,  1,  CHG_ALL,  cpu6812|cpu9s12x, 0 },

  { "andx", OP_IMM16 | OP_PAGE2,         2, 0x84,  1,  1,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "andx", OP_DIRECT | OP_PAGE2,       2, 0x94,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "andx", OP_IND16 | OP_PAGE2,        3, 0xb4,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "andx", OP_IDX | OP_PAGE2,            2, 0xa4,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "andx", OP_IDX_1 | OP_PAGE2,          3, 0xa4,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "andx", OP_IDX_2 | OP_PAGE2,          4, 0xa4,  4,  4,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "andx", OP_D_IDX | OP_PAGE2,          2, 0xa4,  6,  6,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "andx", OP_D_IDX_2 | OP_PAGE2,        4, 0xa4,  6,  6,  CLR_V_CHG_NZ, cpu9s12x, 0 },

  { "andy", OP_IMM16 | OP_PAGE2,         2, 0xc4,  1,  1,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "andy", OP_DIRECT | OP_PAGE2,       2, 0xd4,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "andy", OP_IND16 | OP_PAGE2,        3, 0xf4,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "andy", OP_IDX | OP_PAGE2,            2, 0xe4,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "andy", OP_IDX_1 | OP_PAGE2,          3, 0xe4,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "andy", OP_IDX_2 | OP_PAGE2,          4, 0xe4,  4,  4,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "andy", OP_D_IDX | OP_PAGE2,          2, 0xe4,  6,  6,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "andy", OP_D_IDX_2 | OP_PAGE2,        4, 0xe4,  6,  6,  CLR_V_CHG_NZ, cpu9s12x, 0 },

  { "asl",  OP_IND16,          3, 0x78,  4,  4,  CHG_NZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "asl",  OP_IX,             2, 0x68,  6,  6,  CHG_NZVC, cpu6811, 0 },
  { "asl",  OP_IY | OP_PAGE2,  3, 0x68,  7,  7,  CHG_NZVC, cpu6811, 0 },
  { "asl",  OP_IDX,            2, 0x68,  3,  3,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "asl",  OP_IDX_1,          3, 0x68,  4,  4,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "asl",  OP_IDX_2,          4, 0x68,  5,  5,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "asl",  OP_D_IDX,          2, 0x68,  6,  6,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "asl",  OP_D_IDX_2,        4, 0x68,  6,  6,  CHG_NZVC, cpu6812|cpu9s12x, 0 },

  { "asla", OP_NONE,           1, 0x48,  1,  1,  CHG_NZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "aslb", OP_NONE,           1, 0x58,  1,  1,  CHG_NZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "asld", OP_NONE,           1, 0x05,  3,  3,  CHG_NZVC, cpu6811, 0 },
  { "asld", OP_NONE,           1, 0x59,  1,  1,  CHG_NZVC, cpu6812|cpu9s12x, 0 },

  { "aslw",  OP_IND16 | OP_PAGE2,          3, 0x78,  4,  4,  CHG_NZVC, cpu9s12x, 0 },
  { "aslw",  OP_IDX | OP_PAGE2,            2, 0x68,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "aslw",  OP_IDX_1 | OP_PAGE2,          3, 0x68,  4,  4,  CHG_NZVC, cpu9s12x, 0 },
  { "aslw",  OP_IDX_2 | OP_PAGE2,          4, 0x68,  5,  5,  CHG_NZVC, cpu9s12x, 0 },
  { "aslw",  OP_D_IDX | OP_PAGE2,          2, 0x68,  6,  6,  CHG_NZVC, cpu9s12x, 0 },
  { "aslw",  OP_D_IDX_2 | OP_PAGE2,        4, 0x68,  6,  6,  CHG_NZVC, cpu9s12x, 0 },

  { "aslx", OP_NONE | OP_PAGE2,           1, 0x48,  1,  1,  CHG_NZVC, cpu9s12x, 0 },

  { "asly", OP_NONE | OP_PAGE2,           1, 0x58,  1,  1,  CHG_NZVC, cpu9s12x, 0 },

  { "asr",  OP_IND16,          3, 0x77,  4,  4,  CHG_NZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "asr",  OP_IX,             2, 0x67,  6,  6,  CHG_NZVC, cpu6811, 0 },
  { "asr",  OP_IY | OP_PAGE2,  3, 0x67,  7,  7,  CHG_NZVC, cpu6811, 0 },
  { "asr",  OP_IDX,            2, 0x67,  3,  3,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "asr",  OP_IDX_1,          3, 0x67,  4,  4,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "asr",  OP_IDX_2,          4, 0x67,  5,  5,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "asr",  OP_D_IDX,          2, 0x67,  6,  6,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "asr",  OP_D_IDX_2,        4, 0x67,  6,  6,  CHG_NZVC, cpu6812|cpu9s12x, 0 },

  { "asra", OP_NONE,           1, 0x47,  1,  1,  CHG_NZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "asrb", OP_NONE,           1, 0x57,  1,  1,  CHG_NZVC, cpu6811|cpu6812|cpu9s12x, 0 },

  { "asrw",  OP_IND16 | OP_PAGE2,          3, 0x77,  4,  4,  CHG_NZVC, cpu9s12x, 0 },
  { "asrw",  OP_IDX | OP_PAGE2,            2, 0x67,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "asrw",  OP_IDX_1 | OP_PAGE2,          3, 0x67,  4,  4,  CHG_NZVC, cpu9s12x, 0 },
  { "asrw",  OP_IDX_2 | OP_PAGE2,          4, 0x67,  5,  5,  CHG_NZVC, cpu9s12x, 0 },
  { "asrw",  OP_D_IDX | OP_PAGE2,          2, 0x67,  6,  6,  CHG_NZVC, cpu9s12x, 0 },
  { "asrw",  OP_D_IDX_2 | OP_PAGE2,        4, 0x67,  6,  6,  CHG_NZVC, cpu9s12x, 0 },

  { "asrx", OP_NONE | OP_PAGE2,           1, 0x47,  1,  1,  CHG_NZVC, cpu9s12x, 0 },

  { "asry", OP_NONE | OP_PAGE2,           1, 0x57,  1,  1,  CHG_NZVC, cpu9s12x, 0 },

  { "bcc", OP_JUMP_REL,        2, 0x24,  1,  3,  CHG_NONE, cpu6811|cpu6812|cpu9s12x, 0 },

  { "bclr", OP_BITMASK|OP_DIRECT,  3, 0x15,  6,  6,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "bclr", OP_BITMASK|OP_IX,       3, 0x1d,  7,  7,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "bclr", OP_BITMASK|OP_IY|OP_PAGE2, 4, 0x1d, 8, 8, CLR_V_CHG_NZ, cpu6811, 0 },
  { "bclr", OP_BITMASK|OP_DIRECT,   3, 0x4d,  4,  4,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "bclr", OP_BITMASK|OP_IND16,    4, 0x1d,  4,  4,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "bclr", OP_BITMASK|OP_IDX,      3, 0x0d,  4,  4,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "bclr", OP_BITMASK|OP_IDX_1,    4, 0x0d,  4,  4,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "bclr", OP_BITMASK|OP_IDX_2,    5, 0x0d,  6,  6,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },

  { "bcs", OP_JUMP_REL,        2, 0x25,  1,  3, CHG_NONE, cpu6811 | cpu6812|cpu9s12x, 0 },
  { "beq", OP_JUMP_REL,        2, 0x27,  1,  3, CHG_NONE, cpu6811 | cpu6812|cpu9s12x, 0 },
  { "bge", OP_JUMP_REL,        2, 0x2c,  1,  3, CHG_NONE, cpu6811 | cpu6812|cpu9s12x, 0 },

  { "bgnd", OP_NONE,           1, 0x00,  5,  5, CHG_NONE, cpu6811 | cpu6812|cpu9s12x, 0 },

  { "bgt", OP_JUMP_REL,        2, 0x2e,  1,  3, CHG_NONE, cpu6811 | cpu6812|cpu9s12x, 0 },
  { "bhi", OP_JUMP_REL,        2, 0x22,  1,  3, CHG_NONE, cpu6811 | cpu6812|cpu9s12x, 0 },
  { "bhs", OP_JUMP_REL,        2, 0x24,  1,  3, CHG_NONE, cpu6811 | cpu6812|cpu9s12x, 0 },

  { "bita", OP_IMM8,          2, 0x85,  1,  1, CLR_V_CHG_NZ, cpu6811|cpu6812|cpu9s12x, 0 },
  { "bita", OP_DIRECT,        2, 0x95,  3,  3, CLR_V_CHG_NZ, cpu6811|cpu6812|cpu9s12x, 0 },
  { "bita", OP_IND16,         3, 0xb5,  3,  3, CLR_V_CHG_NZ, cpu6811|cpu6812|cpu9s12x, 0 },
  { "bita", OP_IX,             2, 0xa5,  4,  4,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "bita", OP_IY | OP_PAGE2,  3, 0xa5,  5,  5,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "bita", OP_IDX,            2, 0xa5,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "bita", OP_IDX_1,          3, 0xa5,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "bita", OP_IDX_2,          4, 0xa5,  4,  4,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "bita", OP_D_IDX,          2, 0xa5,  6,  6,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "bita", OP_D_IDX_2,        4, 0xa5,  6,  6,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },

  { "bitb", OP_IMM8,          2, 0xc5,  1,  1, CLR_V_CHG_NZ, cpu6811|cpu6812|cpu9s12x, 0 },
  { "bitb", OP_DIRECT,        2, 0xd5,  3,  3, CLR_V_CHG_NZ, cpu6811|cpu6812|cpu9s12x, 0 },
  { "bitb", OP_IND16,         3, 0xf5,  3,  3, CLR_V_CHG_NZ, cpu6811|cpu6812|cpu9s12x, 0 },
  { "bitb", OP_IX,             2, 0xe5,  4,  4,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "bitb", OP_IY | OP_PAGE2,  3, 0xe5,  5,  5,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "bitb", OP_IDX,            2, 0xe5,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "bitb", OP_IDX_1,          3, 0xe5,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "bitb", OP_IDX_2,          4, 0xe5,  4,  4,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "bitb", OP_D_IDX,          2, 0xe5,  6,  6,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "bitb", OP_D_IDX_2,        4, 0xe5,  6,  6,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },

  { "bitx", OP_IMM16 | OP_PAGE2,          2, 0x85,  1,  1, CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "bitx", OP_DIRECT | OP_PAGE2,        2, 0x95,  3,  3, CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "bitx", OP_IND16 | OP_PAGE2,         3, 0xb5,  3,  3, CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "bitx", OP_IDX | OP_PAGE2,            2, 0xa5,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "bitx", OP_IDX_1 | OP_PAGE2,          3, 0xa5,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "bitx", OP_IDX_2 | OP_PAGE2,          4, 0xa5,  4,  4,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "bitx", OP_D_IDX | OP_PAGE2,          2, 0xa5,  6,  6,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "bitx", OP_D_IDX_2 | OP_PAGE2,        4, 0xa5,  6,  6,  CLR_V_CHG_NZ, cpu9s12x, 0 },

  { "bity", OP_IMM16 | OP_PAGE2,          2, 0xc5,  1,  1, CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "bity", OP_DIRECT | OP_PAGE2,        2, 0xd5,  3,  3, CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "bity", OP_IND16 | OP_PAGE2,         3, 0xf5,  3,  3, CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "bity", OP_IDX | OP_PAGE2,            2, 0xe5,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "bity", OP_IDX_1 | OP_PAGE2,          3, 0xe5,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "bity", OP_IDX_2 | OP_PAGE2,          4, 0xe5,  4,  4,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "bity", OP_D_IDX | OP_PAGE2,          2, 0xe5,  6,  6,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "bity", OP_D_IDX_2 | OP_PAGE2,        4, 0xe5,  6,  6,  CLR_V_CHG_NZ, cpu9s12x, 0 },

  { "ble", OP_JUMP_REL,        2, 0x2f,  1,  3, CHG_NONE, cpu6811 | cpu6812|cpu9s12x, 0 },
  { "blo", OP_JUMP_REL,        2, 0x25,  1,  3, CHG_NONE, cpu6811 | cpu6812|cpu9s12x, 0 },
  { "bls", OP_JUMP_REL,        2, 0x23,  1,  3, CHG_NONE, cpu6811 | cpu6812|cpu9s12x, 0 },
  { "blt", OP_JUMP_REL,        2, 0x2d,  1,  3, CHG_NONE, cpu6811 | cpu6812|cpu9s12x, 0 },
  { "bmi", OP_JUMP_REL,        2, 0x2b,  1,  3, CHG_NONE, cpu6811 | cpu6812|cpu9s12x, 0 },
  { "bne", OP_JUMP_REL,        2, 0x26,  1,  3, CHG_NONE, cpu6811 | cpu6812|cpu9s12x, 0 },
  { "bpl", OP_JUMP_REL,        2, 0x2a,  1,  3, CHG_NONE, cpu6811 | cpu6812|cpu9s12x, 0 },
  { "bra", OP_JUMP_REL,        2, 0x20,  1,  3, CHG_NONE, cpu6811 | cpu6812|cpu9s12x, 0 },

  { "brclr", OP_BITMASK | OP_JUMP_REL
           | OP_DIRECT,         4, 0x13,  6,  6, CHG_NONE, cpu6811, 0 },
  { "brclr", OP_BITMASK | OP_JUMP_REL
           | OP_IX,             4, 0x1f,  7,  7, CHG_NONE, cpu6811, 0 },
  { "brclr", OP_BITMASK | OP_JUMP_REL
           | OP_IY | OP_PAGE2,  5, 0x1f,  8,  8, CHG_NONE, cpu6811, 0 },
  { "brclr", OP_BITMASK | OP_JUMP_REL
           | OP_DIRECT,         4, 0x4f,  4,  4,  CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "brclr", OP_BITMASK | OP_JUMP_REL
           | OP_IND16,          5, 0x1f,  5,  5,  CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "brclr", OP_BITMASK | OP_JUMP_REL
           | OP_IDX,            4, 0x0f,  4,  4,  CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "brclr", OP_BITMASK | OP_JUMP_REL
           | OP_IDX_1,          5, 0x0f,  6,  6,  CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "brclr", OP_BITMASK
           | OP_JUMP_REL
           | OP_IDX_2,          6, 0x0f,  8,  8,  CHG_NONE, cpu6812|cpu9s12x, 0 },

  { "brn", OP_JUMP_REL,         2, 0x21,  1,  3,  CHG_NONE, cpu6811|cpu6812|cpu9s12x, 0 },

  { "brset", OP_BITMASK | OP_JUMP_REL
           | OP_DIRECT,         4, 0x12,  6,  6,  CHG_NONE, cpu6811, 0 },
  { "brset", OP_BITMASK
           | OP_JUMP_REL
           | OP_IX,             4, 0x1e,  7,  7,  CHG_NONE, cpu6811, 0 },
  { "brset", OP_BITMASK | OP_JUMP_REL
           | OP_IY | OP_PAGE2,  5, 0x1e,  8,  8,  CHG_NONE, cpu6811, 0 },
  { "brset", OP_BITMASK | OP_JUMP_REL
           | OP_DIRECT,   4, 0x4e,  4,  4,  CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "brset", OP_BITMASK | OP_JUMP_REL
           | OP_IND16,    5, 0x1e,  5,  5,  CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "brset", OP_BITMASK | OP_JUMP_REL
           | OP_IDX,            4, 0x0e,  4,  4,  CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "brset", OP_BITMASK | OP_JUMP_REL
           | OP_IDX_1,          5, 0x0e,  6,  6,  CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "brset", OP_BITMASK | OP_JUMP_REL
           | OP_IDX_2,          6, 0x0e,  8,  8,  CHG_NONE, cpu6812|cpu9s12x, 0 },


  { "bset", OP_BITMASK | OP_DIRECT,   3, 0x14,  6,  6, CLR_V_CHG_NZ, cpu6811, 0 },
  { "bset", OP_BITMASK | OP_IX,       3, 0x1c,  7,  7, CLR_V_CHG_NZ, cpu6811, 0 },
  { "bset", OP_BITMASK|OP_IY|OP_PAGE2, 4, 0x1c, 8, 8, CLR_V_CHG_NZ, cpu6811, 0 },
  { "bset", OP_BITMASK|OP_DIRECT,   3, 0x4c,  4,  4,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "bset", OP_BITMASK|OP_IND16,    4, 0x1c,  4,  4,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "bset", OP_BITMASK|OP_IDX,      3, 0x0c,  4,  4,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "bset", OP_BITMASK|OP_IDX_1,    4, 0x0c,  4,  4,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "bset", OP_BITMASK|OP_IDX_2,    5, 0x0c,  6,  6,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },

  { "bsr",  OP_JUMP_REL,       2, 0x8d,  6,  6, CHG_NONE, cpu6811, 0 },
  { "bsr",  OP_JUMP_REL,       2, 0x07,  4,  4, CHG_NONE, cpu6812|cpu9s12x, 0 },

  { "btas", OP_BITMASK|OP_DIRECT | OP_PAGE2,   3, 0x35,  4,  4,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "btas", OP_BITMASK|OP_IND16 | OP_PAGE2,    4, 0x36,  4,  4,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "btas", OP_BITMASK|OP_IDX | OP_PAGE2,      3, 0x37,  4,  4,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "btas", OP_BITMASK|OP_IDX_1 | OP_PAGE2,    4, 0x37,  4,  4,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "btas", OP_BITMASK|OP_IDX_2 | OP_PAGE2,    5, 0x37,  6,  6,  CLR_V_CHG_NZ, cpu9s12x, 0 },

  { "bvc",  OP_JUMP_REL,       2, 0x28,  1,  3, CHG_NONE, cpu6811 | cpu6812|cpu9s12x, 0 },
  { "bvs",  OP_JUMP_REL,       2, 0x29,  1,  3, CHG_NONE, cpu6811 | cpu6812|cpu9s12x, 0 },

  { "call", OP_IND16 | OP_PAGE
          | OP_BRANCH,         4, 0x4a,  8,  8,  CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "call", OP_IDX | OP_PAGE
          | OP_BRANCH,         3, 0x4b,  8,  8,  CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "call", OP_IDX_1 | OP_PAGE
          | OP_BRANCH,         4, 0x4b,  8,  8,  CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "call", OP_IDX_2 | OP_PAGE
          | OP_BRANCH,         5, 0x4b,  9,  9,  CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "call", OP_D_IDX
          | OP_BRANCH,         2, 0x4b, 10, 10,  CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "call", OP_D_IDX_2
          | OP_BRANCH,         4, 0x4b, 10, 10,  CHG_NONE, cpu6812|cpu9s12x, 0 },

  { "cba",  OP_NONE,           1, 0x11,  2,  2,  CHG_NZVC, cpu6811, 0 },
  { "cba",  OP_NONE | OP_PAGE2,2, 0x17,  2,  2,  CHG_NZVC, cpu6812|cpu9s12x, 0 },

  { "clc",  OP_NONE,           1, 0x0c,  2,  2,  CLR_C, cpu6811, 0 },
  { "cli",  OP_NONE,           1, 0x0e,  2,  2,  CLR_I, cpu6811, 0 },

  { "clr", OP_IND16,           3, 0x7f,  6,  6,  SET_Z_CLR_NVC, cpu6811, 0 },
  { "clr", OP_IX,              2, 0x6f,  6,  6,  SET_Z_CLR_NVC, cpu6811, 0 },
  { "clr", OP_IY | OP_PAGE2,   3, 0x6f,  7,  7,  SET_Z_CLR_NVC, cpu6811, 0 },
  { "clr", OP_IND16,           3, 0x79,  3,  3,  SET_Z_CLR_NVC, cpu6812|cpu9s12x, 0 },
  { "clr", OP_IDX,             2, 0x69,  2,  2,  SET_Z_CLR_NVC, cpu6812|cpu9s12x, 0 },
  { "clr", OP_IDX_1,           3, 0x69,  3,  3,  SET_Z_CLR_NVC, cpu6812|cpu9s12x, 0 },
  { "clr", OP_IDX_2,           4, 0x69,  4,  4,  SET_Z_CLR_NVC, cpu6812|cpu9s12x, 0 },
  { "clr", OP_D_IDX,           2, 0x69,  5,  5,  SET_Z_CLR_NVC, cpu6812|cpu9s12x, 0 },
  { "clr", OP_D_IDX_2,         4, 0x69,  5,  5,  SET_Z_CLR_NVC, cpu6812|cpu9s12x, 0 },

  { "clra", OP_NONE,           1, 0x4f,  2,  2,  SET_Z_CLR_NVC, cpu6811, 0 },
  { "clrb", OP_NONE,           1, 0x5f,  2,  2,  SET_Z_CLR_NVC, cpu6811, 0 },
  { "clra", OP_NONE,           1, 0x87,  1,  1,  SET_Z_CLR_NVC, cpu6812|cpu9s12x, 0 },
  { "clrb", OP_NONE,           1, 0xc7,  1,  1,  SET_Z_CLR_NVC, cpu6812|cpu9s12x, 0 },

  { "clrw",  OP_IND16 | OP_PAGE2,          3, 0x79,  4,  4,  SET_Z_CLR_NVC, cpu9s12x, 0 },
  { "clrw",  OP_IDX | OP_PAGE2,            2, 0x69,  3,  3,  SET_Z_CLR_NVC, cpu9s12x, 0 },
  { "clrw",  OP_IDX_1 | OP_PAGE2,          3, 0x69,  4,  4,  SET_Z_CLR_NVC, cpu9s12x, 0 },
  { "clrw",  OP_IDX_2 | OP_PAGE2,          4, 0x69,  5,  5,  SET_Z_CLR_NVC, cpu9s12x, 0 },
  { "clrw",  OP_D_IDX | OP_PAGE2,          2, 0x69,  6,  6,  SET_Z_CLR_NVC, cpu9s12x, 0 },
  { "clrw",  OP_D_IDX_2 | OP_PAGE2,        4, 0x69,  6,  6,  SET_Z_CLR_NVC, cpu9s12x, 0 },

  { "clrx",  OP_NONE | OP_PAGE2,          3, 0x87,  4,  4,  SET_Z_CLR_NVC, cpu9s12x, 0 },

  { "clry",  OP_NONE | OP_PAGE2,          3, 0xc7,  4,  4,  SET_Z_CLR_NVC, cpu9s12x, 0 },

  { "clv",  OP_NONE,           1, 0x0a,  2,  2,  CLR_V, cpu6811, 0 },

  { "cmpa", OP_IMM8,           2, 0x81,  1,  1,  CHG_NZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "cmpa", OP_DIRECT,         2, 0x91,  3,  3,  CHG_NZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "cmpa", OP_IND16,          3, 0xb1,  3,  3,  CHG_NZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "cmpa", OP_IX,             2, 0xa1,  4,  4,  CHG_NZVC, cpu6811, 0 },
  { "cmpa", OP_IY | OP_PAGE2,  3, 0xa1,  5,  5,  CHG_NZVC, cpu6811, 0 },
  { "cmpa", OP_IDX,            2, 0xa1,  3,  3,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "cmpa", OP_IDX_1,          3, 0xa1,  3,  3,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "cmpa", OP_IDX_2,          4, 0xa1,  4,  4,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "cmpa", OP_D_IDX,          2, 0xa1,  6,  6,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "cmpa", OP_D_IDX_2,        4, 0xa1,  6,  6,  CHG_NZVC, cpu6812|cpu9s12x, 0 },

  { "cmpb", OP_IMM8,           2, 0xc1,  1,  1,  CHG_NZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "cmpb", OP_DIRECT,         2, 0xd1,  3,  3,  CHG_NZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "cmpb", OP_IND16,          3, 0xf1,  3,  3,  CHG_NZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "cmpb", OP_IX,             2, 0xe1,  4,  4,  CHG_NZVC, cpu6811, 0 },
  { "cmpb", OP_IY | OP_PAGE2,  3, 0xe1,  5,  5,  CHG_NZVC, cpu6811, 0 },
  { "cmpb", OP_IDX,            2, 0xe1,  3,  3,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "cmpb", OP_IDX_1,          3, 0xe1,  3,  3,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "cmpb", OP_IDX_2,          4, 0xe1,  4,  4,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "cmpb", OP_D_IDX,          2, 0xe1,  6,  6,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "cmpb", OP_D_IDX_2,        4, 0xe1,  6,  6,  CHG_NZVC, cpu6812|cpu9s12x, 0 },

  { "com", OP_IND16,           3, 0x73,  6,  6,  SET_C_CLR_V_CHG_NZ, cpu6811, 0 },
  { "com", OP_IX,              2, 0x63,  6,  6,  SET_C_CLR_V_CHG_NZ, cpu6811, 0 },
  { "com", OP_IY | OP_PAGE2,   3, 0x63,  7,  7,  SET_C_CLR_V_CHG_NZ, cpu6811, 0 },
  { "com", OP_IND16,           3, 0x71,  4,  4,  SET_C_CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "com", OP_IDX,             2, 0x61,  3,  3,  SET_C_CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "com", OP_IDX_1,           3, 0x61,  4,  4,  SET_C_CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "com", OP_IDX_2,           4, 0x61,  5,  5,  SET_C_CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "com", OP_D_IDX,           2, 0x61,  6,  6,  SET_C_CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "com", OP_D_IDX_2,         4, 0x61,  6,  6,  SET_C_CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },

  { "coma", OP_NONE,           1, 0x43,  2,  2,  SET_C_CLR_V_CHG_NZ, cpu6811, 0 },
  { "coma", OP_NONE,           1, 0x41,  1,  1,  SET_C_CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "comb", OP_NONE,           1, 0x53,  2,  2,  SET_C_CLR_V_CHG_NZ, cpu6811, 0 },
  { "comb", OP_NONE,           1, 0x51,  1,  1,  SET_C_CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },

  { "comw",  OP_IND16 | OP_PAGE2,          3, 0x71,  4,  4,  SET_C_CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "comw",  OP_IDX | OP_PAGE2,            2, 0x61,  3,  3,  SET_C_CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "comw",  OP_IDX_1 | OP_PAGE2,          3, 0x61,  4,  4,  SET_C_CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "comw",  OP_IDX_2 | OP_PAGE2,          4, 0x61,  5,  5,  SET_C_CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "comw",  OP_D_IDX | OP_PAGE2,          2, 0x61,  6,  6,  SET_C_CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "comw",  OP_D_IDX_2 | OP_PAGE2,        4, 0x61,  6,  6,  SET_C_CLR_V_CHG_NZ, cpu9s12x, 0 },

  { "comx", OP_NONE | OP_PAGE2,           1, 0x41,  2,  2,  SET_C_CLR_V_CHG_NZ, cpu9s12x, 0 },

  { "comy", OP_NONE | OP_PAGE2,           1, 0x51,  2,  2,  SET_C_CLR_V_CHG_NZ, cpu9s12x, 0 },

  { "cpd", OP_IMM16 | OP_PAGE3,  4, 0x83,  5,  5,  CHG_NZVC, cpu6811, 0 },
  { "cpd", OP_DIRECT | OP_PAGE3, 3, 0x93,  6,  6,  CHG_NZVC, cpu6811, 0 },
  { "cpd", OP_IND16 | OP_PAGE3,  4, 0xb3,  7,  7,  CHG_NZVC, cpu6811, 0 },
  { "cpd", OP_IX | OP_PAGE3,     3, 0xa3,  7,  7,  CHG_NZVC, cpu6811, 0 },
  { "cpd", OP_IY | OP_PAGE4,     3, 0xa3,  7,  7,  CHG_NZVC, cpu6811, 0 },
  { "cpd", OP_IMM16,             3, 0x8c,  2,  2,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "cpd", OP_DIRECT,            2, 0x9c,  3,  3,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "cpd", OP_IND16,             3, 0xbc,  3,  3,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "cpd", OP_IDX,               2, 0xac,  3,  3,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "cpd", OP_IDX_1,             3, 0xac,  3,  3,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "cpd", OP_IDX_2,             4, 0xac,  4,  4,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "cpd", OP_D_IDX,             2, 0xac,  6,  6,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "cpd", OP_D_IDX_2,           4, 0xac,  6,  6,  CHG_NZVC, cpu6812|cpu9s12x, 0 },

  { "cped", OP_IMM16 | OP_PAGE2,             3, 0x8c,  2,  2,  CHG_NZVC, cpu9s12x, 0 },
  { "cped", OP_DIRECT | OP_PAGE2,            2, 0x9c,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "cped", OP_IND16 | OP_PAGE2,             3, 0xbc,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "cped", OP_IDX | OP_PAGE2,               2, 0xac,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "cped", OP_IDX_1 | OP_PAGE2,             3, 0xac,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "cped", OP_IDX_2 | OP_PAGE2,             4, 0xac,  4,  4,  CHG_NZVC, cpu9s12x, 0 },
  { "cped", OP_D_IDX | OP_PAGE2,             2, 0xac,  6,  6,  CHG_NZVC, cpu9s12x, 0 },
  { "cped", OP_D_IDX_2 | OP_PAGE2,           4, 0xac,  6,  6,  CHG_NZVC, cpu9s12x, 0 },

  { "cpes", OP_IMM16 | OP_PAGE2,             3, 0x8f,  2,  2,  CHG_NZVC, cpu9s12x, 0 },
  { "cpes", OP_DIRECT | OP_PAGE2,            2, 0x9f,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "cpes", OP_IND16 | OP_PAGE2,             3, 0xbf,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "cpes", OP_IDX | OP_PAGE2,               2, 0xaf,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "cpes", OP_IDX_1 | OP_PAGE2,             3, 0xaf,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "cpes", OP_IDX_2 | OP_PAGE2,             4, 0xaf,  4,  4,  CHG_NZVC, cpu9s12x, 0 },
  { "cpes", OP_D_IDX | OP_PAGE2,             2, 0xaf,  6,  6,  CHG_NZVC, cpu9s12x, 0 },
  { "cpes", OP_D_IDX_2 | OP_PAGE2,           4, 0xaf,  6,  6,  CHG_NZVC, cpu9s12x, 0 },

  { "cpex", OP_IMM16 | OP_PAGE2,             3, 0x8e,  2,  2,  CHG_NZVC, cpu9s12x, 0 },
  { "cpex", OP_DIRECT | OP_PAGE2,            2, 0x9e,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "cpex", OP_IND16 | OP_PAGE2,             3, 0xbe,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "cpex", OP_IDX | OP_PAGE2,               2, 0xae,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "cpex", OP_IDX_1 | OP_PAGE2,             3, 0xae,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "cpex", OP_IDX_2 | OP_PAGE2,             4, 0xae,  4,  4,  CHG_NZVC, cpu9s12x, 0 },
  { "cpex", OP_D_IDX | OP_PAGE2,             2, 0xae,  6,  6,  CHG_NZVC, cpu9s12x, 0 },
  { "cpex", OP_D_IDX_2 | OP_PAGE2,           4, 0xae,  6,  6,  CHG_NZVC, cpu9s12x, 0 },

  { "cpey", OP_IMM16 | OP_PAGE2,             3, 0x8d,  2,  2,  CHG_NZVC, cpu9s12x, 0 },
  { "cpey", OP_DIRECT | OP_PAGE2,            2, 0x9d,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "cpey", OP_IND16 | OP_PAGE2,             3, 0xbd,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "cpey", OP_IDX | OP_PAGE2,               2, 0xad,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "cpey", OP_IDX_1 | OP_PAGE2,             3, 0xad,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "cpey", OP_IDX_2 | OP_PAGE2,             4, 0xad,  4,  4,  CHG_NZVC, cpu9s12x, 0 },
  { "cpey", OP_D_IDX | OP_PAGE2,             2, 0xad,  6,  6,  CHG_NZVC, cpu9s12x, 0 },
  { "cpey", OP_D_IDX_2 | OP_PAGE2,           4, 0xad,  6,  6,  CHG_NZVC, cpu9s12x, 0 },

  { "cps", OP_IMM16,             3, 0x8f,  2,  2,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "cps", OP_DIRECT,            2, 0x9f,  3,  3,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "cps", OP_IND16,             3, 0xbf,  3,  3,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "cps", OP_IDX,               2, 0xaf,  3,  3,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "cps", OP_IDX_1,             3, 0xaf,  3,  3,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "cps", OP_IDX_2,             4, 0xaf,  4,  4,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "cps", OP_D_IDX,             2, 0xaf,  6,  6,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "cps", OP_D_IDX_2,           4, 0xaf,  6,  6,  CHG_NZVC, cpu6812|cpu9s12x, 0 },

  { "cpx", OP_IMM16,             3, 0x8c,  4,  4,  CHG_NZVC, cpu6811, 0 },
  { "cpx", OP_DIRECT,            2, 0x9c,  5,  5,  CHG_NZVC, cpu6811, 0 },
  { "cpx", OP_IND16,             3, 0xbc,  5,  5,  CHG_NZVC, cpu6811, 0 },
  { "cpx", OP_IX,                2, 0xac,  6,  6,  CHG_NZVC, cpu6811, 0 },
  { "cpx", OP_IY | OP_PAGE4,     3, 0xac,  7,  7,  CHG_NZVC, cpu6811, 0 },
  { "cpx", OP_IMM16,             3, 0x8e,  2,  2,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "cpx", OP_DIRECT,            2, 0x9e,  3,  3,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "cpx", OP_IND16,             3, 0xbe,  3,  3,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "cpx", OP_IDX,               2, 0xae,  3,  3,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "cpx", OP_IDX_1,             3, 0xae,  3,  3,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "cpx", OP_IDX_2,             4, 0xae,  4,  4,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "cpx", OP_D_IDX,             2, 0xae,  6,  6,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "cpx", OP_D_IDX_2,           4, 0xae,  6,  6,  CHG_NZVC, cpu6812|cpu9s12x, 0 },

  { "cpy", OP_PAGE2 | OP_IMM16,  4, 0x8c,  5,  5,  CHG_NZVC, cpu6811, 0 },
  { "cpy", OP_PAGE2 | OP_DIRECT, 3, 0x9c,  6,  6,  CHG_NZVC, cpu6811, 0 },
  { "cpy", OP_PAGE2 | OP_IY,     3, 0xac,  7,  7,  CHG_NZVC, cpu6811, 0 },
  { "cpy", OP_PAGE2 | OP_IND16,  4, 0xbc,  7,  7,  CHG_NZVC, cpu6811, 0 },
  { "cpy", OP_PAGE3 | OP_IX,     3, 0xac,  7,  7,  CHG_NZVC, cpu6811, 0 },
  { "cpy", OP_IMM16,             3, 0x8d,  2,  2,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "cpy", OP_DIRECT,            2, 0x9d,  3,  3,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "cpy", OP_IND16,             3, 0xbd,  3,  3,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "cpy", OP_IDX,               2, 0xad,  3,  3,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "cpy", OP_IDX_1,             3, 0xad,  3,  3,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "cpy", OP_IDX_2,             4, 0xad,  4,  4,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "cpy", OP_D_IDX,             2, 0xad,  6,  6,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "cpy", OP_D_IDX_2,           4, 0xad,  6,  6,  CHG_NZVC, cpu6812|cpu9s12x, 0 },

  /* After 'daa', the Z flag is undefined. Mark it as changed.  */
  { "daa", OP_NONE,              1, 0x19,  2,  2,  CHG_NZVC, cpu6811, 0 },
  { "daa", OP_NONE | OP_PAGE2,  2, 0x07,  3,  3,  CHG_NZVC, cpu6812|cpu9s12x, 0 },

  { "dbeq", OP_DBEQ_MARKER
          | OP_REG | OP_JUMP_REL,3, 0x04,  3,  3, CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "dbne", OP_DBNE_MARKER
          | OP_REG | OP_JUMP_REL,3, 0x04,  3,  3, CHG_NONE, cpu6812|cpu9s12x, 0 },

  { "dec", OP_IX,                2, 0x6a,  6,  6,  CHG_NZV, cpu6811, 0 },
  { "dec", OP_IND16,             3, 0x7a,  6,  6,  CHG_NZV, cpu6811, 0 },
  { "dec", OP_IY | OP_PAGE2,     3, 0x6a,  7,  7,  CHG_NZV, cpu6811, 0 },
  { "dec", OP_IND16,             3, 0x73,  4,  4,  CHG_NZV, cpu6812|cpu9s12x, 0 },
  { "dec", OP_IDX,               2, 0x63,  3,  3,  CHG_NZV, cpu6812|cpu9s12x, 0 },
  { "dec", OP_IDX_1,             3, 0x63,  4,  4,  CHG_NZV, cpu6812|cpu9s12x, 0 },
  { "dec", OP_IDX_2,             4, 0x63,  5,  5,  CHG_NZV, cpu6812|cpu9s12x, 0 },
  { "dec", OP_D_IDX,             2, 0x63,  6,  6,  CHG_NZV, cpu6812|cpu9s12x, 0 },
  { "dec", OP_D_IDX_2,           4, 0x63,  6,  6,  CHG_NZV, cpu6812|cpu9s12x, 0 },

  { "des",  OP_NONE,             1, 0x34,  3,  3,  CHG_NONE, cpu6811, 0 },

  { "deca", OP_NONE,             1, 0x4a,  2,  2,  CHG_NZV, cpu6811, 0 },
  { "deca", OP_NONE,             1, 0x43,  1,  1,  CHG_NZV, cpu6812|cpu9s12x, 0 },
  { "decb", OP_NONE,             1, 0x5a,  2,  2,  CHG_NZV, cpu6811, 0 },
  { "decb", OP_NONE,             1, 0x53,  1,  1,  CHG_NZV, cpu6812|cpu9s12x, 0 },

  { "decw",  OP_IND16 | OP_PAGE2,          3, 0x73,  4,  4,  CHG_NZV, cpu9s12x, 0 },
  { "decw",  OP_IDX | OP_PAGE2,            2, 0x63,  3,  3,  CHG_NZV, cpu9s12x, 0 },
  { "decw",  OP_IDX_1 | OP_PAGE2,          3, 0x63,  4,  4,  CHG_NZV, cpu9s12x, 0 },
  { "decw",  OP_IDX_2 | OP_PAGE2,          4, 0x63,  5,  5,  CHG_NZV, cpu9s12x, 0 },
  { "decw",  OP_D_IDX | OP_PAGE2,          2, 0x63,  6,  6,  CHG_NZV, cpu9s12x, 0 },
  { "decw",  OP_D_IDX_2 | OP_PAGE2,        4, 0x63,  6,  6,  CHG_NZV, cpu9s12x, 0 },

  { "decx",  OP_NONE | OP_PAGE2,          3, 0x43,  4,  4,  CHG_NZV, cpu9s12x, 0 },

  { "decy",  OP_NONE | OP_PAGE2,          3, 0x53,  4,  4,  CHG_NZV, cpu9s12x, 0 },

  { "dex",  OP_NONE,             1, 0x09,  1,  1,  CHG_Z, cpu6812|cpu9s12x|cpu6811, 0 },
  { "dey",  OP_NONE | OP_PAGE2,  2, 0x09,  4,  4,  CHG_Z, cpu6811, 0 },
  { "dey",  OP_NONE,             1, 0x03,  1,  1,  CHG_Z, cpu6812|cpu9s12x, 0 },

  { "ediv", OP_NONE,             1, 0x11,  11,  11,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "edivs", OP_NONE | OP_PAGE2, 2, 0x14,  12,  12,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "emacs", OP_IND16 | OP_PAGE2, 4, 0x12,  13,  13,  CHG_NZVC, cpu6812|cpu9s12x, 0 },

  { "emaxd", OP_IDX | OP_PAGE2,     3, 0x1a,  4,  4,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "emaxd", OP_IDX_1 | OP_PAGE2,   4, 0x1a,  4,  4,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "emaxd", OP_IDX_2 | OP_PAGE2,   5, 0x1a,  5,  5,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "emaxd", OP_D_IDX | OP_PAGE2,   3, 0x1a,  7,  7,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "emaxd", OP_D_IDX_2 | OP_PAGE2, 5, 0x1a,  7,  7,  CHG_NZVC, cpu6812|cpu9s12x, 0 },

  { "emaxm", OP_IDX | OP_PAGE2,     3, 0x1e,  4,  4,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "emaxm", OP_IDX_1 | OP_PAGE2,   4, 0x1e,  5,  5,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "emaxm", OP_IDX_2 | OP_PAGE2,   5, 0x1e,  6,  6,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "emaxm", OP_D_IDX | OP_PAGE2,   3, 0x1e,  7,  7,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "emaxm", OP_D_IDX_2 | OP_PAGE2, 5, 0x1e,  7,  7,  CHG_NZVC, cpu6812|cpu9s12x, 0 },

  { "emind", OP_IDX | OP_PAGE2,     3, 0x1b,  4,  4,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "emind", OP_IDX_1 | OP_PAGE2,   4, 0x1b,  4,  4,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "emind", OP_IDX_2 | OP_PAGE2,   5, 0x1b,  5,  5,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "emind", OP_D_IDX | OP_PAGE2,   3, 0x1b,  7,  7,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "emind", OP_D_IDX_2 | OP_PAGE2, 5, 0x1b,  7,  7,  CHG_NZVC, cpu6812|cpu9s12x, 0 },

  { "eminm", OP_IDX | OP_PAGE2,     3, 0x1f,  4,  4,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "eminm", OP_IDX_1 | OP_PAGE2,   4, 0x1f,  5,  5,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "eminm", OP_IDX_2 | OP_PAGE2,   5, 0x1f,  6,  6,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "eminm", OP_D_IDX | OP_PAGE2,   3, 0x1f,  7,  7,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "eminm", OP_D_IDX_2 | OP_PAGE2, 5, 0x1f,  7,  7,  CHG_NZVC, cpu6812|cpu9s12x, 0 },

  { "emul",  OP_NONE,               1, 0x13,  3,  3,  CHG_NZC, cpu6812|cpu9s12x, 0 },
  { "emuls", OP_NONE | OP_PAGE2,    2, 0x13,  3,  3,  CHG_NZC, cpu6812|cpu9s12x, 0 },

  { "eora", OP_IMM8,         2, 0x88,  1,  1,  CLR_V_CHG_NZ, cpu6811|cpu6812|cpu9s12x, 0 },
  { "eora", OP_DIRECT,       2, 0x98,  3,  3,  CLR_V_CHG_NZ, cpu6811|cpu6812|cpu9s12x, 0 },
  { "eora", OP_IND16,        3, 0xb8,  3,  3,  CLR_V_CHG_NZ, cpu6811|cpu6812|cpu9s12x, 0 },
  { "eora", OP_IX,             2, 0xa8,  4,  4,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "eora", OP_IY | OP_PAGE2,  3, 0xa8,  5,  5,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "eora", OP_IDX,            2, 0xa8,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "eora", OP_IDX_1,          3, 0xa8,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "eora", OP_IDX_2,          4, 0xa8,  4,  4,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "eora", OP_D_IDX,          2, 0xa8,  6,  6,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "eora", OP_D_IDX_2,        4, 0xa8,  6,  6,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },

  { "eorb", OP_IMM8,         2, 0xc8,  1,  1,  CLR_V_CHG_NZ, cpu6811|cpu6812|cpu9s12x, 0 },
  { "eorb", OP_DIRECT,       2, 0xd8,  3,  3,  CLR_V_CHG_NZ, cpu6811|cpu6812|cpu9s12x, 0 },
  { "eorb", OP_IND16,        3, 0xf8,  3,  3,  CLR_V_CHG_NZ, cpu6811|cpu6812|cpu9s12x, 0 },
  { "eorb", OP_IX,             2, 0xe8,  4,  4,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "eorb", OP_IY | OP_PAGE2,  3, 0xe8,  5,  5,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "eorb", OP_IDX,            2, 0xe8,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "eorb", OP_IDX_1,          3, 0xe8,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "eorb", OP_IDX_2,          4, 0xe8,  4,  4,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "eorb", OP_D_IDX,          2, 0xe8,  6,  6,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "eorb", OP_D_IDX_2,        4, 0xe8,  6,  6,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },

  { "eorx", OP_IMM16 | OP_PAGE2,         2, 0x88,  1,  1,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "eorx", OP_DIRECT | OP_PAGE2,       2, 0x98,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "eorx", OP_IND16 | OP_PAGE2,        3, 0xb8,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "eorx", OP_IDX | OP_PAGE2,            2, 0xa8,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "eorx", OP_IDX_1 | OP_PAGE2,          3, 0xa8,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "eorx", OP_IDX_2 | OP_PAGE2,          4, 0xa8,  4,  4,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "eorx", OP_D_IDX | OP_PAGE2,          2, 0xa8,  6,  6,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "eorx", OP_D_IDX_2 | OP_PAGE2,        4, 0xa8,  6,  6,  CLR_V_CHG_NZ, cpu9s12x, 0 },

  { "eory", OP_IMM16 | OP_PAGE2,         2, 0xc8,  1,  1,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "eory", OP_DIRECT | OP_PAGE2,       2, 0xd8,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "eory", OP_IND16 | OP_PAGE2,        3, 0xf8,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "eory", OP_IDX | OP_PAGE2,            2, 0xe8,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "eory", OP_IDX_1 | OP_PAGE2,          3, 0xe8,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "eory", OP_IDX_2 | OP_PAGE2,          4, 0xe8,  4,  4,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "eory", OP_D_IDX | OP_PAGE2,          2, 0xe8,  6,  6,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "eory", OP_D_IDX_2 | OP_PAGE2,        4, 0xe8,  6,  6,  CLR_V_CHG_NZ, cpu9s12x, 0 },

  { "etbl", OP_IDX | OP_PAGE2,3, 0x3f, 10, 10,  CHG_NZC, cpu6812|cpu9s12x, 0 },

/* S12X has more exg variants, most are pointless so not supported */
  { "exg",  OP_EXG_MARKER
          | OP_REG | OP_REG_2, 2, 0xb7, 1, 1,  CHG_NONE, cpu6812|cpu9s12x, 0 },

  { "fdiv", OP_NONE,             1, 0x03,  3, 41, CHG_ZVC, cpu6811, 0 },
  { "fdiv", OP_NONE | OP_PAGE2, 2, 0x11, 12, 12, CHG_ZVC, cpu6812|cpu9s12x, 0 },

  { "gldaa", OP_DIRECT | OP_PAGE2,       2, 0x96,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gldaa", OP_IND16 | OP_PAGE2,        3, 0xb6,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gldaa", OP_IDX | OP_PAGE2,            2, 0xa6,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gldaa", OP_IDX_1 | OP_PAGE2,          3, 0xa6,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gldaa", OP_IDX_2 | OP_PAGE2,          4, 0xa6,  4,  4,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gldaa", OP_D_IDX | OP_PAGE2,          2, 0xa6,  6,  6,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gldaa", OP_D_IDX_2 | OP_PAGE2,        4, 0xa6,  6,  6,  CLR_V_CHG_NZ, cpu9s12x, 0 },

  { "gldab", OP_DIRECT | OP_PAGE2,       2, 0xd6,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gldab", OP_IND16 | OP_PAGE2,        3, 0xf6,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gldab", OP_IDX | OP_PAGE2,            2, 0xe6,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gldab", OP_IDX_1 | OP_PAGE2,          3, 0xe6,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gldab", OP_IDX_2 | OP_PAGE2,          4, 0xe6,  4,  4,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gldab", OP_D_IDX | OP_PAGE2,          2, 0xe6,  6,  6,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gldab", OP_D_IDX_2 | OP_PAGE2,        4, 0xe6,  6,  6,  CLR_V_CHG_NZ, cpu9s12x, 0 },

  { "gldd", OP_DIRECT | OP_PAGE2,       2, 0xdc,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gldd", OP_IND16 | OP_PAGE2,        3, 0xfc,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gldd", OP_IDX | OP_PAGE2,            2, 0xec,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gldd", OP_IDX_1 | OP_PAGE2,          3, 0xec,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gldd", OP_IDX_2 | OP_PAGE2,          4, 0xec,  4,  4,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gldd", OP_D_IDX | OP_PAGE2,          2, 0xec,  6,  6,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gldd", OP_D_IDX_2 | OP_PAGE2,        4, 0xec,  6,  6,  CLR_V_CHG_NZ, cpu9s12x, 0 },

  { "glds", OP_DIRECT | OP_PAGE2,       2, 0xdf,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "glds", OP_IND16 | OP_PAGE2,        3, 0xff,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "glds", OP_IDX | OP_PAGE2,            2, 0xef,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "glds", OP_IDX_1 | OP_PAGE2,          3, 0xef,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "glds", OP_IDX_2 | OP_PAGE2,          4, 0xef,  4,  4,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "glds", OP_D_IDX | OP_PAGE2,          2, 0xef,  6,  6,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "glds", OP_D_IDX_2 | OP_PAGE2,        4, 0xef,  6,  6,  CLR_V_CHG_NZ, cpu9s12x, 0 },

  { "gldx", OP_DIRECT | OP_PAGE2,       2, 0xde,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gldx", OP_IND16 | OP_PAGE2,        3, 0xfe,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gldx", OP_IDX | OP_PAGE2,            2, 0xee,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gldx", OP_IDX_1 | OP_PAGE2,          3, 0xee,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gldx", OP_IDX_2 | OP_PAGE2,          4, 0xee,  4,  4,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gldx", OP_D_IDX | OP_PAGE2,          2, 0xee,  6,  6,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gldx", OP_D_IDX_2 | OP_PAGE2,        4, 0xee,  6,  6,  CLR_V_CHG_NZ, cpu9s12x, 0 },

  { "gldy", OP_DIRECT | OP_PAGE2,       2, 0xdd,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gldy", OP_IND16 | OP_PAGE2,        3, 0xfd,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gldy", OP_IDX | OP_PAGE2,            2, 0xed,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gldy", OP_IDX_1 | OP_PAGE2,          3, 0xed,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gldy", OP_IDX_2 | OP_PAGE2,          4, 0xed,  4,  4,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gldy", OP_D_IDX | OP_PAGE2,          2, 0xed,  6,  6,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gldy", OP_D_IDX_2 | OP_PAGE2,        4, 0xed,  6,  6,  CLR_V_CHG_NZ, cpu9s12x, 0 },

  { "gstaa", OP_DIRECT | OP_PAGE2,       2, 0x5a,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gstaa", OP_IND16 | OP_PAGE2,        3, 0x7a,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gstaa", OP_IDX | OP_PAGE2,            2, 0x6a,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gstaa", OP_IDX_1 | OP_PAGE2,          3, 0x6a,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gstaa", OP_IDX_2 | OP_PAGE2,          4, 0x6a,  4,  4,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gstaa", OP_D_IDX | OP_PAGE2,          2, 0x6a,  6,  6,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gstaa", OP_D_IDX_2 | OP_PAGE2,        4, 0x6a,  6,  6,  CLR_V_CHG_NZ, cpu9s12x, 0 },

  { "gstab", OP_DIRECT | OP_PAGE2,       2, 0x5b,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gstab", OP_IND16 | OP_PAGE2,        3, 0x7b,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gstab", OP_IDX | OP_PAGE2,            2, 0x6b,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gstab", OP_IDX_1 | OP_PAGE2,          3, 0x6b,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gstab", OP_IDX_2 | OP_PAGE2,          4, 0x6b,  4,  4,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gstab", OP_D_IDX | OP_PAGE2,          2, 0x6b,  6,  6,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gstab", OP_D_IDX_2 | OP_PAGE2,        4, 0x6b,  6,  6,  CLR_V_CHG_NZ, cpu9s12x, 0 },

  { "gstd", OP_DIRECT | OP_PAGE2,       2, 0x5c,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gstd", OP_IND16 | OP_PAGE2,        3, 0x7c,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gstd", OP_IDX | OP_PAGE2,            2, 0x6c,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gstd", OP_IDX_1 | OP_PAGE2,          3, 0x6c,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gstd", OP_IDX_2 | OP_PAGE2,          4, 0x6c,  4,  4,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gstd", OP_D_IDX | OP_PAGE2,          2, 0x6c,  6,  6,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gstd", OP_D_IDX_2 | OP_PAGE2,        4, 0x6c,  6,  6,  CLR_V_CHG_NZ, cpu9s12x, 0 },

  { "gsts", OP_DIRECT | OP_PAGE2,       2, 0x5f,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gsts", OP_IND16 | OP_PAGE2,        3, 0x6f,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gsts", OP_IDX | OP_PAGE2,            2, 0x6f,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gsts", OP_IDX_1 | OP_PAGE2,          3, 0x6f,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gsts", OP_IDX_2 | OP_PAGE2,          4, 0x6f,  4,  4,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gsts", OP_D_IDX | OP_PAGE2,          2, 0x6f,  6,  6,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gsts", OP_D_IDX_2 | OP_PAGE2,        4, 0x6f,  6,  6,  CLR_V_CHG_NZ, cpu9s12x, 0 },

  { "gstx", OP_DIRECT | OP_PAGE2,       2, 0x5e,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gstx", OP_IND16 | OP_PAGE2,        3, 0x7e,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gstx", OP_IDX | OP_PAGE2,            2, 0x6e,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gstx", OP_IDX_1 | OP_PAGE2,          3, 0x6e,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gstx", OP_IDX_2 | OP_PAGE2,          4, 0x6e,  4,  4,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gstx", OP_D_IDX | OP_PAGE2,          2, 0x6e,  6,  6,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gstx", OP_D_IDX_2 | OP_PAGE2,        4, 0x6e,  6,  6,  CLR_V_CHG_NZ, cpu9s12x, 0 },

  { "gsty", OP_DIRECT | OP_PAGE2,       2, 0x5d,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gsty", OP_IND16 | OP_PAGE2,        3, 0x7d,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gsty", OP_IDX | OP_PAGE2,            2, 0x6d,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gsty", OP_IDX_1 | OP_PAGE2,          3, 0x6d,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gsty", OP_IDX_2 | OP_PAGE2,          4, 0x6d,  4,  4,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gsty", OP_D_IDX | OP_PAGE2,          2, 0x6d,  6,  6,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "gsty", OP_D_IDX_2 | OP_PAGE2,        4, 0x6d,  6,  6,  CLR_V_CHG_NZ, cpu9s12x, 0 },

  { "ibeq", OP_IBEQ_MARKER
          | OP_REG | OP_JUMP_REL,  3, 0x04,  3,  3, CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "ibne", OP_IBNE_MARKER
          | OP_REG | OP_JUMP_REL,  3, 0x04,  3,  3, CHG_NONE, cpu6812|cpu9s12x, 0 },

  { "idiv",  OP_NONE,              1, 0x02,  3, 41, CLR_V_CHG_ZC, cpu6811, 0 },
  { "idiv",  OP_NONE | OP_PAGE2,  2, 0x10, 12, 12, CLR_V_CHG_ZC, cpu6812|cpu9s12x, 0 },
  { "idivs", OP_NONE | OP_PAGE2,  2, 0x15, 12, 12, CHG_NZVC, cpu6812|cpu9s12x, 0 },

  { "inc", OP_IX,                  2, 0x6c,  6,  6,  CHG_NZV, cpu6811, 0 },
  { "inc", OP_IND16,               3, 0x7c,  6,  6,  CHG_NZV, cpu6811, 0 },
  { "inc", OP_IY | OP_PAGE2,       3, 0x6c,  7,  7,  CHG_NZV, cpu6811, 0 },
  { "inc", OP_IND16,               3, 0x72,  4,  4,  CHG_NZV, cpu6812|cpu9s12x, 0 },
  { "inc", OP_IDX,                 2, 0x62,  3,  3,  CHG_NZV, cpu6812|cpu9s12x, 0 },
  { "inc", OP_IDX_1,               3, 0x62,  4,  4,  CHG_NZV, cpu6812|cpu9s12x, 0 },
  { "inc", OP_IDX_2,               4, 0x62,  5,  5,  CHG_NZV, cpu6812|cpu9s12x, 0 },
  { "inc", OP_D_IDX,               2, 0x62,  6,  6,  CHG_NZV, cpu6812|cpu9s12x, 0 },
  { "inc", OP_D_IDX_2,             4, 0x62,  6,  6,  CHG_NZV, cpu6812|cpu9s12x, 0 },

  { "inca", OP_NONE,               1, 0x4c,  2,  2,  CHG_NZV, cpu6811, 0 },
  { "inca", OP_NONE,               1, 0x42,  1,  1,  CHG_NZV, cpu6812|cpu9s12x, 0 },
  { "incb", OP_NONE,               1, 0x5c,  2,  2,  CHG_NZV, cpu6811, 0 },
  { "incb", OP_NONE,               1, 0x52,  1,  1,  CHG_NZV, cpu6812|cpu9s12x, 0 },

  { "incw",  OP_IND16 | OP_PAGE2,          3, 0x72,  4,  4,  CHG_NZV, cpu9s12x, 0 },
  { "incw",  OP_IDX | OP_PAGE2,            2, 0x62,  3,  3,  CHG_NZV, cpu9s12x, 0 },
  { "incw",  OP_IDX_1 | OP_PAGE2,          3, 0x62,  4,  4,  CHG_NZV, cpu9s12x, 0 },
  { "incw",  OP_IDX_2 | OP_PAGE2,          4, 0x62,  5,  5,  CHG_NZV, cpu9s12x, 0 },
  { "incw",  OP_D_IDX | OP_PAGE2,          2, 0x62,  6,  6,  CHG_NZV, cpu9s12x, 0 },
  { "incw",  OP_D_IDX_2 | OP_PAGE2,        4, 0x62,  6,  6,  CHG_NZV, cpu9s12x, 0 },

  { "incx",  OP_NONE | OP_PAGE2,          3, 0x42,  4,  4,  CHG_NZV, cpu9s12x, 0 },

  { "incy",  OP_NONE | OP_PAGE2,          3, 0x52,  4,  4,  CHG_NZV, cpu9s12x, 0 },

  { "ins",  OP_NONE,               1, 0x31,  3,  3,  CHG_NONE, cpu6811, 0 },

  { "inx",  OP_NONE,               1, 0x08,  1,  1,  CHG_Z, cpu6811|cpu6812|cpu9s12x, 0 },
  { "iny",  OP_NONE |OP_PAGE2,     2, 0x08,  4,  4,  CHG_Z, cpu6811, 0 },
  { "iny",  OP_NONE,               1, 0x02,  1,  1,  CHG_Z, cpu6812|cpu9s12x, 0 },

  { "jmp",  OP_IND16 | OP_BRANCH,  3, 0x7e,  3,  3,  CHG_NONE, cpu6811, 0 },
  { "jmp",  OP_IX,                 2, 0x6e,  3,  3,  CHG_NONE, cpu6811, 0 },
  { "jmp",  OP_IY | OP_PAGE2,      3, 0x6e,  4,  4,  CHG_NONE, cpu6811, 0 },
  { "jmp",  OP_IND16 | OP_BRANCH,  3, 0x06,  3,  3,  CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "jmp",  OP_IDX,                2, 0x05,  3,  3,  CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "jmp",  OP_IDX_1,              3, 0x05,  3,  3,  CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "jmp",  OP_IDX_2,              4, 0x05,  4,  4,  CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "jmp",  OP_D_IDX,              2, 0x05,  6,  6,  CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "jmp",  OP_D_IDX_2,            4, 0x05,  6,  6,  CHG_NONE, cpu6812|cpu9s12x, 0 },

  { "jsr",  OP_DIRECT | OP_BRANCH, 2, 0x9d,  5,  5,  CHG_NONE, cpu6811, 0 },
  { "jsr",  OP_IND16 | OP_BRANCH,  3, 0xbd,  6,  6,  CHG_NONE, cpu6811, 0 },
  { "jsr",  OP_IX,                 2, 0xad,  6,  6,  CHG_NONE, cpu6811, 0 },
  { "jsr",  OP_IY | OP_PAGE2,      3, 0xad,  6,  6,  CHG_NONE, cpu6811, 0 },
  { "jsr",  OP_DIRECT | OP_BRANCH, 2, 0x17,  4,  4,  CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "jsr",  OP_IND16 | OP_BRANCH,  3, 0x16,  4,  3,  CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "jsr",  OP_IDX,                2, 0x15,  4,  4,  CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "jsr",  OP_IDX_1,              3, 0x15,  4,  4,  CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "jsr",  OP_IDX_2,              4, 0x15,  5,  5,  CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "jsr",  OP_D_IDX,              2, 0x15,  7,  7,  CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "jsr",  OP_D_IDX_2,            4, 0x15,  7,  7,  CHG_NONE, cpu6812|cpu9s12x, 0 },

  { "lbcc", OP_JUMP_REL16 | OP_PAGE2,  4, 0x24,  3,  4, CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "lbcs", OP_JUMP_REL16 | OP_PAGE2,  4, 0x25,  3,  4, CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "lbeq", OP_JUMP_REL16 | OP_PAGE2,  4, 0x27,  3,  4, CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "lbge", OP_JUMP_REL16 | OP_PAGE2,  4, 0x2c,  3,  4, CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "lbgt", OP_JUMP_REL16 | OP_PAGE2,  4, 0x2e,  3,  4, CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "lbhi", OP_JUMP_REL16 | OP_PAGE2,  4, 0x22,  3,  4, CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "lbhs", OP_JUMP_REL16 | OP_PAGE2,  4, 0x24,  3,  4, CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "lble", OP_JUMP_REL16 | OP_PAGE2,  4, 0x2f,  3,  4, CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "lblo", OP_JUMP_REL16 | OP_PAGE2,  4, 0x25,  3,  4, CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "lbls", OP_JUMP_REL16 | OP_PAGE2,  4, 0x23,  3,  4, CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "lblt", OP_JUMP_REL16 | OP_PAGE2,  4, 0x2d,  3,  4, CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "lbmi", OP_JUMP_REL16 | OP_PAGE2,  4, 0x2b,  3,  4, CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "lbne", OP_JUMP_REL16 | OP_PAGE2,  4, 0x26,  3,  4, CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "lbpl", OP_JUMP_REL16 | OP_PAGE2,  4, 0x2a,  3,  4, CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "lbra", OP_JUMP_REL16 | OP_PAGE2,  4, 0x20,  4,  4, CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "lbrn", OP_JUMP_REL16 | OP_PAGE2,  4, 0x21,  3,  3, CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "lbvc", OP_JUMP_REL16 | OP_PAGE2,  4, 0x28,  3,  4, CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "lbvs", OP_JUMP_REL16 | OP_PAGE2,  4, 0x29,  3,  4, CHG_NONE, cpu6812|cpu9s12x, 0 },

  { "ldaa", OP_IMM8,         2, 0x86,  1,  1,  CLR_V_CHG_NZ, cpu6811|cpu6812|cpu9s12x, 0 },
  { "ldaa", OP_DIRECT,       2, 0x96,  3,  3,  CLR_V_CHG_NZ, cpu6811|cpu6812|cpu9s12x, 0 },
  { "ldaa", OP_IND16,        3, 0xb6,  3,  3,  CLR_V_CHG_NZ, cpu6811|cpu6812|cpu9s12x, 0 },
  { "ldaa", OP_IX,             2, 0xa6,  4,  4,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "ldaa", OP_IY | OP_PAGE2,  3, 0xa6,  5,  5,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "ldaa", OP_IDX,            2, 0xa6,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "ldaa", OP_IDX_1,          3, 0xa6,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "ldaa", OP_IDX_2,          4, 0xa6,  4,  4,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "ldaa", OP_D_IDX,          2, 0xa6,  6,  6,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "ldaa", OP_D_IDX_2,        4, 0xa6,  6,  6,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },

  { "ldab", OP_IMM8,         2, 0xc6,  1,  1,  CLR_V_CHG_NZ, cpu6811|cpu6812|cpu9s12x, 0 },
  { "ldab", OP_DIRECT,       2, 0xd6,  3,  3,  CLR_V_CHG_NZ, cpu6811|cpu6812|cpu9s12x, 0 },
  { "ldab", OP_IND16,        3, 0xf6,  3,  3,  CLR_V_CHG_NZ, cpu6811|cpu6812|cpu9s12x, 0 },
  { "ldab", OP_IX,             2, 0xe6,  4,  4,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "ldab", OP_IY | OP_PAGE2,  3, 0xe6,  5,  5,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "ldab", OP_IDX,            2, 0xe6,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "ldab", OP_IDX_1,          3, 0xe6,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "ldab", OP_IDX_2,          4, 0xe6,  4,  4,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "ldab", OP_D_IDX,          2, 0xe6,  6,  6,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "ldab", OP_D_IDX_2,        4, 0xe6,  6,  6,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },

  { "ldd", OP_IMM16,         3, 0xcc,  2,  2,  CLR_V_CHG_NZ, cpu6811|cpu6812|cpu9s12x, 0 },
  { "ldd", OP_DIRECT,        2, 0xdc,  3,  3,  CLR_V_CHG_NZ, cpu6811|cpu6812|cpu9s12x, 0 },
  { "ldd", OP_IND16,         3, 0xfc,  3,  3,  CLR_V_CHG_NZ, cpu6811|cpu6812|cpu9s12x, 0 },
  { "ldd", OP_IX,              2, 0xec,  5,  5,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "ldd", OP_IY | OP_PAGE2,   3, 0xec,  6,  6,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "ldd", OP_IDX,             2, 0xec,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "ldd", OP_IDX_1,           3, 0xec,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "ldd", OP_IDX_2,           4, 0xec,  4,  4,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "ldd", OP_D_IDX,           2, 0xec,  6,  6,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "ldd", OP_D_IDX_2,         4, 0xec,  6,  6,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },

  { "lds",  OP_IMM16,          3, 0x8e,  3,  3,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "lds",  OP_DIRECT,         2, 0x9e,  4,  4,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "lds",  OP_IND16,          3, 0xbe,  5,  5,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "lds",  OP_IX,             2, 0xae,  5,  5,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "lds",  OP_IY | OP_PAGE2,  3, 0xae,  6,  6,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "lds",  OP_IMM16,          3, 0xcf,  2,  2,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "lds",  OP_DIRECT,         2, 0xdf,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "lds",  OP_IND16,          3, 0xff,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "lds",  OP_IDX,            2, 0xef,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "lds",  OP_IDX_1,          3, 0xef,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "lds",  OP_IDX_2,          4, 0xef,  4,  4,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "lds",  OP_D_IDX,          2, 0xef,  6,  6,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "lds",  OP_D_IDX_2,        4, 0xef,  6,  6,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },

  { "ldx",  OP_IMM16,        3, 0xce,  2,  2,  CLR_V_CHG_NZ, cpu6811|cpu6812|cpu9s12x, 0 },
  { "ldx",  OP_DIRECT,       2, 0xde,  3,  3,  CLR_V_CHG_NZ, cpu6811|cpu6812|cpu9s12x, 0 },
  { "ldx",  OP_IND16,        3, 0xfe,  3,  3,  CLR_V_CHG_NZ, cpu6811|cpu6812|cpu9s12x, 0 },
  { "ldx",  OP_IX,             2, 0xee,  5,  5,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "ldx",  OP_IY | OP_PAGE4,  3, 0xee,  6,  6,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "ldx",  OP_IDX,            2, 0xee,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "ldx",  OP_IDX_1,          3, 0xee,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "ldx",  OP_IDX_2,          4, 0xee,  4,  4,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "ldx",  OP_D_IDX,          2, 0xee,  6,  6,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "ldx",  OP_D_IDX_2,        4, 0xee,  6,  6,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },

  { "ldy",  OP_IMM16 | OP_PAGE2,  4, 0xce, 4, 4, CLR_V_CHG_NZ, cpu6811, 0 },
  { "ldy",  OP_DIRECT | OP_PAGE2, 3, 0xde, 5, 5, CLR_V_CHG_NZ, cpu6811, 0 },
  { "ldy",  OP_IND16 | OP_PAGE2,  4, 0xfe, 6, 6, CLR_V_CHG_NZ, cpu6811, 0 },
  { "ldy",  OP_IX | OP_PAGE3,     3, 0xee, 6, 6, CLR_V_CHG_NZ, cpu6811, 0 },
  { "ldy",  OP_IY | OP_PAGE2,     3, 0xee, 6, 6, CLR_V_CHG_NZ, cpu6811, 0 },
  { "ldy",  OP_IMM16,          3, 0xcd,  2,  2,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "ldy",  OP_DIRECT,         2, 0xdd,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "ldy",  OP_IND16,          3, 0xfd,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "ldy",  OP_IDX,            2, 0xed,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "ldy",  OP_IDX_1,          3, 0xed,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "ldy",  OP_IDX_2,          4, 0xed,  4,  4,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "ldy",  OP_D_IDX,          2, 0xed,  6,  6,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "ldy",  OP_D_IDX_2,        4, 0xed,  6,  6,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },

  { "leas", OP_IDX,            2, 0x1b,  2,  2,  CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "leas", OP_IDX_1,          3, 0x1b,  2,  2,  CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "leas", OP_IDX_2,          4, 0x1b,  2,  2,  CHG_NONE, cpu6812|cpu9s12x, 0 },

  { "leax", OP_IDX,            2, 0x1a,  2,  2,  CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "leax", OP_IDX_1,          3, 0x1a,  2,  2,  CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "leax", OP_IDX_2,          4, 0x1a,  2,  2,  CHG_NONE, cpu6812|cpu9s12x, 0 },

  { "leay", OP_IDX,            2, 0x19,  2,  2,  CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "leay", OP_IDX_1,          3, 0x19,  2,  2,  CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "leay", OP_IDX_2,          4, 0x19,  2,  2,  CHG_NONE, cpu6812|cpu9s12x, 0 },

  { "lsl",  OP_IND16,          3, 0x78,  4,  4,  CHG_NZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "lsl",  OP_IX,             2, 0x68,  6,  6,  CHG_NZVC, cpu6811, 0 },
  { "lsl",  OP_IY | OP_PAGE2,  3, 0x68,  7,  7,  CHG_NZVC, cpu6811, 0 },
  { "lsl",  OP_IDX,            2, 0x68,  3,  3,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "lsl",  OP_IDX_1,          3, 0x68,  4,  4,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "lsl",  OP_IDX_2,          4, 0x68,  5,  5,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "lsl",  OP_D_IDX,          2, 0x68,  6,  6,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "lsl",  OP_D_IDX_2,        4, 0x68,  6,  6,  CHG_NZVC, cpu6812|cpu9s12x, 0 },

  { "lsla", OP_NONE,           1, 0x48,  1,  1,  CHG_NZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "lslb", OP_NONE,           1, 0x58,  1,  1,  CHG_NZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "lsld", OP_NONE,           1, 0x05,  3,  3,  CHG_NZVC, cpu6811, 0 },
  { "lsld", OP_NONE,           1, 0x59,  1,  1,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
/* lslw is the same as aslw. */
  { "lslw",  OP_IND16 | OP_PAGE2,          3, 0x78,  4,  4,  CHG_NZVC, cpu9s12x, 0 },
  { "lslw",  OP_IDX | OP_PAGE2,            2, 0x68,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "lslw",  OP_IDX_1 | OP_PAGE2,          3, 0x68,  4,  4,  CHG_NZVC, cpu9s12x, 0 },
  { "lslw",  OP_IDX_2 | OP_PAGE2,          4, 0x68,  5,  5,  CHG_NZVC, cpu9s12x, 0 },
  { "lslw",  OP_D_IDX | OP_PAGE2,          2, 0x68,  6,  6,  CHG_NZVC, cpu9s12x, 0 },
  { "lslw",  OP_D_IDX_2 | OP_PAGE2,        4, 0x68,  6,  6,  CHG_NZVC, cpu9s12x, 0 },
/* lslx is same as aslx. */
  { "lslx", OP_NONE | OP_PAGE2,           1, 0x48,  1,  1,  CHG_NZVC, cpu9s12x, 0 },
/* lsly is the same as asly. */
  { "lsly", OP_NONE | OP_PAGE2,           1, 0x58,  1,  1,  CHG_NZVC, cpu9s12x, 0 },

  { "lsr",  OP_IND16,        3, 0x74,  4,  4,  CLR_N_CHG_ZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "lsr",  OP_IX,             2, 0x64,  6,  6,  CLR_N_CHG_ZVC, cpu6811, 0 },
  { "lsr",  OP_IY | OP_PAGE2,  3, 0x64,  7,  7,  CLR_V_CHG_ZVC, cpu6811, 0 },
  { "lsr",  OP_IDX,            2, 0x64,  3,  3,  CLR_N_CHG_ZVC, cpu6812|cpu9s12x, 0 },
  { "lsr",  OP_IDX_1,          3, 0x64,  4,  4,  CLR_N_CHG_ZVC, cpu6812|cpu9s12x, 0 },
  { "lsr",  OP_IDX_2,          4, 0x64,  5,  5,  CLR_N_CHG_ZVC, cpu6812|cpu9s12x, 0 },
  { "lsr",  OP_D_IDX,          2, 0x64,  6,  6,  CLR_N_CHG_ZVC, cpu6812|cpu9s12x, 0 },
  { "lsr",  OP_D_IDX_2,        4, 0x64,  6,  6,  CLR_N_CHG_ZVC, cpu6812|cpu9s12x, 0 },

  { "lsra", OP_NONE,         1, 0x44,  1,  1,  CLR_N_CHG_ZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "lsrb", OP_NONE,         1, 0x54,  1,  1,  CLR_N_CHG_ZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "lsrd", OP_NONE,           1, 0x04,  3,  3,  CLR_N_CHG_ZVC, cpu6811, 0 },
  { "lsrd", OP_NONE,           1, 0x49,  1,  1,  CLR_N_CHG_ZVC, cpu6812|cpu9s12x, 0 },

  { "lsrw",  OP_IND16 | OP_PAGE2,          3, 0x74,  4,  4,  CHG_NZVC, cpu9s12x, 0 },
  { "lsrw",  OP_IDX | OP_PAGE2,            2, 0x64,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "lsrw",  OP_IDX_1 | OP_PAGE2,          3, 0x64,  4,  4,  CHG_NZVC, cpu9s12x, 0 },
  { "lsrw",  OP_IDX_2 | OP_PAGE2,          4, 0x64,  5,  5,  CHG_NZVC, cpu9s12x, 0 },
  { "lsrw",  OP_D_IDX | OP_PAGE2,          2, 0x64,  6,  6,  CHG_NZVC, cpu9s12x, 0 },
  { "lsrw",  OP_D_IDX_2 | OP_PAGE2,        4, 0x64,  6,  6,  CHG_NZVC, cpu9s12x, 0 },

  { "lsrx", OP_NONE | OP_PAGE2,           1, 0x44,  1,  1,  CHG_NZVC, cpu9s12x, 0 },

  { "lsry", OP_NONE | OP_PAGE2,           1, 0x54,  1,  1,  CHG_NZVC, cpu9s12x, 0 },

  { "maxa", OP_IDX | OP_PAGE2,     3, 0x18,  4,  4,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "maxa", OP_IDX_1 | OP_PAGE2,   4, 0x18,  4,  4,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "maxa", OP_IDX_2 | OP_PAGE2,   5, 0x18,  5,  5,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "maxa", OP_D_IDX | OP_PAGE2,   3, 0x18,  7,  7,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "maxa", OP_D_IDX_2 | OP_PAGE2, 5, 0x18,  7,  7,  CHG_NZVC, cpu6812|cpu9s12x, 0 },

  { "maxm", OP_IDX | OP_PAGE2,     3, 0x1c,  4,  4,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "maxm", OP_IDX_1 | OP_PAGE2,   4, 0x1c,  5,  5,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "maxm", OP_IDX_2 | OP_PAGE2,   5, 0x1c,  6,  6,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "maxm", OP_D_IDX | OP_PAGE2,   3, 0x1c,  7,  7,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "maxm", OP_D_IDX_2 | OP_PAGE2, 5, 0x1c,  7,  7,  CHG_NZVC, cpu6812|cpu9s12x, 0 },

  { "mem",  OP_NONE,                1, 0x01,  5,  5,  CHG_HNZVC, cpu6812|cpu9s12x, 0 },

  { "mina", OP_IDX | OP_PAGE2,     3, 0x19,  4,  4,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "mina", OP_IDX_1 | OP_PAGE2,   4, 0x19,  4,  4,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "mina", OP_IDX_2 | OP_PAGE2,   5, 0x19,  5,  5,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "mina", OP_D_IDX | OP_PAGE2,   3, 0x19,  7,  7,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "mina", OP_D_IDX_2 | OP_PAGE2, 5, 0x19,  7,  7,  CHG_NZVC, cpu6812|cpu9s12x, 0 },

  { "minm", OP_IDX | OP_PAGE2,     3, 0x1d,  4,  4,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "minm", OP_IDX_1 | OP_PAGE2,   4, 0x1d,  5,  5,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "minm", OP_IDX_2 | OP_PAGE2,   5, 0x1d,  6,  6,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "minm", OP_D_IDX | OP_PAGE2,   3, 0x1d,  7,  7,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "minm", OP_D_IDX_2 | OP_PAGE2, 5, 0x1d,  7,  7,  CHG_NZVC, cpu6812|cpu9s12x, 0 },

/* The S12X additional modes are implemented, but uncommenting here causes a problem */
  { "movb", OP_IMM8|OP_IND16_p2|OP_PAGE2,       5, 0x0b,  4,  4,  CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "movb", OP_IMM8|OP_IDX_p2|OP_PAGE2,         4, 0x08,  4,  4,  CHG_NONE, cpu6812|cpu9s12x, 0 },
/*  { "movb", OP_IMM8|OP_IDX1_p2|OP_PAGE2,        5, 0x08,  4,  4,  CHG_NONE, cpu9s12x, 0 },
  { "movb", OP_IMM8|OP_IDX2_p2|OP_PAGE2,        4, 0x08,  4,  4,  CHG_NONE, cpu9s12x, 0 },
  { "movb", OP_IMM8|OP_D_IDX|OP_PAGE2,       5, 0x08,  4,  4,  CHG_NONE, cpu9s12x, 0 },
  { "movb", OP_IMM8|OP_D_IDX2_p2|OP_PAGE2,      4, 0x08,  4,  4,  CHG_NONE, cpu9s12x, 0 },*/

  { "movb", OP_IND16|OP_IND16_p2|OP_PAGE2,      6, 0x0c,  6,  6,  CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "movb", OP_IND16|OP_IDX_p2|OP_PAGE2,        5, 0x09,  5,  5,  CHG_NONE, cpu6812|cpu9s12x, 0 },
/*  { "movb", OP_IND16|OP_IDX1_p2|OP_PAGE2,       6, 0x09,  6,  6,  CHG_NONE, cpu9s12x, 0 },
  { "movb", OP_IND16|OP_IDX2_p2|OP_PAGE2,       5, 0x09,  5,  5,  CHG_NONE, cpu9s12x, 0 },
  { "movb", OP_IND16|OP_D_IDX_p2|OP_PAGE2,    6, 0x09,  6,  6,  CHG_NONE, cpu9s12x, 0 },
  { "movb", OP_IND16|OP_D_IDX2_p2|OP_PAGE2,     5, 0x09,  5,  5,  CHG_NONE, cpu9s12x, 0 }, */

  { "movb", OP_IDX|OP_IND16_p2|OP_PAGE2,        5, 0x0d,  5,  5,  CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "movb", OP_IDX|OP_IDX_p2|OP_PAGE2,          4, 0x0a,  5,  5,  CHG_NONE, cpu6812|cpu9s12x, 0 },
/*  { "movb", OP_IDX|OP_IDX1_p2|OP_PAGE2,         5, 0x0a,  5,  5,  CHG_NONE, cpu9s12x, 0 },
  { "movb", OP_IDX|OP_IDX2_p2|OP_PAGE2,         4, 0x0a,  5,  5,  CHG_NONE, cpu9s12x, 0 },
  { "movb", OP_IDX|OP_D_IDX_p2|OP_PAGE2,        5, 0x0d,  5,  5,  CHG_NONE, cpu9s12x, 0 },
  { "movb", OP_IDX|OP_D_IDX2_p2|OP_PAGE2,       4, 0x0a,  5,  5,  CHG_NONE, cpu9s12x, 0 },

  { "movb", OP_IDX_1|OP_IND16_p2|OP_PAGE2,       5, 0x0d,  5,  5,  CHG_NONE, cpu9s12x, 0 },
  { "movb", OP_IDX_1|OP_IDX_p2|OP_PAGE2,         4, 0x0a,  5,  5,  CHG_NONE, cpu9s12x, 0 },
  { "movb", OP_IDX_1|OP_IDX1_p2|OP_PAGE2,        6, 0x0a,  6,  6,  CHG_NONE, cpu9s12x, 0 },
  { "movb", OP_IDX_1|OP_IDX2_p2|OP_PAGE2,        5, 0x0a,  5,  5,  CHG_NONE, cpu9s12x, 0 },
  { "movb", OP_IDX_1|OP_D_IDX_p2|OP_PAGE2,       6, 0x0a,  6,  6,  CHG_NONE, cpu9s12x, 0 },
  { "movb", OP_IDX_1|OP_D_IDX2_p2|OP_PAGE2,      5, 0x0a,  5,  5,  CHG_NONE, cpu9s12x, 0 },

  { "movb", OP_IDX_2|OP_IND16_p2|OP_PAGE2,       5, 0x0d,  5,  5,  CHG_NONE, cpu9s12x, 0 },
  { "movb", OP_IDX_2|OP_IDX_p2|OP_PAGE2,         4, 0x0a,  5,  5,  CHG_NONE, cpu9s12x, 0 },
  { "movb", OP_IDX_2|OP_IDX1_p2|OP_PAGE2,        6, 0x0a,  6,  6,  CHG_NONE, cpu9s12x, 0 },
  { "movb", OP_IDX_2|OP_IDX2_p2|OP_PAGE2,        5, 0x0a,  5,  5,  CHG_NONE, cpu9s12x, 0 },
  { "movb", OP_IDX_2|OP_D_IDX_p2|OP_PAGE2,       6, 0x0a,  6,  6,  CHG_NONE, cpu9s12x, 0 },
  { "movb", OP_IDX_2|OP_D_IDX2_p2|OP_PAGE2,      5, 0x0a,  5,  5,  CHG_NONE, cpu9s12x, 0 },

  { "movb", OP_D_IDX|OP_IND16_p2|OP_PAGE2,       5, 0x0d,  5,  5,  CHG_NONE, cpu9s12x, 0 },
  { "movb", OP_D_IDX|OP_IDX_p2|OP_PAGE2,         4, 0x0a,  5,  5,  CHG_NONE, cpu9s12x, 0 },
  { "movb", OP_D_IDX|OP_IDX1_p2|OP_PAGE2,        6, 0x0a,  6,  6,  CHG_NONE, cpu9s12x, 0 },
  { "movb", OP_D_IDX|OP_IDX2_p2|OP_PAGE2,        5, 0x0a,  5,  5,  CHG_NONE, cpu9s12x, 0 },
  { "movb", OP_D_IDX|OP_D_IDX_p2|OP_PAGE2,       6, 0x0a,  6,  6,  CHG_NONE, cpu9s12x, 0 },
  { "movb", OP_D_IDX|OP_D_IDX2_p2|OP_PAGE2,      5, 0x0a,  5,  5,  CHG_NONE, cpu9s12x, 0 },

  { "movb", OP_D_IDX_2|OP_IND16_p2|OP_PAGE2,       5, 0x0d,  5,  5,  CHG_NONE, cpu9s12x, 0 },
  { "movb", OP_D_IDX_2|OP_IDX_p2|OP_PAGE2,         4, 0x0a,  5,  5,  CHG_NONE, cpu9s12x, 0 },
  { "movb", OP_D_IDX_2|OP_IDX1_p2|OP_PAGE2,        6, 0x0a,  6,  6,  CHG_NONE, cpu9s12x, 0 },
  { "movb", OP_D_IDX_2|OP_IDX2_p2|OP_PAGE2,        5, 0x0a,  5,  5,  CHG_NONE, cpu9s12x, 0 },
  { "movb", OP_D_IDX_2|OP_D_IDX_p2|OP_PAGE2,       6, 0x0a,  6,  6,  CHG_NONE, cpu9s12x, 0 },
  { "movb", OP_D_IDX_2|OP_D_IDX2_p2|OP_PAGE2,      5, 0x0a,  5,  5,  CHG_NONE, cpu9s12x, 0 },*/

  { "movw", OP_IMM16 | OP_IND16_p2 | OP_PAGE2,  6, 0x03,  5,  5,  CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "movw", OP_IMM16 | OP_IDX_p2 | OP_PAGE2,    5, 0x00,  4,  4,  CHG_NONE, cpu6812|cpu9s12x, 0 },
/*  { "movw", OP_IMM16|OP_IDX1_p2|OP_PAGE2,        5, 0x00,  4,  4,  CHG_NONE, cpu9s12x, 0 },
  { "movw", OP_IMM16|OP_IDX2_p2|OP_PAGE2,        4, 0x00,  4,  4,  CHG_NONE, cpu9s12x, 0 },
  { "movw", OP_IMM16|OP_D_IDX_p2|OP_PAGE2,       5, 0x00,  4,  4,  CHG_NONE, cpu9s12x, 0 },
  { "movw", OP_IMM16|OP_D_IDX2_p2|OP_PAGE2,      4, 0x00,  4,  4,  CHG_NONE, cpu9s12x, 0 },*/

  { "movw", OP_IND16 | OP_IND16_p2 | OP_PAGE2,  6, 0x04,  6,  6,  CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "movw", OP_IND16 | OP_IDX_p2 | OP_PAGE2,    5, 0x01,  5,  5,  CHG_NONE, cpu6812|cpu9s12x, 0 },
/*  { "movw", OP_IND16|OP_IDX1_p2|OP_PAGE2,       6, 0x01,  6,  6,  CHG_NONE, cpu9s12x, 0 },
  { "movw", OP_IND16|OP_IDX2_p2|OP_PAGE2,       5, 0x01,  5,  5,  CHG_NONE, cpu9s12x, 0 },
  { "movw", OP_IND16|OP_D_IDX_p2|OP_PAGE2,      6, 0x01,  6,  6,  CHG_NONE, cpu9s12x, 0 },
  { "movw", OP_IND16|OP_D_IDX2_p2|OP_PAGE2,     5, 0x01,  5,  5,  CHG_NONE, cpu9s12x, 0 },*/

  { "movw", OP_IDX | OP_IND16_p2 | OP_PAGE2,    5, 0x05,  5,  5,  CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "movw", OP_IDX | OP_IDX_p2 | OP_PAGE2,      4, 0x02,  5,  5,  CHG_NONE, cpu6812|cpu9s12x, 0 },
/*  { "movw", OP_IDX|OP_IDX1_p2|OP_PAGE2,         5, 0x02,  5,  5,  CHG_NONE, cpu9s12x, 0 },
  { "movw", OP_IDX|OP_IDX2_p2|OP_PAGE2,         4, 0x02,  5,  5,  CHG_NONE, cpu9s12x, 0 },
  { "movw", OP_IDX|OP_D_IDX_p2|OP_PAGE2,        5, 0x02,  5,  5,  CHG_NONE, cpu9s12x, 0 },
  { "movw", OP_IDX|OP_D_IDX2_p2|OP_PAGE2,       4, 0x02,  5,  5,  CHG_NONE, cpu9s12x, 0 },

  { "movw", OP_IDX_1|OP_IND16_p2|OP_PAGE2,       5, 0x05,  5,  5,  CHG_NONE, cpu9s12x, 0 },
  { "movw", OP_IDX_1|OP_IDX_p2|OP_PAGE2,         4, 0x02,  5,  5,  CHG_NONE, cpu9s12x, 0 },
  { "movw", OP_IDX_1|OP_IDX1_p2|OP_PAGE2,        6, 0x02,  6,  6,  CHG_NONE, cpu9s12x, 0 },
  { "movw", OP_IDX_1|OP_IDX2_p2|OP_PAGE2,        5, 0x02,  5,  5,  CHG_NONE, cpu9s12x, 0 },
  { "movw", OP_IDX_1|OP_D_IDX_p2|OP_PAGE2,       6, 0x02,  6,  6,  CHG_NONE, cpu9s12x, 0 },
  { "movw", OP_IDX_1|OP_D_IDX2_p2|OP_PAGE2,      5, 0x02,  5,  5,  CHG_NONE, cpu9s12x, 0 },

  { "movw", OP_IDX_2|OP_IND16_p2|OP_PAGE2,       5, 0x05,  5,  5,  CHG_NONE, cpu9s12x, 0 },
  { "movw", OP_IDX_2|OP_IDX_p2|OP_PAGE2,         4, 0x02,  5,  5,  CHG_NONE, cpu9s12x, 0 },
  { "movw", OP_IDX_2|OP_IDX1_p2|OP_PAGE2,        6, 0x02,  6,  6,  CHG_NONE, cpu9s12x, 0 },
  { "movw", OP_IDX_2|OP_IDX2_p2|OP_PAGE2,        5, 0x02,  5,  5,  CHG_NONE, cpu9s12x, 0 },
  { "movw", OP_IDX_2|OP_D_IDX_p2|OP_PAGE2,       6, 0x02,  6,  6,  CHG_NONE, cpu9s12x, 0 },
  { "movw", OP_IDX_2|OP_D_IDX2_p2|OP_PAGE2,      5, 0x02,  5,  5,  CHG_NONE, cpu9s12x, 0 },

  { "movw", OP_D_IDX|OP_IND16_p2|OP_PAGE2,       5, 0x05,  5,  5,  CHG_NONE, cpu9s12x, 0 },
  { "movw", OP_D_IDX|OP_IDX_p2|OP_PAGE2,         4, 0x02,  5,  5,  CHG_NONE, cpu9s12x, 0 },
  { "movw", OP_D_IDX|OP_IDX1_p2|OP_PAGE2,        6, 0x02,  6,  6,  CHG_NONE, cpu9s12x, 0 },
  { "movw", OP_D_IDX|OP_IDX2_p2|OP_PAGE2,        5, 0x02,  5,  5,  CHG_NONE, cpu9s12x, 0 },
  { "movw", OP_D_IDX|OP_D_IDX_p2|OP_PAGE2,       6, 0x02,  6,  6,  CHG_NONE, cpu9s12x, 0 },
  { "movw", OP_D_IDX|OP_D_IDX2_p2|OP_PAGE2,      5, 0x02,  5,  5,  CHG_NONE, cpu9s12x, 0 },

  { "movw", OP_D_IDX_2|OP_IND16_p2|OP_PAGE2,       5, 0x05,  5,  5,  CHG_NONE, cpu9s12x, 0 },
  { "movw", OP_D_IDX_2|OP_IDX_p2|OP_PAGE2,         4, 0x02,  5,  5,  CHG_NONE, cpu9s12x, 0 },
  { "movw", OP_D_IDX_2|OP_IDX1_p2|OP_PAGE2,        6, 0x02,  6,  6,  CHG_NONE, cpu9s12x, 0 },
  { "movw", OP_D_IDX_2|OP_IDX2_p2|OP_PAGE2,        5, 0x02,  5,  5,  CHG_NONE, cpu9s12x, 0 },
  { "movw", OP_D_IDX_2|OP_D_IDX_p2|OP_PAGE2,       6, 0x02,  6,  6,  CHG_NONE, cpu9s12x, 0 },
  { "movw", OP_D_IDX_2|OP_D_IDX2_p2|OP_PAGE2,      5, 0x02,  5,  5,  CHG_NONE, cpu9s12x, 0 },*/

  { "mul",  OP_NONE,           1, 0x3d,  3, 10,  CHG_C, cpu6811, 0 },
  { "mul",  OP_NONE,           1, 0x12,  3,  3,  CHG_C, cpu6812|cpu9s12x, 0 },

  { "neg",  OP_IND16,          3, 0x70,  4,  4,  CHG_NZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "neg",  OP_IX,             2, 0x60,  6,  6,  CHG_NZVC, cpu6811, 0 },
  { "neg",  OP_IY | OP_PAGE2,  3, 0x60,  7,  7,  CHG_NZVC, cpu6811, 0 },
  { "neg",  OP_IDX,            2, 0x60,  3,  3,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "neg",  OP_IDX_1,          3, 0x60,  4,  4,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "neg",  OP_IDX_2,          4, 0x60,  5,  5,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "neg",  OP_D_IDX,          2, 0x60,  6,  6,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "neg",  OP_D_IDX_2,        4, 0x60,  6,  6,  CHG_NZVC, cpu6812|cpu9s12x, 0 },

  { "nega", OP_NONE,           1, 0x40,  1,  1,  CHG_NZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "negb", OP_NONE,           1, 0x50,  1,  1,  CHG_NZVC, cpu6811|cpu6812|cpu9s12x, 0 },

  { "negw",  OP_IND16| OP_PAGE2,          3, 0x70,  4,  4,  CHG_NZVC, cpu9s12x, 0 },
  { "negw",  OP_IDX| OP_PAGE2,            2, 0x60,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "negw",  OP_IDX_1| OP_PAGE2,          3, 0x60,  4,  4,  CHG_NZVC, cpu9s12x, 0 },
  { "negw",  OP_IDX_2| OP_PAGE2,          4, 0x60,  5,  5,  CHG_NZVC, cpu9s12x, 0 },
  { "negw",  OP_D_IDX| OP_PAGE2,          2, 0x60,  6,  6,  CHG_NZVC, cpu9s12x, 0 },
  { "negw",  OP_D_IDX_2| OP_PAGE2,        4, 0x60,  6,  6,  CHG_NZVC, cpu9s12x, 0 },

  { "negx", OP_NONE| OP_PAGE2,           1, 0x40,  1,  1,  CHG_NZVC, cpu9s12x, 0 },

  { "negy", OP_NONE| OP_PAGE2,           1, 0x50,  1,  1,  CHG_NZVC, cpu9s12x, 0 },

  { "nop",  OP_NONE,           1, 0x01,  2,  2,  CHG_NONE, cpu6811, 0 },
  { "nop",  OP_NONE,           1, 0xa7,  1,  1,  CHG_NONE, cpu6812|cpu9s12x, 0 },

  { "oraa", OP_IMM8,         2, 0x8a,  1,  1,  CLR_V_CHG_NZ, cpu6811|cpu6812|cpu9s12x, 0 },
  { "oraa", OP_DIRECT,       2, 0x9a,  3,  3,  CLR_V_CHG_NZ, cpu6811|cpu6812|cpu9s12x, 0 },
  { "oraa", OP_IND16,        3, 0xba,  3,  3,  CLR_V_CHG_NZ, cpu6811|cpu6812|cpu9s12x, 0 },
  { "oraa", OP_IX,             2, 0xaa,  4,  4,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "oraa", OP_IY | OP_PAGE2,  3, 0xaa,  5,  5,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "oraa", OP_IDX,            2, 0xaa,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "oraa", OP_IDX_1,          3, 0xaa,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "oraa", OP_IDX_2,          4, 0xaa,  4,  4,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "oraa", OP_D_IDX,          2, 0xaa,  6,  6,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "oraa", OP_D_IDX_2,        4, 0xaa,  6,  6,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },

  { "orab", OP_IMM8,         2, 0xca,  1,  1,  CLR_V_CHG_NZ, cpu6811|cpu6812|cpu9s12x, 0 },
  { "orab", OP_DIRECT,       2, 0xda,  3,  3,  CLR_V_CHG_NZ, cpu6811|cpu6812|cpu9s12x, 0 },
  { "orab", OP_IND16,        3, 0xfa,  3,  3,  CLR_V_CHG_NZ, cpu6811|cpu6812|cpu9s12x, 0 },
  { "orab", OP_IX,             2, 0xea,  4,  4,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "orab", OP_IY | OP_PAGE2,  3, 0xea,  5,  5,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "orab", OP_IDX,            2, 0xea,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "orab", OP_IDX_1,          3, 0xea,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "orab", OP_IDX_2,          4, 0xea,  4,  4,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "orab", OP_D_IDX,          2, 0xea,  6,  6,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "orab", OP_D_IDX_2,        4, 0xea,  6,  6,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },

  { "orcc", OP_IMM8,           2, 0x14,  1,  1,  CHG_ALL, cpu6812|cpu9s12x, 0 },

  { "orx", OP_IMM16| OP_PAGE2,  2, 0x8a,  1,  1,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "orx", OP_DIRECT| OP_PAGE2, 2, 0x9a,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "orx", OP_IND16| OP_PAGE2,  3, 0xba,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "orx", OP_IDX| OP_PAGE2,    2, 0xaa,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "orx", OP_IDX_1| OP_PAGE2,  3, 0xaa,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "orx", OP_IDX_2| OP_PAGE2,  4, 0xaa,  4,  4,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "orx", OP_D_IDX| OP_PAGE2,  2, 0xaa,  6,  6,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "orx", OP_D_IDX_2| OP_PAGE2,4, 0xaa,  6,  6,  CLR_V_CHG_NZ, cpu9s12x, 0 },

  { "ory", OP_IMM16| OP_PAGE2,  2, 0xca,  1,  1,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "ory", OP_DIRECT| OP_PAGE2, 2, 0xda,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "ory", OP_IND16| OP_PAGE2,  3, 0xfa,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "ory", OP_IDX| OP_PAGE2,    2, 0xea,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "ory", OP_IDX_1| OP_PAGE2,  3, 0xea,  3,  3,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "ory", OP_IDX_2| OP_PAGE2,  4, 0xea,  4,  4,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "ory", OP_D_IDX| OP_PAGE2,  2, 0xea,  6,  6,  CLR_V_CHG_NZ, cpu9s12x, 0 },
  { "ory", OP_D_IDX_2| OP_PAGE2,4, 0xea,  6,  6,  CLR_V_CHG_NZ, cpu9s12x, 0 },

  { "psha", OP_NONE,           1, 0x36,  2,  2,  CHG_NONE, cpu6811|cpu6812|cpu9s12x, 0 },
  { "pshb", OP_NONE,           1, 0x37,  2,  2,  CHG_NONE, cpu6811|cpu6812|cpu9s12x, 0 },
  { "pshc", OP_NONE,           1, 0x39,  2,  2,  CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "pshcw", OP_NONE| OP_PAGE2,1, 0x39,  2,  2,  CHG_NONE, cpu9s12x, 0 },
  { "pshd", OP_NONE,           1, 0x3b,  2,  2,  CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "pshx", OP_NONE,           1, 0x3c,  4,  4,  CHG_NONE, cpu6811, 0 },
  { "pshx", OP_NONE,           1, 0x34,  2,  2,  CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "pshy", OP_NONE | OP_PAGE2,2, 0x3c,  5,  5,  CHG_NONE, cpu6811, 0 },
  { "pshy", OP_NONE,           1, 0x35,  2,  2,  CHG_NONE, cpu6812|cpu9s12x, 0 },

  { "pula", OP_NONE,           1, 0x32,  3,  3,  CHG_NONE, cpu6811|cpu6812|cpu9s12x, 0 },
  { "pulb", OP_NONE,           1, 0x33,  3,  3,  CHG_NONE, cpu6811|cpu6812|cpu9s12x, 0 },
  { "pulc", OP_NONE,           1, 0x38,  3,  3,  CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "pulcw", OP_NONE| OP_PAGE2,1, 0x38,  2,  2,  CHG_NONE, cpu9s12x, 0 },
  { "puld", OP_NONE,           1, 0x3a,  3,  3,  CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "pulx", OP_NONE,           1, 0x38,  5,  5,  CHG_NONE, cpu6811, 0 },
  { "pulx", OP_NONE,           1, 0x30,  3,  3,  CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "puly", OP_NONE | OP_PAGE2,2, 0x38,  6,  6,  CHG_NONE, cpu6811, 0 },
  { "puly", OP_NONE,           1, 0x31,  3,  3,  CHG_NONE, cpu6812|cpu9s12x, 0 },

  { "rev",  OP_NONE | OP_PAGE2, 2, 0x3a,  _M,  _M,  CHG_HNZVC, cpu6812|cpu9s12x, 0 },
  { "revw", OP_NONE | OP_PAGE2, 2, 0x3b,  _M,  _M,  CHG_HNZVC, cpu6812|cpu9s12x, 0 },

  { "rol",  OP_IND16,          3, 0x79,  6,  6,  CHG_NZVC, cpu6811, 0 },
  { "rol",  OP_IX,             2, 0x69,  6,  6,  CHG_NZVC, cpu6811, 0 },
  { "rol",  OP_IY | OP_PAGE2,  3, 0x69,  7,  7,  CHG_NZVC, cpu6811, 0 },
  { "rol",  OP_IND16,          3, 0x75,  4,  4,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "rol",  OP_IDX,            2, 0x65,  3,  3,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "rol",  OP_IDX_1,          3, 0x65,  4,  4,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "rol",  OP_IDX_2,          4, 0x65,  5,  5,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "rol",  OP_D_IDX,          2, 0x65,  6,  6,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "rol",  OP_D_IDX_2,        4, 0x65,  6,  6,  CHG_NZVC, cpu6812|cpu9s12x, 0 },

  { "rola", OP_NONE,           1, 0x49,  2,  2,  CHG_NZVC, cpu6811, 0 },
  { "rola", OP_NONE,           1, 0x45,  1,  1,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "rolb", OP_NONE,           1, 0x59,  2,  2,  CHG_NZVC, cpu6811, 0 },
  { "rolb", OP_NONE,           1, 0x55,  1,  1,  CHG_NZVC, cpu6812|cpu9s12x, 0 },

  { "rolw",  OP_IND16 | OP_PAGE2,          3, 0x75,  4,  4,  CHG_NZVC, cpu9s12x, 0 },
  { "rolw",  OP_IDX | OP_PAGE2,            2, 0x65,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "rolw",  OP_IDX_1 | OP_PAGE2,          3, 0x65,  4,  4,  CHG_NZVC, cpu9s12x, 0 },
  { "rolw",  OP_IDX_2 | OP_PAGE2,          4, 0x65,  5,  5,  CHG_NZVC, cpu9s12x, 0 },
  { "rolw",  OP_D_IDX | OP_PAGE2,          2, 0x65,  6,  6,  CHG_NZVC, cpu9s12x, 0 },
  { "rolw",  OP_D_IDX_2 | OP_PAGE2,        4, 0x65,  6,  6,  CHG_NZVC, cpu9s12x, 0 },

  { "rolx", OP_NONE | OP_PAGE2,           1, 0x45,  1,  1,  CHG_NZVC, cpu9s12x, 0 },
  { "roly", OP_NONE | OP_PAGE2,           1, 0x55,  1,  1,  CHG_NZVC, cpu9s12x, 0 },

  { "ror",  OP_IND16,          3, 0x76,  4,  4,  CHG_NZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "ror",  OP_IX,             2, 0x66,  6,  6,  CHG_NZVC, cpu6811, 0 },
  { "ror",  OP_IY | OP_PAGE2,  3, 0x66,  7,  7,  CHG_NZVC, cpu6811, 0 },
  { "ror",  OP_IDX,            2, 0x66,  3,  3,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "ror",  OP_IDX_1,          3, 0x66,  4,  4,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "ror",  OP_IDX_2,          4, 0x66,  5,  5,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "ror",  OP_D_IDX,          2, 0x66,  6,  6,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "ror",  OP_D_IDX_2,        4, 0x66,  6,  6,  CHG_NZVC, cpu6812|cpu9s12x, 0 },

  { "rora", OP_NONE,           1, 0x46,  1,  1,  CHG_NZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "rorb", OP_NONE,           1, 0x56,  1,  1,  CHG_NZVC, cpu6811|cpu6812|cpu9s12x, 0 },

  { "rorw",  OP_IND16 | OP_PAGE2,          3, 0x76,  4,  4,  CHG_NZVC, cpu9s12x, 0 },
  { "rorw",  OP_IDX | OP_PAGE2,            2, 0x66,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "rorw",  OP_IDX_1 | OP_PAGE2,          3, 0x66,  4,  4,  CHG_NZVC, cpu9s12x, 0 },
  { "rorw",  OP_IDX_2 | OP_PAGE2,          4, 0x66,  5,  5,  CHG_NZVC, cpu9s12x, 0 },
  { "rorw",  OP_D_IDX | OP_PAGE2,          2, 0x66,  6,  6,  CHG_NZVC, cpu9s12x, 0 },
  { "rorw",  OP_D_IDX_2 | OP_PAGE2,        4, 0x66,  6,  6,  CHG_NZVC, cpu9s12x, 0 },

  { "rorx", OP_NONE | OP_PAGE2,           1, 0x46,  1,  1,  CHG_NZVC, cpu9s12x, 0 },
  { "rory", OP_NONE | OP_PAGE2,           1, 0x56,  1,  1,  CHG_NZVC, cpu9s12x, 0 },

  { "rtc",  OP_NONE,           1, 0x0a,  6,  6,  CHG_NONE, cpu6812|cpu9s12x, 0 },
  { "rti",  OP_NONE,           1, 0x3b, 12, 12,  CHG_ALL, cpu6811, 0 },
  { "rti",  OP_NONE,           1, 0x0b,  8, 10,  CHG_ALL, cpu6812|cpu9s12x, 0 },
  { "rts",  OP_NONE,           1, 0x39,  5,  5,  CHG_NONE, cpu6811, 0 },
  { "rts",  OP_NONE,           1, 0x3d,  5,  5,  CHG_NONE, cpu6812|cpu9s12x, 0 },

  { "sba",  OP_NONE,             1, 0x10,  2,  2,  CHG_NZVC, cpu6811, 0 },
  { "sba",  OP_NONE | OP_PAGE2, 2, 0x16,  2,  2,  CHG_NZVC, cpu6812|cpu9s12x, 0 },

  { "sbca", OP_IMM8,           2, 0x82,  1,  1,  CHG_NZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "sbca", OP_DIRECT,         2, 0x92,  3,  3,  CHG_NZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "sbca", OP_IND16,          3, 0xb2,  3,  3,  CHG_NZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "sbca", OP_IX,             2, 0xa2,  4,  4,  CHG_NZVC, cpu6811, 0 },
  { "sbca", OP_IY | OP_PAGE2,  3, 0xa2,  5,  5,  CHG_NZVC, cpu6811, 0 },
  { "sbca", OP_IDX,            2, 0xa2,  3,  3,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "sbca", OP_IDX_1,          3, 0xa2,  3,  3,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "sbca", OP_IDX_2,          4, 0xa2,  4,  4,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "sbca", OP_D_IDX,          2, 0xa2,  6,  6,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "sbca", OP_D_IDX_2,        4, 0xa2,  6,  6,  CHG_NZVC, cpu6812|cpu9s12x, 0 },

  { "sbcb", OP_IMM8,           2, 0xc2,  1,  1,  CHG_NZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "sbcb", OP_DIRECT,         2, 0xd2,  3,  3,  CHG_NZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "sbcb", OP_IND16,          3, 0xf2,  3,  3,  CHG_NZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "sbcb", OP_IX,             2, 0xe2,  4,  4,  CHG_NZVC, cpu6811, 0 },
  { "sbcb", OP_IY | OP_PAGE2,  3, 0xe2,  5,  5,  CHG_NZVC, cpu6811, 0 },
  { "sbcb", OP_IDX,            2, 0xe2,  3,  3,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "sbcb", OP_IDX_1,          3, 0xe2,  3,  3,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "sbcb", OP_IDX_2,          4, 0xe2,  4,  4,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "sbcb", OP_D_IDX,          2, 0xe2,  6,  6,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "sbcb", OP_D_IDX_2,        4, 0xe2,  6,  6,  CHG_NZVC, cpu6812|cpu9s12x, 0 },

  { "sbed", OP_IMM16 | OP_PAGE2,          3, 0x83,  2,  2,  CHG_NZVC, cpu9s12x, 0 },
  { "sbed", OP_DIRECT | OP_PAGE2,         2, 0x93,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "sbed", OP_IND16 | OP_PAGE2,          3, 0xb3,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "sbed", OP_IDX | OP_PAGE2,            2, 0xa3,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "sbed", OP_IDX_1 | OP_PAGE2,          3, 0xa3,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "sbed", OP_IDX_2 | OP_PAGE2,          4, 0xa3,  4,  4,  CHG_NZVC, cpu9s12x, 0 },
  { "sbed", OP_D_IDX | OP_PAGE2,          2, 0xa3,  6,  6,  CHG_NZVC, cpu9s12x, 0 },
  { "sbed", OP_D_IDX_2 | OP_PAGE2,        4, 0xa3,  6,  6,  CHG_NZVC, cpu9s12x, 0 },

  { "sbex", OP_IMM16 | OP_PAGE2,          3, 0x82,  2,  2,  CHG_NZVC, cpu9s12x, 0 },
  { "sbex", OP_DIRECT | OP_PAGE2,         2, 0x92,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "sbex", OP_IND16 | OP_PAGE2,          3, 0xb2,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "sbex", OP_IDX | OP_PAGE2,            2, 0xa2,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "sbex", OP_IDX_1 | OP_PAGE2,          3, 0xa2,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "sbex", OP_IDX_2 | OP_PAGE2,          4, 0xa2,  4,  4,  CHG_NZVC, cpu9s12x, 0 },
  { "sbex", OP_D_IDX | OP_PAGE2,          2, 0xa2,  6,  6,  CHG_NZVC, cpu9s12x, 0 },
  { "sbex", OP_D_IDX_2 | OP_PAGE2,        4, 0xa2,  6,  6,  CHG_NZVC, cpu9s12x, 0 },

  { "sbey", OP_IMM16 | OP_PAGE2,          3, 0xc2,  2,  2,  CHG_NZVC, cpu9s12x, 0 },
  { "sbey", OP_DIRECT | OP_PAGE2,         2, 0xd2,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "sbey", OP_IND16 | OP_PAGE2,          3, 0xf2,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "sbey", OP_IDX | OP_PAGE2,            2, 0xe2,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "sbey", OP_IDX_1 | OP_PAGE2,          3, 0xe2,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "sbey", OP_IDX_2 | OP_PAGE2,          4, 0xe2,  4,  4,  CHG_NZVC, cpu9s12x, 0 },
  { "sbey", OP_D_IDX | OP_PAGE2,          2, 0xe2,  6,  6,  CHG_NZVC, cpu9s12x, 0 },
  { "sbey", OP_D_IDX_2 | OP_PAGE2,        4, 0xe2,  6,  6,  CHG_NZVC, cpu9s12x, 0 },

  { "sec",  OP_NONE,           1, 0x0d,  2,  2,  SET_C, cpu6811, 0 },
  { "sei",  OP_NONE,           1, 0x0f,  2,  2,  SET_I, cpu6811, 0 },
  { "sev",  OP_NONE,           1, 0x0b,  2,  2,  SET_V, cpu6811, 0 },

/* Some sex opcodes are synonyms for tfr. */
  { "sex",  M6812_OP_SEX_MARKER
          | OP_REG | OP_REG_2, 2, 0xb7,  1,  1,  CHG_NONE, cpu6812|cpu9s12x, 0 },

  { "staa", OP_IND16,          3, 0xb7,  4,  4,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "staa", OP_DIRECT,         2, 0x97,  3,  3,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "staa", OP_IX,             2, 0xa7,  4,  4,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "staa", OP_IY | OP_PAGE2,  3, 0xa7,  5,  5,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "staa", OP_DIRECT,         2, 0x5a,  2,  2,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "staa", OP_IND16,          3, 0x7a,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "staa", OP_IDX,            2, 0x6a,  2,  2,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "staa", OP_IDX_1,          3, 0x6a,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "staa", OP_IDX_2,          4, 0x6a,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "staa", OP_D_IDX,          2, 0x6a,  5,  5,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "staa", OP_D_IDX_2,        4, 0x6a,  5,  5,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },

  { "stab", OP_IND16,          3, 0xf7,  4,  4,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "stab", OP_DIRECT,         2, 0xd7,  3,  3,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "stab", OP_IX,             2, 0xe7,  4,  4,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "stab", OP_IY | OP_PAGE2,  3, 0xe7,  5,  5,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "stab", OP_DIRECT,         2, 0x5b,  2,  2,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "stab", OP_IND16,          3, 0x7b,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "stab", OP_IDX,            2, 0x6b,  2,  2,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "stab", OP_IDX_1,          3, 0x6b,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "stab", OP_IDX_2,          4, 0x6b,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "stab", OP_D_IDX,          2, 0x6b,  5,  5,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "stab", OP_D_IDX_2,        4, 0x6b,  5,  5,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },

  { "std",  OP_IND16,          3, 0xfd,  5,  5,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "std",  OP_DIRECT,         2, 0xdd,  4,  4,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "std",  OP_IX,             2, 0xed,  5,  5,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "std",  OP_IY | OP_PAGE2,  3, 0xed,  6,  6,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "std",  OP_DIRECT,         2, 0x5c,  2,  2,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "std",  OP_IND16,          3, 0x7c,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "std",  OP_IDX,            2, 0x6c,  2,  2,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "std",  OP_IDX_1,          3, 0x6c,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "std",  OP_IDX_2,          4, 0x6c,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "std",  OP_D_IDX,          2, 0x6c,  5,  5,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "std",  OP_D_IDX_2,        4, 0x6c,  5,  5,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },

  { "stop", OP_NONE,           1, 0xcf,  2,  2,  CHG_NONE, cpu6811, 0 },
  { "stop", OP_NONE | OP_PAGE2,2, 0x3e,  2,  9,  CHG_NONE, cpu6812|cpu9s12x, 0 },

  { "sts",  OP_IND16,          3, 0xbf,  5,  5,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "sts",  OP_DIRECT,         2, 0x9f,  4,  4,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "sts",  OP_IX,             2, 0xaf,  5,  5,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "sts",  OP_IY | OP_PAGE2,  3, 0xaf,  6,  6,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "sts",  OP_DIRECT,         2, 0x5f,  2,  2,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "sts",  OP_IND16,          3, 0x7f,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "sts",  OP_IDX,            2, 0x6f,  2,  2,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "sts",  OP_IDX_1,          3, 0x6f,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "sts",  OP_IDX_2,          4, 0x6f,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "sts",  OP_D_IDX,          2, 0x6f,  5,  5,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "sts",  OP_D_IDX_2,        4, 0x6f,  5,  5,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },

  { "stx",  OP_IND16,          3, 0xff,  5,  5,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "stx",  OP_DIRECT,         2, 0xdf,  4,  4,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "stx",  OP_IX,             2, 0xef,  5,  5,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "stx",  OP_IY | OP_PAGE4,  3, 0xef,  6,  6,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "stx",  OP_DIRECT,         2, 0x5e,  2,  2,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "stx",  OP_IND16,          3, 0x7e,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "stx",  OP_IDX,            2, 0x6e,  2,  2,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "stx",  OP_IDX_1,          3, 0x6e,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "stx",  OP_IDX_2,          4, 0x6e,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "stx",  OP_D_IDX,          2, 0x6e,  5,  5,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "stx",  OP_D_IDX_2,        4, 0x6e,  5,  5,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },

  { "sty",  OP_IND16 | OP_PAGE2,  4, 0xff, 6, 6, CLR_V_CHG_NZ, cpu6811, 0 },
  { "sty",  OP_DIRECT | OP_PAGE2, 3, 0xdf, 5, 5, CLR_V_CHG_NZ, cpu6811, 0 },
  { "sty",  OP_IY | OP_PAGE2,     3, 0xef, 6, 6, CLR_V_CHG_NZ, cpu6811, 0 },
  { "sty",  OP_IX | OP_PAGE3,     3, 0xef, 6, 6, CLR_V_CHG_NZ, cpu6811, 0 },
  { "sty",  OP_DIRECT,         2, 0x5d,  2,  2,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "sty",  OP_IND16,          3, 0x7d,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "sty",  OP_IDX,            2, 0x6d,  2,  2,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "sty",  OP_IDX_1,          3, 0x6d,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "sty",  OP_IDX_2,          4, 0x6d,  3,  3,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "sty",  OP_D_IDX,          2, 0x6d,  5,  5,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "sty",  OP_D_IDX_2,        4, 0x6d,  5,  5,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },

  { "suba", OP_IMM8,           2, 0x80,  1,  1,  CHG_NZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "suba", OP_DIRECT,         2, 0x90,  3,  3,  CHG_NZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "suba", OP_IND16,          3, 0xb0,  3,  3,  CHG_NZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "suba", OP_IX,             2, 0xa0,  4,  4,  CHG_NZVC, cpu6811, 0 },
  { "suba", OP_IY | OP_PAGE2,  3, 0xa0,  5,  5,  CHG_NZVC, cpu6811, 0 },
  { "suba", OP_IDX,            2, 0xa0,  3,  3,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "suba", OP_IDX_1,          3, 0xa0,  3,  3,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "suba", OP_IDX_2,          4, 0xa0,  4,  4,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "suba", OP_D_IDX,          2, 0xa0,  6,  6,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "suba", OP_D_IDX_2,        4, 0xa0,  6,  6,  CHG_NZVC, cpu6812|cpu9s12x, 0 },

  { "subb", OP_IMM8,           2, 0xc0,  1,  1,  CHG_NZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "subb", OP_DIRECT,         2, 0xd0,  3,  3,  CHG_NZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "subb", OP_IND16,          3, 0xf0,  3,  3,  CHG_NZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "subb", OP_IX,             2, 0xe0,  4,  4,  CHG_NZVC, cpu6811, 0 },
  { "subb", OP_IY | OP_PAGE2,  3, 0xe0,  5,  5,  CHG_NZVC, cpu6811, 0 },
  { "subb", OP_IDX,            2, 0xe0,  3,  3,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "subb", OP_IDX_1,          3, 0xe0,  3,  3,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "subb", OP_IDX_2,          4, 0xe0,  4,  4,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "subb", OP_D_IDX,          2, 0xe0,  6,  6,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "subb", OP_D_IDX_2,        4, 0xe0,  6,  6,  CHG_NZVC, cpu6812|cpu9s12x, 0 },

  { "subd", OP_IMM16,          3, 0x83,  2,  2,  CHG_NZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "subd", OP_DIRECT,         2, 0x93,  3,  3,  CHG_NZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "subd", OP_IND16,          3, 0xb3,  3,  3,  CHG_NZVC, cpu6811|cpu6812|cpu9s12x, 0 },
  { "subd", OP_IX,             2, 0xa3,  6,  6,  CHG_NZVC, cpu6811, 0 },
  { "subd", OP_IY | OP_PAGE2,  3, 0xa3,  7,  7,  CHG_NZVC, cpu6811, 0 },
  { "subd", OP_IDX,            2, 0xa3,  3,  3,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "subd", OP_IDX_1,          3, 0xa3,  3,  3,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "subd", OP_IDX_2,          4, 0xa3,  4,  4,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "subd", OP_D_IDX,          2, 0xa3,  6,  6,  CHG_NZVC, cpu6812|cpu9s12x, 0 },
  { "subd", OP_D_IDX_2,        4, 0xa3,  6,  6,  CHG_NZVC, cpu6812|cpu9s12x, 0 },

  { "subx", OP_IMM16 | OP_PAGE2,          3, 0x80,  2,  2,  CHG_NZVC, cpu9s12x, 0 },
  { "subx", OP_DIRECT | OP_PAGE2,         2, 0x90,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "subx", OP_IND16 | OP_PAGE2,          3, 0xb0,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "subx", OP_IDX | OP_PAGE2,            2, 0xa0,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "subx", OP_IDX_1 | OP_PAGE2,          3, 0xa0,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "subx", OP_IDX_2 | OP_PAGE2,          4, 0xa0,  4,  4,  CHG_NZVC, cpu9s12x, 0 },
  { "subx", OP_D_IDX | OP_PAGE2,          2, 0xa0,  6,  6,  CHG_NZVC, cpu9s12x, 0 },
  { "subx", OP_D_IDX_2 | OP_PAGE2,        4, 0xa0,  6,  6,  CHG_NZVC, cpu9s12x, 0 },

  { "suby", OP_IMM16 | OP_PAGE2,          3, 0xc0,  2,  2,  CHG_NZVC, cpu9s12x, 0 },
  { "suby", OP_DIRECT | OP_PAGE2,         2, 0xd0,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "suby", OP_IND16 | OP_PAGE2,          3, 0xf0,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "suby", OP_IDX | OP_PAGE2,            2, 0xe0,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "suby", OP_IDX_1 | OP_PAGE2,          3, 0xe0,  3,  3,  CHG_NZVC, cpu9s12x, 0 },
  { "suby", OP_IDX_2 | OP_PAGE2,          4, 0xe0,  4,  4,  CHG_NZVC, cpu9s12x, 0 },
  { "suby", OP_D_IDX | OP_PAGE2,          2, 0xe0,  6,  6,  CHG_NZVC, cpu9s12x, 0 },
  { "suby", OP_D_IDX_2 | OP_PAGE2,        4, 0xe0,  6,  6,  CHG_NZVC, cpu9s12x, 0 },

  { "swi",  OP_NONE,           1, 0x3f,  9,  9,  CHG_NONE, cpu6811|cpu6812|cpu9s12x, 0 },
  { "sys",  OP_NONE | OP_PAGE2,2, 0xa7,  9,  9,  SET_I, cpu9s12x, 0 },

  { "tab",  OP_NONE,           1, 0x16,  2,  2,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "tab",  OP_NONE | OP_PAGE2,2, 0x0e,  2,  2,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },

  { "tap",  OP_NONE,           1, 0x06,  2,  2,  CHG_ALL, cpu6811, 0 },

  { "tba",  OP_NONE,           1, 0x17,  2,  2,  CLR_V_CHG_NZ, cpu6811, 0 },
  { "tba",  OP_NONE | OP_PAGE2,2, 0x0f,  2,  2,  CLR_V_CHG_NZ, cpu6812|cpu9s12x, 0 },

  { "test", OP_NONE,           1, 0x00,  5, _M,  CHG_NONE, cpu6811, 0 },

  { "tpa",  OP_NONE,           1, 0x07,  2,  2,  CHG_NONE, cpu6811, 0 },

  { "tbeq", OP_TBEQ_MARKER
          | OP_REG | OP_JUMP_REL,  3, 0x04,  3,  3, CHG_NONE, cpu6812|cpu9s12x, 0 },

  { "tbl",  OP_IDX | OP_PAGE2,  3, 0x3d,  8,  8, CHG_NZC, cpu6812|cpu9s12x, 0 },

  { "tbne", OP_TBNE_MARKER
          | OP_REG | OP_JUMP_REL,  3, 0x04,  3,  3, CHG_NONE, cpu6812|cpu9s12x, 0 },

/* The S12X has more tfr variants, but most are pointless so not supported. */
  { "tfr",  OP_TFR_MARKER
          | OP_REG_1 | OP_REG_2, 2, 0xb7, 1, 1,  CHG_NONE, cpu6812|cpu9s12x, 0 },

  { "trap", OP_IMM8 | OP_TRAP_ID, 2, 0x18,  11,  11,  SET_I, cpu6812|cpu9s12x, 0 },

  { "tst",  OP_IND16,          3, 0x7d,  6,  6,  CLR_VC_CHG_NZ, cpu6811, 0 },
  { "tst",  OP_IX,             2, 0x6d,  6,  6,  CLR_VC_CHG_NZ, cpu6811, 0 },
  { "tst",  OP_IY | OP_PAGE2,  3, 0x6d,  7,  7,  CLR_VC_CHG_NZ, cpu6811, 0 },
  { "tst",  OP_IND16,          3, 0xf7,  3,  3,  CLR_VC_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "tst",  OP_IDX,            2, 0xe7,  3,  3,  CLR_VC_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "tst",  OP_IDX_1,          3, 0xe7,  3,  3,  CLR_VC_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "tst",  OP_IDX_2,          4, 0xe7,  4,  4,  CLR_VC_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "tst",  OP_D_IDX,          2, 0xe7,  6,  6,  CLR_VC_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "tst",  OP_D_IDX_2,        4, 0xe7,  6,  6,  CLR_VC_CHG_NZ, cpu6812|cpu9s12x, 0 },

  { "tsta", OP_NONE,           1, 0x4d,  2,  2,  CLR_VC_CHG_NZ, cpu6811, 0 },
  { "tsta", OP_NONE,           1, 0x97,  1,  1,  CLR_VC_CHG_NZ, cpu6812|cpu9s12x, 0 },
  { "tstb", OP_NONE,           1, 0x5d,  2,  2,  CLR_VC_CHG_NZ, cpu6811, 0 },
  { "tstb", OP_NONE,           1, 0xd7,  1,  1,  CLR_VC_CHG_NZ, cpu6812|cpu9s12x, 0 },

  { "tstw",  OP_IND16| OP_PAGE2,          3, 0xf7,  3,  3,  CLR_VC_CHG_NZ, cpu9s12x, 0 },
  { "tstw",  OP_IDX| OP_PAGE2,            2, 0xe7,  3,  3,  CLR_VC_CHG_NZ, cpu9s12x, 0 },
  { "tstw",  OP_IDX_1| OP_PAGE2,          3, 0xe7,  3,  3,  CLR_VC_CHG_NZ, cpu9s12x, 0 },
  { "tstw",  OP_IDX_2| OP_PAGE2,          4, 0xe7,  4,  4,  CLR_VC_CHG_NZ, cpu9s12x, 0 },
  { "tstw",  OP_D_IDX| OP_PAGE2,          2, 0xe7,  6,  6,  CLR_VC_CHG_NZ, cpu9s12x, 0 },
  { "tstw",  OP_D_IDX_2| OP_PAGE2,        4, 0xe7,  6,  6,  CLR_VC_CHG_NZ, cpu9s12x, 0 },

  { "tstx", OP_NONE| OP_PAGE2,           1, 0x97,  1,  1,  CLR_VC_CHG_NZ, cpu9s12x, 0 },
  { "tsty", OP_NONE| OP_PAGE2,           1, 0xd7,  1,  1,  CLR_VC_CHG_NZ, cpu9s12x, 0 },

  { "tsx",  OP_NONE,           1, 0x30,  3,  3,  CHG_NONE, cpu6811, 0 },
  { "tsy",  OP_NONE | OP_PAGE2,2, 0x30,  4,  4,  CHG_NONE, cpu6811, 0 },
  { "txs",  OP_NONE,           1, 0x35,  3,  3,  CHG_NONE, cpu6811, 0 },
  { "tys",  OP_NONE | OP_PAGE2,2, 0x35,  4,  4,  CHG_NONE, cpu6811, 0 },

  { "wai",  OP_NONE,           1, 0x3e,  5,  _M, CHG_NONE, cpu6811|cpu6812|cpu9s12x, 0 },

  { "wav",  OP_NONE | OP_PAGE2, 2, 0x3c,  8,  _M, SET_Z_CHG_HNVC, cpu6812|cpu9s12x, 0 },

  { "xgdx", OP_NONE,           1, 0x8f,  3,  3,  CHG_NONE, cpu6811, 0 },
  { "xgdy", OP_NONE | OP_PAGE2,2, 0x8f,  4,  4,  CHG_NONE, cpu6811, 0 },

/* XGATE opcodes */
/* Return to Scheduler and Others*/
  { "brk",   M68XG_OP_NONE,           2, 0x0000, 0, 0, 0, 0, 0, cpuxgate, 0xffff },
  { "nop",   M68XG_OP_NONE,           2, 0x0100, 0, 0, 0, 0, 0, cpuxgate, 0xffff },
  { "rts",   M68XG_OP_NONE,           2, 0x0200, 0, 0, 0, 0, 0, cpuxgate, 0xffff },
  { "sif",   M68XG_OP_NONE,           2, 0x0300, 0, 0, 0, 0, 0, cpuxgate, 0xffff },
/* Semaphore Instructions */
  { "csem",  M68XG_OP_IMM3,           2, 0x00f0, 0, 0, 0, 0, 0, cpuxgate, 0xf8ff },
  { "csem",  M68XG_OP_R,              2, 0x00f1, 0, 0, 0, 0, 0, cpuxgate, 0xf8ff },
  { "ssem",  M68XG_OP_IMM3,           2, 0x00f2, 0, 0, 0, 0, 0, cpuxgate, 0xf8ff },
  { "ssem",  M68XG_OP_R,              2, 0x00f3, 0, 0, 0, 0, 0, cpuxgate, 0xf8ff },
/* Single Register Instructions */
  { "sex",   M68XG_OP_R,              2, 0x00f4, 0, 0, 0, 0, 0, cpuxgate, 0xf8ff },
  { "par",   M68XG_OP_R,              2, 0x00f5, 0, 0, 0, 0, 0, cpuxgate, 0xf8ff },
  { "jal",   M68XG_OP_R,              2, 0x00f6, 0, 0, 0, 0, 0, cpuxgate, 0xf8ff },
  { "sif",   M68XG_OP_R,              2, 0x00f7, 0, 0, 0, 0, 0, cpuxgate, 0xf8ff },
/* Special Move instructions */
  { "tfr",   M68XG_OP_R,              2, 0x00f8, 0, 0, 0, 0, 0, cpuxgate, 0xf8ff }, /* RD,CCR */
  { "tfr",   M68XG_OP_R,              2, 0x00f9, 0, 0, 0, 0, 0, cpuxgate, 0xf8ff }, /* CCR,RS */
  { "tfr",   M68XG_OP_R,              2, 0x00fa, 0, 0, 0, 0, 0, cpuxgate, 0xf8ff }, /* RD,PC  */
/* Shift instructions Dyadic */
  { "bffo",  M68XG_OP_R_R,            2, 0x0810, 0, 0, 0, 0, 0, cpuxgate, 0xf81f },
  { "asr",   M68XG_OP_R_R,            2, 0x0811, 0, 0, 0, 0, 0, cpuxgate, 0xf81f },
  { "csl",   M68XG_OP_R_R,            2, 0x0812, 0, 0, 0, 0, 0, cpuxgate, 0xf81f },
  { "csr",   M68XG_OP_R_R,            2, 0x0813, 0, 0, 0, 0, 0, cpuxgate, 0xf81f },
  { "lsl",   M68XG_OP_R_R,            2, 0x0814, 0, 0, 0, 0, 0, cpuxgate, 0xf81f },
  { "lsr",   M68XG_OP_R_R,            2, 0x0815, 0, 0, 0, 0, 0, cpuxgate, 0xf81f },
  { "rol",   M68XG_OP_R_R,            2, 0x0816, 0, 0, 0, 0, 0, cpuxgate, 0xf81f },
  { "ror",   M68XG_OP_R_R,            2, 0x0817, 0, 0, 0, 0, 0, cpuxgate, 0xf81f },
/* Dyadic aliases */
  { "cmp",   M68XG_OP_R_R,            2, 0x1800, 0, 0, 0, 0, 0, cpuxgate, 0x0000 },
  { "com",   M68XG_OP_R_R,            2, 0x1003, 0, 0, 0, 0, 0, cpuxgate, 0x0000 },
  { "cpc",   M68XG_OP_R_R,            2, 0x1801, 0, 0, 0, 0, 0, cpuxgate, 0x0000 },
  { "mov",   M68XG_OP_R_R,            2, 0x1002, 0, 0, 0, 0, 0, cpuxgate, 0x0000 },
  { "neg",   M68XG_OP_R_R,            2, 0x1800, 0, 0, 0, 0, 0, cpuxgate, 0x0000 },
/* Monadic aliases */
  { "com",   M68XG_OP_R,              2, 0x1003, 0, 0, 0, 0, 0, cpuxgate, 0x0000 },
  { "neg",   M68XG_OP_R,              2, 0x1800, 0, 0, 0, 0, 0, cpuxgate, 0x0000 },
  { "tst",   M68XG_OP_R,              2, 0x1800, 0, 0, 0, 0, 0, cpuxgate, 0x0000 },
/* Shift instructions immediate */
  { "asr",   M68XG_OP_R_IMM4,         2, 0x0809, 0, 0, 0, 0, 0, cpuxgate, 0xf80f },
  { "csl",   M68XG_OP_R_IMM4,         2, 0x080a, 0, 0, 0, 0, 0, cpuxgate, 0xf80f },
  { "csr",   M68XG_OP_R_IMM4,         2, 0x080b, 0, 0, 0, 0, 0, cpuxgate, 0xf80f },
  { "lsl",   M68XG_OP_R_IMM4,         2, 0x080c, 0, 0, 0, 0, 0, cpuxgate, 0xf80f },
  { "lsr",   M68XG_OP_R_IMM4,         2, 0x080d, 0, 0, 0, 0, 0, cpuxgate, 0xf80f },
  { "rol",   M68XG_OP_R_IMM4,         2, 0x080e, 0, 0, 0, 0, 0, cpuxgate, 0xf80f },
  { "ror",   M68XG_OP_R_IMM4,         2, 0x080f, 0, 0, 0, 0, 0, cpuxgate, 0xf80f },
/* Logical Triadic */
  { "and",   M68XG_OP_R_R_R,          2, 0x1000, 0, 0, 0, 0, 0, cpuxgate, 0xf803 },
  { "or",    M68XG_OP_R_R_R,          2, 0x1002, 0, 0, 0, 0, 0, cpuxgate, 0xf803 },
  { "xnor",  M68XG_OP_R_R_R,          2, 0x1003, 0, 0, 0, 0, 0, cpuxgate, 0xf803 },
/* Arithmetic Triadic */
  { "sub",   M68XG_OP_R_R_R,          2, 0x1800, 0, 0, 0, 0, 0, cpuxgate, 0xf803 },
  { "sbc",   M68XG_OP_R_R_R,          2, 0x1801, 0, 0, 0, 0, 0, cpuxgate, 0xf803 },
  { "add",   M68XG_OP_R_R_R,          2, 0x1802, 0, 0, 0, 0, 0, cpuxgate, 0xf803 },
  { "adc",   M68XG_OP_R_R_R,          2, 0x1803, 0, 0, 0, 0, 0, cpuxgate, 0xf803 },
/* Branches */
  { "bcc",   M68XG_OP_REL9,           2, 0x2000, 0, 0, 0, 0, 0, cpuxgate, 0xfe00 },
  { "bhs",   M68XG_OP_REL9,           2, 0x2000, 0, 0, 0, 0, 0, cpuxgate, 0x0000 }, /* Synonym. */
  { "bcs",   M68XG_OP_REL9,           2, 0x2200, 0, 0, 0, 0, 0, cpuxgate, 0xfe00 },
  { "blo",   M68XG_OP_REL9,           2, 0x2200, 0, 0, 0, 0, 0, cpuxgate, 0xfe00 }, /* Synonym. */
  { "bne",   M68XG_OP_REL9,           2, 0x2400, 0, 0, 0, 0, 0, cpuxgate, 0xfe00 },
  { "beq",   M68XG_OP_REL9,           2, 0x2600, 0, 0, 0, 0, 0, cpuxgate, 0xfe00 },
  { "bpl",   M68XG_OP_REL9,           2, 0x2800, 0, 0, 0, 0, 0, cpuxgate, 0xfe00 },
  { "bmi",   M68XG_OP_REL9,           2, 0x2a00, 0, 0, 0, 0, 0, cpuxgate, 0xfe00 },
  { "bvc",   M68XG_OP_REL9,           2, 0x2c00, 0, 0, 0, 0, 0, cpuxgate, 0xfe00 },
  { "bvs",   M68XG_OP_REL9,           2, 0x2e00, 0, 0, 0, 0, 0, cpuxgate, 0xfe00 },
  { "bhi",   M68XG_OP_REL9,           2, 0x3000, 0, 0, 0, 0, 0, cpuxgate, 0xfe00 },
  { "bls",   M68XG_OP_REL9,           2, 0x3200, 0, 0, 0, 0, 0, cpuxgate, 0xfe00 },
  { "bge",   M68XG_OP_REL9,           2, 0x3400, 0, 0, 0, 0, 0, cpuxgate, 0xfe00 },
  { "blt",   M68XG_OP_REL9,           2, 0x3600, 0, 0, 0, 0, 0, cpuxgate, 0xfe00 },
  { "bgt",   M68XG_OP_REL9,           2, 0x3800, 0, 0, 0, 0, 0, cpuxgate, 0xfe00 },
  { "ble",   M68XG_OP_REL9,           2, 0x3a00, 0, 0, 0, 0, 0, cpuxgate, 0xfe00 },
  { "bra",   M68XG_OP_REL10,          2, 0x3c00, 0, 0, 0, 0, 0, cpuxgate, 0xfc00 },
/* Load and Store Instructions */
  { "ldb",   M68XG_OP_R_R_OFFS5,      2, 0x4000, 0, 0, 0, 0, 0, cpuxgate, 0xf800 },
  { "ldw",   M68XG_OP_R_R_OFFS5,      2, 0x4800, 0, 0, 0, 0, 0, cpuxgate, 0xf800 },
  { "stb",   M68XG_OP_R_R_OFFS5,      2, 0x5000, 0, 0, 0, 0, 0, cpuxgate, 0xf800 },
  { "stw",   M68XG_OP_R_R_OFFS5,      2, 0x5800, 0, 0, 0, 0, 0, cpuxgate, 0xf800 },

  { "ldb",   M68XG_OP_RD_RB_RI,       2, 0x6000, 0, 0, 0, 0, 0, cpuxgate, 0xf803 },
  { "ldw",   M68XG_OP_RD_RB_RI,       2, 0x6800, 0, 0, 0, 0, 0, cpuxgate, 0xf803 },
  { "stb",   M68XG_OP_RD_RB_RI,       2, 0x7000, 0, 0, 0, 0, 0, cpuxgate, 0xf803 },
  { "stw",   M68XG_OP_RD_RB_RI,       2, 0x7800, 0, 0, 0, 0, 0, cpuxgate, 0xf803 },

  { "ldb",   M68XG_OP_RD_RB_RIp,      2, 0x6001, 0, 0, 0, 0, 0, cpuxgate, 0xf803 },
  { "ldw",   M68XG_OP_RD_RB_RIp,      2, 0x6801, 0, 0, 0, 0, 0, cpuxgate, 0xf803 },
  { "stb",   M68XG_OP_RD_RB_RIp,      2, 0x7001, 0, 0, 0, 0, 0, cpuxgate, 0xf803 },
  { "stw",   M68XG_OP_RD_RB_RIp,      2, 0x7801, 0, 0, 0, 0, 0, cpuxgate, 0xf803 },

  { "ldb",   M68XG_OP_RD_RB_mRI,      2, 0x6002, 0, 0, 0, 0, 0, cpuxgate, 0xf803 },
  { "ldw",   M68XG_OP_RD_RB_mRI,      2, 0x6802, 0, 0, 0, 0, 0, cpuxgate, 0xf803 },
  { "stb",   M68XG_OP_RD_RB_mRI,      2, 0x7002, 0, 0, 0, 0, 0, cpuxgate, 0xf803 },
  { "stw",   M68XG_OP_RD_RB_mRI,      2, 0x7802, 0, 0, 0, 0, 0, cpuxgate, 0xf803 },
/* Bit Field Instructions */
  { "bfext", M68XG_OP_R_R_R,          2, 0x6003, 0, 0, 0, 0, 0, cpuxgate, 0xf803 },
  { "bfins", M68XG_OP_R_R_R,          2, 0x6803, 0, 0, 0, 0, 0, cpuxgate, 0xf803 },
  { "bfinsi",M68XG_OP_R_R_R,          2, 0x7003, 0, 0, 0, 0, 0, cpuxgate, 0xf803 },
  { "bfinsx",M68XG_OP_R_R_R,          2, 0x7803, 0, 0, 0, 0, 0, cpuxgate, 0xf803 },
/* Logic Immediate Instructions */
  { "andl",  M68XG_OP_R_IMM8,         2, 0x8000, 0, 0, 0, 0, 0, cpuxgate, 0xf800 },
  { "andh",  M68XG_OP_R_IMM8,         2, 0x8800, 0, 0, 0, 0, 0, cpuxgate, 0xf800 },
  { "bitl",  M68XG_OP_R_IMM8,         2, 0x9000, 0, 0, 0, 0, 0, cpuxgate, 0xf800 },
  { "bith",  M68XG_OP_R_IMM8,         2, 0x9800, 0, 0, 0, 0, 0, cpuxgate, 0xf800 },
  { "orl",   M68XG_OP_R_IMM8,         2, 0xa000, 0, 0, 0, 0, 0, cpuxgate, 0xf800 },
  { "orh",   M68XG_OP_R_IMM8,         2, 0xa800, 0, 0, 0, 0, 0, cpuxgate, 0xf800 },
  { "xnorl", M68XG_OP_R_IMM8,         2, 0xb000, 0, 0, 0, 0, 0, cpuxgate, 0xf800 },
  { "xnorh", M68XG_OP_R_IMM8,         2, 0xb800, 0, 0, 0, 0, 0, cpuxgate, 0xf800 },
/* Arithmetic Immediate Instructions */
  { "subl",  M68XG_OP_R_IMM8,         2, 0xc000, 0, 0, 0, 0, 0, cpuxgate, 0xf800 },
  { "subh",  M68XG_OP_R_IMM8,         2, 0xc800, 0, 0, 0, 0, 0, cpuxgate, 0xf800 },
  { "cmpl",  M68XG_OP_R_IMM8,         2, 0xd000, 0, 0, 0, 0, 0, cpuxgate, 0xf800 },
  { "cpch",  M68XG_OP_R_IMM8,         2, 0xd800, 0, 0, 0, 0, 0, cpuxgate, 0xf800 },
  { "addl",  M68XG_OP_R_IMM8,         2, 0xe000, 0, 0, 0, 0, 0, cpuxgate, 0xf800 },
  { "addh",  M68XG_OP_R_IMM8,         2, 0xe800, 0, 0, 0, 0, 0, cpuxgate, 0xf800 },
  { "ldl",   M68XG_OP_R_IMM8,         2, 0xf000, 0, 0, 0, 0, 0, cpuxgate, 0xf800 },
  { "ldh",   M68XG_OP_R_IMM8,         2, 0xf800, 0, 0, 0, 0, 0, cpuxgate, 0xf800 },
/* 16 bit versions.
 * These are pseudo opcodes to allow 16 bit addresses to be passed.
 * The mask ensures that we will never disassemble to these instructions.
 */
/* Logic Immediate Instructions */
  { "and",   M68XG_OP_R_IMM16,        2, 0x8000, 0, 0, 0, 0, 0, cpuxgate, 0x0000 },
  { "bit",   M68XG_OP_R_IMM16,        2, 0x9000, 0, 0, 0, 0, 0, cpuxgate, 0x0000 },
  { "or",    M68XG_OP_R_IMM16,        2, 0xa000, 0, 0, 0, 0, 0, cpuxgate, 0x0000 },
  { "xnor",  M68XG_OP_R_IMM16,        2, 0xb000, 0, 0, 0, 0, 0, cpuxgate, 0x0000 },
/* Arithmetic Immediate Instructions */
  { "sub",   M68XG_OP_R_IMM16,        2, 0xc000, 0, 0, 0, 0, 0, cpuxgate, 0x0000 },
  { "cmp",   M68XG_OP_R_IMM16,        2, 0xd000, 0, 0, 0, 0, 0, cpuxgate, 0x0000 },
  { "add",   M68XG_OP_R_IMM16,        2, 0xe000, 0, 0, 0, 0, 0, cpuxgate, 0x0000 },
 /* ld is for backwards compatability only, the correct opcode is ldw */
  { "ld",    M68XG_OP_R_IMM16,        2, 0xf000, 0, 0, 0, 0, 0, cpuxgate, 0x0000 },
  { "ldw",   M68XG_OP_R_IMM16,        2, 0xf000, 0, 0, 0, 0, 0, cpuxgate, 0x0000 }
};

const int m68hc11_num_opcodes = TABLE_SIZE (m68hc11_opcodes);

/* The following alias table provides source compatibility to
   move from 68HC11 assembly to 68HC12.  */
const struct m68hc12_opcode_alias m68hc12_alias[] = {
  { "abx", "leax b,x",   2, 0x1a, 0xe5 },
  { "aby", "leay b,y",   2, 0x19, 0xed },
  { "clc", "andcc #$fe", 2, 0x10, 0xfe },
  { "cli", "andcc #$ef", 2, 0x10, 0xef },
  { "clv", "andcc #$fd", 2, 0x10, 0xfd },
  { "des", "leas -1,sp", 2, 0x1b, 0x9f },
  { "ins", "leas 1,sp",  2, 0x1b, 0x81 },
  { "sec", "orcc #$01",  2, 0x14, 0x01 },
  { "sei", "orcc #$10",  2, 0x14, 0x10 },
  { "sev", "orcc #$02",  2, 0x14, 0x02 },
  { "tap", "tfr a,ccr",  2, 0xb7, 0x02 },
  { "tpa", "tfr ccr,a",  2, 0xb7, 0x20 },
  { "tsx", "tfr sp,x",   2, 0xb7, 0x75 },
  { "tsy", "tfr sp,y",   2, 0xb7, 0x76 },
  { "txs", "tfr x,sp",   2, 0xb7, 0x57 },
  { "tys", "tfr y,sp",   2, 0xb7, 0x67 },
  { "xgdx","exg d,x",    2, 0xb7, 0xc5 },
  { "xgdy","exg d,y",    2, 0xb7, 0xc6 }
};
const int m68hc12_num_alias = TABLE_SIZE (m68hc12_alias);
