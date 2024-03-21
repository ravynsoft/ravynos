/* Copy access control list from one file to file.  -*- coding: utf-8 -*-

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

   Written by Paul Eggert, Andreas Grünbacher, and Bruno Haible.  */

#include <config.h>

#include "acl.h"

#include <errno.h>

#include "quote.h"
#include "error.h"
#include "gettext.h"
#define _(msgid) gettext (msgid)


/* Copy access control lists from one file to another. If SOURCE_DESC is
   a valid file descriptor, use file descriptor operations, else use
   filename based operations on SRC_NAME. Likewise for DEST_DESC and
   DST_NAME.
   If access control lists are not available, fchmod the target file to
   MODE.  Also sets the non-permission bits of the destination file
   (S_ISUID, S_ISGID, S_ISVTX) to those from MODE if any are set.
   Return 0 if successful, otherwise output a diagnostic and return a
   negative error code.  */

int
copy_acl (const char *src_name, int source_desc, const char *dst_name,
          int dest_desc, mode_t mode)
{
  int ret = qcopy_acl (src_name, source_desc, dst_name, dest_desc, mode);
  switch (ret)
    {
    case -2:
      error (0, errno, "%s", quote (src_name));
      break;

    case -1:
      error (0, errno, _("preserving permissions for %s"), quote (dst_name));
      break;

    default:
      break;
    }
  return ret;
}
