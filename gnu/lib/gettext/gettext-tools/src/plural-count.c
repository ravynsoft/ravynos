/* Plural form count.
   Copyright (C) 2003, 2007 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2003.

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
#include "plural-count.h"

#include "plural-exp.h"

/* Extract the number of plural forms from a header entry.  */

unsigned long int
get_plural_count (const char *header)
{
  const struct expression *plural;
  unsigned long int nplurals;

  extract_plural_expression (header, &plural, &nplurals);

  return nplurals;
}
