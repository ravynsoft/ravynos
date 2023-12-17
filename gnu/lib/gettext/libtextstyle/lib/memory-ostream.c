/* DO NOT EDIT! GENERATED AUTOMATICALLY! */

#if !IS_CPLUSPLUS
#define memory_ostream_representation any_ostream_representation
#endif
#line 1 "memory-ostream.oo.c"
/* Output stream that accumulates the output in memory.
   Copyright (C) 2006-2007, 2019-2020, 2023 Free Software Foundation, Inc.
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
#include "memory-ostream.h"

#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "xalloc.h"
#include "xsize.h"
#include "gettext.h"

#define _(str) gettext (str)

#line 40 "memory-ostream.c"
#include "memory_ostream.priv.h"

const typeinfo_t memory_ostream_typeinfo = { "memory_ostream" };

static const typeinfo_t * const memory_ostream_superclasses[] =
  { memory_ostream_SUPERCLASSES };

#define super ostream_vtable

#line 40 "memory-ostream.oo.c"

/* Implementation of ostream_t methods.  */

static void
memory_ostream__write_mem (memory_ostream_t stream,
                           const void *data, size_t len)
{
  if (len > 0)
    {
      if (len > stream->allocated - stream->buflen)
        {
          size_t new_allocated =
            xmax (xsum (stream->buflen, len),
                  xsum (stream->allocated, stream->allocated));
          if (size_overflow_p (new_allocated))
            error (EXIT_FAILURE, 0,
                   _("%s: too much output, buffer size overflow"),
                   "memory_ostream");
          stream->buffer = (char *) xrealloc (stream->buffer, new_allocated);
          stream->allocated = new_allocated;
        }
      memcpy (stream->buffer + stream->buflen, data, len);
      stream->buflen += len;
    }
}

static void
memory_ostream__flush (memory_ostream_t stream, ostream_flush_scope_t scope)
{
}

static void
memory_ostream__free (memory_ostream_t stream)
{
  free (stream->buffer);
  free (stream);
}

/* Implementation of memory_ostream_t methods.  */

static void
memory_ostream__contents (memory_ostream_t stream,
                          const void **bufp, size_t *buflenp)
{
  *bufp = stream->buffer;
  *buflenp = stream->buflen;
}

/* Constructor.  */

memory_ostream_t
memory_ostream_create (void)
{
  memory_ostream_t stream = XMALLOC (struct memory_ostream_representation);

  stream->base.vtable = &memory_ostream_vtable;
  stream->allocated = 250;
  stream->buffer = XNMALLOC (stream->allocated, char);
  stream->buflen = 0;

  return stream;
}

/* Instanceof test.  */

bool
is_instance_of_memory_ostream (ostream_t stream)
{
  return IS_INSTANCE (stream, ostream, memory_ostream);
}

#line 122 "memory-ostream.c"

const struct memory_ostream_implementation memory_ostream_vtable =
{
  memory_ostream_superclasses,
  sizeof (memory_ostream_superclasses) / sizeof (memory_ostream_superclasses[0]),
  sizeof (struct memory_ostream_representation),
  memory_ostream__write_mem,
  memory_ostream__flush,
  memory_ostream__free,
  memory_ostream__contents,
};

#if !HAVE_INLINE

/* Define the functions that invoke the methods.  */

void
memory_ostream_write_mem (memory_ostream_t first_arg, const void *data, size_t len)
{
  const struct memory_ostream_implementation *vtable =
    ((struct memory_ostream_representation_header *) (struct memory_ostream_representation *) first_arg)->vtable;
  vtable->write_mem (first_arg,data,len);
}

void
memory_ostream_flush (memory_ostream_t first_arg, ostream_flush_scope_t scope)
{
  const struct memory_ostream_implementation *vtable =
    ((struct memory_ostream_representation_header *) (struct memory_ostream_representation *) first_arg)->vtable;
  vtable->flush (first_arg,scope);
}

void
memory_ostream_free (memory_ostream_t first_arg)
{
  const struct memory_ostream_implementation *vtable =
    ((struct memory_ostream_representation_header *) (struct memory_ostream_representation *) first_arg)->vtable;
  vtable->free (first_arg);
}

void
memory_ostream_contents (memory_ostream_t first_arg, const void **bufp, size_t *buflenp)
{
  const struct memory_ostream_implementation *vtable =
    ((struct memory_ostream_representation_header *) (struct memory_ostream_representation *) first_arg)->vtable;
  vtable->contents (first_arg,bufp,buflenp);
}

#endif
