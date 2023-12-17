/* Message translation initialization for English.
   Copyright (C) 2001-2003, 2006 Free Software Foundation, Inc.
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
#include "msgl-english.h"

#include <string.h>

#include "xalloc.h"


msgdomain_list_ty *
msgdomain_list_english (msgdomain_list_ty *mdlp)
{
  size_t j, k;

  for (k = 0; k < mdlp->nitems; k++)
    {
      message_list_ty *mlp = mdlp->item[k]->messages;

      for (j = 0; j < mlp->nitems; j++)
        {
          message_ty *mp = mlp->item[j];

          if (mp->msgid_plural == NULL)
            {
              if (mp->msgstr_len == 1 && mp->msgstr[0] == '\0')
                {
                  mp->msgstr = mp->msgid; /* no need for xstrdup */
                  mp->msgstr_len = strlen (mp->msgid) + 1;
                }
            }
          else
            {
              if (mp->msgstr_len == 2
                  && mp->msgstr[0] == '\0' && mp->msgstr[1] == '\0')
                {
                  size_t len0 = strlen (mp->msgid) + 1;
                  size_t len1 = strlen (mp->msgid_plural) + 1;
                  char *cp = XNMALLOC (len0 + len1, char);
                  memcpy (cp, mp->msgid, len0);
                  memcpy (cp + len0, mp->msgid_plural, len1);
                  mp->msgstr = cp;
                  mp->msgstr_len = len0 + len1;
                }
            }
        }
    }

  return mdlp;
}
