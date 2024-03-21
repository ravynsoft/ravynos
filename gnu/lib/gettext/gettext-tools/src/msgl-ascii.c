/* Message list test for ASCII character set.
   Copyright (C) 2001-2002, 2005-2006, 2023 Free Software Foundation, Inc.
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
#include "msgl-ascii.h"

#include "c-ctype.h"


/* This file's structure parallels msgl-iconv.c.  */


bool
is_ascii_string (const char *string)
{
  for (; *string; string++)
    if (!c_isascii ((unsigned char) *string))
      return false;
  return true;
}

bool
is_ascii_string_desc (string_desc_t string)
{
  size_t len = string_desc_length (string);
  size_t i;
  for (i = 0; i < len; i++)
    if (!c_isascii ((unsigned char) string_desc_char_at (string, i)))
      return false;
  return true;
}

bool
is_ascii_string_list (string_list_ty *slp)
{
  size_t i;

  if (slp != NULL)
    for (i = 0; i < slp->nitems; i++)
      if (!is_ascii_string (slp->item[i]))
        return false;
  return true;
}

bool
is_ascii_message (message_ty *mp)
{
  const char *p = mp->msgstr;
  const char *p_end = p + mp->msgstr_len;

  for (; p < p_end; p++)
    if (!c_isascii ((unsigned char) *p))
      return false;

  if (!is_ascii_string_list (mp->comment))
    return false;
  if (!is_ascii_string_list (mp->comment_dot))
    return false;

  /* msgid and msgid_plural are normally ASCII, so why checking?
     Because in complete UTF-8 environments they can be UTF-8, not ASCII.  */
  if (!is_ascii_string (mp->msgid))
    return false;
  if (mp->msgid_plural != NULL && !is_ascii_string (mp->msgid_plural))
    return false;

  /* Likewise for msgctxt.  */
  if (mp->msgctxt != NULL && !is_ascii_string (mp->msgctxt))
    return false;

  /* Likewise for the prev_* fields.  */
  if (mp->prev_msgctxt != NULL && !is_ascii_string (mp->prev_msgctxt))
    return false;
  if (mp->prev_msgid != NULL && !is_ascii_string (mp->prev_msgid))
    return false;
  if (mp->prev_msgid_plural != NULL && !is_ascii_string (mp->prev_msgid_plural))
    return false;

  return true;
}

bool
is_ascii_message_list (message_list_ty *mlp)
{
  size_t j;

  for (j = 0; j < mlp->nitems; j++)
    if (!is_ascii_message (mlp->item[j]))
      return false;

  return true;
}

bool
is_ascii_msgdomain_list (msgdomain_list_ty *mdlp)
{
  size_t k;

  for (k = 0; k < mdlp->nitems; k++)
    if (!is_ascii_message_list (mdlp->item[k]->messages))
      return false;

  return true;
}
