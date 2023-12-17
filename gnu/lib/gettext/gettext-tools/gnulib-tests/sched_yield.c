/* Schedule other threads to run.
   Copyright (C) 2019-2023 Free Software Foundation, Inc.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Bruno Haible <bruno@clisp.org>, 2019.  */

#include <config.h>

/* Specification.  */
#include <sched.h>

#if (defined _WIN32 && ! defined __CYGWIN__) && USE_WINDOWS_THREADS
/* Use Windows threads.  */

# define WIN32_LEAN_AND_MEAN  /* avoid including junk */
# include <windows.h>

int
sched_yield (void)
{
  Sleep (0);
  return 0;
}

#elif defined __KLIBC__
/* OS/2 kLIBC implementation */

# define INCL_DOS
# include <os2.h>

int
sched_yield (void)
{
  DosSleep (0);
  return 0;
}

#else
/* Provide a dummy implementation for single-threaded applications.  */

int
sched_yield (void)
{
  return 0;
}

#endif
