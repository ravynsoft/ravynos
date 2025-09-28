/* read-file.c -- read file contents into a string
   Copyright (C) 2006, 2009-2023 Free Software Foundation, Inc.
   Written by Simon Josefsson and Bruno Haible.

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

#include "read-file.h"

/* Get fstat.  */
#include <sys/stat.h>

/* Get ftello.  */
#include <stdio.h>

/* Get PTRDIFF_MAX.  */
#include <stdint.h>

/* Get malloc, realloc, free. */
#include <stdlib.h>

/* Get memcpy, memset_explicit. */
#include <string.h>

/* Get errno. */
#include <errno.h>

/* Read a STREAM and return a newly allocated string with the content,
   and set *LENGTH to the length of the string.  The string is
   zero-terminated, but the terminating zero byte is not counted in
   *LENGTH.  On errors, *LENGTH is undefined, errno preserves the
   values set by system functions (if any), and NULL is returned.

   If the RF_SENSITIVE flag is set in FLAGS:
     - You should control the buffering of STREAM using 'setvbuf'.  Either
       clear the buffer of STREAM after closing it, or disable buffering of
       STREAM before calling this function.
     - The memory buffer internally allocated will be cleared upon failure.  */
char *
fread_file (FILE *stream, int flags, size_t *length)
{
  char *buf = NULL;
  size_t alloc = BUFSIZ;

  /* For a regular file, allocate a buffer that has exactly the right
     size.  This avoids the need to do dynamic reallocations later.  */
  {
    struct stat st;

    if (fstat (fileno (stream), &st) >= 0 && S_ISREG (st.st_mode))
      {
        off_t pos = ftello (stream);

        if (pos >= 0 && pos < st.st_size)
          {
            off_t alloc_off = st.st_size - pos;

            /* '1' below, accounts for the trailing NUL.  */
            if (PTRDIFF_MAX - 1 < alloc_off)
              {
                errno = ENOMEM;
                return NULL;
              }

            alloc = alloc_off + 1;
          }
      }
  }

  if (!(buf = malloc (alloc)))
    return NULL; /* errno is ENOMEM.  */

  {
    size_t size = 0; /* number of bytes read so far */
    int save_errno;

    for (;;)
      {
        /* This reads 1 more than the size of a regular file
           so that we get eof immediately.  */
        size_t requested = alloc - size;
        size_t count = fread (buf + size, 1, requested, stream);
        size += count;

        if (count != requested)
          {
            save_errno = errno;
            if (ferror (stream))
              break;

            /* Shrink the allocated memory if possible.  */
            if (size < alloc - 1)
              {
                if (flags & RF_SENSITIVE)
                  {
                    char *smaller_buf = malloc (size + 1);
                    if (smaller_buf == NULL)
                      memset_explicit (buf + size, 0, alloc - size);
                    else
                      {
                        memcpy (smaller_buf, buf, size);
                        memset_explicit (buf, 0, alloc);
                        free (buf);
                        buf = smaller_buf;
                      }
                  }
                else
                  {
                    char *smaller_buf = realloc (buf, size + 1);
                    if (smaller_buf != NULL)
                      buf = smaller_buf;
                  }
              }

            buf[size] = '\0';
            *length = size;
            return buf;
          }

        {
          char *new_buf;
          size_t save_alloc = alloc;

          if (alloc == PTRDIFF_MAX)
            {
              save_errno = ENOMEM;
              break;
            }

          if (alloc < PTRDIFF_MAX - alloc / 2)
            alloc = alloc + alloc / 2;
          else
            alloc = PTRDIFF_MAX;

          if (flags & RF_SENSITIVE)
            {
              new_buf = malloc (alloc);
              if (!new_buf)
                {
                  /* BUF should be cleared below after the loop.  */
                  save_errno = errno;
                  break;
                }
              memcpy (new_buf, buf, save_alloc);
              memset_explicit (buf, 0, save_alloc);
              free (buf);
            }
          else if (!(new_buf = realloc (buf, alloc)))
            {
              save_errno = errno;
              break;
            }

          buf = new_buf;
        }
      }

    if (flags & RF_SENSITIVE)
      memset_explicit (buf, 0, alloc);

    free (buf);
    errno = save_errno;
    return NULL;
  }
}

/* Open and read the contents of FILENAME, and return a newly
   allocated string with the content, and set *LENGTH to the length of
   the string.  The string is zero-terminated, but the terminating
   zero byte is not counted in *LENGTH.  On errors, *LENGTH is
   undefined, errno preserves the values set by system functions (if
   any), and NULL is returned.

   If the RF_BINARY flag is set in FLAGS, the file is opened in binary
   mode.  If the RF_SENSITIVE flag is set in FLAGS, the memory buffer
   internally allocated will be cleared upon failure.  */
char *
read_file (const char *filename, int flags, size_t *length)
{
  const char *mode = (flags & RF_BINARY) ? "rbe" : "re";
  FILE *stream = fopen (filename, mode);
  char *out;

  if (!stream)
    return NULL;

  if (flags & RF_SENSITIVE)
    setvbuf (stream, NULL, _IONBF, 0);

  out = fread_file (stream, flags, length);

  if (fclose (stream) != 0)
    {
      if (out)
        {
          if (flags & RF_SENSITIVE)
            memset_explicit (out, 0, *length);
          free (out);
        }
      return NULL;
    }

  return out;
}
