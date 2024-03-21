/* DO NOT EDIT!  -*- buffer-read-only: t -*- vi:set ro:  */
#line 1 "rl78-decode.opc"
/* -*- c -*- */
/* Copyright (C) 2012-2023 Free Software Foundation, Inc.
   Contributed by Red Hat.
   Written by DJ Delorie.

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
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bfd.h"
#include "opintl.h"
#include "opcode/rl78.h"

static int trace = 0;

typedef struct
{
  RL78_Opcode_Decoded * rl78;
  int (* getbyte)(void *);
  void * ptr;
  unsigned char * op;
} LocalData;

#define ID(x) rl78->id = RLO_##x, rl78->lineno = __LINE__
#define OP(n,t,r,a) (rl78->op[n].type = t, \
		     rl78->op[n].reg = r,	     \
		     rl78->op[n].addend = a )
#define OPX(n,t,r1,r2,a) \
	(rl78->op[n].type = t, \
	rl78->op[n].reg = r1, \
	rl78->op[n].reg2 = r2, \
	rl78->op[n].addend = a )

#define W() rl78->size = RL78_Word

#define AU ATTRIBUTE_UNUSED

#define OP_BUF_LEN 20
#define GETBYTE() (ld->rl78->n_bytes < (OP_BUF_LEN - 1) ? ld->op [ld->rl78->n_bytes++] = ld->getbyte (ld->ptr): 0)
#define B ((unsigned long) GETBYTE())

#define SYNTAX(x) rl78->syntax = x

#define UNSUPPORTED() \
  rl78->syntax = "*unknown*"

#define RB(x) ((x)+RL78_Reg_X)
#define RW(x) ((x)+RL78_Reg_AX)

#define Fz	rl78->flags = RL78_PSW_Z
#define Fza	rl78->flags = RL78_PSW_Z | RL78_PSW_AC
#define Fzc	rl78->flags = RL78_PSW_Z | RL78_PSW_CY
#define Fzac	rl78->flags = RL78_PSW_Z | RL78_PSW_AC | RL78_PSW_CY
#define Fa	rl78->flags = RL78_PSW_AC
#define Fc	rl78->flags = RL78_PSW_CY
#define Fac	rl78->flags = RL78_PSW_AC | RL78_PSW_CY

#define IMMU(bytes)   immediate (bytes, 0, ld)
#define IMMS(bytes)   immediate (bytes, 1, ld)

static int
immediate (int bytes, int sign_extend, LocalData * ld)
{
  unsigned long i = 0;

  switch (bytes)
    {
    case 1:
      i |= B;
      if (sign_extend && (i & 0x80))
	i -= 0x100;
      break;
    case 2:
      i |= B;
      i |= B << 8;
      if (sign_extend && (i & 0x8000))
	i -= 0x10000;
      break;
    case 3:
      i |= B;
      i |= B << 8;
      i |= B << 16;
      if (sign_extend && (i & 0x800000))
	i -= 0x1000000;
      break;
    default:
      opcodes_error_handler
	/* xgettext:c-format */
	(_("internal error: immediate() called with invalid byte count %d"),
	   bytes);
      abort();
    }
  return i;
}

#define DC(c)		OP (0, RL78_Operand_Immediate, 0, c)
#define DR(r)		OP (0, RL78_Operand_Register, RL78_Reg_##r, 0)
#define DRB(r)		OP (0, RL78_Operand_Register, RB(r), 0)
#define DRW(r)		OP (0, RL78_Operand_Register, RW(r), 0)
#define DM(r,a)		OP (0, RL78_Operand_Indirect, RL78_Reg_##r, a)
#define DM2(r1,r2,a)	OPX (0, RL78_Operand_Indirect, RL78_Reg_##r1, RL78_Reg_##r2, a)
#define DE()		rl78->op[0].use_es = 1
#define DB(b)		set_bit (rl78->op, b)
#define DCY()		DR(PSW); DB(0)
#define DPUSH()		OP (0, RL78_Operand_PreDec, RL78_Reg_SP, 0);

#define SC(c)		OP (1, RL78_Operand_Immediate, 0, c)
#define SR(r)		OP (1, RL78_Operand_Register, RL78_Reg_##r, 0)
#define SRB(r)		OP (1, RL78_Operand_Register, RB(r), 0)
#define SRW(r)		OP (1, RL78_Operand_Register, RW(r), 0)
#define SM(r,a)		OP (1, RL78_Operand_Indirect, RL78_Reg_##r, a)
#define SM2(r1,r2,a)	OPX (1, RL78_Operand_Indirect, RL78_Reg_##r1, RL78_Reg_##r2, a)
#define SE()		rl78->op[1].use_es = 1
#define SB(b)		set_bit (rl78->op+1, b)
#define SCY()		SR(PSW); SB(0)
#define COND(c)		rl78->op[1].condition = RL78_Condition_##c
#define SPOP()		OP (1, RL78_Operand_PostInc, RL78_Reg_SP, 0);

static void
set_bit (RL78_Opcode_Operand *op, int bit)
{
  op->bit_number = bit;
  switch (op->type) {
  case RL78_Operand_Register:
    op->type = RL78_Operand_Bit;
    break;
  case RL78_Operand_Indirect:
    op->type = RL78_Operand_BitIndirect;
    break;
  default:
    break;
  }
}

static int
saddr (int x)
{
  if (x < 0x20)
    return 0xfff00 + x;
  return 0xffe00 + x;
}

static int
sfr (int x)
{
  return 0xfff00 + x;
}

#define SADDR saddr (IMMU (1))
#define SFR sfr (IMMU (1))

int
rl78_decode_opcode (unsigned long pc AU,
		  RL78_Opcode_Decoded * rl78,
		  int (* getbyte)(void *),
		  void * ptr,
		  RL78_Dis_Isa isa)
{
  LocalData lds, * ld = &lds;
  unsigned char op_buf[OP_BUF_LEN] = {0};
  unsigned char *op = op_buf;
  int op0, op1;

  lds.rl78 = rl78;
  lds.getbyte = getbyte;
  lds.ptr = ptr;
  lds.op = op;

  memset (rl78, 0, sizeof (*rl78));

 start_again:

/* Byte registers, not including A.  */
/* Word registers, not including AX.  */

/*----------------------------------------------------------------------*/
/* ES: prefix								*/

  GETBYTE ();
  switch (op[0] & 0xff)
  {
    case 0x00:
        {
          /** 0000 0000			nop					*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0000 0000			nop					*/",
                     op[0]);
            }
          SYNTAX("nop");
#line 917 "rl78-decode.opc"
          ID(nop);

        /*----------------------------------------------------------------------*/

        }
      break;
    case 0x01:
    case 0x03:
    case 0x05:
    case 0x07:
        {
          /** 0000 0rw1			addw	%0, %1				*/
#line 280 "rl78-decode.opc"
          int rw AU = (op[0] >> 1) & 0x03;
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0000 0rw1			addw	%0, %1				*/",
                     op[0]);
              printf ("  rw = 0x%x\n", rw);
            }
          SYNTAX("addw	%0, %1");
#line 280 "rl78-decode.opc"
          ID(add); W(); DR(AX); SRW(rw); Fzac;

        }
      break;
    case 0x02:
        {
          /** 0000 0010			addw	%0, %e!1			*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0000 0010			addw	%0, %e!1			*/",
                     op[0]);
            }
          SYNTAX("addw	%0, %e!1");
#line 271 "rl78-decode.opc"
          ID(add); W(); DR(AX); SM(None, IMMU(2)); Fzac;

        }
      break;
    case 0x04:
        {
          /** 0000 0100			addw	%0, #%1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0000 0100			addw	%0, #%1				*/",
                     op[0]);
            }
          SYNTAX("addw	%0, #%1");
#line 277 "rl78-decode.opc"
          ID(add); W(); DR(AX); SC(IMMU(2)); Fzac;

        }
      break;
    case 0x06:
        {
          /** 0000 0110			addw	%0, %1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0000 0110			addw	%0, %1				*/",
                     op[0]);
            }
          SYNTAX("addw	%0, %1");
#line 283 "rl78-decode.opc"
          ID(add); W(); DR(AX); SM(None, SADDR); Fzac;

        }
      break;
    case 0x08:
        {
          /** 0000 1000			xch	a, x				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0000 1000			xch	a, x				*/",
                     op[0]);
            }
          SYNTAX("xch	a, x");
#line 1240 "rl78-decode.opc"
          ID(xch); DR(A); SR(X);

        /*----------------------------------------------------------------------*/

        }
      break;
    case 0x09:
        {
          /** 0000 1001			mov	%0, %e1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0000 1001			mov	%0, %e1				*/",
                     op[0]);
            }
          SYNTAX("mov	%0, %e1");
#line 684 "rl78-decode.opc"
          ID(mov); DR(A); SM(B, IMMU(2));

        }
      break;
    case 0x0a:
        {
          /** 0000 1010			add	%0, #%1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0000 1010			add	%0, #%1				*/",
                     op[0]);
            }
          SYNTAX("add	%0, #%1");
#line 234 "rl78-decode.opc"
          ID(add); DM(None, SADDR); SC(IMMU(1)); Fzac;

        /*----------------------------------------------------------------------*/

        }
      break;
    case 0x0b:
        {
          /** 0000 1011			add	%0, %1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0000 1011			add	%0, %1				*/",
                     op[0]);
            }
          SYNTAX("add	%0, %1");
#line 228 "rl78-decode.opc"
          ID(add); DR(A); SM(None, SADDR); Fzac;

        }
      break;
    case 0x0c:
        {
          /** 0000 1100			add	%0, #%1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0000 1100			add	%0, #%1				*/",
                     op[0]);
            }
          SYNTAX("add	%0, #%1");
#line 222 "rl78-decode.opc"
          ID(add); DR(A); SC(IMMU(1)); Fzac;

        }
      break;
    case 0x0d:
        {
          /** 0000 1101			add	%0, %e1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0000 1101			add	%0, %e1				*/",
                     op[0]);
            }
          SYNTAX("add	%0, %e1");
#line 210 "rl78-decode.opc"
          ID(add); DR(A); SM(HL, 0); Fzac;

        }
      break;
    case 0x0e:
        {
          /** 0000 1110			add	%0, %ea1			*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0000 1110			add	%0, %ea1			*/",
                     op[0]);
            }
          SYNTAX("add	%0, %ea1");
#line 216 "rl78-decode.opc"
          ID(add); DR(A); SM(HL, IMMU(1)); Fzac;

        }
      break;
    case 0x0f:
        {
          /** 0000 1111			add	%0, %e!1			*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0000 1111			add	%0, %e!1			*/",
                     op[0]);
            }
          SYNTAX("add	%0, %e!1");
#line 207 "rl78-decode.opc"
          ID(add); DR(A); SM(None, IMMU(2)); Fzac;

        }
      break;
    case 0x10:
        {
          /** 0001 0000			addw	%0, #%1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0001 0000			addw	%0, #%1				*/",
                     op[0]);
            }
          SYNTAX("addw	%0, #%1");
#line 286 "rl78-decode.opc"
          ID(add); W(); DR(SP); SC(IMMU(1)); Fzac;

        /*----------------------------------------------------------------------*/

        }
      break;
    case 0x11:
        {
          /** 0001 0001			es:					*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0001 0001			es:					*/",
                     op[0]);
            }
          SYNTAX("es:");
#line 199 "rl78-decode.opc"
          DE(); SE();
          op ++;
          pc ++;
          goto start_again;

        /*----------------------------------------------------------------------*/

        }
      break;
    case 0x12:
    case 0x14:
    case 0x16:
        {
          /** 0001 0ra0			movw	%0, %1				*/
#line 865 "rl78-decode.opc"
          int ra AU = (op[0] >> 1) & 0x03;
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0001 0ra0			movw	%0, %1				*/",
                     op[0]);
              printf ("  ra = 0x%x\n", ra);
            }
          SYNTAX("movw	%0, %1");
#line 865 "rl78-decode.opc"
          ID(mov); W(); DRW(ra); SR(AX);

        }
      break;
    case 0x13:
    case 0x15:
    case 0x17:
        {
          /** 0001 0ra1			movw	%0, %1				*/
#line 862 "rl78-decode.opc"
          int ra AU = (op[0] >> 1) & 0x03;
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0001 0ra1			movw	%0, %1				*/",
                     op[0]);
              printf ("  ra = 0x%x\n", ra);
            }
          SYNTAX("movw	%0, %1");
#line 862 "rl78-decode.opc"
          ID(mov); W(); DR(AX); SRW(ra);

        }
      break;
    case 0x18:
        {
          /** 0001 1000			mov	%e0, %1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0001 1000			mov	%e0, %1				*/",
                     op[0]);
            }
          SYNTAX("mov	%e0, %1");
#line 735 "rl78-decode.opc"
          ID(mov); DM(B, IMMU(2)); SR(A);

        }
      break;
    case 0x19:
        {
          /** 0001 1001			mov	%e0, #%1			*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0001 1001			mov	%e0, #%1			*/",
                     op[0]);
            }
          SYNTAX("mov	%e0, #%1");
#line 732 "rl78-decode.opc"
          ID(mov); DM(B, IMMU(2)); SC(IMMU(1));

        }
      break;
    case 0x1a:
        {
          /** 0001 1010			addc	%0, #%1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0001 1010			addc	%0, #%1				*/",
                     op[0]);
            }
          SYNTAX("addc	%0, #%1");
#line 266 "rl78-decode.opc"
          ID(addc); DM(None, SADDR); SC(IMMU(1)); Fzac;

        /*----------------------------------------------------------------------*/

        }
      break;
    case 0x1b:
        {
          /** 0001 1011			addc	%0, %1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0001 1011			addc	%0, %1				*/",
                     op[0]);
            }
          SYNTAX("addc	%0, %1");
#line 263 "rl78-decode.opc"
          ID(addc); DR(A); SM(None, SADDR); Fzac;

        }
      break;
    case 0x1c:
        {
          /** 0001 1100			addc	%0, #%1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0001 1100			addc	%0, #%1				*/",
                     op[0]);
            }
          SYNTAX("addc	%0, #%1");
#line 254 "rl78-decode.opc"
          ID(addc); DR(A); SC(IMMU(1)); Fzac;

        }
      break;
    case 0x1d:
        {
          /** 0001 1101			addc	%0, %e1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0001 1101			addc	%0, %e1				*/",
                     op[0]);
            }
          SYNTAX("addc	%0, %e1");
#line 242 "rl78-decode.opc"
          ID(addc); DR(A); SM(HL, 0); Fzac;

        }
      break;
    case 0x1e:
        {
          /** 0001 1110			addc	%0, %ea1			*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0001 1110			addc	%0, %ea1			*/",
                     op[0]);
            }
          SYNTAX("addc	%0, %ea1");
#line 251 "rl78-decode.opc"
          ID(addc); DR(A); SM(HL, IMMU(1)); Fzac;

        }
      break;
    case 0x1f:
        {
          /** 0001 1111			addc	%0, %e!1			*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0001 1111			addc	%0, %e!1			*/",
                     op[0]);
            }
          SYNTAX("addc	%0, %e!1");
#line 239 "rl78-decode.opc"
          ID(addc); DR(A); SM(None, IMMU(2)); Fzac;

        }
      break;
    case 0x20:
        {
          /** 0010 0000			subw	%0, #%1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0010 0000			subw	%0, #%1				*/",
                     op[0]);
            }
          SYNTAX("subw	%0, #%1");
#line 1204 "rl78-decode.opc"
          ID(sub); W(); DR(SP); SC(IMMU(1)); Fzac;

        /*----------------------------------------------------------------------*/

        }
      break;
    case 0x21:
    case 0x23:
    case 0x25:
    case 0x27:
        {
          /** 0010 0rw1			subw	%0, %1				*/
#line 1198 "rl78-decode.opc"
          int rw AU = (op[0] >> 1) & 0x03;
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0010 0rw1			subw	%0, %1				*/",
                     op[0]);
              printf ("  rw = 0x%x\n", rw);
            }
          SYNTAX("subw	%0, %1");
#line 1198 "rl78-decode.opc"
          ID(sub); W(); DR(AX); SRW(rw); Fzac;

        }
      break;
    case 0x22:
        {
          /** 0010 0010			subw	%0, %e!1			*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0010 0010			subw	%0, %e!1			*/",
                     op[0]);
            }
          SYNTAX("subw	%0, %e!1");
#line 1189 "rl78-decode.opc"
          ID(sub); W(); DR(AX); SM(None, IMMU(2)); Fzac;

        }
      break;
    case 0x24:
        {
          /** 0010 0100			subw	%0, #%1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0010 0100			subw	%0, #%1				*/",
                     op[0]);
            }
          SYNTAX("subw	%0, #%1");
#line 1195 "rl78-decode.opc"
          ID(sub); W(); DR(AX); SC(IMMU(2)); Fzac;

        }
      break;
    case 0x26:
        {
          /** 0010 0110			subw	%0, %1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0010 0110			subw	%0, %1				*/",
                     op[0]);
            }
          SYNTAX("subw	%0, %1");
#line 1201 "rl78-decode.opc"
          ID(sub); W(); DR(AX); SM(None, SADDR); Fzac;

        }
      break;
    case 0x28:
        {
          /** 0010 1000			mov	%e0, %1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0010 1000			mov	%e0, %1				*/",
                     op[0]);
            }
          SYNTAX("mov	%e0, %1");
#line 747 "rl78-decode.opc"
          ID(mov); DM(C, IMMU(2)); SR(A);

        }
      break;
    case 0x29:
        {
          /** 0010 1001			mov	%0, %e1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0010 1001			mov	%0, %e1				*/",
                     op[0]);
            }
          SYNTAX("mov	%0, %e1");
#line 690 "rl78-decode.opc"
          ID(mov); DR(A); SM(C, IMMU(2));

        }
      break;
    case 0x2a:
        {
          /** 0010 1010			sub	%0, #%1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0010 1010			sub	%0, #%1				*/",
                     op[0]);
            }
          SYNTAX("sub	%0, #%1");
#line 1152 "rl78-decode.opc"
          ID(sub); DM(None, SADDR); SC(IMMU(1)); Fzac;

        /*----------------------------------------------------------------------*/

        }
      break;
    case 0x2b:
        {
          /** 0010 1011			sub	%0, %1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0010 1011			sub	%0, %1				*/",
                     op[0]);
            }
          SYNTAX("sub	%0, %1");
#line 1146 "rl78-decode.opc"
          ID(sub); DR(A); SM(None, SADDR); Fzac;

        }
      break;
    case 0x2c:
        {
          /** 0010 1100			sub	%0, #%1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0010 1100			sub	%0, #%1				*/",
                     op[0]);
            }
          SYNTAX("sub	%0, #%1");
#line 1140 "rl78-decode.opc"
          ID(sub); DR(A); SC(IMMU(1)); Fzac;

        }
      break;
    case 0x2d:
        {
          /** 0010 1101			sub	%0, %e1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0010 1101			sub	%0, %e1				*/",
                     op[0]);
            }
          SYNTAX("sub	%0, %e1");
#line 1128 "rl78-decode.opc"
          ID(sub); DR(A); SM(HL, 0); Fzac;

        }
      break;
    case 0x2e:
        {
          /** 0010 1110			sub	%0, %ea1			*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0010 1110			sub	%0, %ea1			*/",
                     op[0]);
            }
          SYNTAX("sub	%0, %ea1");
#line 1134 "rl78-decode.opc"
          ID(sub); DR(A); SM(HL, IMMU(1)); Fzac;

        }
      break;
    case 0x2f:
        {
          /** 0010 1111			sub	%0, %e!1			*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0010 1111			sub	%0, %e!1			*/",
                     op[0]);
            }
          SYNTAX("sub	%0, %e!1");
#line 1125 "rl78-decode.opc"
          ID(sub); DR(A); SM(None, IMMU(2)); Fzac;

        }
      break;
    case 0x30:
    case 0x32:
    case 0x34:
    case 0x36:
        {
          /** 0011 0rg0			movw	%0, #%1				*/
#line 859 "rl78-decode.opc"
          int rg AU = (op[0] >> 1) & 0x03;
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0011 0rg0			movw	%0, #%1				*/",
                     op[0]);
              printf ("  rg = 0x%x\n", rg);
            }
          SYNTAX("movw	%0, #%1");
#line 859 "rl78-decode.opc"
          ID(mov); W(); DRW(rg); SC(IMMU(2));

        }
      break;
    case 0x31:
        GETBYTE ();
        switch (op[1] & 0x8f)
        {
          case 0x00:
              {
                /** 0011 0001 0bit 0000		btclr	%s1, $%a0			*/
#line 422 "rl78-decode.opc"
                int bit AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0011 0001 0bit 0000		btclr	%s1, $%a0			*/",
                           op[0], op[1]);
                    printf ("  bit = 0x%x\n", bit);
                  }
                SYNTAX("btclr	%s1, $%a0");
#line 422 "rl78-decode.opc"
                ID(branch_cond_clear); SM(None, SADDR); SB(bit); DC(pc+IMMS(1)+4); COND(T);

              /*----------------------------------------------------------------------*/

              }
            break;
          case 0x01:
              {
                /** 0011 0001 0bit 0001		btclr	%1, $%a0			*/
#line 416 "rl78-decode.opc"
                int bit AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0011 0001 0bit 0001		btclr	%1, $%a0			*/",
                           op[0], op[1]);
                    printf ("  bit = 0x%x\n", bit);
                  }
                SYNTAX("btclr	%1, $%a0");
#line 416 "rl78-decode.opc"
                ID(branch_cond_clear); DC(pc+IMMS(1)+3); SR(A); SB(bit); COND(T);

              }
            break;
          case 0x02:
              {
                /** 0011 0001 0bit 0010		bt	%s1, $%a0			*/
#line 408 "rl78-decode.opc"
                int bit AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0011 0001 0bit 0010		bt	%s1, $%a0			*/",
                           op[0], op[1]);
                    printf ("  bit = 0x%x\n", bit);
                  }
                SYNTAX("bt	%s1, $%a0");
#line 408 "rl78-decode.opc"
                ID(branch_cond); SM(None, SADDR); SB(bit); DC(pc+IMMS(1)+4); COND(T);

              /*----------------------------------------------------------------------*/

              }
            break;
          case 0x03:
              {
                /** 0011 0001 0bit 0011		bt	%1, $%a0			*/
#line 402 "rl78-decode.opc"
                int bit AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0011 0001 0bit 0011		bt	%1, $%a0			*/",
                           op[0], op[1]);
                    printf ("  bit = 0x%x\n", bit);
                  }
                SYNTAX("bt	%1, $%a0");
#line 402 "rl78-decode.opc"
                ID(branch_cond); DC(pc+IMMS(1)+3); SR(A); SB(bit); COND(T);

              }
            break;
          case 0x04:
              {
                /** 0011 0001 0bit 0100		bf	%s1, $%a0			*/
#line 369 "rl78-decode.opc"
                int bit AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0011 0001 0bit 0100		bf	%s1, $%a0			*/",
                           op[0], op[1]);
                    printf ("  bit = 0x%x\n", bit);
                  }
                SYNTAX("bf	%s1, $%a0");
#line 369 "rl78-decode.opc"
                ID(branch_cond); SM(None, SADDR); SB(bit); DC(pc+IMMS(1)+4); COND(F);

              /*----------------------------------------------------------------------*/

              }
            break;
          case 0x05:
              {
                /** 0011 0001 0bit 0101		bf	%1, $%a0			*/
#line 363 "rl78-decode.opc"
                int bit AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0011 0001 0bit 0101		bf	%1, $%a0			*/",
                           op[0], op[1]);
                    printf ("  bit = 0x%x\n", bit);
                  }
                SYNTAX("bf	%1, $%a0");
#line 363 "rl78-decode.opc"
                ID(branch_cond); DC(pc+IMMS(1)+3); SR(A); SB(bit); COND(F);

              }
            break;
          case 0x07:
              {
                /** 0011 0001 0cnt 0111		shl	%0, %1				*/
#line 1081 "rl78-decode.opc"
                int cnt AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0011 0001 0cnt 0111		shl	%0, %1				*/",
                           op[0], op[1]);
                    printf ("  cnt = 0x%x\n", cnt);
                  }
                SYNTAX("shl	%0, %1");
#line 1081 "rl78-decode.opc"
                ID(shl); DR(C); SC(cnt);

              }
            break;
          case 0x08:
              {
                /** 0011 0001 0cnt 1000		shl	%0, %1				*/
#line 1078 "rl78-decode.opc"
                int cnt AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0011 0001 0cnt 1000		shl	%0, %1				*/",
                           op[0], op[1]);
                    printf ("  cnt = 0x%x\n", cnt);
                  }
                SYNTAX("shl	%0, %1");
#line 1078 "rl78-decode.opc"
                ID(shl); DR(B); SC(cnt);

              }
            break;
          case 0x09:
              {
                /** 0011 0001 0cnt 1001		shl	%0, %1				*/
#line 1075 "rl78-decode.opc"
                int cnt AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0011 0001 0cnt 1001		shl	%0, %1				*/",
                           op[0], op[1]);
                    printf ("  cnt = 0x%x\n", cnt);
                  }
                SYNTAX("shl	%0, %1");
#line 1075 "rl78-decode.opc"
                ID(shl); DR(A); SC(cnt);

              }
            break;
          case 0x0a:
              {
                /** 0011 0001 0cnt 1010		shr	%0, %1				*/
#line 1092 "rl78-decode.opc"
                int cnt AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0011 0001 0cnt 1010		shr	%0, %1				*/",
                           op[0], op[1]);
                    printf ("  cnt = 0x%x\n", cnt);
                  }
                SYNTAX("shr	%0, %1");
#line 1092 "rl78-decode.opc"
                ID(shr); DR(A); SC(cnt);

              }
            break;
          case 0x0b:
              {
                /** 0011 0001 0cnt 1011		sar	%0, %1				*/
#line 1039 "rl78-decode.opc"
                int cnt AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0011 0001 0cnt 1011		sar	%0, %1				*/",
                           op[0], op[1]);
                    printf ("  cnt = 0x%x\n", cnt);
                  }
                SYNTAX("sar	%0, %1");
#line 1039 "rl78-decode.opc"
                ID(sar); DR(A); SC(cnt);

              }
            break;
          case 0x0c:
          case 0x8c:
              {
                /** 0011 0001 wcnt 1100		shlw	%0, %1				*/
#line 1087 "rl78-decode.opc"
                int wcnt AU = (op[1] >> 4) & 0x0f;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0011 0001 wcnt 1100		shlw	%0, %1				*/",
                           op[0], op[1]);
                    printf ("  wcnt = 0x%x\n", wcnt);
                  }
                SYNTAX("shlw	%0, %1");
#line 1087 "rl78-decode.opc"
                ID(shl); W(); DR(BC); SC(wcnt);

              /*----------------------------------------------------------------------*/

              }
            break;
          case 0x0d:
          case 0x8d:
              {
                /** 0011 0001 wcnt 1101		shlw	%0, %1				*/
#line 1084 "rl78-decode.opc"
                int wcnt AU = (op[1] >> 4) & 0x0f;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0011 0001 wcnt 1101		shlw	%0, %1				*/",
                           op[0], op[1]);
                    printf ("  wcnt = 0x%x\n", wcnt);
                  }
                SYNTAX("shlw	%0, %1");
#line 1084 "rl78-decode.opc"
                ID(shl); W(); DR(AX); SC(wcnt);

              }
            break;
          case 0x0e:
          case 0x8e:
              {
                /** 0011 0001 wcnt 1110		shrw	%0, %1				*/
#line 1095 "rl78-decode.opc"
                int wcnt AU = (op[1] >> 4) & 0x0f;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0011 0001 wcnt 1110		shrw	%0, %1				*/",
                           op[0], op[1]);
                    printf ("  wcnt = 0x%x\n", wcnt);
                  }
                SYNTAX("shrw	%0, %1");
#line 1095 "rl78-decode.opc"
                ID(shr); W(); DR(AX); SC(wcnt);

              /*----------------------------------------------------------------------*/

              }
            break;
          case 0x0f:
          case 0x8f:
              {
                /** 0011 0001 wcnt 1111		sarw	%0, %1				*/
#line 1042 "rl78-decode.opc"
                int wcnt AU = (op[1] >> 4) & 0x0f;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0011 0001 wcnt 1111		sarw	%0, %1				*/",
                           op[0], op[1]);
                    printf ("  wcnt = 0x%x\n", wcnt);
                  }
                SYNTAX("sarw	%0, %1");
#line 1042 "rl78-decode.opc"
                ID(sar); W(); DR(AX); SC(wcnt);

              /*----------------------------------------------------------------------*/

              }
            break;
          case 0x80:
              {
                /** 0011 0001 1bit 0000		btclr	%s1, $%a0			*/
#line 419 "rl78-decode.opc"
                int bit AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0011 0001 1bit 0000		btclr	%s1, $%a0			*/",
                           op[0], op[1]);
                    printf ("  bit = 0x%x\n", bit);
                  }
                SYNTAX("btclr	%s1, $%a0");
#line 419 "rl78-decode.opc"
                ID(branch_cond_clear); SM(None, SFR); SB(bit); DC(pc+IMMS(1)+4); COND(T);

              }
            break;
          case 0x81:
              {
                /** 0011 0001 1bit 0001		btclr	%e1, $%a0			*/
#line 413 "rl78-decode.opc"
                int bit AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0011 0001 1bit 0001		btclr	%e1, $%a0			*/",
                           op[0], op[1]);
                    printf ("  bit = 0x%x\n", bit);
                  }
                SYNTAX("btclr	%e1, $%a0");
#line 413 "rl78-decode.opc"
                ID(branch_cond_clear); DC(pc+IMMS(1)+3); SM(HL,0); SB(bit); COND(T);

              }
            break;
          case 0x82:
              {
                /** 0011 0001 1bit 0010		bt	%s1, $%a0			*/
#line 405 "rl78-decode.opc"
                int bit AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0011 0001 1bit 0010		bt	%s1, $%a0			*/",
                           op[0], op[1]);
                    printf ("  bit = 0x%x\n", bit);
                  }
                SYNTAX("bt	%s1, $%a0");
#line 405 "rl78-decode.opc"
                ID(branch_cond); SM(None, SFR); SB(bit); DC(pc+IMMS(1)+4); COND(T);

              }
            break;
          case 0x83:
              {
                /** 0011 0001 1bit 0011		bt	%e1, $%a0			*/
#line 399 "rl78-decode.opc"
                int bit AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0011 0001 1bit 0011		bt	%e1, $%a0			*/",
                           op[0], op[1]);
                    printf ("  bit = 0x%x\n", bit);
                  }
                SYNTAX("bt	%e1, $%a0");
#line 399 "rl78-decode.opc"
                ID(branch_cond); DC(pc+IMMS(1)+3); SM(HL,0); SB(bit); COND(T);

              }
            break;
          case 0x84:
              {
                /** 0011 0001 1bit 0100		bf	%s1, $%a0			*/
#line 366 "rl78-decode.opc"
                int bit AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0011 0001 1bit 0100		bf	%s1, $%a0			*/",
                           op[0], op[1]);
                    printf ("  bit = 0x%x\n", bit);
                  }
                SYNTAX("bf	%s1, $%a0");
#line 366 "rl78-decode.opc"
                ID(branch_cond); SM(None, SFR); SB(bit); DC(pc+IMMS(1)+4); COND(F);

              }
            break;
          case 0x85:
              {
                /** 0011 0001 1bit 0101		bf	%e1, $%a0			*/
#line 360 "rl78-decode.opc"
                int bit AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0011 0001 1bit 0101		bf	%e1, $%a0			*/",
                           op[0], op[1]);
                    printf ("  bit = 0x%x\n", bit);
                  }
                SYNTAX("bf	%e1, $%a0");
#line 360 "rl78-decode.opc"
                ID(branch_cond); DC(pc+IMMS(1)+3); SM(HL,0); SB(bit); COND(F);

              }
            break;
          default: UNSUPPORTED(); break;
        }
      break;
    case 0x33:
    case 0x35:
    case 0x37:
        {
          /** 0011 0ra1			xchw	%0, %1				*/
#line 1245 "rl78-decode.opc"
          int ra AU = (op[0] >> 1) & 0x03;
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0011 0ra1			xchw	%0, %1				*/",
                     op[0]);
              printf ("  ra = 0x%x\n", ra);
            }
          SYNTAX("xchw	%0, %1");
#line 1245 "rl78-decode.opc"
          ID(xch); W(); DR(AX); SRW(ra);

        /*----------------------------------------------------------------------*/

        }
      break;
    case 0x38:
        {
          /** 0011 1000			mov	%e0, #%1			*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0011 1000			mov	%e0, #%1			*/",
                     op[0]);
            }
          SYNTAX("mov	%e0, #%1");
#line 744 "rl78-decode.opc"
          ID(mov); DM(C, IMMU(2)); SC(IMMU(1));

        }
      break;
    case 0x39:
        {
          /** 0011 1001			mov	%e0, #%1			*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0011 1001			mov	%e0, #%1			*/",
                     op[0]);
            }
          SYNTAX("mov	%e0, #%1");
#line 738 "rl78-decode.opc"
          ID(mov); DM(BC, IMMU(2)); SC(IMMU(1));

        }
      break;
    case 0x3a:
        {
          /** 0011 1010			subc	%0, #%1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0011 1010			subc	%0, #%1				*/",
                     op[0]);
            }
          SYNTAX("subc	%0, #%1");
#line 1184 "rl78-decode.opc"
          ID(subc); DM(None, SADDR); SC(IMMU(1)); Fzac;

        /*----------------------------------------------------------------------*/

        }
      break;
    case 0x3b:
        {
          /** 0011 1011			subc	%0, %1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0011 1011			subc	%0, %1				*/",
                     op[0]);
            }
          SYNTAX("subc	%0, %1");
#line 1181 "rl78-decode.opc"
          ID(subc); DR(A); SM(None, SADDR); Fzac;

        }
      break;
    case 0x3c:
        {
          /** 0011 1100			subc	%0, #%1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0011 1100			subc	%0, #%1				*/",
                     op[0]);
            }
          SYNTAX("subc	%0, #%1");
#line 1172 "rl78-decode.opc"
          ID(subc); DR(A); SC(IMMU(1)); Fzac;

        }
      break;
    case 0x3d:
        {
          /** 0011 1101			subc	%0, %e1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0011 1101			subc	%0, %e1				*/",
                     op[0]);
            }
          SYNTAX("subc	%0, %e1");
#line 1160 "rl78-decode.opc"
          ID(subc); DR(A); SM(HL, 0); Fzac;

        }
      break;
    case 0x3e:
        {
          /** 0011 1110			subc	%0, %ea1			*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0011 1110			subc	%0, %ea1			*/",
                     op[0]);
            }
          SYNTAX("subc	%0, %ea1");
#line 1169 "rl78-decode.opc"
          ID(subc); DR(A); SM(HL, IMMU(1)); Fzac;

        }
      break;
    case 0x3f:
        {
          /** 0011 1111			subc	%0, %e!1			*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0011 1111			subc	%0, %e!1			*/",
                     op[0]);
            }
          SYNTAX("subc	%0, %e!1");
#line 1157 "rl78-decode.opc"
          ID(subc); DR(A); SM(None, IMMU(2)); Fzac;

        }
      break;
    case 0x40:
        {
          /** 0100 0000			cmp	%e!0, #%1			*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0100 0000			cmp	%e!0, #%1			*/",
                     op[0]);
            }
          SYNTAX("cmp	%e!0, #%1");
#line 486 "rl78-decode.opc"
          ID(cmp); DM(None, IMMU(2)); SC(IMMU(1)); Fzac;

        }
      break;
    case 0x41:
        {
          /** 0100 0001			mov	%0, #%1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0100 0001			mov	%0, #%1				*/",
                     op[0]);
            }
          SYNTAX("mov	%0, #%1");
#line 723 "rl78-decode.opc"
          ID(mov); DR(ES); SC(IMMU(1));

        }
      break;
    case 0x42:
        {
          /** 0100 0010			cmpw	%0, %e!1			*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0100 0010			cmpw	%0, %e!1			*/",
                     op[0]);
            }
          SYNTAX("cmpw	%0, %e!1");
#line 537 "rl78-decode.opc"
          ID(cmp); W(); DR(AX); SM(None, IMMU(2)); Fzac;

        }
      break;
    case 0x43:
    case 0x45:
    case 0x47:
        {
          /** 0100 0ra1			cmpw	%0, %1				*/
#line 546 "rl78-decode.opc"
          int ra AU = (op[0] >> 1) & 0x03;
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0100 0ra1			cmpw	%0, %1				*/",
                     op[0]);
              printf ("  ra = 0x%x\n", ra);
            }
          SYNTAX("cmpw	%0, %1");
#line 546 "rl78-decode.opc"
          ID(cmp); W(); DR(AX); SRW(ra); Fzac;

        }
      break;
    case 0x44:
        {
          /** 0100 0100			cmpw	%0, #%1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0100 0100			cmpw	%0, #%1				*/",
                     op[0]);
            }
          SYNTAX("cmpw	%0, #%1");
#line 543 "rl78-decode.opc"
          ID(cmp); W(); DR(AX); SC(IMMU(2)); Fzac;

        }
      break;
    case 0x46:
        {
          /** 0100 0110			cmpw	%0, %1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0100 0110			cmpw	%0, %1				*/",
                     op[0]);
            }
          SYNTAX("cmpw	%0, %1");
#line 549 "rl78-decode.opc"
          ID(cmp); W(); DR(AX); SM(None, SADDR); Fzac;

        /*----------------------------------------------------------------------*/

        }
      break;
    case 0x48:
        {
          /** 0100 1000			mov	%e0, %1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0100 1000			mov	%e0, %1				*/",
                     op[0]);
            }
          SYNTAX("mov	%e0, %1");
#line 741 "rl78-decode.opc"
          ID(mov); DM(BC, IMMU(2)); SR(A);

        }
      break;
    case 0x49:
        {
          /** 0100 1001			mov	%0, %e1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0100 1001			mov	%0, %e1				*/",
                     op[0]);
            }
          SYNTAX("mov	%0, %e1");
#line 687 "rl78-decode.opc"
          ID(mov); DR(A); SM(BC, IMMU(2));

        }
      break;
    case 0x4a:
        {
          /** 0100 1010			cmp	%0, #%1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0100 1010			cmp	%0, #%1				*/",
                     op[0]);
            }
          SYNTAX("cmp	%0, #%1");
#line 489 "rl78-decode.opc"
          ID(cmp); DM(None, SADDR); SC(IMMU(1)); Fzac;

        }
      break;
    case 0x4b:
        {
          /** 0100 1011			cmp	%0, %1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0100 1011			cmp	%0, %1				*/",
                     op[0]);
            }
          SYNTAX("cmp	%0, %1");
#line 516 "rl78-decode.opc"
          ID(cmp); DR(A); SM(None, SADDR); Fzac;

        /*----------------------------------------------------------------------*/

        }
      break;
    case 0x4c:
        {
          /** 0100 1100			cmp	%0, #%1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0100 1100			cmp	%0, #%1				*/",
                     op[0]);
            }
          SYNTAX("cmp	%0, #%1");
#line 507 "rl78-decode.opc"
          ID(cmp); DR(A); SC(IMMU(1)); Fzac;

        }
      break;
    case 0x4d:
        {
          /** 0100 1101			cmp	%0, %e1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0100 1101			cmp	%0, %e1				*/",
                     op[0]);
            }
          SYNTAX("cmp	%0, %e1");
#line 495 "rl78-decode.opc"
          ID(cmp); DR(A); SM(HL, 0); Fzac;

        }
      break;
    case 0x4e:
        {
          /** 0100 1110			cmp	%0, %ea1			*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0100 1110			cmp	%0, %ea1			*/",
                     op[0]);
            }
          SYNTAX("cmp	%0, %ea1");
#line 504 "rl78-decode.opc"
          ID(cmp); DR(A); SM(HL, IMMU(1)); Fzac;

        }
      break;
    case 0x4f:
        {
          /** 0100 1111			cmp	%0, %e!1			*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0100 1111			cmp	%0, %e!1			*/",
                     op[0]);
            }
          SYNTAX("cmp	%0, %e!1");
#line 492 "rl78-decode.opc"
          ID(cmp); DR(A); SM(None, IMMU(2)); Fzac;

        }
      break;
    case 0x50:
    case 0x51:
    case 0x52:
    case 0x53:
    case 0x54:
    case 0x55:
    case 0x56:
    case 0x57:
        {
          /** 0101 0reg			mov	%0, #%1				*/
#line 675 "rl78-decode.opc"
          int reg AU = op[0] & 0x07;
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0101 0reg			mov	%0, #%1				*/",
                     op[0]);
              printf ("  reg = 0x%x\n", reg);
            }
          SYNTAX("mov	%0, #%1");
#line 675 "rl78-decode.opc"
          ID(mov); DRB(reg); SC(IMMU(1));

        }
      break;
    case 0x58:
        {
          /** 0101 1000			movw	%e0, %1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0101 1000			movw	%e0, %1				*/",
                     op[0]);
            }
          SYNTAX("movw	%e0, %1");
#line 877 "rl78-decode.opc"
          ID(mov); W(); DM(B, IMMU(2)); SR(AX);

        }
      break;
    case 0x59:
        {
          /** 0101 1001			movw	%0, %e1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0101 1001			movw	%0, %e1				*/",
                     op[0]);
            }
          SYNTAX("movw	%0, %e1");
#line 868 "rl78-decode.opc"
          ID(mov); W(); DR(AX); SM(B, IMMU(2));

        }
      break;
    case 0x5a:
        {
          /** 0101 1010	       		and	%0, #%1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0101 1010	       		and	%0, #%1				*/",
                     op[0]);
            }
          SYNTAX("and	%0, #%1");
#line 318 "rl78-decode.opc"
          ID(and); DM(None, SADDR); SC(IMMU(1)); Fz;

        /*----------------------------------------------------------------------*/

        }
      break;
    case 0x5b:
        {
          /** 0101 1011	       		and	%0, %1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0101 1011	       		and	%0, %1				*/",
                     op[0]);
            }
          SYNTAX("and	%0, %1");
#line 315 "rl78-decode.opc"
          ID(and); DR(A); SM(None, SADDR); Fz;

        }
      break;
    case 0x5c:
        {
          /** 0101 1100	       		and	%0, #%1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0101 1100	       		and	%0, #%1				*/",
                     op[0]);
            }
          SYNTAX("and	%0, #%1");
#line 306 "rl78-decode.opc"
          ID(and); DR(A); SC(IMMU(1)); Fz;

        }
      break;
    case 0x5d:
        {
          /** 0101 1101			and	%0, %e1			*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0101 1101			and	%0, %e1			*/",
                     op[0]);
            }
          SYNTAX("and	%0, %e1");
#line 294 "rl78-decode.opc"
          ID(and); DR(A); SM(HL, 0); Fz;

        }
      break;
    case 0x5e:
        {
          /** 0101 1110			and	%0, %ea1			*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0101 1110			and	%0, %ea1			*/",
                     op[0]);
            }
          SYNTAX("and	%0, %ea1");
#line 300 "rl78-decode.opc"
          ID(and); DR(A); SM(HL, IMMU(1)); Fz;

        }
      break;
    case 0x5f:
        {
          /** 0101 1111			and	%0, %e!1			*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0101 1111			and	%0, %e!1			*/",
                     op[0]);
            }
          SYNTAX("and	%0, %e!1");
#line 291 "rl78-decode.opc"
          ID(and); DR(A); SM(None, IMMU(2)); Fz;

        }
      break;
    case 0x60:
    case 0x62:
    case 0x63:
    case 0x64:
    case 0x65:
    case 0x66:
    case 0x67:
        {
          /** 0110 0rba			mov	%0, %1				*/
#line 678 "rl78-decode.opc"
          int rba AU = op[0] & 0x07;
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0110 0rba			mov	%0, %1				*/",
                     op[0]);
              printf ("  rba = 0x%x\n", rba);
            }
          SYNTAX("mov	%0, %1");
#line 678 "rl78-decode.opc"
          ID(mov); DR(A); SRB(rba);

        }
      break;
    case 0x61:
        GETBYTE ();
        switch (op[1] & 0xff)
        {
          case 0x00:
          case 0x01:
          case 0x02:
          case 0x03:
          case 0x04:
          case 0x05:
          case 0x06:
          case 0x07:
              {
                /** 0110 0001 0000 0reg		add	%0, %1				*/
#line 231 "rl78-decode.opc"
                int reg AU = op[1] & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 0000 0reg		add	%0, %1				*/",
                           op[0], op[1]);
                    printf ("  reg = 0x%x\n", reg);
                  }
                SYNTAX("add	%0, %1");
#line 231 "rl78-decode.opc"
                ID(add); DRB(reg); SR(A); Fzac;

              }
            break;
          case 0x08:
          case 0x0a:
          case 0x0b:
          case 0x0c:
          case 0x0d:
          case 0x0e:
          case 0x0f:
              {
                /** 0110 0001 0000 1rba		add	%0, %1				*/
#line 225 "rl78-decode.opc"
                int rba AU = op[1] & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 0000 1rba		add	%0, %1				*/",
                           op[0], op[1]);
                    printf ("  rba = 0x%x\n", rba);
                  }
                SYNTAX("add	%0, %1");
#line 225 "rl78-decode.opc"
                ID(add); DR(A); SRB(rba); Fzac;

              }
            break;
          case 0x09:
              {
                /** 0110 0001 0000 1001		addw	%0, %ea1			*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 0000 1001		addw	%0, %ea1			*/",
                           op[0], op[1]);
                  }
                SYNTAX("addw	%0, %ea1");
#line 274 "rl78-decode.opc"
                ID(add); W(); DR(AX); SM(HL, IMMU(1)); Fzac;

              }
            break;
          case 0x10:
          case 0x11:
          case 0x12:
          case 0x13:
          case 0x14:
          case 0x15:
          case 0x16:
          case 0x17:
              {
                /** 0110 0001 0001 0reg		addc	%0, %1				*/
#line 260 "rl78-decode.opc"
                int reg AU = op[1] & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 0001 0reg		addc	%0, %1				*/",
                           op[0], op[1]);
                    printf ("  reg = 0x%x\n", reg);
                  }
                SYNTAX("addc	%0, %1");
#line 260 "rl78-decode.opc"
                ID(addc); DRB(reg); SR(A); Fzac;

              }
            break;
          case 0x18:
          case 0x1a:
          case 0x1b:
          case 0x1c:
          case 0x1d:
          case 0x1e:
          case 0x1f:
              {
                /** 0110 0001 0001 1rba		addc	%0, %1				*/
#line 257 "rl78-decode.opc"
                int rba AU = op[1] & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 0001 1rba		addc	%0, %1				*/",
                           op[0], op[1]);
                    printf ("  rba = 0x%x\n", rba);
                  }
                SYNTAX("addc	%0, %1");
#line 257 "rl78-decode.opc"
                ID(addc); DR(A); SRB(rba); Fzac;

              }
            break;
          case 0x20:
          case 0x21:
          case 0x22:
          case 0x23:
          case 0x24:
          case 0x25:
          case 0x26:
          case 0x27:
              {
                /** 0110 0001 0010 0reg		sub	%0, %1				*/
#line 1149 "rl78-decode.opc"
                int reg AU = op[1] & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 0010 0reg		sub	%0, %1				*/",
                           op[0], op[1]);
                    printf ("  reg = 0x%x\n", reg);
                  }
                SYNTAX("sub	%0, %1");
#line 1149 "rl78-decode.opc"
                ID(sub); DRB(reg); SR(A); Fzac;

              }
            break;
          case 0x28:
          case 0x2a:
          case 0x2b:
          case 0x2c:
          case 0x2d:
          case 0x2e:
          case 0x2f:
              {
                /** 0110 0001 0010 1rba		sub	%0, %1				*/
#line 1143 "rl78-decode.opc"
                int rba AU = op[1] & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 0010 1rba		sub	%0, %1				*/",
                           op[0], op[1]);
                    printf ("  rba = 0x%x\n", rba);
                  }
                SYNTAX("sub	%0, %1");
#line 1143 "rl78-decode.opc"
                ID(sub); DR(A); SRB(rba); Fzac;

              }
            break;
          case 0x29:
              {
                /** 0110 0001 0010 1001		subw	%0, %ea1			*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 0010 1001		subw	%0, %ea1			*/",
                           op[0], op[1]);
                  }
                SYNTAX("subw	%0, %ea1");
#line 1192 "rl78-decode.opc"
                ID(sub); W(); DR(AX); SM(HL, IMMU(1)); Fzac;

              }
            break;
          case 0x30:
          case 0x31:
          case 0x32:
          case 0x33:
          case 0x34:
          case 0x35:
          case 0x36:
          case 0x37:
              {
                /** 0110 0001 0011 0reg		subc	%0, %1				*/
#line 1178 "rl78-decode.opc"
                int reg AU = op[1] & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 0011 0reg		subc	%0, %1				*/",
                           op[0], op[1]);
                    printf ("  reg = 0x%x\n", reg);
                  }
                SYNTAX("subc	%0, %1");
#line 1178 "rl78-decode.opc"
                ID(subc); DRB(reg); SR(A); Fzac;

              }
            break;
          case 0x38:
          case 0x3a:
          case 0x3b:
          case 0x3c:
          case 0x3d:
          case 0x3e:
          case 0x3f:
              {
                /** 0110 0001 0011 1rba		subc	%0, %1				*/
#line 1175 "rl78-decode.opc"
                int rba AU = op[1] & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 0011 1rba		subc	%0, %1				*/",
                           op[0], op[1]);
                    printf ("  rba = 0x%x\n", rba);
                  }
                SYNTAX("subc	%0, %1");
#line 1175 "rl78-decode.opc"
                ID(subc); DR(A); SRB(rba); Fzac;

              }
            break;
          case 0x40:
          case 0x41:
          case 0x42:
          case 0x43:
          case 0x44:
          case 0x45:
          case 0x46:
          case 0x47:
              {
                /** 0110 0001 0100 0reg		cmp	%0, %1				*/
#line 513 "rl78-decode.opc"
                int reg AU = op[1] & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 0100 0reg		cmp	%0, %1				*/",
                           op[0], op[1]);
                    printf ("  reg = 0x%x\n", reg);
                  }
                SYNTAX("cmp	%0, %1");
#line 513 "rl78-decode.opc"
                ID(cmp); DRB(reg); SR(A); Fzac;

              }
            break;
          case 0x48:
          case 0x4a:
          case 0x4b:
          case 0x4c:
          case 0x4d:
          case 0x4e:
          case 0x4f:
              {
                /** 0110 0001 0100 1rba		cmp	%0, %1				*/
#line 510 "rl78-decode.opc"
                int rba AU = op[1] & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 0100 1rba		cmp	%0, %1				*/",
                           op[0], op[1]);
                    printf ("  rba = 0x%x\n", rba);
                  }
                SYNTAX("cmp	%0, %1");
#line 510 "rl78-decode.opc"
                ID(cmp); DR(A); SRB(rba); Fzac;

              }
            break;
          case 0x49:
              {
                /** 0110 0001 0100 1001		cmpw	%0, %ea1			*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 0100 1001		cmpw	%0, %ea1			*/",
                           op[0], op[1]);
                  }
                SYNTAX("cmpw	%0, %ea1");
#line 540 "rl78-decode.opc"
                ID(cmp); W(); DR(AX); SM(HL, IMMU(1)); Fzac;

              }
            break;
          case 0x50:
          case 0x51:
          case 0x52:
          case 0x53:
          case 0x54:
          case 0x55:
          case 0x56:
          case 0x57:
              {
                /** 0110 0001 0101 0reg		and	%0, %1				*/
#line 312 "rl78-decode.opc"
                int reg AU = op[1] & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 0101 0reg		and	%0, %1				*/",
                           op[0], op[1]);
                    printf ("  reg = 0x%x\n", reg);
                  }
                SYNTAX("and	%0, %1");
#line 312 "rl78-decode.opc"
                ID(and); DRB(reg); SR(A); Fz;

              }
            break;
          case 0x58:
          case 0x5a:
          case 0x5b:
          case 0x5c:
          case 0x5d:
          case 0x5e:
          case 0x5f:
              {
                /** 0110 0001 0101 1rba		and	%0, %1				*/
#line 309 "rl78-decode.opc"
                int rba AU = op[1] & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 0101 1rba		and	%0, %1				*/",
                           op[0], op[1]);
                    printf ("  rba = 0x%x\n", rba);
                  }
                SYNTAX("and	%0, %1");
#line 309 "rl78-decode.opc"
                ID(and); DR(A); SRB(rba); Fz;

              }
            break;
          case 0x59:
              {
                /** 0110 0001 0101 1001		inc	%ea0				*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 0101 1001		inc	%ea0				*/",
                           op[0], op[1]);
                  }
                SYNTAX("inc	%ea0");
#line 590 "rl78-decode.opc"
                ID(add); DM(HL, IMMU(1)); SC(1); Fza;

              }
            break;
          case 0x60:
          case 0x61:
          case 0x62:
          case 0x63:
          case 0x64:
          case 0x65:
          case 0x66:
          case 0x67:
              {
                /** 0110 0001 0110 0reg		or	%0, %1				*/
#line 967 "rl78-decode.opc"
                int reg AU = op[1] & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 0110 0reg		or	%0, %1				*/",
                           op[0], op[1]);
                    printf ("  reg = 0x%x\n", reg);
                  }
                SYNTAX("or	%0, %1");
#line 967 "rl78-decode.opc"
                ID(or); DRB(reg); SR(A); Fz;

              }
            break;
          case 0x68:
          case 0x6a:
          case 0x6b:
          case 0x6c:
          case 0x6d:
          case 0x6e:
          case 0x6f:
              {
                /** 0110 0001 0110 1rba		or	%0, %1				*/
#line 964 "rl78-decode.opc"
                int rba AU = op[1] & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 0110 1rba		or	%0, %1				*/",
                           op[0], op[1]);
                    printf ("  rba = 0x%x\n", rba);
                  }
                SYNTAX("or	%0, %1");
#line 964 "rl78-decode.opc"
                ID(or); DR(A); SRB(rba); Fz;

              }
            break;
          case 0x69:
              {
                /** 0110 0001 0110 1001		dec	%ea0				*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 0110 1001		dec	%ea0				*/",
                           op[0], op[1]);
                  }
                SYNTAX("dec	%ea0");
#line 557 "rl78-decode.opc"
                ID(sub); DM(HL, IMMU(1)); SC(1); Fza;

              }
            break;
          case 0x70:
          case 0x71:
          case 0x72:
          case 0x73:
          case 0x74:
          case 0x75:
          case 0x76:
          case 0x77:
              {
                /** 0110 0001 0111 0reg		xor	%0, %1				*/
#line 1271 "rl78-decode.opc"
                int reg AU = op[1] & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 0111 0reg		xor	%0, %1				*/",
                           op[0], op[1]);
                    printf ("  reg = 0x%x\n", reg);
                  }
                SYNTAX("xor	%0, %1");
#line 1271 "rl78-decode.opc"
                ID(xor); DRB(reg); SR(A); Fz;

              }
            break;
          case 0x78:
          case 0x7a:
          case 0x7b:
          case 0x7c:
          case 0x7d:
          case 0x7e:
          case 0x7f:
              {
                /** 0110 0001 0111 1rba		xor	%0, %1				*/
#line 1268 "rl78-decode.opc"
                int rba AU = op[1] & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 0111 1rba		xor	%0, %1				*/",
                           op[0], op[1]);
                    printf ("  rba = 0x%x\n", rba);
                  }
                SYNTAX("xor	%0, %1");
#line 1268 "rl78-decode.opc"
                ID(xor); DR(A); SRB(rba); Fz;

              }
            break;
          case 0x79:
              {
                /** 0110 0001 0111 1001		incw	%ea0				*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 0111 1001		incw	%ea0				*/",
                           op[0], op[1]);
                  }
                SYNTAX("incw	%ea0");
#line 604 "rl78-decode.opc"
                ID(add); W(); DM(HL, IMMU(1)); SC(1);

              }
            break;
          case 0x80:
          case 0x81:
              {
                /** 0110 0001 1000 000		add	%0, %e1				*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1000 000		add	%0, %e1				*/",
                           op[0], op[1]);
                  }
                SYNTAX("add	%0, %e1");
#line 213 "rl78-decode.opc"
                ID(add); DR(A); SM2(HL, B, 0); Fzac;

              }
            break;
          case 0x82:
              {
                /** 0110 0001 1000 0010		add	%0, %e1				*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1000 0010		add	%0, %e1				*/",
                           op[0], op[1]);
                  }
                SYNTAX("add	%0, %e1");
#line 219 "rl78-decode.opc"
                ID(add); DR(A); SM2(HL, C, 0); Fzac;

              }
            break;
          case 0x84:
          case 0x85:
          case 0x86:
          case 0x87:
          case 0x94:
          case 0x95:
          case 0x96:
          case 0x97:
          case 0xa4:
          case 0xa5:
          case 0xa6:
          case 0xa7:
          case 0xb4:
          case 0xb5:
          case 0xb6:
          case 0xb7:
          case 0xc4:
          case 0xc5:
          case 0xc6:
          case 0xc7:
          case 0xd4:
          case 0xd5:
          case 0xd6:
          case 0xd7:
          case 0xe4:
          case 0xe5:
          case 0xe6:
          case 0xe7:
          case 0xf4:
          case 0xf5:
          case 0xf6:
          case 0xf7:
              {
                /** 0110 0001 1nnn 01mm		callt	[%x0]				*/
#line 439 "rl78-decode.opc"
                int nnn AU = (op[1] >> 4) & 0x07;
#line 439 "rl78-decode.opc"
                int mm AU = op[1] & 0x03;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1nnn 01mm		callt	[%x0]				*/",
                           op[0], op[1]);
                    printf ("  nnn = 0x%x,", nnn);
                    printf ("  mm = 0x%x\n", mm);
                  }
                SYNTAX("callt	[%x0]");
#line 439 "rl78-decode.opc"
                ID(call); DM(None, 0x80 + mm*16 + nnn*2);

              /*----------------------------------------------------------------------*/

              }
            break;
          case 0x88:
          case 0x8a:
          case 0x8b:
          case 0x8c:
          case 0x8d:
          case 0x8e:
          case 0x8f:
              {
                /** 0110 0001 1000 1reg		xch	%0, %1				*/
#line 1230 "rl78-decode.opc"
                int reg AU = op[1] & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1000 1reg		xch	%0, %1				*/",
                           op[0], op[1]);
                    printf ("  reg = 0x%x\n", reg);
                  }
                SYNTAX("xch	%0, %1");
#line 1230 "rl78-decode.opc"
                /* Note: DECW uses reg == X, so this must follow DECW */
                ID(xch); DR(A); SRB(reg);

              }
            break;
          case 0x89:
              {
                /** 0110 0001 1000 1001		decw	%ea0				*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1000 1001		decw	%ea0				*/",
                           op[0], op[1]);
                  }
                SYNTAX("decw	%ea0");
#line 571 "rl78-decode.opc"
                ID(sub); W(); DM(HL, IMMU(1)); SC(1);

              }
            break;
          case 0x90:
              {
                /** 0110 0001 1001 0000		addc	%0, %e1				*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1001 0000		addc	%0, %e1				*/",
                           op[0], op[1]);
                  }
                SYNTAX("addc	%0, %e1");
#line 245 "rl78-decode.opc"
                ID(addc); DR(A); SM2(HL, B, 0); Fzac;

              }
            break;
          case 0x92:
              {
                /** 0110 0001 1001 0010		addc	%0, %e1				*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1001 0010		addc	%0, %e1				*/",
                           op[0], op[1]);
                  }
                SYNTAX("addc	%0, %e1");
#line 248 "rl78-decode.opc"
                ID(addc); DR(A); SM2(HL, C, 0); Fzac;

              }
            break;
          case 0xa0:
          case 0xa1:
              {
                /** 0110 0001 1010 000		sub	%0, %e1				*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1010 000		sub	%0, %e1				*/",
                           op[0], op[1]);
                  }
                SYNTAX("sub	%0, %e1");
#line 1131 "rl78-decode.opc"
                ID(sub); DR(A); SM2(HL, B, 0); Fzac;

              }
            break;
          case 0xa2:
              {
                /** 0110 0001 1010 0010		sub	%0, %e1				*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1010 0010		sub	%0, %e1				*/",
                           op[0], op[1]);
                  }
                SYNTAX("sub	%0, %e1");
#line 1137 "rl78-decode.opc"
                ID(sub); DR(A); SM2(HL, C, 0); Fzac;

              }
            break;
          case 0xa8:
              {
                /** 0110 0001 1010 1000	       	xch	%0, %1				*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1010 1000	       	xch	%0, %1				*/",
                           op[0], op[1]);
                  }
                SYNTAX("xch	%0, %1");
#line 1234 "rl78-decode.opc"
                ID(xch); DR(A); SM(None, SADDR);

              }
            break;
          case 0xa9:
              {
                /** 0110 0001 1010 1001		xch	%0, %e1				*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1010 1001		xch	%0, %e1				*/",
                           op[0], op[1]);
                  }
                SYNTAX("xch	%0, %e1");
#line 1227 "rl78-decode.opc"
                ID(xch); DR(A); SM2(HL, C, 0);

              }
            break;
          case 0xaa:
              {
                /** 0110 0001 1010 1010		xch	%0, %e!1			*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1010 1010		xch	%0, %e!1			*/",
                           op[0], op[1]);
                  }
                SYNTAX("xch	%0, %e!1");
#line 1209 "rl78-decode.opc"
                ID(xch); DR(A); SM(None, IMMU(2));

              }
            break;
          case 0xab:
              {
                /** 0110 0001 1010 1011	       	xch	%0, %s1				*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1010 1011	       	xch	%0, %s1				*/",
                           op[0], op[1]);
                  }
                SYNTAX("xch	%0, %s1");
#line 1237 "rl78-decode.opc"
                ID(xch); DR(A); SM(None, SFR);

              }
            break;
          case 0xac:
              {
                /** 0110 0001 1010 1100		xch	%0, %e1				*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1010 1100		xch	%0, %e1				*/",
                           op[0], op[1]);
                  }
                SYNTAX("xch	%0, %e1");
#line 1218 "rl78-decode.opc"
                ID(xch); DR(A); SM(HL, 0);

              }
            break;
          case 0xad:
              {
                /** 0110 0001 1010 1101		xch	%0, %ea1			*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1010 1101		xch	%0, %ea1			*/",
                           op[0], op[1]);
                  }
                SYNTAX("xch	%0, %ea1");
#line 1224 "rl78-decode.opc"
                ID(xch); DR(A); SM(HL, IMMU(1));

              }
            break;
          case 0xae:
              {
                /** 0110 0001 1010 1110		xch	%0, %e1				*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1010 1110		xch	%0, %e1				*/",
                           op[0], op[1]);
                  }
                SYNTAX("xch	%0, %e1");
#line 1212 "rl78-decode.opc"
                ID(xch); DR(A); SM(DE, 0);

              }
            break;
          case 0xaf:
              {
                /** 0110 0001 1010 1111		xch	%0, %ea1				*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1010 1111		xch	%0, %ea1				*/",
                           op[0], op[1]);
                  }
                SYNTAX("xch	%0, %ea1");
#line 1215 "rl78-decode.opc"
                ID(xch); DR(A); SM(DE, IMMU(1));

              }
            break;
          case 0xb0:
              {
                /** 0110 0001 1011 0000		subc	%0, %e1				*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1011 0000		subc	%0, %e1				*/",
                           op[0], op[1]);
                  }
                SYNTAX("subc	%0, %e1");
#line 1163 "rl78-decode.opc"
                ID(subc); DR(A); SM2(HL, B, 0); Fzac;

              }
            break;
          case 0xb2:
              {
                /** 0110 0001 1011 0010		subc	%0, %e1				*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1011 0010		subc	%0, %e1				*/",
                           op[0], op[1]);
                  }
                SYNTAX("subc	%0, %e1");
#line 1166 "rl78-decode.opc"
                ID(subc); DR(A); SM2(HL, C, 0); Fzac;

              }
            break;
          case 0xb8:
              {
                /** 0110 0001 1011 1000		mov	%0, %1				*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1011 1000		mov	%0, %1				*/",
                           op[0], op[1]);
                  }
                SYNTAX("mov	%0, %1");
#line 729 "rl78-decode.opc"
                ID(mov); DR(ES); SM(None, SADDR);

              }
            break;
          case 0xb9:
              {
                /** 0110 0001 1011 1001		xch	%0, %e1				*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1011 1001		xch	%0, %e1				*/",
                           op[0], op[1]);
                  }
                SYNTAX("xch	%0, %e1");
#line 1221 "rl78-decode.opc"
                ID(xch); DR(A); SM2(HL, B, 0);

              }
            break;
          case 0xc0:
              {
                /** 0110 0001 1100 0000		cmp	%0, %e1				*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1100 0000		cmp	%0, %e1				*/",
                           op[0], op[1]);
                  }
                SYNTAX("cmp	%0, %e1");
#line 498 "rl78-decode.opc"
                ID(cmp); DR(A); SM2(HL, B, 0); Fzac;

              }
            break;
          case 0xc2:
              {
                /** 0110 0001 1100 0010		cmp	%0, %e1				*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1100 0010		cmp	%0, %e1				*/",
                           op[0], op[1]);
                  }
                SYNTAX("cmp	%0, %e1");
#line 501 "rl78-decode.opc"
                ID(cmp); DR(A); SM2(HL, C, 0); Fzac;

              }
            break;
          case 0xc3:
              {
                /** 0110 0001 1100 0011		bh	$%a0				*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1100 0011		bh	$%a0				*/",
                           op[0], op[1]);
                  }
                SYNTAX("bh	$%a0");
#line 346 "rl78-decode.opc"
                ID(branch_cond); DC(pc+IMMS(1)+3); SR(None); COND(H);

              }
            break;
          case 0xc8:
              {
                /** 0110 0001 1100 1000		sk%c1					*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1100 1000		sk%c1					*/",
                           op[0], op[1]);
                  }
                SYNTAX("sk%c1");
#line 1100 "rl78-decode.opc"
                ID(skip); COND(C);

              }
            break;
          case 0xc9:
              {
                /** 0110 0001 1100 1001		mov	%0, %e1				*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1100 1001		mov	%0, %e1				*/",
                           op[0], op[1]);
                  }
                SYNTAX("mov	%0, %e1");
#line 666 "rl78-decode.opc"
                ID(mov); DR(A); SM2(HL, B, 0);

              }
            break;
          case 0xca:
          case 0xda:
          case 0xea:
          case 0xfa:
              {
                /** 0110 0001 11rg 1010		call	%0				*/
#line 436 "rl78-decode.opc"
                int rg AU = (op[1] >> 4) & 0x03;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 11rg 1010		call	%0				*/",
                           op[0], op[1]);
                    printf ("  rg = 0x%x\n", rg);
                  }
                SYNTAX("call	%0");
#line 436 "rl78-decode.opc"
                ID(call); DRW(rg);

              }
            break;
          case 0xcb:
              {
                /** 0110 0001 1100 1011		br	ax				*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1100 1011		br	ax				*/",
                           op[0], op[1]);
                  }
                SYNTAX("br	ax");
#line 386 "rl78-decode.opc"
                ID(branch); DR(AX);

              /*----------------------------------------------------------------------*/

              }
            break;
          case 0xcc:
              {
                /** 0110 0001 1100 1100		brk					*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1100 1100		brk					*/",
                           op[0], op[1]);
                  }
                SYNTAX("brk");
#line 394 "rl78-decode.opc"
                ID(break);

              /*----------------------------------------------------------------------*/

              }
            break;
          case 0xcd:
              {
                /** 0110 0001 1100 1101		pop	%s0				*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1100 1101		pop	%s0				*/",
                           op[0], op[1]);
                  }
                SYNTAX("pop	%s0");
#line 995 "rl78-decode.opc"
                ID(mov); W(); DR(PSW); SPOP();

              /*----------------------------------------------------------------------*/

              }
            break;
          case 0xce:
              {
                /** 0110 0001 1100 1110		movs	%ea0, %1			*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1100 1110		movs	%ea0, %1			*/",
                           op[0], op[1]);
                  }
                SYNTAX("movs	%ea0, %1");
#line 817 "rl78-decode.opc"
                ID(mov); DM(HL, IMMU(1)); SR(X); Fzc;

              /*----------------------------------------------------------------------*/

              }
            break;
          case 0xcf:
          case 0xdf:
          case 0xef:
          case 0xff:
              {
                /** 0110 0001 11rb 1111		sel	rb%1				*/
#line 1047 "rl78-decode.opc"
                int rb AU = (op[1] >> 4) & 0x03;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 11rb 1111		sel	rb%1				*/",
                           op[0], op[1]);
                    printf ("  rb = 0x%x\n", rb);
                  }
                SYNTAX("sel	rb%1");
#line 1047 "rl78-decode.opc"
                ID(sel); SC(rb);

              /*----------------------------------------------------------------------*/

              }
            break;
          case 0xd0:
              {
                /** 0110 0001 1101 0000		and	%0, %e1			*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1101 0000		and	%0, %e1			*/",
                           op[0], op[1]);
                  }
                SYNTAX("and	%0, %e1");
#line 297 "rl78-decode.opc"
                ID(and); DR(A); SM2(HL, B, 0); Fz;

              }
            break;
          case 0xd2:
              {
                /** 0110 0001 1101 0010		and	%0, %e1			*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1101 0010		and	%0, %e1			*/",
                           op[0], op[1]);
                  }
                SYNTAX("and	%0, %e1");
#line 303 "rl78-decode.opc"
                ID(and); DR(A); SM2(HL, C, 0); Fz;

              }
            break;
          case 0xd3:
              {
                /** 0110 0001 1101 0011		bnh	$%a0				*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1101 0011		bnh	$%a0				*/",
                           op[0], op[1]);
                  }
                SYNTAX("bnh	$%a0");
#line 349 "rl78-decode.opc"
                ID(branch_cond); DC(pc+IMMS(1)+3); SR(None); COND(NH);

              }
            break;
          case 0xd8:
              {
                /** 0110 0001 1101 1000		sk%c1					*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1101 1000		sk%c1					*/",
                           op[0], op[1]);
                  }
                SYNTAX("sk%c1");
#line 1106 "rl78-decode.opc"
                ID(skip); COND(NC);

              }
            break;
          case 0xd9:
              {
                /** 0110 0001 1101 1001		mov	%e0, %1				*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1101 1001		mov	%e0, %1				*/",
                           op[0], op[1]);
                  }
                SYNTAX("mov	%e0, %1");
#line 633 "rl78-decode.opc"
                ID(mov); DM2(HL, B, 0); SR(A);

              }
            break;
          case 0xdb:
              {
                /** 0110 0001 1101 1011		ror	%0, %1				*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1101 1011		ror	%0, %1				*/",
                           op[0], op[1]);
                  }
                SYNTAX("ror	%0, %1");
#line 1028 "rl78-decode.opc"
                ID(ror); DR(A); SC(1);

              }
            break;
          case 0xdc:
              {
                /** 0110 0001 1101 1100		rolc	%0, %1				*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1101 1100		rolc	%0, %1				*/",
                           op[0], op[1]);
                  }
                SYNTAX("rolc	%0, %1");
#line 1022 "rl78-decode.opc"
                ID(rolc); DR(A); SC(1);

              }
            break;
          case 0xdd:
              {
                /** 0110 0001 1101 1101		push	%s1				*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1101 1101		push	%s1				*/",
                           op[0], op[1]);
                  }
                SYNTAX("push	%s1");
#line 1003 "rl78-decode.opc"
                ID(mov); W(); DPUSH(); SR(PSW);

              /*----------------------------------------------------------------------*/

              }
            break;
          case 0xde:
              {
                /** 0110 0001 1101 1110		cmps	%0, %ea1			*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1101 1110		cmps	%0, %ea1			*/",
                           op[0], op[1]);
                  }
                SYNTAX("cmps	%0, %ea1");
#line 532 "rl78-decode.opc"
                ID(cmp); DR(X); SM(HL, IMMU(1)); Fzac;

              /*----------------------------------------------------------------------*/

              }
            break;
          case 0xe0:
              {
                /** 0110 0001 1110 0000		or	%0, %e1				*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1110 0000		or	%0, %e1				*/",
                           op[0], op[1]);
                  }
                SYNTAX("or	%0, %e1");
#line 952 "rl78-decode.opc"
                ID(or); DR(A); SM2(HL, B, 0); Fz;

              }
            break;
          case 0xe2:
              {
                /** 0110 0001 1110 0010		or	%0, %e1				*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1110 0010		or	%0, %e1				*/",
                           op[0], op[1]);
                  }
                SYNTAX("or	%0, %e1");
#line 958 "rl78-decode.opc"
                ID(or); DR(A); SM2(HL, C, 0); Fz;

              }
            break;
          case 0xe3:
              {
                /** 0110 0001 1110 0011		sk%c1					*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1110 0011		sk%c1					*/",
                           op[0], op[1]);
                  }
                SYNTAX("sk%c1");
#line 1103 "rl78-decode.opc"
                ID(skip); COND(H);

              }
            break;
          case 0xe8:
              {
                /** 0110 0001 1110 1000		sk%c1					*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1110 1000		sk%c1					*/",
                           op[0], op[1]);
                  }
                SYNTAX("sk%c1");
#line 1115 "rl78-decode.opc"
                ID(skip); COND(Z);

              /*----------------------------------------------------------------------*/

              }
            break;
          case 0xe9:
              {
                /** 0110 0001 1110 1001		mov	%0, %e1				*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1110 1001		mov	%0, %e1				*/",
                           op[0], op[1]);
                  }
                SYNTAX("mov	%0, %e1");
#line 669 "rl78-decode.opc"
                ID(mov); DR(A); SM2(HL, C, 0);

              }
            break;
          case 0xeb:
              {
                /** 0110 0001 1110 1011		rol	%0, %1				*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1110 1011		rol	%0, %1				*/",
                           op[0], op[1]);
                  }
                SYNTAX("rol	%0, %1");
#line 1019 "rl78-decode.opc"
                ID(rol); DR(A); SC(1);

              }
            break;
          case 0xec:
              {
                /** 0110 0001 1110 1100		retb					*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1110 1100		retb					*/",
                           op[0], op[1]);
                  }
                SYNTAX("retb");
#line 1014 "rl78-decode.opc"
                ID(reti);

              /*----------------------------------------------------------------------*/

              }
            break;
          case 0xed:
              {
                /** 0110 0001 1110 1101		halt					*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1110 1101		halt					*/",
                           op[0], op[1]);
                  }
                SYNTAX("halt");
#line 582 "rl78-decode.opc"
                ID(halt);

              /*----------------------------------------------------------------------*/

              }
            break;
          case 0xee:
          case 0xfe:
              {
                /** 0110 0001 111r 1110		rolwc	%0, %1				*/
#line 1025 "rl78-decode.opc"
                int r AU = (op[1] >> 4) & 0x01;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 111r 1110		rolwc	%0, %1				*/",
                           op[0], op[1]);
                    printf ("  r = 0x%x\n", r);
                  }
                SYNTAX("rolwc	%0, %1");
#line 1025 "rl78-decode.opc"
                ID(rolc); W(); DRW(r); SC(1);

              }
            break;
          case 0xf0:
              {
                /** 0110 0001 1111 0000		xor	%0, %e1				*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1111 0000		xor	%0, %e1				*/",
                           op[0], op[1]);
                  }
                SYNTAX("xor	%0, %e1");
#line 1256 "rl78-decode.opc"
                ID(xor); DR(A); SM2(HL, B, 0); Fz;

              }
            break;
          case 0xf2:
              {
                /** 0110 0001 1111 0010		xor	%0, %e1				*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1111 0010		xor	%0, %e1				*/",
                           op[0], op[1]);
                  }
                SYNTAX("xor	%0, %e1");
#line 1262 "rl78-decode.opc"
                ID(xor); DR(A); SM2(HL, C, 0); Fz;

              }
            break;
          case 0xf3:
              {
                /** 0110 0001 1111 0011		sk%c1					*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1111 0011		sk%c1					*/",
                           op[0], op[1]);
                  }
                SYNTAX("sk%c1");
#line 1109 "rl78-decode.opc"
                ID(skip); COND(NH);

              }
            break;
          case 0xf8:
              {
                /** 0110 0001 1111 1000		sk%c1					*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1111 1000		sk%c1					*/",
                           op[0], op[1]);
                  }
                SYNTAX("sk%c1");
#line 1112 "rl78-decode.opc"
                ID(skip); COND(NZ);

              }
            break;
          case 0xf9:
              {
                /** 0110 0001 1111 1001		mov	%e0, %1				*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1111 1001		mov	%e0, %1				*/",
                           op[0], op[1]);
                  }
                SYNTAX("mov	%e0, %1");
#line 642 "rl78-decode.opc"
                ID(mov); DM2(HL, C, 0); SR(A);

              }
            break;
          case 0xfb:
              {
                /** 0110 0001 1111 1011		rorc	%0, %1				*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1111 1011		rorc	%0, %1				*/",
                           op[0], op[1]);
                  }
                SYNTAX("rorc	%0, %1");
#line 1031 "rl78-decode.opc"
                ID(rorc); DR(A); SC(1);

              /*----------------------------------------------------------------------*/

              /* Note that the branch insns need to be listed before the shift
                 ones, as "shift count of zero" means "branch insn" */

              }
            break;
          case 0xfc:
              {
                /** 0110 0001 1111 1100		reti					*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1111 1100		reti					*/",
                           op[0], op[1]);
                  }
                SYNTAX("reti");
#line 1011 "rl78-decode.opc"
                ID(reti);

              }
            break;
          case 0xfd:
              {
                /** 0110 0001 1111 1101	stop						*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0110 0001 1111 1101	stop						*/",
                           op[0], op[1]);
                  }
                SYNTAX("stop");
#line 1120 "rl78-decode.opc"
                ID(stop);

              /*----------------------------------------------------------------------*/

              }
            break;
          default: UNSUPPORTED(); break;
        }
      break;
    case 0x68:
        {
          /** 0110 1000			movw	%e0, %1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0110 1000			movw	%e0, %1				*/",
                     op[0]);
            }
          SYNTAX("movw	%e0, %1");
#line 880 "rl78-decode.opc"
          ID(mov); W(); DM(C, IMMU(2)); SR(AX);

        }
      break;
    case 0x69:
        {
          /** 0110 1001			movw	%0, %e1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0110 1001			movw	%0, %e1				*/",
                     op[0]);
            }
          SYNTAX("movw	%0, %e1");
#line 871 "rl78-decode.opc"
          ID(mov); W(); DR(AX); SM(C, IMMU(2));

        }
      break;
    case 0x6a:
        {
          /** 0110 1010	       		or	%0, #%1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0110 1010	       		or	%0, #%1				*/",
                     op[0]);
            }
          SYNTAX("or	%0, #%1");
#line 973 "rl78-decode.opc"
          ID(or); DM(None, SADDR); SC(IMMU(1)); Fz;

        /*----------------------------------------------------------------------*/

        }
      break;
    case 0x6b:
        {
          /** 0110 1011	       		or	%0, %1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0110 1011	       		or	%0, %1				*/",
                     op[0]);
            }
          SYNTAX("or	%0, %1");
#line 970 "rl78-decode.opc"
          ID(or); DR(A); SM(None, SADDR); Fz;

        }
      break;
    case 0x6c:
        {
          /** 0110 1100	       		or	%0, #%1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0110 1100	       		or	%0, #%1				*/",
                     op[0]);
            }
          SYNTAX("or	%0, #%1");
#line 961 "rl78-decode.opc"
          ID(or); DR(A); SC(IMMU(1)); Fz;

        }
      break;
    case 0x6d:
        {
          /** 0110 1101			or	%0, %e1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0110 1101			or	%0, %e1				*/",
                     op[0]);
            }
          SYNTAX("or	%0, %e1");
#line 949 "rl78-decode.opc"
          ID(or); DR(A); SM(HL, 0); Fz;

        }
      break;
    case 0x6e:
        {
          /** 0110 1110			or	%0, %ea1			*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0110 1110			or	%0, %ea1			*/",
                     op[0]);
            }
          SYNTAX("or	%0, %ea1");
#line 955 "rl78-decode.opc"
          ID(or); DR(A); SM(HL, IMMU(1)); Fz;

        }
      break;
    case 0x6f:
        {
          /** 0110 1111			or	%0, %e!1			*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0110 1111			or	%0, %e!1			*/",
                     op[0]);
            }
          SYNTAX("or	%0, %e!1");
#line 946 "rl78-decode.opc"
          ID(or); DR(A); SM(None, IMMU(2)); Fz;

        }
      break;
    case 0x70:
    case 0x72:
    case 0x73:
    case 0x74:
    case 0x75:
    case 0x76:
    case 0x77:
        {
          /** 0111 0rba			mov	%0, %1				*/
#line 702 "rl78-decode.opc"
          int rba AU = op[0] & 0x07;
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0111 0rba			mov	%0, %1				*/",
                     op[0]);
              printf ("  rba = 0x%x\n", rba);
            }
          SYNTAX("mov	%0, %1");
#line 702 "rl78-decode.opc"
          ID(mov); DRB(rba); SR(A);

        }
      break;
    case 0x71:
        GETBYTE ();
        switch (op[1] & 0xff)
        {
          case 0x00:
          case 0x10:
          case 0x20:
          case 0x30:
          case 0x40:
          case 0x50:
          case 0x60:
          case 0x70:
              {
                /** 0111 0001 0bit 0000		set1	%e!0				*/
#line 1052 "rl78-decode.opc"
                int bit AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0111 0001 0bit 0000		set1	%e!0				*/",
                           op[0], op[1]);
                    printf ("  bit = 0x%x\n", bit);
                  }
                SYNTAX("set1	%e!0");
#line 1052 "rl78-decode.opc"
                ID(mov); DM(None, IMMU(2)); DB(bit); SC(1);

              }
            break;
          case 0x01:
          case 0x11:
          case 0x21:
          case 0x31:
          case 0x41:
          case 0x51:
          case 0x61:
          case 0x71:
              {
                /** 0111 0001 0bit 0001		mov1	%0, cy				*/
#line 809 "rl78-decode.opc"
                int bit AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0111 0001 0bit 0001		mov1	%0, cy				*/",
                           op[0], op[1]);
                    printf ("  bit = 0x%x\n", bit);
                  }
                SYNTAX("mov1	%0, cy");
#line 809 "rl78-decode.opc"
                ID(mov); DM(None, SADDR); DB(bit); SCY();

              }
            break;
          case 0x02:
          case 0x12:
          case 0x22:
          case 0x32:
          case 0x42:
          case 0x52:
          case 0x62:
          case 0x72:
              {
                /** 0111 0001 0bit 0010		set1	%0				*/
#line 1070 "rl78-decode.opc"
                int bit AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0111 0001 0bit 0010		set1	%0				*/",
                           op[0], op[1]);
                    printf ("  bit = 0x%x\n", bit);
                  }
                SYNTAX("set1	%0");
#line 1070 "rl78-decode.opc"
                ID(mov); DM(None, SADDR); DB(bit); SC(1);

              /*----------------------------------------------------------------------*/

              }
            break;
          case 0x03:
          case 0x13:
          case 0x23:
          case 0x33:
          case 0x43:
          case 0x53:
          case 0x63:
          case 0x73:
              {
                /** 0111 0001 0bit 0011		clr1	%0				*/
#line 462 "rl78-decode.opc"
                int bit AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0111 0001 0bit 0011		clr1	%0				*/",
                           op[0], op[1]);
                    printf ("  bit = 0x%x\n", bit);
                  }
                SYNTAX("clr1	%0");
#line 462 "rl78-decode.opc"
                ID(mov); DM(None, SADDR); DB(bit); SC(0);

              /*----------------------------------------------------------------------*/

              }
            break;
          case 0x04:
          case 0x14:
          case 0x24:
          case 0x34:
          case 0x44:
          case 0x54:
          case 0x64:
          case 0x74:
              {
                /** 0111 0001 0bit 0100		mov1	cy, %1				*/
#line 803 "rl78-decode.opc"
                int bit AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0111 0001 0bit 0100		mov1	cy, %1				*/",
                           op[0], op[1]);
                    printf ("  bit = 0x%x\n", bit);
                  }
                SYNTAX("mov1	cy, %1");
#line 803 "rl78-decode.opc"
                ID(mov); DCY(); SM(None, SADDR); SB(bit);

              }
            break;
          case 0x05:
          case 0x15:
          case 0x25:
          case 0x35:
          case 0x45:
          case 0x55:
          case 0x65:
          case 0x75:
              {
                /** 0111 0001 0bit 0101		and1	cy, %s1				*/
#line 332 "rl78-decode.opc"
                int bit AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0111 0001 0bit 0101		and1	cy, %s1				*/",
                           op[0], op[1]);
                    printf ("  bit = 0x%x\n", bit);
                  }
                SYNTAX("and1	cy, %s1");
#line 332 "rl78-decode.opc"
                ID(and); DCY(); SM(None, SADDR); SB(bit);

              /*----------------------------------------------------------------------*/

              /* Note that the branch insns need to be listed before the shift
                 ones, as "shift count of zero" means "branch insn" */

              }
            break;
          case 0x06:
          case 0x16:
          case 0x26:
          case 0x36:
          case 0x46:
          case 0x56:
          case 0x66:
          case 0x76:
              {
                /** 0111 0001 0bit 0110		or1	cy, %s1				*/
#line 987 "rl78-decode.opc"
                int bit AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0111 0001 0bit 0110		or1	cy, %s1				*/",
                           op[0], op[1]);
                    printf ("  bit = 0x%x\n", bit);
                  }
                SYNTAX("or1	cy, %s1");
#line 987 "rl78-decode.opc"
                ID(or); DCY(); SM(None, SADDR); SB(bit);

              /*----------------------------------------------------------------------*/

              }
            break;
          case 0x07:
          case 0x17:
          case 0x27:
          case 0x37:
          case 0x47:
          case 0x57:
          case 0x67:
          case 0x77:
              {
                /** 0111 0001 0bit 0111		xor1	cy, %s1				*/
#line 1291 "rl78-decode.opc"
                int bit AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0111 0001 0bit 0111		xor1	cy, %s1				*/",
                           op[0], op[1]);
                    printf ("  bit = 0x%x\n", bit);
                  }
                SYNTAX("xor1	cy, %s1");
#line 1291 "rl78-decode.opc"
                ID(xor); DCY(); SM(None, SADDR); SB(bit);

              /*----------------------------------------------------------------------*/

              }
            break;
          case 0x08:
          case 0x18:
          case 0x28:
          case 0x38:
          case 0x48:
          case 0x58:
          case 0x68:
          case 0x78:
              {
                /** 0111 0001 0bit 1000		clr1	%e!0				*/
#line 444 "rl78-decode.opc"
                int bit AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0111 0001 0bit 1000		clr1	%e!0				*/",
                           op[0], op[1]);
                    printf ("  bit = 0x%x\n", bit);
                  }
                SYNTAX("clr1	%e!0");
#line 444 "rl78-decode.opc"
                ID(mov); DM(None, IMMU(2)); DB(bit); SC(0);

              }
            break;
          case 0x09:
          case 0x19:
          case 0x29:
          case 0x39:
          case 0x49:
          case 0x59:
          case 0x69:
          case 0x79:
              {
                /** 0111 0001 0bit 1001		mov1	%s0, cy				*/
#line 812 "rl78-decode.opc"
                int bit AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0111 0001 0bit 1001		mov1	%s0, cy				*/",
                           op[0], op[1]);
                    printf ("  bit = 0x%x\n", bit);
                  }
                SYNTAX("mov1	%s0, cy");
#line 812 "rl78-decode.opc"
                ID(mov); DM(None, SFR); DB(bit); SCY();

              /*----------------------------------------------------------------------*/

              }
            break;
          case 0x0a:
          case 0x1a:
          case 0x2a:
          case 0x3a:
          case 0x4a:
          case 0x5a:
          case 0x6a:
          case 0x7a:
              {
                /** 0111 0001 0bit 1010		set1	%s0				*/
#line 1064 "rl78-decode.opc"
                int bit AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0111 0001 0bit 1010		set1	%s0				*/",
                           op[0], op[1]);
                    printf ("  bit = 0x%x\n", bit);
                  }
                SYNTAX("set1	%s0");
#line 1064 "rl78-decode.opc"
                op0 = SFR;
                ID(mov); DM(None, op0); DB(bit); SC(1);
                if (op0 == RL78_SFR_PSW && bit == 7)
                  rl78->syntax = "ei";

              }
            break;
          case 0x0b:
          case 0x1b:
          case 0x2b:
          case 0x3b:
          case 0x4b:
          case 0x5b:
          case 0x6b:
          case 0x7b:
              {
                /** 0111 0001 0bit 1011		clr1	%s0				*/
#line 456 "rl78-decode.opc"
                int bit AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0111 0001 0bit 1011		clr1	%s0				*/",
                           op[0], op[1]);
                    printf ("  bit = 0x%x\n", bit);
                  }
                SYNTAX("clr1	%s0");
#line 456 "rl78-decode.opc"
                op0 = SFR;
                ID(mov); DM(None, op0); DB(bit); SC(0);
                if (op0 == RL78_SFR_PSW && bit == 7)
                  rl78->syntax = "di";

              }
            break;
          case 0x0c:
          case 0x1c:
          case 0x2c:
          case 0x3c:
          case 0x4c:
          case 0x5c:
          case 0x6c:
          case 0x7c:
              {
                /** 0111 0001 0bit 1100		mov1	cy, %s1				*/
#line 806 "rl78-decode.opc"
                int bit AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0111 0001 0bit 1100		mov1	cy, %s1				*/",
                           op[0], op[1]);
                    printf ("  bit = 0x%x\n", bit);
                  }
                SYNTAX("mov1	cy, %s1");
#line 806 "rl78-decode.opc"
                ID(mov); DCY(); SM(None, SFR); SB(bit);

              }
            break;
          case 0x0d:
          case 0x1d:
          case 0x2d:
          case 0x3d:
          case 0x4d:
          case 0x5d:
          case 0x6d:
          case 0x7d:
              {
                /** 0111 0001 0bit 1101		and1	cy, %s1				*/
#line 329 "rl78-decode.opc"
                int bit AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0111 0001 0bit 1101		and1	cy, %s1				*/",
                           op[0], op[1]);
                    printf ("  bit = 0x%x\n", bit);
                  }
                SYNTAX("and1	cy, %s1");
#line 329 "rl78-decode.opc"
                ID(and); DCY(); SM(None, SFR); SB(bit);

              }
            break;
          case 0x0e:
          case 0x1e:
          case 0x2e:
          case 0x3e:
          case 0x4e:
          case 0x5e:
          case 0x6e:
          case 0x7e:
              {
                /** 0111 0001 0bit 1110		or1	cy, %s1				*/
#line 984 "rl78-decode.opc"
                int bit AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0111 0001 0bit 1110		or1	cy, %s1				*/",
                           op[0], op[1]);
                    printf ("  bit = 0x%x\n", bit);
                  }
                SYNTAX("or1	cy, %s1");
#line 984 "rl78-decode.opc"
                ID(or); DCY(); SM(None, SFR); SB(bit);

              }
            break;
          case 0x0f:
          case 0x1f:
          case 0x2f:
          case 0x3f:
          case 0x4f:
          case 0x5f:
          case 0x6f:
          case 0x7f:
              {
                /** 0111 0001 0bit 1111		xor1	cy, %s1				*/
#line 1288 "rl78-decode.opc"
                int bit AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0111 0001 0bit 1111		xor1	cy, %s1				*/",
                           op[0], op[1]);
                    printf ("  bit = 0x%x\n", bit);
                  }
                SYNTAX("xor1	cy, %s1");
#line 1288 "rl78-decode.opc"
                ID(xor); DCY(); SM(None, SFR); SB(bit);

              }
            break;
          case 0x80:
              {
                /** 0111 0001 1000 0000		set1	cy				*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0111 0001 1000 0000		set1	cy				*/",
                           op[0], op[1]);
                  }
                SYNTAX("set1	cy");
#line 1061 "rl78-decode.opc"
                ID(mov); DCY(); SC(1);

              }
            break;
          case 0x81:
          case 0x91:
          case 0xa1:
          case 0xb1:
          case 0xc1:
          case 0xd1:
          case 0xe1:
          case 0xf1:
              {
                /** 0111 0001 1bit 0001		mov1	%e0, cy				*/
#line 791 "rl78-decode.opc"
                int bit AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0111 0001 1bit 0001		mov1	%e0, cy				*/",
                           op[0], op[1]);
                    printf ("  bit = 0x%x\n", bit);
                  }
                SYNTAX("mov1	%e0, cy");
#line 791 "rl78-decode.opc"
                ID(mov); DM(HL, 0); DB(bit); SCY();

              }
            break;
          case 0x82:
          case 0x92:
          case 0xa2:
          case 0xb2:
          case 0xc2:
          case 0xd2:
          case 0xe2:
          case 0xf2:
              {
                /** 0111 0001 1bit 0010		set1	%e0				*/
#line 1055 "rl78-decode.opc"
                int bit AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0111 0001 1bit 0010		set1	%e0				*/",
                           op[0], op[1]);
                    printf ("  bit = 0x%x\n", bit);
                  }
                SYNTAX("set1	%e0");
#line 1055 "rl78-decode.opc"
                ID(mov); DM(HL, 0); DB(bit); SC(1);

              }
            break;
          case 0x83:
          case 0x93:
          case 0xa3:
          case 0xb3:
          case 0xc3:
          case 0xd3:
          case 0xe3:
          case 0xf3:
              {
                /** 0111 0001 1bit 0011		clr1	%e0				*/
#line 447 "rl78-decode.opc"
                int bit AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0111 0001 1bit 0011		clr1	%e0				*/",
                           op[0], op[1]);
                    printf ("  bit = 0x%x\n", bit);
                  }
                SYNTAX("clr1	%e0");
#line 447 "rl78-decode.opc"
                ID(mov); DM(HL, 0); DB(bit); SC(0);

              }
            break;
          case 0x84:
          case 0x94:
          case 0xa4:
          case 0xb4:
          case 0xc4:
          case 0xd4:
          case 0xe4:
          case 0xf4:
              {
                /** 0111 0001 1bit 0100		mov1	cy, %e1				*/
#line 797 "rl78-decode.opc"
                int bit AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0111 0001 1bit 0100		mov1	cy, %e1				*/",
                           op[0], op[1]);
                    printf ("  bit = 0x%x\n", bit);
                  }
                SYNTAX("mov1	cy, %e1");
#line 797 "rl78-decode.opc"
                ID(mov); DCY(); SM(HL, 0); SB(bit);

              }
            break;
          case 0x85:
          case 0x95:
          case 0xa5:
          case 0xb5:
          case 0xc5:
          case 0xd5:
          case 0xe5:
          case 0xf5:
              {
                /** 0111 0001 1bit 0101		and1	cy, %e1			*/
#line 323 "rl78-decode.opc"
                int bit AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0111 0001 1bit 0101		and1	cy, %e1			*/",
                           op[0], op[1]);
                    printf ("  bit = 0x%x\n", bit);
                  }
                SYNTAX("and1	cy, %e1");
#line 323 "rl78-decode.opc"
                ID(and); DCY(); SM(HL, 0); SB(bit);

              }
            break;
          case 0x86:
          case 0x96:
          case 0xa6:
          case 0xb6:
          case 0xc6:
          case 0xd6:
          case 0xe6:
          case 0xf6:
              {
                /** 0111 0001 1bit 0110		or1	cy, %e1				*/
#line 978 "rl78-decode.opc"
                int bit AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0111 0001 1bit 0110		or1	cy, %e1				*/",
                           op[0], op[1]);
                    printf ("  bit = 0x%x\n", bit);
                  }
                SYNTAX("or1	cy, %e1");
#line 978 "rl78-decode.opc"
                ID(or); DCY(); SM(HL, 0); SB(bit);

              }
            break;
          case 0x87:
          case 0x97:
          case 0xa7:
          case 0xb7:
          case 0xc7:
          case 0xd7:
          case 0xe7:
          case 0xf7:
              {
                /** 0111 0001 1bit 0111		xor1	cy, %e1				*/
#line 1282 "rl78-decode.opc"
                int bit AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0111 0001 1bit 0111		xor1	cy, %e1				*/",
                           op[0], op[1]);
                    printf ("  bit = 0x%x\n", bit);
                  }
                SYNTAX("xor1	cy, %e1");
#line 1282 "rl78-decode.opc"
                ID(xor); DCY(); SM(HL, 0); SB(bit);

              }
            break;
          case 0x88:
              {
                /** 0111 0001 1000 1000		clr1	cy				*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0111 0001 1000 1000		clr1	cy				*/",
                           op[0], op[1]);
                  }
                SYNTAX("clr1	cy");
#line 453 "rl78-decode.opc"
                ID(mov); DCY(); SC(0);

              }
            break;
          case 0x89:
          case 0x99:
          case 0xa9:
          case 0xb9:
          case 0xc9:
          case 0xd9:
          case 0xe9:
          case 0xf9:
              {
                /** 0111 0001 1bit 1001		mov1	%e0, cy				*/
#line 794 "rl78-decode.opc"
                int bit AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0111 0001 1bit 1001		mov1	%e0, cy				*/",
                           op[0], op[1]);
                    printf ("  bit = 0x%x\n", bit);
                  }
                SYNTAX("mov1	%e0, cy");
#line 794 "rl78-decode.opc"
                ID(mov); DR(A); DB(bit); SCY();

              }
            break;
          case 0x8a:
          case 0x9a:
          case 0xaa:
          case 0xba:
          case 0xca:
          case 0xda:
          case 0xea:
          case 0xfa:
              {
                /** 0111 0001 1bit 1010		set1	%0				*/
#line 1058 "rl78-decode.opc"
                int bit AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0111 0001 1bit 1010		set1	%0				*/",
                           op[0], op[1]);
                    printf ("  bit = 0x%x\n", bit);
                  }
                SYNTAX("set1	%0");
#line 1058 "rl78-decode.opc"
                ID(mov); DR(A); DB(bit); SC(1);

              }
            break;
          case 0x8b:
          case 0x9b:
          case 0xab:
          case 0xbb:
          case 0xcb:
          case 0xdb:
          case 0xeb:
          case 0xfb:
              {
                /** 0111 0001 1bit 1011		clr1	%0				*/
#line 450 "rl78-decode.opc"
                int bit AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0111 0001 1bit 1011		clr1	%0				*/",
                           op[0], op[1]);
                    printf ("  bit = 0x%x\n", bit);
                  }
                SYNTAX("clr1	%0");
#line 450 "rl78-decode.opc"
                ID(mov); DR(A); DB(bit); SC(0);

              }
            break;
          case 0x8c:
          case 0x9c:
          case 0xac:
          case 0xbc:
          case 0xcc:
          case 0xdc:
          case 0xec:
          case 0xfc:
              {
                /** 0111 0001 1bit 1100		mov1	cy, %e1				*/
#line 800 "rl78-decode.opc"
                int bit AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0111 0001 1bit 1100		mov1	cy, %e1				*/",
                           op[0], op[1]);
                    printf ("  bit = 0x%x\n", bit);
                  }
                SYNTAX("mov1	cy, %e1");
#line 800 "rl78-decode.opc"
                ID(mov); DCY(); SR(A); SB(bit);

              }
            break;
          case 0x8d:
          case 0x9d:
          case 0xad:
          case 0xbd:
          case 0xcd:
          case 0xdd:
          case 0xed:
          case 0xfd:
              {
                /** 0111 0001 1bit 1101		and1	cy, %1				*/
#line 326 "rl78-decode.opc"
                int bit AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0111 0001 1bit 1101		and1	cy, %1				*/",
                           op[0], op[1]);
                    printf ("  bit = 0x%x\n", bit);
                  }
                SYNTAX("and1	cy, %1");
#line 326 "rl78-decode.opc"
                ID(and); DCY(); SR(A); SB(bit);

              }
            break;
          case 0x8e:
          case 0x9e:
          case 0xae:
          case 0xbe:
          case 0xce:
          case 0xde:
          case 0xee:
          case 0xfe:
              {
                /** 0111 0001 1bit 1110		or1	cy, %1				*/
#line 981 "rl78-decode.opc"
                int bit AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0111 0001 1bit 1110		or1	cy, %1				*/",
                           op[0], op[1]);
                    printf ("  bit = 0x%x\n", bit);
                  }
                SYNTAX("or1	cy, %1");
#line 981 "rl78-decode.opc"
                ID(or); DCY(); SR(A); SB(bit);

              }
            break;
          case 0x8f:
          case 0x9f:
          case 0xaf:
          case 0xbf:
          case 0xcf:
          case 0xdf:
          case 0xef:
          case 0xff:
              {
                /** 0111 0001 1bit 1111		xor1	cy, %1				*/
#line 1285 "rl78-decode.opc"
                int bit AU = (op[1] >> 4) & 0x07;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0111 0001 1bit 1111		xor1	cy, %1				*/",
                           op[0], op[1]);
                    printf ("  bit = 0x%x\n", bit);
                  }
                SYNTAX("xor1	cy, %1");
#line 1285 "rl78-decode.opc"
                ID(xor); DCY(); SR(A); SB(bit);

              }
            break;
          case 0xc0:
              {
                /** 0111 0001 1100 0000		not1	cy				*/
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0111 0001 1100 0000		not1	cy				*/",
                           op[0], op[1]);
                  }
                SYNTAX("not1	cy");
#line 922 "rl78-decode.opc"
                ID(xor); DCY(); SC(1);

              /*----------------------------------------------------------------------*/

              }
            break;
          default: UNSUPPORTED(); break;
        }
      break;
    case 0x78:
        {
          /** 0111 1000			movw	%e0, %1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0111 1000			movw	%e0, %1				*/",
                     op[0]);
            }
          SYNTAX("movw	%e0, %1");
#line 883 "rl78-decode.opc"
          ID(mov); W(); DM(BC, IMMU(2)); SR(AX);

        }
      break;
    case 0x79:
        {
          /** 0111 1001			movw	%0, %e1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0111 1001			movw	%0, %e1				*/",
                     op[0]);
            }
          SYNTAX("movw	%0, %e1");
#line 874 "rl78-decode.opc"
          ID(mov); W(); DR(AX); SM(BC, IMMU(2));

        }
      break;
    case 0x7a:
        {
          /** 0111 1010	       		xor	%0, #%1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0111 1010	       		xor	%0, #%1				*/",
                     op[0]);
            }
          SYNTAX("xor	%0, #%1");
#line 1277 "rl78-decode.opc"
          ID(xor); DM(None, SADDR); SC(IMMU(1)); Fz;

        /*----------------------------------------------------------------------*/

        }
      break;
    case 0x7b:
        {
          /** 0111 1011	       		xor	%0, %1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0111 1011	       		xor	%0, %1				*/",
                     op[0]);
            }
          SYNTAX("xor	%0, %1");
#line 1274 "rl78-decode.opc"
          ID(xor); DR(A); SM(None, SADDR); Fz;

        }
      break;
    case 0x7c:
        {
          /** 0111 1100	       		xor	%0, #%1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0111 1100	       		xor	%0, #%1				*/",
                     op[0]);
            }
          SYNTAX("xor	%0, #%1");
#line 1265 "rl78-decode.opc"
          ID(xor); DR(A); SC(IMMU(1)); Fz;

        }
      break;
    case 0x7d:
        {
          /** 0111 1101			xor	%0, %e1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0111 1101			xor	%0, %e1				*/",
                     op[0]);
            }
          SYNTAX("xor	%0, %e1");
#line 1253 "rl78-decode.opc"
          ID(xor); DR(A); SM(HL, 0); Fz;

        }
      break;
    case 0x7e:
        {
          /** 0111 1110			xor	%0, %ea1			*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0111 1110			xor	%0, %ea1			*/",
                     op[0]);
            }
          SYNTAX("xor	%0, %ea1");
#line 1259 "rl78-decode.opc"
          ID(xor); DR(A); SM(HL, IMMU(1)); Fz;

        }
      break;
    case 0x7f:
        {
          /** 0111 1111			xor	%0, %e!1			*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 0111 1111			xor	%0, %e!1			*/",
                     op[0]);
            }
          SYNTAX("xor	%0, %e!1");
#line 1250 "rl78-decode.opc"
          ID(xor); DR(A); SM(None, IMMU(2)); Fz;

        }
      break;
    case 0x80:
    case 0x81:
    case 0x82:
    case 0x83:
    case 0x84:
    case 0x85:
    case 0x86:
    case 0x87:
        {
          /** 1000 0reg			inc	%0				*/
#line 593 "rl78-decode.opc"
          int reg AU = op[0] & 0x07;
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1000 0reg			inc	%0				*/",
                     op[0]);
              printf ("  reg = 0x%x\n", reg);
            }
          SYNTAX("inc	%0");
#line 593 "rl78-decode.opc"
          ID(add); DRB(reg); SC(1); Fza;

        }
      break;
    case 0x88:
        {
          /** 1000 1000			mov	%0, %ea1			*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1000 1000			mov	%0, %ea1			*/",
                     op[0]);
            }
          SYNTAX("mov	%0, %ea1");
#line 672 "rl78-decode.opc"
          ID(mov); DR(A); SM(SP, IMMU(1));

        }
      break;
    case 0x89:
        {
          /** 1000 1001			mov	%0, %e1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1000 1001			mov	%0, %e1				*/",
                     op[0]);
            }
          SYNTAX("mov	%0, %e1");
#line 654 "rl78-decode.opc"
          ID(mov); DR(A); SM(DE, 0);

        }
      break;
    case 0x8a:
        {
          /** 1000 1010			mov	%0, %ea1			*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1000 1010			mov	%0, %ea1			*/",
                     op[0]);
            }
          SYNTAX("mov	%0, %ea1");
#line 657 "rl78-decode.opc"
          ID(mov); DR(A); SM(DE, IMMU(1));

        }
      break;
    case 0x8b:
        {
          /** 1000 1011			mov	%0, %e1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1000 1011			mov	%0, %e1				*/",
                     op[0]);
            }
          SYNTAX("mov	%0, %e1");
#line 660 "rl78-decode.opc"
          ID(mov); DR(A); SM(HL, 0);

        }
      break;
    case 0x8c:
        {
          /** 1000 1100			mov	%0, %ea1			*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1000 1100			mov	%0, %ea1			*/",
                     op[0]);
            }
          SYNTAX("mov	%0, %ea1");
#line 663 "rl78-decode.opc"
          ID(mov); DR(A); SM(HL, IMMU(1));

        }
      break;
    case 0x8d:
        {
          /** 1000 1101			mov	%0, %1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1000 1101			mov	%0, %1				*/",
                     op[0]);
            }
          SYNTAX("mov	%0, %1");
#line 696 "rl78-decode.opc"
          ID(mov); DR(A); SM(None, SADDR);

        }
      break;
    case 0x8e:
        {
          /** 1000 1110			mov	%0, %s1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1000 1110			mov	%0, %s1				*/",
                     op[0]);
            }
          SYNTAX("mov	%0, %s1");
#line 693 "rl78-decode.opc"
          ID(mov); DR(A); SM(None, SFR);

        }
      break;
    case 0x8f:
        {
          /** 1000 1111			mov	%0, %e!1			*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1000 1111			mov	%0, %e!1			*/",
                     op[0]);
            }
          SYNTAX("mov	%0, %e!1");
#line 651 "rl78-decode.opc"
          ID(mov); DR(A); SM(None, IMMU(2));

        }
      break;
    case 0x90:
    case 0x91:
    case 0x92:
    case 0x93:
    case 0x94:
    case 0x95:
    case 0x96:
    case 0x97:
        {
          /** 1001 0reg			dec	%0				*/
#line 560 "rl78-decode.opc"
          int reg AU = op[0] & 0x07;
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1001 0reg			dec	%0				*/",
                     op[0]);
              printf ("  reg = 0x%x\n", reg);
            }
          SYNTAX("dec	%0");
#line 560 "rl78-decode.opc"
          ID(sub); DRB(reg); SC(1); Fza;

        }
      break;
    case 0x98:
        {
          /** 1001 1000			mov	%a0, %1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1001 1000			mov	%a0, %1				*/",
                     op[0]);
            }
          SYNTAX("mov	%a0, %1");
#line 648 "rl78-decode.opc"
          ID(mov); DM(SP, IMMU(1)); SR(A);

        }
      break;
    case 0x99:
        {
          /** 1001 1001			mov	%e0, %1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1001 1001			mov	%e0, %1				*/",
                     op[0]);
            }
          SYNTAX("mov	%e0, %1");
#line 621 "rl78-decode.opc"
          ID(mov); DM(DE, 0); SR(A);

        }
      break;
    case 0x9a:
        {
          /** 1001 1010			mov	%ea0, %1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1001 1010			mov	%ea0, %1				*/",
                     op[0]);
            }
          SYNTAX("mov	%ea0, %1");
#line 627 "rl78-decode.opc"
          ID(mov); DM(DE, IMMU(1)); SR(A);

        }
      break;
    case 0x9b:
        {
          /** 1001 1011			mov	%e0, %1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1001 1011			mov	%e0, %1				*/",
                     op[0]);
            }
          SYNTAX("mov	%e0, %1");
#line 630 "rl78-decode.opc"
          ID(mov); DM(HL, 0); SR(A);

        }
      break;
    case 0x9c:
        {
          /** 1001 1100			mov	%ea0, %1			*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1001 1100			mov	%ea0, %1			*/",
                     op[0]);
            }
          SYNTAX("mov	%ea0, %1");
#line 639 "rl78-decode.opc"
          ID(mov); DM(HL, IMMU(1)); SR(A);

        }
      break;
    case 0x9d:
        {
          /** 1001 1101			mov	%0, %1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1001 1101			mov	%0, %1				*/",
                     op[0]);
            }
          SYNTAX("mov	%0, %1");
#line 753 "rl78-decode.opc"
          ID(mov); DM(None, SADDR); SR(A);

        }
      break;
    case 0x9e:
        {
          /** 1001 1110			mov	%s0, %1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1001 1110			mov	%s0, %1				*/",
                     op[0]);
            }
          SYNTAX("mov	%s0, %1");
#line 786 "rl78-decode.opc"
          ID(mov); DM(None, SFR); SR(A);

        /*----------------------------------------------------------------------*/

        }
      break;
    case 0x9f:
        {
          /** 1001 1111			mov	%e!0, %1			*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1001 1111			mov	%e!0, %1			*/",
                     op[0]);
            }
          SYNTAX("mov	%e!0, %1");
#line 618 "rl78-decode.opc"
          ID(mov); DM(None, IMMU(2)); SR(A);

        }
      break;
    case 0xa0:
        {
          /** 1010 0000			inc	%e!0				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1010 0000			inc	%e!0				*/",
                     op[0]);
            }
          SYNTAX("inc	%e!0");
#line 587 "rl78-decode.opc"
          ID(add); DM(None, IMMU(2)); SC(1); Fza;

        }
      break;
    case 0xa1:
    case 0xa3:
    case 0xa5:
    case 0xa7:
        {
          /** 1010 0rg1			incw	%0				*/
#line 607 "rl78-decode.opc"
          int rg AU = (op[0] >> 1) & 0x03;
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1010 0rg1			incw	%0				*/",
                     op[0]);
              printf ("  rg = 0x%x\n", rg);
            }
          SYNTAX("incw	%0");
#line 607 "rl78-decode.opc"
          ID(add); W(); DRW(rg); SC(1);

        }
      break;
    case 0xa2:
        {
          /** 1010 0010			incw	%e!0				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1010 0010			incw	%e!0				*/",
                     op[0]);
            }
          SYNTAX("incw	%e!0");
#line 601 "rl78-decode.opc"
          ID(add); W(); DM(None, IMMU(2)); SC(1);

        }
      break;
    case 0xa4:
        {
          /** 1010 0100			inc	%0				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1010 0100			inc	%0				*/",
                     op[0]);
            }
          SYNTAX("inc	%0");
#line 596 "rl78-decode.opc"
          ID(add); DM(None, SADDR); SC(1); Fza;

        /*----------------------------------------------------------------------*/

        }
      break;
    case 0xa6:
        {
          /** 1010 0110			incw	%0				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1010 0110			incw	%0				*/",
                     op[0]);
            }
          SYNTAX("incw	%0");
#line 610 "rl78-decode.opc"
          ID(add); W(); DM(None, SADDR); SC(1);

        /*----------------------------------------------------------------------*/

        }
      break;
    case 0xa8:
        {
          /** 1010 1000			movw	%0, %a1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1010 1000			movw	%0, %a1				*/",
                     op[0]);
            }
          SYNTAX("movw	%0, %a1");
#line 856 "rl78-decode.opc"
          ID(mov); W(); DR(AX); SM(SP, IMMU(1));

        }
      break;
    case 0xa9:
        {
          /** 1010 1001			movw	%0, %e1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1010 1001			movw	%0, %e1				*/",
                     op[0]);
            }
          SYNTAX("movw	%0, %e1");
#line 844 "rl78-decode.opc"
          ID(mov); W(); DR(AX); SM(DE, 0);

        }
      break;
    case 0xaa:
        {
          /** 1010 1010			movw	%0, %ea1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1010 1010			movw	%0, %ea1				*/",
                     op[0]);
            }
          SYNTAX("movw	%0, %ea1");
#line 847 "rl78-decode.opc"
          ID(mov); W(); DR(AX); SM(DE, IMMU(1));

        }
      break;
    case 0xab:
        {
          /** 1010 1011			movw	%0, %e1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1010 1011			movw	%0, %e1				*/",
                     op[0]);
            }
          SYNTAX("movw	%0, %e1");
#line 850 "rl78-decode.opc"
          ID(mov); W(); DR(AX); SM(HL, 0);

        }
      break;
    case 0xac:
        {
          /** 1010 1100			movw	%0, %ea1			*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1010 1100			movw	%0, %ea1			*/",
                     op[0]);
            }
          SYNTAX("movw	%0, %ea1");
#line 853 "rl78-decode.opc"
          ID(mov); W(); DR(AX); SM(HL, IMMU(1));

        }
      break;
    case 0xad:
        {
          /** 1010 1101			movw	%0, %1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1010 1101			movw	%0, %1				*/",
                     op[0]);
            }
          SYNTAX("movw	%0, %1");
#line 886 "rl78-decode.opc"
          ID(mov); W(); DR(AX); SM(None, SADDR);

        }
      break;
    case 0xae:
        {
          /** 1010 1110			movw	%0, %s1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1010 1110			movw	%0, %s1				*/",
                     op[0]);
            }
          SYNTAX("movw	%0, %s1");
#line 889 "rl78-decode.opc"
          ID(mov); W(); DR(AX); SM(None, SFR);

        }
      break;
    case 0xaf:
        {
          /** 1010 1111			movw	%0, %e!1			*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1010 1111			movw	%0, %e!1			*/",
                     op[0]);
            }
          SYNTAX("movw	%0, %e!1");
#line 840 "rl78-decode.opc"
          ID(mov); W(); DR(AX); SM(None, IMMU(2));


        }
      break;
    case 0xb0:
        {
          /** 1011 0000			dec	%e!0				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1011 0000			dec	%e!0				*/",
                     op[0]);
            }
          SYNTAX("dec	%e!0");
#line 554 "rl78-decode.opc"
          ID(sub); DM(None, IMMU(2)); SC(1); Fza;

        }
      break;
    case 0xb1:
    case 0xb3:
    case 0xb5:
    case 0xb7:
        {
          /** 1011 0rg1 			decw	%0				*/
#line 574 "rl78-decode.opc"
          int rg AU = (op[0] >> 1) & 0x03;
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1011 0rg1 			decw	%0				*/",
                     op[0]);
              printf ("  rg = 0x%x\n", rg);
            }
          SYNTAX("decw	%0");
#line 574 "rl78-decode.opc"
          ID(sub); W(); DRW(rg); SC(1);

        }
      break;
    case 0xb2:
        {
          /** 1011 0010			decw	%e!0				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1011 0010			decw	%e!0				*/",
                     op[0]);
            }
          SYNTAX("decw	%e!0");
#line 568 "rl78-decode.opc"
          ID(sub); W(); DM(None, IMMU(2)); SC(1);

        }
      break;
    case 0xb4:
        {
          /** 1011 0100			dec	%0				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1011 0100			dec	%0				*/",
                     op[0]);
            }
          SYNTAX("dec	%0");
#line 563 "rl78-decode.opc"
          ID(sub); DM(None, SADDR); SC(1); Fza;

        /*----------------------------------------------------------------------*/

        }
      break;
    case 0xb6:
        {
          /** 1011 0110			decw	%0				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1011 0110			decw	%0				*/",
                     op[0]);
            }
          SYNTAX("decw	%0");
#line 577 "rl78-decode.opc"
          ID(sub); W(); DM(None, SADDR); SC(1);

        /*----------------------------------------------------------------------*/

        }
      break;
    case 0xb8:
        {
          /** 1011 1000			movw	%a0, %1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1011 1000			movw	%a0, %1				*/",
                     op[0]);
            }
          SYNTAX("movw	%a0, %1");
#line 837 "rl78-decode.opc"
          ID(mov); W(); DM(SP, IMMU(1)); SR(AX);

        }
      break;
    case 0xb9:
        {
          /** 1011 1001			movw	%e0, %1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1011 1001			movw	%e0, %1				*/",
                     op[0]);
            }
          SYNTAX("movw	%e0, %1");
#line 825 "rl78-decode.opc"
          ID(mov); W(); DM(DE, 0); SR(AX);

        }
      break;
    case 0xba:
        {
          /** 1011 1010			movw	%ea0, %1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1011 1010			movw	%ea0, %1				*/",
                     op[0]);
            }
          SYNTAX("movw	%ea0, %1");
#line 828 "rl78-decode.opc"
          ID(mov); W(); DM(DE, IMMU(1)); SR(AX);

        }
      break;
    case 0xbb:
        {
          /** 1011 1011			movw	%e0, %1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1011 1011			movw	%e0, %1				*/",
                     op[0]);
            }
          SYNTAX("movw	%e0, %1");
#line 831 "rl78-decode.opc"
          ID(mov); W(); DM(HL, 0); SR(AX);

        }
      break;
    case 0xbc:
        {
          /** 1011 1100			movw	%ea0, %1			*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1011 1100			movw	%ea0, %1			*/",
                     op[0]);
            }
          SYNTAX("movw	%ea0, %1");
#line 834 "rl78-decode.opc"
          ID(mov); W(); DM(HL, IMMU(1)); SR(AX);

        }
      break;
    case 0xbd:
        {
          /** 1011 1101			movw	%0, %1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1011 1101			movw	%0, %1				*/",
                     op[0]);
            }
          SYNTAX("movw	%0, %1");
#line 901 "rl78-decode.opc"
          ID(mov); W(); DM(None, SADDR); SR(AX);

        }
      break;
    case 0xbe:
        {
          /** 1011 1110			movw	%s0, %1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1011 1110			movw	%s0, %1				*/",
                     op[0]);
            }
          SYNTAX("movw	%s0, %1");
#line 907 "rl78-decode.opc"
          ID(mov); W(); DM(None, SFR); SR(AX);

        /*----------------------------------------------------------------------*/

        }
      break;
    case 0xbf:
        {
          /** 1011 1111			movw	%e!0, %1			*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1011 1111			movw	%e!0, %1			*/",
                     op[0]);
            }
          SYNTAX("movw	%e!0, %1");
#line 822 "rl78-decode.opc"
          ID(mov); W(); DM(None, IMMU(2)); SR(AX);

        }
      break;
    case 0xc0:
    case 0xc2:
    case 0xc4:
    case 0xc6:
        {
          /** 1100 0rg0			pop	%0				*/
#line 992 "rl78-decode.opc"
          int rg AU = (op[0] >> 1) & 0x03;
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1100 0rg0			pop	%0				*/",
                     op[0]);
              printf ("  rg = 0x%x\n", rg);
            }
          SYNTAX("pop	%0");
#line 992 "rl78-decode.opc"
          ID(mov); W(); DRW(rg); SPOP();

        }
      break;
    case 0xc1:
    case 0xc3:
    case 0xc5:
    case 0xc7:
        {
          /** 1100 0rg1			push	%1				*/
#line 1000 "rl78-decode.opc"
          int rg AU = (op[0] >> 1) & 0x03;
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1100 0rg1			push	%1				*/",
                     op[0]);
              printf ("  rg = 0x%x\n", rg);
            }
          SYNTAX("push	%1");
#line 1000 "rl78-decode.opc"
          ID(mov); W(); DPUSH(); SRW(rg);

        }
      break;
    case 0xc8:
        {
          /** 1100 1000			mov	%a0, #%1			*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1100 1000			mov	%a0, #%1			*/",
                     op[0]);
            }
          SYNTAX("mov	%a0, #%1");
#line 645 "rl78-decode.opc"
          ID(mov); DM(SP, IMMU(1)); SC(IMMU(1));

        }
      break;
    case 0xc9:
        {
          /** 1100 1001			movw	%0, #%1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1100 1001			movw	%0, #%1				*/",
                     op[0]);
            }
          SYNTAX("movw	%0, #%1");
#line 898 "rl78-decode.opc"
          ID(mov); W(); DM(None, SADDR); SC(IMMU(2));

        }
      break;
    case 0xca:
        {
          /** 1100 1010			mov	%ea0, #%1			*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1100 1010			mov	%ea0, #%1			*/",
                     op[0]);
            }
          SYNTAX("mov	%ea0, #%1");
#line 624 "rl78-decode.opc"
          ID(mov); DM(DE, IMMU(1)); SC(IMMU(1));

        }
      break;
    case 0xcb:
        {
          /** 1100 1011			movw	%s0, #%1			*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1100 1011			movw	%s0, #%1			*/",
                     op[0]);
            }
          SYNTAX("movw	%s0, #%1");
#line 904 "rl78-decode.opc"
          ID(mov); W(); DM(None, SFR); SC(IMMU(2));

        }
      break;
    case 0xcc:
        {
          /** 1100 1100			mov	%ea0, #%1			*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1100 1100			mov	%ea0, #%1			*/",
                     op[0]);
            }
          SYNTAX("mov	%ea0, #%1");
#line 636 "rl78-decode.opc"
          ID(mov); DM(HL, IMMU(1)); SC(IMMU(1));

        }
      break;
    case 0xcd:
        {
          /** 1100 1101			mov	%0, #%1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1100 1101			mov	%0, #%1				*/",
                     op[0]);
            }
          SYNTAX("mov	%0, #%1");
#line 750 "rl78-decode.opc"
          ID(mov); DM(None, SADDR); SC(IMMU(1));

        }
      break;
    case 0xce:
        {
          /** 1100 1110			mov	%s0, #%1			*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1100 1110			mov	%s0, #%1			*/",
                     op[0]);
            }
          SYNTAX("mov	%s0, #%1");
#line 756 "rl78-decode.opc"
          op0 = SFR;
          op1 = IMMU(1);
          ID(mov); DM(None, op0); SC(op1);
          if (op0 == 0xffffb && isa == RL78_ISA_G14)
            switch (op1)
              {
              case 0x01:
        	rl78->syntax = "mulhu"; ID(mulhu);
        	break;
              case 0x02:
        	rl78->syntax = "mulh"; ID(mulh);
        	break;
              case 0x03:
        	rl78->syntax = "divhu"; ID(divhu);
        	break;
              case 0x04:
        	rl78->syntax = "divwu <old-encoding>"; ID(divwu);
        	break;
              case 0x05:
        	rl78->syntax = "machu"; ID(machu);
        	break;
              case 0x06:
        	rl78->syntax = "mach"; ID(mach);
        	break;
              case 0x0b:
        	rl78->syntax = "divwu"; ID(divwu);
        	break;
              }

        }
      break;
    case 0xcf:
        {
          /** 1100 1111			mov	%e!0, #%1			*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1100 1111			mov	%e!0, #%1			*/",
                     op[0]);
            }
          SYNTAX("mov	%e!0, #%1");
#line 615 "rl78-decode.opc"
          ID(mov); DM(None, IMMU(2)); SC(IMMU(1));

        }
      break;
    case 0xd0:
    case 0xd1:
    case 0xd2:
    case 0xd3:
        {
          /** 1101 00rg			cmp0	%0				*/
#line 524 "rl78-decode.opc"
          int rg AU = op[0] & 0x03;
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1101 00rg			cmp0	%0				*/",
                     op[0]);
              printf ("  rg = 0x%x\n", rg);
            }
          SYNTAX("cmp0	%0");
#line 524 "rl78-decode.opc"
          ID(cmp); DRB(rg); SC(0); Fzac;

        }
      break;
    case 0xd4:
        {
          /** 1101 0100			cmp0	%0				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1101 0100			cmp0	%0				*/",
                     op[0]);
            }
          SYNTAX("cmp0	%0");
#line 527 "rl78-decode.opc"
          ID(cmp); DM(None, SADDR); SC(0); Fzac;

        /*----------------------------------------------------------------------*/

        }
      break;
    case 0xd5:
        {
          /** 1101 0101			cmp0	%e!0				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1101 0101			cmp0	%e!0				*/",
                     op[0]);
            }
          SYNTAX("cmp0	%e!0");
#line 521 "rl78-decode.opc"
          ID(cmp); DM(None, IMMU(2)); SC(0); Fzac;

        }
      break;
    case 0xd6:
        {
          /** 1101 0110			mulu	x				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1101 0110			mulu	x				*/",
                     op[0]);
            }
          SYNTAX("mulu	x");
#line 912 "rl78-decode.opc"
          ID(mulu);

        /*----------------------------------------------------------------------*/

        }
      break;
    case 0xd7:
        {
          /** 1101 0111			ret					*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1101 0111			ret					*/",
                     op[0]);
            }
          SYNTAX("ret");
#line 1008 "rl78-decode.opc"
          ID(ret);

        }
      break;
    case 0xd8:
        {
          /** 1101 1000			mov	%0, %1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1101 1000			mov	%0, %1				*/",
                     op[0]);
            }
          SYNTAX("mov	%0, %1");
#line 717 "rl78-decode.opc"
          ID(mov); DR(X); SM(None, SADDR);

        }
      break;
    case 0xd9:
        {
          /** 1101 1001			mov	%0, %e!1			*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1101 1001			mov	%0, %e!1			*/",
                     op[0]);
            }
          SYNTAX("mov	%0, %e!1");
#line 714 "rl78-decode.opc"
          ID(mov); DR(X); SM(None, IMMU(2));

        }
      break;
    case 0xda:
    case 0xea:
    case 0xfa:
        {
          /** 11ra 1010			movw	%0, %1				*/
#line 895 "rl78-decode.opc"
          int ra AU = (op[0] >> 4) & 0x03;
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 11ra 1010			movw	%0, %1				*/",
                     op[0]);
              printf ("  ra = 0x%x\n", ra);
            }
          SYNTAX("movw	%0, %1");
#line 895 "rl78-decode.opc"
          ID(mov); W(); DRW(ra); SM(None, SADDR);

        }
      break;
    case 0xdb:
    case 0xeb:
    case 0xfb:
        {
          /** 11ra 1011			movw	%0, %es!1			*/
#line 892 "rl78-decode.opc"
          int ra AU = (op[0] >> 4) & 0x03;
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 11ra 1011			movw	%0, %es!1			*/",
                     op[0]);
              printf ("  ra = 0x%x\n", ra);
            }
          SYNTAX("movw	%0, %es!1");
#line 892 "rl78-decode.opc"
          ID(mov); W(); DRW(ra); SM(None, IMMU(2));

        }
      break;
    case 0xdc:
        {
          /** 1101 1100			bc	$%a0				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1101 1100			bc	$%a0				*/",
                     op[0]);
            }
          SYNTAX("bc	$%a0");
#line 340 "rl78-decode.opc"
          ID(branch_cond); DC(pc+IMMS(1)+2); SR(None); COND(C);

        }
      break;
    case 0xdd:
        {
          /** 1101 1101			bz	$%a0				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1101 1101			bz	$%a0				*/",
                     op[0]);
            }
          SYNTAX("bz	$%a0");
#line 352 "rl78-decode.opc"
          ID(branch_cond); DC(pc+IMMS(1)+2); SR(None); COND(Z);

        }
      break;
    case 0xde:
        {
          /** 1101 1110			bnc	$%a0				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1101 1110			bnc	$%a0				*/",
                     op[0]);
            }
          SYNTAX("bnc	$%a0");
#line 343 "rl78-decode.opc"
          ID(branch_cond); DC(pc+IMMS(1)+2); SR(None); COND(NC);

        }
      break;
    case 0xdf:
        {
          /** 1101 1111			bnz	$%a0				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1101 1111			bnz	$%a0				*/",
                     op[0]);
            }
          SYNTAX("bnz	$%a0");
#line 355 "rl78-decode.opc"
          ID(branch_cond); DC(pc+IMMS(1)+2); SR(None); COND(NZ);

        /*----------------------------------------------------------------------*/

        }
      break;
    case 0xe0:
    case 0xe1:
    case 0xe2:
    case 0xe3:
        {
          /** 1110 00rg			oneb	%0				*/
#line 930 "rl78-decode.opc"
          int rg AU = op[0] & 0x03;
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1110 00rg			oneb	%0				*/",
                     op[0]);
              printf ("  rg = 0x%x\n", rg);
            }
          SYNTAX("oneb	%0");
#line 930 "rl78-decode.opc"
          ID(mov); DRB(rg); SC(1);

        }
      break;
    case 0xe4:
        {
          /** 1110 0100			oneb	%0				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1110 0100			oneb	%0				*/",
                     op[0]);
            }
          SYNTAX("oneb	%0");
#line 933 "rl78-decode.opc"
          ID(mov); DM(None, SADDR); SC(1);

        /*----------------------------------------------------------------------*/

        }
      break;
    case 0xe5:
        {
          /** 1110 0101			oneb	%e!0				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1110 0101			oneb	%e!0				*/",
                     op[0]);
            }
          SYNTAX("oneb	%e!0");
#line 927 "rl78-decode.opc"
          ID(mov); DM(None, IMMU(2)); SC(1);

        }
      break;
    case 0xe6:
        {
          /** 1110 0110			onew	%0				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1110 0110			onew	%0				*/",
                     op[0]);
            }
          SYNTAX("onew	%0");
#line 938 "rl78-decode.opc"
          ID(mov); DR(AX); SC(1);

        }
      break;
    case 0xe7:
        {
          /** 1110 0111			onew	%0				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1110 0111			onew	%0				*/",
                     op[0]);
            }
          SYNTAX("onew	%0");
#line 941 "rl78-decode.opc"
          ID(mov); DR(BC); SC(1);

        /*----------------------------------------------------------------------*/

        }
      break;
    case 0xe8:
        {
          /** 1110 1000			mov	%0, %1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1110 1000			mov	%0, %1				*/",
                     op[0]);
            }
          SYNTAX("mov	%0, %1");
#line 705 "rl78-decode.opc"
          ID(mov); DR(B); SM(None, SADDR);

        }
      break;
    case 0xe9:
        {
          /** 1110 1001			mov	%0, %e!1			*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1110 1001			mov	%0, %e!1			*/",
                     op[0]);
            }
          SYNTAX("mov	%0, %e!1");
#line 699 "rl78-decode.opc"
          ID(mov); DR(B); SM(None, IMMU(2));

        }
      break;
    case 0xec:
        {
          /** 1110 1100			br	!%!a0				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1110 1100			br	!%!a0				*/",
                     op[0]);
            }
          SYNTAX("br	!%!a0");
#line 374 "rl78-decode.opc"
          ID(branch); DC(IMMU(3));

        }
      break;
    case 0xed:
        {
          /** 1110 1101			br	%!a0				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1110 1101			br	%!a0				*/",
                     op[0]);
            }
          SYNTAX("br	%!a0");
#line 377 "rl78-decode.opc"
          ID(branch); DC(IMMU(2));

        }
      break;
    case 0xee:
        {
          /** 1110 1110			br	$%!a0				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1110 1110			br	$%!a0				*/",
                     op[0]);
            }
          SYNTAX("br	$%!a0");
#line 380 "rl78-decode.opc"
          ID(branch); DC(pc+IMMS(2)+3);

        }
      break;
    case 0xef:
        {
          /** 1110 1111			br	$%a0				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1110 1111			br	$%a0				*/",
                     op[0]);
            }
          SYNTAX("br	$%a0");
#line 383 "rl78-decode.opc"
          ID(branch); DC(pc+IMMS(1)+2);

        }
      break;
    case 0xf0:
    case 0xf1:
    case 0xf2:
    case 0xf3:
        {
          /** 1111 00rg			clrb	%0				*/
#line 470 "rl78-decode.opc"
          int rg AU = op[0] & 0x03;
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1111 00rg			clrb	%0				*/",
                     op[0]);
              printf ("  rg = 0x%x\n", rg);
            }
          SYNTAX("clrb	%0");
#line 470 "rl78-decode.opc"
          ID(mov); DRB(rg); SC(0);

        }
      break;
    case 0xf4:
        {
          /** 1111 0100			clrb	%0				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1111 0100			clrb	%0				*/",
                     op[0]);
            }
          SYNTAX("clrb	%0");
#line 473 "rl78-decode.opc"
          ID(mov); DM(None, SADDR); SC(0);

        /*----------------------------------------------------------------------*/

        }
      break;
    case 0xf5:
        {
          /** 1111 0101			clrb	%e!0				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1111 0101			clrb	%e!0				*/",
                     op[0]);
            }
          SYNTAX("clrb	%e!0");
#line 467 "rl78-decode.opc"
          ID(mov); DM(None, IMMU(2)); SC(0);

        }
      break;
    case 0xf6:
        {
          /** 1111 0110			clrw	%0				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1111 0110			clrw	%0				*/",
                     op[0]);
            }
          SYNTAX("clrw	%0");
#line 478 "rl78-decode.opc"
          ID(mov); DR(AX); SC(0);

        }
      break;
    case 0xf7:
        {
          /** 1111 0111			clrw	%0				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1111 0111			clrw	%0				*/",
                     op[0]);
            }
          SYNTAX("clrw	%0");
#line 481 "rl78-decode.opc"
          ID(mov); DR(BC); SC(0);

        /*----------------------------------------------------------------------*/

        }
      break;
    case 0xf8:
        {
          /** 1111 1000			mov	%0, %1				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1111 1000			mov	%0, %1				*/",
                     op[0]);
            }
          SYNTAX("mov	%0, %1");
#line 711 "rl78-decode.opc"
          ID(mov); DR(C); SM(None, SADDR);

        }
      break;
    case 0xf9:
        {
          /** 1111 1001			mov	%0, %e!1			*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1111 1001			mov	%0, %e!1			*/",
                     op[0]);
            }
          SYNTAX("mov	%0, %e!1");
#line 708 "rl78-decode.opc"
          ID(mov); DR(C); SM(None, IMMU(2));

        }
      break;
    case 0xfc:
        {
          /** 1111 1100			call	!%!a0				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1111 1100			call	!%!a0				*/",
                     op[0]);
            }
          SYNTAX("call	!%!a0");
#line 427 "rl78-decode.opc"
          ID(call); DC(IMMU(3));

        }
      break;
    case 0xfd:
        {
          /** 1111 1101			call	%!a0				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1111 1101			call	%!a0				*/",
                     op[0]);
            }
          SYNTAX("call	%!a0");
#line 430 "rl78-decode.opc"
          ID(call); DC(IMMU(2));

        }
      break;
    case 0xfe:
        {
          /** 1111 1110			call	$%!a0				*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1111 1110			call	$%!a0				*/",
                     op[0]);
            }
          SYNTAX("call	$%!a0");
#line 433 "rl78-decode.opc"
          ID(call); DC(pc+IMMS(2)+3);

        }
      break;
    case 0xff:
        {
          /** 1111 1111			brk1					*/
          if (trace)
            {
              printf ("\033[33m%s\033[0m  %02x\n",
                     "/** 1111 1111			brk1					*/",
                     op[0]);
            }
          SYNTAX("brk1");
#line 391 "rl78-decode.opc"
          ID(break);

        }
      break;
  }
#line 1296 "rl78-decode.opc"

  return rl78->n_bytes;
}
