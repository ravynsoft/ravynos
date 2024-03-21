/* DO NOT EDIT!  -*- buffer-read-only: t -*- vi:set ro:  */
/* Instruction opcode table for mep.

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
#include "mep-desc.h"
#include "mep-opc.h"
#include "libiberty.h"

/* -- opc.c */
#include "elf/mep.h"

/* A mask for all ISAs executed by the core. */
CGEN_ATTR_VALUE_BITSET_TYPE mep_all_core_isas_mask = {0, 0};

void
init_mep_all_core_isas_mask (void)
{
  if (mep_all_core_isas_mask.length != 0)
    return;
  cgen_bitset_init (& mep_all_core_isas_mask, ISA_MAX);
  cgen_bitset_set (& mep_all_core_isas_mask, ISA_MEP);
  /* begin-all-core-isas */
  cgen_bitset_add (& mep_all_core_isas_mask, ISA_EXT_CORE1);
  /* end-all-core-isas */
}

CGEN_ATTR_VALUE_BITSET_TYPE mep_all_cop_isas_mask = {0, 0};

void
init_mep_all_cop_isas_mask (void)
{
  if (mep_all_cop_isas_mask.length != 0)
    return;
  cgen_bitset_init (& mep_all_cop_isas_mask, ISA_MAX);
  /* begin-all-cop-isas */
  cgen_bitset_add (& mep_all_cop_isas_mask, ISA_EXT_COP1_16);
  cgen_bitset_add (& mep_all_cop_isas_mask, ISA_EXT_COP1_32);
  cgen_bitset_add (& mep_all_cop_isas_mask, ISA_EXT_COP1_48);
  cgen_bitset_add (& mep_all_cop_isas_mask, ISA_EXT_COP1_64);
  /* end-all-cop-isas */
}

int
mep_insn_supported_by_isa (const CGEN_INSN *insn, CGEN_ATTR_VALUE_BITSET_TYPE *isa_mask)
{
  CGEN_BITSET insn_isas = CGEN_INSN_BITSET_ATTR_VALUE (insn, CGEN_INSN_ISA);
  return cgen_bitset_intersect_p (& insn_isas, isa_mask);
}

#define OPTION_MASK \
	( (1 << CGEN_INSN_OPTIONAL_BIT_INSN) \
	| (1 << CGEN_INSN_OPTIONAL_MUL_INSN) \
	| (1 << CGEN_INSN_OPTIONAL_DIV_INSN) \
	| (1 << CGEN_INSN_OPTIONAL_DEBUG_INSN) \
	| (1 << CGEN_INSN_OPTIONAL_LDZ_INSN) \
	| (1 << CGEN_INSN_OPTIONAL_ABS_INSN) \
	| (1 << CGEN_INSN_OPTIONAL_AVE_INSN) \
	| (1 << CGEN_INSN_OPTIONAL_MINMAX_INSN) \
	| (1 << CGEN_INSN_OPTIONAL_CLIP_INSN) \
	| (1 << CGEN_INSN_OPTIONAL_SAT_INSN) \
	| (1 << CGEN_INSN_OPTIONAL_UCI_INSN) \
	| (1 << CGEN_INSN_OPTIONAL_DSP_INSN) \
	| (1 << CGEN_INSN_OPTIONAL_CP_INSN) \
	| (1 << CGEN_INSN_OPTIONAL_CP64_INSN) )


mep_config_map_struct mep_config_map[] =
{
  /* config-map-start */
  /* Default entry: first module, with all options enabled. */
  { "", 0,  EF_MEP_COP_IVC2 | EF_MEP_CPU_C5,0, 64, { 1, "\x20" }, { 1, "\x10" }, { 1, "\x8" }, { 1, "\x4" }, { 1, "\x3c" }, { 1, "\xc0" }, OPTION_MASK | (1 << CGEN_INSN_OPTIONAL_DSP_INSN) | (1 << CGEN_INSN_OPTIONAL_UCI_INSN) },
  { "default", CONFIG_DEFAULT, EF_MEP_COP_IVC2 | EF_MEP_CPU_C5, 0, 64, { 1, "\x20" }, { 1, "\x10" }, { 1, "\x8" }, { 1, "\x4" }, { 1, "\x3c" }, { 1, "\xc0" },
	  0
	| (1 << CGEN_INSN_OPTIONAL_CP_INSN)
	| (1 << CGEN_INSN_OPTIONAL_CP64_INSN)
	| (1 << CGEN_INSN_OPTIONAL_MUL_INSN)
	| (1 << CGEN_INSN_OPTIONAL_DIV_INSN)
	| (1 << CGEN_INSN_OPTIONAL_BIT_INSN)
	| (1 << CGEN_INSN_OPTIONAL_LDZ_INSN)
	| (1 << CGEN_INSN_OPTIONAL_ABS_INSN)
	| (1 << CGEN_INSN_OPTIONAL_AVE_INSN)
	| (1 << CGEN_INSN_OPTIONAL_MINMAX_INSN)
	| (1 << CGEN_INSN_OPTIONAL_CLIP_INSN)
	| (1 << CGEN_INSN_OPTIONAL_SAT_INSN) },
  /* config-map-end */
  { 0, 0, 0, 0, 0, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 0 }
};

int mep_config_index = 0;

static int
check_configured_mach (int machs)
{
  /* All base insns are supported.  */
  int mach = 1 << MACH_BASE;
  switch (MEP_CPU & EF_MEP_CPU_MASK)
    {
    case EF_MEP_CPU_C2:
    case EF_MEP_CPU_C3:
      mach |= (1 << MACH_MEP);
      break;
    case EF_MEP_CPU_H1:
      mach |= (1 << MACH_H1);
      break;
    case EF_MEP_CPU_C5:
      mach |= (1 << MACH_MEP);
      mach |= (1 << MACH_C5);
      break;
    default:
      break;
    }
  return machs & mach;
}

int
mep_cgen_insn_supported (CGEN_CPU_DESC cd, const CGEN_INSN *insn)
{
  int iconfig = CGEN_INSN_ATTR_VALUE (insn, CGEN_INSN_CONFIG);
  int machs = CGEN_INSN_ATTR_VALUE (insn, CGEN_INSN_MACH);
  CGEN_BITSET isas = CGEN_INSN_BITSET_ATTR_VALUE (insn, CGEN_INSN_ISA);
  int ok1;
  int ok2;
  int ok3;

  /* If the insn has an option bit set that we don't want,
     reject it.  */
  if (CGEN_INSN_ATTRS (insn)->bool_ & OPTION_MASK & ~MEP_OMASK)
    return 0;

  /* If attributes are absent, assume no restriction. */
  if (machs == 0)
    machs = ~0;

  ok1 = ((machs & cd->machs) && cgen_bitset_intersect_p (& isas, cd->isas));
  /* If the insn is config-specific, make sure it matches.  */
  ok2 =  (iconfig == 0 || iconfig == MEP_CONFIG);
  /* Make sure the insn is supported by the configured mach  */
  ok3 = check_configured_mach (machs);

  return (ok1 && ok2 && ok3);
}

int
mep_cgen_insn_supported_asm (CGEN_CPU_DESC cd, const CGEN_INSN *insn)
{
#ifdef MEP_IVC2_SUPPORTED
  /* If we're assembling VLIW packets, ignore the 12-bit BSR as we
     can't relax that.  The 24-bit BSR is matched instead.  */
  if (insn->base->num == MEP_INSN_BSR12
      && cgen_bitset_contains (cd->isas, ISA_EXT_COP1_64))
    return 0;
#endif

  return mep_cgen_insn_supported (cd, insn);
}
/* The hash functions are recorded here to help keep assembler code out of
   the disassembler and vice versa.  */

static int asm_hash_insn_p        (const CGEN_INSN *);
static unsigned int asm_hash_insn (const char *);
static int dis_hash_insn_p        (const CGEN_INSN *);
static unsigned int dis_hash_insn (const char *, CGEN_INSN_INT);

/* Instruction formats.  */

#define F(f) & mep_cgen_ifld_table[MEP_##f]
static const CGEN_IFMT ifmt_empty ATTRIBUTE_UNUSED = {
  0, 0, 0x0, { { 0 } }
};

static const CGEN_IFMT ifmt_stcb_r ATTRIBUTE_UNUSED = {
  16, 16, 0xf00f, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_RM) }, { F (F_SUB4) }, { 0 } }
};

static const CGEN_IFMT ifmt_pref ATTRIBUTE_UNUSED = {
  16, 16, 0xf00f, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_RM) }, { F (F_SUB4) }, { 0 } }
};

static const CGEN_IFMT ifmt_prefd ATTRIBUTE_UNUSED = {
  32, 32, 0xf00f0000, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_RM) }, { F (F_SUB4) }, { F (F_16S16) }, { 0 } }
};

static const CGEN_IFMT ifmt_casb3 ATTRIBUTE_UNUSED = {
  32, 32, 0xf00ff0ff, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_RM) }, { F (F_SUB4) }, { F (F_C5N4) }, { F (F_RL5) }, { F (F_C5N6) }, { F (F_C5N7) }, { 0 } }
};

static const CGEN_IFMT ifmt_sbcp ATTRIBUTE_UNUSED = {
  32, 32, 0xf00ff000, { { F (F_MAJOR) }, { F (F_CRN) }, { F (F_RM) }, { F (F_SUB4) }, { F (F_EXT4) }, { F (F_12S20) }, { 0 } }
};

static const CGEN_IFMT ifmt_lbucpa ATTRIBUTE_UNUSED = {
  32, 32, 0xf00ffc00, { { F (F_MAJOR) }, { F (F_CRN) }, { F (F_RM) }, { F (F_SUB4) }, { F (F_EXT4) }, { F (F_EXT62) }, { F (F_CDISP10) }, { 0 } }
};

static const CGEN_IFMT ifmt_lhucpa ATTRIBUTE_UNUSED = {
  32, 32, 0xf00ffc00, { { F (F_MAJOR) }, { F (F_CRN) }, { F (F_RM) }, { F (F_SUB4) }, { F (F_EXT4) }, { F (F_EXT62) }, { F (F_CDISP10) }, { 0 } }
};

static const CGEN_IFMT ifmt_uci ATTRIBUTE_UNUSED = {
  32, 32, 0xf00f0000, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_RM) }, { F (F_SUB4) }, { F (F_16S16) }, { 0 } }
};

static const CGEN_IFMT ifmt_dsp ATTRIBUTE_UNUSED = {
  32, 32, 0xf00f0000, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_RM) }, { F (F_SUB4) }, { F (F_16U16) }, { 0 } }
};

static const CGEN_IFMT ifmt_dsp0 ATTRIBUTE_UNUSED = {
  32, 32, 0xf00f0000, { { F (F_MAJOR) }, { F (F_C5_RNMUIMM24) }, { F (F_SUB4) }, { 0 } }
};

static const CGEN_IFMT ifmt_dsp1 ATTRIBUTE_UNUSED = {
  32, 32, 0xf00f0000, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_C5_RMUIMM20) }, { F (F_SUB4) }, { 0 } }
};

static const CGEN_IFMT ifmt_sb ATTRIBUTE_UNUSED = {
  16, 16, 0xf00f, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_RM) }, { F (F_SUB4) }, { 0 } }
};

static const CGEN_IFMT ifmt_sh ATTRIBUTE_UNUSED = {
  16, 16, 0xf00f, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_RM) }, { F (F_SUB4) }, { 0 } }
};

static const CGEN_IFMT ifmt_sw ATTRIBUTE_UNUSED = {
  16, 16, 0xf00f, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_RM) }, { F (F_SUB4) }, { 0 } }
};

static const CGEN_IFMT ifmt_lbu ATTRIBUTE_UNUSED = {
  16, 16, 0xf00f, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_RM) }, { F (F_SUB4) }, { 0 } }
};

static const CGEN_IFMT ifmt_lhu ATTRIBUTE_UNUSED = {
  16, 16, 0xf00f, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_RM) }, { F (F_SUB4) }, { 0 } }
};

static const CGEN_IFMT ifmt_sw_sp ATTRIBUTE_UNUSED = {
  16, 16, 0xf083, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_8) }, { F (F_7U9A4) }, { F (F_SUB2) }, { 0 } }
};

static const CGEN_IFMT ifmt_sb_tp ATTRIBUTE_UNUSED = {
  16, 16, 0xf880, { { F (F_MAJOR) }, { F (F_4) }, { F (F_RN3) }, { F (F_8) }, { F (F_7U9) }, { 0 } }
};

static const CGEN_IFMT ifmt_sh_tp ATTRIBUTE_UNUSED = {
  16, 16, 0xf881, { { F (F_MAJOR) }, { F (F_4) }, { F (F_RN3) }, { F (F_8) }, { F (F_7U9A2) }, { F (F_15) }, { 0 } }
};

static const CGEN_IFMT ifmt_sw_tp ATTRIBUTE_UNUSED = {
  16, 16, 0xf883, { { F (F_MAJOR) }, { F (F_4) }, { F (F_RN3) }, { F (F_8) }, { F (F_7U9A4) }, { F (F_SUB2) }, { 0 } }
};

static const CGEN_IFMT ifmt_lbu_tp ATTRIBUTE_UNUSED = {
  16, 16, 0xf880, { { F (F_MAJOR) }, { F (F_4) }, { F (F_RN3) }, { F (F_8) }, { F (F_7U9) }, { 0 } }
};

static const CGEN_IFMT ifmt_lhu_tp ATTRIBUTE_UNUSED = {
  16, 16, 0xf881, { { F (F_MAJOR) }, { F (F_4) }, { F (F_RN3) }, { F (F_8) }, { F (F_7U9A2) }, { F (F_15) }, { 0 } }
};

static const CGEN_IFMT ifmt_sb16 ATTRIBUTE_UNUSED = {
  32, 32, 0xf00f0000, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_RM) }, { F (F_SUB4) }, { F (F_16S16) }, { 0 } }
};

static const CGEN_IFMT ifmt_sh16 ATTRIBUTE_UNUSED = {
  32, 32, 0xf00f0000, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_RM) }, { F (F_SUB4) }, { F (F_16S16) }, { 0 } }
};

static const CGEN_IFMT ifmt_sw16 ATTRIBUTE_UNUSED = {
  32, 32, 0xf00f0000, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_RM) }, { F (F_SUB4) }, { F (F_16S16) }, { 0 } }
};

static const CGEN_IFMT ifmt_lbu16 ATTRIBUTE_UNUSED = {
  32, 32, 0xf00f0000, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_RM) }, { F (F_SUB4) }, { F (F_16S16) }, { 0 } }
};

static const CGEN_IFMT ifmt_lhu16 ATTRIBUTE_UNUSED = {
  32, 32, 0xf00f0000, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_RM) }, { F (F_SUB4) }, { F (F_16S16) }, { 0 } }
};

static const CGEN_IFMT ifmt_sw24 ATTRIBUTE_UNUSED = {
  32, 32, 0xf0030000, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_24U8A4N) }, { F (F_SUB2) }, { 0 } }
};

static const CGEN_IFMT ifmt_extb ATTRIBUTE_UNUSED = {
  16, 16, 0xf0ff, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_RM) }, { F (F_SUB4) }, { 0 } }
};

static const CGEN_IFMT ifmt_ssarb ATTRIBUTE_UNUSED = {
  16, 16, 0xfc0f, { { F (F_MAJOR) }, { F (F_4) }, { F (F_5) }, { F (F_2U6) }, { F (F_RM) }, { F (F_SUB4) }, { 0 } }
};

static const CGEN_IFMT ifmt_mov ATTRIBUTE_UNUSED = {
  16, 16, 0xf00f, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_RM) }, { F (F_SUB4) }, { 0 } }
};

static const CGEN_IFMT ifmt_movi8 ATTRIBUTE_UNUSED = {
  16, 16, 0xf000, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_8S8) }, { 0 } }
};

static const CGEN_IFMT ifmt_movi16 ATTRIBUTE_UNUSED = {
  32, 32, 0xf0ff0000, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_RM) }, { F (F_SUB4) }, { F (F_16S16) }, { 0 } }
};

static const CGEN_IFMT ifmt_movu24 ATTRIBUTE_UNUSED = {
  32, 32, 0xf8000000, { { F (F_MAJOR) }, { F (F_4) }, { F (F_RN3) }, { F (F_24U8N) }, { 0 } }
};

static const CGEN_IFMT ifmt_movu16 ATTRIBUTE_UNUSED = {
  32, 32, 0xf0ff0000, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_RM) }, { F (F_SUB4) }, { F (F_16U16) }, { 0 } }
};

static const CGEN_IFMT ifmt_add3 ATTRIBUTE_UNUSED = {
  16, 16, 0xf000, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_RM) }, { F (F_RL) }, { 0 } }
};

static const CGEN_IFMT ifmt_add ATTRIBUTE_UNUSED = {
  16, 16, 0xf003, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_6S8) }, { F (F_SUB2) }, { 0 } }
};

static const CGEN_IFMT ifmt_add3i ATTRIBUTE_UNUSED = {
  16, 16, 0xf083, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_8) }, { F (F_7U9A4) }, { F (F_SUB2) }, { 0 } }
};

static const CGEN_IFMT ifmt_slt3i ATTRIBUTE_UNUSED = {
  16, 16, 0xf007, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_5U8) }, { F (F_SUB3) }, { 0 } }
};

static const CGEN_IFMT ifmt_bra ATTRIBUTE_UNUSED = {
  16, 16, 0xf001, { { F (F_MAJOR) }, { F (F_12S4A2) }, { F (F_15) }, { 0 } }
};

static const CGEN_IFMT ifmt_beqz ATTRIBUTE_UNUSED = {
  16, 16, 0xf001, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_8S8A2) }, { F (F_15) }, { 0 } }
};

static const CGEN_IFMT ifmt_beqi ATTRIBUTE_UNUSED = {
  32, 32, 0xf00f0000, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_4U8) }, { F (F_SUB4) }, { F (F_17S16A2) }, { 0 } }
};

static const CGEN_IFMT ifmt_beq ATTRIBUTE_UNUSED = {
  32, 32, 0xf00f0000, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_RM) }, { F (F_SUB4) }, { F (F_17S16A2) }, { 0 } }
};

static const CGEN_IFMT ifmt_bsr24 ATTRIBUTE_UNUSED = {
  32, 32, 0xf80f0000, { { F (F_MAJOR) }, { F (F_4) }, { F (F_24S5A2N) }, { F (F_SUB4) }, { 0 } }
};

static const CGEN_IFMT ifmt_jmp ATTRIBUTE_UNUSED = {
  16, 16, 0xff0f, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_RM) }, { F (F_SUB4) }, { 0 } }
};

static const CGEN_IFMT ifmt_jmp24 ATTRIBUTE_UNUSED = {
  32, 32, 0xf80f0000, { { F (F_MAJOR) }, { F (F_4) }, { F (F_24U5A2N) }, { F (F_SUB4) }, { 0 } }
};

static const CGEN_IFMT ifmt_ret ATTRIBUTE_UNUSED = {
  16, 16, 0xffff, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_RM) }, { F (F_SUB4) }, { 0 } }
};

static const CGEN_IFMT ifmt_repeat ATTRIBUTE_UNUSED = {
  32, 32, 0xf0ff0000, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_RM) }, { F (F_SUB4) }, { F (F_17S16A2) }, { 0 } }
};

static const CGEN_IFMT ifmt_erepeat ATTRIBUTE_UNUSED = {
  32, 32, 0xffff0000, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_RM) }, { F (F_SUB4) }, { F (F_17S16A2) }, { 0 } }
};

static const CGEN_IFMT ifmt_stc_lp ATTRIBUTE_UNUSED = {
  16, 16, 0xf0ff, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_CSRN_LO) }, { F (F_12) }, { F (F_13) }, { F (F_14) }, { F (F_CSRN_HI) }, { 0 } }
};

static const CGEN_IFMT ifmt_stc ATTRIBUTE_UNUSED = {
  16, 16, 0xf00e, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_CSRN) }, { F (F_12) }, { F (F_13) }, { F (F_14) }, { 0 } }
};

static const CGEN_IFMT ifmt_swi ATTRIBUTE_UNUSED = {
  16, 16, 0xffcf, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_8) }, { F (F_9) }, { F (F_2U10) }, { F (F_SUB4) }, { 0 } }
};

static const CGEN_IFMT ifmt_bsetm ATTRIBUTE_UNUSED = {
  16, 16, 0xf80f, { { F (F_MAJOR) }, { F (F_4) }, { F (F_3U5) }, { F (F_RM) }, { F (F_SUB4) }, { 0 } }
};

static const CGEN_IFMT ifmt_madd ATTRIBUTE_UNUSED = {
  32, 32, 0xf00fffff, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_RM) }, { F (F_SUB4) }, { F (F_16U16) }, { 0 } }
};

static const CGEN_IFMT ifmt_clip ATTRIBUTE_UNUSED = {
  32, 32, 0xf0ffff07, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_RM) }, { F (F_SUB4) }, { F (F_EXT) }, { F (F_5U24) }, { F (F_29) }, { F (F_30) }, { F (F_31) }, { 0 } }
};

static const CGEN_IFMT ifmt_swcp ATTRIBUTE_UNUSED = {
  16, 16, 0xf00f, { { F (F_MAJOR) }, { F (F_CRN) }, { F (F_RM) }, { F (F_SUB4) }, { 0 } }
};

static const CGEN_IFMT ifmt_smcp ATTRIBUTE_UNUSED = {
  16, 16, 0xf00f, { { F (F_MAJOR) }, { F (F_CRN) }, { F (F_RM) }, { F (F_SUB4) }, { 0 } }
};

static const CGEN_IFMT ifmt_swcp16 ATTRIBUTE_UNUSED = {
  32, 32, 0xf00f0000, { { F (F_MAJOR) }, { F (F_CRN) }, { F (F_RM) }, { F (F_SUB4) }, { F (F_16S16) }, { 0 } }
};

static const CGEN_IFMT ifmt_smcp16 ATTRIBUTE_UNUSED = {
  32, 32, 0xf00f0000, { { F (F_MAJOR) }, { F (F_CRN) }, { F (F_RM) }, { F (F_SUB4) }, { F (F_16S16) }, { 0 } }
};

static const CGEN_IFMT ifmt_swcpa ATTRIBUTE_UNUSED = {
  32, 32, 0xf00ffc00, { { F (F_MAJOR) }, { F (F_CRN) }, { F (F_RM) }, { F (F_SUB4) }, { F (F_EXT4) }, { F (F_EXT62) }, { F (F_CDISP10) }, { 0 } }
};

static const CGEN_IFMT ifmt_smcpa ATTRIBUTE_UNUSED = {
  32, 32, 0xf00ffc00, { { F (F_MAJOR) }, { F (F_CRN) }, { F (F_RM) }, { F (F_SUB4) }, { F (F_EXT4) }, { F (F_EXT62) }, { F (F_CDISP10) }, { 0 } }
};

static const CGEN_IFMT ifmt_bcpeq ATTRIBUTE_UNUSED = {
  32, 32, 0xff0f0000, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_RM) }, { F (F_SUB4) }, { F (F_17S16A2) }, { 0 } }
};

static const CGEN_IFMT ifmt_sim_syscall ATTRIBUTE_UNUSED = {
  16, 16, 0xf8ef, { { F (F_MAJOR) }, { F (F_4) }, { F (F_CALLNUM) }, { F (F_8) }, { F (F_9) }, { F (F_10) }, { F (F_SUB4) }, { 0 } }
};

static const CGEN_IFMT ifmt_cmov_crn_rm ATTRIBUTE_UNUSED = {
  32, 32, 0xf00ffff7, { { F (F_MAJOR) }, { F (F_CRNX) }, { F (F_RM) }, { F (F_SUB4) }, { F (F_IVC2_4U16) }, { F (F_IVC2_4U20) }, { F (F_IVC2_4U24) }, { F (F_29) }, { F (F_30) }, { F (F_31) }, { 0 } }
};

static const CGEN_IFMT ifmt_cmovc_ccrn_rm ATTRIBUTE_UNUSED = {
  32, 32, 0xf00ffff3, { { F (F_MAJOR) }, { F (F_IVC2_CCRN_C3) }, { F (F_RM) }, { F (F_SUB4) }, { F (F_IVC2_4U16) }, { F (F_IVC2_4U20) }, { F (F_IVC2_4U24) }, { F (F_30) }, { F (F_31) }, { 0 } }
};

static const CGEN_IFMT ifmt_cmov_crn_rm_p0 ATTRIBUTE_UNUSED = {
  32, 32, 0xfff7ff, { { F (F_IVC2_CRNX) }, { F (F_IVC2_CRM) }, { F (F_IVC2_CMOV1) }, { F (F_21) }, { F (F_IVC2_CMOV2) }, { F (F_IVC2_CMOV3) }, { 0 } }
};

static const CGEN_IFMT ifmt_cmovc_ccrn_rm_p0 ATTRIBUTE_UNUSED = {
  32, 32, 0xfff3ff, { { F (F_IVC2_CCRN) }, { F (F_IVC2_CRM) }, { F (F_IVC2_CMOV1) }, { F (F_IVC2_CMOV2) }, { F (F_IVC2_CMOV3) }, { 0 } }
};

static const CGEN_IFMT ifmt_cpadd3_b_C3 ATTRIBUTE_UNUSED = {
  32, 32, 0xfe0ff801, { { F (F_MAJOR) }, { F (F_IVC2_3U4) }, { F (F_IVC2_5U7) }, { F (F_SUB4) }, { F (F_IVC2_5U16) }, { F (F_IVC2_5U21) }, { F (F_IVC2_5U26) }, { F (F_IVC2_1U31) }, { 0 } }
};

static const CGEN_IFMT ifmt_cpfsftbi_C3 ATTRIBUTE_UNUSED = {
  32, 32, 0xf00ff801, { { F (F_MAJOR) }, { F (F_IVC2_3U4) }, { F (F_IVC2_5U7) }, { F (F_SUB4) }, { F (F_IVC2_5U16) }, { F (F_IVC2_5U21) }, { F (F_IVC2_5U26) }, { F (F_IVC2_1U31) }, { 0 } }
};

static const CGEN_IFMT ifmt_cpmovfrcsar0_C3 ATTRIBUTE_UNUSED = {
  32, 32, 0xfe0fffff, { { F (F_MAJOR) }, { F (F_IVC2_3U4) }, { F (F_IVC2_5U7) }, { F (F_SUB4) }, { F (F_IVC2_5U16) }, { F (F_IVC2_5U21) }, { F (F_IVC2_5U26) }, { F (F_IVC2_1U31) }, { 0 } }
};

static const CGEN_IFMT ifmt_cpmovtocsar0_C3 ATTRIBUTE_UNUSED = {
  32, 32, 0xfffff83f, { { F (F_MAJOR) }, { F (F_IVC2_3U4) }, { F (F_IVC2_5U7) }, { F (F_SUB4) }, { F (F_IVC2_5U16) }, { F (F_IVC2_5U21) }, { F (F_IVC2_5U26) }, { F (F_IVC2_1U31) }, { 0 } }
};

static const CGEN_IFMT ifmt_cpmov_C3 ATTRIBUTE_UNUSED = {
  32, 32, 0xfe0ff83f, { { F (F_MAJOR) }, { F (F_IVC2_3U4) }, { F (F_IVC2_5U7) }, { F (F_SUB4) }, { F (F_IVC2_5U16) }, { F (F_IVC2_5U21) }, { F (F_IVC2_5U26) }, { F (F_IVC2_1U31) }, { 0 } }
};

static const CGEN_IFMT ifmt_cpcmpeqz_b_C3 ATTRIBUTE_UNUSED = {
  32, 32, 0xfffff801, { { F (F_MAJOR) }, { F (F_IVC2_3U4) }, { F (F_IVC2_5U7) }, { F (F_SUB4) }, { F (F_IVC2_5U16) }, { F (F_IVC2_5U21) }, { F (F_IVC2_5U26) }, { F (F_IVC2_1U31) }, { 0 } }
};

static const CGEN_IFMT ifmt_cpsrli3_b_C3 ATTRIBUTE_UNUSED = {
  32, 32, 0xfc0ff801, { { F (F_MAJOR) }, { F (F_IVC2_2U4) }, { F (F_IVC2_3U6) }, { F (F_IVC2_3U9) }, { F (F_SUB4) }, { F (F_IVC2_5U16) }, { F (F_IVC2_5U21) }, { F (F_IVC2_5U26) }, { F (F_IVC2_1U31) }, { 0 } }
};

static const CGEN_IFMT ifmt_cpsrli3_h_C3 ATTRIBUTE_UNUSED = {
  32, 32, 0xfc0ff801, { { F (F_MAJOR) }, { F (F_IVC2_2U4) }, { F (F_IVC2_2U6) }, { F (F_IVC2_4U8) }, { F (F_SUB4) }, { F (F_IVC2_5U16) }, { F (F_IVC2_5U21) }, { F (F_IVC2_5U26) }, { F (F_IVC2_1U31) }, { 0 } }
};

static const CGEN_IFMT ifmt_cpsrli3_w_C3 ATTRIBUTE_UNUSED = {
  32, 32, 0xfc0ff801, { { F (F_MAJOR) }, { F (F_IVC2_2U4) }, { F (F_IVC2_1U6) }, { F (F_IVC2_5U7) }, { F (F_SUB4) }, { F (F_IVC2_5U16) }, { F (F_IVC2_5U21) }, { F (F_IVC2_5U26) }, { F (F_IVC2_1U31) }, { 0 } }
};

static const CGEN_IFMT ifmt_cdsrli3_C3 ATTRIBUTE_UNUSED = {
  32, 32, 0xfc0ff801, { { F (F_MAJOR) }, { F (F_IVC2_2U4) }, { F (F_IVC2_6U6) }, { F (F_SUB4) }, { F (F_IVC2_5U16) }, { F (F_IVC2_5U21) }, { F (F_IVC2_5U26) }, { F (F_IVC2_1U31) }, { 0 } }
};

static const CGEN_IFMT ifmt_cpmovi_b_C3 ATTRIBUTE_UNUSED = {
  32, 32, 0xf00ff83f, { { F (F_MAJOR) }, { F (F_IVC2_8S4) }, { F (F_SUB4) }, { F (F_IVC2_5U16) }, { F (F_IVC2_5U21) }, { F (F_IVC2_5U26) }, { F (F_IVC2_1U31) }, { 0 } }
};

static const CGEN_IFMT ifmt_cpmoviu_h_C3 ATTRIBUTE_UNUSED = {
  32, 32, 0xf00ff83f, { { F (F_MAJOR) }, { F (F_IVC2_8U4) }, { F (F_SUB4) }, { F (F_IVC2_5U16) }, { F (F_IVC2_5U21) }, { F (F_IVC2_5U26) }, { F (F_IVC2_1U31) }, { 0 } }
};

static const CGEN_IFMT ifmt_cpsrlia1_P1 ATTRIBUTE_UNUSED = {
  32, 32, 0xfc0fffff, { { F (F_MAJOR) }, { F (F_IVC2_2U4) }, { F (F_IVC2_1U6) }, { F (F_IVC2_5U7) }, { F (F_SUB4) }, { F (F_IVC2_5U16) }, { F (F_IVC2_5U21) }, { F (F_IVC2_5U26) }, { F (F_IVC2_1U31) }, { 0 } }
};

static const CGEN_IFMT ifmt_c0nop_P0_P0S ATTRIBUTE_UNUSED = {
  32, 32, 0xffffffff, { { F (F_IVC2_8U0) }, { F (F_IVC2_5U8) }, { F (F_IVC2_5U13) }, { F (F_IVC2_5U18) }, { F (F_IVC2_5U23) }, { F (F_IVC2_4U28) }, { 0 } }
};

static const CGEN_IFMT ifmt_cpadd3_b_P0S_P1 ATTRIBUTE_UNUSED = {
  32, 32, 0xfff8000f, { { F (F_IVC2_8U0) }, { F (F_IVC2_5U8) }, { F (F_IVC2_5U13) }, { F (F_IVC2_5U18) }, { F (F_IVC2_5U23) }, { F (F_IVC2_4U28) }, { 0 } }
};

static const CGEN_IFMT ifmt_cpmov_P0S_P1 ATTRIBUTE_UNUSED = {
  32, 32, 0xfff83e0f, { { F (F_IVC2_8U0) }, { F (F_IVC2_5U8) }, { F (F_IVC2_5U13) }, { F (F_IVC2_5U18) }, { F (F_IVC2_5U23) }, { F (F_IVC2_4U28) }, { 0 } }
};

static const CGEN_IFMT ifmt_cpccadd_b_P0S_P1 ATTRIBUTE_UNUSED = {
  32, 32, 0xfff83fff, { { F (F_IVC2_8U0) }, { F (F_IVC2_5U8) }, { F (F_IVC2_5U13) }, { F (F_IVC2_5U18) }, { F (F_IVC2_5U23) }, { F (F_IVC2_4U28) }, { 0 } }
};

static const CGEN_IFMT ifmt_cpmovfrcsar0_P0S_P1 ATTRIBUTE_UNUSED = {
  32, 32, 0xfffffe0f, { { F (F_IVC2_8U0) }, { F (F_IVC2_5U8) }, { F (F_IVC2_5U13) }, { F (F_IVC2_5U18) }, { F (F_IVC2_5U23) }, { F (F_IVC2_4U28) }, { 0 } }
};

static const CGEN_IFMT ifmt_cpcmpeqz_b_P0S_P1 ATTRIBUTE_UNUSED = {
  32, 32, 0xfff801ff, { { F (F_IVC2_8U0) }, { F (F_IVC2_5U8) }, { F (F_IVC2_5U13) }, { F (F_IVC2_5U18) }, { F (F_IVC2_5U23) }, { F (F_IVC2_4U28) }, { 0 } }
};

static const CGEN_IFMT ifmt_cpsrlia0_P0S ATTRIBUTE_UNUSED = {
  32, 32, 0xfffffe0f, { { F (F_IVC2_8U0) }, { F (F_IVC2_5U8) }, { F (F_IVC2_5U13) }, { F (F_IVC2_5U18) }, { F (F_IVC2_5U23) }, { F (F_IVC2_4U28) }, { 0 } }
};

static const CGEN_IFMT ifmt_cpfsftbi_P0_P1 ATTRIBUTE_UNUSED = {
  32, 32, 0xf8000f, { { F (F_IVC2_5U0) }, { F (F_IVC2_3U5) }, { F (F_IVC2_5U8) }, { F (F_IVC2_5U13) }, { F (F_IVC2_5U18) }, { F (F_IVC2_5U23) }, { F (F_IVC2_4U28) }, { 0 } }
};

static const CGEN_IFMT ifmt_cpsrli3_b_P0_P1 ATTRIBUTE_UNUSED = {
  32, 32, 0xf83e0f, { { F (F_IVC2_5U0) }, { F (F_IVC2_3U5) }, { F (F_IVC2_5U8) }, { F (F_IVC2_5U13) }, { F (F_IVC2_5U18) }, { F (F_IVC2_5U23) }, { F (F_IVC2_4U28) }, { 0 } }
};

static const CGEN_IFMT ifmt_cpsrli3_h_P0_P1 ATTRIBUTE_UNUSED = {
  32, 32, 0xf83e0f, { { F (F_IVC2_4U0) }, { F (F_IVC2_4U4) }, { F (F_IVC2_5U8) }, { F (F_IVC2_5U13) }, { F (F_IVC2_5U18) }, { F (F_IVC2_5U23) }, { F (F_IVC2_4U28) }, { 0 } }
};

static const CGEN_IFMT ifmt_cpsrli3_w_P0_P1 ATTRIBUTE_UNUSED = {
  32, 32, 0xf83e0f, { { F (F_IVC2_3U0) }, { F (F_IVC2_5U3) }, { F (F_IVC2_5U8) }, { F (F_IVC2_5U13) }, { F (F_IVC2_5U18) }, { F (F_IVC2_5U23) }, { F (F_IVC2_4U28) }, { 0 } }
};

static const CGEN_IFMT ifmt_cdsrli3_P0_P1 ATTRIBUTE_UNUSED = {
  32, 32, 0xf83e0f, { { F (F_IVC2_2U0) }, { F (F_IVC2_6U2) }, { F (F_IVC2_5U8) }, { F (F_IVC2_5U13) }, { F (F_IVC2_5U18) }, { F (F_IVC2_5U23) }, { F (F_IVC2_4U28) }, { 0 } }
};

static const CGEN_IFMT ifmt_cpmovi_h_P0_P1 ATTRIBUTE_UNUSED = {
  32, 32, 0xf8300f, { { F (F_IVC2_SIMM16P0) }, { F (F_IVC2_5U8) }, { F (F_IVC2_5U13) }, { F (F_IVC2_2U18) }, { F (F_IVC2_4U28) }, { 0 } }
};

static const CGEN_IFMT ifmt_cpmoviu_w_P0_P1 ATTRIBUTE_UNUSED = {
  32, 32, 0xf8300f, { { F (F_IVC2_IMM16P0) }, { F (F_IVC2_5U8) }, { F (F_IVC2_5U13) }, { F (F_IVC2_2U18) }, { F (F_IVC2_4U28) }, { 0 } }
};

static const CGEN_IFMT ifmt_cpmovi_b_P0S_P1 ATTRIBUTE_UNUSED = {
  32, 32, 0xfff8300f, { { F (F_IVC2_8U0) }, { F (F_IVC2_5U8) }, { F (F_IVC2_5U13) }, { F (F_IVC2_2U18) }, { F (F_IVC2_8U20) }, { F (F_IVC2_4U28) }, { 0 } }
};

static const CGEN_IFMT ifmt_cpfmulia1s0u_b_P1 ATTRIBUTE_UNUSED = {
  32, 32, 0xf801ff, { { F (F_IVC2_8S0) }, { F (F_IVC2_5U8) }, { F (F_IVC2_5U13) }, { F (F_IVC2_5U18) }, { F (F_IVC2_5U23) }, { F (F_IVC2_4U28) }, { 0 } }
};

static const CGEN_IFMT ifmt_cpfmulia1u_b_P1 ATTRIBUTE_UNUSED = {
  32, 32, 0xf8018f, { { F (F_IVC2_8S0) }, { F (F_IVC2_5U8) }, { F (F_IVC2_5U13) }, { F (F_IVC2_5U18) }, { F (F_IVC2_2U23) }, { F (F_IVC2_3U25) }, { F (F_IVC2_4U28) }, { 0 } }
};

#undef F

#define A(a) (1 << CGEN_INSN_##a)
#define OPERAND(op) MEP_OPERAND_##op
#define MNEM CGEN_SYNTAX_MNEMONIC /* syntax value for mnemonic */
#define OP(field) CGEN_SYNTAX_MAKE_FIELD (OPERAND (field))

/* The instruction table.  */

static const CGEN_OPCODE mep_cgen_insn_opcode_table[MAX_INSNS] =
{
  /* Special null first entry.
     A `num' value of zero is thus invalid.
     Also, the special `invalid' insn resides here.  */
  { { 0, 0, 0, 0 }, {{0}}, 0, {0}},
/* stcb $rn,($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', '(', OP (RMA), ')', 0 } },
    & ifmt_stcb_r, { 0x700c }
  },
/* ldcb $rn,($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', '(', OP (RMA), ')', 0 } },
    & ifmt_stcb_r, { 0x700d }
  },
/* pref $cimm4,($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CIMM4), ',', '(', OP (RMA), ')', 0 } },
    & ifmt_pref, { 0x7005 }
  },
/* pref $cimm4,$sdisp16($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CIMM4), ',', OP (SDISP16), '(', OP (RMA), ')', 0 } },
    & ifmt_prefd, { 0xf0030000 }
  },
/* casb3 $rl5,$rn,($rm) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RL5), ',', OP (RN), ',', '(', OP (RM), ')', 0 } },
    & ifmt_casb3, { 0xf0012000 }
  },
/* cash3 $rl5,$rn,($rm) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RL5), ',', OP (RN), ',', '(', OP (RM), ')', 0 } },
    & ifmt_casb3, { 0xf0012001 }
  },
/* casw3 $rl5,$rn,($rm) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RL5), ',', OP (RN), ',', '(', OP (RM), ')', 0 } },
    & ifmt_casb3, { 0xf0012002 }
  },
/* sbcp $crn,$cdisp12($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN), ',', OP (CDISP12), '(', OP (RMA), ')', 0 } },
    & ifmt_sbcp, { 0xf0060000 }
  },
/* lbcp $crn,$cdisp12($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN), ',', OP (CDISP12), '(', OP (RMA), ')', 0 } },
    & ifmt_sbcp, { 0xf0064000 }
  },
/* lbucp $crn,$cdisp12($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN), ',', OP (CDISP12), '(', OP (RMA), ')', 0 } },
    & ifmt_sbcp, { 0xf006c000 }
  },
/* shcp $crn,$cdisp12($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN), ',', OP (CDISP12), '(', OP (RMA), ')', 0 } },
    & ifmt_sbcp, { 0xf0061000 }
  },
/* lhcp $crn,$cdisp12($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN), ',', OP (CDISP12), '(', OP (RMA), ')', 0 } },
    & ifmt_sbcp, { 0xf0065000 }
  },
/* lhucp $crn,$cdisp12($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN), ',', OP (CDISP12), '(', OP (RMA), ')', 0 } },
    & ifmt_sbcp, { 0xf006d000 }
  },
/* lbucpa $crn,($rma+),$cdisp10 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN), ',', '(', OP (RMA), '+', ')', ',', OP (CDISP10), 0 } },
    & ifmt_lbucpa, { 0xf005c000 }
  },
/* lhucpa $crn,($rma+),$cdisp10a2 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN), ',', '(', OP (RMA), '+', ')', ',', OP (CDISP10A2), 0 } },
    & ifmt_lhucpa, { 0xf005d000 }
  },
/* lbucpm0 $crn,($rma+),$cdisp10 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN), ',', '(', OP (RMA), '+', ')', ',', OP (CDISP10), 0 } },
    & ifmt_lbucpa, { 0xf005c800 }
  },
/* lhucpm0 $crn,($rma+),$cdisp10a2 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN), ',', '(', OP (RMA), '+', ')', ',', OP (CDISP10A2), 0 } },
    & ifmt_lhucpa, { 0xf005d800 }
  },
/* lbucpm1 $crn,($rma+),$cdisp10 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN), ',', '(', OP (RMA), '+', ')', ',', OP (CDISP10), 0 } },
    & ifmt_lbucpa, { 0xf005cc00 }
  },
/* lhucpm1 $crn,($rma+),$cdisp10a2 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN), ',', '(', OP (RMA), '+', ')', ',', OP (CDISP10A2), 0 } },
    & ifmt_lhucpa, { 0xf005dc00 }
  },
/* uci $rn,$rm,$uimm16 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (RM), ',', OP (UIMM16), 0 } },
    & ifmt_uci, { 0xf0020000 }
  },
/* dsp $rn,$rm,$uimm16 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (RM), ',', OP (UIMM16), 0 } },
    & ifmt_dsp, { 0xf0000000 }
  },
/* dsp0 $c5rnmuimm24 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (C5RNMUIMM24), 0 } },
    & ifmt_dsp0, { 0xf0000000 }
  },
/* dsp1 $rn,$c5rmuimm20 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (C5RMUIMM20), 0 } },
    & ifmt_dsp1, { 0xf0000000 }
  },
/* sb $rnc,($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RNC), ',', '(', OP (RMA), ')', 0 } },
    & ifmt_sb, { 0x8 }
  },
/* sh $rns,($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RNS), ',', '(', OP (RMA), ')', 0 } },
    & ifmt_sh, { 0x9 }
  },
/* sw $rnl,($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RNL), ',', '(', OP (RMA), ')', 0 } },
    & ifmt_sw, { 0xa }
  },
/* lb $rnc,($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RNC), ',', '(', OP (RMA), ')', 0 } },
    & ifmt_sb, { 0xc }
  },
/* lh $rns,($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RNS), ',', '(', OP (RMA), ')', 0 } },
    & ifmt_sh, { 0xd }
  },
/* lw $rnl,($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RNL), ',', '(', OP (RMA), ')', 0 } },
    & ifmt_sw, { 0xe }
  },
/* lbu $rnuc,($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RNUC), ',', '(', OP (RMA), ')', 0 } },
    & ifmt_lbu, { 0xb }
  },
/* lhu $rnus,($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RNUS), ',', '(', OP (RMA), ')', 0 } },
    & ifmt_lhu, { 0xf }
  },
/* sw $rnl,$udisp7a4($spr) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RNL), ',', OP (UDISP7A4), '(', OP (SPR), ')', 0 } },
    & ifmt_sw_sp, { 0x4002 }
  },
/* lw $rnl,$udisp7a4($spr) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RNL), ',', OP (UDISP7A4), '(', OP (SPR), ')', 0 } },
    & ifmt_sw_sp, { 0x4003 }
  },
/* sb $rn3c,$udisp7($tpr) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN3C), ',', OP (UDISP7), '(', OP (TPR), ')', 0 } },
    & ifmt_sb_tp, { 0x8000 }
  },
/* sh $rn3s,$udisp7a2($tpr) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN3S), ',', OP (UDISP7A2), '(', OP (TPR), ')', 0 } },
    & ifmt_sh_tp, { 0x8080 }
  },
/* sw $rn3l,$udisp7a4($tpr) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN3L), ',', OP (UDISP7A4), '(', OP (TPR), ')', 0 } },
    & ifmt_sw_tp, { 0x4082 }
  },
/* lb $rn3c,$udisp7($tpr) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN3C), ',', OP (UDISP7), '(', OP (TPR), ')', 0 } },
    & ifmt_sb_tp, { 0x8800 }
  },
/* lh $rn3s,$udisp7a2($tpr) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN3S), ',', OP (UDISP7A2), '(', OP (TPR), ')', 0 } },
    & ifmt_sh_tp, { 0x8880 }
  },
/* lw $rn3l,$udisp7a4($tpr) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN3L), ',', OP (UDISP7A4), '(', OP (TPR), ')', 0 } },
    & ifmt_sw_tp, { 0x4083 }
  },
/* lbu $rn3uc,$udisp7($tpr) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN3UC), ',', OP (UDISP7), '(', OP (TPR), ')', 0 } },
    & ifmt_lbu_tp, { 0x4880 }
  },
/* lhu $rn3us,$udisp7a2($tpr) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN3US), ',', OP (UDISP7A2), '(', OP (TPR), ')', 0 } },
    & ifmt_lhu_tp, { 0x8881 }
  },
/* sb $rnc,$sdisp16($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RNC), ',', OP (SDISP16), '(', OP (RMA), ')', 0 } },
    & ifmt_sb16, { 0xc0080000 }
  },
/* sh $rns,$sdisp16($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RNS), ',', OP (SDISP16), '(', OP (RMA), ')', 0 } },
    & ifmt_sh16, { 0xc0090000 }
  },
/* sw $rnl,$sdisp16($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RNL), ',', OP (SDISP16), '(', OP (RMA), ')', 0 } },
    & ifmt_sw16, { 0xc00a0000 }
  },
/* lb $rnc,$sdisp16($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RNC), ',', OP (SDISP16), '(', OP (RMA), ')', 0 } },
    & ifmt_sb16, { 0xc00c0000 }
  },
/* lh $rns,$sdisp16($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RNS), ',', OP (SDISP16), '(', OP (RMA), ')', 0 } },
    & ifmt_sh16, { 0xc00d0000 }
  },
/* lw $rnl,$sdisp16($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RNL), ',', OP (SDISP16), '(', OP (RMA), ')', 0 } },
    & ifmt_sw16, { 0xc00e0000 }
  },
/* lbu $rnuc,$sdisp16($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RNUC), ',', OP (SDISP16), '(', OP (RMA), ')', 0 } },
    & ifmt_lbu16, { 0xc00b0000 }
  },
/* lhu $rnus,$sdisp16($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RNUS), ',', OP (SDISP16), '(', OP (RMA), ')', 0 } },
    & ifmt_lhu16, { 0xc00f0000 }
  },
/* sw $rnl,($addr24a4) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RNL), ',', '(', OP (ADDR24A4), ')', 0 } },
    & ifmt_sw24, { 0xe0020000 }
  },
/* lw $rnl,($addr24a4) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RNL), ',', '(', OP (ADDR24A4), ')', 0 } },
    & ifmt_sw24, { 0xe0030000 }
  },
/* extb $rn */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), 0 } },
    & ifmt_extb, { 0x100d }
  },
/* exth $rn */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), 0 } },
    & ifmt_extb, { 0x102d }
  },
/* extub $rn */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), 0 } },
    & ifmt_extb, { 0x108d }
  },
/* extuh $rn */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), 0 } },
    & ifmt_extb, { 0x10ad }
  },
/* ssarb $udisp2($rm) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (UDISP2), '(', OP (RM), ')', 0 } },
    & ifmt_ssarb, { 0x100c }
  },
/* mov $rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (RM), 0 } },
    & ifmt_mov, { 0x0 }
  },
/* mov $rn,$simm8 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (SIMM8), 0 } },
    & ifmt_movi8, { 0x5000 }
  },
/* mov $rn,$simm16 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (SIMM16), 0 } },
    & ifmt_movi16, { 0xc0010000 }
  },
/* movu $rn3,$uimm24 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN3), ',', OP (UIMM24), 0 } },
    & ifmt_movu24, { 0xd0000000 }
  },
/* movu $rn,$uimm16 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (UIMM16), 0 } },
    & ifmt_movu16, { 0xc0110000 }
  },
/* movh $rn,$uimm16 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (UIMM16), 0 } },
    & ifmt_movu16, { 0xc0210000 }
  },
/* add3 $rl,$rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RL), ',', OP (RN), ',', OP (RM), 0 } },
    & ifmt_add3, { 0x9000 }
  },
/* add $rn,$simm6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (SIMM6), 0 } },
    & ifmt_add, { 0x6000 }
  },
/* add3 $rn,$spr,$uimm7a4 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (SPR), ',', OP (UIMM7A4), 0 } },
    & ifmt_add3i, { 0x4000 }
  },
/* advck3 \$0,$rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', '$', '0', ',', OP (RN), ',', OP (RM), 0 } },
    & ifmt_mov, { 0x7 }
  },
/* sub $rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (RM), 0 } },
    & ifmt_mov, { 0x4 }
  },
/* sbvck3 \$0,$rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', '$', '0', ',', OP (RN), ',', OP (RM), 0 } },
    & ifmt_mov, { 0x5 }
  },
/* neg $rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (RM), 0 } },
    & ifmt_mov, { 0x1 }
  },
/* slt3 \$0,$rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', '$', '0', ',', OP (RN), ',', OP (RM), 0 } },
    & ifmt_mov, { 0x2 }
  },
/* sltu3 \$0,$rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', '$', '0', ',', OP (RN), ',', OP (RM), 0 } },
    & ifmt_mov, { 0x3 }
  },
/* slt3 \$0,$rn,$uimm5 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', '$', '0', ',', OP (RN), ',', OP (UIMM5), 0 } },
    & ifmt_slt3i, { 0x6001 }
  },
/* sltu3 \$0,$rn,$uimm5 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', '$', '0', ',', OP (RN), ',', OP (UIMM5), 0 } },
    & ifmt_slt3i, { 0x6005 }
  },
/* sl1ad3 \$0,$rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', '$', '0', ',', OP (RN), ',', OP (RM), 0 } },
    & ifmt_mov, { 0x2006 }
  },
/* sl2ad3 \$0,$rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', '$', '0', ',', OP (RN), ',', OP (RM), 0 } },
    & ifmt_mov, { 0x2007 }
  },
/* add3 $rn,$rm,$simm16 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (RM), ',', OP (SIMM16), 0 } },
    & ifmt_uci, { 0xc0000000 }
  },
/* slt3 $rn,$rm,$simm16 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (RM), ',', OP (SIMM16), 0 } },
    & ifmt_uci, { 0xc0020000 }
  },
/* sltu3 $rn,$rm,$uimm16 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (RM), ',', OP (UIMM16), 0 } },
    & ifmt_dsp, { 0xc0030000 }
  },
/* or $rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (RM), 0 } },
    & ifmt_mov, { 0x1000 }
  },
/* and $rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (RM), 0 } },
    & ifmt_mov, { 0x1001 }
  },
/* xor $rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (RM), 0 } },
    & ifmt_mov, { 0x1002 }
  },
/* nor $rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (RM), 0 } },
    & ifmt_mov, { 0x1003 }
  },
/* or3 $rn,$rm,$uimm16 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (RM), ',', OP (UIMM16), 0 } },
    & ifmt_dsp, { 0xc0040000 }
  },
/* and3 $rn,$rm,$uimm16 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (RM), ',', OP (UIMM16), 0 } },
    & ifmt_dsp, { 0xc0050000 }
  },
/* xor3 $rn,$rm,$uimm16 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (RM), ',', OP (UIMM16), 0 } },
    & ifmt_dsp, { 0xc0060000 }
  },
/* sra $rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (RM), 0 } },
    & ifmt_mov, { 0x200d }
  },
/* srl $rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (RM), 0 } },
    & ifmt_mov, { 0x200c }
  },
/* sll $rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (RM), 0 } },
    & ifmt_mov, { 0x200e }
  },
/* sra $rn,$uimm5 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (UIMM5), 0 } },
    & ifmt_slt3i, { 0x6003 }
  },
/* srl $rn,$uimm5 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (UIMM5), 0 } },
    & ifmt_slt3i, { 0x6002 }
  },
/* sll $rn,$uimm5 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (UIMM5), 0 } },
    & ifmt_slt3i, { 0x6006 }
  },
/* sll3 \$0,$rn,$uimm5 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', '$', '0', ',', OP (RN), ',', OP (UIMM5), 0 } },
    & ifmt_slt3i, { 0x6007 }
  },
/* fsft $rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (RM), 0 } },
    & ifmt_mov, { 0x200f }
  },
/* bra $pcrel12a2 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (PCREL12A2), 0 } },
    & ifmt_bra, { 0xb000 }
  },
/* beqz $rn,$pcrel8a2 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (PCREL8A2), 0 } },
    & ifmt_beqz, { 0xa000 }
  },
/* bnez $rn,$pcrel8a2 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (PCREL8A2), 0 } },
    & ifmt_beqz, { 0xa001 }
  },
/* beqi $rn,$uimm4,$pcrel17a2 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (UIMM4), ',', OP (PCREL17A2), 0 } },
    & ifmt_beqi, { 0xe0000000 }
  },
/* bnei $rn,$uimm4,$pcrel17a2 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (UIMM4), ',', OP (PCREL17A2), 0 } },
    & ifmt_beqi, { 0xe0040000 }
  },
/* blti $rn,$uimm4,$pcrel17a2 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (UIMM4), ',', OP (PCREL17A2), 0 } },
    & ifmt_beqi, { 0xe00c0000 }
  },
/* bgei $rn,$uimm4,$pcrel17a2 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (UIMM4), ',', OP (PCREL17A2), 0 } },
    & ifmt_beqi, { 0xe0080000 }
  },
/* beq $rn,$rm,$pcrel17a2 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (RM), ',', OP (PCREL17A2), 0 } },
    & ifmt_beq, { 0xe0010000 }
  },
/* bne $rn,$rm,$pcrel17a2 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (RM), ',', OP (PCREL17A2), 0 } },
    & ifmt_beq, { 0xe0050000 }
  },
/* bsr $pcrel12a2 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (PCREL12A2), 0 } },
    & ifmt_bra, { 0xb001 }
  },
/* bsr $pcrel24a2 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (PCREL24A2), 0 } },
    & ifmt_bsr24, { 0xd8090000 }
  },
/* jmp $rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RM), 0 } },
    & ifmt_jmp, { 0x100e }
  },
/* jmp $pcabs24a2 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (PCABS24A2), 0 } },
    & ifmt_jmp24, { 0xd8080000 }
  },
/* jsr $rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RM), 0 } },
    & ifmt_jmp, { 0x100f }
  },
/* ret */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_ret, { 0x7002 }
  },
/* repeat $rn,$pcrel17a2 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (PCREL17A2), 0 } },
    & ifmt_repeat, { 0xe0090000 }
  },
/* erepeat $pcrel17a2 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (PCREL17A2), 0 } },
    & ifmt_erepeat, { 0xe0190000 }
  },
/* stc $rn,\$lp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', '$', 'l', 'p', 0 } },
    & ifmt_stc_lp, { 0x7018 }
  },
/* stc $rn,\$hi */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', '$', 'h', 'i', 0 } },
    & ifmt_stc_lp, { 0x7078 }
  },
/* stc $rn,\$lo */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', '$', 'l', 'o', 0 } },
    & ifmt_stc_lp, { 0x7088 }
  },
/* stc $rn,$csrn */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (CSRN), 0 } },
    & ifmt_stc, { 0x7008 }
  },
/* ldc $rn,\$lp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', '$', 'l', 'p', 0 } },
    & ifmt_stc_lp, { 0x701a }
  },
/* ldc $rn,\$hi */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', '$', 'h', 'i', 0 } },
    & ifmt_stc_lp, { 0x707a }
  },
/* ldc $rn,\$lo */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', '$', 'l', 'o', 0 } },
    & ifmt_stc_lp, { 0x708a }
  },
/* ldc $rn,$csrn */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (CSRN), 0 } },
    & ifmt_stc, { 0x700a }
  },
/* di */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_ret, { 0x7000 }
  },
/* ei */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_ret, { 0x7010 }
  },
/* reti */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_ret, { 0x7012 }
  },
/* halt */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_ret, { 0x7022 }
  },
/* sleep */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_ret, { 0x7062 }
  },
/* swi $uimm2 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (UIMM2), 0 } },
    & ifmt_swi, { 0x7006 }
  },
/* break */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_ret, { 0x7032 }
  },
/* syncm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_ret, { 0x7011 }
  },
/* stcb $rn,$uimm16 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (UIMM16), 0 } },
    & ifmt_movu16, { 0xf0040000 }
  },
/* ldcb $rn,$uimm16 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (UIMM16), 0 } },
    & ifmt_movu16, { 0xf0140000 }
  },
/* bsetm ($rma),$uimm3 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', '(', OP (RMA), ')', ',', OP (UIMM3), 0 } },
    & ifmt_bsetm, { 0x2000 }
  },
/* bclrm ($rma),$uimm3 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', '(', OP (RMA), ')', ',', OP (UIMM3), 0 } },
    & ifmt_bsetm, { 0x2001 }
  },
/* bnotm ($rma),$uimm3 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', '(', OP (RMA), ')', ',', OP (UIMM3), 0 } },
    & ifmt_bsetm, { 0x2002 }
  },
/* btstm \$0,($rma),$uimm3 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', '$', '0', ',', '(', OP (RMA), ')', ',', OP (UIMM3), 0 } },
    & ifmt_bsetm, { 0x2003 }
  },
/* tas $rn,($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', '(', OP (RMA), ')', 0 } },
    & ifmt_stcb_r, { 0x2004 }
  },
/* cache $cimm4,($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CIMM4), ',', '(', OP (RMA), ')', 0 } },
    & ifmt_pref, { 0x7004 }
  },
/* mul $rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (RM), 0 } },
    & ifmt_mov, { 0x1004 }
  },
/* mulu $rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (RM), 0 } },
    & ifmt_mov, { 0x1005 }
  },
/* mulr $rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (RM), 0 } },
    & ifmt_mov, { 0x1006 }
  },
/* mulru $rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (RM), 0 } },
    & ifmt_mov, { 0x1007 }
  },
/* madd $rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (RM), 0 } },
    & ifmt_madd, { 0xf0013004 }
  },
/* maddu $rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (RM), 0 } },
    & ifmt_madd, { 0xf0013005 }
  },
/* maddr $rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (RM), 0 } },
    & ifmt_madd, { 0xf0013006 }
  },
/* maddru $rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (RM), 0 } },
    & ifmt_madd, { 0xf0013007 }
  },
/* div $rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (RM), 0 } },
    & ifmt_mov, { 0x1008 }
  },
/* divu $rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (RM), 0 } },
    & ifmt_mov, { 0x1009 }
  },
/* dret */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_ret, { 0x7013 }
  },
/* dbreak */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_ret, { 0x7033 }
  },
/* ldz $rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (RM), 0 } },
    & ifmt_madd, { 0xf0010000 }
  },
/* abs $rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (RM), 0 } },
    & ifmt_madd, { 0xf0010003 }
  },
/* ave $rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (RM), 0 } },
    & ifmt_madd, { 0xf0010002 }
  },
/* min $rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (RM), 0 } },
    & ifmt_madd, { 0xf0010004 }
  },
/* max $rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (RM), 0 } },
    & ifmt_madd, { 0xf0010005 }
  },
/* minu $rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (RM), 0 } },
    & ifmt_madd, { 0xf0010006 }
  },
/* maxu $rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (RM), 0 } },
    & ifmt_madd, { 0xf0010007 }
  },
/* clip $rn,$cimm5 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (CIMM5), 0 } },
    & ifmt_clip, { 0xf0011000 }
  },
/* clipu $rn,$cimm5 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (CIMM5), 0 } },
    & ifmt_clip, { 0xf0011001 }
  },
/* sadd $rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (RM), 0 } },
    & ifmt_madd, { 0xf0010008 }
  },
/* ssub $rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (RM), 0 } },
    & ifmt_madd, { 0xf001000a }
  },
/* saddu $rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (RM), 0 } },
    & ifmt_madd, { 0xf0010009 }
  },
/* ssubu $rn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RN), ',', OP (RM), 0 } },
    & ifmt_madd, { 0xf001000b }
  },
/* swcp $crn,($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN), ',', '(', OP (RMA), ')', 0 } },
    & ifmt_swcp, { 0x3008 }
  },
/* lwcp $crn,($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN), ',', '(', OP (RMA), ')', 0 } },
    & ifmt_swcp, { 0x3009 }
  },
/* smcp $crn64,($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN64), ',', '(', OP (RMA), ')', 0 } },
    & ifmt_smcp, { 0x300a }
  },
/* lmcp $crn64,($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN64), ',', '(', OP (RMA), ')', 0 } },
    & ifmt_smcp, { 0x300b }
  },
/* swcpi $crn,($rma+) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN), ',', '(', OP (RMA), '+', ')', 0 } },
    & ifmt_swcp, { 0x3000 }
  },
/* lwcpi $crn,($rma+) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN), ',', '(', OP (RMA), '+', ')', 0 } },
    & ifmt_swcp, { 0x3001 }
  },
/* smcpi $crn64,($rma+) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN64), ',', '(', OP (RMA), '+', ')', 0 } },
    & ifmt_smcp, { 0x3002 }
  },
/* lmcpi $crn64,($rma+) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN64), ',', '(', OP (RMA), '+', ')', 0 } },
    & ifmt_smcp, { 0x3003 }
  },
/* swcp $crn,$sdisp16($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN), ',', OP (SDISP16), '(', OP (RMA), ')', 0 } },
    & ifmt_swcp16, { 0xf00c0000 }
  },
/* lwcp $crn,$sdisp16($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN), ',', OP (SDISP16), '(', OP (RMA), ')', 0 } },
    & ifmt_swcp16, { 0xf00d0000 }
  },
/* smcp $crn64,$sdisp16($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN64), ',', OP (SDISP16), '(', OP (RMA), ')', 0 } },
    & ifmt_smcp16, { 0xf00e0000 }
  },
/* lmcp $crn64,$sdisp16($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN64), ',', OP (SDISP16), '(', OP (RMA), ')', 0 } },
    & ifmt_smcp16, { 0xf00f0000 }
  },
/* sbcpa $crn,($rma+),$cdisp10 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN), ',', '(', OP (RMA), '+', ')', ',', OP (CDISP10), 0 } },
    & ifmt_lbucpa, { 0xf0050000 }
  },
/* lbcpa $crn,($rma+),$cdisp10 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN), ',', '(', OP (RMA), '+', ')', ',', OP (CDISP10), 0 } },
    & ifmt_lbucpa, { 0xf0054000 }
  },
/* shcpa $crn,($rma+),$cdisp10a2 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN), ',', '(', OP (RMA), '+', ')', ',', OP (CDISP10A2), 0 } },
    & ifmt_lhucpa, { 0xf0051000 }
  },
/* lhcpa $crn,($rma+),$cdisp10a2 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN), ',', '(', OP (RMA), '+', ')', ',', OP (CDISP10A2), 0 } },
    & ifmt_lhucpa, { 0xf0055000 }
  },
/* swcpa $crn,($rma+),$cdisp10a4 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN), ',', '(', OP (RMA), '+', ')', ',', OP (CDISP10A4), 0 } },
    & ifmt_swcpa, { 0xf0052000 }
  },
/* lwcpa $crn,($rma+),$cdisp10a4 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN), ',', '(', OP (RMA), '+', ')', ',', OP (CDISP10A4), 0 } },
    & ifmt_swcpa, { 0xf0056000 }
  },
/* smcpa $crn64,($rma+),$cdisp10a8 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN64), ',', '(', OP (RMA), '+', ')', ',', OP (CDISP10A8), 0 } },
    & ifmt_smcpa, { 0xf0053000 }
  },
/* lmcpa $crn64,($rma+),$cdisp10a8 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN64), ',', '(', OP (RMA), '+', ')', ',', OP (CDISP10A8), 0 } },
    & ifmt_smcpa, { 0xf0057000 }
  },
/* sbcpm0 $crn,($rma+),$cdisp10 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN), ',', '(', OP (RMA), '+', ')', ',', OP (CDISP10), 0 } },
    & ifmt_lbucpa, { 0xf0050800 }
  },
/* lbcpm0 $crn,($rma+),$cdisp10 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN), ',', '(', OP (RMA), '+', ')', ',', OP (CDISP10), 0 } },
    & ifmt_lbucpa, { 0xf0054800 }
  },
/* shcpm0 $crn,($rma+),$cdisp10a2 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN), ',', '(', OP (RMA), '+', ')', ',', OP (CDISP10A2), 0 } },
    & ifmt_lhucpa, { 0xf0051800 }
  },
/* lhcpm0 $crn,($rma+),$cdisp10a2 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN), ',', '(', OP (RMA), '+', ')', ',', OP (CDISP10A2), 0 } },
    & ifmt_lhucpa, { 0xf0055800 }
  },
/* swcpm0 $crn,($rma+),$cdisp10a4 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN), ',', '(', OP (RMA), '+', ')', ',', OP (CDISP10A4), 0 } },
    & ifmt_swcpa, { 0xf0052800 }
  },
/* lwcpm0 $crn,($rma+),$cdisp10a4 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN), ',', '(', OP (RMA), '+', ')', ',', OP (CDISP10A4), 0 } },
    & ifmt_swcpa, { 0xf0056800 }
  },
/* smcpm0 $crn64,($rma+),$cdisp10a8 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN64), ',', '(', OP (RMA), '+', ')', ',', OP (CDISP10A8), 0 } },
    & ifmt_smcpa, { 0xf0053800 }
  },
/* lmcpm0 $crn64,($rma+),$cdisp10a8 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN64), ',', '(', OP (RMA), '+', ')', ',', OP (CDISP10A8), 0 } },
    & ifmt_smcpa, { 0xf0057800 }
  },
/* sbcpm1 $crn,($rma+),$cdisp10 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN), ',', '(', OP (RMA), '+', ')', ',', OP (CDISP10), 0 } },
    & ifmt_lbucpa, { 0xf0050c00 }
  },
/* lbcpm1 $crn,($rma+),$cdisp10 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN), ',', '(', OP (RMA), '+', ')', ',', OP (CDISP10), 0 } },
    & ifmt_lbucpa, { 0xf0054c00 }
  },
/* shcpm1 $crn,($rma+),$cdisp10a2 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN), ',', '(', OP (RMA), '+', ')', ',', OP (CDISP10A2), 0 } },
    & ifmt_lhucpa, { 0xf0051c00 }
  },
/* lhcpm1 $crn,($rma+),$cdisp10a2 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN), ',', '(', OP (RMA), '+', ')', ',', OP (CDISP10A2), 0 } },
    & ifmt_lhucpa, { 0xf0055c00 }
  },
/* swcpm1 $crn,($rma+),$cdisp10a4 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN), ',', '(', OP (RMA), '+', ')', ',', OP (CDISP10A4), 0 } },
    & ifmt_swcpa, { 0xf0052c00 }
  },
/* lwcpm1 $crn,($rma+),$cdisp10a4 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN), ',', '(', OP (RMA), '+', ')', ',', OP (CDISP10A4), 0 } },
    & ifmt_swcpa, { 0xf0056c00 }
  },
/* smcpm1 $crn64,($rma+),$cdisp10a8 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN64), ',', '(', OP (RMA), '+', ')', ',', OP (CDISP10A8), 0 } },
    & ifmt_smcpa, { 0xf0053c00 }
  },
/* lmcpm1 $crn64,($rma+),$cdisp10a8 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN64), ',', '(', OP (RMA), '+', ')', ',', OP (CDISP10A8), 0 } },
    & ifmt_smcpa, { 0xf0057c00 }
  },
/* bcpeq $cccc,$pcrel17a2 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CCCC), ',', OP (PCREL17A2), 0 } },
    & ifmt_bcpeq, { 0xd8040000 }
  },
/* bcpne $cccc,$pcrel17a2 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CCCC), ',', OP (PCREL17A2), 0 } },
    & ifmt_bcpeq, { 0xd8050000 }
  },
/* bcpat $cccc,$pcrel17a2 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CCCC), ',', OP (PCREL17A2), 0 } },
    & ifmt_bcpeq, { 0xd8060000 }
  },
/* bcpaf $cccc,$pcrel17a2 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CCCC), ',', OP (PCREL17A2), 0 } },
    & ifmt_bcpeq, { 0xd8070000 }
  },
/* synccp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_ret, { 0x7021 }
  },
/* jsrv $rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RM), 0 } },
    & ifmt_jmp, { 0x180f }
  },
/* bsrv $pcrel24a2 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (PCREL24A2), 0 } },
    & ifmt_bsr24, { 0xd80b0000 }
  },
/* --syscall-- */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_sim_syscall, { 0x7800 }
  },
/* --reserved-- */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_mov, { 0x6 }
  },
/* --reserved-- */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_mov, { 0x100a }
  },
/* --reserved-- */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_mov, { 0x100b }
  },
/* --reserved-- */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_mov, { 0x2005 }
  },
/* --reserved-- */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_mov, { 0x2008 }
  },
/* --reserved-- */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_mov, { 0x2009 }
  },
/* --reserved-- */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_mov, { 0x200a }
  },
/* --reserved-- */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_mov, { 0x200b }
  },
/* --reserved-- */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_mov, { 0x3004 }
  },
/* --reserved-- */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_mov, { 0x3005 }
  },
/* --reserved-- */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_mov, { 0x3006 }
  },
/* --reserved-- */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_mov, { 0x3007 }
  },
/* --reserved-- */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_mov, { 0x300c }
  },
/* --reserved-- */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_mov, { 0x300d }
  },
/* --reserved-- */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_mov, { 0x300e }
  },
/* --reserved-- */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_mov, { 0x300f }
  },
/* --reserved-- */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_mov, { 0x7007 }
  },
/* --reserved-- */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_mov, { 0x700e }
  },
/* --reserved-- */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_mov, { 0x700f }
  },
/* --reserved-- */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_mov, { 0xc007 }
  },
/* --reserved-- */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_mov, { 0xe00d }
  },
/* --reserved-- */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_mov, { 0xf008 }
  },
/* cmov $crnx64,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRNX64), ',', OP (RM), 0 } },
    & ifmt_cmov_crn_rm, { 0xf007f000 }
  },
/* cmov $rm,$crnx64 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RM), ',', OP (CRNX64), 0 } },
    & ifmt_cmov_crn_rm, { 0xf007f001 }
  },
/* cmovc $ivc2c3ccrn,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (IVC2C3CCRN), ',', OP (RM), 0 } },
    & ifmt_cmovc_ccrn_rm, { 0xf007f002 }
  },
/* cmovc $rm,$ivc2c3ccrn */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RM), ',', OP (IVC2C3CCRN), 0 } },
    & ifmt_cmovc_ccrn_rm, { 0xf007f003 }
  },
/* cmovh $crnx64,$rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRNX64), ',', OP (RM), 0 } },
    & ifmt_cmov_crn_rm, { 0xf007f100 }
  },
/* cmovh $rm,$crnx64 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RM), ',', OP (CRNX64), 0 } },
    & ifmt_cmov_crn_rm, { 0xf007f101 }
  },
/* cmov $ivc2crn,$ivc2rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (IVC2CRN), ',', OP (IVC2RM), 0 } },
    & ifmt_cmov_crn_rm_p0, { 0xf00000 }
  },
/* cmov $ivc2rm,$ivc2crn */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (IVC2RM), ',', OP (IVC2CRN), 0 } },
    & ifmt_cmov_crn_rm_p0, { 0xf00100 }
  },
/* cmovc $ivc2ccrn,$ivc2rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (IVC2CCRN), ',', OP (IVC2RM), 0 } },
    & ifmt_cmovc_ccrn_rm_p0, { 0xf00200 }
  },
/* cmovc $ivc2rm,$ivc2ccrn */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (IVC2RM), ',', OP (IVC2CCRN), 0 } },
    & ifmt_cmovc_ccrn_rm_p0, { 0xf00300 }
  },
/* cmovh $ivc2crn,$ivc2rm */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (IVC2CRN), ',', OP (IVC2RM), 0 } },
    & ifmt_cmov_crn_rm_p0, { 0xf10000 }
  },
/* cmovh $ivc2rm,$ivc2crn */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (IVC2RM), ',', OP (IVC2CRN), 0 } },
    & ifmt_cmov_crn_rm_p0, { 0xf10100 }
  },
/* cpadd3.b $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf0070000 }
  },
/* cpadd3.h $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf2070000 }
  },
/* cpadd3.w $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf4070000 }
  },
/* cdadd3 $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf6070000 }
  },
/* cpsub3.b $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf8070000 }
  },
/* cpsub3.h $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xfa070000 }
  },
/* cpsub3.w $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xfc070000 }
  },
/* cdsub3 $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xfe070000 }
  },
/* cpand3 $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf0070800 }
  },
/* cpor3 $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf2070800 }
  },
/* cpnor3 $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf4070800 }
  },
/* cpxor3 $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf6070800 }
  },
/* cpsel $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf8070800 }
  },
/* cpfsftbi $croc,$crqc,$crpc,$imm3p4 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), ',', OP (IMM3P4), 0 } },
    & ifmt_cpfsftbi_C3, { 0xf007e800 }
  },
/* cpfsftbs0 $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xfc070800 }
  },
/* cpfsftbs1 $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xfe070800 }
  },
/* cpunpacku.b $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf0071000 }
  },
/* cpunpacku.h $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf2071000 }
  },
/* cpunpacku.w $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf4071000 }
  },
/* cpunpackl.b $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf8071000 }
  },
/* cpunpackl.h $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xfa071000 }
  },
/* cpunpackl.w $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xfc071000 }
  },
/* cppacku.b $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf8071800 }
  },
/* cppack.b $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xfa071800 }
  },
/* cppack.h $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xfe071800 }
  },
/* cpsrl3.b $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf0072000 }
  },
/* cpssrl3.b $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf2072000 }
  },
/* cpsrl3.h $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf4072000 }
  },
/* cpssrl3.h $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf6072000 }
  },
/* cpsrl3.w $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf8072000 }
  },
/* cpssrl3.w $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xfa072000 }
  },
/* cdsrl3 $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xfc072000 }
  },
/* cpsra3.b $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf0072800 }
  },
/* cpssra3.b $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf2072800 }
  },
/* cpsra3.h $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf4072800 }
  },
/* cpssra3.h $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf6072800 }
  },
/* cpsra3.w $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf8072800 }
  },
/* cpssra3.w $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xfa072800 }
  },
/* cdsra3 $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xfc072800 }
  },
/* cpsll3.b $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf0073000 }
  },
/* cpssll3.b $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf2073000 }
  },
/* cpsll3.h $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf4073000 }
  },
/* cpssll3.h $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf6073000 }
  },
/* cpsll3.w $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf8073000 }
  },
/* cpssll3.w $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xfa073000 }
  },
/* cdsll3 $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xfc073000 }
  },
/* cpsla3.h $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf4073800 }
  },
/* cpsla3.w $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf8073800 }
  },
/* cpsadd3.h $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf4074000 }
  },
/* cpsadd3.w $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf6074000 }
  },
/* cpssub3.h $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xfc074000 }
  },
/* cpssub3.w $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xfe074000 }
  },
/* cpextuaddu3.b $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf0074800 }
  },
/* cpextuadd3.b $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf2074800 }
  },
/* cpextladdu3.b $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf4074800 }
  },
/* cpextladd3.b $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf6074800 }
  },
/* cpextusubu3.b $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf8074800 }
  },
/* cpextusub3.b $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xfa074800 }
  },
/* cpextlsubu3.b $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xfc074800 }
  },
/* cpextlsub3.b $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xfe074800 }
  },
/* cpaveu3.b $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf0075000 }
  },
/* cpave3.b $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf2075000 }
  },
/* cpave3.h $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf4075000 }
  },
/* cpave3.w $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf6075000 }
  },
/* cpaddsru3.b $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf8075000 }
  },
/* cpaddsr3.b $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xfa075000 }
  },
/* cpaddsr3.h $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xfc075000 }
  },
/* cpaddsr3.w $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xfe075000 }
  },
/* cpabsu3.b $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf0075800 }
  },
/* cpabs3.b $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf2075800 }
  },
/* cpabs3.h $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf4075800 }
  },
/* cpmaxu3.b $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf0076000 }
  },
/* cpmax3.b $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf2076000 }
  },
/* cpmax3.h $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf6076000 }
  },
/* cpmaxu3.w $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf8076000 }
  },
/* cpmax3.w $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xfa076000 }
  },
/* cpminu3.b $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf0076800 }
  },
/* cpmin3.b $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf2076800 }
  },
/* cpmin3.h $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf6076800 }
  },
/* cpminu3.w $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xf8076800 }
  },
/* cpmin3.w $croc,$crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpadd3_b_C3, { 0xfa076800 }
  },
/* cpmovfrcsar0 $croc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), 0 } },
    & ifmt_cpmovfrcsar0_C3, { 0xf0078000 }
  },
/* cpmovfrcsar1 $croc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), 0 } },
    & ifmt_cpmovfrcsar0_C3, { 0xf007801e }
  },
/* cpmovfrcc $croc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), 0 } },
    & ifmt_cpmovfrcsar0_C3, { 0xf0078002 }
  },
/* cpmovtocsar0 $crqc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), 0 } },
    & ifmt_cpmovtocsar0_C3, { 0xf0078020 }
  },
/* cpmovtocsar1 $crqc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), 0 } },
    & ifmt_cpmovtocsar0_C3, { 0xf007803e }
  },
/* cpmovtocc $crqc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), 0 } },
    & ifmt_cpmovtocsar0_C3, { 0xf0078022 }
  },
/* cpmov $croc,$crqc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), 0 } },
    & ifmt_cpmov_C3, { 0xf0078800 }
  },
/* cpabsz.b $croc,$crqc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), 0 } },
    & ifmt_cpmov_C3, { 0xf0078802 }
  },
/* cpabsz.h $croc,$crqc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), 0 } },
    & ifmt_cpmov_C3, { 0xf0078804 }
  },
/* cpabsz.w $croc,$crqc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), 0 } },
    & ifmt_cpmov_C3, { 0xf0078806 }
  },
/* cpldz.h $croc,$crqc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), 0 } },
    & ifmt_cpmov_C3, { 0xf0078808 }
  },
/* cpldz.w $croc,$crqc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), 0 } },
    & ifmt_cpmov_C3, { 0xf007880a }
  },
/* cpnorm.h $croc,$crqc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), 0 } },
    & ifmt_cpmov_C3, { 0xf007880c }
  },
/* cpnorm.w $croc,$crqc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), 0 } },
    & ifmt_cpmov_C3, { 0xf007880e }
  },
/* cphaddu.b $croc,$crqc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), 0 } },
    & ifmt_cpmov_C3, { 0xf0078810 }
  },
/* cphadd.b $croc,$crqc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), 0 } },
    & ifmt_cpmov_C3, { 0xf0078812 }
  },
/* cphadd.h $croc,$crqc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), 0 } },
    & ifmt_cpmov_C3, { 0xf0078814 }
  },
/* cphadd.w $croc,$crqc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), 0 } },
    & ifmt_cpmov_C3, { 0xf0078816 }
  },
/* cpccadd.b $crqc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), 0 } },
    & ifmt_cpmov_C3, { 0xf0078818 }
  },
/* cpbcast.b $croc,$crqc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), 0 } },
    & ifmt_cpmov_C3, { 0xf007881a }
  },
/* cpbcast.h $croc,$crqc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), 0 } },
    & ifmt_cpmov_C3, { 0xf007881c }
  },
/* cpbcast.w $croc,$crqc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), 0 } },
    & ifmt_cpmov_C3, { 0xf007881e }
  },
/* cpextuu.b $croc,$crqc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), 0 } },
    & ifmt_cpmov_C3, { 0xf0078820 }
  },
/* cpextu.b $croc,$crqc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), 0 } },
    & ifmt_cpmov_C3, { 0xf0078822 }
  },
/* cpextuu.h $croc,$crqc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), 0 } },
    & ifmt_cpmov_C3, { 0xf0078824 }
  },
/* cpextu.h $croc,$crqc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), 0 } },
    & ifmt_cpmov_C3, { 0xf0078826 }
  },
/* cpextlu.b $croc,$crqc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), 0 } },
    & ifmt_cpmov_C3, { 0xf0078828 }
  },
/* cpextl.b $croc,$crqc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), 0 } },
    & ifmt_cpmov_C3, { 0xf007882a }
  },
/* cpextlu.h $croc,$crqc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), 0 } },
    & ifmt_cpmov_C3, { 0xf007882c }
  },
/* cpextl.h $croc,$crqc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), 0 } },
    & ifmt_cpmov_C3, { 0xf007882e }
  },
/* cpcastub.h $croc,$crqc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), 0 } },
    & ifmt_cpmov_C3, { 0xf0078830 }
  },
/* cpcastb.h $croc,$crqc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), 0 } },
    & ifmt_cpmov_C3, { 0xf0078832 }
  },
/* cpcastub.w $croc,$crqc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), 0 } },
    & ifmt_cpmov_C3, { 0xf0078838 }
  },
/* cpcastb.w $croc,$crqc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), 0 } },
    & ifmt_cpmov_C3, { 0xf007883a }
  },
/* cpcastuh.w $croc,$crqc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), 0 } },
    & ifmt_cpmov_C3, { 0xf007883c }
  },
/* cpcasth.w $croc,$crqc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), 0 } },
    & ifmt_cpmov_C3, { 0xf007883e }
  },
/* cdcastuw $croc,$crqc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), 0 } },
    & ifmt_cpmov_C3, { 0xf0078834 }
  },
/* cdcastw $croc,$crqc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), ',', OP (CRQC), 0 } },
    & ifmt_cpmov_C3, { 0xf0078836 }
  },
/* cpcmpeqz.b $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf0079000 }
  },
/* cpcmpeq.b $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf0179000 }
  },
/* cpcmpeq.h $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf0379000 }
  },
/* cpcmpeq.w $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf0579000 }
  },
/* cpcmpne.b $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf0979000 }
  },
/* cpcmpne.h $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf0b79000 }
  },
/* cpcmpne.w $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf0d79000 }
  },
/* cpcmpgtu.b $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf1079000 }
  },
/* cpcmpgt.b $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf1179000 }
  },
/* cpcmpgt.h $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf1379000 }
  },
/* cpcmpgtu.w $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf1479000 }
  },
/* cpcmpgt.w $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf1579000 }
  },
/* cpcmpgeu.b $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf1879000 }
  },
/* cpcmpge.b $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf1979000 }
  },
/* cpcmpge.h $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf1b79000 }
  },
/* cpcmpgeu.w $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf1c79000 }
  },
/* cpcmpge.w $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf1d79000 }
  },
/* cpacmpeq.b $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf2179000 }
  },
/* cpacmpeq.h $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf2379000 }
  },
/* cpacmpeq.w $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf2579000 }
  },
/* cpacmpne.b $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf2979000 }
  },
/* cpacmpne.h $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf2b79000 }
  },
/* cpacmpne.w $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf2d79000 }
  },
/* cpacmpgtu.b $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf3079000 }
  },
/* cpacmpgt.b $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf3179000 }
  },
/* cpacmpgt.h $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf3379000 }
  },
/* cpacmpgtu.w $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf3479000 }
  },
/* cpacmpgt.w $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf3579000 }
  },
/* cpacmpgeu.b $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf3879000 }
  },
/* cpacmpge.b $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf3979000 }
  },
/* cpacmpge.h $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf3b79000 }
  },
/* cpacmpgeu.w $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf3c79000 }
  },
/* cpacmpge.w $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf3d79000 }
  },
/* cpocmpeq.b $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf4179000 }
  },
/* cpocmpeq.h $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf4379000 }
  },
/* cpocmpeq.w $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf4579000 }
  },
/* cpocmpne.b $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf4979000 }
  },
/* cpocmpne.h $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf4b79000 }
  },
/* cpocmpne.w $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf4d79000 }
  },
/* cpocmpgtu.b $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf5079000 }
  },
/* cpocmpgt.b $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf5179000 }
  },
/* cpocmpgt.h $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf5379000 }
  },
/* cpocmpgtu.w $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf5479000 }
  },
/* cpocmpgt.w $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf5579000 }
  },
/* cpocmpgeu.b $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf5879000 }
  },
/* cpocmpge.b $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf5979000 }
  },
/* cpocmpge.h $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf5b79000 }
  },
/* cpocmpgeu.w $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf5c79000 }
  },
/* cpocmpge.w $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf5d79000 }
  },
/* cpsrli3.b $crqc,$crpc,$imm3p9 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), ',', OP (IMM3P9), 0 } },
    & ifmt_cpsrli3_b_C3, { 0xf007a000 }
  },
/* cpsrli3.h $crqc,$crpc,$imm4p8 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), ',', OP (IMM4P8), 0 } },
    & ifmt_cpsrli3_h_C3, { 0xf407a000 }
  },
/* cpsrli3.w $crqc,$crpc,$imm5p7 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), ',', OP (IMM5P7), 0 } },
    & ifmt_cpsrli3_w_C3, { 0xf807a000 }
  },
/* cdsrli3 $crqc,$crpc,$imm6p6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), ',', OP (IMM6P6), 0 } },
    & ifmt_cdsrli3_C3, { 0xfc07a000 }
  },
/* cpsrai3.b $crqc,$crpc,$imm3p9 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), ',', OP (IMM3P9), 0 } },
    & ifmt_cpsrli3_b_C3, { 0xf007a800 }
  },
/* cpsrai3.h $crqc,$crpc,$imm4p8 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), ',', OP (IMM4P8), 0 } },
    & ifmt_cpsrli3_h_C3, { 0xf407a800 }
  },
/* cpsrai3.w $crqc,$crpc,$imm5p7 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), ',', OP (IMM5P7), 0 } },
    & ifmt_cpsrli3_w_C3, { 0xf807a800 }
  },
/* cdsrai3 $crqc,$crpc,$imm6p6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), ',', OP (IMM6P6), 0 } },
    & ifmt_cdsrli3_C3, { 0xfc07a800 }
  },
/* cpslli3.b $crqc,$crpc,$imm3p9 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), ',', OP (IMM3P9), 0 } },
    & ifmt_cpsrli3_b_C3, { 0xf007b000 }
  },
/* cpslli3.h $crqc,$crpc,$imm4p8 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), ',', OP (IMM4P8), 0 } },
    & ifmt_cpsrli3_h_C3, { 0xf407b000 }
  },
/* cpslli3.w $crqc,$crpc,$imm5p7 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), ',', OP (IMM5P7), 0 } },
    & ifmt_cpsrli3_w_C3, { 0xf807b000 }
  },
/* cdslli3 $crqc,$crpc,$imm6p6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), ',', OP (IMM6P6), 0 } },
    & ifmt_cdsrli3_C3, { 0xfc07b000 }
  },
/* cpslai3.h $crqc,$crpc,$imm4p8 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), ',', OP (IMM4P8), 0 } },
    & ifmt_cpsrli3_h_C3, { 0xf407b800 }
  },
/* cpslai3.w $crqc,$crpc,$imm5p7 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), ',', OP (IMM5P7), 0 } },
    & ifmt_cpsrli3_w_C3, { 0xf807b800 }
  },
/* cpclipiu3.w $crqc,$crpc,$imm5p7 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), ',', OP (IMM5P7), 0 } },
    & ifmt_cpsrli3_w_C3, { 0xf007c000 }
  },
/* cpclipi3.w $crqc,$crpc,$imm5p7 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), ',', OP (IMM5P7), 0 } },
    & ifmt_cpsrli3_w_C3, { 0xf407c000 }
  },
/* cdclipiu3 $crqc,$crpc,$imm6p6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), ',', OP (IMM6P6), 0 } },
    & ifmt_cdsrli3_C3, { 0xf807c000 }
  },
/* cdclipi3 $crqc,$crpc,$imm6p6 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), ',', OP (IMM6P6), 0 } },
    & ifmt_cdsrli3_C3, { 0xfc07c000 }
  },
/* cpmovi.b $crqc,$simm8p4 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (SIMM8P4), 0 } },
    & ifmt_cpmovi_b_C3, { 0xf007c800 }
  },
/* cpmoviu.h $crqc,$imm8p4 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (IMM8P4), 0 } },
    & ifmt_cpmoviu_h_C3, { 0xf007c804 }
  },
/* cpmovi.h $crqc,$simm8p4 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (SIMM8P4), 0 } },
    & ifmt_cpmovi_b_C3, { 0xf007c806 }
  },
/* cpmoviu.w $crqc,$imm8p4 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (IMM8P4), 0 } },
    & ifmt_cpmoviu_h_C3, { 0xf007c808 }
  },
/* cpmovi.w $crqc,$simm8p4 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (SIMM8P4), 0 } },
    & ifmt_cpmovi_b_C3, { 0xf007c80a }
  },
/* cdmoviu $crqc,$imm8p4 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (IMM8P4), 0 } },
    & ifmt_cpmoviu_h_C3, { 0xf007c80c }
  },
/* cdmovi $crqc,$simm8p4 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (SIMM8P4), 0 } },
    & ifmt_cpmovi_b_C3, { 0xf007c80e }
  },
/* cpadda1u.b $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf0070001 }
  },
/* cpadda1.b $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf0170001 }
  },
/* cpaddua1.h $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf0270001 }
  },
/* cpaddla1.h $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf0370001 }
  },
/* cpaddaca1u.b $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf0470001 }
  },
/* cpaddaca1.b $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf0570001 }
  },
/* cpaddacua1.h $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf0670001 }
  },
/* cpaddacla1.h $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf0770001 }
  },
/* cpsuba1u.b $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf0870001 }
  },
/* cpsuba1.b $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf0970001 }
  },
/* cpsubua1.h $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf0a70001 }
  },
/* cpsubla1.h $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf0b70001 }
  },
/* cpsubaca1u.b $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf0c70001 }
  },
/* cpsubaca1.b $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf0d70001 }
  },
/* cpsubacua1.h $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf0e70001 }
  },
/* cpsubacla1.h $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf0f70001 }
  },
/* cpabsa1u.b $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf1070001 }
  },
/* cpabsa1.b $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf1170001 }
  },
/* cpabsua1.h $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf1270001 }
  },
/* cpabsla1.h $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf1370001 }
  },
/* cpsada1u.b $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf1470001 }
  },
/* cpsada1.b $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf1570001 }
  },
/* cpsadua1.h $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf1670001 }
  },
/* cpsadla1.h $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf1770001 }
  },
/* cpseta1.h $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf2070001 }
  },
/* cpsetua1.w $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf2270001 }
  },
/* cpsetla1.w $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf2370001 }
  },
/* cpmova1.b $croc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), 0 } },
    & ifmt_cpmovfrcsar0_C3, { 0xf0072001 }
  },
/* cpmovua1.h $croc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), 0 } },
    & ifmt_cpmovfrcsar0_C3, { 0xf0072005 }
  },
/* cpmovla1.h $croc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), 0 } },
    & ifmt_cpmovfrcsar0_C3, { 0xf0072007 }
  },
/* cpmovuua1.w $croc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), 0 } },
    & ifmt_cpmovfrcsar0_C3, { 0xf0072009 }
  },
/* cpmovula1.w $croc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), 0 } },
    & ifmt_cpmovfrcsar0_C3, { 0xf007200b }
  },
/* cpmovlua1.w $croc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), 0 } },
    & ifmt_cpmovfrcsar0_C3, { 0xf007200d }
  },
/* cpmovlla1.w $croc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), 0 } },
    & ifmt_cpmovfrcsar0_C3, { 0xf007200f }
  },
/* cppacka1u.b $croc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), 0 } },
    & ifmt_cpmovfrcsar0_C3, { 0xf0072021 }
  },
/* cppacka1.b $croc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), 0 } },
    & ifmt_cpmovfrcsar0_C3, { 0xf0072023 }
  },
/* cppackua1.h $croc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), 0 } },
    & ifmt_cpmovfrcsar0_C3, { 0xf0072025 }
  },
/* cppackla1.h $croc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), 0 } },
    & ifmt_cpmovfrcsar0_C3, { 0xf0072027 }
  },
/* cppackua1.w $croc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), 0 } },
    & ifmt_cpmovfrcsar0_C3, { 0xf0072029 }
  },
/* cppackla1.w $croc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), 0 } },
    & ifmt_cpmovfrcsar0_C3, { 0xf007202b }
  },
/* cpmovhua1.w $croc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), 0 } },
    & ifmt_cpmovfrcsar0_C3, { 0xf007202d }
  },
/* cpmovhla1.w $croc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROC), 0 } },
    & ifmt_cpmovfrcsar0_C3, { 0xf007202f }
  },
/* cpsrla1 $crqc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), 0 } },
    & ifmt_cpmovtocsar0_C3, { 0xf0071001 }
  },
/* cpsraa1 $crqc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), 0 } },
    & ifmt_cpmovtocsar0_C3, { 0xf0171001 }
  },
/* cpslla1 $crqc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), 0 } },
    & ifmt_cpmovtocsar0_C3, { 0xf0271001 }
  },
/* cpsrlia1 $imm5p7 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (IMM5P7), 0 } },
    & ifmt_cpsrlia1_P1, { 0xf0071801 }
  },
/* cpsraia1 $imm5p7 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (IMM5P7), 0 } },
    & ifmt_cpsrlia1_P1, { 0xf4071801 }
  },
/* cpsllia1 $imm5p7 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (IMM5P7), 0 } },
    & ifmt_cpsrlia1_P1, { 0xf8071801 }
  },
/* cpssqa1u.b $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf0070801 }
  },
/* cpssqa1.b $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf0170801 }
  },
/* cpssda1u.b $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf0470801 }
  },
/* cpssda1.b $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf0570801 }
  },
/* cpmula1u.b $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf0870801 }
  },
/* cpmula1.b $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf0970801 }
  },
/* cpmulua1.h $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf0a70801 }
  },
/* cpmulla1.h $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf0b70801 }
  },
/* cpmulua1u.w $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf0c70801 }
  },
/* cpmulla1u.w $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf0d70801 }
  },
/* cpmulua1.w $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf0e70801 }
  },
/* cpmulla1.w $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf0f70801 }
  },
/* cpmada1u.b $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf1070801 }
  },
/* cpmada1.b $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf1170801 }
  },
/* cpmadua1.h $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf1270801 }
  },
/* cpmadla1.h $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf1370801 }
  },
/* cpmadua1u.w $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf1470801 }
  },
/* cpmadla1u.w $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf1570801 }
  },
/* cpmadua1.w $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf1670801 }
  },
/* cpmadla1.w $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf1770801 }
  },
/* cpmsbua1.h $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf1a70801 }
  },
/* cpmsbla1.h $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf1b70801 }
  },
/* cpmsbua1u.w $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf1c70801 }
  },
/* cpmsbla1u.w $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf1d70801 }
  },
/* cpmsbua1.w $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf1e70801 }
  },
/* cpmsbla1.w $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf1f70801 }
  },
/* cpsmadua1.h $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf3270801 }
  },
/* cpsmadla1.h $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf3370801 }
  },
/* cpsmadua1.w $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf3670801 }
  },
/* cpsmadla1.w $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf3770801 }
  },
/* cpsmsbua1.h $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf3a70801 }
  },
/* cpsmsbla1.h $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf3b70801 }
  },
/* cpsmsbua1.w $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf3e70801 }
  },
/* cpsmsbla1.w $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf3f70801 }
  },
/* cpmulslua1.h $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf4a70801 }
  },
/* cpmulslla1.h $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf4b70801 }
  },
/* cpmulslua1.w $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf4e70801 }
  },
/* cpmulslla1.w $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf4f70801 }
  },
/* cpsmadslua1.h $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf7270801 }
  },
/* cpsmadslla1.h $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf7370801 }
  },
/* cpsmadslua1.w $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf7670801 }
  },
/* cpsmadslla1.w $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf7770801 }
  },
/* cpsmsbslua1.h $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf7a70801 }
  },
/* cpsmsbslla1.h $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf7b70801 }
  },
/* cpsmsbslua1.w $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf7e70801 }
  },
/* cpsmsbslla1.w $crqc,$crpc */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQC), ',', OP (CRPC), 0 } },
    & ifmt_cpcmpeqz_b_C3, { 0xf7f70801 }
  },
/* c0nop */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_c0nop_P0_P0S, { 0x0 }
  },
/* cpadd3.b $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x80000 }
  },
/* cpadd3.h $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x100000 }
  },
/* cpadd3.w $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x180000 }
  },
/* cpunpacku.b $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x280000 }
  },
/* cpunpacku.h $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x300000 }
  },
/* cpunpacku.w $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x380000 }
  },
/* cpunpackl.b $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x480000 }
  },
/* cpunpackl.h $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x500000 }
  },
/* cpunpackl.w $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x580000 }
  },
/* cpsel $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x200000 }
  },
/* cpfsftbs0 $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x600000 }
  },
/* cpfsftbs1 $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x680000 }
  },
/* cpmov $crop,$crqp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), 0 } },
    & ifmt_cpmov_P0S_P1, { 0x800000 }
  },
/* cpabsz.b $crop,$crqp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), 0 } },
    & ifmt_cpmov_P0S_P1, { 0x800200 }
  },
/* cpabsz.h $crop,$crqp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), 0 } },
    & ifmt_cpmov_P0S_P1, { 0x800400 }
  },
/* cpabsz.w $crop,$crqp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), 0 } },
    & ifmt_cpmov_P0S_P1, { 0x800600 }
  },
/* cpldz.h $crop,$crqp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), 0 } },
    & ifmt_cpmov_P0S_P1, { 0x800800 }
  },
/* cpldz.w $crop,$crqp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), 0 } },
    & ifmt_cpmov_P0S_P1, { 0x800a00 }
  },
/* cpnorm.h $crop,$crqp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), 0 } },
    & ifmt_cpmov_P0S_P1, { 0x800c00 }
  },
/* cpnorm.w $crop,$crqp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), 0 } },
    & ifmt_cpmov_P0S_P1, { 0x800e00 }
  },
/* cphaddu.b $crop,$crqp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), 0 } },
    & ifmt_cpmov_P0S_P1, { 0x801000 }
  },
/* cphadd.b $crop,$crqp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), 0 } },
    & ifmt_cpmov_P0S_P1, { 0x801200 }
  },
/* cphadd.h $crop,$crqp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), 0 } },
    & ifmt_cpmov_P0S_P1, { 0x801400 }
  },
/* cphadd.w $crop,$crqp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), 0 } },
    & ifmt_cpmov_P0S_P1, { 0x801600 }
  },
/* cpccadd.b $crqp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), 0 } },
    & ifmt_cpccadd_b_P0S_P1, { 0x801800 }
  },
/* cpbcast.b $crop,$crqp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), 0 } },
    & ifmt_cpmov_P0S_P1, { 0x801a00 }
  },
/* cpbcast.h $crop,$crqp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), 0 } },
    & ifmt_cpmov_P0S_P1, { 0x801c00 }
  },
/* cpbcast.w $crop,$crqp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), 0 } },
    & ifmt_cpmov_P0S_P1, { 0x801e00 }
  },
/* cpextuu.b $crop,$crqp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), 0 } },
    & ifmt_cpmov_P0S_P1, { 0x802000 }
  },
/* cpextu.b $crop,$crqp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), 0 } },
    & ifmt_cpmov_P0S_P1, { 0x802200 }
  },
/* cpextuu.h $crop,$crqp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), 0 } },
    & ifmt_cpmov_P0S_P1, { 0x802400 }
  },
/* cpextu.h $crop,$crqp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), 0 } },
    & ifmt_cpmov_P0S_P1, { 0x802600 }
  },
/* cpextlu.b $crop,$crqp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), 0 } },
    & ifmt_cpmov_P0S_P1, { 0x802800 }
  },
/* cpextl.b $crop,$crqp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), 0 } },
    & ifmt_cpmov_P0S_P1, { 0x802a00 }
  },
/* cpextlu.h $crop,$crqp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), 0 } },
    & ifmt_cpmov_P0S_P1, { 0x802c00 }
  },
/* cpextl.h $crop,$crqp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), 0 } },
    & ifmt_cpmov_P0S_P1, { 0x802e00 }
  },
/* cpcastub.h $crop,$crqp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), 0 } },
    & ifmt_cpmov_P0S_P1, { 0x803000 }
  },
/* cpcastb.h $crop,$crqp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), 0 } },
    & ifmt_cpmov_P0S_P1, { 0x803200 }
  },
/* cpcastub.w $crop,$crqp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), 0 } },
    & ifmt_cpmov_P0S_P1, { 0x803800 }
  },
/* cpcastb.w $crop,$crqp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), 0 } },
    & ifmt_cpmov_P0S_P1, { 0x803a00 }
  },
/* cpcastuh.w $crop,$crqp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), 0 } },
    & ifmt_cpmov_P0S_P1, { 0x803c00 }
  },
/* cpcasth.w $crop,$crqp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), 0 } },
    & ifmt_cpmov_P0S_P1, { 0x803e00 }
  },
/* cdcastuw $crop,$crqp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), 0 } },
    & ifmt_cpmov_P0S_P1, { 0x803400 }
  },
/* cdcastw $crop,$crqp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), 0 } },
    & ifmt_cpmov_P0S_P1, { 0x803600 }
  },
/* cpmovfrcsar0 $crop */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), 0 } },
    & ifmt_cpmovfrcsar0_P0S_P1, { 0x880000 }
  },
/* cpmovfrcsar1 $crop */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), 0 } },
    & ifmt_cpmovfrcsar0_P0S_P1, { 0x881e00 }
  },
/* cpmovfrcc $crop */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), 0 } },
    & ifmt_cpmovfrcsar0_P0S_P1, { 0x880200 }
  },
/* cpmovtocsar0 $crqp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), 0 } },
    & ifmt_cpccadd_b_P0S_P1, { 0x882000 }
  },
/* cpmovtocsar1 $crqp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), 0 } },
    & ifmt_cpccadd_b_P0S_P1, { 0x883e00 }
  },
/* cpmovtocc $crqp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), 0 } },
    & ifmt_cpccadd_b_P0S_P1, { 0x882200 }
  },
/* cpcmpeqz.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x900000 }
  },
/* cpcmpeq.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x900010 }
  },
/* cpcmpeq.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x900030 }
  },
/* cpcmpeq.w $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x900050 }
  },
/* cpcmpne.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x900090 }
  },
/* cpcmpne.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x9000b0 }
  },
/* cpcmpne.w $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x9000d0 }
  },
/* cpcmpgtu.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x900100 }
  },
/* cpcmpgt.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x900110 }
  },
/* cpcmpgt.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x900130 }
  },
/* cpcmpgtu.w $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x900140 }
  },
/* cpcmpgt.w $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x900150 }
  },
/* cpcmpgeu.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x900180 }
  },
/* cpcmpge.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x900190 }
  },
/* cpcmpge.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x9001b0 }
  },
/* cpcmpgeu.w $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x9001c0 }
  },
/* cpcmpge.w $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x9001d0 }
  },
/* cpadda0u.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc00000 }
  },
/* cpadda0.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc00010 }
  },
/* cpaddua0.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc00020 }
  },
/* cpaddla0.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc00030 }
  },
/* cpaddaca0u.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc00040 }
  },
/* cpaddaca0.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc00050 }
  },
/* cpaddacua0.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc00060 }
  },
/* cpaddacla0.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc00070 }
  },
/* cpsuba0u.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc00080 }
  },
/* cpsuba0.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc00090 }
  },
/* cpsubua0.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc000a0 }
  },
/* cpsubla0.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc000b0 }
  },
/* cpsubaca0u.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc000c0 }
  },
/* cpsubaca0.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc000d0 }
  },
/* cpsubacua0.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc000e0 }
  },
/* cpsubacla0.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc000f0 }
  },
/* cpabsa0u.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc00100 }
  },
/* cpabsa0.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc00110 }
  },
/* cpabsua0.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc00120 }
  },
/* cpabsla0.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc00130 }
  },
/* cpsada0u.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc00140 }
  },
/* cpsada0.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc00150 }
  },
/* cpsadua0.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc00160 }
  },
/* cpsadla0.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc00170 }
  },
/* cpseta0.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc001b0 }
  },
/* cpsetua0.w $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc001c0 }
  },
/* cpsetla0.w $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc001d0 }
  },
/* cpmova0.b $crop */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), 0 } },
    & ifmt_cpmovfrcsar0_P0S_P1, { 0xc80200 }
  },
/* cpmovua0.h $crop */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), 0 } },
    & ifmt_cpmovfrcsar0_P0S_P1, { 0xc80400 }
  },
/* cpmovla0.h $crop */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), 0 } },
    & ifmt_cpmovfrcsar0_P0S_P1, { 0xc80600 }
  },
/* cpmovuua0.w $crop */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), 0 } },
    & ifmt_cpmovfrcsar0_P0S_P1, { 0xc80800 }
  },
/* cpmovula0.w $crop */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), 0 } },
    & ifmt_cpmovfrcsar0_P0S_P1, { 0xc80a00 }
  },
/* cpmovlua0.w $crop */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), 0 } },
    & ifmt_cpmovfrcsar0_P0S_P1, { 0xc80c00 }
  },
/* cpmovlla0.w $crop */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), 0 } },
    & ifmt_cpmovfrcsar0_P0S_P1, { 0xc80e00 }
  },
/* cppacka0u.b $crop */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), 0 } },
    & ifmt_cpmovfrcsar0_P0S_P1, { 0xc81000 }
  },
/* cppacka0.b $crop */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), 0 } },
    & ifmt_cpmovfrcsar0_P0S_P1, { 0xc81200 }
  },
/* cppackua0.h $crop */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), 0 } },
    & ifmt_cpmovfrcsar0_P0S_P1, { 0xc81400 }
  },
/* cppackla0.h $crop */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), 0 } },
    & ifmt_cpmovfrcsar0_P0S_P1, { 0xc81600 }
  },
/* cppackua0.w $crop */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), 0 } },
    & ifmt_cpmovfrcsar0_P0S_P1, { 0xc81800 }
  },
/* cppackla0.w $crop */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), 0 } },
    & ifmt_cpmovfrcsar0_P0S_P1, { 0xc81a00 }
  },
/* cpmovhua0.w $crop */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), 0 } },
    & ifmt_cpmovfrcsar0_P0S_P1, { 0xc81c00 }
  },
/* cpmovhla0.w $crop */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), 0 } },
    & ifmt_cpmovfrcsar0_P0S_P1, { 0xc81e00 }
  },
/* cpacsuma0 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_c0nop_P0_P0S, { 0xc82000 }
  },
/* cpaccpa0 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_c0nop_P0_P0S, { 0xc82200 }
  },
/* cpsrla0 $crqp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), 0 } },
    & ifmt_cpccadd_b_P0S_P1, { 0xc83000 }
  },
/* cpsraa0 $crqp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), 0 } },
    & ifmt_cpccadd_b_P0S_P1, { 0xc83200 }
  },
/* cpslla0 $crqp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), 0 } },
    & ifmt_cpccadd_b_P0S_P1, { 0xc83400 }
  },
/* cpsrlia0 $imm5p23 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (IMM5P23), 0 } },
    & ifmt_cpsrlia0_P0S, { 0xc83800 }
  },
/* cpsraia0 $imm5p23 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (IMM5P23), 0 } },
    & ifmt_cpsrlia0_P0S, { 0xc83a00 }
  },
/* cpsllia0 $imm5p23 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (IMM5P23), 0 } },
    & ifmt_cpsrlia0_P0S, { 0xc83c00 }
  },
/* cpfsftba0s0u.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xf80000 }
  },
/* cpfsftba0s0.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xf80010 }
  },
/* cpfsftbua0s0.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xf80020 }
  },
/* cpfsftbla0s0.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xf80030 }
  },
/* cpfaca0s0u.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xf80040 }
  },
/* cpfaca0s0.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xf80050 }
  },
/* cpfacua0s0.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xf80060 }
  },
/* cpfacla0s0.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xf80070 }
  },
/* cpfsftba0s1u.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xf80080 }
  },
/* cpfsftba0s1.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xf80090 }
  },
/* cpfsftbua0s1.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xf800a0 }
  },
/* cpfsftbla0s1.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xf800b0 }
  },
/* cpfaca0s1u.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xf800c0 }
  },
/* cpfaca0s1.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xf800d0 }
  },
/* cpfacua0s1.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xf800e0 }
  },
/* cpfacla0s1.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xf800f0 }
  },
/* cpfsftbi $crop,$crqp,$crpp,$imm3p5 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), ',', OP (IMM3P5), 0 } },
    & ifmt_cpfsftbi_P0_P1, { 0x400000 }
  },
/* cpacmpeq.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x980010 }
  },
/* cpacmpeq.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x980030 }
  },
/* cpacmpeq.w $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x980050 }
  },
/* cpacmpne.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x980090 }
  },
/* cpacmpne.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x9800b0 }
  },
/* cpacmpne.w $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x9800d0 }
  },
/* cpacmpgtu.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x980100 }
  },
/* cpacmpgt.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x980110 }
  },
/* cpacmpgt.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x980130 }
  },
/* cpacmpgtu.w $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x980140 }
  },
/* cpacmpgt.w $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x980150 }
  },
/* cpacmpgeu.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x980180 }
  },
/* cpacmpge.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x980190 }
  },
/* cpacmpge.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x9801b0 }
  },
/* cpacmpgeu.w $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x9801c0 }
  },
/* cpacmpge.w $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x9801d0 }
  },
/* cpocmpeq.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x1980010 }
  },
/* cpocmpeq.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x1980030 }
  },
/* cpocmpeq.w $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x1980050 }
  },
/* cpocmpne.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x1980090 }
  },
/* cpocmpne.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x19800b0 }
  },
/* cpocmpne.w $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x19800d0 }
  },
/* cpocmpgtu.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x1980100 }
  },
/* cpocmpgt.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x1980110 }
  },
/* cpocmpgt.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x1980130 }
  },
/* cpocmpgtu.w $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x1980140 }
  },
/* cpocmpgt.w $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x1980150 }
  },
/* cpocmpgeu.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x1980180 }
  },
/* cpocmpge.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x1980190 }
  },
/* cpocmpge.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x19801b0 }
  },
/* cpocmpgeu.w $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x19801c0 }
  },
/* cpocmpge.w $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x19801d0 }
  },
/* cdadd3 $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x3a00000 }
  },
/* cpsub3.b $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x4a00000 }
  },
/* cpsub3.h $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x5a00000 }
  },
/* cpsub3.w $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x6a00000 }
  },
/* cdsub3 $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x7a00000 }
  },
/* cpsadd3.h $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0xaa00000 }
  },
/* cpsadd3.w $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0xba00000 }
  },
/* cpssub3.h $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0xea00000 }
  },
/* cpssub3.w $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0xfa00000 }
  },
/* cpextuaddu3.b $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x10a00000 }
  },
/* cpextuadd3.b $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x11a00000 }
  },
/* cpextladdu3.b $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x12a00000 }
  },
/* cpextladd3.b $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x13a00000 }
  },
/* cpextusubu3.b $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x14a00000 }
  },
/* cpextusub3.b $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x15a00000 }
  },
/* cpextlsubu3.b $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x16a00000 }
  },
/* cpextlsub3.b $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x17a00000 }
  },
/* cpaveu3.b $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x18a00000 }
  },
/* cpave3.b $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x19a00000 }
  },
/* cpave3.h $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x1aa00000 }
  },
/* cpave3.w $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x1ba00000 }
  },
/* cpaddsru3.b $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x1ca00000 }
  },
/* cpaddsr3.b $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x1da00000 }
  },
/* cpaddsr3.h $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x1ea00000 }
  },
/* cpaddsr3.w $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x1fa00000 }
  },
/* cpabsu3.b $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x20a00000 }
  },
/* cpabs3.b $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x21a00000 }
  },
/* cpabs3.h $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x22a00000 }
  },
/* cpand3 $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x24a00000 }
  },
/* cpor3 $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x25a00000 }
  },
/* cpnor3 $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x26a00000 }
  },
/* cpxor3 $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x27a00000 }
  },
/* cppacku.b $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x2ca00000 }
  },
/* cppack.b $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x2da00000 }
  },
/* cppack.h $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x2fa00000 }
  },
/* cpmaxu3.b $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x30a00000 }
  },
/* cpmax3.b $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x31a00000 }
  },
/* cpmax3.h $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x33a00000 }
  },
/* cpmaxu3.w $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x34a00000 }
  },
/* cpmax3.w $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x35a00000 }
  },
/* cpminu3.b $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x38a00000 }
  },
/* cpmin3.b $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x39a00000 }
  },
/* cpmin3.h $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x3ba00000 }
  },
/* cpminu3.w $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x3ca00000 }
  },
/* cpmin3.w $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x3da00000 }
  },
/* cpsrl3.b $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x40a00000 }
  },
/* cpssrl3.b $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x41a00000 }
  },
/* cpsrl3.h $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x42a00000 }
  },
/* cpssrl3.h $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x43a00000 }
  },
/* cpsrl3.w $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x44a00000 }
  },
/* cpssrl3.w $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x45a00000 }
  },
/* cdsrl3 $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x46a00000 }
  },
/* cpsra3.b $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x48a00000 }
  },
/* cpssra3.b $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x49a00000 }
  },
/* cpsra3.h $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x4aa00000 }
  },
/* cpssra3.h $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x4ba00000 }
  },
/* cpsra3.w $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x4ca00000 }
  },
/* cpssra3.w $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x4da00000 }
  },
/* cdsra3 $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x4ea00000 }
  },
/* cpsll3.b $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x50a00000 }
  },
/* cpssll3.b $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x51a00000 }
  },
/* cpsll3.h $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x52a00000 }
  },
/* cpssll3.h $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x53a00000 }
  },
/* cpsll3.w $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x54a00000 }
  },
/* cpssll3.w $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x55a00000 }
  },
/* cdsll3 $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x56a00000 }
  },
/* cpsla3.h $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x5aa00000 }
  },
/* cpsla3.w $crop,$crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpadd3_b_P0S_P1, { 0x5ca00000 }
  },
/* cpsrli3.b $crop,$crqp,$imm3p5 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (IMM3P5), 0 } },
    & ifmt_cpsrli3_b_P0_P1, { 0xa80000 }
  },
/* cpsrli3.h $crop,$crqp,$imm4p4 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (IMM4P4), 0 } },
    & ifmt_cpsrli3_h_P0_P1, { 0xa80200 }
  },
/* cpsrli3.w $crop,$crqp,$imm5p3 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (IMM5P3), 0 } },
    & ifmt_cpsrli3_w_P0_P1, { 0xa80400 }
  },
/* cdsrli3 $crop,$crqp,$imm6p2 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (IMM6P2), 0 } },
    & ifmt_cdsrli3_P0_P1, { 0xa80600 }
  },
/* cpsrai3.b $crop,$crqp,$imm3p5 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (IMM3P5), 0 } },
    & ifmt_cpsrli3_b_P0_P1, { 0xa80800 }
  },
/* cpsrai3.h $crop,$crqp,$imm4p4 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (IMM4P4), 0 } },
    & ifmt_cpsrli3_h_P0_P1, { 0xa80a00 }
  },
/* cpsrai3.w $crop,$crqp,$imm5p3 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (IMM5P3), 0 } },
    & ifmt_cpsrli3_w_P0_P1, { 0xa80c00 }
  },
/* cdsrai3 $crop,$crqp,$imm6p2 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (IMM6P2), 0 } },
    & ifmt_cdsrli3_P0_P1, { 0xa80e00 }
  },
/* cpslli3.b $crop,$crqp,$imm3p5 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (IMM3P5), 0 } },
    & ifmt_cpsrli3_b_P0_P1, { 0xa81000 }
  },
/* cpslli3.h $crop,$crqp,$imm4p4 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (IMM4P4), 0 } },
    & ifmt_cpsrli3_h_P0_P1, { 0xa81200 }
  },
/* cpslli3.w $crop,$crqp,$imm5p3 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (IMM5P3), 0 } },
    & ifmt_cpsrli3_w_P0_P1, { 0xa81400 }
  },
/* cdslli3 $crop,$crqp,$imm6p2 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (IMM6P2), 0 } },
    & ifmt_cdsrli3_P0_P1, { 0xa81600 }
  },
/* cpslai3.h $crop,$crqp,$imm4p4 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (IMM4P4), 0 } },
    & ifmt_cpsrli3_h_P0_P1, { 0xa81a00 }
  },
/* cpslai3.w $crop,$crqp,$imm5p3 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (IMM5P3), 0 } },
    & ifmt_cpsrli3_w_P0_P1, { 0xa81c00 }
  },
/* cpclipiu3.w $crop,$crqp,$imm5p3 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (IMM5P3), 0 } },
    & ifmt_cpsrli3_w_P0_P1, { 0xa82000 }
  },
/* cpclipi3.w $crop,$crqp,$imm5p3 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (IMM5P3), 0 } },
    & ifmt_cpsrli3_w_P0_P1, { 0xa82200 }
  },
/* cdclipiu3 $crop,$crqp,$imm6p2 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (IMM6P2), 0 } },
    & ifmt_cdsrli3_P0_P1, { 0xa82400 }
  },
/* cdclipi3 $crop,$crqp,$imm6p2 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), ',', OP (CRQP), ',', OP (IMM6P2), 0 } },
    & ifmt_cdsrli3_P0_P1, { 0xa82600 }
  },
/* cpmovi.h $crqp,$simm16p0 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (SIMM16P0), 0 } },
    & ifmt_cpmovi_h_P0_P1, { 0xb01000 }
  },
/* cpmoviu.w $crqp,$imm16p0 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (IMM16P0), 0 } },
    & ifmt_cpmoviu_w_P0_P1, { 0xb80000 }
  },
/* cpmovi.w $crqp,$simm16p0 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (SIMM16P0), 0 } },
    & ifmt_cpmovi_h_P0_P1, { 0xb81000 }
  },
/* cdmoviu $crqp,$imm16p0 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (IMM16P0), 0 } },
    & ifmt_cpmoviu_w_P0_P1, { 0xb82000 }
  },
/* cdmovi $crqp,$simm16p0 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (SIMM16P0), 0 } },
    & ifmt_cpmovi_h_P0_P1, { 0xb83000 }
  },
/* c1nop */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_c0nop_P0_P0S, { 0x0 }
  },
/* cpmovi.b $crqp,$simm8p20 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (SIMM8P20), 0 } },
    & ifmt_cpmovi_b_P0S_P1, { 0xb00000 }
  },
/* cpadda1u.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc00000 }
  },
/* cpadda1.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc00010 }
  },
/* cpaddua1.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc00020 }
  },
/* cpaddla1.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc00030 }
  },
/* cpaddaca1u.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc00040 }
  },
/* cpaddaca1.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc00050 }
  },
/* cpaddacua1.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc00060 }
  },
/* cpaddacla1.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc00070 }
  },
/* cpsuba1u.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc00080 }
  },
/* cpsuba1.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc00090 }
  },
/* cpsubua1.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc000a0 }
  },
/* cpsubla1.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc000b0 }
  },
/* cpsubaca1u.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc000c0 }
  },
/* cpsubaca1.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc000d0 }
  },
/* cpsubacua1.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc000e0 }
  },
/* cpsubacla1.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc000f0 }
  },
/* cpabsa1u.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc00100 }
  },
/* cpabsa1.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc00110 }
  },
/* cpabsua1.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc00120 }
  },
/* cpabsla1.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc00130 }
  },
/* cpsada1u.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc00140 }
  },
/* cpsada1.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc00150 }
  },
/* cpsadua1.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc00160 }
  },
/* cpsadla1.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc00170 }
  },
/* cpseta1.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc001b0 }
  },
/* cpsetua1.w $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc001c0 }
  },
/* cpsetla1.w $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xc001d0 }
  },
/* cpmova1.b $crop */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), 0 } },
    & ifmt_cpmovfrcsar0_P0S_P1, { 0xc80200 }
  },
/* cpmovua1.h $crop */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), 0 } },
    & ifmt_cpmovfrcsar0_P0S_P1, { 0xc80400 }
  },
/* cpmovla1.h $crop */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), 0 } },
    & ifmt_cpmovfrcsar0_P0S_P1, { 0xc80600 }
  },
/* cpmovuua1.w $crop */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), 0 } },
    & ifmt_cpmovfrcsar0_P0S_P1, { 0xc80800 }
  },
/* cpmovula1.w $crop */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), 0 } },
    & ifmt_cpmovfrcsar0_P0S_P1, { 0xc80a00 }
  },
/* cpmovlua1.w $crop */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), 0 } },
    & ifmt_cpmovfrcsar0_P0S_P1, { 0xc80c00 }
  },
/* cpmovlla1.w $crop */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), 0 } },
    & ifmt_cpmovfrcsar0_P0S_P1, { 0xc80e00 }
  },
/* cppacka1u.b $crop */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), 0 } },
    & ifmt_cpmovfrcsar0_P0S_P1, { 0xc81000 }
  },
/* cppacka1.b $crop */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), 0 } },
    & ifmt_cpmovfrcsar0_P0S_P1, { 0xc81200 }
  },
/* cppackua1.h $crop */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), 0 } },
    & ifmt_cpmovfrcsar0_P0S_P1, { 0xc81400 }
  },
/* cppackla1.h $crop */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), 0 } },
    & ifmt_cpmovfrcsar0_P0S_P1, { 0xc81600 }
  },
/* cppackua1.w $crop */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), 0 } },
    & ifmt_cpmovfrcsar0_P0S_P1, { 0xc81800 }
  },
/* cppackla1.w $crop */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), 0 } },
    & ifmt_cpmovfrcsar0_P0S_P1, { 0xc81a00 }
  },
/* cpmovhua1.w $crop */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), 0 } },
    & ifmt_cpmovfrcsar0_P0S_P1, { 0xc81c00 }
  },
/* cpmovhla1.w $crop */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CROP), 0 } },
    & ifmt_cpmovfrcsar0_P0S_P1, { 0xc81e00 }
  },
/* cpacsuma1 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_c0nop_P0_P0S, { 0xc82000 }
  },
/* cpaccpa1 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_c0nop_P0_P0S, { 0xc82200 }
  },
/* cpacswp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_c0nop_P0_P0S, { 0xc82400 }
  },
/* cpsrla1 $crqp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), 0 } },
    & ifmt_cpccadd_b_P0S_P1, { 0xc83000 }
  },
/* cpsraa1 $crqp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), 0 } },
    & ifmt_cpccadd_b_P0S_P1, { 0xc83200 }
  },
/* cpslla1 $crqp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), 0 } },
    & ifmt_cpccadd_b_P0S_P1, { 0xc83400 }
  },
/* cpsrlia1 $imm5p23 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (IMM5P23), 0 } },
    & ifmt_cpsrlia0_P0S, { 0xc83800 }
  },
/* cpsraia1 $imm5p23 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (IMM5P23), 0 } },
    & ifmt_cpsrlia0_P0S, { 0xc83a00 }
  },
/* cpsllia1 $imm5p23 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (IMM5P23), 0 } },
    & ifmt_cpsrlia0_P0S, { 0xc83c00 }
  },
/* cpfmulia1s0u.b $crqp,$crpp,$simm8p0 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), ',', OP (SIMM8P0), 0 } },
    & ifmt_cpfmulia1s0u_b_P1, { 0xf80000 }
  },
/* cpfmulia1s0.b $crqp,$crpp,$simm8p0 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), ',', OP (SIMM8P0), 0 } },
    & ifmt_cpfmulia1s0u_b_P1, { 0xf80010 }
  },
/* cpfmuliua1s0.h $crqp,$crpp,$simm8p0 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), ',', OP (SIMM8P0), 0 } },
    & ifmt_cpfmulia1s0u_b_P1, { 0xf80020 }
  },
/* cpfmulila1s0.h $crqp,$crpp,$simm8p0 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), ',', OP (SIMM8P0), 0 } },
    & ifmt_cpfmulia1s0u_b_P1, { 0xf80030 }
  },
/* cpfmadia1s0u.b $crqp,$crpp,$simm8p0 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), ',', OP (SIMM8P0), 0 } },
    & ifmt_cpfmulia1s0u_b_P1, { 0xf80040 }
  },
/* cpfmadia1s0.b $crqp,$crpp,$simm8p0 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), ',', OP (SIMM8P0), 0 } },
    & ifmt_cpfmulia1s0u_b_P1, { 0xf80050 }
  },
/* cpfmadiua1s0.h $crqp,$crpp,$simm8p0 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), ',', OP (SIMM8P0), 0 } },
    & ifmt_cpfmulia1s0u_b_P1, { 0xf80060 }
  },
/* cpfmadila1s0.h $crqp,$crpp,$simm8p0 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), ',', OP (SIMM8P0), 0 } },
    & ifmt_cpfmulia1s0u_b_P1, { 0xf80070 }
  },
/* cpfmulia1s1u.b $crqp,$crpp,$simm8p0 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), ',', OP (SIMM8P0), 0 } },
    & ifmt_cpfmulia1s0u_b_P1, { 0xf80080 }
  },
/* cpfmulia1s1.b $crqp,$crpp,$simm8p0 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), ',', OP (SIMM8P0), 0 } },
    & ifmt_cpfmulia1s0u_b_P1, { 0xf80090 }
  },
/* cpfmuliua1s1.h $crqp,$crpp,$simm8p0 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), ',', OP (SIMM8P0), 0 } },
    & ifmt_cpfmulia1s0u_b_P1, { 0xf800a0 }
  },
/* cpfmulila1s1.h $crqp,$crpp,$simm8p0 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), ',', OP (SIMM8P0), 0 } },
    & ifmt_cpfmulia1s0u_b_P1, { 0xf800b0 }
  },
/* cpfmadia1s1u.b $crqp,$crpp,$simm8p0 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), ',', OP (SIMM8P0), 0 } },
    & ifmt_cpfmulia1s0u_b_P1, { 0xf800c0 }
  },
/* cpfmadia1s1.b $crqp,$crpp,$simm8p0 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), ',', OP (SIMM8P0), 0 } },
    & ifmt_cpfmulia1s0u_b_P1, { 0xf800d0 }
  },
/* cpfmadiua1s1.h $crqp,$crpp,$simm8p0 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), ',', OP (SIMM8P0), 0 } },
    & ifmt_cpfmulia1s0u_b_P1, { 0xf800e0 }
  },
/* cpfmadila1s1.h $crqp,$crpp,$simm8p0 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), ',', OP (SIMM8P0), 0 } },
    & ifmt_cpfmulia1s0u_b_P1, { 0xf800f0 }
  },
/* cpamulia1u.b $crqp,$crpp,$simm8p0 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), ',', OP (SIMM8P0), 0 } },
    & ifmt_cpfmulia1s0u_b_P1, { 0xf80100 }
  },
/* cpamulia1.b $crqp,$crpp,$simm8p0 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), ',', OP (SIMM8P0), 0 } },
    & ifmt_cpfmulia1s0u_b_P1, { 0xf80110 }
  },
/* cpamuliua1.h $crqp,$crpp,$simm8p0 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), ',', OP (SIMM8P0), 0 } },
    & ifmt_cpfmulia1s0u_b_P1, { 0xf80120 }
  },
/* cpamulila1.h $crqp,$crpp,$simm8p0 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), ',', OP (SIMM8P0), 0 } },
    & ifmt_cpfmulia1s0u_b_P1, { 0xf80130 }
  },
/* cpamadia1u.b $crqp,$crpp,$simm8p0 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), ',', OP (SIMM8P0), 0 } },
    & ifmt_cpfmulia1s0u_b_P1, { 0xf80140 }
  },
/* cpamadia1.b $crqp,$crpp,$simm8p0 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), ',', OP (SIMM8P0), 0 } },
    & ifmt_cpfmulia1s0u_b_P1, { 0xf80150 }
  },
/* cpamadiua1.h $crqp,$crpp,$simm8p0 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), ',', OP (SIMM8P0), 0 } },
    & ifmt_cpfmulia1s0u_b_P1, { 0xf80160 }
  },
/* cpamadila1.h $crqp,$crpp,$simm8p0 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), ',', OP (SIMM8P0), 0 } },
    & ifmt_cpfmulia1s0u_b_P1, { 0xf80170 }
  },
/* cpfmulia1u.b $crqp,$crpp,$imm3p25,$simm8p0 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), ',', OP (IMM3P25), ',', OP (SIMM8P0), 0 } },
    & ifmt_cpfmulia1u_b_P1, { 0xe00000 }
  },
/* cpfmulia1.b $crqp,$crpp,$imm3p25,$simm8p0 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), ',', OP (IMM3P25), ',', OP (SIMM8P0), 0 } },
    & ifmt_cpfmulia1u_b_P1, { 0xe00080 }
  },
/* cpfmuliua1.h $crqp,$crpp,$imm3p25,$simm8p0 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), ',', OP (IMM3P25), ',', OP (SIMM8P0), 0 } },
    & ifmt_cpfmulia1u_b_P1, { 0xe00100 }
  },
/* cpfmulila1.h $crqp,$crpp,$imm3p25,$simm8p0 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), ',', OP (IMM3P25), ',', OP (SIMM8P0), 0 } },
    & ifmt_cpfmulia1u_b_P1, { 0xe00180 }
  },
/* cpfmadia1u.b $crqp,$crpp,$imm3p25,$simm8p0 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), ',', OP (IMM3P25), ',', OP (SIMM8P0), 0 } },
    & ifmt_cpfmulia1u_b_P1, { 0xe80000 }
  },
/* cpfmadia1.b $crqp,$crpp,$imm3p25,$simm8p0 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), ',', OP (IMM3P25), ',', OP (SIMM8P0), 0 } },
    & ifmt_cpfmulia1u_b_P1, { 0xe80080 }
  },
/* cpfmadiua1.h $crqp,$crpp,$imm3p25,$simm8p0 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), ',', OP (IMM3P25), ',', OP (SIMM8P0), 0 } },
    & ifmt_cpfmulia1u_b_P1, { 0xe80100 }
  },
/* cpfmadila1.h $crqp,$crpp,$imm3p25,$simm8p0 */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), ',', OP (IMM3P25), ',', OP (SIMM8P0), 0 } },
    & ifmt_cpfmulia1u_b_P1, { 0xe80180 }
  },
/* cpssqa1u.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xf00000 }
  },
/* cpssqa1.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xf00010 }
  },
/* cpssda1u.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xf00040 }
  },
/* cpssda1.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xf00050 }
  },
/* cpmula1u.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xf00080 }
  },
/* cpmula1.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xf00090 }
  },
/* cpmulua1.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xf000a0 }
  },
/* cpmulla1.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xf000b0 }
  },
/* cpmulua1u.w $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xf000c0 }
  },
/* cpmulla1u.w $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xf000d0 }
  },
/* cpmulua1.w $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xf000e0 }
  },
/* cpmulla1.w $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xf000f0 }
  },
/* cpmada1u.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xf00100 }
  },
/* cpmada1.b $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xf00110 }
  },
/* cpmadua1.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xf00120 }
  },
/* cpmadla1.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xf00130 }
  },
/* cpmadua1u.w $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xf00140 }
  },
/* cpmadla1u.w $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xf00150 }
  },
/* cpmadua1.w $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xf00160 }
  },
/* cpmadla1.w $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xf00170 }
  },
/* cpmsbua1.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xf001a0 }
  },
/* cpmsbla1.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xf001b0 }
  },
/* cpmsbua1u.w $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xf001c0 }
  },
/* cpmsbla1u.w $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xf001d0 }
  },
/* cpmsbua1.w $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xf001e0 }
  },
/* cpmsbla1.w $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0xf001f0 }
  },
/* cpsmadua1.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x1f00120 }
  },
/* cpsmadla1.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x1f00130 }
  },
/* cpsmadua1.w $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x1f00160 }
  },
/* cpsmadla1.w $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x1f00170 }
  },
/* cpsmsbua1.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x1f001a0 }
  },
/* cpsmsbla1.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x1f001b0 }
  },
/* cpsmsbua1.w $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x1f001e0 }
  },
/* cpsmsbla1.w $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x1f001f0 }
  },
/* cpmulslua1.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x2f000a0 }
  },
/* cpmulslla1.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x2f000b0 }
  },
/* cpmulslua1.w $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x2f000e0 }
  },
/* cpmulslla1.w $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x2f000f0 }
  },
/* cpsmadslua1.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x3f00120 }
  },
/* cpsmadslla1.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x3f00130 }
  },
/* cpsmadslua1.w $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x3f00160 }
  },
/* cpsmadslla1.w $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x3f00170 }
  },
/* cpsmsbslua1.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x3f001a0 }
  },
/* cpsmsbslla1.h $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x3f001b0 }
  },
/* cpsmsbslua1.w $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x3f001e0 }
  },
/* cpsmsbslla1.w $crqp,$crpp */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRQP), ',', OP (CRPP), 0 } },
    & ifmt_cpcmpeqz_b_P0S_P1, { 0x3f001f0 }
  },
};

#undef A
#undef OPERAND
#undef MNEM
#undef OP

/* Formats for ALIAS macro-insns.  */

#define F(f) & mep_cgen_ifld_table[MEP_##f]
static const CGEN_IFMT ifmt_nop ATTRIBUTE_UNUSED = {
  16, 16, 0xffff, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_RM) }, { F (F_SUB4) }, { 0 } }
};

static const CGEN_IFMT ifmt_sb16_0 ATTRIBUTE_UNUSED = {
  16, 16, 0xf00f, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_RM) }, { F (F_SUB4) }, { 0 } }
};

static const CGEN_IFMT ifmt_sh16_0 ATTRIBUTE_UNUSED = {
  16, 16, 0xf00f, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_RM) }, { F (F_SUB4) }, { 0 } }
};

static const CGEN_IFMT ifmt_sw16_0 ATTRIBUTE_UNUSED = {
  16, 16, 0xf00f, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_RM) }, { F (F_SUB4) }, { 0 } }
};

static const CGEN_IFMT ifmt_lb16_0 ATTRIBUTE_UNUSED = {
  16, 16, 0xf00f, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_RM) }, { F (F_SUB4) }, { 0 } }
};

static const CGEN_IFMT ifmt_lh16_0 ATTRIBUTE_UNUSED = {
  16, 16, 0xf00f, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_RM) }, { F (F_SUB4) }, { 0 } }
};

static const CGEN_IFMT ifmt_lw16_0 ATTRIBUTE_UNUSED = {
  16, 16, 0xf00f, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_RM) }, { F (F_SUB4) }, { 0 } }
};

static const CGEN_IFMT ifmt_lbu16_0 ATTRIBUTE_UNUSED = {
  16, 16, 0xf00f, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_RM) }, { F (F_SUB4) }, { 0 } }
};

static const CGEN_IFMT ifmt_lhu16_0 ATTRIBUTE_UNUSED = {
  16, 16, 0xf00f, { { F (F_MAJOR) }, { F (F_RN) }, { F (F_RM) }, { F (F_SUB4) }, { 0 } }
};

static const CGEN_IFMT ifmt_swcp16_0 ATTRIBUTE_UNUSED = {
  16, 16, 0xf00f, { { F (F_MAJOR) }, { F (F_CRN) }, { F (F_RM) }, { F (F_SUB4) }, { 0 } }
};

static const CGEN_IFMT ifmt_lwcp16_0 ATTRIBUTE_UNUSED = {
  16, 16, 0xf00f, { { F (F_MAJOR) }, { F (F_CRN) }, { F (F_RM) }, { F (F_SUB4) }, { 0 } }
};

static const CGEN_IFMT ifmt_smcp16_0 ATTRIBUTE_UNUSED = {
  16, 16, 0xf00f, { { F (F_MAJOR) }, { F (F_CRN) }, { F (F_RM) }, { F (F_SUB4) }, { 0 } }
};

static const CGEN_IFMT ifmt_lmcp16_0 ATTRIBUTE_UNUSED = {
  16, 16, 0xf00f, { { F (F_MAJOR) }, { F (F_CRN) }, { F (F_RM) }, { F (F_SUB4) }, { 0 } }
};

#undef F

/* Each non-simple macro entry points to an array of expansion possibilities.  */

#define A(a) (1 << CGEN_INSN_##a)
#define OPERAND(op) MEP_OPERAND_##op
#define MNEM CGEN_SYNTAX_MNEMONIC /* syntax value for mnemonic */
#define OP(field) CGEN_SYNTAX_MAKE_FIELD (OPERAND (field))

/* The macro instruction table.  */

static const CGEN_IBASE mep_cgen_macro_insn_table[] =
{
/* nop */
  {
    -1, "nop", "nop", 16,
    { 0|A(ALIAS), { { { (1<<MACH_BASE), 0 } }, { { 1, "\x80" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* sb $rnc,$zero($rma) */
  {
    -1, "sb16-0", "sb", 16,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* sh $rns,$zero($rma) */
  {
    -1, "sh16-0", "sh", 16,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* sw $rnl,$zero($rma) */
  {
    -1, "sw16-0", "sw", 16,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lb $rnc,$zero($rma) */
  {
    -1, "lb16-0", "lb", 16,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lh $rns,$zero($rma) */
  {
    -1, "lh16-0", "lh", 16,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lw $rnl,$zero($rma) */
  {
    -1, "lw16-0", "lw", 16,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lbu $rnuc,$zero($rma) */
  {
    -1, "lbu16-0", "lbu", 16,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lhu $rnus,$zero($rma) */
  {
    -1, "lhu16-0", "lhu", 16,
    { 0|A(NO_DIS)|A(ALIAS), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* swcp $crn,$zero($rma) */
  {
    -1, "swcp16-0", "swcp", 16,
    { 0|A(NO_DIS)|A(OPTIONAL_CP_INSN)|A(ALIAS), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lwcp $crn,$zero($rma) */
  {
    -1, "lwcp16-0", "lwcp", 16,
    { 0|A(NO_DIS)|A(OPTIONAL_CP_INSN)|A(ALIAS), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* smcp $crn64,$zero($rma) */
  {
    -1, "smcp16-0", "smcp", 16,
    { 0|A(NO_DIS)|A(OPTIONAL_CP64_INSN)|A(OPTIONAL_CP_INSN)|A(ALIAS), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
/* lmcp $crn64,$zero($rma) */
  {
    -1, "lmcp16-0", "lmcp", 16,
    { 0|A(NO_DIS)|A(OPTIONAL_CP64_INSN)|A(OPTIONAL_CP_INSN)|A(ALIAS), { { { (1<<MACH_BASE), 0 } }, { { 1, "\xc0" } }, { { CPTYPE_CP_DATA_BUS_INT, 0 } }, { { CRET_VOID, 0 } }, { { 0, 0 } }, { { CONFIG_NONE, 0 } }, { { (1<<SLOTS_CORE), 0 } } } }
  },
};

/* The macro instruction opcode table.  */

static const CGEN_OPCODE mep_cgen_macro_insn_opcode_table[] =
{
/* nop */
  {
    { 0, 0, 0, 0 },
    { { MNEM, 0 } },
    & ifmt_nop, { 0x0 }
  },
/* sb $rnc,$zero($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RNC), ',', OP (ZERO), '(', OP (RMA), ')', 0 } },
    & ifmt_sb16_0, { 0x8 }
  },
/* sh $rns,$zero($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RNS), ',', OP (ZERO), '(', OP (RMA), ')', 0 } },
    & ifmt_sh16_0, { 0x9 }
  },
/* sw $rnl,$zero($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RNL), ',', OP (ZERO), '(', OP (RMA), ')', 0 } },
    & ifmt_sw16_0, { 0xa }
  },
/* lb $rnc,$zero($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RNC), ',', OP (ZERO), '(', OP (RMA), ')', 0 } },
    & ifmt_lb16_0, { 0xc }
  },
/* lh $rns,$zero($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RNS), ',', OP (ZERO), '(', OP (RMA), ')', 0 } },
    & ifmt_lh16_0, { 0xd }
  },
/* lw $rnl,$zero($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RNL), ',', OP (ZERO), '(', OP (RMA), ')', 0 } },
    & ifmt_lw16_0, { 0xe }
  },
/* lbu $rnuc,$zero($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RNUC), ',', OP (ZERO), '(', OP (RMA), ')', 0 } },
    & ifmt_lbu16_0, { 0xb }
  },
/* lhu $rnus,$zero($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (RNUS), ',', OP (ZERO), '(', OP (RMA), ')', 0 } },
    & ifmt_lhu16_0, { 0xf }
  },
/* swcp $crn,$zero($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN), ',', OP (ZERO), '(', OP (RMA), ')', 0 } },
    & ifmt_swcp16_0, { 0x3008 }
  },
/* lwcp $crn,$zero($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN), ',', OP (ZERO), '(', OP (RMA), ')', 0 } },
    & ifmt_lwcp16_0, { 0x3009 }
  },
/* smcp $crn64,$zero($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN64), ',', OP (ZERO), '(', OP (RMA), ')', 0 } },
    & ifmt_smcp16_0, { 0x300a }
  },
/* lmcp $crn64,$zero($rma) */
  {
    { 0, 0, 0, 0 },
    { { MNEM, ' ', OP (CRN64), ',', OP (ZERO), '(', OP (RMA), ')', 0 } },
    & ifmt_lmcp16_0, { 0x300b }
  },
};

#undef A
#undef OPERAND
#undef MNEM
#undef OP

#ifndef CGEN_ASM_HASH_P
#define CGEN_ASM_HASH_P(insn) 1
#endif

#ifndef CGEN_DIS_HASH_P
#define CGEN_DIS_HASH_P(insn) 1
#endif

/* Return non-zero if INSN is to be added to the hash table.
   Targets are free to override CGEN_{ASM,DIS}_HASH_P in the .opc file.  */

static int
asm_hash_insn_p (const CGEN_INSN *insn ATTRIBUTE_UNUSED)
{
  return CGEN_ASM_HASH_P (insn);
}

static int
dis_hash_insn_p (const CGEN_INSN *insn)
{
  /* If building the hash table and the NO-DIS attribute is present,
     ignore.  */
  if (CGEN_INSN_ATTR_VALUE (insn, CGEN_INSN_NO_DIS))
    return 0;
  return CGEN_DIS_HASH_P (insn);
}

#ifndef CGEN_ASM_HASH
#define CGEN_ASM_HASH_SIZE 127
#ifdef CGEN_MNEMONIC_OPERANDS
#define CGEN_ASM_HASH(mnem) (*(unsigned char *) (mnem) % CGEN_ASM_HASH_SIZE)
#else
#define CGEN_ASM_HASH(mnem) (*(unsigned char *) (mnem) % CGEN_ASM_HASH_SIZE) /*FIXME*/
#endif
#endif

/* It doesn't make much sense to provide a default here,
   but while this is under development we do.
   BUFFER is a pointer to the bytes of the insn, target order.
   VALUE is the first base_insn_bitsize bits as an int in host order.  */

#ifndef CGEN_DIS_HASH
#define CGEN_DIS_HASH_SIZE 256
#define CGEN_DIS_HASH(buf, value) (*(unsigned char *) (buf))
#endif

/* The result is the hash value of the insn.
   Targets are free to override CGEN_{ASM,DIS}_HASH in the .opc file.  */

static unsigned int
asm_hash_insn (const char *mnem)
{
  return CGEN_ASM_HASH (mnem);
}

/* BUF is a pointer to the bytes of the insn, target order.
   VALUE is the first base_insn_bitsize bits as an int in host order.  */

static unsigned int
dis_hash_insn (const char *buf ATTRIBUTE_UNUSED,
		     CGEN_INSN_INT value ATTRIBUTE_UNUSED)
{
  return CGEN_DIS_HASH (buf, value);
}

/* Set the recorded length of the insn in the CGEN_FIELDS struct.  */

static void
set_fields_bitsize (CGEN_FIELDS *fields, int size)
{
  CGEN_FIELDS_BITSIZE (fields) = size;
}

/* Function to call before using the operand instance table.
   This plugs the opcode entries and macro instructions into the cpu table.  */

void
mep_cgen_init_opcode_table (CGEN_CPU_DESC cd)
{
  int i;
  int num_macros = (sizeof (mep_cgen_macro_insn_table) /
		    sizeof (mep_cgen_macro_insn_table[0]));
  const CGEN_IBASE *ib = & mep_cgen_macro_insn_table[0];
  const CGEN_OPCODE *oc = & mep_cgen_macro_insn_opcode_table[0];
  CGEN_INSN *insns = xmalloc (num_macros * sizeof (CGEN_INSN));

  /* This test has been added to avoid a warning generated
     if memset is called with a third argument of value zero.  */
  if (num_macros >= 1)
    memset (insns, 0, num_macros * sizeof (CGEN_INSN));
  for (i = 0; i < num_macros; ++i)
    {
      insns[i].base = &ib[i];
      insns[i].opcode = &oc[i];
      mep_cgen_build_insn_regex (& insns[i]);
    }
  cd->macro_insn_table.init_entries = insns;
  cd->macro_insn_table.entry_size = sizeof (CGEN_IBASE);
  cd->macro_insn_table.num_init_entries = num_macros;

  oc = & mep_cgen_insn_opcode_table[0];
  insns = (CGEN_INSN *) cd->insn_table.init_entries;
  for (i = 0; i < MAX_INSNS; ++i)
    {
      insns[i].opcode = &oc[i];
      mep_cgen_build_insn_regex (& insns[i]);
    }

  cd->sizeof_fields = sizeof (CGEN_FIELDS);
  cd->set_fields_bitsize = set_fields_bitsize;

  cd->asm_hash_p = asm_hash_insn_p;
  cd->asm_hash = asm_hash_insn;
  cd->asm_hash_size = CGEN_ASM_HASH_SIZE;

  cd->dis_hash_p = dis_hash_insn_p;
  cd->dis_hash = dis_hash_insn;
  cd->dis_hash_size = CGEN_DIS_HASH_SIZE;
}
