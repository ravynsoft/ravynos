/* Source file positions.
   Copyright (C) 1995-1998, 2000-2001, 2021, 2023 Free Software Foundation, Inc.

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

#ifndef _POS_H
#define _POS_H

/* Get size_t.  */
#include <stddef.h>

/* Get bool.  */
#include <stdbool.h>

/* Position of a message within a source file.
   Used for error reporting purposes.  */
typedef struct lex_pos_ty lex_pos_ty;
struct lex_pos_ty
{
  const char *file_name;
  size_t line_number;
};

/* Determines whether the file name in the position has spaces.
   Such spaces need special protection in PO files and .properties files.  */
extern bool pos_filename_has_spaces (const struct lex_pos_ty *pos);

#endif /* _POS_H */
