/* sim-rx.h --- interface between RX simulator and GDB.

   Copyright (C) 2008-2023 Free Software Foundation, Inc.

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

#if !defined (SIM_RX_H)
#define SIM_RX_H

enum sim_rx_regnum
  {
    sim_rx_r0_regnum,
    sim_rx_r1_regnum,
    sim_rx_r2_regnum,
    sim_rx_r3_regnum,
    sim_rx_r4_regnum,
    sim_rx_r5_regnum,
    sim_rx_r6_regnum,
    sim_rx_r7_regnum,
    sim_rx_r8_regnum,
    sim_rx_r9_regnum,
    sim_rx_r10_regnum,
    sim_rx_r11_regnum,
    sim_rx_r12_regnum,
    sim_rx_r13_regnum,
    sim_rx_r14_regnum,
    sim_rx_r15_regnum,
    sim_rx_usp_regnum,
    sim_rx_isp_regnum,
    sim_rx_ps_regnum,
    sim_rx_pc_regnum,
    sim_rx_intb_regnum,
    sim_rx_bpsw_regnum,
    sim_rx_bpc_regnum,
    sim_rx_fintv_regnum,
    sim_rx_fpsw_regnum,
    sim_rx_acc_regnum,
    sim_rx_num_regs
  };

#endif /* SIM_RX_H */
