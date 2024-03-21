/* Keeping track of the flags that apply to a string extracted
   in a certain context.
   Copyright (C) 2001-2018, 2020, 2023 Free Software Foundation, Inc.

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

#ifndef _XGETTEXT_ARGLIST_CONTEXT_H
#define _XGETTEXT_ARGLIST_CONTEXT_H

#include <stdbool.h>

#include "mem-hash-map.h"
#include "message.h"

#ifdef __cplusplus
extern "C" {
#endif


/* Context representing some flags.  */
typedef struct flag_context_ty flag_context_ty;
struct flag_context_ty
{
  /* Regarding the primary formatstring type.  */
  /*enum is_format*/ unsigned int is_format1    : 3;
  /*bool*/           unsigned int pass_format1  : 1;
  /* Regarding the secondary formatstring type.  */
  /*enum is_format*/ unsigned int is_format2    : 3;
  /*bool*/           unsigned int pass_format2  : 1;
  /* Regarding the tertiary formatstring type.  */
  /*enum is_format*/ unsigned int is_format3    : 3;
  /*bool*/           unsigned int pass_format3  : 1;
  /* Regarding the fourth-ranked formatstring type.  */
  /*enum is_format*/ unsigned int is_format4    : 3;
  /*bool*/           unsigned int pass_format4  : 1;
};
/* Null context.  */
extern flag_context_ty null_context;
/* Transparent context.  */
extern flag_context_ty passthrough_context;
/* Compute an inherited context.
   The outer_context is assumed to have all pass_format* flags = false.
   The result will then also have all pass_format* flags = false.  */
extern flag_context_ty
       inherited_context (flag_context_ty outer_context,
                          flag_context_ty modifier_context);

/* Context representing some flags, for each possible argument number.
   This is a linked list, sorted according to the argument number.  */
typedef struct flag_context_list_ty flag_context_list_ty;
struct flag_context_list_ty
{
  int argnum;                   /* current argument number, > 0 */
  flag_context_ty flags;        /* flags for current argument */
  flag_context_list_ty *next;
};

/* Iterator through a flag_context_list_ty.  */
typedef struct flag_context_list_iterator_ty flag_context_list_iterator_ty;
struct flag_context_list_iterator_ty
{
  int argnum;                           /* current argument number, > 0 */
  const flag_context_list_ty* head;     /* tail of list */
};
extern flag_context_list_iterator_ty null_context_list_iterator;
extern flag_context_list_iterator_ty passthrough_context_list_iterator;
extern flag_context_list_iterator_ty
       flag_context_list_iterator (flag_context_list_ty *list);
extern flag_context_ty
       flag_context_list_iterator_advance (flag_context_list_iterator_ty *iter);

/* For nearly each backend, we have a separate table mapping a keyword to
   a flag_context_list_ty *.  */
typedef hash_table /* char[] -> flag_context_list_ty * */
        flag_context_list_table_ty;
extern flag_context_list_ty *
       flag_context_list_table_lookup (flag_context_list_table_ty *flag_table,
                                       const void *key, size_t keylen);
/* Insert the pair (VALUE, PASS) as (is_formatX, pass_formatX) with X = INDEX+1
   in the flags of the element numbered ARGNUM of the list corresponding to NAME
   in the TABLE.  */
extern void
       flag_context_list_table_add (flag_context_list_table_ty *table,
                                    unsigned int index,
                                    const char *name_start, const char *name_end,
                                    int argnum, enum is_format value, bool pass);


#ifdef __cplusplus
}
#endif


#endif /* _XGETTEXT_ARGLIST_CONTEXT_H */
