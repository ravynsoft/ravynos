/* Copyright (C) 1995-2016, 2020 Free Software Foundation, Inc.
   Contributed by Ulrich Drepper <drepper@gnu.ai.mit.edu>, 1995.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation; either version 2.1 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "loadinfo.h"

/* On some strange systems still no definition of NULL is found.  Sigh!  */
#ifndef NULL
# if defined __STDC__ && __STDC__
#  define NULL ((void *) 0)
# else
#  define NULL 0
# endif
#endif

/* @@ end of prolog @@ */


int
_nl_explode_name (char *name,
		  const char **language, const char **modifier,
		  const char **territory, const char **codeset,
		  const char **normalized_codeset)
{
  char *cp;
  int mask;

  *modifier = NULL;
  *territory = NULL;
  *codeset = NULL;
  *normalized_codeset = NULL;

  /* Determine the individual parts of the locale name.
     Accept the XPG syntax

             language[_territory][.codeset][@modifier]

     On AIX systems, also accept the same syntax with an uppercased language,
     and a syntax similar to RFC 5646:

             language[_script]_territory[.codeset]

     where script is a four-letter code for a script, per ISO 15924.
   */

  mask = 0;

  /* First look for the language.  Termination symbols are `_', '.', and `@'.  */
  *language = name;

  cp = name;
  while (cp[0] != '\0' && cp[0] != '_' && cp[0] != '@' && cp[0] != '.')
    ++cp;

  if (cp == name)
    /* This does not make sense: language has to be specified.  Use
       this entry as it is without exploding.  Perhaps it is an alias.  */
    cp = strchr (name, '\0');
  else
    {
      if (cp[0] == '_')
	{
	  *cp++ = '\0';
#if defined _AIX
	  /* Lowercase the language.  */
	  {
	    char *lcp;

	    for (lcp = name; lcp < cp; lcp++)
	      if (*lcp >= 'A' && *lcp <= 'Z')
		*lcp += 'a' - 'A';
	  }

	  /* Next is the script or the territory.  It depends on whether
	     there is another '_'.  */
	  char *next = cp;

	  while (cp[0] != '\0' && cp[0] != '_' && cp[0] != '@' && cp[0] != '.')
	    ++cp;

	  if (cp[0] == '_')
	    {
	      *cp++ = '\0';

	      /* Next is the script.  Translate the script to a modifier.
		 We don't need to support all of ISO 15924 here, only those
		 scripts that actually occur:
		   Latn -> latin
		   Cyrl -> cyrillic
		   Guru -> gurmukhi
		   Hans -> (omitted, redundant with the territory CN or SG)
		   Hant -> (omitted, redundant with the territory TW or HK)  */
	      if (strcmp (next, "Latn") == 0)
		*modifier = "latin";
	      else if (strcmp (next, "Cyrl") == 0)
		*modifier = "cyrillic";
	      else if (strcmp (next, "Guru") == 0)
		*modifier = "gurmukhi";
	      else if (!(strcmp (next, "Hans") == 0
			 || strcmp (next, "Hant") == 0))
		*modifier = next;
	      if (*modifier != NULL && (*modifier)[0] != '\0')
		mask |= XPG_MODIFIER;
	    }
	  else
	    cp = next;
#endif

	  /* Next is the territory.  */
	  *territory = cp;

	  while (cp[0] != '\0' && cp[0] != '.' && cp[0] != '@')
	    ++cp;

	  mask |= XPG_TERRITORY;
	}

      if (cp[0] == '.')
	{
	  /* Next is the codeset.  */
	  *cp++ = '\0';
	  *codeset = cp;

	  while (cp[0] != '\0' && cp[0] != '@')
	    ++cp;

	  mask |= XPG_CODESET;

	  if (*codeset != cp && (*codeset)[0] != '\0')
	    {
	      *normalized_codeset = _nl_normalize_codeset (*codeset,
							   cp - *codeset);
	      if (*normalized_codeset == NULL)
		return -1;
	      else if (strcmp (*codeset, *normalized_codeset) == 0)
		free ((char *) *normalized_codeset);
	      else
		mask |= XPG_NORM_CODESET;
	    }
	}

      if (cp[0] == '@')
	{
	  /* Next is the modifier.  */
	  *cp++ = '\0';
	  *modifier = cp;

	  if (cp[0] != '\0')
	    mask |= XPG_MODIFIER;
	}
    }

  if (*territory != NULL && (*territory)[0] == '\0')
    mask &= ~XPG_TERRITORY;

  if (*codeset != NULL && (*codeset)[0] == '\0')
    mask &= ~XPG_CODESET;

  return mask;
}
