/* ft32-opc.c -- Definitions for ft32 opcodes.
   Copyright (C) 2013-2023 Free Software Foundation, Inc.
   Contributed by FTDI (support@ftdichip.com)

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
#include "opcode/ft32.h"

const ft32_opc_info_t ft32_opc_info[] =
{
{       "nop", 0, 0xffffffffU, 0x40004000U, 0},
{      "move", 1, 0xf8007fffU, 0x40004000U, FT32_FLD_R_D|FT32_FLD_R_1},
{  "streamin", 1, 0xf800000fU, 0xf000000cU, FT32_FLD_R_D|FT32_FLD_R_1|FT32_FLD_RIMM},
{       "ldi", 1, 0xf8000000U, 0xa8000000U, FT32_FLD_R_D|FT32_FLD_R_1|FT32_FLD_K15},
{       "exi", 1, 0xf8000000U, 0xe8000000U, FT32_FLD_R_D|FT32_FLD_R_1|FT32_FLD_K15},
{    "return", 0, 0xfc000000U, 0xa0000000U, 0},
{        "or", 1, 0xf800000fU, 0x40000005U, FT32_FLD_R_D|FT32_FLD_R_1|FT32_FLD_RIMM},
{       "pop", 1, 0xf8000000U, 0x88000000U, FT32_FLD_R_D},
{       "sta", 1, 0xf8000000U, 0xb8000000U, FT32_FLD_AA|FT32_FLD_R_D_POST},
{       "xor", 1, 0xf800000fU, 0x40000006U, FT32_FLD_R_D|FT32_FLD_R_1|FT32_FLD_RIMM},
{    "memcpy", 1, 0xf800000fU, 0xf0000005U, FT32_FLD_R_D|FT32_FLD_R_1|FT32_FLD_RIMM},
{    "strlen", 1, 0xf8007fffU, 0xf0004006U, FT32_FLD_R_D|FT32_FLD_R_1},
{       "cmp", 1, 0xf9f0000fU, 0x59e00002U, FT32_FLD_R_1|FT32_FLD_RIMM},
{      "btst", 1, 0xf9f0000fU, 0x59e0000cU, FT32_FLD_R_1|FT32_FLD_RIMM},
{      "link", 0, 0xfe000000U, 0x94000000U, FT32_FLD_R_D|FT32_FLD_K16},
{      "call", 0, 0xfffc0000U, 0x00340000U, FT32_FLD_PA},
{     "callc", 0, 0xfe340000U, 0x00240000U, FT32_FLD_CBCRCV|FT32_FLD_PA},
{     "callx", 0, 0xf8040000U, 0x00040000U, FT32_FLD_CR|FT32_FLD_CB|FT32_FLD_CV|FT32_FLD_PA},
{ "streamini", 1, 0xf800000fU, 0xf000000dU, FT32_FLD_R_D|FT32_FLD_R_1|FT32_FLD_RIMM},
{       "lda", 1, 0xf8000000U, 0xc0000000U, FT32_FLD_R_D|FT32_FLD_AA},
{       "exa", 1, 0xf8000000U, 0x38000000U, FT32_FLD_R_D|FT32_FLD_AA},
{    "unlink", 0, 0xf8000000U, 0x98000000U, FT32_FLD_R_D},
{     "calli", 0, 0xfffc0000U, 0x08340000U, FT32_FLD_R_2},
{    "stpcpy", 1, 0xf8007fffU, 0xf000400aU, FT32_FLD_R_D|FT32_FLD_R_1},
{       "jmp", 0, 0xfffc0000U, 0x00300000U, FT32_FLD_PA},
{    "strcmp", 1, 0xf800000fU, 0xf0000004U, FT32_FLD_R_D|FT32_FLD_R_1|FT32_FLD_RIMM},
{       "mul", 1, 0xf800000fU, 0xf0000008U, FT32_FLD_R_D|FT32_FLD_R_1|FT32_FLD_RIMM},
{      "push", 1, 0xf8000000U, 0x80000000U, FT32_FLD_R_1},
{       "sti", 1, 0xf8000000U, 0xb0000000U, FT32_FLD_R_D|FT32_FLD_K15|FT32_FLD_R_1_POST},
{       "mod", 1, 0xf800000fU, 0xf0000003U, FT32_FLD_R_D|FT32_FLD_R_1|FT32_FLD_RIMM},
{      "umod", 1, 0xf800000fU, 0xf0000001U, FT32_FLD_R_D|FT32_FLD_R_1|FT32_FLD_RIMM},
{     "addcc", 1, 0xf9f0000fU, 0x59e00000U, FT32_FLD_R_1|FT32_FLD_RIMM},
{ "streamout", 1, 0xf800000fU, 0xf000000eU, FT32_FLD_R_D|FT32_FLD_R_1|FT32_FLD_RIMM},
{      "lpmi", 1, 0xf8000000U, 0xc8000000U, FT32_FLD_R_D|FT32_FLD_R_1|FT32_FLD_K15},
{      "udiv", 1, 0xf800000fU, 0xf0000000U, FT32_FLD_R_D|FT32_FLD_R_1|FT32_FLD_RIMM},
{       "sub", 1, 0xf800000fU, 0x40000002U, FT32_FLD_R_D|FT32_FLD_R_1|FT32_FLD_RIMM},
{      "lshr", 1, 0xf800000fU, 0x40000009U, FT32_FLD_R_D|FT32_FLD_R_1|FT32_FLD_RIMM},
{       "tst", 1, 0xf9f0000fU, 0x59e00004U, FT32_FLD_R_1|FT32_FLD_RIMM},
{      "xnor", 1, 0xf800000fU, 0x40000007U, FT32_FLD_R_D|FT32_FLD_R_1|FT32_FLD_RIMM},
{     "muluh", 1, 0xf800000fU, 0xf0000009U, FT32_FLD_R_D|FT32_FLD_R_1|FT32_FLD_RIMM},
{       "and", 1, 0xf800000fU, 0x40000004U, FT32_FLD_R_D|FT32_FLD_R_1|FT32_FLD_RIMM},
{       "add", 1, 0xf800000fU, 0x40000000U, FT32_FLD_R_D|FT32_FLD_R_1|FT32_FLD_RIMM},
{       "ror", 1, 0xf800000fU, 0x40000001U, FT32_FLD_R_D|FT32_FLD_R_1|FT32_FLD_RIMM},
{       "ldl", 1, 0xf800000fU, 0x40000003U, FT32_FLD_R_D|FT32_FLD_R_1|FT32_FLD_RIMM},
{      "bins", 1, 0xf800000fU, 0x4000000bU, FT32_FLD_R_D|FT32_FLD_R_1|FT32_FLD_RIMM},
{     "bexts", 1, 0xf800000fU, 0x4000000cU, FT32_FLD_R_D|FT32_FLD_R_1|FT32_FLD_RIMM},
{     "bextu", 1, 0xf800000fU, 0x4000000dU, FT32_FLD_R_D|FT32_FLD_R_1|FT32_FLD_RIMM},
{      "flip", 1, 0xf800000fU, 0x4000000eU, FT32_FLD_R_D|FT32_FLD_R_1|FT32_FLD_RIMM},
{    "memset", 1, 0xf800000fU, 0xf0000007U, FT32_FLD_R_D|FT32_FLD_R_1|FT32_FLD_RIMM},
{       "div", 1, 0xf800000fU, 0xf0000002U, FT32_FLD_R_D|FT32_FLD_R_1|FT32_FLD_RIMM},
{       "ldk", 1, 0xf8000000U, 0x60000000U, FT32_FLD_R_D|FT32_FLD_K20},
{      "reti", 0, 0xfc000000U, 0xa4000000U, 0},
{      "ashr", 1, 0xf800000fU, 0x4000000aU, FT32_FLD_R_D|FT32_FLD_R_1|FT32_FLD_RIMM},
{      "jmpi", 0, 0xfffc0000U, 0x08300000U, FT32_FLD_R_2},
{     "jmpic", 0, 0xf8040000U, 0x08000000U, FT32_FLD_CBCRCV|FT32_FLD_R_2},
{     "jmpix", 0, 0xf8040000U, 0x08000000U, FT32_FLD_CR|FT32_FLD_CB|FT32_FLD_CV|FT32_FLD_R_2},
{      "ashl", 1, 0xf800000fU, 0x40000008U, FT32_FLD_R_D|FT32_FLD_R_1|FT32_FLD_RIMM},
{      "jmpc", 0, 0xfe340000U, 0x00200000U, FT32_FLD_CBCRCV|FT32_FLD_PA},
{      "jmpx", 0, 0xf8040000U, 0x00000000U, FT32_FLD_CR|FT32_FLD_CB|FT32_FLD_CV|FT32_FLD_PA},
{       "lpm", 1, 0xf8000000U, 0x68000000U, FT32_FLD_R_D|FT32_FLD_PA},
{"streamouti", 1, 0xf800000fU, 0xf000000fU, FT32_FLD_R_D|FT32_FLD_R_1|FT32_FLD_RIMM},
{      "halt", 0, 0xf8000000U, 0x78000000U, 0},
{ NULL, 0, 0, 0, 0}
};
