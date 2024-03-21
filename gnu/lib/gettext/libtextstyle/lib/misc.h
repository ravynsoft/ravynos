/* Miscellaneous public API.
   Copyright (C) 2019 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2019.

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

#ifndef _MISC_H
#define _MISC_H

#include "styled-ostream.h"
#include "term-ostream.h"

#ifdef __cplusplus
extern "C" {
#endif


/* Create an output stream referring to the file descriptor FD, styled with
   the file CSS_FILENAME if possible.
   FILENAME is used only for error messages.
   TTY_CONTROL specifies the amount of control to take over the underlying tty.
   Note that the resulting stream must be closed before FD can be closed.  */
extern styled_ostream_t
       styled_ostream_create (int fd, const char *filename,
                              ttyctl_t tty_control,
                              const char *css_filename);

/* Set the exit value upon failure within libtextstyle.  */
extern void libtextstyle_set_failure_exit_code (int exit_code);


#ifdef __cplusplus
}
#endif

#endif /* _MISC_H */
