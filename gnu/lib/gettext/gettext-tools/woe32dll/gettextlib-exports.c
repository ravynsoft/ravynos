/* List of exported symbols of libgettextlib on Cygwin and native Windows.
   Copyright (C) 2006-2007, 2010, 2012, 2019, 2023 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2006.

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

#include "woe32dll/export.h"

VARIABLE(argmatch_die)
#if GNULIB_DEFINED_ERROR
# if GNULIB_REPLACE_ERROR
VARIABLE(rpl_error_message_count)
VARIABLE(rpl_error_one_per_line)
VARIABLE(rpl_error_print_progname)
# else
VARIABLE(error_message_count)
VARIABLE(error_one_per_line)
VARIABLE(error_print_progname)
# endif
#endif
VARIABLE(error_with_progname)
VARIABLE(exit_failure)
VARIABLE(gl_linkedhash_list_implementation)
VARIABLE(program_name)
#if GNULIB_DEFINED_GETOPT
VARIABLE(rpl_optarg)
VARIABLE(rpl_optind)
#endif
VARIABLE(simple_backup_suffix)
