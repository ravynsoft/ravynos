/* unlinkdir.c - determine whether we can unlink directories

   Copyright (C) 2005-2006, 2009-2023 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Paul Eggert, Jim Meyering, and David Bartley.  */

#include <config.h>

#include "unlinkdir.h"
#include "priv-set.h"
#include "root-uid.h"
#include <unistd.h>

#if ! UNLINK_CANNOT_UNLINK_DIR

/* Return true if we cannot unlink directories, false if we might be
   able to unlink directories.  */

bool
cannot_unlink_dir (void)
{
  static bool initialized;
  static bool cannot;

  if (! initialized)
    {
# if defined PRIV_SYS_LINKDIR
      /* We might be able to unlink directories if we cannot
         determine our privileges, or if we have the
         PRIV_SYS_LINKDIR privilege.  */
      cannot = (priv_set_ismember (PRIV_SYS_LINKDIR) == 0);
# else
      /* In traditional Unix, only root can unlink directories.  */
      cannot = (geteuid () != ROOT_UID);
# endif
      initialized = true;
    }

  return cannot;
}

#endif
