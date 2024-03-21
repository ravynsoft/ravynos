/* Table of known plural form expressions.
   Copyright (C) 2001-2003 Free Software Foundation, Inc.
   Written by Bruno Haible <haible@clisp.cons.org>, 2002.

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

#ifndef _PLURAL_TABLE_H
#define _PLURAL_TABLE_H

#include <stddef.h>

struct plural_table_entry
{
  const char *lang;
  const char *language;
  const char *value;
};

extern DLL_VARIABLE struct plural_table_entry plural_table[];
extern DLL_VARIABLE const size_t plural_table_size;

#endif /* _PLURAL_TABLE_H */
