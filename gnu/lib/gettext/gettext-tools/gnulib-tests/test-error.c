/* Test of error.h functions.
   Copyright (C) 2023 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Bruno Haible <bruno@clisp.org>, 2023.  */

#include <config.h>

#include "error.h"

#include <errno.h>

#include "macros.h"

/* Custom function to not show the program name in error messages.  */
static void
print_no_progname (void)
{
}

int
main ()
{
  /* Test error() function with zero STATUS and zero ERRNUM.  */
  error (0, 0, "bummer");
  /* With format string arguments.  */
  errno = EINVAL; /* should be ignored */
  error (0, 0, "Zonk %d%d%d is too large", 1, 2, 3);
  /* With non-ASCII characters.  */
  error (0, 0, "Pokémon started");
  /* Verify error_message_count.  */
  ASSERT (error_message_count == 3);

  /* Test error_at_line() function with zero STATUS and zero ERRNUM.  */
  error_at_line (0, 0, "d1/foo.c", 10, "invalid blub");
  error_at_line (0, 0, "d1/foo.c", 10, "invalid blarn");
  /* Verify error_message_count.  */
  ASSERT (error_message_count == 5);

  /* Test error_one_per_line.  */
  error_one_per_line = 1;
  error_at_line (0, 0, "d1/foo.c", 10, "unsupported glink");
  /* Another line number.  */
  error_at_line (0, 0, "d1/foo.c", 13, "invalid brump");
  /* Another file name.  */
  error_at_line (0, 0, "d2/foo.c", 13, "unsupported flinge");
  /* Same file name and same line number => message not shown.  */
  error_at_line (0, 0, "d2/foo.c", 13, "invalid bark");
  /* Verify error_message_count.  */
  ASSERT (error_message_count == 8);
  error_one_per_line = 0;

  /* Test error_print_progname.  */
  error_print_progname = print_no_progname;
  error (0, 0, "hammer");
  error (0, 0, "boing %d%d%d is too large", 1, 2, 3);
  #if 0
  /* The documentation does not describe the output if the file name is NULL. */
  error_at_line (0, 0, NULL, 42, "drummer too loud");
  #endif
  error_at_line (0, 0, "d2/bar.c", 11, "bark too loud");
  /* Verify error_message_count.  */
  ASSERT (error_message_count == 11);
  error_print_progname = NULL;

  /* Test error() function with nonzero ERRNUM.  */
  errno = EINVAL; /* should be ignored */
  error (0, EACCES, "can't steal");
  /* Verify error_message_count.  */
  ASSERT (error_message_count == 12);

  /* Test error() function with nonzero STATUS.  */
  error (4, 0, "fatal error");

  return 0;
}
