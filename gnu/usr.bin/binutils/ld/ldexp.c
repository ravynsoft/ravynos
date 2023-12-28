/* This module handles expression trees.
   Copyright (C) 1991-2023 Free Software Foundation, Inc.
   Written by Steve Chamberlain of Cygnus Support <sac@cygnus.com>.

   This file is part of the GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */


/* This module is in charge of working out the contents of expressions.

   It has to keep track of the relative/absness of a symbol etc. This
   is done by keeping all values in a struct (an etree_value_type)
   which contains a value, a section to which it is relative and a
   valid bit.  */

#include "sysdep.h"
#include "bfd.h"
#include "bfdlink.h"
#include "ctf-api.h"

#include "ld.h"
#include "ldmain.h"
#include "ldmisc.h"
#include "ldexp.h"
#include "ldlex.h"
#include <ldgram.h>
#include "ldlang.h"
#include "libiberty.h"
#include "safe-ctype.h"

static void exp_fold_tree_1 (etree_type *);
static bfd_vma align_n (bfd_vma, bfd_vma);

segment_type *segments;

struct ldexp_control expld;

/* This structure records symbols for which we need to keep track of
   definedness for use in the DEFINED () test.  It is also used in
   making absolute symbols section relative late in the link.   */

struct definedness_hash_entry
{
  struct bfd_hash_entry root;

  /* If this symbol was assigned from "dot" outside of an output
     section statement, the section we'd like it relative to.  */
  asection *final_sec;

  /* Low bits of iteration count.  Symbols with matching iteration have
     been defined in this pass over the script.  */
  unsigned int iteration : 8;

  /* Symbol was defined by an object file.  */
  unsigned int by_object : 1;
};

static struct bfd_hash_table definedness_table;

/* Print the string representation of the given token.  Surround it
   with spaces if INFIX_P is TRUE.  */

static void
exp_print_token (token_code_type code, int infix_p)
{
  static const struct
  {
    token_code_type code;
    const char *name;
  }
  table[] =
  {
    { INT, "int" },
    { NAME, "NAME" },
    { PLUSEQ, "+=" },
    { MINUSEQ, "-=" },
    { MULTEQ, "*=" },
    { DIVEQ, "/=" },
    { LSHIFTEQ, "<<=" },
    { RSHIFTEQ, ">>=" },
    { ANDEQ, "&=" },
    { OREQ, "|=" },
    { OROR, "||" },
    { ANDAND, "&&" },
    { EQ, "==" },
    { NE, "!=" },
    { LE, "<=" },
    { GE, ">=" },
    { LSHIFT, "<<" },
    { RSHIFT, ">>" },
    { LOG2CEIL, "LOG2CEIL" },
    { ALIGN_K, "ALIGN" },
    { BLOCK, "BLOCK" },
    { QUAD, "QUAD" },
    { SQUAD, "SQUAD" },
    { LONG, "LONG" },
    { SHORT, "SHORT" },
    { BYTE, "BYTE" },
    { SECTIONS, "SECTIONS" },
    { SIZEOF_HEADERS, "SIZEOF_HEADERS" },
    { MEMORY, "MEMORY" },
    { DEFINED, "DEFINED" },
    { TARGET_K, "TARGET" },
    { SEARCH_DIR, "SEARCH_DIR" },
    { MAP, "MAP" },
    { ENTRY, "ENTRY" },
    { NEXT, "NEXT" },
    { ALIGNOF, "ALIGNOF" },
    { SIZEOF, "SIZEOF" },
    { ADDR, "ADDR" },
    { LOADADDR, "LOADADDR" },
    { CONSTANT, "CONSTANT" },
    { ABSOLUTE, "ABSOLUTE" },
    { MAX_K, "MAX" },
    { MIN_K, "MIN" },
    { ASSERT_K, "ASSERT" },
    { REL, "relocatable" },
    { DATA_SEGMENT_ALIGN, "DATA_SEGMENT_ALIGN" },
    { DATA_SEGMENT_RELRO_END, "DATA_SEGMENT_RELRO_END" },
    { DATA_SEGMENT_END, "DATA_SEGMENT_END" },
    { ORIGIN, "ORIGIN" },
    { LENGTH, "LENGTH" },
    { SEGMENT_START, "SEGMENT_START" }
  };
  unsigned int idx;

  for (idx = 0; idx < ARRAY_SIZE (table); idx++)
    if (table[idx].code == code)
      break;

  if (infix_p)
    fputc (' ', config.map_file);

  if (idx < ARRAY_SIZE (table))
    fputs (table[idx].name, config.map_file);
  else if (code < 127)
    fputc (code, config.map_file);
  else
    fprintf (config.map_file, "<code %d>", code);

  if (infix_p)
    fputc (' ', config.map_file);
}

static void
make_log2ceil (void)
{
  bfd_vma value = expld.result.value;
  bfd_vma result = -1;
  bool round_up = false;

  do
    {
      result++;
      /* If more than one bit is set in the value we will need to round up.  */
      if ((value > 1) && (value & 1))
	round_up = true;
    }
  while (value >>= 1);

  if (round_up)
    result += 1;
  expld.result.section = NULL;
  expld.result.value = result;
}

static void
make_abs (void)
{
  if (expld.result.section != NULL)
    expld.result.value += expld.result.section->vma;
  expld.result.section = bfd_abs_section_ptr;
  expld.rel_from_abs = false;
}

static void
new_abs (bfd_vma value)
{
  expld.result.valid_p = true;
  expld.result.section = bfd_abs_section_ptr;
  expld.result.value = value;
  expld.result.str = NULL;
}

etree_type *
exp_intop (bfd_vma value)
{
  etree_type *new_e = stat_alloc (sizeof (new_e->value));
  new_e->type.node_code = INT;
  new_e->type.filename = ldlex_filename ();
  new_e->type.lineno = lineno;
  new_e->value.value = value;
  new_e->value.str = NULL;
  new_e->type.node_class = etree_value;
  return new_e;
}

etree_type *
exp_bigintop (bfd_vma value, char *str)
{
  etree_type *new_e = stat_alloc (sizeof (new_e->value));
  new_e->type.node_code = INT;
  new_e->type.filename = ldlex_filename ();
  new_e->type.lineno = lineno;
  new_e->value.value = value;
  new_e->value.str = str;
  new_e->type.node_class = etree_value;
  return new_e;
}

/* Build an expression representing an unnamed relocatable value.  */

etree_type *
exp_relop (asection *section, bfd_vma value)
{
  etree_type *new_e = stat_alloc (sizeof (new_e->rel));
  new_e->type.node_code = REL;
  new_e->type.filename = ldlex_filename ();
  new_e->type.lineno = lineno;
  new_e->type.node_class = etree_rel;
  new_e->rel.section = section;
  new_e->rel.value = value;
  return new_e;
}

static void
new_number (bfd_vma value)
{
  expld.result.valid_p = true;
  expld.result.value = value;
  expld.result.str = NULL;
  expld.result.section = NULL;
}

static void
new_rel (bfd_vma value, asection *section)
{
  expld.result.valid_p = true;
  expld.result.value = value;
  expld.result.str = NULL;
  expld.result.section = section;
}

static void
new_rel_from_abs (bfd_vma value)
{
  asection *s = expld.section;

  expld.rel_from_abs = true;
  expld.result.valid_p = true;
  expld.result.value = value - s->vma;
  expld.result.str = NULL;
  expld.result.section = s;
}

/* New-function for the definedness hash table.  */

static struct bfd_hash_entry *
definedness_newfunc (struct bfd_hash_entry *entry,
		     struct bfd_hash_table *table ATTRIBUTE_UNUSED,
		     const char *name ATTRIBUTE_UNUSED)
{
  struct definedness_hash_entry *ret = (struct definedness_hash_entry *) entry;

  if (ret == NULL)
    ret = (struct definedness_hash_entry *)
      bfd_hash_allocate (table, sizeof (struct definedness_hash_entry));

  if (ret == NULL)
    einfo (_("%F%P: bfd_hash_allocate failed creating symbol %s\n"), name);

  ret->by_object = 0;
  ret->iteration = 0;
  return &ret->root;
}

/* Called during processing of linker script script expressions.
   For symbols assigned in a linker script, return a struct describing
   where the symbol is defined relative to the current expression,
   otherwise return NULL.  */

static struct definedness_hash_entry *
symbol_defined (const char *name)
{
  return ((struct definedness_hash_entry *)
	  bfd_hash_lookup (&definedness_table, name, false, false));
}

/* Update the definedness state of NAME.  Return FALSE if script symbol
   is multiply defining a strong symbol in an object.  */

static bool
update_definedness (const char *name, struct bfd_link_hash_entry *h)
{
  bool ret;
  struct definedness_hash_entry *defentry
    = (struct definedness_hash_entry *)
    bfd_hash_lookup (&definedness_table, name, true, false);

  if (defentry == NULL)
    einfo (_("%F%P: bfd_hash_lookup failed creating symbol %s\n"), name);

  /* If the symbol was already defined, and not by a script, then it
     must be defined by an object file or by the linker target code.  */
  ret = true;
  if (!h->ldscript_def
      && (h->type == bfd_link_hash_defined
	  || h->type == bfd_link_hash_defweak
	  || h->type == bfd_link_hash_common))
    {
      defentry->by_object = 1;
      if (h->type == bfd_link_hash_defined
	  && h->u.def.section->output_section != NULL
	  && !bfd_is_abs_section (h->u.def.section)
	  && !h->linker_def)
	ret = false;
    }

  defentry->iteration = lang_statement_iteration;
  defentry->final_sec = bfd_abs_section_ptr;
  if (expld.phase == lang_final_phase_enum
      && expld.rel_from_abs
      && expld.result.section == bfd_abs_section_ptr)
    defentry->final_sec = section_for_dot ();
  return ret;
}

static void
fold_segment_end (void)
{
  seg_align_type *seg = &expld.dataseg;

  if (expld.phase == lang_first_phase_enum
      || expld.section != bfd_abs_section_ptr)
    {
      expld.result.valid_p = false;
    }
  else if (seg->phase == exp_seg_align_seen
	   || seg->phase == exp_seg_relro_seen)
    {
      seg->phase = exp_seg_end_seen;
      seg->end = expld.result.value;
    }
  else if (seg->phase == exp_seg_done
	   || seg->phase == exp_seg_adjust
	   || seg->phase == exp_seg_relro_adjust)
    {
      /* OK.  */
    }
  else
    expld.result.valid_p = false;
}

static void
fold_unary (etree_type *tree)
{
  exp_fold_tree_1 (tree->unary.child);
  if (expld.result.valid_p)
    {
      switch (tree->type.node_code)
	{
	case ALIGN_K:
	  if (expld.phase != lang_first_phase_enum)
	    new_rel_from_abs (align_n (expld.dot, expld.result.value));
	  else
	    expld.result.valid_p = false;
	  break;

	case ABSOLUTE:
	  make_abs ();
	  break;

	case LOG2CEIL:
	  make_log2ceil ();
	  break;

	case '~':
	  expld.result.value = ~expld.result.value;
	  break;

	case '!':
	  expld.result.value = !expld.result.value;
	  break;

	case '-':
	  expld.result.value = -expld.result.value;
	  break;

	case NEXT:
	  /* Return next place aligned to value.  */
	  if (expld.phase != lang_first_phase_enum)
	    {
	      make_abs ();
	      expld.result.value = align_n (expld.dot, expld.result.value);
	    }
	  else
	    expld.result.valid_p = false;
	  break;

	case DATA_SEGMENT_END:
	  fold_segment_end ();
	  break;

	default:
	  FAIL ();
	  break;
	}
    }
}

/* Arithmetic operators, bitwise AND, bitwise OR and XOR keep the
   section of one of their operands only when the other operand is a
   plain number.  Losing the section when operating on two symbols,
   ie. a result of a plain number, is required for subtraction and
   XOR.  It's justifiable for the other operations on the grounds that
   adding, multiplying etc. two section relative values does not
   really make sense unless they are just treated as numbers.
   The same argument could be made for many expressions involving one
   symbol and a number.  For example, "1 << x" and "100 / x" probably
   should not be given the section of x.  The trouble is that if we
   fuss about such things the rules become complex and it is onerous
   to document ld expression evaluation.  */
static void
arith_result_section (const etree_value_type *lhs)
{
  if (expld.result.section == lhs->section)
    {
      if (expld.section == bfd_abs_section_ptr
	  && !config.sane_expr)
	/* Duplicate the insanity in exp_fold_tree_1 case etree_value.  */
	expld.result.section = bfd_abs_section_ptr;
      else
	expld.result.section = NULL;
    }
}

static void
fold_segment_align (etree_value_type *lhs)
{
  seg_align_type *seg = &expld.dataseg;

  seg->relro = exp_seg_relro_start;
  if (expld.phase == lang_first_phase_enum
      || expld.section != bfd_abs_section_ptr)
    expld.result.valid_p = false;
  else
    {
      bfd_vma maxpage = lhs->value;
      bfd_vma commonpage = expld.result.value;

      expld.result.value = align_n (expld.dot, maxpage);
      if (seg->phase == exp_seg_relro_adjust)
	expld.result.value = seg->base;
      else if (seg->phase == exp_seg_adjust)
	{
	  if (commonpage < maxpage)
	    expld.result.value += ((expld.dot + commonpage - 1)
				   & (maxpage - commonpage));
	}
      else
	{
	  if (!link_info.relro)
	    expld.result.value += expld.dot & (maxpage - 1);
	  if (seg->phase == exp_seg_done)
	    {
	      /* OK.  */
	    }
	  else if (seg->phase == exp_seg_none)
	    {
	      seg->phase = exp_seg_align_seen;
	      seg->base = expld.result.value;
	      seg->commonpagesize = commonpage;
	      seg->maxpagesize = maxpage;
	      seg->relropagesize = maxpage;
	      seg->relro_end = 0;
	    }
	  else
	    expld.result.valid_p = false;
	}
    }
}

static void
fold_segment_relro_end (etree_value_type *lhs)
{
  seg_align_type *seg = &expld.dataseg;

  /* Operands swapped!  XXX_SEGMENT_RELRO_END(offset,exp) has offset
     in expld.result and exp in lhs.  */
  seg->relro = exp_seg_relro_end;
  seg->relro_offset = expld.result.value;
  if (expld.phase == lang_first_phase_enum
      || expld.section != bfd_abs_section_ptr)
    expld.result.valid_p = false;
  else if (seg->phase == exp_seg_align_seen
	   || seg->phase == exp_seg_adjust
	   || seg->phase == exp_seg_relro_adjust
	   || seg->phase == exp_seg_done)
    {
      if (seg->phase == exp_seg_align_seen
	  || seg->phase == exp_seg_relro_adjust)
	seg->relro_end = lhs->value + expld.result.value;

      if (seg->phase == exp_seg_relro_adjust
	  && (seg->relro_end & (seg->relropagesize - 1)))
	{
	  seg->relro_end += seg->relropagesize - 1;
	  seg->relro_end &= ~(seg->relropagesize - 1);
	  expld.result.value = seg->relro_end - expld.result.value;
	}
      else
	expld.result.value = lhs->value;

      if (seg->phase == exp_seg_align_seen)
	seg->phase = exp_seg_relro_seen;
    }
  else
    expld.result.valid_p = false;
}

static void
fold_binary (etree_type *tree)
{
  etree_value_type lhs;
  exp_fold_tree_1 (tree->binary.lhs);

  /* The SEGMENT_START operator is special because its first
     operand is a string, not the name of a symbol.  Note that the
     operands have been swapped, so binary.lhs is second (default)
     operand, binary.rhs is first operand.  */
  if (expld.result.valid_p && tree->type.node_code == SEGMENT_START)
    {
      bfd_vma value = expld.result.value;
      const char *segment_name;
      segment_type *seg;

      /* Check to see if the user has overridden the default
	 value.  */
      segment_name = tree->binary.rhs->name.name;
      for (seg = segments; seg; seg = seg->next)
	if (strcmp (seg->name, segment_name) == 0)
	  {
	    if (!seg->used
		&& config.magic_demand_paged
		&& link_info.maxpagesize != 0
		&& (seg->value % link_info.maxpagesize) != 0)
	      einfo (_("%P: warning: address of `%s' "
		       "isn't multiple of maximum page size\n"),
		     segment_name);
	    seg->used = true;
	    value = seg->value;
	    break;
	  }
      new_rel_from_abs (value);
      return;
    }

  lhs = expld.result;
  exp_fold_tree_1 (tree->binary.rhs);
  expld.result.valid_p &= lhs.valid_p;

  if (expld.result.valid_p)
    {
      if (lhs.section != expld.result.section)
	{
	  /* If the values are from different sections, and neither is
	     just a number, make both the source arguments absolute.  */
	  if (expld.result.section != NULL
	      && lhs.section != NULL)
	    {
	      make_abs ();
	      lhs.value += lhs.section->vma;
	      lhs.section = bfd_abs_section_ptr;
	    }

	  /* If the rhs is just a number, keep the lhs section.  */
	  else if (expld.result.section == NULL)
	    {
	      expld.result.section = lhs.section;
	      /* Make this NULL so that we know one of the operands
		 was just a number, for later tests.  */
	      lhs.section = NULL;
	    }
	}
      /* At this point we know that both operands have the same
	 section, or at least one of them is a plain number.  */

      switch (tree->type.node_code)
	{
#define BOP(x, y) \
	case x:							\
	  expld.result.value = lhs.value y expld.result.value;	\
	  arith_result_section (&lhs);				\
	  break;

	  /* Comparison operators, logical AND, and logical OR always
	     return a plain number.  */
#define BOPN(x, y) \
	case x:							\
	  expld.result.value = lhs.value y expld.result.value;	\
	  expld.result.section = NULL;				\
	  break;

	  BOP ('+', +);
	  BOP ('*', *);
	  BOP ('-', -);
	  BOP (LSHIFT, <<);
	  BOP (RSHIFT, >>);
	  BOP ('&', &);
	  BOP ('^', ^);
	  BOP ('|', |);
	  BOPN (EQ, ==);
	  BOPN (NE, !=);
	  BOPN ('<', <);
	  BOPN ('>', >);
	  BOPN (LE, <=);
	  BOPN (GE, >=);
	  BOPN (ANDAND, &&);
	  BOPN (OROR, ||);

	case '%':
	  if (expld.result.value != 0)
	    expld.result.value = ((bfd_signed_vma) lhs.value
				  % (bfd_signed_vma) expld.result.value);
	  else if (expld.phase != lang_mark_phase_enum)
	    einfo (_("%F%P:%pS %% by zero\n"), tree->binary.rhs);
	  arith_result_section (&lhs);
	  break;

	case '/':
	  if (expld.result.value != 0)
	    expld.result.value = ((bfd_signed_vma) lhs.value
				  / (bfd_signed_vma) expld.result.value);
	  else if (expld.phase != lang_mark_phase_enum)
	    einfo (_("%F%P:%pS / by zero\n"), tree->binary.rhs);
	  arith_result_section (&lhs);
	  break;

	case MAX_K:
	  if (lhs.value > expld.result.value)
	    expld.result.value = lhs.value;
	  break;

	case MIN_K:
	  if (lhs.value < expld.result.value)
	    expld.result.value = lhs.value;
	  break;

	case ALIGN_K:
	  expld.result.value = align_n (lhs.value, expld.result.value);
	  break;

	case DATA_SEGMENT_ALIGN:
	  fold_segment_align (&lhs);
	  break;

	case DATA_SEGMENT_RELRO_END:
	  fold_segment_relro_end (&lhs);
	  break;

	default:
	  FAIL ();
	}
    }
}

static void
fold_trinary (etree_type *tree)
{
  struct bfd_link_hash_entry *save = expld.assign_src;

  exp_fold_tree_1 (tree->trinary.cond);
  expld.assign_src = save;
  if (expld.result.valid_p)
    exp_fold_tree_1 (expld.result.value
		     ? tree->trinary.lhs
		     : tree->trinary.rhs);
}

static void
fold_name (etree_type *tree)
{
  struct bfd_link_hash_entry *h;
  struct definedness_hash_entry *def;

  memset (&expld.result, 0, sizeof (expld.result));

  switch (tree->type.node_code)
    {
    case SIZEOF_HEADERS:
      link_info.load_phdrs = 1;
      if (expld.phase != lang_first_phase_enum)
	{
	  bfd_vma hdr_size = 0;
	  /* Don't find the real header size if only marking sections;
	     The bfd function may cache incorrect data.  */
	  if (expld.phase != lang_mark_phase_enum)
	    hdr_size = (bfd_sizeof_headers (link_info.output_bfd, &link_info)
			/ bfd_octets_per_byte (link_info.output_bfd, NULL));
	  new_number (hdr_size);
	}
      break;

    case DEFINED:
      h = bfd_wrapped_link_hash_lookup (link_info.output_bfd,
					&link_info,
					tree->name.name,
					false, false, true);
      new_number (h != NULL
		  && (h->type == bfd_link_hash_defined
		      || h->type == bfd_link_hash_defweak
		      || h->type == bfd_link_hash_common)
		  && (!h->ldscript_def
		      || (def = symbol_defined (tree->name.name)) == NULL
		      || def->by_object
		      || def->iteration == (lang_statement_iteration & 255)));
      break;

    case NAME:
      if (tree->name.name[0] == '.' && tree->name.name[1] == 0)
	new_rel_from_abs (expld.dot);
      else
	{
	  h = bfd_wrapped_link_hash_lookup (link_info.output_bfd,
					    &link_info,
					    tree->name.name,
					    true, false, true);
	  if (!h)
	    {
	      if (expld.phase != lang_first_phase_enum)
		einfo (_("%F%P: bfd_link_hash_lookup failed: %E\n"));
	    }
	  else if (h->type == bfd_link_hash_defined
		   || h->type == bfd_link_hash_defweak)
	    {
	      asection *output_section;

	      output_section = h->u.def.section->output_section;
	      if (output_section == NULL)
		{
		  if (expld.phase <= lang_mark_phase_enum)
		    new_rel (h->u.def.value, h->u.def.section);
		  else
		    einfo (_("%X%P:%pS: unresolvable symbol `%s'"
			     " referenced in expression\n"),
			   tree, tree->name.name);
		}
	      else if (output_section == bfd_abs_section_ptr
		       && (expld.section != bfd_abs_section_ptr
			   || config.sane_expr))
		new_number (h->u.def.value + h->u.def.section->output_offset);
	      else
		new_rel (h->u.def.value + h->u.def.section->output_offset,
			 output_section);
	    }
	  else if (expld.phase == lang_final_phase_enum
		   || (expld.phase != lang_mark_phase_enum
		       && expld.assigning_to_dot))
	    einfo (_("%F%P:%pS: undefined symbol `%s'"
		     " referenced in expression\n"),
		   tree, tree->name.name);
	  else if (h->type == bfd_link_hash_new)
	    {
	      h->type = bfd_link_hash_undefined;
	      h->u.undef.abfd = NULL;
	      if (h->u.undef.next == NULL && h != link_info.hash->undefs_tail)
		bfd_link_add_undef (link_info.hash, h);
	    }
	  if (expld.assign_src == NULL)
	    expld.assign_src = h;
	  else
	    expld.assign_src = (struct bfd_link_hash_entry *) - 1;

	  /* Self-assignment is only allowed for absolute symbols
	     defined in a linker script.  */
	  if (expld.assign_name != NULL
	      && strcmp (expld.assign_name, tree->name.name) == 0
	      && !(h != NULL
		   && (h->type == bfd_link_hash_defined
		       || h->type == bfd_link_hash_defweak)
		   && h->u.def.section == bfd_abs_section_ptr
		   && (def = symbol_defined (tree->name.name)) != NULL
		   && def->iteration == (lang_statement_iteration & 255)))
	    expld.assign_name = NULL;
	}
      break;

    case ADDR:
      if (expld.phase != lang_first_phase_enum)
	{
	  lang_output_section_statement_type *os;

	  os = lang_output_section_find (tree->name.name);
	  if (os == NULL)
	    {
	      if (expld.phase == lang_final_phase_enum)
		einfo (_("%F%P:%pS: undefined section `%s'"
			 " referenced in expression\n"),
		       tree, tree->name.name);
	    }
	  else if (os->processed_vma)
	    new_rel (0, os->bfd_section);
	}
      break;

    case LOADADDR:
      if (expld.phase != lang_first_phase_enum)
	{
	  lang_output_section_statement_type *os;

	  os = lang_output_section_find (tree->name.name);
	  if (os == NULL)
	    {
	      if (expld.phase == lang_final_phase_enum)
		einfo (_("%F%P:%pS: undefined section `%s'"
			 " referenced in expression\n"),
		       tree, tree->name.name);
	    }
	  else if (os->processed_lma)
	    {
	      if (os->load_base == NULL)
		new_abs (os->bfd_section->lma);
	      else
		{
		  exp_fold_tree_1 (os->load_base);
		  if (expld.result.valid_p)
		    make_abs ();
		}
	    }
	}
      break;

    case SIZEOF:
    case ALIGNOF:
      if (expld.phase != lang_first_phase_enum)
	{
	  lang_output_section_statement_type *os;

	  os = lang_output_section_find (tree->name.name);
	  if (os == NULL)
	    {
	      if (expld.phase == lang_final_phase_enum)
		einfo (_("%F%P:%pS: undefined section `%s'"
			 " referenced in expression\n"),
		       tree, tree->name.name);
	      new_number (0);
	    }
	  else if (os->bfd_section != NULL)
	    {
	      bfd_vma val;

	      if (tree->type.node_code == SIZEOF)
		{
		  if (os->processed_vma)
		    val = os->bfd_section->size;
		  else
		    /* If we've just called lang_reset_memory_regions,
		       size will be zero and a previous estimate of
		       size will be in rawsize.  */
		    val = os->bfd_section->rawsize;
		  val /= bfd_octets_per_byte (link_info.output_bfd,
					      os->bfd_section);
		}
	      else
		val = (bfd_vma)1 << os->bfd_section->alignment_power;

	      new_number (val);
	    }
	  else
	    new_number (0);
	}
      break;

    case LENGTH:
      {
	lang_memory_region_type *mem;

	mem = lang_memory_region_lookup (tree->name.name, false);
	if (mem != NULL)
	  new_number (mem->length);
	else
	  einfo (_("%F%P:%pS: undefined MEMORY region `%s'"
		   " referenced in expression\n"),
		 tree, tree->name.name);
      }
      break;

    case ORIGIN:
      {
	lang_memory_region_type *mem;

	mem = lang_memory_region_lookup (tree->name.name, false);
	if (mem != NULL)
	  new_rel_from_abs (mem->origin);
	else
	  einfo (_("%F%P:%pS: undefined MEMORY region `%s'"
		   " referenced in expression\n"),
		 tree, tree->name.name);
      }
      break;

    case CONSTANT:
      if (strcmp (tree->name.name, "MAXPAGESIZE") == 0)
	new_number (link_info.maxpagesize);
      else if (strcmp (tree->name.name, "COMMONPAGESIZE") == 0)
	new_number (link_info.commonpagesize);
      else
	einfo (_("%F%P:%pS: unknown constant `%s' referenced in expression\n"),
	       tree, tree->name.name);
      break;

    default:
      FAIL ();
      break;
    }
}

/* Return true if TREE is '.'.  */

static bool
is_dot (const etree_type *tree)
{
  return (tree->type.node_class == etree_name
	  && tree->type.node_code == NAME
	  && tree->name.name[0] == '.'
	  && tree->name.name[1] == 0);
}

/* Return true if TREE is a constant equal to VAL.  */

static bool
is_value (const etree_type *tree, bfd_vma val)
{
  return (tree->type.node_class == etree_value
	  && tree->value.value == val);
}

/* Return true if TREE is an absolute symbol equal to VAL defined in
   a linker script.  */

static bool
is_sym_value (const etree_type *tree, bfd_vma val)
{
  struct bfd_link_hash_entry *h;
  struct definedness_hash_entry *def;

  return (tree->type.node_class == etree_name
	  && tree->type.node_code == NAME
	  && (def = symbol_defined (tree->name.name)) != NULL
	  && def->iteration == (lang_statement_iteration & 255)
	  && (h = bfd_wrapped_link_hash_lookup (link_info.output_bfd,
						&link_info,
						tree->name.name,
						false, false, true)) != NULL
	  && h->ldscript_def
	  && h->type == bfd_link_hash_defined
	  && h->u.def.section == bfd_abs_section_ptr
	  && h->u.def.value == val);
}

/* Return true if TREE is ". != 0".  */

static bool
is_dot_ne_0 (const etree_type *tree)
{
  return (tree->type.node_class == etree_binary
	  && tree->type.node_code == NE
	  && is_dot (tree->binary.lhs)
	  && is_value (tree->binary.rhs, 0));
}

/* Return true if TREE is ". = . + 0" or ". = . + sym" where sym is an
   absolute constant with value 0 defined in a linker script.  */

static bool
is_dot_plus_0 (const etree_type *tree)
{
  return (tree->type.node_class == etree_binary
	  && tree->type.node_code == '+'
	  && is_dot (tree->binary.lhs)
	  && (is_value (tree->binary.rhs, 0)
	      || is_sym_value (tree->binary.rhs, 0)));
}

/* Return true if TREE is "ALIGN (. != 0 ? some_expression : 1)".  */

static bool
is_align_conditional (const etree_type *tree)
{
  if (tree->type.node_class == etree_unary
      && tree->type.node_code == ALIGN_K)
    {
      tree = tree->unary.child;
      return (tree->type.node_class == etree_trinary
	      && is_dot_ne_0 (tree->trinary.cond)
	      && is_value (tree->trinary.rhs, 1));
    }
  return false;
}

static void
exp_fold_tree_1 (etree_type *tree)
{
  if (tree == NULL)
    {
      memset (&expld.result, 0, sizeof (expld.result));
      return;
    }

  switch (tree->type.node_class)
    {
    case etree_value:
      if (expld.section == bfd_abs_section_ptr
	  && !config.sane_expr)
	new_abs (tree->value.value);
      else
	new_number (tree->value.value);
      expld.result.str = tree->value.str;
      break;

    case etree_rel:
      if (expld.phase != lang_first_phase_enum)
	{
	  asection *output_section = tree->rel.section->output_section;
	  new_rel (tree->rel.value + tree->rel.section->output_offset,
		   output_section);
	}
      else
	memset (&expld.result, 0, sizeof (expld.result));
      break;

    case etree_assert:
      exp_fold_tree_1 (tree->assert_s.child);
      if (expld.phase == lang_final_phase_enum && !expld.result.value)
	einfo ("%X%P: %s\n", tree->assert_s.message);
      break;

    case etree_unary:
      fold_unary (tree);
      break;

    case etree_binary:
      fold_binary (tree);
      break;

    case etree_trinary:
      fold_trinary (tree);
      break;

    case etree_assign:
    case etree_provide:
    case etree_provided:
      if (tree->assign.dst[0] == '.' && tree->assign.dst[1] == 0)
	{
	  if (tree->type.node_class != etree_assign)
	    einfo (_("%F%P:%pS can not PROVIDE assignment to"
		     " location counter\n"), tree);
	  if (expld.phase != lang_first_phase_enum)
	    {
	      /* Notify the folder that this is an assignment to dot.  */
	      expld.assigning_to_dot = true;
	      exp_fold_tree_1 (tree->assign.src);
	      expld.assigning_to_dot = false;

	      /* If we are assigning to dot inside an output section
		 arrange to keep the section, except for certain
		 expressions that evaluate to zero.  We ignore . = 0,
		 . = . + 0, and . = ALIGN (. != 0 ? expr : 1).
		 We can't ignore all expressions that evaluate to zero
		 because an otherwise empty section might have padding
		 added by an alignment expression that changes with
		 relaxation.  Such a section might have zero size
		 before relaxation and so be stripped incorrectly.  */
	      if (expld.phase == lang_mark_phase_enum
		  && expld.section != bfd_abs_section_ptr
		  && expld.section != bfd_und_section_ptr
		  && !(expld.result.valid_p
		       && expld.result.value == 0
		       && (is_value (tree->assign.src, 0)
			   || is_sym_value (tree->assign.src, 0)
			   || is_dot_plus_0 (tree->assign.src)
			   || is_align_conditional (tree->assign.src))))
		expld.section->flags |= SEC_KEEP;

	      if (!expld.result.valid_p
		  || expld.section == bfd_und_section_ptr)
		{
		  if (expld.phase != lang_mark_phase_enum)
		    einfo (_("%F%P:%pS invalid assignment to"
			     " location counter\n"), tree);
		}
	      else if (expld.dotp == NULL)
		einfo (_("%F%P:%pS assignment to location counter"
			 " invalid outside of SECTIONS\n"), tree);

	      /* After allocation, assignment to dot should not be
		 done inside an output section since allocation adds a
		 padding statement that effectively duplicates the
		 assignment.  */
	      else if (expld.phase <= lang_allocating_phase_enum
		       || expld.section == bfd_abs_section_ptr)
		{
		  bfd_vma nextdot;

		  nextdot = expld.result.value;
		  if (expld.result.section != NULL)
		    nextdot += expld.result.section->vma;
		  else
		    nextdot += expld.section->vma;
		  if (nextdot < expld.dot
		      && expld.section != bfd_abs_section_ptr)
		    einfo (_("%F%P:%pS cannot move location counter backwards"
			     " (from %V to %V)\n"),
			   tree, expld.dot, nextdot);
		  else
		    {
		      expld.dot = nextdot;
		      *expld.dotp = nextdot;
		    }
		}
	    }
	  else
	    memset (&expld.result, 0, sizeof (expld.result));
	}
      else
	{
	  struct bfd_link_hash_entry *h = NULL;

	  if (tree->type.node_class == etree_provide)
	    {
	      h = bfd_link_hash_lookup (link_info.hash, tree->assign.dst,
					false, false, true);
	      if (h == NULL
		  || !(h->type == bfd_link_hash_new
		       || h->type == bfd_link_hash_undefined
		       || h->type == bfd_link_hash_undefweak
		       || h->linker_def))
		{
		  /* Do nothing.  The symbol was never referenced, or
		     was defined in some object file.  Note that
		     undefweak symbols are defined by PROVIDE.  This
		     is to support glibc use of __rela_iplt_start and
		     similar weak references.  */
		  break;
		}
	    }

	  expld.assign_name = tree->assign.dst;
	  expld.assign_src = NULL;
	  exp_fold_tree_1 (tree->assign.src);
	  /* expld.assign_name remaining equal to tree->assign.dst
	     below indicates the evaluation of tree->assign.src did
	     not use the value of tree->assign.dst.  We don't allow
	     self assignment until the final phase for two reasons:
	     1) Expressions are evaluated multiple times.  With
	     relaxation, the number of times may vary.
	     2) Section relative symbol values cannot be correctly
	     converted to absolute values, as is required by many
	     expressions, until final section sizing is complete.  */
	  if (expld.phase == lang_final_phase_enum
	      || expld.phase == lang_fixed_phase_enum
	      || expld.assign_name != NULL)
	    {
	      if (tree->type.node_class == etree_provide)
		tree->type.node_class = etree_provided;

	      if (h == NULL)
		{
		  h = bfd_link_hash_lookup (link_info.hash, tree->assign.dst,
					    true, false, true);
		  if (h == NULL)
		    einfo (_("%F%P:%s: hash creation failed\n"),
			   tree->assign.dst);
		}

              /* If the expression is not valid then fake a zero value.  In
                 the final phase any errors will already have been raised,
                 in earlier phases we want to create this definition so
                 that it can be seen by other expressions.  */
              if (!expld.result.valid_p
                  && h->type == bfd_link_hash_new)
                {
                  expld.result.value = 0;
                  expld.result.section = NULL;
                  expld.result.valid_p = true;
                }

	      if (expld.result.valid_p)
		{
		  if (expld.result.section == NULL)
		    expld.result.section = expld.section;
		  if (!update_definedness (tree->assign.dst, h)
		      && expld.assign_name != NULL)
		    {
		      /* Symbol was already defined, and the script isn't
			 modifying the symbol value for some reason as in
			 ld-elf/var1 and ld-scripts/pr14962.
			 For now this is only a warning.  */
		      unsigned int warn = link_info.warn_multiple_definition;
		      link_info.warn_multiple_definition = 1;
		      (*link_info.callbacks->multiple_definition)
			(&link_info, h, link_info.output_bfd,
			 expld.result.section, expld.result.value);
		      link_info.warn_multiple_definition = warn;
		    }
		  if (expld.phase == lang_fixed_phase_enum)
		    {
		      if (h->type == bfd_link_hash_defined)
			{
			  expld.result.value = h->u.def.value;
			  expld.result.section = h->u.def.section;
			}
		    }
		  else
		    {
		      h->type = bfd_link_hash_defined;
		      h->u.def.value = expld.result.value;
		      h->u.def.section = expld.result.section;
		      h->linker_def = ! tree->assign.type.lineno;
		      h->ldscript_def = 1;
		      h->rel_from_abs = expld.rel_from_abs;
		      if (tree->assign.hidden)
			bfd_link_hide_symbol (link_info.output_bfd,
					      &link_info, h);

		      /* Copy the symbol type and set non_ir_ref_regular
			 on the source if this is an expression only
			 referencing a single symbol.  (If the expression
			 contains ternary conditions, ignoring symbols on
			 false branches.)  */
		      if (expld.assign_src != NULL
			  && (expld.assign_src
			      != (struct bfd_link_hash_entry *) -1))
			{
			  bfd_copy_link_hash_symbol_type (link_info.output_bfd,
							  h, expld.assign_src);
			  expld.assign_src->non_ir_ref_regular = true;
			}
		    }
		}
	    }
	  if (expld.phase != lang_fixed_phase_enum)
	    expld.assign_name = NULL;
	}
      break;

    case etree_name:
      fold_name (tree);
      break;

    default:
      FAIL ();
      memset (&expld.result, 0, sizeof (expld.result));
      break;
    }
}

void
exp_fold_tree (etree_type *tree, asection *current_section, bfd_vma *dotp)
{
  expld.rel_from_abs = false;
  expld.dot = *dotp;
  expld.dotp = dotp;
  expld.section = current_section;
  exp_fold_tree_1 (tree);
}

void
exp_fold_tree_no_dot (etree_type *tree)
{
  expld.rel_from_abs = false;
  expld.dot = 0;
  expld.dotp = NULL;
  expld.section = bfd_abs_section_ptr;
  exp_fold_tree_1 (tree);
}

static void
exp_value_fold (etree_type *tree)
{
  exp_fold_tree_no_dot (tree);
  if (expld.result.valid_p)
    {
      tree->type.node_code = INT;
      tree->value.value = expld.result.value;
      tree->value.str = NULL;
      tree->type.node_class = etree_value;
    }
}

#define MAX(a, b) ((a) > (b) ? (a) : (b))

etree_type *
exp_binop (int code, etree_type *lhs, etree_type *rhs)
{
  etree_type *new_e = stat_alloc (MAX (sizeof (new_e->binary),
				       sizeof (new_e->value)));
  new_e->type.node_code = code;
  new_e->type.filename = lhs->type.filename;
  new_e->type.lineno = lhs->type.lineno;
  new_e->binary.lhs = lhs;
  new_e->binary.rhs = rhs;
  new_e->type.node_class = etree_binary;
  if (lhs->type.node_class == etree_value
      && rhs->type.node_class == etree_value
      && code != ALIGN_K
      && code != DATA_SEGMENT_ALIGN
      && code != DATA_SEGMENT_RELRO_END)
    exp_value_fold (new_e);
  return new_e;
}

etree_type *
exp_trinop (int code, etree_type *cond, etree_type *lhs, etree_type *rhs)
{
  etree_type *new_e = stat_alloc (MAX (sizeof (new_e->trinary),
				       sizeof (new_e->value)));
  new_e->type.node_code = code;
  new_e->type.filename = cond->type.filename;
  new_e->type.lineno = cond->type.lineno;
  new_e->trinary.lhs = lhs;
  new_e->trinary.cond = cond;
  new_e->trinary.rhs = rhs;
  new_e->type.node_class = etree_trinary;
  if (cond->type.node_class == etree_value
      && lhs->type.node_class == etree_value
      && rhs->type.node_class == etree_value)
    exp_value_fold (new_e);
  return new_e;
}

etree_type *
exp_unop (int code, etree_type *child)
{
  etree_type *new_e = stat_alloc (MAX (sizeof (new_e->unary),
				       sizeof (new_e->value)));
  new_e->unary.type.node_code = code;
  new_e->unary.type.filename = child->type.filename;
  new_e->unary.type.lineno = child->type.lineno;
  new_e->unary.child = child;
  new_e->unary.type.node_class = etree_unary;
  if (child->type.node_class == etree_value
      && code != ALIGN_K
      && code != ABSOLUTE
      && code != NEXT
      && code != DATA_SEGMENT_END)
    exp_value_fold (new_e);
  return new_e;
}

etree_type *
exp_nameop (int code, const char *name)
{
  etree_type *new_e = stat_alloc (sizeof (new_e->name));

  new_e->name.type.node_code = code;
  new_e->name.type.filename = ldlex_filename ();
  new_e->name.type.lineno = lineno;
  new_e->name.name = name;
  new_e->name.type.node_class = etree_name;
  return new_e;

}

static etree_type *
exp_assop (const char *dst,
	   etree_type *src,
	   enum node_tree_enum class,
	   bool hidden)
{
  etree_type *n;

  n = stat_alloc (sizeof (n->assign));
  n->assign.type.node_code = '=';
  n->assign.type.filename = src->type.filename;
  n->assign.type.lineno = src->type.lineno;
  n->assign.type.node_class = class;
  n->assign.src = src;
  n->assign.dst = dst;
  n->assign.hidden = hidden;
  return n;
}

/* Handle linker script assignments and HIDDEN.  */

etree_type *
exp_assign (const char *dst, etree_type *src, bool hidden)
{
  return exp_assop (dst, src, etree_assign, hidden);
}

/* Handle --defsym command-line option.  */

etree_type *
exp_defsym (const char *dst, etree_type *src)
{
  return exp_assop (dst, src, etree_assign, false);
}

/* Handle PROVIDE.  */

etree_type *
exp_provide (const char *dst, etree_type *src, bool hidden)
{
  return exp_assop (dst, src, etree_provide, hidden);
}

/* Handle ASSERT.  */

etree_type *
exp_assert (etree_type *exp, const char *message)
{
  etree_type *n;

  n = stat_alloc (sizeof (n->assert_s));
  n->assert_s.type.node_code = '!';
  n->assert_s.type.filename = exp->type.filename;
  n->assert_s.type.lineno = exp->type.lineno;
  n->assert_s.type.node_class = etree_assert;
  n->assert_s.child = exp;
  n->assert_s.message = message;
  return n;
}

void
exp_print_tree (etree_type *tree)
{
  bool function_like;

  if (config.map_file == NULL)
    config.map_file = stderr;

  if (tree == NULL)
    {
      minfo ("NULL TREE\n");
      return;
    }

  switch (tree->type.node_class)
    {
    case etree_value:
      minfo ("0x%v", tree->value.value);
      return;
    case etree_rel:
      if (tree->rel.section->owner != NULL)
	minfo ("%pB:", tree->rel.section->owner);
      minfo ("%s+0x%v", tree->rel.section->name, tree->rel.value);
      return;
    case etree_assign:
      fputs (tree->assign.dst, config.map_file);
      exp_print_token (tree->type.node_code, true);
      exp_print_tree (tree->assign.src);
      break;
    case etree_provide:
    case etree_provided:
      fprintf (config.map_file, "PROVIDE (%s = ", tree->assign.dst);
      exp_print_tree (tree->assign.src);
      fputc (')', config.map_file);
      break;
    case etree_binary:
      function_like = false;
      switch (tree->type.node_code)
	{
	case MAX_K:
	case MIN_K:
	case ALIGN_K:
	case DATA_SEGMENT_ALIGN:
	case DATA_SEGMENT_RELRO_END:
	  function_like = true;
	  break;
	case SEGMENT_START:
	  /* Special handling because arguments are in reverse order and
	     the segment name is quoted.  */
	  exp_print_token (tree->type.node_code, false);
	  fputs (" (\"", config.map_file);
	  exp_print_tree (tree->binary.rhs);
	  fputs ("\", ", config.map_file);
	  exp_print_tree (tree->binary.lhs);
	  fputc (')', config.map_file);
	  return;
	}
      if (function_like)
	{
	  exp_print_token (tree->type.node_code, false);
	  fputc (' ', config.map_file);
	}
      fputc ('(', config.map_file);
      exp_print_tree (tree->binary.lhs);
      if (function_like)
	fprintf (config.map_file, ", ");
      else
	exp_print_token (tree->type.node_code, true);
      exp_print_tree (tree->binary.rhs);
      fputc (')', config.map_file);
      break;
    case etree_trinary:
      exp_print_tree (tree->trinary.cond);
      fputc ('?', config.map_file);
      exp_print_tree (tree->trinary.lhs);
      fputc (':', config.map_file);
      exp_print_tree (tree->trinary.rhs);
      break;
    case etree_unary:
      exp_print_token (tree->unary.type.node_code, false);
      if (tree->unary.child)
	{
	  fprintf (config.map_file, " (");
	  exp_print_tree (tree->unary.child);
	  fputc (')', config.map_file);
	}
      break;

    case etree_assert:
      fprintf (config.map_file, "ASSERT (");
      exp_print_tree (tree->assert_s.child);
      fprintf (config.map_file, ", %s)", tree->assert_s.message);
      break;

    case etree_name:
      if (tree->type.node_code == NAME)
	fputs (tree->name.name, config.map_file);
      else
	{
	  exp_print_token (tree->type.node_code, false);
	  if (tree->name.name)
	    fprintf (config.map_file, " (%s)", tree->name.name);
	}
      break;
    default:
      FAIL ();
      break;
    }
}

bfd_vma
exp_get_vma (etree_type *tree, bfd_vma def, char *name)
{
  if (tree != NULL)
    {
      exp_fold_tree_no_dot (tree);
      if (expld.result.valid_p)
	return expld.result.value;
      else if (name != NULL && expld.phase != lang_mark_phase_enum)
	einfo (_("%F%P:%pS: nonconstant expression for %s\n"),
	       tree, name);
    }
  return def;
}

/* Return the smallest non-negative integer such that two raised to
   that power is at least as large as the vma evaluated at TREE, if
   TREE is a non-NULL expression that can be resolved.  If TREE is
   NULL or cannot be resolved, return -1.  */

int
exp_get_power (etree_type *tree, char *name)
{
  bfd_vma x = exp_get_vma (tree, -1, name);
  bfd_vma p2;
  int n;

  if (x == (bfd_vma) -1)
    return -1;

  for (n = 0, p2 = 1; p2 < x; ++n, p2 <<= 1)
    if (p2 == 0)
      break;

  return n;
}

fill_type *
exp_get_fill (etree_type *tree, fill_type *def, char *name)
{
  fill_type *fill;
  size_t len;
  unsigned int val;

  if (tree == NULL)
    return def;

  exp_fold_tree_no_dot (tree);
  if (!expld.result.valid_p)
    {
      if (name != NULL && expld.phase != lang_mark_phase_enum)
	einfo (_("%F%P:%pS: nonconstant expression for %s\n"),
	       tree, name);
      return def;
    }

  if (expld.result.str != NULL && (len = strlen (expld.result.str)) != 0)
    {
      unsigned char *dst;
      unsigned char *s;
      fill = (fill_type *) xmalloc ((len + 1) / 2 + sizeof (*fill) - 1);
      fill->size = (len + 1) / 2;
      dst = fill->data;
      s = (unsigned char *) expld.result.str;
      val = 0;
      do
	{
	  unsigned int digit;

	  digit = *s++ - '0';
	  if (digit > 9)
	    digit = (digit - 'A' + '0' + 10) & 0xf;
	  val <<= 4;
	  val += digit;
	  --len;
	  if ((len & 1) == 0)
	    {
	      *dst++ = val;
	      val = 0;
	    }
	}
      while (len != 0);
    }
  else
    {
      fill = (fill_type *) xmalloc (4 + sizeof (*fill) - 1);
      val = expld.result.value;
      fill->data[0] = (val >> 24) & 0xff;
      fill->data[1] = (val >> 16) & 0xff;
      fill->data[2] = (val >>  8) & 0xff;
      fill->data[3] = (val >>  0) & 0xff;
      fill->size = 4;
    }
  return fill;
}

bfd_vma
exp_get_abs_int (etree_type *tree, int def, char *name)
{
  if (tree != NULL)
    {
      exp_fold_tree_no_dot (tree);

      if (expld.result.valid_p)
	{
	  if (expld.result.section != NULL)
	    expld.result.value += expld.result.section->vma;
	  return expld.result.value;
	}
      else if (name != NULL && expld.phase != lang_mark_phase_enum)
	{
	  einfo (_("%F%P:%pS: nonconstant expression for %s\n"),
		 tree, name);
	}
    }
  return def;
}

static bfd_vma
align_n (bfd_vma value, bfd_vma align)
{
  if (align <= 1)
    return value;

  value = (value + align - 1) / align;
  return value * align;
}

void
ldexp_init (void)
{
  /* The value "13" is ad-hoc, somewhat related to the expected number of
     assignments in a linker script.  */
  if (!bfd_hash_table_init_n (&definedness_table,
			      definedness_newfunc,
			      sizeof (struct definedness_hash_entry),
			      13))
    einfo (_("%F%P: can not create hash table: %E\n"));
}

/* Convert absolute symbols defined by a script from "dot" (also
   SEGMENT_START or ORIGIN) outside of an output section statement,
   to section relative.  */

static bool
set_sym_sections (struct bfd_hash_entry *bh, void *inf ATTRIBUTE_UNUSED)
{
  struct definedness_hash_entry *def = (struct definedness_hash_entry *) bh;
  if (def->final_sec != bfd_abs_section_ptr)
    {
      struct bfd_link_hash_entry *h;
      h = bfd_link_hash_lookup (link_info.hash, bh->string,
				false, false, true);
      if (h != NULL
	  && h->type == bfd_link_hash_defined
	  && h->u.def.section == bfd_abs_section_ptr)
	{
	  h->u.def.value -= def->final_sec->vma;
	  h->u.def.section = def->final_sec;
	}
    }
  return true;
}

void
ldexp_finalize_syms (void)
{
  bfd_hash_traverse (&definedness_table, set_sym_sections, NULL);
}

/* Determine whether a symbol is going to remain absolute even after
   ldexp_finalize_syms() has run.  */

bool
ldexp_is_final_sym_absolute (const struct bfd_link_hash_entry *h)
{
  if (h->type == bfd_link_hash_defined
      && h->u.def.section == bfd_abs_section_ptr)
    {
      const struct definedness_hash_entry *def;

      if (!h->ldscript_def)
	return true;

      def = symbol_defined (h->root.string);
      if (def != NULL)
	return def->final_sec == bfd_abs_section_ptr;
    }

  return false;
}

void
ldexp_finish (void)
{
  bfd_hash_table_free (&definedness_table);
}
