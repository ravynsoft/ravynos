/* score-datadep.h -- Score Instructions data dependency table
   Copyright (C) 2006-2023 Free Software Foundation, Inc.
   Contributed by: 
   Brain.lin (brain.lin@sunplusct.com)
   Mei Ligang (ligang@sunnorth.com.cn)
   Pei-Lin Tsai (pltsai@sunplus.com)

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
   along with GAS; see the file COPYING3.  If not, write to the Free
   Software Foundation, Inc., 51 Franklin Street - Fifth Floor,
   Boston, MA 02110-1301, USA.  */

#ifndef SCORE_DATA_DEPENDENCY_H
#define SCORE_DATA_DEPENDENCY_H

#define INSN_NAME_LEN 16

enum insn_type_for_dependency
{
  D_mtcr,
  D_all_insn
};

struct insn_to_dependency
{
  char *insn_name;
  enum insn_type_for_dependency type;
};

struct data_dependency
{
  enum insn_type_for_dependency pre_insn_type;
  char pre_reg[6];
  enum insn_type_for_dependency cur_insn_type;
  char cur_reg[6];
  int bubblenum_7;
  int bubblenum_3;
  int warn_or_error;           /* warning - 0; error - 1  */
};

static const struct insn_to_dependency insn_to_dependency_table[] =
{
  /* move spectial instruction.  */
  {"mtcr",      D_mtcr},
};

static const struct data_dependency data_dependency_table[] =
{
  /* Status regiser.  */
  {D_mtcr, "cr0", D_all_insn, "", 5, 1, 0},
};

#endif
