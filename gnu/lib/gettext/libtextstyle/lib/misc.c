/* Miscellaneous public API.
   Copyright (C) 2019, 2021 Free Software Foundation, Inc.
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

#include <config.h>

/* Specification.  */
#include "misc.h"

#include "term-styled-ostream.h"
#include "noop-styled-ostream.h"
#include "fd-ostream.h"
#include "exitfail.h"


styled_ostream_t
styled_ostream_create (int fd, const char *filename, ttyctl_t tty_control,
                       const char *css_filename)
{
  styled_ostream_t stream;

  stream = term_styled_ostream_create (fd, filename, tty_control, css_filename);
  if (stream == NULL)
    stream =
      noop_styled_ostream_create (fd_ostream_create (fd, filename, true), true);

  return stream;
}


void
libtextstyle_set_failure_exit_code (int exit_code)
{
  exit_failure = exit_code;
}
