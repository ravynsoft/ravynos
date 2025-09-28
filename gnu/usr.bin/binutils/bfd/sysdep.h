/* sysdep.h -- handle host dependencies for the BFD library
   Copyright (C) 1995-2023 Free Software Foundation, Inc.
   Written by Cygnus Support.

   This file is part of BFD, the Binary File Descriptor library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#ifndef BFD_SYSDEP_H
#define BFD_SYSDEP_H

#ifdef PACKAGE
#error sysdep.h must be included in lieu of config.h
#endif

#include "config.h"
#include <stdio.h>

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <errno.h>
#include <time.h>

#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif /* HAVE_SYS_RESOURCE_H */

#ifdef USE_BINARY_FOPEN
#include "fopen-bin.h"
#else
#include "fopen-same.h"
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#else
#ifdef HAVE_SYS_FILE_H
#include <sys/file.h>
#endif
#endif

#ifndef O_RDONLY
#define O_RDONLY 0
#endif
#ifndef O_WRONLY
#define O_WRONLY 1
#endif
#ifndef O_RDWR
#define O_RDWR 2
#endif
#ifndef O_ACCMODE
#define O_ACCMODE (O_RDONLY | O_WRONLY | O_RDWR)
#endif
/* Systems that don't already define this, don't need it.  */
#ifndef O_BINARY
#define O_BINARY 0
#endif

#ifndef SEEK_SET
#define SEEK_SET 0
#endif
#ifndef SEEK_CUR
#define SEEK_CUR 1
#endif

#include "filenames.h"

#if !HAVE_DECL_FFS
extern int ffs (int);
#endif

#if !HAVE_DECL_STPCPY
extern char *stpcpy (char *__dest, const char *__src);
#endif

#ifdef HAVE_FTELLO
#if !HAVE_DECL_FTELLO
extern off_t ftello (FILE *stream);
#endif
#endif

#ifdef HAVE_FTELLO64
#if !HAVE_DECL_FTELLO64
extern off64_t ftello64 (FILE *stream);
#endif
#endif

#ifdef HAVE_FSEEKO
#if !HAVE_DECL_FSEEKO
extern int fseeko (FILE *stream, off_t offset, int whence);
#endif
#endif

#ifdef HAVE_FSEEKO64
#if !HAVE_DECL_FSEEKO64
extern int fseeko64 (FILE *stream, off64_t offset, int whence);
#endif
#endif

/* Define offsetof for those systems which lack it */

#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

#ifndef ENABLE_NLS
  /* The Solaris version of locale.h always includes libintl.h.  If we have
     been configured with --disable-nls then ENABLE_NLS will not be defined
     and the dummy definitions of bindtextdomain (et al) below will conflict
     with the defintions in libintl.h.  So we define these values to prevent
     the bogus inclusion of libintl.h.  */
# define _LIBINTL_H
# define _LIBGETTEXT_H
#endif
#include <locale.h>

#ifdef ENABLE_NLS
# include <libintl.h>
/* Note the redefinition of gettext and ngettext here to use PACKAGE.

   This is because the code in this directory is used to build a
   library which will be linked with code in other directories to form
   programs.  We want to maintain a separate translation file for this
   directory however, rather than being forced to merge it with that
   of any program linked to libbfd.  This is a library, so it cannot
   depend on the catalog currently loaded.

   In order to do this, we have to make sure that when we extract
   messages we use the BFD domain rather than the domain of the
   program that included the bfd library, (eg OBJDUMP).  Hence we use
   dgettext (PACKAGE, String) and define PACKAGE to be 'bfd'.
   (See the code in configure).  */
# undef gettext
# define gettext(Msgid) dgettext (PACKAGE, Msgid)
# undef ngettext
# define ngettext(Msgid1, Msgid2, n) dngettext (PACKAGE, Msgid1, Msgid2, n)
# define _(String) gettext (String)
# ifdef gettext_noop
#  define N_(String) gettext_noop (String)
# else
#  define N_(String) (String)
# endif
#else
# define gettext(Msgid) (Msgid)
# define dgettext(Domainname, Msgid) (Msgid)
# define dcgettext(Domainname, Msgid, Category) (Msgid)
# define ngettext(Msgid1, Msgid2, n) \
  (n == 1 ? Msgid1 : Msgid2)
# define dngettext(Domainname, Msgid1, Msgid2, n) \
  (n == 1 ? Msgid1 : Msgid2)
# define dcngettext(Domainname, Msgid1, Msgid2, n, Category) \
  (n == 1 ? Msgid1 : Msgid2)
# define textdomain(Domainname) do {} while (0)
# define bindtextdomain(Domainname, Dirname) do {} while (0)
# define _(String) (String)
# define N_(String) (String)
#endif

#ifndef HAVE_GETUID
#define getuid() 0
#endif

#ifndef HAVE_GETGID
#define getgid() 0
#endif

#define POISON_BFD_BOOLEAN 1

#endif /* ! defined (BFD_SYSDEP_H) */
