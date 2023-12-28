/* This file defines the interface between the FT32 simulator and GDB.

   Copyright (C) 2005-2023 Free Software Foundation, Inc.
   Contributed by FTDI.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */


/* Register numbers of various important registers.  */
enum ft32_regnum
{
  FT32_FP_REGNUM,               /* Address of executing stack frame.  */
  FT32_SP_REGNUM,               /* Address of top of stack.  */
  FT32_R0_REGNUM,
  FT32_R1_REGNUM,
  FT32_CC_REGNUM = 31,
  FT32_PC_REGNUM = 32           /* Program counter.  */
};

/* Number of machine registers.  */
#define FT32_NUM_REGS 33        /* 32 real registers + PC */

