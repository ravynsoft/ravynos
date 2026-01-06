/*
 * The 88110 implemention assumes signed immediate mode.
 */

/* m88k-opcode.h -- Instruction information for the 88100
   Copyright (C) 1989 Free Software Foundation, Inc.

This file is not yet part of GAS, the GNU Assembler.

GAS is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 1, or (at your option)
any later version.

GAS is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GAS; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* macros for shifting operand fields */

#define UEXT(src,off,wid)  ((((unsigned int)src)>>off) & ((1<<wid) - 1))
#define SEXT(src,off,wid)  (((((int)src)<<(32-(off+wid))) >>(32-wid)) )

/* operand decriptor structure */

struct opspec {
   unsigned int offset:5;
   unsigned int width:6;
   unsigned int type:5;
};

/* operands types */

#define NIL	     0    /* invalid parameter */
#define CNST         1    /* 16-bit constant */
#define REG          2    /* register */
#define BF           4    /* bit field */
#define REGSC        5    /* scaled register */
#define CRREG        6    /* control register */
#define FCRREG       7    /* floating point control register */
#define PCREL	     8    /* PC relative (branch offset) */
#define CONDMASK     9    /* bcnd mask */
#define CMPRSLT     10	  /* result of cmp instruction (bb0/1, tb0/1) */
#define ROT         11	  /* 5 bit value for rotation (rot instruction only)*/
#ifdef m88110
#define E4ROT       12	  /* even multiple of 4 value for pixel rotation */
#define EREG        13    /* even register from general register */
#define XREG        14    /* register from extended register file */
#endif /* m88110 */

/* instruction descriptor structure */

struct m88k_opcode {
   unsigned int   opcode;
   char           *name;
   struct opspec  op[3];
#ifdef NeXT_MOD
   int32_t	  delay_slot;
#endif
};

/* and introducing... the Motorola 88100 instruction set... */
   
static const struct m88k_opcode m88k_opcodes[] =   {
 
/*  Opcode     Mnemonic         Op 1 Spec        Op 2 Spec     Op 3 Spec */
                            
 { 0x70000000, "add",         { {21,5,REG},      {16,5,REG},   {0,16,CNST} } },
 { 0xf4007000, "add",         { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0xf4007200, "add.ci",      { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0xf4007300, "add.cio",     { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0xf4007100, "add.co",      { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0x60000000, "addu",        { {21,5,REG},      {16,5,REG},   {0,16,CNST} } },
 { 0xf4006000, "addu",        { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0xf4006200, "addu.ci",     { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0xf4006300, "addu.cio",    { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0xf4006100, "addu.co",     { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0x40000000, "and",         { {21,5,REG},      {16,5,REG},   {0,16,CNST} } },
 { 0xf4004000, "and",         { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0xf4004400, "and.c",       { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0x44000000, "and.u",       { {21,5,REG},      {16,5,REG},   {0,16,CNST} } },
 { 0xd0000000, "bb0",         { {21,5,CMPRSLT},  {16,5,REG},   {0,16,PCREL}} },
 { 0xd4000000, "bb0.n",       { {21,5,CMPRSLT},  {16,5,REG},   {0,16,PCREL}},1},
 { 0xd8000000, "bb1",         { {21,5,CMPRSLT},  {16,5,REG},   {0,16,PCREL}} },
 { 0xdc000000, "bb1.n",       { {21,5,CMPRSLT},  {16,5,REG},   {0,16,PCREL}},1},
 { 0xe8000000, "bcnd",        { {21,5,CONDMASK}, {16,5,REG},   {0,16,PCREL}} },
 { 0xec000000, "bcnd.n",      { {21,5,CONDMASK}, {16,5,REG},   {0,16,PCREL}},1},
 { 0xc0000000, "br",          { {0,26,PCREL},    {0,0,NIL},    {0,0,NIL}   } },
 { 0xc4000000, "br.n",        { {0,26,PCREL},    {0,0,NIL},    {0,0,NIL}   },1},
 { 0xc8000000, "bsr",         { {0,26,PCREL},    {0,0,NIL},    {0,0,NIL}   } },
 { 0xcc000000, "bsr.n",       { {0,26,PCREL},    {0,0,NIL},    {0,0,NIL}   },1},
 { 0XF0008000, "clr",         { {21,5,REG},      {16,5,REG},   {0,10,BF}   } },
 { 0XF4008000, "clr",         { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0x7c000000, "cmp",         { {21,5,REG},      {16,5,REG},   {0,16,CNST} } },
 { 0xf4007c00, "cmp",         { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0x78000000, "div",         { {21,5,REG},      {16,5,REG},   {0,16,CNST} } },
 { 0xf4007800, "div",         { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0x78000000, "divs",        { {21,5,REG},      {16,5,REG},   {0,16,CNST} } },
 { 0xf4007800, "divs",        { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#endif /* m88110 */
 { 0x68000000, "divu",        { {21,5,REG},      {16,5,REG},   {0,16,CNST} } },
 { 0xf4006800, "divu",        { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0xf4006900, "divu.d",      { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#endif /* m88110 */
 { 0XF0009000, "ext",         { {21,5,REG},      {16,5,REG},   {0,10,BF}   } },
 { 0XF4009000, "ext",         { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0XF0009800, "extu",        { {21,5,REG},      {16,5,REG},   {0,10,BF}   } },
 { 0XF4009800, "extu",        { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0X84002AA0, "fadd.ddd",    { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0X8400AAA0, "fadd.ddd",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
#endif /* m88110 */
 { 0X84002A20, "fadd.dds",    { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0X8400AA20, "fadd.dds",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400AB20, "fadd.ddx",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
#endif /* m88110 */
 { 0X840028A0, "fadd.dsd",    { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0X8400A8A0, "fadd.dsd",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
#endif /* m88110 */
 { 0X84002820, "fadd.dss",    { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0X8400A820, "fadd.dss",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400A920, "fadd.dsx",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400ACA0, "fadd.dxd",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400AC20, "fadd.dxs",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400AD20, "fadd.dxx",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
#endif /* m88110 */
 { 0X84002A80, "fadd.sdd",    { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0X8400AA80, "fadd.sdd",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
#endif /* m88110 */
 { 0X84002A00, "fadd.sds",    { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0X8400AA00, "fadd.sds",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400AB00, "fadd.sdx",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
#endif /* m88110 */
 { 0X84002880, "fadd.ssd",    { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0X8400A880, "fadd.ssd",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
#endif /* m88110 */
 { 0X84002800, "fadd.sss",    { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0X8400A800, "fadd.sss",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400A900, "fadd.ssx",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400AC80, "fadd.sxd",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400AC00, "fadd.sxs",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400AD00, "fadd.sxx",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400AAC0, "fadd.xdd",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400AA40, "fadd.xds",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400AB40, "fadd.xdx",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400A8C0, "fadd.xsd",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400A840, "fadd.xss",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400A940, "fadd.xsx",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400ACC0, "fadd.xxd",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400AC40, "fadd.xxs",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400AD40, "fadd.xxx",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
#endif /* m88110 */
 { 0x84003a80, "fcmp.sdd",    { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0X8400BA80, "fcmp.sdd",    { {21,5,REG},      {16,5,XREG},  {0,5,XREG}  } },
#endif /* m88110 */
 { 0x84003a00, "fcmp.sds",    { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0X8400BA00, "fcmp.sds",    { {21,5,REG},      {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400BB00, "fcmp.sdx",    { {21,5,REG},      {16,5,XREG},  {0,5,XREG}  } },
#endif /* m88110 */
 { 0x84003880, "fcmp.ssd",    { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0X8400B880, "fcmp.ssd",    { {21,5,REG},      {16,5,XREG},  {0,5,XREG}  } },
#endif /* m88110 */
 { 0x84003800, "fcmp.sss",    { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0X8400B800, "fcmp.sss",    { {21,5,REG},      {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400B900, "fcmp.ssx",    { {21,5,REG},      {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400BC80, "fcmp.sxd",    { {21,5,REG},      {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400BC00, "fcmp.sxs",    { {21,5,REG},      {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400BD00, "fcmp.sxx",    { {21,5,REG},      {16,5,XREG},  {0,5,XREG}  } },
 { 0X84003AA0, "fcmpu.sdd",   { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0X8400BAA0, "fcmpu.sdd",   { {21,5,REG},      {16,5,XREG},  {0,5,XREG}  } },
 { 0X84003A20, "fcmpu.sds",   { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0X8400BA20, "fcmpu.sds",   { {21,5,REG},      {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400BB20, "fcmpu.sdx",   { {21,5,REG},      {16,5,XREG},  {0,5,XREG}  } },
 { 0X840038A0, "fcmpu.ssd",   { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0X8400B8A0, "fcmpu.ssd",   { {21,5,REG},      {16,5,XREG},  {0,5,XREG}  } },
 { 0X84003820, "fcmpu.sss",   { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0X8400B820, "fcmpu.sss",   { {21,5,REG},      {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400B920, "fcmpu.ssx",   { {21,5,REG},      {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400BCA0, "fcmpu.sxd",   { {21,5,REG},      {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400BC20, "fcmpu.sxs",   { {21,5,REG},      {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400BD20, "fcmpu.sxx",   { {21,5,REG},      {16,5,XREG},  {0,5,XREG}  } },
 { 0X84000820, "fcvt.ds",     { {21,5,REG},      {0,5,REG},    {0,0,NIL}   } },
 { 0X84008820, "fcvt.ds",     { {21,5,XREG},     {0,5,XREG},   {0,0,NIL}   } },
 { 0X84008920, "fcvt.dx",     { {21,5,XREG},     {0,5,XREG},   {0,0,NIL}   } },
 { 0X84000880, "fcvt.sd",     { {21,5,REG},      {0,5,REG},    {0,0,NIL}   } },
 { 0X84008880, "fcvt.sd",     { {21,5,XREG},     {0,5,XREG},   {0,0,NIL}   } },
 { 0X84008900, "fcvt.sx",     { {21,5,XREG},     {0,5,XREG},   {0,0,NIL}   } },
 { 0X840088C0, "fcvt.xd",     { {21,5,XREG},     {0,5,XREG},   {0,0,NIL}   } },
 { 0X84008840, "fcvt.xs",     { {21,5,XREG},     {0,5,XREG},   {0,0,NIL}   } },
#endif /* m88110 */
 { 0x840072a0, "fdiv.ddd",    { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0x8400f2a0, "fdiv.ddd",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
#endif /* m88110 */
 { 0x84007220, "fdiv.dds",    { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0x8400f220, "fdiv.dds",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0x8400f320, "fdiv.ddx",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
#endif /* m88110 */
 { 0x840070a0, "fdiv.dsd",    { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0x8400f0a0, "fdiv.dsd",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
#endif /* m88110 */
 { 0x84007020, "fdiv.dss",    { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0x8400f020, "fdiv.dss",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0x8400f120, "fdiv.dsx",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0x8400f4a0, "fdiv.dxd",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0x8400f420, "fdiv.dxs",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0x8400f520, "fdiv.dxx",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
#endif /* m88110 */
 { 0x84007280, "fdiv.sdd",    { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0x8400f280, "fdiv.sdd",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
#endif /* m88110 */
 { 0x84007200, "fdiv.sds",    { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0x8400f200, "fdiv.sds",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0x8400f300, "fdiv.sdx",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
#endif /* m88110 */
 { 0x84007080, "fdiv.ssd",    { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0x8400f080, "fdiv.ssd",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
#endif /* m88110 */
 { 0X84007000, "fdiv.sss",    { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0x8400f000, "fdiv.sss",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0x8400f100, "fdiv.ssx",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0x8400f480, "fdiv.sxd",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0x8400f400, "fdiv.sxs",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0x8400f500, "fdiv.sxx",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0x8400f2c0, "fdiv.xdd",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0x8400f240, "fdiv.xds",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0x8400f340, "fdiv.xdx",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0x8400f0c0, "fdiv.xsd",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0x8400f040, "fdiv.xss",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0x8400f140, "fdiv.xsx",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0x8400f4c0, "fdiv.xxd",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0x8400f440, "fdiv.xxs",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0x8400f540, "fdiv.xxx",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
#endif /* m88110 */
 { 0XF400EC00, "ff0",         { {21,5,REG},      {0,5,REG},    {0,0,NIL}   } },
 { 0XF400E800, "ff1",         { {21,5,REG},      {0,5,REG},    {0,0,NIL}   } },
 { 0x80004800, "fldcr",       { {21,5,REG},      {5,6,FCRREG}, {0,0,NIL}   } },
 { 0X84002020, "flt.ds",      { {21,5,REG},      {0,5,REG},    {0,0,NIL}   } },
#ifdef m88110
 { 0X84002220, "flt.ds",      { {21,5,XREG},     {0,5,REG},    {0,0,NIL}   } },
#endif /* m88110 */
 { 0X84002000, "flt.ss",      { {21,5,REG},      {0,5,REG},    {0,0,NIL}   } },
#ifdef m88110
 { 0X84002200, "flt.ss",      { {21,5,XREG},     {0,5,REG},    {0,0,NIL}   } },
 { 0X84002240, "flt.xs",      { {21,5,XREG},     {0,5,REG},    {0,0,NIL}   } },
#endif /* m88110 */
 { 0X840002A0, "fmul.ddd",    { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0X840082A0, "fmul.ddd",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
#endif /* m88110 */
 { 0X84000220, "fmul.dds",    { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0X84008220, "fmul.dds",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X84008320, "fmul.ddx",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
#endif /* m88110 */
 { 0X840000A0, "fmul.dsd",    { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0X840080A0, "fmul.dsd",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
#endif /* m88110 */
 { 0X84000020, "fmul.dss",    { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0X84008020, "fmul.dss",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X84008120, "fmul.dsx",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X840084A0, "fmul.dxd",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X84008420, "fmul.dxs",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X84008520, "fmul.dxx",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
#endif /* m88110 */
 { 0X84000280, "fmul.sdd",    { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0X84008280, "fmul.sdd",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
#endif /* m88110 */
 { 0X84000200, "fmul.sds",    { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0X84008200, "fmul.sds",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X84008300, "fmul.sdx",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
#endif /* m88110 */
 { 0X84000080, "fmul.ssd",    { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0X84008080, "fmul.ssd",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
#endif /* m88110 */
 { 0X84000000, "fmul.sss",    { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0X84008000, "fmul.sss",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X84008100, "fmul.ssx",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X84008480, "fmul.sxd",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X84008400, "fmul.sxs",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X84008500, "fmul.sxx",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X840082C0, "fmul.xdd",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X84008240, "fmul.xds",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X84008340, "fmul.xdx",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X840080C0, "fmul.xsd",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X84008040, "fmul.xss",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X84008140, "fmul.xsx",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X840084C0, "fmul.xxd",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X84008440, "fmul.xxs",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X84008540, "fmul.xxx",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X840078A0, "fsqrt.dd",    { {21,5,REG},      {0,5,REG},    {0,0,NIL}   } },
 { 0X8400F8A0, "fsqrt.dd",    { {21,5,XREG},     {0,5,XREG},   {0,0,NIL}   } },
 { 0X84007820, "fsqrt.ds",    { {21,5,REG},      {0,5,REG},    {0,0,NIL}   } },
 { 0X8400F820, "fsqrt.ds",    { {21,5,XREG},     {0,5,XREG},   {0,0,NIL}   } },
 { 0X8400F920, "fsqrt.dx",    { {21,5,XREG},     {0,5,XREG},   {0,0,NIL}   } },
 { 0X84007880, "fsqrt.sd",    { {21,5,REG},      {0,5,REG},    {0,0,NIL}   } },
 { 0X8400F880, "fsqrt.sd",    { {21,5,XREG},     {0,5,XREG},   {0,0,NIL}   } },
 { 0X84007800, "fsqrt.ss",    { {21,5,REG},      {0,5,REG},    {0,0,NIL}   } },
 { 0X8400F800, "fsqrt.ss",    { {21,5,XREG},     {0,5,XREG},   {0,0,NIL}   } },
 { 0X8400F900, "fsqrt.sx",    { {21,5,XREG},     {0,5,XREG},   {0,0,NIL}   } },
 { 0X8400F8C0, "fsqrt.xd",    { {21,5,XREG},     {0,5,XREG},   {0,0,NIL}   } },
 { 0X8400F840, "fsqrt.xs",    { {21,5,XREG},     {0,5,XREG},   {0,0,NIL}   } },
 { 0X8400F940, "fsqrt.xx",    { {21,5,XREG},     {0,5,XREG},   {0,0,NIL}   } },
#endif /* m88110 */
 { 0x80008800, "fstcr",       { {16,5,REG},      {5,6,FCRREG}, {0,0,NIL}   } },
 { 0X840032A0, "fsub.ddd",    { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0X8400B2A0, "fsub.ddd",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
#endif /* m88110 */
 { 0X84003220, "fsub.dds",    { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0X8400B220, "fsub.dds",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400B320, "fsub.ddx",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
#endif /* m88110 */
 { 0X840030A0, "fsub.dsd",    { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0X8400B0A0, "fsub.dsd",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
#endif /* m88110 */
 { 0X84003020, "fsub.dss",    { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0X8400B020, "fsub.dss",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400B120, "fsub.dsx",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400B4A0, "fsub.dxd",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400B420, "fsub.dxs",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400B520, "fsub.dxx",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
#endif /* m88110 */
 { 0X84003280, "fsub.sdd",    { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0X8400B280, "fsub.sdd",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
#endif /* m88110 */
 { 0X84003200, "fsub.sds",    { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0X8400B200, "fsub.sds",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400B300, "fsub.sdx",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
#endif /* m88110 */
 { 0X84003080, "fsub.ssd",    { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0X8400B080, "fsub.ssd",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
#endif /* m88110 */
 { 0X84003000, "fsub.sss",    { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0X8400B000, "fsub.sss",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400B100, "fsub.ssx",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400B480, "fsub.sxd",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400B400, "fsub.sxs",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400B500, "fsub.sxx",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400B2C0, "fsub.xdd",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400B240, "fsub.xds",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400B340, "fsub.xdx",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400B0C0, "fsub.xsd",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400B040, "fsub.xss",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400B140, "fsub.xsx",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400B4C0, "fsub.xxd",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400B440, "fsub.xxs",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
 { 0X8400B540, "fsub.xxx",    { {21,5,XREG},     {16,5,XREG},  {0,5,XREG}  } },
#endif /* m88110 */
 { 0x8000c800, "fxcr",        { {21,5,REG},      {16,5,REG},   {5,6,FCRREG}} },
 { 0xf400fc01, "illop1",      { {0,0,NIL},       {0,0,NIL},    {0,0,NIL}   } },
 { 0xf400fc02, "illop2",      { {0,0,NIL},       {0,0,NIL},    {0,0,NIL}   } },
 { 0xf400fc03, "illop3",      { {0,0,NIL},       {0,0,NIL},    {0,0,NIL}   } },
 { 0x84004880, "int.sd",      { {21,5,REG},      {0,5,REG},    {0,0,NIL}   } },
#ifdef m88110
 { 0x8400c880, "int.sd",      { {21,5,REG},      {0,5,XREG},   {0,0,NIL}   } },
#endif /* m88110 */
 { 0x84004800, "int.ss",      { {21,5,REG},      {0,5,REG},    {0,0,NIL}   } },
#ifdef m88110
 { 0x8400c800, "int.ss",      { {21,5,REG},      {0,5,XREG},   {0,0,NIL}   } },
 { 0x8400c900, "int.sx",      { {21,5,REG},      {0,5,XREG},   {0,0,NIL}   } },
#endif /* m88110 */
 { 0xf400c000, "jmp",         { {0,5,REG},       {0,0,NIL},    {0,0,NIL}   } },
 { 0xf400c400, "jmp.n",       { {0,5,REG},       {0,0,NIL},    {0,0,NIL}   },1},
 { 0xf400c800, "jsr",         { {0,5,REG},       {0,0,NIL},    {0,0,NIL}   } },
 { 0xf400cc00, "jsr.n",       { {0,5,REG},       {0,0,NIL},    {0,0,NIL}   },1},
 { 0x14000000, "ld",          { {21,5,REG},      {16,5,REG},   {0,16,CNST} } },
 { 0xf4001600, "ld",          { {21,5,REG},      {16,5,REG},   {0,5,REGSC} } },
 { 0xf4001400, "ld",          { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0x04000000, "ld",          { {21,5,XREG},     {16,5,REG},   {0,16,CNST} } },
 { 0xf0001600, "ld",          { {21,5,XREG},     {16,5,REG},   {0,5,REGSC} } },
 { 0xf0001400, "ld",          { {21,5,XREG},     {16,5,REG},   {0,5,REG}   } },
#endif /* m88110 */
 { 0x1c000000, "ld.b",        { {21,5,REG},      {16,5,REG},   {0,16,CNST} } },
 { 0xf4001e00, "ld.b",        { {21,5,REG},      {16,5,REG},   {0,5,REGSC} } },
 { 0xf4001c00, "ld.b",        { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0xf4001f00, "ld.b.usr",    { {21,5,REG},      {16,5,REG},   {0,5,REGSC} } },
 { 0xf4001d00, "ld.b.usr",    { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0x0c000000, "ld.bu",       { {21,5,REG},      {16,5,REG},   {0,16,CNST} } },
 { 0xf4000e00, "ld.bu",       { {21,5,REG},      {16,5,REG},   {0,5,REGSC} } },
 { 0xf4000c00, "ld.bu",       { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0xf4000f00, "ld.bu.usr",   { {21,5,REG},      {16,5,REG},   {0,5,REGSC} } },
 { 0xf4000d00, "ld.bu.usr",   { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0x10000000, "ld.d",        { {21,5,REG},      {16,5,REG},   {0,16,CNST} } },
 { 0xf4001200, "ld.d",        { {21,5,REG},      {16,5,REG},   {0,5,REGSC} } },
 { 0xf4001000, "ld.d",        { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0x00000000, "ld.d",        { {21,5,XREG},     {16,5,REG},   {0,16,CNST} } },
 { 0xf0001200, "ld.d",        { {21,5,XREG},     {16,5,REG},   {0,5,REGSC} } },
 { 0xf0001000, "ld.d",        { {21,5,XREG},     {16,5,REG},   {0,5,REG}   } },
#endif /* m88110 */
 { 0xf4001300, "ld.d.usr",    { {21,5,REG},      {16,5,REG},   {0,5,REGSC} } },
#ifdef m88110
 { 0xf0001300, "ld.d.usr",    { {21,5,XREG},     {16,5,REG},   {0,5,REGSC} } },
#endif /* m88110 */
 { 0xf4001100, "ld.d.usr",    { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0xf0001100, "ld.d.usr",    { {21,5,XREG},     {16,5,REG},   {0,5,REG}   } },
#endif /* m88110 */
 { 0x18000000, "ld.h",        { {21,5,REG},      {16,5,REG},   {0,16,CNST} } },
 { 0xf4001a00, "ld.h",        { {21,5,REG},      {16,5,REG},   {0,5,REGSC} } },
 { 0xf4001800, "ld.h",        { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0xf4001b00, "ld.h.usr",    { {21,5,REG},      {16,5,REG},   {0,5,REGSC} } },
 { 0xf4001900, "ld.h.usr",    { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0x08000000, "ld.hu",       { {21,5,REG},      {16,5,REG},   {0,16,CNST} } },
 { 0xf4000a00, "ld.hu",       { {21,5,REG},      {16,5,REG},   {0,5,REGSC} } },
 { 0xf4000800, "ld.hu",       { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0xf4000b00, "ld.hu.usr",   { {21,5,REG},      {16,5,REG},   {0,5,REGSC} } },
 { 0xf4000900, "ld.hu.usr",   { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0xf4001700, "ld.usr",      { {21,5,REG},      {16,5,REG},   {0,5,REGSC} } },
#ifdef m88110
 { 0xf0001700, "ld.usr",      { {21,5,XREG},     {16,5,REG},   {0,5,REGSC} } },
#endif /* m88110 */
 { 0xf4001500, "ld.usr",      { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0xf0001500, "ld.usr",      { {21,5,XREG},     {16,5,REG},   {0,5,REG}   } },
 { 0x3c000000, "ld.x",        { {21,5,XREG},     {16,5,REG},   {0,16,CNST} } },
 { 0xf0001a00, "ld.x",        { {21,5,XREG},     {16,5,REG},   {0,5,REGSC} } },
 { 0xf0001800, "ld.x",        { {21,5,XREG},     {16,5,REG},   {0,5,REG}   } },
 { 0xf0001b00, "ld.x.usr",    { {21,5,XREG},     {16,5,REG},   {0,5,REGSC} } },
 { 0xf0001900, "ld.x.usr",    { {21,5,XREG},     {16,5,REG},   {0,5,REG}   } },
#endif /* m88110 */
#ifndef m88110
 { 0x34000000, "lda",         { {21,5,REG},      {16,5,REG},   {0,16,CNST} } },
#endif /* !defined(m88110) */
 { 0xf4003600, "lda",         { {21,5,REG},      {16,5,REG},   {0,5,REGSC} } },
#ifndef m88110
 { 0xf4003400, "lda",         { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0x3c000000, "lda.b",       { {21,5,REG},      {16,5,REG},   {0,16,CNST} } },
 { 0xf4003e00, "lda.b",       { {21,5,REG},      {16,5,REG},   {0,5,REGSC} } },
 { 0xf4003c00, "lda.b",       { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0x30000000, "lda.d",       { {21,5,REG},      {16,5,REG},   {0,16,CNST} } },
#endif /* !defined(m88110) */
 { 0xf4003200, "lda.d",       { {21,5,REG},      {16,5,REG},   {0,5,REGSC} } },
#ifndef m88110
 { 0xf4003000, "lda.d",       { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0x38000000, "lda.h",       { {21,5,REG},      {16,5,REG},   {0,16,CNST} } },
#endif /* !defined(m88110) */
 { 0xf4003a00, "lda.h",       { {21,5,REG},      {16,5,REG},   {0,5,REGSC} } },
#ifndef m88110
 { 0xf4003800, "lda.h",       { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#endif /* !defined(m88110) */
#ifdef m88110
 { 0xf4003e00, "lda.x",       { {21,5,REG},      {16,5,REG},   {0,5,REGSC} } },
#endif /* m88110 */
 { 0x80004000, "ldcr",        { {21,5,REG},      {5,6,CRREG},  {0,0,NIL}   } },
 { 0XF000A000, "mak",         { {21,5,REG},      {16,5,REG},   {0,10,BF}   } },
 { 0XF400A000, "mak",         { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0X48000000, "mask",        { {21,5,REG},      {16,5,REG},   {0,16,CNST} } },
 { 0X4C000000, "mask.u",      { {21,5,REG},      {16,5,REG},   {0,16,CNST} } },
#ifdef m88110
 { 0X8400C080, "mov.d",       { {21,5,REG},      {0,5,XREG},   {0,0,NIL}   } },
 { 0X84004280, "mov.d",       { {21,5,XREG},     {0,5,REG},    {0,0,NIL}   } },
 { 0X8400C000, "mov.s",       { {21,5,REG},      {0,5,XREG},   {0,0,NIL}   } },
 { 0X84004200, "mov.s",       { {21,5,XREG},     {0,5,REG},    {0,0,NIL}   } },
 { 0X8400C300, "mov",         { {21,5,XREG},     {0,5,XREG},   {0,0,NIL}   } },
#endif /* m88110 */
 { 0x6c000000, "mul",         { {21,5,REG},      {16,5,REG},   {0,16,CNST} } },
 { 0xf4006c00, "mul",         { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0xf4006e00, "muls",        { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0x6c000000, "mulu",        { {21,5,REG},      {16,5,REG},   {0,16,CNST} } },
 { 0xf4006c00, "mulu",        { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0xf4006d00, "mulu.d",      { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#endif /* m88110 */
 { 0x84005080, "nint.sd",     { {21,5,REG},      {0,5,REG},    {0,0,NIL}   } },
#ifdef m88110
 { 0x8400d080, "nint.sd",     { {21,5,REG},      {0,5,XREG},   {0,0,NIL}   } },
#endif /* m88110 */
 { 0x84005000, "nint.ss",     { {21,5,REG},      {0,5,REG},    {0,0,NIL}   } },
#ifdef m88110
 { 0x8400d000, "nint.ss",     { {21,5,REG},      {0,5,XREG},   {0,0,NIL}   } },
 { 0x8400d100, "nint.sx",     { {21,5,REG},      {0,5,XREG},   {0,0,NIL}   } },
#endif /* m88110 */
 { 0X58000000, "or",          { {21,5,REG},      {16,5,REG},   {0,16,CNST} } },
 { 0xf4005800, "or",          { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0xf4005c00, "or.c",        { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0X5C000000, "or.u",        { {21,5,REG},      {16,5,REG},   {0,16,CNST} } },
#ifdef m88110
 { 0x88002060, "padd",        { {21,5,EREG},     {16,5,EREG},  {0,5,EREG}  } },
 { 0x88002020, "padd.b",      { {21,5,EREG},     {16,5,EREG},  {0,5,EREG}  } },
 { 0x88002040, "padd.h",      { {21,5,EREG},     {16,5,EREG},  {0,5,EREG}  } },
 { 0x880021e0, "padds.s",     { {21,5,EREG},     {16,5,EREG},  {0,5,EREG}  } },
 { 0x880021a0, "padds.s.b",   { {21,5,EREG},     {16,5,EREG},  {0,5,EREG}  } },
 { 0x880021c0, "padds.s.h",   { {21,5,EREG},     {16,5,EREG},  {0,5,EREG}  } },
 { 0x880020e0, "padds.u",     { {21,5,EREG},     {16,5,EREG},  {0,5,EREG}  } },
 { 0x880020a0, "padds.u.b",   { {21,5,EREG},     {16,5,EREG},  {0,5,EREG}  } },
 { 0x880020c0, "padds.u.h",   { {21,5,EREG},     {16,5,EREG},  {0,5,EREG}  } },
 { 0x88002160, "padds.us",    { {21,5,EREG},     {16,5,EREG},  {0,5,EREG}  } },
 { 0x88002120, "padds.us.b",  { {21,5,EREG},     {16,5,EREG},  {0,5,EREG}  } },
 { 0x88002140, "padds.us.h",  { {21,5,EREG},     {16,5,EREG},  {0,5,EREG}  } },
 { 0x88003860, "pcmp",        { {21,5,REG},      {16,5,EREG},  {0,5,EREG}  } },
 { 0x88000000, "pmul",        { {21,5,EREG},     {16,5,EREG},  {0,5,REG}   } },
 { 0x88006260, "ppack.16",    { {21,5,EREG},     {16,5,EREG},  {0,5,EREG}  } },
 { 0x88006240, "ppack.16.h",  { {21,5,EREG},     {16,5,EREG},  {0,5,EREG}  } },
 { 0x88006460, "ppack.32",    { {21,5,EREG},     {16,5,EREG},  {0,5,EREG}  } },
 { 0x88006420, "ppack.32.b",  { {21,5,EREG},     {16,5,EREG},  {0,5,EREG}  } },
 { 0x88006440, "ppack.32.h",  { {21,5,EREG},     {16,5,EREG},  {0,5,EREG}  } },
 { 0x88006160, "ppack.8",     { {21,5,EREG},     {16,5,EREG},  {0,5,EREG}  } },
 { 0x88007800, "prot",        { {21,5,EREG},     {16,5,EREG},  {0,5,EREG}  } },
 { 0x88007000, "prot",        { {21,5,EREG},     {16,5,EREG},  {7,4,E4ROT} } },
 { 0x88003060, "psub",        { {21,5,EREG},     {16,5,EREG},  {0,5,EREG}  } },
 { 0x88003020, "psub.b",      { {21,5,EREG},     {16,5,EREG},  {0,5,EREG}  } },
 { 0x88003040, "psub.h",      { {21,5,EREG},     {16,5,EREG},  {0,5,EREG}  } },
 { 0x880031e0, "psubs.s",     { {21,5,EREG},     {16,5,EREG},  {0,5,EREG}  } },
 { 0x880031a0, "psubs.s.b",   { {21,5,EREG},     {16,5,EREG},  {0,5,EREG}  } },
 { 0x880031c0, "psubs.s.h",   { {21,5,EREG},     {16,5,EREG},  {0,5,EREG}  } },
 { 0x880030e0, "psubs.u",     { {21,5,EREG},     {16,5,EREG},  {0,5,EREG}  } },
 { 0x880030a0, "psubs.u.b",   { {21,5,EREG},     {16,5,EREG},  {0,5,EREG}  } },
 { 0x880030c0, "psubs.u.h",   { {21,5,EREG},     {16,5,EREG},  {0,5,EREG}  } },
 { 0x88003160, "psubs.us",    { {21,5,EREG},     {16,5,EREG},  {0,5,EREG}  } },
 { 0x88003120, "psubs.us.b",  { {21,5,EREG},     {16,5,EREG},  {0,5,EREG}  } },
 { 0x88003140, "psubs.us.h",  { {21,5,EREG},     {16,5,EREG},  {0,5,EREG}  } },
 { 0x88006820, "punpk.b",     { {21,5,EREG},     {16,5,REG},   {0,0,NIL}   } },
 { 0x88006840, "punpk.h",     { {21,5,EREG},     {16,5,REG},   {0,0,NIL}   } },
 { 0x88006800, "punpk.n",     { {21,5,EREG},     {16,5,REG},   {0,0,NIL}   } },
#endif /* m88110 */
 { 0XF000A800, "rot",         { {21,5,REG},      {16,5,REG},   {0,5,ROT}   } },
 { 0XF400A800, "rot",         { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0xf400fc00, "rte",         { {0,0,NIL},       {0,0,NIL},    {0,0,NIL}   } },
 { 0XF0008800, "set",         { {21,5,REG},      {16,5,REG},   {0,10,BF}   } },
 { 0XF4008800, "set",         { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0x24000000, "st",          { {21,5,REG},      {16,5,REG},   {0,16,CNST} } },
 { 0xf4002600, "st",          { {21,5,REG},      {16,5,REG},   {0,5,REGSC} } },
 { 0xf4002400, "st",          { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0x34000000, "st",          { {21,5,XREG},     {16,5,REG},   {0,16,CNST} } },
 { 0xf0002600, "st",          { {21,5,XREG},     {16,5,REG},   {0,5,REGSC} } },
 { 0xf0002400, "st",          { {21,5,XREG},     {16,5,REG},   {0,5,REG}   } },
 { 0xf4002680, "st.wt",       { {21,5,REG},      {16,5,REG},   {0,5,REGSC} } },
 { 0xf4002480, "st.wt",       { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0xf0002680, "st.wt",       { {21,5,XREG},     {16,5,REG},   {0,5,REGSC} } },
 { 0xf0002480, "st.wt",       { {21,5,XREG},     {16,5,REG},   {0,5,REG}   } },
#endif /* m88110 */
 { 0x2c000000, "st.b",        { {21,5,REG},      {16,5,REG},   {0,16,CNST} } },
 { 0xf4002e00, "st.b",        { {21,5,REG},      {16,5,REG},   {0,5,REGSC} } },
 { 0xf4002c00, "st.b",        { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0xf4002e80, "st.b.wt",     { {21,5,REG},      {16,5,REG},   {0,5,REGSC} } },
 { 0xf4002c80, "st.b.wt",     { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#endif /* m88110 */
 { 0xf4002f00, "st.b.usr",    { {21,5,REG},      {16,5,REG},   {0,5,REGSC} } },
 { 0xf4002d00, "st.b.usr",    { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0xf4002f80, "st.b.usr.wt", { {21,5,REG},      {16,5,REG},   {0,5,REGSC} } },
 { 0xf4002d80, "st.b.usr.wt", { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#endif /* m88110 */
 { 0x20000000, "st.d",        { {21,5,REG},      {16,5,REG},   {0,16,CNST} } },
 { 0xf4002200, "st.d",        { {21,5,REG},      {16,5,REG},   {0,5,REGSC} } },
 { 0xf4002000, "st.d",        { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0x30000000, "st.d",        { {21,5,XREG},     {16,5,REG},   {0,16,CNST} } },
 { 0xf0002200, "st.d",        { {21,5,XREG},     {16,5,REG},   {0,5,REGSC} } },
 { 0xf0002000, "st.d",        { {21,5,XREG},     {16,5,REG},   {0,5,REG}   } },
 { 0xf4002280, "st.d.wt",     { {21,5,REG},      {16,5,REG},   {0,5,REGSC} } },
 { 0xf4002080, "st.d.wt",     { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0xf0002280, "st.d.wt",     { {21,5,XREG},     {16,5,REG},   {0,5,REGSC} } },
 { 0xf0002080, "st.d.wt",     { {21,5,XREG},     {16,5,REG},   {0,5,REG}   } },
#endif /* m88110 */
 { 0xf4002300, "st.d.usr",    { {21,5,REG},      {16,5,REG},   {0,5,REGSC} } },
 { 0xf4002100, "st.d.usr",    { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0xf0002300, "st.d.usr",    { {21,5,XREG},     {16,5,REG},   {0,5,REGSC} } },
 { 0xf0002100, "st.d.usr",    { {21,5,XREG},     {16,5,REG},   {0,5,REG}   } },
 { 0xf4002380, "st.d.usr.wt", { {21,5,REG},      {16,5,REG},   {0,5,REGSC} } },
 { 0xf0002180, "st.d.usr.wt", { {21,5,XREG},     {16,5,REG},   {0,5,REG}   } },
 { 0xf0002380, "st.d.usr.wt", { {21,5,XREG},     {16,5,REG},   {0,5,REGSC} } },
 { 0xf4002180, "st.d.usr.wt", { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#endif /* m88110 */
 { 0x28000000, "st.h",        { {21,5,REG},      {16,5,REG},   {0,16,CNST} } },
 { 0xf4002a00, "st.h",        { {21,5,REG},      {16,5,REG},   {0,5,REGSC} } },
 { 0xf4002800, "st.h",        { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0xf4002a80, "st.h.wt",     { {21,5,REG},      {16,5,REG},   {0,5,REGSC} } },
 { 0xf4002880, "st.h.wt",     { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#endif /* m88110 */
 { 0xf4002b00, "st.h.usr",    { {21,5,REG},      {16,5,REG},   {0,5,REGSC} } },
 { 0xf4002900, "st.h.usr",    { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0xf4002b80, "st.h.usr.wt", { {21,5,REG},      {16,5,REG},   {0,5,REGSC} } },
 { 0xf4002980, "st.h.usr.wt", { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#endif /* m88110 */
 { 0xf4002700, "st.usr",      { {21,5,REG},      {16,5,REG},   {0,5,REGSC} } },
 { 0xf4002500, "st.usr",      { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifdef m88110
 { 0xf0002700, "st.usr",      { {21,5,XREG},     {16,5,REG},   {0,5,REGSC} } },
 { 0xf0002500, "st.usr",      { {21,5,XREG},     {16,5,REG},   {0,5,REG}   } },
 { 0xf4002780, "st.usr.wt",   { {21,5,REG},      {16,5,REG},   {0,5,REGSC} } },
 { 0xf4002580, "st.usr.wt",   { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0xf0002780, "st.usr.wt",   { {21,5,XREG},     {16,5,REG},   {0,5,REGSC} } },
 { 0xf0002580, "st.usr.wt",   { {21,5,XREG},     {16,5,REG},   {0,5,REG}   } },
 { 0x38000000, "st.x",        { {21,5,XREG},     {16,5,REG},   {0,16,CNST} } },
 { 0xf0002a00, "st.x",        { {21,5,XREG},     {16,5,REG},   {0,5,REGSC} } },
 { 0xf0002800, "st.x",        { {21,5,XREG},     {16,5,REG},   {0,5,REG}   } },
 { 0xf0002a80, "st.x.wt",     { {21,5,XREG},     {16,5,REG},   {0,5,REGSC} } },
 { 0xf0002880, "st.x.wt",     { {21,5,XREG},     {16,5,REG},   {0,5,REG}   } },
 { 0xf0002b00, "st.x.usr",    { {21,5,XREG},     {16,5,REG},   {0,5,REGSC} } },
 { 0xf0002900, "st.x.usr",    { {21,5,XREG},     {16,5,REG},   {0,5,REG}   } },
 { 0xf0002b80, "st.x.usr.wt", { {21,5,XREG},     {16,5,REG},   {0,5,REGSC} } },
 { 0xf0002980, "st.x.usr.wt", { {21,5,XREG},     {16,5,REG},   {0,5,REG}   } },
#endif /* m88110 */
 { 0x80008000, "stcr",        { {16,5,REG},      {5,6,CRREG},  {0,0,NIL}   } },
 { 0x74000000, "sub",         { {21,5,REG},      {16,5,REG},   {0,16,CNST} } },
 { 0xf4007400, "sub",         { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0xf4007600, "sub.ci",      { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0xf4007700, "sub.cio",     { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0xf4007500, "sub.co",      { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0x64000000, "subu",        { {21,5,REG},      {16,5,REG},   {0,16,CNST} } },
 { 0xf4006400, "subu",        { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0xf4006600, "subu.ci",     { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0xf4006700, "subu.cio",    { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0xf4006500, "subu.co",     { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0xf000d000, "tb0",         { {21,5,CMPRSLT},  {16,5,REG},   {0,9,CNST}  } },
 { 0xf000d800, "tb1",         { {21,5,CMPRSLT},  {16,5,REG},   {0,9,CNST}  } },
 { 0xf8000000, "tbnd",        { {16,5,REG},      {0,16,CNST},  {0,0,NIL}   } },
 { 0xf400f800, "tbnd",        { {16,5,REG},      {0,5,REG},    {0,0,NIL}   } },
 { 0xf000e800, "tcnd",        { {21,5,CONDMASK}, {16,5,REG},   {0,9,CNST}  } },
 { 0x84005880, "trnc.sd",     { {21,5,REG},      {0,5,REG},    {0,0,NIL}   } },
#ifdef m88110
 { 0x8400d880, "trnc.sd",     { {21,5,REG},      {0,5,XREG},   {0,0,NIL}   } },
#endif /* m88110 */
 { 0x84005800, "trnc.ss",     { {21,5,REG},      {0,5,REG},    {0,0,NIL}   } },
#ifdef m88110
 { 0x8400d800, "trnc.ss",     { {21,5,REG},      {0,5,XREG},   {0,0,NIL}   } },
 { 0x8400d900, "trnc.sx",     { {21,5,REG},      {0,5,XREG},   {0,0,NIL}   } },
#endif /* m88110 */
 { 0x8000c000, "xcr",         { {21,5,REG},      {16,5,REG},   {5,6,CRREG} } },
#ifndef m88110
 { 0x04000000, "xmem",        { {21,5,REG},      {16,5,REG},   {0,16,CNST} } },
#endif /* !defined(m88110) */
 { 0xf4000600, "xmem",        { {21,5,REG},      {16,5,REG},   {0,5,REGSC} } },
 { 0xf4000400, "xmem",        { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
#ifndef m88110
 { 0x00000000, "xmem.bu",     { {21,5,REG},      {16,5,REG},   {0,16,CNST} } },
#endif /* !defined(m88110) */
 { 0xf4000200, "xmem.bu",     { {21,5,REG},      {16,5,REG},   {0,5,REGSC} } },
 { 0xf4000000, "xmem.bu",     { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0xf4000300, "xmem.bu.usr", { {21,5,REG},      {16,5,REG},   {0,5,REGSC} } },
 { 0xf4000100, "xmem.bu.usr", { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0xf4000700, "xmem.usr",    { {21,5,REG},      {16,5,REG},   {0,5,REGSC} } },
 { 0xf4000500, "xmem.usr",    { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0X50000000, "xor",         { {21,5,REG},      {16,5,REG},   {0,16,CNST} } },
 { 0XF4005000, "xor",         { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0XF4005400, "xor.c",       { {21,5,REG},      {16,5,REG},   {0,5,REG}   } },
 { 0X54000000, "xor.u",       { {21,5,REG},      {16,5,REG},   {0,16,CNST} } },
 { 0x00000000, "",            { {0,0,NIL},       {0,0,NIL},    {0,0,NIL}   } },
};

#define NUMOPCODES ((sizeof m88k_opcodes)/(sizeof m88k_opcodes[0]))
