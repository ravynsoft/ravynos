/* Initialize OpenMP.

   Copyright (C) 2017-2023 Free Software Foundation, Inc.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation, either version 3 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#include <config.h>

/* Specification.  */
#include <omp.h>

#include <stdlib.h>

#include "c-ctype.h"

#if defined _AIX

/* Parse OMP environment variables without dependence on OMP.
   Return 0 for invalid values.  */
static unsigned long int
parse_omp_threads (char const* threads)
{
  unsigned long int ret = 0;

  if (threads == NULL)
    return ret;

  /* The OpenMP spec says that the value assigned to the environment variables
     "may have leading and trailing white space".  */
  while (*threads != '\0' && c_isspace (*threads))
    threads++;

  /* Convert it from positive decimal to 'unsigned long'.  */
  if (c_isdigit (*threads))
    {
      char *endptr = NULL;
      unsigned long int value = strtoul (threads, &endptr, 10);

      if (endptr != NULL)
        {
          while (*endptr != '\0' && c_isspace (*endptr))
            endptr++;
          if (*endptr == '\0')
            return value;
          /* Also accept the first value in a nesting level,
             since we can't determine the nesting level from env vars.  */
          else if (*endptr == ',')
            return value;
        }
    }

  return ret;
}

#endif

void
openmp_init (void)
{
  /* On AIX 7.2, in 32-bit mode, use of OpenMP on machines with 64 or 128
     processors makes the program fail with an error message
     "1587-120 SMP runtime library error. Memory allocation failed when creating thread number 62.".
     The workaround is to tell the OpenMP library to create fewer than 62
     threads.  This can be done through the OMP_THREAD_LIMIT environment
     variable.  */
#if defined _AIX
  if (sizeof (long) == sizeof (int))
    {
      /* Ensure that OMP_THREAD_LIMIT has a value <= 32.  */
      unsigned long int omp_env_limit =
        parse_omp_threads (getenv ("OMP_THREAD_LIMIT"));

      if (!(omp_env_limit > 0 && omp_env_limit <= 32))
        setenv ("OMP_THREAD_LIMIT", "32", 1);
    }
#endif
}
