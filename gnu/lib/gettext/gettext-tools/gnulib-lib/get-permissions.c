/* Get permissions of a file.  -*- coding: utf-8 -*-

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

#include <string.h>
#include "acl.h"

#include "acl-internal.h"

/* Read the permissions of a file into CTX. If DESC is a valid file descriptor,
   use file descriptor operations, else use filename based operations on NAME.
   MODE is the file mode obtained from a previous stat call.
   Return 0 if successful.  Return -1 and set errno upon failure.  */

int
get_permissions (const char *name, int desc, mode_t mode,
                 struct permission_context *ctx)
{
  memset (ctx, 0, sizeof *ctx);
  ctx->mode = mode;

#if USE_ACL && HAVE_ACL_GET_FILE
  /* POSIX 1003.1e (draft 17 -- abandoned) specific version.  */
  /* Linux, FreeBSD, Mac OS X, IRIX, Tru64, Cygwin >= 2.5 */
# if !HAVE_ACL_TYPE_EXTENDED
  /* Linux, FreeBSD, IRIX, Tru64, Cygwin >= 2.5 */

  if (HAVE_ACL_GET_FD && desc != -1)
    ctx->acl = acl_get_fd (desc);
  else
    ctx->acl = acl_get_file (name, ACL_TYPE_ACCESS);
  if (ctx->acl == NULL)
    return acl_errno_valid (errno) ? -1 : 0;

  /* With POSIX ACLs, a file cannot have "no" acl; a file without
     extended permissions has a "minimal" acl which is equivalent to the
     file mode.  */

  if (S_ISDIR (mode))
    {
      ctx->default_acl = acl_get_file (name, ACL_TYPE_DEFAULT);
      if (ctx->default_acl == NULL)
        return -1;
    }

#  if HAVE_ACL_TYPE_NFS4  /* FreeBSD */

  /* TODO (see set_permissions). */

#  endif

# else /* HAVE_ACL_TYPE_EXTENDED */
  /* Mac OS X */

  /* On Mac OS X,  acl_get_file (name, ACL_TYPE_ACCESS)
     and           acl_get_file (name, ACL_TYPE_DEFAULT)
     always return NULL / EINVAL.  You have to use
                   acl_get_file (name, ACL_TYPE_EXTENDED)
     or            acl_get_fd (open (name, ...))
     to retrieve an ACL.
     On the other hand,
                   acl_set_file (name, ACL_TYPE_ACCESS, acl)
     and           acl_set_file (name, ACL_TYPE_DEFAULT, acl)
     have the same effect as
                   acl_set_file (name, ACL_TYPE_EXTENDED, acl):
     Each of these calls sets the file's ACL.  */

  if (HAVE_ACL_GET_FD && desc != -1)
    ctx->acl = acl_get_fd (desc);
  else
    ctx->acl = acl_get_file (name, ACL_TYPE_EXTENDED);
  if (ctx->acl == NULL)
    return acl_errno_valid (errno) ? -1 : 0;

# endif

#elif USE_ACL && defined GETACL /* Solaris, Cygwin, not HP-UX */

  /* Solaris 2.5 through Solaris 10, Cygwin, and contemporaneous versions
     of Unixware.  The acl() call returns the access and default ACL both
     at once.  */
# ifdef ACE_GETACL
  /* Solaris also has a different variant of ACLs, used in ZFS and NFSv4
     file systems (whereas the other ones are used in UFS file systems).
     There is an API
       pathconf (name, _PC_ACL_ENABLED)
       fpathconf (desc, _PC_ACL_ENABLED)
     that allows us to determine which of the two kinds of ACLs is supported
     for the given file.  But some file systems may implement this call
     incorrectly, so better not use it.
     When fetching the source ACL, we simply fetch both ACL types.
     When setting the destination ACL, we try either ACL types, assuming
     that the kernel will translate the ACL from one form to the other.
     (See in <https://docs.oracle.com/cd/E86824_01/html/E54765/acl-2.html>
     the description of ENOTSUP.)  */
  for (;;)
    {
      int ret;

      if (desc != -1)
        ret = facl (desc, ACE_GETACLCNT, 0, NULL);
      else
        ret = acl (name, ACE_GETACLCNT, 0, NULL);
      if (ret < 0)
        {
          if (errno == ENOSYS || errno == EINVAL)
            ret = 0;
          else
            return -1;
        }
      ctx->ace_count = ret;

      if (ctx->ace_count == 0)
        break;

      ctx->ace_entries = (ace_t *) malloc (ctx->ace_count * sizeof (ace_t));
      if (ctx->ace_entries == NULL)
        {
          errno = ENOMEM;
          return -1;
        }

      if (desc != -1)
        ret = facl (desc, ACE_GETACL, ctx->ace_count, ctx->ace_entries);
      else
        ret = acl (name, ACE_GETACL, ctx->ace_count, ctx->ace_entries);
      if (ret < 0)
        {
          if (errno == ENOSYS || errno == EINVAL)
            {
              free (ctx->ace_entries);
              ctx->ace_entries = NULL;
              ctx->ace_count = 0;
              break;
            }
          else
            return -1;
        }
      if (ret <= ctx->ace_count)
        {
          ctx->ace_count = ret;
          break;
        }
      /* Huh? The number of ACL entries has increased since the last call.
         Repeat.  */
      free (ctx->ace_entries);
      ctx->ace_entries = NULL;
    }
# endif

  for (;;)
    {
      int ret;

      if (desc != -1)
        ret = facl (desc, GETACLCNT, 0, NULL);
      else
        ret = acl (name, GETACLCNT, 0, NULL);
      if (ret < 0)
        {
          if (errno == ENOSYS || errno == ENOTSUP || errno == EOPNOTSUPP)
            ret = 0;
          else
            return -1;
        }
      ctx->count = ret;

      if (ctx->count == 0)
        break;

      ctx->entries = (aclent_t *) malloc (ctx->count * sizeof (aclent_t));
      if (ctx->entries == NULL)
        {
          errno = ENOMEM;
          return -1;
        }

      if (desc != -1)
        ret = facl (desc, GETACL, ctx->count, ctx->entries);
      else
        ret = acl (name, GETACL, ctx->count, ctx->entries);
      if (ret < 0)
        {
          if (errno == ENOSYS || errno == ENOTSUP || errno == EOPNOTSUPP)
            {
              free (ctx->entries);
              ctx->entries = NULL;
              ctx->count = 0;
              break;
            }
          else
            return -1;
        }
      if (ret <= ctx->count)
        {
          ctx->count = ret;
          break;
        }
      /* Huh? The number of ACL entries has increased since the last call.
         Repeat.  */
      free (ctx->entries);
      ctx->entries = NULL;
    }

#elif USE_ACL && HAVE_GETACL /* HP-UX */

  {
    int ret;

    if (desc != -1)
      ret = fgetacl (desc, NACLENTRIES, ctx->entries);
    else
      ret = getacl (name, NACLENTRIES, ctx->entries);
    if (ret < 0)
      {
        if (errno == ENOSYS || errno == EOPNOTSUPP || errno == ENOTSUP)
          ret = 0;
        else
          return -1;
      }
    else if (ret > NACLENTRIES)
      /* If NACLENTRIES cannot be trusted, use dynamic memory allocation.  */
      abort ();
    ctx->count = ret;

# if HAVE_ACLV_H
    ret = acl ((char *) name, ACL_GET, NACLVENTRIES, ctx->aclv_entries);
    if (ret < 0)
      {
        if (errno == ENOSYS || errno == EOPNOTSUPP || errno == EINVAL)
          ret = 0;
        else
          return -2;
      }
    else if (ret > NACLVENTRIES)
      /* If NACLVENTRIES cannot be trusted, use dynamic memory allocation.  */
      abort ();
    ctx->aclv_count = ret;
# endif
  }

#elif USE_ACL && HAVE_ACLX_GET && ACL_AIX_WIP /* AIX */

  /* TODO (see set_permissions). */

#elif USE_ACL && HAVE_STATACL /* older AIX */

  {
    int ret;
    if (desc != -1)
      ret = fstatacl (desc, STX_NORMAL, &ctx->u.a, sizeof ctx->u);
    else
      ret = statacl ((char *) name, STX_NORMAL, &ctx->u.a, sizeof ctx->u);
    if (ret == 0)
      ctx->have_u = true;
  }

#elif USE_ACL && HAVE_ACLSORT /* NonStop Kernel */

  {
    int ret = acl ((char *) name, ACL_GET, NACLENTRIES, ctx->entries);
    if (ret < 0)
      return -1;
    else if (ret > NACLENTRIES)
      /* If NACLENTRIES cannot be trusted, use dynamic memory allocation.  */
      abort ();
    ctx->count = ret;
  }

#endif

  return 0;

}
