/* Resolving ambiguity of argument lists: Information given through
   command-line options.
   Copyright (C) 2001-2018 Free Software Foundation, Inc.

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
#include "xg-arglist-callshape.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "xalloc.h"
#include "xsize.h"


void
split_keywordspec (const char *spec,
                   const char **endp, struct callshape *shapep)
{
  const char *p;
  int argnum1 = 0;
  int argnum2 = 0;
  int argnumc = 0;
  bool argnum1_glib_context = false;
  bool argnum2_glib_context = false;
  int argtotal = 0;
  string_list_ty xcomments;

  string_list_init (&xcomments);

  /* Start parsing from the end.  */
  p = spec + strlen (spec);
  while (p > spec)
    {
      if (isdigit ((unsigned char) p[-1])
          || ((p[-1] == 'c' || p[-1] == 'g' || p[-1] == 't')
              && p - 1 > spec && isdigit ((unsigned char) p[-2])))
        {
          bool contextp = (p[-1] == 'c');
          bool glibp = (p[-1] == 'g');
          bool totalp = (p[-1] == 't');

          do
            p--;
          while (p > spec && isdigit ((unsigned char) p[-1]));

          if (p > spec && (p[-1] == ',' || p[-1] == ':'))
            {
              char *dummy;
              int arg = strtol (p, &dummy, 10);

              if (contextp)
                {
                  if (argnumc != 0)
                    /* Only one context argument can be given.  */
                    break;
                  argnumc = arg;
                }
              else if (totalp)
                {
                  if (argtotal != 0)
                    /* Only one total number of arguments can be given.  */
                    break;
                  argtotal = arg;
                }
              else
                {
                  if (argnum2 != 0)
                    /* At most two normal arguments can be given.  */
                    break;
                  argnum2 = argnum1;
                  argnum2_glib_context = argnum1_glib_context;
                  argnum1 = arg;
                  argnum1_glib_context = glibp;
                }
            }
          else
            break;
        }
      else if (p[-1] == '"')
        {
          const char *xcomment_end;

          p--;
          xcomment_end = p;

          while (p > spec && p[-1] != '"')
            p--;

          if (p > spec /* && p[-1] == '"' */)
            {
              const char *xcomment_start;

              xcomment_start = p;
              p--;
              if (p > spec && (p[-1] == ',' || p[-1] == ':'))
                {
                  size_t xcomment_len = xcomment_end - xcomment_start;
                  char *xcomment = XNMALLOC (xcomment_len + 1, char);

                  memcpy (xcomment, xcomment_start, xcomment_len);
                  xcomment[xcomment_len] = '\0';
                  string_list_append (&xcomments, xcomment);
                }
              else
                break;
            }
          else
            break;
        }
      else
        break;

      /* Here an element of the comma-separated list has been parsed.  */
      if (!(p > spec && (p[-1] == ',' || p[-1] == ':')))
        abort ();
      p--;
      if (*p == ':')
        {
          size_t i;

          if (argnum1 == 0 && argnum2 == 0)
            /* At least one non-context argument must be given.  */
            break;
          if (argnumc != 0
              && (argnum1_glib_context || argnum2_glib_context))
            /* Incompatible ways to specify the context.  */
            break;
          *endp = p;
          shapep->argnum1 = (argnum1 > 0 ? argnum1 : 1);
          shapep->argnum2 = argnum2;
          shapep->argnumc = argnumc;
          shapep->argnum1_glib_context = argnum1_glib_context;
          shapep->argnum2_glib_context = argnum2_glib_context;
          shapep->argtotal = argtotal;
          /* Reverse the order of the xcomments.  */
          string_list_init (&shapep->xcomments);
          for (i = xcomments.nitems; i > 0; )
            string_list_append (&shapep->xcomments, xcomments.item[--i]);
          string_list_destroy (&xcomments);
          return;
        }
    }

  /* Couldn't parse the desired syntax.  */
  *endp = spec + strlen (spec);
  shapep->argnum1 = 1;
  shapep->argnum2 = 0;
  shapep->argnumc = 0;
  shapep->argnum1_glib_context = false;
  shapep->argnum2_glib_context = false;
  shapep->argtotal = 0;
  string_list_init (&shapep->xcomments);
  string_list_destroy (&xcomments);
}


void
insert_keyword_callshape (hash_table *table,
                          const char *keyword, size_t keyword_len,
                          const struct callshape *shape)
{
  void *old_value;

  if (hash_find_entry (table, keyword, keyword_len, &old_value))
    {
      /* Create a one-element 'struct callshapes'.  */
      struct callshapes *shapes = XMALLOC (struct callshapes);
      shapes->nshapes = 1;
      shapes->shapes[0] = *shape;
      keyword =
        (const char *) hash_insert_entry (table, keyword, keyword_len, shapes);
      if (keyword == NULL)
        abort ();
      shapes->keyword = keyword;
      shapes->keyword_len = keyword_len;
    }
  else
    {
      /* Found a 'struct callshapes'.  See whether it already contains the
         desired shape.  */
      struct callshapes *old_shapes = (struct callshapes *) old_value;
      bool found;
      size_t i;

      found = false;
      for (i = 0; i < old_shapes->nshapes; i++)
        if (old_shapes->shapes[i].argnum1 == shape->argnum1
            && old_shapes->shapes[i].argnum2 == shape->argnum2
            && old_shapes->shapes[i].argnumc == shape->argnumc
            && old_shapes->shapes[i].argnum1_glib_context
               == shape->argnum1_glib_context
            && old_shapes->shapes[i].argnum2_glib_context
               == shape->argnum2_glib_context
            && old_shapes->shapes[i].argtotal == shape->argtotal)
          {
            old_shapes->shapes[i].xcomments = shape->xcomments;
            found = true;
            break;
          }

      if (!found)
        {
          /* Replace the existing 'struct callshapes' with a new one.  */
          struct callshapes *shapes =
            (struct callshapes *)
            xmalloc (xsum (sizeof (struct callshapes),
                           xtimes (old_shapes->nshapes,
                                   sizeof (struct callshape))));

          shapes->keyword = old_shapes->keyword;
          shapes->keyword_len = old_shapes->keyword_len;
          shapes->nshapes = old_shapes->nshapes + 1;
          for (i = 0; i < old_shapes->nshapes; i++)
            shapes->shapes[i] = old_shapes->shapes[i];
          shapes->shapes[i] = *shape;
          if (hash_set_value (table, keyword, keyword_len, shapes))
            abort ();
          free (old_shapes);
        }
    }
}
