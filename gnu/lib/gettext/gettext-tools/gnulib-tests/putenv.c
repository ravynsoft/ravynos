/* Copyright (C) 1991, 1994, 1997-1998, 2000, 2003-2023 Free Software
   Foundation, Inc.

   NOTE: The canonical source of this file is maintained with the GNU C
   Library.  Bugs can be reported to bug-glibc@prep.ai.mit.edu.

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
#include <stdlib.h>

#include <stddef.h>

/* Include errno.h *after* sys/types.h to work around header problems
   on AIX 3.2.5.  */
#include <errno.h>
#ifndef __set_errno
# define __set_errno(ev) ((errno) = (ev))
#endif

#include <string.h>
#include <unistd.h>

#if defined _WIN32 && ! defined __CYGWIN__
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
#endif

#if _LIBC
# if HAVE_GNU_LD
#  define environ __environ
# else
extern char **environ;
# endif
#endif

#if _LIBC
/* This lock protects against simultaneous modifications of 'environ'.  */
# include <bits/libc-lock.h>
__libc_lock_define_initialized (static, envlock)
# define LOCK   __libc_lock_lock (envlock)
# define UNLOCK __libc_lock_unlock (envlock)
#else
# define LOCK
# define UNLOCK
#endif

#if defined _WIN32 && ! defined __CYGWIN__
/* Don't assume that UNICODE is not defined.  */
# undef SetEnvironmentVariable
# define SetEnvironmentVariable SetEnvironmentVariableA
#endif

static int
_unsetenv (const char *name)
{
  size_t len;
#if !HAVE_DECL__PUTENV
  char **ep;
#endif

  if (name == NULL || *name == '\0' || strchr (name, '=') != NULL)
    {
      __set_errno (EINVAL);
      return -1;
    }

  len = strlen (name);

#if HAVE_DECL__PUTENV
  {
    int putenv_result;
    char *name_ = malloc (len + 2);
    memcpy (name_, name, len);
    name_[len] = '=';
    name_[len + 1] = 0;
    putenv_result = _putenv (name_);
    free (name_);
    return putenv_result;
  }
#else

  LOCK;

  ep = environ;
  while (*ep != NULL)
    if (!strncmp (*ep, name, len) && (*ep)[len] == '=')
      {
        /* Found it.  Remove this pointer by moving later ones back.  */
        char **dp = ep;

        do
          dp[0] = dp[1];
        while (*dp++);
        /* Continue the loop in case NAME appears again.  */
      }
    else
      ++ep;

  UNLOCK;

  return 0;
#endif
}


/* Put STRING, which is of the form "NAME=VALUE", in the environment.
   If STRING contains no '=', then remove STRING from the environment.  */
int
putenv (char *string)
{
  const char *name_end = strchr (string, '=');
  char **ep;

  if (name_end == NULL)
    {
      /* Remove the variable from the environment.  */
      return _unsetenv (string);
    }

#if HAVE_DECL__PUTENV
  /* Rely on _putenv to allocate the new environment.  If other
     parts of the application use _putenv, the !HAVE_DECL__PUTENV code
     would fight over who owns the environ vector, causing a crash.  */
  if (name_end[1])
    return _putenv (string);
  else
    {
      /* _putenv ("NAME=") unsets NAME, so invoke _putenv ("NAME= ")
         to allocate the environ vector and then replace the new
         entry with "NAME=".  */
      int putenv_result;
      char *name_x = malloc (name_end - string + sizeof "= ");
      if (!name_x)
        return -1;
      memcpy (name_x, string, name_end - string + 1);
      name_x[name_end - string + 1] = ' ';
      name_x[name_end - string + 2] = 0;
      putenv_result = _putenv (name_x);
      for (ep = environ; *ep; ep++)
        if (strcmp (*ep, name_x) == 0)
          {
            *ep = string;
            break;
          }
# if defined _WIN32 && ! defined __CYGWIN__
      if (putenv_result == 0)
        {
          /* _putenv propagated "NAME= " into the subprocess environment;
             fix that by calling SetEnvironmentVariable directly.  */
          name_x[name_end - string] = 0;
          putenv_result = SetEnvironmentVariable (name_x, "") ? 0 : -1;
          errno = ENOMEM; /* ENOMEM is the only way to fail.  */
        }
# endif
      free (name_x);
      return putenv_result;
    }
#else
  for (ep = environ; *ep; ep++)
    if (strncmp (*ep, string, name_end - string) == 0
        && (*ep)[name_end - string] == '=')
      break;

  if (*ep)
    *ep = string;
  else
    {
      static char **last_environ = NULL;
      size_t size = ep - environ;
      char **new_environ = malloc ((size + 2) * sizeof *new_environ);
      if (! new_environ)
        return -1;
      new_environ[0] = string;
      memcpy (new_environ + 1, environ, (size + 1) * sizeof *new_environ);
      free (last_environ);
      last_environ = new_environ;
      environ = new_environ;
    }

  return 0;
#endif
}
