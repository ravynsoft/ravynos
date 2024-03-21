/* RISC-V disassembler
   Copyright (C) 2011-2023 Free Software Foundation, Inc.

   Contributed by Andrew Waterman (andrew@sifive.com).
   Based on MIPS target.

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
   along with this program; see the file COPYING3. If not,
   see <http://www.gnu.org/licenses/>.  */

#include "sysdep.h"
#include "disassemble.h"
#include "libiberty.h"
#include "opcode/riscv.h"
#include "opintl.h"
#include "elf-bfd.h"
#include "elf/riscv.h"
#include "elfxx-riscv.h"

#include <stdint.h>
#include <ctype.h>

/* Current XLEN for the disassembler.  */
static unsigned xlen = 0;

/* Default ISA specification version (constant as of now).  */
static enum riscv_spec_class default_isa_spec = ISA_SPEC_CLASS_DRAFT - 1;

/* Default privileged specification
   (as specified by the ELF attributes or the `priv-spec' option).  */
static enum riscv_spec_class default_priv_spec = PRIV_SPEC_CLASS_NONE;

static riscv_subset_list_t riscv_subsets;
static riscv_parse_subset_t riscv_rps_dis =
{
  &riscv_subsets,	/* subset_list.  */
  opcodes_error_handler,/* error_handler.  */
  &xlen,		/* xlen.  */
  &default_isa_spec,	/* isa_spec.  */
  false,		/* check_unknown_prefixed_ext.  */
};

struct riscv_private_data
{
  bfd_vma gp;
  bfd_vma print_addr;
  bfd_vma hi_addr[OP_MASK_RD + 1];
  bool to_print_addr;
  bool has_gp;
};

/* Used for mapping symbols.  */
static int last_map_symbol = -1;
static bfd_vma last_stop_offset = 0;
static bfd_vma last_map_symbol_boundary = 0;
static enum riscv_seg_mstate last_map_state = MAP_NONE;
static asection *last_map_section = NULL;

/* Register names as used by the disassembler.  */
static const char * const *riscv_gpr_names;
static const char * const *riscv_fpr_names;

/* If set, disassemble as most general instruction.  */
static bool no_aliases = false;


/* Set default RISC-V disassembler options.  */

static void
set_default_riscv_dis_options (void)
{
  riscv_gpr_names = riscv_gpr_names_abi;
  riscv_fpr_names = riscv_fpr_names_abi;
  no_aliases = false;
}

/* Parse RISC-V disassembler option (without arguments).  */

static bool
parse_riscv_dis_option_without_args (const char *option)
{
  if (strcmp (option, "no-aliases") == 0)
    no_aliases = true;
  else if (strcmp (option, "numeric") == 0)
    {
      riscv_gpr_names = riscv_gpr_names_numeric;
      riscv_fpr_names = riscv_fpr_names_numeric;
    }
  else
    return false;
  return true;
}

/* Parse RISC-V disassembler option (possibly with arguments).  */

static void
parse_riscv_dis_option (const char *option)
{
  char *equal, *value;

  if (parse_riscv_dis_option_without_args (option))
    return;

  equal = strchr (option, '=');
  if (equal == NULL)
    {
      /* The option without '=' should be defined above.  */
      opcodes_error_handler (_("unrecognized disassembler option: %s"), option);
      return;
    }
  if (equal == option
      || *(equal + 1) == '\0')
    {
      /* Invalid options with '=', no option name before '=',
       and no value after '='.  */
      opcodes_error_handler (_("unrecognized disassembler option with '=': %s"),
                            option);
      return;
    }

  *equal = '\0';
  value = equal + 1;
  if (strcmp (option, "priv-spec") == 0)
    {
      enum riscv_spec_class priv_spec = PRIV_SPEC_CLASS_NONE;
      const char *name = NULL;

      RISCV_GET_PRIV_SPEC_CLASS (value, priv_spec);
      if (priv_spec == PRIV_SPEC_CLASS_NONE)
	opcodes_error_handler (_("unknown privileged spec set by %s=%s"),
			       option, value);
      else if (default_priv_spec == PRIV_SPEC_CLASS_NONE)
	default_priv_spec = priv_spec;
      else if (default_priv_spec != priv_spec)
	{
	  RISCV_GET_PRIV_SPEC_NAME (name, default_priv_spec);
	  opcodes_error_handler (_("mis-matched privilege spec set by %s=%s, "
				   "the elf privilege attribute is %s"),
				 option, value, name);
	}
    }
  else
    {
      /* xgettext:c-format */
      opcodes_error_handler (_("unrecognized disassembler option: %s"), option);
    }
}

/* Parse RISC-V disassembler options.  */

static void
parse_riscv_dis_options (const char *opts_in)
{
  char *opts = xstrdup (opts_in), *opt = opts, *opt_end = opts;

  set_default_riscv_dis_options ();

  for ( ; opt_end != NULL; opt = opt_end + 1)
    {
      if ((opt_end = strchr (opt, ',')) != NULL)
	*opt_end = 0;
      parse_riscv_dis_option (opt);
    }

  free (opts);
}

/* Print one argument from an array.  */

static void
arg_print (struct disassemble_info *info, unsigned long val,
	   const char* const* array, size_t size)
{
  const char *s = val >= size || array[val] == NULL ? "unknown" : array[val];
  (*info->fprintf_styled_func) (info->stream, dis_style_text, "%s", s);
}

/* If we need to print an address, set its value and state.  */

static void
maybe_print_address (struct riscv_private_data *pd, int base_reg, int offset,
		     int wide)
{
  if (pd->hi_addr[base_reg] != (bfd_vma)-1)
    {
      pd->print_addr = (base_reg != 0 ? pd->hi_addr[base_reg] : 0) + offset;
      pd->hi_addr[base_reg] = -1;
    }
  else if (base_reg == X_GP && pd->has_gp)
    pd->print_addr = pd->gp + offset;
  else if (base_reg == X_TP || base_reg == 0)
    pd->print_addr = offset;
  else
    return;  /* Don't print the address.  */
  pd->to_print_addr = true;

  /* Sign-extend a 32-bit value to a 64-bit value.  */
  if (wide)
    pd->print_addr = (bfd_vma)(int32_t) pd->print_addr;

  /* Fit into a 32-bit value on RV32.  */
  if (xlen == 32)
    pd->print_addr = (bfd_vma)(uint32_t)pd->print_addr;
}

/* Print insn arguments for 32/64-bit code.  */

static void
print_insn_args (const char *oparg, insn_t l, bfd_vma pc, disassemble_info *info)
{
  struct riscv_private_data *pd = info->private_data;
  int rs1 = (l >> OP_SH_RS1) & OP_MASK_RS1;
  int rd = (l >> OP_SH_RD) & OP_MASK_RD;
  fprintf_styled_ftype print = info->fprintf_styled_func;
  const char *opargStart;

  if (*oparg != '\0')
    print (info->stream, dis_style_text, "\t");

  for (; *oparg != '\0'; oparg++)
    {
      opargStart = oparg;
      switch (*oparg)
	{
	case 'C': /* RVC */
	  switch (*++oparg)
	    {
	    case 's': /* RS1 x8-x15.  */
	    case 'w': /* RS1 x8-x15.  */
	      print (info->stream, dis_style_register, "%s",
		     riscv_gpr_names[EXTRACT_OPERAND (CRS1S, l) + 8]);
	      break;
	    case 't': /* RS2 x8-x15.  */
	    case 'x': /* RS2 x8-x15.  */
	      print (info->stream, dis_style_register, "%s",
		     riscv_gpr_names[EXTRACT_OPERAND (CRS2S, l) + 8]);
	      break;
	    case 'U': /* RS1, constrained to equal RD.  */
	      print (info->stream, dis_style_register,
		     "%s", riscv_gpr_names[rd]);
	      break;
	    case 'c': /* RS1, constrained to equal sp.  */
	      print (info->stream, dis_style_register, "%s",
		     riscv_gpr_names[X_SP]);
	      break;
	    case 'V': /* RS2 */
	      print (info->stream, dis_style_register, "%s",
		     riscv_gpr_names[EXTRACT_OPERAND (CRS2, l)]);
	      break;
	    case 'o':
	    case 'j':
	      if (((l & MASK_C_ADDI) == MATCH_C_ADDI) && rd != 0)
		maybe_print_address (pd, rd, EXTRACT_CITYPE_IMM (l), 0);
	      if (info->mach == bfd_mach_riscv64
		  && ((l & MASK_C_ADDIW) == MATCH_C_ADDIW) && rd != 0)
		maybe_print_address (pd, rd, EXTRACT_CITYPE_IMM (l), 1);
	      print (info->stream, dis_style_immediate, "%d",
		     (int)EXTRACT_CITYPE_IMM (l));
	      break;
	    case 'k':
	      print (info->stream, dis_style_address_offset, "%d",
		     (int)EXTRACT_CLTYPE_LW_IMM (l));
	      break;
	    case 'l':
	      print (info->stream, dis_style_address_offset, "%d",
		     (int)EXTRACT_CLTYPE_LD_IMM (l));
	      break;
	    case 'm':
	      print (info->stream, dis_style_address_offset, "%d",
		     (int)EXTRACT_CITYPE_LWSP_IMM (l));
	      break;
	    case 'n':
	      print (info->stream, dis_style_address_offset, "%d",
		     (int)EXTRACT_CITYPE_LDSP_IMM (l));
	      break;
	    case 'K':
	      print (info->stream, dis_style_immediate, "%d",
		     (int)EXTRACT_CIWTYPE_ADDI4SPN_IMM (l));
	      break;
	    case 'L':
	      print (info->stream, dis_style_immediate, "%d",
		     (int)EXTRACT_CITYPE_ADDI16SP_IMM (l));
	      break;
	    case 'M':
	      print (info->stream, dis_style_address_offset, "%d",
		     (int)EXTRACT_CSSTYPE_SWSP_IMM (l));
	      break;
	    case 'N':
	      print (info->stream, dis_style_address_offset, "%d",
		     (int)EXTRACT_CSSTYPE_SDSP_IMM (l));
	      break;
	    case 'p':
	      info->target = EXTRACT_CBTYPE_IMM (l) + pc;
	      (*info->print_address_func) (info->target, info);
	      break;
	    case 'a':
	      info->target = EXTRACT_CJTYPE_IMM (l) + pc;
	      (*info->print_address_func) (info->target, info);
	      break;
	    case 'u':
	      print (info->stream, dis_style_immediate, "0x%x",
		     (unsigned)(EXTRACT_CITYPE_IMM (l) & (RISCV_BIGIMM_REACH-1)));
	      break;
	    case '>':
	      print (info->stream, dis_style_immediate, "0x%x",
		     (unsigned)EXTRACT_CITYPE_IMM (l) & 0x3f);
	      break;
	    case '<':
	      print (info->stream, dis_style_immediate, "0x%x",
		     (unsigned)EXTRACT_CITYPE_IMM (l) & 0x1f);
	      break;
	    case 'T': /* Floating-point RS2.  */
	      print (info->stream, dis_style_register, "%s",
		     riscv_fpr_names[EXTRACT_OPERAND (CRS2, l)]);
	      break;
	    case 'D': /* Floating-point RS2 x8-x15.  */
	      print (info->stream, dis_style_register, "%s",
		     riscv_fpr_names[EXTRACT_OPERAND (CRS2S, l) + 8]);
	      break;
	    }
	  break;

	case 'V': /* RVV */
	  switch (*++oparg)
	    {
	    case 'd':
	    case 'f':
	      print (info->stream, dis_style_register, "%s",
		     riscv_vecr_names_numeric[EXTRACT_OPERAND (VD, l)]);
	      break;
	    case 'e':
	      if (!EXTRACT_OPERAND (VWD, l))
		print (info->stream, dis_style_register, "%s",
		       riscv_gpr_names[0]);
	      else
		print (info->stream, dis_style_register, "%s",
		       riscv_vecr_names_numeric[EXTRACT_OPERAND (VD, l)]);
	      break;
	    case 's':
	      print (info->stream, dis_style_register, "%s",
		     riscv_vecr_names_numeric[EXTRACT_OPERAND (VS1, l)]);
	      break;
	    case 't':
	    case 'u': /* VS1 == VS2 already verified at this point.  */
	    case 'v': /* VD == VS1 == VS2 already verified at this point.  */
	      print (info->stream, dis_style_register, "%s",
		     riscv_vecr_names_numeric[EXTRACT_OPERAND (VS2, l)]);
	      break;
	    case '0':
	      print (info->stream, dis_style_register, "%s",
		     riscv_vecr_names_numeric[0]);
	      break;
	    case 'b':
	    case 'c':
	      {
		int imm = (*oparg == 'b') ? EXTRACT_RVV_VB_IMM (l)
					  : EXTRACT_RVV_VC_IMM (l);
		unsigned int imm_vlmul = EXTRACT_OPERAND (VLMUL, imm);
		unsigned int imm_vsew = EXTRACT_OPERAND (VSEW, imm);
		unsigned int imm_vta = EXTRACT_OPERAND (VTA, imm);
		unsigned int imm_vma = EXTRACT_OPERAND (VMA, imm);
		unsigned int imm_vtype_res = (imm >> 8);

		if (imm_vsew < ARRAY_SIZE (riscv_vsew)
		    && imm_vlmul < ARRAY_SIZE (riscv_vlmul)
		    && imm_vta < ARRAY_SIZE (riscv_vta)
		    && imm_vma < ARRAY_SIZE (riscv_vma)
		    && !imm_vtype_res
		    && riscv_vsew[imm_vsew] != NULL
		    && riscv_vlmul[imm_vlmul] != NULL)
		  print (info->stream, dis_style_text, "%s,%s,%s,%s",
			 riscv_vsew[imm_vsew],
			 riscv_vlmul[imm_vlmul], riscv_vta[imm_vta],
			 riscv_vma[imm_vma]);
		else
		  print (info->stream, dis_style_immediate, "%d", imm);
	      }
	      break;
	    case 'i':
	      print (info->stream, dis_style_immediate, "%d",
		     (int)EXTRACT_RVV_VI_IMM (l));
	      break;
	    case 'j':
	      print (info->stream, dis_style_immediate, "%d",
		     (int)EXTRACT_RVV_VI_UIMM (l));
	      break;
	    case 'k':
	      print (info->stream, dis_style_immediate, "%d",
		     (int)EXTRACT_RVV_OFFSET (l));
	      break;
	    case 'l':
	      print (info->stream, dis_style_immediate, "%d",
		     (int)EXTRACT_RVV_VI_UIMM6 (l));
	      break;
	    case 'm':
	      if (!EXTRACT_OPERAND (VMASK, l))
		{
		  print (info->stream, dis_style_text, ",");
		  print (info->stream, dis_style_register, "%s",
			 riscv_vecm_names_numeric[0]);
		}
	      break;
	    }
	  break;

	case ',':
	case '(':
	case ')':
	case '[':
	case ']':
	  print (info->stream, dis_style_text, "%c", *oparg);
	  break;

	case '0':
	  /* Only print constant 0 if it is the last argument.  */
	  if (!oparg[1])
	    print (info->stream, dis_style_immediate, "0");
	  break;

	case 's':
	  if ((l & MASK_JALR) == MATCH_JALR)
	    maybe_print_address (pd, rs1, EXTRACT_ITYPE_IMM (l), 0);
	  print (info->stream, dis_style_register, "%s", riscv_gpr_names[rs1]);
	  break;

	case 't':
	  print (info->stream, dis_style_register, "%s",
		 riscv_gpr_names[EXTRACT_OPERAND (RS2, l)]);
	  break;

	case 'u':
	  print (info->stream, dis_style_immediate, "0x%x",
		 (unsigned)EXTRACT_UTYPE_IMM (l) >> RISCV_IMM_BITS);
	  break;

	case 'm':
	  arg_print (info, EXTRACT_OPERAND (RM, l),
		     riscv_rm, ARRAY_SIZE (riscv_rm));
	  break;

	case 'P':
	  arg_print (info, EXTRACT_OPERAND (PRED, l),
		     riscv_pred_succ, ARRAY_SIZE (riscv_pred_succ));
	  break;

	case 'Q':
	  arg_print (info, EXTRACT_OPERAND (SUCC, l),
		     riscv_pred_succ, ARRAY_SIZE (riscv_pred_succ));
	  break;

	case 'o':
	  maybe_print_address (pd, rs1, EXTRACT_ITYPE_IMM (l), 0);
	  /* Fall through.  */
	case 'j':
	  if (((l & MASK_ADDI) == MATCH_ADDI && rs1 != 0)
	      || (l & MASK_JALR) == MATCH_JALR)
	    maybe_print_address (pd, rs1, EXTRACT_ITYPE_IMM (l), 0);
	  if (info->mach == bfd_mach_riscv64
	      && ((l & MASK_ADDIW) == MATCH_ADDIW) && rs1 != 0)
	    maybe_print_address (pd, rs1, EXTRACT_ITYPE_IMM (l), 1);
	  print (info->stream, dis_style_immediate, "%d",
		 (int)EXTRACT_ITYPE_IMM (l));
	  break;

	case 'q':
	  maybe_print_address (pd, rs1, EXTRACT_STYPE_IMM (l), 0);
	  print (info->stream, dis_style_address_offset, "%d",
		 (int)EXTRACT_STYPE_IMM (l));
	  break;

	case 'a':
	  info->target = EXTRACT_JTYPE_IMM (l) + pc;
	  (*info->print_address_func) (info->target, info);
	  break;

	case 'p':
	  info->target = EXTRACT_BTYPE_IMM (l) + pc;
	  (*info->print_address_func) (info->target, info);
	  break;

	case 'd':
	  if ((l & MASK_AUIPC) == MATCH_AUIPC)
	    pd->hi_addr[rd] = pc + EXTRACT_UTYPE_IMM (l);
	  else if ((l & MASK_LUI) == MATCH_LUI)
	    pd->hi_addr[rd] = EXTRACT_UTYPE_IMM (l);
	  else if ((l & MASK_C_LUI) == MATCH_C_LUI)
	    pd->hi_addr[rd] = EXTRACT_CITYPE_LUI_IMM (l);
	  print (info->stream, dis_style_register, "%s", riscv_gpr_names[rd]);
	  break;

	case 'y':
	  print (info->stream, dis_style_immediate, "0x%x",
		 (unsigned)EXTRACT_OPERAND (BS, l));
	  break;

	case 'z':
	  print (info->stream, dis_style_register, "%s", riscv_gpr_names[0]);
	  break;

	case '>':
	  print (info->stream, dis_style_immediate, "0x%x",
		 (unsigned)EXTRACT_OPERAND (SHAMT, l));
	  break;

	case '<':
	  print (info->stream, dis_style_immediate, "0x%x",
		 (unsigned)EXTRACT_OPERAND (SHAMTW, l));
	  break;

	case 'S':
	case 'U':
	  print (info->stream, dis_style_register, "%s", riscv_fpr_names[rs1]);
	  break;

	case 'T':
	  print (info->stream, dis_style_register, "%s",
		 riscv_fpr_names[EXTRACT_OPERAND (RS2, l)]);
	  break;

	case 'D':
	  print (info->stream, dis_style_register, "%s", riscv_fpr_names[rd]);
	  break;

	case 'R':
	  print (info->stream, dis_style_register, "%s",
		 riscv_fpr_names[EXTRACT_OPERAND (RS3, l)]);
	  break;

	case 'E':
	  {
	    static const char *riscv_csr_hash[4096]; /* Total 2^12 CSRs.  */
	    static bool init_csr = false;
	    unsigned int csr = EXTRACT_OPERAND (CSR, l);

	    if (!init_csr)
	      {
		unsigned int i;
		for (i = 0; i < 4096; i++)
		  riscv_csr_hash[i] = NULL;

		/* Set to the newest privileged version.  */
		if (default_priv_spec == PRIV_SPEC_CLASS_NONE)
		  default_priv_spec = PRIV_SPEC_CLASS_DRAFT - 1;

#define DECLARE_CSR(name, num, class, define_version, abort_version)	\
		if (riscv_csr_hash[num] == NULL 			\
		    && ((define_version == PRIV_SPEC_CLASS_NONE 	\
			 && abort_version == PRIV_SPEC_CLASS_NONE)	\
			|| (default_priv_spec >= define_version 	\
			    && default_priv_spec < abort_version)))	\
		  riscv_csr_hash[num] = #name;
#define DECLARE_CSR_ALIAS(name, num, class, define_version, abort_version) \
		DECLARE_CSR (name, num, class, define_version, abort_version)
#include "opcode/riscv-opc.h"
#undef DECLARE_CSR
	      }

	    if (riscv_csr_hash[csr] != NULL)
	      print (info->stream, dis_style_register, "%s",
		     riscv_csr_hash[csr]);
	    else
	      print (info->stream, dis_style_immediate, "0x%x", csr);
	    break;
	  }

	case 'Y':
	  print (info->stream, dis_style_immediate, "0x%x",
		 (unsigned) EXTRACT_OPERAND (RNUM, l));
	  break;

	case 'Z':
	  print (info->stream, dis_style_immediate, "%d", rs1);
	  break;

	case 'W': /* Various operands.  */
	  {
	    switch (*++oparg)
	      {
	      case 'i':
		switch (*++oparg)
		  {
		  case 'f':
		    print (info->stream, dis_style_address_offset, "%d",
			   (int) EXTRACT_STYPE_IMM (l));
		    break;
		  default:
		    goto undefined_modifier;
		  }
		  break;
	      case 'f':
		switch (*++oparg)
		  {
		  case 'v':
		    if (riscv_fli_symval[rs1])
		      print (info->stream, dis_style_text, "%s",
			     riscv_fli_symval[rs1]);
		    else
		      print (info->stream, dis_style_immediate, "%a",
			     riscv_fli_numval[rs1]);
		    break;
		  default:
		    goto undefined_modifier;
		  }
		break;
	      default:
		goto undefined_modifier;
	      }
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
		  oparg++;
		  while (*oparg && *oparg != ',')
		    {
		      print (info->stream, dis_style_immediate, "%c", *oparg);
		      oparg++;
		    }
		  oparg--;
		  break;
		case 's': /* 'XsN@S' ... N-bit signed immediate at bit S.  */
		  sign = true;
		  goto print_imm;
		case 'u': /* 'XuN@S' ... N-bit unsigned immediate at bit S.  */
		  sign = false;
		  goto print_imm;
		print_imm:
		  n = strtol (oparg + 1, (char **)&oparg, 10);
		  if (*oparg != '@')
		    goto undefined_modifier;
		  s = strtol (oparg + 1, (char **)&oparg, 10);
		  oparg--;

		  if (!sign)
		    print (info->stream, dis_style_immediate, "%lu",
			   (unsigned long)EXTRACT_U_IMM (n, s, l));
		  else
		    print (info->stream, dis_style_immediate, "%li",
			   (signed long)EXTRACT_S_IMM (n, s, l));
		  break;
		default:
		  goto undefined_modifier;
	      }
	  }
	  break;

	default:
	undefined_modifier:
	  /* xgettext:c-format */
	  print (info->stream, dis_style_text,
		 _("# internal error, undefined modifier (%c)"),
		 *opargStart);
	  return;
	}
    }
}

/* Print the RISC-V instruction at address MEMADDR in debugged memory,
   on using INFO.  Returns length of the instruction, in bytes.
   BIGENDIAN must be 1 if this is big-endian code, 0 if
   this is little-endian code.  */

static int
riscv_disassemble_insn (bfd_vma memaddr,
			insn_t word,
			const bfd_byte *packet,
			disassemble_info *info)
{
  const struct riscv_opcode *op;
  static bool init = false;
  static const struct riscv_opcode *riscv_hash[OP_MASK_OP + 1];
  struct riscv_private_data *pd = info->private_data;
  int insnlen, i;
  bool printed;

#define OP_HASH_IDX(i) ((i) & (riscv_insn_length (i) == 2 ? 0x3 : OP_MASK_OP))

  /* Build a hash table to shorten the search time.  */
  if (! init)
    {
      for (op = riscv_opcodes; op->name; op++)
	if (!riscv_hash[OP_HASH_IDX (op->match)])
	  riscv_hash[OP_HASH_IDX (op->match)] = op;

      init = true;
    }

  insnlen = riscv_insn_length (word);

  /* RISC-V instructions are always little-endian.  */
  info->endian_code = BFD_ENDIAN_LITTLE;

  info->bytes_per_chunk = insnlen % 4 == 0 ? 4 : 2;
  info->bytes_per_line = 8;
  /* We don't support constant pools, so this must be code.  */
  info->display_endian = info->endian_code;
  info->insn_info_valid = 1;
  info->branch_delay_insns = 0;
  info->data_size = 0;
  info->insn_type = dis_nonbranch;
  info->target = 0;
  info->target2 = 0;

  op = riscv_hash[OP_HASH_IDX (word)];
  if (op != NULL)
    {
      /* If XLEN is not known, get its value from the ELF class.  */
      if (info->mach == bfd_mach_riscv64)
	xlen = 64;
      else if (info->mach == bfd_mach_riscv32)
	xlen = 32;
      else if (info->section != NULL)
	{
	  Elf_Internal_Ehdr *ehdr = elf_elfheader (info->section->owner);
	  xlen = ehdr->e_ident[EI_CLASS] == ELFCLASS64 ? 64 : 32;
	}

      /* If arch has the Zfinx extension, replace FPR with GPR.  */
      if (riscv_subset_supports (&riscv_rps_dis, "zfinx"))
	riscv_fpr_names = riscv_gpr_names;
      else
	riscv_fpr_names = riscv_gpr_names == riscv_gpr_names_abi ?
			  riscv_fpr_names_abi : riscv_fpr_names_numeric;

      for (; op->name; op++)
	{
	  /* Does the opcode match?  */
	  if (! (op->match_func) (op, word))
	    continue;
	  /* Is this a pseudo-instruction and may we print it as such?  */
	  if (no_aliases && (op->pinfo & INSN_ALIAS))
	    continue;
	  /* Is this instruction restricted to a certain value of XLEN?  */
	  if ((op->xlen_requirement != 0) && (op->xlen_requirement != xlen))
	    continue;
	  /* Is this instruction supported by the current architecture?  */
	  if (!riscv_multi_subset_supports (&riscv_rps_dis, op->insn_class))
	    continue;

	  /* It's a match.  */
	  (*info->fprintf_styled_func) (info->stream, dis_style_mnemonic,
					"%s", op->name);
	  print_insn_args (op->args, word, memaddr, info);

	  /* Try to disassemble multi-instruction addressing sequences.  */
	  if (pd->to_print_addr)
	    {
	      info->target = pd->print_addr;
	      (*info->fprintf_styled_func)
		(info->stream, dis_style_comment_start, " # ");
	      (*info->print_address_func) (info->target, info);
	      pd->to_print_addr = false;
	    }

	  /* Finish filling out insn_info fields.  */
	  switch (op->pinfo & INSN_TYPE)
	    {
	    case INSN_BRANCH:
	      info->insn_type = dis_branch;
	      break;
	    case INSN_CONDBRANCH:
	      info->insn_type = dis_condbranch;
	      break;
	    case INSN_JSR:
	      info->insn_type = dis_jsr;
	      break;
	    case INSN_DREF:
	      info->insn_type = dis_dref;
	      break;
	    default:
	      break;
	    }

	  if (op->pinfo & INSN_DATA_SIZE)
	    {
	      int size = ((op->pinfo & INSN_DATA_SIZE)
			  >> INSN_DATA_SIZE_SHIFT);
	      info->data_size = 1 << (size - 1);
	    }

	  return insnlen;
	}
    }

  /* We did not find a match, so just print the instruction bits in
     the shape of an assembler .insn directive.  */
  info->insn_type = dis_noninsn;
  (*info->fprintf_styled_func)
    (info->stream, dis_style_assembler_directive, ".insn");
  (*info->fprintf_styled_func) (info->stream, dis_style_text, "\t");
  (*info->fprintf_styled_func) (info->stream, dis_style_immediate,
				"%d", insnlen);
  (*info->fprintf_styled_func) (info->stream, dis_style_text, ", ");
  (*info->fprintf_styled_func) (info->stream, dis_style_immediate, "0x");
  for (i = insnlen, printed = false; i >= 2; )
    {
      i -= 2;
      word = bfd_get_bits (packet + i, 16, false);
      if (!word && !printed)
	continue;

      (*info->fprintf_styled_func) (info->stream, dis_style_immediate,
				    "%04x", (unsigned int) word);
      printed = true;
    }

  return insnlen;
}

/* Return true if we find the suitable mapping symbol,
   and also update the STATE.  Otherwise, return false.  */

static bool
riscv_get_map_state (int n,
		     enum riscv_seg_mstate *state,
		     struct disassemble_info *info)
{
  const char *name;

  /* If the symbol is in a different section, ignore it.  */
  if (info->section != NULL
      && info->section != info->symtab[n]->section)
    return false;

  name = bfd_asymbol_name(info->symtab[n]);
  if (strcmp (name, "$x") == 0)
    *state = MAP_INSN;
  else if (strcmp (name, "$d") == 0)
    *state = MAP_DATA;
  else if (strncmp (name, "$xrv", 4) == 0)
    {
      *state = MAP_INSN;
      riscv_release_subset_list (&riscv_subsets);
      riscv_parse_subset (&riscv_rps_dis, name + 2);
    }
  else
    return false;

  return true;
}

/* Check the sorted symbol table (sorted by the symbol value), find the
   suitable mapping symbols.  */

static enum riscv_seg_mstate
riscv_search_mapping_symbol (bfd_vma memaddr,
			     struct disassemble_info *info)
{
  enum riscv_seg_mstate mstate;
  bool from_last_map_symbol;
  bool found = false;
  int symbol = -1;
  int n;

  /* Return the last map state if the address is still within the range of the
     last mapping symbol.  */
  if (last_map_section == info->section
      && (memaddr < last_map_symbol_boundary))
    return last_map_state;

  last_map_section = info->section;

  /* Decide whether to print the data or instruction by default, in case
     we can not find the corresponding mapping symbols.  */
  mstate = MAP_DATA;
  if ((info->section
       && info->section->flags & SEC_CODE)
      || !info->section)
    mstate = MAP_INSN;

  if (info->symtab_size == 0
      || bfd_asymbol_flavour (*info->symtab) != bfd_target_elf_flavour)
    return mstate;

  /* Reset the last_map_symbol if we start to dump a new section.  */
  if (memaddr <= 0)
    last_map_symbol = -1;

  /* If the last stop offset is different from the current one, then
     don't use the last_map_symbol to search.  We usually reset the
     info->stop_offset when handling a new section.  */
  from_last_map_symbol = (last_map_symbol >= 0
			  && info->stop_offset == last_stop_offset);

  /* Start scanning at the start of the function, or wherever
     we finished last time.  */
  n = info->symtab_pos + 1;
  if (from_last_map_symbol && n >= last_map_symbol)
    n = last_map_symbol;

  /* Find the suitable mapping symbol to dump.  */
  for (; n < info->symtab_size; n++)
    {
      bfd_vma addr = bfd_asymbol_value (info->symtab[n]);
      /* We have searched all possible symbols in the range.  */
      if (addr > memaddr)
	break;
      if (riscv_get_map_state (n, &mstate, info))
	{
	  symbol = n;
	  found = true;
	  /* Do not stop searching, in case there are some mapping
	     symbols have the same value, but have different names.
	     Use the last one.  */
	}
    }

  /* We can not find the suitable mapping symbol above.  Therefore, we
     look forwards and try to find it again, but don't go pass the start
     of the section.  Otherwise a data section without mapping symbols
     can pick up a text mapping symbol of a preceeding section.  */
  if (!found)
    {
      n = info->symtab_pos;
      if (from_last_map_symbol && n >= last_map_symbol)
	n = last_map_symbol;

      for (; n >= 0; n--)
	{
	  bfd_vma addr = bfd_asymbol_value (info->symtab[n]);
	  /* We have searched all possible symbols in the range.  */
	  if (addr < (info->section ? info->section->vma : 0))
	    break;
	  /* Stop searching once we find the closed mapping symbol.  */
	  if (riscv_get_map_state (n, &mstate, info))
	    {
	      symbol = n;
	      found = true;
	      break;
	    }
	}
    }

  if (found)
    {
      /* Find the next mapping symbol to determine the boundary of this mapping
	 symbol.  */

      bool found_next = false;
      /* Try to found next mapping symbol.  */
      for (n = symbol + 1; n < info->symtab_size; n++)
	{
	  if (info->symtab[symbol]->section != info->symtab[n]->section)
	    continue;

	  bfd_vma addr = bfd_asymbol_value (info->symtab[n]);
	  const char *sym_name = bfd_asymbol_name(info->symtab[n]);
	  if (sym_name[0] == '$' && (sym_name[1] == 'x' || sym_name[1] == 'd'))
	    {
	      /* The next mapping symbol has been found, and it represents the
		 boundary of this mapping symbol.  */
	      found_next = true;
	      last_map_symbol_boundary = addr;
	      break;
	    }
	}

      /* No further mapping symbol has been found, indicating that the boundary
	 of the current mapping symbol is the end of this section.  */
      if (!found_next)
	last_map_symbol_boundary = info->section->vma + info->section->size;
    }

  /* Save the information for next use.  */
  last_map_symbol = symbol;
  last_stop_offset = info->stop_offset;

  return mstate;
}

/* Decide which data size we should print.  */

static bfd_vma
riscv_data_length (bfd_vma memaddr,
		   disassemble_info *info)
{
  bfd_vma length;
  bool found = false;

  length = 4;
  if (info->symtab_size != 0
      && bfd_asymbol_flavour (*info->symtab) == bfd_target_elf_flavour
      && last_map_symbol >= 0)
    {
      int n;
      enum riscv_seg_mstate m = MAP_NONE;
      for (n = last_map_symbol + 1; n < info->symtab_size; n++)
	{
	  bfd_vma addr = bfd_asymbol_value (info->symtab[n]);
	  if (addr > memaddr
	      && riscv_get_map_state (n, &m, info))
	    {
	      if (addr - memaddr < length)
		length = addr - memaddr;
	      found = true;
	      break;
	    }
	}
    }
  if (!found)
    {
      /* Do not set the length which exceeds the section size.  */
      bfd_vma offset = info->section->vma + info->section->size;
      offset -= memaddr;
      length = (offset < length) ? offset : length;
    }
  length = length == 3 ? 2 : length;
  return length;
}

/* Dump the data contents.  */

static int
riscv_disassemble_data (bfd_vma memaddr ATTRIBUTE_UNUSED,
			insn_t data,
			const bfd_byte *packet ATTRIBUTE_UNUSED,
			disassemble_info *info)
{
  info->display_endian = info->endian;

  switch (info->bytes_per_chunk)
    {
    case 1:
      info->bytes_per_line = 6;
      (*info->fprintf_styled_func)
	(info->stream, dis_style_assembler_directive, ".byte");
      (*info->fprintf_styled_func) (info->stream, dis_style_text, "\t");
      (*info->fprintf_styled_func) (info->stream, dis_style_immediate,
				    "0x%02x", (unsigned)data);
      break;
    case 2:
      info->bytes_per_line = 8;
      (*info->fprintf_styled_func)
	(info->stream, dis_style_assembler_directive, ".short");
      (*info->fprintf_styled_func) (info->stream, dis_style_text, "\t");
      (*info->fprintf_styled_func)
	(info->stream, dis_style_immediate, "0x%04x", (unsigned) data);
      break;
    case 4:
      info->bytes_per_line = 8;
      (*info->fprintf_styled_func)
	(info->stream, dis_style_assembler_directive, ".word");
      (*info->fprintf_styled_func) (info->stream, dis_style_text, "\t");
      (*info->fprintf_styled_func)
	(info->stream, dis_style_immediate, "0x%08lx",
	 (unsigned long) data);
      break;
    case 8:
      info->bytes_per_line = 8;
      (*info->fprintf_styled_func)
	(info->stream, dis_style_assembler_directive, ".dword");
      (*info->fprintf_styled_func) (info->stream, dis_style_text, "\t");
      (*info->fprintf_styled_func)
	(info->stream, dis_style_immediate, "0x%016llx",
	 (unsigned long long) data);
      break;
    default:
      abort ();
    }
  return info->bytes_per_chunk;
}

static bool
riscv_init_disasm_info (struct disassemble_info *info)
{
  int i;

  struct riscv_private_data *pd =
	xcalloc (1, sizeof (struct riscv_private_data));
  pd->gp = 0;
  pd->print_addr = 0;
  for (i = 0; i < (int) ARRAY_SIZE (pd->hi_addr); i++)
    pd->hi_addr[i] = -1;
  pd->to_print_addr = false;
  pd->has_gp = false;

  for (i = 0; i < info->symtab_size; i++)
    {
      asymbol *sym = info->symtab[i];
      if (strcmp (bfd_asymbol_name (sym), RISCV_GP_SYMBOL) == 0)
	{
	  pd->gp = bfd_asymbol_value (sym);
	  pd->has_gp = true;
	}
    }

  info->private_data = pd;
  return true;
}

int
print_insn_riscv (bfd_vma memaddr, struct disassemble_info *info)
{
  bfd_byte packet[RISCV_MAX_INSN_LEN];
  insn_t insn = 0;
  bfd_vma dump_size;
  int status;
  enum riscv_seg_mstate mstate;
  int (*riscv_disassembler) (bfd_vma, insn_t, const bfd_byte *,
			     struct disassemble_info *);

  if (info->disassembler_options != NULL)
    {
      parse_riscv_dis_options (info->disassembler_options);
      /* Avoid repeatedly parsing the options.  */
      info->disassembler_options = NULL;
    }
  else if (riscv_gpr_names == NULL)
    set_default_riscv_dis_options ();

  if (info->private_data == NULL && !riscv_init_disasm_info (info))
    return -1;

  mstate = riscv_search_mapping_symbol (memaddr, info);
  /* Save the last mapping state.  */
  last_map_state = mstate;

  /* Set the size to dump.  */
  if (mstate == MAP_DATA
      && (info->flags & DISASSEMBLE_DATA) == 0)
    {
      dump_size = riscv_data_length (memaddr, info);
      info->bytes_per_chunk = dump_size;
      riscv_disassembler = riscv_disassemble_data;
    }
  else
    {
      /* Get the first 2-bytes to check the lenghth of instruction.  */
      status = (*info->read_memory_func) (memaddr, packet, 2, info);
      if (status != 0)
	{
	  (*info->memory_error_func) (status, memaddr, info);
	  return -1;
	}
      insn = (insn_t) bfd_getl16 (packet);
      dump_size = riscv_insn_length (insn);
      riscv_disassembler = riscv_disassemble_insn;
    }

  /* Fetch the instruction to dump.  */
  status = (*info->read_memory_func) (memaddr, packet, dump_size, info);
  if (status != 0)
    {
      (*info->memory_error_func) (status, memaddr, info);
      return -1;
    }
  insn = (insn_t) bfd_get_bits (packet, dump_size * 8, false);

  return (*riscv_disassembler) (memaddr, insn, packet, info);
}

disassembler_ftype
riscv_get_disassembler (bfd *abfd)
{
  const char *default_arch = "rv64gc";

  if (abfd && bfd_get_flavour (abfd) == bfd_target_elf_flavour)
    {
      const char *sec_name = get_elf_backend_data (abfd)->obj_attrs_section;
      if (bfd_get_section_by_name (abfd, sec_name) != NULL)
	{
	  obj_attribute *attr = elf_known_obj_attributes_proc (abfd);
	  unsigned int Tag_a = Tag_RISCV_priv_spec;
	  unsigned int Tag_b = Tag_RISCV_priv_spec_minor;
	  unsigned int Tag_c = Tag_RISCV_priv_spec_revision;
	  riscv_get_priv_spec_class_from_numbers (attr[Tag_a].i,
						  attr[Tag_b].i,
						  attr[Tag_c].i,
						  &default_priv_spec);
	  default_arch = attr[Tag_RISCV_arch].s;
	}
    }

  riscv_release_subset_list (&riscv_subsets);
  riscv_parse_subset (&riscv_rps_dis, default_arch);
  return print_insn_riscv;
}

/* Prevent use of the fake labels that are generated as part of the DWARF
   and for relaxable relocations in the assembler.  */

bool
riscv_symbol_is_valid (asymbol * sym,
                       struct disassemble_info * info ATTRIBUTE_UNUSED)
{
  const char * name;

  if (sym == NULL)
    return false;

  name = bfd_asymbol_name (sym);

  return (strcmp (name, RISCV_FAKE_LABEL_NAME) != 0
	  && !riscv_elf_is_mapping_symbols (name));
}


/* Indices into option argument vector for options accepting an argument.
   Use RISCV_OPTION_ARG_NONE for options accepting no argument.  */

typedef enum
{
  RISCV_OPTION_ARG_NONE = -1,
  RISCV_OPTION_ARG_PRIV_SPEC,

  RISCV_OPTION_ARG_COUNT
} riscv_option_arg_t;

/* Valid RISCV disassembler options.  */

static struct
{
  const char *name;
  const char *description;
  riscv_option_arg_t arg;
} riscv_options[] =
{
  { "numeric",
    N_("Print numeric register names, rather than ABI names."),
    RISCV_OPTION_ARG_NONE },
  { "no-aliases",
    N_("Disassemble only into canonical instructions."),
    RISCV_OPTION_ARG_NONE },
  { "priv-spec=",
    N_("Print the CSR according to the chosen privilege spec."),
    RISCV_OPTION_ARG_PRIV_SPEC }
};

/* Build the structure representing valid RISCV disassembler options.
   This is done dynamically for maintenance ease purpose; a static
   initializer would be unreadable.  */

const disasm_options_and_args_t *
disassembler_options_riscv (void)
{
  static disasm_options_and_args_t *opts_and_args;

  if (opts_and_args == NULL)
    {
      size_t num_options = ARRAY_SIZE (riscv_options);
      size_t num_args = RISCV_OPTION_ARG_COUNT;
      disasm_option_arg_t *args;
      disasm_options_t *opts;
      size_t i, priv_spec_count;

      args = XNEWVEC (disasm_option_arg_t, num_args + 1);

      args[RISCV_OPTION_ARG_PRIV_SPEC].name = "SPEC";
      priv_spec_count = PRIV_SPEC_CLASS_DRAFT - PRIV_SPEC_CLASS_NONE - 1;
      args[RISCV_OPTION_ARG_PRIV_SPEC].values
        = XNEWVEC (const char *, priv_spec_count + 1);
      for (i = 0; i < priv_spec_count; i++)
	args[RISCV_OPTION_ARG_PRIV_SPEC].values[i]
          = riscv_priv_specs[i].name;
      /* The array we return must be NULL terminated.  */
      args[RISCV_OPTION_ARG_PRIV_SPEC].values[i] = NULL;

      /* The array we return must be NULL terminated.  */
      args[num_args].name = NULL;
      args[num_args].values = NULL;

      opts_and_args = XNEW (disasm_options_and_args_t);
      opts_and_args->args = args;

      opts = &opts_and_args->options;
      opts->name = XNEWVEC (const char *, num_options + 1);
      opts->description = XNEWVEC (const char *, num_options + 1);
      opts->arg = XNEWVEC (const disasm_option_arg_t *, num_options + 1);
      for (i = 0; i < num_options; i++)
	{
	  opts->name[i] = riscv_options[i].name;
	  opts->description[i] = _(riscv_options[i].description);
	  if (riscv_options[i].arg != RISCV_OPTION_ARG_NONE)
	    opts->arg[i] = &args[riscv_options[i].arg];
	  else
	    opts->arg[i] = NULL;
	}
      /* The array we return must be NULL terminated.  */
      opts->name[i] = NULL;
      opts->description[i] = NULL;
      opts->arg[i] = NULL;
    }

  return opts_and_args;
}

void
print_riscv_disassembler_options (FILE *stream)
{
  const disasm_options_and_args_t *opts_and_args;
  const disasm_option_arg_t *args;
  const disasm_options_t *opts;
  size_t max_len = 0;
  size_t i;
  size_t j;

  opts_and_args = disassembler_options_riscv ();
  opts = &opts_and_args->options;
  args = opts_and_args->args;

  fprintf (stream, _("\n\
The following RISC-V specific disassembler options are supported for use\n\
with the -M switch (multiple options should be separated by commas):\n"));
  fprintf (stream, "\n");

  /* Compute the length of the longest option name.  */
  for (i = 0; opts->name[i] != NULL; i++)
    {
      size_t len = strlen (opts->name[i]);

      if (opts->arg[i] != NULL)
	len += strlen (opts->arg[i]->name);
      if (max_len < len)
	max_len = len;
    }

  for (i = 0, max_len++; opts->name[i] != NULL; i++)
    {
      fprintf (stream, "  %s", opts->name[i]);
      if (opts->arg[i] != NULL)
	fprintf (stream, "%s", opts->arg[i]->name);
      if (opts->description[i] != NULL)
	{
	  size_t len = strlen (opts->name[i]);

	  if (opts->arg != NULL && opts->arg[i] != NULL)
	    len += strlen (opts->arg[i]->name);
	  fprintf (stream, "%*c %s", (int) (max_len - len), ' ',
                   opts->description[i]);
	}
      fprintf (stream, "\n");
    }

  for (i = 0; args[i].name != NULL; i++)
    {
      if (args[i].values == NULL)
	continue;
      fprintf (stream, _("\n\
  For the options above, the following values are supported for \"%s\":\n   "),
	       args[i].name);
      for (j = 0; args[i].values[j] != NULL; j++)
	fprintf (stream, " %s", args[i].values[j]);
      fprintf (stream, _("\n"));
    }

  fprintf (stream, _("\n"));
}

void disassemble_free_riscv (struct disassemble_info *info ATTRIBUTE_UNUSED)
{
  riscv_release_subset_list (&riscv_subsets);
}
