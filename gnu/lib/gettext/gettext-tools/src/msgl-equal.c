/* Message list test for equality.
   Copyright (C) 2001-2002, 2005-2006, 2008 Free Software Foundation, Inc.
   Written by Bruno Haible <haible@clisp.cons.org>, 2001.

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
#include "msgl-equal.h"

#include <stddef.h>
#include <string.h>


static inline bool
msgstr_equal (const char *msgstr1, size_t msgstr1_len,
              const char *msgstr2, size_t msgstr2_len)
{
  return (msgstr1_len == msgstr2_len
          && memcmp (msgstr1, msgstr2, msgstr1_len) == 0);
}

static bool
msgstr_equal_ignoring_potcdate (const char *msgstr1, size_t msgstr1_len,
                                const char *msgstr2, size_t msgstr2_len)
{
  const char *msgstr1_end = msgstr1 + msgstr1_len;
  const char *msgstr2_end = msgstr2 + msgstr2_len;
  const char *ptr1;
  const char *ptr2;
  const char *const field = "POT-Creation-Date:";
  const ptrdiff_t fieldlen = sizeof ("POT-Creation-Date:") - 1;

  /* Search for the occurrence of field in msgstr1.  */
  for (ptr1 = msgstr1;;)
    {
      if (msgstr1_end - ptr1 < fieldlen)
        {
          ptr1 = NULL;
          break;
        }
      if (memcmp (ptr1, field, fieldlen) == 0)
        break;
      ptr1 = (const char *) memchr (ptr1, '\n', msgstr1_end - ptr1);
      if (ptr1 == NULL)
        break;
      ptr1++;
    }

  /* Search for the occurrence of field in msgstr2.  */
  for (ptr2 = msgstr2;;)
    {
      if (msgstr2_end - ptr2 < fieldlen)
        {
          ptr2 = NULL;
          break;
        }
      if (memcmp (ptr2, field, fieldlen) == 0)
        break;
      ptr2 = (const char *) memchr (ptr2, '\n', msgstr2_end - ptr2);
      if (ptr2 == NULL)
        break;
      ptr2++;
    }

  if (ptr1 == NULL)
    {
      if (ptr2 == NULL)
        return msgstr_equal (msgstr1, msgstr1_len, msgstr2, msgstr2_len);
    }
  else
    {
      if (ptr2 != NULL)
        {
          /* Compare, ignoring the lines starting at ptr1 and ptr2.  */
          if (msgstr_equal (msgstr1, ptr1 - msgstr1, msgstr2, ptr2 - msgstr2))
            {
              ptr1 = (const char *) memchr (ptr1, '\n', msgstr1_end - ptr1);
              if (ptr1 == NULL)
                ptr1 = msgstr1_end;

              ptr2 = (const char *) memchr (ptr2, '\n', msgstr2_end - ptr2);
              if (ptr2 == NULL)
                ptr2 = msgstr2_end;

              return msgstr_equal (ptr1, msgstr1_end - ptr1,
                                   ptr2, msgstr2_end - ptr2);
            }
        }
    }
  return false;
}

static inline bool
pos_equal (const lex_pos_ty *pos1, const lex_pos_ty *pos2)
{
  return ((pos1->file_name == pos2->file_name
           || strcmp (pos1->file_name, pos2->file_name) == 0)
          && pos1->line_number == pos2->line_number);
}

bool
string_list_equal (const string_list_ty *slp1, const string_list_ty *slp2)
{
  size_t i, i1, i2;

  i1 = (slp1 != NULL ? slp1->nitems : 0);
  i2 = (slp2 != NULL ? slp2->nitems : 0);
  if (i1 != i2)
    return false;
  for (i = 0; i < i1; i++)
    if (strcmp (slp1->item[i], slp2->item[i]) != 0)
      return false;
  return true;
}

bool
message_equal (const message_ty *mp1, const message_ty *mp2,
               bool ignore_potcdate)
{
  size_t i, i1, i2;

  if (!(mp1->msgctxt != NULL
        ? mp2->msgctxt != NULL && strcmp (mp1->msgctxt, mp2->msgctxt) == 0
        : mp2->msgctxt == NULL))
    return false;

  if (strcmp (mp1->msgid, mp2->msgid) != 0)
    return false;

  if (!(mp1->msgid_plural != NULL
        ? mp2->msgid_plural != NULL
          && strcmp (mp1->msgid_plural, mp2->msgid_plural) == 0
        : mp2->msgid_plural == NULL))
    return false;

  if (is_header (mp1) && ignore_potcdate
      ? !msgstr_equal_ignoring_potcdate (mp1->msgstr, mp1->msgstr_len,
                                         mp2->msgstr, mp2->msgstr_len)
      : !msgstr_equal (mp1->msgstr, mp1->msgstr_len,
                       mp2->msgstr, mp2->msgstr_len))
    return false;

  if (!pos_equal (&mp1->pos, &mp2->pos))
    return false;

  if (!string_list_equal (mp1->comment, mp2->comment))
    return false;

  if (!string_list_equal (mp1->comment_dot, mp2->comment_dot))
    return false;

  i1 = mp1->filepos_count;
  i2 = mp2->filepos_count;
  if (i1 != i2)
    return false;
  for (i = 0; i < i1; i++)
    if (!pos_equal (&mp1->filepos[i], &mp2->filepos[i]))
      return false;

  if (mp1->is_fuzzy != mp2->is_fuzzy)
    return false;

  for (i = 0; i < NFORMATS; i++)
    if (mp1->is_format[i] != mp2->is_format[i])
      return false;

  if (!(mp1->range.min == mp2->range.min && mp1->range.max == mp2->range.max))
    return false;

  if (!(mp1->prev_msgctxt != NULL
        ? mp2->prev_msgctxt != NULL
          && strcmp (mp1->prev_msgctxt, mp2->prev_msgctxt) == 0
        : mp2->prev_msgctxt == NULL))
    return false;

  if (!(mp1->prev_msgid != NULL
        ? mp2->prev_msgid != NULL
          && strcmp (mp1->prev_msgid, mp2->prev_msgid) == 0
        : mp2->prev_msgid == NULL))
    return false;

  if (!(mp1->prev_msgid_plural != NULL
        ? mp2->prev_msgid_plural != NULL
          && strcmp (mp1->prev_msgid_plural, mp2->prev_msgid_plural) == 0
        : mp2->prev_msgid_plural == NULL))
    return false;

  if (mp1->obsolete != mp2->obsolete)
    return false;

  return true;
}

bool
message_list_equal (const message_list_ty *mlp1, const message_list_ty *mlp2,
                    bool ignore_potcdate)
{
  size_t i, i1, i2;

  i1 = mlp1->nitems;
  i2 = mlp2->nitems;
  if (i1 != i2)
    return false;
  for (i = 0; i < i1; i++)
    if (!message_equal (mlp1->item[i], mlp2->item[i], ignore_potcdate))
      return false;
  return true;
}

static inline bool
msgdomain_equal (const msgdomain_ty *mdp1, const msgdomain_ty *mdp2,
                 bool ignore_potcdate)
{
  return (strcmp (mdp1->domain, mdp2->domain) == 0
          && message_list_equal (mdp1->messages, mdp2->messages,
                                 ignore_potcdate));
}

bool
msgdomain_list_equal (const msgdomain_list_ty *mdlp1,
                      const msgdomain_list_ty *mdlp2,
                      bool ignore_potcdate)
{
  size_t i, i1, i2;

  i1 = mdlp1->nitems;
  i2 = mdlp2->nitems;
  if (i1 != i2)
    return false;
  for (i = 0; i < i1; i++)
    if (!msgdomain_equal (mdlp1->item[i], mdlp2->item[i], ignore_potcdate))
      return false;
  return true;
}
