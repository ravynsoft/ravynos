/* Use of program name in error-reporting functions.
   Copyright (C) 2001-2003, 2006, 2019, 2023 Free Software Foundation, Inc.
   Written by Bruno Haible <haible@clisp.cons.org>, 2001.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */


#include <config.h>

/* Specification.  */
#include "error-progname.h"

#include <stdio.h>
#include <stdlib.h>

#if IN_LIBGETTEXTPO
# define program_name getprogname ()
#else
# include "progname.h"
#endif


/* Indicates whether errors and warnings get prefixed with program_name.
   Default is true.  */
bool error_with_progname = true;

/* Print program_name prefix on stderr if and only if error_with_progname
   is true.  */
void
maybe_print_progname ()
{
  if (error_with_progname)
    fprintf (stderr, "%s: ", program_name);
}
