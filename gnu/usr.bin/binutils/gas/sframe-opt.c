/* sframe-opt.c - optimize FRE and FDE information in SFrame.
   Copyright (C) 2022-2023 Free Software Foundation, Inc.

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#include "as.h"
#include "sframe.h"

/* The function estimates the size of a rs_sframe variant frag based on
   the current values of the symbols.  It is called before the
   relaxation loop.  We set fr_subtype{0:2} to the expected length.  */

int
sframe_estimate_size_before_relax (fragS *frag)
{
  offsetT width;
  expressionS *exp;
  symbolS *widthS;
  int ret;

  /* We are dealing with two different kind of fragments here which need
     to be fixed up:
       - first, FRE start address in each FRE, and
       - second, Function info in each FDE (function info stores the FRE type)
     The two kind of fragments can be differentiated based on the opcode
     of the symbol.  */
  exp = symbol_get_value_expression (frag->fr_symbol);
  gas_assert ((exp->X_op == O_modulus) || (exp->X_op == O_absent));
  /* Fragment for function info in an SFrame FDE will always write
     only one byte.  */
  if (exp->X_op == O_modulus)
    ret = 1;
  /* Fragment for the start address in an SFrame FRE may write out
     1/2/4 bytes depending on the value of the diff.  */
  else
    {
      /* Get the width expression from the symbol.  */
      widthS = exp->X_op_symbol;
      width = resolve_symbol_value (widthS);

      if (width < (offsetT) SFRAME_FRE_TYPE_ADDR1_LIMIT)
	ret = 1;
      else if (width < (offsetT) SFRAME_FRE_TYPE_ADDR2_LIMIT)
	ret = 2;
      else
	ret = 4;
    }

  frag->fr_subtype = (frag->fr_subtype & ~7) | (ret & 7);

  return ret;
}

/* This function relaxes a rs_sframe variant frag based on the current
   values of the symbols.  fr_subtype{0:2} is the current length of
   the frag.  This returns the change in frag length.  */

int
sframe_relax_frag (fragS *frag)
{
  int oldsize, newsize;

  oldsize = frag->fr_subtype & 7;
  if (oldsize == 7)
    oldsize = -1;
  newsize = sframe_estimate_size_before_relax (frag);
  return newsize - oldsize;
}

/* This function converts a rs_sframe variant frag into a normal fill
   frag.  This is called after all relaxation has been done.
   fr_subtype{0:2} will be the desired length of the frag.  */

void
sframe_convert_frag (fragS *frag)
{
  offsetT fsize;
  offsetT diff;
  offsetT value;

  offsetT rest_of_data;
  uint8_t fde_type, fre_type;
  uint8_t pauth_key;

  expressionS *exp;
  symbolS *dataS;
  symbolS *fsizeS, *diffS;

  /* We are dealing with two different kind of fragments here which need
     to be fixed up:
       - first, FRE start address in each FRE, and
       - second, Function info in each FDE (function info stores the FRE type)
     The two kind of fragments can be differentiated based on the opcode
     of the symbol.  */
  exp = symbol_get_value_expression (frag->fr_symbol);
  gas_assert ((exp->X_op == O_modulus) || (exp->X_op == O_absent));
  /* Fragment for function info in an SFrame FDE.  */
  if (exp->X_op == O_modulus)
    {
      /* Gather the existing value of the rest of the data except
	 the fre_type.  */
      dataS = exp->X_add_symbol;
      rest_of_data = (symbol_get_value_expression(dataS))->X_add_number;
      fde_type = SFRAME_V1_FUNC_FDE_TYPE (rest_of_data);
      pauth_key = SFRAME_V1_FUNC_PAUTH_KEY (rest_of_data);
      gas_assert (fde_type == SFRAME_FDE_TYPE_PCINC);

      /* Calculate the applicable fre_type.  */
      fsizeS = exp->X_op_symbol;
      fsize = resolve_symbol_value (fsizeS);
      if (fsize < (offsetT) SFRAME_FRE_TYPE_ADDR1_LIMIT)
	fre_type = SFRAME_FRE_TYPE_ADDR1;
      else if (fsize < (offsetT) SFRAME_FRE_TYPE_ADDR2_LIMIT)
	fre_type = SFRAME_FRE_TYPE_ADDR2;
      else
	fre_type = SFRAME_FRE_TYPE_ADDR4;

      /* Create the new function info.  */
      value = SFRAME_V1_FUNC_INFO (fde_type, fre_type);
      value = SFRAME_V1_FUNC_INFO_UPDATE_PAUTH_KEY (pauth_key, value);

      frag->fr_literal[frag->fr_fix] = value;
    }
  /* Fragment for the start address in an SFrame FRE.  */
  else
    {
      /* Get the fsize expression from the symbol.  */
      fsizeS = exp->X_op_symbol;
      fsize = resolve_symbol_value (fsizeS);
      /* Get the diff expression from the symbol.  */
      diffS= exp->X_add_symbol;
      diff = resolve_symbol_value (diffS);
      value = diff;

      switch (frag->fr_subtype & 7)
	{
	case 1:
	  gas_assert (fsize < (offsetT) SFRAME_FRE_TYPE_ADDR1_LIMIT);
	  frag->fr_literal[frag->fr_fix] = diff;
	  break;
	case 2:
	  gas_assert (fsize < (offsetT) SFRAME_FRE_TYPE_ADDR2_LIMIT);
	  md_number_to_chars (frag->fr_literal + frag->fr_fix, diff, 2);
	  break;
	case 4:
	  md_number_to_chars (frag->fr_literal + frag->fr_fix, diff, 4);
	  break;
	default:
	  abort ();
	}
    }

  frag->fr_fix += frag->fr_subtype & 7;
  frag->fr_type = rs_fill;
  frag->fr_subtype = 0;
  frag->fr_offset = 0;
  /* FIXME do this now because we have evaluated and fixed up the fragments
     manually ?  */
  frag->fr_symbol = 0;
}
