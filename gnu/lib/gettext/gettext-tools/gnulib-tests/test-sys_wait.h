/* Test of macros shared between <sys/wait.h> and <stdlib.h>.
   Copyright (C) 2010-2023 Free Software Foundation, Inc.

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

/* Written by Eric Blake <ebb9@byu.net>, 2010.  */

static int
test_sys_wait_macros (void)
{
  /* Check subset of <sys/wait.h> macros that must be visible here.
     Note that some of these macros are only portable when operating
     on an lvalue.  */
  int i;
  for (i = 0; i < 0x8000; i = (i ? i << 1 : 1))
    {
      /* POSIX requires that for all valid process statuses, that
         exactly one of these three macros is true.  But not all
         possible 16-bit values map to valid process status.
         Traditionally, 8 of the bits are for WIFEXITED, 7 of the bits
         to tell between WIFSIGNALED and WIFSTOPPED, and either 0x80
         or 0x8000 to flag that core was also dumped.  Since we don't
         know which byte is WIFEXITED, we skip the both possible bits
         that can signal core dump.  */
      if (i == 0x80)
        continue;
      if (!!WIFSIGNALED (i) + !!WIFEXITED (i) + !!WIFSTOPPED (i) != 1)
        return 1;
    }
  i = WEXITSTATUS (i) + WSTOPSIG (i) + WTERMSIG (i);

#if 0
  switch (i)
    {
  /* Gnulib doesn't guarantee these, yet.  */
    case WNOHANG:
    case WUNTRACED:
      break;
    }
#endif
  return 0;
}
