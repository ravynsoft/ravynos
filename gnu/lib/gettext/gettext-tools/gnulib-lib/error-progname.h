/* Use of program name in error-reporting functions.
   Copyright (C) 2001-2003 Free Software Foundation, Inc.
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

#ifndef _ERROR_PROGNAME_H
#define _ERROR_PROGNAME_H

#include <stdbool.h>

/* This file supports selectively prefixing or nor prefixing error messages
   with the program name.

   Programs using this file should do the following in main():
     error_print_progname = maybe_print_progname;
 */


#ifdef __cplusplus
extern "C" {
#endif


/* Indicates whether errors and warnings get prefixed with program_name.
   Default is true.
   A reason to omit the prefix is for better interoperability with Emacs'
   compile.el.  */
extern DLL_VARIABLE bool error_with_progname;

/* Print program_name prefix on stderr if and only if error_with_progname
   is true.  */
extern void maybe_print_progname (void);


#ifdef __cplusplus
}
#endif


#endif /* _ERROR_PROGNAME_H */
