/* msvcrt workarounds.
   Copyright (C) 2025-2026 Free Software Foundation, Inc.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#include <config.h>

/* Specification.  */
#include <stdio.h>

#include <stdckdint.h>
#include <stdlib.h>
#include <string.h>

/* Outputs N bytes starting at S to FP.
   These N bytes are known to be followed by a NUL.
   Finally frees the string at S.
   Returns the number of written bytes.  */
static size_t
workaround_fwrite0 (char *s, size_t n, FILE *fp)
{
  const char *ptr = s;
  /* Use fputs instead of fwrite, which is buggy in msvcrt.  */
  size_t written = 0;
  while (n > 0)
    {
      size_t l = strlen (ptr); /* 0 <= l <= n */
      if (l > 0)
        {
          if (fputs (ptr, fp) == EOF)
            break;
          written += l;
          n -= l;
        }
      if (n == 0)
        break;
      if (fputc ('\0', fp) == EOF)
        break;
      written++;
      n--;
      ptr += l + 1;
    }
  free (s);
  return written;
}

size_t
gl_consolesafe_fwrite (const void *ptr, size_t size, size_t nmemb, FILE *fp)
{
  size_t nbytes;
  if (ckd_mul (&nbytes, size, nmemb) || nbytes == 0)
    /* Overflow, or nothing to do.  */
    return 0;
  char *tmp = malloc (nbytes + 1);
  if (tmp == NULL)
    return 0;
  memcpy (tmp, ptr, nbytes);
  tmp[nbytes] = '\0';
  size_t written = workaround_fwrite0 (tmp, nbytes, fp);
  return written / size;
}

#if defined __MINGW32__ && __USE_MINGW_ANSI_STDIO

# include "fseterr.h"
# include <stdarg.h>

# if !HAVE_VASPRINTF

#  include <errno.h>

/* The old mingw (before mingw-w64) does not have the vasprintf function.
   Define a suitable replacement here, that supports the same format
   specifiers as the mingw *printf functions.  */

static int
local_vasprintf (char **resultp, const char *format, va_list args)
{
  /* First try: Use a stack-allocated buffer.  */
  char buf[2048];
  size_t bufsize = sizeof (buf);
  int ret = __mingw_vsnprintf (buf, bufsize, format, args);
  if (ret < 0)
    return -1;
  size_t nbytes = ret;
  char *mem = (char *) malloc (nbytes + 1);
  if (mem == NULL)
    {
      errno = ENOMEM;
      return -1;
    }
  if (ret < bufsize)
    {
      /* The buffer was sufficiently large.  */
      memcpy (mem, buf, nbytes + 1);
    }
  else
    {
      /* Second try: Use the heap-allocated memory.  */
      ret = __mingw_vsnprintf (mem, nbytes + 1, format, args);
      if (ret < 0)
        {
          int saved_errno = errno;
          free (mem);
          errno = saved_errno;
          return -1;
        }
      if (ret != nbytes)
        abort ();
    }
  *resultp = mem;
  return nbytes;
}

#  undef vasprintf
#  define vasprintf local_vasprintf

# endif

/* Bypass the functions __mingw_[v][f]printf, that trigger a bug in msvcrt,
   but without losing the support for modern format specifiers added by
   __mingw_*printf.  */

int
gl_consolesafe_fprintf (FILE *restrict fp, const char *restrict format, ...)
{
  va_list args;
  va_start (args, format);
  char *tmpstring;
  int result = vasprintf (&tmpstring, format, args);
  va_end (args);
  if (result >= 0)
    {
      if (workaround_fwrite0 (tmpstring, result, fp) < result)
        result = -1;
    }
  else
    fseterr (fp);
  return result;
}

int
gl_consolesafe_printf (const char *restrict format, ...)
{
  va_list args;
  va_start (args, format);
  char *tmpstring;
  int result = vasprintf (&tmpstring, format, args);
  va_end (args);
  if (result >= 0)
    {
      if (workaround_fwrite0 (tmpstring, result, stdout) < result)
        result = -1;
    }
  else
    fseterr (stdout);
  return result;
}

int
gl_consolesafe_vfprintf (FILE *restrict fp,
                         const char *restrict format, va_list args)
{
  char *tmpstring;
  int result = vasprintf (&tmpstring, format, args);
  if (result >= 0)
    {
      if (workaround_fwrite0 (tmpstring, result, fp) < result)
        result = -1;
    }
  else
    fseterr (fp);
  return result;
}

int
gl_consolesafe_vprintf (const char *restrict format, va_list args)
{
  char *tmpstring;
  int result = vasprintf (&tmpstring, format, args);
  if (result >= 0)
    {
      if (workaround_fwrite0 (tmpstring, result, stdout) < result)
        result = -1;
    }
  else
    fseterr (stdout);
  return result;
}

#endif
