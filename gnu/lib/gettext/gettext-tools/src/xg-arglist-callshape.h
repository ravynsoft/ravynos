/* Resolving ambiguity of argument lists: Information given through
   command-line options.
   Copyright (C) 2001-2018, 2020 Free Software Foundation, Inc.

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

#ifndef _XGETTEXT_ARGLIST_CALLSHAPE_H
#define _XGETTEXT_ARGLIST_CALLSHAPE_H

#include <stdbool.h>
#include <stddef.h>

#include "str-list.h"
#include "mem-hash-map.h"

#ifdef __cplusplus
extern "C" {
#endif


/* Calling convention for a given keyword.  */
struct callshape
{
  int argnum1; /* argument number to use for msgid */
  int argnum2; /* argument number to use for msgid_plural */
  int argnumc; /* argument number to use for msgctxt */
  bool argnum1_glib_context; /* argument argnum1 has the syntax "ctxt|msgid" */
  bool argnum2_glib_context; /* argument argnum2 has the syntax "ctxt|msgid" */
  int argtotal; /* total number of arguments */
  string_list_ty xcomments; /* auto-extracted comments */
};

/* Split keyword spec into keyword, argnum1, argnum2, argnumc.  */
extern void split_keywordspec (const char *spec, const char **endp,
                               struct callshape *shapep);

/* Set of alternative calling conventions for a given keyword.  */
struct callshapes
{
  const char *keyword;          /* the keyword, not NUL terminated */
  size_t keyword_len;           /* the keyword's length */
  size_t nshapes;
  struct callshape shapes[1];   /* actually nshapes elements */
};

/* Insert a (keyword, callshape) pair into a hash table mapping keyword to
   'struct callshapes *'.  */
extern void insert_keyword_callshape (hash_table *table,
                                      const char *keyword, size_t keyword_len,
                                      const struct callshape *shape);


#ifdef __cplusplus
}
#endif


#endif /* _XGETTEXT_ARGLIST_CALLSHAPE_H */
