/* Copyright (C) 2011-2023 Free Software Foundation, Inc.
   This file is part of gnulib.

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

/* Specification */
#include <unistd.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#if GNULIB_GETCWD
/* Favor GPL getcwd.c if both getcwd and getcwd-lgpl modules are in use.  */
typedef int dummy;
#else

/* Get the name of the current working directory, and put it in SIZE
   bytes of BUF.  Returns NULL if the directory couldn't be determined
   (perhaps because the absolute name was longer than PATH_MAX, or
   because of missing read/search permissions on parent directories)
   or SIZE was too small.  If successful, returns BUF.  If BUF is
   NULL, an array is allocated with 'malloc'; the array is SIZE bytes
   long, unless SIZE == 0, in which case it is as big as
   necessary.  */

# undef getcwd
# if defined _WIN32 && !defined __CYGWIN__
#  define getcwd _getcwd
# endif

char *
rpl_getcwd (char *buf, size_t size)
{
  char *result;

  /* Handle single size operations.  */
  if (buf)
    {
      /* Check SIZE argument.  */
      if (!size)
        {
          errno = EINVAL;
          return NULL;
        }
      return getcwd (buf, size);
    }

  if (size)
    {
      buf = malloc (size);
      if (!buf)
        {
          errno = ENOMEM;
          return NULL;
        }
      result = getcwd (buf, size);
      if (!result)
        free (buf);
      return result;
    }

  /* Flexible sizing requested.  Avoid over-allocation for the common
     case of a name that fits within a 4k page, minus some space for
     local variables, to be sure we don't skip over a guard page.  */
  {
    char tmp[4032];
    size = sizeof tmp;
    char *ptr = getcwd (tmp, size);
    if (ptr)
      {
        result = strdup (ptr);
        if (!result)
          errno = ENOMEM;
        return result;
      }
    if (errno != ERANGE)
      return NULL;
  }

  /* My what a large directory name we have.  */
  do
    {
      size <<= 1;
      char *ptr = realloc (buf, size);
      if (ptr == NULL)
        {
          free (buf);
          errno = ENOMEM;
          return NULL;
        }
      buf = ptr;
      result = getcwd (buf, size);
    }
  while (!result && errno == ERANGE);

  if (!result)
    free (buf);
  else
    {
      /* Here result == buf.  */
      /* Shrink result before returning it.  */
      size_t actual_size = strlen (result) + 1;
      if (actual_size < size)
        {
          char *shrinked_result = realloc (result, actual_size);
          if (shrinked_result != NULL)
            result = shrinked_result;
        }
    }
  return result;
}

#endif
