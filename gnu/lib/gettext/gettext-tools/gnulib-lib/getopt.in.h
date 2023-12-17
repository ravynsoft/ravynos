/* Declarations for getopt.
   Copyright (C) 1989-2023 Free Software Foundation, Inc.
   This file is part of gnulib.
   Unlike most of the getopt implementation, it is NOT shared
   with the GNU C Library, which supplies a different version of
   this file.

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

#ifndef _@GUARD_PREFIX@_GETOPT_H

#if __GNUC__ >= 3
@PRAGMA_SYSTEM_HEADER@
#endif
@PRAGMA_COLUMNS@

/* The include_next requires a split double-inclusion guard.  We must
   also inform the replacement unistd.h to not recursively use
   <getopt.h>; our definitions will be present soon enough.  */
#if @HAVE_GETOPT_H@
# define _GL_SYSTEM_GETOPT
# @INCLUDE_NEXT@ @NEXT_GETOPT_H@
# undef _GL_SYSTEM_GETOPT
#endif

#define _@GUARD_PREFIX@_GETOPT_H 1

/* Standalone applications should #define __GETOPT_PREFIX to an
   identifier that prefixes the external functions and variables
   defined in getopt-core.h and getopt-ext.h.  When this happens,
   include the headers that might declare getopt so that they will not
   cause confusion if included after this file (if the system had
   <getopt.h>, we have already included it).  */
#if defined __GETOPT_PREFIX
# if !@HAVE_GETOPT_H@
#  define __need_system_stdlib_h
#  include <stdlib.h>
#  undef __need_system_stdlib_h
#  include <stdio.h>
#  include <unistd.h>
# endif
#endif

/* The definition of _GL_ARG_NONNULL is copied here.  */

#include <getopt-cdefs.h>
#include <getopt-pfx-core.h>
#include <getopt-pfx-ext.h>

#endif /* _@GUARD_PREFIX@_GETOPT_H */
