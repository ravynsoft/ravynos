/* DO NOT EDIT! GENERATED AUTOMATICALLY! */

#line 1 "file-ostream.oo.h"
/* Output stream referring to an stdio FILE.
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

#ifndef _FILE_OSTREAM_H
#define _FILE_OSTREAM_H

#include <stdbool.h>
#include <stdio.h>

#include "ostream.h"


#line 31 "file-ostream.h"
struct file_ostream_representation;
/* file_ostream_t is defined as a pointer to struct file_ostream_representation.
   In C++ mode, we use a smart pointer class.
   In C mode, we have no other choice than a typedef to the root class type.  */
#if IS_CPLUSPLUS
struct file_ostream_t
{
private:
  struct file_ostream_representation *_pointer;
public:
  file_ostream_t () : _pointer (NULL) {}
  file_ostream_t (struct file_ostream_representation *pointer) : _pointer (pointer) {}
  struct file_ostream_representation * operator -> () { return _pointer; }
  operator struct file_ostream_representation * () { return _pointer; }
  operator struct any_ostream_representation * () { return (struct any_ostream_representation *) _pointer; }
  operator void * () { return _pointer; }
  bool operator == (const void *p) { return _pointer == p; }
  bool operator != (const void *p) { return _pointer != p; }
  operator ostream_t () { return (ostream_t) (struct any_ostream_representation *) _pointer; }
  explicit file_ostream_t (ostream_t x) : _pointer ((struct file_ostream_representation *) (void *) x) {}
};
#else
typedef ostream_t file_ostream_t;
#endif

/* Functions that invoke the methods.  */
#ifdef __cplusplus
extern "C" {
#endif
extern        void file_ostream_write_mem (file_ostream_t first_arg, const void *data, size_t len);
extern         void file_ostream_flush (file_ostream_t first_arg, ostream_flush_scope_t scope);
extern         void file_ostream_free (file_ostream_t first_arg);
extern       FILE * file_ostream_get_stdio_stream (file_ostream_t first_arg);
#ifdef __cplusplus
}
#endif

/* Type representing an implementation of file_ostream_t.  */
struct file_ostream_implementation
{
  const typeinfo_t * const *superclasses;
  size_t superclasses_length;
  size_t instance_size;
#define THIS_ARG file_ostream_t first_arg
#include "file_ostream.vt.h"
#undef THIS_ARG
};

/* Public portion of the object pointed to by a file_ostream_t.  */
struct file_ostream_representation_header
{
  const struct file_ostream_implementation *vtable;
};

#if HAVE_INLINE

/* Define the functions that invoke the methods as inline accesses to
   the file_ostream_implementation.
   Use #define to avoid a warning because of extern vs. static.  */

# define file_ostream_write_mem file_ostream_write_mem_inline
static inline void
file_ostream_write_mem (file_ostream_t first_arg, const void *data, size_t len)
{
  const struct file_ostream_implementation *vtable =
    ((struct file_ostream_representation_header *) (struct file_ostream_representation *) first_arg)->vtable;
  vtable->write_mem (first_arg,data,len);
}

# define file_ostream_flush file_ostream_flush_inline
static inline void
file_ostream_flush (file_ostream_t first_arg, ostream_flush_scope_t scope)
{
  const struct file_ostream_implementation *vtable =
    ((struct file_ostream_representation_header *) (struct file_ostream_representation *) first_arg)->vtable;
  vtable->flush (first_arg,scope);
}

# define file_ostream_free file_ostream_free_inline
static inline void
file_ostream_free (file_ostream_t first_arg)
{
  const struct file_ostream_implementation *vtable =
    ((struct file_ostream_representation_header *) (struct file_ostream_representation *) first_arg)->vtable;
  vtable->free (first_arg);
}

# define file_ostream_get_stdio_stream file_ostream_get_stdio_stream_inline
static inline FILE *
file_ostream_get_stdio_stream (file_ostream_t first_arg)
{
  const struct file_ostream_implementation *vtable =
    ((struct file_ostream_representation_header *) (struct file_ostream_representation *) first_arg)->vtable;
  return vtable->get_stdio_stream (first_arg);
}

#endif

extern const typeinfo_t file_ostream_typeinfo;
#define file_ostream_SUPERCLASSES &file_ostream_typeinfo, ostream_SUPERCLASSES
#define file_ostream_SUPERCLASSES_LENGTH (1 + ostream_SUPERCLASSES_LENGTH)

extern const struct file_ostream_implementation file_ostream_vtable;

#line 33 "file-ostream.oo.h"


#ifdef __cplusplus
extern "C" {
#endif


/* Create an output stream referring to FP.
   Note that the resulting stream must be closed before FP can be closed.  */
extern file_ostream_t file_ostream_create (FILE *fp);


/* Test whether a given output stream is a file_ostream.  */
extern bool is_instance_of_file_ostream (ostream_t stream);


#ifdef __cplusplus
}
#endif

#endif /* _FILE_OSTREAM_H */
