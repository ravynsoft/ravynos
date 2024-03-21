/* tc-bfin.c -- Assembler for the ADI Blackfin.
   Copyright (C) 2005-2023 Free Software Foundation, Inc.

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
#include "bfin-defs.h"
#include "obstack.h"
#include "safe-ctype.h"
#ifdef OBJ_ELF
#include "dwarf2dbg.h"
#endif
#include "elf/common.h"
#include "elf/bfin.h"

extern int yyparse (void);
struct yy_buffer_state;
typedef struct yy_buffer_state *YY_BUFFER_STATE;
extern YY_BUFFER_STATE yy_scan_string (const char *yy_str);
extern void yy_delete_buffer (YY_BUFFER_STATE b);
static parse_state parse (char *line);

/* Global variables. */
struct bfin_insn *insn;
int last_insn_size;

extern struct obstack mempool;
FILE *errorf;

/* Flags to set in the elf header */
#define DEFAULT_FLAGS 0

#ifdef OBJ_FDPIC_ELF
# define DEFAULT_FDPIC EF_BFIN_FDPIC
#else
# define DEFAULT_FDPIC 0
#endif

static flagword bfin_flags = DEFAULT_FLAGS | DEFAULT_FDPIC;
static const char *bfin_pic_flag = DEFAULT_FDPIC ? "-mfdpic" : (const char *)0;

/* Blackfin specific function to handle FD-PIC pointer initializations.  */

static void
bfin_pic_ptr (int nbytes)
{
  expressionS exp;
  char *p;

  if (nbytes != 4)
    abort ();

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

  do
    {
      bfd_reloc_code_real_type reloc_type = BFD_RELOC_BFIN_FUNCDESC;

      if (strncasecmp (input_line_pointer, "funcdesc(", 9) == 0)
	{
	  input_line_pointer += 9;
	  expression (&exp);
	  if (*input_line_pointer == ')')
	    input_line_pointer++;
	  else
	    as_bad (_("missing ')'"));
	}
      else
	error ("missing funcdesc in picptr");

      p = frag_more (4);
      memset (p, 0, 4);
      fix_new_exp (frag_now, p - frag_now->fr_literal, 4, &exp, 0,
		   reloc_type);
    }
  while (*input_line_pointer++ == ',');

  input_line_pointer--;			/* Put terminator back into stream. */
  demand_empty_rest_of_line ();
}

static void
bfin_s_bss (int ignore ATTRIBUTE_UNUSED)
{
  int temp;

  temp = get_absolute_expression ();
  subseg_set (bss_section, (subsegT) temp);
  demand_empty_rest_of_line ();
}

const pseudo_typeS md_pseudo_table[] = {
  {"align", s_align_bytes, 0},
  {"byte2", cons, 2},
  {"byte4", cons, 4},
  {"picptr", bfin_pic_ptr, 4},
  {"code", obj_elf_section, 0},
  {"db", cons, 1},
  {"dd", cons, 4},
  {"dw", cons, 2},
  {"p", s_ignore, 0},
  {"pdata", s_ignore, 0},
  {"var", s_ignore, 0},
  {"bss", bfin_s_bss, 0},
  {0, 0, 0}
};

/* Characters that are used to denote comments and line separators. */
const char comment_chars[] = "#";
const char line_comment_chars[] = "#";
const char line_separator_chars[] = ";";

/* Characters that can be used to separate the mantissa from the
   exponent in floating point numbers. */
const char EXP_CHARS[] = "eE";

/* Characters that mean this number is a floating point constant.
   As in 0f12.456 or  0d1.2345e12.  */
const char FLT_CHARS[] = "fFdDxX";

typedef enum bfin_cpu_type
{
  BFIN_CPU_UNKNOWN,
  BFIN_CPU_BF504,
  BFIN_CPU_BF506,
  BFIN_CPU_BF512,
  BFIN_CPU_BF514,
  BFIN_CPU_BF516,
  BFIN_CPU_BF518,
  BFIN_CPU_BF522,
  BFIN_CPU_BF523,
  BFIN_CPU_BF524,
  BFIN_CPU_BF525,
  BFIN_CPU_BF526,
  BFIN_CPU_BF527,
  BFIN_CPU_BF531,
  BFIN_CPU_BF532,
  BFIN_CPU_BF533,
  BFIN_CPU_BF534,
  BFIN_CPU_BF536,
  BFIN_CPU_BF537,
  BFIN_CPU_BF538,
  BFIN_CPU_BF539,
  BFIN_CPU_BF542,
  BFIN_CPU_BF542M,
  BFIN_CPU_BF544,
  BFIN_CPU_BF544M,
  BFIN_CPU_BF547,
  BFIN_CPU_BF547M,
  BFIN_CPU_BF548,
  BFIN_CPU_BF548M,
  BFIN_CPU_BF549,
  BFIN_CPU_BF549M,
  BFIN_CPU_BF561,
  BFIN_CPU_BF592,
} bfin_cpu_t;

bfin_cpu_t bfin_cpu_type = BFIN_CPU_UNKNOWN;
/* -msi-revision support. There are three special values:
   -1      -msi-revision=none.
   0xffff  -msi-revision=any.  */
int bfin_si_revision;

unsigned int bfin_anomaly_checks = 0;

struct bfin_cpu
{
  const char *name;
  bfin_cpu_t type;
  int si_revision;
  unsigned int anomaly_checks;
};

struct bfin_cpu bfin_cpus[] =
{
  {"bf504", BFIN_CPU_BF504, 0x0000, AC_05000074},

  {"bf506", BFIN_CPU_BF506, 0x0000, AC_05000074},

  {"bf512", BFIN_CPU_BF512, 0x0002, AC_05000074},
  {"bf512", BFIN_CPU_BF512, 0x0001, AC_05000074},
  {"bf512", BFIN_CPU_BF512, 0x0000, AC_05000074},

  {"bf514", BFIN_CPU_BF514, 0x0002, AC_05000074},
  {"bf514", BFIN_CPU_BF514, 0x0001, AC_05000074},
  {"bf514", BFIN_CPU_BF514, 0x0000, AC_05000074},

  {"bf516", BFIN_CPU_BF516, 0x0002, AC_05000074},
  {"bf516", BFIN_CPU_BF516, 0x0001, AC_05000074},
  {"bf516", BFIN_CPU_BF516, 0x0000, AC_05000074},

  {"bf518", BFIN_CPU_BF518, 0x0002, AC_05000074},
  {"bf518", BFIN_CPU_BF518, 0x0001, AC_05000074},
  {"bf518", BFIN_CPU_BF518, 0x0000, AC_05000074},

  {"bf522", BFIN_CPU_BF522, 0x0002, AC_05000074},
  {"bf522", BFIN_CPU_BF522, 0x0001, AC_05000074},
  {"bf522", BFIN_CPU_BF522, 0x0000, AC_05000074},

  {"bf523", BFIN_CPU_BF523, 0x0002, AC_05000074},
  {"bf523", BFIN_CPU_BF523, 0x0001, AC_05000074},
  {"bf523", BFIN_CPU_BF523, 0x0000, AC_05000074},

  {"bf524", BFIN_CPU_BF524, 0x0002, AC_05000074},
  {"bf524", BFIN_CPU_BF524, 0x0001, AC_05000074},
  {"bf524", BFIN_CPU_BF524, 0x0000, AC_05000074},

  {"bf525", BFIN_CPU_BF525, 0x0002, AC_05000074},
  {"bf525", BFIN_CPU_BF525, 0x0001, AC_05000074},
  {"bf525", BFIN_CPU_BF525, 0x0000, AC_05000074},

  {"bf526", BFIN_CPU_BF526, 0x0002, AC_05000074},
  {"bf526", BFIN_CPU_BF526, 0x0001, AC_05000074},
  {"bf526", BFIN_CPU_BF526, 0x0000, AC_05000074},

  {"bf527", BFIN_CPU_BF527, 0x0002, AC_05000074},
  {"bf527", BFIN_CPU_BF527, 0x0001, AC_05000074},
  {"bf527", BFIN_CPU_BF527, 0x0000, AC_05000074},

  {"bf531", BFIN_CPU_BF531, 0x0006, AC_05000074},
  {"bf531", BFIN_CPU_BF531, 0x0005, AC_05000074},
  {"bf531", BFIN_CPU_BF531, 0x0004, AC_05000074},
  {"bf531", BFIN_CPU_BF531, 0x0003, AC_05000074},

  {"bf532", BFIN_CPU_BF532, 0x0006, AC_05000074},
  {"bf532", BFIN_CPU_BF532, 0x0005, AC_05000074},
  {"bf532", BFIN_CPU_BF532, 0x0004, AC_05000074},
  {"bf532", BFIN_CPU_BF532, 0x0003, AC_05000074},

  {"bf533", BFIN_CPU_BF533, 0x0006, AC_05000074},
  {"bf533", BFIN_CPU_BF533, 0x0005, AC_05000074},
  {"bf533", BFIN_CPU_BF533, 0x0004, AC_05000074},
  {"bf533", BFIN_CPU_BF533, 0x0003, AC_05000074},

  {"bf534", BFIN_CPU_BF534, 0x0003, AC_05000074},
  {"bf534", BFIN_CPU_BF534, 0x0002, AC_05000074},
  {"bf534", BFIN_CPU_BF534, 0x0001, AC_05000074},

  {"bf536", BFIN_CPU_BF536, 0x0003, AC_05000074},
  {"bf536", BFIN_CPU_BF536, 0x0002, AC_05000074},
  {"bf536", BFIN_CPU_BF536, 0x0001, AC_05000074},

  {"bf537", BFIN_CPU_BF537, 0x0003, AC_05000074},
  {"bf537", BFIN_CPU_BF537, 0x0002, AC_05000074},
  {"bf537", BFIN_CPU_BF537, 0x0001, AC_05000074},

  {"bf538", BFIN_CPU_BF538, 0x0005, AC_05000074},
  {"bf538", BFIN_CPU_BF538, 0x0004, AC_05000074},
  {"bf538", BFIN_CPU_BF538, 0x0003, AC_05000074},
  {"bf538", BFIN_CPU_BF538, 0x0002, AC_05000074},

  {"bf539", BFIN_CPU_BF539, 0x0005, AC_05000074},
  {"bf539", BFIN_CPU_BF539, 0x0004, AC_05000074},
  {"bf539", BFIN_CPU_BF539, 0x0003, AC_05000074},
  {"bf539", BFIN_CPU_BF539, 0x0002, AC_05000074},

  {"bf542m", BFIN_CPU_BF542M, 0x0003, AC_05000074},

  {"bf542", BFIN_CPU_BF542, 0x0004, AC_05000074},
  {"bf542", BFIN_CPU_BF542, 0x0002, AC_05000074},
  {"bf542", BFIN_CPU_BF542, 0x0001, AC_05000074},
  {"bf542", BFIN_CPU_BF542, 0x0000, AC_05000074},

  {"bf544m", BFIN_CPU_BF544M, 0x0003, AC_05000074},

  {"bf544", BFIN_CPU_BF544, 0x0004, AC_05000074},
  {"bf544", BFIN_CPU_BF544, 0x0002, AC_05000074},
  {"bf544", BFIN_CPU_BF544, 0x0001, AC_05000074},
  {"bf544", BFIN_CPU_BF544, 0x0000, AC_05000074},

  {"bf547m", BFIN_CPU_BF547M, 0x0003, AC_05000074},

  {"bf547", BFIN_CPU_BF547, 0x0004, AC_05000074},
  {"bf547", BFIN_CPU_BF547, 0x0002, AC_05000074},
  {"bf547", BFIN_CPU_BF547, 0x0001, AC_05000074},
  {"bf547", BFIN_CPU_BF547, 0x0000, AC_05000074},

  {"bf548m", BFIN_CPU_BF548M, 0x0003, AC_05000074},

  {"bf548", BFIN_CPU_BF548, 0x0004, AC_05000074},
  {"bf548", BFIN_CPU_BF548, 0x0002, AC_05000074},
  {"bf548", BFIN_CPU_BF548, 0x0001, AC_05000074},
  {"bf548", BFIN_CPU_BF548, 0x0000, AC_05000074},

  {"bf549m", BFIN_CPU_BF549M, 0x0003, AC_05000074},

  {"bf549", BFIN_CPU_BF549, 0x0004, AC_05000074},
  {"bf549", BFIN_CPU_BF549, 0x0002, AC_05000074},
  {"bf549", BFIN_CPU_BF549, 0x0001, AC_05000074},
  {"bf549", BFIN_CPU_BF549, 0x0000, AC_05000074},

  {"bf561", BFIN_CPU_BF561, 0x0005, AC_05000074},
  {"bf561", BFIN_CPU_BF561, 0x0003, AC_05000074},
  {"bf561", BFIN_CPU_BF561, 0x0002, AC_05000074},

  {"bf592", BFIN_CPU_BF592, 0x0001, AC_05000074},
  {"bf592", BFIN_CPU_BF592, 0x0000, AC_05000074},
};

/* Define bfin-specific command-line options (there are none). */
const char *md_shortopts = "";

#define OPTION_FDPIC		(OPTION_MD_BASE)
#define OPTION_NOPIC		(OPTION_MD_BASE + 1)
#define OPTION_MCPU		(OPTION_MD_BASE + 2)

struct option md_longopts[] =
{
  { "mcpu",		required_argument,	NULL, OPTION_MCPU	},
  { "mfdpic",		no_argument,		NULL, OPTION_FDPIC      },
  { "mnopic",		no_argument,		NULL, OPTION_NOPIC      },
  { "mno-fdpic",	no_argument,		NULL, OPTION_NOPIC      },
  { NULL,		no_argument,		NULL, 0                 },
};

size_t md_longopts_size = sizeof (md_longopts);


int
md_parse_option (int c ATTRIBUTE_UNUSED, const char *arg ATTRIBUTE_UNUSED)
{
  switch (c)
    {
    default:
      return 0;

    case OPTION_MCPU:
      {
	const char *q;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE (bfin_cpus); i++)
	  {
	    const char *p = bfin_cpus[i].name;
	    if (strncmp (arg, p, strlen (p)) == 0)
	      break;
	  }

	if (i == ARRAY_SIZE (bfin_cpus))
	  as_fatal ("-mcpu=%s is not valid", arg);

	bfin_cpu_type = bfin_cpus[i].type;

	q = arg + strlen (bfin_cpus[i].name);

	if (*q == '\0')
	  {
	    bfin_si_revision = bfin_cpus[i].si_revision;
	    bfin_anomaly_checks |= bfin_cpus[i].anomaly_checks;
	  }
	else if (strcmp (q, "-none") == 0)
	  bfin_si_revision = -1;
      	else if (strcmp (q, "-any") == 0)
	  {
	    bfin_si_revision = 0xffff;
	    while (i < ARRAY_SIZE (bfin_cpus)
		   && bfin_cpus[i].type == bfin_cpu_type)
	      {
		bfin_anomaly_checks |= bfin_cpus[i].anomaly_checks;
		i++;
	      }
	  }
	else
	  {
	    unsigned int si_major, si_minor;
	    int rev_len, n;

	    rev_len = strlen (q);

	    if (sscanf (q, "-%u.%u%n", &si_major, &si_minor, &n) != 2
		|| n != rev_len
		|| si_major > 0xff || si_minor > 0xff)
	      {
	      invalid_silicon_revision:
		as_fatal ("-mcpu=%s has invalid silicon revision", arg);
	      }

	    bfin_si_revision = (si_major << 8) | si_minor;

	    while (i < ARRAY_SIZE (bfin_cpus)
		   && bfin_cpus[i].type == bfin_cpu_type
		   && bfin_cpus[i].si_revision != bfin_si_revision)
	      i++;

	    if (i == ARRAY_SIZE (bfin_cpus)
	       	|| bfin_cpus[i].type != bfin_cpu_type)
	      goto invalid_silicon_revision;

	    bfin_anomaly_checks |= bfin_cpus[i].anomaly_checks;
	  }

	break;
      }

    case OPTION_FDPIC:
      bfin_flags |= EF_BFIN_FDPIC;
      bfin_pic_flag = "-mfdpic";
      break;

    case OPTION_NOPIC:
      bfin_flags &= ~(EF_BFIN_FDPIC);
      bfin_pic_flag = 0;
      break;
    }

  return 1;
}

void
md_show_usage (FILE * stream)
{
  fprintf (stream, _(" Blackfin specific assembler options:\n"));
  fprintf (stream, _("  -mcpu=<cpu[-sirevision]> specify the name of the target CPU\n"));
  fprintf (stream, _("  -mfdpic                  assemble for the FDPIC ABI\n"));
  fprintf (stream, _("  -mno-fdpic/-mnopic       disable -mfdpic\n"));
}

/* Perform machine-specific initializations.  */
void
md_begin (void)
{
  /* Set the ELF flags if desired. */
  if (bfin_flags)
    bfd_set_private_flags (stdoutput, bfin_flags);

  /* Set the default machine type. */
  if (!bfd_set_arch_mach (stdoutput, bfd_arch_bfin, 0))
    as_warn (_("Could not set architecture and machine."));

  /* Ensure that lines can begin with '(', for multiple
     register stack pops. */
  lex_type ['('] = LEX_BEGIN_NAME;

#ifdef OBJ_ELF
  record_alignment (text_section, 2);
  record_alignment (data_section, 2);
  record_alignment (bss_section, 2);
#endif

  errorf = stderr;
  obstack_init (&mempool);

#ifdef DEBUG
  extern int debug_codeselection;
  debug_codeselection = 1;
#endif

  last_insn_size = 0;
}

/* Perform the main parsing, and assembly of the input here.  Also,
   call the required routines for alignment and fixups here.
   This is called for every line that contains real assembly code.  */

void
md_assemble (char *line)
{
  char *toP = 0;
  int size, insn_size;
  struct bfin_insn *tmp_insn;
  size_t len;
  static size_t buffer_len = 0;
  static char *current_inputline;
  parse_state state;

  len = strlen (line);
  if (len + 2 > buffer_len)
    {
      buffer_len = len + 40;
      current_inputline = XRESIZEVEC (char, current_inputline, buffer_len);
    }
  memcpy (current_inputline, line, len);
  current_inputline[len] = ';';
  current_inputline[len + 1] = '\0';

  state = parse (current_inputline);
  if (state == NO_INSN_GENERATED)
    return;

  for (insn_size = 0, tmp_insn = insn; tmp_insn; tmp_insn = tmp_insn->next)
    if (!tmp_insn->reloc || !tmp_insn->exp->symbol)
      insn_size += 2;

  if (insn_size)
    toP = frag_more (insn_size);

  last_insn_size = insn_size;

#ifdef DEBUG
  printf ("INS:");
#endif
  while (insn)
    {
      if (insn->reloc && insn->exp->symbol)
	{
	  char *prev_toP = toP - 2;
	  switch (insn->reloc)
	    {
	    case BFD_RELOC_BFIN_24_PCREL_JUMP_L:
	    case BFD_RELOC_24_PCREL:
	    case BFD_RELOC_BFIN_16_LOW:
	    case BFD_RELOC_BFIN_16_HIGH:
	      size = 4;
	      break;
	    default:
	      size = 2;
	    }

	  /* Following if condition checks for the arithmetic relocations.
	     If the case then it doesn't required to generate the code.
	     It has been assumed that, their ID will be contiguous.  */
	  if ((BFD_ARELOC_BFIN_PUSH <= insn->reloc
               && BFD_ARELOC_BFIN_COMP >= insn->reloc)
              || insn->reloc == BFD_RELOC_BFIN_16_IMM)
	    {
	      size = 2;
	    }
	  if (insn->reloc == BFD_ARELOC_BFIN_CONST
              || insn->reloc == BFD_ARELOC_BFIN_PUSH)
	    size = 4;

	  fix_new (frag_now,
                   (prev_toP - frag_now->fr_literal),
		   size, insn->exp->symbol, insn->exp->value,
                   insn->pcrel, insn->reloc);
	}
      else
	{
	  md_number_to_chars (toP, insn->value, 2);
	  toP += 2;
	}

#ifdef DEBUG
      printf (" reloc :");
      printf (" %02x%02x", ((unsigned char *) &insn->value)[0],
              ((unsigned char *) &insn->value)[1]);
      printf ("\n");
#endif
      insn = insn->next;
    }
#ifdef OBJ_ELF
  dwarf2_emit_insn (insn_size);
#endif

  while (*line++ != '\0')
    if (*line == '\n')
      bump_line_counters ();
}

/* Parse one line of instructions, and generate opcode for it.
   To parse the line, YACC and LEX are used, because the instruction set
   syntax doesn't confirm to the AT&T assembly syntax.
   To call a YACC & LEX generated parser, we must provide the input via
   a FILE stream, otherwise stdin is used by default.  Below the input
   to the function will be put into a temporary file, then the generated
   parser uses the temporary file for parsing.  */

static parse_state
parse (char *line)
{
  parse_state state;
  YY_BUFFER_STATE buffstate;

  buffstate = yy_scan_string (line);

  /* our lex requires setting the start state to keyword
     every line as the first word may be a keyword.
     Fixes a bug where we could not have keywords as labels.  */
  set_start_state ();

  /* Call yyparse here.  */
  state = yyparse ();
  if (state == SEMANTIC_ERROR)
    {
      as_bad (_("Parse failed."));
      insn = 0;
    }

  yy_delete_buffer (buffstate);
  return state;
}

/* We need to handle various expressions properly.
   Such as, [SP--] = 34, concerned by md_assemble().  */

void
md_operand (expressionS * expressionP)
{
  if (*input_line_pointer == '[')
    {
      as_tsktsk ("We found a '['!");
      input_line_pointer++;
      expression (expressionP);
    }
}

/* Handle undefined symbols. */
symbolS *
md_undefined_symbol (char *name ATTRIBUTE_UNUSED)
{
  return (symbolS *) 0;
}

int
md_estimate_size_before_relax (fragS * fragP ATTRIBUTE_UNUSED,
                               segT segment ATTRIBUTE_UNUSED)
{
  return 0;
}

/* Convert from target byte order to host byte order.  */

static int
md_chars_to_number (char *val, int n)
{
  int retval;

  for (retval = 0; n--;)
    {
      retval <<= 8;
      retval |= val[n];
    }
  return retval;
}

void
md_apply_fix (fixS *fixP, valueT *valueP, segT seg ATTRIBUTE_UNUSED)
{
  char *where = fixP->fx_frag->fr_literal + fixP->fx_where;

  long value = *valueP;
  long newval;

  switch (fixP->fx_r_type)
    {
    case BFD_RELOC_BFIN_GOT:
    case BFD_RELOC_BFIN_GOT17M4:
    case BFD_RELOC_BFIN_FUNCDESC_GOT17M4:
      fixP->fx_no_overflow = 1;
      newval = md_chars_to_number (where, 2);
      newval |= 0x0 & 0x7f;
      md_number_to_chars (where, newval, 2);
      break;

    case BFD_RELOC_BFIN_10_PCREL:
      if (!value)
	break;
      if (value < -1024 || value > 1022)
	as_bad_where (fixP->fx_file, fixP->fx_line,
                      _("pcrel too far BFD_RELOC_BFIN_10"));

      /* 11 bit offset even numbered, so we remove right bit.  */
      value = value >> 1;
      newval = md_chars_to_number (where, 2);
      newval |= value & 0x03ff;
      md_number_to_chars (where, newval, 2);
      break;

    case BFD_RELOC_BFIN_12_PCREL_JUMP:
    case BFD_RELOC_BFIN_12_PCREL_JUMP_S:
    case BFD_RELOC_12_PCREL:
      if (!value)
	break;

      if (value < -4096 || value > 4094)
	as_bad_where (fixP->fx_file, fixP->fx_line, _("pcrel too far BFD_RELOC_BFIN_12"));
      /* 13 bit offset even numbered, so we remove right bit.  */
      value = value >> 1;
      newval = md_chars_to_number (where, 2);
      newval |= value & 0xfff;
      md_number_to_chars (where, newval, 2);
      break;

    case BFD_RELOC_BFIN_16_LOW:
    case BFD_RELOC_BFIN_16_HIGH:
      fixP->fx_done = false;
      break;

    case BFD_RELOC_BFIN_24_PCREL_JUMP_L:
    case BFD_RELOC_BFIN_24_PCREL_CALL_X:
    case BFD_RELOC_24_PCREL:
      if (!value)
	break;

      if (value < -16777216 || value > 16777214)
	as_bad_where (fixP->fx_file, fixP->fx_line, _("pcrel too far BFD_RELOC_BFIN_24"));

      /* 25 bit offset even numbered, so we remove right bit.  */
      value = value >> 1;
      value++;

      md_number_to_chars (where - 2, value >> 16, 1);
      md_number_to_chars (where, value, 1);
      md_number_to_chars (where + 1, value >> 8, 1);
      break;

    case BFD_RELOC_BFIN_5_PCREL:	/* LSETUP (a, b) : "a" */
      if (!value)
	break;
      if (value < 4 || value > 30)
	as_bad_where (fixP->fx_file, fixP->fx_line, _("pcrel too far BFD_RELOC_BFIN_5"));
      value = value >> 1;
      newval = md_chars_to_number (where, 1);
      newval = (newval & 0xf0) | (value & 0xf);
      md_number_to_chars (where, newval, 1);
      break;

    case BFD_RELOC_BFIN_11_PCREL:	/* LSETUP (a, b) : "b" */
      if (!value)
	break;
      value += 2;
      if (value < 4 || value > 2046)
	as_bad_where (fixP->fx_file, fixP->fx_line, _("pcrel too far BFD_RELOC_BFIN_11_PCREL"));
      /* 11 bit unsigned even, so we remove right bit.  */
      value = value >> 1;
      newval = md_chars_to_number (where, 2);
      newval |= value & 0x03ff;
      md_number_to_chars (where, newval, 2);
      break;

    case BFD_RELOC_8:
      if (value < -0x80 || value >= 0x7f)
	as_bad_where (fixP->fx_file, fixP->fx_line, _("rel too far BFD_RELOC_8"));
      md_number_to_chars (where, value, 1);
      break;

    case BFD_RELOC_BFIN_16_IMM:
    case BFD_RELOC_16:
      if (value < -0x8000 || value >= 0x7fff)
	as_bad_where (fixP->fx_file, fixP->fx_line, _("rel too far BFD_RELOC_16"));
      md_number_to_chars (where, value, 2);
      break;

    case BFD_RELOC_32:
      md_number_to_chars (where, value, 4);
      break;

    case BFD_RELOC_BFIN_PLTPC:
      md_number_to_chars (where, value, 2);
      break;

    case BFD_RELOC_BFIN_FUNCDESC:
    case BFD_RELOC_VTABLE_INHERIT:
    case BFD_RELOC_VTABLE_ENTRY:
      fixP->fx_done = false;
      break;

    default:
      if ((BFD_ARELOC_BFIN_PUSH > fixP->fx_r_type) || (BFD_ARELOC_BFIN_COMP < fixP->fx_r_type))
	{
	  fprintf (stderr, "Relocation %d not handled in gas." " Contact support.\n", fixP->fx_r_type);
	  return;
	}
    }

  if (!fixP->fx_addsy)
    fixP->fx_done = true;

}

/* Round up a section size to the appropriate boundary.  */
valueT
md_section_align (segT segment, valueT size)
{
  int boundary = bfd_section_alignment (segment);
  return ((size + (1 << boundary) - 1) & -(1 << boundary));
}


const char *
md_atof (int type, char * litP, int * sizeP)
{
  return ieee_md_atof (type, litP, sizeP, false);
}


/* If while processing a fixup, a reloc really needs to be created
   then it is done here.  */

arelent *
tc_gen_reloc (asection *seg ATTRIBUTE_UNUSED, fixS *fixp)
{
  arelent *reloc;

  reloc		      = XNEW (arelent);
  reloc->sym_ptr_ptr  = XNEW (asymbol *);
  *reloc->sym_ptr_ptr = symbol_get_bfdsym (fixp->fx_addsy);
  reloc->address      = fixp->fx_frag->fr_address + fixp->fx_where;

  reloc->addend = fixp->fx_offset;
  reloc->howto = bfd_reloc_type_lookup (stdoutput, fixp->fx_r_type);

  if (reloc->howto == (reloc_howto_type *) NULL)
    {
      as_bad_where (fixp->fx_file, fixp->fx_line,
		    /* xgettext:c-format.  */
		    _("reloc %d not supported by object file format"),
		    (int) fixp->fx_r_type);

      xfree (reloc);

      return NULL;
    }

  return reloc;
}

/*  The location from which a PC relative jump should be calculated,
    given a PC relative reloc.  */

long
md_pcrel_from_section (fixS *fixP, segT sec)
{
  if (fixP->fx_addsy != (symbolS *) NULL
      && (!S_IS_DEFINED (fixP->fx_addsy)
      || S_GET_SEGMENT (fixP->fx_addsy) != sec))
    {
      /* The symbol is undefined (or is defined but not in this section).
         Let the linker figure it out.  */
      return 0;
    }
  return fixP->fx_frag->fr_address + fixP->fx_where;
}

/* Return true if the fix can be handled by GAS, false if it must
   be passed through to the linker.  */

bool
bfin_fix_adjustable (fixS *fixP)
{
  switch (fixP->fx_r_type)
    {
  /* Adjust_reloc_syms doesn't know about the GOT.  */
    case BFD_RELOC_BFIN_GOT:
    case BFD_RELOC_BFIN_PLTPC:
  /* We need the symbol name for the VTABLE entries.  */
    case BFD_RELOC_VTABLE_INHERIT:
    case BFD_RELOC_VTABLE_ENTRY:
      return 0;

    default:
      return 1;
    }
}

/* Special extra functions that help bfin-parse.y perform its job.  */

struct obstack mempool;

INSTR_T
conscode (INSTR_T head, INSTR_T tail)
{
  if (!head)
    return tail;
  head->next = tail;
  return head;
}

INSTR_T
conctcode (INSTR_T head, INSTR_T tail)
{
  INSTR_T temp = (head);
  if (!head)
    return tail;
  while (temp->next)
    temp = temp->next;
  temp->next = tail;

  return head;
}

INSTR_T
note_reloc (INSTR_T code, Expr_Node * symbol, int reloc, int pcrel)
{
  /* Assert that the symbol is not an operator.  */
  gas_assert (symbol->type == Expr_Node_Reloc);

  return note_reloc1 (code, symbol->value.s_value, reloc, pcrel);

}

INSTR_T
note_reloc1 (INSTR_T code, const char *symbol, int reloc, int pcrel)
{
  code->reloc = reloc;
  code->exp = mkexpr (0, symbol_find_or_make (symbol));
  code->pcrel = pcrel;
  return code;
}

INSTR_T
note_reloc2 (INSTR_T code, const char *symbol, int reloc, int value, int pcrel)
{
  code->reloc = reloc;
  code->exp = mkexpr (value, symbol_find_or_make (symbol));
  code->pcrel = pcrel;
  return code;
}

INSTR_T
gencode (unsigned long x)
{
  INSTR_T cell = XOBNEW (&mempool, struct bfin_insn);
  memset (cell, 0, sizeof (struct bfin_insn));
  cell->value = (x);
  return cell;
}

int reloc;
int ninsns;
int count_insns;

static void *
allocate (size_t n)
{
  return obstack_alloc (&mempool, n);
}

Expr_Node *
Expr_Node_Create (Expr_Node_Type type,
	          Expr_Node_Value value,
                  Expr_Node *Left_Child,
                  Expr_Node *Right_Child)
{


  Expr_Node *node = (Expr_Node *) allocate (sizeof (Expr_Node));
  node->type = type;
  node->value = value;
  node->Left_Child = Left_Child;
  node->Right_Child = Right_Child;
  return node;
}

static const char *con = ".__constant";
static const char *op = ".__operator";
static INSTR_T Expr_Node_Gen_Reloc_R (Expr_Node * head);
INSTR_T Expr_Node_Gen_Reloc (Expr_Node *head, int parent_reloc);

INSTR_T
Expr_Node_Gen_Reloc (Expr_Node * head, int parent_reloc)
{
  /* Top level relocation expression generator VDSP style.
   If the relocation is just by itself, generate one item
   else generate this convoluted expression.  */

  INSTR_T note = NULL_CODE;
  INSTR_T note1 = NULL_CODE;
  int pcrel = 1;  /* Is the parent reloc pc-relative?
		  This calculation here and HOWTO should match.  */

  if (parent_reloc)
    {
      /*  If it's 32 bit quantity then 16bit code needs to be added.  */
      int value = 0;

      if (head->type == Expr_Node_Constant)
	{
	  /* If note1 is not null code, we have to generate a right
             aligned value for the constant. Otherwise the reloc is
             a part of the basic command and the yacc file
             generates this.  */
	  value = head->value.i_value;
	}
      switch (parent_reloc)
	{
	  /*  Some relocations will need to allocate extra words.  */
	case BFD_RELOC_BFIN_16_IMM:
	case BFD_RELOC_BFIN_16_LOW:
	case BFD_RELOC_BFIN_16_HIGH:
	  note1 = conscode (gencode (value), NULL_CODE);
	  pcrel = 0;
	  break;
	case BFD_RELOC_BFIN_PLTPC:
	  note1 = conscode (gencode (value), NULL_CODE);
	  pcrel = 0;
	  break;
	case BFD_RELOC_16:
	case BFD_RELOC_BFIN_GOT:
	case BFD_RELOC_BFIN_GOT17M4:
	case BFD_RELOC_BFIN_FUNCDESC_GOT17M4:
	  note1 = conscode (gencode (value), NULL_CODE);
	  pcrel = 0;
	  break;
	case BFD_RELOC_24_PCREL:
	case BFD_RELOC_BFIN_24_PCREL_JUMP_L:
	case BFD_RELOC_BFIN_24_PCREL_CALL_X:
	  /* These offsets are even numbered pcrel.  */
	  note1 = conscode (gencode (value >> 1), NULL_CODE);
	  break;
	default:
	  note1 = NULL_CODE;
	}
    }
  if (head->type == Expr_Node_Constant)
    note = note1;
  else if (head->type == Expr_Node_Reloc)
    {
      note = note_reloc1 (gencode (0), head->value.s_value, parent_reloc, pcrel);
      if (note1 != NULL_CODE)
	note = conscode (note1, note);
    }
  else if (head->type == Expr_Node_Binop
	   && (head->value.op_value == Expr_Op_Type_Add
	       || head->value.op_value == Expr_Op_Type_Sub)
	   && head->Left_Child->type == Expr_Node_Reloc
	   && head->Right_Child->type == Expr_Node_Constant)
    {
      int val = head->Right_Child->value.i_value;
      if (head->value.op_value == Expr_Op_Type_Sub)
	val = -val;
      note = conscode (note_reloc2 (gencode (0), head->Left_Child->value.s_value,
				    parent_reloc, val, 0),
		       NULL_CODE);
      if (note1 != NULL_CODE)
	note = conscode (note1, note);
    }
  else
    {
      /* Call the recursive function.  */
      note = note_reloc1 (gencode (0), op, parent_reloc, pcrel);
      if (note1 != NULL_CODE)
	note = conscode (note1, note);
      note = conctcode (Expr_Node_Gen_Reloc_R (head), note);
    }
  return note;
}

static INSTR_T
Expr_Node_Gen_Reloc_R (Expr_Node * head)
{

  INSTR_T note = 0;
  INSTR_T note1 = 0;

  switch (head->type)
    {
    case Expr_Node_Constant:
      note = conscode (note_reloc2 (gencode (0), con, BFD_ARELOC_BFIN_CONST, head->value.i_value, 0), NULL_CODE);
      break;
    case Expr_Node_Reloc:
      note = conscode (note_reloc (gencode (0), head, BFD_ARELOC_BFIN_PUSH, 0), NULL_CODE);
      break;
    case Expr_Node_Binop:
      note1 = conctcode (Expr_Node_Gen_Reloc_R (head->Left_Child), Expr_Node_Gen_Reloc_R (head->Right_Child));
      switch (head->value.op_value)
	{
	case Expr_Op_Type_Add:
	  note = conctcode (note1, conscode (note_reloc1 (gencode (0), op, BFD_ARELOC_BFIN_ADD, 0), NULL_CODE));
	  break;
	case Expr_Op_Type_Sub:
	  note = conctcode (note1, conscode (note_reloc1 (gencode (0), op, BFD_ARELOC_BFIN_SUB, 0), NULL_CODE));
	  break;
	case Expr_Op_Type_Mult:
	  note = conctcode (note1, conscode (note_reloc1 (gencode (0), op, BFD_ARELOC_BFIN_MULT, 0), NULL_CODE));
	  break;
	case Expr_Op_Type_Div:
	  note = conctcode (note1, conscode (note_reloc1 (gencode (0), op, BFD_ARELOC_BFIN_DIV, 0), NULL_CODE));
	  break;
	case Expr_Op_Type_Mod:
	  note = conctcode (note1, conscode (note_reloc1 (gencode (0), op, BFD_ARELOC_BFIN_MOD, 0), NULL_CODE));
	  break;
	case Expr_Op_Type_Lshift:
	  note = conctcode (note1, conscode (note_reloc1 (gencode (0), op, BFD_ARELOC_BFIN_LSHIFT, 0), NULL_CODE));
	  break;
	case Expr_Op_Type_Rshift:
	  note = conctcode (note1, conscode (note_reloc1 (gencode (0), op, BFD_ARELOC_BFIN_RSHIFT, 0), NULL_CODE));
	  break;
	case Expr_Op_Type_BAND:
	  note = conctcode (note1, conscode (note_reloc1 (gencode (0), op, BFD_ARELOC_BFIN_AND, 0), NULL_CODE));
	  break;
	case Expr_Op_Type_BOR:
	  note = conctcode (note1, conscode (note_reloc1 (gencode (0), op, BFD_ARELOC_BFIN_OR, 0), NULL_CODE));
	  break;
	case Expr_Op_Type_BXOR:
	  note = conctcode (note1, conscode (note_reloc1 (gencode (0), op, BFD_ARELOC_BFIN_XOR, 0), NULL_CODE));
	  break;
	case Expr_Op_Type_LAND:
	  note = conctcode (note1, conscode (note_reloc1 (gencode (0), op, BFD_ARELOC_BFIN_LAND, 0), NULL_CODE));
	  break;
	case Expr_Op_Type_LOR:
	  note = conctcode (note1, conscode (note_reloc1 (gencode (0), op, BFD_ARELOC_BFIN_LOR, 0), NULL_CODE));
	  break;
	default:
	  fprintf (stderr, "%s:%d:Unknown operator found for arithmetic" " relocation", __FILE__, __LINE__);


	}
      break;
    case Expr_Node_Unop:
      note1 = conscode (Expr_Node_Gen_Reloc_R (head->Left_Child), NULL_CODE);
      switch (head->value.op_value)
	{
	case Expr_Op_Type_NEG:
	  note = conctcode (note1, conscode (note_reloc1 (gencode (0), op, BFD_ARELOC_BFIN_NEG, 0), NULL_CODE));
	  break;
	case Expr_Op_Type_COMP:
	  note = conctcode (note1, conscode (note_reloc1 (gencode (0), op, BFD_ARELOC_BFIN_COMP, 0), NULL_CODE));
	  break;
	default:
	  fprintf (stderr, "%s:%d:Unknown operator found for arithmetic" " relocation", __FILE__, __LINE__);
	}
      break;
    default:
      fprintf (stderr, "%s:%d:Unknown node expression found during " "arithmetic relocation generation", __FILE__, __LINE__);
    }
  return note;
}

/* Blackfin opcode generation.  */

/* These functions are called by the generated parser
   (from bfin-parse.y), the register type classification
   happens in bfin-lex.l.  */

#include "bfin-aux.h"
#include "opcode/bfin.h"

#define INIT(t)  t c_code = init_##t
#define ASSIGN(x) c_code.opcode |= ((x & c_code.mask_##x)<<c_code.bits_##x)
#define ASSIGNF(x,f) c_code.opcode |= ((x & c_code.mask_##f)<<c_code.bits_##f)
#define ASSIGN_R(x) c_code.opcode |= (((x ? (x->regno & CODE_MASK) : 0) & c_code.mask_##x)<<c_code.bits_##x)

#define HI(x) ((x >> 16) & 0xffff)
#define LO(x) ((x      ) & 0xffff)

#define GROUP(x) ((x->regno & CLASS_MASK) >> 4)

#define GEN_OPCODE32()  \
	conscode (gencode (HI (c_code.opcode)), \
	conscode (gencode (LO (c_code.opcode)), NULL_CODE))

#define GEN_OPCODE16()  \
	conscode (gencode (c_code.opcode), NULL_CODE)


/*  32 BIT INSTRUCTIONS.  */


/* DSP32 instruction generation.  */

INSTR_T
bfin_gen_dsp32mac (int op1, int MM, int mmod, int w1, int P,
	           int h01, int h11, int h00, int h10, int op0,
                   REG_T dst, REG_T src0, REG_T src1, int w0)
{
  INIT (DSP32Mac);

  ASSIGN (op0);
  ASSIGN (op1);
  ASSIGN (MM);
  ASSIGN (mmod);
  ASSIGN (w0);
  ASSIGN (w1);
  ASSIGN (h01);
  ASSIGN (h11);
  ASSIGN (h00);
  ASSIGN (h10);
  ASSIGN (P);

  /* If we have full reg assignments, mask out LSB to encode
  single or simultaneous even/odd register moves.  */
  if (P)
    {
      dst->regno &= 0x06;
    }

  ASSIGN_R (dst);
  ASSIGN_R (src0);
  ASSIGN_R (src1);

  return GEN_OPCODE32 ();
}

INSTR_T
bfin_gen_dsp32mult (int op1, int MM, int mmod, int w1, int P,
	            int h01, int h11, int h00, int h10, int op0,
                    REG_T dst, REG_T src0, REG_T src1, int w0)
{
  INIT (DSP32Mult);

  ASSIGN (op0);
  ASSIGN (op1);
  ASSIGN (MM);
  ASSIGN (mmod);
  ASSIGN (w0);
  ASSIGN (w1);
  ASSIGN (h01);
  ASSIGN (h11);
  ASSIGN (h00);
  ASSIGN (h10);
  ASSIGN (P);

  if (P)
    {
      dst->regno &= 0x06;
    }

  ASSIGN_R (dst);
  ASSIGN_R (src0);
  ASSIGN_R (src1);

  return GEN_OPCODE32 ();
}

INSTR_T
bfin_gen_dsp32alu (int HL, int aopcde, int aop, int s, int x,
              REG_T dst0, REG_T dst1, REG_T src0, REG_T src1)
{
  INIT (DSP32Alu);

  ASSIGN (HL);
  ASSIGN (aopcde);
  ASSIGN (aop);
  ASSIGN (s);
  ASSIGN (x);
  ASSIGN_R (dst0);
  ASSIGN_R (dst1);
  ASSIGN_R (src0);
  ASSIGN_R (src1);

  return GEN_OPCODE32 ();
}

INSTR_T
bfin_gen_dsp32shift (int sopcde, REG_T dst0, REG_T src0,
                REG_T src1, int sop, int HLs)
{
  INIT (DSP32Shift);

  ASSIGN (sopcde);
  ASSIGN (sop);
  ASSIGN (HLs);

  ASSIGN_R (dst0);
  ASSIGN_R (src0);
  ASSIGN_R (src1);

  return GEN_OPCODE32 ();
}

INSTR_T
bfin_gen_dsp32shiftimm (int sopcde, REG_T dst0, int immag,
                   REG_T src1, int sop, int HLs)
{
  INIT (DSP32ShiftImm);

  ASSIGN (sopcde);
  ASSIGN (sop);
  ASSIGN (HLs);

  ASSIGN_R (dst0);
  ASSIGN (immag);
  ASSIGN_R (src1);

  return GEN_OPCODE32 ();
}

/* LOOP SETUP.  */

INSTR_T
bfin_gen_loopsetup (Expr_Node * psoffset, REG_T c, int rop,
               Expr_Node * peoffset, REG_T reg)
{
  int soffset, eoffset;
  INIT (LoopSetup);

  soffset = (EXPR_VALUE (psoffset) >> 1);
  ASSIGN (soffset);
  eoffset = (EXPR_VALUE (peoffset) >> 1);
  ASSIGN (eoffset);
  ASSIGN (rop);
  ASSIGN_R (c);
  ASSIGN_R (reg);

  return
      conscode (gencode (HI (c_code.opcode)),
		conctcode (Expr_Node_Gen_Reloc (psoffset, BFD_RELOC_BFIN_5_PCREL),
			   conctcode (gencode (LO (c_code.opcode)), Expr_Node_Gen_Reloc (peoffset, BFD_RELOC_BFIN_11_PCREL))));

}

/*  Call, Link.  */

INSTR_T
bfin_gen_calla (Expr_Node * addr, int S)
{
  int val;
  int high_val;
  int rel = 0;
  INIT (CALLa);

  switch(S){
   case 0 : rel = BFD_RELOC_BFIN_24_PCREL_JUMP_L; break;
   case 1 : rel = BFD_RELOC_24_PCREL; break;
   case 2 : rel = BFD_RELOC_BFIN_PLTPC; break;
   default : break;
  }

  ASSIGN (S);

  val = EXPR_VALUE (addr) >> 1;
  high_val = val >> 16;

  return conscode (gencode (HI (c_code.opcode) | (high_val & 0xff)),
                     Expr_Node_Gen_Reloc (addr, rel));
  }

INSTR_T
bfin_gen_linkage (int R, int framesize)
{
  INIT (Linkage);

  ASSIGN (R);
  ASSIGN (framesize);

  return GEN_OPCODE32 ();
}


/* Load and Store.  */

INSTR_T
bfin_gen_ldimmhalf (REG_T reg, int H, int S, int Z, Expr_Node * phword, int rel)
{
  int grp, hword;
  unsigned val = EXPR_VALUE (phword);
  INIT (LDIMMhalf);

  ASSIGN (H);
  ASSIGN (S);
  ASSIGN (Z);

  ASSIGN_R (reg);
  grp = (GROUP (reg));
  ASSIGN (grp);
  if (rel == 2)
    {
      return conscode (gencode (HI (c_code.opcode)), Expr_Node_Gen_Reloc (phword, BFD_RELOC_BFIN_16_IMM));
    }
  else if (rel == 1)
    {
      return conscode (gencode (HI (c_code.opcode)), Expr_Node_Gen_Reloc (phword, IS_H (*reg) ? BFD_RELOC_BFIN_16_HIGH : BFD_RELOC_BFIN_16_LOW));
    }
  else
    {
      hword = val;
      ASSIGN (hword);
    }
  return GEN_OPCODE32 ();
}

INSTR_T
bfin_gen_ldstidxi (REG_T ptr, REG_T reg, int W, int sz, int Z, Expr_Node * poffset)
{
  INIT (LDSTidxI);

  if (!IS_PREG (*ptr) || (!IS_DREG (*reg) && !Z))
    {
      fprintf (stderr, "Warning: possible mixup of Preg/Dreg\n");
      return 0;
    }

  ASSIGN_R (ptr);
  ASSIGN_R (reg);
  ASSIGN (W);
  ASSIGN (sz);

  ASSIGN (Z);

  if (poffset->type != Expr_Node_Constant)
    {
      /* a GOT relocation such as R0 = [P5 + symbol@GOT] */
      /* distinguish between R0 = [P5 + symbol@GOT] and
	 P5 = [P5 + _current_shared_library_p5_offset_]
      */
      if (poffset->type == Expr_Node_Reloc
	  && !strcmp (poffset->value.s_value,
		      "_current_shared_library_p5_offset_"))
	{
	  return  conscode (gencode (HI (c_code.opcode)),
			    Expr_Node_Gen_Reloc(poffset, BFD_RELOC_16));
	}
      else if (poffset->type != Expr_Node_GOT_Reloc)
	abort ();

      return conscode (gencode (HI (c_code.opcode)),
		       Expr_Node_Gen_Reloc(poffset->Left_Child,
					   poffset->value.i_value));
    }
  else
    {
      int value, offset;
      switch (sz)
	{				/* load/store access size */
	case 0:			/* 32 bit */
	  value = EXPR_VALUE (poffset) >> 2;
	  break;
	case 1:			/* 16 bit */
	  value = EXPR_VALUE (poffset) >> 1;
	  break;
	case 2:			/* 8 bit */
	  value = EXPR_VALUE (poffset);
	  break;
	default:
	  abort ();
	}

      offset = (value & 0xffff);
      ASSIGN (offset);
      return GEN_OPCODE32 ();
    }
}


INSTR_T
bfin_gen_ldst (REG_T ptr, REG_T reg, int aop, int sz, int Z, int W)
{
  INIT (LDST);

  if (!IS_PREG (*ptr) || (!IS_DREG (*reg) && !Z))
    {
      fprintf (stderr, "Warning: possible mixup of Preg/Dreg\n");
      return 0;
    }

  ASSIGN_R (ptr);
  ASSIGN_R (reg);
  ASSIGN (aop);
  ASSIGN (sz);
  ASSIGN (Z);
  ASSIGN (W);

  return GEN_OPCODE16 ();
}

INSTR_T
bfin_gen_ldstii (REG_T ptr, REG_T reg, Expr_Node * poffset, int W, int opc)
{
  int offset;
  int value = 0;
  INIT (LDSTii);

  if (!IS_PREG (*ptr))
    {
      fprintf (stderr, "Warning: possible mixup of Preg/Dreg\n");
      return 0;
    }

  switch (opc)
    {
    case 1:
    case 2:
      value = EXPR_VALUE (poffset) >> 1;
      break;
    case 0:
    case 3:
      value = EXPR_VALUE (poffset) >> 2;
      break;
    }

  ASSIGN_R (ptr);
  ASSIGN_R (reg);

  offset = value;
  ASSIGN (offset);
  ASSIGN (W);
  ASSIGNF (opc, op);

  return GEN_OPCODE16 ();
}

INSTR_T
bfin_gen_ldstiifp (REG_T sreg, Expr_Node * poffset, int W)
{
  /* Set bit 4 if it's a Preg.  */
  int reg = (sreg->regno & CODE_MASK) | (IS_PREG (*sreg) ? 0x8 : 0x0);
  int offset = ((~(EXPR_VALUE (poffset) >> 2)) & 0x1f) + 1;
  INIT (LDSTiiFP);
  ASSIGN (reg);
  ASSIGN (offset);
  ASSIGN (W);

  return GEN_OPCODE16 ();
}

INSTR_T
bfin_gen_ldstpmod (REG_T ptr, REG_T reg, int aop, int W, REG_T idx)
{
  INIT (LDSTpmod);

  ASSIGN_R (ptr);
  ASSIGN_R (reg);
  ASSIGN (aop);
  ASSIGN (W);
  ASSIGN_R (idx);

  return GEN_OPCODE16 ();
}

INSTR_T
bfin_gen_dspldst (REG_T i, REG_T reg, int aop, int W, int m)
{
  INIT (DspLDST);

  ASSIGN_R (i);
  ASSIGN_R (reg);
  ASSIGN (aop);
  ASSIGN (W);
  ASSIGN (m);

  return GEN_OPCODE16 ();
}

INSTR_T
bfin_gen_logi2op (int opc, int src, int dst)
{
  INIT (LOGI2op);

  ASSIGN (opc);
  ASSIGN (src);
  ASSIGN (dst);

  return GEN_OPCODE16 ();
}

INSTR_T
bfin_gen_brcc (int T, int B, Expr_Node * poffset)
{
  int offset;
  INIT (BRCC);

  ASSIGN (T);
  ASSIGN (B);
  offset = ((EXPR_VALUE (poffset) >> 1));
  ASSIGN (offset);
  return conscode (gencode (c_code.opcode), Expr_Node_Gen_Reloc (poffset, BFD_RELOC_BFIN_10_PCREL));
}

INSTR_T
bfin_gen_ujump (Expr_Node * poffset)
{
  int offset;
  INIT (UJump);

  offset = ((EXPR_VALUE (poffset) >> 1));
  ASSIGN (offset);

  return conscode (gencode (c_code.opcode),
                   Expr_Node_Gen_Reloc (
                       poffset, BFD_RELOC_BFIN_12_PCREL_JUMP_S));
}

INSTR_T
bfin_gen_alu2op (REG_T dst, REG_T src, int opc)
{
  INIT (ALU2op);

  ASSIGN_R (dst);
  ASSIGN_R (src);
  ASSIGN (opc);

  return GEN_OPCODE16 ();
}

INSTR_T
bfin_gen_compi2opd (REG_T dst, int src, int opc)
{
  INIT (COMPI2opD);

  ASSIGN_R (dst);
  ASSIGN (src);
  ASSIGNF (opc, op);

  return GEN_OPCODE16 ();
}

INSTR_T
bfin_gen_compi2opp (REG_T dst, int src, int opc)
{
  INIT (COMPI2opP);

  ASSIGN_R (dst);
  ASSIGN (src);
  ASSIGNF (opc, op);

  return GEN_OPCODE16 ();
}

INSTR_T
bfin_gen_dagmodik (REG_T i, int opc)
{
  INIT (DagMODik);

  ASSIGN_R (i);
  ASSIGNF (opc, op);

  return GEN_OPCODE16 ();
}

INSTR_T
bfin_gen_dagmodim (REG_T i, REG_T m, int opc, int br)
{
  INIT (DagMODim);

  ASSIGN_R (i);
  ASSIGN_R (m);
  ASSIGNF (opc, op);
  ASSIGN (br);

  return GEN_OPCODE16 ();
}

INSTR_T
bfin_gen_ptr2op (REG_T dst, REG_T src, int opc)
{
  INIT (PTR2op);

  ASSIGN_R (dst);
  ASSIGN_R (src);
  ASSIGN (opc);

  return GEN_OPCODE16 ();
}

INSTR_T
bfin_gen_comp3op (REG_T src0, REG_T src1, REG_T dst, int opc)
{
  INIT (COMP3op);

  ASSIGN_R (src0);
  ASSIGN_R (src1);
  ASSIGN_R (dst);
  ASSIGN (opc);

  return GEN_OPCODE16 ();
}

INSTR_T
bfin_gen_ccflag (REG_T x, int y, int opc, int I, int G)
{
  INIT (CCflag);

  ASSIGN_R (x);
  ASSIGN (y);
  ASSIGN (opc);
  ASSIGN (I);
  ASSIGN (G);

  return GEN_OPCODE16 ();
}

INSTR_T
bfin_gen_ccmv (REG_T src, REG_T dst, int T)
{
  int s, d;
  INIT (CCmv);

  ASSIGN_R (src);
  ASSIGN_R (dst);
  s = (GROUP (src));
  ASSIGN (s);
  d = (GROUP (dst));
  ASSIGN (d);
  ASSIGN (T);

  return GEN_OPCODE16 ();
}

INSTR_T
bfin_gen_cc2stat (int cbit, int opc, int D)
{
  INIT (CC2stat);

  ASSIGN (cbit);
  ASSIGNF (opc, op);
  ASSIGN (D);

  return GEN_OPCODE16 ();
}

INSTR_T
bfin_gen_regmv (REG_T src, REG_T dst)
{
  int gs, gd;
  INIT (RegMv);

  ASSIGN_R (src);
  ASSIGN_R (dst);

  gs = (GROUP (src));
  ASSIGN (gs);
  gd = (GROUP (dst));
  ASSIGN (gd);

  return GEN_OPCODE16 ();
}

INSTR_T
bfin_gen_cc2dreg (int opc, REG_T reg)
{
  INIT (CC2dreg);

  ASSIGNF (opc, op);
  ASSIGN_R (reg);

  return GEN_OPCODE16 ();
}

INSTR_T
bfin_gen_progctrl (int prgfunc, int poprnd)
{
  INIT (ProgCtrl);

  ASSIGN (prgfunc);
  ASSIGN (poprnd);

  return GEN_OPCODE16 ();
}

INSTR_T
bfin_gen_cactrl (REG_T reg, int a, int opc)
{
  INIT (CaCTRL);

  ASSIGN_R (reg);
  ASSIGN (a);
  ASSIGNF (opc, op);

  return GEN_OPCODE16 ();
}

INSTR_T
bfin_gen_pushpopmultiple (int dr, int pr, int d, int p, int W)
{
  INIT (PushPopMultiple);

  ASSIGN (dr);
  ASSIGN (pr);
  ASSIGN (d);
  ASSIGN (p);
  ASSIGN (W);

  return GEN_OPCODE16 ();
}

INSTR_T
bfin_gen_pushpopreg (REG_T reg, int W)
{
  int grp;
  INIT (PushPopReg);

  ASSIGN_R (reg);
  grp = (GROUP (reg));
  ASSIGN (grp);
  ASSIGN (W);

  return GEN_OPCODE16 ();
}

/* Pseudo Debugging Support.  */

INSTR_T
bfin_gen_pseudodbg (int fn, int reg, int grp)
{
  INIT (PseudoDbg);

  ASSIGN (fn);
  ASSIGN (reg);
  ASSIGN (grp);

  return GEN_OPCODE16 ();
}

INSTR_T
bfin_gen_pseudodbg_assert (int dbgop, REG_T regtest, int expected)
{
  int grp;
  INIT (PseudoDbg_Assert);

  ASSIGN (dbgop);
  ASSIGN_R (regtest);
  grp = GROUP (regtest);
  ASSIGN (grp);
  ASSIGN (expected);

  return GEN_OPCODE32 ();
}

INSTR_T
bfin_gen_pseudochr (int ch)
{
  INIT (PseudoChr);

  ASSIGN (ch);

  return GEN_OPCODE16 ();
}

/* Multiple instruction generation.  */

INSTR_T
bfin_gen_multi_instr (INSTR_T dsp32, INSTR_T dsp16_grp1, INSTR_T dsp16_grp2)
{
  INSTR_T walk;

  /* If it's a 0, convert into MNOP. */
  if (dsp32)
    {
      walk = dsp32->next;
      SET_MULTI_INSTRUCTION_BIT (dsp32);
    }
  else
    {
      dsp32 = gencode (0xc803);
      walk = gencode (0x1800);
      dsp32->next = walk;
    }

  if (!dsp16_grp1)
    {
      dsp16_grp1 = gencode (0x0000);
    }

  if (!dsp16_grp2)
    {
      dsp16_grp2 = gencode (0x0000);
    }

  walk->next = dsp16_grp1;
  dsp16_grp1->next = dsp16_grp2;
  dsp16_grp2->next = NULL_CODE;

  return dsp32;
}

INSTR_T
bfin_gen_loop (Expr_Node *exp, REG_T reg, int rop, REG_T preg)
{
  const char *loopsym;
  char *lbeginsym, *lendsym;
  Expr_Node_Value lbeginval, lendval;
  Expr_Node *lbegin, *lend;
  symbolS *sym;

  loopsym = exp->value.s_value;
  lbeginsym = (char *) xmalloc (strlen (loopsym) + strlen ("__BEGIN") + 5);
  lendsym = (char *) xmalloc (strlen (loopsym) + strlen ("__END") + 5);

  lbeginsym[0] = 0;
  lendsym[0] = 0;

  strcat (lbeginsym, "L$L$");
  strcat (lbeginsym, loopsym);
  strcat (lbeginsym, "__BEGIN");

  strcat (lendsym, "L$L$");
  strcat (lendsym, loopsym);
  strcat (lendsym, "__END");

  lbeginval.s_value = lbeginsym;
  lendval.s_value = lendsym;

  lbegin = Expr_Node_Create (Expr_Node_Reloc, lbeginval, NULL, NULL);
  lend   = Expr_Node_Create (Expr_Node_Reloc, lendval, NULL, NULL);

  sym = symbol_find(loopsym);
  if (!S_IS_LOCAL (sym) || (S_IS_LOCAL (sym) && !symbol_used_p (sym)))
    symbol_remove (sym, &symbol_rootP, &symbol_lastP);

  return bfin_gen_loopsetup (lbegin, reg, rop, lend, preg);
}

void
bfin_loop_attempt_create_label (Expr_Node *exp, int is_begin)
{
  char *name;
  name = fb_label_name (exp->value.i_value, is_begin);
  exp->value.s_value = xstrdup (name);
  exp->type = Expr_Node_Reloc;
}

void
bfin_loop_beginend (Expr_Node *exp, int begin)
{
  const char *loopsym;
  char *label_name;
  symbolS *linelabel;
  const char *suffix = begin ? "__BEGIN" : "__END";

  loopsym = exp->value.s_value;
  label_name = (char *) xmalloc (strlen (loopsym) + strlen (suffix) + 5);

  label_name[0] = 0;

  strcat (label_name, "L$L$");
  strcat (label_name, loopsym);
  strcat (label_name, suffix);

  linelabel = colon (label_name);

  /* LOOP_END follows the last instruction in the loop.
     Adjust label address.  */
  if (!begin)
    *symbol_X_add_number (linelabel) -= last_insn_size;
}

bool
bfin_eol_in_insn (char *line)
{
   /* Allow a new-line to appear in the middle of a multi-issue instruction.  */

   char *temp = line;

  if (*line != '\n')
    return false;

  /* A semi-colon followed by a newline is always the end of a line.  */
  if (line[-1] == ';')
    return false;

  if (line[-1] == '|')
    return true;

  /* If the || is on the next line, there might be leading whitespace.  */
  temp++;
  while (*temp == ' ' || *temp == '\t') temp++;

  if (*temp == '|')
    return true;

  return false;
}

bool
bfin_start_label (char *s)
{
  while (*s != 0)
    {
      if (*s == '(' || *s == '[')
	return false;
      s++;
    }

  return true;
}

int
bfin_force_relocation (struct fix *fixp)
{
  if (fixp->fx_r_type ==BFD_RELOC_BFIN_16_LOW
      || fixp->fx_r_type == BFD_RELOC_BFIN_16_HIGH)
    return true;

  return generic_force_reloc (fixp);
}

/* This is a stripped down version of the disassembler.  The only thing it
   does is return a mask of registers modified by an instruction.  Only
   instructions that can occur in a parallel-issue bundle are handled, and
   only the registers that can cause a conflict are recorded.  */

#define DREG_MASK(n) (0x101 << (n))
#define DREGH_MASK(n) (0x100 << (n))
#define DREGL_MASK(n) (0x001 << (n))
#define IREG_MASK(n) (1 << ((n) + 16))

static int
decode_ProgCtrl_0 (int iw0)
{
  if (iw0 == 0)
    return 0;
  abort ();
}

static int
decode_LDSTpmod_0 (int iw0)
{
  /* LDSTpmod
     +---+---+---+---|---+---+---+---|---+---+---+---|---+---+---+---+
     | 1 | 0 | 0 | 0 |.W.|.aop...|.reg.......|.idx.......|.ptr.......|
     +---+---+---+---|---+---+---+---|---+---+---+---|---+---+---+---+  */
  int W   = ((iw0 >> LDSTpmod_W_bits) & LDSTpmod_W_mask);
  int aop = ((iw0 >> LDSTpmod_aop_bits) & LDSTpmod_aop_mask);
  int idx = ((iw0 >> LDSTpmod_idx_bits) & LDSTpmod_idx_mask);
  int ptr = ((iw0 >> LDSTpmod_ptr_bits) & LDSTpmod_ptr_mask);
  int reg = ((iw0 >> LDSTpmod_reg_bits) & LDSTpmod_reg_mask);

  if (aop == 1 && W == 0 && idx == ptr)
    return DREGL_MASK (reg);
  else if (aop == 2 && W == 0 && idx == ptr)
    return DREGH_MASK (reg);
  else if (aop == 1 && W == 1 && idx == ptr)
    return 0;
  else if (aop == 2 && W == 1 && idx == ptr)
    return 0;
  else if (aop == 0 && W == 0)
    return DREG_MASK (reg);
  else if (aop == 1 && W == 0)
    return DREGL_MASK (reg);
  else if (aop == 2 && W == 0)
    return DREGH_MASK (reg);
  else if (aop == 3 && W == 0)
    return DREG_MASK (reg);
  else if (aop == 3 && W == 1)
    return DREG_MASK (reg);
  else if (aop == 0 && W == 1)
    return 0;
  else if (aop == 1 && W == 1)
    return 0;
  else if (aop == 2 && W == 1)
    return 0;
  else
    return 0;

  return 2;
}

static int
decode_dagMODim_0 (int iw0)
{
  /* dagMODim
     +---+---+---+---|---+---+---+---|---+---+---+---|---+---+---+---+
     | 1 | 0 | 0 | 1 | 1 | 1 | 1 | 0 |.br| 1 | 1 |.op|.m.....|.i.....|
     +---+---+---+---|---+---+---+---|---+---+---+---|---+---+---+---+  */
  int i  = ((iw0 >> DagMODim_i_bits) & DagMODim_i_mask);
  int opc  = ((iw0 >> DagMODim_op_bits) & DagMODim_op_mask);

  if (opc == 0 || opc == 1)
    return IREG_MASK (i);
  else
    return 0;

  return 2;
}

static int
decode_dagMODik_0 (int iw0)
{
  /* dagMODik
     +---+---+---+---|---+---+---+---|---+---+---+---|---+---+---+---+
     | 1 | 0 | 0 | 1 | 1 | 1 | 1 | 1 | 0 | 1 | 1 | 0 |.op....|.i.....|
     +---+---+---+---|---+---+---+---|---+---+---+---|---+---+---+---+  */
  int i  = ((iw0 >> DagMODik_i_bits) & DagMODik_i_mask);
  return IREG_MASK (i);
}

/* GOOD */
static int
decode_dspLDST_0 (int iw0)
{
  /* dspLDST
     +---+---+---+---|---+---+---+---|---+---+---+---|---+---+---+---+
     | 1 | 0 | 0 | 1 | 1 | 1 |.W.|.aop...|.m.....|.i.....|.reg.......|
     +---+---+---+---|---+---+---+---|---+---+---+---|---+---+---+---+  */
  int i   = ((iw0 >> DspLDST_i_bits) & DspLDST_i_mask);
  int m   = ((iw0 >> DspLDST_m_bits) & DspLDST_m_mask);
  int W   = ((iw0 >> DspLDST_W_bits) & DspLDST_W_mask);
  int aop = ((iw0 >> DspLDST_aop_bits) & DspLDST_aop_mask);
  int reg = ((iw0 >> DspLDST_reg_bits) & DspLDST_reg_mask);

  if (aop == 0 && W == 0 && m == 0)
    return DREG_MASK (reg) | IREG_MASK (i);
  else if (aop == 0 && W == 0 && m == 1)
    return DREGL_MASK (reg) | IREG_MASK (i);
  else if (aop == 0 && W == 0 && m == 2)
    return DREGH_MASK (reg) | IREG_MASK (i);
  else if (aop == 1 && W == 0 && m == 0)
    return DREG_MASK (reg) | IREG_MASK (i);
  else if (aop == 1 && W == 0 && m == 1)
    return DREGL_MASK (reg) | IREG_MASK (i);
  else if (aop == 1 && W == 0 && m == 2)
    return DREGH_MASK (reg) | IREG_MASK (i);
  else if (aop == 2 && W == 0 && m == 0)
    return DREG_MASK (reg);
  else if (aop == 2 && W == 0 && m == 1)
    return DREGL_MASK (reg);
  else if (aop == 2 && W == 0 && m == 2)
    return DREGH_MASK (reg);
  else if (aop == 0 && W == 1 && m == 0)
    return IREG_MASK (i);
  else if (aop == 0 && W == 1 && m == 1)
    return IREG_MASK (i);
  else if (aop == 0 && W == 1 && m == 2)
    return IREG_MASK (i);
  else if (aop == 1 && W == 1 && m == 0)
    return IREG_MASK (i);
  else if (aop == 1 && W == 1 && m == 1)
    return IREG_MASK (i);
  else if (aop == 1 && W == 1 && m == 2)
    return IREG_MASK (i);
  else if (aop == 2 && W == 1 && m == 0)
    return 0;
  else if (aop == 2 && W == 1 && m == 1)
    return 0;
  else if (aop == 2 && W == 1 && m == 2)
    return 0;
  else if (aop == 3 && W == 0)
    return DREG_MASK (reg) | IREG_MASK (i);
  else if (aop == 3 && W == 1)
    return IREG_MASK (i);

  abort ();
}

/* GOOD */
static int
decode_LDST_0 (int iw0)
{
  /* LDST
     +---+---+---+---|---+---+---+---|---+---+---+---|---+---+---+---+
     | 1 | 0 | 0 | 1 |.sz....|.W.|.aop...|.Z.|.ptr.......|.reg.......|
     +---+---+---+---|---+---+---+---|---+---+---+---|---+---+---+---+  */
  int Z   = ((iw0 >> LDST_Z_bits) & LDST_Z_mask);
  int W   = ((iw0 >> LDST_W_bits) & LDST_W_mask);
  int sz  = ((iw0 >> LDST_sz_bits) & LDST_sz_mask);
  int aop = ((iw0 >> LDST_aop_bits) & LDST_aop_mask);
  int reg = ((iw0 >> LDST_reg_bits) & LDST_reg_mask);

  if (aop == 0 && sz == 0 && Z == 0 && W == 0)
    return DREG_MASK (reg);
  else if (aop == 0 && sz == 0 && Z == 1 && W == 0)
    return 0;
  else if (aop == 0 && sz == 1 && Z == 0 && W == 0)
    return DREG_MASK (reg);
  else if (aop == 0 && sz == 1 && Z == 1 && W == 0)
    return DREG_MASK (reg);
  else if (aop == 0 && sz == 2 && Z == 0 && W == 0)
    return DREG_MASK (reg);
  else if (aop == 0 && sz == 2 && Z == 1 && W == 0)
    return DREG_MASK (reg);
  else if (aop == 1 && sz == 0 && Z == 0 && W == 0)
    return DREG_MASK (reg);
  else if (aop == 1 && sz == 0 && Z == 1 && W == 0)
    return 0;
  else if (aop == 1 && sz == 1 && Z == 0 && W == 0)
    return DREG_MASK (reg);
  else if (aop == 1 && sz == 1 && Z == 1 && W == 0)
    return DREG_MASK (reg);
  else if (aop == 1 && sz == 2 && Z == 0 && W == 0)
    return DREG_MASK (reg);
  else if (aop == 1 && sz == 2 && Z == 1 && W == 0)
    return DREG_MASK (reg);
  else if (aop == 2 && sz == 0 && Z == 0 && W == 0)
    return DREG_MASK (reg);
  else if (aop == 2 && sz == 0 && Z == 1 && W == 0)
    return 0;
  else if (aop == 2 && sz == 1 && Z == 0 && W == 0)
    return DREG_MASK (reg);
  else if (aop == 2 && sz == 1 && Z == 1 && W == 0)
    return DREG_MASK (reg);
  else if (aop == 2 && sz == 2 && Z == 0 && W == 0)
    return DREG_MASK (reg);
  else if (aop == 2 && sz == 2 && Z == 1 && W == 0)
    return DREG_MASK (reg);
  else if (aop == 0 && sz == 0 && Z == 0 && W == 1)
    return 0;
  else if (aop == 0 && sz == 0 && Z == 1 && W == 1)
    return 0;
  else if (aop == 0 && sz == 1 && Z == 0 && W == 1)
    return 0;
  else if (aop == 0 && sz == 2 && Z == 0 && W == 1)
    return 0;
  else if (aop == 1 && sz == 0 && Z == 0 && W == 1)
    return 0;
  else if (aop == 1 && sz == 0 && Z == 1 && W == 1)
    return 0;
  else if (aop == 1 && sz == 1 && Z == 0 && W == 1)
    return 0;
  else if (aop == 1 && sz == 2 && Z == 0 && W == 1)
    return 0;
  else if (aop == 2 && sz == 0 && Z == 0 && W == 1)
    return 0;
  else if (aop == 2 && sz == 0 && Z == 1 && W == 1)
    return 0;
  else if (aop == 2 && sz == 1 && Z == 0 && W == 1)
    return 0;
  else if (aop == 2 && sz == 2 && Z == 0 && W == 1)
    return 0;

  abort ();
}

static int
decode_LDSTiiFP_0 (int iw0)
{
  /* LDSTiiFP
     +---+---+---+---|---+---+---+---|---+---+---+---|---+---+---+---+
     | 1 | 0 | 1 | 1 | 1 | 0 |.W.|.offset............|.reg...........|
     +---+---+---+---|---+---+---+---|---+---+---+---|---+---+---+---+  */
  int reg = ((iw0 >> LDSTiiFP_reg_bits) & LDSTiiFP_reg_mask);
  int W = ((iw0 >> LDSTiiFP_W_bits) & LDSTiiFP_W_mask);

  if (W == 0)
    return reg < 8 ? DREG_MASK (reg) : 0;
  else
    return 0;
}

static int
decode_LDSTii_0 (int iw0)
{
  /* LDSTii
     +---+---+---+---|---+---+---+---|---+---+---+---|---+---+---+---+
     | 1 | 0 | 1 |.W.|.op....|.offset........|.ptr.......|.reg.......|
     +---+---+---+---|---+---+---+---|---+---+---+---|---+---+---+---+  */
  int reg = ((iw0 >> LDSTii_reg_bit) & LDSTii_reg_mask);
  int opc = ((iw0 >> LDSTii_op_bit) & LDSTii_op_mask);
  int W = ((iw0 >> LDSTii_W_bit) & LDSTii_W_mask);

  if (W == 0 && opc != 3)
    return DREG_MASK (reg);
  else if (W == 0 && opc == 3)
   return 0;
  else if (W == 1 && opc == 0)
    return 0;
  else if (W == 1 && opc == 1)
    return 0;
  else if (W == 1 && opc == 3)
    return 0;

  abort ();
}

static int
decode_dsp32mac_0 (int iw0, int iw1)
{
  int result = 0;
  /* dsp32mac
     +---+---+---+---|---+---+---+---|---+---+---+---|---+---+---+---+
     | 1 | 1 | 0 | 0 |.M.| 0 | 0 |.mmod..........|.MM|.P.|.w1|.op1...|
     |.h01|.h11|.w0|.op0...|.h00|.h10|.dst.......|.src0......|.src1..|
     +---+---+---+---|---+---+---+---|---+---+---+---|---+---+---+---+  */
  int op1  = ((iw0 >> (DSP32Mac_op1_bits - 16)) & DSP32Mac_op1_mask);
  int w1   = ((iw0 >> (DSP32Mac_w1_bits - 16)) & DSP32Mac_w1_mask);
  int P    = ((iw0 >> (DSP32Mac_p_bits - 16)) & DSP32Mac_p_mask);
  int mmod = ((iw0 >> (DSP32Mac_mmod_bits - 16)) & DSP32Mac_mmod_mask);
  int w0   = ((iw1 >> DSP32Mac_w0_bits) & DSP32Mac_w0_mask);
  int MM   = ((iw1 >> DSP32Mac_MM_bits) & DSP32Mac_MM_mask);
  int dst  = ((iw1 >> DSP32Mac_dst_bits) & DSP32Mac_dst_mask);
  int op0  = ((iw1 >> DSP32Mac_op0_bits) & DSP32Mac_op0_mask);

  if (w0 == 0 && w1 == 0 && op1 == 3 && op0 == 3)
    return 0;

  if (op1 == 3 && MM)
    return 0;

  if ((w1 || w0) && mmod == M_W32)
    return 0;

  if (((1 << mmod) & (P ? 0x131b : 0x1b5f)) == 0)
    return 0;

  if (w1 == 1 || op1 != 3)
    {
      if (w1)
	{
	  if (P)
	    return DREG_MASK (dst + 1);
	  else
	    return DREGH_MASK (dst);
	}
    }

  if (w0 == 1 || op0 != 3)
    {
      if (w0)
	{
	  if (P)
	    return DREG_MASK (dst);
	  else
	    return DREGL_MASK (dst);
	}
    }

  return result;
}

static int
decode_dsp32mult_0 (int iw0, int iw1)
{
  /* dsp32mult
     +---+---+---+---|---+---+---+---|---+---+---+---|---+---+---+---+
     | 1 | 1 | 0 | 0 |.M.| 0 | 1 |.mmod..........|.MM|.P.|.w1|.op1...|
     |.h01|.h11|.w0|.op0...|.h00|.h10|.dst.......|.src0......|.src1..|
     +---+---+---+---|---+---+---+---|---+---+---+---|---+---+---+---+  */
  int w1   = ((iw0 >> (DSP32Mac_w1_bits - 16)) & DSP32Mac_w1_mask);
  int P    = ((iw0 >> (DSP32Mac_p_bits - 16)) & DSP32Mac_p_mask);
  int mmod = ((iw0 >> (DSP32Mac_mmod_bits - 16)) & DSP32Mac_mmod_mask);
  int w0   = ((iw1 >> DSP32Mac_w0_bits) & DSP32Mac_w0_mask);
  int dst  = ((iw1 >> DSP32Mac_dst_bits) & DSP32Mac_dst_mask);
  int result = 0;

  if (w1 == 0 && w0 == 0)
    return 0;

  if (((1 << mmod) & (P ? 0x313 : 0x1b57)) == 0)
    return 0;

  if (w1)
    {
      if (P)
	return DREG_MASK (dst | 1);
      else
	return DREGH_MASK (dst);
    }

  if (w0)
    {
      if (P)
	return DREG_MASK (dst);
      else
	return DREGL_MASK (dst);
    }

  return result;
}

static int
decode_dsp32alu_0 (int iw0, int iw1)
{
  /* dsp32alu
     +---+---+---+---|---+---+---+---|---+---+---+---|---+---+---+---+
     | 1 | 1 | 0 | 0 |.M.| 1 | 0 | - | - | - |.HL|.aopcde............|
     |.aop...|.s.|.x.|.dst0......|.dst1......|.src0......|.src1......|
     +---+---+---+---|---+---+---+---|---+---+---+---|---+---+---+---+  */
  int s    = ((iw1 >> DSP32Alu_s_bits) & DSP32Alu_s_mask);
  int x    = ((iw1 >> DSP32Alu_x_bits) & DSP32Alu_x_mask);
  int aop  = ((iw1 >> DSP32Alu_aop_bits) & DSP32Alu_aop_mask);
  int dst0 = ((iw1 >> DSP32Alu_dst0_bits) & DSP32Alu_dst0_mask);
  int dst1 = ((iw1 >> DSP32Alu_dst1_bits) & DSP32Alu_dst1_mask);
  int HL   = ((iw0 >> (DSP32Alu_HL_bits - 16)) & DSP32Alu_HL_mask);
  int aopcde = ((iw0 >> (DSP32Alu_aopcde_bits - 16)) & DSP32Alu_aopcde_mask);

  if (aop == 0 && aopcde == 9 && s == 0)
    return 0;
  else if (aop == 2 && aopcde == 9 && HL == 0 && s == 0)
    return 0;
  else if (aop >= x * 2 && aopcde == 5)
    return HL ? DREGH_MASK (dst0) : DREGL_MASK (dst0);
  else if (HL == 0 && aopcde == 2)
    return DREGL_MASK (dst0);
  else if (HL == 1 && aopcde == 2)
    return DREGH_MASK (dst0);
  else if (HL == 0 && aopcde == 3)
    return DREGL_MASK (dst0);
  else if (HL == 1 && aopcde == 3)
    return DREGH_MASK (dst0);

  else if (aop == 0 && aopcde == 9 && s == 1)
    return 0;
  else if (aop == 1 && aopcde == 9 && s == 0)
    return 0;
  else if (aop == 2 && aopcde == 9 && s == 1)
    return 0;
  else if (aop == 3 && aopcde == 9 && s == 0)
    return 0;
  else if (aopcde == 8)
    return 0;
  else if (aop == 0 && aopcde == 11)
    return DREG_MASK (dst0);
  else if (aop == 1 && aopcde == 11)
    return HL ? DREGH_MASK (dst0) : DREGL_MASK (dst0);
  else if (aopcde == 11)
    return 0;
  else if (aopcde == 22)
    return DREG_MASK (dst0);

  else if ((aop == 0 || aop == 1) && aopcde == 14)
    return 0;
  else if (aop == 3 && HL == 0 && aopcde == 14)
    return 0;

  else if (aop == 3 && HL == 0 && aopcde == 15)
    return DREG_MASK (dst0);

  else if (aop == 1 && aopcde == 16)
    return 0;

  else if (aop == 0 && aopcde == 16)
    return 0;

  else if (aop == 3 && HL == 0 && aopcde == 16)
    return 0;

  else if (aop == 3 && HL == 0 && aopcde == 7)
    return DREG_MASK (dst0);
  else if ((aop == 0 || aop == 1 || aop == 2) && aopcde == 7)
    return DREG_MASK (dst0);

  else if (aop == 0 && aopcde == 12)
    return DREG_MASK (dst0);
  else if (aop == 1 && aopcde == 12)
    return DREG_MASK (dst0) | DREG_MASK (dst1);
  else if (aop == 3 && aopcde == 12)
    return HL ? DREGH_MASK (dst0) : DREGL_MASK (dst0);

  else if (aopcde == 0)
    return DREG_MASK (dst0);
  else if (aopcde == 1)
    return DREG_MASK (dst0) | DREG_MASK (dst1);

  else if (aop == 0 && aopcde == 10)
    return DREGL_MASK (dst0);
  else if (aop == 1 && aopcde == 10)
    return DREGL_MASK (dst0);

  else if ((aop == 1 || aop == 0) && aopcde == 4)
    return DREG_MASK (dst0);
  else if (aop == 2 && aopcde == 4)
    return DREG_MASK (dst0) | DREG_MASK (dst1);

  else if (aop == 0 && aopcde == 17)
    return DREG_MASK (dst0) | DREG_MASK (dst1);
  else if (aop == 1 && aopcde == 17)
    return DREG_MASK (dst0) | DREG_MASK (dst1);
  else if (aop == 0 && aopcde == 18)
    return 0;
  else if (aop == 3 && aopcde == 18)
    return 0;

  else if ((aop == 0 || aop == 1 || aop == 2) && aopcde == 6)
    return DREG_MASK (dst0);

  else if ((aop == 0 || aop == 1) && aopcde == 20)
    return DREG_MASK (dst0);

  else if ((aop == 0 || aop == 1) && aopcde == 21)
    return DREG_MASK (dst0) | DREG_MASK (dst1);

  else if (aop == 0 && aopcde == 23 && HL == 1)
    return DREG_MASK (dst0);
  else if (aop == 0 && aopcde == 23 && HL == 0)
    return DREG_MASK (dst0);

  else if (aop == 0 && aopcde == 24)
    return DREG_MASK (dst0);
  else if (aop == 1 && aopcde == 24)
    return DREG_MASK (dst0) | DREG_MASK (dst1);
  else if (aopcde == 13)
    return DREG_MASK (dst0) | DREG_MASK (dst1);
  else
    return 0;

  return 4;
}

static int
decode_dsp32shift_0 (int iw0, int iw1)
{
  /* dsp32shift
     +---+---+---+---|---+---+---+---|---+---+---+---|---+---+---+---+
     | 1 | 1 | 0 | 0 |.M.| 1 | 1 | 0 | 0 | - | - |.sopcde............|
     |.sop...|.HLs...|.dst0......| - | - | - |.src0......|.src1......|
     +---+---+---+---|---+---+---+---|---+---+---+---|---+---+---+---+  */
  int HLs  = ((iw1 >> DSP32Shift_HLs_bits) & DSP32Shift_HLs_mask);
  int sop  = ((iw1 >> DSP32Shift_sop_bits) & DSP32Shift_sop_mask);
  int src0 = ((iw1 >> DSP32Shift_src0_bits) & DSP32Shift_src0_mask);
  int src1 = ((iw1 >> DSP32Shift_src1_bits) & DSP32Shift_src1_mask);
  int dst0 = ((iw1 >> DSP32Shift_dst0_bits) & DSP32Shift_dst0_mask);
  int sopcde = ((iw0 >> (DSP32Shift_sopcde_bits - 16)) & DSP32Shift_sopcde_mask);

  if (sop == 0 && sopcde == 0)
    return HLs & 2 ? DREGH_MASK (dst0) : DREGL_MASK (dst0);
  else if (sop == 1 && sopcde == 0)
    return HLs & 2 ? DREGH_MASK (dst0) : DREGL_MASK (dst0);
  else if (sop == 2 && sopcde == 0)
    return HLs & 2 ? DREGH_MASK (dst0) : DREGL_MASK (dst0);
  else if (sop == 0 && sopcde == 3)
    return 0;
  else if (sop == 1 && sopcde == 3)
    return 0;
  else if (sop == 2 && sopcde == 3)
    return 0;
  else if (sop == 3 && sopcde == 3)
    return DREG_MASK (dst0);
  else if (sop == 0 && sopcde == 1)
    return DREG_MASK (dst0);
  else if (sop == 1 && sopcde == 1)
    return DREG_MASK (dst0);
  else if (sop == 2 && sopcde == 1)
    return DREG_MASK (dst0);
  else if (sopcde == 2)
    return DREG_MASK (dst0);
  else if (sopcde == 4)
    return DREG_MASK (dst0);
  else if (sop == 0 && sopcde == 5)
    return DREGL_MASK (dst0);
  else if (sop == 1 && sopcde == 5)
    return DREGL_MASK (dst0);
  else if (sop == 2 && sopcde == 5)
    return DREGL_MASK (dst0);
  else if (sop == 0 && sopcde == 6)
    return DREGL_MASK (dst0);
  else if (sop == 1 && sopcde == 6)
    return DREGL_MASK (dst0);
  else if (sop == 3 && sopcde == 6)
    return DREGL_MASK (dst0);
  else if (sop == 0 && sopcde == 7)
    return DREGL_MASK (dst0);
  else if (sop == 1 && sopcde == 7)
    return DREGL_MASK (dst0);
  else if (sop == 2 && sopcde == 7)
    return DREGL_MASK (dst0);
  else if (sop == 3 && sopcde == 7)
    return DREGL_MASK (dst0);
  else if (sop == 0 && sopcde == 8)
    return DREG_MASK (src0) | DREG_MASK (src1);
#if 0
    {
      OUTS (outf, "BITMUX (");
      OUTS (outf, dregs (src0));
      OUTS (outf, ", ");
      OUTS (outf, dregs (src1));
      OUTS (outf, ", A0) (ASR)");
    }
#endif
  else if (sop == 1 && sopcde == 8)
    return DREG_MASK (src0) | DREG_MASK (src1);
#if 0
    {
      OUTS (outf, "BITMUX (");
      OUTS (outf, dregs (src0));
      OUTS (outf, ", ");
      OUTS (outf, dregs (src1));
      OUTS (outf, ", A0) (ASL)");
    }
#endif
  else if (sopcde == 9)
    return sop < 2 ? DREGL_MASK (dst0) : DREG_MASK (dst0);
  else if (sopcde == 10)
    return DREG_MASK (dst0);
  else if (sop == 0 && sopcde == 11)
    return DREGL_MASK (dst0);
  else if (sop == 1 && sopcde == 11)
    return DREGL_MASK (dst0);
  else if (sop == 0 && sopcde == 12)
    return 0;
  else if (sop == 1 && sopcde == 12)
    return DREGL_MASK (dst0);
  else if (sop == 0 && sopcde == 13)
    return DREG_MASK (dst0);
  else if (sop == 1 && sopcde == 13)
    return DREG_MASK (dst0);
  else if (sop == 2 && sopcde == 13)
    return DREG_MASK (dst0);

  abort ();
}

static int
decode_dsp32shiftimm_0 (int iw0, int iw1)
{
  /* dsp32shiftimm
     +---+---+---+---|---+---+---+---|---+---+---+---|---+---+---+---+
     | 1 | 1 | 0 | 0 |.M.| 1 | 1 | 0 | 1 | - | - |.sopcde............|
     |.sop...|.HLs...|.dst0......|.immag.................|.src1......|
     +---+---+---+---|---+---+---+---|---+---+---+---|---+---+---+---+  */
  int sop      = ((iw1 >> DSP32ShiftImm_sop_bits) & DSP32ShiftImm_sop_mask);
  int bit8     = ((iw1 >> 8) & 0x1);
  int dst0     = ((iw1 >> DSP32ShiftImm_dst0_bits) & DSP32ShiftImm_dst0_mask);
  int sopcde   = ((iw0 >> (DSP32ShiftImm_sopcde_bits - 16)) & DSP32ShiftImm_sopcde_mask);
  int HLs      = ((iw1 >> DSP32ShiftImm_HLs_bits) & DSP32ShiftImm_HLs_mask);


  if (sop == 0 && sopcde == 0)
    return HLs & 2 ? DREGH_MASK (dst0) : DREGL_MASK (dst0);
  else if (sop == 1 && sopcde == 0 && bit8 == 0)
    return HLs & 2 ? DREGH_MASK (dst0) : DREGL_MASK (dst0);
  else if (sop == 1 && sopcde == 0 && bit8 == 1)
    return HLs & 2 ? DREGH_MASK (dst0) : DREGL_MASK (dst0);
  else if (sop == 2 && sopcde == 0 && bit8 == 0)
    return HLs & 2 ? DREGH_MASK (dst0) : DREGL_MASK (dst0);
  else if (sop == 2 && sopcde == 0 && bit8 == 1)
    return HLs & 2 ? DREGH_MASK (dst0) : DREGL_MASK (dst0);
  else if (sop == 2 && sopcde == 3 && HLs == 1)
    return 0;
  else if (sop == 0 && sopcde == 3 && HLs == 0 && bit8 == 0)
    return 0;
  else if (sop == 0 && sopcde == 3 && HLs == 0 && bit8 == 1)
    return 0;
  else if (sop == 0 && sopcde == 3 && HLs == 1 && bit8 == 0)
    return 0;
  else if (sop == 0 && sopcde == 3 && HLs == 1 && bit8 == 1)
    return 0;
  else if (sop == 1 && sopcde == 3 && HLs == 0)
    return 0;
  else if (sop == 1 && sopcde == 3 && HLs == 1)
    return 0;
  else if (sop == 2 && sopcde == 3 && HLs == 0)
    return 0;
  else if (sop == 1 && sopcde == 1 && bit8 == 0)
    return DREG_MASK (dst0);
  else if (sop == 1 && sopcde == 1 && bit8 == 1)
    return DREG_MASK (dst0);
  else if (sop == 2 && sopcde == 1 && bit8 == 1)
    return DREG_MASK (dst0);
  else if (sop == 2 && sopcde == 1 && bit8 == 0)
    return DREG_MASK (dst0);
  else if (sop == 0 && sopcde == 1)
    return DREG_MASK (dst0);
  else if (sop == 1 && sopcde == 2)
    return DREG_MASK (dst0);
  else if (sop == 2 && sopcde == 2 && bit8 == 1)
    return DREG_MASK (dst0);
  else if (sop == 2 && sopcde == 2 && bit8 == 0)
    return DREG_MASK (dst0);
  else if (sop == 3 && sopcde == 2)
    return DREG_MASK (dst0);
  else if (sop == 0 && sopcde == 2)
    return DREG_MASK (dst0);

  abort ();
}

int
insn_regmask (int iw0, int iw1)
{
  if ((iw0 & 0xf7ff) == 0xc003 && iw1 == 0x1800)
    return 0; /* MNOP */
  else if ((iw0 & 0xff00) == 0x0000)
    return decode_ProgCtrl_0 (iw0);
  else if ((iw0 & 0xffc0) == 0x0240)
    abort ();
  else if ((iw0 & 0xff80) == 0x0100)
    abort ();
  else if ((iw0 & 0xfe00) == 0x0400)
    abort ();
  else if ((iw0 & 0xfe00) == 0x0600)
    abort ();
  else if ((iw0 & 0xf800) == 0x0800)
    abort ();
  else if ((iw0 & 0xffe0) == 0x0200)
    abort ();
  else if ((iw0 & 0xff00) == 0x0300)
    abort ();
  else if ((iw0 & 0xf000) == 0x1000)
    abort ();
  else if ((iw0 & 0xf000) == 0x2000)
    abort ();
  else if ((iw0 & 0xf000) == 0x3000)
    abort ();
  else if ((iw0 & 0xfc00) == 0x4000)
    abort ();
  else if ((iw0 & 0xfe00) == 0x4400)
    abort ();
  else if ((iw0 & 0xf800) == 0x4800)
    abort ();
  else if ((iw0 & 0xf000) == 0x5000)
    abort ();
  else if ((iw0 & 0xf800) == 0x6000)
    abort ();
  else if ((iw0 & 0xf800) == 0x6800)
    abort ();
  else if ((iw0 & 0xf000) == 0x8000)
    return decode_LDSTpmod_0 (iw0);
  else if ((iw0 & 0xff60) == 0x9e60)
    return decode_dagMODim_0 (iw0);
  else if ((iw0 & 0xfff0) == 0x9f60)
    return decode_dagMODik_0 (iw0);
  else if ((iw0 & 0xfc00) == 0x9c00)
    return decode_dspLDST_0 (iw0);
  else if ((iw0 & 0xf000) == 0x9000)
    return decode_LDST_0 (iw0);
  else if ((iw0 & 0xfc00) == 0xb800)
    return decode_LDSTiiFP_0 (iw0);
  else if ((iw0 & 0xe000) == 0xA000)
    return decode_LDSTii_0 (iw0);
  else if ((iw0 & 0xff80) == 0xe080 && (iw1 & 0x0C00) == 0x0000)
    abort ();
  else if ((iw0 & 0xff00) == 0xe100 && (iw1 & 0x0000) == 0x0000)
    abort ();
  else if ((iw0 & 0xfe00) == 0xe200 && (iw1 & 0x0000) == 0x0000)
    abort ();
  else if ((iw0 & 0xfc00) == 0xe400 && (iw1 & 0x0000) == 0x0000)
    abort ();
  else if ((iw0 & 0xfffe) == 0xe800 && (iw1 & 0x0000) == 0x0000)
    abort ();
  else if ((iw0 & 0xf600) == 0xc000 && (iw1 & 0x0000) == 0x0000)
    return decode_dsp32mac_0 (iw0, iw1);
  else if ((iw0 & 0xf600) == 0xc200 && (iw1 & 0x0000) == 0x0000)
    return decode_dsp32mult_0 (iw0, iw1);
  else if ((iw0 & 0xf7c0) == 0xc400 && (iw1 & 0x0000) == 0x0000)
    return decode_dsp32alu_0 (iw0, iw1);
  else if ((iw0 & 0xf780) == 0xc600 && (iw1 & 0x01c0) == 0x0000)
    return decode_dsp32shift_0 (iw0, iw1);
  else if ((iw0 & 0xf780) == 0xc680 && (iw1 & 0x0000) == 0x0000)
    return decode_dsp32shiftimm_0 (iw0, iw1);
  else if ((iw0 & 0xff00) == 0xf800)
    abort ();
  else if ((iw0 & 0xFFC0) == 0xf000 && (iw1 & 0x0000) == 0x0000)
    abort ();

  abort ();
}
