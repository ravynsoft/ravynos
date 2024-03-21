/* size_max.h -- declare SIZE_MAX through system headers
   Copyright (C) 2005-2006, 2009-2023 Free Software Foundation, Inc.
   Written by Simon Josefsson.

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

#ifndef GNULIB_SIZE_MAX_H
#define GNULIB_SIZE_MAX_H

/* This file uses HAVE_STDINT_H.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

/* Get SIZE_MAX declaration on systems like Solaris 7/8/9.  */
# include <limits.h>
/* Get SIZE_MAX declaration on systems like glibc 2.  */
# if HAVE_STDINT_H
#  include <stdint.h>
# endif
/* On systems where these include files don't define it, SIZE_MAX is defined
   in config.h.  */

#endif /* GNULIB_SIZE_MAX_H */
