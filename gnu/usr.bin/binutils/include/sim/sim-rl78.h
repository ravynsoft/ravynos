/* sim-rx.h --- interface between rl78 simulator and GDB.

   Copyright (C) 2011-2023 Free Software Foundation, Inc.

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

#if !defined (SIM_RL78_H)
#define SIM_RL78_H

enum sim_rl78_regnum
{
  sim_rl78_bank0_r0_regnum,
  sim_rl78_bank0_r1_regnum,
  sim_rl78_bank0_r2_regnum,
  sim_rl78_bank0_r3_regnum,
  sim_rl78_bank0_r4_regnum,
  sim_rl78_bank0_r5_regnum,
  sim_rl78_bank0_r6_regnum,
  sim_rl78_bank0_r7_regnum,

  sim_rl78_bank1_r0_regnum,
  sim_rl78_bank1_r1_regnum,
  sim_rl78_bank1_r2_regnum,
  sim_rl78_bank1_r3_regnum,
  sim_rl78_bank1_r4_regnum,
  sim_rl78_bank1_r5_regnum,
  sim_rl78_bank1_r6_regnum,
  sim_rl78_bank1_r7_regnum,

  sim_rl78_bank2_r0_regnum,
  sim_rl78_bank2_r1_regnum,
  sim_rl78_bank2_r2_regnum,
  sim_rl78_bank2_r3_regnum,
  sim_rl78_bank2_r4_regnum,
  sim_rl78_bank2_r5_regnum,
  sim_rl78_bank2_r6_regnum,
  sim_rl78_bank2_r7_regnum,

  sim_rl78_bank3_r0_regnum,
  sim_rl78_bank3_r1_regnum,
  sim_rl78_bank3_r2_regnum,
  sim_rl78_bank3_r3_regnum,
  sim_rl78_bank3_r4_regnum,
  sim_rl78_bank3_r5_regnum,
  sim_rl78_bank3_r6_regnum,
  sim_rl78_bank3_r7_regnum,

  sim_rl78_psw_regnum,
  sim_rl78_es_regnum,
  sim_rl78_cs_regnum,
  sim_rl78_pc_regnum,

  sim_rl78_spl_regnum,
  sim_rl78_sph_regnum,
  sim_rl78_pmc_regnum,
  sim_rl78_mem_regnum,

  sim_rl78_num_regs
};

#endif /* SIM_RL78_H */
