/* Return the number of entries in an ACL.

   Copyright (C) 2002-2003, 2005-2023 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.

   Written by Paul Eggert and Andreas Gruenbacher.  */

#include <config.h>

#include "acl-internal.h"

/* This file assumes POSIX-draft like ACLs
   (Linux, FreeBSD, Mac OS X, IRIX, Tru64, Cygwin >= 2.5).  */

/* Return the number of entries in ACL.
   Return -1 and set errno upon failure to determine it.  */

int
acl_entries (acl_t acl)
{
  int count = 0;

  if (acl != NULL)
    {
#if HAVE_ACL_FIRST_ENTRY /* Linux, FreeBSD, Mac OS X, Cygwin >= 2.5 */
# if HAVE_ACL_TYPE_EXTENDED /* Mac OS X */
      /* acl_get_entry returns 0 when it successfully fetches an entry,
         and -1/EINVAL at the end.  */
      acl_entry_t ace;
      int got_one;

      for (got_one = acl_get_entry (acl, ACL_FIRST_ENTRY, &ace);
           got_one >= 0;
           got_one = acl_get_entry (acl, ACL_NEXT_ENTRY, &ace))
        count++;
# else /* Linux, FreeBSD, Cygwin >= 2.5 */
      /* acl_get_entry returns 1 when it successfully fetches an entry,
         and 0 at the end.  */
      acl_entry_t ace;
      int got_one;

      for (got_one = acl_get_entry (acl, ACL_FIRST_ENTRY, &ace);
           got_one > 0;
           got_one = acl_get_entry (acl, ACL_NEXT_ENTRY, &ace))
        count++;
      if (got_one < 0)
        return -1;
# endif
#else /* IRIX, Tru64 */
# if HAVE_ACL_TO_SHORT_TEXT /* IRIX */
      /* Don't use acl_get_entry: it is undocumented.  */
      count = acl->acl_cnt;
# endif
# if HAVE_ACL_FREE_TEXT /* Tru64 */
      /* Don't use acl_get_entry: it takes only one argument and does not
         work.  */
      count = acl->acl_num;
# endif
#endif
    }

  return count;
}
