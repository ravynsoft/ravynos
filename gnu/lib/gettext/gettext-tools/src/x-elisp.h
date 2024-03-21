/* xgettext Emacs Lisp backend.
   Copyright (C) 2002-2003, 2006, 2014, 2018, 2020 Free Software Foundation, Inc.
   Written by Bruno Haible <haible@clisp.cons.org>, 2002.

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


#include <stdio.h>

#include "message.h"
#include "xg-arglist-context.h"


#ifdef __cplusplus
extern "C" {
#endif


#define EXTENSIONS_ELISP \
  { "el",        "EmacsLisp"     },                                     \

#define SCANNERS_ELISP \
  { "EmacsLisp",        extract_elisp, NULL,                            \
                        &flag_table_elisp, &formatstring_elisp, NULL }, \

/* Scan an Emacs Lisp file and add its translatable strings to mdlp.  */
extern void extract_elisp (FILE *fp, const char *real_filename,
                           const char *logical_filename,
                           flag_context_list_table_ty *flag_table,
                           msgdomain_list_ty *mdlp);


/* Handling of options specific to this language.  */

extern void x_elisp_extract_all (void);
extern void x_elisp_keyword (const char *name);

extern void init_flag_table_elisp (void);


#ifdef __cplusplus
}
#endif
