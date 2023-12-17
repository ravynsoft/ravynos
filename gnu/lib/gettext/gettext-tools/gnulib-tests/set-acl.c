/* set-acl.c - set access control list equivalent to a mode

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

   Written by Paul Eggert and Andreas Gruenbacher, and Bruno Haible.  */

#include <config.h>

#include "acl.h"

#include <errno.h>

#include "quote.h"
#include "error.h"
#include "gettext.h"
#define _(msgid) gettext (msgid)

/* Set the access control lists of a file to match *exactly* MODE (this might
   remove inherited ACLs). Note chmod() tends to honor inherited/default
   ACLs. If DESC is a valid file descriptor, use file descriptor operations
   where available, else use filename based operations on NAME.  If access
   control lists are not available, fchmod the target file to MODE.  Also
   sets the non-permission bits of the destination file
   (S_ISUID, S_ISGID, S_ISVTX) to those from MODE if any are set.
    Return 0 if successful.  On failure, output a diagnostic, set errno and
    return -1.  */

int
set_acl (char const *name, int desc, mode_t mode)
{
  int ret = qset_acl (name, desc, mode);
  if (ret != 0)
    error (0, errno, _("setting permissions for %s"), quote (name));
  return ret;
}
