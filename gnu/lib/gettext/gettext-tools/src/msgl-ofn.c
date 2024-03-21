/* Message list test for ordinary file names.
   Copyright (C) 2021 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <bruno@clisp.org>, 2021.  */


#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

/* Specification.  */
#include "msgl-ofn.h"

#include "pos.h"


bool
message_has_filenames_with_spaces (message_ty *mp)
{
  size_t n = mp->filepos_count;
  size_t i;

  for (i = 0; i < n; i++)
    if (pos_filename_has_spaces (&mp->filepos[i]))
      return true;

  return false;
}

bool
message_list_has_filenames_with_spaces (message_list_ty *mlp)
{
  size_t j;

  for (j = 0; j < mlp->nitems; j++)
    if (message_has_filenames_with_spaces (mlp->item[j]))
      return true;

  return false;
}

bool
msgdomain_list_has_filenames_with_spaces (msgdomain_list_ty *mdlp)
{
  size_t k;

  for (k = 0; k < mdlp->nitems; k++)
    if (message_list_has_filenames_with_spaces (mdlp->item[k]->messages))
      return true;

  return false;
}
