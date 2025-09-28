/* Test of scratch_buffer functions.
   Copyright (C) 2018-2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <bruno@clisp.org>, 2018.  */

#include <config.h>

#include <scratch_buffer.h>

#include <string.h>
#include "macros.h"

static int
byte_at (unsigned long long int i)
{
  return ((i % 13) + ((i * i) % 251)) & 0xff;
}

int
main ()
{
  /* Check scratch_buffer_set_array_size.  */
  {
    size_t sizes[] = { 100, 1000, 10000, 100000 };
    size_t s;
    for (s = 0; s < SIZEOF (sizes); s++)
      {
        size_t size = sizes[s];
        struct scratch_buffer buf;
        bool ok;
        size_t i;

        scratch_buffer_init (&buf);

        ok = scratch_buffer_set_array_size (&buf, size, 1);
        ASSERT (ok);

        for (i = 0; i < size; i++)
          ((unsigned char *) buf.data)[i] = byte_at (i);

        memset (buf.data, 'x', buf.length);
        memset (buf.data, 'y', size);

        scratch_buffer_free (&buf);
      }
  }

  /* Check scratch_buffer_grow.  */
  {
    size_t sizes[] = { 100, 1000, 10000, 100000 };
    size_t s;
    for (s = 0; s < SIZEOF (sizes); s++)
      {
        size_t size = sizes[s];
        struct scratch_buffer buf;
        bool ok;
        size_t i;

        scratch_buffer_init (&buf);

        while (buf.length < size)
          {
            ok = scratch_buffer_grow (&buf);
            ASSERT (ok);
          }

        for (i = 0; i < size; i++)
          ((unsigned char *) buf.data)[i] = byte_at (i);

        memset (buf.data, 'x', buf.length);
        memset (buf.data, 'y', size);

        scratch_buffer_free (&buf);
      }
  }

  /* Check scratch_buffer_grow_preserve.  */
  {
    size_t sizes[] = { 100, 1000, 10000, 100000 };
    struct scratch_buffer buf;
    size_t s;
    size_t size;
    bool ok;
    size_t i;

    scratch_buffer_init (&buf);

    s = 0;
    size = sizes[s];
    ok = scratch_buffer_set_array_size (&buf, size, 1);
    ASSERT (ok);

    for (i = 0; i < size; i++)
      ((unsigned char *) buf.data)[i] = byte_at (i);

    for (; s < SIZEOF (sizes); s++)
      {
        size_t oldsize = size;
        size = sizes[s];

        while (buf.length < size)
          {
            ok = scratch_buffer_grow_preserve (&buf);
            ASSERT (ok);
          }

        for (i = 0; i < oldsize; i++)
          ASSERT(((unsigned char *) buf.data)[i] == byte_at (i));
        for (i = oldsize; i < size; i++)
          ((unsigned char *) buf.data)[i] = byte_at (i);
      }

    scratch_buffer_free (&buf);
  }

  return 0;
}
