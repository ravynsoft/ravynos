/* Case-insensitive searching in a string in C locale.
   Copyright (C) 2005, 2009-2023 Free Software Foundation, Inc.

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

#ifndef C_STRCASESTR_H
#define C_STRCASESTR_H


#ifdef __cplusplus
extern "C" {
#endif


/* Find the first occurrence of NEEDLE in HAYSTACK, using case-insensitive
   comparison.  */
extern char *c_strcasestr (const char *haystack, const char *needle);


#ifdef __cplusplus
}
#endif


#endif /* C_STRCASESTR_H */
