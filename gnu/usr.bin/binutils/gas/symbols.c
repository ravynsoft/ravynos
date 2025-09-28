/* symbols.c -symbol table-
   Copyright (C) 1987-2023 Free Software Foundation, Inc.

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

/* #define DEBUG_SYMS / * to debug symbol list maintenance.  */

#include "as.h"
#include "safe-ctype.h"
#include "obstack.h"		/* For "symbols.h" */
#include "subsegs.h"
#include "write.h"

#include <limits.h>
#ifndef CHAR_BIT
#define CHAR_BIT 8
#endif

struct symbol_flags
{
  /* Whether the symbol is a local_symbol.  */
  unsigned int local_symbol : 1;

  /* Weather symbol has been written.  */
  unsigned int written : 1;

  /* Whether symbol value has been completely resolved (used during
     final pass over symbol table).  */
  unsigned int resolved : 1;

  /* Whether the symbol value is currently being resolved (used to
     detect loops in symbol dependencies).  */
  unsigned int resolving : 1;

  /* Whether the symbol value is used in a reloc.  This is used to
     ensure that symbols used in relocs are written out, even if they
     are local and would otherwise not be.  */
  unsigned int used_in_reloc : 1;

  /* Whether the symbol is used as an operand or in an expression.
     NOTE:  Not all the backends keep this information accurate;
     backends which use this bit are responsible for setting it when
     a symbol is used in backend routines.  */
  unsigned int used : 1;

  /* Whether the symbol can be re-defined.  */
  unsigned int volatil : 1;

  /* Whether the symbol is a forward reference, and whether such has
     been determined.  */
  unsigned int forward_ref : 1;
  unsigned int forward_resolved : 1;

  /* This is set if the symbol is defined in an MRI common section.
     We handle such sections as single common symbols, so symbols
     defined within them must be treated specially by the relocation
     routines.  */
  unsigned int mri_common : 1;

  /* This is set if the symbol is set with a .weakref directive.  */
  unsigned int weakrefr : 1;

  /* This is set when the symbol is referenced as part of a .weakref
     directive, but only if the symbol was not in the symbol table
     before.  It is cleared as soon as any direct reference to the
     symbol is present.  */
  unsigned int weakrefd : 1;

  /* Whether the symbol has been marked to be removed by a .symver
     directive.  */
  unsigned int removed : 1;

  /* Set when a warning about the symbol containing multibyte characters
     is generated.  */
  unsigned int multibyte_warned : 1;
};

/* A pointer in the symbol may point to either a complete symbol
   (struct symbol below) or to a local symbol (struct local_symbol
   defined here).  The symbol code can detect the case by examining
   the first field which is present in both structs.

   We do this because we ordinarily only need a small amount of
   information for a local symbol.  The symbol table takes up a lot of
   space, and storing less information for a local symbol can make a
   big difference in assembler memory usage when assembling a large
   file.  */

struct local_symbol
{
  /* Symbol flags.  Only local_symbol and resolved are relevant.  */
  struct symbol_flags flags;

  /* Hash value calculated from name.  */
  hashval_t hash;

  /* The symbol name.  */
  const char *name;

  /* The symbol frag.  */
  fragS *frag;

  /* The symbol section.  */
  asection *section;

  /* The value of the symbol.  */
  valueT value;
};

/* The information we keep for a symbol.  The symbol table holds
   pointers both to this and to local_symbol structures.  The first
   three fields must be identical to struct local_symbol, and the size
   should be the same as or smaller than struct local_symbol.
   Fields that don't fit go to an extension structure.  */

struct symbol
{
  /* Symbol flags.  */
  struct symbol_flags flags;

  /* Hash value calculated from name.  */
  hashval_t hash;

  /* The symbol name.  */
  const char *name;

  /* Pointer to the frag this symbol is attached to, if any.
     Otherwise, NULL.  */
  fragS *frag;

  /* BFD symbol */
  asymbol *bsym;

  /* Extra symbol fields that won't fit.  */
  struct xsymbol *x;
};

/* Extra fields to make up a full symbol.  */

struct xsymbol
{
  /* The value of the symbol.  */
  expressionS value;

  /* Forwards and backwards chain pointers.  */
  struct symbol *next;
  struct symbol *previous;

#ifdef OBJ_SYMFIELD_TYPE
  OBJ_SYMFIELD_TYPE obj;
#endif

#ifdef TC_SYMFIELD_TYPE
  TC_SYMFIELD_TYPE tc;
#endif
};

typedef union symbol_entry
{
  struct local_symbol lsy;
  struct symbol sy;
} symbol_entry_t;

/* Hash function for a symbol_entry.  */

static hashval_t
hash_symbol_entry (const void *e)
{
  symbol_entry_t *entry = (symbol_entry_t *) e;
  if (entry->sy.hash == 0)
    entry->sy.hash = htab_hash_string (entry->sy.name);

  return entry->sy.hash;
}

/* Equality function for a symbol_entry.  */

static int
eq_symbol_entry (const void *a, const void *b)
{
  const symbol_entry_t *ea = (const symbol_entry_t *) a;
  const symbol_entry_t *eb = (const symbol_entry_t *) b;

  return (ea->sy.hash == eb->sy.hash
	  && strcmp (ea->sy.name, eb->sy.name) == 0);
}

static void *
symbol_entry_find (htab_t table, const char *name)
{
  hashval_t hash = htab_hash_string (name);
  symbol_entry_t needle = { { { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
			      hash, name, 0, 0, 0 } };
  return htab_find_with_hash (table, &needle, hash);
}


/* This is non-zero if symbols are case sensitive, which is the
   default.  */
int symbols_case_sensitive = 1;

#ifndef WORKING_DOT_WORD
extern int new_broken_words;
#endif

static htab_t sy_hash;

/* Below are commented in "symbols.h".  */
symbolS *symbol_rootP;
symbolS *symbol_lastP;
symbolS abs_symbol;
struct xsymbol abs_symbol_x;
symbolS dot_symbol;
struct xsymbol dot_symbol_x;

#ifdef DEBUG_SYMS
#define debug_verify_symchain verify_symbol_chain
#else
#define debug_verify_symchain(root, last) ((void) 0)
#endif

#define DOLLAR_LABEL_CHAR	'\001'
#define LOCAL_LABEL_CHAR	'\002'

#ifndef TC_LABEL_IS_LOCAL
#define TC_LABEL_IS_LOCAL(name)	0
#endif

struct obstack notes;

/* Utility functions to allocate and duplicate memory on the notes
   obstack, each like the corresponding function without "notes_"
   prefix.  All of these exit on an allocation failure.  */

void *
notes_alloc (size_t size)
{
  return obstack_alloc (&notes, size);
}

void *
notes_calloc (size_t n, size_t size)
{
  size_t amt;
  void *ret;
  if (gas_mul_overflow (n, size, &amt))
    {
      obstack_alloc_failed_handler ();
      abort ();
    }
  ret = notes_alloc (amt);
  memset (ret, 0, amt);
  return ret;
}

void *
notes_memdup (const void *src, size_t copy_size, size_t alloc_size)
{
  void *ret = obstack_alloc (&notes, alloc_size);
  memcpy (ret, src, copy_size);
  if (alloc_size > copy_size)
    memset ((char *) ret + copy_size, 0, alloc_size - copy_size);
  return ret;
}

char *
notes_strdup (const char *str)
{
  size_t len = strlen (str) + 1;
  return notes_memdup (str, len, len);
}

char *
notes_concat (const char *first, ...)
{
  va_list args;
  const char *str;

  va_start (args, first);
  for (str = first; str; str = va_arg (args, const char *))
    {
      size_t size = strlen (str);
      obstack_grow (&notes, str, size);
    }
  va_end (args);
  obstack_1grow (&notes, 0);
  return obstack_finish (&notes);
}

/* Use with caution!  Frees PTR and all more recently allocated memory
   on the notes obstack.  */

void
notes_free (void *ptr)
{
  obstack_free (&notes, ptr);
}

#ifdef TE_PE
/* The name of an external symbol which is
   used to make weak PE symbol names unique.  */
const char * an_external_name;
#endif

/* Return a pointer to a new symbol.  Die if we can't make a new
   symbol.  Fill in the symbol's values.  Add symbol to end of symbol
   chain.

   This function should be called in the general case of creating a
   symbol.  However, if the output file symbol table has already been
   set, and you are certain that this symbol won't be wanted in the
   output file, you can call symbol_create.  */

symbolS *
symbol_new (const char *name, segT segment, fragS *frag, valueT valu)
{
  symbolS *symbolP = symbol_create (name, segment, frag, valu);

  /* Link to end of symbol chain.  */
  symbol_append (symbolP, symbol_lastP, &symbol_rootP, &symbol_lastP);

  return symbolP;
}

/* Save a symbol name on a permanent obstack, and convert it according
   to the object file format.  */

static const char *
save_symbol_name (const char *name)
{
  char *ret;

  gas_assert (name != NULL);
  ret = notes_strdup (name);

#ifdef tc_canonicalize_symbol_name
  ret = tc_canonicalize_symbol_name (ret);
#endif

  if (! symbols_case_sensitive)
    {
      char *s;

      for (s = ret; *s != '\0'; s++)
	*s = TOUPPER (*s);
    }

  return ret;
}

static void
symbol_init (symbolS *symbolP, const char *name, asection *sec,
	     fragS *frag, valueT valu)
{
  symbolP->frag = frag;
  symbolP->bsym = bfd_make_empty_symbol (stdoutput);
  if (symbolP->bsym == NULL)
    as_fatal ("bfd_make_empty_symbol: %s", bfd_errmsg (bfd_get_error ()));
  symbolP->bsym->name = name;
  symbolP->bsym->section = sec;

  if (multibyte_handling == multibyte_warn_syms
      && ! symbolP->flags.local_symbol
      && sec != undefined_section
      && ! symbolP->flags.multibyte_warned
      && scan_for_multibyte_characters ((const unsigned char *) name,
					(const unsigned char *) name + strlen (name),
					false /* Do not warn.  */))
    {
      as_warn (_("symbol '%s' contains multibyte characters"), name);
      symbolP->flags.multibyte_warned = 1;
    }

  S_SET_VALUE (symbolP, valu);
  if (sec == reg_section)
    symbolP->x->value.X_op = O_register;

  symbol_clear_list_pointers (symbolP);

  obj_symbol_new_hook (symbolP);

#ifdef tc_symbol_new_hook
  tc_symbol_new_hook (symbolP);
#endif
}

/* Create a symbol.  NAME is copied, the caller can destroy/modify.  */

symbolS *
symbol_create (const char *name, segT segment, fragS *frag, valueT valu)
{
  const char *preserved_copy_of_name;
  symbolS *symbolP;
  size_t size;

  preserved_copy_of_name = save_symbol_name (name);

  size = sizeof (symbolS) + sizeof (struct xsymbol);
  symbolP = notes_alloc (size);

  /* symbol must be born in some fixed state.  This seems as good as any.  */
  memset (symbolP, 0, size);
  symbolP->name = preserved_copy_of_name;
  symbolP->x = (struct xsymbol *) (symbolP + 1);

  symbol_init (symbolP, preserved_copy_of_name, segment, frag, valu);

  return symbolP;
}


/* Local symbol support.  If we can get away with it, we keep only a
   small amount of information for local symbols.  */

/* Used for statistics.  */

static unsigned long local_symbol_count;
static unsigned long local_symbol_conversion_count;

/* Create a local symbol and insert it into the local hash table.  */

struct local_symbol *
local_symbol_make (const char *name, segT section, fragS *frag, valueT val)
{
  const char *name_copy;
  struct local_symbol *ret;
  struct symbol_flags flags = { .local_symbol = 1, .resolved = 0 };

  ++local_symbol_count;

  name_copy = save_symbol_name (name);

  ret = notes_alloc (sizeof *ret);
  ret->flags = flags;
  ret->hash = 0;
  ret->name = name_copy;
  ret->frag = frag;
  ret->section = section;
  ret->value = val;

  htab_insert (sy_hash, ret, 1);

  return ret;
}

/* Convert a local symbol into a real symbol.  */

static symbolS *
local_symbol_convert (void *sym)
{
  symbol_entry_t *ent = (symbol_entry_t *) sym;
  struct xsymbol *xtra;
  valueT val;

  gas_assert (ent->lsy.flags.local_symbol);

  ++local_symbol_conversion_count;

  xtra = notes_alloc (sizeof (*xtra));
  memset (xtra, 0, sizeof (*xtra));
  val = ent->lsy.value;
  ent->sy.x = xtra;

  /* Local symbols are always either defined or used.  */
  ent->sy.flags.used = 1;
  ent->sy.flags.local_symbol = 0;

  symbol_init (&ent->sy, ent->lsy.name, ent->lsy.section, ent->lsy.frag, val);
  symbol_append (&ent->sy, symbol_lastP, &symbol_rootP, &symbol_lastP);

  return &ent->sy;
}

static void
define_sym_at_dot (symbolS *symbolP)
{
  symbolP->frag = frag_now;
  S_SET_VALUE (symbolP, (valueT) frag_now_fix ());
  S_SET_SEGMENT (symbolP, now_seg);
}

/* We have just seen "<name>:".
   Creates a struct symbol unless it already exists.

   Gripes if we are redefining a symbol incompatibly (and ignores it).  */

symbolS *
colon (/* Just seen "x:" - rattle symbols & frags.  */
       const char *sym_name	/* Symbol name, as a canonical string.  */
       /* We copy this string: OK to alter later.  */)
{
  symbolS *symbolP;	/* Symbol we are working with.  */

  /* Sun local labels go out of scope whenever a non-local symbol is
     defined.  */
  if (LOCAL_LABELS_DOLLAR
      && !bfd_is_local_label_name (stdoutput, sym_name))
    dollar_label_clear ();

#ifndef WORKING_DOT_WORD
  if (new_broken_words)
    {
      struct broken_word *a;
      int possible_bytes;
      fragS *frag_tmp;
      char *frag_opcode;

      if (now_seg == absolute_section)
	{
	  as_bad (_("cannot define symbol `%s' in absolute section"), sym_name);
	  return NULL;
	}

      possible_bytes = (md_short_jump_size
			+ new_broken_words * md_long_jump_size);

      frag_tmp = frag_now;
      frag_opcode = frag_var (rs_broken_word,
			      possible_bytes,
			      possible_bytes,
			      (relax_substateT) 0,
			      (symbolS *) broken_words,
			      (offsetT) 0,
			      NULL);

      /* We want to store the pointer to where to insert the jump
	 table in the fr_opcode of the rs_broken_word frag.  This
	 requires a little hackery.  */
      while (frag_tmp
	     && (frag_tmp->fr_type != rs_broken_word
		 || frag_tmp->fr_opcode))
	frag_tmp = frag_tmp->fr_next;
      know (frag_tmp);
      frag_tmp->fr_opcode = frag_opcode;
      new_broken_words = 0;

      for (a = broken_words; a && a->dispfrag == 0; a = a->next_broken_word)
	a->dispfrag = frag_tmp;
    }
#endif /* WORKING_DOT_WORD */

#ifdef obj_frob_colon
  obj_frob_colon (sym_name);
#endif

  if ((symbolP = symbol_find (sym_name)) != 0)
    {
      S_CLEAR_WEAKREFR (symbolP);
#ifdef RESOLVE_SYMBOL_REDEFINITION
      if (RESOLVE_SYMBOL_REDEFINITION (symbolP))
	return symbolP;
#endif
      /* Now check for undefined symbols.  */
      if (symbolP->flags.local_symbol)
	{
	  struct local_symbol *locsym = (struct local_symbol *) symbolP;

	  if (locsym->section != undefined_section
	      && (locsym->frag != frag_now
		  || locsym->section != now_seg
		  || locsym->value != frag_now_fix ()))
	    {
	      as_bad (_("symbol `%s' is already defined"), sym_name);
	      return symbolP;
	    }

	  locsym->section = now_seg;
	  locsym->frag = frag_now;
	  locsym->value = frag_now_fix ();
	}
      else if (!(S_IS_DEFINED (symbolP) || symbol_equated_p (symbolP))
	       || S_IS_COMMON (symbolP)
	       || S_IS_VOLATILE (symbolP))
	{
	  if (S_IS_VOLATILE (symbolP))
	    {
	      symbolP = symbol_clone (symbolP, 1);
	      S_SET_VALUE (symbolP, 0);
	      S_CLEAR_VOLATILE (symbolP);
	    }
	  if (S_GET_VALUE (symbolP) == 0)
	    {
	      define_sym_at_dot (symbolP);
#ifdef N_UNDF
	      know (N_UNDF == 0);
#endif /* if we have one, it better be zero.  */

	    }
	  else
	    {
	      /* There are still several cases to check:

		 A .comm/.lcomm symbol being redefined as initialized
		 data is OK

		 A .comm/.lcomm symbol being redefined with a larger
		 size is also OK

		 This only used to be allowed on VMS gas, but Sun cc
		 on the sparc also depends on it.  */

	      if (((!S_IS_DEBUG (symbolP)
		    && (!S_IS_DEFINED (symbolP) || S_IS_COMMON (symbolP))
		    && S_IS_EXTERNAL (symbolP))
		   || S_GET_SEGMENT (symbolP) == bss_section)
		  && (now_seg == data_section
		      || now_seg == bss_section
		      || now_seg == S_GET_SEGMENT (symbolP)))
		{
		  /* Select which of the 2 cases this is.  */
		  if (now_seg != data_section)
		    {
		      /* New .comm for prev .comm symbol.

			 If the new size is larger we just change its
			 value.  If the new size is smaller, we ignore
			 this symbol.  */
		      if (S_GET_VALUE (symbolP)
			  < ((unsigned) frag_now_fix ()))
			{
			  S_SET_VALUE (symbolP, (valueT) frag_now_fix ());
			}
		    }
		  else
		    {
		      /* It is a .comm/.lcomm being converted to initialized
			 data.  */
		      define_sym_at_dot (symbolP);
		    }
		}
	      else
		{
#if (!defined (OBJ_AOUT) && !defined (OBJ_MAYBE_AOUT))
		  static const char *od_buf = "";
#else
		  char od_buf[100];
		  od_buf[0] = '\0';
		  if (OUTPUT_FLAVOR == bfd_target_aout_flavour)
		    sprintf (od_buf, "%d.%d.",
			     S_GET_OTHER (symbolP),
			     S_GET_DESC (symbolP));
#endif
		  as_bad (_("symbol `%s' is already defined as \"%s\"/%s%ld"),
			    sym_name,
			    segment_name (S_GET_SEGMENT (symbolP)),
			    od_buf,
			    (long) S_GET_VALUE (symbolP));
		}
	    }			/* if the undefined symbol has no value  */
	}
      else
	{
	  /* Don't blow up if the definition is the same.  */
	  if (!(frag_now == symbolP->frag
		&& S_GET_VALUE (symbolP) == frag_now_fix ()
		&& S_GET_SEGMENT (symbolP) == now_seg))
	    {
	      as_bad (_("symbol `%s' is already defined"), sym_name);
	      symbolP = symbol_clone (symbolP, 0);
	      define_sym_at_dot (symbolP);
	    }
	}

    }
  else if (! flag_keep_locals && bfd_is_local_label_name (stdoutput, sym_name))
    {
      symbolP = (symbolS *) local_symbol_make (sym_name, now_seg, frag_now,
					       frag_now_fix ());
    }
  else
    {
      symbolP = symbol_new (sym_name, now_seg, frag_now, frag_now_fix ());

      symbol_table_insert (symbolP);
    }

  if (mri_common_symbol != NULL)
    {
      /* This symbol is actually being defined within an MRI common
	 section.  This requires special handling.  */
      if (symbolP->flags.local_symbol)
	symbolP = local_symbol_convert (symbolP);
      symbolP->x->value.X_op = O_symbol;
      symbolP->x->value.X_add_symbol = mri_common_symbol;
      symbolP->x->value.X_add_number = S_GET_VALUE (mri_common_symbol);
      symbolP->frag = &zero_address_frag;
      S_SET_SEGMENT (symbolP, expr_section);
      symbolP->flags.mri_common = 1;
    }

#ifdef tc_frob_label
  tc_frob_label (symbolP);
#endif
#ifdef obj_frob_label
  obj_frob_label (symbolP);
#endif

  return symbolP;
}

/* Die if we can't insert the symbol.  */

void
symbol_table_insert (symbolS *symbolP)
{
  know (symbolP);

  htab_insert (sy_hash, symbolP, 1);
}

/* If a symbol name does not exist, create it as undefined, and insert
   it into the symbol table.  Return a pointer to it.  */

symbolS *
symbol_find_or_make (const char *name)
{
  symbolS *symbolP;

  symbolP = symbol_find (name);

  if (symbolP == NULL)
    {
      if (! flag_keep_locals && bfd_is_local_label_name (stdoutput, name))
	{
	  symbolP = md_undefined_symbol ((char *) name);
	  if (symbolP != NULL)
	    return symbolP;

	  symbolP = (symbolS *) local_symbol_make (name, undefined_section,
						   &zero_address_frag, 0);
	  return symbolP;
	}

      symbolP = symbol_make (name);

      symbol_table_insert (symbolP);
    }				/* if symbol wasn't found */

  return (symbolP);
}

symbolS *
symbol_make (const char *name)
{
  symbolS *symbolP;

  /* Let the machine description default it, e.g. for register names.  */
  symbolP = md_undefined_symbol ((char *) name);

  if (!symbolP)
    symbolP = symbol_new (name, undefined_section, &zero_address_frag, 0);

  return (symbolP);
}

symbolS *
symbol_clone (symbolS *orgsymP, int replace)
{
  symbolS *newsymP;
  asymbol *bsymorg, *bsymnew;

  /* Make sure we never clone the dot special symbol.  */
  gas_assert (orgsymP != &dot_symbol);

  /* When cloning a local symbol it isn't absolutely necessary to
     convert the original, but converting makes the code much
     simpler to cover this unexpected case.  As of 2020-08-21
     symbol_clone won't be called on a local symbol.  */
  if (orgsymP->flags.local_symbol)
    orgsymP = local_symbol_convert (orgsymP);
  bsymorg = orgsymP->bsym;

  newsymP = notes_alloc (sizeof (symbolS) + sizeof (struct xsymbol));
  *newsymP = *orgsymP;
  newsymP->x = (struct xsymbol *) (newsymP + 1);
  *newsymP->x = *orgsymP->x;
  bsymnew = bfd_make_empty_symbol (bfd_asymbol_bfd (bsymorg));
  if (bsymnew == NULL)
    as_fatal ("bfd_make_empty_symbol: %s", bfd_errmsg (bfd_get_error ()));
  newsymP->bsym = bsymnew;
  bsymnew->name = bsymorg->name;
  bsymnew->flags = bsymorg->flags & ~BSF_SECTION_SYM;
  bsymnew->section = bsymorg->section;
  bfd_copy_private_symbol_data (bfd_asymbol_bfd (bsymorg), bsymorg,
				bfd_asymbol_bfd (bsymnew), bsymnew);

#ifdef obj_symbol_clone_hook
  obj_symbol_clone_hook (newsymP, orgsymP);
#endif

#ifdef tc_symbol_clone_hook
  tc_symbol_clone_hook (newsymP, orgsymP);
#endif

  if (replace)
    {
      if (symbol_rootP == orgsymP)
	symbol_rootP = newsymP;
      else if (orgsymP->x->previous)
	{
	  orgsymP->x->previous->x->next = newsymP;
	  orgsymP->x->previous = NULL;
	}
      if (symbol_lastP == orgsymP)
	symbol_lastP = newsymP;
      else if (orgsymP->x->next)
	orgsymP->x->next->x->previous = newsymP;

      /* Symbols that won't be output can't be external.  */
      S_CLEAR_EXTERNAL (orgsymP);
      orgsymP->x->previous = orgsymP->x->next = orgsymP;
      debug_verify_symchain (symbol_rootP, symbol_lastP);

      symbol_table_insert (newsymP);
    }
  else
    {
      /* Symbols that won't be output can't be external.  */
      S_CLEAR_EXTERNAL (newsymP);
      newsymP->x->previous = newsymP->x->next = newsymP;
    }

  return newsymP;
}

/* Referenced symbols, if they are forward references, need to be cloned
   (without replacing the original) so that the value of the referenced
   symbols at the point of use is saved by the clone.  */

#undef symbol_clone_if_forward_ref
symbolS *
symbol_clone_if_forward_ref (symbolS *symbolP, int is_forward)
{
  if (symbolP
      && !symbolP->flags.local_symbol
      && !symbolP->flags.forward_resolved)
    {
      symbolS *orig_add_symbol = symbolP->x->value.X_add_symbol;
      symbolS *orig_op_symbol = symbolP->x->value.X_op_symbol;
      symbolS *add_symbol = orig_add_symbol;
      symbolS *op_symbol = orig_op_symbol;

      if (symbolP->flags.forward_ref)
	is_forward = 1;

      if (is_forward)
	{
	  /* assign_symbol() clones volatile symbols; pre-existing expressions
	     hold references to the original instance, but want the current
	     value.  Just repeat the lookup.  */
	  if (add_symbol && S_IS_VOLATILE (add_symbol))
	    add_symbol = symbol_find_exact (S_GET_NAME (add_symbol));
	  if (op_symbol && S_IS_VOLATILE (op_symbol))
	    op_symbol = symbol_find_exact (S_GET_NAME (op_symbol));
	}

      /* Re-using resolving here, as this routine cannot get called from
	 symbol resolution code.  */
      if ((symbolP->bsym->section == expr_section
	   || symbolP->flags.forward_ref)
	  && !symbolP->flags.resolving)
	{
	  symbolP->flags.resolving = 1;
	  add_symbol = symbol_clone_if_forward_ref (add_symbol, is_forward);
	  op_symbol = symbol_clone_if_forward_ref (op_symbol, is_forward);
	  symbolP->flags.resolving = 0;
	}

      if (symbolP->flags.forward_ref
	  || add_symbol != orig_add_symbol
	  || op_symbol != orig_op_symbol)
	{
	  if (symbolP != &dot_symbol)
	    {
	      symbolP = symbol_clone (symbolP, 0);
	      symbolP->flags.resolving = 0;
	    }
	  else
	    {
	      symbolP = symbol_temp_new_now ();
#ifdef tc_new_dot_label
	      tc_new_dot_label (symbolP);
#endif
	    }
	}

      symbolP->x->value.X_add_symbol = add_symbol;
      symbolP->x->value.X_op_symbol = op_symbol;
      symbolP->flags.forward_resolved = 1;
    }

  return symbolP;
}

symbolS *
symbol_temp_new (segT seg, fragS *frag, valueT ofs)
{
  return symbol_new (FAKE_LABEL_NAME, seg, frag, ofs);
}

symbolS *
symbol_temp_new_now (void)
{
  return symbol_temp_new (now_seg, frag_now, frag_now_fix ());
}

symbolS *
symbol_temp_new_now_octets (void)
{
  return symbol_temp_new (now_seg, frag_now, frag_now_fix_octets ());
}

symbolS *
symbol_temp_make (void)
{
  return symbol_make (FAKE_LABEL_NAME);
}

/* Implement symbol table lookup.
   In:	A symbol's name as a string: '\0' can't be part of a symbol name.
   Out:	NULL if the name was not in the symbol table, else the address
   of a struct symbol associated with that name.  */

symbolS *
symbol_find_exact (const char *name)
{
  return symbol_find_exact_noref (name, 0);
}

symbolS *
symbol_find_exact_noref (const char *name, int noref)
{
  symbolS *sym = symbol_entry_find (sy_hash, name);

  /* Any references to the symbol, except for the reference in
     .weakref, must clear this flag, such that the symbol does not
     turn into a weak symbol.  Note that we don't have to handle the
     local_symbol case, since a weakrefd is always promoted out of the
     local_symbol table when it is turned into a weak symbol.  */
  if (sym && ! noref)
    S_CLEAR_WEAKREFD (sym);

  return sym;
}

symbolS *
symbol_find (const char *name)
{
  return symbol_find_noref (name, 0);
}

symbolS *
symbol_find_noref (const char *name, int noref)
{
  symbolS * result;
  char * copy = NULL;

#ifdef tc_canonicalize_symbol_name
  {
    copy = xstrdup (name);
    name = tc_canonicalize_symbol_name (copy);
  }
#endif

  if (! symbols_case_sensitive)
    {
      const char *orig;
      char *copy2 = NULL;
      unsigned char c;

      orig = name;
      if (copy != NULL)
	copy2 = copy;
      name = copy = XNEWVEC (char, strlen (name) + 1);

      while ((c = *orig++) != '\0')
	*copy++ = TOUPPER (c);
      *copy = '\0';

      free (copy2);
      copy = (char *) name;
    }

  result = symbol_find_exact_noref (name, noref);
  free (copy);
  return result;
}

/* Once upon a time, symbols were kept in a singly linked list.  At
   least coff needs to be able to rearrange them from time to time, for
   which a doubly linked list is much more convenient.  Loic did these
   as macros which seemed dangerous to me so they're now functions.
   xoxorich.  */

/* Link symbol ADDME after symbol TARGET in the chain.  */

void
symbol_append (symbolS *addme, symbolS *target,
	       symbolS **rootPP, symbolS **lastPP)
{
  extern int symbol_table_frozen;
  if (symbol_table_frozen)
    abort ();
  if (addme->flags.local_symbol)
    abort ();
  if (target != NULL && target->flags.local_symbol)
    abort ();

  if (target == NULL)
    {
      know (*rootPP == NULL);
      know (*lastPP == NULL);
      addme->x->next = NULL;
      addme->x->previous = NULL;
      *rootPP = addme;
      *lastPP = addme;
      return;
    }				/* if the list is empty  */

  if (target->x->next != NULL)
    {
      target->x->next->x->previous = addme;
    }
  else
    {
      know (*lastPP == target);
      *lastPP = addme;
    }				/* if we have a next  */

  addme->x->next = target->x->next;
  target->x->next = addme;
  addme->x->previous = target;

  debug_verify_symchain (symbol_rootP, symbol_lastP);
}

/* Set the chain pointers of SYMBOL to null.  */

void
symbol_clear_list_pointers (symbolS *symbolP)
{
  if (symbolP->flags.local_symbol)
    abort ();
  symbolP->x->next = NULL;
  symbolP->x->previous = NULL;
}

/* Remove SYMBOLP from the list.  */

void
symbol_remove (symbolS *symbolP, symbolS **rootPP, symbolS **lastPP)
{
  if (symbolP->flags.local_symbol)
    abort ();

  if (symbolP == *rootPP)
    {
      *rootPP = symbolP->x->next;
    }				/* if it was the root  */

  if (symbolP == *lastPP)
    {
      *lastPP = symbolP->x->previous;
    }				/* if it was the tail  */

  if (symbolP->x->next != NULL)
    {
      symbolP->x->next->x->previous = symbolP->x->previous;
    }				/* if not last  */

  if (symbolP->x->previous != NULL)
    {
      symbolP->x->previous->x->next = symbolP->x->next;
    }				/* if not first  */

  debug_verify_symchain (*rootPP, *lastPP);
}

/* Link symbol ADDME before symbol TARGET in the chain.  */

void
symbol_insert (symbolS *addme, symbolS *target,
	       symbolS **rootPP, symbolS **lastPP ATTRIBUTE_UNUSED)
{
  extern int symbol_table_frozen;
  if (symbol_table_frozen)
    abort ();
  if (addme->flags.local_symbol)
    abort ();
  if (target->flags.local_symbol)
    abort ();

  if (target->x->previous != NULL)
    {
      target->x->previous->x->next = addme;
    }
  else
    {
      know (*rootPP == target);
      *rootPP = addme;
    }				/* if not first  */

  addme->x->previous = target->x->previous;
  target->x->previous = addme;
  addme->x->next = target;

  debug_verify_symchain (*rootPP, *lastPP);
}

void
verify_symbol_chain (symbolS *rootP, symbolS *lastP)
{
  symbolS *symbolP = rootP;

  if (symbolP == NULL)
    return;

  for (; symbol_next (symbolP) != NULL; symbolP = symbol_next (symbolP))
    {
      gas_assert (symbolP->bsym != NULL);
      gas_assert (symbolP->flags.local_symbol == 0);
      gas_assert (symbolP->x->next->x->previous == symbolP);
    }

  gas_assert (lastP == symbolP);
}

int
symbol_on_chain (symbolS *s, symbolS *rootPP, symbolS *lastPP)
{
  return (!s->flags.local_symbol
	  && ((s->x->next != s
	       && s->x->next != NULL
	       && s->x->next->x->previous == s)
	      || s == lastPP)
	  && ((s->x->previous != s
	       && s->x->previous != NULL
	       && s->x->previous->x->next == s)
	      || s == rootPP));
}

#ifdef OBJ_COMPLEX_RELC

static int
use_complex_relocs_for (symbolS * symp)
{
  switch (symp->x->value.X_op)
    {
    case O_constant:
      return 0;

    case O_multiply:
    case O_divide:
    case O_modulus:
    case O_left_shift:
    case O_right_shift:
    case O_bit_inclusive_or:
    case O_bit_or_not:
    case O_bit_exclusive_or:
    case O_bit_and:
    case O_add:
    case O_subtract:
    case O_eq:
    case O_ne:
    case O_lt:
    case O_le:
    case O_ge:
    case O_gt:
    case O_logical_and:
    case O_logical_or:
      if ((S_IS_COMMON (symp->x->value.X_op_symbol)
	   || S_IS_LOCAL (symp->x->value.X_op_symbol))
	  && S_IS_DEFINED (symp->x->value.X_op_symbol)
	  && S_GET_SEGMENT (symp->x->value.X_op_symbol) != expr_section)
	{
	case O_symbol:
	case O_symbol_rva:
	case O_uminus:
	case O_bit_not:
	case O_logical_not:
	  if ((S_IS_COMMON (symp->x->value.X_add_symbol)
	       || S_IS_LOCAL (symp->x->value.X_add_symbol))
	      && S_IS_DEFINED (symp->x->value.X_add_symbol)
	      && S_GET_SEGMENT (symp->x->value.X_add_symbol) != expr_section)
	    return 0;
	}
      break;

    default:
      break;
    }
  return 1;
}
#endif

static void
report_op_error (symbolS *symp, symbolS *left, operatorT op, symbolS *right)
{
  const char *file;
  unsigned int line;
  segT seg_left = left ? S_GET_SEGMENT (left) : 0;
  segT seg_right = S_GET_SEGMENT (right);
  const char *opname;

  switch (op)
    {
    default:
      abort ();
      return;

    case O_uminus:		opname = "-"; break;
    case O_bit_not:		opname = "~"; break;
    case O_logical_not:		opname = "!"; break;
    case O_multiply:		opname = "*"; break;
    case O_divide:		opname = "/"; break;
    case O_modulus:		opname = "%"; break;
    case O_left_shift:		opname = "<<"; break;
    case O_right_shift:		opname = ">>"; break;
    case O_bit_inclusive_or:	opname = "|"; break;
    case O_bit_or_not:		opname = "|~"; break;
    case O_bit_exclusive_or:	opname = "^"; break;
    case O_bit_and:		opname = "&"; break;
    case O_add:			opname = "+"; break;
    case O_subtract:		opname = "-"; break;
    case O_eq:			opname = "=="; break;
    case O_ne:			opname = "!="; break;
    case O_lt:			opname = "<"; break;
    case O_le:			opname = "<="; break;
    case O_ge:			opname = ">="; break;
    case O_gt:			opname = ">"; break;
    case O_logical_and:		opname = "&&"; break;
    case O_logical_or:		opname = "||"; break;
    }

  if (expr_symbol_where (symp, &file, &line))
    {
      if (left)
	as_bad_where (file, line,
		      _("invalid operands (%s and %s sections) for `%s'"),
		      seg_left->name, seg_right->name, opname);
      else
	as_bad_where (file, line,
		      _("invalid operand (%s section) for `%s'"),
		      seg_right->name, opname);
    }
  else
    {
      const char *sname = S_GET_NAME (symp);

      if (left)
	as_bad (_("invalid operands (%s and %s sections) for `%s' when setting `%s'"),
		seg_left->name, seg_right->name, opname, sname);
      else
	as_bad (_("invalid operand (%s section) for `%s' when setting `%s'"),
		seg_right->name, opname, sname);
    }
}

/* Resolve the value of a symbol.  This is called during the final
   pass over the symbol table to resolve any symbols with complex
   values.  */

valueT
resolve_symbol_value (symbolS *symp)
{
  int resolved;
  valueT final_val;
  segT final_seg;

  if (symp->flags.local_symbol)
    {
      struct local_symbol *locsym = (struct local_symbol *) symp;

      final_val = locsym->value;
      if (locsym->flags.resolved)
	return final_val;

      /* Symbols whose section has SEC_ELF_OCTETS set,
	 resolve to octets instead of target bytes. */
      if (locsym->section->flags & SEC_OCTETS)
	final_val += locsym->frag->fr_address;
      else
	final_val += locsym->frag->fr_address / OCTETS_PER_BYTE;

      if (finalize_syms)
	{
	  locsym->value = final_val;
	  locsym->flags.resolved = 1;
	}

      return final_val;
    }

  if (symp->flags.resolved)
    {
      final_val = 0;
      while (symp->x->value.X_op == O_symbol)
	{
	  final_val += symp->x->value.X_add_number;
	  symp = symp->x->value.X_add_symbol;
	  if (symp->flags.local_symbol)
	    {
	      struct local_symbol *locsym = (struct local_symbol *) symp;
	      final_val += locsym->value;
	      return final_val;
	    }
	  if (!symp->flags.resolved)
	    return 0;
	}
      if (symp->x->value.X_op == O_constant)
	final_val += symp->x->value.X_add_number;
      else
	final_val = 0;
      return final_val;
    }

  resolved = 0;
  final_seg = S_GET_SEGMENT (symp);

  if (symp->flags.resolving)
    {
      if (finalize_syms)
	as_bad (_("symbol definition loop encountered at `%s'"),
		S_GET_NAME (symp));
      final_val = 0;
      resolved = 1;
    }
#ifdef OBJ_COMPLEX_RELC
  else if (final_seg == expr_section
	   && use_complex_relocs_for (symp))
    {
      symbolS * relc_symbol = NULL;
      char * relc_symbol_name = NULL;

      relc_symbol_name = symbol_relc_make_expr (& symp->x->value);

      /* For debugging, print out conversion input & output.  */
#ifdef DEBUG_SYMS
      print_expr (& symp->x->value);
      if (relc_symbol_name)
	fprintf (stderr, "-> relc symbol: %s\n", relc_symbol_name);
#endif

      if (relc_symbol_name != NULL)
	relc_symbol = symbol_new (relc_symbol_name, undefined_section,
				  &zero_address_frag, 0);

      if (relc_symbol == NULL)
	{
	  as_bad (_("cannot convert expression symbol %s to complex relocation"),
		  S_GET_NAME (symp));
	  resolved = 0;
	}
      else
	{
	  symbol_table_insert (relc_symbol);

	  /* S_CLEAR_EXTERNAL (relc_symbol); */
	  if (symp->bsym->flags & BSF_SRELC)
	    relc_symbol->bsym->flags |= BSF_SRELC;
	  else
	    relc_symbol->bsym->flags |= BSF_RELC;
	  /* symp->bsym->flags |= BSF_RELC; */
	  copy_symbol_attributes (symp, relc_symbol);
	  symp->x->value.X_op = O_symbol;
	  symp->x->value.X_add_symbol = relc_symbol;
	  symp->x->value.X_add_number = 0;
	  resolved = 1;
	}

      final_val = 0;
      final_seg = undefined_section;
      goto exit_dont_set_value;
    }
#endif
  else
    {
      symbolS *add_symbol, *op_symbol;
      offsetT left, right;
      segT seg_left, seg_right;
      operatorT op;
      int move_seg_ok;

      symp->flags.resolving = 1;

      /* Help out with CSE.  */
      add_symbol = symp->x->value.X_add_symbol;
      op_symbol = symp->x->value.X_op_symbol;
      final_val = symp->x->value.X_add_number;
      op = symp->x->value.X_op;

      switch (op)
	{
	default:
	  BAD_CASE (op);
	  break;

	case O_md1:
	case O_md2:
	case O_md3:
	case O_md4:
	case O_md5:
	case O_md6:
	case O_md7:
	case O_md8:
	case O_md9:
	case O_md10:
	case O_md11:
	case O_md12:
	case O_md13:
	case O_md14:
	case O_md15:
	case O_md16:
	case O_md17:
	case O_md18:
	case O_md19:
	case O_md20:
	case O_md21:
	case O_md22:
	case O_md23:
	case O_md24:
	case O_md25:
	case O_md26:
	case O_md27:
	case O_md28:
	case O_md29:
	case O_md30:
	case O_md31:
	case O_md32:
#ifdef md_resolve_symbol
	  resolved = md_resolve_symbol (symp, &final_val, &final_seg);
	  if (resolved)
	    break;
#endif
	  goto exit_dont_set_value;

	case O_absent:
	  final_val = 0;
	  /* Fall through.  */

	case O_constant:
	  /* Symbols whose section has SEC_ELF_OCTETS set,
	     resolve to octets instead of target bytes. */
	  if (symp->bsym->section->flags & SEC_OCTETS)
	    final_val += symp->frag->fr_address;
	  else
	    final_val += symp->frag->fr_address / OCTETS_PER_BYTE;
	  if (final_seg == expr_section)
	    final_seg = absolute_section;
	  /* Fall through.  */

	case O_register:
	  resolved = 1;
	  break;

	case O_symbol:
	case O_symbol_rva:
	case O_secidx:
	  left = resolve_symbol_value (add_symbol);
	  seg_left = S_GET_SEGMENT (add_symbol);
	  if (finalize_syms)
	    symp->x->value.X_op_symbol = NULL;

	do_symbol:
	  if (S_IS_WEAKREFR (symp))
	    {
	      gas_assert (final_val == 0);
	      if (S_IS_WEAKREFR (add_symbol))
		{
		  gas_assert (add_symbol->x->value.X_op == O_symbol
			      && add_symbol->x->value.X_add_number == 0);
		  add_symbol = add_symbol->x->value.X_add_symbol;
		  gas_assert (! S_IS_WEAKREFR (add_symbol));
		  symp->x->value.X_add_symbol = add_symbol;
		}
	    }

	  if (symp->flags.mri_common)
	    {
	      /* This is a symbol inside an MRI common section.  The
		 relocation routines are going to handle it specially.
		 Don't change the value.  */
	      resolved = symbol_resolved_p (add_symbol);
	      break;
	    }

	  /* Don't leave symbol loops.  */
	  if (finalize_syms
	      && !add_symbol->flags.local_symbol
	      && add_symbol->flags.resolving)
	    break;

	  if (finalize_syms && final_val == 0
#ifdef OBJ_XCOFF
	      /* Avoid changing symp's "within" when dealing with
		 AIX debug symbols. For some storage classes, "within"
	         have a special meaning.
		 C_DWARF should behave like on Linux, thus this check
		 isn't done to be closer.  */
	      && ((symbol_get_bfdsym (symp)->flags & BSF_DEBUGGING) == 0
		  || (S_GET_STORAGE_CLASS (symp) == C_DWARF))
#endif
	      )
	    {
	      if (add_symbol->flags.local_symbol)
		add_symbol = local_symbol_convert (add_symbol);
	      copy_symbol_attributes (symp, add_symbol);
	    }

	  /* If we have equated this symbol to an undefined or common
	     symbol, keep X_op set to O_symbol, and don't change
	     X_add_number.  This permits the routine which writes out
	     relocation to detect this case, and convert the
	     relocation to be against the symbol to which this symbol
	     is equated.  */
	  if (seg_left == undefined_section
	      || bfd_is_com_section (seg_left)
#if defined (OBJ_COFF) && defined (TE_PE)
	      || S_IS_WEAK (add_symbol)
#endif
	      || (finalize_syms
		  && ((final_seg == expr_section
		       && seg_left != expr_section
		       && seg_left != absolute_section)
		      || symbol_shadow_p (symp))))
	    {
	      if (finalize_syms)
		{
		  symp->x->value.X_op = O_symbol;
		  symp->x->value.X_add_symbol = add_symbol;
		  symp->x->value.X_add_number = final_val;
		  /* Use X_op_symbol as a flag.  */
		  symp->x->value.X_op_symbol = add_symbol;
		}
	      final_seg = seg_left;
	      final_val += symp->frag->fr_address + left;
	      resolved = symbol_resolved_p (add_symbol);
	      symp->flags.resolving = 0;

	      if (op == O_secidx && seg_left != undefined_section)
		{
		  final_val = 0;
		  break;
		}

	      goto exit_dont_set_value;
	    }
	  else
	    {
	      final_val += symp->frag->fr_address + left;
	      if (final_seg == expr_section || final_seg == undefined_section)
		final_seg = seg_left;
	    }

	  resolved = symbol_resolved_p (add_symbol);
	  if (S_IS_WEAKREFR (symp))
	    {
	      symp->flags.resolving = 0;
	      goto exit_dont_set_value;
	    }
	  break;

	case O_uminus:
	case O_bit_not:
	case O_logical_not:
	  left = resolve_symbol_value (add_symbol);
	  seg_left = S_GET_SEGMENT (add_symbol);

	  /* By reducing these to the relevant dyadic operator, we get
		!S -> S == 0	permitted on anything,
		-S -> 0 - S	only permitted on absolute
		~S -> S ^ ~0	only permitted on absolute  */
	  if (op != O_logical_not && seg_left != absolute_section
	      && finalize_syms)
	    report_op_error (symp, NULL, op, add_symbol);

	  if (final_seg == expr_section || final_seg == undefined_section)
	    final_seg = absolute_section;

	  if (op == O_uminus)
	    left = -left;
	  else if (op == O_logical_not)
	    left = !left;
	  else
	    left = ~left;

	  final_val += left + symp->frag->fr_address;

	  resolved = symbol_resolved_p (add_symbol);
	  break;

	case O_multiply:
	case O_divide:
	case O_modulus:
	case O_left_shift:
	case O_right_shift:
	case O_bit_inclusive_or:
	case O_bit_or_not:
	case O_bit_exclusive_or:
	case O_bit_and:
	case O_add:
	case O_subtract:
	case O_eq:
	case O_ne:
	case O_lt:
	case O_le:
	case O_ge:
	case O_gt:
	case O_logical_and:
	case O_logical_or:
	  left = resolve_symbol_value (add_symbol);
	  right = resolve_symbol_value (op_symbol);
	  seg_left = S_GET_SEGMENT (add_symbol);
	  seg_right = S_GET_SEGMENT (op_symbol);

	  /* Simplify addition or subtraction of a constant by folding the
	     constant into X_add_number.  */
	  if (op == O_add)
	    {
	      if (seg_right == absolute_section)
		{
		  final_val += right;
		  goto do_symbol;
		}
	      else if (seg_left == absolute_section)
		{
		  final_val += left;
		  add_symbol = op_symbol;
		  left = right;
		  seg_left = seg_right;
		  goto do_symbol;
		}
	    }
	  else if (op == O_subtract)
	    {
	      if (seg_right == absolute_section)
		{
		  final_val -= right;
		  goto do_symbol;
		}
	    }

	  move_seg_ok = 1;
	  /* Equality and non-equality tests are permitted on anything.
	     Subtraction, and other comparison operators are permitted if
	     both operands are in the same section.  Otherwise, both
	     operands must be absolute.  We already handled the case of
	     addition or subtraction of a constant above.  This will
	     probably need to be changed for an object file format which
	     supports arbitrary expressions.  */
	  if (!(seg_left == absolute_section
		&& seg_right == absolute_section)
	      && !(op == O_eq || op == O_ne)
	      && !((op == O_subtract
		    || op == O_lt || op == O_le || op == O_ge || op == O_gt)
		   && seg_left == seg_right
		   && (seg_left != undefined_section
		       || add_symbol == op_symbol)))
	    {
	      /* Don't emit messages unless we're finalizing the symbol value,
		 otherwise we may get the same message multiple times.  */
	      if (finalize_syms)
		report_op_error (symp, add_symbol, op, op_symbol);
	      /* However do not move the symbol into the absolute section
		 if it cannot currently be resolved - this would confuse
		 other parts of the assembler into believing that the
		 expression had been evaluated to zero.  */
	      else
		move_seg_ok = 0;
	    }

	  if (move_seg_ok
	      && (final_seg == expr_section || final_seg == undefined_section))
	    final_seg = absolute_section;

	  /* Check for division by zero.  */
	  if ((op == O_divide || op == O_modulus) && right == 0)
	    {
	      /* If seg_right is not absolute_section, then we've
		 already issued a warning about using a bad symbol.  */
	      if (seg_right == absolute_section && finalize_syms)
		{
		  const char *file;
		  unsigned int line;

		  if (expr_symbol_where (symp, &file, &line))
		    as_bad_where (file, line, _("division by zero"));
		  else
		    as_bad (_("division by zero when setting `%s'"),
			    S_GET_NAME (symp));
		}

	      right = 1;
	    }
	  if ((op == O_left_shift || op == O_right_shift)
	      && (valueT) right >= sizeof (valueT) * CHAR_BIT)
	    {
	      as_warn_value_out_of_range (_("shift count"), right, 0,
					  sizeof (valueT) * CHAR_BIT - 1,
					  NULL, 0);
	      left = right = 0;
	    }

	  switch (symp->x->value.X_op)
	    {
	    case O_multiply:		left *= right; break;
	    case O_divide:		left /= right; break;
	    case O_modulus:		left %= right; break;
	    case O_left_shift:
	      left = (valueT) left << (valueT) right; break;
	    case O_right_shift:
	      left = (valueT) left >> (valueT) right; break;
	    case O_bit_inclusive_or:	left |= right; break;
	    case O_bit_or_not:		left |= ~right; break;
	    case O_bit_exclusive_or:	left ^= right; break;
	    case O_bit_and:		left &= right; break;
	    case O_add:			left += right; break;
	    case O_subtract:		left -= right; break;
	    case O_eq:
	    case O_ne:
	      left = (left == right && seg_left == seg_right
		      && (seg_left != undefined_section
			  || add_symbol == op_symbol)
		      ? ~ (offsetT) 0 : 0);
	      if (symp->x->value.X_op == O_ne)
		left = ~left;
	      break;
	    case O_lt:	left = left <  right ? ~ (offsetT) 0 : 0; break;
	    case O_le:	left = left <= right ? ~ (offsetT) 0 : 0; break;
	    case O_ge:	left = left >= right ? ~ (offsetT) 0 : 0; break;
	    case O_gt:	left = left >  right ? ~ (offsetT) 0 : 0; break;
	    case O_logical_and:	left = left && right; break;
	    case O_logical_or:	left = left || right; break;

	    case O_illegal:
	    case O_absent:
	    case O_constant:
	      /* See PR 20895 for a reproducer.  */
	      as_bad (_("Invalid operation on symbol"));
	      goto exit_dont_set_value;
	      
	    default:
	      abort ();
	    }

	  final_val += symp->frag->fr_address + left;
	  if (final_seg == expr_section || final_seg == undefined_section)
	    {
	      if (seg_left == undefined_section
		  || seg_right == undefined_section)
		final_seg = undefined_section;
	      else if (seg_left == absolute_section)
		final_seg = seg_right;
	      else
		final_seg = seg_left;
	    }
	  resolved = (symbol_resolved_p (add_symbol)
		      && symbol_resolved_p (op_symbol));
	  break;

	case O_big:
	case O_illegal:
	  /* Give an error (below) if not in expr_section.  We don't
	     want to worry about expr_section symbols, because they
	     are fictional (they are created as part of expression
	     resolution), and any problems may not actually mean
	     anything.  */
	  break;
	}

      symp->flags.resolving = 0;
    }

  if (finalize_syms)
    S_SET_VALUE (symp, final_val);

 exit_dont_set_value:
  /* Always set the segment, even if not finalizing the value.
     The segment is used to determine whether a symbol is defined.  */
    S_SET_SEGMENT (symp, final_seg);

  /* Don't worry if we can't resolve an expr_section symbol.  */
  if (finalize_syms)
    {
      if (resolved)
	symp->flags.resolved = 1;
      else if (S_GET_SEGMENT (symp) != expr_section)
	{
	  as_bad (_("can't resolve value for symbol `%s'"),
		  S_GET_NAME (symp));
	  symp->flags.resolved = 1;
	}
    }

  return final_val;
}

/* A static function passed to hash_traverse.  */

static int
resolve_local_symbol (void **slot, void *arg ATTRIBUTE_UNUSED)
{
  symbol_entry_t *entry = *((symbol_entry_t **) slot);
  if (entry->sy.flags.local_symbol)
    resolve_symbol_value (&entry->sy);

  return 1;
}

/* Resolve all local symbols.  */

void
resolve_local_symbol_values (void)
{
  htab_traverse_noresize (sy_hash, resolve_local_symbol, NULL);
}

/* Obtain the current value of a symbol without changing any
   sub-expressions used.  */

int
snapshot_symbol (symbolS **symbolPP, valueT *valueP, segT *segP, fragS **fragPP)
{
  symbolS *symbolP = *symbolPP;

  if (symbolP->flags.local_symbol)
    {
      struct local_symbol *locsym = (struct local_symbol *) symbolP;

      *valueP = locsym->value;
      *segP = locsym->section;
      *fragPP = locsym->frag;
    }
  else
    {
      expressionS exp = symbolP->x->value;

      if (!symbolP->flags.resolved && exp.X_op != O_illegal)
	{
	  int resolved;

	  if (symbolP->flags.resolving)
	    return 0;
	  symbolP->flags.resolving = 1;
	  resolved = resolve_expression (&exp);
	  symbolP->flags.resolving = 0;
	  if (!resolved)
	    return 0;

	  switch (exp.X_op)
	    {
	    case O_constant:
	    case O_register:
	      if (!symbol_equated_p (symbolP))
		break;
	      /* Fallthru.  */
	    case O_symbol:
	    case O_symbol_rva:
	      symbolP = exp.X_add_symbol;
	      break;
	    default:
	      return 0;
	    }
	}

      *symbolPP = symbolP;

      /* A bogus input file can result in resolve_expression()
	 generating a local symbol, so we have to check again.  */
      if (symbolP->flags.local_symbol)
	{
	  struct local_symbol *locsym = (struct local_symbol *) symbolP;

	  *valueP = locsym->value;
	  *segP = locsym->section;
	  *fragPP = locsym->frag;
	}
      else
	{
	  *valueP = exp.X_add_number;
	  *segP = symbolP->bsym->section;
	  *fragPP = symbolP->frag;
	}

      if (*segP == expr_section)
	switch (exp.X_op)
	  {
	  case O_constant: *segP = absolute_section; break;
	  case O_register: *segP = reg_section; break;
	  default: break;
	  }
    }

  return 1;
}

/* Dollar labels look like a number followed by a dollar sign.  Eg, "42$".
   They are *really* local.  That is, they go out of scope whenever we see a
   label that isn't local.  Also, like fb labels, there can be multiple
   instances of a dollar label.  Therefor, we name encode each instance with
   the instance number, keep a list of defined symbols separate from the real
   symbol table, and we treat these buggers as a sparse array.  */

typedef unsigned int dollar_ent;
static dollar_ent *dollar_labels;
static dollar_ent *dollar_label_instances;
static char *dollar_label_defines;
static size_t dollar_label_count;
static size_t dollar_label_max;

int
dollar_label_defined (unsigned int label)
{
  dollar_ent *i;

  know ((dollar_labels != NULL) || (dollar_label_count == 0));

  for (i = dollar_labels; i < dollar_labels + dollar_label_count; ++i)
    if (*i == label)
      return dollar_label_defines[i - dollar_labels];

  /* If we get here, label isn't defined.  */
  return 0;
}

static unsigned int
dollar_label_instance (unsigned int label)
{
  dollar_ent *i;

  know ((dollar_labels != NULL) || (dollar_label_count == 0));

  for (i = dollar_labels; i < dollar_labels + dollar_label_count; ++i)
    if (*i == label)
      return (dollar_label_instances[i - dollar_labels]);

  /* If we get here, we haven't seen the label before.
     Therefore its instance count is zero.  */
  return 0;
}

void
dollar_label_clear (void)
{
  if (dollar_label_count)
    memset (dollar_label_defines, '\0', dollar_label_count);
}

#define DOLLAR_LABEL_BUMP_BY 10

void
define_dollar_label (unsigned int label)
{
  dollar_ent *i;

  for (i = dollar_labels; i < dollar_labels + dollar_label_count; ++i)
    if (*i == label)
      {
	++dollar_label_instances[i - dollar_labels];
	dollar_label_defines[i - dollar_labels] = 1;
	return;
      }

  /* If we get to here, we don't have label listed yet.  */

  if (dollar_labels == NULL)
    {
      dollar_labels = XNEWVEC (dollar_ent, DOLLAR_LABEL_BUMP_BY);
      dollar_label_instances = XNEWVEC (dollar_ent, DOLLAR_LABEL_BUMP_BY);
      dollar_label_defines = XNEWVEC (char, DOLLAR_LABEL_BUMP_BY);
      dollar_label_max = DOLLAR_LABEL_BUMP_BY;
      dollar_label_count = 0;
    }
  else if (dollar_label_count == dollar_label_max)
    {
      dollar_label_max += DOLLAR_LABEL_BUMP_BY;
      dollar_labels = XRESIZEVEC (dollar_ent, dollar_labels,
				  dollar_label_max);
      dollar_label_instances = XRESIZEVEC (dollar_ent,
					   dollar_label_instances,
					   dollar_label_max);
      dollar_label_defines = XRESIZEVEC (char, dollar_label_defines,
					 dollar_label_max);
    }				/* if we needed to grow  */

  dollar_labels[dollar_label_count] = label;
  dollar_label_instances[dollar_label_count] = 1;
  dollar_label_defines[dollar_label_count] = 1;
  ++dollar_label_count;
}

/* Caller must copy returned name: we re-use the area for the next name.

   The mth occurrence of label n: is turned into the symbol "Ln^Am"
   where n is the label number and m is the instance number. "L" makes
   it a label discarded unless debugging and "^A"('\1') ensures no
   ordinary symbol SHOULD get the same name as a local label
   symbol. The first "4:" is "L4^A1" - the m numbers begin at 1.

   fb labels get the same treatment, except that ^B is used in place
   of ^A.

   AUGEND is 0 for current instance, 1 for new instance.  */

char *
dollar_label_name (unsigned int n, unsigned int augend)
{
  /* Returned to caller, then copied.  Used for created names ("4f").  */
  static char symbol_name_build[24];
  char *p = symbol_name_build;

#ifdef LOCAL_LABEL_PREFIX
  *p++ = LOCAL_LABEL_PREFIX;
#endif
  sprintf (p, "L%u%c%u",
	   n, DOLLAR_LABEL_CHAR, dollar_label_instance (n) + augend);
  return symbol_name_build;
}

/* Somebody else's idea of local labels. They are made by "n:" where n
   is any decimal digit. Refer to them with
    "nb" for previous (backward) n:
   or "nf" for next (forward) n:.

   We do a little better and let n be any number, not just a single digit, but
   since the other guy's assembler only does ten, we treat the first ten
   specially.

   Like someone else's assembler, we have one set of local label counters for
   entire assembly, not one set per (sub)segment like in most assemblers. This
   implies that one can refer to a label in another segment, and indeed some
   crufty compilers have done just that.

   Since there could be a LOT of these things, treat them as a sparse
   array.  */

#define FB_LABEL_SPECIAL (10)

typedef unsigned int fb_ent;
static fb_ent fb_low_counter[FB_LABEL_SPECIAL];
static fb_ent *fb_labels;
static fb_ent *fb_label_instances;
static size_t fb_label_count;
static size_t fb_label_max;

/* This must be more than FB_LABEL_SPECIAL.  */
#define FB_LABEL_BUMP_BY (FB_LABEL_SPECIAL + 6)

static void
fb_label_init (void)
{
  memset ((void *) fb_low_counter, '\0', sizeof (fb_low_counter));
}

/* Add one to the instance number of this fb label.  */

void
fb_label_instance_inc (unsigned int label)
{
  fb_ent *i;

  if (label < FB_LABEL_SPECIAL)
    {
      ++fb_low_counter[label];
      return;
    }

  if (fb_labels != NULL)
    {
      for (i = fb_labels + FB_LABEL_SPECIAL;
	   i < fb_labels + fb_label_count; ++i)
	{
	  if (*i == label)
	    {
	      ++fb_label_instances[i - fb_labels];
	      return;
	    }			/* if we find it  */
	}			/* for each existing label  */
    }

  /* If we get to here, we don't have label listed yet.  */

  if (fb_labels == NULL)
    {
      fb_labels = XNEWVEC (fb_ent, FB_LABEL_BUMP_BY);
      fb_label_instances = XNEWVEC (fb_ent, FB_LABEL_BUMP_BY);
      fb_label_max = FB_LABEL_BUMP_BY;
      fb_label_count = FB_LABEL_SPECIAL;

    }
  else if (fb_label_count == fb_label_max)
    {
      fb_label_max += FB_LABEL_BUMP_BY;
      fb_labels = XRESIZEVEC (fb_ent, fb_labels, fb_label_max);
      fb_label_instances = XRESIZEVEC (fb_ent, fb_label_instances,
				       fb_label_max);
    }				/* if we needed to grow  */

  fb_labels[fb_label_count] = label;
  fb_label_instances[fb_label_count] = 1;
  ++fb_label_count;
}

static unsigned int
fb_label_instance (unsigned int label)
{
  fb_ent *i;

  if (label < FB_LABEL_SPECIAL)
    return (fb_low_counter[label]);

  if (fb_labels != NULL)
    {
      for (i = fb_labels + FB_LABEL_SPECIAL;
	   i < fb_labels + fb_label_count; ++i)
	{
	  if (*i == label)
	    return (fb_label_instances[i - fb_labels]);
	}
    }

  /* We didn't find the label, so this must be a reference to the
     first instance.  */
  return 0;
}

/* Caller must copy returned name: we re-use the area for the next name.

   The mth occurrence of label n: is turned into the symbol "Ln^Bm"
   where n is the label number and m is the instance number. "L" makes
   it a label discarded unless debugging and "^B"('\2') ensures no
   ordinary symbol SHOULD get the same name as a local label
   symbol. The first "4:" is "L4^B1" - the m numbers begin at 1.

   dollar labels get the same treatment, except that ^A is used in
   place of ^B.

   AUGEND is 0 for nb, 1 for n:, nf.  */

char *
fb_label_name (unsigned int n, unsigned int augend)
{
  /* Returned to caller, then copied.  Used for created names ("4f").  */
  static char symbol_name_build[24];
  char *p = symbol_name_build;

#ifdef TC_MMIX
  know (augend <= 2 /* See mmix_fb_label.  */);
#else
  know (augend <= 1);
#endif

#ifdef LOCAL_LABEL_PREFIX
  *p++ = LOCAL_LABEL_PREFIX;
#endif
  sprintf (p, "L%u%c%u",
	   n, LOCAL_LABEL_CHAR, fb_label_instance (n) + augend);
  return symbol_name_build;
}

/* Decode name that may have been generated by foo_label_name() above.
   If the name wasn't generated by foo_label_name(), then return it
   unaltered.  This is used for error messages.  */

char *
decode_local_label_name (char *s)
{
  char *p;
  char *symbol_decode;
  int label_number;
  int instance_number;
  const char *type;
  const char *message_format;
  int lindex = 0;

#ifdef LOCAL_LABEL_PREFIX
  if (s[lindex] == LOCAL_LABEL_PREFIX)
    ++lindex;
#endif

  if (s[lindex] != 'L')
    return s;

  for (label_number = 0, p = s + lindex + 1; ISDIGIT (*p); ++p)
    label_number = (10 * label_number) + *p - '0';

  if (*p == DOLLAR_LABEL_CHAR)
    type = "dollar";
  else if (*p == LOCAL_LABEL_CHAR)
    type = "fb";
  else
    return s;

  for (instance_number = 0, p++; ISDIGIT (*p); ++p)
    instance_number = (10 * instance_number) + *p - '0';

  message_format = _("\"%d\" (instance number %d of a %s label)");
  symbol_decode = notes_alloc (strlen (message_format) + 30);
  sprintf (symbol_decode, message_format, label_number, instance_number, type);

  return symbol_decode;
}

/* Get the value of a symbol.  */

valueT
S_GET_VALUE_WHERE (symbolS *s, const char * file, unsigned int line)
{
  if (s->flags.local_symbol)
    return resolve_symbol_value (s);

  if (!s->flags.resolved)
    {
      valueT val = resolve_symbol_value (s);
      if (!finalize_syms)
	return val;
    }
  if (S_IS_WEAKREFR (s))
    return S_GET_VALUE (s->x->value.X_add_symbol);

  if (s->x->value.X_op != O_constant)
    {
      if (! s->flags.resolved
	  || s->x->value.X_op != O_symbol
	  || (S_IS_DEFINED (s) && ! S_IS_COMMON (s)))
	{
	  if (strcmp (S_GET_NAME (s), FAKE_LABEL_NAME) == 0)
	    as_bad_where (file, line, _("expression is too complex to be resolved or converted into relocations"));
	  else if (file != NULL)
	    as_bad_where (file, line, _("attempt to get value of unresolved symbol `%s'"),
			  S_GET_NAME (s));
	  else
	    as_bad (_("attempt to get value of unresolved symbol `%s'"),
		    S_GET_NAME (s));
	}
    }
  return (valueT) s->x->value.X_add_number;
}

valueT
S_GET_VALUE (symbolS *s)
{
  return S_GET_VALUE_WHERE (s, NULL, 0);
}

/* Set the value of a symbol.  */

void
S_SET_VALUE (symbolS *s, valueT val)
{
  if (s->flags.local_symbol)
    {
      ((struct local_symbol *) s)->value = val;
      return;
    }

  s->x->value.X_op = O_constant;
  s->x->value.X_add_number = (offsetT) val;
  s->x->value.X_unsigned = 0;
  S_CLEAR_WEAKREFR (s);
}

void
copy_symbol_attributes (symbolS *dest, symbolS *src)
{
  if (dest->flags.local_symbol)
    dest = local_symbol_convert (dest);
  if (src->flags.local_symbol)
    src = local_symbol_convert (src);

  /* In an expression, transfer the settings of these flags.
     The user can override later, of course.  */
#define COPIED_SYMFLAGS	(BSF_FUNCTION | BSF_OBJECT \
			 | BSF_GNU_INDIRECT_FUNCTION)
  dest->bsym->flags |= src->bsym->flags & COPIED_SYMFLAGS;

#ifdef OBJ_COPY_SYMBOL_ATTRIBUTES
  OBJ_COPY_SYMBOL_ATTRIBUTES (dest, src);
#endif

#ifdef TC_COPY_SYMBOL_ATTRIBUTES
  TC_COPY_SYMBOL_ATTRIBUTES (dest, src);
#endif
}

int
S_IS_FUNCTION (symbolS *s)
{
  flagword flags;

  if (s->flags.local_symbol)
    return 0;

  flags = s->bsym->flags;

  return (flags & BSF_FUNCTION) != 0;
}

int
S_IS_EXTERNAL (symbolS *s)
{
  flagword flags;

  if (s->flags.local_symbol)
    return 0;

  flags = s->bsym->flags;

  /* Sanity check.  */
  if ((flags & BSF_LOCAL) && (flags & BSF_GLOBAL))
    abort ();

  return (flags & BSF_GLOBAL) != 0;
}

int
S_IS_WEAK (symbolS *s)
{
  if (s->flags.local_symbol)
    return 0;
  /* Conceptually, a weakrefr is weak if the referenced symbol is.  We
     could probably handle a WEAKREFR as always weak though.  E.g., if
     the referenced symbol has lost its weak status, there's no reason
     to keep handling the weakrefr as if it was weak.  */
  if (S_IS_WEAKREFR (s))
    return S_IS_WEAK (s->x->value.X_add_symbol);
  return (s->bsym->flags & BSF_WEAK) != 0;
}

int
S_IS_WEAKREFR (symbolS *s)
{
  if (s->flags.local_symbol)
    return 0;
  return s->flags.weakrefr != 0;
}

int
S_IS_WEAKREFD (symbolS *s)
{
  if (s->flags.local_symbol)
    return 0;
  return s->flags.weakrefd != 0;
}

int
S_IS_COMMON (symbolS *s)
{
  if (s->flags.local_symbol)
    return 0;
  return bfd_is_com_section (s->bsym->section);
}

int
S_IS_DEFINED (symbolS *s)
{
  if (s->flags.local_symbol)
    return ((struct local_symbol *) s)->section != undefined_section;
  return s->bsym->section != undefined_section;
}


#ifndef EXTERN_FORCE_RELOC
#define EXTERN_FORCE_RELOC IS_ELF
#endif

/* Return true for symbols that should not be reduced to section
   symbols or eliminated from expressions, because they may be
   overridden by the linker.  */
int
S_FORCE_RELOC (symbolS *s, int strict)
{
  segT sec;
  if (s->flags.local_symbol)
    sec = ((struct local_symbol *) s)->section;
  else
    {
      if ((strict
	   && ((s->bsym->flags & BSF_WEAK) != 0
	       || (EXTERN_FORCE_RELOC
		   && (s->bsym->flags & BSF_GLOBAL) != 0)))
	  || (s->bsym->flags & BSF_GNU_INDIRECT_FUNCTION) != 0)
	return true;
      sec = s->bsym->section;
    }
  return bfd_is_und_section (sec) || bfd_is_com_section (sec);
}

int
S_IS_DEBUG (symbolS *s)
{
  if (s->flags.local_symbol)
    return 0;
  if (s->bsym->flags & BSF_DEBUGGING)
    return 1;
  return 0;
}

int
S_IS_LOCAL (symbolS *s)
{
  flagword flags;
  const char *name;

  if (s->flags.local_symbol)
    return 1;

  flags = s->bsym->flags;

  /* Sanity check.  */
  if ((flags & BSF_LOCAL) && (flags & BSF_GLOBAL))
    abort ();

  if (bfd_asymbol_section (s->bsym) == reg_section)
    return 1;

  if (flag_strip_local_absolute
      /* Keep BSF_FILE symbols in order to allow debuggers to identify
	 the source file even when the object file is stripped.  */
      && (flags & (BSF_GLOBAL | BSF_FILE)) == 0
      && bfd_asymbol_section (s->bsym) == absolute_section)
    return 1;

  name = S_GET_NAME (s);
  return (name != NULL
	  && ! S_IS_DEBUG (s)
	  && (strchr (name, DOLLAR_LABEL_CHAR)
	      || strchr (name, LOCAL_LABEL_CHAR)
#if FAKE_LABEL_CHAR != DOLLAR_LABEL_CHAR
	      || strchr (name, FAKE_LABEL_CHAR)
#endif
	      || TC_LABEL_IS_LOCAL (name)
	      || (! flag_keep_locals
		  && (bfd_is_local_label (stdoutput, s->bsym)
		      || (flag_mri
			  && name[0] == '?'
			  && name[1] == '?')))));
}

int
S_IS_STABD (symbolS *s)
{
  return S_GET_NAME (s) == 0;
}

int
S_CAN_BE_REDEFINED (const symbolS *s)
{
  if (s->flags.local_symbol)
    return (((struct local_symbol *) s)->frag
	    == &predefined_address_frag);
  /* Permit register names to be redefined.  */
  return s->x->value.X_op == O_register;
}

int
S_IS_VOLATILE (const symbolS *s)
{
  if (s->flags.local_symbol)
    return 0;
  return s->flags.volatil;
}

int
S_IS_FORWARD_REF (const symbolS *s)
{
  if (s->flags.local_symbol)
    return 0;
  return s->flags.forward_ref;
}

const char *
S_GET_NAME (symbolS *s)
{
  return s->name;
}

segT
S_GET_SEGMENT (symbolS *s)
{
  if (s->flags.local_symbol)
    return ((struct local_symbol *) s)->section;
  return s->bsym->section;
}

void
S_SET_SEGMENT (symbolS *s, segT seg)
{
  if (s->flags.local_symbol)
    {
      ((struct local_symbol *) s)->section = seg;
      return;
    }

  /* Don't reassign section symbols.  The direct reason is to prevent seg
     faults assigning back to const global symbols such as *ABS*, but it
     shouldn't happen anyway.  */
  if (s->bsym->flags & BSF_SECTION_SYM)
    {
      if (s->bsym->section != seg)
	abort ();
    }
  else
    {
      if (multibyte_handling == multibyte_warn_syms
	  && ! s->flags.local_symbol
	  && seg != undefined_section
	  && ! s->flags.multibyte_warned
	  && scan_for_multibyte_characters ((const unsigned char *) s->name,
					    (const unsigned char *) s->name + strlen (s->name),
					    false))
	{
	  as_warn (_("symbol '%s' contains multibyte characters"), s->name);
	  s->flags.multibyte_warned = 1;
	}

      s->bsym->section = seg;
    }
}

void
S_SET_EXTERNAL (symbolS *s)
{
  if (s->flags.local_symbol)
    s = local_symbol_convert (s);
  if ((s->bsym->flags & BSF_WEAK) != 0)
    {
      /* Let .weak override .global.  */
      return;
    }
  if (s->bsym->flags & BSF_SECTION_SYM)
    {
      /* Do not reassign section symbols.  */
      as_warn (_("can't make section symbol global"));
      return;
    }
#ifndef TC_GLOBAL_REGISTER_SYMBOL_OK
  if (S_GET_SEGMENT (s) == reg_section)
    {
      as_bad (_("can't make register symbol global"));
      return;
    }
#endif
  s->bsym->flags |= BSF_GLOBAL;
  s->bsym->flags &= ~(BSF_LOCAL | BSF_WEAK);

#ifdef TE_PE
  if (! an_external_name && S_GET_NAME(s)[0] != '.')
    an_external_name = S_GET_NAME (s);
#endif
}

void
S_CLEAR_EXTERNAL (symbolS *s)
{
  if (s->flags.local_symbol)
    return;
  if ((s->bsym->flags & BSF_WEAK) != 0)
    {
      /* Let .weak override.  */
      return;
    }
  s->bsym->flags |= BSF_LOCAL;
  s->bsym->flags &= ~(BSF_GLOBAL | BSF_WEAK);
}

void
S_SET_WEAK (symbolS *s)
{
  if (s->flags.local_symbol)
    s = local_symbol_convert (s);
#ifdef obj_set_weak_hook
  obj_set_weak_hook (s);
#endif
  s->bsym->flags |= BSF_WEAK;
  s->bsym->flags &= ~(BSF_GLOBAL | BSF_LOCAL);
}

void
S_SET_WEAKREFR (symbolS *s)
{
  if (s->flags.local_symbol)
    s = local_symbol_convert (s);
  s->flags.weakrefr = 1;
  /* If the alias was already used, make sure we mark the target as
     used as well, otherwise it might be dropped from the symbol
     table.  This may have unintended side effects if the alias is
     later redirected to another symbol, such as keeping the unused
     previous target in the symbol table.  Since it will be weak, it's
     not a big deal.  */
  if (s->flags.used)
    symbol_mark_used (s->x->value.X_add_symbol);
}

void
S_CLEAR_WEAKREFR (symbolS *s)
{
  if (s->flags.local_symbol)
    return;
  s->flags.weakrefr = 0;
}

void
S_SET_WEAKREFD (symbolS *s)
{
  if (s->flags.local_symbol)
    s = local_symbol_convert (s);
  s->flags.weakrefd = 1;
  S_SET_WEAK (s);
}

void
S_CLEAR_WEAKREFD (symbolS *s)
{
  if (s->flags.local_symbol)
    return;
  if (s->flags.weakrefd)
    {
      s->flags.weakrefd = 0;
      /* If a weakref target symbol is weak, then it was never
	 referenced directly before, not even in a .global directive,
	 so decay it to local.  If it remains undefined, it will be
	 later turned into a global, like any other undefined
	 symbol.  */
      if (s->bsym->flags & BSF_WEAK)
	{
#ifdef obj_clear_weak_hook
	  obj_clear_weak_hook (s);
#endif
	  s->bsym->flags &= ~BSF_WEAK;
	  s->bsym->flags |= BSF_LOCAL;
	}
    }
}

void
S_SET_THREAD_LOCAL (symbolS *s)
{
  if (s->flags.local_symbol)
    s = local_symbol_convert (s);
  if (bfd_is_com_section (s->bsym->section)
      && (s->bsym->flags & BSF_THREAD_LOCAL) != 0)
    return;
  s->bsym->flags |= BSF_THREAD_LOCAL;
  if ((s->bsym->flags & BSF_FUNCTION) != 0)
    as_bad (_("Accessing function `%s' as thread-local object"),
	    S_GET_NAME (s));
  else if (! bfd_is_und_section (s->bsym->section)
	   && (s->bsym->section->flags & SEC_THREAD_LOCAL) == 0)
    as_bad (_("Accessing `%s' as thread-local object"),
	    S_GET_NAME (s));
}

void
S_SET_NAME (symbolS *s, const char *name)
{
  s->name = name;
  if (s->flags.local_symbol)
    return;
  s->bsym->name = name;
}

void
S_SET_VOLATILE (symbolS *s)
{
  if (s->flags.local_symbol)
    s = local_symbol_convert (s);
  s->flags.volatil = 1;
}

void
S_CLEAR_VOLATILE (symbolS *s)
{
  if (!s->flags.local_symbol)
    s->flags.volatil = 0;
}

void
S_SET_FORWARD_REF (symbolS *s)
{
  if (s->flags.local_symbol)
    s = local_symbol_convert (s);
  s->flags.forward_ref = 1;
}

/* Return the previous symbol in a chain.  */

symbolS *
symbol_previous (symbolS *s)
{
  if (s->flags.local_symbol)
    abort ();
  return s->x->previous;
}

/* Return the next symbol in a chain.  */

symbolS *
symbol_next (symbolS *s)
{
  if (s->flags.local_symbol)
    abort ();
  return s->x->next;
}

/* Return a pointer to the value of a symbol as an expression.  */

expressionS *
symbol_get_value_expression (symbolS *s)
{
  if (s->flags.local_symbol)
    s = local_symbol_convert (s);
  return &s->x->value;
}

/* Set the value of a symbol to an expression.  */

void
symbol_set_value_expression (symbolS *s, const expressionS *exp)
{
  if (s->flags.local_symbol)
    s = local_symbol_convert (s);
  s->x->value = *exp;
  S_CLEAR_WEAKREFR (s);
}

/* Return whether 2 symbols are the same.  */

int
symbol_same_p (symbolS *s1, symbolS *s2)
{
  return s1 == s2;
}

/* Return a pointer to the X_add_number component of a symbol.  */

offsetT *
symbol_X_add_number (symbolS *s)
{
  if (s->flags.local_symbol)
    return (offsetT *) &((struct local_symbol *) s)->value;

  return &s->x->value.X_add_number;
}

/* Set the value of SYM to the current position in the current segment.  */

void
symbol_set_value_now (symbolS *sym)
{
  S_SET_SEGMENT (sym, now_seg);
  S_SET_VALUE (sym, frag_now_fix ());
  symbol_set_frag (sym, frag_now);
}

/* Set the frag of a symbol.  */

void
symbol_set_frag (symbolS *s, fragS *f)
{
  if (s->flags.local_symbol)
    {
      ((struct local_symbol *) s)->frag = f;
      return;
    }
  s->frag = f;
  S_CLEAR_WEAKREFR (s);
}

/* Return the frag of a symbol.  */

fragS *
symbol_get_frag (symbolS *s)
{
  if (s->flags.local_symbol)
    return ((struct local_symbol *) s)->frag;
  return s->frag;
}

/* Mark a symbol as having been used.  */

void
symbol_mark_used (symbolS *s)
{
  if (s->flags.local_symbol)
    return;
  s->flags.used = 1;
  if (S_IS_WEAKREFR (s))
    symbol_mark_used (s->x->value.X_add_symbol);
}

/* Clear the mark of whether a symbol has been used.  */

void
symbol_clear_used (symbolS *s)
{
  if (s->flags.local_symbol)
    s = local_symbol_convert (s);
  s->flags.used = 0;
}

/* Return whether a symbol has been used.  */

int
symbol_used_p (symbolS *s)
{
  if (s->flags.local_symbol)
    return 1;
  return s->flags.used;
}

/* Mark a symbol as having been used in a reloc.  */

void
symbol_mark_used_in_reloc (symbolS *s)
{
  if (s->flags.local_symbol)
    s = local_symbol_convert (s);
  s->flags.used_in_reloc = 1;
}

/* Clear the mark of whether a symbol has been used in a reloc.  */

void
symbol_clear_used_in_reloc (symbolS *s)
{
  if (s->flags.local_symbol)
    return;
  s->flags.used_in_reloc = 0;
}

/* Return whether a symbol has been used in a reloc.  */

int
symbol_used_in_reloc_p (symbolS *s)
{
  if (s->flags.local_symbol)
    return 0;
  return s->flags.used_in_reloc;
}

/* Mark a symbol as an MRI common symbol.  */

void
symbol_mark_mri_common (symbolS *s)
{
  if (s->flags.local_symbol)
    s = local_symbol_convert (s);
  s->flags.mri_common = 1;
}

/* Clear the mark of whether a symbol is an MRI common symbol.  */

void
symbol_clear_mri_common (symbolS *s)
{
  if (s->flags.local_symbol)
    return;
  s->flags.mri_common = 0;
}

/* Return whether a symbol is an MRI common symbol.  */

int
symbol_mri_common_p (symbolS *s)
{
  if (s->flags.local_symbol)
    return 0;
  return s->flags.mri_common;
}

/* Mark a symbol as having been written.  */

void
symbol_mark_written (symbolS *s)
{
  if (s->flags.local_symbol)
    return;
  s->flags.written = 1;
}

/* Clear the mark of whether a symbol has been written.  */

void
symbol_clear_written (symbolS *s)
{
  if (s->flags.local_symbol)
    return;
  s->flags.written = 0;
}

/* Return whether a symbol has been written.  */

int
symbol_written_p (symbolS *s)
{
  if (s->flags.local_symbol)
    return 0;
  return s->flags.written;
}

/* Mark a symbol as to be removed.  */

void
symbol_mark_removed (symbolS *s)
{
  if (s->flags.local_symbol)
    return;
  s->flags.removed = 1;
}

/* Return whether a symbol has been marked to be removed.  */

int
symbol_removed_p (symbolS *s)
{
  if (s->flags.local_symbol)
    return 0;
  return s->flags.removed;
}

/* Mark a symbol has having been resolved.  */

void
symbol_mark_resolved (symbolS *s)
{
  s->flags.resolved = 1;
}

/* Return whether a symbol has been resolved.  */

int
symbol_resolved_p (symbolS *s)
{
  return s->flags.resolved;
}

/* Return whether a symbol is a section symbol.  */

int
symbol_section_p (symbolS *s)
{
  if (s->flags.local_symbol)
    return 0;
  return (s->bsym->flags & BSF_SECTION_SYM) != 0;
}

/* Return whether a symbol is equated to another symbol.  */

int
symbol_equated_p (symbolS *s)
{
  if (s->flags.local_symbol)
    return 0;
  return s->x->value.X_op == O_symbol;
}

/* Return whether a symbol is equated to another symbol, and should be
   treated specially when writing out relocs.  */

int
symbol_equated_reloc_p (symbolS *s)
{
  if (s->flags.local_symbol)
    return 0;
  /* X_op_symbol, normally not used for O_symbol, is set by
     resolve_symbol_value to flag expression syms that have been
     equated.  */
  return (s->x->value.X_op == O_symbol
#if defined (OBJ_COFF) && defined (TE_PE)
	  && ! S_IS_WEAK (s)
#endif
	  && ((s->flags.resolved && s->x->value.X_op_symbol != NULL)
	      || ! S_IS_DEFINED (s)
	      || S_IS_COMMON (s)));
}

/* Return whether a symbol has a constant value.  */

int
symbol_constant_p (symbolS *s)
{
  if (s->flags.local_symbol)
    return 1;
  return s->x->value.X_op == O_constant;
}

/* Return whether a symbol was cloned and thus removed from the global
   symbol list.  */

int
symbol_shadow_p (symbolS *s)
{
  if (s->flags.local_symbol)
    return 0;
  return s->x->next == s;
}

/* If S is a struct symbol return S, otherwise return NULL.  */

symbolS *
symbol_symbolS (symbolS *s)
{
  if (s->flags.local_symbol)
    return NULL;
  return s;
}

/* Return the BFD symbol for a symbol.  */

asymbol *
symbol_get_bfdsym (symbolS *s)
{
  if (s->flags.local_symbol)
    s = local_symbol_convert (s);
  return s->bsym;
}

/* Set the BFD symbol for a symbol.  */

void
symbol_set_bfdsym (symbolS *s, asymbol *bsym)
{
  if (s->flags.local_symbol)
    s = local_symbol_convert (s);
  /* Usually, it is harmless to reset a symbol to a BFD section
     symbol. For example, obj_elf_change_section sets the BFD symbol
     of an old symbol with the newly created section symbol. But when
     we have multiple sections with the same name, the newly created
     section may have the same name as an old section. We check if the
     old symbol has been already marked as a section symbol before
     resetting it.  */
  if ((s->bsym->flags & BSF_SECTION_SYM) == 0)
    s->bsym = bsym;
  /* else XXX - What do we do now ?  */
}

#ifdef OBJ_SYMFIELD_TYPE

/* Get a pointer to the object format information for a symbol.  */

OBJ_SYMFIELD_TYPE *
symbol_get_obj (symbolS *s)
{
  if (s->flags.local_symbol)
    s = local_symbol_convert (s);
  return &s->x->obj;
}

/* Set the object format information for a symbol.  */

void
symbol_set_obj (symbolS *s, OBJ_SYMFIELD_TYPE *o)
{
  if (s->flags.local_symbol)
    s = local_symbol_convert (s);
  s->x->obj = *o;
}

#endif /* OBJ_SYMFIELD_TYPE */

#ifdef TC_SYMFIELD_TYPE

/* Get a pointer to the processor information for a symbol.  */

TC_SYMFIELD_TYPE *
symbol_get_tc (symbolS *s)
{
  if (s->flags.local_symbol)
    s = local_symbol_convert (s);
  return &s->x->tc;
}

/* Set the processor information for a symbol.  */

void
symbol_set_tc (symbolS *s, TC_SYMFIELD_TYPE *o)
{
  if (s->flags.local_symbol)
    s = local_symbol_convert (s);
  s->x->tc = *o;
}

#endif /* TC_SYMFIELD_TYPE */

void
symbol_begin (void)
{
  symbol_lastP = NULL;
  symbol_rootP = NULL;		/* In case we have 0 symbols (!!)  */
  sy_hash = htab_create_alloc (16, hash_symbol_entry, eq_symbol_entry,
			       NULL, xcalloc, free);

#if defined (EMIT_SECTION_SYMBOLS) || !defined (RELOC_REQUIRES_SYMBOL)
  abs_symbol.bsym = bfd_abs_section_ptr->symbol;
#endif
  abs_symbol.x = &abs_symbol_x;
  abs_symbol.x->value.X_op = O_constant;
  abs_symbol.frag = &zero_address_frag;

  if (LOCAL_LABELS_FB)
    fb_label_init ();
}

void
symbol_end (void)
{
  htab_delete (sy_hash);
}

void
dot_symbol_init (void)
{
  dot_symbol.name = ".";
  dot_symbol.flags.forward_ref = 1;
  dot_symbol.bsym = bfd_make_empty_symbol (stdoutput);
  if (dot_symbol.bsym == NULL)
    as_fatal ("bfd_make_empty_symbol: %s", bfd_errmsg (bfd_get_error ()));
  dot_symbol.bsym->name = ".";
  dot_symbol.x = &dot_symbol_x;
  dot_symbol.x->value.X_op = O_constant;
}

int indent_level;

/* Maximum indent level.
   Available for modification inside a gdb session.  */
static int max_indent_level = 8;

void
print_symbol_value_1 (FILE *file, symbolS *sym)
{
  const char *name = S_GET_NAME (sym);
  if (!name || !name[0])
    name = "(unnamed)";
  fprintf (file, "sym %p %s", sym, name);

  if (sym->flags.local_symbol)
    {
      struct local_symbol *locsym = (struct local_symbol *) sym;

      if (locsym->frag != &zero_address_frag
	  && locsym->frag != NULL)
	fprintf (file, " frag %p", locsym->frag);
      if (locsym->flags.resolved)
	fprintf (file, " resolved");
      fprintf (file, " local");
    }
  else
    {
      if (sym->frag != &zero_address_frag)
	fprintf (file, " frag %p", sym->frag);
      if (sym->flags.written)
	fprintf (file, " written");
      if (sym->flags.resolved)
	fprintf (file, " resolved");
      else if (sym->flags.resolving)
	fprintf (file, " resolving");
      if (sym->flags.used_in_reloc)
	fprintf (file, " used-in-reloc");
      if (sym->flags.used)
	fprintf (file, " used");
      if (S_IS_LOCAL (sym))
	fprintf (file, " local");
      if (S_IS_EXTERNAL (sym))
	fprintf (file, " extern");
      if (S_IS_WEAK (sym))
	fprintf (file, " weak");
      if (S_IS_DEBUG (sym))
	fprintf (file, " debug");
      if (S_IS_DEFINED (sym))
	fprintf (file, " defined");
    }
  if (S_IS_WEAKREFR (sym))
    fprintf (file, " weakrefr");
  if (S_IS_WEAKREFD (sym))
    fprintf (file, " weakrefd");
  fprintf (file, " %s", segment_name (S_GET_SEGMENT (sym)));
  if (symbol_resolved_p (sym))
    {
      segT s = S_GET_SEGMENT (sym);

      if (s != undefined_section
	  && s != expr_section)
	fprintf (file, " %lx", (unsigned long) S_GET_VALUE (sym));
    }
  else if (indent_level < max_indent_level
	   && S_GET_SEGMENT (sym) != undefined_section)
    {
      indent_level++;
      fprintf (file, "\n%*s<", indent_level * 4, "");
      if (sym->flags.local_symbol)
	fprintf (file, "constant %lx",
		 (unsigned long) ((struct local_symbol *) sym)->value);
      else
	print_expr_1 (file, &sym->x->value);
      fprintf (file, ">");
      indent_level--;
    }
  fflush (file);
}

void
print_symbol_value (symbolS *sym)
{
  indent_level = 0;
  print_symbol_value_1 (stderr, sym);
  fprintf (stderr, "\n");
}

static void
print_binary (FILE *file, const char *name, expressionS *exp)
{
  indent_level++;
  fprintf (file, "%s\n%*s<", name, indent_level * 4, "");
  print_symbol_value_1 (file, exp->X_add_symbol);
  fprintf (file, ">\n%*s<", indent_level * 4, "");
  print_symbol_value_1 (file, exp->X_op_symbol);
  fprintf (file, ">");
  indent_level--;
}

void
print_expr_1 (FILE *file, expressionS *exp)
{
  fprintf (file, "expr %p ", exp);
  switch (exp->X_op)
    {
    case O_illegal:
      fprintf (file, "illegal");
      break;
    case O_absent:
      fprintf (file, "absent");
      break;
    case O_constant:
      fprintf (file, "constant %" PRIx64, (uint64_t) exp->X_add_number);
      break;
    case O_symbol:
      indent_level++;
      fprintf (file, "symbol\n%*s<", indent_level * 4, "");
      print_symbol_value_1 (file, exp->X_add_symbol);
      fprintf (file, ">");
    maybe_print_addnum:
      if (exp->X_add_number)
	fprintf (file, "\n%*s%" PRIx64, indent_level * 4, "",
		 (uint64_t) exp->X_add_number);
      indent_level--;
      break;
    case O_register:
      fprintf (file, "register #%d", (int) exp->X_add_number);
      break;
    case O_big:
      fprintf (file, "big");
      break;
    case O_uminus:
      fprintf (file, "uminus -<");
      indent_level++;
      print_symbol_value_1 (file, exp->X_add_symbol);
      fprintf (file, ">");
      goto maybe_print_addnum;
    case O_bit_not:
      fprintf (file, "bit_not");
      break;
    case O_multiply:
      print_binary (file, "multiply", exp);
      break;
    case O_divide:
      print_binary (file, "divide", exp);
      break;
    case O_modulus:
      print_binary (file, "modulus", exp);
      break;
    case O_left_shift:
      print_binary (file, "lshift", exp);
      break;
    case O_right_shift:
      print_binary (file, "rshift", exp);
      break;
    case O_bit_inclusive_or:
      print_binary (file, "bit_ior", exp);
      break;
    case O_bit_exclusive_or:
      print_binary (file, "bit_xor", exp);
      break;
    case O_bit_and:
      print_binary (file, "bit_and", exp);
      break;
    case O_eq:
      print_binary (file, "eq", exp);
      break;
    case O_ne:
      print_binary (file, "ne", exp);
      break;
    case O_lt:
      print_binary (file, "lt", exp);
      break;
    case O_le:
      print_binary (file, "le", exp);
      break;
    case O_ge:
      print_binary (file, "ge", exp);
      break;
    case O_gt:
      print_binary (file, "gt", exp);
      break;
    case O_logical_and:
      print_binary (file, "logical_and", exp);
      break;
    case O_logical_or:
      print_binary (file, "logical_or", exp);
      break;
    case O_add:
      indent_level++;
      fprintf (file, "add\n%*s<", indent_level * 4, "");
      print_symbol_value_1 (file, exp->X_add_symbol);
      fprintf (file, ">\n%*s<", indent_level * 4, "");
      print_symbol_value_1 (file, exp->X_op_symbol);
      fprintf (file, ">");
      goto maybe_print_addnum;
    case O_subtract:
      indent_level++;
      fprintf (file, "subtract\n%*s<", indent_level * 4, "");
      print_symbol_value_1 (file, exp->X_add_symbol);
      fprintf (file, ">\n%*s<", indent_level * 4, "");
      print_symbol_value_1 (file, exp->X_op_symbol);
      fprintf (file, ">");
      goto maybe_print_addnum;
    default:
      fprintf (file, "{unknown opcode %d}", (int) exp->X_op);
      break;
    }
  fflush (stdout);
}

void
print_expr (expressionS *exp)
{
  print_expr_1 (stderr, exp);
  fprintf (stderr, "\n");
}

void
symbol_print_statistics (FILE *file)
{
  htab_print_statistics (file, "symbol table", sy_hash);
  fprintf (file, "%lu mini local symbols created, %lu converted\n",
	   local_symbol_count, local_symbol_conversion_count);
}

#ifdef OBJ_COMPLEX_RELC

/* Convert given symbol to a new complex-relocation symbol name.  This
   may be a recursive function, since it might be called for non-leaf
   nodes (plain symbols) in the expression tree.  The caller owns the
   returning string, so should free it eventually.  Errors are
   indicated via as_bad and a NULL return value.  The given symbol
   is marked with used_in_reloc.  */

char *
symbol_relc_make_sym (symbolS * sym)
{
  char * terminal = NULL;
  const char * sname;
  char typetag;
  int sname_len;

  gas_assert (sym != NULL);

  /* Recurse to symbol_relc_make_expr if this symbol
     is defined as an expression or a plain value.  */
  if (   S_GET_SEGMENT (sym) == expr_section
      || S_GET_SEGMENT (sym) == absolute_section)
    return symbol_relc_make_expr (symbol_get_value_expression (sym));

  /* This may be a "fake symbol", referring to ".".
     Write out a special null symbol to refer to this position.  */
  if (! strcmp (S_GET_NAME (sym), FAKE_LABEL_NAME))
    return xstrdup (".");

  /* We hope this is a plain leaf symbol.  Construct the encoding
     as {S,s}II...:CCCCCCC....
     where 'S'/'s' means section symbol / plain symbol
     III is decimal for the symbol name length
     CCC is the symbol name itself.  */
  symbol_mark_used_in_reloc (sym);

  sname = S_GET_NAME (sym);
  sname_len = strlen (sname);
  typetag = symbol_section_p (sym) ? 'S' : 's';

  terminal = XNEWVEC (char, (1 /* S or s */
			     + 8 /* sname_len in decimal */
			     + 1 /* _ spacer */
			     + sname_len /* name itself */
			     + 1 /* \0 */ ));

  sprintf (terminal, "%c%d:%s", typetag, sname_len, sname);
  return terminal;
}

/* Convert given value to a new complex-relocation symbol name.  This
   is a non-recursive function, since it is be called for leaf nodes
   (plain values) in the expression tree.  The caller owns the
   returning string, so should free() it eventually.  No errors.  */

char *
symbol_relc_make_value (offsetT val)
{
  char * terminal = XNEWVEC (char, 28);  /* Enough for long long.  */

  terminal[0] = '#';
  bfd_sprintf_vma (stdoutput, terminal + 1, val);
  return terminal;
}

/* Convert given expression to a new complex-relocation symbol name.
   This is a recursive function, since it traverses the entire given
   expression tree.  The caller owns the returning string, so should
   free() it eventually.  Errors are indicated via as_bad() and a NULL
   return value.  */

char *
symbol_relc_make_expr (expressionS * exp)
{
  const char * opstr = NULL; /* Operator prefix string.  */
  int    arity = 0;    /* Arity of this operator.  */
  char * operands[3];  /* Up to three operands.  */
  char * concat_string = NULL;

  operands[0] = operands[1] = operands[2] = NULL;

  gas_assert (exp != NULL);

  /* Match known operators -> fill in opstr, arity, operands[] and fall
     through to construct subexpression fragments; may instead return
     string directly for leaf nodes.  */

  /* See expr.h for the meaning of all these enums.  Many operators
     have an unnatural arity (X_add_number implicitly added).  The
     conversion logic expands them to explicit "+" subexpressions.   */

  switch (exp->X_op)
    {
    default:
      as_bad ("Unknown expression operator (enum %d)", exp->X_op);
      break;

      /* Leaf nodes.  */
    case O_constant:
      return symbol_relc_make_value (exp->X_add_number);

    case O_symbol:
      if (exp->X_add_number)
	{
	  arity = 2;
	  opstr = "+";
	  operands[0] = symbol_relc_make_sym (exp->X_add_symbol);
	  operands[1] = symbol_relc_make_value (exp->X_add_number);
	  break;
	}
      else
	return symbol_relc_make_sym (exp->X_add_symbol);

      /* Helper macros for nesting nodes.  */

#define HANDLE_XADD_OPT1(str_)						\
      if (exp->X_add_number)						\
	{								\
	  arity = 2;							\
	  opstr = "+:" str_;						\
	  operands[0] = symbol_relc_make_sym (exp->X_add_symbol);	\
	  operands[1] = symbol_relc_make_value (exp->X_add_number);	\
	  break;							\
	}								\
      else								\
	{								\
	  arity = 1;							\
	  opstr = str_;							\
	  operands[0] = symbol_relc_make_sym (exp->X_add_symbol);	\
	}								\
      break

#define HANDLE_XADD_OPT2(str_)						\
      if (exp->X_add_number)						\
	{								\
	  arity = 3;							\
	  opstr = "+:" str_;						\
	  operands[0] = symbol_relc_make_sym (exp->X_add_symbol);	\
	  operands[1] = symbol_relc_make_sym (exp->X_op_symbol);	\
	  operands[2] = symbol_relc_make_value (exp->X_add_number);	\
	}								\
      else								\
	{								\
	  arity = 2;							\
	  opstr = str_;							\
	  operands[0] = symbol_relc_make_sym (exp->X_add_symbol);	\
	  operands[1] = symbol_relc_make_sym (exp->X_op_symbol);	\
	}								\
      break

      /* Nesting nodes.  */

    case O_uminus:		HANDLE_XADD_OPT1 ("0-");
    case O_bit_not:		HANDLE_XADD_OPT1 ("~");
    case O_logical_not:		HANDLE_XADD_OPT1 ("!");
    case O_multiply:		HANDLE_XADD_OPT2 ("*");
    case O_divide:		HANDLE_XADD_OPT2 ("/");
    case O_modulus:		HANDLE_XADD_OPT2 ("%");
    case O_left_shift:		HANDLE_XADD_OPT2 ("<<");
    case O_right_shift:		HANDLE_XADD_OPT2 (">>");
    case O_bit_inclusive_or:	HANDLE_XADD_OPT2 ("|");
    case O_bit_exclusive_or:	HANDLE_XADD_OPT2 ("^");
    case O_bit_and:		HANDLE_XADD_OPT2 ("&");
    case O_add:			HANDLE_XADD_OPT2 ("+");
    case O_subtract:		HANDLE_XADD_OPT2 ("-");
    case O_eq:			HANDLE_XADD_OPT2 ("==");
    case O_ne:			HANDLE_XADD_OPT2 ("!=");
    case O_lt:			HANDLE_XADD_OPT2 ("<");
    case O_le:			HANDLE_XADD_OPT2 ("<=");
    case O_ge:			HANDLE_XADD_OPT2 (">=");
    case O_gt:			HANDLE_XADD_OPT2 (">");
    case O_logical_and:		HANDLE_XADD_OPT2 ("&&");
    case O_logical_or:		HANDLE_XADD_OPT2 ("||");
    }

  /* Validate & reject early.  */
  if (arity >= 1 && ((operands[0] == NULL) || (strlen (operands[0]) == 0)))
    opstr = NULL;
  if (arity >= 2 && ((operands[1] == NULL) || (strlen (operands[1]) == 0)))
    opstr = NULL;
  if (arity >= 3 && ((operands[2] == NULL) || (strlen (operands[2]) == 0)))
    opstr = NULL;

  if (opstr == NULL)
    concat_string = NULL;
  else if (arity == 0)
    concat_string = xstrdup (opstr);
  else if (arity == 1)
    concat_string = concat (opstr, ":", operands[0], (char *) NULL);
  else if (arity == 2)
    concat_string = concat (opstr, ":", operands[0], ":", operands[1],
			    (char *) NULL);
  else
    concat_string = concat (opstr, ":", operands[0], ":", operands[1], ":",
			    operands[2], (char *) NULL);

  /* Free operand strings (not opstr).  */
  if (arity >= 1) xfree (operands[0]);
  if (arity >= 2) xfree (operands[1]);
  if (arity >= 3) xfree (operands[2]);

  return concat_string;
}

#endif
