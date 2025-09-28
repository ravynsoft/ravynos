/* DO NOT EDIT!  -*- buffer-read-only: t -*- vi:set ro:  */
/* Semantic operand instances for or1k.

THIS FILE IS MACHINE GENERATED WITH CGEN.

Copyright (C) 1996-2023 Free Software Foundation, Inc.

This file is part of the GNU Binutils and/or GDB, the GNU debugger.

   This file is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   It is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.

*/

#include "sysdep.h"
#include "ansidecl.h"
#include "bfd.h"
#include "symcat.h"
#include "or1k-desc.h"
#include "or1k-opc.h"

/* Operand references.  */

#define OP_ENT(op) OR1K_OPERAND_##op
#define INPUT CGEN_OPINST_INPUT
#define OUTPUT CGEN_OPINST_OUTPUT
#define END CGEN_OPINST_END
#define COND_REF CGEN_OPINST_COND_REF

static const CGEN_OPINST sfmt_empty_ops[] ATTRIBUTE_UNUSED = {
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_j_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "disp26", HW_H_IADDR, CGEN_MODE_USI, OP_ENT (DISP26), 0, 0 },
  { INPUT, "sys_cpucfgr_nd", HW_H_SYS_CPUCFGR_ND, CGEN_MODE_USI, 0, 0, 0 },
  { OUTPUT, "pc", HW_H_PC, CGEN_MODE_USI, 0, 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_adrp_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "disp21", HW_H_IADDR, CGEN_MODE_USI, OP_ENT (DISP21), 0, 0 },
  { OUTPUT, "rD", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RD), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_jal_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "disp26", HW_H_IADDR, CGEN_MODE_USI, OP_ENT (DISP26), 0, 0 },
  { INPUT, "pc", HW_H_PC, CGEN_MODE_USI, 0, 0, 0 },
  { INPUT, "sys_cpucfgr_nd", HW_H_SYS_CPUCFGR_ND, CGEN_MODE_USI, 0, 0, 0 },
  { OUTPUT, "h_gpr_USI_9", HW_H_GPR, CGEN_MODE_USI, 0, 9, 0 },
  { OUTPUT, "pc", HW_H_PC, CGEN_MODE_USI, 0, 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_jr_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "rB", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RB), 0, 0 },
  { INPUT, "sys_cpucfgr_nd", HW_H_SYS_CPUCFGR_ND, CGEN_MODE_USI, 0, 0, 0 },
  { OUTPUT, "pc", HW_H_PC, CGEN_MODE_USI, 0, 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_jalr_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "pc", HW_H_PC, CGEN_MODE_USI, 0, 0, 0 },
  { INPUT, "rB", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RB), 0, 0 },
  { INPUT, "sys_cpucfgr_nd", HW_H_SYS_CPUCFGR_ND, CGEN_MODE_USI, 0, 0, 0 },
  { OUTPUT, "h_gpr_USI_9", HW_H_GPR, CGEN_MODE_USI, 0, 9, 0 },
  { OUTPUT, "pc", HW_H_PC, CGEN_MODE_USI, 0, 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_bnf_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "disp26", HW_H_IADDR, CGEN_MODE_USI, OP_ENT (DISP26), 0, COND_REF },
  { INPUT, "pc", HW_H_PC, CGEN_MODE_USI, 0, 0, COND_REF },
  { INPUT, "sys_cpucfgr_nd", HW_H_SYS_CPUCFGR_ND, CGEN_MODE_USI, 0, 0, COND_REF },
  { INPUT, "sys_sr_f", HW_H_SYS_SR_F, CGEN_MODE_USI, 0, 0, 0 },
  { OUTPUT, "pc", HW_H_PC, CGEN_MODE_USI, 0, 0, COND_REF },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_trap_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "pc", HW_H_PC, CGEN_MODE_USI, 0, 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_msync_ops[] ATTRIBUTE_UNUSED = {
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_nop_imm_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "uimm16", HW_H_UIMM16, CGEN_MODE_UINT, OP_ENT (UIMM16), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_movhi_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "uimm16", HW_H_UIMM16, CGEN_MODE_UINT, OP_ENT (UIMM16), 0, 0 },
  { OUTPUT, "rD", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RD), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_macrc_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "mac_maclo", HW_H_MAC_MACLO, CGEN_MODE_USI, 0, 0, 0 },
  { OUTPUT, "mac_machi", HW_H_MAC_MACHI, CGEN_MODE_USI, 0, 0, 0 },
  { OUTPUT, "mac_maclo", HW_H_MAC_MACLO, CGEN_MODE_USI, 0, 0, 0 },
  { OUTPUT, "rD", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RD), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_mfspr_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "rA", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RA), 0, 0 },
  { INPUT, "uimm16", HW_H_UIMM16, CGEN_MODE_UINT, OP_ENT (UIMM16), 0, 0 },
  { OUTPUT, "rD", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RD), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_mtspr_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "rA", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RA), 0, 0 },
  { INPUT, "rB", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RB), 0, 0 },
  { INPUT, "uimm16_split", HW_H_UIMM16, CGEN_MODE_UINT, OP_ENT (UIMM16_SPLIT), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_lwz_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "h_memory_USI_c_call__AI_@cpu@_make_load_store_addr_rA_ext__SI_simm16_4", HW_H_MEMORY, CGEN_MODE_USI, 0, 0, 0 },
  { INPUT, "rA", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RA), 0, 0 },
  { INPUT, "simm16", HW_H_SIMM16, CGEN_MODE_INT, OP_ENT (SIMM16), 0, 0 },
  { OUTPUT, "rD", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RD), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_lws_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "h_memory_SI_c_call__AI_@cpu@_make_load_store_addr_rA_ext__SI_simm16_4", HW_H_MEMORY, CGEN_MODE_SI, 0, 0, 0 },
  { INPUT, "rA", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RA), 0, 0 },
  { INPUT, "simm16", HW_H_SIMM16, CGEN_MODE_INT, OP_ENT (SIMM16), 0, 0 },
  { OUTPUT, "rD", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RD), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_lwa_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "h_memory_USI_c_call__AI_@cpu@_make_load_store_addr_rA_ext__SI_simm16_4", HW_H_MEMORY, CGEN_MODE_USI, 0, 0, 0 },
  { INPUT, "rA", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RA), 0, 0 },
  { INPUT, "simm16", HW_H_SIMM16, CGEN_MODE_INT, OP_ENT (SIMM16), 0, 0 },
  { OUTPUT, "atomic_address", HW_H_ATOMIC_ADDRESS, CGEN_MODE_SI, 0, 0, 0 },
  { OUTPUT, "atomic_reserve", HW_H_ATOMIC_RESERVE, CGEN_MODE_BI, 0, 0, 0 },
  { OUTPUT, "rD", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RD), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_lbz_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "h_memory_UQI_c_call__AI_@cpu@_make_load_store_addr_rA_ext__SI_simm16_1", HW_H_MEMORY, CGEN_MODE_UQI, 0, 0, 0 },
  { INPUT, "rA", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RA), 0, 0 },
  { INPUT, "simm16", HW_H_SIMM16, CGEN_MODE_INT, OP_ENT (SIMM16), 0, 0 },
  { OUTPUT, "rD", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RD), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_lbs_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "h_memory_QI_c_call__AI_@cpu@_make_load_store_addr_rA_ext__SI_simm16_1", HW_H_MEMORY, CGEN_MODE_QI, 0, 0, 0 },
  { INPUT, "rA", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RA), 0, 0 },
  { INPUT, "simm16", HW_H_SIMM16, CGEN_MODE_INT, OP_ENT (SIMM16), 0, 0 },
  { OUTPUT, "rD", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RD), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_lhz_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "h_memory_UHI_c_call__AI_@cpu@_make_load_store_addr_rA_ext__SI_simm16_2", HW_H_MEMORY, CGEN_MODE_UHI, 0, 0, 0 },
  { INPUT, "rA", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RA), 0, 0 },
  { INPUT, "simm16", HW_H_SIMM16, CGEN_MODE_INT, OP_ENT (SIMM16), 0, 0 },
  { OUTPUT, "rD", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RD), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_lhs_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "h_memory_HI_c_call__AI_@cpu@_make_load_store_addr_rA_ext__SI_simm16_2", HW_H_MEMORY, CGEN_MODE_HI, 0, 0, 0 },
  { INPUT, "rA", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RA), 0, 0 },
  { INPUT, "simm16", HW_H_SIMM16, CGEN_MODE_INT, OP_ENT (SIMM16), 0, 0 },
  { OUTPUT, "rD", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RD), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_sw_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "atomic_address", HW_H_ATOMIC_ADDRESS, CGEN_MODE_SI, 0, 0, 0 },
  { INPUT, "rA", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RA), 0, 0 },
  { INPUT, "rB", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RB), 0, 0 },
  { INPUT, "simm16_split", HW_H_SIMM16, CGEN_MODE_INT, OP_ENT (SIMM16_SPLIT), 0, 0 },
  { OUTPUT, "atomic_reserve", HW_H_ATOMIC_RESERVE, CGEN_MODE_BI, 0, 0, COND_REF },
  { OUTPUT, "h_memory_USI_addr", HW_H_MEMORY, CGEN_MODE_USI, 0, 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_sb_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "atomic_address", HW_H_ATOMIC_ADDRESS, CGEN_MODE_SI, 0, 0, 0 },
  { INPUT, "rA", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RA), 0, 0 },
  { INPUT, "rB", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RB), 0, 0 },
  { INPUT, "simm16_split", HW_H_SIMM16, CGEN_MODE_INT, OP_ENT (SIMM16_SPLIT), 0, 0 },
  { OUTPUT, "atomic_reserve", HW_H_ATOMIC_RESERVE, CGEN_MODE_BI, 0, 0, COND_REF },
  { OUTPUT, "h_memory_UQI_addr", HW_H_MEMORY, CGEN_MODE_UQI, 0, 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_sh_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "atomic_address", HW_H_ATOMIC_ADDRESS, CGEN_MODE_SI, 0, 0, 0 },
  { INPUT, "rA", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RA), 0, 0 },
  { INPUT, "rB", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RB), 0, 0 },
  { INPUT, "simm16_split", HW_H_SIMM16, CGEN_MODE_INT, OP_ENT (SIMM16_SPLIT), 0, 0 },
  { OUTPUT, "atomic_reserve", HW_H_ATOMIC_RESERVE, CGEN_MODE_BI, 0, 0, COND_REF },
  { OUTPUT, "h_memory_UHI_addr", HW_H_MEMORY, CGEN_MODE_UHI, 0, 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_swa_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "atomic_address", HW_H_ATOMIC_ADDRESS, CGEN_MODE_SI, 0, 0, 0 },
  { INPUT, "atomic_reserve", HW_H_ATOMIC_RESERVE, CGEN_MODE_BI, 0, 0, 0 },
  { INPUT, "rA", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RA), 0, 0 },
  { INPUT, "rB", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RB), 0, COND_REF },
  { INPUT, "simm16_split", HW_H_SIMM16, CGEN_MODE_INT, OP_ENT (SIMM16_SPLIT), 0, 0 },
  { INPUT, "sys_sr_f", HW_H_SYS_SR_F, CGEN_MODE_USI, 0, 0, 0 },
  { OUTPUT, "atomic_reserve", HW_H_ATOMIC_RESERVE, CGEN_MODE_BI, 0, 0, 0 },
  { OUTPUT, "h_memory_USI_addr", HW_H_MEMORY, CGEN_MODE_USI, 0, 0, COND_REF },
  { OUTPUT, "sys_sr_f", HW_H_SYS_SR_F, CGEN_MODE_USI, 0, 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_sll_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "rA", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RA), 0, 0 },
  { INPUT, "rB", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RB), 0, 0 },
  { OUTPUT, "rD", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RD), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_slli_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "rA", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RA), 0, 0 },
  { INPUT, "uimm6", HW_H_UIMM6, CGEN_MODE_UINT, OP_ENT (UIMM6), 0, 0 },
  { OUTPUT, "rD", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RD), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_and_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "rA", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RA), 0, 0 },
  { INPUT, "rB", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RB), 0, 0 },
  { OUTPUT, "rD", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RD), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_add_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "pc", HW_H_PC, CGEN_MODE_USI, 0, 0, COND_REF },
  { INPUT, "rA", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RA), 0, 0 },
  { INPUT, "rB", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RB), 0, 0 },
  { INPUT, "sys_sr_ov", HW_H_SYS_SR_OV, CGEN_MODE_USI, 0, 0, 0 },
  { INPUT, "sys_sr_ove", HW_H_SYS_SR_OVE, CGEN_MODE_USI, 0, 0, 0 },
  { OUTPUT, "rD", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RD), 0, 0 },
  { OUTPUT, "sys_sr_cy", HW_H_SYS_SR_CY, CGEN_MODE_USI, 0, 0, 0 },
  { OUTPUT, "sys_sr_ov", HW_H_SYS_SR_OV, CGEN_MODE_USI, 0, 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_addc_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "pc", HW_H_PC, CGEN_MODE_USI, 0, 0, COND_REF },
  { INPUT, "rA", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RA), 0, 0 },
  { INPUT, "rB", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RB), 0, 0 },
  { INPUT, "sys_sr_cy", HW_H_SYS_SR_CY, CGEN_MODE_USI, 0, 0, 0 },
  { INPUT, "sys_sr_ov", HW_H_SYS_SR_OV, CGEN_MODE_USI, 0, 0, 0 },
  { INPUT, "sys_sr_ove", HW_H_SYS_SR_OVE, CGEN_MODE_USI, 0, 0, 0 },
  { OUTPUT, "rD", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RD), 0, 0 },
  { OUTPUT, "sys_sr_cy", HW_H_SYS_SR_CY, CGEN_MODE_USI, 0, 0, 0 },
  { OUTPUT, "sys_sr_ov", HW_H_SYS_SR_OV, CGEN_MODE_USI, 0, 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_mul_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "pc", HW_H_PC, CGEN_MODE_USI, 0, 0, COND_REF },
  { INPUT, "rA", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RA), 0, 0 },
  { INPUT, "rB", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RB), 0, 0 },
  { INPUT, "sys_sr_ov", HW_H_SYS_SR_OV, CGEN_MODE_USI, 0, 0, 0 },
  { INPUT, "sys_sr_ove", HW_H_SYS_SR_OVE, CGEN_MODE_USI, 0, 0, 0 },
  { OUTPUT, "rD", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RD), 0, 0 },
  { OUTPUT, "sys_sr_ov", HW_H_SYS_SR_OV, CGEN_MODE_USI, 0, 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_muld_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "rA", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RA), 0, 0 },
  { INPUT, "rB", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RB), 0, 0 },
  { OUTPUT, "mac_machi", HW_H_MAC_MACHI, CGEN_MODE_USI, 0, 0, 0 },
  { OUTPUT, "mac_maclo", HW_H_MAC_MACLO, CGEN_MODE_USI, 0, 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_mulu_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "pc", HW_H_PC, CGEN_MODE_USI, 0, 0, COND_REF },
  { INPUT, "rA", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RA), 0, 0 },
  { INPUT, "rB", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RB), 0, 0 },
  { INPUT, "sys_sr_cy", HW_H_SYS_SR_CY, CGEN_MODE_USI, 0, 0, 0 },
  { INPUT, "sys_sr_ove", HW_H_SYS_SR_OVE, CGEN_MODE_USI, 0, 0, 0 },
  { OUTPUT, "rD", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RD), 0, 0 },
  { OUTPUT, "sys_sr_cy", HW_H_SYS_SR_CY, CGEN_MODE_USI, 0, 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_div_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "pc", HW_H_PC, CGEN_MODE_USI, 0, 0, COND_REF },
  { INPUT, "rA", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RA), 0, COND_REF },
  { INPUT, "rB", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RB), 0, 0 },
  { INPUT, "sys_sr_ove", HW_H_SYS_SR_OVE, CGEN_MODE_USI, 0, 0, COND_REF },
  { OUTPUT, "rD", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RD), 0, COND_REF },
  { OUTPUT, "sys_sr_ov", HW_H_SYS_SR_OV, CGEN_MODE_USI, 0, 0, COND_REF },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_divu_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "pc", HW_H_PC, CGEN_MODE_USI, 0, 0, COND_REF },
  { INPUT, "rA", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RA), 0, COND_REF },
  { INPUT, "rB", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RB), 0, 0 },
  { INPUT, "sys_sr_ove", HW_H_SYS_SR_OVE, CGEN_MODE_USI, 0, 0, COND_REF },
  { OUTPUT, "rD", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RD), 0, COND_REF },
  { OUTPUT, "sys_sr_cy", HW_H_SYS_SR_CY, CGEN_MODE_USI, 0, 0, COND_REF },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_ff1_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "rA", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RA), 0, 0 },
  { OUTPUT, "rD", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RD), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_xori_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "rA", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RA), 0, 0 },
  { INPUT, "simm16", HW_H_SIMM16, CGEN_MODE_INT, OP_ENT (SIMM16), 0, 0 },
  { OUTPUT, "rD", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RD), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_addi_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "pc", HW_H_PC, CGEN_MODE_USI, 0, 0, COND_REF },
  { INPUT, "rA", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RA), 0, 0 },
  { INPUT, "simm16", HW_H_SIMM16, CGEN_MODE_INT, OP_ENT (SIMM16), 0, 0 },
  { INPUT, "sys_sr_ov", HW_H_SYS_SR_OV, CGEN_MODE_USI, 0, 0, 0 },
  { INPUT, "sys_sr_ove", HW_H_SYS_SR_OVE, CGEN_MODE_USI, 0, 0, 0 },
  { OUTPUT, "rD", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RD), 0, 0 },
  { OUTPUT, "sys_sr_cy", HW_H_SYS_SR_CY, CGEN_MODE_USI, 0, 0, 0 },
  { OUTPUT, "sys_sr_ov", HW_H_SYS_SR_OV, CGEN_MODE_USI, 0, 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_addic_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "pc", HW_H_PC, CGEN_MODE_USI, 0, 0, COND_REF },
  { INPUT, "rA", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RA), 0, 0 },
  { INPUT, "simm16", HW_H_SIMM16, CGEN_MODE_INT, OP_ENT (SIMM16), 0, 0 },
  { INPUT, "sys_sr_cy", HW_H_SYS_SR_CY, CGEN_MODE_USI, 0, 0, 0 },
  { INPUT, "sys_sr_ov", HW_H_SYS_SR_OV, CGEN_MODE_USI, 0, 0, 0 },
  { INPUT, "sys_sr_ove", HW_H_SYS_SR_OVE, CGEN_MODE_USI, 0, 0, 0 },
  { OUTPUT, "rD", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RD), 0, 0 },
  { OUTPUT, "sys_sr_cy", HW_H_SYS_SR_CY, CGEN_MODE_USI, 0, 0, 0 },
  { OUTPUT, "sys_sr_ov", HW_H_SYS_SR_OV, CGEN_MODE_USI, 0, 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_muli_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "pc", HW_H_PC, CGEN_MODE_USI, 0, 0, COND_REF },
  { INPUT, "rA", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RA), 0, 0 },
  { INPUT, "simm16", HW_H_SIMM16, CGEN_MODE_INT, OP_ENT (SIMM16), 0, 0 },
  { INPUT, "sys_sr_ov", HW_H_SYS_SR_OV, CGEN_MODE_USI, 0, 0, 0 },
  { INPUT, "sys_sr_ove", HW_H_SYS_SR_OVE, CGEN_MODE_USI, 0, 0, 0 },
  { OUTPUT, "rD", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RD), 0, 0 },
  { OUTPUT, "sys_sr_ov", HW_H_SYS_SR_OV, CGEN_MODE_USI, 0, 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_exths_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "rA", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RA), 0, 0 },
  { OUTPUT, "rD", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RD), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_cmov_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "rA", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RA), 0, COND_REF },
  { INPUT, "rB", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RB), 0, COND_REF },
  { INPUT, "sys_sr_f", HW_H_SYS_SR_F, CGEN_MODE_USI, 0, 0, 0 },
  { OUTPUT, "rD", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RD), 0, COND_REF },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_sfgts_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "rA", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RA), 0, 0 },
  { INPUT, "rB", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RB), 0, 0 },
  { OUTPUT, "sys_sr_f", HW_H_SYS_SR_F, CGEN_MODE_USI, 0, 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_sfgtsi_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "rA", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RA), 0, 0 },
  { INPUT, "simm16", HW_H_SIMM16, CGEN_MODE_INT, OP_ENT (SIMM16), 0, 0 },
  { OUTPUT, "sys_sr_f", HW_H_SYS_SR_F, CGEN_MODE_USI, 0, 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_mac_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "mac_machi", HW_H_MAC_MACHI, CGEN_MODE_USI, 0, 0, 0 },
  { INPUT, "mac_maclo", HW_H_MAC_MACLO, CGEN_MODE_USI, 0, 0, 0 },
  { INPUT, "pc", HW_H_PC, CGEN_MODE_USI, 0, 0, COND_REF },
  { INPUT, "rA", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RA), 0, 0 },
  { INPUT, "rB", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RB), 0, 0 },
  { INPUT, "sys_sr_ov", HW_H_SYS_SR_OV, CGEN_MODE_USI, 0, 0, 0 },
  { INPUT, "sys_sr_ove", HW_H_SYS_SR_OVE, CGEN_MODE_USI, 0, 0, 0 },
  { OUTPUT, "mac_machi", HW_H_MAC_MACHI, CGEN_MODE_USI, 0, 0, 0 },
  { OUTPUT, "mac_maclo", HW_H_MAC_MACLO, CGEN_MODE_USI, 0, 0, 0 },
  { OUTPUT, "sys_sr_ov", HW_H_SYS_SR_OV, CGEN_MODE_USI, 0, 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_maci_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "mac_machi", HW_H_MAC_MACHI, CGEN_MODE_USI, 0, 0, 0 },
  { INPUT, "mac_maclo", HW_H_MAC_MACLO, CGEN_MODE_USI, 0, 0, 0 },
  { INPUT, "pc", HW_H_PC, CGEN_MODE_USI, 0, 0, COND_REF },
  { INPUT, "rA", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RA), 0, 0 },
  { INPUT, "simm16", HW_H_SIMM16, CGEN_MODE_INT, OP_ENT (SIMM16), 0, 0 },
  { INPUT, "sys_sr_ov", HW_H_SYS_SR_OV, CGEN_MODE_USI, 0, 0, 0 },
  { INPUT, "sys_sr_ove", HW_H_SYS_SR_OVE, CGEN_MODE_USI, 0, 0, 0 },
  { OUTPUT, "mac_machi", HW_H_MAC_MACHI, CGEN_MODE_USI, 0, 0, 0 },
  { OUTPUT, "mac_maclo", HW_H_MAC_MACLO, CGEN_MODE_USI, 0, 0, 0 },
  { OUTPUT, "sys_sr_ov", HW_H_SYS_SR_OV, CGEN_MODE_USI, 0, 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_l_macu_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "mac_machi", HW_H_MAC_MACHI, CGEN_MODE_USI, 0, 0, 0 },
  { INPUT, "mac_maclo", HW_H_MAC_MACLO, CGEN_MODE_USI, 0, 0, 0 },
  { INPUT, "pc", HW_H_PC, CGEN_MODE_USI, 0, 0, COND_REF },
  { INPUT, "rA", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RA), 0, 0 },
  { INPUT, "rB", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RB), 0, 0 },
  { INPUT, "sys_sr_cy", HW_H_SYS_SR_CY, CGEN_MODE_USI, 0, 0, 0 },
  { INPUT, "sys_sr_ove", HW_H_SYS_SR_OVE, CGEN_MODE_USI, 0, 0, 0 },
  { OUTPUT, "mac_machi", HW_H_MAC_MACHI, CGEN_MODE_USI, 0, 0, 0 },
  { OUTPUT, "mac_maclo", HW_H_MAC_MACLO, CGEN_MODE_USI, 0, 0, 0 },
  { OUTPUT, "sys_sr_cy", HW_H_SYS_SR_CY, CGEN_MODE_USI, 0, 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_lf_add_s_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "rASF", HW_H_FSR, CGEN_MODE_SF, OP_ENT (RASF), 0, 0 },
  { INPUT, "rBSF", HW_H_FSR, CGEN_MODE_SF, OP_ENT (RBSF), 0, 0 },
  { OUTPUT, "rDSF", HW_H_FSR, CGEN_MODE_SF, OP_ENT (RDSF), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_lf_add_d32_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "rAD32F", HW_H_FD32R, CGEN_MODE_DF, OP_ENT (RAD32F), 0, 0 },
  { INPUT, "rBD32F", HW_H_FD32R, CGEN_MODE_DF, OP_ENT (RBD32F), 0, 0 },
  { OUTPUT, "rDD32F", HW_H_FD32R, CGEN_MODE_DF, OP_ENT (RDD32F), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_lf_itof_s_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "rA", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RA), 0, 0 },
  { INPUT, "sys_fpcsr_rm", HW_H_SYS_FPCSR_RM, CGEN_MODE_USI, 0, 0, 0 },
  { OUTPUT, "rDSF", HW_H_FSR, CGEN_MODE_SF, OP_ENT (RDSF), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_lf_itof_d32_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "rADI", HW_H_I64R, CGEN_MODE_DI, OP_ENT (RADI), 0, 0 },
  { INPUT, "sys_fpcsr_rm", HW_H_SYS_FPCSR_RM, CGEN_MODE_USI, 0, 0, 0 },
  { OUTPUT, "rDD32F", HW_H_FD32R, CGEN_MODE_DF, OP_ENT (RDD32F), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_lf_ftoi_s_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "rASF", HW_H_FSR, CGEN_MODE_SF, OP_ENT (RASF), 0, 0 },
  { INPUT, "sys_fpcsr_rm", HW_H_SYS_FPCSR_RM, CGEN_MODE_USI, 0, 0, 0 },
  { OUTPUT, "rD", HW_H_GPR, CGEN_MODE_USI, OP_ENT (RD), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_lf_ftoi_d32_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "rAD32F", HW_H_FD32R, CGEN_MODE_DF, OP_ENT (RAD32F), 0, 0 },
  { INPUT, "sys_fpcsr_rm", HW_H_SYS_FPCSR_RM, CGEN_MODE_USI, 0, 0, 0 },
  { OUTPUT, "rDDI", HW_H_I64R, CGEN_MODE_DI, OP_ENT (RDDI), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_lf_sfeq_s_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "rASF", HW_H_FSR, CGEN_MODE_SF, OP_ENT (RASF), 0, 0 },
  { INPUT, "rBSF", HW_H_FSR, CGEN_MODE_SF, OP_ENT (RBSF), 0, 0 },
  { OUTPUT, "sys_sr_f", HW_H_SYS_SR_F, CGEN_MODE_USI, 0, 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_lf_sfeq_d32_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "rAD32F", HW_H_FD32R, CGEN_MODE_DF, OP_ENT (RAD32F), 0, 0 },
  { INPUT, "rBD32F", HW_H_FD32R, CGEN_MODE_DF, OP_ENT (RBD32F), 0, 0 },
  { OUTPUT, "sys_sr_f", HW_H_SYS_SR_F, CGEN_MODE_USI, 0, 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_lf_madd_s_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "rASF", HW_H_FSR, CGEN_MODE_SF, OP_ENT (RASF), 0, 0 },
  { INPUT, "rBSF", HW_H_FSR, CGEN_MODE_SF, OP_ENT (RBSF), 0, 0 },
  { INPUT, "rDSF", HW_H_FSR, CGEN_MODE_SF, OP_ENT (RDSF), 0, 0 },
  { OUTPUT, "rDSF", HW_H_FSR, CGEN_MODE_SF, OP_ENT (RDSF), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_lf_madd_d32_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "rAD32F", HW_H_FD32R, CGEN_MODE_DF, OP_ENT (RAD32F), 0, 0 },
  { INPUT, "rBD32F", HW_H_FD32R, CGEN_MODE_DF, OP_ENT (RBD32F), 0, 0 },
  { INPUT, "rDD32F", HW_H_FD32R, CGEN_MODE_DF, OP_ENT (RDD32F), 0, 0 },
  { OUTPUT, "rDD32F", HW_H_FD32R, CGEN_MODE_DF, OP_ENT (RDD32F), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

#undef OP_ENT
#undef INPUT
#undef OUTPUT
#undef END
#undef COND_REF

/* Operand instance lookup table.  */

static const CGEN_OPINST *or1k_cgen_opinst_table[MAX_INSNS] = {
  0,
  & sfmt_l_j_ops[0],
  & sfmt_l_adrp_ops[0],
  & sfmt_l_jal_ops[0],
  & sfmt_l_jr_ops[0],
  & sfmt_l_jalr_ops[0],
  & sfmt_l_bnf_ops[0],
  & sfmt_l_bnf_ops[0],
  & sfmt_l_trap_ops[0],
  & sfmt_l_trap_ops[0],
  & sfmt_l_msync_ops[0],
  & sfmt_l_msync_ops[0],
  & sfmt_l_msync_ops[0],
  & sfmt_l_msync_ops[0],
  & sfmt_l_nop_imm_ops[0],
  & sfmt_l_msync_ops[0],
  & sfmt_l_movhi_ops[0],
  & sfmt_l_macrc_ops[0],
  & sfmt_l_mfspr_ops[0],
  & sfmt_l_mtspr_ops[0],
  & sfmt_l_lwz_ops[0],
  & sfmt_l_lws_ops[0],
  & sfmt_l_lwa_ops[0],
  & sfmt_l_lbz_ops[0],
  & sfmt_l_lbs_ops[0],
  & sfmt_l_lhz_ops[0],
  & sfmt_l_lhs_ops[0],
  & sfmt_l_sw_ops[0],
  & sfmt_l_sb_ops[0],
  & sfmt_l_sh_ops[0],
  & sfmt_l_swa_ops[0],
  & sfmt_l_sll_ops[0],
  & sfmt_l_slli_ops[0],
  & sfmt_l_sll_ops[0],
  & sfmt_l_slli_ops[0],
  & sfmt_l_sll_ops[0],
  & sfmt_l_slli_ops[0],
  & sfmt_l_sll_ops[0],
  & sfmt_l_slli_ops[0],
  & sfmt_l_and_ops[0],
  & sfmt_l_and_ops[0],
  & sfmt_l_and_ops[0],
  & sfmt_l_add_ops[0],
  & sfmt_l_add_ops[0],
  & sfmt_l_addc_ops[0],
  & sfmt_l_mul_ops[0],
  & sfmt_l_muld_ops[0],
  & sfmt_l_mulu_ops[0],
  & sfmt_l_muld_ops[0],
  & sfmt_l_div_ops[0],
  & sfmt_l_divu_ops[0],
  & sfmt_l_ff1_ops[0],
  & sfmt_l_ff1_ops[0],
  & sfmt_l_mfspr_ops[0],
  & sfmt_l_mfspr_ops[0],
  & sfmt_l_xori_ops[0],
  & sfmt_l_addi_ops[0],
  & sfmt_l_addic_ops[0],
  & sfmt_l_muli_ops[0],
  & sfmt_l_exths_ops[0],
  & sfmt_l_exths_ops[0],
  & sfmt_l_exths_ops[0],
  & sfmt_l_exths_ops[0],
  & sfmt_l_exths_ops[0],
  & sfmt_l_exths_ops[0],
  & sfmt_l_cmov_ops[0],
  & sfmt_l_sfgts_ops[0],
  & sfmt_l_sfgtsi_ops[0],
  & sfmt_l_sfgts_ops[0],
  & sfmt_l_sfgtsi_ops[0],
  & sfmt_l_sfgts_ops[0],
  & sfmt_l_sfgtsi_ops[0],
  & sfmt_l_sfgts_ops[0],
  & sfmt_l_sfgtsi_ops[0],
  & sfmt_l_sfgts_ops[0],
  & sfmt_l_sfgtsi_ops[0],
  & sfmt_l_sfgts_ops[0],
  & sfmt_l_sfgtsi_ops[0],
  & sfmt_l_sfgts_ops[0],
  & sfmt_l_sfgtsi_ops[0],
  & sfmt_l_sfgts_ops[0],
  & sfmt_l_sfgtsi_ops[0],
  & sfmt_l_sfgts_ops[0],
  & sfmt_l_sfgtsi_ops[0],
  & sfmt_l_sfgts_ops[0],
  & sfmt_l_sfgtsi_ops[0],
  & sfmt_l_mac_ops[0],
  & sfmt_l_maci_ops[0],
  & sfmt_l_macu_ops[0],
  & sfmt_l_mac_ops[0],
  & sfmt_l_macu_ops[0],
  & sfmt_l_msync_ops[0],
  & sfmt_l_msync_ops[0],
  & sfmt_l_msync_ops[0],
  & sfmt_l_msync_ops[0],
  & sfmt_l_msync_ops[0],
  & sfmt_l_msync_ops[0],
  & sfmt_l_msync_ops[0],
  & sfmt_l_msync_ops[0],
  & sfmt_lf_add_s_ops[0],
  & sfmt_lf_add_d32_ops[0],
  & sfmt_lf_add_s_ops[0],
  & sfmt_lf_add_d32_ops[0],
  & sfmt_lf_add_s_ops[0],
  & sfmt_lf_add_d32_ops[0],
  & sfmt_lf_add_s_ops[0],
  & sfmt_lf_add_d32_ops[0],
  & sfmt_lf_add_s_ops[0],
  & sfmt_lf_add_d32_ops[0],
  & sfmt_lf_itof_s_ops[0],
  & sfmt_lf_itof_d32_ops[0],
  & sfmt_lf_ftoi_s_ops[0],
  & sfmt_lf_ftoi_d32_ops[0],
  & sfmt_lf_sfeq_s_ops[0],
  & sfmt_lf_sfeq_d32_ops[0],
  & sfmt_lf_sfeq_s_ops[0],
  & sfmt_lf_sfeq_d32_ops[0],
  & sfmt_lf_sfeq_s_ops[0],
  & sfmt_lf_sfeq_d32_ops[0],
  & sfmt_lf_sfeq_s_ops[0],
  & sfmt_lf_sfeq_d32_ops[0],
  & sfmt_lf_sfeq_s_ops[0],
  & sfmt_lf_sfeq_d32_ops[0],
  & sfmt_lf_sfeq_s_ops[0],
  & sfmt_lf_sfeq_d32_ops[0],
  & sfmt_lf_sfeq_s_ops[0],
  & sfmt_lf_sfeq_d32_ops[0],
  & sfmt_lf_sfeq_s_ops[0],
  & sfmt_lf_sfeq_d32_ops[0],
  & sfmt_lf_sfeq_s_ops[0],
  & sfmt_lf_sfeq_d32_ops[0],
  & sfmt_lf_sfeq_s_ops[0],
  & sfmt_lf_sfeq_d32_ops[0],
  & sfmt_lf_sfeq_s_ops[0],
  & sfmt_lf_sfeq_d32_ops[0],
  & sfmt_lf_sfeq_s_ops[0],
  & sfmt_lf_sfeq_d32_ops[0],
  & sfmt_lf_sfeq_s_ops[0],
  & sfmt_lf_sfeq_d32_ops[0],
  & sfmt_lf_madd_s_ops[0],
  & sfmt_lf_madd_d32_ops[0],
  & sfmt_l_msync_ops[0],
  & sfmt_l_msync_ops[0],
};

/* Function to call before using the operand instance table.  */

void
or1k_cgen_init_opinst_table (CGEN_CPU_DESC cd)
{
  int i;
  const CGEN_OPINST **oi = & or1k_cgen_opinst_table[0];
  CGEN_INSN *insns = (CGEN_INSN *) cd->insn_table.init_entries;
  for (i = 0; i < MAX_INSNS; ++i)
    insns[i].opinst = oi[i];
}
