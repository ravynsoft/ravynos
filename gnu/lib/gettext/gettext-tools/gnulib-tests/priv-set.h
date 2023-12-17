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

/* This file uses _GL_INLINE_HEADER_BEGIN, _GL_INLINE, HAVE_PRIV_H,
   HAVE_GETPPRIV.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

_GL_INLINE_HEADER_BEGIN
#ifndef PRIV_SET_INLINE
# define PRIV_SET_INLINE _GL_INLINE
#endif

#if HAVE_GETPPRIV && HAVE_PRIV_H

# include <priv.h>

int priv_set_ismember (const char *priv);
int priv_set_remove (const char *priv);
int priv_set_restore (const char *priv);

PRIV_SET_INLINE int
priv_set_remove_linkdir (void)
{
  return priv_set_remove (PRIV_SYS_LINKDIR);
}

PRIV_SET_INLINE int
priv_set_restore_linkdir (void)
{
  return priv_set_restore (PRIV_SYS_LINKDIR);
}

#else

PRIV_SET_INLINE int
priv_set_remove_linkdir (void)
{
  return -1;
}

PRIV_SET_INLINE int
priv_set_restore_linkdir (void)
{
  return -1;
}

#endif

_GL_INLINE_HEADER_END
