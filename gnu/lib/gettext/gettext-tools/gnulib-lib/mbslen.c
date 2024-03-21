/* Counting the multibyte characters in a string.
   Copyright (C) 2007-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2007.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation, either version 3 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#include <config.h>

/* Specification.  */
#include <string.h>

#include <stdlib.h>

#if GNULIB_MCEL_PREFER
# include "mcel.h"
#else
# include "mbuiterf.h"
#endif

/* Return the number of multibyte characters in the character string STRING.  */
size_t
mbslen (const char *string)
{
  if (MB_CUR_MAX > 1)
    {
      size_t count = 0;

#if GNULIB_MCEL_PREFER
      for (; *string; string += mcel_scanz (string).len)
        count++;
#else
      mbuif_state_t state;
      const char *iter;
      for (mbuif_init (state), iter = string; mbuif_avail (state, iter); )
        {
          mbchar_t cur = mbuif_next (state, iter);
          count++;
          iter += mb_len (cur);
        }
#endif

      return count;
    }
  else
    return strlen (string);
}
