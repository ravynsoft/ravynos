/* Sequential list data type implemented by a hash table with a linked list.
   Copyright (C) 2006, 2009-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2006.

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

#ifndef _GL_LINKEDHASH_LIST_H
#define _GL_LINKEDHASH_LIST_H

#include "gl_list.h"

#ifdef __cplusplus
extern "C" {
#endif

extern DLL_VARIABLE const struct gl_list_implementation gl_linkedhash_list_implementation;
#define GL_LINKEDHASH_LIST &gl_linkedhash_list_implementation

#ifdef __cplusplus
}
#endif

#endif /* _GL_LINKEDHASH_LIST_H */
