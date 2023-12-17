/* Test of optional automatic memory allocation.
   Copyright (C) 2005, 2007, 2009-2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <bruno@clisp.org>, 2007.  */

#include <config.h>

#include <alloca.h>

#if HAVE_ALLOCA

static void
do_allocation (int n)
{
  void *volatile ptr = alloca (n);
  (void) ptr;
}

void (*func) (int) = do_allocation;

#endif

int
main ()
{
#if HAVE_ALLOCA
  int i;

  /* Repeat a lot of times, to make sure there's no memory leak.  */
  for (i = 0; i < 100000; i++)
    {
      /* Try various values.
         n = 0 gave a crash on Alpha with gcc-2.5.8.
         Some versions of Mac OS X have a stack size limit of 512 KB.  */
      func (34);
      func (134);
      func (399);
      func (510823);
      func (129321);
      func (0);
      func (4070);
      func (4095);
      func (1);
      func (16582);
    }
#endif

  return 0;
}
