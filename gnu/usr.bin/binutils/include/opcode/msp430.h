/* Opcode table for the TI MSP430 microcontrollers

   Copyright (C) 2002-2023 Free Software Foundation, Inc.
   Contributed by Dmitry Diky <diwil@mail.ru>
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#ifndef __MSP430_H_
#define __MSP430_H_

enum msp430_expp_e
{
  MSP_EXPP_ALL = 0,	/* Use full the value of the expression - default.  */
  MSP_EXPP_LO,		/* Extract least significant word from expression.  */
  MSP_EXPP_HI,		/* Extract 2nd word from expression.  */
  MSP_EXPP_LLO,		/* Extract least significant word from an
			   immediate value.  */
  MSP_EXPP_LHI,		/* Extract 2nd word from an immediate value.  */
  MSP_EXPP_HLO,		/* Extract 3rd word from an immediate value.  */
  MSP_EXPP_HHI,		/* Extract 4th word from an immediate value.  */
};

struct msp430_operand_s
{
  int ol;	/* Operand length words.  */
  int am;	/* Addr mode.  */
  int reg;	/* Register.  */
  int mode;	/* Operand mode.  */
  int vshift;   /* Number of bytes to shift operand down.  */
  enum msp430_expp_e expp;	/* For when the operand is a constant
				   expression, the part of the expression to
				   extract.  */
#define OP_REG		0
#define OP_EXP		1
#ifndef DASM_SECTION
  expressionS	exp;
#endif
};

/* Byte operation flag for all instructions.  Also used as the
   A/L bit in the extension word to indicate a 20-bit operation.  */
#define BYTE_OPERATION    (1 << 6)
/* Z/C bit in the extension word.  If set the carry bit is ignored
   for the duration of the operation, although it may be changed as
   a result of the operation.  */
#define IGNORE_CARRY_BIT  (1 << 8)  

struct  msp430_opcode_s
{
  const char *name;
  int fmt;
  int insn_opnumb;
  int bin_opcode;
  int bin_mask;
};

#define MSP_INSN(name, fmt, numb, bin, mask) { #name, fmt, numb, bin, mask }

static struct msp430_opcode_s msp430_opcodes[] = 
{
  MSP_INSN (and,   1, 2, 0xf000, 0xf000),
  MSP_INSN (inv,   0, 1, 0xe330, 0xfff0),
  MSP_INSN (xor,   1, 2, 0xe000, 0xf000),
  MSP_INSN (setz,  0, 0, 0xd322, 0xffff),
  MSP_INSN (setc,  0, 0, 0xd312, 0xffff),
  MSP_INSN (eint,  0, 0, 0xd232, 0xffff),
  MSP_INSN (setn,  0, 0, 0xd222, 0xffff),
  MSP_INSN (bis,   1, 2, 0xd000, 0xf000),
  MSP_INSN (clrz,  0, 0, 0xc322, 0xffff),
  MSP_INSN (clrc,  0, 0, 0xc312, 0xffff),
  MSP_INSN (dint,  0, 0, 0xc232, 0xffff),
  MSP_INSN (clrn,  0, 0, 0xc222, 0xffff),
  MSP_INSN (bic,   1, 2, 0xc000, 0xf000),
  MSP_INSN (bit,   1, 2, 0xb000, 0xf000),
  MSP_INSN (dadc,  0, 1, 0xa300, 0xff30),
  MSP_INSN (dadd,  1, 2, 0xa000, 0xf000),
  MSP_INSN (tst,   0, 1, 0x9300, 0xff30),
  MSP_INSN (cmp,   1, 2, 0x9000, 0xf000),
  MSP_INSN (decd,  0, 1, 0x8320, 0xff30),
  MSP_INSN (dec,   0, 1, 0x8310, 0xff30),
  MSP_INSN (sub,   1, 2, 0x8000, 0xf000),
  MSP_INSN (sbc,   0, 1, 0x7300, 0xff30),
  MSP_INSN (subc,  1, 2, 0x7000, 0xf000),
  MSP_INSN (adc,   0, 1, 0x6300, 0xff30),
  MSP_INSN (rlc,   0, 2, 0x6000, 0xf000),
  MSP_INSN (addc,  1, 2, 0x6000, 0xf000),
  MSP_INSN (incd,  0, 1, 0x5320, 0xff30),
  MSP_INSN (inc,   0, 1, 0x5310, 0xff30),
  MSP_INSN (rla,   0, 2, 0x5000, 0xf000),
  MSP_INSN (add,   1, 2, 0x5000, 0xf000),
  MSP_INSN (nop,   0, 0, 0x4303, 0xffff),
  MSP_INSN (clr,   0, 1, 0x4300, 0xff30),
  MSP_INSN (ret,   0, 0, 0x4130, 0xff30),
  MSP_INSN (pop,   0, 1, 0x4130, 0xff30),
  MSP_INSN (br,    0, 3, 0x4000, 0xf000),
  MSP_INSN (mov,   1, 2, 0x4000, 0xf000),
  MSP_INSN (jmp,   3, 1, 0x3c00, 0xfc00),
  MSP_INSN (jl,    3, 1, 0x3800, 0xfc00),
  MSP_INSN (jge,   3, 1, 0x3400, 0xfc00),
  MSP_INSN (jn,    3, 1, 0x3000, 0xfc00),
  MSP_INSN (jc,    3, 1, 0x2c00, 0xfc00),
  MSP_INSN (jhs,   3, 1, 0x2c00, 0xfc00),
  MSP_INSN (jnc,   3, 1, 0x2800, 0xfc00),
  MSP_INSN (jlo,   3, 1, 0x2800, 0xfc00),
  MSP_INSN (jz,    3, 1, 0x2400, 0xfc00),
  MSP_INSN (jeq,   3, 1, 0x2400, 0xfc00),
  MSP_INSN (jnz,   3, 1, 0x2000, 0xfc00),
  MSP_INSN (jne,   3, 1, 0x2000, 0xfc00),
  MSP_INSN (reti,  2, 0, 0x1300, 0xffc0),
  MSP_INSN (call,  2, 1, 0x1280, 0xffc0),
  MSP_INSN (push,  2, 1, 0x1200, 0xff80),
  MSP_INSN (sxt,   2, 1, 0x1180, 0xffc0),
  MSP_INSN (rra,   2, 1, 0x1100, 0xff80),
  MSP_INSN (swpb,  2, 1, 0x1080, 0xffc0),
  MSP_INSN (rrc,   2, 1, 0x1000, 0xff80),
  /* Simple polymorphs.  */
  MSP_INSN (beq,   4, 0, 0, 0xffff),
  MSP_INSN (bne,   4, 1, 0, 0xffff),
  MSP_INSN (blt,   4, 2, 0, 0xffff),
  MSP_INSN (bltu,  4, 3, 0, 0xffff),
  MSP_INSN (bge,   4, 4, 0, 0xffff),
  MSP_INSN (bgeu,  4, 5, 0, 0xffff),
  MSP_INSN (bltn,  4, 6, 0, 0xffff),
  MSP_INSN (jump,  4, 7, 0, 0xffff),
  /* Long polymorphs.  */
  MSP_INSN (bgt,   5, 0, 0, 0xffff),
  MSP_INSN (bgtu,  5, 1, 0, 0xffff),
  MSP_INSN (bleu,  5, 2, 0, 0xffff),
  MSP_INSN (ble,   5, 3, 0, 0xffff),

  /* MSP430X instructions - these ones use an extension word.
     A negative format indicates an MSP430X instruction.  */
  MSP_INSN (addcx, -2, 2, 0x6000, 0xf000),
  MSP_INSN (addx,  -2, 2, 0x5000, 0xf000),
  MSP_INSN (andx,  -2, 2, 0xf000, 0xf000),
  MSP_INSN (bicx,  -2, 2, 0xc000, 0xf000),
  MSP_INSN (bisx,  -2, 2, 0xd000, 0xf000),
  MSP_INSN (bitx,  -2, 2, 0xb000, 0xf000),
  MSP_INSN (cmpx,  -2, 2, 0x9000, 0xf000),
  MSP_INSN (daddx, -2, 2, 0xa000, 0xf000),
  MSP_INSN (movx,  -2, 2, 0x4000, 0xf000),
  MSP_INSN (subcx, -2, 2, 0x7000, 0xf000),
  MSP_INSN (subx,  -2, 2, 0x8000, 0xf000),
  MSP_INSN (xorx,  -2, 2, 0xe000, 0xf000),

  /* MSP430X Synthetic instructions.  */
  MSP_INSN (adcx,  -1, 1, 0x6300, 0xff30),
  MSP_INSN (clra,  -1, 1, 0x4300, 0xff30),
  MSP_INSN (clrx,  -1, 1, 0x4300, 0xff30),
  MSP_INSN (dadcx, -1, 1, 0xa300, 0xff30),
  MSP_INSN (decx,  -1, 1, 0x8310, 0xff30),  
  MSP_INSN (decda, -1, 1, 0x8320, 0xff30),
  MSP_INSN (decdx, -1, 1, 0x8320, 0xff30),
  MSP_INSN (incx,  -1, 1, 0x5310, 0xff30),  
  MSP_INSN (incda, -1, 1, 0x5320, 0xff30),
  MSP_INSN (incdx, -1, 1, 0x5320, 0xff30),
  MSP_INSN (invx,  -1, 1, 0xe330, 0xfff0),
  MSP_INSN (popx,  -1, 1, 0x4130, 0xff30),
  MSP_INSN (rlax,  -1, 2, 0x5000, 0xf000),  
  MSP_INSN (rlcx,  -1, 2, 0x6000, 0xf000),
  MSP_INSN (sbcx,  -1, 1, 0x7300, 0xff30),
  MSP_INSN (tsta,  -1, 1, 0x9300, 0xff30),
  MSP_INSN (tstx,  -1, 1, 0x9300, 0xff30),

  MSP_INSN (pushx, -3, 1, 0x1200, 0xff80),
  MSP_INSN (rrax,  -3, 1, 0x1100, 0xff80),
  MSP_INSN (rrcx,  -3, 1, 0x1000, 0xff80), /* Synthesised as RRC but with the Z/C bit clear.  */
  MSP_INSN (rrux,  -3, 1, 0x1000, 0xff80), /* Synthesised as RRC but with the Z/C bit set.  */
  MSP_INSN (swpbx, -3, 1, 0x1080, 0xffc0),
  MSP_INSN (sxtx,  -3, 1, 0x1180, 0xffc0),

  /* MSP430X Address instructions - no extension word needed.
     The insn_opnumb field is used to encode the nature of the
     instruction for assembly and disassembly purposes.  */
  MSP_INSN (calla, -1, 4, 0x1300, 0xff00),

  MSP_INSN (popm,  -1, 5, 0x1600, 0xfe00),
  MSP_INSN (pushm, -1, 5, 0x1400, 0xfe00),

  MSP_INSN (rrcm,  -1, 6, 0x0040, 0xf3e0),
  MSP_INSN (rram,  -1, 6, 0x0140, 0xf3e0),
  MSP_INSN (rlam,  -1, 6, 0x0240, 0xf3e0),
  MSP_INSN (rrum,  -1, 6, 0x0340, 0xf3e0),

  MSP_INSN (adda,  -1, 8, 0x00a0, 0xf0b0),
  MSP_INSN (cmpa,  -1, 8, 0x0090, 0xf0b0),
  MSP_INSN (suba,  -1, 8, 0x00b0, 0xf0b0),

  MSP_INSN (reta,  -1, 9, 0x0110, 0xffff),
  MSP_INSN (bra,   -1, 9, 0x0000, 0xf0cf),
  MSP_INSN (mova,  -1, 9, 0x0000, 0xf080),
  MSP_INSN (mova,  -1, 9, 0x0080, 0xf0b0),
  MSP_INSN (mova,  -1, 9, 0x00c0, 0xf0f0),

  /* Pseudo instruction to set the repeat field in the extension word.  */
  MSP_INSN (rpt,   -1, 10, 0x0000, 0x0000),

  /* End of instruction set.  */
  { NULL, 0, 0, 0, 0 }
};

#endif
