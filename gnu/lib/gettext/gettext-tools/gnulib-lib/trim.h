/* Removes leading and/or trailing whitespaces
   Copyright (C) 2006, 2009-2023 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Davide Angelocola <davide.angelocola@gmail.com> */

/* This file uses _GL_ATTRIBUTE_MALLOC, _GL_ATTRIBUTE_RETURNS_NONNULL.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

#include <stdlib.h>

/* Trim mode. */
#define TRIM_TRAILING 0
#define TRIM_LEADING 1
#define TRIM_BOTH 2

/* Removes trailing and leading whitespaces. */
#define trim(s) trim2(s, TRIM_BOTH)

/* Removes trailing whitespaces. */
#define trim_trailing(s) trim2(s, TRIM_TRAILING)

/* Removes leading whitespaces. */
#define trim_leading(s) trim2(s, TRIM_LEADING)

char *trim2 (const char *, int)
  _GL_ATTRIBUTE_MALLOC _GL_ATTRIBUTE_DEALLOC_FREE
  _GL_ATTRIBUTE_RETURNS_NONNULL;
