/* DO NOT EDIT!  -*- buffer-read-only: t -*- vi:set ro:  */
#line 1 "msp430-decode.opc"
/* -*- c -*- */
/* Copyright (C) 2013-2023 Free Software Foundation, Inc.
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
#include "opcode/msp430-decode.h"

static int trace = 0;

typedef struct
{
  MSP430_Opcode_Decoded *msp430;
  int (*getbyte)(void *);
  void *ptr;
  unsigned char *op;
  int op_ptr;
  int pc;
} LocalData;

#define AU ATTRIBUTE_UNUSED
#define GETBYTE() getbyte_swapped (ld)
#define B ((unsigned long) GETBYTE ())

static int
getbyte_swapped (LocalData *ld)
{
  int b;

  if (ld->op_ptr == ld->msp430->n_bytes)
    {
      do
	{
	  b = ld->getbyte (ld->ptr);
	  ld->op [(ld->msp430->n_bytes++)^1] = b;
	}
      while (ld->msp430->n_bytes & 1);
    }
  return ld->op[ld->op_ptr++];
}

#define ID(x)		msp430->id = x

#define OP(n, t, r, a) (msp430->op[n].type = t,	     \
		        msp430->op[n].reg = r,	     \
		        msp430->op[n].addend = a)

#define OPX(n, t, r1, r2, a)	 \
  (msp430->op[n].type = t,	 \
   msp430->op[n].reg = r1,	 \
   msp430->op[n].reg2 = r2,	 \
   msp430->op[n].addend = a)

#define SYNTAX(x)	msp430->syntax = x
#define UNSUPPORTED()	msp430->syntax = "*unknown*"

#define DC(c)		OP (0, MSP430_Operand_Immediate, 0, c)
#define DR(r)		OP (0, MSP430_Operand_Register, r, 0)
#define DM(r, a)	OP (0, MSP430_Operand_Indirect, r, a)
#define DA(a)		OP (0, MSP430_Operand_Indirect, MSR_None, a)
#define AD(r, ad)	encode_ad (r, ad, ld, 0)
#define ADX(r, ad, x)	encode_ad (r, ad, ld, x)

#define SC(c)		OP (1, MSP430_Operand_Immediate, 0, c)
#define SR(r)		OP (1, MSP430_Operand_Register, r, 0)
#define SM(r, a)	OP (1, MSP430_Operand_Indirect, r, a)
#define SA(a)		OP (1, MSP430_Operand_Indirect, MSR_None, a)
#define SI(r)		OP (1, MSP430_Operand_Indirect_Postinc, r, 0)
#define AS(r, as)	encode_as (r, as, ld, 0)
#define ASX(r, as, x)	encode_as (r, as, ld, x)

#define BW(x)		msp430->size = (x ? 8 : 16)
/* The last 20 is for SWPBX.Z and SXTX.A.  */
#define ABW(a,x)	msp430->size = (a ? ((x ? 8 : 16)) : (x ? 20 : 20))

#define IMMU(bytes)	immediate (bytes, 0, ld)
#define IMMS(bytes)	immediate (bytes, 1, ld)

/* Helper macros for known status bits settings.  */
#define	F_____		msp430->flags_1 = msp430->flags_0 = 0; msp430->flags_set = 0
#define	F_VNZC		msp430->flags_1 = msp430->flags_0 = 0; msp430->flags_set = 0x87
#define	F_0NZC		msp430->flags_1 = 0; msp430->flags_0 = 0x80; msp430->flags_set = 0x07


/* The chip is little-endian, but GETBYTE byte-swaps words because the
   decoder is based on 16-bit "words" so *this* logic is big-endian.  */

static int
immediate (int bytes, int sign_extend, LocalData *ld)
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
      i |= B << 8;
      i |= B;
      if (sign_extend && (i & 0x8000))
	i -= 0x10000;
      break;
    case 3:
      i |= B << 16;
      i |= B << 8;
      i |= B;
      if (sign_extend && (i & 0x800000))
	i -= 0x1000000;
      break;
    case 4:
      i |= B << 24;
      i |= B << 16;
      i |= B << 8;
      i |= B;
      if (sign_extend && (i & 0x80000000ULL))
	i -= 0x100000000ULL;
      break;
    default:
      opcodes_error_handler
	(_("internal error: immediate() called with invalid byte count %d"),
	   bytes);
      abort ();
    }
  return i;
}

/*
		PC	SP	SR	CG
  As
  00	Rn	-	-	R2	#0
  01	X(Rn)	Sym	-	X(abs)	#1
  10	(Rn)	-	-	#4	#2
  11	(Rn++)	#imm	-	#8	#-1

  Ad
  0	Rn	-	-	-	-
  1	X(Rn)	Sym	-	X(abs)	-   */

static void
encode_ad (int reg, int ad, LocalData *ld, int ext)
{
  MSP430_Opcode_Decoded *msp430 = ld->msp430;

  if (ad)
    {
      int x = IMMU(2) | (ext << 16);
      switch (reg)
	{
	case 0: /* (PC) -> Symbolic.  */
	  DA (x + ld->pc + ld->op_ptr - 2);
	  break;
	case 2: /* (SR) -> Absolute.  */
	  DA (x);
	  break;
	default:
	  DM (reg, x);
	  break;
	}
    }
  else
    {
      DR (reg);
    }
}

static void
encode_as (int reg, int as, LocalData *ld, int ext)
{
  MSP430_Opcode_Decoded *msp430 = ld->msp430;
  int x;

  switch (as)
    {
    case 0:
      switch (reg)
	{
	case 3:
	  SC (0);
	  break;
	default:
	  SR (reg);
	  break;
	}
      break;
    case 1:
      switch (reg)
	{
	case 0: /* PC -> Symbolic.  */
	  x = IMMU(2) | (ext << 16);
	  SA (x + ld->pc + ld->op_ptr - 2);
	  break;
	case 2: /* SR -> Absolute.  */
	  x = IMMU(2) | (ext << 16);
	  SA (x);
	  break;
	case 3:
	  SC (1);
	  break;
	default:
	  x = IMMU(2) | (ext << 16);
	  SM (reg, x);
	  break;
	}
      break;
    case 2:
      switch (reg)
	{
	case 2:
	  SC (4);
	  break;
	case 3:
	  SC (2);
	  break;
	case MSR_None:
	  SA (0);
	  break;
	default:
	  SM (reg, 0);
	  break;
	}
      break;
    case 3:
      switch (reg)
	{
	case 0:
	  {
	    /* This fetch *is* the *PC++ that the opcode encodes :-)  */
	    x = IMMU(2) | (ext << 16);
	    SC (x);
	  }
	  break;
	case 2:
	  SC (8);
	  break;
	case 3:
	  SC (-1);
	  break;
	default:
	  SI (reg);
	  break;
	}
      break;
    }
}

static void
encode_rep_zc (int srxt, int dsxt, LocalData *ld)
{
  MSP430_Opcode_Decoded *msp430 = ld->msp430;

  msp430->repeat_reg = srxt & 1;
  msp430->repeats = dsxt;
  msp430->zc = (srxt & 2) ? 1 : 0;
}

#define REPZC(s,d) encode_rep_zc (s, d, ld)

static int
dopc_to_id (int dopc)
{
  switch (dopc)
    {
    case 4: return MSO_mov;
    case 5: return MSO_add;
    case 6: return MSO_addc;
    case 7: return MSO_subc;
    case 8: return MSO_sub;
    case 9: return MSO_cmp;
    case 10: return MSO_dadd;
    case 11: return MSO_bit;
    case 12: return MSO_bic;
    case 13: return MSO_bis;
    case 14: return MSO_xor;
    case 15: return MSO_and;
    default: return MSO_unknown;
    }
}

static int
sopc_to_id (int sop, int c)
{
  switch (sop * 2 + c)
    {
    case 0: return MSO_rrc;
    case 1: return MSO_swpb;
    case 2: return MSO_rra;
    case 3: return MSO_sxt;
    case 4: return MSO_push;
    case 5: return MSO_call;
    case 6: return MSO_reti;
    default: return MSO_unknown;
    }
}

int
msp430_decode_opcode (unsigned long pc,
		      MSP430_Opcode_Decoded *msp430,
		      int (*getbyte)(void *),
		      void *ptr)
{
  LocalData lds, *ld = &lds;
  unsigned char op_buf[20] = {0};
  unsigned char *op = op_buf;
  int raddr;
  int al_bit;
  int srxt_bits, dsxt_bits;

  lds.msp430 = msp430;
  lds.getbyte = getbyte;
  lds.ptr = ptr;
  lds.op = op;
  lds.op_ptr = 0;
  lds.pc = pc;

  memset (msp430, 0, sizeof (*msp430));

  /* These are overridden by an extension word.  */
  al_bit = 1;
  srxt_bits = 0;
  dsxt_bits = 0;

 post_extension_word:
  ;

  /* 430X extention word.  */
  GETBYTE ();
  switch (op[0] & 0xff)
  {
    case 0x00:
        GETBYTE ();
        switch (op[1] & 0xf0)
        {
          case 0x00:
            op_semantics_1:
              {
                /** 0000 srcr 0000 dstr		MOVA @%1, %0 */
#line 440 "msp430-decode.opc"
                int srcr AU = op[0] & 0x0f;
#line 440 "msp430-decode.opc"
                int dstr AU = op[1] & 0x0f;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0000 srcr 0000 dstr		MOVA @%1, %0 */",
                           op[0], op[1]);
                    printf ("  srcr = 0x%x,", srcr);
                    printf ("  dstr = 0x%x\n", dstr);
                  }
                SYNTAX("MOVA @%1, %0");
#line 440 "msp430-decode.opc"
                ID (MSO_mov); SM (srcr, 0); DR (dstr);
                msp430->size = 20;
                msp430->ofs_430x = 1;

              }
            break;
          case 0x10:
            op_semantics_2:
              {
                /** 0000 srcr 0001 dstr		MOVA @%1+, %0 */
#line 445 "msp430-decode.opc"
                int srcr AU = op[0] & 0x0f;
#line 445 "msp430-decode.opc"
                int dstr AU = op[1] & 0x0f;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0000 srcr 0001 dstr		MOVA @%1+, %0 */",
                           op[0], op[1]);
                    printf ("  srcr = 0x%x,", srcr);
                    printf ("  dstr = 0x%x\n", dstr);
                  }
                SYNTAX("MOVA @%1+, %0");
#line 445 "msp430-decode.opc"
                ID (MSO_mov); SI (srcr); DR (dstr);
                msp430->size = 20;
                msp430->ofs_430x = 1;

              }
            break;
          case 0x20:
            op_semantics_3:
              {
                /** 0000 srcr 0010 dstr		MOVA &%1, %0 */
#line 450 "msp430-decode.opc"
                int srcr AU = op[0] & 0x0f;
#line 450 "msp430-decode.opc"
                int dstr AU = op[1] & 0x0f;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0000 srcr 0010 dstr		MOVA &%1, %0 */",
                           op[0], op[1]);
                    printf ("  srcr = 0x%x,", srcr);
                    printf ("  dstr = 0x%x\n", dstr);
                  }
                SYNTAX("MOVA &%1, %0");
#line 450 "msp430-decode.opc"
                ID (MSO_mov); SA ((srcr << 16) + IMMU(2)); DR (dstr);
                msp430->size = 20;
                msp430->ofs_430x = 1;

              }
            break;
          case 0x30:
            op_semantics_4:
              {
                /** 0000 srcr 0011 dstr		MOVA %1, %0 */
#line 455 "msp430-decode.opc"
                int srcr AU = op[0] & 0x0f;
#line 455 "msp430-decode.opc"
                int dstr AU = op[1] & 0x0f;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0000 srcr 0011 dstr		MOVA %1, %0 */",
                           op[0], op[1]);
                    printf ("  srcr = 0x%x,", srcr);
                    printf ("  dstr = 0x%x\n", dstr);
                  }
                SYNTAX("MOVA %1, %0");
#line 455 "msp430-decode.opc"
                ID (MSO_mov); SM (srcr, IMMS(2)); DR (dstr);
                msp430->size = 20;
                msp430->ofs_430x = 1;

              }
            break;
          case 0x40:
          case 0x50:
            op_semantics_5:
              {
                /** 0000 bt00 010w dstr		RRCM.A %c, %0 */
#line 522 "msp430-decode.opc"
                int bt AU = (op[0] >> 2) & 0x03;
#line 522 "msp430-decode.opc"
                int w AU = (op[1] >> 4) & 0x01;
#line 522 "msp430-decode.opc"
                int dstr AU = op[1] & 0x0f;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0000 bt00 010w dstr		RRCM.A %c, %0 */",
                           op[0], op[1]);
                    printf ("  bt = 0x%x,", bt);
                    printf ("  w = 0x%x,", w);
                    printf ("  dstr = 0x%x\n", dstr);
                  }
                SYNTAX("RRCM.A %c, %0");
#line 522 "msp430-decode.opc"
                ID (MSO_rrc); DR (dstr); SR (dstr);
                msp430->repeats = bt;
                msp430->size = w ? 16 : 20;
                msp430->ofs_430x = 1;
                F_0NZC;

              }
            break;
          case 0x60:
            op_semantics_6:
              {
                /** 0000 srcr 0110 dstr		MOVA %1, &%0 */
#line 460 "msp430-decode.opc"
                int srcr AU = op[0] & 0x0f;
#line 460 "msp430-decode.opc"
                int dstr AU = op[1] & 0x0f;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0000 srcr 0110 dstr		MOVA %1, &%0 */",
                           op[0], op[1]);
                    printf ("  srcr = 0x%x,", srcr);
                    printf ("  dstr = 0x%x\n", dstr);
                  }
                SYNTAX("MOVA %1, &%0");
#line 460 "msp430-decode.opc"
                ID (MSO_mov); SR (srcr); DA ((dstr << 16) + IMMU(2));
                msp430->size = 20;
                msp430->ofs_430x = 1;

              }
            break;
          case 0x70:
            op_semantics_7:
              {
                /** 0000 srcr 0111 dstr		MOVA %1, &%0 */
#line 465 "msp430-decode.opc"
                int srcr AU = op[0] & 0x0f;
#line 465 "msp430-decode.opc"
                int dstr AU = op[1] & 0x0f;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0000 srcr 0111 dstr		MOVA %1, &%0 */",
                           op[0], op[1]);
                    printf ("  srcr = 0x%x,", srcr);
                    printf ("  dstr = 0x%x\n", dstr);
                  }
                SYNTAX("MOVA %1, &%0");
#line 465 "msp430-decode.opc"
                ID (MSO_mov); SR (srcr); DM (dstr, IMMS(2));
                msp430->size = 20;
                msp430->ofs_430x = 1;

              }
            break;
          case 0x80:
            op_semantics_8:
              {
                /** 0000 srcr 1000 dstr		MOVA %1, %0 */
#line 470 "msp430-decode.opc"
                int srcr AU = op[0] & 0x0f;
#line 470 "msp430-decode.opc"
                int dstr AU = op[1] & 0x0f;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0000 srcr 1000 dstr		MOVA %1, %0 */",
                           op[0], op[1]);
                    printf ("  srcr = 0x%x,", srcr);
                    printf ("  dstr = 0x%x\n", dstr);
                  }
                SYNTAX("MOVA %1, %0");
#line 470 "msp430-decode.opc"
                ID (MSO_mov); SC ((srcr << 16) + IMMU(2)); DR (dstr);
                msp430->size = 20;
                msp430->ofs_430x = 1;

              }
            break;
          case 0x90:
            op_semantics_9:
              {
                /** 0000 srcr 1001 dstr		CMPA %1, %0 */
#line 475 "msp430-decode.opc"
                int srcr AU = op[0] & 0x0f;
#line 475 "msp430-decode.opc"
                int dstr AU = op[1] & 0x0f;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0000 srcr 1001 dstr		CMPA %1, %0 */",
                           op[0], op[1]);
                    printf ("  srcr = 0x%x,", srcr);
                    printf ("  dstr = 0x%x\n", dstr);
                  }
                SYNTAX("CMPA %1, %0");
#line 475 "msp430-decode.opc"
                ID (MSO_cmp); SC ((srcr << 16) + IMMU(2)); DR (dstr);
                msp430->size = 20;
                msp430->ofs_430x = 1;
                F_VNZC;

              }
            break;
          case 0xa0:
            op_semantics_10:
              {
                /** 0000 srcr 1010 dstr		ADDA %1, %0 */
#line 481 "msp430-decode.opc"
                int srcr AU = op[0] & 0x0f;
#line 481 "msp430-decode.opc"
                int dstr AU = op[1] & 0x0f;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0000 srcr 1010 dstr		ADDA %1, %0 */",
                           op[0], op[1]);
                    printf ("  srcr = 0x%x,", srcr);
                    printf ("  dstr = 0x%x\n", dstr);
                  }
                SYNTAX("ADDA %1, %0");
#line 481 "msp430-decode.opc"
                ID (MSO_add); SC ((srcr << 16) + IMMU(2)); DR (dstr);
                msp430->size = 20;
                msp430->ofs_430x = 1;
                F_VNZC;

              }
            break;
          case 0xb0:
            op_semantics_11:
              {
                /** 0000 srcr 1011 dstr		SUBA %1, %0 */
#line 487 "msp430-decode.opc"
                int srcr AU = op[0] & 0x0f;
#line 487 "msp430-decode.opc"
                int dstr AU = op[1] & 0x0f;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0000 srcr 1011 dstr		SUBA %1, %0 */",
                           op[0], op[1]);
                    printf ("  srcr = 0x%x,", srcr);
                    printf ("  dstr = 0x%x\n", dstr);
                  }
                SYNTAX("SUBA %1, %0");
#line 487 "msp430-decode.opc"
                ID (MSO_sub); SC ((srcr << 16) + IMMU(2)); DR (dstr);
                msp430->size = 20;
                msp430->ofs_430x = 1;
                F_VNZC;

              }
            break;
          case 0xc0:
            op_semantics_12:
              {
                /** 0000 srcr 1100 dstr		MOVA %1, %0 */
#line 499 "msp430-decode.opc"
                int srcr AU = op[0] & 0x0f;
#line 499 "msp430-decode.opc"
                int dstr AU = op[1] & 0x0f;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0000 srcr 1100 dstr		MOVA %1, %0 */",
                           op[0], op[1]);
                    printf ("  srcr = 0x%x,", srcr);
                    printf ("  dstr = 0x%x\n", dstr);
                  }
                SYNTAX("MOVA %1, %0");
#line 499 "msp430-decode.opc"
                ID (MSO_mov); SR (srcr); DR (dstr);
                msp430->size = 20;
                msp430->ofs_430x = 1;

              }
            break;
          case 0xd0:
            op_semantics_13:
              {
                /** 0000 srcr 1101 dstr		CMPA %1, %0 */
#line 504 "msp430-decode.opc"
                int srcr AU = op[0] & 0x0f;
#line 504 "msp430-decode.opc"
                int dstr AU = op[1] & 0x0f;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0000 srcr 1101 dstr		CMPA %1, %0 */",
                           op[0], op[1]);
                    printf ("  srcr = 0x%x,", srcr);
                    printf ("  dstr = 0x%x\n", dstr);
                  }
                SYNTAX("CMPA %1, %0");
#line 504 "msp430-decode.opc"
                ID (MSO_cmp); SR (srcr); DR (dstr);
                msp430->size = 20;
                msp430->ofs_430x = 1;
                F_VNZC;

              }
            break;
          case 0xe0:
            op_semantics_14:
              {
                /** 0000 srcr 1110 dstr		ADDA %1, %0 */
#line 510 "msp430-decode.opc"
                int srcr AU = op[0] & 0x0f;
#line 510 "msp430-decode.opc"
                int dstr AU = op[1] & 0x0f;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0000 srcr 1110 dstr		ADDA %1, %0 */",
                           op[0], op[1]);
                    printf ("  srcr = 0x%x,", srcr);
                    printf ("  dstr = 0x%x\n", dstr);
                  }
                SYNTAX("ADDA %1, %0");
#line 510 "msp430-decode.opc"
                ID (MSO_add); SR (srcr); DR (dstr);
                msp430->size = 20;
                msp430->ofs_430x = 1;
                F_VNZC;

              }
            break;
          case 0xf0:
            op_semantics_15:
              {
                /** 0000 srcr 1111 dstr		SUBA %1, %0 */
#line 516 "msp430-decode.opc"
                int srcr AU = op[0] & 0x0f;
#line 516 "msp430-decode.opc"
                int dstr AU = op[1] & 0x0f;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0000 srcr 1111 dstr		SUBA %1, %0 */",
                           op[0], op[1]);
                    printf ("  srcr = 0x%x,", srcr);
                    printf ("  dstr = 0x%x\n", dstr);
                  }
                SYNTAX("SUBA %1, %0");
#line 516 "msp430-decode.opc"
                ID (MSO_sub); SR (srcr); DR (dstr);
                msp430->size = 20;
                msp430->ofs_430x = 1;
                F_VNZC;

              }
            break;
        }
      break;
    case 0x01:
        GETBYTE ();
        switch (op[1] & 0xf0)
        {
          case 0x00:
            goto op_semantics_1;
            break;
          case 0x10:
            goto op_semantics_2;
            break;
          case 0x20:
            goto op_semantics_3;
            break;
          case 0x30:
            goto op_semantics_4;
            break;
          case 0x40:
          case 0x50:
            op_semantics_16:
              {
                /** 0000 bt01 010w dstr		RRAM.A %c, %0 */
#line 529 "msp430-decode.opc"
                int bt AU = (op[0] >> 2) & 0x03;
#line 529 "msp430-decode.opc"
                int w AU = (op[1] >> 4) & 0x01;
#line 529 "msp430-decode.opc"
                int dstr AU = op[1] & 0x0f;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0000 bt01 010w dstr		RRAM.A %c, %0 */",
                           op[0], op[1]);
                    printf ("  bt = 0x%x,", bt);
                    printf ("  w = 0x%x,", w);
                    printf ("  dstr = 0x%x\n", dstr);
                  }
                SYNTAX("RRAM.A %c, %0");
#line 529 "msp430-decode.opc"
                ID (MSO_rra); DR (dstr); SR (dstr);
                msp430->repeats = bt;
                msp430->size = w ? 16 : 20;
                msp430->ofs_430x = 1;
                F_0NZC;

              }
            break;
          case 0x60:
            goto op_semantics_6;
            break;
          case 0x70:
            goto op_semantics_7;
            break;
          case 0x80:
            goto op_semantics_8;
            break;
          case 0x90:
            goto op_semantics_9;
            break;
          case 0xa0:
            goto op_semantics_10;
            break;
          case 0xb0:
            goto op_semantics_11;
            break;
          case 0xc0:
            goto op_semantics_12;
            break;
          case 0xd0:
            goto op_semantics_13;
            break;
          case 0xe0:
            goto op_semantics_14;
            break;
          case 0xf0:
            goto op_semantics_15;
            break;
        }
      break;
    case 0x02:
        GETBYTE ();
        switch (op[1] & 0xf0)
        {
          case 0x00:
            goto op_semantics_1;
            break;
          case 0x10:
            goto op_semantics_2;
            break;
          case 0x20:
            goto op_semantics_3;
            break;
          case 0x30:
            goto op_semantics_4;
            break;
          case 0x40:
          case 0x50:
            op_semantics_17:
              {
                /** 0000 bt10 010w dstr		RLAM.A %c, %0 */
#line 536 "msp430-decode.opc"
                int bt AU = (op[0] >> 2) & 0x03;
#line 536 "msp430-decode.opc"
                int w AU = (op[1] >> 4) & 0x01;
#line 536 "msp430-decode.opc"
                int dstr AU = op[1] & 0x0f;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0000 bt10 010w dstr		RLAM.A %c, %0 */",
                           op[0], op[1]);
                    printf ("  bt = 0x%x,", bt);
                    printf ("  w = 0x%x,", w);
                    printf ("  dstr = 0x%x\n", dstr);
                  }
                SYNTAX("RLAM.A %c, %0");
#line 536 "msp430-decode.opc"
                ID (MSO_add); DR (dstr); SR (dstr);
                msp430->repeats = bt;
                msp430->size = w ? 16 : 20;
                msp430->ofs_430x = 1;
                F_0NZC;

              }
            break;
          case 0x60:
            goto op_semantics_6;
            break;
          case 0x70:
            goto op_semantics_7;
            break;
          case 0x80:
            goto op_semantics_8;
            break;
          case 0x90:
            goto op_semantics_9;
            break;
          case 0xa0:
            goto op_semantics_10;
            break;
          case 0xb0:
            goto op_semantics_11;
            break;
          case 0xc0:
            goto op_semantics_12;
            break;
          case 0xd0:
            goto op_semantics_13;
            break;
          case 0xe0:
            goto op_semantics_14;
            break;
          case 0xf0:
            goto op_semantics_15;
            break;
        }
      break;
    case 0x03:
        GETBYTE ();
        switch (op[1] & 0xf0)
        {
          case 0x00:
            goto op_semantics_1;
            break;
          case 0x10:
            goto op_semantics_2;
            break;
          case 0x20:
            goto op_semantics_3;
            break;
          case 0x30:
            goto op_semantics_4;
            break;
          case 0x40:
          case 0x50:
            op_semantics_18:
              {
                /** 0000 bt11 010w dstr		RRUM.A %c, %0 */
#line 543 "msp430-decode.opc"
                int bt AU = (op[0] >> 2) & 0x03;
#line 543 "msp430-decode.opc"
                int w AU = (op[1] >> 4) & 0x01;
#line 543 "msp430-decode.opc"
                int dstr AU = op[1] & 0x0f;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0000 bt11 010w dstr		RRUM.A %c, %0 */",
                           op[0], op[1]);
                    printf ("  bt = 0x%x,", bt);
                    printf ("  w = 0x%x,", w);
                    printf ("  dstr = 0x%x\n", dstr);
                  }
                SYNTAX("RRUM.A %c, %0");
#line 543 "msp430-decode.opc"
                ID (MSO_rru); DR (dstr); SR (dstr);
                msp430->repeats = bt;
                msp430->size = w ? 16 : 20;
                msp430->ofs_430x = 1;
                F_0NZC;

              }
            break;
          case 0x60:
            goto op_semantics_6;
            break;
          case 0x70:
            goto op_semantics_7;
            break;
          case 0x80:
            goto op_semantics_8;
            break;
          case 0x90:
            goto op_semantics_9;
            break;
          case 0xa0:
            goto op_semantics_10;
            break;
          case 0xb0:
            goto op_semantics_11;
            break;
          case 0xc0:
            goto op_semantics_12;
            break;
          case 0xd0:
            goto op_semantics_13;
            break;
          case 0xe0:
            goto op_semantics_14;
            break;
          case 0xf0:
            goto op_semantics_15;
            break;
        }
      break;
    case 0x04:
        GETBYTE ();
        switch (op[1] & 0xf0)
        {
          case 0x00:
            goto op_semantics_1;
            break;
          case 0x10:
            goto op_semantics_2;
            break;
          case 0x20:
            goto op_semantics_3;
            break;
          case 0x30:
            goto op_semantics_4;
            break;
          case 0x40:
          case 0x50:
            goto op_semantics_5;
            break;
          case 0x60:
            goto op_semantics_6;
            break;
          case 0x70:
            goto op_semantics_7;
            break;
          case 0x80:
            goto op_semantics_8;
            break;
          case 0x90:
            goto op_semantics_9;
            break;
          case 0xa0:
            goto op_semantics_10;
            break;
          case 0xb0:
            goto op_semantics_11;
            break;
          case 0xc0:
            goto op_semantics_12;
            break;
          case 0xd0:
            goto op_semantics_13;
            break;
          case 0xe0:
            goto op_semantics_14;
            break;
          case 0xf0:
            goto op_semantics_15;
            break;
        }
      break;
    case 0x05:
        GETBYTE ();
        switch (op[1] & 0xf0)
        {
          case 0x00:
            goto op_semantics_1;
            break;
          case 0x10:
            goto op_semantics_2;
            break;
          case 0x20:
            goto op_semantics_3;
            break;
          case 0x30:
            goto op_semantics_4;
            break;
          case 0x40:
          case 0x50:
            goto op_semantics_16;
            break;
          case 0x60:
            goto op_semantics_6;
            break;
          case 0x70:
            goto op_semantics_7;
            break;
          case 0x80:
            goto op_semantics_8;
            break;
          case 0x90:
            goto op_semantics_9;
            break;
          case 0xa0:
            goto op_semantics_10;
            break;
          case 0xb0:
            goto op_semantics_11;
            break;
          case 0xc0:
            goto op_semantics_12;
            break;
          case 0xd0:
            goto op_semantics_13;
            break;
          case 0xe0:
            goto op_semantics_14;
            break;
          case 0xf0:
            goto op_semantics_15;
            break;
        }
      break;
    case 0x06:
        GETBYTE ();
        switch (op[1] & 0xf0)
        {
          case 0x00:
            goto op_semantics_1;
            break;
          case 0x10:
            goto op_semantics_2;
            break;
          case 0x20:
            goto op_semantics_3;
            break;
          case 0x30:
            goto op_semantics_4;
            break;
          case 0x40:
          case 0x50:
            goto op_semantics_17;
            break;
          case 0x60:
            goto op_semantics_6;
            break;
          case 0x70:
            goto op_semantics_7;
            break;
          case 0x80:
            goto op_semantics_8;
            break;
          case 0x90:
            goto op_semantics_9;
            break;
          case 0xa0:
            goto op_semantics_10;
            break;
          case 0xb0:
            goto op_semantics_11;
            break;
          case 0xc0:
            goto op_semantics_12;
            break;
          case 0xd0:
            goto op_semantics_13;
            break;
          case 0xe0:
            goto op_semantics_14;
            break;
          case 0xf0:
            goto op_semantics_15;
            break;
        }
      break;
    case 0x07:
        GETBYTE ();
        switch (op[1] & 0xf0)
        {
          case 0x00:
            goto op_semantics_1;
            break;
          case 0x10:
            goto op_semantics_2;
            break;
          case 0x20:
            goto op_semantics_3;
            break;
          case 0x30:
            goto op_semantics_4;
            break;
          case 0x40:
          case 0x50:
            goto op_semantics_18;
            break;
          case 0x60:
            goto op_semantics_6;
            break;
          case 0x70:
            goto op_semantics_7;
            break;
          case 0x80:
            goto op_semantics_8;
            break;
          case 0x90:
            goto op_semantics_9;
            break;
          case 0xa0:
            goto op_semantics_10;
            break;
          case 0xb0:
            goto op_semantics_11;
            break;
          case 0xc0:
            goto op_semantics_12;
            break;
          case 0xd0:
            goto op_semantics_13;
            break;
          case 0xe0:
            goto op_semantics_14;
            break;
          case 0xf0:
            goto op_semantics_15;
            break;
        }
      break;
    case 0x08:
        GETBYTE ();
        switch (op[1] & 0xf0)
        {
          case 0x00:
            goto op_semantics_1;
            break;
          case 0x10:
            goto op_semantics_2;
            break;
          case 0x20:
            goto op_semantics_3;
            break;
          case 0x30:
            goto op_semantics_4;
            break;
          case 0x40:
          case 0x50:
            goto op_semantics_5;
            break;
          case 0x60:
            goto op_semantics_6;
            break;
          case 0x70:
            goto op_semantics_7;
            break;
          case 0x80:
            goto op_semantics_8;
            break;
          case 0x90:
            goto op_semantics_9;
            break;
          case 0xa0:
            goto op_semantics_10;
            break;
          case 0xb0:
            goto op_semantics_11;
            break;
          case 0xc0:
            goto op_semantics_12;
            break;
          case 0xd0:
            goto op_semantics_13;
            break;
          case 0xe0:
            goto op_semantics_14;
            break;
          case 0xf0:
            goto op_semantics_15;
            break;
        }
      break;
    case 0x09:
        GETBYTE ();
        switch (op[1] & 0xf0)
        {
          case 0x00:
            goto op_semantics_1;
            break;
          case 0x10:
            goto op_semantics_2;
            break;
          case 0x20:
            goto op_semantics_3;
            break;
          case 0x30:
            goto op_semantics_4;
            break;
          case 0x40:
          case 0x50:
            goto op_semantics_16;
            break;
          case 0x60:
            goto op_semantics_6;
            break;
          case 0x70:
            goto op_semantics_7;
            break;
          case 0x80:
            goto op_semantics_8;
            break;
          case 0x90:
            goto op_semantics_9;
            break;
          case 0xa0:
            goto op_semantics_10;
            break;
          case 0xb0:
            goto op_semantics_11;
            break;
          case 0xc0:
            goto op_semantics_12;
            break;
          case 0xd0:
            goto op_semantics_13;
            break;
          case 0xe0:
            goto op_semantics_14;
            break;
          case 0xf0:
            goto op_semantics_15;
            break;
        }
      break;
    case 0x0a:
        GETBYTE ();
        switch (op[1] & 0xf0)
        {
          case 0x00:
            goto op_semantics_1;
            break;
          case 0x10:
            goto op_semantics_2;
            break;
          case 0x20:
            goto op_semantics_3;
            break;
          case 0x30:
            goto op_semantics_4;
            break;
          case 0x40:
          case 0x50:
            goto op_semantics_17;
            break;
          case 0x60:
            goto op_semantics_6;
            break;
          case 0x70:
            goto op_semantics_7;
            break;
          case 0x80:
            goto op_semantics_8;
            break;
          case 0x90:
            goto op_semantics_9;
            break;
          case 0xa0:
            goto op_semantics_10;
            break;
          case 0xb0:
            goto op_semantics_11;
            break;
          case 0xc0:
            goto op_semantics_12;
            break;
          case 0xd0:
            goto op_semantics_13;
            break;
          case 0xe0:
            goto op_semantics_14;
            break;
          case 0xf0:
            goto op_semantics_15;
            break;
        }
      break;
    case 0x0b:
        GETBYTE ();
        switch (op[1] & 0xf0)
        {
          case 0x00:
            goto op_semantics_1;
            break;
          case 0x10:
            goto op_semantics_2;
            break;
          case 0x20:
            goto op_semantics_3;
            break;
          case 0x30:
            goto op_semantics_4;
            break;
          case 0x40:
          case 0x50:
            goto op_semantics_18;
            break;
          case 0x60:
            goto op_semantics_6;
            break;
          case 0x70:
            goto op_semantics_7;
            break;
          case 0x80:
            goto op_semantics_8;
            break;
          case 0x90:
            goto op_semantics_9;
            break;
          case 0xa0:
            goto op_semantics_10;
            break;
          case 0xb0:
            goto op_semantics_11;
            break;
          case 0xc0:
            goto op_semantics_12;
            break;
          case 0xd0:
            goto op_semantics_13;
            break;
          case 0xe0:
            goto op_semantics_14;
            break;
          case 0xf0:
            goto op_semantics_15;
            break;
        }
      break;
    case 0x0c:
        GETBYTE ();
        switch (op[1] & 0xf0)
        {
          case 0x00:
            goto op_semantics_1;
            break;
          case 0x10:
            goto op_semantics_2;
            break;
          case 0x20:
            goto op_semantics_3;
            break;
          case 0x30:
            goto op_semantics_4;
            break;
          case 0x40:
          case 0x50:
            goto op_semantics_5;
            break;
          case 0x60:
            goto op_semantics_6;
            break;
          case 0x70:
            goto op_semantics_7;
            break;
          case 0x80:
            goto op_semantics_8;
            break;
          case 0x90:
            goto op_semantics_9;
            break;
          case 0xa0:
            goto op_semantics_10;
            break;
          case 0xb0:
            goto op_semantics_11;
            break;
          case 0xc0:
            goto op_semantics_12;
            break;
          case 0xd0:
            goto op_semantics_13;
            break;
          case 0xe0:
            goto op_semantics_14;
            break;
          case 0xf0:
            goto op_semantics_15;
            break;
        }
      break;
    case 0x0d:
        GETBYTE ();
        switch (op[1] & 0xf0)
        {
          case 0x00:
            goto op_semantics_1;
            break;
          case 0x10:
            goto op_semantics_2;
            break;
          case 0x20:
            goto op_semantics_3;
            break;
          case 0x30:
            goto op_semantics_4;
            break;
          case 0x40:
          case 0x50:
            goto op_semantics_16;
            break;
          case 0x60:
            goto op_semantics_6;
            break;
          case 0x70:
            goto op_semantics_7;
            break;
          case 0x80:
            goto op_semantics_8;
            break;
          case 0x90:
            goto op_semantics_9;
            break;
          case 0xa0:
            goto op_semantics_10;
            break;
          case 0xb0:
            goto op_semantics_11;
            break;
          case 0xc0:
            goto op_semantics_12;
            break;
          case 0xd0:
            goto op_semantics_13;
            break;
          case 0xe0:
            goto op_semantics_14;
            break;
          case 0xf0:
            goto op_semantics_15;
            break;
        }
      break;
    case 0x0e:
        GETBYTE ();
        switch (op[1] & 0xf0)
        {
          case 0x00:
            goto op_semantics_1;
            break;
          case 0x10:
            goto op_semantics_2;
            break;
          case 0x20:
            goto op_semantics_3;
            break;
          case 0x30:
            goto op_semantics_4;
            break;
          case 0x40:
          case 0x50:
            goto op_semantics_17;
            break;
          case 0x60:
            goto op_semantics_6;
            break;
          case 0x70:
            goto op_semantics_7;
            break;
          case 0x80:
            goto op_semantics_8;
            break;
          case 0x90:
            goto op_semantics_9;
            break;
          case 0xa0:
            goto op_semantics_10;
            break;
          case 0xb0:
            goto op_semantics_11;
            break;
          case 0xc0:
            goto op_semantics_12;
            break;
          case 0xd0:
            goto op_semantics_13;
            break;
          case 0xe0:
            goto op_semantics_14;
            break;
          case 0xf0:
            goto op_semantics_15;
            break;
        }
      break;
    case 0x0f:
        GETBYTE ();
        switch (op[1] & 0xf0)
        {
          case 0x00:
            goto op_semantics_1;
            break;
          case 0x10:
            goto op_semantics_2;
            break;
          case 0x20:
            goto op_semantics_3;
            break;
          case 0x30:
            goto op_semantics_4;
            break;
          case 0x40:
          case 0x50:
            goto op_semantics_18;
            break;
          case 0x60:
            goto op_semantics_6;
            break;
          case 0x70:
            goto op_semantics_7;
            break;
          case 0x80:
            goto op_semantics_8;
            break;
          case 0x90:
            goto op_semantics_9;
            break;
          case 0xa0:
            goto op_semantics_10;
            break;
          case 0xb0:
            goto op_semantics_11;
            break;
          case 0xc0:
            goto op_semantics_12;
            break;
          case 0xd0:
            goto op_semantics_13;
            break;
          case 0xe0:
            goto op_semantics_14;
            break;
          case 0xf0:
            goto op_semantics_15;
            break;
        }
      break;
    case 0x10:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            op_semantics_19:
              {
                /** 0001 00so c b ad dreg	%S%b	%1				*/
#line 396 "msp430-decode.opc"
                int so AU = op[0] & 0x03;
#line 396 "msp430-decode.opc"
                int c AU = (op[1] >> 7) & 0x01;
#line 396 "msp430-decode.opc"
                int b AU = (op[1] >> 6) & 0x01;
#line 396 "msp430-decode.opc"
                int ad AU = (op[1] >> 4) & 0x03;
#line 396 "msp430-decode.opc"
                int dreg AU = op[1] & 0x0f;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0001 00so c b ad dreg	%S%b	%1				*/",
                           op[0], op[1]);
                    printf ("  so = 0x%x,", so);
                    printf ("  c = 0x%x,", c);
                    printf ("  b = 0x%x,", b);
                    printf ("  ad = 0x%x,", ad);
                    printf ("  dreg = 0x%x\n", dreg);
                  }
                SYNTAX("%S%b	%1");
#line 396 "msp430-decode.opc"

                ID (sopc_to_id (so,c)); ASX (dreg, ad, srxt_bits); ABW (al_bit, b);

                if (ad == 0)
                  REPZC (srxt_bits, dsxt_bits);

                /* The helper functions encode for source, but it's
                   both source and dest, with a few documented exceptions.  */
                msp430->op[0] = msp430->op[1];

                /* RETI ignores the operand.  */
                if (msp430->id == MSO_reti)
                  msp430->syntax = "%S";

                switch (msp430->id)
                  {
                  case MSO_rrc:	F_VNZC; break;
                  case MSO_swpb:	F_____; break;
                  case MSO_rra:	F_0NZC; break;
                  case MSO_sxt:	F_0NZC; break;
                  case MSO_push:	F_____; break;
                  case MSO_call:	F_____; break;
                  case MSO_reti:	F_VNZC; break;
                  default: break;
                  }

                /* 20xx 0010 0000 ---- ----
                   3cxx 0011 1100 ---- ----
                        001j mp-- ---- ----.  */
              }
            break;
        }
      break;
    case 0x11:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_19;
            break;
        }
      break;
    case 0x12:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_19;
            break;
        }
      break;
    case 0x13:
        GETBYTE ();
        switch (op[1] & 0xff)
        {
          case 0x00:
              {
                /** 0001 0011 0000 0000		RETI */
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0001 0011 0000 0000		RETI */",
                           op[0], op[1]);
                  }
                SYNTAX("RETI");
#line 550 "msp430-decode.opc"
                ID (MSO_reti);
                msp430->size = 20;
                msp430->ofs_430x = 1;

              }
            break;
          case 0x01:
          case 0x02:
          case 0x03:
          case 0x04:
          case 0x05:
          case 0x06:
          case 0x07:
          case 0x08:
          case 0x09:
          case 0x0a:
          case 0x0b:
          case 0x0c:
          case 0x0d:
          case 0x0e:
          case 0x0f:
          case 0x10:
          case 0x11:
          case 0x12:
          case 0x13:
          case 0x14:
          case 0x15:
          case 0x16:
          case 0x17:
          case 0x18:
          case 0x19:
          case 0x1a:
          case 0x1b:
          case 0x1c:
          case 0x1d:
          case 0x1e:
          case 0x1f:
          case 0x20:
          case 0x21:
          case 0x22:
          case 0x23:
          case 0x24:
          case 0x25:
          case 0x26:
          case 0x27:
          case 0x28:
          case 0x29:
          case 0x2a:
          case 0x2b:
          case 0x2c:
          case 0x2d:
          case 0x2e:
          case 0x2f:
          case 0x30:
          case 0x31:
          case 0x32:
          case 0x33:
          case 0x34:
          case 0x35:
          case 0x36:
          case 0x37:
          case 0x38:
          case 0x39:
          case 0x3a:
          case 0x3b:
          case 0x3c:
          case 0x3d:
          case 0x3e:
          case 0x3f:
          case 0xa0:
          case 0xa1:
          case 0xa2:
          case 0xa3:
          case 0xa4:
          case 0xa5:
          case 0xa6:
          case 0xa7:
          case 0xa8:
          case 0xa9:
          case 0xaa:
          case 0xab:
          case 0xac:
          case 0xad:
          case 0xae:
          case 0xaf:
          case 0xc0:
          case 0xc1:
          case 0xc2:
          case 0xc3:
          case 0xc4:
          case 0xc5:
          case 0xc6:
          case 0xc7:
          case 0xc8:
          case 0xc9:
          case 0xca:
          case 0xcb:
          case 0xcc:
          case 0xcd:
          case 0xce:
          case 0xcf:
          case 0xd0:
          case 0xd1:
          case 0xd2:
          case 0xd3:
          case 0xd4:
          case 0xd5:
          case 0xd6:
          case 0xd7:
          case 0xd8:
          case 0xd9:
          case 0xda:
          case 0xdb:
          case 0xdc:
          case 0xdd:
          case 0xde:
          case 0xdf:
          case 0xe0:
          case 0xe1:
          case 0xe2:
          case 0xe3:
          case 0xe4:
          case 0xe5:
          case 0xe6:
          case 0xe7:
          case 0xe8:
          case 0xe9:
          case 0xea:
          case 0xeb:
          case 0xec:
          case 0xed:
          case 0xee:
          case 0xef:
          case 0xf0:
          case 0xf1:
          case 0xf2:
          case 0xf3:
          case 0xf4:
          case 0xf5:
          case 0xf6:
          case 0xf7:
          case 0xf8:
          case 0xf9:
          case 0xfa:
          case 0xfb:
          case 0xfc:
          case 0xfd:
          case 0xfe:
          case 0xff:
            goto op_semantics_19;
            break;
          case 0x40:
          case 0x41:
          case 0x42:
          case 0x43:
          case 0x44:
          case 0x45:
          case 0x46:
          case 0x47:
          case 0x48:
          case 0x49:
          case 0x4a:
          case 0x4b:
          case 0x4c:
          case 0x4d:
          case 0x4e:
          case 0x4f:
          case 0x50:
          case 0x51:
          case 0x52:
          case 0x53:
          case 0x54:
          case 0x55:
          case 0x56:
          case 0x57:
          case 0x58:
          case 0x59:
          case 0x5a:
          case 0x5b:
          case 0x5c:
          case 0x5d:
          case 0x5e:
          case 0x5f:
          case 0x60:
          case 0x61:
          case 0x62:
          case 0x63:
          case 0x64:
          case 0x65:
          case 0x66:
          case 0x67:
          case 0x68:
          case 0x69:
          case 0x6a:
          case 0x6b:
          case 0x6c:
          case 0x6d:
          case 0x6e:
          case 0x6f:
          case 0x70:
          case 0x71:
          case 0x72:
          case 0x73:
          case 0x74:
          case 0x75:
          case 0x76:
          case 0x77:
          case 0x78:
          case 0x79:
          case 0x7a:
          case 0x7b:
          case 0x7c:
          case 0x7d:
          case 0x7e:
          case 0x7f:
              {
                /** 0001 0011 01as dstr		CALLA %0 */
#line 555 "msp430-decode.opc"
                int as AU = (op[1] >> 4) & 0x03;
#line 555 "msp430-decode.opc"
                int dstr AU = op[1] & 0x0f;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0001 0011 01as dstr		CALLA %0 */",
                           op[0], op[1]);
                    printf ("  as = 0x%x,", as);
                    printf ("  dstr = 0x%x\n", dstr);
                  }
                SYNTAX("CALLA %0");
#line 555 "msp430-decode.opc"
                ID (MSO_call); AS (dstr, as);
                msp430->size = 20;
                msp430->ofs_430x = 1;

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
          case 0x88:
          case 0x89:
          case 0x8a:
          case 0x8b:
          case 0x8c:
          case 0x8d:
          case 0x8e:
          case 0x8f:
              {
                /** 0001 0011 1000 extb		CALLA %0 */
#line 560 "msp430-decode.opc"
                int extb AU = op[1] & 0x0f;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0001 0011 1000 extb		CALLA %0 */",
                           op[0], op[1]);
                    printf ("  extb = 0x%x\n", extb);
                  }
                SYNTAX("CALLA %0");
#line 560 "msp430-decode.opc"
                ID (MSO_call); SA (IMMU(2) | (extb << 16));
                msp430->size = 20;
                msp430->ofs_430x = 1;

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
          case 0x98:
          case 0x99:
          case 0x9a:
          case 0x9b:
          case 0x9c:
          case 0x9d:
          case 0x9e:
          case 0x9f:
              {
                /** 0001 0011 1001 extb		CALLA %0 */
#line 565 "msp430-decode.opc"
                int extb AU = op[1] & 0x0f;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0001 0011 1001 extb		CALLA %0 */",
                           op[0], op[1]);
                    printf ("  extb = 0x%x\n", extb);
                  }
                SYNTAX("CALLA %0");
#line 565 "msp430-decode.opc"
                raddr = IMMU(2) | (extb << 16);
                if (raddr & 0x80000)
                  raddr -= 0x100000;
                ID (MSO_call); SA (pc + raddr + msp430->n_bytes);
                msp430->size = 20;
                msp430->ofs_430x = 1;

              }
            break;
          case 0xb0:
          case 0xb1:
          case 0xb2:
          case 0xb3:
          case 0xb4:
          case 0xb5:
          case 0xb6:
          case 0xb7:
          case 0xb8:
          case 0xb9:
          case 0xba:
          case 0xbb:
          case 0xbc:
          case 0xbd:
          case 0xbe:
          case 0xbf:
              {
                /** 0001 0011 1011 extb		CALLA %0 */
#line 573 "msp430-decode.opc"
                int extb AU = op[1] & 0x0f;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0001 0011 1011 extb		CALLA %0 */",
                           op[0], op[1]);
                    printf ("  extb = 0x%x\n", extb);
                  }
                SYNTAX("CALLA %0");
#line 573 "msp430-decode.opc"
                ID (MSO_call); SC (IMMU(2) | (extb << 16));
                msp430->size = 20;
                msp430->ofs_430x = 1;

              }
            break;
        }
      break;
    case 0x14:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            op_semantics_20:
              {
                /** 0001 010w bits srcr		PUSHM.A %0 */
#line 578 "msp430-decode.opc"
                int w AU = op[0] & 0x01;
#line 578 "msp430-decode.opc"
                int bits AU = (op[1] >> 4) & 0x0f;
#line 578 "msp430-decode.opc"
                int srcr AU = op[1] & 0x0f;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0001 010w bits srcr		PUSHM.A %0 */",
                           op[0], op[1]);
                    printf ("  w = 0x%x,", w);
                    printf ("  bits = 0x%x,", bits);
                    printf ("  srcr = 0x%x\n", srcr);
                  }
                SYNTAX("PUSHM.A %0");
#line 578 "msp430-decode.opc"
                ID (MSO_push); SR (srcr);
                msp430->size = w ? 16 : 20;
                msp430->repeats = bits;
                msp430->ofs_430x = 1;

              }
            break;
        }
      break;
    case 0x15:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_20;
            break;
        }
      break;
    case 0x16:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            op_semantics_21:
              {
                /** 0001 011w bits dstr		POPM.A %0 */
#line 584 "msp430-decode.opc"
                int w AU = op[0] & 0x01;
#line 584 "msp430-decode.opc"
                int bits AU = (op[1] >> 4) & 0x0f;
#line 584 "msp430-decode.opc"
                int dstr AU = op[1] & 0x0f;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0001 011w bits dstr		POPM.A %0 */",
                           op[0], op[1]);
                    printf ("  w = 0x%x,", w);
                    printf ("  bits = 0x%x,", bits);
                    printf ("  dstr = 0x%x\n", dstr);
                  }
                SYNTAX("POPM.A %0");
#line 584 "msp430-decode.opc"
                ID (MSO_pop); DR (dstr);
                msp430->size = w ? 16 : 20;
                msp430->repeats = bits;
                msp430->ofs_430x = 1;

              }
            break;
        }
      break;
    case 0x17:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_21;
            break;
        }
      break;
    case 0x18:
        GETBYTE ();
        switch (op[1] & 0x30)
        {
          case 0x00:
            op_semantics_22:
              {
                /** 0001 1srx t l 00 dsxt 	430x */
#line 352 "msp430-decode.opc"
                int srx AU = op[0] & 0x07;
#line 352 "msp430-decode.opc"
                int t AU = (op[1] >> 7) & 0x01;
#line 352 "msp430-decode.opc"
                int l AU = (op[1] >> 6) & 0x01;
#line 352 "msp430-decode.opc"
                int dsxt AU = op[1] & 0x0f;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 0001 1srx t l 00 dsxt 	430x */",
                           op[0], op[1]);
                    printf ("  srx = 0x%x,", srx);
                    printf ("  t = 0x%x,", t);
                    printf ("  l = 0x%x,", l);
                    printf ("  dsxt = 0x%x\n", dsxt);
                  }
                SYNTAX("430x");
#line 352 "msp430-decode.opc"

                al_bit = l;
                srxt_bits = srx * 2 + t;
                dsxt_bits = dsxt;
                op = op_buf + lds.op_ptr;
                msp430->ofs_430x = 1;
                goto post_extension_word;

              /* double-op insns:
                 opcode:4 sreg:4 Ad:1 BW:1 As:2 Dreg:4

                 single-op insn:
                 opcode:9 BW:1 Ad:2 DSreg:4

                 jumps:
                 opcode:3 Cond:3  pcrel:10. */

              /* Double-Operand "opcode" fields.  */

              }
            break;
          default: UNSUPPORTED(); break;
        }
      break;
    case 0x19:
        GETBYTE ();
        switch (op[1] & 0x30)
        {
          case 0x00:
            goto op_semantics_22;
            break;
          default: UNSUPPORTED(); break;
        }
      break;
    case 0x1a:
        GETBYTE ();
        switch (op[1] & 0x30)
        {
          case 0x00:
            goto op_semantics_22;
            break;
          default: UNSUPPORTED(); break;
        }
      break;
    case 0x1b:
        GETBYTE ();
        switch (op[1] & 0x30)
        {
          case 0x00:
            goto op_semantics_22;
            break;
          default: UNSUPPORTED(); break;
        }
      break;
    case 0x1c:
        GETBYTE ();
        switch (op[1] & 0x30)
        {
          case 0x00:
            goto op_semantics_22;
            break;
          default: UNSUPPORTED(); break;
        }
      break;
    case 0x1d:
        GETBYTE ();
        switch (op[1] & 0x30)
        {
          case 0x00:
            goto op_semantics_22;
            break;
          default: UNSUPPORTED(); break;
        }
      break;
    case 0x1e:
        GETBYTE ();
        switch (op[1] & 0x30)
        {
          case 0x00:
            goto op_semantics_22;
            break;
          default: UNSUPPORTED(); break;
        }
      break;
    case 0x1f:
        GETBYTE ();
        switch (op[1] & 0x30)
        {
          case 0x00:
            goto op_semantics_22;
            break;
          default: UNSUPPORTED(); break;
        }
      break;
    case 0x20:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            op_semantics_23:
              {
                /** 001jmp aa addrlsbs		%J	%1				*/
#line 426 "msp430-decode.opc"
                int jmp AU = (op[0] >> 2) & 0x07;
#line 426 "msp430-decode.opc"
                int aa AU = op[0] & 0x03;
#line 426 "msp430-decode.opc"
                int addrlsbs AU = op[1];
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** 001jmp aa addrlsbs		%J	%1				*/",
                           op[0], op[1]);
                    printf ("  jmp = 0x%x,", jmp);
                    printf ("  aa = 0x%x,", aa);
                    printf ("  addrlsbs = 0x%x\n", addrlsbs);
                  }
                SYNTAX("%J	%1");
#line 426 "msp430-decode.opc"

                raddr = (aa << 9) | (addrlsbs << 1);
                if (raddr & 0x400)
                  raddr = raddr - 0x800;
                /* This is a pc-relative jump, but we don't use SM because that
                   would load the target address from the memory at X(PC), not use
                   PC+X *as* the address.  So we use SC to use the address, not the
                   data at that address.  */
                ID (MSO_jmp); SC (pc + raddr + msp430->n_bytes);
                msp430->cond = jmp;

                /* Extended instructions.  */

              }
            break;
        }
      break;
    case 0x21:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_23;
            break;
        }
      break;
    case 0x22:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_23;
            break;
        }
      break;
    case 0x23:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_23;
            break;
        }
      break;
    case 0x24:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_23;
            break;
        }
      break;
    case 0x25:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_23;
            break;
        }
      break;
    case 0x26:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_23;
            break;
        }
      break;
    case 0x27:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_23;
            break;
        }
      break;
    case 0x28:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_23;
            break;
        }
      break;
    case 0x29:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_23;
            break;
        }
      break;
    case 0x2a:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_23;
            break;
        }
      break;
    case 0x2b:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_23;
            break;
        }
      break;
    case 0x2c:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_23;
            break;
        }
      break;
    case 0x2d:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_23;
            break;
        }
      break;
    case 0x2e:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_23;
            break;
        }
      break;
    case 0x2f:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_23;
            break;
        }
      break;
    case 0x30:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_23;
            break;
        }
      break;
    case 0x31:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_23;
            break;
        }
      break;
    case 0x32:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_23;
            break;
        }
      break;
    case 0x33:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_23;
            break;
        }
      break;
    case 0x34:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_23;
            break;
        }
      break;
    case 0x35:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_23;
            break;
        }
      break;
    case 0x36:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_23;
            break;
        }
      break;
    case 0x37:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_23;
            break;
        }
      break;
    case 0x38:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_23;
            break;
        }
      break;
    case 0x39:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_23;
            break;
        }
      break;
    case 0x3a:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_23;
            break;
        }
      break;
    case 0x3b:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_23;
            break;
        }
      break;
    case 0x3c:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_23;
            break;
        }
      break;
    case 0x3d:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_23;
            break;
        }
      break;
    case 0x3e:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_23;
            break;
        }
      break;
    case 0x3f:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_23;
            break;
        }
      break;
    case 0x40:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            op_semantics_24:
              {
                /** dopc sreg a b as dreg	%D%b	%1,%0				*/
#line 373 "msp430-decode.opc"
                int dopc AU = (op[0] >> 4) & 0x0f;
#line 373 "msp430-decode.opc"
                int sreg AU = op[0] & 0x0f;
#line 373 "msp430-decode.opc"
                int a AU = (op[1] >> 7) & 0x01;
#line 373 "msp430-decode.opc"
                int b AU = (op[1] >> 6) & 0x01;
#line 373 "msp430-decode.opc"
                int as AU = (op[1] >> 4) & 0x03;
#line 373 "msp430-decode.opc"
                int dreg AU = op[1] & 0x0f;
                if (trace)
                  {
                    printf ("\033[33m%s\033[0m  %02x %02x\n",
                           "/** dopc sreg a b as dreg	%D%b	%1,%0				*/",
                           op[0], op[1]);
                    printf ("  dopc = 0x%x,", dopc);
                    printf ("  sreg = 0x%x,", sreg);
                    printf ("  a = 0x%x,", a);
                    printf ("  b = 0x%x,", b);
                    printf ("  as = 0x%x,", as);
                    printf ("  dreg = 0x%x\n", dreg);
                  }
                SYNTAX("%D%b	%1,%0");
#line 373 "msp430-decode.opc"

                ID (dopc_to_id (dopc)); ASX (sreg, as, srxt_bits); ADX (dreg, a, dsxt_bits); ABW (al_bit, b);
                if (a == 0 && as == 0)
                  REPZC (srxt_bits, dsxt_bits);

                switch (msp430->id)
                  {
                  case MSO_mov:	F_____; break;
                  case MSO_add:	F_VNZC; break;
                  case MSO_addc:	F_VNZC; break;
                  case MSO_subc:	F_VNZC; break;
                  case MSO_sub:	F_VNZC; break;
                  case MSO_cmp:	F_VNZC; break;
                  case MSO_dadd:	F_VNZC; break;
                  case MSO_bit:	F_0NZC; break;
                  case MSO_bic:	F_____; break;
                  case MSO_bis:	F_____; break;
                  case MSO_xor:	F_VNZC; break;
                  case MSO_and:	F_0NZC; break;
                  default: break;
                  }

              }
            break;
        }
      break;
    case 0x41:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x42:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x43:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x44:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x45:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x46:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x47:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x48:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x49:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x4a:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x4b:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x4c:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x4d:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x4e:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x4f:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x50:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x51:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x52:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x53:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x54:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x55:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x56:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x57:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x58:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x59:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x5a:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x5b:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x5c:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x5d:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x5e:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x5f:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x60:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x61:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x62:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x63:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x64:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x65:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x66:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x67:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x68:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x69:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x6a:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x6b:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x6c:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x6d:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x6e:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x6f:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x70:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x71:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x72:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x73:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x74:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x75:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x76:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x77:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x78:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x79:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x7a:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x7b:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x7c:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x7d:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x7e:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x7f:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x80:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x81:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x82:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x83:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x84:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x85:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x86:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x87:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x88:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x89:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x8a:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x8b:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x8c:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x8d:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x8e:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x8f:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x90:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x91:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x92:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x93:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x94:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x95:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x96:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x97:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x98:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x99:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x9a:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x9b:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x9c:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x9d:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x9e:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0x9f:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xa0:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xa1:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xa2:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xa3:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xa4:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xa5:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xa6:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xa7:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xa8:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xa9:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xaa:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xab:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xac:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xad:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xae:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xaf:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xb0:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xb1:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xb2:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xb3:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xb4:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xb5:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xb6:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xb7:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xb8:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xb9:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xba:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xbb:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xbc:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xbd:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xbe:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xbf:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xc0:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xc1:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xc2:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xc3:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xc4:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xc5:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xc6:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xc7:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xc8:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xc9:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xca:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xcb:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xcc:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xcd:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xce:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xcf:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xd0:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xd1:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xd2:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xd3:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xd4:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xd5:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xd6:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xd7:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xd8:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xd9:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xda:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xdb:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xdc:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xdd:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xde:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xdf:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xe0:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xe1:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xe2:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xe3:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xe4:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xe5:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xe6:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xe7:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xe8:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xe9:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xea:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xeb:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xec:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xed:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xee:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xef:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xf0:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xf1:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xf2:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xf3:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xf4:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xf5:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xf6:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xf7:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xf8:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xf9:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xfa:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xfb:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xfc:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xfd:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xfe:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
    case 0xff:
        GETBYTE ();
        switch (op[1] & 0x00)
        {
          case 0x00:
            goto op_semantics_24;
            break;
        }
      break;
  }
#line 590 "msp430-decode.opc"

  return msp430->n_bytes;
}
