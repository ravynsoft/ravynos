/* Reference-counted string list.
   Copyright (C) 2001-2018 Free Software Foundation, Inc.

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

#ifndef _RC_STR_LIST_H
#define _RC_STR_LIST_H

#include <stdlib.h>

#include "str-list.h"

#ifdef __cplusplus
extern "C" {
#endif


/* A reference-counted list-of-immutable-strings type.  */

typedef struct refcounted_string_list_ty refcounted_string_list_ty;
struct refcounted_string_list_ty
{
  unsigned int refcount;
  struct string_list_ty contents;
};

static inline refcounted_string_list_ty *
add_reference (refcounted_string_list_ty *rslp)
{
  if (rslp != NULL)
    rslp->refcount++;
  return rslp;
}

static inline void
drop_reference (refcounted_string_list_ty *rslp)
{
  if (rslp != NULL)
    {
      if (rslp->refcount > 1)
        rslp->refcount--;
      else
        {
          string_list_destroy (&rslp->contents);
          free (rslp);
        }
    }
}


#ifdef __cplusplus
}
#endif


#endif /* _RC_STR_LIST_H */
