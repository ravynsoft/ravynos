/* Query, remove, or restore a Solaris privilege.

   Copyright (C) 2009-2023 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.

   Written by David Bartley.  */

#include <config.h>

#define PRIV_SET_INLINE _GL_EXTERN_INLINE

#include "priv-set.h"

#if HAVE_GETPPRIV && HAVE_PRIV_H

# include <errno.h>
# include <priv.h>

/* Holds a (cached) copy of the effective set.  */
static priv_set_t *eff_set;

/* Holds a set of privileges that we have removed.  */
static priv_set_t *rem_set;

static bool initialized;

static int
priv_set_initialize (void)
{
  if (! initialized)
    {
      eff_set = priv_allocset ();
      if (!eff_set)
        {
          return -1;
        }
      rem_set = priv_allocset ();
      if (!rem_set)
        {
          priv_freeset (eff_set);
          return -1;
        }
      if (getppriv (PRIV_EFFECTIVE, eff_set) != 0)
        {
          priv_freeset (eff_set);
          priv_freeset (rem_set);
          return -1;
        }
      priv_emptyset (rem_set);
      initialized = true;
    }

  return 0;
}


/* Check if priv is in the effective set.
   Returns 1 if priv is a member and 0 if not.
   Returns -1 on error with errno set appropriately.  */
int
priv_set_ismember (const char *priv)
{
  if (! initialized && priv_set_initialize () != 0)
    return -1;

  return priv_ismember (eff_set, priv);
}


/* Try to remove priv from the effective set.
   Returns 0 if priv was removed.
   Returns -1 on error with errno set appropriately.  */
int
priv_set_remove (const char *priv)
{
  if (! initialized && priv_set_initialize () != 0)
    return -1;

  if (priv_ismember (eff_set, priv))
    {
      /* priv_addset/priv_delset can only fail if priv is invalid, which is
         checked above by the priv_ismember call.  */
      priv_delset (eff_set, priv);
      if (setppriv (PRIV_SET, PRIV_EFFECTIVE, eff_set) != 0)
        {
          priv_addset (eff_set, priv);
          return -1;
        }
      priv_addset (rem_set, priv);
    }
  else
    {
      errno = EINVAL;
      return -1;
    }

  return 0;
}


/* Try to restore priv to the effective set.
   Returns 0 if priv was re-added to the effective set (after being previously
   removed by a call to priv_set_remove).
   Returns -1 on error with errno set appropriately.  */
int
priv_set_restore (const char *priv)
{
  if (! initialized && priv_set_initialize () != 0)
    return -1;

  if (priv_ismember (rem_set, priv))
    {
      /* priv_addset/priv_delset can only fail if priv is invalid, which is
         checked above by the priv_ismember call.  */
      priv_addset (eff_set, priv);
      if (setppriv (PRIV_SET, PRIV_EFFECTIVE, eff_set) != 0)
        {
          priv_delset (eff_set, priv);
          return -1;
        }
      priv_delset (rem_set, priv);
    }
  else
    {
      errno = EINVAL;
      return -1;
    }

  return 0;
}

#endif
