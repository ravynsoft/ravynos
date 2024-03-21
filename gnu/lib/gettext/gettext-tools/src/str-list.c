/* GNU gettext - internationalization aids
   Copyright (C) 1995, 1998, 2000-2004, 2006, 2009, 2020 Free Software
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

/* Specification.  */
#include "str-list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xalloc.h"


/* Initialize an empty list of strings.  */
void
string_list_init (string_list_ty *slp)
{
  slp->item = NULL;
  slp->nitems = 0;
  slp->nitems_max = 0;
}


/* Return a fresh, empty list of strings.  */
string_list_ty *
string_list_alloc ()
{
  string_list_ty *slp;

  slp = XMALLOC (string_list_ty);
  slp->item = NULL;
  slp->nitems = 0;
  slp->nitems_max = 0;

  return slp;
}


/* Append a single string to the end of a list of strings.  */
void
string_list_append (string_list_ty *slp, const char *s)
{
  /* Grow the list.  */
  if (slp->nitems >= slp->nitems_max)
    {
      size_t nbytes;

      slp->nitems_max = slp->nitems_max * 2 + 4;
      nbytes = slp->nitems_max * sizeof (slp->item[0]);
      slp->item = (const char **) xrealloc (slp->item, nbytes);
    }

  /* Add a copy of the string to the end of the list.  */
  slp->item[slp->nitems++] = xstrdup (s);
}


/* Append a single string to the end of a list of strings, unless it is
   already contained in the list.  */
void
string_list_append_unique (string_list_ty *slp, const char *s)
{
  size_t j;

  /* Do nothing if the string is already in the list.  */
  for (j = 0; j < slp->nitems; ++j)
    if (strcmp (slp->item[j], s) == 0)
      return;

  /* Grow the list.  */
  if (slp->nitems >= slp->nitems_max)
    {
      slp->nitems_max = slp->nitems_max * 2 + 4;
      slp->item = (const char **) xrealloc (slp->item,
                                            slp->nitems_max
                                            * sizeof (slp->item[0]));
    }

  /* Add a copy of the string to the end of the list.  */
  slp->item[slp->nitems++] = xstrdup (s);
}

/* Likewise with a string descriptor as argument.  */
void
string_list_append_unique_desc (string_list_ty *slp,
                                const char *s, size_t s_len)
{
  size_t j;

  /* Do nothing if the string is already in the list.  */
  for (j = 0; j < slp->nitems; ++j)
    if (strlen (slp->item[j]) == s_len && memcmp (slp->item[j], s, s_len) == 0)
      return;

  /* Grow the list.  */
  if (slp->nitems >= slp->nitems_max)
    {
      slp->nitems_max = slp->nitems_max * 2 + 4;
      slp->item = (const char **) xrealloc (slp->item,
                                            slp->nitems_max
                                            * sizeof (slp->item[0]));
    }

  /* Add a copy of the string to the end of the list.  */
  {
    char *copy = XNMALLOC (s_len + 1, char);
    memcpy (copy, s, s_len);
    copy[s_len] = '\0';

    slp->item[slp->nitems++] = copy;
  }
}


/* Destroy a list of strings.  */
void
string_list_destroy (string_list_ty *slp)
{
  size_t j;

  for (j = 0; j < slp->nitems; ++j)
    free ((char *) slp->item[j]);
  if (slp->item != NULL)
    free (slp->item);
}


/* Free a list of strings.  */
void
string_list_free (string_list_ty *slp)
{
  size_t j;

  for (j = 0; j < slp->nitems; ++j)
    free ((char *) slp->item[j]);
  if (slp->item != NULL)
    free (slp->item);
  free (slp);
}


/* Return a freshly allocated string obtained by concatenating all the
   strings in the list.  */
char *
string_list_concat (const string_list_ty *slp)
{
  size_t len;
  size_t j;
  char *result;
  size_t pos;

  len = 1;
  for (j = 0; j < slp->nitems; ++j)
    len += strlen (slp->item[j]);
  result = XNMALLOC (len, char);
  pos = 0;
  for (j = 0; j < slp->nitems; ++j)
    {
      len = strlen (slp->item[j]);
      memcpy (result + pos, slp->item[j], len);
      pos += len;
    }
  result[pos] = '\0';
  return result;
}


/* Return a freshly allocated string obtained by concatenating all the
   strings in the list, and destroy the list.  */
char *
string_list_concat_destroy (string_list_ty *slp)
{
  char *result;

  /* Optimize the most frequent case.  */
  if (slp->nitems == 1)
    {
      result = (char *) slp->item[0];
      free (slp->item);
    }
  else
    {
      result = string_list_concat (slp);
      string_list_destroy (slp);
    }
  return result;
}


/* Return a freshly allocated string obtained by concatenating all the
   strings in the list, separated by the separator string, terminated
   by the terminator character.  The terminator character is not added if
   drop_redundant_terminator is true and the last string already ends with
   the terminator. */
char *
string_list_join (const string_list_ty *slp, const char *separator,
                  char terminator, bool drop_redundant_terminator)
{
  size_t separator_len = strlen (separator);
  size_t len;
  size_t j;
  char *result;
  size_t pos;

  len = 1;
  for (j = 0; j < slp->nitems; ++j)
    {
      if (j > 0)
        len += separator_len;
      len += strlen (slp->item[j]);
    }
  if (terminator)
    ++len;
  result = XNMALLOC (len, char);
  pos = 0;
  for (j = 0; j < slp->nitems; ++j)
    {
      if (j > 0)
        {
          memcpy (result + pos, separator, separator_len);
          pos += separator_len;
        }
      len = strlen (slp->item[j]);
      memcpy (result + pos, slp->item[j], len);
      pos += len;
    }
  if (terminator
      && !(drop_redundant_terminator
           && slp->nitems > 0
           && (len = strlen (slp->item[slp->nitems - 1])) > 0
           && slp->item[slp->nitems - 1][len - 1] == terminator))
    result[pos++] = terminator;
  result[pos] = '\0';
  return result;
}


/* Return 1 if s is contained in the list of strings, 0 otherwise.  */
bool
string_list_member (const string_list_ty *slp, const char *s)
{
  size_t j;

  for (j = 0; j < slp->nitems; ++j)
    if (strcmp (slp->item[j], s) == 0)
      return true;
  return false;
}

/* Likewise with a string descriptor as argument.  */
bool
string_list_member_desc (const string_list_ty *slp, const char *s, size_t s_len)
{
  size_t j;

  for (j = 0; j < slp->nitems; ++j)
    if (strlen (slp->item[j]) == s_len && memcmp (slp->item[j], s, s_len) == 0)
      return true;
  return false;
}


/* Remove s from the list of strings.  Return the removed string or NULL.  */
const char *
string_list_remove (string_list_ty *slp, const char *s)
{
  size_t j;

  for (j = 0; j < slp->nitems; ++j)
    if (strcmp (slp->item[j], s) == 0)
      {
        const char *found = slp->item[j];
        slp->nitems--;
        if (slp->nitems > j)
          memmove (&slp->item[j + 1], &slp->item[j],
                   (slp->nitems - j) * sizeof (const char *));
        return found;
      }
  return NULL;
}
