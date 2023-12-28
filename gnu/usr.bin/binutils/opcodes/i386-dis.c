/* Print i386 instructions for GDB, the GNU debugger.
   Copyright (C) 1988-2023 Free Software Foundation, Inc.

   This file is part of the GNU opcodes library.

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   It is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */


/* 80386 instruction printer by Pace Willisson (pace@prep.ai.mit.edu)
   July 1988
    modified by John Hassey (hassey@dg-rtp.dg.com)
    x86-64 support added by Jan Hubicka (jh@suse.cz)
    VIA PadLock support by Michal Ludvig (mludvig@suse.cz).  */

/* The main tables describing the instructions is essentially a copy
   of the "Opcode Map" chapter (Appendix A) of the Intel 80386
   Programmers Manual.  Usually, there is a capital letter, followed
   by a small letter.  The capital letter tell the addressing mode,
   and the small letter tells about the operand size.  Refer to
   the Intel manual for details.  */

#include "sysdep.h"
#include "disassemble.h"
#include "opintl.h"
#include "opcode/i386.h"
#include "libiberty.h"
#include "safe-ctype.h"

typedef struct instr_info instr_info;

static bool dofloat (instr_info *, int);
static int putop (instr_info *, const char *, int);
static void oappend_with_style (instr_info *, const char *,
				enum disassembler_style);

static bool OP_E (instr_info *, int, int);
static bool OP_E_memory (instr_info *, int, int);
static bool OP_indirE (instr_info *, int, int);
static bool OP_G (instr_info *, int, int);
static bool OP_ST (instr_info *, int, int);
static bool OP_STi (instr_info *, int, int);
static bool OP_Skip_MODRM (instr_info *, int, int);
static bool OP_REG (instr_info *, int, int);
static bool OP_IMREG (instr_info *, int, int);
static bool OP_I (instr_info *, int, int);
static bool OP_I64 (instr_info *, int, int);
static bool OP_sI (instr_info *, int, int);
static bool OP_J (instr_info *, int, int);
static bool OP_SEG (instr_info *, int, int);
static bool OP_DIR (instr_info *, int, int);
static bool OP_OFF (instr_info *, int, int);
static bool OP_OFF64 (instr_info *, int, int);
static bool OP_ESreg (instr_info *, int, int);
static bool OP_DSreg (instr_info *, int, int);
static bool OP_C (instr_info *, int, int);
static bool OP_D (instr_info *, int, int);
static bool OP_T (instr_info *, int, int);
static bool OP_MMX (instr_info *, int, int);
static bool OP_XMM (instr_info *, int, int);
static bool OP_EM (instr_info *, int, int);
static bool OP_EX (instr_info *, int, int);
static bool OP_EMC (instr_info *, int,int);
static bool OP_MXC (instr_info *, int,int);
static bool OP_MS (instr_info *, int, int);
static bool OP_XS (instr_info *, int, int);
static bool OP_M (instr_info *, int, int);
static bool OP_VEX (instr_info *, int, int);
static bool OP_VexR (instr_info *, int, int);
static bool OP_VexW (instr_info *, int, int);
static bool OP_Rounding (instr_info *, int, int);
static bool OP_REG_VexI4 (instr_info *, int, int);
static bool OP_VexI4 (instr_info *, int, int);
static bool OP_0f07 (instr_info *, int, int);
static bool OP_Monitor (instr_info *, int, int);
static bool OP_Mwait (instr_info *, int, int);

static bool PCLMUL_Fixup (instr_info *, int, int);
static bool VPCMP_Fixup (instr_info *, int, int);
static bool VPCOM_Fixup (instr_info *, int, int);
static bool NOP_Fixup (instr_info *, int, int);
static bool OP_3DNowSuffix (instr_info *, int, int);
static bool CMP_Fixup (instr_info *, int, int);
static bool REP_Fixup (instr_info *, int, int);
static bool SEP_Fixup (instr_info *, int, int);
static bool BND_Fixup (instr_info *, int, int);
static bool NOTRACK_Fixup (instr_info *, int, int);
static bool HLE_Fixup1 (instr_info *, int, int);
static bool HLE_Fixup2 (instr_info *, int, int);
static bool HLE_Fixup3 (instr_info *, int, int);
static bool CMPXCHG8B_Fixup (instr_info *, int, int);
static bool XMM_Fixup (instr_info *, int, int);
static bool FXSAVE_Fixup (instr_info *, int, int);
static bool MOVSXD_Fixup (instr_info *, int, int);
static bool DistinctDest_Fixup (instr_info *, int, int);
static bool PREFETCHI_Fixup (instr_info *, int, int);

static void ATTRIBUTE_PRINTF_3 i386_dis_printf (const disassemble_info *,
						enum disassembler_style,
						const char *, ...);

/* This character is used to encode style information within the output
   buffers.  See oappend_insert_style for more details.  */
#define STYLE_MARKER_CHAR '\002'

/* The maximum operand buffer size.  */
#define MAX_OPERAND_BUFFER_SIZE 128

enum address_mode
{
  mode_16bit,
  mode_32bit,
  mode_64bit
};

static const char *prefix_name (enum address_mode, uint8_t, int);

enum x86_64_isa
{
  amd64 = 1,
  intel64
};

struct instr_info
{
  enum address_mode address_mode;

  /* Flags for the prefixes for the current instruction.  See below.  */
  int prefixes;

  /* REX prefix the current instruction.  See below.  */
  uint8_t rex;
  /* Bits of REX we've already used.  */
  uint8_t rex_used;

  bool need_modrm;
  bool need_vex;
  bool has_sib;

  /* Flags for ins->prefixes which we somehow handled when printing the
     current instruction.  */
  int used_prefixes;

  /* Flags for EVEX bits which we somehow handled when printing the
     current instruction.  */
  int evex_used;

  char obuf[MAX_OPERAND_BUFFER_SIZE];
  char *obufp;
  char *mnemonicendp;
  const uint8_t *start_codep;
  uint8_t *codep;
  const uint8_t *end_codep;
  unsigned char nr_prefixes;
  signed char last_lock_prefix;
  signed char last_repz_prefix;
  signed char last_repnz_prefix;
  signed char last_data_prefix;
  signed char last_addr_prefix;
  signed char last_rex_prefix;
  signed char last_seg_prefix;
  signed char fwait_prefix;
  /* The active segment register prefix.  */
  unsigned char active_seg_prefix;

#define MAX_CODE_LENGTH 15
  /* We can up to 14 ins->prefixes since the maximum instruction length is
     15bytes.  */
  uint8_t all_prefixes[MAX_CODE_LENGTH - 1];
  disassemble_info *info;

  struct
  {
    int mod;
    int reg;
    int rm;
  }
  modrm;

  struct
  {
    int scale;
    int index;
    int base;
  }
  sib;

  struct
  {
    int register_specifier;
    int length;
    int prefix;
    int mask_register_specifier;
    int ll;
    bool w;
    bool evex;
    bool r;
    bool v;
    bool zeroing;
    bool b;
    bool no_broadcast;
  }
  vex;

  /* Remember if the current op is a jump instruction.  */
  bool op_is_jump;

  bool two_source_ops;

  unsigned char op_ad;
  signed char op_index[MAX_OPERANDS];
  bool op_riprel[MAX_OPERANDS];
  char *op_out[MAX_OPERANDS];
  bfd_vma op_address[MAX_OPERANDS];
  bfd_vma start_pc;

  /* On the 386's of 1988, the maximum length of an instruction is 15 bytes.
   *   (see topic "Redundant ins->prefixes" in the "Differences from 8086"
   *   section of the "Virtual 8086 Mode" chapter.)
   * 'pc' should be the address of this instruction, it will
   *   be used to print the target address if this is a relative jump or call
   * The function returns the length of this instruction in bytes.
   */
  char intel_syntax;
  bool intel_mnemonic;
  char open_char;
  char close_char;
  char separator_char;
  char scale_char;

  enum x86_64_isa isa64;
};

struct dis_private {
  bfd_vma insn_start;
  int orig_sizeflag;

  /* Indexes first byte not fetched.  */
  unsigned int fetched;
  uint8_t the_buffer[2 * MAX_CODE_LENGTH - 1];
};

/* Mark parts used in the REX prefix.  When we are testing for
   empty prefix (for 8bit register REX extension), just mask it
   out.  Otherwise test for REX bit is excuse for existence of REX
   only in case value is nonzero.  */
#define USED_REX(value)					\
  {							\
    if (value)						\
      {							\
	if ((ins->rex & value))				\
	  ins->rex_used |= (value) | REX_OPCODE;	\
      }							\
    else						\
      ins->rex_used |= REX_OPCODE;			\
  }


#define EVEX_b_used 1
#define EVEX_len_used 2

/* Flags stored in PREFIXES.  */
#define PREFIX_REPZ 1
#define PREFIX_REPNZ 2
#define PREFIX_CS 4
#define PREFIX_SS 8
#define PREFIX_DS 0x10
#define PREFIX_ES 0x20
#define PREFIX_FS 0x40
#define PREFIX_GS 0x80
#define PREFIX_LOCK 0x100
#define PREFIX_DATA 0x200
#define PREFIX_ADDR 0x400
#define PREFIX_FWAIT 0x800

/* Make sure that bytes from INFO->PRIVATE_DATA->BUFFER (inclusive)
   to ADDR (exclusive) are valid.  Returns true for success, false
   on error.  */
static bool
fetch_code (struct disassemble_info *info, const uint8_t *until)
{
  int status = -1;
  struct dis_private *priv = info->private_data;
  bfd_vma start = priv->insn_start + priv->fetched;
  uint8_t *fetch_end = priv->the_buffer + priv->fetched;
  ptrdiff_t needed = until - fetch_end;

  if (needed <= 0)
    return true;

  if (priv->fetched + (size_t) needed <= ARRAY_SIZE (priv->the_buffer))
    status = (*info->read_memory_func) (start, fetch_end, needed, info);
  if (status != 0)
    {
      /* If we did manage to read at least one byte, then
	 print_insn_i386 will do something sensible.  Otherwise, print
	 an error.  We do that here because this is where we know
	 STATUS.  */
      if (!priv->fetched)
	(*info->memory_error_func) (status, start, info);
      return false;
    }

  priv->fetched += needed;
  return true;
}

static bool
fetch_modrm (instr_info *ins)
{
  if (!fetch_code (ins->info, ins->codep + 1))
    return false;

  ins->modrm.mod = (*ins->codep >> 6) & 3;
  ins->modrm.reg = (*ins->codep >> 3) & 7;
  ins->modrm.rm = *ins->codep & 7;

  return true;
}

static int
fetch_error (const instr_info *ins)
{
  /* Getting here means we tried for data but didn't get it.  That
     means we have an incomplete instruction of some sort.  Just
     print the first byte as a prefix or a .byte pseudo-op.  */
  const struct dis_private *priv = ins->info->private_data;
  const char *name = NULL;

  if (ins->codep <= priv->the_buffer)
    return -1;

  if (ins->prefixes || ins->fwait_prefix >= 0 || (ins->rex & REX_OPCODE))
    name = prefix_name (ins->address_mode, priv->the_buffer[0],
			priv->orig_sizeflag);
  if (name != NULL)
    i386_dis_printf (ins->info, dis_style_mnemonic, "%s", name);
  else
    {
      /* Just print the first byte as a .byte instruction.  */
      i386_dis_printf (ins->info, dis_style_assembler_directive, ".byte ");
      i386_dis_printf (ins->info, dis_style_immediate, "%#x",
		       (unsigned int) priv->the_buffer[0]);
    }

  return 1;
}

/* Possible values for prefix requirement.  */
#define PREFIX_IGNORED_SHIFT	16
#define PREFIX_IGNORED_REPZ	(PREFIX_REPZ << PREFIX_IGNORED_SHIFT)
#define PREFIX_IGNORED_REPNZ	(PREFIX_REPNZ << PREFIX_IGNORED_SHIFT)
#define PREFIX_IGNORED_DATA	(PREFIX_DATA << PREFIX_IGNORED_SHIFT)
#define PREFIX_IGNORED_ADDR	(PREFIX_ADDR << PREFIX_IGNORED_SHIFT)
#define PREFIX_IGNORED_LOCK	(PREFIX_LOCK << PREFIX_IGNORED_SHIFT)

/* Opcode prefixes.  */
#define PREFIX_OPCODE		(PREFIX_REPZ \
				 | PREFIX_REPNZ \
				 | PREFIX_DATA)

/* Prefixes ignored.  */
#define PREFIX_IGNORED		(PREFIX_IGNORED_REPZ \
				 | PREFIX_IGNORED_REPNZ \
				 | PREFIX_IGNORED_DATA)

#define XX { NULL, 0 }
#define Bad_Opcode NULL, { { NULL, 0 } }, 0

#define Eb { OP_E, b_mode }
#define Ebnd { OP_E, bnd_mode }
#define EbS { OP_E, b_swap_mode }
#define EbndS { OP_E, bnd_swap_mode }
#define Ev { OP_E, v_mode }
#define Eva { OP_E, va_mode }
#define Ev_bnd { OP_E, v_bnd_mode }
#define EvS { OP_E, v_swap_mode }
#define Ed { OP_E, d_mode }
#define Edq { OP_E, dq_mode }
#define Edb { OP_E, db_mode }
#define Edw { OP_E, dw_mode }
#define Eq { OP_E, q_mode }
#define indirEv { OP_indirE, indir_v_mode }
#define indirEp { OP_indirE, f_mode }
#define stackEv { OP_E, stack_v_mode }
#define Em { OP_E, m_mode }
#define Ew { OP_E, w_mode }
#define M { OP_M, 0 }		/* lea, lgdt, etc. */
#define Ma { OP_M, a_mode }
#define Mb { OP_M, b_mode }
#define Md { OP_M, d_mode }
#define Mdq { OP_M, dq_mode }
#define Mo { OP_M, o_mode }
#define Mp { OP_M, f_mode }		/* 32 or 48 bit memory operand for LDS, LES etc */
#define Mq { OP_M, q_mode }
#define Mv { OP_M, v_mode }
#define Mv_bnd { OP_M, v_bndmk_mode }
#define Mw { OP_M, w_mode }
#define Mx { OP_M, x_mode }
#define Mxmm { OP_M, xmm_mode }
#define Gb { OP_G, b_mode }
#define Gbnd { OP_G, bnd_mode }
#define Gv { OP_G, v_mode }
#define Gd { OP_G, d_mode }
#define Gdq { OP_G, dq_mode }
#define Gm { OP_G, m_mode }
#define Gva { OP_G, va_mode }
#define Gw { OP_G, w_mode }
#define Ib { OP_I, b_mode }
#define sIb { OP_sI, b_mode }	/* sign extened byte */
#define sIbT { OP_sI, b_T_mode } /* sign extened byte like 'T' */
#define Iv { OP_I, v_mode }
#define sIv { OP_sI, v_mode }
#define Iv64 { OP_I64, v_mode }
#define Id { OP_I, d_mode }
#define Iw { OP_I, w_mode }
#define I1 { OP_I, const_1_mode }
#define Jb { OP_J, b_mode }
#define Jv { OP_J, v_mode }
#define Jdqw { OP_J, dqw_mode }
#define Cm { OP_C, m_mode }
#define Dm { OP_D, m_mode }
#define Td { OP_T, d_mode }
#define Skip_MODRM { OP_Skip_MODRM, 0 }

#define RMeAX { OP_REG, eAX_reg }
#define RMeBX { OP_REG, eBX_reg }
#define RMeCX { OP_REG, eCX_reg }
#define RMeDX { OP_REG, eDX_reg }
#define RMeSP { OP_REG, eSP_reg }
#define RMeBP { OP_REG, eBP_reg }
#define RMeSI { OP_REG, eSI_reg }
#define RMeDI { OP_REG, eDI_reg }
#define RMrAX { OP_REG, rAX_reg }
#define RMrBX { OP_REG, rBX_reg }
#define RMrCX { OP_REG, rCX_reg }
#define RMrDX { OP_REG, rDX_reg }
#define RMrSP { OP_REG, rSP_reg }
#define RMrBP { OP_REG, rBP_reg }
#define RMrSI { OP_REG, rSI_reg }
#define RMrDI { OP_REG, rDI_reg }
#define RMAL { OP_REG, al_reg }
#define RMCL { OP_REG, cl_reg }
#define RMDL { OP_REG, dl_reg }
#define RMBL { OP_REG, bl_reg }
#define RMAH { OP_REG, ah_reg }
#define RMCH { OP_REG, ch_reg }
#define RMDH { OP_REG, dh_reg }
#define RMBH { OP_REG, bh_reg }
#define RMAX { OP_REG, ax_reg }
#define RMDX { OP_REG, dx_reg }

#define eAX { OP_IMREG, eAX_reg }
#define AL { OP_IMREG, al_reg }
#define CL { OP_IMREG, cl_reg }
#define zAX { OP_IMREG, z_mode_ax_reg }
#define indirDX { OP_IMREG, indir_dx_reg }

#define Sw { OP_SEG, w_mode }
#define Sv { OP_SEG, v_mode }
#define Ap { OP_DIR, 0 }
#define Ob { OP_OFF64, b_mode }
#define Ov { OP_OFF64, v_mode }
#define Xb { OP_DSreg, eSI_reg }
#define Xv { OP_DSreg, eSI_reg }
#define Xz { OP_DSreg, eSI_reg }
#define Yb { OP_ESreg, eDI_reg }
#define Yv { OP_ESreg, eDI_reg }
#define DSBX { OP_DSreg, eBX_reg }

#define es { OP_REG, es_reg }
#define ss { OP_REG, ss_reg }
#define cs { OP_REG, cs_reg }
#define ds { OP_REG, ds_reg }
#define fs { OP_REG, fs_reg }
#define gs { OP_REG, gs_reg }

#define MX { OP_MMX, 0 }
#define XM { OP_XMM, 0 }
#define XMScalar { OP_XMM, scalar_mode }
#define XMGatherD { OP_XMM, vex_vsib_d_w_dq_mode }
#define XMGatherQ { OP_XMM, vex_vsib_q_w_dq_mode }
#define XMM { OP_XMM, xmm_mode }
#define TMM { OP_XMM, tmm_mode }
#define XMxmmq { OP_XMM, xmmq_mode }
#define EM { OP_EM, v_mode }
#define EMS { OP_EM, v_swap_mode }
#define EMd { OP_EM, d_mode }
#define EMx { OP_EM, x_mode }
#define EXbwUnit { OP_EX, bw_unit_mode }
#define EXb { OP_EX, b_mode }
#define EXw { OP_EX, w_mode }
#define EXd { OP_EX, d_mode }
#define EXdS { OP_EX, d_swap_mode }
#define EXwS { OP_EX, w_swap_mode }
#define EXq { OP_EX, q_mode }
#define EXqS { OP_EX, q_swap_mode }
#define EXdq { OP_EX, dq_mode }
#define EXx { OP_EX, x_mode }
#define EXxh { OP_EX, xh_mode }
#define EXxS { OP_EX, x_swap_mode }
#define EXxmm { OP_EX, xmm_mode }
#define EXymm { OP_EX, ymm_mode }
#define EXtmm { OP_EX, tmm_mode }
#define EXxmmq { OP_EX, xmmq_mode }
#define EXxmmqh { OP_EX, evex_half_bcst_xmmqh_mode }
#define EXEvexHalfBcstXmmq { OP_EX, evex_half_bcst_xmmq_mode }
#define EXxmmdw { OP_EX, xmmdw_mode }
#define EXxmmqd { OP_EX, xmmqd_mode }
#define EXxmmqdh { OP_EX, evex_half_bcst_xmmqdh_mode }
#define EXymmq { OP_EX, ymmq_mode }
#define EXEvexXGscat { OP_EX, evex_x_gscat_mode }
#define EXEvexXNoBcst { OP_EX, evex_x_nobcst_mode }
#define MS { OP_MS, v_mode }
#define XS { OP_XS, v_mode }
#define EMCq { OP_EMC, q_mode }
#define MXC { OP_MXC, 0 }
#define OPSUF { OP_3DNowSuffix, 0 }
#define SEP { SEP_Fixup, 0 }
#define CMP { CMP_Fixup, 0 }
#define XMM0 { XMM_Fixup, 0 }
#define FXSAVE { FXSAVE_Fixup, 0 }

#define Vex { OP_VEX, x_mode }
#define VexW { OP_VexW, x_mode }
#define VexScalar { OP_VEX, scalar_mode }
#define VexScalarR { OP_VexR, scalar_mode }
#define VexGatherD { OP_VEX, vex_vsib_d_w_dq_mode }
#define VexGatherQ { OP_VEX, vex_vsib_q_w_dq_mode }
#define VexGdq { OP_VEX, dq_mode }
#define VexTmm { OP_VEX, tmm_mode }
#define XMVexI4 { OP_REG_VexI4, x_mode }
#define XMVexScalarI4 { OP_REG_VexI4, scalar_mode }
#define VexI4 { OP_VexI4, 0 }
#define PCLMUL { PCLMUL_Fixup, 0 }
#define VPCMP { VPCMP_Fixup, 0 }
#define VPCOM { VPCOM_Fixup, 0 }

#define EXxEVexR { OP_Rounding, evex_rounding_mode }
#define EXxEVexR64 { OP_Rounding, evex_rounding_64_mode }
#define EXxEVexS { OP_Rounding, evex_sae_mode }

#define MaskG { OP_G, mask_mode }
#define MaskE { OP_E, mask_mode }
#define MaskBDE { OP_E, mask_bd_mode }
#define MaskVex { OP_VEX, mask_mode }

#define MVexVSIBDWpX { OP_M, vex_vsib_d_w_dq_mode }
#define MVexVSIBQWpX { OP_M, vex_vsib_q_w_dq_mode }

#define MVexSIBMEM { OP_M, vex_sibmem_mode }

/* Used handle "rep" prefix for string instructions.  */
#define Xbr { REP_Fixup, eSI_reg }
#define Xvr { REP_Fixup, eSI_reg }
#define Ybr { REP_Fixup, eDI_reg }
#define Yvr { REP_Fixup, eDI_reg }
#define Yzr { REP_Fixup, eDI_reg }
#define indirDXr { REP_Fixup, indir_dx_reg }
#define ALr { REP_Fixup, al_reg }
#define eAXr { REP_Fixup, eAX_reg }

/* Used handle HLE prefix for lockable instructions.  */
#define Ebh1 { HLE_Fixup1, b_mode }
#define Evh1 { HLE_Fixup1, v_mode }
#define Ebh2 { HLE_Fixup2, b_mode }
#define Evh2 { HLE_Fixup2, v_mode }
#define Ebh3 { HLE_Fixup3, b_mode }
#define Evh3 { HLE_Fixup3, v_mode }

#define BND { BND_Fixup, 0 }
#define NOTRACK { NOTRACK_Fixup, 0 }

#define cond_jump_flag { NULL, cond_jump_mode }
#define loop_jcxz_flag { NULL, loop_jcxz_mode }

/* bits in sizeflag */
#define SUFFIX_ALWAYS 4
#define AFLAG 2
#define DFLAG 1

enum
{
  /* byte operand */
  b_mode = 1,
  /* byte operand with operand swapped */
  b_swap_mode,
  /* byte operand, sign extend like 'T' suffix */
  b_T_mode,
  /* operand size depends on prefixes */
  v_mode,
  /* operand size depends on prefixes with operand swapped */
  v_swap_mode,
  /* operand size depends on address prefix */
  va_mode,
  /* word operand */
  w_mode,
  /* double word operand  */
  d_mode,
  /* word operand with operand swapped  */
  w_swap_mode,
  /* double word operand with operand swapped */
  d_swap_mode,
  /* quad word operand */
  q_mode,
  /* quad word operand with operand swapped */
  q_swap_mode,
  /* ten-byte operand */
  t_mode,
  /* 16-byte XMM, 32-byte YMM or 64-byte ZMM operand.  In EVEX with
     broadcast enabled.  */
  x_mode,
  /* Similar to x_mode, but with different EVEX mem shifts.  */
  evex_x_gscat_mode,
  /* Similar to x_mode, but with yet different EVEX mem shifts.  */
  bw_unit_mode,
  /* Similar to x_mode, but with disabled broadcast.  */
  evex_x_nobcst_mode,
  /* Similar to x_mode, but with operands swapped and disabled broadcast
     in EVEX.  */
  x_swap_mode,
  /* 16-byte XMM, 32-byte YMM or 64-byte ZMM operand.  In EVEX with
     broadcast of 16bit enabled.  */
  xh_mode,
  /* 16-byte XMM operand */
  xmm_mode,
  /* XMM, XMM or YMM register operand, or quad word, xmmword or ymmword
     memory operand (depending on vector length).  Broadcast isn't
     allowed.  */
  xmmq_mode,
  /* Same as xmmq_mode, but broadcast is allowed.  */
  evex_half_bcst_xmmq_mode,
  /* XMM, XMM or YMM register operand, or quad word, xmmword or ymmword
     memory operand (depending on vector length).  16bit broadcast.  */
  evex_half_bcst_xmmqh_mode,
  /* 16-byte XMM, word, double word or quad word operand.  */
  xmmdw_mode,
  /* 16-byte XMM, double word, quad word operand or xmm word operand.  */
  xmmqd_mode,
  /* 16-byte XMM, double word, quad word operand or xmm word operand.
     16bit broadcast.  */
  evex_half_bcst_xmmqdh_mode,
  /* 32-byte YMM operand */
  ymm_mode,
  /* quad word, ymmword or zmmword memory operand.  */
  ymmq_mode,
  /* TMM operand */
  tmm_mode,
  /* d_mode in 32bit, q_mode in 64bit mode.  */
  m_mode,
  /* pair of v_mode operands */
  a_mode,
  cond_jump_mode,
  loop_jcxz_mode,
  movsxd_mode,
  v_bnd_mode,
  /* like v_bnd_mode in 32bit, no RIP-rel in 64bit mode.  */
  v_bndmk_mode,
  /* operand size depends on REX.W / VEX.W.  */
  dq_mode,
  /* Displacements like v_mode without considering Intel64 ISA.  */
  dqw_mode,
  /* bounds operand */
  bnd_mode,
  /* bounds operand with operand swapped */
  bnd_swap_mode,
  /* 4- or 6-byte pointer operand */
  f_mode,
  const_1_mode,
  /* v_mode for indirect branch opcodes.  */
  indir_v_mode,
  /* v_mode for stack-related opcodes.  */
  stack_v_mode,
  /* non-quad operand size depends on prefixes */
  z_mode,
  /* 16-byte operand */
  o_mode,
  /* registers like d_mode, memory like b_mode.  */
  db_mode,
  /* registers like d_mode, memory like w_mode.  */
  dw_mode,

  /* Operand size depends on the VEX.W bit, with VSIB dword indices.  */
  vex_vsib_d_w_dq_mode,
  /* Operand size depends on the VEX.W bit, with VSIB qword indices.  */
  vex_vsib_q_w_dq_mode,
  /* mandatory non-vector SIB.  */
  vex_sibmem_mode,

  /* scalar, ignore vector length.  */
  scalar_mode,

  /* Static rounding.  */
  evex_rounding_mode,
  /* Static rounding, 64-bit mode only.  */
  evex_rounding_64_mode,
  /* Supress all exceptions.  */
  evex_sae_mode,

  /* Mask register operand.  */
  mask_mode,
  /* Mask register operand.  */
  mask_bd_mode,

  es_reg,
  cs_reg,
  ss_reg,
  ds_reg,
  fs_reg,
  gs_reg,

  eAX_reg,
  eCX_reg,
  eDX_reg,
  eBX_reg,
  eSP_reg,
  eBP_reg,
  eSI_reg,
  eDI_reg,

  al_reg,
  cl_reg,
  dl_reg,
  bl_reg,
  ah_reg,
  ch_reg,
  dh_reg,
  bh_reg,

  ax_reg,
  cx_reg,
  dx_reg,
  bx_reg,
  sp_reg,
  bp_reg,
  si_reg,
  di_reg,

  rAX_reg,
  rCX_reg,
  rDX_reg,
  rBX_reg,
  rSP_reg,
  rBP_reg,
  rSI_reg,
  rDI_reg,

  z_mode_ax_reg,
  indir_dx_reg
};

enum
{
  FLOATCODE = 1,
  USE_REG_TABLE,
  USE_MOD_TABLE,
  USE_RM_TABLE,
  USE_PREFIX_TABLE,
  USE_X86_64_TABLE,
  USE_3BYTE_TABLE,
  USE_XOP_8F_TABLE,
  USE_VEX_C4_TABLE,
  USE_VEX_C5_TABLE,
  USE_VEX_LEN_TABLE,
  USE_VEX_W_TABLE,
  USE_EVEX_TABLE,
  USE_EVEX_LEN_TABLE
};

#define FLOAT			NULL, { { NULL, FLOATCODE } }, 0

#define DIS386(T, I)		NULL, { { NULL, (T)}, { NULL,  (I) } }, 0
#define DIS386_PREFIX(T, I, P)		NULL, { { NULL, (T)}, { NULL,  (I) } }, P
#define REG_TABLE(I)		DIS386 (USE_REG_TABLE, (I))
#define MOD_TABLE(I)		DIS386 (USE_MOD_TABLE, (I))
#define RM_TABLE(I)		DIS386 (USE_RM_TABLE, (I))
#define PREFIX_TABLE(I)		DIS386 (USE_PREFIX_TABLE, (I))
#define X86_64_TABLE(I)		DIS386 (USE_X86_64_TABLE, (I))
#define THREE_BYTE_TABLE(I)	DIS386 (USE_3BYTE_TABLE, (I))
#define THREE_BYTE_TABLE_PREFIX(I, P)	DIS386_PREFIX (USE_3BYTE_TABLE, (I), P)
#define XOP_8F_TABLE(I)		DIS386 (USE_XOP_8F_TABLE, (I))
#define VEX_C4_TABLE(I)		DIS386 (USE_VEX_C4_TABLE, (I))
#define VEX_C5_TABLE(I)		DIS386 (USE_VEX_C5_TABLE, (I))
#define VEX_LEN_TABLE(I)	DIS386 (USE_VEX_LEN_TABLE, (I))
#define VEX_W_TABLE(I)		DIS386 (USE_VEX_W_TABLE, (I))
#define EVEX_TABLE(I)		DIS386 (USE_EVEX_TABLE, (I))
#define EVEX_LEN_TABLE(I)	DIS386 (USE_EVEX_LEN_TABLE, (I))

enum
{
  REG_80 = 0,
  REG_81,
  REG_83,
  REG_8F,
  REG_C0,
  REG_C1,
  REG_C6,
  REG_C7,
  REG_D0,
  REG_D1,
  REG_D2,
  REG_D3,
  REG_F6,
  REG_F7,
  REG_FE,
  REG_FF,
  REG_0F00,
  REG_0F01,
  REG_0F0D,
  REG_0F18,
  REG_0F1C_P_0_MOD_0,
  REG_0F1E_P_1_MOD_3,
  REG_0F38D8_PREFIX_1,
  REG_0F3A0F_PREFIX_1_MOD_3,
  REG_0F71_MOD_0,
  REG_0F72_MOD_0,
  REG_0F73_MOD_0,
  REG_0FA6,
  REG_0FA7,
  REG_0FAE,
  REG_0FBA,
  REG_0FC7,
  REG_VEX_0F71_M_0,
  REG_VEX_0F72_M_0,
  REG_VEX_0F73_M_0,
  REG_VEX_0FAE,
  REG_VEX_0F3849_X86_64_L_0_W_0_M_1_P_0,
  REG_VEX_0F38F3_L_0,

  REG_XOP_09_01_L_0,
  REG_XOP_09_02_L_0,
  REG_XOP_09_12_M_1_L_0,
  REG_XOP_0A_12_L_0,

  REG_EVEX_0F71,
  REG_EVEX_0F72,
  REG_EVEX_0F73,
  REG_EVEX_0F38C6_M_0_L_2,
  REG_EVEX_0F38C7_M_0_L_2
};

enum
{
  MOD_62_32BIT = 0,
  MOD_8D,
  MOD_C4_32BIT,
  MOD_C5_32BIT,
  MOD_C6_REG_7,
  MOD_C7_REG_7,
  MOD_FF_REG_3,
  MOD_FF_REG_5,
  MOD_0F01_REG_0,
  MOD_0F01_REG_1,
  MOD_0F01_REG_2,
  MOD_0F01_REG_3,
  MOD_0F01_REG_5,
  MOD_0F01_REG_7,
  MOD_0F02,
  MOD_0F03,
  MOD_0F12_PREFIX_0,
  MOD_0F12_PREFIX_2,
  MOD_0F13,
  MOD_0F16_PREFIX_0,
  MOD_0F16_PREFIX_2,
  MOD_0F17,
  MOD_0F18_REG_0,
  MOD_0F18_REG_1,
  MOD_0F18_REG_2,
  MOD_0F18_REG_3,
  MOD_0F18_REG_6,
  MOD_0F18_REG_7,
  MOD_0F1A_PREFIX_0,
  MOD_0F1B_PREFIX_0,
  MOD_0F1B_PREFIX_1,
  MOD_0F1C_PREFIX_0,
  MOD_0F1E_PREFIX_1,
  MOD_0F2B_PREFIX_0,
  MOD_0F2B_PREFIX_1,
  MOD_0F2B_PREFIX_2,
  MOD_0F2B_PREFIX_3,
  MOD_0F50,
  MOD_0F71,
  MOD_0F72,
  MOD_0F73,
  MOD_0FAE_REG_0,
  MOD_0FAE_REG_1,
  MOD_0FAE_REG_2,
  MOD_0FAE_REG_3,
  MOD_0FAE_REG_4,
  MOD_0FAE_REG_5,
  MOD_0FAE_REG_6,
  MOD_0FAE_REG_7,
  MOD_0FB2,
  MOD_0FB4,
  MOD_0FB5,
  MOD_0FC3,
  MOD_0FC7_REG_3,
  MOD_0FC7_REG_4,
  MOD_0FC7_REG_5,
  MOD_0FC7_REG_6,
  MOD_0FC7_REG_7,
  MOD_0FD7,
  MOD_0FE7_PREFIX_2,
  MOD_0FF0_PREFIX_3,
  MOD_0F382A,
  MOD_0F38DC_PREFIX_1,
  MOD_0F38DD_PREFIX_1,
  MOD_0F38DE_PREFIX_1,
  MOD_0F38DF_PREFIX_1,
  MOD_0F38F5,
  MOD_0F38F6_PREFIX_0,
  MOD_0F38F8_PREFIX_1,
  MOD_0F38F8_PREFIX_2,
  MOD_0F38F8_PREFIX_3,
  MOD_0F38F9,
  MOD_0F38FA_PREFIX_1,
  MOD_0F38FB_PREFIX_1,
  MOD_0F3A0F_PREFIX_1,

  MOD_VEX_0F12_PREFIX_0,
  MOD_VEX_0F12_PREFIX_2,
  MOD_VEX_0F13,
  MOD_VEX_0F16_PREFIX_0,
  MOD_VEX_0F16_PREFIX_2,
  MOD_VEX_0F17,
  MOD_VEX_0F2B,
  MOD_VEX_0F41_L_1,
  MOD_VEX_0F42_L_1,
  MOD_VEX_0F44_L_0,
  MOD_VEX_0F45_L_1,
  MOD_VEX_0F46_L_1,
  MOD_VEX_0F47_L_1,
  MOD_VEX_0F4A_L_1,
  MOD_VEX_0F4B_L_1,
  MOD_VEX_0F50,
  MOD_VEX_0F71,
  MOD_VEX_0F72,
  MOD_VEX_0F73,
  MOD_VEX_0F91_L_0,
  MOD_VEX_0F92_L_0,
  MOD_VEX_0F93_L_0,
  MOD_VEX_0F98_L_0,
  MOD_VEX_0F99_L_0,
  MOD_VEX_0FAE_REG_2,
  MOD_VEX_0FAE_REG_3,
  MOD_VEX_0FD7,
  MOD_VEX_0FE7,
  MOD_VEX_0FF0_PREFIX_3,
  MOD_VEX_0F381A,
  MOD_VEX_0F382A,
  MOD_VEX_0F382C,
  MOD_VEX_0F382D,
  MOD_VEX_0F382E,
  MOD_VEX_0F382F,
  MOD_VEX_0F3849_X86_64_L_0_W_0,
  MOD_VEX_0F384B_X86_64_L_0_W_0,
  MOD_VEX_0F385A,
  MOD_VEX_0F385C_X86_64,
  MOD_VEX_0F385E_X86_64,
  MOD_VEX_0F386C_X86_64,
  MOD_VEX_0F388C,
  MOD_VEX_0F388E,
  MOD_VEX_0F3A30_L_0,
  MOD_VEX_0F3A31_L_0,
  MOD_VEX_0F3A32_L_0,
  MOD_VEX_0F3A33_L_0,

  MOD_XOP_09_12,

  MOD_EVEX_0F381A,
  MOD_EVEX_0F381B,
  MOD_EVEX_0F3828_P_1,
  MOD_EVEX_0F382A_P_1_W_1,
  MOD_EVEX_0F3838_P_1,
  MOD_EVEX_0F383A_P_1_W_0,
  MOD_EVEX_0F385A,
  MOD_EVEX_0F385B,
  MOD_EVEX_0F387A_W_0,
  MOD_EVEX_0F387B_W_0,
  MOD_EVEX_0F387C,
  MOD_EVEX_0F38C6,
  MOD_EVEX_0F38C7,
};

enum
{
  RM_C6_REG_7 = 0,
  RM_C7_REG_7,
  RM_0F01_REG_0,
  RM_0F01_REG_1,
  RM_0F01_REG_2,
  RM_0F01_REG_3,
  RM_0F01_REG_5_MOD_3,
  RM_0F01_REG_7_MOD_3,
  RM_0F1E_P_1_MOD_3_REG_7,
  RM_0FAE_REG_6_MOD_3_P_0,
  RM_0FAE_REG_7_MOD_3,
  RM_0F3A0F_P_1_MOD_3_REG_0,

  RM_VEX_0F3849_X86_64_L_0_W_0_M_1_P_0_R_0,
  RM_VEX_0F3849_X86_64_L_0_W_0_M_1_P_3,
};

enum
{
  PREFIX_90 = 0,
  PREFIX_0F00_REG_6_X86_64,
  PREFIX_0F01_REG_0_MOD_3_RM_6,
  PREFIX_0F01_REG_1_RM_2,
  PREFIX_0F01_REG_1_RM_4,
  PREFIX_0F01_REG_1_RM_5,
  PREFIX_0F01_REG_1_RM_6,
  PREFIX_0F01_REG_1_RM_7,
  PREFIX_0F01_REG_3_RM_1,
  PREFIX_0F01_REG_5_MOD_0,
  PREFIX_0F01_REG_5_MOD_3_RM_0,
  PREFIX_0F01_REG_5_MOD_3_RM_1,
  PREFIX_0F01_REG_5_MOD_3_RM_2,
  PREFIX_0F01_REG_5_MOD_3_RM_4,
  PREFIX_0F01_REG_5_MOD_3_RM_5,
  PREFIX_0F01_REG_5_MOD_3_RM_6,
  PREFIX_0F01_REG_5_MOD_3_RM_7,
  PREFIX_0F01_REG_7_MOD_3_RM_2,
  PREFIX_0F01_REG_7_MOD_3_RM_5,
  PREFIX_0F01_REG_7_MOD_3_RM_6,
  PREFIX_0F01_REG_7_MOD_3_RM_7,
  PREFIX_0F09,
  PREFIX_0F10,
  PREFIX_0F11,
  PREFIX_0F12,
  PREFIX_0F16,
  PREFIX_0F18_REG_6_MOD_0_X86_64,
  PREFIX_0F18_REG_7_MOD_0_X86_64,
  PREFIX_0F1A,
  PREFIX_0F1B,
  PREFIX_0F1C,
  PREFIX_0F1E,
  PREFIX_0F2A,
  PREFIX_0F2B,
  PREFIX_0F2C,
  PREFIX_0F2D,
  PREFIX_0F2E,
  PREFIX_0F2F,
  PREFIX_0F51,
  PREFIX_0F52,
  PREFIX_0F53,
  PREFIX_0F58,
  PREFIX_0F59,
  PREFIX_0F5A,
  PREFIX_0F5B,
  PREFIX_0F5C,
  PREFIX_0F5D,
  PREFIX_0F5E,
  PREFIX_0F5F,
  PREFIX_0F60,
  PREFIX_0F61,
  PREFIX_0F62,
  PREFIX_0F6F,
  PREFIX_0F70,
  PREFIX_0F78,
  PREFIX_0F79,
  PREFIX_0F7C,
  PREFIX_0F7D,
  PREFIX_0F7E,
  PREFIX_0F7F,
  PREFIX_0FAE_REG_0_MOD_3,
  PREFIX_0FAE_REG_1_MOD_3,
  PREFIX_0FAE_REG_2_MOD_3,
  PREFIX_0FAE_REG_3_MOD_3,
  PREFIX_0FAE_REG_4_MOD_0,
  PREFIX_0FAE_REG_4_MOD_3,
  PREFIX_0FAE_REG_5_MOD_3,
  PREFIX_0FAE_REG_6_MOD_0,
  PREFIX_0FAE_REG_6_MOD_3,
  PREFIX_0FAE_REG_7_MOD_0,
  PREFIX_0FB8,
  PREFIX_0FBC,
  PREFIX_0FBD,
  PREFIX_0FC2,
  PREFIX_0FC7_REG_6_MOD_0,
  PREFIX_0FC7_REG_6_MOD_3,
  PREFIX_0FC7_REG_7_MOD_3,
  PREFIX_0FD0,
  PREFIX_0FD6,
  PREFIX_0FE6,
  PREFIX_0FE7,
  PREFIX_0FF0,
  PREFIX_0FF7,
  PREFIX_0F38D8,
  PREFIX_0F38DC,
  PREFIX_0F38DD,
  PREFIX_0F38DE,
  PREFIX_0F38DF,
  PREFIX_0F38F0,
  PREFIX_0F38F1,
  PREFIX_0F38F6,
  PREFIX_0F38F8,
  PREFIX_0F38FA,
  PREFIX_0F38FB,
  PREFIX_0F38FC,
  PREFIX_0F3A0F,
  PREFIX_VEX_0F10,
  PREFIX_VEX_0F11,
  PREFIX_VEX_0F12,
  PREFIX_VEX_0F16,
  PREFIX_VEX_0F2A,
  PREFIX_VEX_0F2C,
  PREFIX_VEX_0F2D,
  PREFIX_VEX_0F2E,
  PREFIX_VEX_0F2F,
  PREFIX_VEX_0F41_L_1_M_1_W_0,
  PREFIX_VEX_0F41_L_1_M_1_W_1,
  PREFIX_VEX_0F42_L_1_M_1_W_0,
  PREFIX_VEX_0F42_L_1_M_1_W_1,
  PREFIX_VEX_0F44_L_0_M_1_W_0,
  PREFIX_VEX_0F44_L_0_M_1_W_1,
  PREFIX_VEX_0F45_L_1_M_1_W_0,
  PREFIX_VEX_0F45_L_1_M_1_W_1,
  PREFIX_VEX_0F46_L_1_M_1_W_0,
  PREFIX_VEX_0F46_L_1_M_1_W_1,
  PREFIX_VEX_0F47_L_1_M_1_W_0,
  PREFIX_VEX_0F47_L_1_M_1_W_1,
  PREFIX_VEX_0F4A_L_1_M_1_W_0,
  PREFIX_VEX_0F4A_L_1_M_1_W_1,
  PREFIX_VEX_0F4B_L_1_M_1_W_0,
  PREFIX_VEX_0F4B_L_1_M_1_W_1,
  PREFIX_VEX_0F51,
  PREFIX_VEX_0F52,
  PREFIX_VEX_0F53,
  PREFIX_VEX_0F58,
  PREFIX_VEX_0F59,
  PREFIX_VEX_0F5A,
  PREFIX_VEX_0F5B,
  PREFIX_VEX_0F5C,
  PREFIX_VEX_0F5D,
  PREFIX_VEX_0F5E,
  PREFIX_VEX_0F5F,
  PREFIX_VEX_0F6F,
  PREFIX_VEX_0F70,
  PREFIX_VEX_0F7C,
  PREFIX_VEX_0F7D,
  PREFIX_VEX_0F7E,
  PREFIX_VEX_0F7F,
  PREFIX_VEX_0F90_L_0_W_0,
  PREFIX_VEX_0F90_L_0_W_1,
  PREFIX_VEX_0F91_L_0_M_0_W_0,
  PREFIX_VEX_0F91_L_0_M_0_W_1,
  PREFIX_VEX_0F92_L_0_M_1_W_0,
  PREFIX_VEX_0F92_L_0_M_1_W_1,
  PREFIX_VEX_0F93_L_0_M_1_W_0,
  PREFIX_VEX_0F93_L_0_M_1_W_1,
  PREFIX_VEX_0F98_L_0_M_1_W_0,
  PREFIX_VEX_0F98_L_0_M_1_W_1,
  PREFIX_VEX_0F99_L_0_M_1_W_0,
  PREFIX_VEX_0F99_L_0_M_1_W_1,
  PREFIX_VEX_0FC2,
  PREFIX_VEX_0FD0,
  PREFIX_VEX_0FE6,
  PREFIX_VEX_0FF0,
  PREFIX_VEX_0F3849_X86_64_L_0_W_0_M_0,
  PREFIX_VEX_0F3849_X86_64_L_0_W_0_M_1,
  PREFIX_VEX_0F384B_X86_64_L_0_W_0_M_0,
  PREFIX_VEX_0F3850_W_0,
  PREFIX_VEX_0F3851_W_0,
  PREFIX_VEX_0F385C_X86_64_M_1_L_0_W_0,
  PREFIX_VEX_0F385E_X86_64_M_1_L_0_W_0,
  PREFIX_VEX_0F386C_X86_64_M_1_L_0_W_0,
  PREFIX_VEX_0F3872,
  PREFIX_VEX_0F38B0_W_0,
  PREFIX_VEX_0F38B1_W_0,
  PREFIX_VEX_0F38F5_L_0,
  PREFIX_VEX_0F38F6_L_0,
  PREFIX_VEX_0F38F7_L_0,
  PREFIX_VEX_0F3AF0_L_0,

  PREFIX_EVEX_0F5B,
  PREFIX_EVEX_0F6F,
  PREFIX_EVEX_0F70,
  PREFIX_EVEX_0F78,
  PREFIX_EVEX_0F79,
  PREFIX_EVEX_0F7A,
  PREFIX_EVEX_0F7B,
  PREFIX_EVEX_0F7E,
  PREFIX_EVEX_0F7F,
  PREFIX_EVEX_0FC2,
  PREFIX_EVEX_0FE6,
  PREFIX_EVEX_0F3810,
  PREFIX_EVEX_0F3811,
  PREFIX_EVEX_0F3812,
  PREFIX_EVEX_0F3813,
  PREFIX_EVEX_0F3814,
  PREFIX_EVEX_0F3815,
  PREFIX_EVEX_0F3820,
  PREFIX_EVEX_0F3821,
  PREFIX_EVEX_0F3822,
  PREFIX_EVEX_0F3823,
  PREFIX_EVEX_0F3824,
  PREFIX_EVEX_0F3825,
  PREFIX_EVEX_0F3826,
  PREFIX_EVEX_0F3827,
  PREFIX_EVEX_0F3828,
  PREFIX_EVEX_0F3829,
  PREFIX_EVEX_0F382A,
  PREFIX_EVEX_0F3830,
  PREFIX_EVEX_0F3831,
  PREFIX_EVEX_0F3832,
  PREFIX_EVEX_0F3833,
  PREFIX_EVEX_0F3834,
  PREFIX_EVEX_0F3835,
  PREFIX_EVEX_0F3838,
  PREFIX_EVEX_0F3839,
  PREFIX_EVEX_0F383A,
  PREFIX_EVEX_0F3852,
  PREFIX_EVEX_0F3853,
  PREFIX_EVEX_0F3868,
  PREFIX_EVEX_0F3872,
  PREFIX_EVEX_0F389A,
  PREFIX_EVEX_0F389B,
  PREFIX_EVEX_0F38AA,
  PREFIX_EVEX_0F38AB,

  PREFIX_EVEX_0F3A08,
  PREFIX_EVEX_0F3A0A,
  PREFIX_EVEX_0F3A26,
  PREFIX_EVEX_0F3A27,
  PREFIX_EVEX_0F3A56,
  PREFIX_EVEX_0F3A57,
  PREFIX_EVEX_0F3A66,
  PREFIX_EVEX_0F3A67,
  PREFIX_EVEX_0F3AC2,

  PREFIX_EVEX_MAP5_10,
  PREFIX_EVEX_MAP5_11,
  PREFIX_EVEX_MAP5_1D,
  PREFIX_EVEX_MAP5_2A,
  PREFIX_EVEX_MAP5_2C,
  PREFIX_EVEX_MAP5_2D,
  PREFIX_EVEX_MAP5_2E,
  PREFIX_EVEX_MAP5_2F,
  PREFIX_EVEX_MAP5_51,
  PREFIX_EVEX_MAP5_58,
  PREFIX_EVEX_MAP5_59,
  PREFIX_EVEX_MAP5_5A,
  PREFIX_EVEX_MAP5_5B,
  PREFIX_EVEX_MAP5_5C,
  PREFIX_EVEX_MAP5_5D,
  PREFIX_EVEX_MAP5_5E,
  PREFIX_EVEX_MAP5_5F,
  PREFIX_EVEX_MAP5_78,
  PREFIX_EVEX_MAP5_79,
  PREFIX_EVEX_MAP5_7A,
  PREFIX_EVEX_MAP5_7B,
  PREFIX_EVEX_MAP5_7C,
  PREFIX_EVEX_MAP5_7D,

  PREFIX_EVEX_MAP6_13,
  PREFIX_EVEX_MAP6_56,
  PREFIX_EVEX_MAP6_57,
  PREFIX_EVEX_MAP6_D6,
  PREFIX_EVEX_MAP6_D7,
};

enum
{
  X86_64_06 = 0,
  X86_64_07,
  X86_64_0E,
  X86_64_16,
  X86_64_17,
  X86_64_1E,
  X86_64_1F,
  X86_64_27,
  X86_64_2F,
  X86_64_37,
  X86_64_3F,
  X86_64_60,
  X86_64_61,
  X86_64_62,
  X86_64_63,
  X86_64_6D,
  X86_64_6F,
  X86_64_82,
  X86_64_9A,
  X86_64_C2,
  X86_64_C3,
  X86_64_C4,
  X86_64_C5,
  X86_64_CE,
  X86_64_D4,
  X86_64_D5,
  X86_64_E8,
  X86_64_E9,
  X86_64_EA,
  X86_64_0F00_REG_6,
  X86_64_0F01_REG_0,
  X86_64_0F01_REG_0_MOD_3_RM_6_P_1,
  X86_64_0F01_REG_0_MOD_3_RM_6_P_3,
  X86_64_0F01_REG_1,
  X86_64_0F01_REG_1_RM_2_PREFIX_1,
  X86_64_0F01_REG_1_RM_2_PREFIX_3,
  X86_64_0F01_REG_1_RM_5_PREFIX_2,
  X86_64_0F01_REG_1_RM_6_PREFIX_2,
  X86_64_0F01_REG_1_RM_7_PREFIX_2,
  X86_64_0F01_REG_2,
  X86_64_0F01_REG_3,
  X86_64_0F01_REG_5_MOD_3_RM_4_PREFIX_1,
  X86_64_0F01_REG_5_MOD_3_RM_5_PREFIX_1,
  X86_64_0F01_REG_5_MOD_3_RM_6_PREFIX_1,
  X86_64_0F01_REG_5_MOD_3_RM_7_PREFIX_1,
  X86_64_0F01_REG_7_MOD_3_RM_5_PREFIX_1,
  X86_64_0F01_REG_7_MOD_3_RM_6_PREFIX_1,
  X86_64_0F01_REG_7_MOD_3_RM_6_PREFIX_3,
  X86_64_0F01_REG_7_MOD_3_RM_7_PREFIX_1,
  X86_64_0F18_REG_6_MOD_0,
  X86_64_0F18_REG_7_MOD_0,
  X86_64_0F24,
  X86_64_0F26,
  X86_64_0FC7_REG_6_MOD_3_PREFIX_1,

  X86_64_VEX_0F3849,
  X86_64_VEX_0F384B,
  X86_64_VEX_0F385C,
  X86_64_VEX_0F385E,
  X86_64_VEX_0F386C,
  X86_64_VEX_0F38E0,
  X86_64_VEX_0F38E1,
  X86_64_VEX_0F38E2,
  X86_64_VEX_0F38E3,
  X86_64_VEX_0F38E4,
  X86_64_VEX_0F38E5,
  X86_64_VEX_0F38E6,
  X86_64_VEX_0F38E7,
  X86_64_VEX_0F38E8,
  X86_64_VEX_0F38E9,
  X86_64_VEX_0F38EA,
  X86_64_VEX_0F38EB,
  X86_64_VEX_0F38EC,
  X86_64_VEX_0F38ED,
  X86_64_VEX_0F38EE,
  X86_64_VEX_0F38EF,
};

enum
{
  THREE_BYTE_0F38 = 0,
  THREE_BYTE_0F3A
};

enum
{
  XOP_08 = 0,
  XOP_09,
  XOP_0A
};

enum
{
  VEX_0F = 0,
  VEX_0F38,
  VEX_0F3A
};

enum
{
  EVEX_0F = 0,
  EVEX_0F38,
  EVEX_0F3A,
  EVEX_MAP5,
  EVEX_MAP6,
};

enum
{
  VEX_LEN_0F12_P_0_M_0 = 0,
  VEX_LEN_0F12_P_0_M_1,
#define VEX_LEN_0F12_P_2_M_0 VEX_LEN_0F12_P_0_M_0
  VEX_LEN_0F13_M_0,
  VEX_LEN_0F16_P_0_M_0,
  VEX_LEN_0F16_P_0_M_1,
#define VEX_LEN_0F16_P_2_M_0 VEX_LEN_0F16_P_0_M_0
  VEX_LEN_0F17_M_0,
  VEX_LEN_0F41,
  VEX_LEN_0F42,
  VEX_LEN_0F44,
  VEX_LEN_0F45,
  VEX_LEN_0F46,
  VEX_LEN_0F47,
  VEX_LEN_0F4A,
  VEX_LEN_0F4B,
  VEX_LEN_0F6E,
  VEX_LEN_0F77,
  VEX_LEN_0F7E_P_1,
  VEX_LEN_0F7E_P_2,
  VEX_LEN_0F90,
  VEX_LEN_0F91,
  VEX_LEN_0F92,
  VEX_LEN_0F93,
  VEX_LEN_0F98,
  VEX_LEN_0F99,
  VEX_LEN_0FAE_R_2_M_0,
  VEX_LEN_0FAE_R_3_M_0,
  VEX_LEN_0FC4,
  VEX_LEN_0FC5,
  VEX_LEN_0FD6,
  VEX_LEN_0FF7,
  VEX_LEN_0F3816,
  VEX_LEN_0F3819,
  VEX_LEN_0F381A_M_0,
  VEX_LEN_0F3836,
  VEX_LEN_0F3841,
  VEX_LEN_0F3849_X86_64,
  VEX_LEN_0F384B_X86_64,
  VEX_LEN_0F385A_M_0,
  VEX_LEN_0F385C_X86_64_M_1,
  VEX_LEN_0F385E_X86_64_M_1,
  VEX_LEN_0F386C_X86_64_M_1,
  VEX_LEN_0F38DB,
  VEX_LEN_0F38F2,
  VEX_LEN_0F38F3,
  VEX_LEN_0F38F5,
  VEX_LEN_0F38F6,
  VEX_LEN_0F38F7,
  VEX_LEN_0F3A00,
  VEX_LEN_0F3A01,
  VEX_LEN_0F3A06,
  VEX_LEN_0F3A14,
  VEX_LEN_0F3A15,
  VEX_LEN_0F3A16,
  VEX_LEN_0F3A17,
  VEX_LEN_0F3A18,
  VEX_LEN_0F3A19,
  VEX_LEN_0F3A20,
  VEX_LEN_0F3A21,
  VEX_LEN_0F3A22,
  VEX_LEN_0F3A30,
  VEX_LEN_0F3A31,
  VEX_LEN_0F3A32,
  VEX_LEN_0F3A33,
  VEX_LEN_0F3A38,
  VEX_LEN_0F3A39,
  VEX_LEN_0F3A41,
  VEX_LEN_0F3A46,
  VEX_LEN_0F3A60,
  VEX_LEN_0F3A61,
  VEX_LEN_0F3A62,
  VEX_LEN_0F3A63,
  VEX_LEN_0F3ADF,
  VEX_LEN_0F3AF0,
  VEX_LEN_0FXOP_08_85,
  VEX_LEN_0FXOP_08_86,
  VEX_LEN_0FXOP_08_87,
  VEX_LEN_0FXOP_08_8E,
  VEX_LEN_0FXOP_08_8F,
  VEX_LEN_0FXOP_08_95,
  VEX_LEN_0FXOP_08_96,
  VEX_LEN_0FXOP_08_97,
  VEX_LEN_0FXOP_08_9E,
  VEX_LEN_0FXOP_08_9F,
  VEX_LEN_0FXOP_08_A3,
  VEX_LEN_0FXOP_08_A6,
  VEX_LEN_0FXOP_08_B6,
  VEX_LEN_0FXOP_08_C0,
  VEX_LEN_0FXOP_08_C1,
  VEX_LEN_0FXOP_08_C2,
  VEX_LEN_0FXOP_08_C3,
  VEX_LEN_0FXOP_08_CC,
  VEX_LEN_0FXOP_08_CD,
  VEX_LEN_0FXOP_08_CE,
  VEX_LEN_0FXOP_08_CF,
  VEX_LEN_0FXOP_08_EC,
  VEX_LEN_0FXOP_08_ED,
  VEX_LEN_0FXOP_08_EE,
  VEX_LEN_0FXOP_08_EF,
  VEX_LEN_0FXOP_09_01,
  VEX_LEN_0FXOP_09_02,
  VEX_LEN_0FXOP_09_12_M_1,
  VEX_LEN_0FXOP_09_82_W_0,
  VEX_LEN_0FXOP_09_83_W_0,
  VEX_LEN_0FXOP_09_90,
  VEX_LEN_0FXOP_09_91,
  VEX_LEN_0FXOP_09_92,
  VEX_LEN_0FXOP_09_93,
  VEX_LEN_0FXOP_09_94,
  VEX_LEN_0FXOP_09_95,
  VEX_LEN_0FXOP_09_96,
  VEX_LEN_0FXOP_09_97,
  VEX_LEN_0FXOP_09_98,
  VEX_LEN_0FXOP_09_99,
  VEX_LEN_0FXOP_09_9A,
  VEX_LEN_0FXOP_09_9B,
  VEX_LEN_0FXOP_09_C1,
  VEX_LEN_0FXOP_09_C2,
  VEX_LEN_0FXOP_09_C3,
  VEX_LEN_0FXOP_09_C6,
  VEX_LEN_0FXOP_09_C7,
  VEX_LEN_0FXOP_09_CB,
  VEX_LEN_0FXOP_09_D1,
  VEX_LEN_0FXOP_09_D2,
  VEX_LEN_0FXOP_09_D3,
  VEX_LEN_0FXOP_09_D6,
  VEX_LEN_0FXOP_09_D7,
  VEX_LEN_0FXOP_09_DB,
  VEX_LEN_0FXOP_09_E1,
  VEX_LEN_0FXOP_09_E2,
  VEX_LEN_0FXOP_09_E3,
  VEX_LEN_0FXOP_0A_12,
};

enum
{
  EVEX_LEN_0F3816 = 0,
  EVEX_LEN_0F3819,
  EVEX_LEN_0F381A_M_0,
  EVEX_LEN_0F381B_M_0,
  EVEX_LEN_0F3836,
  EVEX_LEN_0F385A_M_0,
  EVEX_LEN_0F385B_M_0,
  EVEX_LEN_0F38C6_M_0,
  EVEX_LEN_0F38C7_M_0,
  EVEX_LEN_0F3A00,
  EVEX_LEN_0F3A01,
  EVEX_LEN_0F3A18,
  EVEX_LEN_0F3A19,
  EVEX_LEN_0F3A1A,
  EVEX_LEN_0F3A1B,
  EVEX_LEN_0F3A23,
  EVEX_LEN_0F3A38,
  EVEX_LEN_0F3A39,
  EVEX_LEN_0F3A3A,
  EVEX_LEN_0F3A3B,
  EVEX_LEN_0F3A43
};

enum
{
  VEX_W_0F41_L_1_M_1 = 0,
  VEX_W_0F42_L_1_M_1,
  VEX_W_0F44_L_0_M_1,
  VEX_W_0F45_L_1_M_1,
  VEX_W_0F46_L_1_M_1,
  VEX_W_0F47_L_1_M_1,
  VEX_W_0F4A_L_1_M_1,
  VEX_W_0F4B_L_1_M_1,
  VEX_W_0F90_L_0,
  VEX_W_0F91_L_0_M_0,
  VEX_W_0F92_L_0_M_1,
  VEX_W_0F93_L_0_M_1,
  VEX_W_0F98_L_0_M_1,
  VEX_W_0F99_L_0_M_1,
  VEX_W_0F380C,
  VEX_W_0F380D,
  VEX_W_0F380E,
  VEX_W_0F380F,
  VEX_W_0F3813,
  VEX_W_0F3816_L_1,
  VEX_W_0F3818,
  VEX_W_0F3819_L_1,
  VEX_W_0F381A_M_0_L_1,
  VEX_W_0F382C_M_0,
  VEX_W_0F382D_M_0,
  VEX_W_0F382E_M_0,
  VEX_W_0F382F_M_0,
  VEX_W_0F3836,
  VEX_W_0F3846,
  VEX_W_0F3849_X86_64_L_0,
  VEX_W_0F384B_X86_64_L_0,
  VEX_W_0F3850,
  VEX_W_0F3851,
  VEX_W_0F3852,
  VEX_W_0F3853,
  VEX_W_0F3858,
  VEX_W_0F3859,
  VEX_W_0F385A_M_0_L_0,
  VEX_W_0F385C_X86_64_M_1_L_0,
  VEX_W_0F385E_X86_64_M_1_L_0,
  VEX_W_0F386C_X86_64_M_1_L_0,
  VEX_W_0F3872_P_1,
  VEX_W_0F3878,
  VEX_W_0F3879,
  VEX_W_0F38B0,
  VEX_W_0F38B1,
  VEX_W_0F38B4,
  VEX_W_0F38B5,
  VEX_W_0F38CF,
  VEX_W_0F3A00_L_1,
  VEX_W_0F3A01_L_1,
  VEX_W_0F3A02,
  VEX_W_0F3A04,
  VEX_W_0F3A05,
  VEX_W_0F3A06_L_1,
  VEX_W_0F3A18_L_1,
  VEX_W_0F3A19_L_1,
  VEX_W_0F3A1D,
  VEX_W_0F3A38_L_1,
  VEX_W_0F3A39_L_1,
  VEX_W_0F3A46_L_1,
  VEX_W_0F3A4A,
  VEX_W_0F3A4B,
  VEX_W_0F3A4C,
  VEX_W_0F3ACE,
  VEX_W_0F3ACF,

  VEX_W_0FXOP_08_85_L_0,
  VEX_W_0FXOP_08_86_L_0,
  VEX_W_0FXOP_08_87_L_0,
  VEX_W_0FXOP_08_8E_L_0,
  VEX_W_0FXOP_08_8F_L_0,
  VEX_W_0FXOP_08_95_L_0,
  VEX_W_0FXOP_08_96_L_0,
  VEX_W_0FXOP_08_97_L_0,
  VEX_W_0FXOP_08_9E_L_0,
  VEX_W_0FXOP_08_9F_L_0,
  VEX_W_0FXOP_08_A6_L_0,
  VEX_W_0FXOP_08_B6_L_0,
  VEX_W_0FXOP_08_C0_L_0,
  VEX_W_0FXOP_08_C1_L_0,
  VEX_W_0FXOP_08_C2_L_0,
  VEX_W_0FXOP_08_C3_L_0,
  VEX_W_0FXOP_08_CC_L_0,
  VEX_W_0FXOP_08_CD_L_0,
  VEX_W_0FXOP_08_CE_L_0,
  VEX_W_0FXOP_08_CF_L_0,
  VEX_W_0FXOP_08_EC_L_0,
  VEX_W_0FXOP_08_ED_L_0,
  VEX_W_0FXOP_08_EE_L_0,
  VEX_W_0FXOP_08_EF_L_0,

  VEX_W_0FXOP_09_80,
  VEX_W_0FXOP_09_81,
  VEX_W_0FXOP_09_82,
  VEX_W_0FXOP_09_83,
  VEX_W_0FXOP_09_C1_L_0,
  VEX_W_0FXOP_09_C2_L_0,
  VEX_W_0FXOP_09_C3_L_0,
  VEX_W_0FXOP_09_C6_L_0,
  VEX_W_0FXOP_09_C7_L_0,
  VEX_W_0FXOP_09_CB_L_0,
  VEX_W_0FXOP_09_D1_L_0,
  VEX_W_0FXOP_09_D2_L_0,
  VEX_W_0FXOP_09_D3_L_0,
  VEX_W_0FXOP_09_D6_L_0,
  VEX_W_0FXOP_09_D7_L_0,
  VEX_W_0FXOP_09_DB_L_0,
  VEX_W_0FXOP_09_E1_L_0,
  VEX_W_0FXOP_09_E2_L_0,
  VEX_W_0FXOP_09_E3_L_0,

  EVEX_W_0F5B_P_0,
  EVEX_W_0F62,
  EVEX_W_0F66,
  EVEX_W_0F6A,
  EVEX_W_0F6B,
  EVEX_W_0F6C,
  EVEX_W_0F6D,
  EVEX_W_0F6F_P_1,
  EVEX_W_0F6F_P_2,
  EVEX_W_0F6F_P_3,
  EVEX_W_0F70_P_2,
  EVEX_W_0F72_R_2,
  EVEX_W_0F72_R_6,
  EVEX_W_0F73_R_2,
  EVEX_W_0F73_R_6,
  EVEX_W_0F76,
  EVEX_W_0F78_P_0,
  EVEX_W_0F78_P_2,
  EVEX_W_0F79_P_0,
  EVEX_W_0F79_P_2,
  EVEX_W_0F7A_P_1,
  EVEX_W_0F7A_P_2,
  EVEX_W_0F7A_P_3,
  EVEX_W_0F7B_P_2,
  EVEX_W_0F7E_P_1,
  EVEX_W_0F7F_P_1,
  EVEX_W_0F7F_P_2,
  EVEX_W_0F7F_P_3,
  EVEX_W_0FD2,
  EVEX_W_0FD3,
  EVEX_W_0FD4,
  EVEX_W_0FD6,
  EVEX_W_0FE6_P_1,
  EVEX_W_0FE7,
  EVEX_W_0FF2,
  EVEX_W_0FF3,
  EVEX_W_0FF4,
  EVEX_W_0FFA,
  EVEX_W_0FFB,
  EVEX_W_0FFE,

  EVEX_W_0F3810_P_1,
  EVEX_W_0F3810_P_2,
  EVEX_W_0F3811_P_1,
  EVEX_W_0F3811_P_2,
  EVEX_W_0F3812_P_1,
  EVEX_W_0F3812_P_2,
  EVEX_W_0F3813_P_1,
  EVEX_W_0F3814_P_1,
  EVEX_W_0F3815_P_1,
  EVEX_W_0F3819_L_n,
  EVEX_W_0F381A_M_0_L_n,
  EVEX_W_0F381B_M_0_L_2,
  EVEX_W_0F381E,
  EVEX_W_0F381F,
  EVEX_W_0F3820_P_1,
  EVEX_W_0F3821_P_1,
  EVEX_W_0F3822_P_1,
  EVEX_W_0F3823_P_1,
  EVEX_W_0F3824_P_1,
  EVEX_W_0F3825_P_1,
  EVEX_W_0F3825_P_2,
  EVEX_W_0F3828_P_2,
  EVEX_W_0F3829_P_2,
  EVEX_W_0F382A_P_1,
  EVEX_W_0F382A_P_2,
  EVEX_W_0F382B,
  EVEX_W_0F3830_P_1,
  EVEX_W_0F3831_P_1,
  EVEX_W_0F3832_P_1,
  EVEX_W_0F3833_P_1,
  EVEX_W_0F3834_P_1,
  EVEX_W_0F3835_P_1,
  EVEX_W_0F3835_P_2,
  EVEX_W_0F3837,
  EVEX_W_0F383A_P_1,
  EVEX_W_0F3859,
  EVEX_W_0F385A_M_0_L_n,
  EVEX_W_0F385B_M_0_L_2,
  EVEX_W_0F3870,
  EVEX_W_0F3872_P_2,
  EVEX_W_0F387A,
  EVEX_W_0F387B,
  EVEX_W_0F3883,

  EVEX_W_0F3A18_L_n,
  EVEX_W_0F3A19_L_n,
  EVEX_W_0F3A1A_L_2,
  EVEX_W_0F3A1B_L_2,
  EVEX_W_0F3A21,
  EVEX_W_0F3A23_L_n,
  EVEX_W_0F3A38_L_n,
  EVEX_W_0F3A39_L_n,
  EVEX_W_0F3A3A_L_2,
  EVEX_W_0F3A3B_L_2,
  EVEX_W_0F3A42,
  EVEX_W_0F3A43_L_n,
  EVEX_W_0F3A70,
  EVEX_W_0F3A72,

  EVEX_W_MAP5_5B_P_0,
  EVEX_W_MAP5_7A_P_3,
};

typedef bool (*op_rtn) (instr_info *ins, int bytemode, int sizeflag);

struct dis386 {
  const char *name;
  struct
    {
      op_rtn rtn;
      int bytemode;
    } op[MAX_OPERANDS];
  unsigned int prefix_requirement;
};

/* Upper case letters in the instruction names here are macros.
   'A' => print 'b' if no register operands or suffix_always is true
   'B' => print 'b' if suffix_always is true
   'C' => print 's' or 'l' ('w' or 'd' in Intel mode) depending on operand
	  size prefix
   'D' => print 'w' if no register operands or 'w', 'l' or 'q', if
	  suffix_always is true
   'E' => print 'e' if 32-bit form of jcxz
   'F' => print 'w' or 'l' depending on address size prefix (loop insns)
   'G' => print 'w' or 'l' depending on operand size prefix (i/o insns)
   'H' => print ",pt" or ",pn" branch hint
   'I' unused.
   'J' unused.
   'K' => print 'd' or 'q' if rex prefix is present.
   'L' unused.
   'M' => print 'r' if intel_mnemonic is false.
   'N' => print 'n' if instruction has no wait "prefix"
   'O' => print 'd' or 'o' (or 'q' in Intel mode)
   'P' => behave as 'T' except with register operand outside of suffix_always
	  mode
   'Q' => print 'w', 'l' or 'q' for memory operand or suffix_always
	  is true
   'R' => print 'w', 'l' or 'q' ('d' for 'l' and 'e' in Intel mode)
   'S' => print 'w', 'l' or 'q' if suffix_always is true
   'T' => print 'w', 'l'/'d', or 'q' if instruction has an operand size
	  prefix or if suffix_always is true.
   'U' unused.
   'V' unused.
   'W' => print 'b', 'w' or 'l' ('d' in Intel mode)
   'X' => print 's', 'd' depending on data16 prefix (for XMM)
   'Y' unused.
   'Z' => print 'q' in 64bit mode and 'l' otherwise, if suffix_always is true.
   '!' => change condition from true to false or from false to true.
   '%' => add 1 upper case letter to the macro.
   '^' => print 'w', 'l', or 'q' (Intel64 ISA only) depending on operand size
	  prefix or suffix_always is true (lcall/ljmp).
   '@' => in 64bit mode for Intel64 ISA or if instruction
	  has no operand sizing prefix, print 'q' if suffix_always is true or
	  nothing otherwise; behave as 'P' in all other cases

   2 upper case letter macros:
   "XY" => print 'x' or 'y' if suffix_always is true or no register
	   operands and no broadcast.
   "XZ" => print 'x', 'y', or 'z' if suffix_always is true or no
	   register operands and no broadcast.
   "XW" => print 's', 'd' depending on the VEX.W bit (for FMA)
   "XD" => print 'd' if !EVEX or EVEX.W=1, EVEX.W=0 is not a valid encoding
   "XH" => print 'h' if EVEX.W=0, EVEX.W=1 is not a valid encoding (for FP16)
   "XS" => print 's' if !EVEX or EVEX.W=0, EVEX.W=1 is not a valid encoding
   "XV" => print "{vex} " pseudo prefix
   "XE" => print "{evex} " pseudo prefix if no EVEX-specific functionality is
	   is used by an EVEX-encoded (AVX512VL) instruction.
   "LQ" => print 'l' ('d' in Intel mode) or 'q' for memory operand, cond
	   being false, or no operand at all in 64bit mode, or if suffix_always
	   is true.
   "LB" => print "abs" in 64bit mode and behave as 'B' otherwise
   "LS" => print "abs" in 64bit mode and behave as 'S' otherwise
   "LV" => print "abs" for 64bit operand and behave as 'S' otherwise
   "DQ" => print 'd' or 'q' depending on the VEX.W bit
   "BW" => print 'b' or 'w' depending on the VEX.W bit
   "LP" => print 'w' or 'l' ('d' in Intel mode) if instruction has
	   an operand size prefix, or suffix_always is true.  print
	   'q' if rex prefix is present.

   Many of the above letters print nothing in Intel mode.  See "putop"
   for the details.

   Braces '{' and '}', and vertical bars '|', indicate alternative
   mnemonic strings for AT&T and Intel.  */

static const struct dis386 dis386[] = {
  /* 00 */
  { "addB",		{ Ebh1, Gb }, 0 },
  { "addS",		{ Evh1, Gv }, 0 },
  { "addB",		{ Gb, EbS }, 0 },
  { "addS",		{ Gv, EvS }, 0 },
  { "addB",		{ AL, Ib }, 0 },
  { "addS",		{ eAX, Iv }, 0 },
  { X86_64_TABLE (X86_64_06) },
  { X86_64_TABLE (X86_64_07) },
  /* 08 */
  { "orB",		{ Ebh1, Gb }, 0 },
  { "orS",		{ Evh1, Gv }, 0 },
  { "orB",		{ Gb, EbS }, 0 },
  { "orS",		{ Gv, EvS }, 0 },
  { "orB",		{ AL, Ib }, 0 },
  { "orS",		{ eAX, Iv }, 0 },
  { X86_64_TABLE (X86_64_0E) },
  { Bad_Opcode },	/* 0x0f extended opcode escape */
  /* 10 */
  { "adcB",		{ Ebh1, Gb }, 0 },
  { "adcS",		{ Evh1, Gv }, 0 },
  { "adcB",		{ Gb, EbS }, 0 },
  { "adcS",		{ Gv, EvS }, 0 },
  { "adcB",		{ AL, Ib }, 0 },
  { "adcS",		{ eAX, Iv }, 0 },
  { X86_64_TABLE (X86_64_16) },
  { X86_64_TABLE (X86_64_17) },
  /* 18 */
  { "sbbB",		{ Ebh1, Gb }, 0 },
  { "sbbS",		{ Evh1, Gv }, 0 },
  { "sbbB",		{ Gb, EbS }, 0 },
  { "sbbS",		{ Gv, EvS }, 0 },
  { "sbbB",		{ AL, Ib }, 0 },
  { "sbbS",		{ eAX, Iv }, 0 },
  { X86_64_TABLE (X86_64_1E) },
  { X86_64_TABLE (X86_64_1F) },
  /* 20 */
  { "andB",		{ Ebh1, Gb }, 0 },
  { "andS",		{ Evh1, Gv }, 0 },
  { "andB",		{ Gb, EbS }, 0 },
  { "andS",		{ Gv, EvS }, 0 },
  { "andB",		{ AL, Ib }, 0 },
  { "andS",		{ eAX, Iv }, 0 },
  { Bad_Opcode },	/* SEG ES prefix */
  { X86_64_TABLE (X86_64_27) },
  /* 28 */
  { "subB",		{ Ebh1, Gb }, 0 },
  { "subS",		{ Evh1, Gv }, 0 },
  { "subB",		{ Gb, EbS }, 0 },
  { "subS",		{ Gv, EvS }, 0 },
  { "subB",		{ AL, Ib }, 0 },
  { "subS",		{ eAX, Iv }, 0 },
  { Bad_Opcode },	/* SEG CS prefix */
  { X86_64_TABLE (X86_64_2F) },
  /* 30 */
  { "xorB",		{ Ebh1, Gb }, 0 },
  { "xorS",		{ Evh1, Gv }, 0 },
  { "xorB",		{ Gb, EbS }, 0 },
  { "xorS",		{ Gv, EvS }, 0 },
  { "xorB",		{ AL, Ib }, 0 },
  { "xorS",		{ eAX, Iv }, 0 },
  { Bad_Opcode },	/* SEG SS prefix */
  { X86_64_TABLE (X86_64_37) },
  /* 38 */
  { "cmpB",		{ Eb, Gb }, 0 },
  { "cmpS",		{ Ev, Gv }, 0 },
  { "cmpB",		{ Gb, EbS }, 0 },
  { "cmpS",		{ Gv, EvS }, 0 },
  { "cmpB",		{ AL, Ib }, 0 },
  { "cmpS",		{ eAX, Iv }, 0 },
  { Bad_Opcode },	/* SEG DS prefix */
  { X86_64_TABLE (X86_64_3F) },
  /* 40 */
  { "inc{S|}",		{ RMeAX }, 0 },
  { "inc{S|}",		{ RMeCX }, 0 },
  { "inc{S|}",		{ RMeDX }, 0 },
  { "inc{S|}",		{ RMeBX }, 0 },
  { "inc{S|}",		{ RMeSP }, 0 },
  { "inc{S|}",		{ RMeBP }, 0 },
  { "inc{S|}",		{ RMeSI }, 0 },
  { "inc{S|}",		{ RMeDI }, 0 },
  /* 48 */
  { "dec{S|}",		{ RMeAX }, 0 },
  { "dec{S|}",		{ RMeCX }, 0 },
  { "dec{S|}",		{ RMeDX }, 0 },
  { "dec{S|}",		{ RMeBX }, 0 },
  { "dec{S|}",		{ RMeSP }, 0 },
  { "dec{S|}",		{ RMeBP }, 0 },
  { "dec{S|}",		{ RMeSI }, 0 },
  { "dec{S|}",		{ RMeDI }, 0 },
  /* 50 */
  { "push{!P|}",		{ RMrAX }, 0 },
  { "push{!P|}",		{ RMrCX }, 0 },
  { "push{!P|}",		{ RMrDX }, 0 },
  { "push{!P|}",		{ RMrBX }, 0 },
  { "push{!P|}",		{ RMrSP }, 0 },
  { "push{!P|}",		{ RMrBP }, 0 },
  { "push{!P|}",		{ RMrSI }, 0 },
  { "push{!P|}",		{ RMrDI }, 0 },
  /* 58 */
  { "pop{!P|}",		{ RMrAX }, 0 },
  { "pop{!P|}",		{ RMrCX }, 0 },
  { "pop{!P|}",		{ RMrDX }, 0 },
  { "pop{!P|}",		{ RMrBX }, 0 },
  { "pop{!P|}",		{ RMrSP }, 0 },
  { "pop{!P|}",		{ RMrBP }, 0 },
  { "pop{!P|}",		{ RMrSI }, 0 },
  { "pop{!P|}",		{ RMrDI }, 0 },
  /* 60 */
  { X86_64_TABLE (X86_64_60) },
  { X86_64_TABLE (X86_64_61) },
  { X86_64_TABLE (X86_64_62) },
  { X86_64_TABLE (X86_64_63) },
  { Bad_Opcode },	/* seg fs */
  { Bad_Opcode },	/* seg gs */
  { Bad_Opcode },	/* op size prefix */
  { Bad_Opcode },	/* adr size prefix */
  /* 68 */
  { "pushP",		{ sIv }, 0 },
  { "imulS",		{ Gv, Ev, Iv }, 0 },
  { "pushP",		{ sIbT }, 0 },
  { "imulS",		{ Gv, Ev, sIb }, 0 },
  { "ins{b|}",		{ Ybr, indirDX }, 0 },
  { X86_64_TABLE (X86_64_6D) },
  { "outs{b|}",		{ indirDXr, Xb }, 0 },
  { X86_64_TABLE (X86_64_6F) },
  /* 70 */
  { "joH",		{ Jb, BND, cond_jump_flag }, 0 },
  { "jnoH",		{ Jb, BND, cond_jump_flag }, 0 },
  { "jbH",		{ Jb, BND, cond_jump_flag }, 0 },
  { "jaeH",		{ Jb, BND, cond_jump_flag }, 0 },
  { "jeH",		{ Jb, BND, cond_jump_flag }, 0 },
  { "jneH",		{ Jb, BND, cond_jump_flag }, 0 },
  { "jbeH",		{ Jb, BND, cond_jump_flag }, 0 },
  { "jaH",		{ Jb, BND, cond_jump_flag }, 0 },
  /* 78 */
  { "jsH",		{ Jb, BND, cond_jump_flag }, 0 },
  { "jnsH",		{ Jb, BND, cond_jump_flag }, 0 },
  { "jpH",		{ Jb, BND, cond_jump_flag }, 0 },
  { "jnpH",		{ Jb, BND, cond_jump_flag }, 0 },
  { "jlH",		{ Jb, BND, cond_jump_flag }, 0 },
  { "jgeH",		{ Jb, BND, cond_jump_flag }, 0 },
  { "jleH",		{ Jb, BND, cond_jump_flag }, 0 },
  { "jgH",		{ Jb, BND, cond_jump_flag }, 0 },
  /* 80 */
  { REG_TABLE (REG_80) },
  { REG_TABLE (REG_81) },
  { X86_64_TABLE (X86_64_82) },
  { REG_TABLE (REG_83) },
  { "testB",		{ Eb, Gb }, 0 },
  { "testS",		{ Ev, Gv }, 0 },
  { "xchgB",		{ Ebh2, Gb }, 0 },
  { "xchgS",		{ Evh2, Gv }, 0 },
  /* 88 */
  { "movB",		{ Ebh3, Gb }, 0 },
  { "movS",		{ Evh3, Gv }, 0 },
  { "movB",		{ Gb, EbS }, 0 },
  { "movS",		{ Gv, EvS }, 0 },
  { "movD",		{ Sv, Sw }, 0 },
  { MOD_TABLE (MOD_8D) },
  { "movD",		{ Sw, Sv }, 0 },
  { REG_TABLE (REG_8F) },
  /* 90 */
  { PREFIX_TABLE (PREFIX_90) },
  { "xchgS",		{ RMeCX, eAX }, 0 },
  { "xchgS",		{ RMeDX, eAX }, 0 },
  { "xchgS",		{ RMeBX, eAX }, 0 },
  { "xchgS",		{ RMeSP, eAX }, 0 },
  { "xchgS",		{ RMeBP, eAX }, 0 },
  { "xchgS",		{ RMeSI, eAX }, 0 },
  { "xchgS",		{ RMeDI, eAX }, 0 },
  /* 98 */
  { "cW{t|}R",		{ XX }, 0 },
  { "cR{t|}O",		{ XX }, 0 },
  { X86_64_TABLE (X86_64_9A) },
  { Bad_Opcode },	/* fwait */
  { "pushfP",		{ XX }, 0 },
  { "popfP",		{ XX }, 0 },
  { "sahf",		{ XX }, 0 },
  { "lahf",		{ XX }, 0 },
  /* a0 */
  { "mov%LB",		{ AL, Ob }, 0 },
  { "mov%LS",		{ eAX, Ov }, 0 },
  { "mov%LB",		{ Ob, AL }, 0 },
  { "mov%LS",		{ Ov, eAX }, 0 },
  { "movs{b|}",		{ Ybr, Xb }, 0 },
  { "movs{R|}",		{ Yvr, Xv }, 0 },
  { "cmps{b|}",		{ Xb, Yb }, 0 },
  { "cmps{R|}",		{ Xv, Yv }, 0 },
  /* a8 */
  { "testB",		{ AL, Ib }, 0 },
  { "testS",		{ eAX, Iv }, 0 },
  { "stosB",		{ Ybr, AL }, 0 },
  { "stosS",		{ Yvr, eAX }, 0 },
  { "lodsB",		{ ALr, Xb }, 0 },
  { "lodsS",		{ eAXr, Xv }, 0 },
  { "scasB",		{ AL, Yb }, 0 },
  { "scasS",		{ eAX, Yv }, 0 },
  /* b0 */
  { "movB",		{ RMAL, Ib }, 0 },
  { "movB",		{ RMCL, Ib }, 0 },
  { "movB",		{ RMDL, Ib }, 0 },
  { "movB",		{ RMBL, Ib }, 0 },
  { "movB",		{ RMAH, Ib }, 0 },
  { "movB",		{ RMCH, Ib }, 0 },
  { "movB",		{ RMDH, Ib }, 0 },
  { "movB",		{ RMBH, Ib }, 0 },
  /* b8 */
  { "mov%LV",		{ RMeAX, Iv64 }, 0 },
  { "mov%LV",		{ RMeCX, Iv64 }, 0 },
  { "mov%LV",		{ RMeDX, Iv64 }, 0 },
  { "mov%LV",		{ RMeBX, Iv64 }, 0 },
  { "mov%LV",		{ RMeSP, Iv64 }, 0 },
  { "mov%LV",		{ RMeBP, Iv64 }, 0 },
  { "mov%LV",		{ RMeSI, Iv64 }, 0 },
  { "mov%LV",		{ RMeDI, Iv64 }, 0 },
  /* c0 */
  { REG_TABLE (REG_C0) },
  { REG_TABLE (REG_C1) },
  { X86_64_TABLE (X86_64_C2) },
  { X86_64_TABLE (X86_64_C3) },
  { X86_64_TABLE (X86_64_C4) },
  { X86_64_TABLE (X86_64_C5) },
  { REG_TABLE (REG_C6) },
  { REG_TABLE (REG_C7) },
  /* c8 */
  { "enterP",		{ Iw, Ib }, 0 },
  { "leaveP",		{ XX }, 0 },
  { "{l|}ret{|f}%LP",	{ Iw }, 0 },
  { "{l|}ret{|f}%LP",	{ XX }, 0 },
  { "int3",		{ XX }, 0 },
  { "int",		{ Ib }, 0 },
  { X86_64_TABLE (X86_64_CE) },
  { "iret%LP",		{ XX }, 0 },
  /* d0 */
  { REG_TABLE (REG_D0) },
  { REG_TABLE (REG_D1) },
  { REG_TABLE (REG_D2) },
  { REG_TABLE (REG_D3) },
  { X86_64_TABLE (X86_64_D4) },
  { X86_64_TABLE (X86_64_D5) },
  { Bad_Opcode },
  { "xlat",		{ DSBX }, 0 },
  /* d8 */
  { FLOAT },
  { FLOAT },
  { FLOAT },
  { FLOAT },
  { FLOAT },
  { FLOAT },
  { FLOAT },
  { FLOAT },
  /* e0 */
  { "loopneFH",		{ Jb, XX, loop_jcxz_flag }, 0 },
  { "loopeFH",		{ Jb, XX, loop_jcxz_flag }, 0 },
  { "loopFH",		{ Jb, XX, loop_jcxz_flag }, 0 },
  { "jEcxzH",		{ Jb, XX, loop_jcxz_flag }, 0 },
  { "inB",		{ AL, Ib }, 0 },
  { "inG",		{ zAX, Ib }, 0 },
  { "outB",		{ Ib, AL }, 0 },
  { "outG",		{ Ib, zAX }, 0 },
  /* e8 */
  { X86_64_TABLE (X86_64_E8) },
  { X86_64_TABLE (X86_64_E9) },
  { X86_64_TABLE (X86_64_EA) },
  { "jmp",		{ Jb, BND }, 0 },
  { "inB",		{ AL, indirDX }, 0 },
  { "inG",		{ zAX, indirDX }, 0 },
  { "outB",		{ indirDX, AL }, 0 },
  { "outG",		{ indirDX, zAX }, 0 },
  /* f0 */
  { Bad_Opcode },	/* lock prefix */
  { "int1",		{ XX }, 0 },
  { Bad_Opcode },	/* repne */
  { Bad_Opcode },	/* repz */
  { "hlt",		{ XX }, 0 },
  { "cmc",		{ XX }, 0 },
  { REG_TABLE (REG_F6) },
  { REG_TABLE (REG_F7) },
  /* f8 */
  { "clc",		{ XX }, 0 },
  { "stc",		{ XX }, 0 },
  { "cli",		{ XX }, 0 },
  { "sti",		{ XX }, 0 },
  { "cld",		{ XX }, 0 },
  { "std",		{ XX }, 0 },
  { REG_TABLE (REG_FE) },
  { REG_TABLE (REG_FF) },
};

static const struct dis386 dis386_twobyte[] = {
  /* 00 */
  { REG_TABLE (REG_0F00 ) },
  { REG_TABLE (REG_0F01 ) },
  { MOD_TABLE (MOD_0F02) },
  { MOD_TABLE (MOD_0F03) },
  { Bad_Opcode },
  { "syscall",		{ XX }, 0 },
  { "clts",		{ XX }, 0 },
  { "sysret%LQ",		{ XX }, 0 },
  /* 08 */
  { "invd",		{ XX }, 0 },
  { PREFIX_TABLE (PREFIX_0F09) },
  { Bad_Opcode },
  { "ud2",		{ XX }, 0 },
  { Bad_Opcode },
  { REG_TABLE (REG_0F0D) },
  { "femms",		{ XX }, 0 },
  { "",			{ MX, EM, OPSUF }, 0 }, /* See OP_3DNowSuffix.  */
  /* 10 */
  { PREFIX_TABLE (PREFIX_0F10) },
  { PREFIX_TABLE (PREFIX_0F11) },
  { PREFIX_TABLE (PREFIX_0F12) },
  { MOD_TABLE (MOD_0F13) },
  { "unpcklpX",		{ XM, EXx }, PREFIX_OPCODE },
  { "unpckhpX",		{ XM, EXx }, PREFIX_OPCODE },
  { PREFIX_TABLE (PREFIX_0F16) },
  { MOD_TABLE (MOD_0F17) },
  /* 18 */
  { REG_TABLE (REG_0F18) },
  { "nopQ",		{ Ev }, 0 },
  { PREFIX_TABLE (PREFIX_0F1A) },
  { PREFIX_TABLE (PREFIX_0F1B) },
  { PREFIX_TABLE (PREFIX_0F1C) },
  { "nopQ",		{ Ev }, 0 },
  { PREFIX_TABLE (PREFIX_0F1E) },
  { "nopQ",		{ Ev }, 0 },
  /* 20 */
  { "movZ",		{ Em, Cm }, 0 },
  { "movZ",		{ Em, Dm }, 0 },
  { "movZ",		{ Cm, Em }, 0 },
  { "movZ",		{ Dm, Em }, 0 },
  { X86_64_TABLE (X86_64_0F24) },
  { Bad_Opcode },
  { X86_64_TABLE (X86_64_0F26) },
  { Bad_Opcode },
  /* 28 */
  { "movapX",		{ XM, EXx }, PREFIX_OPCODE },
  { "movapX",		{ EXxS, XM }, PREFIX_OPCODE },
  { PREFIX_TABLE (PREFIX_0F2A) },
  { PREFIX_TABLE (PREFIX_0F2B) },
  { PREFIX_TABLE (PREFIX_0F2C) },
  { PREFIX_TABLE (PREFIX_0F2D) },
  { PREFIX_TABLE (PREFIX_0F2E) },
  { PREFIX_TABLE (PREFIX_0F2F) },
  /* 30 */
  { "wrmsr",		{ XX }, 0 },
  { "rdtsc",		{ XX }, 0 },
  { "rdmsr",		{ XX }, 0 },
  { "rdpmc",		{ XX }, 0 },
  { "sysenter",		{ SEP }, 0 },
  { "sysexit%LQ",	{ SEP }, 0 },
  { Bad_Opcode },
  { "getsec",		{ XX }, 0 },
  /* 38 */
  { THREE_BYTE_TABLE_PREFIX (THREE_BYTE_0F38, PREFIX_OPCODE) },
  { Bad_Opcode },
  { THREE_BYTE_TABLE_PREFIX (THREE_BYTE_0F3A, PREFIX_OPCODE) },
  { Bad_Opcode },
  { Bad_Opcode },
  { Bad_Opcode },
  { Bad_Opcode },
  { Bad_Opcode },
  /* 40 */
  { "cmovoS",		{ Gv, Ev }, 0 },
  { "cmovnoS",		{ Gv, Ev }, 0 },
  { "cmovbS",		{ Gv, Ev }, 0 },
  { "cmovaeS",		{ Gv, Ev }, 0 },
  { "cmoveS",		{ Gv, Ev }, 0 },
  { "cmovneS",		{ Gv, Ev }, 0 },
  { "cmovbeS",		{ Gv, Ev }, 0 },
  { "cmovaS",		{ Gv, Ev }, 0 },
  /* 48 */
  { "cmovsS",		{ Gv, Ev }, 0 },
  { "cmovnsS",		{ Gv, Ev }, 0 },
  { "cmovpS",		{ Gv, Ev }, 0 },
  { "cmovnpS",		{ Gv, Ev }, 0 },
  { "cmovlS",		{ Gv, Ev }, 0 },
  { "cmovgeS",		{ Gv, Ev }, 0 },
  { "cmovleS",		{ Gv, Ev }, 0 },
  { "cmovgS",		{ Gv, Ev }, 0 },
  /* 50 */
  { MOD_TABLE (MOD_0F50) },
  { PREFIX_TABLE (PREFIX_0F51) },
  { PREFIX_TABLE (PREFIX_0F52) },
  { PREFIX_TABLE (PREFIX_0F53) },
  { "andpX",		{ XM, EXx }, PREFIX_OPCODE },
  { "andnpX",		{ XM, EXx }, PREFIX_OPCODE },
  { "orpX",		{ XM, EXx }, PREFIX_OPCODE },
  { "xorpX",		{ XM, EXx }, PREFIX_OPCODE },
  /* 58 */
  { PREFIX_TABLE (PREFIX_0F58) },
  { PREFIX_TABLE (PREFIX_0F59) },
  { PREFIX_TABLE (PREFIX_0F5A) },
  { PREFIX_TABLE (PREFIX_0F5B) },
  { PREFIX_TABLE (PREFIX_0F5C) },
  { PREFIX_TABLE (PREFIX_0F5D) },
  { PREFIX_TABLE (PREFIX_0F5E) },
  { PREFIX_TABLE (PREFIX_0F5F) },
  /* 60 */
  { PREFIX_TABLE (PREFIX_0F60) },
  { PREFIX_TABLE (PREFIX_0F61) },
  { PREFIX_TABLE (PREFIX_0F62) },
  { "packsswb",		{ MX, EM }, PREFIX_OPCODE },
  { "pcmpgtb",		{ MX, EM }, PREFIX_OPCODE },
  { "pcmpgtw",		{ MX, EM }, PREFIX_OPCODE },
  { "pcmpgtd",		{ MX, EM }, PREFIX_OPCODE },
  { "packuswb",		{ MX, EM }, PREFIX_OPCODE },
  /* 68 */
  { "punpckhbw",	{ MX, EM }, PREFIX_OPCODE },
  { "punpckhwd",	{ MX, EM }, PREFIX_OPCODE },
  { "punpckhdq",	{ MX, EM }, PREFIX_OPCODE },
  { "packssdw",		{ MX, EM }, PREFIX_OPCODE },
  { "punpcklqdq", { XM, EXx }, PREFIX_DATA },
  { "punpckhqdq", { XM, EXx }, PREFIX_DATA },
  { "movK",		{ MX, Edq }, PREFIX_OPCODE },
  { PREFIX_TABLE (PREFIX_0F6F) },
  /* 70 */
  { PREFIX_TABLE (PREFIX_0F70) },
  { MOD_TABLE (MOD_0F71) },
  { MOD_TABLE (MOD_0F72) },
  { MOD_TABLE (MOD_0F73) },
  { "pcmpeqb",		{ MX, EM }, PREFIX_OPCODE },
  { "pcmpeqw",		{ MX, EM }, PREFIX_OPCODE },
  { "pcmpeqd",		{ MX, EM }, PREFIX_OPCODE },
  { "emms",		{ XX }, PREFIX_OPCODE },
  /* 78 */
  { PREFIX_TABLE (PREFIX_0F78) },
  { PREFIX_TABLE (PREFIX_0F79) },
  { Bad_Opcode },
  { Bad_Opcode },
  { PREFIX_TABLE (PREFIX_0F7C) },
  { PREFIX_TABLE (PREFIX_0F7D) },
  { PREFIX_TABLE (PREFIX_0F7E) },
  { PREFIX_TABLE (PREFIX_0F7F) },
  /* 80 */
  { "joH",		{ Jv, BND, cond_jump_flag }, 0 },
  { "jnoH",		{ Jv, BND, cond_jump_flag }, 0 },
  { "jbH",		{ Jv, BND, cond_jump_flag }, 0 },
  { "jaeH",		{ Jv, BND, cond_jump_flag }, 0 },
  { "jeH",		{ Jv, BND, cond_jump_flag }, 0 },
  { "jneH",		{ Jv, BND, cond_jump_flag }, 0 },
  { "jbeH",		{ Jv, BND, cond_jump_flag }, 0 },
  { "jaH",		{ Jv, BND, cond_jump_flag }, 0 },
  /* 88 */
  { "jsH",		{ Jv, BND, cond_jump_flag }, 0 },
  { "jnsH",		{ Jv, BND, cond_jump_flag }, 0 },
  { "jpH",		{ Jv, BND, cond_jump_flag }, 0 },
  { "jnpH",		{ Jv, BND, cond_jump_flag }, 0 },
  { "jlH",		{ Jv, BND, cond_jump_flag }, 0 },
  { "jgeH",		{ Jv, BND, cond_jump_flag }, 0 },
  { "jleH",		{ Jv, BND, cond_jump_flag }, 0 },
  { "jgH",		{ Jv, BND, cond_jump_flag }, 0 },
  /* 90 */
  { "seto",		{ Eb }, 0 },
  { "setno",		{ Eb }, 0 },
  { "setb",		{ Eb }, 0 },
  { "setae",		{ Eb }, 0 },
  { "sete",		{ Eb }, 0 },
  { "setne",		{ Eb }, 0 },
  { "setbe",		{ Eb }, 0 },
  { "seta",		{ Eb }, 0 },
  /* 98 */
  { "sets",		{ Eb }, 0 },
  { "setns",		{ Eb }, 0 },
  { "setp",		{ Eb }, 0 },
  { "setnp",		{ Eb }, 0 },
  { "setl",		{ Eb }, 0 },
  { "setge",		{ Eb }, 0 },
  { "setle",		{ Eb }, 0 },
  { "setg",		{ Eb }, 0 },
  /* a0 */
  { "pushP",		{ fs }, 0 },
  { "popP",		{ fs }, 0 },
  { "cpuid",		{ XX }, 0 },
  { "btS",		{ Ev, Gv }, 0 },
  { "shldS",		{ Ev, Gv, Ib }, 0 },
  { "shldS",		{ Ev, Gv, CL }, 0 },
  { REG_TABLE (REG_0FA6) },
  { REG_TABLE (REG_0FA7) },
  /* a8 */
  { "pushP",		{ gs }, 0 },
  { "popP",		{ gs }, 0 },
  { "rsm",		{ XX }, 0 },
  { "btsS",		{ Evh1, Gv }, 0 },
  { "shrdS",		{ Ev, Gv, Ib }, 0 },
  { "shrdS",		{ Ev, Gv, CL }, 0 },
  { REG_TABLE (REG_0FAE) },
  { "imulS",		{ Gv, Ev }, 0 },
  /* b0 */
  { "cmpxchgB",		{ Ebh1, Gb }, 0 },
  { "cmpxchgS",		{ Evh1, Gv }, 0 },
  { MOD_TABLE (MOD_0FB2) },
  { "btrS",		{ Evh1, Gv }, 0 },
  { MOD_TABLE (MOD_0FB4) },
  { MOD_TABLE (MOD_0FB5) },
  { "movz{bR|x}",	{ Gv, Eb }, 0 },
  { "movz{wR|x}",	{ Gv, Ew }, 0 }, /* yes, there really is movzww ! */
  /* b8 */
  { PREFIX_TABLE (PREFIX_0FB8) },
  { "ud1S",		{ Gv, Ev }, 0 },
  { REG_TABLE (REG_0FBA) },
  { "btcS",		{ Evh1, Gv }, 0 },
  { PREFIX_TABLE (PREFIX_0FBC) },
  { PREFIX_TABLE (PREFIX_0FBD) },
  { "movs{bR|x}",	{ Gv, Eb }, 0 },
  { "movs{wR|x}",	{ Gv, Ew }, 0 }, /* yes, there really is movsww ! */
  /* c0 */
  { "xaddB",		{ Ebh1, Gb }, 0 },
  { "xaddS",		{ Evh1, Gv }, 0 },
  { PREFIX_TABLE (PREFIX_0FC2) },
  { MOD_TABLE (MOD_0FC3) },
  { "pinsrw",		{ MX, Edw, Ib }, PREFIX_OPCODE },
  { "pextrw",		{ Gd, MS, Ib }, PREFIX_OPCODE },
  { "shufpX",		{ XM, EXx, Ib }, PREFIX_OPCODE },
  { REG_TABLE (REG_0FC7) },
  /* c8 */
  { "bswap",		{ RMeAX }, 0 },
  { "bswap",		{ RMeCX }, 0 },
  { "bswap",		{ RMeDX }, 0 },
  { "bswap",		{ RMeBX }, 0 },
  { "bswap",		{ RMeSP }, 0 },
  { "bswap",		{ RMeBP }, 0 },
  { "bswap",		{ RMeSI }, 0 },
  { "bswap",		{ RMeDI }, 0 },
  /* d0 */
  { PREFIX_TABLE (PREFIX_0FD0) },
  { "psrlw",		{ MX, EM }, PREFIX_OPCODE },
  { "psrld",		{ MX, EM }, PREFIX_OPCODE },
  { "psrlq",		{ MX, EM }, PREFIX_OPCODE },
  { "paddq",		{ MX, EM }, PREFIX_OPCODE },
  { "pmullw",		{ MX, EM }, PREFIX_OPCODE },
  { PREFIX_TABLE (PREFIX_0FD6) },
  { MOD_TABLE (MOD_0FD7) },
  /* d8 */
  { "psubusb",		{ MX, EM }, PREFIX_OPCODE },
  { "psubusw",		{ MX, EM }, PREFIX_OPCODE },
  { "pminub",		{ MX, EM }, PREFIX_OPCODE },
  { "pand",		{ MX, EM }, PREFIX_OPCODE },
  { "paddusb",		{ MX, EM }, PREFIX_OPCODE },
  { "paddusw",		{ MX, EM }, PREFIX_OPCODE },
  { "pmaxub",		{ MX, EM }, PREFIX_OPCODE },
  { "pandn",		{ MX, EM }, PREFIX_OPCODE },
  /* e0 */
  { "pavgb",		{ MX, EM }, PREFIX_OPCODE },
  { "psraw",		{ MX, EM }, PREFIX_OPCODE },
  { "psrad",		{ MX, EM }, PREFIX_OPCODE },
  { "pavgw",		{ MX, EM }, PREFIX_OPCODE },
  { "pmulhuw",		{ MX, EM }, PREFIX_OPCODE },
  { "pmulhw",		{ MX, EM }, PREFIX_OPCODE },
  { PREFIX_TABLE (PREFIX_0FE6) },
  { PREFIX_TABLE (PREFIX_0FE7) },
  /* e8 */
  { "psubsb",		{ MX, EM }, PREFIX_OPCODE },
  { "psubsw",		{ MX, EM }, PREFIX_OPCODE },
  { "pminsw",		{ MX, EM }, PREFIX_OPCODE },
  { "por",		{ MX, EM }, PREFIX_OPCODE },
  { "paddsb",		{ MX, EM }, PREFIX_OPCODE },
  { "paddsw",		{ MX, EM }, PREFIX_OPCODE },
  { "pmaxsw",		{ MX, EM }, PREFIX_OPCODE },
  { "pxor",		{ MX, EM }, PREFIX_OPCODE },
  /* f0 */
  { PREFIX_TABLE (PREFIX_0FF0) },
  { "psllw",		{ MX, EM }, PREFIX_OPCODE },
  { "pslld",		{ MX, EM }, PREFIX_OPCODE },
  { "psllq",		{ MX, EM }, PREFIX_OPCODE },
  { "pmuludq",		{ MX, EM }, PREFIX_OPCODE },
  { "pmaddwd",		{ MX, EM }, PREFIX_OPCODE },
  { "psadbw",		{ MX, EM }, PREFIX_OPCODE },
  { PREFIX_TABLE (PREFIX_0FF7) },
  /* f8 */
  { "psubb",		{ MX, EM }, PREFIX_OPCODE },
  { "psubw",		{ MX, EM }, PREFIX_OPCODE },
  { "psubd",		{ MX, EM }, PREFIX_OPCODE },
  { "psubq",		{ MX, EM }, PREFIX_OPCODE },
  { "paddb",		{ MX, EM }, PREFIX_OPCODE },
  { "paddw",		{ MX, EM }, PREFIX_OPCODE },
  { "paddd",		{ MX, EM }, PREFIX_OPCODE },
  { "ud0S",		{ Gv, Ev }, 0 },
};

static const bool onebyte_has_modrm[256] = {
  /*       0 1 2 3 4 5 6 7 8 9 a b c d e f        */
  /*       -------------------------------        */
  /* 00 */ 1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0, /* 00 */
  /* 10 */ 1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0, /* 10 */
  /* 20 */ 1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0, /* 20 */
  /* 30 */ 1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0, /* 30 */
  /* 40 */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 40 */
  /* 50 */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 50 */
  /* 60 */ 0,0,1,1,0,0,0,0,0,1,0,1,0,0,0,0, /* 60 */
  /* 70 */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 70 */
  /* 80 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 80 */
  /* 90 */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 90 */
  /* a0 */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* a0 */
  /* b0 */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* b0 */
  /* c0 */ 1,1,0,0,1,1,1,1,0,0,0,0,0,0,0,0, /* c0 */
  /* d0 */ 1,1,1,1,0,0,0,0,1,1,1,1,1,1,1,1, /* d0 */
  /* e0 */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* e0 */
  /* f0 */ 0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1  /* f0 */
  /*       -------------------------------        */
  /*       0 1 2 3 4 5 6 7 8 9 a b c d e f        */
};

static const bool twobyte_has_modrm[256] = {
  /*       0 1 2 3 4 5 6 7 8 9 a b c d e f        */
  /*       -------------------------------        */
  /* 00 */ 1,1,1,1,0,0,0,0,0,0,0,0,0,1,0,1, /* 0f */
  /* 10 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 1f */
  /* 20 */ 1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1, /* 2f */
  /* 30 */ 0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0, /* 3f */
  /* 40 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 4f */
  /* 50 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 5f */
  /* 60 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 6f */
  /* 70 */ 1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1, /* 7f */
  /* 80 */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 8f */
  /* 90 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 9f */
  /* a0 */ 0,0,0,1,1,1,1,1,0,0,0,1,1,1,1,1, /* af */
  /* b0 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* bf */
  /* c0 */ 1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0, /* cf */
  /* d0 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* df */
  /* e0 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* ef */
  /* f0 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1  /* ff */
  /*       -------------------------------        */
  /*       0 1 2 3 4 5 6 7 8 9 a b c d e f        */
};


struct op
  {
    const char *name;
    unsigned int len;
  };

/* If we are accessing mod/rm/reg without need_modrm set, then the
   values are stale.  Hitting this abort likely indicates that you
   need to update onebyte_has_modrm or twobyte_has_modrm.  */
#define MODRM_CHECK  if (!ins->need_modrm) abort ()

static const char intel_index16[][6] = {
  "bx+si", "bx+di", "bp+si", "bp+di", "si", "di", "bp", "bx"
};

static const char att_names64[][8] = {
  "%rax", "%rcx", "%rdx", "%rbx", "%rsp", "%rbp", "%rsi", "%rdi",
  "%r8", "%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15"
};
static const char att_names32[][8] = {
  "%eax", "%ecx", "%edx", "%ebx", "%esp", "%ebp", "%esi", "%edi",
  "%r8d", "%r9d", "%r10d", "%r11d", "%r12d", "%r13d", "%r14d", "%r15d"
};
static const char att_names16[][8] = {
  "%ax", "%cx", "%dx", "%bx", "%sp", "%bp", "%si", "%di",
  "%r8w", "%r9w", "%r10w", "%r11w", "%r12w", "%r13w", "%r14w", "%r15w"
};
static const char att_names8[][8] = {
  "%al", "%cl", "%dl", "%bl", "%ah", "%ch", "%dh", "%bh",
};
static const char att_names8rex[][8] = {
  "%al", "%cl", "%dl", "%bl", "%spl", "%bpl", "%sil", "%dil",
  "%r8b", "%r9b", "%r10b", "%r11b", "%r12b", "%r13b", "%r14b", "%r15b"
};
static const char att_names_seg[][4] = {
  "%es", "%cs", "%ss", "%ds", "%fs", "%gs", "%?", "%?",
};
static const char att_index64[] = "%riz";
static const char att_index32[] = "%eiz";
static const char att_index16[][8] = {
  "%bx,%si", "%bx,%di", "%bp,%si", "%bp,%di", "%si", "%di", "%bp", "%bx"
};

static const char att_names_mm[][8] = {
  "%mm0", "%mm1", "%mm2", "%mm3",
  "%mm4", "%mm5", "%mm6", "%mm7"
};

static const char att_names_bnd[][8] = {
  "%bnd0", "%bnd1", "%bnd2", "%bnd3"
};

static const char att_names_xmm[][8] = {
  "%xmm0", "%xmm1", "%xmm2", "%xmm3",
  "%xmm4", "%xmm5", "%xmm6", "%xmm7",
  "%xmm8", "%xmm9", "%xmm10", "%xmm11",
  "%xmm12", "%xmm13", "%xmm14", "%xmm15",
  "%xmm16", "%xmm17", "%xmm18", "%xmm19",
  "%xmm20", "%xmm21", "%xmm22", "%xmm23",
  "%xmm24", "%xmm25", "%xmm26", "%xmm27",
  "%xmm28", "%xmm29", "%xmm30", "%xmm31"
};

static const char att_names_ymm[][8] = {
  "%ymm0", "%ymm1", "%ymm2", "%ymm3",
  "%ymm4", "%ymm5", "%ymm6", "%ymm7",
  "%ymm8", "%ymm9", "%ymm10", "%ymm11",
  "%ymm12", "%ymm13", "%ymm14", "%ymm15",
  "%ymm16", "%ymm17", "%ymm18", "%ymm19",
  "%ymm20", "%ymm21", "%ymm22", "%ymm23",
  "%ymm24", "%ymm25", "%ymm26", "%ymm27",
  "%ymm28", "%ymm29", "%ymm30", "%ymm31"
};

static const char att_names_zmm[][8] = {
  "%zmm0", "%zmm1", "%zmm2", "%zmm3",
  "%zmm4", "%zmm5", "%zmm6", "%zmm7",
  "%zmm8", "%zmm9", "%zmm10", "%zmm11",
  "%zmm12", "%zmm13", "%zmm14", "%zmm15",
  "%zmm16", "%zmm17", "%zmm18", "%zmm19",
  "%zmm20", "%zmm21", "%zmm22", "%zmm23",
  "%zmm24", "%zmm25", "%zmm26", "%zmm27",
  "%zmm28", "%zmm29", "%zmm30", "%zmm31"
};

static const char att_names_tmm[][8] = {
  "%tmm0", "%tmm1", "%tmm2", "%tmm3",
  "%tmm4", "%tmm5", "%tmm6", "%tmm7"
};

static const char att_names_mask[][8] = {
  "%k0", "%k1", "%k2", "%k3", "%k4", "%k5", "%k6", "%k7"
};

static const char *const names_rounding[] =
{
  "{rn-",
  "{rd-",
  "{ru-",
  "{rz-"
};

static const struct dis386 reg_table[][8] = {
  /* REG_80 */
  {
    { "addA",	{ Ebh1, Ib }, 0 },
    { "orA",	{ Ebh1, Ib }, 0 },
    { "adcA",	{ Ebh1, Ib }, 0 },
    { "sbbA",	{ Ebh1, Ib }, 0 },
    { "andA",	{ Ebh1, Ib }, 0 },
    { "subA",	{ Ebh1, Ib }, 0 },
    { "xorA",	{ Ebh1, Ib }, 0 },
    { "cmpA",	{ Eb, Ib }, 0 },
  },
  /* REG_81 */
  {
    { "addQ",	{ Evh1, Iv }, 0 },
    { "orQ",	{ Evh1, Iv }, 0 },
    { "adcQ",	{ Evh1, Iv }, 0 },
    { "sbbQ",	{ Evh1, Iv }, 0 },
    { "andQ",	{ Evh1, Iv }, 0 },
    { "subQ",	{ Evh1, Iv }, 0 },
    { "xorQ",	{ Evh1, Iv }, 0 },
    { "cmpQ",	{ Ev, Iv }, 0 },
  },
  /* REG_83 */
  {
    { "addQ",	{ Evh1, sIb }, 0 },
    { "orQ",	{ Evh1, sIb }, 0 },
    { "adcQ",	{ Evh1, sIb }, 0 },
    { "sbbQ",	{ Evh1, sIb }, 0 },
    { "andQ",	{ Evh1, sIb }, 0 },
    { "subQ",	{ Evh1, sIb }, 0 },
    { "xorQ",	{ Evh1, sIb }, 0 },
    { "cmpQ",	{ Ev, sIb }, 0 },
  },
  /* REG_8F */
  {
    { "pop{P|}", { stackEv }, 0 },
    { XOP_8F_TABLE (XOP_09) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { XOP_8F_TABLE (XOP_09) },
  },
  /* REG_C0 */
  {
    { "rolA",	{ Eb, Ib }, 0 },
    { "rorA",	{ Eb, Ib }, 0 },
    { "rclA",	{ Eb, Ib }, 0 },
    { "rcrA",	{ Eb, Ib }, 0 },
    { "shlA",	{ Eb, Ib }, 0 },
    { "shrA",	{ Eb, Ib }, 0 },
    { "shlA",	{ Eb, Ib }, 0 },
    { "sarA",	{ Eb, Ib }, 0 },
  },
  /* REG_C1 */
  {
    { "rolQ",	{ Ev, Ib }, 0 },
    { "rorQ",	{ Ev, Ib }, 0 },
    { "rclQ",	{ Ev, Ib }, 0 },
    { "rcrQ",	{ Ev, Ib }, 0 },
    { "shlQ",	{ Ev, Ib }, 0 },
    { "shrQ",	{ Ev, Ib }, 0 },
    { "shlQ",	{ Ev, Ib }, 0 },
    { "sarQ",	{ Ev, Ib }, 0 },
  },
  /* REG_C6 */
  {
    { "movA",	{ Ebh3, Ib }, 0 },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { MOD_TABLE (MOD_C6_REG_7) },
  },
  /* REG_C7 */
  {
    { "movQ",	{ Evh3, Iv }, 0 },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { MOD_TABLE (MOD_C7_REG_7) },
  },
  /* REG_D0 */
  {
    { "rolA",	{ Eb, I1 }, 0 },
    { "rorA",	{ Eb, I1 }, 0 },
    { "rclA",	{ Eb, I1 }, 0 },
    { "rcrA",	{ Eb, I1 }, 0 },
    { "shlA",	{ Eb, I1 }, 0 },
    { "shrA",	{ Eb, I1 }, 0 },
    { "shlA",	{ Eb, I1 }, 0 },
    { "sarA",	{ Eb, I1 }, 0 },
  },
  /* REG_D1 */
  {
    { "rolQ",	{ Ev, I1 }, 0 },
    { "rorQ",	{ Ev, I1 }, 0 },
    { "rclQ",	{ Ev, I1 }, 0 },
    { "rcrQ",	{ Ev, I1 }, 0 },
    { "shlQ",	{ Ev, I1 }, 0 },
    { "shrQ",	{ Ev, I1 }, 0 },
    { "shlQ",	{ Ev, I1 }, 0 },
    { "sarQ",	{ Ev, I1 }, 0 },
  },
  /* REG_D2 */
  {
    { "rolA",	{ Eb, CL }, 0 },
    { "rorA",	{ Eb, CL }, 0 },
    { "rclA",	{ Eb, CL }, 0 },
    { "rcrA",	{ Eb, CL }, 0 },
    { "shlA",	{ Eb, CL }, 0 },
    { "shrA",	{ Eb, CL }, 0 },
    { "shlA",	{ Eb, CL }, 0 },
    { "sarA",	{ Eb, CL }, 0 },
  },
  /* REG_D3 */
  {
    { "rolQ",	{ Ev, CL }, 0 },
    { "rorQ",	{ Ev, CL }, 0 },
    { "rclQ",	{ Ev, CL }, 0 },
    { "rcrQ",	{ Ev, CL }, 0 },
    { "shlQ",	{ Ev, CL }, 0 },
    { "shrQ",	{ Ev, CL }, 0 },
    { "shlQ",	{ Ev, CL }, 0 },
    { "sarQ",	{ Ev, CL }, 0 },
  },
  /* REG_F6 */
  {
    { "testA",	{ Eb, Ib }, 0 },
    { "testA",	{ Eb, Ib }, 0 },
    { "notA",	{ Ebh1 }, 0 },
    { "negA",	{ Ebh1 }, 0 },
    { "mulA",	{ Eb }, 0 },	/* Don't print the implicit %al register,  */
    { "imulA",	{ Eb }, 0 },	/* to distinguish these opcodes from other */
    { "divA",	{ Eb }, 0 },	/* mul/imul opcodes.  Do the same for div  */
    { "idivA",	{ Eb }, 0 },	/* and idiv for consistency.		   */
  },
  /* REG_F7 */
  {
    { "testQ",	{ Ev, Iv }, 0 },
    { "testQ",	{ Ev, Iv }, 0 },
    { "notQ",	{ Evh1 }, 0 },
    { "negQ",	{ Evh1 }, 0 },
    { "mulQ",	{ Ev }, 0 },	/* Don't print the implicit register.  */
    { "imulQ",	{ Ev }, 0 },
    { "divQ",	{ Ev }, 0 },
    { "idivQ",	{ Ev }, 0 },
  },
  /* REG_FE */
  {
    { "incA",	{ Ebh1 }, 0 },
    { "decA",	{ Ebh1 }, 0 },
  },
  /* REG_FF */
  {
    { "incQ",	{ Evh1 }, 0 },
    { "decQ",	{ Evh1 }, 0 },
    { "call{@|}", { NOTRACK, indirEv, BND }, 0 },
    { MOD_TABLE (MOD_FF_REG_3) },
    { "jmp{@|}", { NOTRACK, indirEv, BND }, 0 },
    { MOD_TABLE (MOD_FF_REG_5) },
    { "push{P|}", { stackEv }, 0 },
    { Bad_Opcode },
  },
  /* REG_0F00 */
  {
    { "sldtD",	{ Sv }, 0 },
    { "strD",	{ Sv }, 0 },
    { "lldt",	{ Ew }, 0 },
    { "ltr",	{ Ew }, 0 },
    { "verr",	{ Ew }, 0 },
    { "verw",	{ Ew }, 0 },
    { X86_64_TABLE (X86_64_0F00_REG_6) },
    { Bad_Opcode },
  },
  /* REG_0F01 */
  {
    { MOD_TABLE (MOD_0F01_REG_0) },
    { MOD_TABLE (MOD_0F01_REG_1) },
    { MOD_TABLE (MOD_0F01_REG_2) },
    { MOD_TABLE (MOD_0F01_REG_3) },
    { "smswD",	{ Sv }, 0 },
    { MOD_TABLE (MOD_0F01_REG_5) },
    { "lmsw",	{ Ew }, 0 },
    { MOD_TABLE (MOD_0F01_REG_7) },
  },
  /* REG_0F0D */
  {
    { "prefetch",	{ Mb }, 0 },
    { "prefetchw",	{ Mb }, 0 },
    { "prefetchwt1",	{ Mb }, 0 },
    { "prefetch",	{ Mb }, 0 },
    { "prefetch",	{ Mb }, 0 },
    { "prefetch",	{ Mb }, 0 },
    { "prefetch",	{ Mb }, 0 },
    { "prefetch",	{ Mb }, 0 },
  },
  /* REG_0F18 */
  {
    { MOD_TABLE (MOD_0F18_REG_0) },
    { MOD_TABLE (MOD_0F18_REG_1) },
    { MOD_TABLE (MOD_0F18_REG_2) },
    { MOD_TABLE (MOD_0F18_REG_3) },
    { "nopQ",		{ Ev }, 0 },
    { "nopQ",		{ Ev }, 0 },
    { MOD_TABLE (MOD_0F18_REG_6) },
    { MOD_TABLE (MOD_0F18_REG_7) },
  },
  /* REG_0F1C_P_0_MOD_0 */
  {
    { "cldemote",	{ Mb }, 0 },
    { "nopQ",		{ Ev }, 0 },
    { "nopQ",		{ Ev }, 0 },
    { "nopQ",		{ Ev }, 0 },
    { "nopQ",		{ Ev }, 0 },
    { "nopQ",		{ Ev }, 0 },
    { "nopQ",		{ Ev }, 0 },
    { "nopQ",		{ Ev }, 0 },
  },
  /* REG_0F1E_P_1_MOD_3 */
  {
    { "nopQ",		{ Ev }, PREFIX_IGNORED },
    { "rdsspK",		{ Edq }, 0 },
    { "nopQ",		{ Ev }, PREFIX_IGNORED },
    { "nopQ",		{ Ev }, PREFIX_IGNORED },
    { "nopQ",		{ Ev }, PREFIX_IGNORED },
    { "nopQ",		{ Ev }, PREFIX_IGNORED },
    { "nopQ",		{ Ev }, PREFIX_IGNORED },
    { RM_TABLE (RM_0F1E_P_1_MOD_3_REG_7) },
  },
  /* REG_0F38D8_PREFIX_1 */
  {
    { "aesencwide128kl",	{ M }, 0 },
    { "aesdecwide128kl",	{ M }, 0 },
    { "aesencwide256kl",	{ M }, 0 },
    { "aesdecwide256kl",	{ M }, 0 },
  },
  /* REG_0F3A0F_PREFIX_1_MOD_3 */
  {
    { RM_TABLE (RM_0F3A0F_P_1_MOD_3_REG_0) },
  },
  /* REG_0F71_MOD_0 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "psrlw",		{ MS, Ib }, PREFIX_OPCODE },
    { Bad_Opcode },
    { "psraw",		{ MS, Ib }, PREFIX_OPCODE },
    { Bad_Opcode },
    { "psllw",		{ MS, Ib }, PREFIX_OPCODE },
  },
  /* REG_0F72_MOD_0 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "psrld",		{ MS, Ib }, PREFIX_OPCODE },
    { Bad_Opcode },
    { "psrad",		{ MS, Ib }, PREFIX_OPCODE },
    { Bad_Opcode },
    { "pslld",		{ MS, Ib }, PREFIX_OPCODE },
  },
  /* REG_0F73_MOD_0 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "psrlq",		{ MS, Ib }, PREFIX_OPCODE },
    { "psrldq",		{ XS, Ib }, PREFIX_DATA },
    { Bad_Opcode },
    { Bad_Opcode },
    { "psllq",		{ MS, Ib }, PREFIX_OPCODE },
    { "pslldq",		{ XS, Ib }, PREFIX_DATA },
  },
  /* REG_0FA6 */
  {
    { "montmul",	{ { OP_0f07, 0 } }, 0 },
    { "xsha1",		{ { OP_0f07, 0 } }, 0 },
    { "xsha256",	{ { OP_0f07, 0 } }, 0 },
  },
  /* REG_0FA7 */
  {
    { "xstore-rng",	{ { OP_0f07, 0 } }, 0 },
    { "xcrypt-ecb",	{ { OP_0f07, 0 } }, 0 },
    { "xcrypt-cbc",	{ { OP_0f07, 0 } }, 0 },
    { "xcrypt-ctr",	{ { OP_0f07, 0 } }, 0 },
    { "xcrypt-cfb",	{ { OP_0f07, 0 } }, 0 },
    { "xcrypt-ofb",	{ { OP_0f07, 0 } }, 0 },
  },
  /* REG_0FAE */
  {
    { MOD_TABLE (MOD_0FAE_REG_0) },
    { MOD_TABLE (MOD_0FAE_REG_1) },
    { MOD_TABLE (MOD_0FAE_REG_2) },
    { MOD_TABLE (MOD_0FAE_REG_3) },
    { MOD_TABLE (MOD_0FAE_REG_4) },
    { MOD_TABLE (MOD_0FAE_REG_5) },
    { MOD_TABLE (MOD_0FAE_REG_6) },
    { MOD_TABLE (MOD_0FAE_REG_7) },
  },
  /* REG_0FBA */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { "btQ",	{ Ev, Ib }, 0 },
    { "btsQ",	{ Evh1, Ib }, 0 },
    { "btrQ",	{ Evh1, Ib }, 0 },
    { "btcQ",	{ Evh1, Ib }, 0 },
  },
  /* REG_0FC7 */
  {
    { Bad_Opcode },
    { "cmpxchg8b", { { CMPXCHG8B_Fixup, q_mode } }, 0 },
    { Bad_Opcode },
    { MOD_TABLE (MOD_0FC7_REG_3) },
    { MOD_TABLE (MOD_0FC7_REG_4) },
    { MOD_TABLE (MOD_0FC7_REG_5) },
    { MOD_TABLE (MOD_0FC7_REG_6) },
    { MOD_TABLE (MOD_0FC7_REG_7) },
  },
  /* REG_VEX_0F71_M_0 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vpsrlw",		{ Vex, XS, Ib }, PREFIX_DATA },
    { Bad_Opcode },
    { "vpsraw",		{ Vex, XS, Ib }, PREFIX_DATA },
    { Bad_Opcode },
    { "vpsllw",		{ Vex, XS, Ib }, PREFIX_DATA },
  },
  /* REG_VEX_0F72_M_0 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vpsrld",		{ Vex, XS, Ib }, PREFIX_DATA },
    { Bad_Opcode },
    { "vpsrad",		{ Vex, XS, Ib }, PREFIX_DATA },
    { Bad_Opcode },
    { "vpslld",		{ Vex, XS, Ib }, PREFIX_DATA },
  },
  /* REG_VEX_0F73_M_0 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vpsrlq",		{ Vex, XS, Ib }, PREFIX_DATA },
    { "vpsrldq",	{ Vex, XS, Ib }, PREFIX_DATA },
    { Bad_Opcode },
    { Bad_Opcode },
    { "vpsllq",		{ Vex, XS, Ib }, PREFIX_DATA },
    { "vpslldq",	{ Vex, XS, Ib }, PREFIX_DATA },
  },
  /* REG_VEX_0FAE */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { MOD_TABLE (MOD_VEX_0FAE_REG_2) },
    { MOD_TABLE (MOD_VEX_0FAE_REG_3) },
  },
  /* REG_VEX_0F3849_X86_64_L_0_W_0_M_1_P_0 */
  {
    { RM_TABLE (RM_VEX_0F3849_X86_64_L_0_W_0_M_1_P_0_R_0) },
  },
  /* REG_VEX_0F38F3_L_0 */
  {
    { Bad_Opcode },
    { "blsrS",		{ VexGdq, Edq }, PREFIX_OPCODE },
    { "blsmskS",	{ VexGdq, Edq }, PREFIX_OPCODE },
    { "blsiS",		{ VexGdq, Edq }, PREFIX_OPCODE },
  },
  /* REG_XOP_09_01_L_0 */
  {
    { Bad_Opcode },
    { "blcfill",	{ VexGdq, Edq }, 0 },
    { "blsfill",	{ VexGdq, Edq }, 0 },
    { "blcs",	{ VexGdq, Edq }, 0 },
    { "tzmsk",	{ VexGdq, Edq }, 0 },
    { "blcic",	{ VexGdq, Edq }, 0 },
    { "blsic",	{ VexGdq, Edq }, 0 },
    { "t1mskc",	{ VexGdq, Edq }, 0 },
  },
  /* REG_XOP_09_02_L_0 */
  {
    { Bad_Opcode },
    { "blcmsk",	{ VexGdq, Edq }, 0 },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { "blci",	{ VexGdq, Edq }, 0 },
  },
  /* REG_XOP_09_12_M_1_L_0 */
  {
    { "llwpcb",	{ Edq }, 0 },
    { "slwpcb",	{ Edq }, 0 },
  },
  /* REG_XOP_0A_12_L_0 */
  {
    { "lwpins",	{ VexGdq, Ed, Id }, 0 },
    { "lwpval",	{ VexGdq, Ed, Id }, 0 },
  },

#include "i386-dis-evex-reg.h"
};

static const struct dis386 prefix_table[][4] = {
  /* PREFIX_90 */
  {
    { "xchgS", { { NOP_Fixup, 0 }, { NOP_Fixup, 1 } }, 0 },
    { "pause", { XX }, 0 },
    { "xchgS", { { NOP_Fixup, 0 }, { NOP_Fixup, 1 } }, 0 },
    { NULL, { { NULL, 0 } }, PREFIX_IGNORED }
  },

  /* PREFIX_0F00_REG_6_X86_64 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { "lkgs",  { Ew }, 0 },
  },

  /* PREFIX_0F01_REG_0_MOD_3_RM_6 */
  {
    { "wrmsrns",        { Skip_MODRM }, 0 },
    { X86_64_TABLE (X86_64_0F01_REG_0_MOD_3_RM_6_P_1) },
    { Bad_Opcode },
    { X86_64_TABLE (X86_64_0F01_REG_0_MOD_3_RM_6_P_3) },
  },

  /* PREFIX_0F01_REG_1_RM_2 */
  {
    { "clac",		{ Skip_MODRM }, 0 },
    { X86_64_TABLE (X86_64_0F01_REG_1_RM_2_PREFIX_1) },
    { Bad_Opcode },
    { X86_64_TABLE (X86_64_0F01_REG_1_RM_2_PREFIX_3)},
  },

  /* PREFIX_0F01_REG_1_RM_4 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "tdcall", 	{ Skip_MODRM }, 0 },
    { Bad_Opcode },
  },

  /* PREFIX_0F01_REG_1_RM_5 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { X86_64_TABLE (X86_64_0F01_REG_1_RM_5_PREFIX_2) },
    { Bad_Opcode },
  },

  /* PREFIX_0F01_REG_1_RM_6 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { X86_64_TABLE (X86_64_0F01_REG_1_RM_6_PREFIX_2) },
    { Bad_Opcode },
  },

  /* PREFIX_0F01_REG_1_RM_7 */
  {
    { "encls",		{ Skip_MODRM }, 0 },
    { Bad_Opcode },
    { X86_64_TABLE (X86_64_0F01_REG_1_RM_7_PREFIX_2) },
    { Bad_Opcode },
  },

  /* PREFIX_0F01_REG_3_RM_1 */
  {
    { "vmmcall",	{ Skip_MODRM }, 0 },
    { "vmgexit",	{ Skip_MODRM }, 0 },
    { Bad_Opcode },
    { "vmgexit",	{ Skip_MODRM }, 0 },
  },

  /* PREFIX_0F01_REG_5_MOD_0 */
  {
    { Bad_Opcode },
    { "rstorssp",	{ Mq }, PREFIX_OPCODE },
  },

  /* PREFIX_0F01_REG_5_MOD_3_RM_0 */
  {
    { "serialize",	{ Skip_MODRM }, PREFIX_OPCODE },
    { "setssbsy",	{ Skip_MODRM }, PREFIX_OPCODE },
    { Bad_Opcode },
    { "xsusldtrk",	{ Skip_MODRM }, PREFIX_OPCODE },
  },

  /* PREFIX_0F01_REG_5_MOD_3_RM_1 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { "xresldtrk",     { Skip_MODRM }, PREFIX_OPCODE },
  },

  /* PREFIX_0F01_REG_5_MOD_3_RM_2 */
  {
    { Bad_Opcode },
    { "saveprevssp",	{ Skip_MODRM }, PREFIX_OPCODE },
  },

  /* PREFIX_0F01_REG_5_MOD_3_RM_4 */
  {
    { Bad_Opcode },
    { X86_64_TABLE (X86_64_0F01_REG_5_MOD_3_RM_4_PREFIX_1) },
  },

  /* PREFIX_0F01_REG_5_MOD_3_RM_5 */
  {
    { Bad_Opcode },
    { X86_64_TABLE (X86_64_0F01_REG_5_MOD_3_RM_5_PREFIX_1) },
  },

  /* PREFIX_0F01_REG_5_MOD_3_RM_6 */
  {
    { "rdpkru", { Skip_MODRM }, 0 },
    { X86_64_TABLE (X86_64_0F01_REG_5_MOD_3_RM_6_PREFIX_1) },
  },

  /* PREFIX_0F01_REG_5_MOD_3_RM_7 */
  {
    { "wrpkru",	{ Skip_MODRM }, 0 },
    { X86_64_TABLE (X86_64_0F01_REG_5_MOD_3_RM_7_PREFIX_1) },
  },

  /* PREFIX_0F01_REG_7_MOD_3_RM_2 */
  {
    { "monitorx",	{ { OP_Monitor, 0 } }, 0  },
    { "mcommit",	{ Skip_MODRM }, 0 },
  },

  /* PREFIX_0F01_REG_7_MOD_3_RM_5 */
  {
    { "rdpru", { Skip_MODRM }, 0 },
    { X86_64_TABLE (X86_64_0F01_REG_7_MOD_3_RM_5_PREFIX_1) },
  },

  /* PREFIX_0F01_REG_7_MOD_3_RM_6 */
  {
    { "invlpgb",        { Skip_MODRM }, 0 },
    { X86_64_TABLE (X86_64_0F01_REG_7_MOD_3_RM_6_PREFIX_1) },
    { Bad_Opcode },
    { X86_64_TABLE (X86_64_0F01_REG_7_MOD_3_RM_6_PREFIX_3) },
  },

  /* PREFIX_0F01_REG_7_MOD_3_RM_7 */
  {
    { "tlbsync",        { Skip_MODRM }, 0 },
    { X86_64_TABLE (X86_64_0F01_REG_7_MOD_3_RM_7_PREFIX_1) },
    { Bad_Opcode },
    { "pvalidate",      { Skip_MODRM }, 0 },
  },

  /* PREFIX_0F09 */
  {
    { "wbinvd",   { XX }, 0 },
    { "wbnoinvd", { XX }, 0 },
  },

  /* PREFIX_0F10 */
  {
    { "movups",	{ XM, EXx }, PREFIX_OPCODE },
    { "movss",	{ XM, EXd }, PREFIX_OPCODE },
    { "movupd",	{ XM, EXx }, PREFIX_OPCODE },
    { "movsd",	{ XM, EXq }, PREFIX_OPCODE },
  },

  /* PREFIX_0F11 */
  {
    { "movups",	{ EXxS, XM }, PREFIX_OPCODE },
    { "movss",	{ EXdS, XM }, PREFIX_OPCODE },
    { "movupd",	{ EXxS, XM }, PREFIX_OPCODE },
    { "movsd",	{ EXqS, XM }, PREFIX_OPCODE },
  },

  /* PREFIX_0F12 */
  {
    { MOD_TABLE (MOD_0F12_PREFIX_0) },
    { "movsldup", { XM, EXx }, PREFIX_OPCODE },
    { MOD_TABLE (MOD_0F12_PREFIX_2) },
    { "movddup", { XM, EXq }, PREFIX_OPCODE },
  },

  /* PREFIX_0F16 */
  {
    { MOD_TABLE (MOD_0F16_PREFIX_0) },
    { "movshdup", { XM, EXx }, PREFIX_OPCODE },
    { MOD_TABLE (MOD_0F16_PREFIX_2) },
  },

  /* PREFIX_0F18_REG_6_MOD_0_X86_64 */
  {
    { "prefetchit1",	{ { PREFETCHI_Fixup, b_mode } }, 0 },
    { "nopQ",		{ Ev }, 0 },
    { "nopQ",		{ Ev }, 0 },
    { "nopQ",		{ Ev }, 0 },
  },

  /* PREFIX_0F18_REG_7_MOD_0_X86_64 */
  {
    { "prefetchit0",	{ { PREFETCHI_Fixup, b_mode } }, 0 },
    { "nopQ",		{ Ev }, 0 },
    { "nopQ",		{ Ev }, 0 },
    { "nopQ",		{ Ev }, 0 },
  },

  /* PREFIX_0F1A */
  {
    { MOD_TABLE (MOD_0F1A_PREFIX_0) },
    { "bndcl",  { Gbnd, Ev_bnd }, 0 },
    { "bndmov", { Gbnd, Ebnd }, 0 },
    { "bndcu",  { Gbnd, Ev_bnd }, 0 },
  },

  /* PREFIX_0F1B */
  {
    { MOD_TABLE (MOD_0F1B_PREFIX_0) },
    { MOD_TABLE (MOD_0F1B_PREFIX_1) },
    { "bndmov", { EbndS, Gbnd }, 0 },
    { "bndcn",  { Gbnd, Ev_bnd }, 0 },
  },

  /* PREFIX_0F1C */
  {
    { MOD_TABLE (MOD_0F1C_PREFIX_0) },
    { "nopQ",	{ Ev }, PREFIX_IGNORED },
    { "nopQ",	{ Ev }, 0 },
    { "nopQ",	{ Ev }, PREFIX_IGNORED },
  },

  /* PREFIX_0F1E */
  {
    { "nopQ",	{ Ev }, 0 },
    { MOD_TABLE (MOD_0F1E_PREFIX_1) },
    { "nopQ",	{ Ev }, 0 },
    { NULL,	{ XX }, PREFIX_IGNORED },
  },

  /* PREFIX_0F2A */
  {
    { "cvtpi2ps", { XM, EMCq }, PREFIX_OPCODE },
    { "cvtsi2ss{%LQ|}", { XM, Edq }, PREFIX_OPCODE },
    { "cvtpi2pd", { XM, EMCq }, PREFIX_OPCODE },
    { "cvtsi2sd{%LQ|}", { XM, Edq }, 0 },
  },

  /* PREFIX_0F2B */
  {
    { MOD_TABLE (MOD_0F2B_PREFIX_0) },
    { MOD_TABLE (MOD_0F2B_PREFIX_1) },
    { MOD_TABLE (MOD_0F2B_PREFIX_2) },
    { MOD_TABLE (MOD_0F2B_PREFIX_3) },
  },

  /* PREFIX_0F2C */
  {
    { "cvttps2pi", { MXC, EXq }, PREFIX_OPCODE },
    { "cvttss2si", { Gdq, EXd }, PREFIX_OPCODE },
    { "cvttpd2pi", { MXC, EXx }, PREFIX_OPCODE },
    { "cvttsd2si", { Gdq, EXq }, PREFIX_OPCODE },
  },

  /* PREFIX_0F2D */
  {
    { "cvtps2pi", { MXC, EXq }, PREFIX_OPCODE },
    { "cvtss2si", { Gdq, EXd }, PREFIX_OPCODE },
    { "cvtpd2pi", { MXC, EXx }, PREFIX_OPCODE },
    { "cvtsd2si", { Gdq, EXq }, PREFIX_OPCODE },
  },

  /* PREFIX_0F2E */
  {
    { "ucomiss",{ XM, EXd }, 0 },
    { Bad_Opcode },
    { "ucomisd",{ XM, EXq }, 0 },
  },

  /* PREFIX_0F2F */
  {
    { "comiss",	{ XM, EXd }, 0 },
    { Bad_Opcode },
    { "comisd",	{ XM, EXq }, 0 },
  },

  /* PREFIX_0F51 */
  {
    { "sqrtps", { XM, EXx }, PREFIX_OPCODE },
    { "sqrtss", { XM, EXd }, PREFIX_OPCODE },
    { "sqrtpd", { XM, EXx }, PREFIX_OPCODE },
    { "sqrtsd",	{ XM, EXq }, PREFIX_OPCODE },
  },

  /* PREFIX_0F52 */
  {
    { "rsqrtps",{ XM, EXx }, PREFIX_OPCODE },
    { "rsqrtss",{ XM, EXd }, PREFIX_OPCODE },
  },

  /* PREFIX_0F53 */
  {
    { "rcpps",	{ XM, EXx }, PREFIX_OPCODE },
    { "rcpss",	{ XM, EXd }, PREFIX_OPCODE },
  },

  /* PREFIX_0F58 */
  {
    { "addps", { XM, EXx }, PREFIX_OPCODE },
    { "addss", { XM, EXd }, PREFIX_OPCODE },
    { "addpd", { XM, EXx }, PREFIX_OPCODE },
    { "addsd", { XM, EXq }, PREFIX_OPCODE },
  },

  /* PREFIX_0F59 */
  {
    { "mulps",	{ XM, EXx }, PREFIX_OPCODE },
    { "mulss",	{ XM, EXd }, PREFIX_OPCODE },
    { "mulpd",	{ XM, EXx }, PREFIX_OPCODE },
    { "mulsd",	{ XM, EXq }, PREFIX_OPCODE },
  },

  /* PREFIX_0F5A */
  {
    { "cvtps2pd", { XM, EXq }, PREFIX_OPCODE },
    { "cvtss2sd", { XM, EXd }, PREFIX_OPCODE },
    { "cvtpd2ps", { XM, EXx }, PREFIX_OPCODE },
    { "cvtsd2ss", { XM, EXq }, PREFIX_OPCODE },
  },

  /* PREFIX_0F5B */
  {
    { "cvtdq2ps", { XM, EXx }, PREFIX_OPCODE },
    { "cvttps2dq", { XM, EXx }, PREFIX_OPCODE },
    { "cvtps2dq", { XM, EXx }, PREFIX_OPCODE },
  },

  /* PREFIX_0F5C */
  {
    { "subps",	{ XM, EXx }, PREFIX_OPCODE },
    { "subss",	{ XM, EXd }, PREFIX_OPCODE },
    { "subpd",	{ XM, EXx }, PREFIX_OPCODE },
    { "subsd",	{ XM, EXq }, PREFIX_OPCODE },
  },

  /* PREFIX_0F5D */
  {
    { "minps",	{ XM, EXx }, PREFIX_OPCODE },
    { "minss",	{ XM, EXd }, PREFIX_OPCODE },
    { "minpd",	{ XM, EXx }, PREFIX_OPCODE },
    { "minsd",	{ XM, EXq }, PREFIX_OPCODE },
  },

  /* PREFIX_0F5E */
  {
    { "divps",	{ XM, EXx }, PREFIX_OPCODE },
    { "divss",	{ XM, EXd }, PREFIX_OPCODE },
    { "divpd",	{ XM, EXx }, PREFIX_OPCODE },
    { "divsd",	{ XM, EXq }, PREFIX_OPCODE },
  },

  /* PREFIX_0F5F */
  {
    { "maxps",	{ XM, EXx }, PREFIX_OPCODE },
    { "maxss",	{ XM, EXd }, PREFIX_OPCODE },
    { "maxpd",	{ XM, EXx }, PREFIX_OPCODE },
    { "maxsd",	{ XM, EXq }, PREFIX_OPCODE },
  },

  /* PREFIX_0F60 */
  {
    { "punpcklbw",{ MX, EMd }, PREFIX_OPCODE },
    { Bad_Opcode },
    { "punpcklbw",{ MX, EMx }, PREFIX_OPCODE },
  },

  /* PREFIX_0F61 */
  {
    { "punpcklwd",{ MX, EMd }, PREFIX_OPCODE },
    { Bad_Opcode },
    { "punpcklwd",{ MX, EMx }, PREFIX_OPCODE },
  },

  /* PREFIX_0F62 */
  {
    { "punpckldq",{ MX, EMd }, PREFIX_OPCODE },
    { Bad_Opcode },
    { "punpckldq",{ MX, EMx }, PREFIX_OPCODE },
  },

  /* PREFIX_0F6F */
  {
    { "movq",	{ MX, EM }, PREFIX_OPCODE },
    { "movdqu",	{ XM, EXx }, PREFIX_OPCODE },
    { "movdqa",	{ XM, EXx }, PREFIX_OPCODE },
  },

  /* PREFIX_0F70 */
  {
    { "pshufw",	{ MX, EM, Ib }, PREFIX_OPCODE },
    { "pshufhw",{ XM, EXx, Ib }, PREFIX_OPCODE },
    { "pshufd",	{ XM, EXx, Ib }, PREFIX_OPCODE },
    { "pshuflw",{ XM, EXx, Ib }, PREFIX_OPCODE },
  },

  /* PREFIX_0F78 */
  {
    {"vmread",	{ Em, Gm }, 0 },
    { Bad_Opcode },
    {"extrq",	{ XS, Ib, Ib }, 0 },
    {"insertq",	{ XM, XS, Ib, Ib }, 0 },
  },

  /* PREFIX_0F79 */
  {
    {"vmwrite",	{ Gm, Em }, 0 },
    { Bad_Opcode },
    {"extrq",	{ XM, XS }, 0 },
    {"insertq",	{ XM, XS }, 0 },
  },

  /* PREFIX_0F7C */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "haddpd",	{ XM, EXx }, PREFIX_OPCODE },
    { "haddps",	{ XM, EXx }, PREFIX_OPCODE },
  },

  /* PREFIX_0F7D */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "hsubpd",	{ XM, EXx }, PREFIX_OPCODE },
    { "hsubps",	{ XM, EXx }, PREFIX_OPCODE },
  },

  /* PREFIX_0F7E */
  {
    { "movK",	{ Edq, MX }, PREFIX_OPCODE },
    { "movq",	{ XM, EXq }, PREFIX_OPCODE },
    { "movK",	{ Edq, XM }, PREFIX_OPCODE },
  },

  /* PREFIX_0F7F */
  {
    { "movq",	{ EMS, MX }, PREFIX_OPCODE },
    { "movdqu",	{ EXxS, XM }, PREFIX_OPCODE },
    { "movdqa",	{ EXxS, XM }, PREFIX_OPCODE },
  },

  /* PREFIX_0FAE_REG_0_MOD_3 */
  {
    { Bad_Opcode },
    { "rdfsbase", { Ev }, 0 },
  },

  /* PREFIX_0FAE_REG_1_MOD_3 */
  {
    { Bad_Opcode },
    { "rdgsbase", { Ev }, 0 },
  },

  /* PREFIX_0FAE_REG_2_MOD_3 */
  {
    { Bad_Opcode },
    { "wrfsbase", { Ev }, 0 },
  },

  /* PREFIX_0FAE_REG_3_MOD_3 */
  {
    { Bad_Opcode },
    { "wrgsbase", { Ev }, 0 },
  },

  /* PREFIX_0FAE_REG_4_MOD_0 */
  {
    { "xsave",	{ FXSAVE }, 0 },
    { "ptwrite{%LQ|}", { Edq }, 0 },
  },

  /* PREFIX_0FAE_REG_4_MOD_3 */
  {
    { Bad_Opcode },
    { "ptwrite{%LQ|}", { Edq }, 0 },
  },

  /* PREFIX_0FAE_REG_5_MOD_3 */
  {
    { "lfence",		{ Skip_MODRM }, 0 },
    { "incsspK",	{ Edq }, PREFIX_OPCODE },
  },

  /* PREFIX_0FAE_REG_6_MOD_0 */
  {
    { "xsaveopt",	{ FXSAVE }, PREFIX_OPCODE },
    { "clrssbsy",	{ Mq }, PREFIX_OPCODE },
    { "clwb",	{ Mb }, PREFIX_OPCODE },
  },

  /* PREFIX_0FAE_REG_6_MOD_3 */
  {
    { RM_TABLE (RM_0FAE_REG_6_MOD_3_P_0) },
    { "umonitor",	{ Eva }, PREFIX_OPCODE },
    { "tpause",	{ Edq }, PREFIX_OPCODE },
    { "umwait",	{ Edq }, PREFIX_OPCODE },
  },

  /* PREFIX_0FAE_REG_7_MOD_0 */
  {
    { "clflush",	{ Mb }, 0 },
    { Bad_Opcode },
    { "clflushopt",	{ Mb }, 0 },
  },

  /* PREFIX_0FB8 */
  {
    { Bad_Opcode },
    { "popcntS", { Gv, Ev }, 0 },
  },

  /* PREFIX_0FBC */
  {
    { "bsfS",	{ Gv, Ev }, 0 },
    { "tzcntS",	{ Gv, Ev }, 0 },
    { "bsfS",	{ Gv, Ev }, 0 },
  },

  /* PREFIX_0FBD */
  {
    { "bsrS",	{ Gv, Ev }, 0 },
    { "lzcntS",	{ Gv, Ev }, 0 },
    { "bsrS",	{ Gv, Ev }, 0 },
  },

  /* PREFIX_0FC2 */
  {
    { "cmpps",	{ XM, EXx, CMP }, PREFIX_OPCODE },
    { "cmpss",	{ XM, EXd, CMP }, PREFIX_OPCODE },
    { "cmppd",	{ XM, EXx, CMP }, PREFIX_OPCODE },
    { "cmpsd",	{ XM, EXq, CMP }, PREFIX_OPCODE },
  },

  /* PREFIX_0FC7_REG_6_MOD_0 */
  {
    { "vmptrld",{ Mq }, 0 },
    { "vmxon",	{ Mq }, 0 },
    { "vmclear",{ Mq }, 0 },
  },

  /* PREFIX_0FC7_REG_6_MOD_3 */
  {
    { "rdrand",	{ Ev }, 0 },
    { X86_64_TABLE (X86_64_0FC7_REG_6_MOD_3_PREFIX_1) },
    { "rdrand",	{ Ev }, 0 }
  },

  /* PREFIX_0FC7_REG_7_MOD_3 */
  {
    { "rdseed",	{ Ev }, 0 },
    { "rdpid",	{ Em }, 0 },
    { "rdseed",	{ Ev }, 0 },
  },

  /* PREFIX_0FD0 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "addsubpd", { XM, EXx }, 0 },
    { "addsubps", { XM, EXx }, 0 },
  },

  /* PREFIX_0FD6 */
  {
    { Bad_Opcode },
    { "movq2dq",{ XM, MS }, 0 },
    { "movq",	{ EXqS, XM }, 0 },
    { "movdq2q",{ MX, XS }, 0 },
  },

  /* PREFIX_0FE6 */
  {
    { Bad_Opcode },
    { "cvtdq2pd", { XM, EXq }, PREFIX_OPCODE },
    { "cvttpd2dq", { XM, EXx }, PREFIX_OPCODE },
    { "cvtpd2dq", { XM, EXx }, PREFIX_OPCODE },
  },

  /* PREFIX_0FE7 */
  {
    { "movntq",	{ Mq, MX }, PREFIX_OPCODE },
    { Bad_Opcode },
    { MOD_TABLE (MOD_0FE7_PREFIX_2) },
  },

  /* PREFIX_0FF0 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { MOD_TABLE (MOD_0FF0_PREFIX_3) },
  },

  /* PREFIX_0FF7 */
  {
    { "maskmovq", { MX, MS }, PREFIX_OPCODE },
    { Bad_Opcode },
    { "maskmovdqu", { XM, XS }, PREFIX_OPCODE },
  },

  /* PREFIX_0F38D8 */
  {
    { Bad_Opcode },
    { REG_TABLE (REG_0F38D8_PREFIX_1) },
  },

  /* PREFIX_0F38DC */
  {
    { Bad_Opcode },
    { MOD_TABLE (MOD_0F38DC_PREFIX_1) },
    { "aesenc", { XM, EXx }, 0 },
  },

  /* PREFIX_0F38DD */
  {
    { Bad_Opcode },
    { MOD_TABLE (MOD_0F38DD_PREFIX_1) },
    { "aesenclast", { XM, EXx }, 0 },
  },

  /* PREFIX_0F38DE */
  {
    { Bad_Opcode },
    { MOD_TABLE (MOD_0F38DE_PREFIX_1) },
    { "aesdec", { XM, EXx }, 0 },
  },

  /* PREFIX_0F38DF */
  {
    { Bad_Opcode },
    { MOD_TABLE (MOD_0F38DF_PREFIX_1) },
    { "aesdeclast", { XM, EXx }, 0 },
  },

  /* PREFIX_0F38F0 */
  {
    { "movbeS",	{ Gv, Mv }, PREFIX_OPCODE },
    { Bad_Opcode },
    { "movbeS",	{ Gv, Mv }, PREFIX_OPCODE },
    { "crc32A",	{ Gdq, Eb }, PREFIX_OPCODE },
  },

  /* PREFIX_0F38F1 */
  {
    { "movbeS",	{ Mv, Gv }, PREFIX_OPCODE },
    { Bad_Opcode },
    { "movbeS",	{ Mv, Gv }, PREFIX_OPCODE },
    { "crc32Q",	{ Gdq, Ev }, PREFIX_OPCODE },
  },

  /* PREFIX_0F38F6 */
  {
    { MOD_TABLE (MOD_0F38F6_PREFIX_0) },
    { "adoxS",	{ Gdq, Edq}, PREFIX_OPCODE },
    { "adcxS",	{ Gdq, Edq}, PREFIX_OPCODE },
    { Bad_Opcode },
  },

  /* PREFIX_0F38F8 */
  {
    { Bad_Opcode },
    { MOD_TABLE (MOD_0F38F8_PREFIX_1) },
    { MOD_TABLE (MOD_0F38F8_PREFIX_2) },
    { MOD_TABLE (MOD_0F38F8_PREFIX_3) },
  },
  /* PREFIX_0F38FA */
  {
    { Bad_Opcode },
    { MOD_TABLE (MOD_0F38FA_PREFIX_1) },
  },

  /* PREFIX_0F38FB */
  {
    { Bad_Opcode },
    { MOD_TABLE (MOD_0F38FB_PREFIX_1) },
  },

  /* PREFIX_0F38FC */
  {
    { "aadd",	{ Mdq, Gdq }, 0 },
    { "axor",	{ Mdq, Gdq }, 0 },
    { "aand",	{ Mdq, Gdq }, 0 },
    { "aor",	{ Mdq, Gdq }, 0 },
  },

  /* PREFIX_0F3A0F */
  {
    { Bad_Opcode },
    { MOD_TABLE (MOD_0F3A0F_PREFIX_1)},
  },

  /* PREFIX_VEX_0F10 */
  {
    { "%XEvmovupX",	{ XM, EXEvexXNoBcst }, 0 },
    { "%XEvmovs%XS",	{ XMScalar, VexScalarR, EXd }, 0 },
    { "%XEvmovupX",	{ XM, EXEvexXNoBcst }, 0 },
    { "%XEvmovs%XD",	{ XMScalar, VexScalarR, EXq }, 0 },
  },

  /* PREFIX_VEX_0F11 */
  {
    { "%XEvmovupX",	{ EXxS, XM }, 0 },
    { "%XEvmovs%XS",	{ EXdS, VexScalarR, XMScalar }, 0 },
    { "%XEvmovupX",	{ EXxS, XM }, 0 },
    { "%XEvmovs%XD",	{ EXqS, VexScalarR, XMScalar }, 0 },
  },

  /* PREFIX_VEX_0F12 */
  {
    { MOD_TABLE (MOD_VEX_0F12_PREFIX_0) },
    { "%XEvmov%XSldup",	{ XM, EXEvexXNoBcst }, 0 },
    { MOD_TABLE (MOD_VEX_0F12_PREFIX_2) },
    { "%XEvmov%XDdup",	{ XM, EXymmq }, 0 },
  },

  /* PREFIX_VEX_0F16 */
  {
    { MOD_TABLE (MOD_VEX_0F16_PREFIX_0) },
    { "%XEvmov%XShdup",	{ XM, EXEvexXNoBcst }, 0 },
    { MOD_TABLE (MOD_VEX_0F16_PREFIX_2) },
  },

  /* PREFIX_VEX_0F2A */
  {
    { Bad_Opcode },
    { "%XEvcvtsi2ss{%LQ|}",	{ XMScalar, VexScalar, EXxEVexR, Edq }, 0 },
    { Bad_Opcode },
    { "%XEvcvtsi2sd{%LQ|}",	{ XMScalar, VexScalar, EXxEVexR64, Edq }, 0 },
  },

  /* PREFIX_VEX_0F2C */
  {
    { Bad_Opcode },
    { "%XEvcvttss2si",	{ Gdq, EXd, EXxEVexS }, 0 },
    { Bad_Opcode },
    { "%XEvcvttsd2si",	{ Gdq, EXq, EXxEVexS }, 0 },
  },

  /* PREFIX_VEX_0F2D */
  {
    { Bad_Opcode },
    { "%XEvcvtss2si",	{ Gdq, EXd, EXxEVexR }, 0 },
    { Bad_Opcode },
    { "%XEvcvtsd2si",	{ Gdq, EXq, EXxEVexR }, 0 },
  },

  /* PREFIX_VEX_0F2E */
  {
    { "%XEvucomisX",	{ XMScalar, EXd, EXxEVexS }, 0 },
    { Bad_Opcode },
    { "%XEvucomisX",	{ XMScalar, EXq, EXxEVexS }, 0 },
  },

  /* PREFIX_VEX_0F2F */
  {
    { "%XEvcomisX",	{ XMScalar, EXd, EXxEVexS }, 0 },
    { Bad_Opcode },
    { "%XEvcomisX",	{ XMScalar, EXq, EXxEVexS }, 0 },
  },

  /* PREFIX_VEX_0F41_L_1_M_1_W_0 */
  {
    { "kandw",          { MaskG, MaskVex, MaskE }, 0 },
    { Bad_Opcode },
    { "kandb",          { MaskG, MaskVex, MaskE }, 0 },
  },

  /* PREFIX_VEX_0F41_L_1_M_1_W_1 */
  {
    { "kandq",          { MaskG, MaskVex, MaskE }, 0 },
    { Bad_Opcode },
    { "kandd",          { MaskG, MaskVex, MaskE }, 0 },
  },

  /* PREFIX_VEX_0F42_L_1_M_1_W_0 */
  {
    { "kandnw",         { MaskG, MaskVex, MaskE }, 0 },
    { Bad_Opcode },
    { "kandnb",         { MaskG, MaskVex, MaskE }, 0 },
  },

  /* PREFIX_VEX_0F42_L_1_M_1_W_1 */
  {
    { "kandnq",         { MaskG, MaskVex, MaskE }, 0 },
    { Bad_Opcode },
    { "kandnd",         { MaskG, MaskVex, MaskE }, 0 },
  },

  /* PREFIX_VEX_0F44_L_0_M_1_W_0 */
  {
    { "knotw",          { MaskG, MaskE }, 0 },
    { Bad_Opcode },
    { "knotb",          { MaskG, MaskE }, 0 },
  },

  /* PREFIX_VEX_0F44_L_0_M_1_W_1 */
  {
    { "knotq",          { MaskG, MaskE }, 0 },
    { Bad_Opcode },
    { "knotd",          { MaskG, MaskE }, 0 },
  },

  /* PREFIX_VEX_0F45_L_1_M_1_W_0 */
  {
    { "korw",       { MaskG, MaskVex, MaskE }, 0 },
    { Bad_Opcode },
    { "korb",       { MaskG, MaskVex, MaskE }, 0 },
  },

  /* PREFIX_VEX_0F45_L_1_M_1_W_1 */
  {
    { "korq",       { MaskG, MaskVex, MaskE }, 0 },
    { Bad_Opcode },
    { "kord",       { MaskG, MaskVex, MaskE }, 0 },
  },

  /* PREFIX_VEX_0F46_L_1_M_1_W_0 */
  {
    { "kxnorw",     { MaskG, MaskVex, MaskE }, 0 },
    { Bad_Opcode },
    { "kxnorb",     { MaskG, MaskVex, MaskE }, 0 },
  },

  /* PREFIX_VEX_0F46_L_1_M_1_W_1 */
  {
    { "kxnorq",     { MaskG, MaskVex, MaskE }, 0 },
    { Bad_Opcode },
    { "kxnord",     { MaskG, MaskVex, MaskE }, 0 },
  },

  /* PREFIX_VEX_0F47_L_1_M_1_W_0 */
  {
    { "kxorw",      { MaskG, MaskVex, MaskE }, 0 },
    { Bad_Opcode },
    { "kxorb",      { MaskG, MaskVex, MaskE }, 0 },
  },

  /* PREFIX_VEX_0F47_L_1_M_1_W_1 */
  {
    { "kxorq",      { MaskG, MaskVex, MaskE }, 0 },
    { Bad_Opcode },
    { "kxord",      { MaskG, MaskVex, MaskE }, 0 },
  },

  /* PREFIX_VEX_0F4A_L_1_M_1_W_0 */
  {
    { "kaddw",          { MaskG, MaskVex, MaskE }, 0 },
    { Bad_Opcode },
    { "kaddb",          { MaskG, MaskVex, MaskE }, 0 },
  },

  /* PREFIX_VEX_0F4A_L_1_M_1_W_1 */
  {
    { "kaddq",          { MaskG, MaskVex, MaskE }, 0 },
    { Bad_Opcode },
    { "kaddd",          { MaskG, MaskVex, MaskE }, 0 },
  },

  /* PREFIX_VEX_0F4B_L_1_M_1_W_0 */
  {
    { "kunpckwd",   { MaskG, MaskVex, MaskE }, 0 },
    { Bad_Opcode },
    { "kunpckbw",   { MaskG, MaskVex, MaskE }, 0 },
  },

  /* PREFIX_VEX_0F4B_L_1_M_1_W_1 */
  {
    { "kunpckdq",   { MaskG, MaskVex, MaskE }, 0 },
  },

  /* PREFIX_VEX_0F51 */
  {
    { "%XEvsqrtpX",	{ XM, EXx, EXxEVexR }, 0 },
    { "%XEvsqrts%XS",	{ XMScalar, VexScalar, EXd, EXxEVexR }, 0 },
    { "%XEvsqrtpX",	{ XM, EXx, EXxEVexR }, 0 },
    { "%XEvsqrts%XD",	{ XMScalar, VexScalar, EXq, EXxEVexR }, 0 },
  },

  /* PREFIX_VEX_0F52 */
  {
    { "vrsqrtps",	{ XM, EXx }, 0 },
    { "vrsqrtss",	{ XMScalar, VexScalar, EXd }, 0 },
  },

  /* PREFIX_VEX_0F53 */
  {
    { "vrcpps",		{ XM, EXx }, 0 },
    { "vrcpss",		{ XMScalar, VexScalar, EXd }, 0 },
  },

  /* PREFIX_VEX_0F58 */
  {
    { "%XEvaddpX",	{ XM, Vex, EXx, EXxEVexR }, 0 },
    { "%XEvadds%XS",	{ XMScalar, VexScalar, EXd, EXxEVexR }, 0 },
    { "%XEvaddpX",	{ XM, Vex, EXx, EXxEVexR }, 0 },
    { "%XEvadds%XD",	{ XMScalar, VexScalar, EXq, EXxEVexR }, 0 },
  },

  /* PREFIX_VEX_0F59 */
  {
    { "%XEvmulpX",	{ XM, Vex, EXx, EXxEVexR }, 0 },
    { "%XEvmuls%XS",	{ XMScalar, VexScalar, EXd, EXxEVexR }, 0 },
    { "%XEvmulpX",	{ XM, Vex, EXx, EXxEVexR }, 0 },
    { "%XEvmuls%XD",	{ XMScalar, VexScalar, EXq, EXxEVexR }, 0 },
  },

  /* PREFIX_VEX_0F5A */
  {
    { "%XEvcvtp%XS2pd", { XM, EXEvexHalfBcstXmmq, EXxEVexS }, 0 },
    { "%XEvcvts%XS2sd", { XMScalar, VexScalar, EXd, EXxEVexS }, 0 },
    { "%XEvcvtp%XD2ps%XY", { XMxmmq, EXx, EXxEVexR }, 0 },
    { "%XEvcvts%XD2ss", { XMScalar, VexScalar, EXq, EXxEVexR }, 0 },
  },

  /* PREFIX_VEX_0F5B */
  {
    { "vcvtdq2ps",	{ XM, EXx }, 0 },
    { "vcvttps2dq",	{ XM, EXx }, 0 },
    { "vcvtps2dq",	{ XM, EXx }, 0 },
  },

  /* PREFIX_VEX_0F5C */
  {
    { "%XEvsubpX",	{ XM, Vex, EXx, EXxEVexR }, 0 },
    { "%XEvsubs%XS",	{ XMScalar, VexScalar, EXd, EXxEVexR }, 0 },
    { "%XEvsubpX",	{ XM, Vex, EXx, EXxEVexR }, 0 },
    { "%XEvsubs%XD",	{ XMScalar, VexScalar, EXq, EXxEVexR }, 0 },
  },

  /* PREFIX_VEX_0F5D */
  {
    { "%XEvminpX",	{ XM, Vex, EXx, EXxEVexS }, 0 },
    { "%XEvmins%XS",	{ XMScalar, VexScalar, EXd, EXxEVexS }, 0 },
    { "%XEvminpX",	{ XM, Vex, EXx, EXxEVexS }, 0 },
    { "%XEvmins%XD",	{ XMScalar, VexScalar, EXq, EXxEVexS }, 0 },
  },

  /* PREFIX_VEX_0F5E */
  {
    { "%XEvdivpX",	{ XM, Vex, EXx, EXxEVexR }, 0 },
    { "%XEvdivs%XS",	{ XMScalar, VexScalar, EXd, EXxEVexR }, 0 },
    { "%XEvdivpX",	{ XM, Vex, EXx, EXxEVexR }, 0 },
    { "%XEvdivs%XD",	{ XMScalar, VexScalar, EXq, EXxEVexR }, 0 },
  },

  /* PREFIX_VEX_0F5F */
  {
    { "%XEvmaxpX",	{ XM, Vex, EXx, EXxEVexS }, 0 },
    { "%XEvmaxs%XS",	{ XMScalar, VexScalar, EXd, EXxEVexS }, 0 },
    { "%XEvmaxpX",	{ XM, Vex, EXx, EXxEVexS }, 0 },
    { "%XEvmaxs%XD",	{ XMScalar, VexScalar, EXq, EXxEVexS }, 0 },
  },

  /* PREFIX_VEX_0F6F */
  {
    { Bad_Opcode },
    { "vmovdqu",	{ XM, EXx }, 0 },
    { "vmovdqa",	{ XM, EXx }, 0 },
  },

  /* PREFIX_VEX_0F70 */
  {
    { Bad_Opcode },
    { "vpshufhw",	{ XM, EXx, Ib }, 0 },
    { "vpshufd",	{ XM, EXx, Ib }, 0 },
    { "vpshuflw",	{ XM, EXx, Ib }, 0 },
  },

  /* PREFIX_VEX_0F7C */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vhaddpd",	{ XM, Vex, EXx }, 0 },
    { "vhaddps",	{ XM, Vex, EXx }, 0 },
  },

  /* PREFIX_VEX_0F7D */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vhsubpd",	{ XM, Vex, EXx }, 0 },
    { "vhsubps",	{ XM, Vex, EXx }, 0 },
  },

  /* PREFIX_VEX_0F7E */
  {
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F7E_P_1) },
    { VEX_LEN_TABLE (VEX_LEN_0F7E_P_2) },
  },

  /* PREFIX_VEX_0F7F */
  {
    { Bad_Opcode },
    { "vmovdqu",	{ EXxS, XM }, 0 },
    { "vmovdqa",	{ EXxS, XM }, 0 },
  },

  /* PREFIX_VEX_0F90_L_0_W_0 */
  {
    { "kmovw",		{ MaskG, MaskE }, 0 },
    { Bad_Opcode },
    { "kmovb",		{ MaskG, MaskBDE }, 0 },
  },

  /* PREFIX_VEX_0F90_L_0_W_1 */
  {
    { "kmovq",		{ MaskG, MaskE }, 0 },
    { Bad_Opcode },
    { "kmovd",		{ MaskG, MaskBDE }, 0 },
  },

  /* PREFIX_VEX_0F91_L_0_M_0_W_0 */
  {
    { "kmovw",		{ Ew, MaskG }, 0 },
    { Bad_Opcode },
    { "kmovb",		{ Eb, MaskG }, 0 },
  },

  /* PREFIX_VEX_0F91_L_0_M_0_W_1 */
  {
    { "kmovq",		{ Eq, MaskG }, 0 },
    { Bad_Opcode },
    { "kmovd",		{ Ed, MaskG }, 0 },
  },

  /* PREFIX_VEX_0F92_L_0_M_1_W_0 */
  {
    { "kmovw",		{ MaskG, Edq }, 0 },
    { Bad_Opcode },
    { "kmovb",		{ MaskG, Edq }, 0 },
    { "kmovd",		{ MaskG, Edq }, 0 },
  },

  /* PREFIX_VEX_0F92_L_0_M_1_W_1 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { "kmovK",		{ MaskG, Edq }, 0 },
  },

  /* PREFIX_VEX_0F93_L_0_M_1_W_0 */
  {
    { "kmovw",		{ Gdq, MaskE }, 0 },
    { Bad_Opcode },
    { "kmovb",		{ Gdq, MaskE }, 0 },
    { "kmovd",		{ Gdq, MaskE }, 0 },
  },

  /* PREFIX_VEX_0F93_L_0_M_1_W_1 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { "kmovK",		{ Gdq, MaskE }, 0 },
  },

  /* PREFIX_VEX_0F98_L_0_M_1_W_0 */
  {
    { "kortestw", { MaskG, MaskE }, 0 },
    { Bad_Opcode },
    { "kortestb", { MaskG, MaskE }, 0 },
  },

  /* PREFIX_VEX_0F98_L_0_M_1_W_1 */
  {
    { "kortestq", { MaskG, MaskE }, 0 },
    { Bad_Opcode },
    { "kortestd", { MaskG, MaskE }, 0 },
  },

  /* PREFIX_VEX_0F99_L_0_M_1_W_0 */
  {
    { "ktestw", { MaskG, MaskE }, 0 },
    { Bad_Opcode },
    { "ktestb", { MaskG, MaskE }, 0 },
  },

  /* PREFIX_VEX_0F99_L_0_M_1_W_1 */
  {
    { "ktestq", { MaskG, MaskE }, 0 },
    { Bad_Opcode },
    { "ktestd", { MaskG, MaskE }, 0 },
  },

  /* PREFIX_VEX_0FC2 */
  {
    { "vcmpps",		{ XM, Vex, EXx, CMP }, 0 },
    { "vcmpss",		{ XMScalar, VexScalar, EXd, CMP }, 0 },
    { "vcmppd",		{ XM, Vex, EXx, CMP }, 0 },
    { "vcmpsd",		{ XMScalar, VexScalar, EXq, CMP }, 0 },
  },

  /* PREFIX_VEX_0FD0 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { "vaddsubpd",	{ XM, Vex, EXx }, 0 },
    { "vaddsubps",	{ XM, Vex, EXx }, 0 },
  },

  /* PREFIX_VEX_0FE6 */
  {
    { Bad_Opcode },
    { "vcvtdq2pd",	{ XM, EXxmmq }, 0 },
    { "vcvttpd2dq%XY",	{ XMM, EXx }, 0 },
    { "vcvtpd2dq%XY",	{ XMM, EXx }, 0 },
  },

  /* PREFIX_VEX_0FF0 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { MOD_TABLE (MOD_VEX_0FF0_PREFIX_3) },
  },

  /* PREFIX_VEX_0F3849_X86_64_L_0_W_0_M_0 */
  {
    { "ldtilecfg", { M }, 0 },
    { Bad_Opcode },
    { "sttilecfg", { M }, 0 },
  },

  /* PREFIX_VEX_0F3849_X86_64_L_0_W_0_M_1 */
  {
    { REG_TABLE (REG_VEX_0F3849_X86_64_L_0_W_0_M_1_P_0) },
    { Bad_Opcode },
    { Bad_Opcode },
    { RM_TABLE (RM_VEX_0F3849_X86_64_L_0_W_0_M_1_P_3) },
  },

  /* PREFIX_VEX_0F384B_X86_64_L_0_W_0_M_0 */
  {
    { Bad_Opcode },
    { "tilestored",	{ MVexSIBMEM, TMM }, 0 },
    { "tileloaddt1",	{ TMM, MVexSIBMEM }, 0 },
    { "tileloadd",	{ TMM, MVexSIBMEM }, 0 },
  },

  /* PREFIX_VEX_0F3850_W_0 */
  {
    { "vpdpbuud",	{ XM, Vex, EXx }, 0 },
    { "vpdpbsud",	{ XM, Vex, EXx }, 0 },
    { "%XVvpdpbusd",	{ XM, Vex, EXx }, 0 },
    { "vpdpbssd",	{ XM, Vex, EXx }, 0 },
  },

  /* PREFIX_VEX_0F3851_W_0 */
  {
    { "vpdpbuuds",	{ XM, Vex, EXx }, 0 },
    { "vpdpbsuds",	{ XM, Vex, EXx }, 0 },
    { "%XVvpdpbusds",	{ XM, Vex, EXx }, 0 },
    { "vpdpbssds",	{ XM, Vex, EXx }, 0 },
  },
  /* PREFIX_VEX_0F385C_X86_64_M_1_L_0_W_0 */
  {
    { Bad_Opcode },
    { "tdpbf16ps", { TMM, EXtmm, VexTmm }, 0 },
    { Bad_Opcode },
    { "tdpfp16ps", { TMM, EXtmm, VexTmm }, 0 },
  },

  /* PREFIX_VEX_0F385E_X86_64_M_1_L_0_W_0 */
  {
    { "tdpbuud", {TMM, EXtmm, VexTmm }, 0 },
    { "tdpbsud", {TMM, EXtmm, VexTmm }, 0 },
    { "tdpbusd", {TMM, EXtmm, VexTmm }, 0 },
    { "tdpbssd", {TMM, EXtmm, VexTmm }, 0 },
  },

  /* PREFIX_VEX_0F386C_X86_64_M_1_L_0_W_0 */
  {
    { "tcmmrlfp16ps", { TMM, EXtmm, VexTmm }, 0 },
    { Bad_Opcode },
    { "tcmmimfp16ps", { TMM, EXtmm, VexTmm }, 0 },
  },

  /* PREFIX_VEX_0F3872 */
  {
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3872_P_1) },
  },

  /* PREFIX_VEX_0F38B0_W_0 */
  {
    { "vcvtneoph2ps", { XM, Mx }, 0 },
    { "vcvtneebf162ps", { XM, Mx }, 0 },
    { "vcvtneeph2ps", { XM, Mx }, 0 },
    { "vcvtneobf162ps", { XM, Mx }, 0 },
  },

  /* PREFIX_VEX_0F38B1_W_0 */
  {
    { Bad_Opcode },
    { "vbcstnebf162ps", { XM, Mw }, 0 },
    { "vbcstnesh2ps", { XM, Mw }, 0 },
  },
 
  /* PREFIX_VEX_0F38F5_L_0 */
  {
    { "bzhiS",		{ Gdq, Edq, VexGdq }, 0 },
    { "pextS",		{ Gdq, VexGdq, Edq }, 0 },
    { Bad_Opcode },
    { "pdepS",		{ Gdq, VexGdq, Edq }, 0 },
  },

  /* PREFIX_VEX_0F38F6_L_0 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { "mulxS",		{ Gdq, VexGdq, Edq }, 0 },
  },

  /* PREFIX_VEX_0F38F7_L_0 */
  {
    { "bextrS",		{ Gdq, Edq, VexGdq }, 0 },
    { "sarxS",		{ Gdq, Edq, VexGdq }, 0 },
    { "shlxS",		{ Gdq, Edq, VexGdq }, 0 },
    { "shrxS",		{ Gdq, Edq, VexGdq }, 0 },
  },

  /* PREFIX_VEX_0F3AF0_L_0 */
  {
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { "rorxS",		{ Gdq, Edq, Ib }, 0 },
  },

#include "i386-dis-evex-prefix.h"
};

static const struct dis386 x86_64_table[][2] = {
  /* X86_64_06 */
  {
    { "pushP", { es }, 0 },
  },

  /* X86_64_07 */
  {
    { "popP", { es }, 0 },
  },

  /* X86_64_0E */
  {
    { "pushP", { cs }, 0 },
  },

  /* X86_64_16 */
  {
    { "pushP", { ss }, 0 },
  },

  /* X86_64_17 */
  {
    { "popP", { ss }, 0 },
  },

  /* X86_64_1E */
  {
    { "pushP", { ds }, 0 },
  },

  /* X86_64_1F */
  {
    { "popP", { ds }, 0 },
  },

  /* X86_64_27 */
  {
    { "daa", { XX }, 0 },
  },

  /* X86_64_2F */
  {
    { "das", { XX }, 0 },
  },

  /* X86_64_37 */
  {
    { "aaa", { XX }, 0 },
  },

  /* X86_64_3F */
  {
    { "aas", { XX }, 0 },
  },

  /* X86_64_60 */
  {
    { "pushaP", { XX }, 0 },
  },

  /* X86_64_61 */
  {
    { "popaP", { XX }, 0 },
  },

  /* X86_64_62 */
  {
    { MOD_TABLE (MOD_62_32BIT) },
    { EVEX_TABLE (EVEX_0F) },
  },

  /* X86_64_63 */
  {
    { "arpl", { Ew, Gw }, 0 },
    { "movs", { Gv, { MOVSXD_Fixup, movsxd_mode } }, 0 },
  },

  /* X86_64_6D */
  {
    { "ins{R|}", { Yzr, indirDX }, 0 },
    { "ins{G|}", { Yzr, indirDX }, 0 },
  },

  /* X86_64_6F */
  {
    { "outs{R|}", { indirDXr, Xz }, 0 },
    { "outs{G|}", { indirDXr, Xz }, 0 },
  },

  /* X86_64_82 */
  {
    /* Opcode 0x82 is an alias of opcode 0x80 in 32-bit mode.  */
    { REG_TABLE (REG_80) },
  },

  /* X86_64_9A */
  {
    { "{l|}call{P|}", { Ap }, 0 },
  },

  /* X86_64_C2 */
  {
    { "retP",		{ Iw, BND }, 0 },
    { "ret@",		{ Iw, BND }, 0 },
  },

  /* X86_64_C3 */
  {
    { "retP",		{ BND }, 0 },
    { "ret@",		{ BND }, 0 },
  },

  /* X86_64_C4 */
  {
    { MOD_TABLE (MOD_C4_32BIT) },
    { VEX_C4_TABLE (VEX_0F) },
  },

  /* X86_64_C5 */
  {
    { MOD_TABLE (MOD_C5_32BIT) },
    { VEX_C5_TABLE (VEX_0F) },
  },

  /* X86_64_CE */
  {
    { "into", { XX }, 0 },
  },

  /* X86_64_D4 */
  {
    { "aam", { Ib }, 0 },
  },

  /* X86_64_D5 */
  {
    { "aad", { Ib }, 0 },
  },

  /* X86_64_E8 */
  {
    { "callP",		{ Jv, BND }, 0 },
    { "call@",		{ Jv, BND }, 0 }
  },

  /* X86_64_E9 */
  {
    { "jmpP",		{ Jv, BND }, 0 },
    { "jmp@",		{ Jv, BND }, 0 }
  },

  /* X86_64_EA */
  {
    { "{l|}jmp{P|}", { Ap }, 0 },
  },

  /* X86_64_0F00_REG_6 */
  {
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_0F00_REG_6_X86_64) },
  },

  /* X86_64_0F01_REG_0 */
  {
    { "sgdt{Q|Q}", { M }, 0 },
    { "sgdt", { M }, 0 },
  },

  /* X86_64_0F01_REG_0_MOD_3_RM_6_P_1 */
  {
    { Bad_Opcode },
    { "wrmsrlist",	{ Skip_MODRM }, 0 },
  },

  /* X86_64_0F01_REG_0_MOD_3_RM_6_P_3 */
  {
    { Bad_Opcode },
    { "rdmsrlist",	{ Skip_MODRM }, 0 },
  },

  /* X86_64_0F01_REG_1 */
  {
    { "sidt{Q|Q}", { M }, 0 },
    { "sidt", { M }, 0 },
  },

  /* X86_64_0F01_REG_1_RM_2_PREFIX_1 */
  {
    { Bad_Opcode },
    { "eretu",		{ Skip_MODRM }, 0 },
  },

  /* X86_64_0F01_REG_1_RM_2_PREFIX_3 */
  {
    { Bad_Opcode },
    { "erets",		{ Skip_MODRM }, 0 },
  },

  /* X86_64_0F01_REG_1_RM_5_PREFIX_2 */
  {
    { Bad_Opcode },
    { "seamret",	{ Skip_MODRM }, 0 },
  },

  /* X86_64_0F01_REG_1_RM_6_PREFIX_2 */
  {
    { Bad_Opcode },
    { "seamops",	{ Skip_MODRM }, 0 },
  },

  /* X86_64_0F01_REG_1_RM_7_PREFIX_2 */
  {
    { Bad_Opcode },
    { "seamcall",	{ Skip_MODRM }, 0 },
  },

  /* X86_64_0F01_REG_2 */
  {
    { "lgdt{Q|Q}", { M }, 0 },
    { "lgdt", { M }, 0 },
  },

  /* X86_64_0F01_REG_3 */
  {
    { "lidt{Q|Q}", { M }, 0 },
    { "lidt", { M }, 0 },
  },

  /* X86_64_0F01_REG_5_MOD_3_RM_4_PREFIX_1 */
  {
    { Bad_Opcode },
    { "uiret",	{ Skip_MODRM }, 0 },
  },

  /* X86_64_0F01_REG_5_MOD_3_RM_5_PREFIX_1 */
  {
    { Bad_Opcode },
    { "testui",	{ Skip_MODRM }, 0 },
  },

  /* X86_64_0F01_REG_5_MOD_3_RM_6_PREFIX_1 */
  {
    { Bad_Opcode },
    { "clui",	{ Skip_MODRM }, 0 },
  },

  /* X86_64_0F01_REG_5_MOD_3_RM_7_PREFIX_1 */
  {
    { Bad_Opcode },
    { "stui",	{ Skip_MODRM }, 0 },
  },

  /* X86_64_0F01_REG_7_MOD_3_RM_5_PREFIX_1 */
  {
    { Bad_Opcode },
    { "rmpquery", { Skip_MODRM }, 0 },
  },

  /* X86_64_0F01_REG_7_MOD_3_RM_6_PREFIX_1 */
  {
    { Bad_Opcode },
    { "rmpadjust",	{ Skip_MODRM }, 0 },
  },

  /* X86_64_0F01_REG_7_MOD_3_RM_6_PREFIX_3 */
  {
    { Bad_Opcode },
    { "rmpupdate",	{ Skip_MODRM }, 0 },
  },

  /* X86_64_0F01_REG_7_MOD_3_RM_7_PREFIX_1 */
  {
    { Bad_Opcode },
    { "psmash",	{ Skip_MODRM }, 0 },
  },

  /* X86_64_0F18_REG_6_MOD_0 */
  {
    { "nopQ",		{ Ev }, 0 },
    { PREFIX_TABLE (PREFIX_0F18_REG_6_MOD_0_X86_64) },
  },

  /* X86_64_0F18_REG_7_MOD_0 */
  {
    { "nopQ",		{ Ev }, 0 },
    { PREFIX_TABLE (PREFIX_0F18_REG_7_MOD_0_X86_64) },
  },

  {
    /* X86_64_0F24 */
    { "movZ",		{ Em, Td }, 0 },
  },

  {
    /* X86_64_0F26 */
    { "movZ",		{ Td, Em }, 0 },
  },

  /* X86_64_0FC7_REG_6_MOD_3_PREFIX_1 */
  {
    { Bad_Opcode },
    { "senduipi",	{ Eq }, 0 },
  },

  /* X86_64_VEX_0F3849 */
  {
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F3849_X86_64) },
  },

  /* X86_64_VEX_0F384B */
  {
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F384B_X86_64) },
  },

  /* X86_64_VEX_0F385C */
  {
    { Bad_Opcode },
    { MOD_TABLE (MOD_VEX_0F385C_X86_64) },
  },

  /* X86_64_VEX_0F385E */
  {
    { Bad_Opcode },
    { MOD_TABLE (MOD_VEX_0F385E_X86_64) },
  },

  /* X86_64_VEX_0F386C */
  {
    { Bad_Opcode },
    { MOD_TABLE (MOD_VEX_0F386C_X86_64) },
  },

  /* X86_64_VEX_0F38E0 */
  {
    { Bad_Opcode },
    { "cmpoxadd", { Mdq, Gdq, VexGdq }, PREFIX_DATA },
  },

  /* X86_64_VEX_0F38E1 */
  {
    { Bad_Opcode },
    { "cmpnoxadd", { Mdq, Gdq, VexGdq }, PREFIX_DATA },
  },

  /* X86_64_VEX_0F38E2 */
  {
    { Bad_Opcode },
    { "cmpbxadd", { Mdq, Gdq, VexGdq }, PREFIX_DATA },
  },

  /* X86_64_VEX_0F38E3 */
  {
    { Bad_Opcode },
    { "cmpnbxadd", { Mdq, Gdq, VexGdq }, PREFIX_DATA },
  },

  /* X86_64_VEX_0F38E4 */
  {
    { Bad_Opcode },
    { "cmpzxadd", { Mdq, Gdq, VexGdq }, PREFIX_DATA },
  },

  /* X86_64_VEX_0F38E5 */
  {
    { Bad_Opcode },
    { "cmpnzxadd", { Mdq, Gdq, VexGdq }, PREFIX_DATA },
  },

  /* X86_64_VEX_0F38E6 */
  {
    { Bad_Opcode },
    { "cmpbexadd", { Mdq, Gdq, VexGdq }, PREFIX_DATA },
  },

  /* X86_64_VEX_0F38E7 */
  {
    { Bad_Opcode },
    { "cmpnbexadd", { Mdq, Gdq, VexGdq }, PREFIX_DATA },
  },

  /* X86_64_VEX_0F38E8 */
  {
    { Bad_Opcode },
    { "cmpsxadd", { Mdq, Gdq, VexGdq }, PREFIX_DATA },
  },

  /* X86_64_VEX_0F38E9 */
  {
    { Bad_Opcode },
    { "cmpnsxadd", { Mdq, Gdq, VexGdq }, PREFIX_DATA },
  },

  /* X86_64_VEX_0F38EA */
  {
    { Bad_Opcode },
    { "cmppxadd", { Mdq, Gdq, VexGdq }, PREFIX_DATA },
  },

  /* X86_64_VEX_0F38EB */
  {
    { Bad_Opcode },
    { "cmpnpxadd", { Mdq, Gdq, VexGdq }, PREFIX_DATA },
  },

  /* X86_64_VEX_0F38EC */
  {
    { Bad_Opcode },
    { "cmplxadd", { Mdq, Gdq, VexGdq }, PREFIX_DATA },
  },

  /* X86_64_VEX_0F38ED */
  {
    { Bad_Opcode },
    { "cmpnlxadd", { Mdq, Gdq, VexGdq }, PREFIX_DATA },
  },

  /* X86_64_VEX_0F38EE */
  {
    { Bad_Opcode },
    { "cmplexadd", { Mdq, Gdq, VexGdq }, PREFIX_DATA },
  },

  /* X86_64_VEX_0F38EF */
  {
    { Bad_Opcode },
    { "cmpnlexadd", { Mdq, Gdq, VexGdq }, PREFIX_DATA },
  },
};

static const struct dis386 three_byte_table[][256] = {

  /* THREE_BYTE_0F38 */
  {
    /* 00 */
    { "pshufb",		{ MX, EM }, PREFIX_OPCODE },
    { "phaddw",		{ MX, EM }, PREFIX_OPCODE },
    { "phaddd",		{ MX, EM }, PREFIX_OPCODE },
    { "phaddsw",	{ MX, EM }, PREFIX_OPCODE },
    { "pmaddubsw",	{ MX, EM }, PREFIX_OPCODE },
    { "phsubw",		{ MX, EM }, PREFIX_OPCODE },
    { "phsubd",		{ MX, EM }, PREFIX_OPCODE },
    { "phsubsw",	{ MX, EM }, PREFIX_OPCODE },
    /* 08 */
    { "psignb",		{ MX, EM }, PREFIX_OPCODE },
    { "psignw",		{ MX, EM }, PREFIX_OPCODE },
    { "psignd",		{ MX, EM }, PREFIX_OPCODE },
    { "pmulhrsw",	{ MX, EM }, PREFIX_OPCODE },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 10 */
    { "pblendvb", { XM, EXx, XMM0 }, PREFIX_DATA },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { "blendvps", { XM, EXx, XMM0 }, PREFIX_DATA },
    { "blendvpd", { XM, EXx, XMM0 }, PREFIX_DATA },
    { Bad_Opcode },
    { "ptest",  { XM, EXx }, PREFIX_DATA },
    /* 18 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { "pabsb",		{ MX, EM }, PREFIX_OPCODE },
    { "pabsw",		{ MX, EM }, PREFIX_OPCODE },
    { "pabsd",		{ MX, EM }, PREFIX_OPCODE },
    { Bad_Opcode },
    /* 20 */
    { "pmovsxbw", { XM, EXq }, PREFIX_DATA },
    { "pmovsxbd", { XM, EXd }, PREFIX_DATA },
    { "pmovsxbq", { XM, EXw }, PREFIX_DATA },
    { "pmovsxwd", { XM, EXq }, PREFIX_DATA },
    { "pmovsxwq", { XM, EXd }, PREFIX_DATA },
    { "pmovsxdq", { XM, EXq }, PREFIX_DATA },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 28 */
    { "pmuldq", { XM, EXx }, PREFIX_DATA },
    { "pcmpeqq", { XM, EXx }, PREFIX_DATA },
    { MOD_TABLE (MOD_0F382A) },
    { "packusdw", { XM, EXx }, PREFIX_DATA },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 30 */
    { "pmovzxbw", { XM, EXq }, PREFIX_DATA },
    { "pmovzxbd", { XM, EXd }, PREFIX_DATA },
    { "pmovzxbq", { XM, EXw }, PREFIX_DATA },
    { "pmovzxwd", { XM, EXq }, PREFIX_DATA },
    { "pmovzxwq", { XM, EXd }, PREFIX_DATA },
    { "pmovzxdq", { XM, EXq }, PREFIX_DATA },
    { Bad_Opcode },
    { "pcmpgtq", { XM, EXx }, PREFIX_DATA },
    /* 38 */
    { "pminsb",	{ XM, EXx }, PREFIX_DATA },
    { "pminsd",	{ XM, EXx }, PREFIX_DATA },
    { "pminuw",	{ XM, EXx }, PREFIX_DATA },
    { "pminud",	{ XM, EXx }, PREFIX_DATA },
    { "pmaxsb",	{ XM, EXx }, PREFIX_DATA },
    { "pmaxsd",	{ XM, EXx }, PREFIX_DATA },
    { "pmaxuw", { XM, EXx }, PREFIX_DATA },
    { "pmaxud", { XM, EXx }, PREFIX_DATA },
    /* 40 */
    { "pmulld", { XM, EXx }, PREFIX_DATA },
    { "phminposuw", { XM, EXx }, PREFIX_DATA },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 48 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 50 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 58 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 60 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 68 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 70 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 78 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 80 */
    { "invept",	{ Gm, Mo }, PREFIX_DATA },
    { "invvpid", { Gm, Mo }, PREFIX_DATA },
    { "invpcid", { Gm, M }, PREFIX_DATA },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 88 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 90 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 98 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* a0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* a8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* b0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* b8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* c0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* c8 */
    { "sha1nexte", { XM, EXxmm }, PREFIX_OPCODE },
    { "sha1msg1", { XM, EXxmm }, PREFIX_OPCODE },
    { "sha1msg2", { XM, EXxmm }, PREFIX_OPCODE },
    { "sha256rnds2", { XM, EXxmm, XMM0 }, PREFIX_OPCODE },
    { "sha256msg1", { XM, EXxmm }, PREFIX_OPCODE },
    { "sha256msg2", { XM, EXxmm }, PREFIX_OPCODE },
    { Bad_Opcode },
    { "gf2p8mulb", { XM, EXxmm }, PREFIX_DATA },
    /* d0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* d8 */
    { PREFIX_TABLE (PREFIX_0F38D8) },
    { Bad_Opcode },
    { Bad_Opcode },
    { "aesimc", { XM, EXx }, PREFIX_DATA },
    { PREFIX_TABLE (PREFIX_0F38DC) },
    { PREFIX_TABLE (PREFIX_0F38DD) },
    { PREFIX_TABLE (PREFIX_0F38DE) },
    { PREFIX_TABLE (PREFIX_0F38DF) },
    /* e0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* e8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* f0 */
    { PREFIX_TABLE (PREFIX_0F38F0) },
    { PREFIX_TABLE (PREFIX_0F38F1) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { MOD_TABLE (MOD_0F38F5) },
    { PREFIX_TABLE (PREFIX_0F38F6) },
    { Bad_Opcode },
    /* f8 */
    { PREFIX_TABLE (PREFIX_0F38F8) },
    { MOD_TABLE (MOD_0F38F9) },
    { PREFIX_TABLE (PREFIX_0F38FA) },
    { PREFIX_TABLE (PREFIX_0F38FB) },
    { PREFIX_TABLE (PREFIX_0F38FC) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
  },
  /* THREE_BYTE_0F3A */
  {
    /* 00 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 08 */
    { "roundps", { XM, EXx, Ib }, PREFIX_DATA },
    { "roundpd", { XM, EXx, Ib }, PREFIX_DATA },
    { "roundss", { XM, EXd, Ib }, PREFIX_DATA },
    { "roundsd", { XM, EXq, Ib }, PREFIX_DATA },
    { "blendps", { XM, EXx, Ib }, PREFIX_DATA },
    { "blendpd", { XM, EXx, Ib }, PREFIX_DATA },
    { "pblendw", { XM, EXx, Ib }, PREFIX_DATA },
    { "palignr",	{ MX, EM, Ib }, PREFIX_OPCODE },
    /* 10 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { "pextrb",	{ Edb, XM, Ib }, PREFIX_DATA },
    { "pextrw",	{ Edw, XM, Ib }, PREFIX_DATA },
    { "pextrK",	{ Edq, XM, Ib }, PREFIX_DATA },
    { "extractps", { Ed, XM, Ib }, PREFIX_DATA },
    /* 18 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 20 */
    { "pinsrb",	{ XM, Edb, Ib }, PREFIX_DATA },
    { "insertps", { XM, EXd, Ib }, PREFIX_DATA },
    { "pinsrK",	{ XM, Edq, Ib }, PREFIX_DATA },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 28 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 30 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 38 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 40 */
    { "dpps",	{ XM, EXx, Ib }, PREFIX_DATA },
    { "dppd",	{ XM, EXx, Ib }, PREFIX_DATA },
    { "mpsadbw", { XM, EXx, Ib }, PREFIX_DATA },
    { Bad_Opcode },
    { "pclmulqdq", { XM, EXx, PCLMUL }, PREFIX_DATA },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 48 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 50 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 58 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 60 */
    { "pcmpestrm!%LQ", { XM, EXx, Ib }, PREFIX_DATA },
    { "pcmpestri!%LQ", { XM, EXx, Ib }, PREFIX_DATA },
    { "pcmpistrm", { XM, EXx, Ib }, PREFIX_DATA },
    { "pcmpistri", { XM, EXx, Ib }, PREFIX_DATA },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 68 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 70 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 78 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 80 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 88 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 90 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 98 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* a0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* a8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* b0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* b8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* c0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* c8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { "sha1rnds4", { XM, EXxmm, Ib }, PREFIX_OPCODE },
    { Bad_Opcode },
    { "gf2p8affineqb", { XM, EXxmm, Ib }, PREFIX_DATA },
    { "gf2p8affineinvqb", { XM, EXxmm, Ib }, PREFIX_DATA },
    /* d0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* d8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { "aeskeygenassist", { XM, EXx, Ib }, PREFIX_DATA },
    /* e0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* e8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* f0 */
    { PREFIX_TABLE (PREFIX_0F3A0F) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* f8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
  },
};

static const struct dis386 xop_table[][256] = {
  /* XOP_08 */
  {
    /* 00 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 08 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 10 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 18 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 20 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 28 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 30 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 38 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 40 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 48 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 50 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 58 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 60 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 68 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 70 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 78 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 80 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_08_85) },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_08_86) },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_08_87) },
    /* 88 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_08_8E) },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_08_8F) },
    /* 90 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_08_95) },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_08_96) },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_08_97) },
    /* 98 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_08_9E) },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_08_9F) },
    /* a0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { "vpcmov", 	{ XM, Vex, EXx, XMVexI4 }, 0 },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_08_A3) },
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_08_A6) },
    { Bad_Opcode },
    /* a8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* b0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_08_B6) },
    { Bad_Opcode },
    /* b8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* c0 */
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_08_C0) },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_08_C1) },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_08_C2) },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_08_C3) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* c8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_08_CC) },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_08_CD) },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_08_CE) },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_08_CF) },
    /* d0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* d8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* e0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* e8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_08_EC) },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_08_ED) },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_08_EE) },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_08_EF) },
    /* f0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* f8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
  },
  /* XOP_09 */
  {
    /* 00 */
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_09_01) },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_09_02) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 08 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 10 */
    { Bad_Opcode },
    { Bad_Opcode },
    { MOD_TABLE (MOD_XOP_09_12) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 18 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 20 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 28 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 30 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 38 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 40 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 48 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 50 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 58 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 60 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 68 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 70 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 78 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 80 */
    { VEX_W_TABLE (VEX_W_0FXOP_09_80) },
    { VEX_W_TABLE (VEX_W_0FXOP_09_81) },
    { VEX_W_TABLE (VEX_W_0FXOP_09_82) },
    { VEX_W_TABLE (VEX_W_0FXOP_09_83) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 88 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 90 */
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_09_90) },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_09_91) },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_09_92) },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_09_93) },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_09_94) },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_09_95) },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_09_96) },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_09_97) },
    /* 98 */
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_09_98) },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_09_99) },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_09_9A) },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_09_9B) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* a0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* a8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* b0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* b8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* c0 */
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_09_C1) },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_09_C2) },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_09_C3) },
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_09_C6) },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_09_C7) },
    /* c8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_09_CB) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* d0 */
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_09_D1) },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_09_D2) },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_09_D3) },
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_09_D6) },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_09_D7) },
    /* d8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_09_DB) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* e0 */
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_09_E1) },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_09_E2) },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_09_E3) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* e8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* f0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* f8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
  },
  /* XOP_0A */
  {
    /* 00 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 08 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 10 */
    { "bextrS",	{ Gdq, Edq, Id }, 0 },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_0A_12) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 18 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 20 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 28 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 30 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 38 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 40 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 48 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 50 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 58 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 60 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 68 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 70 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 78 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 80 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 88 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 90 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 98 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* a0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* a8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* b0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* b8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* c0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* c8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* d0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* d8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* e0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* e8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* f0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* f8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
  },
};

static const struct dis386 vex_table[][256] = {
  /* VEX_0F */
  {
    /* 00 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 08 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 10 */
    { PREFIX_TABLE (PREFIX_VEX_0F10) },
    { PREFIX_TABLE (PREFIX_VEX_0F11) },
    { PREFIX_TABLE (PREFIX_VEX_0F12) },
    { MOD_TABLE (MOD_VEX_0F13) },
    { "vunpcklpX",	{ XM, Vex, EXx }, PREFIX_OPCODE },
    { "vunpckhpX",	{ XM, Vex, EXx }, PREFIX_OPCODE },
    { PREFIX_TABLE (PREFIX_VEX_0F16) },
    { MOD_TABLE (MOD_VEX_0F17) },
    /* 18 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 20 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 28 */
    { "vmovapX",	{ XM, EXx }, PREFIX_OPCODE },
    { "vmovapX",	{ EXxS, XM }, PREFIX_OPCODE },
    { PREFIX_TABLE (PREFIX_VEX_0F2A) },
    { MOD_TABLE (MOD_VEX_0F2B) },
    { PREFIX_TABLE (PREFIX_VEX_0F2C) },
    { PREFIX_TABLE (PREFIX_VEX_0F2D) },
    { PREFIX_TABLE (PREFIX_VEX_0F2E) },
    { PREFIX_TABLE (PREFIX_VEX_0F2F) },
    /* 30 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 38 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 40 */
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F41) },
    { VEX_LEN_TABLE (VEX_LEN_0F42) },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F44) },
    { VEX_LEN_TABLE (VEX_LEN_0F45) },
    { VEX_LEN_TABLE (VEX_LEN_0F46) },
    { VEX_LEN_TABLE (VEX_LEN_0F47) },
    /* 48 */
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F4A) },
    { VEX_LEN_TABLE (VEX_LEN_0F4B) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 50 */
    { MOD_TABLE (MOD_VEX_0F50) },
    { PREFIX_TABLE (PREFIX_VEX_0F51) },
    { PREFIX_TABLE (PREFIX_VEX_0F52) },
    { PREFIX_TABLE (PREFIX_VEX_0F53) },
    { "vandpX",		{ XM, Vex, EXx }, PREFIX_OPCODE },
    { "vandnpX",	{ XM, Vex, EXx }, PREFIX_OPCODE },
    { "vorpX",		{ XM, Vex, EXx }, PREFIX_OPCODE },
    { "vxorpX",		{ XM, Vex, EXx }, PREFIX_OPCODE },
    /* 58 */
    { PREFIX_TABLE (PREFIX_VEX_0F58) },
    { PREFIX_TABLE (PREFIX_VEX_0F59) },
    { PREFIX_TABLE (PREFIX_VEX_0F5A) },
    { PREFIX_TABLE (PREFIX_VEX_0F5B) },
    { PREFIX_TABLE (PREFIX_VEX_0F5C) },
    { PREFIX_TABLE (PREFIX_VEX_0F5D) },
    { PREFIX_TABLE (PREFIX_VEX_0F5E) },
    { PREFIX_TABLE (PREFIX_VEX_0F5F) },
    /* 60 */
    { "vpunpcklbw",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpunpcklwd",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpunpckldq",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpacksswb",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpcmpgtb",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpcmpgtw",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpcmpgtd",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpackuswb",	{ XM, Vex, EXx }, PREFIX_DATA },
    /* 68 */
    { "vpunpckhbw",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpunpckhwd",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpunpckhdq",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpackssdw",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpunpcklqdq",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpunpckhqdq",	{ XM, Vex, EXx }, PREFIX_DATA },
    { VEX_LEN_TABLE (VEX_LEN_0F6E) },
    { PREFIX_TABLE (PREFIX_VEX_0F6F) },
    /* 70 */
    { PREFIX_TABLE (PREFIX_VEX_0F70) },
    { MOD_TABLE (MOD_VEX_0F71) },
    { MOD_TABLE (MOD_VEX_0F72) },
    { MOD_TABLE (MOD_VEX_0F73) },
    { "vpcmpeqb",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpcmpeqw",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpcmpeqd",	{ XM, Vex, EXx }, PREFIX_DATA },
    { VEX_LEN_TABLE (VEX_LEN_0F77) },
    /* 78 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_VEX_0F7C) },
    { PREFIX_TABLE (PREFIX_VEX_0F7D) },
    { PREFIX_TABLE (PREFIX_VEX_0F7E) },
    { PREFIX_TABLE (PREFIX_VEX_0F7F) },
    /* 80 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 88 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 90 */
    { VEX_LEN_TABLE (VEX_LEN_0F90) },
    { VEX_LEN_TABLE (VEX_LEN_0F91) },
    { VEX_LEN_TABLE (VEX_LEN_0F92) },
    { VEX_LEN_TABLE (VEX_LEN_0F93) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 98 */
    { VEX_LEN_TABLE (VEX_LEN_0F98) },
    { VEX_LEN_TABLE (VEX_LEN_0F99) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* a0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* a8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { REG_TABLE (REG_VEX_0FAE) },
    { Bad_Opcode },
    /* b0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* b8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* c0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_VEX_0FC2) },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0FC4) },
    { VEX_LEN_TABLE (VEX_LEN_0FC5) },
    { "vshufpX",	{ XM, Vex, EXx, Ib }, PREFIX_OPCODE },
    { Bad_Opcode },
    /* c8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* d0 */
    { PREFIX_TABLE (PREFIX_VEX_0FD0) },
    { "vpsrlw",		{ XM, Vex, EXxmm }, PREFIX_DATA },
    { "vpsrld",		{ XM, Vex, EXxmm }, PREFIX_DATA },
    { "vpsrlq",		{ XM, Vex, EXxmm }, PREFIX_DATA },
    { "vpaddq",		{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpmullw",	{ XM, Vex, EXx }, PREFIX_DATA },
    { VEX_LEN_TABLE (VEX_LEN_0FD6) },
    { MOD_TABLE (MOD_VEX_0FD7) },
    /* d8 */
    { "vpsubusb",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpsubusw",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpminub",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpand",		{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpaddusb",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpaddusw",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpmaxub",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpandn",		{ XM, Vex, EXx }, PREFIX_DATA },
    /* e0 */
    { "vpavgb",		{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpsraw",		{ XM, Vex, EXxmm }, PREFIX_DATA },
    { "vpsrad",		{ XM, Vex, EXxmm }, PREFIX_DATA },
    { "vpavgw",		{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpmulhuw",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpmulhw",	{ XM, Vex, EXx }, PREFIX_DATA },
    { PREFIX_TABLE (PREFIX_VEX_0FE6) },
    { MOD_TABLE (MOD_VEX_0FE7) },
    /* e8 */
    { "vpsubsb",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpsubsw",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpminsw",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpor",		{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpaddsb",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpaddsw",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpmaxsw",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpxor",		{ XM, Vex, EXx }, PREFIX_DATA },
    /* f0 */
    { PREFIX_TABLE (PREFIX_VEX_0FF0) },
    { "vpsllw",		{ XM, Vex, EXxmm }, PREFIX_DATA },
    { "vpslld",		{ XM, Vex, EXxmm }, PREFIX_DATA },
    { "vpsllq",		{ XM, Vex, EXxmm }, PREFIX_DATA },
    { "vpmuludq",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpmaddwd",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpsadbw",	{ XM, Vex, EXx }, PREFIX_DATA },
    { VEX_LEN_TABLE (VEX_LEN_0FF7) },
    /* f8 */
    { "vpsubb",		{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpsubw",		{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpsubd",		{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpsubq",		{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpaddb",		{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpaddw",		{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpaddd",		{ XM, Vex, EXx }, PREFIX_DATA },
    { Bad_Opcode },
  },
  /* VEX_0F38 */
  {
    /* 00 */
    { "vpshufb",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vphaddw",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vphaddd",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vphaddsw",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpmaddubsw",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vphsubw",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vphsubd",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vphsubsw",	{ XM, Vex, EXx }, PREFIX_DATA },
    /* 08 */
    { "vpsignb",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpsignw",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpsignd",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpmulhrsw",	{ XM, Vex, EXx }, PREFIX_DATA },
    { VEX_W_TABLE (VEX_W_0F380C) },
    { VEX_W_TABLE (VEX_W_0F380D) },
    { VEX_W_TABLE (VEX_W_0F380E) },
    { VEX_W_TABLE (VEX_W_0F380F) },
    /* 10 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3813) },
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F3816) },
    { "vptest",		{ XM, EXx }, PREFIX_DATA },
    /* 18 */
    { VEX_W_TABLE (VEX_W_0F3818) },
    { VEX_LEN_TABLE (VEX_LEN_0F3819) },
    { MOD_TABLE (MOD_VEX_0F381A) },
    { Bad_Opcode },
    { "vpabsb",		{ XM, EXx }, PREFIX_DATA },
    { "vpabsw",		{ XM, EXx }, PREFIX_DATA },
    { "vpabsd",		{ XM, EXx }, PREFIX_DATA },
    { Bad_Opcode },
    /* 20 */
    { "vpmovsxbw",	{ XM, EXxmmq }, PREFIX_DATA },
    { "vpmovsxbd",	{ XM, EXxmmqd }, PREFIX_DATA },
    { "vpmovsxbq",	{ XM, EXxmmdw }, PREFIX_DATA },
    { "vpmovsxwd",	{ XM, EXxmmq }, PREFIX_DATA },
    { "vpmovsxwq",	{ XM, EXxmmqd }, PREFIX_DATA },
    { "vpmovsxdq",	{ XM, EXxmmq }, PREFIX_DATA },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 28 */
    { "vpmuldq",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpcmpeqq",	{ XM, Vex, EXx }, PREFIX_DATA },
    { MOD_TABLE (MOD_VEX_0F382A) },
    { "vpackusdw",	{ XM, Vex, EXx }, PREFIX_DATA },
    { MOD_TABLE (MOD_VEX_0F382C) },
    { MOD_TABLE (MOD_VEX_0F382D) },
    { MOD_TABLE (MOD_VEX_0F382E) },
    { MOD_TABLE (MOD_VEX_0F382F) },
    /* 30 */
    { "vpmovzxbw",	{ XM, EXxmmq }, PREFIX_DATA },
    { "vpmovzxbd",	{ XM, EXxmmqd }, PREFIX_DATA },
    { "vpmovzxbq",	{ XM, EXxmmdw }, PREFIX_DATA },
    { "vpmovzxwd",	{ XM, EXxmmq }, PREFIX_DATA },
    { "vpmovzxwq",	{ XM, EXxmmqd }, PREFIX_DATA },
    { "vpmovzxdq",	{ XM, EXxmmq }, PREFIX_DATA },
    { VEX_LEN_TABLE (VEX_LEN_0F3836) },
    { "vpcmpgtq",	{ XM, Vex, EXx }, PREFIX_DATA },
    /* 38 */
    { "vpminsb",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpminsd",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpminuw",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpminud",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpmaxsb",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpmaxsd",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpmaxuw",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vpmaxud",	{ XM, Vex, EXx }, PREFIX_DATA },
    /* 40 */
    { "vpmulld",	{ XM, Vex, EXx }, PREFIX_DATA },
    { VEX_LEN_TABLE (VEX_LEN_0F3841) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { "vpsrlv%DQ", { XM, Vex, EXx }, PREFIX_DATA },
    { VEX_W_TABLE (VEX_W_0F3846) },
    { "vpsllv%DQ", { XM, Vex, EXx }, PREFIX_DATA },
    /* 48 */
    { Bad_Opcode },
    { X86_64_TABLE (X86_64_VEX_0F3849) },
    { Bad_Opcode },
    { X86_64_TABLE (X86_64_VEX_0F384B) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 50 */
    { VEX_W_TABLE (VEX_W_0F3850) },
    { VEX_W_TABLE (VEX_W_0F3851) },
    { VEX_W_TABLE (VEX_W_0F3852) },
    { VEX_W_TABLE (VEX_W_0F3853) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 58 */
    { VEX_W_TABLE (VEX_W_0F3858) },
    { VEX_W_TABLE (VEX_W_0F3859) },
    { MOD_TABLE (MOD_VEX_0F385A) },
    { Bad_Opcode },
    { X86_64_TABLE (X86_64_VEX_0F385C) },
    { Bad_Opcode },
    { X86_64_TABLE (X86_64_VEX_0F385E) },
    { Bad_Opcode },
    /* 60 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 68 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { X86_64_TABLE (X86_64_VEX_0F386C) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 70 */
    { Bad_Opcode },
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_VEX_0F3872) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 78 */
    { VEX_W_TABLE (VEX_W_0F3878) },
    { VEX_W_TABLE (VEX_W_0F3879) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 80 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 88 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { MOD_TABLE (MOD_VEX_0F388C) },
    { Bad_Opcode },
    { MOD_TABLE (MOD_VEX_0F388E) },
    { Bad_Opcode },
    /* 90 */
    { "vpgatherd%DQ", { XM, MVexVSIBDWpX, VexGatherD }, PREFIX_DATA },
    { "vpgatherq%DQ", { XMGatherQ, MVexVSIBQWpX, VexGatherQ }, PREFIX_DATA },
    { "vgatherdp%XW", { XM, MVexVSIBDWpX, VexGatherD }, PREFIX_DATA },
    { "vgatherqp%XW", { XMGatherQ, MVexVSIBQWpX, VexGatherQ }, PREFIX_DATA },
    { Bad_Opcode },
    { Bad_Opcode },
    { "vfmaddsub132p%XW", { XM, Vex, EXx }, PREFIX_DATA },
    { "vfmsubadd132p%XW", { XM, Vex, EXx }, PREFIX_DATA },
    /* 98 */
    { "vfmadd132p%XW", { XM, Vex, EXx }, PREFIX_DATA },
    { "vfmadd132s%XW", { XMScalar, VexScalar, EXdq }, PREFIX_DATA },
    { "vfmsub132p%XW", { XM, Vex, EXx }, PREFIX_DATA },
    { "vfmsub132s%XW", { XMScalar, VexScalar, EXdq }, PREFIX_DATA },
    { "vfnmadd132p%XW", { XM, Vex, EXx }, PREFIX_DATA },
    { "vfnmadd132s%XW", { XMScalar, VexScalar, EXdq }, PREFIX_DATA },
    { "vfnmsub132p%XW", { XM, Vex, EXx }, PREFIX_DATA },
    { "vfnmsub132s%XW", { XMScalar, VexScalar, EXdq }, PREFIX_DATA },
    /* a0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { "vfmaddsub213p%XW", { XM, Vex, EXx }, PREFIX_DATA },
    { "vfmsubadd213p%XW", { XM, Vex, EXx }, PREFIX_DATA },
    /* a8 */
    { "vfmadd213p%XW", { XM, Vex, EXx }, PREFIX_DATA },
    { "vfmadd213s%XW", { XMScalar, VexScalar, EXdq }, PREFIX_DATA },
    { "vfmsub213p%XW", { XM, Vex, EXx }, PREFIX_DATA },
    { "vfmsub213s%XW", { XMScalar, VexScalar, EXdq }, PREFIX_DATA },
    { "vfnmadd213p%XW", { XM, Vex, EXx }, PREFIX_DATA },
    { "vfnmadd213s%XW", { XMScalar, VexScalar, EXdq }, PREFIX_DATA },
    { "vfnmsub213p%XW", { XM, Vex, EXx }, PREFIX_DATA },
    { "vfnmsub213s%XW", { XMScalar, VexScalar, EXdq }, PREFIX_DATA },
    /* b0 */
    { VEX_W_TABLE (VEX_W_0F38B0) },
    { VEX_W_TABLE (VEX_W_0F38B1) },
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F38B4) },
    { VEX_W_TABLE (VEX_W_0F38B5) },
    { "vfmaddsub231p%XW", { XM, Vex, EXx }, PREFIX_DATA },
    { "vfmsubadd231p%XW", { XM, Vex, EXx }, PREFIX_DATA },
    /* b8 */
    { "vfmadd231p%XW", { XM, Vex, EXx }, PREFIX_DATA },
    { "vfmadd231s%XW", { XMScalar, VexScalar, EXdq }, PREFIX_DATA },
    { "vfmsub231p%XW", { XM, Vex, EXx }, PREFIX_DATA },
    { "vfmsub231s%XW", { XMScalar, VexScalar, EXdq }, PREFIX_DATA },
    { "vfnmadd231p%XW", { XM, Vex, EXx }, PREFIX_DATA },
    { "vfnmadd231s%XW", { XMScalar, VexScalar, EXdq }, PREFIX_DATA },
    { "vfnmsub231p%XW", { XM, Vex, EXx }, PREFIX_DATA },
    { "vfnmsub231s%XW", { XMScalar, VexScalar, EXdq }, PREFIX_DATA },
    /* c0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* c8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F38CF) },
    /* d0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* d8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F38DB) },
    { "vaesenc",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vaesenclast",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vaesdec",	{ XM, Vex, EXx }, PREFIX_DATA },
    { "vaesdeclast",	{ XM, Vex, EXx }, PREFIX_DATA },
    /* e0 */
    { X86_64_TABLE (X86_64_VEX_0F38E0) },
    { X86_64_TABLE (X86_64_VEX_0F38E1) },
    { X86_64_TABLE (X86_64_VEX_0F38E2) },
    { X86_64_TABLE (X86_64_VEX_0F38E3) },
    { X86_64_TABLE (X86_64_VEX_0F38E4) },
    { X86_64_TABLE (X86_64_VEX_0F38E5) },
    { X86_64_TABLE (X86_64_VEX_0F38E6) },
    { X86_64_TABLE (X86_64_VEX_0F38E7) },
    /* e8 */
    { X86_64_TABLE (X86_64_VEX_0F38E8) },
    { X86_64_TABLE (X86_64_VEX_0F38E9) },
    { X86_64_TABLE (X86_64_VEX_0F38EA) },
    { X86_64_TABLE (X86_64_VEX_0F38EB) },
    { X86_64_TABLE (X86_64_VEX_0F38EC) },
    { X86_64_TABLE (X86_64_VEX_0F38ED) },
    { X86_64_TABLE (X86_64_VEX_0F38EE) },
    { X86_64_TABLE (X86_64_VEX_0F38EF) },
    /* f0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F38F2) },
    { VEX_LEN_TABLE (VEX_LEN_0F38F3) },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F38F5) },
    { VEX_LEN_TABLE (VEX_LEN_0F38F6) },
    { VEX_LEN_TABLE (VEX_LEN_0F38F7) },
    /* f8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
  },
  /* VEX_0F3A */
  {
    /* 00 */
    { VEX_LEN_TABLE (VEX_LEN_0F3A00) },
    { VEX_LEN_TABLE (VEX_LEN_0F3A01) },
    { VEX_W_TABLE (VEX_W_0F3A02) },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3A04) },
    { VEX_W_TABLE (VEX_W_0F3A05) },
    { VEX_LEN_TABLE (VEX_LEN_0F3A06) },
    { Bad_Opcode },
    /* 08 */
    { "vroundps",	{ XM, EXx, Ib }, PREFIX_DATA },
    { "vroundpd",	{ XM, EXx, Ib }, PREFIX_DATA },
    { "vroundss",	{ XMScalar, VexScalar, EXd, Ib }, PREFIX_DATA },
    { "vroundsd",	{ XMScalar, VexScalar, EXq, Ib }, PREFIX_DATA },
    { "vblendps",	{ XM, Vex, EXx, Ib }, PREFIX_DATA },
    { "vblendpd",	{ XM, Vex, EXx, Ib }, PREFIX_DATA },
    { "vpblendw",	{ XM, Vex, EXx, Ib }, PREFIX_DATA },
    { "vpalignr",	{ XM, Vex, EXx, Ib }, PREFIX_DATA },
    /* 10 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F3A14) },
    { VEX_LEN_TABLE (VEX_LEN_0F3A15) },
    { VEX_LEN_TABLE (VEX_LEN_0F3A16) },
    { VEX_LEN_TABLE (VEX_LEN_0F3A17) },
    /* 18 */
    { VEX_LEN_TABLE (VEX_LEN_0F3A18) },
    { VEX_LEN_TABLE (VEX_LEN_0F3A19) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3A1D) },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 20 */
    { VEX_LEN_TABLE (VEX_LEN_0F3A20) },
    { VEX_LEN_TABLE (VEX_LEN_0F3A21) },
    { VEX_LEN_TABLE (VEX_LEN_0F3A22) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 28 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 30 */
    { VEX_LEN_TABLE (VEX_LEN_0F3A30) },
    { VEX_LEN_TABLE (VEX_LEN_0F3A31) },
    { VEX_LEN_TABLE (VEX_LEN_0F3A32) },
    { VEX_LEN_TABLE (VEX_LEN_0F3A33) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 38 */
    { VEX_LEN_TABLE (VEX_LEN_0F3A38) },
    { VEX_LEN_TABLE (VEX_LEN_0F3A39) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 40 */
    { "vdpps",		{ XM, Vex, EXx, Ib }, PREFIX_DATA },
    { VEX_LEN_TABLE (VEX_LEN_0F3A41) },
    { "vmpsadbw",	{ XM, Vex, EXx, Ib }, PREFIX_DATA },
    { Bad_Opcode },
    { "vpclmulqdq",	{ XM, Vex, EXx, PCLMUL }, PREFIX_DATA },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F3A46) },
    { Bad_Opcode },
    /* 48 */
    { "vpermil2ps",	{ XM, Vex, EXx, XMVexI4, VexI4 }, PREFIX_DATA },
    { "vpermil2pd",	{ XM, Vex, EXx, XMVexI4, VexI4 }, PREFIX_DATA },
    { VEX_W_TABLE (VEX_W_0F3A4A) },
    { VEX_W_TABLE (VEX_W_0F3A4B) },
    { VEX_W_TABLE (VEX_W_0F3A4C) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 50 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 58 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { "vfmaddsubps", { XM, Vex, EXx, XMVexI4 }, PREFIX_DATA },
    { "vfmaddsubpd", { XM, Vex, EXx, XMVexI4 }, PREFIX_DATA },
    { "vfmsubaddps", { XM, Vex, EXx, XMVexI4 }, PREFIX_DATA },
    { "vfmsubaddpd", { XM, Vex, EXx, XMVexI4 }, PREFIX_DATA },
    /* 60 */
    { VEX_LEN_TABLE (VEX_LEN_0F3A60) },
    { VEX_LEN_TABLE (VEX_LEN_0F3A61) },
    { VEX_LEN_TABLE (VEX_LEN_0F3A62) },
    { VEX_LEN_TABLE (VEX_LEN_0F3A63) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 68 */
    { "vfmaddps", { XM, Vex, EXx, XMVexI4 }, PREFIX_DATA },
    { "vfmaddpd", { XM, Vex, EXx, XMVexI4 }, PREFIX_DATA },
    { "vfmaddss",	{ XMScalar, VexScalar, EXd, XMVexScalarI4 }, PREFIX_DATA },
    { "vfmaddsd",	{ XMScalar, VexScalar, EXq, XMVexScalarI4 }, PREFIX_DATA },
    { "vfmsubps", { XM, Vex, EXx, XMVexI4 }, PREFIX_DATA },
    { "vfmsubpd", { XM, Vex, EXx, XMVexI4 }, PREFIX_DATA },
    { "vfmsubss",	{ XMScalar, VexScalar, EXd, XMVexScalarI4 }, PREFIX_DATA },
    { "vfmsubsd",	{ XMScalar, VexScalar, EXq, XMVexScalarI4 }, PREFIX_DATA },
    /* 70 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 78 */
    { "vfnmaddps", { XM, Vex, EXx, XMVexI4 }, PREFIX_DATA },
    { "vfnmaddpd", { XM, Vex, EXx, XMVexI4 }, PREFIX_DATA },
    { "vfnmaddss",	{ XMScalar, VexScalar, EXd, XMVexScalarI4 }, PREFIX_DATA },
    { "vfnmaddsd",	{ XMScalar, VexScalar, EXq, XMVexScalarI4 }, PREFIX_DATA },
    { "vfnmsubps", { XM, Vex, EXx, XMVexI4 }, PREFIX_DATA },
    { "vfnmsubpd", { XM, Vex, EXx, XMVexI4 }, PREFIX_DATA },
    { "vfnmsubss",	{ XMScalar, VexScalar, EXd, XMVexScalarI4 }, PREFIX_DATA },
    { "vfnmsubsd",	{ XMScalar, VexScalar, EXq, XMVexScalarI4 }, PREFIX_DATA },
    /* 80 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 88 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 90 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* 98 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* a0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* a8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* b0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* b8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* c0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* c8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3ACE) },
    { VEX_W_TABLE (VEX_W_0F3ACF) },
    /* d0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* d8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F3ADF) },
    /* e0 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* e8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* f0 */
    { VEX_LEN_TABLE (VEX_LEN_0F3AF0) },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    /* f8 */
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
  },
};

#include "i386-dis-evex.h"

static const struct dis386 vex_len_table[][2] = {
  /* VEX_LEN_0F12_P_0_M_0 / VEX_LEN_0F12_P_2_M_0 */
  {
    { "%XEvmovlpX",	{ XM, Vex, EXq }, 0 },
  },

  /* VEX_LEN_0F12_P_0_M_1 */
  {
    { "%XEvmovhlp%XS",	{ XM, Vex, EXq }, 0 },
  },

  /* VEX_LEN_0F13_M_0 */
  {
    { "%XEvmovlpX",	{ EXq, XM }, PREFIX_OPCODE },
  },

  /* VEX_LEN_0F16_P_0_M_0 / VEX_LEN_0F16_P_2_M_0 */
  {
    { "%XEvmovhpX",	{ XM, Vex, EXq }, 0 },
  },

  /* VEX_LEN_0F16_P_0_M_1 */
  {
    { "%XEvmovlhp%XS",	{ XM, Vex, EXq }, 0 },
  },

  /* VEX_LEN_0F17_M_0 */
  {
    { "%XEvmovhpX",	{ EXq, XM }, PREFIX_OPCODE },
  },

  /* VEX_LEN_0F41 */
  {
    { Bad_Opcode },
    { MOD_TABLE (MOD_VEX_0F41_L_1) },
  },

  /* VEX_LEN_0F42 */
  {
    { Bad_Opcode },
    { MOD_TABLE (MOD_VEX_0F42_L_1) },
  },

  /* VEX_LEN_0F44 */
  {
    { MOD_TABLE (MOD_VEX_0F44_L_0) },
  },

  /* VEX_LEN_0F45 */
  {
    { Bad_Opcode },
    { MOD_TABLE (MOD_VEX_0F45_L_1) },
  },

  /* VEX_LEN_0F46 */
  {
    { Bad_Opcode },
    { MOD_TABLE (MOD_VEX_0F46_L_1) },
  },

  /* VEX_LEN_0F47 */
  {
    { Bad_Opcode },
    { MOD_TABLE (MOD_VEX_0F47_L_1) },
  },

  /* VEX_LEN_0F4A */
  {
    { Bad_Opcode },
    { MOD_TABLE (MOD_VEX_0F4A_L_1) },
  },

  /* VEX_LEN_0F4B */
  {
    { Bad_Opcode },
    { MOD_TABLE (MOD_VEX_0F4B_L_1) },
  },

  /* VEX_LEN_0F6E */
  {
    { "%XEvmovK",	{ XMScalar, Edq }, PREFIX_DATA },
  },

  /* VEX_LEN_0F77 */
  {
    { "vzeroupper",	{ XX }, 0 },
    { "vzeroall",	{ XX }, 0 },
  },

  /* VEX_LEN_0F7E_P_1 */
  {
    { "%XEvmovq",	{ XMScalar, EXq }, 0 },
  },

  /* VEX_LEN_0F7E_P_2 */
  {
    { "%XEvmovK",	{ Edq, XMScalar }, 0 },
  },

  /* VEX_LEN_0F90 */
  {
    { VEX_W_TABLE (VEX_W_0F90_L_0) },
  },

  /* VEX_LEN_0F91 */
  {
    { MOD_TABLE (MOD_VEX_0F91_L_0) },
  },

  /* VEX_LEN_0F92 */
  {
    { MOD_TABLE (MOD_VEX_0F92_L_0) },
  },

  /* VEX_LEN_0F93 */
  {
    { MOD_TABLE (MOD_VEX_0F93_L_0) },
  },

  /* VEX_LEN_0F98 */
  {
    { MOD_TABLE (MOD_VEX_0F98_L_0) },
  },

  /* VEX_LEN_0F99 */
  {
    { MOD_TABLE (MOD_VEX_0F99_L_0) },
  },

  /* VEX_LEN_0FAE_R_2_M_0 */
  {
    { "vldmxcsr",	{ Md }, 0 },
  },

  /* VEX_LEN_0FAE_R_3_M_0 */
  {
    { "vstmxcsr",	{ Md }, 0 },
  },

  /* VEX_LEN_0FC4 */
  {
    { "%XEvpinsrw",	{ XM, Vex, Edw, Ib }, PREFIX_DATA },
  },

  /* VEX_LEN_0FC5 */
  {
    { "%XEvpextrw",	{ Gd, XS, Ib }, PREFIX_DATA },
  },

  /* VEX_LEN_0FD6 */
  {
    { "%XEvmovq",	{ EXqS, XMScalar }, PREFIX_DATA },
  },

  /* VEX_LEN_0FF7 */
  {
    { "vmaskmovdqu",	{ XM, XS }, PREFIX_DATA },
  },

  /* VEX_LEN_0F3816 */
  {
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3816_L_1) },
  },

  /* VEX_LEN_0F3819 */
  {
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3819_L_1) },
  },

  /* VEX_LEN_0F381A_M_0 */
  {
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F381A_M_0_L_1) },
  },

  /* VEX_LEN_0F3836 */
  {
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3836) },
  },

  /* VEX_LEN_0F3841 */
  {
    { "vphminposuw",	{ XM, EXx }, PREFIX_DATA },
  },

  /* VEX_LEN_0F3849_X86_64 */
  {
    { VEX_W_TABLE (VEX_W_0F3849_X86_64_L_0) },
  },

  /* VEX_LEN_0F384B_X86_64 */
  {
    { VEX_W_TABLE (VEX_W_0F384B_X86_64_L_0) },
  },

  /* VEX_LEN_0F385A_M_0 */
  {
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F385A_M_0_L_0) },
  },

  /* VEX_LEN_0F385C_X86_64_M_1 */
  {
    { VEX_W_TABLE (VEX_W_0F385C_X86_64_M_1_L_0) },
  },

  /* VEX_LEN_0F385E_X86_64_M_1 */
  {
    { VEX_W_TABLE (VEX_W_0F385E_X86_64_M_1_L_0) },
  },

  /* VEX_LEN_0F386C_X86_64_M_1 */
  {
    { VEX_W_TABLE (VEX_W_0F386C_X86_64_M_1_L_0) },
  },

  /* VEX_LEN_0F38DB */
  {
    { "vaesimc",	{ XM, EXx }, PREFIX_DATA },
  },

  /* VEX_LEN_0F38F2 */
  {
    { "andnS",		{ Gdq, VexGdq, Edq }, PREFIX_OPCODE },
  },

  /* VEX_LEN_0F38F3 */
  {
    { REG_TABLE(REG_VEX_0F38F3_L_0) },
  },

  /* VEX_LEN_0F38F5 */
  {
    { PREFIX_TABLE(PREFIX_VEX_0F38F5_L_0) },
  },

  /* VEX_LEN_0F38F6 */
  {
    { PREFIX_TABLE(PREFIX_VEX_0F38F6_L_0) },
  },

  /* VEX_LEN_0F38F7 */
  {
    { PREFIX_TABLE(PREFIX_VEX_0F38F7_L_0) },
  },

  /* VEX_LEN_0F3A00 */
  {
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3A00_L_1) },
  },

  /* VEX_LEN_0F3A01 */
  {
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3A01_L_1) },
  },

  /* VEX_LEN_0F3A06 */
  {
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3A06_L_1) },
  },

  /* VEX_LEN_0F3A14 */
  {
    { "%XEvpextrb",	{ Edb, XM, Ib }, PREFIX_DATA },
  },

  /* VEX_LEN_0F3A15 */
  {
    { "%XEvpextrw",	{ Edw, XM, Ib }, PREFIX_DATA },
  },

  /* VEX_LEN_0F3A16  */
  {
    { "%XEvpextrK",	{ Edq, XM, Ib }, PREFIX_DATA },
  },

  /* VEX_LEN_0F3A17 */
  {
    { "%XEvextractps",	{ Ed, XM, Ib }, PREFIX_DATA },
  },

  /* VEX_LEN_0F3A18 */
  {
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3A18_L_1) },
  },

  /* VEX_LEN_0F3A19 */
  {
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3A19_L_1) },
  },

  /* VEX_LEN_0F3A20 */
  {
    { "%XEvpinsrb",	{ XM, Vex, Edb, Ib }, PREFIX_DATA },
  },

  /* VEX_LEN_0F3A21 */
  {
    { "%XEvinsertps",	{ XM, Vex, EXd, Ib }, PREFIX_DATA },
  },

  /* VEX_LEN_0F3A22 */
  {
    { "%XEvpinsrK",	{ XM, Vex, Edq, Ib }, PREFIX_DATA },
  },

  /* VEX_LEN_0F3A30 */
  {
    { MOD_TABLE (MOD_VEX_0F3A30_L_0) },
  },

  /* VEX_LEN_0F3A31 */
  {
    { MOD_TABLE (MOD_VEX_0F3A31_L_0) },
  },

  /* VEX_LEN_0F3A32 */
  {
    { MOD_TABLE (MOD_VEX_0F3A32_L_0) },
  },

  /* VEX_LEN_0F3A33 */
  {
    { MOD_TABLE (MOD_VEX_0F3A33_L_0) },
  },

  /* VEX_LEN_0F3A38 */
  {
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3A38_L_1) },
  },

  /* VEX_LEN_0F3A39 */
  {
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3A39_L_1) },
  },

  /* VEX_LEN_0F3A41 */
  {
    { "vdppd",		{ XM, Vex, EXx, Ib }, PREFIX_DATA },
  },

  /* VEX_LEN_0F3A46 */
  {
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F3A46_L_1) },
  },

  /* VEX_LEN_0F3A60 */
  {
    { "vpcmpestrm!%LQ",	{ XM, EXx, Ib }, PREFIX_DATA },
  },

  /* VEX_LEN_0F3A61 */
  {
    { "vpcmpestri!%LQ",	{ XM, EXx, Ib }, PREFIX_DATA },
  },

  /* VEX_LEN_0F3A62 */
  {
    { "vpcmpistrm",	{ XM, EXx, Ib }, PREFIX_DATA },
  },

  /* VEX_LEN_0F3A63 */
  {
    { "vpcmpistri",	{ XM, EXx, Ib }, PREFIX_DATA },
  },

  /* VEX_LEN_0F3ADF */
  {
    { "vaeskeygenassist", { XM, EXx, Ib }, PREFIX_DATA },
  },

  /* VEX_LEN_0F3AF0 */
  {
    { PREFIX_TABLE (PREFIX_VEX_0F3AF0_L_0) },
  },

  /* VEX_LEN_0FXOP_08_85 */
  {
    { VEX_W_TABLE (VEX_W_0FXOP_08_85_L_0) },
  },

  /* VEX_LEN_0FXOP_08_86 */
  {
    { VEX_W_TABLE (VEX_W_0FXOP_08_86_L_0) },
  },

  /* VEX_LEN_0FXOP_08_87 */
  {
    { VEX_W_TABLE (VEX_W_0FXOP_08_87_L_0) },
  },

  /* VEX_LEN_0FXOP_08_8E */
  {
    { VEX_W_TABLE (VEX_W_0FXOP_08_8E_L_0) },
  },

  /* VEX_LEN_0FXOP_08_8F */
  {
    { VEX_W_TABLE (VEX_W_0FXOP_08_8F_L_0) },
  },

  /* VEX_LEN_0FXOP_08_95 */
  {
    { VEX_W_TABLE (VEX_W_0FXOP_08_95_L_0) },
  },

  /* VEX_LEN_0FXOP_08_96 */
  {
    { VEX_W_TABLE (VEX_W_0FXOP_08_96_L_0) },
  },

  /* VEX_LEN_0FXOP_08_97 */
  {
    { VEX_W_TABLE (VEX_W_0FXOP_08_97_L_0) },
  },

  /* VEX_LEN_0FXOP_08_9E */
  {
    { VEX_W_TABLE (VEX_W_0FXOP_08_9E_L_0) },
  },

  /* VEX_LEN_0FXOP_08_9F */
  {
    { VEX_W_TABLE (VEX_W_0FXOP_08_9F_L_0) },
  },

  /* VEX_LEN_0FXOP_08_A3 */
  {
    { "vpperm", 	{ XM, Vex, EXx, XMVexI4 }, 0 },
  },

  /* VEX_LEN_0FXOP_08_A6 */
  {
    { VEX_W_TABLE (VEX_W_0FXOP_08_A6_L_0) },
  },

  /* VEX_LEN_0FXOP_08_B6 */
  {
    { VEX_W_TABLE (VEX_W_0FXOP_08_B6_L_0) },
  },

  /* VEX_LEN_0FXOP_08_C0 */
  {
    { VEX_W_TABLE (VEX_W_0FXOP_08_C0_L_0) },
  },

  /* VEX_LEN_0FXOP_08_C1 */
  {
    { VEX_W_TABLE (VEX_W_0FXOP_08_C1_L_0) },
  },

  /* VEX_LEN_0FXOP_08_C2 */
  {
    { VEX_W_TABLE (VEX_W_0FXOP_08_C2_L_0) },
  },

  /* VEX_LEN_0FXOP_08_C3 */
  {
    { VEX_W_TABLE (VEX_W_0FXOP_08_C3_L_0) },
  },

  /* VEX_LEN_0FXOP_08_CC */
  {
    { VEX_W_TABLE (VEX_W_0FXOP_08_CC_L_0) },
  },

  /* VEX_LEN_0FXOP_08_CD */
  {
    { VEX_W_TABLE (VEX_W_0FXOP_08_CD_L_0) },
  },

  /* VEX_LEN_0FXOP_08_CE */
  {
    { VEX_W_TABLE (VEX_W_0FXOP_08_CE_L_0) },
  },

  /* VEX_LEN_0FXOP_08_CF */
  {
    { VEX_W_TABLE (VEX_W_0FXOP_08_CF_L_0) },
  },

  /* VEX_LEN_0FXOP_08_EC */
  {
    { VEX_W_TABLE (VEX_W_0FXOP_08_EC_L_0) },
  },

  /* VEX_LEN_0FXOP_08_ED */
  {
    { VEX_W_TABLE (VEX_W_0FXOP_08_ED_L_0) },
  },

  /* VEX_LEN_0FXOP_08_EE */
  {
    { VEX_W_TABLE (VEX_W_0FXOP_08_EE_L_0) },
  },

  /* VEX_LEN_0FXOP_08_EF */
  {
    { VEX_W_TABLE (VEX_W_0FXOP_08_EF_L_0) },
  },

  /* VEX_LEN_0FXOP_09_01 */
  {
    { REG_TABLE (REG_XOP_09_01_L_0) },
  },

  /* VEX_LEN_0FXOP_09_02 */
  {
    { REG_TABLE (REG_XOP_09_02_L_0) },
  },

  /* VEX_LEN_0FXOP_09_12_M_1 */
  {
    { REG_TABLE (REG_XOP_09_12_M_1_L_0) },
  },

  /* VEX_LEN_0FXOP_09_82_W_0 */
  {
    { "vfrczss", 	{ XM, EXd }, 0 },
  },

  /* VEX_LEN_0FXOP_09_83_W_0 */
  {
    { "vfrczsd", 	{ XM, EXq }, 0 },
  },

  /* VEX_LEN_0FXOP_09_90 */
  {
    { "vprotb",		{ XM, EXx, VexW }, 0 },
  },

  /* VEX_LEN_0FXOP_09_91 */
  {
    { "vprotw",		{ XM, EXx, VexW }, 0 },
  },

  /* VEX_LEN_0FXOP_09_92 */
  {
    { "vprotd",		{ XM, EXx, VexW }, 0 },
  },

  /* VEX_LEN_0FXOP_09_93 */
  {
    { "vprotq",		{ XM, EXx, VexW }, 0 },
  },

  /* VEX_LEN_0FXOP_09_94 */
  {
    { "vpshlb",		{ XM, EXx, VexW }, 0 },
  },

  /* VEX_LEN_0FXOP_09_95 */
  {
    { "vpshlw",		{ XM, EXx, VexW }, 0 },
  },

  /* VEX_LEN_0FXOP_09_96 */
  {
    { "vpshld",		{ XM, EXx, VexW }, 0 },
  },

  /* VEX_LEN_0FXOP_09_97 */
  {
    { "vpshlq",		{ XM, EXx, VexW }, 0 },
  },

  /* VEX_LEN_0FXOP_09_98 */
  {
    { "vpshab",		{ XM, EXx, VexW }, 0 },
  },

  /* VEX_LEN_0FXOP_09_99 */
  {
    { "vpshaw",		{ XM, EXx, VexW }, 0 },
  },

  /* VEX_LEN_0FXOP_09_9A */
  {
    { "vpshad",		{ XM, EXx, VexW }, 0 },
  },

  /* VEX_LEN_0FXOP_09_9B */
  {
    { "vpshaq",		{ XM, EXx, VexW }, 0 },
  },

  /* VEX_LEN_0FXOP_09_C1 */
  {
    { VEX_W_TABLE (VEX_W_0FXOP_09_C1_L_0) },
  },

  /* VEX_LEN_0FXOP_09_C2 */
  {
    { VEX_W_TABLE (VEX_W_0FXOP_09_C2_L_0) },
  },

  /* VEX_LEN_0FXOP_09_C3 */
  {
    { VEX_W_TABLE (VEX_W_0FXOP_09_C3_L_0) },
  },

  /* VEX_LEN_0FXOP_09_C6 */
  {
    { VEX_W_TABLE (VEX_W_0FXOP_09_C6_L_0) },
  },

  /* VEX_LEN_0FXOP_09_C7 */
  {
    { VEX_W_TABLE (VEX_W_0FXOP_09_C7_L_0) },
  },

  /* VEX_LEN_0FXOP_09_CB */
  {
    { VEX_W_TABLE (VEX_W_0FXOP_09_CB_L_0) },
  },

  /* VEX_LEN_0FXOP_09_D1 */
  {
    { VEX_W_TABLE (VEX_W_0FXOP_09_D1_L_0) },
  },

  /* VEX_LEN_0FXOP_09_D2 */
  {
    { VEX_W_TABLE (VEX_W_0FXOP_09_D2_L_0) },
  },

  /* VEX_LEN_0FXOP_09_D3 */
  {
    { VEX_W_TABLE (VEX_W_0FXOP_09_D3_L_0) },
  },

  /* VEX_LEN_0FXOP_09_D6 */
  {
    { VEX_W_TABLE (VEX_W_0FXOP_09_D6_L_0) },
  },

  /* VEX_LEN_0FXOP_09_D7 */
  {
    { VEX_W_TABLE (VEX_W_0FXOP_09_D7_L_0) },
  },

  /* VEX_LEN_0FXOP_09_DB */
  {
    { VEX_W_TABLE (VEX_W_0FXOP_09_DB_L_0) },
  },

  /* VEX_LEN_0FXOP_09_E1 */
  {
    { VEX_W_TABLE (VEX_W_0FXOP_09_E1_L_0) },
  },

  /* VEX_LEN_0FXOP_09_E2 */
  {
    { VEX_W_TABLE (VEX_W_0FXOP_09_E2_L_0) },
  },

  /* VEX_LEN_0FXOP_09_E3 */
  {
    { VEX_W_TABLE (VEX_W_0FXOP_09_E3_L_0) },
  },

  /* VEX_LEN_0FXOP_0A_12 */
  {
    { REG_TABLE (REG_XOP_0A_12_L_0) },
  },
};

#include "i386-dis-evex-len.h"

static const struct dis386 vex_w_table[][2] = {
  {
    /* VEX_W_0F41_L_1_M_1 */
    { PREFIX_TABLE (PREFIX_VEX_0F41_L_1_M_1_W_0) },
    { PREFIX_TABLE (PREFIX_VEX_0F41_L_1_M_1_W_1) },
  },
  {
    /* VEX_W_0F42_L_1_M_1 */
    { PREFIX_TABLE (PREFIX_VEX_0F42_L_1_M_1_W_0) },
    { PREFIX_TABLE (PREFIX_VEX_0F42_L_1_M_1_W_1) },
  },
  {
    /* VEX_W_0F44_L_0_M_1 */
    { PREFIX_TABLE (PREFIX_VEX_0F44_L_0_M_1_W_0) },
    { PREFIX_TABLE (PREFIX_VEX_0F44_L_0_M_1_W_1) },
  },
  {
    /* VEX_W_0F45_L_1_M_1 */
    { PREFIX_TABLE (PREFIX_VEX_0F45_L_1_M_1_W_0) },
    { PREFIX_TABLE (PREFIX_VEX_0F45_L_1_M_1_W_1) },
  },
  {
    /* VEX_W_0F46_L_1_M_1 */
    { PREFIX_TABLE (PREFIX_VEX_0F46_L_1_M_1_W_0) },
    { PREFIX_TABLE (PREFIX_VEX_0F46_L_1_M_1_W_1) },
  },
  {
    /* VEX_W_0F47_L_1_M_1 */
    { PREFIX_TABLE (PREFIX_VEX_0F47_L_1_M_1_W_0) },
    { PREFIX_TABLE (PREFIX_VEX_0F47_L_1_M_1_W_1) },
  },
  {
    /* VEX_W_0F4A_L_1_M_1 */
    { PREFIX_TABLE (PREFIX_VEX_0F4A_L_1_M_1_W_0) },
    { PREFIX_TABLE (PREFIX_VEX_0F4A_L_1_M_1_W_1) },
  },
  {
    /* VEX_W_0F4B_L_1_M_1 */
    { PREFIX_TABLE (PREFIX_VEX_0F4B_L_1_M_1_W_0) },
    { PREFIX_TABLE (PREFIX_VEX_0F4B_L_1_M_1_W_1) },
  },
  {
    /* VEX_W_0F90_L_0 */
    { PREFIX_TABLE (PREFIX_VEX_0F90_L_0_W_0) },
    { PREFIX_TABLE (PREFIX_VEX_0F90_L_0_W_1) },
  },
  {
    /* VEX_W_0F91_L_0_M_0 */
    { PREFIX_TABLE (PREFIX_VEX_0F91_L_0_M_0_W_0) },
    { PREFIX_TABLE (PREFIX_VEX_0F91_L_0_M_0_W_1) },
  },
  {
    /* VEX_W_0F92_L_0_M_1 */
    { PREFIX_TABLE (PREFIX_VEX_0F92_L_0_M_1_W_0) },
    { PREFIX_TABLE (PREFIX_VEX_0F92_L_0_M_1_W_1) },
  },
  {
    /* VEX_W_0F93_L_0_M_1 */
    { PREFIX_TABLE (PREFIX_VEX_0F93_L_0_M_1_W_0) },
    { PREFIX_TABLE (PREFIX_VEX_0F93_L_0_M_1_W_1) },
  },
  {
    /* VEX_W_0F98_L_0_M_1 */
    { PREFIX_TABLE (PREFIX_VEX_0F98_L_0_M_1_W_0) },
    { PREFIX_TABLE (PREFIX_VEX_0F98_L_0_M_1_W_1) },
  },
  {
    /* VEX_W_0F99_L_0_M_1 */
    { PREFIX_TABLE (PREFIX_VEX_0F99_L_0_M_1_W_0) },
    { PREFIX_TABLE (PREFIX_VEX_0F99_L_0_M_1_W_1) },
  },
  {
    /* VEX_W_0F380C  */
    { "%XEvpermilps",	{ XM, Vex, EXx }, PREFIX_DATA },
  },
  {
    /* VEX_W_0F380D  */
    { "vpermilpd",	{ XM, Vex, EXx }, PREFIX_DATA },
  },
  {
    /* VEX_W_0F380E  */
    { "vtestps",	{ XM, EXx }, PREFIX_DATA },
  },
  {
    /* VEX_W_0F380F  */
    { "vtestpd",	{ XM, EXx }, PREFIX_DATA },
  },
  {
    /* VEX_W_0F3813 */
    { "vcvtph2ps", { XM, EXxmmq }, PREFIX_DATA },
  },
  {
    /* VEX_W_0F3816_L_1  */
    { "vpermps",	{ XM, Vex, EXx }, PREFIX_DATA },
  },
  {
    /* VEX_W_0F3818 */
    { "%XEvbroadcastss",	{ XM, EXd }, PREFIX_DATA },
  },
  {
    /* VEX_W_0F3819_L_1 */
    { "vbroadcastsd",	{ XM, EXq }, PREFIX_DATA },
  },
  {
    /* VEX_W_0F381A_M_0_L_1 */
    { "vbroadcastf128",	{ XM, Mxmm }, PREFIX_DATA },
  },
  {
    /* VEX_W_0F382C_M_0 */
    { "vmaskmovps",	{ XM, Vex, Mx }, PREFIX_DATA },
  },
  {
    /* VEX_W_0F382D_M_0 */
    { "vmaskmovpd",	{ XM, Vex, Mx }, PREFIX_DATA },
  },
  {
    /* VEX_W_0F382E_M_0 */
    { "vmaskmovps",	{ Mx, Vex, XM }, PREFIX_DATA },
  },
  {
    /* VEX_W_0F382F_M_0 */
    { "vmaskmovpd",	{ Mx, Vex, XM }, PREFIX_DATA },
  },
  {
    /* VEX_W_0F3836  */
    { "vpermd",		{ XM, Vex, EXx }, PREFIX_DATA },
  },
  {
    /* VEX_W_0F3846 */
    { "vpsravd",	{ XM, Vex, EXx }, PREFIX_DATA },
  },
  {
    /* VEX_W_0F3849_X86_64_L_0 */
    { MOD_TABLE (MOD_VEX_0F3849_X86_64_L_0_W_0) },
  },
  {
    /* VEX_W_0F384B_X86_64_L_0 */
    { MOD_TABLE (MOD_VEX_0F384B_X86_64_L_0_W_0) },
  },
  {
    /* VEX_W_0F3850 */
    { PREFIX_TABLE (PREFIX_VEX_0F3850_W_0) },
  },
  {
    /* VEX_W_0F3851 */
    { PREFIX_TABLE (PREFIX_VEX_0F3851_W_0) },
  },
  {
    /* VEX_W_0F3852 */
    { "%XVvpdpwssd",	{ XM, Vex, EXx }, PREFIX_DATA },
  },
  {
    /* VEX_W_0F3853 */
    { "%XVvpdpwssds",	{ XM, Vex, EXx }, PREFIX_DATA },
  },
  {
    /* VEX_W_0F3858 */
    { "%XEvpbroadcastd", { XM, EXd }, PREFIX_DATA },
  },
  {
    /* VEX_W_0F3859 */
    { "vpbroadcastq", { XM, EXq }, PREFIX_DATA },
  },
  {
    /* VEX_W_0F385A_M_0_L_0 */
    { "vbroadcasti128", { XM, Mxmm }, PREFIX_DATA },
  },
  {
    /* VEX_W_0F385C_X86_64_M_1_L_0 */
    { PREFIX_TABLE (PREFIX_VEX_0F385C_X86_64_M_1_L_0_W_0) },
  },
  {
    /* VEX_W_0F385E_X86_64_M_1_L_0 */
    { PREFIX_TABLE (PREFIX_VEX_0F385E_X86_64_M_1_L_0_W_0) },
  },
  {
    /* VEX_W_0F386C_X86_64_M_1_L_0 */
    { PREFIX_TABLE (PREFIX_VEX_0F386C_X86_64_M_1_L_0_W_0) },
  },
  {
    /* VEX_W_0F3872_P_1 */
    { "%XVvcvtneps2bf16%XY", { XMM, EXx }, 0 },
  },
  {
    /* VEX_W_0F3878 */
    { "%XEvpbroadcastb",	{ XM, EXb }, PREFIX_DATA },
  },
  {
    /* VEX_W_0F3879 */
    { "%XEvpbroadcastw",	{ XM, EXw }, PREFIX_DATA },
  },
  {
    /* VEX_W_0F38B0 */
    { PREFIX_TABLE (PREFIX_VEX_0F38B0_W_0) },
  },
  {
    /* VEX_W_0F38B1 */
    { PREFIX_TABLE (PREFIX_VEX_0F38B1_W_0) },
  },
  {
    /* VEX_W_0F38B4 */
    { Bad_Opcode },
    { "%XVvpmadd52luq",	{ XM, Vex, EXx }, PREFIX_DATA },
  },
  {
    /* VEX_W_0F38B5 */
    { Bad_Opcode },
    { "%XVvpmadd52huq",	{ XM, Vex, EXx }, PREFIX_DATA },
  },
  {
    /* VEX_W_0F38CF */
    { "%XEvgf2p8mulb", { XM, Vex, EXx }, PREFIX_DATA },
  },
  {
    /* VEX_W_0F3A00_L_1 */
    { Bad_Opcode },
    { "%XEvpermq",		{ XM, EXx, Ib }, PREFIX_DATA },
  },
  {
    /* VEX_W_0F3A01_L_1 */
    { Bad_Opcode },
    { "%XEvpermpd",	{ XM, EXx, Ib }, PREFIX_DATA },
  },
  {
    /* VEX_W_0F3A02 */
    { "vpblendd",	{ XM, Vex, EXx, Ib }, PREFIX_DATA },
  },
  {
    /* VEX_W_0F3A04 */
    { "%XEvpermilps",	{ XM, EXx, Ib }, PREFIX_DATA },
  },
  {
    /* VEX_W_0F3A05 */
    { "vpermilpd",	{ XM, EXx, Ib }, PREFIX_DATA },
  },
  {
    /* VEX_W_0F3A06_L_1 */
    { "vperm2f128",	{ XM, Vex, EXx, Ib }, PREFIX_DATA },
  },
  {
    /* VEX_W_0F3A18_L_1 */
    { "vinsertf128",	{ XM, Vex, EXxmm, Ib }, PREFIX_DATA },
  },
  {
    /* VEX_W_0F3A19_L_1 */
    { "vextractf128",	{ EXxmm, XM, Ib }, PREFIX_DATA },
  },
  {
    /* VEX_W_0F3A1D */
    { "%XEvcvtps2ph", { EXxmmq, XM, EXxEVexS, Ib }, PREFIX_DATA },
  },
  {
    /* VEX_W_0F3A38_L_1 */
    { "vinserti128",	{ XM, Vex, EXxmm, Ib }, PREFIX_DATA },
  },
  {
    /* VEX_W_0F3A39_L_1 */
    { "vextracti128",	{ EXxmm, XM, Ib }, PREFIX_DATA },
  },
  {
    /* VEX_W_0F3A46_L_1 */
    { "vperm2i128",	{ XM, Vex, EXx, Ib }, PREFIX_DATA },
  },
  {
    /* VEX_W_0F3A4A */
    { "vblendvps",	{ XM, Vex, EXx, XMVexI4 }, PREFIX_DATA },
  },
  {
    /* VEX_W_0F3A4B */
    { "vblendvpd",	{ XM, Vex, EXx, XMVexI4 }, PREFIX_DATA },
  },
  {
    /* VEX_W_0F3A4C */
    { "vpblendvb",	{ XM, Vex, EXx, XMVexI4 }, PREFIX_DATA },
  },
  {
    /* VEX_W_0F3ACE */
    { Bad_Opcode },
    { "%XEvgf2p8affineqb", { XM, Vex, EXx, Ib }, PREFIX_DATA },
  },
  {
    /* VEX_W_0F3ACF */
    { Bad_Opcode },
    { "%XEvgf2p8affineinvqb",  { XM, Vex, EXx, Ib }, PREFIX_DATA },
  },
  /* VEX_W_0FXOP_08_85_L_0 */
  {
    { "vpmacssww", 	{ XM, Vex, EXx, XMVexI4 }, 0 },
  },
  /* VEX_W_0FXOP_08_86_L_0 */
  {
    { "vpmacsswd", 	{ XM, Vex, EXx, XMVexI4 }, 0 },
  },
  /* VEX_W_0FXOP_08_87_L_0 */
  {
    { "vpmacssdql", 	{ XM, Vex, EXx, XMVexI4 }, 0 },
  },
  /* VEX_W_0FXOP_08_8E_L_0 */
  {
    { "vpmacssdd", 	{ XM, Vex, EXx, XMVexI4 }, 0 },
  },
  /* VEX_W_0FXOP_08_8F_L_0 */
  {
    { "vpmacssdqh", 	{ XM, Vex, EXx, XMVexI4 }, 0 },
  },
  /* VEX_W_0FXOP_08_95_L_0 */
  {
    { "vpmacsww", 	{ XM, Vex, EXx, XMVexI4 }, 0 },
  },
  /* VEX_W_0FXOP_08_96_L_0 */
  {
    { "vpmacswd", 	{ XM, Vex, EXx, XMVexI4 }, 0 },
  },
  /* VEX_W_0FXOP_08_97_L_0 */
  {
    { "vpmacsdql", 	{ XM, Vex, EXx, XMVexI4 }, 0 },
  },
  /* VEX_W_0FXOP_08_9E_L_0 */
  {
    { "vpmacsdd", 	{ XM, Vex, EXx, XMVexI4 }, 0 },
  },
  /* VEX_W_0FXOP_08_9F_L_0 */
  {
    { "vpmacsdqh", 	{ XM, Vex, EXx, XMVexI4 }, 0 },
  },
  /* VEX_W_0FXOP_08_A6_L_0 */
  {
    { "vpmadcsswd", 	{ XM, Vex, EXx, XMVexI4 }, 0 },
  },
  /* VEX_W_0FXOP_08_B6_L_0 */
  {
    { "vpmadcswd", 	{ XM, Vex, EXx, XMVexI4 }, 0 },
  },
  /* VEX_W_0FXOP_08_C0_L_0 */
  {
    { "vprotb", 	{ XM, EXx, Ib }, 0 },
  },
  /* VEX_W_0FXOP_08_C1_L_0 */
  {
    { "vprotw", 	{ XM, EXx, Ib }, 0 },
  },
  /* VEX_W_0FXOP_08_C2_L_0 */
  {
    { "vprotd", 	{ XM, EXx, Ib }, 0 },
  },
  /* VEX_W_0FXOP_08_C3_L_0 */
  {
    { "vprotq", 	{ XM, EXx, Ib }, 0 },
  },
  /* VEX_W_0FXOP_08_CC_L_0 */
  {
     { "vpcomb",	{ XM, Vex, EXx, VPCOM }, 0 },
  },
  /* VEX_W_0FXOP_08_CD_L_0 */
  {
     { "vpcomw",	{ XM, Vex, EXx, VPCOM }, 0 },
  },
  /* VEX_W_0FXOP_08_CE_L_0 */
  {
     { "vpcomd",	{ XM, Vex, EXx, VPCOM }, 0 },
  },
  /* VEX_W_0FXOP_08_CF_L_0 */
  {
     { "vpcomq",	{ XM, Vex, EXx, VPCOM }, 0 },
  },
  /* VEX_W_0FXOP_08_EC_L_0 */
  {
     { "vpcomub",	{ XM, Vex, EXx, VPCOM }, 0 },
  },
  /* VEX_W_0FXOP_08_ED_L_0 */
  {
     { "vpcomuw",	{ XM, Vex, EXx, VPCOM }, 0 },
  },
  /* VEX_W_0FXOP_08_EE_L_0 */
  {
     { "vpcomud",	{ XM, Vex, EXx, VPCOM }, 0 },
  },
  /* VEX_W_0FXOP_08_EF_L_0 */
  {
     { "vpcomuq",	{ XM, Vex, EXx, VPCOM }, 0 },
  },
  /* VEX_W_0FXOP_09_80 */
  {
    { "vfrczps",	{ XM, EXx }, 0 },
  },
  /* VEX_W_0FXOP_09_81 */
  {
    { "vfrczpd",	{ XM, EXx }, 0 },
  },
  /* VEX_W_0FXOP_09_82 */
  {
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_09_82_W_0) },
  },
  /* VEX_W_0FXOP_09_83 */
  {
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_09_83_W_0) },
  },
  /* VEX_W_0FXOP_09_C1_L_0 */
  {
    { "vphaddbw",	{ XM, EXxmm }, 0 },
  },
  /* VEX_W_0FXOP_09_C2_L_0 */
  {
    { "vphaddbd",	{ XM, EXxmm }, 0 },
  },
  /* VEX_W_0FXOP_09_C3_L_0 */
  {
    { "vphaddbq",	{ XM, EXxmm }, 0 },
  },
  /* VEX_W_0FXOP_09_C6_L_0 */
  {
    { "vphaddwd",	{ XM, EXxmm }, 0 },
  },
  /* VEX_W_0FXOP_09_C7_L_0 */
  {
    { "vphaddwq",	{ XM, EXxmm }, 0 },
  },
  /* VEX_W_0FXOP_09_CB_L_0 */
  {
    { "vphadddq",	{ XM, EXxmm }, 0 },
  },
  /* VEX_W_0FXOP_09_D1_L_0 */
  {
    { "vphaddubw",	{ XM, EXxmm }, 0 },
  },
  /* VEX_W_0FXOP_09_D2_L_0 */
  {
    { "vphaddubd",	{ XM, EXxmm }, 0 },
  },
  /* VEX_W_0FXOP_09_D3_L_0 */
  {
    { "vphaddubq",	{ XM, EXxmm }, 0 },
  },
  /* VEX_W_0FXOP_09_D6_L_0 */
  {
    { "vphadduwd",	{ XM, EXxmm }, 0 },
  },
  /* VEX_W_0FXOP_09_D7_L_0 */
  {
    { "vphadduwq",	{ XM, EXxmm }, 0 },
  },
  /* VEX_W_0FXOP_09_DB_L_0 */
  {
    { "vphaddudq",	{ XM, EXxmm }, 0 },
  },
  /* VEX_W_0FXOP_09_E1_L_0 */
  {
    { "vphsubbw",	{ XM, EXxmm }, 0 },
  },
  /* VEX_W_0FXOP_09_E2_L_0 */
  {
    { "vphsubwd",	{ XM, EXxmm }, 0 },
  },
  /* VEX_W_0FXOP_09_E3_L_0 */
  {
    { "vphsubdq",	{ XM, EXxmm }, 0 },
  },

#include "i386-dis-evex-w.h"
};

static const struct dis386 mod_table[][2] = {
  {
    /* MOD_62_32BIT */
    { "bound{S|}",	{ Gv, Ma }, 0 },
    { EVEX_TABLE (EVEX_0F) },
  },
  {
    /* MOD_8D */
    { "leaS",		{ Gv, M }, 0 },
  },
  {
    /* MOD_C4_32BIT */
    { "lesS",		{ Gv, Mp }, 0 },
    { VEX_C4_TABLE (VEX_0F) },
  },
  {
    /* MOD_C5_32BIT */
    { "ldsS",		{ Gv, Mp }, 0 },
    { VEX_C5_TABLE (VEX_0F) },
  },
  {
    /* MOD_C6_REG_7 */
    { Bad_Opcode },
    { RM_TABLE (RM_C6_REG_7) },
  },
  {
    /* MOD_C7_REG_7 */
    { Bad_Opcode },
    { RM_TABLE (RM_C7_REG_7) },
  },
  {
    /* MOD_FF_REG_3 */
    { "{l|}call^", { indirEp }, 0 },
  },
  {
    /* MOD_FF_REG_5 */
    { "{l|}jmp^", { indirEp }, 0 },
  },
  {
    /* MOD_0F01_REG_0 */
    { X86_64_TABLE (X86_64_0F01_REG_0) },
    { RM_TABLE (RM_0F01_REG_0) },
  },
  {
    /* MOD_0F01_REG_1 */
    { X86_64_TABLE (X86_64_0F01_REG_1) },
    { RM_TABLE (RM_0F01_REG_1) },
  },
  {
    /* MOD_0F01_REG_2 */
    { X86_64_TABLE (X86_64_0F01_REG_2) },
    { RM_TABLE (RM_0F01_REG_2) },
  },
  {
    /* MOD_0F01_REG_3 */
    { X86_64_TABLE (X86_64_0F01_REG_3) },
    { RM_TABLE (RM_0F01_REG_3) },
  },
  {
    /* MOD_0F01_REG_5 */
    { PREFIX_TABLE (PREFIX_0F01_REG_5_MOD_0) },
    { RM_TABLE (RM_0F01_REG_5_MOD_3) },
  },
  {
    /* MOD_0F01_REG_7 */
    { "invlpg",		{ Mb }, 0 },
    { RM_TABLE (RM_0F01_REG_7_MOD_3) },
  },
  {
    /* MOD_0F02 */
    { "larS",		{ Gv, Mw }, 0 },
    { "larS",		{ Gv, Ev }, 0 },
  },
  {
    /* MOD_0F03 */
    { "lslS",		{ Gv, Mw }, 0 },
    { "lslS",		{ Gv, Ev }, 0 },
  },
  {
    /* MOD_0F12_PREFIX_0 */
    { "movlpX",		{ XM, EXq }, 0 },
    { "movhlps",	{ XM, EXq }, 0 },
  },
  {
    /* MOD_0F12_PREFIX_2 */
    { "movlpX",	{ XM, EXq }, 0 },
  },
  {
    /* MOD_0F13 */
    { "movlpX",		{ EXq, XM }, PREFIX_OPCODE },
  },
  {
    /* MOD_0F16_PREFIX_0 */
    { "movhpX",		{ XM, EXq }, 0 },
    { "movlhps",	{ XM, EXq }, 0 },
  },
  {
    /* MOD_0F16_PREFIX_2 */
    { "movhpX",	{ XM, EXq }, 0 },
  },
  {
    /* MOD_0F17 */
    { "movhpX",		{ EXq, XM }, PREFIX_OPCODE },
  },
  {
    /* MOD_0F18_REG_0 */
    { "prefetchnta",	{ Mb }, 0 },
    { "nopQ",		{ Ev }, 0 },
  },
  {
    /* MOD_0F18_REG_1 */
    { "prefetcht0",	{ Mb }, 0 },
    { "nopQ",		{ Ev }, 0 },
  },
  {
    /* MOD_0F18_REG_2 */
    { "prefetcht1",	{ Mb }, 0 },
    { "nopQ",		{ Ev }, 0 },
  },
  {
    /* MOD_0F18_REG_3 */
    { "prefetcht2",	{ Mb }, 0 },
    { "nopQ",		{ Ev }, 0 },
  },
  {
    /* MOD_0F18_REG_6 */
    { X86_64_TABLE (X86_64_0F18_REG_6_MOD_0) },
    { "nopQ",		{ Ev }, 0 },
  },
  {
    /* MOD_0F18_REG_7 */
    { X86_64_TABLE (X86_64_0F18_REG_7_MOD_0) },
    { "nopQ",		{ Ev }, 0 },
  },
  {
    /* MOD_0F1A_PREFIX_0 */
    { "bndldx",		{ Gbnd, Mv_bnd }, 0 },
    { "nopQ",		{ Ev }, 0 },
  },
  {
    /* MOD_0F1B_PREFIX_0 */
    { "bndstx",		{ Mv_bnd, Gbnd }, 0 },
    { "nopQ",		{ Ev }, 0 },
  },
  {
    /* MOD_0F1B_PREFIX_1 */
    { "bndmk",		{ Gbnd, Mv_bnd }, 0 },
    { "nopQ",		{ Ev }, PREFIX_IGNORED },
  },
  {
    /* MOD_0F1C_PREFIX_0 */
    { REG_TABLE (REG_0F1C_P_0_MOD_0) },
    { "nopQ",		{ Ev }, 0 },
  },
  {
    /* MOD_0F1E_PREFIX_1 */
    { "nopQ",		{ Ev }, PREFIX_IGNORED },
    { REG_TABLE (REG_0F1E_P_1_MOD_3) },
  },
  {
    /* MOD_0F2B_PREFIX_0 */
    {"movntps",		{ Mx, XM }, PREFIX_OPCODE },
  },
  {
    /* MOD_0F2B_PREFIX_1 */
    {"movntss",		{ Md, XM }, PREFIX_OPCODE },
  },
  {
    /* MOD_0F2B_PREFIX_2 */
    {"movntpd",		{ Mx, XM }, PREFIX_OPCODE },
  },
  {
    /* MOD_0F2B_PREFIX_3 */
    {"movntsd",		{ Mq, XM }, PREFIX_OPCODE },
  },
  {
    /* MOD_0F50 */
    { Bad_Opcode },
    { "movmskpX",	{ Gdq, XS }, PREFIX_OPCODE },
  },
  {
    /* MOD_0F71 */
    { Bad_Opcode },
    { REG_TABLE (REG_0F71_MOD_0) },
  },
  {
    /* MOD_0F72 */
    { Bad_Opcode },
    { REG_TABLE (REG_0F72_MOD_0) },
  },
  {
    /* MOD_0F73 */
    { Bad_Opcode },
    { REG_TABLE (REG_0F73_MOD_0) },
  },
  {
    /* MOD_0FAE_REG_0 */
    { "fxsave",		{ FXSAVE }, 0 },
    { PREFIX_TABLE (PREFIX_0FAE_REG_0_MOD_3) },
  },
  {
    /* MOD_0FAE_REG_1 */
    { "fxrstor",	{ FXSAVE }, 0 },
    { PREFIX_TABLE (PREFIX_0FAE_REG_1_MOD_3) },
  },
  {
    /* MOD_0FAE_REG_2 */
    { "ldmxcsr",	{ Md }, 0 },
    { PREFIX_TABLE (PREFIX_0FAE_REG_2_MOD_3) },
  },
  {
    /* MOD_0FAE_REG_3 */
    { "stmxcsr",	{ Md }, 0 },
    { PREFIX_TABLE (PREFIX_0FAE_REG_3_MOD_3) },
  },
  {
    /* MOD_0FAE_REG_4 */
    { PREFIX_TABLE (PREFIX_0FAE_REG_4_MOD_0) },
    { PREFIX_TABLE (PREFIX_0FAE_REG_4_MOD_3) },
  },
  {
    /* MOD_0FAE_REG_5 */
    { "xrstor",		{ FXSAVE }, PREFIX_OPCODE },
    { PREFIX_TABLE (PREFIX_0FAE_REG_5_MOD_3) },
  },
  {
    /* MOD_0FAE_REG_6 */
    { PREFIX_TABLE (PREFIX_0FAE_REG_6_MOD_0) },
    { PREFIX_TABLE (PREFIX_0FAE_REG_6_MOD_3) },
  },
  {
    /* MOD_0FAE_REG_7 */
    { PREFIX_TABLE (PREFIX_0FAE_REG_7_MOD_0) },
    { RM_TABLE (RM_0FAE_REG_7_MOD_3) },
  },
  {
    /* MOD_0FB2 */
    { "lssS",		{ Gv, Mp }, 0 },
  },
  {
    /* MOD_0FB4 */
    { "lfsS",		{ Gv, Mp }, 0 },
  },
  {
    /* MOD_0FB5 */
    { "lgsS",		{ Gv, Mp }, 0 },
  },
  {
    /* MOD_0FC3 */
    { "movntiS", { Edq, Gdq }, PREFIX_OPCODE },
  },
  {
    /* MOD_0FC7_REG_3 */
    { "xrstors",	{ FXSAVE }, 0 },
  },
  {
    /* MOD_0FC7_REG_4 */
    { "xsavec",		{ FXSAVE }, 0 },
  },
  {
    /* MOD_0FC7_REG_5 */
    { "xsaves",		{ FXSAVE }, 0 },
  },
  {
    /* MOD_0FC7_REG_6 */
    { PREFIX_TABLE (PREFIX_0FC7_REG_6_MOD_0) },
    { PREFIX_TABLE (PREFIX_0FC7_REG_6_MOD_3) }
  },
  {
    /* MOD_0FC7_REG_7 */
    { "vmptrst",	{ Mq }, 0 },
    { PREFIX_TABLE (PREFIX_0FC7_REG_7_MOD_3) }
  },
  {
    /* MOD_0FD7 */
    { Bad_Opcode },
    { "pmovmskb",	{ Gdq, MS }, 0 },
  },
  {
    /* MOD_0FE7_PREFIX_2 */
    { "movntdq",	{ Mx, XM }, 0 },
  },
  {
    /* MOD_0FF0_PREFIX_3 */
    { "lddqu",		{ XM, M }, 0 },
  },
  {
    /* MOD_0F382A */
    { "movntdqa",	{ XM, Mx }, PREFIX_DATA },
  },
  {
    /* MOD_0F38DC_PREFIX_1 */
    { "aesenc128kl",    { XM, M }, 0 },
    { "loadiwkey",      { XM, EXx }, 0 },
  },
  {
    /* MOD_0F38DD_PREFIX_1 */
    { "aesdec128kl",    { XM, M }, 0 },
  },
  {
    /* MOD_0F38DE_PREFIX_1 */
    { "aesenc256kl",    { XM, M }, 0 },
  },
  {
    /* MOD_0F38DF_PREFIX_1 */
    { "aesdec256kl",    { XM, M }, 0 },
  },
  {
    /* MOD_0F38F5 */
    { "wrussK",		{ M, Gdq }, PREFIX_DATA },
  },
  {
    /* MOD_0F38F6_PREFIX_0 */
    { "wrssK",		{ M, Gdq }, PREFIX_OPCODE },
  },
  {
    /* MOD_0F38F8_PREFIX_1 */
    { "enqcmds",	{ Gva, M }, PREFIX_OPCODE },
  },
  {
    /* MOD_0F38F8_PREFIX_2 */
    { "movdir64b",	{ Gva, M }, PREFIX_OPCODE },
  },
  {
    /* MOD_0F38F8_PREFIX_3 */
    { "enqcmd",		{ Gva, M }, PREFIX_OPCODE },
  },
  {
    /* MOD_0F38F9 */
    { "movdiri",	{ Edq, Gdq }, PREFIX_OPCODE },
  },
  {
    /* MOD_0F38FA_PREFIX_1 */
    { Bad_Opcode },
    { "encodekey128", { Gd, Ed }, 0 },
  },
  {
    /* MOD_0F38FB_PREFIX_1 */
    { Bad_Opcode },
    { "encodekey256", { Gd, Ed }, 0 },
  },
  {
    /* MOD_0F3A0F_PREFIX_1 */
    { Bad_Opcode },
    { REG_TABLE (REG_0F3A0F_PREFIX_1_MOD_3) },
  },
  {
    /* MOD_VEX_0F12_PREFIX_0 */
    { VEX_LEN_TABLE (VEX_LEN_0F12_P_0_M_0) },
    { VEX_LEN_TABLE (VEX_LEN_0F12_P_0_M_1) },
  },
  {
    /* MOD_VEX_0F12_PREFIX_2 */
    { VEX_LEN_TABLE (VEX_LEN_0F12_P_2_M_0) },
  },
  {
    /* MOD_VEX_0F13 */
    { VEX_LEN_TABLE (VEX_LEN_0F13_M_0) },
  },
  {
    /* MOD_VEX_0F16_PREFIX_0 */
    { VEX_LEN_TABLE (VEX_LEN_0F16_P_0_M_0) },
    { VEX_LEN_TABLE (VEX_LEN_0F16_P_0_M_1) },
  },
  {
    /* MOD_VEX_0F16_PREFIX_2 */
    { VEX_LEN_TABLE (VEX_LEN_0F16_P_2_M_0) },
  },
  {
    /* MOD_VEX_0F17 */
    { VEX_LEN_TABLE (VEX_LEN_0F17_M_0) },
  },
  {
    /* MOD_VEX_0F2B */
    { "%XEvmovntpX",	{ Mx, XM }, PREFIX_OPCODE },
  },
  {
    /* MOD_VEX_0F41_L_1 */
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F41_L_1_M_1) },
  },
  {
    /* MOD_VEX_0F42_L_1 */
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F42_L_1_M_1) },
  },
  {
    /* MOD_VEX_0F44_L_0 */
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F44_L_0_M_1) },
  },
  {
    /* MOD_VEX_0F45_L_1 */
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F45_L_1_M_1) },
  },
  {
    /* MOD_VEX_0F46_L_1 */
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F46_L_1_M_1) },
  },
  {
    /* MOD_VEX_0F47_L_1 */
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F47_L_1_M_1) },
  },
  {
    /* MOD_VEX_0F4A_L_1 */
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F4A_L_1_M_1) },
  },
  {
    /* MOD_VEX_0F4B_L_1 */
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F4B_L_1_M_1) },
  },
  {
    /* MOD_VEX_0F50 */
    { Bad_Opcode },
    { "vmovmskpX",	{ Gdq, XS }, PREFIX_OPCODE },
  },
  {
    /* MOD_VEX_0F71 */
    { Bad_Opcode },
    { REG_TABLE (REG_VEX_0F71_M_0) },
  },
  {
    /* MOD_VEX_0F72 */
    { Bad_Opcode },
    { REG_TABLE (REG_VEX_0F72_M_0) },
  },
  {
    /* MOD_VEX_0F73 */
    { Bad_Opcode },
    { REG_TABLE (REG_VEX_0F73_M_0) },
  },
  {
    /* MOD_VEX_0F91_L_0 */
    { VEX_W_TABLE (VEX_W_0F91_L_0_M_0) },
  },
  {
    /* MOD_VEX_0F92_L_0 */
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F92_L_0_M_1) },
  },
  {
    /* MOD_VEX_0F93_L_0 */
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F93_L_0_M_1) },
  },
  {
    /* MOD_VEX_0F98_L_0 */
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F98_L_0_M_1) },
  },
  {
    /* MOD_VEX_0F99_L_0 */
    { Bad_Opcode },
    { VEX_W_TABLE (VEX_W_0F99_L_0_M_1) },
  },
  {
    /* MOD_VEX_0FAE_REG_2 */
    { VEX_LEN_TABLE (VEX_LEN_0FAE_R_2_M_0) },
  },
  {
    /* MOD_VEX_0FAE_REG_3 */
    { VEX_LEN_TABLE (VEX_LEN_0FAE_R_3_M_0) },
  },
  {
    /* MOD_VEX_0FD7 */
    { Bad_Opcode },
    { "vpmovmskb",	{ Gdq, XS }, PREFIX_DATA },
  },
  {
    /* MOD_VEX_0FE7 */
    { "vmovntdq",	{ Mx, XM }, PREFIX_DATA },
  },
  {
    /* MOD_VEX_0FF0_PREFIX_3 */
    { "vlddqu",		{ XM, M }, 0 },
  },
  {
    /* MOD_VEX_0F381A */
    { VEX_LEN_TABLE (VEX_LEN_0F381A_M_0) },
  },
  {
    /* MOD_VEX_0F382A */
    { "vmovntdqa",	{ XM, Mx }, PREFIX_DATA },
  },
  {
    /* MOD_VEX_0F382C */
    { VEX_W_TABLE (VEX_W_0F382C_M_0) },
  },
  {
    /* MOD_VEX_0F382D */
    { VEX_W_TABLE (VEX_W_0F382D_M_0) },
  },
  {
    /* MOD_VEX_0F382E */
    { VEX_W_TABLE (VEX_W_0F382E_M_0) },
  },
  {
    /* MOD_VEX_0F382F */
    { VEX_W_TABLE (VEX_W_0F382F_M_0) },
  },
  {
    /* MOD_VEX_0F3849_X86_64_L_0_W_0 */
    { PREFIX_TABLE (PREFIX_VEX_0F3849_X86_64_L_0_W_0_M_0) },
    { PREFIX_TABLE (PREFIX_VEX_0F3849_X86_64_L_0_W_0_M_1) },
  },
  {
    /* MOD_VEX_0F384B_X86_64_L_0_W_0 */
    { PREFIX_TABLE (PREFIX_VEX_0F384B_X86_64_L_0_W_0_M_0) },
  },
  {
    /* MOD_VEX_0F385A */
    { VEX_LEN_TABLE (VEX_LEN_0F385A_M_0) },
  },
  {
    /* MOD_VEX_0F385C_X86_64 */
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F385C_X86_64_M_1) },
  },
  {
    /* MOD_VEX_0F385E_X86_64 */
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F385E_X86_64_M_1) },
  },
  {
    /* MOD_VEX_0F386C_X86_64 */
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0F386C_X86_64_M_1) },
  },
  {
    /* MOD_VEX_0F388C */
    { "vpmaskmov%DQ",	{ XM, Vex, Mx }, PREFIX_DATA },
  },
  {
    /* MOD_VEX_0F388E */
    { "vpmaskmov%DQ",	{ Mx, Vex, XM }, PREFIX_DATA },
  },
  {
    /* MOD_VEX_0F3A30_L_0 */
    { Bad_Opcode },
    { "kshiftr%BW",	{ MaskG, MaskE, Ib }, PREFIX_DATA },
  },
  {
    /* MOD_VEX_0F3A31_L_0 */
    { Bad_Opcode },
    { "kshiftr%DQ",	{ MaskG, MaskE, Ib }, PREFIX_DATA },
  },
  {
    /* MOD_VEX_0F3A32_L_0 */
    { Bad_Opcode },
    { "kshiftl%BW",	{ MaskG, MaskE, Ib }, PREFIX_DATA },
  },
  {
    /* MOD_VEX_0F3A33_L_0 */
    { Bad_Opcode },
    { "kshiftl%DQ",	{ MaskG, MaskE, Ib }, PREFIX_DATA },
  },
  {
    /* MOD_XOP_09_12 */
    { Bad_Opcode },
    { VEX_LEN_TABLE (VEX_LEN_0FXOP_09_12_M_1) },
  },

#include "i386-dis-evex-mod.h"
};

static const struct dis386 rm_table[][8] = {
  {
    /* RM_C6_REG_7 */
    { "xabort",		{ Skip_MODRM, Ib }, 0 },
  },
  {
    /* RM_C7_REG_7 */
    { "xbeginT",	{ Skip_MODRM, Jdqw }, 0 },
  },
  {
    /* RM_0F01_REG_0 */
    { "enclv",		{ Skip_MODRM }, 0 },
    { "vmcall",		{ Skip_MODRM }, 0 },
    { "vmlaunch",	{ Skip_MODRM }, 0 },
    { "vmresume",	{ Skip_MODRM }, 0 },
    { "vmxoff",		{ Skip_MODRM }, 0 },
    { "pconfig",	{ Skip_MODRM }, 0 },
    { PREFIX_TABLE (PREFIX_0F01_REG_0_MOD_3_RM_6) },
  },
  {
    /* RM_0F01_REG_1 */
    { "monitor",	{ { OP_Monitor, 0 } }, 0 },
    { "mwait",		{ { OP_Mwait, 0 } }, 0 },
    { PREFIX_TABLE (PREFIX_0F01_REG_1_RM_2) },
    { "stac",		{ Skip_MODRM }, 0 },
    { PREFIX_TABLE (PREFIX_0F01_REG_1_RM_4) },
    { PREFIX_TABLE (PREFIX_0F01_REG_1_RM_5) },
    { PREFIX_TABLE (PREFIX_0F01_REG_1_RM_6) },
    { PREFIX_TABLE (PREFIX_0F01_REG_1_RM_7) },
  },
  {
    /* RM_0F01_REG_2 */
    { "xgetbv",		{ Skip_MODRM }, 0 },
    { "xsetbv",		{ Skip_MODRM }, 0 },
    { Bad_Opcode },
    { Bad_Opcode },
    { "vmfunc",		{ Skip_MODRM }, 0 },
    { "xend",		{ Skip_MODRM }, 0 },
    { "xtest",		{ Skip_MODRM }, 0 },
    { "enclu",		{ Skip_MODRM }, 0 },
  },
  {
    /* RM_0F01_REG_3 */
    { "vmrun",		{ Skip_MODRM }, 0 },
    { PREFIX_TABLE (PREFIX_0F01_REG_3_RM_1) },
    { "vmload",		{ Skip_MODRM }, 0 },
    { "vmsave",		{ Skip_MODRM }, 0 },
    { "stgi",		{ Skip_MODRM }, 0 },
    { "clgi",		{ Skip_MODRM }, 0 },
    { "skinit",		{ Skip_MODRM }, 0 },
    { "invlpga",	{ Skip_MODRM }, 0 },
  },
  {
    /* RM_0F01_REG_5_MOD_3 */
    { PREFIX_TABLE (PREFIX_0F01_REG_5_MOD_3_RM_0) },
    { PREFIX_TABLE (PREFIX_0F01_REG_5_MOD_3_RM_1) },
    { PREFIX_TABLE (PREFIX_0F01_REG_5_MOD_3_RM_2) },
    { Bad_Opcode },
    { PREFIX_TABLE (PREFIX_0F01_REG_5_MOD_3_RM_4) },
    { PREFIX_TABLE (PREFIX_0F01_REG_5_MOD_3_RM_5) },
    { PREFIX_TABLE (PREFIX_0F01_REG_5_MOD_3_RM_6) },
    { PREFIX_TABLE (PREFIX_0F01_REG_5_MOD_3_RM_7) },
  },
  {
    /* RM_0F01_REG_7_MOD_3 */
    { "swapgs",		{ Skip_MODRM }, 0  },
    { "rdtscp",		{ Skip_MODRM }, 0  },
    { PREFIX_TABLE (PREFIX_0F01_REG_7_MOD_3_RM_2) },
    { "mwaitx",		{ { OP_Mwait, eBX_reg } }, PREFIX_OPCODE },
    { "clzero",		{ Skip_MODRM }, 0  },
    { PREFIX_TABLE (PREFIX_0F01_REG_7_MOD_3_RM_5) },
    { PREFIX_TABLE (PREFIX_0F01_REG_7_MOD_3_RM_6) },
    { PREFIX_TABLE (PREFIX_0F01_REG_7_MOD_3_RM_7) },
  },
  {
    /* RM_0F1E_P_1_MOD_3_REG_7 */
    { "nopQ",		{ Ev }, PREFIX_IGNORED },
    { "nopQ",		{ Ev }, PREFIX_IGNORED },
    { "endbr64",	{ Skip_MODRM }, 0 },
    { "endbr32",	{ Skip_MODRM }, 0 },
    { "nopQ",		{ Ev }, PREFIX_IGNORED },
    { "nopQ",		{ Ev }, PREFIX_IGNORED },
    { "nopQ",		{ Ev }, PREFIX_IGNORED },
    { "nopQ",		{ Ev }, PREFIX_IGNORED },
  },
  {
    /* RM_0FAE_REG_6_MOD_3 */
    { "mfence",		{ Skip_MODRM }, 0 },
  },
  {
    /* RM_0FAE_REG_7_MOD_3 */
    { "sfence",		{ Skip_MODRM }, 0 },
  },
  {
    /* RM_0F3A0F_P_1_MOD_3_REG_0 */
    { "hreset",		{ Skip_MODRM, Ib }, 0 },
  },
  {
    /* RM_VEX_0F3849_X86_64_L_0_W_0_M_1_P_0_R_0 */
    { "tilerelease",	{ Skip_MODRM }, 0 },
  },
  {
    /* RM_VEX_0F3849_X86_64_L_0_W_0_M_1_P_3 */
    { "tilezero",	{ TMM, Skip_MODRM }, 0 },
  },
};

#define INTERNAL_DISASSEMBLER_ERROR _("<internal disassembler error>")

/* The values used here must be non-zero, fit in 'unsigned char', and not be
   in conflict with actual prefix opcodes.  */
#define REP_PREFIX	0x01
#define XACQUIRE_PREFIX	0x02
#define XRELEASE_PREFIX	0x03
#define BND_PREFIX	0x04
#define NOTRACK_PREFIX	0x05

static enum {
  ckp_okay,
  ckp_bogus,
  ckp_fetch_error,
}
ckprefix (instr_info *ins)
{
  int i, length;
  uint8_t newrex;

  i = 0;
  length = 0;
  /* The maximum instruction length is 15bytes.  */
  while (length < MAX_CODE_LENGTH - 1)
    {
      if (!fetch_code (ins->info, ins->codep + 1))
	return ckp_fetch_error;
      newrex = 0;
      switch (*ins->codep)
	{
	/* REX prefixes family.  */
	case 0x40:
	case 0x41:
	case 0x42:
	case 0x43:
	case 0x44:
	case 0x45:
	case 0x46:
	case 0x47:
	case 0x48:
	case 0x49:
	case 0x4a:
	case 0x4b:
	case 0x4c:
	case 0x4d:
	case 0x4e:
	case 0x4f:
	  if (ins->address_mode == mode_64bit)
	    newrex = *ins->codep;
	  else
	    return ckp_okay;
	  ins->last_rex_prefix = i;
	  break;
	case 0xf3:
	  ins->prefixes |= PREFIX_REPZ;
	  ins->last_repz_prefix = i;
	  break;
	case 0xf2:
	  ins->prefixes |= PREFIX_REPNZ;
	  ins->last_repnz_prefix = i;
	  break;
	case 0xf0:
	  ins->prefixes |= PREFIX_LOCK;
	  ins->last_lock_prefix = i;
	  break;
	case 0x2e:
	  ins->prefixes |= PREFIX_CS;
	  ins->last_seg_prefix = i;
	  if (ins->address_mode != mode_64bit)
	    ins->active_seg_prefix = PREFIX_CS;
	  break;
	case 0x36:
	  ins->prefixes |= PREFIX_SS;
	  ins->last_seg_prefix = i;
	  if (ins->address_mode != mode_64bit)
	    ins->active_seg_prefix = PREFIX_SS;
	  break;
	case 0x3e:
	  ins->prefixes |= PREFIX_DS;
	  ins->last_seg_prefix = i;
	  if (ins->address_mode != mode_64bit)
	    ins->active_seg_prefix = PREFIX_DS;
	  break;
	case 0x26:
	  ins->prefixes |= PREFIX_ES;
	  ins->last_seg_prefix = i;
	  if (ins->address_mode != mode_64bit)
	    ins->active_seg_prefix = PREFIX_ES;
	  break;
	case 0x64:
	  ins->prefixes |= PREFIX_FS;
	  ins->last_seg_prefix = i;
	  ins->active_seg_prefix = PREFIX_FS;
	  break;
	case 0x65:
	  ins->prefixes |= PREFIX_GS;
	  ins->last_seg_prefix = i;
	  ins->active_seg_prefix = PREFIX_GS;
	  break;
	case 0x66:
	  ins->prefixes |= PREFIX_DATA;
	  ins->last_data_prefix = i;
	  break;
	case 0x67:
	  ins->prefixes |= PREFIX_ADDR;
	  ins->last_addr_prefix = i;
	  break;
	case FWAIT_OPCODE:
	  /* fwait is really an instruction.  If there are prefixes
	     before the fwait, they belong to the fwait, *not* to the
	     following instruction.  */
	  ins->fwait_prefix = i;
	  if (ins->prefixes || ins->rex)
	    {
	      ins->prefixes |= PREFIX_FWAIT;
	      ins->codep++;
	      /* This ensures that the previous REX prefixes are noticed
		 as unused prefixes, as in the return case below.  */
	      return ins->rex ? ckp_bogus : ckp_okay;
	    }
	  ins->prefixes = PREFIX_FWAIT;
	  break;
	default:
	  return ckp_okay;
	}
      /* Rex is ignored when followed by another prefix.  */
      if (ins->rex)
	return ckp_bogus;
      if (*ins->codep != FWAIT_OPCODE)
	ins->all_prefixes[i++] = *ins->codep;
      ins->rex = newrex;
      ins->codep++;
      length++;
    }
  return ckp_bogus;
}

/* Return the name of the prefix byte PREF, or NULL if PREF is not a
   prefix byte.  */

static const char *
prefix_name (enum address_mode mode, uint8_t pref, int sizeflag)
{
  static const char *rexes [16] =
    {
      "rex",		/* 0x40 */
      "rex.B",		/* 0x41 */
      "rex.X",		/* 0x42 */
      "rex.XB",		/* 0x43 */
      "rex.R",		/* 0x44 */
      "rex.RB",		/* 0x45 */
      "rex.RX",		/* 0x46 */
      "rex.RXB",	/* 0x47 */
      "rex.W",		/* 0x48 */
      "rex.WB",		/* 0x49 */
      "rex.WX",		/* 0x4a */
      "rex.WXB",	/* 0x4b */
      "rex.WR",		/* 0x4c */
      "rex.WRB",	/* 0x4d */
      "rex.WRX",	/* 0x4e */
      "rex.WRXB",	/* 0x4f */
    };

  switch (pref)
    {
    /* REX prefixes family.  */
    case 0x40:
    case 0x41:
    case 0x42:
    case 0x43:
    case 0x44:
    case 0x45:
    case 0x46:
    case 0x47:
    case 0x48:
    case 0x49:
    case 0x4a:
    case 0x4b:
    case 0x4c:
    case 0x4d:
    case 0x4e:
    case 0x4f:
      return rexes [pref - 0x40];
    case 0xf3:
      return "repz";
    case 0xf2:
      return "repnz";
    case 0xf0:
      return "lock";
    case 0x2e:
      return "cs";
    case 0x36:
      return "ss";
    case 0x3e:
      return "ds";
    case 0x26:
      return "es";
    case 0x64:
      return "fs";
    case 0x65:
      return "gs";
    case 0x66:
      return (sizeflag & DFLAG) ? "data16" : "data32";
    case 0x67:
      if (mode == mode_64bit)
	return (sizeflag & AFLAG) ? "addr32" : "addr64";
      else
	return (sizeflag & AFLAG) ? "addr16" : "addr32";
    case FWAIT_OPCODE:
      return "fwait";
    case REP_PREFIX:
      return "rep";
    case XACQUIRE_PREFIX:
      return "xacquire";
    case XRELEASE_PREFIX:
      return "xrelease";
    case BND_PREFIX:
      return "bnd";
    case NOTRACK_PREFIX:
      return "notrack";
    default:
      return NULL;
    }
}

void
print_i386_disassembler_options (FILE *stream)
{
  fprintf (stream, _("\n\
The following i386/x86-64 specific disassembler options are supported for use\n\
with the -M switch (multiple options should be separated by commas):\n"));

  fprintf (stream, _("  x86-64      Disassemble in 64bit mode\n"));
  fprintf (stream, _("  i386        Disassemble in 32bit mode\n"));
  fprintf (stream, _("  i8086       Disassemble in 16bit mode\n"));
  fprintf (stream, _("  att         Display instruction in AT&T syntax\n"));
  fprintf (stream, _("  intel       Display instruction in Intel syntax\n"));
  fprintf (stream, _("  att-mnemonic\n"
		     "              Display instruction in AT&T mnemonic\n"));
  fprintf (stream, _("  intel-mnemonic\n"
		     "              Display instruction in Intel mnemonic\n"));
  fprintf (stream, _("  addr64      Assume 64bit address size\n"));
  fprintf (stream, _("  addr32      Assume 32bit address size\n"));
  fprintf (stream, _("  addr16      Assume 16bit address size\n"));
  fprintf (stream, _("  data32      Assume 32bit data size\n"));
  fprintf (stream, _("  data16      Assume 16bit data size\n"));
  fprintf (stream, _("  suffix      Always display instruction suffix in AT&T syntax\n"));
  fprintf (stream, _("  amd64       Display instruction in AMD64 ISA\n"));
  fprintf (stream, _("  intel64     Display instruction in Intel64 ISA\n"));
}

/* Bad opcode.  */
static const struct dis386 bad_opcode = { "(bad)", { XX }, 0 };

/* Fetch error indicator.  */
static const struct dis386 err_opcode = { NULL, { XX }, 0 };

/* Get a pointer to struct dis386 with a valid name.  */

static const struct dis386 *
get_valid_dis386 (const struct dis386 *dp, instr_info *ins)
{
  int vindex, vex_table_index;

  if (dp->name != NULL)
    return dp;

  switch (dp->op[0].bytemode)
    {
    case USE_REG_TABLE:
      dp = &reg_table[dp->op[1].bytemode][ins->modrm.reg];
      break;

    case USE_MOD_TABLE:
      vindex = ins->modrm.mod == 0x3 ? 1 : 0;
      dp = &mod_table[dp->op[1].bytemode][vindex];
      break;

    case USE_RM_TABLE:
      dp = &rm_table[dp->op[1].bytemode][ins->modrm.rm];
      break;

    case USE_PREFIX_TABLE:
      if (ins->need_vex)
	{
	  /* The prefix in VEX is implicit.  */
	  switch (ins->vex.prefix)
	    {
	    case 0:
	      vindex = 0;
	      break;
	    case REPE_PREFIX_OPCODE:
	      vindex = 1;
	      break;
	    case DATA_PREFIX_OPCODE:
	      vindex = 2;
	      break;
	    case REPNE_PREFIX_OPCODE:
	      vindex = 3;
	      break;
	    default:
	      abort ();
	      break;
	    }
	}
      else
	{
	  int last_prefix = -1;
	  int prefix = 0;
	  vindex = 0;
	  /* We check PREFIX_REPNZ and PREFIX_REPZ before PREFIX_DATA.
	     When there are multiple PREFIX_REPNZ and PREFIX_REPZ, the
	     last one wins.  */
	  if ((ins->prefixes & (PREFIX_REPZ | PREFIX_REPNZ)) != 0)
	    {
	      if (ins->last_repz_prefix > ins->last_repnz_prefix)
		{
		  vindex = 1;
		  prefix = PREFIX_REPZ;
		  last_prefix = ins->last_repz_prefix;
		}
	      else
		{
		  vindex = 3;
		  prefix = PREFIX_REPNZ;
		  last_prefix = ins->last_repnz_prefix;
		}

	      /* Check if prefix should be ignored.  */
	      if ((((prefix_table[dp->op[1].bytemode][vindex].prefix_requirement
		     & PREFIX_IGNORED) >> PREFIX_IGNORED_SHIFT)
		   & prefix) != 0
		  && !prefix_table[dp->op[1].bytemode][vindex].name)
		vindex = 0;
	    }

	  if (vindex == 0 && (ins->prefixes & PREFIX_DATA) != 0)
	    {
	      vindex = 2;
	      prefix = PREFIX_DATA;
	      last_prefix = ins->last_data_prefix;
	    }

	  if (vindex != 0)
	    {
	      ins->used_prefixes |= prefix;
	      ins->all_prefixes[last_prefix] = 0;
	    }
	}
      dp = &prefix_table[dp->op[1].bytemode][vindex];
      break;

    case USE_X86_64_TABLE:
      vindex = ins->address_mode == mode_64bit ? 1 : 0;
      dp = &x86_64_table[dp->op[1].bytemode][vindex];
      break;

    case USE_3BYTE_TABLE:
      if (!fetch_code (ins->info, ins->codep + 2))
	return &err_opcode;
      vindex = *ins->codep++;
      dp = &three_byte_table[dp->op[1].bytemode][vindex];
      ins->end_codep = ins->codep;
      if (!fetch_modrm (ins))
	return &err_opcode;
      break;

    case USE_VEX_LEN_TABLE:
      if (!ins->need_vex)
	abort ();

      switch (ins->vex.length)
	{
	case 128:
	  vindex = 0;
	  break;
	case 512:
	  /* This allows re-using in particular table entries where only
	     128-bit operand size (VEX.L=0 / EVEX.L'L=0) are valid.  */
	  if (ins->vex.evex)
	    {
	case 256:
	      vindex = 1;
	      break;
	    }
	/* Fall through.  */
	default:
	  abort ();
	  break;
	}

      dp = &vex_len_table[dp->op[1].bytemode][vindex];
      break;

    case USE_EVEX_LEN_TABLE:
      if (!ins->vex.evex)
	abort ();

      switch (ins->vex.length)
	{
	case 128:
	  vindex = 0;
	  break;
	case 256:
	  vindex = 1;
	  break;
	case 512:
	  vindex = 2;
	  break;
	default:
	  abort ();
	  break;
	}

      dp = &evex_len_table[dp->op[1].bytemode][vindex];
      break;

    case USE_XOP_8F_TABLE:
      if (!fetch_code (ins->info, ins->codep + 3))
	return &err_opcode;
      ins->rex = ~(*ins->codep >> 5) & 0x7;

      /* VEX_TABLE_INDEX is the mmmmm part of the XOP byte 1 "RCB.mmmmm".  */
      switch ((*ins->codep & 0x1f))
	{
	default:
	  dp = &bad_opcode;
	  return dp;
	case 0x8:
	  vex_table_index = XOP_08;
	  break;
	case 0x9:
	  vex_table_index = XOP_09;
	  break;
	case 0xa:
	  vex_table_index = XOP_0A;
	  break;
	}
      ins->codep++;
      ins->vex.w = *ins->codep & 0x80;
      if (ins->vex.w && ins->address_mode == mode_64bit)
	ins->rex |= REX_W;

      ins->vex.register_specifier = (~(*ins->codep >> 3)) & 0xf;
      if (ins->address_mode != mode_64bit)
	{
	  /* In 16/32-bit mode REX_B is silently ignored.  */
	  ins->rex &= ~REX_B;
	}

      ins->vex.length = (*ins->codep & 0x4) ? 256 : 128;
      switch ((*ins->codep & 0x3))
	{
	case 0:
	  break;
	case 1:
	  ins->vex.prefix = DATA_PREFIX_OPCODE;
	  break;
	case 2:
	  ins->vex.prefix = REPE_PREFIX_OPCODE;
	  break;
	case 3:
	  ins->vex.prefix = REPNE_PREFIX_OPCODE;
	  break;
	}
      ins->need_vex = true;
      ins->codep++;
      vindex = *ins->codep++;
      dp = &xop_table[vex_table_index][vindex];

      ins->end_codep = ins->codep;
      if (!fetch_modrm (ins))
	return &err_opcode;

      /* No XOP encoding so far allows for a non-zero embedded prefix. Avoid
	 having to decode the bits for every otherwise valid encoding.  */
      if (ins->vex.prefix)
	return &bad_opcode;
      break;

    case USE_VEX_C4_TABLE:
      /* VEX prefix.  */
      if (!fetch_code (ins->info, ins->codep + 3))
	return &err_opcode;
      ins->rex = ~(*ins->codep >> 5) & 0x7;
      switch ((*ins->codep & 0x1f))
	{
	default:
	  dp = &bad_opcode;
	  return dp;
	case 0x1:
	  vex_table_index = VEX_0F;
	  break;
	case 0x2:
	  vex_table_index = VEX_0F38;
	  break;
	case 0x3:
	  vex_table_index = VEX_0F3A;
	  break;
	}
      ins->codep++;
      ins->vex.w = *ins->codep & 0x80;
      if (ins->address_mode == mode_64bit)
	{
	  if (ins->vex.w)
	    ins->rex |= REX_W;
	}
      else
	{
	  /* For the 3-byte VEX prefix in 32-bit mode, the REX_B bit
	     is ignored, other REX bits are 0 and the highest bit in
	     VEX.vvvv is also ignored (but we mustn't clear it here).  */
	  ins->rex = 0;
	}
      ins->vex.register_specifier = (~(*ins->codep >> 3)) & 0xf;
      ins->vex.length = (*ins->codep & 0x4) ? 256 : 128;
      switch ((*ins->codep & 0x3))
	{
	case 0:
	  break;
	case 1:
	  ins->vex.prefix = DATA_PREFIX_OPCODE;
	  break;
	case 2:
	  ins->vex.prefix = REPE_PREFIX_OPCODE;
	  break;
	case 3:
	  ins->vex.prefix = REPNE_PREFIX_OPCODE;
	  break;
	}
      ins->need_vex = true;
      ins->codep++;
      vindex = *ins->codep++;
      dp = &vex_table[vex_table_index][vindex];
      ins->end_codep = ins->codep;
      /* There is no MODRM byte for VEX0F 77.  */
      if ((vex_table_index != VEX_0F || vindex != 0x77)
	  && !fetch_modrm (ins))
	return &err_opcode;
      break;

    case USE_VEX_C5_TABLE:
      /* VEX prefix.  */
      if (!fetch_code (ins->info, ins->codep + 2))
	return &err_opcode;
      ins->rex = (*ins->codep & 0x80) ? 0 : REX_R;

      /* For the 2-byte VEX prefix in 32-bit mode, the highest bit in
	 VEX.vvvv is 1.  */
      ins->vex.register_specifier = (~(*ins->codep >> 3)) & 0xf;
      ins->vex.length = (*ins->codep & 0x4) ? 256 : 128;
      switch ((*ins->codep & 0x3))
	{
	case 0:
	  break;
	case 1:
	  ins->vex.prefix = DATA_PREFIX_OPCODE;
	  break;
	case 2:
	  ins->vex.prefix = REPE_PREFIX_OPCODE;
	  break;
	case 3:
	  ins->vex.prefix = REPNE_PREFIX_OPCODE;
	  break;
	}
      ins->need_vex = true;
      ins->codep++;
      vindex = *ins->codep++;
      dp = &vex_table[dp->op[1].bytemode][vindex];
      ins->end_codep = ins->codep;
      /* There is no MODRM byte for VEX 77.  */
      if (vindex != 0x77 && !fetch_modrm (ins))
	return &err_opcode;
      break;

    case USE_VEX_W_TABLE:
      if (!ins->need_vex)
	abort ();

      dp = &vex_w_table[dp->op[1].bytemode][ins->vex.w];
      break;

    case USE_EVEX_TABLE:
      ins->two_source_ops = false;
      /* EVEX prefix.  */
      ins->vex.evex = true;
      if (!fetch_code (ins->info, ins->codep + 4))
	return &err_opcode;
      /* The first byte after 0x62.  */
      ins->rex = ~(*ins->codep >> 5) & 0x7;
      ins->vex.r = *ins->codep & 0x10;
      switch ((*ins->codep & 0xf))
	{
	default:
	  return &bad_opcode;
	case 0x1:
	  vex_table_index = EVEX_0F;
	  break;
	case 0x2:
	  vex_table_index = EVEX_0F38;
	  break;
	case 0x3:
	  vex_table_index = EVEX_0F3A;
	  break;
	case 0x5:
	  vex_table_index = EVEX_MAP5;
	  break;
	case 0x6:
	  vex_table_index = EVEX_MAP6;
	  break;
	}

      /* The second byte after 0x62.  */
      ins->codep++;
      ins->vex.w = *ins->codep & 0x80;
      if (ins->vex.w && ins->address_mode == mode_64bit)
	ins->rex |= REX_W;

      ins->vex.register_specifier = (~(*ins->codep >> 3)) & 0xf;

      /* The U bit.  */
      if (!(*ins->codep & 0x4))
	return &bad_opcode;

      switch ((*ins->codep & 0x3))
	{
	case 0:
	  break;
	case 1:
	  ins->vex.prefix = DATA_PREFIX_OPCODE;
	  break;
	case 2:
	  ins->vex.prefix = REPE_PREFIX_OPCODE;
	  break;
	case 3:
	  ins->vex.prefix = REPNE_PREFIX_OPCODE;
	  break;
	}

      /* The third byte after 0x62.  */
      ins->codep++;

      /* Remember the static rounding bits.  */
      ins->vex.ll = (*ins->codep >> 5) & 3;
      ins->vex.b = *ins->codep & 0x10;

      ins->vex.v = *ins->codep & 0x8;
      ins->vex.mask_register_specifier = *ins->codep & 0x7;
      ins->vex.zeroing = *ins->codep & 0x80;

      if (ins->address_mode != mode_64bit)
	{
	  /* In 16/32-bit mode silently ignore following bits.  */
	  ins->rex &= ~REX_B;
	  ins->vex.r = true;
	}

      ins->need_vex = true;
      ins->codep++;
      vindex = *ins->codep++;
      dp = &evex_table[vex_table_index][vindex];
      ins->end_codep = ins->codep;
      if (!fetch_modrm (ins))
	return &err_opcode;

      /* Set vector length.  */
      if (ins->modrm.mod == 3 && ins->vex.b)
	ins->vex.length = 512;
      else
	{
	  switch (ins->vex.ll)
	    {
	    case 0x0:
	      ins->vex.length = 128;
	      break;
	    case 0x1:
	      ins->vex.length = 256;
	      break;
	    case 0x2:
	      ins->vex.length = 512;
	      break;
	    default:
	      return &bad_opcode;
	    }
	}
      break;

    case 0:
      dp = &bad_opcode;
      break;

    default:
      abort ();
    }

  if (dp->name != NULL)
    return dp;
  else
    return get_valid_dis386 (dp, ins);
}

static bool
get_sib (instr_info *ins, int sizeflag)
{
  /* If modrm.mod == 3, operand must be register.  */
  if (ins->need_modrm
      && ((sizeflag & AFLAG) || ins->address_mode == mode_64bit)
      && ins->modrm.mod != 3
      && ins->modrm.rm == 4)
    {
      if (!fetch_code (ins->info, ins->codep + 2))
	return false;
      ins->sib.index = (ins->codep[1] >> 3) & 7;
      ins->sib.scale = (ins->codep[1] >> 6) & 3;
      ins->sib.base = ins->codep[1] & 7;
      ins->has_sib = true;
    }
  else
    ins->has_sib = false;

  return true;
}

/* Like oappend_with_style (below) but always with text style.  */

static void
oappend (instr_info *ins, const char *s)
{
  oappend_with_style (ins, s, dis_style_text);
}

/* Like oappend (above), but S is a string starting with '%'.  In
   Intel syntax, the '%' is elided.  */

static void
oappend_register (instr_info *ins, const char *s)
{
  oappend_with_style (ins, s + ins->intel_syntax, dis_style_register);
}

/* Wrap around a call to INS->info->fprintf_styled_func, printing FMT.
   STYLE is the default style to use in the fprintf_styled_func calls,
   however, FMT might include embedded style markers (see oappend_style),
   these embedded markers are not printed, but instead change the style
   used in the next fprintf_styled_func call.  */

static void ATTRIBUTE_PRINTF_3
i386_dis_printf (const disassemble_info *info, enum disassembler_style style,
		 const char *fmt, ...)
{
  va_list ap;
  enum disassembler_style curr_style = style;
  const char *start, *curr;
  char staging_area[40];

  va_start (ap, fmt);
  /* In particular print_insn()'s processing of op_txt[] can hand rather long
     strings here.  Bypass vsnprintf() in such cases to avoid capacity issues
     with the staging area.  */
  if (strcmp (fmt, "%s"))
    {
      int res = vsnprintf (staging_area, sizeof (staging_area), fmt, ap);

      va_end (ap);

      if (res < 0)
	return;

      if ((size_t) res >= sizeof (staging_area))
	abort ();

      start = curr = staging_area;
    }
  else
    {
      start = curr = va_arg (ap, const char *);
      va_end (ap);
    }

  do
    {
      if (*curr == '\0'
	  || (*curr == STYLE_MARKER_CHAR
	      && ISXDIGIT (*(curr + 1))
	      && *(curr + 2) == STYLE_MARKER_CHAR))
	{
	  /* Output content between our START position and CURR.  */
	  int len = curr - start;
	  int n = (*info->fprintf_styled_func) (info->stream, curr_style,
						"%.*s", len, start);
	  if (n < 0)
	    break;

	  if (*curr == '\0')
	    break;

	  /* Skip over the initial STYLE_MARKER_CHAR.  */
	  ++curr;

	  /* Update the CURR_STYLE.  As there are less than 16 styles, it
	     is possible, that if the input is corrupted in some way, that
	     we might set CURR_STYLE to an invalid value.  Don't worry
	     though, we check for this situation.  */
	  if (*curr >= '0' && *curr <= '9')
	    curr_style = (enum disassembler_style) (*curr - '0');
	  else if (*curr >= 'a' && *curr <= 'f')
	    curr_style = (enum disassembler_style) (*curr - 'a' + 10);
	  else
	    curr_style = dis_style_text;

	  /* Check for an invalid style having been selected.  This should
	     never happen, but it doesn't hurt to be a little paranoid.  */
	  if (curr_style > dis_style_comment_start)
	    curr_style = dis_style_text;

	  /* Skip the hex character, and the closing STYLE_MARKER_CHAR.  */
	  curr += 2;

	  /* Reset the START to after the style marker.  */
	  start = curr;
	}
      else
	++curr;
    }
  while (true);
}

static int
print_insn (bfd_vma pc, disassemble_info *info, int intel_syntax)
{
  const struct dis386 *dp;
  int i;
  int ret;
  char *op_txt[MAX_OPERANDS];
  int needcomma;
  bool intel_swap_2_3;
  int sizeflag, orig_sizeflag;
  const char *p;
  struct dis_private priv;
  int prefix_length;
  int op_count;
  instr_info ins = {
    .info = info,
    .intel_syntax = intel_syntax >= 0
		    ? intel_syntax
		    : (info->mach & bfd_mach_i386_intel_syntax) != 0,
    .intel_mnemonic = !SYSV386_COMPAT,
    .op_index[0 ... MAX_OPERANDS - 1] = -1,
    .start_pc = pc,
    .start_codep = priv.the_buffer,
    .codep = priv.the_buffer,
    .obufp = ins.obuf,
    .last_lock_prefix = -1,
    .last_repz_prefix = -1,
    .last_repnz_prefix = -1,
    .last_data_prefix = -1,
    .last_addr_prefix = -1,
    .last_rex_prefix = -1,
    .last_seg_prefix = -1,
    .fwait_prefix = -1,
  };
  char op_out[MAX_OPERANDS][MAX_OPERAND_BUFFER_SIZE];

  priv.orig_sizeflag = AFLAG | DFLAG;
  if ((info->mach & bfd_mach_i386_i386) != 0)
    ins.address_mode = mode_32bit;
  else if (info->mach == bfd_mach_i386_i8086)
    {
      ins.address_mode = mode_16bit;
      priv.orig_sizeflag = 0;
    }
  else
    ins.address_mode = mode_64bit;

  for (p = info->disassembler_options; p != NULL;)
    {
      if (startswith (p, "amd64"))
	ins.isa64 = amd64;
      else if (startswith (p, "intel64"))
	ins.isa64 = intel64;
      else if (startswith (p, "x86-64"))
	{
	  ins.address_mode = mode_64bit;
	  priv.orig_sizeflag |= AFLAG | DFLAG;
	}
      else if (startswith (p, "i386"))
	{
	  ins.address_mode = mode_32bit;
	  priv.orig_sizeflag |= AFLAG | DFLAG;
	}
      else if (startswith (p, "i8086"))
	{
	  ins.address_mode = mode_16bit;
	  priv.orig_sizeflag &= ~(AFLAG | DFLAG);
	}
      else if (startswith (p, "intel"))
	{
	  ins.intel_syntax = 1;
	  if (startswith (p + 5, "-mnemonic"))
	    ins.intel_mnemonic = true;
	}
      else if (startswith (p, "att"))
	{
	  ins.intel_syntax = 0;
	  if (startswith (p + 3, "-mnemonic"))
	    ins.intel_mnemonic = false;
	}
      else if (startswith (p, "addr"))
	{
	  if (ins.address_mode == mode_64bit)
	    {
	      if (p[4] == '3' && p[5] == '2')
		priv.orig_sizeflag &= ~AFLAG;
	      else if (p[4] == '6' && p[5] == '4')
		priv.orig_sizeflag |= AFLAG;
	    }
	  else
	    {
	      if (p[4] == '1' && p[5] == '6')
		priv.orig_sizeflag &= ~AFLAG;
	      else if (p[4] == '3' && p[5] == '2')
		priv.orig_sizeflag |= AFLAG;
	    }
	}
      else if (startswith (p, "data"))
	{
	  if (p[4] == '1' && p[5] == '6')
	    priv.orig_sizeflag &= ~DFLAG;
	  else if (p[4] == '3' && p[5] == '2')
	    priv.orig_sizeflag |= DFLAG;
	}
      else if (startswith (p, "suffix"))
	priv.orig_sizeflag |= SUFFIX_ALWAYS;

      p = strchr (p, ',');
      if (p != NULL)
	p++;
    }

  if (ins.address_mode == mode_64bit && sizeof (bfd_vma) < 8)
    {
      i386_dis_printf (info, dis_style_text, _("64-bit address is disabled"));
      return -1;
    }

  if (ins.intel_syntax)
    {
      ins.open_char = '[';
      ins.close_char = ']';
      ins.separator_char = '+';
      ins.scale_char = '*';
    }
  else
    {
      ins.open_char = '(';
      ins.close_char =  ')';
      ins.separator_char = ',';
      ins.scale_char = ',';
    }

  /* The output looks better if we put 7 bytes on a line, since that
     puts most long word instructions on a single line.  */
  info->bytes_per_line = 7;

  info->private_data = &priv;
  priv.fetched = 0;
  priv.insn_start = pc;

  for (i = 0; i < MAX_OPERANDS; ++i)
    {
      op_out[i][0] = 0;
      ins.op_out[i] = op_out[i];
    }

  sizeflag = priv.orig_sizeflag;

  switch (ckprefix (&ins))
    {
    case ckp_okay:
      break;

    case ckp_bogus:
      /* Too many prefixes or unused REX prefixes.  */
      for (i = 0;
	   i < (int) ARRAY_SIZE (ins.all_prefixes) && ins.all_prefixes[i];
	   i++)
	i386_dis_printf (info, dis_style_mnemonic, "%s%s",
			 (i == 0 ? "" : " "),
			 prefix_name (ins.address_mode, ins.all_prefixes[i],
				      sizeflag));
      ret = i;
      goto out;

    case ckp_fetch_error:
      goto fetch_error_out;
    }

  ins.nr_prefixes = ins.codep - ins.start_codep;

  if (!fetch_code (info, ins.codep + 1))
    {
    fetch_error_out:
      ret = fetch_error (&ins);
      goto out;
    }

  ins.two_source_ops = (*ins.codep == 0x62 || *ins.codep == 0xc8);

  if ((ins.prefixes & PREFIX_FWAIT)
      && (*ins.codep < 0xd8 || *ins.codep > 0xdf))
    {
      /* Handle ins.prefixes before fwait.  */
      for (i = 0; i < ins.fwait_prefix && ins.all_prefixes[i];
	   i++)
	i386_dis_printf (info, dis_style_mnemonic, "%s ",
			 prefix_name (ins.address_mode, ins.all_prefixes[i],
				      sizeflag));
      i386_dis_printf (info, dis_style_mnemonic, "fwait");
      ret = i + 1;
      goto out;
    }

  if (*ins.codep == 0x0f)
    {
      unsigned char threebyte;

      ins.codep++;
      if (!fetch_code (info, ins.codep + 1))
	goto fetch_error_out;
      threebyte = *ins.codep;
      dp = &dis386_twobyte[threebyte];
      ins.need_modrm = twobyte_has_modrm[threebyte];
      ins.codep++;
    }
  else
    {
      dp = &dis386[*ins.codep];
      ins.need_modrm = onebyte_has_modrm[*ins.codep];
      ins.codep++;
    }

  /* Save sizeflag for printing the extra ins.prefixes later before updating
     it for mnemonic and operand processing.  The prefix names depend
     only on the address mode.  */
  orig_sizeflag = sizeflag;
  if (ins.prefixes & PREFIX_ADDR)
    sizeflag ^= AFLAG;
  if ((ins.prefixes & PREFIX_DATA))
    sizeflag ^= DFLAG;

  ins.end_codep = ins.codep;
  if (ins.need_modrm && !fetch_modrm (&ins))
    goto fetch_error_out;

  if (dp->name == NULL && dp->op[0].bytemode == FLOATCODE)
    {
      if (!get_sib (&ins, sizeflag)
	  || !dofloat (&ins, sizeflag))
	goto fetch_error_out;
    }
  else
    {
      dp = get_valid_dis386 (dp, &ins);
      if (dp == &err_opcode)
	goto fetch_error_out;
      if (dp != NULL && putop (&ins, dp->name, sizeflag) == 0)
	{
	  if (!get_sib (&ins, sizeflag))
	    goto fetch_error_out;
	  for (i = 0; i < MAX_OPERANDS; ++i)
	    {
	      ins.obufp = ins.op_out[i];
	      ins.op_ad = MAX_OPERANDS - 1 - i;
	      if (dp->op[i].rtn
		  && !dp->op[i].rtn (&ins, dp->op[i].bytemode, sizeflag))
		goto fetch_error_out;
	      /* For EVEX instruction after the last operand masking
		 should be printed.  */
	      if (i == 0 && ins.vex.evex)
		{
		  /* Don't print {%k0}.  */
		  if (ins.vex.mask_register_specifier)
		    {
		      const char *reg_name
			= att_names_mask[ins.vex.mask_register_specifier];

		      oappend (&ins, "{");
		      oappend_register (&ins, reg_name);
		      oappend (&ins, "}");
		    }
		  if (ins.vex.zeroing)
		    oappend (&ins, "{z}");

		  /* S/G insns require a mask and don't allow
		     zeroing-masking.  */
		  if ((dp->op[0].bytemode == vex_vsib_d_w_dq_mode
		       || dp->op[0].bytemode == vex_vsib_q_w_dq_mode)
		      && (ins.vex.mask_register_specifier == 0
			  || ins.vex.zeroing))
		    oappend (&ins, "/(bad)");
		}
	    }

	  /* Check whether rounding control was enabled for an insn not
	     supporting it.  */
	  if (ins.modrm.mod == 3 && ins.vex.b
	      && !(ins.evex_used & EVEX_b_used))
	    {
	      for (i = 0; i < MAX_OPERANDS; ++i)
		{
		  ins.obufp = ins.op_out[i];
		  if (*ins.obufp)
		    continue;
		  oappend (&ins, names_rounding[ins.vex.ll]);
		  oappend (&ins, "bad}");
		  break;
		}
	    }
	}
    }

  /* Clear instruction information.  */
  info->insn_info_valid = 0;
  info->branch_delay_insns = 0;
  info->data_size = 0;
  info->insn_type = dis_noninsn;
  info->target = 0;
  info->target2 = 0;

  /* Reset jump operation indicator.  */
  ins.op_is_jump = false;
  {
    int jump_detection = 0;

    /* Extract flags.  */
    for (i = 0; i < MAX_OPERANDS; ++i)
      {
	if ((dp->op[i].rtn == OP_J)
	    || (dp->op[i].rtn == OP_indirE))
	  jump_detection |= 1;
	else if ((dp->op[i].rtn == BND_Fixup)
		 || (!dp->op[i].rtn && !dp->op[i].bytemode))
	  jump_detection |= 2;
	else if ((dp->op[i].bytemode == cond_jump_mode)
		 || (dp->op[i].bytemode == loop_jcxz_mode))
	  jump_detection |= 4;
      }

    /* Determine if this is a jump or branch.  */
    if ((jump_detection & 0x3) == 0x3)
      {
	ins.op_is_jump = true;
	if (jump_detection & 0x4)
	  info->insn_type = dis_condbranch;
	else
	  info->insn_type = (dp->name && !strncmp (dp->name, "call", 4))
	    ? dis_jsr : dis_branch;
      }
  }

  /* If VEX.vvvv and EVEX.vvvv are unused, they must be all 1s, which
     are all 0s in inverted form.  */
  if (ins.need_vex && ins.vex.register_specifier != 0)
    {
      i386_dis_printf (info, dis_style_text, "(bad)");
      ret = ins.end_codep - priv.the_buffer;
      goto out;
    }

  /* If EVEX.z is set, there must be an actual mask register in use.  */
  if (ins.vex.zeroing && ins.vex.mask_register_specifier == 0)
    {
      i386_dis_printf (info, dis_style_text, "(bad)");
      ret = ins.end_codep - priv.the_buffer;
      goto out;
    }

  switch (dp->prefix_requirement)
    {
    case PREFIX_DATA:
      /* If only the data prefix is marked as mandatory, its absence renders
	 the encoding invalid.  Most other PREFIX_OPCODE rules still apply.  */
      if (ins.need_vex ? !ins.vex.prefix : !(ins.prefixes & PREFIX_DATA))
	{
	  i386_dis_printf (info, dis_style_text, "(bad)");
	  ret = ins.end_codep - priv.the_buffer;
	  goto out;
	}
      ins.used_prefixes |= PREFIX_DATA;
      /* Fall through.  */
    case PREFIX_OPCODE:
      /* If the mandatory PREFIX_REPZ/PREFIX_REPNZ/PREFIX_DATA prefix is
	 unused, opcode is invalid.  Since the PREFIX_DATA prefix may be
	 used by putop and MMX/SSE operand and may be overridden by the
	 PREFIX_REPZ/PREFIX_REPNZ fix, we check the PREFIX_DATA prefix
	 separately.  */
      if (((ins.need_vex
	    ? ins.vex.prefix == REPE_PREFIX_OPCODE
	      || ins.vex.prefix == REPNE_PREFIX_OPCODE
	    : (ins.prefixes
	       & (PREFIX_REPZ | PREFIX_REPNZ)) != 0)
	   && (ins.used_prefixes
	       & (PREFIX_REPZ | PREFIX_REPNZ)) == 0)
	  || (((ins.need_vex
		? ins.vex.prefix == DATA_PREFIX_OPCODE
		: ((ins.prefixes
		    & (PREFIX_REPZ | PREFIX_REPNZ | PREFIX_DATA))
		   == PREFIX_DATA))
	       && (ins.used_prefixes & PREFIX_DATA) == 0))
	  || (ins.vex.evex && dp->prefix_requirement != PREFIX_DATA
	      && !ins.vex.w != !(ins.used_prefixes & PREFIX_DATA)))
	{
	  i386_dis_printf (info, dis_style_text, "(bad)");
	  ret = ins.end_codep - priv.the_buffer;
	  goto out;
	}
      break;

    case PREFIX_IGNORED:
      /* Zap data size and rep prefixes from used_prefixes and reinstate their
	 origins in all_prefixes.  */
      ins.used_prefixes &= ~PREFIX_OPCODE;
      if (ins.last_data_prefix >= 0)
	ins.all_prefixes[ins.last_data_prefix] = 0x66;
      if (ins.last_repz_prefix >= 0)
	ins.all_prefixes[ins.last_repz_prefix] = 0xf3;
      if (ins.last_repnz_prefix >= 0)
	ins.all_prefixes[ins.last_repnz_prefix] = 0xf2;
      break;
    }

  /* Check if the REX prefix is used.  */
  if ((ins.rex ^ ins.rex_used) == 0
      && !ins.need_vex && ins.last_rex_prefix >= 0)
    ins.all_prefixes[ins.last_rex_prefix] = 0;

  /* Check if the SEG prefix is used.  */
  if ((ins.prefixes & (PREFIX_CS | PREFIX_SS | PREFIX_DS | PREFIX_ES
		       | PREFIX_FS | PREFIX_GS)) != 0
      && (ins.used_prefixes & ins.active_seg_prefix) != 0)
    ins.all_prefixes[ins.last_seg_prefix] = 0;

  /* Check if the ADDR prefix is used.  */
  if ((ins.prefixes & PREFIX_ADDR) != 0
      && (ins.used_prefixes & PREFIX_ADDR) != 0)
    ins.all_prefixes[ins.last_addr_prefix] = 0;

  /* Check if the DATA prefix is used.  */
  if ((ins.prefixes & PREFIX_DATA) != 0
      && (ins.used_prefixes & PREFIX_DATA) != 0
      && !ins.need_vex)
    ins.all_prefixes[ins.last_data_prefix] = 0;

  /* Print the extra ins.prefixes.  */
  prefix_length = 0;
  for (i = 0; i < (int) ARRAY_SIZE (ins.all_prefixes); i++)
    if (ins.all_prefixes[i])
      {
	const char *name = prefix_name (ins.address_mode, ins.all_prefixes[i],
					orig_sizeflag);

	if (name == NULL)
	  abort ();
	prefix_length += strlen (name) + 1;
	i386_dis_printf (info, dis_style_mnemonic, "%s ", name);
      }

  /* Check maximum code length.  */
  if ((ins.codep - ins.start_codep) > MAX_CODE_LENGTH)
    {
      i386_dis_printf (info, dis_style_text, "(bad)");
      ret = MAX_CODE_LENGTH;
      goto out;
    }

  /* Calculate the number of operands this instruction has.  */
  op_count = 0;
  for (i = 0; i < MAX_OPERANDS; ++i)
    if (*ins.op_out[i] != '\0')
      ++op_count;

  /* Calculate the number of spaces to print after the mnemonic.  */
  ins.obufp = ins.mnemonicendp;
  if (op_count > 0)
    {
      i = strlen (ins.obuf) + prefix_length;
      if (i < 7)
	i = 7 - i;
      else
	i = 1;
    }
  else
    i = 0;

  /* Print the instruction mnemonic along with any trailing whitespace.  */
  i386_dis_printf (info, dis_style_mnemonic, "%s%*s", ins.obuf, i, "");

  /* The enter and bound instructions are printed with operands in the same
     order as the intel book; everything else is printed in reverse order.  */
  intel_swap_2_3 = false;
  if (ins.intel_syntax || ins.two_source_ops)
    {
      for (i = 0; i < MAX_OPERANDS; ++i)
	op_txt[i] = ins.op_out[i];

      if (ins.intel_syntax && dp && dp->op[2].rtn == OP_Rounding
          && dp->op[3].rtn == OP_E && dp->op[4].rtn == NULL)
	{
	  op_txt[2] = ins.op_out[3];
	  op_txt[3] = ins.op_out[2];
	  intel_swap_2_3 = true;
	}

      for (i = 0; i < (MAX_OPERANDS >> 1); ++i)
	{
	  bool riprel;

	  ins.op_ad = ins.op_index[i];
	  ins.op_index[i] = ins.op_index[MAX_OPERANDS - 1 - i];
	  ins.op_index[MAX_OPERANDS - 1 - i] = ins.op_ad;
	  riprel = ins.op_riprel[i];
	  ins.op_riprel[i] = ins.op_riprel[MAX_OPERANDS - 1 - i];
	  ins.op_riprel[MAX_OPERANDS - 1 - i] = riprel;
	}
    }
  else
    {
      for (i = 0; i < MAX_OPERANDS; ++i)
	op_txt[MAX_OPERANDS - 1 - i] = ins.op_out[i];
    }

  needcomma = 0;
  for (i = 0; i < MAX_OPERANDS; ++i)
    if (*op_txt[i])
      {
	/* In Intel syntax embedded rounding / SAE are not separate operands.
	   Instead they're attached to the prior register operand.  Simply
	   suppress emission of the comma to achieve that effect.  */
	switch (i & -(ins.intel_syntax && dp))
	  {
	  case 2:
	    if (dp->op[2].rtn == OP_Rounding && !intel_swap_2_3)
	      needcomma = 0;
	    break;
	  case 3:
	    if (dp->op[3].rtn == OP_Rounding || intel_swap_2_3)
	      needcomma = 0;
	    break;
	  }
	if (needcomma)
	  i386_dis_printf (info, dis_style_text, ",");
	if (ins.op_index[i] != -1 && !ins.op_riprel[i])
	  {
	    bfd_vma target = (bfd_vma) ins.op_address[ins.op_index[i]];

	    if (ins.op_is_jump)
	      {
		info->insn_info_valid = 1;
		info->branch_delay_insns = 0;
		info->data_size = 0;
		info->target = target;
		info->target2 = 0;
	      }
	    (*info->print_address_func) (target, info);
	  }
	else
	  i386_dis_printf (info, dis_style_text, "%s", op_txt[i]);
	needcomma = 1;
      }

  for (i = 0; i < MAX_OPERANDS; i++)
    if (ins.op_index[i] != -1 && ins.op_riprel[i])
      {
	i386_dis_printf (info, dis_style_comment_start, "        # ");
	(*info->print_address_func)
	  ((bfd_vma)(ins.start_pc + (ins.codep - ins.start_codep)
		     + ins.op_address[ins.op_index[i]]),
	  info);
	break;
      }
  ret = ins.codep - priv.the_buffer;
 out:
  info->private_data = NULL;
  return ret;
}

/* Here for backwards compatibility.  When gdb stops using
   print_insn_i386_att and print_insn_i386_intel these functions can
   disappear, and print_insn_i386 be merged into print_insn.  */
int
print_insn_i386_att (bfd_vma pc, disassemble_info *info)
{
  return print_insn (pc, info, 0);
}

int
print_insn_i386_intel (bfd_vma pc, disassemble_info *info)
{
  return print_insn (pc, info, 1);
}

int
print_insn_i386 (bfd_vma pc, disassemble_info *info)
{
  return print_insn (pc, info, -1);
}

static const char *float_mem[] = {
  /* d8 */
  "fadd{s|}",
  "fmul{s|}",
  "fcom{s|}",
  "fcomp{s|}",
  "fsub{s|}",
  "fsubr{s|}",
  "fdiv{s|}",
  "fdivr{s|}",
  /* d9 */
  "fld{s|}",
  "(bad)",
  "fst{s|}",
  "fstp{s|}",
  "fldenv{C|C}",
  "fldcw",
  "fNstenv{C|C}",
  "fNstcw",
  /* da */
  "fiadd{l|}",
  "fimul{l|}",
  "ficom{l|}",
  "ficomp{l|}",
  "fisub{l|}",
  "fisubr{l|}",
  "fidiv{l|}",
  "fidivr{l|}",
  /* db */
  "fild{l|}",
  "fisttp{l|}",
  "fist{l|}",
  "fistp{l|}",
  "(bad)",
  "fld{t|}",
  "(bad)",
  "fstp{t|}",
  /* dc */
  "fadd{l|}",
  "fmul{l|}",
  "fcom{l|}",
  "fcomp{l|}",
  "fsub{l|}",
  "fsubr{l|}",
  "fdiv{l|}",
  "fdivr{l|}",
  /* dd */
  "fld{l|}",
  "fisttp{ll|}",
  "fst{l||}",
  "fstp{l|}",
  "frstor{C|C}",
  "(bad)",
  "fNsave{C|C}",
  "fNstsw",
  /* de */
  "fiadd{s|}",
  "fimul{s|}",
  "ficom{s|}",
  "ficomp{s|}",
  "fisub{s|}",
  "fisubr{s|}",
  "fidiv{s|}",
  "fidivr{s|}",
  /* df */
  "fild{s|}",
  "fisttp{s|}",
  "fist{s|}",
  "fistp{s|}",
  "fbld",
  "fild{ll|}",
  "fbstp",
  "fistp{ll|}",
};

static const unsigned char float_mem_mode[] = {
  /* d8 */
  d_mode,
  d_mode,
  d_mode,
  d_mode,
  d_mode,
  d_mode,
  d_mode,
  d_mode,
  /* d9 */
  d_mode,
  0,
  d_mode,
  d_mode,
  0,
  w_mode,
  0,
  w_mode,
  /* da */
  d_mode,
  d_mode,
  d_mode,
  d_mode,
  d_mode,
  d_mode,
  d_mode,
  d_mode,
  /* db */
  d_mode,
  d_mode,
  d_mode,
  d_mode,
  0,
  t_mode,
  0,
  t_mode,
  /* dc */
  q_mode,
  q_mode,
  q_mode,
  q_mode,
  q_mode,
  q_mode,
  q_mode,
  q_mode,
  /* dd */
  q_mode,
  q_mode,
  q_mode,
  q_mode,
  0,
  0,
  0,
  w_mode,
  /* de */
  w_mode,
  w_mode,
  w_mode,
  w_mode,
  w_mode,
  w_mode,
  w_mode,
  w_mode,
  /* df */
  w_mode,
  w_mode,
  w_mode,
  w_mode,
  t_mode,
  q_mode,
  t_mode,
  q_mode
};

#define ST { OP_ST, 0 }
#define STi { OP_STi, 0 }

#define FGRPd9_2 NULL, { { NULL, 1 } }, 0
#define FGRPd9_4 NULL, { { NULL, 2 } }, 0
#define FGRPd9_5 NULL, { { NULL, 3 } }, 0
#define FGRPd9_6 NULL, { { NULL, 4 } }, 0
#define FGRPd9_7 NULL, { { NULL, 5 } }, 0
#define FGRPda_5 NULL, { { NULL, 6 } }, 0
#define FGRPdb_4 NULL, { { NULL, 7 } }, 0
#define FGRPde_3 NULL, { { NULL, 8 } }, 0
#define FGRPdf_4 NULL, { { NULL, 9 } }, 0

static const struct dis386 float_reg[][8] = {
  /* d8 */
  {
    { "fadd",	{ ST, STi }, 0 },
    { "fmul",	{ ST, STi }, 0 },
    { "fcom",	{ STi }, 0 },
    { "fcomp",	{ STi }, 0 },
    { "fsub",	{ ST, STi }, 0 },
    { "fsubr",	{ ST, STi }, 0 },
    { "fdiv",	{ ST, STi }, 0 },
    { "fdivr",	{ ST, STi }, 0 },
  },
  /* d9 */
  {
    { "fld",	{ STi }, 0 },
    { "fxch",	{ STi }, 0 },
    { FGRPd9_2 },
    { Bad_Opcode },
    { FGRPd9_4 },
    { FGRPd9_5 },
    { FGRPd9_6 },
    { FGRPd9_7 },
  },
  /* da */
  {
    { "fcmovb",	{ ST, STi }, 0 },
    { "fcmove",	{ ST, STi }, 0 },
    { "fcmovbe",{ ST, STi }, 0 },
    { "fcmovu",	{ ST, STi }, 0 },
    { Bad_Opcode },
    { FGRPda_5 },
    { Bad_Opcode },
    { Bad_Opcode },
  },
  /* db */
  {
    { "fcmovnb",{ ST, STi }, 0 },
    { "fcmovne",{ ST, STi }, 0 },
    { "fcmovnbe",{ ST, STi }, 0 },
    { "fcmovnu",{ ST, STi }, 0 },
    { FGRPdb_4 },
    { "fucomi",	{ ST, STi }, 0 },
    { "fcomi",	{ ST, STi }, 0 },
    { Bad_Opcode },
  },
  /* dc */
  {
    { "fadd",	{ STi, ST }, 0 },
    { "fmul",	{ STi, ST }, 0 },
    { Bad_Opcode },
    { Bad_Opcode },
    { "fsub{!M|r}",	{ STi, ST }, 0 },
    { "fsub{M|}",	{ STi, ST }, 0 },
    { "fdiv{!M|r}",	{ STi, ST }, 0 },
    { "fdiv{M|}",	{ STi, ST }, 0 },
  },
  /* dd */
  {
    { "ffree",	{ STi }, 0 },
    { Bad_Opcode },
    { "fst",	{ STi }, 0 },
    { "fstp",	{ STi }, 0 },
    { "fucom",	{ STi }, 0 },
    { "fucomp",	{ STi }, 0 },
    { Bad_Opcode },
    { Bad_Opcode },
  },
  /* de */
  {
    { "faddp",	{ STi, ST }, 0 },
    { "fmulp",	{ STi, ST }, 0 },
    { Bad_Opcode },
    { FGRPde_3 },
    { "fsub{!M|r}p",	{ STi, ST }, 0 },
    { "fsub{M|}p",	{ STi, ST }, 0 },
    { "fdiv{!M|r}p",	{ STi, ST }, 0 },
    { "fdiv{M|}p",	{ STi, ST }, 0 },
  },
  /* df */
  {
    { "ffreep",	{ STi }, 0 },
    { Bad_Opcode },
    { Bad_Opcode },
    { Bad_Opcode },
    { FGRPdf_4 },
    { "fucomip", { ST, STi }, 0 },
    { "fcomip", { ST, STi }, 0 },
    { Bad_Opcode },
  },
};

static const char *const fgrps[][8] = {
  /* Bad opcode 0 */
  {
    "(bad)","(bad)","(bad)","(bad)","(bad)","(bad)","(bad)","(bad)",
  },

  /* d9_2  1 */
  {
    "fnop","(bad)","(bad)","(bad)","(bad)","(bad)","(bad)","(bad)",
  },

  /* d9_4  2 */
  {
    "fchs","fabs","(bad)","(bad)","ftst","fxam","(bad)","(bad)",
  },

  /* d9_5  3 */
  {
    "fld1","fldl2t","fldl2e","fldpi","fldlg2","fldln2","fldz","(bad)",
  },

  /* d9_6  4 */
  {
    "f2xm1","fyl2x","fptan","fpatan","fxtract","fprem1","fdecstp","fincstp",
  },

  /* d9_7  5 */
  {
    "fprem","fyl2xp1","fsqrt","fsincos","frndint","fscale","fsin","fcos",
  },

  /* da_5  6 */
  {
    "(bad)","fucompp","(bad)","(bad)","(bad)","(bad)","(bad)","(bad)",
  },

  /* db_4  7 */
  {
    "fNeni(8087 only)","fNdisi(8087 only)","fNclex","fNinit",
    "fNsetpm(287 only)","frstpm(287 only)","(bad)","(bad)",
  },

  /* de_3  8 */
  {
    "(bad)","fcompp","(bad)","(bad)","(bad)","(bad)","(bad)","(bad)",
  },

  /* df_4  9 */
  {
    "fNstsw","(bad)","(bad)","(bad)","(bad)","(bad)","(bad)","(bad)",
  },
};

static void
swap_operand (instr_info *ins)
{
  ins->mnemonicendp[0] = '.';
  ins->mnemonicendp[1] = 's';
  ins->mnemonicendp[2] = '\0';
  ins->mnemonicendp += 2;
}

static bool
OP_Skip_MODRM (instr_info *ins, int bytemode ATTRIBUTE_UNUSED,
	       int sizeflag ATTRIBUTE_UNUSED)
{
  /* Skip mod/rm byte.  */
  MODRM_CHECK;
  ins->codep++;
  return true;
}

static bool
dofloat (instr_info *ins, int sizeflag)
{
  const struct dis386 *dp;
  unsigned char floatop = ins->codep[-1];

  if (ins->modrm.mod != 3)
    {
      int fp_indx = (floatop - 0xd8) * 8 + ins->modrm.reg;

      putop (ins, float_mem[fp_indx], sizeflag);
      ins->obufp = ins->op_out[0];
      ins->op_ad = 2;
      return OP_E (ins, float_mem_mode[fp_indx], sizeflag);
    }
  /* Skip mod/rm byte.  */
  MODRM_CHECK;
  ins->codep++;

  dp = &float_reg[floatop - 0xd8][ins->modrm.reg];
  if (dp->name == NULL)
    {
      putop (ins, fgrps[dp->op[0].bytemode][ins->modrm.rm], sizeflag);

      /* Instruction fnstsw is only one with strange arg.  */
      if (floatop == 0xdf && ins->codep[-1] == 0xe0)
	strcpy (ins->op_out[0], att_names16[0] + ins->intel_syntax);
    }
  else
    {
      putop (ins, dp->name, sizeflag);

      ins->obufp = ins->op_out[0];
      ins->op_ad = 2;
      if (dp->op[0].rtn
	  && !dp->op[0].rtn (ins, dp->op[0].bytemode, sizeflag))
	return false;

      ins->obufp = ins->op_out[1];
      ins->op_ad = 1;
      if (dp->op[1].rtn
	  && !dp->op[1].rtn (ins, dp->op[1].bytemode, sizeflag))
	return false;
    }
  return true;
}

static bool
OP_ST (instr_info *ins, int bytemode ATTRIBUTE_UNUSED,
       int sizeflag ATTRIBUTE_UNUSED)
{
  oappend_register (ins, "%st");
  return true;
}

static bool
OP_STi (instr_info *ins, int bytemode ATTRIBUTE_UNUSED,
	int sizeflag ATTRIBUTE_UNUSED)
{
  char scratch[8];
  int res = snprintf (scratch, ARRAY_SIZE (scratch), "%%st(%d)", ins->modrm.rm);

  if (res < 0 || (size_t) res >= ARRAY_SIZE (scratch))
    abort ();
  oappend_register (ins, scratch);
  return true;
}

/* Capital letters in template are macros.  */
static int
putop (instr_info *ins, const char *in_template, int sizeflag)
{
  const char *p;
  int alt = 0;
  int cond = 1;
  unsigned int l = 0, len = 0;
  char last[4];

  for (p = in_template; *p; p++)
    {
      if (len > l)
	{
	  if (l >= sizeof (last) || !ISUPPER (*p))
	    abort ();
	  last[l++] = *p;
	  continue;
	}
      switch (*p)
	{
	default:
	  *ins->obufp++ = *p;
	  break;
	case '%':
	  len++;
	  break;
	case '!':
	  cond = 0;
	  break;
	case '{':
	  if (ins->intel_syntax)
	    {
	      while (*++p != '|')
		if (*p == '}' || *p == '\0')
		  abort ();
	      alt = 1;
	    }
	  break;
	case '|':
	  while (*++p != '}')
	    {
	      if (*p == '\0')
		abort ();
	    }
	  break;
	case '}':
	  alt = 0;
	  break;
	case 'A':
	  if (ins->intel_syntax)
	    break;
	  if ((ins->need_modrm && ins->modrm.mod != 3)
	      || (sizeflag & SUFFIX_ALWAYS))
	    *ins->obufp++ = 'b';
	  break;
	case 'B':
	  if (l == 0)
	    {
	    case_B:
	      if (ins->intel_syntax)
		break;
	      if (sizeflag & SUFFIX_ALWAYS)
		*ins->obufp++ = 'b';
	    }
	  else if (l == 1 && last[0] == 'L')
	    {
	      if (ins->address_mode == mode_64bit
		  && !(ins->prefixes & PREFIX_ADDR))
		{
		  *ins->obufp++ = 'a';
		  *ins->obufp++ = 'b';
		  *ins->obufp++ = 's';
		}

	      goto case_B;
	    }
	  else
	    abort ();
	  break;
	case 'C':
	  if (ins->intel_syntax && !alt)
	    break;
	  if ((ins->prefixes & PREFIX_DATA) || (sizeflag & SUFFIX_ALWAYS))
	    {
	      if (sizeflag & DFLAG)
		*ins->obufp++ = ins->intel_syntax ? 'd' : 'l';
	      else
		*ins->obufp++ = ins->intel_syntax ? 'w' : 's';
	      ins->used_prefixes |= (ins->prefixes & PREFIX_DATA);
	    }
	  break;
	case 'D':
	  if (l == 1)
	    {
	      switch (last[0])
	      {
	      case 'X':
		if (!ins->vex.evex || ins->vex.w)
		  *ins->obufp++ = 'd';
		else
		  oappend (ins, "{bad}");
		break;
	      default:
		abort ();
	      }
	      break;
	    }
	  if (l)
	    abort ();
	  if (ins->intel_syntax || !(sizeflag & SUFFIX_ALWAYS))
	    break;
	  USED_REX (REX_W);
	  if (ins->modrm.mod == 3)
	    {
	      if (ins->rex & REX_W)
		*ins->obufp++ = 'q';
	      else
		{
		  if (sizeflag & DFLAG)
		    *ins->obufp++ = ins->intel_syntax ? 'd' : 'l';
		  else
		    *ins->obufp++ = 'w';
		  ins->used_prefixes |= (ins->prefixes & PREFIX_DATA);
		}
	    }
	  else
	    *ins->obufp++ = 'w';
	  break;
	case 'E':
	  if (l == 1)
	    {
	      switch (last[0])
		{
		case 'X':
		  if (!ins->vex.evex || ins->vex.b || ins->vex.ll >= 2
		      || !ins->vex.r
		      || (ins->modrm.mod == 3 && (ins->rex & REX_X))
		      || !ins->vex.v || ins->vex.mask_register_specifier)
		    break;
		  /* AVX512 extends a number of V*D insns to also have V*Q variants,
		     merely distinguished by EVEX.W.  Look for a use of the
		     respective macro.  */
		  if (ins->vex.w)
		    {
		      const char *pct = strchr (p + 1, '%');

		      if (pct != NULL && pct[1] == 'D' && pct[2] == 'Q')
			break;
		    }
		  *ins->obufp++ = '{';
		  *ins->obufp++ = 'e';
		  *ins->obufp++ = 'v';
		  *ins->obufp++ = 'e';
		  *ins->obufp++ = 'x';
		  *ins->obufp++ = '}';
		  *ins->obufp++ = ' ';
		  break;
		default:
		  abort ();
		}
		break;
	    }
	  /* For jcxz/jecxz */
	  if (ins->address_mode == mode_64bit)
	    {
	      if (sizeflag & AFLAG)
		*ins->obufp++ = 'r';
	      else
		*ins->obufp++ = 'e';
	    }
	  else
	    if (sizeflag & AFLAG)
	      *ins->obufp++ = 'e';
	  ins->used_prefixes |= (ins->prefixes & PREFIX_ADDR);
	  break;
	case 'F':
	  if (ins->intel_syntax)
	    break;
	  if ((ins->prefixes & PREFIX_ADDR) || (sizeflag & SUFFIX_ALWAYS))
	    {
	      if (sizeflag & AFLAG)
		*ins->obufp++ = ins->address_mode == mode_64bit ? 'q' : 'l';
	      else
		*ins->obufp++ = ins->address_mode == mode_64bit ? 'l' : 'w';
	      ins->used_prefixes |= (ins->prefixes & PREFIX_ADDR);
	    }
	  break;
	case 'G':
	  if (ins->intel_syntax || (ins->obufp[-1] != 's'
				    && !(sizeflag & SUFFIX_ALWAYS)))
	    break;
	  if ((ins->rex & REX_W) || (sizeflag & DFLAG))
	    *ins->obufp++ = 'l';
	  else
	    *ins->obufp++ = 'w';
	  if (!(ins->rex & REX_W))
	    ins->used_prefixes |= (ins->prefixes & PREFIX_DATA);
	  break;
	case 'H':
	  if (l == 0)
	    {
	      if (ins->intel_syntax)
	        break;
	      if ((ins->prefixes & (PREFIX_CS | PREFIX_DS)) == PREFIX_CS
		  || (ins->prefixes & (PREFIX_CS | PREFIX_DS)) == PREFIX_DS)
		{
		  ins->used_prefixes |= ins->prefixes & (PREFIX_CS | PREFIX_DS);
		  *ins->obufp++ = ',';
		  *ins->obufp++ = 'p';

		  /* Set active_seg_prefix even if not set in 64-bit mode
		     because here it is a valid branch hint. */
		  if (ins->prefixes & PREFIX_DS)
		    {
		      ins->active_seg_prefix = PREFIX_DS;
		      *ins->obufp++ = 't';
		    }
		  else
		    {
		      ins->active_seg_prefix = PREFIX_CS;
		      *ins->obufp++ = 'n';
		    }
		}
	    }
	  else if (l == 1 && last[0] == 'X')
	    {
	      if (!ins->vex.w)
		*ins->obufp++ = 'h';
	      else
		oappend (ins, "{bad}");
	    }
	  else
	    abort ();
	  break;
	case 'K':
	  USED_REX (REX_W);
	  if (ins->rex & REX_W)
	    *ins->obufp++ = 'q';
	  else
	    *ins->obufp++ = 'd';
	  break;
	case 'L':
	  abort ();
	case 'M':
	  if (ins->intel_mnemonic != cond)
	    *ins->obufp++ = 'r';
	  break;
	case 'N':
	  if ((ins->prefixes & PREFIX_FWAIT) == 0)
	    *ins->obufp++ = 'n';
	  else
	    ins->used_prefixes |= PREFIX_FWAIT;
	  break;
	case 'O':
	  USED_REX (REX_W);
	  if (ins->rex & REX_W)
	    *ins->obufp++ = 'o';
	  else if (ins->intel_syntax && (sizeflag & DFLAG))
	    *ins->obufp++ = 'q';
	  else
	    *ins->obufp++ = 'd';
	  if (!(ins->rex & REX_W))
	    ins->used_prefixes |= (ins->prefixes & PREFIX_DATA);
	  break;
	case '@':
	  if (ins->address_mode == mode_64bit
	      && (ins->isa64 == intel64 || (ins->rex & REX_W)
		  || !(ins->prefixes & PREFIX_DATA)))
	    {
	      if (sizeflag & SUFFIX_ALWAYS)
		*ins->obufp++ = 'q';
	      break;
	    }
	  /* Fall through.  */
	case 'P':
	  if (l == 0)
	    {
	      if ((ins->modrm.mod == 3 || !cond)
		  && !(sizeflag & SUFFIX_ALWAYS))
		break;
	  /* Fall through.  */
	case 'T':
	      if ((!(ins->rex & REX_W) && (ins->prefixes & PREFIX_DATA))
		  || ((sizeflag & SUFFIX_ALWAYS)
		      && ins->address_mode != mode_64bit))
		{
		  *ins->obufp++ = (sizeflag & DFLAG)
				  ? ins->intel_syntax ? 'd' : 'l' : 'w';
		  ins->used_prefixes |= (ins->prefixes & PREFIX_DATA);
		}
	      else if (sizeflag & SUFFIX_ALWAYS)
		*ins->obufp++ = 'q';
	    }
	  else if (l == 1 && last[0] == 'L')
	    {
	      if ((ins->prefixes & PREFIX_DATA)
		  || (ins->rex & REX_W)
		  || (sizeflag & SUFFIX_ALWAYS))
		{
		  USED_REX (REX_W);
		  if (ins->rex & REX_W)
		    *ins->obufp++ = 'q';
		  else
		    {
		      if (sizeflag & DFLAG)
			*ins->obufp++ = ins->intel_syntax ? 'd' : 'l';
		      else
			*ins->obufp++ = 'w';
		      ins->used_prefixes |= (ins->prefixes & PREFIX_DATA);
		    }
		}
	    }
	  else
	    abort ();
	  break;
	case 'Q':
	  if (l == 0)
	    {
	      if (ins->intel_syntax && !alt)
		break;
	      USED_REX (REX_W);
	      if ((ins->need_modrm && ins->modrm.mod != 3)
		  || (sizeflag & SUFFIX_ALWAYS))
		{
		  if (ins->rex & REX_W)
		    *ins->obufp++ = 'q';
		  else
		    {
		      if (sizeflag & DFLAG)
			*ins->obufp++ = ins->intel_syntax ? 'd' : 'l';
		      else
			*ins->obufp++ = 'w';
		      ins->used_prefixes |= (ins->prefixes & PREFIX_DATA);
		    }
		}
	    }
	  else if (l == 1 && last[0] == 'D')
	    *ins->obufp++ = ins->vex.w ? 'q' : 'd';
	  else if (l == 1 && last[0] == 'L')
	    {
	      if (cond ? ins->modrm.mod == 3 && !(sizeflag & SUFFIX_ALWAYS)
		       : ins->address_mode != mode_64bit)
		break;
	      if ((ins->rex & REX_W))
		{
		  USED_REX (REX_W);
		  *ins->obufp++ = 'q';
		}
	      else if ((ins->address_mode == mode_64bit && cond)
		      || (sizeflag & SUFFIX_ALWAYS))
		*ins->obufp++ = ins->intel_syntax? 'd' : 'l';
	    }
	  else
	    abort ();
	  break;
	case 'R':
	  USED_REX (REX_W);
	  if (ins->rex & REX_W)
	    *ins->obufp++ = 'q';
	  else if (sizeflag & DFLAG)
	    {
	      if (ins->intel_syntax)
		  *ins->obufp++ = 'd';
	      else
		  *ins->obufp++ = 'l';
	    }
	  else
	    *ins->obufp++ = 'w';
	  if (ins->intel_syntax && !p[1]
	      && ((ins->rex & REX_W) || (sizeflag & DFLAG)))
	    *ins->obufp++ = 'e';
	  if (!(ins->rex & REX_W))
	    ins->used_prefixes |= (ins->prefixes & PREFIX_DATA);
	  break;
	case 'S':
	  if (l == 0)
	    {
	    case_S:
	      if (ins->intel_syntax)
		break;
	      if (sizeflag & SUFFIX_ALWAYS)
		{
		  if (ins->rex & REX_W)
		    *ins->obufp++ = 'q';
		  else
		    {
		      if (sizeflag & DFLAG)
			*ins->obufp++ = 'l';
		      else
			*ins->obufp++ = 'w';
		      ins->used_prefixes |= (ins->prefixes & PREFIX_DATA);
		    }
		}
	      break;
	    }
	  if (l != 1)
	    abort ();
	  switch (last[0])
	    {
	    case 'L':
	      if (ins->address_mode == mode_64bit
		  && !(ins->prefixes & PREFIX_ADDR))
		{
		  *ins->obufp++ = 'a';
		  *ins->obufp++ = 'b';
		  *ins->obufp++ = 's';
		}

	      goto case_S;
	    case 'X':
	      if (!ins->vex.evex || !ins->vex.w)
		*ins->obufp++ = 's';
	      else
		oappend (ins, "{bad}");
	      break;
	    default:
	      abort ();
	    }
	  break;
	case 'V':
	  if (l == 0)
	    abort ();
	  else if (l == 1)
	    {
	      switch (last[0])
		{
		case 'X':
		  if (ins->vex.evex)
		    break;
		  *ins->obufp++ = '{';
		  *ins->obufp++ = 'v';
		  *ins->obufp++ = 'e';
		  *ins->obufp++ = 'x';
		  *ins->obufp++ = '}';
		  *ins->obufp++ = ' ';
		  break;
		case 'L':
		  if (ins->rex & REX_W)
		    {
		      *ins->obufp++ = 'a';
		      *ins->obufp++ = 'b';
		      *ins->obufp++ = 's';
		    }
		  goto case_S;
		default:
		  abort ();
		}
	    }
	  else
	    abort ();
	  break;
	case 'W':
	  if (l == 0)
	    {
	      /* operand size flag for cwtl, cbtw */
	      USED_REX (REX_W);
	      if (ins->rex & REX_W)
		{
		  if (ins->intel_syntax)
		    *ins->obufp++ = 'd';
		  else
		    *ins->obufp++ = 'l';
		}
	      else if (sizeflag & DFLAG)
		*ins->obufp++ = 'w';
	      else
		*ins->obufp++ = 'b';
	      if (!(ins->rex & REX_W))
		ins->used_prefixes |= (ins->prefixes & PREFIX_DATA);
	    }
	  else if (l == 1)
	    {
	      if (!ins->need_vex)
		abort ();
	      if (last[0] == 'X')
		*ins->obufp++ = ins->vex.w ? 'd': 's';
	      else if (last[0] == 'B')
		*ins->obufp++ = ins->vex.w ? 'w': 'b';
	      else
		abort ();
	    }
	  else
	    abort ();
	  break;
	case 'X':
	  if (l != 0)
	    abort ();
	  if (ins->need_vex
	      ? ins->vex.prefix == DATA_PREFIX_OPCODE
	      : ins->prefixes & PREFIX_DATA)
	    {
	      *ins->obufp++ = 'd';
	      ins->used_prefixes |= PREFIX_DATA;
	    }
	  else
	    *ins->obufp++ = 's';
	  break;
	case 'Y':
	  if (l == 1 && last[0] == 'X')
	    {
	      if (!ins->need_vex)
		abort ();
	      if (ins->intel_syntax
		  || ((ins->modrm.mod == 3 || ins->vex.b)
		      && !(sizeflag & SUFFIX_ALWAYS)))
		break;
	      switch (ins->vex.length)
		{
		case 128:
		  *ins->obufp++ = 'x';
		  break;
		case 256:
		  *ins->obufp++ = 'y';
		  break;
		case 512:
		  if (!ins->vex.evex)
		default:
		    abort ();
		}
	    }
	  else
	    abort ();
	  break;
	case 'Z':
	  if (l == 0)
	    {
	      /* These insns ignore ModR/M.mod: Force it to 3 for OP_E().  */
	      ins->modrm.mod = 3;
	      if (!ins->intel_syntax && (sizeflag & SUFFIX_ALWAYS))
		*ins->obufp++ = ins->address_mode == mode_64bit ? 'q' : 'l';
	    }
	  else if (l == 1 && last[0] == 'X')
	    {
	      if (!ins->vex.evex)
		abort ();
	      if (ins->intel_syntax
		  || ((ins->modrm.mod == 3 || ins->vex.b)
		      && !(sizeflag & SUFFIX_ALWAYS)))
		break;
	      switch (ins->vex.length)
		{
		case 128:
		  *ins->obufp++ = 'x';
		  break;
		case 256:
		  *ins->obufp++ = 'y';
		  break;
		case 512:
		  *ins->obufp++ = 'z';
		  break;
		default:
		  abort ();
		}
	    }
	  else
	    abort ();
	  break;
	case '^':
	  if (ins->intel_syntax)
	    break;
	  if (ins->isa64 == intel64 && (ins->rex & REX_W))
	    {
	      USED_REX (REX_W);
	      *ins->obufp++ = 'q';
	      break;
	    }
	  if ((ins->prefixes & PREFIX_DATA) || (sizeflag & SUFFIX_ALWAYS))
	    {
	      if (sizeflag & DFLAG)
		*ins->obufp++ = 'l';
	      else
		*ins->obufp++ = 'w';
	      ins->used_prefixes |= (ins->prefixes & PREFIX_DATA);
	    }
	  break;
	}

      if (len == l)
	len = l = 0;
    }
  *ins->obufp = 0;
  ins->mnemonicendp = ins->obufp;
  return 0;
}

/* Add a style marker to *INS->obufp that encodes STYLE.  This assumes that
   the buffer pointed to by INS->obufp has space.  A style marker is made
   from the STYLE_MARKER_CHAR followed by STYLE converted to a single hex
   digit, followed by another STYLE_MARKER_CHAR.  This function assumes
   that the number of styles is not greater than 16.  */

static void
oappend_insert_style (instr_info *ins, enum disassembler_style style)
{
  unsigned num = (unsigned) style;

  /* We currently assume that STYLE can be encoded as a single hex
     character.  If more styles are added then this might start to fail,
     and we'll need to expand this code.  */
  if (num > 0xf)
    abort ();

  *ins->obufp++ = STYLE_MARKER_CHAR;
  *ins->obufp++ = (num < 10 ? ('0' + num)
		   : ((num < 16) ? ('a' + (num - 10)) : '0'));
  *ins->obufp++ = STYLE_MARKER_CHAR;

  /* This final null character is not strictly necessary, after inserting a
     style marker we should always be inserting some additional content.
     However, having the buffer null terminated doesn't cost much, and make
     it easier to debug what's going on.  Also, if we do ever forget to add
     any additional content after this style marker, then the buffer will
     still be well formed.  */
  *ins->obufp = '\0';
}

static void
oappend_with_style (instr_info *ins, const char *s,
		    enum disassembler_style style)
{
  oappend_insert_style (ins, style);
  ins->obufp = stpcpy (ins->obufp, s);
}

/* Add a single character C to the buffer pointer to by INS->obufp, marking
   the style for the character as STYLE.  */

static void
oappend_char_with_style (instr_info *ins, const char c,
			 enum disassembler_style style)
{
  oappend_insert_style (ins, style);
  *ins->obufp++ = c;
  *ins->obufp = '\0';
}

/* Like oappend_char_with_style, but always uses dis_style_text.  */

static void
oappend_char (instr_info *ins, const char c)
{
  oappend_char_with_style (ins, c, dis_style_text);
}

static void
append_seg (instr_info *ins)
{
  /* Only print the active segment register.  */
  if (!ins->active_seg_prefix)
    return;

  ins->used_prefixes |= ins->active_seg_prefix;
  switch (ins->active_seg_prefix)
    {
    case PREFIX_CS:
      oappend_register (ins, att_names_seg[1]);
      break;
    case PREFIX_DS:
      oappend_register (ins, att_names_seg[3]);
      break;
    case PREFIX_SS:
      oappend_register (ins, att_names_seg[2]);
      break;
    case PREFIX_ES:
      oappend_register (ins, att_names_seg[0]);
      break;
    case PREFIX_FS:
      oappend_register (ins, att_names_seg[4]);
      break;
    case PREFIX_GS:
      oappend_register (ins, att_names_seg[5]);
      break;
    default:
      break;
    }
  oappend_char (ins, ':');
}

static bool
OP_indirE (instr_info *ins, int bytemode, int sizeflag)
{
  if (!ins->intel_syntax)
    oappend (ins, "*");
  return OP_E (ins, bytemode, sizeflag);
}

static void
print_operand_value (instr_info *ins, bfd_vma disp,
		     enum disassembler_style style)
{
  char tmp[30];

  if (ins->address_mode != mode_64bit)
    disp &= 0xffffffff;
  sprintf (tmp, "0x%" PRIx64, (uint64_t) disp);
  oappend_with_style (ins, tmp, style);
}

/* Like oappend, but called for immediate operands.  */

static void
oappend_immediate (instr_info *ins, bfd_vma imm)
{
  if (!ins->intel_syntax)
    oappend_char_with_style (ins, '$', dis_style_immediate);
  print_operand_value (ins, imm, dis_style_immediate);
}

/* Put DISP in BUF as signed hex number.  */

static void
print_displacement (instr_info *ins, bfd_signed_vma val)
{
  char tmp[30];

  if (val < 0)
    {
      oappend_char_with_style (ins, '-', dis_style_address_offset);
      val = (bfd_vma) 0 - val;

      /* Check for possible overflow.  */
      if (val < 0)
	{
	  switch (ins->address_mode)
	    {
	    case mode_64bit:
	      oappend_with_style (ins, "0x8000000000000000",
				  dis_style_address_offset);
	      break;
	    case mode_32bit:
	      oappend_with_style (ins, "0x80000000",
				  dis_style_address_offset);
	      break;
	    case mode_16bit:
	      oappend_with_style (ins, "0x8000",
				  dis_style_address_offset);
	      break;
	    }
	  return;
	}
    }

  sprintf (tmp, "0x%" PRIx64, (int64_t) val);
  oappend_with_style (ins, tmp, dis_style_address_offset);
}

static void
intel_operand_size (instr_info *ins, int bytemode, int sizeflag)
{
  if (ins->vex.b)
    {
      if (!ins->vex.no_broadcast)
	switch (bytemode)
	  {
	  case x_mode:
	  case evex_half_bcst_xmmq_mode:
	    if (ins->vex.w)
	      oappend (ins, "QWORD BCST ");
	    else
	      oappend (ins, "DWORD BCST ");
	    break;
	  case xh_mode:
	  case evex_half_bcst_xmmqh_mode:
	  case evex_half_bcst_xmmqdh_mode:
	    oappend (ins, "WORD BCST ");
	    break;
	  default:
	    ins->vex.no_broadcast = true;
	    break;
	  }
      return;
    }
  switch (bytemode)
    {
    case b_mode:
    case b_swap_mode:
    case db_mode:
      oappend (ins, "BYTE PTR ");
      break;
    case w_mode:
    case w_swap_mode:
    case dw_mode:
      oappend (ins, "WORD PTR ");
      break;
    case indir_v_mode:
      if (ins->address_mode == mode_64bit && ins->isa64 == intel64)
	{
	  oappend (ins, "QWORD PTR ");
	  break;
	}
      /* Fall through.  */
    case stack_v_mode:
      if (ins->address_mode == mode_64bit && ((sizeflag & DFLAG)
					      || (ins->rex & REX_W)))
	{
	  oappend (ins, "QWORD PTR ");
	  break;
	}
      /* Fall through.  */
    case v_mode:
    case v_swap_mode:
    case dq_mode:
      USED_REX (REX_W);
      if (ins->rex & REX_W)
	oappend (ins, "QWORD PTR ");
      else if (bytemode == dq_mode)
	oappend (ins, "DWORD PTR ");
      else
	{
	  if (sizeflag & DFLAG)
	    oappend (ins, "DWORD PTR ");
	  else
	    oappend (ins, "WORD PTR ");
	  ins->used_prefixes |= (ins->prefixes & PREFIX_DATA);
	}
      break;
    case z_mode:
      if ((ins->rex & REX_W) || (sizeflag & DFLAG))
	*ins->obufp++ = 'D';
      oappend (ins, "WORD PTR ");
      if (!(ins->rex & REX_W))
	ins->used_prefixes |= (ins->prefixes & PREFIX_DATA);
      break;
    case a_mode:
      if (sizeflag & DFLAG)
	oappend (ins, "QWORD PTR ");
      else
	oappend (ins, "DWORD PTR ");
      ins->used_prefixes |= (ins->prefixes & PREFIX_DATA);
      break;
    case movsxd_mode:
      if (!(sizeflag & DFLAG) && ins->isa64 == intel64)
	oappend (ins, "WORD PTR ");
      else
	oappend (ins, "DWORD PTR ");
      ins->used_prefixes |= (ins->prefixes & PREFIX_DATA);
      break;
    case d_mode:
    case d_swap_mode:
      oappend (ins, "DWORD PTR ");
      break;
    case q_mode:
    case q_swap_mode:
      oappend (ins, "QWORD PTR ");
      break;
    case m_mode:
      if (ins->address_mode == mode_64bit)
	oappend (ins, "QWORD PTR ");
      else
	oappend (ins, "DWORD PTR ");
      break;
    case f_mode:
      if (sizeflag & DFLAG)
	oappend (ins, "FWORD PTR ");
      else
	oappend (ins, "DWORD PTR ");
      ins->used_prefixes |= (ins->prefixes & PREFIX_DATA);
      break;
    case t_mode:
      oappend (ins, "TBYTE PTR ");
      break;
    case x_mode:
    case xh_mode:
    case x_swap_mode:
    case evex_x_gscat_mode:
    case evex_x_nobcst_mode:
    case bw_unit_mode:
      if (ins->need_vex)
	{
	  switch (ins->vex.length)
	    {
	    case 128:
	      oappend (ins, "XMMWORD PTR ");
	      break;
	    case 256:
	      oappend (ins, "YMMWORD PTR ");
	      break;
	    case 512:
	      oappend (ins, "ZMMWORD PTR ");
	      break;
	    default:
	      abort ();
	    }
	}
      else
	oappend (ins, "XMMWORD PTR ");
      break;
    case xmm_mode:
      oappend (ins, "XMMWORD PTR ");
      break;
    case ymm_mode:
      oappend (ins, "YMMWORD PTR ");
      break;
    case xmmq_mode:
    case evex_half_bcst_xmmqh_mode:
    case evex_half_bcst_xmmq_mode:
      if (!ins->need_vex)
	abort ();

      switch (ins->vex.length)
	{
	case 128:
	  oappend (ins, "QWORD PTR ");
	  break;
	case 256:
	  oappend (ins, "XMMWORD PTR ");
	  break;
	case 512:
	  oappend (ins, "YMMWORD PTR ");
	  break;
	default:
	  abort ();
	}
      break;
    case xmmdw_mode:
      if (!ins->need_vex)
	abort ();

      switch (ins->vex.length)
	{
	case 128:
	  oappend (ins, "WORD PTR ");
	  break;
	case 256:
	  oappend (ins, "DWORD PTR ");
	  break;
	case 512:
	  oappend (ins, "QWORD PTR ");
	  break;
	default:
	  abort ();
	}
      break;
    case xmmqd_mode:
    case evex_half_bcst_xmmqdh_mode:
      if (!ins->need_vex)
	abort ();

      switch (ins->vex.length)
	{
	case 128:
	  oappend (ins, "DWORD PTR ");
	  break;
	case 256:
	  oappend (ins, "QWORD PTR ");
	  break;
	case 512:
	  oappend (ins, "XMMWORD PTR ");
	  break;
	default:
	  abort ();
	}
      break;
    case ymmq_mode:
      if (!ins->need_vex)
	abort ();

      switch (ins->vex.length)
	{
	case 128:
	  oappend (ins, "QWORD PTR ");
	  break;
	case 256:
	  oappend (ins, "YMMWORD PTR ");
	  break;
	case 512:
	  oappend (ins, "ZMMWORD PTR ");
	  break;
	default:
	  abort ();
	}
      break;
    case o_mode:
      oappend (ins, "OWORD PTR ");
      break;
    case vex_vsib_d_w_dq_mode:
    case vex_vsib_q_w_dq_mode:
      if (!ins->need_vex)
	abort ();
      if (ins->vex.w)
	oappend (ins, "QWORD PTR ");
      else
	oappend (ins, "DWORD PTR ");
      break;
    case mask_bd_mode:
      if (!ins->need_vex || ins->vex.length != 128)
	abort ();
      if (ins->vex.w)
	oappend (ins, "DWORD PTR ");
      else
	oappend (ins, "BYTE PTR ");
      break;
    case mask_mode:
      if (!ins->need_vex)
	abort ();
      if (ins->vex.w)
	oappend (ins, "QWORD PTR ");
      else
	oappend (ins, "WORD PTR ");
      break;
    case v_bnd_mode:
    case v_bndmk_mode:
    default:
      break;
    }
}

static void
print_register (instr_info *ins, unsigned int reg, unsigned int rexmask,
		int bytemode, int sizeflag)
{
  const char (*names)[8];

  USED_REX (rexmask);
  if (ins->rex & rexmask)
    reg += 8;

  switch (bytemode)
    {
    case b_mode:
    case b_swap_mode:
      if (reg & 4)
	USED_REX (0);
      if (ins->rex)
	names = att_names8rex;
      else
	names = att_names8;
      break;
    case w_mode:
      names = att_names16;
      break;
    case d_mode:
    case dw_mode:
    case db_mode:
      names = att_names32;
      break;
    case q_mode:
      names = att_names64;
      break;
    case m_mode:
    case v_bnd_mode:
      names = ins->address_mode == mode_64bit ? att_names64 : att_names32;
      break;
    case bnd_mode:
    case bnd_swap_mode:
      if (reg > 0x3)
	{
	  oappend (ins, "(bad)");
	  return;
	}
      names = att_names_bnd;
      break;
    case indir_v_mode:
      if (ins->address_mode == mode_64bit && ins->isa64 == intel64)
	{
	  names = att_names64;
	  break;
	}
      /* Fall through.  */
    case stack_v_mode:
      if (ins->address_mode == mode_64bit && ((sizeflag & DFLAG)
					      || (ins->rex & REX_W)))
	{
	  names = att_names64;
	  break;
	}
      bytemode = v_mode;
      /* Fall through.  */
    case v_mode:
    case v_swap_mode:
    case dq_mode:
      USED_REX (REX_W);
      if (ins->rex & REX_W)
	names = att_names64;
      else if (bytemode != v_mode && bytemode != v_swap_mode)
	names = att_names32;
      else
	{
	  if (sizeflag & DFLAG)
	    names = att_names32;
	  else
	    names = att_names16;
	  ins->used_prefixes |= (ins->prefixes & PREFIX_DATA);
	}
      break;
    case movsxd_mode:
      if (!(sizeflag & DFLAG) && ins->isa64 == intel64)
	names = att_names16;
      else
	names = att_names32;
      ins->used_prefixes |= (ins->prefixes & PREFIX_DATA);
      break;
    case va_mode:
      names = (ins->address_mode == mode_64bit
	       ? att_names64 : att_names32);
      if (!(ins->prefixes & PREFIX_ADDR))
	names = (ins->address_mode == mode_16bit
		     ? att_names16 : names);
      else
	{
	  /* Remove "addr16/addr32".  */
	  ins->all_prefixes[ins->last_addr_prefix] = 0;
	  names = (ins->address_mode != mode_32bit
		       ? att_names32 : att_names16);
	  ins->used_prefixes |= PREFIX_ADDR;
	}
      break;
    case mask_bd_mode:
    case mask_mode:
      if (reg > 0x7)
	{
	  oappend (ins, "(bad)");
	  return;
	}
      names = att_names_mask;
      break;
    case 0:
      return;
    default:
      oappend (ins, INTERNAL_DISASSEMBLER_ERROR);
      return;
    }
  oappend_register (ins, names[reg]);
}

static bool
get8s (instr_info *ins, bfd_vma *res)
{
  if (!fetch_code (ins->info, ins->codep + 1))
    return false;
  *res = ((bfd_vma) *ins->codep++ ^ 0x80) - 0x80;
  return true;
}

static bool
get16 (instr_info *ins, bfd_vma *res)
{
  if (!fetch_code (ins->info, ins->codep + 2))
    return false;
  *res = *ins->codep++;
  *res |= (bfd_vma) *ins->codep++ << 8;
  return true;
}

static bool
get16s (instr_info *ins, bfd_vma *res)
{
  if (!get16 (ins, res))
    return false;
  *res = (*res ^ 0x8000) - 0x8000;
  return true;
}

static bool
get32 (instr_info *ins, bfd_vma *res)
{
  if (!fetch_code (ins->info, ins->codep + 4))
    return false;
  *res = *ins->codep++;
  *res |= (bfd_vma) *ins->codep++ << 8;
  *res |= (bfd_vma) *ins->codep++ << 16;
  *res |= (bfd_vma) *ins->codep++ << 24;
  return true;
}

static bool
get32s (instr_info *ins, bfd_vma *res)
{
  if (!get32 (ins, res))
    return false;

  *res = (*res ^ ((bfd_vma) 1 << 31)) - ((bfd_vma) 1 << 31);

  return true;
}

static bool
get64 (instr_info *ins, uint64_t *res)
{
  unsigned int a;
  unsigned int b;

  if (!fetch_code (ins->info, ins->codep + 8))
    return false;
  a = *ins->codep++;
  a |= (unsigned int) *ins->codep++ << 8;
  a |= (unsigned int) *ins->codep++ << 16;
  a |= (unsigned int) *ins->codep++ << 24;
  b = *ins->codep++;
  b |= (unsigned int) *ins->codep++ << 8;
  b |= (unsigned int) *ins->codep++ << 16;
  b |= (unsigned int) *ins->codep++ << 24;
  *res = a + ((uint64_t) b << 32);
  return true;
}

static void
set_op (instr_info *ins, bfd_vma op, bool riprel)
{
  ins->op_index[ins->op_ad] = ins->op_ad;
  if (ins->address_mode == mode_64bit)
    ins->op_address[ins->op_ad] = op;
  else /* Mask to get a 32-bit address.  */
    ins->op_address[ins->op_ad] = op & 0xffffffff;
  ins->op_riprel[ins->op_ad] = riprel;
}

static bool
BadOp (instr_info *ins)
{
  /* Throw away prefixes and 1st. opcode byte.  */
  struct dis_private *priv = ins->info->private_data;

  ins->codep = priv->the_buffer + ins->nr_prefixes + 1;
  ins->obufp = stpcpy (ins->obufp, "(bad)");
  return true;
}

static bool
OP_E_memory (instr_info *ins, int bytemode, int sizeflag)
{
  int add = (ins->rex & REX_B) ? 8 : 0;
  int riprel = 0;
  int shift;

  if (ins->vex.evex)
    {
      switch (bytemode)
	{
	case dw_mode:
	case w_mode:
	case w_swap_mode:
	  shift = 1;
	  break;
	case db_mode:
	case b_mode:
	  shift = 0;
	  break;
	case dq_mode:
	  if (ins->address_mode != mode_64bit)
	    {
	case d_mode:
	case d_swap_mode:
	      shift = 2;
	      break;
	    }
	    /* fall through */
	case vex_vsib_d_w_dq_mode:
	case vex_vsib_q_w_dq_mode:
	case evex_x_gscat_mode:
	  shift = ins->vex.w ? 3 : 2;
	  break;
	case xh_mode:
	case evex_half_bcst_xmmqh_mode:
	case evex_half_bcst_xmmqdh_mode:
	  if (ins->vex.b)
	    {
	      shift = ins->vex.w ? 2 : 1;
	      break;
	    }
	  /* Fall through.  */
	case x_mode:
	case evex_half_bcst_xmmq_mode:
	  if (ins->vex.b)
	    {
	      shift = ins->vex.w ? 3 : 2;
	      break;
	    }
	  /* Fall through.  */
	case xmmqd_mode:
	case xmmdw_mode:
	case xmmq_mode:
	case ymmq_mode:
	case evex_x_nobcst_mode:
	case x_swap_mode:
	  switch (ins->vex.length)
	    {
	    case 128:
	      shift = 4;
	      break;
	    case 256:
	      shift = 5;
	      break;
	    case 512:
	      shift = 6;
	      break;
	    default:
	      abort ();
	    }
	  /* Make necessary corrections to shift for modes that need it.  */
	  if (bytemode == xmmq_mode
	      || bytemode == evex_half_bcst_xmmqh_mode
	      || bytemode == evex_half_bcst_xmmq_mode
	      || (bytemode == ymmq_mode && ins->vex.length == 128))
	    shift -= 1;
	  else if (bytemode == xmmqd_mode
	           || bytemode == evex_half_bcst_xmmqdh_mode)
	    shift -= 2;
	  else if (bytemode == xmmdw_mode)
	    shift -= 3;
	  break;
	case ymm_mode:
	  shift = 5;
	  break;
	case xmm_mode:
	  shift = 4;
	  break;
	case q_mode:
	case q_swap_mode:
	  shift = 3;
	  break;
	case bw_unit_mode:
	  shift = ins->vex.w ? 1 : 0;
	  break;
	default:
	  abort ();
	}
    }
  else
    shift = 0;

  USED_REX (REX_B);
  if (ins->intel_syntax)
    intel_operand_size (ins, bytemode, sizeflag);
  append_seg (ins);

  if ((sizeflag & AFLAG) || ins->address_mode == mode_64bit)
    {
      /* 32/64 bit address mode */
      bfd_vma disp = 0;
      int havedisp;
      int havebase;
      int needindex;
      int needaddr32;
      int base, rbase;
      int vindex = 0;
      int scale = 0;
      int addr32flag = !((sizeflag & AFLAG)
			 || bytemode == v_bnd_mode
			 || bytemode == v_bndmk_mode
			 || bytemode == bnd_mode
			 || bytemode == bnd_swap_mode);
      bool check_gather = false;
      const char (*indexes)[8] = NULL;

      havebase = 1;
      base = ins->modrm.rm;

      if (base == 4)
	{
	  vindex = ins->sib.index;
	  USED_REX (REX_X);
	  if (ins->rex & REX_X)
	    vindex += 8;
	  switch (bytemode)
	    {
	    case vex_vsib_d_w_dq_mode:
	    case vex_vsib_q_w_dq_mode:
	      if (!ins->need_vex)
		abort ();
	      if (ins->vex.evex)
		{
		  if (!ins->vex.v)
		    vindex += 16;
		  check_gather = ins->obufp == ins->op_out[1];
		}

	      switch (ins->vex.length)
		{
		case 128:
		  indexes = att_names_xmm;
		  break;
		case 256:
		  if (!ins->vex.w
		      || bytemode == vex_vsib_q_w_dq_mode)
		    indexes = att_names_ymm;
		  else
		    indexes = att_names_xmm;
		  break;
		case 512:
		  if (!ins->vex.w
		      || bytemode == vex_vsib_q_w_dq_mode)
		    indexes = att_names_zmm;
		  else
		    indexes = att_names_ymm;
		  break;
		default:
		  abort ();
		}
	      break;
	    default:
	      if (vindex != 4)
		indexes = ins->address_mode == mode_64bit && !addr32flag
			  ? att_names64 : att_names32;
	      break;
	    }
	  scale = ins->sib.scale;
	  base = ins->sib.base;
	  ins->codep++;
	}
      else
	{
	  /* Check for mandatory SIB.  */
	  if (bytemode == vex_vsib_d_w_dq_mode
	      || bytemode == vex_vsib_q_w_dq_mode
	      || bytemode == vex_sibmem_mode)
	    {
	      oappend (ins, "(bad)");
	      return true;
	    }
	}
      rbase = base + add;

      switch (ins->modrm.mod)
	{
	case 0:
	  if (base == 5)
	    {
	      havebase = 0;
	      if (ins->address_mode == mode_64bit && !ins->has_sib)
		riprel = 1;
	      if (!get32s (ins, &disp))
		return false;
	      if (riprel && bytemode == v_bndmk_mode)
		{
		  oappend (ins, "(bad)");
		  return true;
		}
	    }
	  break;
	case 1:
	  if (!get8s (ins, &disp))
	    return false;
	  if (ins->vex.evex && shift > 0)
	    disp <<= shift;
	  break;
	case 2:
	  if (!get32s (ins, &disp))
	    return false;
	  break;
	}

      needindex = 0;
      needaddr32 = 0;
      if (ins->has_sib
	  && !havebase
	  && !indexes
	  && ins->address_mode != mode_16bit)
	{
	  if (ins->address_mode == mode_64bit)
	    {
	      if (addr32flag)
		{
		  /* Without base nor index registers, zero-extend the
		     lower 32-bit displacement to 64 bits.  */
		  disp &= 0xffffffff;
		  needindex = 1;
		}
	      needaddr32 = 1;
	    }
	  else
	    {
	      /* In 32-bit mode, we need index register to tell [offset]
		 from [eiz*1 + offset].  */
	      needindex = 1;
	    }
	}

      havedisp = (havebase
		  || needindex
		  || (ins->has_sib && (indexes || scale != 0)));

      if (!ins->intel_syntax)
	if (ins->modrm.mod != 0 || base == 5)
	  {
	    if (havedisp || riprel)
	      print_displacement (ins, disp);
	    else
	      print_operand_value (ins, disp, dis_style_address_offset);
	    if (riprel)
	      {
		set_op (ins, disp, true);
		oappend_char (ins, '(');
		oappend_with_style (ins, !addr32flag ? "%rip" : "%eip",
				    dis_style_register);
		oappend_char (ins, ')');
	      }
	  }

      if ((havebase || indexes || needindex || needaddr32 || riprel)
	  && (ins->address_mode != mode_64bit
	      || ((bytemode != v_bnd_mode)
		  && (bytemode != v_bndmk_mode)
		  && (bytemode != bnd_mode)
		  && (bytemode != bnd_swap_mode))))
	ins->used_prefixes |= PREFIX_ADDR;

      if (havedisp || (ins->intel_syntax && riprel))
	{
	  oappend_char (ins, ins->open_char);
	  if (ins->intel_syntax && riprel)
	    {
	      set_op (ins, disp, true);
	      oappend_with_style (ins, !addr32flag ? "rip" : "eip",
				  dis_style_register);
	    }
	  if (havebase)
	    oappend_register
	      (ins,
	       (ins->address_mode == mode_64bit && !addr32flag
		? att_names64 : att_names32)[rbase]);
	  if (ins->has_sib)
	    {
	      /* ESP/RSP won't allow index.  If base isn't ESP/RSP,
		 print index to tell base + index from base.  */
	      if (scale != 0
		  || needindex
		  || indexes
		  || (havebase && base != ESP_REG_NUM))
		{
		  if (!ins->intel_syntax || havebase)
		    oappend_char (ins, ins->separator_char);
		  if (indexes)
		    {
		      if (ins->address_mode == mode_64bit || vindex < 16)
			oappend_register (ins, indexes[vindex]);
		      else
			oappend (ins, "(bad)");
		    }
		  else
		    oappend_register (ins,
				      ins->address_mode == mode_64bit
				      && !addr32flag
				      ? att_index64
				      : att_index32);

		  oappend_char (ins, ins->scale_char);
		  oappend_char_with_style (ins, '0' + (1 << scale),
					   dis_style_immediate);
		}
	    }
	  if (ins->intel_syntax
	      && (disp || ins->modrm.mod != 0 || base == 5))
	    {
	      if (!havedisp || (bfd_signed_vma) disp >= 0)
		  oappend_char (ins, '+');
	      if (havedisp)
		print_displacement (ins, disp);
	      else
		print_operand_value (ins, disp, dis_style_address_offset);
	    }

	  oappend_char (ins, ins->close_char);

	  if (check_gather)
	    {
	      /* Both XMM/YMM/ZMM registers must be distinct.  */
	      int modrm_reg = ins->modrm.reg;

	      if (ins->rex & REX_R)
	        modrm_reg += 8;
	      if (!ins->vex.r)
	        modrm_reg += 16;
	      if (vindex == modrm_reg)
		oappend (ins, "/(bad)");
	    }
	}
      else if (ins->intel_syntax)
	{
	  if (ins->modrm.mod != 0 || base == 5)
	    {
	      if (!ins->active_seg_prefix)
		{
		  oappend_register (ins, att_names_seg[ds_reg - es_reg]);
		  oappend (ins, ":");
		}
	      print_operand_value (ins, disp, dis_style_text);
	    }
	}
    }
  else if (bytemode == v_bnd_mode
	   || bytemode == v_bndmk_mode
	   || bytemode == bnd_mode
	   || bytemode == bnd_swap_mode
	   || bytemode == vex_vsib_d_w_dq_mode
	   || bytemode == vex_vsib_q_w_dq_mode)
    {
      oappend (ins, "(bad)");
      return true;
    }
  else
    {
      /* 16 bit address mode */
      bfd_vma disp = 0;

      ins->used_prefixes |= ins->prefixes & PREFIX_ADDR;
      switch (ins->modrm.mod)
	{
	case 0:
	  if (ins->modrm.rm == 6)
	    {
	case 2:
	      if (!get16s (ins, &disp))
		return false;
	    }
	  break;
	case 1:
	  if (!get8s (ins, &disp))
	    return false;
	  if (ins->vex.evex && shift > 0)
	    disp <<= shift;
	  break;
	}

      if (!ins->intel_syntax)
	if (ins->modrm.mod != 0 || ins->modrm.rm == 6)
	  print_displacement (ins, disp);

      if (ins->modrm.mod != 0 || ins->modrm.rm != 6)
	{
	  oappend_char (ins, ins->open_char);
	  oappend (ins, ins->intel_syntax ? intel_index16[ins->modrm.rm]
					  : att_index16[ins->modrm.rm]);
	  if (ins->intel_syntax
	      && (disp || ins->modrm.mod != 0 || ins->modrm.rm == 6))
	    {
	      if ((bfd_signed_vma) disp >= 0)
		oappend_char (ins, '+');
	      print_displacement (ins, disp);
	    }

	  oappend_char (ins, ins->close_char);
	}
      else if (ins->intel_syntax)
	{
	  if (!ins->active_seg_prefix)
	    {
	      oappend_register (ins, att_names_seg[ds_reg - es_reg]);
	      oappend (ins, ":");
	    }
	  print_operand_value (ins, disp & 0xffff, dis_style_text);
	}
    }
  if (ins->vex.b)
    {
      ins->evex_used |= EVEX_b_used;

      /* Broadcast can only ever be valid for memory sources.  */
      if (ins->obufp == ins->op_out[0])
	ins->vex.no_broadcast = true;

      if (!ins->vex.no_broadcast
	  && (!ins->intel_syntax || !(ins->evex_used & EVEX_len_used)))
	{
	  if (bytemode == xh_mode)
	    {
	      switch (ins->vex.length)
		{
		case 128:
		  oappend (ins, "{1to8}");
		  break;
		case 256:
		  oappend (ins, "{1to16}");
		  break;
		case 512:
		  oappend (ins, "{1to32}");
		  break;
		default:
		  abort ();
		}
	    }
	  else if (bytemode == q_mode
		   || bytemode == ymmq_mode)
	    ins->vex.no_broadcast = true;
	  else if (ins->vex.w
		   || bytemode == evex_half_bcst_xmmqdh_mode
		   || bytemode == evex_half_bcst_xmmq_mode)
	    {
	      switch (ins->vex.length)
		{
		case 128:
		  oappend (ins, "{1to2}");
		  break;
		case 256:
		  oappend (ins, "{1to4}");
		  break;
		case 512:
		  oappend (ins, "{1to8}");
		  break;
		default:
		  abort ();
		}
	    }
	  else if (bytemode == x_mode
		   || bytemode == evex_half_bcst_xmmqh_mode)
	    {
	      switch (ins->vex.length)
		{
		case 128:
		  oappend (ins, "{1to4}");
		  break;
		case 256:
		  oappend (ins, "{1to8}");
		  break;
		case 512:
		  oappend (ins, "{1to16}");
		  break;
		default:
		  abort ();
		}
	    }
	  else
	    ins->vex.no_broadcast = true;
	}
      if (ins->vex.no_broadcast)
	oappend (ins, "{bad}");
    }

  return true;
}

static bool
OP_E (instr_info *ins, int bytemode, int sizeflag)
{
  /* Skip mod/rm byte.  */
  MODRM_CHECK;
  ins->codep++;

  if (ins->modrm.mod == 3)
    {
      if ((sizeflag & SUFFIX_ALWAYS)
	  && (bytemode == b_swap_mode
	      || bytemode == bnd_swap_mode
	      || bytemode == v_swap_mode))
	swap_operand (ins);

      print_register (ins, ins->modrm.rm, REX_B, bytemode, sizeflag);
      return true;
    }

  return OP_E_memory (ins, bytemode, sizeflag);
}

static bool
OP_G (instr_info *ins, int bytemode, int sizeflag)
{
  if (ins->vex.evex && !ins->vex.r && ins->address_mode == mode_64bit)
    oappend (ins, "(bad)");
  else
    print_register (ins, ins->modrm.reg, REX_R, bytemode, sizeflag);
  return true;
}

static bool
OP_REG (instr_info *ins, int code, int sizeflag)
{
  const char *s;
  int add;

  switch (code)
    {
    case es_reg: case ss_reg: case cs_reg:
    case ds_reg: case fs_reg: case gs_reg:
      oappend_register (ins, att_names_seg[code - es_reg]);
      return true;
    }

  USED_REX (REX_B);
  if (ins->rex & REX_B)
    add = 8;
  else
    add = 0;

  switch (code)
    {
    case ax_reg: case cx_reg: case dx_reg: case bx_reg:
    case sp_reg: case bp_reg: case si_reg: case di_reg:
      s = att_names16[code - ax_reg + add];
      break;
    case ah_reg: case ch_reg: case dh_reg: case bh_reg:
      USED_REX (0);
      /* Fall through.  */
    case al_reg: case cl_reg: case dl_reg: case bl_reg:
      if (ins->rex)
	s = att_names8rex[code - al_reg + add];
      else
	s = att_names8[code - al_reg];
      break;
    case rAX_reg: case rCX_reg: case rDX_reg: case rBX_reg:
    case rSP_reg: case rBP_reg: case rSI_reg: case rDI_reg:
      if (ins->address_mode == mode_64bit
	  && ((sizeflag & DFLAG) || (ins->rex & REX_W)))
	{
	  s = att_names64[code - rAX_reg + add];
	  break;
	}
      code += eAX_reg - rAX_reg;
      /* Fall through.  */
    case eAX_reg: case eCX_reg: case eDX_reg: case eBX_reg:
    case eSP_reg: case eBP_reg: case eSI_reg: case eDI_reg:
      USED_REX (REX_W);
      if (ins->rex & REX_W)
	s = att_names64[code - eAX_reg + add];
      else
	{
	  if (sizeflag & DFLAG)
	    s = att_names32[code - eAX_reg + add];
	  else
	    s = att_names16[code - eAX_reg + add];
	  ins->used_prefixes |= (ins->prefixes & PREFIX_DATA);
	}
      break;
    default:
      oappend (ins, INTERNAL_DISASSEMBLER_ERROR);
      return true;
    }
  oappend_register (ins, s);
  return true;
}

static bool
OP_IMREG (instr_info *ins, int code, int sizeflag)
{
  const char *s;

  switch (code)
    {
    case indir_dx_reg:
      if (!ins->intel_syntax)
	{
	  oappend (ins, "(%dx)");
	  return true;
	}
      s = att_names16[dx_reg - ax_reg];
      break;
    case al_reg: case cl_reg:
      s = att_names8[code - al_reg];
      break;
    case eAX_reg:
      USED_REX (REX_W);
      if (ins->rex & REX_W)
	{
	  s = *att_names64;
	  break;
	}
      /* Fall through.  */
    case z_mode_ax_reg:
      if ((ins->rex & REX_W) || (sizeflag & DFLAG))
	s = *att_names32;
      else
	s = *att_names16;
      if (!(ins->rex & REX_W))
	ins->used_prefixes |= (ins->prefixes & PREFIX_DATA);
      break;
    default:
      oappend (ins, INTERNAL_DISASSEMBLER_ERROR);
      return true;
    }
  oappend_register (ins, s);
  return true;
}

static bool
OP_I (instr_info *ins, int bytemode, int sizeflag)
{
  bfd_vma op;

  switch (bytemode)
    {
    case b_mode:
      if (!fetch_code (ins->info, ins->codep + 1))
	return false;
      op = *ins->codep++;
      break;
    case v_mode:
      USED_REX (REX_W);
      if (ins->rex & REX_W)
	{
	  if (!get32s (ins, &op))
	    return false;
	}
      else
	{
	  ins->used_prefixes |= (ins->prefixes & PREFIX_DATA);
	  if (sizeflag & DFLAG)
	    {
    case d_mode:
	      if (!get32 (ins, &op))
		return false;
	    }
	  else
	    {
	      /* Fall through.  */
    case w_mode:
	      if (!get16 (ins, &op))
		return false;
	    }
	}
      break;
    case const_1_mode:
      if (ins->intel_syntax)
	oappend (ins, "1");
      return true;
    default:
      oappend (ins, INTERNAL_DISASSEMBLER_ERROR);
      return true;
    }

  oappend_immediate (ins, op);
  return true;
}

static bool
OP_I64 (instr_info *ins, int bytemode, int sizeflag)
{
  uint64_t op;

  if (bytemode != v_mode || ins->address_mode != mode_64bit
      || !(ins->rex & REX_W))
    return OP_I (ins, bytemode, sizeflag);

  USED_REX (REX_W);

  if (!get64 (ins, &op))
    return false;

  oappend_immediate (ins, op);
  return true;
}

static bool
OP_sI (instr_info *ins, int bytemode, int sizeflag)
{
  bfd_vma op;

  switch (bytemode)
    {
    case b_mode:
    case b_T_mode:
      if (!get8s (ins, &op))
	return false;
      if (bytemode == b_T_mode)
	{
	  if (ins->address_mode != mode_64bit
	      || !((sizeflag & DFLAG) || (ins->rex & REX_W)))
	    {
	      /* The operand-size prefix is overridden by a REX prefix.  */
	      if ((sizeflag & DFLAG) || (ins->rex & REX_W))
		op &= 0xffffffff;
	      else
		op &= 0xffff;
	  }
	}
      else
	{
	  if (!(ins->rex & REX_W))
	    {
	      if (sizeflag & DFLAG)
		op &= 0xffffffff;
	      else
		op &= 0xffff;
	    }
	}
      break;
    case v_mode:
      /* The operand-size prefix is overridden by a REX prefix.  */
      if (!(sizeflag & DFLAG) && !(ins->rex & REX_W))
	{
	  if (!get16 (ins, &op))
	    return false;
	}
      else if (!get32s (ins, &op))
	return false;
      break;
    default:
      oappend (ins, INTERNAL_DISASSEMBLER_ERROR);
      return true;
    }

  oappend_immediate (ins, op);
  return true;
}

static bool
OP_J (instr_info *ins, int bytemode, int sizeflag)
{
  bfd_vma disp;
  bfd_vma mask = -1;
  bfd_vma segment = 0;

  switch (bytemode)
    {
    case b_mode:
      if (!get8s (ins, &disp))
	return false;
      break;
    case v_mode:
    case dqw_mode:
      if ((sizeflag & DFLAG)
	  || (ins->address_mode == mode_64bit
	      && ((ins->isa64 == intel64 && bytemode != dqw_mode)
		  || (ins->rex & REX_W))))
	{
	  if (!get32s (ins, &disp))
	    return false;
	}
      else
	{
	  if (!get16s (ins, &disp))
	    return false;
	  /* In 16bit mode, address is wrapped around at 64k within
	     the same segment.  Otherwise, a data16 prefix on a jump
	     instruction means that the pc is masked to 16 bits after
	     the displacement is added!  */
	  mask = 0xffff;
	  if ((ins->prefixes & PREFIX_DATA) == 0)
	    segment = ((ins->start_pc + (ins->codep - ins->start_codep))
		       & ~((bfd_vma) 0xffff));
	}
      if (ins->address_mode != mode_64bit
	  || (ins->isa64 != intel64 && !(ins->rex & REX_W)))
	ins->used_prefixes |= (ins->prefixes & PREFIX_DATA);
      break;
    default:
      oappend (ins, INTERNAL_DISASSEMBLER_ERROR);
      return true;
    }
  disp = ((ins->start_pc + (ins->codep - ins->start_codep) + disp) & mask)
	 | segment;
  set_op (ins, disp, false);
  print_operand_value (ins, disp, dis_style_text);
  return true;
}

static bool
OP_SEG (instr_info *ins, int bytemode, int sizeflag)
{
  if (bytemode == w_mode)
    {
      oappend_register (ins, att_names_seg[ins->modrm.reg]);
      return true;
    }
  return OP_E (ins, ins->modrm.mod == 3 ? bytemode : w_mode, sizeflag);
}

static bool
OP_DIR (instr_info *ins, int dummy ATTRIBUTE_UNUSED, int sizeflag)
{
  bfd_vma seg, offset;
  int res;
  char scratch[24];

  if (sizeflag & DFLAG)
    {
      if (!get32 (ins, &offset))
	return false;;
    }
  else if (!get16 (ins, &offset))
    return false;
  if (!get16 (ins, &seg))
    return false;;
  ins->used_prefixes |= (ins->prefixes & PREFIX_DATA);

  res = snprintf (scratch, ARRAY_SIZE (scratch),
		  ins->intel_syntax ? "0x%x:0x%x" : "$0x%x,$0x%x",
		  (unsigned) seg, (unsigned) offset);
  if (res < 0 || (size_t) res >= ARRAY_SIZE (scratch))
    abort ();
  oappend (ins, scratch);
  return true;
}

static bool
OP_OFF (instr_info *ins, int bytemode, int sizeflag)
{
  bfd_vma off;

  if (ins->intel_syntax && (sizeflag & SUFFIX_ALWAYS))
    intel_operand_size (ins, bytemode, sizeflag);
  append_seg (ins);

  if ((sizeflag & AFLAG) || ins->address_mode == mode_64bit)
    {
      if (!get32 (ins, &off))
	return false;
    }
  else
    {
      if (!get16 (ins, &off))
	return false;
    }

  if (ins->intel_syntax)
    {
      if (!ins->active_seg_prefix)
	{
	  oappend_register (ins, att_names_seg[ds_reg - es_reg]);
	  oappend (ins, ":");
	}
    }
  print_operand_value (ins, off, dis_style_address_offset);
  return true;
}

static bool
OP_OFF64 (instr_info *ins, int bytemode, int sizeflag)
{
  uint64_t off;

  if (ins->address_mode != mode_64bit
      || (ins->prefixes & PREFIX_ADDR))
    return OP_OFF (ins, bytemode, sizeflag);

  if (ins->intel_syntax && (sizeflag & SUFFIX_ALWAYS))
    intel_operand_size (ins, bytemode, sizeflag);
  append_seg (ins);

  if (!get64 (ins, &off))
    return false;

  if (ins->intel_syntax)
    {
      if (!ins->active_seg_prefix)
	{
	  oappend_register (ins, att_names_seg[ds_reg - es_reg]);
	  oappend (ins, ":");
	}
    }
  print_operand_value (ins, off, dis_style_address_offset);
  return true;
}

static void
ptr_reg (instr_info *ins, int code, int sizeflag)
{
  const char *s;

  *ins->obufp++ = ins->open_char;
  ins->used_prefixes |= (ins->prefixes & PREFIX_ADDR);
  if (ins->address_mode == mode_64bit)
    {
      if (!(sizeflag & AFLAG))
	s = att_names32[code - eAX_reg];
      else
	s = att_names64[code - eAX_reg];
    }
  else if (sizeflag & AFLAG)
    s = att_names32[code - eAX_reg];
  else
    s = att_names16[code - eAX_reg];
  oappend_register (ins, s);
  oappend_char (ins, ins->close_char);
}

static bool
OP_ESreg (instr_info *ins, int code, int sizeflag)
{
  if (ins->intel_syntax)
    {
      switch (ins->codep[-1])
	{
	case 0x6d:	/* insw/insl */
	  intel_operand_size (ins, z_mode, sizeflag);
	  break;
	case 0xa5:	/* movsw/movsl/movsq */
	case 0xa7:	/* cmpsw/cmpsl/cmpsq */
	case 0xab:	/* stosw/stosl */
	case 0xaf:	/* scasw/scasl */
	  intel_operand_size (ins, v_mode, sizeflag);
	  break;
	default:
	  intel_operand_size (ins, b_mode, sizeflag);
	}
    }
  oappend_register (ins, att_names_seg[0]);
  oappend_char (ins, ':');
  ptr_reg (ins, code, sizeflag);
  return true;
}

static bool
OP_DSreg (instr_info *ins, int code, int sizeflag)
{
  if (ins->intel_syntax)
    {
      switch (ins->codep[-1])
	{
	case 0x6f:	/* outsw/outsl */
	  intel_operand_size (ins, z_mode, sizeflag);
	  break;
	case 0xa5:	/* movsw/movsl/movsq */
	case 0xa7:	/* cmpsw/cmpsl/cmpsq */
	case 0xad:	/* lodsw/lodsl/lodsq */
	  intel_operand_size (ins, v_mode, sizeflag);
	  break;
	default:
	  intel_operand_size (ins, b_mode, sizeflag);
	}
    }
  /* Set ins->active_seg_prefix to PREFIX_DS if it is unset so that the
     default segment register DS is printed.  */
  if (!ins->active_seg_prefix)
    ins->active_seg_prefix = PREFIX_DS;
  append_seg (ins);
  ptr_reg (ins, code, sizeflag);
  return true;
}

static bool
OP_C (instr_info *ins, int dummy ATTRIBUTE_UNUSED,
      int sizeflag ATTRIBUTE_UNUSED)
{
  int add, res;
  char scratch[8];

  if (ins->rex & REX_R)
    {
      USED_REX (REX_R);
      add = 8;
    }
  else if (ins->address_mode != mode_64bit && (ins->prefixes & PREFIX_LOCK))
    {
      ins->all_prefixes[ins->last_lock_prefix] = 0;
      ins->used_prefixes |= PREFIX_LOCK;
      add = 8;
    }
  else
    add = 0;
  res = snprintf (scratch, ARRAY_SIZE (scratch), "%%cr%d",
		  ins->modrm.reg + add);
  if (res < 0 || (size_t) res >= ARRAY_SIZE (scratch))
    abort ();
  oappend_register (ins, scratch);
  return true;
}

static bool
OP_D (instr_info *ins, int dummy ATTRIBUTE_UNUSED,
      int sizeflag ATTRIBUTE_UNUSED)
{
  int add, res;
  char scratch[8];

  USED_REX (REX_R);
  if (ins->rex & REX_R)
    add = 8;
  else
    add = 0;
  res = snprintf (scratch, ARRAY_SIZE (scratch),
		  ins->intel_syntax ? "dr%d" : "%%db%d",
		  ins->modrm.reg + add);
  if (res < 0 || (size_t) res >= ARRAY_SIZE (scratch))
    abort ();
  oappend (ins, scratch);
  return true;
}

static bool
OP_T (instr_info *ins, int dummy ATTRIBUTE_UNUSED,
      int sizeflag ATTRIBUTE_UNUSED)
{
  int res;
  char scratch[8];

  res = snprintf (scratch, ARRAY_SIZE (scratch), "%%tr%d", ins->modrm.reg);
  if (res < 0 || (size_t) res >= ARRAY_SIZE (scratch))
    abort ();
  oappend_register (ins, scratch);
  return true;
}

static bool
OP_MMX (instr_info *ins, int bytemode ATTRIBUTE_UNUSED,
	int sizeflag ATTRIBUTE_UNUSED)
{
  int reg = ins->modrm.reg;
  const char (*names)[8];

  ins->used_prefixes |= (ins->prefixes & PREFIX_DATA);
  if (ins->prefixes & PREFIX_DATA)
    {
      names = att_names_xmm;
      USED_REX (REX_R);
      if (ins->rex & REX_R)
	reg += 8;
    }
  else
    names = att_names_mm;
  oappend_register (ins, names[reg]);
  return true;
}

static void
print_vector_reg (instr_info *ins, unsigned int reg, int bytemode)
{
  const char (*names)[8];

  if (bytemode == xmmq_mode
      || bytemode == evex_half_bcst_xmmqh_mode
      || bytemode == evex_half_bcst_xmmq_mode)
    {
      switch (ins->vex.length)
	{
	case 128:
	case 256:
	  names = att_names_xmm;
	  break;
	case 512:
	  names = att_names_ymm;
	  ins->evex_used |= EVEX_len_used;
	  break;
	default:
	  abort ();
	}
    }
  else if (bytemode == ymm_mode)
    names = att_names_ymm;
  else if (bytemode == tmm_mode)
    {
      if (reg >= 8)
	{
	  oappend (ins, "(bad)");
	  return;
	}
      names = att_names_tmm;
    }
  else if (ins->need_vex
	   && bytemode != xmm_mode
	   && bytemode != scalar_mode
	   && bytemode != xmmdw_mode
	   && bytemode != xmmqd_mode
	   && bytemode != evex_half_bcst_xmmqdh_mode
	   && bytemode != w_swap_mode
	   && bytemode != b_mode
	   && bytemode != w_mode
	   && bytemode != d_mode
	   && bytemode != q_mode)
    {
      ins->evex_used |= EVEX_len_used;
      switch (ins->vex.length)
	{
	case 128:
	  names = att_names_xmm;
	  break;
	case 256:
	  if (ins->vex.w
	      || bytemode != vex_vsib_q_w_dq_mode)
	    names = att_names_ymm;
	  else
	    names = att_names_xmm;
	  break;
	case 512:
	  if (ins->vex.w
	      || bytemode != vex_vsib_q_w_dq_mode)
	    names = att_names_zmm;
	  else
	    names = att_names_ymm;
	  break;
	default:
	  abort ();
	}
    }
  else
    names = att_names_xmm;
  oappend_register (ins, names[reg]);
}

static bool
OP_XMM (instr_info *ins, int bytemode, int sizeflag ATTRIBUTE_UNUSED)
{
  unsigned int reg = ins->modrm.reg;

  USED_REX (REX_R);
  if (ins->rex & REX_R)
    reg += 8;
  if (ins->vex.evex)
    {
      if (!ins->vex.r)
	reg += 16;
    }

  if (bytemode == tmm_mode)
    ins->modrm.reg = reg;
  else if (bytemode == scalar_mode)
    ins->vex.no_broadcast = true;

  print_vector_reg (ins, reg, bytemode);
  return true;
}

static bool
OP_EM (instr_info *ins, int bytemode, int sizeflag)
{
  int reg;
  const char (*names)[8];

  if (ins->modrm.mod != 3)
    {
      if (ins->intel_syntax
	  && (bytemode == v_mode || bytemode == v_swap_mode))
	{
	  bytemode = (ins->prefixes & PREFIX_DATA) ? x_mode : q_mode;
	  ins->used_prefixes |= (ins->prefixes & PREFIX_DATA);
	}
      return OP_E (ins, bytemode, sizeflag);
    }

  if ((sizeflag & SUFFIX_ALWAYS) && bytemode == v_swap_mode)
    swap_operand (ins);

  /* Skip mod/rm byte.  */
  MODRM_CHECK;
  ins->codep++;
  ins->used_prefixes |= (ins->prefixes & PREFIX_DATA);
  reg = ins->modrm.rm;
  if (ins->prefixes & PREFIX_DATA)
    {
      names = att_names_xmm;
      USED_REX (REX_B);
      if (ins->rex & REX_B)
	reg += 8;
    }
  else
    names = att_names_mm;
  oappend_register (ins, names[reg]);
  return true;
}

/* cvt* are the only instructions in sse2 which have
   both SSE and MMX operands and also have 0x66 prefix
   in their opcode. 0x66 was originally used to differentiate
   between SSE and MMX instruction(operands). So we have to handle the
   cvt* separately using OP_EMC and OP_MXC */
static bool
OP_EMC (instr_info *ins, int bytemode, int sizeflag)
{
  if (ins->modrm.mod != 3)
    {
      if (ins->intel_syntax && bytemode == v_mode)
	{
	  bytemode = (ins->prefixes & PREFIX_DATA) ? x_mode : q_mode;
	  ins->used_prefixes |= (ins->prefixes & PREFIX_DATA);
	}
      return OP_E (ins, bytemode, sizeflag);
    }

  /* Skip mod/rm byte.  */
  MODRM_CHECK;
  ins->codep++;
  ins->used_prefixes |= (ins->prefixes & PREFIX_DATA);
  oappend_register (ins, att_names_mm[ins->modrm.rm]);
  return true;
}

static bool
OP_MXC (instr_info *ins, int bytemode ATTRIBUTE_UNUSED,
	int sizeflag ATTRIBUTE_UNUSED)
{
  ins->used_prefixes |= (ins->prefixes & PREFIX_DATA);
  oappend_register (ins, att_names_mm[ins->modrm.reg]);
  return true;
}

static bool
OP_EX (instr_info *ins, int bytemode, int sizeflag)
{
  int reg;

  /* Skip mod/rm byte.  */
  MODRM_CHECK;
  ins->codep++;

  if (bytemode == dq_mode)
    bytemode = ins->vex.w ? q_mode : d_mode;

  if (ins->modrm.mod != 3)
    return OP_E_memory (ins, bytemode, sizeflag);

  reg = ins->modrm.rm;
  USED_REX (REX_B);
  if (ins->rex & REX_B)
    reg += 8;
  if (ins->vex.evex)
    {
      USED_REX (REX_X);
      if ((ins->rex & REX_X))
	reg += 16;
    }

  if ((sizeflag & SUFFIX_ALWAYS)
      && (bytemode == x_swap_mode
	  || bytemode == w_swap_mode
	  || bytemode == d_swap_mode
	  || bytemode == q_swap_mode))
    swap_operand (ins);

  if (bytemode == tmm_mode)
    ins->modrm.rm = reg;

  print_vector_reg (ins, reg, bytemode);
  return true;
}

static bool
OP_MS (instr_info *ins, int bytemode, int sizeflag)
{
  if (ins->modrm.mod == 3)
    return OP_EM (ins, bytemode, sizeflag);
  return BadOp (ins);
}

static bool
OP_XS (instr_info *ins, int bytemode, int sizeflag)
{
  if (ins->modrm.mod == 3)
    return OP_EX (ins, bytemode, sizeflag);
  return BadOp (ins);
}

static bool
OP_M (instr_info *ins, int bytemode, int sizeflag)
{
  if (ins->modrm.mod == 3)
    /* bad bound,lea,lds,les,lfs,lgs,lss,cmpxchg8b,vmptrst modrm */
    return BadOp (ins);
  return OP_E (ins, bytemode, sizeflag);
}

static bool
OP_0f07 (instr_info *ins, int bytemode, int sizeflag)
{
  if (ins->modrm.mod != 3 || ins->modrm.rm != 0)
    return BadOp (ins);
  return OP_E (ins, bytemode, sizeflag);
}

/* NOP is an alias of "xchg %ax,%ax" in 16bit mode, "xchg %eax,%eax" in
   32bit mode and "xchg %rax,%rax" in 64bit mode.  */

static bool
NOP_Fixup (instr_info *ins, int opnd, int sizeflag)
{
  if ((ins->prefixes & PREFIX_DATA) == 0 && (ins->rex & REX_B) == 0)
    {
      ins->mnemonicendp = stpcpy (ins->obuf, "nop");
      return true;
    }
  if (opnd == 0)
    return OP_REG (ins, eAX_reg, sizeflag);
  return OP_IMREG (ins, eAX_reg, sizeflag);
}

static const char *const Suffix3DNow[] = {
/* 00 */	NULL,		NULL,		NULL,		NULL,
/* 04 */	NULL,		NULL,		NULL,		NULL,
/* 08 */	NULL,		NULL,		NULL,		NULL,
/* 0C */	"pi2fw",	"pi2fd",	NULL,		NULL,
/* 10 */	NULL,		NULL,		NULL,		NULL,
/* 14 */	NULL,		NULL,		NULL,		NULL,
/* 18 */	NULL,		NULL,		NULL,		NULL,
/* 1C */	"pf2iw",	"pf2id",	NULL,		NULL,
/* 20 */	NULL,		NULL,		NULL,		NULL,
/* 24 */	NULL,		NULL,		NULL,		NULL,
/* 28 */	NULL,		NULL,		NULL,		NULL,
/* 2C */	NULL,		NULL,		NULL,		NULL,
/* 30 */	NULL,		NULL,		NULL,		NULL,
/* 34 */	NULL,		NULL,		NULL,		NULL,
/* 38 */	NULL,		NULL,		NULL,		NULL,
/* 3C */	NULL,		NULL,		NULL,		NULL,
/* 40 */	NULL,		NULL,		NULL,		NULL,
/* 44 */	NULL,		NULL,		NULL,		NULL,
/* 48 */	NULL,		NULL,		NULL,		NULL,
/* 4C */	NULL,		NULL,		NULL,		NULL,
/* 50 */	NULL,		NULL,		NULL,		NULL,
/* 54 */	NULL,		NULL,		NULL,		NULL,
/* 58 */	NULL,		NULL,		NULL,		NULL,
/* 5C */	NULL,		NULL,		NULL,		NULL,
/* 60 */	NULL,		NULL,		NULL,		NULL,
/* 64 */	NULL,		NULL,		NULL,		NULL,
/* 68 */	NULL,		NULL,		NULL,		NULL,
/* 6C */	NULL,		NULL,		NULL,		NULL,
/* 70 */	NULL,		NULL,		NULL,		NULL,
/* 74 */	NULL,		NULL,		NULL,		NULL,
/* 78 */	NULL,		NULL,		NULL,		NULL,
/* 7C */	NULL,		NULL,		NULL,		NULL,
/* 80 */	NULL,		NULL,		NULL,		NULL,
/* 84 */	NULL,		NULL,		NULL,		NULL,
/* 88 */	NULL,		NULL,		"pfnacc",	NULL,
/* 8C */	NULL,		NULL,		"pfpnacc",	NULL,
/* 90 */	"pfcmpge",	NULL,		NULL,		NULL,
/* 94 */	"pfmin",	NULL,		"pfrcp",	"pfrsqrt",
/* 98 */	NULL,		NULL,		"pfsub",	NULL,
/* 9C */	NULL,		NULL,		"pfadd",	NULL,
/* A0 */	"pfcmpgt",	NULL,		NULL,		NULL,
/* A4 */	"pfmax",	NULL,		"pfrcpit1",	"pfrsqit1",
/* A8 */	NULL,		NULL,		"pfsubr",	NULL,
/* AC */	NULL,		NULL,		"pfacc",	NULL,
/* B0 */	"pfcmpeq",	NULL,		NULL,		NULL,
/* B4 */	"pfmul",	NULL,		"pfrcpit2",	"pmulhrw",
/* B8 */	NULL,		NULL,		NULL,		"pswapd",
/* BC */	NULL,		NULL,		NULL,		"pavgusb",
/* C0 */	NULL,		NULL,		NULL,		NULL,
/* C4 */	NULL,		NULL,		NULL,		NULL,
/* C8 */	NULL,		NULL,		NULL,		NULL,
/* CC */	NULL,		NULL,		NULL,		NULL,
/* D0 */	NULL,		NULL,		NULL,		NULL,
/* D4 */	NULL,		NULL,		NULL,		NULL,
/* D8 */	NULL,		NULL,		NULL,		NULL,
/* DC */	NULL,		NULL,		NULL,		NULL,
/* E0 */	NULL,		NULL,		NULL,		NULL,
/* E4 */	NULL,		NULL,		NULL,		NULL,
/* E8 */	NULL,		NULL,		NULL,		NULL,
/* EC */	NULL,		NULL,		NULL,		NULL,
/* F0 */	NULL,		NULL,		NULL,		NULL,
/* F4 */	NULL,		NULL,		NULL,		NULL,
/* F8 */	NULL,		NULL,		NULL,		NULL,
/* FC */	NULL,		NULL,		NULL,		NULL,
};

static bool
OP_3DNowSuffix (instr_info *ins, int bytemode ATTRIBUTE_UNUSED,
		int sizeflag ATTRIBUTE_UNUSED)
{
  const char *mnemonic;

  if (!fetch_code (ins->info, ins->codep + 1))
    return false;
  /* AMD 3DNow! instructions are specified by an opcode suffix in the
     place where an 8-bit immediate would normally go.  ie. the last
     byte of the instruction.  */
  ins->obufp = ins->mnemonicendp;
  mnemonic = Suffix3DNow[*ins->codep++];
  if (mnemonic)
    ins->obufp = stpcpy (ins->obufp, mnemonic);
  else
    {
      /* Since a variable sized ins->modrm/ins->sib chunk is between the start
	 of the opcode (0x0f0f) and the opcode suffix, we need to do
	 all the ins->modrm processing first, and don't know until now that
	 we have a bad opcode.  This necessitates some cleaning up.  */
      ins->op_out[0][0] = '\0';
      ins->op_out[1][0] = '\0';
      BadOp (ins);
    }
  ins->mnemonicendp = ins->obufp;
  return true;
}

static const struct op simd_cmp_op[] =
{
  { STRING_COMMA_LEN ("eq") },
  { STRING_COMMA_LEN ("lt") },
  { STRING_COMMA_LEN ("le") },
  { STRING_COMMA_LEN ("unord") },
  { STRING_COMMA_LEN ("neq") },
  { STRING_COMMA_LEN ("nlt") },
  { STRING_COMMA_LEN ("nle") },
  { STRING_COMMA_LEN ("ord") }
};

static const struct op vex_cmp_op[] =
{
  { STRING_COMMA_LEN ("eq_uq") },
  { STRING_COMMA_LEN ("nge") },
  { STRING_COMMA_LEN ("ngt") },
  { STRING_COMMA_LEN ("false") },
  { STRING_COMMA_LEN ("neq_oq") },
  { STRING_COMMA_LEN ("ge") },
  { STRING_COMMA_LEN ("gt") },
  { STRING_COMMA_LEN ("true") },
  { STRING_COMMA_LEN ("eq_os") },
  { STRING_COMMA_LEN ("lt_oq") },
  { STRING_COMMA_LEN ("le_oq") },
  { STRING_COMMA_LEN ("unord_s") },
  { STRING_COMMA_LEN ("neq_us") },
  { STRING_COMMA_LEN ("nlt_uq") },
  { STRING_COMMA_LEN ("nle_uq") },
  { STRING_COMMA_LEN ("ord_s") },
  { STRING_COMMA_LEN ("eq_us") },
  { STRING_COMMA_LEN ("nge_uq") },
  { STRING_COMMA_LEN ("ngt_uq") },
  { STRING_COMMA_LEN ("false_os") },
  { STRING_COMMA_LEN ("neq_os") },
  { STRING_COMMA_LEN ("ge_oq") },
  { STRING_COMMA_LEN ("gt_oq") },
  { STRING_COMMA_LEN ("true_us") },
};

static bool
CMP_Fixup (instr_info *ins, int bytemode ATTRIBUTE_UNUSED,
	   int sizeflag ATTRIBUTE_UNUSED)
{
  unsigned int cmp_type;

  if (!fetch_code (ins->info, ins->codep + 1))
    return false;
  cmp_type = *ins->codep++;
  if (cmp_type < ARRAY_SIZE (simd_cmp_op))
    {
      char suffix[3];
      char *p = ins->mnemonicendp - 2;
      suffix[0] = p[0];
      suffix[1] = p[1];
      suffix[2] = '\0';
      sprintf (p, "%s%s", simd_cmp_op[cmp_type].name, suffix);
      ins->mnemonicendp += simd_cmp_op[cmp_type].len;
    }
  else if (ins->need_vex
	   && cmp_type < ARRAY_SIZE (simd_cmp_op) + ARRAY_SIZE (vex_cmp_op))
    {
      char suffix[3];
      char *p = ins->mnemonicendp - 2;
      suffix[0] = p[0];
      suffix[1] = p[1];
      suffix[2] = '\0';
      cmp_type -= ARRAY_SIZE (simd_cmp_op);
      sprintf (p, "%s%s", vex_cmp_op[cmp_type].name, suffix);
      ins->mnemonicendp += vex_cmp_op[cmp_type].len;
    }
  else
    {
      /* We have a reserved extension byte.  Output it directly.  */
      oappend_immediate (ins, cmp_type);
    }
  return true;
}

static bool
OP_Mwait (instr_info *ins, int bytemode, int sizeflag ATTRIBUTE_UNUSED)
{
  /* mwait %eax,%ecx / mwaitx %eax,%ecx,%ebx  */
  if (!ins->intel_syntax)
    {
      strcpy (ins->op_out[0], att_names32[0] + ins->intel_syntax);
      strcpy (ins->op_out[1], att_names32[1] + ins->intel_syntax);
      if (bytemode == eBX_reg)
	strcpy (ins->op_out[2], att_names32[3] + ins->intel_syntax);
      ins->two_source_ops = true;
    }
  /* Skip mod/rm byte.  */
  MODRM_CHECK;
  ins->codep++;
  return true;
}

static bool
OP_Monitor (instr_info *ins, int bytemode ATTRIBUTE_UNUSED,
	    int sizeflag ATTRIBUTE_UNUSED)
{
  /* monitor %{e,r,}ax,%ecx,%edx"  */
  if (!ins->intel_syntax)
    {
      const char (*names)[8] = (ins->address_mode == mode_64bit
				? att_names64 : att_names32);

      if (ins->prefixes & PREFIX_ADDR)
	{
	  /* Remove "addr16/addr32".  */
	  ins->all_prefixes[ins->last_addr_prefix] = 0;
	  names = (ins->address_mode != mode_32bit
		   ? att_names32 : att_names16);
	  ins->used_prefixes |= PREFIX_ADDR;
	}
      else if (ins->address_mode == mode_16bit)
	names = att_names16;
      strcpy (ins->op_out[0], names[0] + ins->intel_syntax);
      strcpy (ins->op_out[1], att_names32[1] + ins->intel_syntax);
      strcpy (ins->op_out[2], att_names32[2] + ins->intel_syntax);
      ins->two_source_ops = true;
    }
  /* Skip mod/rm byte.  */
  MODRM_CHECK;
  ins->codep++;
  return true;
}

static bool
REP_Fixup (instr_info *ins, int bytemode, int sizeflag)
{
  /* The 0xf3 prefix should be displayed as "rep" for ins, outs, movs,
     lods and stos.  */
  if (ins->prefixes & PREFIX_REPZ)
    ins->all_prefixes[ins->last_repz_prefix] = REP_PREFIX;

  switch (bytemode)
    {
    case al_reg:
    case eAX_reg:
    case indir_dx_reg:
      return OP_IMREG (ins, bytemode, sizeflag);
    case eDI_reg:
      return OP_ESreg (ins, bytemode, sizeflag);
    case eSI_reg:
      return OP_DSreg (ins, bytemode, sizeflag);
    default:
      abort ();
      break;
    }
  return true;
}

static bool
SEP_Fixup (instr_info *ins, int bytemode ATTRIBUTE_UNUSED,
	   int sizeflag ATTRIBUTE_UNUSED)
{
  if (ins->isa64 != amd64)
    return true;

  ins->obufp = ins->obuf;
  BadOp (ins);
  ins->mnemonicendp = ins->obufp;
  ++ins->codep;
  return true;
}

/* For BND-prefixed instructions 0xF2 prefix should be displayed as
   "bnd".  */

static bool
BND_Fixup (instr_info *ins, int bytemode ATTRIBUTE_UNUSED,
	   int sizeflag ATTRIBUTE_UNUSED)
{
  if (ins->prefixes & PREFIX_REPNZ)
    ins->all_prefixes[ins->last_repnz_prefix] = BND_PREFIX;
  return true;
}

/* For NOTRACK-prefixed instructions, 0x3E prefix should be displayed as
   "notrack".  */

static bool
NOTRACK_Fixup (instr_info *ins, int bytemode ATTRIBUTE_UNUSED,
	       int sizeflag ATTRIBUTE_UNUSED)
{
  /* Since active_seg_prefix is not set in 64-bit mode, check whether
     we've seen a PREFIX_DS.  */
  if ((ins->prefixes & PREFIX_DS) != 0
      && (ins->address_mode != mode_64bit || ins->last_data_prefix < 0))
    {
      /* NOTRACK prefix is only valid on indirect branch instructions.
	 NB: DATA prefix is unsupported for Intel64.  */
      ins->active_seg_prefix = 0;
      ins->all_prefixes[ins->last_seg_prefix] = NOTRACK_PREFIX;
    }
  return true;
}

/* Similar to OP_E.  But the 0xf2/0xf3 ins->prefixes should be displayed as
   "xacquire"/"xrelease" for memory operand if there is a LOCK prefix.
 */

static bool
HLE_Fixup1 (instr_info *ins, int bytemode, int sizeflag)
{
  if (ins->modrm.mod != 3
      && (ins->prefixes & PREFIX_LOCK) != 0)
    {
      if (ins->prefixes & PREFIX_REPZ)
	ins->all_prefixes[ins->last_repz_prefix] = XRELEASE_PREFIX;
      if (ins->prefixes & PREFIX_REPNZ)
	ins->all_prefixes[ins->last_repnz_prefix] = XACQUIRE_PREFIX;
    }

  return OP_E (ins, bytemode, sizeflag);
}

/* Similar to OP_E.  But the 0xf2/0xf3 ins->prefixes should be displayed as
   "xacquire"/"xrelease" for memory operand.  No check for LOCK prefix.
 */

static bool
HLE_Fixup2 (instr_info *ins, int bytemode, int sizeflag)
{
  if (ins->modrm.mod != 3)
    {
      if (ins->prefixes & PREFIX_REPZ)
	ins->all_prefixes[ins->last_repz_prefix] = XRELEASE_PREFIX;
      if (ins->prefixes & PREFIX_REPNZ)
	ins->all_prefixes[ins->last_repnz_prefix] = XACQUIRE_PREFIX;
    }

  return OP_E (ins, bytemode, sizeflag);
}

/* Similar to OP_E.  But the 0xf3 prefixes should be displayed as
   "xrelease" for memory operand.  No check for LOCK prefix.   */

static bool
HLE_Fixup3 (instr_info *ins, int bytemode, int sizeflag)
{
  if (ins->modrm.mod != 3
      && ins->last_repz_prefix > ins->last_repnz_prefix
      && (ins->prefixes & PREFIX_REPZ) != 0)
    ins->all_prefixes[ins->last_repz_prefix] = XRELEASE_PREFIX;

  return OP_E (ins, bytemode, sizeflag);
}

static bool
CMPXCHG8B_Fixup (instr_info *ins, int bytemode, int sizeflag)
{
  USED_REX (REX_W);
  if (ins->rex & REX_W)
    {
      /* Change cmpxchg8b to cmpxchg16b.  */
      char *p = ins->mnemonicendp - 2;
      ins->mnemonicendp = stpcpy (p, "16b");
      bytemode = o_mode;
    }
  else if ((ins->prefixes & PREFIX_LOCK) != 0)
    {
      if (ins->prefixes & PREFIX_REPZ)
	ins->all_prefixes[ins->last_repz_prefix] = XRELEASE_PREFIX;
      if (ins->prefixes & PREFIX_REPNZ)
	ins->all_prefixes[ins->last_repnz_prefix] = XACQUIRE_PREFIX;
    }

  return OP_M (ins, bytemode, sizeflag);
}

static bool
XMM_Fixup (instr_info *ins, int reg, int sizeflag ATTRIBUTE_UNUSED)
{
  const char (*names)[8] = att_names_xmm;

  if (ins->need_vex)
    {
      switch (ins->vex.length)
	{
	case 128:
	  break;
	case 256:
	  names = att_names_ymm;
	  break;
	default:
	  abort ();
	}
    }
  oappend_register (ins, names[reg]);
  return true;
}

static bool
FXSAVE_Fixup (instr_info *ins, int bytemode, int sizeflag)
{
  /* Add proper suffix to "fxsave" and "fxrstor".  */
  USED_REX (REX_W);
  if (ins->rex & REX_W)
    {
      char *p = ins->mnemonicendp;
      *p++ = '6';
      *p++ = '4';
      *p = '\0';
      ins->mnemonicendp = p;
    }
  return OP_M (ins, bytemode, sizeflag);
}

/* Display the destination register operand for instructions with
   VEX. */

static bool
OP_VEX (instr_info *ins, int bytemode, int sizeflag ATTRIBUTE_UNUSED)
{
  int reg, modrm_reg, sib_index = -1;
  const char (*names)[8];

  if (!ins->need_vex)
    abort ();

  reg = ins->vex.register_specifier;
  ins->vex.register_specifier = 0;
  if (ins->address_mode != mode_64bit)
    {
      if (ins->vex.evex && !ins->vex.v)
	{
	  oappend (ins, "(bad)");
	  return true;
	}

      reg &= 7;
    }
  else if (ins->vex.evex && !ins->vex.v)
    reg += 16;

  switch (bytemode)
    {
    case scalar_mode:
      oappend_register (ins, att_names_xmm[reg]);
      return true;

    case vex_vsib_d_w_dq_mode:
    case vex_vsib_q_w_dq_mode:
      /* This must be the 3rd operand.  */
      if (ins->obufp != ins->op_out[2])
	abort ();
      if (ins->vex.length == 128
	  || (bytemode != vex_vsib_d_w_dq_mode
	      && !ins->vex.w))
	oappend_register (ins, att_names_xmm[reg]);
      else
	oappend_register (ins, att_names_ymm[reg]);

      /* All 3 XMM/YMM registers must be distinct.  */
      modrm_reg = ins->modrm.reg;
      if (ins->rex & REX_R)
	modrm_reg += 8;

      if (ins->has_sib && ins->modrm.rm == 4)
	{
	  sib_index = ins->sib.index;
	  if (ins->rex & REX_X)
	    sib_index += 8;
	}

      if (reg == modrm_reg || reg == sib_index)
	strcpy (ins->obufp, "/(bad)");
      if (modrm_reg == sib_index || modrm_reg == reg)
	strcat (ins->op_out[0], "/(bad)");
      if (sib_index == modrm_reg || sib_index == reg)
	strcat (ins->op_out[1], "/(bad)");

      return true;

    case tmm_mode:
      /* All 3 TMM registers must be distinct.  */
      if (reg >= 8)
	oappend (ins, "(bad)");
      else
	{
	  /* This must be the 3rd operand.  */
	  if (ins->obufp != ins->op_out[2])
	    abort ();
	  oappend_register (ins, att_names_tmm[reg]);
	  if (reg == ins->modrm.reg || reg == ins->modrm.rm)
	    strcpy (ins->obufp, "/(bad)");
	}

      if (ins->modrm.reg == ins->modrm.rm || ins->modrm.reg == reg
	  || ins->modrm.rm == reg)
	{
	  if (ins->modrm.reg <= 8
	      && (ins->modrm.reg == ins->modrm.rm || ins->modrm.reg == reg))
	    strcat (ins->op_out[0], "/(bad)");
	  if (ins->modrm.rm <= 8
	      && (ins->modrm.rm == ins->modrm.reg || ins->modrm.rm == reg))
	    strcat (ins->op_out[1], "/(bad)");
	}

      return true;
    }

  switch (ins->vex.length)
    {
    case 128:
      switch (bytemode)
	{
	case x_mode:
	  names = att_names_xmm;
	  ins->evex_used |= EVEX_len_used;
	  break;
	case dq_mode:
	  if (ins->rex & REX_W)
	    names = att_names64;
	  else
	    names = att_names32;
	  break;
	case mask_bd_mode:
	case mask_mode:
	  if (reg > 0x7)
	    {
	      oappend (ins, "(bad)");
	      return true;
	    }
	  names = att_names_mask;
	  break;
	default:
	  abort ();
	  return true;
	}
      break;
    case 256:
      switch (bytemode)
	{
	case x_mode:
	  names = att_names_ymm;
	  ins->evex_used |= EVEX_len_used;
	  break;
	case mask_bd_mode:
	case mask_mode:
	  if (reg <= 0x7)
	    {
	      names = att_names_mask;
	      break;
	    }
	  /* Fall through.  */
	default:
	  /* See PR binutils/20893 for a reproducer.  */
	  oappend (ins, "(bad)");
	  return true;
	}
      break;
    case 512:
      names = att_names_zmm;
      ins->evex_used |= EVEX_len_used;
      break;
    default:
      abort ();
      break;
    }
  oappend_register (ins, names[reg]);
  return true;
}

static bool
OP_VexR (instr_info *ins, int bytemode, int sizeflag)
{
  if (ins->modrm.mod == 3)
    return OP_VEX (ins, bytemode, sizeflag);
  return true;
}

static bool
OP_VexW (instr_info *ins, int bytemode, int sizeflag)
{
  OP_VEX (ins, bytemode, sizeflag);

  if (ins->vex.w)
    {
      /* Swap 2nd and 3rd operands.  */
      char *tmp = ins->op_out[2];

      ins->op_out[2] = ins->op_out[1];
      ins->op_out[1] = tmp;
    }
  return true;
}

static bool
OP_REG_VexI4 (instr_info *ins, int bytemode, int sizeflag ATTRIBUTE_UNUSED)
{
  int reg;
  const char (*names)[8] = att_names_xmm;

  if (!fetch_code (ins->info, ins->codep + 1))
    return false;
  reg = *ins->codep++;

  if (bytemode != x_mode && bytemode != scalar_mode)
    abort ();

  reg >>= 4;
  if (ins->address_mode != mode_64bit)
    reg &= 7;

  if (bytemode == x_mode && ins->vex.length == 256)
    names = att_names_ymm;

  oappend_register (ins, names[reg]);

  if (ins->vex.w)
    {
      /* Swap 3rd and 4th operands.  */
      char *tmp = ins->op_out[3];

      ins->op_out[3] = ins->op_out[2];
      ins->op_out[2] = tmp;
    }
  return true;
}

static bool
OP_VexI4 (instr_info *ins, int bytemode ATTRIBUTE_UNUSED,
	  int sizeflag ATTRIBUTE_UNUSED)
{
  oappend_immediate (ins, ins->codep[-1] & 0xf);
  return true;
}

static bool
VPCMP_Fixup (instr_info *ins, int bytemode ATTRIBUTE_UNUSED,
	     int sizeflag ATTRIBUTE_UNUSED)
{
  unsigned int cmp_type;

  if (!ins->vex.evex)
    abort ();

  if (!fetch_code (ins->info, ins->codep + 1))
    return false;
  cmp_type = *ins->codep++;
  /* There are aliases for immediates 0, 1, 2, 4, 5, 6.
     If it's the case, print suffix, otherwise - print the immediate.  */
  if (cmp_type < ARRAY_SIZE (simd_cmp_op)
      && cmp_type != 3
      && cmp_type != 7)
    {
      char suffix[3];
      char *p = ins->mnemonicendp - 2;

      /* vpcmp* can have both one- and two-lettered suffix.  */
      if (p[0] == 'p')
	{
	  p++;
	  suffix[0] = p[0];
	  suffix[1] = '\0';
	}
      else
	{
	  suffix[0] = p[0];
	  suffix[1] = p[1];
	  suffix[2] = '\0';
	}

      sprintf (p, "%s%s", simd_cmp_op[cmp_type].name, suffix);
      ins->mnemonicendp += simd_cmp_op[cmp_type].len;
    }
  else
    {
      /* We have a reserved extension byte.  Output it directly.  */
      oappend_immediate (ins, cmp_type);
    }
  return true;
}

static const struct op xop_cmp_op[] =
{
  { STRING_COMMA_LEN ("lt") },
  { STRING_COMMA_LEN ("le") },
  { STRING_COMMA_LEN ("gt") },
  { STRING_COMMA_LEN ("ge") },
  { STRING_COMMA_LEN ("eq") },
  { STRING_COMMA_LEN ("neq") },
  { STRING_COMMA_LEN ("false") },
  { STRING_COMMA_LEN ("true") }
};

static bool
VPCOM_Fixup (instr_info *ins, int bytemode ATTRIBUTE_UNUSED,
	     int sizeflag ATTRIBUTE_UNUSED)
{
  unsigned int cmp_type;

  if (!fetch_code (ins->info, ins->codep + 1))
    return false;
  cmp_type = *ins->codep++;
  if (cmp_type < ARRAY_SIZE (xop_cmp_op))
    {
      char suffix[3];
      char *p = ins->mnemonicendp - 2;

      /* vpcom* can have both one- and two-lettered suffix.  */
      if (p[0] == 'm')
	{
	  p++;
	  suffix[0] = p[0];
	  suffix[1] = '\0';
	}
      else
	{
	  suffix[0] = p[0];
	  suffix[1] = p[1];
	  suffix[2] = '\0';
	}

      sprintf (p, "%s%s", xop_cmp_op[cmp_type].name, suffix);
      ins->mnemonicendp += xop_cmp_op[cmp_type].len;
    }
  else
    {
      /* We have a reserved extension byte.  Output it directly.  */
      oappend_immediate (ins, cmp_type);
    }
  return true;
}

static const struct op pclmul_op[] =
{
  { STRING_COMMA_LEN ("lql") },
  { STRING_COMMA_LEN ("hql") },
  { STRING_COMMA_LEN ("lqh") },
  { STRING_COMMA_LEN ("hqh") }
};

static bool
PCLMUL_Fixup (instr_info *ins, int bytemode ATTRIBUTE_UNUSED,
	      int sizeflag ATTRIBUTE_UNUSED)
{
  unsigned int pclmul_type;

  if (!fetch_code (ins->info, ins->codep + 1))
    return false;
  pclmul_type = *ins->codep++;
  switch (pclmul_type)
    {
    case 0x10:
      pclmul_type = 2;
      break;
    case 0x11:
      pclmul_type = 3;
      break;
    default:
      break;
    }
  if (pclmul_type < ARRAY_SIZE (pclmul_op))
    {
      char suffix[4];
      char *p = ins->mnemonicendp - 3;
      suffix[0] = p[0];
      suffix[1] = p[1];
      suffix[2] = p[2];
      suffix[3] = '\0';
      sprintf (p, "%s%s", pclmul_op[pclmul_type].name, suffix);
      ins->mnemonicendp += pclmul_op[pclmul_type].len;
    }
  else
    {
      /* We have a reserved extension byte.  Output it directly.  */
      oappend_immediate (ins, pclmul_type);
    }
  return true;
}

static bool
MOVSXD_Fixup (instr_info *ins, int bytemode, int sizeflag)
{
  /* Add proper suffix to "movsxd".  */
  char *p = ins->mnemonicendp;

  switch (bytemode)
    {
    case movsxd_mode:
      if (!ins->intel_syntax)
	{
	  USED_REX (REX_W);
	  if (ins->rex & REX_W)
	    {
	      *p++ = 'l';
	      *p++ = 'q';
	      break;
	    }
	}

      *p++ = 'x';
      *p++ = 'd';
      break;
    default:
      oappend (ins, INTERNAL_DISASSEMBLER_ERROR);
      break;
    }

  ins->mnemonicendp = p;
  *p = '\0';
  return OP_E (ins, bytemode, sizeflag);
}

static bool
DistinctDest_Fixup (instr_info *ins, int bytemode, int sizeflag)
{
  unsigned int reg = ins->vex.register_specifier;
  unsigned int modrm_reg = ins->modrm.reg;
  unsigned int modrm_rm = ins->modrm.rm;

  /* Calc destination register number.  */
  if (ins->rex & REX_R)
    modrm_reg += 8;
  if (!ins->vex.r)
    modrm_reg += 16;

  /* Calc src1 register number.  */
  if (ins->address_mode != mode_64bit)
    reg &= 7;
  else if (ins->vex.evex && !ins->vex.v)
    reg += 16;

  /* Calc src2 register number.  */
  if (ins->modrm.mod == 3)
    {
      if (ins->rex & REX_B)
        modrm_rm += 8;
      if (ins->rex & REX_X)
        modrm_rm += 16;
    }

  /* Destination and source registers must be distinct, output bad if
     dest == src1 or dest == src2.  */
  if (modrm_reg == reg
      || (ins->modrm.mod == 3
	  && modrm_reg == modrm_rm))
    {
      oappend (ins, "(bad)");
      return true;
    }
  return OP_XMM (ins, bytemode, sizeflag);
}

static bool
OP_Rounding (instr_info *ins, int bytemode, int sizeflag ATTRIBUTE_UNUSED)
{
  if (ins->modrm.mod != 3 || !ins->vex.b)
    return true;

  switch (bytemode)
    {
    case evex_rounding_64_mode:
      if (ins->address_mode != mode_64bit || !ins->vex.w)
        return true;
      /* Fall through.  */
    case evex_rounding_mode:
      ins->evex_used |= EVEX_b_used;
      oappend (ins, names_rounding[ins->vex.ll]);
      break;
    case evex_sae_mode:
      ins->evex_used |= EVEX_b_used;
      oappend (ins, "{");
      break;
    default:
      abort ();
    }
  oappend (ins, "sae}");
  return true;
}

static bool
PREFETCHI_Fixup (instr_info *ins, int bytemode, int sizeflag)
{
  if (ins->modrm.mod != 0 || ins->modrm.rm != 5)
    {
      if (ins->intel_syntax)
	{
	  ins->mnemonicendp = stpcpy (ins->obuf, "nop   ");
	}
      else
	{
	  USED_REX (REX_W);
	  if (ins->rex & REX_W)
	    ins->mnemonicendp = stpcpy (ins->obuf, "nopq  ");
	  else
	    {
	      if (sizeflag & DFLAG)
		ins->mnemonicendp = stpcpy (ins->obuf, "nopl  ");
	      else
		ins->mnemonicendp = stpcpy (ins->obuf, "nopw  ");
	      ins->used_prefixes |= (ins->prefixes & PREFIX_DATA);
	    }
	}
      bytemode = v_mode;
    }

  return OP_M (ins, bytemode, sizeflag);
}
