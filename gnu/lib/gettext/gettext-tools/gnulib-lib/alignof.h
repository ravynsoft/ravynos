/* Determine alignment of types.
   Copyright (C) 2003-2004, 2006, 2009-2023 Free Software Foundation, Inc.

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

#ifndef _ALIGNOF_H
#define _ALIGNOF_H

/* This file uses alignof.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

#include <stddef.h>

/* alignof_slot (TYPE)
   Determine the alignment of a structure slot (field) of a given type,
   at compile time.  Note that the result depends on the ABI.
   This is the same as alignof (TYPE).
   Note: The result cannot be used as a value for an 'enum' constant,
   due to bugs in HP-UX 10.20 cc and AIX 3.2.5 xlc.  */
#if defined __cplusplus
  template <class type> struct alignof_helper { char __slot1; type __slot2; };
# define alignof_slot(type) offsetof (alignof_helper<type>, __slot2)
#else
# define alignof_slot(type) alignof (type)
#endif

/* alignof_type (TYPE)
   Determine the good alignment of an object of the given type at compile time.
   Note that this is not necessarily the same as alignof_slot(type).
   For example, with GNU C on x86 platforms and with clang on Linux/x86:
   alignof_type(long long) = 8, but alignof_slot(long long) = 4.
   And alignof_type(double) = 8, but
   - when -malign-double is not specified:  alignof_slot(double) = 4,
   - when -malign-double is specified:      alignof_slot(double) = 8.
   Note: The result cannot be used as a value for an 'enum' constant,
   due to bugs in HP-UX 10.20 cc and AIX 3.2.5 xlc.  */
#if defined __GNUC__ || defined __clang__ || defined __IBM__ALIGNOF__
# define alignof_type __alignof__
#else
# define alignof_type alignof_slot
#endif

#endif /* _ALIGNOF_H */
