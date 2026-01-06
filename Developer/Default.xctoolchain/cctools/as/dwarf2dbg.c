/* dwarf2dbg.c - DWARF2 debug support
   Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   Contributed by David Mosberger-Tang <davidm@hpl.hp.com>

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

/* Logical line numbers can be controlled by the compiler via the
   following directives:

	.file FILENO "file.c"
	.loc  FILENO LINENO [COLUMN] [basic_block] [prologue_end] \
	      [epilogue_begin] [is_stmt VALUE] [isa VALUE]
*/

#include <stdlib.h>
#include <string.h>
#include <strings.h> /* cctools-port: For bcmp, bzero ... */
#include <unistd.h>
#include <sys/param.h>
#include <libgen.h>
#define ATTRIBUTE_UNUSED
#define NO_LISTING

#include "as.h"
#include "md.h"
#include "obstack.h"
/* Use ctype.h and these directly instead of "safe-ctype.h" */
#include "ctype.h"
#define ISALPHA(c) isalpha(c)
#define ISDIGIT(c) isdigit(c)

#ifdef HAVE_LIMITS_H
#include <limits.h>
#else
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifndef INT_MAX
#define INT_MAX (int) (((unsigned) (-1)) >> 1)
#endif
#endif

#include "dwarf2dbg.h"
#include <filenames.h>

#ifdef HAVE_DOS_BASED_FILE_SYSTEM
/* We need to decide which character to use as a directory separator.
   Just because HAVE_DOS_BASED_FILE_SYSTEM is defined, it does not
   necessarily mean that the backslash character is the one to use.
   Some environments, eg Cygwin, can support both naming conventions.
   So we use the heuristic that we only need to use the backslash if
   the path is an absolute path starting with a DOS style drive
   selector.  eg C: or D:  */
# define INSERT_DIR_SEPARATOR(string, offset) \
  do \
    { \
      if (offset > 1 \
          && string[0] != 0 \
          && string[1] == ':') \
       string [offset] = '\\'; \
      else \
       string [offset] = '/'; \
    } \
  while (0)
#else
# define INSERT_DIR_SEPARATOR(string, offset) string[offset] = '/'
#endif

#ifndef DWARF2_FORMAT
# define DWARF2_FORMAT() dwarf2_format_32bit
#endif

#ifndef DWARF2_ADDR_SIZE
# define DWARF2_ADDR_SIZE(bfd) (bfd_arch_bits_per_address (bfd) / 8)
#endif

#include "sections.h"
#include "frags.h"
#include "read.h"
#include "xmalloc.h"
#include "input-scrub.h"
#include "messages.h"
#include "symbols.h"
#include "layout.h"

#include "elf/dwarf2.h"

/* Since we can't generate the prolog until the body is complete, we
   use three different subsegments for .debug_line: one holding the
   prolog, one for the directory and filename info, and one for the
   body ("statement program").  */
#define DL_PROLOG	0
#define DL_FILES	1
#define DL_BODY		2

/* If linker relaxation might change offsets in the code, the DWARF special
   opcodes and variable-length operands cannot be used.  If this macro is
   nonzero, use the DW_LNS_fixed_advance_pc opcode instead.  */
#ifndef DWARF2_USE_FIXED_ADVANCE_PC
# define DWARF2_USE_FIXED_ADVANCE_PC	0
#endif

/* First special line opcde - leave room for the standard opcodes.
   Note: If you want to change this, you'll have to update the
   "standard_opcode_lengths" table that is emitted below in
   out_debug_line().  */
#define DWARF2_LINE_OPCODE_BASE		13

#ifndef DWARF2_LINE_BASE
  /* Minimum line offset in a special line info. opcode.  This value
     was chosen to give a reasonable range of values.  */
# define DWARF2_LINE_BASE		-5
#endif

/* Range of line offsets in a special line info. opcode.  */
#ifndef DWARF2_LINE_RANGE
# define DWARF2_LINE_RANGE		14
#endif

#ifndef DWARF2_LINE_MIN_INSN_LENGTH
  /* Define the architecture-dependent minimum instruction length (in
     bytes).  This value should be rather too small than too big.  */
# define DWARF2_LINE_MIN_INSN_LENGTH	1
#endif

/* Flag that indicates the initial value of the is_stmt_start flag.  */
#define	DWARF2_LINE_DEFAULT_IS_STMT	1

/* Given a special op, return the line skip amount.  */
#define SPECIAL_LINE(op) \
	(((op) - DWARF2_LINE_OPCODE_BASE)%DWARF2_LINE_RANGE + DWARF2_LINE_BASE)

/* Given a special op, return the address skip amount (in units of
   DWARF2_LINE_MIN_INSN_LENGTH.  */
#define SPECIAL_ADDR(op) (((op) - DWARF2_LINE_OPCODE_BASE)/DWARF2_LINE_RANGE)

/* The maximum address skip amount that can be encoded with a special op.  */
#define MAX_SPECIAL_ADDR_DELTA		SPECIAL_ADDR(255)

struct line_entry {
  struct line_entry *next;
  symbolS *label;
  struct dwarf2_line_info loc;
};

struct line_subseg {
  struct line_subseg *next;
  subsegT subseg;
  struct line_entry *head;
  struct line_entry **ptail;
};

struct line_seg {
  struct line_seg *next;
  segT seg;
  struct line_subseg *head;
  symbolS *text_start;
  symbolS *text_end;
};

/* Collects data for all line table entries during assembly.  */
static struct line_seg *all_segs;

struct file_entry {
  char *filename;
  unsigned int dir;
};

/* Table of files used by .debug_line.  */
static struct file_entry *files;
static unsigned int files_in_use;
static unsigned int files_allocated;

/* Table of directories used by .debug_line.  */
static char **dirs;
static unsigned int dirs_in_use;
static unsigned int dirs_allocated;

/* TRUE when we've seen a .loc directive recently.  Used to avoid
   doing work when there's nothing to do.  */
static bfd_boolean loc_directive_seen;

/* TRUE when we're supposed to set the basic block mark whenever a
   label is seen.  */
bfd_boolean dwarf2_loc_mark_labels;

/* Current location as indicated by the most recent .loc directive.  */
static struct dwarf2_line_info current = {
  1, 1, 0, 0,
  DWARF2_LINE_DEFAULT_IS_STMT ? DWARF2_FLAG_IS_STMT : 0
};

/* When creating dwarf2 debugging information for assembly files, the --gdwarf2
   flag is specified, the variable dwarf2_file_number is used to generate a
   .file for each assembly source file in read_a_source_file().  */
uint32_t dwarf2_file_number = 0;

/* Info gathered where creating dwarf2 debugging information for assembly files
   when --gdwarf2 is specified.  This is done in make_subprogram_for_symbol()
   in symbols.c . */
struct dwarf2_subprogram_info *dwarf2_subprograms_info = NULL;

/* The size of an address on the target.  */
static unsigned int sizeof_address;

static struct line_subseg *get_line_subseg (segT, subsegT);
static unsigned int get_filenum (char *, unsigned int);
static struct frag *first_frag_for_seg (segT);
static struct frag *last_frag_for_seg (segT);
static void out_byte (int);
static void out_opcode (int);
static void out_two (int);
static void out_four (int);
static void out_abbrev (int, int);
static void out_uleb128 (unsigned int);
static void out_sleb128 (int);
static offsetT get_frag_fix (fragS *, segT);
static void out_set_addr (symbolS *);
static int size_inc_line_addr (int, addressT);
static void emit_inc_line_addr (int, addressT, char *, int);
static void out_inc_line_addr (int, addressT);
static void out_fixed_inc_line_addr (int, symbolS *, symbolS *);
static void relax_inc_line_addr (int, symbolS *, symbolS *);
static int process_entries (segT, struct line_entry *);
static void out_file_list (void);
static void out_debug_line (struct frchain *);
static void out_debug_aranges (struct frchain *, struct frchain *);
static void out_debug_abbrev (struct frchain *);

/*
 * This version of emit_expr() was based on the one in GAS's read.c but greatly
 * simplified for the ported specific uses in here and the Mach-O version of the
 * expression stuct.  As such this looks at the X_seg field (aka X_op) and only
 * deals with the expressions which have "type's" of SEG_SECT (aka O_symbol),
 * and SEG_DIFFSECT.
 *
 * The function of this routine is to:
 * Put the contents of expression EXP into the object file using
 * NBYTES bytes.
 */
static
void
emit_expr (expressionS *exp, unsigned int nbytes)
{
  segT X_seg;
  char *p;

  X_seg = exp->X_seg;
  if(X_seg != SEG_SECT && X_seg != SEG_DIFFSECT)
    as_fatal("internal error, emit_expr() called with unexpected expression "
	     "type");

  p = frag_more ((int) nbytes);

  fix_new(frag_now,
	  (int)(p - frag_now->fr_literal),
	  nbytes,
	  exp->X_add_symbol,
	  exp->X_subtract_symbol,
	  (signed_target_addr_t)exp->X_add_number,
	  0,
	  0,
	  0);
}

#define TC_DWARF2_EMIT_OFFSET macho_dwarf2_emit_offset
static void
macho_dwarf2_emit_offset (symbolS *symbol, unsigned int size);

#ifndef TC_DWARF2_EMIT_OFFSET
#define TC_DWARF2_EMIT_OFFSET  generic_dwarf2_emit_offset

/* Create an offset to .dwarf2_*.  */

static void
generic_dwarf2_emit_offset (symbolS *symbol, unsigned int size)
{
  expressionS expr;

  memset(&expr, '\0', sizeof(expr));
  expr.X_op = O_symbol;
  expr.X_add_symbol = symbol;
  expr.X_add_number = 0;
  emit_expr (&expr, size);
}
#endif

/* Find or create an entry for SEG+SUBSEG in ALL_SEGS.  */

static struct line_subseg *
get_line_subseg (segT seg, subsegT subseg)
{
  static segT last_seg;
  static subsegT last_subseg;
  static struct line_subseg *last_line_subseg;

  struct line_seg **ps, *s;
  struct line_subseg **pss, *ss;

  if (seg == last_seg && subseg == last_subseg)
    return last_line_subseg;

  for (ps = &all_segs; (s = *ps) != NULL; ps = &s->next)
    if (s->seg == seg)
      goto found_seg;

  s = (struct line_seg *) xmalloc (sizeof (*s));
  s->next = NULL;
  s->seg = seg;
  s->head = NULL;
  *ps = s;

 found_seg:
  for (pss = &s->head; (ss = *pss) != NULL ; pss = &ss->next)
    {
      if (ss->subseg == subseg)
	goto found_subseg;
      if (ss->subseg > subseg)
	break;
    }

  ss = (struct line_subseg *) xmalloc (sizeof (*ss));
  ss->next = *pss;
  ss->subseg = subseg;
  ss->head = NULL;
  ss->ptail = &ss->head;
  *pss = ss;

 found_subseg:
  last_seg = seg;
  last_subseg = subseg;
  last_line_subseg = ss;

  return ss;
}

/* Record an entry for LOC occurring at LABEL.  */

static void
dwarf2_gen_line_info_1 (symbolS *label, struct dwarf2_line_info *loc)
{
  struct line_subseg *ss;
  struct line_entry *e;

  e = (struct line_entry *) xmalloc (sizeof (*e));
  e->next = NULL;
  e->label = label;
  e->loc = *loc;

  ss = get_line_subseg (now_seg, now_subseg);
  *ss->ptail = e;
  ss->ptail = &e->next;
}

/* Record an entry for LOC occurring at OFS within the current fragment.  */

void
dwarf2_gen_line_info (addressT ofs, struct dwarf2_line_info *loc)
{
  static unsigned int line = -1;
  static unsigned int filenum = -1;

  symbolS *sym;

  /* Early out for as-yet incomplete location information.  */
  if (loc->filenum == 0 || loc->line == 0)
    return;

  /* Don't emit sequences of line symbols for the same line when the
     symbols apply to assembler code.  It is necessary to emit
     duplicate line symbols when a compiler asks for them, because GDB
     uses them to determine the end of the prologue.  */
  if (debug_type == DEBUG_DWARF2
      && line == loc->line && filenum == loc->filenum)
    return;

  line = loc->line;
  filenum = loc->filenum;

  sym = symbol_temp_new (now_seg, (valueT)ofs, frag_now);
  dwarf2_gen_line_info_1 (sym, loc);
}

/* Returns the current source information.  If .file directives have
   been encountered, the info for the corresponding source file is
   returned.  Otherwise, the info for the assembly source file is
   returned.  */

void
dwarf2_where (struct dwarf2_line_info *line)
{
  if (debug_type == DEBUG_DWARF2)
    {
      char *filename;
#ifdef OLD
      as_where (&filename, &line->line);
#else
      as_file_and_line(&filename, &line->line);
#endif
      line->filenum = get_filenum (filename, 0);
      line->column = 0;
      line->flags = DWARF2_FLAG_IS_STMT;
      line->isa = current.isa;
    }
  else
    *line = current;
}

/* A hook to allow the target backend to inform the line number state 
   machine of isa changes when assembler debug info is enabled.  */

void
dwarf2_set_isa (unsigned int isa)
{
  current.isa = isa;
}

/* Called for each machine instruction, or relatively atomic group of
   machine instructions (ie built-in macro).  The instruction or group
   is SIZE bytes in length.  If dwarf2 line number generation is called
   for, emit a line statement appropriately.  */

void
dwarf2_emit_insn (int size)
{
  struct dwarf2_line_info loc;

  if (loc_directive_seen)
    {
      /* Use the last location established by a .loc directive, not
	 the value returned by dwarf2_where().  That calls as_where()
	 which will return either the logical input file name (foo.c)
	or the physical input file name (foo.s) and not the file name
	specified in the most recent .loc directive (eg foo.h).  */
      loc = current;

      /* Unless we generate DWARF2 debugging information for each
	 assembler line, we only emit one line symbol for one LOC.  */
      if (debug_type != DEBUG_DWARF2)
	loc_directive_seen = FALSE;
    }
  else if (debug_type != DEBUG_DWARF2 || frchain_now->frch_nsect != text_nsect)
    return;
  else
    dwarf2_where (&loc);

  dwarf2_gen_line_info (frag_now_fix () - size, &loc);

  current.flags &= ~(DWARF2_FLAG_BASIC_BLOCK
		     | DWARF2_FLAG_PROLOGUE_END
		     | DWARF2_FLAG_EPILOGUE_BEGIN);
}

/* Called for each (preferably code) label.  If dwarf2_loc_mark_labels
   is enabled, emit a basic block marker.  */

void
dwarf2_emit_label (symbolS *label)
{
  struct dwarf2_line_info loc;

  if (!dwarf2_loc_mark_labels)
    return;
#ifdef OLD
  if (S_GET_SEGMENT (label) != now_seg)
#else
  if (label->sy_nlist.n_sect != now_seg)
#endif
      return;
#ifdef OLD
  if (!(bfd_get_section_flags (stdoutput, now_seg) & SEC_CODE))
#else
  if ((get_section_by_nsect(now_seg)->frch_section.flags &
       S_ATTR_SOME_INSTRUCTIONS) == 0)
#endif
    return;

  if (debug_type == DEBUG_DWARF2)
    dwarf2_where (&loc);
  else
    {
      loc = current;
      loc_directive_seen = FALSE;
    }

  loc.flags |= DWARF2_FLAG_BASIC_BLOCK;

  current.flags &= ~(DWARF2_FLAG_BASIC_BLOCK
		     | DWARF2_FLAG_PROLOGUE_END
		     | DWARF2_FLAG_EPILOGUE_BEGIN);

  dwarf2_gen_line_info_1 (label, &loc);
}

#ifndef OLD
/*
 * Internal lbasename() routine that must be used in this code.  As it returns
 * a pointer into the path as get_filenum() expects for the base name of the
 * path.
 */
static
char *
lbasename(
char *path)
{
    char *p;

	p = strrchr(path, '/');
	if(p != NULL)
	    return(p + 1);
	else
	    return(path);
}
#endif /* !defined(OLD) */

/* Get a .debug_line file number for FILENAME.  If NUM is nonzero,
   allocate it on that file table slot, otherwise return the first
   empty one.  */

static unsigned int
get_filenum (char *filename, unsigned int num)
{
  static unsigned int last_used;
  static size_t last_used_dir_len;
  char *file;
  size_t dir_len;
  unsigned int i, dir;

  if (num == 0 && last_used)
    {
      if (! files[last_used].dir
	  && strcmp (filename, files[last_used].filename) == 0)
	return last_used;
      if (files[last_used].dir
	  && strncmp (filename, dirs[files[last_used].dir],
		      last_used_dir_len) == 0
	  && IS_DIR_SEPARATOR (filename [last_used_dir_len])
	  && strcmp (filename + last_used_dir_len + 1,
		     files[last_used].filename) == 0)
	return last_used;
    }

  file = lbasename (filename);
  /* Don't make empty string from / or A: from A:/ .  */
#ifdef HAVE_DOS_BASED_FILE_SYSTEM
  if (file <= filename + 3)
    file = filename;
#else
  if (file == filename + 1)
    file = filename;
#endif
  dir_len = file - filename;

  dir = 0;
  if (dir_len)
    {
      --dir_len;
      for (dir = 1; dir < dirs_in_use; ++dir)
	if (strncmp (filename, dirs[dir], dir_len) == 0
	    && dirs[dir][dir_len] == '\0')
	  break;

      if (dir >= dirs_in_use)
	{
	  if (dir >= dirs_allocated)
	    {
	      dirs_allocated = dir + 32;
	      dirs = (char **)
		     xrealloc (dirs, (dir + 32) * sizeof (const char *));
	    }

	  dirs[dir] = xmalloc (dir_len + 1);
	  memcpy (dirs[dir], filename, dir_len);
	  dirs[dir][dir_len] = '\0';
	  dirs_in_use = dir + 1;
	}
    }

  if (num == 0)
    {
      for (i = 1; i < files_in_use; ++i)
	if (files[i].dir == dir
	    && files[i].filename
	    && strcmp (file, files[i].filename) == 0)
	  {
	    last_used = i;
	    last_used_dir_len = dir_len;
	    return i;
	  }
    }
  else
    i = num;

  if (i >= files_allocated)
    {
      unsigned int old = files_allocated;

      files_allocated = i + 32;
      files = (struct file_entry *)
	xrealloc (files, (i + 32) * sizeof (struct file_entry));

      memset (files + old, 0, (i + 32 - old) * sizeof (struct file_entry));
    }

#ifdef OLD
  files[i].filename = num ? file : xstrdup (file);
#else
  if(num == 0)
      files[i].filename = file;
  else
    {
      files[i].filename = xmalloc(strlen(file) + 1);
      strcpy(files[i].filename, file);
    }
#endif
  files[i].dir = dir;
  if (files_in_use < i + 1)
    files_in_use = i + 1;
  last_used = i;
  last_used_dir_len = dir_len;

  return i;
}

/* Handle two forms of .file directive:
   - Pass .file "source.c" to s_app_file
   - Handle .file 1 "source.c" by adding an entry to the DWARF-2 file table

   If an entry is added to the file table, return a pointer to the filename. */

char *
dwarf2_directive_file (uintptr_t dummy ATTRIBUTE_UNUSED)
{
  offsetT num;
  char *filename;
  int filename_len;

  /* Continue to accept a bare string and pass it off.  */
  SKIP_WHITESPACE ();
  if (*input_line_pointer == '"')
    {
      s_app_file (0);
      return NULL;
    }

  num = get_absolute_expression ();
  filename = demand_copy_C_string (&filename_len);
  if (filename == NULL)
    return NULL;
  demand_empty_rest_of_line ();

  /*
   * If the --gdwarf2 flag is present to generate dwarf2 for assembly code
   * then it is an error to see a .file directive in the source.
   */
  if (debug_type == DEBUG_DWARF2)
    {
      as_bad (_("input can't have .file dwarf directives when -g is used to "
		"generate dwarf debug info for assembly code"));
      return NULL;
    }

  if (num < 1)
    {
      as_bad (_("file number less than one"));
      return NULL;
    }

  if (num < (int) files_in_use && files[num].filename != 0)
    {
      as_bad (_("file number %ld already allocated"), (long) num);
      return NULL;
    }

  get_filenum (filename, (unsigned int)num);

  return filename;
}

/*
 * Same functionality as above but this is used when generating dwarf debugging
 * info for assembly files with --gdwarf2.
 */
void
dwarf2_file (
char *filename,
offsetT num)
{
  get_filenum (filename, (unsigned int)num);
}

void
dwarf2_directive_loc (uintptr_t dummy ATTRIBUTE_UNUSED)
{
  unsigned int filenum, line;

  filenum = (unsigned int)get_absolute_expression ();
  SKIP_WHITESPACE ();
  line = (unsigned int)get_absolute_expression ();

  if (filenum < 1)
    {
      as_bad (_("file number less than one"));
      return;
    }
  if (filenum >= (int) files_in_use || files[filenum].filename == 0)
    {
      as_bad (_("unassigned file number %ld"), (long) filenum);
      return;
    }

  current.filenum = filenum;
  current.line = line;

#ifndef NO_LISTING
  if (listing)
    {
      if (files[filenum].dir)
	{
	  size_t dir_len = strlen (dirs[files[filenum].dir]);
	  size_t file_len = strlen (files[filenum].filename);
	  char *cp = (char *) alloca (dir_len + 1 + file_len + 1);

	  memcpy (cp, dirs[files[filenum].dir], dir_len);
	  INSERT_DIR_SEPARATOR (cp, dir_len);
	  memcpy (cp + dir_len + 1, files[filenum].filename, file_len);
	  cp[dir_len + file_len + 1] = '\0';
	  listing_source_file (cp);
	}
      else
	listing_source_file (files[filenum].filename);
      listing_source_line (line);
    }
#endif

  SKIP_WHITESPACE ();
  if (ISDIGIT (*input_line_pointer))
    {
      current.column = (unsigned int)get_absolute_expression ();
      SKIP_WHITESPACE ();
    }

  while (ISALPHA (*input_line_pointer))
    {
      char *p, c;
      unsigned int value;

      p = input_line_pointer;
      c = get_symbol_end ();

      if (strcmp (p, "basic_block") == 0)
	{
	  current.flags |= DWARF2_FLAG_BASIC_BLOCK;
	  *input_line_pointer = c;
	}
      else if (strcmp (p, "prologue_end") == 0)
	{
	  current.flags |= DWARF2_FLAG_PROLOGUE_END;
	  *input_line_pointer = c;
	}
      else if (strcmp (p, "epilogue_begin") == 0)
	{
	  current.flags |= DWARF2_FLAG_EPILOGUE_BEGIN;
	  *input_line_pointer = c;
	}
      else if (strcmp (p, "is_stmt") == 0)
	{
	  *input_line_pointer = c;
	  value = (unsigned int)get_absolute_expression ();
	  if (value == 0)
	    current.flags &= ~DWARF2_FLAG_IS_STMT;
	  else if (value == 1)
	    current.flags |= DWARF2_FLAG_IS_STMT;
	  else
	    {
	      as_bad (_("is_stmt value not 0 or 1"));
	      return;
	    }
	}
      else if (strcmp (p, "isa") == 0)
	{
          *input_line_pointer = c;
	  value = (unsigned int)get_absolute_expression ();
	  if (value >= 0)
	    current.isa = value;
	  else
	    {
	      as_bad (_("isa number less than zero"));
	      return;
	    }
	}
      else
	{
	  as_bad (_("unknown .loc sub-directive `%s'"), p);
          *input_line_pointer = c;
	  return;
	}

      SKIP_WHITESPACE ();
    }

  demand_empty_rest_of_line ();
  loc_directive_seen = TRUE;
}

/*
 * Same functionality as above but this is used when generating dwarf debugging
 * info for assembly files with --gdwarf2.
 */
void
dwarf2_loc (offsetT filenum, offsetT line)
{
  current.filenum = (unsigned int)filenum;
  current.line = (unsigned int)line;
}

void
dwarf2_directive_loc_mark_labels (uintptr_t dummy ATTRIBUTE_UNUSED)
{
  offsetT value = get_absolute_expression ();

  if (value != 0 && value != 1)
    {
      as_bad (_("expected 0 or 1"));
      ignore_rest_of_line ();
    }
  else
    {
      dwarf2_loc_mark_labels = value != 0;
      demand_empty_rest_of_line ();
    }
}

static struct frag *
first_frag_for_seg (segT seg)
{
  return get_section_by_nsect(seg)->frch_root;
}

static struct frag *
last_frag_for_seg (segT seg)
{
  frchainS *f = get_section_by_nsect(seg);

#ifdef OLD
  while (f->frch_next != NULL)
    f = f->frch_next;
#endif

  return f->frch_last;
}

/* Emit a single byte into the current segment.  */

static inline void
out_byte (int byte)
{
  FRAG_APPEND_1_CHAR (byte);
}

/* Emit a statement program opcode into the current segment.  */

static inline void
out_opcode (int opc)
{
  out_byte (opc);
}

/* Emit a two-byte word into the current segment.  */

static inline void
out_two (int data)
{
  md_number_to_chars (frag_more (2), data, 2);
}

/* Emit a four byte word into the current segment.  */

static inline void
out_four (int data)
{
  md_number_to_chars (frag_more (4), data, 4);
}

/* Emit an unsigned "little-endian base 128" number.  */

static void
out_uleb128 (unsigned int value)
{
  output_leb128 (frag_more (sizeof_leb128 (value, 0)), value, 0);
}

/* Emit a signed "little-endian base 128" number.  */

static void
out_sleb128 (int value)
{
  output_leb128 (frag_more (sizeof_leb128 (value, 1)), value, 1);
}

/* Emit a tuple for .debug_abbrev.  */

static inline void
out_abbrev (int name, int form)
{
  out_uleb128 (name);
  out_uleb128 (form);
}

/* Get the size of a fragment.  */

static offsetT
get_frag_fix (fragS *frag, segT seg)
{
#ifdef OLD
#endif
  frchainS *fr;

  if (frag->fr_next)
    return frag->fr_fix;

  /* If a fragment is the last in the chain, special measures must be
     taken to find its size before relaxation, since it may be pending
     on some subsegment chain.  */
  for (fr = get_section_by_nsect(seg); fr; fr = fr->frch_next)
    if (fr->frch_last == frag) {
#ifdef OLD
      return (char *) obstack_next_free (&frags /* was  &fr->frch_obstack */) - frag->fr_literal;

      abort ();
#else
     /* This depends on add_last_frags_to_sections() being called to make
        sure the last frag in a section is a ".fill 0".  Since in the Mach-O
        assembler we don't have an obstack per section using the above
        code to subtract the frag's fr_literal from what ever was the last
        frag allocated will not work.   */
      return(0);
#endif
    }
    as_fatal("internal error, did not find frag in section in "
	     "get_frag_fix()");
    return(0); /* not reached, but here remove compiler warning */
}

/* Set an absolute address (may result in a relocation entry).  */

static void
out_set_addr (symbolS *sym)
{
  expressionS expr;

  out_opcode (DW_LNS_extended_op);
  out_uleb128 (sizeof_address + 1);

  out_opcode (DW_LNE_set_address);
  memset(&expr, '\0', sizeof(expr));
  expr.X_op = O_symbol;
  expr.X_add_symbol = sym;
  expr.X_add_number = 0;
  emit_expr (&expr, sizeof_address);
}

#if DWARF2_LINE_MIN_INSN_LENGTH > 1
static void scale_addr_delta (addressT *);

static void
scale_addr_delta (addressT *addr_delta)
{
  static int printed_this = 0;
  if (*addr_delta % DWARF2_LINE_MIN_INSN_LENGTH != 0)
    {
      if (!printed_this)
	as_bad("unaligned opcodes detected in executable segment");
      printed_this = 1;
    }
  *addr_delta /= DWARF2_LINE_MIN_INSN_LENGTH;
}
#else
#define scale_addr_delta(A)
#endif

/* Encode a pair of line and address skips as efficiently as possible.
   Note that the line skip is signed, whereas the address skip is unsigned.

   The following two routines *must* be kept in sync.  This is
   enforced by making emit_inc_line_addr abort if we do not emit
   exactly the expected number of bytes.  */

static int
size_inc_line_addr (int line_delta, addressT addr_delta)
{
  unsigned int tmp, opcode;
  int len = 0;

  /* Scale the address delta by the minimum instruction length.  */
  scale_addr_delta (&addr_delta);

  /* INT_MAX is a signal that this is actually a DW_LNE_end_sequence.
     We cannot use special opcodes here, since we want the end_sequence
     to emit the matrix entry.  */
  if (line_delta == INT_MAX)
    {
      if (addr_delta == MAX_SPECIAL_ADDR_DELTA)
	len = 1;
      else
	len = 1 + (int)sizeof_leb128((valueT)addr_delta, 0);
      return len + 3;
    }

  /* Bias the line delta by the base.  */
  tmp = line_delta - DWARF2_LINE_BASE;

  /* If the line increment is out of range of a special opcode, we
     must encode it with DW_LNS_advance_line.  */
  if (tmp >= DWARF2_LINE_RANGE)
    {
      len = 1 + sizeof_leb128 (line_delta, 1);
      line_delta = 0;
      tmp = 0 - DWARF2_LINE_BASE;
    }

  /* Bias the opcode by the special opcode base.  */
  tmp += DWARF2_LINE_OPCODE_BASE;

  /* Avoid overflow when addr_delta is large.  */
  if (addr_delta < 256 + MAX_SPECIAL_ADDR_DELTA)
    {
      /* Try using a special opcode.  */
      opcode = (unsigned int)(tmp + addr_delta * DWARF2_LINE_RANGE);
      if (opcode <= 255)
	return len + 1;

      /* Try using DW_LNS_const_add_pc followed by special op.  */
      opcode = (unsigned int)(tmp + (addr_delta - MAX_SPECIAL_ADDR_DELTA) * DWARF2_LINE_RANGE);
      if (opcode <= 255)
	return len + 2;
    }

  /* Otherwise use DW_LNS_advance_pc.  */
  len += 1 + (int)sizeof_leb128 ((valueT)addr_delta, 0);

  /* DW_LNS_copy or special opcode.  */
  len += 1;

  return len;
}

static void
emit_inc_line_addr (int line_delta, addressT addr_delta, char *p, int len)
{
  unsigned int tmp, opcode;
  int need_copy = 0;
  char *end = p + len;

  /* Line number sequences cannot go backward in addresses.  This means
     we've incorrectly ordered the statements in the sequence.  */
  assert ((offsetT) addr_delta >= 0);

  /* Scale the address delta by the minimum instruction length.  */
  scale_addr_delta (&addr_delta);

  /* INT_MAX is a signal that this is actually a DW_LNE_end_sequence.
     We cannot use special opcodes here, since we want the end_sequence
     to emit the matrix entry.  */
  if (line_delta == INT_MAX)
    {
      if (addr_delta == MAX_SPECIAL_ADDR_DELTA)
	*p++ = DW_LNS_const_add_pc;
      else
	{
	  *p++ = DW_LNS_advance_pc;
	  p +=
	    (int)output_leb128 (p, (valueT)addr_delta, 0);
	}

      *p++ = DW_LNS_extended_op;
      *p++ = 1;
      *p++ = DW_LNE_end_sequence;
      goto done;
    }

  /* Bias the line delta by the base.  */
  tmp = line_delta - DWARF2_LINE_BASE;

  /* If the line increment is out of range of a special opcode, we
     must encode it with DW_LNS_advance_line.  */
  if (tmp >= DWARF2_LINE_RANGE)
    {
      *p++ = DW_LNS_advance_line;
      p += output_leb128 (p, line_delta, 1);

      line_delta = 0;
      tmp = 0 - DWARF2_LINE_BASE;
      need_copy = 1;
    }

  /* Prettier, I think, to use DW_LNS_copy instead of a "line +0, addr +0"
     special opcode.  */
  if (line_delta == 0 && addr_delta == 0)
    {
      *p++ = DW_LNS_copy;
      goto done;
    }

  /* Bias the opcode by the special opcode base.  */
  tmp += DWARF2_LINE_OPCODE_BASE;

  /* Avoid overflow when addr_delta is large.  */
  if (addr_delta < 256 + MAX_SPECIAL_ADDR_DELTA)
    {
      /* Try using a special opcode.  */
      opcode = (unsigned int)(tmp + addr_delta * DWARF2_LINE_RANGE);
      if (opcode <= 255)
	{
	  *p++ = opcode;
	  goto done;
	}

      /* Try using DW_LNS_const_add_pc followed by special op.  */
      opcode = (unsigned int)(tmp + (addr_delta - MAX_SPECIAL_ADDR_DELTA) * DWARF2_LINE_RANGE);
      if (opcode <= 255)
	{
	  *p++ = DW_LNS_const_add_pc;
	  *p++ = opcode;
	  goto done;
	}
    }

  /* Otherwise use DW_LNS_advance_pc.  */
  *p++ = DW_LNS_advance_pc;
  p += (int)output_leb128 (p, (valueT)addr_delta, 0);

  if (need_copy)
    *p++ = DW_LNS_copy;
  else
    *p++ = tmp;

 done:
  if (p != end)
    as_fatal("internal error, p != end at the end of emit_inc_line_addr()");
}

/* Handy routine to combine calls to the above two routines.  */

static void
out_inc_line_addr (int line_delta, addressT addr_delta)
{
  int len = size_inc_line_addr (line_delta, addr_delta);
  emit_inc_line_addr (line_delta, addr_delta, frag_more (len), len);
}

/* Write out an alternative form of line and address skips using
   DW_LNS_fixed_advance_pc opcodes.  This uses more space than the default
   line and address information, but it helps support linker relaxation that
   changes the code offsets.  */

static void
out_fixed_inc_line_addr (int line_delta, symbolS *to_sym, symbolS *from_sym)
{
  expressionS expr;

  /* INT_MAX is a signal that this is actually a DW_LNE_end_sequence.  */
  if (line_delta == INT_MAX)
    {
      out_opcode (DW_LNS_fixed_advance_pc);
      memset(&expr, '\0', sizeof(expr));
#ifdef OLD
      expr.X_op = O_subtract;
#else
      expr.X_seg = SEG_DIFFSECT;
#endif
      expr.X_add_symbol = to_sym;
#ifdef OLD
      expr.X_op_symbol = from_sym;
#else
      expr.X_subtract_symbol = from_sym;
#endif
      expr.X_add_number = 0;
      emit_expr (&expr, 2);

      out_opcode (DW_LNS_extended_op);
      out_byte (1);
      out_opcode (DW_LNE_end_sequence);
      return;
    }

  out_opcode (DW_LNS_advance_line);
  out_sleb128 (line_delta);

  out_opcode (DW_LNS_fixed_advance_pc);
  memset(&expr, '\0', sizeof(expr));
#ifdef OLD
  expr.X_op = O_subtract;
#else
  expr.X_seg = SEG_DIFFSECT;
#endif
  expr.X_add_symbol = to_sym;
#ifdef OLD
  expr.X_op_symbol = from_sym;
#else
  expr.X_subtract_symbol = from_sym;
#endif
  expr.X_add_number = 0;
  emit_expr (&expr, 2);

  out_opcode (DW_LNS_copy);
}

/* Generate a variant frag that we can use to relax address/line
   increments between fragments of the target segment.  */

static void
relax_inc_line_addr (int line_delta, symbolS *to_sym, symbolS *from_sym)
{
  int max_chars;
#ifdef OLD
  expressionS expr;
#else
  symbolS *sym;
  expressionS *expression;

  sym = symbol_temp_new(to_sym->sy_other, 0, NULL);
  expression = xmalloc(sizeof(expressionS));
  memset(expression, '\0', sizeof(expressionS));
  sym->expression = expression;
  sym->sy_frag = &zero_address_frag;
#endif

#ifdef OLD
  expr.X_op = O_subtract;
  expr.X_add_symbol = to_sym;
  expr.X_op_symbol = from_sym;
  expr.X_add_number = 0;
#else
  expression->X_seg = SEG_DIFFSECT;
  expression->X_add_symbol = to_sym;
  expression->X_subtract_symbol = from_sym;
  expression->X_add_number = 0;
#endif

  /* The maximum size of the frag is the line delta with a maximum
     sized address delta.  */
  max_chars = size_inc_line_addr (line_delta, -DWARF2_LINE_MIN_INSN_LENGTH);

#ifdef OLD
  frag_var (rs_dwarf2dbg, max_chars, max_chars, 1,
	    make_expr_symbol (&expr), line_delta, NULL);
#else
  frag_var (rs_dwarf2dbg, max_chars, max_chars, 1,
	    sym, line_delta, NULL);
#endif
}


/* The function estimates the size of a rs_dwarf2dbg variant frag
   based on the current values of the symbols.  It is called before
   the relaxation loop.  We set fr_subtype to the expected length.  */

int
dwarf2dbg_estimate_size_before_relax (fragS *frag)
{
  offsetT addr_delta;
  int size;
#ifndef OLD
  expressionS *expression;
#endif

#ifdef OLD
  addr_delta = resolve_symbol_value (frag->fr_symbol);
#else
  if(frag->fr_symbol == NULL || frag->fr_symbol->expression == NULL)
    as_bad("Internal error dwarf2dbg_estimate_size_before_relax() called with "
	   "frag without symbol or expression\n");

  expression = (expressionS *)frag->fr_symbol->expression;
  if(expression->X_seg != SEG_DIFFSECT ||
     expression->X_add_symbol == NULL ||
     expression->X_subtract_symbol == NULL)
    as_bad("Internal error dwarf2dbg_estimate_size_before_relax() called with "
	   "frag symbol with bad expression\n");

  addr_delta =   (expression->X_add_symbol->sy_nlist.n_value +
                  expression->X_add_symbol->sy_frag->fr_address)
	       - (expression->X_subtract_symbol->sy_nlist.n_value +
		  expression->X_subtract_symbol->sy_frag->fr_address)
	       + expression->X_add_number;
#endif

  size = size_inc_line_addr (frag->fr_offset, addr_delta);

  frag->fr_subtype = size;

  return size;
}

/* This function relaxes a rs_dwarf2dbg variant frag based on the
   current values of the symbols.  fr_subtype is the current length
   of the frag.  This returns the change in frag length.  */

int
dwarf2dbg_relax_frag (fragS *frag)
{
  int old_size, new_size;

  old_size = frag->fr_subtype;
  new_size = dwarf2dbg_estimate_size_before_relax (frag);

  return new_size - old_size;
}

/* This function converts a rs_dwarf2dbg variant frag into a normal
   fill frag.  This is called after all relaxation has been done.
   fr_subtype will be the desired length of the frag.  */

void
dwarf2dbg_convert_frag (fragS *frag)
{
  offsetT addr_diff;
#ifndef OLD
  expressionS *expression;
#endif

#ifdef OLD
  addr_diff = resolve_symbol_value (frag->fr_symbol);
#else
  if(frag->fr_symbol == NULL || frag->fr_symbol->expression == NULL)
    as_bad("Internal error dwarf2dbg_convert_frag() called with "
	   "frag without symbol or expression\n");

  expression = (expressionS *)frag->fr_symbol->expression;
  if(expression->X_seg != SEG_DIFFSECT ||
     expression->X_add_symbol == NULL ||
     expression->X_subtract_symbol == NULL)
    as_bad("Internal error dwarf2dbg_convert_frag() called with "
	   "frag symbol with bad expression\n");

  addr_diff =   expression->X_add_symbol->sy_nlist.n_value
	      - expression->X_subtract_symbol->sy_nlist.n_value
	      + expression->X_add_number;
#endif

  /* fr_var carries the max_chars that we created the fragment with.
     fr_subtype carries the current expected length.  We must, of
     course, have allocated enough memory earlier.  */
  assert (frag->fr_var >= (int) frag->fr_subtype);

  emit_inc_line_addr (frag->fr_offset, addr_diff,
		      frag->fr_literal + frag->fr_fix, frag->fr_subtype);

  frag->fr_fix += frag->fr_subtype;
  frag->fr_type = rs_fill;
  frag->fr_var = 0;
  frag->fr_offset = 0;
}

/* Generate .debug_line content for the chain of line number entries
   beginning at E, for segment SEG.  */

static int
process_entries (segT seg, struct line_entry *e)
{
  unsigned filenum = 1;
  unsigned line = 1;
  unsigned column = 0;
  unsigned isa = 0;
  unsigned flags = DWARF2_LINE_DEFAULT_IS_STMT ? DWARF2_FLAG_IS_STMT : 0;
  fragS *last_frag = NULL, *frag;
  addressT last_frag_ofs = 0, frag_ofs;
  symbolS *last_lab = NULL, *lab;
  struct line_entry *next;
  int output_something = 0;

  do
    {
      int line_delta;

      if (filenum != e->loc.filenum)
	{
	  filenum = e->loc.filenum;
	  out_opcode (DW_LNS_set_file);
	  out_uleb128 (filenum);
	}

      if (column != e->loc.column)
	{
	  column = e->loc.column;
	  out_opcode (DW_LNS_set_column);
	  out_uleb128 (column);
	}

      if (isa != e->loc.isa)
	{
	  isa = e->loc.isa;
	  out_opcode (DW_LNS_set_isa);
	  out_uleb128 (isa);
	}

      if ((e->loc.flags ^ flags) & DWARF2_FLAG_IS_STMT)
	{
	  flags = e->loc.flags;
	  out_opcode (DW_LNS_negate_stmt);
	}

      if (e->loc.flags & DWARF2_FLAG_BASIC_BLOCK)
	out_opcode (DW_LNS_set_basic_block);

      if (e->loc.flags & DWARF2_FLAG_PROLOGUE_END)
	out_opcode (DW_LNS_set_prologue_end);

      if (e->loc.flags & DWARF2_FLAG_EPILOGUE_BEGIN)
	out_opcode (DW_LNS_set_epilogue_begin);

      /* Don't try to optimize away redundant entries; gdb wants two
	 entries for a function where the code starts on the same line as
	 the open curly brace, and there's no way to identify that case here.
	 Trust gcc to optimize appropriately.  */
      line_delta = e->loc.line - line;
      lab = e->label;
      frag = symbol_get_frag (lab);
#ifdef OLD
      frag_ofs = S_GET_VALUE (lab);
#else
      frag_ofs = lab->sy_nlist.n_value + frag->fr_address + frag->fr_offset;
#endif

      if (last_frag == NULL)
	{
	  out_set_addr (lab);
	  out_inc_line_addr (line_delta, 0);
	}
      else if (DWARF2_USE_FIXED_ADVANCE_PC)
	out_fixed_inc_line_addr (line_delta, lab, last_lab);
      else if (frag == last_frag)
	out_inc_line_addr (line_delta, frag_ofs - last_frag_ofs);
      else
	relax_inc_line_addr (line_delta, lab, last_lab);

      line = e->loc.line;
      last_lab = lab;
      last_frag = frag;
      last_frag_ofs = frag_ofs;

      next = e->next;
      free (e);
      e = next;
      output_something = 1;
    }
  while (e);

  /* Emit a DW_LNE_end_sequence for the end of the section.  */
  frag = last_frag_for_seg (seg);
  frag_ofs = get_frag_fix (frag, seg);
  if (DWARF2_USE_FIXED_ADVANCE_PC)
    {
      lab = symbol_temp_new (seg, frag_ofs, frag);
      out_fixed_inc_line_addr (INT_MAX, lab, last_lab);
    }
  else if (frag == last_frag)
    out_inc_line_addr (INT_MAX, frag_ofs - last_frag_ofs);
  else
    {
      lab = symbol_temp_new (seg, (valueT)frag_ofs, frag);
      relax_inc_line_addr (INT_MAX, lab, last_lab);
    }

  return (output_something);
}

/* Emit the directory and file tables for .debug_line.  */

static void
out_file_list (void)
{
  size_t size;
  char *cp;
  unsigned int i;

  /* Emit directory list.  */
  for (i = 1; i < dirs_in_use; ++i)
    {
      size = strlen (dirs[i]) + 1;
      cp = frag_more ((int)size);
      memcpy (cp, dirs[i], size);
    }
  /* Terminate it.  */
  out_byte ('\0');

  for (i = 1; i < files_in_use; ++i)
    {
      if (files[i].filename == NULL)
	{
	  as_bad (_("unassigned file number %ld"), (long) i);
	  /* Prevent a crash later, particularly for file 1.  */
	  files[i].filename = "";
	  continue;
	}

      size = strlen (files[i].filename) + 1;
      cp = frag_more ((int)size);
      memcpy (cp, files[i].filename, size);

      out_uleb128 (files[i].dir);	/* directory number */
      out_uleb128 (0);			/* last modification timestamp */
      out_uleb128 (0);			/* filesize */
    }

  /* Terminate filename list.  */
  out_byte (0);
}

/* Emit the collected .debug_line data.  */

static void
out_debug_line(
struct frchain *line_section)
{
  expressionS expr;
  symbolS *line_start;
  symbolS *prologue_end;
  symbolS *line_end;
  struct line_seg *s;
  enum dwarf2_format d2f;
  int sizeof_offset, output_something;

  sizeof_offset = 0;
  section_set(line_section);

  line_start = symbol_temp_new_now ();
  prologue_end = symbol_temp_make ();
  line_end = symbol_temp_make ();

  /* Total length of the information for this compilation unit.  */
  memset(&expr, '\0', sizeof(expr));
#ifdef OLD
  expr.X_op = O_subtract;
#else
  expr.X_seg = SEG_DIFFSECT;
#endif
  expr.X_add_symbol = line_end;
#ifdef OLD
  expr.X_op_symbol = line_start;
#else
  expr.X_subtract_symbol = line_start;
#endif

  d2f = DWARF2_FORMAT ();
  if (d2f == dwarf2_format_32bit)
    {
      expr.X_add_number = -4;
      emit_expr (&expr, 4);
      sizeof_offset = 4;
    }
  else if (d2f == dwarf2_format_64bit)
    {
      expr.X_add_number = -12;
      out_four (-1);
      emit_expr (&expr, 8);
      sizeof_offset = 8;
    }
  else if (d2f == dwarf2_format_64bit_irix)
    {
      expr.X_add_number = -8;
      emit_expr (&expr, 8);
      sizeof_offset = 8;
    }
  else
    {
      as_fatal (_("internal error: unknown dwarf2 format"));
    }

  /* Version.  */
  out_two (2);

  memset(&expr, '\0', sizeof(expr));
  /* Length of the prologue following this length.  */
#ifdef OLD
  expr.X_op = O_subtract;
#else
  expr.X_seg = SEG_DIFFSECT;
#endif
  expr.X_add_symbol = prologue_end;
#ifdef OLD
  expr.X_op_symbol = line_start;
#else
  expr.X_subtract_symbol = line_start;
#endif
  expr.X_add_number = - (4 + 2 + 4);
  emit_expr (&expr, sizeof_offset);

  /* Parameters of the state machine.  */
  out_byte (DWARF2_LINE_MIN_INSN_LENGTH);
  out_byte (DWARF2_LINE_DEFAULT_IS_STMT);
  out_byte (DWARF2_LINE_BASE);
  out_byte (DWARF2_LINE_RANGE);
  out_byte (DWARF2_LINE_OPCODE_BASE);

  /* Standard opcode lengths.  */
  out_byte (0);			/* DW_LNS_copy */
  out_byte (1);			/* DW_LNS_advance_pc */
  out_byte (1);			/* DW_LNS_advance_line */
  out_byte (1);			/* DW_LNS_set_file */
  out_byte (1);			/* DW_LNS_set_column */
  out_byte (0);			/* DW_LNS_negate_stmt */
  out_byte (0);			/* DW_LNS_set_basic_block */
  out_byte (0);			/* DW_LNS_const_add_pc */
  out_byte (1);			/* DW_LNS_fixed_advance_pc */
  out_byte (0);			/* DW_LNS_set_prologue_end */
  out_byte (0);			/* DW_LNS_set_epilogue_begin */
  out_byte (1);			/* DW_LNS_set_isa */

  out_file_list ();

  symbol_set_value_now (prologue_end);

  /* For each section, emit a statement program.  */
  output_something = 0;
  for (s = all_segs; s; s = s->next)
    output_something += process_entries (s->seg, s->head->head);

  /*
   * If we have not output anything the tools on Mac OS X need to have a
   * DW_LNE_set_address sequence to set the address to zero and a
   * DW_LNE_end_sequence which consists of 3 bytes '00 01 01'
   * (00 is the code for extended opcodes, followed by a ULEB128 length of the
   * extended opcode (01), and the DW_LNE_end_sequence (01).
   */
  if (output_something == 0)
    {
      /*
       * This is the DW_LNE_set_address sequence to set the address to zero.
       */
#ifdef ARCH64
      out_byte(0);
      out_byte(9);
      out_byte(2);
      out_byte(0);
      out_byte(0);
      out_byte(0);
      out_byte(0);
      out_byte(0);
      out_byte(0);
      out_byte(0);
      out_byte(0);
#else
      out_byte(0);
      out_byte(5);
      out_byte(2);
      out_byte(0);
      out_byte(0);
      out_byte(0);
      out_byte(0);
#endif
      out_byte(DW_LNS_extended_op);
      out_byte(1);
      out_byte(DW_LNE_end_sequence);
    }
  symbol_set_value_now (line_end);
}

static void
out_debug_ranges(
struct frchain *ranges_section)
{
  unsigned int addr_size = sizeof_address;
  struct line_seg *s;
  expressionS expr;
  unsigned int i;

  section_set(ranges_section);

  /* Base Address Entry.  */
  for (i = 0; i < addr_size; i++) 
    out_byte (0xff);
  for (i = 0; i < addr_size; i++) 
    out_byte (0);

  /* Range List Entry.  */
  for (s = all_segs; s; s = s->next)
    {
      fragS *frag;
      symbolS *beg, *end;

      frag = first_frag_for_seg (s->seg);
      beg = symbol_temp_new (s->seg, 0, frag);
      s->text_start = beg;

      frag = last_frag_for_seg (s->seg);
      end = symbol_temp_new (s->seg, (valueT)get_frag_fix (frag, s->seg), frag);
      s->text_end = end;

      memset(&expr, '\0', sizeof(expr));
      expr.X_op = O_symbol;
      expr.X_add_symbol = beg;
      expr.X_add_number = 0;
      emit_expr (&expr, addr_size);

      memset(&expr, '\0', sizeof(expr));
      expr.X_op = O_symbol;
      expr.X_add_symbol = end;
      expr.X_add_number = 0;
      emit_expr (&expr, addr_size);
    }

  /* End of Range Entry.   */
  for (i = 0; i < addr_size; i++) 
    out_byte (0);
  for (i = 0; i < addr_size; i++) 
    out_byte (0);
}

/* When generating debug info directly for assembly files, this routine is
   called to emit data for .debug_aranges and or set the text_end field which
   is used by out_debug_info() in the compile_unit.  */

static
void
out_debug_aranges(
struct frchain *aranges_section,
struct frchain *info_section)
{
  unsigned int addr_size = sizeof_address;
  addressT size, skip;
  struct line_seg *s;
  expressionS expr;
  char *p;

  size = 4 + 2 + 4 + 1 + 1;

  skip = 2 * addr_size - (size & (2 * addr_size - 1));
  if (skip == 2 * addr_size)
    skip = 0;
  size += skip;

  for (s = all_segs; s; s = s->next)
    size += 2 * addr_size;

  size += 2 * addr_size;

  section_set(aranges_section);

  /* Length of the compilation unit.  */
  out_four ((int)size - 4);

  /* Version.  */
  out_two (2);

  /* Offset to .debug_info.  */
  /* ??? sizeof_offset */
  TC_DWARF2_EMIT_OFFSET (section_symbol (info_section), 4);

  /* Size of an address (offset portion).  */
  out_byte (addr_size);

  /* Size of a segment descriptor.  */
  out_byte (0);

  /* Align the header.  */
  if (skip)
    {
      char fill;

      fill = '\0';
      frag_align (ffs (2 * addr_size) - 1, &fill, 1, 0);
    }

  for (s = all_segs; s; s = s->next)
    {
      fragS *frag;
      symbolS *beg, *end;

      frag = first_frag_for_seg (s->seg);
      beg = symbol_temp_new (s->seg, 0, frag);
      s->text_start = beg;

      frag = last_frag_for_seg (s->seg);
      end = symbol_temp_new (s->seg, (valueT)get_frag_fix (frag, s->seg), frag);
      s->text_end = end;

      memset(&expr, '\0', sizeof(expr));
      expr.X_op = O_symbol;
      expr.X_add_symbol = beg;
      expr.X_add_number = 0;
      emit_expr (&expr, addr_size);

      memset(&expr, '\0', sizeof(expr));
#ifdef OLD
      expr.X_op = O_subtract;
#else
      expr.X_seg = SEG_DIFFSECT;
#endif
      expr.X_add_symbol = end;
#ifdef OLD
      expr.X_op_symbol = beg;
#else
      expr.X_subtract_symbol = beg;
#endif
      expr.X_add_number = 0;
      emit_expr (&expr, addr_size);
    }
  p = frag_more (2 * addr_size);
  md_number_to_chars (p, 0, addr_size);
  md_number_to_chars (p + addr_size, 0, addr_size);
}

/* When generating debug info directly for assembly files, this routine is
   called to output the data for .debug_abbrev.  Note that this must be kept in
   sync with out_debug_info below.  */

static
void
out_debug_abbrev(
struct frchain * abbrev_section)
{
  section_set(abbrev_section);

  /* DW_TAG_compile_unit DIE abbrev (1) */
  out_uleb128 (1);
  out_uleb128 (DW_TAG_compile_unit);
  out_byte (DW_CHILDREN_yes);
  out_abbrev (DW_AT_stmt_list, DW_FORM_data4);
  if (all_segs->next == NULL)
    {
      out_abbrev (DW_AT_low_pc, DW_FORM_addr);
      out_abbrev (DW_AT_high_pc, DW_FORM_addr);
    }
  else
    {
      if (DWARF2_FORMAT () == dwarf2_format_32bit)
	out_abbrev (DW_AT_ranges, DW_FORM_data4);
// The goggles do nothing
//      else
//	out_abbrev (DW_AT_ranges, DW_FORM_data8);
    }
  out_abbrev (DW_AT_name, DW_FORM_string);
  out_abbrev (DW_AT_comp_dir, DW_FORM_string);
  if(apple_flags != NULL)
    out_abbrev (DW_AT_APPLE_flags, DW_FORM_string);
  out_abbrev (DW_AT_producer, DW_FORM_string);
  out_abbrev (DW_AT_language, DW_FORM_data2);
  out_abbrev (0, 0);

  /* DW_TAG_subprogram DIE abbrev (2) */
  out_uleb128 (2);
  out_uleb128 (DW_TAG_subprogram);
  out_byte (DW_CHILDREN_yes);
  out_abbrev (DW_AT_name, DW_FORM_string);
  out_abbrev (DW_AT_decl_file, DW_FORM_data4);
  out_abbrev (DW_AT_decl_line, DW_FORM_data4);
  out_abbrev (DW_AT_low_pc, DW_FORM_addr);
  out_abbrev (DW_AT_high_pc, DW_FORM_addr);
  out_abbrev (DW_AT_prototyped, DW_FORM_flag);
  out_abbrev (0, 0);

  /* DW_TAG_unspecified_parameters DIE abbrev (3) */
  out_uleb128 (3);
  out_uleb128 (DW_TAG_unspecified_parameters);
  out_byte (DW_CHILDREN_no);
  out_abbrev (0, 0);

  /* Terminate the abbreviations for this compilation unit.  */
  out_byte (0);
}

/* When generating debug info directly for assembly files, this routine is
   called to output the compilation unit and subprograms for .debug_info . */

static
void
out_debug_info(
struct frchain *info_section,
struct frchain *abbrev_section,
struct frchain *line_section,
struct frchain *ranges_section)
{
  char producer[128];
  char *comp_dir;
  expressionS expr;
  symbolS *info_start;
  symbolS *info_end;
  char *p;
  int len;
  enum dwarf2_format d2f;
  int sizeof_offset;
  struct dwarf2_subprogram_info *subs;
  symbolS *prev_subs_symbol;

  sizeof_offset = 0;
  section_set(info_section);

  info_start = symbol_temp_new_now ();
  info_end = symbol_temp_make ();

  /* Compilation Unit length.  */
  memset(&expr, '\0', sizeof(expr));
#ifdef OLD
  expr.X_op = O_subtract;
#else
  expr.X_seg = SEG_DIFFSECT;
#endif
  expr.X_add_symbol = info_end;
#ifdef OLD
  expr.X_op_symbol = info_start;
#else
  expr.X_subtract_symbol = info_start;
#endif

  d2f = DWARF2_FORMAT ();
  if (d2f == dwarf2_format_32bit)
    {
      expr.X_add_number = -4;
      emit_expr (&expr, 4);
      sizeof_offset = 4;
    }
  else if (d2f == dwarf2_format_64bit)
    {
      expr.X_add_number = -12;
      out_four (-1);
      emit_expr (&expr, 8);
      sizeof_offset = 8;
    }
  else if (d2f == dwarf2_format_64bit_irix)
    {
      expr.X_add_number = -8;
      emit_expr (&expr, 8);
      sizeof_offset = 8;
    }
  else
    {
      as_fatal (_("internal error: unknown dwarf2 format"));
    }

  /* DWARF version.  */
  out_two (2);

  /* .debug_abbrev offset */
  TC_DWARF2_EMIT_OFFSET (section_symbol (abbrev_section), sizeof_offset);

  /* Target address size.  */
  out_byte (sizeof_address);

  /* DW_TAG_compile_unit DIE abbrev */
  out_uleb128 (1);

  /* DW_AT_stmt_list */
  /* ??? sizeof_offset */
  TC_DWARF2_EMIT_OFFSET (section_symbol (line_section), 4);

  /* These two attributes are emitted if all of the code is contiguous.  */
  if (all_segs->next == NULL)
    {
      /* DW_AT_low_pc */
      memset(&expr, '\0', sizeof(expr));
      expr.X_op = O_symbol;
      expr.X_add_symbol = all_segs->text_start;
      expr.X_add_number = 0;
      emit_expr (&expr, sizeof_address);

      /* DW_AT_high_pc */
      memset(&expr, '\0', sizeof(expr));
      expr.X_op = O_symbol;
      expr.X_add_symbol = all_segs->text_end;
      expr.X_add_number = 0;
      emit_expr (&expr, sizeof_address);
    }
  else
    {
      /* This attribute is emitted if the code is disjoint.  */
      /* DW_AT_ranges.  */
      TC_DWARF2_EMIT_OFFSET (section_symbol (ranges_section), sizeof_offset);
    }

  /* DW_AT_name.  We don't have the actual file name that was present
     on the command line, so assume files[1] is the main input file.
     We're not supposed to get called unless at least one line number
     entry was emitted, so this should always be defined.  */
  if (!files || files_in_use < 1)
    abort ();
  if (files[1].dir)
    {
      len = (int)strlen (dirs[files[1].dir]);
      p = frag_more (len + 1);
      memcpy (p, dirs[files[1].dir], len);
      INSERT_DIR_SEPARATOR (p, len);
    }
  len = (int)strlen (files[1].filename) + 1;
  p = frag_more (len);
  memcpy (p, files[1].filename, len);

  /* DW_AT_comp_dir */
#ifdef OLD
  comp_dir = getpwd ();
#else
  comp_dir = getcwd(xmalloc(MAXPATHLEN + 1), MAXPATHLEN + 1); /* cctools-port: getwd() -> getcwd() */
#endif
  len = (int)strlen (comp_dir) + 1;
  p = frag_more (len);
  memcpy (p, comp_dir, len);

  if(apple_flags != NULL){
    /* DW_AT_APPLE_flags */
    len = (int)strlen (apple_flags) + 1;
    p = frag_more (len);
    memcpy (p, apple_flags, len);
  }

  /* DW_AT_producer */
  sprintf (producer, "%s %s, %s", APPLE_INC_VERSION, apple_version,
	   version_string);
  len = (int)strlen (producer) + 1;
  p = frag_more (len);
  memcpy (p, producer, len);

  /* DW_AT_language.  Yes, this is probably not really MIPS, but the
     dwarf2 draft has no standard code for assembler.  */
  out_two (DW_LANG_Mips_Assembler);

  prev_subs_symbol = NULL;
  for(subs = dwarf2_subprograms_info; subs != NULL; subs = subs->next){
    uint32_t n_sect;
    fragS *frag;
    symbolS *end;
    struct dwarf2_subprogram_info *subs_next;

    /* Don't emit zero length TAG_subprogram dwarf debug info. */
    if(prev_subs_symbol != NULL &&
       prev_subs_symbol->sy_frag == subs->symbol->sy_frag &&
       prev_subs_symbol->sy_value == subs->symbol->sy_value)
      {
        continue;
      }
    else
      {
        prev_subs_symbol = subs->symbol;
      }

    /* DW_TAG_subprogram DIE abbrev */
    out_uleb128 (2);

    /* DW_AT_name */
    len = (int)strlen (subs->name) + 1;
    p = frag_more (len);
    memcpy (p, subs->name, len);

    /* DW_AT_decl_file */
    out_four (subs->file_number);

    /* DW_AT_decl_line */
    out_four (subs->line_number);

    /* DW_AT_low_pc */
    memset(&expr, '\0', sizeof(expr));
    expr.X_op = O_symbol;
    expr.X_add_symbol = subs->symbol;
    expr.X_add_number = 0;
    emit_expr (&expr, sizeof_address);

    /* DW_AT_high_pc */
    end = NULL;
    if(subs->next != NULL)
      {
	end = subs->next->symbol;
	subs_next = subs->next;
        while(subs->symbol->sy_frag == end->sy_frag &&
              subs->symbol->sy_value == end->sy_value)
	  {
	  if(subs_next->next == NULL)
	    {
	      end = NULL;
	      break;
	    }
	  else
	    {
	      subs_next = subs_next->next;
	      end = subs_next->symbol;
	    }
	 }
      }
    if(end == NULL)
      {
        n_sect = subs->symbol->sy_nlist.n_sect;
        frag = last_frag_for_seg (n_sect);
        end = symbol_temp_new (n_sect, (valueT)get_frag_fix(frag,n_sect),frag);
      }
    memset(&expr, '\0', sizeof(expr));
    expr.X_op = O_symbol;
    expr.X_add_symbol = end;
    expr.X_add_number = 0;
    emit_expr (&expr, sizeof_address);

    /* DW_AT_prototyped */
    out_byte (0);

    /* DW_TAG_unspecified_parameters DIE abbrev */
    out_uleb128 (3);
    /* Add the NULL DIE terminating the DW_TAG_unspecified_parameters DIE's. */
    out_byte (0);
  }

  /* Add the NULL DIE terminating the Compile Unit DIE's.  */
  out_byte (0);

  symbol_set_value_now (info_end);
}

/* Finish the dwarf2 debug sections.  We emit .debug.line if there
   were any .file/.loc directives, or --gdwarf2 was given, or if the
   file has a non-empty .debug_info section.  If we emit .debug_line,
   and the .debug_info section is empty, we also emit .debug_info,
   .debug_aranges and .debug_abbrev.  ALL_SEGS will be non-null if
   there were any .file/.loc directives, or --gdwarf2 was given and
   there were any located instructions emitted.  */

void
dwarf2_finish (void)
{
#ifdef OLD
  segT info_seg;
  segT line_seg;
#else
  struct frchain *info_section, *line_section;
#endif
  struct line_seg *s;
  int emit_other_sections = 0;

#ifdef OLD
  info_seg = bfd_get_section_by_name (stdoutput, ".debug_info");
  emit_other_sections = info_seg == NULL || !seg_not_empty_p (info_seg);
#else
  info_section = get_section_by_name("__DWARF", "__debug_info");
  emit_other_sections = info_section == NULL || !seg_not_empty_p(info_section);
#endif

  if (!all_segs && emit_other_sections && files_in_use == 0)
    /* There is no line information and no non-empty .debug_info
       section.  */
    return;

  /* To make get_frag_fix() work we force what would happen in
     layout_addresses to add a last zero sized frag to each section.  */
  add_last_frags_to_sections();

  /* Calculate the size of an address for the target machine.  */
#ifdef OLD
  sizeof_address = DWARF2_ADDR_SIZE (stdoutput);
#else
  if(md_cputype & CPU_ARCH_ABI64)
    sizeof_address = 8;
  else
    sizeof_address = 4;
#endif

  /* Create and switch to the line number section.  */
#ifdef OLD
  line_seg = subseg_new (".debug_line", 0);
  bfd_set_section_flags (stdoutput, line_seg, SEC_READONLY | SEC_DEBUGGING);
#else
  line_section = section_new("__DWARF", "__debug_line",
			     S_REGULAR, S_ATTR_DEBUG, 0);
#endif

  /* For each subsection, chain the debug entries together.  */
  for (s = all_segs; s; s = s->next)
    {
      struct line_subseg *ss = s->head;
      struct line_entry **ptail = ss->ptail;

      while ((ss = ss->next) != NULL)
	{
	  *ptail = ss->head;
	  ptail = ss->ptail;
	}
    }

#ifdef OLD
  out_debug_line (line_seg);
#else
  /* If might be possible that an older compiler is not using .file and .loc
     in that case make sure not to put out a second empty line table.  But it
     is possible only .file is used and in that case put out the line table.  */
  if(all_segs != NULL || files_in_use)
    out_debug_line (line_section);
#endif

  /* If this is assembler generated line info, and there is no
     debug_info already, we need .debug_info and .debug_abbrev
     sections as well.  */
  if (emit_other_sections)
    {
#ifdef OLD
      segT abbrev_seg;
      segT aranges_seg;
      segT ranges_seg;
#else
      struct frchain *abbrev_section, *aranges_section, *ranges_section;
#endif

      assert (all_segs);
  
#ifdef OLD
      info_seg = subseg_new (".debug_info", 0);
      abbrev_seg = subseg_new (".debug_abbrev", 0);
      aranges_seg = subseg_new (".debug_aranges", 0);

      bfd_set_section_flags (stdoutput, info_seg,
			     SEC_READONLY | SEC_DEBUGGING);
      bfd_set_section_flags (stdoutput, abbrev_seg,
			     SEC_READONLY | SEC_DEBUGGING);
      bfd_set_section_flags (stdoutput, aranges_seg,
			     SEC_READONLY | SEC_DEBUGGING);

      record_alignment (aranges_seg, ffs (2 * sizeof_address) - 1);
#else
      info_section = section_new("__DWARF", "__debug_info",
				 S_REGULAR, S_ATTR_DEBUG, 0);
      abbrev_section = section_new("__DWARF", "__debug_abbrev",
				   S_REGULAR, S_ATTR_DEBUG, 0);
      aranges_section = section_new("__DWARF", "__debug_aranges",
				    S_REGULAR, S_ATTR_DEBUG, 0);
      /*
       * The alignment of the Mach-O (__DWARF, __debug_aranges) is the default
       * 1-byte alignment, 2^0, so the aranges_section->frch_section->align
       * field should remain zero and not changed.
       */
#endif

      if (all_segs == NULL || all_segs->next == NULL)
#ifdef OLD
	ranges_seg = NULL;
#else
	ranges_section = NULL;
#endif
      else
	{
#ifdef OLD
	  ranges_seg = subseg_new (".debug_ranges", 0);
	  bfd_set_section_flags (stdoutput, ranges_seg, 
				 SEC_READONLY | SEC_DEBUGGING);
	  record_alignment (ranges_seg, ffs (2 * sizeof_address) - 1);
	  out_debug_ranges (ranges_seg);
#else
	  ranges_section = section_new("__DWARF", "__debug_ranges",
				       S_REGULAR, S_ATTR_DEBUG, 0);
	  /*
	   * The alignment of the Mach-O (__DWARF, __debug_ranges) is the
	   * default 1-byte alignment, 2^0, so the
	   * ranges_section->frch_section->align field should remain zero
	   * and not changed.
	   */
	  out_debug_ranges (ranges_section);
#endif
	}

#ifdef OLD
      out_debug_aranges (aranges_seg, info_seg);
      out_debug_abbrev (abbrev_seg);
      out_debug_info (info_seg, abbrev_seg, line_seg, ranges_seg);
#else
      if (all_segs != NULL &&
	  (all_segs->next != NULL || debug_type == DEBUG_DWARF2))
	{
	  out_debug_aranges (aranges_section, info_section);
	  out_debug_abbrev (abbrev_section);
	  out_debug_info (info_section, abbrev_section, line_section,
			  ranges_section);
	}
#endif
    }
}

/*
 * macho_dwarf2_emit_offset() is used via the TC_DWARF2_EMIT_OFFSET macro to
 * output an "offset".  Where it is used the offset is to the start of
 * information in a specific section for a field in another section.  This
 * appears to always be passed the section_symbol() for the specific section
 * and that offset is always to the start of that section. So this is why
 * we put out zeros.
 */
static void
macho_dwarf2_emit_offset(
symbolS *symbol,
unsigned int size)
{
    unsigned int i;

	for(i = 0; i < size; i++)
	    out_byte(0);
}
