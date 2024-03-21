/* Substitute for <sys/random.h>.
   Copyright (C) 2020-2023 Free Software Foundation, Inc.

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

# if __GNUC__ >= 3
@PRAGMA_SYSTEM_HEADER@
# endif
@PRAGMA_COLUMNS@

#ifndef _@GUARD_PREFIX@_SYS_RANDOM_H

#if @HAVE_SYS_RANDOM_H@

/* On uClibc < 1.0.35, <sys/random.h> assumes prior inclusion of <stddef.h>.
   Do not use __UCLIBC__ here, as it might not be defined yet.
   But avoid namespace pollution on glibc systems.  */
# ifndef __GLIBC__
#  include <stddef.h>
# endif
/* On Mac OS X 10.5, <sys/random.h> assumes prior inclusion of <sys/types.h>.
   On Max OS X 10.13, <sys/random.h> assumes prior inclusion of a file that
   includes <Availability.h>, such as <stdlib.h> or <unistd.h>.  */
# if defined __APPLE__ && defined __MACH__                  /* Mac OS X */
#  include <sys/types.h>
#  include <stdlib.h>
# endif

/* The include_next requires a split double-inclusion guard.  */
# @INCLUDE_NEXT@ @NEXT_SYS_RANDOM_H@

#endif

#ifndef _@GUARD_PREFIX@_SYS_RANDOM_H
#define _@GUARD_PREFIX@_SYS_RANDOM_H

/* This file uses GNULIB_POSIXCHECK, HAVE_RAW_DECL_*.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

#include <sys/types.h>

/* Define the GRND_* constants.  */
#ifndef GRND_NONBLOCK
# define GRND_NONBLOCK 1
# define GRND_RANDOM 2
#endif

/* The definitions of _GL_FUNCDECL_RPL etc. are copied here.  */

/* The definition of _GL_ARG_NONNULL is copied here.  */

/* The definition of _GL_WARN_ON_USE is copied here.  */


/* Declare overridden functions.  */


#if @GNULIB_GETRANDOM@
/* Fill a buffer with random bytes.  */
# if @REPLACE_GETRANDOM@
#  if !(defined __cplusplus && defined GNULIB_NAMESPACE)
#   undef getrandom
#   define getrandom rpl_getrandom
#  endif
_GL_FUNCDECL_RPL (getrandom, ssize_t,
                  (void *buffer, size_t length, unsigned int flags)
                  _GL_ARG_NONNULL ((1)));
_GL_CXXALIAS_RPL (getrandom, ssize_t,
                  (void *buffer, size_t length, unsigned int flags));
# else
#  if !@HAVE_GETRANDOM@
_GL_FUNCDECL_SYS (getrandom, ssize_t,
                  (void *buffer, size_t length, unsigned int flags)
                  _GL_ARG_NONNULL ((1)));
#  endif
_GL_CXXALIAS_SYS (getrandom, ssize_t,
                  (void *buffer, size_t length, unsigned int flags));
# endif
# if __GLIBC__ + (__GLIBC_MINOR__ >= 25) > 2
_GL_CXXALIASWARN (getrandom);
# endif
#elif defined GNULIB_POSIXCHECK
# undef getrandom
# if HAVE_RAW_DECL_GETRANDOM
_GL_WARN_ON_USE (getrandom, "getrandom is unportable - "
                 "use gnulib module getrandom for portability");
# endif
#endif


#endif /* _@GUARD_PREFIX@_SYS_RANDOM_H */
#endif /* _@GUARD_PREFIX@_SYS_RANDOM_H */
