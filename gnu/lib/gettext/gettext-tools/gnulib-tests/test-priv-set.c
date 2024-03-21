/* Test the priv-set module.
   Copyright (C) 2009-2023 Free Software Foundation, Inc.

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

/* Written by David Bartley <dtbartle@csclub.uwaterloo.ca>, 2007.  */

#include <config.h>

#include "priv-set.h"

#if HAVE_GETPPRIV && HAVE_PRIV_H
# include <priv.h>
#endif
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>

#include "macros.h"

int
main (void)
{
#if HAVE_GETPPRIV && HAVE_PRIV_H
    priv_set_t *set;

    ASSERT (set = priv_allocset ());
    ASSERT (getppriv (PRIV_EFFECTIVE, set) == 0);
    ASSERT (priv_ismember (set, PRIV_PROC_EXEC) == 1);

    /* Do a series of removes and restores making sure that the results are
       consistent with our ismember function and solaris' priv_ismember.  */
    ASSERT (priv_set_ismember (PRIV_PROC_EXEC) == 1);
        ASSERT (getppriv (PRIV_EFFECTIVE, set) == 0);
        ASSERT (priv_ismember (set, PRIV_PROC_EXEC) == 1);
    ASSERT (priv_set_restore (PRIV_PROC_EXEC) == -1);
        ASSERT (errno == EINVAL);
    ASSERT (priv_set_ismember (PRIV_PROC_EXEC) == 1);
        ASSERT (getppriv (PRIV_EFFECTIVE, set) == 0);
        ASSERT (priv_ismember (set, PRIV_PROC_EXEC) == 1);
    ASSERT (priv_set_remove (PRIV_PROC_EXEC) == 0);
    ASSERT (priv_set_ismember (PRIV_PROC_EXEC) == 0);
        ASSERT (getppriv (PRIV_EFFECTIVE, set) == 0);
        ASSERT (priv_ismember (set, PRIV_PROC_EXEC) == 0);
    ASSERT (priv_set_remove (PRIV_PROC_EXEC) == -1);
        ASSERT (errno == EINVAL);
    ASSERT (priv_set_ismember (PRIV_PROC_EXEC) == 0);
        ASSERT (getppriv (PRIV_EFFECTIVE, set) == 0);
        ASSERT (priv_ismember (set, PRIV_PROC_EXEC) == 0);
    ASSERT (priv_set_restore (PRIV_PROC_EXEC) == 0);
    ASSERT (priv_set_ismember (PRIV_PROC_EXEC) == 1);
        ASSERT (getppriv (PRIV_EFFECTIVE, set) == 0);
        ASSERT (priv_ismember (set, PRIV_PROC_EXEC) == 1);
    ASSERT (priv_set_restore (PRIV_PROC_EXEC) == -1);
        ASSERT (errno == EINVAL);
    ASSERT (priv_set_ismember (PRIV_PROC_EXEC) == 1);
        ASSERT (getppriv (PRIV_EFFECTIVE, set) == 0);
        ASSERT (priv_ismember (set, PRIV_PROC_EXEC) == 1);

    /* Test the priv_set_linkdir wrappers.  */
    ASSERT (getppriv (PRIV_EFFECTIVE, set) == 0);
    if (priv_ismember (set, PRIV_SYS_LINKDIR))
      {
        ASSERT (priv_set_restore_linkdir () == -1);
            ASSERT (errno == EINVAL);
        ASSERT (priv_set_remove_linkdir () == 0);
        ASSERT (priv_set_remove_linkdir () == -1);
            ASSERT (errno == EINVAL);
        ASSERT (priv_set_restore_linkdir () == 0);
      }
#else
    ASSERT (priv_set_restore_linkdir () == -1);
    ASSERT (priv_set_remove_linkdir () == -1);
#endif

    return 0;
}
