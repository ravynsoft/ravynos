/* tc-loongarch.c -- Assemble for the LoongArch ISA

   Copyright (C) 2021-2023 Free Software Foundation, Inc.
   Contributed by Loongson Ltd.

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
   along with this program; see the file COPYING3.  If not,
   see <http://www.gnu.org/licenses/>.  */

#include "as.h"
#include "subsegs.h"
#include "dw2gencfi.h"
#include "loongarch-lex.h"
#include "elf/loongarch.h"
#include "opcode/loongarch.h"
#include "obj-elf.h"
#include "bfd/elfxx-loongarch.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

/* All information about an instruction during assemble.  */
struct loongarch_cl_insn
{
  /* First split string.  */
  const char *name;
  const char *arg_strs[MAX_ARG_NUM_PLUS_2];
  size_t arg_num;

  /* Second analyze name_str and each actual args string to match the insn
     in 'loongarch-opc.c'. And actual args may need be relocated.
     We get length of insn.  If 'insn_length == 0 && insn_mo->macro != NULL',
     it's a macro insntruction and we call 'md_assemble' recursively
     after expanding it.  */
  int match_now;
  int all_match;

  const struct loongarch_opcode *insn;
  size_t insn_length;

  offsetT args[MAX_ARG_NUM_PLUS_2];
  struct reloc_info reloc_info[MAX_RELOC_NUMBER_A_INSN];
  size_t reloc_num;

  /* For relax reserved.  We not support relax now.
     'insn_length < relax_max_length' means need to relax.
     And 'insn_length == relax_max_length' means no need to relax.  */
  size_t relax_max_length;
  relax_substateT subtype;

  /* Then we get the binary representation of insn
     and write it in to section.  */
  insn_t insn_bin;

  /* The frag that contains the instruction.  */
  struct frag *frag;
  /* The offset into FRAG of the first instruction byte.  */
  long where;
  /* The relocs associated with the instruction, if any.  */
  fixS *fixp[MAX_RELOC_NUMBER_A_INSN];
  long macro_id;
};

#ifndef DEFAULT_ARCH
#define DEFAULT_ARCH "loongarch64"
#endif

/* This array holds the chars that always start a comment.  If the
   pre-processor is disabled, these aren't very useful.  */
const char comment_chars[] = "#";

/* This array holds the chars that only start a comment at the beginning of
   a line.  If the line seems to have the form '# 123 filename'
   .line and .file directives will appear in the pre-processed output.  */
/* Note that input_file.c hand checks for '#' at the beginning of the
   first line of the input file.  This is because the compiler outputs
   #NO_APP at the beginning of its output.  */
/* Also note that C style comments are always supported.  */
const char line_comment_chars[] = "#";

/* This array holds machine specific line separator characters.  */
const char line_separator_chars[] = ";";

/* Chars that can be used to separate mant from exp in floating point nums.  */
const char EXP_CHARS[] = "eE";

/* Chars that mean this number is a floating point constant.  */
/* As in 0f12.456.  */
/* or    0d1.2345e12.  */
const char FLT_CHARS[] = "rRsSfFdDxXpP";

const char *md_shortopts = "O::g::G:";

static const char default_arch[] = DEFAULT_ARCH;

enum options
{
  OPTION_IGNORE = OPTION_MD_BASE,

  OPTION_ABI,
  OPTION_FLOAT_ABI,

  OPTION_FLOAT_ISA,

  OPTION_LA_LOCAL_WITH_ABS,
  OPTION_LA_GLOBAL_WITH_PCREL,
  OPTION_LA_GLOBAL_WITH_ABS,
  OPTION_RELAX,
  OPTION_NO_RELAX,

  OPTION_END_OF_ENUM,
};

struct option md_longopts[] =
{
  { "mabi", required_argument, NULL, OPTION_ABI },

  { "mfpu", required_argument, NULL, OPTION_FLOAT_ISA },

  { "mla-local-with-abs", no_argument, NULL, OPTION_LA_LOCAL_WITH_ABS },
  { "mla-global-with-pcrel", no_argument, NULL, OPTION_LA_GLOBAL_WITH_PCREL },
  { "mla-global-with-abs", no_argument, NULL, OPTION_LA_GLOBAL_WITH_ABS },

  { "mrelax", no_argument, NULL, OPTION_RELAX },
  { "mno-relax", no_argument, NULL, OPTION_NO_RELAX },

  { NULL, no_argument, NULL, 0 }
};

size_t md_longopts_size = sizeof (md_longopts);

int
md_parse_option (int c, const char *arg)
{
  int ret = 1;
  char lp64[256] = "";
  char ilp32[256] = "";
  unsigned char *suf = (unsigned char *)arg;

  lp64['s'] = lp64['S'] = EF_LOONGARCH_ABI_SOFT_FLOAT;
  lp64['f'] = lp64['F'] = EF_LOONGARCH_ABI_SINGLE_FLOAT;
  lp64['d'] = lp64['D'] = EF_LOONGARCH_ABI_DOUBLE_FLOAT;

  ilp32['s'] = ilp32['S'] = EF_LOONGARCH_ABI_SOFT_FLOAT;
  ilp32['f'] = ilp32['F'] = EF_LOONGARCH_ABI_SINGLE_FLOAT;
  ilp32['d'] = ilp32['D'] = EF_LOONGARCH_ABI_DOUBLE_FLOAT;

  switch (c)
    {
    case OPTION_ABI:
      if (strncasecmp (arg, "lp64", 4) == 0 && lp64[suf[4]] != 0)
	{
	  LARCH_opts.ase_ilp32 = 1;
	  LARCH_opts.ase_lp64 = 1;
	  LARCH_opts.ase_lsx = 1;
	  LARCH_opts.ase_lasx = 1;
	  LARCH_opts.ase_lvz = 1;
	  LARCH_opts.ase_lbt = 1;
	  LARCH_opts.ase_abi = lp64[suf[4]];
	}
      else if (strncasecmp (arg, "ilp32", 5) == 0 && ilp32[suf[5]] != 0)
	{
	  LARCH_opts.ase_abi = ilp32[suf[5]];
	  LARCH_opts.ase_ilp32 = 1;
	}
      else
	ret = 0;
      break;

    case OPTION_FLOAT_ISA:
      if (strcasecmp (arg, "soft") == 0)
	LARCH_opts.ase_nf = 1;
      else if (strcasecmp (arg, "single") == 0)
	LARCH_opts.ase_sf = 1;
      else if (strcasecmp (arg, "double") == 0)
	{
	  LARCH_opts.ase_sf = 1;
	  LARCH_opts.ase_df = 1;
	}
      else
	ret = 0;
      break;

    case OPTION_LA_LOCAL_WITH_ABS:
      LARCH_opts.ase_labs = 1;
      break;

    case OPTION_LA_GLOBAL_WITH_PCREL:
      LARCH_opts.ase_gpcr = 1;
      break;

    case OPTION_LA_GLOBAL_WITH_ABS:
      LARCH_opts.ase_gabs = 1;
      break;

    case OPTION_RELAX:
      LARCH_opts.relax = 1;
      break;

    case OPTION_NO_RELAX:
      LARCH_opts.relax = 0;
      break;

    case OPTION_IGNORE:
      break;

    default:
      ret = 0;
      break;
    }
  return ret;
}

static const char *const *r_abi_names = NULL;
static const char *const *f_abi_names = NULL;
static struct htab *r_htab = NULL;
static struct htab *r_deprecated_htab = NULL;
static struct htab *f_htab = NULL;
static struct htab *f_deprecated_htab = NULL;
static struct htab *fc_htab = NULL;
static struct htab *fcn_htab = NULL;
static struct htab *c_htab = NULL;
static struct htab *cr_htab = NULL;
static struct htab *v_htab = NULL;
static struct htab *x_htab = NULL;

void
loongarch_after_parse_args ()
{
  /* Set default ABI/ISA LP64D.  */
  if (!LARCH_opts.ase_ilp32)
    {
      if (strcmp (default_arch, "loongarch64") == 0)
	{
	  LARCH_opts.ase_abi = EF_LOONGARCH_ABI_DOUBLE_FLOAT;
	  LARCH_opts.ase_ilp32 = 1;
	  LARCH_opts.ase_lp64 = 1;
	  LARCH_opts.ase_lsx = 1;
	  LARCH_opts.ase_lasx = 1;
	  LARCH_opts.ase_lvz = 1;
	  LARCH_opts.ase_lbt = 1;
	}
      else if (strcmp (default_arch, "loongarch32") == 0)
	{
	  LARCH_opts.ase_abi = EF_LOONGARCH_ABI_DOUBLE_FLOAT;
	  LARCH_opts.ase_ilp32 = 1;
	}
      else
	as_bad ("unknown default architecture `%s'", default_arch);
    }

  LARCH_opts.ase_abi |= EF_LOONGARCH_OBJABI_V1;
  /* Set default ISA double-float.  */
  if (!LARCH_opts.ase_nf
      && !LARCH_opts.ase_sf
      && !LARCH_opts.ase_df)
    {
      LARCH_opts.ase_sf = 1;
      LARCH_opts.ase_df = 1;
    }

  size_t i;

  assert(LARCH_opts.ase_ilp32);

  /* Init ilp32/lp64 registers names.  */
  if (!r_htab)
    r_htab = str_htab_create (), str_hash_insert (r_htab, "", 0, 0);
  if (!r_deprecated_htab)
    r_deprecated_htab = str_htab_create (),
			str_hash_insert (r_deprecated_htab, "", 0, 0);

  r_abi_names = loongarch_r_normal_name;
  for (i = 0; i < ARRAY_SIZE (loongarch_r_normal_name); i++)
    str_hash_insert (r_htab, loongarch_r_normal_name[i], (void *) (i + 1), 0);

  if (!cr_htab)
    cr_htab = str_htab_create (), str_hash_insert (cr_htab, "", 0, 0);

  for (i = 0; i < ARRAY_SIZE (loongarch_cr_normal_name); i++)
    str_hash_insert (cr_htab, loongarch_cr_normal_name[i], (void *) (i + 1), 0);

  /* Init single/double float registers names.  */
  if (LARCH_opts.ase_sf || LARCH_opts.ase_df)
    {
      if (!f_htab)
	f_htab = str_htab_create (), str_hash_insert (f_htab, "", 0, 0);
      if (!f_deprecated_htab)
	f_deprecated_htab = str_htab_create (),
			    str_hash_insert (f_deprecated_htab, "", 0, 0);

      f_abi_names = loongarch_f_normal_name;
      for (i = 0; i < ARRAY_SIZE (loongarch_f_normal_name); i++)
	str_hash_insert (f_htab, loongarch_f_normal_name[i], (void *) (i + 1),
			 0);

      if (!fc_htab)
	fc_htab = str_htab_create (), str_hash_insert (fc_htab, "", 0, 0);

      for (i = 0; i < ARRAY_SIZE (loongarch_fc_normal_name); i++)
	str_hash_insert (fc_htab, loongarch_fc_normal_name[i], (void *) (i + 1),
			 0);

      if (!fcn_htab)
	fcn_htab = str_htab_create (), str_hash_insert (fcn_htab, "", 0, 0);

      for (i = 0; i < ARRAY_SIZE (loongarch_fc_numeric_name); i++)
	str_hash_insert (fcn_htab, loongarch_fc_numeric_name[i], (void *) (i + 1),
			 0);

      if (!c_htab)
	c_htab = str_htab_create (), str_hash_insert (c_htab, "", 0, 0);

      for (i = 0; i < ARRAY_SIZE (loongarch_c_normal_name); i++)
	str_hash_insert (c_htab, loongarch_c_normal_name[i], (void *) (i + 1),
			 0);

    }

  /* Init lsx registers names.  */
  if (LARCH_opts.ase_lsx)
    {
      if (!v_htab)
	v_htab = str_htab_create (), str_hash_insert (v_htab, "", 0, 0);
      for (i = 0; i < ARRAY_SIZE (loongarch_v_normal_name); i++)
	str_hash_insert (v_htab, loongarch_v_normal_name[i], (void *) (i + 1),
			 0);
    }

  /* Init lasx registers names.  */
  if (LARCH_opts.ase_lasx)
    {
      if (!x_htab)
	x_htab = str_htab_create (), str_hash_insert (x_htab, "", 0, 0);
      for (i = 0; i < ARRAY_SIZE (loongarch_x_normal_name); i++)
	str_hash_insert (x_htab, loongarch_x_normal_name[i], (void *) (i + 1),
			 0);
    }

  /* Init lp64 registers alias.  */
  if (LARCH_opts.ase_lp64)
    {
      r_abi_names = loongarch_r_lp64_name;
      for (i = 0; i < ARRAY_SIZE (loongarch_r_lp64_name); i++)
	str_hash_insert (r_htab, loongarch_r_lp64_name[i], (void *) (i + 1),
			 0);
      for (i = 0; i < ARRAY_SIZE (loongarch_r_lp64_name_deprecated); i++)
	str_hash_insert (r_deprecated_htab, loongarch_r_lp64_name_deprecated[i],
			 (void *) (i + 1), 0);
    }

  /* Init float-lp64 registers alias */
  if ((LARCH_opts.ase_sf || LARCH_opts.ase_df) && LARCH_opts.ase_lp64)
    {
      f_abi_names = loongarch_f_lp64_name;
      for (i = 0; i < ARRAY_SIZE (loongarch_f_lp64_name); i++)
	str_hash_insert (f_htab, loongarch_f_lp64_name[i],
			 (void *) (i + 1), 0);
      for (i = 0; i < ARRAY_SIZE (loongarch_f_lp64_name_deprecated); i++)
	str_hash_insert (f_deprecated_htab, loongarch_f_lp64_name_deprecated[i],
			 (void *) (i + 1), 0);
    }
}

const char *
loongarch_target_format ()
{
  return LARCH_opts.ase_lp64 ? "elf64-loongarch" : "elf32-loongarch";
}

void
md_begin ()
{
  const struct loongarch_opcode *it;
  struct loongarch_ase *ase;
  for (ase = loongarch_ASEs; ase->enabled; ase++)
    for (it = ase->opcodes; it->name; it++)
      {
	if (loongarch_check_format (it->format) != 0)
	  as_fatal (_("insn name: %s\tformat: %s\tsyntax error"),
		    it->name, it->format);
	if (it->mask == 0 && it->macro == 0)
	  as_fatal (_("insn name: %s\nformat: %s\nwe want macro but "
		      "macro is NULL"),
		    it->name, it->format);
	if (it->macro
	    && loongarch_check_macro (it->format, it->macro) != 0)
	  as_fatal (_("insn name: %s\nformat: %s\nmacro: %s\tsyntax error"),
		    it->name, it->format, it->macro);
      }

  /* FIXME: expressionS use 'offsetT' as constant,
   * we want this is 64-bit type.  */
  assert (8 <= sizeof (offsetT));
}

unsigned long
loongarch_mach (void)
{
  return LARCH_opts.ase_lp64 ? bfd_mach_loongarch64 : bfd_mach_loongarch32;
}

static const expressionS const_0 = { .X_op = O_constant, .X_add_number = 0 };

static void
s_loongarch_align (int arg)
{
  const char *t = input_line_pointer;
  while (!is_end_of_line[(unsigned char) *t] && *t != ',')
    ++t;
  if (*t == ',')
    s_align_ptwo (arg);
  else
    s_align_ptwo (0);
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
      as_bad (_("Unsupported use of %s"),
	      (bytes == 8 ? ".dtpreldword" : ".dtprelword"));
      ignore_rest_of_line ();
    }

  p = frag_more (bytes);
  md_number_to_chars (p, 0, bytes);
  fix_new_exp (frag_now, p - frag_now->fr_literal, bytes, &ex, FALSE,
	       (bytes == 8
		? BFD_RELOC_LARCH_TLS_DTPREL64
		: BFD_RELOC_LARCH_TLS_DTPREL32));

  demand_empty_rest_of_line ();
}

static const pseudo_typeS loongarch_pseudo_table[] =
{
  { "align", s_loongarch_align, -4 },
  { "dword", cons, 8 },
  { "word", cons, 4 },
  { "half", cons, 2 },
  { "dtprelword", s_dtprel, 4 },
  { "dtpreldword", s_dtprel, 8 },
  { NULL, NULL, 0 },
};

void
loongarch_pop_insert (void)
{
  pop_insert (loongarch_pseudo_table);
}

#define INTERNAL_LABEL_SPECIAL 10
static unsigned long internal_label_count[INTERNAL_LABEL_SPECIAL] = { 0 };

static const char *
loongarch_internal_label_name (unsigned long label, int augend)
{
  static char symbol_name_build[24];
  unsigned long want_label;
  char *p;

  want_label = internal_label_count[label] + augend;

  p = symbol_name_build;
#ifdef LOCAL_LABEL_PREFIX
  *p++ = LOCAL_LABEL_PREFIX;
#endif
  *p++ = 'L';
  for (; label; label /= 10)
    *p++ = label % 10 + '0';
  /* Make sure internal label never belong to normal label namespace.  */
  *p++ = ':';
  for (; want_label; want_label /= 10)
    *p++ = want_label % 10 + '0';
  *p++ = '\0';
  return symbol_name_build;
}

static void
setup_internal_label_here (unsigned long label)
{
  assert (label < INTERNAL_LABEL_SPECIAL);
  internal_label_count[label]++;
  colon (loongarch_internal_label_name (label, 0));
}

void
get_internal_label (expressionS *label_expr, unsigned long label,
		    int augend /* 0 for previous, 1 for next.  */)
{
  assert (label < INTERNAL_LABEL_SPECIAL);
    as_fatal (_("internal error: we have no internal label yet"));
  label_expr->X_op = O_symbol;
  label_expr->X_add_symbol =
    symbol_find_or_make (loongarch_internal_label_name (label, augend));
  label_expr->X_add_number = 0;
}

static int
is_internal_label (const char *c_str)
{
  do
    {
      if (*c_str != ':')
	break;
      c_str++;
      if (!('0' <= *c_str && *c_str <= '9'))
	break;
      while ('0' <= *c_str && *c_str <= '9')
	c_str++;
      if (*c_str != 'b' && *c_str != 'f')
	break;
      c_str++;
      return *c_str == '\0';
    }
  while (0);
  return 0;
}

static int
is_label (const char *c_str)
{
  if (is_internal_label (c_str))
    return 1;
  else if ('0' <= *c_str && *c_str <= '9')
    {
      /* [0-9]+[bf]  */
      while ('0' <= *c_str && *c_str <= '9')
	c_str++;
      return *c_str == 'b' || *c_str == 'f';
    }
  else if (is_name_beginner (*c_str))
    {
      /* [a-zA-Z\._\$][0-9a-zA-Z\._\$]*  */
      c_str++;
      while (is_part_of_name (*c_str))
	c_str++;
      return *c_str == '\0';
    }
  else
    return 0;
}

static int
is_label_with_addend (const char *c_str)
{
  if (is_internal_label (c_str))
    return 1;
  else if ('0' <= *c_str && *c_str <= '9')
    {
      /* [0-9]+[bf]  */
      while ('0' <= *c_str && *c_str <= '9')
	c_str++;
      if (*c_str == 'b' || *c_str == 'f')
	c_str++;
      else
	return 0;
      return *c_str == '\0'
		       || ((*c_str == '-' || *c_str == '+')
			   && is_unsigned (c_str + 1));
    }
  else if (is_name_beginner (*c_str))
    {
      /* [a-zA-Z\._\$][0-9a-zA-Z\._\$]*  */
      c_str++;
      while (is_part_of_name (*c_str))
	c_str++;
      return *c_str == '\0'
		       || ((*c_str == '-' || *c_str == '+')
			   && is_unsigned (c_str + 1));
    }
  else
    return 0;
}

static int32_t
loongarch_args_parser_can_match_arg_helper (char esc_ch1, char esc_ch2,
					    const char *bit_field,
					    const char *arg, void *context)
{
  struct loongarch_cl_insn *ip = context;
  offsetT imm, ret = 0;
  size_t reloc_num_we_have = MAX_RELOC_NUMBER_A_INSN - ip->reloc_num;
  size_t reloc_num = 0;

  if (!ip->match_now)
    return 0;

  switch (esc_ch1)
    {
    case 'l':
      switch (esc_ch2)
	{
	default:
	  ip->match_now = is_label (arg);
	  if (!ip->match_now && is_label_with_addend (arg))
	    as_fatal (_("This label shouldn't be with addend."));
	  break;
	case 'a':
	  ip->match_now = is_label_with_addend (arg);
	  break;
	}
      break;
    case 's':
    case 'u':
      ip->match_now =
	loongarch_parse_expr (arg, ip->reloc_info + ip->reloc_num,
			      reloc_num_we_have, &reloc_num, &imm) == 0;

      if (!ip->match_now)
	break;

      ret = imm;
      if (reloc_num)
	{
	  bfd_reloc_code_real_type reloc_type = BFD_RELOC_NONE;
	  reloc_num_we_have -= reloc_num;
	  if (reloc_num_we_have == 0)
	    as_fatal (_("expr too huge") /* Want one more reloc.  */);
	  if (esc_ch1 == 'u')
	    {
	      if (strncmp (bit_field, "10:12", strlen ("10:12")) == 0)
		reloc_type = BFD_RELOC_LARCH_SOP_POP_32_U_10_12;
	    }
	  else if (esc_ch1 == 's')
	    {
	      if (strncmp (bit_field, "10:16<<2", strlen ("10:16<<2")) == 0)
		reloc_type = BFD_RELOC_LARCH_SOP_POP_32_S_10_16_S2;
	      else if (strncmp (bit_field, "0:5|10:16<<2",
				strlen ("0:5|10:16<<2")) == 0)
		reloc_type = BFD_RELOC_LARCH_SOP_POP_32_S_0_5_10_16_S2;
	      else if (strncmp (bit_field, "0:10|10:16<<2",
				strlen ("0:10|10:16<<2")) == 0)
		reloc_type = BFD_RELOC_LARCH_SOP_POP_32_S_0_10_10_16_S2;
	      else if (strncmp (bit_field, "10:12", strlen ("10:12")) == 0)
		reloc_type = BFD_RELOC_LARCH_SOP_POP_32_S_10_12;
	      else if (strncmp (bit_field, "5:20", strlen ("5:20")) == 0)
		reloc_type = BFD_RELOC_LARCH_SOP_POP_32_S_5_20;
	      else if (strncmp (bit_field, "10:16", strlen ("10:16")) == 0)
		reloc_type = BFD_RELOC_LARCH_SOP_POP_32_S_10_16;
	      else if (strncmp (bit_field, "10:5", strlen ("10:5")) == 0)
		reloc_type = BFD_RELOC_LARCH_SOP_POP_32_S_10_5;
	    }
	  if (reloc_type == BFD_RELOC_NONE)
	    as_fatal (
		      _("not support reloc bit-field\nfmt: %c%c %s\nargs: %s"),
		      esc_ch1, esc_ch2, bit_field, arg);

	  if (ip->reloc_info[0].type >= BFD_RELOC_LARCH_B16
	      && ip->reloc_info[0].type < BFD_RELOC_LARCH_64_PCREL)
	    {
	      /* As we compact stack-relocs, it is no need for pop operation.
		 But break out until here in order to check the imm field.
		 May be reloc_num > 1 if implement relax?  */
	      ip->reloc_num += reloc_num;
	      reloc_type = ip->reloc_info[0].type;

	      if (LARCH_opts.relax && ip->macro_id
		    && (BFD_RELOC_LARCH_PCALA_HI20 == reloc_type
			|| BFD_RELOC_LARCH_PCALA_LO12 == reloc_type
			|| BFD_RELOC_LARCH_GOT_PC_HI20 == reloc_type
			|| BFD_RELOC_LARCH_GOT_PC_LO12 == reloc_type))
		{
		  ip->reloc_info[ip->reloc_num].type = BFD_RELOC_LARCH_RELAX;
		  ip->reloc_info[ip->reloc_num].value = const_0;
		  ip->reloc_num++;
		}
	      break;
	    }
	  reloc_num++;
	  ip->reloc_num += reloc_num;
	  ip->reloc_info[ip->reloc_num - 1].type = reloc_type;
	  ip->reloc_info[ip->reloc_num - 1].value = const_0;
	}
      break;
    case 'r':
      imm = (intptr_t) str_hash_find (r_htab, arg);
      ip->match_now = 0 < imm;
      ret = imm - 1;
      if (ip->match_now)
	break;
      /* Handle potential usage of deprecated register aliases.  */
      imm = (intptr_t) str_hash_find (r_deprecated_htab, arg);
      ip->match_now = 0 < imm;
      ret = imm - 1;
      if (ip->match_now && !ip->macro_id)
	as_warn (_("register alias %s is deprecated, use %s instead"),
		 arg, r_abi_names[ret]);
      break;
    case 'f':
      switch (esc_ch2)
	{
	case 'c':
	  imm = (intptr_t) str_hash_find (fc_htab, arg);
	  if (0 >= imm)
	    {
	      imm = (intptr_t) str_hash_find (fcn_htab, arg);
	    }
	  break;
	default:
	  imm = (intptr_t) str_hash_find (f_htab, arg);
	}
      ip->match_now = 0 < imm;
      ret = imm - 1;
      if (ip->match_now && !ip->macro_id)
	break;
      /* Handle potential usage of deprecated register aliases.  */
      imm = (intptr_t) str_hash_find (f_deprecated_htab, arg);
      ip->match_now = 0 < imm;
      ret = imm - 1;
      if (ip->match_now)
	as_warn (_("register alias %s is deprecated, use %s instead"),
		 arg, f_abi_names[ret]);
      break;
    case 'c':
      switch (esc_ch2)
	{
	case 'r':
	  imm = (intptr_t) str_hash_find (cr_htab, arg);
	  break;
	default:
	  imm = (intptr_t) str_hash_find (c_htab, arg);
	}
      ip->match_now = 0 < imm;
      ret = imm - 1;
      break;
    case 'v':
      imm = (intptr_t) str_hash_find (v_htab, arg);
      ip->match_now = 0 < imm;
      ret = imm - 1;
      break;
    case 'x':
      imm = (intptr_t) str_hash_find (x_htab, arg);
      ip->match_now = 0 < imm;
      ret = imm - 1;
      break;
    case '\0':
      ip->all_match = ip->match_now;
      ip->insn_length =
	ip->insn->mask ? loongarch_insn_length (ip->insn->match) : 0;
      /* FIXME: now we have no relax insn.  */
      ip->relax_max_length = ip->insn_length;
      break;
    default:
      as_fatal (_("unknown escape"));
    }

  do
    {
      /* Check imm overflow.  */
      int bit_width, bits_needed_s, bits_needed_u;
      char *t;

      if (!ip->match_now)
	break;

      if (0 < reloc_num)
	break;

      bit_width = loongarch_get_bit_field_width (bit_field, &t);

      if (bit_width == -1)
	/* No specify bit width.  */
	break;

      imm = ret;
      if (t[0] == '<' && t[1] == '<')
	{
	  int i = strtol (t += 2, &t, 10), j;
	  for (j = i; 0 < j; j--, imm >>= 1)
	    if (imm & 1)
	      as_fatal (_("require imm low %d bit is 0."), i);
	}

      if (*t == '+')
	imm -= strtol (t, &t, 10);

      bits_needed_s = loongarch_bits_imm_needed (imm, 1);
      bits_needed_u = loongarch_bits_imm_needed (imm, 0);

      if ((esc_ch1 == 's' && bit_width < bits_needed_s)
	  || (esc_ch1 != 's' && bit_width < bits_needed_u))
	/* How to do after we detect overflow.  */
	as_fatal (_("Immediate overflow.\n"
		    "format: %c%c%s\n"
		    "arg: %s"),
		  esc_ch1, esc_ch2, bit_field, arg);
    }
  while (0);

  if (esc_ch1 != '\0')
    {
      ip->args[ip->arg_num] = ret;
      ip->arg_num++;
    }
  return ret;
}

static void
get_loongarch_opcode (struct loongarch_cl_insn *insn)
{
  const struct loongarch_opcode *it;
  struct loongarch_ase *ase;
  for (ase = loongarch_ASEs; ase->enabled; ase++)
    {
      if (!*ase->enabled || (ase->include && !*ase->include)
	  || (ase->exclude && *ase->exclude))
	continue;

      if (!ase->name_hash_entry)
	{
	  ase->name_hash_entry = str_htab_create ();
	  for (it = ase->opcodes; it->name; it++)
	    {
	      if ((!it->include || (it->include && *it->include))
		  && (!it->exclude || (it->exclude && !(*it->exclude)))
		  && !(it->pinfo & INSN_DIS_ALIAS))
		str_hash_insert (ase->name_hash_entry, it->name,
				 (void *) it, 0);
	    }
	}

      if ((it = str_hash_find (ase->name_hash_entry, insn->name)) == NULL)
	continue;

      do
	{
	  insn->insn = it;
	  insn->match_now = 1;
	  insn->all_match = 0;
	  insn->arg_num = 0;
	  insn->reloc_num = 0;
	  insn->insn_bin = (loongarch_foreach_args
			    (it->format, insn->arg_strs,
			     loongarch_args_parser_can_match_arg_helper,
			     insn));
	  if (insn->all_match && !(it->include && !*it->include)
	      && !(it->exclude && *it->exclude))
	    {
	      insn->insn_bin |= it->match;
	      return;
	    }
	  it++;
	}
      while (it->name && strcasecmp (it->name, insn->name) == 0);
    }
}

static int
check_this_insn_before_appending (struct loongarch_cl_insn *ip)
{
  int ret = 0;

  if (strncmp (ip->name, "la.abs", 6) == 0)
    {
      ip->reloc_info[ip->reloc_num].type = BFD_RELOC_LARCH_MARK_LA;
      ip->reloc_info[ip->reloc_num].value = const_0;
      ip->reloc_num++;
    }
  else if (ip->insn->mask == 0xffff8000
	   /* amswap.w  rd, rk, rj  */
	   && ((ip->insn_bin & 0xfff00000) == 0x38600000
	       /* ammax_db.wu  rd, rk, rj  */
	       || (ip->insn_bin & 0xffff0000) == 0x38700000
	       /* ammin_db.wu  rd, rk, rj  */
	       || (ip->insn_bin & 0xffff0000) == 0x38710000))
    {
      /* For AMO insn amswap.[wd], amadd.[wd], etc.  */
      if (ip->args[0] != 0
	  && (ip->args[0] == ip->args[1] || ip->args[0] == ip->args[2]))
	as_fatal (_("AMO insns require rd != base && rd != rt"
		    " when rd isn't $r0"));
    }
  else if ((ip->insn->mask == 0xffe08000
	    /* bstrins.w  rd, rj, msbw, lsbw  */
	    && (ip->insn_bin & 0xffe00000) == 0x00600000)
	   || (ip->insn->mask == 0xffc00000
	       /* bstrins.d  rd, rj, msbd, lsbd  */
	       && (ip->insn_bin & 0xff800000) == 0x00800000))
    {
      /* For bstr(ins|pick).[wd].  */
      if (ip->args[2] < ip->args[3])
	as_fatal (_("bstr(ins|pick).[wd] require msbd >= lsbd"));
    }
  else if (ip->insn->mask != 0 && (ip->insn_bin & 0xfe0003c0) == 0x04000000
	   /* csrxchg  rd, rj, csr_num  */
	   && (strcmp ("csrxchg", ip->name) == 0))
    as_fatal (_("csrxchg require rj != $r0 && rj != $r1"));

  return ret;
}

static void
install_insn (const struct loongarch_cl_insn *insn)
{
  char *f = insn->frag->fr_literal + insn->where;
  if (0 < insn->insn_length)
    md_number_to_chars (f, insn->insn_bin, insn->insn_length);
}

static void
move_insn (struct loongarch_cl_insn *insn, fragS *frag, long where)
{
  size_t i;
  insn->frag = frag;
  insn->where = where;
  for (i = 0; i < insn->reloc_num; i++)
    {
      if (insn->fixp[i])
	{
	  insn->fixp[i]->fx_frag = frag;
	  insn->fixp[i]->fx_where = where;
	}
    }
  install_insn (insn);
}

/* Add INSN to the end of the output.  */
static void
append_fixed_insn (struct loongarch_cl_insn *insn)
{
  char *f = frag_more (insn->insn_length);
  move_insn (insn, frag_now, f - frag_now->fr_literal);
}

static void
append_fixp_and_insn (struct loongarch_cl_insn *ip)
{
  reloc_howto_type *howto;
  bfd_reloc_code_real_type reloc_type;
  struct reloc_info *reloc_info = ip->reloc_info;
  size_t i;

  dwarf2_emit_insn (0);

  for (i = 0; i < ip->reloc_num; i++)
    {
      reloc_type = reloc_info[i].type;
      howto = bfd_reloc_type_lookup (stdoutput, reloc_type);
      if (howto == NULL)
	as_fatal (_("no HOWTO loong relocation number %d"), reloc_type);

      ip->fixp[i] =
	fix_new_exp (ip->frag, ip->where, bfd_get_reloc_size (howto),
		     &reloc_info[i].value, FALSE, reloc_type);
    }

  if (ip->insn_length < ip->relax_max_length)
    as_fatal (_("Internal error: not support relax now"));
  else
    append_fixed_insn (ip);

  /* We need to start a new frag after any instruction that can be
     optimized away or compressed by the linker during relaxation, to prevent
     the assembler from computing static offsets across such an instruction.

     This is necessary to get correct .eh_frame cfa info. If one cfa's two
     symbol is not in the same frag, it will generate relocs to calculate
     symbol subtraction. (gas/dw2gencfi.c:output_cfi_insn:
     if (symbol_get_frag (to) == symbol_get_frag (from)))  */
  if (LARCH_opts.relax
      && (BFD_RELOC_LARCH_PCALA_HI20 == reloc_info[0].type
	  || BFD_RELOC_LARCH_GOT_PC_HI20 == reloc_info[0].type))
    {
      frag_wane (frag_now);
      frag_new (0);
    }
}

/* Ask helper for returning a malloced c_str or NULL.  */
static char *
assember_macro_helper (const char *const args[], void *context_ptr)
{
  struct loongarch_cl_insn *insn = context_ptr;
  char *ret = NULL;
  if ( strcmp (insn->name, "li.w") == 0 || strcmp (insn->name, "li.d") == 0)
    {
      char args_buf[50], insns_buf[200];
      const char *arg_strs[6];
      uint32_t hi32, lo32;

      /* We pay attention to sign extend beacause it is chance of reduce insn.
	 The exception is 12-bit and hi-12-bit unsigned,
	 we need a 'ori' or a 'lu52i.d' accordingly.  */
      char all0_bit_vec, sign_bit_vec, allf_bit_vec, paritial_is_sext_of_prev;

      lo32 = insn->args[1] & 0xffffffff;
      hi32 = insn->args[1] >> 32;

      if (strcmp (insn->name, "li.w") == 0)
	{
	  if (hi32 != 0 && hi32 != 0xffffffff)
	    as_fatal (_("li overflow: hi32:0x%x lo32:0x%x"), hi32, lo32);
	  hi32 = lo32 & 0x80000000 ? 0xffffffff : 0;
	}

      if (strcmp (insn->name, "li.d") == 0 && !LARCH_opts.ase_lp64)
	as_fatal (_("we can't li.d on 32bit-arch"));

      snprintf (args_buf, sizeof (args_buf), "0x%x,0x%x,0x%x,0x%x,%s",
		(hi32 >> 20) & 0xfff, hi32 & 0xfffff, (lo32 >> 12) & 0xfffff,
		lo32 & 0xfff, args[0]);
      loongarch_split_args_by_comma (args_buf, arg_strs);

      all0_bit_vec =
	((((hi32 & 0xfff00000) == 0) << 3) | (((hi32 & 0x000fffff) == 0) << 2)
	 | (((lo32 & 0xfffff000) == 0) << 1) | ((lo32 & 0x00000fff) == 0));
      sign_bit_vec =
	((((hi32 & 0x80000000) != 0) << 3) | (((hi32 & 0x00080000) != 0) << 2)
	 | (((lo32 & 0x80000000) != 0) << 1) | ((lo32 & 0x00000800) != 0));
      allf_bit_vec =
	((((hi32 & 0xfff00000) == 0xfff00000) << 3)
	 | (((hi32 & 0x000fffff) == 0x000fffff) << 2)
	 | (((lo32 & 0xfffff000) == 0xfffff000) << 1)
	 | ((lo32 & 0x00000fff) == 0x00000fff));
      paritial_is_sext_of_prev =
	(all0_bit_vec ^ allf_bit_vec) & (all0_bit_vec ^ (sign_bit_vec << 1));

      static const char *const li_32bit[] =
	{
	  "lu12i.w %5,%3&0x80000?%3-0x100000:%3;ori %5,%5,%4;",
	  "lu12i.w %5,%3&0x80000?%3-0x100000:%3;",
	  "addi.w %5,$r0,%4&0x800?%4-0x1000:%4;",
	  "or %5,$r0,$r0;",
	};
      static const char *const li_hi_32bit[] =
	{
	  "lu32i.d %5,%2&0x80000?%2-0x100000:%2;"
	  "lu52i.d %5,%5,%1&0x800?%1-0x1000:%1;",
	  "lu52i.d %5,%5,%1&0x800?%1-0x1000:%1;",
	  "lu32i.d %5,%2&0x80000?%2-0x100000:%2;",
	  "",
	};
      do
	{
	  insns_buf[0] = '\0';
	  if (paritial_is_sext_of_prev == 0x7)
	    {
	      strcat (insns_buf, "lu52i.d %5,$r0,%1&0x800?%1-0x1000:%1;");
	      break;
	    }
	  if ((all0_bit_vec & 0x3) == 0x2)
	    strcat (insns_buf, "ori %5,$r0,%4;");
	  else
	    strcat (insns_buf, li_32bit[paritial_is_sext_of_prev & 0x3]);
	  strcat (insns_buf, li_hi_32bit[paritial_is_sext_of_prev >> 2]);
	}
      while (0);

      ret = loongarch_expand_macro (insns_buf, arg_strs, NULL, NULL,
				    sizeof (args_buf));
    }

  return ret;
}

/* Accept instructions separated by ';'
 * assuming 'not starting with space and not ending with space' or pass in
 * empty c_str.  */
static void
loongarch_assemble_INSNs (char *str, struct loongarch_cl_insn *ctx)
{
  char *rest;
  size_t len_str = strlen(str);

  for (rest = str; *rest != ';' && *rest != '\0'; rest++);
  if (*rest == ';')
    *rest++ = '\0';

  if (*str == ':')
    {
      str++;
      setup_internal_label_here (strtol (str, &str, 10));
      str++;
    }

  do
    {
      if (*str == '\0')
	break;

      struct loongarch_cl_insn the_one = { 0 };
      the_one.name = str;
      the_one.macro_id = ctx->macro_id;

      for (; *str && *str != ' '; str++)
	;
      if (*str == ' ')
	*str++ = '\0';

      loongarch_split_args_by_comma (str, the_one.arg_strs);
      get_loongarch_opcode (&the_one);

      if (!the_one.all_match)
	{
	  char *ss = loongarch_cat_splited_strs (the_one.arg_strs);
	  as_bad (_("no match insn: %s\t%s"), the_one.name, ss ? ss : "");
	  free(ss);
	  return;
	}

      if (check_this_insn_before_appending (&the_one) != 0)
	break;

      append_fixp_and_insn (&the_one);
      if (the_one.insn_length == 0 && the_one.insn->macro)
	{
	  the_one.macro_id = 1;

	  char *c_str = loongarch_expand_macro (the_one.insn->macro,
						the_one.arg_strs,
						assember_macro_helper,
						&the_one, len_str);
	  loongarch_assemble_INSNs (c_str, &the_one);
	  free (c_str);
	}
    }
  while (0);

  if (*rest != '\0')
    loongarch_assemble_INSNs (rest, ctx);
}

void
md_assemble (char *str)
{
  struct loongarch_cl_insn the_one = { 0 };
  loongarch_assemble_INSNs (str, &the_one);
}

const char *
md_atof (int type, char *litP, int *sizeP)
{
  return ieee_md_atof (type, litP, sizeP, FALSE);
}

void
md_number_to_chars (char *buf, valueT val, int n)
{
  number_to_chars_littleendian (buf, val, n);
}

/* The location from which a PC relative jump should be calculated,
   given a PC relative reloc.  */
long
md_pcrel_from (fixS *fixP ATTRIBUTE_UNUSED)
{
  return 0;
}

static void fix_reloc_insn (fixS *fixP, bfd_vma reloc_val, char *buf)
{
  reloc_howto_type *howto;
  insn_t insn;
  howto = bfd_reloc_type_lookup (stdoutput, fixP->fx_r_type);

  insn = bfd_getl32 (buf);

  if (!loongarch_adjust_reloc_bitsfield (NULL, howto, &reloc_val))
    as_warn_where (fixP->fx_file, fixP->fx_line, "Reloc overflow");

  insn = (insn & (insn_t)howto->src_mask)
    | ((insn & (~(insn_t)howto->dst_mask)) | reloc_val);

  bfd_putl32 (insn, buf);
}

void
md_apply_fix (fixS *fixP, valueT *valP, segT seg ATTRIBUTE_UNUSED)
{
  static int64_t stack_top;
  static int last_reloc_is_sop_push_pcrel_1 = 0;
  int last_reloc_is_sop_push_pcrel = last_reloc_is_sop_push_pcrel_1;
  segT sub_segment;
  last_reloc_is_sop_push_pcrel_1 = 0;

  char *buf = fixP->fx_frag->fr_literal + fixP->fx_where;
  switch (fixP->fx_r_type)
    {
    case BFD_RELOC_LARCH_SOP_PUSH_TLS_TPREL:
    case BFD_RELOC_LARCH_SOP_PUSH_TLS_GD:
    case BFD_RELOC_LARCH_SOP_PUSH_TLS_GOT:
    case BFD_RELOC_LARCH_TLS_LE_HI20:
    case BFD_RELOC_LARCH_TLS_LE_LO12:
    case BFD_RELOC_LARCH_TLS_LE64_LO20:
    case BFD_RELOC_LARCH_TLS_LE64_HI12:
    case BFD_RELOC_LARCH_TLS_IE_PC_HI20:
    case BFD_RELOC_LARCH_TLS_IE_PC_LO12:
    case BFD_RELOC_LARCH_TLS_IE64_PC_LO20:
    case BFD_RELOC_LARCH_TLS_IE64_PC_HI12:
    case BFD_RELOC_LARCH_TLS_IE_HI20:
    case BFD_RELOC_LARCH_TLS_IE_LO12:
    case BFD_RELOC_LARCH_TLS_IE64_LO20:
    case BFD_RELOC_LARCH_TLS_IE64_HI12:
    case BFD_RELOC_LARCH_TLS_LD_PC_HI20:
    case BFD_RELOC_LARCH_TLS_LD_HI20:
    case BFD_RELOC_LARCH_TLS_GD_PC_HI20:
    case BFD_RELOC_LARCH_TLS_GD_HI20:
      /* Add tls lo (got_lo reloc type).  */
      if (fixP->fx_addsy == NULL)
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("Relocation against a constant"));
      S_SET_THREAD_LOCAL (fixP->fx_addsy);
      break;

    case BFD_RELOC_LARCH_SOP_PUSH_PCREL:
      if (fixP->fx_addsy == NULL)
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("Relocation against a constant"));

      last_reloc_is_sop_push_pcrel_1 = 1;
      if (S_GET_SEGMENT (fixP->fx_addsy) == seg)
	stack_top = (S_GET_VALUE (fixP->fx_addsy) + fixP->fx_offset
		     - (fixP->fx_where + fixP->fx_frag->fr_address));
      else
	stack_top = 0;
      break;

    case BFD_RELOC_LARCH_SOP_POP_32_S_10_5:
    case BFD_RELOC_LARCH_SOP_POP_32_S_10_12:
    case BFD_RELOC_LARCH_SOP_POP_32_U_10_12:
    case BFD_RELOC_LARCH_SOP_POP_32_S_10_16:
    case BFD_RELOC_LARCH_SOP_POP_32_S_10_16_S2:
    case BFD_RELOC_LARCH_SOP_POP_32_S_5_20:
    case BFD_RELOC_LARCH_SOP_POP_32_U:
    case BFD_RELOC_LARCH_SOP_POP_32_S_0_5_10_16_S2:
    case BFD_RELOC_LARCH_SOP_POP_32_S_0_10_10_16_S2:
      if (!last_reloc_is_sop_push_pcrel)
	break;

      fix_reloc_insn (fixP, (bfd_vma)stack_top, buf);
      break;

    /* LARCH only has R_LARCH_64/32, not has R_LARCH_24/16/8.
       For BFD_RELOC_64/32, if fx_addsy and fx_subsy not null, wer need
       generate BFD_RELOC_LARCH_ADD64/32 and BFD_RELOC_LARCH_SUB64/32 here.
       Then will parse howto table bfd_reloc_code_real_type to generate
       R_LARCH_ADD64/32 and R_LARCH_SUB64/32 reloc at tc_gen_reloc function.
       If only fx_addsy not null, skip here directly, then generate
       R_LARCH_64/32.

       For BFD_RELOC_24/16/8, if fx_addsy and fx_subsy not null, wer need
       generate BFD_RELOC_LARCH_ADD24/16/8 and BFD_RELOC_LARCH_SUB24/16/8 here.
       Then will parse howto table bfd_reloc_code_real_type to generate
       R_LARCH_ADD24/16/8 and R_LARCH_SUB24/16/8 reloc at tc_gen_reloc
       function. If only fx_addsy not null, we generate
       BFD_RELOC_LARCH_ADD24/16/8 only, then generate R_LARCH_24/16/8.
       To avoid R_LARCH_ADDxx add extra value, we write 0 first
       (use md_number_to_chars (buf, 0, fixP->fx_size)).  */
    case BFD_RELOC_64:
    case BFD_RELOC_32:
      if (fixP->fx_r_type == BFD_RELOC_32
	  && fixP->fx_addsy && fixP->fx_subsy
	  && (sub_segment = S_GET_SEGMENT (fixP->fx_subsy))
	  && strcmp (sub_segment->name, ".eh_frame") == 0
	  && S_GET_VALUE (fixP->fx_subsy)
	  == fixP->fx_frag->fr_address + fixP->fx_where)
	{
	  fixP->fx_r_type = BFD_RELOC_LARCH_32_PCREL;
	  fixP->fx_subsy = NULL;
	  break;
	}

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
	      fixP->fx_r_type = BFD_RELOC_LARCH_ADD64;
	      fixP->fx_next->fx_r_type = BFD_RELOC_LARCH_SUB64;
	      break;
	    case BFD_RELOC_32:
	      fixP->fx_r_type = BFD_RELOC_LARCH_ADD32;
	      fixP->fx_next->fx_r_type = BFD_RELOC_LARCH_SUB32;
	      break;
	    default:
	      break;
	    }

	  md_number_to_chars (buf, 0, fixP->fx_size);
	}

      if (fixP->fx_addsy == NULL)
	{
	  fixP->fx_done = 1;
	  md_number_to_chars (buf, *valP, fixP->fx_size);
	}
      break;

    case BFD_RELOC_24:
    case BFD_RELOC_16:
    case BFD_RELOC_8:
      if (fixP->fx_addsy)
	{
	  fixP->fx_next = xmemdup (fixP, sizeof (*fixP), sizeof (*fixP));
	  fixP->fx_next->fx_addsy = fixP->fx_subsy;
	  fixP->fx_next->fx_subsy = NULL;
	  fixP->fx_next->fx_offset = 0;
	  fixP->fx_subsy = NULL;

	  switch (fixP->fx_r_type)
	    {
	    case BFD_RELOC_24:
	      fixP->fx_r_type = BFD_RELOC_LARCH_ADD24;
	      fixP->fx_next->fx_r_type = BFD_RELOC_LARCH_SUB24;
	      break;
	    case BFD_RELOC_16:
	      fixP->fx_r_type = BFD_RELOC_LARCH_ADD16;
	      fixP->fx_next->fx_r_type = BFD_RELOC_LARCH_SUB16;
	      break;
	    case BFD_RELOC_8:
	      fixP->fx_r_type = BFD_RELOC_LARCH_ADD8;
	      fixP->fx_next->fx_r_type = BFD_RELOC_LARCH_SUB8;
	      break;
	    default:
	      break;
	    }

	  md_number_to_chars (buf, 0, fixP->fx_size);

	  if (fixP->fx_next->fx_addsy == NULL)
	    fixP->fx_next->fx_done = 1;
	}

      if (fixP->fx_addsy == NULL)
	{
	  fixP->fx_done = 1;
	  md_number_to_chars (buf, *valP, fixP->fx_size);
	}
      break;

    case BFD_RELOC_LARCH_CFA:
      if (fixP->fx_addsy && fixP->fx_subsy)
	{
	  fixP->fx_next = xmemdup (fixP, sizeof (*fixP), sizeof (*fixP));
	  fixP->fx_next->fx_addsy = fixP->fx_subsy;
	  fixP->fx_next->fx_subsy = NULL;
	  fixP->fx_next->fx_offset = 0;
	  fixP->fx_subsy = NULL;

	  unsigned int subtype;
	  offsetT loc;
	  subtype = bfd_get_8 (NULL, &((fragS *)
		      (fixP->fx_frag->fr_opcode))->fr_literal[fixP->fx_where]);
	  loc = fixP->fx_frag->fr_fix - (subtype & 7);
	  switch (subtype)
	    {
	    case DW_CFA_advance_loc1:
	      fixP->fx_where = loc + 1;
	      fixP->fx_next->fx_where = loc + 1;
	      fixP->fx_r_type = BFD_RELOC_LARCH_ADD8;
	      fixP->fx_next->fx_r_type = BFD_RELOC_LARCH_SUB8;
	      md_number_to_chars (buf+1, 0, fixP->fx_size);
	      break;

	    case DW_CFA_advance_loc2:
	      fixP->fx_size = 2;
	      fixP->fx_next->fx_size = 2;
	      fixP->fx_where = loc + 1;
	      fixP->fx_next->fx_where = loc + 1;
	      fixP->fx_r_type = BFD_RELOC_LARCH_ADD16;
	      fixP->fx_next->fx_r_type = BFD_RELOC_LARCH_SUB16;
	      md_number_to_chars (buf+1, 0, fixP->fx_size);
	      break;

	    case DW_CFA_advance_loc4:
	      fixP->fx_size = 4;
	      fixP->fx_next->fx_size = 4;
	      fixP->fx_where = loc;
	      fixP->fx_next->fx_where = loc;
	      fixP->fx_r_type = BFD_RELOC_LARCH_ADD32;
	      fixP->fx_next->fx_r_type = BFD_RELOC_LARCH_SUB32;
	      md_number_to_chars (buf+1, 0, fixP->fx_size);
	      break;

	    default:
	      if (subtype < 0x80 && (subtype & 0x40))
		{
		  /* DW_CFA_advance_loc.  */
		  fixP->fx_frag = (fragS *) fixP->fx_frag->fr_opcode;
		  fixP->fx_next->fx_frag = fixP->fx_frag;
		  fixP->fx_r_type = BFD_RELOC_LARCH_ADD6;
		  fixP->fx_next->fx_r_type = BFD_RELOC_LARCH_SUB6;
		  md_number_to_chars (buf, 0x40, fixP->fx_size);
		  }
	      else
		as_fatal (_("internal: bad CFA value #%d"), subtype);
	      break;
	    }
	}
      break;

    case BFD_RELOC_LARCH_B16:
    case BFD_RELOC_LARCH_B21:
    case BFD_RELOC_LARCH_B26:
      if (fixP->fx_addsy == NULL)
	{
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			_ ("Relocation against a constant."));
	}
      if (S_GET_SEGMENT (fixP->fx_addsy) == seg
	  && !S_FORCE_RELOC (fixP->fx_addsy, 1))
	{
	  int64_t sym_addend = S_GET_VALUE (fixP->fx_addsy) + fixP->fx_offset;
	  int64_t pc = fixP->fx_where + fixP->fx_frag->fr_address;
	  fix_reloc_insn (fixP, sym_addend - pc, buf);

	  /* If relax, symbol value may change at link time, so reloc need to
	     be saved.  */
	  if (!LARCH_opts.relax)
	    fixP->fx_done = 1;
	}
      break;

    /* Because ADD_ULEB128/SUB_ULEB128 always occur in pairs.
       So just deal with one is ok.
    case BFD_RELOC_LARCH_ADD_ULEB128:  */
    case BFD_RELOC_LARCH_SUB_ULEB128:
      {
	unsigned int len = 0;
	len = loongarch_get_uleb128_length ((bfd_byte *)buf);
	bfd_byte *endp = (bfd_byte*) buf + len -1;
	/* Clean the uleb128 value to 0. Do not reduce the length.  */
	memset (buf, 0x80, len - 1);
	*endp = 0;
	break;
      }

    default:
      break;
    }
}

int
loongarch_relax_frag (asection *sec ATTRIBUTE_UNUSED,
		      fragS *fragp ATTRIBUTE_UNUSED,
		      long stretch ATTRIBUTE_UNUSED)
{
  return 0;
}

int
md_estimate_size_before_relax (fragS *fragp ATTRIBUTE_UNUSED,
			       asection *segtype ATTRIBUTE_UNUSED)
{
  return (fragp->fr_var = 4);
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
  reloc->addend = fixp->fx_offset;

  reloc->howto = bfd_reloc_type_lookup (stdoutput, fixp->fx_r_type);
  if (reloc->howto == NULL)
    {
      as_bad_where (fixp->fx_file, fixp->fx_line,
		    _("cannot represent %s relocation in object file"),
		    bfd_get_reloc_code_name (fixp->fx_r_type));
      return NULL;
    }

  return reloc;
}

/* Convert a machine dependent frag.  */
void
md_convert_frag (bfd *abfd ATTRIBUTE_UNUSED, segT asec ATTRIBUTE_UNUSED,
		 fragS *fragp)
{
  expressionS exp;
  exp.X_op = O_symbol;
  exp.X_add_symbol = fragp->fr_symbol;
  exp.X_add_number = fragp->fr_offset;
  bfd_byte *buf = (bfd_byte *)fragp->fr_literal + fragp->fr_fix;

  fixS *fixp = fix_new_exp (fragp, buf - (bfd_byte *)fragp->fr_literal,
				4, &exp, false, fragp->fr_subtype);
  buf += 4;

  fixp->fx_file = fragp->fr_file;
  fixp->fx_line = fragp->fr_line;
  fragp->fr_fix += fragp->fr_var;

  gas_assert (fragp->fr_next == NULL
	      || (fragp->fr_next->fr_address - fragp->fr_address
		  == fragp->fr_fix));
}

/* Standard calling conventions leave the CFA at SP on entry.  */
void
loongarch_cfi_frame_initial_instructions (void)
{
  cfi_add_CFA_def_cfa_register (3 /* $sp */);
}

void
loongarch_pre_output_hook (void)
{
  const frchainS *frch;
  segT s;

  if (!LARCH_opts.relax)
    return;

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
			     BFD_RELOC_LARCH_CFA);
	      }
	  }
      }

  /* Restore the original segment info.  */
  subseg_set (seg, subseg);
}

void
tc_loongarch_parse_to_dw2regnum (expressionS *exp)
{
  expression_and_evaluate (exp);
}

void
md_show_usage (FILE *stream)
{
  fprintf (stream, _("LARCH options:\n"));
  /* FIXME */
}

static void
loongarch_make_nops (char *buf, bfd_vma bytes)
{
  bfd_vma i = 0;

  /* Fill with 4-byte NOPs.  */
  for ( ; i < bytes; i += 4)
    number_to_chars_littleendian (buf + i, LARCH_NOP, 4);
}

/* Called from md_do_align.  Used to create an alignment frag in a
   code section by emitting a worst-case NOP sequence that the linker
   will later relax to the correct number of NOPs.  We can't compute
   the correct alignment now because of other linker relaxations.  */

bool
loongarch_frag_align_code (int n)
{
  bfd_vma bytes = (bfd_vma) 1 << n;
  bfd_vma insn_alignment = 4;
  bfd_vma worst_case_bytes = bytes - insn_alignment;
  char *nops;
  expressionS ex;

  /* If we are moving to a smaller alignment than the instruction size, then no
     alignment is required.  */
  if (bytes <= insn_alignment)
    return true;

  /* When not relaxing, loongarch_handle_align handles code alignment.  */
  if (!LARCH_opts.relax)
    return false;

  nops = frag_more (worst_case_bytes);

  ex.X_op = O_constant;
  ex.X_add_number = worst_case_bytes;

  loongarch_make_nops (nops, worst_case_bytes);

  fix_new_exp (frag_now, nops - frag_now->fr_literal, 0,
	       &ex, false, BFD_RELOC_LARCH_ALIGN);

  /* We need to start a new frag after the alignment which may be removed by
     the linker, to prevent the assembler from computing static offsets.
     This is necessary to get correct EH info.  */
  frag_wane (frag_now);
  frag_new (0);

  return true;
}

/* Fill in an rs_align_code fragment.  We want to fill 'andi $r0,$r0,0'.  */
void
loongarch_handle_align (fragS *fragp)
{
  /* char nop_opcode; */
  char *p;
  int bytes, size, excess;
  valueT opcode;

  if (fragp->fr_type != rs_align_code)
    return;

  struct loongarch_cl_insn nop =
    { .name = "andi", .arg_strs = { "$r0", "$r0", "0", NULL } };

  get_loongarch_opcode (&nop);
  gas_assert (nop.all_match);

  p = fragp->fr_literal + fragp->fr_fix;
  opcode = nop.insn_bin;
  size = 4;

  bytes = fragp->fr_next->fr_address - fragp->fr_address - fragp->fr_fix;
  excess = bytes % size;

  gas_assert (excess < 4);
  fragp->fr_fix += excess;

  while (excess-- != 0)
    *p++ = 0;

  md_number_to_chars (p, opcode, size);
  fragp->fr_var = size;
}

/* Scan uleb128 subtraction expressions and insert fixups for them.
   e.g., .uleb128 .L1 - .L0
   Because relaxation may change the value of the subtraction, we
   must resolve them at link-time.  */

static void
loongarch_insert_uleb128_fixes (bfd *abfd ATTRIBUTE_UNUSED,
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

      /* FIXME: Skip for .sleb128.  */
      if (fragP->fr_subtype != 0)
	continue;

      exp_dup = xmemdup (exp, sizeof (*exp), sizeof (*exp));
      exp_dup->X_op = O_symbol;
      exp_dup->X_op_symbol = NULL;

      exp_dup->X_add_symbol = exp->X_add_symbol;
      fix_new_exp (fragP, fragP->fr_fix, 0,
		   exp_dup, 0, BFD_RELOC_LARCH_ADD_ULEB128);

      /* From binutils/testsuite/binutils-all/dw5.S
	 section .debug_rnglists
	 .uleb128 .Letext0-.Ltext0    Range length (*.LLRL2)
    Offset             Info             Type               Symbol's Value  Symbol's Name + Addend
0000000000000015  0000000200000079 R_LARCH_ADD_ULEB128    0000000000000000 .text + 2
0000000000000015  000000020000007a R_LARCH_SUB_ULEB128    0000000000000000 .text + 0.  */

      /* Only the ADD_ULEB128 has X_add_number (Addend)?  */
      exp_dup->X_add_number = 0;
      exp_dup->X_add_symbol = exp->X_op_symbol;
      fix_new_exp (fragP, fragP->fr_fix, 0,
		   exp_dup, 0, BFD_RELOC_LARCH_SUB_ULEB128);
    }
}

void
loongarch_md_finish (void)
{
  /* Insert relocations for uleb128 directives, so the values can be recomputed
     at link time.  */
  if (LARCH_opts.relax)
    bfd_map_over_sections (stdoutput, loongarch_insert_uleb128_fixes, NULL);
}

void
loongarch_elf_final_processing (void)
{
  elf_elfheader (stdoutput)->e_flags = LARCH_opts.ase_abi;
}
