/* Load needed message catalogs.
   Copyright (C) 1995-2021 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation; either version 2.1 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Tell glibc's <string.h> to provide a prototype for mempcpy().
   This must come before <config.h> because <config.h> may include
   <features.h>, and once <features.h> has been included, it's too late.  */
#ifndef _GNU_SOURCE
# define _GNU_SOURCE    1
#endif

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef __GNUC__
# undef  alloca
# define alloca __builtin_alloca
# define HAVE_ALLOCA 1
#else
# ifdef _MSC_VER
#  include <malloc.h>
#  define alloca _alloca
# else
#  if defined HAVE_ALLOCA_H || defined _LIBC
#   include <alloca.h>
#  else
#   ifdef _AIX
 #pragma alloca
#   else
#    ifndef alloca
char *alloca ();
#    endif
#   endif
#  endif
# endif
#endif

#include <stdlib.h>
#include <string.h>

#if defined HAVE_UNISTD_H || defined _LIBC
# include <unistd.h>
#elif defined _WIN32 && !defined __CYGWIN__
# include <io.h>
#endif

#ifdef _LIBC
# include <langinfo.h>
# include <locale.h>
#endif

#if (defined HAVE_MMAP && defined HAVE_MUNMAP && !defined DISALLOW_MMAP) \
    || (defined _LIBC && defined _POSIX_MAPPED_FILES)
# include <sys/mman.h>
# undef HAVE_MMAP
# define HAVE_MMAP	1
#else
# undef HAVE_MMAP
#endif

#if defined HAVE_STDINT_H_WITH_UINTMAX || defined _LIBC
# include <stdint.h>
#endif
#if defined HAVE_INTTYPES_H || defined _LIBC
# include <inttypes.h>
#endif

#include "gmo.h"
#include "gettextP.h"
#include "hash-string.h"
#include "plural-exp.h"

#ifdef _LIBC
# include "../locale/localeinfo.h"
# include <not-cancel.h>
#endif

/* Handle multi-threaded applications.  */
#ifdef _LIBC
# include <bits/libc-lock.h>
#else
# include "glthread/lock.h"
#endif

/* Provide fallback values for macros that ought to be defined in <inttypes.h>.
   Note that our fallback values need not be literal strings, because we don't
   use them with preprocessor string concatenation.  */
#if !defined PRId8
# define PRId8 "d"
#endif
#if !defined PRIi8
# define PRIi8 "i"
#endif
#if !defined PRIo8
# define PRIo8 "o"
#endif
#if !defined PRIu8
# define PRIu8 "u"
#endif
#if !defined PRIx8
# define PRIx8 "x"
#endif
#if !defined PRIX8
# define PRIX8 "X"
#endif
#if !defined PRId16
# define PRId16 "d"
#endif
#if !defined PRIi16
# define PRIi16 "i"
#endif
#if !defined PRIo16
# define PRIo16 "o"
#endif
#if !defined PRIu16
# define PRIu16 "u"
#endif
#if !defined PRIx16
# define PRIx16 "x"
#endif
#if !defined PRIX16
# define PRIX16 "X"
#endif
#if !defined PRId32
# define PRId32 "d"
#endif
#if !defined PRIi32
# define PRIi32 "i"
#endif
#if !defined PRIo32
# define PRIo32 "o"
#endif
#if !defined PRIu32
# define PRIu32 "u"
#endif
#if !defined PRIx32
# define PRIx32 "x"
#endif
#if !defined PRIX32
# define PRIX32 "X"
#endif
#if !defined PRId64
# define PRId64 (sizeof (long) == 8 ? "ld" : "lld")
#endif
#if !defined PRIi64
# define PRIi64 (sizeof (long) == 8 ? "li" : "lli")
#endif
#if !defined PRIo64
# define PRIo64 (sizeof (long) == 8 ? "lo" : "llo")
#endif
#if !defined PRIu64
# define PRIu64 (sizeof (long) == 8 ? "lu" : "llu")
#endif
#if !defined PRIx64
# define PRIx64 (sizeof (long) == 8 ? "lx" : "llx")
#endif
#if !defined PRIX64
# define PRIX64 (sizeof (long) == 8 ? "lX" : "llX")
#endif
#if !defined PRIdLEAST8
# define PRIdLEAST8 "d"
#endif
#if !defined PRIiLEAST8
# define PRIiLEAST8 "i"
#endif
#if !defined PRIoLEAST8
# define PRIoLEAST8 "o"
#endif
#if !defined PRIuLEAST8
# define PRIuLEAST8 "u"
#endif
#if !defined PRIxLEAST8
# define PRIxLEAST8 "x"
#endif
#if !defined PRIXLEAST8
# define PRIXLEAST8 "X"
#endif
#if !defined PRIdLEAST16
# define PRIdLEAST16 "d"
#endif
#if !defined PRIiLEAST16
# define PRIiLEAST16 "i"
#endif
#if !defined PRIoLEAST16
# define PRIoLEAST16 "o"
#endif
#if !defined PRIuLEAST16
# define PRIuLEAST16 "u"
#endif
#if !defined PRIxLEAST16
# define PRIxLEAST16 "x"
#endif
#if !defined PRIXLEAST16
# define PRIXLEAST16 "X"
#endif
#if !defined PRIdLEAST32
# define PRIdLEAST32 "d"
#endif
#if !defined PRIiLEAST32
# define PRIiLEAST32 "i"
#endif
#if !defined PRIoLEAST32
# define PRIoLEAST32 "o"
#endif
#if !defined PRIuLEAST32
# define PRIuLEAST32 "u"
#endif
#if !defined PRIxLEAST32
# define PRIxLEAST32 "x"
#endif
#if !defined PRIXLEAST32
# define PRIXLEAST32 "X"
#endif
#if !defined PRIdLEAST64
# define PRIdLEAST64 PRId64
#endif
#if !defined PRIiLEAST64
# define PRIiLEAST64 PRIi64
#endif
#if !defined PRIoLEAST64
# define PRIoLEAST64 PRIo64
#endif
#if !defined PRIuLEAST64
# define PRIuLEAST64 PRIu64
#endif
#if !defined PRIxLEAST64
# define PRIxLEAST64 PRIx64
#endif
#if !defined PRIXLEAST64
# define PRIXLEAST64 PRIX64
#endif
#if !defined PRIdFAST8
# define PRIdFAST8 "d"
#endif
#if !defined PRIiFAST8
# define PRIiFAST8 "i"
#endif
#if !defined PRIoFAST8
# define PRIoFAST8 "o"
#endif
#if !defined PRIuFAST8
# define PRIuFAST8 "u"
#endif
#if !defined PRIxFAST8
# define PRIxFAST8 "x"
#endif
#if !defined PRIXFAST8
# define PRIXFAST8 "X"
#endif
#if !defined PRIdFAST16
# define PRIdFAST16 "d"
#endif
#if !defined PRIiFAST16
# define PRIiFAST16 "i"
#endif
#if !defined PRIoFAST16
# define PRIoFAST16 "o"
#endif
#if !defined PRIuFAST16
# define PRIuFAST16 "u"
#endif
#if !defined PRIxFAST16
# define PRIxFAST16 "x"
#endif
#if !defined PRIXFAST16
# define PRIXFAST16 "X"
#endif
#if !defined PRIdFAST32
# define PRIdFAST32 "d"
#endif
#if !defined PRIiFAST32
# define PRIiFAST32 "i"
#endif
#if !defined PRIoFAST32
# define PRIoFAST32 "o"
#endif
#if !defined PRIuFAST32
# define PRIuFAST32 "u"
#endif
#if !defined PRIxFAST32
# define PRIxFAST32 "x"
#endif
#if !defined PRIXFAST32
# define PRIXFAST32 "X"
#endif
#if !defined PRIdFAST64
# define PRIdFAST64 PRId64
#endif
#if !defined PRIiFAST64
# define PRIiFAST64 PRIi64
#endif
#if !defined PRIoFAST64
# define PRIoFAST64 PRIo64
#endif
#if !defined PRIuFAST64
# define PRIuFAST64 PRIu64
#endif
#if !defined PRIxFAST64
# define PRIxFAST64 PRIx64
#endif
#if !defined PRIXFAST64
# define PRIXFAST64 PRIX64
#endif
#if !defined PRIdMAX
# define PRIdMAX (sizeof (uintmax_t) == sizeof (long) ? "ld" : "lld")
#endif
#if !defined PRIiMAX
# define PRIiMAX (sizeof (uintmax_t) == sizeof (long) ? "li" : "lli")
#endif
#if !defined PRIoMAX
# define PRIoMAX (sizeof (uintmax_t) == sizeof (long) ? "lo" : "llo")
#endif
#if !defined PRIuMAX
# define PRIuMAX (sizeof (uintmax_t) == sizeof (long) ? "lu" : "llu")
#endif
#if !defined PRIxMAX
# define PRIxMAX (sizeof (uintmax_t) == sizeof (long) ? "lx" : "llx")
#endif
#if !defined PRIXMAX
# define PRIXMAX (sizeof (uintmax_t) == sizeof (long) ? "lX" : "llX")
#endif
#if !defined PRIdPTR
# define PRIdPTR \
  (sizeof (void *) == sizeof (long) ? "ld" : \
   sizeof (void *) == sizeof (int) ? "d" : \
   "lld")
#endif
#if !defined PRIiPTR
# define PRIiPTR \
  (sizeof (void *) == sizeof (long) ? "li" : \
   sizeof (void *) == sizeof (int) ? "i" : \
   "lli")
#endif
#if !defined PRIoPTR
# define PRIoPTR \
  (sizeof (void *) == sizeof (long) ? "lo" : \
   sizeof (void *) == sizeof (int) ? "o" : \
   "llo")
#endif
#if !defined PRIuPTR
# define PRIuPTR \
  (sizeof (void *) == sizeof (long) ? "lu" : \
   sizeof (void *) == sizeof (int) ? "u" : \
   "llu")
#endif
#if !defined PRIxPTR
# define PRIxPTR \
  (sizeof (void *) == sizeof (long) ? "lx" : \
   sizeof (void *) == sizeof (int) ? "x" : \
   "llx")
#endif
#if !defined PRIXPTR
# define PRIXPTR \
  (sizeof (void *) == sizeof (long) ? "lX" : \
   sizeof (void *) == sizeof (int) ? "X" : \
   "llX")
#endif

/* @@ end of prolog @@ */

#ifdef _LIBC
/* Rename the non ISO C functions.  This is required by the standard
   because some ISO C functions will require linking with this object
   file and the name space must not be polluted.  */
# define open(name, flags)	open_not_cancel_2 (name, flags)
# define close(fd)		close_not_cancel_no_status (fd)
# define read(fd, buf, n)	read_not_cancel (fd, buf, n)
# define mmap(addr, len, prot, flags, fd, offset) \
  __mmap (addr, len, prot, flags, fd, offset)
# define munmap(addr, len)	__munmap (addr, len)
#elif defined _WIN32 && !defined __CYGWIN__
/* On native Windows, don't require linking with '-loldnames'.  */
# define open _open
# define read _read
# define close _close
#endif

/* For those losing systems which don't have `alloca' we have to add
   some additional code emulating it.  */
#ifdef HAVE_ALLOCA
# define freea(p) /* nothing */
#else
# define alloca(n) malloc (n)
# define freea(p) free (p)
#endif

/* For systems that distinguish between text and binary I/O.
   O_BINARY is usually declared in <fcntl.h>. */
#if !defined O_BINARY && defined _O_BINARY
  /* For MSC-compatible compilers.  */
# define O_BINARY _O_BINARY
# define O_TEXT _O_TEXT
#endif
#ifdef __BEOS__
  /* BeOS 5 has O_BINARY and O_TEXT, but they have no effect.  */
# undef O_BINARY
# undef O_TEXT
#endif
/* On reasonable systems, binary I/O is the default.  */
#ifndef O_BINARY
# define O_BINARY 0
#endif


/* We need a sign, whether a new catalog was loaded, which can be associated
   with all translations.  This is important if the translations are
   cached by one of GCC's features.  */
#if defined __APPLE__ && defined __MACH__
/* On macOS 10.13 with Apple clang-902.0.39.1 and cctools-895, when linking
   statically, we need an explicit zero-initialization, in order to avoid a
   link-time error that __nl_msg_cat_cntr is an undefined symbol.  It could
   be a compiler bug or a ranlib bug.  */
int _nl_msg_cat_cntr = 0;
#else
int _nl_msg_cat_cntr;
#endif


/* Expand a system dependent string segment.  Return NULL if unsupported.  */
static const char *
get_sysdep_segment_value (const char *name)
{
  /* Test for an ISO C 99 section 7.8.1 format string directive.
     Syntax:
     P R I { d | i | o | u | x | X }
     { { | LEAST | FAST } { 8 | 16 | 32 | 64 } | MAX | PTR }  */
  /* We don't use a table of 14 times 6 'const char *' strings here, because
     data relocations cost startup time.  */
  if (name[0] == 'P' && name[1] == 'R' && name[2] == 'I')
    {
      if (name[3] == 'd' || name[3] == 'i' || name[3] == 'o' || name[3] == 'u'
	  || name[3] == 'x' || name[3] == 'X')
	{
	  if (name[4] == '8' && name[5] == '\0')
	    {
	      if (name[3] == 'd')
		return PRId8;
	      if (name[3] == 'i')
		return PRIi8;
	      if (name[3] == 'o')
		return PRIo8;
	      if (name[3] == 'u')
		return PRIu8;
	      if (name[3] == 'x')
		return PRIx8;
	      if (name[3] == 'X')
		return PRIX8;
	      abort ();
	    }
	  if (name[4] == '1' && name[5] == '6' && name[6] == '\0')
	    {
	      if (name[3] == 'd')
		return PRId16;
	      if (name[3] == 'i')
		return PRIi16;
	      if (name[3] == 'o')
		return PRIo16;
	      if (name[3] == 'u')
		return PRIu16;
	      if (name[3] == 'x')
		return PRIx16;
	      if (name[3] == 'X')
		return PRIX16;
	      abort ();
	    }
	  if (name[4] == '3' && name[5] == '2' && name[6] == '\0')
	    {
	      if (name[3] == 'd')
		return PRId32;
	      if (name[3] == 'i')
		return PRIi32;
	      if (name[3] == 'o')
		return PRIo32;
	      if (name[3] == 'u')
		return PRIu32;
	      if (name[3] == 'x')
		return PRIx32;
	      if (name[3] == 'X')
		return PRIX32;
	      abort ();
	    }
	  if (name[4] == '6' && name[5] == '4' && name[6] == '\0')
	    {
	      if (name[3] == 'd')
		return PRId64;
	      if (name[3] == 'i')
		return PRIi64;
	      if (name[3] == 'o')
		return PRIo64;
	      if (name[3] == 'u')
		return PRIu64;
	      if (name[3] == 'x')
		return PRIx64;
	      if (name[3] == 'X')
		return PRIX64;
	      abort ();
	    }
	  if (name[4] == 'L' && name[5] == 'E' && name[6] == 'A'
	      && name[7] == 'S' && name[8] == 'T')
	    {
	      if (name[9] == '8' && name[10] == '\0')
		{
		  if (name[3] == 'd')
		    return PRIdLEAST8;
		  if (name[3] == 'i')
		    return PRIiLEAST8;
		  if (name[3] == 'o')
		    return PRIoLEAST8;
		  if (name[3] == 'u')
		    return PRIuLEAST8;
		  if (name[3] == 'x')
		    return PRIxLEAST8;
		  if (name[3] == 'X')
		    return PRIXLEAST8;
		  abort ();
		}
	      if (name[9] == '1' && name[10] == '6' && name[11] == '\0')
		{
		  if (name[3] == 'd')
		    return PRIdLEAST16;
		  if (name[3] == 'i')
		    return PRIiLEAST16;
		  if (name[3] == 'o')
		    return PRIoLEAST16;
		  if (name[3] == 'u')
		    return PRIuLEAST16;
		  if (name[3] == 'x')
		    return PRIxLEAST16;
		  if (name[3] == 'X')
		    return PRIXLEAST16;
		  abort ();
		}
	      if (name[9] == '3' && name[10] == '2' && name[11] == '\0')
		{
		  if (name[3] == 'd')
		    return PRIdLEAST32;
		  if (name[3] == 'i')
		    return PRIiLEAST32;
		  if (name[3] == 'o')
		    return PRIoLEAST32;
		  if (name[3] == 'u')
		    return PRIuLEAST32;
		  if (name[3] == 'x')
		    return PRIxLEAST32;
		  if (name[3] == 'X')
		    return PRIXLEAST32;
		  abort ();
		}
	      if (name[9] == '6' && name[10] == '4' && name[11] == '\0')
		{
		  if (name[3] == 'd')
		    return PRIdLEAST64;
		  if (name[3] == 'i')
		    return PRIiLEAST64;
		  if (name[3] == 'o')
		    return PRIoLEAST64;
		  if (name[3] == 'u')
		    return PRIuLEAST64;
		  if (name[3] == 'x')
		    return PRIxLEAST64;
		  if (name[3] == 'X')
		    return PRIXLEAST64;
		  abort ();
		}
	    }
	  if (name[4] == 'F' && name[5] == 'A' && name[6] == 'S'
	      && name[7] == 'T')
	    {
	      if (name[8] == '8' && name[9] == '\0')
		{
		  if (name[3] == 'd')
		    return PRIdFAST8;
		  if (name[3] == 'i')
		    return PRIiFAST8;
		  if (name[3] == 'o')
		    return PRIoFAST8;
		  if (name[3] == 'u')
		    return PRIuFAST8;
		  if (name[3] == 'x')
		    return PRIxFAST8;
		  if (name[3] == 'X')
		    return PRIXFAST8;
		  abort ();
		}
	      if (name[8] == '1' && name[9] == '6' && name[10] == '\0')
		{
		  if (name[3] == 'd')
		    return PRIdFAST16;
		  if (name[3] == 'i')
		    return PRIiFAST16;
		  if (name[3] == 'o')
		    return PRIoFAST16;
		  if (name[3] == 'u')
		    return PRIuFAST16;
		  if (name[3] == 'x')
		    return PRIxFAST16;
		  if (name[3] == 'X')
		    return PRIXFAST16;
		  abort ();
		}
	      if (name[8] == '3' && name[9] == '2' && name[10] == '\0')
		{
		  if (name[3] == 'd')
		    return PRIdFAST32;
		  if (name[3] == 'i')
		    return PRIiFAST32;
		  if (name[3] == 'o')
		    return PRIoFAST32;
		  if (name[3] == 'u')
		    return PRIuFAST32;
		  if (name[3] == 'x')
		    return PRIxFAST32;
		  if (name[3] == 'X')
		    return PRIXFAST32;
		  abort ();
		}
	      if (name[8] == '6' && name[9] == '4' && name[10] == '\0')
		{
		  if (name[3] == 'd')
		    return PRIdFAST64;
		  if (name[3] == 'i')
		    return PRIiFAST64;
		  if (name[3] == 'o')
		    return PRIoFAST64;
		  if (name[3] == 'u')
		    return PRIuFAST64;
		  if (name[3] == 'x')
		    return PRIxFAST64;
		  if (name[3] == 'X')
		    return PRIXFAST64;
		  abort ();
		}
	    }
	  if (name[4] == 'M' && name[5] == 'A' && name[6] == 'X'
	      && name[7] == '\0')
	    {
	      if (name[3] == 'd')
		return PRIdMAX;
	      if (name[3] == 'i')
		return PRIiMAX;
	      if (name[3] == 'o')
		return PRIoMAX;
	      if (name[3] == 'u')
		return PRIuMAX;
	      if (name[3] == 'x')
		return PRIxMAX;
	      if (name[3] == 'X')
		return PRIXMAX;
	      abort ();
	    }
	  if (name[4] == 'P' && name[5] == 'T' && name[6] == 'R'
	      && name[7] == '\0')
	    {
	      if (name[3] == 'd')
		return PRIdPTR;
	      if (name[3] == 'i')
		return PRIiPTR;
	      if (name[3] == 'o')
		return PRIoPTR;
	      if (name[3] == 'u')
		return PRIuPTR;
	      if (name[3] == 'x')
		return PRIxPTR;
	      if (name[3] == 'X')
		return PRIXPTR;
	      abort ();
	    }
	}
    }
  /* Test for a glibc specific printf() format directive flag.  */
  if (name[0] == 'I' && name[1] == '\0')
    {
#if defined _LIBC \
    || ((__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 2)) \
        && !defined __UCLIBC__)
      /* The 'I' flag, in numeric format directives, replaces ASCII digits
	 with the 'outdigits' defined in the LC_CTYPE locale facet.  This is
	 used for Farsi (Persian), some Indic languages, and maybe Arabic.  */
      return "I";
#else
      return "";
#endif
    }
  /* Other system dependent strings are not valid.  */
  return NULL;
}

/* Lock that protects the various 'struct loaded_l10nfile' objects.  */
__libc_lock_define_initialized_recursive (static, lock);

/* Load the message catalogs specified by FILENAME.  If it is no valid
   message catalog do nothing.  */
void
internal_function
_nl_load_domain (struct loaded_l10nfile *domain_file,
		 struct binding *domainbinding)
{
  int fd = -1;
  size_t size;
#ifdef _LIBC
  struct stat64 st;
#else
  struct stat st;
#endif
  struct mo_file_header *data = (struct mo_file_header *) -1;
  int use_mmap = 0;
  struct loaded_domain *domain;
  int revision;
  const char *nullentry;
  size_t nullentrylen;

  __libc_lock_lock_recursive (lock);
  if (domain_file->decided != 0)
    {
      /* There are two possibilities:

	 + this is the same thread calling again during this initialization
	   via _nl_find_msg.  We have initialized everything this call needs.

	 + this is another thread which tried to initialize this object.
	   Not necessary anymore since if the lock is available this
	   is finished.
      */
      goto done;
    }

  domain_file->decided = -1;
  domain_file->data = NULL;

  /* Note that it would be useless to store domainbinding in domain_file
     because domainbinding might be == NULL now but != NULL later (after
     a call to bind_textdomain_codeset).  */

  /* If the record does not represent a valid locale the FILENAME
     might be NULL.  This can happen when according to the given
     specification the locale file name is different for XPG and CEN
     syntax.  */
  if (domain_file->filename != NULL)
    {
      /* Try to open the addressed file.  */
      fd = open (domain_file->filename, O_RDONLY | O_BINARY);
      if (fd == -1)
        goto out;
    }
#if defined _WIN32 && !defined __CYGWIN__
  else if (domain_file->wfilename != NULL)
    {
      /* Try to open the addressed file.  */
      fd = _wopen (domain_file->wfilename, O_RDONLY | O_BINARY);
      if (fd == -1)
        goto out;
    }
#endif
  else
    goto out;

  /* We must know about the size of the file.  */
  if (
#ifdef _LIBC
      __builtin_expect (fstat64 (fd, &st) != 0, 0)
#else
      __builtin_expect (fstat (fd, &st) != 0, 0)
#endif
      || __builtin_expect ((size = (size_t) st.st_size) != st.st_size, 0)
      || __builtin_expect (size < sizeof (struct mo_file_header), 0))
    /* Something went wrong.  */
    goto out;

#ifdef HAVE_MMAP
  /* Now we are ready to load the file.  If mmap() is available we try
     this first.  If not available or it failed we try to load it.  */
  data = (struct mo_file_header *) mmap (NULL, size, PROT_READ,
					 MAP_PRIVATE, fd, 0);

  if (__builtin_expect (data != MAP_FAILED, 1))
    {
      /* mmap() call was successful.  */
      close (fd);
      fd = -1;
      use_mmap = 1;
    }

  assert (MAP_FAILED == (void *) -1);
#endif

  /* If the data is not yet available (i.e. mmap'ed) we try to load
     it manually.  */
  if (data == (struct mo_file_header *) -1)
    {
      size_t to_read;
      char *read_ptr;

      data = (struct mo_file_header *) malloc (size);
      if (data == NULL)
	goto out;

      to_read = size;
      read_ptr = (char *) data;
      do
	{
	  long int nb = (long int) read (fd, read_ptr, to_read);
	  if (nb <= 0)
	    {
#ifdef EINTR
	      if (nb == -1 && errno == EINTR)
		continue;
#endif
              free (data);
	      goto out;
	    }
	  read_ptr += nb;
	  to_read -= nb;
	}
      while (to_read > 0);

      close (fd);
      fd = -1;
    }

  /* Using the magic number we can test whether it really is a message
     catalog file.  */
  if (__builtin_expect (data->magic != _MAGIC && data->magic != _MAGIC_SWAPPED,
			0))
    {
      /* The magic number is wrong: not a message catalog file.  */
#ifdef HAVE_MMAP
      if (use_mmap)
	munmap ((caddr_t) data, size);
      else
#endif
	free (data);
      goto out;
    }

  domain = (struct loaded_domain *) malloc (sizeof (struct loaded_domain));
  if (domain == NULL)
    {
#ifdef HAVE_MMAP
      if (use_mmap)
	munmap ((caddr_t) data, size);
      else
#endif
	free (data);
      goto out;
    }
  domain_file->data = domain;

  domain->data = (char *) data;
  domain->use_mmap = use_mmap;
  domain->mmap_size = size;
  domain->must_swap = data->magic != _MAGIC;
  domain->malloced = NULL;

  /* Fill in the information about the available tables.  */
  revision = W (domain->must_swap, data->revision);
  /* We support only the major revisions 0 and 1.  */
  switch (revision >> 16)
    {
    case 0:
    case 1:
      domain->nstrings = W (domain->must_swap, data->nstrings);
      domain->orig_tab = (const struct string_desc *)
	((char *) data + W (domain->must_swap, data->orig_tab_offset));
      domain->trans_tab = (const struct string_desc *)
	((char *) data + W (domain->must_swap, data->trans_tab_offset));
      domain->hash_size = W (domain->must_swap, data->hash_tab_size);
      domain->hash_tab =
	(domain->hash_size > 2
	 ? (const nls_uint32 *)
	   ((char *) data + W (domain->must_swap, data->hash_tab_offset))
	 : NULL);
      domain->must_swap_hash_tab = domain->must_swap;

      /* Now dispatch on the minor revision.  */
      switch (revision & 0xffff)
	{
	case 0:
	  domain->n_sysdep_strings = 0;
	  domain->orig_sysdep_tab = NULL;
	  domain->trans_sysdep_tab = NULL;
	  break;
	case 1:
	default:
	  {
	    nls_uint32 n_sysdep_strings;

	    if (domain->hash_tab == NULL)
	      /* This is invalid.  These minor revisions need a hash table.  */
	      goto invalid;

	    n_sysdep_strings =
	      W (domain->must_swap, data->n_sysdep_strings);
	    if (n_sysdep_strings > 0)
	      {
		nls_uint32 n_sysdep_segments;
		const struct sysdep_segment *sysdep_segments;
		const char **sysdep_segment_values;
		const nls_uint32 *orig_sysdep_tab;
		const nls_uint32 *trans_sysdep_tab;
		nls_uint32 n_inmem_sysdep_strings;
		size_t memneed;
		char *mem;
		struct sysdep_string_desc *inmem_orig_sysdep_tab;
		struct sysdep_string_desc *inmem_trans_sysdep_tab;
		nls_uint32 *inmem_hash_tab;
		unsigned int i, j;

		/* Get the values of the system dependent segments.  */
		n_sysdep_segments =
		  W (domain->must_swap, data->n_sysdep_segments);
		sysdep_segments = (const struct sysdep_segment *)
		  ((char *) data
		   + W (domain->must_swap, data->sysdep_segments_offset));
		sysdep_segment_values =
		  (const char **)
		  alloca (n_sysdep_segments * sizeof (const char *));
		for (i = 0; i < n_sysdep_segments; i++)
		  {
		    const char *name =
		      (char *) data
		      + W (domain->must_swap, sysdep_segments[i].offset);
		    nls_uint32 namelen =
		      W (domain->must_swap, sysdep_segments[i].length);

		    if (!(namelen > 0 && name[namelen - 1] == '\0'))
		      {
			freea (sysdep_segment_values);
			goto invalid;
		      }

		    sysdep_segment_values[i] = get_sysdep_segment_value (name);
		  }

		orig_sysdep_tab = (const nls_uint32 *)
		  ((char *) data
		   + W (domain->must_swap, data->orig_sysdep_tab_offset));
		trans_sysdep_tab = (const nls_uint32 *)
		  ((char *) data
		   + W (domain->must_swap, data->trans_sysdep_tab_offset));

		/* Compute the amount of additional memory needed for the
		   system dependent strings and the augmented hash table.
		   At the same time, also drop string pairs which refer to
		   an undefined system dependent segment.  */
		n_inmem_sysdep_strings = 0;
		memneed = domain->hash_size * sizeof (nls_uint32);
		for (i = 0; i < n_sysdep_strings; i++)
		  {
		    int valid = 1;
		    size_t needs[2];

		    for (j = 0; j < 2; j++)
		      {
			const struct sysdep_string *sysdep_string =
			  (const struct sysdep_string *)
			  ((char *) data
			   + W (domain->must_swap,
				j == 0
				? orig_sysdep_tab[i]
				: trans_sysdep_tab[i]));
			size_t need = 0;
			const char *static_segments =
			  (char *) data
			  + W (domain->must_swap, sysdep_string->offset);
			const struct segment_pair *p = sysdep_string->segments;

			if (W (domain->must_swap, p->sysdepref) != SEGMENTS_END)
			  for (;; p++)
			    {
			      nls_uint32 segsize;
			      nls_uint32 sysdepref;

			      segsize = W (domain->must_swap, p->segsize);
			      need += segsize;

			      sysdepref = W (domain->must_swap, p->sysdepref);
			      if (sysdepref == SEGMENTS_END)
				break;

			      static_segments += segsize;

			      if (sysdepref >= n_sysdep_segments)
				{
				  /* Invalid.  */
				  freea (sysdep_segment_values);
				  goto invalid;
				}

			      if (sysdep_segment_values[sysdepref] == NULL)
				{
				  /* This particular string pair is invalid.  */
				  valid = 0;
				}

			      need += strlen (sysdep_segment_values[sysdepref]);
			    }

			/* The last static segment must end in a NUL.  */
			{
			  nls_uint32 segsize =
			    W (domain->must_swap, p->segsize);

			  if (!(segsize > 0
				&& static_segments[segsize - 1] == '\0'))
			    {
			      /* Invalid.  */
			      freea (sysdep_segment_values);
			      goto invalid;
			    }
			}

			needs[j] = need;
			if (!valid)
			  break;
		      }

		    if (valid)
		      {
			n_inmem_sysdep_strings++;
			memneed += needs[0] + needs[1];
		      }
		  }
		memneed += 2 * n_inmem_sysdep_strings
			   * sizeof (struct sysdep_string_desc);

		if (n_inmem_sysdep_strings > 0)
		  {
		    unsigned int k;

		    /* Allocate additional memory.  */
		    mem = (char *) malloc (memneed);
		    if (mem == NULL)
		      goto invalid;

		    domain->malloced = mem;
		    inmem_orig_sysdep_tab = (struct sysdep_string_desc *) mem;
		    mem += n_inmem_sysdep_strings
			   * sizeof (struct sysdep_string_desc);
		    inmem_trans_sysdep_tab = (struct sysdep_string_desc *) mem;
		    mem += n_inmem_sysdep_strings
			   * sizeof (struct sysdep_string_desc);
		    inmem_hash_tab = (nls_uint32 *) mem;
		    mem += domain->hash_size * sizeof (nls_uint32);

		    /* Compute the system dependent strings.  */
		    k = 0;
		    for (i = 0; i < n_sysdep_strings; i++)
		      {
			int valid = 1;

			for (j = 0; j < 2; j++)
			  {
			    const struct sysdep_string *sysdep_string =
			      (const struct sysdep_string *)
			      ((char *) data
			       + W (domain->must_swap,
				    j == 0
				    ? orig_sysdep_tab[i]
				    : trans_sysdep_tab[i]));
			    const struct segment_pair *p =
			      sysdep_string->segments;

			    if (W (domain->must_swap, p->sysdepref)
				!= SEGMENTS_END)
			      for (;; p++)
				{
				  nls_uint32 sysdepref;

				  sysdepref =
				    W (domain->must_swap, p->sysdepref);
				  if (sysdepref == SEGMENTS_END)
				    break;

				  if (sysdep_segment_values[sysdepref] == NULL)
				    {
				      /* This particular string pair is
					 invalid.  */
				      valid = 0;
				      break;
				    }
				}

			    if (!valid)
			      break;
			  }

			if (valid)
			  {
			    for (j = 0; j < 2; j++)
			      {
				const struct sysdep_string *sysdep_string =
				  (const struct sysdep_string *)
				  ((char *) data
				   + W (domain->must_swap,
					j == 0
					? orig_sysdep_tab[i]
					: trans_sysdep_tab[i]));
				const char *static_segments =
				  (char *) data
				  + W (domain->must_swap, sysdep_string->offset);
				const struct segment_pair *p =
				  sysdep_string->segments;

				/* Concatenate the segments, and fill
				   inmem_orig_sysdep_tab[k] (for j == 0) and
				   inmem_trans_sysdep_tab[k] (for j == 1).  */

				struct sysdep_string_desc *inmem_tab_entry =
				  (j == 0
				   ? inmem_orig_sysdep_tab
				   : inmem_trans_sysdep_tab)
				  + k;

				if (W (domain->must_swap, p->sysdepref)
				    == SEGMENTS_END)
				  {
				    /* Only one static segment.  */
				    inmem_tab_entry->length =
				      W (domain->must_swap, p->segsize);
				    inmem_tab_entry->pointer = static_segments;
				  }
				else
				  {
				    inmem_tab_entry->pointer = mem;

				    for (;; p++)
				      {
					nls_uint32 segsize =
					  W (domain->must_swap, p->segsize);
					nls_uint32 sysdepref =
					  W (domain->must_swap, p->sysdepref);
					size_t n;

					if (segsize > 0)
					  {
					    memcpy (mem, static_segments, segsize);
					    mem += segsize;
					    static_segments += segsize;
					  }

					if (sysdepref == SEGMENTS_END)
					  break;

					n = strlen (sysdep_segment_values[sysdepref]);
					memcpy (mem, sysdep_segment_values[sysdepref], n);
					mem += n;
				      }

				    inmem_tab_entry->length =
				      mem - inmem_tab_entry->pointer;
				  }
			      }

			    k++;
			  }
		      }
		    if (k != n_inmem_sysdep_strings)
		      abort ();

		    /* Compute the augmented hash table.  */
		    for (i = 0; i < domain->hash_size; i++)
		      inmem_hash_tab[i] =
			W (domain->must_swap_hash_tab, domain->hash_tab[i]);
		    for (i = 0; i < n_inmem_sysdep_strings; i++)
		      {
			const char *msgid = inmem_orig_sysdep_tab[i].pointer;
			nls_uint32 hash_val = __hash_string (msgid);
			nls_uint32 idx = hash_val % domain->hash_size;
			nls_uint32 incr =
			  1 + (hash_val % (domain->hash_size - 2));

			for (;;)
			  {
			    if (inmem_hash_tab[idx] == 0)
			      {
				/* Hash table entry is empty.  Use it.  */
				inmem_hash_tab[idx] = 1 + domain->nstrings + i;
				break;
			      }

			    if (idx >= domain->hash_size - incr)
			      idx -= domain->hash_size - incr;
			    else
			      idx += incr;
			  }
		      }

		    domain->n_sysdep_strings = n_inmem_sysdep_strings;
		    domain->orig_sysdep_tab = inmem_orig_sysdep_tab;
		    domain->trans_sysdep_tab = inmem_trans_sysdep_tab;

		    domain->hash_tab = inmem_hash_tab;
		    domain->must_swap_hash_tab = 0;
		  }
		else
		  {
		    domain->n_sysdep_strings = 0;
		    domain->orig_sysdep_tab = NULL;
		    domain->trans_sysdep_tab = NULL;
		  }

		freea (sysdep_segment_values);
	      }
	    else
	      {
		domain->n_sysdep_strings = 0;
		domain->orig_sysdep_tab = NULL;
		domain->trans_sysdep_tab = NULL;
	      }
	  }
	  break;
	}
      break;
    default:
      /* This is an invalid revision.  */
    invalid:
      /* This is an invalid .mo file or we ran out of resources.  */
      free (domain->malloced);
#ifdef HAVE_MMAP
      if (use_mmap)
	munmap ((caddr_t) data, size);
      else
#endif
	free (data);
      free (domain);
      domain_file->data = NULL;
      goto out;
    }

  /* No caches of converted translations so far.  */
  domain->conversions = NULL;
  domain->nconversions = 0;
#ifdef _LIBC
  __libc_rwlock_init (domain->conversions_lock);
#else
  gl_rwlock_init (domain->conversions_lock);
#endif

  /* Get the header entry and look for a plural specification.  */
#ifdef IN_LIBGLOCALE
  nullentry =
    _nl_find_msg (domain_file, domainbinding, NULL, "", &nullentrylen);
#else
  nullentry = _nl_find_msg (domain_file, domainbinding, "", 0, &nullentrylen);
#endif
  if (__builtin_expect (nullentry == (char *) -1, 0))
    {
#ifdef _LIBC
      __libc_rwlock_fini (domain->conversions_lock);
#else
      gl_rwlock_destroy (domain->conversions_lock);
#endif
      goto invalid;
    }
  EXTRACT_PLURAL_EXPRESSION (nullentry, &domain->plural, &domain->nplurals);

 out:
  if (fd != -1)
    close (fd);

  domain_file->decided = 1;

 done:
  __libc_lock_unlock_recursive (lock);
}


#ifdef _LIBC
void
internal_function __libc_freeres_fn_section
_nl_unload_domain (struct loaded_domain *domain)
{
  size_t i;

  if (domain->plural != &__gettext_germanic_plural)
    __gettext_free_exp ((struct expression *) domain->plural);

  for (i = 0; i < domain->nconversions; i++)
    {
      struct converted_domain *convd = &domain->conversions[i];

      free ((char *) convd->encoding);
      if (convd->conv_tab != NULL && convd->conv_tab != (char **) -1)
	free (convd->conv_tab);
      if (convd->conv != (__gconv_t) -1)
	__gconv_close (convd->conv);
    }
  free (domain->conversions);
  __libc_rwlock_fini (domain->conversions_lock);

  free (domain->malloced);

# ifdef _POSIX_MAPPED_FILES
  if (domain->use_mmap)
    munmap ((caddr_t) domain->data, domain->mmap_size);
  else
# endif	/* _POSIX_MAPPED_FILES */
    free ((void *) domain->data);

  free (domain);
}
#endif
