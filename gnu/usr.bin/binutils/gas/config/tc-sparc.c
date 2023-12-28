/* tc-sparc.c -- Assemble for the SPARC
   Copyright (C) 1989-2023 Free Software Foundation, Inc.
   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public
   License along with GAS; see the file COPYING.  If not, write
   to the Free Software Foundation, 51 Franklin Street - Fifth Floor,
   Boston, MA 02110-1301, USA.  */

#include "as.h"
#include "safe-ctype.h"
#include "subsegs.h"

#include "opcode/sparc.h"
#include "dw2gencfi.h"

#include "elf/sparc.h"
#include "dwarf2dbg.h"

/* Some ancient Sun C compilers would not take such hex constants as
   unsigned, and would end up sign-extending them to form an offsetT,
   so use these constants instead.  */
#define U0xffffffff ((((unsigned long) 1 << 16) << 16) - 1)
#define U0x80000000 ((((unsigned long) 1 << 16) << 15))

static int sparc_ip (char *, const struct sparc_opcode **);
static int parse_sparc_asi (char **, const sparc_asi **);
static int parse_keyword_arg (int (*) (const char *), char **, int *);
static int parse_const_expr_arg (char **, int *);
static int get_expression (char *);

/* Default architecture.  */
/* ??? The default value should be V8, but sparclite support was added
   by making it the default.  GCC now passes -Asparclite, so maybe sometime in
   the future we can set this to V8.  */
#ifndef DEFAULT_ARCH
#define DEFAULT_ARCH "sparclite"
#endif
static const char *default_arch = DEFAULT_ARCH;

/* Non-zero if the initial values of `max_architecture' and `sparc_arch_size'
   have been set.  */
static int default_init_p;

/* Current architecture.  We don't bump up unless necessary.  */
static enum sparc_opcode_arch_val current_architecture = SPARC_OPCODE_ARCH_V6;

/* The maximum architecture level we can bump up to.
   In a 32 bit environment, don't allow bumping up to v9 by default.
   The native assembler works this way.  The user is required to pass
   an explicit argument before we'll create v9 object files.  However, if
   we don't see any v9 insns, a v8plus object file is not created.  */
static enum sparc_opcode_arch_val max_architecture;

/* Either 32 or 64, selects file format.  */
static int sparc_arch_size;
/* Initial (default) value, recorded separately in case a user option
   changes the value before md_show_usage is called.  */
static int default_arch_size;

/* The currently selected v9 memory model.  Currently only used for
   ELF.  */
static enum { MM_TSO, MM_PSO, MM_RMO } sparc_memory_model = MM_RMO;

#ifndef TE_SOLARIS
/* Bitmask of instruction types seen so far, used to populate the
   GNU attributes section with hwcap information.  */
static uint64_t hwcap_seen;
#endif

static uint64_t hwcap_allowed;

static int architecture_requested;
static int warn_on_bump;

/* If warn_on_bump and the needed architecture is higher than this
   architecture, issue a warning.  */
static enum sparc_opcode_arch_val warn_after_architecture;

/* Non-zero if the assembler should generate error if an undeclared
   g[23] register has been used in -64.  */
static int no_undeclared_regs;

/* Non-zero if the assembler should generate a warning if an
   unpredictable DCTI (delayed control transfer instruction) couple is
   found.  */
static int dcti_couples_detect;

/* Non-zero if we should try to relax jumps and calls.  */
static int sparc_relax;

/* Non-zero if we are generating PIC code.  */
int sparc_pic_code;

/* Non-zero if we should give an error when misaligned data is seen.  */
static int enforce_aligned_data;

extern int target_big_endian;

static int target_little_endian_data;

/* Symbols for global registers on v9.  */
static symbolS *globals[8];

/* The dwarf2 data alignment, adjusted for 32 or 64 bit.  */
int sparc_cie_data_alignment;

/* V9 and 86x have big and little endian data, but instructions are always big
   endian.  The sparclet has bi-endian support but both data and insns have
   the same endianness.  Global `target_big_endian' is used for data.
   The following macro is used for instructions.  */
#ifndef INSN_BIG_ENDIAN
#define INSN_BIG_ENDIAN (target_big_endian \
			 || default_arch_type == sparc86x \
			 || SPARC_OPCODE_ARCH_V9_P (max_architecture))
#endif

/* Handle of the OPCODE hash table.  */
static htab_t op_hash;

static void s_data1 (void);
static void s_seg (int);
static void s_proc (int);
static void s_reserve (int);
static void s_common (int);
static void s_empty (int);
static void s_uacons (int);
static void s_ncons (int);
static void s_register (int);

const pseudo_typeS md_pseudo_table[] =
{
  {"align", s_align_bytes, 0},	/* Defaulting is invalid (0).  */
  {"common", s_common, 0},
  {"empty", s_empty, 0},
  {"global", s_globl, 0},
  {"half", cons, 2},
  {"nword", s_ncons, 0},
  {"optim", s_ignore, 0},
  {"proc", s_proc, 0},
  {"reserve", s_reserve, 0},
  {"seg", s_seg, 0},
  {"skip", s_space, 0},
  {"word", cons, 4},
  {"xword", cons, 8},
  {"uahalf", s_uacons, 2},
  {"uaword", s_uacons, 4},
  {"uaxword", s_uacons, 8},
  /* These are specific to sparc/svr4.  */
  {"2byte", s_uacons, 2},
  {"4byte", s_uacons, 4},
  {"8byte", s_uacons, 8},
  {"register", s_register, 0},
  {NULL, 0, 0},
};

/* This array holds the chars that always start a comment.  If the
   pre-processor is disabled, these aren't very useful.  */
const char comment_chars[] = "!";	/* JF removed '|' from
                                           comment_chars.  */

/* This array holds the chars that only start a comment at the beginning of
   a line.  If the line seems to have the form '# 123 filename'
   .line and .file directives will appear in the pre-processed output.  */
/* Note that input_file.c hand checks for '#' at the beginning of the
   first line of the input file.  This is because the compiler outputs
   #NO_APP at the beginning of its output.  */
/* Also note that comments started like this one will always
   work if '/' isn't otherwise defined.  */
const char line_comment_chars[] = "#";

const char line_separator_chars[] = ";";

/* Chars that can be used to separate mant from exp in floating point
   nums.  */
const char EXP_CHARS[] = "eE";

/* Chars that mean this number is a floating point constant.
   As in 0f12.456
   or    0d1.2345e12  */
const char FLT_CHARS[] = "rRsSfFdDxXpP";

/* Also be aware that MAXIMUM_NUMBER_OF_CHARS_FOR_FLOAT may have to be
   changed in read.c.  Ideally it shouldn't have to know about it at all,
   but nothing is ideal around here.  */

#define isoctal(c)  ((unsigned) ((c) - '0') < 8)

struct sparc_it
  {
    const char *error;
    unsigned long opcode;
    struct nlist *nlistp;
    expressionS exp;
    expressionS exp2;
    int pcrel;
    bfd_reloc_code_real_type reloc;
  };

struct sparc_it the_insn, set_insn;

static void output_insn (const struct sparc_opcode *, struct sparc_it *);

/* Table of arguments to -A.
   The sparc_opcode_arch table in sparc-opc.c is insufficient and incorrect
   for this use.  That table is for opcodes only.  This table is for opcodes
   and file formats.  */

enum sparc_arch_types {v6, v7, v8, leon, sparclet, sparclite, sparc86x, v8plus,
		       v8plusa, v9, v9a, v9b, v9_64};

static struct sparc_arch {
  const char *name;
  const char *opcode_arch;
  enum sparc_arch_types arch_type;
  /* Default word size, as specified during configuration.
     A value of zero means can't be used to specify default architecture.  */
  int default_arch_size;
  /* Allowable arg to -A?  */
  int user_option_p;
  /* Extra hardware capabilities allowed.  These are added to the
     hardware capabilities associated with the opcode
     architecture.  */
  int hwcap_allowed;
  int hwcap2_allowed;
} sparc_arch_table[] = {
  { "v6",         "v6",  v6,  0, 1, 0, 0 },
  { "v7",         "v7",  v7,  0, 1, 0, 0 },
  { "v8",         "v8",  v8, 32, 1, 0, 0 },
  { "v8a",        "v8",  v8, 32, 1, 0, 0 },
  { "sparc",      "v9",  v9,  0, 1, HWCAP_V8PLUS, 0 },
  { "sparcvis",   "v9a", v9,  0, 1, 0, 0 },
  { "sparcvis2",  "v9b", v9,  0, 1, 0, 0 },
  { "sparcfmaf",  "v9b", v9,  0, 1, HWCAP_FMAF, 0 },
  { "sparcima",   "v9b", v9,  0, 1, HWCAP_FMAF|HWCAP_IMA, 0 },
  { "sparcvis3",  "v9b", v9,  0, 1, HWCAP_FMAF|HWCAP_VIS3|HWCAP_HPC, 0 },
  { "sparcvis3r", "v9b", v9,  0, 1, HWCAP_FMAF|HWCAP_VIS3|HWCAP_HPC|HWCAP_FJFMAU, 0 },

  { "sparc4",     "v9v", v9,  0, 1, 0, 0 },
  { "sparc5",     "v9m", v9,  0, 1, 0, 0 },
  { "sparc6",     "m8",  v9,  0, 1, 0, 0 },

  { "leon",      "leon",      leon,      32, 1, 0, 0 },
  { "sparclet",  "sparclet",  sparclet,  32, 1, 0, 0 },
  { "sparclite", "sparclite", sparclite, 32, 1, 0, 0 },
  { "sparc86x",  "sparclite", sparc86x,  32, 1, 0, 0 },

  { "v8plus",  "v9",  v9,  0, 1, HWCAP_V8PLUS, 0 },
  { "v8plusa", "v9a", v9,  0, 1, HWCAP_V8PLUS, 0 },
  { "v8plusb", "v9b", v9,  0, 1, HWCAP_V8PLUS, 0 },
  { "v8plusc", "v9c", v9,  0, 1, HWCAP_V8PLUS, 0 },
  { "v8plusd", "v9d", v9,  0, 1, HWCAP_V8PLUS, 0 },
  { "v8pluse", "v9e", v9,  0, 1, HWCAP_V8PLUS, 0 },
  { "v8plusv", "v9v", v9,  0, 1, HWCAP_V8PLUS, 0 },
  { "v8plusm", "v9m", v9,  0, 1, HWCAP_V8PLUS, 0 },
  { "v8plusm8", "m8", v9,  0, 1, HWCAP_V8PLUS, 0 },
  
  { "v9",      "v9",  v9,  0, 1, 0, 0 },
  { "v9a",     "v9a", v9,  0, 1, 0, 0 },
  { "v9b",     "v9b", v9,  0, 1, 0, 0 },
  { "v9c",     "v9c", v9,  0, 1, 0, 0 },
  { "v9d",     "v9d", v9,  0, 1, 0, 0 },
  { "v9e",     "v9e", v9,  0, 1, 0, 0 },
  { "v9v",     "v9v", v9,  0, 1, 0, 0 },
  { "v9m",     "v9m", v9,  0, 1, 0, 0 },
  { "v9m8",     "m8", v9,  0, 1, 0, 0 },

  /* This exists to allow configure.tgt to pass one
     value to specify both the default machine and default word size.  */
  { "v9-64",   "v9",  v9, 64, 0, 0, 0 },
  { NULL, NULL, v8, 0, 0, 0, 0 }
};

/* Variant of default_arch */
static enum sparc_arch_types default_arch_type;

static struct sparc_arch *
lookup_arch (const char *name)
{
  struct sparc_arch *sa;

  for (sa = &sparc_arch_table[0]; sa->name != NULL; sa++)
    if (strcmp (sa->name, name) == 0)
      break;
  if (sa->name == NULL)
    return NULL;
  return sa;
}

/* Initialize the default opcode arch and word size from the default
   architecture name.  */

static void
init_default_arch (void)
{
  struct sparc_arch *sa = lookup_arch (default_arch);

  if (sa == NULL
      || sa->default_arch_size == 0)
    as_fatal (_("Invalid default architecture, broken assembler."));

  max_architecture = sparc_opcode_lookup_arch (sa->opcode_arch);
  if (max_architecture == SPARC_OPCODE_ARCH_BAD)
    as_fatal (_("Bad opcode table, broken assembler."));
  default_arch_size = sparc_arch_size = sa->default_arch_size;
  default_init_p = 1;
  default_arch_type = sa->arch_type;
}

/* Called by TARGET_MACH.  */

unsigned long
sparc_mach (void)
{
  /* We don't get a chance to initialize anything before we're called,
     so handle that now.  */
  if (! default_init_p)
    init_default_arch ();

  return sparc_arch_size == 64 ? bfd_mach_sparc_v9 : bfd_mach_sparc;
}

/* Called by TARGET_FORMAT.  */

const char *
sparc_target_format (void)
{
  /* We don't get a chance to initialize anything before we're called,
     so handle that now.  */
  if (! default_init_p)
    init_default_arch ();

#ifdef TE_VXWORKS
  return "elf32-sparc-vxworks";
#endif

  return sparc_arch_size == 64 ? ELF64_TARGET_FORMAT : ELF_TARGET_FORMAT;
}

/* md_parse_option
 *	Invocation line includes a switch not recognized by the base assembler.
 *	See if it's a processor-specific option.  These are:
 *
 *	-bump
 *		Warn on architecture bumps.  See also -A.
 *
 *	-Av6, -Av7, -Av8, -Aleon, -Asparclite, -Asparclet
 *		Standard 32 bit architectures.
 *	-Av9, -Av9a, -Av9b
 *		Sparc64 in either a 32 or 64 bit world (-32/-64 says which).
 *		This used to only mean 64 bits, but properly specifying it
 *		complicated gcc's ASM_SPECs, so now opcode selection is
 *		specified orthogonally to word size (except when specifying
 *		the default, but that is an internal implementation detail).
 *	-Av8plus, -Av8plusa, -Av8plusb
 *		Same as -Av9{,a,b}.
 *	-xarch=v8plus, -xarch=v8plusa, -xarch=v8plusb
 *		Same as -Av8plus{,a,b} -32, for compatibility with Sun's
 *		assembler.
 *	-xarch=v9, -xarch=v9a, -xarch=v9b
 *		Same as -Av9{,a,b} -64, for compatibility with Sun's
 *		assembler.
 *
 *		Select the architecture and possibly the file format.
 *		Instructions or features not supported by the selected
 *		architecture cause fatal errors.
 *
 *		The default is to start at v6, and bump the architecture up
 *		whenever an instruction is seen at a higher level.  In 32 bit
 *		environments, v9 is not bumped up to, the user must pass
 * 		-Av8plus{,a,b}.
 *
 *		If -bump is specified, a warning is printing when bumping to
 *		higher levels.
 *
 *		If an architecture is specified, all instructions must match
 *		that architecture.  Any higher level instructions are flagged
 *		as errors.  Note that in the 32 bit environment specifying
 *		-Av8plus does not automatically create a v8plus object file, a
 *		v9 insn must be seen.
 *
 *		If both an architecture and -bump are specified, the
 *		architecture starts at the specified level, but bumps are
 *		warnings.  Note that we can't set `current_architecture' to
 *		the requested level in this case: in the 32 bit environment,
 *		we still must avoid creating v8plus object files unless v9
 * 		insns are seen.
 *
 * Note:
 *		Bumping between incompatible architectures is always an
 *		error.  For example, from sparclite to v9.
 */

const char *md_shortopts = "A:K:VQ:sq";
struct option md_longopts[] = {
#define OPTION_BUMP (OPTION_MD_BASE)
  {"bump", no_argument, NULL, OPTION_BUMP},
#define OPTION_SPARC (OPTION_MD_BASE + 1)
  {"sparc", no_argument, NULL, OPTION_SPARC},
#define OPTION_XARCH (OPTION_MD_BASE + 2)
  {"xarch", required_argument, NULL, OPTION_XARCH},
#define OPTION_32 (OPTION_MD_BASE + 3)
  {"32", no_argument, NULL, OPTION_32},
#define OPTION_64 (OPTION_MD_BASE + 4)
  {"64", no_argument, NULL, OPTION_64},
#define OPTION_TSO (OPTION_MD_BASE + 5)
  {"TSO", no_argument, NULL, OPTION_TSO},
#define OPTION_PSO (OPTION_MD_BASE + 6)
  {"PSO", no_argument, NULL, OPTION_PSO},
#define OPTION_RMO (OPTION_MD_BASE + 7)
  {"RMO", no_argument, NULL, OPTION_RMO},
#ifdef SPARC_BIENDIAN
#define OPTION_LITTLE_ENDIAN (OPTION_MD_BASE + 8)
  {"EL", no_argument, NULL, OPTION_LITTLE_ENDIAN},
#define OPTION_BIG_ENDIAN (OPTION_MD_BASE + 9)
  {"EB", no_argument, NULL, OPTION_BIG_ENDIAN},
#endif
#define OPTION_ENFORCE_ALIGNED_DATA (OPTION_MD_BASE + 10)
  {"enforce-aligned-data", no_argument, NULL, OPTION_ENFORCE_ALIGNED_DATA},
#define OPTION_LITTLE_ENDIAN_DATA (OPTION_MD_BASE + 11)
  {"little-endian-data", no_argument, NULL, OPTION_LITTLE_ENDIAN_DATA},
#define OPTION_NO_UNDECLARED_REGS (OPTION_MD_BASE + 12)
  {"no-undeclared-regs", no_argument, NULL, OPTION_NO_UNDECLARED_REGS},
#define OPTION_UNDECLARED_REGS (OPTION_MD_BASE + 13)
  {"undeclared-regs", no_argument, NULL, OPTION_UNDECLARED_REGS},
#define OPTION_RELAX (OPTION_MD_BASE + 14)
  {"relax", no_argument, NULL, OPTION_RELAX},
#define OPTION_NO_RELAX (OPTION_MD_BASE + 15)
  {"no-relax", no_argument, NULL, OPTION_NO_RELAX},
#define OPTION_DCTI_COUPLES_DETECT (OPTION_MD_BASE + 16)
  {"dcti-couples-detect", no_argument, NULL, OPTION_DCTI_COUPLES_DETECT},
  {NULL, no_argument, NULL, 0}
};

size_t md_longopts_size = sizeof (md_longopts);

int
md_parse_option (int c, const char *arg)
{
  /* We don't get a chance to initialize anything before we're called,
     so handle that now.  */
  if (! default_init_p)
    init_default_arch ();

  switch (c)
    {
    case OPTION_BUMP:
      warn_on_bump = 1;
      warn_after_architecture = SPARC_OPCODE_ARCH_V6;
      break;

    case OPTION_XARCH:
      if (startswith (arg, "v9"))
	md_parse_option (OPTION_64, NULL);
      else
	{
	  if (startswith (arg, "v8")
	      || startswith (arg, "v7")
	      || startswith (arg, "v6")
	      || !strcmp (arg, "sparclet")
	      || !strcmp (arg, "sparclite")
	      || !strcmp (arg, "sparc86x"))
	    md_parse_option (OPTION_32, NULL);
	}
      /* Fall through.  */

    case 'A':
      {
	struct sparc_arch *sa;
	enum sparc_opcode_arch_val opcode_arch;

	sa = lookup_arch (arg);
	if (sa == NULL
	    || ! sa->user_option_p)
	  {
	    if (c == OPTION_XARCH)
	      as_bad (_("invalid architecture -xarch=%s"), arg);
	    else
	      as_bad (_("invalid architecture -A%s"), arg);
	    return 0;
	  }

	opcode_arch = sparc_opcode_lookup_arch (sa->opcode_arch);
	if (opcode_arch == SPARC_OPCODE_ARCH_BAD)
	  as_fatal (_("Bad opcode table, broken assembler."));

	if (!architecture_requested
	    || opcode_arch > max_architecture)
	  max_architecture = opcode_arch;

	/* The allowed hardware capabilities are the implied by the
	   opcodes arch plus any extra capabilities defined in the GAS
	   arch.  */
	hwcap_allowed
	  = (hwcap_allowed
	     | ((uint64_t) sparc_opcode_archs[opcode_arch].hwcaps2 << 32)
	     | ((uint64_t) sa->hwcap2_allowed << 32)
	     | sparc_opcode_archs[opcode_arch].hwcaps
	     | sa->hwcap_allowed);
	architecture_requested = 1;
      }
      break;

    case OPTION_SPARC:
      /* Ignore -sparc, used by SunOS make default .s.o rule.  */
      break;

    case OPTION_ENFORCE_ALIGNED_DATA:
      enforce_aligned_data = 1;
      break;

#ifdef SPARC_BIENDIAN
    case OPTION_LITTLE_ENDIAN:
      target_big_endian = 0;
      if (default_arch_type != sparclet)
	as_fatal ("This target does not support -EL");
      break;
    case OPTION_LITTLE_ENDIAN_DATA:
      target_little_endian_data = 1;
      target_big_endian = 0;
      if (default_arch_type != sparc86x
	  && default_arch_type != v9)
	as_fatal ("This target does not support --little-endian-data");
      break;
    case OPTION_BIG_ENDIAN:
      target_big_endian = 1;
      break;
#endif

    case OPTION_32:
    case OPTION_64:
      {
	const char **list, **l;

	sparc_arch_size = c == OPTION_32 ? 32 : 64;
	list = bfd_target_list ();
	for (l = list; *l != NULL; l++)
	  {
	    if (sparc_arch_size == 32)
	      {
		if (startswith (*l, "elf32-sparc"))
		  break;
	      }
	    else
	      {
		if (startswith (*l, "elf64-sparc"))
		  break;
	      }
	  }
	if (*l == NULL)
	  as_fatal (_("No compiled in support for %d bit object file format"),
		    sparc_arch_size);
	free (list);

	if (sparc_arch_size == 64
	    && max_architecture < SPARC_OPCODE_ARCH_V9)
	  max_architecture = SPARC_OPCODE_ARCH_V9;
      }
      break;

    case OPTION_TSO:
      sparc_memory_model = MM_TSO;
      break;

    case OPTION_PSO:
      sparc_memory_model = MM_PSO;
      break;

    case OPTION_RMO:
      sparc_memory_model = MM_RMO;
      break;

    case 'V':
      print_version_id ();
      break;

    case 'Q':
      /* Qy - do emit .comment
	 Qn - do not emit .comment.  */
      break;

    case 's':
      /* Use .stab instead of .stab.excl.  */
      break;

    case 'q':
      /* quick -- Native assembler does fewer checks.  */
      break;

    case 'K':
      if (strcmp (arg, "PIC") != 0)
	as_warn (_("Unrecognized option following -K"));
      else
	sparc_pic_code = 1;
      break;

    case OPTION_NO_UNDECLARED_REGS:
      no_undeclared_regs = 1;
      break;

    case OPTION_UNDECLARED_REGS:
      no_undeclared_regs = 0;
      break;

    case OPTION_RELAX:
      sparc_relax = 1;
      break;

    case OPTION_NO_RELAX:
      sparc_relax = 0;
      break;

    case OPTION_DCTI_COUPLES_DETECT:
      dcti_couples_detect = 1;
      break;

    default:
      return 0;
    }

  return 1;
}

void
md_show_usage (FILE *stream)
{
  const struct sparc_arch *arch;
  int column;

  /* We don't get a chance to initialize anything before we're called,
     so handle that now.  */
  if (! default_init_p)
    init_default_arch ();

  fprintf (stream, _("SPARC options:\n"));
  column = 0;
  for (arch = &sparc_arch_table[0]; arch->name; arch++)
    {
      if (!arch->user_option_p)
	continue;
      if (arch != &sparc_arch_table[0])
	fprintf (stream, " | ");
      if (column + strlen (arch->name) > 70)
	{
	  column = 0;
	  fputc ('\n', stream);
	}
      column += 5 + 2 + strlen (arch->name);
      fprintf (stream, "-A%s", arch->name);
    }
  for (arch = &sparc_arch_table[0]; arch->name; arch++)
    {
      if (!arch->user_option_p)
	continue;
      fprintf (stream, " | ");
      if (column + strlen (arch->name) > 65)
	{
	  column = 0;
	  fputc ('\n', stream);
	}
      column += 5 + 7 + strlen (arch->name);
      fprintf (stream, "-xarch=%s", arch->name);
    }
  fprintf (stream, _("\n\
			specify variant of SPARC architecture\n\
-bump			warn when assembler switches architectures\n\
-sparc			ignored\n\
--enforce-aligned-data	force .long, etc., to be aligned correctly\n\
-relax			relax jumps and branches (default)\n\
-no-relax		avoid changing any jumps and branches\n"));
  fprintf (stream, _("\
-32			create 32 bit object file\n\
-64			create 64 bit object file\n"));
  fprintf (stream, _("\
			[default is %d]\n"), default_arch_size);
  fprintf (stream, _("\
-TSO			use Total Store Ordering\n\
-PSO			use Partial Store Ordering\n\
-RMO			use Relaxed Memory Ordering\n"));
  fprintf (stream, _("\
			[default is %s]\n"), (default_arch_size == 64) ? "RMO" : "TSO");
  fprintf (stream, _("\
-KPIC			generate PIC\n\
-V			print assembler version number\n\
-undeclared-regs	ignore application global register usage without\n\
			appropriate .register directive (default)\n\
-no-undeclared-regs	force error on application global register usage\n\
			without appropriate .register directive\n\
--dcti-couples-detect	warn when an unpredictable DCTI couple is found\n\
-q			ignored\n\
-Qy, -Qn		ignored\n\
-s			ignored\n"));
#ifdef SPARC_BIENDIAN
  fprintf (stream, _("\
-EL			generate code for a little endian machine\n\
-EB			generate code for a big endian machine\n\
--little-endian-data	generate code for a machine having big endian\n\
                        instructions and little endian data.\n"));
#endif
}

/* Native operand size opcode translation.  */
static struct
  {
    const char *name;
    const char *name32;
    const char *name64;
  } native_op_table[] =
{
  {"ldn", "ld", "ldx"},
  {"ldna", "lda", "ldxa"},
  {"stn", "st", "stx"},
  {"stna", "sta", "stxa"},
  {"slln", "sll", "sllx"},
  {"srln", "srl", "srlx"},
  {"sran", "sra", "srax"},
  {"casn", "cas", "casx"},
  {"casna", "casa", "casxa"},
  {"clrn", "clr", "clrx"},
  {NULL, NULL, NULL},
};

/* sparc64 privileged and hyperprivileged registers.  */

struct priv_reg_entry
{
  const char *name;
  int regnum;
};

struct priv_reg_entry priv_reg_table[] =
{
  {"tpc", 0},
  {"tnpc", 1},
  {"tstate", 2},
  {"tt", 3},
  {"tick", 4},
  {"tba", 5},
  {"pstate", 6},
  {"tl", 7},
  {"pil", 8},
  {"cwp", 9},
  {"cansave", 10},
  {"canrestore", 11},
  {"cleanwin", 12},
  {"otherwin", 13},
  {"wstate", 14},
  {"fq", 15},
  {"gl", 16},
  {"pmcdper", 23},
  {"ver", 31},
  {NULL, -1},			/* End marker.  */
};

struct priv_reg_entry hpriv_reg_table[] =
{
  {"hpstate", 0},
  {"htstate", 1},
  {"hintp", 3},
  {"htba", 5},
  {"hver", 6},
  {"hmcdper", 23},
  {"hmcddfr", 24},
  {"hva_mask_nz", 27},
  {"hstick_offset", 28},
  {"hstick_enable", 29},
  {"hstick_cmpr", 31},
  {NULL, -1},			/* End marker.  */
};

/* v9a or later specific ancillary state registers. */

struct priv_reg_entry v9a_asr_table[] =
{
  {"tick_cmpr", 23},
  {"sys_tick_cmpr", 25},
  {"sys_tick", 24},
  {"stick_cmpr", 25},
  {"stick", 24},
  {"softint_clear", 21},
  {"softint_set", 20},
  {"softint", 22},
  {"set_softint", 20},
  {"pause", 27},
  {"pic", 17},
  {"pcr", 16},
  {"mwait", 28},
  {"gsr", 19},
  {"dcr", 18},
  {"cfr", 26},
  {"clear_softint", 21},
  {NULL, -1},			/* End marker.  */
};

static int
cmp_reg_entry (const void *parg, const void *qarg)
{
  const struct priv_reg_entry *p = (const struct priv_reg_entry *) parg;
  const struct priv_reg_entry *q = (const struct priv_reg_entry *) qarg;

  if (p->name == q->name)
    return 0;
  else if (p->name == NULL)
    return 1;
  else if (q->name == NULL)
    return -1;
  else
    return strcmp (q->name, p->name);
}

/* sparc %-pseudo-operations.  */


#define F_POP_V9       0x1 /* The pseudo-op is for v9 only.  */
#define F_POP_PCREL    0x2 /* The pseudo-op can be used in pc-relative
                              contexts.  */
#define F_POP_TLS_CALL 0x4 /* The pseudo-op marks a tls call.  */
#define F_POP_POSTFIX  0x8 /* The pseudo-op should appear after the
                              last operand of an
                              instruction. (Generally they can appear
                              anywhere an immediate operand is
                              expected.  */
struct pop_entry
{
  /* The name as it appears in assembler.  */
  const char *name;
  /* The reloc this pseudo-op translates to.  */
  bfd_reloc_code_real_type reloc;
  /* Flags.  See F_POP_* above.  */
  int flags;
};

struct pop_entry pop_table[] =
{
  { "hix",		BFD_RELOC_SPARC_HIX22,		F_POP_V9 },
  { "lox",		BFD_RELOC_SPARC_LOX10, 		F_POP_V9 },
  { "hi",		BFD_RELOC_HI22,			F_POP_PCREL },
  { "lo",		BFD_RELOC_LO10,			F_POP_PCREL },
  { "pc22",		BFD_RELOC_SPARC_PC22,		F_POP_PCREL },
  { "pc10",		BFD_RELOC_SPARC_PC10,		F_POP_PCREL },
  { "hh",		BFD_RELOC_SPARC_HH22,		F_POP_V9|F_POP_PCREL },
  { "hm",		BFD_RELOC_SPARC_HM10,		F_POP_V9|F_POP_PCREL },
  { "lm",		BFD_RELOC_SPARC_LM22,		F_POP_V9|F_POP_PCREL },
  { "h34",		BFD_RELOC_SPARC_H34,		F_POP_V9 },
  { "l34",		BFD_RELOC_SPARC_L44,		F_POP_V9 },
  { "h44",		BFD_RELOC_SPARC_H44,		F_POP_V9 },
  { "m44",		BFD_RELOC_SPARC_M44,		F_POP_V9 },
  { "l44",		BFD_RELOC_SPARC_L44,		F_POP_V9 },
  { "uhi",		BFD_RELOC_SPARC_HH22,		F_POP_V9 },
  { "ulo",		BFD_RELOC_SPARC_HM10,		F_POP_V9 },
  { "tgd_hi22",		BFD_RELOC_SPARC_TLS_GD_HI22, 	0 },
  { "tgd_lo10",		BFD_RELOC_SPARC_TLS_GD_LO10, 	0 },
  { "tldm_hi22",	BFD_RELOC_SPARC_TLS_LDM_HI22, 	0 },
  { "tldm_lo10",	BFD_RELOC_SPARC_TLS_LDM_LO10, 	0 },
  { "tldo_hix22",	BFD_RELOC_SPARC_TLS_LDO_HIX22, 	0 },
  { "tldo_lox10",	BFD_RELOC_SPARC_TLS_LDO_LOX10, 	0 },
  { "tie_hi22",		BFD_RELOC_SPARC_TLS_IE_HI22, 	0 },
  { "tie_lo10",		BFD_RELOC_SPARC_TLS_IE_LO10, 	0 },
  { "tle_hix22",	BFD_RELOC_SPARC_TLS_LE_HIX22, 	0 },
  { "tle_lox10",	BFD_RELOC_SPARC_TLS_LE_LOX10, 	0 },
  { "gdop_hix22",	BFD_RELOC_SPARC_GOTDATA_OP_HIX22, 0 },
  { "gdop_lox10",	BFD_RELOC_SPARC_GOTDATA_OP_LOX10, 0 },
  { "tgd_add", 		BFD_RELOC_SPARC_TLS_GD_ADD,	F_POP_POSTFIX },
  { "tgd_call",		BFD_RELOC_SPARC_TLS_GD_CALL, 	F_POP_POSTFIX|F_POP_TLS_CALL },
  { "tldm_add",		BFD_RELOC_SPARC_TLS_LDM_ADD, 	F_POP_POSTFIX },
  { "tldm_call",	BFD_RELOC_SPARC_TLS_LDM_CALL,	F_POP_POSTFIX|F_POP_TLS_CALL },
  { "tldo_add",		BFD_RELOC_SPARC_TLS_LDO_ADD, 	F_POP_POSTFIX },
  { "tie_ldx",		BFD_RELOC_SPARC_TLS_IE_LDX, 	F_POP_POSTFIX },
  { "tie_ld",		BFD_RELOC_SPARC_TLS_IE_LD,	F_POP_POSTFIX },
  { "tie_add",		BFD_RELOC_SPARC_TLS_IE_ADD,	F_POP_POSTFIX },
  { "gdop",	 	BFD_RELOC_SPARC_GOTDATA_OP,	F_POP_POSTFIX }
};

/* Table of %-names that can appear in a sparc assembly program.  This
   table is initialized in md_begin and contains entries for each
   privileged/hyperprivileged/alternate register and %-pseudo-op.  */

enum perc_entry_type
{
  perc_entry_none = 0,
  perc_entry_reg,
  perc_entry_post_pop,
  perc_entry_imm_pop
};

struct perc_entry
{
  /* Entry type.  */
  enum perc_entry_type type;
  /* Name of the %-entity.  */
  const char *name;
  /* strlen (name).  */
  int len;
  /* Value.  Either a pop or a reg depending on type.*/
  union
  {
    struct pop_entry *pop;
    struct priv_reg_entry *reg;
  };
};

#define NUM_PERC_ENTRIES \
  (((sizeof (priv_reg_table) / sizeof (priv_reg_table[0])) - 1)         \
   + ((sizeof (hpriv_reg_table) / sizeof (hpriv_reg_table[0])) - 1)     \
   + ((sizeof (v9a_asr_table) / sizeof (v9a_asr_table[0])) - 1)         \
   + ARRAY_SIZE (pop_table)						\
   + 1)

struct perc_entry perc_table[NUM_PERC_ENTRIES];

static int
cmp_perc_entry (const void *parg, const void *qarg)
{
  const struct perc_entry *p = (const struct perc_entry *) parg;
  const struct perc_entry *q = (const struct perc_entry *) qarg;

  if (p->name == q->name)
    return 0;
  else if (p->name == NULL)
    return 1;
  else if (q->name == NULL)
    return -1;
  else
    return strcmp (q->name, p->name);
}

/* This function is called once, at assembler startup time.  It should
   set up all the tables, etc. that the MD part of the assembler will
   need.  */

void
md_begin (void)
{
  int lose = 0;
  unsigned int i = 0;

  /* We don't get a chance to initialize anything before md_parse_option
     is called, and it may not be called, so handle default initialization
     now if not already done.  */
  if (! default_init_p)
    init_default_arch ();

  sparc_cie_data_alignment = sparc_arch_size == 64 ? -8 : -4;
  op_hash = str_htab_create ();

  while (i < (unsigned int) sparc_num_opcodes)
    {
      const char *name = sparc_opcodes[i].name;
      if (str_hash_insert (op_hash, name, &sparc_opcodes[i], 0) != NULL)
	{
	  as_bad (_("duplicate %s"), name);
	  lose = 1;
	}
      do
	{
	  if (sparc_opcodes[i].match & sparc_opcodes[i].lose)
	    {
	      as_bad (_("Internal error: losing opcode: `%s' \"%s\"\n"),
		      sparc_opcodes[i].name, sparc_opcodes[i].args);
	      lose = 1;
	    }
	  ++i;
	}
      while (i < (unsigned int) sparc_num_opcodes
	     && !strcmp (sparc_opcodes[i].name, name));
    }

  for (i = 0; native_op_table[i].name; i++)
    {
      const struct sparc_opcode *insn;
      const char *name = ((sparc_arch_size == 32)
		    ? native_op_table[i].name32
		    : native_op_table[i].name64);
      insn = (struct sparc_opcode *) str_hash_find (op_hash, name);
      if (insn == NULL)
	{
	  as_bad (_("Internal error: can't find opcode `%s' for `%s'\n"),
		  name, native_op_table[i].name);
	  lose = 1;
	}
      else if (str_hash_insert (op_hash, native_op_table[i].name, insn, 0))
	{
	  as_bad (_("duplicate %s"), native_op_table[i].name);
	  lose = 1;
	}
    }

  if (lose)
    as_fatal (_("Broken assembler.  No assembly attempted."));

  qsort (priv_reg_table, sizeof (priv_reg_table) / sizeof (priv_reg_table[0]),
	 sizeof (priv_reg_table[0]), cmp_reg_entry);
  qsort (hpriv_reg_table, sizeof (hpriv_reg_table) / sizeof (hpriv_reg_table[0]),
	 sizeof (hpriv_reg_table[0]), cmp_reg_entry);
  qsort (v9a_asr_table, sizeof (v9a_asr_table) / sizeof (v9a_asr_table[0]),
	 sizeof (v9a_asr_table[0]), cmp_reg_entry);
  
  /* If -bump, record the architecture level at which we start issuing
     warnings.  The behaviour is different depending upon whether an
     architecture was explicitly specified.  If it wasn't, we issue warnings
     for all upwards bumps.  If it was, we don't start issuing warnings until
     we need to bump beyond the requested architecture or when we bump between
     conflicting architectures.  */

  if (warn_on_bump
      && architecture_requested)
    {
      /* `max_architecture' records the requested architecture.
	 Issue warnings if we go above it.  */
      warn_after_architecture = max_architecture;
    }

  /* Find the highest architecture level that doesn't conflict with
     the requested one.  */

  if (warn_on_bump
      || !architecture_requested)
  {
    enum sparc_opcode_arch_val current_max_architecture
      = max_architecture;

    for (max_architecture = SPARC_OPCODE_ARCH_MAX;
	 max_architecture > warn_after_architecture;
	 --max_architecture)
      if (! SPARC_OPCODE_CONFLICT_P (max_architecture,
				     current_max_architecture))
	break;
  }

  /* Prepare the tables of %-pseudo-ops.  */
  {
    struct priv_reg_entry *reg_tables[]
      = {priv_reg_table, hpriv_reg_table, v9a_asr_table, NULL};
    struct priv_reg_entry **reg_table;
    int entry = 0;

    /* Add registers.  */
    for (reg_table = reg_tables; reg_table[0]; reg_table++)
      {
        struct priv_reg_entry *reg;
        for (reg = *reg_table; reg->name; reg++)
          {
            struct perc_entry *p = &perc_table[entry++];
            p->type = perc_entry_reg;
            p->name = reg->name;
            p->len = strlen (reg->name);
            p->reg = reg;
          }
      }

    /* Add %-pseudo-ops.  */
    for (i = 0; i < ARRAY_SIZE (pop_table); i++)
      {
	struct perc_entry *p = &perc_table[entry++];
	p->type = (pop_table[i].flags & F_POP_POSTFIX
		   ? perc_entry_post_pop : perc_entry_imm_pop);
	p->name = pop_table[i].name;
	p->len = strlen (pop_table[i].name);
	p->pop = &pop_table[i];
      }

    /* Last entry is the sentinel.  */
    perc_table[entry].type = perc_entry_none;

    qsort (perc_table, sizeof (perc_table) / sizeof (perc_table[0]),
           sizeof (perc_table[0]), cmp_perc_entry);

  }
}

/* Called after all assembly has been done.  */

void
sparc_md_finish (void)
{
  unsigned long mach;
#ifndef TE_SOLARIS
  int hwcaps, hwcaps2;
#endif

  if (sparc_arch_size == 64)
    switch (current_architecture)
      {
      case SPARC_OPCODE_ARCH_V9A: mach = bfd_mach_sparc_v9a; break;
      case SPARC_OPCODE_ARCH_V9B: mach = bfd_mach_sparc_v9b; break;
      case SPARC_OPCODE_ARCH_V9C: mach = bfd_mach_sparc_v9c; break;
      case SPARC_OPCODE_ARCH_V9D: mach = bfd_mach_sparc_v9d; break;
      case SPARC_OPCODE_ARCH_V9E: mach = bfd_mach_sparc_v9e; break;
      case SPARC_OPCODE_ARCH_V9V: mach = bfd_mach_sparc_v9v; break;
      case SPARC_OPCODE_ARCH_V9M: mach = bfd_mach_sparc_v9m; break;
      case SPARC_OPCODE_ARCH_M8:  mach = bfd_mach_sparc_v9m8; break;
      default: mach = bfd_mach_sparc_v9; break;
      }
  else
    switch (current_architecture)
      {
      case SPARC_OPCODE_ARCH_SPARCLET: mach = bfd_mach_sparc_sparclet; break;
      case SPARC_OPCODE_ARCH_V9: mach = bfd_mach_sparc_v8plus; break;
      case SPARC_OPCODE_ARCH_V9A: mach = bfd_mach_sparc_v8plusa; break;
      case SPARC_OPCODE_ARCH_V9B: mach = bfd_mach_sparc_v8plusb; break;
      case SPARC_OPCODE_ARCH_V9C: mach = bfd_mach_sparc_v8plusc; break;
      case SPARC_OPCODE_ARCH_V9D: mach = bfd_mach_sparc_v8plusd; break;
      case SPARC_OPCODE_ARCH_V9E: mach = bfd_mach_sparc_v8pluse; break;
      case SPARC_OPCODE_ARCH_V9V: mach = bfd_mach_sparc_v8plusv; break;
      case SPARC_OPCODE_ARCH_V9M: mach = bfd_mach_sparc_v8plusm; break;
      case SPARC_OPCODE_ARCH_M8:  mach = bfd_mach_sparc_v8plusm8; break;
      /* The sparclite is treated like a normal sparc.  Perhaps it shouldn't
	 be but for now it is (since that's the way it's always been
	 treated).  */
      default: mach = bfd_mach_sparc; break;
      }
  bfd_set_arch_mach (stdoutput, bfd_arch_sparc, mach);

#ifndef TE_SOLARIS
  hwcaps = hwcap_seen & U0xffffffff;
  hwcaps2 = hwcap_seen >> 32;

  if (hwcaps)
    bfd_elf_add_obj_attr_int (stdoutput, OBJ_ATTR_GNU, Tag_GNU_Sparc_HWCAPS, hwcaps);
  if (hwcaps2)
    bfd_elf_add_obj_attr_int (stdoutput, OBJ_ATTR_GNU, Tag_GNU_Sparc_HWCAPS2, hwcaps2);
#endif
}

/* Return non-zero if VAL is in the range -(MAX+1) to MAX.  */

static inline int
in_signed_range (bfd_signed_vma val, bfd_signed_vma max)
{
  if (max <= 0)
    abort ();
  /* Sign-extend the value from the architecture word size, so that
     0xffffffff is always considered -1 on sparc32.  */
  if (sparc_arch_size == 32)
    {
      bfd_vma sign = (bfd_vma) 1 << 31;
      val = ((val & U0xffffffff) ^ sign) - sign;
    }
  if (val > max)
    return 0;
  if (val < ~max)
    return 0;
  return 1;
}

/* Return non-zero if VAL is in the range 0 to MAX.  */

static inline int
in_unsigned_range (bfd_vma val, bfd_vma max)
{
  if (val > max)
    return 0;
  return 1;
}

/* Return non-zero if VAL is in the range -(MAX/2+1) to MAX.
   (e.g. -15 to +31).  */

static inline int
in_bitfield_range (bfd_signed_vma val, bfd_signed_vma max)
{
  if (max <= 0)
    abort ();
  if (val > max)
    return 0;
  if (val < ~(max >> 1))
    return 0;
  return 1;
}

static int
sparc_ffs (unsigned int mask)
{
  int i;

  if (mask == 0)
    return -1;

  for (i = 0; (mask & 1) == 0; ++i)
    mask >>= 1;
  return i;
}

/* Implement big shift right.  */
static bfd_vma
BSR (bfd_vma val, int amount)
{
  if (sizeof (bfd_vma) <= 4 && amount >= 32)
    as_fatal (_("Support for 64-bit arithmetic not compiled in."));
  return val >> amount;
}

/* For communication between sparc_ip and get_expression.  */
static char *expr_parse_end;

/* Values for `special_case'.
   Instructions that require weird handling because they're longer than
   4 bytes.  */
#define SPECIAL_CASE_NONE	0
#define	SPECIAL_CASE_SET	1
#define SPECIAL_CASE_SETSW	2
#define SPECIAL_CASE_SETX	3
/* FIXME: sparc-opc.c doesn't have necessary "S" trigger to enable this.  */
#define	SPECIAL_CASE_FDIV	4

/* Bit masks of various insns.  */
#define NOP_INSN 0x01000000
#define OR_INSN 0x80100000
#define XOR_INSN 0x80180000
#define FMOVS_INSN 0x81A00020
#define SETHI_INSN 0x01000000
#define SLLX_INSN 0x81281000
#define SRA_INSN 0x81380000

/* The last instruction to be assembled.  */
static const struct sparc_opcode *last_insn;
/* The assembled opcode of `last_insn'.  */
static unsigned long last_opcode;

/* Handle the set and setuw synthetic instructions.  */

static void
synthetize_setuw (const struct sparc_opcode *insn)
{
  int need_hi22_p = 0;
  int rd = (the_insn.opcode & RD (~0)) >> 25;

  if (the_insn.exp.X_op == O_constant)
    {
      if (SPARC_OPCODE_ARCH_V9_P (max_architecture))
	{
	  if (sizeof (offsetT) > 4
	      && (the_insn.exp.X_add_number < 0
		  || the_insn.exp.X_add_number > (offsetT) U0xffffffff))
	    as_warn (_("set: number not in 0..4294967295 range"));
	}
      else
	{
	  if (sizeof (offsetT) > 4
	      && (the_insn.exp.X_add_number < -(offsetT) U0x80000000
		  || the_insn.exp.X_add_number > (offsetT) U0xffffffff))
	    as_warn (_("set: number not in -2147483648..4294967295 range"));
	  the_insn.exp.X_add_number = (int) the_insn.exp.X_add_number;
	}
    }

  /* See if operand is absolute and small; skip sethi if so.  */
  if (the_insn.exp.X_op != O_constant
      || the_insn.exp.X_add_number >= (1 << 12)
      || the_insn.exp.X_add_number < -(1 << 12))
    {
      the_insn.opcode = (SETHI_INSN | RD (rd)
			 | ((the_insn.exp.X_add_number >> 10)
			    & (the_insn.exp.X_op == O_constant
			       ? 0x3fffff : 0)));
      the_insn.reloc = (the_insn.exp.X_op != O_constant
			? BFD_RELOC_HI22 : BFD_RELOC_NONE);
      output_insn (insn, &the_insn);
      need_hi22_p = 1;
    }

  /* See if operand has no low-order bits; skip OR if so.  */
  if (the_insn.exp.X_op != O_constant
      || (need_hi22_p && (the_insn.exp.X_add_number & 0x3FF) != 0)
      || ! need_hi22_p)
    {
      the_insn.opcode = (OR_INSN | (need_hi22_p ? RS1 (rd) : 0)
			 | RD (rd) | IMMED
			 | (the_insn.exp.X_add_number
			    & (the_insn.exp.X_op != O_constant
			       ? 0 : need_hi22_p ? 0x3ff : 0x1fff)));
      the_insn.reloc = (the_insn.exp.X_op != O_constant
			? BFD_RELOC_LO10 : BFD_RELOC_NONE);
      output_insn (insn, &the_insn);
    }
}

/* Handle the setsw synthetic instruction.  */

static void
synthetize_setsw (const struct sparc_opcode *insn)
{
  int low32, rd, opc;

  rd = (the_insn.opcode & RD (~0)) >> 25;

  if (the_insn.exp.X_op != O_constant)
    {
      synthetize_setuw (insn);

      /* Need to sign extend it.  */
      the_insn.opcode = (SRA_INSN | RS1 (rd) | RD (rd));
      the_insn.reloc = BFD_RELOC_NONE;
      output_insn (insn, &the_insn);
      return;
    }

  if (sizeof (offsetT) > 4
      && (the_insn.exp.X_add_number < -(offsetT) U0x80000000
	  || the_insn.exp.X_add_number > (offsetT) U0xffffffff))
    as_warn (_("setsw: number not in -2147483648..4294967295 range"));

  low32 = the_insn.exp.X_add_number;

  if (low32 >= 0)
    {
      synthetize_setuw (insn);
      return;
    }

  opc = OR_INSN;

  the_insn.reloc = BFD_RELOC_NONE;
  /* See if operand is absolute and small; skip sethi if so.  */
  if (low32 < -(1 << 12))
    {
      the_insn.opcode = (SETHI_INSN | RD (rd)
			 | (((~the_insn.exp.X_add_number) >> 10) & 0x3fffff));
      output_insn (insn, &the_insn);
      low32 = 0x1c00 | (low32 & 0x3ff);
      opc = RS1 (rd) | XOR_INSN;
    }

  the_insn.opcode = (opc | RD (rd) | IMMED
		     | (low32 & 0x1fff));
  output_insn (insn, &the_insn);
}

/* Handle the setx synthetic instruction.  */

static void
synthetize_setx (const struct sparc_opcode *insn)
{
  int upper32, lower32;
  int tmpreg = (the_insn.opcode & RS1 (~0)) >> 14;
  int dstreg = (the_insn.opcode & RD (~0)) >> 25;
  int upper_dstreg;
  int need_hh22_p = 0, need_hm10_p = 0, need_hi22_p = 0, need_lo10_p = 0;
  int need_xor10_p = 0;

#define SIGNEXT32(x) ((((x) & U0xffffffff) ^ U0x80000000) - U0x80000000)
  lower32 = SIGNEXT32 (the_insn.exp.X_add_number);
  upper32 = SIGNEXT32 (BSR (the_insn.exp.X_add_number, 32));
#undef SIGNEXT32

  upper_dstreg = tmpreg;
  /* The tmp reg should not be the dst reg.  */
  if (tmpreg == dstreg)
    as_warn (_("setx: temporary register same as destination register"));

  /* ??? Obviously there are other optimizations we can do
     (e.g. sethi+shift for 0x1f0000000) and perhaps we shouldn't be
     doing some of these.  Later.  If you do change things, try to
     change all of this to be table driven as well.  */
  /* What to output depends on the number if it's constant.
     Compute that first, then output what we've decided upon.  */
  if (the_insn.exp.X_op != O_constant)
    {
      if (sparc_arch_size == 32)
	{
	  /* When arch size is 32, we want setx to be equivalent
	     to setuw for anything but constants.  */
	  the_insn.exp.X_add_number &= 0xffffffff;
	  synthetize_setuw (insn);
	  return;
	}
      need_hh22_p = need_hm10_p = need_hi22_p = need_lo10_p = 1;
      lower32 = 0;
      upper32 = 0;
    }
  else
    {
      /* Reset X_add_number, we've extracted it as upper32/lower32.
	 Otherwise fixup_segment will complain about not being able to
	 write an 8 byte number in a 4 byte field.  */
      the_insn.exp.X_add_number = 0;

      /* Only need hh22 if `or' insn can't handle constant.  */
      if (upper32 < -(1 << 12) || upper32 >= (1 << 12))
	need_hh22_p = 1;

      /* Does bottom part (after sethi) have bits?  */
      if ((need_hh22_p && (upper32 & 0x3ff) != 0)
	  /* No hh22, but does upper32 still have bits we can't set
	     from lower32?  */
	  || (! need_hh22_p && upper32 != 0 && upper32 != -1))
	need_hm10_p = 1;

      /* If the lower half is all zero, we build the upper half directly
	 into the dst reg.  */
      if (lower32 != 0
	  /* Need lower half if number is zero or 0xffffffff00000000.  */
	  || (! need_hh22_p && ! need_hm10_p))
	{
	  /* No need for sethi if `or' insn can handle constant.  */
	  if (lower32 < -(1 << 12) || lower32 >= (1 << 12)
	      /* Note that we can't use a negative constant in the `or'
		 insn unless the upper 32 bits are all ones.  */
	      || (lower32 < 0 && upper32 != -1)
	      || (lower32 >= 0 && upper32 == -1))
	    need_hi22_p = 1;

	  if (need_hi22_p && upper32 == -1)
	    need_xor10_p = 1;

	  /* Does bottom part (after sethi) have bits?  */
	  else if ((need_hi22_p && (lower32 & 0x3ff) != 0)
		   /* No sethi.  */
		   || (! need_hi22_p && (lower32 & 0x1fff) != 0)
		   /* Need `or' if we didn't set anything else.  */
		   || (! need_hi22_p && ! need_hh22_p && ! need_hm10_p))
	    need_lo10_p = 1;
	}
      else
	/* Output directly to dst reg if lower 32 bits are all zero.  */
	upper_dstreg = dstreg;
    }

  if (!upper_dstreg && dstreg)
    as_warn (_("setx: illegal temporary register g0"));

  if (need_hh22_p)
    {
      the_insn.opcode = (SETHI_INSN | RD (upper_dstreg)
			 | ((upper32 >> 10) & 0x3fffff));
      the_insn.reloc = (the_insn.exp.X_op != O_constant
			? BFD_RELOC_SPARC_HH22 : BFD_RELOC_NONE);
      output_insn (insn, &the_insn);
    }

  if (need_hi22_p)
    {
      the_insn.opcode = (SETHI_INSN | RD (dstreg)
			 | (((need_xor10_p ? ~lower32 : lower32)
			     >> 10) & 0x3fffff));
      the_insn.reloc = (the_insn.exp.X_op != O_constant
			? BFD_RELOC_SPARC_LM22 : BFD_RELOC_NONE);
      output_insn (insn, &the_insn);
    }

  if (need_hm10_p)
    {
      the_insn.opcode = (OR_INSN
			 | (need_hh22_p ? RS1 (upper_dstreg) : 0)
			 | RD (upper_dstreg)
			 | IMMED
			 | (upper32 & (need_hh22_p ? 0x3ff : 0x1fff)));
      the_insn.reloc = (the_insn.exp.X_op != O_constant
			? BFD_RELOC_SPARC_HM10 : BFD_RELOC_NONE);
      output_insn (insn, &the_insn);
    }

  if (need_lo10_p)
    {
      /* FIXME: One nice optimization to do here is to OR the low part
	 with the highpart if hi22 isn't needed and the low part is
	 positive.  */
      the_insn.opcode = (OR_INSN | (need_hi22_p ? RS1 (dstreg) : 0)
			 | RD (dstreg)
			 | IMMED
			 | (lower32 & (need_hi22_p ? 0x3ff : 0x1fff)));
      the_insn.reloc = (the_insn.exp.X_op != O_constant
			? BFD_RELOC_LO10 : BFD_RELOC_NONE);
      output_insn (insn, &the_insn);
    }

  /* If we needed to build the upper part, shift it into place.  */
  if (need_hh22_p || need_hm10_p)
    {
      the_insn.opcode = (SLLX_INSN | RS1 (upper_dstreg) | RD (upper_dstreg)
			 | IMMED | 32);
      the_insn.reloc = BFD_RELOC_NONE;
      output_insn (insn, &the_insn);
    }

  /* To get -1 in upper32, we do sethi %hi(~x), r; xor r, -0x400 | x, r.  */
  if (need_xor10_p)
    {
      the_insn.opcode = (XOR_INSN | RS1 (dstreg) | RD (dstreg) | IMMED
			 | 0x1c00 | (lower32 & 0x3ff));
      the_insn.reloc = BFD_RELOC_NONE;
      output_insn (insn, &the_insn);
    }

  /* If we needed to build both upper and lower parts, OR them together.  */
  else if ((need_hh22_p || need_hm10_p) && (need_hi22_p || need_lo10_p))
    {
      the_insn.opcode = (OR_INSN | RS1 (dstreg) | RS2 (upper_dstreg)
			 | RD (dstreg));
      the_insn.reloc = BFD_RELOC_NONE;
      output_insn (insn, &the_insn);
    }
}

/* Main entry point to assemble one instruction.  */

void
md_assemble (char *str)
{
  const struct sparc_opcode *insn;
  int special_case;

  know (str);
  special_case = sparc_ip (str, &insn);
  if (insn == NULL)
    return;

  /* Certain instructions may not appear on delay slots.  Check for
     these situations.  */
  if (last_insn != NULL
      && (last_insn->flags & F_DELAYED) != 0)
    {
      /* Before SPARC V9 the effect of having a delayed branch
         instruction in the delay slot of a conditional delayed branch
         was undefined.

         In SPARC V9 DCTI couples are well defined.

         However, starting with the UltraSPARC Architecture 2005, DCTI
         couples (of all kind) are deprecated and should not be used,
         as they may be slow or behave differently to what the
         programmer expects.  */
      if (dcti_couples_detect
          && (insn->flags & F_DELAYED) != 0
          && ((max_architecture < SPARC_OPCODE_ARCH_V9
               && (last_insn->flags & F_CONDBR) != 0)
              || max_architecture >= SPARC_OPCODE_ARCH_V9C))
        as_warn (_("unpredictable DCTI couple"));


      /* We warn about attempts to put a floating point branch in a
         delay slot, unless the delay slot has been annulled.  */
      if ((insn->flags & F_FBR) != 0
          /* ??? This test isn't completely accurate.  We assume anything with
             F_{UNBR,CONDBR,FBR} set is annullable.  */
          && ((last_insn->flags & (F_UNBR | F_CONDBR | F_FBR)) == 0
              || (last_opcode & ANNUL) == 0))
        as_warn (_("FP branch in delay slot"));
    }

  /* SPARC before v9 does not allow a floating point compare
     directly before a floating point branch.  Insert a nop
     instruction if needed, with a warning.  */
  if (max_architecture < SPARC_OPCODE_ARCH_V9
      && last_insn != NULL
      && (insn->flags & F_FBR) != 0
      && (last_insn->flags & F_FLOAT) != 0
      && (last_insn->match & OP3 (0x35)) == OP3 (0x35))
    {
      struct sparc_it nop_insn;

      nop_insn.opcode = NOP_INSN;
      nop_insn.reloc = BFD_RELOC_NONE;
      output_insn (insn, &nop_insn);
      as_warn (_("FP branch preceded by FP compare; NOP inserted"));
    }

  switch (special_case)
    {
    case SPECIAL_CASE_NONE:
      /* Normal insn.  */
      output_insn (insn, &the_insn);
      break;

    case SPECIAL_CASE_SETSW:
      synthetize_setsw (insn);
      break;

    case SPECIAL_CASE_SET:
      synthetize_setuw (insn);
      break;

    case SPECIAL_CASE_SETX:
      synthetize_setx (insn);
      break;

    case SPECIAL_CASE_FDIV:
      {
	int rd = (the_insn.opcode >> 25) & 0x1f;

	output_insn (insn, &the_insn);

	/* According to information leaked from Sun, the "fdiv" instructions
	   on early SPARC machines would produce incorrect results sometimes.
	   The workaround is to add an fmovs of the destination register to
	   itself just after the instruction.  This was true on machines
	   with Weitek 1165 float chips, such as the Sun-4/260 and /280.  */
	gas_assert (the_insn.reloc == BFD_RELOC_NONE);
	the_insn.opcode = FMOVS_INSN | rd | RD (rd);
	output_insn (insn, &the_insn);
	return;
      }

    default:
      as_fatal (_("failed special case insn sanity check"));
    }
}

static const char *
get_hwcap_name (uint64_t mask)
{
  if (mask & HWCAP_MUL32)
    return "mul32";
  if (mask & HWCAP_DIV32)
    return "div32";
  if (mask & HWCAP_FSMULD)
    return "fsmuld";
  if (mask & HWCAP_V8PLUS)
    return "v8plus";
  if (mask & HWCAP_POPC)
    return "popc";
  if (mask & HWCAP_VIS)
    return "vis";
  if (mask & HWCAP_VIS2)
    return "vis2";
  if (mask & HWCAP_ASI_BLK_INIT)
    return "ASIBlkInit";
  if (mask & HWCAP_FMAF)
    return "fmaf";
  if (mask & HWCAP_VIS3)
    return "vis3";
  if (mask & HWCAP_HPC)
    return "hpc";
  if (mask & HWCAP_RANDOM)
    return "random";
  if (mask & HWCAP_TRANS)
    return "trans";
  if (mask & HWCAP_FJFMAU)
    return "fjfmau";
  if (mask & HWCAP_IMA)
    return "ima";
  if (mask & HWCAP_ASI_CACHE_SPARING)
    return "cspare";
  if (mask & HWCAP_AES)
    return "aes";
  if (mask & HWCAP_DES)
    return "des";
  if (mask & HWCAP_KASUMI)
    return "kasumi";
  if (mask & HWCAP_CAMELLIA)
    return "camellia";
  if (mask & HWCAP_MD5)
    return "md5";
  if (mask & HWCAP_SHA1)
    return "sha1";
  if (mask & HWCAP_SHA256)
    return "sha256";
  if (mask & HWCAP_SHA512)
    return "sha512";
  if (mask & HWCAP_MPMUL)
    return "mpmul";
  if (mask & HWCAP_MONT)
    return "mont";
  if (mask & HWCAP_PAUSE)
    return "pause";
  if (mask & HWCAP_CBCOND)
    return "cbcond";
  if (mask & HWCAP_CRC32C)
    return "crc32c";

  mask = mask >> 32;
  if (mask & HWCAP2_FJATHPLUS)
    return "fjathplus";
  if (mask & HWCAP2_VIS3B)
    return "vis3b";
  if (mask & HWCAP2_ADP)
    return "adp";
  if (mask & HWCAP2_SPARC5)
    return "sparc5";
  if (mask & HWCAP2_MWAIT)
    return "mwait";
  if (mask & HWCAP2_XMPMUL)
    return "xmpmul";
  if (mask & HWCAP2_XMONT)
    return "xmont";
  if (mask & HWCAP2_NSEC)
    return "nsec";
  if (mask & HWCAP2_SPARC6)
    return "sparc6";
  if (mask & HWCAP2_ONADDSUB)
    return "onaddsub";
  if (mask & HWCAP2_ONMUL)
    return "onmul";
  if (mask & HWCAP2_ONDIV)
    return "ondiv";
  if (mask & HWCAP2_DICTUNP)
    return "dictunp";
  if (mask & HWCAP2_FPCMPSHL)
    return "fpcmpshl";
  if (mask & HWCAP2_RLE)
    return "rle";
  if (mask & HWCAP2_SHA3)
    return "sha3";

  return "UNKNOWN";
}

/* Subroutine of md_assemble to do the actual parsing.  */

static int
sparc_ip (char *str, const struct sparc_opcode **pinsn)
{
  const char *error_message = "";
  char *s;
  const char *args;
  char c;
  const struct sparc_opcode *insn;
  char *argsStart;
  unsigned long opcode;
  unsigned int mask = 0;
  int match = 0;
  int comma = 0;
  int v9_arg_p;
  int special_case = SPECIAL_CASE_NONE;
  const sparc_asi *sasi = NULL;

  s = str;
  if (ISLOWER (*s))
    {
      do
	++s;
      while (ISLOWER (*s) || ISDIGIT (*s) || *s == '_');
    }

  switch (*s)
    {
    case '\0':
      break;

    case ',':
      comma = 1;
      /* Fall through.  */

    case ' ':
      *s++ = '\0';
      break;

    default:
      as_bad (_("Unknown opcode: `%s'"), str);
      *pinsn = NULL;
      return special_case;
    }
  insn = (struct sparc_opcode *) str_hash_find (op_hash, str);
  *pinsn = insn;
  if (insn == NULL)
    {
      as_bad (_("Unknown opcode: `%s'"), str);
      return special_case;
    }
  if (comma)
    {
      *--s = ',';
    }

  argsStart = s;
  for (;;)
    {
      opcode = insn->match;
      memset (&the_insn, '\0', sizeof (the_insn));
      the_insn.reloc = BFD_RELOC_NONE;
      v9_arg_p = 0;

      /* Build the opcode, checking as we go to make sure that the
         operands match.  */
      for (args = insn->args;; ++args)
	{
	  switch (*args)
	    {
	    case 'K':
	      {
		int kmask = 0;

		/* Parse a series of masks.  */
		if (*s == '#')
		  {
		    while (*s == '#')
		      {
			int jmask;

			if (! parse_keyword_arg (sparc_encode_membar, &s,
						 &jmask))
			  {
			    error_message = _(": invalid membar mask name");
			    goto error;
			  }
			kmask |= jmask;
			while (*s == ' ')
			  ++s;
			if (*s == '|' || *s == '+')
			  ++s;
			while (*s == ' ')
			  ++s;
		      }
		  }
		else
		  {
		    if (! parse_const_expr_arg (&s, &kmask))
		      {
			error_message = _(": invalid membar mask expression");
			goto error;
		      }
		    if (kmask < 0 || kmask > 127)
		      {
			error_message = _(": invalid membar mask number");
			goto error;
		      }
		  }

		opcode |= MEMBAR (kmask);
		continue;
	      }

	    case '3':
	      {
		int smask = 0;

		if (! parse_const_expr_arg (&s, &smask))
		  {
		    error_message = _(": invalid siam mode expression");
		    goto error;
		  }
		if (smask < 0 || smask > 7)
		  {
		    error_message = _(": invalid siam mode number");
		    goto error;
		  }
		opcode |= smask;
		continue;
	      }

	    case '*':
	      {
		int fcn = 0;

		/* Parse a prefetch function.  */
		if (*s == '#')
		  {
		    if (! parse_keyword_arg (sparc_encode_prefetch, &s, &fcn))
		      {
			error_message = _(": invalid prefetch function name");
			goto error;
		      }
		  }
		else
		  {
		    if (! parse_const_expr_arg (&s, &fcn))
		      {
			error_message = _(": invalid prefetch function expression");
			goto error;
		      }
		    if (fcn < 0 || fcn > 31)
		      {
			error_message = _(": invalid prefetch function number");
			goto error;
		      }
		  }
		opcode |= RD (fcn);
		continue;
	      }

	    case '!':
	    case '?':
	      /* Parse a sparc64 privileged register.  */
	      if (*s == '%')
		{
		  struct priv_reg_entry *p;
		  unsigned int len = 9999999; /* Init to make gcc happy.  */

		  s += 1;
                  for (p = priv_reg_table; p->name; p++)
                    if (p->name[0] == s[0])
                      {
                        len = strlen (p->name);
                        if (strncmp (p->name, s, len) == 0)
                          break;
                      }

		  if (!p->name)
		    {
		      error_message = _(": unrecognizable privileged register");
		      goto error;
		    }
                  
                  if (((opcode >> (*args == '?' ? 14 : 25)) & 0x1f) != (unsigned) p->regnum)
                    {
                      error_message = _(": unrecognizable privileged register");
                      goto error;
                    }

		  s += len;
		  continue;
		}
	      else
		{
		  error_message = _(": unrecognizable privileged register");
		  goto error;
		}

	    case '$':
	    case '%':
	      /* Parse a sparc64 hyperprivileged register.  */
	      if (*s == '%')
		{
		  struct priv_reg_entry *p;
		  unsigned int len = 9999999; /* Init to make gcc happy.  */

		  s += 1;
                  for (p = hpriv_reg_table; p->name; p++)
                    if (p->name[0] == s[0])
                      {
                        len = strlen (p->name);
                        if (strncmp (p->name, s, len) == 0)
                          break;
                      }

		  if (!p->name)
		    {
		      error_message = _(": unrecognizable hyperprivileged register");
		      goto error;
		    }

                  if (((opcode >> (*args == '$' ? 14 : 25)) & 0x1f) != (unsigned) p->regnum)
                    {
                      error_message = _(": unrecognizable hyperprivileged register");
                      goto error;
                    }

                  s += len;
		  continue;
		}
	      else
		{
		  error_message = _(": unrecognizable hyperprivileged register");
		  goto error;
		}

	    case '_':
	    case '/':
	      /* Parse a v9a or later ancillary state register.  */
	      if (*s == '%')
		{
		  struct priv_reg_entry *p;
		  unsigned int len = 9999999; /* Init to make gcc happy.  */

		  s += 1;
                  for (p = v9a_asr_table; p->name; p++)
                    if (p->name[0] == s[0])
                      {
                        len = strlen (p->name);
                        if (strncmp (p->name, s, len) == 0)
                          break;
                      }

		  if (!p->name)
		    {
		      error_message = _(": unrecognizable ancillary state register");
		      goto error;
		    }

                  if (((opcode >> (*args == '/' ? 14 : 25)) & 0x1f) != (unsigned) p->regnum)
                     {
                       error_message = _(": unrecognizable ancillary state register");
                       goto error;
                     }

		  s += len;
		  continue;
		}
	      else
		{
		  error_message = _(": unrecognizable ancillary state register");
		  goto error;
		}

	    case 'M':
	    case 'm':
	      if (startswith (s, "%asr"))
		{
		  s += 4;

		  if (ISDIGIT (*s))
		    {
		      long num = 0;

		      while (ISDIGIT (*s))
			{
			  num = num * 10 + *s - '0';
			  ++s;
			}

                      /* We used to check here for the asr number to
                         be between 16 and 31 in V9 and later, as
                         mandated by the section C.1.1 "Register
                         Names" in the SPARC spec.  However, we
                         decided to remove this restriction as a) it
                         introduces problems when new V9 asr registers
                         are introduced, b) the Solaris assembler
                         doesn't implement this restriction and c) the
                         restriction will go away in future revisions
                         of the Oracle SPARC Architecture.  */

                      if (num < 0 || 31 < num)
                        {
                          error_message = _(": asr number must be between 0 and 31");
                          goto error;
                        }

		      opcode |= (*args == 'M' ? RS1 (num) : RD (num));
		      continue;
		    }
		  else
		    {
		      error_message = _(": expecting %asrN");
		      goto error;
		    }
		} /* if %asr  */
	      break;

	    case 'I':
	      the_insn.reloc = BFD_RELOC_SPARC_11;
	      goto immediate;

	    case 'j':
	      the_insn.reloc = BFD_RELOC_SPARC_10;
	      goto immediate;

	    case ')':
	      if (*s == ' ')
		s++;
	      if ((s[0] == '0' && s[1] == 'x' && ISXDIGIT (s[2]))
		  || ISDIGIT (*s))
		{
		  long num = 0;

		  if (s[0] == '0' && s[1] == 'x')
		    {
		      s += 2;
		      while (ISXDIGIT (*s))
			{
			  num <<= 4;
			  num |= hex_value (*s);
			  ++s;
			}
		    }
		  else
		    {
		      while (ISDIGIT (*s))
			{
			  num = num * 10 + *s - '0';
			  ++s;
			}
		    }
		  if (num < 0 || num > 31)
		    {
		      error_message = _(": crypto immediate must be between 0 and 31");
		      goto error;
		    }

		  opcode |= RS3 (num);
		  continue;
		}
	      else
		{
		  error_message = _(": expecting crypto immediate");
		  goto error;
		}

	    case 'X':
	      /* V8 systems don't understand BFD_RELOC_SPARC_5.  */
	      if (SPARC_OPCODE_ARCH_V9_P (max_architecture))
		the_insn.reloc = BFD_RELOC_SPARC_5;
	      else
		the_insn.reloc = BFD_RELOC_SPARC13;
	      /* These fields are unsigned, but for upward compatibility,
		 allow negative values as well.  */
	      goto immediate;

	    case 'Y':
	      /* V8 systems don't understand BFD_RELOC_SPARC_6.  */
	      if (SPARC_OPCODE_ARCH_V9_P (max_architecture))
		the_insn.reloc = BFD_RELOC_SPARC_6;
	      else
		the_insn.reloc = BFD_RELOC_SPARC13;
	      /* These fields are unsigned, but for upward compatibility,
		 allow negative values as well.  */
	      goto immediate;

	    case 'k':
	      the_insn.reloc = /* RELOC_WDISP2_14 */ BFD_RELOC_SPARC_WDISP16;
	      the_insn.pcrel = 1;
	      goto immediate;

	    case '=':
	      the_insn.reloc = /* RELOC_WDISP2_8 */ BFD_RELOC_SPARC_WDISP10;
	      the_insn.pcrel = 1;
	      goto immediate;

	    case 'G':
	      the_insn.reloc = BFD_RELOC_SPARC_WDISP19;
	      the_insn.pcrel = 1;
	      goto immediate;

	    case 'N':
	      if (*s == 'p' && s[1] == 'n')
		{
		  s += 2;
		  continue;
		}
	      break;

	    case 'T':
	      if (*s == 'p' && s[1] == 't')
		{
		  s += 2;
		  continue;
		}
	      break;

	    case 'z':
	      if (*s == ' ')
		{
		  ++s;
		}
	      if ((startswith (s, "%icc"))
                  || (sparc_arch_size == 32 && startswith (s, "%ncc")))
		{
		  s += 4;
		  continue;
		}
	      break;

	    case 'Z':
	      if (*s == ' ')
		{
		  ++s;
		}
              if ((startswith (s, "%xcc"))
                  || (sparc_arch_size == 64 && startswith (s, "%ncc")))
		{
		  s += 4;
		  continue;
		}
	      break;

	    case '6':
	      if (*s == ' ')
		{
		  ++s;
		}
	      if (startswith (s, "%fcc0"))
		{
		  s += 5;
		  continue;
		}
	      break;

	    case '7':
	      if (*s == ' ')
		{
		  ++s;
		}
	      if (startswith (s, "%fcc1"))
		{
		  s += 5;
		  continue;
		}
	      break;

	    case '8':
	      if (*s == ' ')
		{
		  ++s;
		}
	      if (startswith (s, "%fcc2"))
		{
		  s += 5;
		  continue;
		}
	      break;

	    case '9':
	      if (*s == ' ')
		{
		  ++s;
		}
	      if (startswith (s, "%fcc3"))
		{
		  s += 5;
		  continue;
		}
	      break;

	    case 'P':
	      if (startswith (s, "%pc"))
		{
		  s += 3;
		  continue;
		}
	      break;

	    case 'W':
	      if (startswith (s, "%tick"))
		{
		  s += 5;
		  continue;
		}
	      break;

	    case '\0':		/* End of args.  */
	      if (s[0] == ',' && s[1] == '%')
		{
		  char *s1;
		  int npar = 0;
                  const struct perc_entry *p;

                  for (p = perc_table; p->type != perc_entry_none; p++)
                    if ((p->type == perc_entry_post_pop || p->type == perc_entry_reg)
                        && strncmp (s + 2, p->name, p->len) == 0)
                      break;
                  if (p->type == perc_entry_none || p->type == perc_entry_reg)
                    break;

		  if (s[p->len + 2] != '(')
		    {
		      as_bad (_("Illegal operands: %%%s requires arguments in ()"), p->name);
		      return special_case;
		    }

		  if (! (p->pop->flags & F_POP_TLS_CALL)
                      && the_insn.reloc != BFD_RELOC_NONE)
		    {
		      as_bad (_("Illegal operands: %%%s cannot be used together with other relocs in the insn ()"),
			      p->name);
		      return special_case;
		    }

		  if ((p->pop->flags & F_POP_TLS_CALL)
		      && (the_insn.reloc != BFD_RELOC_32_PCREL_S2
			  || the_insn.exp.X_add_number != 0
			  || the_insn.exp.X_add_symbol
			     != symbol_find_or_make ("__tls_get_addr")))
		    {
		      as_bad (_("Illegal operands: %%%s can be only used with call __tls_get_addr"),
			      p->name);
		      return special_case;
		    }

		  the_insn.reloc = p->pop->reloc;
		  memset (&the_insn.exp, 0, sizeof (the_insn.exp));
		  s += p->len + 3;

		  for (s1 = s; *s1 && *s1 != ',' && *s1 != ']'; s1++)
		    if (*s1 == '(')
		      npar++;
		    else if (*s1 == ')')
		      {
			if (!npar)
			  break;
			npar--;
		      }

		  if (*s1 != ')')
		    {
		      as_bad (_("Illegal operands: %%%s requires arguments in ()"), p->name);
		      return special_case;
		    }

		  *s1 = '\0';
		  (void) get_expression (s);
		  *s1 = ')';
		  s = s1 + 1;
		}
	      if (*s == '\0')
		match = 1;
	      break;

	    case '+':
	      if (*s == '+')
		{
		  ++s;
		  continue;
		}
	      if (*s == '-')
		{
		  continue;
		}
	      break;

	    case '[':		/* These must match exactly.  */
	    case ']':
	    case ',':
	    case ' ':
	      if (*s++ == *args)
		continue;
	      break;

	    case '#':		/* Must be at least one digit.  */
	      if (ISDIGIT (*s++))
		{
		  while (ISDIGIT (*s))
		    {
		      ++s;
		    }
		  continue;
		}
	      break;

	    case 'C':		/* Coprocessor state register.  */
	      if (startswith (s, "%csr"))
		{
		  s += 4;
		  continue;
		}
	      break;

	    case 'b':		/* Next operand is a coprocessor register.  */
	    case 'c':
	    case 'D':
	      if (*s++ == '%' && *s++ == 'c' && ISDIGIT (*s))
		{
		  mask = *s++;
		  if (ISDIGIT (*s))
		    {
		      mask = 10 * (mask - '0') + (*s++ - '0');
		      if (mask >= 32)
			{
			  break;
			}
		    }
		  else
		    {
		      mask -= '0';
		    }
		  switch (*args)
		    {

		    case 'b':
		      opcode |= mask << 14;
		      continue;

		    case 'c':
		      opcode |= mask;
		      continue;

		    case 'D':
		      opcode |= mask << 25;
		      continue;
		    }
		}
	      break;

	    case 'r':		/* next operand must be a register */
	    case 'O':
	    case '1':
	    case '2':
	    case 'd':
	      if (*s++ == '%')
		{
		  switch (c = *s++)
		    {

		    case 'f':	/* frame pointer */
		      if (*s++ == 'p')
			{
			  mask = 0x1e;
			  break;
			}
		      goto error;

		    case 'g':	/* global register */
		      c = *s++;
		      if (isoctal (c))
			{
			  mask = c - '0';
			  break;
			}
		      goto error;

		    case 'i':	/* in register */
		      c = *s++;
		      if (isoctal (c))
			{
			  mask = c - '0' + 24;
			  break;
			}
		      goto error;

		    case 'l':	/* local register */
		      c = *s++;
		      if (isoctal (c))
			{
			  mask = (c - '0' + 16);
			  break;
			}
		      goto error;

		    case 'o':	/* out register */
		      c = *s++;
		      if (isoctal (c))
			{
			  mask = (c - '0' + 8);
			  break;
			}
		      goto error;

		    case 's':	/* stack pointer */
		      if (*s++ == 'p')
			{
			  mask = 0xe;
			  break;
			}
		      goto error;

		    case 'r':	/* any register */
		      if (!ISDIGIT ((c = *s++)))
			{
			  goto error;
			}
		      /* FALLTHROUGH */
		    case '0':
		    case '1':
		    case '2':
		    case '3':
		    case '4':
		    case '5':
		    case '6':
		    case '7':
		    case '8':
		    case '9':
		      if (ISDIGIT (*s))
			{
			  if ((c = 10 * (c - '0') + (*s++ - '0')) >= 32)
			    {
			      goto error;
			    }
			}
		      else
			{
			  c -= '0';
			}
		      mask = c;
		      break;

		    default:
		      goto error;
		    }

		  if ((mask & ~1) == 2 && sparc_arch_size == 64
		      && no_undeclared_regs && ! globals[mask])
		    as_bad (_("detected global register use not covered by .register pseudo-op"));

		  /* Got the register, now figure out where
		     it goes in the opcode.  */
		  switch (*args)
		    {
		    case '1':
		      opcode |= mask << 14;
		      continue;

		    case '2':
		      opcode |= mask;
		      continue;

		    case 'd':
		      opcode |= mask << 25;
		      continue;

		    case 'r':
		      opcode |= (mask << 25) | (mask << 14);
		      continue;

		    case 'O':
		      opcode |= (mask << 25) | (mask << 0);
		      continue;
		    }
		}
	      break;

	    case 'e':		/* next operand is a floating point register */
	    case 'v':
	    case 'V':
            case ';':

	    case 'f':
	    case 'B':
	    case 'R':
            case ':':
            case '\'':

	    case '4':
	    case '5':

	    case 'g':
	    case 'H':
	    case 'J':
	    case '}':
            case '^':
	      {
		char format;

		if (*s++ == '%'
		    && ((format = *s) == 'f'
                        || format == 'd'
                        || format == 'q')
		    && ISDIGIT (*++s))
		  {
		    for (mask = 0; ISDIGIT (*s); ++s)
		      {
			mask = 10 * mask + (*s - '0');
		      }		/* read the number */

		    if ((*args == 'v'
			 || *args == 'B'
			 || *args == '5'
			 || *args == 'H'
                         || *args == '\''
			 || format == 'd')
			&& (mask & 1))
		      {
                        /* register must be even numbered */
			break;
		      }

		    if ((*args == 'V'
			 || *args == 'R'
			 || *args == 'J'
			 || format == 'q')
			&& (mask & 3))
		      {
                        /* register must be multiple of 4 */
			break;
		      }

                    if ((*args == ':'
                         || *args == ';'
                         || *args == '^')
                        && (mask & 7))
                      {
                        /* register must be multiple of 8 */
                        break;
                      }

                    if (*args == '\'' && mask < 48)
                      {
                        /* register must be higher or equal than %f48 */
                        break;
                      }

		    if (mask >= 64)
		      {
			if (SPARC_OPCODE_ARCH_V9_P (max_architecture))
			  error_message = _(": There are only 64 f registers; [0-63]");
			else
			  error_message = _(": There are only 32 f registers; [0-31]");
			goto error;
		      }	/* on error */
		    else if (mask >= 32)
		      {
			if (SPARC_OPCODE_ARCH_V9_P (max_architecture))
			  {
			    if (*args == 'e' || *args == 'f' || *args == 'g')
			      {
				error_message
				  = _(": There are only 32 single precision f registers; [0-31]");
				goto error;
			      }
			    v9_arg_p = 1;
			    mask -= 31;	/* wrap high bit */
			  }
			else
			  {
			    error_message = _(": There are only 32 f registers; [0-31]");
			    goto error;
			  }
		      }
		  }
		else
		  {
		    break;
		  }	/* if not an 'f' register.  */

		if (*args == '}' && mask != RS2 (opcode))
		  {
		    error_message
		      = _(": Instruction requires frs2 and frsd must be the same register");
		    goto error;
		  }

		switch (*args)
		  {
		  case 'v':
		  case 'V':
		  case 'e':
                  case ';':
		    opcode |= RS1 (mask);
		    continue;

		  case 'f':
		  case 'B':
		  case 'R':
                  case ':':
		    opcode |= RS2 (mask);
		    continue;

                  case '\'':
                    opcode |= RS2 (mask & 0xe);
                    continue;
                    
		  case '4':
		  case '5':
		    opcode |= RS3 (mask);
		    continue;

		  case 'g':
		  case 'H':
		  case 'J':
		  case '}':
                  case '^':
		    opcode |= RD (mask);
		    continue;
		  }		/* Pack it in.  */

		know (0);
		break;
	      }			/* float arg  */

	    case 'F':
	      if (startswith (s, "%fsr"))
		{
		  s += 4;
		  continue;
		}
	      break;

	    case '(':
	      if (startswith (s, "%efsr"))
		{
		  s += 5;
		  continue;
		}
	      break;

	    case '0':		/* 64 bit immediate (set, setsw, setx insn)  */
	      the_insn.reloc = BFD_RELOC_NONE; /* reloc handled elsewhere  */
	      goto immediate;

	    case 'l':		/* 22 bit PC relative immediate  */
	      the_insn.reloc = BFD_RELOC_SPARC_WDISP22;
	      the_insn.pcrel = 1;
	      goto immediate;

	    case 'L':		/* 30 bit immediate  */
	      the_insn.reloc = BFD_RELOC_32_PCREL_S2;
	      the_insn.pcrel = 1;
	      goto immediate;

	    case 'h':
	    case 'n':		/* 22 bit immediate  */
	      the_insn.reloc = BFD_RELOC_SPARC22;
	      goto immediate;

	    case 'i':		/* 13 bit immediate  */
	      the_insn.reloc = BFD_RELOC_SPARC13;

	      /* fallthrough */

	    immediate:
	      if (*s == ' ')
		s++;

	      {
		char *s1;
		const char *op_arg = NULL;
		static expressionS op_exp;
		bfd_reloc_code_real_type old_reloc = the_insn.reloc;

		/* Check for %hi, etc.  */
		if (*s == '%')
		  {
                    const struct perc_entry *p;
                    
                    for (p = perc_table; p->type != perc_entry_none; p++)
                      if ((p->type == perc_entry_imm_pop || p->type == perc_entry_reg)
                          && strncmp (s + 1, p->name, p->len) == 0)
                        break;
                    if (p->type == perc_entry_none || p->type == perc_entry_reg)
                      break;

		    if (s[p->len + 1] != '(')
		      {
			as_bad (_("Illegal operands: %%%s requires arguments in ()"), p->name);
			return special_case;
		      }

		    op_arg = p->name;
		    the_insn.reloc = p->pop->reloc;
		    s += p->len + 2;
		    v9_arg_p = p->pop->flags & F_POP_V9;
		  }

		/* Note that if the get_expression() fails, we will still
		   have created U entries in the symbol table for the
		   'symbols' in the input string.  Try not to create U
		   symbols for registers, etc.  */

		/* This stuff checks to see if the expression ends in
		   +%reg.  If it does, it removes the register from
		   the expression, and re-sets 's' to point to the
		   right place.  */

		if (op_arg)
		  {
		    int npar = 0;

		    for (s1 = s; *s1 && *s1 != ',' && *s1 != ']'; s1++)
		      if (*s1 == '(')
			npar++;
		      else if (*s1 == ')')
			{
			  if (!npar)
			    break;
			  npar--;
			}

		    if (*s1 != ')')
		      {
			as_bad (_("Illegal operands: %%%s requires arguments in ()"), op_arg);
			return special_case;
		      }

		    *s1 = '\0';
		    (void) get_expression (s);
		    *s1 = ')';
		    if (expr_parse_end != s1)
		      {
			as_bad (_("Expression inside %%%s could not be parsed"), op_arg);
			return special_case;
		      }
		    s = s1 + 1;
		    if (*s == ',' || *s == ']' || !*s)
		      continue;
		    if (*s != '+' && *s != '-')
		      {
			as_bad (_("Illegal operands: Can't do arithmetics other than + and - involving %%%s()"), op_arg);
			return special_case;
		      }
		    *s1 = '0';
		    s = s1;
		    op_exp = the_insn.exp;
		    memset (&the_insn.exp, 0, sizeof (the_insn.exp));
		  }

		for (s1 = s; *s1 && *s1 != ',' && *s1 != ']'; s1++)
		  ;

		if (s1 != s && ISDIGIT (s1[-1]))
		  {
		    if (s1[-2] == '%' && s1[-3] == '+')
		      s1 -= 3;
		    else if (strchr ("golir0123456789", s1[-2]) && s1[-3] == '%' && s1[-4] == '+')
		      s1 -= 4;
		    else if (s1[-3] == 'r' && s1[-4] == '%' && s1[-5] == '+')
		      s1 -= 5;
		    else
		      s1 = NULL;
		    if (s1)
		      {
			*s1 = '\0';
			if (op_arg && s1 == s + 1)
			  the_insn.exp.X_op = O_absent;
			else
			  (void) get_expression (s);
			*s1 = '+';
			if (op_arg)
			  *s = ')';
			s = s1;
		      }
		  }
		else
		  s1 = NULL;

		if (!s1)
		  {
		    (void) get_expression (s);
		    if (op_arg)
		      *s = ')';
		    s = expr_parse_end;
		  }

		if (op_arg)
		  {
		    the_insn.exp2 = the_insn.exp;
		    the_insn.exp = op_exp;
		    if (the_insn.exp2.X_op == O_absent)
		      the_insn.exp2.X_op = O_illegal;
		    else if (the_insn.exp.X_op == O_absent)
		      {
			the_insn.exp = the_insn.exp2;
			the_insn.exp2.X_op = O_illegal;
		      }
		    else if (the_insn.exp.X_op == O_constant)
		      {
			valueT val = the_insn.exp.X_add_number;
			switch (the_insn.reloc)
			  {
			  default:
			    break;

			  case BFD_RELOC_SPARC_HH22:
			    val = BSR (val, 32);
			    /* Fall through.  */

			  case BFD_RELOC_SPARC_LM22:
			  case BFD_RELOC_HI22:
			    val = (val >> 10) & 0x3fffff;
			    break;

			  case BFD_RELOC_SPARC_HM10:
			    val = BSR (val, 32);
			    /* Fall through.  */

			  case BFD_RELOC_LO10:
			    val &= 0x3ff;
			    break;

			  case BFD_RELOC_SPARC_H34:
			    val >>= 12;
			    val &= 0x3fffff;
			    break;

			  case BFD_RELOC_SPARC_H44:
			    val >>= 22;
			    val &= 0x3fffff;
			    break;

			  case BFD_RELOC_SPARC_M44:
			    val >>= 12;
			    val &= 0x3ff;
			    break;

			  case BFD_RELOC_SPARC_L44:
			    val &= 0xfff;
			    break;

			  case BFD_RELOC_SPARC_HIX22:
			    val = ~val;
			    val = (val >> 10) & 0x3fffff;
			    break;

			  case BFD_RELOC_SPARC_LOX10:
			    val = (val & 0x3ff) | 0x1c00;
			    break;
			  }
			the_insn.exp = the_insn.exp2;
			the_insn.exp.X_add_number += val;
			the_insn.exp2.X_op = O_illegal;
			the_insn.reloc = old_reloc;
		      }
		    else if (the_insn.exp2.X_op != O_constant)
		      {
			as_bad (_("Illegal operands: Can't add non-constant expression to %%%s()"), op_arg);
			return special_case;
		      }
		    else
		      {
			if (old_reloc != BFD_RELOC_SPARC13
			    || the_insn.reloc != BFD_RELOC_LO10
			    || sparc_arch_size != 64
			    || sparc_pic_code)
			  {
			    as_bad (_("Illegal operands: Can't do arithmetics involving %%%s() of a relocatable symbol"), op_arg);
			    return special_case;
			  }
			the_insn.reloc = BFD_RELOC_SPARC_OLO10;
		      }
		  }
	      }
	      /* Check for constants that don't require emitting a reloc.  */
	      if (the_insn.exp.X_op == O_constant
		  && the_insn.exp.X_add_symbol == 0
		  && the_insn.exp.X_op_symbol == 0)
		{
		  /* For pc-relative call instructions, we reject
		     constants to get better code.  */
		  if (the_insn.pcrel
		      && the_insn.reloc == BFD_RELOC_32_PCREL_S2
		      && in_signed_range (the_insn.exp.X_add_number, 0x3fff))
		    {
		      error_message = _(": PC-relative operand can't be a constant");
		      goto error;
		    }

		  if (the_insn.reloc >= BFD_RELOC_SPARC_TLS_GD_HI22
		      && the_insn.reloc <= BFD_RELOC_SPARC_TLS_TPOFF64)
		    {
		      error_message = _(": TLS operand can't be a constant");
		      goto error;
		    }

		  /* Constants that won't fit are checked in md_apply_fix
		     and bfd_install_relocation.
		     ??? It would be preferable to install the constants
		     into the insn here and save having to create a fixS
		     for each one.  There already exists code to handle
		     all the various cases (e.g. in md_apply_fix and
		     bfd_install_relocation) so duplicating all that code
		     here isn't right.  */

		  /* This is a special case to handle cbcond instructions
		     properly, which can need two relocations.  The first
		     one is for the 5-bit immediate field and the latter
		     is going to be for the WDISP10 branch part.  We
		     handle the R_SPARC_5 immediate directly here so that
		     we don't need to add support for multiple relocations
		     in one instruction just yet.  */
		  if (the_insn.reloc == BFD_RELOC_SPARC_5
                      && ((insn->match & OP(0x3)) == 0))
		    {
		      valueT val = the_insn.exp.X_add_number;

		      the_insn.reloc = BFD_RELOC_NONE;
		      if (! in_bitfield_range (val, 0x1f))
			{
			  error_message = _(": Immediate value in cbcond is out of range.");
			  goto error;
			}
		      opcode |= val & 0x1f;
		    }
		}

	      continue;

	    case 'a':
	      if (*s++ == 'a')
		{
		  opcode |= ANNUL;
		  continue;
		}
	      break;

	    case 'A':
	      {
		int asi = 0;

		/* Parse an asi.  */
		if (*s == '#')
		  {
		    if (! parse_sparc_asi (&s, &sasi))
		      {
			error_message = _(": invalid ASI name");
			goto error;
		      }
		    asi = sasi->value;
		  }
		else
		  {
		    if (! parse_const_expr_arg (&s, &asi))
		      {
			error_message = _(": invalid ASI expression");
			goto error;
		      }
		    if (asi < 0 || asi > 255)
		      {
			error_message = _(": invalid ASI number");
			goto error;
		      }
		  }
		opcode |= ASI (asi);
		continue;
	      }			/* Alternate space.  */

	    case 'p':
	      if (startswith (s, "%psr"))
		{
		  s += 4;
		  continue;
		}
	      break;

	    case 'q':		/* Floating point queue.  */
	      if (startswith (s, "%fq"))
		{
		  s += 3;
		  continue;
		}
	      break;

	    case 'Q':		/* Coprocessor queue.  */
	      if (startswith (s, "%cq"))
		{
		  s += 3;
		  continue;
		}
	      break;

	    case 'S':
	      if (strcmp (str, "set") == 0
		  || strcmp (str, "setuw") == 0)
		{
		  special_case = SPECIAL_CASE_SET;
		  continue;
		}
	      else if (strcmp (str, "setsw") == 0)
		{
		  special_case = SPECIAL_CASE_SETSW;
		  continue;
		}
	      else if (strcmp (str, "setx") == 0)
		{
		  special_case = SPECIAL_CASE_SETX;
		  continue;
		}
	      else if (startswith (str, "fdiv"))
		{
		  special_case = SPECIAL_CASE_FDIV;
		  continue;
		}
	      break;

	    case 'o':
	      if (!startswith (s, "%asi"))
		break;
	      s += 4;
	      continue;

	    case 's':
	      if (!startswith (s, "%fprs"))
		break;
	      s += 5;
	      continue;

	    case '{':
	      if (!startswith (s, "%mcdper"))
		break;
	      s += 7;
	      continue;

            case '&':
              if (!startswith (s, "%entropy"))
                break;
              s += 8;
              continue;

	    case 'E':
	      if (!startswith (s, "%ccr"))
		break;
	      s += 4;
	      continue;

	    case 't':
	      if (!startswith (s, "%tbr"))
		break;
	      s += 4;
	      continue;

	    case 'w':
	      if (!startswith (s, "%wim"))
		break;
	      s += 4;
	      continue;

            case '|':
              {
                int imm2 = 0;

                /* Parse a 2-bit immediate.  */
                if (! parse_const_expr_arg (&s, &imm2))
                  {
                    error_message = _(": non-immdiate imm2 operand");
                    goto error;
                  }
                if ((imm2 & ~0x3) != 0)
                  {
                    error_message = _(": imm2 immediate operand out of range (0-3)");
                    goto error;
                  }

                opcode |= ((imm2 & 0x2) << 3) | (imm2 & 0x1);
                continue;
              }
              
	    case 'x':
	      {
		char *push = input_line_pointer;
		expressionS e;

		input_line_pointer = s;
		expression (&e);
		if (e.X_op == O_constant)
		  {
		    int n = e.X_add_number;
		    if (n != e.X_add_number || (n & ~0x1ff) != 0)
		      as_bad (_("OPF immediate operand out of range (0-0x1ff)"));
		    else
		      opcode |= e.X_add_number << 5;
		  }
		else
		  as_bad (_("non-immediate OPF operand, ignored"));
		s = input_line_pointer;
		input_line_pointer = push;
		continue;
	      }

	    case 'y':
	      if (!startswith (s, "%y"))
		break;
	      s += 2;
	      continue;

	    case 'u':
	    case 'U':
	      {
		/* Parse a sparclet cpreg.  */
		int cpreg;
		if (! parse_keyword_arg (sparc_encode_sparclet_cpreg, &s, &cpreg))
		  {
		    error_message = _(": invalid cpreg name");
		    goto error;
		  }
		opcode |= (*args == 'U' ? RS1 (cpreg) : RD (cpreg));
		continue;
	      }

	    default:
	      as_fatal (_("failed sanity check."));
	    }			/* switch on arg code.  */

	  /* Break out of for() loop.  */
	  break;
	}			/* For each arg that we expect.  */

    error:
      if (match == 0)
	{
	  /* Args don't match.  */
	  if (&insn[1] - sparc_opcodes < sparc_num_opcodes
	      && (insn->name == insn[1].name
		  || !strcmp (insn->name, insn[1].name)))
	    {
	      ++insn;
	      s = argsStart;
	      continue;
	    }
	  else
	    {
	      as_bad (_("Illegal operands%s"), error_message);
	      return special_case;
	    }
	}
      else
	{
	  /* We have a match.  Now see if the architecture is OK.  */
	  /* String to use in case of architecture warning.  */
	  const char *msg_str = str;
	  int needed_arch_mask = insn->architecture;

          /* Include the ASI architecture needed as well */
          if (sasi && needed_arch_mask > sasi->architecture)
            {
              needed_arch_mask = sasi->architecture;
              msg_str = sasi->name;
            }

	  uint64_t hwcaps = ((uint64_t) insn->hwcaps2 << 32) | insn->hwcaps;

#ifndef TE_SOLARIS
	  if (hwcaps)
		  hwcap_seen |= hwcaps;
#endif
	  if (v9_arg_p)
	    {
	      needed_arch_mask &=
		~(SPARC_OPCODE_ARCH_MASK (SPARC_OPCODE_ARCH_V9) - 1);
	      if (! needed_arch_mask)
		needed_arch_mask =
		  SPARC_OPCODE_ARCH_MASK (SPARC_OPCODE_ARCH_V9);
	    }

	  if (needed_arch_mask
	      & SPARC_OPCODE_SUPPORTED (current_architecture))
	    /* OK.  */
	    ;
	  /* Can we bump up the architecture?  */
	  else if (needed_arch_mask
		   & SPARC_OPCODE_SUPPORTED (max_architecture))
	    {
	      enum sparc_opcode_arch_val needed_architecture =
		sparc_ffs (SPARC_OPCODE_SUPPORTED (max_architecture)
			   & needed_arch_mask);

	      gas_assert (needed_architecture <= SPARC_OPCODE_ARCH_MAX);
	      if (warn_on_bump
		  && needed_architecture > warn_after_architecture)
		{
		  as_warn (_("architecture bumped from \"%s\" to \"%s\" on \"%s\""),
			   sparc_opcode_archs[current_architecture].name,
			   sparc_opcode_archs[needed_architecture].name,
			   msg_str);
		  warn_after_architecture = needed_architecture;
		}
	      current_architecture = needed_architecture;
	      hwcap_allowed
		= (hwcap_allowed
		   | hwcaps
		   | ((uint64_t) sparc_opcode_archs[current_architecture].hwcaps2 << 32)
		   | sparc_opcode_archs[current_architecture].hwcaps);
	    }
	  /* Conflict.  */
	  /* ??? This seems to be a bit fragile.  What if the next entry in
	     the opcode table is the one we want and it is supported?
	     It is possible to arrange the table today so that this can't
	     happen but what about tomorrow?  */
	  else
	    {
	      int arch, printed_one_p = 0;
	      char *p;
	      char required_archs[SPARC_OPCODE_ARCH_MAX * 16];

	      /* Create a list of the architectures that support the insn.  */
	      needed_arch_mask &= ~SPARC_OPCODE_SUPPORTED (max_architecture);
	      p = required_archs;
	      arch = sparc_ffs (needed_arch_mask);
	      while ((1 << arch) <= needed_arch_mask)
		{
		  if ((1 << arch) & needed_arch_mask)
		    {
		      if (printed_one_p)
			*p++ = '|';
		      strcpy (p, sparc_opcode_archs[arch].name);
		      p += strlen (p);
		      printed_one_p = 1;
		    }
		  ++arch;
		}

	      as_bad (_("Architecture mismatch on \"%s %s\"."), str, argsStart);
	      as_tsktsk (_("(Requires %s; requested architecture is %s.)"),
			 required_archs,
			 sparc_opcode_archs[max_architecture].name);
	      return special_case;
	    }

	  /* Make sure the hwcaps used by the instruction are
	     currently enabled.  */
	  if (hwcaps & ~hwcap_allowed)
	    {
	      const char *hwcap_name = get_hwcap_name(hwcaps & ~hwcap_allowed);

	      as_bad (_("Hardware capability \"%s\" not enabled for \"%s\"."),
		      hwcap_name, str);
	      return special_case;
	    }
	} /* If no match.  */

      break;
    } /* Forever looking for a match.  */

  the_insn.opcode = opcode;
  return special_case;
}

static char *
skip_over_keyword (char *q)
{
  for (q = q + (*q == '#' || *q == '%');
       ISALNUM (*q) || *q == '_';
       ++q)
    continue;
  return q;
}

static int
parse_sparc_asi (char **input_pointer_p, const sparc_asi **value_p)
{
  const sparc_asi *value;
  char c, *p, *q;

  p = *input_pointer_p;
  q = skip_over_keyword(p);
  c = *q;
  *q = 0;
  value = sparc_encode_asi (p);
  *q = c;
  if (value == NULL)
    return 0;
  *value_p = value;
  *input_pointer_p = q;
  return 1;
}

/* Parse an argument that can be expressed as a keyword.
   (eg: #StoreStore or %ccfr).
   The result is a boolean indicating success.
   If successful, INPUT_POINTER is updated.  */

static int
parse_keyword_arg (int (*lookup_fn) (const char *),
		   char **input_pointerP,
		   int *valueP)
{
  int value;
  char c, *p, *q;

  p = *input_pointerP;
  q = skip_over_keyword(p);
  c = *q;
  *q = 0;
  value = (*lookup_fn) (p);
  *q = c;
  if (value == -1)
    return 0;
  *valueP = value;
  *input_pointerP = q;
  return 1;
}

/* Parse an argument that is a constant expression.
   The result is a boolean indicating success.  */

static int
parse_const_expr_arg (char **input_pointerP, int *valueP)
{
  char *save = input_line_pointer;
  expressionS exp;

  input_line_pointer = *input_pointerP;
  /* The next expression may be something other than a constant
     (say if we're not processing the right variant of the insn).
     Don't call expression unless we're sure it will succeed as it will
     signal an error (which we want to defer until later).  */
  /* FIXME: It might be better to define md_operand and have it recognize
     things like %asi, etc. but continuing that route through to the end
     is a lot of work.  */
  if (*input_line_pointer == '%')
    {
      input_line_pointer = save;
      return 0;
    }
  expression (&exp);
  *input_pointerP = input_line_pointer;
  input_line_pointer = save;
  if (exp.X_op != O_constant)
    return 0;
  *valueP = exp.X_add_number;
  return 1;
}

/* Subroutine of sparc_ip to parse an expression.  */

static int
get_expression (char *str)
{
  char *save_in;
  segT seg;

  save_in = input_line_pointer;
  input_line_pointer = str;
  seg = expression (&the_insn.exp);
  if (seg != absolute_section
      && seg != text_section
      && seg != data_section
      && seg != bss_section
      && seg != undefined_section)
    {
      the_insn.error = _("bad segment");
      expr_parse_end = input_line_pointer;
      input_line_pointer = save_in;
      return 1;
    }
  expr_parse_end = input_line_pointer;
  input_line_pointer = save_in;
  return 0;
}

/* Subroutine of md_assemble to output one insn.  */

static void
output_insn (const struct sparc_opcode *insn, struct sparc_it *theinsn)
{
  char *toP = frag_more (4);

  /* Put out the opcode.  */
  if (INSN_BIG_ENDIAN)
    number_to_chars_bigendian (toP, (valueT) theinsn->opcode, 4);
  else
    number_to_chars_littleendian (toP, (valueT) theinsn->opcode, 4);

  /* Put out the symbol-dependent stuff.  */
  if (theinsn->reloc != BFD_RELOC_NONE)
    {
      fixS *fixP =  fix_new_exp (frag_now,	/* Which frag.  */
				 (toP - frag_now->fr_literal),	/* Where.  */
				 4,		/* Size.  */
				 &theinsn->exp,
				 theinsn->pcrel,
				 theinsn->reloc);
      /* Turn off overflow checking in fixup_segment.  We'll do our
	 own overflow checking in md_apply_fix.  This is necessary because
	 the insn size is 4 and fixup_segment will signal an overflow for
	 large 8 byte quantities.  */
      fixP->fx_no_overflow = 1;
      if (theinsn->reloc == BFD_RELOC_SPARC_OLO10)
	fixP->tc_fix_data = theinsn->exp2.X_add_number;
    }

  last_insn = insn;
  last_opcode = theinsn->opcode;

  dwarf2_emit_insn (4);
}

const char *
md_atof (int type, char *litP, int *sizeP)
{
  return ieee_md_atof (type, litP, sizeP, target_big_endian);
}

/* Write a value out to the object file, using the appropriate
   endianness.  */

void
md_number_to_chars (char *buf, valueT val, int n)
{
  if (target_big_endian)
    number_to_chars_bigendian (buf, val, n);
  else if (target_little_endian_data
	   && ((n == 4 || n == 2) && ~now_seg->flags & SEC_ALLOC))
    /* Output debug words, which are not in allocated sections, as big
       endian.  */
    number_to_chars_bigendian (buf, val, n);
  else if (target_little_endian_data || ! target_big_endian)
    number_to_chars_littleendian (buf, val, n);
}

/* Apply a fixS to the frags, now that we know the value it ought to
   hold.  */

void
md_apply_fix (fixS *fixP, valueT *valP, segT segment ATTRIBUTE_UNUSED)
{
  char *buf = fixP->fx_where + fixP->fx_frag->fr_literal;
  offsetT val = * (offsetT *) valP;
  long insn;

  gas_assert (fixP->fx_r_type < BFD_RELOC_UNUSED);

  fixP->fx_addnumber = val;	/* Remember value for emit_reloc.  */

  /* SPARC ELF relocations don't use an addend in the data field.  */
  if (fixP->fx_addsy != NULL)
    {
      switch (fixP->fx_r_type)
	{
	case BFD_RELOC_SPARC_TLS_GD_HI22:
	case BFD_RELOC_SPARC_TLS_GD_LO10:
	case BFD_RELOC_SPARC_TLS_GD_ADD:
	case BFD_RELOC_SPARC_TLS_GD_CALL:
	case BFD_RELOC_SPARC_TLS_LDM_HI22:
	case BFD_RELOC_SPARC_TLS_LDM_LO10:
	case BFD_RELOC_SPARC_TLS_LDM_ADD:
	case BFD_RELOC_SPARC_TLS_LDM_CALL:
	case BFD_RELOC_SPARC_TLS_LDO_HIX22:
	case BFD_RELOC_SPARC_TLS_LDO_LOX10:
	case BFD_RELOC_SPARC_TLS_LDO_ADD:
	case BFD_RELOC_SPARC_TLS_IE_HI22:
	case BFD_RELOC_SPARC_TLS_IE_LO10:
	case BFD_RELOC_SPARC_TLS_IE_LD:
	case BFD_RELOC_SPARC_TLS_IE_LDX:
	case BFD_RELOC_SPARC_TLS_IE_ADD:
	case BFD_RELOC_SPARC_TLS_LE_HIX22:
	case BFD_RELOC_SPARC_TLS_LE_LOX10:
	case BFD_RELOC_SPARC_TLS_DTPMOD32:
	case BFD_RELOC_SPARC_TLS_DTPMOD64:
	case BFD_RELOC_SPARC_TLS_DTPOFF32:
	case BFD_RELOC_SPARC_TLS_DTPOFF64:
	case BFD_RELOC_SPARC_TLS_TPOFF32:
	case BFD_RELOC_SPARC_TLS_TPOFF64:
	  S_SET_THREAD_LOCAL (fixP->fx_addsy);

	default:
	  break;
	}

      return;
    }

  /* This is a hack.  There should be a better way to
     handle this.  Probably in terms of howto fields, once
     we can look at these fixups in terms of howtos.  */
  if (fixP->fx_r_type == BFD_RELOC_32_PCREL_S2 && fixP->fx_addsy)
    val += fixP->fx_where + fixP->fx_frag->fr_address;

  /* If this is a data relocation, just output VAL.  */

  if (fixP->fx_r_type == BFD_RELOC_8)
    {
      md_number_to_chars (buf, val, 1);
    }
  else if (fixP->fx_r_type == BFD_RELOC_16
	   || fixP->fx_r_type == BFD_RELOC_SPARC_UA16)
    {
      md_number_to_chars (buf, val, 2);
    }
  else if (fixP->fx_r_type == BFD_RELOC_32
	   || fixP->fx_r_type == BFD_RELOC_SPARC_UA32
	   || fixP->fx_r_type == BFD_RELOC_SPARC_REV32)
    {
      md_number_to_chars (buf, val, 4);
    }
  else if (fixP->fx_r_type == BFD_RELOC_64
	   || fixP->fx_r_type == BFD_RELOC_SPARC_UA64)
    {
      md_number_to_chars (buf, val, 8);
    }
  else if (fixP->fx_r_type == BFD_RELOC_VTABLE_INHERIT
           || fixP->fx_r_type == BFD_RELOC_VTABLE_ENTRY)
    {
      fixP->fx_done = 0;
      return;
    }
  else
    {
      /* It's a relocation against an instruction.  */

      if (INSN_BIG_ENDIAN)
	insn = bfd_getb32 ((unsigned char *) buf);
      else
	insn = bfd_getl32 ((unsigned char *) buf);

      switch (fixP->fx_r_type)
	{
	case BFD_RELOC_32_PCREL_S2:
	  val = val >> 2;
	  /* FIXME: This increment-by-one deserves a comment of why it's
	     being done!  */
	  if (! sparc_pic_code
	      || fixP->fx_addsy == NULL
	      || symbol_section_p (fixP->fx_addsy))
	    ++val;

	  insn |= val & 0x3fffffff;

	  /* See if we have a delay slot.  In that case we attempt to
             optimize several cases transforming CALL instructions
             into branches.  But we can only do that if the relocation
             can be completely resolved here, i.e. if no undefined
             symbol is associated with it.  */
	  if (sparc_relax && fixP->fx_addsy == NULL
	      && fixP->fx_where + 8 <= fixP->fx_frag->fr_fix)
	    {
#define G0		0
#define O7		15
#define XCC		(2 << 20)
#define COND(x)		(((x)&0xf)<<25)
#define CONDA		COND(0x8)
#define INSN_BPA	(F2(0,1) | CONDA | BPRED | XCC)
#define INSN_BA		(F2(0,2) | CONDA)
#define INSN_OR		F3(2, 0x2, 0)
#define INSN_NOP	F2(0,4)

	      long delay;

	      /* If the instruction is a call with either:
		 restore
		 arithmetic instruction with rd == %o7
		 where rs1 != %o7 and rs2 if it is register != %o7
		 then we can optimize if the call destination is near
		 by changing the call into a branch always.  */
	      if (INSN_BIG_ENDIAN)
		delay = bfd_getb32 ((unsigned char *) buf + 4);
	      else
		delay = bfd_getl32 ((unsigned char *) buf + 4);
	      if ((insn & OP (~0)) != OP (1) || (delay & OP (~0)) != OP (2))
		break;
	      if ((delay & OP3 (~0)) != OP3 (0x3d) /* Restore.  */
		  && ((delay & OP3 (0x28)) != 0 /* Arithmetic.  */
		      || ((delay & RD (~0)) != RD (O7))))
		break;
	      if ((delay & RS1 (~0)) == RS1 (O7)
		  || ((delay & F3I (~0)) == 0
		      && (delay & RS2 (~0)) == RS2 (O7)))
		break;
	      /* Ensure the branch will fit into simm22.  */
	      if ((val & 0x3fe00000)
		  && (val & 0x3fe00000) != 0x3fe00000)
		break;
	      /* Check if the arch is v9 and branch will fit
		 into simm19.  */
	      if (((val & 0x3c0000) == 0
		   || (val & 0x3c0000) == 0x3c0000)
		  && (sparc_arch_size == 64
		      || current_architecture >= SPARC_OPCODE_ARCH_V9))
		/* ba,pt %xcc  */
		insn = INSN_BPA | (val & 0x7ffff);
	      else
		/* ba  */
		insn = INSN_BA | (val & 0x3fffff);
	      if (fixP->fx_where >= 4
		  && ((delay & (0xffffffff ^ RS1 (~0)))
		      == (INSN_OR | RD (O7) | RS2 (G0))))
		{
		  long setter;
		  int reg;

		  if (INSN_BIG_ENDIAN)
		    setter = bfd_getb32 ((unsigned char *) buf - 4);
		  else
		    setter = bfd_getl32 ((unsigned char *) buf - 4);
		  if ((setter & (0xffffffff ^ RD (~0)))
		      != (INSN_OR | RS1 (O7) | RS2 (G0)))
		    break;
		  /* The sequence was
		     or %o7, %g0, %rN
		     call foo
		     or %rN, %g0, %o7

		     If call foo was replaced with ba, replace
		     or %rN, %g0, %o7 with nop.  */
		  reg = (delay & RS1 (~0)) >> 14;
		  if (reg != ((setter & RD (~0)) >> 25)
		      || reg == G0 || reg == O7)
		    break;

		  if (INSN_BIG_ENDIAN)
		    bfd_putb32 (INSN_NOP, (unsigned char *) buf + 4);
		  else
		    bfd_putl32 (INSN_NOP, (unsigned char *) buf + 4);
		}
	    }
	  break;

	case BFD_RELOC_SPARC_11:
	  if (! in_signed_range (val, 0x7ff))
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("relocation overflow"));
	  insn |= val & 0x7ff;
	  break;

	case BFD_RELOC_SPARC_10:
	  if (! in_signed_range (val, 0x3ff))
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("relocation overflow"));
	  insn |= val & 0x3ff;
	  break;

	case BFD_RELOC_SPARC_7:
	  if (! in_bitfield_range (val, 0x7f))
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("relocation overflow"));
	  insn |= val & 0x7f;
	  break;

	case BFD_RELOC_SPARC_6:
	  if (! in_bitfield_range (val, 0x3f))
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("relocation overflow"));
	  insn |= val & 0x3f;
	  break;

	case BFD_RELOC_SPARC_5:
	  if (! in_bitfield_range (val, 0x1f))
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("relocation overflow"));
	  insn |= val & 0x1f;
	  break;

	case BFD_RELOC_SPARC_WDISP10:
	  if ((val & 3)
	      || val >= 0x007fc
	      || val <= -(offsetT) 0x808)
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("relocation overflow"));
	  /* FIXME: The +1 deserves a comment.  */
	  val = (val >> 2) + 1;
	  insn |= ((val & 0x300) << 11)
	    | ((val & 0xff) << 5);
	  break;

	case BFD_RELOC_SPARC_WDISP16:
	  if ((val & 3)
	      || val >= 0x1fffc
	      || val <= -(offsetT) 0x20008)
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("relocation overflow"));
	  /* FIXME: The +1 deserves a comment.  */
	  val = (val >> 2) + 1;
	  insn |= ((val & 0xc000) << 6) | (val & 0x3fff);
	  break;

	case BFD_RELOC_SPARC_WDISP19:
	  if ((val & 3)
	      || val >= 0xffffc
	      || val <= -(offsetT) 0x100008)
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("relocation overflow"));
	  /* FIXME: The +1 deserves a comment.  */
	  val = (val >> 2) + 1;
	  insn |= val & 0x7ffff;
	  break;

	case BFD_RELOC_SPARC_HH22:
	  val = BSR (val, 32);
	  /* Fall through.  */

	case BFD_RELOC_SPARC_LM22:
	case BFD_RELOC_HI22:
	  if (!fixP->fx_addsy)
	    insn |= (val >> 10) & 0x3fffff;
	  else
	    /* FIXME: Need comment explaining why we do this.  */
	    insn &= ~0xffff;
	  break;

	case BFD_RELOC_SPARC22:
	  if (val & ~0x003fffff)
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("relocation overflow"));
	  insn |= (val & 0x3fffff);
	  break;

	case BFD_RELOC_SPARC_HM10:
	  val = BSR (val, 32);
	  /* Fall through.  */

	case BFD_RELOC_LO10:
	  if (!fixP->fx_addsy)
	    insn |= val & 0x3ff;
	  else
	    /* FIXME: Need comment explaining why we do this.  */
	    insn &= ~0xff;
	  break;

	case BFD_RELOC_SPARC_OLO10:
	  val &= 0x3ff;
	  val += fixP->tc_fix_data;
	  /* Fall through.  */

	case BFD_RELOC_SPARC13:
	  if (! in_signed_range (val, 0x1fff))
	    as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("relocation overflow"));
	  insn |= val & 0x1fff;
	  break;

	case BFD_RELOC_SPARC_WDISP22:
	  val = (val >> 2) + 1;
	  /* Fall through.  */
	case BFD_RELOC_SPARC_BASE22:
	  insn |= val & 0x3fffff;
	  break;

	case BFD_RELOC_SPARC_H34:
	  if (!fixP->fx_addsy)
	    {
	      bfd_vma tval = val;
	      tval >>= 12;
	      insn |= tval & 0x3fffff;
	    }
	  break;

	case BFD_RELOC_SPARC_H44:
	  if (!fixP->fx_addsy)
	    {
	      bfd_vma tval = val;
	      tval >>= 22;
	      insn |= tval & 0x3fffff;
	    }
	  break;

	case BFD_RELOC_SPARC_M44:
	  if (!fixP->fx_addsy)
	    insn |= (val >> 12) & 0x3ff;
	  break;

	case BFD_RELOC_SPARC_L44:
	  if (!fixP->fx_addsy)
	    insn |= val & 0xfff;
	  break;

	case BFD_RELOC_SPARC_HIX22:
	  if (!fixP->fx_addsy)
	    {
	      val ^= ~(offsetT) 0;
	      insn |= (val >> 10) & 0x3fffff;
	    }
	  break;

	case BFD_RELOC_SPARC_LOX10:
	  if (!fixP->fx_addsy)
	    insn |= 0x1c00 | (val & 0x3ff);
	  break;

	case BFD_RELOC_NONE:
	default:
	  as_bad_where (fixP->fx_file, fixP->fx_line,
			_("bad or unhandled relocation type: 0x%02x"),
			fixP->fx_r_type);
	  break;
	}

      if (INSN_BIG_ENDIAN)
	bfd_putb32 (insn, (unsigned char *) buf);
      else
	bfd_putl32 (insn, (unsigned char *) buf);
    }

  /* Are we finished with this relocation now?  */
  if (fixP->fx_addsy == 0 && !fixP->fx_pcrel)
    fixP->fx_done = 1;
}

/* Translate internal representation of relocation info to BFD target
   format.  */

arelent **
tc_gen_reloc (asection *section, fixS *fixp)
{
  static arelent *relocs[3];
  arelent *reloc;
  bfd_reloc_code_real_type code;

  relocs[0] = reloc = XNEW (arelent);
  relocs[1] = NULL;

  reloc->sym_ptr_ptr = XNEW (asymbol *);
  *reloc->sym_ptr_ptr = symbol_get_bfdsym (fixp->fx_addsy);
  reloc->address = fixp->fx_frag->fr_address + fixp->fx_where;

  switch (fixp->fx_r_type)
    {
    case BFD_RELOC_8:
    case BFD_RELOC_16:
    case BFD_RELOC_32:
    case BFD_RELOC_64:
      if (fixp->fx_pcrel)
	{
	  switch (fixp->fx_size)
	    {
	    default:
	      as_bad_where (fixp->fx_file, fixp->fx_line,
			    _("can not do %d byte pc-relative relocation"),
			    fixp->fx_size);
	      code = fixp->fx_r_type;
	      fixp->fx_pcrel = 0;
	      break;
	    case 1: code = BFD_RELOC_8_PCREL;  break;
	    case 2: code = BFD_RELOC_16_PCREL; break;
	    case 4: code = BFD_RELOC_32_PCREL; break;
#ifdef BFD64
	    case 8: code = BFD_RELOC_64_PCREL; break;
#endif
	    }
	  if (fixp->fx_pcrel)
	    fixp->fx_addnumber = fixp->fx_offset;
	  break;
	}
      /* Fall through.  */
    case BFD_RELOC_HI22:
    case BFD_RELOC_LO10:
    case BFD_RELOC_32_PCREL_S2:
    case BFD_RELOC_SPARC13:
    case BFD_RELOC_SPARC22:
    case BFD_RELOC_SPARC_PC22:
    case BFD_RELOC_SPARC_PC10:
    case BFD_RELOC_SPARC_BASE13:
    case BFD_RELOC_SPARC_WDISP10:
    case BFD_RELOC_SPARC_WDISP16:
    case BFD_RELOC_SPARC_WDISP19:
    case BFD_RELOC_SPARC_WDISP22:
    case BFD_RELOC_SPARC_5:
    case BFD_RELOC_SPARC_6:
    case BFD_RELOC_SPARC_7:
    case BFD_RELOC_SPARC_10:
    case BFD_RELOC_SPARC_11:
    case BFD_RELOC_SPARC_HH22:
    case BFD_RELOC_SPARC_HM10:
    case BFD_RELOC_SPARC_LM22:
    case BFD_RELOC_SPARC_PC_HH22:
    case BFD_RELOC_SPARC_PC_HM10:
    case BFD_RELOC_SPARC_PC_LM22:
    case BFD_RELOC_SPARC_H34:
    case BFD_RELOC_SPARC_H44:
    case BFD_RELOC_SPARC_M44:
    case BFD_RELOC_SPARC_L44:
    case BFD_RELOC_SPARC_HIX22:
    case BFD_RELOC_SPARC_LOX10:
    case BFD_RELOC_SPARC_REV32:
    case BFD_RELOC_SPARC_OLO10:
    case BFD_RELOC_SPARC_UA16:
    case BFD_RELOC_SPARC_UA32:
    case BFD_RELOC_SPARC_UA64:
    case BFD_RELOC_8_PCREL:
    case BFD_RELOC_16_PCREL:
    case BFD_RELOC_32_PCREL:
    case BFD_RELOC_64_PCREL:
    case BFD_RELOC_SPARC_PLT32:
    case BFD_RELOC_SPARC_PLT64:
    case BFD_RELOC_VTABLE_ENTRY:
    case BFD_RELOC_VTABLE_INHERIT:
    case BFD_RELOC_SPARC_TLS_GD_HI22:
    case BFD_RELOC_SPARC_TLS_GD_LO10:
    case BFD_RELOC_SPARC_TLS_GD_ADD:
    case BFD_RELOC_SPARC_TLS_GD_CALL:
    case BFD_RELOC_SPARC_TLS_LDM_HI22:
    case BFD_RELOC_SPARC_TLS_LDM_LO10:
    case BFD_RELOC_SPARC_TLS_LDM_ADD:
    case BFD_RELOC_SPARC_TLS_LDM_CALL:
    case BFD_RELOC_SPARC_TLS_LDO_HIX22:
    case BFD_RELOC_SPARC_TLS_LDO_LOX10:
    case BFD_RELOC_SPARC_TLS_LDO_ADD:
    case BFD_RELOC_SPARC_TLS_IE_HI22:
    case BFD_RELOC_SPARC_TLS_IE_LO10:
    case BFD_RELOC_SPARC_TLS_IE_LD:
    case BFD_RELOC_SPARC_TLS_IE_LDX:
    case BFD_RELOC_SPARC_TLS_IE_ADD:
    case BFD_RELOC_SPARC_TLS_LE_HIX22:
    case BFD_RELOC_SPARC_TLS_LE_LOX10:
    case BFD_RELOC_SPARC_TLS_DTPOFF32:
    case BFD_RELOC_SPARC_TLS_DTPOFF64:
    case BFD_RELOC_SPARC_GOTDATA_OP_HIX22:
    case BFD_RELOC_SPARC_GOTDATA_OP_LOX10:
    case BFD_RELOC_SPARC_GOTDATA_OP:
      code = fixp->fx_r_type;
      break;
    default:
      abort ();
      return NULL;
    }

  /* If we are generating PIC code, we need to generate a different
     set of relocs.  */

#define GOT_NAME "_GLOBAL_OFFSET_TABLE_"
#ifdef TE_VXWORKS
#define GOTT_BASE "__GOTT_BASE__"
#define GOTT_INDEX "__GOTT_INDEX__"
#endif

  /* This code must be parallel to tc_fix_adjustable.  */

  if (sparc_pic_code)
    {
      switch (code)
	{
	case BFD_RELOC_32_PCREL_S2:
	  if (generic_force_reloc (fixp))
	    code = BFD_RELOC_SPARC_WPLT30;
	  break;
	case BFD_RELOC_HI22:
	  code = BFD_RELOC_SPARC_GOT22;
	  if (fixp->fx_addsy != NULL)
	    {
	      if (strcmp (S_GET_NAME (fixp->fx_addsy), GOT_NAME) == 0)
		code = BFD_RELOC_SPARC_PC22;
#ifdef TE_VXWORKS
	      if (strcmp (S_GET_NAME (fixp->fx_addsy), GOTT_BASE) == 0
		  || strcmp (S_GET_NAME (fixp->fx_addsy), GOTT_INDEX) == 0)
		code = BFD_RELOC_HI22; /* Unchanged.  */
#endif
	    }
	  break;
	case BFD_RELOC_LO10:
	  code = BFD_RELOC_SPARC_GOT10;
	  if (fixp->fx_addsy != NULL)
	    {
	      if (strcmp (S_GET_NAME (fixp->fx_addsy), GOT_NAME) == 0)
		code = BFD_RELOC_SPARC_PC10;
#ifdef TE_VXWORKS
	      if (strcmp (S_GET_NAME (fixp->fx_addsy), GOTT_BASE) == 0
		  || strcmp (S_GET_NAME (fixp->fx_addsy), GOTT_INDEX) == 0)
		code = BFD_RELOC_LO10; /* Unchanged.  */
#endif
	    }
	  break;
	case BFD_RELOC_SPARC13:
	  code = BFD_RELOC_SPARC_GOT13;
	  break;
	default:
	  break;
	}
    }

  /* Nothing is aligned in DWARF debugging sections.  */
  if (bfd_section_flags (section) & SEC_DEBUGGING)
    switch (code)
      {
      case BFD_RELOC_16: code = BFD_RELOC_SPARC_UA16; break;
      case BFD_RELOC_32: code = BFD_RELOC_SPARC_UA32; break;
      case BFD_RELOC_64: code = BFD_RELOC_SPARC_UA64; break;
      default: break;
      }

  if (code == BFD_RELOC_SPARC_OLO10)
    reloc->howto = bfd_reloc_type_lookup (stdoutput, BFD_RELOC_LO10);
  else
    reloc->howto = bfd_reloc_type_lookup (stdoutput, code);
  if (reloc->howto == 0)
    {
      as_bad_where (fixp->fx_file, fixp->fx_line,
		    _("internal error: can't export reloc type %d (`%s')"),
		    fixp->fx_r_type, bfd_get_reloc_code_name (code));
      xfree (reloc);
      relocs[0] = NULL;
      return relocs;
    }

  /* @@ Why fx_addnumber sometimes and fx_offset other times?  */
  if (code != BFD_RELOC_32_PCREL_S2
      && code != BFD_RELOC_SPARC_WDISP22
      && code != BFD_RELOC_SPARC_WDISP16
      && code != BFD_RELOC_SPARC_WDISP19
      && code != BFD_RELOC_SPARC_WDISP10
      && code != BFD_RELOC_SPARC_WPLT30
      && code != BFD_RELOC_SPARC_TLS_GD_CALL
      && code != BFD_RELOC_SPARC_TLS_LDM_CALL)
    reloc->addend = fixp->fx_addnumber;
  else if (symbol_section_p (fixp->fx_addsy))
    reloc->addend = (section->vma
		     + fixp->fx_addnumber
		     + md_pcrel_from (fixp));
  else
    reloc->addend = fixp->fx_offset;

  /* We expand R_SPARC_OLO10 to R_SPARC_LO10 and R_SPARC_13
     on the same location.  */
  if (code == BFD_RELOC_SPARC_OLO10)
    {
      relocs[1] = reloc = XNEW (arelent);
      relocs[2] = NULL;

      reloc->sym_ptr_ptr = XNEW (asymbol *);
      *reloc->sym_ptr_ptr
	= symbol_get_bfdsym (section_symbol (absolute_section));
      reloc->address = fixp->fx_frag->fr_address + fixp->fx_where;
      reloc->howto = bfd_reloc_type_lookup (stdoutput, BFD_RELOC_SPARC13);
      reloc->addend = fixp->tc_fix_data;
    }

  return relocs;
}

/* We have no need to default values of symbols.  */

symbolS *
md_undefined_symbol (char *name ATTRIBUTE_UNUSED)
{
  return 0;
}

/* Round up a section size to the appropriate boundary.  */

valueT
md_section_align (segT segment ATTRIBUTE_UNUSED, valueT size)
{
  return size;
}

/* Exactly what point is a PC-relative offset relative TO?
   On the sparc, they're relative to the address of the offset, plus
   its size.  This gets us to the following instruction.
   (??? Is this right?  FIXME-SOON)  */
long
md_pcrel_from (fixS *fixP)
{
  long ret;

  ret = fixP->fx_where + fixP->fx_frag->fr_address;
  if (! sparc_pic_code
      || fixP->fx_addsy == NULL
      || symbol_section_p (fixP->fx_addsy))
    ret += fixP->fx_size;
  return ret;
}

/* Return log2 (VALUE), or -1 if VALUE is not an exact positive power
   of two.  */

static int
mylog2 (int value)
{
  int shift;

  if (value <= 0)
    return -1;

  for (shift = 0; (value & 1) == 0; value >>= 1)
    ++shift;

  return (value == 1) ? shift : -1;
}

/* Sort of like s_lcomm.  */

static void
s_reserve (int ignore ATTRIBUTE_UNUSED)
{
  char *name;
  char *p;
  char c;
  int align;
  int size;
  int temp;
  symbolS *symbolP;

  c = get_symbol_name (&name);
  p = input_line_pointer;
  *p = c;
  SKIP_WHITESPACE_AFTER_NAME ();

  if (*input_line_pointer != ',')
    {
      as_bad (_("Expected comma after name"));
      ignore_rest_of_line ();
      return;
    }

  ++input_line_pointer;

  if ((size = get_absolute_expression ()) < 0)
    {
      as_bad (_("BSS length (%d.) <0! Ignored."), size);
      ignore_rest_of_line ();
      return;
    }				/* Bad length.  */

  *p = 0;
  symbolP = symbol_find_or_make (name);
  *p = c;

  if (!startswith (input_line_pointer, ",\"bss\"")
      && !startswith (input_line_pointer, ",\".bss\""))
    {
      as_bad (_("bad .reserve segment -- expected BSS segment"));
      return;
    }

  if (input_line_pointer[2] == '.')
    input_line_pointer += 7;
  else
    input_line_pointer += 6;
  SKIP_WHITESPACE ();

  if (*input_line_pointer == ',')
    {
      ++input_line_pointer;

      SKIP_WHITESPACE ();
      if (*input_line_pointer == '\n')
	{
	  as_bad (_("missing alignment"));
	  ignore_rest_of_line ();
	  return;
	}

      align = (int) get_absolute_expression ();

      if (align < 0)
	{
	  as_bad (_("negative alignment"));
	  ignore_rest_of_line ();
	  return;
	}

      if (align != 0)
	{
	  temp = mylog2 (align);
	  if (temp < 0)
	    {
	      as_bad (_("alignment not a power of 2"));
	      ignore_rest_of_line ();
	      return;
	    }

	  align = temp;
	}

      record_alignment (bss_section, align);
    }
  else
    align = 0;

  if (!S_IS_DEFINED (symbolP))
    {
      if (! need_pass_2)
	{
	  char *pfrag;
	  segT current_seg = now_seg;
	  subsegT current_subseg = now_subseg;

	  /* Switch to bss.  */
	  subseg_set (bss_section, 1);

	  if (align)
	    /* Do alignment.  */
	    frag_align (align, 0, 0);

	  /* Detach from old frag.  */
	  if (S_GET_SEGMENT (symbolP) == bss_section)
	    symbol_get_frag (symbolP)->fr_symbol = NULL;

	  symbol_set_frag (symbolP, frag_now);
	  pfrag = frag_var (rs_org, 1, 1, (relax_substateT) 0, symbolP,
			    (offsetT) size, (char *) 0);
	  *pfrag = 0;

	  S_SET_SEGMENT (symbolP, bss_section);

	  subseg_set (current_seg, current_subseg);

	  S_SET_SIZE (symbolP, size);
	}
    }
  else
    {
      as_warn (_("Ignoring attempt to re-define symbol %s"),
	       S_GET_NAME (symbolP));
    }

  demand_empty_rest_of_line ();
}

static void
s_common (int ignore ATTRIBUTE_UNUSED)
{
  char *name;
  char c;
  char *p;
  offsetT temp, size;
  symbolS *symbolP;

  c = get_symbol_name (&name);
  /* Just after name is now '\0'.  */
  p = input_line_pointer;
  *p = c;
  SKIP_WHITESPACE_AFTER_NAME ();
  if (*input_line_pointer != ',')
    {
      as_bad (_("Expected comma after symbol-name"));
      ignore_rest_of_line ();
      return;
    }

  /* Skip ','.  */
  input_line_pointer++;

  if ((temp = get_absolute_expression ()) < 0)
    {
      as_bad (_(".COMMon length (%lu) out of range ignored"),
	      (unsigned long) temp);
      ignore_rest_of_line ();
      return;
    }
  size = temp;
  *p = 0;
  symbolP = symbol_find_or_make (name);
  *p = c;
  if (S_IS_DEFINED (symbolP) && ! S_IS_COMMON (symbolP))
    {
      as_bad (_("Ignoring attempt to re-define symbol"));
      ignore_rest_of_line ();
      return;
    }
  if (S_GET_VALUE (symbolP) != 0)
    {
      if (S_GET_VALUE (symbolP) != (valueT) size)
	{
	  as_warn (_("Length of .comm \"%s\" is already %ld. Not changed to %ld."),
		   S_GET_NAME (symbolP), (long) S_GET_VALUE (symbolP), (long) size);
	}
    }
  know (symbol_get_frag (symbolP) == &zero_address_frag);
  if (*input_line_pointer != ',')
    {
      as_bad (_("Expected comma after common length"));
      ignore_rest_of_line ();
      return;
    }
  input_line_pointer++;
  SKIP_WHITESPACE ();
  if (*input_line_pointer != '"')
    {
      temp = get_absolute_expression ();

      if (temp < 0)
	{
	  as_bad (_("negative alignment"));
	  ignore_rest_of_line ();
	  return;
	}

      if (symbol_get_obj (symbolP)->local)
	{
	  segT old_sec;
	  int old_subsec;
	  int align;

	  old_sec = now_seg;
	  old_subsec = now_subseg;

	  if (temp == 0)
	    align = 0;
	  else
	    align = mylog2 (temp);

	  if (align < 0)
	    {
	      as_bad (_("alignment not a power of 2"));
	      ignore_rest_of_line ();
	      return;
	    }

	  record_alignment (bss_section, align);
	  subseg_set (bss_section, 0);
	  if (align)
	    frag_align (align, 0, 0);
	  if (S_GET_SEGMENT (symbolP) == bss_section)
	    symbol_get_frag (symbolP)->fr_symbol = 0;
	  symbol_set_frag (symbolP, frag_now);
	  p = frag_var (rs_org, 1, 1, (relax_substateT) 0, symbolP,
			(offsetT) size, (char *) 0);
	  *p = 0;
	  S_SET_SEGMENT (symbolP, bss_section);
	  S_CLEAR_EXTERNAL (symbolP);
	  S_SET_SIZE (symbolP, size);
	  subseg_set (old_sec, old_subsec);
	}
      else
	{
	allocate_common:
	  S_SET_VALUE (symbolP, (valueT) size);
	  S_SET_ALIGN (symbolP, temp);
	  S_SET_SIZE (symbolP, size);
	  S_SET_EXTERNAL (symbolP);
	  S_SET_SEGMENT (symbolP, bfd_com_section_ptr);
	}
    }
  else
    {
      input_line_pointer++;
      /* @@ Some use the dot, some don't.  Can we get some consistency??  */
      if (*input_line_pointer == '.')
	input_line_pointer++;
      /* @@ Some say data, some say bss.  */
      if (!startswith (input_line_pointer, "bss\"")
	  && !startswith (input_line_pointer, "data\""))
	{
	  while (*--input_line_pointer != '"')
	    ;
	  input_line_pointer--;
	  goto bad_common_segment;
	}
      while (*input_line_pointer++ != '"')
	;
      goto allocate_common;
    }

  symbol_get_bfdsym (symbolP)->flags |= BSF_OBJECT;

  demand_empty_rest_of_line ();
  return;

  {
  bad_common_segment:
    p = input_line_pointer;
    while (*p && *p != '\n')
      p++;
    c = *p;
    *p = '\0';
    as_bad (_("bad .common segment %s"), input_line_pointer + 1);
    *p = c;
    input_line_pointer = p;
    ignore_rest_of_line ();
    return;
  }
}

/* Handle the .empty pseudo-op.  This suppresses the warnings about
   invalid delay slot usage.  */

static void
s_empty (int ignore ATTRIBUTE_UNUSED)
{
  /* The easy way to implement is to just forget about the last
     instruction.  */
  last_insn = NULL;
}

static void
s_seg (int ignore ATTRIBUTE_UNUSED)
{

  if (startswith (input_line_pointer, "\"text\""))
    {
      input_line_pointer += 6;
      s_text (0);
      return;
    }
  if (startswith (input_line_pointer, "\"data\""))
    {
      input_line_pointer += 6;
      s_data (0);
      return;
    }
  if (startswith (input_line_pointer, "\"data1\""))
    {
      input_line_pointer += 7;
      s_data1 ();
      return;
    }
  if (startswith (input_line_pointer, "\"bss\""))
    {
      input_line_pointer += 5;
      /* We only support 2 segments -- text and data -- for now, so
	 things in the "bss segment" will have to go into data for now.
	 You can still allocate SEG_BSS stuff with .lcomm or .reserve.  */
      subseg_set (data_section, 255);	/* FIXME-SOMEDAY.  */
      return;
    }
  as_bad (_("Unknown segment type"));
  demand_empty_rest_of_line ();
}

static void
s_data1 (void)
{
  subseg_set (data_section, 1);
  demand_empty_rest_of_line ();
}

static void
s_proc (int ignore ATTRIBUTE_UNUSED)
{
  while (!is_end_of_line[(unsigned char) *input_line_pointer])
    {
      ++input_line_pointer;
    }
  ++input_line_pointer;
}

/* This static variable is set by s_uacons to tell sparc_cons_align
   that the expression does not need to be aligned.  */

static int sparc_no_align_cons = 0;

/* This handles the unaligned space allocation pseudo-ops, such as
   .uaword.  .uaword is just like .word, but the value does not need
   to be aligned.  */

static void
s_uacons (int bytes)
{
  /* Tell sparc_cons_align not to align this value.  */
  sparc_no_align_cons = 1;
  cons (bytes);
  sparc_no_align_cons = 0;
}

/* This handles the native word allocation pseudo-op .nword.
   For sparc_arch_size 32 it is equivalent to .word,  for
   sparc_arch_size 64 it is equivalent to .xword.  */

static void
s_ncons (int bytes ATTRIBUTE_UNUSED)
{
  cons (sparc_arch_size == 32 ? 4 : 8);
}

/* Handle the SPARC ELF .register pseudo-op.  This sets the binding of a
   global register.
   The syntax is:

   .register %g[2367],{#scratch|symbolname|#ignore}
*/

static void
s_register (int ignore ATTRIBUTE_UNUSED)
{
  char c;
  int reg;
  int flags;
  char *regname;

  if (input_line_pointer[0] != '%'
      || input_line_pointer[1] != 'g'
      || ((input_line_pointer[2] & ~1) != '2'
	  && (input_line_pointer[2] & ~1) != '6')
      || input_line_pointer[3] != ',')
    as_bad (_("register syntax is .register %%g[2367],{#scratch|symbolname|#ignore}"));
  reg = input_line_pointer[2] - '0';
  input_line_pointer += 4;

  if (*input_line_pointer == '#')
    {
      ++input_line_pointer;
      c = get_symbol_name (&regname);
      if (strcmp (regname, "scratch") && strcmp (regname, "ignore"))
	as_bad (_("register syntax is .register %%g[2367],{#scratch|symbolname|#ignore}"));
      if (regname[0] == 'i')
	regname = NULL;
      else
	regname = (char *) "";
    }
  else
    {
      c = get_symbol_name (&regname);
    }

  if (sparc_arch_size == 64)
    {
      if (globals[reg])
	{
	  if ((regname && globals[reg] != (symbolS *) 1
	       && strcmp (S_GET_NAME (globals[reg]), regname))
	      || ((regname != NULL) ^ (globals[reg] != (symbolS *) 1)))
	    as_bad (_("redefinition of global register"));
	}
      else
	{
	  if (regname == NULL)
	    globals[reg] = (symbolS *) 1;
	  else
	    {
	      if (*regname)
		{
		  if (symbol_find (regname))
		    as_bad (_("Register symbol %s already defined."),
			    regname);
		}
	      globals[reg] = symbol_make (regname);
	      flags = symbol_get_bfdsym (globals[reg])->flags;
	      if (! *regname)
		flags = flags & ~(BSF_GLOBAL|BSF_LOCAL|BSF_WEAK);
	      if (! (flags & (BSF_GLOBAL|BSF_LOCAL|BSF_WEAK)))
		flags |= BSF_GLOBAL;
	      symbol_get_bfdsym (globals[reg])->flags = flags;
	      S_SET_VALUE (globals[reg], (valueT) reg);
	      S_SET_ALIGN (globals[reg], reg);
	      S_SET_SIZE (globals[reg], 0);
	      /* Although we actually want undefined_section here,
		 we have to use absolute_section, because otherwise
		 generic as code will make it a COM section.
		 We fix this up in sparc_adjust_symtab.  */
	      S_SET_SEGMENT (globals[reg], absolute_section);
	      S_SET_OTHER (globals[reg], 0);
	      elf_symbol (symbol_get_bfdsym (globals[reg]))
		->internal_elf_sym.st_info =
		  ELF_ST_INFO(STB_GLOBAL, STT_REGISTER);
	      elf_symbol (symbol_get_bfdsym (globals[reg]))
		->internal_elf_sym.st_shndx = SHN_UNDEF;
	    }
	}
    }

  (void) restore_line_pointer (c);

  demand_empty_rest_of_line ();
}

/* Adjust the symbol table.  We set undefined sections for STT_REGISTER
   symbols which need it.  */

void
sparc_adjust_symtab (void)
{
  symbolS *sym;

  for (sym = symbol_rootP; sym != NULL; sym = symbol_next (sym))
    {
      if (ELF_ST_TYPE (elf_symbol (symbol_get_bfdsym (sym))
		       ->internal_elf_sym.st_info) != STT_REGISTER)
	continue;

      if (ELF_ST_TYPE (elf_symbol (symbol_get_bfdsym (sym))
		       ->internal_elf_sym.st_shndx != SHN_UNDEF))
	continue;

      S_SET_SEGMENT (sym, undefined_section);
    }
}

/* If the --enforce-aligned-data option is used, we require .word,
   et. al., to be aligned correctly.  We do it by setting up an
   rs_align_code frag, and checking in HANDLE_ALIGN to make sure that
   no unexpected alignment was introduced.

   The SunOS and Solaris native assemblers enforce aligned data by
   default.  We don't want to do that, because gcc can deliberately
   generate misaligned data if the packed attribute is used.  Instead,
   we permit misaligned data by default, and permit the user to set an
   option to check for it.  */

void
sparc_cons_align (int nbytes)
{
  int nalign;

  /* Only do this if we are enforcing aligned data.  */
  if (! enforce_aligned_data)
    return;

  /* Don't align if this is an unaligned pseudo-op.  */
  if (sparc_no_align_cons)
    return;

  nalign = mylog2 (nbytes);
  if (nalign == 0)
    return;

  gas_assert (nalign > 0);

  if (now_seg == absolute_section)
    {
      if ((abs_section_offset & ((1 << nalign) - 1)) != 0)
	as_bad (_("misaligned data"));
      return;
    }

  frag_var (rs_align_test, 1, 1, (relax_substateT) 0,
	    (symbolS *) NULL, (offsetT) nalign, (char *) NULL);

  record_alignment (now_seg, nalign);
}

/* This is called from HANDLE_ALIGN in tc-sparc.h.  */

void
sparc_handle_align (fragS *fragp)
{
  int count, fix;
  char *p;

  count = fragp->fr_next->fr_address - fragp->fr_address - fragp->fr_fix;

  switch (fragp->fr_type)
    {
    case rs_align_test:
      if (count != 0)
	as_bad_where (fragp->fr_file, fragp->fr_line, _("misaligned data"));
      break;

    case rs_align_code:
      p = fragp->fr_literal + fragp->fr_fix;
      fix = 0;

      if (count & 3)
	{
	  fix = count & 3;
	  memset (p, 0, fix);
	  p += fix;
	  count -= fix;
	}

      if (SPARC_OPCODE_ARCH_V9_P (max_architecture) && count > 8)
	{
	  unsigned wval = (0x30680000 | count >> 2); /* ba,a,pt %xcc, 1f  */
	  if (INSN_BIG_ENDIAN)
	    number_to_chars_bigendian (p, wval, 4);
	  else
	    number_to_chars_littleendian (p, wval, 4);
	  p += 4;
	  count -= 4;
	  fix += 4;
	}

      if (INSN_BIG_ENDIAN)
	number_to_chars_bigendian (p, 0x01000000, 4);
      else
	number_to_chars_littleendian (p, 0x01000000, 4);

      fragp->fr_fix += fix;
      fragp->fr_var = 4;
      break;

    default:
      break;
    }
}

/* Some special processing for a Sparc ELF file.  */

void
sparc_elf_final_processing (void)
{
  /* Set the Sparc ELF flag bits.  FIXME: There should probably be some
     sort of BFD interface for this.  */
  if (sparc_arch_size == 64)
    {
      switch (sparc_memory_model)
	{
	case MM_RMO:
	  elf_elfheader (stdoutput)->e_flags |= EF_SPARCV9_RMO;
	  break;
	case MM_PSO:
	  elf_elfheader (stdoutput)->e_flags |= EF_SPARCV9_PSO;
	  break;
	default:
	  break;
	}
    }
  else if (current_architecture >= SPARC_OPCODE_ARCH_V9)
    elf_elfheader (stdoutput)->e_flags |= EF_SPARC_32PLUS;
  if (current_architecture == SPARC_OPCODE_ARCH_V9A)
    elf_elfheader (stdoutput)->e_flags |= EF_SPARC_SUN_US1;
  else if (current_architecture == SPARC_OPCODE_ARCH_V9B)
    elf_elfheader (stdoutput)->e_flags |= EF_SPARC_SUN_US1|EF_SPARC_SUN_US3;
}

const char *
sparc_cons (expressionS *exp, int size)
{
  char *save;
  const char *sparc_cons_special_reloc = NULL;

  SKIP_WHITESPACE ();
  save = input_line_pointer;
  if (input_line_pointer[0] == '%'
      && input_line_pointer[1] == 'r'
      && input_line_pointer[2] == '_')
    {
      if (startswith (input_line_pointer + 3, "disp"))
	{
	  input_line_pointer += 7;
	  sparc_cons_special_reloc = "disp";
	}
      else if (startswith (input_line_pointer + 3, "plt"))
	{
	  if (size != 4 && size != 8)
	    as_bad (_("Illegal operands: %%r_plt in %d-byte data field"), size);
	  else
	    {
	      input_line_pointer += 6;
	      sparc_cons_special_reloc = "plt";
	    }
	}
      else if (startswith (input_line_pointer + 3, "tls_dtpoff"))
	{
	  if (size != 4 && size != 8)
	    as_bad (_("Illegal operands: %%r_tls_dtpoff in %d-byte data field"), size);
	  else
	    {
	      input_line_pointer += 13;
	      sparc_cons_special_reloc = "tls_dtpoff";
	    }
	}
      if (sparc_cons_special_reloc)
	{
	  int bad = 0;

	  switch (size)
	    {
	    case 1:
	      if (*input_line_pointer != '8')
		bad = 1;
	      input_line_pointer--;
	      break;
	    case 2:
	      if (input_line_pointer[0] != '1' || input_line_pointer[1] != '6')
		bad = 1;
	      break;
	    case 4:
	      if (input_line_pointer[0] != '3' || input_line_pointer[1] != '2')
		bad = 1;
	      break;
	    case 8:
	      if (input_line_pointer[0] != '6' || input_line_pointer[1] != '4')
		bad = 1;
	      break;
	    default:
	      bad = 1;
	      break;
	    }

	  if (bad)
	    {
	      as_bad (_("Illegal operands: Only %%r_%s%d allowed in %d-byte data fields"),
		      sparc_cons_special_reloc, size * 8, size);
	    }
	  else
	    {
	      input_line_pointer += 2;
	      if (*input_line_pointer != '(')
		{
		  as_bad (_("Illegal operands: %%r_%s%d requires arguments in ()"),
			  sparc_cons_special_reloc, size * 8);
		  bad = 1;
		}
	    }

	  if (bad)
	    {
	      input_line_pointer = save;
	      sparc_cons_special_reloc = NULL;
	    }
	  else
	    {
	      int c;
	      char *end = ++input_line_pointer;
	      int npar = 0;

	      while (! is_end_of_line[(c = *end)])
		{
		  if (c == '(')
	  	    npar++;
		  else if (c == ')')
	  	    {
		      if (!npar)
	      		break;
		      npar--;
		    }
	    	  end++;
		}

	      if (c != ')')
		as_bad (_("Illegal operands: %%r_%s%d requires arguments in ()"),
			sparc_cons_special_reloc, size * 8);
	      else
		{
		  *end = '\0';
		  expression (exp);
		  *end = c;
		  if (input_line_pointer != end)
		    {
		      as_bad (_("Illegal operands: %%r_%s%d requires arguments in ()"),
			      sparc_cons_special_reloc, size * 8);
		    }
		  else
		    {
		      input_line_pointer++;
		      SKIP_WHITESPACE ();
		      c = *input_line_pointer;
		      if (! is_end_of_line[c] && c != ',')
			as_bad (_("Illegal operands: garbage after %%r_%s%d()"),
			        sparc_cons_special_reloc, size * 8);
		    }
		}
	    }
	}
    }
  if (sparc_cons_special_reloc == NULL)
    expression (exp);
  return sparc_cons_special_reloc;
}

/* This is called by emit_expr via TC_CONS_FIX_NEW when creating a
   reloc for a cons.  We could use the definition there, except that
   we want to handle little endian relocs specially.  */

void
cons_fix_new_sparc (fragS *frag,
		    int where,
		    unsigned int nbytes,
		    expressionS *exp,
		    const char *sparc_cons_special_reloc)
{
  bfd_reloc_code_real_type r;

  r = (nbytes == 1 ? BFD_RELOC_8 :
       (nbytes == 2 ? BFD_RELOC_16 :
	(nbytes == 4 ? BFD_RELOC_32 : BFD_RELOC_64)));

  if (target_little_endian_data
      && nbytes == 4
      && now_seg->flags & SEC_ALLOC)
    r = BFD_RELOC_SPARC_REV32;

#ifdef TE_SOLARIS
  /* The Solaris linker does not allow R_SPARC_UA64
     relocations for 32-bit executables.  */
  if (!target_little_endian_data
      && sparc_arch_size != 64
      && r == BFD_RELOC_64)
    r = BFD_RELOC_32;
#endif

  if (sparc_cons_special_reloc)
    {
      if (*sparc_cons_special_reloc == 'd')
	switch (nbytes)
	  {
	  case 1: r = BFD_RELOC_8_PCREL; break;
	  case 2: r = BFD_RELOC_16_PCREL; break;
	  case 4: r = BFD_RELOC_32_PCREL; break;
	  case 8: r = BFD_RELOC_64_PCREL; break;
	  default: abort ();
	  }
      else if (*sparc_cons_special_reloc == 'p')
	switch (nbytes)
	  {
	  case 4: r = BFD_RELOC_SPARC_PLT32; break;
	  case 8: r = BFD_RELOC_SPARC_PLT64; break;
	  }
      else
	switch (nbytes)
	  {
	  case 4: r = BFD_RELOC_SPARC_TLS_DTPOFF32; break;
	  case 8: r = BFD_RELOC_SPARC_TLS_DTPOFF64; break;
	  }
    }
  else if (sparc_no_align_cons
	   || /* PR 20803 - relocs in the .eh_frame section
		 need to support unaligned access.  */
	   strcmp (now_seg->name, ".eh_frame") == 0)
    {
      switch (nbytes)
	{
	case 2: r = BFD_RELOC_SPARC_UA16; break;
	case 4: r = BFD_RELOC_SPARC_UA32; break;
#ifdef TE_SOLARIS
        /* The Solaris linker does not allow R_SPARC_UA64
	   relocations for 32-bit executables.  */
        case 8: r = sparc_arch_size == 64 ?
                    BFD_RELOC_SPARC_UA64 : BFD_RELOC_SPARC_UA32; break;
#else
	case 8: r = BFD_RELOC_SPARC_UA64; break;
#endif
	default: abort ();
	}
   }

  fix_new_exp (frag, where, (int) nbytes, exp, 0, r);
}

void
sparc_cfi_frame_initial_instructions (void)
{
  cfi_add_CFA_def_cfa (14, sparc_arch_size == 64 ? 0x7ff : 0);
}

int
sparc_regname_to_dw2regnum (char *regname)
{
  char *q;
  int i;

  if (!regname[0])
    return -1;

  switch (regname[0])
    {
    case 'g': i = 0; break;
    case 'o': i = 1; break;
    case 'l': i = 2; break;
    case 'i': i = 3; break;
    default: i = -1; break;
    }
  if (i != -1)
    {
      if (regname[1] < '0' || regname[1] > '8' || regname[2])
	return -1;
      return i * 8 + regname[1] - '0';
    }
  if (regname[0] == 's' && regname[1] == 'p' && !regname[2])
    return 14;
  if (regname[0] == 'f' && regname[1] == 'p' && !regname[2])
    return 30;
  if (regname[0] == 'f' || regname[0] == 'r')
    {
      unsigned int regnum;

      regnum = strtoul (regname + 1, &q, 10);
      if (q == NULL || *q)
        return -1;
      if (regnum >= ((regname[0] == 'f'
		      && SPARC_OPCODE_ARCH_V9_P (max_architecture))
		     ? 64 : 32))
	return -1;
      if (regname[0] == 'f')
	{
          regnum += 32;
          if (regnum >= 64 && (regnum & 1))
	    return -1;
        }
      return regnum;
    }
  return -1;
}

void
sparc_cfi_emit_pcrel_expr (expressionS *exp, unsigned int nbytes)
{
  sparc_no_align_cons = 1;
  emit_expr_with_reloc (exp, nbytes, "disp");
  sparc_no_align_cons = 0;
}
