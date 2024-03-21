/* tc-aarch64.c -- Assemble for the AArch64 ISA

   Copyright (C) 2009-2023 Free Software Foundation, Inc.
   Contributed by ARM Ltd.

   This file is part of GAS.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the license, or
   (at your option) any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING3. If not,
   see <http://www.gnu.org/licenses/>.  */

#include "as.h"
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#define	 NO_RELOC 0
#include "safe-ctype.h"
#include "subsegs.h"
#include "obstack.h"

#ifdef OBJ_ELF
#include "elf/aarch64.h"
#include "dw2gencfi.h"
#include "sframe.h"
#include "gen-sframe.h"
#endif

#include "dw2gencfi.h"
#include "dwarf2dbg.h"

/* Types of processor to assemble for.  */
#ifndef CPU_DEFAULT
#define CPU_DEFAULT AARCH64_ARCH_V8
#endif

#define streq(a, b)	      (strcmp (a, b) == 0)

#define END_OF_INSN '\0'

static aarch64_feature_set cpu_variant;

/* Variables that we set while parsing command-line options.  Once all
   options have been read we re-process these values to set the real
   assembly flags.  */
static const aarch64_feature_set *mcpu_cpu_opt = NULL;
static const aarch64_feature_set *march_cpu_opt = NULL;

/* Constants for known architecture features.  */
static const aarch64_feature_set cpu_default = CPU_DEFAULT;

/* Currently active instruction sequence.  */
static aarch64_instr_sequence *insn_sequence = NULL;

#ifdef OBJ_ELF
/* Pre-defined "_GLOBAL_OFFSET_TABLE_"	*/
static symbolS *GOT_symbol;
#endif

/* Which ABI to use.  */
enum aarch64_abi_type
{
  AARCH64_ABI_NONE = 0,
  AARCH64_ABI_LP64 = 1,
  AARCH64_ABI_ILP32 = 2,
  AARCH64_ABI_LLP64 = 3
};

unsigned int aarch64_sframe_cfa_sp_reg;
/* The other CFA base register for SFrame stack trace info.  */
unsigned int aarch64_sframe_cfa_fp_reg;
unsigned int aarch64_sframe_cfa_ra_reg;

#ifndef DEFAULT_ARCH
#define DEFAULT_ARCH "aarch64"
#endif

#ifdef OBJ_ELF
/* DEFAULT_ARCH is initialized in gas/configure.tgt.  */
static const char *default_arch = DEFAULT_ARCH;
#endif

/* AArch64 ABI for the output file.  */
static enum aarch64_abi_type aarch64_abi = AARCH64_ABI_NONE;

/* When non-zero, program to a 32-bit model, in which the C data types
   int, long and all pointer types are 32-bit objects (ILP32); or to a
   64-bit model, in which the C int type is 32-bits but the C long type
   and all pointer types are 64-bit objects (LP64).  */
#define ilp32_p		(aarch64_abi == AARCH64_ABI_ILP32)

/* When non zero, C types int and long are 32 bit,
   pointers, however are 64 bit */
#define llp64_p (aarch64_abi == AARCH64_ABI_LLP64)

enum vector_el_type
{
  NT_invtype = -1,
  NT_b,
  NT_h,
  NT_s,
  NT_d,
  NT_q,
  NT_zero,
  NT_merge
};

/* Bits for DEFINED field in vector_type_el.  */
#define NTA_HASTYPE     1
#define NTA_HASINDEX    2
#define NTA_HASVARWIDTH 4

struct vector_type_el
{
  enum vector_el_type type;
  unsigned char defined;
  unsigned element_size;
  unsigned width;
  int64_t index;
};

#define FIXUP_F_HAS_EXPLICIT_SHIFT	0x00000001

struct reloc
{
  bfd_reloc_code_real_type type;
  expressionS exp;
  int pc_rel;
  enum aarch64_opnd opnd;
  uint32_t flags;
  unsigned need_libopcodes_p : 1;
};

struct aarch64_instruction
{
  /* libopcodes structure for instruction intermediate representation.  */
  aarch64_inst base;
  /* Record assembly errors found during the parsing.  */
  aarch64_operand_error parsing_error;
  /* The condition that appears in the assembly line.  */
  int cond;
  /* Relocation information (including the GAS internal fixup).  */
  struct reloc reloc;
  /* Need to generate an immediate in the literal pool.  */
  unsigned gen_lit_pool : 1;
};

typedef struct aarch64_instruction aarch64_instruction;

static aarch64_instruction inst;

static bool parse_operands (char *, const aarch64_opcode *);
static bool programmer_friendly_fixup (aarch64_instruction *);

/* If an AARCH64_OPDE_SYNTAX_ERROR has no error string, its first three
   data fields contain the following information:

   data[0].i:
     A mask of register types that would have been acceptable as bare
     operands, outside of a register list.  In addition, SEF_DEFAULT_ERROR
     is set if a general parsing error occured for an operand (that is,
     an error not related to registers, and having no error string).

   data[1].i:
     A mask of register types that would have been acceptable inside
     a register list.  In addition, SEF_IN_REGLIST is set if the
     operand contained a '{' and if we got to the point of trying
     to parse a register inside a list.

   data[2].i:
     The mask associated with the register that was actually seen, or 0
     if none.  A nonzero value describes a register inside a register
     list if data[1].i & SEF_IN_REGLIST, otherwise it describes a bare
     register.

   The idea is that stringless errors from multiple opcode templates can
   be ORed together to give a summary of the available alternatives.  */
#define SEF_DEFAULT_ERROR (1U << 31)
#define SEF_IN_REGLIST (1U << 31)

/* Diagnostics inline function utilities.

   These are lightweight utilities which should only be called by parse_operands
   and other parsers.  GAS processes each assembly line by parsing it against
   instruction template(s), in the case of multiple templates (for the same
   mnemonic name), those templates are tried one by one until one succeeds or
   all fail.  An assembly line may fail a few templates before being
   successfully parsed; an error saved here in most cases is not a user error
   but an error indicating the current template is not the right template.
   Therefore it is very important that errors can be saved at a low cost during
   the parsing; we don't want to slow down the whole parsing by recording
   non-user errors in detail.

   Remember that the objective is to help GAS pick up the most appropriate
   error message in the case of multiple templates, e.g. FMOV which has 8
   templates.  */

static inline void
clear_error (void)
{
  memset (&inst.parsing_error, 0, sizeof (inst.parsing_error));
  inst.parsing_error.kind = AARCH64_OPDE_NIL;
}

static inline bool
error_p (void)
{
  return inst.parsing_error.kind != AARCH64_OPDE_NIL;
}

static inline void
set_error (enum aarch64_operand_error_kind kind, const char *error)
{
  memset (&inst.parsing_error, 0, sizeof (inst.parsing_error));
  inst.parsing_error.index = -1;
  inst.parsing_error.kind = kind;
  inst.parsing_error.error = error;
}

static inline void
set_recoverable_error (const char *error)
{
  set_error (AARCH64_OPDE_RECOVERABLE, error);
}

/* Use the DESC field of the corresponding aarch64_operand entry to compose
   the error message.  */
static inline void
set_default_error (void)
{
  set_error (AARCH64_OPDE_SYNTAX_ERROR, NULL);
  inst.parsing_error.data[0].i = SEF_DEFAULT_ERROR;
}

static inline void
set_expected_error (unsigned int flags)
{
  set_error (AARCH64_OPDE_SYNTAX_ERROR, NULL);
  inst.parsing_error.data[0].i = flags;
}

static inline void
set_syntax_error (const char *error)
{
  set_error (AARCH64_OPDE_SYNTAX_ERROR, error);
}

static inline void
set_first_syntax_error (const char *error)
{
  if (! error_p ())
    set_error (AARCH64_OPDE_SYNTAX_ERROR, error);
}

static inline void
set_fatal_syntax_error (const char *error)
{
  set_error (AARCH64_OPDE_FATAL_SYNTAX_ERROR, error);
}

/* Return value for certain parsers when the parsing fails; those parsers
   return the information of the parsed result, e.g. register number, on
   success.  */
#define PARSE_FAIL -1

/* This is an invalid condition code that means no conditional field is
   present. */
#define COND_ALWAYS 0x10

typedef struct
{
  const char *template;
  uint32_t value;
} asm_nzcv;

struct reloc_entry
{
  char *name;
  bfd_reloc_code_real_type reloc;
};

/* Macros to define the register types and masks for the purpose
   of parsing.  */

#undef AARCH64_REG_TYPES
#define AARCH64_REG_TYPES	\
  BASIC_REG_TYPE(R_32)	/* w[0-30] */	\
  BASIC_REG_TYPE(R_64)	/* x[0-30] */	\
  BASIC_REG_TYPE(SP_32)	/* wsp     */	\
  BASIC_REG_TYPE(SP_64)	/* sp      */	\
  BASIC_REG_TYPE(ZR_32)	/* wzr     */	\
  BASIC_REG_TYPE(ZR_64)	/* xzr     */	\
  BASIC_REG_TYPE(FP_B)	/* b[0-31] *//* NOTE: keep FP_[BHSDQ] consecutive! */\
  BASIC_REG_TYPE(FP_H)	/* h[0-31] */	\
  BASIC_REG_TYPE(FP_S)	/* s[0-31] */	\
  BASIC_REG_TYPE(FP_D)	/* d[0-31] */	\
  BASIC_REG_TYPE(FP_Q)	/* q[0-31] */	\
  BASIC_REG_TYPE(V)	/* v[0-31] */	\
  BASIC_REG_TYPE(Z)	/* z[0-31] */	\
  BASIC_REG_TYPE(P)	/* p[0-15] */	\
  BASIC_REG_TYPE(PN)	/* pn[0-15] */	\
  BASIC_REG_TYPE(ZA)	/* za */	\
  BASIC_REG_TYPE(ZAT)	/* za[0-15] (ZA tile) */			\
  BASIC_REG_TYPE(ZATH)	/* za[0-15]h (ZA tile horizontal slice) */ 	\
  BASIC_REG_TYPE(ZATV)	/* za[0-15]v (ZA tile vertical slice) */	\
  BASIC_REG_TYPE(ZT0)	/* zt0 */					\
  /* Typecheck: any 64-bit int reg         (inc SP exc XZR).  */	\
  MULTI_REG_TYPE(R64_SP, REG_TYPE(R_64) | REG_TYPE(SP_64))		\
  /* Typecheck: same, plus SVE registers.  */				\
  MULTI_REG_TYPE(SVE_BASE, REG_TYPE(R_64) | REG_TYPE(SP_64)		\
		 | REG_TYPE(Z))						\
  /* Typecheck: x[0-30], w[0-30] or [xw]zr.  */				\
  MULTI_REG_TYPE(R_ZR, REG_TYPE(R_32) | REG_TYPE(R_64)			\
		 | REG_TYPE(ZR_32) | REG_TYPE(ZR_64))			\
  /* Typecheck: same, plus SVE registers.  */				\
  MULTI_REG_TYPE(SVE_OFFSET, REG_TYPE(R_32) | REG_TYPE(R_64)		\
		 | REG_TYPE(ZR_32) | REG_TYPE(ZR_64)			\
		 | REG_TYPE(Z))						\
  /* Typecheck: x[0-30], w[0-30] or {w}sp.  */				\
  MULTI_REG_TYPE(R_SP, REG_TYPE(R_32) | REG_TYPE(R_64)			\
		 | REG_TYPE(SP_32) | REG_TYPE(SP_64))			\
  /* Typecheck: any int                    (inc {W}SP inc [WX]ZR).  */	\
  MULTI_REG_TYPE(R_ZR_SP, REG_TYPE(R_32) | REG_TYPE(R_64)		\
		 | REG_TYPE(SP_32) | REG_TYPE(SP_64)			\
		 | REG_TYPE(ZR_32) | REG_TYPE(ZR_64)) 			\
  /* Typecheck: any [BHSDQ]P FP.  */					\
  MULTI_REG_TYPE(BHSDQ, REG_TYPE(FP_B) | REG_TYPE(FP_H)			\
		 | REG_TYPE(FP_S) | REG_TYPE(FP_D) | REG_TYPE(FP_Q))	\
  /* Typecheck: any int or [BHSDQ]P FP or V reg (exc SP inc [WX]ZR).  */ \
  MULTI_REG_TYPE(R_ZR_BHSDQ_V, REG_TYPE(R_32) | REG_TYPE(R_64)		\
		 | REG_TYPE(ZR_32) | REG_TYPE(ZR_64) | REG_TYPE(V)	\
		 | REG_TYPE(FP_B) | REG_TYPE(FP_H)			\
		 | REG_TYPE(FP_S) | REG_TYPE(FP_D) | REG_TYPE(FP_Q))	\
  /* Typecheck: as above, but also Zn, Pn, and {W}SP.  This should only	\
     be used for SVE instructions, since Zn and Pn are valid symbols	\
     in other contexts.  */						\
  MULTI_REG_TYPE(R_ZR_SP_BHSDQ_VZP, REG_TYPE(R_32) | REG_TYPE(R_64)	\
		 | REG_TYPE(SP_32) | REG_TYPE(SP_64)			\
		 | REG_TYPE(ZR_32) | REG_TYPE(ZR_64) | REG_TYPE(V)	\
		 | REG_TYPE(FP_B) | REG_TYPE(FP_H)			\
		 | REG_TYPE(FP_S) | REG_TYPE(FP_D) | REG_TYPE(FP_Q)	\
		 | REG_TYPE(Z) | REG_TYPE(P))				\
  /* Likewise, but with predicate-as-counter registers added.  */	\
  MULTI_REG_TYPE(R_ZR_SP_BHSDQ_VZP_PN, REG_TYPE(R_32) | REG_TYPE(R_64)	\
		 | REG_TYPE(SP_32) | REG_TYPE(SP_64)			\
		 | REG_TYPE(ZR_32) | REG_TYPE(ZR_64) | REG_TYPE(V)	\
		 | REG_TYPE(FP_B) | REG_TYPE(FP_H)			\
		 | REG_TYPE(FP_S) | REG_TYPE(FP_D) | REG_TYPE(FP_Q)	\
		 | REG_TYPE(Z) | REG_TYPE(P) | REG_TYPE(PN))		\
  /* Any integer register; used for error messages only.  */		\
  MULTI_REG_TYPE(R_N, REG_TYPE(R_32) | REG_TYPE(R_64)			\
		 | REG_TYPE(SP_32) | REG_TYPE(SP_64)			\
		 | REG_TYPE(ZR_32) | REG_TYPE(ZR_64))			\
  /* Any vector register.  */						\
  MULTI_REG_TYPE(VZ, REG_TYPE(V) | REG_TYPE(Z))				\
  /* An SVE vector or predicate register.  */				\
  MULTI_REG_TYPE(ZP, REG_TYPE(Z) | REG_TYPE(P))				\
  /* Any vector or predicate register.  */				\
  MULTI_REG_TYPE(VZP, REG_TYPE(V) | REG_TYPE(Z) | REG_TYPE(P))		\
  /* The whole of ZA or a single tile.  */				\
  MULTI_REG_TYPE(ZA_ZAT, REG_TYPE(ZA) | REG_TYPE(ZAT))			\
  /* A horizontal or vertical slice of a ZA tile.  */			\
  MULTI_REG_TYPE(ZATHV, REG_TYPE(ZATH) | REG_TYPE(ZATV))		\
  /* Pseudo type to mark the end of the enumerator sequence.  */	\
  END_REG_TYPE(MAX)

#undef BASIC_REG_TYPE
#define BASIC_REG_TYPE(T)	REG_TYPE_##T,
#undef MULTI_REG_TYPE
#define MULTI_REG_TYPE(T,V)	BASIC_REG_TYPE(T)
#undef END_REG_TYPE
#define END_REG_TYPE(T)		BASIC_REG_TYPE(T)

/* Register type enumerators.  */
typedef enum aarch64_reg_type_
{
  /* A list of REG_TYPE_*.  */
  AARCH64_REG_TYPES
} aarch64_reg_type;

#undef BASIC_REG_TYPE
#define BASIC_REG_TYPE(T)	1 << REG_TYPE_##T,
#undef REG_TYPE
#define REG_TYPE(T)		(1 << REG_TYPE_##T)
#undef MULTI_REG_TYPE
#define MULTI_REG_TYPE(T,V)	V,
#undef END_REG_TYPE
#define END_REG_TYPE(T)		0

/* Structure for a hash table entry for a register.  */
typedef struct
{
  const char *name;
  unsigned char number;
  ENUM_BITFIELD (aarch64_reg_type_) type : 8;
  unsigned char builtin;
} reg_entry;

/* Values indexed by aarch64_reg_type to assist the type checking.  */
static const unsigned reg_type_masks[] =
{
  AARCH64_REG_TYPES
};

#undef BASIC_REG_TYPE
#undef REG_TYPE
#undef MULTI_REG_TYPE
#undef END_REG_TYPE
#undef AARCH64_REG_TYPES

/* We expected one of the registers in MASK to be specified.  If a register
   of some kind was specified, SEEN is a mask that contains that register,
   otherwise it is zero.

   If it is possible to provide a relatively pithy message that describes
   the error exactly, return a string that does so, reporting the error
   against "operand %d".  Return null otherwise.

   From a QoI perspective, any REG_TYPE_* that is passed as the first
   argument to set_expected_reg_error should generally have its own message.
   Providing messages for combinations of such REG_TYPE_*s can be useful if
   it is possible to summarize the combination in a relatively natural way.
   On the other hand, it seems better to avoid long lists of unrelated
   things.  */

static const char *
get_reg_expected_msg (unsigned int mask, unsigned int seen)
{
  /* First handle messages that use SEEN.  */
  if ((mask & reg_type_masks[REG_TYPE_ZAT])
      && (seen & reg_type_masks[REG_TYPE_ZATHV]))
    return N_("expected an unsuffixed ZA tile at operand %d");

  if ((mask & reg_type_masks[REG_TYPE_ZATHV])
      && (seen & reg_type_masks[REG_TYPE_ZAT]))
    return N_("missing horizontal or vertical suffix at operand %d");

  if ((mask & reg_type_masks[REG_TYPE_ZA])
      && (seen & (reg_type_masks[REG_TYPE_ZAT]
		  | reg_type_masks[REG_TYPE_ZATHV])))
    return N_("expected 'za' rather than a ZA tile at operand %d");

  if ((mask & reg_type_masks[REG_TYPE_PN])
      && (seen & reg_type_masks[REG_TYPE_P]))
    return N_("expected a predicate-as-counter rather than predicate-as-mask"
	      " register at operand %d");

  if ((mask & reg_type_masks[REG_TYPE_P])
      && (seen & reg_type_masks[REG_TYPE_PN]))
    return N_("expected a predicate-as-mask rather than predicate-as-counter"
	      " register at operand %d");

  /* Integer, zero and stack registers.  */
  if (mask == reg_type_masks[REG_TYPE_R_64])
    return N_("expected a 64-bit integer register at operand %d");
  if (mask == reg_type_masks[REG_TYPE_R_ZR])
    return N_("expected an integer or zero register at operand %d");
  if (mask == reg_type_masks[REG_TYPE_R_SP])
    return N_("expected an integer or stack pointer register at operand %d");

  /* Floating-point and SIMD registers.  */
  if (mask == reg_type_masks[REG_TYPE_BHSDQ])
    return N_("expected a scalar SIMD or floating-point register"
	      " at operand %d");
  if (mask == reg_type_masks[REG_TYPE_V])
    return N_("expected an Advanced SIMD vector register at operand %d");
  if (mask == reg_type_masks[REG_TYPE_Z])
    return N_("expected an SVE vector register at operand %d");
  if (mask == reg_type_masks[REG_TYPE_P]
      || mask == (reg_type_masks[REG_TYPE_P] | reg_type_masks[REG_TYPE_PN]))
    /* Use this error for "predicate-as-mask only" and "either kind of
       predicate".  We report a more specific error if P is used where
       PN is expected, and vice versa, so the issue at this point is
       "predicate-like" vs. "not predicate-like".  */
    return N_("expected an SVE predicate register at operand %d");
  if (mask == reg_type_masks[REG_TYPE_PN])
    return N_("expected an SVE predicate-as-counter register at operand %d");
  if (mask == reg_type_masks[REG_TYPE_VZ])
    return N_("expected a vector register at operand %d");
  if (mask == reg_type_masks[REG_TYPE_ZP])
    return N_("expected an SVE vector or predicate register at operand %d");
  if (mask == reg_type_masks[REG_TYPE_VZP])
    return N_("expected a vector or predicate register at operand %d");

  /* SME-related registers.  */
  if (mask == reg_type_masks[REG_TYPE_ZA])
    return N_("expected a ZA array vector at operand %d");
  if (mask == (reg_type_masks[REG_TYPE_ZA_ZAT] | reg_type_masks[REG_TYPE_ZT0]))
    return N_("expected ZT0 or a ZA mask at operand %d");
  if (mask == reg_type_masks[REG_TYPE_ZAT])
    return N_("expected a ZA tile at operand %d");
  if (mask == reg_type_masks[REG_TYPE_ZATHV])
    return N_("expected a ZA tile slice at operand %d");

  /* Integer and vector combos.  */
  if (mask == (reg_type_masks[REG_TYPE_R_ZR] | reg_type_masks[REG_TYPE_V]))
    return N_("expected an integer register or Advanced SIMD vector register"
	      " at operand %d");
  if (mask == (reg_type_masks[REG_TYPE_R_ZR] | reg_type_masks[REG_TYPE_Z]))
    return N_("expected an integer register or SVE vector register"
	      " at operand %d");
  if (mask == (reg_type_masks[REG_TYPE_R_ZR] | reg_type_masks[REG_TYPE_VZ]))
    return N_("expected an integer or vector register at operand %d");
  if (mask == (reg_type_masks[REG_TYPE_R_ZR] | reg_type_masks[REG_TYPE_P]))
    return N_("expected an integer or predicate register at operand %d");
  if (mask == (reg_type_masks[REG_TYPE_R_ZR] | reg_type_masks[REG_TYPE_VZP]))
    return N_("expected an integer, vector or predicate register"
	      " at operand %d");

  /* SVE and SME combos.  */
  if (mask == (reg_type_masks[REG_TYPE_Z] | reg_type_masks[REG_TYPE_ZATHV]))
    return N_("expected an SVE vector register or ZA tile slice"
	      " at operand %d");

  return NULL;
}

/* Record that we expected a register of type TYPE but didn't see one.
   REG is the register that we actually saw, or null if we didn't see a
   recognized register.  FLAGS is SEF_IN_REGLIST if we are parsing the
   contents of a register list, otherwise it is zero.  */

static inline void
set_expected_reg_error (aarch64_reg_type type, const reg_entry *reg,
			unsigned int flags)
{
  assert (flags == 0 || flags == SEF_IN_REGLIST);
  set_error (AARCH64_OPDE_SYNTAX_ERROR, NULL);
  if (flags & SEF_IN_REGLIST)
    inst.parsing_error.data[1].i = reg_type_masks[type] | flags;
  else
    inst.parsing_error.data[0].i = reg_type_masks[type];
  if (reg)
    inst.parsing_error.data[2].i = reg_type_masks[reg->type];
}

/* Record that we expected a register list containing registers of type TYPE,
   but didn't see the opening '{'.  If we saw a register instead, REG is the
   register that we saw, otherwise it is null.  */

static inline void
set_expected_reglist_error (aarch64_reg_type type, const reg_entry *reg)
{
  set_error (AARCH64_OPDE_SYNTAX_ERROR, NULL);
  inst.parsing_error.data[1].i = reg_type_masks[type];
  if (reg)
    inst.parsing_error.data[2].i = reg_type_masks[reg->type];
}

/* Some well known registers that we refer to directly elsewhere.  */
#define REG_SP	31
#define REG_ZR	31

/* Instructions take 4 bytes in the object file.  */
#define INSN_SIZE	4

static htab_t aarch64_ops_hsh;
static htab_t aarch64_cond_hsh;
static htab_t aarch64_shift_hsh;
static htab_t aarch64_sys_regs_hsh;
static htab_t aarch64_pstatefield_hsh;
static htab_t aarch64_sys_regs_ic_hsh;
static htab_t aarch64_sys_regs_dc_hsh;
static htab_t aarch64_sys_regs_at_hsh;
static htab_t aarch64_sys_regs_tlbi_hsh;
static htab_t aarch64_sys_regs_sr_hsh;
static htab_t aarch64_reg_hsh;
static htab_t aarch64_barrier_opt_hsh;
static htab_t aarch64_nzcv_hsh;
static htab_t aarch64_pldop_hsh;
static htab_t aarch64_hint_opt_hsh;

/* Stuff needed to resolve the label ambiguity
   As:
     ...
     label:   <insn>
   may differ from:
     ...
     label:
	      <insn>  */

static symbolS *last_label_seen;

/* Literal pool structure.  Held on a per-section
   and per-sub-section basis.  */

#define MAX_LITERAL_POOL_SIZE 1024
typedef struct literal_expression
{
  expressionS exp;
  /* If exp.op == O_big then this bignum holds a copy of the global bignum value.  */
  LITTLENUM_TYPE * bignum;
} literal_expression;

typedef struct literal_pool
{
  literal_expression literals[MAX_LITERAL_POOL_SIZE];
  unsigned int next_free_entry;
  unsigned int id;
  symbolS *symbol;
  segT section;
  subsegT sub_section;
  int size;
  struct literal_pool *next;
} literal_pool;

/* Pointer to a linked list of literal pools.  */
static literal_pool *list_of_pools = NULL;

/* Pure syntax.	 */

/* This array holds the chars that always start a comment.  If the
   pre-processor is disabled, these aren't very useful.	 */
const char comment_chars[] = "";

/* This array holds the chars that only start a comment at the beginning of
   a line.  If the line seems to have the form '# 123 filename'
   .line and .file directives will appear in the pre-processed output.	*/
/* Note that input_file.c hand checks for '#' at the beginning of the
   first line of the input file.  This is because the compiler outputs
   #NO_APP at the beginning of its output.  */
/* Also note that comments like this one will always work.  */
const char line_comment_chars[] = "#";

const char line_separator_chars[] = ";";

/* Chars that can be used to separate mant
   from exp in floating point numbers.	*/
const char EXP_CHARS[] = "eE";

/* Chars that mean this number is a floating point constant.  */
/* As in 0f12.456  */
/* or	 0d1.2345e12  */

const char FLT_CHARS[] = "rRsSfFdDxXeEpPhHb";

/* Prefix character that indicates the start of an immediate value.  */
#define is_immediate_prefix(C) ((C) == '#')

/* Separator character handling.  */

#define skip_whitespace(str)  do { if (*(str) == ' ') ++(str); } while (0)

static inline bool
skip_past_char (char **str, char c)
{
  if (**str == c)
    {
      (*str)++;
      return true;
    }
  else
    return false;
}

#define skip_past_comma(str) skip_past_char (str, ',')

/* Arithmetic expressions (possibly involving symbols).	 */

static bool in_aarch64_get_expression = false;

/* Third argument to aarch64_get_expression.  */
#define GE_NO_PREFIX  false
#define GE_OPT_PREFIX true

/* Fourth argument to aarch64_get_expression.  */
#define ALLOW_ABSENT  false
#define REJECT_ABSENT true

/* Return TRUE if the string pointed by *STR is successfully parsed
   as an valid expression; *EP will be filled with the information of
   such an expression.  Otherwise return FALSE.

   If ALLOW_IMMEDIATE_PREFIX is true then skip a '#' at the start.
   If REJECT_ABSENT is true then trat missing expressions as an error.  */

static bool
aarch64_get_expression (expressionS *  ep,
			char **        str,
			bool           allow_immediate_prefix,
			bool           reject_absent)
{
  char *save_in;
  segT seg;
  bool prefix_present = false;

  if (allow_immediate_prefix)
    {
      if (is_immediate_prefix (**str))
	{
	  (*str)++;
	  prefix_present = true;
	}
    }

  memset (ep, 0, sizeof (expressionS));

  save_in = input_line_pointer;
  input_line_pointer = *str;
  in_aarch64_get_expression = true;
  seg = expression (ep);
  in_aarch64_get_expression = false;

  if (ep->X_op == O_illegal || (reject_absent && ep->X_op == O_absent))
    {
      /* We found a bad expression in md_operand().  */
      *str = input_line_pointer;
      input_line_pointer = save_in;
      if (prefix_present && ! error_p ())
	set_fatal_syntax_error (_("bad expression"));
      else
	set_first_syntax_error (_("bad expression"));
      return false;
    }

#ifdef OBJ_AOUT
  if (seg != absolute_section
      && seg != text_section
      && seg != data_section
      && seg != bss_section
      && seg != undefined_section)
    {
      set_syntax_error (_("bad segment"));
      *str = input_line_pointer;
      input_line_pointer = save_in;
      return false;
    }
#else
  (void) seg;
#endif

  *str = input_line_pointer;
  input_line_pointer = save_in;
  return true;
}

/* Turn a string in input_line_pointer into a floating point constant
   of type TYPE, and store the appropriate bytes in *LITP.  The number
   of LITTLENUMS emitted is stored in *SIZEP.  An error message is
   returned, or NULL on OK.  */

const char *
md_atof (int type, char *litP, int *sizeP)
{
  return ieee_md_atof (type, litP, sizeP, target_big_endian);
}

/* We handle all bad expressions here, so that we can report the faulty
   instruction in the error message.  */
void
md_operand (expressionS * exp)
{
  if (in_aarch64_get_expression)
    exp->X_op = O_illegal;
}

/* Immediate values.  */

/* Errors may be set multiple times during parsing or bit encoding
   (particularly in the Neon bits), but usually the earliest error which is set
   will be the most meaningful. Avoid overwriting it with later (cascading)
   errors by calling this function.  */

static void
first_error (const char *error)
{
  if (! error_p ())
    set_syntax_error (error);
}

/* Similar to first_error, but this function accepts formatted error
   message.  */
static void
first_error_fmt (const char *format, ...)
{
  va_list args;
  enum
  { size = 100 };
  /* N.B. this single buffer will not cause error messages for different
     instructions to pollute each other; this is because at the end of
     processing of each assembly line, error message if any will be
     collected by as_bad.  */
  static char buffer[size];

  if (! error_p ())
    {
      int ret ATTRIBUTE_UNUSED;
      va_start (args, format);
      ret = vsnprintf (buffer, size, format, args);
      know (ret <= size - 1 && ret >= 0);
      va_end (args);
      set_syntax_error (buffer);
    }
}

/* Internal helper routine converting a vector_type_el structure *VECTYPE
   to a corresponding operand qualifier.  */

static inline aarch64_opnd_qualifier_t
vectype_to_qualifier (const struct vector_type_el *vectype)
{
  /* Element size in bytes indexed by vector_el_type.  */
  const unsigned char ele_size[5]
    = {1, 2, 4, 8, 16};
  const unsigned int ele_base [5] =
    {
      AARCH64_OPND_QLF_V_4B,
      AARCH64_OPND_QLF_V_2H,
      AARCH64_OPND_QLF_V_2S,
      AARCH64_OPND_QLF_V_1D,
      AARCH64_OPND_QLF_V_1Q
  };

  if (!vectype->defined || vectype->type == NT_invtype)
    goto vectype_conversion_fail;

  if (vectype->type == NT_zero)
    return AARCH64_OPND_QLF_P_Z;
  if (vectype->type == NT_merge)
    return AARCH64_OPND_QLF_P_M;

  gas_assert (vectype->type >= NT_b && vectype->type <= NT_q);

  if (vectype->defined & (NTA_HASINDEX | NTA_HASVARWIDTH))
    {
      /* Special case S_4B.  */
      if (vectype->type == NT_b && vectype->width == 4)
	return AARCH64_OPND_QLF_S_4B;

      /* Special case S_2H.  */
      if (vectype->type == NT_h && vectype->width == 2)
	return AARCH64_OPND_QLF_S_2H;

      /* Vector element register.  */
      return AARCH64_OPND_QLF_S_B + vectype->type;
    }
  else
    {
      /* Vector register.  */
      int reg_size = ele_size[vectype->type] * vectype->width;
      unsigned offset;
      unsigned shift;
      if (reg_size != 16 && reg_size != 8 && reg_size != 4)
	goto vectype_conversion_fail;

      /* The conversion is by calculating the offset from the base operand
	 qualifier for the vector type.  The operand qualifiers are regular
	 enough that the offset can established by shifting the vector width by
	 a vector-type dependent amount.  */
      shift = 0;
      if (vectype->type == NT_b)
	shift = 3;
      else if (vectype->type == NT_h || vectype->type == NT_s)
	shift = 2;
      else if (vectype->type >= NT_d)
	shift = 1;
      else
	gas_assert (0);

      offset = ele_base [vectype->type] + (vectype->width >> shift);
      gas_assert (AARCH64_OPND_QLF_V_4B <= offset
		  && offset <= AARCH64_OPND_QLF_V_1Q);
      return offset;
    }

 vectype_conversion_fail:
  first_error (_("bad vector arrangement type"));
  return AARCH64_OPND_QLF_NIL;
}

/* Register parsing.  */

/* Generic register parser which is called by other specialized
   register parsers.
   CCP points to what should be the beginning of a register name.
   If it is indeed a valid register name, advance CCP over it and
   return the reg_entry structure; otherwise return NULL.
   It does not issue diagnostics.  */

static reg_entry *
parse_reg (char **ccp)
{
  char *start = *ccp;
  char *p;
  reg_entry *reg;

#ifdef REGISTER_PREFIX
  if (*start != REGISTER_PREFIX)
    return NULL;
  start++;
#endif

  p = start;
  if (!ISALPHA (*p) || !is_name_beginner (*p))
    return NULL;

  do
    p++;
  while (ISALPHA (*p) || ISDIGIT (*p) || *p == '_');

  reg = (reg_entry *) str_hash_find_n (aarch64_reg_hsh, start, p - start);

  if (!reg)
    return NULL;

  *ccp = p;
  return reg;
}

/* Return the operand qualifier associated with all uses of REG, or
   AARCH64_OPND_QLF_NIL if none.  AARCH64_OPND_QLF_NIL means either
   that qualifiers don't apply to REG or that qualifiers are added
   using suffixes.  */

static aarch64_opnd_qualifier_t
inherent_reg_qualifier (const reg_entry *reg)
{
  switch (reg->type)
    {
    case REG_TYPE_R_32:
    case REG_TYPE_SP_32:
    case REG_TYPE_ZR_32:
      return AARCH64_OPND_QLF_W;

    case REG_TYPE_R_64:
    case REG_TYPE_SP_64:
    case REG_TYPE_ZR_64:
      return AARCH64_OPND_QLF_X;

    case REG_TYPE_FP_B:
    case REG_TYPE_FP_H:
    case REG_TYPE_FP_S:
    case REG_TYPE_FP_D:
    case REG_TYPE_FP_Q:
      return AARCH64_OPND_QLF_S_B + (reg->type - REG_TYPE_FP_B);

    default:
      return AARCH64_OPND_QLF_NIL;
    }
}

/* Return TRUE if REG->TYPE is a valid type of TYPE; otherwise
   return FALSE.  */
static bool
aarch64_check_reg_type (const reg_entry *reg, aarch64_reg_type type)
{
  return (reg_type_masks[type] & (1 << reg->type)) != 0;
}

/* Try to parse a base or offset register.  Allow SVE base and offset
   registers if REG_TYPE includes SVE registers.  Return the register
   entry on success, setting *QUALIFIER to the register qualifier.
   Return null otherwise.

   Note that this function does not issue any diagnostics.  */

static const reg_entry *
aarch64_addr_reg_parse (char **ccp, aarch64_reg_type reg_type,
			aarch64_opnd_qualifier_t *qualifier)
{
  char *str = *ccp;
  const reg_entry *reg = parse_reg (&str);

  if (reg == NULL)
    return NULL;

  switch (reg->type)
    {
    case REG_TYPE_Z:
      if ((reg_type_masks[reg_type] & (1 << REG_TYPE_Z)) == 0
	  || str[0] != '.')
	return NULL;
      switch (TOLOWER (str[1]))
	{
	case 's':
	  *qualifier = AARCH64_OPND_QLF_S_S;
	  break;
	case 'd':
	  *qualifier = AARCH64_OPND_QLF_S_D;
	  break;
	default:
	  return NULL;
	}
      str += 2;
      break;

    default:
      if (!aarch64_check_reg_type (reg, REG_TYPE_R_ZR_SP))
	return NULL;
      *qualifier = inherent_reg_qualifier (reg);
      break;
    }

  *ccp = str;

  return reg;
}

/* Try to parse a base or offset register.  Return the register entry
   on success, setting *QUALIFIER to the register qualifier.  Return null
   otherwise.

   Note that this function does not issue any diagnostics.  */

static const reg_entry *
aarch64_reg_parse_32_64 (char **ccp, aarch64_opnd_qualifier_t *qualifier)
{
  return aarch64_addr_reg_parse (ccp, REG_TYPE_R_ZR_SP, qualifier);
}

/* Parse the qualifier of a vector register or vector element of type
   REG_TYPE.  Fill in *PARSED_TYPE and return TRUE if the parsing
   succeeds; otherwise return FALSE.

   Accept only one occurrence of:
   4b 8b 16b 2h 4h 8h 2s 4s 1d 2d
   b h s d q  */
static bool
parse_vector_type_for_operand (aarch64_reg_type reg_type,
			       struct vector_type_el *parsed_type, char **str)
{
  char *ptr = *str;
  unsigned width;
  unsigned element_size;
  enum vector_el_type type;

  /* skip '.' */
  gas_assert (*ptr == '.');
  ptr++;

  if (reg_type != REG_TYPE_V || !ISDIGIT (*ptr))
    {
      width = 0;
      goto elt_size;
    }
  width = strtoul (ptr, &ptr, 10);
  if (width != 1 && width != 2 && width != 4 && width != 8 && width != 16)
    {
      first_error_fmt (_("bad size %d in vector width specifier"), width);
      return false;
    }

 elt_size:
  switch (TOLOWER (*ptr))
    {
    case 'b':
      type = NT_b;
      element_size = 8;
      break;
    case 'h':
      type = NT_h;
      element_size = 16;
      break;
    case 's':
      type = NT_s;
      element_size = 32;
      break;
    case 'd':
      type = NT_d;
      element_size = 64;
      break;
    case 'q':
      if (reg_type != REG_TYPE_V || width == 1)
	{
	  type = NT_q;
	  element_size = 128;
	  break;
	}
      /* fall through.  */
    default:
      if (*ptr != '\0')
	first_error_fmt (_("unexpected character `%c' in element size"), *ptr);
      else
	first_error (_("missing element size"));
      return false;
    }
  if (width != 0 && width * element_size != 64
      && width * element_size != 128
      && !(width == 2 && element_size == 16)
      && !(width == 4 && element_size == 8))
    {
      first_error_fmt (_
		       ("invalid element size %d and vector size combination %c"),
		       width, *ptr);
      return false;
    }
  ptr++;

  parsed_type->type = type;
  parsed_type->width = width;
  parsed_type->element_size = element_size;

  *str = ptr;

  return true;
}

/* *STR contains an SVE zero/merge predication suffix.  Parse it into
   *PARSED_TYPE and point *STR at the end of the suffix.  */

static bool
parse_predication_for_operand (struct vector_type_el *parsed_type, char **str)
{
  char *ptr = *str;

  /* Skip '/'.  */
  gas_assert (*ptr == '/');
  ptr++;
  switch (TOLOWER (*ptr))
    {
    case 'z':
      parsed_type->type = NT_zero;
      break;
    case 'm':
      parsed_type->type = NT_merge;
      break;
    default:
      if (*ptr != '\0' && *ptr != ',')
	first_error_fmt (_("unexpected character `%c' in predication type"),
			 *ptr);
      else
	first_error (_("missing predication type"));
      return false;
    }
  parsed_type->width = 0;
  *str = ptr + 1;
  return true;
}

/* Return true if CH is a valid suffix character for registers of
   type TYPE.  */

static bool
aarch64_valid_suffix_char_p (aarch64_reg_type type, char ch)
{
  switch (type)
    {
    case REG_TYPE_V:
    case REG_TYPE_Z:
    case REG_TYPE_ZA:
    case REG_TYPE_ZAT:
    case REG_TYPE_ZATH:
    case REG_TYPE_ZATV:
      return ch == '.';

    case REG_TYPE_P:
    case REG_TYPE_PN:
      return ch == '.' || ch == '/';

    default:
      return false;
    }
}

/* Parse an index expression at *STR, storing it in *IMM on success.  */

static bool
parse_index_expression (char **str, int64_t *imm)
{
  expressionS exp;

  aarch64_get_expression (&exp, str, GE_NO_PREFIX, REJECT_ABSENT);
  if (exp.X_op != O_constant)
    {
      first_error (_("constant expression required"));
      return false;
    }
  *imm = exp.X_add_number;
  return true;
}

/* Parse a register of the type TYPE.

   Return null if the string pointed to by *CCP is not a valid register
   name or the parsed register is not of TYPE.

   Otherwise return the register, and optionally return the register
   shape and element index information in *TYPEINFO.

   FLAGS includes PTR_IN_REGLIST if the caller is parsing a register list.

   FLAGS includes PTR_FULL_REG if the function should ignore any potential
   register index.

   FLAGS includes PTR_GOOD_MATCH if we are sufficiently far into parsing
   an operand that we can be confident that it is a good match.  */

#define PTR_IN_REGLIST (1U << 0)
#define PTR_FULL_REG (1U << 1)
#define PTR_GOOD_MATCH (1U << 2)

static const reg_entry *
parse_typed_reg (char **ccp, aarch64_reg_type type,
		 struct vector_type_el *typeinfo, unsigned int flags)
{
  char *str = *ccp;
  bool isalpha = ISALPHA (*str);
  const reg_entry *reg = parse_reg (&str);
  struct vector_type_el atype;
  struct vector_type_el parsetype;
  bool is_typed_vecreg = false;
  unsigned int err_flags = (flags & PTR_IN_REGLIST) ? SEF_IN_REGLIST : 0;

  atype.defined = 0;
  atype.type = NT_invtype;
  atype.width = -1;
  atype.element_size = 0;
  atype.index = 0;

  if (reg == NULL)
    {
      if (typeinfo)
	*typeinfo = atype;
      if (!isalpha && (flags & PTR_IN_REGLIST))
	set_fatal_syntax_error (_("syntax error in register list"));
      else if (flags & PTR_GOOD_MATCH)
	set_fatal_syntax_error (NULL);
      else
	set_expected_reg_error (type, reg, err_flags);
      return NULL;
    }

  if (! aarch64_check_reg_type (reg, type))
    {
      DEBUG_TRACE ("reg type check failed");
      if (flags & PTR_GOOD_MATCH)
	set_fatal_syntax_error (NULL);
      else
	set_expected_reg_error (type, reg, err_flags);
      return NULL;
    }
  type = reg->type;

  if (aarch64_valid_suffix_char_p (reg->type, *str))
    {
      if (*str == '.')
	{
	  if (!parse_vector_type_for_operand (type, &parsetype, &str))
	    return NULL;
	  if ((reg->type == REG_TYPE_ZAT
	       || reg->type == REG_TYPE_ZATH
	       || reg->type == REG_TYPE_ZATV)
	      && reg->number * 8 >= parsetype.element_size)
	    {
	      set_syntax_error (_("ZA tile number out of range"));
	      return NULL;
	    }
	}
      else
	{
	  if (!parse_predication_for_operand (&parsetype, &str))
	    return NULL;
	}

      /* Register if of the form Vn.[bhsdq].  */
      is_typed_vecreg = true;

      if (type != REG_TYPE_V)
	{
	  /* The width is always variable; we don't allow an integer width
	     to be specified.  */
	  gas_assert (parsetype.width == 0);
	  atype.defined |= NTA_HASVARWIDTH | NTA_HASTYPE;
	}
      else if (parsetype.width == 0)
	/* Expect index. In the new scheme we cannot have
	   Vn.[bhsdq] represent a scalar. Therefore any
	   Vn.[bhsdq] should have an index following it.
	   Except in reglists of course.  */
	atype.defined |= NTA_HASINDEX;
      else
	atype.defined |= NTA_HASTYPE;

      atype.type = parsetype.type;
      atype.width = parsetype.width;
    }

  if (!(flags & PTR_FULL_REG) && skip_past_char (&str, '['))
    {
      /* Reject Sn[index] syntax.  */
      if (reg->type != REG_TYPE_Z
	  && reg->type != REG_TYPE_PN
	  && reg->type != REG_TYPE_ZT0
	  && !is_typed_vecreg)
	{
	  first_error (_("this type of register can't be indexed"));
	  return NULL;
	}

      if (flags & PTR_IN_REGLIST)
	{
	  first_error (_("index not allowed inside register list"));
	  return NULL;
	}

      atype.defined |= NTA_HASINDEX;

      if (!parse_index_expression (&str, &atype.index))
	return NULL;

      if (! skip_past_char (&str, ']'))
	return NULL;
    }
  else if (!(flags & PTR_IN_REGLIST) && (atype.defined & NTA_HASINDEX) != 0)
    {
      /* Indexed vector register expected.  */
      first_error (_("indexed vector register expected"));
      return NULL;
    }

  /* A vector reg Vn should be typed or indexed.  */
  if (type == REG_TYPE_V && atype.defined == 0)
    {
      first_error (_("invalid use of vector register"));
    }

  if (typeinfo)
    *typeinfo = atype;

  *ccp = str;

  return reg;
}

/* Parse register.

   Return the register on success; return null otherwise.

   If this is a NEON vector register with additional type information, fill
   in the struct pointed to by VECTYPE (if non-NULL).

   This parser does not handle register lists.  */

static const reg_entry *
aarch64_reg_parse (char **ccp, aarch64_reg_type type,
		   struct vector_type_el *vectype)
{
  return parse_typed_reg (ccp, type, vectype, 0);
}

static inline bool
eq_vector_type_el (struct vector_type_el e1, struct vector_type_el e2)
{
  return (e1.type == e2.type
	  && e1.defined == e2.defined
	  && e1.width == e2.width
	  && e1.element_size == e2.element_size
	  && e1.index == e2.index);
}

/* Return the register number mask for registers of type REG_TYPE.  */

static inline int
reg_type_mask (aarch64_reg_type reg_type)
{
  return reg_type == REG_TYPE_P ? 15 : 31;
}

/* This function parses a list of vector registers of type TYPE.
   On success, it returns the parsed register list information in the
   following encoded format:

   bit   18-22   |   13-17   |   7-11    |    2-6    |   0-1
       4th regno | 3rd regno | 2nd regno | 1st regno | num_of_reg

   The information of the register shape and/or index is returned in
   *VECTYPE.

   It returns PARSE_FAIL if the register list is invalid.

   The list contains one to four registers.
   Each register can be one of:
   <Vt>.<T>[<index>]
   <Vt>.<T>
   All <T> should be identical.
   All <index> should be identical.
   There are restrictions on <Vt> numbers which are checked later
   (by reg_list_valid_p).  */

static int
parse_vector_reg_list (char **ccp, aarch64_reg_type type,
		       struct vector_type_el *vectype)
{
  char *str = *ccp;
  int nb_regs;
  struct vector_type_el typeinfo, typeinfo_first;
  uint32_t val, val_range, mask;
  int in_range;
  int ret_val;
  bool error = false;
  bool expect_index = false;
  unsigned int ptr_flags = PTR_IN_REGLIST;

  if (*str != '{')
    {
      set_expected_reglist_error (type, parse_reg (&str));
      return PARSE_FAIL;
    }
  str++;

  nb_regs = 0;
  typeinfo_first.defined = 0;
  typeinfo_first.type = NT_invtype;
  typeinfo_first.width = -1;
  typeinfo_first.element_size = 0;
  typeinfo_first.index = 0;
  ret_val = 0;
  val = -1u;
  val_range = -1u;
  in_range = 0;
  mask = reg_type_mask (type);
  do
    {
      if (in_range)
	{
	  str++;		/* skip over '-' */
	  val_range = val;
	}
      const reg_entry *reg = parse_typed_reg (&str, type, &typeinfo,
					      ptr_flags);
      if (!reg)
	{
	  set_first_syntax_error (_("invalid vector register in list"));
	  error = true;
	  continue;
	}
      val = reg->number;
      /* reject [bhsd]n */
      if (type == REG_TYPE_V && typeinfo.defined == 0)
	{
	  set_first_syntax_error (_("invalid scalar register in list"));
	  error = true;
	  continue;
	}

      if (typeinfo.defined & NTA_HASINDEX)
	expect_index = true;

      if (in_range)
	{
	  if (val == val_range)
	    {
	      set_first_syntax_error
		(_("invalid range in vector register list"));
	      error = true;
	    }
	  val_range = (val_range + 1) & mask;
	}
      else
	{
	  val_range = val;
	  if (nb_regs == 0)
	    typeinfo_first = typeinfo;
	  else if (! eq_vector_type_el (typeinfo_first, typeinfo))
	    {
	      set_first_syntax_error
		(_("type mismatch in vector register list"));
	      error = true;
	    }
	}
      if (! error)
	for (;;)
	  {
	    ret_val |= val_range << ((5 * nb_regs) & 31);
	    nb_regs++;
	    if (val_range == val)
	      break;
	    val_range = (val_range + 1) & mask;
	  }
      in_range = 0;
      ptr_flags |= PTR_GOOD_MATCH;
    }
  while (skip_past_comma (&str) || (in_range = 1, *str == '-'));

  skip_whitespace (str);
  if (*str != '}')
    {
      set_first_syntax_error (_("end of vector register list not found"));
      error = true;
    }
  str++;

  skip_whitespace (str);

  if (expect_index)
    {
      if (skip_past_char (&str, '['))
	{
	  if (!parse_index_expression (&str, &typeinfo_first.index))
	    error = true;
	  if (! skip_past_char (&str, ']'))
	    error = true;
	}
      else
	{
	  set_first_syntax_error (_("expected index"));
	  error = true;
	}
    }

  if (nb_regs > 4)
    {
      set_first_syntax_error (_("too many registers in vector register list"));
      error = true;
    }
  else if (nb_regs == 0)
    {
      set_first_syntax_error (_("empty vector register list"));
      error = true;
    }

  *ccp = str;
  if (! error)
    *vectype = typeinfo_first;

  return error ? PARSE_FAIL : (ret_val << 2) | (nb_regs - 1);
}

/* Directives: register aliases.  */

static reg_entry *
insert_reg_alias (char *str, int number, aarch64_reg_type type)
{
  reg_entry *new;
  const char *name;

  if ((new = str_hash_find (aarch64_reg_hsh, str)) != 0)
    {
      if (new->builtin)
	as_warn (_("ignoring attempt to redefine built-in register '%s'"),
		 str);

      /* Only warn about a redefinition if it's not defined as the
         same register.  */
      else if (new->number != number || new->type != type)
	as_warn (_("ignoring redefinition of register alias '%s'"), str);

      return NULL;
    }

  name = xstrdup (str);
  new = XNEW (reg_entry);

  new->name = name;
  new->number = number;
  new->type = type;
  new->builtin = false;

  str_hash_insert (aarch64_reg_hsh, name, new, 0);

  return new;
}

/* Look for the .req directive.	 This is of the form:

	new_register_name .req existing_register_name

   If we find one, or if it looks sufficiently like one that we want to
   handle any error here, return TRUE.  Otherwise return FALSE.  */

static bool
create_register_alias (char *newname, char *p)
{
  const reg_entry *old;
  char *oldname, *nbuf;
  size_t nlen;

  /* The input scrubber ensures that whitespace after the mnemonic is
     collapsed to single spaces.  */
  oldname = p;
  if (!startswith (oldname, " .req "))
    return false;

  oldname += 6;
  if (*oldname == '\0')
    return false;

  old = str_hash_find (aarch64_reg_hsh, oldname);
  if (!old)
    {
      as_warn (_("unknown register '%s' -- .req ignored"), oldname);
      return true;
    }

  /* If TC_CASE_SENSITIVE is defined, then newname already points to
     the desired alias name, and p points to its end.  If not, then
     the desired alias name is in the global original_case_string.  */
#ifdef TC_CASE_SENSITIVE
  nlen = p - newname;
#else
  newname = original_case_string;
  nlen = strlen (newname);
#endif

  nbuf = xmemdup0 (newname, nlen);

  /* Create aliases under the new name as stated; an all-lowercase
     version of the new name; and an all-uppercase version of the new
     name.  */
  if (insert_reg_alias (nbuf, old->number, old->type) != NULL)
    {
      for (p = nbuf; *p; p++)
	*p = TOUPPER (*p);

      if (strncmp (nbuf, newname, nlen))
	{
	  /* If this attempt to create an additional alias fails, do not bother
	     trying to create the all-lower case alias.  We will fail and issue
	     a second, duplicate error message.  This situation arises when the
	     programmer does something like:
	     foo .req r0
	     Foo .req r1
	     The second .req creates the "Foo" alias but then fails to create
	     the artificial FOO alias because it has already been created by the
	     first .req.  */
	  if (insert_reg_alias (nbuf, old->number, old->type) == NULL)
	    {
	      free (nbuf);
	      return true;
	    }
	}

      for (p = nbuf; *p; p++)
	*p = TOLOWER (*p);

      if (strncmp (nbuf, newname, nlen))
	insert_reg_alias (nbuf, old->number, old->type);
    }

  free (nbuf);
  return true;
}

/* Should never be called, as .req goes between the alias and the
   register name, not at the beginning of the line.  */
static void
s_req (int a ATTRIBUTE_UNUSED)
{
  as_bad (_("invalid syntax for .req directive"));
}

/* The .unreq directive deletes an alias which was previously defined
   by .req.  For example:

       my_alias .req r11
       .unreq my_alias	  */

static void
s_unreq (int a ATTRIBUTE_UNUSED)
{
  char *name;
  char saved_char;

  name = input_line_pointer;
  input_line_pointer = find_end_of_line (input_line_pointer, flag_m68k_mri);
  saved_char = *input_line_pointer;
  *input_line_pointer = 0;

  if (!*name)
    as_bad (_("invalid syntax for .unreq directive"));
  else
    {
      reg_entry *reg = str_hash_find (aarch64_reg_hsh, name);

      if (!reg)
	as_bad (_("unknown register alias '%s'"), name);
      else if (reg->builtin)
	as_warn (_("ignoring attempt to undefine built-in register '%s'"),
		 name);
      else
	{
	  char *p;
	  char *nbuf;

	  str_hash_delete (aarch64_reg_hsh, name);
	  free ((char *) reg->name);
	  free (reg);

	  /* Also locate the all upper case and all lower case versions.
	     Do not complain if we cannot find one or the other as it
	     was probably deleted above.  */

	  nbuf = strdup (name);
	  for (p = nbuf; *p; p++)
	    *p = TOUPPER (*p);
	  reg = str_hash_find (aarch64_reg_hsh, nbuf);
	  if (reg)
	    {
	      str_hash_delete (aarch64_reg_hsh, nbuf);
	      free ((char *) reg->name);
	      free (reg);
	    }

	  for (p = nbuf; *p; p++)
	    *p = TOLOWER (*p);
	  reg = str_hash_find (aarch64_reg_hsh, nbuf);
	  if (reg)
	    {
	      str_hash_delete (aarch64_reg_hsh, nbuf);
	      free ((char *) reg->name);
	      free (reg);
	    }

	  free (nbuf);
	}
    }

  *input_line_pointer = saved_char;
  demand_empty_rest_of_line ();
}

/* Directives: Instruction set selection.  */

#if defined OBJ_ELF || defined OBJ_COFF
/* This code is to handle mapping symbols as defined in the ARM AArch64 ELF
   spec.  (See "Mapping symbols", section 4.5.4, ARM AAELF64 version 0.05).
   Note that previously, $a and $t has type STT_FUNC (BSF_OBJECT flag),
   and $d has type STT_OBJECT (BSF_OBJECT flag). Now all three are untyped.  */

/* Create a new mapping symbol for the transition to STATE.  */

static void
make_mapping_symbol (enum mstate state, valueT value, fragS * frag)
{
  symbolS *symbolP;
  const char *symname;
  int type;

  switch (state)
    {
    case MAP_DATA:
      symname = "$d";
      type = BSF_NO_FLAGS;
      break;
    case MAP_INSN:
      symname = "$x";
      type = BSF_NO_FLAGS;
      break;
    default:
      abort ();
    }

  symbolP = symbol_new (symname, now_seg, frag, value);
  symbol_get_bfdsym (symbolP)->flags |= type | BSF_LOCAL;

  /* Save the mapping symbols for future reference.  Also check that
     we do not place two mapping symbols at the same offset within a
     frag.  We'll handle overlap between frags in
     check_mapping_symbols.

     If .fill or other data filling directive generates zero sized data,
     the mapping symbol for the following code will have the same value
     as the one generated for the data filling directive.  In this case,
     we replace the old symbol with the new one at the same address.  */
  if (value == 0)
    {
      if (frag->tc_frag_data.first_map != NULL)
	{
	  know (S_GET_VALUE (frag->tc_frag_data.first_map) == 0);
	  symbol_remove (frag->tc_frag_data.first_map, &symbol_rootP,
			 &symbol_lastP);
	}
      frag->tc_frag_data.first_map = symbolP;
    }
  if (frag->tc_frag_data.last_map != NULL)
    {
      know (S_GET_VALUE (frag->tc_frag_data.last_map) <=
	    S_GET_VALUE (symbolP));
      if (S_GET_VALUE (frag->tc_frag_data.last_map) == S_GET_VALUE (symbolP))
	symbol_remove (frag->tc_frag_data.last_map, &symbol_rootP,
		       &symbol_lastP);
    }
  frag->tc_frag_data.last_map = symbolP;
}

/* We must sometimes convert a region marked as code to data during
   code alignment, if an odd number of bytes have to be padded.  The
   code mapping symbol is pushed to an aligned address.  */

static void
insert_data_mapping_symbol (enum mstate state,
			    valueT value, fragS * frag, offsetT bytes)
{
  /* If there was already a mapping symbol, remove it.  */
  if (frag->tc_frag_data.last_map != NULL
      && S_GET_VALUE (frag->tc_frag_data.last_map) ==
      frag->fr_address + value)
    {
      symbolS *symp = frag->tc_frag_data.last_map;

      if (value == 0)
	{
	  know (frag->tc_frag_data.first_map == symp);
	  frag->tc_frag_data.first_map = NULL;
	}
      frag->tc_frag_data.last_map = NULL;
      symbol_remove (symp, &symbol_rootP, &symbol_lastP);
    }

  make_mapping_symbol (MAP_DATA, value, frag);
  make_mapping_symbol (state, value + bytes, frag);
}

static void mapping_state_2 (enum mstate state, int max_chars);

/* Set the mapping state to STATE.  Only call this when about to
   emit some STATE bytes to the file.  */

void
mapping_state (enum mstate state)
{
  enum mstate mapstate = seg_info (now_seg)->tc_segment_info_data.mapstate;

  if (state == MAP_INSN)
    /* AArch64 instructions require 4-byte alignment.  When emitting
       instructions into any section, record the appropriate section
       alignment.  */
    record_alignment (now_seg, 2);

  if (mapstate == state)
    /* The mapping symbol has already been emitted.
       There is nothing else to do.  */
    return;

#define TRANSITION(from, to) (mapstate == (from) && state == (to))
  if (TRANSITION (MAP_UNDEFINED, MAP_DATA) && !subseg_text_p (now_seg))
    /* Emit MAP_DATA within executable section in order.  Otherwise, it will be
       evaluated later in the next else.  */
    return;
  else if (TRANSITION (MAP_UNDEFINED, MAP_INSN))
    {
      /* Only add the symbol if the offset is > 0:
	 if we're at the first frag, check it's size > 0;
	 if we're not at the first frag, then for sure
	 the offset is > 0.  */
      struct frag *const frag_first = seg_info (now_seg)->frchainP->frch_root;
      const int add_symbol = (frag_now != frag_first)
	|| (frag_now_fix () > 0);

      if (add_symbol)
	make_mapping_symbol (MAP_DATA, (valueT) 0, frag_first);
    }
#undef TRANSITION

  mapping_state_2 (state, 0);
}

/* Same as mapping_state, but MAX_CHARS bytes have already been
   allocated.  Put the mapping symbol that far back.  */

static void
mapping_state_2 (enum mstate state, int max_chars)
{
  enum mstate mapstate = seg_info (now_seg)->tc_segment_info_data.mapstate;

  if (!SEG_NORMAL (now_seg))
    return;

  if (mapstate == state)
    /* The mapping symbol has already been emitted.
       There is nothing else to do.  */
    return;

  seg_info (now_seg)->tc_segment_info_data.mapstate = state;
  make_mapping_symbol (state, (valueT) frag_now_fix () - max_chars, frag_now);
}
#else
#define mapping_state(x)	/* nothing */
#define mapping_state_2(x, y)	/* nothing */
#endif

/* Directives: sectioning and alignment.  */

static void
s_bss (int ignore ATTRIBUTE_UNUSED)
{
  /* We don't support putting frags in the BSS segment, we fake it by
     marking in_bss, then looking at s_skip for clues.  */
  subseg_set (bss_section, 0);
  demand_empty_rest_of_line ();
  mapping_state (MAP_DATA);
}

static void
s_even (int ignore ATTRIBUTE_UNUSED)
{
  /* Never make frag if expect extra pass.  */
  if (!need_pass_2)
    frag_align (1, 0, 0);

  record_alignment (now_seg, 1);

  demand_empty_rest_of_line ();
}

/* Directives: Literal pools.  */

static literal_pool *
find_literal_pool (int size)
{
  literal_pool *pool;

  for (pool = list_of_pools; pool != NULL; pool = pool->next)
    {
      if (pool->section == now_seg
	  && pool->sub_section == now_subseg && pool->size == size)
	break;
    }

  return pool;
}

static literal_pool *
find_or_make_literal_pool (int size)
{
  /* Next literal pool ID number.  */
  static unsigned int latest_pool_num = 1;
  literal_pool *pool;

  pool = find_literal_pool (size);

  if (pool == NULL)
    {
      /* Create a new pool.  */
      pool = XNEW (literal_pool);
      if (!pool)
	return NULL;

      /* Currently we always put the literal pool in the current text
         section.  If we were generating "small" model code where we
         knew that all code and initialised data was within 1MB then
         we could output literals to mergeable, read-only data
         sections. */

      pool->next_free_entry = 0;
      pool->section = now_seg;
      pool->sub_section = now_subseg;
      pool->size = size;
      pool->next = list_of_pools;
      pool->symbol = NULL;

      /* Add it to the list.  */
      list_of_pools = pool;
    }

  /* New pools, and emptied pools, will have a NULL symbol.  */
  if (pool->symbol == NULL)
    {
      pool->symbol = symbol_create (FAKE_LABEL_NAME, undefined_section,
				    &zero_address_frag, 0);
      pool->id = latest_pool_num++;
    }

  /* Done.  */
  return pool;
}

/* Add the literal of size SIZE in *EXP to the relevant literal pool.
   Return TRUE on success, otherwise return FALSE.  */
static bool
add_to_lit_pool (expressionS *exp, int size)
{
  literal_pool *pool;
  unsigned int entry;

  pool = find_or_make_literal_pool (size);

  /* Check if this literal value is already in the pool.  */
  for (entry = 0; entry < pool->next_free_entry; entry++)
    {
      expressionS * litexp = & pool->literals[entry].exp;

      if ((litexp->X_op == exp->X_op)
	  && (exp->X_op == O_constant)
	  && (litexp->X_add_number == exp->X_add_number)
	  && (litexp->X_unsigned == exp->X_unsigned))
	break;

      if ((litexp->X_op == exp->X_op)
	  && (exp->X_op == O_symbol)
	  && (litexp->X_add_number == exp->X_add_number)
	  && (litexp->X_add_symbol == exp->X_add_symbol)
	  && (litexp->X_op_symbol == exp->X_op_symbol))
	break;
    }

  /* Do we need to create a new entry?  */
  if (entry == pool->next_free_entry)
    {
      if (entry >= MAX_LITERAL_POOL_SIZE)
	{
	  set_syntax_error (_("literal pool overflow"));
	  return false;
	}

      pool->literals[entry].exp = *exp;
      pool->next_free_entry += 1;
      if (exp->X_op == O_big)
	{
	  /* PR 16688: Bignums are held in a single global array.  We must
	     copy and preserve that value now, before it is overwritten.  */
	  pool->literals[entry].bignum = XNEWVEC (LITTLENUM_TYPE,
						  exp->X_add_number);
	  memcpy (pool->literals[entry].bignum, generic_bignum,
		  CHARS_PER_LITTLENUM * exp->X_add_number);
	}
      else
	pool->literals[entry].bignum = NULL;
    }

  exp->X_op = O_symbol;
  exp->X_add_number = ((int) entry) * size;
  exp->X_add_symbol = pool->symbol;

  return true;
}

/* Can't use symbol_new here, so have to create a symbol and then at
   a later date assign it a value. That's what these functions do.  */

static void
symbol_locate (symbolS * symbolP,
	       const char *name,/* It is copied, the caller can modify.  */
	       segT segment,	/* Segment identifier (SEG_<something>).  */
	       valueT valu,	/* Symbol value.  */
	       fragS * frag)	/* Associated fragment.  */
{
  size_t name_length;
  char *preserved_copy_of_name;

  name_length = strlen (name) + 1;	/* +1 for \0.  */
  obstack_grow (&notes, name, name_length);
  preserved_copy_of_name = obstack_finish (&notes);

#ifdef tc_canonicalize_symbol_name
  preserved_copy_of_name =
    tc_canonicalize_symbol_name (preserved_copy_of_name);
#endif

  S_SET_NAME (symbolP, preserved_copy_of_name);

  S_SET_SEGMENT (symbolP, segment);
  S_SET_VALUE (symbolP, valu);
  symbol_clear_list_pointers (symbolP);

  symbol_set_frag (symbolP, frag);

  /* Link to end of symbol chain.  */
  {
    extern int symbol_table_frozen;

    if (symbol_table_frozen)
      abort ();
  }

  symbol_append (symbolP, symbol_lastP, &symbol_rootP, &symbol_lastP);

  obj_symbol_new_hook (symbolP);

#ifdef tc_symbol_new_hook
  tc_symbol_new_hook (symbolP);
#endif

#ifdef DEBUG_SYMS
  verify_symbol_chain (symbol_rootP, symbol_lastP);
#endif /* DEBUG_SYMS  */
}


static void
s_ltorg (int ignored ATTRIBUTE_UNUSED)
{
  unsigned int entry;
  literal_pool *pool;
  char sym_name[20];
  int align;

  for (align = 2; align <= 4; align++)
    {
      int size = 1 << align;

      pool = find_literal_pool (size);
      if (pool == NULL || pool->symbol == NULL || pool->next_free_entry == 0)
	continue;

      /* Align pool as you have word accesses.
         Only make a frag if we have to.  */
      if (!need_pass_2)
	frag_align (align, 0, 0);

      mapping_state (MAP_DATA);

      record_alignment (now_seg, align);

      sprintf (sym_name, "$$lit_\002%x", pool->id);

      symbol_locate (pool->symbol, sym_name, now_seg,
		     (valueT) frag_now_fix (), frag_now);
      symbol_table_insert (pool->symbol);

      for (entry = 0; entry < pool->next_free_entry; entry++)
	{
	  expressionS * exp = & pool->literals[entry].exp;

	  if (exp->X_op == O_big)
	    {
	      /* PR 16688: Restore the global bignum value.  */
	      gas_assert (pool->literals[entry].bignum != NULL);
	      memcpy (generic_bignum, pool->literals[entry].bignum,
		      CHARS_PER_LITTLENUM * exp->X_add_number);
	    }

	  /* First output the expression in the instruction to the pool.  */
	  emit_expr (exp, size);	/* .word|.xword  */

	  if (exp->X_op == O_big)
	    {
	      free (pool->literals[entry].bignum);
	      pool->literals[entry].bignum = NULL;
	    }
	}

      /* Mark the pool as empty.  */
      pool->next_free_entry = 0;
      pool->symbol = NULL;
    }
}

#if defined(OBJ_ELF) || defined(OBJ_COFF)
/* Forward declarations for functions below, in the MD interface
   section.  */
static struct reloc_table_entry * find_reloc_table_entry (char **);

/* Directives: Data.  */
/* N.B. the support for relocation suffix in this directive needs to be
   implemented properly.  */

static void
s_aarch64_cons (int nbytes)
{
  expressionS exp;

#ifdef md_flush_pending_output
  md_flush_pending_output ();
#endif

  if (is_it_end_of_statement ())
    {
      demand_empty_rest_of_line ();
      return;
    }

#ifdef md_cons_align
  md_cons_align (nbytes);
#endif

  mapping_state (MAP_DATA);
  do
    {
      struct reloc_table_entry *reloc;

      expression (&exp);

      if (exp.X_op != O_symbol)
	emit_expr (&exp, (unsigned int) nbytes);
      else
	{
	  skip_past_char (&input_line_pointer, '#');
	  if (skip_past_char (&input_line_pointer, ':'))
	    {
	      reloc = find_reloc_table_entry (&input_line_pointer);
	      if (reloc == NULL)
		as_bad (_("unrecognized relocation suffix"));
	      else
		as_bad (_("unimplemented relocation suffix"));
	      ignore_rest_of_line ();
	      return;
	    }
	  else
	    emit_expr (&exp, (unsigned int) nbytes);
	}
    }
  while (*input_line_pointer++ == ',');

  /* Put terminator back into stream.  */
  input_line_pointer--;
  demand_empty_rest_of_line ();
}
#endif

#ifdef OBJ_ELF
/* Forward declarations for functions below, in the MD interface
   section.  */
 static fixS *fix_new_aarch64 (fragS *, int, short, expressionS *, int, int);

/* Mark symbol that it follows a variant PCS convention.  */

static void
s_variant_pcs (int ignored ATTRIBUTE_UNUSED)
{
  char *name;
  char c;
  symbolS *sym;
  asymbol *bfdsym;
  elf_symbol_type *elfsym;

  c = get_symbol_name (&name);
  if (!*name)
    as_bad (_("Missing symbol name in directive"));
  sym = symbol_find_or_make (name);
  restore_line_pointer (c);
  demand_empty_rest_of_line ();
  bfdsym = symbol_get_bfdsym (sym);
  elfsym = elf_symbol_from (bfdsym);
  gas_assert (elfsym);
  elfsym->internal_elf_sym.st_other |= STO_AARCH64_VARIANT_PCS;
}
#endif /* OBJ_ELF */

/* Output a 32-bit word, but mark as an instruction.  */

static void
s_aarch64_inst (int ignored ATTRIBUTE_UNUSED)
{
  expressionS exp;
  unsigned n = 0;

#ifdef md_flush_pending_output
  md_flush_pending_output ();
#endif

  if (is_it_end_of_statement ())
    {
      demand_empty_rest_of_line ();
      return;
    }

  /* Sections are assumed to start aligned. In executable section, there is no
     MAP_DATA symbol pending. So we only align the address during
     MAP_DATA --> MAP_INSN transition.
     For other sections, this is not guaranteed.  */
  enum mstate mapstate = seg_info (now_seg)->tc_segment_info_data.mapstate;
  if (!need_pass_2 && subseg_text_p (now_seg) && mapstate == MAP_DATA)
    frag_align_code (2, 0);

#ifdef OBJ_ELF
  mapping_state (MAP_INSN);
#endif

  do
    {
      expression (&exp);
      if (exp.X_op != O_constant)
	{
	  as_bad (_("constant expression required"));
	  ignore_rest_of_line ();
	  return;
	}

      if (target_big_endian)
	{
	  unsigned int val = exp.X_add_number;
	  exp.X_add_number = SWAP_32 (val);
	}
      emit_expr (&exp, INSN_SIZE);
      ++n;
    }
  while (*input_line_pointer++ == ',');

  dwarf2_emit_insn (n * INSN_SIZE);

  /* Put terminator back into stream.  */
  input_line_pointer--;
  demand_empty_rest_of_line ();
}

static void
s_aarch64_cfi_b_key_frame (int ignored ATTRIBUTE_UNUSED)
{
  demand_empty_rest_of_line ();
  struct fde_entry *fde = frchain_now->frch_cfi_data->cur_fde_data;
  fde->pauth_key = AARCH64_PAUTH_KEY_B;
}

#ifdef OBJ_ELF
/* Emit BFD_RELOC_AARCH64_TLSDESC_ADD on the next ADD instruction.  */

static void
s_tlsdescadd (int ignored ATTRIBUTE_UNUSED)
{
  expressionS exp;

  expression (&exp);
  frag_grow (4);
  fix_new_aarch64 (frag_now, frag_more (0) - frag_now->fr_literal, 4, &exp, 0,
		   BFD_RELOC_AARCH64_TLSDESC_ADD);

  demand_empty_rest_of_line ();
}

/* Emit BFD_RELOC_AARCH64_TLSDESC_CALL on the next BLR instruction.  */

static void
s_tlsdesccall (int ignored ATTRIBUTE_UNUSED)
{
  expressionS exp;

  /* Since we're just labelling the code, there's no need to define a
     mapping symbol.  */
  expression (&exp);
  /* Make sure there is enough room in this frag for the following
     blr.  This trick only works if the blr follows immediately after
     the .tlsdesc directive.  */
  frag_grow (4);
  fix_new_aarch64 (frag_now, frag_more (0) - frag_now->fr_literal, 4, &exp, 0,
		   BFD_RELOC_AARCH64_TLSDESC_CALL);

  demand_empty_rest_of_line ();
}

/* Emit BFD_RELOC_AARCH64_TLSDESC_LDR on the next LDR instruction.  */

static void
s_tlsdescldr (int ignored ATTRIBUTE_UNUSED)
{
  expressionS exp;

  expression (&exp);
  frag_grow (4);
  fix_new_aarch64 (frag_now, frag_more (0) - frag_now->fr_literal, 4, &exp, 0,
		   BFD_RELOC_AARCH64_TLSDESC_LDR);

  demand_empty_rest_of_line ();
}
#endif	/* OBJ_ELF */

#ifdef TE_PE
static void
s_secrel (int dummy ATTRIBUTE_UNUSED)
{
  expressionS exp;

  do
    {
      expression (&exp);
      if (exp.X_op == O_symbol)
	exp.X_op = O_secrel;

      emit_expr (&exp, 4);
    }
  while (*input_line_pointer++ == ',');

  input_line_pointer--;
  demand_empty_rest_of_line ();
}

void
tc_pe_dwarf2_emit_offset (symbolS *symbol, unsigned int size)
{
  expressionS exp;

  exp.X_op = O_secrel;
  exp.X_add_symbol = symbol;
  exp.X_add_number = 0;
  emit_expr (&exp, size);
}

static void
s_secidx (int dummy ATTRIBUTE_UNUSED)
{
  expressionS exp;

  do
    {
      expression (&exp);
      if (exp.X_op == O_symbol)
	exp.X_op = O_secidx;

      emit_expr (&exp, 2);
    }
  while (*input_line_pointer++ == ',');

  input_line_pointer--;
  demand_empty_rest_of_line ();
}
#endif	/* TE_PE */

static void s_aarch64_arch (int);
static void s_aarch64_cpu (int);
static void s_aarch64_arch_extension (int);

/* This table describes all the machine specific pseudo-ops the assembler
   has to support.  The fields are:
     pseudo-op name without dot
     function to call to execute this pseudo-op
     Integer arg to pass to the function.  */

const pseudo_typeS md_pseudo_table[] = {
  /* Never called because '.req' does not start a line.  */
  {"req", s_req, 0},
  {"unreq", s_unreq, 0},
  {"bss", s_bss, 0},
  {"even", s_even, 0},
  {"ltorg", s_ltorg, 0},
  {"pool", s_ltorg, 0},
  {"cpu", s_aarch64_cpu, 0},
  {"arch", s_aarch64_arch, 0},
  {"arch_extension", s_aarch64_arch_extension, 0},
  {"inst", s_aarch64_inst, 0},
  {"cfi_b_key_frame", s_aarch64_cfi_b_key_frame, 0},
#ifdef OBJ_ELF
  {"tlsdescadd", s_tlsdescadd, 0},
  {"tlsdesccall", s_tlsdesccall, 0},
  {"tlsdescldr", s_tlsdescldr, 0},
  {"variant_pcs", s_variant_pcs, 0},
#endif
#if defined(OBJ_ELF) || defined(OBJ_COFF)
  {"word", s_aarch64_cons, 4},
  {"long", s_aarch64_cons, 4},
  {"xword", s_aarch64_cons, 8},
  {"dword", s_aarch64_cons, 8},
#endif
#ifdef TE_PE
  {"secrel32", s_secrel, 0},
  {"secidx", s_secidx, 0},
#endif
  {"float16", float_cons, 'h'},
  {"bfloat16", float_cons, 'b'},
  {0, 0, 0}
};


/* Check whether STR points to a register name followed by a comma or the
   end of line; REG_TYPE indicates which register types are checked
   against.  Return TRUE if STR is such a register name; otherwise return
   FALSE.  The function does not intend to produce any diagnostics, but since
   the register parser aarch64_reg_parse, which is called by this function,
   does produce diagnostics, we call clear_error to clear any diagnostics
   that may be generated by aarch64_reg_parse.
   Also, the function returns FALSE directly if there is any user error
   present at the function entry.  This prevents the existing diagnostics
   state from being spoiled.
   The function currently serves parse_constant_immediate and
   parse_big_immediate only.  */
static bool
reg_name_p (char *str, aarch64_reg_type reg_type)
{
  const reg_entry *reg;

  /* Prevent the diagnostics state from being spoiled.  */
  if (error_p ())
    return false;

  reg = aarch64_reg_parse (&str, reg_type, NULL);

  /* Clear the parsing error that may be set by the reg parser.  */
  clear_error ();

  if (!reg)
    return false;

  skip_whitespace (str);
  if (*str == ',' || is_end_of_line[(unsigned char) *str])
    return true;

  return false;
}

/* Parser functions used exclusively in instruction operands.  */

/* Parse an immediate expression which may not be constant.

   To prevent the expression parser from pushing a register name
   into the symbol table as an undefined symbol, firstly a check is
   done to find out whether STR is a register of type REG_TYPE followed
   by a comma or the end of line.  Return FALSE if STR is such a string.  */

static bool
parse_immediate_expression (char **str, expressionS *exp,
			    aarch64_reg_type reg_type)
{
  if (reg_name_p (*str, reg_type))
    {
      set_recoverable_error (_("immediate operand required"));
      return false;
    }

  aarch64_get_expression (exp, str, GE_OPT_PREFIX, REJECT_ABSENT);

  if (exp->X_op == O_absent)
    {
      set_fatal_syntax_error (_("missing immediate expression"));
      return false;
    }

  return true;
}

/* Constant immediate-value read function for use in insn parsing.
   STR points to the beginning of the immediate (with the optional
   leading #); *VAL receives the value.  REG_TYPE says which register
   names should be treated as registers rather than as symbolic immediates.

   Return TRUE on success; otherwise return FALSE.  */

static bool
parse_constant_immediate (char **str, int64_t *val, aarch64_reg_type reg_type)
{
  expressionS exp;

  if (! parse_immediate_expression (str, &exp, reg_type))
    return false;

  if (exp.X_op != O_constant)
    {
      set_syntax_error (_("constant expression required"));
      return false;
    }

  *val = exp.X_add_number;
  return true;
}

static uint32_t
encode_imm_float_bits (uint32_t imm)
{
  return ((imm >> 19) & 0x7f)	/* b[25:19] -> b[6:0] */
    | ((imm >> (31 - 7)) & 0x80);	/* b[31]    -> b[7]   */
}

/* Return TRUE if the single-precision floating-point value encoded in IMM
   can be expressed in the AArch64 8-bit signed floating-point format with
   3-bit exponent and normalized 4 bits of precision; in other words, the
   floating-point value must be expressable as
     (+/-) n / 16 * power (2, r)
   where n and r are integers such that 16 <= n <=31 and -3 <= r <= 4.  */

static bool
aarch64_imm_float_p (uint32_t imm)
{
  /* If a single-precision floating-point value has the following bit
     pattern, it can be expressed in the AArch64 8-bit floating-point
     format:

     3 32222222 2221111111111
     1 09876543 21098765432109876543210
     n Eeeeeexx xxxx0000000000000000000

     where n, e and each x are either 0 or 1 independently, with
     E == ~ e.  */

  uint32_t pattern;

  /* Prepare the pattern for 'Eeeeee'.  */
  if (((imm >> 30) & 0x1) == 0)
    pattern = 0x3e000000;
  else
    pattern = 0x40000000;

  return (imm & 0x7ffff) == 0		/* lower 19 bits are 0.  */
    && ((imm & 0x7e000000) == pattern);	/* bits 25 - 29 == ~ bit 30.  */
}

/* Return TRUE if the IEEE double value encoded in IMM can be expressed
   as an IEEE float without any loss of precision.  Store the value in
   *FPWORD if so.  */

static bool
can_convert_double_to_float (uint64_t imm, uint32_t *fpword)
{
  /* If a double-precision floating-point value has the following bit
     pattern, it can be expressed in a float:

     6 66655555555 5544 44444444 33333333 33222222 22221111 111111
     3 21098765432 1098 76543210 98765432 10987654 32109876 54321098 76543210
     n E~~~eeeeeee ssss ssssssss ssssssss SSS00000 00000000 00000000 00000000

       ----------------------------->     nEeeeeee esssssss ssssssss sssssSSS
	 if Eeee_eeee != 1111_1111

     where n, e, s and S are either 0 or 1 independently and where ~ is the
     inverse of E.  */

  uint32_t pattern;
  uint32_t high32 = imm >> 32;
  uint32_t low32 = imm;

  /* Lower 29 bits need to be 0s.  */
  if ((imm & 0x1fffffff) != 0)
    return false;

  /* Prepare the pattern for 'Eeeeeeeee'.  */
  if (((high32 >> 30) & 0x1) == 0)
    pattern = 0x38000000;
  else
    pattern = 0x40000000;

  /* Check E~~~.  */
  if ((high32 & 0x78000000) != pattern)
    return false;

  /* Check Eeee_eeee != 1111_1111.  */
  if ((high32 & 0x7ff00000) == 0x47f00000)
    return false;

  *fpword = ((high32 & 0xc0000000)		/* 1 n bit and 1 E bit.  */
	     | ((high32 << 3) & 0x3ffffff8)	/* 7 e and 20 s bits.  */
	     | (low32 >> 29));			/* 3 S bits.  */
  return true;
}

/* Return true if we should treat OPERAND as a double-precision
   floating-point operand rather than a single-precision one.  */
static bool
double_precision_operand_p (const aarch64_opnd_info *operand)
{
  /* Check for unsuffixed SVE registers, which are allowed
     for LDR and STR but not in instructions that require an
     immediate.  We get better error messages if we arbitrarily
     pick one size, parse the immediate normally, and then
     report the match failure in the normal way.  */
  return (operand->qualifier == AARCH64_OPND_QLF_NIL
	  || aarch64_get_qualifier_esize (operand->qualifier) == 8);
}

/* Parse a floating-point immediate.  Return TRUE on success and return the
   value in *IMMED in the format of IEEE754 single-precision encoding.
   *CCP points to the start of the string; DP_P is TRUE when the immediate
   is expected to be in double-precision (N.B. this only matters when
   hexadecimal representation is involved).  REG_TYPE says which register
   names should be treated as registers rather than as symbolic immediates.

   This routine accepts any IEEE float; it is up to the callers to reject
   invalid ones.  */

static bool
parse_aarch64_imm_float (char **ccp, int *immed, bool dp_p,
			 aarch64_reg_type reg_type)
{
  char *str = *ccp;
  char *fpnum;
  LITTLENUM_TYPE words[MAX_LITTLENUMS];
  int64_t val = 0;
  unsigned fpword = 0;
  bool hex_p = false;

  skip_past_char (&str, '#');

  fpnum = str;
  skip_whitespace (fpnum);

  if (startswith (fpnum, "0x"))
    {
      /* Support the hexadecimal representation of the IEEE754 encoding.
	 Double-precision is expected when DP_P is TRUE, otherwise the
	 representation should be in single-precision.  */
      if (! parse_constant_immediate (&str, &val, reg_type))
	goto invalid_fp;

      if (dp_p)
	{
	  if (!can_convert_double_to_float (val, &fpword))
	    goto invalid_fp;
	}
      else if ((uint64_t) val > 0xffffffff)
	goto invalid_fp;
      else
	fpword = val;

      hex_p = true;
    }
  else if (reg_name_p (str, reg_type))
   {
     set_recoverable_error (_("immediate operand required"));
     return false;
    }

  if (! hex_p)
    {
      int i;

      if ((str = atof_ieee (str, 's', words)) == NULL)
	goto invalid_fp;

      /* Our FP word must be 32 bits (single-precision FP).  */
      for (i = 0; i < 32 / LITTLENUM_NUMBER_OF_BITS; i++)
	{
	  fpword <<= LITTLENUM_NUMBER_OF_BITS;
	  fpword |= words[i];
	}
    }

  *immed = fpword;
  *ccp = str;
  return true;

 invalid_fp:
  set_fatal_syntax_error (_("invalid floating-point constant"));
  return false;
}

/* Less-generic immediate-value read function with the possibility of loading
   a big (64-bit) immediate, as required by AdvSIMD Modified immediate
   instructions.

   To prevent the expression parser from pushing a register name into the
   symbol table as an undefined symbol, a check is firstly done to find
   out whether STR is a register of type REG_TYPE followed by a comma or
   the end of line.  Return FALSE if STR is such a register.  */

static bool
parse_big_immediate (char **str, int64_t *imm, aarch64_reg_type reg_type)
{
  char *ptr = *str;

  if (reg_name_p (ptr, reg_type))
    {
      set_syntax_error (_("immediate operand required"));
      return false;
    }

  aarch64_get_expression (&inst.reloc.exp, &ptr, GE_OPT_PREFIX, REJECT_ABSENT);

  if (inst.reloc.exp.X_op == O_constant)
    *imm = inst.reloc.exp.X_add_number;

  *str = ptr;

  return true;
}

/* Set operand IDX of the *INSTR that needs a GAS internal fixup.
   if NEED_LIBOPCODES is non-zero, the fixup will need
   assistance from the libopcodes.   */

static inline void
aarch64_set_gas_internal_fixup (struct reloc *reloc,
				const aarch64_opnd_info *operand,
				int need_libopcodes_p)
{
  reloc->type = BFD_RELOC_AARCH64_GAS_INTERNAL_FIXUP;
  reloc->opnd = operand->type;
  if (need_libopcodes_p)
    reloc->need_libopcodes_p = 1;
};

/* Return TRUE if the instruction needs to be fixed up later internally by
   the GAS; otherwise return FALSE.  */

static inline bool
aarch64_gas_internal_fixup_p (void)
{
  return inst.reloc.type == BFD_RELOC_AARCH64_GAS_INTERNAL_FIXUP;
}

/* Assign the immediate value to the relevant field in *OPERAND if
   RELOC->EXP is a constant expression; otherwise, flag that *OPERAND
   needs an internal fixup in a later stage.
   ADDR_OFF_P determines whether it is the field ADDR.OFFSET.IMM or
   IMM.VALUE that may get assigned with the constant.  */
static inline void
assign_imm_if_const_or_fixup_later (struct reloc *reloc,
				    aarch64_opnd_info *operand,
				    int addr_off_p,
				    int need_libopcodes_p,
				    int skip_p)
{
  if (reloc->exp.X_op == O_constant)
    {
      if (addr_off_p)
	operand->addr.offset.imm = reloc->exp.X_add_number;
      else
	operand->imm.value = reloc->exp.X_add_number;
      reloc->type = BFD_RELOC_UNUSED;
    }
  else
    {
      aarch64_set_gas_internal_fixup (reloc, operand, need_libopcodes_p);
      /* Tell libopcodes to ignore this operand or not.  This is helpful
	 when one of the operands needs to be fixed up later but we need
	 libopcodes to check the other operands.  */
      operand->skip = skip_p;
    }
}

/* Relocation modifiers.  Each entry in the table contains the textual
   name for the relocation which may be placed before a symbol used as
   a load/store offset, or add immediate. It must be surrounded by a
   leading and trailing colon, for example:

	ldr	x0, [x1, #:rello:varsym]
	add	x0, x1, #:rello:varsym  */

struct reloc_table_entry
{
  const char *name;
  int pc_rel;
  bfd_reloc_code_real_type adr_type;
  bfd_reloc_code_real_type adrp_type;
  bfd_reloc_code_real_type movw_type;
  bfd_reloc_code_real_type add_type;
  bfd_reloc_code_real_type ldst_type;
  bfd_reloc_code_real_type ld_literal_type;
};

static struct reloc_table_entry reloc_table[] =
{
  /* Low 12 bits of absolute address: ADD/i and LDR/STR */
  {"lo12", 0,
   0,				/* adr_type */
   0,
   0,
   BFD_RELOC_AARCH64_ADD_LO12,
   BFD_RELOC_AARCH64_LDST_LO12,
   0},

  /* Higher 21 bits of pc-relative page offset: ADRP */
  {"pg_hi21", 1,
   0,				/* adr_type */
   BFD_RELOC_AARCH64_ADR_HI21_PCREL,
   0,
   0,
   0,
   0},

  /* Higher 21 bits of pc-relative page offset: ADRP, no check */
  {"pg_hi21_nc", 1,
   0,				/* adr_type */
   BFD_RELOC_AARCH64_ADR_HI21_NC_PCREL,
   0,
   0,
   0,
   0},

  /* Most significant bits 0-15 of unsigned address/value: MOVZ */
  {"abs_g0", 0,
   0,				/* adr_type */
   0,
   BFD_RELOC_AARCH64_MOVW_G0,
   0,
   0,
   0},

  /* Most significant bits 0-15 of signed address/value: MOVN/Z */
  {"abs_g0_s", 0,
   0,				/* adr_type */
   0,
   BFD_RELOC_AARCH64_MOVW_G0_S,
   0,
   0,
   0},

  /* Less significant bits 0-15 of address/value: MOVK, no check */
  {"abs_g0_nc", 0,
   0,				/* adr_type */
   0,
   BFD_RELOC_AARCH64_MOVW_G0_NC,
   0,
   0,
   0},

  /* Most significant bits 16-31 of unsigned address/value: MOVZ */
  {"abs_g1", 0,
   0,				/* adr_type */
   0,
   BFD_RELOC_AARCH64_MOVW_G1,
   0,
   0,
   0},

  /* Most significant bits 16-31 of signed address/value: MOVN/Z */
  {"abs_g1_s", 0,
   0,				/* adr_type */
   0,
   BFD_RELOC_AARCH64_MOVW_G1_S,
   0,
   0,
   0},

  /* Less significant bits 16-31 of address/value: MOVK, no check */
  {"abs_g1_nc", 0,
   0,				/* adr_type */
   0,
   BFD_RELOC_AARCH64_MOVW_G1_NC,
   0,
   0,
   0},

  /* Most significant bits 32-47 of unsigned address/value: MOVZ */
  {"abs_g2", 0,
   0,				/* adr_type */
   0,
   BFD_RELOC_AARCH64_MOVW_G2,
   0,
   0,
   0},

  /* Most significant bits 32-47 of signed address/value: MOVN/Z */
  {"abs_g2_s", 0,
   0,				/* adr_type */
   0,
   BFD_RELOC_AARCH64_MOVW_G2_S,
   0,
   0,
   0},

  /* Less significant bits 32-47 of address/value: MOVK, no check */
  {"abs_g2_nc", 0,
   0,				/* adr_type */
   0,
   BFD_RELOC_AARCH64_MOVW_G2_NC,
   0,
   0,
   0},

  /* Most significant bits 48-63 of signed/unsigned address/value: MOVZ */
  {"abs_g3", 0,
   0,				/* adr_type */
   0,
   BFD_RELOC_AARCH64_MOVW_G3,
   0,
   0,
   0},

  /* Most significant bits 0-15 of signed/unsigned address/value: MOVZ */
  {"prel_g0", 1,
   0,				/* adr_type */
   0,
   BFD_RELOC_AARCH64_MOVW_PREL_G0,
   0,
   0,
   0},

  /* Most significant bits 0-15 of signed/unsigned address/value: MOVK */
  {"prel_g0_nc", 1,
   0,				/* adr_type */
   0,
   BFD_RELOC_AARCH64_MOVW_PREL_G0_NC,
   0,
   0,
   0},

  /* Most significant bits 16-31 of signed/unsigned address/value: MOVZ */
  {"prel_g1", 1,
   0,				/* adr_type */
   0,
   BFD_RELOC_AARCH64_MOVW_PREL_G1,
   0,
   0,
   0},

  /* Most significant bits 16-31 of signed/unsigned address/value: MOVK */
  {"prel_g1_nc", 1,
   0,				/* adr_type */
   0,
   BFD_RELOC_AARCH64_MOVW_PREL_G1_NC,
   0,
   0,
   0},

  /* Most significant bits 32-47 of signed/unsigned address/value: MOVZ */
  {"prel_g2", 1,
   0,				/* adr_type */
   0,
   BFD_RELOC_AARCH64_MOVW_PREL_G2,
   0,
   0,
   0},

  /* Most significant bits 32-47 of signed/unsigned address/value: MOVK */
  {"prel_g2_nc", 1,
   0,				/* adr_type */
   0,
   BFD_RELOC_AARCH64_MOVW_PREL_G2_NC,
   0,
   0,
   0},

  /* Most significant bits 48-63 of signed/unsigned address/value: MOVZ */
  {"prel_g3", 1,
   0,				/* adr_type */
   0,
   BFD_RELOC_AARCH64_MOVW_PREL_G3,
   0,
   0,
   0},

  /* Get to the page containing GOT entry for a symbol.  */
  {"got", 1,
   0,				/* adr_type */
   BFD_RELOC_AARCH64_ADR_GOT_PAGE,
   0,
   0,
   0,
   BFD_RELOC_AARCH64_GOT_LD_PREL19},

  /* 12 bit offset into the page containing GOT entry for that symbol.  */
  {"got_lo12", 0,
   0,				/* adr_type */
   0,
   0,
   0,
   BFD_RELOC_AARCH64_LD_GOT_LO12_NC,
   0},

  /* 0-15 bits of address/value: MOVk, no check.  */
  {"gotoff_g0_nc", 0,
   0,				/* adr_type */
   0,
   BFD_RELOC_AARCH64_MOVW_GOTOFF_G0_NC,
   0,
   0,
   0},

  /* Most significant bits 16-31 of address/value: MOVZ.  */
  {"gotoff_g1", 0,
   0,				/* adr_type */
   0,
   BFD_RELOC_AARCH64_MOVW_GOTOFF_G1,
   0,
   0,
   0},

  /* 15 bit offset into the page containing GOT entry for that symbol.  */
  {"gotoff_lo15", 0,
   0,				/* adr_type */
   0,
   0,
   0,
   BFD_RELOC_AARCH64_LD64_GOTOFF_LO15,
   0},

  /* Get to the page containing GOT TLS entry for a symbol */
  {"gottprel_g0_nc", 0,
   0,				/* adr_type */
   0,
   BFD_RELOC_AARCH64_TLSIE_MOVW_GOTTPREL_G0_NC,
   0,
   0,
   0},

  /* Get to the page containing GOT TLS entry for a symbol */
  {"gottprel_g1", 0,
   0,				/* adr_type */
   0,
   BFD_RELOC_AARCH64_TLSIE_MOVW_GOTTPREL_G1,
   0,
   0,
   0},

  /* Get to the page containing GOT TLS entry for a symbol */
  {"tlsgd", 0,
   BFD_RELOC_AARCH64_TLSGD_ADR_PREL21, /* adr_type */
   BFD_RELOC_AARCH64_TLSGD_ADR_PAGE21,
   0,
   0,
   0,
   0},

  /* 12 bit offset into the page containing GOT TLS entry for a symbol */
  {"tlsgd_lo12", 0,
   0,				/* adr_type */
   0,
   0,
   BFD_RELOC_AARCH64_TLSGD_ADD_LO12_NC,
   0,
   0},

  /* Lower 16 bits address/value: MOVk.  */
  {"tlsgd_g0_nc", 0,
   0,				/* adr_type */
   0,
   BFD_RELOC_AARCH64_TLSGD_MOVW_G0_NC,
   0,
   0,
   0},

  /* Most significant bits 16-31 of address/value: MOVZ.  */
  {"tlsgd_g1", 0,
   0,				/* adr_type */
   0,
   BFD_RELOC_AARCH64_TLSGD_MOVW_G1,
   0,
   0,
   0},

  /* Get to the page containing GOT TLS entry for a symbol */
  {"tlsdesc", 0,
   BFD_RELOC_AARCH64_TLSDESC_ADR_PREL21, /* adr_type */
   BFD_RELOC_AARCH64_TLSDESC_ADR_PAGE21,
   0,
   0,
   0,
   BFD_RELOC_AARCH64_TLSDESC_LD_PREL19},

  /* 12 bit offset into the page containing GOT TLS entry for a symbol */
  {"tlsdesc_lo12", 0,
   0,				/* adr_type */
   0,
   0,
   BFD_RELOC_AARCH64_TLSDESC_ADD_LO12,
   BFD_RELOC_AARCH64_TLSDESC_LD_LO12_NC,
   0},

  /* Get to the page containing GOT TLS entry for a symbol.
     The same as GD, we allocate two consecutive GOT slots
     for module index and module offset, the only difference
     with GD is the module offset should be initialized to
     zero without any outstanding runtime relocation. */
  {"tlsldm", 0,
   BFD_RELOC_AARCH64_TLSLD_ADR_PREL21, /* adr_type */
   BFD_RELOC_AARCH64_TLSLD_ADR_PAGE21,
   0,
   0,
   0,
   0},

  /* 12 bit offset into the page containing GOT TLS entry for a symbol */
  {"tlsldm_lo12_nc", 0,
   0,				/* adr_type */
   0,
   0,
   BFD_RELOC_AARCH64_TLSLD_ADD_LO12_NC,
   0,
   0},

  /* 12 bit offset into the module TLS base address.  */
  {"dtprel_lo12", 0,
   0,				/* adr_type */
   0,
   0,
   BFD_RELOC_AARCH64_TLSLD_ADD_DTPREL_LO12,
   BFD_RELOC_AARCH64_TLSLD_LDST_DTPREL_LO12,
   0},

  /* Same as dtprel_lo12, no overflow check.  */
  {"dtprel_lo12_nc", 0,
   0,				/* adr_type */
   0,
   0,
   BFD_RELOC_AARCH64_TLSLD_ADD_DTPREL_LO12_NC,
   BFD_RELOC_AARCH64_TLSLD_LDST_DTPREL_LO12_NC,
   0},

  /* bits[23:12] of offset to the module TLS base address.  */
  {"dtprel_hi12", 0,
   0,				/* adr_type */
   0,
   0,
   BFD_RELOC_AARCH64_TLSLD_ADD_DTPREL_HI12,
   0,
   0},

  /* bits[15:0] of offset to the module TLS base address.  */
  {"dtprel_g0", 0,
   0,				/* adr_type */
   0,
   BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G0,
   0,
   0,
   0},

  /* No overflow check version of BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G0.  */
  {"dtprel_g0_nc", 0,
   0,				/* adr_type */
   0,
   BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G0_NC,
   0,
   0,
   0},

  /* bits[31:16] of offset to the module TLS base address.  */
  {"dtprel_g1", 0,
   0,				/* adr_type */
   0,
   BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G1,
   0,
   0,
   0},

  /* No overflow check version of BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G1.  */
  {"dtprel_g1_nc", 0,
   0,				/* adr_type */
   0,
   BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G1_NC,
   0,
   0,
   0},

  /* bits[47:32] of offset to the module TLS base address.  */
  {"dtprel_g2", 0,
   0,				/* adr_type */
   0,
   BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G2,
   0,
   0,
   0},

  /* Lower 16 bit offset into GOT entry for a symbol */
  {"tlsdesc_off_g0_nc", 0,
   0,				/* adr_type */
   0,
   BFD_RELOC_AARCH64_TLSDESC_OFF_G0_NC,
   0,
   0,
   0},

  /* Higher 16 bit offset into GOT entry for a symbol */
  {"tlsdesc_off_g1", 0,
   0,				/* adr_type */
   0,
   BFD_RELOC_AARCH64_TLSDESC_OFF_G1,
   0,
   0,
   0},

  /* Get to the page containing GOT TLS entry for a symbol */
  {"gottprel", 0,
   0,				/* adr_type */
   BFD_RELOC_AARCH64_TLSIE_ADR_GOTTPREL_PAGE21,
   0,
   0,
   0,
   BFD_RELOC_AARCH64_TLSIE_LD_GOTTPREL_PREL19},

  /* 12 bit offset into the page containing GOT TLS entry for a symbol */
  {"gottprel_lo12", 0,
   0,				/* adr_type */
   0,
   0,
   0,
   BFD_RELOC_AARCH64_TLSIE_LD_GOTTPREL_LO12_NC,
   0},

  /* Get tp offset for a symbol.  */
  {"tprel", 0,
   0,				/* adr_type */
   0,
   0,
   BFD_RELOC_AARCH64_TLSLE_ADD_TPREL_LO12,
   0,
   0},

  /* Get tp offset for a symbol.  */
  {"tprel_lo12", 0,
   0,				/* adr_type */
   0,
   0,
   BFD_RELOC_AARCH64_TLSLE_ADD_TPREL_LO12,
   BFD_RELOC_AARCH64_TLSLE_LDST_TPREL_LO12,
   0},

  /* Get tp offset for a symbol.  */
  {"tprel_hi12", 0,
   0,				/* adr_type */
   0,
   0,
   BFD_RELOC_AARCH64_TLSLE_ADD_TPREL_HI12,
   0,
   0},

  /* Get tp offset for a symbol.  */
  {"tprel_lo12_nc", 0,
   0,				/* adr_type */
   0,
   0,
   BFD_RELOC_AARCH64_TLSLE_ADD_TPREL_LO12_NC,
   BFD_RELOC_AARCH64_TLSLE_LDST_TPREL_LO12_NC,
   0},

  /* Most significant bits 32-47 of address/value: MOVZ.  */
  {"tprel_g2", 0,
   0,				/* adr_type */
   0,
   BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G2,
   0,
   0,
   0},

  /* Most significant bits 16-31 of address/value: MOVZ.  */
  {"tprel_g1", 0,
   0,				/* adr_type */
   0,
   BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G1,
   0,
   0,
   0},

  /* Most significant bits 16-31 of address/value: MOVZ, no check.  */
  {"tprel_g1_nc", 0,
   0,				/* adr_type */
   0,
   BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G1_NC,
   0,
   0,
   0},

  /* Most significant bits 0-15 of address/value: MOVZ.  */
  {"tprel_g0", 0,
   0,				/* adr_type */
   0,
   BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G0,
   0,
   0,
   0},

  /* Most significant bits 0-15 of address/value: MOVZ, no check.  */
  {"tprel_g0_nc", 0,
   0,				/* adr_type */
   0,
   BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G0_NC,
   0,
   0,
   0},

  /* 15bit offset from got entry to base address of GOT table.  */
  {"gotpage_lo15", 0,
   0,
   0,
   0,
   0,
   BFD_RELOC_AARCH64_LD64_GOTPAGE_LO15,
   0},

  /* 14bit offset from got entry to base address of GOT table.  */
  {"gotpage_lo14", 0,
   0,
   0,
   0,
   0,
   BFD_RELOC_AARCH64_LD32_GOTPAGE_LO14,
   0},
};

/* Given the address of a pointer pointing to the textual name of a
   relocation as may appear in assembler source, attempt to find its
   details in reloc_table.  The pointer will be updated to the character
   after the trailing colon.  On failure, NULL will be returned;
   otherwise return the reloc_table_entry.  */

static struct reloc_table_entry *
find_reloc_table_entry (char **str)
{
  unsigned int i;
  for (i = 0; i < ARRAY_SIZE (reloc_table); i++)
    {
      int length = strlen (reloc_table[i].name);

      if (strncasecmp (reloc_table[i].name, *str, length) == 0
	  && (*str)[length] == ':')
	{
	  *str += (length + 1);
	  return &reloc_table[i];
	}
    }

  return NULL;
}

/* Returns 0 if the relocation should never be forced,
   1 if the relocation must be forced, and -1 if either
   result is OK.  */

static signed int
aarch64_force_reloc (unsigned int type)
{
  switch (type)
    {
    case BFD_RELOC_AARCH64_GAS_INTERNAL_FIXUP:
      /* Perform these "immediate" internal relocations
         even if the symbol is extern or weak.  */
      return 0;

    case BFD_RELOC_AARCH64_LD_GOT_LO12_NC:
    case BFD_RELOC_AARCH64_TLSDESC_LD_LO12_NC:
    case BFD_RELOC_AARCH64_TLSIE_LD_GOTTPREL_LO12_NC:
      /* Pseudo relocs that need to be fixed up according to
	 ilp32_p.  */
      return 1;

    case BFD_RELOC_AARCH64_ADD_LO12:
    case BFD_RELOC_AARCH64_ADR_GOT_PAGE:
    case BFD_RELOC_AARCH64_ADR_HI21_NC_PCREL:
    case BFD_RELOC_AARCH64_ADR_HI21_PCREL:
    case BFD_RELOC_AARCH64_GOT_LD_PREL19:
    case BFD_RELOC_AARCH64_LD32_GOT_LO12_NC:
    case BFD_RELOC_AARCH64_LD32_GOTPAGE_LO14:
    case BFD_RELOC_AARCH64_LD64_GOTOFF_LO15:
    case BFD_RELOC_AARCH64_LD64_GOTPAGE_LO15:
    case BFD_RELOC_AARCH64_LD64_GOT_LO12_NC:
    case BFD_RELOC_AARCH64_LDST128_LO12:
    case BFD_RELOC_AARCH64_LDST16_LO12:
    case BFD_RELOC_AARCH64_LDST32_LO12:
    case BFD_RELOC_AARCH64_LDST64_LO12:
    case BFD_RELOC_AARCH64_LDST8_LO12:
    case BFD_RELOC_AARCH64_LDST_LO12:
    case BFD_RELOC_AARCH64_TLSDESC_ADD_LO12:
    case BFD_RELOC_AARCH64_TLSDESC_ADR_PAGE21:
    case BFD_RELOC_AARCH64_TLSDESC_ADR_PREL21:
    case BFD_RELOC_AARCH64_TLSDESC_LD32_LO12_NC:
    case BFD_RELOC_AARCH64_TLSDESC_LD64_LO12:
    case BFD_RELOC_AARCH64_TLSDESC_LD_PREL19:
    case BFD_RELOC_AARCH64_TLSDESC_OFF_G0_NC:
    case BFD_RELOC_AARCH64_TLSDESC_OFF_G1:
    case BFD_RELOC_AARCH64_TLSGD_ADD_LO12_NC:
    case BFD_RELOC_AARCH64_TLSGD_ADR_PAGE21:
    case BFD_RELOC_AARCH64_TLSGD_ADR_PREL21:
    case BFD_RELOC_AARCH64_TLSGD_MOVW_G0_NC:
    case BFD_RELOC_AARCH64_TLSGD_MOVW_G1:
    case BFD_RELOC_AARCH64_TLSIE_ADR_GOTTPREL_PAGE21:
    case BFD_RELOC_AARCH64_TLSIE_LD32_GOTTPREL_LO12_NC:
    case BFD_RELOC_AARCH64_TLSIE_LD64_GOTTPREL_LO12_NC:
    case BFD_RELOC_AARCH64_TLSIE_LD_GOTTPREL_PREL19:
    case BFD_RELOC_AARCH64_TLSIE_MOVW_GOTTPREL_G0_NC:
    case BFD_RELOC_AARCH64_TLSIE_MOVW_GOTTPREL_G1:
    case BFD_RELOC_AARCH64_TLSLD_ADD_DTPREL_HI12:
    case BFD_RELOC_AARCH64_TLSLD_ADD_DTPREL_LO12:
    case BFD_RELOC_AARCH64_TLSLD_ADD_DTPREL_LO12_NC:
    case BFD_RELOC_AARCH64_TLSLD_ADD_LO12_NC:
    case BFD_RELOC_AARCH64_TLSLD_ADR_PAGE21:
    case BFD_RELOC_AARCH64_TLSLD_ADR_PREL21:
    case BFD_RELOC_AARCH64_TLSLD_LDST16_DTPREL_LO12:
    case BFD_RELOC_AARCH64_TLSLD_LDST16_DTPREL_LO12_NC:
    case BFD_RELOC_AARCH64_TLSLD_LDST32_DTPREL_LO12:
    case BFD_RELOC_AARCH64_TLSLD_LDST32_DTPREL_LO12_NC:
    case BFD_RELOC_AARCH64_TLSLD_LDST64_DTPREL_LO12:
    case BFD_RELOC_AARCH64_TLSLD_LDST64_DTPREL_LO12_NC:
    case BFD_RELOC_AARCH64_TLSLD_LDST8_DTPREL_LO12:
    case BFD_RELOC_AARCH64_TLSLD_LDST8_DTPREL_LO12_NC:
    case BFD_RELOC_AARCH64_TLSLD_LDST_DTPREL_LO12:
    case BFD_RELOC_AARCH64_TLSLD_LDST_DTPREL_LO12_NC:
    case BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G0:
    case BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G0_NC:
    case BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G1:
    case BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G1_NC:
    case BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G2:
    case BFD_RELOC_AARCH64_TLSLE_LDST16_TPREL_LO12:
    case BFD_RELOC_AARCH64_TLSLE_LDST16_TPREL_LO12_NC:
    case BFD_RELOC_AARCH64_TLSLE_LDST32_TPREL_LO12:
    case BFD_RELOC_AARCH64_TLSLE_LDST32_TPREL_LO12_NC:
    case BFD_RELOC_AARCH64_TLSLE_LDST64_TPREL_LO12:
    case BFD_RELOC_AARCH64_TLSLE_LDST64_TPREL_LO12_NC:
    case BFD_RELOC_AARCH64_TLSLE_LDST8_TPREL_LO12:
    case BFD_RELOC_AARCH64_TLSLE_LDST8_TPREL_LO12_NC:
    case BFD_RELOC_AARCH64_TLSLE_LDST_TPREL_LO12:
    case BFD_RELOC_AARCH64_TLSLE_LDST_TPREL_LO12_NC:
    case BFD_RELOC_AARCH64_TLSLE_ADD_TPREL_HI12:
    case BFD_RELOC_AARCH64_TLSLE_ADD_TPREL_LO12:
    case BFD_RELOC_AARCH64_TLSLE_ADD_TPREL_LO12_NC:
    case BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G0:
    case BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G0_NC:
    case BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G1:
    case BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G1_NC:
    case BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G2:
      /* Always leave these relocations for the linker.  */
      return 1;

    default:
      return -1;
    }
}

int
aarch64_force_relocation (struct fix *fixp)
{
  int res = aarch64_force_reloc (fixp->fx_r_type);

  if (res == -1)
    return generic_force_reloc (fixp);
  return res;
}

/* Mode argument to parse_shift and parser_shifter_operand.  */
enum parse_shift_mode
{
  SHIFTED_NONE,			/* no shifter allowed  */
  SHIFTED_ARITH_IMM,		/* "rn{,lsl|lsr|asl|asr|uxt|sxt #n}" or
				   "#imm{,lsl #n}"  */
  SHIFTED_LOGIC_IMM,		/* "rn{,lsl|lsr|asl|asr|ror #n}" or
				   "#imm"  */
  SHIFTED_LSL,			/* bare "lsl #n"  */
  SHIFTED_MUL,			/* bare "mul #n"  */
  SHIFTED_LSL_MSL,		/* "lsl|msl #n"  */
  SHIFTED_MUL_VL,		/* "mul vl"  */
  SHIFTED_REG_OFFSET		/* [su]xtw|sxtx {#n} or lsl #n  */
};

/* Parse a <shift> operator on an AArch64 data processing instruction.
   Return TRUE on success; otherwise return FALSE.  */
static bool
parse_shift (char **str, aarch64_opnd_info *operand, enum parse_shift_mode mode)
{
  const struct aarch64_name_value_pair *shift_op;
  enum aarch64_modifier_kind kind;
  expressionS exp;
  int exp_has_prefix;
  char *s = *str;
  char *p = s;

  for (p = *str; ISALPHA (*p); p++)
    ;

  if (p == *str)
    {
      set_syntax_error (_("shift expression expected"));
      return false;
    }

  shift_op = str_hash_find_n (aarch64_shift_hsh, *str, p - *str);

  if (shift_op == NULL)
    {
      set_syntax_error (_("shift operator expected"));
      return false;
    }

  kind = aarch64_get_operand_modifier (shift_op);

  if (kind == AARCH64_MOD_MSL && mode != SHIFTED_LSL_MSL)
    {
      set_syntax_error (_("invalid use of 'MSL'"));
      return false;
    }

  if (kind == AARCH64_MOD_MUL
      && mode != SHIFTED_MUL
      && mode != SHIFTED_MUL_VL)
    {
      set_syntax_error (_("invalid use of 'MUL'"));
      return false;
    }

  switch (mode)
    {
    case SHIFTED_LOGIC_IMM:
      if (aarch64_extend_operator_p (kind))
	{
	  set_syntax_error (_("extending shift is not permitted"));
	  return false;
	}
      break;

    case SHIFTED_ARITH_IMM:
      if (kind == AARCH64_MOD_ROR)
	{
	  set_syntax_error (_("'ROR' shift is not permitted"));
	  return false;
	}
      break;

    case SHIFTED_LSL:
      if (kind != AARCH64_MOD_LSL)
	{
	  set_syntax_error (_("only 'LSL' shift is permitted"));
	  return false;
	}
      break;

    case SHIFTED_MUL:
      if (kind != AARCH64_MOD_MUL)
	{
	  set_syntax_error (_("only 'MUL' is permitted"));
	  return false;
	}
      break;

    case SHIFTED_MUL_VL:
      /* "MUL VL" consists of two separate tokens.  Require the first
	 token to be "MUL" and look for a following "VL".  */
      if (kind == AARCH64_MOD_MUL)
	{
	  skip_whitespace (p);
	  if (strncasecmp (p, "vl", 2) == 0 && !ISALPHA (p[2]))
	    {
	      p += 2;
	      kind = AARCH64_MOD_MUL_VL;
	      break;
	    }
	}
      set_syntax_error (_("only 'MUL VL' is permitted"));
      return false;

    case SHIFTED_REG_OFFSET:
      if (kind != AARCH64_MOD_UXTW && kind != AARCH64_MOD_LSL
	  && kind != AARCH64_MOD_SXTW && kind != AARCH64_MOD_SXTX)
	{
	  set_fatal_syntax_error
	    (_("invalid shift for the register offset addressing mode"));
	  return false;
	}
      break;

    case SHIFTED_LSL_MSL:
      if (kind != AARCH64_MOD_LSL && kind != AARCH64_MOD_MSL)
	{
	  set_syntax_error (_("invalid shift operator"));
	  return false;
	}
      break;

    default:
      abort ();
    }

  /* Whitespace can appear here if the next thing is a bare digit.  */
  skip_whitespace (p);

  /* Parse shift amount.  */
  exp_has_prefix = 0;
  if ((mode == SHIFTED_REG_OFFSET && *p == ']') || kind == AARCH64_MOD_MUL_VL)
    exp.X_op = O_absent;
  else
    {
      if (is_immediate_prefix (*p))
	{
	  p++;
	  exp_has_prefix = 1;
	}
      aarch64_get_expression (&exp, &p, GE_NO_PREFIX, ALLOW_ABSENT);
    }
  if (kind == AARCH64_MOD_MUL_VL)
    /* For consistency, give MUL VL the same shift amount as an implicit
       MUL #1.  */
    operand->shifter.amount = 1;
  else if (exp.X_op == O_absent)
    {
      if (!aarch64_extend_operator_p (kind) || exp_has_prefix)
	{
	  set_syntax_error (_("missing shift amount"));
	  return false;
	}
      operand->shifter.amount = 0;
    }
  else if (exp.X_op != O_constant)
    {
      set_syntax_error (_("constant shift amount required"));
      return false;
    }
  /* For parsing purposes, MUL #n has no inherent range.  The range
     depends on the operand and will be checked by operand-specific
     routines.  */
  else if (kind != AARCH64_MOD_MUL
	   && (exp.X_add_number < 0 || exp.X_add_number > 63))
    {
      set_fatal_syntax_error (_("shift amount out of range 0 to 63"));
      return false;
    }
  else
    {
      operand->shifter.amount = exp.X_add_number;
      operand->shifter.amount_present = 1;
    }

  operand->shifter.operator_present = 1;
  operand->shifter.kind = kind;

  *str = p;
  return true;
}

/* Parse a <shifter_operand> for a data processing instruction:

      #<immediate>
      #<immediate>, LSL #imm

   Validation of immediate operands is deferred to md_apply_fix.

   Return TRUE on success; otherwise return FALSE.  */

static bool
parse_shifter_operand_imm (char **str, aarch64_opnd_info *operand,
			   enum parse_shift_mode mode)
{
  char *p;

  if (mode != SHIFTED_ARITH_IMM && mode != SHIFTED_LOGIC_IMM)
    return false;

  p = *str;

  /* Accept an immediate expression.  */
  if (! aarch64_get_expression (&inst.reloc.exp, &p, GE_OPT_PREFIX,
				REJECT_ABSENT))
    return false;

  /* Accept optional LSL for arithmetic immediate values.  */
  if (mode == SHIFTED_ARITH_IMM && skip_past_comma (&p))
    if (! parse_shift (&p, operand, SHIFTED_LSL))
      return false;

  /* Not accept any shifter for logical immediate values.  */
  if (mode == SHIFTED_LOGIC_IMM && skip_past_comma (&p)
      && parse_shift (&p, operand, mode))
    {
      set_syntax_error (_("unexpected shift operator"));
      return false;
    }

  *str = p;
  return true;
}

/* Parse a <shifter_operand> for a data processing instruction:

      <Rm>
      <Rm>, <shift>
      #<immediate>
      #<immediate>, LSL #imm

   where <shift> is handled by parse_shift above, and the last two
   cases are handled by the function above.

   Validation of immediate operands is deferred to md_apply_fix.

   Return TRUE on success; otherwise return FALSE.  */

static bool
parse_shifter_operand (char **str, aarch64_opnd_info *operand,
		       enum parse_shift_mode mode)
{
  const reg_entry *reg;
  aarch64_opnd_qualifier_t qualifier;
  enum aarch64_operand_class opd_class
    = aarch64_get_operand_class (operand->type);

  reg = aarch64_reg_parse_32_64 (str, &qualifier);
  if (reg)
    {
      if (opd_class == AARCH64_OPND_CLASS_IMMEDIATE)
	{
	  set_syntax_error (_("unexpected register in the immediate operand"));
	  return false;
	}

      if (!aarch64_check_reg_type (reg, REG_TYPE_R_ZR))
	{
	  set_expected_reg_error (REG_TYPE_R_ZR, reg, 0);
	  return false;
	}

      operand->reg.regno = reg->number;
      operand->qualifier = qualifier;

      /* Accept optional shift operation on register.  */
      if (! skip_past_comma (str))
	return true;

      if (! parse_shift (str, operand, mode))
	return false;

      return true;
    }
  else if (opd_class == AARCH64_OPND_CLASS_MODIFIED_REG)
    {
      set_syntax_error
	(_("integer register expected in the extended/shifted operand "
	   "register"));
      return false;
    }

  /* We have a shifted immediate variable.  */
  return parse_shifter_operand_imm (str, operand, mode);
}

/* Return TRUE on success; return FALSE otherwise.  */

static bool
parse_shifter_operand_reloc (char **str, aarch64_opnd_info *operand,
			     enum parse_shift_mode mode)
{
  char *p = *str;

  /* Determine if we have the sequence of characters #: or just :
     coming next.  If we do, then we check for a :rello: relocation
     modifier.  If we don't, punt the whole lot to
     parse_shifter_operand.  */

  if ((p[0] == '#' && p[1] == ':') || p[0] == ':')
    {
      struct reloc_table_entry *entry;

      if (p[0] == '#')
	p += 2;
      else
	p++;
      *str = p;

      /* Try to parse a relocation.  Anything else is an error.  */
      if (!(entry = find_reloc_table_entry (str)))
	{
	  set_syntax_error (_("unknown relocation modifier"));
	  return false;
	}

      if (entry->add_type == 0)
	{
	  set_syntax_error
	    (_("this relocation modifier is not allowed on this instruction"));
	  return false;
	}

      /* Save str before we decompose it.  */
      p = *str;

      /* Next, we parse the expression.  */
      if (! aarch64_get_expression (&inst.reloc.exp, str, GE_NO_PREFIX,
				    REJECT_ABSENT))
	return false;
      
      /* Record the relocation type (use the ADD variant here).  */
      inst.reloc.type = entry->add_type;
      inst.reloc.pc_rel = entry->pc_rel;

      /* If str is empty, we've reached the end, stop here.  */
      if (**str == '\0')
	return true;

      /* Otherwise, we have a shifted reloc modifier, so rewind to
         recover the variable name and continue parsing for the shifter.  */
      *str = p;
      return parse_shifter_operand_imm (str, operand, mode);
    }

  return parse_shifter_operand (str, operand, mode);
}

/* Parse all forms of an address expression.  Information is written
   to *OPERAND and/or inst.reloc.

   The A64 instruction set has the following addressing modes:

   Offset
     [base]			 // in SIMD ld/st structure
     [base{,#0}]		 // in ld/st exclusive
     [base{,#imm}]
     [base,Xm{,LSL #imm}]
     [base,Xm,SXTX {#imm}]
     [base,Wm,(S|U)XTW {#imm}]
   Pre-indexed
     [base]!                    // in ldraa/ldrab exclusive
     [base,#imm]!
   Post-indexed
     [base],#imm
     [base],Xm			 // in SIMD ld/st structure
   PC-relative (literal)
     label
   SVE:
     [base,#imm,MUL VL]
     [base,Zm.D{,LSL #imm}]
     [base,Zm.S,(S|U)XTW {#imm}]
     [base,Zm.D,(S|U)XTW {#imm}] // ignores top 32 bits of Zm.D elements
     [Zn.S,#imm]
     [Zn.D,#imm]
     [Zn.S{, Xm}]
     [Zn.S,Zm.S{,LSL #imm}]      // in ADR
     [Zn.D,Zm.D{,LSL #imm}]      // in ADR
     [Zn.D,Zm.D,(S|U)XTW {#imm}] // in ADR

   (As a convenience, the notation "=immediate" is permitted in conjunction
   with the pc-relative literal load instructions to automatically place an
   immediate value or symbolic address in a nearby literal pool and generate
   a hidden label which references it.)

   Upon a successful parsing, the address structure in *OPERAND will be
   filled in the following way:

     .base_regno = <base>
     .offset.is_reg	// 1 if the offset is a register
     .offset.imm = <imm>
     .offset.regno = <Rm>

   For different addressing modes defined in the A64 ISA:

   Offset
     .pcrel=0; .preind=1; .postind=0; .writeback=0
   Pre-indexed
     .pcrel=0; .preind=1; .postind=0; .writeback=1
   Post-indexed
     .pcrel=0; .preind=0; .postind=1; .writeback=1
   PC-relative (literal)
     .pcrel=1; .preind=1; .postind=0; .writeback=0

   The shift/extension information, if any, will be stored in .shifter.
   The base and offset qualifiers will be stored in *BASE_QUALIFIER and
   *OFFSET_QUALIFIER respectively, with NIL being used if there's no
   corresponding register.

   BASE_TYPE says which types of base register should be accepted and
   OFFSET_TYPE says the same for offset registers.  IMM_SHIFT_MODE
   is the type of shifter that is allowed for immediate offsets,
   or SHIFTED_NONE if none.

   In all other respects, it is the caller's responsibility to check
   for addressing modes not supported by the instruction, and to set
   inst.reloc.type.  */

static bool
parse_address_main (char **str, aarch64_opnd_info *operand,
		    aarch64_opnd_qualifier_t *base_qualifier,
		    aarch64_opnd_qualifier_t *offset_qualifier,
		    aarch64_reg_type base_type, aarch64_reg_type offset_type,
		    enum parse_shift_mode imm_shift_mode)
{
  char *p = *str;
  const reg_entry *reg;
  expressionS *exp = &inst.reloc.exp;

  *base_qualifier = AARCH64_OPND_QLF_NIL;
  *offset_qualifier = AARCH64_OPND_QLF_NIL;
  if (! skip_past_char (&p, '['))
    {
      /* =immediate or label.  */
      operand->addr.pcrel = 1;
      operand->addr.preind = 1;

      /* #:<reloc_op>:<symbol>  */
      skip_past_char (&p, '#');
      if (skip_past_char (&p, ':'))
	{
	  bfd_reloc_code_real_type ty;
	  struct reloc_table_entry *entry;

	  /* Try to parse a relocation modifier.  Anything else is
	     an error.  */
	  entry = find_reloc_table_entry (&p);
	  if (! entry)
	    {
	      set_syntax_error (_("unknown relocation modifier"));
	      return false;
	    }

	  switch (operand->type)
	    {
	    case AARCH64_OPND_ADDR_PCREL21:
	      /* adr */
	      ty = entry->adr_type;
	      break;

	    default:
	      ty = entry->ld_literal_type;
	      break;
	    }

	  if (ty == 0)
	    {
	      set_syntax_error
		(_("this relocation modifier is not allowed on this "
		   "instruction"));
	      return false;
	    }

	  /* #:<reloc_op>:  */
	  if (! aarch64_get_expression (exp, &p, GE_NO_PREFIX, REJECT_ABSENT))
	    {
	      set_syntax_error (_("invalid relocation expression"));
	      return false;
	    }
	  /* #:<reloc_op>:<expr>  */
	  /* Record the relocation type.  */
	  inst.reloc.type = ty;
	  inst.reloc.pc_rel = entry->pc_rel;
	}
      else
	{
	  if (skip_past_char (&p, '='))
	    /* =immediate; need to generate the literal in the literal pool. */
	    inst.gen_lit_pool = 1;

	  if (!aarch64_get_expression (exp, &p, GE_NO_PREFIX, REJECT_ABSENT))
	    {
	      set_syntax_error (_("invalid address"));
	      return false;
	    }
	}

      *str = p;
      return true;
    }

  /* [ */

  bool alpha_base_p = ISALPHA (*p);
  reg = aarch64_addr_reg_parse (&p, base_type, base_qualifier);
  if (!reg || !aarch64_check_reg_type (reg, base_type))
    {
      if (reg
	  && aarch64_check_reg_type (reg, REG_TYPE_R_SP)
	  && *base_qualifier == AARCH64_OPND_QLF_W)
	set_syntax_error (_("expected a 64-bit base register"));
      else if (alpha_base_p)
	set_syntax_error (_("invalid base register"));
      else
	set_syntax_error (_("expected a base register"));
      return false;
    }
  operand->addr.base_regno = reg->number;

  /* [Xn */
  if (skip_past_comma (&p))
    {
      /* [Xn, */
      operand->addr.preind = 1;

      reg = aarch64_addr_reg_parse (&p, offset_type, offset_qualifier);
      if (reg)
	{
	  if (!aarch64_check_reg_type (reg, offset_type))
	    {
	      set_syntax_error (_("invalid offset register"));
	      return false;
	    }

	  /* [Xn,Rm  */
	  operand->addr.offset.regno = reg->number;
	  operand->addr.offset.is_reg = 1;
	  /* Shifted index.  */
	  if (skip_past_comma (&p))
	    {
	      /* [Xn,Rm,  */
	      if (! parse_shift (&p, operand, SHIFTED_REG_OFFSET))
		/* Use the diagnostics set in parse_shift, so not set new
		   error message here.  */
		return false;
	    }
	  /* We only accept:
	     [base,Xm]  # For vector plus scalar SVE2 indexing.
	     [base,Xm{,LSL #imm}]
	     [base,Xm,SXTX {#imm}]
	     [base,Wm,(S|U)XTW {#imm}]  */
	  if (operand->shifter.kind == AARCH64_MOD_NONE
	      || operand->shifter.kind == AARCH64_MOD_LSL
	      || operand->shifter.kind == AARCH64_MOD_SXTX)
	    {
	      if (*offset_qualifier == AARCH64_OPND_QLF_W)
		{
		  set_syntax_error (_("invalid use of 32-bit register offset"));
		  return false;
		}
	      if (aarch64_get_qualifier_esize (*base_qualifier)
		  != aarch64_get_qualifier_esize (*offset_qualifier)
		  && (operand->type != AARCH64_OPND_SVE_ADDR_ZX
		      || *base_qualifier != AARCH64_OPND_QLF_S_S
		      || *offset_qualifier != AARCH64_OPND_QLF_X))
		{
		  set_syntax_error (_("offset has different size from base"));
		  return false;
		}
	    }
	  else if (*offset_qualifier == AARCH64_OPND_QLF_X)
	    {
	      set_syntax_error (_("invalid use of 64-bit register offset"));
	      return false;
	    }
	}
      else
	{
	  /* [Xn,#:<reloc_op>:<symbol>  */
	  skip_past_char (&p, '#');
	  if (skip_past_char (&p, ':'))
	    {
	      struct reloc_table_entry *entry;

	      /* Try to parse a relocation modifier.  Anything else is
		 an error.  */
	      if (!(entry = find_reloc_table_entry (&p)))
		{
		  set_syntax_error (_("unknown relocation modifier"));
		  return false;
		}

	      if (entry->ldst_type == 0)
		{
		  set_syntax_error
		    (_("this relocation modifier is not allowed on this "
		       "instruction"));
		  return false;
		}

	      /* [Xn,#:<reloc_op>:  */
	      /* We now have the group relocation table entry corresponding to
	         the name in the assembler source.  Next, we parse the
	         expression.  */
	      if (! aarch64_get_expression (exp, &p, GE_NO_PREFIX, REJECT_ABSENT))
		{
		  set_syntax_error (_("invalid relocation expression"));
		  return false;
		}

	      /* [Xn,#:<reloc_op>:<expr>  */
	      /* Record the load/store relocation type.  */
	      inst.reloc.type = entry->ldst_type;
	      inst.reloc.pc_rel = entry->pc_rel;
	    }
	  else
	    {
	      if (! aarch64_get_expression (exp, &p, GE_OPT_PREFIX, REJECT_ABSENT))
		{
		  set_syntax_error (_("invalid expression in the address"));
		  return false;
		}
	      /* [Xn,<expr>  */
	      if (imm_shift_mode != SHIFTED_NONE && skip_past_comma (&p))
		/* [Xn,<expr>,<shifter>  */
		if (! parse_shift (&p, operand, imm_shift_mode))
		  return false;
	    }
	}
    }

  if (! skip_past_char (&p, ']'))
    {
      set_syntax_error (_("']' expected"));
      return false;
    }

  if (skip_past_char (&p, '!'))
    {
      if (operand->addr.preind && operand->addr.offset.is_reg)
	{
	  set_syntax_error (_("register offset not allowed in pre-indexed "
			      "addressing mode"));
	  return false;
	}
      /* [Xn]! */
      operand->addr.writeback = 1;
    }
  else if (skip_past_comma (&p))
    {
      /* [Xn], */
      operand->addr.postind = 1;
      operand->addr.writeback = 1;

      if (operand->addr.preind)
	{
	  set_syntax_error (_("cannot combine pre- and post-indexing"));
	  return false;
	}

      reg = aarch64_reg_parse_32_64 (&p, offset_qualifier);
      if (reg)
	{
	  /* [Xn],Xm */
	  if (!aarch64_check_reg_type (reg, REG_TYPE_R_64))
	    {
	      set_syntax_error (_("invalid offset register"));
	      return false;
	    }

	  operand->addr.offset.regno = reg->number;
	  operand->addr.offset.is_reg = 1;
	}
      else if (! aarch64_get_expression (exp, &p, GE_OPT_PREFIX, REJECT_ABSENT))
	{
	  /* [Xn],#expr */
	  set_syntax_error (_("invalid expression in the address"));
	  return false;
	}
    }

  /* If at this point neither .preind nor .postind is set, we have a
     bare [Rn]{!}; only accept [Rn]! as a shorthand for [Rn,#0]! for ldraa and
     ldrab, accept [Rn] as a shorthand for [Rn,#0].
     For SVE2 vector plus scalar offsets, allow [Zn.<T>] as shorthand for
     [Zn.<T>, xzr].  */
  if (operand->addr.preind == 0 && operand->addr.postind == 0)
    {
      if (operand->addr.writeback)
	{
	  if (operand->type == AARCH64_OPND_ADDR_SIMM10)
            {
              /* Accept [Rn]! as a shorthand for [Rn,#0]!   */
              operand->addr.offset.is_reg = 0;
              operand->addr.offset.imm = 0;
              operand->addr.preind = 1;
            }
          else
           {
	     /* Reject [Rn]!   */
	     set_syntax_error (_("missing offset in the pre-indexed address"));
	     return false;
	   }
	}
       else
	{
          operand->addr.preind = 1;
          if (operand->type == AARCH64_OPND_SVE_ADDR_ZX)
	   {
	     operand->addr.offset.is_reg = 1;
	     operand->addr.offset.regno = REG_ZR;
	     *offset_qualifier = AARCH64_OPND_QLF_X;
 	   }
          else
	   {
	     inst.reloc.exp.X_op = O_constant;
	     inst.reloc.exp.X_add_number = 0;
	   }
	}
    }

  *str = p;
  return true;
}

/* Parse a base AArch64 address (as opposed to an SVE one).  Return TRUE
   on success.  */
static bool
parse_address (char **str, aarch64_opnd_info *operand)
{
  aarch64_opnd_qualifier_t base_qualifier, offset_qualifier;
  return parse_address_main (str, operand, &base_qualifier, &offset_qualifier,
			     REG_TYPE_R64_SP, REG_TYPE_R_ZR, SHIFTED_NONE);
}

/* Parse an address in which SVE vector registers and MUL VL are allowed.
   The arguments have the same meaning as for parse_address_main.
   Return TRUE on success.  */
static bool
parse_sve_address (char **str, aarch64_opnd_info *operand,
		   aarch64_opnd_qualifier_t *base_qualifier,
		   aarch64_opnd_qualifier_t *offset_qualifier)
{
  return parse_address_main (str, operand, base_qualifier, offset_qualifier,
			     REG_TYPE_SVE_BASE, REG_TYPE_SVE_OFFSET,
			     SHIFTED_MUL_VL);
}

/* Parse a register X0-X30.  The register must be 64-bit and register 31
   is unallocated.  */
static bool
parse_x0_to_x30 (char **str, aarch64_opnd_info *operand)
{
  const reg_entry *reg = parse_reg (str);
  if (!reg || !aarch64_check_reg_type (reg, REG_TYPE_R_64))
    {
      set_expected_reg_error (REG_TYPE_R_64, reg, 0);
      return false;
    }
  operand->reg.regno = reg->number;
  operand->qualifier = AARCH64_OPND_QLF_X;
  return true;
}

/* Parse an operand for a MOVZ, MOVN or MOVK instruction.
   Return TRUE on success; otherwise return FALSE.  */
static bool
parse_half (char **str, int *internal_fixup_p)
{
  char *p = *str;

  skip_past_char (&p, '#');

  gas_assert (internal_fixup_p);
  *internal_fixup_p = 0;

  if (*p == ':')
    {
      struct reloc_table_entry *entry;

      /* Try to parse a relocation.  Anything else is an error.  */
      ++p;

      if (!(entry = find_reloc_table_entry (&p)))
	{
	  set_syntax_error (_("unknown relocation modifier"));
	  return false;
	}

      if (entry->movw_type == 0)
	{
	  set_syntax_error
	    (_("this relocation modifier is not allowed on this instruction"));
	  return false;
	}

      inst.reloc.type = entry->movw_type;
    }
  else
    *internal_fixup_p = 1;

  if (! aarch64_get_expression (&inst.reloc.exp, &p, GE_NO_PREFIX, REJECT_ABSENT))
    return false;

  *str = p;
  return true;
}

/* Parse an operand for an ADRP instruction:
     ADRP <Xd>, <label>
   Return TRUE on success; otherwise return FALSE.  */

static bool
parse_adrp (char **str)
{
  char *p;

  p = *str;
  if (*p == ':')
    {
      struct reloc_table_entry *entry;

      /* Try to parse a relocation.  Anything else is an error.  */
      ++p;
      if (!(entry = find_reloc_table_entry (&p)))
	{
	  set_syntax_error (_("unknown relocation modifier"));
	  return false;
	}

      if (entry->adrp_type == 0)
	{
	  set_syntax_error
	    (_("this relocation modifier is not allowed on this instruction"));
	  return false;
	}

      inst.reloc.type = entry->adrp_type;
    }
  else
    inst.reloc.type = BFD_RELOC_AARCH64_ADR_HI21_PCREL;

  inst.reloc.pc_rel = 1;
  if (! aarch64_get_expression (&inst.reloc.exp, &p, GE_NO_PREFIX, REJECT_ABSENT))
    return false;
  *str = p;
  return true;
}

/* Miscellaneous. */

/* Parse a symbolic operand such as "pow2" at *STR.  ARRAY is an array
   of SIZE tokens in which index I gives the token for field value I,
   or is null if field value I is invalid.  If the symbolic operand
   can also be given as a 0-based integer, REG_TYPE says which register
   names should be treated as registers rather than as symbolic immediates
   while parsing that integer.  REG_TYPE is REG_TYPE_MAX otherwise.

   Return true on success, moving *STR past the operand and storing the
   field value in *VAL.  */

static int
parse_enum_string (char **str, int64_t *val, const char *const *array,
		   size_t size, aarch64_reg_type reg_type)
{
  expressionS exp;
  char *p, *q;
  size_t i;

  /* Match C-like tokens.  */
  p = q = *str;
  while (ISALNUM (*q))
    q++;

  for (i = 0; i < size; ++i)
    if (array[i]
	&& strncasecmp (array[i], p, q - p) == 0
	&& array[i][q - p] == 0)
      {
	*val = i;
	*str = q;
	return true;
      }

  if (reg_type == REG_TYPE_MAX)
    return false;

  if (!parse_immediate_expression (&p, &exp, reg_type))
    return false;

  if (exp.X_op == O_constant
      && (uint64_t) exp.X_add_number < size)
    {
      *val = exp.X_add_number;
      *str = p;
      return true;
    }

  /* Use the default error for this operand.  */
  return false;
}

/* Parse an option for a preload instruction.  Returns the encoding for the
   option, or PARSE_FAIL.  */

static int
parse_pldop (char **str)
{
  char *p, *q;
  const struct aarch64_name_value_pair *o;

  p = q = *str;
  while (ISALNUM (*q))
    q++;

  o = str_hash_find_n (aarch64_pldop_hsh, p, q - p);
  if (!o)
    return PARSE_FAIL;

  *str = q;
  return o->value;
}

/* Parse an option for a barrier instruction.  Returns the encoding for the
   option, or PARSE_FAIL.  */

static int
parse_barrier (char **str)
{
  char *p, *q;
  const struct aarch64_name_value_pair *o;

  p = q = *str;
  while (ISALPHA (*q))
    q++;

  o = str_hash_find_n (aarch64_barrier_opt_hsh, p, q - p);
  if (!o)
    return PARSE_FAIL;

  *str = q;
  return o->value;
}

/* Parse an operand for a PSB barrier.  Set *HINT_OPT to the hint-option record
   return 0 if successful.  Otherwise return PARSE_FAIL.  */

static int
parse_barrier_psb (char **str,
		   const struct aarch64_name_value_pair ** hint_opt)
{
  char *p, *q;
  const struct aarch64_name_value_pair *o;

  p = q = *str;
  while (ISALPHA (*q))
    q++;

  o = str_hash_find_n (aarch64_hint_opt_hsh, p, q - p);
  if (!o)
    {
      set_fatal_syntax_error
	( _("unknown or missing option to PSB/TSB"));
      return PARSE_FAIL;
    }

  if (o->value != 0x11)
    {
      /* PSB only accepts option name 'CSYNC'.  */
      set_syntax_error
	(_("the specified option is not accepted for PSB/TSB"));
      return PARSE_FAIL;
    }

  *str = q;
  *hint_opt = o;
  return 0;
}

/* Parse an operand for BTI.  Set *HINT_OPT to the hint-option record
   return 0 if successful.  Otherwise return PARSE_FAIL.  */

static int
parse_bti_operand (char **str,
		   const struct aarch64_name_value_pair ** hint_opt)
{
  char *p, *q;
  const struct aarch64_name_value_pair *o;

  p = q = *str;
  while (ISALPHA (*q))
    q++;

  o = str_hash_find_n (aarch64_hint_opt_hsh, p, q - p);
  if (!o)
    {
      set_fatal_syntax_error
	( _("unknown option to BTI"));
      return PARSE_FAIL;
    }

  switch (o->value)
    {
    /* Valid BTI operands.  */
    case HINT_OPD_C:
    case HINT_OPD_J:
    case HINT_OPD_JC:
      break;

    default:
      set_syntax_error
	(_("unknown option to BTI"));
      return PARSE_FAIL;
    }

  *str = q;
  *hint_opt = o;
  return 0;
}

/* Parse STR for reg of REG_TYPE and following '.' and QUALIFIER.
   Function returns REG_ENTRY struct and QUALIFIER [bhsdq] or NULL
   on failure. Format:

     REG_TYPE.QUALIFIER

   Side effect: Update STR with current parse position of success.

   FLAGS is as for parse_typed_reg.  */

static const reg_entry *
parse_reg_with_qual (char **str, aarch64_reg_type reg_type,
		     aarch64_opnd_qualifier_t *qualifier, unsigned int flags)
{
  struct vector_type_el vectype;
  const reg_entry *reg = parse_typed_reg (str, reg_type, &vectype,
					  PTR_FULL_REG | flags);
  if (!reg)
    return NULL;

  if (vectype.type == NT_invtype)
    *qualifier = AARCH64_OPND_QLF_NIL;
  else
    {
      *qualifier = vectype_to_qualifier (&vectype);
      if (*qualifier == AARCH64_OPND_QLF_NIL)
	return NULL;
    }

  return reg;
}

/* Parse STR for unsigned, immediate (1-2 digits) in format:

     #<imm>
     <imm>

  Function return TRUE if immediate was found, or FALSE.
*/
static bool
parse_sme_immediate (char **str, int64_t *imm)
{
  int64_t val;
  if (! parse_constant_immediate (str, &val, REG_TYPE_R_N))
    return false;

  *imm = val;
  return true;
}

/* Parse index with selection register and immediate offset:

   [<Wv>, <imm>]
   [<Wv>, #<imm>]

   Return true on success, populating OPND with the parsed index.  */

static bool
parse_sme_za_index (char **str, struct aarch64_indexed_za *opnd)
{
  const reg_entry *reg;

  if (!skip_past_char (str, '['))
    {
      set_syntax_error (_("expected '['"));
      return false;
    }

  /* The selection register, encoded in the 2-bit Rv field.  */
  reg = parse_reg (str);
  if (reg == NULL || reg->type != REG_TYPE_R_32)
    {
      set_syntax_error (_("expected a 32-bit selection register"));
      return false;
    }
  opnd->index.regno = reg->number;

  if (!skip_past_char (str, ','))
    {
      set_syntax_error (_("missing immediate offset"));
      return false;
    }

  if (!parse_sme_immediate (str, &opnd->index.imm))
    {
      set_syntax_error (_("expected a constant immediate offset"));
      return false;
    }

  if (skip_past_char (str, ':'))
    {
      int64_t end;
      if (!parse_sme_immediate (str, &end))
	{
	  set_syntax_error (_("expected a constant immediate offset"));
	  return false;
	}
      if (end < opnd->index.imm)
	{
	  set_syntax_error (_("the last offset is less than the"
			      " first offset"));
	  return false;
	}
      if (end == opnd->index.imm)
	{
	  set_syntax_error (_("the last offset is equal to the"
			      " first offset"));
	  return false;
	}
      opnd->index.countm1 = (uint64_t) end - opnd->index.imm;
    }

  opnd->group_size = 0;
  if (skip_past_char (str, ','))
    {
      if (strncasecmp (*str, "vgx2", 4) == 0 && !ISALPHA ((*str)[4]))
	{
	  *str += 4;
	  opnd->group_size = 2;
	}
      else if (strncasecmp (*str, "vgx4", 4) == 0 && !ISALPHA ((*str)[4]))
	{
	  *str += 4;
	  opnd->group_size = 4;
	}
      else
	{
	  set_syntax_error (_("invalid vector group size"));
	  return false;
	}
    }

  if (!skip_past_char (str, ']'))
    {
      set_syntax_error (_("expected ']'"));
      return false;
    }

  return true;
}

/* Parse a register of type REG_TYPE that might have an element type
   qualifier and that is indexed by two values: a 32-bit register,
   followed by an immediate.  The ranges of the register and the
   immediate vary by opcode and are checked in libopcodes.

   Return true on success, populating OPND with information about
   the operand and setting QUALIFIER to the register qualifier.

   Field format examples:

   <Pm>.<T>[<Wv>< #<imm>]
   ZA[<Wv>, #<imm>]
   <ZAn><HV>.<T>[<Wv>, #<imm>]

   FLAGS is as for parse_typed_reg.  */

static bool
parse_dual_indexed_reg (char **str, aarch64_reg_type reg_type,
			struct aarch64_indexed_za *opnd,
			aarch64_opnd_qualifier_t *qualifier,
			unsigned int flags)
{
  const reg_entry *reg = parse_reg_with_qual (str, reg_type, qualifier, flags);
  if (!reg)
    return false;

  opnd->v = aarch64_check_reg_type (reg, REG_TYPE_ZATV);
  opnd->regno = reg->number;

  return parse_sme_za_index (str, opnd);
}

/* Like parse_sme_za_hv_tiles_operand, but expect braces around the
   operand.  */

static bool
parse_sme_za_hv_tiles_operand_with_braces (char **str,
					   struct aarch64_indexed_za *opnd,
                                           aarch64_opnd_qualifier_t *qualifier)
{
  if (!skip_past_char (str, '{'))
    {
      set_expected_reglist_error (REG_TYPE_ZATHV, parse_reg (str));
      return false;
    }

  if (!parse_dual_indexed_reg (str, REG_TYPE_ZATHV, opnd, qualifier,
			       PTR_IN_REGLIST))
    return false;

  if (!skip_past_char (str, '}'))
    {
      set_syntax_error (_("expected '}'"));
      return false;
    }

  return true;
}

/* Parse list of up to eight 64-bit element tile names separated by commas in
   SME's ZERO instruction:

     ZERO { <mask> }

   Function returns <mask>:

     an 8-bit list of 64-bit element tiles named ZA0.D to ZA7.D.
*/
static int
parse_sme_zero_mask(char **str)
{
  char *q;
  int mask;
  aarch64_opnd_qualifier_t qualifier;
  unsigned int ptr_flags = PTR_IN_REGLIST;

  mask = 0x00;
  q = *str;
  do
    {
      const reg_entry *reg = parse_reg_with_qual (&q, REG_TYPE_ZA_ZAT,
						  &qualifier, ptr_flags);
      if (!reg)
	return PARSE_FAIL;

      if (reg->type == REG_TYPE_ZA)
	{
	  if (qualifier != AARCH64_OPND_QLF_NIL)
	    {
	      set_syntax_error ("ZA should not have a size suffix");
	      return PARSE_FAIL;
	    }
          /* { ZA } is assembled as all-ones immediate.  */
          mask = 0xff;
	}
      else
	{
          int regno = reg->number;
          if (qualifier == AARCH64_OPND_QLF_S_B)
            {
              /* { ZA0.B } is assembled as all-ones immediate.  */
              mask = 0xff;
            }
          else if (qualifier == AARCH64_OPND_QLF_S_H)
            mask |= 0x55 << regno;
          else if (qualifier == AARCH64_OPND_QLF_S_S)
            mask |= 0x11 << regno;
          else if (qualifier == AARCH64_OPND_QLF_S_D)
            mask |= 0x01 << regno;
	  else if (qualifier == AARCH64_OPND_QLF_S_Q)
	    {
              set_syntax_error (_("ZA tile masks do not operate at .Q"
				  " granularity"));
              return PARSE_FAIL;
	    }
	  else if (qualifier == AARCH64_OPND_QLF_NIL)
	    {
              set_syntax_error (_("missing ZA tile size"));
              return PARSE_FAIL;
	    }
          else
            {
              set_syntax_error (_("invalid ZA tile"));
              return PARSE_FAIL;
            }
        }
      ptr_flags |= PTR_GOOD_MATCH;
    }
  while (skip_past_char (&q, ','));

  *str = q;
  return mask;
}

/* Wraps in curly braces <mask> operand ZERO instruction:

   ZERO { <mask> }

   Function returns value of <mask> bit-field.
*/
static int
parse_sme_list_of_64bit_tiles (char **str)
{
  int regno;

  if (!skip_past_char (str, '{'))
    {
      set_syntax_error (_("expected '{'"));
      return PARSE_FAIL;
    }

  /* Empty <mask> list is an all-zeros immediate.  */
  if (!skip_past_char (str, '}'))
    {
      regno = parse_sme_zero_mask (str);
      if (regno == PARSE_FAIL)
         return PARSE_FAIL;

      if (!skip_past_char (str, '}'))
        {
          set_syntax_error (_("expected '}'"));
          return PARSE_FAIL;
        }
    }
  else
    regno = 0x00;

  return regno;
}

/* Parse streaming mode operand for SMSTART and SMSTOP.

   {SM | ZA}

   Function returns 's' if SM or 'z' if ZM is parsed. Otherwise PARSE_FAIL.
*/
static int
parse_sme_sm_za (char **str)
{
  char *p, *q;

  p = q = *str;
  while (ISALPHA (*q))
    q++;

  if ((q - p != 2)
      || (strncasecmp ("sm", p, 2) != 0 && strncasecmp ("za", p, 2) != 0))
    {
      set_syntax_error (_("expected SM or ZA operand"));
      return PARSE_FAIL;
    }

  *str = q;
  return TOLOWER (p[0]);
}

/* Parse a system register or a PSTATE field name for an MSR/MRS instruction.
   Returns the encoding for the option, or PARSE_FAIL.

   If IMPLE_DEFINED_P is non-zero, the function will also try to parse the
   implementation defined system register name S<op0>_<op1>_<Cn>_<Cm>_<op2>.

   If PSTATEFIELD_P is non-zero, the function will parse the name as a PSTATE
   field, otherwise as a system register.
*/

static int
parse_sys_reg (char **str, htab_t sys_regs,
	       int imple_defined_p, int pstatefield_p,
	       uint32_t* flags)
{
  char *p, *q;
  char buf[AARCH64_MAX_SYSREG_NAME_LEN];
  const aarch64_sys_reg *o;
  int value;

  p = buf;
  for (q = *str; ISALNUM (*q) || *q == '_'; q++)
    if (p < buf + (sizeof (buf) - 1))
      *p++ = TOLOWER (*q);
  *p = '\0';

  /* If the name is longer than AARCH64_MAX_SYSREG_NAME_LEN then it cannot be a
     valid system register.  This is enforced by construction of the hash
     table.  */
  if (p - buf != q - *str)
    return PARSE_FAIL;

  o = str_hash_find (sys_regs, buf);
  if (!o)
    {
      if (!imple_defined_p)
	return PARSE_FAIL;
      else
	{
	  /* Parse S<op0>_<op1>_<Cn>_<Cm>_<op2>.  */
	  unsigned int op0, op1, cn, cm, op2;

	  if (sscanf (buf, "s%u_%u_c%u_c%u_%u", &op0, &op1, &cn, &cm, &op2)
	      != 5)
	    return PARSE_FAIL;
	  if (op0 > 3 || op1 > 7 || cn > 15 || cm > 15 || op2 > 7)
	    return PARSE_FAIL;
	  value = (op0 << 14) | (op1 << 11) | (cn << 7) | (cm << 3) | op2;
	  if (flags)
	    *flags = 0;
	}
    }
  else
    {
      if (pstatefield_p && !aarch64_pstatefield_supported_p (cpu_variant, o))
	as_bad (_("selected processor does not support PSTATE field "
		  "name '%s'"), buf);
      if (!pstatefield_p
	  && !aarch64_sys_ins_reg_supported_p (cpu_variant, o->name,
					       o->value, o->flags, o->features))
	as_bad (_("selected processor does not support system register "
		  "name '%s'"), buf);
      if (aarch64_sys_reg_deprecated_p (o->flags))
	as_warn (_("system register name '%s' is deprecated and may be "
		   "removed in a future release"), buf);
      value = o->value;
      if (flags)
	*flags = o->flags;
    }

  *str = q;
  return value;
}

/* Parse a system reg for ic/dc/at/tlbi instructions.  Returns the table entry
   for the option, or NULL.  */

static const aarch64_sys_ins_reg *
parse_sys_ins_reg (char **str, htab_t sys_ins_regs)
{
  char *p, *q;
  char buf[AARCH64_MAX_SYSREG_NAME_LEN];
  const aarch64_sys_ins_reg *o;

  p = buf;
  for (q = *str; ISALNUM (*q) || *q == '_'; q++)
    if (p < buf + (sizeof (buf) - 1))
      *p++ = TOLOWER (*q);
  *p = '\0';

  /* If the name is longer than AARCH64_MAX_SYSREG_NAME_LEN then it cannot be a
     valid system register.  This is enforced by construction of the hash
     table.  */
  if (p - buf != q - *str)
    return NULL;

  o = str_hash_find (sys_ins_regs, buf);
  if (!o)
    return NULL;

  if (!aarch64_sys_ins_reg_supported_p (cpu_variant,
					o->name, o->value, o->flags, 0))
    as_bad (_("selected processor does not support system register "
	      "name '%s'"), buf);
  if (aarch64_sys_reg_deprecated_p (o->flags))
    as_warn (_("system register name '%s' is deprecated and may be "
          "removed in a future release"), buf);

  *str = q;
  return o;
}

#define po_char_or_fail(chr) do {				\
    if (! skip_past_char (&str, chr))				\
      goto failure;						\
} while (0)

#define po_reg_or_fail(regtype) do {				\
    reg = aarch64_reg_parse (&str, regtype, NULL);		\
    if (!reg)							\
      goto failure;						\
  } while (0)

#define po_int_fp_reg_or_fail(reg_type) do {			\
    reg = parse_reg (&str);					\
    if (!reg || !aarch64_check_reg_type (reg, reg_type))	\
      {								\
	set_expected_reg_error (reg_type, reg, 0);		\
	goto failure;						\
      }								\
    info->reg.regno = reg->number;				\
    info->qualifier = inherent_reg_qualifier (reg);		\
  } while (0)

#define po_imm_nc_or_fail() do {				\
    if (! parse_constant_immediate (&str, &val, imm_reg_type))	\
      goto failure;						\
  } while (0)

#define po_imm_or_fail(min, max) do {				\
    if (! parse_constant_immediate (&str, &val, imm_reg_type))	\
      goto failure;						\
    if (val < min || val > max)					\
      {								\
	set_fatal_syntax_error (_("immediate value out of range "\
#min " to "#max));						\
	goto failure;						\
      }								\
  } while (0)

#define po_enum_or_fail(array) do {				\
    if (!parse_enum_string (&str, &val, array,			\
			    ARRAY_SIZE (array), imm_reg_type))	\
      goto failure;						\
  } while (0)

#define po_strict_enum_or_fail(array) do {			\
    if (!parse_enum_string (&str, &val, array,			\
			    ARRAY_SIZE (array), REG_TYPE_MAX))	\
      goto failure;						\
  } while (0)

#define po_misc_or_fail(expr) do {				\
    if (!expr)							\
      goto failure;						\
  } while (0)

/* A primitive log calculator.  */

static inline unsigned int
get_log2 (unsigned int n)
{
  unsigned int count = 0;
  while (n > 1)
    {
      n >>= 1;
      count += 1;
    }
  return count;
}

/* encode the 12-bit imm field of Add/sub immediate */
static inline uint32_t
encode_addsub_imm (uint32_t imm)
{
  return imm << 10;
}

/* encode the shift amount field of Add/sub immediate */
static inline uint32_t
encode_addsub_imm_shift_amount (uint32_t cnt)
{
  return cnt << 22;
}


/* encode the imm field of Adr instruction */
static inline uint32_t
encode_adr_imm (uint32_t imm)
{
  return (((imm & 0x3) << 29)	/*  [1:0] -> [30:29] */
	  | ((imm & (0x7ffff << 2)) << 3));	/* [20:2] -> [23:5]  */
}

/* encode the immediate field of Move wide immediate */
static inline uint32_t
encode_movw_imm (uint32_t imm)
{
  return imm << 5;
}

/* encode the 26-bit offset of unconditional branch */
static inline uint32_t
encode_branch_ofs_26 (uint32_t ofs)
{
  return ofs & ((1 << 26) - 1);
}

/* encode the 19-bit offset of conditional branch and compare & branch */
static inline uint32_t
encode_cond_branch_ofs_19 (uint32_t ofs)
{
  return (ofs & ((1 << 19) - 1)) << 5;
}

/* encode the 19-bit offset of ld literal */
static inline uint32_t
encode_ld_lit_ofs_19 (uint32_t ofs)
{
  return (ofs & ((1 << 19) - 1)) << 5;
}

/* Encode the 14-bit offset of test & branch.  */
static inline uint32_t
encode_tst_branch_ofs_14 (uint32_t ofs)
{
  return (ofs & ((1 << 14) - 1)) << 5;
}

/* Encode the 16-bit imm field of svc/hvc/smc.  */
static inline uint32_t
encode_svc_imm (uint32_t imm)
{
  return imm << 5;
}

/* Reencode add(s) to sub(s), or sub(s) to add(s).  */
static inline uint32_t
reencode_addsub_switch_add_sub (uint32_t opcode)
{
  return opcode ^ (1 << 30);
}

static inline uint32_t
reencode_movzn_to_movz (uint32_t opcode)
{
  return opcode | (1 << 30);
}

static inline uint32_t
reencode_movzn_to_movn (uint32_t opcode)
{
  return opcode & ~(1 << 30);
}

/* Overall per-instruction processing.	*/

/* We need to be able to fix up arbitrary expressions in some statements.
   This is so that we can handle symbols that are an arbitrary distance from
   the pc.  The most common cases are of the form ((+/-sym -/+ . - 8) & mask),
   which returns part of an address in a form which will be valid for
   a data instruction.	We do this by pushing the expression into a symbol
   in the expr_section, and creating a fix for that.  */

static fixS *
fix_new_aarch64 (fragS * frag,
		 int where,
		 short int size,
		 expressionS * exp,
		 int pc_rel,
		 int reloc)
{
  fixS *new_fix;

  switch (exp->X_op)
    {
    case O_constant:
    case O_symbol:
    case O_add:
    case O_subtract:
      new_fix = fix_new_exp (frag, where, size, exp, pc_rel, reloc);
      break;

    default:
      new_fix = fix_new (frag, where, size, make_expr_symbol (exp), 0,
			 pc_rel, reloc);
      break;
    }
  return new_fix;
}

/* Diagnostics on operands errors.  */

/* By default, output verbose error message.
   Disable the verbose error message by -mno-verbose-error.  */
static int verbose_error_p = 1;

#ifdef DEBUG_AARCH64
/* N.B. this is only for the purpose of debugging.  */
const char* operand_mismatch_kind_names[] =
{
  "AARCH64_OPDE_NIL",
  "AARCH64_OPDE_RECOVERABLE",
  "AARCH64_OPDE_A_SHOULD_FOLLOW_B",
  "AARCH64_OPDE_EXPECTED_A_AFTER_B",
  "AARCH64_OPDE_SYNTAX_ERROR",
  "AARCH64_OPDE_FATAL_SYNTAX_ERROR",
  "AARCH64_OPDE_INVALID_VARIANT",
  "AARCH64_OPDE_INVALID_VG_SIZE",
  "AARCH64_OPDE_REG_LIST_LENGTH",
  "AARCH64_OPDE_REG_LIST_STRIDE",
  "AARCH64_OPDE_UNTIED_IMMS",
  "AARCH64_OPDE_UNTIED_OPERAND",
  "AARCH64_OPDE_OUT_OF_RANGE",
  "AARCH64_OPDE_UNALIGNED",
  "AARCH64_OPDE_OTHER_ERROR",
  "AARCH64_OPDE_INVALID_REGNO",
};
#endif /* DEBUG_AARCH64 */

/* Return TRUE if LHS is of higher severity than RHS, otherwise return FALSE.

   When multiple errors of different kinds are found in the same assembly
   line, only the error of the highest severity will be picked up for
   issuing the diagnostics.  */

static inline bool
operand_error_higher_severity_p (enum aarch64_operand_error_kind lhs,
				 enum aarch64_operand_error_kind rhs)
{
  gas_assert (AARCH64_OPDE_RECOVERABLE > AARCH64_OPDE_NIL);
  gas_assert (AARCH64_OPDE_A_SHOULD_FOLLOW_B > AARCH64_OPDE_RECOVERABLE);
  gas_assert (AARCH64_OPDE_EXPECTED_A_AFTER_B > AARCH64_OPDE_RECOVERABLE);
  gas_assert (AARCH64_OPDE_SYNTAX_ERROR > AARCH64_OPDE_A_SHOULD_FOLLOW_B);
  gas_assert (AARCH64_OPDE_SYNTAX_ERROR > AARCH64_OPDE_EXPECTED_A_AFTER_B);
  gas_assert (AARCH64_OPDE_FATAL_SYNTAX_ERROR > AARCH64_OPDE_SYNTAX_ERROR);
  gas_assert (AARCH64_OPDE_INVALID_VARIANT > AARCH64_OPDE_FATAL_SYNTAX_ERROR);
  gas_assert (AARCH64_OPDE_INVALID_VG_SIZE > AARCH64_OPDE_INVALID_VARIANT);
  gas_assert (AARCH64_OPDE_REG_LIST_LENGTH > AARCH64_OPDE_INVALID_VG_SIZE);
  gas_assert (AARCH64_OPDE_REG_LIST_STRIDE > AARCH64_OPDE_REG_LIST_LENGTH);
  gas_assert (AARCH64_OPDE_OUT_OF_RANGE > AARCH64_OPDE_REG_LIST_STRIDE);
  gas_assert (AARCH64_OPDE_UNALIGNED > AARCH64_OPDE_OUT_OF_RANGE);
  gas_assert (AARCH64_OPDE_OTHER_ERROR > AARCH64_OPDE_REG_LIST_STRIDE);
  gas_assert (AARCH64_OPDE_INVALID_REGNO > AARCH64_OPDE_OTHER_ERROR);
  return lhs > rhs;
}

/* Helper routine to get the mnemonic name from the assembly instruction
   line; should only be called for the diagnosis purpose, as there is
   string copy operation involved, which may affect the runtime
   performance if used in elsewhere.  */

static const char*
get_mnemonic_name (const char *str)
{
  static char mnemonic[32];
  char *ptr;

  /* Get the first 15 bytes and assume that the full name is included.  */
  strncpy (mnemonic, str, 31);
  mnemonic[31] = '\0';

  /* Scan up to the end of the mnemonic, which must end in white space,
     '.', or end of string.  */
  for (ptr = mnemonic; is_part_of_name(*ptr); ++ptr)
    ;

  *ptr = '\0';

  /* Append '...' to the truncated long name.  */
  if (ptr - mnemonic == 31)
    mnemonic[28] = mnemonic[29] = mnemonic[30] = '.';

  return mnemonic;
}

static void
reset_aarch64_instruction (aarch64_instruction *instruction)
{
  memset (instruction, '\0', sizeof (aarch64_instruction));
  instruction->reloc.type = BFD_RELOC_UNUSED;
}

/* Data structures storing one user error in the assembly code related to
   operands.  */

struct operand_error_record
{
  const aarch64_opcode *opcode;
  aarch64_operand_error detail;
  struct operand_error_record *next;
};

typedef struct operand_error_record operand_error_record;

struct operand_errors
{
  operand_error_record *head;
  operand_error_record *tail;
};

typedef struct operand_errors operand_errors;

/* Top-level data structure reporting user errors for the current line of
   the assembly code.
   The way md_assemble works is that all opcodes sharing the same mnemonic
   name are iterated to find a match to the assembly line.  In this data
   structure, each of the such opcodes will have one operand_error_record
   allocated and inserted.  In other words, excessive errors related with
   a single opcode are disregarded.  */
operand_errors operand_error_report;

/* Free record nodes.  */
static operand_error_record *free_opnd_error_record_nodes = NULL;

/* Initialize the data structure that stores the operand mismatch
   information on assembling one line of the assembly code.  */
static void
init_operand_error_report (void)
{
  if (operand_error_report.head != NULL)
    {
      gas_assert (operand_error_report.tail != NULL);
      operand_error_report.tail->next = free_opnd_error_record_nodes;
      free_opnd_error_record_nodes = operand_error_report.head;
      operand_error_report.head = NULL;
      operand_error_report.tail = NULL;
      return;
    }
  gas_assert (operand_error_report.tail == NULL);
}

/* Return TRUE if some operand error has been recorded during the
   parsing of the current assembly line using the opcode *OPCODE;
   otherwise return FALSE.  */
static inline bool
opcode_has_operand_error_p (const aarch64_opcode *opcode)
{
  operand_error_record *record = operand_error_report.head;
  return record && record->opcode == opcode;
}

/* Add the error record *NEW_RECORD to operand_error_report.  The record's
   OPCODE field is initialized with OPCODE.
   N.B. only one record for each opcode, i.e. the maximum of one error is
   recorded for each instruction template.  */

static void
add_operand_error_record (const operand_error_record* new_record)
{
  const aarch64_opcode *opcode = new_record->opcode;
  operand_error_record* record = operand_error_report.head;

  /* The record may have been created for this opcode.  If not, we need
     to prepare one.  */
  if (! opcode_has_operand_error_p (opcode))
    {
      /* Get one empty record.  */
      if (free_opnd_error_record_nodes == NULL)
	{
	  record = XNEW (operand_error_record);
	}
      else
	{
	  record = free_opnd_error_record_nodes;
	  free_opnd_error_record_nodes = record->next;
	}
      record->opcode = opcode;
      /* Insert at the head.  */
      record->next = operand_error_report.head;
      operand_error_report.head = record;
      if (operand_error_report.tail == NULL)
	operand_error_report.tail = record;
    }
  else if (record->detail.kind != AARCH64_OPDE_NIL
	   && record->detail.index <= new_record->detail.index
	   && operand_error_higher_severity_p (record->detail.kind,
					       new_record->detail.kind))
    {
      /* In the case of multiple errors found on operands related with a
	 single opcode, only record the error of the leftmost operand and
	 only if the error is of higher severity.  */
      DEBUG_TRACE ("error %s on operand %d not added to the report due to"
		   " the existing error %s on operand %d",
		   operand_mismatch_kind_names[new_record->detail.kind],
		   new_record->detail.index,
		   operand_mismatch_kind_names[record->detail.kind],
		   record->detail.index);
      return;
    }

  record->detail = new_record->detail;
}

static inline void
record_operand_error_info (const aarch64_opcode *opcode,
			   aarch64_operand_error *error_info)
{
  operand_error_record record;
  record.opcode = opcode;
  record.detail = *error_info;
  add_operand_error_record (&record);
}

/* Record an error of kind KIND and, if ERROR is not NULL, of the detailed
   error message *ERROR, for operand IDX (count from 0).  */

static void
record_operand_error (const aarch64_opcode *opcode, int idx,
		      enum aarch64_operand_error_kind kind,
		      const char* error)
{
  aarch64_operand_error info;
  memset(&info, 0, sizeof (info));
  info.index = idx;
  info.kind = kind;
  info.error = error;
  info.non_fatal = false;
  record_operand_error_info (opcode, &info);
}

static void
record_operand_error_with_data (const aarch64_opcode *opcode, int idx,
				enum aarch64_operand_error_kind kind,
				const char* error, const int *extra_data)
{
  aarch64_operand_error info;
  info.index = idx;
  info.kind = kind;
  info.error = error;
  info.data[0].i = extra_data[0];
  info.data[1].i = extra_data[1];
  info.data[2].i = extra_data[2];
  info.non_fatal = false;
  record_operand_error_info (opcode, &info);
}

static void
record_operand_out_of_range_error (const aarch64_opcode *opcode, int idx,
				   const char* error, int lower_bound,
				   int upper_bound)
{
  int data[3] = {lower_bound, upper_bound, 0};
  record_operand_error_with_data (opcode, idx, AARCH64_OPDE_OUT_OF_RANGE,
				  error, data);
}

/* Remove the operand error record for *OPCODE.  */
static void ATTRIBUTE_UNUSED
remove_operand_error_record (const aarch64_opcode *opcode)
{
  if (opcode_has_operand_error_p (opcode))
    {
      operand_error_record* record = operand_error_report.head;
      gas_assert (record != NULL && operand_error_report.tail != NULL);
      operand_error_report.head = record->next;
      record->next = free_opnd_error_record_nodes;
      free_opnd_error_record_nodes = record;
      if (operand_error_report.head == NULL)
	{
	  gas_assert (operand_error_report.tail == record);
	  operand_error_report.tail = NULL;
	}
    }
}

/* Given the instruction in *INSTR, return the index of the best matched
   qualifier sequence in the list (an array) headed by QUALIFIERS_LIST.

   Return -1 if there is no qualifier sequence; return the first match
   if there is multiple matches found.  */

static int
find_best_match (const aarch64_inst *instr,
		 const aarch64_opnd_qualifier_seq_t *qualifiers_list)
{
  int i, num_opnds, max_num_matched, idx;

  num_opnds = aarch64_num_of_operands (instr->opcode);
  if (num_opnds == 0)
    {
      DEBUG_TRACE ("no operand");
      return -1;
    }

  max_num_matched = 0;
  idx = 0;

  /* For each pattern.  */
  for (i = 0; i < AARCH64_MAX_QLF_SEQ_NUM; ++i, ++qualifiers_list)
    {
      int j, num_matched;
      const aarch64_opnd_qualifier_t *qualifiers = *qualifiers_list;

      /* Most opcodes has much fewer patterns in the list.  */
      if (empty_qualifier_sequence_p (qualifiers))
	{
	  DEBUG_TRACE_IF (i == 0, "empty list of qualifier sequence");
	  break;
	}

      for (j = 0, num_matched = 0; j < num_opnds; ++j, ++qualifiers)
	if (*qualifiers == instr->operands[j].qualifier)
	  ++num_matched;

      if (num_matched > max_num_matched)
	{
	  max_num_matched = num_matched;
	  idx = i;
	}
    }

  DEBUG_TRACE ("return with %d", idx);
  return idx;
}

/* Assign qualifiers in the qualifier sequence (headed by QUALIFIERS) to the
   corresponding operands in *INSTR.  */

static inline void
assign_qualifier_sequence (aarch64_inst *instr,
			   const aarch64_opnd_qualifier_t *qualifiers)
{
  int i = 0;
  int num_opnds = aarch64_num_of_operands (instr->opcode);
  gas_assert (num_opnds);
  for (i = 0; i < num_opnds; ++i, ++qualifiers)
    instr->operands[i].qualifier = *qualifiers;
}

/* Callback used by aarch64_print_operand to apply STYLE to the
   disassembler output created from FMT and ARGS.  The STYLER object holds
   any required state.  Must return a pointer to a string (created from FMT
   and ARGS) that will continue to be valid until the complete disassembled
   instruction has been printed.

   We don't currently add any styling to the output of the disassembler as
   used within assembler error messages, and so STYLE is ignored here.  A
   new string is allocated on the obstack help within STYLER and returned
   to the caller.  */

static const char *aarch64_apply_style
	(struct aarch64_styler *styler,
	 enum disassembler_style style ATTRIBUTE_UNUSED,
	 const char *fmt, va_list args)
{
  int res;
  char *ptr;
  struct obstack *stack = (struct obstack *) styler->state;
  va_list ap;

  /* Calculate the required space.  */
  va_copy (ap, args);
  res = vsnprintf (NULL, 0, fmt, ap);
  va_end (ap);
  gas_assert (res >= 0);

  /* Allocate space on the obstack and format the result.  */
  ptr = (char *) obstack_alloc (stack, res + 1);
  res = vsnprintf (ptr, (res + 1), fmt, args);
  gas_assert (res >= 0);

  return ptr;
}

/* Print operands for the diagnosis purpose.  */

static void
print_operands (char *buf, const aarch64_opcode *opcode,
		const aarch64_opnd_info *opnds)
{
  int i;
  struct aarch64_styler styler;
  struct obstack content;
  obstack_init (&content);

  styler.apply_style = aarch64_apply_style;
  styler.state = (void *) &content;

  for (i = 0; i < AARCH64_MAX_OPND_NUM; ++i)
    {
      char str[128];
      char cmt[128];

      /* We regard the opcode operand info more, however we also look into
	 the inst->operands to support the disassembling of the optional
	 operand.
	 The two operand code should be the same in all cases, apart from
	 when the operand can be optional.  */
      if (opcode->operands[i] == AARCH64_OPND_NIL
	  || opnds[i].type == AARCH64_OPND_NIL)
	break;

      /* Generate the operand string in STR.  */
      aarch64_print_operand (str, sizeof (str), 0, opcode, opnds, i, NULL, NULL,
			     NULL, cmt, sizeof (cmt), cpu_variant, &styler);

      /* Delimiter.  */
      if (str[0] != '\0')
	strcat (buf, i == 0 ? " " : ", ");

      /* Append the operand string.  */
      strcat (buf, str);

      /* Append a comment.  This works because only the last operand ever
	 adds a comment.  If that ever changes then we'll need to be
	 smarter here.  */
      if (cmt[0] != '\0')
	{
	  strcat (buf, "\t// ");
	  strcat (buf, cmt);
	}
    }

  obstack_free (&content, NULL);
}

/* Send to stderr a string as information.  */

static void
output_info (const char *format, ...)
{
  const char *file;
  unsigned int line;
  va_list args;

  file = as_where (&line);
  if (file)
    {
      if (line != 0)
	fprintf (stderr, "%s:%u: ", file, line);
      else
	fprintf (stderr, "%s: ", file);
    }
  fprintf (stderr, _("Info: "));
  va_start (args, format);
  vfprintf (stderr, format, args);
  va_end (args);
  (void) putc ('\n', stderr);
}

/* See if the AARCH64_OPDE_SYNTAX_ERROR error described by DETAIL
   relates to registers or register lists.  If so, return a string that
   reports the error against "operand %d", otherwise return null.  */

static const char *
get_reg_error_message (const aarch64_operand_error *detail)
{
  /* Handle the case where we found a register that was expected
     to be in a register list outside of a register list.  */
  if ((detail->data[1].i & detail->data[2].i) != 0
      && (detail->data[1].i & SEF_IN_REGLIST) == 0)
    return _("missing braces at operand %d");

  /* If some opcodes expected a register, and we found a register,
     complain about the difference.  */
  if (detail->data[2].i)
    {
      unsigned int expected = (detail->data[1].i & SEF_IN_REGLIST
			       ? detail->data[1].i & ~SEF_IN_REGLIST
			       : detail->data[0].i & ~SEF_DEFAULT_ERROR);
      const char *msg = get_reg_expected_msg (expected, detail->data[2].i);
      if (!msg)
	msg = N_("unexpected register type at operand %d");
      return msg;
    }

  /* Handle the case where we got to the point of trying to parse a
     register within a register list, but didn't find a known register.  */
  if (detail->data[1].i & SEF_IN_REGLIST)
    {
      unsigned int expected = detail->data[1].i & ~SEF_IN_REGLIST;
      const char *msg = get_reg_expected_msg (expected, 0);
      if (!msg)
	msg = _("invalid register list at operand %d");
      return msg;
    }

  /* Punt if register-related problems weren't the only errors.  */
  if (detail->data[0].i & SEF_DEFAULT_ERROR)
    return NULL;

  /* Handle the case where the only acceptable things are registers.  */
  if (detail->data[1].i == 0)
    {
      const char *msg = get_reg_expected_msg (detail->data[0].i, 0);
      if (!msg)
	msg = _("expected a register at operand %d");
      return msg;
    }

  /* Handle the case where the only acceptable things are register lists,
     and there was no opening '{'.  */
  if (detail->data[0].i == 0)
    return _("expected '{' at operand %d");

  return _("expected a register or register list at operand %d");
}

/* Output one operand error record.  */

static void
output_operand_error_record (const operand_error_record *record, char *str)
{
  const aarch64_operand_error *detail = &record->detail;
  int idx = detail->index;
  const aarch64_opcode *opcode = record->opcode;
  enum aarch64_opnd opd_code = (idx >= 0 ? opcode->operands[idx]
				: AARCH64_OPND_NIL);

  typedef void (*handler_t)(const char *format, ...);
  handler_t handler = detail->non_fatal ? as_warn : as_bad;
  const char *msg = detail->error;

  switch (detail->kind)
    {
    case AARCH64_OPDE_NIL:
      gas_assert (0);
      break;

    case AARCH64_OPDE_A_SHOULD_FOLLOW_B:
      handler (_("this `%s' should have an immediately preceding `%s'"
		 " -- `%s'"),
	       detail->data[0].s, detail->data[1].s, str);
      break;

    case AARCH64_OPDE_EXPECTED_A_AFTER_B:
      handler (_("the preceding `%s' should be followed by `%s` rather"
		 " than `%s` -- `%s'"),
	       detail->data[1].s, detail->data[0].s, opcode->name, str);
      break;

    case AARCH64_OPDE_SYNTAX_ERROR:
      if (!msg && idx >= 0)
	{
	  msg = get_reg_error_message (detail);
	  if (msg)
	    {
	      char *full_msg = xasprintf (msg, idx + 1);
	      handler (_("%s -- `%s'"), full_msg, str);
	      free (full_msg);
	      break;
	    }
	}
      /* Fall through.  */

    case AARCH64_OPDE_RECOVERABLE:
    case AARCH64_OPDE_FATAL_SYNTAX_ERROR:
    case AARCH64_OPDE_OTHER_ERROR:
      /* Use the prepared error message if there is, otherwise use the
	 operand description string to describe the error.  */
      if (msg != NULL)
	{
	  if (idx < 0)
	    handler (_("%s -- `%s'"), msg, str);
	  else
	    handler (_("%s at operand %d -- `%s'"),
		     msg, idx + 1, str);
	}
      else
	{
	  gas_assert (idx >= 0);
	  handler (_("operand %d must be %s -- `%s'"), idx + 1,
		   aarch64_get_operand_desc (opd_code), str);
	}
      break;

    case AARCH64_OPDE_INVALID_VARIANT:
      handler (_("operand mismatch -- `%s'"), str);
      if (verbose_error_p)
	{
	  /* We will try to correct the erroneous instruction and also provide
	     more information e.g. all other valid variants.

	     The string representation of the corrected instruction and other
	     valid variants are generated by

	     1) obtaining the intermediate representation of the erroneous
	     instruction;
	     2) manipulating the IR, e.g. replacing the operand qualifier;
	     3) printing out the instruction by calling the printer functions
	     shared with the disassembler.

	     The limitation of this method is that the exact input assembly
	     line cannot be accurately reproduced in some cases, for example an
	     optional operand present in the actual assembly line will be
	     omitted in the output; likewise for the optional syntax rules,
	     e.g. the # before the immediate.  Another limitation is that the
	     assembly symbols and relocation operations in the assembly line
	     currently cannot be printed out in the error report.  Last but not
	     least, when there is other error(s) co-exist with this error, the
	     'corrected' instruction may be still incorrect, e.g.  given
	       'ldnp h0,h1,[x0,#6]!'
	     this diagnosis will provide the version:
	       'ldnp s0,s1,[x0,#6]!'
	     which is still not right.  */
	  size_t len = strlen (get_mnemonic_name (str));
	  int i, qlf_idx;
	  bool result;
	  char buf[2048];
	  aarch64_inst *inst_base = &inst.base;
	  const aarch64_opnd_qualifier_seq_t *qualifiers_list;

	  /* Init inst.  */
	  reset_aarch64_instruction (&inst);
	  inst_base->opcode = opcode;

	  /* Reset the error report so that there is no side effect on the
	     following operand parsing.  */
	  init_operand_error_report ();

	  /* Fill inst.  */
	  result = parse_operands (str + len, opcode)
	    && programmer_friendly_fixup (&inst);
	  gas_assert (result);
	  result = aarch64_opcode_encode (opcode, inst_base, &inst_base->value,
					  NULL, NULL, insn_sequence);
	  gas_assert (!result);

	  /* Find the most matched qualifier sequence.  */
	  qlf_idx = find_best_match (inst_base, opcode->qualifiers_list);
	  gas_assert (qlf_idx > -1);

	  /* Assign the qualifiers.  */
	  assign_qualifier_sequence (inst_base,
				     opcode->qualifiers_list[qlf_idx]);

	  /* Print the hint.  */
	  output_info (_("   did you mean this?"));
	  snprintf (buf, sizeof (buf), "\t%s", get_mnemonic_name (str));
	  print_operands (buf, opcode, inst_base->operands);
	  output_info (_("   %s"), buf);

	  /* Print out other variant(s) if there is any.  */
	  if (qlf_idx != 0 ||
	      !empty_qualifier_sequence_p (opcode->qualifiers_list[1]))
	    output_info (_("   other valid variant(s):"));

	  /* For each pattern.  */
	  qualifiers_list = opcode->qualifiers_list;
	  for (i = 0; i < AARCH64_MAX_QLF_SEQ_NUM; ++i, ++qualifiers_list)
	    {
	      /* Most opcodes has much fewer patterns in the list.
		 First NIL qualifier indicates the end in the list.   */
	      if (empty_qualifier_sequence_p (*qualifiers_list))
		break;

	      if (i != qlf_idx)
		{
		  /* Mnemonics name.  */
		  snprintf (buf, sizeof (buf), "\t%s", get_mnemonic_name (str));

		  /* Assign the qualifiers.  */
		  assign_qualifier_sequence (inst_base, *qualifiers_list);

		  /* Print instruction.  */
		  print_operands (buf, opcode, inst_base->operands);

		  output_info (_("   %s"), buf);
		}
	    }
	}
      break;

    case AARCH64_OPDE_UNTIED_IMMS:
      handler (_("operand %d must have the same immediate value "
                 "as operand 1 -- `%s'"),
               detail->index + 1, str);
      break;

    case AARCH64_OPDE_UNTIED_OPERAND:
      handler (_("operand %d must be the same register as operand 1 -- `%s'"),
               detail->index + 1, str);
      break;

    case AARCH64_OPDE_INVALID_REGNO:
      handler (_("%s%d-%s%d expected at operand %d -- `%s'"),
	       detail->data[0].s, detail->data[1].i,
	       detail->data[0].s, detail->data[2].i, idx + 1, str);
      break;

    case AARCH64_OPDE_OUT_OF_RANGE:
      if (detail->data[0].i != detail->data[1].i)
	handler (_("%s out of range %d to %d at operand %d -- `%s'"),
		 msg ? msg : _("immediate value"),
		 detail->data[0].i, detail->data[1].i, idx + 1, str);
      else
	handler (_("%s must be %d at operand %d -- `%s'"),
		 msg ? msg : _("immediate value"),
		 detail->data[0].i, idx + 1, str);
      break;

    case AARCH64_OPDE_INVALID_VG_SIZE:
      if (detail->data[0].i == 0)
	handler (_("unexpected vector group size at operand %d -- `%s'"),
		 idx + 1, str);
      else
	handler (_("operand %d must have a vector group size of %d -- `%s'"),
		 idx + 1, detail->data[0].i, str);
      break;

    case AARCH64_OPDE_REG_LIST_LENGTH:
      if (detail->data[0].i == (1 << 1))
	handler (_("expected a single-register list at operand %d -- `%s'"),
		 idx + 1, str);
      else if ((detail->data[0].i & -detail->data[0].i) == detail->data[0].i)
	handler (_("expected a list of %d registers at operand %d -- `%s'"),
		 get_log2 (detail->data[0].i), idx + 1, str);
      else if (detail->data[0].i == 0x14)
	handler (_("expected a list of %d or %d registers at"
		   " operand %d -- `%s'"),
		 2, 4, idx + 1, str);
      else
	handler (_("invalid number of registers in the list"
		   " at operand %d -- `%s'"), idx + 1, str);
      break;

    case AARCH64_OPDE_REG_LIST_STRIDE:
      if (detail->data[0].i == (1 << 1))
	handler (_("the register list must have a stride of %d"
		   " at operand %d -- `%s'"), 1, idx + 1, str);
      else if (detail->data[0].i == 0x12 || detail->data[0].i == 0x102)
	handler (_("the register list must have a stride of %d or %d"
		   " at operand %d -- `%s`"), 1,
		 detail->data[0].i == 0x12 ? 4 : 8, idx + 1, str);
      else
	handler (_("invalid register stride at operand %d -- `%s'"),
		 idx + 1, str);
      break;

    case AARCH64_OPDE_UNALIGNED:
      handler (_("immediate value must be a multiple of "
		 "%d at operand %d -- `%s'"),
	       detail->data[0].i, idx + 1, str);
      break;

    default:
      gas_assert (0);
      break;
    }
}

/* Return true if the presence of error A against an instruction means
   that error B should not be reported.  This is only used as a first pass,
   to pick the kind of error that we should report.  */

static bool
better_error_p (operand_error_record *a, operand_error_record *b)
{
  /* For errors reported during parsing, prefer errors that relate to
     later operands, since that implies that the earlier operands were
     syntactically valid.

     For example, if we see a register R instead of an immediate in
     operand N, we'll report that as a recoverable "immediate operand
     required" error.  This is because there is often another opcode
     entry that accepts a register operand N, and any errors about R
     should be reported against the register forms of the instruction.
     But if no such register form exists, the recoverable error should
     still win over a syntax error against operand N-1.

     For these purposes, count an error reported at the end of the
     assembly string as equivalent to an error reported against the
     final operand.  This means that opcode entries that expect more
     operands win over "unexpected characters following instruction".  */
  if (a->detail.kind <= AARCH64_OPDE_FATAL_SYNTAX_ERROR
      && b->detail.kind <= AARCH64_OPDE_FATAL_SYNTAX_ERROR)
    {
      int a_index = (a->detail.index < 0
		     ? aarch64_num_of_operands (a->opcode) - 1
		     : a->detail.index);
      int b_index = (b->detail.index < 0
		     ? aarch64_num_of_operands (b->opcode) - 1
		     : b->detail.index);
      if (a_index != b_index)
	return a_index > b_index;
    }
  return operand_error_higher_severity_p (a->detail.kind, b->detail.kind);
}

/* Process and output the error message about the operand mismatching.

   When this function is called, the operand error information had
   been collected for an assembly line and there will be multiple
   errors in the case of multiple instruction templates; output the
   error message that most closely describes the problem.

   The errors to be printed can be filtered on printing all errors
   or only non-fatal errors.  This distinction has to be made because
   the error buffer may already be filled with fatal errors we don't want to
   print due to the different instruction templates.  */

static void
output_operand_error_report (char *str, bool non_fatal_only)
{
  enum aarch64_operand_error_kind kind;
  operand_error_record *curr;
  operand_error_record *head = operand_error_report.head;
  operand_error_record *record;

  /* No error to report.  */
  if (head == NULL)
    return;

  gas_assert (head != NULL && operand_error_report.tail != NULL);

  /* Only one error.  */
  if (head == operand_error_report.tail)
    {
      /* If the only error is a non-fatal one and we don't want to print it,
	 just exit.  */
      if (!non_fatal_only || head->detail.non_fatal)
	{
	  DEBUG_TRACE ("single opcode entry with error kind: %s",
		       operand_mismatch_kind_names[head->detail.kind]);
	  output_operand_error_record (head, str);
	}
      return;
    }

  /* Find the error kind of the highest severity.  */
  DEBUG_TRACE ("multiple opcode entries with error kind");
  record = NULL;
  for (curr = head; curr != NULL; curr = curr->next)
    {
      gas_assert (curr->detail.kind != AARCH64_OPDE_NIL);
      if (curr->detail.kind == AARCH64_OPDE_SYNTAX_ERROR)
	{
	  DEBUG_TRACE ("\t%s [%x, %x, %x]",
		       operand_mismatch_kind_names[curr->detail.kind],
		       curr->detail.data[0].i, curr->detail.data[1].i,
		       curr->detail.data[2].i);
	}
      else if (curr->detail.kind == AARCH64_OPDE_REG_LIST_LENGTH
	       || curr->detail.kind == AARCH64_OPDE_REG_LIST_STRIDE)
	{
	  DEBUG_TRACE ("\t%s [%x]",
		       operand_mismatch_kind_names[curr->detail.kind],
		       curr->detail.data[0].i);
	}
      else
	{
	  DEBUG_TRACE ("\t%s", operand_mismatch_kind_names[curr->detail.kind]);
	}
      if ((!non_fatal_only || curr->detail.non_fatal)
	  && (!record || better_error_p (curr, record)))
	record = curr;
    }

  kind = (record ? record->detail.kind : AARCH64_OPDE_NIL);
  gas_assert (kind != AARCH64_OPDE_NIL || non_fatal_only);

  /* Pick up one of errors of KIND to report.  */
  record = NULL;
  for (curr = head; curr != NULL; curr = curr->next)
    {
      /* If we don't want to print non-fatal errors then don't consider them
	 at all.  */
      if (curr->detail.kind != kind
	  || (non_fatal_only && !curr->detail.non_fatal))
	continue;
      /* If there are multiple errors, pick up the one with the highest
	 mismatching operand index.  In the case of multiple errors with
	 the equally highest operand index, pick up the first one or the
	 first one with non-NULL error message.  */
      if (!record || curr->detail.index > record->detail.index)
	record = curr;
      else if (curr->detail.index == record->detail.index
	       && !record->detail.error)
	{
	  if (curr->detail.error)
	    record = curr;
	  else if (kind == AARCH64_OPDE_SYNTAX_ERROR)
	    {
	      record->detail.data[0].i |= curr->detail.data[0].i;
	      record->detail.data[1].i |= curr->detail.data[1].i;
	      record->detail.data[2].i |= curr->detail.data[2].i;
	      DEBUG_TRACE ("\t--> %s [%x, %x, %x]",
			   operand_mismatch_kind_names[kind],
			   curr->detail.data[0].i, curr->detail.data[1].i,
			   curr->detail.data[2].i);
	    }
	  else if (kind == AARCH64_OPDE_REG_LIST_LENGTH
		   || kind == AARCH64_OPDE_REG_LIST_STRIDE)
	    {
	      record->detail.data[0].i |= curr->detail.data[0].i;
	      DEBUG_TRACE ("\t--> %s [%x]",
			   operand_mismatch_kind_names[kind],
			   curr->detail.data[0].i);
	    }
	  /* Pick the variant with the cloest match.  */
	  else if (kind == AARCH64_OPDE_INVALID_VARIANT
		   && record->detail.data[0].i > curr->detail.data[0].i)
	    record = curr;
	}
    }

  /* The way errors are collected in the back-end is a bit non-intuitive.  But
     essentially, because each operand template is tried recursively you may
     always have errors collected from the previous tried OPND.  These are
     usually skipped if there is one successful match.  However now with the
     non-fatal errors we have to ignore those previously collected hard errors
     when we're only interested in printing the non-fatal ones.  This condition
     prevents us from printing errors that are not appropriate, since we did
     match a condition, but it also has warnings that it wants to print.  */
  if (non_fatal_only && !record)
    return;

  gas_assert (record);
  DEBUG_TRACE ("Pick up error kind %s to report",
	       operand_mismatch_kind_names[kind]);

  /* Output.  */
  output_operand_error_record (record, str);
}

/* Write an AARCH64 instruction to buf - always little-endian.  */
static void
put_aarch64_insn (char *buf, uint32_t insn)
{
  unsigned char *where = (unsigned char *) buf;
  where[0] = insn;
  where[1] = insn >> 8;
  where[2] = insn >> 16;
  where[3] = insn >> 24;
}

static uint32_t
get_aarch64_insn (char *buf)
{
  unsigned char *where = (unsigned char *) buf;
  uint32_t result;
  result = ((where[0] | (where[1] << 8) | (where[2] << 16)
	     | ((uint32_t) where[3] << 24)));
  return result;
}

static void
output_inst (struct aarch64_inst *new_inst)
{
  char *to = NULL;

  to = frag_more (INSN_SIZE);

  frag_now->tc_frag_data.recorded = 1;

  put_aarch64_insn (to, inst.base.value);

  if (inst.reloc.type != BFD_RELOC_UNUSED)
    {
      fixS *fixp = fix_new_aarch64 (frag_now, to - frag_now->fr_literal,
				    INSN_SIZE, &inst.reloc.exp,
				    inst.reloc.pc_rel,
				    inst.reloc.type);
      DEBUG_TRACE ("Prepared relocation fix up");
      /* Don't check the addend value against the instruction size,
         that's the job of our code in md_apply_fix(). */
      fixp->fx_no_overflow = 1;
      if (new_inst != NULL)
	fixp->tc_fix_data.inst = new_inst;
      if (aarch64_gas_internal_fixup_p ())
	{
	  gas_assert (inst.reloc.opnd != AARCH64_OPND_NIL);
	  fixp->tc_fix_data.opnd = inst.reloc.opnd;
	  fixp->fx_addnumber = inst.reloc.flags;
	}
    }

  dwarf2_emit_insn (INSN_SIZE);
}

/* Link together opcodes of the same name.  */

struct templates
{
  const aarch64_opcode *opcode;
  struct templates *next;
};

typedef struct templates templates;

static templates *
lookup_mnemonic (const char *start, int len)
{
  templates *templ = NULL;

  templ = str_hash_find_n (aarch64_ops_hsh, start, len);
  return templ;
}

/* Subroutine of md_assemble, responsible for looking up the primary
   opcode from the mnemonic the user wrote.  BASE points to the beginning
   of the mnemonic, DOT points to the first '.' within the mnemonic
   (if any) and END points to the end of the mnemonic.  */

static templates *
opcode_lookup (char *base, char *dot, char *end)
{
  const aarch64_cond *cond;
  char condname[16];
  int len;

  if (dot == end)
    return 0;

  inst.cond = COND_ALWAYS;

  /* Handle a possible condition.  */
  if (dot)
    {
      cond = str_hash_find_n (aarch64_cond_hsh, dot + 1, end - dot - 1);
      if (!cond)
	return 0;
      inst.cond = cond->value;
      len = dot - base;
    }
  else
    len = end - base;

  if (inst.cond == COND_ALWAYS)
    {
      /* Look for unaffixed mnemonic.  */
      return lookup_mnemonic (base, len);
    }
  else if (len <= 13)
    {
      /* append ".c" to mnemonic if conditional */
      memcpy (condname, base, len);
      memcpy (condname + len, ".c", 2);
      base = condname;
      len += 2;
      return lookup_mnemonic (base, len);
    }

  return NULL;
}

/* Process an optional operand that is found omitted from the assembly line.
   Fill *OPERAND for such an operand of type TYPE.  OPCODE points to the
   instruction's opcode entry while IDX is the index of this omitted operand.
   */

static void
process_omitted_operand (enum aarch64_opnd type, const aarch64_opcode *opcode,
			 int idx, aarch64_opnd_info *operand)
{
  aarch64_insn default_value = get_optional_operand_default_value (opcode);
  gas_assert (optional_operand_p (opcode, idx));
  gas_assert (!operand->present);

  switch (type)
    {
    case AARCH64_OPND_Rd:
    case AARCH64_OPND_Rn:
    case AARCH64_OPND_Rm:
    case AARCH64_OPND_Rt:
    case AARCH64_OPND_Rt2:
    case AARCH64_OPND_Rt_LS64:
    case AARCH64_OPND_Rt_SP:
    case AARCH64_OPND_Rs:
    case AARCH64_OPND_Ra:
    case AARCH64_OPND_Rt_SYS:
    case AARCH64_OPND_Rd_SP:
    case AARCH64_OPND_Rn_SP:
    case AARCH64_OPND_Rm_SP:
    case AARCH64_OPND_Fd:
    case AARCH64_OPND_Fn:
    case AARCH64_OPND_Fm:
    case AARCH64_OPND_Fa:
    case AARCH64_OPND_Ft:
    case AARCH64_OPND_Ft2:
    case AARCH64_OPND_Sd:
    case AARCH64_OPND_Sn:
    case AARCH64_OPND_Sm:
    case AARCH64_OPND_Va:
    case AARCH64_OPND_Vd:
    case AARCH64_OPND_Vn:
    case AARCH64_OPND_Vm:
    case AARCH64_OPND_VdD1:
    case AARCH64_OPND_VnD1:
      operand->reg.regno = default_value;
      break;

    case AARCH64_OPND_Ed:
    case AARCH64_OPND_En:
    case AARCH64_OPND_Em:
    case AARCH64_OPND_Em16:
    case AARCH64_OPND_SM3_IMM2:
      operand->reglane.regno = default_value;
      break;

    case AARCH64_OPND_IDX:
    case AARCH64_OPND_BIT_NUM:
    case AARCH64_OPND_IMMR:
    case AARCH64_OPND_IMMS:
    case AARCH64_OPND_SHLL_IMM:
    case AARCH64_OPND_IMM_VLSL:
    case AARCH64_OPND_IMM_VLSR:
    case AARCH64_OPND_CCMP_IMM:
    case AARCH64_OPND_FBITS:
    case AARCH64_OPND_UIMM4:
    case AARCH64_OPND_UIMM3_OP1:
    case AARCH64_OPND_UIMM3_OP2:
    case AARCH64_OPND_IMM:
    case AARCH64_OPND_IMM_2:
    case AARCH64_OPND_WIDTH:
    case AARCH64_OPND_UIMM7:
    case AARCH64_OPND_NZCV:
    case AARCH64_OPND_SVE_PATTERN:
    case AARCH64_OPND_SVE_PRFOP:
      operand->imm.value = default_value;
      break;

    case AARCH64_OPND_SVE_PATTERN_SCALED:
      operand->imm.value = default_value;
      operand->shifter.kind = AARCH64_MOD_MUL;
      operand->shifter.amount = 1;
      break;

    case AARCH64_OPND_EXCEPTION:
      inst.reloc.type = BFD_RELOC_UNUSED;
      break;

    case AARCH64_OPND_BARRIER_ISB:
      operand->barrier = aarch64_barrier_options + default_value;
      break;

    case AARCH64_OPND_BTI_TARGET:
      operand->hint_option = aarch64_hint_options + default_value;
      break;

    default:
      break;
    }
}

/* Process the relocation type for move wide instructions.
   Return TRUE on success; otherwise return FALSE.  */

static bool
process_movw_reloc_info (void)
{
  int is32;
  unsigned shift;

  is32 = inst.base.operands[0].qualifier == AARCH64_OPND_QLF_W ? 1 : 0;

  if (inst.base.opcode->op == OP_MOVK)
    switch (inst.reloc.type)
      {
      case BFD_RELOC_AARCH64_MOVW_G0_S:
      case BFD_RELOC_AARCH64_MOVW_G1_S:
      case BFD_RELOC_AARCH64_MOVW_G2_S:
      case BFD_RELOC_AARCH64_MOVW_PREL_G0:
      case BFD_RELOC_AARCH64_MOVW_PREL_G1:
      case BFD_RELOC_AARCH64_MOVW_PREL_G2:
      case BFD_RELOC_AARCH64_MOVW_PREL_G3:
      case BFD_RELOC_AARCH64_TLSGD_MOVW_G1:
      case BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G0:
      case BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G1:
      case BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G2:
	set_syntax_error
	  (_("the specified relocation type is not allowed for MOVK"));
	return false;
      default:
	break;
      }

  switch (inst.reloc.type)
    {
    case BFD_RELOC_AARCH64_MOVW_G0:
    case BFD_RELOC_AARCH64_MOVW_G0_NC:
    case BFD_RELOC_AARCH64_MOVW_G0_S:
    case BFD_RELOC_AARCH64_MOVW_GOTOFF_G0_NC:
    case BFD_RELOC_AARCH64_MOVW_PREL_G0:
    case BFD_RELOC_AARCH64_MOVW_PREL_G0_NC:
    case BFD_RELOC_AARCH64_TLSDESC_OFF_G0_NC:
    case BFD_RELOC_AARCH64_TLSGD_MOVW_G0_NC:
    case BFD_RELOC_AARCH64_TLSIE_MOVW_GOTTPREL_G0_NC:
    case BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G0:
    case BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G0_NC:
    case BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G0:
    case BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G0_NC:
      shift = 0;
      break;
    case BFD_RELOC_AARCH64_MOVW_G1:
    case BFD_RELOC_AARCH64_MOVW_G1_NC:
    case BFD_RELOC_AARCH64_MOVW_G1_S:
    case BFD_RELOC_AARCH64_MOVW_GOTOFF_G1:
    case BFD_RELOC_AARCH64_MOVW_PREL_G1:
    case BFD_RELOC_AARCH64_MOVW_PREL_G1_NC:
    case BFD_RELOC_AARCH64_TLSDESC_OFF_G1:
    case BFD_RELOC_AARCH64_TLSGD_MOVW_G1:
    case BFD_RELOC_AARCH64_TLSIE_MOVW_GOTTPREL_G1:
    case BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G1:
    case BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G1_NC:
    case BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G1:
    case BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G1_NC:
      shift = 16;
      break;
    case BFD_RELOC_AARCH64_MOVW_G2:
    case BFD_RELOC_AARCH64_MOVW_G2_NC:
    case BFD_RELOC_AARCH64_MOVW_G2_S:
    case BFD_RELOC_AARCH64_MOVW_PREL_G2:
    case BFD_RELOC_AARCH64_MOVW_PREL_G2_NC:
    case BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G2:
    case BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G2:
      if (is32)
	{
	  set_fatal_syntax_error
	    (_("the specified relocation type is not allowed for 32-bit "
	       "register"));
	  return false;
	}
      shift = 32;
      break;
    case BFD_RELOC_AARCH64_MOVW_G3:
    case BFD_RELOC_AARCH64_MOVW_PREL_G3:
      if (is32)
	{
	  set_fatal_syntax_error
	    (_("the specified relocation type is not allowed for 32-bit "
	       "register"));
	  return false;
	}
      shift = 48;
      break;
    default:
      /* More cases should be added when more MOVW-related relocation types
         are supported in GAS.  */
      gas_assert (aarch64_gas_internal_fixup_p ());
      /* The shift amount should have already been set by the parser.  */
      return true;
    }
  inst.base.operands[1].shifter.amount = shift;
  return true;
}

/* Determine and return the real reloc type code for an instruction
   with the pseudo reloc type code BFD_RELOC_AARCH64_LDST_LO12.  */

static inline bfd_reloc_code_real_type
ldst_lo12_determine_real_reloc_type (void)
{
  unsigned logsz, max_logsz;
  enum aarch64_opnd_qualifier opd0_qlf = inst.base.operands[0].qualifier;
  enum aarch64_opnd_qualifier opd1_qlf = inst.base.operands[1].qualifier;

  const bfd_reloc_code_real_type reloc_ldst_lo12[5][5] = {
    {
      BFD_RELOC_AARCH64_LDST8_LO12,
      BFD_RELOC_AARCH64_LDST16_LO12,
      BFD_RELOC_AARCH64_LDST32_LO12,
      BFD_RELOC_AARCH64_LDST64_LO12,
      BFD_RELOC_AARCH64_LDST128_LO12
    },
    {
      BFD_RELOC_AARCH64_TLSLD_LDST8_DTPREL_LO12,
      BFD_RELOC_AARCH64_TLSLD_LDST16_DTPREL_LO12,
      BFD_RELOC_AARCH64_TLSLD_LDST32_DTPREL_LO12,
      BFD_RELOC_AARCH64_TLSLD_LDST64_DTPREL_LO12,
      BFD_RELOC_AARCH64_NONE
    },
    {
      BFD_RELOC_AARCH64_TLSLD_LDST8_DTPREL_LO12_NC,
      BFD_RELOC_AARCH64_TLSLD_LDST16_DTPREL_LO12_NC,
      BFD_RELOC_AARCH64_TLSLD_LDST32_DTPREL_LO12_NC,
      BFD_RELOC_AARCH64_TLSLD_LDST64_DTPREL_LO12_NC,
      BFD_RELOC_AARCH64_NONE
    },
    {
      BFD_RELOC_AARCH64_TLSLE_LDST8_TPREL_LO12,
      BFD_RELOC_AARCH64_TLSLE_LDST16_TPREL_LO12,
      BFD_RELOC_AARCH64_TLSLE_LDST32_TPREL_LO12,
      BFD_RELOC_AARCH64_TLSLE_LDST64_TPREL_LO12,
      BFD_RELOC_AARCH64_NONE
    },
    {
      BFD_RELOC_AARCH64_TLSLE_LDST8_TPREL_LO12_NC,
      BFD_RELOC_AARCH64_TLSLE_LDST16_TPREL_LO12_NC,
      BFD_RELOC_AARCH64_TLSLE_LDST32_TPREL_LO12_NC,
      BFD_RELOC_AARCH64_TLSLE_LDST64_TPREL_LO12_NC,
      BFD_RELOC_AARCH64_NONE
    }
  };

  gas_assert (inst.reloc.type == BFD_RELOC_AARCH64_LDST_LO12
	      || inst.reloc.type == BFD_RELOC_AARCH64_TLSLD_LDST_DTPREL_LO12
	      || (inst.reloc.type
		  == BFD_RELOC_AARCH64_TLSLD_LDST_DTPREL_LO12_NC)
	      || (inst.reloc.type
		  == BFD_RELOC_AARCH64_TLSLE_LDST_TPREL_LO12)
	      || (inst.reloc.type
		  == BFD_RELOC_AARCH64_TLSLE_LDST_TPREL_LO12_NC));
  gas_assert (inst.base.opcode->operands[1] == AARCH64_OPND_ADDR_UIMM12);

  if (opd1_qlf == AARCH64_OPND_QLF_NIL)
    opd1_qlf =
      aarch64_get_expected_qualifier (inst.base.opcode->qualifiers_list,
				      1, opd0_qlf, 0);
  gas_assert (opd1_qlf != AARCH64_OPND_QLF_NIL);

  logsz = get_log2 (aarch64_get_qualifier_esize (opd1_qlf));

  if (inst.reloc.type == BFD_RELOC_AARCH64_TLSLD_LDST_DTPREL_LO12
      || inst.reloc.type == BFD_RELOC_AARCH64_TLSLD_LDST_DTPREL_LO12_NC
      || inst.reloc.type == BFD_RELOC_AARCH64_TLSLE_LDST_TPREL_LO12
      || inst.reloc.type == BFD_RELOC_AARCH64_TLSLE_LDST_TPREL_LO12_NC)
    max_logsz = 3;
  else
    max_logsz = 4;

  if (logsz > max_logsz)
    {
      /* SEE PR 27904 for an example of this.  */
      set_fatal_syntax_error
	(_("relocation qualifier does not match instruction size"));
      return BFD_RELOC_AARCH64_NONE;
    }

  /* In reloc.c, these pseudo relocation types should be defined in similar
     order as above reloc_ldst_lo12 array. Because the array index calculation
     below relies on this.  */
  return reloc_ldst_lo12[inst.reloc.type - BFD_RELOC_AARCH64_LDST_LO12][logsz];
}

/* Check whether a register list REGINFO is valid.  The registers have type
   REG_TYPE and must be numbered in increasing order (modulo the register
   bank size).  They must have a consistent stride.

   Return true if the list is valid, describing it in LIST if so.  */

static bool
reg_list_valid_p (uint32_t reginfo, struct aarch64_reglist *list,
		  aarch64_reg_type reg_type)
{
  uint32_t i, nb_regs, prev_regno, incr, mask;
  mask = reg_type_mask (reg_type);

  nb_regs = 1 + (reginfo & 0x3);
  reginfo >>= 2;
  prev_regno = reginfo & 0x1f;
  incr = 1;

  list->first_regno = prev_regno;
  list->num_regs = nb_regs;

  for (i = 1; i < nb_regs; ++i)
    {
      uint32_t curr_regno, curr_incr;
      reginfo >>= 5;
      curr_regno = reginfo & 0x1f;
      curr_incr = (curr_regno - prev_regno) & mask;
      if (curr_incr == 0)
	return false;
      else if (i == 1)
	incr = curr_incr;
      else if (curr_incr != incr)
	return false;
      prev_regno = curr_regno;
    }

  list->stride = incr;
  return true;
}

/* Generic instruction operand parser.	This does no encoding and no
   semantic validation; it merely squirrels values away in the inst
   structure.  Returns TRUE or FALSE depending on whether the
   specified grammar matched.  */

static bool
parse_operands (char *str, const aarch64_opcode *opcode)
{
  int i;
  char *backtrack_pos = 0;
  const enum aarch64_opnd *operands = opcode->operands;
  aarch64_reg_type imm_reg_type;

  clear_error ();
  skip_whitespace (str);

  if (AARCH64_CPU_HAS_FEATURE (*opcode->avariant, AARCH64_FEATURE_SME2))
    imm_reg_type = REG_TYPE_R_ZR_SP_BHSDQ_VZP_PN;
  else if (AARCH64_CPU_HAS_ANY_FEATURES (*opcode->avariant,
					 AARCH64_FEATURE_SVE
					 | AARCH64_FEATURE_SVE2))
    imm_reg_type = REG_TYPE_R_ZR_SP_BHSDQ_VZP;
  else
    imm_reg_type = REG_TYPE_R_ZR_BHSDQ_V;

  for (i = 0; operands[i] != AARCH64_OPND_NIL; i++)
    {
      int64_t val;
      const reg_entry *reg;
      int comma_skipped_p = 0;
      struct vector_type_el vectype;
      aarch64_opnd_qualifier_t qualifier, base_qualifier, offset_qualifier;
      aarch64_opnd_info *info = &inst.base.operands[i];
      aarch64_reg_type reg_type;

      DEBUG_TRACE ("parse operand %d", i);

      /* Assign the operand code.  */
      info->type = operands[i];

      if (optional_operand_p (opcode, i))
	{
	  /* Remember where we are in case we need to backtrack.  */
	  gas_assert (!backtrack_pos);
	  backtrack_pos = str;
	}

      /* Expect comma between operands; the backtrack mechanism will take
	 care of cases of omitted optional operand.  */
      if (i > 0 && ! skip_past_char (&str, ','))
	{
	  set_syntax_error (_("comma expected between operands"));
	  goto failure;
	}
      else
	comma_skipped_p = 1;

      switch (operands[i])
	{
	case AARCH64_OPND_Rd:
	case AARCH64_OPND_Rn:
	case AARCH64_OPND_Rm:
	case AARCH64_OPND_Rt:
	case AARCH64_OPND_Rt2:
	case AARCH64_OPND_Rs:
	case AARCH64_OPND_Ra:
	case AARCH64_OPND_Rt_LS64:
	case AARCH64_OPND_Rt_SYS:
	case AARCH64_OPND_PAIRREG:
	case AARCH64_OPND_SVE_Rm:
	  po_int_fp_reg_or_fail (REG_TYPE_R_ZR);

	  /* In LS64 load/store instructions Rt register number must be even
	     and <=22.  */
	  if (operands[i] == AARCH64_OPND_Rt_LS64)
	  {
	    /* We've already checked if this is valid register.
	       This will check if register number (Rt) is not undefined for LS64
	       instructions:
	       if Rt<4:3> == '11' || Rt<0> == '1' then UNDEFINED.  */
	    if ((info->reg.regno & 0x18) == 0x18 || (info->reg.regno & 0x01) == 0x01)
	    {
	      set_syntax_error (_("invalid Rt register number in 64-byte load/store"));
	      goto failure;
	    }
	  }
	  break;

	case AARCH64_OPND_Rd_SP:
	case AARCH64_OPND_Rn_SP:
	case AARCH64_OPND_Rt_SP:
	case AARCH64_OPND_SVE_Rn_SP:
	case AARCH64_OPND_Rm_SP:
	  po_int_fp_reg_or_fail (REG_TYPE_R_SP);
	  break;

	case AARCH64_OPND_Rm_EXT:
	case AARCH64_OPND_Rm_SFT:
	  po_misc_or_fail (parse_shifter_operand
			   (&str, info, (operands[i] == AARCH64_OPND_Rm_EXT
					 ? SHIFTED_ARITH_IMM
					 : SHIFTED_LOGIC_IMM)));
	  if (!info->shifter.operator_present)
	    {
	      /* Default to LSL if not present.  Libopcodes prefers shifter
		 kind to be explicit.  */
	      gas_assert (info->shifter.kind == AARCH64_MOD_NONE);
	      info->shifter.kind = AARCH64_MOD_LSL;
	      /* For Rm_EXT, libopcodes will carry out further check on whether
		 or not stack pointer is used in the instruction (Recall that
		 "the extend operator is not optional unless at least one of
		 "Rd" or "Rn" is '11111' (i.e. WSP)").  */
	    }
	  break;

	case AARCH64_OPND_Fd:
	case AARCH64_OPND_Fn:
	case AARCH64_OPND_Fm:
	case AARCH64_OPND_Fa:
	case AARCH64_OPND_Ft:
	case AARCH64_OPND_Ft2:
	case AARCH64_OPND_Sd:
	case AARCH64_OPND_Sn:
	case AARCH64_OPND_Sm:
	case AARCH64_OPND_SVE_VZn:
	case AARCH64_OPND_SVE_Vd:
	case AARCH64_OPND_SVE_Vm:
	case AARCH64_OPND_SVE_Vn:
	  po_int_fp_reg_or_fail (REG_TYPE_BHSDQ);
	  break;

	case AARCH64_OPND_SVE_Pd:
	case AARCH64_OPND_SVE_Pg3:
	case AARCH64_OPND_SVE_Pg4_5:
	case AARCH64_OPND_SVE_Pg4_10:
	case AARCH64_OPND_SVE_Pg4_16:
	case AARCH64_OPND_SVE_Pm:
	case AARCH64_OPND_SVE_Pn:
	case AARCH64_OPND_SVE_Pt:
	case AARCH64_OPND_SME_Pm:
	  reg_type = REG_TYPE_P;
	  goto vector_reg;

	case AARCH64_OPND_SVE_Za_5:
	case AARCH64_OPND_SVE_Za_16:
	case AARCH64_OPND_SVE_Zd:
	case AARCH64_OPND_SVE_Zm_5:
	case AARCH64_OPND_SVE_Zm_16:
	case AARCH64_OPND_SVE_Zn:
	case AARCH64_OPND_SVE_Zt:
	case AARCH64_OPND_SME_Zm:
	  reg_type = REG_TYPE_Z;
	  goto vector_reg;

	case AARCH64_OPND_SVE_PNd:
	case AARCH64_OPND_SVE_PNg4_10:
	case AARCH64_OPND_SVE_PNn:
	case AARCH64_OPND_SVE_PNt:
	case AARCH64_OPND_SME_PNd3:
	case AARCH64_OPND_SME_PNg3:
	case AARCH64_OPND_SME_PNn:
	  reg_type = REG_TYPE_PN;
	  goto vector_reg;

	case AARCH64_OPND_Va:
	case AARCH64_OPND_Vd:
	case AARCH64_OPND_Vn:
	case AARCH64_OPND_Vm:
	  reg_type = REG_TYPE_V;
	vector_reg:
	  reg = aarch64_reg_parse (&str, reg_type, &vectype);
	  if (!reg)
	    goto failure;
	  if (vectype.defined & NTA_HASINDEX)
	    goto failure;

	  info->reg.regno = reg->number;
	  if ((reg_type == REG_TYPE_P
	       || reg_type == REG_TYPE_PN
	       || reg_type == REG_TYPE_Z)
	      && vectype.type == NT_invtype)
	    /* Unqualified P and Z registers are allowed in certain
	       contexts.  Rely on F_STRICT qualifier checking to catch
	       invalid uses.  */
	    info->qualifier = AARCH64_OPND_QLF_NIL;
	  else
	    {
	      info->qualifier = vectype_to_qualifier (&vectype);
	      if (info->qualifier == AARCH64_OPND_QLF_NIL)
		goto failure;
	    }
	  break;

	case AARCH64_OPND_VdD1:
	case AARCH64_OPND_VnD1:
	  reg = aarch64_reg_parse (&str, REG_TYPE_V, &vectype);
	  if (!reg)
	    goto failure;
	  if (vectype.type != NT_d || vectype.index != 1)
	    {
	      set_fatal_syntax_error
		(_("the top half of a 128-bit FP/SIMD register is expected"));
	      goto failure;
	    }
	  info->reg.regno = reg->number;
	  /* N.B: VdD1 and VnD1 are treated as an fp or advsimd scalar register
	     here; it is correct for the purpose of encoding/decoding since
	     only the register number is explicitly encoded in the related
	     instructions, although this appears a bit hacky.  */
	  info->qualifier = AARCH64_OPND_QLF_S_D;
	  break;

	case AARCH64_OPND_SVE_Zm3_INDEX:
	case AARCH64_OPND_SVE_Zm3_22_INDEX:
	case AARCH64_OPND_SVE_Zm3_19_INDEX:
	case AARCH64_OPND_SVE_Zm3_11_INDEX:
	case AARCH64_OPND_SVE_Zm4_11_INDEX:
	case AARCH64_OPND_SVE_Zm4_INDEX:
	case AARCH64_OPND_SVE_Zn_INDEX:
	case AARCH64_OPND_SME_Zm_INDEX1:
	case AARCH64_OPND_SME_Zm_INDEX2:
	case AARCH64_OPND_SME_Zm_INDEX3_1:
	case AARCH64_OPND_SME_Zm_INDEX3_2:
	case AARCH64_OPND_SME_Zm_INDEX3_10:
	case AARCH64_OPND_SME_Zm_INDEX4_1:
	case AARCH64_OPND_SME_Zm_INDEX4_10:
	case AARCH64_OPND_SME_Zn_INDEX1_16:
	case AARCH64_OPND_SME_Zn_INDEX2_15:
	case AARCH64_OPND_SME_Zn_INDEX2_16:
	case AARCH64_OPND_SME_Zn_INDEX3_14:
	case AARCH64_OPND_SME_Zn_INDEX3_15:
	case AARCH64_OPND_SME_Zn_INDEX4_14:
	  reg_type = REG_TYPE_Z;
	  goto vector_reg_index;

	case AARCH64_OPND_Ed:
	case AARCH64_OPND_En:
	case AARCH64_OPND_Em:
	case AARCH64_OPND_Em16:
	case AARCH64_OPND_SM3_IMM2:
	  reg_type = REG_TYPE_V;
	vector_reg_index:
	  reg = aarch64_reg_parse (&str, reg_type, &vectype);
	  if (!reg)
	    goto failure;
	  if (!(vectype.defined & NTA_HASINDEX))
	    goto failure;

	  if (reg->type == REG_TYPE_Z && vectype.type == NT_invtype)
	    /* Unqualified Zn[index] is allowed in LUTI2 instructions.  */
	    info->qualifier = AARCH64_OPND_QLF_NIL;
	  else
	    {
	      if (vectype.type == NT_invtype)
		goto failure;
	      info->qualifier = vectype_to_qualifier (&vectype);
	      if (info->qualifier == AARCH64_OPND_QLF_NIL)
		goto failure;
	    }

	  info->reglane.regno = reg->number;
	  info->reglane.index = vectype.index;
	  break;

	case AARCH64_OPND_SVE_ZnxN:
	case AARCH64_OPND_SVE_ZtxN:
	case AARCH64_OPND_SME_Zdnx2:
	case AARCH64_OPND_SME_Zdnx4:
	case AARCH64_OPND_SME_Zmx2:
	case AARCH64_OPND_SME_Zmx4:
	case AARCH64_OPND_SME_Znx2:
	case AARCH64_OPND_SME_Znx4:
	case AARCH64_OPND_SME_Ztx2_STRIDED:
	case AARCH64_OPND_SME_Ztx4_STRIDED:
	  reg_type = REG_TYPE_Z;
	  goto vector_reg_list;

	case AARCH64_OPND_SME_Pdx2:
	case AARCH64_OPND_SME_PdxN:
	  reg_type = REG_TYPE_P;
	  goto vector_reg_list;

	case AARCH64_OPND_LVn:
	case AARCH64_OPND_LVt:
	case AARCH64_OPND_LVt_AL:
	case AARCH64_OPND_LEt:
	  reg_type = REG_TYPE_V;
	vector_reg_list:
	  if (reg_type == REG_TYPE_Z
	      && get_opcode_dependent_value (opcode) == 1
	      && *str != '{')
	    {
	      reg = aarch64_reg_parse (&str, reg_type, &vectype);
	      if (!reg)
		goto failure;
	      info->reglist.first_regno = reg->number;
	      info->reglist.num_regs = 1;
	      info->reglist.stride = 1;
	    }
	  else
	    {
	      val = parse_vector_reg_list (&str, reg_type, &vectype);
	      if (val == PARSE_FAIL)
		goto failure;

	      if (! reg_list_valid_p (val, &info->reglist, reg_type))
		{
		  set_fatal_syntax_error (_("invalid register list"));
		  goto failure;
		}

	      if ((int) vectype.width > 0 && *str != ',')
		{
		  set_fatal_syntax_error
		    (_("expected element type rather than vector type"));
		  goto failure;
		}
	    }
	  if (operands[i] == AARCH64_OPND_LEt)
	    {
	      if (!(vectype.defined & NTA_HASINDEX))
		goto failure;
	      info->reglist.has_index = 1;
	      info->reglist.index = vectype.index;
	    }
	  else
	    {
	      if (vectype.defined & NTA_HASINDEX)
		goto failure;
	      if (!(vectype.defined & NTA_HASTYPE))
		{
		  if (reg_type == REG_TYPE_Z || reg_type == REG_TYPE_P)
		    set_fatal_syntax_error (_("missing type suffix"));
		  goto failure;
		}
	    }
	  info->qualifier = vectype_to_qualifier (&vectype);
	  if (info->qualifier == AARCH64_OPND_QLF_NIL)
	    goto failure;
	  break;

	case AARCH64_OPND_CRn:
	case AARCH64_OPND_CRm:
	    {
	      char prefix = *(str++);
	      if (prefix != 'c' && prefix != 'C')
		goto failure;

	      po_imm_nc_or_fail ();
	      if (val > 15)
		{
		  set_fatal_syntax_error (_(N_ ("C0 - C15 expected")));
		  goto failure;
		}
	      info->qualifier = AARCH64_OPND_QLF_CR;
	      info->imm.value = val;
	      break;
	    }

	case AARCH64_OPND_SHLL_IMM:
	case AARCH64_OPND_IMM_VLSR:
	  po_imm_or_fail (1, 64);
	  info->imm.value = val;
	  break;

	case AARCH64_OPND_CCMP_IMM:
	case AARCH64_OPND_SIMM5:
	case AARCH64_OPND_FBITS:
	case AARCH64_OPND_TME_UIMM16:
	case AARCH64_OPND_UIMM4:
	case AARCH64_OPND_UIMM4_ADDG:
	case AARCH64_OPND_UIMM10:
	case AARCH64_OPND_UIMM3_OP1:
	case AARCH64_OPND_UIMM3_OP2:
	case AARCH64_OPND_IMM_VLSL:
	case AARCH64_OPND_IMM:
	case AARCH64_OPND_IMM_2:
	case AARCH64_OPND_WIDTH:
	case AARCH64_OPND_SVE_INV_LIMM:
	case AARCH64_OPND_SVE_LIMM:
	case AARCH64_OPND_SVE_LIMM_MOV:
	case AARCH64_OPND_SVE_SHLIMM_PRED:
	case AARCH64_OPND_SVE_SHLIMM_UNPRED:
	case AARCH64_OPND_SVE_SHLIMM_UNPRED_22:
	case AARCH64_OPND_SME_SHRIMM4:
	case AARCH64_OPND_SME_SHRIMM5:
	case AARCH64_OPND_SVE_SHRIMM_PRED:
	case AARCH64_OPND_SVE_SHRIMM_UNPRED:
	case AARCH64_OPND_SVE_SHRIMM_UNPRED_22:
	case AARCH64_OPND_SVE_SIMM5:
	case AARCH64_OPND_SVE_SIMM5B:
	case AARCH64_OPND_SVE_SIMM6:
	case AARCH64_OPND_SVE_SIMM8:
	case AARCH64_OPND_SVE_UIMM3:
	case AARCH64_OPND_SVE_UIMM7:
	case AARCH64_OPND_SVE_UIMM8:
	case AARCH64_OPND_SVE_UIMM8_53:
	case AARCH64_OPND_IMM_ROT1:
	case AARCH64_OPND_IMM_ROT2:
	case AARCH64_OPND_IMM_ROT3:
	case AARCH64_OPND_SVE_IMM_ROT1:
	case AARCH64_OPND_SVE_IMM_ROT2:
	case AARCH64_OPND_SVE_IMM_ROT3:
	case AARCH64_OPND_CSSC_SIMM8:
	case AARCH64_OPND_CSSC_UIMM8:
	  po_imm_nc_or_fail ();
	  info->imm.value = val;
	  break;

	case AARCH64_OPND_SVE_AIMM:
	case AARCH64_OPND_SVE_ASIMM:
	  po_imm_nc_or_fail ();
	  info->imm.value = val;
	  skip_whitespace (str);
	  if (skip_past_comma (&str))
	    po_misc_or_fail (parse_shift (&str, info, SHIFTED_LSL));
	  else
	    inst.base.operands[i].shifter.kind = AARCH64_MOD_LSL;
	  break;

	case AARCH64_OPND_SVE_PATTERN:
	  po_enum_or_fail (aarch64_sve_pattern_array);
	  info->imm.value = val;
	  break;

	case AARCH64_OPND_SVE_PATTERN_SCALED:
	  po_enum_or_fail (aarch64_sve_pattern_array);
	  info->imm.value = val;
	  if (skip_past_comma (&str)
	      && !parse_shift (&str, info, SHIFTED_MUL))
	    goto failure;
	  if (!info->shifter.operator_present)
	    {
	      gas_assert (info->shifter.kind == AARCH64_MOD_NONE);
	      info->shifter.kind = AARCH64_MOD_MUL;
	      info->shifter.amount = 1;
	    }
	  break;

	case AARCH64_OPND_SVE_PRFOP:
	  po_enum_or_fail (aarch64_sve_prfop_array);
	  info->imm.value = val;
	  break;

	case AARCH64_OPND_UIMM7:
	  po_imm_or_fail (0, 127);
	  info->imm.value = val;
	  break;

	case AARCH64_OPND_IDX:
	case AARCH64_OPND_MASK:
	case AARCH64_OPND_BIT_NUM:
	case AARCH64_OPND_IMMR:
	case AARCH64_OPND_IMMS:
	  po_imm_or_fail (0, 63);
	  info->imm.value = val;
	  break;

	case AARCH64_OPND_IMM0:
	  po_imm_nc_or_fail ();
	  if (val != 0)
	    {
	      set_fatal_syntax_error (_("immediate zero expected"));
	      goto failure;
	    }
	  info->imm.value = 0;
	  break;

	case AARCH64_OPND_FPIMM0:
	  {
	    int qfloat;
	    bool res1 = false, res2 = false;
	    /* N.B. -0.0 will be rejected; although -0.0 shouldn't be rejected,
	       it is probably not worth the effort to support it.  */
	    if (!(res1 = parse_aarch64_imm_float (&str, &qfloat, false,
						  imm_reg_type))
		&& (error_p ()
		    || !(res2 = parse_constant_immediate (&str, &val,
							  imm_reg_type))))
	      goto failure;
	    if ((res1 && qfloat == 0) || (res2 && val == 0))
	      {
		info->imm.value = 0;
		info->imm.is_fp = 1;
		break;
	      }
	    set_fatal_syntax_error (_("immediate zero expected"));
	    goto failure;
	  }

	case AARCH64_OPND_IMM_MOV:
	  {
	    char *saved = str;
	    if (reg_name_p (str, REG_TYPE_R_ZR_SP)
		|| reg_name_p (str, REG_TYPE_V))
	      goto failure;
	    str = saved;
	    po_misc_or_fail (aarch64_get_expression (&inst.reloc.exp, &str,
						     GE_OPT_PREFIX, REJECT_ABSENT));
	    /* The MOV immediate alias will be fixed up by fix_mov_imm_insn
	       later.  fix_mov_imm_insn will try to determine a machine
	       instruction (MOVZ, MOVN or ORR) for it and will issue an error
	       message if the immediate cannot be moved by a single
	       instruction.  */
	    aarch64_set_gas_internal_fixup (&inst.reloc, info, 1);
	    inst.base.operands[i].skip = 1;
	  }
	  break;

	case AARCH64_OPND_SIMD_IMM:
	case AARCH64_OPND_SIMD_IMM_SFT:
	  if (! parse_big_immediate (&str, &val, imm_reg_type))
	    goto failure;
	  assign_imm_if_const_or_fixup_later (&inst.reloc, info,
					      /* addr_off_p */ 0,
					      /* need_libopcodes_p */ 1,
					      /* skip_p */ 1);
	  /* Parse shift.
	     N.B. although AARCH64_OPND_SIMD_IMM doesn't permit any
	     shift, we don't check it here; we leave the checking to
	     the libopcodes (operand_general_constraint_met_p).  By
	     doing this, we achieve better diagnostics.  */
	  if (skip_past_comma (&str)
	      && ! parse_shift (&str, info, SHIFTED_LSL_MSL))
	    goto failure;
	  if (!info->shifter.operator_present
	      && info->type == AARCH64_OPND_SIMD_IMM_SFT)
	    {
	      /* Default to LSL if not present.  Libopcodes prefers shifter
		 kind to be explicit.  */
	      gas_assert (info->shifter.kind == AARCH64_MOD_NONE);
	      info->shifter.kind = AARCH64_MOD_LSL;
	    }
	  break;

	case AARCH64_OPND_FPIMM:
	case AARCH64_OPND_SIMD_FPIMM:
	case AARCH64_OPND_SVE_FPIMM8:
	  {
	    int qfloat;
	    bool dp_p;

	    dp_p = double_precision_operand_p (&inst.base.operands[0]);
	    if (!parse_aarch64_imm_float (&str, &qfloat, dp_p, imm_reg_type)
		|| !aarch64_imm_float_p (qfloat))
	      {
		if (!error_p ())
		  set_fatal_syntax_error (_("invalid floating-point"
					    " constant"));
		goto failure;
	      }
	    inst.base.operands[i].imm.value = encode_imm_float_bits (qfloat);
	    inst.base.operands[i].imm.is_fp = 1;
	  }
	  break;

	case AARCH64_OPND_SVE_I1_HALF_ONE:
	case AARCH64_OPND_SVE_I1_HALF_TWO:
	case AARCH64_OPND_SVE_I1_ZERO_ONE:
	  {
	    int qfloat;
	    bool dp_p;

	    dp_p = double_precision_operand_p (&inst.base.operands[0]);
	    if (!parse_aarch64_imm_float (&str, &qfloat, dp_p, imm_reg_type))
	      {
		if (!error_p ())
		  set_fatal_syntax_error (_("invalid floating-point"
					    " constant"));
		goto failure;
	      }
	    inst.base.operands[i].imm.value = qfloat;
	    inst.base.operands[i].imm.is_fp = 1;
	  }
	  break;

	case AARCH64_OPND_LIMM:
	  po_misc_or_fail (parse_shifter_operand (&str, info,
						  SHIFTED_LOGIC_IMM));
	  if (info->shifter.operator_present)
	    {
	      set_fatal_syntax_error
		(_("shift not allowed for bitmask immediate"));
	      goto failure;
	    }
	  assign_imm_if_const_or_fixup_later (&inst.reloc, info,
					      /* addr_off_p */ 0,
					      /* need_libopcodes_p */ 1,
					      /* skip_p */ 1);
	  break;

	case AARCH64_OPND_AIMM:
	  if (opcode->op == OP_ADD)
	    /* ADD may have relocation types.  */
	    po_misc_or_fail (parse_shifter_operand_reloc (&str, info,
							  SHIFTED_ARITH_IMM));
	  else
	    po_misc_or_fail (parse_shifter_operand (&str, info,
						    SHIFTED_ARITH_IMM));
	  switch (inst.reloc.type)
	    {
	    case BFD_RELOC_AARCH64_TLSLE_ADD_TPREL_HI12:
	      info->shifter.amount = 12;
	      break;
	    case BFD_RELOC_UNUSED:
	      aarch64_set_gas_internal_fixup (&inst.reloc, info, 0);
	      if (info->shifter.kind != AARCH64_MOD_NONE)
		inst.reloc.flags = FIXUP_F_HAS_EXPLICIT_SHIFT;
	      inst.reloc.pc_rel = 0;
	      break;
	    default:
	      break;
	    }
	  info->imm.value = 0;
	  if (!info->shifter.operator_present)
	    {
	      /* Default to LSL if not present.  Libopcodes prefers shifter
		 kind to be explicit.  */
	      gas_assert (info->shifter.kind == AARCH64_MOD_NONE);
	      info->shifter.kind = AARCH64_MOD_LSL;
	    }
	  break;

	case AARCH64_OPND_HALF:
	    {
	      /* #<imm16> or relocation.  */
	      int internal_fixup_p;
	      po_misc_or_fail (parse_half (&str, &internal_fixup_p));
	      if (internal_fixup_p)
		aarch64_set_gas_internal_fixup (&inst.reloc, info, 0);
	      skip_whitespace (str);
	      if (skip_past_comma (&str))
		{
		  /* {, LSL #<shift>}  */
		  if (! aarch64_gas_internal_fixup_p ())
		    {
		      set_fatal_syntax_error (_("can't mix relocation modifier "
						"with explicit shift"));
		      goto failure;
		    }
		  po_misc_or_fail (parse_shift (&str, info, SHIFTED_LSL));
		}
	      else
		inst.base.operands[i].shifter.amount = 0;
	      inst.base.operands[i].shifter.kind = AARCH64_MOD_LSL;
	      inst.base.operands[i].imm.value = 0;
	      if (! process_movw_reloc_info ())
		goto failure;
	    }
	  break;

	case AARCH64_OPND_EXCEPTION:
	case AARCH64_OPND_UNDEFINED:
	  po_misc_or_fail (parse_immediate_expression (&str, &inst.reloc.exp,
						       imm_reg_type));
	  assign_imm_if_const_or_fixup_later (&inst.reloc, info,
					      /* addr_off_p */ 0,
					      /* need_libopcodes_p */ 0,
					      /* skip_p */ 1);
	  break;

	case AARCH64_OPND_NZCV:
	  {
	    const asm_nzcv *nzcv = str_hash_find_n (aarch64_nzcv_hsh, str, 4);
	    if (nzcv != NULL)
	      {
		str += 4;
		info->imm.value = nzcv->value;
		break;
	      }
	    po_imm_or_fail (0, 15);
	    info->imm.value = val;
	  }
	  break;

	case AARCH64_OPND_COND:
	case AARCH64_OPND_COND1:
	  {
	    char *start = str;
	    do
	      str++;
	    while (ISALPHA (*str));
	    info->cond = str_hash_find_n (aarch64_cond_hsh, start, str - start);
	    if (info->cond == NULL)
	      {
		set_syntax_error (_("invalid condition"));
		goto failure;
	      }
	    else if (operands[i] == AARCH64_OPND_COND1
		     && (info->cond->value & 0xe) == 0xe)
	      {
		/* Do not allow AL or NV.  */
		set_default_error ();
		goto failure;
	      }
	  }
	  break;

	case AARCH64_OPND_ADDR_ADRP:
	  po_misc_or_fail (parse_adrp (&str));
	  /* Clear the value as operand needs to be relocated.  */
	  info->imm.value = 0;
	  break;

	case AARCH64_OPND_ADDR_PCREL14:
	case AARCH64_OPND_ADDR_PCREL19:
	case AARCH64_OPND_ADDR_PCREL21:
	case AARCH64_OPND_ADDR_PCREL26:
	  po_misc_or_fail (parse_address (&str, info));
	  if (!info->addr.pcrel)
	    {
	      set_syntax_error (_("invalid pc-relative address"));
	      goto failure;
	    }
	  if (inst.gen_lit_pool
	      && (opcode->iclass != loadlit || opcode->op == OP_PRFM_LIT))
	    {
	      /* Only permit "=value" in the literal load instructions.
		 The literal will be generated by programmer_friendly_fixup.  */
	      set_syntax_error (_("invalid use of \"=immediate\""));
	      goto failure;
	    }
	  if (inst.reloc.exp.X_op == O_symbol && find_reloc_table_entry (&str))
	    {
	      set_syntax_error (_("unrecognized relocation suffix"));
	      goto failure;
	    }
	  if (inst.reloc.exp.X_op == O_constant && !inst.gen_lit_pool)
	    {
	      info->imm.value = inst.reloc.exp.X_add_number;
	      inst.reloc.type = BFD_RELOC_UNUSED;
	    }
	  else
	    {
	      info->imm.value = 0;
	      if (inst.reloc.type == BFD_RELOC_UNUSED)
		switch (opcode->iclass)
		  {
		  case compbranch:
		  case condbranch:
		    /* e.g. CBZ or B.COND  */
		    gas_assert (operands[i] == AARCH64_OPND_ADDR_PCREL19);
		    inst.reloc.type = BFD_RELOC_AARCH64_BRANCH19;
		    break;
		  case testbranch:
		    /* e.g. TBZ  */
		    gas_assert (operands[i] == AARCH64_OPND_ADDR_PCREL14);
		    inst.reloc.type = BFD_RELOC_AARCH64_TSTBR14;
		    break;
		  case branch_imm:
		    /* e.g. B or BL  */
		    gas_assert (operands[i] == AARCH64_OPND_ADDR_PCREL26);
		    inst.reloc.type =
		      (opcode->op == OP_BL) ? BFD_RELOC_AARCH64_CALL26
			 : BFD_RELOC_AARCH64_JUMP26;
		    break;
		  case loadlit:
		    gas_assert (operands[i] == AARCH64_OPND_ADDR_PCREL19);
		    inst.reloc.type = BFD_RELOC_AARCH64_LD_LO19_PCREL;
		    break;
		  case pcreladdr:
		    gas_assert (operands[i] == AARCH64_OPND_ADDR_PCREL21);
		    inst.reloc.type = BFD_RELOC_AARCH64_ADR_LO21_PCREL;
		    break;
		  default:
		    gas_assert (0);
		    abort ();
		  }
	      inst.reloc.pc_rel = 1;
	    }
	  break;

	case AARCH64_OPND_ADDR_SIMPLE:
	case AARCH64_OPND_SIMD_ADDR_SIMPLE:
	  {
	    /* [<Xn|SP>{, #<simm>}]  */
	    char *start = str;
	    /* First use the normal address-parsing routines, to get
	       the usual syntax errors.  */
	    po_misc_or_fail (parse_address (&str, info));
	    if (info->addr.pcrel || info->addr.offset.is_reg
		|| !info->addr.preind || info->addr.postind
		|| info->addr.writeback)
	      {
		set_syntax_error (_("invalid addressing mode"));
		goto failure;
	      }

	    /* Then retry, matching the specific syntax of these addresses.  */
	    str = start;
	    po_char_or_fail ('[');
	    po_reg_or_fail (REG_TYPE_R64_SP);
	    /* Accept optional ", #0".  */
	    if (operands[i] == AARCH64_OPND_ADDR_SIMPLE
		&& skip_past_char (&str, ','))
	      {
		skip_past_char (&str, '#');
		if (! skip_past_char (&str, '0'))
		  {
		    set_fatal_syntax_error
		      (_("the optional immediate offset can only be 0"));
		    goto failure;
		  }
	      }
	    po_char_or_fail (']');
	    break;
	  }

	case AARCH64_OPND_ADDR_REGOFF:
	  /* [<Xn|SP>, <R><m>{, <extend> {<amount>}}]  */
	  po_misc_or_fail (parse_address (&str, info));
	regoff_addr:
	  if (info->addr.pcrel || !info->addr.offset.is_reg
	      || !info->addr.preind || info->addr.postind
	      || info->addr.writeback)
	    {
	      set_syntax_error (_("invalid addressing mode"));
	      goto failure;
	    }
	  if (!info->shifter.operator_present)
	    {
	      /* Default to LSL if not present.  Libopcodes prefers shifter
		 kind to be explicit.  */
	      gas_assert (info->shifter.kind == AARCH64_MOD_NONE);
	      info->shifter.kind = AARCH64_MOD_LSL;
	    }
	  /* Qualifier to be deduced by libopcodes.  */
	  break;

	case AARCH64_OPND_ADDR_SIMM7:
	  po_misc_or_fail (parse_address (&str, info));
	  if (info->addr.pcrel || info->addr.offset.is_reg
	      || (!info->addr.preind && !info->addr.postind))
	    {
	      set_syntax_error (_("invalid addressing mode"));
	      goto failure;
	    }
	  if (inst.reloc.type != BFD_RELOC_UNUSED)
	    {
	      set_syntax_error (_("relocation not allowed"));
	      goto failure;
	    }
	  assign_imm_if_const_or_fixup_later (&inst.reloc, info,
					      /* addr_off_p */ 1,
					      /* need_libopcodes_p */ 1,
					      /* skip_p */ 0);
	  break;

	case AARCH64_OPND_ADDR_SIMM9:
	case AARCH64_OPND_ADDR_SIMM9_2:
	case AARCH64_OPND_ADDR_SIMM11:
	case AARCH64_OPND_ADDR_SIMM13:
	  po_misc_or_fail (parse_address (&str, info));
	  if (info->addr.pcrel || info->addr.offset.is_reg
	      || (!info->addr.preind && !info->addr.postind)
	      || (operands[i] == AARCH64_OPND_ADDR_SIMM9_2
		  && info->addr.writeback))
	    {
	      set_syntax_error (_("invalid addressing mode"));
	      goto failure;
	    }
	  if (inst.reloc.type != BFD_RELOC_UNUSED)
	    {
	      set_syntax_error (_("relocation not allowed"));
	      goto failure;
	    }
	  assign_imm_if_const_or_fixup_later (&inst.reloc, info,
					      /* addr_off_p */ 1,
					      /* need_libopcodes_p */ 1,
					      /* skip_p */ 0);
	  break;

	case AARCH64_OPND_ADDR_SIMM10:
	case AARCH64_OPND_ADDR_OFFSET:
	  po_misc_or_fail (parse_address (&str, info));
	  if (info->addr.pcrel || info->addr.offset.is_reg
	      || !info->addr.preind || info->addr.postind)
	    {
	      set_syntax_error (_("invalid addressing mode"));
	      goto failure;
	    }
	  if (inst.reloc.type != BFD_RELOC_UNUSED)
	    {
	      set_syntax_error (_("relocation not allowed"));
	      goto failure;
	    }
	  assign_imm_if_const_or_fixup_later (&inst.reloc, info,
					      /* addr_off_p */ 1,
					      /* need_libopcodes_p */ 1,
					      /* skip_p */ 0);
	  break;

	case AARCH64_OPND_ADDR_UIMM12:
	  po_misc_or_fail (parse_address (&str, info));
	  if (info->addr.pcrel || info->addr.offset.is_reg
	      || !info->addr.preind || info->addr.writeback)
	    {
	      set_syntax_error (_("invalid addressing mode"));
	      goto failure;
	    }
	  if (inst.reloc.type == BFD_RELOC_UNUSED)
	    aarch64_set_gas_internal_fixup (&inst.reloc, info, 1);
	  else if (inst.reloc.type == BFD_RELOC_AARCH64_LDST_LO12
		   || (inst.reloc.type
		       == BFD_RELOC_AARCH64_TLSLD_LDST_DTPREL_LO12)
		   || (inst.reloc.type
		       == BFD_RELOC_AARCH64_TLSLD_LDST_DTPREL_LO12_NC)
		   || (inst.reloc.type
		       == BFD_RELOC_AARCH64_TLSLE_LDST_TPREL_LO12)
		   || (inst.reloc.type
		       == BFD_RELOC_AARCH64_TLSLE_LDST_TPREL_LO12_NC))
	    inst.reloc.type = ldst_lo12_determine_real_reloc_type ();
	  /* Leave qualifier to be determined by libopcodes.  */
	  break;

	case AARCH64_OPND_SIMD_ADDR_POST:
	  /* [<Xn|SP>], <Xm|#<amount>>  */
	  po_misc_or_fail (parse_address (&str, info));
	  if (!info->addr.postind || !info->addr.writeback)
	    {
	      set_syntax_error (_("invalid addressing mode"));
	      goto failure;
	    }
	  if (!info->addr.offset.is_reg)
	    {
	      if (inst.reloc.exp.X_op == O_constant)
		info->addr.offset.imm = inst.reloc.exp.X_add_number;
	      else
		{
		  set_fatal_syntax_error
		    (_("writeback value must be an immediate constant"));
		  goto failure;
		}
	    }
	  /* No qualifier.  */
	  break;

	case AARCH64_OPND_SME_SM_ZA:
	  /* { SM | ZA }  */
	  if ((val = parse_sme_sm_za (&str)) == PARSE_FAIL)
	    {
	      set_syntax_error (_("unknown or missing PSTATE field name"));
	      goto failure;
	    }
	  info->reg.regno = val;
	  break;

	case AARCH64_OPND_SME_PnT_Wm_imm:
	  if (!parse_dual_indexed_reg (&str, REG_TYPE_P,
				       &info->indexed_za, &qualifier, 0))
	    goto failure;
	  info->qualifier = qualifier;
	  break;

	case AARCH64_OPND_SVE_ADDR_RI_S4x16:
	case AARCH64_OPND_SVE_ADDR_RI_S4x32:
	case AARCH64_OPND_SVE_ADDR_RI_S4xVL:
	case AARCH64_OPND_SME_ADDR_RI_U4xVL:
	case AARCH64_OPND_SVE_ADDR_RI_S4x2xVL:
	case AARCH64_OPND_SVE_ADDR_RI_S4x3xVL:
	case AARCH64_OPND_SVE_ADDR_RI_S4x4xVL:
	case AARCH64_OPND_SVE_ADDR_RI_S6xVL:
	case AARCH64_OPND_SVE_ADDR_RI_S9xVL:
	case AARCH64_OPND_SVE_ADDR_RI_U6:
	case AARCH64_OPND_SVE_ADDR_RI_U6x2:
	case AARCH64_OPND_SVE_ADDR_RI_U6x4:
	case AARCH64_OPND_SVE_ADDR_RI_U6x8:
	  /* [X<n>{, #imm, MUL VL}]
	     [X<n>{, #imm}]
	     but recognizing SVE registers.  */
	  po_misc_or_fail (parse_sve_address (&str, info, &base_qualifier,
					      &offset_qualifier));
	  if (base_qualifier != AARCH64_OPND_QLF_X)
	    {
	      set_syntax_error (_("invalid addressing mode"));
	      goto failure;
	    }
	sve_regimm:
	  if (info->addr.pcrel || info->addr.offset.is_reg
	      || !info->addr.preind || info->addr.writeback)
	    {
	      set_syntax_error (_("invalid addressing mode"));
	      goto failure;
	    }
	  if (inst.reloc.type != BFD_RELOC_UNUSED
	      || inst.reloc.exp.X_op != O_constant)
	    {
	      /* Make sure this has priority over
		 "invalid addressing mode".  */
	      set_fatal_syntax_error (_("constant offset required"));
	      goto failure;
	    }
	  info->addr.offset.imm = inst.reloc.exp.X_add_number;
	  break;

	case AARCH64_OPND_SVE_ADDR_R:
	  /* [<Xn|SP>{, <R><m>}]
	     but recognizing SVE registers.  */
	  po_misc_or_fail (parse_sve_address (&str, info, &base_qualifier,
					      &offset_qualifier));
	  if (offset_qualifier == AARCH64_OPND_QLF_NIL)
	    {
	      offset_qualifier = AARCH64_OPND_QLF_X;
	      info->addr.offset.is_reg = 1;
	      info->addr.offset.regno = 31;
	    }
	  else if (base_qualifier != AARCH64_OPND_QLF_X
	      || offset_qualifier != AARCH64_OPND_QLF_X)
	    {
	      set_syntax_error (_("invalid addressing mode"));
	      goto failure;
	    }
	  goto regoff_addr;

	case AARCH64_OPND_SVE_ADDR_RR:
	case AARCH64_OPND_SVE_ADDR_RR_LSL1:
	case AARCH64_OPND_SVE_ADDR_RR_LSL2:
	case AARCH64_OPND_SVE_ADDR_RR_LSL3:
	case AARCH64_OPND_SVE_ADDR_RR_LSL4:
	case AARCH64_OPND_SVE_ADDR_RX:
	case AARCH64_OPND_SVE_ADDR_RX_LSL1:
	case AARCH64_OPND_SVE_ADDR_RX_LSL2:
	case AARCH64_OPND_SVE_ADDR_RX_LSL3:
	  /* [<Xn|SP>, <R><m>{, lsl #<amount>}]
	     but recognizing SVE registers.  */
	  po_misc_or_fail (parse_sve_address (&str, info, &base_qualifier,
					      &offset_qualifier));
	  if (base_qualifier != AARCH64_OPND_QLF_X
	      || offset_qualifier != AARCH64_OPND_QLF_X)
	    {
	      set_syntax_error (_("invalid addressing mode"));
	      goto failure;
	    }
	  goto regoff_addr;

	case AARCH64_OPND_SVE_ADDR_RZ:
	case AARCH64_OPND_SVE_ADDR_RZ_LSL1:
	case AARCH64_OPND_SVE_ADDR_RZ_LSL2:
	case AARCH64_OPND_SVE_ADDR_RZ_LSL3:
	case AARCH64_OPND_SVE_ADDR_RZ_XTW_14:
	case AARCH64_OPND_SVE_ADDR_RZ_XTW_22:
	case AARCH64_OPND_SVE_ADDR_RZ_XTW1_14:
	case AARCH64_OPND_SVE_ADDR_RZ_XTW1_22:
	case AARCH64_OPND_SVE_ADDR_RZ_XTW2_14:
	case AARCH64_OPND_SVE_ADDR_RZ_XTW2_22:
	case AARCH64_OPND_SVE_ADDR_RZ_XTW3_14:
	case AARCH64_OPND_SVE_ADDR_RZ_XTW3_22:
	  /* [<Xn|SP>, Z<m>.D{, LSL #<amount>}]
	     [<Xn|SP>, Z<m>.<T>, <extend> {#<amount>}]  */
	  po_misc_or_fail (parse_sve_address (&str, info, &base_qualifier,
					      &offset_qualifier));
	  if (base_qualifier != AARCH64_OPND_QLF_X
	      || (offset_qualifier != AARCH64_OPND_QLF_S_S
		  && offset_qualifier != AARCH64_OPND_QLF_S_D))
	    {
	      set_syntax_error (_("invalid addressing mode"));
	      goto failure;
	    }
	  info->qualifier = offset_qualifier;
	  goto regoff_addr;

	case AARCH64_OPND_SVE_ADDR_ZX:
	  /* [Zn.<T>{, <Xm>}].  */
	  po_misc_or_fail (parse_sve_address (&str, info, &base_qualifier,
					      &offset_qualifier));
	  /* Things to check:
	      base_qualifier either S_S or S_D
	      offset_qualifier must be X
	      */
	  if ((base_qualifier != AARCH64_OPND_QLF_S_S
	       && base_qualifier != AARCH64_OPND_QLF_S_D)
	      || offset_qualifier != AARCH64_OPND_QLF_X)
	    {
	      set_syntax_error (_("invalid addressing mode"));
	      goto failure;
	    }
	  info->qualifier = base_qualifier;
	  if (!info->addr.offset.is_reg || info->addr.pcrel
	      || !info->addr.preind || info->addr.writeback
	      || info->shifter.operator_present != 0)
	    {
	      set_syntax_error (_("invalid addressing mode"));
	      goto failure;
	    }
	  info->shifter.kind = AARCH64_MOD_LSL;
	  break;


	case AARCH64_OPND_SVE_ADDR_ZI_U5:
	case AARCH64_OPND_SVE_ADDR_ZI_U5x2:
	case AARCH64_OPND_SVE_ADDR_ZI_U5x4:
	case AARCH64_OPND_SVE_ADDR_ZI_U5x8:
	  /* [Z<n>.<T>{, #imm}]  */
	  po_misc_or_fail (parse_sve_address (&str, info, &base_qualifier,
					      &offset_qualifier));
	  if (base_qualifier != AARCH64_OPND_QLF_S_S
	      && base_qualifier != AARCH64_OPND_QLF_S_D)
	    {
	      set_syntax_error (_("invalid addressing mode"));
	      goto failure;
	    }
	  info->qualifier = base_qualifier;
	  goto sve_regimm;

	case AARCH64_OPND_SVE_ADDR_ZZ_LSL:
	case AARCH64_OPND_SVE_ADDR_ZZ_SXTW:
	case AARCH64_OPND_SVE_ADDR_ZZ_UXTW:
	  /* [Z<n>.<T>, Z<m>.<T>{, LSL #<amount>}]
	     [Z<n>.D, Z<m>.D, <extend> {#<amount>}]

	     We don't reject:

	     [Z<n>.S, Z<m>.S, <extend> {#<amount>}]

	     here since we get better error messages by leaving it to
	     the qualifier checking routines.  */
	  po_misc_or_fail (parse_sve_address (&str, info, &base_qualifier,
					      &offset_qualifier));
	  if ((base_qualifier != AARCH64_OPND_QLF_S_S
	       && base_qualifier != AARCH64_OPND_QLF_S_D)
	      || offset_qualifier != base_qualifier)
	    {
	      set_syntax_error (_("invalid addressing mode"));
	      goto failure;
	    }
	  info->qualifier = base_qualifier;
	  goto regoff_addr;

	case AARCH64_OPND_SYSREG:
	  {
	    uint32_t sysreg_flags;
	    if ((val = parse_sys_reg (&str, aarch64_sys_regs_hsh, 1, 0,
				      &sysreg_flags)) == PARSE_FAIL)
	      {
		set_syntax_error (_("unknown or missing system register name"));
		goto failure;
	      }
	    inst.base.operands[i].sysreg.value = val;
	    inst.base.operands[i].sysreg.flags = sysreg_flags;
	    break;
	  }

	case AARCH64_OPND_PSTATEFIELD:
	  {
	    uint32_t sysreg_flags;
	    if ((val = parse_sys_reg (&str, aarch64_pstatefield_hsh, 0, 1,
				      &sysreg_flags)) == PARSE_FAIL)
	      {
	        set_syntax_error (_("unknown or missing PSTATE field name"));
	        goto failure;
	      }
	    inst.base.operands[i].pstatefield = val;
	    inst.base.operands[i].sysreg.flags = sysreg_flags;
	    break;
	  }

	case AARCH64_OPND_SYSREG_IC:
	  inst.base.operands[i].sysins_op =
	    parse_sys_ins_reg (&str, aarch64_sys_regs_ic_hsh);
	  goto sys_reg_ins;

	case AARCH64_OPND_SYSREG_DC:
	  inst.base.operands[i].sysins_op =
	    parse_sys_ins_reg (&str, aarch64_sys_regs_dc_hsh);
	  goto sys_reg_ins;

	case AARCH64_OPND_SYSREG_AT:
	  inst.base.operands[i].sysins_op =
	    parse_sys_ins_reg (&str, aarch64_sys_regs_at_hsh);
	  goto sys_reg_ins;

	case AARCH64_OPND_SYSREG_SR:
	  inst.base.operands[i].sysins_op =
	    parse_sys_ins_reg (&str, aarch64_sys_regs_sr_hsh);
	  goto sys_reg_ins;

	case AARCH64_OPND_SYSREG_TLBI:
	  inst.base.operands[i].sysins_op =
	    parse_sys_ins_reg (&str, aarch64_sys_regs_tlbi_hsh);
	sys_reg_ins:
	  if (inst.base.operands[i].sysins_op == NULL)
	    {
	      set_fatal_syntax_error ( _("unknown or missing operation name"));
	      goto failure;
	    }
	  break;

	case AARCH64_OPND_BARRIER:
	case AARCH64_OPND_BARRIER_ISB:
	  val = parse_barrier (&str);
	  if (val != PARSE_FAIL
	      && operands[i] == AARCH64_OPND_BARRIER_ISB && val != 0xf)
	    {
	      /* ISB only accepts options name 'sy'.  */
	      set_syntax_error
		(_("the specified option is not accepted in ISB"));
	      /* Turn off backtrack as this optional operand is present.  */
	      backtrack_pos = 0;
	      goto failure;
	    }
	  if (val != PARSE_FAIL
	      && operands[i] == AARCH64_OPND_BARRIER)
	    {
	      /* Regular barriers accept options CRm (C0-C15).
	         DSB nXS barrier variant accepts values > 15.  */
	      if (val < 0 || val > 15)
	      {
	        set_syntax_error (_("the specified option is not accepted in DSB"));
	        goto failure;
	      }
	    }
	  /* This is an extension to accept a 0..15 immediate.  */
	  if (val == PARSE_FAIL)
	    po_imm_or_fail (0, 15);
	  info->barrier = aarch64_barrier_options + val;
	  break;

	case AARCH64_OPND_BARRIER_DSB_NXS:
	  val = parse_barrier (&str);
	  if (val != PARSE_FAIL)
	    {
	      /* DSB nXS barrier variant accept only <option>nXS qualifiers.  */
	      if (!(val == 16 || val == 20 || val == 24 || val == 28))
	        {
	          set_syntax_error (_("the specified option is not accepted in DSB"));
	          /* Turn off backtrack as this optional operand is present.  */
	          backtrack_pos = 0;
	          goto failure;
	        }
	    }
	  else
	    {
	      /* DSB nXS barrier variant accept 5-bit unsigned immediate, with
	         possible values 16, 20, 24 or 28 , encoded as val<3:2>.  */
	      if (! parse_constant_immediate (&str, &val, imm_reg_type))
	        goto failure;
	      if (!(val == 16 || val == 20 || val == 24 || val == 28))
	        {
	          set_syntax_error (_("immediate value must be 16, 20, 24, 28"));
	          goto failure;
	        }
	    }
	  /* Option index is encoded as 2-bit value in val<3:2>.  */
	  val = (val >> 2) - 4;
	  info->barrier = aarch64_barrier_dsb_nxs_options + val;
	  break;

	case AARCH64_OPND_PRFOP:
	  val = parse_pldop (&str);
	  /* This is an extension to accept a 0..31 immediate.  */
	  if (val == PARSE_FAIL)
	    po_imm_or_fail (0, 31);
	  inst.base.operands[i].prfop = aarch64_prfops + val;
	  break;

	case AARCH64_OPND_RPRFMOP:
	  po_enum_or_fail (aarch64_rprfmop_array);
	  info->imm.value = val;
	  break;

	case AARCH64_OPND_BARRIER_PSB:
	  val = parse_barrier_psb (&str, &(info->hint_option));
	  if (val == PARSE_FAIL)
	    goto failure;
	  break;

	case AARCH64_OPND_SME_ZT0:
	  po_reg_or_fail (REG_TYPE_ZT0);
	  break;

	case AARCH64_OPND_SME_ZT0_INDEX:
	  reg = aarch64_reg_parse (&str, REG_TYPE_ZT0, &vectype);
	  if (!reg || vectype.type != NT_invtype)
	    goto failure;
	  if (!(vectype.defined & NTA_HASINDEX))
	    {
	      set_syntax_error (_("missing register index"));
	      goto failure;
	    }
	  info->imm.value = vectype.index;
	  break;

	case AARCH64_OPND_SME_ZT0_LIST:
	  if (*str != '{')
	    {
	      set_expected_reglist_error (REG_TYPE_ZT0, parse_reg (&str));
	      goto failure;
	    }
	  str++;
	  if (!parse_typed_reg (&str, REG_TYPE_ZT0, &vectype, PTR_IN_REGLIST))
	    goto failure;
	  if (*str != '}')
	    {
	      set_syntax_error (_("expected '}' after ZT0"));
	      goto failure;
	    }
	  str++;
	  break;

	case AARCH64_OPND_SME_PNn3_INDEX1:
	case AARCH64_OPND_SME_PNn3_INDEX2:
	  reg = aarch64_reg_parse (&str, REG_TYPE_PN, &vectype);
	  if (!reg)
	    goto failure;
	  if (!(vectype.defined & NTA_HASINDEX))
	    {
	      set_syntax_error (_("missing register index"));
	      goto failure;
	    }
	  info->reglane.regno = reg->number;
	  info->reglane.index = vectype.index;
	  if (vectype.type == NT_invtype)
	    info->qualifier = AARCH64_OPND_QLF_NIL;
	  else
	    info->qualifier = vectype_to_qualifier (&vectype);
	  break;

	case AARCH64_OPND_BTI_TARGET:
	  val = parse_bti_operand (&str, &(info->hint_option));
	  if (val == PARSE_FAIL)
	    goto failure;
	  break;

	case AARCH64_OPND_SME_ZAda_2b:
	case AARCH64_OPND_SME_ZAda_3b:
	  reg = parse_reg_with_qual (&str, REG_TYPE_ZAT, &qualifier, 0);
	  if (!reg)
	    goto failure;
	  info->reg.regno = reg->number;
	  info->qualifier = qualifier;
	  break;

	case AARCH64_OPND_SME_ZA_HV_idx_src:
	case AARCH64_OPND_SME_ZA_HV_idx_srcxN:
	case AARCH64_OPND_SME_ZA_HV_idx_dest:
	case AARCH64_OPND_SME_ZA_HV_idx_destxN:
	case AARCH64_OPND_SME_ZA_HV_idx_ldstr:
	  if (operands[i] == AARCH64_OPND_SME_ZA_HV_idx_ldstr
	      ? !parse_sme_za_hv_tiles_operand_with_braces (&str,
							    &info->indexed_za,
							    &qualifier)
	      : !parse_dual_indexed_reg (&str, REG_TYPE_ZATHV,
					 &info->indexed_za, &qualifier, 0))
	    goto failure;
	  info->qualifier = qualifier;
	  break;

	case AARCH64_OPND_SME_list_of_64bit_tiles:
	  val = parse_sme_list_of_64bit_tiles (&str);
	  if (val == PARSE_FAIL)
	    goto failure;
	  info->imm.value = val;
	  break;

	case AARCH64_OPND_SME_ZA_array_off1x4:
	case AARCH64_OPND_SME_ZA_array_off2x2:
	case AARCH64_OPND_SME_ZA_array_off2x4:
	case AARCH64_OPND_SME_ZA_array_off3_0:
	case AARCH64_OPND_SME_ZA_array_off3_5:
	case AARCH64_OPND_SME_ZA_array_off3x2:
	case AARCH64_OPND_SME_ZA_array_off4:
	  if (!parse_dual_indexed_reg (&str, REG_TYPE_ZA,
				       &info->indexed_za, &qualifier, 0))
	    goto failure;
	  info->qualifier = qualifier;
	  break;

	case AARCH64_OPND_SME_VLxN_10:
	case AARCH64_OPND_SME_VLxN_13:
	  po_strict_enum_or_fail (aarch64_sme_vlxn_array);
	  info->imm.value = val;
	  break;

	case AARCH64_OPND_MOPS_ADDR_Rd:
	case AARCH64_OPND_MOPS_ADDR_Rs:
	  po_char_or_fail ('[');
	  if (!parse_x0_to_x30 (&str, info))
	    goto failure;
	  po_char_or_fail (']');
	  po_char_or_fail ('!');
	  break;

	case AARCH64_OPND_MOPS_WB_Rn:
	  if (!parse_x0_to_x30 (&str, info))
	    goto failure;
	  po_char_or_fail ('!');
	  break;

	default:
	  as_fatal (_("unhandled operand code %d"), operands[i]);
	}

      /* If we get here, this operand was successfully parsed.  */
      inst.base.operands[i].present = 1;
      continue;

    failure:
      /* The parse routine should already have set the error, but in case
	 not, set a default one here.  */
      if (! error_p ())
	set_default_error ();

      if (! backtrack_pos)
	goto parse_operands_return;

      {
	/* We reach here because this operand is marked as optional, and
	   either no operand was supplied or the operand was supplied but it
	   was syntactically incorrect.  In the latter case we report an
	   error.  In the former case we perform a few more checks before
	   dropping through to the code to insert the default operand.  */

	char *tmp = backtrack_pos;
	char endchar = END_OF_INSN;

	if (i != (aarch64_num_of_operands (opcode) - 1))
	  endchar = ',';
	skip_past_char (&tmp, ',');

	if (*tmp != endchar)
	  /* The user has supplied an operand in the wrong format.  */
	  goto parse_operands_return;

	/* Make sure there is not a comma before the optional operand.
	   For example the fifth operand of 'sys' is optional:

	     sys #0,c0,c0,#0,  <--- wrong
	     sys #0,c0,c0,#0   <--- correct.  */
	if (comma_skipped_p && i && endchar == END_OF_INSN)
	  {
	    set_fatal_syntax_error
	      (_("unexpected comma before the omitted optional operand"));
	    goto parse_operands_return;
	  }
      }

      /* Reaching here means we are dealing with an optional operand that is
	 omitted from the assembly line.  */
      gas_assert (optional_operand_p (opcode, i));
      info->present = 0;
      process_omitted_operand (operands[i], opcode, i, info);

      /* Try again, skipping the optional operand at backtrack_pos.  */
      str = backtrack_pos;
      backtrack_pos = 0;

      /* Clear any error record after the omitted optional operand has been
	 successfully handled.  */
      clear_error ();
    }

  /* Check if we have parsed all the operands.  */
  if (*str != '\0' && ! error_p ())
    {
      /* Set I to the index of the last present operand; this is
	 for the purpose of diagnostics.  */
      for (i -= 1; i >= 0 && !inst.base.operands[i].present; --i)
	;
      set_fatal_syntax_error
	(_("unexpected characters following instruction"));
    }

 parse_operands_return:

  if (error_p ())
    {
      inst.parsing_error.index = i;
      DEBUG_TRACE ("parsing FAIL: %s - %s",
		   operand_mismatch_kind_names[inst.parsing_error.kind],
		   inst.parsing_error.error);
      /* Record the operand error properly; this is useful when there
	 are multiple instruction templates for a mnemonic name, so that
	 later on, we can select the error that most closely describes
	 the problem.  */
      record_operand_error_info (opcode, &inst.parsing_error);
      return false;
    }
  else
    {
      DEBUG_TRACE ("parsing SUCCESS");
      return true;
    }
}

/* It does some fix-up to provide some programmer friendly feature while
   keeping the libopcodes happy, i.e. libopcodes only accepts
   the preferred architectural syntax.
   Return FALSE if there is any failure; otherwise return TRUE.  */

static bool
programmer_friendly_fixup (aarch64_instruction *instr)
{
  aarch64_inst *base = &instr->base;
  const aarch64_opcode *opcode = base->opcode;
  enum aarch64_op op = opcode->op;
  aarch64_opnd_info *operands = base->operands;

  DEBUG_TRACE ("enter");

  switch (opcode->iclass)
    {
    case testbranch:
      /* TBNZ Xn|Wn, #uimm6, label
	 Test and Branch Not Zero: conditionally jumps to label if bit number
	 uimm6 in register Xn is not zero.  The bit number implies the width of
	 the register, which may be written and should be disassembled as Wn if
	 uimm is less than 32.  */
      if (operands[0].qualifier == AARCH64_OPND_QLF_W)
	{
	  if (operands[1].imm.value >= 32)
	    {
	      record_operand_out_of_range_error (opcode, 1, _("immediate value"),
						 0, 31);
	      return false;
	    }
	  operands[0].qualifier = AARCH64_OPND_QLF_X;
	}
      break;
    case loadlit:
      /* LDR Wt, label | =value
	 As a convenience assemblers will typically permit the notation
	 "=value" in conjunction with the pc-relative literal load instructions
	 to automatically place an immediate value or symbolic address in a
	 nearby literal pool and generate a hidden label which references it.
	 ISREG has been set to 0 in the case of =value.  */
      if (instr->gen_lit_pool
	  && (op == OP_LDR_LIT || op == OP_LDRV_LIT || op == OP_LDRSW_LIT))
	{
	  int size = aarch64_get_qualifier_esize (operands[0].qualifier);
	  if (op == OP_LDRSW_LIT)
	    size = 4;
	  if (instr->reloc.exp.X_op != O_constant
	      && instr->reloc.exp.X_op != O_big
	      && instr->reloc.exp.X_op != O_symbol)
	    {
	      record_operand_error (opcode, 1,
				    AARCH64_OPDE_FATAL_SYNTAX_ERROR,
				    _("constant expression expected"));
	      return false;
	    }
	  if (! add_to_lit_pool (&instr->reloc.exp, size))
	    {
	      record_operand_error (opcode, 1,
				    AARCH64_OPDE_OTHER_ERROR,
				    _("literal pool insertion failed"));
	      return false;
	    }
	}
      break;
    case log_shift:
    case bitfield:
      /* UXT[BHW] Wd, Wn
	 Unsigned Extend Byte|Halfword|Word: UXT[BH] is architectural alias
	 for UBFM Wd,Wn,#0,#7|15, while UXTW is pseudo instruction which is
	 encoded using ORR Wd, WZR, Wn (MOV Wd,Wn).
	 A programmer-friendly assembler should accept a destination Xd in
	 place of Wd, however that is not the preferred form for disassembly.
	 */
      if ((op == OP_UXTB || op == OP_UXTH || op == OP_UXTW)
	  && operands[1].qualifier == AARCH64_OPND_QLF_W
	  && operands[0].qualifier == AARCH64_OPND_QLF_X)
	operands[0].qualifier = AARCH64_OPND_QLF_W;
      break;

    case addsub_ext:
	{
	  /* In the 64-bit form, the final register operand is written as Wm
	     for all but the (possibly omitted) UXTX/LSL and SXTX
	     operators.
	     As a programmer-friendly assembler, we accept e.g.
	     ADDS <Xd>, <Xn|SP>, <Xm>{, UXTB {#<amount>}} and change it to
	     ADDS <Xd>, <Xn|SP>, <Wm>{, UXTB {#<amount>}}.  */
	  int idx = aarch64_operand_index (opcode->operands,
					   AARCH64_OPND_Rm_EXT);
	  gas_assert (idx == 1 || idx == 2);
	  if (operands[0].qualifier == AARCH64_OPND_QLF_X
	      && operands[idx].qualifier == AARCH64_OPND_QLF_X
	      && operands[idx].shifter.kind != AARCH64_MOD_LSL
	      && operands[idx].shifter.kind != AARCH64_MOD_UXTX
	      && operands[idx].shifter.kind != AARCH64_MOD_SXTX)
	    operands[idx].qualifier = AARCH64_OPND_QLF_W;
	}
      break;

    default:
      break;
    }

  DEBUG_TRACE ("exit with SUCCESS");
  return true;
}

/* Check for loads and stores that will cause unpredictable behavior.  */

static void
warn_unpredictable_ldst (aarch64_instruction *instr, char *str)
{
  aarch64_inst *base = &instr->base;
  const aarch64_opcode *opcode = base->opcode;
  const aarch64_opnd_info *opnds = base->operands;
  switch (opcode->iclass)
    {
    case ldst_pos:
    case ldst_imm9:
    case ldst_imm10:
    case ldst_unscaled:
    case ldst_unpriv:
      /* Loading/storing the base register is unpredictable if writeback.  */
      if ((aarch64_get_operand_class (opnds[0].type)
	   == AARCH64_OPND_CLASS_INT_REG)
	  && opnds[0].reg.regno == opnds[1].addr.base_regno
	  && opnds[1].addr.base_regno != REG_SP
	  /* Exempt STG/STZG/ST2G/STZ2G.  */
	  && !(opnds[1].type == AARCH64_OPND_ADDR_SIMM13)
	  && opnds[1].addr.writeback)
	as_warn (_("unpredictable transfer with writeback -- `%s'"), str);
      break;

    case ldstpair_off:
    case ldstnapair_offs:
    case ldstpair_indexed:
      /* Loading/storing the base register is unpredictable if writeback.  */
      if ((aarch64_get_operand_class (opnds[0].type)
	   == AARCH64_OPND_CLASS_INT_REG)
	  && (opnds[0].reg.regno == opnds[2].addr.base_regno
	    || opnds[1].reg.regno == opnds[2].addr.base_regno)
	  && opnds[2].addr.base_regno != REG_SP
	  /* Exempt STGP.  */
	  && !(opnds[2].type == AARCH64_OPND_ADDR_SIMM11)
	  && opnds[2].addr.writeback)
	    as_warn (_("unpredictable transfer with writeback -- `%s'"), str);
      /* Load operations must load different registers.  */
      if ((opcode->opcode & (1 << 22))
	  && opnds[0].reg.regno == opnds[1].reg.regno)
	    as_warn (_("unpredictable load of register pair -- `%s'"), str);
      break;

    case ldstexcl:
      if ((aarch64_get_operand_class (opnds[0].type)
	   == AARCH64_OPND_CLASS_INT_REG)
	  && (aarch64_get_operand_class (opnds[1].type)
	      == AARCH64_OPND_CLASS_INT_REG))
	{
          if ((opcode->opcode & (1 << 22)))
	    {
	      /* It is unpredictable if load-exclusive pair with Rt == Rt2.  */
	      if ((opcode->opcode & (1 << 21))
		  && opnds[0].reg.regno == opnds[1].reg.regno)
		as_warn (_("unpredictable load of register pair -- `%s'"), str);
	    }
	  else
	    {
	      /*  Store-Exclusive is unpredictable if Rt == Rs.  */
	      if (opnds[0].reg.regno == opnds[1].reg.regno)
		as_warn
		  (_("unpredictable: identical transfer and status registers"
		     " --`%s'"),str);

	      if (opnds[0].reg.regno == opnds[2].reg.regno)
		{
		  if (!(opcode->opcode & (1 << 21)))
	            /*  Store-Exclusive is unpredictable if Rn == Rs.  */
		    as_warn
		      (_("unpredictable: identical base and status registers"
			 " --`%s'"),str);
		  else
	            /*  Store-Exclusive pair is unpredictable if Rt2 == Rs.  */
		    as_warn
		      (_("unpredictable: "
			 "identical transfer and status registers"
			 " --`%s'"),str);
		}

	      /* Store-Exclusive pair is unpredictable if Rn == Rs.  */
	      if ((opcode->opcode & (1 << 21))
		  && opnds[0].reg.regno == opnds[3].reg.regno
		  && opnds[3].reg.regno != REG_SP)
		as_warn (_("unpredictable: identical base and status registers"
			   " --`%s'"),str);
	    }
	}
      break;

    default:
      break;
    }
}

static void
force_automatic_sequence_close (void)
{
  struct aarch64_segment_info_type *tc_seg_info;

  tc_seg_info = &seg_info (now_seg)->tc_segment_info_data;
  if (tc_seg_info->insn_sequence.instr)
    {
      as_warn_where (tc_seg_info->last_file, tc_seg_info->last_line,
		     _("previous `%s' sequence has not been closed"),
		     tc_seg_info->insn_sequence.instr->opcode->name);
      init_insn_sequence (NULL, &tc_seg_info->insn_sequence);
    }
}

/* A wrapper function to interface with libopcodes on encoding and
   record the error message if there is any.

   Return TRUE on success; otherwise return FALSE.  */

static bool
do_encode (const aarch64_opcode *opcode, aarch64_inst *instr,
	   aarch64_insn *code)
{
  aarch64_operand_error error_info;
  memset (&error_info, '\0', sizeof (error_info));
  error_info.kind = AARCH64_OPDE_NIL;
  if (aarch64_opcode_encode (opcode, instr, code, NULL, &error_info, insn_sequence)
      && !error_info.non_fatal)
    return true;

  gas_assert (error_info.kind != AARCH64_OPDE_NIL);
  record_operand_error_info (opcode, &error_info);
  return error_info.non_fatal;
}

#ifdef DEBUG_AARCH64
static inline void
dump_opcode_operands (const aarch64_opcode *opcode)
{
  int i = 0;
  while (opcode->operands[i] != AARCH64_OPND_NIL)
    {
      aarch64_verbose ("\t\t opnd%d: %s", i,
		       aarch64_get_operand_name (opcode->operands[i])[0] != '\0'
		       ? aarch64_get_operand_name (opcode->operands[i])
		       : aarch64_get_operand_desc (opcode->operands[i]));
      ++i;
    }
}
#endif /* DEBUG_AARCH64 */

/* This is the guts of the machine-dependent assembler.  STR points to a
   machine dependent instruction.  This function is supposed to emit
   the frags/bytes it assembles to.  */

void
md_assemble (char *str)
{
  templates *template;
  const aarch64_opcode *opcode;
  struct aarch64_segment_info_type *tc_seg_info;
  aarch64_inst *inst_base;
  unsigned saved_cond;

  /* Align the previous label if needed.  */
  if (last_label_seen != NULL)
    {
      symbol_set_frag (last_label_seen, frag_now);
      S_SET_VALUE (last_label_seen, (valueT) frag_now_fix ());
      S_SET_SEGMENT (last_label_seen, now_seg);
    }

  /* Update the current insn_sequence from the segment.  */
  tc_seg_info = &seg_info (now_seg)->tc_segment_info_data;
  insn_sequence = &tc_seg_info->insn_sequence;
  tc_seg_info->last_file = as_where (&tc_seg_info->last_line);

  inst.reloc.type = BFD_RELOC_UNUSED;

  DEBUG_TRACE ("\n\n");
  DEBUG_TRACE ("==============================");
  DEBUG_TRACE ("Enter md_assemble with %s", str);

  /* Scan up to the end of the mnemonic, which must end in whitespace,
     '.', or end of string.  */
  char *p = str;
  char *dot = 0;
  for (; is_part_of_name (*p); p++)
    if (*p == '.' && !dot)
      dot = p;

  if (p == str)
    {
      as_bad (_("unknown mnemonic -- `%s'"), str);
      return;
    }

  if (!dot && create_register_alias (str, p))
    return;

  template = opcode_lookup (str, dot, p);
  if (!template)
    {
      as_bad (_("unknown mnemonic `%s' -- `%s'"), get_mnemonic_name (str),
	      str);
      return;
    }

  skip_whitespace (p);
  if (*p == ',')
    {
      as_bad (_("unexpected comma after the mnemonic name `%s' -- `%s'"),
	      get_mnemonic_name (str), str);
      return;
    }

  init_operand_error_report ();

  /* Sections are assumed to start aligned. In executable section, there is no
     MAP_DATA symbol pending. So we only align the address during
     MAP_DATA --> MAP_INSN transition.
     For other sections, this is not guaranteed.  */
  enum mstate mapstate = seg_info (now_seg)->tc_segment_info_data.mapstate;
  if (!need_pass_2 && subseg_text_p (now_seg) && mapstate == MAP_DATA)
    frag_align_code (2, 0);

  saved_cond = inst.cond;
  reset_aarch64_instruction (&inst);
  inst.cond = saved_cond;

  /* Iterate through all opcode entries with the same mnemonic name.  */
  do
    {
      opcode = template->opcode;

      DEBUG_TRACE ("opcode %s found", opcode->name);
#ifdef DEBUG_AARCH64
      if (debug_dump)
	dump_opcode_operands (opcode);
#endif /* DEBUG_AARCH64 */

      mapping_state (MAP_INSN);

      inst_base = &inst.base;
      inst_base->opcode = opcode;

      /* Truly conditionally executed instructions, e.g. b.cond.  */
      if (opcode->flags & F_COND)
	{
	  gas_assert (inst.cond != COND_ALWAYS);
	  inst_base->cond = get_cond_from_value (inst.cond);
	  DEBUG_TRACE ("condition found %s", inst_base->cond->names[0]);
	}
      else if (inst.cond != COND_ALWAYS)
	{
	  /* It shouldn't arrive here, where the assembly looks like a
	     conditional instruction but the found opcode is unconditional.  */
	  gas_assert (0);
	  continue;
	}

      if (parse_operands (p, opcode)
	  && programmer_friendly_fixup (&inst)
	  && do_encode (inst_base->opcode, &inst.base, &inst_base->value))
	{
	  /* Check that this instruction is supported for this CPU.  */
	  if (!aarch64_cpu_supports_inst_p (cpu_variant, inst_base))
	    {
	      as_bad (_("selected processor does not support `%s'"), str);
	      return;
	    }

	  warn_unpredictable_ldst (&inst, str);

	  if (inst.reloc.type == BFD_RELOC_UNUSED
	      || !inst.reloc.need_libopcodes_p)
	    output_inst (NULL);
	  else
	    {
	      /* If there is relocation generated for the instruction,
	         store the instruction information for the future fix-up.  */
	      struct aarch64_inst *copy;
	      gas_assert (inst.reloc.type != BFD_RELOC_UNUSED);
	      copy = XNEW (struct aarch64_inst);
	      memcpy (copy, &inst.base, sizeof (struct aarch64_inst));
	      output_inst (copy);
	    }

	  /* Issue non-fatal messages if any.  */
	  output_operand_error_report (str, true);
	  return;
	}

      template = template->next;
      if (template != NULL)
	{
	  reset_aarch64_instruction (&inst);
	  inst.cond = saved_cond;
	}
    }
  while (template != NULL);

  /* Issue the error messages if any.  */
  output_operand_error_report (str, false);
}

/* Various frobbings of labels and their addresses.  */

void
aarch64_start_line_hook (void)
{
  last_label_seen = NULL;
}

void
aarch64_frob_label (symbolS * sym)
{
  last_label_seen = sym;

  dwarf2_emit_label (sym);
}

void
aarch64_frob_section (asection *sec ATTRIBUTE_UNUSED)
{
  /* Check to see if we have a block to close.  */
  force_automatic_sequence_close ();
}

int
aarch64_data_in_code (void)
{
  if (startswith (input_line_pointer + 1, "data:"))
    {
      *input_line_pointer = '/';
      input_line_pointer += 5;
      *input_line_pointer = 0;
      return 1;
    }

  return 0;
}

char *
aarch64_canonicalize_symbol_name (char *name)
{
  int len;

  if ((len = strlen (name)) > 5 && streq (name + len - 5, "/data"))
    *(name + len - 5) = 0;

  return name;
}

/* Table of all register names defined by default.  The user can
   define additional names with .req.  Note that all register names
   should appear in both upper and lowercase variants.	Some registers
   also have mixed-case names.	*/

#define REGDEF(s,n,t) { #s, n, REG_TYPE_##t, true }
#define REGDEF_ALIAS(s, n, t) { #s, n, REG_TYPE_##t, false}
#define REGNUM(p,n,t) REGDEF(p##n, n, t)
#define REGNUMS(p,n,s,t) REGDEF(p##n##s, n, t)
#define REGSET16(p,t) \
  REGNUM(p, 0,t), REGNUM(p, 1,t), REGNUM(p, 2,t), REGNUM(p, 3,t), \
  REGNUM(p, 4,t), REGNUM(p, 5,t), REGNUM(p, 6,t), REGNUM(p, 7,t), \
  REGNUM(p, 8,t), REGNUM(p, 9,t), REGNUM(p,10,t), REGNUM(p,11,t), \
  REGNUM(p,12,t), REGNUM(p,13,t), REGNUM(p,14,t), REGNUM(p,15,t)
#define REGSET16S(p,s,t) \
  REGNUMS(p, 0,s,t), REGNUMS(p, 1,s,t), REGNUMS(p, 2,s,t), REGNUMS(p, 3,s,t), \
  REGNUMS(p, 4,s,t), REGNUMS(p, 5,s,t), REGNUMS(p, 6,s,t), REGNUMS(p, 7,s,t), \
  REGNUMS(p, 8,s,t), REGNUMS(p, 9,s,t), REGNUMS(p,10,s,t), REGNUMS(p,11,s,t), \
  REGNUMS(p,12,s,t), REGNUMS(p,13,s,t), REGNUMS(p,14,s,t), REGNUMS(p,15,s,t)
#define REGSET31(p,t) \
  REGSET16(p, t), \
  REGNUM(p,16,t), REGNUM(p,17,t), REGNUM(p,18,t), REGNUM(p,19,t), \
  REGNUM(p,20,t), REGNUM(p,21,t), REGNUM(p,22,t), REGNUM(p,23,t), \
  REGNUM(p,24,t), REGNUM(p,25,t), REGNUM(p,26,t), REGNUM(p,27,t), \
  REGNUM(p,28,t), REGNUM(p,29,t), REGNUM(p,30,t)
#define REGSET(p,t) \
  REGSET31(p,t), REGNUM(p,31,t)

/* These go into aarch64_reg_hsh hash-table.  */
static const reg_entry reg_names[] = {
  /* Integer registers.  */
  REGSET31 (x, R_64), REGSET31 (X, R_64),
  REGSET31 (w, R_32), REGSET31 (W, R_32),

  REGDEF_ALIAS (ip0, 16, R_64), REGDEF_ALIAS (IP0, 16, R_64),
  REGDEF_ALIAS (ip1, 17, R_64), REGDEF_ALIAS (IP1, 17, R_64),
  REGDEF_ALIAS (fp, 29, R_64), REGDEF_ALIAS (FP, 29, R_64),
  REGDEF_ALIAS (lr, 30, R_64), REGDEF_ALIAS (LR, 30, R_64),
  REGDEF (wsp, 31, SP_32), REGDEF (WSP, 31, SP_32),
  REGDEF (sp, 31, SP_64), REGDEF (SP, 31, SP_64),

  REGDEF (wzr, 31, ZR_32), REGDEF (WZR, 31, ZR_32),
  REGDEF (xzr, 31, ZR_64), REGDEF (XZR, 31, ZR_64),

  /* Floating-point single precision registers.  */
  REGSET (s, FP_S), REGSET (S, FP_S),

  /* Floating-point double precision registers.  */
  REGSET (d, FP_D), REGSET (D, FP_D),

  /* Floating-point half precision registers.  */
  REGSET (h, FP_H), REGSET (H, FP_H),

  /* Floating-point byte precision registers.  */
  REGSET (b, FP_B), REGSET (B, FP_B),

  /* Floating-point quad precision registers.  */
  REGSET (q, FP_Q), REGSET (Q, FP_Q),

  /* FP/SIMD registers.  */
  REGSET (v, V), REGSET (V, V),

  /* SVE vector registers.  */
  REGSET (z, Z), REGSET (Z, Z),

  /* SVE predicate(-as-mask) registers.  */
  REGSET16 (p, P), REGSET16 (P, P),

  /* SVE predicate-as-counter registers.  */
  REGSET16 (pn, PN), REGSET16 (PN, PN),

  /* SME ZA.  We model this as a register because it acts syntactically
     like ZA0H, supporting qualifier suffixes and indexing.  */
  REGDEF (za, 0, ZA), REGDEF (ZA, 0, ZA),

  /* SME ZA tile registers.  */
  REGSET16 (za, ZAT), REGSET16 (ZA, ZAT),

  /* SME ZA tile registers (horizontal slice).  */
  REGSET16S (za, h, ZATH), REGSET16S (ZA, H, ZATH),

  /* SME ZA tile registers (vertical slice).  */
  REGSET16S (za, v, ZATV), REGSET16S (ZA, V, ZATV),

  /* SME2 ZT0.  */
  REGDEF (zt0, 0, ZT0), REGDEF (ZT0, 0, ZT0)
};

#undef REGDEF
#undef REGDEF_ALIAS
#undef REGNUM
#undef REGSET16
#undef REGSET31
#undef REGSET

#define N 1
#define n 0
#define Z 1
#define z 0
#define C 1
#define c 0
#define V 1
#define v 0
#define B(a,b,c,d) (((a) << 3) | ((b) << 2) | ((c) << 1) | (d))
static const asm_nzcv nzcv_names[] = {
  {"nzcv", B (n, z, c, v)},
  {"nzcV", B (n, z, c, V)},
  {"nzCv", B (n, z, C, v)},
  {"nzCV", B (n, z, C, V)},
  {"nZcv", B (n, Z, c, v)},
  {"nZcV", B (n, Z, c, V)},
  {"nZCv", B (n, Z, C, v)},
  {"nZCV", B (n, Z, C, V)},
  {"Nzcv", B (N, z, c, v)},
  {"NzcV", B (N, z, c, V)},
  {"NzCv", B (N, z, C, v)},
  {"NzCV", B (N, z, C, V)},
  {"NZcv", B (N, Z, c, v)},
  {"NZcV", B (N, Z, c, V)},
  {"NZCv", B (N, Z, C, v)},
  {"NZCV", B (N, Z, C, V)}
};

#undef N
#undef n
#undef Z
#undef z
#undef C
#undef c
#undef V
#undef v
#undef B

/* MD interface: bits in the object file.  */

/* Turn an integer of n bytes (in val) into a stream of bytes appropriate
   for use in the a.out file, and stores them in the array pointed to by buf.
   This knows about the endian-ness of the target machine and does
   THE RIGHT THING, whatever it is.  Possible values for n are 1 (byte)
   2 (short) and 4 (long)  Floating numbers are put out as a series of
   LITTLENUMS (shorts, here at least).	*/

void
md_number_to_chars (char *buf, valueT val, int n)
{
  if (target_big_endian)
    number_to_chars_bigendian (buf, val, n);
  else
    number_to_chars_littleendian (buf, val, n);
}

/* MD interface: Sections.  */

/* Estimate the size of a frag before relaxing.  Assume everything fits in
   4 bytes.  */

int
md_estimate_size_before_relax (fragS * fragp, segT segtype ATTRIBUTE_UNUSED)
{
  fragp->fr_var = 4;
  return 4;
}

/* Round up a section size to the appropriate boundary.	 */

valueT
md_section_align (segT segment ATTRIBUTE_UNUSED, valueT size)
{
  return size;
}

/* This is called from HANDLE_ALIGN in write.c.	 Fill in the contents
   of an rs_align_code fragment.

   Here we fill the frag with the appropriate info for padding the
   output stream.  The resulting frag will consist of a fixed (fr_fix)
   and of a repeating (fr_var) part.

   The fixed content is always emitted before the repeating content and
   these two parts are used as follows in constructing the output:
   - the fixed part will be used to align to a valid instruction word
     boundary, in case that we start at a misaligned address; as no
     executable instruction can live at the misaligned location, we
     simply fill with zeros;
   - the variable part will be used to cover the remaining padding and
     we fill using the AArch64 NOP instruction.

   Note that the size of a RS_ALIGN_CODE fragment is always 7 to provide
   enough storage space for up to 3 bytes for padding the back to a valid
   instruction alignment and exactly 4 bytes to store the NOP pattern.  */

void
aarch64_handle_align (fragS * fragP)
{
  /* NOP = d503201f */
  /* AArch64 instructions are always little-endian.  */
  static unsigned char const aarch64_noop[4] = { 0x1f, 0x20, 0x03, 0xd5 };

  int bytes, fix, noop_size;
  char *p;

  if (fragP->fr_type != rs_align_code)
    return;

  bytes = fragP->fr_next->fr_address - fragP->fr_address - fragP->fr_fix;
  p = fragP->fr_literal + fragP->fr_fix;

#ifdef OBJ_ELF
  gas_assert (fragP->tc_frag_data.recorded);
#endif

  noop_size = sizeof (aarch64_noop);

  fix = bytes & (noop_size - 1);
  if (fix)
    {
#if defined OBJ_ELF || defined OBJ_COFF
      insert_data_mapping_symbol (MAP_INSN, fragP->fr_fix, fragP, fix);
#endif
      memset (p, 0, fix);
      p += fix;
      fragP->fr_fix += fix;
    }

  if (noop_size)
    memcpy (p, aarch64_noop, noop_size);
  fragP->fr_var = noop_size;
}

/* Perform target specific initialisation of a frag.
   Note - despite the name this initialisation is not done when the frag
   is created, but only when its type is assigned.  A frag can be created
   and used a long time before its type is set, so beware of assuming that
   this initialisation is performed first.  */

#ifndef OBJ_ELF
void
aarch64_init_frag (fragS * fragP ATTRIBUTE_UNUSED,
		   int max_chars ATTRIBUTE_UNUSED)
{
}

#else /* OBJ_ELF is defined.  */
void
aarch64_init_frag (fragS * fragP, int max_chars)
{
  /* Record a mapping symbol for alignment frags.  We will delete this
     later if the alignment ends up empty.  */
  if (!fragP->tc_frag_data.recorded)
    fragP->tc_frag_data.recorded = 1;

  /* PR 21809: Do not set a mapping state for debug sections
     - it just confuses other tools.  */
  if (bfd_section_flags (now_seg) & SEC_DEBUGGING)
    return;

  switch (fragP->fr_type)
    {
    case rs_align_test:
    case rs_fill:
      mapping_state_2 (MAP_DATA, max_chars);
      break;
    case rs_align:
      /* PR 20364: We can get alignment frags in code sections,
	 so do not just assume that we should use the MAP_DATA state.  */
      mapping_state_2 (subseg_text_p (now_seg) ? MAP_INSN : MAP_DATA, max_chars);
      break;
    case rs_align_code:
      mapping_state_2 (MAP_INSN, max_chars);
      break;
    default:
      break;
    }
}

/* Whether SFrame stack trace info is supported.  */

bool
aarch64_support_sframe_p (void)
{
  /* At this time, SFrame is supported for aarch64 only.  */
  return (aarch64_abi == AARCH64_ABI_LP64);
}

/* Specify if RA tracking is needed.  */

bool
aarch64_sframe_ra_tracking_p (void)
{
  return true;
}

/* Specify the fixed offset to recover RA from CFA.
   (useful only when RA tracking is not needed).  */

offsetT
aarch64_sframe_cfa_ra_offset (void)
{
  return (offsetT) SFRAME_CFA_FIXED_RA_INVALID;
}

/* Get the abi/arch indentifier for SFrame.  */

unsigned char
aarch64_sframe_get_abi_arch (void)
{
  unsigned char sframe_abi_arch = 0;

  if (aarch64_support_sframe_p ())
    {
      sframe_abi_arch = target_big_endian
	? SFRAME_ABI_AARCH64_ENDIAN_BIG
	: SFRAME_ABI_AARCH64_ENDIAN_LITTLE;
    }

  return sframe_abi_arch;
}

#endif /* OBJ_ELF */

/* Initialize the DWARF-2 unwind information for this procedure.  */

void
tc_aarch64_frame_initial_instructions (void)
{
  cfi_add_CFA_def_cfa (REG_SP, 0);
}

/* Convert REGNAME to a DWARF-2 register number.  */

int
tc_aarch64_regname_to_dw2regnum (char *regname)
{
  const reg_entry *reg = parse_reg (&regname);
  if (reg == NULL)
    return -1;

  switch (reg->type)
    {
    case REG_TYPE_SP_32:
    case REG_TYPE_SP_64:
    case REG_TYPE_R_32:
    case REG_TYPE_R_64:
      return reg->number;

    case REG_TYPE_FP_B:
    case REG_TYPE_FP_H:
    case REG_TYPE_FP_S:
    case REG_TYPE_FP_D:
    case REG_TYPE_FP_Q:
      return reg->number + 64;

    default:
      break;
    }
  return -1;
}

/* Implement DWARF2_ADDR_SIZE.  */

int
aarch64_dwarf2_addr_size (void)
{
  if (ilp32_p)
    return 4;
  else if (llp64_p)
    return 8;
  return bfd_arch_bits_per_address (stdoutput) / 8;
}

/* MD interface: Symbol and relocation handling.  */

/* Return the address within the segment that a PC-relative fixup is
   relative to.  For AArch64 PC-relative fixups applied to instructions
   are generally relative to the location plus AARCH64_PCREL_OFFSET bytes.  */

long
md_pcrel_from_section (fixS * fixP, segT seg)
{
  offsetT base = fixP->fx_where + fixP->fx_frag->fr_address;

  /* If this is pc-relative and we are going to emit a relocation
     then we just want to put out any pipeline compensation that the linker
     will need.  Otherwise we want to use the calculated base.  */
  if (fixP->fx_pcrel
      && ((fixP->fx_addsy && S_GET_SEGMENT (fixP->fx_addsy) != seg)
	  || aarch64_force_relocation (fixP)))
    base = 0;

  /* AArch64 should be consistent for all pc-relative relocations.  */
  return base + AARCH64_PCREL_OFFSET;
}

/* Under ELF we need to default _GLOBAL_OFFSET_TABLE.
   Otherwise we have no need to default values of symbols.  */

symbolS *
md_undefined_symbol (char *name ATTRIBUTE_UNUSED)
{
#ifdef OBJ_ELF
  if (name[0] == '_' && name[1] == 'G'
      && streq (name, GLOBAL_OFFSET_TABLE_NAME))
    {
      if (!GOT_symbol)
	{
	  if (symbol_find (name))
	    as_bad (_("GOT already in the symbol table"));

	  GOT_symbol = symbol_new (name, undefined_section,
				   &zero_address_frag, 0);
	}

      return GOT_symbol;
    }
#endif

  return 0;
}

/* Return non-zero if the indicated VALUE has overflowed the maximum
   range expressible by a unsigned number with the indicated number of
   BITS.  */

static bool
unsigned_overflow (valueT value, unsigned bits)
{
  valueT lim;
  if (bits >= sizeof (valueT) * 8)
    return false;
  lim = (valueT) 1 << bits;
  return (value >= lim);
}


/* Return non-zero if the indicated VALUE has overflowed the maximum
   range expressible by an signed number with the indicated number of
   BITS.  */

static bool
signed_overflow (offsetT value, unsigned bits)
{
  offsetT lim;
  if (bits >= sizeof (offsetT) * 8)
    return false;
  lim = (offsetT) 1 << (bits - 1);
  return (value < -lim || value >= lim);
}

/* Given an instruction in *INST, which is expected to be a scaled, 12-bit,
   unsigned immediate offset load/store instruction, try to encode it as
   an unscaled, 9-bit, signed immediate offset load/store instruction.
   Return TRUE if it is successful; otherwise return FALSE.

   As a programmer-friendly assembler, LDUR/STUR instructions can be generated
   in response to the standard LDR/STR mnemonics when the immediate offset is
   unambiguous, i.e. when it is negative or unaligned.  */

static bool
try_to_encode_as_unscaled_ldst (aarch64_inst *instr)
{
  int idx;
  enum aarch64_op new_op;
  const aarch64_opcode *new_opcode;

  gas_assert (instr->opcode->iclass == ldst_pos);

  switch (instr->opcode->op)
    {
    case OP_LDRB_POS:new_op = OP_LDURB; break;
    case OP_STRB_POS: new_op = OP_STURB; break;
    case OP_LDRSB_POS: new_op = OP_LDURSB; break;
    case OP_LDRH_POS: new_op = OP_LDURH; break;
    case OP_STRH_POS: new_op = OP_STURH; break;
    case OP_LDRSH_POS: new_op = OP_LDURSH; break;
    case OP_LDR_POS: new_op = OP_LDUR; break;
    case OP_STR_POS: new_op = OP_STUR; break;
    case OP_LDRF_POS: new_op = OP_LDURV; break;
    case OP_STRF_POS: new_op = OP_STURV; break;
    case OP_LDRSW_POS: new_op = OP_LDURSW; break;
    case OP_PRFM_POS: new_op = OP_PRFUM; break;
    default: new_op = OP_NIL; break;
    }

  if (new_op == OP_NIL)
    return false;

  new_opcode = aarch64_get_opcode (new_op);
  gas_assert (new_opcode != NULL);

  DEBUG_TRACE ("Check programmer-friendly STURB/LDURB -> STRB/LDRB: %d == %d",
	       instr->opcode->op, new_opcode->op);

  aarch64_replace_opcode (instr, new_opcode);

  /* Clear up the ADDR_SIMM9's qualifier; otherwise the
     qualifier matching may fail because the out-of-date qualifier will
     prevent the operand being updated with a new and correct qualifier.  */
  idx = aarch64_operand_index (instr->opcode->operands,
			       AARCH64_OPND_ADDR_SIMM9);
  gas_assert (idx == 1);
  instr->operands[idx].qualifier = AARCH64_OPND_QLF_NIL;

  DEBUG_TRACE ("Found LDURB entry to encode programmer-friendly LDRB");

  if (!aarch64_opcode_encode (instr->opcode, instr, &instr->value, NULL, NULL,
			      insn_sequence))
    return false;

  return true;
}

/* Called by fix_insn to fix a MOV immediate alias instruction.

   Operand for a generic move immediate instruction, which is an alias
   instruction that generates a single MOVZ, MOVN or ORR instruction to loads
   a 32-bit/64-bit immediate value into general register.  An assembler error
   shall result if the immediate cannot be created by a single one of these
   instructions. If there is a choice, then to ensure reversability an
   assembler must prefer a MOVZ to MOVN, and MOVZ or MOVN to ORR.  */

static void
fix_mov_imm_insn (fixS *fixP, char *buf, aarch64_inst *instr, offsetT value)
{
  const aarch64_opcode *opcode;

  /* Need to check if the destination is SP/ZR.  The check has to be done
     before any aarch64_replace_opcode.  */
  int try_mov_wide_p = !aarch64_stack_pointer_p (&instr->operands[0]);
  int try_mov_bitmask_p = !aarch64_zero_register_p (&instr->operands[0]);

  instr->operands[1].imm.value = value;
  instr->operands[1].skip = 0;

  if (try_mov_wide_p)
    {
      /* Try the MOVZ alias.  */
      opcode = aarch64_get_opcode (OP_MOV_IMM_WIDE);
      aarch64_replace_opcode (instr, opcode);
      if (aarch64_opcode_encode (instr->opcode, instr,
				 &instr->value, NULL, NULL, insn_sequence))
	{
	  put_aarch64_insn (buf, instr->value);
	  return;
	}
      /* Try the MOVK alias.  */
      opcode = aarch64_get_opcode (OP_MOV_IMM_WIDEN);
      aarch64_replace_opcode (instr, opcode);
      if (aarch64_opcode_encode (instr->opcode, instr,
				 &instr->value, NULL, NULL, insn_sequence))
	{
	  put_aarch64_insn (buf, instr->value);
	  return;
	}
    }

  if (try_mov_bitmask_p)
    {
      /* Try the ORR alias.  */
      opcode = aarch64_get_opcode (OP_MOV_IMM_LOG);
      aarch64_replace_opcode (instr, opcode);
      if (aarch64_opcode_encode (instr->opcode, instr,
				 &instr->value, NULL, NULL, insn_sequence))
	{
	  put_aarch64_insn (buf, instr->value);
	  return;
	}
    }

  as_bad_where (fixP->fx_file, fixP->fx_line,
		_("immediate cannot be moved by a single instruction"));
}

/* An instruction operand which is immediate related may have symbol used
   in the assembly, e.g.

     mov     w0, u32
     .set    u32,    0x00ffff00

   At the time when the assembly instruction is parsed, a referenced symbol,
   like 'u32' in the above example may not have been seen; a fixS is created
   in such a case and is handled here after symbols have been resolved.
   Instruction is fixed up with VALUE using the information in *FIXP plus
   extra information in FLAGS.

   This function is called by md_apply_fix to fix up instructions that need
   a fix-up described above but does not involve any linker-time relocation.  */

static void
fix_insn (fixS *fixP, uint32_t flags, offsetT value)
{
  int idx;
  uint32_t insn;
  char *buf = fixP->fx_where + fixP->fx_frag->fr_literal;
  enum aarch64_opnd opnd = fixP->tc_fix_data.opnd;
  aarch64_inst *new_inst = fixP->tc_fix_data.inst;

  if (new_inst)
    {
      /* Now the instruction is about to be fixed-up, so the operand that
	 was previously marked as 'ignored' needs to be unmarked in order
	 to get the encoding done properly.  */
      idx = aarch64_operand_index (new_inst->opcode->operands, opnd);
      new_inst->operands[idx].skip = 0;
    }

  gas_assert (opnd != AARCH64_OPND_NIL);

  switch (opnd)
    {
    case AARCH64_OPND_EXCEPTION:
    case AARCH64_OPND_UNDEFINED:
      if (unsigned_overflow (value, 16))
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("immediate out of range"));
      insn = get_aarch64_insn (buf);
      insn |= (opnd == AARCH64_OPND_EXCEPTION) ? encode_svc_imm (value) : value;
      put_aarch64_insn (buf, insn);
      break;

    case AARCH64_OPND_AIMM:
      /* ADD or SUB with immediate.
	 NOTE this assumes we come here with a add/sub shifted reg encoding
		  3  322|2222|2  2  2 21111 111111
		  1  098|7654|3  2  1 09876 543210 98765 43210
	 0b000000 sf 000|1011|shift 0 Rm    imm6   Rn    Rd    ADD
	 2b000000 sf 010|1011|shift 0 Rm    imm6   Rn    Rd    ADDS
	 4b000000 sf 100|1011|shift 0 Rm    imm6   Rn    Rd    SUB
	 6b000000 sf 110|1011|shift 0 Rm    imm6   Rn    Rd    SUBS
	 ->
		  3  322|2222|2 2   221111111111
		  1  098|7654|3 2   109876543210 98765 43210
	 11000000 sf 001|0001|shift imm12        Rn    Rd    ADD
	 31000000 sf 011|0001|shift imm12        Rn    Rd    ADDS
	 51000000 sf 101|0001|shift imm12        Rn    Rd    SUB
	 71000000 sf 111|0001|shift imm12        Rn    Rd    SUBS
	 Fields sf Rn Rd are already set.  */
      insn = get_aarch64_insn (buf);
      if (value < 0)
	{
	  /* Add <-> sub.  */
	  insn = reencode_addsub_switch_add_sub (insn);
	  value = -value;
	}

      if ((flags & FIXUP_F_HAS_EXPLICIT_SHIFT) == 0
	  && unsigned_overflow (value, 12))
	{
	  /* Try to shift the value by 12 to make it fit.  */
	  if (((value >> 12) << 12) == value
	      && ! unsigned_overflow (value, 12 + 12))
	    {
	      value >>= 12;
	      insn |= encode_addsub_imm_shift_amount (1);
	    }
	}

      if (unsigned_overflow (value, 12))
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("immediate out of range"));

      insn |= encode_addsub_imm (value);

      put_aarch64_insn (buf, insn);
      break;

    case AARCH64_OPND_SIMD_IMM:
    case AARCH64_OPND_SIMD_IMM_SFT:
    case AARCH64_OPND_LIMM:
      /* Bit mask immediate.  */
      gas_assert (new_inst != NULL);
      idx = aarch64_operand_index (new_inst->opcode->operands, opnd);
      new_inst->operands[idx].imm.value = value;
      if (aarch64_opcode_encode (new_inst->opcode, new_inst,
				 &new_inst->value, NULL, NULL, insn_sequence))
	put_aarch64_insn (buf, new_inst->value);
      else
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("invalid immediate"));
      break;

    case AARCH64_OPND_HALF:
      /* 16-bit unsigned immediate.  */
      if (unsigned_overflow (value, 16))
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("immediate out of range"));
      insn = get_aarch64_insn (buf);
      insn |= encode_movw_imm (value & 0xffff);
      put_aarch64_insn (buf, insn);
      break;

    case AARCH64_OPND_IMM_MOV:
      /* Operand for a generic move immediate instruction, which is
	 an alias instruction that generates a single MOVZ, MOVN or ORR
	 instruction to loads a 32-bit/64-bit immediate value into general
	 register.  An assembler error shall result if the immediate cannot be
	 created by a single one of these instructions. If there is a choice,
	 then to ensure reversability an assembler must prefer a MOVZ to MOVN,
	 and MOVZ or MOVN to ORR.  */
      gas_assert (new_inst != NULL);
      fix_mov_imm_insn (fixP, buf, new_inst, value);
      break;

    case AARCH64_OPND_ADDR_SIMM7:
    case AARCH64_OPND_ADDR_SIMM9:
    case AARCH64_OPND_ADDR_SIMM9_2:
    case AARCH64_OPND_ADDR_SIMM10:
    case AARCH64_OPND_ADDR_UIMM12:
    case AARCH64_OPND_ADDR_SIMM11:
    case AARCH64_OPND_ADDR_SIMM13:
      /* Immediate offset in an address.  */
      insn = get_aarch64_insn (buf);

      gas_assert (new_inst != NULL && new_inst->value == insn);
      gas_assert (new_inst->opcode->operands[1] == opnd
		  || new_inst->opcode->operands[2] == opnd);

      /* Get the index of the address operand.  */
      if (new_inst->opcode->operands[1] == opnd)
	/* e.g. STR <Xt>, [<Xn|SP>, <R><m>{, <extend> {<amount>}}].  */
	idx = 1;
      else
	/* e.g. LDP <Qt1>, <Qt2>, [<Xn|SP>{, #<imm>}].  */
	idx = 2;

      /* Update the resolved offset value.  */
      new_inst->operands[idx].addr.offset.imm = value;

      /* Encode/fix-up.  */
      if (aarch64_opcode_encode (new_inst->opcode, new_inst,
				 &new_inst->value, NULL, NULL, insn_sequence))
	{
	  put_aarch64_insn (buf, new_inst->value);
	  break;
	}
      else if (new_inst->opcode->iclass == ldst_pos
	       && try_to_encode_as_unscaled_ldst (new_inst))
	{
	  put_aarch64_insn (buf, new_inst->value);
	  break;
	}

      as_bad_where (fixP->fx_file, fixP->fx_line,
		    _("immediate offset out of range"));
      break;

    default:
      gas_assert (0);
      as_fatal (_("unhandled operand code %d"), opnd);
    }
}

/* Apply a fixup (fixP) to segment data, once it has been determined
   by our caller that we have all the info we need to fix it up.

   Parameter valP is the pointer to the value of the bits.  */

void
md_apply_fix (fixS * fixP, valueT * valP, segT seg)
{
  offsetT value = *valP;
  uint32_t insn;
  char *buf = fixP->fx_where + fixP->fx_frag->fr_literal;
  int scale;
  unsigned flags = fixP->fx_addnumber;

  DEBUG_TRACE ("\n\n");
  DEBUG_TRACE ("~~~~~~~~~~~~~~~~~~~~~~~~~");
  DEBUG_TRACE ("Enter md_apply_fix");

  gas_assert (fixP->fx_r_type <= BFD_RELOC_UNUSED);

  /* Note whether this will delete the relocation.  */

  if (fixP->fx_addsy == 0 && !fixP->fx_pcrel
      && aarch64_force_reloc (fixP->fx_r_type) <= 0)
    fixP->fx_done = 1;

  /* Process the relocations.  */
  switch (fixP->fx_r_type)
    {
    case BFD_RELOC_NONE:
      /* This will need to go in the object file.  */
      fixP->fx_done = 0;
      break;

    case BFD_RELOC_8:
    case BFD_RELOC_8_PCREL:
      if (fixP->fx_done || !seg->use_rela_p)
	md_number_to_chars (buf, value, 1);
      break;

    case BFD_RELOC_16:
    case BFD_RELOC_16_PCREL:
      if (fixP->fx_done || !seg->use_rela_p)
	md_number_to_chars (buf, value, 2);
      break;

    case BFD_RELOC_32:
    case BFD_RELOC_32_PCREL:
      if (fixP->fx_done || !seg->use_rela_p)
	md_number_to_chars (buf, value, 4);
      break;

    case BFD_RELOC_64:
    case BFD_RELOC_64_PCREL:
      if (fixP->fx_done || !seg->use_rela_p)
	md_number_to_chars (buf, value, 8);
      break;

    case BFD_RELOC_AARCH64_GAS_INTERNAL_FIXUP:
      /* We claim that these fixups have been processed here, even if
         in fact we generate an error because we do not have a reloc
         for them, so tc_gen_reloc() will reject them.  */
      fixP->fx_done = 1;
      if (fixP->fx_addsy && !S_IS_DEFINED (fixP->fx_addsy))
	{
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			_("undefined symbol %s used as an immediate value"),
			S_GET_NAME (fixP->fx_addsy));
	  goto apply_fix_return;
	}
      fix_insn (fixP, flags, value);
      break;

    case BFD_RELOC_AARCH64_LD_LO19_PCREL:
      if (fixP->fx_done || !seg->use_rela_p)
	{
	  if (value & 3)
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("pc-relative load offset not word aligned"));
	  if (signed_overflow (value, 21))
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("pc-relative load offset out of range"));
	  insn = get_aarch64_insn (buf);
	  insn |= encode_ld_lit_ofs_19 (value >> 2);
	  put_aarch64_insn (buf, insn);
	}
      break;

    case BFD_RELOC_AARCH64_ADR_LO21_PCREL:
      if (fixP->fx_done || !seg->use_rela_p)
	{
	  if (signed_overflow (value, 21))
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("pc-relative address offset out of range"));
	  insn = get_aarch64_insn (buf);
	  insn |= encode_adr_imm (value);
	  put_aarch64_insn (buf, insn);
	}
      break;

    case BFD_RELOC_AARCH64_BRANCH19:
      if (fixP->fx_done || !seg->use_rela_p)
	{
	  if (value & 3)
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("conditional branch target not word aligned"));
	  if (signed_overflow (value, 21))
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("conditional branch out of range"));
	  insn = get_aarch64_insn (buf);
	  insn |= encode_cond_branch_ofs_19 (value >> 2);
	  put_aarch64_insn (buf, insn);
	}
      break;

    case BFD_RELOC_AARCH64_TSTBR14:
      if (fixP->fx_done || !seg->use_rela_p)
	{
	  if (value & 3)
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("conditional branch target not word aligned"));
	  if (signed_overflow (value, 16))
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("conditional branch out of range"));
	  insn = get_aarch64_insn (buf);
	  insn |= encode_tst_branch_ofs_14 (value >> 2);
	  put_aarch64_insn (buf, insn);
	}
      break;

    case BFD_RELOC_AARCH64_CALL26:
    case BFD_RELOC_AARCH64_JUMP26:
      if (fixP->fx_done || !seg->use_rela_p)
	{
	  if (value & 3)
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("branch target not word aligned"));
	  if (signed_overflow (value, 28))
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("branch out of range"));
	  insn = get_aarch64_insn (buf);
	  insn |= encode_branch_ofs_26 (value >> 2);
	  put_aarch64_insn (buf, insn);
	}
      break;

    case BFD_RELOC_AARCH64_MOVW_G0:
    case BFD_RELOC_AARCH64_MOVW_G0_NC:
    case BFD_RELOC_AARCH64_MOVW_G0_S:
    case BFD_RELOC_AARCH64_MOVW_GOTOFF_G0_NC:
    case BFD_RELOC_AARCH64_MOVW_PREL_G0:
    case BFD_RELOC_AARCH64_MOVW_PREL_G0_NC:
      scale = 0;
      goto movw_common;
    case BFD_RELOC_AARCH64_MOVW_G1:
    case BFD_RELOC_AARCH64_MOVW_G1_NC:
    case BFD_RELOC_AARCH64_MOVW_G1_S:
    case BFD_RELOC_AARCH64_MOVW_GOTOFF_G1:
    case BFD_RELOC_AARCH64_MOVW_PREL_G1:
    case BFD_RELOC_AARCH64_MOVW_PREL_G1_NC:
      scale = 16;
      goto movw_common;
    case BFD_RELOC_AARCH64_TLSDESC_OFF_G0_NC:
      scale = 0;
      S_SET_THREAD_LOCAL (fixP->fx_addsy);
      /* Should always be exported to object file, see
	 aarch64_force_relocation().  */
      gas_assert (!fixP->fx_done);
      gas_assert (seg->use_rela_p);
      goto movw_common;
    case BFD_RELOC_AARCH64_TLSDESC_OFF_G1:
      scale = 16;
      S_SET_THREAD_LOCAL (fixP->fx_addsy);
      /* Should always be exported to object file, see
	 aarch64_force_relocation().  */
      gas_assert (!fixP->fx_done);
      gas_assert (seg->use_rela_p);
      goto movw_common;
    case BFD_RELOC_AARCH64_MOVW_G2:
    case BFD_RELOC_AARCH64_MOVW_G2_NC:
    case BFD_RELOC_AARCH64_MOVW_G2_S:
    case BFD_RELOC_AARCH64_MOVW_PREL_G2:
    case BFD_RELOC_AARCH64_MOVW_PREL_G2_NC:
      scale = 32;
      goto movw_common;
    case BFD_RELOC_AARCH64_MOVW_G3:
    case BFD_RELOC_AARCH64_MOVW_PREL_G3:
      scale = 48;
    movw_common:
      if (fixP->fx_done || !seg->use_rela_p)
	{
	  insn = get_aarch64_insn (buf);

	  if (!fixP->fx_done)
	    {
	      /* REL signed addend must fit in 16 bits */
	      if (signed_overflow (value, 16))
		as_bad_where (fixP->fx_file, fixP->fx_line,
			      _("offset out of range"));
	    }
	  else
	    {
	      /* Check for overflow and scale. */
	      switch (fixP->fx_r_type)
		{
		case BFD_RELOC_AARCH64_MOVW_G0:
		case BFD_RELOC_AARCH64_MOVW_G1:
		case BFD_RELOC_AARCH64_MOVW_G2:
		case BFD_RELOC_AARCH64_MOVW_G3:
		case BFD_RELOC_AARCH64_MOVW_GOTOFF_G1:
		case BFD_RELOC_AARCH64_TLSDESC_OFF_G1:
		  if (unsigned_overflow (value, scale + 16))
		    as_bad_where (fixP->fx_file, fixP->fx_line,
				  _("unsigned value out of range"));
		  break;
		case BFD_RELOC_AARCH64_MOVW_G0_S:
		case BFD_RELOC_AARCH64_MOVW_G1_S:
		case BFD_RELOC_AARCH64_MOVW_G2_S:
		case BFD_RELOC_AARCH64_MOVW_PREL_G0:
		case BFD_RELOC_AARCH64_MOVW_PREL_G1:
		case BFD_RELOC_AARCH64_MOVW_PREL_G2:
		  /* NOTE: We can only come here with movz or movn. */
		  if (signed_overflow (value, scale + 16))
		    as_bad_where (fixP->fx_file, fixP->fx_line,
				  _("signed value out of range"));
		  if (value < 0)
		    {
		      /* Force use of MOVN.  */
		      value = ~value;
		      insn = reencode_movzn_to_movn (insn);
		    }
		  else
		    {
		      /* Force use of MOVZ.  */
		      insn = reencode_movzn_to_movz (insn);
		    }
		  break;
		default:
		  /* Unchecked relocations.  */
		  break;
		}
	      value >>= scale;
	    }

	  /* Insert value into MOVN/MOVZ/MOVK instruction. */
	  insn |= encode_movw_imm (value & 0xffff);

	  put_aarch64_insn (buf, insn);
	}
      break;

    case BFD_RELOC_AARCH64_TLSIE_LD_GOTTPREL_LO12_NC:
      fixP->fx_r_type = (ilp32_p
			 ? BFD_RELOC_AARCH64_TLSIE_LD32_GOTTPREL_LO12_NC
			 : BFD_RELOC_AARCH64_TLSIE_LD64_GOTTPREL_LO12_NC);
      S_SET_THREAD_LOCAL (fixP->fx_addsy);
      /* Should always be exported to object file, see
	 aarch64_force_relocation().  */
      gas_assert (!fixP->fx_done);
      gas_assert (seg->use_rela_p);
      break;

    case BFD_RELOC_AARCH64_TLSDESC_LD_LO12_NC:
      fixP->fx_r_type = (ilp32_p
			 ? BFD_RELOC_AARCH64_TLSDESC_LD32_LO12_NC
			 : BFD_RELOC_AARCH64_TLSDESC_LD64_LO12);
      S_SET_THREAD_LOCAL (fixP->fx_addsy);
      /* Should always be exported to object file, see
	 aarch64_force_relocation().  */
      gas_assert (!fixP->fx_done);
      gas_assert (seg->use_rela_p);
      break;

    case BFD_RELOC_AARCH64_TLSDESC_ADD_LO12:
    case BFD_RELOC_AARCH64_TLSDESC_ADR_PAGE21:
    case BFD_RELOC_AARCH64_TLSDESC_ADR_PREL21:
    case BFD_RELOC_AARCH64_TLSDESC_LD32_LO12_NC:
    case BFD_RELOC_AARCH64_TLSDESC_LD64_LO12:
    case BFD_RELOC_AARCH64_TLSDESC_LD_PREL19:
    case BFD_RELOC_AARCH64_TLSGD_ADD_LO12_NC:
    case BFD_RELOC_AARCH64_TLSGD_ADR_PAGE21:
    case BFD_RELOC_AARCH64_TLSGD_ADR_PREL21:
    case BFD_RELOC_AARCH64_TLSGD_MOVW_G0_NC:
    case BFD_RELOC_AARCH64_TLSGD_MOVW_G1:
    case BFD_RELOC_AARCH64_TLSIE_ADR_GOTTPREL_PAGE21:
    case BFD_RELOC_AARCH64_TLSIE_LD32_GOTTPREL_LO12_NC:
    case BFD_RELOC_AARCH64_TLSIE_LD64_GOTTPREL_LO12_NC:
    case BFD_RELOC_AARCH64_TLSIE_LD_GOTTPREL_PREL19:
    case BFD_RELOC_AARCH64_TLSIE_MOVW_GOTTPREL_G0_NC:
    case BFD_RELOC_AARCH64_TLSIE_MOVW_GOTTPREL_G1:
    case BFD_RELOC_AARCH64_TLSLD_ADD_DTPREL_HI12:
    case BFD_RELOC_AARCH64_TLSLD_ADD_DTPREL_LO12:
    case BFD_RELOC_AARCH64_TLSLD_ADD_DTPREL_LO12_NC:
    case BFD_RELOC_AARCH64_TLSLD_ADD_LO12_NC:
    case BFD_RELOC_AARCH64_TLSLD_ADR_PAGE21:
    case BFD_RELOC_AARCH64_TLSLD_ADR_PREL21:
    case BFD_RELOC_AARCH64_TLSLD_LDST16_DTPREL_LO12:
    case BFD_RELOC_AARCH64_TLSLD_LDST16_DTPREL_LO12_NC:
    case BFD_RELOC_AARCH64_TLSLD_LDST32_DTPREL_LO12:
    case BFD_RELOC_AARCH64_TLSLD_LDST32_DTPREL_LO12_NC:
    case BFD_RELOC_AARCH64_TLSLD_LDST64_DTPREL_LO12:
    case BFD_RELOC_AARCH64_TLSLD_LDST64_DTPREL_LO12_NC:
    case BFD_RELOC_AARCH64_TLSLD_LDST8_DTPREL_LO12:
    case BFD_RELOC_AARCH64_TLSLD_LDST8_DTPREL_LO12_NC:
    case BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G0:
    case BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G0_NC:
    case BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G1:
    case BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G1_NC:
    case BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G2:
    case BFD_RELOC_AARCH64_TLSLE_LDST16_TPREL_LO12:
    case BFD_RELOC_AARCH64_TLSLE_LDST16_TPREL_LO12_NC:
    case BFD_RELOC_AARCH64_TLSLE_LDST32_TPREL_LO12:
    case BFD_RELOC_AARCH64_TLSLE_LDST32_TPREL_LO12_NC:
    case BFD_RELOC_AARCH64_TLSLE_LDST64_TPREL_LO12:
    case BFD_RELOC_AARCH64_TLSLE_LDST64_TPREL_LO12_NC:
    case BFD_RELOC_AARCH64_TLSLE_LDST8_TPREL_LO12:
    case BFD_RELOC_AARCH64_TLSLE_LDST8_TPREL_LO12_NC:
    case BFD_RELOC_AARCH64_TLSLE_ADD_TPREL_HI12:
    case BFD_RELOC_AARCH64_TLSLE_ADD_TPREL_LO12:
    case BFD_RELOC_AARCH64_TLSLE_ADD_TPREL_LO12_NC:
    case BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G0:
    case BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G0_NC:
    case BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G1:
    case BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G1_NC:
    case BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G2:
      S_SET_THREAD_LOCAL (fixP->fx_addsy);
      /* Should always be exported to object file, see
	 aarch64_force_relocation().  */
      gas_assert (!fixP->fx_done);
      gas_assert (seg->use_rela_p);
      break;

    case BFD_RELOC_AARCH64_LD_GOT_LO12_NC:
      /* Should always be exported to object file, see
	 aarch64_force_relocation().  */
      fixP->fx_r_type = (ilp32_p
			 ? BFD_RELOC_AARCH64_LD32_GOT_LO12_NC
			 : BFD_RELOC_AARCH64_LD64_GOT_LO12_NC);
      gas_assert (!fixP->fx_done);
      gas_assert (seg->use_rela_p);
      break;

    case BFD_RELOC_AARCH64_ADD_LO12:
    case BFD_RELOC_AARCH64_ADR_GOT_PAGE:
    case BFD_RELOC_AARCH64_ADR_HI21_NC_PCREL:
    case BFD_RELOC_AARCH64_ADR_HI21_PCREL:
    case BFD_RELOC_AARCH64_GOT_LD_PREL19:
    case BFD_RELOC_AARCH64_LD32_GOT_LO12_NC:
    case BFD_RELOC_AARCH64_LD32_GOTPAGE_LO14:
    case BFD_RELOC_AARCH64_LD64_GOTOFF_LO15:
    case BFD_RELOC_AARCH64_LD64_GOTPAGE_LO15:
    case BFD_RELOC_AARCH64_LD64_GOT_LO12_NC:
    case BFD_RELOC_AARCH64_LDST128_LO12:
    case BFD_RELOC_AARCH64_LDST16_LO12:
    case BFD_RELOC_AARCH64_LDST32_LO12:
    case BFD_RELOC_AARCH64_LDST64_LO12:
    case BFD_RELOC_AARCH64_LDST8_LO12:
      /* Should always be exported to object file, see
	 aarch64_force_relocation().  */
      gas_assert (!fixP->fx_done);
      gas_assert (seg->use_rela_p);
      break;

    case BFD_RELOC_AARCH64_TLSDESC_ADD:
    case BFD_RELOC_AARCH64_TLSDESC_CALL:
    case BFD_RELOC_AARCH64_TLSDESC_LDR:
      break;

    case BFD_RELOC_UNUSED:
      /* An error will already have been reported.  */
      break;

    case BFD_RELOC_RVA:
    case BFD_RELOC_32_SECREL:
    case BFD_RELOC_16_SECIDX:
      break;

    default:
      as_bad_where (fixP->fx_file, fixP->fx_line,
		    _("unexpected %s fixup"),
		    bfd_get_reloc_code_name (fixP->fx_r_type));
      break;
    }

 apply_fix_return:
  /* Free the allocated the struct aarch64_inst.
     N.B. currently there are very limited number of fix-up types actually use
     this field, so the impact on the performance should be minimal .  */
  free (fixP->tc_fix_data.inst);

  return;
}

/* Translate internal representation of relocation info to BFD target
   format.  */

arelent *
tc_gen_reloc (asection * section, fixS * fixp)
{
  arelent *reloc;
  bfd_reloc_code_real_type code;

  reloc = XNEW (arelent);

  reloc->sym_ptr_ptr = XNEW (asymbol *);
  *reloc->sym_ptr_ptr = symbol_get_bfdsym (fixp->fx_addsy);
  reloc->address = fixp->fx_frag->fr_address + fixp->fx_where;

  if (fixp->fx_pcrel)
    {
      if (section->use_rela_p)
	fixp->fx_offset -= md_pcrel_from_section (fixp, section);
      else
	fixp->fx_offset = reloc->address;
    }
  reloc->addend = fixp->fx_offset;

  code = fixp->fx_r_type;
  switch (code)
    {
    case BFD_RELOC_16:
      if (fixp->fx_pcrel)
	code = BFD_RELOC_16_PCREL;
      break;

    case BFD_RELOC_32:
      if (fixp->fx_pcrel)
	code = BFD_RELOC_32_PCREL;
      break;

    case BFD_RELOC_64:
      if (fixp->fx_pcrel)
	code = BFD_RELOC_64_PCREL;
      break;

    default:
      break;
    }

  reloc->howto = bfd_reloc_type_lookup (stdoutput, code);
  if (reloc->howto == NULL)
    {
      as_bad_where (fixp->fx_file, fixp->fx_line,
		    _
		    ("cannot represent %s relocation in this object file format"),
		    bfd_get_reloc_code_name (code));
      return NULL;
    }

  return reloc;
}

/* This fix_new is called by cons via TC_CONS_FIX_NEW.	*/

void
cons_fix_new_aarch64 (fragS * frag, int where, int size, expressionS * exp)
{
  bfd_reloc_code_real_type type;
  int pcrel = 0;

#ifdef TE_PE
  if (exp->X_op == O_secrel)
    {
      exp->X_op = O_symbol;
      type = BFD_RELOC_32_SECREL;
    }
  else if (exp->X_op == O_secidx)
    {
      exp->X_op = O_symbol;
      type = BFD_RELOC_16_SECIDX;
    }
  else
    {
#endif
    /* Pick a reloc.
       FIXME: @@ Should look at CPU word size.  */
    switch (size)
      {
      case 1:
	type = BFD_RELOC_8;
	break;
      case 2:
	type = BFD_RELOC_16;
	break;
      case 4:
	type = BFD_RELOC_32;
	break;
      case 8:
	type = BFD_RELOC_64;
	break;
      default:
	as_bad (_("cannot do %u-byte relocation"), size);
	type = BFD_RELOC_UNUSED;
	break;
      }
#ifdef TE_PE
    }
#endif

  fix_new_exp (frag, where, (int) size, exp, pcrel, type);
}

/* Implement md_after_parse_args.  This is the earliest time we need to decide
   ABI.  If no -mabi specified, the ABI will be decided by target triplet.  */

void
aarch64_after_parse_args (void)
{
  if (aarch64_abi != AARCH64_ABI_NONE)
    return;

#ifdef OBJ_ELF
  /* DEFAULT_ARCH will have ":32" extension if it's configured for ILP32.  */
  if (strlen (default_arch) > 7 && strcmp (default_arch + 7, ":32") == 0)
    aarch64_abi = AARCH64_ABI_ILP32;
  else
    aarch64_abi = AARCH64_ABI_LP64;
#else
  aarch64_abi = AARCH64_ABI_LLP64;
#endif
}

#ifdef OBJ_ELF
const char *
elf64_aarch64_target_format (void)
{
#ifdef TE_CLOUDABI
  /* FIXME: What to do for ilp32_p ?  */
  if (target_big_endian)
    return "elf64-bigaarch64-cloudabi";
  else
    return "elf64-littleaarch64-cloudabi";
#else
  if (target_big_endian)
    return ilp32_p ? "elf32-bigaarch64" : "elf64-bigaarch64";
  else
    return ilp32_p ? "elf32-littleaarch64" : "elf64-littleaarch64";
#endif
}

void
aarch64elf_frob_symbol (symbolS * symp, int *puntp)
{
  elf_frob_symbol (symp, puntp);
}
#elif defined OBJ_COFF
const char *
coff_aarch64_target_format (void)
{
  return "pe-aarch64-little";
}
#endif

/* MD interface: Finalization.	*/

/* A good place to do this, although this was probably not intended
   for this kind of use.  We need to dump the literal pool before
   references are made to a null symbol pointer.  */

void
aarch64_cleanup (void)
{
  literal_pool *pool;

  for (pool = list_of_pools; pool; pool = pool->next)
    {
      /* Put it at the end of the relevant section.  */
      subseg_set (pool->section, pool->sub_section);
      s_ltorg (0);
    }
}

#ifdef OBJ_ELF
/* Remove any excess mapping symbols generated for alignment frags in
   SEC.  We may have created a mapping symbol before a zero byte
   alignment; remove it if there's a mapping symbol after the
   alignment.  */
static void
check_mapping_symbols (bfd * abfd ATTRIBUTE_UNUSED, asection * sec,
		       void *dummy ATTRIBUTE_UNUSED)
{
  segment_info_type *seginfo = seg_info (sec);
  fragS *fragp;

  if (seginfo == NULL || seginfo->frchainP == NULL)
    return;

  for (fragp = seginfo->frchainP->frch_root;
       fragp != NULL; fragp = fragp->fr_next)
    {
      symbolS *sym = fragp->tc_frag_data.last_map;
      fragS *next = fragp->fr_next;

      /* Variable-sized frags have been converted to fixed size by
         this point.  But if this was variable-sized to start with,
         there will be a fixed-size frag after it.  So don't handle
         next == NULL.  */
      if (sym == NULL || next == NULL)
	continue;

      if (S_GET_VALUE (sym) < next->fr_address)
	/* Not at the end of this frag.  */
	continue;
      know (S_GET_VALUE (sym) == next->fr_address);

      do
	{
	  if (next->tc_frag_data.first_map != NULL)
	    {
	      /* Next frag starts with a mapping symbol.  Discard this
	         one.  */
	      symbol_remove (sym, &symbol_rootP, &symbol_lastP);
	      break;
	    }

	  if (next->fr_next == NULL)
	    {
	      /* This mapping symbol is at the end of the section.  Discard
	         it.  */
	      know (next->fr_fix == 0 && next->fr_var == 0);
	      symbol_remove (sym, &symbol_rootP, &symbol_lastP);
	      break;
	    }

	  /* As long as we have empty frags without any mapping symbols,
	     keep looking.  */
	  /* If the next frag is non-empty and does not start with a
	     mapping symbol, then this mapping symbol is required.  */
	  if (next->fr_address != next->fr_next->fr_address)
	    break;

	  next = next->fr_next;
	}
      while (next != NULL);
    }
}
#endif

/* Adjust the symbol table.  */

void
aarch64_adjust_symtab (void)
{
#ifdef OBJ_ELF
  /* Remove any overlapping mapping symbols generated by alignment frags.  */
  bfd_map_over_sections (stdoutput, check_mapping_symbols, (char *) 0);
  /* Now do generic ELF adjustments.  */
  elf_adjust_symtab ();
#endif
}

static void
checked_hash_insert (htab_t table, const char *key, void *value)
{
  str_hash_insert (table, key, value, 0);
}

static void
sysreg_hash_insert (htab_t table, const char *key, void *value)
{
  gas_assert (strlen (key) < AARCH64_MAX_SYSREG_NAME_LEN);
  checked_hash_insert (table, key, value);
}

static void
fill_instruction_hash_table (void)
{
  const aarch64_opcode *opcode = aarch64_opcode_table;

  while (opcode->name != NULL)
    {
      templates *templ, *new_templ;
      templ = str_hash_find (aarch64_ops_hsh, opcode->name);

      new_templ = XNEW (templates);
      new_templ->opcode = opcode;
      new_templ->next = NULL;

      if (!templ)
	checked_hash_insert (aarch64_ops_hsh, opcode->name, (void *) new_templ);
      else
	{
	  new_templ->next = templ->next;
	  templ->next = new_templ;
	}
      ++opcode;
    }
}

static inline void
convert_to_upper (char *dst, const char *src, size_t num)
{
  unsigned int i;
  for (i = 0; i < num && *src != '\0'; ++i, ++dst, ++src)
    *dst = TOUPPER (*src);
  *dst = '\0';
}

/* Assume STR point to a lower-case string, allocate, convert and return
   the corresponding upper-case string.  */
static inline const char*
get_upper_str (const char *str)
{
  char *ret;
  size_t len = strlen (str);
  ret = XNEWVEC (char, len + 1);
  convert_to_upper (ret, str, len);
  return ret;
}

/* MD interface: Initialization.  */

void
md_begin (void)
{
  unsigned mach;
  unsigned int i;

  aarch64_ops_hsh = str_htab_create ();
  aarch64_cond_hsh = str_htab_create ();
  aarch64_shift_hsh = str_htab_create ();
  aarch64_sys_regs_hsh = str_htab_create ();
  aarch64_pstatefield_hsh = str_htab_create ();
  aarch64_sys_regs_ic_hsh = str_htab_create ();
  aarch64_sys_regs_dc_hsh = str_htab_create ();
  aarch64_sys_regs_at_hsh = str_htab_create ();
  aarch64_sys_regs_tlbi_hsh = str_htab_create ();
  aarch64_sys_regs_sr_hsh = str_htab_create ();
  aarch64_reg_hsh = str_htab_create ();
  aarch64_barrier_opt_hsh = str_htab_create ();
  aarch64_nzcv_hsh = str_htab_create ();
  aarch64_pldop_hsh = str_htab_create ();
  aarch64_hint_opt_hsh = str_htab_create ();

  fill_instruction_hash_table ();

  for (i = 0; aarch64_sys_regs[i].name != NULL; ++i)
    sysreg_hash_insert (aarch64_sys_regs_hsh, aarch64_sys_regs[i].name,
			 (void *) (aarch64_sys_regs + i));

  for (i = 0; aarch64_pstatefields[i].name != NULL; ++i)
    sysreg_hash_insert (aarch64_pstatefield_hsh,
			 aarch64_pstatefields[i].name,
			 (void *) (aarch64_pstatefields + i));

  for (i = 0; aarch64_sys_regs_ic[i].name != NULL; i++)
    sysreg_hash_insert (aarch64_sys_regs_ic_hsh,
			 aarch64_sys_regs_ic[i].name,
			 (void *) (aarch64_sys_regs_ic + i));

  for (i = 0; aarch64_sys_regs_dc[i].name != NULL; i++)
    sysreg_hash_insert (aarch64_sys_regs_dc_hsh,
			 aarch64_sys_regs_dc[i].name,
			 (void *) (aarch64_sys_regs_dc + i));

  for (i = 0; aarch64_sys_regs_at[i].name != NULL; i++)
    sysreg_hash_insert (aarch64_sys_regs_at_hsh,
			 aarch64_sys_regs_at[i].name,
			 (void *) (aarch64_sys_regs_at + i));

  for (i = 0; aarch64_sys_regs_tlbi[i].name != NULL; i++)
    sysreg_hash_insert (aarch64_sys_regs_tlbi_hsh,
			 aarch64_sys_regs_tlbi[i].name,
			 (void *) (aarch64_sys_regs_tlbi + i));

  for (i = 0; aarch64_sys_regs_sr[i].name != NULL; i++)
    sysreg_hash_insert (aarch64_sys_regs_sr_hsh,
			 aarch64_sys_regs_sr[i].name,
			 (void *) (aarch64_sys_regs_sr + i));

  for (i = 0; i < ARRAY_SIZE (reg_names); i++)
    checked_hash_insert (aarch64_reg_hsh, reg_names[i].name,
			 (void *) (reg_names + i));

  for (i = 0; i < ARRAY_SIZE (nzcv_names); i++)
    checked_hash_insert (aarch64_nzcv_hsh, nzcv_names[i].template,
			 (void *) (nzcv_names + i));

  for (i = 0; aarch64_operand_modifiers[i].name != NULL; i++)
    {
      const char *name = aarch64_operand_modifiers[i].name;
      checked_hash_insert (aarch64_shift_hsh, name,
			   (void *) (aarch64_operand_modifiers + i));
      /* Also hash the name in the upper case.  */
      checked_hash_insert (aarch64_shift_hsh, get_upper_str (name),
			   (void *) (aarch64_operand_modifiers + i));
    }

  for (i = 0; i < ARRAY_SIZE (aarch64_conds); i++)
    {
      unsigned int j;
      /* A condition code may have alias(es), e.g. "cc", "lo" and "ul" are
	 the same condition code.  */
      for (j = 0; j < ARRAY_SIZE (aarch64_conds[i].names); ++j)
	{
	  const char *name = aarch64_conds[i].names[j];
	  if (name == NULL)
	    break;
	  checked_hash_insert (aarch64_cond_hsh, name,
			       (void *) (aarch64_conds + i));
	  /* Also hash the name in the upper case.  */
	  checked_hash_insert (aarch64_cond_hsh, get_upper_str (name),
			       (void *) (aarch64_conds + i));
	}
    }

  for (i = 0; i < ARRAY_SIZE (aarch64_barrier_options); i++)
    {
      const char *name = aarch64_barrier_options[i].name;
      /* Skip xx00 - the unallocated values of option.  */
      if ((i & 0x3) == 0)
	continue;
      checked_hash_insert (aarch64_barrier_opt_hsh, name,
			   (void *) (aarch64_barrier_options + i));
      /* Also hash the name in the upper case.  */
      checked_hash_insert (aarch64_barrier_opt_hsh, get_upper_str (name),
			   (void *) (aarch64_barrier_options + i));
    }

  for (i = 0; i < ARRAY_SIZE (aarch64_barrier_dsb_nxs_options); i++)
    {
      const char *name = aarch64_barrier_dsb_nxs_options[i].name;
      checked_hash_insert (aarch64_barrier_opt_hsh, name,
			   (void *) (aarch64_barrier_dsb_nxs_options + i));
      /* Also hash the name in the upper case.  */
      checked_hash_insert (aarch64_barrier_opt_hsh, get_upper_str (name),
			   (void *) (aarch64_barrier_dsb_nxs_options + i));
    }

  for (i = 0; i < ARRAY_SIZE (aarch64_prfops); i++)
    {
      const char* name = aarch64_prfops[i].name;
      /* Skip the unallocated hint encodings.  */
      if (name == NULL)
	continue;
      checked_hash_insert (aarch64_pldop_hsh, name,
			   (void *) (aarch64_prfops + i));
      /* Also hash the name in the upper case.  */
      checked_hash_insert (aarch64_pldop_hsh, get_upper_str (name),
			   (void *) (aarch64_prfops + i));
    }

  for (i = 0; aarch64_hint_options[i].name != NULL; i++)
    {
      const char* name = aarch64_hint_options[i].name;
      const char* upper_name = get_upper_str(name);

      checked_hash_insert (aarch64_hint_opt_hsh, name,
			   (void *) (aarch64_hint_options + i));

      /* Also hash the name in the upper case if not the same.  */
      if (strcmp (name, upper_name) != 0)
	checked_hash_insert (aarch64_hint_opt_hsh, upper_name,
			     (void *) (aarch64_hint_options + i));
    }

  /* Set the cpu variant based on the command-line options.  */
  if (!mcpu_cpu_opt)
    mcpu_cpu_opt = march_cpu_opt;

  if (!mcpu_cpu_opt)
    mcpu_cpu_opt = &cpu_default;

  cpu_variant = *mcpu_cpu_opt;

  /* Record the CPU type.  */
  if(ilp32_p)
    mach = bfd_mach_aarch64_ilp32;
  else if (llp64_p)
    mach = bfd_mach_aarch64_llp64;
  else
    mach = bfd_mach_aarch64;

  bfd_set_arch_mach (stdoutput, TARGET_ARCH, mach);
#ifdef OBJ_ELF
  /* FIXME - is there a better way to do it ?  */
  aarch64_sframe_cfa_sp_reg = 31;
  aarch64_sframe_cfa_fp_reg = 29; /* x29.  */
  aarch64_sframe_cfa_ra_reg = 30;
#endif
}

/* Command line processing.  */

const char *md_shortopts = "m:";

#ifdef AARCH64_BI_ENDIAN
#define OPTION_EB (OPTION_MD_BASE + 0)
#define OPTION_EL (OPTION_MD_BASE + 1)
#else
#if TARGET_BYTES_BIG_ENDIAN
#define OPTION_EB (OPTION_MD_BASE + 0)
#else
#define OPTION_EL (OPTION_MD_BASE + 1)
#endif
#endif

struct option md_longopts[] = {
#ifdef OPTION_EB
  {"EB", no_argument, NULL, OPTION_EB},
#endif
#ifdef OPTION_EL
  {"EL", no_argument, NULL, OPTION_EL},
#endif
  {NULL, no_argument, NULL, 0}
};

size_t md_longopts_size = sizeof (md_longopts);

struct aarch64_option_table
{
  const char *option;			/* Option name to match.  */
  const char *help;			/* Help information.  */
  int *var;			/* Variable to change.  */
  int value;			/* What to change it to.  */
  char *deprecated;		/* If non-null, print this message.  */
};

static struct aarch64_option_table aarch64_opts[] = {
  {"mbig-endian", N_("assemble for big-endian"), &target_big_endian, 1, NULL},
  {"mlittle-endian", N_("assemble for little-endian"), &target_big_endian, 0,
   NULL},
#ifdef DEBUG_AARCH64
  {"mdebug-dump", N_("temporary switch for dumping"), &debug_dump, 1, NULL},
#endif /* DEBUG_AARCH64 */
  {"mverbose-error", N_("output verbose error messages"), &verbose_error_p, 1,
   NULL},
  {"mno-verbose-error", N_("do not output verbose error messages"),
   &verbose_error_p, 0, NULL},
  {NULL, NULL, NULL, 0, NULL}
};

struct aarch64_cpu_option_table
{
  const char *name;
  const aarch64_feature_set value;
  /* The canonical name of the CPU, or NULL to use NAME converted to upper
     case.  */
  const char *canonical_name;
};

/* This list should, at a minimum, contain all the cpu names
   recognized by GCC.  */
static const struct aarch64_cpu_option_table aarch64_cpus[] = {
  {"all", AARCH64_ANY, NULL},
  {"cortex-a34", AARCH64_FEATURE (AARCH64_ARCH_V8,
				  AARCH64_FEATURE_CRC), "Cortex-A34"},
  {"cortex-a35", AARCH64_FEATURE (AARCH64_ARCH_V8,
				  AARCH64_FEATURE_CRC), "Cortex-A35"},
  {"cortex-a53", AARCH64_FEATURE (AARCH64_ARCH_V8,
				  AARCH64_FEATURE_CRC), "Cortex-A53"},
  {"cortex-a57", AARCH64_FEATURE (AARCH64_ARCH_V8,
				  AARCH64_FEATURE_CRC), "Cortex-A57"},
  {"cortex-a72", AARCH64_FEATURE (AARCH64_ARCH_V8,
				  AARCH64_FEATURE_CRC), "Cortex-A72"},
  {"cortex-a73", AARCH64_FEATURE (AARCH64_ARCH_V8,
				  AARCH64_FEATURE_CRC), "Cortex-A73"},
  {"cortex-a55", AARCH64_FEATURE (AARCH64_ARCH_V8_2,
				  AARCH64_FEATURE_RCPC | AARCH64_FEATURE_F16 | AARCH64_FEATURE_DOTPROD),
				  "Cortex-A55"},
  {"cortex-a75", AARCH64_FEATURE (AARCH64_ARCH_V8_2,
				  AARCH64_FEATURE_RCPC | AARCH64_FEATURE_F16 | AARCH64_FEATURE_DOTPROD),
				  "Cortex-A75"},
  {"cortex-a76", AARCH64_FEATURE (AARCH64_ARCH_V8_2,
				  AARCH64_FEATURE_RCPC | AARCH64_FEATURE_F16 | AARCH64_FEATURE_DOTPROD),
				  "Cortex-A76"},
  {"cortex-a76ae", AARCH64_FEATURE (AARCH64_ARCH_V8_2,
				    AARCH64_FEATURE_F16 | AARCH64_FEATURE_RCPC
				    | AARCH64_FEATURE_DOTPROD
				    | AARCH64_FEATURE_SSBS),
				    "Cortex-A76AE"},
  {"cortex-a77", AARCH64_FEATURE (AARCH64_ARCH_V8_2,
				  AARCH64_FEATURE_F16 | AARCH64_FEATURE_RCPC
				  | AARCH64_FEATURE_DOTPROD
				  | AARCH64_FEATURE_SSBS),
				  "Cortex-A77"},
  {"cortex-a65", AARCH64_FEATURE (AARCH64_ARCH_V8_2,
				  AARCH64_FEATURE_F16 | AARCH64_FEATURE_RCPC
				  | AARCH64_FEATURE_DOTPROD
				  | AARCH64_FEATURE_SSBS),
				  "Cortex-A65"},
  {"cortex-a65ae", AARCH64_FEATURE (AARCH64_ARCH_V8_2,
				    AARCH64_FEATURE_F16 | AARCH64_FEATURE_RCPC
				    | AARCH64_FEATURE_DOTPROD
				    | AARCH64_FEATURE_SSBS),
				    "Cortex-A65AE"},
  {"cortex-a78", AARCH64_FEATURE (AARCH64_ARCH_V8_2,
                 AARCH64_FEATURE_F16
                 | AARCH64_FEATURE_RCPC
                 | AARCH64_FEATURE_DOTPROD
                 | AARCH64_FEATURE_SSBS
                 | AARCH64_FEATURE_PROFILE),
   "Cortex-A78"},
  {"cortex-a78ae", AARCH64_FEATURE (AARCH64_ARCH_V8_2,
                   AARCH64_FEATURE_F16
                   | AARCH64_FEATURE_RCPC
                   | AARCH64_FEATURE_DOTPROD
                   | AARCH64_FEATURE_SSBS
                   | AARCH64_FEATURE_PROFILE),
   "Cortex-A78AE"},
  {"cortex-a78c", AARCH64_FEATURE (AARCH64_ARCH_V8_2,
                   AARCH64_FEATURE_DOTPROD
                   | AARCH64_FEATURE_F16
                   | AARCH64_FEATURE_FLAGM
                   | AARCH64_FEATURE_PAC
                   | AARCH64_FEATURE_PROFILE
                   | AARCH64_FEATURE_RCPC
                   | AARCH64_FEATURE_SSBS),
   "Cortex-A78C"},
  {"cortex-a510", AARCH64_FEATURE (AARCH64_ARCH_V9,
                  AARCH64_FEATURE_BFLOAT16
                  | AARCH64_FEATURE_I8MM
                  | AARCH64_FEATURE_MEMTAG
                  | AARCH64_FEATURE_SVE2_BITPERM),
   "Cortex-A510"},
  {"cortex-a710", AARCH64_FEATURE (AARCH64_ARCH_V9,
                  AARCH64_FEATURE_BFLOAT16
                  | AARCH64_FEATURE_I8MM
                  | AARCH64_FEATURE_MEMTAG
                  | AARCH64_FEATURE_SVE2_BITPERM),
   "Cortex-A710"},
  {"ares", AARCH64_FEATURE (AARCH64_ARCH_V8_2,
				  AARCH64_FEATURE_RCPC | AARCH64_FEATURE_F16
				  | AARCH64_FEATURE_DOTPROD
				  | AARCH64_FEATURE_PROFILE),
				  "Ares"},
  {"exynos-m1", AARCH64_FEATURE (AARCH64_ARCH_V8,
				 AARCH64_FEATURE_CRC | AARCH64_FEATURE_CRYPTO),
				"Samsung Exynos M1"},
  {"falkor", AARCH64_FEATURE (AARCH64_ARCH_V8,
			      AARCH64_FEATURE_CRC | AARCH64_FEATURE_CRYPTO
			      | AARCH64_FEATURE_RDMA),
   "Qualcomm Falkor"},
  {"neoverse-e1", AARCH64_FEATURE (AARCH64_ARCH_V8_2,
				  AARCH64_FEATURE_RCPC | AARCH64_FEATURE_F16
				  | AARCH64_FEATURE_DOTPROD
				  | AARCH64_FEATURE_SSBS),
				  "Neoverse E1"},
  {"neoverse-n1", AARCH64_FEATURE (AARCH64_ARCH_V8_2,
				  AARCH64_FEATURE_RCPC | AARCH64_FEATURE_F16
				  | AARCH64_FEATURE_DOTPROD
				  | AARCH64_FEATURE_PROFILE),
				  "Neoverse N1"},
  {"neoverse-n2", AARCH64_FEATURE (AARCH64_ARCH_V8_5,
				   AARCH64_FEATURE_BFLOAT16
				 | AARCH64_FEATURE_I8MM
				 | AARCH64_FEATURE_F16
				 | AARCH64_FEATURE_SVE
				 | AARCH64_FEATURE_SVE2
				 | AARCH64_FEATURE_SVE2_BITPERM
				 | AARCH64_FEATURE_MEMTAG
				 | AARCH64_FEATURE_RNG),
				 "Neoverse N2"},
  {"neoverse-v1", AARCH64_FEATURE (AARCH64_ARCH_V8_4,
			    AARCH64_FEATURE_PROFILE
			  | AARCH64_FEATURE_CVADP
			  | AARCH64_FEATURE_SVE
			  | AARCH64_FEATURE_SSBS
			  | AARCH64_FEATURE_RNG
			  | AARCH64_FEATURE_F16
			  | AARCH64_FEATURE_BFLOAT16
			  | AARCH64_FEATURE_I8MM), "Neoverse V1"},
  {"qdf24xx", AARCH64_FEATURE (AARCH64_ARCH_V8,
			       AARCH64_FEATURE_CRC | AARCH64_FEATURE_CRYPTO
			       | AARCH64_FEATURE_RDMA),
   "Qualcomm QDF24XX"},
  {"saphira", AARCH64_FEATURE (AARCH64_ARCH_V8_4,
			       AARCH64_FEATURE_CRYPTO | AARCH64_FEATURE_PROFILE),
   "Qualcomm Saphira"},
  {"thunderx", AARCH64_FEATURE (AARCH64_ARCH_V8,
				AARCH64_FEATURE_CRC | AARCH64_FEATURE_CRYPTO),
   "Cavium ThunderX"},
  {"vulcan", AARCH64_FEATURE (AARCH64_ARCH_V8_1,
			      AARCH64_FEATURE_CRYPTO),
  "Broadcom Vulcan"},
  /* The 'xgene-1' name is an older name for 'xgene1', which was used
     in earlier releases and is superseded by 'xgene1' in all
     tools.  */
  {"xgene-1", AARCH64_ARCH_V8, "APM X-Gene 1"},
  {"xgene1", AARCH64_ARCH_V8, "APM X-Gene 1"},
  {"xgene2", AARCH64_FEATURE (AARCH64_ARCH_V8,
			      AARCH64_FEATURE_CRC), "APM X-Gene 2"},
  {"cortex-r82", AARCH64_ARCH_V8_R, "Cortex-R82"},
  {"cortex-x1", AARCH64_FEATURE (AARCH64_ARCH_V8_2,
                AARCH64_FEATURE_F16
                | AARCH64_FEATURE_RCPC
                | AARCH64_FEATURE_DOTPROD
                | AARCH64_FEATURE_SSBS
                | AARCH64_FEATURE_PROFILE),
                "Cortex-X1"},
  {"cortex-x2", AARCH64_FEATURE (AARCH64_ARCH_V9,
                AARCH64_FEATURE_BFLOAT16
                | AARCH64_FEATURE_I8MM
                | AARCH64_FEATURE_MEMTAG
                | AARCH64_FEATURE_SVE2_BITPERM),
                "Cortex-X2"},
  {"generic", AARCH64_ARCH_V8, NULL},

  {NULL, AARCH64_ARCH_NONE, NULL}
};

struct aarch64_arch_option_table
{
  const char *name;
  const aarch64_feature_set value;
};

/* This list should, at a minimum, contain all the architecture names
   recognized by GCC.  */
static const struct aarch64_arch_option_table aarch64_archs[] = {
  {"all", AARCH64_ANY},
  {"armv8-a", AARCH64_ARCH_V8},
  {"armv8.1-a", AARCH64_ARCH_V8_1},
  {"armv8.2-a", AARCH64_ARCH_V8_2},
  {"armv8.3-a", AARCH64_ARCH_V8_3},
  {"armv8.4-a", AARCH64_ARCH_V8_4},
  {"armv8.5-a", AARCH64_ARCH_V8_5},
  {"armv8.6-a", AARCH64_ARCH_V8_6},
  {"armv8.7-a", AARCH64_ARCH_V8_7},
  {"armv8.8-a", AARCH64_ARCH_V8_8},
  {"armv8-r",	AARCH64_ARCH_V8_R},
  {"armv9-a",	AARCH64_ARCH_V9},
  {"armv9.1-a",	AARCH64_ARCH_V9_1},
  {"armv9.2-a",	AARCH64_ARCH_V9_2},
  {"armv9.3-a",	AARCH64_ARCH_V9_3},
  {NULL, AARCH64_ARCH_NONE}
};

/* ISA extensions.  */
struct aarch64_option_cpu_value_table
{
  const char *name;
  const aarch64_feature_set value;
  const aarch64_feature_set require; /* Feature dependencies.  */
};

static const struct aarch64_option_cpu_value_table aarch64_features[] = {
  {"crc",		AARCH64_FEATURE (AARCH64_FEATURE_CRC, 0),
			AARCH64_ARCH_NONE},
  {"crypto",		AARCH64_FEATURE (AARCH64_FEATURE_CRYPTO, 0),
			AARCH64_FEATURE (AARCH64_FEATURE_SIMD, 0)},
  {"fp",		AARCH64_FEATURE (AARCH64_FEATURE_FP, 0),
			AARCH64_ARCH_NONE},
  {"lse",		AARCH64_FEATURE (AARCH64_FEATURE_LSE, 0),
			AARCH64_ARCH_NONE},
  {"simd",		AARCH64_FEATURE (AARCH64_FEATURE_SIMD, 0),
			AARCH64_FEATURE (AARCH64_FEATURE_FP, 0)},
  {"pan",		AARCH64_FEATURE (AARCH64_FEATURE_PAN, 0),
			AARCH64_ARCH_NONE},
  {"lor",		AARCH64_FEATURE (AARCH64_FEATURE_LOR, 0),
			AARCH64_ARCH_NONE},
  {"ras",		AARCH64_FEATURE (AARCH64_FEATURE_RAS, 0),
			AARCH64_ARCH_NONE},
  {"rdma",		AARCH64_FEATURE (AARCH64_FEATURE_RDMA, 0),
			AARCH64_FEATURE (AARCH64_FEATURE_SIMD, 0)},
  {"fp16",		AARCH64_FEATURE (AARCH64_FEATURE_F16, 0),
			AARCH64_FEATURE (AARCH64_FEATURE_FP, 0)},
  {"fp16fml",		AARCH64_FEATURE (AARCH64_FEATURE_F16_FML, 0),
			AARCH64_FEATURE (AARCH64_FEATURE_F16, 0)},
  {"profile",		AARCH64_FEATURE (AARCH64_FEATURE_PROFILE, 0),
			AARCH64_ARCH_NONE},
  {"sve",		AARCH64_FEATURE (AARCH64_FEATURE_SVE, 0),
			AARCH64_FEATURE (AARCH64_FEATURE_COMPNUM, 0)},
  {"tme",		AARCH64_FEATURE (AARCH64_FEATURE_TME, 0),
			AARCH64_ARCH_NONE},
  {"compnum",		AARCH64_FEATURE (AARCH64_FEATURE_COMPNUM, 0),
			AARCH64_FEATURE (AARCH64_FEATURE_F16
					 | AARCH64_FEATURE_SIMD, 0)},
  {"rcpc",		AARCH64_FEATURE (AARCH64_FEATURE_RCPC, 0),
			AARCH64_ARCH_NONE},
  {"dotprod",		AARCH64_FEATURE (AARCH64_FEATURE_DOTPROD, 0),
			AARCH64_FEATURE (AARCH64_FEATURE_SIMD, 0)},
  {"sha2",		AARCH64_FEATURE (AARCH64_FEATURE_SHA2, 0),
			AARCH64_FEATURE (AARCH64_FEATURE_FP, 0)},
  {"sb",		AARCH64_FEATURE (AARCH64_FEATURE_SB, 0),
			AARCH64_ARCH_NONE},
  {"predres",		AARCH64_FEATURE (AARCH64_FEATURE_PREDRES, 0),
			AARCH64_ARCH_NONE},
  {"aes",		AARCH64_FEATURE (AARCH64_FEATURE_AES, 0),
			AARCH64_FEATURE (AARCH64_FEATURE_SIMD, 0)},
  {"sm4",		AARCH64_FEATURE (AARCH64_FEATURE_SM4, 0),
			AARCH64_FEATURE (AARCH64_FEATURE_SIMD, 0)},
  {"sha3",		AARCH64_FEATURE (AARCH64_FEATURE_SHA3, 0),
			AARCH64_FEATURE (AARCH64_FEATURE_SHA2, 0)},
  {"rng",		AARCH64_FEATURE (AARCH64_FEATURE_RNG, 0),
			AARCH64_ARCH_NONE},
  {"ssbs",		AARCH64_FEATURE (AARCH64_FEATURE_SSBS, 0),
			AARCH64_ARCH_NONE},
  {"memtag",		AARCH64_FEATURE (AARCH64_FEATURE_MEMTAG, 0),
			AARCH64_ARCH_NONE},
  {"sve2",		AARCH64_FEATURE (AARCH64_FEATURE_SVE2, 0),
			AARCH64_FEATURE (AARCH64_FEATURE_SVE, 0)},
  {"sve2-sm4",		AARCH64_FEATURE (AARCH64_FEATURE_SVE2_SM4, 0),
			AARCH64_FEATURE (AARCH64_FEATURE_SVE2
					 | AARCH64_FEATURE_SM4, 0)},
  {"sve2-aes",		AARCH64_FEATURE (AARCH64_FEATURE_SVE2_AES, 0),
			AARCH64_FEATURE (AARCH64_FEATURE_SVE2
					 | AARCH64_FEATURE_AES, 0)},
  {"sve2-sha3",		AARCH64_FEATURE (AARCH64_FEATURE_SVE2_SHA3, 0),
			AARCH64_FEATURE (AARCH64_FEATURE_SVE2
					 | AARCH64_FEATURE_SHA3, 0)},
  {"sve2-bitperm",	AARCH64_FEATURE (AARCH64_FEATURE_SVE2_BITPERM, 0),
			AARCH64_FEATURE (AARCH64_FEATURE_SVE2, 0)},
  {"sme",		AARCH64_FEATURE (AARCH64_FEATURE_SME, 0),
			AARCH64_FEATURE (AARCH64_FEATURE_SVE2
					 | AARCH64_FEATURE_BFLOAT16, 0)},
  {"sme-f64",		AARCH64_FEATURE (AARCH64_FEATURE_SME_F64F64, 0),
			AARCH64_FEATURE (AARCH64_FEATURE_SME, 0)},
  {"sme-f64f64",	AARCH64_FEATURE (AARCH64_FEATURE_SME_F64F64, 0),
			AARCH64_FEATURE (AARCH64_FEATURE_SME, 0)},
  {"sme-i64",		AARCH64_FEATURE (AARCH64_FEATURE_SME_I16I64, 0),
			AARCH64_FEATURE (AARCH64_FEATURE_SME, 0)},
  {"sme-i16i64",	AARCH64_FEATURE (AARCH64_FEATURE_SME_I16I64, 0),
			AARCH64_FEATURE (AARCH64_FEATURE_SME, 0)},
  {"sme2",		AARCH64_FEATURE (AARCH64_FEATURE_SME2, 0),
			AARCH64_FEATURE (AARCH64_FEATURE_SME, 0)},
  {"bf16",		AARCH64_FEATURE (AARCH64_FEATURE_BFLOAT16, 0),
			AARCH64_FEATURE (AARCH64_FEATURE_FP, 0)},
  {"i8mm",		AARCH64_FEATURE (AARCH64_FEATURE_I8MM, 0),
			AARCH64_FEATURE (AARCH64_FEATURE_SIMD, 0)},
  {"f32mm",		AARCH64_FEATURE (AARCH64_FEATURE_F32MM, 0),
			AARCH64_FEATURE (AARCH64_FEATURE_SVE, 0)},
  {"f64mm",		AARCH64_FEATURE (AARCH64_FEATURE_F64MM, 0),
			AARCH64_FEATURE (AARCH64_FEATURE_SVE, 0)},
  {"ls64",		AARCH64_FEATURE (AARCH64_FEATURE_LS64, 0),
			AARCH64_ARCH_NONE},
  {"flagm",		AARCH64_FEATURE (AARCH64_FEATURE_FLAGM, 0),
			AARCH64_ARCH_NONE},
  {"pauth",		AARCH64_FEATURE (AARCH64_FEATURE_PAC, 0),
			AARCH64_ARCH_NONE},
  {"mops",		AARCH64_FEATURE (AARCH64_FEATURE_MOPS, 0),
			AARCH64_ARCH_NONE},
  {"hbc",		AARCH64_FEATURE (AARCH64_FEATURE_HBC, 0),
			AARCH64_ARCH_NONE},
  {"cssc",		AARCH64_FEATURE (AARCH64_FEATURE_CSSC, 0),
			AARCH64_ARCH_NONE},
  {NULL,		AARCH64_ARCH_NONE, AARCH64_ARCH_NONE},
};

struct aarch64_long_option_table
{
  const char *option;			/* Substring to match.  */
  const char *help;			/* Help information.  */
  int (*func) (const char *subopt);	/* Function to decode sub-option.  */
  char *deprecated;		/* If non-null, print this message.  */
};

/* Transitive closure of features depending on set.  */
static aarch64_feature_set
aarch64_feature_disable_set (aarch64_feature_set set)
{
  const struct aarch64_option_cpu_value_table *opt;
  aarch64_feature_set prev = 0;

  while (prev != set) {
    prev = set;
    for (opt = aarch64_features; opt->name != NULL; opt++)
      if (AARCH64_CPU_HAS_ANY_FEATURES (opt->require, set))
        AARCH64_MERGE_FEATURE_SETS (set, set, opt->value);
  }
  return set;
}

/* Transitive closure of dependencies of set.  */
static aarch64_feature_set
aarch64_feature_enable_set (aarch64_feature_set set)
{
  const struct aarch64_option_cpu_value_table *opt;
  aarch64_feature_set prev = 0;

  while (prev != set) {
    prev = set;
    for (opt = aarch64_features; opt->name != NULL; opt++)
      if (AARCH64_CPU_HAS_FEATURE (set, opt->value))
        AARCH64_MERGE_FEATURE_SETS (set, set, opt->require);
  }
  return set;
}

static int
aarch64_parse_features (const char *str, const aarch64_feature_set **opt_p,
			bool ext_only)
{
  /* We insist on extensions being added before being removed.  We achieve
     this by using the ADDING_VALUE variable to indicate whether we are
     adding an extension (1) or removing it (0) and only allowing it to
     change in the order -1 -> 1 -> 0.  */
  int adding_value = -1;
  aarch64_feature_set *ext_set = XNEW (aarch64_feature_set);

  /* Copy the feature set, so that we can modify it.  */
  *ext_set = **opt_p;
  *opt_p = ext_set;

  while (str != NULL && *str != 0)
    {
      const struct aarch64_option_cpu_value_table *opt;
      const char *ext = NULL;
      int optlen;

      if (!ext_only)
	{
	  if (*str != '+')
	    {
	      as_bad (_("invalid architectural extension"));
	      return 0;
	    }

	  ext = strchr (++str, '+');
	}

      if (ext != NULL)
	optlen = ext - str;
      else
	optlen = strlen (str);

      if (optlen >= 2 && startswith (str, "no"))
	{
	  if (adding_value != 0)
	    adding_value = 0;
	  optlen -= 2;
	  str += 2;
	}
      else if (optlen > 0)
	{
	  if (adding_value == -1)
	    adding_value = 1;
	  else if (adding_value != 1)
	    {
	      as_bad (_("must specify extensions to add before specifying "
			"those to remove"));
	      return false;
	    }
	}

      if (optlen == 0)
	{
	  as_bad (_("missing architectural extension"));
	  return 0;
	}

      gas_assert (adding_value != -1);

      for (opt = aarch64_features; opt->name != NULL; opt++)
	if (strncmp (opt->name, str, optlen) == 0)
	  {
	    aarch64_feature_set set;

	    /* Add or remove the extension.  */
	    if (adding_value)
	      {
		set = aarch64_feature_enable_set (opt->value);
		AARCH64_MERGE_FEATURE_SETS (*ext_set, *ext_set, set);
	      }
	    else
	      {
		set = aarch64_feature_disable_set (opt->value);
		AARCH64_CLEAR_FEATURE (*ext_set, *ext_set, set);
	      }
	    break;
	  }

      if (opt->name == NULL)
	{
	  as_bad (_("unknown architectural extension `%s'"), str);
	  return 0;
	}

      str = ext;
    };

  return 1;
}

static int
aarch64_parse_cpu (const char *str)
{
  const struct aarch64_cpu_option_table *opt;
  const char *ext = strchr (str, '+');
  size_t optlen;

  if (ext != NULL)
    optlen = ext - str;
  else
    optlen = strlen (str);

  if (optlen == 0)
    {
      as_bad (_("missing cpu name `%s'"), str);
      return 0;
    }

  for (opt = aarch64_cpus; opt->name != NULL; opt++)
    if (strlen (opt->name) == optlen && strncmp (str, opt->name, optlen) == 0)
      {
	mcpu_cpu_opt = &opt->value;
	if (ext != NULL)
	  return aarch64_parse_features (ext, &mcpu_cpu_opt, false);

	return 1;
      }

  as_bad (_("unknown cpu `%s'"), str);
  return 0;
}

static int
aarch64_parse_arch (const char *str)
{
  const struct aarch64_arch_option_table *opt;
  const char *ext = strchr (str, '+');
  size_t optlen;

  if (ext != NULL)
    optlen = ext - str;
  else
    optlen = strlen (str);

  if (optlen == 0)
    {
      as_bad (_("missing architecture name `%s'"), str);
      return 0;
    }

  for (opt = aarch64_archs; opt->name != NULL; opt++)
    if (strlen (opt->name) == optlen && strncmp (str, opt->name, optlen) == 0)
      {
	march_cpu_opt = &opt->value;
	if (ext != NULL)
	  return aarch64_parse_features (ext, &march_cpu_opt, false);

	return 1;
      }

  as_bad (_("unknown architecture `%s'\n"), str);
  return 0;
}

/* ABIs.  */
struct aarch64_option_abi_value_table
{
  const char *name;
  enum aarch64_abi_type value;
};

static const struct aarch64_option_abi_value_table aarch64_abis[] = {
#ifdef OBJ_ELF
  {"ilp32",		AARCH64_ABI_ILP32},
  {"lp64",		AARCH64_ABI_LP64},
#else
  {"llp64",		AARCH64_ABI_LLP64},
#endif
};

static int
aarch64_parse_abi (const char *str)
{
  unsigned int i;

  if (str[0] == '\0')
    {
      as_bad (_("missing abi name `%s'"), str);
      return 0;
    }

  for (i = 0; i < ARRAY_SIZE (aarch64_abis); i++)
    if (strcmp (str, aarch64_abis[i].name) == 0)
      {
	aarch64_abi = aarch64_abis[i].value;
	return 1;
      }

  as_bad (_("unknown abi `%s'\n"), str);
  return 0;
}

static struct aarch64_long_option_table aarch64_long_opts[] = {
  {"mabi=", N_("<abi name>\t  specify for ABI <abi name>"),
   aarch64_parse_abi, NULL},
  {"mcpu=", N_("<cpu name>\t  assemble for CPU <cpu name>"),
   aarch64_parse_cpu, NULL},
  {"march=", N_("<arch name>\t  assemble for architecture <arch name>"),
   aarch64_parse_arch, NULL},
  {NULL, NULL, 0, NULL}
};

int
md_parse_option (int c, const char *arg)
{
  struct aarch64_option_table *opt;
  struct aarch64_long_option_table *lopt;

  switch (c)
    {
#ifdef OPTION_EB
    case OPTION_EB:
      target_big_endian = 1;
      break;
#endif

#ifdef OPTION_EL
    case OPTION_EL:
      target_big_endian = 0;
      break;
#endif

    case 'a':
      /* Listing option.  Just ignore these, we don't support additional
         ones.  */
      return 0;

    default:
      for (opt = aarch64_opts; opt->option != NULL; opt++)
	{
	  if (c == opt->option[0]
	      && ((arg == NULL && opt->option[1] == 0)
		  || streq (arg, opt->option + 1)))
	    {
	      /* If the option is deprecated, tell the user.  */
	      if (opt->deprecated != NULL)
		as_tsktsk (_("option `-%c%s' is deprecated: %s"), c,
			   arg ? arg : "", _(opt->deprecated));

	      if (opt->var != NULL)
		*opt->var = opt->value;

	      return 1;
	    }
	}

      for (lopt = aarch64_long_opts; lopt->option != NULL; lopt++)
	{
	  /* These options are expected to have an argument.  */
	  if (c == lopt->option[0]
	      && arg != NULL
	      && startswith (arg, lopt->option + 1))
	    {
	      /* If the option is deprecated, tell the user.  */
	      if (lopt->deprecated != NULL)
		as_tsktsk (_("option `-%c%s' is deprecated: %s"), c, arg,
			   _(lopt->deprecated));

	      /* Call the sup-option parser.  */
	      return lopt->func (arg + strlen (lopt->option) - 1);
	    }
	}

      return 0;
    }

  return 1;
}

void
md_show_usage (FILE * fp)
{
  struct aarch64_option_table *opt;
  struct aarch64_long_option_table *lopt;

  fprintf (fp, _(" AArch64-specific assembler options:\n"));

  for (opt = aarch64_opts; opt->option != NULL; opt++)
    if (opt->help != NULL)
      fprintf (fp, "  -%-23s%s\n", opt->option, _(opt->help));

  for (lopt = aarch64_long_opts; lopt->option != NULL; lopt++)
    if (lopt->help != NULL)
      fprintf (fp, "  -%s%s\n", lopt->option, _(lopt->help));

#ifdef OPTION_EB
  fprintf (fp, _("\
  -EB                     assemble code for a big-endian cpu\n"));
#endif

#ifdef OPTION_EL
  fprintf (fp, _("\
  -EL                     assemble code for a little-endian cpu\n"));
#endif
}

/* Parse a .cpu directive.  */

static void
s_aarch64_cpu (int ignored ATTRIBUTE_UNUSED)
{
  const struct aarch64_cpu_option_table *opt;
  char saved_char;
  char *name;
  char *ext;
  size_t optlen;

  name = input_line_pointer;
  input_line_pointer = find_end_of_line (input_line_pointer, flag_m68k_mri);
  saved_char = *input_line_pointer;
  *input_line_pointer = 0;

  ext = strchr (name, '+');

  if (ext != NULL)
    optlen = ext - name;
  else
    optlen = strlen (name);

  /* Skip the first "all" entry.  */
  for (opt = aarch64_cpus + 1; opt->name != NULL; opt++)
    if (strlen (opt->name) == optlen
	&& strncmp (name, opt->name, optlen) == 0)
      {
	mcpu_cpu_opt = &opt->value;
	if (ext != NULL)
	  if (!aarch64_parse_features (ext, &mcpu_cpu_opt, false))
	    return;

	cpu_variant = *mcpu_cpu_opt;

	*input_line_pointer = saved_char;
	demand_empty_rest_of_line ();
	return;
      }
  as_bad (_("unknown cpu `%s'"), name);
  *input_line_pointer = saved_char;
  ignore_rest_of_line ();
}


/* Parse a .arch directive.  */

static void
s_aarch64_arch (int ignored ATTRIBUTE_UNUSED)
{
  const struct aarch64_arch_option_table *opt;
  char saved_char;
  char *name;
  char *ext;
  size_t optlen;

  name = input_line_pointer;
  input_line_pointer = find_end_of_line (input_line_pointer, flag_m68k_mri);
  saved_char = *input_line_pointer;
  *input_line_pointer = 0;

  ext = strchr (name, '+');

  if (ext != NULL)
    optlen = ext - name;
  else
    optlen = strlen (name);

  /* Skip the first "all" entry.  */
  for (opt = aarch64_archs + 1; opt->name != NULL; opt++)
    if (strlen (opt->name) == optlen
	&& strncmp (name, opt->name, optlen) == 0)
      {
	mcpu_cpu_opt = &opt->value;
	if (ext != NULL)
	  if (!aarch64_parse_features (ext, &mcpu_cpu_opt, false))
	    return;

	cpu_variant = *mcpu_cpu_opt;

	*input_line_pointer = saved_char;
	demand_empty_rest_of_line ();
	return;
      }

  as_bad (_("unknown architecture `%s'\n"), name);
  *input_line_pointer = saved_char;
  ignore_rest_of_line ();
}

/* Parse a .arch_extension directive.  */

static void
s_aarch64_arch_extension (int ignored ATTRIBUTE_UNUSED)
{
  char saved_char;
  char *ext = input_line_pointer;

  input_line_pointer = find_end_of_line (input_line_pointer, flag_m68k_mri);
  saved_char = *input_line_pointer;
  *input_line_pointer = 0;

  if (!aarch64_parse_features (ext, &mcpu_cpu_opt, true))
    return;

  cpu_variant = *mcpu_cpu_opt;

  *input_line_pointer = saved_char;
  demand_empty_rest_of_line ();
}

/* Copy symbol information.  */

void
aarch64_copy_symbol_attributes (symbolS * dest, symbolS * src)
{
  AARCH64_GET_FLAG (dest) = AARCH64_GET_FLAG (src);
}

#ifdef OBJ_ELF
/* Same as elf_copy_symbol_attributes, but without copying st_other.
   This is needed so AArch64 specific st_other values can be independently
   specified for an IFUNC resolver (that is called by the dynamic linker)
   and the symbol it resolves (aliased to the resolver).  In particular,
   if a function symbol has special st_other value set via directives,
   then attaching an IFUNC resolver to that symbol should not override
   the st_other setting.  Requiring the directive on the IFUNC resolver
   symbol would be unexpected and problematic in C code, where the two
   symbols appear as two independent function declarations.  */

void
aarch64_elf_copy_symbol_attributes (symbolS *dest, symbolS *src)
{
  struct elf_obj_sy *srcelf = symbol_get_obj (src);
  struct elf_obj_sy *destelf = symbol_get_obj (dest);
  /* If size is unset, copy size from src.  Because we don't track whether
     .size has been used, we can't differentiate .size dest, 0 from the case
     where dest's size is unset.  */
  if (!destelf->size && S_GET_SIZE (dest) == 0)
    {
      if (srcelf->size)
	{
	  destelf->size = XNEW (expressionS);
	  *destelf->size = *srcelf->size;
	}
      S_SET_SIZE (dest, S_GET_SIZE (src));
    }
}
#endif
