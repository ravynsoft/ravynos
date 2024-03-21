/* tc-riscv.c -- RISC-V assembler
   Copyright (C) 2011-2023 Free Software Foundation, Inc.

   Contributed by Andrew Waterman (andrew@sifive.com).
   Based on MIPS target.

   This file is part of GAS.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING3. If not,
   see <http://www.gnu.org/licenses/>.  */

#include "as.h"
#include "config.h"
#include "subsegs.h"
#include "safe-ctype.h"

#include "itbl-ops.h"
#include "dwarf2dbg.h"
#include "dw2gencfi.h"

#include "bfd/elfxx-riscv.h"
#include "elf/riscv.h"
#include "opcode/riscv.h"

#include <stdint.h>

/* Information about an instruction, including its format, operands
   and fixups.  */
struct riscv_cl_insn
{
  /* The opcode's entry in riscv_opcodes.  */
  const struct riscv_opcode *insn_mo;

  /* The encoded instruction bits
     (first bits enough to extract instruction length on a long opcode).  */
  insn_t insn_opcode;

  /* The long encoded instruction bits ([0] is non-zero on a long opcode).  */
  char insn_long_opcode[RISCV_MAX_INSN_LEN];

  /* The frag that contains the instruction.  */
  struct frag *frag;

  /* The offset into FRAG of the first instruction byte.  */
  long where;

  /* The relocs associated with the instruction, if any.  */
  fixS *fixp;
};

/* All RISC-V CSR belong to one of these classes.  */
enum riscv_csr_class
{
  CSR_CLASS_NONE,

  CSR_CLASS_I,
  CSR_CLASS_I_32,	/* rv32 only */
  CSR_CLASS_F,		/* f-ext only */
  CSR_CLASS_ZKR,	/* zkr only */
  CSR_CLASS_V,		/* rvv only */
  CSR_CLASS_DEBUG,	/* debug CSR */
  CSR_CLASS_H,		/* hypervisor */
  CSR_CLASS_H_32,	/* hypervisor, rv32 only */
  CSR_CLASS_SMAIA,		/* Smaia */
  CSR_CLASS_SMAIA_32,		/* Smaia, rv32 only */
  CSR_CLASS_SMSTATEEN,		/* Smstateen only */
  CSR_CLASS_SMSTATEEN_32,	/* Smstateen RV32 only */
  CSR_CLASS_SSAIA,		/* Ssaia */
  CSR_CLASS_SSAIA_AND_H,	/* Ssaia with H */
  CSR_CLASS_SSAIA_32,		/* Ssaia, rv32 only */
  CSR_CLASS_SSAIA_AND_H_32,	/* Ssaia with H, rv32 only */
  CSR_CLASS_SSSTATEEN,		/* S[ms]stateen only */
  CSR_CLASS_SSSTATEEN_AND_H,	/* S[ms]stateen only (with H) */
  CSR_CLASS_SSSTATEEN_AND_H_32,	/* S[ms]stateen RV32 only (with H) */
  CSR_CLASS_SSCOFPMF,		/* Sscofpmf only */
  CSR_CLASS_SSCOFPMF_32,	/* Sscofpmf RV32 only */
  CSR_CLASS_SSTC,		/* Sstc only */
  CSR_CLASS_SSTC_AND_H,		/* Sstc only (with H) */
  CSR_CLASS_SSTC_32,		/* Sstc RV32 only */
  CSR_CLASS_SSTC_AND_H_32,	/* Sstc RV32 only (with H) */
};

/* This structure holds all restricted conditions for a CSR.  */
struct riscv_csr_extra
{
  /* Class to which this CSR belongs.  Used to decide whether or
     not this CSR is legal in the current -march context.  */
  enum riscv_csr_class csr_class;

  /* CSR may have differnet numbers in the previous priv spec.  */
  unsigned address;

  /* Record the CSR is defined/valid in which versions.  */
  enum riscv_spec_class define_version;

  /* Record the CSR is aborted/invalid from which versions.  If it isn't
     aborted in the current version, then it should be PRIV_SPEC_CLASS_DRAFT.  */
  enum riscv_spec_class abort_version;

  /* The CSR may have more than one setting.  */
  struct riscv_csr_extra *next;
};

/* This structure contains information about errors that occur within the
   riscv_ip function */
struct riscv_ip_error
{
  /* General error message */
  const char* msg;

  /* Statement that caused the error */
  char* statement;

  /* Missing extension that needs to be enabled */
  const char* missing_ext;
};

#ifndef DEFAULT_ARCH
#define DEFAULT_ARCH "riscv64"
#endif

#ifndef DEFAULT_RISCV_ATTR
#define DEFAULT_RISCV_ATTR 0
#endif

/* Let riscv_after_parse_args set the default value according to xlen.  */
#ifndef DEFAULT_RISCV_ARCH_WITH_EXT
#define DEFAULT_RISCV_ARCH_WITH_EXT NULL
#endif

/* Need to sync the version with RISC-V compiler.  */
#ifndef DEFAULT_RISCV_ISA_SPEC
#define DEFAULT_RISCV_ISA_SPEC "20191213"
#endif

#ifndef DEFAULT_RISCV_PRIV_SPEC
#define DEFAULT_RISCV_PRIV_SPEC "1.11"
#endif

static const char default_arch[] = DEFAULT_ARCH;
static const char *default_arch_with_ext = DEFAULT_RISCV_ARCH_WITH_EXT;
static enum riscv_spec_class default_isa_spec = ISA_SPEC_CLASS_NONE;
static enum riscv_spec_class default_priv_spec = PRIV_SPEC_CLASS_NONE;

static unsigned xlen = 0; /* The width of an x-register.  */
static unsigned abi_xlen = 0; /* The width of a pointer in the ABI.  */
static bool rve_abi = false;
enum float_abi
{
  FLOAT_ABI_DEFAULT = -1,
  FLOAT_ABI_SOFT,
  FLOAT_ABI_SINGLE,
  FLOAT_ABI_DOUBLE,
  FLOAT_ABI_QUAD
};
static enum float_abi float_abi = FLOAT_ABI_DEFAULT;

#define LOAD_ADDRESS_INSN (abi_xlen == 64 ? "ld" : "lw")
#define ADD32_INSN (xlen == 64 ? "addiw" : "addi")

static unsigned elf_flags = 0;

static bool probing_insn_operands;

/* Set the default_isa_spec.  Return 0 if the spec isn't supported.
   Otherwise, return 1.  */

static int
riscv_set_default_isa_spec (const char *s)
{
  enum riscv_spec_class class = ISA_SPEC_CLASS_NONE;
  RISCV_GET_ISA_SPEC_CLASS (s, class);
  if (class == ISA_SPEC_CLASS_NONE)
    {
      as_bad ("unknown default ISA spec `%s' set by "
	      "-misa-spec or --with-isa-spec", s);
      return 0;
    }
  else
    default_isa_spec = class;
  return 1;
}

/* Set the default_priv_spec.  Find the privileged elf attributes when
   the input string is NULL.  Return 0 if the spec isn't supported.
   Otherwise, return 1.  */

static int
riscv_set_default_priv_spec (const char *s)
{
  enum riscv_spec_class class = PRIV_SPEC_CLASS_NONE;
  unsigned major, minor, revision;
  obj_attribute *attr;

  RISCV_GET_PRIV_SPEC_CLASS (s, class);
  if (class != PRIV_SPEC_CLASS_NONE)
    {
      default_priv_spec = class;
      return 1;
    }

  if (s != NULL)
    {
      as_bad (_("unknown default privileged spec `%s' set by "
		"-mpriv-spec or --with-priv-spec"), s);
      return 0;
    }

  /* Set the default_priv_spec by the privileged elf attributes.  */
  attr = elf_known_obj_attributes_proc (stdoutput);
  major = (unsigned) attr[Tag_RISCV_priv_spec].i;
  minor = (unsigned) attr[Tag_RISCV_priv_spec_minor].i;
  revision = (unsigned) attr[Tag_RISCV_priv_spec_revision].i;
  /* Version 0.0.0 is the default value and meningless.  */
  if (major == 0 && minor == 0 && revision == 0)
    return 1;

  riscv_get_priv_spec_class_from_numbers (major, minor, revision, &class);
  if (class != PRIV_SPEC_CLASS_NONE)
    {
      default_priv_spec = class;
      return 1;
    }

  /* Still can not find the privileged spec class.  */
  as_bad (_("unknown default privileged spec `%d.%d.%d' set by "
	    "privileged elf attributes"), major, minor, revision);
  return 0;
}

/* This is the set of options which the .option pseudo-op may modify.  */
struct riscv_set_options
{
  int pic; /* Generate position-independent code.  */
  int rvc; /* Generate RVC code.  */
  int relax; /* Emit relocs the linker is allowed to relax.  */
  int arch_attr; /* Emit architecture and privileged elf attributes.  */
  int csr_check; /* Enable the CSR checking.  */
};

static struct riscv_set_options riscv_opts =
{
  0, /* pic */
  0, /* rvc */
  1, /* relax */
  DEFAULT_RISCV_ATTR, /* arch_attr */
  0, /* csr_check */
};

/* Enable or disable the rvc flags for riscv_opts.  Turn on the rvc flag
   for elf_flags once we have enabled c extension.  */

static void
riscv_set_rvc (bool rvc_value)
{
  if (rvc_value)
    elf_flags |= EF_RISCV_RVC;

  riscv_opts.rvc = rvc_value;
}

/* Turn on the tso flag for elf_flags once we have enabled ztso extension.  */

static void
riscv_set_tso (void)
{
  elf_flags |= EF_RISCV_TSO;
}

/* The linked list hanging off of .subsets_list records all enabled extensions,
   which are parsed from the architecture string.  The architecture string can
   be set by the -march option, the elf architecture attributes, and the
   --with-arch configure option.  */
static riscv_parse_subset_t riscv_rps_as =
{
  NULL,			/* subset_list, we will set it later once
			   riscv_opts_stack is created or updated.  */
  as_bad,		/* error_handler.  */
  &xlen,		/* xlen.  */
  &default_isa_spec,	/* isa_spec.  */
  true,			/* check_unknown_prefixed_ext.  */
};

/* Update the architecture string in the subset_list.  */

static void
riscv_reset_subsets_list_arch_str (void)
{
  riscv_subset_list_t *subsets = riscv_rps_as.subset_list;
  if (subsets->arch_str != NULL)
    free ((void *) subsets->arch_str);
  subsets->arch_str = riscv_arch_str (xlen, subsets);
}

/* This structure is used to hold a stack of .option values.  */
struct riscv_option_stack
{
  struct riscv_option_stack *next;
  struct riscv_set_options options;
  riscv_subset_list_t *subset_list;
};

static struct riscv_option_stack *riscv_opts_stack = NULL;

/* Set which ISA and extensions are available.  */

static void
riscv_set_arch (const char *s)
{
  if (s != NULL && strcmp (s, "") == 0)
    {
      as_bad (_("the architecture string of -march and elf architecture "
		"attributes cannot be empty"));
      return;
    }

  if (riscv_rps_as.subset_list == NULL)
    {
      riscv_rps_as.subset_list = XNEW (riscv_subset_list_t);
      riscv_rps_as.subset_list->head = NULL;
      riscv_rps_as.subset_list->tail = NULL;
      riscv_rps_as.subset_list->arch_str = NULL;
    }
  riscv_release_subset_list (riscv_rps_as.subset_list);
  riscv_parse_subset (&riscv_rps_as, s);
  riscv_reset_subsets_list_arch_str ();

  riscv_set_rvc (false);
  if (riscv_subset_supports (&riscv_rps_as, "c"))
    riscv_set_rvc (true);

  if (riscv_subset_supports (&riscv_rps_as, "ztso"))
    riscv_set_tso ();
}

/* Indicate -mabi option is explictly set.  */
static bool explicit_mabi = false;

/* Set the abi information.  */

static void
riscv_set_abi (unsigned new_xlen, enum float_abi new_float_abi, bool rve)
{
  abi_xlen = new_xlen;
  float_abi = new_float_abi;
  rve_abi = rve;
}

/* If the -mabi option isn't set, then set the abi according to the
   ISA string.  Otherwise, check if there is any conflict.  */

static void
riscv_set_abi_by_arch (void)
{
  if (!explicit_mabi)
    {
      if (riscv_subset_supports (&riscv_rps_as, "q"))
	riscv_set_abi (xlen, FLOAT_ABI_QUAD, false);
      else if (riscv_subset_supports (&riscv_rps_as, "d"))
	riscv_set_abi (xlen, FLOAT_ABI_DOUBLE, false);
      else if (riscv_subset_supports (&riscv_rps_as, "e"))
	riscv_set_abi (xlen, FLOAT_ABI_SOFT, true);
      else
	riscv_set_abi (xlen, FLOAT_ABI_SOFT, false);
    }
  else
    {
      gas_assert (abi_xlen != 0 && xlen != 0 && float_abi != FLOAT_ABI_DEFAULT);
      if (abi_xlen > xlen)
	as_bad ("can't have %d-bit ABI on %d-bit ISA", abi_xlen, xlen);
      else if (abi_xlen < xlen)
	as_bad ("%d-bit ABI not yet supported on %d-bit ISA", abi_xlen, xlen);

      if (riscv_subset_supports (&riscv_rps_as, "e") && !rve_abi)
	as_bad ("only the ilp32e ABI is supported for e extension");

      if (float_abi == FLOAT_ABI_SINGLE
	  && !riscv_subset_supports (&riscv_rps_as, "f"))
	as_bad ("ilp32f/lp64f ABI can't be used when f extension "
		"isn't supported");
      else if (float_abi == FLOAT_ABI_DOUBLE
	       && !riscv_subset_supports (&riscv_rps_as, "d"))
	as_bad ("ilp32d/lp64d ABI can't be used when d extension "
		"isn't supported");
      else if (float_abi == FLOAT_ABI_QUAD
	       && !riscv_subset_supports (&riscv_rps_as, "q"))
	as_bad ("ilp32q/lp64q ABI can't be used when q extension "
		"isn't supported");
    }

  /* Update the EF_RISCV_FLOAT_ABI field of elf_flags.  */
  elf_flags &= ~EF_RISCV_FLOAT_ABI;
  elf_flags |= float_abi << 1;

  if (rve_abi)
    elf_flags |= EF_RISCV_RVE;
}

/* Handle of the OPCODE hash table.  */
static htab_t op_hash = NULL;

/* Handle of the type of .insn hash table.  */
static htab_t insn_type_hash = NULL;

/* This array holds the chars that always start a comment.  If the
   pre-processor is disabled, these aren't very useful.  */
const char comment_chars[] = "#";

/* This array holds the chars that only start a comment at the beginning of
   a line.  If the line seems to have the form '# 123 filename'
   .line and .file directives will appear in the pre-processed output

   Note that input_file.c hand checks for '#' at the beginning of the
   first line of the input file.  This is because the compiler outputs
   #NO_APP at the beginning of its output.

   Also note that C style comments are always supported.  */
const char line_comment_chars[] = "#";

/* This array holds machine specific line separator characters.  */
const char line_separator_chars[] = ";";

/* Chars that can be used to separate mant from exp in floating point nums.  */
const char EXP_CHARS[] = "eE";

/* Chars that mean this number is a floating point constant.
   As in 0f12.456 or 0d1.2345e12.  */
const char FLT_CHARS[] = "rRsSfFdDxXpPhH";

/* Indicate we are already assemble any instructions or not.  */
static bool start_assemble = false;

/* Indicate ELF attributes are explicitly set.  */
static bool explicit_attr = false;

/* Indicate CSR or priv instructions are explicitly used.  */
static bool explicit_priv_attr = false;

static char *expr_parse_end;

/* Macros for encoding relaxation state for RVC branches and far jumps.  */
#define RELAX_BRANCH_ENCODE(uncond, rvc, length)	\
  ((relax_substateT) 					\
   (0xc0000000						\
    | ((uncond) ? 1 : 0)				\
    | ((rvc) ? 2 : 0)					\
    | ((length) << 2)))
#define RELAX_BRANCH_P(i) (((i) & 0xf0000000) == 0xc0000000)
#define RELAX_BRANCH_LENGTH(i) (((i) >> 2) & 0xF)
#define RELAX_BRANCH_RVC(i) (((i) & 2) != 0)
#define RELAX_BRANCH_UNCOND(i) (((i) & 1) != 0)

/* Is the given value a sign-extended 32-bit value?  */
#define IS_SEXT_32BIT_NUM(x)						\
  (((x) &~ (offsetT) 0x7fffffff) == 0					\
   || (((x) &~ (offsetT) 0x7fffffff) == ~ (offsetT) 0x7fffffff))

/* Is the given value a zero-extended 32-bit value?  Or a negated one?  */
#define IS_ZEXT_32BIT_NUM(x)						\
  (((x) &~ (offsetT) 0xffffffff) == 0					\
   || (((x) &~ (offsetT) 0xffffffff) == ~ (offsetT) 0xffffffff))

/* Change INSN's opcode so that the operand given by FIELD has value VALUE.
   INSN is a riscv_cl_insn structure and VALUE is evaluated exactly once.  */
#define INSERT_OPERAND(FIELD, INSN, VALUE) \
  INSERT_BITS ((INSN).insn_opcode, VALUE, OP_MASK_##FIELD, OP_SH_##FIELD)

#define INSERT_IMM(n, s, INSN, VALUE) \
  INSERT_BITS ((INSN).insn_opcode, VALUE, (1ULL<<n) - 1, s)

/* Determine if an instruction matches an opcode.  */
#define OPCODE_MATCHES(OPCODE, OP) \
  (((OPCODE) & MASK_##OP) == MATCH_##OP)

/* Create a new mapping symbol for the transition to STATE.  */

static void
make_mapping_symbol (enum riscv_seg_mstate state,
		     valueT value,
		     fragS *frag,
		     const char *arch_str,
		     bool odd_data_padding)
{
  const char *name;
  char *buff = NULL;
  switch (state)
    {
    case MAP_DATA:
      name = "$d";
      break;
    case MAP_INSN:
      if (arch_str != NULL)
	{
	  size_t size = strlen (arch_str) + 3; /* "$x" + '\0'  */
	  buff = xmalloc (size);
	  snprintf (buff, size, "$x%s", arch_str);
	  name = buff;
	}
      else
	name = "$x";
      break;
    default:
      abort ();
    }

  symbolS *symbol = symbol_new (name, now_seg, frag, value);
  symbol_get_bfdsym (symbol)->flags |= (BSF_NO_FLAGS | BSF_LOCAL);
  if (arch_str != NULL)
    {
      /* Store current $x+arch into tc_segment_info.  */
      seg_info (now_seg)->tc_segment_info_data.arch_map_symbol = symbol;
      xfree ((void *) buff);
    }

  /* If .fill or other data filling directive generates zero sized data,
     then mapping symbol for the following code will have the same value.

     Please see gas/testsuite/gas/riscv/mapping.s: .text.zero.fill.first
     and .text.zero.fill.last.  */
  symbolS *first = frag->tc_frag_data.first_map_symbol;
  symbolS *last = frag->tc_frag_data.last_map_symbol;
  symbolS *removed = NULL;
  if (value == 0)
    {
      if (first != NULL)
	{
	  know (S_GET_VALUE (first) == S_GET_VALUE (symbol)
		&& first == last);
	  /* Remove the old one.  */
	  removed = first;
	}
      frag->tc_frag_data.first_map_symbol = symbol;
    }
  else if (last != NULL)
    {
      /* The mapping symbols should be added in offset order.  */
      know (S_GET_VALUE (last) <= S_GET_VALUE (symbol));
      /* Remove the old one.  */
      if (S_GET_VALUE (last) == S_GET_VALUE (symbol))
	removed = last;
    }
  frag->tc_frag_data.last_map_symbol = symbol;

  if (removed == NULL)
    return;

  if (odd_data_padding)
    {
      /* If the removed mapping symbol is $x+arch, then add it back to
	 the next $x.  */
      const char *str = strncmp (S_GET_NAME (removed), "$xrv", 4) == 0
			? S_GET_NAME (removed) + 2 : NULL;
      make_mapping_symbol (MAP_INSN, frag->fr_fix + 1, frag, str,
			   false/* odd_data_padding */);
    }
  symbol_remove (removed, &symbol_rootP, &symbol_lastP);
}

/* Set the mapping state for frag_now.  */

void
riscv_mapping_state (enum riscv_seg_mstate to_state,
		     int max_chars,
		     bool fr_align_code)
{
  enum riscv_seg_mstate from_state =
	seg_info (now_seg)->tc_segment_info_data.map_state;
  bool reset_seg_arch_str = false;

  if (!SEG_NORMAL (now_seg)
      /* For now we only add the mapping symbols to text sections.
	 Therefore, the dis-assembler only show the actual contents
	 distribution for text.  Other sections will be shown as
	 data without the details.  */
      || !subseg_text_p (now_seg))
    return;

  /* The mapping symbol should be emitted if not in the right
     mapping state.  */
  symbolS *seg_arch_symbol =
	seg_info (now_seg)->tc_segment_info_data.arch_map_symbol;
  if (to_state == MAP_INSN && seg_arch_symbol == 0)
    {
      /* Always add $x+arch at the first instruction of section.  */
      reset_seg_arch_str = true;
    }
  else if (seg_arch_symbol != 0
	   && to_state == MAP_INSN
	   && !fr_align_code
	   && strcmp (riscv_rps_as.subset_list->arch_str,
		      S_GET_NAME (seg_arch_symbol) + 2) != 0)
    {
      reset_seg_arch_str = true;
    }
  else if (from_state == to_state)
    return;

  valueT value = (valueT) (frag_now_fix () - max_chars);
  seg_info (now_seg)->tc_segment_info_data.map_state = to_state;
  const char *arch_str = reset_seg_arch_str
			 ? riscv_rps_as.subset_list->arch_str : NULL;
  make_mapping_symbol (to_state, value, frag_now, arch_str,
		       false/* odd_data_padding */);
}

/* Add the odd bytes of paddings for riscv_handle_align.  */

static void
riscv_add_odd_padding_symbol (fragS *frag)
{
  /* If there was already a mapping symbol, it should be
     removed in the make_mapping_symbol.

     Please see gas/testsuite/gas/riscv/mapping.s: .text.odd.align.*.  */
  make_mapping_symbol (MAP_DATA, frag->fr_fix, frag,
		       NULL/* arch_str */, true/* odd_data_padding */);
}

/* Remove any excess mapping symbols generated for alignment frags in
   SEC.  We may have created a mapping symbol before a zero byte
   alignment; remove it if there's a mapping symbol after the
   alignment.  */

static void
riscv_check_mapping_symbols (bfd *abfd ATTRIBUTE_UNUSED,
			     asection *sec,
			     void *dummy ATTRIBUTE_UNUSED)
{
  segment_info_type *seginfo = seg_info (sec);
  fragS *fragp;

  if (seginfo == NULL || seginfo->frchainP == NULL)
    return;

  for (fragp = seginfo->frchainP->frch_root;
       fragp != NULL;
       fragp = fragp->fr_next)
    {
      symbolS *last = fragp->tc_frag_data.last_map_symbol;
      fragS *next = fragp->fr_next;

      if (last == NULL || next == NULL)
	continue;

      /* Check the last mapping symbol if it is at the boundary of
	 fragment.  */
      if (S_GET_VALUE (last) < next->fr_address)
	continue;
      know (S_GET_VALUE (last) == next->fr_address);

      do
	{
	  symbolS *next_first = next->tc_frag_data.first_map_symbol;
	  if (next_first != NULL)
	    {
	      /* The last mapping symbol overlaps with another one
		 which at the start of the next frag.

		 Please see the gas/testsuite/gas/riscv/mapping.s:
		 .text.zero.fill.align.A and .text.zero.fill.align.B.  */
	      know (S_GET_VALUE (last) == S_GET_VALUE (next_first));
	      symbolS *removed = last;
	      if (strncmp (S_GET_NAME (last), "$xrv", 4) == 0
		  && strcmp (S_GET_NAME (next_first), "$x") == 0)
		removed = next_first;
	      symbol_remove (removed, &symbol_rootP, &symbol_lastP);
	      break;
	    }

	  if (next->fr_next == NULL)
	    {
	      /* The last mapping symbol is at the end of the section.

		 Please see the gas/testsuite/gas/riscv/mapping.s:
		 .text.last.section.  */
	      know (next->fr_fix == 0 && next->fr_var == 0);
	      symbol_remove (last, &symbol_rootP, &symbol_lastP);
	      break;
	    }

	  /* Since we may have empty frags without any mapping symbols,
	     keep looking until the non-empty frag.  */
	  if (next->fr_address != next->fr_next->fr_address)
	    break;

	  next = next->fr_next;
	}
      while (next != NULL);
    }
}

/* The default target format to use.  */

const char *
riscv_target_format (void)
{
  if (target_big_endian)
    return xlen == 64 ? "elf64-bigriscv" : "elf32-bigriscv";
  else
    return xlen == 64 ? "elf64-littleriscv" : "elf32-littleriscv";
}

/* Return the length of instruction INSN.  */

static inline unsigned int
insn_length (const struct riscv_cl_insn *insn)
{
  return riscv_insn_length (insn->insn_opcode);
}

/* Initialise INSN from opcode entry MO.  Leave its position unspecified.  */

static void
create_insn (struct riscv_cl_insn *insn, const struct riscv_opcode *mo)
{
  insn->insn_mo = mo;
  insn->insn_opcode = mo->match;
  insn->insn_long_opcode[0] = 0;
  insn->frag = NULL;
  insn->where = 0;
  insn->fixp = NULL;
}

/* Install INSN at the location specified by its "frag" and "where" fields.  */

static void
install_insn (const struct riscv_cl_insn *insn)
{
  char *f = insn->frag->fr_literal + insn->where;
  if (insn->insn_long_opcode[0] != 0)
    memcpy (f, insn->insn_long_opcode, insn_length (insn));
  else
    number_to_chars_littleendian (f, insn->insn_opcode, insn_length (insn));
}

/* Move INSN to offset WHERE in FRAG.  Adjust the fixups accordingly
   and install the opcode in the new location.  */

static void
move_insn (struct riscv_cl_insn *insn, fragS *frag, long where)
{
  insn->frag = frag;
  insn->where = where;
  if (insn->fixp != NULL)
    {
      insn->fixp->fx_frag = frag;
      insn->fixp->fx_where = where;
    }
  install_insn (insn);
}

/* Add INSN to the end of the output.  */

static void
add_fixed_insn (struct riscv_cl_insn *insn)
{
  char *f = frag_more (insn_length (insn));
  move_insn (insn, frag_now, f - frag_now->fr_literal);
}

static void
add_relaxed_insn (struct riscv_cl_insn *insn, int max_chars, int var,
      relax_substateT subtype, symbolS *symbol, offsetT offset)
{
  frag_grow (max_chars);
  move_insn (insn, frag_now, frag_more (0) - frag_now->fr_literal);
  frag_var (rs_machine_dependent, max_chars, var,
	    subtype, symbol, offset, NULL);
}

/* Compute the length of a branch sequence, and adjust the stored length
   accordingly.  If FRAGP is NULL, the worst-case length is returned.  */

static unsigned
relaxed_branch_length (fragS *fragp, asection *sec, int update)
{
  int jump, rvc, length = 8;

  if (!fragp)
    return length;

  jump = RELAX_BRANCH_UNCOND (fragp->fr_subtype);
  rvc = RELAX_BRANCH_RVC (fragp->fr_subtype);
  length = RELAX_BRANCH_LENGTH (fragp->fr_subtype);

  /* Assume jumps are in range; the linker will catch any that aren't.  */
  length = jump ? 4 : 8;

  if (fragp->fr_symbol != NULL
      && S_IS_DEFINED (fragp->fr_symbol)
      && !S_IS_WEAK (fragp->fr_symbol)
      && sec == S_GET_SEGMENT (fragp->fr_symbol))
    {
      offsetT val = S_GET_VALUE (fragp->fr_symbol) + fragp->fr_offset;
      bfd_vma rvc_range = jump ? RVC_JUMP_REACH : RVC_BRANCH_REACH;
      val -= fragp->fr_address + fragp->fr_fix;

      if (rvc && (bfd_vma)(val + rvc_range/2) < rvc_range)
	length = 2;
      else if ((bfd_vma)(val + RISCV_BRANCH_REACH/2) < RISCV_BRANCH_REACH)
	length = 4;
      else if (!jump && rvc)
	length = 6;
    }

  if (update)
    fragp->fr_subtype = RELAX_BRANCH_ENCODE (jump, rvc, length);

  return length;
}

/* Information about an opcode name, mnemonics and its value.  */
struct opcode_name_t
{
  const char *name;
  unsigned int val;
};

/* List for all supported opcode name.  */
static const struct opcode_name_t opcode_name_list[] =
{
  {"C0",        0x0},
  {"C1",        0x1},
  {"C2",        0x2},

  {"LOAD",      0x03},
  {"LOAD_FP",   0x07},
  {"CUSTOM_0",  0x0b},
  {"MISC_MEM",  0x0f},
  {"OP_IMM",    0x13},
  {"AUIPC",     0x17},
  {"OP_IMM_32", 0x1b},
  /* 48b        0x1f.  */

  {"STORE",     0x23},
  {"STORE_FP",  0x27},
  {"CUSTOM_1",  0x2b},
  {"AMO",       0x2f},
  {"OP",        0x33},
  {"LUI",       0x37},
  {"OP_32",     0x3b},
  /* 64b        0x3f.  */

  {"MADD",      0x43},
  {"MSUB",      0x47},
  {"NMADD",     0x4f},
  {"NMSUB",     0x4b},
  {"OP_FP",     0x53},
  {"OP_V",      0x57},
  {"CUSTOM_2",  0x5b},
  /* 48b        0x5f.  */

  {"BRANCH",    0x63},
  {"JALR",      0x67},
  /*reserved    0x5b.  */
  {"JAL",       0x6f},
  {"SYSTEM",    0x73},
  /*reserved    0x77.  */
  {"CUSTOM_3",  0x7b},
  /* >80b       0x7f.  */

  {NULL, 0}
};

/* Hash table for lookup opcode name.  */
static htab_t opcode_names_hash = NULL;

/* Initialization for hash table of opcode name.  */

static void
init_opcode_names_hash (void)
{
  const struct opcode_name_t *opcode;

  for (opcode = &opcode_name_list[0]; opcode->name != NULL; ++opcode)
    if (str_hash_insert (opcode_names_hash, opcode->name, opcode, 0) != NULL)
      as_fatal (_("internal: duplicate %s"), opcode->name);
}

/* Find `s` is a valid opcode name or not, return the opcode name info
   if found.  */

static const struct opcode_name_t *
opcode_name_lookup (char **s)
{
  char *e;
  char save_c;
  struct opcode_name_t *o;

  /* Find end of name.  */
  e = *s;
  if (is_name_beginner (*e))
    ++e;
  while (is_part_of_name (*e))
    ++e;

  /* Terminate name.  */
  save_c = *e;
  *e = '\0';

  o = (struct opcode_name_t *) str_hash_find (opcode_names_hash, *s);

  /* Advance to next token if one was recognized.  */
  if (o)
    *s = e;

  *e = save_c;
  expr_parse_end = e;

  return o;
}

/* All RISC-V registers belong to one of these classes.  */
enum reg_class
{
  RCLASS_GPR,
  RCLASS_FPR,
  RCLASS_VECR,
  RCLASS_VECM,
  RCLASS_MAX,

  RCLASS_CSR
};

static htab_t reg_names_hash = NULL;
static htab_t csr_extra_hash = NULL;

#define ENCODE_REG_HASH(cls, n) \
  ((void *)(uintptr_t)((n) * RCLASS_MAX + (cls) + 1))
#define DECODE_REG_CLASS(hash) (((uintptr_t)(hash) - 1) % RCLASS_MAX)
#define DECODE_REG_NUM(hash) (((uintptr_t)(hash) - 1) / RCLASS_MAX)

static void
hash_reg_name (enum reg_class class, const char *name, unsigned n)
{
  void *hash = ENCODE_REG_HASH (class, n);
  if (str_hash_insert (reg_names_hash, name, hash, 0) != NULL)
    as_fatal (_("internal: duplicate %s"), name);
}

static void
hash_reg_names (enum reg_class class, const char * const names[], unsigned n)
{
  unsigned i;

  for (i = 0; i < n; i++)
    hash_reg_name (class, names[i], i);
}

/* Init hash table csr_extra_hash to handle CSR.  */

static void
riscv_init_csr_hash (const char *name,
		     unsigned address,
		     enum riscv_csr_class class,
		     enum riscv_spec_class define_version,
		     enum riscv_spec_class abort_version)
{
  struct riscv_csr_extra *entry, *pre_entry;
  bool need_enrty = true;

  pre_entry = NULL;
  entry = (struct riscv_csr_extra *) str_hash_find (csr_extra_hash, name);
  while (need_enrty && entry != NULL)
    {
      if (entry->csr_class == class
	  && entry->address == address
	  && entry->define_version == define_version
	  && entry->abort_version == abort_version)
	need_enrty = false;
      pre_entry = entry;
      entry = entry->next;
    }

  /* Duplicate CSR.  */
  if (!need_enrty)
    return;

  entry = notes_alloc (sizeof (*entry));
  entry->csr_class = class;
  entry->address = address;
  entry->define_version = define_version;
  entry->abort_version = abort_version;
  entry->next = NULL;

  if (pre_entry == NULL)
    str_hash_insert (csr_extra_hash, name, entry, 0);
  else
    pre_entry->next = entry;
}

/* Return the CSR address after checking the ISA dependency and
   the privileged spec version.

   There are one warning and two errors for CSR,

   Invalid CSR: the CSR was defined, but isn't allowed for the current ISA
   or the privileged spec, report warning only if -mcsr-check is set.
   Unknown CSR: the CSR has never been defined, report error.
   Improper CSR: the CSR number over the range (> 0xfff), report error.  */

static unsigned int
riscv_csr_address (const char *csr_name,
		   struct riscv_csr_extra *entry)
{
  struct riscv_csr_extra *saved_entry = entry;
  enum riscv_csr_class csr_class = entry->csr_class;
  bool need_check_version = false;
  bool is_rv32_only = false;
  bool is_h_required = false;
  const char* extension = NULL;

  switch (csr_class)
    {
    case CSR_CLASS_I_32:
      is_rv32_only = true;
      /* Fall through.  */
    case CSR_CLASS_I:
      need_check_version = true;
      extension = "i";
      break;
    case CSR_CLASS_H_32:
      is_rv32_only = true;
      /* Fall through.  */
    case CSR_CLASS_H:
      extension = "h";
      break;
    case CSR_CLASS_F:
      extension = "f";
      break;
    case CSR_CLASS_ZKR:
      extension = "zkr";
      break;
    case CSR_CLASS_V:
      extension = "zve32x";
      break;
    case CSR_CLASS_SMAIA_32:
      is_rv32_only = true;
      /* Fall through.  */
    case CSR_CLASS_SMAIA:
      extension = "smaia";
      break;
    case CSR_CLASS_SMSTATEEN_32:
      is_rv32_only = true;
      /* Fall through.  */
    case CSR_CLASS_SMSTATEEN:
      extension = "smstateen";
      break;
    case CSR_CLASS_SSAIA:
    case CSR_CLASS_SSAIA_AND_H:
    case CSR_CLASS_SSAIA_32:
    case CSR_CLASS_SSAIA_AND_H_32:
      is_rv32_only = (csr_class == CSR_CLASS_SSAIA_32
		      || csr_class == CSR_CLASS_SSAIA_AND_H_32);
      is_h_required = (csr_class == CSR_CLASS_SSAIA_AND_H
		       || csr_class == CSR_CLASS_SSAIA_AND_H_32);
      extension = "ssaia";
      break;
    case CSR_CLASS_SSSTATEEN_AND_H_32:
      is_rv32_only = true;
      /* Fall through.  */
    case CSR_CLASS_SSSTATEEN_AND_H:
      is_h_required = true;
      /* Fall through.  */
    case CSR_CLASS_SSSTATEEN:
      extension = "ssstateen";
      break;
    case CSR_CLASS_SSCOFPMF_32:
      is_rv32_only = true;
      /* Fall through.  */
    case CSR_CLASS_SSCOFPMF:
      extension = "sscofpmf";
      break;
    case CSR_CLASS_SSTC:
    case CSR_CLASS_SSTC_AND_H:
    case CSR_CLASS_SSTC_32:
    case CSR_CLASS_SSTC_AND_H_32:
      is_rv32_only = (csr_class == CSR_CLASS_SSTC_32
		      || csr_class == CSR_CLASS_SSTC_AND_H_32);
      is_h_required = (csr_class == CSR_CLASS_SSTC_AND_H
		      || csr_class == CSR_CLASS_SSTC_AND_H_32);
      extension = "sstc";
      break;
    case CSR_CLASS_DEBUG:
      break;
    default:
      as_bad (_("internal: bad RISC-V CSR class (0x%x)"), csr_class);
    }

  if (riscv_opts.csr_check)
    {
      if (is_rv32_only && xlen != 32)
	as_warn (_("invalid CSR `%s', needs rv32i extension"), csr_name);
      if (is_h_required && !riscv_subset_supports (&riscv_rps_as, "h"))
	as_warn (_("invalid CSR `%s', needs `h' extension"), csr_name);

      if (extension != NULL
	  && !riscv_subset_supports (&riscv_rps_as, extension))
	as_warn (_("invalid CSR `%s', needs `%s' extension"),
		 csr_name, extension);
    }

  while (entry != NULL)
    {
      if (!need_check_version
	  || (default_priv_spec >= entry->define_version
	      && default_priv_spec < entry->abort_version))
       {
         /* Find the CSR according to the specific version.  */
         return entry->address;
       }
      entry = entry->next;
    }

  /* Can not find the CSR address from the chosen privileged version,
     so use the newly defined value.  */
  if (riscv_opts.csr_check)
    {
      const char *priv_name = NULL;
      RISCV_GET_PRIV_SPEC_NAME (priv_name, default_priv_spec);
      if (priv_name != NULL)
	as_warn (_("invalid CSR `%s' for the privileged spec `%s'"),
		 csr_name, priv_name);
    }

  return saved_entry->address;
}

/* Return -1 if the CSR has never been defined.  Otherwise, return
   the address.  */

static unsigned int
reg_csr_lookup_internal (const char *s)
{
  struct riscv_csr_extra *r =
    (struct riscv_csr_extra *) str_hash_find (csr_extra_hash, s);

  if (r == NULL)
    return -1U;

  return riscv_csr_address (s, r);
}

static unsigned int
reg_lookup_internal (const char *s, enum reg_class class)
{
  void *r;

  if (class == RCLASS_CSR)
    return reg_csr_lookup_internal (s);

  r = str_hash_find (reg_names_hash, s);
  if (r == NULL || DECODE_REG_CLASS (r) != class)
    return -1;

  if (riscv_subset_supports (&riscv_rps_as, "e")
      && class == RCLASS_GPR
      && DECODE_REG_NUM (r) > 15)
    return -1;

  return DECODE_REG_NUM (r);
}

static bool
reg_lookup (char **s, enum reg_class class, unsigned int *regnop)
{
  char *e;
  char save_c;
  int reg = -1;

  /* Find end of name.  */
  e = *s;
  if (is_name_beginner (*e))
    ++e;
  while (is_part_of_name (*e))
    ++e;

  /* Terminate name.  */
  save_c = *e;
  *e = '\0';

  /* Look for the register.  Advance to next token if one was recognized.  */
  if ((reg = reg_lookup_internal (*s, class)) >= 0)
    *s = e;

  *e = save_c;
  if (regnop)
    *regnop = reg;
  return reg >= 0;
}

static bool
arg_lookup (char **s, const char *const *array, size_t size, unsigned *regnop)
{
  const char *p = strchr (*s, ',');
  size_t i, len = p ? (size_t)(p - *s) : strlen (*s);

  if (len == 0)
    return false;

  for (i = 0; i < size; i++)
    if (array[i] != NULL && strncmp (array[i], *s, len) == 0
	&& array[i][len] == '\0')
      {
	*regnop = i;
	*s += len;
	return true;
      }

  return false;
}

static bool
flt_lookup (float f, const float *array, size_t size, unsigned *regnop)
{
  size_t i;

  for (i = 0; i < size; i++)
    if (array[i] == f)
      {
	*regnop = i;
	return true;
      }

  return false;
}

#define USE_BITS(mask,shift) (used_bits |= ((insn_t)(mask) << (shift)))
#define USE_IMM(n, s) \
  (used_bits |= ((insn_t)((1ull<<n)-1) << (s)))

/* For consistency checking, verify that all bits are specified either
   by the match/mask part of the instruction definition, or by the
   operand list. The `length` could be the actual instruction length or
   0 for auto-detection.  */

static bool
validate_riscv_insn (const struct riscv_opcode *opc, int length)
{
  const char *oparg, *opargStart;
  insn_t used_bits = opc->mask;
  int insn_width;
  insn_t required_bits;

  if (length == 0)
    length = riscv_insn_length (opc->match);
  /* We don't support instructions longer than 64-bits yet.  */
  if (length > 8)
    length = 8;
  insn_width = 8 * length;

  required_bits = ((insn_t)~0ULL) >> (64 - insn_width);

  if ((used_bits & opc->match) != (opc->match & required_bits))
    {
      as_bad (_("internal: bad RISC-V opcode (mask error): %s %s"),
	      opc->name, opc->args);
      return false;
    }

  for (oparg = opc->args; *oparg; ++oparg)
    {
      opargStart = oparg;
      switch (*oparg)
	{
	case 'C': /* RVC */
	  switch (*++oparg)
	    {
	    case 'U': break; /* CRS1, constrained to equal RD.  */
	    case 'c': break; /* CRS1, constrained to equal sp.  */
	    case 'T': /* CRS2, floating point.  */
	    case 'V': USE_BITS (OP_MASK_CRS2, OP_SH_CRS2); break;
	    case 'S': /* CRS1S, floating point.  */
	    case 's': USE_BITS (OP_MASK_CRS1S, OP_SH_CRS1S); break;
	    case 'w': break; /* CRS1S, constrained to equal RD.  */
	    case 'D': /* CRS2S, floating point.  */
	    case 't': USE_BITS (OP_MASK_CRS2S, OP_SH_CRS2S); break;
	    case 'x': break; /* CRS2S, constrained to equal RD.  */
	    case 'z': break; /* CRS2S, constrained to be x0.  */
	    case '>': /* CITYPE immediate, compressed shift.  */
	    case 'u': /* CITYPE immediate, compressed lui.  */
	    case 'v': /* CITYPE immediate, li to compressed lui.  */
	    case 'o': /* CITYPE immediate, allow zero.  */
	    case 'j': used_bits |= ENCODE_CITYPE_IMM (-1U); break;
	    case 'L': used_bits |= ENCODE_CITYPE_ADDI16SP_IMM (-1U); break;
	    case 'm': used_bits |= ENCODE_CITYPE_LWSP_IMM (-1U); break;
	    case 'n': used_bits |= ENCODE_CITYPE_LDSP_IMM (-1U); break;
	    case '6': used_bits |= ENCODE_CSSTYPE_IMM (-1U); break;
	    case 'M': used_bits |= ENCODE_CSSTYPE_SWSP_IMM (-1U); break;
	    case 'N': used_bits |= ENCODE_CSSTYPE_SDSP_IMM (-1U); break;
	    case '8': used_bits |= ENCODE_CIWTYPE_IMM (-1U); break;
	    case 'K': used_bits |= ENCODE_CIWTYPE_ADDI4SPN_IMM (-1U); break;
	    /* CLTYPE and CSTYPE have the same immediate encoding.  */
	    case '5': used_bits |= ENCODE_CLTYPE_IMM (-1U); break;
	    case 'k': used_bits |= ENCODE_CLTYPE_LW_IMM (-1U); break;
	    case 'l': used_bits |= ENCODE_CLTYPE_LD_IMM (-1U); break;
	    case 'p': used_bits |= ENCODE_CBTYPE_IMM (-1U); break;
	    case 'a': used_bits |= ENCODE_CJTYPE_IMM (-1U); break;
	    case 'F': /* Compressed funct for .insn directive.  */
	      switch (*++oparg)
		{
		case '6': USE_BITS (OP_MASK_CFUNCT6, OP_SH_CFUNCT6); break;
		case '4': USE_BITS (OP_MASK_CFUNCT4, OP_SH_CFUNCT4); break;
		case '3': USE_BITS (OP_MASK_CFUNCT3, OP_SH_CFUNCT3); break;
		case '2': USE_BITS (OP_MASK_CFUNCT2, OP_SH_CFUNCT2); break;
		default:
		  goto unknown_validate_operand;
		}
	      break;
	    default:
	      goto unknown_validate_operand;
	    }
	  break;  /* end RVC */
	case 'V': /* RVV */
	  switch (*++oparg)
	    {
	    case 'd':
	    case 'f': USE_BITS (OP_MASK_VD, OP_SH_VD); break;
	    case 'e': USE_BITS (OP_MASK_VWD, OP_SH_VWD); break;
	    case 's': USE_BITS (OP_MASK_VS1, OP_SH_VS1); break;
	    case 't': USE_BITS (OP_MASK_VS2, OP_SH_VS2); break;
	    case 'u': USE_BITS (OP_MASK_VS1, OP_SH_VS1);
		      USE_BITS (OP_MASK_VS2, OP_SH_VS2); break;
	    case 'v': USE_BITS (OP_MASK_VD, OP_SH_VD);
		      USE_BITS (OP_MASK_VS1, OP_SH_VS1);
		      USE_BITS (OP_MASK_VS2, OP_SH_VS2); break;
	    case '0': break;
	    case 'b': used_bits |= ENCODE_RVV_VB_IMM (-1U); break;
	    case 'c': used_bits |= ENCODE_RVV_VC_IMM (-1U); break;
	    case 'i':
	    case 'j':
	    case 'k': USE_BITS (OP_MASK_VIMM, OP_SH_VIMM); break;
	    case 'l': used_bits |= ENCODE_RVV_VI_UIMM6 (-1U); break;
	    case 'm': USE_BITS (OP_MASK_VMASK, OP_SH_VMASK); break;
	    case 'M': break; /* Macro operand, must be a mask register.  */
	    case 'T': break; /* Macro operand, must be a vector register.  */
	    default:
	      goto unknown_validate_operand;
	    }
	  break; /* end RVV */
	case ',': break;
	case '(': break;
	case ')': break;
	case '<': USE_BITS (OP_MASK_SHAMTW, OP_SH_SHAMTW); break;
	case '>': USE_BITS (OP_MASK_SHAMT, OP_SH_SHAMT); break;
	case 'A': break; /* Macro operand, must be symbol.  */
	case 'B': break; /* Macro operand, must be symbol or constant.  */
	case 'c': break; /* Macro operand, must be symbol or constant.  */
	case 'I': break; /* Macro operand, must be constant.  */
	case 'D': /* RD, floating point.  */
	case 'd': USE_BITS (OP_MASK_RD, OP_SH_RD); break;
	case 'y': USE_BITS (OP_MASK_BS,	OP_SH_BS); break;
	case 'Y': USE_BITS (OP_MASK_RNUM, OP_SH_RNUM); break;
	case 'Z': /* RS1, CSR number.  */
	case 'S': /* RS1, floating point.  */
	case 's': USE_BITS (OP_MASK_RS1, OP_SH_RS1); break;
	case 'U': /* RS1 and RS2 are the same, floating point.  */
	  USE_BITS (OP_MASK_RS1, OP_SH_RS1);
	  /* Fall through.  */
	case 'T': /* RS2, floating point.  */
	case 't': USE_BITS (OP_MASK_RS2, OP_SH_RS2); break;
	case 'R': /* RS3, floating point.  */
	case 'r': USE_BITS (OP_MASK_RS3, OP_SH_RS3); break;
	case 'm': USE_BITS (OP_MASK_RM, OP_SH_RM); break;
	case 'E': USE_BITS (OP_MASK_CSR, OP_SH_CSR); break;
	case 'P': USE_BITS (OP_MASK_PRED, OP_SH_PRED); break;
	case 'Q': USE_BITS (OP_MASK_SUCC, OP_SH_SUCC); break;
	case 'o': /* ITYPE immediate, load displacement.  */
	case 'j': used_bits |= ENCODE_ITYPE_IMM (-1U); break;
	case 'a': used_bits |= ENCODE_JTYPE_IMM (-1U); break;
	case 'p': used_bits |= ENCODE_BTYPE_IMM (-1U); break;
	case 'q': used_bits |= ENCODE_STYPE_IMM (-1U); break;
	case 'u': used_bits |= ENCODE_UTYPE_IMM (-1U); break;
	case 'z': break; /* Zero immediate.  */
	case '[': break; /* Unused operand.  */
	case ']': break; /* Unused operand.  */
	case '0': break; /* AMO displacement, must to zero.  */
	case '1': break; /* Relaxation operand.  */
	case 'F': /* Funct for .insn directive.  */
	  switch (*++oparg)
	    {
	      case '7': USE_BITS (OP_MASK_FUNCT7, OP_SH_FUNCT7); break;
	      case '3': USE_BITS (OP_MASK_FUNCT3, OP_SH_FUNCT3); break;
	      case '2': USE_BITS (OP_MASK_FUNCT2, OP_SH_FUNCT2); break;
	      default:
		goto unknown_validate_operand;
	    }
	  break;
	case 'O': /* Opcode for .insn directive.  */
	  switch (*++oparg)
	    {
	      case '4': USE_BITS (OP_MASK_OP, OP_SH_OP); break;
	      case '2': USE_BITS (OP_MASK_OP2, OP_SH_OP2); break;
	      default:
		goto unknown_validate_operand;
	    }
	  break;
	case 'W': /* Various operands.  */
	  switch (*++oparg)
	    {
	    case 'i':
	      switch (*++oparg)
		{
		case 'f': used_bits |= ENCODE_STYPE_IMM (-1U); break;
		default:
		  goto unknown_validate_operand;
		}
	      break;
	    case 'f':
	      switch (*++oparg)
		{
		case 'v': USE_BITS (OP_MASK_RS1, OP_SH_RS1); break;
		default:
		  goto unknown_validate_operand;
		}
	      break;
	    default:
	      goto unknown_validate_operand;
	    }
	  break;
	case 'X': /* Integer immediate.  */
	  {
	    size_t n;
	    size_t s;

	    switch (*++oparg)
	      {
		case 'l': /* Literal.  */
		  oparg += strcspn(oparg, ",") - 1;
		  break;
		case 's': /* 'XsN@S' ... N-bit signed immediate at bit S.  */
		  goto use_imm;
		case 'u': /* 'XuN@S' ... N-bit unsigned immediate at bit S.  */
		  goto use_imm;
		use_imm:
		  n = strtol (oparg + 1, (char **)&oparg, 10);
		  if (*oparg != '@')
		    goto unknown_validate_operand;
		  s = strtol (oparg + 1, (char **)&oparg, 10);
		  oparg--;

		  USE_IMM (n, s);
		  break;
		default:
		  goto unknown_validate_operand;
	      }
	  }
	  break;
	default:
	unknown_validate_operand:
	  as_bad (_("internal: bad RISC-V opcode "
		    "(unknown operand type `%s'): %s %s"),
		  opargStart, opc->name, opc->args);
	  return false;
	}
    }

  if (used_bits != required_bits)
    {
      as_bad (_("internal: bad RISC-V opcode "
		"(bits %#llx undefined or invalid): %s %s"),
	      (unsigned long long)(used_bits ^ required_bits),
	      opc->name, opc->args);
      return false;
    }
  return true;
}

#undef USE_BITS

struct percent_op_match
{
  const char *str;
  bfd_reloc_code_real_type reloc;
};

/* Common hash table initialization function for instruction and .insn
   directive.  */

static htab_t
init_opcode_hash (const struct riscv_opcode *opcodes,
		  bool insn_directive_p)
{
  int i = 0;
  int length;
  htab_t hash = str_htab_create ();
  while (opcodes[i].name)
    {
      const char *name = opcodes[i].name;
      if (str_hash_insert (hash, name, &opcodes[i], 0) != NULL)
	as_fatal (_("internal: duplicate %s"), name);

      do
	{
	  if (opcodes[i].pinfo != INSN_MACRO)
	    {
	      if (insn_directive_p)
		length = ((name[0] == 'c') ? 2 : 4);
	      else
		length = 0; /* Let assembler determine the length.  */
	      if (!validate_riscv_insn (&opcodes[i], length))
		as_fatal (_("internal: broken assembler.  "
			    "No assembly attempted"));
	    }
	  else
	    gas_assert (!insn_directive_p);
	  ++i;
	}
      while (opcodes[i].name && !strcmp (opcodes[i].name, name));
    }

  return hash;
}

/* This function is called once, at assembler startup time.  It should set up
   all the tables, etc. that the MD part of the assembler will need.  */

void
md_begin (void)
{
  unsigned long mach = xlen == 64 ? bfd_mach_riscv64 : bfd_mach_riscv32;

  if (! bfd_set_arch_mach (stdoutput, bfd_arch_riscv, mach))
    as_warn (_("could not set architecture and machine"));

  op_hash = init_opcode_hash (riscv_opcodes, false);
  insn_type_hash = init_opcode_hash (riscv_insn_types, true);

  reg_names_hash = str_htab_create ();
  hash_reg_names (RCLASS_GPR, riscv_gpr_names_numeric, NGPR);
  hash_reg_names (RCLASS_GPR, riscv_gpr_names_abi, NGPR);
  hash_reg_names (RCLASS_FPR, riscv_fpr_names_numeric, NFPR);
  hash_reg_names (RCLASS_FPR, riscv_fpr_names_abi, NFPR);
  hash_reg_names (RCLASS_VECR, riscv_vecr_names_numeric, NVECR);
  hash_reg_names (RCLASS_VECM, riscv_vecm_names_numeric, NVECM);
  /* Add "fp" as an alias for "s0".  */
  hash_reg_name (RCLASS_GPR, "fp", 8);

  /* Create and insert CSR hash tables.  */
  csr_extra_hash = str_htab_create ();
#define DECLARE_CSR(name, num, class, define_version, abort_version) \
  riscv_init_csr_hash (#name, num, class, define_version, abort_version);
#define DECLARE_CSR_ALIAS(name, num, class, define_version, abort_version) \
  DECLARE_CSR(name, num, class, define_version, abort_version);
#include "opcode/riscv-opc.h"
#undef DECLARE_CSR

  opcode_names_hash = str_htab_create ();
  init_opcode_names_hash ();

  /* Set the default alignment for the text section.  */
  record_alignment (text_section, riscv_opts.rvc ? 1 : 2);
}

static insn_t
riscv_apply_const_reloc (bfd_reloc_code_real_type reloc_type, bfd_vma value)
{
  switch (reloc_type)
    {
    case BFD_RELOC_32:
      return value;

    case BFD_RELOC_RISCV_HI20:
      return ENCODE_UTYPE_IMM (RISCV_CONST_HIGH_PART (value));

    case BFD_RELOC_RISCV_LO12_S:
      return ENCODE_STYPE_IMM (value);

    case BFD_RELOC_RISCV_LO12_I:
      return ENCODE_ITYPE_IMM (value);

    default:
      abort ();
    }
}

/* Output an instruction.  IP is the instruction information.
   ADDRESS_EXPR is an operand of the instruction to be used with
   RELOC_TYPE.  */

static void
append_insn (struct riscv_cl_insn *ip, expressionS *address_expr,
	     bfd_reloc_code_real_type reloc_type)
{
  dwarf2_emit_insn (0);

  if (reloc_type != BFD_RELOC_UNUSED)
    {
      reloc_howto_type *howto;

      gas_assert (address_expr);
      if (reloc_type == BFD_RELOC_12_PCREL
	  || reloc_type == BFD_RELOC_RISCV_JMP)
	{
	  int j = reloc_type == BFD_RELOC_RISCV_JMP;
	  int best_case = insn_length (ip);
	  unsigned worst_case = relaxed_branch_length (NULL, NULL, 0);

	  if (now_seg == absolute_section)
	    {
	      as_bad (_("relaxable branches not supported in absolute section"));
	      return;
	    }

	  add_relaxed_insn (ip, worst_case, best_case,
			    RELAX_BRANCH_ENCODE (j, best_case == 2, worst_case),
			    address_expr->X_add_symbol,
			    address_expr->X_add_number);
	  return;
	}
      else
	{
	  howto = bfd_reloc_type_lookup (stdoutput, reloc_type);
	  if (howto == NULL)
	    as_bad (_("internal: unsupported RISC-V relocation number %d"),
		    reloc_type);

	  ip->fixp = fix_new_exp (ip->frag, ip->where,
				  bfd_get_reloc_size (howto),
				  address_expr, false, reloc_type);

	  ip->fixp->fx_tcbit = riscv_opts.relax;
	}
    }

  add_fixed_insn (ip);

  /* We need to start a new frag after any instruction that can be
     optimized away or compressed by the linker during relaxation, to prevent
     the assembler from computing static offsets across such an instruction.
     This is necessary to get correct EH info.  */
  if (reloc_type == BFD_RELOC_RISCV_HI20
      || reloc_type == BFD_RELOC_RISCV_PCREL_HI20
      || reloc_type == BFD_RELOC_RISCV_TPREL_HI20
      || reloc_type == BFD_RELOC_RISCV_TPREL_ADD)
    {
      frag_wane (frag_now);
      frag_new (0);
    }
}

/* Build an instruction created by a macro expansion.  This is passed
   a pointer to the count of instructions created so far, an expression,
   the name of the instruction to build, an operand format string, and
   corresponding arguments.  */

static void
macro_build (expressionS *ep, const char *name, const char *fmt, ...)
{
  const struct riscv_opcode *mo;
  struct riscv_cl_insn insn;
  bfd_reloc_code_real_type r;
  va_list args;
  const char *fmtStart;

  va_start (args, fmt);

  r = BFD_RELOC_UNUSED;
  mo = (struct riscv_opcode *) str_hash_find (op_hash, name);
  gas_assert (mo);

  /* Find a non-RVC variant of the instruction.  append_insn will compress
     it if possible.  */
  while (riscv_insn_length (mo->match) < 4)
    mo++;
  gas_assert (strcmp (name, mo->name) == 0);

  create_insn (&insn, mo);
  for (;; ++fmt)
    {
      fmtStart = fmt;
      switch (*fmt)
	{
	case 'V': /* RVV */
	  switch (*++fmt)
	    {
	    case 'd':
	      INSERT_OPERAND (VD, insn, va_arg (args, int));
	      continue;
	    case 's':
	      INSERT_OPERAND (VS1, insn, va_arg (args, int));
	      continue;
	    case 't':
	      INSERT_OPERAND (VS2, insn, va_arg (args, int));
	      continue;
	    case 'm':
	      {
		int reg = va_arg (args, int);
		if (reg == -1)
		  {
		    INSERT_OPERAND (VMASK, insn, 1);
		    continue;
		  }
		else if (reg == 0)
		  {
		    INSERT_OPERAND (VMASK, insn, 0);
		    continue;
		  }
		else
		  goto unknown_macro_argument;
	      }
	    default:
	      goto unknown_macro_argument;
	    }
	  break;

	case 'd':
	  INSERT_OPERAND (RD, insn, va_arg (args, int));
	  continue;
	case 's':
	  INSERT_OPERAND (RS1, insn, va_arg (args, int));
	  continue;
	case 't':
	  INSERT_OPERAND (RS2, insn, va_arg (args, int));
	  continue;

	case 'j':
	case 'u':
	case 'q':
	  gas_assert (ep != NULL);
	  r = va_arg (args, int);
	  continue;

	case '\0':
	  break;
	case ',':
	  continue;
	default:
	unknown_macro_argument:
	  as_fatal (_("internal: invalid macro argument `%s'"), fmtStart);
	}
      break;
    }
  va_end (args);
  gas_assert (r == BFD_RELOC_UNUSED ? ep == NULL : ep != NULL);

  append_insn (&insn, ep, r);
}

/* Build an instruction created by a macro expansion.  Like md_assemble but
   accept a printf-style format string and arguments.  */

static void
md_assemblef (const char *format, ...)
{
  char *buf = NULL;
  va_list ap;
  int r;

  va_start (ap, format);

  r = vasprintf (&buf, format, ap);

  if (r < 0)
    as_fatal (_("internal: vasprintf failed"));

  md_assemble (buf);
  free(buf);

  va_end (ap);
}

/* Sign-extend 32-bit mode constants that have bit 31 set and all higher bits
   unset.  */

static void
normalize_constant_expr (expressionS *ex)
{
  if (xlen > 32)
    return;
  if ((ex->X_op == O_constant || ex->X_op == O_symbol)
      && IS_ZEXT_32BIT_NUM (ex->X_add_number))
    ex->X_add_number = (((ex->X_add_number & 0xffffffff) ^ 0x80000000)
			- 0x80000000);
}

/* Fail if an expression EX is not a constant.  IP is the instruction using EX.
   MAYBE_CSR is true if the symbol may be an unrecognized CSR name.  */

static void
check_absolute_expr (struct riscv_cl_insn *ip, expressionS *ex,
		     bool maybe_csr)
{
  if (ex->X_op == O_big)
    as_bad (_("unsupported large constant"));
  else if (maybe_csr && ex->X_op == O_symbol)
    as_bad (_("unknown CSR `%s'"),
	    S_GET_NAME (ex->X_add_symbol));
  else if (ex->X_op != O_constant)
    as_bad (_("instruction %s requires absolute expression"),
	    ip->insn_mo->name);
  normalize_constant_expr (ex);
}

static symbolS *
make_internal_label (void)
{
  return (symbolS *) local_symbol_make (FAKE_LABEL_NAME, now_seg, frag_now,
					frag_now_fix ());
}

/* Load an entry from the GOT.  */

static void
pcrel_access (int destreg, int tempreg, expressionS *ep,
	      const char *lo_insn, const char *lo_pattern,
	      bfd_reloc_code_real_type hi_reloc,
	      bfd_reloc_code_real_type lo_reloc)
{
  expressionS ep2;
  ep2.X_op = O_symbol;
  ep2.X_add_symbol = make_internal_label ();
  ep2.X_add_number = 0;

  macro_build (ep, "auipc", "d,u", tempreg, hi_reloc);
  macro_build (&ep2, lo_insn, lo_pattern, destreg, tempreg, lo_reloc);
}

static void
pcrel_load (int destreg, int tempreg, expressionS *ep, const char *lo_insn,
	    bfd_reloc_code_real_type hi_reloc,
	    bfd_reloc_code_real_type lo_reloc)
{
  pcrel_access (destreg, tempreg, ep, lo_insn, "d,s,j", hi_reloc, lo_reloc);
}

static void
pcrel_store (int srcreg, int tempreg, expressionS *ep, const char *lo_insn,
	     bfd_reloc_code_real_type hi_reloc,
	     bfd_reloc_code_real_type lo_reloc)
{
  pcrel_access (srcreg, tempreg, ep, lo_insn, "t,s,q", hi_reloc, lo_reloc);
}

/* PC-relative function call using AUIPC/JALR, relaxed to JAL.  */

static void
riscv_call (int destreg, int tempreg, expressionS *ep,
	    bfd_reloc_code_real_type reloc)
{
  /* Ensure the jalr is emitted to the same frag as the auipc.  */
  frag_grow (8);
  macro_build (ep, "auipc", "d,u", tempreg, reloc);
  macro_build (NULL, "jalr", "d,s", destreg, tempreg);
  /* See comment at end of append_insn.  */
  frag_wane (frag_now);
  frag_new (0);
}

/* Load an integer constant into a register.  */

static void
load_const (int reg, expressionS *ep)
{
  int shift = RISCV_IMM_BITS;
  bfd_vma upper_imm, sign = (bfd_vma) 1 << (RISCV_IMM_BITS - 1);
  expressionS upper = *ep, lower = *ep;
  lower.X_add_number = ((ep->X_add_number & (sign + sign - 1)) ^ sign) - sign;
  upper.X_add_number -= lower.X_add_number;

  if (ep->X_op != O_constant)
    {
      as_bad (_("unsupported large constant"));
      return;
    }

  if (xlen > 32 && !IS_SEXT_32BIT_NUM (ep->X_add_number))
    {
      /* Reduce to a signed 32-bit constant using SLLI and ADDI.  */
      while (((upper.X_add_number >> shift) & 1) == 0)
	shift++;

      upper.X_add_number = (int64_t) upper.X_add_number >> shift;
      load_const (reg, &upper);

      md_assemblef ("slli x%d, x%d, 0x%x", reg, reg, shift);
      if (lower.X_add_number != 0)
	md_assemblef ("addi x%d, x%d, %" PRId64, reg, reg,
		      (int64_t) lower.X_add_number);
    }
  else
    {
      /* Simply emit LUI and/or ADDI to build a 32-bit signed constant.  */
      int hi_reg = 0;

      if (upper.X_add_number != 0)
	{
	  /* Discard low part and zero-extend upper immediate.  */
	  upper_imm = ((uint32_t)upper.X_add_number >> shift);

	  md_assemblef ("lui x%d, 0x%" PRIx64, reg, (uint64_t) upper_imm);
	  hi_reg = reg;
	}

      if (lower.X_add_number != 0 || hi_reg == 0)
	md_assemblef ("%s x%d, x%d, %" PRId64, ADD32_INSN, reg, hi_reg,
		      (int64_t) lower.X_add_number);
    }
}

/* Zero extend and sign extend byte/half-word/word.  */

static void
riscv_ext (int destreg, int srcreg, unsigned shift, bool sign)
{
  if (sign)
    {
      md_assemblef ("slli x%d, x%d, 0x%x", destreg, srcreg, shift);
      md_assemblef ("srai x%d, x%d, 0x%x", destreg, destreg, shift);
    }
  else
    {
      md_assemblef ("slli x%d, x%d, 0x%x", destreg, srcreg, shift);
      md_assemblef ("srli x%d, x%d, 0x%x", destreg, destreg, shift);
    }
}

/* Expand RISC-V Vector macros into one or more instructions.  */

static void
vector_macro (struct riscv_cl_insn *ip)
{
  int vd = (ip->insn_opcode >> OP_SH_VD) & OP_MASK_VD;
  int vs1 = (ip->insn_opcode >> OP_SH_VS1) & OP_MASK_VS1;
  int vs2 = (ip->insn_opcode >> OP_SH_VS2) & OP_MASK_VS2;
  int vm = (ip->insn_opcode >> OP_SH_VMASK) & OP_MASK_VMASK;
  int vtemp = (ip->insn_opcode >> OP_SH_VFUNCT6) & OP_MASK_VFUNCT6;
  int mask = ip->insn_mo->mask;

  switch (mask)
    {
    case M_VMSGE:
      if (vm)
	{
	  /* Unmasked.  */
	  macro_build (NULL, "vmslt.vx", "Vd,Vt,sVm", vd, vs2, vs1, -1);
	  macro_build (NULL, "vmnand.mm", "Vd,Vt,Vs", vd, vd, vd);
	  break;
	}
      if (vtemp != 0)
	{
	  /* Masked.  Have vtemp to avoid overlap constraints.  */
	  if (vd == vm)
	    {
	      macro_build (NULL, "vmslt.vx", "Vd,Vt,s", vtemp, vs2, vs1);
	      macro_build (NULL, "vmandnot.mm", "Vd,Vt,Vs", vd, vm, vtemp);
	    }
	  else
	    {
	      /* Preserve the value of vd if not updating by vm.  */
	      macro_build (NULL, "vmslt.vx", "Vd,Vt,s", vtemp, vs2, vs1);
	      macro_build (NULL, "vmandnot.mm", "Vd,Vt,Vs", vtemp, vm, vtemp);
	      macro_build (NULL, "vmandnot.mm", "Vd,Vt,Vs", vd, vd, vm);
	      macro_build (NULL, "vmor.mm", "Vd,Vt,Vs", vd, vtemp, vd);
	    }
	}
      else if (vd != vm)
	{
	  /* Masked.  This may cause the vd overlaps vs2, when LMUL > 1.  */
	  macro_build (NULL, "vmslt.vx", "Vd,Vt,sVm", vd, vs2, vs1, vm);
	  macro_build (NULL, "vmxor.mm", "Vd,Vt,Vs", vd, vd, vm);
	}
      else
	as_bad (_("must provide temp if destination overlaps mask"));
      break;

    case M_VMSGEU:
      if (vm)
	{
	  /* Unmasked.  */
	  macro_build (NULL, "vmsltu.vx", "Vd,Vt,sVm", vd, vs2, vs1, -1);
	  macro_build (NULL, "vmnand.mm", "Vd,Vt,Vs", vd, vd, vd);
	  break;
	}
      if (vtemp != 0)
	{
	  /* Masked.  Have vtemp to avoid overlap constraints.  */
	  if (vd == vm)
	    {
	      macro_build (NULL, "vmsltu.vx", "Vd,Vt,s", vtemp, vs2, vs1);
	      macro_build (NULL, "vmandnot.mm", "Vd,Vt,Vs", vd, vm, vtemp);
	    }
	  else
	    {
	      /* Preserve the value of vd if not updating by vm.  */
	      macro_build (NULL, "vmsltu.vx", "Vd,Vt,s", vtemp, vs2, vs1);
	      macro_build (NULL, "vmandnot.mm", "Vd,Vt,Vs", vtemp, vm, vtemp);
	      macro_build (NULL, "vmandnot.mm", "Vd,Vt,Vs", vd, vd, vm);
	      macro_build (NULL, "vmor.mm", "Vd,Vt,Vs", vd, vtemp, vd);
	    }
	}
      else if (vd != vm)
	{
	  /* Masked.  This may cause the vd overlaps vs2, when LMUL > 1.  */
	  macro_build (NULL, "vmsltu.vx", "Vd,Vt,sVm", vd, vs2, vs1, vm);
	  macro_build (NULL, "vmxor.mm", "Vd,Vt,Vs", vd, vd, vm);
	}
      else
	as_bad (_("must provide temp if destination overlaps mask"));
      break;

    default:
      break;
    }
}

/* Expand RISC-V assembly macros into one or more instructions.  */

static void
macro (struct riscv_cl_insn *ip, expressionS *imm_expr,
       bfd_reloc_code_real_type *imm_reloc)
{
  int rd = (ip->insn_opcode >> OP_SH_RD) & OP_MASK_RD;
  int rs1 = (ip->insn_opcode >> OP_SH_RS1) & OP_MASK_RS1;
  int rs2 = (ip->insn_opcode >> OP_SH_RS2) & OP_MASK_RS2;
  int mask = ip->insn_mo->mask;

  switch (mask)
    {
    case M_LI:
      load_const (rd, imm_expr);
      break;

    case M_LA:
    case M_LLA:
    case M_LGA:
      /* Load the address of a symbol into a register.  */
      if (!IS_SEXT_32BIT_NUM (imm_expr->X_add_number))
	as_bad (_("offset too large"));

      if (imm_expr->X_op == O_constant)
	load_const (rd, imm_expr);
      /* Global PIC symbol.  */
      else if ((riscv_opts.pic && mask == M_LA)
	       || mask == M_LGA)
	pcrel_load (rd, rd, imm_expr, LOAD_ADDRESS_INSN,
		    BFD_RELOC_RISCV_GOT_HI20, BFD_RELOC_RISCV_PCREL_LO12_I);
      /* Local PIC symbol, or any non-PIC symbol.  */
      else
	pcrel_load (rd, rd, imm_expr, "addi",
		    BFD_RELOC_RISCV_PCREL_HI20, BFD_RELOC_RISCV_PCREL_LO12_I);
      break;

    case M_LA_TLS_GD:
      pcrel_load (rd, rd, imm_expr, "addi",
		  BFD_RELOC_RISCV_TLS_GD_HI20, BFD_RELOC_RISCV_PCREL_LO12_I);
      break;

    case M_LA_TLS_IE:
      pcrel_load (rd, rd, imm_expr, LOAD_ADDRESS_INSN,
		  BFD_RELOC_RISCV_TLS_GOT_HI20, BFD_RELOC_RISCV_PCREL_LO12_I);
      break;

    case M_LB:
      pcrel_load (rd, rd, imm_expr, "lb",
		  BFD_RELOC_RISCV_PCREL_HI20, BFD_RELOC_RISCV_PCREL_LO12_I);
      break;

    case M_LBU:
      pcrel_load (rd, rd, imm_expr, "lbu",
		  BFD_RELOC_RISCV_PCREL_HI20, BFD_RELOC_RISCV_PCREL_LO12_I);
      break;

    case M_LH:
      pcrel_load (rd, rd, imm_expr, "lh",
		  BFD_RELOC_RISCV_PCREL_HI20, BFD_RELOC_RISCV_PCREL_LO12_I);
      break;

    case M_LHU:
      pcrel_load (rd, rd, imm_expr, "lhu",
		  BFD_RELOC_RISCV_PCREL_HI20, BFD_RELOC_RISCV_PCREL_LO12_I);
      break;

    case M_LW:
      pcrel_load (rd, rd, imm_expr, "lw",
		  BFD_RELOC_RISCV_PCREL_HI20, BFD_RELOC_RISCV_PCREL_LO12_I);
      break;

    case M_LWU:
      pcrel_load (rd, rd, imm_expr, "lwu",
		  BFD_RELOC_RISCV_PCREL_HI20, BFD_RELOC_RISCV_PCREL_LO12_I);
      break;

    case M_LD:
      pcrel_load (rd, rd, imm_expr, "ld",
		  BFD_RELOC_RISCV_PCREL_HI20, BFD_RELOC_RISCV_PCREL_LO12_I);
      break;

    case M_FLW:
      pcrel_load (rd, rs1, imm_expr, "flw",
		  BFD_RELOC_RISCV_PCREL_HI20, BFD_RELOC_RISCV_PCREL_LO12_I);
      break;

    case M_FLD:
      pcrel_load (rd, rs1, imm_expr, "fld",
		  BFD_RELOC_RISCV_PCREL_HI20, BFD_RELOC_RISCV_PCREL_LO12_I);
      break;

    case M_SB:
      pcrel_store (rs2, rs1, imm_expr, "sb",
		   BFD_RELOC_RISCV_PCREL_HI20, BFD_RELOC_RISCV_PCREL_LO12_S);
      break;

    case M_SH:
      pcrel_store (rs2, rs1, imm_expr, "sh",
		   BFD_RELOC_RISCV_PCREL_HI20, BFD_RELOC_RISCV_PCREL_LO12_S);
      break;

    case M_SW:
      pcrel_store (rs2, rs1, imm_expr, "sw",
		   BFD_RELOC_RISCV_PCREL_HI20, BFD_RELOC_RISCV_PCREL_LO12_S);
      break;

    case M_SD:
      pcrel_store (rs2, rs1, imm_expr, "sd",
		   BFD_RELOC_RISCV_PCREL_HI20, BFD_RELOC_RISCV_PCREL_LO12_S);
      break;

    case M_FSW:
      pcrel_store (rs2, rs1, imm_expr, "fsw",
		   BFD_RELOC_RISCV_PCREL_HI20, BFD_RELOC_RISCV_PCREL_LO12_S);
      break;

    case M_FSD:
      pcrel_store (rs2, rs1, imm_expr, "fsd",
		   BFD_RELOC_RISCV_PCREL_HI20, BFD_RELOC_RISCV_PCREL_LO12_S);
      break;

    case M_CALL:
      riscv_call (rd, rs1, imm_expr, *imm_reloc);
      break;

    case M_ZEXTH:
      riscv_ext (rd, rs1, xlen - 16, false);
      break;

    case M_ZEXTW:
      riscv_ext (rd, rs1, xlen - 32, false);
      break;

    case M_SEXTB:
      riscv_ext (rd, rs1, xlen - 8, true);
      break;

    case M_SEXTH:
      riscv_ext (rd, rs1, xlen - 16, true);
      break;

    case M_VMSGE:
    case M_VMSGEU:
      vector_macro (ip);
      break;

    case M_FLH:
      pcrel_load (rd, rs1, imm_expr, "flh",
		  BFD_RELOC_RISCV_PCREL_HI20, BFD_RELOC_RISCV_PCREL_LO12_I);
      break;
    case M_FSH:
      pcrel_store (rs2, rs1, imm_expr, "fsh",
		   BFD_RELOC_RISCV_PCREL_HI20, BFD_RELOC_RISCV_PCREL_LO12_S);
      break;

    default:
      as_bad (_("internal: macro %s not implemented"), ip->insn_mo->name);
      break;
    }
}

static const struct percent_op_match percent_op_utype[] =
{
  {"tprel_hi", BFD_RELOC_RISCV_TPREL_HI20},
  {"pcrel_hi", BFD_RELOC_RISCV_PCREL_HI20},
  {"got_pcrel_hi", BFD_RELOC_RISCV_GOT_HI20},
  {"tls_ie_pcrel_hi", BFD_RELOC_RISCV_TLS_GOT_HI20},
  {"tls_gd_pcrel_hi", BFD_RELOC_RISCV_TLS_GD_HI20},
  {"hi", BFD_RELOC_RISCV_HI20},
  {0, 0}
};

static const struct percent_op_match percent_op_itype[] =
{
  {"lo", BFD_RELOC_RISCV_LO12_I},
  {"tprel_lo", BFD_RELOC_RISCV_TPREL_LO12_I},
  {"pcrel_lo", BFD_RELOC_RISCV_PCREL_LO12_I},
  {0, 0}
};

static const struct percent_op_match percent_op_stype[] =
{
  {"lo", BFD_RELOC_RISCV_LO12_S},
  {"tprel_lo", BFD_RELOC_RISCV_TPREL_LO12_S},
  {"pcrel_lo", BFD_RELOC_RISCV_PCREL_LO12_S},
  {0, 0}
};

static const struct percent_op_match percent_op_rtype[] =
{
  {"tprel_add", BFD_RELOC_RISCV_TPREL_ADD},
  {0, 0}
};

static const struct percent_op_match percent_op_null[] =
{
  {0, 0}
};

/* Return true if *STR points to a relocation operator.  When returning true,
   move *STR over the operator and store its relocation code in *RELOC.
   Leave both *STR and *RELOC alone when returning false.  */

static bool
parse_relocation (char **str, bfd_reloc_code_real_type *reloc,
		  const struct percent_op_match *percent_op)
{
  for ( ; percent_op->str; percent_op++)
    if (strncasecmp (*str + 1, percent_op->str, strlen (percent_op->str)) == 0)
      {
	size_t len = 1 + strlen (percent_op->str);

	while (ISSPACE ((*str)[len]))
	  ++len;
	if ((*str)[len] != '(')
	  continue;

	*str += len;
	*reloc = percent_op->reloc;

	/* Check whether the output BFD supports this relocation.
	   If not, issue an error and fall back on something safe.  */
	if (*reloc != BFD_RELOC_UNUSED
	    && !bfd_reloc_type_lookup (stdoutput, *reloc))
	  {
	    as_bad ("internal: relocation %s isn't supported by the "
		    "current ABI", percent_op->str);
	    *reloc = BFD_RELOC_UNUSED;
	  }
	return true;
      }
  return false;
}

static void
my_getExpression (expressionS *ep, char *str)
{
  char *save_in;

  save_in = input_line_pointer;
  input_line_pointer = str;
  expression (ep);
  expr_parse_end = input_line_pointer;
  input_line_pointer = save_in;
}

/* Parse string STR as a 16-bit relocatable operand.  Store the
   expression in *EP and the relocation, if any, in RELOC.
   Return the number of relocation operators used (0 or 1).

   On exit, EXPR_PARSE_END points to the first character after the
   expression.  */

static size_t
my_getSmallExpression (expressionS *ep, bfd_reloc_code_real_type *reloc,
		       char *str, const struct percent_op_match *percent_op)
{
  size_t reloc_index;
  unsigned crux_depth, str_depth;
  bool orig_probing = probing_insn_operands;
  char *crux;

  /* Search for the start of the main expression.

     End the loop with CRUX pointing to the start of the main expression and
     with CRUX_DEPTH containing the number of open brackets at that point.  */
  reloc_index = -1;
  str_depth = 0;
  do
    {
      reloc_index++;
      crux = str;
      crux_depth = str_depth;

      /* Skip over whitespace and brackets, keeping count of the number
	 of brackets.  */
      while (*str == ' ' || *str == '\t' || *str == '(')
	if (*str++ == '(')
	  str_depth++;
    }
  while (*str == '%'
	 && reloc_index < 1
	 && parse_relocation (&str, reloc, percent_op));

  if (*str == '%')
    {
       /* expression() will choke on anything looking like an (unrecognized)
	  relocation specifier.  Don't even call it, avoiding multiple (and
	  perhaps redundant) error messages; our caller will issue one.  */
       ep->X_op = O_illegal;
       return 0;
    }

  /* Anything inside parentheses or subject to a relocation operator cannot
     be a register and hence can be treated the same as operands to
     directives (other than .insn).  */
  if (str_depth || reloc_index)
    probing_insn_operands = false;

  my_getExpression (ep, crux);
  str = expr_parse_end;

  probing_insn_operands = orig_probing;

  /* Match every open bracket.  */
  while (crux_depth > 0 && (*str == ')' || *str == ' ' || *str == '\t'))
    if (*str++ == ')')
      crux_depth--;

  if (crux_depth > 0)
    as_bad ("unclosed '('");

  expr_parse_end = str;

  return reloc_index;
}

/* Parse opcode name, could be an mnemonics or number.  */

static size_t
my_getOpcodeExpression (expressionS *ep, bfd_reloc_code_real_type *reloc,
			char *str)
{
  const struct opcode_name_t *o = opcode_name_lookup (&str);

  if (o != NULL)
    {
      ep->X_op = O_constant;
      ep->X_add_number = o->val;
      return 0;
    }

  return my_getSmallExpression (ep, reloc, str, percent_op_null);
}

/* Parse string STR as a vsetvli operand.  Store the expression in *EP.
   On exit, EXPR_PARSE_END points to the first character after the
   expression.  */

static void
my_getVsetvliExpression (expressionS *ep, char *str)
{
  unsigned int vsew_value = 0, vlmul_value = 0;
  unsigned int vta_value = 0, vma_value = 0;
  bfd_boolean vsew_found = FALSE, vlmul_found = FALSE;
  bfd_boolean vta_found = FALSE, vma_found = FALSE;

  if (arg_lookup (&str, riscv_vsew, ARRAY_SIZE (riscv_vsew), &vsew_value))
    {
      if (*str == ',')
	++str;
      if (vsew_found)
	as_bad (_("multiple vsew constants"));
      vsew_found = TRUE;
    }
  if (arg_lookup (&str, riscv_vlmul, ARRAY_SIZE (riscv_vlmul), &vlmul_value))
    {
      if (*str == ',')
	++str;
      if (vlmul_found)
	as_bad (_("multiple vlmul constants"));
      vlmul_found = TRUE;
    }
  if (arg_lookup (&str, riscv_vta, ARRAY_SIZE (riscv_vta), &vta_value))
    {
      if (*str == ',')
	++str;
      if (vta_found)
	as_bad (_("multiple vta constants"));
      vta_found = TRUE;
    }
  if (arg_lookup (&str, riscv_vma, ARRAY_SIZE (riscv_vma), &vma_value))
    {
      if (*str == ',')
	++str;
      if (vma_found)
	as_bad (_("multiple vma constants"));
      vma_found = TRUE;
    }

  if (vsew_found || vlmul_found || vta_found || vma_found)
    {
      ep->X_op = O_constant;
      ep->X_add_number = (vlmul_value << OP_SH_VLMUL)
			 | (vsew_value << OP_SH_VSEW)
			 | (vta_value << OP_SH_VTA)
			 | (vma_value << OP_SH_VMA);
      expr_parse_end = str;
    }
  else
    {
      my_getExpression (ep, str);
      str = expr_parse_end;
    }
}

/* Detect and handle implicitly zero load-store offsets.  For example,
   "lw t0, (t1)" is shorthand for "lw t0, 0(t1)".  Return true if such
   an implicit offset was detected.  */

static bool
riscv_handle_implicit_zero_offset (expressionS *ep, const char *s)
{
  /* Check whether there is only a single bracketed expression left.
     If so, it must be the base register and the constant must be zero.  */
  if (*s == '(' && strchr (s + 1, '(') == 0)
    {
      ep->X_op = O_constant;
      ep->X_add_number = 0;
      return true;
    }

  return false;
}

/* All RISC-V CSR instructions belong to one of these classes.  */
enum csr_insn_type
{
  INSN_NOT_CSR,
  INSN_CSRRW,
  INSN_CSRRS,
  INSN_CSRRC
};

/* Return which CSR instruction is checking.  */

static enum csr_insn_type
riscv_csr_insn_type (insn_t insn)
{
  if (((insn ^ MATCH_CSRRW) & MASK_CSRRW) == 0
      || ((insn ^ MATCH_CSRRWI) & MASK_CSRRWI) == 0)
    return INSN_CSRRW;
  else if (((insn ^ MATCH_CSRRS) & MASK_CSRRS) == 0
	   || ((insn ^ MATCH_CSRRSI) & MASK_CSRRSI) == 0)
    return INSN_CSRRS;
  else if (((insn ^ MATCH_CSRRC) & MASK_CSRRC) == 0
	   || ((insn ^ MATCH_CSRRCI) & MASK_CSRRCI) == 0)
    return INSN_CSRRC;
  else
    return INSN_NOT_CSR;
}

/* CSRRW and CSRRWI always write CSR.  CSRRS, CSRRC, CSRRSI and CSRRCI write
   CSR when RS1 isn't zero.  The CSR is read only if the [11:10] bits of
   CSR address is 0x3.  */

static bool
riscv_csr_read_only_check (insn_t insn)
{
  int csr = (insn & (OP_MASK_CSR << OP_SH_CSR)) >> OP_SH_CSR;
  int rs1 = (insn & (OP_MASK_RS1 << OP_SH_RS1)) >> OP_SH_RS1;
  int readonly = (((csr & (0x3 << 10)) >> 10) == 0x3);
  enum csr_insn_type csr_insn = riscv_csr_insn_type (insn);

  if (readonly
      && (((csr_insn == INSN_CSRRS
	    || csr_insn == INSN_CSRRC)
	   && rs1 != 0)
	  || csr_insn == INSN_CSRRW))
    return false;

  return true;
}

/* Return true if it is a privileged instruction.  Otherwise, return false.

   uret is actually a N-ext instruction.  So it is better to regard it as
   an user instruction rather than the priv instruction.

   hret is used to return from traps in H-mode.  H-mode is removed since
   the v1.10 priv spec, but probably be added in the new hypervisor spec.
   Therefore, hret should be controlled by the hypervisor spec rather than
   priv spec in the future.

   dret is defined in the debug spec, so it should be checked in the future,
   too.  */

static bool
riscv_is_priv_insn (insn_t insn)
{
  return (((insn ^ MATCH_SRET) & MASK_SRET) == 0
	  || ((insn ^ MATCH_MRET) & MASK_MRET) == 0
	  || ((insn ^ MATCH_SFENCE_VMA) & MASK_SFENCE_VMA) == 0
	  || ((insn ^ MATCH_WFI) & MASK_WFI) == 0
  /* The sfence.vm is dropped in the v1.10 priv specs, but we still need to
     check it here to keep the compatible.  */
	  || ((insn ^ MATCH_SFENCE_VM) & MASK_SFENCE_VM) == 0);
}

static symbolS *deferred_sym_rootP;
static symbolS *deferred_sym_lastP;
/* Since symbols can't easily be freed, try to recycle ones which weren't
   committed.  */
static symbolS *orphan_sym_rootP;
static symbolS *orphan_sym_lastP;

/* This routine assembles an instruction into its binary format.  As a
   side effect, it sets the global variable imm_reloc to the type of
   relocation to do if one of the operands is an address expression.  */

static struct riscv_ip_error
riscv_ip (char *str, struct riscv_cl_insn *ip, expressionS *imm_expr,
	  bfd_reloc_code_real_type *imm_reloc, htab_t hash)
{
  /* The operand string defined in the riscv_opcodes.  */
  const char *oparg, *opargStart;
  /* The parsed operands from assembly.  */
  char *asarg, *asargStart;
  char save_c = 0;
  struct riscv_opcode *insn;
  unsigned int regno;
  const struct percent_op_match *p;
  struct riscv_ip_error error;
  error.msg = "unrecognized opcode";
  error.statement = str;
  error.missing_ext = NULL;
  /* Indicate we are assembling instruction with CSR.  */
  bool insn_with_csr = false;

  /* Parse the name of the instruction.  Terminate the string if whitespace
     is found so that str_hash_find only sees the name part of the string.  */
  for (asarg = str; *asarg!= '\0'; ++asarg)
    if (ISSPACE (*asarg))
      {
	save_c = *asarg;
	*asarg++ = '\0';
	break;
      }

  insn = (struct riscv_opcode *) str_hash_find (hash, str);

  probing_insn_operands = true;

  asargStart = asarg;
  for ( ; insn && insn->name && strcmp (insn->name, str) == 0; insn++)
    {
      if ((insn->xlen_requirement != 0) && (xlen != insn->xlen_requirement))
	continue;

      if (!riscv_multi_subset_supports (&riscv_rps_as, insn->insn_class))
	{
	  error.missing_ext = riscv_multi_subset_supports_ext (&riscv_rps_as,
							       insn->insn_class);
	  continue;
	}

      /* Reset error message of the previous round.  */
      error.msg = _("illegal operands");
      error.missing_ext = NULL;

      /* Purge deferred symbols from the previous round, if any.  */
      while (deferred_sym_rootP)
	{
	  symbolS *sym = deferred_sym_rootP;

	  symbol_remove (sym, &deferred_sym_rootP, &deferred_sym_lastP);
	  symbol_append (sym, orphan_sym_lastP, &orphan_sym_rootP,
			 &orphan_sym_lastP);
	}

      create_insn (ip, insn);

      imm_expr->X_op = O_absent;
      *imm_reloc = BFD_RELOC_UNUSED;
      p = percent_op_null;

      for (oparg = insn->args;; ++oparg)
	{
	  opargStart = oparg;
	  asarg += strspn (asarg, " \t");
	  switch (*oparg)
	    {
	    case '\0': /* End of args.  */
	      if (insn->pinfo != INSN_MACRO)
		{
		  if (!insn->match_func (insn, ip->insn_opcode))
		    break;

		  /* For .insn, insn->match and insn->mask are 0.  */
		  if (riscv_insn_length ((insn->match == 0 && insn->mask == 0)
					 ? ip->insn_opcode
					 : insn->match) == 2
		      && !riscv_opts.rvc)
		    break;

		  if (riscv_is_priv_insn (ip->insn_opcode))
		    explicit_priv_attr = true;

		  /* Check if we write a read-only CSR by the CSR
		     instruction.  */
		  if (insn_with_csr
		      && riscv_opts.csr_check
		      && !riscv_csr_read_only_check (ip->insn_opcode))
		    {
		      /* Restore the character in advance, since we want to
			 report the detailed warning message here.  */
		      if (save_c)
			*(asargStart - 1) = save_c;
		      as_warn (_("read-only CSR is written `%s'"), str);
		      insn_with_csr = false;
		    }

		  /* The (segmant) load and store with EEW 64 cannot be used
		     when zve32x is enabled.  */
		  if (ip->insn_mo->pinfo & INSN_V_EEW64
		      && riscv_subset_supports (&riscv_rps_as, "zve32x")
		      && !riscv_subset_supports (&riscv_rps_as, "zve64x"))
		    {
		      error.msg = _("illegal opcode for zve32x");
		      break;
		    }
		}
	      if (*asarg != '\0')
		break;

	      /* Successful assembly.  */
	      error.msg = NULL;
	      insn_with_csr = false;

	      /* Commit deferred symbols, if any.  */
	      while (deferred_sym_rootP)
		{
		  symbolS *sym = deferred_sym_rootP;

		  symbol_remove (sym, &deferred_sym_rootP,
				 &deferred_sym_lastP);
		  symbol_append (sym, symbol_lastP, &symbol_rootP,
				 &symbol_lastP);
		  symbol_table_insert (sym);
		}
	      goto out;

	    case 'C': /* RVC */
	      switch (*++oparg)
		{
		case 's': /* RS1 x8-x15.  */
		  if (!reg_lookup (&asarg, RCLASS_GPR, &regno)
		      || !(regno >= 8 && regno <= 15))
		    break;
		  INSERT_OPERAND (CRS1S, *ip, regno % 8);
		  continue;
		case 'w': /* RS1 x8-x15, constrained to equal RD x8-x15.  */
		  if (!reg_lookup (&asarg, RCLASS_GPR, &regno)
		      || EXTRACT_OPERAND (CRS1S, ip->insn_opcode) + 8 != regno)
		    break;
		  continue;
		case 't': /* RS2 x8-x15.  */
		  if (!reg_lookup (&asarg, RCLASS_GPR, &regno)
		      || !(regno >= 8 && regno <= 15))
		    break;
		  INSERT_OPERAND (CRS2S, *ip, regno % 8);
		  continue;
		case 'x': /* RS2 x8-x15, constrained to equal RD x8-x15.  */
		  if (!reg_lookup (&asarg, RCLASS_GPR, &regno)
		      || EXTRACT_OPERAND (CRS2S, ip->insn_opcode) + 8 != regno)
		    break;
		  continue;
		case 'U': /* RS1, constrained to equal RD.  */
		  if (!reg_lookup (&asarg, RCLASS_GPR, &regno)
		      || EXTRACT_OPERAND (RD, ip->insn_opcode) != regno)
		    break;
		  continue;
		case 'V': /* RS2 */
		  if (!reg_lookup (&asarg, RCLASS_GPR, &regno))
		    break;
		  INSERT_OPERAND (CRS2, *ip, regno);
		  continue;
		case 'c': /* RS1, constrained to equal sp.  */
		  if (!reg_lookup (&asarg, RCLASS_GPR, &regno)
		      || regno != X_SP)
		    break;
		  continue;
		case 'z': /* RS2, constrained to equal x0.  */
		  if (!reg_lookup (&asarg, RCLASS_GPR, &regno)
		      || regno != 0)
		    break;
		  continue;
		case '>': /* Shift amount, 0 - (XLEN-1).  */
		  if (my_getSmallExpression (imm_expr, imm_reloc, asarg, p)
		      || imm_expr->X_op != O_constant
		      || (unsigned long) imm_expr->X_add_number >= xlen)
		    break;
		  ip->insn_opcode |= ENCODE_CITYPE_IMM (imm_expr->X_add_number);
		rvc_imm_done:
		  asarg = expr_parse_end;
		  imm_expr->X_op = O_absent;
		  continue;
		case '5':
		  if (my_getSmallExpression (imm_expr, imm_reloc, asarg, p)
		      || imm_expr->X_op != O_constant
		      || imm_expr->X_add_number < 0
		      || imm_expr->X_add_number >= 32
		      || !VALID_CLTYPE_IMM ((valueT) imm_expr->X_add_number))
		    break;
		  ip->insn_opcode |= ENCODE_CLTYPE_IMM (imm_expr->X_add_number);
		  goto rvc_imm_done;
		case '6':
		  if (my_getSmallExpression (imm_expr, imm_reloc, asarg, p)
		      || imm_expr->X_op != O_constant
		      || imm_expr->X_add_number < 0
		      || imm_expr->X_add_number >= 64
		      || !VALID_CSSTYPE_IMM ((valueT) imm_expr->X_add_number))
		    break;
		  ip->insn_opcode |= ENCODE_CSSTYPE_IMM (imm_expr->X_add_number);
		  goto rvc_imm_done;
		case '8':
		  if (my_getSmallExpression (imm_expr, imm_reloc, asarg, p)
		      || imm_expr->X_op != O_constant
		      || imm_expr->X_add_number < 0
		      || imm_expr->X_add_number >= 256
		      || !VALID_CIWTYPE_IMM ((valueT) imm_expr->X_add_number))
		    break;
		  ip->insn_opcode |= ENCODE_CIWTYPE_IMM (imm_expr->X_add_number);
		  goto rvc_imm_done;
		case 'j':
		  if (my_getSmallExpression (imm_expr, imm_reloc, asarg, p)
		      || imm_expr->X_op != O_constant
		      || imm_expr->X_add_number == 0
		      || !VALID_CITYPE_IMM ((valueT) imm_expr->X_add_number))
		    break;
		  ip->insn_opcode |= ENCODE_CITYPE_IMM (imm_expr->X_add_number);
		  goto rvc_imm_done;
		case 'k':
		  if (riscv_handle_implicit_zero_offset (imm_expr, asarg))
		    continue;
		  if (my_getSmallExpression (imm_expr, imm_reloc, asarg, p)
		      || imm_expr->X_op != O_constant
		      || !VALID_CLTYPE_LW_IMM ((valueT) imm_expr->X_add_number))
		    break;
		  ip->insn_opcode |= ENCODE_CLTYPE_LW_IMM (imm_expr->X_add_number);
		  goto rvc_imm_done;
		case 'l':
		  if (riscv_handle_implicit_zero_offset (imm_expr, asarg))
		    continue;
		  if (my_getSmallExpression (imm_expr, imm_reloc, asarg, p)
		      || imm_expr->X_op != O_constant
		      || !VALID_CLTYPE_LD_IMM ((valueT) imm_expr->X_add_number))
		    break;
		  ip->insn_opcode |= ENCODE_CLTYPE_LD_IMM (imm_expr->X_add_number);
		  goto rvc_imm_done;
		case 'm':
		  if (riscv_handle_implicit_zero_offset (imm_expr, asarg))
		    continue;
		  if (my_getSmallExpression (imm_expr, imm_reloc, asarg, p)
		      || imm_expr->X_op != O_constant
		      || !VALID_CITYPE_LWSP_IMM ((valueT) imm_expr->X_add_number))
		    break;
		  ip->insn_opcode |=
		    ENCODE_CITYPE_LWSP_IMM (imm_expr->X_add_number);
		  goto rvc_imm_done;
		case 'n':
		  if (riscv_handle_implicit_zero_offset (imm_expr, asarg))
		    continue;
		  if (my_getSmallExpression (imm_expr, imm_reloc, asarg, p)
		      || imm_expr->X_op != O_constant
		      || !VALID_CITYPE_LDSP_IMM ((valueT) imm_expr->X_add_number))
		    break;
		  ip->insn_opcode |=
		    ENCODE_CITYPE_LDSP_IMM (imm_expr->X_add_number);
		  goto rvc_imm_done;
		case 'o':
		  if (my_getSmallExpression (imm_expr, imm_reloc, asarg, p)
		      || imm_expr->X_op != O_constant
		      /* C.addiw, c.li, and c.andi allow zero immediate.
			 C.addi allows zero immediate as hint.  Otherwise this
			 is same as 'j'.  */
		      || !VALID_CITYPE_IMM ((valueT) imm_expr->X_add_number))
		    break;
		  ip->insn_opcode |= ENCODE_CITYPE_IMM (imm_expr->X_add_number);
		  goto rvc_imm_done;
		case 'K':
		  if (my_getSmallExpression (imm_expr, imm_reloc, asarg, p)
		      || imm_expr->X_op != O_constant
		      || imm_expr->X_add_number == 0
		      || !VALID_CIWTYPE_ADDI4SPN_IMM ((valueT) imm_expr->X_add_number))
		    break;
		  ip->insn_opcode |=
		    ENCODE_CIWTYPE_ADDI4SPN_IMM (imm_expr->X_add_number);
		  goto rvc_imm_done;
		case 'L':
		  if (my_getSmallExpression (imm_expr, imm_reloc, asarg, p)
		      || imm_expr->X_op != O_constant
		      || !VALID_CITYPE_ADDI16SP_IMM ((valueT) imm_expr->X_add_number))
		    break;
		  ip->insn_opcode |=
		    ENCODE_CITYPE_ADDI16SP_IMM (imm_expr->X_add_number);
		  goto rvc_imm_done;
		case 'M':
		  if (riscv_handle_implicit_zero_offset (imm_expr, asarg))
		    continue;
		  if (my_getSmallExpression (imm_expr, imm_reloc, asarg, p)
		      || imm_expr->X_op != O_constant
		      || !VALID_CSSTYPE_SWSP_IMM ((valueT) imm_expr->X_add_number))
		    break;
		  ip->insn_opcode |=
		    ENCODE_CSSTYPE_SWSP_IMM (imm_expr->X_add_number);
		  goto rvc_imm_done;
		case 'N':
		  if (riscv_handle_implicit_zero_offset (imm_expr, asarg))
		    continue;
		  if (my_getSmallExpression (imm_expr, imm_reloc, asarg, p)
		      || imm_expr->X_op != O_constant
		      || !VALID_CSSTYPE_SDSP_IMM ((valueT) imm_expr->X_add_number))
		    break;
		  ip->insn_opcode |=
		    ENCODE_CSSTYPE_SDSP_IMM (imm_expr->X_add_number);
		  goto rvc_imm_done;
		case 'u':
		  p = percent_op_utype;
		  if (my_getSmallExpression (imm_expr, imm_reloc, asarg, p))
		    break;
		rvc_lui:
		  if (imm_expr->X_op != O_constant
		      || imm_expr->X_add_number <= 0
		      || imm_expr->X_add_number >= RISCV_BIGIMM_REACH
		      || (imm_expr->X_add_number >= RISCV_RVC_IMM_REACH / 2
			  && (imm_expr->X_add_number <
			      RISCV_BIGIMM_REACH - RISCV_RVC_IMM_REACH / 2)))
		    break;
		  ip->insn_opcode |= ENCODE_CITYPE_IMM (imm_expr->X_add_number);
		  goto rvc_imm_done;
		case 'v':
		  if (my_getSmallExpression (imm_expr, imm_reloc, asarg, p)
		      || (imm_expr->X_add_number & (RISCV_IMM_REACH - 1))
		      || ((int32_t)imm_expr->X_add_number
			  != imm_expr->X_add_number))
		    break;
		  imm_expr->X_add_number =
		    ((uint32_t) imm_expr->X_add_number) >> RISCV_IMM_BITS;
		  goto rvc_lui;
		case 'p':
		  goto branch;
		case 'a':
		  goto jump;
		case 'S': /* Floating-point RS1 x8-x15.  */
		  if (!reg_lookup (&asarg, RCLASS_FPR, &regno)
		      || !(regno >= 8 && regno <= 15))
		    break;
		  INSERT_OPERAND (CRS1S, *ip, regno % 8);
		  continue;
		case 'D': /* Floating-point RS2 x8-x15.  */
		  if (!reg_lookup (&asarg, RCLASS_FPR, &regno)
		      || !(regno >= 8 && regno <= 15))
		    break;
		  INSERT_OPERAND (CRS2S, *ip, regno % 8);
		  continue;
		case 'T': /* Floating-point RS2.  */
		  if (!reg_lookup (&asarg, RCLASS_FPR, &regno))
		    break;
		  INSERT_OPERAND (CRS2, *ip, regno);
		  continue;
		case 'F':
		  switch (*++oparg)
		    {
		      case '6':
		        if (my_getSmallExpression (imm_expr, imm_reloc, asarg, p)
			    || imm_expr->X_op != O_constant
			    || imm_expr->X_add_number < 0
			    || imm_expr->X_add_number >= 64)
			  {
			    as_bad (_("bad value for compressed funct6 "
				      "field, value must be 0...63"));
			    break;
			  }
			INSERT_OPERAND (CFUNCT6, *ip, imm_expr->X_add_number);
			imm_expr->X_op = O_absent;
			asarg = expr_parse_end;
			continue;

		      case '4':
		        if (my_getSmallExpression (imm_expr, imm_reloc, asarg, p)
			    || imm_expr->X_op != O_constant
			    || imm_expr->X_add_number < 0
			    || imm_expr->X_add_number >= 16)
			  {
			    as_bad (_("bad value for compressed funct4 "
				      "field, value must be 0...15"));
			    break;
			  }
			INSERT_OPERAND (CFUNCT4, *ip, imm_expr->X_add_number);
			imm_expr->X_op = O_absent;
			asarg = expr_parse_end;
			continue;

		      case '3':
			if (my_getSmallExpression (imm_expr, imm_reloc, asarg, p)
			    || imm_expr->X_op != O_constant
			    || imm_expr->X_add_number < 0
			    || imm_expr->X_add_number >= 8)
			  {
			    as_bad (_("bad value for compressed funct3 "
				      "field, value must be 0...7"));
			    break;
			  }
			INSERT_OPERAND (CFUNCT3, *ip, imm_expr->X_add_number);
			imm_expr->X_op = O_absent;
			asarg = expr_parse_end;
			continue;

		      case '2':
			if (my_getSmallExpression (imm_expr, imm_reloc, asarg, p)
			    || imm_expr->X_op != O_constant
			    || imm_expr->X_add_number < 0
			    || imm_expr->X_add_number >= 4)
			  {
			    as_bad (_("bad value for compressed funct2 "
				      "field, value must be 0...3"));
			    break;
			  }
			INSERT_OPERAND (CFUNCT2, *ip, imm_expr->X_add_number);
			imm_expr->X_op = O_absent;
			asarg = expr_parse_end;
			continue;

		      default:
			goto unknown_riscv_ip_operand;
		    }
		  break;

		default:
		  goto unknown_riscv_ip_operand;
		}
	      break; /* end RVC */

	    case 'V': /* RVV */
	      switch (*++oparg)
		{
		case 'd': /* VD */
		  if (!reg_lookup (&asarg, RCLASS_VECR, &regno))
		    break;
		  INSERT_OPERAND (VD, *ip, regno);
		  continue;

		case 'e': /* AMO VD */
		  if (reg_lookup (&asarg, RCLASS_GPR, &regno) && regno == 0)
		    INSERT_OPERAND (VWD, *ip, 0);
		  else if (reg_lookup (&asarg, RCLASS_VECR, &regno))
		    {
		      INSERT_OPERAND (VWD, *ip, 1);
		      INSERT_OPERAND (VD, *ip, regno);
		    }
		  else
		    break;
		  continue;

		case 'f': /* AMO VS3 */
		  if (!reg_lookup (&asarg, RCLASS_VECR, &regno))
		    break;
		  if (!EXTRACT_OPERAND (VWD, ip->insn_opcode))
		    INSERT_OPERAND (VD, *ip, regno);
		  else
		    {
		      /* VS3 must match VD.  */
		      if (EXTRACT_OPERAND (VD, ip->insn_opcode) != regno)
			break;
		    }
		  continue;

		case 's': /* VS1 */
		  if (!reg_lookup (&asarg, RCLASS_VECR, &regno))
		    break;
		  INSERT_OPERAND (VS1, *ip, regno);
		  continue;

		case 't': /* VS2 */
		  if (!reg_lookup (&asarg, RCLASS_VECR, &regno))
		    break;
		  INSERT_OPERAND (VS2, *ip, regno);
		  continue;

		case 'u': /* VS1 == VS2 */
		  if (!reg_lookup (&asarg, RCLASS_VECR, &regno))
		    break;
		  INSERT_OPERAND (VS1, *ip, regno);
		  INSERT_OPERAND (VS2, *ip, regno);
		  continue;

		case 'v': /* VD == VS1 == VS2 */
		  if (!reg_lookup (&asarg, RCLASS_VECR, &regno))
		    break;
		  INSERT_OPERAND (VD, *ip, regno);
		  INSERT_OPERAND (VS1, *ip, regno);
		  INSERT_OPERAND (VS2, *ip, regno);
		  continue;

		/* The `V0` is carry-in register for v[m]adc and v[m]sbc,
		   and is used to choose vs1/rs1/frs1/imm or vs2 for
		   v[f]merge.  It use the same encoding as the vector mask
		   register.  */
		case '0':
		  if (reg_lookup (&asarg, RCLASS_VECR, &regno) && regno == 0)
		    continue;
		  break;

		case 'b': /* vtypei for vsetivli */
		  my_getVsetvliExpression (imm_expr, asarg);
		  check_absolute_expr (ip, imm_expr, FALSE);
		  if (!VALID_RVV_VB_IMM (imm_expr->X_add_number))
		    as_bad (_("bad value for vsetivli immediate field, "
			      "value must be 0..1023"));
		  ip->insn_opcode
		    |= ENCODE_RVV_VB_IMM (imm_expr->X_add_number);
		  imm_expr->X_op = O_absent;
		  asarg = expr_parse_end;
		  continue;

		case 'c': /* vtypei for vsetvli */
		  my_getVsetvliExpression (imm_expr, asarg);
		  check_absolute_expr (ip, imm_expr, FALSE);
		  if (!VALID_RVV_VC_IMM (imm_expr->X_add_number))
		    as_bad (_("bad value for vsetvli immediate field, "
			      "value must be 0..2047"));
		  ip->insn_opcode
		    |= ENCODE_RVV_VC_IMM (imm_expr->X_add_number);
		  imm_expr->X_op = O_absent;
		  asarg = expr_parse_end;
		  continue;

		case 'i': /* vector arith signed immediate */
		  my_getExpression (imm_expr, asarg);
		  check_absolute_expr (ip, imm_expr, FALSE);
		  if (imm_expr->X_add_number > 15
		      || imm_expr->X_add_number < -16)
		    as_bad (_("bad value for vector immediate field, "
			      "value must be -16...15"));
		  INSERT_OPERAND (VIMM, *ip, imm_expr->X_add_number);
		  imm_expr->X_op = O_absent;
		  asarg = expr_parse_end;
		  continue;

		case 'j': /* vector arith unsigned immediate */
		  my_getExpression (imm_expr, asarg);
		  check_absolute_expr (ip, imm_expr, FALSE);
		  if (imm_expr->X_add_number < 0
		      || imm_expr->X_add_number >= 32)
		    as_bad (_("bad value for vector immediate field, "
			      "value must be 0...31"));
		  INSERT_OPERAND (VIMM, *ip, imm_expr->X_add_number);
		  imm_expr->X_op = O_absent;
		  asarg = expr_parse_end;
		  continue;

		case 'k': /* vector arith signed immediate, minus 1 */
		  my_getExpression (imm_expr, asarg);
		  check_absolute_expr (ip, imm_expr, FALSE);
		  if (imm_expr->X_add_number > 16
		      || imm_expr->X_add_number < -15)
		    as_bad (_("bad value for vector immediate field, "
			      "value must be -15...16"));
		  INSERT_OPERAND (VIMM, *ip, imm_expr->X_add_number - 1);
		  imm_expr->X_op = O_absent;
		  asarg = expr_parse_end;
		  continue;

		case 'l': /* 6-bit vector arith unsigned immediate */
		  my_getExpression (imm_expr, asarg);
		  check_absolute_expr (ip, imm_expr, FALSE);
		  if (imm_expr->X_add_number < 0
		      || imm_expr->X_add_number >= 64)
		    as_bad (_("bad value for vector immediate field, "
			      "value must be 0...63"));
		  ip->insn_opcode |= ENCODE_RVV_VI_UIMM6 (imm_expr->X_add_number);
		  imm_expr->X_op = O_absent;
		  asarg = expr_parse_end;
		  continue;

		case 'm': /* optional vector mask */
		  if (*asarg == '\0')
		    {
		      INSERT_OPERAND (VMASK, *ip, 1);
		      continue;
		    }
		  else if (*asarg == ',' && asarg++
			   && reg_lookup (&asarg, RCLASS_VECM, &regno)
			   && regno == 0)
		    {
		      INSERT_OPERAND (VMASK, *ip, 0);
		      continue;
		    }
		  break;

		case 'M': /* required vector mask */
		  if (reg_lookup (&asarg, RCLASS_VECM, &regno) && regno == 0)
		    {
		      INSERT_OPERAND (VMASK, *ip, 0);
		      continue;
		    }
		  break;

		case 'T': /* vector macro temporary register */
		  if (!reg_lookup (&asarg, RCLASS_VECR, &regno) || regno == 0)
		    break;
		  /* Store it in the FUNCT6 field as we don't have anyplace
		     else to store it.  */
		  INSERT_OPERAND (VFUNCT6, *ip, regno);
		  continue;

		default:
		  goto unknown_riscv_ip_operand;
		}
	      break; /* end RVV */

	    case ',':
	      if (*asarg++ == *oparg)
		continue;
	      asarg--;
	      break;

	    case '(':
	    case ')':
	    case '[':
	    case ']':
	      if (*asarg++ == *oparg)
		continue;
	      break;

	    case '<': /* Shift amount, 0 - 31.  */
	      my_getExpression (imm_expr, asarg);
	      check_absolute_expr (ip, imm_expr, false);
	      if ((unsigned long) imm_expr->X_add_number > 31)
		as_bad (_("improper shift amount (%"PRIu64")"),
			imm_expr->X_add_number);
	      INSERT_OPERAND (SHAMTW, *ip, imm_expr->X_add_number);
	      imm_expr->X_op = O_absent;
	      asarg = expr_parse_end;
	      continue;

	    case '>': /* Shift amount, 0 - (XLEN-1).  */
	      my_getExpression (imm_expr, asarg);
	      check_absolute_expr (ip, imm_expr, false);
	      if ((unsigned long) imm_expr->X_add_number >= xlen)
		as_bad (_("improper shift amount (%"PRIu64")"),
			imm_expr->X_add_number);
	      INSERT_OPERAND (SHAMT, *ip, imm_expr->X_add_number);
	      imm_expr->X_op = O_absent;
	      asarg = expr_parse_end;
	      continue;

	    case 'Z': /* CSRRxI immediate.  */
	      my_getExpression (imm_expr, asarg);
	      check_absolute_expr (ip, imm_expr, false);
	      if ((unsigned long) imm_expr->X_add_number > 31)
		as_bad (_("improper CSRxI immediate (%"PRIu64")"),
			imm_expr->X_add_number);
	      INSERT_OPERAND (RS1, *ip, imm_expr->X_add_number);
	      imm_expr->X_op = O_absent;
	      asarg = expr_parse_end;
	      continue;

	    case 'E': /* Control register.  */
	      insn_with_csr = true;
	      explicit_priv_attr = true;
	      if (reg_lookup (&asarg, RCLASS_CSR, &regno))
		INSERT_OPERAND (CSR, *ip, regno);
	      else
		{
		  my_getExpression (imm_expr, asarg);
		  check_absolute_expr (ip, imm_expr, true);
		  if ((unsigned long) imm_expr->X_add_number > 0xfff)
		    as_bad (_("improper CSR address (%"PRIu64")"),
			    imm_expr->X_add_number);
		  INSERT_OPERAND (CSR, *ip, imm_expr->X_add_number);
		  imm_expr->X_op = O_absent;
		  asarg = expr_parse_end;
		}
	      continue;

	    case 'm': /* Rounding mode.  */
	      if (arg_lookup (&asarg, riscv_rm,
			      ARRAY_SIZE (riscv_rm), &regno))
		{
		  INSERT_OPERAND (RM, *ip, regno);
		  continue;
		}
	      break;

	    case 'P':
	    case 'Q': /* Fence predecessor/successor.  */
	      if (arg_lookup (&asarg, riscv_pred_succ,
			      ARRAY_SIZE (riscv_pred_succ), &regno))
		{
		  if (*oparg == 'P')
		    INSERT_OPERAND (PRED, *ip, regno);
		  else
		    INSERT_OPERAND (SUCC, *ip, regno);
		  continue;
		}
	      break;

	    case 'd': /* Destination register.  */
	    case 's': /* Source register.  */
	    case 't': /* Target register.  */
	    case 'r': /* RS3 */
	      if (reg_lookup (&asarg, RCLASS_GPR, &regno))
		{
		  char c = *oparg;
		  if (*asarg == ' ')
		    ++asarg;

		  /* Now that we have assembled one operand, we use the args
		     string to figure out where it goes in the instruction.  */
		  switch (c)
		    {
		    case 's':
		      INSERT_OPERAND (RS1, *ip, regno);
		      break;
		    case 'd':
		      INSERT_OPERAND (RD, *ip, regno);
		      break;
		    case 't':
		      INSERT_OPERAND (RS2, *ip, regno);
		      break;
		    case 'r':
		      INSERT_OPERAND (RS3, *ip, regno);
		      break;
		    }
		  continue;
		}
	      break;

	    case 'D': /* Floating point RD.  */
	    case 'S': /* Floating point RS1.  */
	    case 'T': /* Floating point RS2.  */
	    case 'U': /* Floating point RS1 and RS2.  */
	    case 'R': /* Floating point RS3.  */
	      if (reg_lookup (&asarg,
			      (riscv_subset_supports (&riscv_rps_as, "zfinx")
			      ? RCLASS_GPR : RCLASS_FPR), &regno))
		{
		  char c = *oparg;
		  if (*asarg == ' ')
		    ++asarg;
		  switch (c)
		    {
		    case 'D':
		      INSERT_OPERAND (RD, *ip, regno);
		      break;
		    case 'S':
		      INSERT_OPERAND (RS1, *ip, regno);
		      break;
		    case 'U':
		      INSERT_OPERAND (RS1, *ip, regno);
		      /* Fall through.  */
		    case 'T':
		      INSERT_OPERAND (RS2, *ip, regno);
		      break;
		    case 'R':
		      INSERT_OPERAND (RS3, *ip, regno);
		      break;
		    }
		  continue;
		}
	      break;

	    case 'I':
	      my_getExpression (imm_expr, asarg);
	      if (imm_expr->X_op != O_big
		  && imm_expr->X_op != O_constant)
		break;
	      normalize_constant_expr (imm_expr);
	      asarg = expr_parse_end;
	      continue;

	    case 'A':
	      my_getExpression (imm_expr, asarg);
	      normalize_constant_expr (imm_expr);
	      /* The 'A' format specifier must be a symbol.  */
	      if (imm_expr->X_op != O_symbol)
	        break;
	      *imm_reloc = BFD_RELOC_32;
	      asarg = expr_parse_end;
	      continue;

	    case 'B':
	      my_getExpression (imm_expr, asarg);
	      normalize_constant_expr (imm_expr);
	      /* The 'B' format specifier must be a symbol or a constant.  */
	      if (imm_expr->X_op != O_symbol && imm_expr->X_op != O_constant)
	        break;
	      if (imm_expr->X_op == O_symbol)
	        *imm_reloc = BFD_RELOC_32;
	      asarg = expr_parse_end;
	      continue;

	    case 'j': /* Sign-extended immediate.  */
	      p = percent_op_itype;
	      *imm_reloc = BFD_RELOC_RISCV_LO12_I;
	      goto alu_op;
	    case 'q': /* Store displacement.  */
	      p = percent_op_stype;
	      *imm_reloc = BFD_RELOC_RISCV_LO12_S;
	      goto load_store;
	    case 'o': /* Load displacement.  */
	      p = percent_op_itype;
	      *imm_reloc = BFD_RELOC_RISCV_LO12_I;
	      goto load_store;
	    case '1':
	      /* This is used for TLS, where the fourth operand is
		 %tprel_add, to get a relocation applied to an add
		 instruction, for relaxation to use.  */
	      p = percent_op_rtype;
	      goto alu_op;
	    case '0': /* AMO displacement, which must be zero.  */
	    load_store:
	      if (riscv_handle_implicit_zero_offset (imm_expr, asarg))
		continue;
	    alu_op:
	      /* If this value won't fit into a 16 bit offset, then go
		 find a macro that will generate the 32 bit offset
		 code pattern.  */
	      if (!my_getSmallExpression (imm_expr, imm_reloc, asarg, p))
		{
		  normalize_constant_expr (imm_expr);
		  if (imm_expr->X_op != O_constant
		      || (*oparg == '0' && imm_expr->X_add_number != 0)
		      || (*oparg == '1')
		      || imm_expr->X_add_number >= (signed)RISCV_IMM_REACH/2
		      || imm_expr->X_add_number < -(signed)RISCV_IMM_REACH/2)
		    break;
		}
	      asarg = expr_parse_end;
	      continue;

	    case 'p': /* PC-relative offset.  */
	    branch:
	      *imm_reloc = BFD_RELOC_12_PCREL;
	      my_getExpression (imm_expr, asarg);
	      asarg = expr_parse_end;
	      continue;

	    case 'u': /* Upper 20 bits.  */
	      p = percent_op_utype;
	      if (!my_getSmallExpression (imm_expr, imm_reloc, asarg, p))
		{
		  if (imm_expr->X_op != O_constant)
		    break;

		  if (imm_expr->X_add_number < 0
		      || imm_expr->X_add_number >= (signed)RISCV_BIGIMM_REACH)
		    as_bad (_("lui expression not in range 0..1048575"));

		  *imm_reloc = BFD_RELOC_RISCV_HI20;
		  imm_expr->X_add_number <<= RISCV_IMM_BITS;
		}
	      asarg = expr_parse_end;
	      continue;

	    case 'a': /* 20-bit PC-relative offset.  */
	    jump:
	      my_getExpression (imm_expr, asarg);
	      asarg = expr_parse_end;
	      *imm_reloc = BFD_RELOC_RISCV_JMP;
	      continue;

	    case 'c':
	      my_getExpression (imm_expr, asarg);
	      asarg = expr_parse_end;
	      if (strcmp (asarg, "@plt") == 0)
		asarg += 4;
	      *imm_reloc = BFD_RELOC_RISCV_CALL_PLT;
	      continue;

	    case 'O':
	      switch (*++oparg)
		{
		case '4':
		  if (my_getOpcodeExpression (imm_expr, imm_reloc, asarg)
		      || imm_expr->X_op != O_constant
		      || imm_expr->X_add_number < 0
		      || imm_expr->X_add_number >= 128
		      || (imm_expr->X_add_number & 0x3) != 3)
		    {
		      as_bad (_("bad value for opcode field, "
				"value must be 0...127 and "
				"lower 2 bits must be 0x3"));
		      break;
		    }
		  INSERT_OPERAND (OP, *ip, imm_expr->X_add_number);
		  imm_expr->X_op = O_absent;
		  asarg = expr_parse_end;
		  continue;

		case '2':
		  if (my_getOpcodeExpression (imm_expr, imm_reloc, asarg)
		      || imm_expr->X_op != O_constant
		      || imm_expr->X_add_number < 0
		      || imm_expr->X_add_number >= 3)
		    {
		      as_bad (_("bad value for opcode field, "
				"value must be 0...2"));
		      break;
		    }
		  INSERT_OPERAND (OP2, *ip, imm_expr->X_add_number);
		  imm_expr->X_op = O_absent;
		  asarg = expr_parse_end;
		  continue;

		default:
		  goto unknown_riscv_ip_operand;
		}
	      break;

	    case 'F':
	      switch (*++oparg)
		{
		case '7':
		  if (my_getSmallExpression (imm_expr, imm_reloc, asarg, p)
		      || imm_expr->X_op != O_constant
		      || imm_expr->X_add_number < 0
		      || imm_expr->X_add_number >= 128)
		    {
		      as_bad (_("bad value for funct7 field, "
				"value must be 0...127"));
		      break;
		    }
		  INSERT_OPERAND (FUNCT7, *ip, imm_expr->X_add_number);
		  imm_expr->X_op = O_absent;
		  asarg = expr_parse_end;
		  continue;

		case '3':
		  if (my_getSmallExpression (imm_expr, imm_reloc, asarg, p)
		      || imm_expr->X_op != O_constant
		      || imm_expr->X_add_number < 0
		      || imm_expr->X_add_number >= 8)
		    {
		      as_bad (_("bad value for funct3 field, "
			        "value must be 0...7"));
		      break;
		    }
		  INSERT_OPERAND (FUNCT3, *ip, imm_expr->X_add_number);
		  imm_expr->X_op = O_absent;
		  asarg = expr_parse_end;
		  continue;

		case '2':
		  if (my_getSmallExpression (imm_expr, imm_reloc, asarg, p)
		      || imm_expr->X_op != O_constant
		      || imm_expr->X_add_number < 0
		      || imm_expr->X_add_number >= 4)
		    {
		      as_bad (_("bad value for funct2 field, "
			        "value must be 0...3"));
		      break;
		    }
		  INSERT_OPERAND (FUNCT2, *ip, imm_expr->X_add_number);
		  imm_expr->X_op = O_absent;
		  asarg = expr_parse_end;
		  continue;

		default:
		  goto unknown_riscv_ip_operand;
		}
	      break;

	    case 'y': /* bs immediate */
	      my_getExpression (imm_expr, asarg);
	      check_absolute_expr (ip, imm_expr, FALSE);
	      if ((unsigned long)imm_expr->X_add_number > 3)
		as_bad(_("Improper bs immediate (%lu)"),
		       (unsigned long)imm_expr->X_add_number);
	      INSERT_OPERAND(BS, *ip, imm_expr->X_add_number);
	      imm_expr->X_op = O_absent;
	      asarg = expr_parse_end;
	      continue;

	    case 'Y': /* rnum immediate */
	      my_getExpression (imm_expr, asarg);
	      check_absolute_expr (ip, imm_expr, FALSE);
	      if ((unsigned long)imm_expr->X_add_number > 10)
		as_bad(_("Improper rnum immediate (%lu)"),
		       (unsigned long)imm_expr->X_add_number);
	      INSERT_OPERAND(RNUM, *ip, imm_expr->X_add_number);
	      imm_expr->X_op = O_absent;
	      asarg = expr_parse_end;
	      continue;

	    case 'z':
	      if (my_getSmallExpression (imm_expr, imm_reloc, asarg, p)
		  || imm_expr->X_op != O_constant
		  || imm_expr->X_add_number != 0)
		break;
	      asarg = expr_parse_end;
	      imm_expr->X_op = O_absent;
	      continue;

	    case 'W': /* Various operands.  */
	      switch (*++oparg)
		{
		case 'i':
		  switch (*++oparg)
		    {
		    case 'f':
		      /* Prefetch offset for 'Zicbop' extension.
			 pseudo S-type but lower 5-bits zero.  */
		      if (riscv_handle_implicit_zero_offset (imm_expr, asarg))
			continue;
		      my_getExpression (imm_expr, asarg);
		      check_absolute_expr (ip, imm_expr, false);
		      if (((unsigned) (imm_expr->X_add_number) & 0x1fU)
			  || imm_expr->X_add_number >= RISCV_IMM_REACH / 2
			  || imm_expr->X_add_number < -RISCV_IMM_REACH / 2)
			as_bad (_ ("improper prefetch offset (%ld)"),
				(long) imm_expr->X_add_number);
		      ip->insn_opcode |= ENCODE_STYPE_IMM (
			  (unsigned) (imm_expr->X_add_number) & ~0x1fU);
		      imm_expr->X_op = O_absent;
		      asarg = expr_parse_end;
		      continue;
		    default:
		      goto unknown_riscv_ip_operand;
		    }
		  break;
		case 'f':
		  switch (*++oparg)
		    {
		    case 'v':
		      /* FLI.[HSDQ] value field for 'Zfa' extension.  */
		      if (!arg_lookup (&asarg, riscv_fli_symval,
				       ARRAY_SIZE (riscv_fli_symval), &regno))
			{
			  /* 0.0 is not a valid entry in riscv_fli_numval.  */
			  errno = 0;
			  float f = strtof (asarg, &asarg);
			  if (errno != 0 || f == 0.0
			      || !flt_lookup (f, riscv_fli_numval,
					     ARRAY_SIZE(riscv_fli_numval),
					     &regno))
			    {
			      as_bad (_("bad fli constant operand, "
					"supported constants must be in "
					"decimal or hexadecimal floating-point "
					"literal form"));
			      break;
			    }
			}
		      INSERT_OPERAND (RS1, *ip, regno);
		      continue;
		    default:
		      goto unknown_riscv_ip_operand;
		    }
		  break;
		default:
		  goto unknown_riscv_ip_operand;
		}
	      break;

	    case 'X': /* Integer immediate.  */
	      {
		size_t n;
		size_t s;
		bool sign;

		switch (*++oparg)
		  {
		    case 'l': /* Literal.  */
		      n = strcspn (++oparg, ",");
		      if (strncmp (oparg, asarg, n))
			as_bad (_("unexpected literal (%s)"), asarg);
		      oparg += n - 1;
		      asarg += n;
		      continue;
		    case 's': /* 'XsN@S' ... N-bit signed immediate at bit S.  */
		      sign = true;
		      goto parse_imm;
		    case 'u': /* 'XuN@S' ... N-bit unsigned immediate at bit S.  */
		      sign = false;
		      goto parse_imm;
		    parse_imm:
		      n = strtol (oparg + 1, (char **)&oparg, 10);
		      if (*oparg != '@')
			goto unknown_riscv_ip_operand;
		      s = strtol (oparg + 1, (char **)&oparg, 10);
		      oparg--;

		      my_getExpression (imm_expr, asarg);
		      check_absolute_expr (ip, imm_expr, false);
		      if (!sign)
			{
			  if (!VALIDATE_U_IMM (imm_expr->X_add_number, n))
			    as_bad (_("improper immediate value (%"PRIu64")"),
				    imm_expr->X_add_number);
			}
		      else
			{
			  if (!VALIDATE_S_IMM (imm_expr->X_add_number, n))
			    as_bad (_("improper immediate value (%"PRIi64")"),
				    imm_expr->X_add_number);
			}
		      INSERT_IMM (n, s, *ip, imm_expr->X_add_number);
		      imm_expr->X_op = O_absent;
		      asarg = expr_parse_end;
		      continue;
		    default:
		      goto unknown_riscv_ip_operand;
		  }
	      }
	      break;

	    default:
	    unknown_riscv_ip_operand:
	      as_fatal (_("internal: unknown argument type `%s'"),
			opargStart);
	    }
	  break;
	}
      asarg = asargStart;
      insn_with_csr = false;
    }

 out:
  /* Restore the character we might have clobbered above.  */
  if (save_c)
    *(asargStart  - 1) = save_c;

  probing_insn_operands = false;

  return error;
}

/* Similar to riscv_ip, but assembles an instruction according to the
   hardcode values of .insn directive.  */

static const char *
riscv_ip_hardcode (char *str,
		   struct riscv_cl_insn *ip,
		   expressionS *imm_expr,
		   const char *error)
{
  struct riscv_opcode *insn;
  insn_t values[2] = {0, 0};
  unsigned int num = 0;

  input_line_pointer = str;
  do
    {
      expression (imm_expr);
      switch (imm_expr->X_op)
	{
	case O_constant:
	  values[num++] = (insn_t) imm_expr->X_add_number;
	  break;
	case O_big:
	  /* Extract lower 32-bits of a big number.
	     Assume that generic_bignum_to_int32 work on such number.  */
	  values[num++] = (insn_t) generic_bignum_to_int32 ();
	  break;
	default:
	  /* The first value isn't constant, so it should be
	     .insn <type> <operands>.  We have been parsed it
	     in the riscv_ip.  */
	  if (num == 0)
	    return error;
	  return _("values must be constant");
	}
    }
  while (*input_line_pointer++ == ',' && num < 2 && imm_expr->X_op != O_big);

  input_line_pointer--;
  if (*input_line_pointer != '\0')
    return _("unrecognized values");

  insn = XNEW (struct riscv_opcode);
  insn->match = values[num - 1];
  create_insn (ip, insn);
  unsigned int bytes = riscv_insn_length (insn->match);

  if (num == 2 && values[0] != bytes)
    return _("value conflicts with instruction length");

  if (imm_expr->X_op == O_big)
    {
      unsigned int llen = 0;
      for (LITTLENUM_TYPE lval = generic_bignum[imm_expr->X_add_number - 1];
	   lval != 0; llen++)
	lval >>= BITS_PER_CHAR;
      unsigned int repr_bytes
	  = (imm_expr->X_add_number - 1) * CHARS_PER_LITTLENUM + llen;
      if (bytes < repr_bytes)
	return _("value conflicts with instruction length");
      for (num = 0; num < imm_expr->X_add_number - 1; ++num)
	number_to_chars_littleendian (
	    ip->insn_long_opcode + num * CHARS_PER_LITTLENUM,
	    generic_bignum[num],
	    CHARS_PER_LITTLENUM);
      if (llen != 0)
	number_to_chars_littleendian (
	    ip->insn_long_opcode + num * CHARS_PER_LITTLENUM,
	    generic_bignum[num],
	    llen);
      memset(ip->insn_long_opcode + repr_bytes, 0, bytes - repr_bytes);
      return NULL;
    }

  if (bytes < sizeof(values[0]) && values[num - 1] >> (8 * bytes) != 0)
    return _("value conflicts with instruction length");

  return NULL;
}

void
md_assemble (char *str)
{
  struct riscv_cl_insn insn;
  expressionS imm_expr;
  bfd_reloc_code_real_type imm_reloc = BFD_RELOC_UNUSED;

  /* The architecture and privileged elf attributes should be set
     before assembling.  */
  if (!start_assemble)
    {
      start_assemble = true;

      riscv_set_abi_by_arch ();
      if (!riscv_set_default_priv_spec (NULL))
       return;
    }

  riscv_mapping_state (MAP_INSN, 0, false/* fr_align_code */);

  const struct riscv_ip_error error = riscv_ip (str, &insn, &imm_expr,
						&imm_reloc, op_hash);

  if (error.msg)
    {
      if (error.missing_ext)
	as_bad ("%s `%s', extension `%s' required", error.msg,
		error.statement, error.missing_ext);
      else
	as_bad ("%s `%s'", error.msg, error.statement);
      return;
    }

  if (insn.insn_mo->pinfo == INSN_MACRO)
    macro (&insn, &imm_expr, &imm_reloc);
  else
    append_insn (&insn, &imm_expr, imm_reloc);
}

const char *
md_atof (int type, char *litP, int *sizeP)
{
  return ieee_md_atof (type, litP, sizeP, target_big_endian);
}

void
md_number_to_chars (char *buf, valueT val, int n)
{
  if (target_big_endian)
    number_to_chars_bigendian (buf, val, n);
  else
    number_to_chars_littleendian (buf, val, n);
}

const char *md_shortopts = "O::g::G:";

enum options
{
  OPTION_MARCH = OPTION_MD_BASE,
  OPTION_PIC,
  OPTION_NO_PIC,
  OPTION_MABI,
  OPTION_RELAX,
  OPTION_NO_RELAX,
  OPTION_ARCH_ATTR,
  OPTION_NO_ARCH_ATTR,
  OPTION_CSR_CHECK,
  OPTION_NO_CSR_CHECK,
  OPTION_MISA_SPEC,
  OPTION_MPRIV_SPEC,
  OPTION_BIG_ENDIAN,
  OPTION_LITTLE_ENDIAN,
  OPTION_END_OF_ENUM
};

struct option md_longopts[] =
{
  {"march", required_argument, NULL, OPTION_MARCH},
  {"fPIC", no_argument, NULL, OPTION_PIC},
  {"fpic", no_argument, NULL, OPTION_PIC},
  {"fno-pic", no_argument, NULL, OPTION_NO_PIC},
  {"mabi", required_argument, NULL, OPTION_MABI},
  {"mrelax", no_argument, NULL, OPTION_RELAX},
  {"mno-relax", no_argument, NULL, OPTION_NO_RELAX},
  {"march-attr", no_argument, NULL, OPTION_ARCH_ATTR},
  {"mno-arch-attr", no_argument, NULL, OPTION_NO_ARCH_ATTR},
  {"mcsr-check", no_argument, NULL, OPTION_CSR_CHECK},
  {"mno-csr-check", no_argument, NULL, OPTION_NO_CSR_CHECK},
  {"misa-spec", required_argument, NULL, OPTION_MISA_SPEC},
  {"mpriv-spec", required_argument, NULL, OPTION_MPRIV_SPEC},
  {"mbig-endian", no_argument, NULL, OPTION_BIG_ENDIAN},
  {"mlittle-endian", no_argument, NULL, OPTION_LITTLE_ENDIAN},

  {NULL, no_argument, NULL, 0}
};
size_t md_longopts_size = sizeof (md_longopts);

int
md_parse_option (int c, const char *arg)
{
  switch (c)
    {
    case OPTION_MARCH:
      default_arch_with_ext = arg;
      break;

    case OPTION_NO_PIC:
      riscv_opts.pic = false;
      break;

    case OPTION_PIC:
      riscv_opts.pic = true;
      break;

    case OPTION_MABI:
      if (strcmp (arg, "ilp32") == 0)
	riscv_set_abi (32, FLOAT_ABI_SOFT, false);
      else if (strcmp (arg, "ilp32e") == 0)
	riscv_set_abi (32, FLOAT_ABI_SOFT, true);
      else if (strcmp (arg, "ilp32f") == 0)
	riscv_set_abi (32, FLOAT_ABI_SINGLE, false);
      else if (strcmp (arg, "ilp32d") == 0)
	riscv_set_abi (32, FLOAT_ABI_DOUBLE, false);
      else if (strcmp (arg, "ilp32q") == 0)
	riscv_set_abi (32, FLOAT_ABI_QUAD, false);
      else if (strcmp (arg, "lp64") == 0)
	riscv_set_abi (64, FLOAT_ABI_SOFT, false);
      else if (strcmp (arg, "lp64f") == 0)
	riscv_set_abi (64, FLOAT_ABI_SINGLE, false);
      else if (strcmp (arg, "lp64d") == 0)
	riscv_set_abi (64, FLOAT_ABI_DOUBLE, false);
      else if (strcmp (arg, "lp64q") == 0)
	riscv_set_abi (64, FLOAT_ABI_QUAD, false);
      else
	return 0;
      explicit_mabi = true;
      break;

    case OPTION_RELAX:
      riscv_opts.relax = true;
      break;

    case OPTION_NO_RELAX:
      riscv_opts.relax = false;
      break;

    case OPTION_ARCH_ATTR:
      riscv_opts.arch_attr = true;
      break;

    case OPTION_NO_ARCH_ATTR:
      riscv_opts.arch_attr = false;
      break;

    case OPTION_CSR_CHECK:
      riscv_opts.csr_check = true;
      break;

    case OPTION_NO_CSR_CHECK:
      riscv_opts.csr_check = false;
      break;

    case OPTION_MISA_SPEC:
      return riscv_set_default_isa_spec (arg);

    case OPTION_MPRIV_SPEC:
      return riscv_set_default_priv_spec (arg);

    case OPTION_BIG_ENDIAN:
      target_big_endian = 1;
      break;

    case OPTION_LITTLE_ENDIAN:
      target_big_endian = 0;
      break;

    default:
      return 0;
    }

  return 1;
}

void
riscv_after_parse_args (void)
{
  /* The --with-arch is optional for now, so we still need to set the xlen
     according to the default_arch, which is set by the --target.  */
  if (xlen == 0)
    {
      if (strcmp (default_arch, "riscv32") == 0)
	xlen = 32;
      else if (strcmp (default_arch, "riscv64") == 0)
	xlen = 64;
      else
	as_bad ("unknown default architecture `%s'", default_arch);
    }

  /* Set default specs.  */
  if (default_isa_spec == ISA_SPEC_CLASS_NONE)
    riscv_set_default_isa_spec (DEFAULT_RISCV_ISA_SPEC);
  if (default_priv_spec == PRIV_SPEC_CLASS_NONE)
    riscv_set_default_priv_spec (DEFAULT_RISCV_PRIV_SPEC);

  riscv_set_arch (default_arch_with_ext);

  /* If the CIE to be produced has not been overridden on the command line,
     then produce version 3 by default.  This allows us to use the full
     range of registers in a .cfi_return_column directive.  */
  if (flag_dwarf_cie_version == -1)
    flag_dwarf_cie_version = 3;
}

bool riscv_parse_name (const char *name, struct expressionS *ep,
		       enum expr_mode mode)
{
  unsigned int regno;
  symbolS *sym;

  if (!probing_insn_operands)
    return false;

  gas_assert (mode == expr_normal);

  regno = reg_lookup_internal (name, RCLASS_GPR);
  if (regno == (unsigned int)-1)
    return false;

  if (symbol_find (name) != NULL)
    return false;

  /* Create a symbol without adding it to the symbol table yet.
     Insertion will happen only once we commit to using the insn
     we're probing operands for.  */
  for (sym = deferred_sym_rootP; sym; sym = symbol_next (sym))
    if (strcmp (name, S_GET_NAME (sym)) == 0)
      break;
  if (!sym)
    {
      for (sym = orphan_sym_rootP; sym; sym = symbol_next (sym))
	if (strcmp (name, S_GET_NAME (sym)) == 0)
	  {
	    symbol_remove (sym, &orphan_sym_rootP, &orphan_sym_lastP);
	    break;
	  }
      if (!sym)
	sym = symbol_create (name, undefined_section,
			     &zero_address_frag, 0);

      symbol_append (sym, deferred_sym_lastP, &deferred_sym_rootP,
		     &deferred_sym_lastP);
    }

  ep->X_op = O_symbol;
  ep->X_add_symbol = sym;
  ep->X_add_number = 0;

  return true;
}

long
md_pcrel_from (fixS *fixP)
{
  return fixP->fx_where + fixP->fx_frag->fr_address;
}

/* Apply a fixup to the object file.  */

void
md_apply_fix (fixS *fixP, valueT *valP, segT seg ATTRIBUTE_UNUSED)
{
  unsigned int subtype;
  bfd_byte *buf = (bfd_byte *) (fixP->fx_frag->fr_literal + fixP->fx_where);
  bool relaxable = false;
  offsetT loc;
  segT sub_segment;

  /* Remember value for tc_gen_reloc.  */
  fixP->fx_addnumber = *valP;

  switch (fixP->fx_r_type)
    {
    case BFD_RELOC_RISCV_HI20:
    case BFD_RELOC_RISCV_LO12_I:
    case BFD_RELOC_RISCV_LO12_S:
      bfd_putl32 (riscv_apply_const_reloc (fixP->fx_r_type, *valP)
		  | bfd_getl32 (buf), buf);
      if (fixP->fx_addsy == NULL)
	fixP->fx_done = true;
      relaxable = true;
      break;

    case BFD_RELOC_RISCV_GOT_HI20:
    case BFD_RELOC_RISCV_ADD8:
    case BFD_RELOC_RISCV_ADD16:
    case BFD_RELOC_RISCV_ADD32:
    case BFD_RELOC_RISCV_ADD64:
    case BFD_RELOC_RISCV_SUB6:
    case BFD_RELOC_RISCV_SUB8:
    case BFD_RELOC_RISCV_SUB16:
    case BFD_RELOC_RISCV_SUB32:
    case BFD_RELOC_RISCV_SUB64:
    case BFD_RELOC_RISCV_RELAX:
    /* cvt_frag_to_fill () has called output_leb128 ().  */
    case BFD_RELOC_RISCV_SET_ULEB128:
    case BFD_RELOC_RISCV_SUB_ULEB128:
      break;

    case BFD_RELOC_RISCV_TPREL_HI20:
    case BFD_RELOC_RISCV_TPREL_LO12_I:
    case BFD_RELOC_RISCV_TPREL_LO12_S:
    case BFD_RELOC_RISCV_TPREL_ADD:
      relaxable = true;
      /* Fall through.  */

    case BFD_RELOC_RISCV_TLS_GOT_HI20:
    case BFD_RELOC_RISCV_TLS_GD_HI20:
    case BFD_RELOC_RISCV_TLS_DTPREL32:
    case BFD_RELOC_RISCV_TLS_DTPREL64:
      if (fixP->fx_addsy != NULL)
	S_SET_THREAD_LOCAL (fixP->fx_addsy);
      else
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("TLS relocation against a constant"));
      break;

    case BFD_RELOC_32:
      /* Use pc-relative relocation for FDE initial location.
	 The symbol address in .eh_frame may be adjusted in
	 _bfd_elf_discard_section_eh_frame, and the content of
	 .eh_frame will be adjusted in _bfd_elf_write_section_eh_frame.
	 Therefore, we cannot insert a relocation whose addend symbol is
	 in .eh_frame.  Othrewise, the value may be adjusted twice.  */
      if (fixP->fx_addsy && fixP->fx_subsy
	  && (sub_segment = S_GET_SEGMENT (fixP->fx_subsy))
	  && strcmp (sub_segment->name, ".eh_frame") == 0
	  && S_GET_VALUE (fixP->fx_subsy)
	     == fixP->fx_frag->fr_address + fixP->fx_where)
	{
	  fixP->fx_r_type = BFD_RELOC_RISCV_32_PCREL;
	  fixP->fx_subsy = NULL;
	  break;
	}
      /* Fall through.  */
    case BFD_RELOC_64:
    case BFD_RELOC_16:
    case BFD_RELOC_8:
    case BFD_RELOC_RISCV_CFA:
      if (fixP->fx_addsy && fixP->fx_subsy)
	{
	  fixP->fx_next = xmemdup (fixP, sizeof (*fixP), sizeof (*fixP));
	  fixP->fx_next->fx_addsy = fixP->fx_subsy;
	  fixP->fx_next->fx_subsy = NULL;
	  fixP->fx_next->fx_offset = 0;
	  fixP->fx_subsy = NULL;

	  switch (fixP->fx_r_type)
	    {
	    case BFD_RELOC_64:
	      fixP->fx_r_type = BFD_RELOC_RISCV_ADD64;
	      fixP->fx_next->fx_r_type = BFD_RELOC_RISCV_SUB64;
	      break;

	    case BFD_RELOC_32:
	      fixP->fx_r_type = BFD_RELOC_RISCV_ADD32;
	      fixP->fx_next->fx_r_type = BFD_RELOC_RISCV_SUB32;
	      break;

	    case BFD_RELOC_16:
	      fixP->fx_r_type = BFD_RELOC_RISCV_ADD16;
	      fixP->fx_next->fx_r_type = BFD_RELOC_RISCV_SUB16;
	      break;

	    case BFD_RELOC_8:
	      fixP->fx_r_type = BFD_RELOC_RISCV_ADD8;
	      fixP->fx_next->fx_r_type = BFD_RELOC_RISCV_SUB8;
	      break;

	    case BFD_RELOC_RISCV_CFA:
	      /* Load the byte to get the subtype.  */
	      subtype = bfd_get_8 (NULL, &((fragS *) (fixP->fx_frag->fr_opcode))->fr_literal[fixP->fx_where]);
	      loc = fixP->fx_frag->fr_fix - (subtype & 7);
	      switch (subtype)
		{
		case DW_CFA_advance_loc1:
		  fixP->fx_where = loc + 1;
		  fixP->fx_next->fx_where = loc + 1;
		  fixP->fx_r_type = BFD_RELOC_RISCV_SET8;
		  fixP->fx_next->fx_r_type = BFD_RELOC_RISCV_SUB8;
		  break;

		case DW_CFA_advance_loc2:
		  fixP->fx_size = 2;
		  fixP->fx_next->fx_size = 2;
		  fixP->fx_where = loc + 1;
		  fixP->fx_next->fx_where = loc + 1;
		  fixP->fx_r_type = BFD_RELOC_RISCV_SET16;
		  fixP->fx_next->fx_r_type = BFD_RELOC_RISCV_SUB16;
		  break;

		case DW_CFA_advance_loc4:
		  fixP->fx_size = 4;
		  fixP->fx_next->fx_size = 4;
		  fixP->fx_where = loc;
		  fixP->fx_next->fx_where = loc;
		  fixP->fx_r_type = BFD_RELOC_RISCV_SET32;
		  fixP->fx_next->fx_r_type = BFD_RELOC_RISCV_SUB32;
		  break;

		default:
		  if (subtype < 0x80 && (subtype & 0x40))
		    {
		      /* DW_CFA_advance_loc */
		      fixP->fx_frag = (fragS *) fixP->fx_frag->fr_opcode;
		      fixP->fx_next->fx_frag = fixP->fx_frag;
		      fixP->fx_r_type = BFD_RELOC_RISCV_SET6;
		      fixP->fx_next->fx_r_type = BFD_RELOC_RISCV_SUB6;
		    }
		  else
		    as_fatal (_("internal: bad CFA value #%d"), subtype);
		  break;
		}
	      break;

	    default:
	      /* This case is unreachable.  */
	      abort ();
	    }
	}
      /* Fall through.  */

    case BFD_RELOC_RVA:
      /* If we are deleting this reloc entry, we must fill in the
	 value now.  This can happen if we have a .word which is not
	 resolved when it appears but is later defined.  */
      if (fixP->fx_addsy == NULL)
	{
	  gas_assert (fixP->fx_size <= sizeof (valueT));
	  md_number_to_chars ((char *) buf, *valP, fixP->fx_size);
	  fixP->fx_done = 1;
	}
      break;

    case BFD_RELOC_RISCV_JMP:
      if (fixP->fx_addsy)
	{
	  /* Fill in a tentative value to improve objdump readability.  */
	  bfd_vma target = S_GET_VALUE (fixP->fx_addsy) + *valP;
	  bfd_vma delta = target - md_pcrel_from (fixP);
	  bfd_putl32 (bfd_getl32 (buf) | ENCODE_JTYPE_IMM (delta), buf);
	}
      break;

    case BFD_RELOC_12_PCREL:
      if (fixP->fx_addsy)
	{
	  /* Fill in a tentative value to improve objdump readability.  */
	  bfd_vma target = S_GET_VALUE (fixP->fx_addsy) + *valP;
	  bfd_vma delta = target - md_pcrel_from (fixP);
	  bfd_putl32 (bfd_getl32 (buf) | ENCODE_BTYPE_IMM (delta), buf);
	}
      break;

    case BFD_RELOC_RISCV_RVC_BRANCH:
      if (fixP->fx_addsy)
	{
	  /* Fill in a tentative value to improve objdump readability.  */
	  bfd_vma target = S_GET_VALUE (fixP->fx_addsy) + *valP;
	  bfd_vma delta = target - md_pcrel_from (fixP);
	  bfd_putl16 (bfd_getl16 (buf) | ENCODE_CBTYPE_IMM (delta), buf);
	}
      break;

    case BFD_RELOC_RISCV_RVC_JUMP:
      if (fixP->fx_addsy)
	{
	  /* Fill in a tentative value to improve objdump readability.  */
	  bfd_vma target = S_GET_VALUE (fixP->fx_addsy) + *valP;
	  bfd_vma delta = target - md_pcrel_from (fixP);
	  bfd_putl16 (bfd_getl16 (buf) | ENCODE_CJTYPE_IMM (delta), buf);
	}
      break;

    case BFD_RELOC_RISCV_CALL:
    case BFD_RELOC_RISCV_CALL_PLT:
      relaxable = true;
      break;

    case BFD_RELOC_RISCV_PCREL_HI20:
    case BFD_RELOC_RISCV_PCREL_LO12_S:
    case BFD_RELOC_RISCV_PCREL_LO12_I:
      relaxable = riscv_opts.relax;
      break;

    case BFD_RELOC_RISCV_ALIGN:
      break;

    default:
      /* We ignore generic BFD relocations we don't know about.  */
      if (bfd_reloc_type_lookup (stdoutput, fixP->fx_r_type) != NULL)
	as_fatal (_("internal: bad relocation #%d"), fixP->fx_r_type);
    }

  if (fixP->fx_subsy != NULL)
    as_bad_subtract (fixP);

  /* Add an R_RISCV_RELAX reloc if the reloc is relaxable.  */
  if (relaxable && fixP->fx_tcbit && fixP->fx_addsy != NULL)
    {
      fixP->fx_next = xmemdup (fixP, sizeof (*fixP), sizeof (*fixP));
      fixP->fx_next->fx_addsy = fixP->fx_next->fx_subsy = NULL;
      fixP->fx_next->fx_r_type = BFD_RELOC_RISCV_RELAX;
      fixP->fx_next->fx_size = 0;
    }
}

/* Because the value of .cfi_remember_state may changed after relaxation,
   we insert a fix to relocate it again in link-time.  */

void
riscv_pre_output_hook (void)
{
  const frchainS *frch;
  segT s;

  /* Save the current segment info.  */
  segT seg = now_seg;
  subsegT subseg = now_subseg;

  for (s = stdoutput->sections; s; s = s->next)
    for (frch = seg_info (s)->frchainP; frch; frch = frch->frch_next)
      {
	fragS *frag;

	for (frag = frch->frch_root; frag; frag = frag->fr_next)
	  {
	    if (frag->fr_type == rs_cfa)
	      {
		expressionS exp;
		expressionS *symval;

		symval = symbol_get_value_expression (frag->fr_symbol);
		exp.X_op = O_subtract;
		exp.X_add_symbol = symval->X_add_symbol;
		exp.X_add_number = 0;
		exp.X_op_symbol = symval->X_op_symbol;

		/* We must set the segment before creating a frag after all
		   frag chains have been chained together.  */
		subseg_set (s, frch->frch_subseg);

		fix_new_exp (frag, (int) frag->fr_offset, 1, &exp, 0,
			     BFD_RELOC_RISCV_CFA);
	      }
	  }
      }

  /* Restore the original segment info.  */
  subseg_set (seg, subseg);
}

/* Handle the .option pseudo-op.  */

static void
s_riscv_option (int x ATTRIBUTE_UNUSED)
{
  char *name = input_line_pointer, ch;

  while (!is_end_of_line[(unsigned char) *input_line_pointer])
    ++input_line_pointer;
  ch = *input_line_pointer;
  *input_line_pointer = '\0';

  if (strcmp (name, "rvc") == 0)
    {
      riscv_update_subset (&riscv_rps_as, "+c");
      riscv_reset_subsets_list_arch_str ();
      riscv_set_rvc (true);
    }
  else if (strcmp (name, "norvc") == 0)
    {
      riscv_update_subset (&riscv_rps_as, "-c");
      riscv_reset_subsets_list_arch_str ();
      riscv_set_rvc (false);
    }
  else if (strcmp (name, "pic") == 0)
    riscv_opts.pic = true;
  else if (strcmp (name, "nopic") == 0)
    riscv_opts.pic = false;
  else if (strcmp (name, "relax") == 0)
    riscv_opts.relax = true;
  else if (strcmp (name, "norelax") == 0)
    riscv_opts.relax = false;
  else if (strcmp (name, "csr-check") == 0)
    riscv_opts.csr_check = true;
  else if (strcmp (name, "no-csr-check") == 0)
    riscv_opts.csr_check = false;
  else if (strncmp (name, "arch,", 5) == 0)
    {
      name += 5;
      if (ISSPACE (*name) && *name != '\0')
	name++;
      riscv_update_subset (&riscv_rps_as, name);
      riscv_reset_subsets_list_arch_str ();

      riscv_set_rvc (false);
      if (riscv_subset_supports (&riscv_rps_as, "c"))
	riscv_set_rvc (true);

      if (riscv_subset_supports (&riscv_rps_as, "ztso"))
	riscv_set_tso ();
    }
  else if (strcmp (name, "push") == 0)
    {
      struct riscv_option_stack *s;

      s = XNEW (struct riscv_option_stack);
      s->next = riscv_opts_stack;
      s->options = riscv_opts;
      s->subset_list = riscv_rps_as.subset_list;
      riscv_opts_stack = s;
      riscv_rps_as.subset_list = riscv_copy_subset_list (s->subset_list);
    }
  else if (strcmp (name, "pop") == 0)
    {
      struct riscv_option_stack *s;

      s = riscv_opts_stack;
      if (s == NULL)
	as_bad (_(".option pop with no .option push"));
      else
	{
	  riscv_subset_list_t *release_subsets = riscv_rps_as.subset_list;
	  riscv_opts_stack = s->next;
	  riscv_opts = s->options;
	  riscv_rps_as.subset_list = s->subset_list;
	  riscv_release_subset_list (release_subsets);
	  free (s);
	}
    }
  else
    {
      as_warn (_("unrecognized .option directive: %s"), name);
    }
  *input_line_pointer = ch;
  demand_empty_rest_of_line ();
}

/* Handle the .dtprelword and .dtpreldword pseudo-ops.  They generate
   a 32-bit or 64-bit DTP-relative relocation (BYTES says which) for
   use in DWARF debug information.  */

static void
s_dtprel (int bytes)
{
  expressionS ex;
  char *p;

  expression (&ex);

  if (ex.X_op != O_symbol)
    {
      as_bad (_("unsupported use of %s"), (bytes == 8
					   ? ".dtpreldword"
					   : ".dtprelword"));
      ignore_rest_of_line ();
    }

  p = frag_more (bytes);
  md_number_to_chars (p, 0, bytes);
  fix_new_exp (frag_now, p - frag_now->fr_literal, bytes, &ex, false,
	       (bytes == 8
		? BFD_RELOC_RISCV_TLS_DTPREL64
		: BFD_RELOC_RISCV_TLS_DTPREL32));

  demand_empty_rest_of_line ();
}

/* Handle the .bss pseudo-op.  */

static void
s_bss (int ignore ATTRIBUTE_UNUSED)
{
  subseg_set (bss_section, 0);
  demand_empty_rest_of_line ();
}

static void
riscv_make_nops (char *buf, bfd_vma bytes)
{
  bfd_vma i = 0;

  /* RISC-V instructions cannot begin or end on odd addresses, so this case
     means we are not within a valid instruction sequence.  It is thus safe
     to use a zero byte, even though that is not a valid instruction.  */
  if (bytes % 2 == 1)
    buf[i++] = 0;

  /* Use at most one 2-byte NOP.  */
  if ((bytes - i) % 4 == 2)
    {
      number_to_chars_littleendian (buf + i, RVC_NOP, 2);
      i += 2;
    }

  /* Fill the remainder with 4-byte NOPs.  */
  for ( ; i < bytes; i += 4)
    number_to_chars_littleendian (buf + i, RISCV_NOP, 4);
}

/* Called from md_do_align.  Used to create an alignment frag in a
   code section by emitting a worst-case NOP sequence that the linker
   will later relax to the correct number of NOPs.  We can't compute
   the correct alignment now because of other linker relaxations.  */

bool
riscv_frag_align_code (int n)
{
  bfd_vma bytes = (bfd_vma) 1 << n;
  bfd_vma insn_alignment = riscv_opts.rvc ? 2 : 4;
  bfd_vma worst_case_bytes = bytes - insn_alignment;
  char *nops;
  expressionS ex;

  /* If we are moving to a smaller alignment than the instruction size, then no
     alignment is required. */
  if (bytes <= insn_alignment)
    return true;

  /* When not relaxing, riscv_handle_align handles code alignment.  */
  if (!riscv_opts.relax)
    return false;

  /* Maybe we should use frag_var to create a new rs_align_code fragment,
     rather than just use frag_more to handle an alignment here?  So that we
     don't need to call riscv_mapping_state again later, and then only need
     to check frag->fr_type to see if it is frag_align_code.  */
  nops = frag_more (worst_case_bytes);

  ex.X_op = O_constant;
  ex.X_add_number = worst_case_bytes;

  riscv_make_nops (nops, worst_case_bytes);

  fix_new_exp (frag_now, nops - frag_now->fr_literal, 0,
	       &ex, false, BFD_RELOC_RISCV_ALIGN);

  riscv_mapping_state (MAP_INSN, worst_case_bytes, true/* fr_align_code */);

  /* We need to start a new frag after the alignment which may be removed by
     the linker, to prevent the assembler from computing static offsets.
     This is necessary to get correct EH info.  */
  frag_wane (frag_now);
  frag_new (0);

  return true;
}

/* Implement HANDLE_ALIGN.  */

void
riscv_handle_align (fragS *fragP)
{
  switch (fragP->fr_type)
    {
    case rs_align_code:
      /* When relaxing, riscv_frag_align_code handles code alignment.  */
      if (!riscv_opts.relax)
	{
	  bfd_signed_vma bytes = (fragP->fr_next->fr_address
				  - fragP->fr_address - fragP->fr_fix);
	  /* We have 4 byte uncompressed nops.  */
	  bfd_signed_vma size = 4;
	  bfd_signed_vma excess = bytes % size;
	  bfd_boolean odd_padding = (excess % 2 == 1);
	  char *p = fragP->fr_literal + fragP->fr_fix;

	  if (bytes <= 0)
	    break;

	  /* Insert zeros or compressed nops to get 4 byte alignment.  */
	  if (excess)
	    {
	      if (odd_padding)
		riscv_add_odd_padding_symbol (fragP);
	      riscv_make_nops (p, excess);
	      fragP->fr_fix += excess;
	      p += excess;
	    }

	  /* The frag will be changed to `rs_fill` later.  The function
	     `write_contents` will try to fill the remaining spaces
	     according to the patterns we give.  In this case, we give
	     a 4 byte uncompressed nop as the pattern, and set the size
	     of the pattern into `fr_var`.  The nop will be output to the
	     file `fr_offset` times.  However, `fr_offset` could be zero
	     if we don't need to pad the boundary finally.  */
	  riscv_make_nops (p, size);
	  fragP->fr_var = size;
	}
      break;

    default:
      break;
    }
}

/* This usually called from frag_var.  */

void
riscv_init_frag (fragS * fragP, int max_chars)
{
  /* Do not add mapping symbol to debug sections.  */
  if (bfd_section_flags (now_seg) & SEC_DEBUGGING)
    return;

  switch (fragP->fr_type)
    {
    case rs_fill:
    case rs_align:
    case rs_align_test:
      riscv_mapping_state (MAP_DATA, max_chars, false/* fr_align_code */);
      break;
    case rs_align_code:
      riscv_mapping_state (MAP_INSN, max_chars, true/* fr_align_code */);
      break;
    default:
      break;
    }
}

int
md_estimate_size_before_relax (fragS *fragp, asection *segtype)
{
  return (fragp->fr_var = relaxed_branch_length (fragp, segtype, false));
}

/* Translate internal representation of relocation info to BFD target
   format.  */

arelent *
tc_gen_reloc (asection *section ATTRIBUTE_UNUSED, fixS *fixp)
{
  arelent *reloc = (arelent *) xmalloc (sizeof (arelent));

  reloc->sym_ptr_ptr = (asymbol **) xmalloc (sizeof (asymbol *));
  *reloc->sym_ptr_ptr = symbol_get_bfdsym (fixp->fx_addsy);
  reloc->address = fixp->fx_frag->fr_address + fixp->fx_where;
  reloc->addend = fixp->fx_addnumber;

  reloc->howto = bfd_reloc_type_lookup (stdoutput, fixp->fx_r_type);
  if (reloc->howto == NULL)
    {
      if ((fixp->fx_r_type == BFD_RELOC_16 || fixp->fx_r_type == BFD_RELOC_8)
	  && fixp->fx_addsy != NULL && fixp->fx_subsy != NULL)
	{
	  /* We don't have R_RISCV_8/16, but for this special case,
	     we can use R_RISCV_ADD8/16 with R_RISCV_SUB8/16.  */
	  return reloc;
	}

      as_bad_where (fixp->fx_file, fixp->fx_line,
		    _("cannot represent %s relocation in object file"),
		    bfd_get_reloc_code_name (fixp->fx_r_type));
      return NULL;
    }

  return reloc;
}

int
riscv_relax_frag (asection *sec, fragS *fragp, long stretch ATTRIBUTE_UNUSED)
{
  if (RELAX_BRANCH_P (fragp->fr_subtype))
    {
      offsetT old_var = fragp->fr_var;
      fragp->fr_var = relaxed_branch_length (fragp, sec, true);
      return fragp->fr_var - old_var;
    }

  return 0;
}

/* Expand far branches to multi-instruction sequences.  */

static void
md_convert_frag_branch (fragS *fragp)
{
  bfd_byte *buf;
  expressionS exp;
  fixS *fixp;
  insn_t insn;
  int rs1, reloc;

  buf = (bfd_byte *)fragp->fr_literal + fragp->fr_fix;

  exp.X_op = O_symbol;
  exp.X_add_symbol = fragp->fr_symbol;
  exp.X_add_number = fragp->fr_offset;

  gas_assert (fragp->fr_var == RELAX_BRANCH_LENGTH (fragp->fr_subtype));

  if (RELAX_BRANCH_RVC (fragp->fr_subtype))
    {
      switch (RELAX_BRANCH_LENGTH (fragp->fr_subtype))
	{
	  case 8:
	  case 4:
	    /* Expand the RVC branch into a RISC-V one.  */
	    insn = bfd_getl16 (buf);
	    rs1 = 8 + ((insn >> OP_SH_CRS1S) & OP_MASK_CRS1S);
	    if ((insn & MASK_C_J) == MATCH_C_J)
	      insn = MATCH_JAL;
	    else if ((insn & MASK_C_JAL) == MATCH_C_JAL)
	      insn = MATCH_JAL | (X_RA << OP_SH_RD);
	    else if ((insn & MASK_C_BEQZ) == MATCH_C_BEQZ)
	      insn = MATCH_BEQ | (rs1 << OP_SH_RS1);
	    else if ((insn & MASK_C_BNEZ) == MATCH_C_BNEZ)
	      insn = MATCH_BNE | (rs1 << OP_SH_RS1);
	    else
	      abort ();
	    bfd_putl32 (insn, buf);
	    break;

	  case 6:
	    /* Invert the branch condition.  Branch over the jump.  */
	    insn = bfd_getl16 (buf);
	    insn ^= MATCH_C_BEQZ ^ MATCH_C_BNEZ;
	    insn |= ENCODE_CBTYPE_IMM (6);
	    bfd_putl16 (insn, buf);
	    buf += 2;
	    goto jump;

	  case 2:
	    /* Just keep the RVC branch.  */
	    reloc = RELAX_BRANCH_UNCOND (fragp->fr_subtype)
		    ? BFD_RELOC_RISCV_RVC_JUMP : BFD_RELOC_RISCV_RVC_BRANCH;
	    fixp = fix_new_exp (fragp, buf - (bfd_byte *)fragp->fr_literal,
				2, &exp, false, reloc);
	    buf += 2;
	    goto done;

	  default:
	    abort ();
	}
    }

  switch (RELAX_BRANCH_LENGTH (fragp->fr_subtype))
    {
    case 8:
      gas_assert (!RELAX_BRANCH_UNCOND (fragp->fr_subtype));

      /* Invert the branch condition.  Branch over the jump.  */
      insn = bfd_getl32 (buf);
      insn ^= MATCH_BEQ ^ MATCH_BNE;
      insn |= ENCODE_BTYPE_IMM (8);
      bfd_putl32 (insn, buf);
      buf += 4;

    jump:
      /* Jump to the target.  */
      fixp = fix_new_exp (fragp, buf - (bfd_byte *)fragp->fr_literal,
			  4, &exp, false, BFD_RELOC_RISCV_JMP);
      bfd_putl32 (MATCH_JAL, buf);
      buf += 4;
      break;

    case 4:
      reloc = RELAX_BRANCH_UNCOND (fragp->fr_subtype)
	      ? BFD_RELOC_RISCV_JMP : BFD_RELOC_12_PCREL;
      fixp = fix_new_exp (fragp, buf - (bfd_byte *)fragp->fr_literal,
			  4, &exp, false, reloc);
      buf += 4;
      break;

    default:
      abort ();
    }

 done:
  fixp->fx_file = fragp->fr_file;
  fixp->fx_line = fragp->fr_line;

  gas_assert (buf == (bfd_byte *)fragp->fr_literal
	      + fragp->fr_fix + fragp->fr_var);

  fragp->fr_fix += fragp->fr_var;
}

/* Relax a machine dependent frag.  This returns the amount by which
   the current size of the frag should change.  */

void
md_convert_frag (bfd *abfd ATTRIBUTE_UNUSED, segT asec ATTRIBUTE_UNUSED,
		 fragS *fragp)
{
  gas_assert (RELAX_BRANCH_P (fragp->fr_subtype));
  md_convert_frag_branch (fragp);
}

void
md_show_usage (FILE *stream)
{
  fprintf (stream, _("\
RISC-V options:\n\
  -fpic or -fPIC              generate position-independent code\n\
  -fno-pic                    don't generate position-independent code (default)\n\
  -march=ISA                  set the RISC-V architecture\n\
  -misa-spec=ISAspec          set the RISC-V ISA spec (2.2, 20190608, 20191213)\n\
  -mpriv-spec=PRIVspec        set the RISC-V privilege spec (1.9.1, 1.10, 1.11, 1.12)\n\
  -mabi=ABI                   set the RISC-V ABI\n\
  -mrelax                     enable relax (default)\n\
  -mno-relax                  disable relax\n\
  -march-attr                 generate RISC-V arch attribute\n\
  -mno-arch-attr              don't generate RISC-V arch attribute\n\
  -mcsr-check                 enable the csr ISA and privilege spec version checks\n\
  -mno-csr-check              disable the csr ISA and privilege spec version checks (default)\n\
  -mbig-endian                assemble for big-endian\n\
  -mlittle-endian             assemble for little-endian\n\
"));
}

/* Standard calling conventions leave the CFA at SP on entry.  */

void
riscv_cfi_frame_initial_instructions (void)
{
  cfi_add_CFA_def_cfa_register (X_SP);
}

int
tc_riscv_regname_to_dw2regnum (char *regname)
{
  int reg;

  if ((reg = reg_lookup_internal (regname, RCLASS_GPR)) >= 0)
    return reg;

  if ((reg = reg_lookup_internal (regname, RCLASS_FPR)) >= 0)
    return reg + 32;

  if ((reg = reg_lookup_internal (regname, RCLASS_VECR)) >= 0)
    return reg + 96;

  /* CSRs are numbered 4096 -> 8191.  */
  if ((reg = reg_lookup_internal (regname, RCLASS_CSR)) >= 0)
    return reg + 4096;

  as_bad (_("unknown register `%s'"), regname);
  return -1;
}

void
riscv_elf_final_processing (void)
{
  riscv_set_abi_by_arch ();
  riscv_release_subset_list (riscv_rps_as.subset_list);
  elf_elfheader (stdoutput)->e_flags |= elf_flags;
}

/* Parse the .sleb128 and .uleb128 pseudos.  Only allow constant expressions,
   since these directives break relaxation when used with symbol deltas.  */

static void
s_riscv_leb128 (int sign)
{
  expressionS exp;
  char *save_in = input_line_pointer;

  expression (&exp);
  if (sign && exp.X_op != O_constant)
    as_bad (_("non-constant .sleb128 is not supported"));
  else if (!sign && exp.X_op != O_constant && exp.X_op != O_subtract)
    as_bad (_(".uleb128 only supports constant or subtract expressions"));

  demand_empty_rest_of_line ();

  input_line_pointer = save_in;
  return s_leb128 (sign);
}

/* Parse the .insn directive.  There are three formats,
   Format 1: .insn <type> <operand1>, <operand2>, ...
   Format 2: .insn <length>, <value>
   Format 3: .insn <value>.  */

static void
s_riscv_insn (int x ATTRIBUTE_UNUSED)
{
  char *str = input_line_pointer;
  struct riscv_cl_insn insn;
  expressionS imm_expr;
  bfd_reloc_code_real_type imm_reloc = BFD_RELOC_UNUSED;
  char save_c;

  while (!is_end_of_line[(unsigned char) *input_line_pointer])
    ++input_line_pointer;

  save_c = *input_line_pointer;
  *input_line_pointer = '\0';

  riscv_mapping_state (MAP_INSN, 0, false/* fr_align_code */);

  struct riscv_ip_error error = riscv_ip (str, &insn, &imm_expr,
				&imm_reloc, insn_type_hash);
  if (error.msg)
    {
      char *save_in = input_line_pointer;
      error.msg = riscv_ip_hardcode (str, &insn, &imm_expr, error.msg);
      input_line_pointer = save_in;
    }

  if (error.msg)
    {
      if (error.missing_ext)
	as_bad ("%s `%s', extension `%s' required", error.msg, error.statement,
		error.missing_ext);
      else
	as_bad ("%s `%s'", error.msg, error.statement);
    }
  else
    {
      gas_assert (insn.insn_mo->pinfo != INSN_MACRO);
      append_insn (&insn, &imm_expr, imm_reloc);
    }

  *input_line_pointer = save_c;
  demand_empty_rest_of_line ();
}

/* Update architecture and privileged elf attributes.  If we don't set
   them, then try to output the default ones.  */

static void
riscv_write_out_attrs (void)
{
  const char *arch_str, *priv_str, *p;
  /* versions[0]: major version.
     versions[1]: minor version.
     versions[2]: revision version.  */
  unsigned versions[3] = {0}, number = 0;
  unsigned int i;

  /* Re-write architecture elf attribute.  */
  arch_str = riscv_rps_as.subset_list->arch_str;
  bfd_elf_add_proc_attr_string (stdoutput, Tag_RISCV_arch, arch_str);

  /* For the file without any instruction, we don't set the default_priv_spec
     according to the privileged elf attributes since the md_assemble isn't
     called.  */
  if (!start_assemble
      && !riscv_set_default_priv_spec (NULL))
    return;

  /* If we already have set privileged elf attributes, then no need to do
     anything.  Otherwise, don't generate or update them when no CSR and
     privileged instructions are used.  */
  if (!explicit_priv_attr)
    return;

  RISCV_GET_PRIV_SPEC_NAME (priv_str, default_priv_spec);
  p = priv_str;
  for (i = 0; *p; ++p)
    {
      if (*p == '.' && i < 3)
       {
         versions[i++] = number;
         number = 0;
       }
      else if (ISDIGIT (*p))
       number = (number * 10) + (*p - '0');
      else
       {
         as_bad (_("internal: bad RISC-V privileged spec (%s)"), priv_str);
         return;
       }
    }
  versions[i] = number;

  /* Re-write privileged elf attributes.  */
  bfd_elf_add_proc_attr_int (stdoutput, Tag_RISCV_priv_spec, versions[0]);
  bfd_elf_add_proc_attr_int (stdoutput, Tag_RISCV_priv_spec_minor, versions[1]);
  bfd_elf_add_proc_attr_int (stdoutput, Tag_RISCV_priv_spec_revision, versions[2]);
}

/* Add the default contents for the .riscv.attributes section.  */

static void
riscv_set_public_attributes (void)
{
  if (riscv_opts.arch_attr || explicit_attr)
    riscv_write_out_attrs ();
}

/* Scan uleb128 subtraction expressions and insert fixups for them.
   e.g., .uleb128 .L1 - .L0
   Because relaxation may change the value of the subtraction, we
   must resolve them at link-time.  */

static void
riscv_insert_uleb128_fixes (bfd *abfd ATTRIBUTE_UNUSED,
			    asection *sec, void *xxx ATTRIBUTE_UNUSED)
{
  segment_info_type *seginfo = seg_info (sec);
  struct frag *fragP;

  subseg_set (sec, 0);

  for (fragP = seginfo->frchainP->frch_root;
       fragP; fragP = fragP->fr_next)
    {
      expressionS *exp, *exp_dup;

      if (fragP->fr_type != rs_leb128  || fragP->fr_symbol == NULL)
	continue;

      exp = symbol_get_value_expression (fragP->fr_symbol);

      if (exp->X_op != O_subtract)
	continue;

      /* Only unsigned leb128 can be handled.  */
      gas_assert (fragP->fr_subtype == 0);
      exp_dup = xmemdup (exp, sizeof (*exp), sizeof (*exp));
      exp_dup->X_op = O_symbol;
      exp_dup->X_op_symbol = NULL;

      /* Insert relocations to resolve the subtraction at link-time.
	 Emit the SET relocation first in riscv.  */
      exp_dup->X_add_symbol = exp->X_add_symbol;
      fix_new_exp (fragP, fragP->fr_fix, 0,
		   exp_dup, 0, BFD_RELOC_RISCV_SET_ULEB128);
      exp_dup->X_add_symbol = exp->X_op_symbol;
      fix_new_exp (fragP, fragP->fr_fix, 0,
		   exp_dup, 0, BFD_RELOC_RISCV_SUB_ULEB128);
    }
}

/* Called after all assembly has been done.  */

void
riscv_md_finish (void)
{
  riscv_set_public_attributes ();
  if (riscv_opts.relax)
    bfd_map_over_sections (stdoutput, riscv_insert_uleb128_fixes, NULL);
}

/* Adjust the symbol table.  */

void
riscv_adjust_symtab (void)
{
  bfd_map_over_sections (stdoutput, riscv_check_mapping_symbols, (char *) 0);
  elf_adjust_symtab ();
}

/* Given a symbolic attribute NAME, return the proper integer value.
   Returns -1 if the attribute is not known.  */

int
riscv_convert_symbolic_attribute (const char *name)
{
  static const struct
  {
    const char *name;
    const int tag;
  }
  attribute_table[] =
  {
    /* When you modify this table you should
       also modify the list in doc/c-riscv.texi.  */
#define T(tag) {#tag, Tag_RISCV_##tag}, {"Tag_RISCV_" #tag, Tag_RISCV_##tag}
    T(arch),
    T(priv_spec),
    T(priv_spec_minor),
    T(priv_spec_revision),
    T(unaligned_access),
    T(stack_align),
#undef T
  };

  if (name == NULL)
    return -1;

  unsigned int i;
  for (i = 0; i < ARRAY_SIZE (attribute_table); i++)
    if (strcmp (name, attribute_table[i].name) == 0)
      return attribute_table[i].tag;

  return -1;
}

/* Parse a .attribute directive.  */

static void
s_riscv_attribute (int ignored ATTRIBUTE_UNUSED)
{
  int tag = obj_elf_vendor_attribute (OBJ_ATTR_PROC);
  unsigned old_xlen;
  obj_attribute *attr;

  explicit_attr = true;
  switch (tag)
    {
    case Tag_RISCV_arch:
      old_xlen = xlen;
      attr = elf_known_obj_attributes_proc (stdoutput);
      if (!start_assemble)
	riscv_set_arch (attr[Tag_RISCV_arch].s);
      else
	as_fatal (_("architecture elf attributes must set before "
		    "any instructions"));

      if (old_xlen != xlen)
	{
	  /* We must re-init bfd again if xlen is changed.  */
	  unsigned long mach = xlen == 64 ? bfd_mach_riscv64 : bfd_mach_riscv32;
	  bfd_find_target (riscv_target_format (), stdoutput);

	  if (! bfd_set_arch_mach (stdoutput, bfd_arch_riscv, mach))
	    as_warn (_("could not set architecture and machine"));
	}
      break;

    case Tag_RISCV_priv_spec:
    case Tag_RISCV_priv_spec_minor:
    case Tag_RISCV_priv_spec_revision:
      if (start_assemble)
       as_fatal (_("privileged elf attributes must set before "
		   "any instructions"));
      break;

    default:
      break;
    }
}

/* Mark symbol that it follows a variant CC convention.  */

static void
s_variant_cc (int ignored ATTRIBUTE_UNUSED)
{
  char *name;
  char c;
  symbolS *sym;
  asymbol *bfdsym;
  elf_symbol_type *elfsym;

  c = get_symbol_name (&name);
  if (!*name)
    as_bad (_("missing symbol name for .variant_cc directive"));
  sym = symbol_find_or_make (name);
  restore_line_pointer (c);
  demand_empty_rest_of_line ();

  bfdsym = symbol_get_bfdsym (sym);
  elfsym = elf_symbol_from (bfdsym);
  gas_assert (elfsym);
  elfsym->internal_elf_sym.st_other |= STO_RISCV_VARIANT_CC;
}

/* Same as elf_copy_symbol_attributes, but without copying st_other.
   This is needed so RISC-V specific st_other values can be independently
   specified for an IFUNC resolver (that is called by the dynamic linker)
   and the symbol it resolves (aliased to the resolver).  In particular,
   if a function symbol has special st_other value set via directives,
   then attaching an IFUNC resolver to that symbol should not override
   the st_other setting.  Requiring the directive on the IFUNC resolver
   symbol would be unexpected and problematic in C code, where the two
   symbols appear as two independent function declarations.  */

void
riscv_elf_copy_symbol_attributes (symbolS *dest, symbolS *src)
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

/* RISC-V pseudo-ops table.  */
static const pseudo_typeS riscv_pseudo_table[] =
{
  {"option", s_riscv_option, 0},
  {"half", cons, 2},
  {"word", cons, 4},
  {"dword", cons, 8},
  {"dtprelword", s_dtprel, 4},
  {"dtpreldword", s_dtprel, 8},
  {"bss", s_bss, 0},
  {"uleb128", s_riscv_leb128, 0},
  {"sleb128", s_riscv_leb128, 1},
  {"insn", s_riscv_insn, 0},
  {"attribute", s_riscv_attribute, 0},
  {"variant_cc", s_variant_cc, 0},
  {"float16", float_cons, 'h'},

  { NULL, NULL, 0 },
};

void
riscv_pop_insert (void)
{
  extern void pop_insert (const pseudo_typeS *);

  pop_insert (riscv_pseudo_table);
}
