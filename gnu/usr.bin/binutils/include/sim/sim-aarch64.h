/* sim-aarch64.h --- interface between AArch64 simulator and GDB.

   Copyright (C) 2015-2023 Free Software Foundation, Inc.

   Contributed by Red Hat.

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

#if !defined (SIM_AARCH64_H)
#define SIM_AARCH64_H

enum sim_aarch64_regnum
{
  SIM_AARCH64_R0_REGNUM,
  SIM_AARCH64_R1_REGNUM,
  SIM_AARCH64_R2_REGNUM,
  SIM_AARCH64_R3_REGNUM,
  SIM_AARCH64_R4_REGNUM,
  SIM_AARCH64_R5_REGNUM,
  SIM_AARCH64_R6_REGNUM,
  SIM_AARCH64_R7_REGNUM,
  SIM_AARCH64_R8_REGNUM,
  SIM_AARCH64_R9_REGNUM,
  SIM_AARCH64_R10_REGNUM,
  SIM_AARCH64_R11_REGNUM,
  SIM_AARCH64_R12_REGNUM,
  SIM_AARCH64_R13_REGNUM,
  SIM_AARCH64_R14_REGNUM,
  SIM_AARCH64_R15_REGNUM,
  SIM_AARCH64_SP_REGNUM,
  SIM_AARCH64_PC_REGNUM,
  SIM_AARCH64_NUM_REGS
};

#endif
