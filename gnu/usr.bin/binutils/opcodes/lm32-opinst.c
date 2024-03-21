/* DO NOT EDIT!  -*- buffer-read-only: t -*- vi:set ro:  */
/* Semantic operand instances for lm32.

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
#include "lm32-desc.h"
#include "lm32-opc.h"

/* Operand references.  */

#define OP_ENT(op) LM32_OPERAND_##op
#define INPUT CGEN_OPINST_INPUT
#define OUTPUT CGEN_OPINST_OUTPUT
#define END CGEN_OPINST_END
#define COND_REF CGEN_OPINST_COND_REF

static const CGEN_OPINST sfmt_empty_ops[] ATTRIBUTE_UNUSED = {
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_add_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "r0", HW_H_GR, CGEN_MODE_SI, OP_ENT (R0), 0, 0 },
  { INPUT, "r1", HW_H_GR, CGEN_MODE_SI, OP_ENT (R1), 0, 0 },
  { OUTPUT, "r2", HW_H_GR, CGEN_MODE_SI, OP_ENT (R2), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_addi_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "imm", HW_H_SINT, CGEN_MODE_INT, OP_ENT (IMM), 0, 0 },
  { INPUT, "r0", HW_H_GR, CGEN_MODE_SI, OP_ENT (R0), 0, 0 },
  { OUTPUT, "r1", HW_H_GR, CGEN_MODE_SI, OP_ENT (R1), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_andi_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "r0", HW_H_GR, CGEN_MODE_SI, OP_ENT (R0), 0, 0 },
  { INPUT, "uimm", HW_H_UINT, CGEN_MODE_UINT, OP_ENT (UIMM), 0, 0 },
  { OUTPUT, "r1", HW_H_GR, CGEN_MODE_SI, OP_ENT (R1), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_andhii_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "hi16", HW_H_UINT, CGEN_MODE_UINT, OP_ENT (HI16), 0, 0 },
  { INPUT, "r0", HW_H_GR, CGEN_MODE_SI, OP_ENT (R0), 0, 0 },
  { OUTPUT, "r1", HW_H_GR, CGEN_MODE_SI, OP_ENT (R1), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_b_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "f_r0", HW_H_UINT, CGEN_MODE_UINT, 0, 0, 0 },
  { INPUT, "r0", HW_H_GR, CGEN_MODE_SI, OP_ENT (R0), 0, 0 },
  { OUTPUT, "pc", HW_H_PC, CGEN_MODE_USI, 0, 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_bi_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "call", HW_H_IADDR, CGEN_MODE_USI, OP_ENT (CALL), 0, 0 },
  { OUTPUT, "pc", HW_H_PC, CGEN_MODE_USI, 0, 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_be_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "branch", HW_H_IADDR, CGEN_MODE_USI, OP_ENT (BRANCH), 0, COND_REF },
  { INPUT, "r0", HW_H_GR, CGEN_MODE_SI, OP_ENT (R0), 0, 0 },
  { INPUT, "r1", HW_H_GR, CGEN_MODE_SI, OP_ENT (R1), 0, 0 },
  { OUTPUT, "pc", HW_H_PC, CGEN_MODE_USI, 0, 0, COND_REF },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_call_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "pc", HW_H_PC, CGEN_MODE_USI, 0, 0, 0 },
  { INPUT, "r0", HW_H_GR, CGEN_MODE_SI, OP_ENT (R0), 0, 0 },
  { OUTPUT, "h_gr_SI_29", HW_H_GR, CGEN_MODE_SI, 0, 29, 0 },
  { OUTPUT, "pc", HW_H_PC, CGEN_MODE_USI, 0, 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_calli_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "call", HW_H_IADDR, CGEN_MODE_USI, OP_ENT (CALL), 0, 0 },
  { INPUT, "pc", HW_H_PC, CGEN_MODE_USI, 0, 0, 0 },
  { OUTPUT, "h_gr_SI_29", HW_H_GR, CGEN_MODE_SI, 0, 29, 0 },
  { OUTPUT, "pc", HW_H_PC, CGEN_MODE_USI, 0, 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_divu_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "f_r0", HW_H_UINT, CGEN_MODE_UINT, 0, 0, 0 },
  { INPUT, "f_r1", HW_H_UINT, CGEN_MODE_UINT, 0, 0, 0 },
  { INPUT, "f_r2", HW_H_UINT, CGEN_MODE_UINT, 0, 0, 0 },
  { INPUT, "pc", HW_H_PC, CGEN_MODE_USI, 0, 0, 0 },
  { OUTPUT, "pc", HW_H_PC, CGEN_MODE_USI, 0, 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_lb_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "h_memory_QI_add__SI_r0_ext__SI_trunc__HI_imm", HW_H_MEMORY, CGEN_MODE_QI, 0, 0, 0 },
  { INPUT, "imm", HW_H_SINT, CGEN_MODE_INT, OP_ENT (IMM), 0, 0 },
  { INPUT, "r0", HW_H_GR, CGEN_MODE_SI, OP_ENT (R0), 0, 0 },
  { OUTPUT, "r1", HW_H_GR, CGEN_MODE_SI, OP_ENT (R1), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_lh_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "h_memory_HI_add__SI_r0_ext__SI_trunc__HI_imm", HW_H_MEMORY, CGEN_MODE_HI, 0, 0, 0 },
  { INPUT, "imm", HW_H_SINT, CGEN_MODE_INT, OP_ENT (IMM), 0, 0 },
  { INPUT, "r0", HW_H_GR, CGEN_MODE_SI, OP_ENT (R0), 0, 0 },
  { OUTPUT, "r1", HW_H_GR, CGEN_MODE_SI, OP_ENT (R1), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_lw_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "h_memory_SI_add__SI_r0_ext__SI_trunc__HI_imm", HW_H_MEMORY, CGEN_MODE_SI, 0, 0, 0 },
  { INPUT, "imm", HW_H_SINT, CGEN_MODE_INT, OP_ENT (IMM), 0, 0 },
  { INPUT, "r0", HW_H_GR, CGEN_MODE_SI, OP_ENT (R0), 0, 0 },
  { OUTPUT, "r1", HW_H_GR, CGEN_MODE_SI, OP_ENT (R1), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_ori_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "lo16", HW_H_UINT, CGEN_MODE_UINT, OP_ENT (LO16), 0, 0 },
  { INPUT, "r0", HW_H_GR, CGEN_MODE_SI, OP_ENT (R0), 0, 0 },
  { OUTPUT, "r1", HW_H_GR, CGEN_MODE_SI, OP_ENT (R1), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_rcsr_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "csr", HW_H_CSR, CGEN_MODE_SI, OP_ENT (CSR), 0, 0 },
  { OUTPUT, "r2", HW_H_GR, CGEN_MODE_SI, OP_ENT (R2), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_sb_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "imm", HW_H_SINT, CGEN_MODE_INT, OP_ENT (IMM), 0, 0 },
  { INPUT, "r0", HW_H_GR, CGEN_MODE_SI, OP_ENT (R0), 0, 0 },
  { INPUT, "r1", HW_H_GR, CGEN_MODE_SI, OP_ENT (R1), 0, 0 },
  { OUTPUT, "h_memory_QI_add__SI_r0_ext__SI_trunc__HI_imm", HW_H_MEMORY, CGEN_MODE_QI, 0, 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_sextb_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "r0", HW_H_GR, CGEN_MODE_SI, OP_ENT (R0), 0, 0 },
  { OUTPUT, "r2", HW_H_GR, CGEN_MODE_SI, OP_ENT (R2), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_sh_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "imm", HW_H_SINT, CGEN_MODE_INT, OP_ENT (IMM), 0, 0 },
  { INPUT, "r0", HW_H_GR, CGEN_MODE_SI, OP_ENT (R0), 0, 0 },
  { INPUT, "r1", HW_H_GR, CGEN_MODE_SI, OP_ENT (R1), 0, 0 },
  { OUTPUT, "h_memory_HI_add__SI_r0_ext__SI_trunc__HI_imm", HW_H_MEMORY, CGEN_MODE_HI, 0, 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_sw_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "imm", HW_H_SINT, CGEN_MODE_INT, OP_ENT (IMM), 0, 0 },
  { INPUT, "r0", HW_H_GR, CGEN_MODE_SI, OP_ENT (R0), 0, 0 },
  { INPUT, "r1", HW_H_GR, CGEN_MODE_SI, OP_ENT (R1), 0, 0 },
  { OUTPUT, "h_memory_SI_add__SI_r0_ext__SI_trunc__HI_imm", HW_H_MEMORY, CGEN_MODE_SI, 0, 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_user_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "r0", HW_H_GR, CGEN_MODE_SI, OP_ENT (R0), 0, 0 },
  { INPUT, "r1", HW_H_GR, CGEN_MODE_SI, OP_ENT (R1), 0, 0 },
  { INPUT, "user", HW_H_UINT, CGEN_MODE_UINT, OP_ENT (USER), 0, 0 },
  { OUTPUT, "r2", HW_H_GR, CGEN_MODE_SI, OP_ENT (R2), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_wcsr_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "f_csr", HW_H_UINT, CGEN_MODE_UINT, 0, 0, 0 },
  { INPUT, "r1", HW_H_GR, CGEN_MODE_SI, OP_ENT (R1), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_break_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "pc", HW_H_PC, CGEN_MODE_USI, 0, 0, 0 },
  { OUTPUT, "pc", HW_H_PC, CGEN_MODE_USI, 0, 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_bret_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "r0", HW_H_GR, CGEN_MODE_SI, OP_ENT (R0), 0, 0 },
  { OUTPUT, "pc", HW_H_PC, CGEN_MODE_USI, 0, 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_mvi_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "imm", HW_H_SINT, CGEN_MODE_INT, OP_ENT (IMM), 0, 0 },
  { INPUT, "r0", HW_H_GR, CGEN_MODE_SI, OP_ENT (R0), 0, 0 },
  { OUTPUT, "r1", HW_H_GR, CGEN_MODE_SI, OP_ENT (R1), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_mvui_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "lo16", HW_H_UINT, CGEN_MODE_UINT, OP_ENT (LO16), 0, 0 },
  { OUTPUT, "r1", HW_H_GR, CGEN_MODE_SI, OP_ENT (R1), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_mvhi_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "hi16", HW_H_UINT, CGEN_MODE_UINT, OP_ENT (HI16), 0, 0 },
  { INPUT, "r0", HW_H_GR, CGEN_MODE_SI, OP_ENT (R0), 0, 0 },
  { OUTPUT, "r1", HW_H_GR, CGEN_MODE_SI, OP_ENT (R1), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_mva_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "gp16", HW_H_SINT, CGEN_MODE_INT, OP_ENT (GP16), 0, 0 },
  { INPUT, "r0", HW_H_GR, CGEN_MODE_SI, OP_ENT (R0), 0, 0 },
  { OUTPUT, "r1", HW_H_GR, CGEN_MODE_SI, OP_ENT (R1), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_nop_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "r0", HW_H_GR, CGEN_MODE_SI, OP_ENT (R0), 0, 0 },
  { OUTPUT, "r0", HW_H_GR, CGEN_MODE_SI, OP_ENT (R0), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_lbgprel_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "gp16", HW_H_SINT, CGEN_MODE_INT, OP_ENT (GP16), 0, 0 },
  { INPUT, "h_memory_QI_add__SI_r0_ext__SI_trunc__HI_gp16", HW_H_MEMORY, CGEN_MODE_QI, 0, 0, 0 },
  { INPUT, "r0", HW_H_GR, CGEN_MODE_SI, OP_ENT (R0), 0, 0 },
  { OUTPUT, "r1", HW_H_GR, CGEN_MODE_SI, OP_ENT (R1), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_lhgprel_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "gp16", HW_H_SINT, CGEN_MODE_INT, OP_ENT (GP16), 0, 0 },
  { INPUT, "h_memory_HI_add__SI_r0_ext__SI_trunc__HI_gp16", HW_H_MEMORY, CGEN_MODE_HI, 0, 0, 0 },
  { INPUT, "r0", HW_H_GR, CGEN_MODE_SI, OP_ENT (R0), 0, 0 },
  { OUTPUT, "r1", HW_H_GR, CGEN_MODE_SI, OP_ENT (R1), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_lwgprel_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "gp16", HW_H_SINT, CGEN_MODE_INT, OP_ENT (GP16), 0, 0 },
  { INPUT, "h_memory_SI_add__SI_r0_ext__SI_trunc__HI_gp16", HW_H_MEMORY, CGEN_MODE_SI, 0, 0, 0 },
  { INPUT, "r0", HW_H_GR, CGEN_MODE_SI, OP_ENT (R0), 0, 0 },
  { OUTPUT, "r1", HW_H_GR, CGEN_MODE_SI, OP_ENT (R1), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_sbgprel_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "gp16", HW_H_SINT, CGEN_MODE_INT, OP_ENT (GP16), 0, 0 },
  { INPUT, "r0", HW_H_GR, CGEN_MODE_SI, OP_ENT (R0), 0, 0 },
  { INPUT, "r1", HW_H_GR, CGEN_MODE_SI, OP_ENT (R1), 0, 0 },
  { OUTPUT, "h_memory_QI_add__SI_r0_ext__SI_trunc__HI_gp16", HW_H_MEMORY, CGEN_MODE_QI, 0, 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_shgprel_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "gp16", HW_H_SINT, CGEN_MODE_INT, OP_ENT (GP16), 0, 0 },
  { INPUT, "r0", HW_H_GR, CGEN_MODE_SI, OP_ENT (R0), 0, 0 },
  { INPUT, "r1", HW_H_GR, CGEN_MODE_SI, OP_ENT (R1), 0, 0 },
  { OUTPUT, "h_memory_HI_add__SI_r0_ext__SI_trunc__HI_gp16", HW_H_MEMORY, CGEN_MODE_HI, 0, 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_swgprel_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "gp16", HW_H_SINT, CGEN_MODE_INT, OP_ENT (GP16), 0, 0 },
  { INPUT, "r0", HW_H_GR, CGEN_MODE_SI, OP_ENT (R0), 0, 0 },
  { INPUT, "r1", HW_H_GR, CGEN_MODE_SI, OP_ENT (R1), 0, 0 },
  { OUTPUT, "h_memory_SI_add__SI_r0_ext__SI_trunc__HI_gp16", HW_H_MEMORY, CGEN_MODE_SI, 0, 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_lwgotrel_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "got16", HW_H_SINT, CGEN_MODE_INT, OP_ENT (GOT16), 0, 0 },
  { INPUT, "h_memory_SI_add__SI_r0_ext__SI_trunc__HI_got16", HW_H_MEMORY, CGEN_MODE_SI, 0, 0, 0 },
  { INPUT, "r0", HW_H_GR, CGEN_MODE_SI, OP_ENT (R0), 0, 0 },
  { OUTPUT, "r1", HW_H_GR, CGEN_MODE_SI, OP_ENT (R1), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_orhigotoffi_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "gotoffhi16", HW_H_SINT, CGEN_MODE_INT, OP_ENT (GOTOFFHI16), 0, 0 },
  { INPUT, "r0", HW_H_GR, CGEN_MODE_SI, OP_ENT (R0), 0, 0 },
  { OUTPUT, "r1", HW_H_GR, CGEN_MODE_SI, OP_ENT (R1), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_addgotoff_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "gotofflo16", HW_H_SINT, CGEN_MODE_INT, OP_ENT (GOTOFFLO16), 0, 0 },
  { INPUT, "r0", HW_H_GR, CGEN_MODE_SI, OP_ENT (R0), 0, 0 },
  { OUTPUT, "r1", HW_H_GR, CGEN_MODE_SI, OP_ENT (R1), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_swgotoff_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "gotofflo16", HW_H_SINT, CGEN_MODE_INT, OP_ENT (GOTOFFLO16), 0, 0 },
  { INPUT, "r0", HW_H_GR, CGEN_MODE_SI, OP_ENT (R0), 0, 0 },
  { INPUT, "r1", HW_H_GR, CGEN_MODE_SI, OP_ENT (R1), 0, 0 },
  { OUTPUT, "h_memory_SI_add__SI_r0_ext__SI_trunc__HI_gotofflo16", HW_H_MEMORY, CGEN_MODE_SI, 0, 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_lwgotoff_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "gotofflo16", HW_H_SINT, CGEN_MODE_INT, OP_ENT (GOTOFFLO16), 0, 0 },
  { INPUT, "h_memory_SI_add__SI_r0_ext__SI_trunc__HI_gotofflo16", HW_H_MEMORY, CGEN_MODE_SI, 0, 0, 0 },
  { INPUT, "r0", HW_H_GR, CGEN_MODE_SI, OP_ENT (R0), 0, 0 },
  { OUTPUT, "r1", HW_H_GR, CGEN_MODE_SI, OP_ENT (R1), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_shgotoff_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "gotofflo16", HW_H_SINT, CGEN_MODE_INT, OP_ENT (GOTOFFLO16), 0, 0 },
  { INPUT, "r0", HW_H_GR, CGEN_MODE_SI, OP_ENT (R0), 0, 0 },
  { INPUT, "r1", HW_H_GR, CGEN_MODE_SI, OP_ENT (R1), 0, 0 },
  { OUTPUT, "h_memory_HI_add__SI_r0_ext__SI_trunc__HI_gotofflo16", HW_H_MEMORY, CGEN_MODE_HI, 0, 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_lhgotoff_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "gotofflo16", HW_H_SINT, CGEN_MODE_INT, OP_ENT (GOTOFFLO16), 0, 0 },
  { INPUT, "h_memory_HI_add__SI_r0_ext__SI_trunc__HI_gotofflo16", HW_H_MEMORY, CGEN_MODE_HI, 0, 0, 0 },
  { INPUT, "r0", HW_H_GR, CGEN_MODE_SI, OP_ENT (R0), 0, 0 },
  { OUTPUT, "r1", HW_H_GR, CGEN_MODE_SI, OP_ENT (R1), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_sbgotoff_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "gotofflo16", HW_H_SINT, CGEN_MODE_INT, OP_ENT (GOTOFFLO16), 0, 0 },
  { INPUT, "r0", HW_H_GR, CGEN_MODE_SI, OP_ENT (R0), 0, 0 },
  { INPUT, "r1", HW_H_GR, CGEN_MODE_SI, OP_ENT (R1), 0, 0 },
  { OUTPUT, "h_memory_QI_add__SI_r0_ext__SI_trunc__HI_gotofflo16", HW_H_MEMORY, CGEN_MODE_QI, 0, 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

static const CGEN_OPINST sfmt_lbgotoff_ops[] ATTRIBUTE_UNUSED = {
  { INPUT, "gotofflo16", HW_H_SINT, CGEN_MODE_INT, OP_ENT (GOTOFFLO16), 0, 0 },
  { INPUT, "h_memory_QI_add__SI_r0_ext__SI_trunc__HI_gotofflo16", HW_H_MEMORY, CGEN_MODE_QI, 0, 0, 0 },
  { INPUT, "r0", HW_H_GR, CGEN_MODE_SI, OP_ENT (R0), 0, 0 },
  { OUTPUT, "r1", HW_H_GR, CGEN_MODE_SI, OP_ENT (R1), 0, 0 },
  { END, (const char *)0, (enum cgen_hw_type)0, (enum cgen_mode)0, (enum cgen_operand_type)0, 0, 0 }
};

#undef OP_ENT
#undef INPUT
#undef OUTPUT
#undef END
#undef COND_REF

/* Operand instance lookup table.  */

static const CGEN_OPINST *lm32_cgen_opinst_table[MAX_INSNS] = {
  0,
  & sfmt_add_ops[0],
  & sfmt_addi_ops[0],
  & sfmt_add_ops[0],
  & sfmt_andi_ops[0],
  & sfmt_andhii_ops[0],
  & sfmt_b_ops[0],
  & sfmt_bi_ops[0],
  & sfmt_be_ops[0],
  & sfmt_be_ops[0],
  & sfmt_be_ops[0],
  & sfmt_be_ops[0],
  & sfmt_be_ops[0],
  & sfmt_be_ops[0],
  & sfmt_call_ops[0],
  & sfmt_calli_ops[0],
  & sfmt_add_ops[0],
  & sfmt_addi_ops[0],
  & sfmt_add_ops[0],
  & sfmt_addi_ops[0],
  & sfmt_add_ops[0],
  & sfmt_addi_ops[0],
  & sfmt_add_ops[0],
  & sfmt_andi_ops[0],
  & sfmt_add_ops[0],
  & sfmt_andi_ops[0],
  & sfmt_add_ops[0],
  & sfmt_addi_ops[0],
  & sfmt_divu_ops[0],
  & sfmt_lb_ops[0],
  & sfmt_lb_ops[0],
  & sfmt_lh_ops[0],
  & sfmt_lh_ops[0],
  & sfmt_lw_ops[0],
  & sfmt_divu_ops[0],
  & sfmt_add_ops[0],
  & sfmt_addi_ops[0],
  & sfmt_add_ops[0],
  & sfmt_andi_ops[0],
  & sfmt_add_ops[0],
  & sfmt_ori_ops[0],
  & sfmt_andhii_ops[0],
  & sfmt_rcsr_ops[0],
  & sfmt_sb_ops[0],
  & sfmt_sextb_ops[0],
  & sfmt_sextb_ops[0],
  & sfmt_sh_ops[0],
  & sfmt_add_ops[0],
  & sfmt_addi_ops[0],
  & sfmt_add_ops[0],
  & sfmt_addi_ops[0],
  & sfmt_add_ops[0],
  & sfmt_addi_ops[0],
  & sfmt_add_ops[0],
  & sfmt_sw_ops[0],
  & sfmt_user_ops[0],
  & sfmt_wcsr_ops[0],
  & sfmt_add_ops[0],
  & sfmt_andi_ops[0],
  & sfmt_add_ops[0],
  & sfmt_andi_ops[0],
  & sfmt_break_ops[0],
  & sfmt_break_ops[0],
  & sfmt_bret_ops[0],
  & sfmt_bret_ops[0],
  & sfmt_bret_ops[0],
  & sfmt_sextb_ops[0],
  & sfmt_mvi_ops[0],
  & sfmt_mvui_ops[0],
  & sfmt_mvhi_ops[0],
  & sfmt_mva_ops[0],
  & sfmt_sextb_ops[0],
  & sfmt_nop_ops[0],
  & sfmt_lbgprel_ops[0],
  & sfmt_lbgprel_ops[0],
  & sfmt_lhgprel_ops[0],
  & sfmt_lhgprel_ops[0],
  & sfmt_lwgprel_ops[0],
  & sfmt_sbgprel_ops[0],
  & sfmt_shgprel_ops[0],
  & sfmt_swgprel_ops[0],
  & sfmt_lwgotrel_ops[0],
  & sfmt_orhigotoffi_ops[0],
  & sfmt_addgotoff_ops[0],
  & sfmt_swgotoff_ops[0],
  & sfmt_lwgotoff_ops[0],
  & sfmt_shgotoff_ops[0],
  & sfmt_lhgotoff_ops[0],
  & sfmt_lhgotoff_ops[0],
  & sfmt_sbgotoff_ops[0],
  & sfmt_lbgotoff_ops[0],
  & sfmt_lbgotoff_ops[0],
};

/* Function to call before using the operand instance table.  */

void
lm32_cgen_init_opinst_table (CGEN_CPU_DESC cd)
{
  int i;
  const CGEN_OPINST **oi = & lm32_cgen_opinst_table[0];
  CGEN_INSN *insns = (CGEN_INSN *) cd->insn_table.init_entries;
  for (i = 0; i < MAX_INSNS; ++i)
    insns[i].opinst = oi[i];
}
