/* DO NOT EDIT! GENERATED AUTOMATICALLY! */

#if !IS_CPLUSPLUS
#define ostream_representation any_ostream_representation
#endif
#line 1 "ostream.oo.c"
/* Abstract output stream data type.
   Copyright (C) 2006, 2019 Free Software Foundation, Inc.
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
#include "ostream.h"

#include <stdio.h>

#line 32 "ostream.c"
#include "ostream.priv.h"

const typeinfo_t ostream_typeinfo = { "ostream" };

static const typeinfo_t * const ostream_superclasses[] =
  { ostream_SUPERCLASSES };

#line 29 "ostream.oo.c"

#if !HAVE_INLINE

void
ostream_write_str (ostream_t stream, const char *string)
{
  ostream_write_mem (stream, string, strlen (string));
}

#endif

ptrdiff_t
ostream_printf (ostream_t stream, const char *format, ...)
{
  va_list args;
  char *temp_string;
  ptrdiff_t ret;

  va_start (args, format);
  ret = vasprintf (&temp_string, format, args);
  va_end (args);
  if (ret >= 0)
    {
      if (ret > 0)
        ostream_write_str (stream, temp_string);
      free (temp_string);
    }
  return ret;
}

ptrdiff_t
ostream_vprintf (ostream_t stream, const char *format, va_list args)
{
  char *temp_string;
  ptrdiff_t ret = vasprintf (&temp_string, format, args);
  if (ret >= 0)
    {
      if (ret > 0)
        ostream_write_str (stream, temp_string);
      free (temp_string);
    }
  return ret;
}

#line 85 "ostream.c"
void ostream__write_mem (ostream_t first_arg, const void *data, size_t len);
void
ostream__write_mem (ostream_t first_arg, const void *data, size_t len)
{
  /* Abstract (unimplemented) method called.  */
  abort ();
  #ifndef __GNUC__
  ostream__write_mem (first_arg,data,len);
  #endif
}

void ostream__flush (ostream_t first_arg, ostream_flush_scope_t scope);
void
ostream__flush (ostream_t first_arg, ostream_flush_scope_t scope)
{
  /* Abstract (unimplemented) method called.  */
  abort ();
  #ifndef __GNUC__
  ostream__flush (first_arg,scope);
  #endif
}

void ostream__free (ostream_t first_arg);
void
ostream__free (ostream_t first_arg)
{
  /* Abstract (unimplemented) method called.  */
  abort ();
  #ifndef __GNUC__
  ostream__free (first_arg);
  #endif
}


const struct ostream_implementation ostream_vtable =
{
  ostream_superclasses,
  sizeof (ostream_superclasses) / sizeof (ostream_superclasses[0]),
  sizeof (struct ostream_representation),
  ostream__write_mem,
  ostream__flush,
  ostream__free,
};

#if !HAVE_INLINE

/* Define the functions that invoke the methods.  */

void
ostream_write_mem (ostream_t first_arg, const void *data, size_t len)
{
  const struct ostream_implementation *vtable =
    ((struct ostream_representation_header *) (struct any_ostream_representation *) first_arg)->vtable;
  vtable->write_mem (first_arg,data,len);
}

void
ostream_flush (ostream_t first_arg, ostream_flush_scope_t scope)
{
  const struct ostream_implementation *vtable =
    ((struct ostream_representation_header *) (struct any_ostream_representation *) first_arg)->vtable;
  vtable->flush (first_arg,scope);
}

void
ostream_free (ostream_t first_arg)
{
  const struct ostream_implementation *vtable =
    ((struct ostream_representation_header *) (struct any_ostream_representation *) first_arg)->vtable;
  vtable->free (first_arg);
}

#endif
