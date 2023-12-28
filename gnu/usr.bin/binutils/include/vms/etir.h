/* Alpha VMS external format of Extended Text Information and Relocation.

   Copyright (C) 2010-2023 Free Software Foundation, Inc.
   Written by Tristan Gingold <gingold@adacore.com>, AdaCore.

   This file is part of BFD, the Binary File Descriptor library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#ifndef _VMS_ETIR_H
#define _VMS_ETIR_H

#define ETIR__C_MINSTACOD 0	/* Minimum stack code.		*/
#define ETIR__C_STA_GBL   0	/* Stack global symbol value.	*/
#define ETIR__C_STA_LW    1	/* Stack longword.		*/
#define ETIR__C_STA_QW    2	/* Stack quadword.		*/
#define ETIR__C_STA_PQ    3	/* Stack psect base + quadword off.  */
#define ETIR__C_STA_LI    4	/* Stack literal.		*/
#define ETIR__C_STA_MOD   5	/* Stack module.		*/
#define ETIR__C_STA_CKARG 6	/* Check Arguments.		*/
#define ETIR__C_MAXSTACOD 6	/* Maximum stack code.		*/

#define ETIR__C_MINSTOCOD  50	/* Minimum store code.		*/
#define ETIR__C_STO_B      50	/* Store byte.			*/
#define ETIR__C_STO_W      51	/* Store word.			*/
#define ETIR__C_STO_LW     52	/* Store longword.		*/
#define ETIR__C_STO_QW     53	/* Store quadword.		*/
#define ETIR__C_STO_IMMR   54	/* Store immediate Repeated.	*/
#define ETIR__C_STO_GBL    55	/* Store global.		*/
#define ETIR__C_STO_CA     56	/* Store code address.		*/
#define ETIR__C_STO_RB     57	/* Store relative branch.	*/
#define ETIR__C_STO_AB     58	/* Store absolute branch.	*/
#define ETIR__C_STO_OFF    59	/* Store offset within psect.	*/
#define ETIR__C_STO_IMM    61	/* Store immediate.		*/
#define ETIR__C_STO_GBL_LW 62	/* Store global Longword.	*/
#define ETIR__C_STO_LP_PSB 63	/* STO_LP_PSB not valid in level 2 use STC_LP_PSB.  */
#define ETIR__C_STO_HINT_GBL 64  /* Store 14 bit HINT at global address.  */
#define ETIR__C_STO_HINT_PS  65  /* Store 14 bit HINT at psect + offset */
#define ETIR__C_MAXSTOCOD    65  /* Maximum store code.		*/

/* Operate codes.  */
#define ETIR__C_MINOPRCOD 100	/* Minimum operate code.	*/
#define ETIR__C_OPR_NOP   100	/* No-op.			*/
#define ETIR__C_OPR_ADD   101	/* Add.				*/
#define ETIR__C_OPR_SUB   102	/* Subtract.			*/
#define ETIR__C_OPR_MUL   103	/* Multiply.			*/
#define ETIR__C_OPR_DIV   104	/* Divide.			*/
#define ETIR__C_OPR_AND   105	/* Logical AND.			*/
#define ETIR__C_OPR_IOR   106	/* Logical inclusive OR.	*/
#define ETIR__C_OPR_EOR   107	/* Logical exclusive OR.	*/
#define ETIR__C_OPR_NEG   108	/* Negate.			*/
#define ETIR__C_OPR_COM   109	/* Complement.			*/
#define ETIR__C_OPR_INSV  110	/* Insert bit field.		*/
#define ETIR__C_OPR_ASH   111	/* Arithmetic shift.		*/
#define ETIR__C_OPR_USH   112	/* Unsigned shift.		*/
#define ETIR__C_OPR_ROT   113	/* Rotate.			*/
#define ETIR__C_OPR_SEL   114	/* Select one of 3 long on top of stack.   */
#define ETIR__C_OPR_REDEF 115	/* Redefine this symbol after pass 2.  */
#define ETIR__C_OPR_DFLIT 116	/* Define a literal.		*/
#define ETIR__C_MAXOPRCOD 116	/* Maximum operate code.	*/

/* Control codes.  */
#define ETIR__C_MINCTLCOD 150	/* Minimum control code.	*/
#define ETIR__C_CTL_SETRB 150	/* Set relocation base.		*/
#define ETIR__C_CTL_AUGRB 151	/* Augment relocation base.	*/
#define ETIR__C_CTL_DFLOC 152	/* Define debug location.	*/
#define ETIR__C_CTL_STLOC 153	/* Set debug location.		*/
#define ETIR__C_CTL_STKDL 154	/* Stack debug location.	*/
#define ETIR__C_MAXCTLCOD 154	/* Maximum control code.	*/

/* Store-conditional (STC) codes.  */
#define ETIR__C_MINSTCCOD   200 /* Minimum store-conditional code.   */
#define ETIR__C_STC_LP      200 /* STC Linkage Pair.   */
#define ETIR__C_STC_LP_PSB  201 /* STC Linkage Pair with Proc Signature.  */
#define ETIR__C_STC_GBL     202 /* STC Address at global address.  */
#define ETIR__C_STC_GCA     203 /* STC Code Address at global address.  */
#define ETIR__C_STC_PS      204 /* STC Address at psect + offset.  */
#define ETIR__C_STC_NOP_GBL 205 /* STC NOP at address of global.  */
#define ETIR__C_STC_NOP_PS  206 /* STC NOP at pect + offset.  */
#define ETIR__C_STC_BSR_GBL 207 /* STC BSR at global address.  */
#define ETIR__C_STC_BSR_PS  208 /* STC BSR at pect + offset.  */
#define ETIR__C_STC_LDA_GBL 209 /* STC LDA at global address.  */
#define ETIR__C_STC_LDA_PS  210 /* STC LDA at psect + offset.  */
#define ETIR__C_STC_BOH_GBL 211 /* STC BSR or Hint at global address.  */
#define ETIR__C_STC_BOH_PS  212 /* STC BSR or Hint at pect + offset.  */
#define ETIR__C_STC_NBH_GBL 213 /* STC NOP,BSR or HINT at global address.  */
#define ETIR__C_STC_NBH_PS  214 /* STC NOP,BSR or HINT at psect + offset.  */
#define ETIR__C_MAXSTCCOD   214 /* Maximum store-conditional code.   */

#define ETIR__C_HEADER_SIZE 4	/* Size of the header of a command */

struct vms_etir
{
  /* Commands.  See above.  */
  unsigned char rectyp[2];

  /* Size (including this header).  */
  unsigned char size[2];
};

#endif /* _VMS_ETIR_H */
