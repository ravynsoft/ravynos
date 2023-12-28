/* tc-rx.c -- Assembler for the Renesas RX
   Copyright (C) 2008-2023 Free Software Foundation, Inc.

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
#include "dwarf2dbg.h"
#include "elf/common.h"
#include "elf/rx.h"
#include "rx-defs.h"
#include "filenames.h"
#include "listing.h"
#include "sb.h"
#include "macro.h"

#define RX_OPCODE_BIG_ENDIAN 0

const char comment_chars[]        = ";";
/* Note that input_file.c hand checks for '#' at the beginning of the
   first line of the input file.  This is because the compiler outputs
   #NO_APP at the beginning of its output.  */
const char line_comment_chars[]   = "#";
const char line_separator_chars[] = "!";

const char EXP_CHARS[]            = "eE";
const char FLT_CHARS[]            = "dD";

#ifndef TE_LINUX
bool rx_use_conventional_section_names = false;
static int elf_flags = E_FLAG_RX_ABI;
#else
bool rx_use_conventional_section_names = true;
static int elf_flags;
#endif

static bool rx_use_small_data_limit = false;
static bool rx_pid_mode = false;
static int rx_num_int_regs = 0;
int rx_pid_register;
int rx_gp_register;

enum rx_cpu_types rx_cpu = RX600;

static void rx_fetchalign (int ignore ATTRIBUTE_UNUSED);

enum options
{
  OPTION_BIG = OPTION_MD_BASE,
  OPTION_LITTLE,
  OPTION_32BIT_DOUBLES,
  OPTION_64BIT_DOUBLES,
  OPTION_CONVENTIONAL_SECTION_NAMES,
  OPTION_RENESAS_SECTION_NAMES,
  OPTION_SMALL_DATA_LIMIT,
  OPTION_RELAX,
  OPTION_PID,
  OPTION_INT_REGS,
  OPTION_USES_GCC_ABI,
  OPTION_USES_RX_ABI,
  OPTION_CPU,
  OPTION_DISALLOW_STRING_INSNS,
};

#define RX_SHORTOPTS ""
const char * md_shortopts = RX_SHORTOPTS;

/* Assembler options.  */
struct option md_longopts[] =
{
  {"mbig-endian-data", no_argument, NULL, OPTION_BIG},
  {"mlittle-endian-data", no_argument, NULL, OPTION_LITTLE},
  /* The next two switches are here because the
     generic parts of the linker testsuite uses them.  */
  {"EB", no_argument, NULL, OPTION_BIG},
  {"EL", no_argument, NULL, OPTION_LITTLE},
  {"m32bit-doubles", no_argument, NULL, OPTION_32BIT_DOUBLES},
  {"m64bit-doubles", no_argument, NULL, OPTION_64BIT_DOUBLES},
  /* This option is here mainly for the binutils testsuites,
     as many of their tests assume conventional section naming.  */
  {"muse-conventional-section-names", no_argument, NULL, OPTION_CONVENTIONAL_SECTION_NAMES},
  {"muse-renesas-section-names", no_argument, NULL, OPTION_RENESAS_SECTION_NAMES},
  {"msmall-data-limit", no_argument, NULL, OPTION_SMALL_DATA_LIMIT},
  {"relax", no_argument, NULL, OPTION_RELAX},
  {"mpid", no_argument, NULL, OPTION_PID},
  {"mint-register", required_argument, NULL, OPTION_INT_REGS},
  {"mgcc-abi", no_argument, NULL, OPTION_USES_GCC_ABI},
  {"mrx-abi", no_argument, NULL, OPTION_USES_RX_ABI},
  {"mcpu", required_argument, NULL, OPTION_CPU},
  {"mno-allow-string-insns", no_argument, NULL, OPTION_DISALLOW_STRING_INSNS},
  {NULL, no_argument, NULL, 0}
};
size_t md_longopts_size = sizeof (md_longopts);

struct cpu_type
{
  const char *cpu_name;
  enum rx_cpu_types type;
  int flag;
};

struct cpu_type  cpu_type_list[] =
{
  {"rx100", RX100, 0},
  {"rx200", RX200, 0},
  {"rx600", RX600, 0},
  {"rx610", RX610, 0},
  {"rxv2",  RXV2,  E_FLAG_RX_V2},
  {"rxv3",  RXV3,  E_FLAG_RX_V3},
  {"rxv3-dfpu",  RXV3FPU,  E_FLAG_RX_V3},
};

int
md_parse_option (int c ATTRIBUTE_UNUSED, const char * arg ATTRIBUTE_UNUSED)
{
  switch (c)
    {
    case OPTION_BIG:
      target_big_endian = 1;
      return 1;

    case OPTION_LITTLE:
      target_big_endian = 0;
      return 1;

    case OPTION_32BIT_DOUBLES:
      elf_flags &= ~ E_FLAG_RX_64BIT_DOUBLES;
      return 1;

    case OPTION_64BIT_DOUBLES:
      elf_flags |= E_FLAG_RX_64BIT_DOUBLES;
      return 1;

    case OPTION_CONVENTIONAL_SECTION_NAMES:
      rx_use_conventional_section_names = true;
      return 1;

    case OPTION_RENESAS_SECTION_NAMES:
      rx_use_conventional_section_names = false;
      return 1;

    case OPTION_SMALL_DATA_LIMIT:
      rx_use_small_data_limit = true;
      return 1;

    case OPTION_RELAX:
      linkrelax = 1;
      return 1;

    case OPTION_PID:
      rx_pid_mode = true;
      elf_flags |= E_FLAG_RX_PID;
      return 1;

    case OPTION_INT_REGS:
      rx_num_int_regs = atoi (optarg);
      return 1;

    case OPTION_USES_GCC_ABI:
      elf_flags &= ~ E_FLAG_RX_ABI;
      return 1;

    case OPTION_USES_RX_ABI:
      elf_flags |= E_FLAG_RX_ABI;
      return 1;

    case OPTION_CPU:
      {
	unsigned int i;
	for (i = 0; i < ARRAY_SIZE (cpu_type_list); i++)
	  {
	    if (strcasecmp (arg, cpu_type_list[i].cpu_name) == 0)
	      {
		rx_cpu = cpu_type_list[i].type;
		elf_flags |= cpu_type_list[i].flag;
		return 1;
	      }
	  }
	as_warn (_("unrecognised RX CPU type %s"), arg);
	break;
      }

    case OPTION_DISALLOW_STRING_INSNS:
      elf_flags |= E_FLAG_RX_SINSNS_SET | E_FLAG_RX_SINSNS_NO;
      return 1;
    }

  return 0;
}

void
md_show_usage (FILE * stream)
{
  fprintf (stream, _(" RX specific command line options:\n"));
  fprintf (stream, _("  --mbig-endian-data\n"));
  fprintf (stream, _("  --mlittle-endian-data [default]\n"));
  fprintf (stream, _("  --m32bit-doubles [default]\n"));
  fprintf (stream, _("  --m64bit-doubles\n"));
  fprintf (stream, _("  --muse-conventional-section-names\n"));
  fprintf (stream, _("  --muse-renesas-section-names [default]\n"));
  fprintf (stream, _("  --msmall-data-limit\n"));
  fprintf (stream, _("  --mrelax\n"));
  fprintf (stream, _("  --mpid\n"));
  fprintf (stream, _("  --mint-register=<value>\n"));
  fprintf (stream, _("  --mcpu=<rx100|rx200|rx600|rx610|rxv2|rxv3|rxv3-dfpu>\n"));
  fprintf (stream, _("  --mno-allow-string-insns"));
}

static void
s_bss (int ignore ATTRIBUTE_UNUSED)
{
  int temp;

  temp = get_absolute_expression ();
  subseg_set (bss_section, (subsegT) temp);
  demand_empty_rest_of_line ();
}

static void
rx_float_cons (int ignore ATTRIBUTE_UNUSED)
{
  if (elf_flags & E_FLAG_RX_64BIT_DOUBLES)
    return float_cons ('d');
  return float_cons ('f');
}

static char *
rx_strcasestr (const char *string, const char *sub)
{
  int subl;
  int strl;

  if (!sub || !sub[0])
    return (char *)string;

  subl = strlen (sub);
  strl = strlen (string);

  while (strl >= subl)
    {
      /* strncasecmp is in libiberty.  */
      if (strncasecmp (string, sub, subl) == 0)
	return (char *)string;

      string ++;
      strl --;
    }
  return NULL;
}

static void
rx_include (int ignore)
{
  FILE * try;
  char * path;
  char * filename;
  const char * current_filename;
  char * last_char;
  const char * p;
  const char * d;
  char * f;
  char   end_char;
  size_t len;

  /* The RX version of the .INCLUDE pseudo-op does not
     have to have the filename inside double quotes.  */
  SKIP_WHITESPACE ();
  if (*input_line_pointer == '"')
    {
      /* Treat as the normal GAS .include pseudo-op.  */
      s_include (ignore);
      return;
    }

  /* Get the filename.  Spaces are allowed, NUL characters are not.  */
  filename = input_line_pointer;
  last_char = find_end_of_line (filename, false);
  input_line_pointer = last_char;

  while (last_char >= filename && (* last_char == ' ' || * last_char == '\n'))
    -- last_char;
  end_char = *(++ last_char);
  * last_char = 0;
  if (last_char == filename)
    {
      as_bad (_("no filename following .INCLUDE pseudo-op"));
      * last_char = end_char;
      return;
    }

   current_filename = as_where (NULL);
  f = XNEWVEC (char, strlen (current_filename) + strlen (filename) + 1);

  /* Check the filename.  If [@]..FILE[@] is found then replace
     this with the current assembler source filename, stripped
     of any directory prefixes or extensions.  */
  if ((p = rx_strcasestr (filename, "..file")) != NULL)
    {
      const char * c;

      len = 6; /* strlen ("..file"); */

      if (p > filename && p[-1] == '@')
	-- p, ++len;

      if (p[len] == '@')
	len ++;

      for (d = c = current_filename; *c; c++)
	if (IS_DIR_SEPARATOR (* c))
	  d = c + 1;
      for (c = d; *c; c++)
	if (*c == '.')
	  break;

      sprintf (f, "%.*s%.*s%.*s", (int) (p - filename), filename,
	       (int) (c - d), d,
	       (int) (strlen (filename) - ((p + len) - filename)),
	       p + len);
    }
  else
    strcpy (f, filename);

  /* RX .INCLUDE semantics say that 'filename' is located by:

     1. If filename is absolute, just try that.  Otherwise...

     2. If the current source file includes a directory component
        then prepend that to the filename and try.  Otherwise...

     3. Try any directories specified by the -I command line
        option(s).

     4 .Try a directory specified by the INC100 environment variable.  */

  if (IS_ABSOLUTE_PATH (f))
    try = fopen (path = f, FOPEN_RT);
  else
    {
      char * env = getenv ("INC100");

      try = NULL;

      len = strlen (current_filename);
      if ((size_t) include_dir_maxlen > len)
	len = include_dir_maxlen;
      if (env && strlen (env) > len)
	len = strlen (env);

      path = XNEWVEC (char, strlen (f) + len + 5);

      if (current_filename != NULL)
	{
	  for (d = NULL, p = current_filename; *p; p++)
	    if (IS_DIR_SEPARATOR (* p))
	      d = p;

	  if (d != NULL)
	    {
	      sprintf (path, "%.*s/%s", (int) (d - current_filename), current_filename,
		       f);
	      try = fopen (path, FOPEN_RT);
	    }
	}

      if (try == NULL)
	{
	  for (size_t i = 0; i < include_dir_count; i++)
	    {
	      sprintf (path, "%s/%s", include_dirs[i], f);
	      if ((try = fopen (path, FOPEN_RT)) != NULL)
		break;
	    }
	}

      if (try == NULL && env != NULL)
	{
	  sprintf (path, "%s/%s", env, f);
	  try = fopen (path, FOPEN_RT);
	}

      free (f);
    }

  if (try == NULL)
    {
      as_bad (_("unable to locate include file: %s"), filename);
      free (path);
    }
  else
    {
      fclose (try);
      register_dependency (path);
      input_scrub_insert_file (path);
    }

  * last_char = end_char;
}

static void
parse_rx_section (char * name)
{
  asection * sec;
  int   type;
  int   attr = SHF_ALLOC | SHF_EXECINSTR;
  int   align = 1;
  char  end_char;

  do
    {
      char * p;

      SKIP_WHITESPACE ();
      for (p = input_line_pointer; *p && strchr ("\n\t, =", *p) == NULL; p++)
	;
      end_char = *p;
      *p = 0;

      if (strcasecmp (input_line_pointer, "ALIGN") == 0)
	{
	  *p = end_char;

	  if (end_char == ' ')
	    while (ISSPACE (*p))
	      p++;

	  if (*p == '=')
	    {
	      ++ p;
	      while (ISSPACE (*p))
		p++;
	      switch (*p)
		{
		case '2': align = 1; break;
		case '4': align = 2; break;
		case '8': align = 3; break;
		default:
		  as_bad (_("unrecognised alignment value in .SECTION directive: %s"), p);
		  ignore_rest_of_line ();
		  return;
		}
	      ++ p;
	    }

	  end_char = *p;
	}
      else if (strcasecmp (input_line_pointer, "CODE") == 0)
	attr = SHF_ALLOC | SHF_EXECINSTR;
      else if (strcasecmp (input_line_pointer, "DATA") == 0)
	attr = SHF_ALLOC | SHF_WRITE;
      else if (strcasecmp (input_line_pointer, "ROMDATA") == 0)
	attr = SHF_ALLOC;
      else
	{
	  as_bad (_("unknown parameter following .SECTION directive: %s"),
		  input_line_pointer);

	  *p = end_char;
	  input_line_pointer = p + 1;
	  ignore_rest_of_line ();
	  return;
	}

      *p = end_char;
      input_line_pointer = p + 1;
    }
  while (end_char != '\n' && end_char != 0);

  if ((sec = bfd_get_section_by_name (stdoutput, name)) == NULL)
    {
      if (strcmp (name, "B") && strcmp (name, "B_1") && strcmp (name, "B_2"))
	type = SHT_NULL;
      else
	type = SHT_NOBITS;

      obj_elf_change_section (name, type, attr, 0, NULL, false, false);
    }
  else /* Try not to redefine a section, especially B_1.  */
    {
      int flags = sec->flags;

      type = elf_section_type (sec);

      attr = ((flags & SEC_READONLY) ? 0 : SHF_WRITE)
	| ((flags & SEC_ALLOC) ? SHF_ALLOC : 0)
	| ((flags & SEC_CODE) ? SHF_EXECINSTR : 0)
	| ((flags & SEC_MERGE) ? SHF_MERGE : 0)
	| ((flags & SEC_STRINGS) ? SHF_STRINGS : 0)
	| ((flags & SEC_THREAD_LOCAL) ? SHF_TLS : 0);

      obj_elf_change_section (name, type, attr, 0, NULL, false, false);
    }

  bfd_set_section_alignment (now_seg, align);
}

static void
rx_section (int ignore)
{
  char * p;

  /* The as100 assembler supports a different syntax for the .section
     pseudo-op.  So check for it and handle it here if necessary. */
  SKIP_WHITESPACE ();

  /* Peek past the section name to see if arguments follow.  */
  for (p = input_line_pointer; *p; p++)
    if (*p == ',' || *p == '\n')
      break;

  if (*p == ',')
    {
      int len = p - input_line_pointer;

      while (ISSPACE (*++p))
	;

      if (*p != '"' && *p != '#')
	{
	  char *name = xmemdup0 (input_line_pointer, len);

	  input_line_pointer = p;
	  parse_rx_section (name);
	  return;
	}
    }

  obj_elf_section (ignore);
}

static void
rx_list (int ignore ATTRIBUTE_UNUSED)
{
  SKIP_WHITESPACE ();

  if (strncasecmp (input_line_pointer, "OFF", 3))
    listing_list (0);
  else if (strncasecmp (input_line_pointer, "ON", 2))
    listing_list (1);
  else
    as_warn (_("expecting either ON or OFF after .list"));
}

/* Like the .rept pseudo op, but supports the
   use of ..MACREP inside the repeated region.  */

static void
rx_rept (int ignore ATTRIBUTE_UNUSED)
{
  size_t count = get_absolute_expression ();

  do_repeat (count, "MREPEAT", "ENDR", "..MACREP");
}

/* Like cons() accept that strings are allowed.  */

static void
rx_cons (int size)
{
  SKIP_WHITESPACE ();

  if (* input_line_pointer == '"')
    stringer (8+0);
  else
    cons (size);
}

static void
rx_nop (int ignore ATTRIBUTE_UNUSED)
{
  ignore_rest_of_line ();
}

static void
rx_unimp (int idx)
{
  as_warn (_("The \".%s\" pseudo-op is not implemented\n"),
	   md_pseudo_table[idx].poc_name);
  ignore_rest_of_line ();
}

/* The target specific pseudo-ops which we support.  */
const pseudo_typeS md_pseudo_table[] =
{
  /* These are unimplemented.  They're listed first so that we can use
     the poc_value as the index into this array, to get the name of
     the pseudo.  So, keep these (1) first, and (2) in order, with (3)
     the poc_value's in sequence.  */
  { "btglb",    rx_unimp,       0 },
  { "call",     rx_unimp,       1 },
  { "einsf",    rx_unimp,       2 },
  { "fb",       rx_unimp,       3 },
  { "fbsym",    rx_unimp,       4 },
  { "id",       rx_unimp,       5 },
  { "initsct",  rx_unimp,       6 },
  { "insf",     rx_unimp,       7 },
  { "instr",    rx_unimp,       8 },
  { "lbba",     rx_unimp,       9 },
  { "len",      rx_unimp,       10 },
  { "optj",     rx_unimp,       11 },
  { "rvector",  rx_unimp,       12 },
  { "sb",       rx_unimp,       13 },
  { "sbbit",    rx_unimp,       14 },
  { "sbsym",    rx_unimp,       15 },
  { "sbsym16",  rx_unimp,       16 },

  /* These are the do-nothing pseudos.  */
  { "stk",      rx_nop,         0 },
  /* The manual documents ".stk" but the compiler emits ".stack".  */
  { "stack",    rx_nop,         0 },

  /* These are Renesas as100 assembler pseudo-ops that we do support.  */
  { "addr",     rx_cons,        3 },
  { "align",    s_align_bytes,  2 },
  { "byte",     rx_cons,        1 },
  { "fixed",    float_cons,    'f' },
  { "form",     listing_psize,  0 },
  { "glb",      s_globl,        0 },
  { "include",  rx_include,     0 },
  { "list",     rx_list,        0 },
  { "lword",    rx_cons,        4 },
  { "mrepeat",  rx_rept,        0 },
  { "section",  rx_section,     0 },

  /* FIXME: The following pseudo-ops place their values (and associated
     label if present) in the data section, regardless of whatever
     section we are currently in.  At the moment this code does not
     implement that part of the semantics.  */
  { "blka",     s_space,        3 },
  { "blkb",     s_space,        1 },
  { "blkd",     s_space,        8 },
  { "blkf",     s_space,        4 },
  { "blkl",     s_space,        4 },
  { "blkw",     s_space,        2 },

  /* Our "standard" pseudos. */
  { "double",   rx_float_cons,  0 },
  { "bss",	s_bss, 		0 },
  { "3byte",	cons,		3 },
  { "int",	cons,		4 },
  { "word",	cons,		4 },

  { "fetchalign", rx_fetchalign, 0 },

  /* End of list marker.  */
  { NULL, 	NULL, 		0 }
};

static asymbol * gp_symbol;
static asymbol * rx_pid_symbol;

static symbolS * rx_pidreg_symbol;
static symbolS * rx_gpreg_symbol;

void
md_begin (void)
{
  /* Make the __gp and __pid_base symbols now rather
     than after the symbol table is frozen.  We only do this
     when supporting small data limits because otherwise we
     pollute the symbol table.  */

  /* The meta-registers %pidreg and %gpreg depend on what other
     options are specified.  The __rx_*_defined symbols exist so we
     can .ifdef asm code based on what options were passed to gas,
     without needing a preprocessor  */

  if (rx_pid_mode)
    {
      rx_pid_register = 13 - rx_num_int_regs;
      rx_pid_symbol = symbol_get_bfdsym (symbol_find_or_make ("__pid_base"));
      rx_pidreg_symbol = symbol_find_or_make ("__rx_pidreg_defined");
      S_SET_VALUE (rx_pidreg_symbol, rx_pid_register);
      S_SET_SEGMENT (rx_pidreg_symbol, absolute_section);
    }

  if (rx_use_small_data_limit)
    {
      if (rx_pid_mode)
	rx_gp_register = rx_pid_register - 1;
      else
	rx_gp_register = 13 - rx_num_int_regs;
      gp_symbol = symbol_get_bfdsym (symbol_find_or_make ("__gp"));
      rx_gpreg_symbol = symbol_find_or_make ("__rx_gpreg_defined");
      S_SET_VALUE (rx_gpreg_symbol, rx_gp_register);
      S_SET_SEGMENT (rx_gpreg_symbol, absolute_section);
    }
}

char * rx_lex_start;
char * rx_lex_end;

/* These negative numbers are found in rx_bytesT.n_base for non-opcode
   md_frags */
#define RX_NBASE_FETCHALIGN	-1

typedef struct rx_bytesT
{
  char base[4];
  /* If this is negative, it's a special-purpose frag as per the defines above. */
  int n_base;
  char ops[8];
  int n_ops;
  struct
  {
    expressionS  exp;
    char         offset;
    char         nbits;
    char         type; /* RXREL_*.  */
    int          reloc;
    fixS *       fixP;
  } fixups[2];
  int n_fixups;
  char post[1];
  int n_post;
  struct
  {
    char type;
    char field_pos;
    char val_ofs;
  } relax[2];
  int n_relax;
  int link_relax;
  fixS *link_relax_fixP;
  unsigned long times_grown;
  unsigned long times_shrank;
} rx_bytesT;

static rx_bytesT rx_bytes;
/* We set n_ops to be "size of next opcode" if the next opcode doesn't relax.  */
static rx_bytesT *fetchalign_bytes = NULL;

static void
rx_fetchalign (int ignore ATTRIBUTE_UNUSED)
{
  char * bytes;
  fragS * frag_then;

  memset (& rx_bytes, 0, sizeof (rx_bytes));
  rx_bytes.n_base = RX_NBASE_FETCHALIGN;

  bytes = frag_more (8);
  frag_then = frag_now;
  frag_variant (rs_machine_dependent,
		0 /* max_chars */,
		0 /* var */,
		0 /* subtype */,
		0 /* symbol */,
		0 /* offset */,
		0 /* opcode */);
  frag_then->fr_opcode = bytes;
  frag_then->fr_subtype = 0;
  fetchalign_bytes = frag_then->tc_frag_data;
}

void
rx_relax (int type, int pos)
{
  rx_bytes.relax[rx_bytes.n_relax].type = type;
  rx_bytes.relax[rx_bytes.n_relax].field_pos = pos;
  rx_bytes.relax[rx_bytes.n_relax].val_ofs = rx_bytes.n_base + rx_bytes.n_ops;
  rx_bytes.n_relax ++;
}

void
rx_linkrelax_dsp (int pos)
{
  switch (pos)
    {
    case 4:
      rx_bytes.link_relax |= RX_RELAXA_DSP4;
      break;
    case 6:
      rx_bytes.link_relax |= RX_RELAXA_DSP6;
      break;
    case 14:
      rx_bytes.link_relax |= RX_RELAXA_DSP14;
      break;
    }
}

void
rx_linkrelax_imm (int pos)
{
  switch (pos)
    {
    case 6:
      rx_bytes.link_relax |= RX_RELAXA_IMM6;
      break;
    case 12:
      rx_bytes.link_relax |= RX_RELAXA_IMM12;
      break;
    }
}

void
rx_linkrelax_branch (void)
{
  rx_bytes.link_relax |= RX_RELAXA_BRA;
}

static void
rx_fixup (expressionS exp, int offsetbits, int nbits, int type)
{
  rx_bytes.fixups[rx_bytes.n_fixups].exp = exp;
  rx_bytes.fixups[rx_bytes.n_fixups].offset = offsetbits;
  rx_bytes.fixups[rx_bytes.n_fixups].nbits = nbits;
  rx_bytes.fixups[rx_bytes.n_fixups].type = type;
  rx_bytes.fixups[rx_bytes.n_fixups].reloc = exp.X_md;
  rx_bytes.n_fixups ++;
}

#define rx_field_fixup(exp, offset, nbits, type)	\
  rx_fixup (exp, offset, nbits, type)

#define rx_op_fixup(exp, offset, nbits, type)		\
  rx_fixup (exp, offset + 8 * rx_bytes.n_base, nbits, type)

void
rx_base1 (int b1)
{
  rx_bytes.base[0] = b1;
  rx_bytes.n_base = 1;
}

void
rx_base2 (int b1, int b2)
{
  rx_bytes.base[0] = b1;
  rx_bytes.base[1] = b2;
  rx_bytes.n_base = 2;
}

void
rx_base3 (int b1, int b2, int b3)
{
  rx_bytes.base[0] = b1;
  rx_bytes.base[1] = b2;
  rx_bytes.base[2] = b3;
  rx_bytes.n_base = 3;
}

void
rx_base4 (int b1, int b2, int b3, int b4)
{
  rx_bytes.base[0] = b1;
  rx_bytes.base[1] = b2;
  rx_bytes.base[2] = b3;
  rx_bytes.base[3] = b4;
  rx_bytes.n_base = 4;
}

/* This gets complicated when the field spans bytes, because fields
   are numbered from the MSB of the first byte as zero, and bits are
   stored LSB towards the LSB of the byte.  Thus, a simple four-bit
   insertion of 12 at position 4 of 0x00 yields: 0x0b.  A three-bit
   insertion of b'MXL at position 7 is like this:

     - - - -  - - - -   - - - -  - - - -
                    M   X L               */

void
rx_field (int val, int pos, int sz)
{
  int valm;
  int bytep, bitp;

  if (sz > 0)
    {
      if (val < 0 || val >= (1 << sz))
	as_bad (_("Value %d doesn't fit in unsigned %d-bit field"), val, sz);
    }
  else
    {
      sz = - sz;
      if (val < -(1 << (sz - 1)) || val >= (1 << (sz - 1)))
	as_bad (_("Value %d doesn't fit in signed %d-bit field"), val, sz);
    }

  /* This code points at 'M' in the above example.  */
  bytep = pos / 8;
  bitp = pos % 8;

  while (bitp + sz > 8)
    {
      int ssz = 8 - bitp;
      int svalm;

      svalm = val >> (sz - ssz);
      svalm = svalm & ((1 << ssz) - 1);
      svalm = svalm << (8 - bitp - ssz);
      gas_assert (bytep < rx_bytes.n_base);
      rx_bytes.base[bytep] |= svalm;

      bitp = 0;
      sz -= ssz;
      bytep ++;
    }
  valm = val & ((1 << sz) - 1);
  valm = valm << (8 - bitp - sz);
  gas_assert (bytep < rx_bytes.n_base);
  rx_bytes.base[bytep] |= valm;
}

/* Special case of the above, for 3-bit displacements of 2..9.  */

void
rx_disp3 (expressionS exp, int pos)
{
  rx_field_fixup (exp, pos, 3, RXREL_PCREL);
}

/* Special case of the above, for split 5-bit displacements.  Assumes
   the displacement has been checked with rx_disp5op.  */
/* ---- -432 1--- 0--- */

void
rx_field5s (expressionS exp)
{
  int val;

  val = exp.X_add_number;
  rx_bytes.base[0] |= val >> 2;
  rx_bytes.base[1] |= (val << 6) & 0x80;
  rx_bytes.base[1] |= (val << 3) & 0x08;
}

/* ---- ---- 4--- 3210 */

void
rx_field5s2 (expressionS exp)
{
  int val;

  val = exp.X_add_number;
  rx_bytes.base[1] |= (val << 3) & 0x80;
  rx_bytes.base[1] |= (val     ) & 0x0f;
}

void
rx_bfield(expressionS s, expressionS d, expressionS w)
{
  int slsb = s.X_add_number;
  int dlsb = d.X_add_number;
  int width = w.X_add_number;
  unsigned int imm =
    (((dlsb + width) & 0x1f) << 10 | (dlsb << 5) |
     ((dlsb - slsb) & 0x1f));
  if ((slsb + width) > 32)
        as_warn (_("Value %d and %d out of range"), slsb, width);
  if ((dlsb + width) > 32)
        as_warn (_("Value %d and %d out of range"), dlsb, width);
  rx_bytes.ops[0] = imm & 0xff;
  rx_bytes.ops[1] = (imm >> 8);
  rx_bytes.n_ops = 2;
}

#define OP(x) rx_bytes.ops[rx_bytes.n_ops++] = (x)

#define F_PRECISION 2

void
rx_op (expressionS exp, int nbytes, int type)
{
  offsetT v = 0;

  if ((exp.X_op == O_constant || exp.X_op == O_big)
      && type != RXREL_PCREL)
    {
      if (exp.X_op == O_big)
	{
	  if (exp.X_add_number == -1)
	    {
	      LITTLENUM_TYPE w[2];
	      char * ip = rx_bytes.ops + rx_bytes.n_ops;

	      gen_to_words (w, F_PRECISION, 8);
#if RX_OPCODE_BIG_ENDIAN
	      ip[0] = w[0] >> 8;
	      ip[1] = w[0];
	      ip[2] = w[1] >> 8;
	      ip[3] = w[1];
#else
	      ip[3] = w[0] >> 8;
	      ip[2] = w[0];
	      ip[1] = w[1] >> 8;
	      ip[0] = w[1];
#endif
	      rx_bytes.n_ops += 4;
	      return;
	    }

	  v = ((generic_bignum[1] & LITTLENUM_MASK) << LITTLENUM_NUMBER_OF_BITS)
	    |  (generic_bignum[0] & LITTLENUM_MASK);

	}
      else
	v = exp.X_add_number;

      while (nbytes)
	{
#if RX_OPCODE_BIG_ENDIAN
	  OP ((v >> (8 * (nbytes - 1))) & 0xff);
#else
	  OP (v & 0xff);
	  v >>= 8;
#endif
	  nbytes --;
	}
    }
  else
    {
      rx_op_fixup (exp, rx_bytes.n_ops * 8, nbytes * 8, type);
      memset (rx_bytes.ops + rx_bytes.n_ops, 0, nbytes);
      rx_bytes.n_ops += nbytes;
    }
}

void rx_post(char byte)
{
  rx_bytes.post[rx_bytes.n_post++] = byte;
}

int
rx_wrap (void)
{
  return 0;
}

#define APPEND(B, N_B)				       \
  if (rx_bytes.N_B)				       \
    {						       \
      memcpy (bytes + idx, rx_bytes.B, rx_bytes.N_B);  \
      idx += rx_bytes.N_B;			       \
    }

void
rx_frag_init (fragS * fragP)
{
  if (rx_bytes.n_relax || rx_bytes.link_relax || rx_bytes.n_base < 0)
    {
      fragP->tc_frag_data = XNEW (rx_bytesT);
      memcpy (fragP->tc_frag_data, & rx_bytes, sizeof (rx_bytesT));
    }
  else
    fragP->tc_frag_data = 0;
}

/* Handle the as100's version of the .equ pseudo-op.  It has the syntax:
   <symbol_name> .equ <expression>   */

static void
rx_equ (char * name, char * expression)
{
  char   saved_name_end_char;
  char * name_end;
  char * saved_ilp;

  while (ISSPACE (* name))
    name ++;

  for (name_end = name + 1; *name_end; name_end ++)
    if (! ISALNUM (* name_end))
      break;

  saved_name_end_char = * name_end;
  * name_end = 0;

  saved_ilp = input_line_pointer;
  input_line_pointer = expression;

  equals (name, 1);

  input_line_pointer = saved_ilp;
  * name_end = saved_name_end_char;
}

/* Look for Renesas as100 pseudo-ops that occur after a symbol name
   rather than at the start of a line.  (eg .EQU or .DEFINE).  If one
   is found, process it and return TRUE otherwise return FALSE.  */

static bool
scan_for_infix_rx_pseudo_ops (char * str)
{
  char * p;
  char * pseudo_op;
  char * dot = strchr (str, '.');

  if (dot == NULL || dot == str)
    return false;

  /* A real pseudo-op must be preceded by whitespace.  */
  if (dot[-1] != ' ' && dot[-1] != '\t')
    return false;

  pseudo_op = dot + 1;

  if (!ISALNUM (* pseudo_op))
    return false;

  for (p = pseudo_op + 1; ISALNUM (* p); p++)
    ;

  if (strncasecmp ("EQU", pseudo_op, p - pseudo_op) == 0)
    rx_equ (str, p);
  else if (strncasecmp ("DEFINE", pseudo_op, p - pseudo_op) == 0)
    as_warn (_("The .DEFINE pseudo-op is not implemented"));
  else if (strncasecmp ("MACRO", pseudo_op, p - pseudo_op) == 0)
    as_warn (_("The .MACRO pseudo-op is not implemented"));
  else if (strncasecmp ("BTEQU", pseudo_op, p - pseudo_op) == 0)
    as_warn (_("The .BTEQU pseudo-op is not implemented."));
  else
    return false;

  return true;
}

void
md_assemble (char * str)
{
  char * bytes;
  int idx = 0;
  int i, rel;
  fragS * frag_then = frag_now;
  expressionS  *exp;

  memset (& rx_bytes, 0, sizeof (rx_bytes));

  rx_lex_init (str, str + strlen (str));
  if (scan_for_infix_rx_pseudo_ops (str))
    return;
  rx_parse ();

  /* This simplifies the relaxation code.  */
  if (rx_bytes.n_relax || rx_bytes.link_relax)
    {
      /* We do it this way because we want the frag to have the
	 rx_bytes in it, which we initialize above.  */
      bytes = frag_more (12);
      frag_then = frag_now;
      frag_variant (rs_machine_dependent,
		    0 /* max_chars */,
		    0 /* var */,
		    0 /* subtype */,
		    0 /* symbol */,
		    0 /* offset */,
		    0 /* opcode */);
      frag_then->fr_opcode = bytes;
      frag_then->fr_fix += rx_bytes.n_base + rx_bytes.n_ops + rx_bytes.n_post;
      frag_then->fr_subtype = rx_bytes.n_base + rx_bytes.n_ops + rx_bytes.n_post;
    }
  else
    {
      bytes = frag_more (rx_bytes.n_base + rx_bytes.n_ops + rx_bytes.n_post);
      frag_then = frag_now;
      if (fetchalign_bytes)
	fetchalign_bytes->n_ops = rx_bytes.n_base + rx_bytes.n_ops + rx_bytes.n_post;
    }

  fetchalign_bytes = NULL;

  APPEND (base, n_base);
  APPEND (ops, n_ops);
  APPEND (post, n_post);

  if (rx_bytes.link_relax && rx_bytes.n_fixups)
    {
      fixS * f;

      f = fix_new (frag_then,
		   (char *) bytes - frag_then->fr_literal,
		   0,
		   abs_section_sym,
		   rx_bytes.link_relax | rx_bytes.n_fixups,
		   0,
		   BFD_RELOC_RX_RELAX);
      frag_then->tc_frag_data->link_relax_fixP = f;
    }

  for (i = 0; i < rx_bytes.n_fixups; i ++)
    {
      /* index: [nbytes][type] */
      static int reloc_map[5][4] =
	{
	  { 0,                  0,                0,                  BFD_RELOC_RX_DIR3U_PCREL },
	  { BFD_RELOC_8,        BFD_RELOC_RX_8U,  BFD_RELOC_RX_NEG8,  BFD_RELOC_8_PCREL },
	  { BFD_RELOC_RX_16_OP, BFD_RELOC_RX_16U, BFD_RELOC_RX_NEG16, BFD_RELOC_16_PCREL },
	  { BFD_RELOC_RX_24_OP, BFD_RELOC_RX_24U, BFD_RELOC_RX_NEG24, BFD_RELOC_24_PCREL },
	  { BFD_RELOC_RX_32_OP, BFD_RELOC_32,     BFD_RELOC_RX_NEG32, BFD_RELOC_32_PCREL },
	};
      fixS * f;

      idx = rx_bytes.fixups[i].offset / 8;
      rel = reloc_map [rx_bytes.fixups[i].nbits / 8][(int) rx_bytes.fixups[i].type];

      if (rx_bytes.fixups[i].reloc)
	rel = rx_bytes.fixups[i].reloc;

      if (frag_then->tc_frag_data)
	exp = & frag_then->tc_frag_data->fixups[i].exp;
      else
	exp = & rx_bytes.fixups[i].exp;

      f = fix_new_exp (frag_then,
		       (char *) bytes + idx - frag_then->fr_literal,
		       rx_bytes.fixups[i].nbits / 8,
		       exp,
		       rx_bytes.fixups[i].type == RXREL_PCREL ? 1 : 0,
		       rel);
      if (frag_then->tc_frag_data)
	frag_then->tc_frag_data->fixups[i].fixP = f;
    }
  dwarf2_emit_insn (idx);
}

void
rx_md_end (void)
{
}

/* Write a value out to the object file, using the appropriate endianness.  */

void
md_number_to_chars (char * buf, valueT val, int n)
{
  if (target_big_endian)
    number_to_chars_bigendian (buf, val, n);
  else
    number_to_chars_littleendian (buf, val, n);
}

static struct
{
  const char * fname;
  int    reloc;
}
reloc_functions[] =
{
  { "gp", BFD_RELOC_GPREL16 },
  { 0, 0 }
};

void
md_operand (expressionS * exp ATTRIBUTE_UNUSED)
{
  int reloc = 0;
  int i;

  for (i = 0; reloc_functions[i].fname; i++)
    {
      int flen = strlen (reloc_functions[i].fname);

      if (input_line_pointer[0] == '%'
	  && strncasecmp (input_line_pointer + 1, reloc_functions[i].fname, flen) == 0
	  && input_line_pointer[flen + 1] == '(')
	{
	  reloc = reloc_functions[i].reloc;
	  input_line_pointer += flen + 2;
	  break;
	}
    }
  if (reloc == 0)
    return;

  expression (exp);
  if (* input_line_pointer == ')')
    input_line_pointer ++;

  exp->X_md = reloc;
}

valueT
md_section_align (segT segment, valueT size)
{
  int align = bfd_section_alignment (segment);
  return ((size + (1 << align) - 1) & -(1 << align));
}

				/* NOP - 1 cycle */
static unsigned char nop_1[] = { 0x03};
				/* MOV.L R0,R0 - 1 cycle */
static unsigned char nop_2[] = { 0xef, 0x00};
				/* MAX R0,R0 - 1 cycle */
static unsigned char nop_3[] = { 0xfc, 0x13, 0x00 };
				/* MUL #1,R0 - 1 cycle */
static unsigned char nop_4[] = { 0x76, 0x10, 0x01, 0x00 };
				/* MUL #1,R0 - 1 cycle */
static unsigned char nop_5[] = { 0x77, 0x10, 0x01, 0x00, 0x00 };
				/* MUL #1,R0 - 1 cycle */
static unsigned char nop_6[] = { 0x74, 0x10, 0x01, 0x00, 0x00, 0x00 };
				/* MAX 0x80000000,R0 - 1 cycle */
static unsigned char nop_7[] = { 0xFD, 0x70, 0x40, 0x00, 0x00, 0x00, 0x80 };

static unsigned char *nops[] = { NULL, nop_1, nop_2, nop_3, nop_4, nop_5, nop_6, nop_7 };
#define BIGGEST_NOP 7

/* When relaxing, we need to output a reloc for any .align directive
   so that we can retain this alignment as we adjust opcode sizes.  */
void
rx_handle_align (fragS * frag)
{
  /* If handling an alignment frag, use an optimal NOP pattern.
     Only do this if a fill value has not already been provided.
     FIXME: This test fails if the provided fill value is zero.  */
  if ((frag->fr_type == rs_align
       || frag->fr_type == rs_align_code)
      && subseg_text_p (now_seg))
    {
      int count = (frag->fr_next->fr_address
		   - frag->fr_address
		   - frag->fr_fix);
      unsigned char *base = (unsigned char *)frag->fr_literal + frag->fr_fix;

      if (* base == 0)
	{
	  if (count > BIGGEST_NOP)
	    {
	      base[0] = 0x2e;
	      base[1] = count;
	      frag->fr_var = 2;
	    }
	  else if (count > 0)
	    {
	      memcpy (base, nops[count], count);
	      frag->fr_var = count;
	    }
	}
    }

  if (linkrelax
      && (frag->fr_type == rs_align
	  || frag->fr_type == rs_align_code)
      && frag->fr_address + frag->fr_fix > 0
      && frag->fr_offset > 0
      && now_seg != bss_section)
    {
      fix_new (frag, frag->fr_fix, 0,
	       &abs_symbol, RX_RELAXA_ALIGN + frag->fr_offset,
	       0, BFD_RELOC_RX_RELAX);
      /* For the purposes of relaxation, this relocation is attached
	 to the byte *after* the alignment - i.e. the byte that must
	 remain aligned.  */
      fix_new (frag->fr_next, 0, 0,
	       &abs_symbol, RX_RELAXA_ELIGN + frag->fr_offset,
	       0, BFD_RELOC_RX_RELAX);
    }
}

const char *
md_atof (int type, char * litP, int * sizeP)
{
  return ieee_md_atof (type, litP, sizeP, target_big_endian);
}

symbolS *
md_undefined_symbol (char * name ATTRIBUTE_UNUSED)
{
  return NULL;
}

/*----------------------------------------------------------------------*/
/* To recap: we estimate everything based on md_estimate_size, then
   adjust based on rx_relax_frag.  When it all settles, we call
   md_convert frag to update the bytes.  The relaxation types and
   relocations are in fragP->tc_frag_data, which is a copy of that
   rx_bytes.

   Our scheme is as follows: fr_fix has the size of the smallest
   opcode (like BRA.S).  We store the number of total bytes we need in
   fr_subtype.  When we're done relaxing, we use fr_subtype and the
   existing opcode bytes to figure out what actual opcode we need to
   put in there.  If the fixup isn't resolvable now, we use the
   maximal size.  */

#define TRACE_RELAX 0
#define tprintf if (TRACE_RELAX) printf

typedef enum
{
  OT_other,
  OT_bra,
  OT_beq,
  OT_bne,
  OT_bsr,
  OT_bcc
} op_type_T;

/* We're looking for these types of relaxations:

   BRA.S	00001dsp
   BRA.B	00101110 dspppppp
   BRA.W	00111000 dspppppp pppppppp
   BRA.A	00000100 dspppppp pppppppp pppppppp

   BEQ.S	00010dsp
   BEQ.B	00100000 dspppppp
   BEQ.W	00111010 dspppppp pppppppp

   BNE.S	00011dsp
   BNE.B	00100001 dspppppp
   BNE.W	00111011 dspppppp pppppppp

   BSR.W	00111001 dspppppp pppppppp
   BSR.A	00000101 dspppppp pppppppp pppppppp

   Bcc.B	0010cond dspppppp

   Additionally, we can synthesize longer conditional branches using
   pairs of opcodes, one with an inverted conditional (flip LSB):

   Bcc.W	0010ncnd 00000110 00111000 dspppppp pppppppp
   Bcc.A	0010ncnd 00000111 00000100 dspppppp pppppppp pppppppp
   BEQ.A	00011100 00000100 dspppppp pppppppp pppppppp
   BNE.A	00010100 00000100 dspppppp pppppppp pppppppp  */

/* Given the opcode bytes at OP, figure out which opcode it is and
   return the type of opcode.  We use this to re-encode the opcode as
   a different size later.  */

static op_type_T
rx_opcode_type (char * op)
{
  unsigned char b = (unsigned char) op[0];

  switch (b & 0xf8)
    {
    case 0x08: return OT_bra;
    case 0x10: return OT_beq;
    case 0x18: return OT_bne;
    }

  switch (b)
    {
    case 0x2e: return OT_bra;
    case 0x38: return OT_bra;
    case 0x04: return OT_bra;

    case 0x20: return OT_beq;
    case 0x3a: return OT_beq;

    case 0x21: return OT_bne;
    case 0x3b: return OT_bne;

    case 0x39: return OT_bsr;
    case 0x05: return OT_bsr;
    }

  if ((b & 0xf0) == 0x20)
    return OT_bcc;

  return OT_other;
}

/* Returns zero if *addrP has the target address.  Else returns nonzero
   if we cannot compute the target address yet.  */

static int
rx_frag_fix_value (fragS *    fragP,
		   segT       segment,
		   int        which,
		   addressT * addrP,
		   int        need_diff,
		   addressT * sym_addr)
{
  addressT addr = 0;
  rx_bytesT * b = fragP->tc_frag_data;
  expressionS * exp = & b->fixups[which].exp;

  if (need_diff && exp->X_op != O_subtract)
    return 1;

  if (exp->X_add_symbol)
    {
      if (S_FORCE_RELOC (exp->X_add_symbol, 1))
	return 1;
      if (S_GET_SEGMENT (exp->X_add_symbol) != segment)
	return 1;
      addr += S_GET_VALUE (exp->X_add_symbol);
    }

  if (exp->X_op_symbol)
    {
      if (exp->X_op != O_subtract)
	return 1;
      if (S_FORCE_RELOC (exp->X_op_symbol, 1))
	return 1;
      if (S_GET_SEGMENT (exp->X_op_symbol) != segment)
	return 1;
      addr -= S_GET_VALUE (exp->X_op_symbol);
    }
  if (sym_addr)
    * sym_addr = addr;
  addr += exp->X_add_number;
  * addrP = addr;
  return 0;
}

/* Estimate how big the opcode is after this relax pass.  The return
   value is the difference between fr_fix and the actual size.  We
   compute the total size in rx_relax_frag and store it in fr_subtype,
   so we only need to subtract fx_fix and return it.  */

int
md_estimate_size_before_relax (fragS * fragP ATTRIBUTE_UNUSED, segT segment ATTRIBUTE_UNUSED)
{
  int opfixsize;
  int delta;

  tprintf ("\033[32m  est frag: addr %08lx fix %ld var %ld ofs %ld lit %p opc %p type %d sub %d\033[0m\n",
	   (unsigned long) (fragP->fr_address
			    + (fragP->fr_opcode - fragP->fr_literal)),
	   (long) fragP->fr_fix, (long) fragP->fr_var, (long) fragP->fr_offset,
	   fragP->fr_literal, fragP->fr_opcode, fragP->fr_type, fragP->fr_subtype);

  /* This is the size of the opcode that's accounted for in fr_fix.  */
  opfixsize = fragP->fr_fix - (fragP->fr_opcode - fragP->fr_literal);
  /* This is the size of the opcode that isn't.  */
  delta = (fragP->fr_subtype - opfixsize);

  tprintf (" -> opfixsize %d delta %d\n", opfixsize, delta);
  return delta;
}

/* Given a frag FRAGP, return the "next" frag that contains an
   opcode.  Assumes the next opcode is relaxable, and thus rs_machine_dependent.  */

static fragS *
rx_next_opcode (fragS *fragP)
{
  do {
    fragP = fragP->fr_next;
  } while (fragP && fragP->fr_type != rs_machine_dependent);
  return fragP;
}

/* Given the new addresses for this relax pass, figure out how big
   each opcode must be.  We store the total number of bytes needed in
   fr_subtype.  The return value is the difference between the size
   after the last pass and the size after this pass, so we use the old
   fr_subtype to calculate the difference.  */

int
rx_relax_frag (segT segment ATTRIBUTE_UNUSED, fragS * fragP, long stretch, unsigned long max_iterations)
{
  addressT addr0, sym_addr;
  addressT mypc;
  int disp;
  int oldsize = fragP->fr_subtype;
  int newsize = oldsize;
  op_type_T optype;
   /* Index of relaxation we care about.  */
  int ri;

  tprintf ("\033[36mrelax frag: addr %08lx fix %ld var %ld ofs %ld lit %p opc %p type %d sub %d str %ld\033[0m\n",
	   (unsigned long) (fragP->fr_address
			    + (fragP->fr_opcode - fragP->fr_literal)),
	   (long) fragP->fr_fix, (long) fragP->fr_var, (long) fragP->fr_offset,
	   fragP->fr_literal, fragP->fr_opcode, fragP->fr_type, fragP->fr_subtype, stretch);

  mypc = fragP->fr_address + (fragP->fr_opcode - fragP->fr_literal);

  if (fragP->tc_frag_data->n_base == RX_NBASE_FETCHALIGN)
    {
      unsigned int next_size;
      if (fragP->fr_next == NULL)
	return 0;

      next_size = fragP->tc_frag_data->n_ops;
      if (next_size == 0)
	{
	  fragS *n = rx_next_opcode (fragP);
	  next_size = n->fr_subtype;
	}

      fragP->fr_subtype = (8-(mypc & 7)) & 7;
      tprintf("subtype %u\n", fragP->fr_subtype);
      if (fragP->fr_subtype >= next_size)
	fragP->fr_subtype = 0;
      tprintf ("\033[34m -> mypc %lu next_size %u new %d old %d delta %d (fetchalign)\033[0m\n",
	       (unsigned long) (mypc & 7),
	       next_size, fragP->fr_subtype, oldsize, fragP->fr_subtype-oldsize);

      newsize = fragP->fr_subtype;

      return newsize - oldsize;
    }

  optype = rx_opcode_type (fragP->fr_opcode);

  /* In the one case where we have both a disp and imm relaxation, we want
     the imm relaxation here.  */
  ri = 0;
  if (fragP->tc_frag_data->n_relax > 1
      && fragP->tc_frag_data->relax[0].type == RX_RELAX_DISP)
    ri = 1;

  /* Try to get the target address.  */
  if (rx_frag_fix_value (fragP, segment, ri, & addr0,
			 fragP->tc_frag_data->relax[ri].type != RX_RELAX_BRANCH,
			 & sym_addr))
    {
      /* If we don't, we must use the maximum size for the linker.
         Note that we don't use synthetically expanded conditionals
         for this.  */
      switch (fragP->tc_frag_data->relax[ri].type)
	{
	case RX_RELAX_BRANCH:
	  switch (optype)
	    {
	    case OT_bra:
	    case OT_bsr:
	      newsize = 4;
	      break;
	    case OT_beq:
	    case OT_bne:
	      newsize = 3;
	      break;
	    case OT_bcc:
	      newsize = 2;
	      break;
	    case OT_other:
	      newsize = oldsize;
	      break;
	    }
	  break;

	case RX_RELAX_IMM:
	  newsize = fragP->tc_frag_data->relax[ri].val_ofs + 4;
	  break;
	}
      fragP->fr_subtype = newsize;
      tprintf (" -> new %d old %d delta %d (external)\n", newsize, oldsize, newsize-oldsize);
      return newsize - oldsize;
    }

  if (sym_addr > mypc)
    addr0 += stretch;

  switch (fragP->tc_frag_data->relax[ri].type)
    {
    case  RX_RELAX_BRANCH:
      tprintf ("branch, addr %08lx pc %08lx disp %ld\n",
	       (unsigned long) addr0, (unsigned long) mypc,
	       (long) (addr0 - mypc));
      disp = (int) addr0 - (int) mypc;

      switch (optype)
	{
	case OT_bcc:
	  if (disp >= -128 && (disp - (oldsize-2)) <= 127)
	    /* bcc.b */
	    newsize = 2;
	  else if (disp >= -32768 && (disp - (oldsize-5)) <= 32767)
	    /* bncc.b/bra.w */
	    newsize = 5;
	  else
	    /* bncc.b/bra.a */
	    newsize = 6;
	  break;

	case OT_beq:
	case OT_bne:
	  if ((disp - (oldsize-1)) >= 3 && (disp - (oldsize-1)) <= 10 && !linkrelax)
	    /* beq.s */
	    newsize = 1;
	  else if (disp >= -128 && (disp - (oldsize-2)) <= 127)
	    /* beq.b */
	    newsize = 2;
	  else if (disp >= -32768 && (disp - (oldsize-3)) <= 32767)
	    /* beq.w */
	    newsize = 3;
	  else
	    /* bne.s/bra.a */
	    newsize = 5;
	  break;

	case OT_bra:
	case OT_bsr:
	  if ((disp - (oldsize-1)) >= 3 && (disp - (oldsize-1)) <= 10 && !linkrelax)
	    /* bra.s */
	    newsize = 1;
	  else if (disp >= -128 && (disp - (oldsize-2)) <= 127)
	    /* bra.b */
	    newsize = 2;
	  else if (disp >= -32768 && (disp - (oldsize-3)) <= 32767)
	    /* bra.w */
	    newsize = 3;
	  else
	    /* bra.a */
	    newsize = 4;
	  break;

	case OT_other:
	  break;
	}
      tprintf (" - newsize %d\n", newsize);
      break;

    case RX_RELAX_IMM:
      tprintf ("other, addr %08lx pc %08lx LI %d OF %d\n",
	       (unsigned long) addr0, (unsigned long) mypc,
	       fragP->tc_frag_data->relax[ri].field_pos,
	       fragP->tc_frag_data->relax[ri].val_ofs);

      newsize = fragP->tc_frag_data->relax[ri].val_ofs;

      if ((long) addr0 >= -128 && (long) addr0 <= 127)
	newsize += 1;
      else if ((long) addr0 >= -32768 && (long) addr0 <= 32767)
	newsize += 2;
      else if ((long) addr0 >= -8388608 && (long) addr0 <= 8388607)
	newsize += 3;
      else
	newsize += 4;
      break;

    default:
      break;
    }

  if (fragP->tc_frag_data->relax[ri].type == RX_RELAX_BRANCH)
    switch (optype)
      {
      case OT_bra:
      case OT_bcc:
      case OT_beq:
      case OT_bne:
	break;
      case OT_bsr:
	if (newsize < 3)
	  newsize = 3;
	break;
      case OT_other:
	break;
      }

  /* This prevents infinite loops in align-heavy sources.  */
  if (newsize < oldsize)
    {
      /* Make sure that our iteration limit is no bigger than the one being
	 used inside write.c:relax_segment().  Otherwise we can end up
	 iterating for too long, and triggering a fatal error there.  See
	 PR 24464 for more details.  */
      unsigned long limit = max_iterations > 10 ? 10 : max_iterations;

      if (fragP->tc_frag_data->times_shrank > limit
	  && fragP->tc_frag_data->times_grown > limit)
	newsize = oldsize;

      if (fragP->tc_frag_data->times_shrank < 20)
       fragP->tc_frag_data->times_shrank ++;
    }
  else if (newsize > oldsize)
    {
      if (fragP->tc_frag_data->times_grown < 20)
       fragP->tc_frag_data->times_grown ++;
    }

  fragP->fr_subtype = newsize;
  tprintf (" -> new %d old %d delta %d\n", newsize, oldsize, newsize-oldsize);
  return newsize - oldsize;
}

/* This lets us test for the opcode type and the desired size in a
   switch statement.  */
#define OPCODE(type,size) ((type) * 16 + (size))

/* Given the opcode stored in fr_opcode and the number of bytes we
   think we need, encode a new opcode.  We stored a pointer to the
   fixup for this opcode in the tc_frag_data structure.  If we can do
   the fixup here, we change the relocation type to "none" (we test
   for that in tc_gen_reloc) else we change it to the right type for
   the new (biggest) opcode.  */

void
md_convert_frag (bfd *   abfd ATTRIBUTE_UNUSED,
		 segT    segment ATTRIBUTE_UNUSED,
		 fragS * fragP ATTRIBUTE_UNUSED)
{
  rx_bytesT * rxb = fragP->tc_frag_data;
  addressT addr0, mypc;
  int disp;
  int reloc_adjust;
  bfd_reloc_code_real_type reloc_type;
  char * op = fragP->fr_opcode;
  int keep_reloc = 0;
  int ri;
  int fi = (rxb->n_fixups > 1) ? 1 : 0;
  fixS * fix = rxb->fixups[fi].fixP;

  tprintf ("\033[31mconvrt frag: addr %08lx fix %ld var %ld ofs %ld lit %p opc %p type %d sub %d\033[0m\n",
	   (unsigned long) (fragP->fr_address
			    + (fragP->fr_opcode - fragP->fr_literal)),
	   (long) fragP->fr_fix, (long) fragP->fr_var, (long) fragP->fr_offset,
	   fragP->fr_literal, fragP->fr_opcode, fragP->fr_type,
	   fragP->fr_subtype);

#if TRACE_RELAX
  {
    int i;

    printf ("lit 0x%p opc 0x%p", fragP->fr_literal, fragP->fr_opcode);
    for (i = 0; i < 10; i++)
      printf (" %02x", (unsigned char) (fragP->fr_opcode[i]));
    printf ("\n");
  }
#endif

  if (fragP->tc_frag_data->n_base == RX_NBASE_FETCHALIGN)
    {
      int count = fragP->fr_subtype;
      if (count == 0)
	;
      else if (count > BIGGEST_NOP)
	{
	  op[0] = 0x2e;
	  op[1] = count;
	}
      else if (count > 0)
	{
	  memcpy (op, nops[count], count);
	}
    }

  /* In the one case where we have both a disp and imm relaxation, we want
     the imm relaxation here.  */
  ri = 0;
  if (fragP->tc_frag_data->n_relax > 1
      && fragP->tc_frag_data->relax[0].type == RX_RELAX_DISP)
    ri = 1;

  /* We used a new frag for this opcode, so the opcode address should
     be the frag address.  */
  mypc = fragP->fr_address + (fragP->fr_opcode - fragP->fr_literal);

  /* Try to get the target address.  If we fail here, we just use the
     largest format.  */
  if (rx_frag_fix_value (fragP, segment, 0, & addr0,
			 fragP->tc_frag_data->relax[ri].type != RX_RELAX_BRANCH, 0))
    {
      /* We don't know the target address.  */
      keep_reloc = 1;
      addr0 = 0;
      disp = 0;
    }
  else
    {
      /* We know the target address, and it's in addr0.  */
      disp = (int) addr0 - (int) mypc;
    }

  if (linkrelax)
    keep_reloc = 1;

  reloc_type = BFD_RELOC_NONE;
  reloc_adjust = 0;

  tprintf ("convert, op is %d, disp %d (%lx-%lx)\n",
	   rx_opcode_type (fragP->fr_opcode), disp,
	   (unsigned long) addr0, (unsigned long) mypc);
  switch (fragP->tc_frag_data->relax[ri].type)
    {
    case RX_RELAX_BRANCH:
      switch (OPCODE (rx_opcode_type (fragP->fr_opcode), fragP->fr_subtype))
	{
	case OPCODE (OT_bra, 1): /* BRA.S - no change.  */
	  op[0] = 0x08 + (disp & 7);
	  break;
	case OPCODE (OT_bra, 2): /* BRA.B - 8 bit.  */
	  op[0] = 0x2e;
	  op[1] = disp;
	  reloc_type = keep_reloc ? BFD_RELOC_8_PCREL : BFD_RELOC_NONE;
	  reloc_adjust = 1;
	  break;
	case OPCODE (OT_bra, 3): /* BRA.W - 16 bit.  */
	  op[0] = 0x38;
#if RX_OPCODE_BIG_ENDIAN
	  op[1] = (disp >> 8) & 0xff;
	  op[2] = disp;
#else
	  op[2] = (disp >> 8) & 0xff;
	  op[1] = disp;
#endif
	  reloc_adjust = 1;
	  reloc_type = keep_reloc ? BFD_RELOC_16_PCREL : BFD_RELOC_NONE;
	  break;
	case OPCODE (OT_bra, 4): /* BRA.A - 24 bit.  */
	  op[0] = 0x04;
#if RX_OPCODE_BIG_ENDIAN
	  op[1] = (disp >> 16) & 0xff;
	  op[2] = (disp >> 8) & 0xff;
	  op[3] = disp;
#else
	  op[3] = (disp >> 16) & 0xff;
	  op[2] = (disp >> 8) & 0xff;
	  op[1] = disp;
#endif
	  reloc_type = keep_reloc ? BFD_RELOC_24_PCREL : BFD_RELOC_NONE;
	  reloc_adjust = 1;
	  break;

	case OPCODE (OT_beq, 1): /* BEQ.S - no change.  */
	  op[0] = 0x10 + (disp & 7);
	  break;
	case OPCODE (OT_beq, 2): /* BEQ.B - 8 bit.  */
	  op[0] = 0x20;
	  op[1] = disp;
	  reloc_adjust = 1;
	  reloc_type = keep_reloc ? BFD_RELOC_8_PCREL : BFD_RELOC_NONE;
	  break;
	case OPCODE (OT_beq, 3): /* BEQ.W - 16 bit.  */
	  op[0] = 0x3a;
#if RX_OPCODE_BIG_ENDIAN
	  op[1] = (disp >> 8) & 0xff;
	  op[2] = disp;
#else
	  op[2] = (disp >> 8) & 0xff;
	  op[1] = disp;
#endif
	  reloc_type = keep_reloc ? BFD_RELOC_16_PCREL : BFD_RELOC_NONE;
	  reloc_adjust = 1;
	  break;
	case OPCODE (OT_beq, 5): /* BEQ.A - synthetic.  */
	  op[0] = 0x1d; /* bne.s .+5.  */
	  op[1] = 0x04; /* bra.a dsp:24.  */
	  disp -= 1;
#if RX_OPCODE_BIG_ENDIAN
	  op[2] = (disp >> 16) & 0xff;
	  op[3] = (disp >> 8) & 0xff;
	  op[4] = disp;
#else
	  op[4] = (disp >> 16) & 0xff;
	  op[3] = (disp >> 8) & 0xff;
	  op[2] = disp;
#endif
	  reloc_type = keep_reloc ? BFD_RELOC_24_PCREL : BFD_RELOC_NONE;
	  reloc_adjust = 2;
	  break;

	case OPCODE (OT_bne, 1): /* BNE.S - no change.  */
	  op[0] = 0x18 + (disp & 7);
	  break;
	case OPCODE (OT_bne, 2): /* BNE.B - 8 bit.  */
	  op[0] = 0x21;
	  op[1] = disp;
	  reloc_adjust = 1;
	  reloc_type = keep_reloc ? BFD_RELOC_8_PCREL : BFD_RELOC_NONE;
	  break;
	case OPCODE (OT_bne, 3): /* BNE.W - 16 bit.  */
	  op[0] = 0x3b;
#if RX_OPCODE_BIG_ENDIAN
	  op[1] = (disp >> 8) & 0xff;
	  op[2] = disp;
#else
	  op[2] = (disp >> 8) & 0xff;
	  op[1] = disp;
#endif
	  reloc_type = keep_reloc ? BFD_RELOC_16_PCREL : BFD_RELOC_NONE;
	  reloc_adjust = 1;
	  break;
	case OPCODE (OT_bne, 5): /* BNE.A - synthetic.  */
	  op[0] = 0x15; /* beq.s .+5.  */
	  op[1] = 0x04; /* bra.a dsp:24.  */
	  disp -= 1;
#if RX_OPCODE_BIG_ENDIAN
	  op[2] = (disp >> 16) & 0xff;
	  op[3] = (disp >> 8) & 0xff;
	  op[4] = disp;
#else
	  op[4] = (disp >> 16) & 0xff;
	  op[3] = (disp >> 8) & 0xff;
	  op[2] = disp;
#endif
	  reloc_type = keep_reloc ? BFD_RELOC_24_PCREL : BFD_RELOC_NONE;
	  reloc_adjust = 2;
	  break;

	case OPCODE (OT_bsr, 3): /* BSR.W - 16 bit.  */
	  op[0] = 0x39;
#if RX_OPCODE_BIG_ENDIAN
	  op[1] = (disp >> 8) & 0xff;
	  op[2] = disp;
#else
	  op[2] = (disp >> 8) & 0xff;
	  op[1] = disp;
#endif
	  reloc_type = keep_reloc ? BFD_RELOC_16_PCREL : BFD_RELOC_NONE;
	  reloc_adjust = 0;
	  break;
	case OPCODE (OT_bsr, 4): /* BSR.A - 24 bit.  */
	  op[0] = 0x05;
#if RX_OPCODE_BIG_ENDIAN
	  op[1] = (disp >> 16) & 0xff;
	  op[2] = (disp >> 8) & 0xff;
	  op[3] = disp;
#else
	  op[3] = (disp >> 16) & 0xff;
	  op[2] = (disp >> 8) & 0xff;
	  op[1] = disp;
#endif
	  reloc_type = keep_reloc ? BFD_RELOC_24_PCREL : BFD_RELOC_NONE;
	  reloc_adjust = 0;
	  break;

	case OPCODE (OT_bcc, 2): /* Bcond.B - 8 bit.  */
	  op[1] = disp;
	  reloc_type = keep_reloc ? BFD_RELOC_8_PCREL : BFD_RELOC_NONE;
	  break;
	case OPCODE (OT_bcc, 5): /* Bcond.W - synthetic.  */
	  op[0] ^= 1; /* Invert condition.  */
	  op[1] = 5;  /* Displacement.  */
	  op[2] = 0x38;
	  disp -= 2;
#if RX_OPCODE_BIG_ENDIAN
	  op[3] = (disp >> 8) & 0xff;
	  op[4] = disp;
#else
	  op[4] = (disp >> 8) & 0xff;
	  op[3] = disp;
#endif
	  reloc_type = keep_reloc ? BFD_RELOC_16_PCREL : BFD_RELOC_NONE;
	  reloc_adjust = 2;
	  break;
	case OPCODE (OT_bcc, 6): /* Bcond.S - synthetic.  */
	  op[0] ^= 1; /* Invert condition.  */
	  op[1] = 6;  /* Displacement.  */
	  op[2] = 0x04;
	  disp -= 2;
#if RX_OPCODE_BIG_ENDIAN
	  op[3] = (disp >> 16) & 0xff;
	  op[4] = (disp >> 8) & 0xff;
	  op[5] = disp;
#else
	  op[5] = (disp >> 16) & 0xff;
	  op[4] = (disp >> 8) & 0xff;
	  op[3] = disp;
#endif
	  reloc_type = keep_reloc ? BFD_RELOC_24_PCREL : BFD_RELOC_NONE;
	  reloc_adjust = 2;
	  break;

	default:
	  /* These are opcodes we'll relax in th linker, later.  */
	  if (rxb->n_fixups)
	    reloc_type = rxb->fixups[ri].fixP->fx_r_type;
	  break;
	}
      break;

    case RX_RELAX_IMM:
      {
	int nbytes = fragP->fr_subtype - fragP->tc_frag_data->relax[ri].val_ofs;
	int li;
	char * imm = op + fragP->tc_frag_data->relax[ri].val_ofs;

	switch (nbytes)
	  {
	  case 1:
	    li = 1;
	    imm[0] = addr0;
	    reloc_type = BFD_RELOC_8;
	    break;
	  case 2:
	    li = 2;
#if RX_OPCODE_BIG_ENDIAN
	    imm[1] = addr0;
	    imm[0] = addr0 >> 8;
#else
	    imm[0] = addr0;
	    imm[1] = addr0 >> 8;
#endif
	    reloc_type = BFD_RELOC_RX_16_OP;
	    break;
	  case 3:
	    li = 3;
#if RX_OPCODE_BIG_ENDIAN
	    imm[2] = addr0;
	    imm[1] = addr0 >> 8;
	    imm[0] = addr0 >> 16;
#else
	    imm[0] = addr0;
	    imm[1] = addr0 >> 8;
	    imm[2] = addr0 >> 16;
#endif
	    reloc_type = BFD_RELOC_RX_24_OP;
	    break;
	  case 4:
	    li = 0;
#if RX_OPCODE_BIG_ENDIAN
	    imm[3] = addr0;
	    imm[2] = addr0 >> 8;
	    imm[1] = addr0 >> 16;
	    imm[0] = addr0 >> 24;
#else
	    imm[0] = addr0;
	    imm[1] = addr0 >> 8;
	    imm[2] = addr0 >> 16;
	    imm[3] = addr0 >> 24;
#endif
	    reloc_type = BFD_RELOC_RX_32_OP;
	    break;
	  default:
	    as_bad (_("invalid immediate size"));
	    li = -1;
	  }

	switch (fragP->tc_frag_data->relax[ri].field_pos)
	  {
	  case 6:
	    op[0] &= 0xfc;
	    op[0] |= li;
	    break;
	  case 12:
	    op[1] &= 0xf3;
	    op[1] |= li << 2;
	    break;
	  case 20:
	    op[2] &= 0xf3;
	    op[2] |= li << 2;
	    break;
	  default:
	    as_bad (_("invalid immediate field position"));
	  }
      }
      break;

    default:
      if (rxb->n_fixups)
	{
	  reloc_type = fix->fx_r_type;
	  reloc_adjust = 0;
	}
      break;
    }

  if (rxb->n_fixups)
    {

      fix->fx_r_type = reloc_type;
      fix->fx_where += reloc_adjust;
      switch (reloc_type)
	{
	case BFD_RELOC_NONE:
	  fix->fx_size = 0;
	  break;
	case BFD_RELOC_8:
	  fix->fx_size = 1;
	  break;
	case BFD_RELOC_16_PCREL:
	case BFD_RELOC_RX_16_OP:
	  fix->fx_size = 2;
	  break;
	case BFD_RELOC_24_PCREL:
	case BFD_RELOC_RX_24_OP:
	  fix->fx_size = 3;
	  break;
	case BFD_RELOC_RX_32_OP:
	  fix->fx_size = 4;
	  break;
	default:
	  break;
	}
    }

  fragP->fr_fix = fragP->fr_subtype + (fragP->fr_opcode - fragP->fr_literal);
  tprintf ("fragP->fr_fix now %ld (%d + (%p - %p)\n", (long) fragP->fr_fix,
	  fragP->fr_subtype, fragP->fr_opcode, fragP->fr_literal);
  fragP->fr_var = 0;

  if (fragP->fr_next != NULL
      && fragP->fr_next->fr_address - fragP->fr_address != fragP->fr_fix)
    as_bad (_("bad frag at %p : fix %ld addr %ld %ld \n"), fragP,
	    (long) fragP->fr_fix,
	    (long) fragP->fr_address, (long) fragP->fr_next->fr_address);
}

#undef OPCODE

int
rx_validate_fix_sub (struct fix * f)
{
  /* We permit the subtraction of two symbols in a few cases.  */
  /* mov #sym1-sym2, R3 */
  if (f->fx_r_type == BFD_RELOC_RX_32_OP)
    return 1;
  /* .long sym1-sym2 */
  if (f->fx_r_type == BFD_RELOC_RX_DIFF
      && ! f->fx_pcrel
      && (f->fx_size == 4 || f->fx_size == 2 || f->fx_size == 1))
    return 1;
  return 0;
}

long
md_pcrel_from_section (fixS * fixP, segT sec)
{
  long rv;

  if (fixP->fx_addsy != NULL
      && (! S_IS_DEFINED (fixP->fx_addsy)
	  || S_GET_SEGMENT (fixP->fx_addsy) != sec))
    /* The symbol is undefined (or is defined but not in this section).
       Let the linker figure it out.  */
    return 0;

  rv = fixP->fx_frag->fr_address + fixP->fx_where;
  switch (fixP->fx_r_type)
    {
    case BFD_RELOC_RX_DIR3U_PCREL:
      return rv;
    default:
      return rv - 1;
    }
}

void
rx_cons_fix_new (fragS *	frag,
		 int		where,
		 int		size,
		 expressionS *  exp,
		 bfd_reloc_code_real_type type)
{
  switch (size)
    {
    case 1:
      type = BFD_RELOC_8;
      break;
    case 2:
      type = BFD_RELOC_16;
      break;
    case 3:
      type = BFD_RELOC_24;
      break;
    case 4:
      type = BFD_RELOC_32;
      break;
    default:
      as_bad (_("unsupported constant size %d\n"), size);
      return;
    }

  if (exp->X_op == O_subtract && exp->X_op_symbol)
    {
      if (size != 4 && size != 2 && size != 1)
	as_bad (_("difference of two symbols only supported with .long, .short, or .byte"));
      else
	type = BFD_RELOC_RX_DIFF;
    }

  fix_new_exp (frag, where, (int) size, exp, 0, type);
}

void
md_apply_fix (struct fix * f ATTRIBUTE_UNUSED,
	      valueT *     t ATTRIBUTE_UNUSED,
	      segT         s ATTRIBUTE_UNUSED)
{
  /* Instruction bytes are always little endian.  */
  char * op;
  unsigned long val;

  if (f->fx_addsy && S_FORCE_RELOC (f->fx_addsy, 1))
    return;
  if (f->fx_subsy && S_FORCE_RELOC (f->fx_subsy, 1))
    return;

#define OP2(x) op[target_big_endian ? 1-x : x]
#define OP3(x) op[target_big_endian ? 2-x : x]
#define OP4(x) op[target_big_endian ? 3-x : x]

  op = f->fx_frag->fr_literal + f->fx_where;
  val = (unsigned long) * t;

  /* Opcode words are always the same endian.  Data words are either
     big or little endian.  */

  switch (f->fx_r_type)
    {
    case BFD_RELOC_NONE:
      break;

    case BFD_RELOC_RX_RELAX:
      f->fx_done = 1;
      break;

    case BFD_RELOC_RX_DIR3U_PCREL:
      if (val < 3 || val > 10)
	as_bad_where (f->fx_file, f->fx_line,
		      _("jump not 3..10 bytes away (is %d)"), (int) val);
      op[0] &= 0xf8;
      op[0] |= val & 0x07;
      break;

    case BFD_RELOC_8:
    case BFD_RELOC_8_PCREL:
    case BFD_RELOC_RX_8U:
      op[0] = val;
      break;

    case BFD_RELOC_16:
      OP2(1) = val & 0xff;
      OP2(0) = (val >> 8) & 0xff;
      break;

    case BFD_RELOC_16_PCREL:
    case BFD_RELOC_RX_16_OP:
    case BFD_RELOC_RX_16U:
#if RX_OPCODE_BIG_ENDIAN
      op[1] = val & 0xff;
      op[0] = (val >> 8) & 0xff;
#else
      op[0] = val & 0xff;
      op[1] = (val >> 8) & 0xff;
#endif
      break;

    case BFD_RELOC_24:
      OP3(0) = val & 0xff;
      OP3(1) = (val >> 8) & 0xff;
      OP3(2) = (val >> 16) & 0xff;
      break;

    case BFD_RELOC_24_PCREL:
    case BFD_RELOC_RX_24_OP:
    case BFD_RELOC_RX_24U:
#if RX_OPCODE_BIG_ENDIAN
      op[2] = val & 0xff;
      op[1] = (val >> 8) & 0xff;
      op[0] = (val >> 16) & 0xff;
#else
      op[0] = val & 0xff;
      op[1] = (val >> 8) & 0xff;
      op[2] = (val >> 16) & 0xff;
#endif
      break;

    case BFD_RELOC_RX_DIFF:
      switch (f->fx_size)
	{
	case 1:
	  op[0] = val & 0xff;
	  break;
	case 2:
	  OP2(0) = val & 0xff;
	  OP2(1) = (val >> 8) & 0xff;
	  break;
	case 4:
	  OP4(0) = val & 0xff;
	  OP4(1) = (val >> 8) & 0xff;
	  OP4(2) = (val >> 16) & 0xff;
	  OP4(3) = (val >> 24) & 0xff;
	  break;
	}
      break;

    case BFD_RELOC_32:
      OP4(0) = val & 0xff;
      OP4(1) = (val >> 8) & 0xff;
      OP4(2) = (val >> 16) & 0xff;
      OP4(3) = (val >> 24) & 0xff;
      break;

    case BFD_RELOC_RX_32_OP:
#if RX_OPCODE_BIG_ENDIAN
      op[3] = val & 0xff;
      op[2] = (val >> 8) & 0xff;
      op[1] = (val >> 16) & 0xff;
      op[0] = (val >> 24) & 0xff;
#else
      op[0] = val & 0xff;
      op[1] = (val >> 8) & 0xff;
      op[2] = (val >> 16) & 0xff;
      op[3] = (val >> 24) & 0xff;
#endif
      break;

    case BFD_RELOC_RX_NEG8:
      op[0] = - val;
      break;

    case BFD_RELOC_RX_NEG16:
      val = -val;
#if RX_OPCODE_BIG_ENDIAN
      op[1] = val & 0xff;
      op[0] = (val >> 8) & 0xff;
#else
      op[0] = val & 0xff;
      op[1] = (val >> 8) & 0xff;
#endif
      break;

    case BFD_RELOC_RX_NEG24:
      val = -val;
#if RX_OPCODE_BIG_ENDIAN
      op[2] = val & 0xff;
      op[1] = (val >> 8) & 0xff;
      op[0] = (val >> 16) & 0xff;
#else
      op[0] = val & 0xff;
      op[1] = (val >> 8) & 0xff;
      op[2] = (val >> 16) & 0xff;
#endif
      break;

    case BFD_RELOC_RX_NEG32:
      val = -val;
#if RX_OPCODE_BIG_ENDIAN
      op[3] = val & 0xff;
      op[2] = (val >> 8) & 0xff;
      op[1] = (val >> 16) & 0xff;
      op[0] = (val >> 24) & 0xff;
#else
      op[0] = val & 0xff;
      op[1] = (val >> 8) & 0xff;
      op[2] = (val >> 16) & 0xff;
      op[3] = (val >> 24) & 0xff;
#endif
      break;

    case BFD_RELOC_RX_GPRELL:
      val >>= 1;
      /* Fall through.  */
    case BFD_RELOC_RX_GPRELW:
      val >>= 1;
      /* Fall through.  */
    case BFD_RELOC_RX_GPRELB:
#if RX_OPCODE_BIG_ENDIAN
      op[1] = val & 0xff;
      op[0] = (val >> 8) & 0xff;
#else
      op[0] = val & 0xff;
      op[1] = (val >> 8) & 0xff;
#endif
      break;

    default:
      as_bad (_("Unknown reloc in md_apply_fix: %s"),
	      bfd_get_reloc_code_name (f->fx_r_type));
      break;
    }

  if (f->fx_addsy == NULL)
    f->fx_done = 1;
}

arelent **
tc_gen_reloc (asection * sec ATTRIBUTE_UNUSED, fixS * fixp)
{
  static arelent * reloc[5];
  bool is_opcode = false;

  if (fixp->fx_r_type == BFD_RELOC_NONE)
    {
      reloc[0] = NULL;
      return reloc;
    }

  if (fixp->fx_subsy
      && S_GET_SEGMENT (fixp->fx_subsy) == absolute_section)
    {
      fixp->fx_offset -= S_GET_VALUE (fixp->fx_subsy);
      fixp->fx_subsy = NULL;
    }

  reloc[0]		  = XNEW (arelent);
  reloc[0]->sym_ptr_ptr   = XNEW (asymbol *);
  * reloc[0]->sym_ptr_ptr = symbol_get_bfdsym (fixp->fx_addsy);
  reloc[0]->address       = fixp->fx_frag->fr_address + fixp->fx_where;
  reloc[0]->addend        = fixp->fx_offset;

  if (fixp->fx_r_type == BFD_RELOC_RX_32_OP
      && fixp->fx_subsy)
    {
      fixp->fx_r_type = BFD_RELOC_RX_DIFF;
      is_opcode = true;
    }
  else if (sec)
    is_opcode = sec->flags & SEC_CODE;

  /* Certain BFD relocations cannot be translated directly into
     a single (non-Red Hat) RX relocation, but instead need
     multiple RX relocations - handle them here.  */
  switch (fixp->fx_r_type)
    {
    case BFD_RELOC_RX_DIFF:
      reloc[0]->howto         = bfd_reloc_type_lookup (stdoutput, BFD_RELOC_RX_SYM);

      reloc[1]		      = XNEW (arelent);
      reloc[1]->sym_ptr_ptr   = XNEW (asymbol *);
      * reloc[1]->sym_ptr_ptr = symbol_get_bfdsym (fixp->fx_subsy);
      reloc[1]->address       = fixp->fx_frag->fr_address + fixp->fx_where;
      reloc[1]->addend        = 0;
      reloc[1]->howto         = bfd_reloc_type_lookup (stdoutput, BFD_RELOC_RX_SYM);

      reloc[2]		      = XNEW (arelent);
      reloc[2]->howto         = bfd_reloc_type_lookup (stdoutput, BFD_RELOC_RX_OP_SUBTRACT);
      reloc[2]->addend        = 0;
      reloc[2]->sym_ptr_ptr   = reloc[1]->sym_ptr_ptr;
      reloc[2]->address       = fixp->fx_frag->fr_address + fixp->fx_where;

      reloc[3]		      = XNEW (arelent);
      switch (fixp->fx_size)
	{
	case 1:
	  reloc[3]->howto   = bfd_reloc_type_lookup (stdoutput, BFD_RELOC_RX_ABS8);
	  break;
	case 2:
	  if (!is_opcode && target_big_endian)
	    reloc[3]->howto   = bfd_reloc_type_lookup (stdoutput, BFD_RELOC_RX_ABS16_REV);
	  else if (is_opcode)
	    reloc[3]->howto   = bfd_reloc_type_lookup (stdoutput, BFD_RELOC_RX_ABS16UL);
	  else
	    reloc[3]->howto   = bfd_reloc_type_lookup (stdoutput, BFD_RELOC_RX_ABS16);
	  break;
	case 4:
	  if (!is_opcode && target_big_endian)
	    reloc[3]->howto   = bfd_reloc_type_lookup (stdoutput, BFD_RELOC_RX_ABS32_REV);
	  else
	    reloc[3]->howto   = bfd_reloc_type_lookup (stdoutput, BFD_RELOC_RX_ABS32);
	  break;
	}
      reloc[3]->addend      = 0;
      reloc[3]->sym_ptr_ptr = reloc[1]->sym_ptr_ptr;
      reloc[3]->address     = fixp->fx_frag->fr_address + fixp->fx_where;

      reloc[4] = NULL;
      break;

    case BFD_RELOC_RX_GPRELL:
      reloc[0]->howto         = bfd_reloc_type_lookup (stdoutput, BFD_RELOC_RX_SYM);

      reloc[1]		      = XNEW (arelent);
      reloc[1]->sym_ptr_ptr   = XNEW (asymbol *);
      if (gp_symbol == NULL)
	{
	  if (symbol_table_frozen)
	    {
	      symbolS * gp;

	      gp = symbol_find ("__gp");
	      if (gp == NULL)
		as_bad (("unable to create __gp symbol: please re-assemble with the -msmall-data-limit option specified"));
	      else
		gp_symbol = symbol_get_bfdsym (gp);
	    }
	  else
	    gp_symbol = symbol_get_bfdsym (symbol_find_or_make ("__gp"));
	}
      * reloc[1]->sym_ptr_ptr = gp_symbol;
      reloc[1]->address       = fixp->fx_frag->fr_address + fixp->fx_where;
      reloc[1]->addend        = 0;
      reloc[1]->howto         = bfd_reloc_type_lookup (stdoutput, BFD_RELOC_RX_SYM);

      reloc[2]		    = XNEW (arelent);
      reloc[2]->howto       = bfd_reloc_type_lookup (stdoutput, BFD_RELOC_RX_OP_SUBTRACT);
      reloc[2]->addend      = 0;
      reloc[2]->sym_ptr_ptr = reloc[1]->sym_ptr_ptr;
      reloc[2]->address     = fixp->fx_frag->fr_address + fixp->fx_where;

      reloc[3]		    = XNEW (arelent);
      reloc[3]->howto       = bfd_reloc_type_lookup (stdoutput, BFD_RELOC_RX_ABS16UL);
      reloc[3]->addend      = 0;
      reloc[3]->sym_ptr_ptr = reloc[1]->sym_ptr_ptr;
      reloc[3]->address     = fixp->fx_frag->fr_address + fixp->fx_where;

      reloc[4] = NULL;
      break;

    case BFD_RELOC_RX_GPRELW:
      reloc[0]->howto         = bfd_reloc_type_lookup (stdoutput, BFD_RELOC_RX_SYM);

      reloc[1]		      = XNEW (arelent);
      reloc[1]->sym_ptr_ptr   = XNEW (asymbol *);
      if (gp_symbol == NULL)
	{
	  if (symbol_table_frozen)
	    {
	      symbolS * gp;

	      gp = symbol_find ("__gp");
	      if (gp == NULL)
		as_bad (("unable to create __gp symbol: please re-assemble with the -msmall-data-limit option specified"));
	      else
		gp_symbol = symbol_get_bfdsym (gp);
	    }
	  else
	    gp_symbol = symbol_get_bfdsym (symbol_find_or_make ("__gp"));
	}
      * reloc[1]->sym_ptr_ptr = gp_symbol;
      reloc[1]->address       = fixp->fx_frag->fr_address + fixp->fx_where;
      reloc[1]->addend        = 0;
      reloc[1]->howto         = bfd_reloc_type_lookup (stdoutput, BFD_RELOC_RX_SYM);

      reloc[2]		    = XNEW (arelent);
      reloc[2]->howto       = bfd_reloc_type_lookup (stdoutput, BFD_RELOC_RX_OP_SUBTRACT);
      reloc[2]->addend      = 0;
      reloc[2]->sym_ptr_ptr = reloc[1]->sym_ptr_ptr;
      reloc[2]->address     = fixp->fx_frag->fr_address + fixp->fx_where;

      reloc[3]		    = XNEW (arelent);
      reloc[3]->howto       = bfd_reloc_type_lookup (stdoutput, BFD_RELOC_RX_ABS16UW);
      reloc[3]->addend      = 0;
      reloc[3]->sym_ptr_ptr = reloc[1]->sym_ptr_ptr;
      reloc[3]->address     = fixp->fx_frag->fr_address + fixp->fx_where;

      reloc[4] = NULL;
      break;

    case BFD_RELOC_RX_GPRELB:
      reloc[0]->howto         = bfd_reloc_type_lookup (stdoutput, BFD_RELOC_RX_SYM);

      reloc[1]		      = XNEW (arelent);
      reloc[1]->sym_ptr_ptr   = XNEW (asymbol *);
      if (gp_symbol == NULL)
	{
	  if (symbol_table_frozen)
	    {
	      symbolS * gp;

	      gp = symbol_find ("__gp");
	      if (gp == NULL)
		as_bad (("unable to create __gp symbol: please re-assemble with the -msmall-data-limit option specified"));
	      else
		gp_symbol = symbol_get_bfdsym (gp);
	    }
	  else
	    gp_symbol = symbol_get_bfdsym (symbol_find_or_make ("__gp"));
	}
      * reloc[1]->sym_ptr_ptr = gp_symbol;
      reloc[1]->address       = fixp->fx_frag->fr_address + fixp->fx_where;
      reloc[1]->addend        = 0;
      reloc[1]->howto         = bfd_reloc_type_lookup (stdoutput, BFD_RELOC_RX_SYM);

      reloc[2]		    = XNEW (arelent);
      reloc[2]->howto       = bfd_reloc_type_lookup (stdoutput, BFD_RELOC_RX_OP_SUBTRACT);
      reloc[2]->addend      = 0;
      reloc[2]->sym_ptr_ptr = reloc[1]->sym_ptr_ptr;
      reloc[2]->address     = fixp->fx_frag->fr_address + fixp->fx_where;

      reloc[3]		    = XNEW (arelent);
      reloc[3]->howto       = bfd_reloc_type_lookup (stdoutput, BFD_RELOC_RX_ABS16U);
      reloc[3]->addend      = 0;
      reloc[3]->sym_ptr_ptr = reloc[1]->sym_ptr_ptr;
      reloc[3]->address     = fixp->fx_frag->fr_address + fixp->fx_where;

      reloc[4] = NULL;
      break;

    case BFD_RELOC_RX_NEG32:
      reloc[0]->howto         = bfd_reloc_type_lookup (stdoutput, BFD_RELOC_RX_SYM);

      reloc[1]		    = XNEW (arelent);
      reloc[1]->howto       = bfd_reloc_type_lookup (stdoutput, BFD_RELOC_RX_OP_NEG);
      reloc[1]->addend      = 0;
      reloc[1]->sym_ptr_ptr = reloc[0]->sym_ptr_ptr;
      reloc[1]->address     = fixp->fx_frag->fr_address + fixp->fx_where;

      reloc[2]		    = XNEW (arelent);
      reloc[2]->howto       = bfd_reloc_type_lookup (stdoutput, BFD_RELOC_RX_ABS32);
      reloc[2]->addend      = 0;
      reloc[2]->sym_ptr_ptr = reloc[0]->sym_ptr_ptr;
      reloc[2]->address     = fixp->fx_frag->fr_address + fixp->fx_where;

      reloc[3] = NULL;
      break;

    default:
      reloc[0]->howto = bfd_reloc_type_lookup (stdoutput, fixp->fx_r_type);
      reloc[1] = NULL;
      break;
    }

  return reloc;
}

void
rx_note_string_insn_use (void)
{
  if ((elf_flags & E_FLAG_RX_SINSNS_MASK) == (E_FLAG_RX_SINSNS_SET | E_FLAG_RX_SINSNS_NO))
    as_bad (_("Use of an RX string instruction detected in a file being assembled without string instruction support"));
  elf_flags |= E_FLAG_RX_SINSNS_SET | E_FLAG_RX_SINSNS_YES;
}

/* Set the ELF specific flags.  */

void
rx_elf_final_processing (void)
{
  elf_elfheader (stdoutput)->e_flags |= elf_flags;
}

/* Scan the current input line for occurrences of Renesas
   local labels and replace them with the GAS version.  */

void
rx_start_line (void)
{
  int in_double_quote = 0;
  int in_single_quote = 0;
  int done = 0;
  char * p = input_line_pointer;
  char prev_char = 0;

  /* Scan the line looking for question marks.  Skip past quote enclosed regions.  */
  do
    {
      switch (*p)
	{
	case '\n':
	case 0:
	  done = 1;
	  break;

	case '"':
	  /* Handle escaped double quote \" inside a string.  */
	  if (prev_char != '\\')
	    in_double_quote = ! in_double_quote;
	  break;

	case '\'':
	  in_single_quote = ! in_single_quote;
	  break;

	case '?':
	  if (in_double_quote || in_single_quote)
	    break;

	  if (p[1] == ':')
	    *p = '1';
	  else if (p[1] == '+')
	    {
	      p[0] = '1';
	      p[1] = 'f';
	    }
	  else if (p[1] == '-')
	    {
	      p[0] = '1';
	      p[1] = 'b';
	    }
	  break;

	default:
	  break;
	}

      prev_char = *p++;
    }
  while (! done);
}
