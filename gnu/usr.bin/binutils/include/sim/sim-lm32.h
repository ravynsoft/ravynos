/* This file defines the interface between the LM32 simulator and GDB.
   Contributed by Jon Beniston <jon@beniston.com>

   Copyright (C) 2009-2023 Free Software Foundation, Inc.

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

#ifndef SIM_LM32_H
#define SIM_LM32_H

enum sim_lm32_regs
{
  SIM_LM32_R0_REGNUM,
  SIM_LM32_R1_REGNUM,     
  SIM_LM32_R2_REGNUM,     
  SIM_LM32_R3_REGNUM,     
  SIM_LM32_R4_REGNUM,     
  SIM_LM32_R5_REGNUM,     
  SIM_LM32_R6_REGNUM,     
  SIM_LM32_R7_REGNUM,     
  SIM_LM32_R8_REGNUM,     
  SIM_LM32_R9_REGNUM,     
  SIM_LM32_R10_REGNUM,    
  SIM_LM32_R11_REGNUM,    
  SIM_LM32_R12_REGNUM,    
  SIM_LM32_R13_REGNUM,    
  SIM_LM32_R14_REGNUM,    
  SIM_LM32_R15_REGNUM,    
  SIM_LM32_R16_REGNUM,    
  SIM_LM32_R17_REGNUM,    
  SIM_LM32_R18_REGNUM,    
  SIM_LM32_R19_REGNUM,    
  SIM_LM32_R20_REGNUM,    
  SIM_LM32_R21_REGNUM,    
  SIM_LM32_R22_REGNUM,    
  SIM_LM32_R23_REGNUM,    
  SIM_LM32_R24_REGNUM,    
  SIM_LM32_R25_REGNUM,    
  SIM_LM32_GP_REGNUM,     
  SIM_LM32_FP_REGNUM,     
  SIM_LM32_SP_REGNUM,     
  SIM_LM32_RA_REGNUM,     
  SIM_LM32_BA_REGNUM,     
  SIM_LM32_EA_REGNUM,     
  SIM_LM32_PC_REGNUM,
  SIM_LM32_EID_REGNUM,
  SIM_LM32_EBA_REGNUM,  
  SIM_LM32_DEBA_REGNUM, 
  SIM_LM32_IE_REGNUM,  
  SIM_LM32_IM_REGNUM,  
  SIM_LM32_IP_REGNUM,  
  SIM_LM32_NUM_REGS      
};

#endif
