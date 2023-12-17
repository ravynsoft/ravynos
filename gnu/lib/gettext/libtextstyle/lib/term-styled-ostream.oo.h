/* Output stream for CSS styled text, producing ANSI escape sequences.
   Copyright (C) 2006, 2019-2020 Free Software Foundation, Inc.
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

#ifndef _TERM_STYLED_OSTREAM_H
#define _TERM_STYLED_OSTREAM_H

#include <stdbool.h>

#include "styled-ostream.h"
#include "term-ostream.h"


struct term_styled_ostream : struct styled_ostream
{
methods:
  /* Accessors.  */
  term_ostream_t get_destination (term_styled_ostream_t stream);
  const char *   get_css_filename (term_styled_ostream_t stream);
};


#ifdef __cplusplus
extern "C" {
#endif


/* Create an output stream referring to the file descriptor FD, styled with
   the file CSS_FILENAME.
   FILENAME is used only for error messages.
   TTY_CONTROL specifies the amount of control to take over the underlying tty.
   Note that the resulting stream must be closed before FD can be closed.
   Return NULL upon failure.  */
extern term_styled_ostream_t
       term_styled_ostream_create (int fd, const char *filename,
                                   ttyctl_t tty_control,
                                   const char *css_filename);


/* Test whether a given output stream is a term_styled_ostream.  */
extern bool is_instance_of_term_styled_ostream (ostream_t stream);


#ifdef __cplusplus
}
#endif

#endif /* _TERM_STYLED_OSTREAM_H */
