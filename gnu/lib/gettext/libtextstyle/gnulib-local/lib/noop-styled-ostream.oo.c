/* Output stream with no-op styling.
   Copyright (C) 2006, 2019-2020 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2019.

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
#include "noop-styled-ostream.h"

#include "xalloc.h"


struct noop_styled_ostream : struct styled_ostream
{
fields:
  /* The destination stream.  */
  ostream_t destination;
  bool own_destination;
  /* The current hyperlink ref and id.  */
  char *hyperlink_ref;
  char *hyperlink_id;
};

/* Implementation of ostream_t methods.  */

static void
noop_styled_ostream::write_mem (noop_styled_ostream_t stream,
                                const void *data, size_t len)
{
  ostream_write_mem (stream->destination, data, len);
}

static void
noop_styled_ostream::flush (noop_styled_ostream_t stream,
                            ostream_flush_scope_t scope)
{
  ostream_flush (stream->destination, scope);
}

static void
noop_styled_ostream::free (noop_styled_ostream_t stream)
{
  if (stream->own_destination)
    ostream_free (stream->destination);
  free (stream->hyperlink_ref);
  free (stream->hyperlink_id);
  free (stream);
}

/* Implementation of styled_ostream_t methods.  */

static void
noop_styled_ostream::begin_use_class (noop_styled_ostream_t stream,
                                      const char *classname)
{
}

static void
noop_styled_ostream::end_use_class (noop_styled_ostream_t stream,
                                    const char *classname)
{
}

static const char *
noop_styled_ostream::get_hyperlink_ref (noop_styled_ostream_t stream)
{
  return stream->hyperlink_ref;
}

static const char *
noop_styled_ostream::get_hyperlink_id (noop_styled_ostream_t stream)
{
  return stream->hyperlink_id;
}

static void
noop_styled_ostream::set_hyperlink (noop_styled_ostream_t stream,
                                    const char *ref, const char *id)
{
  char *ref_copy = (ref != NULL ? xstrdup (ref) : NULL);
  char *id_copy = (id != NULL ? xstrdup (id) : NULL);

  free (stream->hyperlink_ref);
  stream->hyperlink_ref = ref_copy;
  free (stream->hyperlink_id);
  stream->hyperlink_id = id_copy;
}

static void
noop_styled_ostream::flush_to_current_style (noop_styled_ostream_t stream)
{
  ostream_flush (stream->destination, FLUSH_THIS_STREAM);
}

/* Constructor.  */

noop_styled_ostream_t
noop_styled_ostream_create (ostream_t destination, bool pass_ownership)
{
  noop_styled_ostream_t stream =
    XMALLOC (struct noop_styled_ostream_representation);

  stream->base.base.vtable = &noop_styled_ostream_vtable;
  stream->destination = destination;
  stream->own_destination = pass_ownership;
  stream->hyperlink_ref = NULL;
  stream->hyperlink_id = NULL;

  return stream;
}

/* Accessors.  */

static ostream_t
noop_styled_ostream::get_destination (noop_styled_ostream_t stream)
{
  return stream->destination;
}

static bool
noop_styled_ostream::is_owning_destination (noop_styled_ostream_t stream)
{
  return stream->own_destination;
}

/* Instanceof test.  */

bool
is_instance_of_noop_styled_ostream (ostream_t stream)
{
  return IS_INSTANCE (stream, ostream, noop_styled_ostream);
}
