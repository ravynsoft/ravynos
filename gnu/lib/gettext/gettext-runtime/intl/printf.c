/* Formatted output to strings, using POSIX/XSI format strings with positions.
   Copyright (C) 2003, 2006-2007, 2009-2011, 2018, 2020-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2003.

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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef __GNUC__
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

#include <stdio.h>

#if !HAVE_POSIX_PRINTF

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

/* Specifications of the libintl_*printf functions.  */
#include "libgnuintl.h"

/* Some systems, like OSF/1 4.0 and Woe32, don't have EOVERFLOW.  */
#ifndef EOVERFLOW
# define EOVERFLOW E2BIG
#endif

/* When building a DLL, we must export some functions.  Note that because
   the functions are only defined for binary backward compatibility, we
   don't need to use __declspec(dllimport) in any case.  */
#if HAVE_VISIBILITY && BUILDING_DLL
# define DLL_EXPORTED __attribute__((__visibility__("default")))
#elif defined _MSC_VER && BUILDING_DLL
/* When building with MSVC, exporting a symbol means that the object file
   contains a "linker directive" of the form /EXPORT:symbol.  This can be
   inspected through the "objdump -s --section=.drectve FILE" or
   "dumpbin /directives FILE" commands.
   The symbols from this file should be exported if and only if the object
   file gets included in a DLL.  Libtool, on Windows platforms, defines
   the C macro DLL_EXPORT (together with PIC) when compiling for a DLL
   and does not define it when compiling an object file meant to be linked
   statically into some executable.  */
# if defined DLL_EXPORT
#  define DLL_EXPORTED __declspec(dllexport)
# else
#  define DLL_EXPORTED
# endif
#else
# define DLL_EXPORTED
#endif

#define STATIC static

/* You can enable this for debugging on Windows.  But not in a release!  */
#if 0
# define ENABLE_WCHAR_FALLBACK 1
#endif

/* This needs to be consistent with libgnuintl.in.h.  */
#if defined __NetBSD__ || defined __BEOS__ || defined __CYGWIN__ || defined __MINGW32__
/* Don't break __attribute__((format(printf,M,N))).
   This redefinition is only possible because the libc in NetBSD, Cygwin,
   mingw does not have a function __printf__.  */
# define libintl_printf __printf__
#endif

#if 0 /* not needed */

/* Define auxiliary functions declared in "printf-args.h".  */
#include "printf-args.c"

/* Define auxiliary functions declared in "printf-parse.h".  */
#include "printf-parse.c"

/* Define functions declared in "vasnprintf.h".  */
#define vasnprintf _libintl_vasnprintf
#include "vasnprintf.c"
#define asnprintf _libintl_asnprintf
#include "asnprintf.c"

#else

/* Get the declaration of _libintl_vasnprintf.  */
#include "vasnprintf.h"

#endif

/* Users don't expect libintl_fprintf to be less POSIX compliant
   than the fprintf implementation provided by gnulib or - on mingw -
   the one provided by mingw libs when __USE_MINGW_ANSI_STDIO is in
   effect.  */
#define USE_REPLACEMENT_CODE_ALWAYS 1

DLL_EXPORTED
int
libintl_vfprintf (FILE *stream, const char *format, va_list args)
{
#if !USE_REPLACEMENT_CODE_ALWAYS
  if (strchr (format, '$') == NULL)
    return vfprintf (stream, format, args);
  else
#endif
    {
      size_t length;
      char *result = _libintl_vasnprintf (NULL, &length, format, args);
      int retval = -1;
      if (result != NULL)
        {
          size_t written = fwrite (result, 1, length, stream);
          free (result);
          if (written == length)
            {
              if (length > INT_MAX)
                errno = EOVERFLOW;
              else
                retval = length;
            }
        }
      return retval;
    }
}

DLL_EXPORTED
int
libintl_fprintf (FILE *stream, const char *format, ...)
{
  va_list args;
  int retval;

  va_start (args, format);
  retval = libintl_vfprintf (stream, format, args);
  va_end (args);
  return retval;
}

DLL_EXPORTED
int
libintl_vprintf (const char *format, va_list args)
{
  return libintl_vfprintf (stdout, format, args);
}

DLL_EXPORTED
int
libintl_printf (const char *format, ...)
{
  va_list args;
  int retval;

  va_start (args, format);
  retval = libintl_vprintf (format, args);
  va_end (args);
  return retval;
}

DLL_EXPORTED
int
libintl_vsprintf (char *resultbuf, const char *format, va_list args)
{
#if !USE_REPLACEMENT_CODE_ALWAYS
  if (strchr (format, '$') == NULL)
    return vsprintf (resultbuf, format, args);
  else
#endif
    {
      size_t length = (size_t) ~0 / (4 * sizeof (char));
      char *result = _libintl_vasnprintf (resultbuf, &length, format, args);
      if (result != resultbuf)
        {
          free (result);
          return -1;
        }
      if (length > INT_MAX)
        {
          errno = EOVERFLOW;
          return -1;
        }
      else
        return length;
    }
}

DLL_EXPORTED
int
libintl_sprintf (char *resultbuf, const char *format, ...)
{
  va_list args;
  int retval;

  va_start (args, format);
  retval = libintl_vsprintf (resultbuf, format, args);
  va_end (args);
  return retval;
}

#if HAVE_SNPRINTF

# if HAVE_DECL__SNPRINTF
   /* Windows.  The mingw function vsnprintf() has fewer bugs than the MSVCRT
      function _vsnprintf(), so prefer that.  */
#  if defined __MINGW32__
#   define system_vsnprintf vsnprintf
#  else
#   define system_vsnprintf _vsnprintf
#  endif
# else
   /* Unix.  */
#  define system_vsnprintf vsnprintf
# endif

DLL_EXPORTED
int
libintl_vsnprintf (char *resultbuf, size_t length, const char *format, va_list args)
{
# if !USE_REPLACEMENT_CODE_ALWAYS
  if (strchr (format, '$') == NULL)
    return system_vsnprintf (resultbuf, length, format, args);
  else
# endif
    {
      size_t maxlength = length;
      char *result = _libintl_vasnprintf (resultbuf, &length, format, args);
      if (result == NULL)
        return -1;
      if (result != resultbuf)
        {
          if (maxlength > 0)
            {
              size_t pruned_length =
                (length < maxlength ? length : maxlength - 1);
              memcpy (resultbuf, result, pruned_length);
              resultbuf[pruned_length] = '\0';
            }
          free (result);
        }
      if (length > INT_MAX)
        {
          errno = EOVERFLOW;
          return -1;
        }
      else
        return length;
    }
}

DLL_EXPORTED
int
libintl_snprintf (char *resultbuf, size_t length, const char *format, ...)
{
  va_list args;
  int retval;

  va_start (args, format);
  retval = libintl_vsnprintf (resultbuf, length, format, args);
  va_end (args);
  return retval;
}

#endif

#if HAVE_ASPRINTF

DLL_EXPORTED
int
libintl_vasprintf (char **resultp, const char *format, va_list args)
{
  size_t length;
  char *result = _libintl_vasnprintf (NULL, &length, format, args);
  if (result == NULL)
    return -1;
  if (length > INT_MAX)
    {
      free (result);
      errno = EOVERFLOW;
      return -1;
    }
  *resultp = result;
  return length;
}

DLL_EXPORTED
int
libintl_asprintf (char **resultp, const char *format, ...)
{
  va_list args;
  int retval;

  va_start (args, format);
  retval = libintl_vasprintf (resultp, format, args);
  va_end (args);
  return retval;
}

#endif

#if HAVE_WPRINTF

#include <wchar.h>

#if 0 /* not needed */

/* Define auxiliary functions declared in "printf-args.h".  */
#include "printf-args.c"

/* Define auxiliary functions declared in "wprintf-parse.h".  */
#include "wprintf-parse.c"

/* Define functions declared in "vasnwprintf.h".  */
#define vasnwprintf _libintl_vasnwprintf
#include "vasnwprintf.c"
#define asnwprintf _libintl_asnwprintf
#include "asnwprintf.c"

#else

/* Get the declaration of _libintl_vasnwprintf.  */
#include "vasnwprintf.h"

#endif

# if HAVE_DECL__SNWPRINTF
   /* Windows.  The function vswprintf() has a different signature than
      on Unix; we use the function _vsnwprintf() instead.  */
#  define system_vswprintf _vsnwprintf
# else
   /* Unix.  */
#  define system_vswprintf vswprintf
# endif

DLL_EXPORTED
int
libintl_vfwprintf (FILE *stream, const wchar_t *format, va_list args)
{
# if !USE_REPLACEMENT_CODE_ALWAYS
  if (wcschr (format, '$') == NULL)
    return vfwprintf (stream, format, args);
  else
# endif
    {
      size_t length;
      wchar_t *result = _libintl_vasnwprintf (NULL, &length, format, args);
      int retval = -1;
      if (result != NULL)
        {
          size_t i;
          for (i = 0; i < length; i++)
            if (fputwc (result[i], stream) == WEOF)
              break;
          free (result);
          if (i == length)
            {
              if (length > INT_MAX)
                errno = EOVERFLOW;
              else
                retval = length;
            }
        }
      return retval;
    }
}

DLL_EXPORTED
int
libintl_fwprintf (FILE *stream, const wchar_t *format, ...)
{
  va_list args;
  int retval;

  va_start (args, format);
  retval = libintl_vfwprintf (stream, format, args);
  va_end (args);
  return retval;
}

DLL_EXPORTED
int
libintl_vwprintf (const wchar_t *format, va_list args)
{
  return libintl_vfwprintf (stdout, format, args);
}

DLL_EXPORTED
int
libintl_wprintf (const wchar_t *format, ...)
{
  va_list args;
  int retval;

  va_start (args, format);
  retval = libintl_vwprintf (format, args);
  va_end (args);
  return retval;
}

DLL_EXPORTED
int
libintl_vswprintf (wchar_t *resultbuf, size_t length, const wchar_t *format, va_list args)
{
# if !USE_REPLACEMENT_CODE_ALWAYS
  if (wcschr (format, '$') == NULL)
    return system_vswprintf (resultbuf, length, format, args);
  else
# endif
    {
      size_t maxlength = length;
      wchar_t *result = _libintl_vasnwprintf (resultbuf, &length, format, args);
      if (result == NULL)
        return -1;
      if (result != resultbuf)
        {
          if (maxlength > 0)
            {
              size_t pruned_length =
                (length < maxlength ? length : maxlength - 1);
              memcpy (resultbuf, result, pruned_length * sizeof (wchar_t));
              resultbuf[pruned_length] = 0;
            }
          free (result);
          /* Unlike vsnprintf, which has to return the number of character that
             would have been produced if the resultbuf had been sufficiently
             large, the vswprintf function has to return a negative value if
             the resultbuf was not sufficiently large.  */
          if (length >= maxlength)
            return -1;
        }
      if (length > INT_MAX)
        {
          errno = EOVERFLOW;
          return -1;
        }
      else
        return length;
    }
}

DLL_EXPORTED
int
libintl_swprintf (wchar_t *resultbuf, size_t length, const wchar_t *format, ...)
{
  va_list args;
  int retval;

  va_start (args, format);
  retval = libintl_vswprintf (resultbuf, length, format, args);
  va_end (args);
  return retval;
}

#endif

#endif
