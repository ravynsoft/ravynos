/* Test harness for pipe-filter-ii.

   Copyright (C) 2009-2023 Free Software Foundation, Inc.
   Written by Paolo Bonzini <bonzini@gnu.org>, 2009.

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

#include <config.h>

#include "pipe-filter.h"

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include "binary-io.h"
#include "full-write.h"
#include "macros.h"

struct locals
{
  const char *input;
  size_t size;
  size_t nwritten;
  size_t nread;
  char buf[5];
};

static const void *
prepare_write (size_t *num_bytes_p, void *private_data)
{
  struct locals *l = (struct locals *) private_data;
  if (l->nwritten < l->size)
    {
      *num_bytes_p = l->size - l->nwritten;
      return l->input + l->nwritten;
    }
  else
    return NULL;
}

static void
done_write (void *data_written, size_t num_bytes_written, void *private_data)
{
  struct locals *l = (struct locals *) private_data;
  l->nwritten += num_bytes_written;
}

static void *
prepare_read (size_t *num_bytes_p, void *private_data)
{
  struct locals *l = (struct locals *) private_data;
  *num_bytes_p = sizeof (l->buf);
  return l->buf;
}

/* Callback that ignores the data that has been read.  */

static void
ignore_done_read (void *data_read, size_t num_bytes_read, void *private_data)
{
}

/* Callback that outputs the data that has been read.  */

static void
output_done_read (void *data_read, size_t num_bytes_read, void *private_data)
{
  full_write (STDOUT_FILENO, data_read, num_bytes_read);
}

int
main (int argc, char **argv)
{
  const char *path[] = { NULL, NULL };

  ASSERT (argc == 2);

  set_binary_mode (STDOUT_FILENO, O_BINARY);

  /* Test writing to a nonexistent program traps sooner or later.  */
  {
    struct locals l;
    int rc;

    l.input = "";
    l.size = 1;
    l.nwritten = 0;
    l.nread = 0;
    path[0] = "/nonexistent/blah";
    rc = pipe_filter_ii_execute ("pipe-filter-test", path[0], path, true, false,
                                 prepare_write, done_write,
                                 prepare_read, ignore_done_read,
                                 &l);
    ASSERT (rc == 127 || rc == -1);
    printf ("Test 1 passed.\n");
    fflush (stdout);
  }

  /* Test returning the exit status.  */
  {
    struct locals l;
    int rc;

    l.input = "1 -1";
    l.size = strlen (l.input);
    l.nwritten = 0;
    l.nread = 0;
    path[0] = argv[1];
    rc = pipe_filter_ii_execute ("pipe-filter-test", path[0], path, false, false,
                                 prepare_write, done_write,
                                 prepare_read, ignore_done_read,
                                 &l);
    ASSERT (rc == 1);
    printf ("Test 2 passed.\n");
    fflush (stdout);
  }

  /* Now test asynchronous I/O.  */
  {
    struct locals l;
    int rc;

    l.input = "1 50\n51\n100";
    l.size = strlen (l.input);
    l.nwritten = 0;
    l.nread = 0;
    path[0] = argv[1];
    rc = pipe_filter_ii_execute ("pipe-filter-test", path[0], path, false, true,
                                 prepare_write, done_write,
                                 prepare_read, output_done_read,
                                 &l);
    ASSERT (rc == 0);
    printf ("Test 3 passed.\n");
    fflush (stdout);
  }

  return 0;
}
