/* Substitute for and wrapper around <stdarg.h>.
   Copyright (C) 2008-2023 Free Software Foundation, Inc.

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

#ifndef _@GUARD_PREFIX@_STDARG_H

#if __GNUC__ >= 3
@PRAGMA_SYSTEM_HEADER@
#endif
@PRAGMA_COLUMNS@

/* The include_next requires a split double-inclusion guard.  */
#@INCLUDE_NEXT@ @NEXT_STDARG_H@

#ifndef _@GUARD_PREFIX@_STDARG_H
#define _@GUARD_PREFIX@_STDARG_H

/* This file uses va_copy.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

#ifndef va_copy
# define va_copy(a,b) ((a) = (b))
#endif

#endif /* _@GUARD_PREFIX@_STDARG_H */
#endif /* _@GUARD_PREFIX@_STDARG_H */
