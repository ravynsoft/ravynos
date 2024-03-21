/* Fake setlocale - platform independent, for testing purposes.
   Copyright (C) 2001-2002, 2019-2020 Free Software Foundation, Inc.

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

#include <stdlib.h>
#include <locale.h>
#include <string.h>

/* Make this override available independently of possible overrides in
   libgnuintl.h or locale.h.
   Note: On platforms where _nl_locale_name_posix invokes setlocale_null, this
   override *must* be called 'setlocale'.  */
#undef setlocale
/* Avoid a link error on MSVC.  */
#if defined _WIN32 && !defined __CYGWIN__
# define setlocale fake_setlocale
#endif

/* Return string representation of locale CATEGORY.  */
static const char *
category_to_name (int category)
{
  const char *retval;

  switch (category)
  {
#ifdef LC_COLLATE
  case LC_COLLATE:
    retval = "LC_COLLATE";
    break;
#endif
#ifdef LC_CTYPE
  case LC_CTYPE:
    retval = "LC_CTYPE";
    break;
#endif
#ifdef LC_MONETARY
  case LC_MONETARY:
    retval = "LC_MONETARY";
    break;
#endif
#ifdef LC_NUMERIC
  case LC_NUMERIC:
    retval = "LC_NUMERIC";
    break;
#endif
#ifdef LC_TIME
  case LC_TIME:
    retval = "LC_TIME";
    break;
#endif
#ifdef LC_MESSAGES
  case LC_MESSAGES:
    retval = "LC_MESSAGES";
    break;
#endif
#ifdef LC_RESPONSE
  case LC_RESPONSE:
    retval = "LC_RESPONSE";
    break;
#endif
#ifdef LC_ALL
  case LC_ALL:
    /* This might not make sense but is perhaps better than any other
       value.  */
    retval = "LC_ALL";
    break;
#endif
  default:
    /* If you have a better idea for a default value let me know.  */
    retval = "LC_XXX";
  }

  return retval;
}

/* An implementation of setlocale that always succeeds, but doesn't
   actually change the behaviour of locale dependent functions.
   Assumes setenv()/putenv() is not called.  */
char *
setlocale (int category, const char *locale)
{
  static char C_string[] = "C";
  static char *current_locale = C_string;
  struct list
  {
    int category;
    char *current_locale;
    struct list *next;
  };
  static struct list *facets = NULL;
  struct list *facetp;
  char *retval;

  if (locale != NULL)
    {
      char *copy;

      copy = (char *) malloc (strlen (locale) + 1);
      strcpy (copy, locale);

      if (category == LC_ALL)
        {
          while ((facetp = facets) != NULL)
            {
              facets = facetp->next;
              free (facetp->current_locale);
              free (facetp);
            }
          if (current_locale != C_string)
            free (current_locale);
          current_locale = copy;
        }
      else
        {
          for (facetp = facets; facetp != NULL; facetp = facetp->next)
            if (category == facetp->category)
              {
                free (facetp->current_locale);
                facetp->current_locale = copy;
                break;
              }
          if (facetp == NULL)
            {
              facetp = (struct list *) malloc (sizeof (struct list));
              facetp->category = category;
              facetp->current_locale = copy;
              facetp->next = facets;
              facets = facetp;
            }
        }
    }

  retval = current_locale;
  for (facetp = facets; facetp != NULL; facetp = facetp->next)
    if (category == facetp->category)
      {
        retval = facetp->current_locale;
        break;
      }

  if (retval[0] == '\0')
    {
      retval = getenv ("LC_ALL");
      if (retval == NULL || retval[0] == '\0')
        {
          retval = getenv (category_to_name (category));
          if (retval == NULL || retval[0] == '\0')
            {
              retval = getenv ("LANG");
              if (retval == NULL || retval[0] == '\0')
                retval = "C";
            }
        }
    }
  return retval;
}
