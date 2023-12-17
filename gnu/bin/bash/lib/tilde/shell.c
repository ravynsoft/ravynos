/* shell.c -- tilde utility functions that are normally provided by
	      bash when readline is linked as part of the shell. */

/* Copyright (C) 1998-2017 Free Software Foundation, Inc.

   This file is part of the GNU Tilde Library.

   The GNU Tilde Library is free software: you can redistribute it and/or
   modify it under the terms of the GNU General Public License as published
   by the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   The GNU Tilde Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the GNU Tilde Library.  If not, see <http://www.gnu.org/licenses/>.
*/

#if defined (HAVE_CONFIG_H)
#  include <config.h>
#endif

#if defined (HAVE_UNISTD_H)
#  ifdef _MINIX
#    include <sys/types.h>
#  endif
#  include <unistd.h>
#endif /* HAVE_UNISTD_H */

#if defined (HAVE_STDLIB_H)
#  include <stdlib.h>
#else
#  include "ansi_stdlib.h"
#endif /* HAVE_STDLIB_H */

#if defined (HAVE_STRING_H)
#  include <string.h>
#else
#  include <strings.h>
#endif /* !HAVE_STRING_H */

#include <pwd.h>

#if !defined (HAVE_GETPW_DECLS)
extern struct passwd *getpwuid ();
#endif /* !HAVE_GETPW_DECLS */

char *
get_env_value (char *varname)
{
  return ((char *)getenv (varname));
}

/* If we're not using $HOME, assume that the passwd file information won't
   change while this shell instance is running. */
char *
get_home_dir (void)
{
  static char *home_dir = (char *)NULL;
  struct passwd *entry;

  if (home_dir)
    return (home_dir);

#if defined (HAVE_GETPWUID)
  entry = getpwuid (getuid ());
  if (entry)
    home_dir = savestring (entry->pw_dir);
#endif

#if defined (HAVE_GETPWENT)
  endpwent ();		/* some systems need this */
#endif

  return (home_dir);
}
