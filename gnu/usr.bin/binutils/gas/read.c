/* read.c - read a source file -
   Copyright (C) 1986-2023 Free Software Foundation, Inc.

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

/* If your chars aren't 8 bits, you will change this a bit (eg. to 0xFF).
   But then, GNU isn't supposed to run on your machine anyway.
   (RMS is so shortsighted sometimes.)  */
#define MASK_CHAR ((int)(unsigned char) -1)

/* This is the largest known floating point format (for now). It will
   grow when we do 4361 style flonums.  */
#define MAXIMUM_NUMBER_OF_CHARS_FOR_FLOAT (16)

/* Routines that read assembler source text to build spaghetti in memory.
   Another group of these functions is in the expr.c module.  */

#include "as.h"
#include "safe-ctype.h"
#include "subsegs.h"
#include "sb.h"
#include "macro.h"
#include "obstack.h"
#include "ecoff.h"
#include "dw2gencfi.h"
#include "codeview.h"
#include "wchar.h"
#include "filenames.h"

#include <limits.h>

#ifndef TC_START_LABEL
#define TC_START_LABEL(STR, NUL_CHAR, NEXT_CHAR) (NEXT_CHAR == ':')
#endif

/* Set by the object-format or the target.  */
#ifndef TC_IMPLICIT_LCOMM_ALIGNMENT
#define TC_IMPLICIT_LCOMM_ALIGNMENT(SIZE, P2VAR)		\
  do								\
    {								\
      if ((SIZE) >= 8)						\
	(P2VAR) = 3;						\
      else if ((SIZE) >= 4)					\
	(P2VAR) = 2;						\
      else if ((SIZE) >= 2)					\
	(P2VAR) = 1;						\
      else							\
	(P2VAR) = 0;						\
    }								\
  while (0)
#endif

char *input_line_pointer;	/*->next char of source file to parse.  */
bool input_from_string = false;

#if BITS_PER_CHAR != 8
/*  The following table is indexed by[(char)] and will break if
    a char does not have exactly 256 states (hopefully 0:255!)!  */
die horribly;
#endif

#ifndef LEX_AT
#define LEX_AT 0
#endif

#ifndef LEX_BR
/* The RS/6000 assembler uses {,},[,] as parts of symbol names.  */
#define LEX_BR 0
#endif

#ifndef LEX_PCT
/* The Delta 68k assembler permits % inside label names.  */
#define LEX_PCT 0
#endif

#ifndef LEX_QM
/* The PowerPC Windows NT assemblers permits ? inside label names.  */
#define LEX_QM 0
#endif

#ifndef LEX_HASH
/* The IA-64 assembler uses # as a suffix designating a symbol.  We include
   it in the symbol and strip it out in tc_canonicalize_symbol_name.  */
#define LEX_HASH 0
#endif

#ifndef LEX_DOLLAR
#define LEX_DOLLAR 3
#endif

#ifndef LEX_TILDE
/* The Delta 68k assembler permits ~ at start of label names.  */
#define LEX_TILDE 0
#endif

/* Used by is_... macros. our ctype[].  */
char lex_type[256] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* @ABCDEFGHIJKLMNO */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* PQRSTUVWXYZ[\]^_ */
  0, 0, 0, LEX_HASH, LEX_DOLLAR, LEX_PCT, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, /* _!"#$%&'()*+,-./ */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, LEX_QM,	/* 0123456789:;<=>? */
  LEX_AT, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,	/* @ABCDEFGHIJKLMNO */
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, LEX_BR, 0, LEX_BR, 0, 3, /* PQRSTUVWXYZ[\]^_ */
  0, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,	/* `abcdefghijklmno */
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, LEX_BR, 0, LEX_BR, LEX_TILDE, 0, /* pqrstuvwxyz{|}~.  */
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3
};

/* In: a character.
   Out: 1 if this character ends a line.
	2 if this character is a line separator.  */
char is_end_of_line[256] = {
#ifdef CR_EOL
  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0,	/* @abcdefghijklmno */
#else
  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,	/* @abcdefghijklmno */
#endif
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* _!"#$%&'()*+,-./ */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 0123456789:;<=>? */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0	/* */
};

#ifndef TC_CASE_SENSITIVE
char original_case_string[128];
#endif

/* Functions private to this file.  */

static char *buffer;	/* 1st char of each buffer of lines is here.  */
static char *buffer_limit;	/*->1 + last char in buffer.  */

/* TARGET_BYTES_BIG_ENDIAN is required to be defined to either 0 or 1
   in the tc-<CPU>.h file.  See the "Porting GAS" section of the
   internals manual.  */
int target_big_endian = TARGET_BYTES_BIG_ENDIAN;

/* Variables for handling include file directory table.  */

/* Table of pointers to directories to search for .include's.  */
const char **include_dirs;

/* How many are in the table.  */
size_t include_dir_count;

/* Length of longest in table.  */
size_t include_dir_maxlen;

#ifndef WORKING_DOT_WORD
struct broken_word *broken_words;
int new_broken_words;
#endif

/* The current offset into the absolute section.  We don't try to
   build frags in the absolute section, since no data can be stored
   there.  We just keep track of the current offset.  */
addressT abs_section_offset;

/* If this line had an MRI style label, it is stored in this variable.
   This is used by some of the MRI pseudo-ops.  */
symbolS *line_label;

/* This global variable is used to support MRI common sections.  We
   translate such sections into a common symbol.  This variable is
   non-NULL when we are in an MRI common section.  */
symbolS *mri_common_symbol;

/* In MRI mode, after a dc.b pseudo-op with an odd number of bytes, we
   need to align to an even byte boundary unless the next pseudo-op is
   dc.b, ds.b, or dcb.b.  This variable is set to 1 if an alignment
   may be needed.  */
static int mri_pending_align;

/* Record the current function so that we can issue an error message for
   misplaced .func,.endfunc, and also so that .endfunc needs no
   arguments.  */
static char *current_name;
static char *current_label;

#ifndef NO_LISTING
#ifdef OBJ_ELF
static int dwarf_file;
static int dwarf_line;

/* This variable is set to be non-zero if the next string we see might
   be the name of the source file in DWARF debugging information.  See
   the comment in emit_expr for the format we look for.  */
static int dwarf_file_string;
#endif
#endif

/* If the target defines the md_frag_max_var hook then we know
   enough to implement the .bundle_align_mode features.  */
#ifdef md_frag_max_var
# define HANDLE_BUNDLE
#endif

#ifdef HANDLE_BUNDLE
/* .bundle_align_mode sets this.  Normally it's zero.  When nonzero,
   it's the exponent of the bundle size, and aligned instruction bundle
   mode is in effect.  */
static unsigned int bundle_align_p2;

/* These are set by .bundle_lock and .bundle_unlock.  .bundle_lock sets
   bundle_lock_frag to frag_now and then starts a new frag with
   frag_align_code.  At the same time, bundle_lock_frain gets frchain_now,
   so that .bundle_unlock can verify that we didn't change segments.
   .bundle_unlock resets both to NULL.  If we detect a bundling violation,
   then we reset bundle_lock_frchain to NULL as an indicator that we've
   already diagnosed the error with as_bad and don't need a cascade of
   redundant errors, but bundle_lock_frag remains set to indicate that
   we are expecting to see .bundle_unlock.  */
static fragS *bundle_lock_frag;
static frchainS *bundle_lock_frchain;

/* This is incremented by .bundle_lock and decremented by .bundle_unlock,
   to allow nesting.  */
static unsigned int bundle_lock_depth;
#endif

static void do_s_func (int end_p, const char *default_prefix);
static void s_align (int, int);
static void s_altmacro (int);
static void s_bad_end (int);
static void s_reloc (int);
static int hex_float (int, char *);
static segT get_known_segmented_expression (expressionS * expP);
static void pobegin (void);
static void poend (void);
static size_t get_non_macro_line_sb (sb *);
static void generate_file_debug (void);
static char *_find_end_of_line (char *, int, int, int);

void
read_begin (void)
{
  const char *p;

  pobegin ();
  obj_read_begin_hook ();

  obstack_begin (&cond_obstack, chunksize);

#ifndef tc_line_separator_chars
#define tc_line_separator_chars line_separator_chars
#endif
  /* Use machine dependent syntax.  */
  for (p = tc_line_separator_chars; *p; p++)
    is_end_of_line[(unsigned char) *p] = 2;
  /* Use more.  FIXME-SOMEDAY.  */

  if (flag_mri)
    lex_type['?'] = 3;
  stabs_begin ();

#ifndef WORKING_DOT_WORD
  broken_words = NULL;
  new_broken_words = 0;
#endif

  abs_section_offset = 0;

  line_label = NULL;
  mri_common_symbol = NULL;
  mri_pending_align = 0;

  current_name = NULL;
  current_label = NULL;

#ifndef NO_LISTING
#ifdef OBJ_ELF
  dwarf_file = 0;
  dwarf_line = -1;
  dwarf_file_string = 0;
#endif
#endif

#ifdef HANDLE_BUNDLE
  bundle_align_p2 = 0;
  bundle_lock_frag = NULL;
  bundle_lock_frchain = NULL;
  bundle_lock_depth = 0;
#endif
}

void
read_end (void)
{
  stabs_end ();
  poend ();
  _obstack_free (&cond_obstack, NULL);
  free (current_name);
  free (current_label);
}

#ifndef TC_ADDRESS_BYTES
#define TC_ADDRESS_BYTES address_bytes

static inline int
address_bytes (void)
{
  /* Choose smallest of 1, 2, 4, 8 bytes that is large enough to
     contain an address.  */
  int n = (stdoutput->arch_info->bits_per_address - 1) / 8;
  n |= n >> 1;
  n |= n >> 2;
  n += 1;
  return n;
}
#endif

/* Set up pseudo-op tables.  */

static htab_t po_hash;

static const pseudo_typeS potable[] = {
  {"abort", s_abort, 0},
  {"align", s_align_ptwo, 0},
  {"altmacro", s_altmacro, 1},
  {"ascii", stringer, 8+0},
  {"asciz", stringer, 8+1},
  {"balign", s_align_bytes, 0},
  {"balignw", s_align_bytes, -2},
  {"balignl", s_align_bytes, -4},
/* block  */
#ifdef HANDLE_BUNDLE
  {"bundle_align_mode", s_bundle_align_mode, 0},
  {"bundle_lock", s_bundle_lock, 0},
  {"bundle_unlock", s_bundle_unlock, 0},
#endif
  {"byte", cons, 1},
  {"comm", s_comm, 0},
  {"common", s_mri_common, 0},
  {"common.s", s_mri_common, 1},
  {"data", s_data, 0},
  {"dc", cons, 2},
  {"dc.a", cons, 0},
  {"dc.b", cons, 1},
  {"dc.d", float_cons, 'd'},
  {"dc.l", cons, 4},
  {"dc.s", float_cons, 'f'},
  {"dc.w", cons, 2},
  {"dc.x", float_cons, 'x'},
  {"dcb", s_space, 2},
  {"dcb.b", s_space, 1},
  {"dcb.d", s_float_space, 'd'},
  {"dcb.l", s_space, 4},
  {"dcb.s", s_float_space, 'f'},
  {"dcb.w", s_space, 2},
  {"dcb.x", s_float_space, 'x'},
  {"ds", s_space, 2},
  {"ds.b", s_space, 1},
  {"ds.d", s_space, 8},
  {"ds.l", s_space, 4},
  {"ds.p", s_space, 'p'},
  {"ds.s", s_space, 4},
  {"ds.w", s_space, 2},
  {"ds.x", s_space, 'x'},
  {"debug", s_ignore, 0},
#ifdef S_SET_DESC
  {"desc", s_desc, 0},
#endif
/* dim  */
  {"double", float_cons, 'd'},
/* dsect  */
  {"eject", listing_eject, 0},	/* Formfeed listing.  */
  {"else", s_else, 0},
  {"elsec", s_else, 0},
  {"elseif", s_elseif, (int) O_ne},
  {"end", s_end, 0},
  {"endc", s_endif, 0},
  {"endfunc", s_func, 1},
  {"endif", s_endif, 0},
  {"endm", s_bad_end, 0},
  {"endr", s_bad_end, 1},
/* endef  */
  {"equ", s_set, 0},
  {"equiv", s_set, 1},
  {"eqv", s_set, -1},
  {"err", s_err, 0},
  {"error", s_errwarn, 1},
  {"exitm", s_mexit, 0},
/* extend  */
  {"extern", s_ignore, 0},	/* We treat all undef as ext.  */
  {"fail", s_fail, 0},
  {"file", s_file, 0},
  {"fill", s_fill, 0},
  {"float", float_cons, 'f'},
  {"format", s_ignore, 0},
  {"func", s_func, 0},
  {"global", s_globl, 0},
  {"globl", s_globl, 0},
  {"hword", cons, 2},
  {"if", s_if, (int) O_ne},
  {"ifb", s_ifb, 1},
  {"ifc", s_ifc, 0},
  {"ifdef", s_ifdef, 0},
  {"ifeq", s_if, (int) O_eq},
  {"ifeqs", s_ifeqs, 0},
  {"ifge", s_if, (int) O_ge},
  {"ifgt", s_if, (int) O_gt},
  {"ifle", s_if, (int) O_le},
  {"iflt", s_if, (int) O_lt},
  {"ifnb", s_ifb, 0},
  {"ifnc", s_ifc, 1},
  {"ifndef", s_ifdef, 1},
  {"ifne", s_if, (int) O_ne},
  {"ifnes", s_ifeqs, 1},
  {"ifnotdef", s_ifdef, 1},
  {"incbin", s_incbin, 0},
  {"include", s_include, 0},
  {"int", cons, 4},
  {"irp", s_irp, 0},
  {"irep", s_irp, 0},
  {"irpc", s_irp, 1},
  {"irepc", s_irp, 1},
  {"lcomm", s_lcomm, 0},
  {"lflags", s_ignore, 0},	/* Listing flags.  */
  {"linefile", s_linefile, 0},
  {"linkonce", s_linkonce, 0},
  {"list", listing_list, 1},	/* Turn listing on.  */
  {"llen", listing_psize, 1},
  {"long", cons, 4},
  {"lsym", s_lsym, 0},
  {"macro", s_macro, 0},
  {"mexit", s_mexit, 0},
  {"mri", s_mri, 0},
  {".mri", s_mri, 0},	/* Special case so .mri works in MRI mode.  */
  {"name", s_ignore, 0},
  {"noaltmacro", s_altmacro, 0},
  {"noformat", s_ignore, 0},
  {"nolist", listing_list, 0},	/* Turn listing off.  */
  {"nopage", listing_nopage, 0},
  {"nop", s_nop, 0},
  {"nops", s_nops, 0},
  {"octa", cons, 16},
  {"offset", s_struct, 0},
  {"org", s_org, 0},
  {"p2align", s_align_ptwo, 0},
  {"p2alignw", s_align_ptwo, -2},
  {"p2alignl", s_align_ptwo, -4},
  {"page", listing_eject, 0},
  {"plen", listing_psize, 0},
  {"print", s_print, 0},
  {"psize", listing_psize, 0},	/* Set paper size.  */
  {"purgem", s_purgem, 0},
  {"quad", cons, 8},
  {"reloc", s_reloc, 0},
  {"rep", s_rept, 0},
  {"rept", s_rept, 0},
  {"rva", s_rva, 4},
  {"sbttl", listing_title, 1},	/* Subtitle of listing.  */
/* scl  */
/* sect  */
  {"set", s_set, 0},
  {"short", cons, 2},
  {"single", float_cons, 'f'},
/* size  */
  {"space", s_space, 0},
  {"skip", s_space, 0},
  {"sleb128", s_leb128, 1},
  {"spc", s_ignore, 0},
  {"stabd", s_stab, 'd'},
  {"stabn", s_stab, 'n'},
  {"stabs", s_stab, 's'},
  {"string", stringer, 8+1},
  {"string8", stringer, 8+1},
  {"string16", stringer, 16+1},
  {"string32", stringer, 32+1},
  {"string64", stringer, 64+1},
  {"struct", s_struct, 0},
/* tag  */
  {"text", s_text, 0},

  /* This is for gcc to use.  It's only just been added (2/94), so gcc
     won't be able to use it for a while -- probably a year or more.
     But once this has been released, check with gcc maintainers
     before deleting it or even changing the spelling.  */
  {"this_GCC_requires_the_GNU_assembler", s_ignore, 0},
  /* If we're folding case -- done for some targets, not necessarily
     all -- the above string in an input file will be converted to
     this one.  Match it either way...  */
  {"this_gcc_requires_the_gnu_assembler", s_ignore, 0},

  {"title", listing_title, 0},	/* Listing title.  */
  {"ttl", listing_title, 0},
/* type  */
  {"uleb128", s_leb128, 0},
/* use  */
/* val  */
  {"xcom", s_comm, 0},
  {"xdef", s_globl, 0},
  {"xref", s_ignore, 0},
  {"xstabs", s_xstab, 's'},
  {"warning", s_errwarn, 0},
  {"weakref", s_weakref, 0},
  {"word", cons, 2},
  {"zero", s_space, 0},
  {"2byte", cons, 2},
  {"4byte", cons, 4},
  {"8byte", cons, 8},
  {NULL, NULL, 0}			/* End sentinel.  */
};

static offsetT
get_absolute_expr (expressionS *exp)
{
  expression_and_evaluate (exp);

  if (exp->X_op != O_constant)
    {
      if (exp->X_op != O_absent)
	as_bad (_("bad or irreducible absolute expression"));
      exp->X_add_number = 0;
    }
  return exp->X_add_number;
}

offsetT
get_absolute_expression (void)
{
  expressionS exp;

  return get_absolute_expr (&exp);
}

static int pop_override_ok;
static const char *pop_table_name;

void
pop_insert (const pseudo_typeS *table)
{
  const pseudo_typeS *pop;
  for (pop = table; pop->poc_name; pop++)
    {
      if (str_hash_insert (po_hash, pop->poc_name, pop, 0) != NULL)
	{
	  if (!pop_override_ok)
	    as_fatal (_("error constructing %s pseudo-op table"),
		      pop_table_name);
	}
    }
}

#ifndef md_pop_insert
#define md_pop_insert()		pop_insert(md_pseudo_table)
#endif

#ifndef obj_pop_insert
#define obj_pop_insert()	pop_insert(obj_pseudo_table)
#endif

#ifndef cfi_pop_insert
#define cfi_pop_insert()	pop_insert(cfi_pseudo_table)
#endif

static void
pobegin (void)
{
  po_hash = str_htab_create ();

  /* Do the target-specific pseudo ops.  */
  pop_table_name = "md";
  pop_override_ok = 0;
  md_pop_insert ();

  /* Now object specific.  Skip any that were in the target table.  */
  pop_table_name = "obj";
  pop_override_ok = 1;
  obj_pop_insert ();

  /* Now portable ones.  Skip any that we've seen already.  */
  pop_table_name = "standard";
  pop_insert (potable);

  /* Now CFI ones.  */
  pop_table_name = "cfi";
  cfi_pop_insert ();
}

static void
poend (void)
{
  htab_delete (po_hash);
}

#define HANDLE_CONDITIONAL_ASSEMBLY(num_read)				\
  if (ignore_input ())							\
    {									\
      char *eol = find_end_of_line (input_line_pointer - (num_read),	\
				    flag_m68k_mri);			\
      input_line_pointer = (input_line_pointer <= buffer_limit		\
			    && eol >= buffer_limit)			\
			   ? buffer_limit				\
			   : eol + 1;					\
      continue;								\
    }

/* Helper function of read_a_source_file, which tries to expand a macro.  */
static int
try_macro (char term, const char *line)
{
  sb out;
  const char *err;
  macro_entry *macro;

  if (check_macro (line, &out, &err, &macro))
    {
      if (err != NULL)
	as_bad ("%s", err);
      *input_line_pointer++ = term;
      input_scrub_include_sb (&out,
			      input_line_pointer, expanding_macro);
      sb_kill (&out);
      buffer_limit =
	input_scrub_next_buffer (&input_line_pointer);
#ifdef md_macro_info
      md_macro_info (macro);
#endif
      return 1;
    }
  return 0;
}

#ifdef HANDLE_BUNDLE
/* Start a new instruction bundle.  Returns the rs_align_code frag that
   will be used to align the new bundle.  */
static fragS *
start_bundle (void)
{
  fragS *frag = frag_now;

  frag_align_code (0, 0);

  while (frag->fr_type != rs_align_code)
    frag = frag->fr_next;

  gas_assert (frag != frag_now);

  return frag;
}

/* Calculate the maximum size after relaxation of the region starting
   at the given frag and extending through frag_now (which is unfinished).  */
static unsigned int
pending_bundle_size (fragS *frag)
{
  unsigned int offset = frag->fr_fix;
  unsigned int size = 0;

  gas_assert (frag != frag_now);
  gas_assert (frag->fr_type == rs_align_code);

  while (frag != frag_now)
    {
      /* This should only happen in what will later become an error case.  */
      if (frag == NULL)
	return 0;

      size += frag->fr_fix;
      if (frag->fr_type == rs_machine_dependent)
	size += md_frag_max_var (frag);

      frag = frag->fr_next;
    }

  gas_assert (frag == frag_now);
  size += frag_now_fix ();
  if (frag->fr_type == rs_machine_dependent)
    size += md_frag_max_var (frag);

  gas_assert (size >= offset);

  return size - offset;
}

/* Finish off the frag created to ensure bundle alignment.  */
static void
finish_bundle (fragS *frag, unsigned int size)
{
  gas_assert (bundle_align_p2 > 0);
  gas_assert (frag->fr_type == rs_align_code);

  if (size > 1)
    {
      /* If there is more than a single byte, then we need to set up the
	 alignment frag.  Otherwise we leave it at its initial state from
	 calling frag_align_code (0, 0), so that it does nothing.  */
      frag->fr_offset = bundle_align_p2;
      frag->fr_subtype = size - 1;
    }

  /* We do this every time rather than just in s_bundle_align_mode
     so that we catch any affected section without needing hooks all
     over for all paths that do section changes.  It's cheap enough.  */
  if (bundle_align_p2 > OCTETS_PER_BYTE_POWER)
    record_alignment (now_seg, bundle_align_p2 - OCTETS_PER_BYTE_POWER);
}

/* Assemble one instruction.  This takes care of the bundle features
   around calling md_assemble.  */
static void
assemble_one (char *line)
{
  fragS *insn_start_frag = NULL;

  if (bundle_lock_frchain != NULL && bundle_lock_frchain != frchain_now)
    {
      as_bad (_("cannot change section or subsection inside .bundle_lock"));
      /* Clearing this serves as a marker that we have already complained.  */
      bundle_lock_frchain = NULL;
    }

  if (bundle_lock_frchain == NULL && bundle_align_p2 > 0)
    insn_start_frag = start_bundle ();

  md_assemble (line);

  if (bundle_lock_frchain != NULL)
    {
      /* Make sure this hasn't pushed the locked sequence
	 past the bundle size.  */
      unsigned int bundle_size = pending_bundle_size (bundle_lock_frag);
      if (bundle_size > 1U << bundle_align_p2)
	as_bad (_ (".bundle_lock sequence at %u bytes, "
		   "but .bundle_align_mode limit is %u bytes"),
		bundle_size, 1U << bundle_align_p2);
    }
  else if (bundle_align_p2 > 0)
    {
      unsigned int insn_size = pending_bundle_size (insn_start_frag);

      if (insn_size > 1U << bundle_align_p2)
	as_bad (_("single instruction is %u bytes long, "
		  "but .bundle_align_mode limit is %u bytes"),
		insn_size, 1U << bundle_align_p2);

      finish_bundle (insn_start_frag, insn_size);
    }
}

#else  /* !HANDLE_BUNDLE */

# define assemble_one(line) md_assemble(line)

#endif  /* HANDLE_BUNDLE */

static bool
in_bss (void)
{
  flagword flags = bfd_section_flags (now_seg);

  return (flags & SEC_ALLOC) && !(flags & (SEC_LOAD | SEC_HAS_CONTENTS));
}

/* Guts of .align directive:
   N is the power of two to which to align.  A value of zero is accepted but
    ignored: the default alignment of the section will be at least this.
   FILL may be NULL, or it may point to the bytes of the fill pattern.
   LEN is the length of whatever FILL points to, if anything.  If LEN is zero
    but FILL is not NULL then LEN is treated as if it were one.
   MAX is the maximum number of characters to skip when doing the alignment,
    or 0 if there is no maximum.  */

void
do_align (unsigned int n, char *fill, unsigned int len, unsigned int max)
{
  if (now_seg == absolute_section || in_bss ())
    {
      if (fill != NULL)
	while (len-- > 0)
	  if (*fill++ != '\0')
	    {
	      if (now_seg == absolute_section)
		as_warn (_("ignoring fill value in absolute section"));
	      else
		as_warn (_("ignoring fill value in section `%s'"),
			 segment_name (now_seg));
	      break;
	    }
      fill = NULL;
      len = 0;
    }

#ifdef md_flush_pending_output
  md_flush_pending_output ();
#endif

#ifdef md_do_align
  md_do_align (n, fill, len, max, just_record_alignment);
#endif

  /* Only make a frag if we HAVE to...  */
  if ((n > OCTETS_PER_BYTE_POWER) && !need_pass_2)
    {
      if (fill == NULL)
	{
	  if (subseg_text_p (now_seg))
	    frag_align_code (n, max);
	  else
	    frag_align (n, 0, max);
	}
      else if (len <= 1)
	frag_align (n, *fill, max);
      else
	frag_align_pattern (n, fill, len, max);
    }

#ifdef md_do_align
 just_record_alignment: ATTRIBUTE_UNUSED_LABEL
#endif

  if (n > OCTETS_PER_BYTE_POWER)
    record_alignment (now_seg, n - OCTETS_PER_BYTE_POWER);
}

/* We read the file, putting things into a web that represents what we
   have been reading.  */
void
read_a_source_file (const char *name)
{
  char nul_char;
  char next_char;
  char *s;		/* String of symbol, '\0' appended.  */
  long temp;
  const pseudo_typeS *pop;

#ifdef WARN_COMMENTS
  found_comment = 0;
#endif

  buffer = input_scrub_new_file (name);

  listing_file (name);
  listing_newline (NULL);
  register_dependency (name);

  /* Generate debugging information before we've read anything in to denote
     this file as the "main" source file and not a subordinate one
     (e.g. N_SO vs N_SOL in stabs).  */
  generate_file_debug ();

  while ((buffer_limit = input_scrub_next_buffer (&input_line_pointer)) != 0)
    {				/* We have another line to parse.  */
#ifndef NO_LISTING
      /* In order to avoid listing macro expansion lines with labels
	 multiple times, keep track of which line was last issued.  */
      char *last_eol = NULL;

#endif
      while (input_line_pointer < buffer_limit)
	{
	  bool was_new_line;
	  /* We have more of this buffer to parse.  */

	  /* We now have input_line_pointer->1st char of next line.
	     If input_line_pointer [-1] == '\n' then we just
	     scanned another line: so bump line counters.  */
	  was_new_line = is_end_of_line[(unsigned char) input_line_pointer[-1]];
	  if (was_new_line)
	    {
	      symbol_set_value_now (&dot_symbol);
#ifdef md_start_line_hook
	      md_start_line_hook ();
#endif
	      if (input_line_pointer[-1] == '\n')
		bump_line_counters ();
	    }

#ifndef NO_LISTING
	  /* If listing is on, and we are expanding a macro, then give
	     the listing code the contents of the expanded line.  */
	  if (listing)
	    {
	      if ((listing & LISTING_MACEXP) && macro_nest > 0)
		{
		  /* Find the end of the current expanded macro line.  */
		  s = find_end_of_line (input_line_pointer, flag_m68k_mri);

		  if (s != last_eol
		      && !startswith (input_line_pointer,
				      !flag_m68k_mri ? " .linefile "
						     : " linefile "))
		    {
		      char *copy;
		      size_t len;

		      last_eol = s;
		      /* Copy it for safe keeping.  Also give an indication of
			 how much macro nesting is involved at this point.  */
		      len = s - input_line_pointer;
		      copy = XNEWVEC (char, len + macro_nest + 2);
		      memset (copy, '>', macro_nest);
		      copy[macro_nest] = ' ';
		      memcpy (copy + macro_nest + 1, input_line_pointer, len);
		      copy[macro_nest + 1 + len] = '\0';

		      /* Install the line with the listing facility.  */
		      listing_newline (copy);
		    }
		}
	      else
		listing_newline (NULL);
	    }
#endif
	  if (was_new_line)
	    {
	      line_label = NULL;

	      if (LABELS_WITHOUT_COLONS || flag_m68k_mri)
		{
		  next_char = * input_line_pointer;
		  /* Text at the start of a line must be a label, we
		     run down and stick a colon in.  */
		  if (is_name_beginner (next_char) || next_char == '"')
		    {
		      char *line_start;
		      int mri_line_macro;

		      HANDLE_CONDITIONAL_ASSEMBLY (0);

		      nul_char = get_symbol_name (& line_start);
		      next_char = (nul_char == '"' ? input_line_pointer[1] : nul_char);

		      /* In MRI mode, the EQU and MACRO pseudoops must
			 be handled specially.  */
		      mri_line_macro = 0;
		      if (flag_m68k_mri)
			{
			  char *rest = input_line_pointer + 1;

			  if (*rest == ':')
			    ++rest;
			  if (*rest == ' ' || *rest == '\t')
			    ++rest;
			  if ((strncasecmp (rest, "EQU", 3) == 0
			       || strncasecmp (rest, "SET", 3) == 0)
			      && (rest[3] == ' ' || rest[3] == '\t'))
			    {
			      input_line_pointer = rest + 3;
			      equals (line_start,
				      strncasecmp (rest, "SET", 3) == 0);
			      continue;
			    }
			  if (strncasecmp (rest, "MACRO", 5) == 0
			      && (rest[5] == ' '
				  || rest[5] == '\t'
				  || is_end_of_line[(unsigned char) rest[5]]))
			    mri_line_macro = 1;
			}

		      /* In MRI mode, we need to handle the MACRO
			 pseudo-op specially: we don't want to put the
			 symbol in the symbol table.  */
		      if (!mri_line_macro
#ifdef TC_START_LABEL_WITHOUT_COLON
			  && TC_START_LABEL_WITHOUT_COLON (nul_char, next_char)
#endif
			  )
			line_label = colon (line_start);
		      else
			line_label = symbol_create (line_start,
						    absolute_section,
						    &zero_address_frag, 0);

		      next_char = restore_line_pointer (nul_char);
		      if (next_char == ':')
			input_line_pointer++;
		    }
		}
	    }

	  /* We are at the beginning of a line, or similar place.
	     We expect a well-formed assembler statement.
	     A "symbol-name:" is a statement.

	     Depending on what compiler is used, the order of these tests
	     may vary to catch most common case 1st.
	     Each test is independent of all other tests at the (top)
	     level.  */
	  do
	    nul_char = next_char = *input_line_pointer++;
	  while (next_char == '\t' || next_char == ' ' || next_char == '\f');

	  /* C is the 1st significant character.
	     Input_line_pointer points after that character.  */
	  if (is_name_beginner (next_char) || next_char == '"')
	    {
	      char *rest;

	      /* Want user-defined label or pseudo/opcode.  */
	      HANDLE_CONDITIONAL_ASSEMBLY (1);

	      --input_line_pointer;
	      nul_char = get_symbol_name (& s);	/* name's delimiter.  */
	      next_char = (nul_char == '"' ? input_line_pointer[1] : nul_char);
	      rest = input_line_pointer + (nul_char == '"' ? 2 : 1);

	      /* NEXT_CHAR is character after symbol.
		 The end of symbol in the input line is now '\0'.
		 S points to the beginning of the symbol.
		   [In case of pseudo-op, s->'.'.]
		 Input_line_pointer->'\0' where NUL_CHAR was.  */
	      if (TC_START_LABEL (s, nul_char, next_char))
		{
		  if (flag_m68k_mri)
		    {
		      /* In MRI mode, \tsym: set 0 is permitted.  */
		      if (*rest == ':')
			++rest;

		      if (*rest == ' ' || *rest == '\t')
			++rest;

		      if ((strncasecmp (rest, "EQU", 3) == 0
			   || strncasecmp (rest, "SET", 3) == 0)
			  && (rest[3] == ' ' || rest[3] == '\t'))
			{
			  input_line_pointer = rest + 3;
			  equals (s, 1);
			  continue;
			}
		    }

		  line_label = colon (s);	/* User-defined label.  */
		  restore_line_pointer (nul_char);
		  ++ input_line_pointer;
#ifdef tc_check_label
		  tc_check_label (line_label);
#endif
		  /* Input_line_pointer->after ':'.  */
		  SKIP_WHITESPACE ();
		}
	      else if ((next_char == '=' && *rest == '=')
		       || ((next_char == ' ' || next_char == '\t')
			   && rest[0] == '='
			   && rest[1] == '='))
		{
		  equals (s, -1);
		  demand_empty_rest_of_line ();
		}
	      else if ((next_char == '='
		       || ((next_char == ' ' || next_char == '\t')
			    && *rest == '='))
#ifdef TC_EQUAL_IN_INSN
			   && !TC_EQUAL_IN_INSN (next_char, s)
#endif
			   )
		{
		  equals (s, 1);
		  demand_empty_rest_of_line ();
		}
	      else
		{
		  /* Expect pseudo-op or machine instruction.  */
		  pop = NULL;

#ifndef TC_CASE_SENSITIVE
		  {
		    char *s2 = s;

		    strncpy (original_case_string, s2,
			     sizeof (original_case_string) - 1);
		    original_case_string[sizeof (original_case_string) - 1] = 0;

		    while (*s2)
		      {
			*s2 = TOLOWER (*s2);
			s2++;
		      }
		  }
#endif
		  if (NO_PSEUDO_DOT || flag_m68k_mri)
		    {
		      /* The MRI assembler uses pseudo-ops without
			 a period.  */
		      pop = str_hash_find (po_hash, s);
		      if (pop != NULL && pop->poc_handler == NULL)
			pop = NULL;
		    }

		  if (pop != NULL
		      || (!flag_m68k_mri && *s == '.'))
		    {
		      /* PSEUDO - OP.

			 WARNING: next_char may be end-of-line.
			 We lookup the pseudo-op table with s+1 because we
			 already know that the pseudo-op begins with a '.'.  */

		      if (pop == NULL)
			pop = str_hash_find (po_hash, s + 1);
		      if (pop && !pop->poc_handler)
			pop = NULL;

		      /* In MRI mode, we may need to insert an
			 automatic alignment directive.  What a hack
			 this is.  */
		      if (mri_pending_align
			  && (pop == NULL
			      || !((pop->poc_handler == cons
				    && pop->poc_val == 1)
				   || (pop->poc_handler == s_space
				       && pop->poc_val == 1)
#ifdef tc_conditional_pseudoop
				   || tc_conditional_pseudoop (pop)
#endif
				   || pop->poc_handler == s_if
				   || pop->poc_handler == s_ifdef
				   || pop->poc_handler == s_ifc
				   || pop->poc_handler == s_ifeqs
				   || pop->poc_handler == s_else
				   || pop->poc_handler == s_endif
				   || pop->poc_handler == s_globl
				   || pop->poc_handler == s_ignore)))
			{
			  do_align (1, (char *) NULL, 0, 0);
			  mri_pending_align = 0;

			  if (line_label != NULL)
			    {
			      symbol_set_frag (line_label, frag_now);
			      S_SET_VALUE (line_label, frag_now_fix ());
			    }
			}

		      /* Print the error msg now, while we still can.  */
		      if (pop == NULL)
			{
			  char *end = input_line_pointer;

			  (void) restore_line_pointer (nul_char);
			  s_ignore (0);
			  nul_char = next_char = *--input_line_pointer;
			  *input_line_pointer = '\0';
			  if (! macro_defined || ! try_macro (next_char, s))
			    {
			      *end = '\0';
			      as_bad (_("unknown pseudo-op: `%s'"), s);
			      *input_line_pointer++ = nul_char;
			    }
			  continue;
			}

		      /* Put it back for error messages etc.  */
		      next_char = restore_line_pointer (nul_char);
		      /* The following skip of whitespace is compulsory.
			 A well shaped space is sometimes all that separates
			 keyword from operands.  */
		      if (next_char == ' ' || next_char == '\t')
			input_line_pointer++;

		      /* Input_line is restored.
			 Input_line_pointer->1st non-blank char
			 after pseudo-operation.  */
		      (*pop->poc_handler) (pop->poc_val);

		      /* If that was .end, just get out now.  */
		      if (pop->poc_handler == s_end)
			goto quit;
		    }
		  else
		    {
		      /* WARNING: next_char may be end-of-line.  */
		      /* Also: input_line_pointer->`\0` where nul_char was.  */
		      (void) restore_line_pointer (nul_char);
		      input_line_pointer = _find_end_of_line (input_line_pointer, flag_m68k_mri, 1, 0);
		      next_char = nul_char = *input_line_pointer;
		      *input_line_pointer = '\0';

		      generate_lineno_debug ();

		      if (macro_defined && try_macro (next_char, s))
			continue;

		      if (mri_pending_align)
			{
			  do_align (1, (char *) NULL, 0, 0);
			  mri_pending_align = 0;
			  if (line_label != NULL)
			    {
			      symbol_set_frag (line_label, frag_now);
			      S_SET_VALUE (line_label, frag_now_fix ());
			    }
			}

		      assemble_one (s); /* Assemble 1 instruction.  */

		      /* PR 19630: The backend may have set ilp to NULL
			 if it encountered a catastrophic failure.  */
		      if (input_line_pointer == NULL)
			as_fatal (_("unable to continue with assembly."));
 
		      *input_line_pointer++ = nul_char;

		      /* We resume loop AFTER the end-of-line from
			 this instruction.  */
		    }
		}
	      continue;
	    }

	  /* Empty statement?  */
	  if (is_end_of_line[(unsigned char) next_char])
	    continue;

	  if ((LOCAL_LABELS_DOLLAR || LOCAL_LABELS_FB) && ISDIGIT (next_char))
	    {
	      /* local label  ("4:")  */
	      char *backup = input_line_pointer;

	      HANDLE_CONDITIONAL_ASSEMBLY (1);

	      temp = next_char - '0';

	      if (nul_char == '"')
		++ input_line_pointer;

	      /* Read the whole number.  */
	      while (ISDIGIT (*input_line_pointer))
		{
		  const long digit = *input_line_pointer - '0';
		  if (temp > (INT_MAX - digit) / 10)
		    {
		      as_bad (_("local label too large near %s"), backup);
		      temp = -1;
		      break;
		    }
		  temp = temp * 10 + digit;
		  ++input_line_pointer;
		}

	      /* Overflow: stop processing the label.  */
	      if (temp == -1)
		{
		  ignore_rest_of_line ();
		  continue;
		}

	      if (LOCAL_LABELS_DOLLAR
		  && *input_line_pointer == '$'
		  && *(input_line_pointer + 1) == ':')
		{
		  input_line_pointer += 2;

		  if (dollar_label_defined (temp))
		    {
		      as_fatal (_("label \"%ld$\" redefined"), temp);
		    }

		  define_dollar_label (temp);
		  colon (dollar_label_name (temp, 0));
		  continue;
		}

	      if (LOCAL_LABELS_FB
		  && *input_line_pointer++ == ':')
		{
		  fb_label_instance_inc (temp);
		  colon (fb_label_name (temp, 0));
		  continue;
		}

	      input_line_pointer = backup;
	    }

	  if (next_char && strchr (line_comment_chars, next_char))
	    {			/* Its a comment.  Better say APP or NO_APP.  */
	      sb sbuf;
	      char *ends;
	      size_t len;

	      s = input_line_pointer;
	      if (!startswith (s, "APP\n"))
		{
		  /* We ignore it.  */
		  ignore_rest_of_line ();
		  continue;
		}
	      bump_line_counters ();
	      s += 4;

	      ends = strstr (s, "#NO_APP\n");
	      len = ends ? ends - s : buffer_limit - s;

	      sb_build (&sbuf, len + 100);
	      sb_add_buffer (&sbuf, s, len);
	      if (!ends)
		{
		  /* The end of the #APP wasn't in this buffer.  We
		     keep reading in buffers until we find the #NO_APP
		     that goes with this #APP  There is one.  The specs
		     guarantee it...  */
		  do
		    {
		      buffer_limit = input_scrub_next_buffer (&buffer);
		      if (!buffer_limit)
			break;
		      ends = strstr (buffer, "#NO_APP\n");
		      len = ends ? ends - buffer : buffer_limit - buffer;
		      sb_add_buffer (&sbuf, buffer, len);
		    }
		  while (!ends);
		}

	      input_line_pointer = ends ? ends + 8 : NULL;
	      input_scrub_include_sb (&sbuf, input_line_pointer, expanding_none);
	      sb_kill (&sbuf);
	      buffer_limit = input_scrub_next_buffer (&input_line_pointer);
	      continue;
	    }

	  HANDLE_CONDITIONAL_ASSEMBLY (1);

#ifdef tc_unrecognized_line
	  if (tc_unrecognized_line (next_char))
	    continue;
#endif
	  input_line_pointer--;
	  /* Report unknown char as error.  */
	  demand_empty_rest_of_line ();
	}
    }

 quit:
  symbol_set_value_now (&dot_symbol);

#ifdef HANDLE_BUNDLE
  if (bundle_lock_frag != NULL)
    {
      as_bad_where (bundle_lock_frag->fr_file, bundle_lock_frag->fr_line,
		    _(".bundle_lock with no matching .bundle_unlock"));
      bundle_lock_frag = NULL;
      bundle_lock_frchain = NULL;
      bundle_lock_depth = 0;
    }
#endif

#ifdef md_cleanup
  md_cleanup ();
#endif
  /* Close the input file.  */
  input_scrub_close ();
#ifdef WARN_COMMENTS
  {
    if (warn_comment && found_comment)
      as_warn_where (found_comment_file, found_comment,
		     "first comment found here");
  }
#endif
}

/* Convert O_constant expression EXP into the equivalent O_big representation.
   Take the sign of the number from SIGN rather than X_add_number.  */

static void
convert_to_bignum (expressionS *exp, int sign)
{
  valueT value;
  unsigned int i;

  value = exp->X_add_number;
  for (i = 0; i < sizeof (exp->X_add_number) / CHARS_PER_LITTLENUM; i++)
    {
      generic_bignum[i] = value & LITTLENUM_MASK;
      value >>= LITTLENUM_NUMBER_OF_BITS;
    }
  /* Add a sequence of sign bits if the top bit of X_add_number is not
     the sign of the original value.  */
  if ((exp->X_add_number < 0) == !sign)
    generic_bignum[i++] = sign ? LITTLENUM_MASK : 0;
  exp->X_op = O_big;
  exp->X_add_number = i;
}

/* For most MRI pseudo-ops, the line actually ends at the first
   nonquoted space.  This function looks for that point, stuffs a null
   in, and sets *STOPCP to the character that used to be there, and
   returns the location.

   Until I hear otherwise, I am going to assume that this is only true
   for the m68k MRI assembler.  */

char *
mri_comment_field (char *stopcp)
{
  char *s;
#ifdef TC_M68K
  int inquote = 0;

  know (flag_m68k_mri);

  for (s = input_line_pointer;
       ((!is_end_of_line[(unsigned char) *s] && *s != ' ' && *s != '\t')
	|| inquote);
       s++)
    {
      if (*s == '\'')
	inquote = !inquote;
    }
#else
  for (s = input_line_pointer;
       !is_end_of_line[(unsigned char) *s];
       s++)
    ;
#endif
  *stopcp = *s;
  *s = '\0';

  return s;
}

/* Skip to the end of an MRI comment field.  */

void
mri_comment_end (char *stop, int stopc)
{
  know (flag_mri);

  input_line_pointer = stop;
  *stop = stopc;
  while (!is_end_of_line[(unsigned char) *input_line_pointer])
    ++input_line_pointer;
}

void
s_abort (int ignore ATTRIBUTE_UNUSED)
{
  as_fatal (_(".abort detected.  Abandoning ship."));
}

/* Handle the .align pseudo-op.  A positive ARG is a default alignment
   (in bytes).  A negative ARG is the negative of the length of the
   fill pattern.  BYTES_P is non-zero if the alignment value should be
   interpreted as the byte boundary, rather than the power of 2.  */
#ifndef TC_ALIGN_LIMIT
#define TC_ALIGN_LIMIT (stdoutput->arch_info->bits_per_address - 1)
#endif

static void
s_align (signed int arg, int bytes_p)
{
  unsigned int align_limit = TC_ALIGN_LIMIT;
  addressT align;
  char *stop = NULL;
  char stopc = 0;
  offsetT fill = 0;
  unsigned int max;
  int fill_p;

  if (flag_mri)
    stop = mri_comment_field (&stopc);

  if (is_end_of_line[(unsigned char) *input_line_pointer])
    {
      if (arg < 0)
	align = 0;
      else
	align = arg;	/* Default value from pseudo-op table.  */
    }
  else
    {
      align = get_absolute_expression ();
      SKIP_WHITESPACE ();

#ifdef TC_ALIGN_ZERO_IS_DEFAULT
      if (arg > 0 && align == 0)
	align = arg;
#endif
    }

  if (bytes_p)
    {
      /* Convert to a power of 2.  */
      if (align != 0)
	{
	  unsigned int i;

	  for (i = 0; (align & 1) == 0; align >>= 1, ++i)
	    ;
	  if (align != 1)
	    as_bad (_("alignment not a power of 2"));

	  align = i;
	}
    }

  if (align > align_limit)
    {
      align = align_limit;
      as_warn (_("alignment too large: %u assumed"), align_limit);
    }

  if (*input_line_pointer != ',')
    {
      fill_p = 0;
      max = 0;
    }
  else
    {
      ++input_line_pointer;
      if (*input_line_pointer == ',')
	fill_p = 0;
      else
	{
	  fill = get_absolute_expression ();
	  SKIP_WHITESPACE ();
	  fill_p = 1;
	}

      if (*input_line_pointer != ',')
	max = 0;
      else
	{
	  ++input_line_pointer;
	  max = get_absolute_expression ();
	}
    }

  if (!fill_p)
    {
      if (arg < 0)
	as_warn (_("expected fill pattern missing"));
      do_align (align, (char *) NULL, 0, max);
    }
  else
    {
      unsigned int fill_len;

      if (arg >= 0)
	fill_len = 1;
      else
	fill_len = -arg;

      if (fill_len <= 1)
	{
	  char fill_char = 0;

	  fill_char = fill;
	  do_align (align, &fill_char, fill_len, max);
	}
      else
	{
	  char ab[16];

	  if ((size_t) fill_len > sizeof ab)
	    {
	      as_warn (_("fill pattern too long, truncating to %u"),
		       (unsigned) sizeof ab);
	      fill_len = sizeof ab;
	    }

	  md_number_to_chars (ab, fill, fill_len);
	  do_align (align, ab, fill_len, max);
	}
    }

  demand_empty_rest_of_line ();

  if (flag_mri)
    mri_comment_end (stop, stopc);
}

/* Handle the .align pseudo-op on machines where ".align 4" means
   align to a 4 byte boundary.  */

void
s_align_bytes (int arg)
{
  s_align (arg, 1);
}

/* Handle the .align pseudo-op on machines where ".align 4" means align
   to a 2**4 boundary.  */

void
s_align_ptwo (int arg)
{
  s_align (arg, 0);
}

/* Switch in and out of alternate macro mode.  */

static void
s_altmacro (int on)
{
  demand_empty_rest_of_line ();
  flag_macro_alternate = on;
}

/* Read a symbol name from input_line_pointer.

   Stores the symbol name in a buffer and returns a pointer to this buffer.
   The buffer is xalloc'ed.  It is the caller's responsibility to free
   this buffer.

   The name is not left in the i_l_p buffer as it may need processing
   to handle escape characters.

   Advances i_l_p to the next non-whitespace character.

   If a symbol name could not be read, the routine issues an error
   messages, skips to the end of the line and returns NULL.  */

char *
read_symbol_name (void)
{
  char * name;
  char * start;
  char c;

  c = *input_line_pointer++;

  if (c == '"')
    {
#define SYM_NAME_CHUNK_LEN 128
      ptrdiff_t len = SYM_NAME_CHUNK_LEN;
      char * name_end;
      unsigned int C;

      start = name = XNEWVEC (char, len + 1);

      name_end = name + SYM_NAME_CHUNK_LEN;

      while (is_a_char (C = next_char_of_string ()))
	{
	  if (name >= name_end)
	    {
	      ptrdiff_t sofar;

	      sofar = name - start;
	      len += SYM_NAME_CHUNK_LEN;
	      start = XRESIZEVEC (char, start, len + 1);
	      name_end = start + len;
	      name = start + sofar;
	    }

	  *name++ = (char) C;
	}
      *name = 0;

      /* Since quoted symbol names can contain non-ASCII characters,
	 check the string and warn if it cannot be recognised by the
	 current character set.  */
      /* PR 29447: mbstowcs ignores the third (length) parameter when
	 the first (destination) parameter is NULL.  For clarity sake
	 therefore we pass 0 rather than 'len' as the third parameter.  */
      if (mbstowcs (NULL, name, 0) == (size_t) -1)
	as_warn (_("symbol name not recognised in the current locale"));
    }
  else if (is_name_beginner (c) || (input_from_string && c == FAKE_LABEL_CHAR))
    {
      ptrdiff_t len;

      name = input_line_pointer - 1;

      /* We accept FAKE_LABEL_CHAR in a name in case this is
	 being called with a constructed string.  */
      while (is_part_of_name (c = *input_line_pointer++)
	     || (input_from_string && c == FAKE_LABEL_CHAR))
	;

      len = (input_line_pointer - name) - 1;
      start = XNEWVEC (char, len + 1);

      memcpy (start, name, len);
      start[len] = 0;

      /* Skip a name ender char if one is present.  */
      if (! is_name_ender (c))
	--input_line_pointer;
    }
  else
    name = start = NULL;

  if (name == start)
    {
      as_bad (_("expected symbol name"));
      ignore_rest_of_line ();
      free (start);
      return NULL;
    }

  SKIP_WHITESPACE ();

  return start;
}


symbolS *
s_comm_internal (int param,
		 symbolS *(*comm_parse_extra) (int, symbolS *, addressT))
{
  char *name;
  offsetT temp, size;
  symbolS *symbolP = NULL;
  char *stop = NULL;
  char stopc = 0;
  expressionS exp;

  if (flag_mri)
    stop = mri_comment_field (&stopc);

  if ((name = read_symbol_name ()) == NULL)
    goto out;

  /* Accept an optional comma after the name.  The comma used to be
     required, but Irix 5 cc does not generate it for .lcomm.  */
  if (*input_line_pointer == ',')
    input_line_pointer++;

  temp = get_absolute_expr (&exp);
  size = temp;
  size &= ((addressT) 2 << (stdoutput->arch_info->bits_per_address - 1)) - 1;
  if (exp.X_op == O_absent)
    {
      as_bad (_("missing size expression"));
      ignore_rest_of_line ();
      goto out;
    }
  else if (temp != size || (!exp.X_unsigned && exp.X_add_number < 0))
    {
      as_warn (_("size (%ld) out of range, ignored"), (long) temp);
      ignore_rest_of_line ();
      goto out;
    }

  symbolP = symbol_find_or_make (name);
  if ((S_IS_DEFINED (symbolP) || symbol_equated_p (symbolP))
      && !S_IS_COMMON (symbolP))
    {
      if (!S_IS_VOLATILE (symbolP))
	{
	  symbolP = NULL;
	  as_bad (_("symbol `%s' is already defined"), name);
	  ignore_rest_of_line ();
	  goto out;
	}
      symbolP = symbol_clone (symbolP, 1);
      S_SET_SEGMENT (symbolP, undefined_section);
      S_SET_VALUE (symbolP, 0);
      symbol_set_frag (symbolP, &zero_address_frag);
      S_CLEAR_VOLATILE (symbolP);
    }

  size = S_GET_VALUE (symbolP);
  if (size == 0)
    size = temp;
  else if (size != temp)
    as_warn (_("size of \"%s\" is already %ld; not changing to %ld"),
	     name, (long) size, (long) temp);

  if (comm_parse_extra != NULL)
    symbolP = (*comm_parse_extra) (param, symbolP, size);
  else
    {
      S_SET_VALUE (symbolP, (valueT) size);
      S_SET_EXTERNAL (symbolP);
      S_SET_SEGMENT (symbolP, bfd_com_section_ptr);
    }

  demand_empty_rest_of_line ();
 out:
  if (flag_mri)
    mri_comment_end (stop, stopc);
  free (name);
  return symbolP;
}

void
s_comm (int ignore)
{
  s_comm_internal (ignore, NULL);
}

/* The MRI COMMON pseudo-op.  We handle this by creating a common
   symbol with the appropriate name.  We make s_space do the right
   thing by increasing the size.  */

void
s_mri_common (int small ATTRIBUTE_UNUSED)
{
  char *name;
  char c;
  char *alc = NULL;
  symbolS *sym;
  offsetT align;
  char *stop = NULL;
  char stopc = 0;

  if (!flag_mri)
    {
      s_comm (0);
      return;
    }

  stop = mri_comment_field (&stopc);

  SKIP_WHITESPACE ();

  name = input_line_pointer;
  if (!ISDIGIT (*name))
    c = get_symbol_name (& name);
  else
    {
      do
	{
	  ++input_line_pointer;
	}
      while (ISDIGIT (*input_line_pointer));

      c = *input_line_pointer;
      *input_line_pointer = '\0';

      if (line_label != NULL)
	{
	  alc = XNEWVEC (char, strlen (S_GET_NAME (line_label))
			 + (input_line_pointer - name) + 1);
	  sprintf (alc, "%s%s", name, S_GET_NAME (line_label));
	  name = alc;
	}
    }

  sym = symbol_find_or_make (name);
  c = restore_line_pointer (c);
  free (alc);

  if (*input_line_pointer != ',')
    align = 0;
  else
    {
      ++input_line_pointer;
      align = get_absolute_expression ();
    }

  if (S_IS_DEFINED (sym) && !S_IS_COMMON (sym))
    {
      as_bad (_("symbol `%s' is already defined"), S_GET_NAME (sym));
      mri_comment_end (stop, stopc);
      return;
    }

  S_SET_EXTERNAL (sym);
  S_SET_SEGMENT (sym, bfd_com_section_ptr);
  mri_common_symbol = sym;

#ifdef S_SET_ALIGN
  if (align != 0)
    S_SET_ALIGN (sym, align);
#else
  (void) align;
#endif

  if (line_label != NULL)
    {
      expressionS exp;
      exp.X_op = O_symbol;
      exp.X_add_symbol = sym;
      exp.X_add_number = 0;
      symbol_set_value_expression (line_label, &exp);
      symbol_set_frag (line_label, &zero_address_frag);
      S_SET_SEGMENT (line_label, expr_section);
    }

  /* FIXME: We just ignore the small argument, which distinguishes
     COMMON and COMMON.S.  I don't know what we can do about it.  */

  /* Ignore the type and hptype.  */
  if (*input_line_pointer == ',')
    input_line_pointer += 2;
  if (*input_line_pointer == ',')
    input_line_pointer += 2;

  demand_empty_rest_of_line ();

  mri_comment_end (stop, stopc);
}

void
s_data (int ignore ATTRIBUTE_UNUSED)
{
  segT section;
  int temp;

  temp = get_absolute_expression ();
  if (flag_readonly_data_in_text)
    {
      section = text_section;
      temp += 1000;
    }
  else
    section = data_section;

  subseg_set (section, (subsegT) temp);

  demand_empty_rest_of_line ();
}

/* Handle the .file pseudo-op.  This default definition may be overridden by
   the object or CPU specific pseudo-ops.  */

void
s_file_string (char *file)
{
#ifdef LISTING
  if (listing)
    listing_source_file (file);
#endif
  register_dependency (file);
#ifdef obj_app_file
  obj_app_file (file);
#endif
}

void
s_file (int ignore ATTRIBUTE_UNUSED)
{
  char *s;
  int length;

  /* Some assemblers tolerate immediately following '"'.  */
  if ((s = demand_copy_string (&length)) != 0)
    {
      new_logical_line_flags (s, -1, 1);

      /* In MRI mode, the preprocessor may have inserted an extraneous
	 backquote.  */
      if (flag_m68k_mri
	  && *input_line_pointer == '\''
	  && is_end_of_line[(unsigned char) input_line_pointer[1]])
	++input_line_pointer;

      demand_empty_rest_of_line ();
      s_file_string (s);
    }
}

static bool
get_linefile_number (int *flag)
{
  expressionS exp;

  SKIP_WHITESPACE ();

  if (*input_line_pointer < '0' || *input_line_pointer > '9')
    return false;

  /* Don't mistakenly interpret octal numbers as line numbers.  */
  if (*input_line_pointer == '0')
    {
      *flag = 0;
      ++input_line_pointer;
      return true;
    }

  expression_and_evaluate (&exp);
  if (exp.X_op != O_constant)
    return false;

#if defined (BFD64) || LONG_MAX > INT_MAX
  if (exp.X_add_number < INT_MIN || exp.X_add_number > INT_MAX)
    return false;
#endif

  *flag = exp.X_add_number;

  return true;
}

/* Handle the .linefile pseudo-op.  This is automatically generated by
   do_scrub_chars when a preprocessor # line comment is seen.  This
   default definition may be overridden by the object or CPU specific
   pseudo-ops.  */

void
s_linefile (int ignore ATTRIBUTE_UNUSED)
{
  char *file = NULL;
  int linenum, flags = 0;

  /* The given number is that of the next line.  */
  if (!get_linefile_number (&linenum))
    {
      ignore_rest_of_line ();
      return;
    }

  if (linenum < 0)
    /* Some of the back ends can't deal with non-positive line numbers.
       Besides, it's silly.  GCC however will generate a line number of
       zero when it is pre-processing builtins for assembler-with-cpp files:

	  # 0 "<built-in>"

       We do not want to barf on this, especially since such files are used
       in the GCC and GDB testsuites.  So we check for negative line numbers
       rather than non-positive line numbers.  */
    as_warn (_("line numbers must be positive; line number %d rejected"),
	     linenum);
  else
    {
      int length = 0;

      SKIP_WHITESPACE ();

      if (*input_line_pointer == '"')
	file = demand_copy_string (&length);
      else if (*input_line_pointer == '.')
	{
	  /* buffer_and_nest() may insert this form.  */
	  ++input_line_pointer;
	  flags = 1 << 3;
	}

      if (file)
	{
	  int this_flag;

	  while (get_linefile_number (&this_flag))
	    switch (this_flag)
	      {
		/* From GCC's cpp documentation:
		   1: start of a new file.
		   2: returning to a file after having included another file.
		   3: following text comes from a system header file.
		   4: following text should be treated as extern "C".

		   4 is nonsensical for the assembler; 3, we don't care about,
		   so we ignore it just in case a system header file is
		   included while preprocessing assembly.  So 1 and 2 are all
		   we care about, and they are mutually incompatible.
		   new_logical_line_flags() demands this.  */
	      case 1:
	      case 2:
		if (flags && flags != (1 << this_flag))
		  as_warn (_("incompatible flag %i in line directive"),
			   this_flag);
		else
		  flags |= 1 << this_flag;
		break;

	      case 3:
	      case 4:
		/* We ignore these.  */
		break;

	      default:
		as_warn (_("unsupported flag %i in line directive"),
			 this_flag);
		break;
	      }

	  if (!is_end_of_line[(unsigned char)*input_line_pointer])
	    file = NULL;
        }

      if (file || flags)
	{
	  demand_empty_rest_of_line ();

	  /* read_a_source_file() will bump the line number only if the line
	     is terminated by '\n'.  */
	  if (input_line_pointer[-1] == '\n')
	    linenum--;

	  new_logical_line_flags (file, linenum, flags);
#ifdef LISTING
	  if (listing)
	    listing_source_line (linenum);
#endif
	  return;
	}
    }
  ignore_rest_of_line ();
}

/* Handle the .end pseudo-op.  Actually, the real work is done in
   read_a_source_file.  */

void
s_end (int ignore ATTRIBUTE_UNUSED)
{
  if (flag_mri)
    {
      /* The MRI assembler permits the start symbol to follow .end,
	 but we don't support that.  */
      SKIP_WHITESPACE ();
      if (!is_end_of_line[(unsigned char) *input_line_pointer]
	  && *input_line_pointer != '*'
	  && *input_line_pointer != '!')
	as_warn (_("start address not supported"));
    }
}

/* Handle the .err pseudo-op.  */

void
s_err (int ignore ATTRIBUTE_UNUSED)
{
  as_bad (_(".err encountered"));
  demand_empty_rest_of_line ();
}

/* Handle the .error and .warning pseudo-ops.  */

void
s_errwarn (int err)
{
  int len;
  /* The purpose for the conditional assignment is not to
     internationalize the directive itself, but that we need a
     self-contained message, one that can be passed like the
     demand_copy_C_string return value, and with no assumption on the
     location of the name of the directive within the message.  */
  const char *msg
    = (err ? _(".error directive invoked in source file")
       : _(".warning directive invoked in source file"));

  if (!is_it_end_of_statement ())
    {
      if (*input_line_pointer != '\"')
	{
	  as_bad (_("%s argument must be a string"),
		  err ? ".error" : ".warning");
	  ignore_rest_of_line ();
	  return;
	}

      msg = demand_copy_C_string (&len);
      if (msg == NULL)
	return;
    }

  if (err)
    as_bad ("%s", msg);
  else
    as_warn ("%s", msg);
  demand_empty_rest_of_line ();
}

/* Handle the MRI fail pseudo-op.  */

void
s_fail (int ignore ATTRIBUTE_UNUSED)
{
  offsetT temp;
  char *stop = NULL;
  char stopc = 0;

  if (flag_mri)
    stop = mri_comment_field (&stopc);

  temp = get_absolute_expression ();
  if (temp >= 500)
    as_warn (_(".fail %ld encountered"), (long) temp);
  else
    as_bad (_(".fail %ld encountered"), (long) temp);

  demand_empty_rest_of_line ();

  if (flag_mri)
    mri_comment_end (stop, stopc);
}

void
s_fill (int ignore ATTRIBUTE_UNUSED)
{
  expressionS rep_exp;
  long size = 1;
  long fill = 0;
  char *p;

#ifdef md_flush_pending_output
  md_flush_pending_output ();
#endif

#ifdef md_cons_align
  md_cons_align (1);
#endif

  expression (&rep_exp);
  if (*input_line_pointer == ',')
    {
      input_line_pointer++;
      size = get_absolute_expression ();
      if (*input_line_pointer == ',')
	{
	  input_line_pointer++;
	  fill = get_absolute_expression ();
	}
    }

  /* This is to be compatible with BSD 4.2 AS, not for any rational reason.  */
#define BSD_FILL_SIZE_CROCK_8 (8)
  if (size > BSD_FILL_SIZE_CROCK_8)
    {
      as_warn (_(".fill size clamped to %d"), BSD_FILL_SIZE_CROCK_8);
      size = BSD_FILL_SIZE_CROCK_8;
    }
  if (size < 0)
    {
      as_warn (_("size negative; .fill ignored"));
      size = 0;
    }
  else if (rep_exp.X_op == O_constant && rep_exp.X_add_number <= 0)
    {
      if (rep_exp.X_add_number < 0)
	as_warn (_("repeat < 0; .fill ignored"));
      size = 0;
    }
  else if (size && !need_pass_2)
    {
      if (now_seg == absolute_section && rep_exp.X_op != O_constant)
	{
	  as_bad (_("non-constant fill count for absolute section"));
	  size = 0;
	}
      else if (now_seg == absolute_section && fill && rep_exp.X_add_number != 0)
	{
	  as_bad (_("attempt to fill absolute section with non-zero value"));
	  size = 0;
	}
      else if (fill
	       && (rep_exp.X_op != O_constant || rep_exp.X_add_number != 0)
	       && in_bss ())
	{
	  as_bad (_("attempt to fill section `%s' with non-zero value"),
		  segment_name (now_seg));
	  size = 0;
	}
    }

  if (size && !need_pass_2)
    {
      if (now_seg == absolute_section)
	abs_section_offset += rep_exp.X_add_number * size;

      if (rep_exp.X_op == O_constant)
	{
	  p = frag_var (rs_fill, (int) size, (int) size,
			(relax_substateT) 0, (symbolS *) 0,
			(offsetT) rep_exp.X_add_number,
			(char *) 0);
	}
      else
	{
	  /* We don't have a constant repeat count, so we can't use
	     rs_fill.  We can get the same results out of rs_space,
	     but its argument is in bytes, so we must multiply the
	     repeat count by size.  */

	  symbolS *rep_sym;
	  rep_sym = make_expr_symbol (&rep_exp);
	  if (size != 1)
	    {
	      expressionS size_exp;
	      size_exp.X_op = O_constant;
	      size_exp.X_add_number = size;

	      rep_exp.X_op = O_multiply;
	      rep_exp.X_add_symbol = rep_sym;
	      rep_exp.X_op_symbol = make_expr_symbol (&size_exp);
	      rep_exp.X_add_number = 0;
	      rep_sym = make_expr_symbol (&rep_exp);
	    }

	  p = frag_var (rs_space, (int) size, (int) size,
			(relax_substateT) 0, rep_sym, (offsetT) 0, (char *) 0);
	}

      memset (p, 0, (unsigned int) size);

      /* The magic number BSD_FILL_SIZE_CROCK_4 is from BSD 4.2 VAX
	 flavoured AS.  The following bizarre behaviour is to be
	 compatible with above.  I guess they tried to take up to 8
	 bytes from a 4-byte expression and they forgot to sign
	 extend.  */
#define BSD_FILL_SIZE_CROCK_4 (4)
      md_number_to_chars (p, (valueT) fill,
			  (size > BSD_FILL_SIZE_CROCK_4
			   ? BSD_FILL_SIZE_CROCK_4
			   : (int) size));
      /* Note: .fill (),0 emits no frag (since we are asked to .fill 0 bytes)
	 but emits no error message because it seems a legal thing to do.
	 It is a degenerate case of .fill but could be emitted by a
	 compiler.  */
    }
  demand_empty_rest_of_line ();
}

void
s_globl (int ignore ATTRIBUTE_UNUSED)
{
  char *name;
  int c;
  symbolS *symbolP;
  char *stop = NULL;
  char stopc = 0;

  if (flag_mri)
    stop = mri_comment_field (&stopc);

  do
    {
      if ((name = read_symbol_name ()) == NULL)
	return;

      symbolP = symbol_find_or_make (name);
      S_SET_EXTERNAL (symbolP);

      SKIP_WHITESPACE ();
      c = *input_line_pointer;
      if (c == ',')
	{
	  input_line_pointer++;
	  SKIP_WHITESPACE ();
	  if (is_end_of_line[(unsigned char) *input_line_pointer])
	    c = '\n';
	}

      free (name);
    }
  while (c == ',');

  demand_empty_rest_of_line ();

  if (flag_mri)
    mri_comment_end (stop, stopc);
}

/* Handle the MRI IRP and IRPC pseudo-ops.  */

void
s_irp (int irpc)
{
  char * eol;
  const char * file;
  unsigned int line;
  sb s;
  const char *err;
  sb out;

  file = as_where (&line);

  eol = find_end_of_line (input_line_pointer, 0);
  sb_build (&s, eol - input_line_pointer);
  sb_add_buffer (&s, input_line_pointer, eol - input_line_pointer);
  input_line_pointer = eol;

  sb_new (&out);

  err = expand_irp (irpc, 0, &s, &out, get_non_macro_line_sb);
  if (err != NULL)
    as_bad_where (file, line, "%s", err);

  sb_kill (&s);

  input_scrub_include_sb (&out, input_line_pointer, expanding_repeat);
  sb_kill (&out);
  buffer_limit = input_scrub_next_buffer (&input_line_pointer);
}

/* Handle the .linkonce pseudo-op.  This tells the assembler to mark
   the section to only be linked once.  However, this is not supported
   by most object file formats.  This takes an optional argument,
   which is what to do about duplicates.  */

void
s_linkonce (int ignore ATTRIBUTE_UNUSED)
{
  enum linkonce_type type;

  SKIP_WHITESPACE ();

  type = LINKONCE_DISCARD;

  if (!is_end_of_line[(unsigned char) *input_line_pointer])
    {
      char *s;
      char c;

      c = get_symbol_name (& s);
      if (strcasecmp (s, "discard") == 0)
	type = LINKONCE_DISCARD;
      else if (strcasecmp (s, "one_only") == 0)
	type = LINKONCE_ONE_ONLY;
      else if (strcasecmp (s, "same_size") == 0)
	type = LINKONCE_SAME_SIZE;
      else if (strcasecmp (s, "same_contents") == 0)
	type = LINKONCE_SAME_CONTENTS;
      else
	as_warn (_("unrecognized .linkonce type `%s'"), s);

      (void) restore_line_pointer (c);
    }

#ifdef obj_handle_link_once
  obj_handle_link_once (type);
#else /* ! defined (obj_handle_link_once) */
  {
    flagword flags;

    if ((bfd_applicable_section_flags (stdoutput) & SEC_LINK_ONCE) == 0)
      as_warn (_(".linkonce is not supported for this object file format"));

    flags = bfd_section_flags (now_seg);
    flags |= SEC_LINK_ONCE;
    switch (type)
      {
      default:
	abort ();
      case LINKONCE_DISCARD:
	flags |= SEC_LINK_DUPLICATES_DISCARD;
	break;
      case LINKONCE_ONE_ONLY:
	flags |= SEC_LINK_DUPLICATES_ONE_ONLY;
	break;
      case LINKONCE_SAME_SIZE:
	flags |= SEC_LINK_DUPLICATES_SAME_SIZE;
	break;
      case LINKONCE_SAME_CONTENTS:
	flags |= SEC_LINK_DUPLICATES_SAME_CONTENTS;
	break;
      }
    if (!bfd_set_section_flags (now_seg, flags))
      as_bad (_("bfd_set_section_flags: %s"),
	      bfd_errmsg (bfd_get_error ()));
  }
#endif /* ! defined (obj_handle_link_once) */

  demand_empty_rest_of_line ();
}

void
bss_alloc (symbolS *symbolP, addressT size, unsigned int align)
{
  char *pfrag;
  segT current_seg = now_seg;
  subsegT current_subseg = now_subseg;
  segT bss_seg = bss_section;

#if defined (TC_MIPS) || defined (TC_ALPHA)
  if (OUTPUT_FLAVOR == bfd_target_ecoff_flavour
      || OUTPUT_FLAVOR == bfd_target_elf_flavour)
    {
      /* For MIPS and Alpha ECOFF or ELF, small objects are put in .sbss.  */
      if (size <= bfd_get_gp_size (stdoutput))
	{
	  bss_seg = subseg_new (".sbss", 1);
	  seg_info (bss_seg)->bss = 1;
	  if (!bfd_set_section_flags (bss_seg, SEC_ALLOC | SEC_SMALL_DATA))
	    as_warn (_("error setting flags for \".sbss\": %s"),
		     bfd_errmsg (bfd_get_error ()));
	}
    }
#endif
  subseg_set (bss_seg, 1);

  if (align > OCTETS_PER_BYTE_POWER)
    {
      record_alignment (bss_seg, align);
      frag_align (align, 0, 0);
    }

  /* Detach from old frag.  */
  if (S_GET_SEGMENT (symbolP) == bss_seg)
    symbol_get_frag (symbolP)->fr_symbol = NULL;

  symbol_set_frag (symbolP, frag_now);
  pfrag = frag_var (rs_org, 1, 1, 0, symbolP, size * OCTETS_PER_BYTE, NULL);
  *pfrag = 0;

#ifdef S_SET_SIZE
  S_SET_SIZE (symbolP, size);
#endif
  S_SET_SEGMENT (symbolP, bss_seg);

#ifdef OBJ_COFF
  /* The symbol may already have been created with a preceding
     ".globl" directive -- be careful not to step on storage class
     in that case.  Otherwise, set it to static.  */
  if (S_GET_STORAGE_CLASS (symbolP) != C_EXT)
    S_SET_STORAGE_CLASS (symbolP, C_STAT);
#endif /* OBJ_COFF */

  subseg_set (current_seg, current_subseg);
}

offsetT
parse_align (int align_bytes)
{
  expressionS exp;
  addressT align;

  SKIP_WHITESPACE ();
  if (*input_line_pointer != ',')
    {
    no_align:
      as_bad (_("expected alignment after size"));
      ignore_rest_of_line ();
      return -1;
    }

  input_line_pointer++;
  SKIP_WHITESPACE ();

  align = get_absolute_expr (&exp);
  if (exp.X_op == O_absent)
    goto no_align;

  if (!exp.X_unsigned && exp.X_add_number < 0)
    {
      as_warn (_("alignment negative; 0 assumed"));
      align = 0;
    }

  if (align_bytes && align != 0)
    {
      /* convert to a power of 2 alignment */
      unsigned int alignp2 = 0;
      while ((align & 1) == 0)
	align >>= 1, ++alignp2;
      if (align != 1)
	{
	  as_bad (_("alignment not a power of 2"));
	  ignore_rest_of_line ();
	  return -1;
	}
      align = alignp2;
    }
  return align;
}

/* Called from s_comm_internal after symbol name and size have been
   parsed.  NEEDS_ALIGN is 0 if it was an ".lcomm" (2 args only),
   1 if this was a ".bss" directive which has a 3rd argument
   (alignment as a power of 2), or 2 if this was a ".bss" directive
   with alignment in bytes.  */

symbolS *
s_lcomm_internal (int needs_align, symbolS *symbolP, addressT size)
{
  addressT align = 0;

  if (needs_align)
    {
      align = parse_align (needs_align - 1);
      if (align == (addressT) -1)
	return NULL;
    }
  else
    /* Assume some objects may require alignment on some systems.  */
    TC_IMPLICIT_LCOMM_ALIGNMENT (size, align);

  bss_alloc (symbolP, size, align);
  return symbolP;
}

void
s_lcomm (int needs_align)
{
  s_comm_internal (needs_align, s_lcomm_internal);
}

void
s_lcomm_bytes (int needs_align)
{
  s_comm_internal (needs_align * 2, s_lcomm_internal);
}

void
s_lsym (int ignore ATTRIBUTE_UNUSED)
{
  char *name;
  expressionS exp;
  symbolS *symbolP;

  /* We permit ANY defined expression: BSD4.2 demands constants.  */
  if ((name = read_symbol_name ()) == NULL)
    return;

  if (*input_line_pointer != ',')
    {
      as_bad (_("expected comma after \"%s\""), name);
      goto err_out;
    }

  input_line_pointer++;
  expression_and_evaluate (&exp);

  if (exp.X_op != O_constant
      && exp.X_op != O_register)
    {
      as_bad (_("bad expression"));
      goto err_out;
    }

  symbolP = symbol_find_or_make (name);

  if (S_GET_SEGMENT (symbolP) == undefined_section)
    {
      /* The name might be an undefined .global symbol; be sure to
	 keep the "external" bit.  */
      S_SET_SEGMENT (symbolP,
		     (exp.X_op == O_constant
		      ? absolute_section
		      : reg_section));
      S_SET_VALUE (symbolP, (valueT) exp.X_add_number);
    }
  else
    {
      as_bad (_("symbol `%s' is already defined"), name);
    }

  demand_empty_rest_of_line ();
  free (name);
  return;

 err_out:
  ignore_rest_of_line ();
  free (name);
  return;
}

/* Read a line into an sb.  Returns the character that ended the line
   or zero if there are no more lines.  */

static int
get_line_sb (sb *line, int in_macro)
{
  char *eol;

  if (input_line_pointer[-1] == '\n')
    bump_line_counters ();

  if (input_line_pointer >= buffer_limit)
    {
      buffer_limit = input_scrub_next_buffer (&input_line_pointer);
      if (buffer_limit == 0)
	return 0;
    }

  eol = _find_end_of_line (input_line_pointer, flag_m68k_mri, 0, in_macro);
  sb_add_buffer (line, input_line_pointer, eol - input_line_pointer);
  input_line_pointer = eol;

  /* Don't skip multiple end-of-line characters, because that breaks support
     for the IA-64 stop bit (;;) which looks like two consecutive end-of-line
     characters but isn't.  Instead just skip one end of line character and
     return the character skipped so that the caller can re-insert it if
     necessary.   */
  return *input_line_pointer++;
}

static size_t
get_non_macro_line_sb (sb *line)
{
  return get_line_sb (line, 0);
}

static size_t
get_macro_line_sb (sb *line)
{
  return get_line_sb (line, 1);
}

/* Define a macro.  This is an interface to macro.c.  */

void
s_macro (int ignore ATTRIBUTE_UNUSED)
{
  char *eol;
  sb s;
  macro_entry *macro;

  eol = find_end_of_line (input_line_pointer, 0);
  sb_build (&s, eol - input_line_pointer);
  sb_add_buffer (&s, input_line_pointer, eol - input_line_pointer);
  input_line_pointer = eol;

  if (line_label != NULL)
    {
      sb label;
      size_t len;
      const char *name;

      name = S_GET_NAME (line_label);
      len = strlen (name);
      sb_build (&label, len);
      sb_add_buffer (&label, name, len);
      macro = define_macro (&s, &label, get_macro_line_sb);
      sb_kill (&label);
    }
  else
    macro = define_macro (&s, NULL, get_macro_line_sb);
  if (macro != NULL)
    {
      if (line_label != NULL)
	{
	  S_SET_SEGMENT (line_label, absolute_section);
	  S_SET_VALUE (line_label, 0);
	  symbol_set_frag (line_label, &zero_address_frag);
	}

      if (((NO_PSEUDO_DOT || flag_m68k_mri)
	   && str_hash_find (po_hash, macro->name) != NULL)
	  || (!flag_m68k_mri
	      && macro->name[0] == '.'
	      && str_hash_find (po_hash, macro->name + 1) != NULL))
	{
	  as_warn_where (macro->file, macro->line,
			 _("attempt to redefine pseudo-op `%s' ignored"),
			 macro->name);
	  str_hash_delete (macro_hash, macro->name);
	}
    }

  sb_kill (&s);
}

/* Handle the .mexit pseudo-op, which immediately exits a macro
   expansion.  */

void
s_mexit (int ignore ATTRIBUTE_UNUSED)
{
  if (macro_nest)
    {
      cond_exit_macro (macro_nest);
      buffer_limit = input_scrub_next_buffer (&input_line_pointer);
    }
  else
    as_warn (_("ignoring macro exit outside a macro definition."));
}

/* Switch in and out of MRI mode.  */

void
s_mri (int ignore ATTRIBUTE_UNUSED)
{
  int on;
#ifdef MRI_MODE_CHANGE
  int old_flag;
#endif

  on = get_absolute_expression ();
#ifdef MRI_MODE_CHANGE
  old_flag = flag_mri;
#endif
  if (on != 0)
    {
      flag_mri = 1;
#ifdef TC_M68K
      flag_m68k_mri = 1;
#endif
    }
  else
    {
      flag_mri = 0;
#ifdef TC_M68K
      flag_m68k_mri = 0;
#endif
    }

  /* Operator precedence changes in m68k MRI mode, so we need to
     update the operator rankings.  */
  expr_set_precedence ();

#ifdef MRI_MODE_CHANGE
  if (on != old_flag)
    MRI_MODE_CHANGE (on);
#endif

  demand_empty_rest_of_line ();
}

/* Handle changing the location counter.  */

static void
do_org (segT segment, expressionS *exp, int fill)
{
  if (segment != now_seg
      && segment != absolute_section
      && segment != expr_section)
    as_bad (_("invalid segment \"%s\""), segment_name (segment));

  if (now_seg == absolute_section)
    {
      if (fill != 0)
	as_warn (_("ignoring fill value in absolute section"));
      if (exp->X_op != O_constant)
	{
	  as_bad (_("only constant offsets supported in absolute section"));
	  exp->X_add_number = 0;
	}
      abs_section_offset = exp->X_add_number;
    }
  else
    {
      char *p;
      symbolS *sym = exp->X_add_symbol;
      offsetT off = exp->X_add_number * OCTETS_PER_BYTE;

      if (fill && in_bss ())
	as_warn (_("ignoring fill value in section `%s'"),
		 segment_name (now_seg));

      if (exp->X_op != O_constant && exp->X_op != O_symbol)
	{
	  /* Handle complex expressions.  */
	  sym = make_expr_symbol (exp);
	  off = 0;
	}

      p = frag_var (rs_org, 1, 1, (relax_substateT) 0, sym, off, (char *) 0);
      *p = fill;
    }
}

void
s_org (int ignore ATTRIBUTE_UNUSED)
{
  segT segment;
  expressionS exp;
  long temp_fill;

#ifdef md_flush_pending_output
  md_flush_pending_output ();
#endif

  /* The m68k MRI assembler has a different meaning for .org.  It
     means to create an absolute section at a given address.  We can't
     support that--use a linker script instead.  */
  if (flag_m68k_mri)
    {
      as_bad (_("MRI style ORG pseudo-op not supported"));
      ignore_rest_of_line ();
      return;
    }

  /* Don't believe the documentation of BSD 4.2 AS.  There is no such
     thing as a sub-segment-relative origin.  Any absolute origin is
     given a warning, then assumed to be segment-relative.  Any
     segmented origin expression ("foo+42") had better be in the right
     segment or the .org is ignored.

     BSD 4.2 AS warns if you try to .org backwards. We cannot because
     we never know sub-segment sizes when we are reading code.  BSD
     will crash trying to emit negative numbers of filler bytes in
     certain .orgs. We don't crash, but see as-write for that code.

     Don't make frag if need_pass_2==1.  */
  segment = get_known_segmented_expression (&exp);
  if (*input_line_pointer == ',')
    {
      input_line_pointer++;
      temp_fill = get_absolute_expression ();
    }
  else
    temp_fill = 0;

  if (!need_pass_2)
    do_org (segment, &exp, temp_fill);

  demand_empty_rest_of_line ();
}

/* Handle parsing for the MRI SECT/SECTION pseudo-op.  This should be
   called by the obj-format routine which handles section changing
   when in MRI mode.  It will create a new section, and return it.  It
   will set *TYPE to the section type: one of 'C' (code), 'D' (data),
   'M' (mixed), or 'R' (romable).  The flags will be set in the section.  */

void
s_mri_sect (char *type ATTRIBUTE_UNUSED)
{
#ifdef TC_M68K

  char *name;
  char c;
  segT seg;

  SKIP_WHITESPACE ();

  name = input_line_pointer;
  if (!ISDIGIT (*name))
    c = get_symbol_name (& name);
  else
    {
      do
	{
	  ++input_line_pointer;
	}
      while (ISDIGIT (*input_line_pointer));

      c = *input_line_pointer;
      *input_line_pointer = '\0';
    }

  name = xstrdup (name);

  c = restore_line_pointer (c);

  seg = subseg_new (name, 0);

  if (c == ',')
    {
      unsigned int align;

      ++input_line_pointer;
      align = get_absolute_expression ();
      record_alignment (seg, align);
    }

  *type = 'C';
  if (*input_line_pointer == ',')
    {
      c = *++input_line_pointer;
      c = TOUPPER (c);
      if (c == 'C' || c == 'D' || c == 'M' || c == 'R')
	*type = c;
      else
	as_bad (_("unrecognized section type"));
      ++input_line_pointer;

      {
	flagword flags;

	flags = SEC_NO_FLAGS;
	if (*type == 'C')
	  flags = SEC_ALLOC | SEC_LOAD | SEC_READONLY | SEC_CODE;
	else if (*type == 'D' || *type == 'M')
	  flags = SEC_ALLOC | SEC_LOAD | SEC_DATA;
	else if (*type == 'R')
	  flags = SEC_ALLOC | SEC_LOAD | SEC_DATA | SEC_READONLY | SEC_ROM;
	if (flags != SEC_NO_FLAGS)
	  {
	    if (!bfd_set_section_flags (seg, flags))
	      as_warn (_("error setting flags for \"%s\": %s"),
		       bfd_section_name (seg),
		       bfd_errmsg (bfd_get_error ()));
	  }
      }
    }

  /* Ignore the HP type.  */
  if (*input_line_pointer == ',')
    input_line_pointer += 2;

  demand_empty_rest_of_line ();

#else /* ! TC_M68K */
  /* The MRI assembler seems to use different forms of .sect for
     different targets.  */
  as_bad ("MRI mode not supported for this target");
  ignore_rest_of_line ();
#endif /* ! TC_M68K */
}

/* Handle the .print pseudo-op.  */

void
s_print (int ignore ATTRIBUTE_UNUSED)
{
  char *s;
  int len;

  s = demand_copy_C_string (&len);
  if (s != NULL)
    printf ("%s\n", s);
  demand_empty_rest_of_line ();
}

/* Handle the .purgem pseudo-op.  */

void
s_purgem (int ignore ATTRIBUTE_UNUSED)
{
  if (is_it_end_of_statement ())
    {
      demand_empty_rest_of_line ();
      return;
    }

  do
    {
      char *name;
      char c;

      SKIP_WHITESPACE ();
      c = get_symbol_name (& name);
      delete_macro (name);
      *input_line_pointer = c;
      SKIP_WHITESPACE_AFTER_NAME ();
    }
  while (*input_line_pointer++ == ',');

  --input_line_pointer;
  demand_empty_rest_of_line ();
}

/* Handle the .endm/.endr pseudo-ops.  */

static void
s_bad_end (int endr)
{
  as_warn (_(".end%c encountered without preceding %s"),
	   endr ? 'r' : 'm',
	   endr ? ".rept, .irp, or .irpc" : ".macro");
  demand_empty_rest_of_line ();
}

/* Handle the .rept pseudo-op.  */

void
s_rept (int ignore ATTRIBUTE_UNUSED)
{
  size_t count;

  count = (size_t) get_absolute_expression ();

  do_repeat (count, "REPT", "ENDR", NULL);
}

/* This function provides a generic repeat block implementation.   It allows
   different directives to be used as the start/end keys.  Any text matching
   the optional EXPANDER in the block is replaced by the remaining iteration
   count.  */

void
do_repeat (size_t count, const char *start, const char *end,
	   const char *expander)
{
  sb one;
  sb many;

  if (((ssize_t) count) < 0)
    {
      as_bad (_("negative count for %s - ignored"), start);
      count = 0;
    }

  sb_new (&one);
  if (!buffer_and_nest (start, end, &one, get_non_macro_line_sb))
    {
      as_bad (_("%s without %s"), start, end);
      sb_kill (&one);
      return;
    }

  if (expander == NULL || strstr (one.ptr, expander) == NULL)
    {
      sb_build (&many, count * one.len);
      while (count-- > 0)
	sb_add_sb (&many, &one);
    }
  else
    {
      sb_new (&many);

      while (count -- > 0)
	{
	  int len;
	  char * sub;
	  sb processed;

	  sb_build (& processed, one.len);
	  sb_add_sb (& processed, & one);
	  sub = strstr (processed.ptr, expander);
	  len = sprintf (sub, "%lu", (unsigned long) count);
	  gas_assert (len < 8);
	  memmove (sub + len, sub + 8,
		   processed.ptr + processed.len - (sub + 8));
	  processed.len -= (8 - len);
	  sb_add_sb (& many, & processed);
	  sb_kill (& processed);
	}
    }

  sb_kill (&one);

  input_scrub_include_sb (&many, input_line_pointer, expanding_repeat);
  sb_kill (&many);
  buffer_limit = input_scrub_next_buffer (&input_line_pointer);
}

/* Skip to end of current repeat loop; EXTRA indicates how many additional
   input buffers to skip.  Assumes that conditionals preceding the loop end
   are properly nested.

   This function makes it easier to implement a premature "break" out of the
   loop.  The EXTRA arg accounts for other buffers we might have inserted,
   such as line substitutions.  */

void
end_repeat (int extra)
{
  cond_exit_macro (macro_nest);
  while (extra-- >= 0)
    buffer_limit = input_scrub_next_buffer (&input_line_pointer);
}

static void
assign_symbol (char *name, int mode)
{
  symbolS *symbolP;

  if (name[0] == '.' && name[1] == '\0')
    {
      /* Turn '. = mumble' into a .org mumble.  */
      segT segment;
      expressionS exp;

      segment = get_known_segmented_expression (&exp);

      if (!need_pass_2)
	do_org (segment, &exp, 0);

      return;
    }

  if ((symbolP = symbol_find (name)) == NULL
      && (symbolP = md_undefined_symbol (name)) == NULL)
    {
      symbolP = symbol_find_or_make (name);
#ifndef NO_LISTING
      /* When doing symbol listings, play games with dummy fragments living
	 outside the normal fragment chain to record the file and line info
	 for this symbol.  */
      if (listing & LISTING_SYMBOLS)
	{
	  extern struct list_info_struct *listing_tail;
	  fragS *dummy_frag = notes_calloc (1, sizeof (*dummy_frag));
	  dummy_frag->line = listing_tail;
	  dummy_frag->fr_symbol = symbolP;
	  symbol_set_frag (symbolP, dummy_frag);
	}
#endif
#if defined (OBJ_COFF) && !defined (TE_PE)
      /* "set" symbols are local unless otherwise specified.  */
      SF_SET_LOCAL (symbolP);
#endif
    }

  if (S_IS_DEFINED (symbolP) || symbol_equated_p (symbolP))
    {
      if ((mode != 0 || !S_IS_VOLATILE (symbolP))
	  && !S_CAN_BE_REDEFINED (symbolP))
	{
	  as_bad (_("symbol `%s' is already defined"), name);
	  ignore_rest_of_line ();
	  input_line_pointer--;
	  return;
	}
      /* If the symbol is volatile, copy the symbol and replace the
	 original with the copy, so that previous uses of the symbol will
	 retain the value of the symbol at the point of use.  */
      else if (S_IS_VOLATILE (symbolP))
	symbolP = symbol_clone (symbolP, 1);
    }

  if (mode == 0)
    S_SET_VOLATILE (symbolP);
  else if (mode < 0)
    S_SET_FORWARD_REF (symbolP);

  pseudo_set (symbolP);
}

/* Handle the .equ, .equiv, .eqv, and .set directives.  If EQUIV is 1,
   then this is .equiv, and it is an error if the symbol is already
   defined.  If EQUIV is -1, the symbol additionally is a forward
   reference.  */

void
s_set (int equiv)
{
  char *name;

  /* Especial apologies for the random logic:
     this just grew, and could be parsed much more simply!
     Dean in haste.  */
  if ((name = read_symbol_name ()) == NULL)
    return;

  if (*input_line_pointer != ',')
    {
      as_bad (_("expected comma after \"%s\""), name);
      ignore_rest_of_line ();
      free (name);
      return;
    }

  input_line_pointer++;
  assign_symbol (name, equiv);
  demand_empty_rest_of_line ();
  free (name);
}

void
s_space (int mult)
{
  expressionS exp;
  expressionS val;
  char *p = 0;
  char *stop = NULL;
  char stopc = 0;
  int bytes;

#ifdef md_flush_pending_output
  md_flush_pending_output ();
#endif

  switch (mult)
    {
    case 'x':
#ifdef X_PRECISION
# ifndef P_PRECISION
#  define P_PRECISION     X_PRECISION
#  define P_PRECISION_PAD X_PRECISION_PAD
# endif
      mult = (X_PRECISION + X_PRECISION_PAD) * sizeof (LITTLENUM_TYPE);
      if (!mult)
#endif
	mult = 12;
      break;

    case 'p':
#ifdef P_PRECISION
      mult = (P_PRECISION + P_PRECISION_PAD) * sizeof (LITTLENUM_TYPE);
      if (!mult)
#endif
	mult = 12;
      break;
    }

#ifdef md_cons_align
  md_cons_align (1);
#endif

  if (flag_mri)
    stop = mri_comment_field (&stopc);

  /* In m68k MRI mode, we need to align to a word boundary, unless
     this is ds.b.  */
  if (flag_m68k_mri && mult > 1)
    {
      if (now_seg == absolute_section)
	{
	  abs_section_offset += abs_section_offset & 1;
	  if (line_label != NULL)
	    S_SET_VALUE (line_label, abs_section_offset);
	}
      else if (mri_common_symbol != NULL)
	{
	  valueT mri_val;

	  mri_val = S_GET_VALUE (mri_common_symbol);
	  if ((mri_val & 1) != 0)
	    {
	      S_SET_VALUE (mri_common_symbol, mri_val + 1);
	      if (line_label != NULL)
		{
		  expressionS *symexp;

		  symexp = symbol_get_value_expression (line_label);
		  know (symexp->X_op == O_symbol);
		  know (symexp->X_add_symbol == mri_common_symbol);
		  symexp->X_add_number += 1;
		}
	    }
	}
      else
	{
	  do_align (1, (char *) NULL, 0, 0);
	  if (line_label != NULL)
	    {
	      symbol_set_frag (line_label, frag_now);
	      S_SET_VALUE (line_label, frag_now_fix ());
	    }
	}
    }

  bytes = mult;

  expression (&exp);

  SKIP_WHITESPACE ();
  if (*input_line_pointer == ',')
    {
      ++input_line_pointer;
      expression (&val);
    }
  else
    {
      val.X_op = O_constant;
      val.X_add_number = 0;
    }

  if ((val.X_op != O_constant
       || val.X_add_number < - 0x80
       || val.X_add_number > 0xff
       || (mult != 0 && mult != 1 && val.X_add_number != 0))
      && (now_seg != absolute_section && !in_bss ()))
    {
      resolve_expression (&exp);
      if (exp.X_op != O_constant)
	as_bad (_("unsupported variable size or fill value"));
      else
	{
	  offsetT i;

	  /* PR 20901: Check for excessive values.
	     FIXME: 1<<10 is an arbitrary limit.  Maybe use maxpagesize instead ?  */
	  if (exp.X_add_number < 0 || exp.X_add_number > (1 << 10))
	    as_bad (_("size value for space directive too large: %lx"),
		    (long) exp.X_add_number);
	  else
	    {
	      if (mult == 0)
		mult = 1;
	      bytes = mult * exp.X_add_number;

	      for (i = 0; i < exp.X_add_number; i++)
		emit_expr (&val, mult);
	    }
	}
    }
  else
    {
      if (now_seg == absolute_section || mri_common_symbol != NULL)
	resolve_expression (&exp);

      if (exp.X_op == O_constant)
	{
	  addressT repeat = exp.X_add_number;
	  addressT total;

	  bytes = 0;
	  if ((offsetT) repeat < 0)
	    {
	      as_warn (_(".space repeat count is negative, ignored"));
	      goto getout;
	    }
	  if (repeat == 0)
	    {
	      if (!flag_mri)
		as_warn (_(".space repeat count is zero, ignored"));
	      goto getout;
	    }
	  if ((unsigned int) mult <= 1)
	    total = repeat;
	  else if (gas_mul_overflow (repeat, mult, &total)
		   || (offsetT) total < 0)
	    {
	      as_warn (_(".space repeat count overflow, ignored"));
	      goto getout;
	    }
	  bytes = total;

	  /* If we are in the absolute section, just bump the offset.  */
	  if (now_seg == absolute_section)
	    {
	      if (val.X_op != O_constant || val.X_add_number != 0)
		as_warn (_("ignoring fill value in absolute section"));
	      abs_section_offset += total;
	      goto getout;
	    }

	  /* If we are secretly in an MRI common section, then
	     creating space just increases the size of the common
	     symbol.  */
	  if (mri_common_symbol != NULL)
	    {
	      S_SET_VALUE (mri_common_symbol,
			   S_GET_VALUE (mri_common_symbol) + total);
	      goto getout;
	    }

	  if (!need_pass_2)
	    p = frag_var (rs_fill, 1, 1, (relax_substateT) 0, (symbolS *) 0,
			  (offsetT) total, (char *) 0);
	}
      else
	{
	  if (now_seg == absolute_section)
	    {
	      as_bad (_("space allocation too complex in absolute section"));
	      subseg_set (text_section, 0);
	    }

	  if (mri_common_symbol != NULL)
	    {
	      as_bad (_("space allocation too complex in common section"));
	      mri_common_symbol = NULL;
	    }

	  if (!need_pass_2)
	    p = frag_var (rs_space, 1, 1, (relax_substateT) 0,
			  make_expr_symbol (&exp), (offsetT) 0, (char *) 0);
	}

      if ((val.X_op != O_constant || val.X_add_number != 0) && in_bss ())
	as_warn (_("ignoring fill value in section `%s'"),
		 segment_name (now_seg));
      else if (p)
	*p = val.X_add_number;
    }

 getout:

  /* In MRI mode, after an odd number of bytes, we must align to an
     even word boundary, unless the next instruction is a dc.b, ds.b
     or dcb.b.  */
  if (flag_mri && (bytes & 1) != 0)
    mri_pending_align = 1;

  demand_empty_rest_of_line ();

  if (flag_mri)
    mri_comment_end (stop, stopc);
}

void
s_nop (int ignore ATTRIBUTE_UNUSED)
{
  expressionS exp;
  fragS *start;
  addressT start_off;
  offsetT frag_off;

#ifdef md_flush_pending_output
  md_flush_pending_output ();
#endif

#ifdef md_cons_align
  md_cons_align (1);
#endif

  SKIP_WHITESPACE ();
  expression (&exp);
  demand_empty_rest_of_line ();

  start = frag_now;
  start_off = frag_now_fix ();
  do
    {
#ifdef md_emit_single_noop
      md_emit_single_noop;
#else
      char *nop;

#ifndef md_single_noop_insn
#define md_single_noop_insn "nop"
#endif
      /* md_assemble might modify its argument, so
	 we must pass it a string that is writable.  */
      if (asprintf (&nop, "%s", md_single_noop_insn) < 0)
	as_fatal ("%s", xstrerror (errno));

      /* Some targets assume that they can update input_line_pointer
	 inside md_assemble, and, worse, that they can leave it
	 assigned to the string pointer that was provided as an
	 argument.  So preserve ilp here.  */
      char *saved_ilp = input_line_pointer;
      md_assemble (nop);
      input_line_pointer = saved_ilp;
      free (nop);
#endif
#ifdef md_flush_pending_output
      md_flush_pending_output ();
#endif
    } while (exp.X_op == O_constant
	     && exp.X_add_number > 0
	     && frag_offset_ignore_align_p (start, frag_now, &frag_off)
	     && frag_off + frag_now_fix () < start_off + exp.X_add_number);
}

void
s_nops (int ignore ATTRIBUTE_UNUSED)
{
  expressionS exp;
  expressionS val;

#ifdef md_flush_pending_output
  md_flush_pending_output ();
#endif

#ifdef md_cons_align
  md_cons_align (1);
#endif

  SKIP_WHITESPACE ();
  expression (&exp);
  /* Note - this expression is tested for an absolute value in
     write.c:relax_segment().  */

  SKIP_WHITESPACE ();
  if (*input_line_pointer == ',')
    {
      ++input_line_pointer;
      expression (&val);
    }
  else
    {
      val.X_op = O_constant;
      val.X_add_number = 0;
    }

  if (val.X_op != O_constant)
    {
      as_bad (_("unsupported variable nop control in .nops directive"));
      val.X_op = O_constant;
      val.X_add_number = 0;
    }
  else if (val.X_add_number < 0)
    {
      as_warn (_("negative nop control byte, ignored"));
      val.X_add_number = 0;
    }

  demand_empty_rest_of_line ();

  if (need_pass_2)
    /* Ignore this directive if we are going to perform a second pass.  */
    return;

  /* Store the no-op instruction control byte in the first byte of frag.  */
  char *p;
  symbolS *sym = make_expr_symbol (&exp);
  p = frag_var (rs_space_nop, 1, 1, (relax_substateT) 0,
		sym, (offsetT) 0, (char *) 0);
  *p = val.X_add_number;
}

/* Obtain the size of a floating point number, given a type.  */

static int
float_length (int float_type, int *pad_p)
{
  int length, pad = 0;

  switch (float_type)
    {
    case 'b':
    case 'B':
    case 'h':
    case 'H':
      length = 2;
      break;

    case 'f':
    case 'F':
    case 's':
    case 'S':
      length = 4;
      break;

    case 'd':
    case 'D':
    case 'r':
    case 'R':
      length = 8;
      break;

    case 'x':
    case 'X':
#ifdef X_PRECISION
      length = X_PRECISION * sizeof (LITTLENUM_TYPE);
      pad = X_PRECISION_PAD * sizeof (LITTLENUM_TYPE);
      if (!length)
#endif
	length = 12;
      break;

    case 'p':
    case 'P':
#ifdef P_PRECISION
      length = P_PRECISION * sizeof (LITTLENUM_TYPE);
      pad = P_PRECISION_PAD * sizeof (LITTLENUM_TYPE);
      if (!length)
#endif
	length = 12;
      break;

    default:
      as_bad (_("unknown floating type '%c'"), float_type);
      length = -1;
      break;
    }

  if (pad_p)
    *pad_p = pad;

  return length;
}

static int
parse_one_float (int float_type, char temp[MAXIMUM_NUMBER_OF_CHARS_FOR_FLOAT])
{
  int length;

  SKIP_WHITESPACE ();

  /* Skip any 0{letter} that may be present.  Don't even check if the
     letter is legal.  Someone may invent a "z" format and this routine
     has no use for such information. Lusers beware: you get
     diagnostics if your input is ill-conditioned.  */
  if (input_line_pointer[0] == '0'
      && ISALPHA (input_line_pointer[1]))
    input_line_pointer += 2;

  /* Accept :xxxx, where the x's are hex digits, for a floating point
     with the exact digits specified.  */
  if (input_line_pointer[0] == ':')
    {
      ++input_line_pointer;
      length = hex_float (float_type, temp);
      if (length < 0)
	{
	  ignore_rest_of_line ();
	  return length;
	}
    }
  else
    {
      const char *err;

      err = md_atof (float_type, temp, &length);
      know (length <= MAXIMUM_NUMBER_OF_CHARS_FOR_FLOAT);
      know (err != NULL || length > 0);
      if (err)
	{
	  as_bad (_("bad floating literal: %s"), err);
	  ignore_rest_of_line ();
	  return -1;
	}
    }

  return length;
}

/* This is like s_space, but the value is a floating point number with
   the given precision.  This is for the MRI dcb.s pseudo-op and
   friends.  */

void
s_float_space (int float_type)
{
  offsetT count;
  int flen;
  char temp[MAXIMUM_NUMBER_OF_CHARS_FOR_FLOAT];
  char *stop = NULL;
  char stopc = 0;

#ifdef md_cons_align
  md_cons_align (1);
#endif

  if (flag_mri)
    stop = mri_comment_field (&stopc);

  count = get_absolute_expression ();

  SKIP_WHITESPACE ();
  if (*input_line_pointer != ',')
    {
      int pad;

      flen = float_length (float_type, &pad);
      if (flen >= 0)
	memset (temp, 0, flen += pad);
    }
  else
    {
      ++input_line_pointer;

      flen = parse_one_float (float_type, temp);
    }

  if (flen < 0)
    {
      if (flag_mri)
	mri_comment_end (stop, stopc);
      return;
    }

  while (--count >= 0)
    {
      char *p;

      p = frag_more (flen);
      memcpy (p, temp, (unsigned int) flen);
    }

  demand_empty_rest_of_line ();

  if (flag_mri)
    mri_comment_end (stop, stopc);
}

/* Handle the .struct pseudo-op, as found in MIPS assemblers.  */

void
s_struct (int ignore ATTRIBUTE_UNUSED)
{
  char *stop = NULL;
  char stopc = 0;

  if (flag_mri)
    stop = mri_comment_field (&stopc);
  abs_section_offset = get_absolute_expression ();
#if defined (OBJ_ELF) || defined (OBJ_MAYBE_ELF)
  /* The ELF backend needs to know that we are changing sections, so
     that .previous works correctly. */
  if (IS_ELF)
    obj_elf_section_change_hook ();
#endif
  subseg_set (absolute_section, 0);
  demand_empty_rest_of_line ();
  if (flag_mri)
    mri_comment_end (stop, stopc);
}

void
s_text (int ignore ATTRIBUTE_UNUSED)
{
  int temp;

  temp = get_absolute_expression ();
  subseg_set (text_section, (subsegT) temp);
  demand_empty_rest_of_line ();
}

/* .weakref x, y sets x as an alias to y that, as long as y is not
   referenced directly, will cause y to become a weak symbol.  */
void
s_weakref (int ignore ATTRIBUTE_UNUSED)
{
  char *name;
  symbolS *symbolP;
  symbolS *symbolP2;
  expressionS exp;

  if ((name = read_symbol_name ()) == NULL)
    return;

  symbolP = symbol_find_or_make (name);

  if (S_IS_DEFINED (symbolP) || symbol_equated_p (symbolP))
    {
      if (!S_IS_VOLATILE (symbolP))
	{
	  as_bad (_("symbol `%s' is already defined"), name);
	  goto err_out;
	}
      symbolP = symbol_clone (symbolP, 1);
      S_CLEAR_VOLATILE (symbolP);
    }

  SKIP_WHITESPACE ();

  if (*input_line_pointer != ',')
    {
      as_bad (_("expected comma after \"%s\""), name);
      goto err_out;
    }

  input_line_pointer++;

  SKIP_WHITESPACE ();
  free (name);

  if ((name = read_symbol_name ()) == NULL)
    return;

  if ((symbolP2 = symbol_find_noref (name, 1)) == NULL
      && (symbolP2 = md_undefined_symbol (name)) == NULL)
    {
      symbolP2 = symbol_find_or_make (name);
      S_SET_WEAKREFD (symbolP2);
    }
  else
    {
      symbolS *symp = symbolP2;

      while (S_IS_WEAKREFR (symp) && symp != symbolP)
	{
	  expressionS *expP = symbol_get_value_expression (symp);

	  gas_assert (expP->X_op == O_symbol
		  && expP->X_add_number == 0);
	  symp = expP->X_add_symbol;
	}
      if (symp == symbolP)
	{
	  char *loop;

	  loop = concat (S_GET_NAME (symbolP),
			 " => ", S_GET_NAME (symbolP2), (const char *) NULL);

	  symp = symbolP2;
	  while (symp != symbolP)
	    {
	      char *old_loop = loop;

	      symp = symbol_get_value_expression (symp)->X_add_symbol;
	      loop = concat (loop, " => ", S_GET_NAME (symp),
			     (const char *) NULL);
	      free (old_loop);
	    }

	  as_bad (_("%s: would close weakref loop: %s"),
		  S_GET_NAME (symbolP), loop);

	  free (loop);
	  free (name);
	  ignore_rest_of_line ();
	  return;
	}

      /* Short-circuiting instead of just checking here might speed
	 things up a tiny little bit, but loop error messages would
	 miss intermediate links.  */
      /* symbolP2 = symp; */
    }

  memset (&exp, 0, sizeof (exp));
  exp.X_op = O_symbol;
  exp.X_add_symbol = symbolP2;

  S_SET_SEGMENT (symbolP, undefined_section);
  symbol_set_value_expression (symbolP, &exp);
  symbol_set_frag (symbolP, &zero_address_frag);
  S_SET_WEAKREFR (symbolP);

  demand_empty_rest_of_line ();
  free (name);
  return;

 err_out:
  ignore_rest_of_line ();
  free (name);
  return;
}


/* Verify that we are at the end of a line.  If not, issue an error and
   skip to EOL.  This function may leave input_line_pointer one past
   buffer_limit, so should not be called from places that may
   dereference input_line_pointer unconditionally.  Note that when the
   gas parser is switched to handling a string (where buffer_limit
   should be the size of the string excluding the NUL terminator) this
   will be one past the NUL; is_end_of_line(0) returns true.  */

void
demand_empty_rest_of_line (void)
{
  SKIP_WHITESPACE ();
  if (input_line_pointer > buffer_limit)
    return;
  if (is_end_of_line[(unsigned char) *input_line_pointer])
    input_line_pointer++;
  else
    {
      if (ISPRINT (*input_line_pointer))
	as_bad (_("junk at end of line, first unrecognized character is `%c'"),
		 *input_line_pointer);
      else
	as_bad (_("junk at end of line, first unrecognized character valued 0x%x"),
		 *input_line_pointer);
      ignore_rest_of_line ();
    }
  /* Return pointing just after end-of-line.  */
}

/* Silently advance to the end of line.  Use this after already having
   issued an error about something bad.  Like demand_empty_rest_of_line,
   this function may leave input_line_pointer one after buffer_limit;
   Don't call it from within expression parsing code in an attempt to
   silence further errors.  */

void
ignore_rest_of_line (void)
{
  while (input_line_pointer <= buffer_limit)
    if (is_end_of_line[(unsigned char) *input_line_pointer++])
      break;
  /* Return pointing just after end-of-line.  */
}

/* Sets frag for given symbol to zero_address_frag, except when the
   symbol frag is already set to a dummy listing frag.  */

static void
set_zero_frag (symbolS *symbolP)
{
  if (symbol_get_frag (symbolP)->fr_type != rs_dummy)
    symbol_set_frag (symbolP, &zero_address_frag);
}

/* In:	Pointer to a symbol.
	Input_line_pointer->expression.

   Out:	Input_line_pointer->just after any whitespace after expression.
	Tried to set symbol to value of expression.
	Will change symbols type, value, and frag;  */

void
pseudo_set (symbolS *symbolP)
{
  expressionS exp;
  segT seg;

  know (symbolP);		/* NULL pointer is logic error.  */

  if (!S_IS_FORWARD_REF (symbolP))
    (void) expression (&exp);
  else
    (void) deferred_expression (&exp);

  if (exp.X_op == O_illegal)
    as_bad (_("illegal expression"));
  else if (exp.X_op == O_absent)
    as_bad (_("missing expression"));
  else if (exp.X_op == O_big)
    {
      if (exp.X_add_number > 0)
	as_bad (_("bignum invalid"));
      else
	as_bad (_("floating point number invalid"));
    }
  else if (exp.X_op == O_subtract
	   && !S_IS_FORWARD_REF (symbolP)
	   && SEG_NORMAL (S_GET_SEGMENT (exp.X_add_symbol))
	   && (symbol_get_frag (exp.X_add_symbol)
	       == symbol_get_frag (exp.X_op_symbol)))
    {
      exp.X_op = O_constant;
      exp.X_add_number = (S_GET_VALUE (exp.X_add_symbol)
			  - S_GET_VALUE (exp.X_op_symbol));
    }

  if (symbol_section_p (symbolP))
    {
      as_bad ("attempt to set value of section symbol");
      return;
    }

  switch (exp.X_op)
    {
    case O_illegal:
    case O_absent:
    case O_big:
      exp.X_add_number = 0;
      /* Fall through.  */
    case O_constant:
      S_SET_SEGMENT (symbolP, absolute_section);
      S_SET_VALUE (symbolP, (valueT) exp.X_add_number);
      set_zero_frag (symbolP);
      break;

    case O_register:
#ifndef TC_GLOBAL_REGISTER_SYMBOL_OK
      if (S_IS_EXTERNAL (symbolP))
	{
	  as_bad ("can't equate global symbol `%s' with register name",
		  S_GET_NAME (symbolP));
	  return;
	}
#endif
      /* Make sure symbol_equated_p() recognizes the symbol as an equate.  */
      exp.X_add_symbol = make_expr_symbol (&exp);
      exp.X_add_number = 0;
      exp.X_op = O_symbol;
      symbol_set_value_expression (symbolP, &exp);
      S_SET_SEGMENT (symbolP, reg_section);
      set_zero_frag (symbolP);
      break;

    case O_symbol:
      seg = S_GET_SEGMENT (exp.X_add_symbol);
      /* For x=undef+const, create an expression symbol.
	 For x=x+const, just update x except when x is an undefined symbol
	 For x=defined+const, evaluate x.  */
      if (symbolP == exp.X_add_symbol
	  && (seg != undefined_section
	      || !symbol_constant_p (symbolP)))
	{
	  *symbol_X_add_number (symbolP) += exp.X_add_number;
	  break;
	}
      else if (!S_IS_FORWARD_REF (symbolP) && seg != undefined_section)
	{
	  symbolS *s = exp.X_add_symbol;

	  if (S_IS_COMMON (s))
	    as_bad (_("`%s' can't be equated to common symbol `%s'"),
		    S_GET_NAME (symbolP), S_GET_NAME (s));

	  S_SET_SEGMENT (symbolP, seg);
	  S_SET_VALUE (symbolP, exp.X_add_number + S_GET_VALUE (s));
	  symbol_set_frag (symbolP, symbol_get_frag (s));
	  copy_symbol_attributes (symbolP, s);
	  break;
	}
      S_SET_SEGMENT (symbolP, undefined_section);
      symbol_set_value_expression (symbolP, &exp);
      copy_symbol_attributes (symbolP, exp.X_add_symbol);
      set_zero_frag (symbolP);
      break;

    default:
      /* The value is some complex expression.  */
      S_SET_SEGMENT (symbolP, expr_section);
      symbol_set_value_expression (symbolP, &exp);
      set_zero_frag (symbolP);
      break;
    }
}

/*			cons()

   CONStruct more frag of .bytes, or .words etc.
   Should need_pass_2 be 1 then emit no frag(s).
   This understands EXPRESSIONS.

   Bug (?)

   This has a split personality. We use expression() to read the
   value. We can detect if the value won't fit in a byte or word.
   But we can't detect if expression() discarded significant digits
   in the case of a long. Not worth the crocks required to fix it.  */

/* Select a parser for cons expressions.  */

/* Some targets need to parse the expression in various fancy ways.
   You can define TC_PARSE_CONS_EXPRESSION to do whatever you like
   (for example, the HPPA does this).  Otherwise, you can define
   REPEAT_CONS_EXPRESSIONS to permit repeat counts.  If none of these
   are defined, which is the normal case, then only simple expressions
   are permitted.  */

#ifdef TC_M68K
static void
parse_mri_cons (expressionS *exp, unsigned int nbytes);
#endif

#ifndef TC_PARSE_CONS_EXPRESSION
#ifdef REPEAT_CONS_EXPRESSIONS
#define TC_PARSE_CONS_EXPRESSION(EXP, NBYTES) \
  (parse_repeat_cons (EXP, NBYTES), TC_PARSE_CONS_RETURN_NONE)
static void
parse_repeat_cons (expressionS *exp, unsigned int nbytes);
#endif

/* If we haven't gotten one yet, just call expression.  */
#ifndef TC_PARSE_CONS_EXPRESSION
#define TC_PARSE_CONS_EXPRESSION(EXP, NBYTES) \
  (expression (EXP), TC_PARSE_CONS_RETURN_NONE)
#endif
#endif

void
do_parse_cons_expression (expressionS *exp,
			  int nbytes ATTRIBUTE_UNUSED)
{
  (void) TC_PARSE_CONS_EXPRESSION (exp, nbytes);
}


/* Worker to do .byte etc statements.
   Clobbers input_line_pointer and checks end-of-line.  */

static void
cons_worker (int nbytes,	/* 1=.byte, 2=.word, 4=.long.  */
	     int rva)
{
  int c;
  expressionS exp;
  char *stop = NULL;
  char stopc = 0;

#ifdef md_flush_pending_output
  md_flush_pending_output ();
#endif

  if (flag_mri)
    stop = mri_comment_field (&stopc);

  if (is_it_end_of_statement ())
    {
      demand_empty_rest_of_line ();
      if (flag_mri)
	mri_comment_end (stop, stopc);
      return;
    }

  if (nbytes == 0)
    nbytes = TC_ADDRESS_BYTES ();

#ifdef md_cons_align
  md_cons_align (nbytes);
#endif

  c = 0;
  do
    {
      TC_PARSE_CONS_RETURN_TYPE ret = TC_PARSE_CONS_RETURN_NONE;
#ifdef TC_CONS_FIX_CHECK
      fixS **cur_fix = &frchain_now->fix_tail;

      if (*cur_fix != NULL)
	cur_fix = &(*cur_fix)->fx_next;
#endif

#ifdef TC_M68K
      if (flag_m68k_mri)
	parse_mri_cons (&exp, (unsigned int) nbytes);
      else
#endif
	{
#if 0
	  if (*input_line_pointer == '"')
	    {
	      as_bad (_("unexpected `\"' in expression"));
	      ignore_rest_of_line ();
	      return;
	    }
#endif
	  ret = TC_PARSE_CONS_EXPRESSION (&exp, (unsigned int) nbytes);
	}

      if (rva)
	{
	  if (exp.X_op == O_symbol)
	    exp.X_op = O_symbol_rva;
	  else
	    as_fatal (_("rva without symbol"));
	}
      emit_expr_with_reloc (&exp, (unsigned int) nbytes, ret);
#ifdef TC_CONS_FIX_CHECK
      TC_CONS_FIX_CHECK (&exp, nbytes, *cur_fix);
#endif
      ++c;
    }
  while (*input_line_pointer++ == ',');

  /* In MRI mode, after an odd number of bytes, we must align to an
     even word boundary, unless the next instruction is a dc.b, ds.b
     or dcb.b.  */
  if (flag_mri && nbytes == 1 && (c & 1) != 0)
    mri_pending_align = 1;

  input_line_pointer--;		/* Put terminator back into stream.  */

  demand_empty_rest_of_line ();

  if (flag_mri)
    mri_comment_end (stop, stopc);
}

void
cons (int size)
{
  cons_worker (size, 0);
}

void
s_rva (int size)
{
  cons_worker (size, 1);
}

/* .reloc offset, reloc_name, symbol+addend.  */

static void
s_reloc (int ignore ATTRIBUTE_UNUSED)
{
  char *stop = NULL;
  char stopc = 0;
  expressionS exp;
  char *r_name;
  int c;
  struct reloc_list *reloc;
  struct _bfd_rel { const char * name; bfd_reloc_code_real_type code; };
  static const struct _bfd_rel bfd_relocs[] =
  {
    { "NONE", BFD_RELOC_NONE },
    { "8",  BFD_RELOC_8 },
    { "16", BFD_RELOC_16 },
    { "32", BFD_RELOC_32 },
    { "64", BFD_RELOC_64 }
  };

  reloc = XNEW (struct reloc_list);

  if (flag_mri)
    stop = mri_comment_field (&stopc);

  expression (&exp);
  switch (exp.X_op)
    {
    case O_illegal:
    case O_absent:
    case O_big:
    case O_register:
      as_bad (_("missing or bad offset expression"));
      goto err_out;
    case O_constant:
      exp.X_add_symbol = section_symbol (now_seg);
      /* Mark the section symbol used in relocation so that it will be
	 included in the symbol table.  */
      symbol_mark_used_in_reloc (exp.X_add_symbol);
      exp.X_op = O_symbol;
      /* Fallthru */
    case O_symbol:
      if (exp.X_add_number == 0)
	{
	  reloc->u.a.offset_sym = exp.X_add_symbol;
	  break;
	}
      /* Fallthru */
    default:
      reloc->u.a.offset_sym = make_expr_symbol (&exp);
      break;
    }

  SKIP_WHITESPACE ();
  if (*input_line_pointer != ',')
    {
      as_bad (_("missing reloc type"));
      goto err_out;
    }

  ++input_line_pointer;
  SKIP_WHITESPACE ();
  c = get_symbol_name (& r_name);
  if (strncasecmp (r_name, "BFD_RELOC_", 10) == 0)
    {
      unsigned int i;

      for (reloc->u.a.howto = NULL, i = 0; i < ARRAY_SIZE (bfd_relocs); i++)
	if (strcasecmp (r_name + 10, bfd_relocs[i].name) == 0)
	  {
	    reloc->u.a.howto = bfd_reloc_type_lookup (stdoutput,
						      bfd_relocs[i].code);
	    break;
	  }
    }
  else
    reloc->u.a.howto = bfd_reloc_name_lookup (stdoutput, r_name);
  *input_line_pointer = c;
  if (reloc->u.a.howto == NULL)
    {
      as_bad (_("unrecognized reloc type"));
      goto err_out;
    }

  exp.X_op = O_absent;
  SKIP_WHITESPACE_AFTER_NAME ();
  if (*input_line_pointer == ',')
    {
      ++input_line_pointer;
      expression (&exp);
    }
  switch (exp.X_op)
    {
    case O_illegal:
    case O_big:
    case O_register:
      as_bad (_("bad reloc expression"));
    err_out:
      ignore_rest_of_line ();
      free (reloc);
      if (flag_mri)
	mri_comment_end (stop, stopc);
      return;
    case O_absent:
      reloc->u.a.sym = NULL;
      reloc->u.a.addend = 0;
      break;
    case O_constant:
      reloc->u.a.sym = NULL;
      reloc->u.a.addend = exp.X_add_number;
      break;
    case O_symbol:
      reloc->u.a.sym = exp.X_add_symbol;
      reloc->u.a.addend = exp.X_add_number;
      break;
    default:
      reloc->u.a.sym = make_expr_symbol (&exp);
      reloc->u.a.addend = 0;
      break;
    }

  reloc->file = as_where (&reloc->line);
  reloc->next = reloc_list;
  reloc_list = reloc;

  demand_empty_rest_of_line ();
  if (flag_mri)
    mri_comment_end (stop, stopc);
}

/* Put the contents of expression EXP into the object file using
   NBYTES bytes.  If need_pass_2 is 1, this does nothing.  */

void
emit_expr (expressionS *exp, unsigned int nbytes)
{
  emit_expr_with_reloc (exp, nbytes, TC_PARSE_CONS_RETURN_NONE);
}

void
emit_expr_with_reloc (expressionS *exp,
		      unsigned int nbytes,
		      TC_PARSE_CONS_RETURN_TYPE reloc)
{
  operatorT op;
  char *p;
  valueT extra_digit = 0;

  /* Don't do anything if we are going to make another pass.  */
  if (need_pass_2)
    return;

  frag_grow (nbytes);
  dot_value = frag_now_fix ();
  dot_frag = frag_now;

#ifndef NO_LISTING
#ifdef OBJ_ELF
  /* When gcc emits DWARF 1 debugging pseudo-ops, a line number will
     appear as a four byte positive constant in the .line section,
     followed by a 2 byte 0xffff.  Look for that case here.  */
  if (strcmp (segment_name (now_seg), ".line") != 0)
    dwarf_line = -1;
  else if (dwarf_line >= 0
	   && nbytes == 2
	   && exp->X_op == O_constant
	   && (exp->X_add_number == -1 || exp->X_add_number == 0xffff))
    listing_source_line ((unsigned int) dwarf_line);
  else if (nbytes == 4
	   && exp->X_op == O_constant
	   && exp->X_add_number >= 0)
    dwarf_line = exp->X_add_number;
  else
    dwarf_line = -1;

  /* When gcc emits DWARF 1 debugging pseudo-ops, a file name will
     appear as a 2 byte TAG_compile_unit (0x11) followed by a 2 byte
     AT_sibling (0x12) followed by a four byte address of the sibling
     followed by a 2 byte AT_name (0x38) followed by the name of the
     file.  We look for that case here.  */
  if (strcmp (segment_name (now_seg), ".debug") != 0)
    dwarf_file = 0;
  else if (dwarf_file == 0
	   && nbytes == 2
	   && exp->X_op == O_constant
	   && exp->X_add_number == 0x11)
    dwarf_file = 1;
  else if (dwarf_file == 1
	   && nbytes == 2
	   && exp->X_op == O_constant
	   && exp->X_add_number == 0x12)
    dwarf_file = 2;
  else if (dwarf_file == 2
	   && nbytes == 4)
    dwarf_file = 3;
  else if (dwarf_file == 3
	   && nbytes == 2
	   && exp->X_op == O_constant
	   && exp->X_add_number == 0x38)
    dwarf_file = 4;
  else
    dwarf_file = 0;

  /* The variable dwarf_file_string tells stringer that the string
     may be the name of the source file.  */
  if (dwarf_file == 4)
    dwarf_file_string = 1;
  else
    dwarf_file_string = 0;
#endif
#endif

  if (check_eh_frame (exp, &nbytes))
    return;

  op = exp->X_op;

  /* Handle a negative bignum.  */
  if (op == O_uminus
      && exp->X_add_number == 0
      && symbol_get_value_expression (exp->X_add_symbol)->X_op == O_big
      && symbol_get_value_expression (exp->X_add_symbol)->X_add_number > 0)
    {
      int i;
      unsigned long carry;

      exp = symbol_get_value_expression (exp->X_add_symbol);

      /* Negate the bignum: one's complement each digit and add 1.  */
      carry = 1;
      for (i = 0; i < exp->X_add_number; i++)
	{
	  unsigned long next;

	  next = (((~(generic_bignum[i] & LITTLENUM_MASK))
		   & LITTLENUM_MASK)
		  + carry);
	  generic_bignum[i] = next & LITTLENUM_MASK;
	  carry = next >> LITTLENUM_NUMBER_OF_BITS;
	}

      /* We can ignore any carry out, because it will be handled by
	 extra_digit if it is needed.  */

      extra_digit = (valueT) -1;
      op = O_big;
    }

  if (op == O_absent || op == O_illegal)
    {
      as_warn (_("zero assumed for missing expression"));
      exp->X_add_number = 0;
      op = O_constant;
    }
  else if (op == O_big && exp->X_add_number <= 0)
    {
      as_bad (_("floating point number invalid"));
      exp->X_add_number = 0;
      op = O_constant;
    }
  else if (op == O_register)
    {
      as_warn (_("register value used as expression"));
      op = O_constant;
    }

  /* Allow `.word 0' in the absolute section.  */
  if (now_seg == absolute_section)
    {
      if (op != O_constant || exp->X_add_number != 0)
	as_bad (_("attempt to store value in absolute section"));
      abs_section_offset += nbytes;
      return;
    }

  /* Allow `.word 0' in BSS style sections.  */
  if ((op != O_constant || exp->X_add_number != 0) && in_bss ())
    as_bad (_("attempt to store non-zero value in section `%s'"),
	    segment_name (now_seg));

  p = frag_more ((int) nbytes);

  if (reloc != TC_PARSE_CONS_RETURN_NONE)
    {
      emit_expr_fix (exp, nbytes, frag_now, p, reloc);
      return;
    }

#ifndef WORKING_DOT_WORD
  /* If we have the difference of two symbols in a word, save it on
     the broken_words list.  See the code in write.c.  */
  if (op == O_subtract && nbytes == 2)
    {
      struct broken_word *x;

      x = XNEW (struct broken_word);
      x->next_broken_word = broken_words;
      broken_words = x;
      x->seg = now_seg;
      x->subseg = now_subseg;
      x->frag = frag_now;
      x->word_goes_here = p;
      x->dispfrag = 0;
      x->add = exp->X_add_symbol;
      x->sub = exp->X_op_symbol;
      x->addnum = exp->X_add_number;
      x->added = 0;
      x->use_jump = 0;
      new_broken_words++;
      return;
    }
#endif

  /* If we have an integer, but the number of bytes is too large to
     pass to md_number_to_chars, handle it as a bignum.  */
  if (op == O_constant && nbytes > sizeof (valueT))
    {
      extra_digit = exp->X_unsigned ? 0 : -1;
      convert_to_bignum (exp, !exp->X_unsigned);
      op = O_big;
    }

  if (op == O_constant)
    {
      valueT get;
      valueT use;
      valueT mask;
      valueT unmask;

      /* JF << of >= number of bits in the object is undefined.  In
	 particular SPARC (Sun 4) has problems.  */
      if (nbytes >= sizeof (valueT))
	{
	  know (nbytes == sizeof (valueT));
	  mask = 0;
	}
      else
	{
	  /* Don't store these bits.  */
	  mask = ~(valueT) 0 << (BITS_PER_CHAR * nbytes);
	}

      unmask = ~mask;		/* Do store these bits.  */

#ifdef NEVER
      "Do this mod if you want every overflow check to assume SIGNED 2's complement data.";
      mask = ~(unmask >> 1);	/* Includes sign bit now.  */
#endif

      get = exp->X_add_number;
      use = get & unmask;
      if ((get & mask) != 0 && (-get & mask) != 0)
	{
	  /* Leading bits contain both 0s & 1s.  */
	  as_warn (_("value 0x%" PRIx64 " truncated to 0x%" PRIx64),
		   (uint64_t) get, (uint64_t) use);
	}
      /* Put bytes in right order.  */
      md_number_to_chars (p, use, (int) nbytes);
    }
  else if (op == O_big)
    {
      unsigned int size;
      LITTLENUM_TYPE *nums;

      size = exp->X_add_number * CHARS_PER_LITTLENUM;
      if (nbytes < size)
	{
	  int i = nbytes / CHARS_PER_LITTLENUM;

	  if (i != 0)
	    {
	      LITTLENUM_TYPE sign = 0;
	      if ((generic_bignum[--i]
		   & (1 << (LITTLENUM_NUMBER_OF_BITS - 1))) != 0)
		sign = ~(LITTLENUM_TYPE) 0;

	      while (++i < exp->X_add_number)
		if (generic_bignum[i] != sign)
		  break;
	    }
	  else if (nbytes == 1)
	    {
	      /* We have nbytes == 1 and CHARS_PER_LITTLENUM == 2 (probably).
		 Check that bits 8.. of generic_bignum[0] match bit 7
		 and that they match all of generic_bignum[1..exp->X_add_number].  */
	      LITTLENUM_TYPE sign = (generic_bignum[0] & (1 << 7)) ? -1 : 0;
	      LITTLENUM_TYPE himask = LITTLENUM_MASK & ~ 0xFF;

	      if ((generic_bignum[0] & himask) == (sign & himask))
		{
		  while (++i < exp->X_add_number)
		    if (generic_bignum[i] != sign)
		      break;
		}
	    }

	  if (i < exp->X_add_number)
	    as_warn (ngettext ("bignum truncated to %d byte",
			       "bignum truncated to %d bytes",
			       nbytes),
		     nbytes);
	  size = nbytes;
	}

      if (nbytes == 1)
	{
	  md_number_to_chars (p, (valueT) generic_bignum[0], 1);
	  return;
	}
      know (nbytes % CHARS_PER_LITTLENUM == 0);

      if (target_big_endian)
	{
	  while (nbytes > size)
	    {
	      md_number_to_chars (p, extra_digit, CHARS_PER_LITTLENUM);
	      nbytes -= CHARS_PER_LITTLENUM;
	      p += CHARS_PER_LITTLENUM;
	    }

	  nums = generic_bignum + size / CHARS_PER_LITTLENUM;
	  while (size >= CHARS_PER_LITTLENUM)
	    {
	      --nums;
	      md_number_to_chars (p, (valueT) *nums, CHARS_PER_LITTLENUM);
	      size -= CHARS_PER_LITTLENUM;
	      p += CHARS_PER_LITTLENUM;
	    }
	}
      else
	{
	  nums = generic_bignum;
	  while (size >= CHARS_PER_LITTLENUM)
	    {
	      md_number_to_chars (p, (valueT) *nums, CHARS_PER_LITTLENUM);
	      ++nums;
	      size -= CHARS_PER_LITTLENUM;
	      p += CHARS_PER_LITTLENUM;
	      nbytes -= CHARS_PER_LITTLENUM;
	    }

	  while (nbytes >= CHARS_PER_LITTLENUM)
	    {
	      md_number_to_chars (p, extra_digit, CHARS_PER_LITTLENUM);
	      nbytes -= CHARS_PER_LITTLENUM;
	      p += CHARS_PER_LITTLENUM;
	    }
	}
    }
  else
    emit_expr_fix (exp, nbytes, frag_now, p, TC_PARSE_CONS_RETURN_NONE);
}

void
emit_expr_fix (expressionS *exp, unsigned int nbytes, fragS *frag, char *p,
	       TC_PARSE_CONS_RETURN_TYPE r ATTRIBUTE_UNUSED)
{
  int offset = 0;
  unsigned int size = nbytes;

  memset (p, 0, size);

  /* Generate a fixS to record the symbol value.  */

#ifdef TC_CONS_FIX_NEW
  TC_CONS_FIX_NEW (frag, p - frag->fr_literal + offset, size, exp, r);
#else
  if (r != TC_PARSE_CONS_RETURN_NONE)
    {
      reloc_howto_type *reloc_howto;

      reloc_howto = bfd_reloc_type_lookup (stdoutput, r);
      size = bfd_get_reloc_size (reloc_howto);

      if (size > nbytes)
	{
	  as_bad (ngettext ("%s relocations do not fit in %u byte",
			    "%s relocations do not fit in %u bytes",
			    nbytes),
		  reloc_howto->name, nbytes);
	  return;
	}
      else if (target_big_endian)
	offset = nbytes - size;
    }
  else
    switch (size)
      {
      case 1:
	r = BFD_RELOC_8;
	break;
      case 2:
	r = BFD_RELOC_16;
	break;
      case 3:
	r = BFD_RELOC_24;
	break;
      case 4:
	r = BFD_RELOC_32;
	break;
      case 8:
	r = BFD_RELOC_64;
	break;
      default:
	as_bad (_("unsupported BFD relocation size %u"), size);
	return;
      }
  fix_new_exp (frag, p - frag->fr_literal + offset, size,
	       exp, 0, r);
#endif
}

/* Handle an MRI style string expression.  */

#ifdef TC_M68K
static void
parse_mri_cons (expressionS *exp, unsigned int nbytes)
{
  if (*input_line_pointer != '\''
      && (input_line_pointer[1] != '\''
	  || (*input_line_pointer != 'A'
	      && *input_line_pointer != 'E')))
    (void) TC_PARSE_CONS_EXPRESSION (exp, nbytes);
  else
    {
      unsigned int scan;
      unsigned int result = 0;

      /* An MRI style string.  Cut into as many bytes as will fit into
	 a nbyte chunk, left justify if necessary, and separate with
	 commas so we can try again later.  */
      if (*input_line_pointer == 'A')
	++input_line_pointer;
      else if (*input_line_pointer == 'E')
	{
	  as_bad (_("EBCDIC constants are not supported"));
	  ++input_line_pointer;
	}

      input_line_pointer++;
      for (scan = 0; scan < nbytes; scan++)
	{
	  if (*input_line_pointer == '\'')
	    {
	      if (input_line_pointer[1] == '\'')
		{
		  input_line_pointer++;
		}
	      else
		break;
	    }
	  result = (result << 8) | (*input_line_pointer++);
	}

      /* Left justify.  */
      while (scan < nbytes)
	{
	  result <<= 8;
	  scan++;
	}

      /* Create correct expression.  */
      exp->X_op = O_constant;
      exp->X_add_number = result;

      /* Fake it so that we can read the next char too.  */
      if (input_line_pointer[0] != '\'' ||
	  (input_line_pointer[0] == '\'' && input_line_pointer[1] == '\''))
	{
	  input_line_pointer -= 2;
	  input_line_pointer[0] = ',';
	  input_line_pointer[1] = '\'';
	}
      else
	input_line_pointer++;
    }
}
#endif /* TC_M68K */

#ifdef REPEAT_CONS_EXPRESSIONS

/* Parse a repeat expression for cons.  This is used by the MIPS
   assembler.  The format is NUMBER:COUNT; NUMBER appears in the
   object file COUNT times.

   To use this for a target, define REPEAT_CONS_EXPRESSIONS.  */

static void
parse_repeat_cons (expressionS *exp, unsigned int nbytes)
{
  expressionS count;
  int i;

  expression (exp);

  if (*input_line_pointer != ':')
    {
      /* No repeat count.  */
      return;
    }

  ++input_line_pointer;
  expression (&count);
  if (count.X_op != O_constant
      || count.X_add_number <= 0)
    {
      as_warn (_("unresolvable or nonpositive repeat count; using 1"));
      return;
    }

  /* The cons function is going to output this expression once.  So we
     output it count - 1 times.  */
  for (i = count.X_add_number - 1; i > 0; i--)
    emit_expr (exp, nbytes);
}

#endif /* REPEAT_CONS_EXPRESSIONS */

/* Parse a floating point number represented as a hex constant.  This
   permits users to specify the exact bits they want in the floating
   point number.  */

static int
hex_float (int float_type, char *bytes)
{
  int pad, length = float_length (float_type, &pad);
  int i;

  if (length < 0)
    return length;

  /* It would be nice if we could go through expression to parse the
     hex constant, but if we get a bignum it's a pain to sort it into
     the buffer correctly.  */
  i = 0;
  while (hex_p (*input_line_pointer) || *input_line_pointer == '_')
    {
      int d;

      /* The MRI assembler accepts arbitrary underscores strewn about
	 through the hex constant, so we ignore them as well.  */
      if (*input_line_pointer == '_')
	{
	  ++input_line_pointer;
	  continue;
	}

      if (i >= length)
	{
	  as_warn (_("floating point constant too large"));
	  return -1;
	}
      d = hex_value (*input_line_pointer) << 4;
      ++input_line_pointer;
      while (*input_line_pointer == '_')
	++input_line_pointer;
      if (hex_p (*input_line_pointer))
	{
	  d += hex_value (*input_line_pointer);
	  ++input_line_pointer;
	}
      if (target_big_endian)
	bytes[i] = d;
      else
	bytes[length - i - 1] = d;
      ++i;
    }

  if (i < length)
    {
      if (target_big_endian)
	memset (bytes + i, 0, length - i);
      else
	memset (bytes, 0, length - i);
    }

  memset (bytes + length, 0, pad);

  return length + pad;
}

/*			float_cons()

   CONStruct some more frag chars of .floats .ffloats etc.
   Makes 0 or more new frags.
   If need_pass_2 == 1, no frags are emitted.
   This understands only floating literals, not expressions. Sorry.

   A floating constant is defined by atof_generic(), except it is preceded
   by 0d 0f 0g or 0h. After observing the STRANGE way my BSD AS does its
   reading, I decided to be incompatible. This always tries to give you
   rounded bits to the precision of the pseudo-op. Former AS did premature
   truncation, restored noisy bits instead of trailing 0s AND gave you
   a choice of 2 flavours of noise according to which of 2 floating-point
   scanners you directed AS to use.

   In:	input_line_pointer->whitespace before, or '0' of flonum.  */

void
float_cons (/* Clobbers input_line-pointer, checks end-of-line.  */
	    int float_type	/* 'f':.ffloat ... 'F':.float ...  */)
{
  char *p;
  int length;			/* Number of chars in an object.  */
  char temp[MAXIMUM_NUMBER_OF_CHARS_FOR_FLOAT];

  if (is_it_end_of_statement ())
    {
      demand_empty_rest_of_line ();
      return;
    }

  if (now_seg == absolute_section)
    {
      as_bad (_("attempt to store float in absolute section"));
      ignore_rest_of_line ();
      return;
    }

  if (in_bss ())
    {
      as_bad (_("attempt to store float in section `%s'"),
	      segment_name (now_seg));
      ignore_rest_of_line ();
      return;
    }

#ifdef md_flush_pending_output
  md_flush_pending_output ();
#endif

#ifdef md_cons_align
  md_cons_align (1);
#endif

  do
    {
      length = parse_one_float (float_type, temp);
      if (length < 0)
	return;

      if (!need_pass_2)
	{
	  int count;

	  count = 1;

#ifdef REPEAT_CONS_EXPRESSIONS
	  if (*input_line_pointer == ':')
	    {
	      expressionS count_exp;

	      ++input_line_pointer;
	      expression (&count_exp);

	      if (count_exp.X_op != O_constant
		  || count_exp.X_add_number <= 0)
		as_warn (_("unresolvable or nonpositive repeat count; using 1"));
	      else
		count = count_exp.X_add_number;
	    }
#endif

	  while (--count >= 0)
	    {
	      p = frag_more (length);
	      memcpy (p, temp, (unsigned int) length);
	    }
	}
      SKIP_WHITESPACE ();
    }
  while (*input_line_pointer++ == ',');

  /* Put terminator back into stream.  */
  --input_line_pointer;
  demand_empty_rest_of_line ();
}

/* LEB128 Encoding.

   Note - we are using the DWARF standard's definition of LEB128 encoding
   where each 7-bit value is a stored in a byte, *not* an octet.  This
   means that on targets where a byte contains multiple octets there is
   a *huge waste of space*.  (This also means that we do not have to
   have special versions of these functions for when OCTETS_PER_BYTE_POWER
   is non-zero).

   If the 7-bit values were to be packed into N-bit bytes (where N > 8)
   we would then have to consider whether multiple, successive LEB128
   values should be packed into the bytes without padding (bad idea) or
   whether each LEB128 number is padded out to a whole number of bytes.
   Plus you have to decide on the endianness of packing octets into a
   byte.  */

/* Return the size of a LEB128 value in bytes.  */

static inline unsigned int
sizeof_sleb128 (offsetT value)
{
  int size = 0;
  unsigned byte;

  do
    {
      byte = (value & 0x7f);
      /* Sadly, we cannot rely on typical arithmetic right shift behaviour.
	 Fortunately, we can structure things so that the extra work reduces
	 to a noop on systems that do things "properly".  */
      value = (value >> 7) | ~(-(offsetT)1 >> 7);
      size += 1;
    }
  while (!(((value == 0) && ((byte & 0x40) == 0))
	   || ((value == -1) && ((byte & 0x40) != 0))));

  return size;
}

static inline unsigned int
sizeof_uleb128 (valueT value)
{
  int size = 0;

  do
    {
      value >>= 7;
      size += 1;
    }
  while (value != 0);

  return size;
}

unsigned int
sizeof_leb128 (valueT value, int sign)
{
  if (sign)
    return sizeof_sleb128 ((offsetT) value);
  else
    return sizeof_uleb128 (value);
}

/* Output a LEB128 value.  Returns the number of bytes used.  */

static inline unsigned int
output_sleb128 (char *p, offsetT value)
{
  char *orig = p;
  int more;

  do
    {
      unsigned byte = (value & 0x7f);

      /* Sadly, we cannot rely on typical arithmetic right shift behaviour.
	 Fortunately, we can structure things so that the extra work reduces
	 to a noop on systems that do things "properly".  */
      value = (value >> 7) | ~(-(offsetT)1 >> 7);

      more = !((((value == 0) && ((byte & 0x40) == 0))
		|| ((value == -1) && ((byte & 0x40) != 0))));
      if (more)
	byte |= 0x80;

      *p++ = byte;
    }
  while (more);

  return p - orig;
}

static inline unsigned int
output_uleb128 (char *p, valueT value)
{
  char *orig = p;

  do
    {
      unsigned byte = (value & 0x7f);

      value >>= 7;
      if (value != 0)
	/* More bytes to follow.  */
	byte |= 0x80;

      *p++ = byte;
    }
  while (value != 0);

  return p - orig;
}

unsigned int
output_leb128 (char *p, valueT value, int sign)
{
  if (sign)
    return output_sleb128 (p, (offsetT) value);
  else
    return output_uleb128 (p, value);
}

/* Do the same for bignums.  We combine sizeof with output here in that
   we don't output for NULL values of P.  It isn't really as critical as
   for "normal" values that this be streamlined.  Returns the number of
   bytes used.  */

static inline unsigned int
output_big_sleb128 (char *p, LITTLENUM_TYPE *bignum, unsigned int size)
{
  char *orig = p;
  valueT val = 0;
  int loaded = 0;
  unsigned byte;

  /* Strip leading sign extensions off the bignum.  */
  while (size > 1
	 && bignum[size - 1] == LITTLENUM_MASK
	 && bignum[size - 2] > LITTLENUM_MASK / 2)
    size--;

  do
    {
      /* OR in the next part of the littlenum.  */
      val |= (*bignum << loaded);
      loaded += LITTLENUM_NUMBER_OF_BITS;
      size--;
      bignum++;

      /* Add bytes until there are less than 7 bits left in VAL
	 or until every non-sign bit has been written.  */
      do
	{
	  byte = val & 0x7f;
	  loaded -= 7;
	  val >>= 7;
	  if (size > 0
	      || val != ((byte & 0x40) == 0 ? 0 : ((valueT) 1 << loaded) - 1))
	    byte |= 0x80;

	  if (orig)
	    *p = byte;
	  p++;
	}
      while ((byte & 0x80) != 0 && loaded >= 7);
    }
  while (size > 0);

  /* Mop up any left-over bits (of which there will be less than 7).  */
  if ((byte & 0x80) != 0)
    {
      /* Sign-extend VAL.  */
      if (val & (1 << (loaded - 1)))
	val |= ~0U << loaded;
      if (orig)
	*p = val & 0x7f;
      p++;
    }

  return p - orig;
}

static inline unsigned int
output_big_uleb128 (char *p, LITTLENUM_TYPE *bignum, unsigned int size)
{
  char *orig = p;
  valueT val = 0;
  int loaded = 0;
  unsigned byte;

  /* Strip leading zeros off the bignum.  */
  /* XXX: Is this needed?  */
  while (size > 0 && bignum[size - 1] == 0)
    size--;

  do
    {
      if (loaded < 7 && size > 0)
	{
	  val |= (*bignum << loaded);
	  loaded += 8 * CHARS_PER_LITTLENUM;
	  size--;
	  bignum++;
	}

      byte = val & 0x7f;
      loaded -= 7;
      val >>= 7;

      if (size > 0 || val)
	byte |= 0x80;

      if (orig)
	*p = byte;
      p++;
    }
  while (byte & 0x80);

  return p - orig;
}

static unsigned int
output_big_leb128 (char *p, LITTLENUM_TYPE *bignum, unsigned int size, int sign)
{
  if (sign)
    return output_big_sleb128 (p, bignum, size);
  else
    return output_big_uleb128 (p, bignum, size);
}

/* Generate the appropriate fragments for a given expression to emit a
   leb128 value.  SIGN is 1 for sleb, 0 for uleb.  */

void
emit_leb128_expr (expressionS *exp, int sign)
{
  operatorT op = exp->X_op;
  unsigned int nbytes;

  if (op == O_absent || op == O_illegal)
    {
      as_warn (_("zero assumed for missing expression"));
      exp->X_add_number = 0;
      op = O_constant;
    }
  else if (op == O_big && exp->X_add_number <= 0)
    {
      as_bad (_("floating point number invalid"));
      exp->X_add_number = 0;
      op = O_constant;
    }
  else if (op == O_register)
    {
      as_warn (_("register value used as expression"));
      op = O_constant;
    }
  else if (op == O_constant
	   && sign
	   && (exp->X_add_number < 0) == !exp->X_extrabit)
    {
      /* We're outputting a signed leb128 and the sign of X_add_number
	 doesn't reflect the sign of the original value.  Convert EXP
	 to a correctly-extended bignum instead.  */
      convert_to_bignum (exp, exp->X_extrabit);
      op = O_big;
    }

  if (now_seg == absolute_section)
    {
      if (op != O_constant || exp->X_add_number != 0)
	as_bad (_("attempt to store value in absolute section"));
      abs_section_offset++;
      return;
    }

  if ((op != O_constant || exp->X_add_number != 0) && in_bss ())
    as_bad (_("attempt to store non-zero value in section `%s'"),
	    segment_name (now_seg));

  /* Let check_eh_frame know that data is being emitted.  nbytes == -1 is
     a signal that this is leb128 data.  It shouldn't optimize this away.  */
  nbytes = (unsigned int) -1;
  if (check_eh_frame (exp, &nbytes))
    abort ();

  /* Let the backend know that subsequent data may be byte aligned.  */
#ifdef md_cons_align
  md_cons_align (1);
#endif

  if (op == O_constant)
    {
      /* If we've got a constant, emit the thing directly right now.  */

      valueT value = exp->X_add_number;
      unsigned int size;
      char *p;

      size = sizeof_leb128 (value, sign);
      p = frag_more (size);
      if (output_leb128 (p, value, sign) > size)
	abort ();
    }
  else if (op == O_big)
    {
      /* O_big is a different sort of constant.  */
      int nbr_digits = exp->X_add_number;
      unsigned int size;
      char *p;

      /* If the leading littenum is 0xffff, prepend a 0 to avoid confusion with
	 a signed number.  Unary operators like - or ~ always extend the
	 bignum to its largest size.  */
      if (exp->X_unsigned
	  && nbr_digits < SIZE_OF_LARGE_NUMBER
	  && generic_bignum[nbr_digits - 1] == LITTLENUM_MASK)
	generic_bignum[nbr_digits++] = 0;

      size = output_big_leb128 (NULL, generic_bignum, nbr_digits, sign);
      p = frag_more (size);
      if (output_big_leb128 (p, generic_bignum, nbr_digits, sign) > size)
	abort ();
    }
  else
    {
      /* Otherwise, we have to create a variable sized fragment and
	 resolve things later.  */

      frag_var (rs_leb128, sizeof_uleb128 (~(valueT) 0), 0, sign,
		make_expr_symbol (exp), 0, (char *) NULL);
    }
}

/* Parse the .sleb128 and .uleb128 pseudos.  */

void
s_leb128 (int sign)
{
  expressionS exp;

#ifdef md_flush_pending_output
  md_flush_pending_output ();
#endif

  do
    {
      expression (&exp);
      emit_leb128_expr (&exp, sign);
    }
  while (*input_line_pointer++ == ',');

  input_line_pointer--;
  demand_empty_rest_of_line ();
}

static void
stringer_append_char (int c, int bitsize)
{
  if (c && in_bss ())
    as_bad (_("attempt to store non-empty string in section `%s'"),
	    segment_name (now_seg));

  if (!target_big_endian)
    FRAG_APPEND_1_CHAR (c);

  switch (bitsize)
    {
    case 64:
      FRAG_APPEND_1_CHAR (0);
      FRAG_APPEND_1_CHAR (0);
      FRAG_APPEND_1_CHAR (0);
      FRAG_APPEND_1_CHAR (0);
      /* Fall through.  */
    case 32:
      FRAG_APPEND_1_CHAR (0);
      FRAG_APPEND_1_CHAR (0);
      /* Fall through.  */
    case 16:
      FRAG_APPEND_1_CHAR (0);
      /* Fall through.  */
    case 8:
      break;
    default:
      /* Called with invalid bitsize argument.  */
      abort ();
      break;
    }
  if (target_big_endian)
    FRAG_APPEND_1_CHAR (c);
}

/* Worker to do .ascii etc statements.
   Reads 0 or more ',' separated, double-quoted strings.
   Caller should have checked need_pass_2 is FALSE because we don't
   check it.
   Checks for end-of-line.
   BITS_APPENDZERO says how many bits are in a target char.
   The bottom bit is set if a NUL char should be appended to the strings.  */

void
stringer (int bits_appendzero)
{
  const int bitsize = bits_appendzero & ~7;
  const int append_zero = bits_appendzero & 1;
  unsigned int c;
#if !defined(NO_LISTING) && defined (OBJ_ELF)
  char *start;
#endif

#ifdef md_flush_pending_output
  md_flush_pending_output ();
#endif

#ifdef md_cons_align
  md_cons_align (1);
#endif

  /* If we have been switched into the abs_section then we
     will not have an obstack onto which we can hang strings.  */
  if (now_seg == absolute_section)
    {
      as_bad (_("strings must be placed into a section"));
      ignore_rest_of_line ();
      return;
    }

  /* The following awkward logic is to parse ZERO or more strings,
     comma separated. Recall a string expression includes spaces
     before the opening '\"' and spaces after the closing '\"'.
     We fake a leading ',' if there is (supposed to be)
     a 1st, expression. We keep demanding expressions for each ','.  */
  if (is_it_end_of_statement ())
    {
      c = 0;			/* Skip loop.  */
      ++input_line_pointer;	/* Compensate for end of loop.  */
    }
  else
    {
      c = ',';			/* Do loop.  */
    }

  while (c == ',' || c == '<' || c == '"')
    {
      SKIP_WHITESPACE ();
      switch (*input_line_pointer)
	{
	case '\"':
	  ++input_line_pointer;	/*->1st char of string.  */
#if !defined(NO_LISTING) && defined (OBJ_ELF)
	  start = input_line_pointer;
#endif

	  while (is_a_char (c = next_char_of_string ()))
	    stringer_append_char (c, bitsize);

	  /* Treat "a" "b" as "ab".  Even if we are appending zeros.  */
	  SKIP_ALL_WHITESPACE ();
	  if (*input_line_pointer == '"')
	    break;

	  if (append_zero)
	    stringer_append_char (0, bitsize);

#if !defined(NO_LISTING) && defined (OBJ_ELF)
	  /* In ELF, when gcc is emitting DWARF 1 debugging output, it
	     will emit .string with a filename in the .debug section
	     after a sequence of constants.  See the comment in
	     emit_expr for the sequence.  emit_expr will set
	     dwarf_file_string to non-zero if this string might be a
	     source file name.  */
	  if (strcmp (segment_name (now_seg), ".debug") != 0)
	    dwarf_file_string = 0;
	  else if (dwarf_file_string)
	    {
	      c = input_line_pointer[-1];
	      input_line_pointer[-1] = '\0';
	      listing_source_file (start);
	      input_line_pointer[-1] = c;
	    }
#endif

	  break;
	case '<':
	  input_line_pointer++;
	  c = get_single_number ();
	  stringer_append_char (c, bitsize);
	  if (*input_line_pointer != '>')
	    {
	      as_bad (_("expected <nn>"));
	      ignore_rest_of_line ();
	      return;
	    }
	  input_line_pointer++;
	  break;
	case ',':
	  input_line_pointer++;
	  break;
	}
      SKIP_WHITESPACE ();
      c = *input_line_pointer;
    }

  demand_empty_rest_of_line ();
}

/* FIXME-SOMEDAY: I had trouble here on characters with the
    high bits set.  We'll probably also have trouble with
    multibyte chars, wide chars, etc.  Also be careful about
    returning values bigger than 1 byte.  xoxorich.  */

unsigned int
next_char_of_string (void)
{
  unsigned int c;

  c = *input_line_pointer++ & CHAR_MASK;
  switch (c)
    {
    case 0:
      /* PR 20902: Do not advance past the end of the buffer.  */
      -- input_line_pointer;
      c = NOT_A_CHAR;
      break;

    case '\"':
      c = NOT_A_CHAR;
      break;

    case '\n':
      as_warn (_("unterminated string; newline inserted"));
      bump_line_counters ();
      break;

    case '\\':
      if (!TC_STRING_ESCAPES)
	break;
      switch (c = *input_line_pointer++ & CHAR_MASK)
	{
	case 'b':
	  c = '\b';
	  break;

	case 'f':
	  c = '\f';
	  break;

	case 'n':
	  c = '\n';
	  break;

	case 'r':
	  c = '\r';
	  break;

	case 't':
	  c = '\t';
	  break;

	case 'v':
	  c = '\013';
	  break;

	case '\\':
	case '"':
	  break;		/* As itself.  */

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
	  {
	    unsigned number;
	    int i;

	    for (i = 0, number = 0;
		 ISDIGIT (c) && i < 3;
		 c = *input_line_pointer++, i++)
	      {
		number = number * 8 + c - '0';
	      }

	    c = number & CHAR_MASK;
	  }
	  --input_line_pointer;
	  break;

	case 'x':
	case 'X':
	  {
	    unsigned number;

	    number = 0;
	    c = *input_line_pointer++;
	    while (ISXDIGIT (c))
	      {
		if (ISDIGIT (c))
		  number = number * 16 + c - '0';
		else if (ISUPPER (c))
		  number = number * 16 + c - 'A' + 10;
		else
		  number = number * 16 + c - 'a' + 10;
		c = *input_line_pointer++;
	      }
	    c = number & CHAR_MASK;
	    --input_line_pointer;
	  }
	  break;

	case '\n':
	  /* To be compatible with BSD 4.2 as: give the luser a linefeed!!  */
	  as_warn (_("unterminated string; newline inserted"));
	  c = '\n';
	  bump_line_counters ();
	  break;

	case 0:
	  /* Do not advance past the end of the buffer.  */
	  -- input_line_pointer;
	  c = NOT_A_CHAR;
	  break;

	default:

#ifdef ONLY_STANDARD_ESCAPES
	  as_bad (_("bad escaped character in string"));
	  c = '?';
#endif /* ONLY_STANDARD_ESCAPES */

	  break;
	}
      break;

    default:
      break;
    }
  return (c);
}

static segT
get_segmented_expression (expressionS *expP)
{
  segT retval;

  retval = expression (expP);
  if (expP->X_op == O_illegal
      || expP->X_op == O_absent
      || expP->X_op == O_big)
    {
      as_bad (_("expected address expression"));
      expP->X_op = O_constant;
      expP->X_add_number = 0;
      retval = absolute_section;
    }
  return retval;
}

static segT
get_known_segmented_expression (expressionS *expP)
{
  segT retval = get_segmented_expression (expP);

  if (retval == undefined_section)
    {
      /* There is no easy way to extract the undefined symbol from the
	 expression.  */
      if (expP->X_add_symbol != NULL
	  && S_GET_SEGMENT (expP->X_add_symbol) != expr_section)
	as_warn (_("symbol \"%s\" undefined; zero assumed"),
		 S_GET_NAME (expP->X_add_symbol));
      else
	as_warn (_("some symbol undefined; zero assumed"));
      retval = absolute_section;
      expP->X_op = O_constant;
      expP->X_add_number = 0;
    }
  return retval;
}

char				/* Return terminator.  */
get_absolute_expression_and_terminator (long *val_pointer /* Return value of expression.  */)
{
  /* FIXME: val_pointer should probably be offsetT *.  */
  *val_pointer = (long) get_absolute_expression ();
  return (*input_line_pointer++);
}

/* Like demand_copy_string, but return NULL if the string contains any '\0's.
   Give a warning if that happens.  */

char *
demand_copy_C_string (int *len_pointer)
{
  char *s;

  if ((s = demand_copy_string (len_pointer)) != 0)
    {
      int len;

      for (len = *len_pointer; len > 0; len--)
	{
	  if (s[len - 1] == 0)
	    {
	      s = 0;
	      *len_pointer = 0;
	      as_bad (_("this string may not contain \'\\0\'"));
	      break;
	    }
	}
    }

  return s;
}

/* Demand string, but return a safe (=private) copy of the string.
   Return NULL if we can't read a string here.  */

char *
demand_copy_string (int *lenP)
{
  unsigned int c;
  int len;
  char *retval;

  len = 0;
  SKIP_WHITESPACE ();
  if (*input_line_pointer == '\"')
    {
      input_line_pointer++;	/* Skip opening quote.  */

      while (is_a_char (c = next_char_of_string ()))
	{
	  obstack_1grow (&notes, c);
	  len++;
	}
      /* JF this next line is so demand_copy_C_string will return a
	 null terminated string.  */
      obstack_1grow (&notes, '\0');
      retval = (char *) obstack_finish (&notes);
    }
  else
    {
      as_bad (_("missing string"));
      retval = NULL;
      ignore_rest_of_line ();
    }
  *lenP = len;
  return (retval);
}

/* In:	Input_line_pointer->next character.

   Do:	Skip input_line_pointer over all whitespace.

   Out:	1 if input_line_pointer->end-of-line.  */

int
is_it_end_of_statement (void)
{
  SKIP_WHITESPACE ();
  return (is_end_of_line[(unsigned char) *input_line_pointer]);
}

void
equals (char *sym_name, int reassign)
{
  char *stop = NULL;
  char stopc = 0;

  input_line_pointer++;
  if (*input_line_pointer == '=')
    input_line_pointer++;
  if (reassign < 0 && *input_line_pointer == '=')
    input_line_pointer++;

  while (*input_line_pointer == ' ' || *input_line_pointer == '\t')
    input_line_pointer++;

  if (flag_mri)
    stop = mri_comment_field (&stopc);

  assign_symbol (sym_name, reassign >= 0 ? !reassign : reassign);

  if (flag_mri)
    {
      demand_empty_rest_of_line ();
      mri_comment_end (stop, stopc);
    }
}

/* Open FILENAME, first trying the unadorned file name, then if that
   fails and the file name is not an absolute path, attempt to open
   the file in current -I include paths.  PATH is a preallocated
   buffer which will be set to the file opened, or FILENAME if no file
   is found.  */

FILE *
search_and_open (const char *filename, char *path)
{
  FILE *f = fopen (filename, FOPEN_RB);
  if (f == NULL && !IS_ABSOLUTE_PATH (filename))
    {
      for (size_t i = 0; i < include_dir_count; i++)
	{
	  sprintf (path, "%s/%s", include_dirs[i], filename);
	  f = fopen (path, FOPEN_RB);
	  if (f != NULL)
	    return f;
	}
    }
  strcpy (path, filename);
  return f;
}

/* .incbin -- include a file verbatim at the current location.  */

void
s_incbin (int x ATTRIBUTE_UNUSED)
{
  FILE * binfile;
  char * path;
  char * filename;
  char * binfrag;
  long   skip = 0;
  long   count = 0;
  long   bytes;
  int    len;

#ifdef md_flush_pending_output
  md_flush_pending_output ();
#endif

#ifdef md_cons_align
  md_cons_align (1);
#endif

  SKIP_WHITESPACE ();
  filename = demand_copy_string (& len);
  if (filename == NULL)
    return;

  SKIP_WHITESPACE ();

  /* Look for optional skip and count.  */
  if (* input_line_pointer == ',')
    {
      ++ input_line_pointer;
      skip = get_absolute_expression ();

      SKIP_WHITESPACE ();

      if (* input_line_pointer == ',')
	{
	  ++ input_line_pointer;

	  count = get_absolute_expression ();
	  if (count == 0)
	    as_warn (_(".incbin count zero, ignoring `%s'"), filename);

	  SKIP_WHITESPACE ();
	}
    }

  demand_empty_rest_of_line ();

  path = XNEWVEC (char, len + include_dir_maxlen + 2);
  binfile = search_and_open (filename, path);

  if (binfile == NULL)
    as_bad (_("file not found: %s"), filename);
  else
    {
      long   file_len;
      struct stat filestat;

      if (fstat (fileno (binfile), &filestat) != 0
	  || ! S_ISREG (filestat.st_mode)
	  || S_ISDIR (filestat.st_mode))
	{
	  as_bad (_("unable to include `%s'"), path);
	  goto done;
	}
      
      register_dependency (path);

      /* Compute the length of the file.  */
      if (fseek (binfile, 0, SEEK_END) != 0)
	{
	  as_bad (_("seek to end of .incbin file failed `%s'"), path);
	  goto done;
	}
      file_len = ftell (binfile);

      /* If a count was not specified use the remainder of the file.  */
      if (count == 0)
	count = file_len - skip;

      if (skip < 0 || count < 0 || file_len < 0 || skip + count > file_len)
	{
	  as_bad (_("skip (%ld) or count (%ld) invalid for file size (%ld)"),
		  skip, count, file_len);
	  goto done;
	}

      if (fseek (binfile, skip, SEEK_SET) != 0)
	{
	  as_bad (_("could not skip to %ld in file `%s'"), skip, path);
	  goto done;
	}

      /* Allocate frag space and store file contents in it.  */
      binfrag = frag_more (count);

      bytes = fread (binfrag, 1, count, binfile);
      if (bytes < count)
	as_warn (_("truncated file `%s', %ld of %ld bytes read"),
		 path, bytes, count);
    }
 done:
  if (binfile != NULL)
    fclose (binfile);
  free (path);
}

/* .include -- include a file at this point.  */

void
s_include (int arg ATTRIBUTE_UNUSED)
{
  char *filename;
  int i;
  FILE *try_file;
  char *path;

  if (!flag_m68k_mri)
    {
      filename = demand_copy_string (&i);
      if (filename == NULL)
	{
	  /* demand_copy_string has already printed an error and
	     called ignore_rest_of_line.  */
	  return;
	}
    }
  else
    {
      SKIP_WHITESPACE ();
      i = 0;
      while (!is_end_of_line[(unsigned char) *input_line_pointer]
	     && *input_line_pointer != ' '
	     && *input_line_pointer != '\t')
	{
	  obstack_1grow (&notes, *input_line_pointer);
	  ++input_line_pointer;
	  ++i;
	}

      obstack_1grow (&notes, '\0');
      filename = (char *) obstack_finish (&notes);
      while (!is_end_of_line[(unsigned char) *input_line_pointer])
	++input_line_pointer;
    }

  demand_empty_rest_of_line ();

  path = notes_alloc (i + include_dir_maxlen + 2);
  try_file = search_and_open (filename, path);
  if (try_file)
    fclose (try_file);

  register_dependency (path);
  input_scrub_insert_file (path);
}

void
init_include_dir (void)
{
  include_dirs = XNEWVEC (const char *, 1);
  include_dirs[0] = ".";	/* Current dir.  */
  include_dir_count = 1;
  include_dir_maxlen = 1;
}

void
add_include_dir (char *path)
{
  include_dir_count++;
  include_dirs = XRESIZEVEC (const char *, include_dirs, include_dir_count);
  include_dirs[include_dir_count - 1] = path;	/* New one.  */

  size_t i = strlen (path);
  if (i > include_dir_maxlen)
    include_dir_maxlen = i;
}

/* Output debugging information to denote the source file.  */

static void
generate_file_debug (void)
{
  if (debug_type == DEBUG_STABS)
    stabs_generate_asm_file ();
}

/* Output line number debugging information for the current source line.  */

void
generate_lineno_debug (void)
{
  switch (debug_type)
    {
    case DEBUG_UNSPECIFIED:
    case DEBUG_NONE:
    case DEBUG_DWARF:
      break;
    case DEBUG_STABS:
      stabs_generate_asm_lineno ();
      break;
    case DEBUG_ECOFF:
      ecoff_generate_asm_lineno ();
      break;
    case DEBUG_DWARF2:
      /* ??? We could here indicate to dwarf2dbg.c that something
	 has changed.  However, since there is additional backend
	 support that is required (calling dwarf2_emit_insn), we
	 let dwarf2dbg.c call as_where on its own.  */
      break;
    case DEBUG_CODEVIEW:
      codeview_generate_asm_lineno ();
      break;
    }
}

/* Output debugging information to mark a function entry point or end point.
   END_P is zero for .func, and non-zero for .endfunc.  */

void
s_func (int end_p)
{
  do_s_func (end_p, NULL);
}

/* Subroutine of s_func so targets can choose a different default prefix.
   If DEFAULT_PREFIX is NULL, use the target's "leading char".  */

static void
do_s_func (int end_p, const char *default_prefix)
{
  if (end_p)
    {
      if (current_name == NULL)
	{
	  as_bad (_("missing .func"));
	  ignore_rest_of_line ();
	  return;
	}

      if (debug_type == DEBUG_STABS)
	stabs_generate_asm_endfunc (current_name, current_label);

      free (current_name);
      free (current_label);
      current_name = current_label = NULL;
    }
  else /* ! end_p */
    {
      char *name, *label;
      char delim1, delim2;

      if (current_name != NULL)
	{
	  as_bad (_(".endfunc missing for previous .func"));
	  ignore_rest_of_line ();
	  return;
	}

      delim1 = get_symbol_name (& name);
      name = xstrdup (name);
      *input_line_pointer = delim1;
      SKIP_WHITESPACE_AFTER_NAME ();
      if (*input_line_pointer != ',')
	{
	  if (default_prefix)
	    {
	      if (asprintf (&label, "%s%s", default_prefix, name) == -1)
		as_fatal ("%s", xstrerror (errno));
	    }
	  else
	    {
	      char leading_char = bfd_get_symbol_leading_char (stdoutput);
	      /* Missing entry point, use function's name with the leading
		 char prepended.  */
	      if (leading_char)
		{
		  if (asprintf (&label, "%c%s", leading_char, name) == -1)
		    as_fatal ("%s", xstrerror (errno));
		}
	      else
		label = xstrdup (name);
	    }
	}
      else
	{
	  ++input_line_pointer;
	  SKIP_WHITESPACE ();
	  delim2 = get_symbol_name (& label);
	  label = xstrdup (label);
	  restore_line_pointer (delim2);
	}

      if (debug_type == DEBUG_STABS)
	stabs_generate_asm_func (name, label);

      current_name = name;
      current_label = label;
    }

  demand_empty_rest_of_line ();
}

#ifdef HANDLE_BUNDLE

void
s_bundle_align_mode (int arg ATTRIBUTE_UNUSED)
{
  unsigned int align = get_absolute_expression ();
  SKIP_WHITESPACE ();
  demand_empty_rest_of_line ();

  if (align > (unsigned int) TC_ALIGN_LIMIT)
    as_fatal (_(".bundle_align_mode alignment too large (maximum %u)"),
	      (unsigned int) TC_ALIGN_LIMIT);

  if (bundle_lock_frag != NULL)
    {
      as_bad (_("cannot change .bundle_align_mode inside .bundle_lock"));
      return;
    }

  bundle_align_p2 = align;
}

void
s_bundle_lock (int arg ATTRIBUTE_UNUSED)
{
  demand_empty_rest_of_line ();

  if (bundle_align_p2 == 0)
    {
      as_bad (_(".bundle_lock is meaningless without .bundle_align_mode"));
      return;
    }

  if (bundle_lock_depth == 0)
    {
      bundle_lock_frchain = frchain_now;
      bundle_lock_frag = start_bundle ();
    }
  ++bundle_lock_depth;
}

void
s_bundle_unlock (int arg ATTRIBUTE_UNUSED)
{
  unsigned int size;

  demand_empty_rest_of_line ();

  if (bundle_lock_frag == NULL)
    {
      as_bad (_(".bundle_unlock without preceding .bundle_lock"));
      return;
    }

  gas_assert (bundle_align_p2 > 0);

  gas_assert (bundle_lock_depth > 0);
  if (--bundle_lock_depth > 0)
    return;

  size = pending_bundle_size (bundle_lock_frag);

  if (size > 1U << bundle_align_p2)
    as_bad (_(".bundle_lock sequence is %u bytes, "
	      "but bundle size is only %u bytes"),
	    size, 1u << bundle_align_p2);
  else
    finish_bundle (bundle_lock_frag, size);

  bundle_lock_frag = NULL;
  bundle_lock_frchain = NULL;
}

#endif  /* HANDLE_BUNDLE */

void
s_ignore (int arg ATTRIBUTE_UNUSED)
{
  ignore_rest_of_line ();
}

void
read_print_statistics (FILE *file)
{
  htab_print_statistics (file, "pseudo-op table", po_hash);
}

/* Inserts the given line into the input stream.

   This call avoids macro/conditionals nesting checking, since the contents of
   the line are assumed to replace the contents of a line already scanned.

   An appropriate use of this function would be substitution of input lines when
   called by md_start_line_hook().  The given line is assumed to already be
   properly scrubbed.  */

void
input_scrub_insert_line (const char *line)
{
  sb newline;
  size_t len = strlen (line);
  sb_build (&newline, len);
  sb_add_buffer (&newline, line, len);
  input_scrub_include_sb (&newline, input_line_pointer, expanding_none);
  sb_kill (&newline);
  buffer_limit = input_scrub_next_buffer (&input_line_pointer);
}

/* Insert a file into the input stream; the path must resolve to an actual
   file; no include path searching or dependency registering is performed.  */

void
input_scrub_insert_file (char *path)
{
  input_scrub_include_file (path, input_line_pointer);
  buffer_limit = input_scrub_next_buffer (&input_line_pointer);
}

/* Find the end of a line, considering quotation and escaping of quotes.  */

#if !defined(TC_SINGLE_QUOTE_STRINGS) && defined(SINGLE_QUOTE_STRINGS)
# define TC_SINGLE_QUOTE_STRINGS 1
#endif

static char *
_find_end_of_line (char *s, int mri_string, int insn ATTRIBUTE_UNUSED,
		   int in_macro)
{
  char inquote = '\0';
  int inescape = 0;

  while (!is_end_of_line[(unsigned char) *s]
	 || (inquote && !ISCNTRL (*s))
	 || (inquote == '\'' && flag_mri)
#ifdef TC_EOL_IN_INSN
	 || (insn && TC_EOL_IN_INSN (s))
#endif
	 /* PR 6926:  When we are parsing the body of a macro the sequence
	    \@ is special - it refers to the invocation count.  If the @
	    character happens to be registered as a line-separator character
	    by the target, then the is_end_of_line[] test above will have
	    returned true, but we need to ignore the line separating
	    semantics in this particular case.  */
	 || (in_macro && inescape && *s == '@')
	)
    {
      if (mri_string && *s == '\'')
	inquote ^= *s;
      else if (inescape)
	inescape = 0;
      else if (*s == '\\')
	inescape = 1;
      else if (!inquote
	       ? *s == '"'
#ifdef TC_SINGLE_QUOTE_STRINGS
		 || (TC_SINGLE_QUOTE_STRINGS && *s == '\'')
#endif
	       : *s == inquote)
	inquote ^= *s;
      ++s;
    }
  if (inquote)
    as_warn (_("missing closing `%c'"), inquote);
  if (inescape && !ignore_input ())
    as_warn (_("stray `\\'"));
  return s;
}

char *
find_end_of_line (char *s, int mri_string)
{
  return _find_end_of_line (s, mri_string, 0, 0);
}

static char *saved_ilp;
static char *saved_limit;

/* Use BUF as a temporary input pointer for calling other functions in this
   file.  BUF must be a C string, so that its end can be found by strlen.
   Also sets the buffer_limit variable (local to this file) so that buffer
   overruns should not occur.  Saves the current input line pointer so that
   it can be restored by calling restore_ilp().

   Does not support recursion.  */

void
temp_ilp (char *buf)
{
  gas_assert (saved_ilp == NULL);
  gas_assert (buf != NULL);

  saved_ilp = input_line_pointer;
  saved_limit = buffer_limit;
  /* Prevent the assert in restore_ilp from triggering if
     the input_line_pointer has not yet been initialised.  */
  if (saved_ilp == NULL)
    saved_limit = saved_ilp = (char *) "";

  input_line_pointer = buf;
  buffer_limit = buf + strlen (buf);
  input_from_string = true;
}

/* Restore a saved input line pointer.  */

void
restore_ilp (void)
{
  gas_assert (saved_ilp != NULL);

  input_line_pointer = saved_ilp;
  buffer_limit = saved_limit;
  input_from_string = false;

  saved_ilp = NULL;
}
