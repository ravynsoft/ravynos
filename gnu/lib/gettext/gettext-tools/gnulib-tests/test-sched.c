/* Test of <sched.h> substitute.
   Copyright (C) 2008-2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <bruno@clisp.org>, 2008.  */

#include <config.h>

#include <sched.h>

/* Check that 'struct sched_param' is defined.  */
static struct sched_param a;

/* Check that the SCHED_* macros are defined and compile-time constants.  */
int b[] = { SCHED_FIFO, SCHED_RR, SCHED_OTHER };

/* Check that the types are all defined.  */
pid_t t1;

int f1;

int
main ()
{
  /* Check fields of 'struct sched_param'.  */
  f1 = a.sched_priority;

  return 0;
}
