/* DO NOT EDIT! GENERATED AUTOMATICALLY! */

#line 1 "html-ostream.oo.h"
/* Output stream that produces HTML output.
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

#ifndef _HTML_OSTREAM_H
#define _HTML_OSTREAM_H

#include <stdbool.h>

#include "ostream.h"


#line 30 "html-ostream.h"
struct html_ostream_representation;
/* html_ostream_t is defined as a pointer to struct html_ostream_representation.
   In C++ mode, we use a smart pointer class.
   In C mode, we have no other choice than a typedef to the root class type.  */
#if IS_CPLUSPLUS
struct html_ostream_t
{
private:
  struct html_ostream_representation *_pointer;
public:
  html_ostream_t () : _pointer (NULL) {}
  html_ostream_t (struct html_ostream_representation *pointer) : _pointer (pointer) {}
  struct html_ostream_representation * operator -> () { return _pointer; }
  operator struct html_ostream_representation * () { return _pointer; }
  operator struct any_ostream_representation * () { return (struct any_ostream_representation *) _pointer; }
  operator void * () { return _pointer; }
  bool operator == (const void *p) { return _pointer == p; }
  bool operator != (const void *p) { return _pointer != p; }
  operator ostream_t () { return (ostream_t) (struct any_ostream_representation *) _pointer; }
  explicit html_ostream_t (ostream_t x) : _pointer ((struct html_ostream_representation *) (void *) x) {}
};
#else
typedef ostream_t html_ostream_t;
#endif

/* Functions that invoke the methods.  */
#ifdef __cplusplus
extern "C" {
#endif
extern        void html_ostream_write_mem (html_ostream_t first_arg, const void *data, size_t len);
extern         void html_ostream_flush (html_ostream_t first_arg, ostream_flush_scope_t scope);
extern         void html_ostream_free (html_ostream_t first_arg);
extern          void html_ostream_begin_span (html_ostream_t first_arg, const char *classname);
extern          void html_ostream_end_span (html_ostream_t first_arg, const char *classname);
extern         const char * html_ostream_get_hyperlink_ref (html_ostream_t first_arg);
extern    void html_ostream_set_hyperlink_ref (html_ostream_t first_arg, const char *ref);
extern              void html_ostream_flush_to_current_style (html_ostream_t first_arg);
extern         ostream_t html_ostream_get_destination (html_ostream_t first_arg);
#ifdef __cplusplus
}
#endif

/* Type representing an implementation of html_ostream_t.  */
struct html_ostream_implementation
{
  const typeinfo_t * const *superclasses;
  size_t superclasses_length;
  size_t instance_size;
#define THIS_ARG html_ostream_t first_arg
#include "html_ostream.vt.h"
#undef THIS_ARG
};

/* Public portion of the object pointed to by a html_ostream_t.  */
struct html_ostream_representation_header
{
  const struct html_ostream_implementation *vtable;
};

#if HAVE_INLINE

/* Define the functions that invoke the methods as inline accesses to
   the html_ostream_implementation.
   Use #define to avoid a warning because of extern vs. static.  */

# define html_ostream_write_mem html_ostream_write_mem_inline
static inline void
html_ostream_write_mem (html_ostream_t first_arg, const void *data, size_t len)
{
  const struct html_ostream_implementation *vtable =
    ((struct html_ostream_representation_header *) (struct html_ostream_representation *) first_arg)->vtable;
  vtable->write_mem (first_arg,data,len);
}

# define html_ostream_flush html_ostream_flush_inline
static inline void
html_ostream_flush (html_ostream_t first_arg, ostream_flush_scope_t scope)
{
  const struct html_ostream_implementation *vtable =
    ((struct html_ostream_representation_header *) (struct html_ostream_representation *) first_arg)->vtable;
  vtable->flush (first_arg,scope);
}

# define html_ostream_free html_ostream_free_inline
static inline void
html_ostream_free (html_ostream_t first_arg)
{
  const struct html_ostream_implementation *vtable =
    ((struct html_ostream_representation_header *) (struct html_ostream_representation *) first_arg)->vtable;
  vtable->free (first_arg);
}

# define html_ostream_begin_span html_ostream_begin_span_inline
static inline void
html_ostream_begin_span (html_ostream_t first_arg, const char *classname)
{
  const struct html_ostream_implementation *vtable =
    ((struct html_ostream_representation_header *) (struct html_ostream_representation *) first_arg)->vtable;
  vtable->begin_span (first_arg,classname);
}

# define html_ostream_end_span html_ostream_end_span_inline
static inline void
html_ostream_end_span (html_ostream_t first_arg, const char *classname)
{
  const struct html_ostream_implementation *vtable =
    ((struct html_ostream_representation_header *) (struct html_ostream_representation *) first_arg)->vtable;
  vtable->end_span (first_arg,classname);
}

# define html_ostream_get_hyperlink_ref html_ostream_get_hyperlink_ref_inline
static inline const char *
html_ostream_get_hyperlink_ref (html_ostream_t first_arg)
{
  const struct html_ostream_implementation *vtable =
    ((struct html_ostream_representation_header *) (struct html_ostream_representation *) first_arg)->vtable;
  return vtable->get_hyperlink_ref (first_arg);
}

# define html_ostream_set_hyperlink_ref html_ostream_set_hyperlink_ref_inline
static inline void
html_ostream_set_hyperlink_ref (html_ostream_t first_arg, const char *ref)
{
  const struct html_ostream_implementation *vtable =
    ((struct html_ostream_representation_header *) (struct html_ostream_representation *) first_arg)->vtable;
  vtable->set_hyperlink_ref (first_arg,ref);
}

# define html_ostream_flush_to_current_style html_ostream_flush_to_current_style_inline
static inline void
html_ostream_flush_to_current_style (html_ostream_t first_arg)
{
  const struct html_ostream_implementation *vtable =
    ((struct html_ostream_representation_header *) (struct html_ostream_representation *) first_arg)->vtable;
  vtable->flush_to_current_style (first_arg);
}

# define html_ostream_get_destination html_ostream_get_destination_inline
static inline ostream_t
html_ostream_get_destination (html_ostream_t first_arg)
{
  const struct html_ostream_implementation *vtable =
    ((struct html_ostream_representation_header *) (struct html_ostream_representation *) first_arg)->vtable;
  return vtable->get_destination (first_arg);
}

#endif

extern const typeinfo_t html_ostream_typeinfo;
#define html_ostream_SUPERCLASSES &html_ostream_typeinfo, ostream_SUPERCLASSES
#define html_ostream_SUPERCLASSES_LENGTH (1 + ostream_SUPERCLASSES_LENGTH)

extern const struct html_ostream_implementation html_ostream_vtable;

#line 54 "html-ostream.oo.h"


#ifdef __cplusplus
extern "C" {
#endif


/* Create an output stream that takes input in the UTF-8 encoding and
   writes it in HTML form on DESTINATION.
   This stream produces a sequence of lines.  The caller is responsible
   for opening the <body><html> elements before and for closing them after
   the use of this stream.
   Note that the resulting stream must be closed before DESTINATION can be
   closed.  */
extern html_ostream_t html_ostream_create (ostream_t destination);


/* Test whether a given output stream is a html_ostream.  */
extern bool is_instance_of_html_ostream (ostream_t stream);


#ifdef __cplusplus
}
#endif

#endif /* _HTML_OSTREAM_H */
