/* DO NOT EDIT! GENERATED AUTOMATICALLY! */

#line 1 "iconv-ostream.oo.h"
/* Output stream that converts the output to another encoding.
   Copyright (C) 2006, 2020 Free Software Foundation, Inc.
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

#ifndef _ICONV_OSTREAM_H
#define _ICONV_OSTREAM_H

/* Note that this stream does not provide accurate error messages with line
   and column number when the conversion fails.  */

#include <stdbool.h>

#include "ostream.h"


#line 33 "iconv-ostream.h"
struct iconv_ostream_representation;
/* iconv_ostream_t is defined as a pointer to struct iconv_ostream_representation.
   In C++ mode, we use a smart pointer class.
   In C mode, we have no other choice than a typedef to the root class type.  */
#if IS_CPLUSPLUS
struct iconv_ostream_t
{
private:
  struct iconv_ostream_representation *_pointer;
public:
  iconv_ostream_t () : _pointer (NULL) {}
  iconv_ostream_t (struct iconv_ostream_representation *pointer) : _pointer (pointer) {}
  struct iconv_ostream_representation * operator -> () { return _pointer; }
  operator struct iconv_ostream_representation * () { return _pointer; }
  operator struct any_ostream_representation * () { return (struct any_ostream_representation *) _pointer; }
  operator void * () { return _pointer; }
  bool operator == (const void *p) { return _pointer == p; }
  bool operator != (const void *p) { return _pointer != p; }
  operator ostream_t () { return (ostream_t) (struct any_ostream_representation *) _pointer; }
  explicit iconv_ostream_t (ostream_t x) : _pointer ((struct iconv_ostream_representation *) (void *) x) {}
};
#else
typedef ostream_t iconv_ostream_t;
#endif

/* Functions that invoke the methods.  */
#ifdef __cplusplus
extern "C" {
#endif
extern        void iconv_ostream_write_mem (iconv_ostream_t first_arg, const void *data, size_t len);
extern         void iconv_ostream_flush (iconv_ostream_t first_arg, ostream_flush_scope_t scope);
extern         void iconv_ostream_free (iconv_ostream_t first_arg);
extern       const char * iconv_ostream_get_from_encoding (iconv_ostream_t first_arg);
extern    const char * iconv_ostream_get_to_encoding (iconv_ostream_t first_arg);
extern    ostream_t    iconv_ostream_get_destination (iconv_ostream_t first_arg);
#ifdef __cplusplus
}
#endif

/* Type representing an implementation of iconv_ostream_t.  */
struct iconv_ostream_implementation
{
  const typeinfo_t * const *superclasses;
  size_t superclasses_length;
  size_t instance_size;
#define THIS_ARG iconv_ostream_t first_arg
#include "iconv_ostream.vt.h"
#undef THIS_ARG
};

/* Public portion of the object pointed to by a iconv_ostream_t.  */
struct iconv_ostream_representation_header
{
  const struct iconv_ostream_implementation *vtable;
};

#if HAVE_INLINE

/* Define the functions that invoke the methods as inline accesses to
   the iconv_ostream_implementation.
   Use #define to avoid a warning because of extern vs. static.  */

# define iconv_ostream_write_mem iconv_ostream_write_mem_inline
static inline void
iconv_ostream_write_mem (iconv_ostream_t first_arg, const void *data, size_t len)
{
  const struct iconv_ostream_implementation *vtable =
    ((struct iconv_ostream_representation_header *) (struct iconv_ostream_representation *) first_arg)->vtable;
  vtable->write_mem (first_arg,data,len);
}

# define iconv_ostream_flush iconv_ostream_flush_inline
static inline void
iconv_ostream_flush (iconv_ostream_t first_arg, ostream_flush_scope_t scope)
{
  const struct iconv_ostream_implementation *vtable =
    ((struct iconv_ostream_representation_header *) (struct iconv_ostream_representation *) first_arg)->vtable;
  vtable->flush (first_arg,scope);
}

# define iconv_ostream_free iconv_ostream_free_inline
static inline void
iconv_ostream_free (iconv_ostream_t first_arg)
{
  const struct iconv_ostream_implementation *vtable =
    ((struct iconv_ostream_representation_header *) (struct iconv_ostream_representation *) first_arg)->vtable;
  vtable->free (first_arg);
}

# define iconv_ostream_get_from_encoding iconv_ostream_get_from_encoding_inline
static inline const char *
iconv_ostream_get_from_encoding (iconv_ostream_t first_arg)
{
  const struct iconv_ostream_implementation *vtable =
    ((struct iconv_ostream_representation_header *) (struct iconv_ostream_representation *) first_arg)->vtable;
  return vtable->get_from_encoding (first_arg);
}

# define iconv_ostream_get_to_encoding iconv_ostream_get_to_encoding_inline
static inline const char *
iconv_ostream_get_to_encoding (iconv_ostream_t first_arg)
{
  const struct iconv_ostream_implementation *vtable =
    ((struct iconv_ostream_representation_header *) (struct iconv_ostream_representation *) first_arg)->vtable;
  return vtable->get_to_encoding (first_arg);
}

# define iconv_ostream_get_destination iconv_ostream_get_destination_inline
static inline ostream_t
iconv_ostream_get_destination (iconv_ostream_t first_arg)
{
  const struct iconv_ostream_implementation *vtable =
    ((struct iconv_ostream_representation_header *) (struct iconv_ostream_representation *) first_arg)->vtable;
  return vtable->get_destination (first_arg);
}

#endif

extern const typeinfo_t iconv_ostream_typeinfo;
#define iconv_ostream_SUPERCLASSES &iconv_ostream_typeinfo, ostream_SUPERCLASSES
#define iconv_ostream_SUPERCLASSES_LENGTH (1 + ostream_SUPERCLASSES_LENGTH)

extern const struct iconv_ostream_implementation iconv_ostream_vtable;

#line 37 "iconv-ostream.oo.h"


#ifdef __cplusplus
extern "C" {
#endif


#if HAVE_ICONV

/* Create an output stream that converts from FROM_ENCODING to TO_ENCODING,
   writing the result to DESTINATION.  */
extern iconv_ostream_t iconv_ostream_create (const char *from_encoding,
                                             const char *to_encoding,
                                             ostream_t destination);

#endif /* HAVE_ICONV */


/* Test whether a given output stream is an iconv_ostream.  */
extern bool is_instance_of_iconv_ostream (ostream_t stream);


#ifdef __cplusplus
}
#endif

#endif /* _ICONV_OSTREAM_H */
