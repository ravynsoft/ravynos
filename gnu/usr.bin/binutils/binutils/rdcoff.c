/* stabs.c -- Parse COFF debugging information
   Copyright (C) 1996-2023 Free Software Foundation, Inc.
   Written by Ian Lance Taylor <ian@cygnus.com>.

   This file is part of GNU Binutils.

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
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

/* This file contains code which parses COFF debugging information.  */

#include "sysdep.h"
#include "bfd.h"
#include "coff/internal.h"
#include "libiberty.h"
#include "bucomm.h"
#include "debug.h"
#include "budbg.h"

/* FIXME: We should not need this BFD internal file.  We need it for
   the N_BTMASK, etc., values.  */
#include "libcoff.h"

/* These macros extract the right mask and shifts for this BFD.  They
   assume that there is a local variable named ABFD.  This is so that
   macros like ISFCN and DECREF, from coff/internal.h, will work
   without modification.  */
#define N_BTMASK (coff_data (abfd)->local_n_btmask)
#define	N_BTSHFT (coff_data (abfd)->local_n_btshft)
#define	N_TMASK  (coff_data (abfd)->local_n_tmask)
#define	N_TSHIFT (coff_data (abfd)->local_n_tshift)

/* This structure is used to hold the symbols, as well as the current
   location within the symbols.  */

struct coff_symbols
{
  /* The symbols.  */
  asymbol **syms;
  /* The number of symbols.  */
  long symcount;
  /* The index of the current symbol.  */
  long symno;
  /* The index of the current symbol in the COFF symbol table (where
     each auxent counts as a symbol).  */
  long coff_symno;
};

/* This structure is used to map symbol indices to types.  */

struct coff_types
{
  /* Next set of slots.  */
  struct coff_types *next;
  /* Where the TYPES array starts.  */
  unsigned int base_index;
  /* Slots.  */
#define COFF_SLOTS (16)
  debug_type types[COFF_SLOTS];
};

static debug_type parse_coff_base_type
  (bfd *, struct coff_symbols *, struct coff_types **, long, int,
   union internal_auxent *, void *);
static debug_type parse_coff_struct_type
  (bfd *, struct coff_symbols *, struct coff_types **, int,
   union internal_auxent *, void *);
static debug_type parse_coff_enum_type
  (bfd *, struct coff_symbols *, struct coff_types **,
   union internal_auxent *, void *);

/* Return the slot for a type.  */

static debug_type *
coff_get_slot (void *dhandle, struct coff_types **types, long indx)
{
  unsigned int base_index;

  base_index = indx / COFF_SLOTS * COFF_SLOTS;
  indx -= base_index;

  while (*types && (*types)->base_index < base_index)
    types = &(*types)->next;

  if (*types == NULL || (*types)->base_index != base_index)
    {
      struct coff_types *n = debug_xzalloc (dhandle, sizeof (*n));
      n->next = *types;
      n->base_index = base_index;
      *types = n;
    }

  return (*types)->types + indx;
}

/* Parse a COFF type code in NTYPE.  */

static debug_type
parse_coff_type (bfd *abfd, struct coff_symbols *symbols,
		 struct coff_types **types, long coff_symno, int ntype,
		 union internal_auxent *pauxent, bool useaux,
		 void *dhandle)
{
  debug_type type;

  if ((ntype & ~N_BTMASK) != 0)
    {
      int newtype;

      newtype = DECREF (ntype);

      if (ISPTR (ntype))
	{
	  type = parse_coff_type (abfd, symbols, types, coff_symno, newtype,
				  pauxent, useaux, dhandle);
	  type = debug_make_pointer_type (dhandle, type);
	}
      else if (ISFCN (ntype))
	{
	  type = parse_coff_type (abfd, symbols, types, coff_symno, newtype,
				  pauxent, useaux, dhandle);
	  type = debug_make_function_type (dhandle, type, (debug_type *) NULL,
					   false);
	}
      else if (ISARY (ntype))
	{
	  int n;

	  if (pauxent == NULL)
	    n = 0;
	  else
	    {
	      unsigned short *dim;
	      int i;

	      /* FIXME: If pauxent->x_sym.x_tagndx.l == 0, gdb sets
                 the c_naux field of the syment to 0.  */

	      /* Move the dimensions down, so that the next array
                 picks up the next one.  */
	      dim = pauxent->x_sym.x_fcnary.x_ary.x_dimen;
	      n = dim[0];
	      for (i = 0; *dim != 0 && i < DIMNUM - 1; i++, dim++)
		*dim = *(dim + 1);
	      *dim = 0;
	    }

	  type = parse_coff_type (abfd, symbols, types, coff_symno, newtype,
				  pauxent, false, dhandle);
	  type = debug_make_array_type (dhandle, type,
					parse_coff_base_type (abfd, symbols,
							      types,
							      coff_symno,
							      T_INT,
							      NULL, dhandle),
					0, n - 1, false);
	}
      else
	{
	  non_fatal (_("parse_coff_type: Bad type code 0x%x"), ntype);
	  return DEBUG_TYPE_NULL;
	}

      return type;
    }

  if (pauxent != NULL && (int32_t) pauxent->x_sym.x_tagndx.u32 > 0)
    {
      debug_type *slot;

      /* This is a reference to an existing type.  FIXME: gdb checks
	 that the class is not C_STRTAG, nor C_UNTAG, nor C_ENTAG.  */
      slot = coff_get_slot (dhandle, types, pauxent->x_sym.x_tagndx.u32);
      if (*slot != DEBUG_TYPE_NULL)
	return *slot;
      else
	return debug_make_indirect_type (dhandle, slot, (const char *) NULL);
    }

  /* If the aux entry has already been used for something, useaux will
     have been set to false, indicating that parse_coff_base_type
     should not use it.  We need to do it this way, rather than simply
     passing pauxent as NULL, because we need to be able handle
     multiple array dimensions while still discarding pauxent after
     having handled all of them.  */
  if (! useaux)
    pauxent = NULL;

  return parse_coff_base_type (abfd, symbols, types, coff_symno, ntype,
			       pauxent, dhandle);
}

/* Parse a basic COFF type in NTYPE.  */

static debug_type
parse_coff_base_type (bfd *abfd, struct coff_symbols *symbols,
		      struct coff_types **types, long coff_symno, int ntype,
		      union internal_auxent *pauxent, void *dhandle)
{
  debug_type ret;
  const char *name = NULL;

  switch (ntype)
    {
    default:
      ret = debug_make_void_type (dhandle);
      break;

    case T_NULL:
    case T_VOID:
      ret = debug_make_void_type (dhandle);
      name = "void";
      break;

    case T_CHAR:
      ret = debug_make_int_type (dhandle, 1, false);
      name = "char";
      break;

    case T_SHORT:
      ret = debug_make_int_type (dhandle, 2, false);
      name = "short";
      break;

    case T_INT:
      /* FIXME: Perhaps the size should depend upon the architecture.  */
      ret = debug_make_int_type (dhandle, 4, false);
      name = "int";
      break;

    case T_LONG:
      ret = debug_make_int_type (dhandle, 4, false);
      name = "long";
      break;

    case T_FLOAT:
      ret = debug_make_float_type (dhandle, 4);
      name = "float";
      break;

    case T_DOUBLE:
      ret = debug_make_float_type (dhandle, 8);
      name = "double";
      break;

    case T_LNGDBL:
      ret = debug_make_float_type (dhandle, 12);
      name = "long double";
      break;

    case T_UCHAR:
      ret = debug_make_int_type (dhandle, 1, true);
      name = "unsigned char";
      break;

    case T_USHORT:
      ret = debug_make_int_type (dhandle, 2, true);
      name = "unsigned short";
      break;

    case T_UINT:
      ret = debug_make_int_type (dhandle, 4, true);
      name = "unsigned int";
      break;

    case T_ULONG:
      ret = debug_make_int_type (dhandle, 4, true);
      name = "unsigned long";
      break;

    case T_STRUCT:
      if (pauxent == NULL)
	ret = debug_make_struct_type (dhandle, true, 0,
				      (debug_field *) NULL);
      else
	ret = parse_coff_struct_type (abfd, symbols, types, ntype, pauxent,
				      dhandle);
      break;

    case T_UNION:
      if (pauxent == NULL)
	ret = debug_make_struct_type (dhandle, false, 0, (debug_field *) NULL);
      else
	ret = parse_coff_struct_type (abfd, symbols, types, ntype, pauxent,
				      dhandle);
      break;

    case T_ENUM:
      if (pauxent == NULL)
	ret = debug_make_enum_type (dhandle, (const char **) NULL,
				    (bfd_signed_vma *) NULL);
      else
	ret = parse_coff_enum_type (abfd, symbols, types, pauxent, dhandle);
      break;
    }

  if (name != NULL)
    ret = debug_name_type (dhandle, name, ret);

  debug_type *slot = coff_get_slot (dhandle, types, coff_symno);
  *slot = ret;

  return ret;
}

/* Parse a struct type.  */

static debug_type
parse_coff_struct_type (bfd *abfd, struct coff_symbols *symbols,
			struct coff_types **types, int ntype,
			union internal_auxent *pauxent, void *dhandle)
{
  long symend;
  int alloc;
  debug_field *fields, *xfields;
  int count;
  bool done;

  symend = pauxent->x_sym.x_fcnary.x_fcn.x_endndx.u32;

  alloc = 10;
  fields = (debug_field *) xmalloc (alloc * sizeof *fields);
  count = 0;

  done = false;
  while (! done
	 && symbols->coff_symno < symend
	 && symbols->symno < symbols->symcount)
    {
      asymbol *sym;
      long this_coff_symno;
      struct internal_syment syment;
      union internal_auxent auxent;
      union internal_auxent *psubaux;
      bfd_vma bitpos = 0, bitsize = 0;

      sym = symbols->syms[symbols->symno];

      if (! bfd_coff_get_syment (abfd, sym, &syment))
	{
	  non_fatal (_("bfd_coff_get_syment failed: %s"),
		     bfd_errmsg (bfd_get_error ()));
	  free (fields);
	  return DEBUG_TYPE_NULL;
	}

      this_coff_symno = symbols->coff_symno;

      ++symbols->symno;
      symbols->coff_symno += 1 + syment.n_numaux;

      if (syment.n_numaux == 0)
	psubaux = NULL;
      else
	{
	  if (! bfd_coff_get_auxent (abfd, sym, 0, &auxent))
	    {
	      non_fatal (_("bfd_coff_get_auxent failed: %s"),
			 bfd_errmsg (bfd_get_error ()));
	      free (fields);
	      return DEBUG_TYPE_NULL;
	    }
	  psubaux = &auxent;
	}

      switch (syment.n_sclass)
	{
	case C_MOS:
	case C_MOU:
	  bitpos = 8 * bfd_asymbol_value (sym);
	  bitsize = 0;
	  break;

	case C_FIELD:
	  bitpos = bfd_asymbol_value (sym);
	  if (psubaux != NULL)
	    bitsize = psubaux->x_sym.x_misc.x_lnsz.x_size;
	  break;

	case C_EOS:
	  done = true;
	  break;
	}

      if (! done)
	{
	  debug_type ftype;
	  debug_field f;

	  ftype = parse_coff_type (abfd, symbols, types, this_coff_symno,
				   syment.n_type, psubaux, true, dhandle);
	  f = debug_make_field (dhandle, bfd_asymbol_name (sym), ftype,
				bitpos, bitsize, DEBUG_VISIBILITY_PUBLIC);
	  if (f == DEBUG_FIELD_NULL)
	    {
	      free (fields);
	      return DEBUG_TYPE_NULL;
	    }

	  if (count + 1 >= alloc)
	    {
	      alloc += 10;
	      fields = ((debug_field *)
			xrealloc (fields, alloc * sizeof *fields));
	    }

	  fields[count] = f;
	  ++count;
	}
    }

  fields[count] = DEBUG_FIELD_NULL;
  xfields = debug_xalloc (dhandle, (count + 1) * sizeof (*fields));
  memcpy (xfields, fields, (count + 1) * sizeof (*fields));
  free (fields);

  return debug_make_struct_type (dhandle, ntype == T_STRUCT,
				 pauxent->x_sym.x_misc.x_lnsz.x_size,
				 xfields);
}

/* Parse an enum type.  */

static debug_type
parse_coff_enum_type (bfd *abfd, struct coff_symbols *symbols,
		      struct coff_types **types ATTRIBUTE_UNUSED,
		      union internal_auxent *pauxent, void *dhandle)
{
  long symend;
  int alloc;
  const char **names, **xnames;
  bfd_signed_vma *vals, *xvals;
  int count;
  bool done;

  symend = pauxent->x_sym.x_fcnary.x_fcn.x_endndx.u32;

  alloc = 10;
  names = (const char **) xmalloc (alloc * sizeof *names);
  vals = (bfd_signed_vma *) xmalloc (alloc * sizeof *vals);
  count = 0;

  done = false;
  while (! done
	 && symbols->coff_symno < symend
	 && symbols->symno < symbols->symcount)
    {
      asymbol *sym;
      struct internal_syment syment;

      sym = symbols->syms[symbols->symno];

      if (! bfd_coff_get_syment (abfd, sym, &syment))
	{
	  non_fatal (_("bfd_coff_get_syment failed: %s"),
		     bfd_errmsg (bfd_get_error ()));
	  free (names);
	  free (vals);
	  return DEBUG_TYPE_NULL;
	}

      ++symbols->symno;
      symbols->coff_symno += 1 + syment.n_numaux;

      switch (syment.n_sclass)
	{
	case C_MOE:
	  if (count + 1 >= alloc)
	    {
	      alloc += 10;
	      names = ((const char **)
		       xrealloc (names, alloc * sizeof *names));
	      vals = ((bfd_signed_vma *)
		      xrealloc (vals, alloc * sizeof *vals));
	    }

	  names[count] = bfd_asymbol_name (sym);
	  vals[count] = bfd_asymbol_value (sym);
	  ++count;
	  break;

	case C_EOS:
	  done = true;
	  break;
	}
    }

  names[count] = NULL;
  vals[count] = 0;
  xnames = debug_xalloc (dhandle, (count + 1) * sizeof (*names));
  memcpy (xnames, names, (count + 1) * sizeof (*names));
  free (names);
  xvals = debug_xalloc (dhandle, (count + 1) * sizeof (*vals));
  memcpy (xvals, vals, (count + 1) * sizeof (*vals));
  free (vals);

  return debug_make_enum_type (dhandle, xnames, xvals);
}

/* Handle a single COFF symbol.  */

static bool
parse_coff_symbol (bfd *abfd ATTRIBUTE_UNUSED, struct coff_types **types,
		   asymbol *sym, long coff_symno,
		   struct internal_syment *psyment, void *dhandle,
		   debug_type type, bool within_function)
{
  switch (psyment->n_sclass)
    {
    case C_NULL:
      break;

    case C_AUTO:
      if (! debug_record_variable (dhandle, bfd_asymbol_name (sym), type,
				   DEBUG_LOCAL, bfd_asymbol_value (sym)))
	return false;
      break;

    case C_WEAKEXT:
    case C_EXT:
      if (! debug_record_variable (dhandle, bfd_asymbol_name (sym), type,
				   DEBUG_GLOBAL, bfd_asymbol_value (sym)))
	return false;
      break;

    case C_STAT:
      if (! debug_record_variable (dhandle, bfd_asymbol_name (sym), type,
				   (within_function
				    ? DEBUG_LOCAL_STATIC
				    : DEBUG_STATIC),
				   bfd_asymbol_value (sym)))
	return false;
      break;

    case C_REG:
      /* FIXME: We may need to convert the register number.  */
      if (! debug_record_variable (dhandle, bfd_asymbol_name (sym), type,
				   DEBUG_REGISTER, bfd_asymbol_value (sym)))
	return false;
      break;

    case C_LABEL:
      break;

    case C_ARG:
      if (! debug_record_parameter (dhandle, bfd_asymbol_name (sym), type,
				    DEBUG_PARM_STACK, bfd_asymbol_value (sym)))
	return false;
      break;

    case C_REGPARM:
      /* FIXME: We may need to convert the register number.  */
      if (! debug_record_parameter (dhandle, bfd_asymbol_name (sym), type,
				    DEBUG_PARM_REG, bfd_asymbol_value (sym)))
	return false;
      break;

    case C_TPDEF:
      type = debug_name_type (dhandle, bfd_asymbol_name (sym), type);
      if (type == DEBUG_TYPE_NULL)
	return false;
      break;

    case C_STRTAG:
    case C_UNTAG:
    case C_ENTAG:
      {
	debug_type *slot;

	type = debug_tag_type (dhandle, bfd_asymbol_name (sym), type);
	if (type == DEBUG_TYPE_NULL)
	  return false;

	/* Store the named type into the slot, so that references get
           the name.  */
	slot = coff_get_slot (dhandle, types, coff_symno);
	*slot = type;
      }
      break;

    default:
      break;
    }

  return true;
}

/* Determine if a symbol has external visibility.  */

static bool
external_coff_symbol_p (int sym_class)
{
  switch (sym_class)
    {
    case C_EXT:
    case C_WEAKEXT:
      return true;
    default:
      break;
    }
  return false;
}

/* This is the main routine.  It looks through all the symbols and
   handles them.  */

bool
parse_coff (bfd *abfd, asymbol **syms, long symcount, void *dhandle)
{
  struct coff_symbols symbols;
  struct coff_types *types;
  long next_c_file;
  const char *fnname;
  int fnclass;
  int fntype;
  bfd_vma fnend;
  alent *linenos;
  bool within_function;
  long this_coff_symno;

  symbols.syms = syms;
  symbols.symcount = symcount;
  symbols.symno = 0;
  symbols.coff_symno = 0;

  types= NULL;

  next_c_file = -1;
  fnname = NULL;
  fnclass = 0;
  fntype = 0;
  fnend = 0;
  linenos = NULL;
  within_function = false;

  while (symbols.symno < symcount)
    {
      asymbol *sym;
      const char *name;
      struct internal_syment syment;
      union internal_auxent auxent;
      union internal_auxent *paux;
      debug_type type;

      sym = syms[symbols.symno];

      if (! bfd_coff_get_syment (abfd, sym, &syment))
	{
	  non_fatal (_("bfd_coff_get_syment failed: %s"),
		     bfd_errmsg (bfd_get_error ()));
	  return false;
	}

      name = bfd_asymbol_name (sym);

      this_coff_symno = symbols.coff_symno;

      ++symbols.symno;
      symbols.coff_symno += 1 + syment.n_numaux;

      /* We only worry about the first auxent, because that is the
	 only one which is relevant for debugging information.  */
      if (syment.n_numaux == 0)
	paux = NULL;
      else
	{
	  if (! bfd_coff_get_auxent (abfd, sym, 0, &auxent))
	    {
	      non_fatal (_("bfd_coff_get_auxent failed: %s"),
			 bfd_errmsg (bfd_get_error ()));
	      return false;
	    }
	  paux = &auxent;
	}

      if (this_coff_symno == next_c_file && syment.n_sclass != C_FILE)
	{
	  /* The last C_FILE symbol points to the first external
             symbol.  */
	  if (! debug_set_filename (dhandle, "*globals*"))
	    return false;
	}

      switch (syment.n_sclass)
	{
	case C_EFCN:
	case C_EXTDEF:
	case C_ULABEL:
	case C_USTATIC:
	case C_LINE:
	case C_ALIAS:
	case C_HIDDEN:
	  /* Just ignore these classes.  */
	  break;

	case C_FILE:
	  next_c_file = syment.n_value;
	  if (! debug_set_filename (dhandle, name))
	    return false;
	  break;

	case C_STAT:
	  /* Ignore static symbols with a type of T_NULL.  These
             represent section entries.  */
	  if (syment.n_type == T_NULL)
	    break;
	  /* Fall through.  */
	case C_WEAKEXT:
	case C_EXT:
	  if (ISFCN (syment.n_type))
	    {
	      fnname = name;
	      fnclass = syment.n_sclass;
	      fntype = syment.n_type;
	      if (syment.n_numaux > 0)
		fnend = bfd_asymbol_value (sym) + auxent.x_sym.x_misc.x_fsize;
	      else
		fnend = 0;
	      linenos = BFD_SEND (abfd, _get_lineno, (abfd, sym));
	      break;
	    }
	  type = parse_coff_type (abfd, &symbols, &types, this_coff_symno,
				  syment.n_type, paux, true, dhandle);
	  if (type == DEBUG_TYPE_NULL)
	    return false;
	  if (! parse_coff_symbol (abfd, &types, sym, this_coff_symno, &syment,
				   dhandle, type, within_function))
	    return false;
	  break;

	case C_FCN:
	  if (strcmp (name, ".bf") == 0)
	    {
	      if (fnname == NULL)
		{
		  non_fatal (_("%ld: .bf without preceding function"),
			     this_coff_symno);
		  return false;
		}

	      type = parse_coff_type (abfd, &symbols, &types, this_coff_symno,
				      DECREF (fntype), paux, false, dhandle);
	      if (type == DEBUG_TYPE_NULL)
		return false;

	      if (! debug_record_function (dhandle, fnname, type,
					   external_coff_symbol_p (fnclass),
					   bfd_asymbol_value (sym)))
		return false;

	      if (linenos != NULL)
		{
		  int base;
		  bfd_vma addr;

		  if (syment.n_numaux == 0)
		    base = 0;
		  else
		    base = auxent.x_sym.x_misc.x_lnsz.x_lnno - 1;

		  addr = bfd_section_vma (bfd_asymbol_section (sym));

		  ++linenos;

		  while (linenos->line_number != 0)
		    {
		      if (! debug_record_line (dhandle,
					       linenos->line_number + base,
					       linenos->u.offset + addr))
			return false;
		      ++linenos;
		    }
		}

	      fnname = NULL;
	      linenos = NULL;
	      fnclass = 0;
	      fntype = 0;

	      within_function = true;
	    }
	  else if (strcmp (name, ".ef") == 0)
	    {
	      if (! within_function)
		{
		  non_fatal (_("%ld: unexpected .ef\n"), this_coff_symno);
		  return false;
		}

	      if (bfd_asymbol_value (sym) > fnend)
		fnend = bfd_asymbol_value (sym);
	      if (! debug_end_function (dhandle, fnend))
		return false;

	      fnend = 0;
	      within_function = false;
	    }
	  break;

	case C_BLOCK:
	  if (strcmp (name, ".bb") == 0)
	    {
	      if (! debug_start_block (dhandle, bfd_asymbol_value (sym)))
		return false;
	    }
	  else if (strcmp (name, ".eb") == 0)
	    {
	      if (! debug_end_block (dhandle, bfd_asymbol_value (sym)))
		return false;
	    }
	  break;

	default:
	  type = parse_coff_type (abfd, &symbols, &types, this_coff_symno,
				  syment.n_type, paux, true, dhandle);
	  if (type == DEBUG_TYPE_NULL)
	    return false;
	  if (! parse_coff_symbol (abfd, &types, sym, this_coff_symno, &syment,
				   dhandle, type, within_function))
	    return false;
	  break;
	}
    }

  return true;
}
