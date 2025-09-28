/* dwarf2dbg.c - DWARF2 debug support
   Copyright (C) 1999-2023 Free Software Foundation, Inc.
   Contributed by David Mosberger-Tang <davidm@hpl.hp.com>

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

/* Logical line numbers can be controlled by the compiler via the
   following directives:

	.file FILENO "file.c"
	.loc  FILENO LINENO [COLUMN] [basic_block] [prologue_end] \
	      [epilogue_begin] [is_stmt VALUE] [isa VALUE] \
	      [discriminator VALUE] [view VALUE]
*/

#include "as.h"
#include "safe-ctype.h"
#include <limits.h>
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
# define DWARF2_FORMAT(SEC) dwarf2_format_32bit
#endif

#ifndef DWARF2_ADDR_SIZE
# define DWARF2_ADDR_SIZE(bfd) (bfd_arch_bits_per_address (bfd) / 8)
#endif

#ifndef DWARF2_FILE_NAME
#define DWARF2_FILE_NAME(FILENAME, DIRNAME) FILENAME
#endif

#ifndef DWARF2_FILE_TIME_NAME
#define DWARF2_FILE_TIME_NAME(FILENAME,DIRNAME) -1
#endif

#ifndef DWARF2_FILE_SIZE_NAME
#define DWARF2_FILE_SIZE_NAME(FILENAME,DIRNAME) -1
#endif

#ifndef DWARF2_VERSION
#define DWARF2_VERSION dwarf_level
#endif

/* The .debug_aranges version has been 2 in DWARF version 2, 3 and 4. */
#ifndef DWARF2_ARANGES_VERSION
#define DWARF2_ARANGES_VERSION 2
#endif

/* The .debug_line version is the same as the .debug_info version.  */
#ifndef DWARF2_LINE_VERSION
#define DWARF2_LINE_VERSION DWARF2_VERSION
#endif

/* The .debug_rnglists has only been in DWARF version 5. */
#ifndef DWARF2_RNGLISTS_VERSION
#define DWARF2_RNGLISTS_VERSION 5
#endif

#include "subsegs.h"

#include "dwarf2.h"

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
# define DWARF2_USE_FIXED_ADVANCE_PC	linkrelax
#endif

/* First special line opcode - leave room for the standard opcodes.
   Note: If you want to change this, you'll have to update the
   "standard_opcode_lengths" table that is emitted below in
   out_debug_line().  */
#define DWARF2_LINE_OPCODE_BASE		(DWARF2_LINE_VERSION == 2 ? 10 : 13)

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

#ifndef DWARF2_LINE_MAX_OPS_PER_INSN
#define DWARF2_LINE_MAX_OPS_PER_INSN	1
#endif

/* Given a special op, return the line skip amount.  */
#define SPECIAL_LINE(op) \
	(((op) - DWARF2_LINE_OPCODE_BASE)%DWARF2_LINE_RANGE + DWARF2_LINE_BASE)

/* Given a special op, return the address skip amount (in units of
   DWARF2_LINE_MIN_INSN_LENGTH.  */
#define SPECIAL_ADDR(op) (((op) - DWARF2_LINE_OPCODE_BASE)/DWARF2_LINE_RANGE)

/* The maximum address skip amount that can be encoded with a special op.  */
#define MAX_SPECIAL_ADDR_DELTA		SPECIAL_ADDR(255)

#ifndef TC_PARSE_CONS_RETURN_NONE
#define TC_PARSE_CONS_RETURN_NONE BFD_RELOC_NONE
#endif

#define GAS_ABBREV_COMP_UNIT 1
#define GAS_ABBREV_SUBPROG   2
#define GAS_ABBREV_NO_TYPE   3

struct line_entry
{
  struct line_entry *next;
  symbolS *label;
  struct dwarf2_line_info loc;
};

/* Don't change the offset of next in line_entry.  set_or_check_view
   calls in dwarf2_gen_line_info_1 depend on it.  */
static char unused[offsetof(struct line_entry, next) ? -1 : 1]
ATTRIBUTE_UNUSED;

struct line_subseg
{
  struct line_subseg *next;
  subsegT subseg;
  struct line_entry *head;
  struct line_entry **ptail;
  struct line_entry **pmove_tail;
};

struct line_seg
{
  struct line_seg *next;
  segT seg;
  struct line_subseg *head;
  symbolS *text_start;
  symbolS *text_end;
};

/* Collects data for all line table entries during assembly.  */
static struct line_seg *all_segs;
static struct line_seg **last_seg_ptr;

#define NUM_MD5_BYTES       16

struct file_entry
{
  const char *   filename;
  unsigned int   dir;
  unsigned char  md5[NUM_MD5_BYTES];
};

/* Table of files used by .debug_line.  */
static struct file_entry *files;
static unsigned int files_in_use;
static unsigned int files_allocated;

/* Table of directories used by .debug_line.  */
static char **       dirs;
static unsigned int  dirs_in_use;
static unsigned int  dirs_allocated;

/* TRUE when we've seen a .loc directive recently.  Used to avoid
   doing work when there's nothing to do.  Will be reset by
   dwarf2_consume_line_info.  */
bool dwarf2_loc_directive_seen;

/* TRUE when we've seen any .loc directive at any time during parsing.
   Indicates the user wants us to generate a .debug_line section.
   Used in dwarf2_finish as sanity check.  */
static bool dwarf2_any_loc_directive_seen;

/* TRUE when we're supposed to set the basic block mark whenever a
   label is seen.  */
bool dwarf2_loc_mark_labels;

/* Current location as indicated by the most recent .loc directive.  */
static struct dwarf2_line_info current;

/* This symbol is used to recognize view number forced resets in loc
   lists.  */
static symbolS *force_reset_view;

/* This symbol evaluates to an expression that, if nonzero, indicates
   some view assert check failed.  */
static symbolS *view_assert_failed;

/* The size of an address on the target.  */
static unsigned int sizeof_address;

#ifndef TC_DWARF2_EMIT_OFFSET
#define TC_DWARF2_EMIT_OFFSET  generic_dwarf2_emit_offset

/* Create an offset to .dwarf2_*.  */

static void
generic_dwarf2_emit_offset (symbolS *symbol, unsigned int size)
{
  expressionS exp;

  memset (&exp, 0, sizeof exp);
  exp.X_op = O_symbol;
  exp.X_add_symbol = symbol;
  exp.X_add_number = 0;
  emit_expr (&exp, size);
}
#endif

/* Find or create (if CREATE_P) an entry for SEG+SUBSEG in ALL_SEGS.  */

static struct line_subseg *
get_line_subseg (segT seg, subsegT subseg, bool create_p)
{
  struct line_seg *s = seg_info (seg)->dwarf2_line_seg;
  struct line_subseg **pss, *lss;

  if (s == NULL)
    {
      if (!create_p)
	return NULL;

      s = XNEW (struct line_seg);
      s->next = NULL;
      s->seg = seg;
      s->head = NULL;
      *last_seg_ptr = s;
      last_seg_ptr = &s->next;
      seg_info (seg)->dwarf2_line_seg = s;
    }

  gas_assert (seg == s->seg);

  for (pss = &s->head; (lss = *pss) != NULL ; pss = &lss->next)
    {
      if (lss->subseg == subseg)
	goto found_subseg;
      if (lss->subseg > subseg)
	break;
    }

  lss = XNEW (struct line_subseg);
  lss->next = *pss;
  lss->subseg = subseg;
  lss->head = NULL;
  lss->ptail = &lss->head;
  lss->pmove_tail = &lss->head;
  *pss = lss;

 found_subseg:
  return lss;
}

/* (Un)reverse the line_entry list starting from H.  */

static struct line_entry *
reverse_line_entry_list (struct line_entry *h)
{
  struct line_entry *p = NULL, *e, *n;

  for (e = h; e; e = n)
    {
      n = e->next;
      e->next = p;
      p = e;
    }
  return p;
}

/* Compute the view for E based on the previous entry P.  If we
   introduce an (undefined) view symbol for P, and H is given (P must
   be the tail in this case), introduce view symbols for earlier list
   entries as well, until one of them is constant.  */

static void
set_or_check_view (struct line_entry *e, struct line_entry *p,
		   struct line_entry *h)
{
  expressionS viewx;

  memset (&viewx, 0, sizeof (viewx));
  viewx.X_unsigned = 1;

  /* First, compute !(E->label > P->label), to tell whether or not
     we're to reset the view number.  If we can't resolve it to a
     constant, keep it symbolic.  */
  if (!p || (e->loc.u.view == force_reset_view && force_reset_view))
    {
      viewx.X_op = O_constant;
      viewx.X_add_number = 0;
      viewx.X_add_symbol = NULL;
      viewx.X_op_symbol = NULL;
    }
  else
    {
      viewx.X_op = O_gt;
      viewx.X_add_number = 0;
      viewx.X_add_symbol = e->label;
      viewx.X_op_symbol = p->label;
      resolve_expression (&viewx);
      if (viewx.X_op == O_constant)
	viewx.X_add_number = !viewx.X_add_number;
      else
	{
	  viewx.X_add_symbol = make_expr_symbol (&viewx);
	  viewx.X_add_number = 0;
	  viewx.X_op_symbol = NULL;
	  viewx.X_op = O_logical_not;
	}
    }

  if (S_IS_DEFINED (e->loc.u.view) && symbol_constant_p (e->loc.u.view))
    {
      expressionS *value = symbol_get_value_expression (e->loc.u.view);
      /* We can't compare the view numbers at this point, because in
	 VIEWX we've only determined whether we're to reset it so
	 far.  */
      if (viewx.X_op == O_constant)
	{
	  if (!value->X_add_number != !viewx.X_add_number)
	    as_bad (_("view number mismatch"));
	}
      /* Record the expression to check it later.  It is the result of
	 a logical not, thus 0 or 1.  We just add up all such deferred
	 expressions, and resolve it at the end.  */
      else if (!value->X_add_number)
	{
	  symbolS *deferred = make_expr_symbol (&viewx);
	  if (view_assert_failed)
	    {
	      expressionS chk;

	      memset (&chk, 0, sizeof (chk));
	      chk.X_unsigned = 1;
	      chk.X_op = O_add;
	      chk.X_add_number = 0;
	      chk.X_add_symbol = view_assert_failed;
	      chk.X_op_symbol = deferred;
	      deferred = make_expr_symbol (&chk);
	    }
	  view_assert_failed = deferred;
	}
    }

  if (viewx.X_op != O_constant || viewx.X_add_number)
    {
      expressionS incv;
      expressionS *p_view;

      if (!p->loc.u.view)
	p->loc.u.view = symbol_temp_make ();

      memset (&incv, 0, sizeof (incv));
      incv.X_unsigned = 1;
      incv.X_op = O_symbol;
      incv.X_add_symbol = p->loc.u.view;
      incv.X_add_number = 1;
      p_view = symbol_get_value_expression (p->loc.u.view);
      if (p_view->X_op == O_constant || p_view->X_op == O_symbol)
	{
	  /* If we can, constant fold increments so that a chain of
	     expressions v + 1 + 1 ... + 1 is not created.
	     resolve_expression isn't ideal for this purpose.  The
	     base v might not be resolvable until later.  */
	  incv.X_op = p_view->X_op;
	  incv.X_add_symbol = p_view->X_add_symbol;
	  incv.X_add_number = p_view->X_add_number + 1;
	}

      if (viewx.X_op == O_constant)
	{
	  gas_assert (viewx.X_add_number == 1);
	  viewx = incv;
	}
      else
	{
	  viewx.X_add_symbol = make_expr_symbol (&viewx);
	  viewx.X_add_number = 0;
	  viewx.X_op_symbol = make_expr_symbol (&incv);
	  viewx.X_op = O_multiply;
	}
    }

  if (!S_IS_DEFINED (e->loc.u.view))
    {
      symbol_set_value_expression (e->loc.u.view, &viewx);
      S_SET_SEGMENT (e->loc.u.view, expr_section);
      symbol_set_frag (e->loc.u.view, &zero_address_frag);
    }

  /* Define and attempt to simplify any earlier views needed to
     compute E's.  */
  if (h && p && p->loc.u.view && !S_IS_DEFINED (p->loc.u.view))
    {
      struct line_entry *h2;
      /* Reverse the list to avoid quadratic behavior going backwards
	 in a single-linked list.  */
      struct line_entry *r = reverse_line_entry_list (h);

      gas_assert (r == p);
      /* Set or check views until we find a defined or absent view.  */
      do
	{
	  /* Do not define the head of a (sub?)segment view while
	     handling others.  It would be defined too early, without
	     regard to the last view of other subsegments.
	     set_or_check_view will be called for every head segment
	     that needs it.  */
	  if (r == h)
	    break;
	  set_or_check_view (r, r->next, NULL);
	}
      while (r->next
	     && r->next->loc.u.view
	     && !S_IS_DEFINED (r->next->loc.u.view)
	     && (r = r->next));

      /* Unreverse the list, so that we can go forward again.  */
      h2 = reverse_line_entry_list (p);
      gas_assert (h2 == h);

      /* Starting from the last view we just defined, attempt to
	 simplify the view expressions, until we do so to P.  */
      do
	{
	  /* The head view of a subsegment may remain undefined while
	     handling other elements, before it is linked to the last
	     view of the previous subsegment.  */
	  if (r == h)
	    continue;
	  gas_assert (S_IS_DEFINED (r->loc.u.view));
	  resolve_expression (symbol_get_value_expression (r->loc.u.view));
	}
      while (r != p && (r = r->next));

      /* Now that we've defined and computed all earlier views that might
	 be needed to compute E's, attempt to simplify it.  */
      resolve_expression (symbol_get_value_expression (e->loc.u.view));
    }
}

/* Record an entry for LOC occurring at LABEL.  */

static void
dwarf2_gen_line_info_1 (symbolS *label, struct dwarf2_line_info *loc)
{
  struct line_subseg *lss;
  struct line_entry *e;
  flagword need_flags = SEC_LOAD | SEC_CODE;

  /* PR 26850: Do not record LOCs in non-executable or non-loaded
     sections.  SEC_ALLOC isn't tested for non-ELF because obj-coff.c
     obj_coff_section is careless in setting SEC_ALLOC.  */
  if (IS_ELF)
    need_flags |= SEC_ALLOC;
  if ((now_seg->flags & need_flags) != need_flags)
    {
      /* FIXME: Add code to suppress multiple warnings ?  */
      if (debug_type != DEBUG_DWARF2)
	as_warn ("dwarf line number information for %s ignored",
		 segment_name (now_seg));
      return;
    }

  e = XNEW (struct line_entry);
  e->next = NULL;
  e->label = label;
  e->loc = *loc;

  lss = get_line_subseg (now_seg, now_subseg, true);

  /* Subseg heads are chained to previous subsegs in
     dwarf2_finish.  */
  if (loc->filenum != -1u && loc->u.view && lss->head)
    set_or_check_view (e, (struct line_entry *) lss->ptail, lss->head);

  *lss->ptail = e;
  lss->ptail = &e->next;
}

/* Record an entry for LOC occurring at OFS within the current fragment.  */

static unsigned int dw2_line;
static const char *dw2_filename;
static int label_num;

void
dwarf2_gen_line_info (addressT ofs, struct dwarf2_line_info *loc)
{
  symbolS *sym;

  /* Early out for as-yet incomplete location information.  */
  if (loc->line == 0)
    return;
  if (loc->filenum == 0)
    {
      if (dwarf_level < 5)
	dwarf_level = 5;
      if (DWARF2_LINE_VERSION < 5)
	return;
    }

  /* Don't emit sequences of line symbols for the same line when the
     symbols apply to assembler code.  It is necessary to emit
     duplicate line symbols when a compiler asks for them, because GDB
     uses them to determine the end of the prologue.  */
  if (debug_type == DEBUG_DWARF2)
    {
      if (dw2_line == loc->line)
	{
	  if (dw2_filename == loc->u.filename)
	    return;
	  if (filename_cmp (dw2_filename, loc->u.filename) == 0)
	    {
	      dw2_filename = loc->u.filename;
	      return;
	    }
	}

      dw2_line = loc->line;
      dw2_filename = loc->u.filename;
    }

  if (linkrelax)
    {
      char name[32];

      /* Use a non-fake name for the line number location,
	 so that it can be referred to by relocations.  */
      sprintf (name, ".Loc.%u", label_num);
      label_num++;
      sym = symbol_new (name, now_seg, frag_now, ofs);
    }
  else
    sym = symbol_temp_new (now_seg, frag_now, ofs);
  dwarf2_gen_line_info_1 (sym, loc);
}

static const char *
get_basename (const char * pathname)
{
  const char * file;

  file = lbasename (pathname);
  /* Don't make empty string from / or A: from A:/ .  */
#ifdef HAVE_DOS_BASED_FILE_SYSTEM
  if (file <= pathname + 3)
    file = pathname;
#else
  if (file == pathname + 1)
    file = pathname;
#endif
  return file;
}

static unsigned int
get_directory_table_entry (const char *dirname,
			   const char *file0_dirname,
			   size_t dirlen,
			   bool can_use_zero)
{
  unsigned int d;

  if (dirlen == 0)
    return 0;

#ifndef DWARF2_DIR_SHOULD_END_WITH_SEPARATOR
  if (IS_DIR_SEPARATOR (dirname[dirlen - 1]))
    {
      -- dirlen;
      if (dirlen == 0)
	return 0;
    }
#endif

  for (d = 0; d < dirs_in_use; ++d)
    {
      if (dirs[d] != NULL
	  && filename_ncmp (dirname, dirs[d], dirlen) == 0
	  && dirs[d][dirlen] == '\0')
	return d;
    }

  if (can_use_zero)
    {
      if (dirs == NULL || dirs[0] == NULL)
	{
	  const char * pwd = file0_dirname ? file0_dirname : getpwd ();

	  if (dwarf_level >= 5 && filename_cmp (dirname, pwd) != 0)
	    {
	      /* In DWARF-5 the 0 entry in the directory table is
		 expected to be the same as the DW_AT_comp_dir (which
		 is set to the current build directory).  Since we are
		 about to create a directory entry that is not the
		 same, allocate the current directory first.  */
	      (void) get_directory_table_entry (pwd, file0_dirname,
						strlen (pwd), true);
	      d = 1;
	    }
	  else
	    d = 0;
	}
    }
  else if (d == 0)
    d = 1;

  if (d >= dirs_allocated)
    {
      unsigned int old = dirs_allocated;
#define DIR_TABLE_INCREMENT 32
      dirs_allocated = d + DIR_TABLE_INCREMENT;
      dirs = XRESIZEVEC (char *, dirs, dirs_allocated);
      memset (dirs + old, 0, (dirs_allocated - old) * sizeof (char *));
    }

  dirs[d] = xmemdup0 (dirname, dirlen);
  if (dirs_in_use <= d)
    dirs_in_use = d + 1;

  return d;  
}

static bool
assign_file_to_slot (unsigned int i, const char *file, unsigned int dir)
{
  if (i >= files_allocated)
    {
      unsigned int want = i + 32;

      /* Catch wraparound.  */
      if (want < files_allocated
	  || want < i
	  || want > UINT_MAX / sizeof (struct file_entry))
	{
	  as_bad (_("file number %u is too big"), i);
	  return false;
	}

      files = XRESIZEVEC (struct file_entry, files, want);
      memset (files + files_allocated, 0,
	      (want - files_allocated) * sizeof (struct file_entry));
      files_allocated = want;
    }

  files[i].filename = file;
  files[i].dir = dir;
  memset (files[i].md5, 0, NUM_MD5_BYTES);

  if (files_in_use < i + 1)
    files_in_use = i + 1;

  return true;
}

/* Get a .debug_line file number for PATHNAME.  If there is a
   directory component to PATHNAME, then this will be stored
   in the directory table, if it is not already present.
   Returns the slot number allocated to that filename or -1
   if there was a problem.  */

static int last_used;
static int last_used_dir_len;

static signed int
allocate_filenum (const char * pathname)
{
  const char *file;
  size_t dir_len;
  unsigned int i, dir;

  /* Short circuit the common case of adding the same pathname
     as last time.  */
  if (last_used != -1)
    {
      const char * dirname = NULL;

      if (dirs != NULL)
	dirname = dirs[files[last_used].dir];

      if (dirname == NULL)
	{
	  if (filename_cmp (pathname, files[last_used].filename) == 0)
	    return last_used;
	}
      else
	{
	  if (filename_ncmp (pathname, dirname, last_used_dir_len - 1) == 0
	      && IS_DIR_SEPARATOR (pathname [last_used_dir_len - 1])
	      && filename_cmp (pathname + last_used_dir_len,
			       files[last_used].filename) == 0)
	    return last_used;
	}
    }

  file = get_basename (pathname);
  dir_len = file - pathname;

  dir = get_directory_table_entry (pathname, NULL, dir_len, false);

  /* Do not use slot-0.  That is specifically reserved for use by
     the '.file 0 "name"' directive.  */
  for (i = 1; i < files_in_use; ++i)
    if (files[i].dir == dir
	&& files[i].filename
	&& filename_cmp (file, files[i].filename) == 0)
      {
	last_used = i;
	last_used_dir_len = dir_len;
	return i;
      }

  if (!assign_file_to_slot (i, file, dir))
    return -1;

  last_used = i;
  last_used_dir_len = dir_len;

  return i;
}

/* Run through the list of line entries starting at E, allocating
   file entries for gas generated debug.  */

static void
do_allocate_filenum (struct line_entry *e)
{
  do
    {
      if (e->loc.filenum == -1u)
	{
	  e->loc.filenum = allocate_filenum (e->loc.u.filename);
	  e->loc.u.view = NULL;
	}
      e = e->next;
    }
  while (e);
}

/* Remove any generated line entries.  These don't live comfortably
   with compiler generated line info.  If THELOT then remove
   everything, freeing all list entries we have created.  */

static void
purge_generated_debug (bool thelot)
{
  struct line_seg *s, *nexts;

  for (s = all_segs; s; s = nexts)
    {
      struct line_subseg *lss, *nextlss;

      for (lss = s->head; lss; lss = nextlss)
	{
	  struct line_entry *e, *next;

	  for (e = lss->head; e; e = next)
	    {
	      if (!thelot)
		know (e->loc.filenum == -1u);
	      next = e->next;
	      free (e);
	    }

	  lss->head = NULL;
	  lss->ptail = &lss->head;
	  lss->pmove_tail = &lss->head;
	  nextlss = lss->next;
	  if (thelot)
	    free (lss);
	}
      nexts = s->next;
      if (thelot)
	{
	  seg_info (s->seg)->dwarf2_line_seg = NULL;
	  free (s);
	}
    }
}

/* Allocate slot NUM in the .debug_line file table to FILENAME.
   If DIRNAME is not NULL or there is a directory component to FILENAME
   then this will be stored in the directory table, if not already present.
   if WITH_MD5 is TRUE then there is a md5 value in generic_bignum.
   Returns TRUE if allocation succeeded, FALSE otherwise.  */

static bool
allocate_filename_to_slot (const char *dirname,
			   const char *filename,
			   unsigned int num,
			   bool with_md5)
{
  const char *file;
  size_t dirlen;
  unsigned int i, d;
  const char *file0_dirname;

  /* Short circuit the common case of adding the same pathname
     as last time.  */
  if (num < files_allocated && files[num].filename != NULL)
    {
      const char * dir = NULL;

      if (dirs != NULL)
	dir = dirs[files[num].dir];

      if (with_md5
	  && memcmp (generic_bignum, files[num].md5, NUM_MD5_BYTES) != 0)
	goto fail;

      if (dirname != NULL)
	{
	  if (dir != NULL && filename_cmp (dir, dirname) != 0)
	    goto fail;
      
	  if (filename_cmp (filename, files[num].filename) != 0)
	    goto fail;

	  /* If the filenames match, but the directory table entry was
	     empty, then fill it with the provided directory name.  */
	  if (dir == NULL)
	    {
	      if (dirs == NULL)
		{
		  dirs_allocated = files[num].dir + DIR_TABLE_INCREMENT;
		  dirs = XCNEWVEC (char *, dirs_allocated);
		}
	      
	      dirs[files[num].dir] = xmemdup0 (dirname, strlen (dirname));
	    }
	    
	  return true;
	}
      else if (dir != NULL) 
	{
	  dirlen = strlen (dir);
	  if (filename_ncmp (filename, dir, dirlen) == 0
	      && IS_DIR_SEPARATOR (filename [dirlen])
	      && filename_cmp (filename + dirlen + 1, files[num].filename) == 0)
	    return true;
	}
      else /* dir == NULL  */
	{
	  file = get_basename (filename);
	  if (filename_cmp (file, files[num].filename) == 0)
	    {
	      /* The filenames match, but the directory table entry is empty.
		 Fill it with the provided directory name.  */
	      if (file > filename)
		{
		  if (dirs == NULL)
		    {
		      dirs_allocated = files[num].dir + DIR_TABLE_INCREMENT;
		      dirs = XCNEWVEC (char *, dirs_allocated);
		    }

		  dirs[files[num].dir] = xmemdup0 (filename, file - filename);
		}
	      return true;
	    }
	}

    fail:
      as_bad (_("file table slot %u is already occupied by a different file (%s%s%s vs %s%s%s)"),
	      num,
	      dir == NULL ? "" : dir,
	      dir == NULL ? "" : "/",
	      files[num].filename,
	      dirname == NULL ? "" : dirname,
	      dirname == NULL ? "" : "/",
	      filename);
      return false;
    }

  /* For file .0, the directory name is the current directory and the file
     may be in another directory contained in the file name.  */
  if (num == 0)
    {
      file0_dirname = dirname;

      file = get_basename (filename);

      if (dirname && file == filename)
	dirlen = strlen (dirname);
      else
	{
	  dirname = filename;
	  dirlen = file - filename;
	}
    }
  else
    {
      file0_dirname = NULL;

      if (dirname == NULL)
	{
	  dirname = filename;
	  file = get_basename (filename);
	  dirlen = file - filename;
	}
      else
	{
	  dirlen = strlen (dirname);
	  file = filename;
	}
    }

  d = get_directory_table_entry (dirname, file0_dirname, dirlen, num == 0);
  i = num;

  if (! assign_file_to_slot (i, file, d))
    return false;

  if (with_md5)
    {
      if (target_big_endian)
	{
	  /* md5's are stored in litte endian format.  */
	  unsigned int     bits_remaining = NUM_MD5_BYTES * BITS_PER_CHAR;
	  unsigned int     byte = NUM_MD5_BYTES;
	  unsigned int     bignum_index = 0;

	  while (bits_remaining)
	    {
	      unsigned int bignum_bits_remaining = LITTLENUM_NUMBER_OF_BITS;
	      valueT       bignum_value = generic_bignum [bignum_index];
	      bignum_index ++;

	      while (bignum_bits_remaining)
		{
		  files[i].md5[--byte] = bignum_value & 0xff;
		  bignum_value >>= 8;
		  bignum_bits_remaining -= 8;
		  bits_remaining -= 8;
		}
	    }
	}
      else
	{
	  unsigned int     bits_remaining = NUM_MD5_BYTES * BITS_PER_CHAR;
	  unsigned int     byte = 0;
	  unsigned int     bignum_index = 0;

	  while (bits_remaining)
	    {
	      unsigned int bignum_bits_remaining = LITTLENUM_NUMBER_OF_BITS;
	      valueT       bignum_value = generic_bignum [bignum_index];

	      bignum_index ++;

	      while (bignum_bits_remaining)
		{
		  files[i].md5[byte++] = bignum_value & 0xff;
		  bignum_value >>= 8;
		  bignum_bits_remaining -= 8;
		  bits_remaining -= 8;
		}
	    }
	}
    }
  else
    memset (files[i].md5, 0, NUM_MD5_BYTES);

  return true;
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
      line->u.filename = as_where (&line->line);
      line->filenum = -1u;
      line->column = 0;
      line->flags = DWARF2_FLAG_IS_STMT;
      line->isa = current.isa;
      line->discriminator = current.discriminator;
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

  if (debug_type != DEBUG_DWARF2
      ? !dwarf2_loc_directive_seen
      : !seen_at_least_1_file ())
    return;

  dwarf2_where (&loc);

  dwarf2_gen_line_info ((frag_now_fix_octets () - size) / OCTETS_PER_BYTE, &loc);
  dwarf2_consume_line_info ();
}

/* Move all previously-emitted line entries for the current position by
   DELTA bytes.  This function cannot be used to move the same entries
   twice.  */

void
dwarf2_move_insn (int delta)
{
  struct line_subseg *lss;
  struct line_entry *e;
  valueT now;

  if (delta == 0)
    return;

  lss = get_line_subseg (now_seg, now_subseg, false);
  if (!lss)
    return;

  now = frag_now_fix ();
  while ((e = *lss->pmove_tail))
    {
      if (S_GET_VALUE (e->label) == now)
	S_SET_VALUE (e->label, now + delta);
      lss->pmove_tail = &e->next;
    }
}

/* Called after the current line information has been either used with
   dwarf2_gen_line_info or saved with a machine instruction for later use.
   This resets the state of the line number information to reflect that
   it has been used.  */

void
dwarf2_consume_line_info (void)
{
  /* Unless we generate DWARF2 debugging information for each
     assembler line, we only emit one line symbol for one LOC.  */
  dwarf2_loc_directive_seen = false;

  current.flags &= ~(DWARF2_FLAG_BASIC_BLOCK
		     | DWARF2_FLAG_PROLOGUE_END
		     | DWARF2_FLAG_EPILOGUE_BEGIN);
  current.discriminator = 0;
  current.u.view = NULL;
}

/* Called for each (preferably code) label.  If dwarf2_loc_mark_labels
   is enabled, emit a basic block marker.  */

void
dwarf2_emit_label (symbolS *label)
{
  struct dwarf2_line_info loc;

  if (!dwarf2_loc_mark_labels)
    return;
  if (S_GET_SEGMENT (label) != now_seg)
    return;
  if (!(bfd_section_flags (now_seg) & SEC_CODE))
    return;
  if (files_in_use == 0 && debug_type != DEBUG_DWARF2)
    return;

  dwarf2_where (&loc);

  loc.flags |= DWARF2_FLAG_BASIC_BLOCK;

  dwarf2_gen_line_info_1 (label, &loc);
  dwarf2_consume_line_info ();
}

/* Handle two forms of .file directive:
   - Pass .file "source.c" to s_file
   - Handle .file 1 "source.c" by adding an entry to the DWARF-2 file table

   If an entry is added to the file table, return a pointer to the filename.  */

char *
dwarf2_directive_filename (void)
{
  bool with_md5 = false;
  valueT num;
  char *filename;
  const char * dirname = NULL;
  int filename_len;

  /* Continue to accept a bare string and pass it off.  */
  SKIP_WHITESPACE ();
  if (*input_line_pointer == '"')
    {
      s_file (0);
      return NULL;
    }

  num = get_absolute_expression ();

  if ((offsetT) num < 1)
    {
      if (num == 0 && dwarf_level < 5)
	dwarf_level = 5;
      if ((offsetT) num < 0 || DWARF2_LINE_VERSION < 5)
	{
	  as_bad (_("file number less than one"));
	  ignore_rest_of_line ();
	  return NULL;
	}
    }

  /* FIXME: Should we allow ".file <N>\n" as an expression meaning
     "switch back to the already allocated file <N> as the current
     file" ?  */

  filename = demand_copy_C_string (&filename_len);
  if (filename == NULL)
    /* demand_copy_C_string will have already generated an error message.  */
    return NULL;

  /* For DWARF-5 support we also accept:
     .file <NUM> ["<dir>"] "<file>" [md5 <NUM>]  */
  if (DWARF2_LINE_VERSION > 4)
    {
      SKIP_WHITESPACE ();
      if (*input_line_pointer == '"')
	{
	  dirname = filename;
	  filename = demand_copy_C_string (&filename_len);
	  if (filename == NULL)
	    return NULL;
	  SKIP_WHITESPACE ();
	}

      if (startswith (input_line_pointer, "md5"))
	{
	  input_line_pointer += 3;
	  SKIP_WHITESPACE ();

	  expressionS exp;
	  expression_and_evaluate (& exp);
	  if (exp.X_op != O_big)
	    as_bad (_("md5 value too small or not a constant"));
	  else
	    with_md5 = true;
	}
    }

  demand_empty_rest_of_line ();

  /* A .file directive implies compiler generated debug information is
     being supplied.  Turn off gas generated debug info.  */
  if (debug_type == DEBUG_DWARF2)
    purge_generated_debug (false);
  debug_type = DEBUG_NONE;

  if (num != (unsigned int) num
      || num >= (size_t) -1 / sizeof (struct file_entry) - 32)
    {
      as_bad (_("file number %lu is too big"), (unsigned long) num);
      return NULL;
    }

  if (! allocate_filename_to_slot (dirname, filename, (unsigned int) num,
				   with_md5))
    return NULL;

  return filename;
}

/* Calls dwarf2_directive_filename, but discards its result.
   Used in pseudo-op tables where the function result is ignored.  */

void
dwarf2_directive_file (int dummy ATTRIBUTE_UNUSED)
{
  (void) dwarf2_directive_filename ();
}

void
dwarf2_directive_loc (int dummy ATTRIBUTE_UNUSED)
{
  offsetT filenum, line;

  /* If we see two .loc directives in a row, force the first one to be
     output now.  */
  if (dwarf2_loc_directive_seen)
    dwarf2_emit_insn (0);

  filenum = get_absolute_expression ();
  SKIP_WHITESPACE ();
  line = get_absolute_expression ();

  if (filenum < 1)
    {
      if (filenum == 0 && dwarf_level < 5)
	dwarf_level = 5;
      if (filenum < 0 || DWARF2_LINE_VERSION < 5)
	{
	  as_bad (_("file number less than one"));
	  return;
	}
    }

  if ((valueT) filenum >= files_in_use || files[filenum].filename == NULL)
    {
      as_bad (_("unassigned file number %ld"), (long) filenum);
      return;
    }

  /* debug_type will be turned off by dwarf2_directive_filename, and
     if we don't have a dwarf style .file then files_in_use will be
     zero and the above error will trigger.  */
  gas_assert (debug_type == DEBUG_NONE);

  current.filenum = filenum;
  current.line = line;
  current.discriminator = 0;

#ifndef NO_LISTING
  if (listing)
    {
      if (files[filenum].dir)
	{
	  size_t dir_len = strlen (dirs[files[filenum].dir]);
	  size_t file_len = strlen (files[filenum].filename);
	  char *cp = XNEWVEC (char, dir_len + 1 + file_len + 1);

	  memcpy (cp, dirs[files[filenum].dir], dir_len);
	  INSERT_DIR_SEPARATOR (cp, dir_len);
	  memcpy (cp + dir_len + 1, files[filenum].filename, file_len);
	  cp[dir_len + file_len + 1] = '\0';
	  listing_source_file (cp);
	  free (cp);
	}
      else
	listing_source_file (files[filenum].filename);
      listing_source_line (line);
    }
#endif

  SKIP_WHITESPACE ();
  if (ISDIGIT (*input_line_pointer))
    {
      current.column = get_absolute_expression ();
      SKIP_WHITESPACE ();
    }

  while (ISALPHA (*input_line_pointer))
    {
      char *p, c;
      offsetT value;

      c = get_symbol_name (& p);

      if (strcmp (p, "basic_block") == 0)
	{
	  current.flags |= DWARF2_FLAG_BASIC_BLOCK;
	  *input_line_pointer = c;
	}
      else if (strcmp (p, "prologue_end") == 0)
	{
	  if (dwarf_level < 3)
	    dwarf_level = 3;
	  current.flags |= DWARF2_FLAG_PROLOGUE_END;
	  *input_line_pointer = c;
	}
      else if (strcmp (p, "epilogue_begin") == 0)
	{
	  if (dwarf_level < 3)
	    dwarf_level = 3;
	  current.flags |= DWARF2_FLAG_EPILOGUE_BEGIN;
	  *input_line_pointer = c;
	}
      else if (strcmp (p, "is_stmt") == 0)
	{
	  (void) restore_line_pointer (c);
	  value = get_absolute_expression ();
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
	  if (dwarf_level < 3)
	    dwarf_level = 3;
	  (void) restore_line_pointer (c);
	  value = get_absolute_expression ();
	  if (value >= 0)
	    current.isa = value;
	  else
	    {
	      as_bad (_("isa number less than zero"));
	      return;
	    }
	}
      else if (strcmp (p, "discriminator") == 0)
	{
	  (void) restore_line_pointer (c);
	  value = get_absolute_expression ();
	  if (value >= 0)
	    current.discriminator = value;
	  else
	    {
	      as_bad (_("discriminator less than zero"));
	      return;
	    }
	}
      else if (strcmp (p, "view") == 0)
	{
	  symbolS *sym;

	  (void) restore_line_pointer (c);
	  SKIP_WHITESPACE ();

	  if (ISDIGIT (*input_line_pointer)
	      || *input_line_pointer == '-')
	    {
	      bool force_reset = *input_line_pointer == '-';

	      value = get_absolute_expression ();
	      if (value != 0)
		{
		  as_bad (_("numeric view can only be asserted to zero"));
		  return;
		}
	      if (force_reset && force_reset_view)
		sym = force_reset_view;
	      else
		{
		  sym = symbol_temp_new (absolute_section, &zero_address_frag,
					 value);
		  if (force_reset)
		    force_reset_view = sym;
		}
	    }
	  else
	    {
	      char *name = read_symbol_name ();

	      if (!name)
		return;
	      sym = symbol_find_or_make (name);
	      free (name);
	      if (S_IS_DEFINED (sym) || symbol_equated_p (sym))
		{
		  if (S_IS_VOLATILE (sym))
		    sym = symbol_clone (sym, 1);
		  else if (!S_CAN_BE_REDEFINED (sym))
		    {
		      as_bad (_("symbol `%s' is already defined"),
			      S_GET_NAME (sym));
		      return;
		    }
		}
	      S_SET_SEGMENT (sym, undefined_section);
	      S_SET_VALUE (sym, 0);
	      symbol_set_frag (sym, &zero_address_frag);
	    }
	  current.u.view = sym;
	}
      else
	{
	  as_bad (_("unknown .loc sub-directive `%s'"), p);
	  (void) restore_line_pointer (c);
	  return;
	}

      SKIP_WHITESPACE_AFTER_NAME ();
    }

  demand_empty_rest_of_line ();
  dwarf2_any_loc_directive_seen = dwarf2_loc_directive_seen = true;

  /* If we were given a view id, emit the row right away.  */
  if (current.u.view)
    dwarf2_emit_insn (0);
}

void
dwarf2_directive_loc_mark_labels (int dummy ATTRIBUTE_UNUSED)
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
  return seg_info (seg)->frchainP->frch_root;
}

static struct frag *
last_frag_for_seg (segT seg)
{
  frchainS *f = seg_info (seg)->frchainP;

  while (f->frch_next != NULL)
    f = f->frch_next;

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
out_uleb128 (addressT value)
{
  output_leb128 (frag_more (sizeof_leb128 (value, 0)), value, 0);
}

/* Emit a signed "little-endian base 128" number.  */

static void
out_leb128 (addressT value)
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
  frchainS *fr;

  if (frag->fr_next)
    return frag->fr_fix;

  /* If a fragment is the last in the chain, special measures must be
     taken to find its size before relaxation, since it may be pending
     on some subsegment chain.  */
  for (fr = seg_info (seg)->frchainP; fr; fr = fr->frch_next)
    if (fr->frch_last == frag)
      return (char *) obstack_next_free (&fr->frch_obstack) - frag->fr_literal;

  abort ();
}

/* Set an absolute address (may result in a relocation entry).  */

static void
out_set_addr (symbolS *sym)
{
  expressionS exp;

  memset (&exp, 0, sizeof exp);
  out_opcode (DW_LNS_extended_op);
  out_uleb128 (sizeof_address + 1);

  out_opcode (DW_LNE_set_address);
  exp.X_op = O_symbol;
  exp.X_add_symbol = sym;
  exp.X_add_number = 0;
  emit_expr (&exp, sizeof_address);
}

static void
scale_addr_delta (int line_delta, addressT *addr_delta)
{
  static int printed_this = 0;
  if (DWARF2_LINE_MIN_INSN_LENGTH > 1)
    {
      /* Don't error on non-instruction bytes at end of section.  */
      if (line_delta != INT_MAX
	  && *addr_delta % DWARF2_LINE_MIN_INSN_LENGTH != 0  && !printed_this)
	{
	  as_bad("unaligned opcodes detected in executable segment");
	  printed_this = 1;
	}
      *addr_delta /= DWARF2_LINE_MIN_INSN_LENGTH;
    }
}

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
  scale_addr_delta (line_delta, &addr_delta);

  /* INT_MAX is a signal that this is actually a DW_LNE_end_sequence.
     We cannot use special opcodes here, since we want the end_sequence
     to emit the matrix entry.  */
  if (line_delta == INT_MAX)
    {
      if (addr_delta == MAX_SPECIAL_ADDR_DELTA)
	len = 1;
      else if (addr_delta)
	len = 1 + sizeof_leb128 (addr_delta, 0);
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
  if (addr_delta < 256U + MAX_SPECIAL_ADDR_DELTA)
    {
      /* Try using a special opcode.  */
      opcode = tmp + addr_delta * DWARF2_LINE_RANGE;
      if (opcode <= 255)
	return len + 1;

      /* Try using DW_LNS_const_add_pc followed by special op.  */
      opcode = tmp + (addr_delta - MAX_SPECIAL_ADDR_DELTA) * DWARF2_LINE_RANGE;
      if (opcode <= 255)
	return len + 2;
    }

  /* Otherwise use DW_LNS_advance_pc.  */
  len += 1 + sizeof_leb128 (addr_delta, 0);

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
  gas_assert ((offsetT) addr_delta >= 0);

  /* Scale the address delta by the minimum instruction length.  */
  scale_addr_delta (line_delta, &addr_delta);

  /* INT_MAX is a signal that this is actually a DW_LNE_end_sequence.
     We cannot use special opcodes here, since we want the end_sequence
     to emit the matrix entry.  */
  if (line_delta == INT_MAX)
    {
      if (addr_delta == MAX_SPECIAL_ADDR_DELTA)
	*p++ = DW_LNS_const_add_pc;
      else if (addr_delta)
	{
	  *p++ = DW_LNS_advance_pc;
	  p += output_leb128 (p, addr_delta, 0);
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
  if (addr_delta < 256U + MAX_SPECIAL_ADDR_DELTA)
    {
      /* Try using a special opcode.  */
      opcode = tmp + addr_delta * DWARF2_LINE_RANGE;
      if (opcode <= 255)
	{
	  *p++ = opcode;
	  goto done;
	}

      /* Try using DW_LNS_const_add_pc followed by special op.  */
      opcode = tmp + (addr_delta - MAX_SPECIAL_ADDR_DELTA) * DWARF2_LINE_RANGE;
      if (opcode <= 255)
	{
	  *p++ = DW_LNS_const_add_pc;
	  *p++ = opcode;
	  goto done;
	}
    }

  /* Otherwise use DW_LNS_advance_pc.  */
  *p++ = DW_LNS_advance_pc;
  p += output_leb128 (p, addr_delta, 0);

  if (need_copy)
    *p++ = DW_LNS_copy;
  else
    *p++ = tmp;

 done:
  gas_assert (p == end);
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
   line and address information, but it is required if linker relaxation
   could change the code offsets.  The following two routines *must* be
   kept in sync.  */
#define ADDR_DELTA_LIMIT 50000

static int
size_fixed_inc_line_addr (int line_delta, addressT addr_delta)
{
  int len = 0;

  /* INT_MAX is a signal that this is actually a DW_LNE_end_sequence.  */
  if (line_delta != INT_MAX)
    len = 1 + sizeof_leb128 (line_delta, 1);

  if (addr_delta > ADDR_DELTA_LIMIT)
    {
      /* DW_LNS_extended_op */
      len += 1 + sizeof_leb128 (sizeof_address + 1, 0);
      /* DW_LNE_set_address */
      len += 1 + sizeof_address;
    }
  else
    /* DW_LNS_fixed_advance_pc */
    len += 3;

  if (line_delta == INT_MAX)
    /* DW_LNS_extended_op + DW_LNE_end_sequence */
    len += 3;
  else
    /* DW_LNS_copy */
    len += 1;

  return len;
}

static void
emit_fixed_inc_line_addr (int line_delta, addressT addr_delta, fragS *frag,
			  char *p, int len)
{
  expressionS *pexp;
  char *end = p + len;

  /* Line number sequences cannot go backward in addresses.  This means
     we've incorrectly ordered the statements in the sequence.  */
  gas_assert ((offsetT) addr_delta >= 0);

  /* Verify that we have kept in sync with size_fixed_inc_line_addr.  */
  gas_assert (len == size_fixed_inc_line_addr (line_delta, addr_delta));

  /* INT_MAX is a signal that this is actually a DW_LNE_end_sequence.  */
  if (line_delta != INT_MAX)
    {
      *p++ = DW_LNS_advance_line;
      p += output_leb128 (p, line_delta, 1);
    }

  pexp = symbol_get_value_expression (frag->fr_symbol);

  /* The DW_LNS_fixed_advance_pc opcode has a 2-byte operand so it can
     advance the address by at most 64K.  Linker relaxation (without
     which this function would not be used) could change the operand by
     an unknown amount.  If the address increment is getting close to
     the limit, just reset the address.  */
  if (addr_delta > ADDR_DELTA_LIMIT)
    {
      symbolS *to_sym;
      expressionS exp;

      memset (&exp, 0, sizeof exp);
      gas_assert (pexp->X_op == O_subtract);
      to_sym = pexp->X_add_symbol;

      *p++ = DW_LNS_extended_op;
      p += output_leb128 (p, sizeof_address + 1, 0);
      *p++ = DW_LNE_set_address;
      exp.X_op = O_symbol;
      exp.X_add_symbol = to_sym;
      exp.X_add_number = 0;
      emit_expr_fix (&exp, sizeof_address, frag, p, TC_PARSE_CONS_RETURN_NONE);
      p += sizeof_address;
    }
  else
    {
      *p++ = DW_LNS_fixed_advance_pc;
      emit_expr_fix (pexp, 2, frag, p, TC_PARSE_CONS_RETURN_NONE);
      p += 2;
    }

  if (line_delta == INT_MAX)
    {
      *p++ = DW_LNS_extended_op;
      *p++ = 1;
      *p++ = DW_LNE_end_sequence;
    }
  else
    *p++ = DW_LNS_copy;

  gas_assert (p == end);
}

/* Generate a variant frag that we can use to relax address/line
   increments between fragments of the target segment.  */

static void
relax_inc_line_addr (int line_delta, symbolS *to_sym, symbolS *from_sym)
{
  expressionS exp;
  int max_chars;

  memset (&exp, 0, sizeof exp);
  exp.X_op = O_subtract;
  exp.X_add_symbol = to_sym;
  exp.X_op_symbol = from_sym;
  exp.X_add_number = 0;

  /* The maximum size of the frag is the line delta with a maximum
     sized address delta.  */
  if (DWARF2_USE_FIXED_ADVANCE_PC)
    max_chars = size_fixed_inc_line_addr (line_delta,
					  -DWARF2_LINE_MIN_INSN_LENGTH);
  else
    max_chars = size_inc_line_addr (line_delta, -DWARF2_LINE_MIN_INSN_LENGTH);

  frag_var (rs_dwarf2dbg, max_chars, max_chars, 1,
	    make_expr_symbol (&exp), line_delta, NULL);
}

/* The function estimates the size of a rs_dwarf2dbg variant frag
   based on the current values of the symbols.  It is called before
   the relaxation loop.  We set fr_subtype to the expected length.  */

int
dwarf2dbg_estimate_size_before_relax (fragS *frag)
{
  offsetT addr_delta;
  int size;

  addr_delta = resolve_symbol_value (frag->fr_symbol);
  if (DWARF2_USE_FIXED_ADVANCE_PC)
    size = size_fixed_inc_line_addr (frag->fr_offset, addr_delta);
  else
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

  if (DWARF2_USE_FIXED_ADVANCE_PC)
    {
      /* If linker relaxation is enabled then the distance between the two
	 symbols in the frag->fr_symbol expression might change.  Hence we
	 cannot rely upon the value computed by resolve_symbol_value.
	 Instead we leave the expression unfinalized and allow
	 emit_fixed_inc_line_addr to create a fixup (which later becomes a
	 relocation) that will allow the linker to correctly compute the
	 actual address difference.  We have to use a fixed line advance for
	 this as we cannot (easily) relocate leb128 encoded values.  */
      int saved_finalize_syms = finalize_syms;

      finalize_syms = 0;
      addr_diff = resolve_symbol_value (frag->fr_symbol);
      finalize_syms = saved_finalize_syms;
    }
  else
    addr_diff = resolve_symbol_value (frag->fr_symbol);

  /* fr_var carries the max_chars that we created the fragment with.
     fr_subtype carries the current expected length.  We must, of
     course, have allocated enough memory earlier.  */
  gas_assert (frag->fr_var >= (int) frag->fr_subtype);

  if (DWARF2_USE_FIXED_ADVANCE_PC)
    emit_fixed_inc_line_addr (frag->fr_offset, addr_diff, frag,
			      frag->fr_literal + frag->fr_fix,
			      frag->fr_subtype);
  else
    emit_inc_line_addr (frag->fr_offset, addr_diff,
			frag->fr_literal + frag->fr_fix, frag->fr_subtype);

  frag->fr_fix += frag->fr_subtype;
  frag->fr_type = rs_fill;
  frag->fr_var = 0;
  frag->fr_offset = 0;
}

/* Generate .debug_line content for the chain of line number entries
   beginning at E, for segment SEG.  */

static void
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

  if (flag_dwarf_sections)
    {
      char * name;
      const char * sec_name;

      /* Switch to the relevant sub-section before we start to emit
	 the line number table.

	 FIXME: These sub-sections do not have a normal Line Number
	 Program Header, thus strictly speaking they are not valid
	 DWARF sections.  Unfortunately the DWARF standard assumes
	 a one-to-one relationship between compilation units and
	 line number tables.  Thus we have to have a .debug_line
	 section, as well as our sub-sections, and we have to ensure
	 that all of the sub-sections are merged into a proper
	 .debug_line section before a debugger sees them.  */

      sec_name = bfd_section_name (seg);
      if (strcmp (sec_name, ".text") != 0)
	{
	  name = concat (".debug_line", sec_name, (char *) NULL);
	  subseg_set (subseg_get (name, false), 0);
	}
      else
	/* Don't create a .debug_line.text section -
	   that is redundant.  Instead just switch back to the
	   normal .debug_line section.  */
	subseg_set (subseg_get (".debug_line", false), 0);
    }

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

      if (e->loc.discriminator != 0)
	{
	  out_opcode (DW_LNS_extended_op);
	  out_leb128 (1 + sizeof_leb128 (e->loc.discriminator, 0));
	  out_opcode (DW_LNE_set_discriminator);
	  out_uleb128 (e->loc.discriminator);
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
	 the {, and there's no way to identify that case here.  Trust gcc
	 to optimize appropriately.  */
      line_delta = e->loc.line - line;
      lab = e->label;
      frag = symbol_get_frag (lab);
      frag_ofs = S_GET_VALUE (lab);

      if (last_frag == NULL
	  || (e->loc.u.view == force_reset_view && force_reset_view
	      /* If we're going to reset the view, but we know we're
		 advancing the PC, we don't have to force with
		 set_address.  We know we do when we're at the same
		 address of the same frag, and we know we might when
		 we're in the beginning of a frag, and we were at the
		 end of the previous frag.  */
	      && (frag == last_frag
		  ? (last_frag_ofs == frag_ofs)
		  : (frag_ofs == 0
		     && ((offsetT)last_frag_ofs
			 >= get_frag_fix (last_frag, seg))))))
	{
	  out_set_addr (lab);
	  out_inc_line_addr (line_delta, 0);
	}
      else if (frag == last_frag && ! DWARF2_USE_FIXED_ADVANCE_PC)
	out_inc_line_addr (line_delta, frag_ofs - last_frag_ofs);
      else
	relax_inc_line_addr (line_delta, lab, last_lab);

      line = e->loc.line;
      last_lab = lab;
      last_frag = frag;
      last_frag_ofs = frag_ofs;

      e = e->next;
    }
  while (e);

  /* Emit a DW_LNE_end_sequence for the end of the section.  */
  frag = last_frag_for_seg (seg);
  frag_ofs = get_frag_fix (frag, seg);
  if (frag == last_frag && ! DWARF2_USE_FIXED_ADVANCE_PC)
    out_inc_line_addr (INT_MAX, frag_ofs - last_frag_ofs);
  else
    {
      lab = symbol_temp_new (seg, frag, frag_ofs);
      relax_inc_line_addr (INT_MAX, lab, last_lab);
    }
}

/* Switch to LINE_STR_SEG and output the given STR.  Return the
   symbol pointing to the new string in the section.  */

static symbolS *
add_line_strp (segT line_str_seg, const char *str)
{
  char *cp;
  size_t size;
  symbolS *sym;

  subseg_set (line_str_seg, 0);

  sym = symbol_temp_new_now_octets ();

  size = strlen (str) + 1;
  cp = frag_more (size);
  memcpy (cp, str, size);

  return sym;
}


/* Emit the directory and file tables for .debug_line.  */

static void
out_dir_and_file_list (segT line_seg, int sizeof_offset)
{
  size_t size;
  char *dir;
  char *cp;
  unsigned int i, j;
  bool emit_md5 = false;
  bool emit_timestamps = true;
  bool emit_filesize = true;
  segT line_str_seg = NULL;
  symbolS *line_strp, *file0_strp = NULL;

  /* Output the Directory Table.  */
  if (DWARF2_LINE_VERSION >= 5)
    {
      /* We only have one column in the directory table.  */
      out_byte (1);

      /* Describe the purpose and format of the column.  */
      out_uleb128 (DW_LNCT_path);
      /* Store these strings in the .debug_line_str section so they
	 can be shared.  */
      out_uleb128 (DW_FORM_line_strp);

      /* Now state how many rows there are in the table.  We need at
	 least 1 if there is one or more file names to store the
	 "working directory".  */
      if (dirs_in_use == 0 && files_in_use > 0)
	out_uleb128 (1);
      else
	out_uleb128 (dirs_in_use);
    }
      
  /* Emit directory list.  */
  if (DWARF2_LINE_VERSION >= 5 && (dirs_in_use > 0 || files_in_use > 0))
    {
      line_str_seg = subseg_new (".debug_line_str", 0);
      bfd_set_section_flags (line_str_seg,
			     SEC_READONLY | SEC_DEBUGGING | SEC_OCTETS
			     | SEC_MERGE | SEC_STRINGS);
      line_str_seg->entsize = 1;

      /* DWARF5 uses slot zero, but that is only set explicitly
	 using a .file 0 directive.  Otherwise use pwd as main file
	 directory.  */
      if (dirs_in_use > 0 && dirs[0] != NULL)
	dir = remap_debug_filename (dirs[0]);
      else
	dir = remap_debug_filename (getpwd ());

      line_strp = add_line_strp (line_str_seg, dir);
      free (dir);
      subseg_set (line_seg, 0);
      TC_DWARF2_EMIT_OFFSET (line_strp, sizeof_offset);
    }
  for (i = 1; i < dirs_in_use; ++i)
    {
      dir = remap_debug_filename (dirs[i]);
      if (DWARF2_LINE_VERSION < 5)
	{
	  size = strlen (dir) + 1;
	  cp = frag_more (size);
	  memcpy (cp, dir, size);
	}
      else
	{
	  line_strp = add_line_strp (line_str_seg, dir);
	  subseg_set (line_seg, 0);
	  TC_DWARF2_EMIT_OFFSET (line_strp, sizeof_offset);
	}
      free (dir);
    }

  if (DWARF2_LINE_VERSION < 5)
    /* Terminate it.  */
    out_byte ('\0');

  /* Output the File Name Table.  */
  if (DWARF2_LINE_VERSION >= 5)
    {
      unsigned int columns = 4;

      if (((unsigned long) DWARF2_FILE_TIME_NAME ("", "")) == -1UL)
	{
	  emit_timestamps = false;
	  -- columns;
	}

      if (DWARF2_FILE_SIZE_NAME ("", "") == -1)
	{
	  emit_filesize = false;
	  -- columns;
	}

      for (i = 0; i < files_in_use; ++i)
	if (files[i].md5[0] != 0)
	  break;
      if (i < files_in_use)
	{
	  emit_md5 = true;
	  ++ columns;
	}
      
      /* The number of format entries to follow.  */
      out_byte (columns);
      /* The format of the file name.  */
      out_uleb128 (DW_LNCT_path);
      /* Store these strings in the .debug_line_str section so they
	 can be shared.  */
      out_uleb128 (DW_FORM_line_strp);

      /* The format of the directory index.  */
      out_uleb128 (DW_LNCT_directory_index);
      out_uleb128 (DW_FORM_udata);

      if (emit_timestamps)
	{
	  /* The format of the timestamp.  */
	  out_uleb128 (DW_LNCT_timestamp);
	  out_uleb128 (DW_FORM_udata);
	}

      if (emit_filesize)
	{
	  /* The format of the file size.  */
	  out_uleb128 (DW_LNCT_size);
	  out_uleb128 (DW_FORM_udata);
	}

      if (emit_md5)
	{
	  /* The format of the MD5 sum.  */
	  out_uleb128 (DW_LNCT_MD5);
	  out_uleb128 (DW_FORM_data16);
	}

      /* The number of entries in the table.  */
      out_uleb128 (files_in_use);
   }
      
  for (i = DWARF2_LINE_VERSION > 4 ? 0 : 1; i < files_in_use; ++i)
    {
      const char *fullfilename;

      if (files[i].filename == NULL)
	{
	  if (DWARF2_LINE_VERSION < 5 || i != 0)
	    {
	      as_bad (_("unassigned file number %ld"), (long) i);
	      continue;
	    }
	  /* DWARF5 uses slot zero, but that is only set explicitly using
	     a .file 0 directive.  If that isn't used, but file 1 is, then
	     use that as main file name.  */
	  if (files_in_use > 1 && files[1].filename != NULL)
	    {
	      files[0].filename = files[1].filename;
	      files[0].dir = files[1].dir;
	      if (emit_md5)
		for (j = 0; j < NUM_MD5_BYTES; ++j)
		  files[0].md5[j] = files[1].md5[j];
	    }
	  else
	    files[0].filename = "";
	}

      fullfilename = DWARF2_FILE_NAME (files[i].filename,
				       files[i].dir ? dirs [files [i].dir] : "");
      if (DWARF2_LINE_VERSION < 5)
	{
	  size = strlen (fullfilename) + 1;
	  cp = frag_more (size);
	  memcpy (cp, fullfilename, size);
	}
      else
	{
	  if (!file0_strp)
	    line_strp = add_line_strp (line_str_seg, fullfilename);
	  else
	    line_strp = file0_strp;
	  subseg_set (line_seg, 0);
	  TC_DWARF2_EMIT_OFFSET (line_strp, sizeof_offset);
	  if (i == 0 && files_in_use > 1
	      && files[0].filename == files[1].filename)
	    file0_strp = line_strp;
	  else
	    file0_strp = NULL;
	}

      /* Directory number.  */
      out_uleb128 (files[i].dir);

      /* Output the last modification timestamp.  */
      if (emit_timestamps)
	{
	  offsetT timestamp;

	  timestamp = DWARF2_FILE_TIME_NAME (files[i].filename,
					     files[i].dir ? dirs [files [i].dir] : "");
	  if (timestamp == -1)
	    timestamp = 0;
	  out_uleb128 (timestamp);
	}

      /* Output the filesize.  */
      if (emit_filesize)
	{
	  offsetT filesize;
	  filesize = DWARF2_FILE_SIZE_NAME (files[i].filename,
					    files[i].dir ? dirs [files [i].dir] : "");
	  if (filesize == -1)
	    filesize = 0;
	  out_uleb128 (filesize);
	}

      /* Output the md5 sum.  */
      if (emit_md5)
	{
	  int b;

	  for (b = 0; b < NUM_MD5_BYTES; b++)
	    out_byte (files[i].md5[b]);
	}
    }

  if (DWARF2_LINE_VERSION < 5)
    /* Terminate filename list.  */
    out_byte (0);
}

/* Switch to SEC and output a header length field.  Return the size of
   offsets used in SEC.  The caller must set EXPR->X_add_symbol value
   to the end of the section.  EXPR->X_add_number will be set to the
   negative size of the header.  */

static int
out_header (asection *sec, expressionS *exp)
{
  symbolS *start_sym;
  symbolS *end_sym;

  subseg_set (sec, 0);

  if (flag_dwarf_sections)
    {
      /* If we are going to put the start and end symbols in different
	 sections, then we need real symbols, not just fake, local ones.  */
      frag_now_fix ();
      start_sym = symbol_make (".Ldebug_line_start");
      end_sym = symbol_make (".Ldebug_line_end");
      symbol_set_value_now (start_sym);
    }
  else
    {
      start_sym = symbol_temp_new_now_octets ();
      end_sym = symbol_temp_make ();
    }

  /* Total length of the information.  */
  exp->X_op = O_subtract;
  exp->X_add_symbol = end_sym;
  exp->X_op_symbol = start_sym;

  switch (DWARF2_FORMAT (sec))
    {
    case dwarf2_format_32bit:
      exp->X_add_number = -4;
      emit_expr (exp, 4);
      return 4;

    case dwarf2_format_64bit:
      exp->X_add_number = -12;
      out_four (-1);
      emit_expr (exp, 8);
      return 8;

    case dwarf2_format_64bit_irix:
      exp->X_add_number = -8;
      emit_expr (exp, 8);
      return 8;
    }

  as_fatal (_("internal error: unknown dwarf2 format"));
  return 0;
}

/* Emit the collected .debug_line data.  */

static void
out_debug_line (segT line_seg)
{
  expressionS exp;
  symbolS *prologue_start, *prologue_end;
  symbolS *line_end;
  struct line_seg *s;
  int sizeof_offset;

  memset (&exp, 0, sizeof exp);
  sizeof_offset = out_header (line_seg, &exp);
  line_end = exp.X_add_symbol;

  /* Version.  */
  out_two (DWARF2_LINE_VERSION);

  if (DWARF2_LINE_VERSION >= 5)
    {
      out_byte (sizeof_address);
      out_byte (0); /* Segment Selector size.  */
    }
  /* Length of the prologue following this length.  */
  prologue_start = symbol_temp_make ();
  prologue_end = symbol_temp_make ();
  exp.X_op = O_subtract;
  exp.X_add_symbol = prologue_end;
  exp.X_op_symbol = prologue_start;
  exp.X_add_number = 0;
  emit_expr (&exp, sizeof_offset);
  symbol_set_value_now (prologue_start);

  /* Parameters of the state machine.  */
  out_byte (DWARF2_LINE_MIN_INSN_LENGTH);
  if (DWARF2_LINE_VERSION >= 4)
    out_byte (DWARF2_LINE_MAX_OPS_PER_INSN);
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
  if (DWARF2_LINE_VERSION >= 3)
    {
      out_byte (0);			/* DW_LNS_set_prologue_end */
      out_byte (0);			/* DW_LNS_set_epilogue_begin */
      out_byte (1);			/* DW_LNS_set_isa */
      /* We have emitted 12 opcode lengths, so make that this
	 matches up to the opcode base value we have been using.  */
      gas_assert (DWARF2_LINE_OPCODE_BASE == 13);
    }
  else
    gas_assert (DWARF2_LINE_OPCODE_BASE == 10);

  out_dir_and_file_list (line_seg, sizeof_offset);

  symbol_set_value_now (prologue_end);

  /* For each section, emit a statement program.  */
  for (s = all_segs; s; s = s->next)
    /* Paranoia - this check should have already have
       been handled in dwarf2_gen_line_info_1().  */
    if (s->head->head && SEG_NORMAL (s->seg))
      process_entries (s->seg, s->head->head);

  if (flag_dwarf_sections)
    /* We have to switch to the special .debug_line_end section
       before emitting the end-of-debug_line symbol.  The linker
       script arranges for this section to be placed after all the
       (potentially garbage collected) .debug_line.<foo> sections.
       This section contains the line_end symbol which is used to
       compute the size of the linked .debug_line section, as seen
       in the DWARF Line Number header.  */
    subseg_set (subseg_get (".debug_line_end", false), 0);

  symbol_set_value_now (line_end);
}

static void
out_debug_ranges (segT ranges_seg, symbolS **ranges_sym)
{
  unsigned int addr_size = sizeof_address;
  struct line_seg *s;
  expressionS exp;
  unsigned int i;

  memset (&exp, 0, sizeof exp);
  subseg_set (ranges_seg, 0);

  /* For DW_AT_ranges to point at (there is no header, so really start
     of section, but see out_debug_rnglists).  */
  *ranges_sym = symbol_temp_new_now_octets ();

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
      beg = symbol_temp_new (s->seg, frag, 0);
      s->text_start = beg;

      frag = last_frag_for_seg (s->seg);
      end = symbol_temp_new (s->seg, frag, get_frag_fix (frag, s->seg));
      s->text_end = end;

      exp.X_op = O_symbol;
      exp.X_add_symbol = beg;
      exp.X_add_number = 0;
      emit_expr (&exp, addr_size);

      exp.X_op = O_symbol;
      exp.X_add_symbol = end;
      exp.X_add_number = 0;
      emit_expr (&exp, addr_size);
    }

  /* End of Range Entry.   */
  for (i = 0; i < addr_size; i++)
    out_byte (0);
  for (i = 0; i < addr_size; i++)
    out_byte (0);
}

static void
out_debug_rnglists (segT ranges_seg, symbolS **ranges_sym)
{
  expressionS exp;
  symbolS *ranges_end;
  struct line_seg *s;

  /* Unit length.  */
  memset (&exp, 0, sizeof exp);
  out_header (ranges_seg, &exp);
  ranges_end = exp.X_add_symbol;

  out_two (DWARF2_RNGLISTS_VERSION);
  out_byte (sizeof_address);
  out_byte (0); /* Segment Selector size.  */
  out_four (0); /* Offset entry count.  */

  /* For DW_AT_ranges to point at (must be after the header).   */
  *ranges_sym = symbol_temp_new_now_octets ();

  for (s = all_segs; s; s = s->next)
    {
      fragS *frag;
      symbolS *beg, *end;

      out_byte (DW_RLE_start_length);

      frag = first_frag_for_seg (s->seg);
      beg = symbol_temp_new (s->seg, frag, 0);
      s->text_start = beg;

      frag = last_frag_for_seg (s->seg);
      end = symbol_temp_new (s->seg, frag, get_frag_fix (frag, s->seg));
      s->text_end = end;

      exp.X_op = O_symbol;
      exp.X_add_symbol = beg;
      exp.X_add_number = 0;
      emit_expr (&exp, sizeof_address);

      exp.X_op = O_symbol;
      exp.X_add_symbol = end;
      exp.X_add_number = 0;
      emit_leb128_expr (&exp, 0);
    }

  out_byte (DW_RLE_end_of_list);

  symbol_set_value_now (ranges_end);
}

/* Emit data for .debug_aranges.  */

static void
out_debug_aranges (segT aranges_seg, segT info_seg)
{
  unsigned int addr_size = sizeof_address;
  offsetT size;
  struct line_seg *s;
  expressionS exp;
  symbolS *aranges_end;
  char *p;
  int sizeof_offset;

  memset (&exp, 0, sizeof exp);
  sizeof_offset = out_header (aranges_seg, &exp);
  aranges_end = exp.X_add_symbol;
  size = -exp.X_add_number;

  /* Version.  */
  out_two (DWARF2_ARANGES_VERSION);
  size += 2;

  /* Offset to .debug_info.  */
  TC_DWARF2_EMIT_OFFSET (section_symbol (info_seg), sizeof_offset);
  size += sizeof_offset;

  /* Size of an address (offset portion).  */
  out_byte (addr_size);
  size++;

  /* Size of a segment descriptor.  */
  out_byte (0);
  size++;

  /* Align the header.  */
  while ((size++ % (2 * addr_size)) > 0)
    out_byte (0);

  for (s = all_segs; s; s = s->next)
    {
      fragS *frag;
      symbolS *beg, *end;

      frag = first_frag_for_seg (s->seg);
      beg = symbol_temp_new (s->seg, frag, 0);
      s->text_start = beg;

      frag = last_frag_for_seg (s->seg);
      end = symbol_temp_new (s->seg, frag, get_frag_fix (frag, s->seg));
      s->text_end = end;

      exp.X_op = O_symbol;
      exp.X_add_symbol = beg;
      exp.X_add_number = 0;
      emit_expr (&exp, addr_size);

      exp.X_op = O_subtract;
      exp.X_add_symbol = end;
      exp.X_op_symbol = beg;
      exp.X_add_number = 0;
      emit_expr (&exp, addr_size);
    }

  p = frag_more (2 * addr_size);
  md_number_to_chars (p, 0, addr_size);
  md_number_to_chars (p + addr_size, 0, addr_size);

  symbol_set_value_now (aranges_end);
}

/* Emit data for .debug_abbrev.  Note that this must be kept in
   sync with out_debug_info below.  */

static void
out_debug_abbrev (segT abbrev_seg,
		  segT info_seg ATTRIBUTE_UNUSED,
		  segT line_seg ATTRIBUTE_UNUSED,
		  unsigned char *func_formP)
{
  int secoff_form;
  bool have_efunc = false, have_lfunc = false;

  /* Check the symbol table for function symbols which also have their size
     specified.  */
  if (symbol_rootP)
    {
      symbolS *symp;

      for (symp = symbol_rootP; symp; symp = symbol_next (symp))
	{
	  /* A warning construct is a warning symbol followed by the
	     symbol warned about.  Skip this and the following symbol.  */
	  if (symbol_get_bfdsym (symp)->flags & BSF_WARNING)
	    {
	      symp = symbol_next (symp);
	      if (!symp)
	        break;
	      continue;
	    }

	  if (!S_IS_DEFINED (symp) || !S_IS_FUNCTION (symp))
	    continue;

#if defined (OBJ_ELF) /* || defined (OBJ_MAYBE_ELF) */
	  if (S_GET_SIZE (symp) == 0)
	    {
	      if (!IS_ELF || symbol_get_obj (symp)->size == NULL)
		continue;
	    }
#else
	  continue;
#endif

	  if (S_IS_EXTERNAL (symp))
	    have_efunc = true;
	  else
	    have_lfunc = true;
	}
    }

  subseg_set (abbrev_seg, 0);

  out_uleb128 (GAS_ABBREV_COMP_UNIT);
  out_uleb128 (DW_TAG_compile_unit);
  out_byte (have_efunc || have_lfunc ? DW_CHILDREN_yes : DW_CHILDREN_no);
  if (DWARF2_VERSION < 4)
    {
      if (DWARF2_FORMAT (line_seg) == dwarf2_format_32bit)
	secoff_form = DW_FORM_data4;
      else
	secoff_form = DW_FORM_data8;
    }
  else
    secoff_form = DW_FORM_sec_offset;
  out_abbrev (DW_AT_stmt_list, secoff_form);
  if (all_segs->next == NULL)
    {
      out_abbrev (DW_AT_low_pc, DW_FORM_addr);
      if (DWARF2_VERSION < 4)
	out_abbrev (DW_AT_high_pc, DW_FORM_addr);
      else
	out_abbrev (DW_AT_high_pc, DW_FORM_udata);
    }
  else
    out_abbrev (DW_AT_ranges, secoff_form);
  out_abbrev (DW_AT_name, DW_FORM_strp);
  out_abbrev (DW_AT_comp_dir, DW_FORM_strp);
  out_abbrev (DW_AT_producer, DW_FORM_strp);
  out_abbrev (DW_AT_language, DW_FORM_data2);
  out_abbrev (0, 0);

  if (have_efunc || have_lfunc)
    {
      out_uleb128 (GAS_ABBREV_SUBPROG);
      out_uleb128 (DW_TAG_subprogram);
      out_byte (DW_CHILDREN_no);
      out_abbrev (DW_AT_name, DW_FORM_strp);
      if (have_efunc)
	{
	  if (have_lfunc || DWARF2_VERSION < 4)
	    *func_formP = DW_FORM_flag;
	  else
	    *func_formP = DW_FORM_flag_present;
	  out_abbrev (DW_AT_external, *func_formP);
	}
      else
	/* Any non-zero value other than DW_FORM_flag will do.  */
	*func_formP = DW_FORM_block;

      /* PR 29517: Provide a return type for the function.  */
      if (DWARF2_VERSION > 2)
	out_abbrev (DW_AT_type, DW_FORM_ref_udata);

      out_abbrev (DW_AT_low_pc, DW_FORM_addr);
      out_abbrev (DW_AT_high_pc,
		  DWARF2_VERSION < 4 ? DW_FORM_addr : DW_FORM_udata);
      out_abbrev (0, 0);

      if (DWARF2_VERSION > 2)
	{
	  /* PR 29517: We do not actually know the return type of these
	     functions, so provide an abbrev that uses DWARF's unspecified
	     type.  */
	  out_uleb128 (GAS_ABBREV_NO_TYPE);
	  out_uleb128 (DW_TAG_unspecified_type);
	  out_byte (DW_CHILDREN_no);
	  out_abbrev (0, 0);
	}
    }

  /* Terminate the abbreviations for this compilation unit.  */
  out_byte (0);
}

/* Emit a description of this compilation unit for .debug_info.  */

static void
out_debug_info (segT info_seg, segT abbrev_seg, segT line_seg, segT str_seg,
		symbolS *ranges_sym, symbolS *name_sym,
		symbolS *comp_dir_sym, symbolS *producer_sym,
		unsigned char func_form)
{
  expressionS exp;
  symbolS *info_end;
  int sizeof_offset;

  memset (&exp, 0, sizeof exp);
  sizeof_offset = out_header (info_seg, &exp);
  info_end = exp.X_add_symbol;

  /* DWARF version.  */
  out_two (DWARF2_VERSION);

  if (DWARF2_VERSION < 5)
    {
      /* .debug_abbrev offset */
      TC_DWARF2_EMIT_OFFSET (section_symbol (abbrev_seg), sizeof_offset);
    }
  else
    {
      /* unit (header) type */
      out_byte (DW_UT_compile);
    }

  /* Target address size.  */
  out_byte (sizeof_address);

  if (DWARF2_VERSION >= 5)
    {
      /* .debug_abbrev offset */
      TC_DWARF2_EMIT_OFFSET (section_symbol (abbrev_seg), sizeof_offset);
    }

  /* DW_TAG_compile_unit DIE abbrev */
  out_uleb128 (GAS_ABBREV_COMP_UNIT);

  /* DW_AT_stmt_list */
  TC_DWARF2_EMIT_OFFSET (section_symbol (line_seg),
			 (DWARF2_FORMAT (line_seg) == dwarf2_format_32bit
			  ? 4 : 8));

  /* These two attributes are emitted if all of the code is contiguous.  */
  if (all_segs->next == NULL)
    {
      /* DW_AT_low_pc */
      exp.X_op = O_symbol;
      exp.X_add_symbol = all_segs->text_start;
      exp.X_add_number = 0;
      emit_expr (&exp, sizeof_address);

      /* DW_AT_high_pc */
      if (DWARF2_VERSION < 4)
	exp.X_op = O_symbol;
      else
	{
	  exp.X_op = O_subtract;
	  exp.X_op_symbol = all_segs->text_start;
	}
      exp.X_add_symbol = all_segs->text_end;
      exp.X_add_number = 0;
      if (DWARF2_VERSION < 4)
	emit_expr (&exp, sizeof_address);
      else
	emit_leb128_expr (&exp, 0);
    }
  else
    {
      /* This attribute is emitted if the code is disjoint.  */
      /* DW_AT_ranges.  */
      TC_DWARF2_EMIT_OFFSET (ranges_sym, sizeof_offset);
    }

  /* DW_AT_name, DW_AT_comp_dir and DW_AT_producer.  Symbols in .debug_str
     setup in out_debug_str below.  */
  TC_DWARF2_EMIT_OFFSET (name_sym, sizeof_offset);
  TC_DWARF2_EMIT_OFFSET (comp_dir_sym, sizeof_offset);
  TC_DWARF2_EMIT_OFFSET (producer_sym, sizeof_offset);

  /* DW_AT_language.  Yes, this is probably not really MIPS, but the
     dwarf2 draft has no standard code for assembler.  */
  out_two (DW_LANG_Mips_Assembler);

  if (func_form)
    {
      symbolS *symp;
      symbolS *no_type_tag;

      if (DWARF2_VERSION > 2)
	no_type_tag = symbol_make (".Ldebug_no_type_tag");
      else
	no_type_tag = NULL;

      for (symp = symbol_rootP; symp; symp = symbol_next (symp))
	{
	  const char *name;
	  size_t len;
	  expressionS size = { .X_op = O_constant };

	  /* Skip warning constructs (see above).  */
	  if (symbol_get_bfdsym (symp)->flags & BSF_WARNING)
	    {
	      symp = symbol_next (symp);
	      if (!symp)
	        break;
	      continue;
	    }

	  if (!S_IS_DEFINED (symp) || !S_IS_FUNCTION (symp))
	    continue;

#if defined (OBJ_ELF) /* || defined (OBJ_MAYBE_ELF) */
	  size.X_add_number = S_GET_SIZE (symp);
	  if (size.X_add_number == 0 && IS_ELF
	      && symbol_get_obj (symp)->size != NULL)
	    {
	      size.X_op = O_add;
	      size.X_op_symbol = make_expr_symbol (symbol_get_obj (symp)->size);
	    }
#endif
	  if (size.X_op == O_constant && size.X_add_number == 0)
	    continue;

	  subseg_set (str_seg, 0);
	  name_sym = symbol_temp_new_now_octets ();
	  name = S_GET_NAME (symp);
	  len = strlen (name) + 1;
	  memcpy (frag_more (len), name, len);

	  subseg_set (info_seg, 0);

	  /* DW_TAG_subprogram DIE abbrev */
	  out_uleb128 (GAS_ABBREV_SUBPROG);

	  /* DW_AT_name */
	  TC_DWARF2_EMIT_OFFSET (name_sym, sizeof_offset);

	  /* DW_AT_external.  */
	  if (func_form == DW_FORM_flag)
	    out_byte (S_IS_EXTERNAL (symp));

	  /* PR 29517: Let consumers know that we do not have
	     return type information for this function.  */
	  if (DWARF2_VERSION > 2)
	    {
	      exp.X_op = O_symbol;
	      exp.X_add_symbol = no_type_tag;
	      exp.X_add_number = 0;
	      emit_leb128_expr (&exp, 0);
	    }

	  /* DW_AT_low_pc */
	  exp.X_op = O_symbol;
	  exp.X_add_symbol = symp;
	  exp.X_add_number = 0;
	  emit_expr (&exp, sizeof_address);

	  /* DW_AT_high_pc */
	  if (DWARF2_VERSION < 4)
	    {
	      if (size.X_op == O_constant)
		size.X_op = O_symbol;
	      size.X_add_symbol = symp;
	      emit_expr (&size, sizeof_address);
	    }
	  else if (size.X_op == O_constant)
	    out_uleb128 (size.X_add_number);
	  else
	    emit_leb128_expr (symbol_get_value_expression (size.X_op_symbol), 0);
	}

      if (DWARF2_VERSION > 2)
	{
	  /* PR 29517: Generate a DIE for the unspecified type abbrev.
	     We do it here because it cannot be part of the top level DIE.   */
	  subseg_set (info_seg, 0);
	  symbol_set_value_now (no_type_tag);
	  out_uleb128 (GAS_ABBREV_NO_TYPE);
	}

      /* End of children.  */
      out_leb128 (0);
    }

  symbol_set_value_now (info_end);
}

/* Emit the three debug strings needed in .debug_str and setup symbols
   to them for use in out_debug_info.  */
static void
out_debug_str (segT str_seg, symbolS **name_sym, symbolS **comp_dir_sym,
	       symbolS **producer_sym)
{
  char producer[128];
  char *p;
  int len;
  int first_file = DWARF2_LINE_VERSION > 4 ? 0 : 1;

  subseg_set (str_seg, 0);

  /* DW_AT_name.  We don't have the actual file name that was present
     on the command line, so assume files[first_file] is the main input file.
     We're not supposed to get called unless at least one line number
     entry was emitted, so this should always be defined.  */
  *name_sym = symbol_temp_new_now_octets ();
  if (files_in_use == 0)
    abort ();
  if (files[first_file].dir)
    {
      char *dirname = remap_debug_filename (dirs[files[first_file].dir]);
      len = strlen (dirname);
#ifdef TE_VMS
      /* Already has trailing slash.  */
      p = frag_more (len);
      memcpy (p, dirname, len);
#else
      p = frag_more (len + 1);
      memcpy (p, dirname, len);
      INSERT_DIR_SEPARATOR (p, len);
#endif
      free (dirname);
    }
  len = strlen (files[first_file].filename) + 1;
  p = frag_more (len);
  memcpy (p, files[first_file].filename, len);

  /* DW_AT_comp_dir */
  *comp_dir_sym = symbol_temp_new_now_octets ();
  char *comp_dir = remap_debug_filename (getpwd ());
  len = strlen (comp_dir) + 1;
  p = frag_more (len);
  memcpy (p, comp_dir, len);
  free (comp_dir);

  /* DW_AT_producer */
  *producer_sym = symbol_temp_new_now_octets ();
  sprintf (producer, "GNU AS %s", VERSION);
  len = strlen (producer) + 1;
  p = frag_more (len);
  memcpy (p, producer, len);
}

void
dwarf2_init (void)
{
  all_segs = NULL;
  last_seg_ptr = &all_segs;
  files = NULL;
  files_in_use = 0;
  files_allocated = 0;
  dirs = NULL;
  dirs_in_use = 0;
  dirs_allocated = 0;
  dwarf2_loc_directive_seen = false;
  dwarf2_any_loc_directive_seen = false;
  dwarf2_loc_mark_labels = false;
  current.filenum = 1;
  current.line = 1;
  current.column = 0;
  current.isa = 0;
  current.flags = DWARF2_LINE_DEFAULT_IS_STMT ? DWARF2_FLAG_IS_STMT : 0;
  current.discriminator = 0;
  current.u.view = NULL;
  force_reset_view = NULL;
  view_assert_failed = NULL;
  dw2_line = -1;
  dw2_filename = NULL;
  label_num = 0;
  last_used = -1;

  /* Select the default CIE version to produce here.  The global
     starts with a value of -1 and will be modified to a valid value
     either by the user providing a command line option, or some
     targets will select their own default in md_after_parse_args.  If
     we get here and the global still contains -1 then it is up to us
     to pick a sane default.  The default we choose is 1, this is the
     CIE version gas has produced for a long time, and there seems no
     reason to change it yet.  */
  if (flag_dwarf_cie_version == -1)
    flag_dwarf_cie_version = 1;
}

static void
dwarf2_cleanup (void)
{
  purge_generated_debug (true);
  free (files);
  for (unsigned int i = 0; i < dirs_in_use; i++)
    free (dirs[i]);
  free (dirs);
}

/* Finish the dwarf2 debug sections.  We emit .debug.line if there
   were any .file/.loc directives, or --gdwarf2 was given, and if the
   file has a non-empty .debug_info section and an empty .debug_line
   section.  If we emit .debug_line, and the .debug_info section is
   empty, we also emit .debug_info, .debug_aranges and .debug_abbrev.
   ALL_SEGS will be non-null if there were any .file/.loc directives,
   or --gdwarf2 was given and there were any located instructions
   emitted.  */

void
dwarf2_finish (void)
{
  segT line_seg;
  struct line_seg *s;
  segT info_seg;
  int emit_other_sections = 0;
  int empty_debug_line = 0;

  info_seg = bfd_get_section_by_name (stdoutput, ".debug_info");
  emit_other_sections = info_seg == NULL || !seg_not_empty_p (info_seg);

  line_seg = bfd_get_section_by_name (stdoutput, ".debug_line");
  empty_debug_line = line_seg == NULL || !seg_not_empty_p (line_seg);

  /* We can't construct a new debug_line section if we already have one.
     Give an error if we have seen any .loc, otherwise trust the user
     knows what they are doing and want to generate the .debug_line
     (and all other debug sections) themselves.  */
  if (all_segs && !empty_debug_line && dwarf2_any_loc_directive_seen)
    as_fatal ("duplicate .debug_line sections");

  if ((!all_segs && emit_other_sections)
      || (!emit_other_sections && !empty_debug_line))
    /* If there is no line information and no non-empty .debug_info
       section, or if there is both a non-empty .debug_info and a non-empty
       .debug_line, then we do nothing.  */
    {
      dwarf2_cleanup ();
      return;
    }

  /* Calculate the size of an address for the target machine.  */
  sizeof_address = DWARF2_ADDR_SIZE (stdoutput);

  /* Create and switch to the line number section.  */
  if (empty_debug_line)
    {
      line_seg = subseg_new (".debug_line", 0);
      bfd_set_section_flags (line_seg,
			     SEC_READONLY | SEC_DEBUGGING | SEC_OCTETS);
    }

  for (s = all_segs; s; s = s->next)
    {
      struct line_subseg *lss;

      for (lss = s->head; lss; lss = lss->next)
	if (lss->head)
	  do_allocate_filenum (lss->head);
    }

  /* For each subsection, chain the debug entries together.  */
  for (s = all_segs; s; s = s->next)
    {
      struct line_subseg *lss = s->head;
      struct line_entry **ptail = lss->ptail;

      /* Reset the initial view of the first subsection of the
	 section.  */
      if (lss->head && lss->head->loc.u.view)
	set_or_check_view (lss->head, NULL, NULL);

      while ((lss = lss->next) != NULL)
	{
	  /* Link the first view of subsequent subsections to the
	     previous view.  */
	  if (lss->head && lss->head->loc.u.view)
	    set_or_check_view (lss->head,
			       !s->head ? NULL : (struct line_entry *)ptail,
			       s->head ? s->head->head : NULL);
	  *ptail = lss->head;
	  lss->head = NULL;
	  ptail = lss->ptail;
	}
    }

  if (empty_debug_line)
    out_debug_line (line_seg);

  /* If this is assembler generated line info, and there is no
     debug_info already, we need .debug_info, .debug_abbrev and
     .debug_str sections as well.  */
  if (emit_other_sections)
    {
      segT abbrev_seg;
      segT aranges_seg;
      segT str_seg;
      symbolS *name_sym, *comp_dir_sym, *producer_sym, *ranges_sym;
      unsigned char func_form = 0;

      gas_assert (all_segs);

      info_seg = subseg_new (".debug_info", 0);
      abbrev_seg = subseg_new (".debug_abbrev", 0);
      aranges_seg = subseg_new (".debug_aranges", 0);
      str_seg = subseg_new (".debug_str", 0);

      bfd_set_section_flags (info_seg,
			      SEC_READONLY | SEC_DEBUGGING | SEC_OCTETS);
      bfd_set_section_flags (abbrev_seg,
			      SEC_READONLY | SEC_DEBUGGING | SEC_OCTETS);
      bfd_set_section_flags (aranges_seg,
			      SEC_READONLY | SEC_DEBUGGING | SEC_OCTETS);
      bfd_set_section_flags (str_seg,
			      SEC_READONLY | SEC_DEBUGGING | SEC_OCTETS
				       | SEC_MERGE | SEC_STRINGS);
      str_seg->entsize = 1;

      record_alignment (aranges_seg, ffs (2 * sizeof_address) - 1);

      if (all_segs->next == NULL)
	ranges_sym = NULL;
      else
	{
	  if (DWARF2_VERSION < 5)
	    {
	      segT ranges_seg = subseg_new (".debug_ranges", 0);
	      bfd_set_section_flags (ranges_seg, (SEC_READONLY
						  | SEC_DEBUGGING
						  | SEC_OCTETS));
	      record_alignment (ranges_seg, ffs (2 * sizeof_address) - 1);
	      out_debug_ranges (ranges_seg, &ranges_sym);
	    }
	  else
	    {
	      segT rnglists_seg = subseg_new (".debug_rnglists", 0);
	      bfd_set_section_flags (rnglists_seg, (SEC_READONLY
						    | SEC_DEBUGGING
						    | SEC_OCTETS));
	      out_debug_rnglists (rnglists_seg, &ranges_sym);
	    }
	}

      out_debug_aranges (aranges_seg, info_seg);
      out_debug_abbrev (abbrev_seg, info_seg, line_seg, &func_form);
      out_debug_str (str_seg, &name_sym, &comp_dir_sym, &producer_sym);
      out_debug_info (info_seg, abbrev_seg, line_seg, str_seg,
		      ranges_sym, name_sym, comp_dir_sym, producer_sym,
		      func_form);
    }
  dwarf2_cleanup ();
}

/* Perform any deferred checks pertaining to debug information.  */

void
dwarf2dbg_final_check (void)
{
  /* Perform reset-view checks.  Don't evaluate view_assert_failed
     recursively: it could be very deep.  It's a chain of adds, with
     each chain element pointing to the next in X_add_symbol, and
     holding the check value in X_op_symbol.  */
  while (view_assert_failed)
    {
      expressionS *exp;
      symbolS *sym;
      offsetT failed;

      gas_assert (!symbol_resolved_p (view_assert_failed));

      exp = symbol_get_value_expression (view_assert_failed);
      sym = view_assert_failed;

      /* If view_assert_failed looks like a compound check in the
	 chain, break it up.  */
      if (exp->X_op == O_add && exp->X_add_number == 0 && exp->X_unsigned)
	{
	  view_assert_failed = exp->X_add_symbol;
	  sym = exp->X_op_symbol;
	}
      else
	view_assert_failed = NULL;

      failed = resolve_symbol_value (sym);
      if (!symbol_resolved_p (sym) || failed)
	{
	  as_bad (_("view number mismatch"));
	  break;
	}
    }
}
