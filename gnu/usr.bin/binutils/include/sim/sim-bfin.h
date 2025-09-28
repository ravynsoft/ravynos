/* This file defines the interface between the Blackfin simulator and GDB.

   Copyright (C) 2005-2023 Free Software Foundation, Inc.
   Contributed by Analog Devices.

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

enum sim_bfin_regnum {
  SIM_BFIN_R0_REGNUM = 0,
  SIM_BFIN_R1_REGNUM,
  SIM_BFIN_R2_REGNUM,
  SIM_BFIN_R3_REGNUM,
  SIM_BFIN_R4_REGNUM,
  SIM_BFIN_R5_REGNUM,
  SIM_BFIN_R6_REGNUM,
  SIM_BFIN_R7_REGNUM,
  SIM_BFIN_P0_REGNUM,
  SIM_BFIN_P1_REGNUM,
  SIM_BFIN_P2_REGNUM,
  SIM_BFIN_P3_REGNUM,
  SIM_BFIN_P4_REGNUM,
  SIM_BFIN_P5_REGNUM,
  SIM_BFIN_SP_REGNUM,
  SIM_BFIN_FP_REGNUM,
  SIM_BFIN_I0_REGNUM,
  SIM_BFIN_I1_REGNUM,
  SIM_BFIN_I2_REGNUM,
  SIM_BFIN_I3_REGNUM,
  SIM_BFIN_M0_REGNUM,
  SIM_BFIN_M1_REGNUM,
  SIM_BFIN_M2_REGNUM,
  SIM_BFIN_M3_REGNUM,
  SIM_BFIN_B0_REGNUM,
  SIM_BFIN_B1_REGNUM,
  SIM_BFIN_B2_REGNUM,
  SIM_BFIN_B3_REGNUM,
  SIM_BFIN_L0_REGNUM,
  SIM_BFIN_L1_REGNUM,
  SIM_BFIN_L2_REGNUM,
  SIM_BFIN_L3_REGNUM,
  SIM_BFIN_A0_DOT_X_REGNUM,
  SIM_BFIN_A0_DOT_W_REGNUM,
  SIM_BFIN_A1_DOT_X_REGNUM,
  SIM_BFIN_A1_DOT_W_REGNUM,
  SIM_BFIN_ASTAT_REGNUM,
  SIM_BFIN_RETS_REGNUM,
  SIM_BFIN_LC0_REGNUM,
  SIM_BFIN_LT0_REGNUM,
  SIM_BFIN_LB0_REGNUM,
  SIM_BFIN_LC1_REGNUM,
  SIM_BFIN_LT1_REGNUM,
  SIM_BFIN_LB1_REGNUM,
  SIM_BFIN_CYCLES_REGNUM,
  SIM_BFIN_CYCLES2_REGNUM,
  SIM_BFIN_USP_REGNUM,
  SIM_BFIN_SEQSTAT_REGNUM,
  SIM_BFIN_SYSCFG_REGNUM,
  SIM_BFIN_RETI_REGNUM,
  SIM_BFIN_RETX_REGNUM,
  SIM_BFIN_RETN_REGNUM,
  SIM_BFIN_RETE_REGNUM,
  SIM_BFIN_PC_REGNUM,
  SIM_BFIN_CC_REGNUM,
  SIM_BFIN_TEXT_ADDR,
  SIM_BFIN_TEXT_END_ADDR,
  SIM_BFIN_DATA_ADDR,
  SIM_BFIN_IPEND_REGNUM
};

