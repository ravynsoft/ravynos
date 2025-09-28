/* Test whether a file has a nontrivial ACL.  -*- coding: utf-8 -*-

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

#if USE_ACL && HAVE_ACL_GET_FILE /* Linux, FreeBSD, Mac OS X, IRIX, Tru64, Cygwin >= 2.5 */

# if HAVE_ACL_TYPE_EXTENDED /* Mac OS X */

/* ACL is an ACL, from a file, stored as type ACL_TYPE_EXTENDED.
   Return 1 if the given ACL is non-trivial.
   Return 0 if it is trivial.  */
int
acl_extended_nontrivial (acl_t acl)
{
  /* acl is non-trivial if it is non-empty.  */
  return (acl_entries (acl) > 0);
}

# else /* Linux, FreeBSD, IRIX, Tru64, Cygwin >= 2.5 */

/* ACL is an ACL, from a file, stored as type ACL_TYPE_ACCESS.
   Return 1 if the given ACL is non-trivial.
   Return 0 if it is trivial, i.e. equivalent to a simple stat() mode.
   Return -1 and set errno upon failure to determine it.  */
int
acl_access_nontrivial (acl_t acl)
{
  /* acl is non-trivial if it has some entries other than for "user::",
     "group::", and "other::".  Normally these three should be present
     at least, allowing us to write
        return (3 < acl_entries (acl));
     but the following code is more robust.  */
#  if HAVE_ACL_FIRST_ENTRY /* Linux, FreeBSD, Cygwin >= 2.5 */

  acl_entry_t ace;
  int got_one;

  for (got_one = acl_get_entry (acl, ACL_FIRST_ENTRY, &ace);
       got_one > 0;
       got_one = acl_get_entry (acl, ACL_NEXT_ENTRY, &ace))
    {
      acl_tag_t tag;
      if (acl_get_tag_type (ace, &tag) < 0)
        return -1;
      if (!(tag == ACL_USER_OBJ || tag == ACL_GROUP_OBJ || tag == ACL_OTHER))
        return 1;
    }
  return got_one;

#  elif HAVE_ACL_TO_SHORT_TEXT /* IRIX */
  /* Don't use acl_get_entry: it is undocumented.  */

  int count = acl->acl_cnt;
  int i;

  for (i = 0; i < count; i++)
    {
      acl_entry_t ace = &acl->acl_entry[i];
      acl_tag_t tag = ace->ae_tag;

      if (!(tag == ACL_USER_OBJ || tag == ACL_GROUP_OBJ
            || tag == ACL_OTHER_OBJ))
        return 1;
    }
  return 0;

#  elif HAVE_ACL_FREE_TEXT /* Tru64 */
  /* Don't use acl_get_entry: it takes only one argument and does not work.  */

  int count = acl->acl_num;
  acl_entry_t ace;

  for (ace = acl->acl_first; count > 0; ace = ace->next, count--)
    {
      acl_tag_t tag;
      acl_perm_t perm;

      tag = ace->entry->acl_type;
      if (!(tag == ACL_USER_OBJ || tag == ACL_GROUP_OBJ || tag == ACL_OTHER))
        return 1;

      perm = ace->entry->acl_perm;
      /* On Tru64, perm can also contain non-standard bits such as
         PERM_INSERT, PERM_DELETE, PERM_MODIFY, PERM_LOOKUP, ... */
      if ((perm & ~(ACL_READ | ACL_WRITE | ACL_EXECUTE)) != 0)
        return 1;
    }
  return 0;

#  else

  errno = ENOSYS;
  return -1;
#  endif
}

int
acl_default_nontrivial (acl_t acl)
{
  /* acl is non-trivial if it is non-empty.  */
  return (acl_entries (acl) > 0);
}

# endif

#elif USE_ACL && HAVE_FACL && defined GETACL /* Solaris, Cygwin < 2.5, not HP-UX */

/* Test an ACL retrieved with GETACL.
   Return 1 if the given ACL, consisting of COUNT entries, is non-trivial.
   Return 0 if it is trivial, i.e. equivalent to a simple stat() mode.  */
int
acl_nontrivial (int count, aclent_t *entries)
{
  int i;

  for (i = 0; i < count; i++)
    {
      aclent_t *ace = &entries[i];

      /* Note: If ace->a_type = USER_OBJ, ace->a_id is the st_uid from stat().
         If ace->a_type = GROUP_OBJ, ace->a_id is the st_gid from stat().
         We don't need to check ace->a_id in these cases.  */
      if (!(ace->a_type == USER_OBJ
            || ace->a_type == GROUP_OBJ
            || ace->a_type == OTHER_OBJ
            /* Note: Cygwin does not return a CLASS_OBJ ("mask:") entry
               sometimes.  */
            || ace->a_type == CLASS_OBJ))
        return 1;
    }
  return 0;
}

# ifdef ACE_GETACL

/* A shortcut for a bitmask.  */
#  define NEW_ACE_WRITEA_DATA (NEW_ACE_WRITE_DATA | NEW_ACE_APPEND_DATA)

/* Test an ACL retrieved with ACE_GETACL.
   Return 1 if the given ACL, consisting of COUNT entries, is non-trivial.
   Return 0 if it is trivial, i.e. equivalent to a simple stat() mode.  */
int
acl_ace_nontrivial (int count, ace_t *entries)
{
  int i;

  /* The flags in the ace_t structure changed in a binary incompatible way
     when ACL_NO_TRIVIAL etc. were introduced in <sys/acl.h> version 1.15.
     How to distinguish the two conventions at runtime?
     In the old convention, usually three ACEs have a_flags = ACE_OWNER /
     ACE_GROUP / ACE_OTHER, in the range 0x0100..0x0400.  In the new
     convention, these values are not used.  */
  int old_convention = 0;

  for (i = 0; i < count; i++)
    if (entries[i].a_flags & (OLD_ACE_OWNER | OLD_ACE_GROUP | OLD_ACE_OTHER))
      {
        old_convention = 1;
        break;
      }

  if (old_convention)
    /* Running on Solaris 10.  */
    for (i = 0; i < count; i++)
      {
        ace_t *ace = &entries[i];

        /* Note:
           If ace->a_flags = ACE_OWNER, ace->a_who is the st_uid from stat().
           If ace->a_flags = ACE_GROUP, ace->a_who is the st_gid from stat().
           We don't need to check ace->a_who in these cases.  */
        if (!(ace->a_type == OLD_ALLOW
              && (ace->a_flags == OLD_ACE_OWNER
                  || ace->a_flags == OLD_ACE_GROUP
                  || ace->a_flags == OLD_ACE_OTHER)))
          return 1;
      }
  else
    {
      /* Running on Solaris 10 (newer version) or Solaris 11.  */
      unsigned int access_masks[6] =
        {
          0, /* owner@ deny */
          0, /* owner@ allow */
          0, /* group@ deny */
          0, /* group@ allow */
          0, /* everyone@ deny */
          0  /* everyone@ allow */
        };

      for (i = 0; i < count; i++)
        {
          ace_t *ace = &entries[i];
          unsigned int index1;
          unsigned int index2;

          if (ace->a_type == NEW_ACE_ACCESS_ALLOWED_ACE_TYPE)
            index1 = 1;
          else if (ace->a_type == NEW_ACE_ACCESS_DENIED_ACE_TYPE)
            index1 = 0;
          else
            return 1;

          if (ace->a_flags == NEW_ACE_OWNER)
            index2 = 0;
          else if (ace->a_flags == (NEW_ACE_GROUP | NEW_ACE_IDENTIFIER_GROUP))
            index2 = 2;
          else if (ace->a_flags == NEW_ACE_EVERYONE)
            index2 = 4;
          else
            return 1;

          access_masks[index1 + index2] |= ace->a_access_mask;
        }

      /* The same bit shouldn't be both allowed and denied.  */
      if (access_masks[0] & access_masks[1])
        return 1;
      if (access_masks[2] & access_masks[3])
        return 1;
      if (access_masks[4] & access_masks[5])
        return 1;

      /* Check minimum masks.  */
      if ((NEW_ACE_WRITE_NAMED_ATTRS
           | NEW_ACE_WRITE_ATTRIBUTES
           | NEW_ACE_WRITE_ACL
           | NEW_ACE_WRITE_OWNER)
          & ~ access_masks[1])
        return 1;
      access_masks[1] &= ~(NEW_ACE_WRITE_NAMED_ATTRS
                           | NEW_ACE_WRITE_ATTRIBUTES
                           | NEW_ACE_WRITE_ACL
                           | NEW_ACE_WRITE_OWNER);
      if ((NEW_ACE_READ_NAMED_ATTRS
           | NEW_ACE_READ_ATTRIBUTES
           | NEW_ACE_READ_ACL
           | NEW_ACE_SYNCHRONIZE)
          & ~ access_masks[5])
        return 1;
      access_masks[5] &= ~(NEW_ACE_READ_NAMED_ATTRS
                           | NEW_ACE_READ_ATTRIBUTES
                           | NEW_ACE_READ_ACL
                           | NEW_ACE_SYNCHRONIZE);

      /* Check the allowed or denied bits.  */
      switch ((access_masks[0] | access_masks[1])
              & ~(NEW_ACE_READ_NAMED_ATTRS
                  | NEW_ACE_READ_ATTRIBUTES
                  | NEW_ACE_READ_ACL
                  | NEW_ACE_SYNCHRONIZE))
        {
        case 0:
        case NEW_ACE_READ_DATA:
        case                     NEW_ACE_WRITEA_DATA:
        case NEW_ACE_READ_DATA | NEW_ACE_WRITEA_DATA:
        case                                           NEW_ACE_EXECUTE:
        case NEW_ACE_READ_DATA |                       NEW_ACE_EXECUTE:
        case                     NEW_ACE_WRITEA_DATA | NEW_ACE_EXECUTE:
        case NEW_ACE_READ_DATA | NEW_ACE_WRITEA_DATA | NEW_ACE_EXECUTE:
          break;
        default:
          return 1;
        }
      switch ((access_masks[2] | access_masks[3])
              & ~(NEW_ACE_READ_NAMED_ATTRS
                  | NEW_ACE_READ_ATTRIBUTES
                  | NEW_ACE_READ_ACL
                  | NEW_ACE_SYNCHRONIZE))
        {
        case 0:
        case NEW_ACE_READ_DATA:
        case                     NEW_ACE_WRITEA_DATA:
        case NEW_ACE_READ_DATA | NEW_ACE_WRITEA_DATA:
        case                                           NEW_ACE_EXECUTE:
        case NEW_ACE_READ_DATA |                       NEW_ACE_EXECUTE:
        case                     NEW_ACE_WRITEA_DATA | NEW_ACE_EXECUTE:
        case NEW_ACE_READ_DATA | NEW_ACE_WRITEA_DATA | NEW_ACE_EXECUTE:
          break;
        default:
          return 1;
        }
      switch ((access_masks[4] | access_masks[5])
              & ~(NEW_ACE_WRITE_NAMED_ATTRS
                  | NEW_ACE_WRITE_ATTRIBUTES
                  | NEW_ACE_WRITE_ACL
                  | NEW_ACE_WRITE_OWNER))
        {
        case 0:
        case NEW_ACE_READ_DATA:
        case                     NEW_ACE_WRITEA_DATA:
        case NEW_ACE_READ_DATA | NEW_ACE_WRITEA_DATA:
        case                                           NEW_ACE_EXECUTE:
        case NEW_ACE_READ_DATA |                       NEW_ACE_EXECUTE:
        case                     NEW_ACE_WRITEA_DATA | NEW_ACE_EXECUTE:
        case NEW_ACE_READ_DATA | NEW_ACE_WRITEA_DATA | NEW_ACE_EXECUTE:
          break;
        default:
          return 1;
        }

      /* Check that the NEW_ACE_WRITE_DATA and NEW_ACE_APPEND_DATA bits are
         either both allowed or both denied.  */
      if (((access_masks[0] & NEW_ACE_WRITE_DATA) != 0)
          != ((access_masks[0] & NEW_ACE_APPEND_DATA) != 0))
        return 1;
      if (((access_masks[2] & NEW_ACE_WRITE_DATA) != 0)
          != ((access_masks[2] & NEW_ACE_APPEND_DATA) != 0))
        return 1;
      if (((access_masks[4] & NEW_ACE_WRITE_DATA) != 0)
          != ((access_masks[4] & NEW_ACE_APPEND_DATA) != 0))
        return 1;
    }

  return 0;
}

# endif

#elif USE_ACL && HAVE_GETACL /* HP-UX */

/* Return 1 if the given ACL is non-trivial.
   Return 0 if it is trivial, i.e. equivalent to a simple stat() mode.  */
int
acl_nontrivial (int count, struct acl_entry *entries)
{
  int i;

  if (count > 3)
    return 1;

  for (i = 0; i < count; i++)
    {
      struct acl_entry *ace = &entries[i];

      if (ace->uid != ACL_NSUSER && ace->gid != ACL_NSGROUP)
        return 1;
    }
  return 0;
}

# if HAVE_ACLV_H /* HP-UX >= 11.11 */

/* Return 1 if the given ACL is non-trivial.
   Return 0 if it is trivial, i.e. equivalent to a simple stat() mode.  */
int
aclv_nontrivial (int count, struct acl *entries)
{
  int i;

  for (i = 0; i < count; i++)
    {
      struct acl *ace = &entries[i];

      /* Note: If ace->a_type = USER_OBJ, ace->a_id is the st_uid from stat().
         If ace->a_type = GROUP_OBJ, ace->a_id is the st_gid from stat().
         We don't need to check ace->a_id in these cases.  */
      if (!(ace->a_type == USER_OBJ /* no need to check ace->a_id here */
            || ace->a_type == GROUP_OBJ /* no need to check ace->a_id here */
            || ace->a_type == CLASS_OBJ
            || ace->a_type == OTHER_OBJ))
        return 1;
    }
  return 0;
}

# endif

#elif USE_ACL && (HAVE_ACLX_GET || HAVE_STATACL) /* AIX */

/* Return 1 if the given ACL is non-trivial.
   Return 0 if it is trivial, i.e. equivalent to a simple stat() mode.  */
int
acl_nontrivial (struct acl *a)
{
  /* The normal way to iterate through an ACL is like this:
       struct acl_entry *ace;
       for (ace = a->acl_ext; ace != acl_last (a); ace = acl_nxt (ace))
         {
           struct ace_id *aei;
           switch (ace->ace_type)
             {
             case ACC_PERMIT:
             case ACC_DENY:
             case ACC_SPECIFY:
               ...;
             }
           for (aei = ace->ace_id; aei != id_last (ace); aei = id_nxt (aei))
             ...
         }
   */
  return (acl_last (a) != a->acl_ext ? 1 : 0);
}

# if HAVE_ACLX_GET && defined ACL_AIX_WIP /* newer AIX */

/* Return 1 if the given ACL is non-trivial.
   Return 0 if it is trivial, i.e. equivalent to a simple stat() mode.  */
int
acl_nfs4_nontrivial (nfs4_acl_int_t *a)
{
#  if 1 /* let's try this first */
  return (a->aclEntryN > 0 ? 1 : 0);
#  else
  int count = a->aclEntryN;
  int i;

  for (i = 0; i < count; i++)
    {
      nfs4_ace_int_t *ace = &a->aclEntry[i];

      if (!((ace->flags & ACE4_ID_SPECIAL) != 0
            && (ace->aceWho.special_whoid == ACE4_WHO_OWNER
                || ace->aceWho.special_whoid == ACE4_WHO_GROUP
                || ace->aceWho.special_whoid == ACE4_WHO_EVERYONE)
            && ace->aceType == ACE4_ACCESS_ALLOWED_ACE_TYPE
            && ace->aceFlags == 0
            && (ace->aceMask & ~(ACE4_READ_DATA | ACE4_LIST_DIRECTORY
                                 | ACE4_WRITE_DATA | ACE4_ADD_FILE
                                 | ACE4_EXECUTE)) == 0))
        return 1;
    }
  return 0;
#  endif
}

# endif

#elif USE_ACL && HAVE_ACLSORT /* NonStop Kernel */

/* Test an ACL retrieved with ACL_GET.
   Return 1 if the given ACL, consisting of COUNT entries, is non-trivial.
   Return 0 if it is trivial, i.e. equivalent to a simple stat() mode.  */
int
acl_nontrivial (int count, struct acl *entries)
{
  int i;

  for (i = 0; i < count; i++)
    {
      struct acl *ace = &entries[i];

      /* Note: If ace->a_type = USER_OBJ, ace->a_id is the st_uid from stat().
         If ace->a_type = GROUP_OBJ, ace->a_id is the st_gid from stat().
         We don't need to check ace->a_id in these cases.  */
      if (!(ace->a_type == USER_OBJ /* no need to check ace->a_id here */
            || ace->a_type == GROUP_OBJ /* no need to check ace->a_id here */
            || ace->a_type == CLASS_OBJ
            || ace->a_type == OTHER_OBJ))
        return 1;
    }
  return 0;
}

#endif

void
free_permission_context (struct permission_context *ctx)
{
#if USE_ACL
# if HAVE_ACL_GET_FILE /* Linux, FreeBSD, Mac OS X, IRIX, Tru64, Cygwin >= 2.5 */
  if (ctx->acl)
    acl_free (ctx->acl);
#  if !HAVE_ACL_TYPE_EXTENDED
  if (ctx->default_acl)
    acl_free (ctx->default_acl);
#  endif

# elif defined GETACL /* Solaris, Cygwin < 2.5 */
  free (ctx->entries);
#  ifdef ACE_GETACL
  free (ctx->ace_entries);
#  endif

# elif HAVE_GETACL /* HP-UX */

#  if HAVE_ACLV_H
#  endif

# elif HAVE_STATACL /* older AIX */

# elif HAVE_ACLSORT /* NonStop Kernel */

# endif
#endif
}
