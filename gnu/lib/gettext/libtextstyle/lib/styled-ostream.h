/* DO NOT EDIT! GENERATED AUTOMATICALLY! */

#line 1 "styled-ostream.oo.h"
/* Abstract output stream for CSS styled text.
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

#ifndef _STYLED_OSTREAM_H
#define _STYLED_OSTREAM_H

#include <stdbool.h>

#include "ostream.h"


/* A styled output stream is an object to which one can feed a sequence of
   bytes, marking some runs of text as belonging to specific CSS classes,
   where the rendering of the CSS classes is defined through a CSS (cascading
   style sheet).  */

#line 35 "styled-ostream.h"
struct styled_ostream_representation;
/* styled_ostream_t is defined as a pointer to struct styled_ostream_representation.
   In C++ mode, we use a smart pointer class.
   In C mode, we have no other choice than a typedef to the root class type.  */
#if IS_CPLUSPLUS
struct styled_ostream_t
{
private:
  struct styled_ostream_representation *_pointer;
public:
  styled_ostream_t () : _pointer (NULL) {}
  styled_ostream_t (struct styled_ostream_representation *pointer) : _pointer (pointer) {}
  struct styled_ostream_representation * operator -> () { return _pointer; }
  operator struct styled_ostream_representation * () { return _pointer; }
  operator struct any_ostream_representation * () { return (struct any_ostream_representation *) _pointer; }
  operator void * () { return _pointer; }
  bool operator == (const void *p) { return _pointer == p; }
  bool operator != (const void *p) { return _pointer != p; }
  operator ostream_t () { return (ostream_t) (struct any_ostream_representation *) _pointer; }
  explicit styled_ostream_t (ostream_t x) : _pointer ((struct styled_ostream_representation *) (void *) x) {}
};
#else
typedef ostream_t styled_ostream_t;
#endif

/* Functions that invoke the methods.  */
#ifdef __cplusplus
extern "C" {
#endif
extern        void styled_ostream_write_mem (styled_ostream_t first_arg, const void *data, size_t len);
extern         void styled_ostream_flush (styled_ostream_t first_arg, ostream_flush_scope_t scope);
extern         void styled_ostream_free (styled_ostream_t first_arg);
extern          void styled_ostream_begin_use_class (styled_ostream_t first_arg, const char *classname);
extern          void styled_ostream_end_use_class (styled_ostream_t first_arg, const char *classname);
extern         const char * styled_ostream_get_hyperlink_ref (styled_ostream_t first_arg);
extern    const char * styled_ostream_get_hyperlink_id (styled_ostream_t first_arg);
extern    void         styled_ostream_set_hyperlink (styled_ostream_t first_arg,                               const char *ref, const char *id);
extern              void styled_ostream_flush_to_current_style (styled_ostream_t first_arg);
#ifdef __cplusplus
}
#endif

/* Type representing an implementation of styled_ostream_t.  */
struct styled_ostream_implementation
{
  const typeinfo_t * const *superclasses;
  size_t superclasses_length;
  size_t instance_size;
#define THIS_ARG styled_ostream_t first_arg
#include "styled_ostream.vt.h"
#undef THIS_ARG
};

/* Public portion of the object pointed to by a styled_ostream_t.  */
struct styled_ostream_representation_header
{
  const struct styled_ostream_implementation *vtable;
};

#if HAVE_INLINE

/* Define the functions that invoke the methods as inline accesses to
   the styled_ostream_implementation.
   Use #define to avoid a warning because of extern vs. static.  */

# define styled_ostream_write_mem styled_ostream_write_mem_inline
static inline void
styled_ostream_write_mem (styled_ostream_t first_arg, const void *data, size_t len)
{
  const struct styled_ostream_implementation *vtable =
    ((struct styled_ostream_representation_header *) (struct styled_ostream_representation *) first_arg)->vtable;
  vtable->write_mem (first_arg,data,len);
}

# define styled_ostream_flush styled_ostream_flush_inline
static inline void
styled_ostream_flush (styled_ostream_t first_arg, ostream_flush_scope_t scope)
{
  const struct styled_ostream_implementation *vtable =
    ((struct styled_ostream_representation_header *) (struct styled_ostream_representation *) first_arg)->vtable;
  vtable->flush (first_arg,scope);
}

# define styled_ostream_free styled_ostream_free_inline
static inline void
styled_ostream_free (styled_ostream_t first_arg)
{
  const struct styled_ostream_implementation *vtable =
    ((struct styled_ostream_representation_header *) (struct styled_ostream_representation *) first_arg)->vtable;
  vtable->free (first_arg);
}

# define styled_ostream_begin_use_class styled_ostream_begin_use_class_inline
static inline void
styled_ostream_begin_use_class (styled_ostream_t first_arg, const char *classname)
{
  const struct styled_ostream_implementation *vtable =
    ((struct styled_ostream_representation_header *) (struct styled_ostream_representation *) first_arg)->vtable;
  vtable->begin_use_class (first_arg,classname);
}

# define styled_ostream_end_use_class styled_ostream_end_use_class_inline
static inline void
styled_ostream_end_use_class (styled_ostream_t first_arg, const char *classname)
{
  const struct styled_ostream_implementation *vtable =
    ((struct styled_ostream_representation_header *) (struct styled_ostream_representation *) first_arg)->vtable;
  vtable->end_use_class (first_arg,classname);
}

# define styled_ostream_get_hyperlink_ref styled_ostream_get_hyperlink_ref_inline
static inline const char *
styled_ostream_get_hyperlink_ref (styled_ostream_t first_arg)
{
  const struct styled_ostream_implementation *vtable =
    ((struct styled_ostream_representation_header *) (struct styled_ostream_representation *) first_arg)->vtable;
  return vtable->get_hyperlink_ref (first_arg);
}

# define styled_ostream_get_hyperlink_id styled_ostream_get_hyperlink_id_inline
static inline const char *
styled_ostream_get_hyperlink_id (styled_ostream_t first_arg)
{
  const struct styled_ostream_implementation *vtable =
    ((struct styled_ostream_representation_header *) (struct styled_ostream_representation *) first_arg)->vtable;
  return vtable->get_hyperlink_id (first_arg);
}

# define styled_ostream_set_hyperlink styled_ostream_set_hyperlink_inline
static inline void
styled_ostream_set_hyperlink (styled_ostream_t first_arg,                               const char *ref, const char *id)
{
  const struct styled_ostream_implementation *vtable =
    ((struct styled_ostream_representation_header *) (struct styled_ostream_representation *) first_arg)->vtable;
  vtable->set_hyperlink (first_arg,ref,id);
}

# define styled_ostream_flush_to_current_style styled_ostream_flush_to_current_style_inline
static inline void
styled_ostream_flush_to_current_style (styled_ostream_t first_arg)
{
  const struct styled_ostream_implementation *vtable =
    ((struct styled_ostream_representation_header *) (struct styled_ostream_representation *) first_arg)->vtable;
  vtable->flush_to_current_style (first_arg);
}

#endif

extern const typeinfo_t styled_ostream_typeinfo;
#define styled_ostream_SUPERCLASSES &styled_ostream_typeinfo, ostream_SUPERCLASSES
#define styled_ostream_SUPERCLASSES_LENGTH (1 + ostream_SUPERCLASSES_LENGTH)

extern const struct styled_ostream_implementation styled_ostream_vtable;

#line 58 "styled-ostream.oo.h"

#ifdef __cplusplus
extern "C" {
#endif


/* Test whether a given output stream is a styled_ostream.  */
extern bool is_instance_of_styled_ostream (ostream_t stream);


#ifdef __cplusplus
}
#endif

#endif /* _STYLED_OSTREAM_H */
