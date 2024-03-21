/* Replacements for Posix functions and Posix functionality for MS-Windows.

Copyright (C) 2013-2023 Free Software Foundation, Inc.
This file is part of GNU Make.

GNU Make is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 3 of the License, or (at your option) any later
version.

GNU Make is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program.  If not, see <https://www.gnu.org/licenses/>.  */

#include "makeint.h"

#include <string.h>
#include <io.h>
#include <stdarg.h>
#include <errno.h>
#include <windows.h>

#include "dlfcn.h"

#include "job.h"

#if MAKE_LOAD

/* Support for dynamic loading of objects.  */

static DWORD last_err;

void *
dlopen (const char *file, int mode)
{
  char dllfn[MAX_PATH], *p;
  HANDLE dllhandle;

  if ((mode & ~(RTLD_LAZY | RTLD_NOW | RTLD_GLOBAL)) != 0)
    {
      errno = EINVAL;
      last_err = ERROR_INVALID_PARAMETER;
      return NULL;
    }

  if (!file)
    dllhandle = GetModuleHandle (NULL);
  else
    {
      /* MSDN says to be sure to use backslashes in the DLL file name.  */
      strcpy (dllfn, file);
      for (p = dllfn; *p; p++)
        if (*p == '/')
          *p = '\\';

      dllhandle = LoadLibrary (dllfn);
    }
  if (!dllhandle)
    last_err = GetLastError ();

  return dllhandle;
}

char *
dlerror (void)
{
  static char errbuf[1024];
  DWORD ret;

  if (!last_err)
    return NULL;

  ret = FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM
                       | FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL, last_err, 0, errbuf, sizeof (errbuf), NULL);
  while (ret > 0 && (errbuf[ret - 1] == '\n' || errbuf[ret - 1] == '\r'))
    --ret;

  errbuf[ret] = '\0';
  if (!ret)
    sprintf (errbuf, "Error code %lu", last_err);

  last_err = 0;
  return errbuf;
}

void *
dlsym (void *handle, const char *name)
{
  FARPROC addr = NULL;

  if (!handle || handle == INVALID_HANDLE_VALUE)
    {
      last_err = ERROR_INVALID_PARAMETER;
      return NULL;
    }

  addr = GetProcAddress (handle, name);
  if (!addr)
    last_err = GetLastError ();

  return (void *)addr;
}

int
dlclose (void *handle)
{
  if (!handle || handle == INVALID_HANDLE_VALUE)
    return -1;
  if (!FreeLibrary (handle))
    return -1;

  return 0;
}


#endif  /* MAKE_LOAD */


/* MS runtime's isatty returns non-zero for any character device,
   including the null device, which is not what we want.  */
int
isatty (int fd)
{
  HANDLE fh = (HANDLE) _get_osfhandle (fd);
  DWORD con_mode;

  if (fh == INVALID_HANDLE_VALUE)
    {
      errno = EBADF;
      return 0;
    }
  if (GetConsoleMode (fh, &con_mode))
    return 1;

  errno = ENOTTY;
  return 0;
}

char *
ttyname (int fd)
{
  /* This "knows" that Make only asks about stdout and stderr.  A more
     sophisticated implementation should test whether FD is open for
     input or output.  We can do that by looking at the mode returned
     by GetConsoleMode.  */
  return "CONOUT$";
}
