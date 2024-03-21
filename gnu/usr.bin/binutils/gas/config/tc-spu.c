/* spu.c -- Assembler for the IBM Synergistic Processing Unit (SPU)

   Copyright (C) 2006-2023 Free Software Foundation, Inc.

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#include "as.h"
#include "safe-ctype.h"
#include "subsegs.h"
#include "dwarf2dbg.h"

const struct spu_opcode spu_opcodes[] = {
#define APUOP(TAG,MACFORMAT,OPCODE,MNEMONIC,ASMFORMAT,DEP,PIPE) \
	{ MACFORMAT, (OPCODE ## u) << (32-11), MNEMONIC, ASMFORMAT },
#define APUOPFB(TAG,MACFORMAT,OPCODE,FB,MNEMONIC,ASMFORMAT,DEP,PIPE) \
	{ MACFORMAT, ((OPCODE) << (32-11)) | ((FB) << (32-18)), MNEMONIC, ASMFORMAT },
#include "opcode/spu-insns.h"
#undef APUOP
#undef APUOPFB
};

static const int spu_num_opcodes =
  sizeof (spu_opcodes) / sizeof (spu_opcodes[0]);

#define MAX_RELOCS 2

struct spu_insn
{
  unsigned int opcode;
  expressionS exp[MAX_RELOCS];
  int reloc_arg[MAX_RELOCS];
  bfd_reloc_code_real_type reloc[MAX_RELOCS];
  enum spu_insns tag;
};

static const char *get_imm (const char *param, struct spu_insn *insn, int arg);
static const char *get_reg (const char *param, struct spu_insn *insn, int arg,
			    int accept_expr);
static int calcop (struct spu_opcode *format, const char *param,
		   struct spu_insn *insn);
static void spu_brinfo (int);
static void spu_cons (int);

extern char *myname;
static htab_t op_hash = NULL;

/* These bits should be turned off in the first address of every segment */
int md_seg_align = 7;

/* These chars start a comment anywhere in a source file (except inside
   another comment */
const char comment_chars[] = "#";

/* These chars only start a comment at the beginning of a line.  */
const char line_comment_chars[] = "#";

/* gods own line continuation char */
const char line_separator_chars[] = ";";

/* Chars that can be used to separate mant from exp in floating point nums */
const char EXP_CHARS[] = "eE";

/* Chars that mean this number is a floating point constant */
/* as in 0f123.456 */
/* or    0H1.234E-12 (see exp chars above) */
const char FLT_CHARS[] = "dDfF";

const pseudo_typeS md_pseudo_table[] =
{
  {"align", s_align_ptwo, 4},
  {"brinfo", spu_brinfo, 0},
  {"bss", s_lcomm_bytes, 1},
  {"def", s_set, 0},
  {"dfloat", float_cons, 'd'},
  {"ffloat", float_cons, 'f'},
  {"global", s_globl, 0},
  {"half", cons, 2},
  {"int", spu_cons, 4},
  {"long", spu_cons, 4},
  {"quad", spu_cons, 8},
  {"string", stringer, 8 + 1},
  {"word", spu_cons, 4},
  /* Force set to be treated as an instruction.  */
  {"set", NULL, 0},
  {".set", s_set, 0},
  /* Likewise for eqv.  */
  {"eqv", NULL, 0},
  {".eqv", s_set, -1},
  {0,0,0}
};

/* Bits plugged into branch instruction offset field.  */
unsigned int brinfo;

void
md_begin (void)
{
  int i;

  op_hash = str_htab_create ();

  /* Hash each mnemonic and record its position.  There are
     duplicates, keep just the first.  */
  for (i = 0; i < spu_num_opcodes; i++)
    str_hash_insert (op_hash, spu_opcodes[i].mnemonic, &spu_opcodes[i], 0);
}

const char *md_shortopts = "";
struct option md_longopts[] = {
#define OPTION_APUASM (OPTION_MD_BASE)
  {"apuasm", no_argument, NULL, OPTION_APUASM},
#define OPTION_DD2 (OPTION_MD_BASE+1)
  {"mdd2.0", no_argument, NULL, OPTION_DD2},
#define OPTION_DD1 (OPTION_MD_BASE+2)
  {"mdd1.0", no_argument, NULL, OPTION_DD1},
#define OPTION_DD3 (OPTION_MD_BASE+3)
  {"mdd3.0", no_argument, NULL, OPTION_DD3},
  { NULL, no_argument, NULL, 0 }
};
size_t md_longopts_size = sizeof (md_longopts);

/* When set (by -apuasm) our assembler emulates the behaviour of apuasm.
 * e.g. don't add bias to float conversion and don't right shift
 * immediate values. */
static int emulate_apuasm;

/* Use the dd2.0 instructions set.  The only differences are some new
 * register names and the orx insn */
static int use_dd2 = 1;

int
md_parse_option (int c, const char *arg ATTRIBUTE_UNUSED)
{
  switch (c)
    {
    case OPTION_APUASM:
      emulate_apuasm = 1;
      break;
    case OPTION_DD3:
      use_dd2 = 1;
      break;
    case OPTION_DD2:
      use_dd2 = 1;
      break;
    case OPTION_DD1:
      use_dd2 = 0;
      break;
    default:
      return 0;
    }
  return 1;
}

void
md_show_usage (FILE *stream)
{
  fputs (_("\
SPU options:\n\
  --apuasm		  emulate behaviour of apuasm\n"),
	stream);
}


struct arg_encode {
  int size;
  int pos;
  int rshift;
  int lo, hi;
  int wlo, whi;
  bfd_reloc_code_real_type reloc;
};

static struct arg_encode arg_encode[A_MAX] = {
  {  7,  0, 0,       0,    127,    0,   -1,  0 }, /* A_T */
  {  7,  7, 0,       0,    127,    0,   -1,  0 }, /* A_A */
  {  7, 14, 0,       0,    127,    0,   -1,  0 }, /* A_B */
  {  7, 21, 0,       0,    127,    0,   -1,  0 }, /* A_C */
  {  7,  7, 0,       0,    127,    0,   -1,  0 }, /* A_S */
  {  7,  7, 0,       0,    127,    0,   -1,  0 }, /* A_H */
  {  0,  0, 0,       0,     -1,    0,   -1,  0 }, /* A_P */
  {  7, 14, 0,       0,     -1,    0,   -1,  BFD_RELOC_SPU_IMM7 }, /* A_S3 */
  {  7, 14, 0,     -32,     31,  -31,    0,  BFD_RELOC_SPU_IMM7 }, /* A_S6 */
  {  7, 14, 0,       0,     -1,    0,   -1,  BFD_RELOC_SPU_IMM7 }, /* A_S7N */
  {  7, 14, 0,     -64,     63,  -63,    0,  BFD_RELOC_SPU_IMM7 }, /* A_S7 */
  {  8, 14, 0,       0,    127,    0,   -1,  BFD_RELOC_SPU_IMM8 }, /* A_U7A */
  {  8, 14, 0,       0,    127,    0,   -1,  BFD_RELOC_SPU_IMM8 }, /* A_U7B */
  { 10, 14, 0,    -512,    511, -128,  255,  BFD_RELOC_SPU_IMM10 }, /* A_S10B */
  { 10, 14, 0,    -512,    511,    0,   -1,  BFD_RELOC_SPU_IMM10 }, /* A_S10 */
  {  2, 23, 9,   -1024,   1023,    0,   -1,  BFD_RELOC_SPU_PCREL9a }, /* A_S11 */
  {  2, 14, 9,   -1024,   1023,    0,   -1,  BFD_RELOC_SPU_PCREL9b }, /* A_S11I */
  { 10, 14, 4,   -8192,   8191,    0,   -1,  BFD_RELOC_SPU_IMM10W }, /* A_S14 */
  { 16,  7, 0,  -32768,  32767,    0,   -1,  BFD_RELOC_SPU_IMM16 }, /* A_S16 */
  { 16,  7, 2, -131072, 262143,    0,   -1,  BFD_RELOC_SPU_IMM16W }, /* A_S18 */
  { 16,  7, 2, -262144, 262143,    0,   -1,  BFD_RELOC_SPU_PCREL16 }, /* A_R18 */
  {  7, 14, 0,       0,     -1,    0,   -1,  BFD_RELOC_SPU_IMM7 }, /* A_U3 */
  {  7, 14, 0,       0,    127,    0,   31,  BFD_RELOC_SPU_IMM7 }, /* A_U5 */
  {  7, 14, 0,       0,    127,    0,   63,  BFD_RELOC_SPU_IMM7 }, /* A_U6 */
  {  7, 14, 0,       0,     -1,    0,   -1,  BFD_RELOC_SPU_IMM7 }, /* A_U7 */
  { 14,  0, 0,       0,  16383,    0,   -1,  0 }, /* A_U14 */
  { 16,  7, 0,  -32768,  65535,    0,   -1,  BFD_RELOC_SPU_IMM16 }, /* A_X16 */
  { 18,  7, 0,       0, 262143,    0,   -1,  BFD_RELOC_SPU_IMM18 }, /* A_U18 */
};

/* Some flags for handling errors.  This is very hackish and added after
 * the fact. */
static int syntax_error_arg;
static const char *syntax_error_param;
static int syntax_reg;

static char *
insn_fmt_string (struct spu_opcode *format)
{
  static char buf[64];
  int len = 0;
  int i;

  len += sprintf (&buf[len], "%s\t", format->mnemonic);
  for (i = 1; i <= format->arg[0]; i++)
    {
      int arg = format->arg[i];
      const char *exp;
      if (i > 1 && arg != A_P && format->arg[i-1] != A_P)
	buf[len++] =  ',';
      if (arg == A_P)
	exp = "(";
      else if (arg < A_P)
	exp = i == syntax_error_arg ? "REG" : "reg";
      else
	exp = i == syntax_error_arg ? "IMM" : "imm";
      len += sprintf (&buf[len], "%s", exp);
      if (i > 1 && format->arg[i-1] == A_P)
	buf[len++] =  ')';
    }
  buf[len] = 0;
  return buf;
}

void
md_assemble (char *op)
{
  char *param, *thisfrag;
  char c;
  struct spu_opcode *format;
  struct spu_insn insn;
  int i;

  gas_assert (op);

  /* skip over instruction to find parameters */

  for (param = op; *param != 0 && !ISSPACE (*param); param++)
    ;
  c = *param;
  *param = 0;

  if (c != 0 && c != '\n')
    param++;

  /* try to find the instruction in the hash table */

  if ((format = (struct spu_opcode *) str_hash_find (op_hash, op)) == NULL)
    {
      as_bad (_("Invalid mnemonic '%s'"), op);
      return;
    }

  if (!use_dd2 && strcmp (format->mnemonic, "orx") == 0)
    {
      as_bad (_("'%s' is only available in DD2.0 or higher."), op);
      return;
    }

  while (1)
    {
      /* try parsing this instruction into insn */
      for (i = 0; i < MAX_RELOCS; i++)
	{
	  insn.exp[i].X_add_symbol = 0;
	  insn.exp[i].X_op_symbol = 0;
	  insn.exp[i].X_add_number = 0;
	  insn.exp[i].X_op = O_illegal;
	  insn.reloc_arg[i] = -1;
	  insn.reloc[i] = BFD_RELOC_NONE;
	}
      insn.opcode = format->opcode;
      insn.tag = (enum spu_insns) (format - spu_opcodes);

      syntax_error_arg = 0;
      syntax_error_param = 0;
      syntax_reg = 0;
      if (calcop (format, param, &insn))
	break;

      /* if it doesn't parse try the next instruction */
      if (!strcmp (format[0].mnemonic, format[1].mnemonic))
	format++;
      else
	{
	  int parg = format[0].arg[syntax_error_arg-1];

	  as_fatal (_("Error in argument %d.  Expecting:  \"%s\""),
		    syntax_error_arg - (parg == A_P),
		    insn_fmt_string (format));
	  return;
	}
    }

  if ((syntax_reg & 4)
      && ! (insn.tag == M_RDCH
	    || insn.tag == M_RCHCNT
	    || insn.tag == M_WRCH))
    as_warn (_("Mixing register syntax, with and without '$'."));
  if (syntax_error_param)
    {
      const char *d = syntax_error_param;
      while (*d != '$')
	d--;
      as_warn (_("Treating '%-*s' as a symbol."), (int)(syntax_error_param - d), d);
    }

  if (brinfo != 0
      && (insn.tag <= M_BRASL
	  || (insn.tag >= M_BRZ && insn.tag <= M_BRHNZ))
      && (insn.opcode & 0x7ff80) == 0
      && (insn.reloc_arg[0] == A_R18
	  || insn.reloc_arg[0] == A_S18
	  || insn.reloc_arg[1] == A_R18
	  || insn.reloc_arg[1] == A_S18))
    insn.opcode |= brinfo << 7;

  /* grow the current frag and plop in the opcode */

  thisfrag = frag_more (4);
  md_number_to_chars (thisfrag, insn.opcode, 4);

  /* if this instruction requires labels mark it for later */

  for (i = 0; i < MAX_RELOCS; i++)
    if (insn.reloc_arg[i] >= 0)
      {
        fixS *fixP;
        bfd_reloc_code_real_type reloc = insn.reloc[i];
	int pcrel = 0;

	if (reloc == BFD_RELOC_SPU_PCREL9a
	    || reloc == BFD_RELOC_SPU_PCREL9b
	    || reloc == BFD_RELOC_SPU_PCREL16)
	  pcrel = 1;
	fixP = fix_new_exp (frag_now,
			    thisfrag - frag_now->fr_literal,
			    4,
			    &insn.exp[i],
			    pcrel,
			    reloc);
	fixP->tc_fix_data.arg_format = insn.reloc_arg[i];
	fixP->tc_fix_data.insn_tag = insn.tag;
      }
  dwarf2_emit_insn (4);

  /* .brinfo lasts exactly one instruction.  */
  brinfo = 0;
}

static int
calcop (struct spu_opcode *format, const char *param, struct spu_insn *insn)
{
  int i;
  int paren = 0;
  int arg;

  for (i = 1; i <= format->arg[0]; i++)
    {
      arg = format->arg[i];
      syntax_error_arg = i;

      while (ISSPACE (*param))
        param++;
      if (*param == 0 || *param == ',')
	return 0;
      if (arg < A_P)
        param = get_reg (param, insn, arg, 1);
      else if (arg > A_P)
        param = get_imm (param, insn, arg);
      else if (arg == A_P)
	{
	  paren++;
	  if ('(' != *param++)
	    return 0;
	}

      if (!param)
	return 0;

      while (ISSPACE (*param))
        param++;

      if (arg != A_P && paren)
	{
	  paren--;
	  if (')' != *param++)
	    return 0;
	}
      else if (i < format->arg[0]
	       && format->arg[i] != A_P
	       && format->arg[i+1] != A_P)
	{
	  if (',' != *param++)
	    {
	      syntax_error_arg++;
	      return 0;
	    }
	}
    }
  while (ISSPACE (*param))
    param++;
  return !paren && (*param == 0 || *param == '\n');
}

struct reg_name {
    unsigned int regno;
    unsigned int length;
    char name[32];
};

#define REG_NAME(NO,NM) { NO, sizeof (NM) - 1, NM }

static struct reg_name reg_name[] = {
  REG_NAME (0, "lr"),  /* link register */
  REG_NAME (1, "sp"),  /* stack pointer */
  REG_NAME (0, "rp"),  /* link register */
  REG_NAME (127, "fp"),  /* frame pointer */
};

static struct reg_name sp_reg_name[] = {
};

static struct reg_name ch_reg_name[] = {
  REG_NAME (  0, "SPU_RdEventStat"),
  REG_NAME (  1, "SPU_WrEventMask"),
  REG_NAME (  2, "SPU_WrEventAck"),
  REG_NAME (  3, "SPU_RdSigNotify1"),
  REG_NAME (  4, "SPU_RdSigNotify2"),
  REG_NAME (  7, "SPU_WrDec"),
  REG_NAME (  8, "SPU_RdDec"),
  REG_NAME ( 11, "SPU_RdEventMask"), /* DD2.0 only */
  REG_NAME ( 13, "SPU_RdMachStat"),
  REG_NAME ( 14, "SPU_WrSRR0"),
  REG_NAME ( 15, "SPU_RdSRR0"),
  REG_NAME ( 28, "SPU_WrOutMbox"),
  REG_NAME ( 29, "SPU_RdInMbox"),
  REG_NAME ( 30, "SPU_WrOutIntrMbox"),
  REG_NAME (  9, "MFC_WrMSSyncReq"),
  REG_NAME ( 12, "MFC_RdTagMask"),   /* DD2.0 only */
  REG_NAME ( 16, "MFC_LSA"),
  REG_NAME ( 17, "MFC_EAH"),
  REG_NAME ( 18, "MFC_EAL"),
  REG_NAME ( 19, "MFC_Size"),
  REG_NAME ( 20, "MFC_TagID"),
  REG_NAME ( 21, "MFC_Cmd"),
  REG_NAME ( 22, "MFC_WrTagMask"),
  REG_NAME ( 23, "MFC_WrTagUpdate"),
  REG_NAME ( 24, "MFC_RdTagStat"),
  REG_NAME ( 25, "MFC_RdListStallStat"),
  REG_NAME ( 26, "MFC_WrListStallAck"),
  REG_NAME ( 27, "MFC_RdAtomicStat"),
};
#undef REG_NAME

static const char *
get_reg (const char *param, struct spu_insn *insn, int arg, int accept_expr)
{
  unsigned regno;
  int saw_prefix = 0;

  if (*param == '$')
    {
      saw_prefix = 1;
      param++;
    }

  if (arg == A_H) /* Channel */
    {
      if ((param[0] == 'c' || param[0] == 'C')
	  && (param[1] == 'h' || param[1] == 'H')
	  && ISDIGIT (param[2]))
        param += 2;
    }
  else if (arg == A_S) /* Special purpose register */
    {
      if ((param[0] == 's' || param[0] == 'S')
	  && (param[1] == 'p' || param[1] == 'P')
	  && ISDIGIT (param[2]))
        param += 2;
    }

  if (ISDIGIT (*param))
    {
      regno = 0;
      while (ISDIGIT (*param))
	regno = regno * 10 + *param++ - '0';
    }
  else
    {
      struct reg_name *rn;
      unsigned int i, n, l = 0;

      if (arg == A_H) /* Channel */
	{
	  rn = ch_reg_name;
	  n = sizeof (ch_reg_name) / sizeof (*ch_reg_name);
	}
      else if (arg == A_S) /* Special purpose register */
	{
	  rn = sp_reg_name;
	  n = sizeof (sp_reg_name) / sizeof (*sp_reg_name);
	}
      else
	{
	  rn = reg_name;
	  n = sizeof (reg_name) / sizeof (*reg_name);
	}
      regno = 128;
      for (i = 0; i < n; i++)
	if (rn[i].length > l
	    && 0 == strncasecmp (param, rn[i].name, rn[i].length))
          {
	    l = rn[i].length;
            regno = rn[i].regno;
          }
      param += l;
    }

  if (!use_dd2
      && arg == A_H)
    {
      if (regno == 11)
	as_bad (_("'SPU_RdEventMask' (channel 11) is only available in DD2.0 or higher."));
      else if (regno == 12)
	as_bad (_("'MFC_RdTagMask' (channel 12) is only available in DD2.0 or higher."));
    }

  if (regno < 128)
    {
      insn->opcode |= regno << arg_encode[arg].pos;
      if ((!saw_prefix && syntax_reg == 1)
	  || (saw_prefix && syntax_reg == 2))
	syntax_reg |= 4;
      syntax_reg |= saw_prefix ? 1 : 2;
      return param;
    }

  if (accept_expr)
    {
      char *save_ptr;
      expressionS ex;
      save_ptr = input_line_pointer;
      input_line_pointer = (char *)param;
      expression (&ex);
      param = input_line_pointer;
      input_line_pointer = save_ptr;
      resolve_register (&ex);
      if (ex.X_op == O_register || ex.X_op == O_constant)
	{
	  insn->opcode |= ex.X_add_number << arg_encode[arg].pos;
	  return param;
	}
    }
  return 0;
}

static const char *
get_imm (const char *param, struct spu_insn *insn, int arg)
{
  int val;
  char *save_ptr;
  int low = 0, high = 0;
  int reloc_i = insn->reloc_arg[0] >= 0 ? 1 : 0;

  if (strncasecmp (param, "%lo(", 4) == 0)
    {
      param += 3;
      low = 1;
      as_warn (_("Using old style, %%lo(expr), please change to PPC style, expr@l."));
    }
  else if (strncasecmp (param, "%hi(", 4) == 0)
    {
      param += 3;
      high = 1;
      as_warn (_("Using old style, %%hi(expr), please change to PPC style, expr@h."));
    }
  else if (strncasecmp (param, "%pic(", 5) == 0)
    {
      /* Currently we expect %pic(expr) == expr, so do nothing here.
	 i.e. for code loaded at address 0 $toc will be 0.  */
      param += 4;
    }

  if (*param == '$')
    {
      /* Symbols can start with $, but if this symbol matches a register
	 name, it's probably a mistake.  The only way to avoid this
	 warning is to rename the symbol.  */
      struct spu_insn tmp_insn;
      const char *np = get_reg (param, &tmp_insn, arg, 0);

      if (np)
	syntax_error_param = np;
    }

  save_ptr = input_line_pointer;
  input_line_pointer = (char *) param;
  expression (&insn->exp[reloc_i]);
  param = input_line_pointer;
  input_line_pointer = save_ptr;

  /* Similar to ppc_elf_suffix in tc-ppc.c.  We have so few cases to
     handle we do it inlined here. */
  if (param[0] == '@' && !ISALNUM (param[2]) && param[2] != '@')
    {
      if (param[1] == 'h' || param[1] == 'H')
	{
	  high = 1;
	  param += 2;
	}
      else if (param[1] == 'l' || param[1] == 'L')
	{
	  low = 1;
	  param += 2;
	}
    }

  if (insn->exp[reloc_i].X_op == O_constant)
    {
      val = insn->exp[reloc_i].X_add_number;

      if (emulate_apuasm)
	{
	  /* Convert the value to a format we expect. */
          val <<= arg_encode[arg].rshift;
	  if (arg == A_U7A)
	    val = 173 - val;
	  else if (arg == A_U7B)
	    val = 155 - val;
	}

      if (high)
	val = val >> 16;
      else if (low)
	val = val & 0xffff;

      /* Warn about out of range expressions. */
      {
	int hi = arg_encode[arg].hi;
	int lo = arg_encode[arg].lo;
	int whi = arg_encode[arg].whi;
	int wlo = arg_encode[arg].wlo;

	if (hi > lo && (val < lo || val > hi))
	  as_fatal (_("Constant expression %d out of range, [%d, %d]."),
		    val, lo, hi);
	else if (whi > wlo && (val < wlo || val > whi))
	  as_warn (_("Constant expression %d out of range, [%d, %d]."),
		   val, wlo, whi);
      }

      if (arg == A_U7A)
        val = 173 - val;
      else if (arg == A_U7B)
        val = 155 - val;

      /* Branch hints have a split encoding.  Do the bottom part. */
      if (arg == A_S11 || arg == A_S11I)
	insn->opcode |= ((val >> 2) & 0x7f);

      insn->opcode |= (((val >> arg_encode[arg].rshift)
			& ((1 << arg_encode[arg].size) - 1))
		       << arg_encode[arg].pos);
    }
  else
    {
      insn->reloc_arg[reloc_i] = arg;
      if (high)
	insn->reloc[reloc_i] = BFD_RELOC_SPU_HI16;
      else if (low)
	insn->reloc[reloc_i] = BFD_RELOC_SPU_LO16;
      else
	insn->reloc[reloc_i] = arg_encode[arg].reloc;
    }

  return param;
}

const char *
md_atof (int type, char *litP, int *sizeP)
{
  return ieee_md_atof (type, litP, sizeP, true);
}

#ifndef WORKING_DOT_WORD
int md_short_jump_size = 4;

void
md_create_short_jump (char *ptr,
		      addressT from_addr ATTRIBUTE_UNUSED,
		      addressT to_addr ATTRIBUTE_UNUSED,
		      fragS *frag,
		      symbolS *to_symbol)
{
  ptr[0] = (char) 0xc0;
  ptr[1] = 0x00;
  ptr[2] = 0x00;
  ptr[3] = 0x00;
  fix_new (frag,
	   ptr - frag->fr_literal,
	   4,
	   to_symbol,
	   (offsetT) 0,
	   0,
	   BFD_RELOC_SPU_PCREL16);
}

int md_long_jump_size = 4;

void
md_create_long_jump (char *ptr,
		     addressT from_addr ATTRIBUTE_UNUSED,
		     addressT to_addr ATTRIBUTE_UNUSED,
		     fragS *frag,
		     symbolS *to_symbol)
{
  ptr[0] = (char) 0xc0;
  ptr[1] = 0x00;
  ptr[2] = 0x00;
  ptr[3] = 0x00;
  fix_new (frag,
	   ptr - frag->fr_literal,
	   4,
	   to_symbol,
	   (offsetT) 0,
	   0,
	   BFD_RELOC_SPU_PCREL16);
}
#endif

/* Handle .brinfo <priority>,<lrlive>.  */
static void
spu_brinfo (int ignore ATTRIBUTE_UNUSED)
{
  addressT priority;
  addressT lrlive;

  priority = get_absolute_expression ();
  SKIP_WHITESPACE ();

  lrlive = 0;
  if (*input_line_pointer == ',')
    {
      ++input_line_pointer;
      lrlive = get_absolute_expression ();
    }

  if (priority > 0x1fff)
    {
      as_bad (_("invalid priority '%lu'"), (unsigned long) priority);
      priority = 0;
    }

  if (lrlive > 7)
    {
      as_bad (_("invalid lrlive '%lu'"), (unsigned long) lrlive);
      lrlive = 0;
    }

  brinfo = (lrlive << 13) | priority;
  demand_empty_rest_of_line ();
}

/* Support @ppu on symbols referenced in .int/.long/.word/.quad.  */
static void
spu_cons (int nbytes)
{
  expressionS exp;

  if (is_it_end_of_statement ())
    {
      demand_empty_rest_of_line ();
      return;
    }

  do
    {
      char *save = input_line_pointer;

      /* Use deferred_expression here so that an expression involving
	 a symbol that happens to be defined already as an spu symbol,
	 is not resolved.  */
      deferred_expression (&exp);
      if ((exp.X_op == O_symbol
	   || exp.X_op == O_constant)
	  && strncasecmp (input_line_pointer, "@ppu", 4) == 0)
	{
	  char *p = frag_more (nbytes);
	  enum bfd_reloc_code_real reloc;

	  /* Check for identifier@suffix+constant.  */
	  input_line_pointer += 4;
	  if (*input_line_pointer == '-' || *input_line_pointer == '+')
	    {
	      expressionS new_exp;

	      save = input_line_pointer;
	      expression (&new_exp);
	      if (new_exp.X_op == O_constant)
		exp.X_add_number += new_exp.X_add_number;
	      else
		input_line_pointer = save;
	    }

	  reloc = nbytes == 4 ? BFD_RELOC_SPU_PPU32 : BFD_RELOC_SPU_PPU64;
	  fix_new_exp (frag_now, p - frag_now->fr_literal, nbytes,
		       &exp, 0, reloc);
	}
      else
	{
	  /* Don't use deferred_expression for anything else.
	     deferred_expression won't evaulate dot at the point it is
	     used.  */
	  input_line_pointer = save;
	  expression (&exp);
	  emit_expr (&exp, nbytes);
	}
    }
  while (*input_line_pointer++ == ',');

  /* Put terminator back into stream.  */
  input_line_pointer--;
  demand_empty_rest_of_line ();
}

int
md_estimate_size_before_relax (fragS *fragP ATTRIBUTE_UNUSED,
			       segT segment_type ATTRIBUTE_UNUSED)
{
  as_fatal (_("Relaxation should never occur"));
  return -1;
}

/* If while processing a fixup, a reloc really needs to be created,
   then it is done here.  */

arelent *
tc_gen_reloc (asection *seg ATTRIBUTE_UNUSED, fixS *fixp)
{
  arelent *reloc;
  reloc = XNEW (arelent);
  reloc->sym_ptr_ptr = XNEW (asymbol *);
  *reloc->sym_ptr_ptr = symbol_get_bfdsym (fixp->fx_addsy);
  reloc->address = fixp->fx_frag->fr_address + fixp->fx_where;
  reloc->howto = bfd_reloc_type_lookup (stdoutput, fixp->fx_r_type);
  if (reloc->howto == (reloc_howto_type *) NULL)
    {
      as_bad_where (fixp->fx_file, fixp->fx_line,
		    _("reloc %d not supported by object file format"),
		    (int) fixp->fx_r_type);
      free (reloc->sym_ptr_ptr);
      free (reloc);
      return NULL;
    }
  reloc->addend = fixp->fx_addnumber;
  return reloc;
}

/* Round up a section's size to the appropriate boundary.  */

valueT
md_section_align (segT seg, valueT size)
{
  int align = bfd_section_alignment (seg);
  valueT mask = ((valueT) 1 << align) - 1;

  return (size + mask) & ~mask;
}

/* Where a PC relative offset is calculated from.  On the spu they
   are calculated from the beginning of the branch instruction.  */

long
md_pcrel_from (fixS *fixp)
{
  return fixp->fx_frag->fr_address + fixp->fx_where;
}

/* Fill in rs_align_code fragments.  */

void
spu_handle_align (fragS *fragp)
{
  static const unsigned char nop_pattern[8] = {
    0x40, 0x20, 0x00, 0x00, /* even nop */
    0x00, 0x20, 0x00, 0x00, /* odd  nop */
  };

  int bytes;
  char *p;

  if (fragp->fr_type != rs_align_code)
    return;

  bytes = fragp->fr_next->fr_address - fragp->fr_address - fragp->fr_fix;
  p = fragp->fr_literal + fragp->fr_fix;

  if (bytes & 3)
    {
      int fix = bytes & 3;
      memset (p, 0, fix);
      p += fix;
      bytes -= fix;
      fragp->fr_fix += fix;
    }
  if (bytes & 4)
    {
      memcpy (p, &nop_pattern[4], 4);
      p += 4;
      bytes -= 4;
      fragp->fr_fix += 4;
    }

  memcpy (p, nop_pattern, 8);
  fragp->fr_var = 8;
}

void
md_apply_fix (fixS *fixP, valueT *valP, segT seg ATTRIBUTE_UNUSED)
{
  unsigned int res;
  unsigned int mask;
  valueT val = *valP;
  char *place = fixP->fx_where + fixP->fx_frag->fr_literal;

  if (fixP->fx_subsy != (symbolS *) NULL)
    {
      /* We can't actually support subtracting a symbol.  */
      as_bad_subtract (fixP);
    }

  if (fixP->fx_addsy != NULL)
    {
      if (fixP->fx_pcrel)
	{
	  /* Hack around bfd_install_relocation brain damage.  */
	  val += fixP->fx_frag->fr_address + fixP->fx_where;

	  switch (fixP->fx_r_type)
	    {
	    case BFD_RELOC_32:
	      fixP->fx_r_type = BFD_RELOC_32_PCREL;
	      break;

	    case BFD_RELOC_SPU_PCREL16:
	    case BFD_RELOC_SPU_PCREL9a:
	    case BFD_RELOC_SPU_PCREL9b:
	    case BFD_RELOC_32_PCREL:
	      break;

	    default:
	      as_bad_where (fixP->fx_file, fixP->fx_line,
			    _("expression too complex"));
	      break;
	    }
	}
    }

  fixP->fx_addnumber = val;

  if (fixP->fx_r_type == BFD_RELOC_SPU_PPU32
      || fixP->fx_r_type == BFD_RELOC_SPU_PPU64
      || fixP->fx_r_type == BFD_RELOC_SPU_ADD_PIC)
    return;

  if (fixP->fx_addsy == NULL && fixP->fx_pcrel == 0)
    {
      fixP->fx_done = 1;
      res = 0;
      mask = 0;
      if (fixP->tc_fix_data.arg_format > A_P)
	{
	  int hi = arg_encode[fixP->tc_fix_data.arg_format].hi;
	  int lo = arg_encode[fixP->tc_fix_data.arg_format].lo;
	  if (hi > lo && ((offsetT) val < lo || (offsetT) val > hi))
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("Relocation doesn't fit. (relocation value = 0x%lx)"),
			  (long) val);
	}

      switch (fixP->fx_r_type)
	{
	case BFD_RELOC_8:
	  md_number_to_chars (place, val, 1);
	  return;

	case BFD_RELOC_16:
	  md_number_to_chars (place, val, 2);
	  return;

	case BFD_RELOC_32:
	case BFD_RELOC_32_PCREL:
	  md_number_to_chars (place, val, 4);
	  return;

	case BFD_RELOC_64:
	  md_number_to_chars (place, val, 8);
	  return;

	case BFD_RELOC_SPU_IMM7:
	  res = val << 14;
	  mask = 0x7f << 14;
	  break;

	case BFD_RELOC_SPU_IMM8:
	  res = val << 14;
	  mask = 0xff << 14;
	  break;

	case BFD_RELOC_SPU_IMM10:
	  res = val << 14;
	  mask = 0x3ff << 14;
	  break;

	case BFD_RELOC_SPU_IMM10W:
	  res = val << 10;
	  mask = 0x3ff0 << 10;
	  break;

	case BFD_RELOC_SPU_IMM16:
	  res = val << 7;
	  mask = 0xffff << 7;
	  break;

	case BFD_RELOC_SPU_IMM16W:
	  res = val << 5;
	  mask = 0x3fffc << 5;
	  break;

	case BFD_RELOC_SPU_IMM18:
	  res = val << 7;
	  mask = 0x3ffff << 7;
	  break;

	case BFD_RELOC_SPU_PCREL9a:
	  res = ((val & 0x1fc) >> 2) | ((val & 0x600) << 14);
	  mask = (0x1fc >> 2) | (0x600 << 14);
	  break;

	case BFD_RELOC_SPU_PCREL9b:
	  res = ((val & 0x1fc) >> 2) | ((val & 0x600) << 5);
	  mask = (0x1fc >> 2) | (0x600 << 5);
	  break;

	case BFD_RELOC_SPU_PCREL16:
	  res = val << 5;
	  mask = 0x3fffc << 5;
	  break;

	case BFD_RELOC_SPU_HI16:
	  res = val >> 9;
	  mask = 0xffff << 7;
	  break;

	case BFD_RELOC_SPU_LO16:
	  res = val << 7;
	  mask = 0xffff << 7;
	  break;

	default:
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			_("reloc %d not supported by object file format"),
			(int) fixP->fx_r_type);
	}

      res &= mask;
      place[0] = (place[0] & (~mask >> 24)) | ((res >> 24) & 0xff);
      place[1] = (place[1] & (~mask >> 16)) | ((res >> 16) & 0xff);
      place[2] = (place[2] & (~mask >> 8)) | ((res >> 8) & 0xff);
      place[3] = (place[3] & ~mask) | (res & 0xff);
    }
}
