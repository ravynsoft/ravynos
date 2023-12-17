/* DO NOT EDIT! GENERATED AUTOMATICALLY! */

#if !IS_CPLUSPLUS
#define noop_styled_ostream_representation any_ostream_representation
#endif
#line 1 "noop-styled-ostream.oo.c"
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


#line 33 "noop-styled-ostream.c"
#include "noop_styled_ostream.priv.h"

const typeinfo_t noop_styled_ostream_typeinfo = { "noop_styled_ostream" };

static const typeinfo_t * const noop_styled_ostream_superclasses[] =
  { noop_styled_ostream_SUPERCLASSES };

#define super styled_ostream_vtable

#line 36 "noop-styled-ostream.oo.c"

/* Implementation of ostream_t methods.  */

static void
noop_styled_ostream__write_mem (noop_styled_ostream_t stream,
                                const void *data, size_t len)
{
  ostream_write_mem (stream->destination, data, len);
}

static void
noop_styled_ostream__flush (noop_styled_ostream_t stream,
                            ostream_flush_scope_t scope)
{
  ostream_flush (stream->destination, scope);
}

static void
noop_styled_ostream__free (noop_styled_ostream_t stream)
{
  if (stream->own_destination)
    ostream_free (stream->destination);
  free (stream->hyperlink_ref);
  free (stream->hyperlink_id);
  free (stream);
}

/* Implementation of styled_ostream_t methods.  */

static void
noop_styled_ostream__begin_use_class (noop_styled_ostream_t stream,
                                      const char *classname)
{
}

static void
noop_styled_ostream__end_use_class (noop_styled_ostream_t stream,
                                    const char *classname)
{
}

static const char *
noop_styled_ostream__get_hyperlink_ref (noop_styled_ostream_t stream)
{
  return stream->hyperlink_ref;
}

static const char *
noop_styled_ostream__get_hyperlink_id (noop_styled_ostream_t stream)
{
  return stream->hyperlink_id;
}

static void
noop_styled_ostream__set_hyperlink (noop_styled_ostream_t stream,
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
noop_styled_ostream__flush_to_current_style (noop_styled_ostream_t stream)
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
noop_styled_ostream__get_destination (noop_styled_ostream_t stream)
{
  return stream->destination;
}

static bool
noop_styled_ostream__is_owning_destination (noop_styled_ostream_t stream)
{
  return stream->own_destination;
}

/* Instanceof test.  */

bool
is_instance_of_noop_styled_ostream (ostream_t stream)
{
  return IS_INSTANCE (stream, ostream, noop_styled_ostream);
}

#line 155 "noop-styled-ostream.c"

const struct noop_styled_ostream_implementation noop_styled_ostream_vtable =
{
  noop_styled_ostream_superclasses,
  sizeof (noop_styled_ostream_superclasses) / sizeof (noop_styled_ostream_superclasses[0]),
  sizeof (struct noop_styled_ostream_representation),
  noop_styled_ostream__write_mem,
  noop_styled_ostream__flush,
  noop_styled_ostream__free,
  noop_styled_ostream__begin_use_class,
  noop_styled_ostream__end_use_class,
  noop_styled_ostream__get_hyperlink_ref,
  noop_styled_ostream__get_hyperlink_id,
  noop_styled_ostream__set_hyperlink,
  noop_styled_ostream__flush_to_current_style,
  noop_styled_ostream__get_destination,
  noop_styled_ostream__is_owning_destination,
};

#if !HAVE_INLINE

/* Define the functions that invoke the methods.  */

void
noop_styled_ostream_write_mem (noop_styled_ostream_t first_arg, const void *data, size_t len)
{
  const struct noop_styled_ostream_implementation *vtable =
    ((struct noop_styled_ostream_representation_header *) (struct noop_styled_ostream_representation *) first_arg)->vtable;
  vtable->write_mem (first_arg,data,len);
}

void
noop_styled_ostream_flush (noop_styled_ostream_t first_arg, ostream_flush_scope_t scope)
{
  const struct noop_styled_ostream_implementation *vtable =
    ((struct noop_styled_ostream_representation_header *) (struct noop_styled_ostream_representation *) first_arg)->vtable;
  vtable->flush (first_arg,scope);
}

void
noop_styled_ostream_free (noop_styled_ostream_t first_arg)
{
  const struct noop_styled_ostream_implementation *vtable =
    ((struct noop_styled_ostream_representation_header *) (struct noop_styled_ostream_representation *) first_arg)->vtable;
  vtable->free (first_arg);
}

void
noop_styled_ostream_begin_use_class (noop_styled_ostream_t first_arg, const char *classname)
{
  const struct noop_styled_ostream_implementation *vtable =
    ((struct noop_styled_ostream_representation_header *) (struct noop_styled_ostream_representation *) first_arg)->vtable;
  vtable->begin_use_class (first_arg,classname);
}

void
noop_styled_ostream_end_use_class (noop_styled_ostream_t first_arg, const char *classname)
{
  const struct noop_styled_ostream_implementation *vtable =
    ((struct noop_styled_ostream_representation_header *) (struct noop_styled_ostream_representation *) first_arg)->vtable;
  vtable->end_use_class (first_arg,classname);
}

const char *
noop_styled_ostream_get_hyperlink_ref (noop_styled_ostream_t first_arg)
{
  const struct noop_styled_ostream_implementation *vtable =
    ((struct noop_styled_ostream_representation_header *) (struct noop_styled_ostream_representation *) first_arg)->vtable;
  return vtable->get_hyperlink_ref (first_arg);
}

const char *
noop_styled_ostream_get_hyperlink_id (noop_styled_ostream_t first_arg)
{
  const struct noop_styled_ostream_implementation *vtable =
    ((struct noop_styled_ostream_representation_header *) (struct noop_styled_ostream_representation *) first_arg)->vtable;
  return vtable->get_hyperlink_id (first_arg);
}

void
noop_styled_ostream_set_hyperlink (noop_styled_ostream_t first_arg,                               const char *ref, const char *id)
{
  const struct noop_styled_ostream_implementation *vtable =
    ((struct noop_styled_ostream_representation_header *) (struct noop_styled_ostream_representation *) first_arg)->vtable;
  vtable->set_hyperlink (first_arg,ref,id);
}

void
noop_styled_ostream_flush_to_current_style (noop_styled_ostream_t first_arg)
{
  const struct noop_styled_ostream_implementation *vtable =
    ((struct noop_styled_ostream_representation_header *) (struct noop_styled_ostream_representation *) first_arg)->vtable;
  vtable->flush_to_current_style (first_arg);
}

ostream_t
noop_styled_ostream_get_destination (noop_styled_ostream_t first_arg)
{
  const struct noop_styled_ostream_implementation *vtable =
    ((struct noop_styled_ostream_representation_header *) (struct noop_styled_ostream_representation *) first_arg)->vtable;
  return vtable->get_destination (first_arg);
}

bool
noop_styled_ostream_is_owning_destination (noop_styled_ostream_t first_arg)
{
  const struct noop_styled_ostream_implementation *vtable =
    ((struct noop_styled_ostream_representation_header *) (struct noop_styled_ostream_representation *) first_arg)->vtable;
  return vtable->is_owning_destination (first_arg);
}

#endif
