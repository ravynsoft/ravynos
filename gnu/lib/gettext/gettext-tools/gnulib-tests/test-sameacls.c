/* Test whether two files have the same ACLs.
   Copyright (C) 2008-2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <bruno@clisp.org>, 2008.  */

#include <config.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#if HAVE_ACL_GET_FILE || HAVE_FACL || HAVE_GETACL || HAVE_ACLX_GET || HAVE_STATACL || HAVE_ACLSORT
# include <sys/types.h>
# include <sys/acl.h>
#endif
#if HAVE_ACLV_H
# include <sys/types.h>
# include <aclv.h>
#endif

#include "read-file.h"
#include "xalloc.h"
#include "macros.h"

int
main (int argc, char *argv[])
{
  const char *file1;
  const char *file2;

  ASSERT (argc == 3);

  file1 = argv[1];
  file2 = argv[2];

  /* Compare the contents of the two files.  */
  {
    size_t size1;
    char *contents1;
    size_t size2;
    char *contents2;

    contents1 = read_file (file1, 0, &size1);
    if (contents1 == NULL)
      {
        fprintf (stderr, "error reading file %s: errno = %d\n", file1, errno);
        fflush (stderr);
        abort ();
      }
    contents2 = read_file (file2, 0, &size2);
    if (contents2 == NULL)
      {
        fprintf (stderr, "error reading file %s: errno = %d\n", file2, errno);
        fflush (stderr);
        abort ();
      }

    if (size2 != size1)
      {
        fprintf (stderr, "files %s and %s have different sizes\n",
                 file1, file2);
        fflush (stderr);
        abort ();
      }
    if (memcmp (contents1, contents2, size1) != 0)
      {
        fprintf (stderr, "files %s and %s have different contents\n",
                 file1, file2);
        fflush (stderr);
        abort ();
      }

    free (contents2);
    free (contents1);
  }

  /* Compare the access permissions of the two files, including ACLs.  */
  {
    struct stat statbuf1;
    struct stat statbuf2;

    if (stat (file1, &statbuf1) < 0)
      {
        fprintf (stderr, "error accessing file %s: errno = %d\n", file1, errno);
        fflush (stderr);
        abort ();
      }
    if (stat (file2, &statbuf2) < 0)
      {
        fprintf (stderr, "error accessing file %s: errno = %d\n", file2, errno);
        fflush (stderr);
        abort ();
      }
    if (statbuf1.st_mode != statbuf2.st_mode)
      {
        fprintf (stderr, "files %s and %s have different access modes: %03o and %03o\n",
                 file1, file2,
                (unsigned int) statbuf1.st_mode, (unsigned int) statbuf2.st_mode);
        return 1;
      }
  }
  {
#if HAVE_ACL_GET_FILE /* Linux, FreeBSD, Mac OS X, IRIX, Tru64 */
    static const int types[] =
      {
        ACL_TYPE_ACCESS
# if HAVE_ACL_TYPE_EXTENDED /* Mac OS X */
        , ACL_TYPE_EXTENDED
# endif
      };
    int t;

    for (t = 0; t < sizeof (types) / sizeof (types[0]); t++)
      {
        int type = types[t];
        acl_t acl1;
        char *text1;
        int errno1;
        acl_t acl2;
        char *text2;
        int errno2;

        acl1 = acl_get_file (file1, type);
        if (acl1 == (acl_t)NULL)
          {
            text1 = NULL;
            errno1 = errno;
          }
        else
          {
            text1 = acl_to_text (acl1, NULL);
            if (text1 == NULL)
              errno1 = errno;
            else
              errno1 = 0;
          }
        acl2 = acl_get_file (file2, type);
        if (acl2 == (acl_t)NULL)
          {
            text2 = NULL;
            errno2 = errno;
          }
        else
          {
            text2 = acl_to_text (acl2, NULL);
            if (text2 == NULL)
              errno2 = errno;
            else
              errno2 = 0;
          }

        if (acl1 != (acl_t)NULL)
          {
            if (acl2 != (acl_t)NULL)
              {
                if (text1 != NULL)
                  {
                    if (text2 != NULL)
                      {
                        if (strcmp (text1, text2) != 0)
                          {
                            fprintf (stderr, "files %s and %s have different ACLs:\n%s\n%s\n",
                                     file1, file2, text1, text2);
                            return 1;
                          }
                      }
                    else
                      {
                        fprintf (stderr, "file %s has a valid ACL, but file %s has an invalid ACL\n",
                                 file1, file2);
                        return 1;
                      }
                  }
                else
                  {
                    if (text2 != NULL)
                      {
                        fprintf (stderr, "file %s has an invalid ACL, but file %s has a valid ACL\n",
                                 file1, file2);
                        return 1;
                      }
                    else
                      {
                        if (errno1 != errno2)
                          {
                            fprintf (stderr, "files %s and %s have differently invalid ACLs, errno = %d vs. %d\n",
                                     file1, file2, errno1, errno2);
                            return 1;
                          }
                      }
                  }
              }
            else
              {
                fprintf (stderr, "file %s has an ACL, but file %s has no ACL\n",
                         file1, file2);
                return 1;
              }
          }
        else
          {
            if (acl2 != (acl_t)NULL)
              {
                fprintf (stderr, "file %s has no ACL, but file %s has an ACL\n",
                         file1, file2);
                return 1;
              }
          }
        acl_free (text2);
        if (acl2 != (acl_t)NULL)
          acl_free (acl2);
        acl_free (text1);
        if (acl1 != (acl_t)NULL)
          acl_free (acl1);
      }
#elif HAVE_FACL && defined GETACL /* Solaris, Cygwin, not HP-UX */
  int count1;
  int count2;

  count1 = acl (file1, GETACLCNT, 0, NULL);
  if (count1 < 0 && errno == ENOSYS) /* Can happen on Solaris 10 with ZFS */
    count1 = 0;
  count2 = acl (file2, GETACLCNT, 0, NULL);
  if (count2 < 0 && errno == ENOSYS) /* Can happen on Solaris 10 with ZFS */
    count2 = 0;

  if (count1 < 0)
    {
      fprintf (stderr, "error accessing the ACLs of file %s\n", file1);
      fflush (stderr);
      abort ();
    }
  if (count2 < 0)
    {
      fprintf (stderr, "error accessing the ACLs of file %s\n", file2);
      fflush (stderr);
      abort ();
    }
  if (count1 != count2)
    {
      fprintf (stderr, "files %s and %s have different number of ACLs: %d and %d\n",
               file1, file2, count1, count2);
      return 1;
    }
  else
    {
      aclent_t *entries1 = XNMALLOC (count1, aclent_t);
      aclent_t *entries2 = XNMALLOC (count2, aclent_t);
      int i;

      if (count1 > 0 && acl (file1, GETACL, count1, entries1) < count1)
        {
          fprintf (stderr, "error retrieving the ACLs of file %s\n", file1);
          fflush (stderr);
          abort ();
        }
      if (count2 > 0 && acl (file2, GETACL, count2, entries2) < count1)
        {
          fprintf (stderr, "error retrieving the ACLs of file %s\n", file2);
          fflush (stderr);
          abort ();
        }
      for (i = 0; i < count1; i++)
        {
          if (entries1[i].a_type != entries2[i].a_type)
            {
              fprintf (stderr, "files %s and %s: different ACL entry #%d: different types %d and %d\n",
                       file1, file2, i, entries1[i].a_type, entries2[i].a_type);
              return 1;
            }
          if (entries1[i].a_id != entries2[i].a_id)
            {
              fprintf (stderr, "files %s and %s: different ACL entry #%d: different ids %d and %d\n",
                       file1, file2, i, (int)entries1[i].a_id, (int)entries2[i].a_id);
              return 1;
            }
          if (entries1[i].a_perm != entries2[i].a_perm)
            {
              fprintf (stderr, "files %s and %s: different ACL entry #%d: different permissions %03o and %03o\n",
                       file1, file2, i, (unsigned int) entries1[i].a_perm, (unsigned int) entries2[i].a_perm);
              return 1;
            }
        }
      free (entries2);
      free (entries1);
    }
# ifdef ACE_GETACL
  count1 = acl (file1, ACE_GETACLCNT, 0, NULL);
  if (count1 < 0 && errno == EINVAL)
    count1 = 0;
  count2 = acl (file2, ACE_GETACLCNT, 0, NULL);
  if (count2 < 0 && errno == EINVAL)
    count2 = 0;
  if (count1 < 0)
    {
      fprintf (stderr, "error accessing the ACE-ACLs of file %s\n", file1);
      fflush (stderr);
      abort ();
    }
  if (count2 < 0)
    {
      fprintf (stderr, "error accessing the ACE-ACLs of file %s\n", file2);
      fflush (stderr);
      abort ();
    }
  {
    ace_t *entries1 = XNMALLOC (count1, ace_t);
    ace_t *entries2 = XNMALLOC (count2, ace_t);
    int ret;
    int i;

    ret = acl (file1, ACE_GETACL, count1, entries1);
    if (ret < 0 && errno == EINVAL)
      count1 = 0;
    else if (ret < count1)
      {
        fprintf (stderr, "error retrieving the ACE-ACLs of file %s\n", file1);
        fflush (stderr);
        abort ();
      }
    ret = acl (file2, ACE_GETACL, count2, entries2);
    if (ret < 0 && errno == EINVAL)
      count2 = 0;
    else if (ret < count2)
      {
        fprintf (stderr, "error retrieving the ACE-ACLs of file %s\n", file2);
        fflush (stderr);
        abort ();
      }

    if (count1 != count2)
      {
        fprintf (stderr, "files %s and %s have different number of ACE-ACLs: %d and %d\n",
                 file1, file2, count1, count2);
        return 1;
      }

    for (i = 0; i < count1; i++)
      {
        if (entries1[i].a_type != entries2[i].a_type)
          {
            fprintf (stderr, "files %s and %s: different ACE-ACL entry #%d: different types %d and %d\n",
                     file1, file2, i, entries1[i].a_type, entries2[i].a_type);
            return 1;
          }
        if (entries1[i].a_who != entries2[i].a_who)
          {
            fprintf (stderr, "files %s and %s: different ACE-ACL entry #%d: different ids %d and %d\n",
                     file1, file2, i, (int)entries1[i].a_who, (int)entries2[i].a_who);
            return 1;
          }
        if (entries1[i].a_access_mask != entries2[i].a_access_mask)
          {
            fprintf (stderr, "files %s and %s: different ACE-ACL entry #%d: different access masks %03o and %03o\n",
                     file1, file2, i, (unsigned int) entries1[i].a_access_mask, (unsigned int) entries2[i].a_access_mask);
            return 1;
          }
        if (entries1[i].a_flags != entries2[i].a_flags)
          {
            fprintf (stderr, "files %s and %s: different ACE-ACL entry #%d: different flags 0x%x and 0x%x\n",
                     file1, file2, i, (unsigned int) entries1[i].a_flags, (unsigned int) entries2[i].a_flags);
            return 1;
          }
      }
    free (entries2);
    free (entries1);
  }
# endif
#elif HAVE_GETACL /* HP-UX */
  int count1;
  int count2;

  count1 = getacl (file1, 0, NULL);
  if (count1 < 0
      && (errno == ENOSYS || errno == EOPNOTSUPP || errno == ENOTSUP))
    count1 = 0;
  count2 = getacl (file2, 0, NULL);
  if (count2 < 0
      && (errno == ENOSYS || errno == EOPNOTSUPP || errno == ENOTSUP))
    count2 = 0;

  if (count1 < 0)
    {
      fprintf (stderr, "error accessing the ACLs of file %s\n", file1);
      fflush (stderr);
      abort ();
    }
  if (count2 < 0)
    {
      fprintf (stderr, "error accessing the ACLs of file %s\n", file2);
      fflush (stderr);
      abort ();
    }
  if (count1 != count2)
    {
      fprintf (stderr, "files %s and %s have different number of ACLs: %d and %d\n",
               file1, file2, count1, count2);
      return 1;
    }
  else if (count1 > 0)
    {
      struct acl_entry *entries1 = XNMALLOC (count1, struct acl_entry);
      struct acl_entry *entries2 = XNMALLOC (count2, struct acl_entry);
      int i;

      if (getacl (file1, count1, entries1) < count1)
        {
          fprintf (stderr, "error retrieving the ACLs of file %s\n", file1);
          fflush (stderr);
          abort ();
        }
      if (getacl (file2, count2, entries2) < count1)
        {
          fprintf (stderr, "error retrieving the ACLs of file %s\n", file2);
          fflush (stderr);
          abort ();
        }
      for (i = 0; i < count1; i++)
        {
          if (entries1[i].uid != entries2[i].uid)
            {
              fprintf (stderr, "files %s and %s: different ACL entry #%d: different uids %d and %d\n",
                       file1, file2, i, (int)entries1[i].uid, (int)entries2[i].uid);
              return 1;
            }
          if (entries1[i].gid != entries2[i].gid)
            {
              fprintf (stderr, "files %s and %s: different ACL entry #%d: different gids %d and %d\n",
                       file1, file2, i, (int)entries1[i].gid, (int)entries2[i].gid);
              return 1;
            }
          if (entries1[i].mode != entries2[i].mode)
            {
              fprintf (stderr, "files %s and %s: different ACL entry #%d: different permissions %03o and %03o\n",
                       file1, file2, i, (unsigned int) entries1[i].mode, (unsigned int) entries2[i].mode);
              return 1;
            }
        }
      free (entries2);
      free (entries1);
    }

# if HAVE_ACLV_H /* HP-UX >= 11.11 */
  {
    struct acl dummy_entries[NACLVENTRIES];

    count1 = acl ((char *) file1, ACL_CNT, NACLVENTRIES, dummy_entries);
    if (count1 < 0
        && (errno == ENOSYS || errno == EOPNOTSUPP || errno == EINVAL))
      count1 = 0;
    count2 = acl ((char *) file2, ACL_CNT, NACLVENTRIES, dummy_entries);
    if (count2 < 0
        && (errno == ENOSYS || errno == EOPNOTSUPP || errno == EINVAL))
      count2 = 0;
  }

  if (count1 < 0)
    {
      fprintf (stderr, "error accessing the ACLs of file %s\n", file1);
      fflush (stderr);
      abort ();
    }
  if (count2 < 0)
    {
      fprintf (stderr, "error accessing the ACLs of file %s\n", file2);
      fflush (stderr);
      abort ();
    }
  if (count1 != count2)
    {
      fprintf (stderr, "files %s and %s have different number of ACLs: %d and %d\n",
               file1, file2, count1, count2);
      return 1;
    }
  else if (count1 > 0)
    {
      struct acl *entries1 = XNMALLOC (count1, struct acl);
      struct acl *entries2 = XNMALLOC (count2, struct acl);
      int i;

      if (acl ((char *) file1, ACL_GET, count1, entries1) < count1)
        {
          fprintf (stderr, "error retrieving the ACLs of file %s\n", file1);
          fflush (stderr);
          abort ();
        }
      if (acl ((char *) file2, ACL_GET, count2, entries2) < count1)
        {
          fprintf (stderr, "error retrieving the ACLs of file %s\n", file2);
          fflush (stderr);
          abort ();
        }
      for (i = 0; i < count1; i++)
        {
          if (entries1[i].a_type != entries2[i].a_type)
            {
              fprintf (stderr, "files %s and %s: different ACL entry #%d: different types %d and %d\n",
                       file1, file2, i, entries1[i].a_type, entries2[i].a_type);
              return 1;
            }
          if (entries1[i].a_id != entries2[i].a_id)
            {
              fprintf (stderr, "files %s and %s: different ACL entry #%d: different ids %d and %d\n",
                       file1, file2, i, (int)entries1[i].a_id, (int)entries2[i].a_id);
              return 1;
            }
          if (entries1[i].a_perm != entries2[i].a_perm)
            {
              fprintf (stderr, "files %s and %s: different ACL entry #%d: different permissions %03o and %03o\n",
                       file1, file2, i, (unsigned int) entries1[i].a_perm, (unsigned int) entries2[i].a_perm);
              return 1;
            }
        }
      free (entries2);
      free (entries1);
    }
# endif
#elif HAVE_ACLX_GET /* AIX */
  acl_type_t type1;
  char acl1[1000];
  size_t aclsize1 = sizeof (acl1);
  mode_t mode1;
  char text1[1000];
  size_t textsize1 = sizeof (text1);
  acl_type_t type2;
  char acl2[1000];
  size_t aclsize2 = sizeof (acl2);
  mode_t mode2;
  char text2[1000];
  size_t textsize2 = sizeof (text2);

  /* The docs say that type1 being 0 is equivalent to ACL_ANY, but it is not
     true, in AIX 5.3.  */
  type1.u64 = ACL_ANY;
  if (aclx_get (file1, 0, &type1, acl1, &aclsize1, &mode1) < 0)
    {
      if (errno == ENOSYS)
        text1[0] = '\0';
      else
        {
          fprintf (stderr, "error accessing the ACLs of file %s\n", file1);
          fflush (stderr);
          abort ();
        }
    }
  else
    if (aclx_printStr (text1, &textsize1, acl1, aclsize1, type1, file1, 0) < 0)
      {
        fprintf (stderr, "cannot convert the ACLs of file %s to text\n", file1);
        fflush (stderr);
        abort ();
      }

  /* The docs say that type2 being 0 is equivalent to ACL_ANY, but it is not
     true, in AIX 5.3.  */
  type2.u64 = ACL_ANY;
  if (aclx_get (file2, 0, &type2, acl2, &aclsize2, &mode2) < 0)
    {
      if (errno == ENOSYS)
        text2[0] = '\0';
      else
        {
          fprintf (stderr, "error accessing the ACLs of file %s\n", file2);
          fflush (stderr);
          abort ();
        }
    }
  else
    if (aclx_printStr (text2, &textsize2, acl2, aclsize2, type2, file2, 0) < 0)
      {
        fprintf (stderr, "cannot convert the ACLs of file %s to text\n", file2);
        fflush (stderr);
        abort ();
      }

  if (strcmp (text1, text2) != 0)
    {
      fprintf (stderr, "files %s and %s have different ACLs:\n%s\n%s\n",
               file1, file2, text1, text2);
      return 1;
    }
#elif HAVE_STATACL /* older AIX */
  union { struct acl a; char room[4096]; } acl1;
  union { struct acl a; char room[4096]; } acl2;
  unsigned int i;

  if (statacl (file1, STX_NORMAL, &acl1.a, sizeof (acl1)) < 0)
    {
      fprintf (stderr, "error accessing the ACLs of file %s\n", file1);
      fflush (stderr);
      abort ();
    }
  if (statacl (file2, STX_NORMAL, &acl2.a, sizeof (acl2)) < 0)
    {
      fprintf (stderr, "error accessing the ACLs of file %s\n", file2);
      fflush (stderr);
      abort ();
    }

  if (acl1.a.acl_len != acl2.a.acl_len)
    {
      fprintf (stderr, "files %s and %s have different ACL lengths: %u and %u\n",
               file1, file2, acl1.a.acl_len, acl2.a.acl_len);
      return 1;
    }
  if (acl1.a.acl_mode != acl2.a.acl_mode)
    {
      fprintf (stderr, "files %s and %s have different ACL modes: %03o and %03o\n",
               file1, file2, acl1.a.acl_mode, acl2.a.acl_mode);
      return 1;
    }
  if (acl1.a.u_access != acl2.a.u_access
      || acl1.a.g_access != acl2.a.g_access
      || acl1.a.o_access != acl2.a.o_access)
    {
      fprintf (stderr, "files %s and %s have different ACL access masks: %03o %03o %03o and %03o %03o %03o\n",
               file1, file2,
               acl1.a.u_access, acl1.a.g_access, acl1.a.o_access,
               acl2.a.u_access, acl2.a.g_access, acl2.a.o_access);
      return 1;
    }
  if (memcmp (acl1.a.acl_ext, acl2.a.acl_ext, acl1.a.acl_len) != 0)
    {
      fprintf (stderr, "files %s and %s have different ACL entries\n",
               file1, file2);
      return 1;
    }
#elif HAVE_ACLSORT /* NonStop Kernel */
  int count1;
  int count2;

  count1 = acl ((char *) file1, ACL_CNT, NACLENTRIES, NULL);
  count2 = acl ((char *) file2, ACL_CNT, NACLENTRIES, NULL);

  if (count1 < 0)
    {
      fprintf (stderr, "error accessing the ACLs of file %s\n", file1);
      fflush (stderr);
      abort ();
    }
  if (count2 < 0)
    {
      fprintf (stderr, "error accessing the ACLs of file %s\n", file2);
      fflush (stderr);
      abort ();
    }
  if (count1 != count2)
    {
      fprintf (stderr, "files %s and %s have different number of ACLs: %d and %d\n",
               file1, file2, count1, count2);
      return 1;
    }
  else if (count1 > 0)
    {
      struct acl *entries1 = XNMALLOC (count1, struct acl);
      struct acl *entries2 = XNMALLOC (count2, struct acl);
      int i;

      if (acl ((char *) file1, ACL_GET, count1, entries1) < count1)
        {
          fprintf (stderr, "error retrieving the ACLs of file %s\n", file1);
          fflush (stderr);
          abort ();
        }
      if (acl ((char *) file2, ACL_GET, count2, entries2) < count1)
        {
          fprintf (stderr, "error retrieving the ACLs of file %s\n", file2);
          fflush (stderr);
          abort ();
        }
      for (i = 0; i < count1; i++)
        {
          if (entries1[i].a_type != entries2[i].a_type)
            {
              fprintf (stderr, "files %s and %s: different ACL entry #%d: different types %d and %d\n",
                       file1, file2, i, entries1[i].a_type, entries2[i].a_type);
              return 1;
            }
          if (entries1[i].a_id != entries2[i].a_id)
            {
              fprintf (stderr, "files %s and %s: different ACL entry #%d: different ids %d and %d\n",
                       file1, file2, i, (int)entries1[i].a_id, (int)entries2[i].a_id);
              return 1;
            }
          if (entries1[i].a_perm != entries2[i].a_perm)
            {
              fprintf (stderr, "files %s and %s: different ACL entry #%d: different permissions %03o and %03o\n",
                       file1, file2, i, (unsigned int) entries1[i].a_perm, (unsigned int) entries2[i].a_perm);
              return 1;
            }
        }
      free (entries2);
      free (entries1);
    }
#endif
  }

  return 0;
}
