/* Formatted output to strings.
   Copyright (C) 2003-2023 Free Software Foundation, Inc.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* This file can be parametrized with the following macros:
     STATIC             Set to 'static' to declare the function static.  */

#ifndef STATIC
# include <config.h>
#endif

/* Specification.  */
#include "wprintf-parse.h"

#define PRINTF_PARSE wprintf_parse
#define CHAR_T wchar_t
#define DIRECTIVE wchar_t_directive
#define DIRECTIVES wchar_t_directives
#include "printf-parse.c"
