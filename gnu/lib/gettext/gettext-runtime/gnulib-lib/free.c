/* Make free() preserve errno.

   Copyright (C) 2003, 2006, 2009-2023 Free Software Foundation, Inc.

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

/* written by Paul Eggert */

#include <config.h>

/* Specification.  */
#include <stdlib.h>

/* A function definition is only needed if HAVE_FREE_POSIX is not defined.  */
#if !HAVE_FREE_POSIX

# include <errno.h>

void
rpl_free (void *p)
# undef free
{
# if defined __GNUC__ && !defined __clang__
  /* An invalid GCC optimization
     <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=98396>
     would optimize away the assignments in the code below, when link-time
     optimization (LTO) is enabled.  Make the code more complicated, so that
     GCC does not grok how to optimize it.  */
  int err[2];
  err[0] = errno;
  err[1] = errno;
  errno = 0;
  free (p);
  errno = err[errno == 0];
# else
  int err = errno;
  free (p);
  errno = err;
# endif
}

#endif
