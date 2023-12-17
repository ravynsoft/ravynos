/* Copy access control list from one file to another.  -*- coding: utf-8 -*-

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

#include "acl-internal.h"

#if USE_XATTR

# include <attr/libattr.h>

/* Returns 1 if NAME is the name of an extended attribute that is related
   to permissions, i.e. ACLs.  Returns 0 otherwise.  */

static int
is_attr_permissions (const char *name, struct error_context *ctx)
{
  return attr_copy_action (name, ctx) == ATTR_ACTION_PERMISSIONS;
}

#endif  /* USE_XATTR */

/* Copy access control lists from one file to another. If SOURCE_DESC is
   a valid file descriptor, use file descriptor operations, else use
   filename based operations on SRC_NAME. Likewise for DEST_DESC and
   DST_NAME.
   If access control lists are not available, fchmod the target file to
   MODE.  Also sets the non-permission bits of the destination file
   (S_ISUID, S_ISGID, S_ISVTX) to those from MODE if any are set.
   Return 0 if successful.
   Return -2 and set errno for an error relating to the source file.
   Return -1 and set errno for an error relating to the destination file.  */

int
qcopy_acl (const char *src_name, int source_desc, const char *dst_name,
           int dest_desc, mode_t mode)
{
  int ret;

#ifdef USE_XATTR
  /* in case no ACLs present and also to set higher mode bits
     we chmod before setting ACLs as doing it after could overwrite them
     (especially true for NFSv4, posix ACL has that ugly "mask" hack that
     nobody understands) */
  ret = chmod_or_fchmod (dst_name, dest_desc, mode);
  /* Rather than fiddling with acls one by one, we just copy the whole ACL xattrs
     (Posix or NFSv4). Of course, that won't address ACLs conversion
     (i.e. posix <-> nfs4) but we can't do it anyway, so for now, we don't care
     Functions attr_copy_* return 0 in case we copied something OR nothing
     to copy */
  if (ret == 0)
    ret = source_desc <= 0 || dest_desc <= 0
      ? attr_copy_file (src_name, dst_name, is_attr_permissions, NULL)
      : attr_copy_fd (src_name, source_desc, dst_name, dest_desc,
                      is_attr_permissions, NULL);
#else
  /* no XATTR, so we proceed the old dusty way */
  struct permission_context ctx;

  ret = get_permissions (src_name, source_desc, mode, &ctx);
  if (ret != 0)
    return -2;
  ret = set_permissions (&ctx, dst_name, dest_desc);
  free_permission_context (&ctx);
#endif
  return ret;
}
