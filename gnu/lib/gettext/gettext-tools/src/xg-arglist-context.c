/* Keeping track of the flags that apply to a string extracted
   in a certain context.
   Copyright (C) 2001-2018, 2023 Free Software Foundation, Inc.

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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/* Specification.  */
#include "xg-arglist-context.h"

#include <stdlib.h>

#include "xalloc.h"
#include "xmalloca.h"
#include "verify.h"


/* Null context.  */
flag_context_ty null_context =
  {
    undecided, false,
    undecided, false,
    undecided, false,
    undecided, false
  };

/* Transparent context.  */
flag_context_ty passthrough_context =
  {
    undecided, true,
    undecided, true,
    undecided, true,
    undecided, true
  };


flag_context_ty
inherited_context (flag_context_ty outer_context,
                   flag_context_ty modifier_context)
{
  flag_context_ty result = modifier_context;

  if (result.pass_format1)
    {
      result.is_format1 = outer_context.is_format1;
      result.pass_format1 = false;
    }
  if (result.pass_format2)
    {
      result.is_format2 = outer_context.is_format2;
      result.pass_format2 = false;
    }
  if (result.pass_format3)
    {
      result.is_format3 = outer_context.is_format3;
      result.pass_format3 = false;
    }
  if (result.pass_format4)
    {
      result.is_format4 = outer_context.is_format4;
      result.pass_format4 = false;
    }
  return result;
}


/* Null context list iterator.  */
flag_context_list_iterator_ty null_context_list_iterator = { 1, NULL };

/* Transparent context list iterator.  */
static flag_context_list_ty passthrough_context_circular_list =
  {
    1,
    { undecided, true, undecided, true, undecided, true, undecided, true },
    &passthrough_context_circular_list
  };
flag_context_list_iterator_ty passthrough_context_list_iterator =
  {
    1,
    &passthrough_context_circular_list
  };


flag_context_list_iterator_ty
flag_context_list_iterator (flag_context_list_ty *list)
{
  flag_context_list_iterator_ty result;

  result.argnum = 1;
  result.head = list;
  return result;
}


flag_context_ty
flag_context_list_iterator_advance (flag_context_list_iterator_ty *iter)
{
  if (iter->head == NULL)
    return null_context;
  if (iter->argnum == iter->head->argnum)
    {
      flag_context_ty result = iter->head->flags;

      /* Special casing of circular list.  */
      if (iter->head != iter->head->next)
        {
          iter->head = iter->head->next;
          iter->argnum++;
        }

      return result;
    }
  else
    {
      iter->argnum++;
      return null_context;
    }
}


flag_context_list_ty *
flag_context_list_table_lookup (flag_context_list_table_ty *flag_table,
                                const void *key, size_t keylen)
{
  void *entry;

  if (flag_table->table != NULL
      && hash_find_entry (flag_table, key, keylen, &entry) == 0)
    return (flag_context_list_ty *) entry;
  else
    return NULL;
}


/* In the FLAGS, set the pair (is_formatX, pass_formatX) with X = INDEX+1
   to (VALUE, PASS).  */
static void
set_flags_for_formatstring_type (flag_context_ty *flags, unsigned int index,
                                 enum is_format value, bool pass)
{
  switch (index)
    {
    case 0:
      flags->is_format1 = value;
      flags->pass_format1 = pass;
      break;
    case 1:
      flags->is_format2 = value;
      flags->pass_format2 = pass;
      break;
    case 2:
      flags->is_format3 = value;
      flags->pass_format3 = pass;
      break;
    case 3:
      flags->is_format4 = value;
      flags->pass_format4 = pass;
      break;
    default:
      abort ();
    }
}


void
flag_context_list_table_add (flag_context_list_table_ty *table,
                             unsigned int index,
                             const char *name_start, const char *name_end,
                             int argnum, enum is_format value, bool pass)
{
  if (table->table == NULL)
    hash_init (table, 100);
  {
    void *entry;

    if (hash_find_entry (table, name_start, name_end - name_start, &entry) != 0)
      {
        /* Create new hash table entry.  */
        flag_context_list_ty *list = XMALLOC (flag_context_list_ty);
        list->argnum = argnum;
        memset (&list->flags, '\0', sizeof (list->flags));
        set_flags_for_formatstring_type (&list->flags, index, value, pass);
        list->next = NULL;
        hash_insert_entry (table, name_start, name_end - name_start, list);
      }
    else
      {
        /* We don't put NULL entries into the table.  */
        assume (entry != NULL);

        flag_context_list_ty *list = (flag_context_list_ty *)entry;
        flag_context_list_ty **lastp = NULL;
        /* Invariant: list == (lastp != NULL ? *lastp : entry).  */

        while (list != NULL && list->argnum < argnum)
          {
            lastp = &list->next;
            list = *lastp;
          }
        if (list != NULL && list->argnum == argnum)
          {
            /* Add this flag to the current argument number.  */
            set_flags_for_formatstring_type (&list->flags, index, value, pass);
          }
        else if (lastp != NULL)
          {
            /* Add a new list entry for this argument number.  */
            list = XMALLOC (flag_context_list_ty);
            list->argnum = argnum;
            memset (&list->flags, '\0', sizeof (list->flags));
            set_flags_for_formatstring_type (&list->flags, index, value, pass);
            list->next = *lastp;
            *lastp = list;
          }
        else
          {
            /* Add a new list entry for this argument number, at the beginning
               of the list.  Since we don't have an API for replacing the
               value of a key in the hash table, we have to copy the first
               list element.  */
            flag_context_list_ty *copy = XMALLOC (flag_context_list_ty);
            *copy = *list;

            list->argnum = argnum;
            memset (&list->flags, '\0', sizeof (list->flags));
            set_flags_for_formatstring_type (&list->flags, index, value, pass);
            list->next = copy;
          }
      }
  }
}
