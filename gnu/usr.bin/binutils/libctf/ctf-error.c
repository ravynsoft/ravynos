/* Error table.
   Copyright (C) 2019-2023 Free Software Foundation, Inc.

   This file is part of libctf.

   libctf is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 3, or (at your option) any later
   version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not see
   <http://www.gnu.org/licenses/>.  */

#include <ctf-impl.h>
#include <stddef.h>
#include <string.h>

/* This construct is due to Bruno Haible: much thanks.  */

/* Give each structure member a unique name.  The name does not matter, so we
   use the enum constant to uniquify them.  */

#define ERRSTRFIELD(N) ctf_errstr##N

/* In this file, we want to treat the first item of the ctf error
   macro like subsequent items.  */
#define _CTF_FIRST(NAME, VALUE) _CTF_ITEM(NAME, VALUE)

/* The error message strings, each in a unique structure member precisely big
   enough for that error, plus a str member to access them all as a string
   table.  */

static const union _ctf_errlist_t
{
  __extension__ struct
  {
#define _CTF_ITEM(n, s) char ERRSTRFIELD (n) [sizeof (s)];
_CTF_ERRORS
#undef _CTF_ITEM
  };
  char str[1];
} _ctf_errlist =
  {
   {
#define _CTF_ITEM(n, s) N_(s),
_CTF_ERRORS
#undef _CTF_ITEM
   }
  };

/* Offsets to each member in the string table, computed using offsetof.  */

static const unsigned int _ctf_erridx[] =
  {
#define _CTF_ITEM(n, s) [n - ECTF_BASE] = offsetof (union _ctf_errlist_t, ERRSTRFIELD (n)),
_CTF_ERRORS
#undef _CTF_ITEM
  };

const char *
ctf_errmsg (int error)
{
  const char *str;

  if (error >= ECTF_BASE && (error - ECTF_BASE) < ECTF_NERR)
    str = _ctf_errlist.str + _ctf_erridx[error - ECTF_BASE];
  else
    str = (const char *) strerror (error);

  return (str ? gettext (str) : _("Unknown error"));
}

int
ctf_errno (ctf_dict_t * fp)
{
  return fp->ctf_errno;
}
