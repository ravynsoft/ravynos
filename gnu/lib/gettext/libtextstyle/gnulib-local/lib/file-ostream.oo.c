/* Output stream referring to an stdio FILE.
   Copyright (C) 2006, 2019-2020 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2006.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#include <config.h>

/* Specification.  */
#include "file-ostream.h"

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#if HAVE_TCDRAIN
# include <termios.h>
#endif

#include "xalloc.h"

struct file_ostream : struct ostream
{
fields:
  FILE *fp;
};

#if HAVE_TCDRAIN

/* EINTR handling for tcdrain().
   This function can return -1/EINTR even though we don't have any
   signal handlers set up, namely when we get interrupted via SIGSTOP.  */

static inline int
nonintr_tcdrain (int fd)
{
  int retval;

  do
    retval = tcdrain (fd);
  while (retval < 0 && errno == EINTR);

  return retval;
}

#endif

/* Implementation of ostream_t methods.  */

static void
file_ostream::write_mem (file_ostream_t stream, const void *data, size_t len)
{
  if (len > 0)
    fwrite (data, 1, len, stream->fp);
}

static void
file_ostream::flush (file_ostream_t stream, ostream_flush_scope_t scope)
{
  /* This ostream has no internal buffer, therefore nothing to do for
     scope == FLUSH_THIS_STREAM.  */
  if (scope != FLUSH_THIS_STREAM)
    {
      fflush (stream->fp);
      if (scope == FLUSH_ALL)
        {
          int fd = fileno (stream->fp);
          if (fd >= 0)
            {
              /* For streams connected to a disk file:  */
              fsync (fd);
              #if HAVE_TCDRAIN
              /* For streams connected to a terminal:  */
              nonintr_tcdrain (fd);
              #endif
            }
        }
    }
}

static void
file_ostream::free (file_ostream_t stream)
{
  free (stream);
}

/* Constructor.  */

file_ostream_t
file_ostream_create (FILE *fp)
{
  file_ostream_t stream = XMALLOC (struct file_ostream_representation);

  stream->base.vtable = &file_ostream_vtable;
  stream->fp = fp;

  return stream;
}

/* Accessors.  */

static FILE *
file_ostream::get_stdio_stream (file_ostream_t stream)
{
  return stream->fp;
}

/* Instanceof test.  */

bool
is_instance_of_file_ostream (ostream_t stream)
{
  return IS_INSTANCE (stream, ostream, file_ostream);
}
