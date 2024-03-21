/* sframe-error.c - Error messages.

   Copyright (C) 2022-2023 Free Software Foundation, Inc.

   This file is part of libsframe.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include "sframe-api.h"
#include <stddef.h>
#include <string.h>

/* In this file, we want to treat the first item of the SFrame error
   macro like subsequent items.  */
#define _SFRAME_FIRST(NAME, VALUE) _SFRAME_ITEM(NAME, VALUE)

/* The error message strings, each in a unique structure member precisely big
   enough for that error, plus a str member to access them all as a string
   table.  */

static const char *const _sframe_errlist[] = {
#define _SFRAME_ITEM(n, s) s,
_SFRAME_ERRORS
#undef _SFRAME_ITEM
};

const char *
sframe_errmsg (int error)
{
  const char *str;

  if (error >= SFRAME_ERR_BASE && (error - SFRAME_ERR_BASE) < SFRAME_ERR_NERR)
    str = _sframe_errlist[error - SFRAME_ERR_BASE];
  else
    str = (const char *) strerror (error);

  return (str ? str : "Unknown error");
}
