/* xgettext Scheme backend.
   Copyright (C) 2004, 2006, 2014, 2018, 2020 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2004.

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


#define EXTENSIONS_SCHEME \
  { "scm",       "Scheme"     },                                        \

#define SCANNERS_SCHEME \
  { "Scheme",           extract_scheme, NULL,                             \
                        &flag_table_scheme, &formatstring_scheme, NULL }, \

/* Scan a Scheme file and add its translatable strings to mdlp.  */
extern void extract_scheme (FILE *fp, const char *real_filename,
                            const char *logical_filename,
                            flag_context_list_table_ty *flag_table,
                            msgdomain_list_ty *mdlp);


/* Handling of options specific to this language.  */

extern void x_scheme_extract_all (void);
extern void x_scheme_keyword (const char *name);

extern void init_flag_table_scheme (void);


#ifdef __cplusplus
}
#endif
