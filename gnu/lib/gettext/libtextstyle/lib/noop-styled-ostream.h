/* DO NOT EDIT! GENERATED AUTOMATICALLY! */

#line 1 "noop-styled-ostream.oo.h"
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

#ifndef _NOOP_STYLED_OSTREAM_H
#define _NOOP_STYLED_OSTREAM_H

#include <stdbool.h>

#include "styled-ostream.h"


#line 30 "noop-styled-ostream.h"
struct noop_styled_ostream_representation;
/* noop_styled_ostream_t is defined as a pointer to struct noop_styled_ostream_representation.
   In C++ mode, we use a smart pointer class.
   In C mode, we have no other choice than a typedef to the root class type.  */
#if IS_CPLUSPLUS
struct noop_styled_ostream_t
{
private:
  struct noop_styled_ostream_representation *_pointer;
public:
  noop_styled_ostream_t () : _pointer (NULL) {}
  noop_styled_ostream_t (struct noop_styled_ostream_representation *pointer) : _pointer (pointer) {}
  struct noop_styled_ostream_representation * operator -> () { return _pointer; }
  operator struct noop_styled_ostream_representation * () { return _pointer; }
  operator struct any_ostream_representation * () { return (struct any_ostream_representation *) _pointer; }
  operator struct styled_ostream_representation * () { return (struct styled_ostream_representation *) _pointer; }
  operator void * () { return _pointer; }
  bool operator == (const void *p) { return _pointer == p; }
  bool operator != (const void *p) { return _pointer != p; }
  operator ostream_t () { return (ostream_t) (struct any_ostream_representation *) _pointer; }
  explicit noop_styled_ostream_t (ostream_t x) : _pointer ((struct noop_styled_ostream_representation *) (void *) x) {}
  operator styled_ostream_t () { return (styled_ostream_t) (struct styled_ostream_representation *) _pointer; }
  explicit noop_styled_ostream_t (styled_ostream_t x) : _pointer ((struct noop_styled_ostream_representation *) (void *) x) {}
};
#else
typedef styled_ostream_t noop_styled_ostream_t;
#endif

/* Functions that invoke the methods.  */
#ifdef __cplusplus
extern "C" {
#endif
extern        void noop_styled_ostream_write_mem (noop_styled_ostream_t first_arg, const void *data, size_t len);
extern         void noop_styled_ostream_flush (noop_styled_ostream_t first_arg, ostream_flush_scope_t scope);
extern         void noop_styled_ostream_free (noop_styled_ostream_t first_arg);
extern          void noop_styled_ostream_begin_use_class (noop_styled_ostream_t first_arg, const char *classname);
extern          void noop_styled_ostream_end_use_class (noop_styled_ostream_t first_arg, const char *classname);
extern         const char * noop_styled_ostream_get_hyperlink_ref (noop_styled_ostream_t first_arg);
extern    const char * noop_styled_ostream_get_hyperlink_id (noop_styled_ostream_t first_arg);
extern    void         noop_styled_ostream_set_hyperlink (noop_styled_ostream_t first_arg,                               const char *ref, const char *id);
extern              void noop_styled_ostream_flush_to_current_style (noop_styled_ostream_t first_arg);
extern       ostream_t noop_styled_ostream_get_destination (noop_styled_ostream_t first_arg);
extern    bool      noop_styled_ostream_is_owning_destination (noop_styled_ostream_t first_arg);
#ifdef __cplusplus
}
#endif

/* Type representing an implementation of noop_styled_ostream_t.  */
struct noop_styled_ostream_implementation
{
  const typeinfo_t * const *superclasses;
  size_t superclasses_length;
  size_t instance_size;
#define THIS_ARG noop_styled_ostream_t first_arg
#include "noop_styled_ostream.vt.h"
#undef THIS_ARG
};

/* Public portion of the object pointed to by a noop_styled_ostream_t.  */
struct noop_styled_ostream_representation_header
{
  const struct noop_styled_ostream_implementation *vtable;
};

#if HAVE_INLINE

/* Define the functions that invoke the methods as inline accesses to
   the noop_styled_ostream_implementation.
   Use #define to avoid a warning because of extern vs. static.  */

# define noop_styled_ostream_write_mem noop_styled_ostream_write_mem_inline
static inline void
noop_styled_ostream_write_mem (noop_styled_ostream_t first_arg, const void *data, size_t len)
{
  const struct noop_styled_ostream_implementation *vtable =
    ((struct noop_styled_ostream_representation_header *) (struct noop_styled_ostream_representation *) first_arg)->vtable;
  vtable->write_mem (first_arg,data,len);
}

# define noop_styled_ostream_flush noop_styled_ostream_flush_inline
static inline void
noop_styled_ostream_flush (noop_styled_ostream_t first_arg, ostream_flush_scope_t scope)
{
  const struct noop_styled_ostream_implementation *vtable =
    ((struct noop_styled_ostream_representation_header *) (struct noop_styled_ostream_representation *) first_arg)->vtable;
  vtable->flush (first_arg,scope);
}

# define noop_styled_ostream_free noop_styled_ostream_free_inline
static inline void
noop_styled_ostream_free (noop_styled_ostream_t first_arg)
{
  const struct noop_styled_ostream_implementation *vtable =
    ((struct noop_styled_ostream_representation_header *) (struct noop_styled_ostream_representation *) first_arg)->vtable;
  vtable->free (first_arg);
}

# define noop_styled_ostream_begin_use_class noop_styled_ostream_begin_use_class_inline
static inline void
noop_styled_ostream_begin_use_class (noop_styled_ostream_t first_arg, const char *classname)
{
  const struct noop_styled_ostream_implementation *vtable =
    ((struct noop_styled_ostream_representation_header *) (struct noop_styled_ostream_representation *) first_arg)->vtable;
  vtable->begin_use_class (first_arg,classname);
}

# define noop_styled_ostream_end_use_class noop_styled_ostream_end_use_class_inline
static inline void
noop_styled_ostream_end_use_class (noop_styled_ostream_t first_arg, const char *classname)
{
  const struct noop_styled_ostream_implementation *vtable =
    ((struct noop_styled_ostream_representation_header *) (struct noop_styled_ostream_representation *) first_arg)->vtable;
  vtable->end_use_class (first_arg,classname);
}

# define noop_styled_ostream_get_hyperlink_ref noop_styled_ostream_get_hyperlink_ref_inline
static inline const char *
noop_styled_ostream_get_hyperlink_ref (noop_styled_ostream_t first_arg)
{
  const struct noop_styled_ostream_implementation *vtable =
    ((struct noop_styled_ostream_representation_header *) (struct noop_styled_ostream_representation *) first_arg)->vtable;
  return vtable->get_hyperlink_ref (first_arg);
}

# define noop_styled_ostream_get_hyperlink_id noop_styled_ostream_get_hyperlink_id_inline
static inline const char *
noop_styled_ostream_get_hyperlink_id (noop_styled_ostream_t first_arg)
{
  const struct noop_styled_ostream_implementation *vtable =
    ((struct noop_styled_ostream_representation_header *) (struct noop_styled_ostream_representation *) first_arg)->vtable;
  return vtable->get_hyperlink_id (first_arg);
}

# define noop_styled_ostream_set_hyperlink noop_styled_ostream_set_hyperlink_inline
static inline void
noop_styled_ostream_set_hyperlink (noop_styled_ostream_t first_arg,                               const char *ref, const char *id)
{
  const struct noop_styled_ostream_implementation *vtable =
    ((struct noop_styled_ostream_representation_header *) (struct noop_styled_ostream_representation *) first_arg)->vtable;
  vtable->set_hyperlink (first_arg,ref,id);
}

# define noop_styled_ostream_flush_to_current_style noop_styled_ostream_flush_to_current_style_inline
static inline void
noop_styled_ostream_flush_to_current_style (noop_styled_ostream_t first_arg)
{
  const struct noop_styled_ostream_implementation *vtable =
    ((struct noop_styled_ostream_representation_header *) (struct noop_styled_ostream_representation *) first_arg)->vtable;
  vtable->flush_to_current_style (first_arg);
}

# define noop_styled_ostream_get_destination noop_styled_ostream_get_destination_inline
static inline ostream_t
noop_styled_ostream_get_destination (noop_styled_ostream_t first_arg)
{
  const struct noop_styled_ostream_implementation *vtable =
    ((struct noop_styled_ostream_representation_header *) (struct noop_styled_ostream_representation *) first_arg)->vtable;
  return vtable->get_destination (first_arg);
}

# define noop_styled_ostream_is_owning_destination noop_styled_ostream_is_owning_destination_inline
static inline bool
noop_styled_ostream_is_owning_destination (noop_styled_ostream_t first_arg)
{
  const struct noop_styled_ostream_implementation *vtable =
    ((struct noop_styled_ostream_representation_header *) (struct noop_styled_ostream_representation *) first_arg)->vtable;
  return vtable->is_owning_destination (first_arg);
}

#endif

extern const typeinfo_t noop_styled_ostream_typeinfo;
#define noop_styled_ostream_SUPERCLASSES &noop_styled_ostream_typeinfo, styled_ostream_SUPERCLASSES
#define noop_styled_ostream_SUPERCLASSES_LENGTH (1 + styled_ostream_SUPERCLASSES_LENGTH)

extern const struct noop_styled_ostream_implementation noop_styled_ostream_vtable;

#line 33 "noop-styled-ostream.oo.h"


#ifdef __cplusplus
extern "C" {
#endif


/* Create an output stream that delegates to DESTINATION and that supports
   the styling operations as no-ops.
   If PASS_OWNERSHIP is true, closing the resulting stream will automatically
   close the DESTINATION.
   Note that if PASS_OWNERSHIP is false, the resulting stream must be closed
   before DESTINATION can be closed.  */
extern noop_styled_ostream_t
       noop_styled_ostream_create (ostream_t destination, bool pass_ownership);


/* Test whether a given output stream is a noop_styled_ostream.  */
extern bool is_instance_of_noop_styled_ostream (ostream_t stream);


#ifdef __cplusplus
}
#endif

#endif /* _NOOP_STYLED_OSTREAM_H */
