/* BFD COFF interfaces used outside of BFD.
   Copyright (C) 1990-2023 Free Software Foundation, Inc.
   Written by Cygnus Support.

   This file is part of BFD, the Binary File Descriptor library.

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

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "coff/internal.h"
#include "libcoff.h"

/* Return the COFF syment for a symbol.  */

bool
bfd_coff_get_syment (bfd *abfd,
		     asymbol *symbol,
		     struct internal_syment *psyment)
{
  coff_symbol_type *csym;

  csym = coff_symbol_from (symbol);
  if (csym == NULL || csym->native == NULL
      || ! csym->native->is_sym)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return false;
    }

  *psyment = csym->native->u.syment;

  if (csym->native->fix_value)
    {
      psyment->n_value =
	((psyment->n_value - (uintptr_t) obj_raw_syments (abfd))
	 / sizeof (combined_entry_type));
      csym->native->fix_value = 0;
    }

  /* FIXME: We should handle fix_line here.  */

  return true;
}

/* Return the COFF auxent for a symbol.  */

bool
bfd_coff_get_auxent (bfd *abfd,
		     asymbol *symbol,
		     int indx,
		     union internal_auxent *pauxent)
{
  coff_symbol_type *csym;
  combined_entry_type *ent;

  csym = coff_symbol_from (symbol);

  if (csym == NULL
      || csym->native == NULL
      || ! csym->native->is_sym
      || indx >= csym->native->u.syment.n_numaux)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return false;
    }

  ent = csym->native + indx + 1;

  BFD_ASSERT (! ent->is_sym);
  *pauxent = ent->u.auxent;

  if (ent->fix_tag)
    {
      pauxent->x_sym.x_tagndx.u32 =
	((combined_entry_type *) pauxent->x_sym.x_tagndx.p
	 - obj_raw_syments (abfd));
      ent->fix_tag = 0;
    }

  if (ent->fix_end)
    {
      pauxent->x_sym.x_fcnary.x_fcn.x_endndx.u32 =
	((combined_entry_type *) pauxent->x_sym.x_fcnary.x_fcn.x_endndx.p
	 - obj_raw_syments (abfd));
      ent->fix_end = 0;
    }

  if (ent->fix_scnlen)
    {
      pauxent->x_csect.x_scnlen.u64 =
	((combined_entry_type *) pauxent->x_csect.x_scnlen.p
	 - obj_raw_syments (abfd));
      ent->fix_scnlen = 0;
    }

  return true;
}
