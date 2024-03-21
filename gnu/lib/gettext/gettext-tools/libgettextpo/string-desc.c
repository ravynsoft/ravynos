/* String descriptors.
   Copyright (C) 2023 Free Software Foundation, Inc.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation, either version 3 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Bruno Haible <bruno@clisp.org>, 2023.  */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#define GL_STRING_DESC_INLINE _GL_EXTERN_INLINE

/* Specification and inline definitions.  */
#include "string-desc.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "ialloc.h"
#include "full-write.h"


/* ==== Side-effect-free operations on string descriptors ==== */

/* Return true if A and B are equal.  */
bool
string_desc_equals (string_desc_t a, string_desc_t b)
{
  return (a._nbytes == b._nbytes
          && (a._nbytes == 0 || memcmp (a._data, b._data, a._nbytes) == 0));
}

bool
string_desc_startswith (string_desc_t s, string_desc_t prefix)
{
  return (s._nbytes >= prefix._nbytes
          && (prefix._nbytes == 0
              || memcmp (s._data, prefix._data, prefix._nbytes) == 0));
}

bool
string_desc_endswith (string_desc_t s, string_desc_t suffix)
{
  return (s._nbytes >= suffix._nbytes
          && (suffix._nbytes == 0
              || memcmp (s._data + (s._nbytes - suffix._nbytes), suffix._data,
                         suffix._nbytes) == 0));
}

int
string_desc_cmp (string_desc_t a, string_desc_t b)
{
  if (a._nbytes > b._nbytes)
    {
      if (b._nbytes == 0)
        return 1;
      return (memcmp (a._data, b._data, b._nbytes) < 0 ? -1 : 1);
    }
  else if (a._nbytes < b._nbytes)
    {
      if (a._nbytes == 0)
        return -1;
      return (memcmp (a._data, b._data, a._nbytes) > 0 ? 1 : -1);
    }
  else /* a._nbytes == b._nbytes */
    {
      if (a._nbytes == 0)
        return 0;
      return memcmp (a._data, b._data, a._nbytes);
    }
}

ptrdiff_t
string_desc_index (string_desc_t s, char c)
{
  if (s._nbytes > 0)
    {
      void *found = memchr (s._data, (unsigned char) c, s._nbytes);
      if (found != NULL)
        return (char *) found - s._data;
    }
  return -1;
}

ptrdiff_t
string_desc_last_index (string_desc_t s, char c)
{
  if (s._nbytes > 0)
    {
      void *found = memrchr (s._data, (unsigned char) c, s._nbytes);
      if (found != NULL)
        return (char *) found - s._data;
    }
  return -1;
}

string_desc_t
string_desc_new_empty (void)
{
  string_desc_t result;

  result._nbytes = 0;
  result._data = NULL;

  return result;

}

string_desc_t
string_desc_from_c (const char *s)
{
  string_desc_t result;

  result._nbytes = strlen (s);
  result._data = (char *) s;

  return result;
}

string_desc_t
string_desc_substring (string_desc_t s, idx_t start, idx_t end)
{
  string_desc_t result;

  if (!(start >= 0 && start <= end))
    /* Invalid arguments.  */
    abort ();

  result._nbytes = end - start;
  result._data = s._data + start;

  return result;
}

int
string_desc_write (int fd, string_desc_t s)
{
  if (s._nbytes > 0)
    if (full_write (fd, s._data, s._nbytes) != s._nbytes)
      /* errno is set here.  */
      return -1;
  return 0;
}

int
string_desc_fwrite (FILE *fp, string_desc_t s)
{
  if (s._nbytes > 0)
    if (fwrite (s._data, 1, s._nbytes, fp) != s._nbytes)
      return -1;
  return 0;
}


/* ==== Memory-allocating operations on string descriptors ==== */

int
string_desc_new (string_desc_t *resultp, idx_t n)
{
  string_desc_t result;

  if (!(n >= 0))
    /* Invalid argument.  */
    abort ();

  result._nbytes = n;
  if (n == 0)
    result._data = NULL;
  else
    {
      result._data = (char *) imalloc (n);
      if (result._data == NULL)
        /* errno is set here.  */
        return -1;
    }

  *resultp = result;
  return 0;
}

string_desc_t
string_desc_new_addr (idx_t n, char *addr)
{
  string_desc_t result;

  result._nbytes = n;
  if (n == 0)
    result._data = NULL;
  else
    result._data = addr;

  return result;
}

int
string_desc_new_filled (string_desc_t *resultp, idx_t n, char c)
{
  string_desc_t result;

  result._nbytes = n;
  if (n == 0)
    result._data = NULL;
  else
    {
      result._data = (char *) imalloc (n);
      if (result._data == NULL)
        /* errno is set here.  */
        return -1;
      memset (result._data, (unsigned char) c, n);
    }

  *resultp = result;
  return 0;
}

int
string_desc_copy (string_desc_t *resultp, string_desc_t s)
{
  string_desc_t result;
  idx_t n = s._nbytes;

  result._nbytes = n;
  if (n == 0)
    result._data = NULL;
  else
    {
      result._data = (char *) imalloc (n);
      if (result._data == NULL)
        /* errno is set here.  */
        return -1;
      memcpy (result._data, s._data, n);
    }

  *resultp = result;
  return 0;
}

int
string_desc_concat (string_desc_t *resultp, idx_t n, string_desc_t string1, ...)
{
  if (n <= 0)
    /* Invalid argument.  */
    abort ();

  idx_t total = 0;
  total += string1._nbytes;
  if (n > 1)
    {
      va_list other_strings;
      idx_t i;

      va_start (other_strings, string1);
      for (i = n - 1; i > 0; i--)
        {
          string_desc_t arg = va_arg (other_strings, string_desc_t);
          total += arg._nbytes;
        }
      va_end (other_strings);
    }

  char *combined = (char *) imalloc (total);
  if (combined == NULL)
    /* errno is set here.  */
    return -1;
  idx_t pos = 0;
  memcpy (combined, string1._data, string1._nbytes);
  pos += string1._nbytes;
  if (n > 1)
    {
      va_list other_strings;
      idx_t i;

      va_start (other_strings, string1);
      for (i = n - 1; i > 0; i--)
        {
          string_desc_t arg = va_arg (other_strings, string_desc_t);
          if (arg._nbytes > 0)
            memcpy (combined + pos, arg._data, arg._nbytes);
          pos += arg._nbytes;
        }
      va_end (other_strings);
    }

  string_desc_t result;
  result._nbytes = total;
  result._data = combined;

  *resultp = result;
  return 0;
}

char *
string_desc_c (string_desc_t s)
{
  idx_t n = s._nbytes;
  char *result = (char *) imalloc (n + 1);
  if (result == NULL)
    /* errno is set here.  */
    return NULL;
  if (n > 0)
    memcpy (result, s._data, n);
  result[n] = '\0';

  return result;
}


/* ==== Operations with side effects on string descriptors ==== */

void
string_desc_set_char_at (string_desc_t s, idx_t i, char c)
{
  if (!(i >= 0 && i < s._nbytes))
    /* Invalid argument.  */
    abort ();
  s._data[i] = c;
}

void
string_desc_fill (string_desc_t s, idx_t start, idx_t end, char c)
{
  if (!(start >= 0 && start <= end))
    /* Invalid arguments.  */
    abort ();

  if (start < end)
    memset (s._data + start, (unsigned char) c, end - start);
}

void
string_desc_overwrite (string_desc_t s, idx_t start, string_desc_t t)
{
  if (!(start >= 0 && start + t._nbytes <= s._nbytes))
    /* Invalid arguments.  */
    abort ();

  if (t._nbytes > 0)
    memcpy (s._data + start, t._data, t._nbytes);
}

void
string_desc_free (string_desc_t s)
{
  free (s._data);
}
