/* Reading Desktop Entry files.
   Copyright (C) 1995-1998, 2000-2003, 2005-2006, 2008-2009, 2014-2016, 2020 Free Software Foundation, Inc.
   This file was written by Daiki Ueno <ueno@gnu.org>.

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

#ifndef _READ_DESKTOP_H
#define _READ_DESKTOP_H

#include <sys/types.h>
#include <stdio.h>
#include "mem-hash-map.h"
#include "po-lex.h"
#include "str-list.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declaration.  */
struct desktop_reader_ty;


/* This first structure, playing the role of the "Class" in OO sense,
   contains pointers to functions.  Each function is a method for the
   class (base or derived).  Use a NULL pointer where no action is
   required.  */

typedef struct desktop_reader_class_ty desktop_reader_class_ty;
struct desktop_reader_class_ty
{
  /* how many bytes to malloc for an instance of this class */
  size_t size;

  /* what to do immediately after the instance is malloc()ed */
  void (*constructor) (struct desktop_reader_ty *pop);

  /* what to do immediately before the instance is free()ed */
  void (*destructor) (struct desktop_reader_ty *pop);

  /* what to do with a group header */
  void (*handle_group) (struct desktop_reader_ty *pop,
                        const char *group);

  /* what to do with a key/value pair */
  void (*handle_pair) (struct desktop_reader_ty *pop,
                       lex_pos_ty *key_pos,
                       const char *key,
                       const char *locale,
                       const char *value);

  /* what to do with a comment */
  void (*handle_comment) (struct desktop_reader_ty *pop, const char *s);

  /* what to do with a blank line */
  void (*handle_blank) (struct desktop_reader_ty *pop, const char *s);
};

/* This next structure defines the base class passed to the methods.
   Derived methods will often need to cast their first argument before
   using it (this corresponds to the implicit 'this' argument in C++).

   When declaring derived classes, use the DESKTOP_READER_TY define
   at the start of the structure, to declare inherited instance variables,
   etc.  */

#define DESKTOP_READER_TY              \
  desktop_reader_class_ty *methods;

typedef struct desktop_reader_ty desktop_reader_ty;
struct desktop_reader_ty
{
  DESKTOP_READER_TY
};

extern desktop_reader_ty *
       desktop_reader_alloc (desktop_reader_class_ty *methods);
extern void desktop_reader_free (desktop_reader_ty *reader);

extern void desktop_reader_handle_group (desktop_reader_ty *reader,
                                         const char *group);

extern void desktop_reader_handle_pair (desktop_reader_ty *reader,
                                        lex_pos_ty *key_pos,
                                 const char *key,
                                 const char *locale,
                                 const char *value);

extern void desktop_reader_handle_comment (desktop_reader_ty *reader,
                                           const char *s);

extern void desktop_reader_handle_blank (desktop_reader_ty *reader,
                                         const char *s);


extern void desktop_parse (desktop_reader_ty *reader, FILE *file,
                           const char *real_filename,
                           const char *logical_filename);


extern char *desktop_escape_string (const char *s, bool is_list);
extern char *desktop_unescape_string (const char *s, bool is_list);

extern void desktop_add_keyword (hash_table *keywords, const char *name,
                                 bool is_list);
extern void desktop_add_default_keywords (hash_table *keywords);

#ifdef __cplusplus
}
#endif


#endif /* _READ_DESKTOP_H */
