/* GNU gettext - internationalization aids
   Copyright (C) 1995-1996, 1998, 2000-2004, 2009, 2020 Free Software
   Foundation, Inc.

   This file was written by Peter Miller <millerp@canb.auug.org.au>

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

#ifndef _STR_LIST_H
#define _STR_LIST_H 1

/* Get size_t and NULL.  */
#include <stddef.h>

/* Get bool.  */
#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif


/* Type describing list of immutable strings,
   implemented using a dynamic array.  */
typedef struct string_list_ty string_list_ty;
struct string_list_ty
{
  const char **item;
  size_t nitems;
  size_t nitems_max;
};

/* Initialize an empty list of strings.  */
extern void string_list_init (string_list_ty *slp);

/* Return a fresh, empty list of strings.  */
extern string_list_ty *string_list_alloc (void);

/* Append a single string to the end of a list of strings.  */
extern void string_list_append (string_list_ty *slp, const char *s);

/* Append a single string to the end of a list of strings, unless it is
   already contained in the list.  */
extern void string_list_append_unique (string_list_ty *slp, const char *s);
/* Likewise with a string descriptor as argument.  */
extern void string_list_append_unique_desc (string_list_ty *slp,
                                            const char *s, size_t s_len);

/* Destroy a list of strings.  */
extern void string_list_destroy (string_list_ty *slp);

/* Free a list of strings.  */
extern void string_list_free (string_list_ty *slp);

/* Return a freshly allocated string obtained by concatenating all the
   strings in the list.  */
extern char *string_list_concat (const string_list_ty *slp);

/* Return a freshly allocated string obtained by concatenating all the
   strings in the list, and destroy the list.  */
extern char *string_list_concat_destroy (string_list_ty *slp);

/* Return a freshly allocated string obtained by concatenating all the
   strings in the list, separated by the separator string, terminated
   by the terminator character.  The terminator character is not added if
   drop_redundant_terminator is true and the last string already ends with
   the terminator. */
extern char *string_list_join (const string_list_ty *slp, const char *separator,
                               char terminator, bool drop_redundant_terminator);

/* Return 1 if s is contained in the list of strings, 0 otherwise.  */
extern bool string_list_member (const string_list_ty *slp, const char *s);
/* Likewise with a string descriptor as argument.  */
extern bool string_list_member_desc (const string_list_ty *slp,
                                     const char *s, size_t s_len);

/* Remove s from the list of strings.  Return the removed string or NULL.  */
extern const char * string_list_remove (string_list_ty *slp, const char *s);


#ifdef __cplusplus
}
#endif


#endif /* _STR_LIST_H */
