/* Reading PO files.
   Copyright (C) 1995-1996, 1998, 2000-2006 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/* Specification.  */
#include "read-po.h"

#include "po-lex.h"
#include "po-gram.h"

/* For compiling this file in C++ mode.  */
#ifdef __cplusplus
# define this thiss
#endif


/* Read a .po / .pot file from a stream, and dispatch to the various
   abstract_catalog_reader_class_ty methods.  */
static void
po_parse (abstract_catalog_reader_ty *this, FILE *fp,
          const char *real_filename, const char *logical_filename)
{
  lex_start (fp, real_filename, logical_filename);
  po_gram_parse ();
  lex_end ();
}

const struct catalog_input_format input_format_po =
{
  po_parse,                             /* parse */
  false                                 /* produces_utf8 */
};
