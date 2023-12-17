/* Internal implementation of access control lists.  -*- coding: utf-8 -*-

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

/* This file uses _GL_INLINE_HEADER_BEGIN, _GL_INLINE, _GL_ATTRIBUTE_PURE.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

#include "acl.h"

#include <stdlib.h>

/* All systems define the ACL related API in <sys/acl.h>.  */
#if HAVE_SYS_ACL_H
# include <sys/acl.h>
#endif
#if defined HAVE_FACL && ! defined GETACLCNT && defined ACL_CNT
# define GETACLCNT ACL_CNT
#endif

/* On Linux and Cygwin >= 2.5, additional ACL related API is available in
   <acl/libacl.h>.  */
#ifdef HAVE_ACL_LIBACL_H
# include <acl/libacl.h>
#endif

/* On HP-UX >= 11.11, additional ACL API is available in <aclv.h>.  */
#if HAVE_ACLV_H
# include <sys/types.h>
# include <aclv.h>
/* HP-UX 11.11 lacks these declarations.  */
extern int acl (char *, int, int, struct acl *);
extern int aclsort (int, int, struct acl *);
#endif

#include <errno.h>

#include <limits.h>

#ifndef SIZE_MAX
# define SIZE_MAX ((size_t) -1)
#endif

#ifndef HAVE_FCHMOD
# define HAVE_FCHMOD false
# define fchmod(fd, mode) (-1)
#endif

_GL_INLINE_HEADER_BEGIN
#ifndef ACL_INTERNAL_INLINE
# define ACL_INTERNAL_INLINE _GL_INLINE
#endif

#if USE_ACL

# if HAVE_ACL_GET_FILE
/* POSIX 1003.1e (draft 17 -- abandoned) specific version.  */
/* Linux, FreeBSD, Mac OS X, IRIX, Tru64, Cygwin >= 2.5 */

#  ifndef MIN_ACL_ENTRIES
#   define MIN_ACL_ENTRIES 4
#  endif

/* POSIX 1003.1e (draft 17) */
#  ifdef HAVE_ACL_GET_FD
/* Most platforms have a 1-argument acl_get_fd, only OSF/1 has a 2-argument
   macro(!).  */
#   if HAVE_ACL_FREE_TEXT /* OSF/1 */
ACL_INTERNAL_INLINE acl_t
rpl_acl_get_fd (int fd)
{
  return acl_get_fd (fd, ACL_TYPE_ACCESS);
}
#    undef acl_get_fd
#    define acl_get_fd rpl_acl_get_fd
#   endif
#  else
#   define HAVE_ACL_GET_FD false
#   undef acl_get_fd
#   define acl_get_fd(fd) (NULL)
#  endif

/* POSIX 1003.1e (draft 17) */
#  ifdef HAVE_ACL_SET_FD
/* Most platforms have a 2-argument acl_set_fd, only OSF/1 has a 3-argument
   macro(!).  */
#   if HAVE_ACL_FREE_TEXT /* OSF/1 */
ACL_INTERNAL_INLINE int
rpl_acl_set_fd (int fd, acl_t acl)
{
  return acl_set_fd (fd, ACL_TYPE_ACCESS, acl);
}
#    undef acl_set_fd
#    define acl_set_fd rpl_acl_set_fd
#   endif
#  else
#   define HAVE_ACL_SET_FD false
#   undef acl_set_fd
#   define acl_set_fd(fd, acl) (-1)
#  endif

/* POSIX 1003.1e (draft 13) */
#  if ! HAVE_ACL_FREE_TEXT
#   define acl_free_text(buf) acl_free (buf)
#  endif

/* Linux-specific */
/* Cygwin >= 2.5 implements this function, but it returns 1 for all
   directories, thus is unusable.  */
#  if !defined HAVE_ACL_EXTENDED_FILE || defined __CYGWIN__
#   undef HAVE_ACL_EXTENDED_FILE
#   define HAVE_ACL_EXTENDED_FILE false
#   define acl_extended_file(name) (-1)
#  endif

#  if ! defined HAVE_ACL_FROM_MODE && ! defined HAVE_ACL_FROM_TEXT
#   define acl_from_mode (NULL)
#  endif

/* Set to 0 if a file's mode is stored independently from the ACL.  */
#  if (HAVE_ACL_COPY_EXT_NATIVE && HAVE_ACL_CREATE_ENTRY_NP) || defined __sgi /* Mac OS X, IRIX */
#   define MODE_INSIDE_ACL 0
#  endif

/* Return the number of entries in ACL.
   Return -1 and set errno upon failure to determine it.  */
/* Define a replacement for acl_entries if needed. (Only Linux has it.)  */
#  if !HAVE_ACL_ENTRIES
#   define acl_entries rpl_acl_entries
extern int acl_entries (acl_t);
#  endif

#  if HAVE_ACL_TYPE_EXTENDED /* Mac OS X */
/* ACL is an ACL, from a file, stored as type ACL_TYPE_EXTENDED.
   Return 1 if the given ACL is non-trivial.
   Return 0 if it is trivial.  */
extern int acl_extended_nontrivial (acl_t);
#  else
/* ACL is an ACL, from a file, stored as type ACL_TYPE_ACCESS.
   Return 1 if the given ACL is non-trivial.
   Return 0 if it is trivial, i.e. equivalent to a simple stat() mode.
   Return -1 and set errno upon failure to determine it.  */
extern int acl_access_nontrivial (acl_t);

/* ACL is an ACL, from a file, stored as type ACL_TYPE_DEFAULT.
   Return 1 if the given ACL is non-trivial.
   Return 0 if it is trivial, i.e. equivalent to a simple stat() mode.
   Return -1 and set errno upon failure to determine it.  */
extern int acl_default_nontrivial (acl_t);
#  endif

# elif HAVE_FACL && defined GETACL /* Solaris, Cygwin < 2.5, not HP-UX */

/* Set to 0 if a file's mode is stored independently from the ACL.  */
#  if defined __CYGWIN__ /* Cygwin */
#   define MODE_INSIDE_ACL 0
#  endif

/* Return 1 if the given ACL is non-trivial.
   Return 0 if it is trivial, i.e. equivalent to a simple stat() mode.  */
extern int acl_nontrivial (int count, aclent_t *entries) _GL_ATTRIBUTE_PURE;

#  ifdef ACE_GETACL /* Solaris 10 */

/* Test an ACL retrieved with ACE_GETACL.
   Return 1 if the given ACL, consisting of COUNT entries, is non-trivial.
   Return 0 if it is trivial, i.e. equivalent to a simple stat() mode.  */
extern int acl_ace_nontrivial (int count, ace_t *entries) _GL_ATTRIBUTE_PURE;

/* Definitions for when the built executable is executed on Solaris 10
   (newer version) or Solaris 11.  */
/* For a_type.  */
#   define OLD_ALLOW 0
#   define OLD_DENY  1
#   define NEW_ACE_ACCESS_ALLOWED_ACE_TYPE 0 /* replaces ALLOW */
#   define NEW_ACE_ACCESS_DENIED_ACE_TYPE  1 /* replaces DENY */
/* For a_flags.  */
#   define OLD_ACE_OWNER            0x0100
#   define OLD_ACE_GROUP            0x0200
#   define OLD_ACE_OTHER            0x0400
#   define NEW_ACE_OWNER            0x1000
#   define NEW_ACE_GROUP            0x2000
#   define NEW_ACE_IDENTIFIER_GROUP 0x0040
#   define NEW_ACE_EVERYONE         0x4000
/* For a_access_mask.  */
#   define NEW_ACE_READ_DATA         0x001 /* corresponds to 'r' */
#   define NEW_ACE_WRITE_DATA        0x002 /* corresponds to 'w' */
#   define NEW_ACE_APPEND_DATA       0x004
#   define NEW_ACE_READ_NAMED_ATTRS  0x008
#   define NEW_ACE_WRITE_NAMED_ATTRS 0x010
#   define NEW_ACE_EXECUTE           0x020
#   define NEW_ACE_DELETE_CHILD      0x040
#   define NEW_ACE_READ_ATTRIBUTES   0x080
#   define NEW_ACE_WRITE_ATTRIBUTES  0x100
#   define NEW_ACE_DELETE          0x10000
#   define NEW_ACE_READ_ACL        0x20000
#   define NEW_ACE_WRITE_ACL       0x40000
#   define NEW_ACE_WRITE_OWNER     0x80000
#   define NEW_ACE_SYNCHRONIZE    0x100000

#  endif

# elif HAVE_GETACL /* HP-UX */

/* Return 1 if the given ACL is non-trivial.
   Return 0 if it is trivial, i.e. equivalent to a simple stat() mode.  */
extern int acl_nontrivial (int count, struct acl_entry *entries);

#  if HAVE_ACLV_H /* HP-UX >= 11.11 */

/* Return 1 if the given ACL is non-trivial.
   Return 0 if it is trivial, i.e. equivalent to a simple stat() mode.  */
extern int aclv_nontrivial (int count, struct acl *entries);

#  endif

# elif HAVE_ACLX_GET && 0 /* AIX */

/* TODO */

# elif HAVE_STATACL /* older AIX */

/* Return 1 if the given ACL is non-trivial.
   Return 0 if it is trivial, i.e. equivalent to a simple stat() mode.  */
extern int acl_nontrivial (struct acl *a);

# elif HAVE_ACLSORT /* NonStop Kernel */

/* Return 1 if the given ACL is non-trivial.
   Return 0 if it is trivial, i.e. equivalent to a simple stat() mode.  */
extern int acl_nontrivial (int count, struct acl *entries);

# endif

/* Set to 1 if a file's mode is implicit by the ACL.  */
# ifndef MODE_INSIDE_ACL
#  define MODE_INSIDE_ACL 1
# endif

#endif

struct permission_context {
  mode_t mode;
#if USE_ACL
# if HAVE_ACL_GET_FILE /* Linux, FreeBSD, Mac OS X, IRIX, Tru64, Cygwin >= 2.5 */
  acl_t acl;
#  if !HAVE_ACL_TYPE_EXTENDED
  acl_t default_acl;
#  endif
  bool acls_not_supported;

# elif defined GETACL /* Solaris, Cygwin < 2.5 */
  int count;
  aclent_t *entries;
#  ifdef ACE_GETACL
  int ace_count;
  ace_t *ace_entries;
#  endif

# elif HAVE_GETACL /* HP-UX */
  struct acl_entry entries[NACLENTRIES];
  int count;
#  if HAVE_ACLV_H
  struct acl aclv_entries[NACLVENTRIES];
  int aclv_count;
#  endif

# elif HAVE_STATACL /* older AIX */
  union { struct acl a; char room[4096]; } u;
  bool have_u;

# elif HAVE_ACLSORT /* NonStop Kernel */
  struct acl entries[NACLENTRIES];
  int count;

# endif
#endif
};

int get_permissions (const char *, int, mode_t, struct permission_context *);
int set_permissions (struct permission_context *, const char *, int);
void free_permission_context (struct permission_context *);

_GL_INLINE_HEADER_END
