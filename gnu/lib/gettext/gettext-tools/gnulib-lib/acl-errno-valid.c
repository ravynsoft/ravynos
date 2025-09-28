/* Test whether ACLs are well supported on this system.

   Copyright 2013-2023 Free Software Foundation, Inc.

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

   Written by Paul Eggert.  */

#include <config.h>

#include <acl.h>

#include <errno.h>

/* Return true if errno value ERRNUM indicates that ACLs are well
   supported on this system.  ERRNUM should be an errno value obtained
   after an ACL-related system call fails.  */
bool
acl_errno_valid (int errnum)
{
  /* Recognize some common errors such as from an NFS mount that does
     not support ACLs, even when local drives do.  */
  switch (errnum)
    {
    case EBUSY: return false;
    case EINVAL: return false;
#if defined __APPLE__ && defined __MACH__
    case ENOENT: return false;
#endif
    case ENOSYS: return false;

#if defined ENOTSUP && ENOTSUP != EOPNOTSUPP
# if ENOTSUP != ENOSYS /* Needed for the MS-Windows port of GNU Emacs.  */
    case ENOTSUP: return false;
# endif
#endif

    case EOPNOTSUPP: return false;
    default: return true;
    }
}
