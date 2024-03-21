/* DO NOT EDIT! GENERATED AUTOMATICALLY! */

#line 1 "memory-ostream.oo.h"
/* Output stream that accumulates the output in memory.
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

#ifndef _MEMORY_OSTREAM_H
#define _MEMORY_OSTREAM_H

#include <stdbool.h>
#include <stddef.h>

#include "ostream.h"

#line 30 "memory-ostream.h"
struct memory_ostream_representation;
/* memory_ostream_t is defined as a pointer to struct memory_ostream_representation.
   In C++ mode, we use a smart pointer class.
   In C mode, we have no other choice than a typedef to the root class type.  */
#if IS_CPLUSPLUS
struct memory_ostream_t
{
private:
  struct memory_ostream_representation *_pointer;
public:
  memory_ostream_t () : _pointer (NULL) {}
  memory_ostream_t (struct memory_ostream_representation *pointer) : _pointer (pointer) {}
  struct memory_ostream_representation * operator -> () { return _pointer; }
  operator struct memory_ostream_representation * () { return _pointer; }
  operator struct any_ostream_representation * () { return (struct any_ostream_representation *) _pointer; }
  operator void * () { return _pointer; }
  bool operator == (const void *p) { return _pointer == p; }
  bool operator != (const void *p) { return _pointer != p; }
  operator ostream_t () { return (ostream_t) (struct any_ostream_representation *) _pointer; }
  explicit memory_ostream_t (ostream_t x) : _pointer ((struct memory_ostream_representation *) (void *) x) {}
};
#else
typedef ostream_t memory_ostream_t;
#endif

/* Functions that invoke the methods.  */
#ifdef __cplusplus
extern "C" {
#endif
extern        void memory_ostream_write_mem (memory_ostream_t first_arg, const void *data, size_t len);
extern         void memory_ostream_flush (memory_ostream_t first_arg, ostream_flush_scope_t scope);
extern         void memory_ostream_free (memory_ostream_t first_arg);
extern          void memory_ostream_contents (memory_ostream_t first_arg, const void **bufp, size_t *buflenp);
#ifdef __cplusplus
}
#endif

/* Type representing an implementation of memory_ostream_t.  */
struct memory_ostream_implementation
{
  const typeinfo_t * const *superclasses;
  size_t superclasses_length;
  size_t instance_size;
#define THIS_ARG memory_ostream_t first_arg
#include "memory_ostream.vt.h"
#undef THIS_ARG
};

/* Public portion of the object pointed to by a memory_ostream_t.  */
struct memory_ostream_representation_header
{
  const struct memory_ostream_implementation *vtable;
};

#if HAVE_INLINE

/* Define the functions that invoke the methods as inline accesses to
   the memory_ostream_implementation.
   Use #define to avoid a warning because of extern vs. static.  */

# define memory_ostream_write_mem memory_ostream_write_mem_inline
static inline void
memory_ostream_write_mem (memory_ostream_t first_arg, const void *data, size_t len)
{
  const struct memory_ostream_implementation *vtable =
    ((struct memory_ostream_representation_header *) (struct memory_ostream_representation *) first_arg)->vtable;
  vtable->write_mem (first_arg,data,len);
}

# define memory_ostream_flush memory_ostream_flush_inline
static inline void
memory_ostream_flush (memory_ostream_t first_arg, ostream_flush_scope_t scope)
{
  const struct memory_ostream_implementation *vtable =
    ((struct memory_ostream_representation_header *) (struct memory_ostream_representation *) first_arg)->vtable;
  vtable->flush (first_arg,scope);
}

# define memory_ostream_free memory_ostream_free_inline
static inline void
memory_ostream_free (memory_ostream_t first_arg)
{
  const struct memory_ostream_implementation *vtable =
    ((struct memory_ostream_representation_header *) (struct memory_ostream_representation *) first_arg)->vtable;
  vtable->free (first_arg);
}

# define memory_ostream_contents memory_ostream_contents_inline
static inline void
memory_ostream_contents (memory_ostream_t first_arg, const void **bufp, size_t *buflenp)
{
  const struct memory_ostream_implementation *vtable =
    ((struct memory_ostream_representation_header *) (struct memory_ostream_representation *) first_arg)->vtable;
  vtable->contents (first_arg,bufp,buflenp);
}

#endif

extern const typeinfo_t memory_ostream_typeinfo;
#define memory_ostream_SUPERCLASSES &memory_ostream_typeinfo, ostream_SUPERCLASSES
#define memory_ostream_SUPERCLASSES_LENGTH (1 + ostream_SUPERCLASSES_LENGTH)

extern const struct memory_ostream_implementation memory_ostream_vtable;

#line 35 "memory-ostream.oo.h"

#ifdef __cplusplus
extern "C" {
#endif


/* Create an output stream that accumulates the output in a memory buffer.  */
extern memory_ostream_t memory_ostream_create (void);


/* Test whether a given output stream is a memory_ostream.  */
extern bool is_instance_of_memory_ostream (ostream_t stream);


#ifdef __cplusplus
}
#endif

#endif /* _MEMORY_OSTREAM_H */
